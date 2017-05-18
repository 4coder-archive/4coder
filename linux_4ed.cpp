/*
 * Mr. 4th Dimention - Allen Webster
 *  (Mostly by insofaras)
 *
 * 14.11.2015
 *
 * Linux layer for 4coder
 *
 */

// TOP

#include <assert.h>
#include <string.h>
#include "4tech_defines.h"
#include "4coder_API/version.h"

#include "4coder_lib/4coder_utf8.h"

#if defined(FRED_SUPER)
# include "4coder_API/keycodes.h"
# include "4coder_API/style.h"

# define FSTRING_IMPLEMENTATION
# define FSTRING_C
# include "4coder_lib/4coder_string.h"
# include "4coder_lib/4coder_mem.h"

# include "4coder_API/types.h"
# include "4ed_os_custom_api.h"

#else
# include "4coder_default_bindings.cpp"
#endif

#include "4ed_math.h"

#include "4ed_system.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <time.h>
#include <locale.h>
#include <memory.h>
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <ucontext.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <linux/fs.h>
#include <linux/input.h>


//
// Linux macros
//

#define LINUX_MAX_PASTE_CHARS 0x10000L
#define FPS 60L
#define frame_useconds (1000000UL / FPS)

#define LinuxGetMemory(size) LinuxGetMemory_(size, __LINE__, __FILE__)

#if FRED_INTERNAL
#define LINUX_FN_DEBUG(fmt, ...) do { \
    fprintf(stderr, "%s: " fmt "\n", __func__, ##__VA_ARGS__); \
} while(0)
#else
#define LINUX_FN_DEBUG(fmt, ...)
#endif

#define SUPPORT_DPI 1

#define InterlockedCompareExchange(dest, ex, comp) __sync_val_compare_and_swap((dest), (comp), (ex))

#include "4ed_file_track.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_system_shared.h"

//
// Linux structs / enums
//

enum {
    LINUX_4ED_EVENT_X11          = (UINT64_C(1) << 32),
    LINUX_4ED_EVENT_X11_INTERNAL = (UINT64_C(2) << 32),
    LINUX_4ED_EVENT_STEP         = (UINT64_C(3) << 32),
    LINUX_4ED_EVENT_STEP_TIMER   = (UINT64_C(4) << 32),
    LINUX_4ED_EVENT_CLI          = (UINT64_C(5) << 32),
};

struct Linux_Coroutine {
    Coroutine coroutine;
    Linux_Coroutine *next;
    ucontext_t ctx, yield_ctx;
    stack_t stack;
    b32 done;
};

struct Thread_Context{
    u32 job_id;
    b32 running;
    b32 cancel;
    
    Work_Queue *queue;
    u32 id;
    u32 group_id;
    pthread_t handle;
};

struct Thread_Group{
    Thread_Context *threads;
    i32 count;
    
    Unbounded_Work_Queue queue;
    
    i32 cancel_lock0;
    i32 cancel_cv0;
};

struct Linux_Vars{
    Display *XDisplay;
    Window XWindow;
    Render_Target target;
    
    XIM input_method;
    XIMStyle input_style;
    XIC input_context;
    
    Application_Step_Input input;
    
    String clipboard_contents;
    String clipboard_outgoing;
    b32 new_clipboard;
    
    Atom atom_TARGETS;
    Atom atom_CLIPBOARD;
    Atom atom_UTF8_STRING;
    Atom atom__NET_WM_STATE;
    Atom atom__NET_WM_STATE_MAXIMIZED_HORZ;
    Atom atom__NET_WM_STATE_MAXIMIZED_VERT;
    Atom atom__NET_WM_STATE_FULLSCREEN;
    Atom atom__NET_WM_PING;
    Atom atom__NET_WM_WINDOW_TYPE;
    Atom atom__NET_WM_WINDOW_TYPE_NORMAL;
    Atom atom__NET_WM_PID;
    Atom atom_WM_DELETE_WINDOW;
    
    b32 has_xfixes;
    int xfixes_selection_event;
    
    int epoll;
    
    int step_timer_fd;
    int step_event_fd;
    int x11_fd;
    int inotify_fd;
    
    u64 last_step;
    
    b32 keep_running;
    
    Application_Mouse_Cursor cursor;
    b32 hide_cursor;
    Cursor hidden_cursor;
    
    void *app_code;
    void *custom;
    
    Thread_Memory *thread_memory;
    Thread_Group groups[THREAD_GROUP_COUNT];
    Work_Queue queues[THREAD_GROUP_COUNT];
    pthread_mutex_t locks[LOCK_COUNT];
    pthread_cond_t conds[8];
    sem_t thread_semaphore;
    
#if SUPPORT_DPI
    i32 dpi_x, dpi_y;
#endif
    
    Plat_Settings settings;
    System_Functions system;
    App_Functions app;
    Custom_API custom_api;
    b32 vsync;
    
    Linux_Coroutine coroutine_data[18];
    Linux_Coroutine *coroutine_free;
};

//
// Linux globals
//

global Linux_Vars linuxvars;
global Application_Memory memory_vars;

//
// Linux forward declarations
//

internal void        LinuxScheduleStep(void);

internal Plat_Handle LinuxSemToHandle(sem_t*);
internal sem_t*      LinuxHandleToSem(Plat_Handle);

internal Plat_Handle LinuxFDToHandle(int);
internal int         LinuxHandleToFD(Plat_Handle);

internal void        LinuxStringDup(String*, void*, size_t);
internal void        LinuxToggleFullscreen(Display*, Window);
internal void        LinuxFatalErrorMsg(const char* msg);

internal             Sys_Acquire_Lock_Sig(system_acquire_lock);
internal             Sys_Release_Lock_Sig(system_release_lock);

internal void        system_wait_cv(i32, i32);
internal void        system_signal_cv(i32, i32);

//
// Shared system functions (system_shared.h)
//

internal void*
LinuxGetMemory_(i32 size, i32 line_number, char *file_name){
    void *result = 0;
    
    Assert(size != 0);
    
    size_t real_size = size + sizeof(size_t);
    result = mmap(0, real_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(result == MAP_FAILED){
        perror("mmap");
        result = NULL;
    } else {
        memcpy(result, &real_size, sizeof(size_t));
        result = (char*)result + sizeof(size_t);
    }
    
    return(result);
}

internal void
LinuxFreeMemory(void *block){
    if (block){
        block = (char*)block - sizeof(size_t);
        size_t size = *(size_t*)block;
        munmap(block, size);
    }
}

internal
Sys_File_Can_Be_Made_Sig(system_file_can_be_made){
    b32 result = access((char*)filename, W_OK) == 0;
    LINUX_FN_DEBUG("%s = %d", filename, result);
    return(result);
}

internal
Sys_Get_Binary_Path_Sig(system_get_binary_path){
    ssize_t size = readlink("/proc/self/exe", out->str, out->memory_size - 1);
    if(size != -1 && size < out->memory_size - 1){
        out->size = size;
        remove_last_folder(out);
        terminate_with_null(out);
        size = out->size;
    } else {
        size = 0;
    }
    
    return size;
}

//
// custom.h
//

internal
Sys_Memory_Allocate_Sig(system_memory_allocate){
    // NOTE(allen): This must return the exact base of the vpage.
    // We will count on the user to keep track of size themselves.
    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(result == MAP_FAILED){
        perror("mmap");
        result = NULL;
    }
    return(result);
}

internal 
Sys_Memory_Set_Protection_Sig(system_memory_set_protection){
    // NOTE(allen):
    // There is no such thing as "write only" in windows
    // so I just made write = write + read in all cases.
    bool32 result = 1;
    int protect = 0;
    
    flags = flags & 0x7;
    
    switch (flags){
        case 0:
        protect = PROT_NONE; break;
        
        case MemProtect_Read:
        protect = PROT_READ; break;
        
        case MemProtect_Write:
        case MemProtect_Read|MemProtect_Write:
        protect = PROT_READ | PROT_WRITE; break;
        
        case MemProtect_Execute:
        protect = PROT_EXEC; break;
        
        case MemProtect_Execute|MemProtect_Read:
        protect = PROT_READ | PROT_EXEC; break;
        
        // NOTE(inso): some W^X protection things might be unhappy about this one
        case MemProtect_Execute|MemProtect_Write:
        case MemProtect_Execute|MemProtect_Write|MemProtect_Read:
        protect = PROT_READ | PROT_WRITE | PROT_EXEC; break;
    }
    
    if(mprotect(ptr, size, protect) == -1){
        result = 0;
        perror("mprotect");
    }
    
    return(result);
}

internal
Sys_Memory_Free_Sig(system_memory_free){
    // NOTE(allen): This must take the exact base of the vpage.
    munmap(ptr, size);
}

//
// System Functions (4ed_system.h)
//

//
// Files
//

internal
Sys_Set_File_List_Sig(system_set_file_list){
    DIR *d;
    struct dirent *entry;
    char *fname, *cursor, *cursor_start;
    File_Info *info_ptr;
    i32 character_count, file_count, size, required_size, length;
    b32 clear_list = false;
    
    if(directory == 0){
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
        return;
    }
    
    LINUX_FN_DEBUG("%s", directory);
    
    d = opendir(directory);
    if (d){
        if (canon_directory_out != 0){
            u32 length = copy_fast_unsafe_cc(canon_directory_out, directory);
            if (canon_directory_out[length-1] != '/'){
                canon_directory_out[length++] = '/';
            }
            canon_directory_out[length] = 0;
            *canon_directory_size_out = length;
        }
        
        character_count = 0;
        file_count = 0;
        for (entry = readdir(d);
             entry != 0;
             entry = readdir(d)){
            fname = entry->d_name;
            if (match_cc(fname, ".") || match_cc(fname, "..")){
                continue;
            }
            ++file_count;            
            for (size = 0; fname[size]; ++size);
            character_count += size + 1;
        }
        
        required_size = character_count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_memory_free(file_list->block, file_list->block_size);
            file_list->block = system_memory_allocate(required_size);
            file_list->block_size = required_size;
        }
        
        file_list->infos = (File_Info*)file_list->block;
        cursor = (char*)(file_list->infos + file_count);
        
        if (file_list->block != 0){
            rewinddir(d);
            info_ptr = file_list->infos;
            for (entry = readdir(d);
                 entry != 0;
                 entry = readdir(d)){
                fname = entry->d_name;
                if (match_cc(fname, ".") || match_cc(fname, "..")){
                    continue;
                }
                cursor_start = cursor;
                length = copy_fast_unsafe_cc(cursor_start, fname);
                cursor += length;
                
                if(entry->d_type == DT_LNK){
                    struct stat st;
                    if(stat(entry->d_name, &st) != -1){
                        info_ptr->folder = S_ISDIR(st.st_mode);
                    } else {
                        info_ptr->folder = 0;
                    }
                } else {
                    info_ptr->folder = (entry->d_type == DT_DIR);
                }
                
                info_ptr->filename = cursor_start;
                info_ptr->filename_len = length;
                *cursor++ = 0;
                ++info_ptr;
            }
        }
        
        file_list->count = file_count;
        
        closedir(d);
    } else {
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
    }
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    char* path = (char*) alloca(len + 1);
    char* write_p = path;
    const char* read_p = filename;
    
    // return 0 for relative paths (e.g. cmdline args)
    if(len > 0 && filename[0] != '/'){
        return 0;
    }
    
    if (max == 0){
        return 0;
    }
    
    max -= 1;
    
    while(read_p < filename + len){
        if(read_p == filename || read_p[0] == '/'){
            if(read_p[1] == '/'){
                ++read_p;
            } else if(read_p[1] == '.'){
                if(read_p[2] == '/' || !read_p[2]){
                    read_p += 2;
                } else if(read_p[2] == '.' && (read_p[3] == '/' || !read_p[3])){
                    while(write_p > path && *--write_p != '/');
                    read_p += 3;
                } else {
                    *write_p++ = *read_p++;
                }
            } else {
                *write_p++ = *read_p++;
            }
        } else {
            *write_p++ = *read_p++;
        }
    }
    if(write_p == path) *write_p++ = '/';
    
    if(max >= (write_p - path)){
        memcpy(buffer, path, write_p - path);
    } else {
        write_p = path;
    }
    
#if FRED_INTERNAL
    if(len != (write_p - path) || memcmp(filename, path, len) != 0){
        LINUX_FN_DEBUG("[%.*s] -> [%.*s]", len, filename, (int)(write_p - path), path);
    }
#endif
    
    u32 length = (i32)(write_p - path);
    buffer[length] = 0;
    return(length);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = 0;
    
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        perror("open");
    } else {
        *(int*)handle_out = fd;
        result = 1;
    }
    
    return result;
}

