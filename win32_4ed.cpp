/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for project codename "4ed"
 *
 */

// TOP

#include "4coder_default_bindings.cpp"

#include "4ed_meta.h"

#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4ed_mem.cpp"
#include "4ed_math.cpp"

#include "4ed_system.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include <windows.h>
#include <GL/gl.h>

#include "system_shared.h"

#define FPS 60
#define frame_useconds (1000000 / FPS)

#define WM_4coder_ANIMATE (WM_USER + 1)

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
    
    i32 cancel_lock0;
    i32 cancel_cv0;
};

#define UseWinDll 1

#if UseWinDll == 0
#include "4ed_dll_reader.h"
#include "4ed_dll_reader.cpp"
#endif

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
inline Win32_Input_Chunk_Transient
win32_input_chunk_transient_zero(){
    Win32_Input_Chunk_Transient result = {0};
    return(result);
}

struct Win32_Input_Chunk_Persistent{
    i32 mouse_x, mouse_y;
    b8 mouse_l, mouse_r;
    
    Control_Keys controls;
    b8 control_keys[MDFR_INDEX_COUNT];
};

struct Win32_Input_Chunk{
    Win32_Input_Chunk_Transient trans;
    Win32_Input_Chunk_Persistent pers;
};

struct Win32_Coroutine{
    Coroutine coroutine;
    Win32_Coroutine *next;
    int done;
};

#if FRED_INTERNAL
struct Sys_Bubble : public Bubble{
    i32 line_number;
    char *file_name;
};
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

