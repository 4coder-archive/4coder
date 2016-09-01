/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for project codename "4ed"
 *
 */

// TOP

#include <assert.h>
#include "4ed_defines.h"

#if defined(FRED_SUPER)

# define FSTRING_IMPLEMENTATION
# define FSTRING_C
# include "4coder_string.h"

#include "4coder_version.h"
# include "4coder_keycodes.h"
# include "4coder_style.h"
# include "4coder_rect.h"

# include "4coder_mem.h"

// TODO(allen): This is duplicated from 4coder_custom.h
// I need to work out a way to avoid this.
#define VIEW_ROUTINE_SIG(name) void name(struct Application_Links *app, int32_t view_id)
#define GET_BINDING_DATA(name) int32_t name(void *data, int32_t size)
#define _GET_VERSION_SIG(n) int32_t n(int32_t maj, int32_t min, int32_t patch)

typedef VIEW_ROUTINE_SIG(View_Routine_Function);
typedef GET_BINDING_DATA(Get_Binding_Data_Function);
typedef _GET_VERSION_SIG(_Get_Version_Function);

struct Custom_API{
    View_Routine_Function *view_routine;
    Get_Binding_Data_Function *get_bindings;
    _Get_Version_Function *get_alpha_4coder_version;
};


typedef void Custom_Command_Function;
#include "4coder_types.h"
struct Application_Links;
# include "4ed_os_custom_api.h"

//# include "4coder_custom.h"
#else
# include "4coder_default_bindings.cpp"

# define FSTRING_IMPLEMENTATION
# define FSTRING_C
# include "4coder_string.h"

#endif

#include "4ed_math.h"

#include "4ed_system.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include <Windows.h>
#include <GL/gl.h>

#define GL_TEXTURE_MAX_LEVEL 0x813D

#include "filetrack/4tech_file_track_win32.c"
#include "system_shared.h"

#define SUPPORT_DPI 1
#define USE_FT_FONTS 1

#define FPS 60
#define frame_useconds (1000000 / FPS)

#define WM_4coder_ANIMATE (WM_USER + 0)

//
// Win32_Vars structs
//

struct Thread_Context{
    u32 job_id;
    b32 running;
    b32 cancel;
    
    Work_Queue *queue;
    u32 id;
    u32 group_id;
    u32 windows_id;
    HANDLE handle;
};

struct Thread_Group{
    Thread_Context *threads;
    i32 count;
    
    Unbounded_Work_Queue queue;
    
    i32 cancel_lock0;
    i32 cancel_cv0;
};

struct Control_Keys{
    b8 l_ctrl;
    b8 r_ctrl;
    b8 l_alt;
    b8 r_alt;
};
inline Control_Keys
control_keys_zero(){
    Control_Keys result = {0};
    return(result);
}

struct Win32_Input_Chunk_Transient{
    Key_Input_Data key_data;
    b8 mouse_l_press, mouse_l_release;
    b8 mouse_r_press, mouse_r_release;
    b8 out_of_window;
    i8 mouse_wheel;
    b8 trying_to_kill;
};
static Win32_Input_Chunk_Transient null_input_chunk_transient = {0};

struct Win32_Input_Chunk_Persistent{
    i32 mouse_x, mouse_y;
    b8 mouse_l, mouse_r;
    
    Control_Keys controls;
    b8 control_keys[MDFR_INDEX_COUNT];
};

typedef struct Win32_Input_Chunk{
    Win32_Input_Chunk_Transient trans;
    Win32_Input_Chunk_Persistent pers;
} Win32_Input_Chunk;

typedef struct Win32_Coroutine{
    Coroutine coroutine;
    Win32_Coroutine *next;
    i32 done;
} Win32_Coroutine;

#if FRED_INTERNAL
struct Sys_Bubble : public Bubble{
    i32 line_number;
    char *file_name;
};
typedef struct Sys_Bubble Sys_Bubble;
#endif

enum CV_ID{
    CANCEL_CV0,
    CANCEL_CV1,
    CANCEL_CV2,
    CANCEL_CV3,
    CANCEL_CV4,
    CANCEL_CV5,
    CANCEL_CV6,
    CANCEL_CV7,
    CV_COUNT
};

typedef struct Drive_Strings{
    char *prefix_[26];
    char **prefix;
} Drive_Strings;

typedef struct Win32_Vars{
    System_Functions system;
    App_Functions app;
    Custom_API custom_api;
    HMODULE app_code;
    HMODULE custom;
    Plat_Settings settings;
    
    
    Work_Queue queues[THREAD_GROUP_COUNT];
    Thread_Group groups[THREAD_GROUP_COUNT];
    CRITICAL_SECTION locks[LOCK_COUNT];
    CONDITION_VARIABLE condition_vars[CV_COUNT];
    Thread_Memory *thread_memory;
    Win32_Coroutine coroutine_data[18];
    Win32_Coroutine *coroutine_free;
    
    
    Win32_Input_Chunk input_chunk;
    b32 lctrl_lalt_is_altgr;
    b32 got_useful_event;
    
    b32 full_screen;
    b32 do_toggle;
    WINDOWPLACEMENT GlobalWindowPosition;
    b32 send_exit_signal;
    
    HCURSOR cursor_ibeam;
    HCURSOR cursor_arrow;
    HCURSOR cursor_leftright;
    HCURSOR cursor_updown;
    String clipboard_contents;
    b32 next_clipboard_is_self;
    DWORD clipboard_sequence;
    
    
    HWND window_handle;
    Render_Target target;
    Partition font_part;
#if SUPPORT_DPI
    i32 dpi_x, dpi_y;
#endif
    
    u64 count_per_usecond;
    b32 first;
    i32 running_cli;
    
    Drive_Strings dstrings;
    
#if FRED_INTERNAL
    CRITICAL_SECTION DEBUG_sysmem_lock;
    Sys_Bubble internal_bubble;
#endif
} Win32_Vars;

globalvar Win32_Vars win32vars;
globalvar Application_Memory memory_vars;


//
// Helpers
//

internal HANDLE
Win32Handle(Plat_Handle h){
    HANDLE result;
    memcpy(&result, &h, sizeof(result));
    return(result);
}

internal Plat_Handle
Win32Handle(HANDLE h){
    Plat_Handle result = {0};
    Assert(sizeof(Plat_Handle) >= sizeof(h));
    memcpy(&result, &h, sizeof(h));
    return(result);
}

internal void*
Win32Ptr(Plat_Handle h){
    void *result;
    memcpy(&result, &h, sizeof(result));
    return(result);
}

internal Plat_Handle
Win32Ptr(void *h){
    Plat_Handle result = {0};
    memcpy(&result, &h, sizeof(h));
    return(result);
}


//
// Memory (not exposed to application, but needed in system_shared.cpp)
//