internal
Sys_Load_Size_Sig(system_load_size){
    u32 result = 0;
    
    int fd = *(int*)&handle;
    struct stat st;
    
    if(fstat(fd, &st) == -1){
        perror("fstat");
    } else {
        result = st.st_size;
    }
    
    return result;
}

internal
Sys_Load_File_Sig(system_load_file){
    
    int fd = *(int*)&handle;
    do {
        ssize_t n = read(fd, buffer, size);
        if(n == -1){
            if(errno != EINTR){
                perror("read");
                break;
            }
        } else {
            size -= n;
            buffer += n;
        }
    } while(size);
    
    return size == 0;
}

internal
Sys_Load_Close_Sig(system_load_close){
    b32 result = 1;
    
    int fd = *(int*)&handle;
    if(close(fd) == -1){
        perror("close");
        result = 0;
    }
    
    return result;
}

internal
Sys_Save_File_Sig(system_save_file){
    b32 result = 0;
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 00640);
    
    LINUX_FN_DEBUG("%s %d", filename, size);
    
    if(fd < 0){
        fprintf(stderr, "system_save_file: open '%s': %s\n", filename, strerror(errno));
    } else {
        do {
            ssize_t written = write(fd, buffer, size);
            if(written == -1){
                if(errno != EINTR){
                    perror("system_save_file: write");
                    break;
                }
            } else {
                size -= written;
                buffer += written;
            }
        } while(size);
        
        close(fd);
    }
    
    return (size == 0);
}

//
// Time
//

internal
Sys_Now_Time_Sig(system_now_time){
    struct timespec spec;
    u64 result;
    
    clock_gettime(CLOCK_REALTIME, &spec);
    result = (spec.tv_sec * UINT64_C(1000000)) + (spec.tv_nsec / UINT64_C(1000));
    
    return(result);
}

internal
Sys_File_Exists_Sig(system_file_exists){
    int result = 0;
    char buff[PATH_MAX] = {};
    
    if(len + 1 > PATH_MAX){
        fputs("system_directory_has_file: path too long\n", stderr);
    } else {
        memcpy(buff, filename, len);
        buff[len] = 0;
        struct stat st;
        result = stat(buff, &st) == 0 && S_ISREG(st.st_mode);
    }
    
    LINUX_FN_DEBUG("%s: %d", buff, result);
    
    return(result);
}

internal
Sys_Directory_CD_Sig(system_directory_cd){
    String directory = make_string_cap(dir, *len, cap);
    b32 result = 0;
    i32 old_size;
    
    if (rel_path[0] != 0){
        if (rel_path[0] == '.' && rel_path[1] == 0){
            result = 1;
        }
        else if (rel_path[0] == '.' && rel_path[1] == '.' && rel_path[2] == 0){
            result = remove_last_folder(&directory);
            terminate_with_null(&directory);
        }
        else{
            if (directory.size + rel_len + 1 > directory.memory_size){
                old_size = directory.size;
                append_partial_sc(&directory, rel_path);
                append_s_char(&directory, '/');
                terminate_with_null(&directory);
                
                struct stat st;
                if (stat(directory.str, &st) == 0 && S_ISDIR(st.st_mode)){
                    result = 1;
                }
                else{
                    directory.size = old_size;
                }
            }
        }
    }
    
    *len = directory.size;
    
    LINUX_FN_DEBUG("%.*s: %d", directory.size, directory.str, result);
    
    return(result);
}

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    String str = make_string_cap(out, 0, capacity);
    return(system_get_binary_path(&str));
}

internal
Sys_Show_Mouse_Cursor_Sig(system_show_mouse_cursor){
    linuxvars.hide_cursor = !show;
    XDefineCursor(linuxvars.XDisplay, linuxvars.XWindow, show ? None : linuxvars.hidden_cursor);
}

internal
Sys_Toggle_Fullscreen_Sig(system_toggle_fullscreen){
    b32 success = true;
    LinuxToggleFullscreen(linuxvars.XDisplay, linuxvars.XWindow);
    return(success);
}

internal
Sys_Is_Fullscreen_Sig(system_is_fullscreen){
    b32 result = false;
    
    Atom type, *prop;
    unsigned long nitems, pad;
    int fmt;
    
    int ret = XGetWindowProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_STATE, 0, 32, False, XA_ATOM, &type, &fmt, &nitems, &pad, (unsigned char**)&prop);
    
    if (ret == Success && prop){
        result = (*prop == linuxvars.atom__NET_WM_STATE_FULLSCREEN);
        XFree((unsigned char*)prop);
    }
    
    return result;
}

internal
Sys_Send_Exit_Signal_Sig(system_send_exit_signal){
    linuxvars.keep_running = 0;
}

//
// Clipboard
//

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
    LinuxStringDup(&linuxvars.clipboard_outgoing, str.str, str.size);
    XSetSelectionOwner(linuxvars.XDisplay, linuxvars.atom_CLIPBOARD, linuxvars.XWindow, CurrentTime);
}

//
// Coroutine
//

internal Linux_Coroutine*
LinuxAllocCoroutine(){
    Linux_Coroutine *result = linuxvars.coroutine_free;
    Assert(result != 0);
    if(getcontext(&result->ctx) == -1){
        perror("getcontext");
    }
    result->ctx.uc_stack = result->stack;
    linuxvars.coroutine_free = result->next;
    return(result);
}

internal void
LinuxFreeCoroutine(Linux_Coroutine *data){
    data->next = linuxvars.coroutine_free;
    linuxvars.coroutine_free = data;
}

internal void
LinuxCoroutineMain(void *arg_){
    Linux_Coroutine *c = (Linux_Coroutine*)arg_;
    c->coroutine.func(&c->coroutine);
    c->done = 1;
    LinuxFreeCoroutine(c);
    setcontext((ucontext_t*)c->coroutine.yield_handle);
}

internal
Sys_Create_Coroutine_Sig(system_create_coroutine){
    Linux_Coroutine *c = LinuxAllocCoroutine();
    c->done = 0;
    
    makecontext(&c->ctx, (void (*)())LinuxCoroutineMain, 1, &c->coroutine);
    
    *(ucontext_t**)&c->coroutine.plat_handle = &c->ctx;
    c->coroutine.func = func;
    
    return(&c->coroutine);
}

internal
Sys_Launch_Coroutine_Sig(system_launch_coroutine){
    Linux_Coroutine *c = (Linux_Coroutine*)coroutine;
    ucontext_t* ctx = *(ucontext**)&coroutine->plat_handle;
    
    coroutine->yield_handle = &c->yield_ctx;
    coroutine->in = in;
    coroutine->out = out;
    
    swapcontext(&c->yield_ctx, ctx); 
    
    if (c->done){
        LinuxFreeCoroutine(c);
        coroutine = 0;
    }
    
    return(coroutine);
}

internal
Sys_Resume_Coroutine_Sig(system_resume_coroutine){
    Linux_Coroutine *c = (Linux_Coroutine*)coroutine;
    void *fiber;
    
    Assert(!c->done);
    
    coroutine->yield_handle = &c->yield_ctx;
    coroutine->in = in;
    coroutine->out = out;
    
    ucontext *ctx = *(ucontext**)&coroutine->plat_handle;
    
    swapcontext(&c->yield_ctx, ctx);
    
    if (c->done){
        LinuxFreeCoroutine(c);
        coroutine = 0;
    }
    
    return(coroutine);
}

internal
Sys_Yield_Coroutine_Sig(system_yield_coroutine){
    swapcontext(*(ucontext_t**)&coroutine->plat_handle, (ucontext*)coroutine->yield_handle);
}

//
// CLI
//

internal
Sys_CLI_Call_Sig(system_cli_call){
    LINUX_FN_DEBUG("%s %s", path, script_name);
    
    int pipe_fds[2];
    if(pipe(pipe_fds) == -1){
        perror("system_cli_call: pipe");
        return 0;
    }
    
    pid_t child_pid = fork();
    if(child_pid == -1){
        perror("system_cli_call: fork");
        return 0;
    }
    
    enum { PIPE_FD_READ, PIPE_FD_WRITE };
    
    // child
    if(child_pid == 0){
        close(pipe_fds[PIPE_FD_READ]);
        dup2(pipe_fds[PIPE_FD_WRITE], STDOUT_FILENO);
        dup2(pipe_fds[PIPE_FD_WRITE], STDERR_FILENO);
        
        if(chdir(path) == -1){
            perror("system_cli_call: chdir");
            exit(1);
        };
        
        char* argv[] = { "sh", "-c", script_name, NULL };
        
        if(execv("/bin/sh", argv) == -1){
            perror("system_cli_call: execv");
        }
        exit(1);
    } else {
        close(pipe_fds[PIPE_FD_WRITE]);
        
        *(pid_t*)&cli_out->proc = child_pid;
        *(int*)&cli_out->out_read = pipe_fds[PIPE_FD_READ];
        *(int*)&cli_out->out_write = pipe_fds[PIPE_FD_WRITE];
        
        struct epoll_event e = {};
        e.events = EPOLLIN | EPOLLET;
        e.data.u64 = LINUX_4ED_EVENT_CLI;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, pipe_fds[PIPE_FD_READ], &e);
    }
    
    return 1;
}

internal
Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    // NOTE(inso): I don't think anything needs to be done here.
}

internal
Sys_CLI_Update_Step_Sig(system_cli_update_step){
    
    int pipe_read_fd = *(int*)&cli->out_read;
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(pipe_read_fd, &fds);
    
    struct timeval tv = {};
    
    size_t space_left = max;
    char* ptr = dest;
    
    while(space_left > 0 && select(pipe_read_fd + 1, &fds, NULL, NULL, &tv) == 1){
        ssize_t num = read(pipe_read_fd, ptr, space_left);
        if(num == -1){
            perror("system_cli_update_step: read");
        } else if(num == 0){
            // NOTE(inso): EOF
            break;
        } else {
            ptr += num;
            space_left -= num;
        }
    }
    
    *amount = (ptr - dest);
    return (ptr - dest) > 0;
}

internal
Sys_CLI_End_Update_Sig(system_cli_end_update){
    pid_t pid = *(pid_t*)&cli->proc;
    b32 close_me = 0;
    
    int status;
    if(pid && waitpid(pid, &status, WNOHANG) > 0){
        close_me = 1;
        
        cli->exit = WEXITSTATUS(status);
        
        struct epoll_event e = {};
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_DEL, *(int*)&cli->out_read, &e);
        
        close(*(int*)&cli->out_read);
        close(*(int*)&cli->out_write);
    }
    
    return close_me;
}

//
// Threads
//

internal
Sys_Acquire_Lock_Sig(system_acquire_lock){
    pthread_mutex_lock(linuxvars.locks + id);
}

internal
Sys_Release_Lock_Sig(system_release_lock){
    pthread_mutex_unlock(linuxvars.locks + id);
}

internal void
system_wait_cv(i32 lock_id, i32 cv_id){
    pthread_cond_wait(linuxvars.conds + cv_id, linuxvars.locks + lock_id);
}

internal void
system_signal_cv(i32 lock_id, i32 cv_id){
    pthread_cond_signal(linuxvars.conds + cv_id);
}

