/*
 * chr - Andrew Chronister
 *
 * 12.19.2019
 *
 * Updated linux layer for 4coder
 *
 */

// TOP

#define FPS 60
#define frame_useconds (1000000 / FPS)

#include "4coder_base_types.h"
#include "4coder_version.h"
#include "4coder_events.h"

#include "4coder_table.h"
#include "4coder_types.h"
#include "4coder_default_colors.h"

#include "4coder_system_types.h"
#define STATIC_LINK_API
#include "generated/system_api.h"

#include "4ed_font_interface.h"
#define STATIC_LINK_API
#include "generated/graphics_api.h"
#define STATIC_LINK_API
#include "generated/font_api.h"

#include "4ed_font_set.h"
#include "4ed_render_target.h"
#include "4ed_search_list.h"
#include "4ed.h"

#include "generated/system_api.cpp"
#include "generated/graphics_api.cpp"
#include "generated/font_api.cpp"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_events.cpp"
#include "4coder_hash_functions.cpp"
#include "4coder_table.cpp"
#include "4coder_log.cpp"

#include "4ed_search_list.cpp"

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define Cursor XCursor
#undef function
#include <X11/Xlib.h>
#define function static
#undef Cursor

////////////////////////////

struct Linux_Vars {
    int epoll;
    Node free_linux_objects;

    System_Mutex global_frame_mutex;
};

enum {
    LINUX_4ED_EVENT_CLI          = (UINT64_C(5) << 32),
};

typedef i32 Linux_Object_Kind;
enum {
    LinuxObjectKind_Thread = 1,
    LinuxObjectKind_Mutex = 2,
    LinuxObjectKind_ConditionVariable = 3,
};

struct Linux_Object {
    Linux_Object_Kind kind;
    Node node;
    union {
        struct {
            pthread_t pthread;
            Thread_Function* proc;
            void* ptr;
        } thread;
        pthread_mutex_t mutex;
        pthread_cond_t condition_variable;
    };
};


////////////////////////////

global Linux_Vars linuxvars;

////////////////////////////

internal Linux_Object*
linux_alloc_object(Linux_Object_Kind kind){
    Linux_Object* result = NULL;
    if (linuxvars.free_linux_objects.next != &linuxvars.free_linux_objects) {
        result = CastFromMember(Linux_Object, node, linuxvars.free_linux_objects.next);
    }
    if (result == NULL) {
        i32 count = 512;
        Linux_Object* objects = (Linux_Object*)system_memory_allocate(
            sizeof(Linux_Object), file_name_line_number_lit_u8);
        objects[0].node.prev = &linuxvars.free_linux_objects;
        for (i32 i = 1; i < count; ++i) {
            objects[i - 1].node.next = &objects[i].node;
            objects[i].node.prev = &objects[i - 1].node;
        }
        objects[count - 1].node.next = &linuxvars.free_linux_objects;
        linuxvars.free_linux_objects.prev = &objects[count - 1].node;
        result = CastFromMember(Linux_Object, node, linuxvars.free_linux_objects.next);
    }
    Assert(result != 0);
    dll_remove(&result->node);
    block_zero_struct(result);
    result->kind = kind;
    return result;
}

internal void
linux_free_object(Linux_Object *object){
    if (object->node.next != 0){
        dll_remove(&object->node);
    }
    dll_insert(&linuxvars.free_linux_objects, &object->node);
}

internal
system_get_path_sig(){
    // Arena* arena, System_Path_Code path_code
    String_Const_u8 result = {};
    switch (path_code){
        case SystemPath_CurrentDirectory:
        {
            // glibc extension: getcwd allocates its own memory if passed NULL
            char *working_dir = getcwd(NULL, 0);
            u64 working_dir_len = cstring_length(working_dir);
            u8 *out = push_array(arena, u8, working_dir_len);
            block_copy(out, working_dir, working_dir_len);
            free(working_dir);
            result = SCu8(out, working_dir_len);
        }break;

        case SystemPath_Binary:
        {
            // linux-specific: binary path symlinked at /proc/self/exe
            ssize_t binary_path_len = readlink("/proc/self/exe", NULL, 0);
            u8* out = push_array(arena, u8, binary_path_len);
            readlink("/proc/self/exe", (char*)out, binary_path_len);
            String_u8 out_str = Su8(out, binary_path_len);
            out_str.string = string_remove_last_folder(out_str.string);
            string_null_terminate(&out_str);
            result = out_str.string;
        }break;
    }
    return(result);
}

