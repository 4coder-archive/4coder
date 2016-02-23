/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.11.2015
 *
 * Linux layer for project codename "4ed"
 *
 */

// TOP

#include "4ed_config.h"

#include "4ed_meta.h"

#define FCPP_FORBID_MALLOC

#include "4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4ed_mem.cpp"
#include "4ed_math.cpp"

#include "4coder_custom.h"
#include "4ed_system.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <xmmintrin.h>
#include <linux/fs.h>
//#include <X11/extensions/XInput2.h>
#include <linux/input.h>
#include <time.h>
#include <dlfcn.h>
#include <unistd.h>

#include "4ed_internal.h"
#include "4ed_linux_keyboard.cpp"
#include "system_shared.h"

#include <stdlib.h>
#include <locale.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

struct Linux_Semaphore {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

struct Linux_Semaphore_Handle {
    pthread_mutex_t *mutex_p;
    pthread_cond_t *cond_p;
};

struct Thread_Context{
    u32 job_id;
    b32 running;
    
    Work_Queue *queue;
    u32 id;
    pthread_t handle;
};

struct Thread_Group{
    Thread_Context *threads;
    i32 count;
};

struct Linux_Vars{
    Display *XDisplay;
    Window XWindow;
    Render_Target target;

    XIM input_method;
    XIMStyle input_style;
    XIC input_context;
    Key_Codes key_codes;

	Key_Input_Data key_data;
    Mouse_State mouse_data;

    String clipboard_contents;
    String clipboard_outgoing;
   
    Atom atom_CLIPBOARD;
    Atom atom_UTF8_STRING;

    void *app_code;
    void *custom;

    Thread_Memory *thread_memory;
    Thread_Group groups[THREAD_GROUP_COUNT];
    Linux_Semaphore thread_locks[THREAD_GROUP_COUNT];
    Linux_Semaphore locks[LOCK_COUNT];
    
    Plat_Settings settings;
    System_Functions *system;
    App_Functions app;
    Custom_API custom_api;
    b32 first;
    b32 redraw;
    b32 vsync;
    
#if FRED_INTERNAL
    Sys_Bubble internal_bubble;
#endif

    Font_Load_System fnt;
};

#define LINUX_MAX_PASTE_CHARS 0x10000L
#define FPS 60
#define frame_useconds (1000000 / FPS)

#define DBG_FN do { fprintf(stderr, "Fn called: %s\n", __PRETTY_FUNCTION__); } while(0)

globalvar Linux_Vars linuxvars;
globalvar Application_Memory memory_vars;
globalvar Exchange exchange_vars;

#define LinuxGetMemory(size) LinuxGetMemory_(size, __LINE__, __FILE__)

internal void*
LinuxGetMemory_(i32 size, i32 line_number, char *file_name){
    // TODO(allen): Implement without stdlib.h
    void *result = 0;
    
    Assert(size != 0);
    
#if FRED_INTERNAL
    Sys_Bubble *bubble;
    
    result = malloc(size + sizeof(Sys_Bubble));
    bubble = (Sys_Bubble*)result;
    bubble->flags = MEM_BUBBLE_SYS_DEBUG;
    bubble->line_number = line_number;
    bubble->file_name = file_name;
    bubble->size = size;
    
    // TODO(allen): make Sys_Bubble list thread safe
    insert_bubble(&linuxvars.internal_bubble, bubble);
    result = bubble + 1;
    
#else
    result = malloc(size);
#endif
    
    return(result);
}

internal void
LinuxFreeMemory(void *block){
    // TODO(allen): Implement without stdlib.h
    
    if (block){
#if FRED_INTERNAL
        Sys_Bubble *bubble;
        
        bubble = (Sys_Bubble*)block;
        --bubble;
        Assert((bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_SYS_DEBUG);

        // TODO(allen): make Sys_Bubble list thread safe
        remove_bubble(bubble);
        
        free(bubble);
#else
        free(block);
#endif
    }
}

internal Partition
LinuxScratchPartition(i32 size){
    Partition part;
    void *data;
    data = LinuxGetMemory(size);
    part = partition_open(data, size);
    return(part);
}

internal void
LinuxScratchPartitionGrow(Partition *part, i32 new_size){
    void *data;
    if (new_size > part->max){
        data = LinuxGetMemory(new_size);
        memcpy(data, part->base, part->pos);
        LinuxFreeMemory(part->base);
        part->base = (u8*)data;
    }
}

internal void
LinuxScratchPartitionDouble(Partition *part){
    LinuxScratchPartitionGrow(part, part->max*2);
}

internal
Sys_Get_Memory_Sig(system_get_memory_){
    return(LinuxGetMemory_(size, line_number, file_name));
}

internal
Sys_Free_Memory_Sig(system_free_memory){
    LinuxFreeMemory(block);
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

#if (defined(_BSD_SOURCE) || defined(_SVID_SOURCE))
#define TimeBySt
#endif
#if (_POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700)
#define TimeBySt
#endif

#ifdef TimeBySt
#define nano_mtime_field st_mtim.tv_nsec
#undef TimeBySt
#else
#define nano_mtime_field st_mtimensec
#endif

Sys_File_Time_Stamp_Sig(system_file_time_stamp){
    struct stat info;
    i64 nanosecond_timestamp;
    u64 result;
    
    stat(filename, &info);
    nanosecond_timestamp = info.nano_mtime_field;
    if (nanosecond_timestamp != 0){
        result = (u64)(nanosecond_timestamp / 1000);
    }
    else{
        result = (u64)(info.st_mtime * 1000000);
    }

    return(result);
}

// TODO(allen): DOES THIS AGREE WITH THE FILESTAMP TIMES?
// NOTE(inso): I don't think so, CLOCK_MONOTONIC is an arbitrary number
Sys_Time_Sig(system_time){
    struct timespec spec;
    u64 result;
    
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * 1000000) + (spec.tv_nsec / 1000);

    return(result);
}

Sys_Set_File_List_Sig(system_set_file_list){
    DIR *d;
    struct dirent *entry;
    char *fname, *cursor, *cursor_start;
    File_Info *info_ptr;
    i32 count, file_count, size, required_size;
    
    terminate_with_null(&directory);

    d = opendir(directory.str);
    if (d){
        count = 0;
        file_count = 0;
        for (entry = readdir(d);
             entry != 0;
             entry = readdir(d)){
            fname = entry->d_name;
            if(fname[0] == '.' && (fname[1] == 0 || (fname[1] == '.' && fname[2] == 0))){
                continue;
            }
            ++file_count;            
            for (size = 0; fname[size]; ++size);
            count += size + 1;
        }

        required_size = count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_free_memory(file_list->block);
            file_list->block = system_get_memory(required_size);
        }
        
        file_list->infos = (File_Info*)file_list->block;
        cursor = (char*)(file_list->infos + file_count);
        
        rewinddir(d);
        info_ptr = file_list->infos;
        for (entry = readdir(d);
            entry != 0;
            entry = readdir(d)){
            fname = entry->d_name;
            if(fname[0] == '.' && (fname[1] == 0 || (fname[1] == '.' && fname[2] == 0))){
                continue;
            }
            cursor_start = cursor;
            for (; *fname; ) *cursor++ = *fname++;

#ifdef _DIRENT_HAVE_D_TYPE
            if(entry->d_type != DT_UNKNOWN){
                info_ptr->folder = entry->d_type == DT_DIR;
            } else
#endif
            {
                struct stat st;
                if(lstat(entry->d_name, &st) != -1){
                    info_ptr->folder = S_ISDIR(st.st_mode);
                } else {
                    info_ptr->folder = 0;
                }
            }

            info_ptr->filename.str = cursor_start;
            info_ptr->filename.size = (i32)(cursor - cursor_start);
            *cursor++ = 0;
            info_ptr->filename.memory_size = info_ptr->filename.size + 1;
            ++info_ptr;
        }

        file_list->count = file_count;

        closedir(d);
    }
}

Sys_Post_Clipboard_Sig(system_post_clipboard){
    LinuxStringDup(&linuxvars.clipboard_outgoing, str.str, str.size);
    XSetSelectionOwner(linuxvars.XDisplay, linuxvars.atom_CLIPBOARD, linuxvars.XWindow, CurrentTime);
}

Sys_CLI_Call_Sig(system_cli_call){
    // TODO(allen): Implement
    DBG_FN;
    AllowLocal(path);
    AllowLocal(script_name);
    AllowLocal(cli_out);
}

Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    // TODO(allen): Implement
    DBG_FN;
    AllowLocal(cli);
}