internal void*
JobThreadProc(void* lpParameter){
    Thread_Context *thread = (Thread_Context*)lpParameter;
    Work_Queue *queue = linuxvars.queues + thread->group_id;
    Thread_Group *group = linuxvars.groups + thread->group_id;
    
    i32 thread_index = thread->id - 1;
    
    i32 cancel_lock = group->cancel_lock0 + thread_index;
    i32 cancel_cv = group->cancel_cv0 + thread_index;
    
    Thread_Memory *thread_memory = linuxvars.thread_memory + thread_index;
    
    if (thread_memory->size == 0){
        i32 new_size = KB(64);
        thread_memory->data = LinuxGetMemory(new_size);
        thread_memory->size = new_size;
    }
    
    for (;;){
        u32 read_index = queue->read_position;
        u32 write_index = queue->write_position;
        
        if (read_index != write_index){
            // NOTE(allen): Previously I was wrapping by the job wrap then
            // wrapping by the queue wrap.  That was super stupid what was that?
            // Now it just wraps by the queue wrap.
            u32 next_read_index = (read_index + 1) % QUEUE_WRAP;
            u32 safe_read_index =
                InterlockedCompareExchange(&queue->read_position,
                                           next_read_index, read_index);
            
            if (safe_read_index == read_index){
                Full_Job_Data *full_job = queue->jobs + safe_read_index;
                // NOTE(allen): This is interlocked so that it plays nice
                // with the cancel job routine, which may try to cancel this job
                // at the same time that we try to run it
                
                i32 safe_running_thread =
                    InterlockedCompareExchange(&full_job->running_thread,
                                               thread->id, THREAD_NOT_ASSIGNED);
                
                if (safe_running_thread == THREAD_NOT_ASSIGNED){
                    thread->job_id = full_job->id;
                    thread->running = 1;
                    
                    full_job->job.callback(&linuxvars.system,
                                           thread, thread_memory, full_job->job.data);
                    LinuxScheduleStep();
                    //full_job->running_thread = 0;
                    thread->running = 0;
                    
                    system_acquire_lock(cancel_lock);
                    if (thread->cancel){
                        thread->cancel = 0;
                        system_signal_cv(cancel_lock, cancel_cv);
                    }
                    system_release_lock(cancel_lock);
                }
            }
        }
        else{
            sem_wait(LinuxHandleToSem(queue->semaphore));
        }
    }
}

internal void
initialize_unbounded_queue(Unbounded_Work_Queue *source_queue){
    i32 max = 512;
    source_queue->jobs = (Full_Job_Data*)system_memory_allocate(max*sizeof(Full_Job_Data));
    source_queue->count = 0;
    source_queue->max = max;
    source_queue->skip = 0;
}

inline i32
get_work_queue_available_space(i32 write, i32 read){
    // NOTE(allen): The only time that queue->write_position == queue->read_position
    // is allowed is when the queue is empty.  Thus if
    // queue->write_position+1 == queue->read_position the available space is zero.
    // So these computations both end up leaving one slot unused. The only way I can
    // think to easily eliminate this is to have read and write wrap at twice the size
    // of the underlying array but modulo their values into the array then if write
    // has caught up with read it still will not be equal... but lots of modulos... ehh.
    
    i32 available_space = 0;
    if (write >= read){
        available_space = QUEUE_WRAP - (write - read) - 1;
    }
    else{
        available_space = (read - write) - 1;
    }
    
    return(available_space);
}

#define UNBOUNDED_SKIP_MAX 128

internal void
flush_to_direct_queue(Unbounded_Work_Queue *source_queue, Work_Queue *queue, i32 thread_count){
    // NOTE(allen): It is understood that read_position may be changed by other
    // threads but it will only make more space in the queue if it is changed.
    // Meanwhile write_position should not ever be changed by anything but the
    // main thread in this system, so it will not be interlocked.
    u32 read_position = queue->read_position;
    u32 write_position = queue->write_position;
    u32 available_space = get_work_queue_available_space(write_position, read_position);
    u32 available_jobs = source_queue->count - source_queue->skip;
    
    u32 writable_count = Min(available_space, available_jobs);
    
    if (writable_count > 0){
        u32 count1 = writable_count;
        
        if (count1+write_position > QUEUE_WRAP){
            count1 = QUEUE_WRAP - write_position;
        }
        
        u32 count2 = writable_count - count1;
        
        Full_Job_Data *job_src1 = source_queue->jobs + source_queue->skip;
        Full_Job_Data *job_src2 = job_src1 + count1;
        
        Full_Job_Data *job_dst1 = queue->jobs + write_position;
        Full_Job_Data *job_dst2 = queue->jobs;
        
        Assert((job_src1->id % QUEUE_WRAP) == write_position);
        
        memcpy(job_dst1, job_src1, sizeof(Full_Job_Data)*count1);
        memcpy(job_dst2, job_src2, sizeof(Full_Job_Data)*count2);
        queue->write_position = (write_position + writable_count) % QUEUE_WRAP;
        
        source_queue->skip += writable_count;
        
        if (source_queue->skip == source_queue->count){
            source_queue->skip = source_queue->count = 0;
        }
        else if (source_queue->skip > UNBOUNDED_SKIP_MAX){
            u32 left_over = source_queue->count - source_queue->skip;
            memmove(source_queue->jobs, source_queue->jobs + source_queue->skip,
                    sizeof(Full_Job_Data)*left_over);
            source_queue->count = left_over;
            source_queue->skip = 0;
        }
    }
    
    i32 semaphore_release_count = writable_count;
    if (semaphore_release_count > thread_count){
        semaphore_release_count = thread_count;
    }
    
    // NOTE(allen): platform dependent portion...
    for (i32 i = 0; i < semaphore_release_count; ++i){
        sem_post(LinuxHandleToSem(queue->semaphore));
    }
}

internal void
flush_thread_group(i32 group_id){
    Thread_Group *group = linuxvars.groups + group_id;
    Work_Queue *queue = linuxvars.queues + group_id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    flush_to_direct_queue(source_queue, queue, group->count);
}

// Note(allen): post_job puts the job on the unbounded queue.
// The unbounded queue is entirely managed by the main thread.
// The thread safe queue is bounded in size so the unbounded
// queue is periodically flushed into the direct work queue.
internal
Sys_Post_Job_Sig(system_post_job){
    Thread_Group *group = linuxvars.groups + group_id;
    Unbounded_Work_Queue *queue = &group->queue;
    
    u32 result = queue->next_job_id++;
    
    while (queue->count >= queue->max){
        i32 new_max = queue->max*2;
        u32 job_size = sizeof(Full_Job_Data);
        Full_Job_Data *new_jobs = (Full_Job_Data*)system_memory_allocate(new_max*job_size);
        
        memcpy(new_jobs, queue->jobs, queue->count);
        
        system_memory_free(queue->jobs, queue->max*job_size);
        
        queue->jobs = new_jobs;
        queue->max = new_max;
    }
    
    Full_Job_Data full_job;
    
    full_job.job = job;
    full_job.running_thread = THREAD_NOT_ASSIGNED;
    full_job.id = result;
    
    queue->jobs[queue->count++] = full_job;
    
    Work_Queue *direct_queue = linuxvars.queues + group_id;
    flush_to_direct_queue(queue, direct_queue, group->count);
    
    return(result);
}

internal
Sys_Cancel_Job_Sig(system_cancel_job){
    Thread_Group *group = linuxvars.groups + group_id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    
    b32 handled_in_unbounded = false;
    if (source_queue->skip < source_queue->count){
        Full_Job_Data *first_job = source_queue->jobs + source_queue->skip;
        if (first_job->id <= job_id){
            u32 index = source_queue->skip + (job_id - first_job->id);
            Full_Job_Data *job = source_queue->jobs + index;
            job->running_thread = 0;
            handled_in_unbounded = true;
        }
    }
    
    if (!handled_in_unbounded){
        Work_Queue *queue = linuxvars.queues + group_id;
        Full_Job_Data *job = queue->jobs + (job_id % QUEUE_WRAP);
        Assert(job->id == job_id);
        
        u32 thread_id =
            InterlockedCompareExchange(&job->running_thread,
                                       0, THREAD_NOT_ASSIGNED);
        
        if (thread_id != THREAD_NOT_ASSIGNED && thread_id != 0){
            i32 thread_index = thread_id - 1;
            
            i32 cancel_lock = group->cancel_lock0 + thread_index;
            i32 cancel_cv = group->cancel_cv0 + thread_index;
            Thread_Context *thread = group->threads + thread_index;
            
            
            system_acquire_lock(cancel_lock);
            
            thread->cancel = 1;
            
            system_release_lock(FRAME_LOCK);
            do{
                system_wait_cv(cancel_lock, cancel_cv);
            }while (thread->cancel == 1);
            system_acquire_lock(FRAME_LOCK);
            
            system_release_lock(cancel_lock);
        }
    }
}

internal
Sys_Check_Cancel_Sig(system_check_cancel){
    b32 result = 0;
    
    Thread_Group *group = linuxvars.groups + thread->group_id;
    i32 thread_index = thread->id - 1;
    i32 cancel_lock = group->cancel_lock0 + thread_index;
    
    system_acquire_lock(cancel_lock);
    if (thread->cancel){
        result = 1;
    }
    system_release_lock(cancel_lock);
    
    return(result);
}