internal
system_get_canonical_sig(){
    // TODO(andrew): Resolve symlinks ?
    // TODO(andrew): Resolve . and .. in paths
    // TODO(andrew): Use realpath(3)
	return name;
}


internal int
linux_system_get_file_list_filter(const struct dirent *dirent) {
    String_Const_u8 file_name = SCu8((u8*)dirent->d_name);
    if (string_match(file_name, string_u8_litexpr("."))) {
        return 0;
    }
    else if (string_match(file_name, string_u8_litexpr(".."))) {
        return 0;
    }
    return 1;
}

internal int
linux_u64_from_timespec(const struct timespec timespec) {
    return timespec.tv_nsec + 1000000000 * timespec.tv_sec;
}

internal File_Attribute_Flag
linux_convert_file_attribute_flags(int mode) {
    File_Attribute_Flag result = {};
    MovFlag(mode, S_IFDIR, result, FileAttribute_IsDirectory);
    return result;
}

internal File_Attributes
linux_file_attributes_from_struct_stat(struct stat file_stat) {
    File_Attributes result = {};
    result.size = file_stat.st_size;
    result.last_write_time = linux_u64_from_timespec(file_stat.st_mtim);
    result.flags = linux_convert_file_attribute_flags(file_stat.st_mode);
    return result;
}

internal
system_get_file_list_sig(){
    File_List result = {};
    String_Const_u8 search_pattern = {};
    if (character_is_slash(string_get_character(directory, directory.size - 1))){
        search_pattern = push_u8_stringf(arena, "%.*s*", string_expand(directory));
    }
    else{
        search_pattern = push_u8_stringf(arena, "%.*s/*", string_expand(directory));
    }

    struct dirent** dir_ents = NULL;
    int num_ents = scandir(
        (const char*)search_pattern.str, &dir_ents, linux_system_get_file_list_filter, alphasort);

    File_Info *first = 0;
    File_Info *last = 0;
    for (int i = 0; i < num_ents; ++i) {
        struct dirent* dirent = dir_ents[i];
        File_Info *info = push_array(arena, File_Info, 1);
        sll_queue_push(first, last, info);

        info->file_name = SCu8((u8*)dirent->d_name);

        struct stat file_stat;
        stat((const char*)dirent->d_name, &file_stat);
        info->attributes = linux_file_attributes_from_struct_stat(file_stat);
    }

    result.infos = push_array(arena, File_Info*, num_ents);
    result.count = num_ents;
    i32 info_index = 0;
    for (File_Info* node = first; node != NULL; node = node->next) {
        result.infos[info_index] = node;
        info_index += 1;
    }
    return result;
}

internal
system_quick_file_attributes_sig(){
    struct stat file_stat;
    stat((const char*)file_name.str, &file_stat);
    return linux_file_attributes_from_struct_stat(file_stat);
}

internal
system_load_handle_sig(){
    int fd = open(file_name, O_RDONLY);
    if (fd != -1) {
        *(int*)out = fd;
        return true;
    }
    return false;
}

internal
system_load_attributes_sig(){
    struct stat file_stat;
    fstat(*(int*)&handle, &file_stat);
    return linux_file_attributes_from_struct_stat(file_stat);
}

internal
system_load_file_sig(){
    int fd = *(int*)&handle;
    int bytes_read = read(fd, buffer, size);
    if (bytes_read == size) {
        return true;
    }
    return false;
}

internal
system_load_close_sig(){
    int fd = *(int*)&handle;
    return close(fd) == 0;
}

