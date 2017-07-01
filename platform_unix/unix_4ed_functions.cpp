/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 30.06.2017
 *
 * General unix functions
 *
 */

// TOP

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <alloca.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <xmmintrin.h>

#if defined(USE_LOG)
# include <stdio.h>
#endif

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

struct Unix_Vars{
    b32 do_logging;
    
    Thread_Memory *thread_memory;
    Thread_Group groups[THREAD_GROUP_COUNT];
    Work_Queue queues[THREAD_GROUP_COUNT];
    pthread_mutex_t locks[LOCK_COUNT];
    pthread_cond_t conds[8];
    sem_t thread_semaphore;
};

static Unix_Vars unixvars;

//
// Intrinsics
//

#define InterlockedCompareExchange(dest, ex, comp) \
__sync_val_compare_and_swap((dest), (comp), (ex))

//
// 4ed Path
//

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    ssize_t size = readlink("/proc/self/exe", out, capacity - 1);
    if (size != -1 && size < capacity - 1){
        String str = make_string(out, size);
        remove_last_folder(&str);
        terminate_with_null(&str);
        size = str.size;
    }
    else{
        size = 0;
    }
    return(size);
}

//
// Logging
//

internal
Sys_Log_Sig(system_log){
    if (unixvars.do_logging){
        i32 fd = open("4coder_log.txt", O_WRONLY | O_CREAT, 00640);
        if (fd >= 0){
            do{
                ssize_t written = write(fd, message, length);
                if (written != -1){
                    length -= written;
                    message += written;
                }
            } while(length > 0);
            close(fd);
        }
    }
}

//
// Shared system functions (system_shared.h)
//

internal
Sys_File_Can_Be_Made_Sig(system_file_can_be_made){
    b32 result = access((char*)filename, W_OK) == 0;
    LOGF("%s = %d", filename, result);
    return(result);
}

//
// Memory
//

internal
Sys_Memory_Allocate_Sig(system_memory_allocate){
    // NOTE(allen): This must return the exact base of the vpage.
    // We will count on the user to keep track of size themselves.
    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(result == MAP_FAILED){
        LOG("mmap failed\n");
        result = NULL;
    }
    return(result);
}

internal 
Sys_Memory_Set_Protection_Sig(system_memory_set_protection){
    bool32 result = true;
    
    int protect = 0;
    switch (flags & 0x7){
        case 0: protect = PROT_NONE; break;
        
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
        LOG("mprotect");
    }
    
    return(result);
}

internal
Sys_Memory_Free_Sig(system_memory_free){
    // NOTE(allen): This must take the exact base of the vpage.
    munmap(ptr, size);
}

//
// Files
//