internal
Sys_Grow_Thread_Memory_Sig(system_grow_thread_memory){
    void *old_data;
    i32 old_size, new_size;
    
    system_acquire_lock(CANCEL_LOCK0 + memory->id - 1);
    old_data = memory->data;
    old_size = memory->size;
    new_size = l_round_up_i32(memory->size*2, KB(4));
    memory->data = system_memory_allocate(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        system_memory_free(old_data, old_size);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}

//
// Debug
//

#if FRED_INTERNAL

#ifdef OLD_JOB_QUEUE
internal
INTERNAL_Sys_Get_Thread_States_Sig(internal_get_thread_states){
    Work_Queue *queue = linuxvars.queues + id;
    u32 write = queue->write_position;
    u32 read = queue->read_position;
    if (write < read) write += QUEUE_WRAP;
    *pending = (i32)(write - read);
    
    Thread_Group *group = linuxvars.groups + id;
    for (i32 i = 0; i < group->count; ++i){
        running[i] = (group->threads[i].running != 0);
    }
}
#else
internal
INTERNAL_Sys_Get_Thread_States_Sig(internal_get_thread_states){
    Thread_Group *group = linuxvars.groups + id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    Work_Queue *queue = linuxvars.queues + id;
    u32 write = queue->write_position;
    u32 read = queue->read_position;
    if (write < read) write += QUEUE_WRAP;
    *pending = (i32)(write - read) + source_queue->count - source_queue->skip;
    
    for (i32 i = 0; i < group->count; ++i){
        running[i] = (group->threads[i].running != 0);
    }
}
#endif

#endif

//
// Linux rendering/font system functions
//

#include "4ed_font_data.h"
#include "4ed_system_shared.cpp"

//
// End of system funcs
//

//
// Linux init functions
//

internal b32
LinuxLoadAppCode(String* base_dir){
    b32 result = 0;
    App_Get_Functions *get_funcs = 0;
    
    if(!sysshared_to_binary_path(base_dir, "4ed_app.so")){
        return 0;
    }
    
    linuxvars.app_code = dlopen(base_dir->str, RTLD_LAZY);
    if (linuxvars.app_code){
        get_funcs = (App_Get_Functions*)
            dlsym(linuxvars.app_code, "app_get_functions");
    } else {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
    }
    
    if (get_funcs){
        result = 1;
        linuxvars.app = get_funcs();
    }
    
    return(result);
}

internal void
LinuxLoadSystemCode(){
    // files
    linuxvars.system.set_file_list = system_set_file_list;
    linuxvars.system.get_canonical = system_get_canonical;
    linuxvars.system.add_listener = system_add_listener;
    linuxvars.system.remove_listener = system_remove_listener;
    linuxvars.system.get_file_change = system_get_file_change;
    linuxvars.system.load_handle = system_load_handle;
    linuxvars.system.load_size = system_load_size;
    linuxvars.system.load_file = system_load_file;
    linuxvars.system.load_close = system_load_close;
    linuxvars.system.save_file = system_save_file;
    
    // time
    linuxvars.system.now_time = system_now_time;
    
    // custom.h
    linuxvars.system.memory_allocate = system_memory_allocate;
    linuxvars.system.memory_set_protection = system_memory_set_protection;
    linuxvars.system.memory_free = system_memory_free;
    linuxvars.system.file_exists = system_file_exists;
    linuxvars.system.directory_cd = system_directory_cd;
    linuxvars.system.get_4ed_path = system_get_4ed_path;
    linuxvars.system.show_mouse_cursor = system_show_mouse_cursor;
    linuxvars.system.toggle_fullscreen = system_toggle_fullscreen;
    linuxvars.system.is_fullscreen = system_is_fullscreen;
    linuxvars.system.send_exit_signal = system_send_exit_signal;
    
    // clipboard
    linuxvars.system.post_clipboard = system_post_clipboard;
    
    // coroutine
    linuxvars.system.create_coroutine = system_create_coroutine;
    linuxvars.system.launch_coroutine = system_launch_coroutine;
    linuxvars.system.resume_coroutine = system_resume_coroutine;
    linuxvars.system.yield_coroutine = system_yield_coroutine;
    
    // cli
    linuxvars.system.cli_call = system_cli_call;
    linuxvars.system.cli_begin_update = system_cli_begin_update;
    linuxvars.system.cli_update_step = system_cli_update_step;
    linuxvars.system.cli_end_update = system_cli_end_update;
    
    // threads
    linuxvars.system.post_job = system_post_job;
    linuxvars.system.cancel_job = system_cancel_job;
    linuxvars.system.check_cancel = system_check_cancel;
    linuxvars.system.grow_thread_memory = system_grow_thread_memory;
    linuxvars.system.acquire_lock = system_acquire_lock;
    linuxvars.system.release_lock = system_release_lock;
    
    // debug
#if FRED_INTERNAL
    linuxvars.system.internal_get_thread_states = internal_get_thread_states;
#endif
}

internal void
LinuxLoadRenderCode(){
    linuxvars.target.push_clip = draw_push_clip;
    linuxvars.target.pop_clip = draw_pop_clip;
    linuxvars.target.push_piece = draw_push_piece;
}

//
// Renderer
//

internal void
LinuxRedrawTarget(){
    launch_rendering(&linuxvars.system, &linuxvars.target);
    //glFlush();
    glXSwapBuffers(linuxvars.XDisplay, linuxvars.XWindow);
}

internal void
LinuxResizeTarget(i32 width, i32 height){
    if (width > 0 && height > 0){
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glScissor(0, 0, width, height);
        
        linuxvars.target.width = width;
        linuxvars.target.height = height;
    }
}

//
// OpenGL init
//

// NOTE(allen): Thanks to Casey for providing the linux OpenGL launcher.
static bool ctxErrorOccurred = false;

internal int
ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

#if FRED_INTERNAL

static void LinuxGLDebugCallback(
GLenum source,
GLenum type,
GLuint id,
GLenum severity,
GLsizei length,
const GLchar* message,
const void* userParam
){
    fprintf(stderr, "GL DEBUG: %s\n", message);
}

#endif

internal GLXContext
InitializeOpenGLContext(Display *XDisplay, Window XWindow, GLXFBConfig &bestFbc, b32 &IsLegacy)
{
    IsLegacy = false;
    
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    
    typedef PFNGLXSWAPINTERVALEXTPROC     glXSwapIntervalEXTProc;
    typedef PFNGLXSWAPINTERVALMESAPROC    glXSwapIntervalMESAProc;
    typedef PFNGLXGETSWAPINTERVALMESAPROC glXGetSwapIntervalMESAProc;
    typedef PFNGLXSWAPINTERVALSGIPROC     glXSwapIntervalSGIProc;
    
    const char *glxExts = glXQueryExtensionsString(XDisplay, DefaultScreen(XDisplay));
    
#define GLXLOAD(x) x ## Proc x = (x ## Proc) glXGetProcAddressARB( (const GLubyte*) #x);
    
    GLXLOAD(glXCreateContextAttribsARB);
    
    GLXContext ctx = 0;
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);
    
    if (!glXCreateContextAttribsARB)
    {
        fprintf(stderr,  "glXCreateContextAttribsARB() not found, using old-style GLX context\n" );
        ctx = glXCreateNewContext( XDisplay, bestFbc, GLX_RGBA_TYPE, 0, True );
    } 
    else
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB , GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#if FRED_INTERNAL
            GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
            None
        };
        
        fprintf(stderr,  "Creating GL 4.3 context...\n");
        ctx = glXCreateContextAttribsARB(XDisplay, bestFbc, 0, True, context_attribs);
        
        XSync( XDisplay, False );
        if (!ctxErrorOccurred && ctx)
        {
            fprintf(stderr,  "Created GL 4.3 context.\n" );
        }
        else
        {
            ctxErrorOccurred = false;
            
            context_attribs[1] = 3;
            context_attribs[3] = 2;
            
            fprintf(stderr,  "GL 4.3 unavailable, creating GL 3.2 context...\n" );
            ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0, True, context_attribs );
            
            XSync(XDisplay, False);
            
            if (!ctxErrorOccurred && ctx)
            {
                fprintf(stderr,  "Created GL 3.2 context.\n" );
            }
            else
            {
                context_attribs[1] = 1;
                context_attribs[3] = 2;
                
                ctxErrorOccurred = false;
                
                fprintf(stderr, "Failed to create GL 3.2 context, using old-style GLX context\n");
                ctx = glXCreateContextAttribsARB(XDisplay, bestFbc, 0, True, context_attribs);
                
                IsLegacy = true;
            }
        }
    }
    
    XSync(XDisplay, False);
    XSetErrorHandler(oldHandler);
    
    if (ctxErrorOccurred || !ctx)
    {
        fprintf(stderr,  "Failed to create an OpenGL context\n");
        exit(1);
    }
    
    b32 Direct;
    if (!glXIsDirect(XDisplay, ctx))
    {
        fprintf(stderr,  "Indirect GLX rendering context obtained\n");
        Direct = 0;
    }
    else
    {
        fprintf(stderr,  "Direct GLX rendering context obtained\n");
        Direct = 1;
    }
    
    fprintf(stderr,  "Making context current\n");
    glXMakeCurrent( XDisplay, XWindow, ctx );
    
    char *Vendor   = (char *)glGetString(GL_VENDOR);
    char *Renderer = (char *)glGetString(GL_RENDERER);
    char *Version  = (char *)glGetString(GL_VERSION);
    
    //TODO(inso): glGetStringi is required in core profile if the GL version is >= 3.0
    char *Extensions = (char *)glGetString(GL_EXTENSIONS);
    
    fprintf(stderr, "GL_VENDOR: %s\n", Vendor);
    fprintf(stderr, "GL_RENDERER: %s\n", Renderer);
    fprintf(stderr, "GL_VERSION: %s\n", Version);
    //  fprintf(stderr, "GL_EXTENSIONS: %s\n", Extensions);
    
    //NOTE(inso): enable vsync if available. this should probably be optional
    if(Direct && strstr(glxExts, "GLX_EXT_swap_control ")){
        
        GLXLOAD(glXSwapIntervalEXT);
        
        if(glXSwapIntervalEXT){
            glXSwapIntervalEXT(XDisplay, XWindow, 1);
            
            unsigned int swap_val = 0;
            glXQueryDrawable(XDisplay, XWindow, GLX_SWAP_INTERVAL_EXT, &swap_val);
            
            linuxvars.vsync = swap_val == 1;
            fprintf(stderr, "VSync enabled? %s.\n", linuxvars.vsync ? "Yes" : "No");
        }
        
    } else if(Direct && strstr(glxExts, "GLX_MESA_swap_control ")){
        
        GLXLOAD(glXSwapIntervalMESA);
        GLXLOAD(glXGetSwapIntervalMESA);
        
        if(glXSwapIntervalMESA){
            glXSwapIntervalMESA(1);
            
            if(glXGetSwapIntervalMESA){
                linuxvars.vsync = glXGetSwapIntervalMESA();
                fprintf(stderr, "VSync enabled? %s (MESA)\n", linuxvars.vsync ? "Yes" : "No");
            } else {
                // NOTE(inso): assume it worked?
                linuxvars.vsync = 1;
                fputs("VSync enabled? possibly (MESA)\n", stderr);
            }
        }
        
    } else if(Direct && strstr(glxExts, "GLX_SGI_swap_control ")){
        
        GLXLOAD(glXSwapIntervalSGI);
        
        if(glXSwapIntervalSGI){
            glXSwapIntervalSGI(1);
            
            //NOTE(inso): The SGI one doesn't seem to have a way to confirm we got it...
            linuxvars.vsync = 1;
            fputs("VSync enabled? hopefully (SGI)\n", stderr);
        }
        
    } else {
        fputs("VSync enabled? nope, no suitable extension\n", stderr);
    }
    
#if FRED_INTERNAL
    typedef PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackProc;
    
    GLXLOAD(glDebugMessageCallback);
    
    if(glDebugMessageCallback){
        fputs("Enabling GL Debug Callback\n", stderr);
        glDebugMessageCallback(&LinuxGLDebugCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);
    }
#endif
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
#undef GLXLOAD
    
    return(ctx);
}

internal b32
GLXCanUseFBConfig(Display *XDisplay)
{
    b32 Result = false;
    
    int GLXMajor, GLXMinor;
    
    char *XVendor = ServerVendor(XDisplay);
    fprintf(stderr, "XWindows vendor: %s\n", XVendor);
    if(glXQueryVersion(XDisplay, &GLXMajor, &GLXMinor))
    {
        fprintf(stderr, "GLX version %d.%d\n", GLXMajor, GLXMinor);
        if(((GLXMajor == 1 ) && (GLXMinor >= 3)) || (GLXMajor > 1))
        {
            Result = true;
        }
    }
    
    return(Result);
}

typedef struct glx_config_result{
    b32 Found;
    GLXFBConfig BestConfig;
    XVisualInfo BestInfo;
} glx_config_result;

internal glx_config_result
ChooseGLXConfig(Display *XDisplay, int XScreenIndex)
{
    glx_config_result Result = {0};
    
    int DesiredAttributes[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };    
    
    int ConfigCount = 0;
    GLXFBConfig *Configs = glXChooseFBConfig(XDisplay,
                                             XScreenIndex,
                                             DesiredAttributes,
                                             &ConfigCount);
    if(Configs && ConfigCount > 0)
    {
        XVisualInfo* VI = glXGetVisualFromFBConfig(XDisplay, Configs[0]);
        if(VI)
        {
            Result.Found = true;
            Result.BestConfig = Configs[0];
            Result.BestInfo = *VI;
            
            int id = 0;
            glXGetFBConfigAttrib(XDisplay, Result.BestConfig, GLX_FBCONFIG_ID, &id);
            fprintf(stderr, "Using FBConfig: %d (0x%x)\n", id, id);
            
            XFree(VI);
        }
        
        XFree(Configs);
    }
    
    return(Result);
}

//
// X11 input / events init
//

struct Init_Input_Result{
    XIM input_method;
    XIMStyle best_style;
    XIC xic;
};
static Init_Input_Result null_init_input_result = {0};

