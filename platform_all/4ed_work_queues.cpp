/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Cross platform logic for work queues.
 *
 */

// TOP

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

struct Thread_Context{
    u32 job_id;
    b32 running;
    b32 cancel;
    
    Work_Queue *queue;
    u32 id;
    u32 group_id;
    u32 windows_id;
    Thread thread;
};

struct Thread_Group{
    Thread_Context *threads;
    i32 count;
    
    Unbounded_Work_Queue queue;
    
    i32 cancel_lock0;
    i32 cancel_cv0;
};

struct Threading_Vars{
    Thread_Memory *thread_memory;
    Work_Queue queues[THREAD_GROUP_COUNT];
    Thread_Group groups[THREAD_GROUP_COUNT];
    Mutex locks[LOCK_COUNT];
    Condition_Variable conds[CV_COUNT];
};
global Threading_Vars threadvars;

internal
Sys_Acquire_Lock_Sig(system_acquire_lock){
    system_acquire_lock(&threadvars.locks[id]);
}

internal
Sys_Release_Lock_Sig(system_release_lock){
    system_release_lock(&threadvars.locks[id]);
}

internal
PLAT_THREAD_SIG(job_thread_proc){
    Thread_Context *thread = (Thread_Context*)ptr;
    
    Work_Queue *queue = threadvars.queues + thread->group_id;
    Thread_Group *group = threadvars.groups + thread->group_id;
    
    i32 thread_index = thread->id - 1;
    Thread_Memory *thread_memory = threadvars.thread_memory + thread_index;
    
    i32 cancel_lock_id = group->cancel_lock0 + thread_index;
    i32 cancel_cv_id = group->cancel_cv0 + thread_index;
    Mutex *cancel_lock = &threadvars.locks[cancel_lock_id];
    Condition_Variable *cancel_cv = &threadvars.conds[cancel_cv_id];
    
    if (thread_memory->size == 0){
        i32 new_size = KB(64);
        thread_memory->data = system_memory_allocate(new_size);
        thread_memory->size = new_size;
    }
    
    for (;;){
        u32 read_index = queue->read_position;
        u32 write_index = queue->write_position;
        
        if (read_index != write_index){
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
                    thread->running = true;
                    full_job->job.callback(&sysfunc, thread, thread_memory, full_job->job.data);
                    system_schedule_step();
                    thread->running = false;
                    
                    system_acquire_lock(cancel_lock);
                    if (thread->cancel){
                        thread->cancel = 0;
                        system_signal_cv(cancel_cv, cancel_lock);
                    }
                    system_release_lock(cancel_lock);
                }
            }
        }
        else{
            system_wait_on(queue->semaphore);
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
    // is allowed is when the queue is empty.  Thus if (write_position+1 == read_position) 
    // the available space is zero. So these computations both end up leaving one slot unused.
    
    // TODO(allen): The only way I can think to easily eliminate this is to have read and write wrap 
    // at twice the size of the underlying array but modulo their values into the array then if write
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

internal i32
flush_thread_group(Thread_Group_ID group_id){
    Thread_Group *group = threadvars.groups + group_id;
    Work_Queue *queue = threadvars.queues + group_id;
    Unbounded_Work_Queue *source_queue = &group->queue;
    i32 thread_count = group->count;
    
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
    
    for (i32 i = 0; i < semaphore_release_count; ++i){
        system_release_semaphore(queue->semaphore);
    }
    
    return(semaphore_release_count);
}

// Note(allen): post_job puts the job on the unbounded queue.
// The unbounded queue is entirely managed by the main thread.
// The thread safe queue is bounded in size so the unbounded
// queue is periodically flushed into the direct work queue.
internal
Sys_Post_Job_Sig(system_post_job){
    Thread_Group *group = threadvars.groups + group_id;
    Unbounded_Work_Queue *queue = &group->queue;
    
    u32 result = queue->next_job_id++;
    
    if (queue->count >= queue->max){
        u32 new_max = round_up_pot_u32(queue->count + 1);
        Full_Job_Data *new_jobs = (Full_Job_Data*)system_memory_allocate(new_max*sizeof(Full_Job_Data));
        memcpy(new_jobs, queue->jobs, queue->count);
        system_memory_free(queue->jobs, queue->max*sizeof(Full_Job_Data));
        queue->jobs = new_jobs;
        queue->max = new_max;
    }
    
    Full_Job_Data full_job = {0};
    full_job.job = job;
    full_job.running_thread = THREAD_NOT_ASSIGNED;
    full_job.id = result;
    
    queue->jobs[queue->count++] = full_job;
    flush_thread_group(group_id);
    
    return(result);
}

internal
Sys_Cancel_Job_Sig(system_cancel_job){
    Thread_Group *group = threadvars.groups + group_id;
    Work_Queue *queue = threadvars.queues + group_id;
    
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
        Full_Job_Data *job = queue->jobs + (job_id % QUEUE_WRAP);
        Assert(job->id == job_id);
        
        u32 thread_id = InterlockedCompareExchange(&job->running_thread, 0, THREAD_NOT_ASSIGNED);
        
        if (thread_id != THREAD_NOT_ASSIGNED && thread_id != 0){
            i32 thread_index = thread_id - 1;
            
            i32 cancel_lock_id = group->cancel_lock0 + thread_index;
            i32 cancel_cv_id = group->cancel_cv0 + thread_index;
            Mutex *cancel_lock = &threadvars.locks[cancel_lock_id];
            Condition_Variable *cancel_cv = &threadvars.conds[cancel_cv_id];
            
            Thread_Context *thread = group->threads + thread_index;
            
            system_acquire_lock(cancel_lock);
            thread->cancel = true;
            system_release_lock(&threadvars.locks[FRAME_LOCK]);
            
            do{
                system_wait_cv(cancel_cv, cancel_lock);
            }while (thread->cancel);
            
            system_acquire_lock(&threadvars.locks[FRAME_LOCK]);
            system_release_lock(cancel_lock);
        }
    }
}

internal
Sys_Check_Cancel_Sig(system_check_cancel){
    Thread_Group *group = threadvars.groups + thread->group_id;
    
    b32 result = false;
    i32 thread_index = thread->id - 1;
    i32 cancel_lock_id = group->cancel_lock0 + thread_index;
    Mutex *cancel_lock = &threadvars.locks[cancel_lock_id];
    
    system_acquire_lock(cancel_lock);
    if (thread->cancel){
        result = true;
    }
    system_release_lock(cancel_lock);
    return(result);
}

internal
Sys_Grow_Thread_Memory_Sig(system_grow_thread_memory){
    Mutex *cancel_lock = &threadvars.locks[CANCEL_LOCK0 + memory->id - 1];
    
    system_acquire_lock(cancel_lock);
    void *old_data = memory->data;
    i32 old_size = memory->size;
    i32 new_size = l_round_up_i32(memory->size*2, KB(4));
    memory->data = system_memory_allocate(new_size);
    memory->size = new_size;
    if (old_data != 0){
        memcpy(memory->data, old_data, old_size);
        system_memory_free(old_data, old_size);
    }
    system_release_lock(cancel_lock);
}

internal
INTERNAL_Sys_Get_Thread_States_Sig(system_internal_get_thread_states){
    Thread_Group *group = threadvars.groups + id;
    Work_Queue *queue = threadvars.queues + id;
    Unbounded_Work_Queue *source_queue = &group->queue;
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

// BOTTOM

