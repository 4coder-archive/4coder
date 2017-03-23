/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for 4coder
 *
 */

// TOP

//
// Architecture cracking
//

#if defined(_M_AMD64)
# define CALL_CONVENTION
# define BUILD_X64
#elif defined(_M_IX86)
# define CALL_CONVENTION __stdcall
# define BUILD_X86
#else
# error architecture not supported yet
#endif


//
// Program setup
//

#define SUPPORT_DPI 1
#define UNICODE

#define FPS 60
#define frame_useconds (1000000 / FPS)

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

#include <Windows.h>
#include <GL/gl.h>
#include "win32_gl.h"

#define GL_TEXTURE_MAX_LEVEL 0x813D

//////////////////////////////

#include "4ed_file_track.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_system_shared.h"

//
// Win32_Vars structs
//

#define WM_4coder_ANIMATE (WM_USER + 0)

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
static Control_Keys null_control_keys = {0};

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
#if SUPPORT_DPI
    i32 dpi_x, dpi_y;
#endif
    
    u64 count_per_usecond;
    b32 first;
    i32 running_cli;
    
} Win32_Vars;

global Win32_Vars win32vars;
global Application_Memory memory_vars;


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
Sys_Memory_Allocate_Sig(system_memory_allocate){
    void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return(result);
}

internal
Sys_Memory_Set_Protection_Sig(system_memory_set_protection){
    bool32 result = false;
    DWORD old_protect = 0;
    DWORD protect = 0;
    
    flags = flags & 0x7;
    
    switch (flags){
        case 0: protect = PAGE_NOACCESS; break;
        
        case MemProtect_Read: protect = PAGE_READONLY; break;
        
        case MemProtect_Write:
        case MemProtect_Read|MemProtect_Write:
        protect = PAGE_READWRITE; break;
        
        case MemProtect_Execute: protect = PAGE_EXECUTE; break;
        
        case MemProtect_Execute|MemProtect_Read: protect = PAGE_EXECUTE_READ; break;
        
        case MemProtect_Execute|MemProtect_Write:
        case MemProtect_Execute|MemProtect_Write|MemProtect_Read:
        protect = PAGE_EXECUTE_READWRITE; break;
    }
    
    VirtualProtect(ptr, size, protect, &old_protect);
    return(result);
}

internal
Sys_Memory_Free_Sig(system_memory_free){
    VirtualFree(ptr, 0, MEM_RELEASE);
}

#define Win32GetMemory(size) system_memory_allocate(size)
#define Win32FreeMemory(ptr) system_memory_free(ptr, 0)

#define Win32ScratchPartition sysshared_scratch_partition
#define Win32ScratchPartitionGrow sysshared_partition_grow
#define Win32ScratchPartitionDouble sysshared_partition_double


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
    SleepConditionVariableCS(win32vars.condition_vars + cv_id, win32vars.locks + crit_id, INFINITE);
}

internal void
system_signal_cv(i32 crit_id, i32 cv_id){
    AllowLocal(crit_id);
    WakeConditionVariable(win32vars.condition_vars + cv_id);
}

