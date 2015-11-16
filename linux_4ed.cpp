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
#include <SDL/SDL.h>
#include <GL/gl.h>

#include "4ed_internal.h"
#include "4ed_linux_keyboard.cpp"

#define printe(m) printf("%s:%d: %s\n", __FILE__, __LINE__, #m)

struct Linux_Vars{
    Application_Memory mem;
    Key_Input_Data input;
    b32 keep_going;
    
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
    
    linuxvars.app_code = SDL_LoadObject(path);
    if (linuxvars.app_code){
        result = 1;
        linuxvars.app.init = (App_Init*)
            SDL_LoadFunction(linuxvars.app_code, "app_init");
        linuxvars.app.step = (App_Step*)
            SDL_LoadFunction(linuxvars.app_code, "app_step");
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

int main(){
    linuxvars = {};
    
    if (SDL_Init(SDL_INIT_VIDEO)){
        // TODO(allen): initialization failure
        printe(SDL_Init);
        return(1);
    }
    SDL_EnableUNICODE(1);
    
    if (!LinuxLoadAppCode()){
        return(1);
    }
    
    System_Functions system_;
    System_Functions *system = &system_;
    linuxvars.system = system;
    LinuxLoadSystemCode();
    
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info){
        // TODO(allen): initialization failure
        printe(info);
        return(1);
    }
    
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
    i32 bpp = info->vfmt->BitsPerPixel;
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    i32 flags = SDL_OPENGL;
    
    if (SDL_SetVideoMode(width, height, bpp, flags) == 0){
        // TODO(allen): initialization failure
        printe(SDL_SetVideoMode);
        return(1);
    }
    
    SDL_WM_SetCaption("4coder-window", 0);
    
    system_acquire_lock(FRAME_LOCK);
    b32 first = 1;
    linuxvars.keep_going = 1;
    
    SDL_Event event;
    for (;linuxvars.keep_going && SDL_WaitEvent(&event);){
        b32 pass_to_app = 0;
        
        linuxvars.input = {};
        linuxvars.clipboard = {};
        switch (event.type){
        case SDL_ACTIVEEVENT:
        {
            if ((event.active.state & SDL_APPACTIVE) && event.active.gain)
                pass_to_app = 1;
        }break;
        
        case SDL_KEYDOWN:
        {
            pass_to_app = 1;
            linuxvars.input.press[linuxvars.input.press_count++] =
                get_key_event(&event);
        }break;
        
        case SDL_QUIT:
        {
            linuxvars.keep_going = 0;
        }break;
        
        }
        
        if (pass_to_app){
            Application_Step_Result app_result =
                linuxvars.app.step(linuxvars.system,
                                   0,
                                   0,
                                   &linuxvars.input,
                                   0,
                                   1,
                                   0,
                                   &linuxvars.mem,
                                   linuxvars.clipboard,
                                   first, 1);
            
            
            
            SDL_GL_SwapBuffers();
        }
    }
    
    return(0);
}

// BOTTOM