internal Init_Input_Result
LinuxInputInit(Display *dpy, Window XWindow){
    Init_Input_Result result = {};
    XIMStyles *styles = 0;
    XIMStyle style;
    unsigned long xim_event_mask = 0;
    
    setlocale(LC_ALL, "");
    XSetLocaleModifiers("");
    fprintf(stderr, "Supported locale?: %s.\n", XSupportsLocale() ? "Yes" : "No");
    // TODO(inso): handle the case where it isn't supported somehow?
    
    result.input_method = XOpenIM(dpy, 0, 0, 0);
    if (!result.input_method){
        // NOTE(inso): Try falling back to the internal XIM implementation that
        // should in theory always exist.
        
        XSetLocaleModifiers("@im=none");
        result.input_method = XOpenIM(dpy, 0, 0, 0);
    }
    
    if (result.input_method){
        if (!XGetIMValues(result.input_method, XNQueryInputStyle, &styles, NULL) && styles){
            for (i32 i = 0; i < styles->count_styles; ++i){
                style = styles->supported_styles[i];
                if (style == (XIMPreeditNothing | XIMStatusNothing)){
                    result.best_style = style;
                    break;
                }
            }
        }
        
        if (result.best_style){
            XFree(styles);
            
            result.xic = XCreateIC(result.input_method, XNInputStyle, result.best_style, XNClientWindow, XWindow, XNFocusWindow, XWindow, NULL);
            
            if (XGetICValues(result.xic, XNFilterEvents, &xim_event_mask, NULL)){
                xim_event_mask = 0;
            }
        }
        else{
            result = null_init_input_result;
            fputs("Could not get minimum required input style.\n", stderr);
        }
    }
    else{
        result = null_init_input_result;
        fprintf(stderr, "Could not open X Input Method.\n");
    }
    
    u32 flags = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | FocusChangeMask | StructureNotifyMask | MappingNotify | ExposureMask | VisibilityChangeMask | xim_event_mask;
    
    XSelectInput(linuxvars.XDisplay, linuxvars.XWindow, flags);
    
    return(result);
}

//
// Keyboard handling funcs
//

global Key_Code keycode_lookup_table[255];

internal void
LinuxKeycodeInit(Display* dpy){
    
    // NOTE(inso): This looks a bit dumb, but it's the best way I can think of to do it, since:
    // KeySyms are the type representing "virtual" keys, like XK_BackSpace, but they are 32-bit ints.
    // KeyCodes are guaranteed to fit in 1 byte (and therefore the keycode_lookup_table) but
    // have dynamic numbers assigned by the XServer.
    //  There is XKeysymToKeycode, but it only returns 1 KeyCode for a KeySym. I have my capslock
    // rebound to esc, so there are two KeyCodes for the XK_Escape KeyCode but XKeysymToKeycode only
    // gets one of them, hence the need for this crazy lookup which works correctly with rebound keys.
    
    memset(keycode_lookup_table, 0, sizeof(keycode_lookup_table));
    
    struct SymMapping {
        KeySym sym;
        u16 code;
    } sym_table[] = {
        { XK_BackSpace, key_back },
        { XK_Delete, key_del },
        { XK_Up, key_up },
        { XK_Down, key_down },
        { XK_Left, key_left },
        { XK_Right, key_right },
        { XK_Insert, key_insert },
        { XK_Home, key_home },
        { XK_End, key_end },
        { XK_Page_Up, key_page_up },
        { XK_Page_Down, key_page_down },
        { XK_Escape, key_esc },
        { XK_F1, key_f1 },
        { XK_F2, key_f2 },
        { XK_F3, key_f3 },
        { XK_F4, key_f4 },
        { XK_F5, key_f5 },
        { XK_F6, key_f6 },
        { XK_F7, key_f7 },
        { XK_F8, key_f8 },
        { XK_F9, key_f9 },
        { XK_F10, key_f10 },
        { XK_F11, key_f11 },
        { XK_F12, key_f12 },
        { XK_F13, key_f13 },
        { XK_F14, key_f14 },
        { XK_F15, key_f15 },
        { XK_F16, key_f16 },
    };
    
    const int table_size = sizeof(sym_table) / sizeof(struct SymMapping);
    
    int key_min, key_max, syms_per_code;
    XDisplayKeycodes(dpy, &key_min, &key_max);
    
    int key_count = (key_max - key_min) + 1;
    
    KeySym* syms = XGetKeyboardMapping(
        dpy,
        key_min,
        key_count,
        &syms_per_code
        );
    
    if(!syms) return;
    
    int key = key_min;
    for(int i = 0; i < key_count * syms_per_code; ++i){
        for(int j = 0; j < table_size; ++j){
            if(sym_table[j].sym == syms[i]){
                keycode_lookup_table[key + (i/syms_per_code)] = sym_table[j].code;
                break;
            }
        }
    }
    
    XFree(syms);
    
}

internal void
LinuxPushKey(Key_Code code, Key_Code chr, Key_Code chr_nocaps, b8 (*mods)[MDFR_INDEX_COUNT])
{
    i32 *count = &linuxvars.input.keys.count;
    Key_Event_Data *data = linuxvars.input.keys.keys;
    
    if(*count < KEY_INPUT_BUFFER_SIZE){
        data[*count].keycode = code;
        data[*count].character = chr;
        data[*count].character_no_caps_lock = chr_nocaps;
        
        memcpy(data[*count].modifiers, *mods, sizeof(*mods));
        
        ++(*count);
    }
}

//
// Misc utility funcs
//

internal Plat_Handle
LinuxSemToHandle(sem_t* sem){
    return *(Plat_Handle*)&sem;
}

internal sem_t*
LinuxHandleToSem(Plat_Handle h){
    return *(sem_t**)&h;
}

internal Plat_Handle
LinuxFDToHandle(int fd){
    return *(Plat_Handle*)&fd;
}

internal int
LinuxHandleToFD(Plat_Handle h){
    return *(int*)&h;
}

internal void
LinuxStringDup(String* str, void* data, size_t size){
    if(str->memory_size < size){
        if(str->str){
            LinuxFreeMemory(str->str);
        }
        str->memory_size = size;
        str->str = (char*)LinuxGetMemory(size);
        //TODO(inso): handle alloc failure case
    }
    
    str->size = size;
    memcpy(str->str, data, size);
}

internal void
LinuxScheduleStep(void){
    u64 now  = system_now_time();
    u64 diff = (now - linuxvars.last_step);
    
    if (diff > (u64)frame_useconds){
        u64 ev = 1;
        ssize_t size = write(linuxvars.step_event_fd, &ev, sizeof(ev));
        (void)size;
    }
    else{
        struct itimerspec its = {};
        timerfd_gettime(linuxvars.step_timer_fd, &its);
        
        if (its.it_value.tv_sec == 0 && its.it_value.tv_nsec == 0){
            its.it_value.tv_nsec = (frame_useconds - diff) * 1000UL;
            timerfd_settime(linuxvars.step_timer_fd, 0, &its, NULL);
        }
    }
}

//
// X11 utility funcs
//

internal void
LinuxSetWMState(Display* d, Window w, Atom one, Atom two, int mode){
    //NOTE(inso): this will only work after it is mapped
    
    enum { STATE_REMOVE, STATE_ADD, STATE_TOGGLE };
    
    XEvent e = {};
    
    e.xany.type = ClientMessage;
    e.xclient.message_type = linuxvars.atom__NET_WM_STATE;
    e.xclient.format = 32;
    e.xclient.window = w;
    e.xclient.data.l[0] = mode;
    e.xclient.data.l[1] = one;
    e.xclient.data.l[2] = two;
    e.xclient.data.l[3] = 1L;
    
    XSendEvent(d, RootWindow(d, 0), 0, SubstructureNotifyMask | SubstructureRedirectMask, &e);
}

internal void
LinuxMaximizeWindow(Display* d, Window w, b32 maximize)
{
    LinuxSetWMState(d, w, linuxvars.atom__NET_WM_STATE_MAXIMIZED_HORZ, linuxvars.atom__NET_WM_STATE_MAXIMIZED_VERT, maximize != 0);
}

internal void
LinuxToggleFullscreen(Display* d, Window w)
{
    LinuxSetWMState(d, w, linuxvars.atom__NET_WM_STATE_FULLSCREEN, 0, 2);
}

#include "linux_icon.h"
internal void
LinuxSetIcon(Display* d, Window w)
{
    Atom WM_ICON = XInternAtom(d, "_NET_WM_ICON", False);
    XChangeProperty(d, w, WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)linux_icon, sizeof(linux_icon) / sizeof(long));
}

internal void
LinuxX11ConnectionWatch(Display* dpy, XPointer cdata, int fd, Bool opening, XPointer* wdata)
{
    struct epoll_event e = {};
    e.events = EPOLLIN | EPOLLET;
    e.data.u64 = LINUX_4ED_EVENT_X11_INTERNAL | fd;
    
    int op = opening ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
    epoll_ctl(linuxvars.epoll, op, fd, &e);
}

// NOTE(inso): this was a quick hack, might need some cleanup.
internal void
LinuxFatalErrorMsg(const char* msg)
{
    fprintf(stderr, "Fatal Error: %s\n", msg);
    
    Display *dpy = XOpenDisplay(0);
    if(!dpy){
        exit(1);
    }
    
    const int num_cols = 50;
    int win_w = (num_cols + 10) * 9;
    int win_h = 140;
    
    {
        const char *start_p = msg, *space_p = NULL;
        for(const char* p = msg; *p; ++p){
            if(*p == ' ') space_p = p;
            if(*p == '\n' || p - start_p > num_cols){
                win_h += 18;
                start_p = space_p ? space_p + 1 : p;
                space_p = NULL;
            }
        }
    }
    
    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, win_w, win_h, 0, 0, 0x227A3B);
    XStoreName(dpy, w, "4coder Error");
    
    XSizeHints* sh = XAllocSizeHints();
    sh->flags = PMinSize;
    sh->min_width = win_w;
    sh->min_height = win_h;
    XSetWMNormalHints(dpy, w, sh);
    
    Atom type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    
    XChangeProperty(dpy, w,
                    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False),
                    XA_ATOM,
                    32,
                    PropModeReplace,
                    (unsigned char*) &type,
                    1);
    
    Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &WM_DELETE_WINDOW, 1);
    
    LinuxSetIcon(dpy, w);
    
    XMapRaised(dpy, w);
    XSync(dpy, False);
    
    XSelectInput(dpy, w, ExposureMask | StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask);
    
    XFontStruct* font = XLoadQueryFont(dpy, "-*-fixed-bold-*-*-*-*-140-*-*-*-*-iso8859-1");
    if(!font){
        exit(1);
    }
    
    XGCValues gcv;
    gcv.foreground = WhitePixel(dpy, 0);
    gcv.line_width = 2;
    gcv.font = font->fid;
    
    GC gc1 = XCreateGC(dpy, w, GCForeground | GCFont | GCLineWidth, &gcv);
    gcv.foreground = BlackPixel(dpy, 0);
    GC gc2 = XCreateGC(dpy, w, GCForeground | GCFont | GCLineWidth, &gcv);
    
    int button_trigger = 0;
    int button_hi = 0;
    
    XEvent ev;
    while (1){
        XNextEvent(dpy, &ev);
        
        int redraw = 0;
        
        if(ev.type == Expose) redraw = 1;
        
        if(ev.type == ConfigureNotify){
            redraw = 1;
            win_w = ev.xconfigure.width;
            win_h = ev.xconfigure.height;
        }
        
        XRectangle button_rect = { win_w/2-40, win_h*0.8f, 80, 20 };
        
        if(ev.type == MotionNotify){
            int new_hi = (ev.xmotion.x > button_rect.x &&
                          ev.xmotion.y > button_rect.y &&
                          ev.xmotion.x < button_rect.x + button_rect.width &&
                          ev.xmotion.y < button_rect.y + button_rect.height);
            
            if(new_hi != button_hi){
                button_hi = new_hi;
                redraw = 1;
            }
        }
        
        if(ev.type == KeyPress){
            KeySym sym = XLookupKeysym(&ev.xkey, 0);
            if(sym == XK_Escape || sym == XK_Return){
                exit(1);
            }
        }
        
        if(ev.type == ButtonPress && ev.xbutton.button == Button1){
            if(button_hi) button_trigger = 1;
            redraw = 1;
        }
        
        if(ev.type == ButtonRelease && ev.xbutton.button == Button1){
            if(button_trigger){
                if(button_hi){
                    exit(1);
                } else {
                    button_trigger = 0;
                }
            }
            redraw = 1;
        }
        
        if(ev.type == ClientMessage && ev.xclient.window == w && (Atom)ev.xclient.data.l[0] == WM_DELETE_WINDOW){
            exit(1);
        }
        
#define DRAW_STR(x, y, str, len) \
        XDrawString(dpy, w, gc2, (x)+1, (y)+1, (str), (len)); \
        XDrawString(dpy, w, gc1, (x)  , (y)  , (str), (len))
        
        if(redraw){
            XClearWindow(dpy, w);
            
            const char* line_start = msg;
            const char* last_space = NULL;
            int y = 30;
            
            {
                const char title[] = "4coder - Fatal Error";
                int width = XTextWidth(font, title, sizeof(title)-1);
                int x = (win_w/2) - (width/2);
                DRAW_STR(x, y, title, sizeof(title)-1);
            }
            
            y += 36;
            int width = XTextWidth(font, "x", 1) * num_cols;
            int x = (win_w/2) - (width/2);
            
            for(const char* p = line_start; *p; ++p){
                if(*p == ' ') last_space = p;
                if(p - line_start > num_cols || *p == '\n' || !p[1]){
                    
                    const char* new_line_start = last_space + 1;
                    if(!last_space || *p == '\n' || !p[1]){
                        new_line_start = last_space = (p + !p[1]);
                    }
                    
                    DRAW_STR(x, y, line_start, last_space - line_start);
                    
                    line_start = new_line_start;
                    last_space = NULL;
                    y += 18;
                }
            }
            
            XDrawRectangles(dpy, w, gc1, &button_rect, 1);
            if(button_hi || button_trigger){
                XDrawRectangle(dpy, w, gc2, button_rect.x+1, button_rect.y+1, button_rect.width-2, button_rect.height-2);
            }
            
            DRAW_STR(button_rect.x + 20, button_rect.y + 15, "Drat!", 5);
        }
    }
