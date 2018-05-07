/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * Working_Set data structure
 *
 */

// TOP

// TODO(allen): Find a real home for this... string library needs a makeover.
internal String
push_string(Partition *part, char *str, i32 len){
    char *space = (char*)push_array(part, char, len + 1);
    memcpy(space, str, len);
    space[len] = 0;
    String string = make_string_cap(space, len, len + 1);
    return(string);
}

internal String
push_string(Partition *part, String str){
    String res = push_string(part, str.str, str.size);
    return(res);
}

internal String
push_string(Partition *part, char *str, i32 len, i32 cap){
    cap = clamp_bottom(len + 1, cap);
    char *space = (char*)push_array(part, char, cap);
    memcpy(space, str, len);
    space[len] = 0;
    String string = make_string_cap(space, len, cap);
    return(string);
}

internal String
push_string(Partition *part, String str, i32 cap){
    String res = push_string(part, str.str, str.size, cap);
    return(res);
}

////////////////////////////////

internal void
working_set_extend_memory(Working_Set *working_set, Editing_File *new_space, i16 number_of_files){
    Assert(working_set->array_count < working_set->array_max);
    
    i16 high_part = working_set->array_count++;
    working_set->file_arrays[high_part].files = new_space;
    working_set->file_arrays[high_part].size = number_of_files;
    
    working_set->file_max += number_of_files;
    
    Buffer_Slot_ID id = {0};
    id.part[1] = high_part;
    
    Editing_File *file_ptr = new_space;
    File_Node *free_sentinel = &working_set->free_sentinel;
    for (i16 i = 0; i < number_of_files; ++i, ++file_ptr){
        id.part[0] = i;
        file_ptr->id = id;
        dll_insert(free_sentinel, &file_ptr->node);
    }
}

internal Editing_File*
working_set_alloc(Working_Set *working_set){
    Editing_File *result = 0;
    
    if (working_set->file_count < working_set->file_max){
        File_Node *node = working_set->free_sentinel.next;
        Assert(node != &working_set->free_sentinel);
        result = (Editing_File*)node;
        
        dll_remove(node);
        Buffer_Slot_ID id = result->id;
        memset(result, 0, sizeof(*result));
        result->id = id;
        dll_insert(&working_set->used_sentinel, node);
        result->settings.display_width = working_set->default_display_width;
        result->settings.minimum_base_display_width = working_set->default_minimum_base_display_width;
        result->settings.wrap_indicator = WrapIndicator_Show_At_Wrap_Edge;
        init_file_markers_state(&result->markers);
        ++working_set->file_count;
    }
    
    return(result);
}

internal Editing_File*
working_set_alloc_always(Working_Set *working_set, General_Memory *general){
    Editing_File *result = 0;
    if (working_set->file_count == working_set->file_max && working_set->array_count < working_set->array_max){
        i16 new_count = (i16)clamp_top(working_set->file_max, max_i16);
        Editing_File *new_chunk = gen_array(general, Editing_File, new_count);
        working_set_extend_memory(working_set, new_chunk, new_count);
    }
    result = working_set_alloc(working_set);
    return(result);
}

inline void
working_set_free_file(General_Memory *general, Working_Set  *working_set, Editing_File *file){
    if (working_set->sync_check_iter == &file->node){
        working_set->sync_check_iter = working_set->sync_check_iter->next;
    }
    
    file->is_dummy = 1;
    dll_remove(&file->node);
    dll_insert(&working_set->free_sentinel, &file->node);
    --working_set->file_count;
}

inline Editing_File*
working_set_index(Working_Set *working_set, Buffer_Slot_ID id){
    Editing_File *result = 0;
    File_Array *array = 0;
    
    if (id.part[1] >= 0 && id.part[1] < working_set->array_count){
        array = working_set->file_arrays + id.part[1];
        if (id.part[0] >= 0 && id.part[0] < array->size){
            result = array->files + id.part[0];
        }
    }
    
    return(result);
}

inline Editing_File*
working_set_index(Working_Set *working_set, i32 id){
    Editing_File *result = working_set_index(working_set, to_file_id(id));
    return(result);
}

