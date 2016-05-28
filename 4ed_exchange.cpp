/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 9.12.2015
 *
 * Exchange stuff
 *
 */

// TOP

// NOTE(allen): Uhhh.... is it just me or did it get awkward
// in here when I deleted all the file exchange stuff?

internal b32
queue_job_is_pending(Work_Queue *queue, u32 job_id){
    b32 result;
    u32 job_index;
    Full_Job_Data *full_job;
    
    job_index = job_id % QUEUE_WRAP;
    full_job = queue->jobs + job_index;
    
    Assert(full_job->id == job_id);
    
    result = 0;
    if (full_job->running_thread != 0){
        result = 1;
    }
    
    return(result);
}

// BOTTOM