internal
Sys_Get_Memory_Sig(system_get_memory_){
    void *ptr = 0;
    if (size > 0){
#if FRED_INTERNAL
        ptr = VirtualAlloc(0, size + sizeof(Sys_Bubble), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        Sys_Bubble *bubble = (Sys_Bubble*)ptr;
        bubble->flags = 0;
        bubble->line_number = line_number;
        bubble->file_name = file_name;
        bubble->size = size;
        EnterCriticalSection(&win32vars.DEBUG_sysmem_lock);
        insert_bubble(&win32vars.internal_bubble, bubble);
        LeaveCriticalSection(&win32vars.DEBUG_sysmem_lock);
        ptr = bubble + 1;
#else
        ptr = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#endif
    }
    return(ptr);
}

internal
Sys_Free_Memory_Sig(system_free_memory){
    if (block){
#if FRED_INTERNAL
        Sys_Bubble *bubble = ((Sys_Bubble*)block) - 1;
        EnterCriticalSection(&win32vars.DEBUG_sysmem_lock);
        remove_bubble(bubble);
        LeaveCriticalSection(&win32vars.DEBUG_sysmem_lock);
        VirtualFree(bubble, 0, MEM_RELEASE);
#else
        VirtualFree(block, 0, MEM_RELEASE);
#endif
    }
}

#define Win32GetMemory(size) system_get_memory_(size, __LINE__, __FILE__)
#define Win32FreeMemory(ptr) system_free_memory(ptr)

#define Win32ScratchPartition sysshared_scratch_partition
#define Win32ScratchPartitionGrow sysshared_partition_grow
#define Win32ScratchPartitionDouble sysshared_partition_double

#if FRED_INTERNAL
internal Bubble*
INTERNAL_system_sentinel(){
    return (&win32vars.internal_bubble);
}

internal void
INTERNAL_system_debug_message(char *message){
    OutputDebugStringA(message);
}
#endif


//
// Multithreading
//

internal
Sys_Acquire_Lock_Sig(system_acquire_lock){
    EnterCriticalSection(&win32vars.locks[id]);
}

internal
Sys_Release_Lock_Sig(system_release_lock){
    LeaveCriticalSection(&win32vars.locks[id]);
}

internal void
system_wait_cv(i32 crit_id, i32 cv_id){
    SleepConditionVariableCS(win32vars.condition_vars + cv_id,
                             win32vars.locks + crit_id,
                             INFINITE);
}

internal void
system_signal_cv(i32 crit_id, i32 cv_id){
    AllowLocal(crit_id);
    WakeConditionVariable(win32vars.condition_vars + cv_id);
}

internal DWORD
JobThreadProc(LPVOID lpParameter){
    Thread_Context *thread = (Thread_Context*)lpParameter;
    Work_Queue *queue = win32vars.queues + thread->group_id;
    Thread_Group *group = win32vars.groups + thread->group_id;
    
    i32 thread_index = thread->id - 1;
    
    i32 cancel_lock = group->cancel_lock0 + thread_index;
    i32 cancel_cv = group->cancel_cv0 + thread_index;
    
    Thread_Memory *thread_memory = win32vars.thread_memory + thread_index;
    
    if (thread_memory->size == 0){
        i32 new_size = Kbytes(64);
        thread_memory->data = Win32GetMemory(new_size);
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
                    
                    full_job->job.callback(&win32vars.system,
                                           thread, thread_memory, full_job->job.data);
                    PostMessage(win32vars.window_handle, WM_4coder_ANIMATE, 0, 0);
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
            WaitForSingleObject(Win32Handle(queue->semaphore), INFINITE);
        }
    }
}

internal void
initialize_unbounded_queue(Unbounded_Work_Queue *source_queue){
    i32 max = 512;
    source_queue->jobs = (Full_Job_Data*)system_get_memory(max*sizeof(Full_Job_Data));
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
    // TODO(allen): pull out the duplicated part once I see
    // that this is pretty much the same on linux.
    for (i32 i = 0; i < semaphore_release_count; ++i){
        ReleaseSemaphore(Win32Handle(queue->semaphore), 1, 0);
    }
}

internal void
flush_thread_group(i32 group_id){
    Thread_Group *group = win32vars.groups + group_id;
    Work_Queue *queue = win32vars.queues + group_id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    flush_to_direct_queue(source_queue, queue, group->count);
}

// Note(allen): post_job puts the job on the unbounded queue.
// The unbounded queue is entirely managed by the main thread.
// The thread safe queue is bounded in size so the unbounded
// queue is periodically flushed into the direct work queue.
internal
Sys_Post_Job_Sig(system_post_job){
    Thread_Group *group = win32vars.groups + group_id;
    Unbounded_Work_Queue *queue = &group->queue;
    
    u32 result = queue->next_job_id++;
    
    while (queue->count >= queue->max){
        i32 new_max = queue->max*2;
        Full_Job_Data *new_jobs = (Full_Job_Data*)
            system_get_memory(new_max*sizeof(Full_Job_Data));
        
        memcpy(new_jobs, queue->jobs, queue->count);
        
        system_free_memory(queue->jobs);
        
        queue->jobs = new_jobs;
        queue->max = new_max;
    }
    
    Full_Job_Data full_job;
    
    full_job.job = job;
    full_job.running_thread = THREAD_NOT_ASSIGNED;
    full_job.id = result;
    
    queue->jobs[queue->count++] = full_job;
    
    Work_Queue *direct_queue = win32vars.queues + group_id;
    flush_to_direct_queue(queue, direct_queue, group->count);
    
    return(result);
}