internal DWORD CALL_CONVENTION
JobThreadProc(LPVOID lpParameter){
    Thread_Context *thread = (Thread_Context*)lpParameter;
    Work_Queue *queue = win32vars.queues + thread->group_id;
    Thread_Group *group = win32vars.groups + thread->group_id;
    
    i32 thread_index = thread->id - 1;
    
    i32 cancel_lock = group->cancel_lock0 + thread_index;
    i32 cancel_cv = group->cancel_cv0 + thread_index;
    
    Thread_Memory *thread_memory = win32vars.thread_memory + thread_index;
    
    if (thread_memory->size == 0){
        i32 new_size = KB(64);
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
            u32 safe_read_index = InterlockedCompareExchange(&queue->read_position, next_read_index, read_index);
            
            if (safe_read_index == read_index){
                Full_Job_Data *full_job = queue->jobs + safe_read_index;
                // NOTE(allen): This is interlocked so that it plays nice
                // with the cancel job routine, which may try to cancel this job
                // at the same time that we try to run it
                
                i32 safe_running_thread =InterlockedCompareExchange(&full_job->running_thread, thread->id, THREAD_NOT_ASSIGNED);
                
                if (safe_running_thread == THREAD_NOT_ASSIGNED){
                    thread->job_id = full_job->id;
                    thread->running = 1;
                    
                    full_job->job.callback(&win32vars.system, thread, thread_memory, full_job->job.data);
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
    u32 max = 512;
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
            system_memory_allocate(new_max*sizeof(Full_Job_Data));
        
        memcpy(new_jobs, queue->jobs, queue->count);
        
        system_memory_free(queue->jobs, 0);
        
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
    system_acquire_lock(CANCEL_LOCK0 + memory->id - 1);
    void *old_data = memory->data;
    i32 old_size = memory->size;
    i32 new_size = l_round_up_i32(memory->size*2, KB(4));
    memory->data = system_memory_allocate(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        system_memory_free(old_data, 0);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}

#if FRED_INTERNAL
internal void
INTERNAL_get_thread_states(Thread_Group_ID id, b8 *running, i32 *pending){
    Thread_Group *group = win32vars.groups + id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    Work_Queue *queue = win32vars.queues + id;
    u32 write = queue->write_position;
    u32 read = queue->read_position;
    if (write < read){
        write += QUEUE_WRAP;
    }
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

internal void CALL_CONVENTION
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
    b32 result = false;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    u32 len = str_size(filename);
    u32 filename_16_max = (len+1)*2;
    u16 *filename_16 = push_array(scratch, u16, filename_16_max);
    
    b32 convert_error = false;
    u32 filename_16_len = (u32)utf8_to_utf16_minimal_checking(filename_16, filename_16_max-1, (u8*)filename, len, &convert_error);
    filename_16[filename_16_len] = 0;;
    
    if (!convert_error){
        filename_16[filename_16_len] = 0;
        
        HANDLE file = CreateFile((LPCWSTR)filename_16, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != 0 && file != INVALID_HANDLE_VALUE){
            result = true;
            CloseHandle(file);
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal
Sys_Set_File_List_Sig(system_set_file_list){
    b32 clear_list = true;
    if (directory != 0){
        Partition *scratch = &shared_vars.scratch;
        Temp_Memory temp = begin_temp_memory(scratch);
        
        char dir_space[MAX_PATH + 32];
        String dir = make_string_cap(dir_space, 0, MAX_PATH + 32);
        append_sc(&dir, directory);
        terminate_with_null(&dir);
        
        umem filename_16_max = (dir.size+1)*2;
        u16 *filename_16 = push_array(scratch, u16, filename_16_max);
        b32 convert_error = false;
        umem length = utf8_to_utf16_minimal_checking(filename_16, filename_16_max-1, (u8*)dir.str, dir.size, &convert_error);
        
        if (!convert_error){
            filename_16[length] = 0;
            
            HANDLE dir_handle = CreateFile((LPWSTR)filename_16, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
            
            if (dir_handle != INVALID_HANDLE_VALUE){
                DWORD final_length = GetFinalPathNameByHandle(dir_handle, (LPWSTR)filename_16, (DWORD)filename_16_max, 0);
                CloseHandle(dir_handle);
                
                if (final_length < sizeof(dir_space)){
                    final_length -= 4;
                    memmove(filename_16, filename_16+4, final_length*sizeof(*filename_16));
                    filename_16[final_length] = '\\';
                    filename_16[final_length+1] = '*';
                    filename_16[final_length+2] = 0;
                    
                    if (canon_directory_out != 0){
                        umem out_length = utf16_to_utf8_minimal_checking((u8*)canon_directory_out, canon_directory_max-1, filename_16, final_length, &convert_error);
                        
                        if (!convert_error){
                            if (canon_directory_out[out_length-1] != '\\'){
                                canon_directory_out[out_length++] = '\\';
                            }
                            canon_directory_out[out_length] = 0;
                            *canon_directory_size_out = (u32)out_length;
                        }
                        else{
                            u32 length = copy_fast_unsafe_cc(canon_directory_out, directory);
                            canon_directory_out[length] = 0;
                            *canon_directory_size_out = length;
                        }
                    }
                    
                    WIN32_FIND_DATA find_data;
                    HANDLE search = FindFirstFile((LPWSTR)filename_16, &find_data);
                    
                    if (search != INVALID_HANDLE_VALUE){            
                        u32 character_count = 0;
                        u32 file_count = 0;
                        BOOL more_files = true;
                        do{
                            if (!(find_data.cFileName[0] == '.' && find_data.cFileName[1] == 0) &&
                                !(find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.' && find_data.cFileName[2] == 0)){
                                ++file_count;
                                u32 size = 0;
                                for(;find_data.cFileName[size];++size);
                                character_count += size + 1;
                            }
                            more_files = FindNextFile(search, &find_data);
                        }while(more_files);
                        FindClose(search);
                        
                        u32 required_size = character_count*2 + file_count * sizeof(File_Info);
                        if (file_list->block_size < (i32)required_size){
                            Win32FreeMemory(file_list->block);
                            file_list->block = system_memory_allocate(required_size);
                            file_list->block_size = required_size;
                        }
                        
                        umem remaining_size = required_size;
                        file_list->infos = (File_Info*)file_list->block;
                        u8 *name = (u8*)(file_list->infos + file_count);
                        if (file_list->block != 0){
                            search = FindFirstFile((LPWSTR)filename_16, &find_data);
                            
                            if (search != INVALID_HANDLE_VALUE){
                                File_Info *info = file_list->infos;
                                more_files = true;
                                do{
                                    if (!(find_data.cFileName[0] == '.' && find_data.cFileName[1] == 0) &&
                                        !(find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.' && find_data.cFileName[2] == 0)){
                                        info->folder = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                                        info->filename = name;
                                        
                                        u32 size = 0;
                                        for(;find_data.cFileName[size];++size);
                                        umem length = utf16_to_utf8_minimal_checking(info->filename, remaining_size, (u16*)find_data.cFileName, size, &convert_error);
                                        
                                        if (!convert_error){
                                            name += length;
                                            remaining_size += length;
                                            
                                            info->filename_len = (u32)length;
                                            *name++ = 0;
                                            String fname = make_string_cap(info->filename, info->filename_len, info->filename_len+1);
                                            replace_char(&fname, '\\', '/');
                                            ++info;
                                        }
                                    }
                                    more_files = FindNextFile(search, &find_data);
                                }while(more_files);
                                FindClose(search);
                                
                                file_list->count = file_count;
                                clear_list = false;
                            }
                        }
                    }
                }
            }
        }
        
        end_temp_memory(temp);
    }
    
    if (clear_list){
        Win32FreeMemory(file_list->block);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
    }
}

internal u32
win32_canonical_name(u16 *src, u32 len, u16 *dst, u32 max){
    u32 result = 0;
    
    if (len >= 2 && ((src[0] >= 'a' && src[0] <= 'z') || (src[0] >= 'A' && src[0] <= 'Z')) && src[1] == ':'){
        
        HANDLE file = CreateFile((LPWSTR)src, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != INVALID_HANDLE_VALUE){
            DWORD final_length = GetFinalPathNameByHandle(file, (LPWSTR)dst, max, 0);
            
            if (final_length < max && final_length >= 4){
                if (dst[final_length-1] == 0){
                    --final_length;
                }
                final_length -= 4;
                memmove(dst, dst+4, final_length*sizeof(u16));
                dst[final_length] = 0;
                result = (u32)final_length;
            }
            
            CloseHandle(file);
        }
        else{
            //String src_str = make_string(src, len);
            //String path_str = path_of_directory(src_str);
            //String front_str = front_of_directory(src_str);
            
            Partition *scratch = &shared_vars.scratch;
            Temp_Memory temp = begin_temp_memory(scratch);
            
            u16 *path_src = push_array(scratch, u16, len+1);
            memcpy(path_src, src, len*sizeof(u16));
            u32 path_end = len;
            for (u32 j = len; j > 0; --j){
                if (path_src[j] == '/' || path_src[j] == '\\'){
                    path_end = j+1;
                    break;
                }
            }
            path_src[path_end] = 0;
            
            u32 front_size = len - path_end;
            u16 *front_src = path_src + path_end;
            
            HANDLE dir = CreateFile((LPWSTR)src, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
            
            if (dir != INVALID_HANDLE_VALUE){
                DWORD final_length = GetFinalPathNameByHandle(dir, (LPWSTR)dst, max, 0);
                
                if (final_length < max && final_length >= 4){
                    if (dst[final_length-1] == 0){
                        --final_length;
                    }
                    final_length -= 4;
                    memmove(dst, dst+4, final_length);
                    dst[final_length++] = '\\';
                    memcpy(dst + final_length, front_src, front_size);
                    final_length += front_size;
                    dst[final_length] = 0;
                    result = (u32)final_length;
                }
                
                CloseHandle(dir);
            }
            
            end_temp_memory(temp);
        }
        
    }
    
    return(result);
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    u32 result = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 src_16_max = (len+1)*2;
    u16 *src_16 = push_array(scratch, u16, src_16_max);
    
    u32 dst_16_max = (max+1)*2;
    u16 *dst_16 = push_array(scratch, u16, dst_16_max);
    
    b32 convert_error = false;
    u32 src_16_len = (u32)utf8_to_utf16_minimal_checking(src_16, src_16_max-1, src, len, &convert_error);
    src_16[src_16_len] = 0;
    
    if (!convert_error){
        u32 dst_16_len = win32_canonical_name(src_16, src_16_len, dst_16, dst_16_max);
        
        result = (u32)utf16_to_utf8_minimal_checking(dst, max-1, dst_16, dst_16_len, &convert_error);
        if (!convert_error){
            dst[result] = 0;
        }
        else{
            result = 0;
        }
    }
    
    end_temp_memory(temp);
    return(result);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = false;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 len = 0;
    for (;filename[len];++len);
    
    u32 filename_16_max = (len+1)*2;
    u16 *filename_16 = push_array(scratch, u16, filename_16_max);
    
    HANDLE file = CreateFile((LPWSTR)filename_16, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE){
        *(HANDLE*)handle_out = file;
        result = true;
    }
    
    end_temp_memory(temp);
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
    
    if (ReadFile(file, buffer, size, &read_size, 0)){
        if (read_size == size){
            result = 1;
        }
    }
    
    return(result);
}

internal
Sys_Load_Close_Sig(system_load_close){
    b32 result = false;
    HANDLE file = *(HANDLE*)(&handle);
    if (CloseHandle(file)){
        result = true;
    }
    return(result);
}

internal
Sys_Save_File_Sig(system_save_file){
    b32 result = false;
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 len = 0;
    for (;filename[len];++len);
    
    u32 filename_16_max = (len+1)*2;
    u16 *filename_16 = push_array(scratch, u16, filename_16_max);
    
    b32 convert_error = false;
    u32 filename_16_len = (u32)utf8_to_utf16_minimal_checking(filename_16, filename_16_max-1, (u8*)filename, len, &convert_error);
    
    if (!convert_error){
        filename_16[filename_16_len] = 0;
        
        HANDLE file = CreateFile((LPWSTR)filename_16, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (file != INVALID_HANDLE_VALUE){
            DWORD written_total = 0;
            DWORD written_size = 0;
            
            result = true;
            
            while (written_total < size){
                if (!WriteFile(file, buffer + written_total, size - written_total, &written_size, 0)){
                    result = false;
                    break;
                }
                written_total += written_size;
            }
            
            CloseHandle(file);
        }
    }
    
    end_temp_memory(temp);
    return(result);
}

internal
Sys_Now_Time_Sig(system_now_time){
    u64 result = __rdtsc();
    return(result);
}

internal b32
Win32DirectoryExists(u8 *path){
    b32 result = false;
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 len = 0;
    for(;path[len];++len);
    
    u32 path_16_max = (len+1)*2;
    u16 *path_16 = push_array(scratch, u16, path_16_max);
    
    b32 convert_error = false;
    u32 path_16_len = (u32)utf8_to_utf16_minimal_checking(path_16, path_16_max-1, path, len, &convert_error);
    
    if (!convert_error){
        path_16[path_16_len] = 0;
        
        DWORD attrib = GetFileAttributes((LPWSTR)path_16);
        result = (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
    }
    
    end_temp_memory(temp);
    return(result);
}

internal
Sys_Get_Binary_Path_Sig(system_get_binary_path){
    i32 result = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 filename_16_max = (out->memory_size+1)*2;
    u16 *filename_16 = push_array(scratch, u16, filename_16_max);
    
    u32 length_16 = GetModuleFileName(0, (LPWSTR)filename_16, filename_16_max);
    
    b32 convert_error = false;
    u32 size = (u32)utf16_to_utf8_minimal_checking((u8*)out->str, out->memory_size-1, filename_16, length_16, &convert_error);
    
    if (!convert_error){
        out->size = size;
        remove_last_folder(out);
        terminate_with_null(out);
        result = out->size;
    }
    
    end_temp_memory(temp);
    return(result);
}

internal
Sys_File_Exists_Sig(system_file_exists){
    b32 result = false;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    u32 filename_16_max = (len+1)*2;
    u16 *filename_16 = push_array(scratch, u16, filename_16_max);
    
    b32 convert_error = false;
    u32 filename_16_len = (u32)utf8_to_utf16_minimal_checking(filename_16, filename_16_max, (u8*)filename, len, &convert_error);
    
    if (!convert_error){
        filename_16[filename_16_len] = 0;
        
        HANDLE file = CreateFile((LPWSTR)filename_16, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (file != INVALID_HANDLE_VALUE){
            CloseHandle(file);
            result = true;
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal
Sys_Directory_CD_Sig(system_directory_cd){
    String directory = make_string_cap(dir, *len, cap);
    b32 result = false;
    i32 old_size;
    
    char rel_path_space[1024];
    String rel_path_string = make_fixed_width_string(rel_path_space);
    copy_ss(&rel_path_string, make_string(rel_path, rel_len));
    terminate_with_null(&rel_path_string);
    
    if (rel_path[0] != 0){
        if (rel_path[0] == '.' && rel_path[1] == 0){
            result = true;
        }
        else if (rel_path[0] == '.' && rel_path[1] == '.' && rel_path[2] == 0){
            result = remove_last_folder(&directory);
            terminate_with_null(&directory);
        }
        else{
            if (directory.size + rel_len + 1 > directory.memory_size){
                old_size = directory.size;
                append_partial_sc(&directory, rel_path);
                append_s_char(&directory, '\\');
                if (Win32DirectoryExists((u8*)directory.str)){
                    result = true;
                }
                else{
                    directory.size = old_size;
                }
            }
        }
    }
    
    *len = directory.size;
    
    return(result);
}

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    String str = make_string_cap(out, 0, capacity);
    int32_t size = system_get_binary_path(&str);
    return(size);
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

internal b32
win32_create_process_utf8(char *cmd, char *command_line, char *path, STARTUPINFO *startup, PROCESS_INFORMATION *info){
    b32 result = false;
    LPWSTR env_variables = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 cmd_len = 0;
    for (;cmd[cmd_len];++cmd_len);
    
    u32 command_line_len = 0;
    for (;command_line[command_line_len];++command_line_len);
    
    u32 path_len = 0;
    for (;path[path_len];++path_len);
    
    u32 cmd_16_max = (cmd_len+1)*2;
    u32 command_line_16_max = (command_line_len+1)*2;
    u32 path_16_max = (path_len+1)*2;
    
    u16 *cmd_16 = push_array(scratch, u16, cmd_16_max);
    u16 *command_line_16 = push_array(scratch, u16, command_line_16_max);
    u16 *path_16 = push_array(scratch, u16, path_16_max);
    
    b32 convert_error = false;
    u32 cmd_16_len = (u32)utf8_to_utf16_minimal_checking(cmd_16, cmd_16_max - 1, (u8*)cmd, cmd_len, &convert_error);
    if (!convert_error){
        u32 command_line_16_len = (u32)utf8_to_utf16_minimal_checking(command_line_16, command_line_16_max - 1, (u8*)command_line, command_line_len, &convert_error);
        
        if (!convert_error){
            u32 path_16_len = (u32)utf8_to_utf16_minimal_checking(path_16, path_16_max - 1, (u8*)path, path_len, &convert_error);
            
            if (!convert_error){
                cmd_16[cmd_16_len] = 0;
                command_line_16[command_line_16_len] = 0;
                path_16[path_16_len] = 0;
                
                result = CreateProcess((LPWSTR)cmd_16, (LPWSTR)command_line_16, 0, 0, TRUE, 0, env_variables, (LPWSTR)path_16, startup, info);
            }
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal
Sys_CLI_Call_Sig(system_cli_call){
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
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
                if (win32_create_process_utf8(cmd, command_line, path, &startup, &info)){
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
        if (GetExitCodeProcess(proc, &result) == 0){
            cli->exit = -1;
        }
        else{
            cli->exit = (i32)result;
        }
        
        close_me = 1;
        CloseHandle(*(HANDLE*)&cli->proc);
        CloseHandle(*(HANDLE*)&cli->out_read);
        CloseHandle(*(HANDLE*)&cli->out_write);
        
        --win32vars.running_cli;
    }
    return(close_me);
}

//
// Appearence Settings
//

// TODO(allen): add a "shown but auto-hides on timer" setting here.
internal
Sys_Show_Mouse_Cursor_Sig(system_show_mouse_cursor){
    switch (show){
        case MouseCursorShow_Never:
        ShowCursor(false);
        break;
        
        case MouseCursorShow_Always:
        ShowCursor(true);
        break;
        
        // TODO(allen): MouseCursor_HideWhenStill
    }
}

internal
Sys_Toggle_Fullscreen_Sig(system_toggle_fullscreen){
    /* NOTE(allen): Don't actually change window size now!
    Tell the platform layer to do the toggle (or to cancel the toggle)
    later when the app.step function isn't running. If the size changes
    mid step, it messes up the rendering rules and stuff. */
    
    b32 success = false;
    
    // NOTE(allen): On windows we must be in stream mode to go fullscreen.
    if (win32vars.settings.stream_mode){
        win32vars.do_toggle = !win32vars.do_toggle;
        success = true;
    }
    
    return(success);
}

internal
Sys_Is_Fullscreen_Sig(system_is_fullscreen){
    /* NOTE(allen): This is a fancy way to say 'full_screen XOR do_toggle'
    This way this function can always report the state the fullscreen
    will have when the next frame runs, given the number of toggles
    that have occurred this frame and the original value. */
    bool32 result = (win32vars.full_screen + win32vars.do_toggle) & 1;
    return(result);
}

internal
Sys_Send_Exit_Signal_Sig(system_send_exit_signal){
    win32vars.send_exit_signal = 1;
}

#include "4ed_system_shared.cpp"

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
    
    win32vars.system.memory_allocate = system_memory_allocate;
    win32vars.system.memory_set_protection = system_memory_set_protection;
    win32vars.system.memory_free = system_memory_free;
    win32vars.system.file_exists = system_file_exists;
    win32vars.system.directory_cd = system_directory_cd;
    win32vars.system.get_4ed_path = system_get_4ed_path;
    win32vars.system.toggle_fullscreen = system_toggle_fullscreen;
    win32vars.system.is_fullscreen = system_is_fullscreen;win32vars.system.show_mouse_cursor = system_show_mouse_cursor;
    win32vars.system.send_exit_signal = system_send_exit_signal;
    
#if FRED_INTERNAL
    win32vars.system.internal_get_thread_states = INTERNAL_get_thread_states;
#endif
}

internal void
Win32LoadRenderCode(){
    win32vars.target.push_clip = draw_push_clip;
    win32vars.target.pop_clip = draw_pop_clip;
    win32vars.target.push_piece = draw_push_piece;
}

//
// Helpers
//

global Key_Code keycode_lookup_table[255];

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
    launch_rendering(&win32vars.system, &win32vars.target);
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

internal void*
win32_load_gl_always(char *name, HMODULE module){
    void *p = (void *)wglGetProcAddress(name), *r = 0;
    if(p == 0 ||
       (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
       (p == (void*)-1) ){
        r = (void *)GetProcAddress(module, name);
    }
    else{
        r = p;
    }
    return(r);
}

internal void CALL_CONVENTION
OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam){
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

internal void
Win32InitGL(){
    // GL context initialization
    {
        PIXELFORMATDESCRIPTOR format;
        format.nSize = sizeof(format);
        format.nVersion = 1;
        format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
        format.iPixelType = PFD_TYPE_RGBA;
        format.cColorBits = 32;
        format.cRedBits = 0;
        format.cRedShift = 0;
        format.cGreenBits = 0;
        format.cGreenShift = 0;
        format.cBlueBits = 0;
        format.cBlueShift = 0;
        format.cAlphaBits = 0;
        format.cAlphaShift = 0;
        format.cAccumBits = 0;
        format.cAccumRedBits = 0;
        format.cAccumGreenBits = 0;
        format.cAccumBlueBits = 0;
        format.cAccumAlphaBits = 0;
        format.cDepthBits = 24;
        format.cStencilBits = 8;
        format.cAuxBuffers = 0;
        format.iLayerType = PFD_MAIN_PLANE;
        format.bReserved = 0;
        format.dwLayerMask = 0;
        format.dwVisibleMask = 0;
        format.dwDamageMask = 0;
        
        HDC dc = GetDC(win32vars.window_handle);
        Assert(dc);
        int format_id = ChoosePixelFormat(dc, &format);
        Assert(format_id != 0);
        BOOL success = SetPixelFormat(dc, format_id, &format);
        Assert(success == TRUE); AllowLocal(success);
        
        HGLRC glcontext = wglCreateContext(dc);
        wglMakeCurrent(dc, glcontext);
        
        HMODULE module = LoadLibraryA("opengl32.dll");
        AllowLocal(module);
        
        wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
        wglCreateContextAttribsARB = (wglCreateContextAttribsARB_Function*)
            win32_load_gl_always("wglCreateContextAttribsARB", module);
        
        wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
        wglChoosePixelFormatARB = (wglChoosePixelFormatARB_Function*)
            win32_load_gl_always("wglChoosePixelFormatARB", module);
        
        if (wglCreateContextAttribsARB != 0 && wglChoosePixelFormatARB != 0){
            const int choosePixel_attribList[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, TRUE,
                WGL_SUPPORT_OPENGL_ARB, TRUE,
                //WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, 32,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 8,
                0,
            };
            
            i32 extended_format_id = 0;
            u32 num_formats = 0;
            BOOL result =  wglChoosePixelFormatARB(dc, choosePixel_attribList, 0, 1, &extended_format_id, &num_formats);
            
            if (result != 0 && num_formats > 0){
                const int createContext_attribList[] = {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                    0
                };
                
                if (extended_format_id == format_id){
                    HGLRC extended_context = wglCreateContextAttribsARB(dc, 0, createContext_attribList);
                    if (extended_context){
                        wglMakeCurrent(dc, extended_context);
                        wglDeleteContext(glcontext);
                        glcontext = extended_context;
                    }
                }
            }
        }
        
#if (defined(BUILD_X64) && 1) || (defined(BUILD_X86) && 0)
#if FRED_INTERNAL
        // NOTE(casey): This slows down GL but puts error messages to
        // the debug console immediately whenever you do something wrong
        glDebugMessageCallback_type *glDebugMessageCallback = 
            (glDebugMessageCallback_type *)win32_load_gl_always("glDebugMessageCallback", module);
        glDebugMessageControl_type *glDebugMessageControl = 
            (glDebugMessageControl_type *)win32_load_gl_always("glDebugMessageControl", module);
        if(glDebugMessageCallback != 0 && glDebugMessageControl != 0)
        {
            glDebugMessageCallback(OpenGLDebugCallback, 0);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
#endif
#endif
        
        ReleaseDC(win32vars.window_handle, dc);
    }
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
        case WM_MENUCHAR:break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            switch (wParam){
                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                case VK_MENU:case VK_LMENU:case VK_RMENU:
                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:
                {
                    Control_Keys *controls = &win32vars.input_chunk.pers.controls;
                    b8 *control_keys = win32vars.input_chunk.pers.control_keys;
                    
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
                        
                        b8 ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
                        b8 alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
                        
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
                    b8 current_state = ((lParam & Bit_31)?(0):(1));
                    
                    if (current_state){
                        Key_Code key = keycode_lookup_table[(u8)wParam];
                        
                        if (key != 0){
                            i32 *count = &win32vars.input_chunk.trans.key_data.count;
                            Key_Event_Data *data = win32vars.input_chunk.trans.key_data.keys;
                            b8 *control_keys = win32vars.input_chunk.pers.control_keys;
                            i32 control_keys_size = sizeof(win32vars.input_chunk.pers.control_keys);
                            
                            Assert(*count < KEY_INPUT_BUFFER_SIZE);
                            data[*count].character = 0;
                            data[*count].character_no_caps_lock = 0;
                            data[*count].keycode = key;
                            memcpy(data[*count].modifiers, control_keys, control_keys_size);
                            ++(*count);
                            
                            win32vars.got_useful_event = 1;
                        }
                    }
                }break;
            }/* switch */
        }break;
        
        case WM_CHAR: case WM_SYSCHAR: case WM_UNICHAR:
        {
            u16 character = (u16)wParam;
            
            if (character == '\r'){
                character = '\n';
            }
            else if (character == '\t'){
                character = '\t';
            }
            else if (character < 32 || character == 127){
                break;
            }
            
            u16 character_no_caps_lock = character;
            
            i32 *count = &win32vars.input_chunk.trans.key_data.count;
            Key_Event_Data *data = win32vars.input_chunk.trans.key_data.keys;
            b8 *control_keys = win32vars.input_chunk.pers.control_keys;
            i32 control_keys_size = sizeof(win32vars.input_chunk.pers.control_keys);
            
            BYTE state[256];
            GetKeyboardState(state);
            if (state[VK_CAPITAL]){
                if (character_no_caps_lock >= 'a' && character_no_caps_lock <= 'z'){
                    character_no_caps_lock -= (u8)('a' - 'A');
                }
                else if (character_no_caps_lock >= 'A' && character_no_caps_lock <= 'Z'){
                    character_no_caps_lock += (u8)('a' - 'A');
                }
            }
            
            Assert(*count < KEY_INPUT_BUFFER_SIZE);
            data[*count].character = character;
            data[*count].character_no_caps_lock = character_no_caps_lock;
            data[*count].keycode = character_no_caps_lock;
            memcpy(data[*count].modifiers, control_keys, control_keys_size);
            ++(*count);
            
            win32vars.got_useful_event = 1;
        }break;
        
        case WM_MOUSEMOVE:
        {
            i32 new_x = LOWORD(lParam);
            i32 new_y = HIWORD(lParam);
            
            if (new_x != win32vars.input_chunk.pers.mouse_x || new_y != win32vars.input_chunk.pers.mouse_y){
                win32vars.input_chunk.pers.mouse_x = new_x;
                win32vars.input_chunk.pers.mouse_y = new_y;
                
                win32vars.got_useful_event = 1;
            }
        }break;
        
        case WM_MOUSEWHEEL:
        {
            win32vars.got_useful_event = 1;
            Font_ID rotation = GET_WHEEL_DELTA_WPARAM(wParam);
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
            
            for (i32 i = 0; i < MDFR_INDEX_COUNT; ++i){
                win32vars.input_chunk.pers.control_keys[i] = 0;
            }
            win32vars.input_chunk.pers.controls = null_control_keys;
        }break;
        
        case WM_SIZE:
        {
            win32vars.got_useful_event = 1;
            i32 new_width = LOWORD(lParam);
            i32 new_height = HIWORD(lParam);
            
            Win32Resize(new_width, new_height);
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
        {
            win32vars.got_useful_event = 1;
        }break;
        
        case WM_CANCELMODE:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }
    
    return(result);
}

int CALL_CONVENTION
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    i32 argc = __argc;
    char **argv = __argv;
    
    memset(&win32vars, 0, sizeof(win32vars));
    
    win32vars.GlobalWindowPosition.length = sizeof(win32vars.GlobalWindowPosition);
    
    //
    // Threads and Coroutines
    //
    
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
    // Memory Initialization
    //
    
    LPVOID base;
#if FRED_INTERNAL
#if defined(BUILD_X64)
    base = (LPVOID)TB(1);
#elif defined(BUILD_X86)
    base = (LPVOID)MB(96);
#endif
#else
    base = (LPVOID)0;
#endif
    
    memory_vars.vars_memory_size = MB(2);
    memory_vars.vars_memory = VirtualAlloc(base, memory_vars.vars_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
#if FRED_INTERNAL
#if defined(BUILD_X64)
    base = (LPVOID)TB(2);
#elif defined(BUILD_X86)
    base = (LPVOID)MB(98);
#endif
#else
    base = (LPVOID)0;
#endif
    memory_vars.target_memory_size = MB(512);
    memory_vars.target_memory = VirtualAlloc(base, memory_vars.target_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    base = (LPVOID)0;
    memory_vars.user_memory_size = MB(2);
    memory_vars.user_memory = VirtualAlloc(base, memory_vars.target_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    win32vars.target.max = MB(1);
    win32vars.target.push_buffer = (char*)system_memory_allocate(win32vars.target.max);
    
    if (memory_vars.vars_memory == 0 || memory_vars.target_memory == 0 || memory_vars.user_memory == 0 || win32vars.target.push_buffer == 0){
        exit(1);
    }
    
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
    
    DWORD cd_required = (GetCurrentDirectory(0, 0)*4) + 1;
    u16 *current_directory_16 = (u16*)system_memory_allocate(cd_required*4);
    DWORD written_16 = GetCurrentDirectory(cd_required, (LPWSTR)current_directory_16);
    
    u8 *current_directory_mem = (u8*)(current_directory_16 + cd_required);
    
    b32 convert_error = false;
    u32 current_directory_len = (u32)utf16_to_utf8_minimal_checking(current_directory_mem, cd_required*2, current_directory_16, written_16, &convert_error);
    
    String current_directory = make_string_cap(current_directory_mem, current_directory_len, cd_required*2);
    terminate_with_null(&current_directory);
    replace_char(&current_directory, '\\', '/');
    
    Command_Line_Parameters clparams;
    clparams.argv = argv;
    clparams.argc = argc;
    
    char **files = 0;
    i32 *file_count = 0;
    
    win32vars.app.read_command_line(&win32vars.system, &memory_vars, current_directory, &win32vars.settings, &files, &file_count, clparams);
    
    sysshared_filter_real_files(files, file_count);
    
    //
    // Custom Layer Linkage
    //
    
#if defined(FRED_SUPER)
    char *custom_file_default = "custom_4coder.dll";
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
            MessageBox(0, L"Error: The application and custom version numbers don't match.\n", L"Error", 0);
            exit(1);
        }
        win32vars.custom_api.get_bindings = (Get_Binding_Data_Function*)
            GetProcAddress(win32vars.custom, "get_bindings");
    }
    
    if (win32vars.custom_api.get_bindings == 0){
        MessageBox(0, L"Error: The custom dll is missing.\n", L"Error", 0);
        exit(1);
    }
    
#else
    win32vars.custom_api.get_bindings = (Get_Binding_Data_Function*)get_bindings;
#endif
    
    //
    // Window and GL Initialization
    //
    
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = (WNDPROC)(Win32Callback);
    window_class.hInstance = hInstance;
    window_class.lpszClassName = L"4coder-win32-wndclass";
    window_class.hIcon = LoadIcon(hInstance, L"main");
    
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
    
#define WINDOW_NAME L"4coder-window: " L_VERSION
    
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
    
    win32vars.window_handle = CreateWindow(window_class.lpszClassName, WINDOW_NAME, window_style, window_x, window_y, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, 0, 0, hInstance, 0);
    
    if (win32vars.window_handle == 0){
        exit(1);
    }
    
    {
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
        ReleaseDC(win32vars.window_handle, hdc);
    }
    
    Win32InitGL();
    Win32Resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    //
    // Font System Init
    //
    
    system_font_init(&win32vars.system.font, 0, 0, 16, true);
    
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
    
    win32vars.app.init(&win32vars.system, &win32vars.target, &memory_vars, win32vars.clipboard_contents, current_directory, win32vars.custom_api);
    
    system_memory_free(current_directory_16, 0);
    
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
        
        // NOTE(allen): When we're in stream mode we don't have
        // double buffering so we need to move ahead and call
        // the first step right away so it will render into the
        // window. With double buffering this is not an issue
        // for reasons I cannot at all comprehend.
        if (!(win32vars.first && win32vars.settings.stream_mode)){
            system_release_lock(FRAME_LOCK);
            
            if (win32vars.running_cli == 0){
                win32vars.got_useful_event = false;
            }
            
            b32 get_more_messages = true;
            do{
                if (win32vars.got_useful_event == 0){
                    get_more_messages = GetMessage(&msg, 0, 0, 0);
                }
                else{
                    get_more_messages = PeekMessage(&msg, 0, 0, 0, 1);
                }
                
                if (get_more_messages){
                    if (msg.message == WM_QUIT){
                        keep_playing = 0;
                    }else{
                        b32 treat_normally = true;
                        if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN){
                            switch (msg.wParam){
                                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                                case VK_MENU:case VK_LMENU:case VK_RMENU:
                                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:break;
                                
                                default: treat_normally = false; break;
                            }
                        }
                        
                        if (treat_normally){
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                        else{
                            Control_Keys *controls = &win32vars.input_chunk.pers.controls;
                            
                            b8 ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
                            b8 alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
                            
                            if (win32vars.lctrl_lalt_is_altgr){
                                if (controls->l_alt && controls->l_ctrl){
                                    ctrl = 0;
                                    alt = 0;
                                }
                            }
                            
                            BYTE ctrl_state = 0, alt_state = 0;
                            BYTE state[256];
                            if (ctrl || alt){
                                GetKeyboardState(state);
                                if (ctrl){
                                    ctrl_state = state[VK_CONTROL];
                                    state[VK_CONTROL] = 0;
                                }
                                if (alt){
                                    alt_state = state[VK_MENU];
                                    state[VK_MENU] = 0;
                                }
                                SetKeyboardState(state);
                                
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                                
                                if (ctrl){
                                    state[VK_CONTROL] = ctrl_state;
                                }
                                if (alt){
                                    state[VK_MENU] = alt_state;
                                }
                                SetKeyboardState(state);
                            }
                            else{
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                    }
                }
            }while(get_more_messages);
            
            system_acquire_lock(FRAME_LOCK);
        }
        
        POINT mouse_point;
        if (GetCursorPos(&mouse_point) &&
            ScreenToClient(win32vars.window_handle, &mouse_point)){
            
            i32_Rect screen;
            screen.x0 = 0;
            screen.y0 = 0;
            screen.x1 = win32vars.target.width;
            screen.y1 = win32vars.target.height;
            
            i32 mx = mouse_point.x;
            i32 my = mouse_point.y;
            
            b32 is_hit = false;
            if (mx >= screen.x0 && mx < screen.x1 && my >= screen.y0 && my < screen.y1){
                is_hit = true;
            }
            
            if (!is_hit){
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
        
        win32vars.clipboard_contents = null_string;
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
        
        win32vars.app.step(&win32vars.system, &win32vars.target, &memory_vars, &input, &result, clparams);
        
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

#include "4ed_font_static_functions.cpp"

#include "win32_4ed_fonts.cpp"
#include "win32_4ed_file_track.cpp"

#if 0
// NOTE(allen): In case I want to switch back to a console
// application at some point.
int main(int argc, char **argv){
    HINSTANCE hInstance = GetModuleHandle(0);
}
#endif

// BOTTOM