#undef DRAW_STR
}

internal int
LinuxGetXSettingsDPI(Display* dpy, int screen)
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
    
    if(XSET_SEL == None || XSET_SET == None){
        fputs("XSETTINGS unavailable.\n", stderr);
        return dpi;
    }
    
    Window xset_win = XGetSelectionOwner(dpy, XSET_SEL);
    if(xset_win == None){
        // TODO(inso): listen for the ClientMessage about it becoming available?
        //             there's not much point atm if DPI scaling is only done at startup 
        goto out;
    }
    
    {
        Atom type;
        int fmt;
        unsigned long pad, num;
        
        if(XGetWindowProperty(dpy, xset_win, XSET_SET, 0, 1024, False, XSET_SET, &type, &fmt, &num, &pad, &prop) != Success){
            fputs("XSETTINGS: GetWindowProperty failed.\n", stderr);
            goto out;
        }
        
        if(fmt != 8){
            fputs("XSETTINGS: Wrong format.\n", stderr);
            goto out;
        }
    }
    
    xs = (struct XSettings*)prop;
    p  = (char*)(xs + 1);
    
    if(xs->byte_order != 0){
        fputs("FIXME: XSETTINGS not host byte order?\n", stderr);
        goto out;
    }
    
    for(int i = 0; i < xs->num_settings; ++i){
        struct XSettingHeader* h = (struct XSettingHeader*)p;
        
        // const char* strs[] = { "Int", "String", "Color" };
        // printf("%s:\t\"%.*s\"\n", strs[h->type], h->name_len, h->name);
        
        p += sizeof(struct XSettingHeader);
        p += h->name_len;
        p += ((4 - (h->name_len & 3)) & 3);
        p += 4; // serial
        
        switch(h->type){
            case XSettingsTypeInt: {
                if(strncmp(h->name, "Xft/DPI", h->name_len) == 0){
                    dpi = *(i32*)p;
                    if(dpi != -1) dpi /= 1024;
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
                fputs("XSETTINGS: Got invalid type...\n", stderr);
                goto out;
            } break;
        }
    }
    
    out:
    if(prop){
        XFree(prop);
    }
    
    return dpi;
}

//
// X11 window init
//

internal i32
size_change(i32 x, i32 y){
    f32 xs = x/96.f;
    f32 ys = y/96.f;
    f32 s = Min(xs, ys);
    i32 r = floor32(s);
    return(r);
}

#define BASE_W 800
#define BASE_H 600

internal b32
LinuxX11WindowInit(int argc, char** argv, int* window_width, int* window_height){
    if (linuxvars.settings.set_window_size){
        *window_width = linuxvars.settings.window_w;
        *window_height = linuxvars.settings.window_h;
    } else {
        i32 schange = size_change(linuxvars.dpi_x, linuxvars.dpi_y);
        *window_width = BASE_W * schange;
        *window_height = BASE_H * schange;
    }
    
    if (!GLXCanUseFBConfig(linuxvars.XDisplay)){
        LinuxFatalErrorMsg("Your XServer's GLX version is too old. GLX 1.3+ is required.");
        return false;
    }
    
    glx_config_result Config = ChooseGLXConfig(linuxvars.XDisplay, DefaultScreen(linuxvars.XDisplay));
    if (!Config.Found){
        LinuxFatalErrorMsg("Could not get a matching GLX FBConfig. Check your OpenGL drivers are installed correctly.");
        return false;
    }
    
    XSetWindowAttributes swa = {};
    swa.backing_store = WhenMapped;
    swa.event_mask = StructureNotifyMask;
    swa.bit_gravity = NorthWestGravity;
    swa.colormap = XCreateColormap(linuxvars.XDisplay, RootWindow(linuxvars.XDisplay, Config.BestInfo.screen), Config.BestInfo.visual, AllocNone);
    
    u32 CWflags = CWBackingStore|CWBitGravity|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask;
    linuxvars.XWindow = XCreateWindow(linuxvars.XDisplay, RootWindow(linuxvars.XDisplay, Config.BestInfo.screen), 0, 0, *window_width, *window_height, 0, Config.BestInfo.depth, InputOutput, Config.BestInfo.visual, CWflags, &swa);
    
    if (!linuxvars.XWindow){
        LinuxFatalErrorMsg("XCreateWindow failed. Make sure your display is set up correctly.");
        return false;
    }
    
    //NOTE(inso): Set the window's type to normal
    XChangeProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)&linuxvars.atom__NET_WM_WINDOW_TYPE_NORMAL, 1);
    
    //NOTE(inso): window managers want the PID as a window property for some reason.
    pid_t pid = getpid();
    XChangeProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&pid, 1);
    
#define WINDOW_NAME "4coder 4linux: " VERSION
    
    //NOTE(inso): set wm properties
    XStoreName(linuxvars.XDisplay, linuxvars.XWindow, WINDOW_NAME);
    
    XSizeHints *sz_hints = XAllocSizeHints();
    XWMHints   *wm_hints = XAllocWMHints();
    XClassHint *cl_hints = XAllocClassHint();
    
    sz_hints->flags = PMinSize | PMaxSize | PWinGravity;
    
    sz_hints->min_width = 50;
    sz_hints->min_height = 50;
    
    sz_hints->max_width = sz_hints->max_height = (1UL << 16UL);
    
    /* NOTE(inso): fluxbox thinks this is minimum, so don't set it
        sz_hints->base_width = BASE_W;
        sz_hints->base_height = BASE_H;
    */
    sz_hints->win_gravity = NorthWestGravity;
    
    if (linuxvars.settings.set_window_pos){
        sz_hints->flags |= USPosition;
        sz_hints->x = linuxvars.settings.window_x;
        sz_hints->y = linuxvars.settings.window_y;
    }
    
    wm_hints->flags |= InputHint | StateHint;
    wm_hints->input = True;
    wm_hints->initial_state = NormalState;
    
    cl_hints->res_name = "4coder";
    cl_hints->res_class = "4coder";
    
    char* win_name_list[] = { WINDOW_NAME };
    XTextProperty win_name;
    XStringListToTextProperty(win_name_list, 1, &win_name);
    
    XSetWMProperties(linuxvars.XDisplay, linuxvars.XWindow, &win_name, NULL, argv, argc, sz_hints, wm_hints, cl_hints);
    
    XFree(win_name.value);
    
    XFree(sz_hints);
    XFree(wm_hints);
    XFree(cl_hints);
    
    LinuxSetIcon(linuxvars.XDisplay, linuxvars.XWindow);
    
    //NOTE(inso): make the window visible
    XMapWindow(linuxvars.XDisplay, linuxvars.XWindow);
    
    b32 IsLegacy = false;
    GLXContext GLContext =
        InitializeOpenGLContext(linuxvars.XDisplay, linuxvars.XWindow, Config.BestConfig, IsLegacy);
    
    XRaiseWindow(linuxvars.XDisplay, linuxvars.XWindow);
    
    if (linuxvars.settings.set_window_pos){
        XMoveWindow(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.settings.window_x, linuxvars.settings.window_y);
    }
    
    if (linuxvars.settings.maximize_window){
        LinuxMaximizeWindow(linuxvars.XDisplay, linuxvars.XWindow, 1);
    }
    else if(linuxvars.settings.fullscreen_window){
        LinuxToggleFullscreen(linuxvars.XDisplay, linuxvars.XWindow);
    }
    
    XSync(linuxvars.XDisplay, False);
    
    XWindowAttributes WinAttribs;
    if (XGetWindowAttributes(linuxvars.XDisplay, linuxvars.XWindow, &WinAttribs))
    {
        *window_width = WinAttribs.width;
        *window_height = WinAttribs.height;
    }
    
    Atom wm_protos[] = {
        linuxvars.atom_WM_DELETE_WINDOW,
        linuxvars.atom__NET_WM_PING
    };
    
    XSetWMProtocols(linuxvars.XDisplay, linuxvars.XWindow, wm_protos, 2);
    
    return true;
}