internal
Sys_Cancel_Job_Sig(system_cancel_job){
    Thread_Group *group = win32vars.groups + group_id;
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
        Work_Queue *queue = win32vars.queues + group_id;
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
    
    Thread_Group *group = win32vars.groups + thread->group_id;
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
    new_size = LargeRoundUp(memory->size*2, Kbytes(4));
    memory->data = system_get_memory(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        system_free_memory(old_data);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}

#if FRED_INTERNAL
internal void
INTERNAL_get_thread_states(Thread_Group_ID id, bool8 *running, i32 *pending){
    Thread_Group *group = win32vars.groups + id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    Work_Queue *queue = win32vars.queues + id;
    u32 write = queue->write_position;
    u32 read = queue->read_position;
    if (write < read) write += QUEUE_WRAP;
    *pending = (i32)(write - read) + source_queue->count - source_queue->skip;
    
    for (i32 i = 0; i < group->count; ++i){
        running[i] = (group->threads[i].running != 0);
    }
}
#endif


//
// Coroutines
//

internal Win32_Coroutine*
Win32AllocCoroutine(){
    Win32_Coroutine *result = win32vars.coroutine_free;
    Assert(result != 0);
    win32vars.coroutine_free = result->next;
    return(result);
}

internal void
Win32FreeCoroutine(Win32_Coroutine *data){
    data->next = win32vars.coroutine_free;
    win32vars.coroutine_free = data;
}

internal void
Win32CoroutineMain(void *arg_){
    Win32_Coroutine *c = (Win32_Coroutine*)arg_;
    c->coroutine.func(&c->coroutine);
    c->done = 1;
    Win32FreeCoroutine(c);
    SwitchToFiber(c->coroutine.yield_handle);
}

internal
Sys_Create_Coroutine_Sig(system_create_coroutine){
    Win32_Coroutine *c;
    Coroutine *coroutine;
    void *fiber;
    
    c = Win32AllocCoroutine();
    c->done = 0;
    
    coroutine = &c->coroutine;
    
    fiber = CreateFiber(0, Win32CoroutineMain, coroutine);
    
    coroutine->plat_handle = Win32Handle(fiber);
    coroutine->func = func;
    
    return(coroutine);
}

internal
Sys_Launch_Coroutine_Sig(system_launch_coroutine){
    Win32_Coroutine *c = (Win32_Coroutine*)coroutine;
    void *fiber;
    
    fiber = Win32Handle(coroutine->plat_handle);
    coroutine->yield_handle = GetCurrentFiber();
    coroutine->in = in;
    coroutine->out = out;
    
    SwitchToFiber(fiber);
    
    if (c->done){
        DeleteFiber(fiber);
        Win32FreeCoroutine(c);
        coroutine = 0;
    }
    
    return(coroutine);
}

Sys_Resume_Coroutine_Sig(system_resume_coroutine){
    Win32_Coroutine *c = (Win32_Coroutine*)coroutine;
    void *fiber;
    
    Assert(c->done == 0);
    
    coroutine->yield_handle = GetCurrentFiber();
    coroutine->in = in;
    coroutine->out = out;
    
    fiber = Win32Ptr(coroutine->plat_handle);
    
    SwitchToFiber(fiber);
    
    if (c->done){
        DeleteFiber(fiber);
        Win32FreeCoroutine(c);
        coroutine = 0;
    }
    
    return(coroutine);
}

Sys_Yield_Coroutine_Sig(system_yield_coroutine){
    SwitchToFiber(coroutine->yield_handle);
}


//
// Files
//

internal
Sys_File_Can_Be_Made_Sig(system_file_can_be_made){
    HANDLE file;
    file = CreateFile((char*)filename, FILE_APPEND_DATA, 0, 0,
                      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (!file || file == INVALID_HANDLE_VALUE){
        return 0;
    }
    
    CloseHandle(file);
    
    return(1);
}

internal
Sys_Set_File_List_Sig(system_set_file_list){
    if (directory.size > 0){
        char dir_space[MAX_PATH + 32];
        String dir = make_string_cap(dir_space, 0, MAX_PATH + 32);
        append_ss(&dir, directory);
        append_ss(&dir, make_lit_string("\\*"));
        terminate_with_null(&dir);
        
        char *c_str_dir = dir.str;
        
        WIN32_FIND_DATA find_data;
        HANDLE search;
        search = FindFirstFileA(c_str_dir, &find_data);
        
        if (search != INVALID_HANDLE_VALUE){            
            i32 count = 0;
            i32 file_count = 0;
            BOOL more_files = 1;
            do{
                if (!match_cs(find_data.cFileName, make_lit_string(".")) &&
                    !match_cs(find_data.cFileName, make_lit_string(".."))){
                    ++file_count;
                    i32 size = 0;
                    for(;find_data.cFileName[size];++size);
                    count += size + 1;
                }
                more_files = FindNextFile(search, &find_data);
            }while(more_files);
            FindClose(search);
            
            i32 required_size = count + file_count * sizeof(File_Info);
            if (file_list->block_size < required_size){
                Win32FreeMemory(file_list->block);
                file_list->block = Win32GetMemory(required_size);
                file_list->block_size = required_size;
            }
            
            file_list->infos = (File_Info*)file_list->block;
            char *name = (char*)(file_list->infos + file_count);
            if (file_list->block){
                search = FindFirstFileA(c_str_dir, &find_data);
                
                if (search != INVALID_HANDLE_VALUE){
                    File_Info *info = file_list->infos;
                    more_files = 1;
                    do{
                        if (!match_cs(find_data.cFileName, make_lit_string(".")) &&
                            !match_cs(find_data.cFileName, make_lit_string(".."))){
                            info->folder = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                            info->filename = name;
                            
                            i32 i = 0;
                            for(;find_data.cFileName[i];++i) *name++ = find_data.cFileName[i];
                            info->filename_len = i;
                            *name++ = 0;
                            String fname = make_string_cap(info->filename,
                                                           info->filename_len,
                                                           info->filename_len+1);
                            replace_char(&fname, '\\', '/');
                            ++info;
                        }
                        more_files = FindNextFile(search, &find_data);
                    }while(more_files);
                    FindClose(search);
                    
                    file_list->count = file_count;
                    
                }else{
                    Win32FreeMemory(file_list->block);
                    file_list->block = 0;
                    file_list->block_size = 0;
                }
            }
        }
    }
    else{
        if (directory.str == 0){
            Win32FreeMemory(file_list->block);
            file_list->block = 0;
            file_list->block_size = 0;
        }
        file_list->infos = 0;
        file_list->count = 0;
    }
}

internal void
set_volume_prefix(Drive_Strings *dstrings, char *vol){
    char c = vol[0];
    if (dstrings->prefix[c]){
        system_free_memory(dstrings->prefix[c]);
    }
    
    HANDLE hdir = CreateFile(
        vol,
        GENERIC_READ,
        FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0);
    
    if (hdir != INVALID_HANDLE_VALUE){
        char *s = 0;
        DWORD len = GetFinalPathNameByHandle(hdir, 0, 0, 0);
        len = len + 1;
        s = (char*)system_get_memory(len);
        len = GetFinalPathNameByHandle(hdir, s, len, 0);
        s[len] = 0;
        if (s[len-1] == '\\') s[len-1] = 0;
        dstrings->prefix[c] = s + 4;
        CloseHandle(hdir);
    }
    else{
        dstrings->prefix[c] = 0;
    }
}

internal void
win32_init_drive_strings(Drive_Strings *dstrings){
    dstrings->prefix = dstrings->prefix_ - 'A';
    
    char vol[4] = "A:\\";
    for (char c = 'A'; c <= 'Z'; ++c){
        vol[0] = c;
        set_volume_prefix(dstrings, vol);
    }
}

// NOTE(allen): This does not chase down symbolic links because doing so
// would require a lot of heavy duty OS calls.  I've decided to give up
// a little ground on always recognizing files as equivalent in exchange
// for the ability to handle them very quickly when nothing strange is
// going on.
internal int32_t
win32_canonical_ansi_name(Drive_Strings *dstrings, char *src, i32 len, char *dst, i32 max){
    char *wrt = dst;
    char *wrt_stop = dst + max;
    char *src_stop = src + len;
    char c = 0;
    char **prefix_array = dstrings->prefix;
    char *prefix = 0;
    
    if (len >= 2 && max > 0){
        c = src[0];
        if (c >= 'a' && c <= 'z'){
            c -= 'a' - 'A';
        }
        
        if (c >= 'A' && c <= 'Z' && src[1] == ':'){
            prefix = prefix_array[c];
            if (prefix){
                for (;*prefix;){
                    *(wrt++) = *(prefix++);
                    if (wrt == wrt_stop) goto fail;
                }
            }
            else{
                *(wrt++) = c;
                if (wrt == wrt_stop) goto fail;
                *(wrt++) = ':';
                if (wrt == wrt_stop) goto fail;
            }
            src += 2;
            
            for (; src < src_stop; ++src){
                c = src[0];
                
                if (c >= 'A' && c <= 'Z'){
                    c += 'a' - 'A';
                }
                
                if (c == '/' || c == '\\'){
                    c = '\\';
                    if (wrt > dst && wrt[-1] == '\\'){
                        continue;
                    }
                    else if (src[1] == '.'){
                        if (src[2] == '\\' || src[2] == '/'){
                            src += 1;
                        }
                        else if (src[2] == '.' && (src[3] == '\\' || src[3] == '/')){
                            src += 2;
                            while (wrt > dst && wrt[0] != '\\'){
                                --wrt;
                            }
                            if (wrt == dst) goto fail;
                        }
                    }
                }
                
                *wrt = c;
                ++wrt;
                if (wrt == wrt_stop) goto fail;
            }
        }
    }
    
    if (0){
        fail:;
        wrt = dst;
    }
    
    int32_t result = (int32_t)(wrt - dst);
    return(result);
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    i32 result = win32_canonical_ansi_name(&win32vars.dstrings, filename, len, buffer, max);
    return(result);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = 0;
    HANDLE file = CreateFile(filename,
                             GENERIC_READ,
                             0,
                             0,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             0);
    
    if (file != INVALID_HANDLE_VALUE){
        *(HANDLE*)handle_out = file;
        result = 1;
    }
    
    return(result);
}

internal
Sys_Load_Size_Sig(system_load_size){
    u32 result = 0;
    HANDLE file = *(HANDLE*)(&handle);
    
    DWORD hi = 0;
    DWORD lo = GetFileSize(file, &hi);
    
    if (hi == 0){
        result = lo;
    }
    
    return(result);
}

internal
Sys_Load_File_Sig(system_load_file){
    b32 result = 0;
    HANDLE file = *(HANDLE*)(&handle);
    
    DWORD read_size = 0;
    
    if (ReadFile(file,
                 buffer,
                 size,
                 &read_size,
                 0)){
        if (read_size == size){
            result = 1;
        }
    }
    
    return(result);
}

internal
Sys_Load_Close_Sig(system_load_close){
    b32 result = 0;
    HANDLE file = *(HANDLE*)(&handle);
    if (CloseHandle(file)){
        result = 1;
    }
    return(result);
}

internal
Sys_Save_File_Sig(system_save_file){
    b32 result = 0;
    HANDLE file = CreateFile(filename,
                             GENERIC_WRITE,
                             0,
                             0,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             0);
    
    if (file != INVALID_HANDLE_VALUE){
        DWORD written_total = 0;
        DWORD written_size = 0;
        
        result = 1;
        
        while (written_total < size){
            if (!WriteFile(file, buffer, size, &written_size, 0)){
                result = 0;
                break;
            }
            written_total += written_size;
        }
        
        CloseHandle(file);
    }
    
    return(result);
}

internal
Sys_Now_Time_Sig(system_now_time){
    u64 result = __rdtsc();
    return(result);
}

b32 Win32DirectoryExists(char *path){
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

internal
Sys_Get_Binary_Path_Sig(system_get_binary_path){
    i32 result = 0;
    i32 size = GetModuleFileName(0, out->str, out->memory_size);
    if (size < out->memory_size-1){
        out->size = size;
        remove_last_folder(out);
        terminate_with_null(out);
        result = out->size;
    }
    return(result);
}


/*
NOTE(casey): This follows Raymond Chen's prescription
for fullscreen toggling, see:
http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
*/

internal void
Win32ToggleFullscreen(void){
    HWND Window = win32vars.window_handle;
    LONG_PTR Style = GetWindowLongPtr(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW){
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &win32vars.GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLongPtr(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            win32vars.full_screen = 1;
        }
    }
    else{
        SetWindowLongPtr(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &win32vars.GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        win32vars.full_screen = 0;
    }
}

/*
NOTE(allen): 
This is the crazy hacky nonsense I came up with to get alt-tab
working in full screen mode.  It puts the window back into
bordered mode when the alt-tabbing begins.  When the window regains
focus it is automatically refullscreened.
*/

internal void
Win32FixFullscreenLoseFocus(b32 lose_focus){
    if (win32vars.full_screen){
        
        HWND Window = win32vars.window_handle;
        LONG_PTR Style = GetWindowLongPtr(Window, GWL_STYLE);
        
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            if (lose_focus){
                SetWindowLongPtr(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
            }
            else{
                SetWindowLongPtr(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            }
            
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
}

#include "win32_api_impl.cpp"

//
// Clipboard
//

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
	if (OpenClipboard(win32vars.window_handle)){
		EmptyClipboard();
		HANDLE memory_handle;
		memory_handle = GlobalAlloc(GMEM_MOVEABLE, str.size+1);
		if (memory_handle){
			char *dest = (char*)GlobalLock(memory_handle);
            copy_fast_unsafe_cs(dest, str);
			GlobalUnlock(memory_handle);
			SetClipboardData(CF_TEXT, memory_handle);
			win32vars.next_clipboard_is_self = 1;
		}
		CloseClipboard();
	}
}

internal b32
Win32ReadClipboardContents(){
    b32 result = 0;
    
    if (IsClipboardFormatAvailable(CF_TEXT)){
        result = 1;
        if (OpenClipboard(win32vars.window_handle)){
            HANDLE clip_data;
            clip_data = GetClipboardData(CF_TEXT);
            if (clip_data){
                win32vars.clipboard_contents.str = (char*)GlobalLock(clip_data);
                if (win32vars.clipboard_contents.str){
                    win32vars.clipboard_contents.size = str_size((char*)win32vars.clipboard_contents.str);
                    GlobalUnlock(clip_data);
                }
            }
            CloseClipboard();
        }
    }
    
    return(result);
}


//
// Command Line Exectuion
//

internal
Sys_CLI_Call_Sig(system_cli_call){
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    char command_line[2048];
    
    String s = make_fixed_width_string(command_line);
    copy_ss(&s, make_lit_string("/C "));
    append_partial_sc(&s, script_name);
    b32 success = terminate_with_null(&s);
    
    if (success){
        success = 0;
        
        SECURITY_ATTRIBUTES sec_attributes = {};
        HANDLE out_read;
        HANDLE out_write;
        
        sec_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        sec_attributes.bInheritHandle = TRUE;
        
        if (CreatePipe(&out_read, &out_write, &sec_attributes, 0)){
            if (SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)){
                STARTUPINFO startup = {};
                startup.cb = sizeof(STARTUPINFO);
                startup.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
                startup.hStdError = out_write;
                startup.hStdOutput = out_write;
                startup.wShowWindow = SW_HIDE;
                
                PROCESS_INFORMATION info = {};
                
                Assert(sizeof(Plat_Handle) >= sizeof(HANDLE));
                if (CreateProcess(cmd, command_line,
                                  0, 0, TRUE, 0,
                                  env_variables, path,
                                  &startup, &info)){
                    success = 1;
                    CloseHandle(info.hThread);
                    *(HANDLE*)&cli_out->proc = info.hProcess;
                    *(HANDLE*)&cli_out->out_read = out_read;
                    *(HANDLE*)&cli_out->out_write = out_write;
                    
                    ++win32vars.running_cli;
                }
                else{
                    CloseHandle(out_read);
                    CloseHandle(out_write);
                    *(HANDLE*)&cli_out->proc = INVALID_HANDLE_VALUE;
                    *(HANDLE*)&cli_out->out_read = INVALID_HANDLE_VALUE;
                    *(HANDLE*)&cli_out->out_write = INVALID_HANDLE_VALUE;
                }
            }
            else{
                // TODO(allen): failed SetHandleInformation
                CloseHandle(out_read);
                CloseHandle(out_write);
                *(HANDLE*)&cli_out->proc = INVALID_HANDLE_VALUE;
                *(HANDLE*)&cli_out->out_read = INVALID_HANDLE_VALUE;
                *(HANDLE*)&cli_out->out_write = INVALID_HANDLE_VALUE;
            }
        }
        else{
            // TODO(allen): failed CreatePipe
        }
    }
    
    return success;
}

struct CLI_Loop_Control{
    u32 remaining_amount;
};

internal
Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    Assert(sizeof(cli->scratch_space) >= sizeof(CLI_Loop_Control));
    CLI_Loop_Control *loop = (CLI_Loop_Control*)cli->scratch_space;
    loop->remaining_amount = 0;
}

internal
Sys_CLI_Update_Step_Sig(system_cli_update_step){
    HANDLE handle = *(HANDLE*)&cli->out_read;
    CLI_Loop_Control *loop = (CLI_Loop_Control*)cli->scratch_space;
    b32 has_more = 0;
    DWORD remaining = loop->remaining_amount;
    u32 pos = 0;
    DWORD read_amount = 0;
    
    for (;;){
        if (remaining == 0){
            if (!PeekNamedPipe(handle, 0, 0, 0, &remaining, 0)) break;
            if (remaining == 0) break;
        }
        
        if (remaining + pos < max){
            has_more = 1;
            ReadFile(handle, dest + pos, remaining, &read_amount, 0);
            TentativeAssert(remaining == read_amount);
            pos += remaining;
            remaining = 0;
        }
        else{
            has_more = 1;
            ReadFile(handle, dest + pos, max - pos, &read_amount, 0);
            TentativeAssert(max - pos == read_amount);
            loop->remaining_amount = remaining - (max - pos);
            pos = max;
            break;
        }
    }
    *amount = pos;
    
    return(has_more);
}

internal
Sys_CLI_End_Update_Sig(system_cli_end_update){
    b32 close_me = 0;
    HANDLE proc = *(HANDLE*)&cli->proc;
    DWORD result = 0;
    
    if (WaitForSingleObject(proc, 0) == WAIT_OBJECT_0){
        if (GetExitCodeProcess(proc, &result) == 0)
            cli->exit = -1;
        else
            cli->exit = (i32)result;
        
        close_me = 1;
        CloseHandle(*(HANDLE*)&cli->proc);
        CloseHandle(*(HANDLE*)&cli->out_read);
        CloseHandle(*(HANDLE*)&cli->out_write);
        
        --win32vars.running_cli;
    }
    return(close_me);
}

#include "system_shared.cpp"

#if USE_FT_FONTS
# include "win32_ft_font.cpp"
#endif

internal f32
size_change(i32 dpi_x, i32 dpi_y){
    // TODO(allen): We're just hoping dpi_x == dpi_y for now I guess.
    f32 size_x = dpi_x / 96.f;
    f32 size_y = dpi_y / 96.f;
    f32 size_max = Max(size_x, size_y);
    return(size_max);
}

internal
Font_Load_Sig(system_draw_font_load){
    if (win32vars.font_part.base == 0){
        win32vars.font_part = Win32ScratchPartition(Mbytes(8));
    }
    
    i32 oversample = 2;
    AllowLocal(oversample);
    
#if SUPPORT_DPI
    pt_size = ROUND32(pt_size * size_change(win32vars.dpi_x, win32vars.dpi_y));
#endif
    
    for (b32 success = 0; success == 0;){
#if USE_WIN32_FONTS
        
        success = win32_font_load(&win32vars.font_part,
                                  font_out,
                                  filename,
                                  fontname,
                                  pt_size,
                                  tab_width,
                                  oversample,
                                  store_texture);
        
#elif USE_FT_FONTS
        
        success = win32_ft_font_load(&win32vars.font_part,
                                     font_out,
                                     filename,
                                     pt_size,
                                     tab_width,
                                     win32vars.settings.use_hinting);
        
#else
        
        success = stb_font_load(&win32vars.font_part,
                                font_out,
                                filename,
                                pt_size,
                                tab_width,
                                oversample,
                                store_texture);
        
#endif
        
        // TODO(allen): Make the growable partition something
        // that can just be passed directly to font load and
        // let it be grown there.
        if (!success){
            Win32ScratchPartitionDouble(&win32vars.font_part);
        }
    }
    
    return(1);
}

//
// Linkage to Custom and Application
//

internal b32
Win32LoadAppCode(){
    b32 result = 0;
    App_Get_Functions *get_funcs = 0;
    
    win32vars.app_code = LoadLibraryA("4ed_app.dll");
    if (win32vars.app_code){
        get_funcs = (App_Get_Functions*)
            GetProcAddress(win32vars.app_code, "app_get_functions");
    }
    
    if (get_funcs){
        result = 1;
        win32vars.app = get_funcs();        
    }

    return result;
}

internal void
Win32LoadSystemCode(){
    win32vars.system.set_file_list = system_set_file_list;
    win32vars.system.get_canonical = system_get_canonical;
    win32vars.system.add_listener = system_add_listener;
    win32vars.system.remove_listener = system_remove_listener;
    win32vars.system.get_file_change = system_get_file_change;
    win32vars.system.load_handle = system_load_handle;
    win32vars.system.load_size = system_load_size;
    win32vars.system.load_file = system_load_file;
    win32vars.system.load_close = system_load_close;
    win32vars.system.save_file = system_save_file;
    
    win32vars.system.now_time = system_now_time;
    
    win32vars.system.memory_allocate = Memory_Allocate;
    win32vars.system.memory_set_protection = Memory_Set_Protection;
    win32vars.system.memory_free = Memory_Free;
    
    win32vars.system.file_exists = File_Exists;
    win32vars.system.directory_cd = Directory_CD;
    win32vars.system.get_4ed_path = Get_4ed_Path;
    win32vars.system.show_mouse_cursor = Show_Mouse_Cursor;
    
    win32vars.system.toggle_fullscreen = Toggle_Fullscreen;
    win32vars.system.is_fullscreen = Is_Fullscreen;
    win32vars.system.send_exit_signal = Send_Exit_Signal;
    
    win32vars.system.post_clipboard = system_post_clipboard;
    
    win32vars.system.create_coroutine = system_create_coroutine;
    win32vars.system.launch_coroutine = system_launch_coroutine;
    win32vars.system.resume_coroutine = system_resume_coroutine;
    win32vars.system.yield_coroutine = system_yield_coroutine;
    
    win32vars.system.cli_call = system_cli_call;
    win32vars.system.cli_begin_update = system_cli_begin_update;
    win32vars.system.cli_update_step = system_cli_update_step;
    win32vars.system.cli_end_update = system_cli_end_update;
    
    win32vars.system.post_job = system_post_job;
    win32vars.system.cancel_job = system_cancel_job;
    win32vars.system.check_cancel = system_check_cancel;
    win32vars.system.grow_thread_memory = system_grow_thread_memory;
    win32vars.system.acquire_lock = system_acquire_lock;
    win32vars.system.release_lock = system_release_lock;
    
#if FRED_INTERNAL
    win32vars.system.internal_sentinel = INTERNAL_system_sentinel;
    win32vars.system.internal_get_thread_states = INTERNAL_get_thread_states;
    win32vars.system.internal_debug_message = INTERNAL_system_debug_message;
#endif
    
    win32vars.system.slash = '/';
}

internal void
Win32LoadRenderCode(){
    win32vars.target.push_clip = draw_push_clip;
    win32vars.target.pop_clip = draw_pop_clip;
    win32vars.target.push_piece = draw_push_piece;
    
    win32vars.target.font_set.font_load = system_draw_font_load;
    win32vars.target.font_set.release_font = draw_release_font;
}

//
// Helpers
//

globalvar u8 keycode_lookup_table[255];

internal void
Win32KeycodeInit(){
    keycode_lookup_table[VK_BACK] = key_back;
    keycode_lookup_table[VK_DELETE] = key_del;
    keycode_lookup_table[VK_UP] = key_up;
    keycode_lookup_table[VK_DOWN] = key_down;
    keycode_lookup_table[VK_LEFT] = key_left;
    keycode_lookup_table[VK_RIGHT] = key_right;
    keycode_lookup_table[VK_INSERT] = key_insert;
    keycode_lookup_table[VK_HOME] = key_home;
    keycode_lookup_table[VK_END] = key_end;
    keycode_lookup_table[VK_PRIOR] = key_page_up;
    keycode_lookup_table[VK_NEXT] = key_page_down;
    keycode_lookup_table[VK_ESCAPE] = key_esc;
    
    keycode_lookup_table[VK_F1] = key_f1;
    keycode_lookup_table[VK_F2] = key_f2;
    keycode_lookup_table[VK_F3] = key_f3;
    keycode_lookup_table[VK_F4] = key_f4;
    keycode_lookup_table[VK_F5] = key_f5;
    keycode_lookup_table[VK_F6] = key_f6;
    keycode_lookup_table[VK_F7] = key_f7;
    keycode_lookup_table[VK_F8] = key_f8;
    keycode_lookup_table[VK_F9] = key_f9;
    
    keycode_lookup_table[VK_F10] = key_f10;
    keycode_lookup_table[VK_F11] = key_f11;
    keycode_lookup_table[VK_F12] = key_f12;
    keycode_lookup_table[VK_F13] = key_f13;
    keycode_lookup_table[VK_F14] = key_f14;
    keycode_lookup_table[VK_F15] = key_f15;
    keycode_lookup_table[VK_F16] = key_f16;
}

internal void
Win32RedrawScreen(HDC hdc){
    launch_rendering(&win32vars.target);
    glFlush();
    SwapBuffers(hdc);
}

internal void
Win32Resize(i32 width, i32 height){
    if (width > 0 && height > 0){
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glScissor(0, 0, width, height);
        
        win32vars.target.width = width;
        win32vars.target.height = height;
    }
}

internal void
Win32SetCursorFromUpdate(Application_Mouse_Cursor cursor){
    switch (cursor){
        case APP_MOUSE_CURSOR_ARROW:
        SetCursor(win32vars.cursor_arrow); break;
        
        case APP_MOUSE_CURSOR_IBEAM:
        SetCursor(win32vars.cursor_ibeam); break;
        
        case APP_MOUSE_CURSOR_LEFTRIGHT:
        SetCursor(win32vars.cursor_leftright); break;
        
        case APP_MOUSE_CURSOR_UPDOWN:
        SetCursor(win32vars.cursor_updown); break;
    }
}

internal u64
Win32HighResolutionTime(){
    u64 result = 0;
    LARGE_INTEGER t;
    if (QueryPerformanceCounter(&t)){
        result = (u64)t.QuadPart / win32vars.count_per_usecond;
    }
    return(result);
}



internal LRESULT
Win32Callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    
    switch (uMsg){
        case WM_MENUCHAR:
        case WM_SYSCHAR:break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            win32vars.got_useful_event = 1;
            switch (wParam){
                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                case VK_MENU:case VK_LMENU:case VK_RMENU:
                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:
                {
                    Control_Keys *controls = 0;
                    b8 *control_keys = 0;
                    controls = &win32vars.input_chunk.pers.controls;
                    control_keys = win32vars.input_chunk.pers.control_keys;
                    
                    b8 down = ((lParam & Bit_31)?(0):(1));
                    b8 is_right = ((lParam & Bit_24)?(1):(0));
                    
                    if (wParam != 255){
                        switch (wParam){
                            case VK_SHIFT:
                            {
                                control_keys[MDFR_SHIFT_INDEX] = down;
                            }break;
                            
                            case VK_CONTROL:
                            {
                                if (is_right) controls->r_ctrl = down;
                                else controls->l_ctrl = down;
                            }break;
                            
                            case VK_MENU:
                            {
                                if (is_right) controls->r_alt = down;
                                else controls->l_alt = down;
                            }break;
                        }
                        
                        b8 ctrl, alt;
                        ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
                        alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
                        
                        if (win32vars.lctrl_lalt_is_altgr){
                            if (controls->l_alt && controls->l_ctrl){
                                ctrl = 0;
                                alt = 0;
                            }
                        }
                        
                        control_keys[MDFR_CONTROL_INDEX] = ctrl;
                        control_keys[MDFR_ALT_INDEX] = alt;
                    }
                }break;
                
                default:
                {
                    b8 previous_state = ((lParam & Bit_30)?(1):(0));
                    b8 current_state = ((lParam & Bit_31)?(0):(1));
                    
                    if (current_state){
                        u8 key = keycode_lookup_table[(u8)wParam];
                        
                        i32 *count = 0;
                        Key_Event_Data *data = 0;
                        b8 *control_keys = 0;
                        i32 control_keys_size = 0;
                        
                        if (!previous_state){
                            count = &win32vars.input_chunk.trans.key_data.press_count;
                            data = win32vars.input_chunk.trans.key_data.press;
                        }
                        else{
                            count = &win32vars.input_chunk.trans.key_data.hold_count;
                            data = win32vars.input_chunk.trans.key_data.hold;
                        }
                        control_keys = win32vars.input_chunk.pers.control_keys;
                        control_keys_size = sizeof(win32vars.input_chunk.pers.control_keys);
                        
                        if (*count < KEY_INPUT_BUFFER_SIZE){
                            if (!key){
                                UINT vk = (UINT)wParam;
                                UINT scan = (UINT)((lParam >> 16) & 0x7F);
                                BYTE state[256];
                                BYTE control_state = 0;
                                WORD x1 = 0, x2 = 0, x = 0, junk_x;
                                i32 result1 = 0, result2 = 0, result = 0;
                                
                                GetKeyboardState(state);
                                x1 = 0;
                                result1 = ToAscii(vk, scan, state, &x1, 0);
                                if (result1 < 0){
                                    ToAscii(vk, scan, state, &junk_x, 0);
                                }
                                result1 = (result1 == 1);
                                if (!usable_ascii((char)x1)){
                                    result1 = 0;
                                }
                                
                                control_state = state[VK_CONTROL];
                                state[VK_CONTROL] = 0;
                                x2 = 0;
                                result2 = ToAscii(vk, scan, state, &x2, 0);
                                if (result2 < 0){
                                    ToAscii(vk, scan, state, &junk_x, 0);
                                }
                                result2 = (result2 == 1);
                                if (!usable_ascii((char)x2)){
                                    result2 = 0;
                                }
                                
                                // TODO(allen): This is becoming a really major issue.
                                // Apparently control + i outputs a '\t' which is VALID ascii
                                // according to this system. So it reports the key as '\t'.
                                // This wasn't an issue before because we were ignoring control
                                // when computing character_no_caps_lock which is what is used
                                // for commands. But that is incorrect for some keyboard layouts
                                // where control+alt is used to signal AltGr for important keys.
                                if (result1 && result2){
                                    char c1 = char_to_upper((char)x1);
                                    char cParam = char_to_upper((char)wParam);
                                    
                                    if ((c1 == '\n' || c1 == '\r') && cParam != VK_RETURN){
                                        result1 = 0;
                                    }
                                    if (c1 == '\t' && cParam != VK_TAB){
                                        result1 = 0;
                                    }
                                }
                                
                                if (result1){
                                    x = x1;
                                    state[VK_CONTROL] = control_state;
                                    result = 1;
                                }
                                else if (result2){
                                    x = x2;
                                    result = 1;
                                }
                                
                                if (result == 1 && x < 128){
                                    key = (u8)x;
                                    if (key == '\r') key = '\n';
                                    data[*count].character = key;
                                    
                                    state[VK_CAPITAL] = 0;
                                    x = 0;
                                    result = ToAscii(vk, scan, state, &x, 0);
                                    if (result < 0){
                                        ToAscii(vk, scan, state, &junk_x, 0);
                                    }
                                    result = (result == 1);
                                    if (!usable_ascii((char)x)){
                                        result = 0;
                                    }
                                    
                                    if (result){
                                        key = (u8)x;
                                        if (key == '\r') key = '\n';
                                        data[*count].character_no_caps_lock = key;
                                        data[*count].keycode = key;
                                    }
                                }
                                if (result != 1 || x >= 128){
                                    data[*count].character = 0;
                                    data[*count].character_no_caps_lock = 0;
                                    data[*count].keycode = 0;
                                }
                            }
                            else{
                                data[*count].character = 0;
                                data[*count].character_no_caps_lock = 0;
                                data[*count].keycode = key;
                            }
                            memcpy(data[*count].modifiers, control_keys, control_keys_size);
                            data[*count].modifiers[MDFR_HOLD_INDEX] = previous_state;
                            ++(*count);
                        }
                    }
                    
                    result = DefWindowProc(hwnd, uMsg, wParam, lParam);
                }break;
            }/* switch */
        }break;
        
        case WM_MOUSEMOVE:
        {
            i32 new_x = LOWORD(lParam);
            i32 new_y = HIWORD(lParam);
            
            if (new_x != win32vars.input_chunk.pers.mouse_x
                || new_y != win32vars.input_chunk.pers.mouse_y){
                win32vars.input_chunk.pers.mouse_x = new_x;
                win32vars.input_chunk.pers.mouse_y = new_y;
                
                win32vars.got_useful_event = 1;
            }
        }break;
        
        case WM_MOUSEWHEEL:
        {
            win32vars.got_useful_event = 1;
            i16 rotation = GET_WHEEL_DELTA_WPARAM(wParam);
            if (rotation > 0){
                win32vars.input_chunk.trans.mouse_wheel = 1;
            }
            else{
                win32vars.input_chunk.trans.mouse_wheel = -1;
            }
        }break;
        
        case WM_LBUTTONDOWN:
        {
            win32vars.got_useful_event = 1;
            win32vars.input_chunk.trans.mouse_l_press = 1;
            win32vars.input_chunk.pers.mouse_l = 1;
        }break;
        
        case WM_RBUTTONDOWN:
        {
            win32vars.got_useful_event = 1;
            win32vars.input_chunk.trans.mouse_r_press = 1;
            win32vars.input_chunk.pers.mouse_r = 1;
        }break;
        
        case WM_LBUTTONUP:
        {
            win32vars.got_useful_event = 1;
            win32vars.input_chunk.trans.mouse_l_release = 1;
            win32vars.input_chunk.pers.mouse_l = 0;
        }break;
        
        case WM_RBUTTONUP:
        {
            win32vars.got_useful_event = 1;
            win32vars.input_chunk.trans.mouse_r_release = 1;
            win32vars.input_chunk.pers.mouse_r = 0;
        }break;
        
        case WM_KILLFOCUS:
        case WM_SETFOCUS:
        {
            win32vars.got_useful_event = 1;
            win32vars.input_chunk.pers.mouse_l = 0;
            win32vars.input_chunk.pers.mouse_r = 0;
            
            win32vars.input_chunk.pers.controls = control_keys_zero();
            
            if (uMsg == WM_SETFOCUS){
                Win32FixFullscreenLoseFocus(false);
            }
        }break;
        
        case WM_SIZE:
        {
            win32vars.got_useful_event = 1;
            if (win32vars.target.handle){
                i32 new_width = LOWORD(lParam);
                i32 new_height = HIWORD(lParam);
                
                Win32Resize(new_width, new_height);
            }
        }break;
        
        case WM_PAINT:
        {
            win32vars.got_useful_event = 1;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            Win32RedrawScreen(hdc);
            EndPaint(hwnd, &ps);
        }break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            win32vars.got_useful_event = 1;
            win32vars.input_chunk.trans.trying_to_kill = 1;
        }break;
        
        case WM_4coder_ANIMATE:
        win32vars.got_useful_event = 1;
        break;
        
        case WM_CANCELMODE:
        {
            Win32FixFullscreenLoseFocus(true);
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }
    
    return(result);
}

#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEBUG_OUTPUT 0x92E0

typedef void GLDEBUGPROC_TYPE(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char * message, const GLvoid * userParam);
typedef GLDEBUGPROC_TYPE * GLDEBUGPROC;
typedef void glDebugMessageControl_type(GLenum source, GLenum type, GLenum severity, GLsizei count, GLuint * ids, GLboolean enabled);
typedef void glDebugMessageCallback_type(GLDEBUGPROC callback, void * userParam);

internal void
OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

int
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow){
    
    i32 argc = __argc;
    char **argv = __argv;
    
    memset(&win32vars, 0, sizeof(win32vars));
    
    win32vars.GlobalWindowPosition.length = sizeof(win32vars.GlobalWindowPosition);
    
    //
    // Threads and Coroutines
    //
    
    // NOTE(allen): These should come before threads are started!
    // Threads now get memory right away and so they use
    // the internal_bubble and DEBUG_sysmem_lock
    
#if FRED_INTERNAL
    win32vars.internal_bubble.next = &win32vars.internal_bubble;
    win32vars.internal_bubble.prev = &win32vars.internal_bubble;
    win32vars.internal_bubble.flags = 0;
    
    InitializeCriticalSection(&win32vars.DEBUG_sysmem_lock);
#endif
    
    for (i32 i = 0; i < LOCK_COUNT; ++i){
        InitializeCriticalSection(&win32vars.locks[i]);
    }
    
    for (i32 i = 0; i < CV_COUNT; ++i){
        InitializeConditionVariable(&win32vars.condition_vars[i]);
    }
    
    
    Thread_Context background[4];
    memset(background, 0, sizeof(background));
    win32vars.groups[BACKGROUND_THREADS].threads = background;
    win32vars.groups[BACKGROUND_THREADS].count = ArrayCount(background);
    win32vars.groups[BACKGROUND_THREADS].cancel_lock0 = CANCEL_LOCK0;
    win32vars.groups[BACKGROUND_THREADS].cancel_cv0 = CANCEL_CV0;
    
    Thread_Memory thread_memory[ArrayCount(background)];
    win32vars.thread_memory = thread_memory;
    
    win32vars.queues[BACKGROUND_THREADS].semaphore =
        Win32Handle(CreateSemaphore(0, 0,
                                    win32vars.groups[BACKGROUND_THREADS].count,
                                    0));
    
    u32 creation_flag = 0;
    for (i32 i = 0; i < win32vars.groups[BACKGROUND_THREADS].count; ++i){
        Thread_Context *thread = win32vars.groups[BACKGROUND_THREADS].threads + i;
        thread->id = i + 1;
        thread->group_id = BACKGROUND_THREADS;
        
        Thread_Memory *memory = win32vars.thread_memory + i;
        *memory = thread_memory_zero();
        memory->id = thread->id;
        
        thread->queue = &win32vars.queues[BACKGROUND_THREADS];
        thread->handle = CreateThread(0, 0, JobThreadProc, thread, creation_flag, (LPDWORD)&thread->windows_id);
    }
    
    initialize_unbounded_queue(&win32vars.groups[BACKGROUND_THREADS].queue);
    
    ConvertThreadToFiber(0);
    win32vars.coroutine_free = win32vars.coroutine_data;
    for (i32 i = 0; i+1 < ArrayCount(win32vars.coroutine_data); ++i){
        win32vars.coroutine_data[i].next = win32vars.coroutine_data + i + 1;
    }
    
    //
    // Volume Initialization
    //
    win32_init_drive_strings(&win32vars.dstrings);
    
    //
    // Memory Initialization
    //
    
    LPVOID base;
#if FRED_INTERNAL
    base = (LPVOID)Tbytes(1);
#else
    base = (LPVOID)0;
#endif
    
    memory_vars.vars_memory_size = Mbytes(2);
    memory_vars.vars_memory = VirtualAlloc(base, memory_vars.vars_memory_size,
                                           MEM_COMMIT | MEM_RESERVE,
                                           PAGE_READWRITE);
    
#if FRED_INTERNAL
    base = (LPVOID)Tbytes(2);
#else
    base = (LPVOID)0;
#endif
    memory_vars.target_memory_size = Mbytes(512);
    memory_vars.target_memory =
        VirtualAlloc(base, memory_vars.target_memory_size,
                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    base = (LPVOID)0;
    memory_vars.user_memory_size = Mbytes(2);
    memory_vars.user_memory =
        VirtualAlloc(base, memory_vars.target_memory_size,
                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    if (!memory_vars.vars_memory){
        exit(1);
    }
    
    win32vars.target.max = Mbytes(1);
    win32vars.target.push_buffer = (char*)system_get_memory(win32vars.target.max);
    
    
    //
    // System and Application Layer Linkage
    //
    
    if (!Win32LoadAppCode()){
        exit(1);
    }
    
    Win32LoadSystemCode();
    
    Win32LoadRenderCode();
    
    
    //
    // Shared Systems Init
    //
    
    init_shared_vars();
    
    
    //
    // Read Command Line
    //
    
    DWORD required = (GetCurrentDirectory(0, 0)*4) + 1;
    char *current_directory_mem = (char*)system_get_memory(required);
    DWORD written = GetCurrentDirectory(required, current_directory_mem);
    
    String current_directory = make_string_cap(current_directory_mem, written, required);
    terminate_with_null(&current_directory);
    replace_char(&current_directory, '\\', '/');
    
    Command_Line_Parameters clparams;
    clparams.argv = argv;
    clparams.argc = argc;
    
    char **files = 0;
    i32 *file_count = 0;
    
    win32vars.app.read_command_line(&win32vars.system,
                                    &memory_vars,
                                    current_directory,
                                    &win32vars.settings,
                                    &files, &file_count,
                                    clparams);
    
    sysshared_filter_real_files(files, file_count);
    
    
    //
    // Custom Layer Linkage
    //
    
#if defined(FRED_SUPER)
    char *custom_file_default = "4coder_custom.dll";
    char *custom_file = 0;
    if (win32vars.settings.custom_dll) custom_file = win32vars.settings.custom_dll;
    else custom_file = custom_file_default;
    
    win32vars.custom = LoadLibraryA(custom_file);
    if (!win32vars.custom && custom_file != custom_file_default){
        if (!win32vars.settings.custom_dll_is_strict){
            win32vars.custom = LoadLibraryA(custom_file_default);
        }
    }
    
    if (win32vars.custom){
        win32vars.custom_api.get_alpha_4coder_version = (_Get_Version_Function*)
            GetProcAddress(win32vars.custom, "get_alpha_4coder_version");
        
        if (win32vars.custom_api.get_alpha_4coder_version == 0 ||
                win32vars.custom_api.get_alpha_4coder_version(MAJOR, MINOR, PATCH) == 0){
            MessageBoxA(0,"Error: The application and custom version numbers don't match.\n", "Error",0);
            exit(1);
        }
        win32vars.custom_api.get_bindings = (Get_Binding_Data_Function*)
            GetProcAddress(win32vars.custom, "get_bindings");
        
        // NOTE(allen): I am temporarily taking the view routine
        // back out, it will be back soon.
#if 0
        win32vars.custom_api.view_routine = (View_Routine_Function*)
            GetProcAddress(win32vars.custom, "view_routine");
#endif
    }
    
    if (win32vars.custom_api.get_bindings == 0){
        MessageBoxA(0,"Error: The custom dll is missing.\n", "Error",0);
        exit(1);
    }
    
#else
    win32vars.custom_api.get_bindings = (Get_Binding_Data_Function*)get_bindings;
#endif
    
    win32vars.custom_api.view_routine = (View_Routine_Function*)0;
    
#if 0
    if (win32vars.custom_api.view_routine == 0){
        win32vars.custom_api.view_routine = (View_Routine_Function*)view_routine;
    }
#endif
    
    
    //
    // Window and GL Initialization
    //
    
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = Win32Callback;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "4coder-win32-wndclass";
    window_class.hIcon = LoadIcon(hInstance, "main");
    
    if (!RegisterClass(&window_class)){
        exit(1);
    }
    
    RECT window_rect = {};
    
    if (win32vars.settings.set_window_size){
        window_rect.right = win32vars.settings.window_w;
        window_rect.bottom = win32vars.settings.window_h;
    }
    else{
        window_rect.right = 800;
        window_rect.bottom = 600;
    }
    
    if (!AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)){
        // TODO(allen): non-fatal diagnostics
    }
    
#define WINDOW_NAME "4coder-window: " VERSION
    
    i32 window_x = CW_USEDEFAULT;
    i32 window_y = CW_USEDEFAULT;
    
    if (win32vars.settings.set_window_pos){
        window_x = win32vars.settings.window_x;
        window_y = win32vars.settings.window_y;
    }
    
    i32 window_style = WS_OVERLAPPEDWINDOW;
    if (!win32vars.settings.fullscreen_window && win32vars.settings.maximize_window){
        window_style |= WS_MAXIMIZE;
    }
    
    win32vars.window_handle =
        CreateWindowA(window_class.lpszClassName,
                      WINDOW_NAME, window_style,
                      window_x, window_y,
                      window_rect.right - window_rect.left,
                      window_rect.bottom - window_rect.top,
                      0, 0, hInstance, 0);
    
    if (win32vars.window_handle == 0){
        exit(1);
    }
    
    HDC hdc = GetDC(win32vars.window_handle);
    
#if SUPPORT_DPI
    // TODO(allen): not Windows XP compatible, how do I handle that?
    SetProcessDPIAware();
    
    win32vars.dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
    win32vars.dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
#else
    win32vars.dpi_x = 1;
    win32vars.dpi_y = 1;
#endif
    
    GetClientRect(win32vars.window_handle, &window_rect);
    
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0 };
    
    {
        i32 pixel_format;
        pixel_format = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pixel_format, &pfd);
        
        win32vars.target.handle = hdc;
        win32vars.target.context = wglCreateContext(hdc);
        wglMakeCurrent(hdc, (HGLRC)win32vars.target.context);
    }
    ReleaseDC(win32vars.window_handle, hdc);
    
#if FRED_INTERNAL
    // NOTE(casey): This slows down GL but puts error messages to
    // the debug console immediately whenever you do something wrong
    glDebugMessageCallback_type *glDebugMessageCallback =
        (glDebugMessageCallback_type *)wglGetProcAddress("glDebugMessageCallback");
    glDebugMessageControl_type *glDebugMessageControl =
        (glDebugMessageControl_type *)wglGetProcAddress("glDebugMessageControl");
    if(glDebugMessageCallback && glDebugMessageControl)
    {
        glDebugMessageCallback(OpenGLDebugCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Win32Resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    
    //
    // Misc System Initializations
    //
    
    win32vars.clipboard_sequence = GetClipboardSequenceNumber();
    if (win32vars.clipboard_sequence == 0){
        system_post_clipboard(make_lit_string(""));
        
        win32vars.clipboard_sequence = GetClipboardSequenceNumber();
        win32vars.next_clipboard_is_self = 0;
        
        if (win32vars.clipboard_sequence == 0){
            OutputDebugStringA("Failure while initializing clipboard\n");
        }
    }
    else{
        Win32ReadClipboardContents();
    }
    
    Win32KeycodeInit();
    
    win32vars.cursor_ibeam = LoadCursor(NULL, IDC_IBEAM);
    win32vars.cursor_arrow = LoadCursor(NULL, IDC_ARROW);
    win32vars.cursor_leftright = LoadCursor(NULL, IDC_SIZEWE);
    win32vars.cursor_updown = LoadCursor(NULL, IDC_SIZENS);
    
    LARGE_INTEGER f;
    if (QueryPerformanceFrequency(&f)){
        win32vars.count_per_usecond = (u64)f.QuadPart / 1000000;
    }
    else{
        // NOTE(allen): Just guess.
        win32vars.count_per_usecond = 1;
    }
    
    //
    // Main Loop
    //
    
    win32vars.app.init(&win32vars.system,
                       &win32vars.target,
                       &memory_vars,
                       win32vars.clipboard_contents,
                       current_directory,
                       win32vars.custom_api);
    
    system_free_memory(current_directory.str);
    
    b32 keep_playing = 1;
    win32vars.first = 1;
    timeBeginPeriod(1);
    
    if (win32vars.settings.fullscreen_window){
        Win32ToggleFullscreen();
    }
    
    SetForegroundWindow(win32vars.window_handle);
    SetActiveWindow(win32vars.window_handle);
    ShowWindow(win32vars.window_handle, SW_SHOW);
    
    u64 timer_start = Win32HighResolutionTime();
    system_acquire_lock(FRAME_LOCK);
    MSG msg;
    for (;keep_playing;){
        // TODO(allen): Find a good way to wait on a pipe
        // without interfering with the reading process
        //  Looks like we can ReadFile with a size of zero
        // in an IOCP for this effect.
        
        system_release_lock(FRAME_LOCK);
        
        if (win32vars.running_cli == 0){
            win32vars.got_useful_event = 0;
            for (;win32vars.got_useful_event == 0;){
                if (GetMessage(&msg, 0, 0, 0)){
                    if (msg.message == WM_QUIT){
                        keep_playing = 0;
                    }else{
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
        }
        
        while (PeekMessage(&msg, 0, 0, 0, 1)){
            if (msg.message == WM_QUIT){
                keep_playing = 0;
            }else{
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        system_acquire_lock(FRAME_LOCK);
        
        POINT mouse_point;
        if (GetCursorPos(&mouse_point) &&
            ScreenToClient(win32vars.window_handle, &mouse_point)){
            
            i32_Rect screen =
                i32R(0, 0, win32vars.target.width, win32vars.target.height);
            
            if (!hit_check(mouse_point.x, mouse_point.y, screen)){
                win32vars.input_chunk.trans.out_of_window = 1;
            }
            
            win32vars.input_chunk.pers.mouse_x = mouse_point.x;
            win32vars.input_chunk.pers.mouse_y = mouse_point.y;
        }
        else{
            win32vars.input_chunk.trans.out_of_window = 1;
        }
        
        Win32_Input_Chunk input_chunk = win32vars.input_chunk;
        win32vars.input_chunk.trans = null_input_chunk_transient;
        
        input_chunk.pers.control_keys[MDFR_CAPS_INDEX] = GetKeyState(VK_CAPITAL) & 0x1;
        
        win32vars.clipboard_contents = string_zero();
        if (win32vars.clipboard_sequence != 0){
            DWORD new_number = GetClipboardSequenceNumber();
            if (new_number != win32vars.clipboard_sequence){
                win32vars.clipboard_sequence = new_number;
                if (win32vars.next_clipboard_is_self){
                    win32vars.next_clipboard_is_self = 0;
                }
                else{
                    Win32ReadClipboardContents();
                }
            }
        }
        
        Application_Step_Input input = {0};
        
        input.first_step = win32vars.first;
        
        // NOTE(allen): The expected dt given the frame limit in seconds.
        input.dt = frame_useconds / 1000000.f;
        
        input.keys = input_chunk.trans.key_data;
        memcpy(input.keys.modifiers, input_chunk.pers.control_keys, sizeof(input_chunk.pers.control_keys));
        
        input.mouse.out_of_window = input_chunk.trans.out_of_window;
        
        input.mouse.l = input_chunk.pers.mouse_l;
        input.mouse.press_l = input_chunk.trans.mouse_l_press;
        input.mouse.release_l = input_chunk.trans.mouse_l_release;
        
        input.mouse.r = input_chunk.pers.mouse_r;
        input.mouse.press_r = input_chunk.trans.mouse_r_press;
        input.mouse.release_r = input_chunk.trans.mouse_r_release;
        
        input.mouse.wheel = input_chunk.trans.mouse_wheel;
        input.mouse.x = input_chunk.pers.mouse_x;
        input.mouse.y = input_chunk.pers.mouse_y;
        
        input.clipboard = win32vars.clipboard_contents;
        
        
        Application_Step_Result result = {(Application_Mouse_Cursor)0};
        result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
        result.lctrl_lalt_is_altgr = win32vars.lctrl_lalt_is_altgr;
        result.trying_to_kill = input_chunk.trans.trying_to_kill;
        result.perform_kill = 0;
        
        if (win32vars.send_exit_signal){
            result.trying_to_kill = 1;
            win32vars.send_exit_signal = 0;
        }
        
        win32vars.app.step(&win32vars.system,
                           &win32vars.target,
                           &memory_vars,
                           &input,
                           &result);
        
        if (result.perform_kill){
            keep_playing = 0;
        }
        
        if (win32vars.do_toggle){
            Win32ToggleFullscreen();
            win32vars.do_toggle = 0;
        }
        
        Win32SetCursorFromUpdate(result.mouse_cursor_type);
        win32vars.lctrl_lalt_is_altgr = result.lctrl_lalt_is_altgr;
        
        HDC hdc = GetDC(win32vars.window_handle);
        Win32RedrawScreen(hdc);
        ReleaseDC(win32vars.window_handle, hdc);
        
        win32vars.first = 0;
        
        if (result.animating){
            PostMessage(win32vars.window_handle, WM_4coder_ANIMATE, 0, 0);
        }
        
        flush_thread_group(BACKGROUND_THREADS);
        
        u64 timer_end = Win32HighResolutionTime();
        u64 end_target = timer_start + frame_useconds;
        
        system_release_lock(FRAME_LOCK);
        while (timer_end < end_target){
            DWORD samount = (DWORD)((end_target - timer_end) / 1000);
            if (samount > 0) Sleep(samount);
            timer_end = Win32HighResolutionTime();
        }
        system_acquire_lock(FRAME_LOCK);
        timer_start = Win32HighResolutionTime();
    }
    
    return(0);
}

#if 0
// NOTE(allen): In case I want to switch back to a console
// application at some point.
int main(int argc, char **argv){
    HINSTANCE hInstance = GetModuleHandle(0);
#endif

// BOTTOM


