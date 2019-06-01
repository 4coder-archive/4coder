/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * Working_Set data structure
 *
 */

// TOP

internal void
working_set_extend_memory(Working_Set *working_set, Editing_File *new_space, i16 number_of_files){
    Assert(working_set->array_count < working_set->array_max);
    
    i16 high_part = working_set->array_count++;
    working_set->file_arrays[high_part].files = new_space;
    working_set->file_arrays[high_part].size = number_of_files;
    
    working_set->file_max += number_of_files;
    
    Buffer_Slot_ID id = {};
    id.part[1] = high_part;
    
    Editing_File *file_ptr = new_space;
    Node *free_sentinel = &working_set->free_sentinel;
    for (i16 i = 0; i < number_of_files; ++i, ++file_ptr){
        id.part[0] = i;
        file_ptr->id = id;
        dll_insert(free_sentinel, &file_ptr->main_chain_node);
    }
}

internal void
working_set_file_default_settings(Working_Set *working_set, Editing_File *file){
    memset(&file->settings, 0, sizeof(file->settings));
    file->settings.display_width = working_set->default_display_width;
    file->settings.minimum_base_display_width = working_set->default_minimum_base_display_width;
    file->settings.wrap_indicator = WrapIndicator_Show_At_Wrap_Edge;
}

// TODO(allen): do(restructure so that editing file is returned cleared to zero)
internal Editing_File*
working_set_alloc_always(Working_Set *working_set, Heap *heap, Lifetime_Allocator *lifetime_allocator){
    Editing_File *result = 0;
    if (working_set->file_count == working_set->file_max && working_set->array_count < working_set->array_max){
        i16 new_count = (i16)clamp_top(working_set->file_max, max_i16);
        Editing_File *new_chunk = heap_array(heap, Editing_File, new_count);
        working_set_extend_memory(working_set, new_chunk, new_count);
    }
    
    if (working_set->file_count < working_set->file_max){
        Node *node = working_set->free_sentinel.next;
        Assert(node != &working_set->free_sentinel);
        result = CastFromMember(Editing_File, main_chain_node, node);
        
        ++working_set->file_count;
        
        dll_remove(node);
        dll_insert(&working_set->used_sentinel, node);
        
        Node node_val = result->main_chain_node;
        Buffer_Slot_ID id_val = result->id;
        memset(result, 0, sizeof(*result));
        result->main_chain_node  = node_val;
        result->id = id_val;
        
        working_set_file_default_settings(working_set, result);
    }
    
    return(result);
}

internal void
working_set_free_file(Heap *heap, Working_Set  *working_set, Editing_File *file){
    if (working_set->sync_check_iter == &file->main_chain_node){
        working_set->sync_check_iter = working_set->sync_check_iter->next;
    }
    file->is_dummy = true;
    dll_remove(&file->main_chain_node);
    dll_insert(&working_set->free_sentinel, &file->main_chain_node);
    --working_set->file_count;
}

internal Editing_File*
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

internal Editing_File*
working_set_index(Working_Set *working_set, i32 id){
    return(working_set_index(working_set, to_file_id(id)));
}

internal Editing_File*
working_set_get_active_file(Working_Set *working_set, Buffer_Slot_ID id){
    Editing_File *result = working_set_index(working_set, id);
    if (result != 0 && result->is_dummy){
        result = 0;
    }
    return(result);
}

internal Editing_File*
working_set_get_active_file(Working_Set *working_set, Buffer_ID id){
    return(working_set_get_active_file(working_set, to_file_id(id)));
}