inline Editing_File*
working_set_get_active_file(Working_Set *working_set, Buffer_Slot_ID id){
    Editing_File *result = 0;
    result = working_set_index(working_set, id);
    if (result != 0 && result->is_dummy){
        result = 0;
    }
    return(result);
}

inline Editing_File*
working_set_get_active_file(Working_Set *working_set, i32 id){
    Editing_File *result= working_set_get_active_file(working_set, to_file_id(id));
    return(result);
}

internal void
working_set_init(Working_Set *working_set, Partition *partition, General_Memory *general){
    i16 init_count = 16;
    i16 array_init_count = 256;
    
    dll_init_sentinel(&working_set->free_sentinel);
    dll_init_sentinel(&working_set->used_sentinel);
    
    working_set->array_max = array_init_count;
    working_set->file_arrays = push_array(partition, File_Array, array_init_count);
    
    Editing_File *files = push_array(partition, Editing_File, init_count);
    working_set_extend_memory(working_set, files, init_count);
    
    // TODO(NAME): Unclear that this is still needed.  But double check that the buffer id 0 does not start getting used by the next real buffer when this is removed before actually removing it.  Buffer id cannot be allowed to be zero on real buffers.
#if 1
    // NOTE(allen): init null file
    {
        Editing_File *null_file = working_set_index(working_set, 0);
        dll_remove(&null_file->node);
        null_file->is_dummy = 1;
        ++working_set->file_count;
    }
#endif
    
    // NOTE(allen): init canon table
    {
        i32 item_size = sizeof(File_Name_Entry);
        i32 table_size = working_set->file_max;
        i32 mem_size = table_required_mem_size(table_size, item_size);
        void *mem = general_memory_allocate(general, mem_size);
        memset(mem, 0, mem_size);
        table_init_memory(&working_set->canon_table, mem, table_size, item_size);
    }
    
    // NOTE(allen): init name table
    {
        i32 item_size = sizeof(File_Name_Entry);
        i32 table_size = working_set->file_max;
        i32 mem_size = table_required_mem_size(table_size, item_size);
        void *mem = general_memory_allocate(general, mem_size);
        memset(mem, 0, mem_size);
        table_init_memory(&working_set->name_table, mem, table_size, item_size);
    }
}

inline void
working_set__grow_if_needed(Table *table, General_Memory *general, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    if (table_at_capacity(table)){
        Table btable = {0};
        i32 new_max = table->max * 2;
        i32 mem_size = table_required_mem_size(new_max, table->item_size);
        void *mem = general_memory_allocate(general, mem_size);
        table_init_memory(&btable, mem, new_max, table->item_size);
        table_clear(&btable);
        table_rehash(table, &btable, 0, hash_func, comp_func);
        general_memory_free(general, table->hash_array);
        *table = btable;
    }
}

internal Editing_File*
working_set_contains_basic(Working_Set *working_set, Table *table, String name){
    Editing_File *result = 0;
    
    File_Name_Entry *entry = (File_Name_Entry*)
        table_find_item(table, &name, 0, tbl_string_hash, tbl_string_compare);
    if (entry){
        result = working_set_index(working_set, entry->id);
    }
    
    return(result);
}

internal b32
working_set_add_basic(General_Memory *general, Working_Set *working_set, Table *table, Editing_File *file, String name){
    working_set__grow_if_needed(table, general, 0, tbl_string_hash, tbl_string_compare);
    
    File_Name_Entry entry;
    entry.name = name;
    entry.id = file->id;
    b32 result = (table_add(table, &entry, 0, tbl_string_hash, tbl_string_compare) == 0);
    return(result);
}

internal void
working_set_remove_basic(Working_Set *working_set, Table *table, String name){
    table_remove_match(table, &name, 0, tbl_string_hash, tbl_string_compare);
}

internal Editing_File*
working_set_contains_canon(Working_Set *working_set, String name){
    Editing_File *result = working_set_contains_basic(working_set, &working_set->canon_table, name);
    return(result);
}

