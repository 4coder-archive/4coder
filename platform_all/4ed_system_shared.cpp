/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

#if !defined(FCODER_SYSTEM_SHARED_CPP)
#define FCODER_SYSTEM_SHARED_CPP

//
// Standard implementation of file system stuff based on the file track layer.
//

internal i32
system_get_binary_path_string(String *out){
    out->size = system_get_4ed_path(out->str, out->memory_size);
    return(out->size);
}

internal void
init_shared_vars(){
    umem scratch_size = KB(128);
    void *scratch_memory = system_memory_allocate(scratch_size);
    shared_vars.scratch = make_part(scratch_memory, (i32)scratch_size);
    
    umem font_scratch_size = MB(4);
    void *font_scratch_memory = system_memory_allocate(font_scratch_size);
    shared_vars.font_scratch = make_part(font_scratch_memory, (i32)font_scratch_size);
    
    umem pixel_scratch_size = MB(4);
    void *pixel_scratch_memory = system_memory_allocate(pixel_scratch_size);
    shared_vars.pixel_scratch = make_part(pixel_scratch_memory, (i32)pixel_scratch_size);
    
    shared_vars.track_table_size = KB(16);
    shared_vars.track_table = system_memory_allocate(shared_vars.track_table_size);
    
    shared_vars.track_node_size = KB(16);
    void *track_nodes = system_memory_allocate(shared_vars.track_node_size);
    
    i32 track_result = init_track_system(&shared_vars.track, &shared_vars.scratch, shared_vars.track_table, shared_vars.track_table_size, track_nodes, shared_vars.track_node_size);
    
    if (track_result != FileTrack_Good){
        exit(1);
    }
}

internal b32
handle_track_out_of_memory(i32 val){
    b32 result = false;
    
    switch (val){
        case FileTrack_OutOfTableMemory:
        {
            u32 new_table_size = shared_vars.track_table_size*2;
            void *new_table = system_memory_allocate(new_table_size);
            move_track_system(&shared_vars.track, &shared_vars.scratch, new_table, new_table_size);
            system_memory_free(shared_vars.track_table, shared_vars.track_table_size);
            shared_vars.track_table_size = new_table_size;
            shared_vars.track_table = new_table;
        }break;
        
        case FileTrack_OutOfListenerMemory:
        {
            shared_vars.track_node_size *= 2;
            void *node_expansion = system_memory_allocate(shared_vars.track_node_size);
            expand_track_system_listeners(&shared_vars.track, &shared_vars.scratch, node_expansion, shared_vars.track_node_size);
        }break;
        
        default: result = true; break;
    }
    
    return(result);
}

internal
Sys_Add_Listener_Sig(system_add_listener){
    b32 result = false;
    for (;;){
        i32 track_result = add_listener(&shared_vars.track, &shared_vars.scratch, (u8*)filename);
        if (handle_track_out_of_memory(track_result)){
            if (track_result == FileTrack_Good){
                result = true;
            }
            break;
        }
    }
    return(result);
}

internal
Sys_Remove_Listener_Sig(system_remove_listener){
    b32 result = false;
    i32 track_result = remove_listener(&shared_vars.track, &shared_vars.scratch, (u8*)filename);
    if (track_result == FileTrack_Good){
        result = true;
    }
    return(result);
}

internal
Sys_Get_File_Change_Sig(system_get_file_change){
    b32 result = false;
    
    i32 size = 0;
    i32 get_result = get_change_event(&shared_vars.track, &shared_vars.scratch, (u8*)buffer, max, &size);
    
    *required_size = size;
    *mem_too_small = false;
    if (get_result == FileTrack_Good){
        result = true;
    }
    else if (get_result == FileTrack_MemoryTooSmall){
        *mem_too_small = true;
        result = true;
    }
    
    return(result);
}

//
// General shared pieces
//

internal File_Data
sysshared_load_file(char *filename){
    File_Data result = {};
    
    Plat_Handle handle = {};
    if (system_load_handle(filename, &handle)){
        u32 size = system_load_size(handle);
        
        result.got_file = 1;
        if (size > 0){
            result.size = size;
            result.data = (char*)system_memory_allocate(size+1);
            
            if (!result.data){
                result = null_file_data;
            }
            else{
                if (!system_load_file(handle, result.data, size)){
                    system_memory_free(result.data, size+1);
                    result = null_file_data;
                }
            }
        }
        
        system_load_close(handle);
    }
    
    return(result);
}

internal void
sysshared_filter_real_files(char **files, i32 *file_count){
    i32 end = *file_count;
    i32 i = 0, j = 0;
    for (; i < end; ++i){
        if (system_file_can_be_made((u8*)files[i])){
            files[j] = files[i];
            ++j;
        }
    }
    *file_count = j;
}

// HACK(allen): Get rid of this now!?
internal Partition
sysshared_scratch_partition(i32 size){
    void *data = system_memory_allocate((umem)size);
    Partition part = make_part(data, size);
    return(part);
}

internal void
sysshared_partition_grow(Partition *part, i32 new_size){
    Assert(part->pos == 0);
    
    void *data = 0;
    if (new_size > part->max){
        data = system_memory_allocate((umem)new_size);
        memcpy(data, part->base, part->pos);
        system_memory_free(part->base, part->max);
        part->base = (char*)data;
        part->max = new_size;
    }
}

internal void*
sysshared_push_block(Partition *part, i32 size){
    void *result = push_array(part, i8, size);
    if (result == 0){
        sysshared_partition_grow(part, size + part->max);
        result = push_array(part, i8, size);
    }
    return(result);
}

internal b32
sysshared_to_binary_path(String *out_filename, char *filename){
    b32 translate_success = 0;
    i32 max = out_filename->memory_size;
    i32 size = out_filename->size = system_get_4ed_path(out_filename->str, out_filename->memory_size);
    if (size > 0 && size < max-1){
        out_filename->size = size;
        if (append_sc(out_filename, filename) && terminate_with_null(out_filename)){
            translate_success = 1;
        }
    }
    return(translate_success);
}

#endif

// BOTTOM

