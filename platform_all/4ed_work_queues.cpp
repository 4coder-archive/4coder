/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Cross platform logic for work queues.
 *
 */

// TOP

internal void
job_proc(System_Functions *system, void *lpParameter){
    Thread_Context *thread = (Thread_Context*)lpParameter;
    Work_Queue *queue = win32vars.queues + thread->group_id;
    Thread_Group *group = win32vars.groups + thread->group_id;
    
    i32 thread_index = thread->id - 1;
    
    i32 cancel_lock = group->cancel_lock0 + thread_index;
    i32 cancel_cv = group->cancel_cv0 + thread_index;
    
    Thread_Memory *thread_memory = win32vars.thread_memory + thread_index;
    
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
                    full_job->job.callback(system, thread, thread_memory, full_job->job.data);
                    system_schedule_step();
                    thread->running = false;
                    
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
flush_unbounded_queue_to_main(Unbounded_Work_Queue *source_queue, Work_Queue *queue, i32 thread_count){
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
    
    return(semaphore_release_count);
}

// BOTTOM