internal b32
working_set_canon_add(General_Memory *general, Working_Set *working_set, Editing_File *file, String name){
    b32 result = working_set_add_basic(general,working_set, &working_set->canon_table, file, name);
    return(result);
}

internal void
working_set_canon_remove(Working_Set *working_set, String name){
    working_set_remove_basic(working_set, &working_set->canon_table, name);
}

internal Editing_File*
working_set_contains_name(Working_Set *working_set, String name){
    Editing_File *result = working_set_contains_basic(working_set, &working_set->name_table, name);
    return(result);
}

internal b32
working_set_add_name(General_Memory *general, Working_Set *working_set, Editing_File *file, String name){
    b32 result = working_set_add_basic(general, working_set, &working_set->name_table, file, name);
    return(result);
}

internal void
working_set_remove_name(Working_Set *working_set, String name){
    working_set_remove_basic(working_set, &working_set->name_table, name);
}


// TODO(allen): Pick better first options.
internal Editing_File*
working_set_lookup_file(Working_Set *working_set, String string){
    Editing_File *file = 0;
    
    // TODO(allen): use the name table for this
    for (File_Node *node = working_set->used_sentinel.next;
         node != &working_set->used_sentinel;
         node = node->next){
        Editing_File *nfile = (Editing_File*)node;
        if (string.size == 0 || match_ss(string, nfile->unique_name.name)){
            file = nfile;
            break;
        }
    }
    
    if (file == 0){
        for (File_Node *node = working_set->used_sentinel.next;
             node = &working_set->used_sentinel;
             node = node->next){
            Editing_File *nfile = (Editing_File*)node;
            if (string.size == 0 || has_substr_s(nfile->unique_name.name, string)){
                file = nfile;
                break;
            }
        }
    }
    
    return(file);
}

internal void
touch_file(Working_Set *working_set, Editing_File *file){
    Assert(file != 0);
    Assert(!file->is_dummy);
    dll_remove(&file->node);
    dll_insert(&working_set->used_sentinel, &file->node);
}

////////////////////////////////

// TODO(allen): Bring the clipboard fully to the custom side.
internal String*
working_set_next_clipboard_string(General_Memory *general, Working_Set *working, i32 str_size){
    i32 clipboard_current = working->clipboard_current;
    if (working->clipboard_size == 0){
        clipboard_current = 0;
        working->clipboard_size = 1;
    }
    else{
        ++clipboard_current;
        if (clipboard_current >= working->clipboard_max_size){
            clipboard_current = 0;
        }
        else if (working->clipboard_size <= clipboard_current){
            working->clipboard_size = clipboard_current + 1;
        }
    }
    String *result = &working->clipboards[clipboard_current];
    working->clipboard_current = clipboard_current;
    working->clipboard_rolling = clipboard_current;
    char *new_str;
    if (result->str != 0){
        new_str = (char*)general_memory_reallocate(general, result->str, result->size, str_size);
    }
    else{
        new_str = (char*)general_memory_allocate(general, str_size+1);
    }
    // TODO(allen): What if new_str == 0?
    *result = make_string_cap(new_str, 0, str_size);
    return(result);
}

internal String*
working_set_clipboard_index(Working_Set *working, i32 index){
    String *result = 0;
    i32 size = working->clipboard_size;
    i32 current = working->clipboard_current;
    if (index >= 0 && size > 0){
        index = index % size;
        index = current + size - index;
        index = index % size;
        result = &working->clipboards[index];
    }
    return(result);
}

internal String*
working_set_clipboard_head(Working_Set *working){
    String *result = 0;
    if (working->clipboard_size > 0){
        working->clipboard_rolling = 0;
        result = working_set_clipboard_index(working, working->clipboard_rolling);
    }
    return(result);
}

internal String*
working_set_clipboard_roll_down(Working_Set *working){
    String *result = 0;
    if (working->clipboard_size > 0){
        i32 clipboard_index = working->clipboard_rolling;
        ++clipboard_index;
        working->clipboard_rolling = clipboard_index;
        result = working_set_clipboard_index(working, working->clipboard_rolling);
    }
    return(result);
}

////////////////////////////////