struct Win32_Vars{
    System_Functions system;
    App_Functions app;
    Custom_API custom_api;
#if UseWinDll
    HMODULE app_code;
    HMODULE custom;
#else
    DLL_Loaded app_dll;
    DLL_Loaded custom_dll;
#endif
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
    
    
    u64 count_per_usecond;
    b32 first;
    i32 running_cli;
    
    
#if FRED_INTERNAL
    CRITICAL_SECTION DEBUG_sysmem_lock;
    Sys_Bubble internal_bubble;
#endif
};

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
        bubble->flags = MEM_BUBBLE_SYS_DEBUG;
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
        Assert((bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_SYS_DEBUG);
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
    
    for (;;){
        u32 read_index = queue->read_position;
        u32 write_index = queue->write_position;
        
        if (read_index != write_index){
            u32 next_read_index = (read_index + 1) % JOB_ID_WRAP;
            u32 safe_read_index =
                InterlockedCompareExchange(&queue->read_position,
                                           next_read_index, read_index);
            
            if (safe_read_index == read_index){
                Full_Job_Data *full_job = queue->jobs + (safe_read_index % QUEUE_WRAP);
                // NOTE(allen): This is interlocked so that it plays nice
                // with the cancel job routine, which may try to cancel this job
                // at the same time that we try to run it
                
                i32 safe_running_thread =
                    InterlockedCompareExchange(&full_job->running_thread,
                                               thread->id, THREAD_NOT_ASSIGNED);
                
                if (safe_running_thread == THREAD_NOT_ASSIGNED){
                    thread->job_id = full_job->id;
                    thread->running = 1;
                    Thread_Memory *thread_memory = 0;
                    
                    // TODO(allen): remove memory_request
                    if (full_job->job.memory_request != 0){
                        thread_memory = win32vars.thread_memory + thread->id - 1;
                        if (thread_memory->size < full_job->job.memory_request){
                            if (thread_memory->data){
                                Win32FreeMemory(thread_memory->data);
                            }
                            i32 new_size = LargeRoundUp(full_job->job.memory_request, Kbytes(4));
                            thread_memory->data = Win32GetMemory(new_size);
                            thread_memory->size = new_size;
                        }
                    }
                    full_job->job.callback(&win32vars.system,
                                           thread, thread_memory, full_job->job.data);
                    PostMessage(win32vars.window_handle, WM_4coder_ANIMATE, 0, 0);
                    full_job->running_thread = 0;
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

internal
Sys_Post_Job_Sig(system_post_job){
    Work_Queue *queue = win32vars.queues + group_id;
    
    Assert((queue->write_position + 1) % QUEUE_WRAP != queue->read_position % QUEUE_WRAP);
    
    b32 success = 0;
    u32 result = 0;
    while (!success){
        u32 write_index = queue->write_position;
        u32 next_write_index = (write_index + 1) % JOB_ID_WRAP;
        u32 safe_write_index =
            InterlockedCompareExchange(&queue->write_position,
                                       next_write_index, write_index);
        if (safe_write_index  == write_index){
            result = write_index;
            write_index = write_index % QUEUE_WRAP;
            queue->jobs[write_index].job = job;
            queue->jobs[write_index].running_thread = THREAD_NOT_ASSIGNED;
            queue->jobs[write_index].id = result;
            success = 1;
        }
    }
    
    ReleaseSemaphore(Win32Handle(queue->semaphore), 1, 0);
    
    return result;
}

// NOTE(allen): New job cancelling system:
//
//  Jobs are expected to periodically check their cancelation
// state, especially if they are taking a while.
//
//  When the main thread asks to cancel a job it sets the cancel
// state and does not resume until the thread running the job
// signals that it is okay.
//
//  Don't hold the frame lock while sleeping, as this can dead-lock
// the job thread and the main thread, and since main is sleeping
// they won't collide anyway.
internal
Sys_Cancel_Job_Sig(system_cancel_job){
    Work_Queue *queue = win32vars.queues + group_id;
    Thread_Group *group = win32vars.groups + group_id;
    
    u32 job_index;
    u32 thread_id;
    Full_Job_Data *full_job;
    
    job_index = job_id % QUEUE_WRAP;
    full_job = queue->jobs + job_index;
    
    Assert(full_job->id == job_id);
    thread_id =
        InterlockedCompareExchange(&full_job->running_thread,
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
    Work_Queue *queue = win32vars.queues + id;
    u32 write = queue->write_position;
    u32 read = queue->read_position;
    if (write < read) write += JOB_ID_WRAP;
    *pending = (i32)(write - read);
    
    Thread_Group *group = win32vars.groups + id;
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
Sys_File_Load_Begin_Sig(system_file_load_begin){
    File_Loading loading = {0};
    HANDLE file = 0;
    
    String fname_str = make_string_slowly(filename);
    if (fname_str.size < 1024){
        char fixed_space[1024];
        String fixed_str = make_fixed_width_string(fixed_space);
        copy(&fixed_str, fname_str);
        terminate_with_null(&fixed_str);
        
        replace_char(fixed_str, '/', '\\');
        
        file = CreateFile(fixed_str.str, GENERIC_READ, 0, 0,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file && file != INVALID_HANDLE_VALUE){
            DWORD lo, hi;
            lo = GetFileSize(file, &hi);
            
            if (hi == 0){
                loading.handle = Win32Handle(file);
                loading.size = lo;
                loading.exists = 1;
            }
            else{
                CloseHandle(file);
            }
        }
    }
    
    return(loading);
}

internal
Sys_File_Load_End_Sig(system_file_load_end){
    b32 success = 0;
    HANDLE file = Win32Handle(loading.handle);
    
    DWORD read_size = 0;
    BOOL read_result = 0;
    
    if (loading.exists && file != INVALID_HANDLE_VALUE){
        read_result = 
            ReadFile(file,
                     buffer, loading.size,
                     &read_size, 0);
        
        if (read_result && read_size == (DWORD)loading.size){
            success = 1;
        }
        
        CloseHandle(file);
    }
    
    return(success);
}

internal
Sys_File_Save_Sig(system_file_save){
    b32 success = 0;
    
    HANDLE file =
        CreateFile((char*)filename, GENERIC_WRITE, 0, 0,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
	if (!file || file == INVALID_HANDLE_VALUE){
        success = 0;
	}
    else{
        BOOL write_result = 0;
        DWORD bytes_written = 0;
        
        if (buffer){
            write_result = WriteFile(file, buffer, size, &bytes_written, 0);
        }
        
        CloseHandle(file);
        
        if (!write_result || bytes_written != (u32)size){
            success = 0;
        }
    }
    
    return(success);
}

internal
Sys_File_Time_Stamp_Sig(system_file_time_stamp){
    u64 result = 0;
    
    FILETIME last_write;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx((char*)filename, GetFileExInfoStandard, &data)){
        last_write = data.ftLastWriteTime;
        
        result = ((u64)last_write.dwHighDateTime << 32) | (last_write.dwLowDateTime);
    }
    
    return(result);
}

internal
Sys_Now_Time_Stamp_Sig(system_now_time_stamp){
	u64 result = 0;
    FILETIME filetime;
    GetSystemTimeAsFileTime(&filetime);
    result = ((u64)filetime.dwHighDateTime << 32) | (filetime.dwLowDateTime);
    return(result);
}

internal
Sys_Set_File_List_Sig(system_set_file_list){
    if (directory.size > 0){
        char dir_space[MAX_PATH + 32];
        String dir = make_string(dir_space, 0, MAX_PATH + 32);
        append(&dir, directory);
        char trail_str[] = "\\*";
        append(&dir, trail_str);
        
        char *c_str_dir = make_c_str(dir);
        
        WIN32_FIND_DATA find_data;
        HANDLE search;
        search = FindFirstFileA(c_str_dir, &find_data);
        
        if (search != INVALID_HANDLE_VALUE){            
            i32 count = 0;
            i32 file_count = 0;
            BOOL more_files = 1;
            do{
                if (!match(find_data.cFileName, ".") &&
                    !match(find_data.cFileName, "..")){
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
                        if (!match(find_data.cFileName, ".") &&
                            !match(find_data.cFileName, "..")){
                            info->folder = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                            info->filename.str = name;
                            
                            i32 i = 0;
                            for(;find_data.cFileName[i];++i) *name++ = find_data.cFileName[i];
                            info->filename.size = i;
                            info->filename.memory_size = info->filename.size + 1;
                            *name++ = 0;
                            replace_char(info->filename, '\\', '/');
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

internal
Sys_File_Track_Sig(system_file_track){
}

internal
Sys_File_Untrack_Sig(system_file_untrack){
}

internal
Sys_File_Unique_Hash_Sig(system_file_unique_hash){
    Unique_Hash hash = {0};
    BY_HANDLE_FILE_INFORMATION info;
    HANDLE handle;
    char space[1024];
    String str;

    if (filename.size < sizeof(space)){
        str = make_fixed_width_string(space);
        copy(&str, filename);
        terminate_with_null(&str);
        
        handle = CreateFile(str.str, GENERIC_READ, 0, 0,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        *success = 0;
        if (handle && handle != INVALID_HANDLE_VALUE){
            if (GetFileInformationByHandle(handle, &info)){
                hash.d[2] = info.dwVolumeSerialNumber;
                hash.d[1] = info.nFileIndexHigh;
                hash.d[0] = info.nFileIndexLow;
                *success = 1;
            }

            CloseHandle(handle);
        }
    }
    
    return(hash);
}

// NOTE(allen): Exposed to the custom layer.
internal
FILE_EXISTS_SIG(system_file_exists){
    char full_filename_space[1024];
    String full_filename;
    HANDLE file;
    b32 result;

    result = 0;
    
    if (len < sizeof(full_filename_space)){
        full_filename = make_fixed_width_string(full_filename_space);
        copy(&full_filename, make_string(filename, len));
        terminate_with_null(&full_filename);
        
        file = CreateFile(full_filename.str, GENERIC_READ, 0, 0,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != INVALID_HANDLE_VALUE){
            CloseHandle(file);
            result = 1;
        }
    }
    
    return(result);
}

b32 Win32DirectoryExists(char *path){
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// NOTE(allen): Exposed to the custom layer.
internal
DIRECTORY_CD_SIG(system_directory_cd){
    String directory = make_string(dir, *len, capacity);
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
                append_partial(&directory, rel_path);
                append_partial(&directory, "\\");
                if (Win32DirectoryExists(directory.str)){
                    result = 1;
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

// NOTE(allen): Exposed to the custom layer.
GET_4ED_PATH_SIG(system_get_4ed_path){
    String str = make_string(out, 0, capacity);
    return(system_get_binary_path(&str));
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
            copy_fast_unsafe(dest, str);
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
    
    b32 success = 1;
    String s = make_fixed_width_string(command_line);
    copy(&s, make_lit_string("/C "));
    append_partial(&s, script_name);
    success = terminate_with_null(&s);
    
    if (success){
        success = 0;
        
        SECURITY_ATTRIBUTES sec_attributes;
        HANDLE out_read;
        HANDLE out_write;
        
        sec_attributes = {};
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

    return has_more;
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
    return close_me;
}


#include "system_shared.cpp"
#include "4ed_rendering.cpp"

internal
Font_Load_Sig(system_draw_font_load){
    if (win32vars.font_part.base == 0){
        win32vars.font_part = Win32ScratchPartition(Mbytes(8));
    }
    
    i32 oversample = 2;
    
    for (b32 success = 0; success == 0;){
        success = draw_font_load(&win32vars.font_part,
                                 font_out,
                                 filename,
                                 pt_size,
                                 tab_width,
                                 oversample);
        
        // TODO(allen): Make the growable partition something that can
        // just be passed directly to font load and let it be grown there.
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

#if UseWinDll
    win32vars.app_code = LoadLibraryA("4ed_app.dll");
    if (win32vars.app_code){
        get_funcs = (App_Get_Functions*)
            GetProcAddress(win32vars.app_code, "app_get_functions");
    }

#else
    File_Data file = system_load_file("4ed_app.dll");

    if (file.got_file){
        i32 error;
        DLL_Data dll_data;
        if (dll_parse_headers(file.data, &dll_data, &error)){
            Data img;
            img.size = dll_total_loaded_size(&dll_data);
            img.data = (byte*)
                VirtualAlloc((LPVOID)Tbytes(3), img.size,
                MEM_COMMIT | MEM_RESERVE,
                PAGE_READWRITE);

            dll_load(img, &win32vars.app_dll, file.data, &dll_data);

            DWORD extra_;
            VirtualProtect(img.data + win32vars.app_dll.text_start,
                win32vars.app_dll.text_size,
                PAGE_EXECUTE_READ,
                &extra_);

            get_funcs = (App_Get_Functions*)
                dll_load_function(&win32vars.app_dll, "app_get_functions", 17);
        }
        else{
            // TODO(allen): file loading error
        }

        Win32FreeMemory(file.data.data);
    }
    else{
        // TODO(allen): file loading error
    }

#endif

    if (get_funcs){
        result = 1;
        win32vars.app = get_funcs();        
    }

    return result;
}

internal void
Win32LoadSystemCode(){
    win32vars.system.file_time_stamp = system_file_time_stamp;
    win32vars.system.now_time_stamp = system_now_time_stamp;
    win32vars.system.file_unique_hash = system_file_unique_hash;
    win32vars.system.set_file_list = system_set_file_list;
    win32vars.system.file_track = system_file_track;
    win32vars.system.file_untrack = system_file_untrack;
    win32vars.system.file_load_begin = system_file_load_begin;
    win32vars.system.file_load_end = system_file_load_end;
    win32vars.system.file_save = system_file_save;

    win32vars.system.file_exists = system_file_exists;
    win32vars.system.directory_cd = system_directory_cd;
    win32vars.system.get_4ed_path = system_get_4ed_path;

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

    win32vars.target.font_set.font_info_load = draw_font_info_load;
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
    LRESULT result = {};
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
                b8 previous_state, current_state;
                previous_state = ((lParam & Bit_30)?(1):(0));
                current_state = ((lParam & Bit_31)?(0):(1));

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
                            int result1 = 0, result2 = 0, result = 0;
                            
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
                            
                            // TODO(allen): This is becoming a really major issue.  Apparently
                            // control + i outputs a '\t' which is VALID ascii according to this system.
                            // So it reports the key as '\t'.  This wasn't an issue before because we were
                            // ignoring control when computing character_no_caps_lock which is what
                            // is used for commands.  But that is incorrect for some keyboard layouts where
                            // control+alt is used to signal AltGr for important keys.
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
                        ++(*count);
                    }
                }
                
                result = DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
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

            b8 *control_keys = win32vars.input_chunk.pers.control_keys;
            for (int i = 0; i < MDFR_INDEX_COUNT; ++i) control_keys[i] = 0;
            win32vars.input_chunk.pers.controls = control_keys_zero();
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
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }
    return(result);
}

#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEBUG_OUTPUT 0x92E0

typedef void GLDEBUGPROC_TYPE(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, char * message, GLvoid * userParam);
typedef GLDEBUGPROC_TYPE * GLDEBUGPROC;
typedef void glDebugMessageControl_type(GLenum source, GLenum type, GLenum severity, GLsizei count, GLuint * ids, GLboolean enabled);
typedef void glDebugMessageCallback_type(GLDEBUGPROC callback, void * userParam);

internal void
OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, char *message, void *userParam)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

int
WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow){
    
    int argc = __argc;
    char **argv = __argv;
    
    memset(&win32vars, 0, sizeof(win32vars));
    
    
    //
    // Threads and Coroutines
    //
    
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
    
    Assert(win32vars.locks);
    for (i32 i = 0; i < LOCK_COUNT; ++i){
        InitializeCriticalSection(&win32vars.locks[i]);
    }
    InitializeCriticalSection(&win32vars.DEBUG_sysmem_lock);
        
    ConvertThreadToFiber(0);
    win32vars.coroutine_free = win32vars.coroutine_data;
    for (i32 i = 0; i+1 < ArrayCount(win32vars.coroutine_data); ++i){
        win32vars.coroutine_data[i].next = win32vars.coroutine_data + i + 1;
    }
    
    
    //
    // Memory Initialization
    //
    
#if FRED_INTERNAL
    win32vars.internal_bubble.next = &win32vars.internal_bubble;
    win32vars.internal_bubble.prev = &win32vars.internal_bubble;
    win32vars.internal_bubble.flags = MEM_BUBBLE_SYS_DEBUG;
#endif
    
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
    win32vars.target.push_buffer = (byte*)system_get_memory(win32vars.target.max);
    
    
    //
    // System and Application Layer Linkage
    //
    
    if (!Win32LoadAppCode()){
        exit(1);
    }
    
    Win32LoadSystemCode();
    
    Win32LoadRenderCode();
    
    
    //
    // Read Command Line
    //
    
    DWORD required = (GetCurrentDirectory(0, 0)*4) + 1;
    char *current_directory_mem = (char*)system_get_memory(required);
    DWORD written = GetCurrentDirectory(required, current_directory_mem);
    
    String current_directory = make_string(current_directory_mem, written, required);
    terminate_with_null(&current_directory);
    replace_char(current_directory, '\\', '/');
    
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
    
#ifdef FRED_SUPER
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
            OutputDebugStringA("Error: application and custom version numbers don't match\n");
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
#endif
    
    if (win32vars.custom_api.get_bindings == 0){
        win32vars.custom_api.get_bindings = (Get_Binding_Data_Function*)get_bindings;
    }
    
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
    
    i32 window_x;
    i32 window_y;
    i32 window_style;
    
    if (win32vars.settings.set_window_pos){
        window_x = win32vars.settings.window_x;
        window_y = win32vars.settings.window_y;
    }
    else{
        window_x = CW_USEDEFAULT;
        window_y = CW_USEDEFAULT;
    }
    
    window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    if (win32vars.settings.maximize_window){
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
    
    
    // TODO(allen): not Windows XP compatible, do we care?
    // SetProcessDPIAware();
    
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
    
    HDC hdc = GetDC(win32vars.window_handle);
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
    // NOTE(casey): This slows down GL but puts error messages to the debug console immediately whenever you do something wrong
    glDebugMessageCallback_type *glDebugMessageCallback = (glDebugMessageCallback_type *)wglGetProcAddress("glDebugMessageCallback");
    glDebugMessageControl_type *glDebugMessageControl = (glDebugMessageControl_type *)wglGetProcAddress("glDebugMessageControl");
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
    
    
    SetForegroundWindow(win32vars.window_handle);
    SetActiveWindow(win32vars.window_handle);
    
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
        win32vars.input_chunk.trans = win32_input_chunk_transient_zero();
        
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
        
        win32vars.app.step(&win32vars.system,
                           &win32vars.target,
                           &memory_vars,
                           &input,
                           &result);
        
        if (result.perform_kill){
            keep_playing = 0;
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
}
#endif

// BOTTOM