internal
Sys_Set_File_List_Sig(system_set_file_list){
    if (directory == 0){
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
        return;
    }
    
    LOGF("%s", directory);
    
    DIR *d = opendir(directory);
    if (d != 0){
        if (canon_directory_out != 0){
            u32 length = copy_fast_unsafe_cc(canon_directory_out, directory);
            if (canon_directory_out[length-1] != '/'){
                canon_directory_out[length++] = '/';
            }
            canon_directory_out[length] = 0;
            *canon_directory_size_out = length;
        }
        
        i32 character_count = 0;
        i32 file_count = 0;
        for (struct dirent *entry = readdir(d);
             entry != 0;
             entry = readdir(d)){
            char *fname = entry->d_name;
            if (match_cc(fname, ".") || match_cc(fname, "..")){
                continue;
            }
            ++file_count;
            i32 size = 0;
            for (; fname[size]; ++size);
            character_count += size + 1;
        }
        
        i32 required_size = character_count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_memory_free(file_list->block, file_list->block_size);
            file_list->block = system_memory_allocate(required_size);
            file_list->block_size = required_size;
        }
        
        file_list->infos = (File_Info*)file_list->block;
        char *cursor = (char*)(file_list->infos + file_count);
        
        if (file_list->block != 0){
            rewinddir(d);
            File_Info *info_ptr = file_list->infos;
            for (struct dirent *entry = readdir(d);
                 entry != 0;
                 entry = readdir(d)){
                char *fname = entry->d_name;
                if (match_cc(fname, ".") || match_cc(fname, "..")){
                    continue;
                }
                char *cursor_start = cursor;
                i32 length = copy_fast_unsafe_cc(cursor_start, fname);
                cursor += length;
                
                if(entry->d_type == DT_LNK){
                    struct stat st;
                    if(stat(entry->d_name, &st) != -1){
                        info_ptr->folder = S_ISDIR(st.st_mode);
                    }
                    else{
                        info_ptr->folder = 0;
                    }
                }
                else{
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
    }
    else{
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
    }
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    char* path = (char*)alloca(len + 1);
    char* write_p = path;
    const char* read_p = filename;
    
    // return 0 for relative paths (e.g. cmdline args)
    if (len > 0 && filename[0] != '/'){
        return 0;
    }
    
    if (max == 0){
        return 0;
    }
    
    max -= 1;
    
    while (read_p < filename + len){
        if (read_p == filename || read_p[0] == '/'){
            if (read_p[1] == '/'){
                ++read_p;
            }
            else if (read_p[1] == '.'){
                if (read_p[2] == '/' || !read_p[2]){
                    read_p += 2;
                }
                else if (read_p[2] == '.' && (read_p[3] == '/' || !read_p[3])){
                    while(write_p > path && *--write_p != '/');
                    read_p += 3;
                }
                else{
                    *write_p++ = *read_p++;
                }
            }
            else{
                *write_p++ = *read_p++;
            }
        }
        else{
            *write_p++ = *read_p++;
        }
    }
    if (write_p == path) *write_p++ = '/';
    
    if (max >= (write_p - path)){
        memcpy(buffer, path, write_p - path);
    } 
    else{
        write_p = path;
    }
    
#if FRED_INTERNAL
    if (len != (write_p - path) || memcmp(filename, path, len) != 0){
        LOGF("[%.*s] -> [%.*s]", len, filename, (int)(write_p - path), path);
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
        LOG("open");
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
        LOG("fstat");
    }
    else{
        result = st.st_size;
    }
    
    return result;
}

internal
Sys_Load_File_Sig(system_load_file){
    int fd = *(int*)&handle;
    
    do{
        ssize_t n = read(fd, buffer, size);
        if(n == -1){
            if(errno != EINTR){
                LOG("read");
                break;
            }
        }
        else{
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
        LOG("close");
        result = 0;
    }
    
    return result;
}

internal
Sys_Save_File_Sig(system_save_file){
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 00640);
    LOGF("%s %d", filename, size);
    if(fd < 0){
        LOGF("system_save_file: open '%s': %s\n", filename, strerror(errno));
    }
    else{
        do {
            ssize_t written = write(fd, buffer, size);
            if(written == -1){
                if(errno != EINTR){
                    LOG("system_save_file: write");
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
// File System
//

internal
Sys_File_Exists_Sig(system_file_exists){
    int result = 0;
    char buff[PATH_MAX] = {};
    
    if (len + 1 > PATH_MAX){
        LOG("system_directory_has_file: path too long\n");
    }
    else{
        memcpy(buff, filename, len);
        buff[len] = 0;
        struct stat st;
        result = stat(buff, &st) == 0 && S_ISREG(st.st_mode);
    }
    
    LOGF("%s: %d", buff, result);
    
    return(result);
}

internal
Sys_Directory_CD_Sig(system_directory_cd){
    String directory = make_string_cap(dir, *len, cap);
    b32 result = false;
    
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
                i32 old_size = directory.size;
                append_partial_sc(&directory, rel_path);
                append_s_char(&directory, '/');
                terminate_with_null(&directory);
                
                struct stat st;
                if (stat(directory.str, &st) == 0 && S_ISDIR(st.st_mode)){
                    result = true;
                }
                else{
                    directory.size = old_size;
                }
            }
        }
    }
    
    *len = directory.size;
    LOGF("%.*s: %d", directory.size, directory.str, result);
    
    return(result);
}

//
// Time
//

internal
Sys_Now_Time_Sig(system_now_time){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    u64 result = (spec.tv_sec * UINT64_C(1000000)) + (spec.tv_nsec / UINT64_C(1000));
    return(result);
}

//
// Threads
//

internal
Sys_Acquire_Lock_Sig(system_acquire_lock){
    pthread_mutex_lock(unixvars.locks + id);
}

internal
Sys_Release_Lock_Sig(system_release_lock){
    pthread_mutex_unlock(unixvars.locks + id);
}

internal void
system_wait_cv(i32 lock_id, i32 cv_id){
    pthread_cond_wait(unixvars.conds + cv_id, unixvars.locks + lock_id);
}

internal void
system_signal_cv(i32 lock_id, i32 cv_id){
    pthread_cond_signal(unixvars.conds + cv_id);
}

internal void*
JobThreadProc(void* lpParameter){
    Thread_Context *thread = (Thread_Context*)lpParameter;
    Work_Queue *queue = unixvars.queues + thread->group_id;
    Thread_Group *group = unixvars.groups + thread->group_id;
    
    i32 thread_index = thread->id - 1;
    
    i32 cancel_lock = group->cancel_lock0 + thread_index;
    i32 cancel_cv = group->cancel_cv0 + thread_index;
    
    Thread_Memory *thread_memory = unixvars.thread_memory + thread_index;
    
    if (thread_memory->size == 0){
        i32 new_size = KB(64);
        thread_memory->data = system_memory_allocate(new_size);
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

// BOTTOM