// TODO(allen): REWRITE all of working set
internal void
working_set_init(System_Functions *system, Working_Set *working_set, Arena *arena, Heap *heap){
    i16 init_count = 16;
    i16 array_init_count = 256;
    
    dll_init_sentinel(&working_set->free_sentinel);
    dll_init_sentinel(&working_set->used_sentinel);
    
    working_set->edit_finished_list_first = 0;
    working_set->edit_finished_list_last = 0;
    working_set->edit_finished_count = 0;
    
    working_set->time_of_next_edit_finished_signal = 0;
    working_set->edit_finished_timer = system->wake_up_timer_create();
    working_set->do_not_mark_edits = false;
    
    working_set->array_max = array_init_count;
    working_set->file_arrays = push_array(arena, File_Array, array_init_count);
    
    Editing_File *files = push_array(arena, Editing_File, init_count);
    working_set_extend_memory(working_set, files, init_count);
    
    // TODO(NAME): do(clean up the rest of the null_file)
    // Unclear that this is still needed.  But double check that the buffer id 0 does not start getting used by the next real buffer when this 
    // is removed before actually removing it.  Buffer id cannot be allowed to be zero on real buffers.
#if 1
    // NOTE(allen): init null file
    {
        Editing_File *null_file = working_set_index(working_set, 0);
        dll_remove(&null_file->main_chain_node);
        null_file->is_dummy = true;
        ++working_set->file_count;
    }
#endif
    
    // NOTE(allen): init canon table
    {
        i32 item_size = sizeof(File_Name_Entry);
        i32 table_size = working_set->file_max;
        i32 mem_size = table_required_mem_size(table_size, item_size);
        void *mem = heap_allocate(heap, mem_size);
        memset(mem, 0, mem_size);
        table_init_memory(&working_set->canon_table, mem, table_size, item_size);
    }
    
    // NOTE(allen): init name table
    {
        i32 item_size = sizeof(File_Name_Entry);
        i32 table_size = working_set->file_max;
        i32 mem_size = table_required_mem_size(table_size, item_size);
        void *mem = heap_allocate(heap, mem_size);
        memset(mem, 0, mem_size);
        table_init_memory(&working_set->name_table, mem, table_size, item_size);
    }
}

internal void
working_set__grow_if_needed(Table *table, Heap *heap, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    if (table_at_capacity(table)){
        Table btable = {};
        i32 new_max = table->max * 2;
        i32 mem_size = table_required_mem_size(new_max, table->item_size);
        void *mem = heap_allocate(heap, mem_size);
        table_init_memory(&btable, mem, new_max, table->item_size);
        table_clear(&btable);
        table_rehash(table, &btable, 0, hash_func, comp_func);
        heap_free(heap, table->hash_array);
        *table = btable;
    }
}

internal Editing_File*
working_set_contains_basic(Working_Set *working_set, Table *table, String_Const_u8 name){
    Editing_File *result = 0;
    
    File_Name_Entry *entry = (File_Name_Entry*)
        table_find_item(table, &name, 0, tbl_string_hash, tbl_string_compare);
    if (entry){
        result = working_set_index(working_set, entry->id);
    }
    
    return(result);
}

internal b32
working_set_add_basic(Heap *heap, Working_Set *working_set, Table *table, Editing_File *file, String_Const_u8 name){
    working_set__grow_if_needed(table, heap, 0, tbl_string_hash, tbl_string_compare);
    
    File_Name_Entry entry;
    entry.name = name;
    entry.id = file->id;
    b32 result = (table_add(table, &entry, 0, tbl_string_hash, tbl_string_compare) == 0);
    return(result);
}

internal void
working_set_remove_basic(Working_Set *working_set, Table *table, String_Const_u8 name){
    table_remove_match(table, &name, 0, tbl_string_hash, tbl_string_compare);
}

internal Editing_File*
working_set_contains_canon(Working_Set *working_set, String_Const_u8 name){
    Editing_File *result = working_set_contains_basic(working_set, &working_set->canon_table, name);
    return(result);
}

internal b32
working_set_canon_add(Heap *heap, Working_Set *working_set, Editing_File *file, String_Const_u8 name){
    b32 result = working_set_add_basic(heap,working_set, &working_set->canon_table, file, name);
    return(result);
}

internal void
working_set_canon_remove(Working_Set *working_set, String_Const_u8 name){
    working_set_remove_basic(working_set, &working_set->canon_table, name);
}

internal Editing_File*
working_set_contains_name(Working_Set *working_set, String_Const_u8 name){
    return(working_set_contains_basic(working_set, &working_set->name_table, name));
}