internal
system_save_file_sig(){
    File_Attributes result = {};
    int fd = open(file_name, O_WRONLY, O_CREAT);
    if (fd != -1) {
        int bytes_written = write(fd, data.str, data.size);
        if (bytes_written != -1) {
            struct stat file_stat;
            fstat(fd, &file_stat);
            return linux_file_attributes_from_struct_stat(file_stat);
        }
    }
    return result;
}

typedef void* shared_object_handle;

internal
system_load_library_sig(){
    shared_object_handle library = dlopen((const char*)file_name.str, RTLD_LAZY);
    if (library != NULL) {
        *(shared_object_handle*)out = library;
        return true;
    }
    return false;
}

internal
system_release_library_sig(){
    return dlclose(*(shared_object_handle*)&handle) == 0;
}

internal
system_get_proc_sig(){
    return (Void_Func*)dlsym(*(shared_object_handle*)&handle, proc_name);
}

internal
system_now_time_sig(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    return linux_u64_from_timespec(time);
}

// system_wake_up_timer_create_sig
// system_wake_up_timer_release_sig
// system_wake_up_timer_set_sig
// system_signal_step_sig

internal
system_sleep_sig(){
    struct timespec requested;
    struct timespec remaining;
    u64 seconds = microseconds / Million(1);
    requested.tv_sec = seconds;
    requested.tv_nsec = (microseconds - seconds * Million(1)) * Thousand(1);
    nanosleep(&requested, &remaining);
}

// system_post_clipboard_sig

internal
system_cli_call_sig() {
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1){
        perror("system_cli_call: pipe");
        return 0;
    }
    
    pid_t child_pid = fork();
    if (child_pid == -1){
        perror("system_cli_call: fork");
        return 0;
    }
    
    enum { PIPE_FD_READ, PIPE_FD_WRITE };
    
    // child
    if (child_pid == 0){
        close(pipe_fds[PIPE_FD_READ]);
        dup2(pipe_fds[PIPE_FD_WRITE], STDOUT_FILENO);
        dup2(pipe_fds[PIPE_FD_WRITE], STDERR_FILENO);
        
        if (chdir(path) == -1){
            perror("system_cli_call: chdir");
            exit(1);
        }
        
        char* argv[] = { "sh", "-c", script, NULL };
        
        if (execv("/bin/sh", argv) == -1){
            perror("system_cli_call: execv");
        }
        exit(1);
    }
    else{
        close(pipe_fds[PIPE_FD_WRITE]);
        
        *(pid_t*)&cli_out->proc = child_pid;
        *(int*)&cli_out->out_read = pipe_fds[PIPE_FD_READ];
        *(int*)&cli_out->out_write = pipe_fds[PIPE_FD_WRITE];
        
        struct epoll_event e = {};
        e.events = EPOLLIN | EPOLLET;
        e.data.u64 = LINUX_4ED_EVENT_CLI;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, pipe_fds[PIPE_FD_READ], &e);
    }
    
    return(true);
}

internal
system_cli_begin_update_sig() {
    // NOTE(inso): I don't think anything needs to be done here.
}

internal
system_cli_update_step_sig(){
    int pipe_read_fd = *(int*)&cli->out_read;
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(pipe_read_fd, &fds);
    
    struct timeval tv = {};
    
    size_t space_left = max;
    char* ptr = dest;
    
    while (space_left > 0 && select(pipe_read_fd + 1, &fds, NULL, NULL, &tv) == 1){
        ssize_t num = read(pipe_read_fd, ptr, space_left);
        if (num == -1){
            perror("system_cli_update_step: read");
        } else if (num == 0){
            // NOTE(inso): EOF
            break;
        } else {
            ptr += num;
            space_left -= num;
        }
    }
    
    *amount = (ptr - dest);
    return((ptr - dest) > 0);
}

