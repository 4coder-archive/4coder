/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.11.2015
 *
 * Linux layer for project codename "4ed"
 *
 */

// TOP

#ifdef FRED_NOT_PACKAGE

#define FRED_INTERNAL 1
#define FRED_SLOW 1

#define FRED_PRINT_DEBUG 1
#define FRED_PRINT_DEBUG_FILE_LINE 0
#define FRED_PROFILING 1
#define FRED_PROFILING_OS 0
#define FRED_FULL_ERRORS 0

#else

#define FRED_SLOW 0
#define FRED_INTERNAL 0

#define FRED_PRINT_DEBUG 0
#define FRED_PRINT_DEBUG_FILE_LINE 0
#define FRED_PROFILING 0
#define FRED_PROFILING_OS 0
#define FRED_FULL_ERRORS 0

#endif

#define SOFTWARE_RENDER 0

#if FRED_INTERNAL == 0
# undef FRED_PRINT_DEBUG
# define FRED_PRINT_DEBUG 0
# undef FRED_PROFILING
# define FRED_PROFILING 0
# undef FRED_PROFILING_OS
# define FRED_PROFILING_OS 0
#endif

#if FRED_PRINT_DEBUG == 0
# undef FRED_PRINT_DEBUG_FILE_LINE
# define FRED_PRINT_DEBUG_FILE_LINE 0
# undef FRED_PRINT_DEBUG_FILE_LINE
# define FRED_PROFILING_OS 0
#endif

#define FPS 30
#define frame_useconds (1000000 / FPS)

#include "4ed_meta.h"

#define FCPP_FORBID_MALLOC

#include "4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4cpp_string.h"

#include "4ed_mem.cpp"

#include "4ed_math.cpp"
#include "4coder_custom.h"
#include "4ed_system.h"
#include "4ed.h"
#include "4ed_rendering.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/gl.h>

#include <stdio.h>

#include "4ed_internal.h"
#include "4ed_linux_keyboard.cpp"

#define printe(m) printf("%s:%d: %s\n", __FILE__, __LINE__, #m)

struct Linux_Vars{
    Key_Codes codes;
    Key_Input_Data input;
    Mouse_State mouse;
    Render_Target target;
    Application_Memory mem;
    b32 keep_going;
    b32 force_redraw;
    
    Clipboard_Contents clipboard;

    void *app_code, *custom;
    
    System_Functions *system;
    App_Functions app;
    Config_API config_api;
    
};

Linux_Vars linuxvars;

internal
Sys_Get_Memory_Sig(system_get_memory_){
    void *ptr = 0;
    i32 prot = PROT_READ | PROT_WRITE;
    i32 flags = MAP_PRIVATE | MAP_ANON;

#if FRED_INTERNAL
    ptr = mmap(0, size + sizeof(Sys_Bubble), prot, flags, -1, 0);

    Sys_Bubble *bubble = (Sys_Bubble*)ptr;
    bubble->flags = MEM_BUBBLE_SYS_DEBUG;
    bubble->line_number = line_number;
    bubble->file_name = file_name;
    bubble->size = size;
    //WaitForSingleObject(win32vars.DEBUG_sysmem_lock, INFINITE);
    insert_bubble(&win32vars.internal_bubble, bubble);
    //ReleaseSemaphore(win32vars.DEBUG_sysmem_lock, 1, 0);
    ptr = bubble + 1;
#else
    ptr = mmap(0, size + sizeof(i32), prot, flags, -1, 0);
    
    i32 *size_ptr = (i32*)ptr;
    *size_ptr = size;
    ptr = size_ptr + 1;
#endif
    
    return ptr;
}

#define system_get_memory(size) system_get_memory_(size, __LINE__, __FILE__)

internal
Sys_Free_Memory_Sig(system_free_memory){
    if (block){
#if FRED_INTERNAL
        Sys_Bubble *bubble = ((Sys_Bubble*)block) - 1;
        Assert((bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_SYS_DEBUG);
        //WaitForSingleObject(win32vars.DEBUG_sysmem_lock, INFINITE);
        remove_bubble(bubble);
        //ReleaseSemaphore(win32vars.DEBUG_sysmem_lock, 1, 0);
        munmap(bubble, bubble->size);
#else
        i32 *size_ptr = (i32*)block - 1;
        munmap(size_ptr, *size_ptr);
#endif
    }
}

internal
Sys_Time_Sig(system_time){
    i64 result = 0;
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0){
        result = tp.tv_sec*1000000 + tp.tv_nsec/1000;
    }
    return result;
}

internal
Sys_Load_File_Sig(system_load_file){
    File_Data result = {};
    i32 prot = PROT_READ;
    i32 flags = MAP_PRIVATE | MAP_ANON;
    
    struct stat sb;
    if (stat(filename, &sb) == 0){
        result.size = sb.st_size;
        result.data = system_get_memory(result.size);
        if (result.data){
            i32 file = open(filename, O_RDONLY);
            i32 result_size = read(file, result.data, result.size);
            Assert(result_size == result.size);
            close(file);
        }
    }
    
    return result;
}

internal
Sys_Save_File_Sig(system_save_file){
    i32 file = open(filename, O_CREAT | O_WRONLY);
    if (!file) return 0;

    i32 result_size = write(file, data, size);
    Assert(result_size == size);

    close(file);
    return 1;
}