Sys_CLI_Update_Step_Sig(system_cli_update_step){
    // TODO(allen): Implement
    DBG_FN;
    AllowLocal(cli);
    AllowLocal(dest);
    AllowLocal(max);
    AllowLocal(amount);
}

Sys_CLI_End_Update_Sig(system_cli_end_update){
    // TODO(allen): Implement
    DBG_FN;
    AllowLocal(cli);
}

static_assert(sizeof(Plat_Handle) >= sizeof(Linux_Semaphore_Handle), "Plat_Handle not big enough");

internal Plat_Handle
LinuxSemToHandle(Linux_Semaphore* sem){
    Linux_Semaphore_Handle h = { &sem->mutex, &sem->cond };
    return *(Plat_Handle*)&h;
}

internal void*
ThreadProc(void* arg){
    Thread_Context *thread = (Thread_Context*)arg;
    Work_Queue *queue = thread->queue;
    
    for (;;){
        u32 read_index = queue->read_position;
        u32 write_index = queue->write_position;
        
        if (read_index != write_index){
            u32 next_read_index = (read_index + 1) % JOB_ID_WRAP;
            u32 safe_read_index =
                __sync_val_compare_and_swap(&queue->read_position,
                                           read_index, next_read_index);
            
            if (safe_read_index == read_index){
                Full_Job_Data *full_job = queue->jobs + (safe_read_index % QUEUE_WRAP);
                // NOTE(allen): This is interlocked so that it plays nice
                // with the cancel job routine, which may try to cancel this job
                // at the same time that we try to run it
                
                i32 safe_running_thread =
                    __sync_val_compare_and_swap(&full_job->running_thread,
                                               THREAD_NOT_ASSIGNED, thread->id);
                
                if (safe_running_thread == THREAD_NOT_ASSIGNED){
                    thread->job_id = full_job->id;
                    thread->running = 1;
                    Thread_Memory *thread_memory = 0;
                    
                    // TODO(allen): remove memory_request
                    if (full_job->job.memory_request != 0){
                        thread_memory = linuxvars.thread_memory + thread->id - 1;
                        if (thread_memory->size < full_job->job.memory_request){
                            if (thread_memory->data){
                                LinuxFreeMemory(thread_memory->data);
                            }
                            i32 new_size = LargeRoundUp(full_job->job.memory_request, Kbytes(4));
                            thread_memory->data = LinuxGetMemory(new_size);
                            thread_memory->size = new_size;
                        }
                    }
                    full_job->job.callback(linuxvars.system, thread, thread_memory,
                                           &exchange_vars.thread, full_job->job.data);
                    full_job->running_thread = 0;
                    thread->running = 0;
                }
            }
        }
        else{
            Linux_Semaphore_Handle* h = (Linux_Semaphore_Handle*)&(queue->semaphore);
            pthread_cond_wait(h->cond_p, h->mutex_p);
        }
    }
}


Sys_Post_Job_Sig(system_post_job){
    // TODO(allen): Implement
    DBG_FN;
    AllowLocal(group_id);
    AllowLocal(job);

    Work_Queue *queue = exchange_vars.thread.queues + group_id;
    
    Assert((queue->write_position + 1) % QUEUE_WRAP != queue->read_position % QUEUE_WRAP);
    
    b32 success = 0;
    u32 result = 0;
    while (!success){
        u32 write_index = queue->write_position;
        u32 next_write_index = (write_index + 1) % JOB_ID_WRAP;
        u32 safe_write_index =
            __sync_val_compare_and_swap(&queue->write_position,
                                       write_index, next_write_index);
        if (safe_write_index  == write_index){
            result = write_index;
            write_index = write_index % QUEUE_WRAP;
            queue->jobs[write_index].job = job;
            queue->jobs[write_index].running_thread = THREAD_NOT_ASSIGNED;
            queue->jobs[write_index].id = result;
            success = 1;
        }
    }
    
    Linux_Semaphore_Handle* h = (Linux_Semaphore_Handle*)&(queue->semaphore);
    pthread_cond_broadcast(h->cond_p);
    
    return result;
}

Sys_Acquire_Lock_Sig(system_acquire_lock){
   // printf("%s: id: %d\n", __PRETTY_FUNCTION__, id);
    pthread_mutex_lock(&linuxvars.locks[id].mutex);
    //pthread_cond_wait(&linuxvars.locks[id].cond, &linuxvars.locks[id].mutex);
    pthread_mutex_unlock(&linuxvars.locks[id].mutex);
}

Sys_Release_Lock_Sig(system_release_lock){
    //printf("%s: id: %d\n", __PRETTY_FUNCTION__, id);
    pthread_mutex_lock(&linuxvars.locks[id].mutex);
    pthread_cond_broadcast(&linuxvars.locks[id].cond);
    pthread_mutex_unlock(&linuxvars.locks[id].mutex);
}