internal
system_cli_end_update_sig(){
    pid_t pid = *(pid_t*)&cli->proc;
    b32 close_me = false;
    
    int status;
    if (pid && waitpid(pid, &status, WNOHANG) > 0){
        cli->exit = WEXITSTATUS(status);
        
        close_me = true;
        close(*(int*)&cli->out_read);
        close(*(int*)&cli->out_write);
        
        struct epoll_event e = {};
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_DEL, *(int*)&cli->out_read, &e);
    }
    
    return(close_me);
}

// system_open_color_picker

internal int
linux_get_xsettings_dpi(Display* dpy, int screen)
{
    struct XSettingHeader {
        u8 type;
        u8 pad0;
        u16 name_len;
        char name[0];
    };
    
    struct XSettings {
        u8 byte_order;
        u8 pad[3];
        u32 serial;
        u32 num_settings;
    };
    
    enum { XSettingsTypeInt, XSettingsTypeString, XSettingsTypeColor };
    
    int dpi = -1;
    unsigned char* prop = NULL;
    char sel_buffer[64];
    struct XSettings* xs;
    const char* p;
    
    snprintf(sel_buffer, sizeof(sel_buffer), "_XSETTINGS_S%d", screen);
    
    Atom XSET_SEL = XInternAtom(dpy, sel_buffer, True);
    Atom XSET_SET = XInternAtom(dpy, "_XSETTINGS_SETTINGS", True);
    
    if (XSET_SEL == None || XSET_SET == None){
        //LOG("XSETTINGS unavailable.\n");
        return(dpi);
    }
    
    Window xset_win = XGetSelectionOwner(dpy, XSET_SEL);
    if (xset_win == None){
        // TODO(inso): listen for the ClientMessage about it becoming available?
        //             there's not much point atm if DPI scaling is only done at startup 
        goto out;
    }
    
    {
        Atom type;
        int fmt;
        unsigned long pad, num;
        
        if (XGetWindowProperty(dpy, xset_win, XSET_SET, 0, 1024, False, XSET_SET, &type, &fmt, &num, &pad, &prop) != Success){
            //LOG("XSETTINGS: GetWindowProperty failed.\n");
            goto out;
        }
        
        if (fmt != 8){
            //LOG("XSETTINGS: Wrong format.\n");
            goto out;
        }
    }
    
    xs = (struct XSettings*)prop;
    p  = (char*)(xs + 1);
    
    if (xs->byte_order != 0){
        //LOG("FIXME: XSETTINGS not host byte order?\n");
        goto out;
    }
    
    for (int i = 0; i < xs->num_settings; ++i){
        struct XSettingHeader* h = (struct XSettingHeader*)p;
        
        p += sizeof(struct XSettingHeader);
        p += h->name_len;
        p += ((4 - (h->name_len & 3)) & 3);
        p += 4; // serial
        
        switch (h->type){
            case XSettingsTypeInt: {
                if (strncmp(h->name, "Xft/DPI", h->name_len) == 0){
                    dpi = *(i32*)p;
                    if (dpi != -1) dpi /= 1024;
                }
                p += 4;
            } break;
            
            case XSettingsTypeString: {
                u32 len = *(u32*)p;
                p += 4;
                p += len;
                p += ((4 - (len & 3)) & 3);
            } break;
            
            case XSettingsTypeColor: {
                p += 8;
            } break;
            
            default: {
                //LOG("XSETTINGS: Got invalid type...\n");
                goto out;
            } break;
        }
    }
    
    out:
    if (prop){
        XFree(prop);
    }
    
    return dpi;
}

// internal
// system_get_screen_scale_factor_sig(){
//     return linux_get_xsettings_dpi() / 96.0f;
// }

internal void*
linux_thread_proc_start(void* arg) {
    Linux_Object* info = (Linux_Object*)arg;
    Assert(info->kind == LinuxObjectKind_Thread);
    info->thread.proc(info->thread.ptr);
    return NULL;
}

