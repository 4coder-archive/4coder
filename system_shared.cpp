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
        result.data.data = (byte*)system_get_memory(result.data.size);
        
        if (!result.data.data){
            system_file_load_end(loading, 0);
            result = file_data_zero();
        }
        else{
            if (!system_file_load_end(loading, (char*)result.data.data)){
                system_free_memory(result.data.data);
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