internal b32
LinuxLoadAppCode(){
    b32 result = 0;
    
    char app_code_file[] = "4ed_app.so";
    i32 app_code_file_len = sizeof(app_code_file) - 1;
    
    char path[1024];
    i32 size = readlink("/proc/self/exe", path,
                        1024 - app_code_file_len - 1);
    
    for (--size;
         path[size] != '/' && size > 0;
         --size);
    memcpy(path + size + 1, app_code_file, app_code_file_len + 1);
    
    linuxvars.app_code = 0;
    if (linuxvars.app_code){
        result = 1;
        linuxvars.app.init = (App_Init*)0;
        linuxvars.app.step = (App_Step*)0;
    }
    else{
        // TODO(allen): initialization failure
        printe(app_code);
    }
    
    return result;
}

internal
Sys_Acquire_Lock_Sig(system_acquire_lock){
    AllowLocal(id);
}

internal
Sys_Release_Lock_Sig(system_release_lock){
    AllowLocal(id);
}

internal
Sys_Force_Redraw_Sig(system_force_redraw){
    linuxvars.force_redraw = 1;
}

internal void
LinuxLoadSystemCode(){
    linuxvars.system->get_memory_full = system_get_memory_;
    linuxvars.system->free_memory = system_free_memory;

    linuxvars.system->load_file = system_load_file;
    linuxvars.system->save_file = system_save_file;
    
#if 0
    linuxvars.system->file_time_stamp = system_file_time_stamp;
    linuxvars.system->time_stamp_now = system_time_stamp_now;
    linuxvars.system->free_file = system_free_file;

    linuxvars.system->get_current_directory = system_get_current_directory;
    linuxvars.system->get_easy_directory = system_get_easy_directory;
    
    linuxvars.system->get_file_list = system_get_file_list;
    linuxvars.system->free_file_list = system_free_file_list;

    linuxvars.system->post_clipboard = system_post_clipboard;
    linuxvars.system->time = system_time;
    
    linuxvars.system->cli_call = system_cli_call;
    linuxvars.system->cli_begin_update = system_cli_begin_update;
    linuxvars.system->cli_update_step = system_cli_update_step;
    linuxvars.system->cli_end_update = system_cli_end_update;

    linuxvars.system->thread_get_id = system_thread_get_id;
    linuxvars.system->thread_current_job_id = system_thread_current_job_id;
    linuxvars.system->post_job = system_post_job;
    linuxvars.system->cancel_job = system_cancel_job;
    linuxvars.system->job_is_pending = system_job_is_pending;
    linuxvars.system->grow_thread_memory = system_grow_thread_memory;
    linuxvars.system->acquire_lock = system_acquire_lock;
    linuxvars.system->release_lock = system_release_lock;
    
    linuxvars.system->force_redraw = system_force_redraw;
    
    linuxvars.system->internal_sentinel = INTERNAL_system_sentinel;
    linuxvars.system->internal_get_thread_states = INTERNAL_get_thread_states;
#endif
}

internal void
LinuxShowScreen(){
}

int main(){
    linuxvars = {};
    
    if (!LinuxLoadAppCode()){
        return(1);
    }
    
    System_Functions system_;
    System_Functions *system = &system_;
    linuxvars.system = system;
    LinuxLoadSystemCode();
        
    {
        void *base;
#if FRED_INTERNAL
        base = (void*)Mbytes(128);
#else
        base = (void*)0;
#endif
        
        i32 prot = PROT_READ | PROT_WRITE;
        i32 flags = MAP_PRIVATE | MAP_ANON;
        
        linuxvars.mem.vars_memory_size = Mbytes(2);
        linuxvars.mem.vars_memory = mmap(base, linuxvars.mem.vars_memory_size,
                                         prot, flags, -1, 0);
        
#if FRED_INTERNAL
        base = (void*)Mbytes(160);
#else
        base = (void*)0;
#endif
        
        linuxvars.mem.target_memory_size = Mbytes(64);
        linuxvars.mem.target_memory = mmap(base, linuxvars.mem.target_memory_size,
                                           prot, flags, -1, 0);
    }
    
    i32 width = 800;
    i32 height = 600;
    i32 bpp = 24;
    
    linuxvars.target.width = width;
    linuxvars.target.height = height;

    keycode_init(&linuxvars.codes);

    system_acquire_lock(FRAME_LOCK);
    b32 first = 1;
    linuxvars.keep_going = 1;
    i64 timer_start = system_time();

    for (;linuxvars.keep_going;){
        linuxvars.input = {};
        linuxvars.clipboard = {};
        linuxvars.mouse.wheel = 0;
        linuxvars.force_redraw = 0;
        
        for (;0;){
        }
        
        linuxvars.mouse.left_button_prev = linuxvars.mouse.left_button;
        linuxvars.mouse.right_button_prev = linuxvars.mouse.right_button;
                
#if 0
        Application_Step_Result app_result =
            linuxvars.app.step(linuxvars.system,
                               &linuxvars.codes,
                               &linuxvars.input,
                               &linuxvars.mouse,
                               &linuxvars.target,
                               &linuxvars.mem,
                               linuxvars.clipboard,
                               1, first, linuxvars.force_redraw);
#else
        Application_Step_Result app_result = {};
#endif
        
        if (1 || linuxvars.force_redraw){
            //glClearColor(1.f, 1.f, 0.f, 1.f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            LinuxShowScreen();
            linuxvars.force_redraw = 0;
        }
        
        i64 timer_target = timer_start + frame_useconds;
        i64 timer_end = system_time();
        for (;timer_end < timer_target;){
            i64 samount = timer_target - timer_end;
            usleep(samount);
            timer_end = system_time();
        }
        timer_start = system_time();
    }
    
    return(0);
}

// BOTTOM