internal
system_thread_launch_sig(){
    System_Thread result = {};
    Linux_Object* thread_info = linux_alloc_object(LinuxObjectKind_Thread);
    thread_info->thread.proc = proc;
    thread_info->thread.ptr = ptr;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int create_result = pthread_create(
        &thread_info->thread.pthread, &thread_attr, linux_thread_proc_start, (void*)thread_info);
    pthread_attr_destroy(&thread_attr);
    // TODO(andrew): Need to wait for thread to confirm it launched?
    if (create_result == 0) {
        static_assert(sizeof(Linux_Object*) <= sizeof(System_Thread));
        *(Linux_Object**)&result = thread_info;
        return result;
    }
    return result;
}

internal
system_thread_join_sig(){
    Linux_Object* object = *(Linux_Object**)&thread;
    void* retval_ignored;
    int result = pthread_join(object->thread.pthread, &retval_ignored);
}

internal
system_thread_free_sig(){
    Linux_Object* object = *(Linux_Object**)&thread;
    Assert(object->kind == LinuxObjectKind_Thread);
    linux_free_object(object);
}

internal
system_thread_get_id_sig(){
    pthread_t tid = pthread_self();
    Assert(tid <= (u64)max_i32);
    return (i32)tid;
}

internal
system_acquire_global_frame_mutex_sig(){
    if (tctx->kind == ThreadKind_AsyncTasks){
        system_mutex_acquire(linuxvars.global_frame_mutex);
    }
}

internal
system_release_global_frame_mutex_sig(){
    if (tctx->kind == ThreadKind_AsyncTasks){
        system_mutex_release(linuxvars.global_frame_mutex);
    }
}

internal
system_mutex_make_sig(){
    System_Mutex result = {};
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_Mutex);
    pthread_mutex_init(&object->mutex, NULL);
    *(Linux_Object**)&result = object;
    return result;
}

internal
system_mutex_acquire_sig(){
    Linux_Object* object = *(Linux_Object**)&mutex;
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_lock(&object->mutex);
}

internal
system_mutex_release_sig(){
    Linux_Object* object = *(Linux_Object**)&mutex;
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_unlock(&object->mutex);
}

internal
system_mutex_free_sig(){
    Linux_Object* object = *(Linux_Object**)&mutex;
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_destroy(&object->mutex);
    linux_free_object(object);
}

internal
system_condition_variable_make_sig(){
    System_Condition_Variable result = {};
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_ConditionVariable);
    pthread_cond_init(&object->condition_variable, NULL);
    *(Linux_Object**)&result = object;
    return result;
}

internal
system_condition_variable_wait_sig(){
    Linux_Object* cv_object = *(Linux_Object**)&cv;
    Linux_Object* mutex_object = *(Linux_Object**)&mutex;
    Assert(cv_object->kind == LinuxObjectKind_ConditionVariable);
    Assert(mutex_object->kind == LinuxObjectKind_Mutex);
    pthread_cond_wait(&cv_object->condition_variable, &mutex_object->mutex);
}

internal
system_condition_variable_signal_sig(){
    Linux_Object* object = *(Linux_Object**)&cv;
    Assert(object->kind == LinuxObjectKind_ConditionVariable);
    pthread_cond_signal(&object->condition_variable);
}

internal
system_condition_variable_free_sig(){
    Linux_Object* object = *(Linux_Object**)&cv;
    Assert(object->kind == LinuxObjectKind_ConditionVariable);
    pthread_cond_destroy(&object->condition_variable);
    linux_free_object(object);
}

internal
system_memory_allocate_sig(){
    void* result = mmap(
        NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // TODO(andrew): Allocation tracking?
    return result;
}

internal
system_memory_set_protection_sig(){
    int protect = 0;
    MovFlag(flags, MemProtect_Read, protect, PROT_READ);
    MovFlag(flags, MemProtect_Write, protect, PROT_WRITE);
    MovFlag(flags, MemProtect_Execute, protect, PROT_EXEC);
    int result = mprotect(ptr, size, protect);
    return result == 0;
}

internal
system_memory_free_sig(){
    munmap(ptr, size);
}

int main(int argc, char **argv){
}