internal void
LinuxHandleX11Events(void)
{
    static XEvent PrevEvent = {};
    b32 should_step = 0;
    
    while(XPending(linuxvars.XDisplay))
    {
        XEvent Event;
        XNextEvent(linuxvars.XDisplay, &Event);
        
        if (XFilterEvent(&Event, None) == True){
            continue;
        }
        
        switch (Event.type){
            case KeyPress: {
                should_step = 1;
                
                b32 is_hold = (PrevEvent.type         == KeyRelease &&
                               PrevEvent.xkey.time    == Event.xkey.time &&
                               PrevEvent.xkey.keycode == Event.xkey.keycode);
                
                b8 mods[MDFR_INDEX_COUNT] = {};
                mods[MDFR_HOLD_INDEX] = is_hold;
                
                if(Event.xkey.state & ShiftMask) mods[MDFR_SHIFT_INDEX] = 1;
                if(Event.xkey.state & ControlMask) mods[MDFR_CONTROL_INDEX] = 1;
                if(Event.xkey.state & LockMask) mods[MDFR_CAPS_INDEX] = 1;
                if(Event.xkey.state & Mod1Mask) mods[MDFR_ALT_INDEX] = 1;
                
                Event.xkey.state &= ~(ControlMask);
                
                Status status;
                KeySym keysym = NoSymbol;
                u8 buff[32] = {};
                
                Xutf8LookupString(linuxvars.input_context, &Event.xkey, (char*)buff, sizeof(buff) - 1, &keysym, &status);
                
                if(status == XBufferOverflow){
                    //TODO(inso): handle properly
                    Xutf8ResetIC(linuxvars.input_context);
                    XSetICFocus(linuxvars.input_context);
                    fputs("FIXME: XBufferOverflow from LookupString.\n", stderr);
                }
                
                u32 key = utf8_to_u32_unchecked(buff);
                u32 key_no_caps = key;
                
                if(mods[MDFR_CAPS_INDEX] && status == XLookupBoth && Event.xkey.keycode){
                    u8 buff_no_caps[32] = {0};
                    Event.xkey.state &= ~(LockMask);
                    
                    XLookupString(&Event.xkey, (char*)buff_no_caps, sizeof(buff_no_caps) - 1, NULL, NULL);
                    
                    if(*buff_no_caps){
                        key_no_caps = utf8_to_u32_unchecked(buff_no_caps);
                    }
                }
                
                if(key         == '\r') key         = '\n';
                if(key_no_caps == '\r') key_no_caps = '\n';
                
                // don't push modifiers
                if(keysym >= XK_Shift_L && keysym <= XK_Hyper_R){
                    break;
                }
                
                if(keysym == XK_ISO_Left_Tab){
                    key = key_no_caps = '\t';
                    mods[MDFR_SHIFT_INDEX] = 1;
                }
                
                Key_Code special_key = keycode_lookup_table[(u8)Event.xkey.keycode];
                
                if(special_key){
                    LinuxPushKey(special_key, 0, 0, &mods);
                } else if(key < 256){
                    LinuxPushKey(key, key, key_no_caps, &mods);
                } else {
                    LinuxPushKey(0, 0, 0, &mods);
                }
            }break;
            
            case KeyRelease: {
                should_step = 1;
            }break;
            
            case MotionNotify: {
                should_step = 1;
                linuxvars.input.mouse.x = Event.xmotion.x;
                linuxvars.input.mouse.y = Event.xmotion.y;
            }break;
            
            case ButtonPress: {
                should_step = 1;
                switch(Event.xbutton.button){
                    case Button1: {
                        linuxvars.input.mouse.press_l = 1;
                        linuxvars.input.mouse.l = 1;
                    } break;
                    case Button3: {
                        linuxvars.input.mouse.press_r = 1;
                        linuxvars.input.mouse.r = 1;
                    } break;
                    
                    //NOTE(inso): scroll up
                    case Button4: {
                        linuxvars.input.mouse.wheel = 1;
                    }break;
                    
                    //NOTE(inso): scroll down
                    case Button5: {
                        linuxvars.input.mouse.wheel = -1;
                    }break;
                }
            }break;
            
            case ButtonRelease: {
                should_step = 1;
                switch(Event.xbutton.button){
                    case Button1: {
                        linuxvars.input.mouse.release_l = 1;
                        linuxvars.input.mouse.l = 0;
                    } break;
                    case Button3: {
                        linuxvars.input.mouse.release_r = 1;
                        linuxvars.input.mouse.r = 0;
                    } break;
                }
            }break;
            
            case EnterNotify: {
                should_step = 1;
                linuxvars.input.mouse.out_of_window = 0;
            }break;
            
            case LeaveNotify: {
                should_step = 1;
                linuxvars.input.mouse.out_of_window = 1;
            }break;
            
            case FocusIn:
            case FocusOut: {
                should_step = 1;
                linuxvars.input.mouse.l = 0;
                linuxvars.input.mouse.r = 0;
            }break;
            
            case ConfigureNotify: {
                should_step = 1;
                i32 w = Event.xconfigure.width;
                i32 h = Event.xconfigure.height;
                
                if(w != linuxvars.target.width || h != linuxvars.target.height){
                    LinuxResizeTarget(w, h);
                }
            }break;
            
            case MappingNotify: {
                if(Event.xmapping.request == MappingModifier || Event.xmapping.request == MappingKeyboard){
                    XRefreshKeyboardMapping(&Event.xmapping);
                    LinuxKeycodeInit(linuxvars.XDisplay);
                }
            }break;
            
            case ClientMessage: {
                if ((Atom)Event.xclient.data.l[0] == linuxvars.atom_WM_DELETE_WINDOW) {
                    should_step = 1;
                    linuxvars.keep_running = 0;
                }
                else if ((Atom)Event.xclient.data.l[0] == linuxvars.atom__NET_WM_PING) {
                    Event.xclient.window = DefaultRootWindow(linuxvars.XDisplay);
                    XSendEvent(
                        linuxvars.XDisplay,
                        Event.xclient.window,
                        False,
                        SubstructureRedirectMask | SubstructureNotifyMask,
                        &Event
                        );
                }
            }break;
            
            // NOTE(inso): Someone wants us to give them the clipboard data.
            case SelectionRequest: {
                XSelectionRequestEvent request = Event.xselectionrequest;
                
                XSelectionEvent response = {};
                response.type = SelectionNotify;
                response.requestor = request.requestor;
                response.selection = request.selection;
                response.target = request.target;
                response.time = request.time;
                response.property = None;
                
                if (
                    linuxvars.clipboard_outgoing.size &&
                    request.selection == linuxvars.atom_CLIPBOARD &&
                    request.property != None &&
                    request.display &&
                    request.requestor
                    ){
                    Atom atoms[] = {
                        XA_STRING,
                        linuxvars.atom_UTF8_STRING
                    };
                    
                    if(request.target == linuxvars.atom_TARGETS){
                        
                        XChangeProperty(
                            request.display,
                            request.requestor,
                            request.property,
                            XA_ATOM,
                            32,
                            PropModeReplace,
                            (u8*)atoms,
                            ArrayCount(atoms)
                            );
                        
                        response.property = request.property;
                        
                    } else {
                        b32 found = false;
                        for(int i = 0; i < ArrayCount(atoms); ++i){
                            if(request.target == atoms[i]){
                                found = true;
                                break;
                            }
                        }
                        
                        if(found){
                            XChangeProperty(
                                request.display,
                                request.requestor,
                                request.property,
                                request.target,
                                8,
                                PropModeReplace,
                                (u8*)linuxvars.clipboard_outgoing.str,
                                linuxvars.clipboard_outgoing.size
                                );
                            
                            response.property = request.property;
                        }
                    }
                }
                
                XSendEvent(request.display, request.requestor, True, 0, (XEvent*)&response);
                
            }break;
            
            // NOTE(inso): Another program is now the clipboard owner.
            case SelectionClear: {
                if(Event.xselectionclear.selection == linuxvars.atom_CLIPBOARD){
                    linuxvars.clipboard_outgoing.size = 0;
                }
            }break;
            
            // NOTE(inso): A program is giving us the clipboard data we asked for.
            case SelectionNotify: {
                XSelectionEvent* e = (XSelectionEvent*)&Event;
                if(e->selection == linuxvars.atom_CLIPBOARD && e->target == linuxvars.atom_UTF8_STRING && e->property != None){
                    Atom type;
                    int fmt;
                    unsigned long nitems, bytes_left;
                    u8 *data;
                    
                    int result = XGetWindowProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom_CLIPBOARD, 0L, LINUX_MAX_PASTE_CHARS/4L, False, linuxvars.atom_UTF8_STRING, &type, &fmt, &nitems, &bytes_left, &data);
                    
                    if(result == Success && fmt == 8){
                        LinuxStringDup(&linuxvars.clipboard_contents, data, nitems);
                        should_step = 1;
                        linuxvars.new_clipboard = 1;
                        XFree(data);
                        XDeleteProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom_CLIPBOARD);
                    }
                }
            }break;
            
            case Expose:
            case VisibilityNotify: {
                should_step = 1;
            }break;
            
            default: {
                if(Event.type == linuxvars.xfixes_selection_event){
                    XFixesSelectionNotifyEvent* sne = (XFixesSelectionNotifyEvent*)&Event;
                    if(sne->subtype == XFixesSelectionNotify && sne->owner != linuxvars.XWindow){
                        XConvertSelection(linuxvars.XDisplay, linuxvars.atom_CLIPBOARD, linuxvars.atom_UTF8_STRING, linuxvars.atom_CLIPBOARD, linuxvars.XWindow, CurrentTime);
                    }
                }
            }break;
        }
        
        PrevEvent = Event;
    }
    
    if(should_step){
        LinuxScheduleStep();
    }
}

//
// Entry point
//