Sys_Cancel_Job_Sig(system_cancel_job){
    DBG_FN;
    AllowLocal(group_id);
    AllowLocal(job_id);

    Work_Queue *queue = exchange_vars.thread.queues + group_id;
    Thread_Group *group = linuxvars.groups + group_id;
    
    u32 job_index;
    u32 thread_id;
    Full_Job_Data *full_job;
    Thread_Context *thread;
    
    job_index = job_id % QUEUE_WRAP;
    full_job = queue->jobs + job_index;
    
    Assert(full_job->id == job_id);
    thread_id =
        __sync_val_compare_and_swap(&full_job->running_thread,
                                   THREAD_NOT_ASSIGNED, 0);
    
    if (thread_id != THREAD_NOT_ASSIGNED){
        system_acquire_lock(CANCEL_LOCK0 + thread_id - 1);
        thread = group->threads + thread_id - 1;
        pthread_kill(thread->handle, SIGINT); //NOTE(inso) SIGKILL if you really want it to die.
        pthread_create(&thread->handle, NULL, &ThreadProc, thread);
        system_release_lock(CANCEL_LOCK0 + thread_id - 1);
        thread->running = 0;
    }

}


Sys_Grow_Thread_Memory_Sig(system_grow_thread_memory){
    void *old_data;
    i32 old_size, new_size;
    
    system_acquire_lock(CANCEL_LOCK0 + memory->id - 1);
    old_data = memory->data;
    old_size = memory->size;
    new_size = LargeRoundUp(memory->size*2, Kbytes(4));
    memory->data = system_get_memory(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        system_free_memory(old_data);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}

INTERNAL_Sys_Sentinel_Sig(internal_sentinel){
    Bubble *result;
#if FRED_INTERNAL
    result = &linuxvars.internal_bubble;
#else
    result = 0;
#endif
    return(result);
}

INTERNAL_Sys_Get_Thread_States_Sig(internal_get_thread_states){
    // TODO(allen): Implement
    DBG_FN;
    AllowLocal(id);
    AllowLocal(running);
    AllowLocal(pending);
}

INTERNAL_Sys_Debug_Message_Sig(internal_debug_message){
    printf("%s", message);
}

DIRECTORY_HAS_FILE_SIG(system_directory_has_file){
    int result = 0;

    //TODO(inso): implement
    char buff[PATH_MAX] = {};
    memcpy(buff, dir.str, dir.size);
    printf("Has file %s\n", buff);

    return(result);
}

DIRECTORY_CD_SIG(system_directory_cd){
    int result = 0;
    // TODO(allen): Implement

    printf("Dir CD: %.*s\n", dir->size, dir->str);

    AllowLocal(dir);
    AllowLocal(rel_path);
    return(result);
}

internal
Sys_File_Can_Be_Made(system_file_can_be_made){
    // TODO(allen): Implement
    
    printf("File can be made: %s\n", filename);
    AllowLocal(filename);
    return(0);
}

internal
Sys_Load_File_Sig(system_load_file){
    Data result = {};
    struct stat info = {};
    int fd;
    u8 *ptr, *read_ptr;
    size_t bytes_to_read;
    ssize_t num;

    fd = open(filename, O_RDONLY);
    if(fd < 0){
        perror("sys_open_file: open");
        goto out;
    }
    if(fstat(fd, &info) < 0){
        perror("sys_open_file: stat");
        goto out;
    }
    if(info.st_size <= 0){
        printf("st_size < 0: %ld\n", info.st_size);
        goto out;
    }

    ptr = (u8*)LinuxGetMemory(info.st_size);
    if(!ptr){
        puts("null pointer from LGM");
        goto out;
    }

    read_ptr = ptr;
    bytes_to_read = info.st_size;

    do {
        num = read(fd, read_ptr, bytes_to_read);
        if(num < 0){
            if(errno == EINTR){
                continue;
            } else {
                //TODO(inso): error handling
                perror("sys_load_file: read");
                LinuxFreeMemory(ptr);
                goto out;
            }
        } else {
            bytes_to_read -= num;
            read_ptr += num;
        }
    } while(bytes_to_read);

    result.size = info.st_size;
    result.data = ptr;

out:
    if(fd >= 0) close(fd);
    return(result);
}

internal
Sys_Save_File_Sig(system_save_file){
    b32 result = 0;
    DBG_FN;

    const size_t save_fsz   = strlen(filename);
    const char   tmp_end[]  = ".4ed.XXXXXX";
    char*        tmp_fname  = (char*) alloca(save_fsz + sizeof(tmp_end));

    memcpy(tmp_fname, filename, save_fsz);
    memcpy(tmp_fname + save_fsz, tmp_end, sizeof(tmp_end));

    int tmp_fd = mkstemp(tmp_fname);
    if(tmp_fd == -1){
        perror("system_save_file: mkstemp");
        return result;
    }

    size_t remaining = size;
    do {
        ssize_t written = write(tmp_fd, data, size);
        if(written == -1){
            if(errno == EINTR){
                continue;
            } else {
                perror("system_save_file: write");
                unlink(tmp_fname);
                return result;
            }
        } else {
            remaining -= written;
        }
    } while(remaining);

    if(rename(tmp_fname, filename) == -1){
        perror("system_save_file: rename");
        unlink(tmp_fname);
        return result;
    }

    result = 1;
    return(result);
}

// TODO(allen): Implement this.  Also where is this
// macro define? Let's try to organize these functions
// a little better now that they're starting to settle
// into their places.

#include "system_shared.cpp"
#include "4ed_rendering.cpp"

internal
Font_Load_Sig(system_draw_font_load){
    Font_Load_Parameters *params;
    
    system_acquire_lock(FONT_LOCK);
    params = linuxvars.fnt.free_param.next;
    fnt__remove(params);
    fnt__insert(&linuxvars.fnt.used_param, params);
    system_release_lock(FONT_LOCK);

    if (linuxvars.fnt.part.base == 0){
        linuxvars.fnt.part = LinuxScratchPartition(Mbytes(8));
    }

    b32 done = 0;
    while(!(done = 
            draw_font_load(
                linuxvars.fnt.part.base,
                linuxvars.fnt.part.max,
                font_out,
                filename,
                pt_size,
                tab_width
            )
    )){
        //FIXME(inso): This is an infinite loop if the fonts aren't found!
        // Figure out how draw_font_load can fail
        
        printf("draw_font_load failed, %d\n", linuxvars.fnt.part.max);
        LinuxScratchPartitionDouble(&linuxvars.fnt.part);
    }

    system_acquire_lock(FONT_LOCK);
    fnt__remove(params);
    fnt__insert(&linuxvars.fnt.free_param, params);
    system_release_lock(FONT_LOCK);

    return(0);
}

internal
Sys_To_Binary_Path(system_to_binary_path){
    b32 translate_success = 0;
    i32 max = out_filename->memory_size;
    i32 size = readlink("/proc/self/exe", out_filename->str, max);
    if (size > 0 && size < max-1){
        out_filename->str[size] = '\0';
        out_filename->size = size + 1;
        truncate_to_path_of_directory(out_filename);
        if (append(out_filename, filename) && terminate_with_null(out_filename)){
            translate_success = 1;
        }
    }
    return (translate_success);
}

internal b32
LinuxLoadAppCode(){
    b32 result = 0;
    App_Get_Functions *get_funcs = 0;

    linuxvars.app_code = dlopen("./4ed_app.so", RTLD_LAZY);
    if (linuxvars.app_code){
        get_funcs = (App_Get_Functions*)
            dlsym(linuxvars.app_code, "app_get_functions");
    }
    
    if (get_funcs){
        result = 1;
        linuxvars.app = get_funcs();
    }
    
    return(result);
}

internal void
LinuxLoadSystemCode(){
    linuxvars.system->file_time_stamp = system_file_time_stamp;
    linuxvars.system->set_file_list = system_set_file_list;

    linuxvars.system->directory_has_file = system_directory_has_file;
    linuxvars.system->directory_cd = system_directory_cd;

    linuxvars.system->post_clipboard = system_post_clipboard;
    linuxvars.system->time = system_time;
    
    linuxvars.system->cli_call = system_cli_call;
    linuxvars.system->cli_begin_update = system_cli_begin_update;
    linuxvars.system->cli_update_step = system_cli_update_step;
    linuxvars.system->cli_end_update = system_cli_end_update;

    linuxvars.system->post_job = system_post_job;
    linuxvars.system->cancel_job = system_cancel_job;
    linuxvars.system->grow_thread_memory = system_grow_thread_memory;
    linuxvars.system->acquire_lock = system_acquire_lock;
    linuxvars.system->release_lock = system_release_lock;
    
    linuxvars.system->internal_sentinel = internal_sentinel;
    linuxvars.system->internal_get_thread_states = internal_get_thread_states;
    linuxvars.system->internal_debug_message = internal_debug_message;

    linuxvars.system->slash = '/';
}

internal void
LinuxLoadRenderCode(){
    linuxvars.target.push_clip = draw_push_clip;
    linuxvars.target.pop_clip = draw_pop_clip;
    linuxvars.target.push_piece = draw_push_piece;
    
    linuxvars.target.font_set.font_info_load = draw_font_info_load;
    linuxvars.target.font_set.font_load = system_draw_font_load;
    linuxvars.target.font_set.release_font = draw_release_font;
}

internal void
LinuxRedrawTarget(){
    system_acquire_lock(RENDER_LOCK);
    launch_rendering(&linuxvars.target);
    system_release_lock(RENDER_LOCK);
//    glFlush();
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
        linuxvars.redraw = 1;
    }
}