internal b32
working_set_add_name(Heap *heap, Working_Set *working_set, Editing_File *file, String_Const_u8 name){
    b32 result = working_set_add_basic(heap, working_set, &working_set->name_table, file, name);
    return(result);
}

internal void
working_set_remove_name(Working_Set *working_set, String_Const_u8 name){
    working_set_remove_basic(working_set, &working_set->name_table, name);
}

internal Editing_File*
get_file_from_identifier(System_Functions *system, Working_Set *working_set, Buffer_Identifier buffer){
    Editing_File *file = 0;
    if (buffer.id != 0){
        file = working_set_get_active_file(working_set, buffer.id);
    }
    else if (buffer.name != 0){
        String_Const_u8 name = SCu8(buffer.name, buffer.name_len);
        file = working_set_contains_name(working_set, name);
    }
    return(file);
}

////////////////////////////////

// TODO(allen): Bring the clipboard fully to the custom side.
internal String_Const_u8*
working_set_next_clipboard_string(Heap *heap, Working_Set *working, umem str_size){
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
    String_Const_u8 *result = &working->clipboards[clipboard_current];
    working->clipboard_current = clipboard_current;
    working->clipboard_rolling = clipboard_current;
    if (result->str != 0){
        heap_free(heap, result->str);
    }
    u8 *new_str = (u8*)heap_allocate(heap, (i32)(str_size + 1));
    *result = SCu8(new_str, str_size);
    return(result);
}