internal b32
get_canon_name(System_Functions *system, String filename,
               Editing_File_Name *canon_name){
    canon_name->name = make_fixed_width_string(canon_name->name_);
    
    canon_name->name.size = system->get_canonical(filename.str, filename.size, canon_name->name.str, canon_name->name.memory_size);
    terminate_with_null(&canon_name->name);
    
    b32 result = (canon_name->name.size != 0);
    return(result);
}

internal void
buffer_bind_file(System_Functions *system, General_Memory *general,
                 Working_Set *working_set,
                 Editing_File *file, String canon_filename){
    Assert(file->unique_name.name.size == 0);
    Assert(file->canon.name.size == 0);
    
    file->canon.name = make_fixed_width_string(file->canon.name_);
    copy(&file->canon.name, canon_filename);
    terminate_with_null(&file->canon.name);
    system->add_listener(file->canon.name.str);
    if (!working_set_canon_add(general, working_set, file, file->canon.name)){
        InvalidCodePath;
    }
}

internal void
buffer_unbind_file(System_Functions *system, Working_Set *working_set, Editing_File *file){
    Assert(file->unique_name.name.size == 0);
    Assert(file->canon.name.size != 0);
    
    system->remove_listener(file->canon.name_);
    working_set_canon_remove(working_set, file->canon.name);
    file->canon.name.size = 0;
}

internal b32
buffer_name_has_conflict(Working_Set *working_set, String base_name){
    b32 hit_conflict = false;
    
    File_Node *used_nodes = &working_set->used_sentinel;
    for (File_Node *node = used_nodes->next; node != used_nodes; node = node->next){
        Editing_File *file_ptr = (Editing_File*)node;if (file_is_ready(file_ptr) && match(base_name, file_ptr->unique_name.name)){
            hit_conflict = true;
            break;
        }
    }
    
    return(hit_conflict);
}

internal void
buffer_resolve_name_low_level(Working_Set *working_set, Editing_File_Name *name, String base_name){
    Assert(name->name.str != 0);
    
    copy(&name->name, base_name);
    
    i32 original_len = name->name.size;
    i32 file_x = 0;
    for (b32 hit_conflict = true; hit_conflict;){
        hit_conflict = buffer_name_has_conflict(working_set, name->name);
        if (hit_conflict){
            ++file_x;
            name->name.size = original_len;
            append(&name->name, " (");
            append_int_to_str(&name->name, file_x);
            append(&name->name, ")");
        }
    }
}

internal void
buffer_bind_name_low_level(General_Memory *general, Working_Set *working_set, Editing_File *file, String base_name, String name){
    Assert(file->base_name.name.size == 0);
    Assert(file->unique_name.name.size == 0);
    
    Editing_File_Name new_name = {0};
    new_name.name = make_fixed_width_string(new_name.name_);
    buffer_resolve_name_low_level(working_set, &new_name, name);
    
    file->base_name.name = make_fixed_width_string(file->base_name.name_);
    copy(&file->base_name.name, base_name);
    
    file->unique_name.name = make_fixed_width_string(file->unique_name.name_);
    copy(&file->unique_name.name, new_name.name);
    
    if (!working_set_add_name(general, working_set, file, file->unique_name.name)){
        InvalidCodePath;
    }
}

internal void
buffer_unbind_name_low_level(Working_Set *working_set, Editing_File *file){
    Assert(file->base_name.name.size != 0);
    Assert(file->unique_name.name.size != 0);
    working_set_remove_name(working_set, file->unique_name.name);
    file->base_name.name.size = 0;
    file->unique_name.name.size = 0;
}

