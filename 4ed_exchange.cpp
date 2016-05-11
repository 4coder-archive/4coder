/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 9.12.2015
 *
 * Exchange stuff
 *
 */

// TOP

internal void
ex__file_insert(File_Slot *pos, File_Slot *file){
    pos->next->prev = file;
    file->next = pos->next;
    pos->next = file;
    file->prev = pos;
}

internal void
ex__file_remove(File_Slot *file){
    file->next->prev = file->prev;
    file->prev->next = file->next;
}

internal void
ex__check_file(File_Slot *pos){
    File_Slot *file = pos;
    
    Assert(pos == pos->next->prev);
    
    for (pos = pos->next;
         file != pos;
         pos = pos->next){
        Assert(pos == pos->next->prev);
    }
}

internal void
ex__check(File_Exchange *file_exchange){
    ex__check_file(&file_exchange->available);
    ex__check_file(&file_exchange->active);
    ex__check_file(&file_exchange->free_list);
}

internal void
ex__clear(File_Slot *file){
    file->data = 0;
    file->size = 0;
    file->max = 0;
    file->flags = 0;
}

internal File_Slot*
ex__get_file(Exchange *exchange){
    File_Exchange *files = &exchange->file;
    File_Slot *file;

    ++files->num_active;

    file = files->available.next;
    ex__file_remove(file);
    ex__clear(file);
    ex__file_insert(&files->active, file);
    ex__check(files);

    return file;
}

internal void
ex__set_filename(File_Slot *file, char *filename, int len){
    memcpy(file->filename, filename, len);
    file->filename[len] = 0;
    file->filename_len = len;
}

internal i32
exchange_request_file(Exchange *exchange, char *filename, int len){
    File_Exchange *files = &exchange->file;
    i32 result = 0;

    if (len+1 < FileNameMax){
        if (files->num_active < files->max){
            File_Slot *file = ex__get_file(exchange);
            ex__set_filename(file, filename, len);
            
            file->flags |= FEx_Request;
            result = (int)(file - files->files) + 1;
        }
    }
    
    return result;
}

struct File_Ready_Result{
    byte *data;
    i32 size;
    i32 max;
    b32 exists;
    b32 ready;
};

internal File_Ready_Result
exchange_file_ready(Exchange *exchange, i32 file_id){
    File_Ready_Result result = {0};
    File_Exchange *files = &exchange->file;
    File_Slot *file = 0;
    
    if (file_id > 0 && file_id <= files->max){
        file = files->files + file_id - 1;
        if (file->flags & FEx_Ready){
            result.data = file->data;
            result.size = file->size;
            result.max = file->max;
            result.exists = 1;
            result.ready = 1;
        }
        if (file->flags & FEx_Not_Exist){
            result.data = 0;
            result.size = 0;
            result.max = 0;
            result.exists = 0;
            result.ready = 1;
        }
    }
    
    return(result);
}

internal b32
exchange_file_does_not_exist(Exchange *exchange, i32 file_id){
    File_Exchange *files = &exchange->file;
    b32 result = 1;
    File_Slot *slot;
    
    if (file_id > 0 && file_id <= files->max){
        slot = files->files + file_id - 1;
        if (!(slot->flags & FEx_Not_Exist)){
            result = 0;
        }
    }
    
    return result;
}

internal i32
exchange_save_file(Exchange *exchange, char *filename, int len,
                   byte *data, int size, int max){
    File_Exchange *files = &exchange->file;
    i32 result = 0;

    if (len+1 < FileNameMax){
        if (files->num_active < files->max){
            File_Slot *file = ex__get_file(exchange);
            ex__set_filename(file, filename, len);
            
            file->flags |= FEx_Save;
            file->data = data;
            file->size = size;
            file->max = max;
            
            result = (int)(file - files->files) + 1;
        }
    }
    
    return result;
}

internal b32
exchange_file_save_complete(Exchange *exchange, i32 file_id, byte **data, int *size, int *max, int *failed){
    File_Exchange *files = &exchange->file;
    b32 result = 0;

    if (file_id > 0 && file_id <= files->max){
        File_Slot *file = files->files + file_id - 1;
        if (file->flags & FEx_Save_Complete || file->flags & FEx_Save_Failed){
            *data = file->data;
            *size = file->size;
            *max = file->max;
            result = 1;
            
            *failed = (file->flags & FEx_Save_Complete)?(1):(0);
        }
    }
    
    return result;
}

internal char*
exchange_file_filename(Exchange *exchange, i32 file_id, i32 *size = 0){
    File_Exchange *files = &exchange->file;
    char *result = 0;

    if (file_id > 0 && file_id <= files->max){
        File_Slot *file = files->files + file_id - 1;
        result = file->filename;
        if (size) *size = file->filename_len;
    }
    
    return result;
}

internal void
exchange_free_file(Exchange *exchange, i32 file_id){
    File_Exchange *files = &exchange->file;

    if (file_id > 0 && file_id <= files->max){
        File_Slot *file = files->files + file_id - 1;
        ex__file_remove(file);
        ex__file_insert(&files->free_list, file);
        ex__check(files);
    }
}

internal void
exchange_clear_file(Exchange *exchange, i32 file_id){
    File_Exchange *files = &exchange->file;

    if (file_id > 0 && file_id <= files->max){
        File_Slot *file = files->files + file_id - 1;
        ex__clear(file);
    }
}

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