// NOTE(allen): Thanks to Casey for providing the linux OpenGL launcher.
static bool ctxErrorOccurred = false;
static int XInput2OpCode = 0;
internal int
ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

#if FRED_INTERNAL
static void gl_log(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
){
    printf("GL DEBUG: %s\n", message);
}
#endif

internal GLXContext
InitializeOpenGLContext(Display *XDisplay, Window XWindow, GLXFBConfig &bestFbc, b32 &IsLegacy)
{
    IsLegacy = false;
    
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    const char *glxExts = glXQueryExtensionsString(XDisplay, DefaultScreen(XDisplay));
    
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
        glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
 
    GLXContext ctx = 0;
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) =
        XSetErrorHandler(&ctxErrorHandler);
    if (!glXCreateContextAttribsARB)
    {
        printf( "glXCreateContextAttribsARB() not found"
                " ... using old-style GLX context\n" );
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

        printf("Attribs: %d %d %d %d %d\n",
               context_attribs[0],
               context_attribs[1],
               context_attribs[2],
               context_attribs[3],
               context_attribs[4]);
 
        printf( "Creating context\n" );
        ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0,
                                          True, context_attribs );
 
        XSync( XDisplay, False );
        if ( !ctxErrorOccurred && ctx )
        {
            printf( "Created GL 4.3 context\n" );
        }
        else
        {
            ctxErrorOccurred = false;

            context_attribs[1] = 4;
            context_attribs[3] = 0;
 
            printf( "Creating context\n" );
            ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0,
                                              True, context_attribs );
 
            XSync( XDisplay, False );

            if ( !ctxErrorOccurred && ctx )
            {
                printf( "Created GL 4.0 context\n" );
            }
            else
            {
                context_attribs[1] = 1;
                context_attribs[3] = 0;
 
                ctxErrorOccurred = false;
 
                printf( "Failed to create GL 4.0 context"
                        " ... using old-style GLX context\n" );
                ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0, 
                                                  True, context_attribs );

                IsLegacy = true;
            }
        }
    }
 
    XSync( XDisplay, False );
    XSetErrorHandler( oldHandler );
 
    if ( ctxErrorOccurred || !ctx )
    {
        printf( "Failed to create an OpenGL context\n" );
        exit(1);
    }
 
    if ( ! glXIsDirect ( XDisplay, ctx ) )
    {
        printf( "Indirect GLX rendering context obtained\n" );
    }
    else
    {
        printf( "Direct GLX rendering context obtained\n" );
    }
    
    printf( "Making context current\n" );
    glXMakeCurrent( XDisplay, XWindow, ctx );
    
    GLint n;
    char *Vendor = (char *)glGetString(GL_VENDOR);
    char *Renderer = (char *)glGetString(GL_RENDERER);
    char *Version = (char *)glGetString(GL_VERSION);

    //TODO(inso): glGetStringi is required if the GL version is >= 3.0
    char *Extensions = (char *)glGetString(GL_EXTENSIONS);

    printf("GL_VENDOR: %s\n", Vendor);
    printf("GL_RENDERER: %s\n", Renderer);
    printf("GL_VERSION: %s\n", Version);
    printf("GL_EXTENSIONS: %s\n", Extensions);

    //TODO(inso): this should be optional
    if(strstr(glxExts, "GLX_EXT_swap_control ")){
        PFNGLXSWAPINTERVALEXTPROC glx_swap_interval =
        (PFNGLXSWAPINTERVALEXTPROC) glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");

        if(glx_swap_interval){
            glx_swap_interval(XDisplay, XWindow, 1);

            unsigned int swap_val = 0;
            glXQueryDrawable(XDisplay, XWindow, GLX_SWAP_INTERVAL_EXT, &swap_val);
            linuxvars.vsync = swap_val == 1;
            printf("VSync enabled? %d\n", linuxvars.vsync);
        }
    }

#if FRED_INTERNAL
    PFNGLDEBUGMESSAGECALLBACKARBPROC gl_dbg_callback = (PFNGLDEBUGMESSAGECALLBACKARBPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageCallback");
    if(gl_dbg_callback){
        puts("enabling gl debug");
        gl_dbg_callback(&gl_log, 0);
        glEnable(GL_DEBUG_OUTPUT);
    }
#endif

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return(ctx);
}