internal void
buffer_bind_name(Models *models, General_Memory *general, Partition *scratch,
                 Working_Set *working_set,
                 Editing_File *file, String base_name){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    // List of conflict files.
    Editing_File **conflict_file_ptrs = push_array(scratch, Editing_File*, 0);
    int32_t conflict_count = 0;
    
    {
        Editing_File **new_file_ptr = push_array(scratch, Editing_File*, 1);
        *new_file_ptr = file;
        ++conflict_count;
    }
    
    File_Node *used_nodes = &working_set->used_sentinel;
    for (File_Node *node = used_nodes->next; node != used_nodes; node = node->next){
        Editing_File *file_ptr = (Editing_File*)node;
        if (file_is_ready(file_ptr) && match(base_name, file_ptr->base_name.name)){
            Editing_File **new_file_ptr = push_array(scratch, Editing_File*, 1);
            *new_file_ptr = file_ptr;
            ++conflict_count;
        }
    }
    
    // Fill conflict array.
    Buffer_Name_Conflict_Entry *conflicts = push_array(scratch, Buffer_Name_Conflict_Entry, conflict_count);
    
    for (int32_t i = 0; i < conflict_count; ++i){
        Editing_File *file_ptr = conflict_file_ptrs[i];
        Buffer_Name_Conflict_Entry *entry = &conflicts[i];
        entry->buffer_id = file_ptr->id.id;
        
        String file_name = push_string(scratch, file_ptr->canon.name);
        entry->file_name = file_name.str;
        entry->file_name_len = file_name.size;
        
        String term_base_name = push_string(scratch, base_name);
        entry->base_name = term_base_name.str;
        entry->base_name_len = term_base_name.size;
        
        String b = base_name;
        if (i > 0){
            b = file_ptr->unique_name.name;
        }
        i32 unique_name_capacity = 256;
        String unique_name = push_string(scratch, b, unique_name_capacity);
        entry->unique_name_in_out = unique_name.str;
        entry->unique_name_len_in_out = unique_name.size;
        entry->unique_name_capacity = unique_name_capacity;
    }
    
    // Get user's resolution data.
    if (models->buffer_name_resolver != 0){
        models->buffer_name_resolver(&models->app_links, conflicts, conflict_count);
    }
    
    // Re-bind all of the files
    for (int32_t i = 0; i < conflict_count; ++i){
        Editing_File *file_ptr = conflict_file_ptrs[i];
        if (file_ptr->unique_name.name.str != 0){
            buffer_unbind_name_low_level(working_set, file_ptr);
        }
    }
    for (int32_t i = 0; i < conflict_count; ++i){
        Editing_File *file_ptr = conflict_file_ptrs[i];
        Buffer_Name_Conflict_Entry *entry = &conflicts[i];
        String unique_name = make_string(entry->unique_name_in_out, entry->unique_name_len_in_out);
        buffer_bind_name_low_level(general, working_set, file_ptr, base_name, unique_name);
    }
    
    end_temp_memory(temp);
}

////////////////////////////////

internal Editing_File*
open_file(System_Functions *system, Models *models, String filename){
    Editing_File *file = 0;
    Editing_File_Name canon_name = {0};
    
    if (terminate_with_null(&filename) &&
        get_canon_name(system, filename, &canon_name)){
        Working_Set *working_set = &models->working_set;
        file = working_set_contains_canon(working_set, canon_name.name);
        if (file == 0){
            Plat_Handle handle;
            if (system->load_handle(canon_name.name.str, &handle)){
                Mem_Options *mem = &models->mem;
                General_Memory *general = &mem->general;
                Partition *part = &mem->part;
                
                file = working_set_alloc_always(working_set, general);
                buffer_bind_file(system, general, working_set, file, canon_name.name);
                buffer_bind_name(models, general, part, working_set, file, front_of_directory(filename));
                
                i32 size = system->load_size(handle);
                char *buffer = 0;
                b32 gen_buffer = 0;
                
                Temp_Memory temp = begin_temp_memory(part);
                
                buffer = push_array(part, char, size);
                if (buffer == 0){
                    buffer = (char*)general_memory_allocate(general, size);
                    Assert(buffer);
                    gen_buffer = 1;
                }
                
                if (system->load_file(handle, buffer, size)){
                    system->load_close(handle);
                    init_normal_file(system, models, buffer, size, file);
                }
                else{
                    system->load_close(handle);
                }
                
                if (gen_buffer){
                    general_memory_free(general, buffer);
                }
                
                end_temp_memory(temp);
            }
        }
    }
    
    return(file);
}

// BOTTOM