internal String_Const_u8*
working_set_clipboard_index(Working_Set *working, i32 index){
    String_Const_u8 *result = 0;
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

internal String_Const_u8*
working_set_clipboard_head(Working_Set *working){
    String_Const_u8 *result = 0;
    if (working->clipboard_size > 0){
        working->clipboard_rolling = 0;
        result = working_set_clipboard_index(working, working->clipboard_rolling);
    }
    return(result);
}

internal String_Const_u8*
working_set_clipboard_roll_down(Working_Set *working){
    String_Const_u8 *result = 0;
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
get_canon_name(System_Functions *system, String_Const_u8 file_name, Editing_File_Name *canon_name){
    canon_name->name_size = system->get_canonical((char*)file_name.str, (u32)file_name.size,
                                                  (char*)canon_name->name_space, sizeof(canon_name->name_space));
    file_name_terminate(canon_name);
    return(canon_name->name_size > 0);
}

internal void
file_bind_file_name(System_Functions *system, Heap *heap, Working_Set *working_set, Editing_File *file, String_Const_u8 canon_file_name){
    Assert(file->unique_name.name_size == 0);
    Assert(file->canon.name_size == 0);
    umem size = canon_file_name.size;
    size = clamp_top(size, sizeof(file->canon.name_space) - 1);
    file->canon.name_size = size;
    block_copy(file->canon.name_space, canon_file_name.str, size);
    file_name_terminate(&file->canon);
    system->add_listener((char*)file->canon.name_space);
    b32 result = working_set_canon_add(heap, working_set, file, string_from_file_name(&file->canon));
    Assert(result);
}

internal void
buffer_unbind_file(System_Functions *system, Working_Set *working_set, Editing_File *file){
    Assert(file->unique_name.name_size == 0);
    Assert(file->canon.name_size != 0);
    system->remove_listener((char*)file->canon.name_space);
    working_set_canon_remove(working_set, string_from_file_name(&file->canon));
    file->canon.name_size = 0;
}

internal b32
buffer_name_has_conflict(Working_Set *working_set, String_Const_u8 base_name){
    b32 hit_conflict = false;
    Node *used_nodes = &working_set->used_sentinel;
    for (Node *node = used_nodes->next; node != used_nodes; node = node->next){
        Editing_File *file_ptr = CastFromMember(Editing_File, main_chain_node, node);
        if (file_is_ready(file_ptr) && string_match(base_name, string_from_file_name(&file_ptr->unique_name))){
            hit_conflict = true;
            break;
        }
    }
    return(hit_conflict);
}

internal void
buffer_resolve_name_low_level(Arena *scratch, Working_Set *working_set, Editing_File_Name *name, String_Const_u8 base_name){
    umem size = base_name.size;
    size = clamp_top(size, sizeof(name->name_space));
    block_copy(name->name_space, base_name.str, size);
    String_u8 string = Su8(name->name_space, size, sizeof(name->name_space));
    umem original_size = string.size;
    u64 file_x = 0;
    for (b32 hit_conflict = true; hit_conflict;){
        hit_conflict = buffer_name_has_conflict(working_set, string.string);
        if (hit_conflict){
            file_x += 1;
            string.size = original_size;
            Temp_Memory temp = begin_temp(scratch);
            String_Const_u8 int_str = string_from_integer(scratch, file_x, 10);
            string_append(&string, string_u8_litexpr(" ("));
            string_append(&string, int_str);
            string_append(&string, string_u8_litexpr(")"));
            end_temp(temp);
        }
    }
    name->name_size = string.size;
}

internal void
buffer_bind_name_low_level(Arena *scratch, Heap *heap, Working_Set *working_set, Editing_File *file, String_Const_u8 base_name, String_Const_u8 name){
    Assert(file->base_name.name_size == 0);
    Assert(file->unique_name.name_size == 0);
    
    Editing_File_Name new_name = {};
    buffer_resolve_name_low_level(scratch, working_set, &new_name, name);
    
    {
        umem size = base_name.size;
        size = clamp_top(size, sizeof(file->base_name.name_space));
        block_copy(file->base_name.name_space, base_name.str, size);
        file->base_name.name_size = size;
    }
    
    {
        umem size = new_name.name_size;
        block_copy(file->unique_name.name_space, new_name.name_space, size);
        file->unique_name.name_size = size;
    }
    
    b32 result = working_set_add_name(heap, working_set, file, string_from_file_name(&file->unique_name));
    Assert(result);
}

internal void
buffer_unbind_name_low_level(Working_Set *working_set, Editing_File *file){
    Assert(file->base_name.name_size != 0);
    Assert(file->unique_name.name_size != 0);
    working_set_remove_name(working_set, string_from_file_name(&file->unique_name));
    file->base_name.name_size = 0;
    file->unique_name.name_size = 0;
}

internal void
buffer_bind_name(Models *models, Heap *heap, Arena *scratch, Working_Set *working_set, Editing_File *file, String_Const_u8 base_name){
    Temp_Memory temp = begin_temp(scratch);
    
    // List of conflict files.
    struct Node_Ptr{
        Node_Ptr *next;
        Editing_File *file_ptr;
    };
    Node_Ptr *conflict_first = 0;
    Node_Ptr *conflict_last = 0;
    i32 conflict_count = 0;
    
    {
        Node_Ptr *node = push_array(scratch, Node_Ptr, 1);
        sll_queue_push(conflict_first, conflict_last, node);
        node->file_ptr = file;
        conflict_count += 1;
    }
    
    Node *used_nodes = &working_set->used_sentinel;
    for (Node *node = used_nodes->next;
         node != used_nodes;
         node = node->next){
        Editing_File *file_ptr = CastFromMember(Editing_File, main_chain_node, node);
        if (file_is_ready(file_ptr) && string_match(base_name, string_from_file_name(&file_ptr->base_name))){
            Node_Ptr *new_node = push_array(scratch, Node_Ptr, 1);
            sll_queue_push(conflict_first, conflict_last, new_node);
            new_node->file_ptr = file_ptr;
            conflict_count += 1;
        }
    }
    
    // Fill conflict array.
    Buffer_Name_Conflict_Entry *conflicts = push_array(scratch, Buffer_Name_Conflict_Entry, conflict_count);
    
    {
        i32 i = 0;
        for (Node_Ptr *node = conflict_first;
             node != 0;
             node = node->next, i += 1){
            Editing_File *file_ptr = node->file_ptr;
            Buffer_Name_Conflict_Entry *entry = &conflicts[i];
            entry->buffer_id = file_ptr->id.id;
            
            entry->file_name = string_copy(scratch, string_from_file_name(&file_ptr->canon));
            entry->base_name = string_copy(scratch, base_name);
            
            String_Const_u8 b = base_name;
            if (i > 0){
                b = string_from_file_name(&file_ptr->unique_name);
            }
            umem unique_name_capacity = 256;
            u8 *unique_name_buffer = push_array(scratch, u8, unique_name_capacity);
            Assert(b.size <= unique_name_capacity);
            block_copy(unique_name_buffer, b.str, b.size);
            entry->unique_name_in_out = unique_name_buffer;
            entry->unique_name_len_in_out = b.size;
            entry->unique_name_capacity = unique_name_capacity;
        }
    }
    
    // Get user's resolution data.
    if (models->buffer_name_resolver != 0){
        models->buffer_name_resolver(&models->app_links, conflicts, conflict_count);
    }
    
    // Re-bind all of the files
    {
        i32 i = 0;
        for (Node_Ptr *node = conflict_first;
             node != 0;
             node = node->next, i += 1){
            Editing_File *file_ptr = node->file_ptr;
            if (file_ptr->unique_name.name_size > 0){
                buffer_unbind_name_low_level(working_set, file_ptr);
            }
        }
    }
    
    {
        i32 i = 0;
        for (Node_Ptr *node = conflict_first;
             node != 0;
             node = node->next, i += 1){
            Editing_File *file_ptr = node->file_ptr;
            Buffer_Name_Conflict_Entry *entry = &conflicts[i];
            String_Const_u8 unique_name = SCu8(entry->unique_name_in_out, entry->unique_name_len_in_out);
            buffer_bind_name_low_level(scratch, heap, working_set, file_ptr, base_name, unique_name);
        }
    }
    
    end_temp(temp);
}

////////////////////////////////

internal void
file_touch(Working_Set *working_set, Editing_File *file){
    Assert(file != 0);
    Assert(!file->is_dummy);
    dll_remove(&file->main_chain_node);
    dll_insert(&working_set->used_sentinel, &file->main_chain_node);
}

internal Editing_File*
file_get_next(Working_Set *working_set, Editing_File *file){
    if (file != 0){
        file = CastFromMember(Editing_File, main_chain_node, file->main_chain_node.next);
        if (file == CastFromMember(Editing_File, main_chain_node, &working_set->used_sentinel)){
            file = 0;
        }
    }
    else{
        if (working_set->file_count > 0){
            Node *node = working_set->used_sentinel.next;
            file = CastFromMember(Editing_File, main_chain_node, node);
        }
    }
    return(file);
}

internal void
file_mark_edit_finished(Working_Set *working_set, Editing_File *file){
    // TODO(allen): do(propogate do_not_mark_edits down the edit pipeline to here)
    // This current method only works for synchronous calls, asynchronous calls will get the
    // wrong do_not_mark_edits value.
    if (!working_set->do_not_mark_edits){
        if (!file->edit_finished_marked == 0){
            zdll_push_back(working_set->edit_finished_list_first,
                           working_set->edit_finished_list_last,
                           &file->edit_finished_mark_node);
            file->edit_finished_marked = true;
            working_set->edit_finished_count += 1;
        }
    }
}

internal b32
file_unmark_edit_finished(Working_Set *working_set, Editing_File *file){
    b32 result = false;
    if (!working_set->do_not_mark_edits){
        if (file->edit_finished_marked){
            zdll_remove(working_set->edit_finished_list_first,
                        working_set->edit_finished_list_last,
                        &file->edit_finished_mark_node);
            file->edit_finished_mark_node.next = 0;
            file->edit_finished_mark_node.prev = 0;
            file->edit_finished_marked = false;
            working_set->edit_finished_count -= 1;
            
            result = true;
        }
    }
    return(result);
}

////////////////////////////////

internal Editing_File*
imp_get_file(Models *models, Buffer_ID buffer_id){
    Working_Set *working_set = &models->working_set;
    Editing_File *file = working_set_get_active_file(working_set, buffer_id);
    if (file != 0 && !file_is_ready(file)){
        file = 0;
    }
    return(file);
}

// BOTTOM