internal b32
GLXSupportsModernContexts(Display *XDisplay)
{
    b32 Result = false;
    
    int GLXMajor, GLXMinor;

    char *XVendor = ServerVendor(XDisplay);
    printf("XWindows vendor: %s\n", XVendor);
    if(glXQueryVersion(XDisplay, &GLXMajor, &GLXMinor))
    {
        printf("GLX version %d.%d\n", GLXMajor, GLXMinor);
        if(((GLXMajor == 1 ) && (GLXMinor >= 3)) ||
           (GLXMajor > 1))
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

    {
        int ConfigCount;
        GLXFBConfig *Configs = glXChooseFBConfig(XDisplay,
                                                 XScreenIndex,
                                                 DesiredAttributes,
                                                 &ConfigCount);

#if 0
        int DiffValues[GLXValueCount];
#endif
        {for(int ConfigIndex = 0;
             ConfigIndex < ConfigCount;
             ++ConfigIndex)
        {
            GLXFBConfig &Config = Configs[ConfigIndex];
            XVisualInfo *VisualInfo = glXGetVisualFromFBConfig(XDisplay, Config);

#if 0
            printf("  Option %d:\n", ConfigIndex);
            printf("    Depth: %d\n", VisualInfo->depth);
            printf("    Bits per channel: %d\n", VisualInfo->bits_per_rgb);
            printf("    Mask: R%06x G%06x B%06x\n",
                   (uint32)VisualInfo->red_mask,
                   (uint32)VisualInfo->green_mask,
                   (uint32)VisualInfo->blue_mask);
            printf("    Class: %d\n", VisualInfo->c_class);
#endif
            
#if 0
            {for(int ValueIndex = 0;
                 ValueIndex < GLXValueCount;
                 ++ValueIndex)
            {
                glx_value_info &ValueInfo = GLXValues[ValueIndex];
                int Value;
                glXGetFBConfigAttrib(XDisplay, Config, ValueInfo.ID, &Value);
                if(DiffValues[ValueIndex] != Value)
                {
                    printf("    %s: %d\n", ValueInfo.Name, Value);
                    DiffValues[ValueIndex] = Value;
                }
            }}
#endif
            
            // TODO(casey): How the hell are you supposed to pick a config here??
            if(ConfigIndex == 0)
            {
                Result.Found = true;
                Result.BestConfig = Config;
                Result.BestInfo = *VisualInfo;
            }
                        
            XFree(VisualInfo);
        }}
                    
        XFree(Configs);
    }

    return(Result);
}

struct Init_Input_Result{
    XIM input_method;
    XIMStyle best_style;
    XIC xic;
};

internal Init_Input_Result
InitializeXInput(Display *dpy, Window XWindow)
{
#if 0
    int event, error;
    if(XQueryExtension(dpy, "XInputExtension", &XInput2OpCode, &event, &error))
    {
        int major = 2, minor = 0;
        if(XIQueryVersion(dpy, &major, &minor) != BadRequest)
        {
            printf("XInput initialized version %d.%d\n", major, minor);
        }
        else
        {
            printf("XI2 not available. Server supports %d.%d\n", major, minor);
        }
    }
    else
    {
        printf("X Input extension not available.\n");
    }

    /*
      TODO(casey): So, this is all one big clusterfuck I guess.

      The problem here is that you want to be able to get input
      from all possible devices that could be a mouse or keyboard
      (or gamepad, or whatever).  So you'd like to be able to
      register events for XIAllDevices, so that when a new
      input device is connected, you will start receiving
      input from it automatically, without having to periodically
      poll XIQueryDevice to see if a new device has appeared.

      UNFORTUNATELY, this is not actually possible in Linux because
      there was a bug in Xorg (as of early 2013, it is still not
      fixed in most distributions people are using, AFAICT) which
      makes the XServer return an error if you actually try to
      do this :(

      But EVENTUALLY, if that shit gets fixed, then that is
      the way this should work.
    */

#if 0
    int DeviceCount;
    XIDeviceInfo *DeviceInfo = XIQueryDevice(dpy, XIAllDevices, &DeviceCount);
    {for(int32x DeviceIndex = 0;
         DeviceIndex < DeviceCount;
         ++DeviceIndex)
    {
        XIDeviceInfo *Device = DeviceInfo + DeviceIndex;
        printf("Device %d: %s\n", Device->deviceid, Device->name);
    }}
    XIFreeDeviceInfo(DeviceInfo);
#endif
    
    XIEventMask Mask = {0};
    Mask.deviceid = XIAllDevices;
    Mask.mask_len = XIMaskLen(XI_RawMotion);
    size_t MaskSize = Mask.mask_len * sizeof(char unsigned);
    Mask.mask = (char unsigned *)alloca(MaskSize);
    memset(Mask.mask, 0, MaskSize);
    if(Mask.mask)
    {
        XISetMask(Mask.mask, XI_ButtonPress);
        XISetMask(Mask.mask, XI_ButtonRelease);
        XISetMask(Mask.mask, XI_KeyPress);
        XISetMask(Mask.mask, XI_KeyRelease);
        XISetMask(Mask.mask, XI_Motion);
        XISetMask(Mask.mask, XI_DeviceChanged);
        XISetMask(Mask.mask, XI_Enter);
        XISetMask(Mask.mask, XI_Leave);
        XISetMask(Mask.mask, XI_FocusIn);
        XISetMask(Mask.mask, XI_FocusOut);
        XISetMask(Mask.mask, XI_HierarchyChanged);
        XISetMask(Mask.mask, XI_PropertyEvent);
        XISelectEvents(dpy, XWindow, &Mask, 1);
        XSync(dpy, False);

        Mask.deviceid = XIAllMasterDevices;
        memset(Mask.mask, 0, MaskSize);
        XISetMask(Mask.mask, XI_RawKeyPress);
        XISetMask(Mask.mask, XI_RawKeyRelease);
        XISetMask(Mask.mask, XI_RawButtonPress);
        XISetMask(Mask.mask, XI_RawButtonRelease);
        XISetMask(Mask.mask, XI_RawMotion);
        XISelectEvents(dpy, DefaultRootWindow(dpy), &Mask, 1);
    }
#endif
    
    // NOTE(allen): Annnndddd... here goes some guess work of my own.
    Init_Input_Result result = {};
    XIMStyle style;
    XIMStyles *styles = 0;
    i32 i, count;

	setlocale(LC_ALL, "");
    XSetLocaleModifiers("");
	printf("is current locale supported?: %d\n", XSupportsLocale());
    // TODO(inso): handle the case where it isn't supported somehow?

    XSelectInput(
        linuxvars.XDisplay,
        linuxvars.XWindow,
        ExposureMask |
        KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask |
        EnterWindowMask | LeaveWindowMask |
        PointerMotionMask |
        FocusChangeMask |
        StructureNotifyMask |
        MappingNotify |
        ExposureMask
    );

    result.input_method = XOpenIM(dpy, 0, 0, 0);
    if (!result.input_method){
        // NOTE(inso): Try falling back to the internal XIM implementation that
        //            should in theory always exist.

        XSetLocaleModifiers("@im=none");
        result.input_method = XOpenIM(dpy, 0, 0, 0);
    }

    if (result.input_method){

        if (!XGetIMValues(result.input_method, XNQueryInputStyle, &styles, NULL) &&
            styles){
            result.best_style = 0;
            count = styles->count_styles;
            for (i = 0; i < count; ++i){
                style = styles->supported_styles[i];
                if (style == (XIMPreeditNothing | XIMStatusNothing)){
                    result.best_style = style;
                    break;
                }
            }
            
            if (result.best_style){
                XFree(styles);

                result.xic =
                    XCreateIC(result.input_method, XNInputStyle, result.best_style,
                              XNClientWindow, XWindow,
                              XNFocusWindow, XWindow,
                              0, 0,
                              NULL);
            }
            else{
                result = {};
                puts("Could not get minimum required input style");
            }
        }
    }
    else{
        result = {};
        puts("Could not open X Input Method");
    }

    return(result);
}

static void push_key(u8 code, u8 chr, u8 chr_nocaps, b8 (*mods)[CONTROL_KEY_COUNT], b32 is_hold){
    i32 *count;
    Key_Event_Data *data;

    if(is_hold){
        count = &linuxvars.key_data.hold_count;
        data = linuxvars.key_data.hold;
    } else {
        count = &linuxvars.key_data.press_count;
        data = linuxvars.key_data.press;
    }

	if(*count < KEY_INPUT_BUFFER_SIZE){
		data[*count].keycode = code;
		data[*count].character = chr;
		data[*count].character_no_caps_lock = chr_nocaps;

		memcpy(data[*count].modifiers, *mods, sizeof(*mods));

		++(*count);
	}
}

int
main(int argc, char **argv)
{
    i32 COUNTER = 0;
    linuxvars = {};
    exchange_vars = {};
    
#if FRED_INTERNAL
    linuxvars.internal_bubble.next = &linuxvars.internal_bubble;
    linuxvars.internal_bubble.prev = &linuxvars.internal_bubble;
    linuxvars.internal_bubble.flags = MEM_BUBBLE_SYS_DEBUG;
#endif
    
    if (!LinuxLoadAppCode()){
        // TODO(allen): Failed to load app code, serious problem.
        return 99;
    }
    
    System_Functions system_;
    System_Functions *system = &system_;
    linuxvars.system = system;
    LinuxLoadSystemCode();
    
	memory_vars.vars_memory_size = Mbytes(2);
    memory_vars.vars_memory = system_get_memory(memory_vars.vars_memory_size);
    memory_vars.target_memory_size = Mbytes(512);
    memory_vars.target_memory = system_get_memory(memory_vars.target_memory_size);
    
    String current_directory;
    i32 curdir_req, curdir_size;
    char *curdir_mem;
    
    curdir_req = (1 << 9);
    curdir_mem = (char*)system_get_memory(curdir_req);
    for (; getcwd(curdir_mem, curdir_req) == 0 && curdir_req < (1 << 13);){
        system_free_memory(curdir_mem);
        curdir_req *= 4;
        curdir_mem = (char*)system_get_memory(curdir_req);
    }
    
    if (curdir_req >= (1 << 13)){
        // TODO(allen): bullshit string APIs makin' me pissed
        return 57;
    }
   
    for (curdir_size = 0; curdir_mem[curdir_size]; ++curdir_size);
    
    current_directory = make_string(curdir_mem, curdir_size, curdir_req);
    
    Command_Line_Parameters clparams;
    clparams.argv = argv;
    clparams.argc = argc;
    
    char **files;
    i32 *file_count;
    i32 output_size;

    output_size =
        linuxvars.app.read_command_line(system,
                                        &memory_vars,
                                        current_directory,
                                        &linuxvars.settings,
                                        &files, &file_count,
                                        clparams);
    
    if (output_size > 0){
        // TODO(allen): crt free version
        printf("%.*s", output_size, (char*)memory_vars.target_memory);
    }
    if (output_size != 0) return 0;

    sysshared_filter_real_files(files, file_count);
    
    linuxvars.XDisplay = XOpenDisplay(0);

    keycode_init(linuxvars.XDisplay, &linuxvars.key_codes);

#ifdef FRED_SUPER
    char *custom_file_default = "4coder_custom.so";
    char *custom_file;
    if (linuxvars.settings.custom_dll) custom_file = linuxvars.settings.custom_dll;
    else custom_file = custom_file_default;
    
    linuxvars.custom = dlopen(custom_file, RTLD_LAZY);
    if (!linuxvars.custom && custom_file != custom_file_default){
        if (!linuxvars.settings.custom_dll_is_strict){
            linuxvars.custom = dlopen(custom_file_default, RTLD_LAZY);
        }
    }
    
    if (linuxvars.custom){
        linuxvars.custom_api.get_bindings = (Get_Binding_Data_Function*)
            dlsym(linuxvars.custom, "get_bindings");
    }
#endif
    
    // TODO(allen): Setup background threads and locks

    Thread_Context background[4] = {};
    linuxvars.groups[BACKGROUND_THREADS].threads = background;
    linuxvars.groups[BACKGROUND_THREADS].count = ArrayCount(background);

    Thread_Memory thread_memory[ArrayCount(background)];
    linuxvars.thread_memory = thread_memory;

    pthread_mutex_init(&linuxvars.thread_locks[BACKGROUND_THREADS].mutex, NULL);
    pthread_cond_init(&linuxvars.thread_locks[BACKGROUND_THREADS].cond, NULL);

    exchange_vars.thread.queues[BACKGROUND_THREADS].semaphore = 
        LinuxSemToHandle(&linuxvars.thread_locks[BACKGROUND_THREADS]);

    for(i32 i = 0; i < linuxvars.groups[BACKGROUND_THREADS].count; ++i){
        Thread_Context *thread = linuxvars.groups[BACKGROUND_THREADS].threads + i;
        thread->id = i + 1;

        Thread_Memory *memory = linuxvars.thread_memory + i;
        *memory = {};
        memory->id = thread->id;

        thread->queue = &exchange_vars.thread.queues[BACKGROUND_THREADS];
        pthread_create(&thread->handle, NULL, &ThreadProc, thread);
    }

    Assert(linuxvars.locks);
    for(i32 i = 0; i < LOCK_COUNT; ++i){
        pthread_mutex_init(&linuxvars.locks[i].mutex, NULL);
        pthread_cond_init(&linuxvars.locks[i].cond, NULL);
    }

    LinuxLoadRenderCode();
    linuxvars.target.max = Mbytes(1);
    linuxvars.target.push_buffer = (byte*)system_get_memory(linuxvars.target.max);
    
    File_Slot file_slots[32];
    sysshared_init_file_exchange(&exchange_vars, file_slots, ArrayCount(file_slots), 0);
    
    Font_Load_Parameters params[8];
    sysshared_init_font_params(&linuxvars.fnt, params, ArrayCount(params));
    

    
    // NOTE(allen): Here begins the linux screen setup stuff.
    // Behold the true nature of this wonderful OS:
    // (thanks again to Casey for providing this stuff)
    Colormap cmap;
    XSetWindowAttributes swa;
    int WinWidth, WinHeight;
    b32 window_setup_success = 0;

    WinWidth = 800;
    WinHeight = 600;

    if(linuxvars.XDisplay && GLXSupportsModernContexts(linuxvars.XDisplay))
    {
        int XScreenCount = ScreenCount(linuxvars.XDisplay);
        for(int XScreenIndex = 0;
            XScreenIndex < XScreenCount;
            ++XScreenIndex)
        {
            Screen *XScreen = ScreenOfDisplay(linuxvars.XDisplay, XScreenIndex);

            i32 ScrnWidth, ScrnHeight;
            ScrnWidth = WidthOfScreen(XScreen);
            ScrnHeight = HeightOfScreen(XScreen);
            
            if (ScrnWidth + 50 < WinWidth) WinWidth = ScrnWidth + 50;
            if (ScrnHeight + 50 < WinHeight) WinHeight = ScrnHeight + 50;
            
            glx_config_result Config = ChooseGLXConfig(linuxvars.XDisplay, XScreenIndex);
            if(Config.Found)
            {
                swa.colormap = cmap = XCreateColormap(linuxvars.XDisplay,
                                                      RootWindow(linuxvars.XDisplay, Config.BestInfo.screen ), 
                                                      Config.BestInfo.visual, AllocNone);
                swa.background_pixmap = None;
                swa.border_pixel = 0;
                swa.event_mask = StructureNotifyMask;
 
                linuxvars.XWindow =
                    XCreateWindow(linuxvars.XDisplay,
                                  RootWindow(linuxvars.XDisplay, Config.BestInfo.screen),
                                  0, 0, WinWidth, WinHeight,
                                  0, Config.BestInfo.depth, InputOutput, 
                                  Config.BestInfo.visual, 
                                  CWBorderPixel|CWColormap|CWEventMask, &swa );
                
                if(linuxvars.XWindow)
                {
                    XStoreName(linuxvars.XDisplay, linuxvars.XWindow, "4coder-window");
                    XMapWindow(linuxvars.XDisplay, linuxvars.XWindow);

                    Init_Input_Result input_result = 
                        InitializeXInput(linuxvars.XDisplay, linuxvars.XWindow);

                    linuxvars.input_method = input_result.input_method;
                    linuxvars.input_style = input_result.best_style;
                    linuxvars.input_context = input_result.xic;

                    b32 IsLegacy = false;
                    GLXContext GLContext =
                        InitializeOpenGLContext(linuxvars.XDisplay, linuxvars.XWindow, Config.BestConfig, IsLegacy);

                    XWindowAttributes WinAttribs;
                    if(XGetWindowAttributes(linuxvars.XDisplay, linuxvars.XWindow, &WinAttribs))
                    {
                        WinWidth = WinAttribs.width;
                        WinHeight = WinAttribs.height;
                    }
                    
                    XRaiseWindow(linuxvars.XDisplay, linuxvars.XWindow);
                    XSync(linuxvars.XDisplay, False);

                    window_setup_success = 1;
                }
            }
        }
    }
    
    if (!window_setup_success){
        fprintf(stderr, "Error creating window.");
        exit(1);
    }

    XSetICFocus(linuxvars.input_context);

    linuxvars.atom_CLIPBOARD = XInternAtom(linuxvars.XDisplay, "CLIPBOARD", False);
    linuxvars.atom_UTF8_STRING = XInternAtom(linuxvars.XDisplay, "UTF8_STRING", False);

    Atom WM_DELETE_WINDOW = XInternAtom(linuxvars.XDisplay, "WM_DELETE_WINDOW", False);
    if(WM_DELETE_WINDOW != None){
        XSetWMProtocols(linuxvars.XDisplay, linuxvars.XWindow, &WM_DELETE_WINDOW, 1);
    }

    linuxvars.app.init(linuxvars.system, &linuxvars.target,
                       &memory_vars, &exchange_vars, &linuxvars.key_codes,
                       linuxvars.clipboard_contents, current_directory,
                       linuxvars.custom_api);

    LinuxResizeTarget(WinWidth, WinHeight);
    b32 keep_running = 1;

    while(keep_running)
    {
        XEvent PrevEvent = {};

        while(XPending(linuxvars.XDisplay))
        {
            XEvent Event;
            XNextEvent(linuxvars.XDisplay, &Event);

            if (XFilterEvent(&Event, None) == True){
                continue;
            }

            switch (Event.type){
                case KeyPress: {
                    b32 is_hold =
                    PrevEvent.type == KeyRelease &&
                    PrevEvent.xkey.time == Event.xkey.time &&
                    PrevEvent.xkey.keycode == Event.xkey.keycode;

                    b8 mods[CONTROL_KEY_COUNT] = {};
                    if(Event.xkey.state & ShiftMask) mods[CONTROL_KEY_SHIFT] = 1;
                    if(Event.xkey.state & ControlMask) mods[CONTROL_KEY_CONTROL] = 1;
                    if(Event.xkey.state & LockMask) mods[CONTROL_KEY_CAPS] = 1;
                    if(Event.xkey.state & Mod1Mask) mods[CONTROL_KEY_ALT] = 1;
                    // NOTE(inso): mod5 == AltGr
                    // if(Event.xkey.state & Mod5Mask) mods[CONTROL_KEY_ALT] = 1;

                    KeySym keysym = NoSymbol;
                    char buff[32], no_caps_buff[32];

                    // NOTE(inso): Turn ControlMask off like the win32 code does.
                    if(mods[CONTROL_KEY_CONTROL] && !mods[CONTROL_KEY_ALT]){
                        Event.xkey.state &= ~(ControlMask);
                    }

                    // TODO(inso): Use one of the Xutf8LookupString funcs to allow for non-ascii chars
                    XLookupString(&Event.xkey, buff, sizeof(buff), &keysym, NULL);

                    Event.xkey.state &= ~LockMask;
                    XLookupString(&Event.xkey, no_caps_buff, sizeof(no_caps_buff), NULL, NULL);

                    u8 key = keycode_lookup(Event.xkey.keycode);

                    if(key){
                        push_key(key, 0, 0, &mods, is_hold);
                    } else {
                            key = buff[0] & 0xFF;
                            if(key < 128){
                                u8 no_caps_key = no_caps_buff[0] & 0xFF;
                                if(key == '\r') key = '\n';
                                if(no_caps_key == '\r') no_caps_key = '\n';
                                push_key(key, key, no_caps_key, &mods, is_hold);
                            } else {
                                push_key(0, 0, 0, &mods, is_hold);
                            }
                    }
                }break;

                case MotionNotify: {
                    linuxvars.mouse_data.x = Event.xmotion.x;
                    linuxvars.mouse_data.y = Event.xmotion.y;
                }break;

                case ButtonPress: {
                    switch(Event.xbutton.button){
                        case Button1: {
                            linuxvars.mouse_data.left_button_pressed = 1;
                            linuxvars.mouse_data.left_button = 1;
                        } break;
                        case Button3: {
                            linuxvars.mouse_data.right_button_pressed = 1;
                            linuxvars.mouse_data.right_button = 1;
                        } break;
                    }
                }break;

                case ButtonRelease: {
                    switch(Event.xbutton.button){
                        case Button1: {
                            linuxvars.mouse_data.left_button_released = 1;
                            linuxvars.mouse_data.left_button = 0;
                        } break;
                        case Button3: {
                            linuxvars.mouse_data.right_button_released = 1;
                            linuxvars.mouse_data.right_button = 0;
                        } break;
                    }
                }break;

                case EnterNotify: {
                    linuxvars.mouse_data.out_of_window = 0;
                }break;

                case LeaveNotify: {
                    linuxvars.mouse_data.out_of_window = 1;
                }break;

                case FocusIn:
                case FocusOut: {
                    linuxvars.mouse_data.left_button = 0;
                    linuxvars.mouse_data.right_button = 0;
                }break;

                case ConfigureNotify: {
                    i32 w = Event.xconfigure.width, h = Event.xconfigure.height;

                    if(w != linuxvars.target.width || h != linuxvars.target.height){
                        LinuxResizeTarget(Event.xconfigure.width, Event.xconfigure.height);
                    }
                }break;

                case MappingNotify: {
                    if(Event.xmapping.request == MappingModifier || Event.xmapping.request == MappingKeyboard){
                        XRefreshKeyboardMapping(&Event.xmapping);
                        keycode_init(linuxvars.XDisplay, &linuxvars.key_codes);
                    }
                }break;

                case ClientMessage: {
                    if ((Atom)Event.xclient.data.l[0] == WM_DELETE_WINDOW) {
                        keep_running = false;
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

                    //TODO(inso): handle TARGETS negotiation instead of requiring UTF8_STRING
                    if (
                        linuxvars.clipboard_outgoing.size &&
                        request.target == linuxvars.atom_UTF8_STRING &&
                        request.selection == linuxvars.atom_CLIPBOARD &&
                        request.property != None &&
                        request.display &&
                        request.requestor
                    ){
                        XChangeProperty(
                            request.display,
                            request.requestor,
                            request.property,
                            request.target,
                            8,
                            PropModeReplace,
                            (unsigned char*)linuxvars.clipboard_outgoing.str,
                            linuxvars.clipboard_outgoing.size
                        );

                        response.property = request.property;
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
                    if(
                        e->selection == linuxvars.atom_CLIPBOARD &&
                        e->target == linuxvars.atom_UTF8_STRING &&
                        e->property != None
                    ){
                        Atom type;
                        int fmt;
                        unsigned long nitems, bytes_left;
                        u8 *data;

                        int result = XGetWindowProperty(
                            linuxvars.XDisplay,
                            linuxvars.XWindow,
                            linuxvars.atom_CLIPBOARD,
                            0L,
                            LINUX_MAX_PASTE_CHARS/4L,
                            False,
                            linuxvars.atom_UTF8_STRING,
                            &type,
                            &fmt,
                            &nitems,
                            &bytes_left,
                            &data
                        );

                        if(result == Success && fmt == 8){
                            LinuxStringDup(&linuxvars.clipboard_contents, data, nitems);
                            XFree(data);
                        }
                    }
                }break;

                case Expose: {
                    linuxvars.redraw = 1;
                }break;
            }

            PrevEvent = Event;
        }

        // FIXME(inso): is getting the clipboard every frame a bad idea?
        XConvertSelection(
            linuxvars.XDisplay,
            linuxvars.atom_CLIPBOARD,
            linuxvars.atom_UTF8_STRING,
            linuxvars.atom_CLIPBOARD,
            linuxvars.XWindow,
            CurrentTime
        );

        Key_Input_Data input_data;
        Mouse_State mouse;
        Application_Step_Result result;

        input_data = linuxvars.key_data;
        mouse = linuxvars.mouse_data;

        result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;

        if(__sync_bool_compare_and_swap(&exchange_vars.thread.force_redraw, 1, 0)){
            linuxvars.redraw = 1;
        }

        result.redraw = linuxvars.redraw;
        result.lctrl_lalt_is_altgr = 0;

        u64 start_time = system_time();

        linuxvars.app.step(linuxvars.system,
                           &linuxvars.key_codes,
                           &input_data,
                           &mouse,
                           &linuxvars.target,
                           &memory_vars,
                           &exchange_vars,
                           linuxvars.clipboard_contents,
                           1, linuxvars.first, linuxvars.redraw,
                           &result);

        if (result.redraw){
            LinuxRedrawTarget();
        }

        u64 time_diff = system_time() - start_time;
        if(time_diff < frame_useconds){
            usleep(frame_useconds - time_diff);
        }

        linuxvars.redraw = 0;
        linuxvars.key_data = {};
        linuxvars.mouse_data.left_button_pressed = 0;
        linuxvars.mouse_data.left_button_released = 0;
        linuxvars.mouse_data.right_button_pressed = 0;
        linuxvars.mouse_data.right_button_released = 0;

        ProfileStart(OS_file_process);
        {
            File_Slot *file;
            int d = 0;
            
            for (file = exchange_vars.file.active.next;
                 file != &exchange_vars.file.active;
                 file = file->next){
                ++d;
                
                if (file->flags & FEx_Save){
                    Assert((file->flags & FEx_Request) == 0);
                    file->flags &= (~FEx_Save);
                    if (system_save_file(file->filename, (char*)file->data, file->size)){
                        file->flags |= FEx_Save_Complete;
                    }
                    else{
                        file->flags |= FEx_Save_Failed;
                    }
                }
                
                if (file->flags & FEx_Request){
                    Assert((file->flags & FEx_Save) == 0);
                    file->flags &= (~FEx_Request);
                    Data sysfile =
                        system_load_file(file->filename);
                    if (sysfile.data == 0){
                        file->flags |= FEx_Not_Exist;
                    }
                    else{
                        file->flags |= FEx_Ready;
                        file->data = sysfile.data;
                        file->size = sysfile.size;
                    }
                }
            }
            
            Assert(d == exchange_vars.file.num_active);
            
            for (file = exchange_vars.file.free_list.next;
                 file != &exchange_vars.file.free_list;
                 file = file->next){
                if (file->data){
                    system_free_memory(file->data);
                }
            }

            if (exchange_vars.file.free_list.next != &exchange_vars.file.free_list){
                ex__insert_range(exchange_vars.file.free_list.next, exchange_vars.file.free_list.prev,
                                 &exchange_vars.file.available);
            }

            ex__check(&exchange_vars.file);
        }
        ProfileEnd(OS_file_process);
    }
}

// BOTTOM