int
main(int argc, char **argv){
    //
    // System & Memory init
    //
    
    char base_dir_mem[PATH_MAX];
    String base_dir = make_fixed_width_string(base_dir_mem);
    
    if (!LinuxLoadAppCode(&base_dir)){
        LinuxFatalErrorMsg("Could not load '4ed_app.so'. This file should be in the same directory as the main '4ed' executable.");
        return 99;
    }
    
    LinuxLoadSystemCode();
    LinuxLoadRenderCode();
    
    memory_vars.vars_memory_size   = MB(2);
    memory_vars.vars_memory        = system_memory_allocate(memory_vars.vars_memory_size);
    memory_vars.target_memory_size = MB(512);
    memory_vars.target_memory      = system_memory_allocate(memory_vars.target_memory_size);
    memory_vars.user_memory_size   = MB(2);
    memory_vars.user_memory        = system_memory_allocate(memory_vars.user_memory_size);
    
    linuxvars.target.max         = MB(1);
    linuxvars.target.push_buffer = (char*)system_memory_allocate(linuxvars.target.max);
    
    if(memory_vars.vars_memory == NULL || memory_vars.target_memory == NULL || memory_vars.user_memory == NULL || linuxvars.target.push_buffer == NULL){
        LinuxFatalErrorMsg("Could not allocate sufficient memory. Please make sure you have atleast 512Mb of RAM free. (This requirement will be relaxed in the future).");
        exit(1);
    }
    
    init_shared_vars();
    
    //
    // Read command line
    //
    
    char* cwd = get_current_dir_name();
    if(!cwd){
        char buf[1024];
        snprintf(buf, sizeof(buf), "Call to get_current_dir_name failed: %s", strerror(errno));
        LinuxFatalErrorMsg(buf);
        return 1;
    }
    
    String current_directory = make_string_slowly(cwd);
    
    Command_Line_Parameters clparams;
    clparams.argv = argv;
    clparams.argc = argc;
    
    char **files;
    i32 *file_count;
    i32 output_size;
    
    output_size = linuxvars.app.read_command_line(&linuxvars.system, &memory_vars, current_directory, &linuxvars.settings, &files, &file_count, clparams);
    
    if (output_size > 0){
        // TODO(allen): crt free version
        fprintf(stdout, "%.*s", output_size, (char*)memory_vars.target_memory);
    }
    if (output_size != 0){
        LinuxFatalErrorMsg("Error reading command-line arguments.");
        return 1;
    }
    
    sysshared_filter_real_files(files, file_count);
    
    //
    // Custom layer linkage
    //
    
#ifdef FRED_SUPER
    
    char *custom_file_default = "custom_4coder.so";
    sysshared_to_binary_path(&base_dir, custom_file_default);
    custom_file_default = base_dir.str;
    
    char *custom_file;
    if (linuxvars.settings.custom_dll){
        custom_file = linuxvars.settings.custom_dll;
    } else {
        custom_file = custom_file_default;
    }
    
    linuxvars.custom = dlopen(custom_file, RTLD_LAZY);
    if (!linuxvars.custom && custom_file != custom_file_default){
        if (!linuxvars.settings.custom_dll_is_strict){
            linuxvars.custom = dlopen(custom_file_default, RTLD_LAZY);
        }
    }
    
    if (linuxvars.custom){
        linuxvars.custom_api.get_alpha_4coder_version = (_Get_Version_Function*)
            dlsym(linuxvars.custom, "get_alpha_4coder_version");
        
        if (linuxvars.custom_api.get_alpha_4coder_version == 0 ||
            linuxvars.custom_api.get_alpha_4coder_version(MAJOR, MINOR, PATCH) == 0){
            LinuxFatalErrorMsg("Failed to load 'custom_4coder.so': Version mismatch. Try rebuilding it with 'buildsuper.sh'.");
            exit(1);
        }
        else{
            linuxvars.custom_api.get_bindings = (Get_Binding_Data_Function*)
                dlsym(linuxvars.custom, "get_bindings");
            
            if (linuxvars.custom_api.get_bindings == 0){
                LinuxFatalErrorMsg("Failed to load 'custom_4coder.so': "
                                   "It does not export the required 'get_bindings' function. "
                                   "Try rebuilding it with 'buildsuper.sh'.");
                exit(1);
            }
            else{
                fprintf(stderr, "Successfully loaded custom_4coder.so\n");
            }
        }
    } else {
        char buf[4096];
        const char* error = dlerror();
        snprintf(buf, sizeof(buf), "Error loading custom: %s. "
                 "Make sure this file is in the same directory as the main '4ed' executable.",
                 error ? error : "'custom_4coder.so' missing");
        LinuxFatalErrorMsg(buf);
        exit(1);
    }
#else
    linuxvars.custom_api.get_bindings = get_bindings;
#endif
    
    //
    // Coroutine / Thread / Semaphore / Mutex init
    //
    
    linuxvars.coroutine_free = linuxvars.coroutine_data;
    for (i32 i = 0; i+1 < ArrayCount(linuxvars.coroutine_data); ++i){
        linuxvars.coroutine_data[i].next = linuxvars.coroutine_data + i + 1;
    }
    
    const size_t stack_size = MB(2);
    for (i32 i = 0; i < ArrayCount(linuxvars.coroutine_data); ++i){
        linuxvars.coroutine_data[i].stack.ss_size = stack_size;
        linuxvars.coroutine_data[i].stack.ss_sp = system_memory_allocate(stack_size);
    }
    
    Thread_Context background[4] = {};
    linuxvars.groups[BACKGROUND_THREADS].threads = background;
    linuxvars.groups[BACKGROUND_THREADS].count = ArrayCount(background);
    linuxvars.groups[BACKGROUND_THREADS].cancel_lock0 = CANCEL_LOCK0;
    linuxvars.groups[BACKGROUND_THREADS].cancel_cv0 = 0;
    
    Thread_Memory thread_memory[ArrayCount(background)];
    linuxvars.thread_memory = thread_memory;
    
    sem_init(&linuxvars.thread_semaphore, 0, 0);
    linuxvars.queues[BACKGROUND_THREADS].semaphore = LinuxSemToHandle(&linuxvars.thread_semaphore);
    
    for(i32 i = 0; i < linuxvars.groups[BACKGROUND_THREADS].count; ++i){
        Thread_Context *thread = linuxvars.groups[BACKGROUND_THREADS].threads + i;
        thread->id = i + 1;
        thread->group_id = BACKGROUND_THREADS;
        
        Thread_Memory *memory = linuxvars.thread_memory + i;
        *memory = thread_memory_zero();
        memory->id = thread->id;
        
        thread->queue = &linuxvars.queues[BACKGROUND_THREADS];
        pthread_create(&thread->handle, NULL, &JobThreadProc, thread);
    }
    
    initialize_unbounded_queue(&linuxvars.groups[BACKGROUND_THREADS].queue);
    
    for(i32 i = 0; i < LOCK_COUNT; ++i){
        pthread_mutex_init(linuxvars.locks + i, NULL);
    }
    
    for(i32 i = 0; i < ArrayCount(linuxvars.conds); ++i){
        pthread_cond_init(linuxvars.conds + i, NULL);
    }
    
    //
    // X11 init
    //
    
    linuxvars.XDisplay = XOpenDisplay(0);
    if(!linuxvars.XDisplay){
        // NOTE(inso): probably not worth trying the popup in this case...
        fprintf(stderr, "Can't open display!\n");
        return 1;
    }
    
#define LOAD_ATOM(x) linuxvars.atom_##x = XInternAtom(linuxvars.XDisplay, #x, False);
    
    LOAD_ATOM(TARGETS);
    LOAD_ATOM(CLIPBOARD);
    LOAD_ATOM(UTF8_STRING);
    LOAD_ATOM(_NET_WM_STATE);
    LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
    LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
    LOAD_ATOM(_NET_WM_STATE_FULLSCREEN);
    LOAD_ATOM(_NET_WM_PING);
    LOAD_ATOM(_NET_WM_WINDOW_TYPE);
    LOAD_ATOM(_NET_WM_WINDOW_TYPE_NORMAL);
    LOAD_ATOM(_NET_WM_PID);
    LOAD_ATOM(WM_DELETE_WINDOW);
    
#undef LOAD_ATOM
    
#if SUPPORT_DPI
    linuxvars.dpi_x = linuxvars.dpi_y = LinuxGetXSettingsDPI(linuxvars.XDisplay, DefaultScreen(linuxvars.XDisplay));
    
    // fallback
    if(linuxvars.dpi_x == -1){
        int scr = DefaultScreen(linuxvars.XDisplay);
        
        int dw = DisplayWidth(linuxvars.XDisplay, scr);
        int dh = DisplayHeight(linuxvars.XDisplay, scr);
        
        int dw_mm = DisplayWidthMM(linuxvars.XDisplay, scr);
        int dh_mm = DisplayHeightMM(linuxvars.XDisplay, scr);
        
        linuxvars.dpi_x = dw_mm ? dw / (dw_mm / 25.4) : 96;
        linuxvars.dpi_y = dh_mm ? dh / (dh_mm / 25.4) : 96;
        
        fprintf(stderr, "%dx%d - %dmmx%dmm DPI: %dx%d\n", dw, dh, dw_mm, dh_mm, linuxvars.dpi_x, linuxvars.dpi_y);
    }
    else{
        fprintf(stderr, "DPI from XSETTINGS: %d\n", linuxvars.dpi_x);
    }
#endif
    
    int window_width, window_height;
    if (!LinuxX11WindowInit(argc, argv, &window_width, &window_height)){
        return 1;
    }
    
    int xfixes_version_unused, xfixes_err_unused;
    b32 xquery_extension_r = XQueryExtension(linuxvars.XDisplay, "XFIXES", &xfixes_version_unused, &linuxvars.xfixes_selection_event, &xfixes_err_unused);
    linuxvars.has_xfixes = (xquery_extension_r == True);
    
    if (linuxvars.has_xfixes){
        XFixesSelectSelectionInput(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom_CLIPBOARD, XFixesSetSelectionOwnerNotifyMask);
    }
    else{
        fprintf(stderr, "Your X server doesn't support XFIXES, mention this fact if you report any clipboard-related issues.\n");
    }
    
    Init_Input_Result input_result = LinuxInputInit(linuxvars.XDisplay, linuxvars.XWindow);
    
    linuxvars.input_method = input_result.input_method;
    linuxvars.input_style = input_result.best_style;
    linuxvars.input_context = input_result.xic;
    
    LinuxKeycodeInit(linuxvars.XDisplay);
    
    Cursor xcursors[APP_MOUSE_CURSOR_COUNT] = {
        None,
        None,
        XCreateFontCursor(linuxvars.XDisplay, XC_xterm),
        XCreateFontCursor(linuxvars.XDisplay, XC_sb_h_double_arrow),
        XCreateFontCursor(linuxvars.XDisplay, XC_sb_v_double_arrow)
    };
    
    {
        char data = 0;
        XColor c  = {};
        Pixmap p  = XCreateBitmapFromData(linuxvars.XDisplay, linuxvars.XWindow, &data, 1, 1);
        
        linuxvars.hidden_cursor = XCreatePixmapCursor(linuxvars.XDisplay, p, p, &c, &c, 0, 0);
        
        XFreePixmap(linuxvars.XDisplay, p);
    }
    
    
    //
    // Font System Init
    //
    
    system_font_init(&linuxvars.system.font, 0, 0, linuxvars.settings.font_size, linuxvars.settings.use_hinting);
    
    //
    // Epoll init
    //
    
    linuxvars.x11_fd        = ConnectionNumber(linuxvars.XDisplay);
    linuxvars.inotify_fd    = inotify_init1(IN_NONBLOCK);
    linuxvars.step_event_fd = eventfd(0, EFD_NONBLOCK);
    linuxvars.step_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    
    linuxvars.epoll = epoll_create(16);
    
    {
        struct epoll_event e = {};
        e.events = EPOLLIN | EPOLLET;
        
        e.data.u64 = LINUX_4ED_EVENT_X11;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.x11_fd, &e);
        
        e.data.u64 = LINUX_4ED_EVENT_STEP;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.step_event_fd, &e);
        
        e.data.u64 = LINUX_4ED_EVENT_STEP_TIMER;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.step_timer_fd, &e);
    }
    
    //
    // App init
    //
    
    XAddConnectionWatch(linuxvars.XDisplay, &LinuxX11ConnectionWatch, NULL);
    
    linuxvars.app.init(&linuxvars.system, &linuxvars.target, &memory_vars, linuxvars.clipboard_contents, current_directory, linuxvars.custom_api);
    
    LinuxResizeTarget(window_width, window_height);
    
    //
    // Main loop
    //
    
    system_acquire_lock(FRAME_LOCK);
    
    LinuxScheduleStep();
    
    linuxvars.keep_running = 1;
    linuxvars.input.first_step = 1;
    linuxvars.input.dt = (frame_useconds / 1000000.f);
    
    while (1){
        if (XEventsQueued(linuxvars.XDisplay, QueuedAlready)){
            LinuxHandleX11Events();
        }
        
        system_release_lock(FRAME_LOCK);
        
        struct epoll_event events[16];
        int num_events = epoll_wait(linuxvars.epoll, events, ArrayCount(events), -1);
        
        system_acquire_lock(FRAME_LOCK);
        
        if (num_events == -1){
            if (errno != EINTR){
                fprintf(stderr, "epoll_wait\n");
            }
            continue;
        }
        
        b32 do_step = 0;
        
        for(int i = 0; i < num_events; ++i){
            int fd   = events[i].data.u64 & UINT32_MAX;
            u64 type = events[i].data.u64 & ~fd;
            
            switch(type){
                case LINUX_4ED_EVENT_X11: {
                    LinuxHandleX11Events();
                } break;
                
                case LINUX_4ED_EVENT_X11_INTERNAL: {
                    XProcessInternalConnection(linuxvars.XDisplay, fd);
                } break;
                
                case LINUX_4ED_EVENT_STEP: {
                    u64 ev;
                    int ret;
                    do {
                        ret = read(linuxvars.step_event_fd, &ev, 8);
                    } while(ret != -1 || errno != EAGAIN);
                    do_step = 1;
                } break;
                
                case LINUX_4ED_EVENT_STEP_TIMER: {
                    u64 count;
                    int ret;
                    do {
                        ret = read(linuxvars.step_timer_fd, &count, 8);
                    } while(ret != -1 || errno != EAGAIN);
                    do_step = 1;
                } break;
                
                case LINUX_4ED_EVENT_CLI: {
                    LinuxScheduleStep();
                } break;
            }
        }
        
        if(do_step){
            linuxvars.last_step = system_now_time();
            
            if(linuxvars.input.first_step || !linuxvars.has_xfixes){
                XConvertSelection(
                    linuxvars.XDisplay,
                    linuxvars.atom_CLIPBOARD,
                    linuxvars.atom_UTF8_STRING,
                    linuxvars.atom_CLIPBOARD,
                    linuxvars.XWindow,
                    CurrentTime
                    );
            }
            
            Application_Step_Result result = {};
            result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
            result.trying_to_kill = !linuxvars.keep_running;
            
            if(linuxvars.new_clipboard){
                linuxvars.input.clipboard = linuxvars.clipboard_contents;
                linuxvars.new_clipboard = 0;
            } else {
                linuxvars.input.clipboard = null_string;
            }
            
            b32 keep_running = linuxvars.keep_running;
            
            linuxvars.app.step(
                &linuxvars.system,
                &linuxvars.target,
                &memory_vars,
                &linuxvars.input,
                &result,
                clparams
                );
            
            if(result.perform_kill){
                break;
            } else if(!keep_running && !linuxvars.keep_running){
                linuxvars.keep_running = 1;
            }
            
            if(result.animating){
                LinuxScheduleStep();
            }
            
            LinuxRedrawTarget();
            
            if(result.mouse_cursor_type != linuxvars.cursor && !linuxvars.input.mouse.l){
                Cursor c = xcursors[result.mouse_cursor_type];
                if(!linuxvars.hide_cursor){
                    XDefineCursor(linuxvars.XDisplay, linuxvars.XWindow, c);
                }
                linuxvars.cursor = result.mouse_cursor_type;
            }
            
            flush_thread_group(BACKGROUND_THREADS);
            
            linuxvars.input.first_step = 0;
            linuxvars.input.keys = null_key_input_data;
            linuxvars.input.mouse.press_l = 0;
            linuxvars.input.mouse.release_l = 0;
            linuxvars.input.mouse.press_r = 0;
            linuxvars.input.mouse.release_r = 0;
            linuxvars.input.mouse.wheel = 0;
        }
    }
    
    if(linuxvars.XDisplay){
        if(linuxvars.XWindow) XDestroyWindow(linuxvars.XDisplay, linuxvars.XWindow);
        XCloseDisplay(linuxvars.XDisplay);
    }
    
    return 0;
}

#include "linux_4ed_fonts.cpp"
#include "linux_4ed_file_track.cpp"
#include "4ed_font_static_functions.cpp"

// BOTTOM
// vim: expandtab:ts=4:sts=4:sw=4

