/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

internal File_Data
sysshared_load_file(char *filename){
    File_Data result = {0};
    
    File_Loading loading =
        system_file_load_begin(filename);
    
    result.got_file = loading.exists;
    
    if (loading.size > 0){
        result.data.size = loading.size;
        result.data.data = (byte*)Win32GetMemory(result.data.size);
        
        if (!result.data.data){
            system_file_load_end(loading, 0);
            result = file_data_zero();
        }
        else{
            if (!system_file_load_end(loading, (char*)result.data.data)){
                Win32FreeMemory(result.data.data);
                result = file_data_zero();
            }
        }
    }
    
    return(result);
}

internal b32
sysshared_save_file(char *filename, char *data, i32 size){
    return(system_file_save(filename, data, size));
}

internal b32
usable_ascii(char c){
    b32 result = 1;
    if ((c < ' ' || c > '~') && c != '\n' && c != '\r' && c != '\t'){
        result = 0;
    }
    return(result);
}

internal void
sysshared_filter_real_files(char **files, i32 *file_count){
    i32 i, j;
    i32 end;
    
    end = *file_count;
    for (i = 0, j = 0; i < end; ++i){
        if (system_file_can_be_made(files[i])){
            files[j] = files[i];
            ++j;
        }
    }
    *file_count = j;
}

void
ex__file_insert(File_Slot *pos, File_Slot *file){
    file->next = pos->next;
    file->next->prev = file;
    file->prev = pos;
    pos->next = file;
}

void
ex__insert_range(File_Slot *start, File_Slot *end, File_Slot *pos){
    end->next->prev = start->prev;
    start->prev->next = end->next;
    
    end->next = pos->next;
    start->prev = pos;
    pos->next->prev = end;
    pos->next = start;
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
sysshared_init_file_exchange(
    Exchange *exchange, File_Slot *slots, i32 max,
    char **filename_space_out){
    char *filename_space;
    i32 i;
    
    exchange->file.max = max;
    exchange->file.available = file_slot_zero();
    exchange->file.available.next = &exchange->file.available;
    exchange->file.available.prev = &exchange->file.available;
    
    exchange->file.active = file_slot_zero();
    exchange->file.active.next = &exchange->file.active;
    exchange->file.active.prev = &exchange->file.active;
    
    exchange->file.free_list = file_slot_zero();
    exchange->file.free_list.next = &exchange->file.free_list;
    exchange->file.free_list.prev = &exchange->file.free_list;

    exchange->file.files = slots;
    memset(slots, 0, sizeof(File_Slot)*max);

    filename_space = (char*)
        system_get_memory(FileNameMax*exchange->file.max);

    File_Slot *slot = slots;
    for (i = 0; i < exchange->file.max; ++i, ++slot){
        ex__file_insert(&exchange->file.available, slot);
        slot->filename = filename_space;
        filename_space += FileNameMax;
    }

    if (filename_space_out) *filename_space_out = filename_space;
}

internal Partition
sysshared_scratch_partition(i32 size){
    void *data = system_get_memory(size);
    Partition part = make_part(data, size);
    return(part);
}

internal void
sysshared_partition_grow(Partition *part, i32 new_size){
    void *data = 0;
    if (new_size > part->max){
        // TODO(allen): attempt to grow in place by just
        // acquiring next vpages?!
        data = system_get_memory(new_size);
        memcpy(data, part->base, part->pos);
        system_free_memory(part->base);
        part->base = (char*)data;
    }
}

internal void
sysshared_partition_double(Partition *part){
    sysshared_partition_grow(part, part->max*2);
}

internal void*
sysshared_push_block(Partition *part, i32 size){
    void *result = 0;
    result = push_block(part, size);
    if (!result){
        sysshared_partition_grow(part, size+part->max);
        result = push_block(part, size);
    }
    return(result);
}

internal b32
sysshared_to_binary_path(String *out_filename, char *filename){
    b32 translate_success = 0;
    i32 max = out_filename->memory_size;
    i32 size = system_get_binary_path(out_filename);
    if (size > 0 && size < max-1){
        out_filename->size = size;
        if (append(out_filename, filename) && terminate_with_null(out_filename)){
            translate_success = 1;
        }
    }
    return (translate_success);
}

// BOTTOM

