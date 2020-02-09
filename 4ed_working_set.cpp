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
working_set_file_default_settings(Working_Set *working_set, Editing_File *file){
    block_zero_struct(&file->settings);
}

////////////////////////////////

internal void
file_change_notification_check(Arena *scratch, Working_Set *working_set, Editing_File *file){
    if (file->canon.name_size > 0 && !file->settings.unimportant){
        String_Const_u8 name = SCu8(file->canon.name_space, file->canon.name_size);
        File_Attributes attributes = system_quick_file_attributes(scratch, name);
        if ((attributes.last_write_time > file->attributes.last_write_time) ||
            (attributes.last_write_time == 0 && file->attributes.last_write_time > 0)){
            if (file->state.save_state == FileSaveState_SavedWaitingForNotification){
                file->state.save_state = FileSaveState_Normal;
                file->attributes = attributes;
            }
            else{
                file_add_dirty_flag(file, DirtyState_UnloadedChanges);
                if (file->external_mod_node.next == 0){
                    LogEventF(log_string(M), &working_set->arena, file->id, 0, system_thread_get_id(),
                              "external modification [lwt=0x%llx]", attributes.last_write_time);
                    dll_insert_back(&working_set->has_external_mod_sentinel, &file->external_mod_node);
                    system_signal_step(0);
                }
            }
        }
        file->attributes = attributes;
    }
}

internal void
file_change_notification_thread_main(void *ptr){
    Models *models = (Models*)ptr;
    Arena arena = make_arena_system();
    Working_Set *working_set = &models->working_set;
    for (;;){
        system_sleep(Thousand(250));
        Mutex_Lock lock(working_set->mutex);
        if (working_set->active_file_count > 0){
            i32 check_count = working_set->active_file_count/16;
            check_count = clamp(1, check_count, 100);
            Node *used = &working_set->active_file_sentinel;
            Node *node = working_set->sync_check_iterator;
            if (node == 0 || node == used){
                node = used->next;
            }
            for (i32 i = 0; i < check_count; i += 1){
                Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
                node = node->next;
                if (node == used){
                    node = node->next;
                }
                file_change_notification_check(&arena, working_set, file);
            }
            working_set->sync_check_iterator = node;
        }
    }
}

////////////////////////////////

internal Editing_File*
working_set_allocate_file(Working_Set *working_set, Lifetime_Allocator *lifetime_allocator){
    Editing_File *file = working_set->free_files;
    if (file == 0){
        file = push_array(&working_set->arena, Editing_File, 1);
    }
    else{
        sll_stack_pop(working_set->free_files);
    }
    block_zero_struct(file);
    
    dll_insert_back(&working_set->active_file_sentinel, &file->main_chain_node);
    dll_insert_back(&working_set->touch_order_sentinel, &file->touch_node);
    working_set->active_file_count += 1;
    
    file->id = working_set->id_counter;
    working_set->id_counter += 1;
    
    working_set_file_default_settings(working_set, file);
    
    table_insert(&working_set->id_to_ptr_table,
                 (u64)file->id, (u64)(PtrAsInt(file)));
    
    return(file);
}

internal void
working_set_free_file(Heap *heap, Working_Set *working_set, Editing_File *file){
    if (working_set->sync_check_iterator == &file->main_chain_node){
        working_set->sync_check_iterator = working_set->sync_check_iterator->next;
    }
    dll_remove(&file->main_chain_node);
    dll_remove(&file->touch_node);
    working_set->active_file_count -= 1;
    table_erase(&working_set->id_to_ptr_table, file->id);
    sll_stack_push(working_set->free_files, file);
}

internal Editing_File*
working_set_get_file(Working_Set *working_set, Buffer_ID id){
    Editing_File *result = 0;
    u64 val = 0;
    if (table_read(&working_set->id_to_ptr_table, id, &val)){
        result = (Editing_File*)(IntAsPtr(val));
    }
    return(result);
}

internal void
working_set_init(Models *models, Working_Set *working_set){
    block_zero_struct(working_set);
    working_set->arena = make_arena_system();
    
    working_set->id_counter = 1;
    
    dll_init_sentinel(&working_set->active_file_sentinel);
    dll_init_sentinel(&working_set->touch_order_sentinel);
    
    local_const i32 slot_count = 128;
    Base_Allocator *allocator = get_base_allocator_system();
    working_set->id_to_ptr_table = make_table_u64_u64(allocator, slot_count);
    working_set->canon_table = make_table_Data_u64(allocator, slot_count);
    working_set->name_table = make_table_Data_u64(allocator, slot_count);
    
    dll_init_sentinel(&working_set->has_external_mod_sentinel);
    working_set->mutex = system_mutex_make();
    working_set->file_change_thread = system_thread_launch(file_change_notification_thread_main, models);
}

internal Editing_File*
working_set_contains__generic(Working_Set *working_set, Table_Data_u64 *table, String_Const_u8 name){
    Editing_File *result = 0;
    u64 val = 0;
    if (table_read(table, make_data(name.str, name.size), &val)){
        result = working_set_get_file(working_set, (Buffer_ID)val);
    }
    return(result);
}

internal b32
working_set_add__generic(Table_Data_u64 *table, Buffer_ID id, String_Const_u8 name){
    return(table_insert(table, make_data(name.str, name.size), id));
}

internal void
working_set_remove__generic(Table_Data_u64 *table, String_Const_u8 name){
    table_erase(table, make_data(name.str, name.size));
}

internal Editing_File*
working_set_contains_canon(Working_Set *working_set, String_Const_u8 name){
    return(working_set_contains__generic(working_set, &working_set->canon_table, name));
}

internal b32
working_set_canon_add(Working_Set *working_set, Editing_File *file, String_Const_u8 name){
    return(working_set_add__generic(&working_set->canon_table, file->id, name));
}

internal void
working_set_canon_remove(Working_Set *working_set, String_Const_u8 name){
    working_set_remove__generic(&working_set->canon_table, name);
}

internal Editing_File*
working_set_contains_name(Working_Set *working_set, String_Const_u8 name){
    return(working_set_contains__generic(working_set, &working_set->name_table, name));
}

internal b32
working_set_add_name(Working_Set *working_set, Editing_File *file, String_Const_u8 name){
    return(working_set_add__generic(&working_set->name_table, file->id, name));
}

internal void
working_set_remove_name(Working_Set *working_set, String_Const_u8 name){
    working_set_remove__generic(&working_set->name_table, name);
}

internal Editing_File*
get_file_from_identifier(Working_Set *working_set, Buffer_Identifier buffer){
    Editing_File *file = 0;
    if (buffer.id != 0){
        file = working_set_get_file(working_set, buffer.id);
    }
    else if (buffer.name != 0){
        String_Const_u8 name = SCu8(buffer.name, buffer.name_len);
        file = working_set_contains_name(working_set, name);
    }
    return(file);
}

////////////////////////////////

#if 0
// TODO(allen): Bring the clipboard fully to the custom side.
internal void
working_set_clipboard_clear(Heap *heap, Working_Set *working){
    String_Const_u8 *str = working->clipboards;
    for (i32 i = 0; i < working->clipboard_size; i += 1, str += 1){
        heap_free(heap, str->str);
        block_zero_struct(str);
    }
    working->clipboard_size = 0;
    working->clipboard_current = 0;
}

internal String_Const_u8*
working_set_next_clipboard_string(Heap *heap, Working_Set *working, u64 str_size){
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
#endif

////////////////////////////////

// TODO(allen): get rid of this???
internal b32
get_canon_name(Arena *scratch, String_Const_u8 file_name, Editing_File_Name *canon_name){
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 canonical = system_get_canonical(scratch, file_name);
    u64 size = Min(sizeof(canon_name->name_space), canonical.size);
    block_copy(canon_name->name_space, canonical.str, size);
    canon_name->name_size = size;
    end_temp(temp);
    file_name_terminate(canon_name);
    return(canon_name->name_size > 0);
}

internal void
file_bind_file_name(Working_Set *working_set, Editing_File *file, String_Const_u8 canon_file_name){
    Assert(file->unique_name.name_size == 0);
    Assert(file->canon.name_size == 0);
    u64 size = canon_file_name.size;
    size = clamp_top(size, sizeof(file->canon.name_space) - 1);
    file->canon.name_size = size;
    block_copy(file->canon.name_space, canon_file_name.str, size);
    file_name_terminate(&file->canon);
    b32 result = working_set_canon_add(working_set, file, string_from_file_name(&file->canon));
    Assert(result);
}

internal void
buffer_unbind_file(Working_Set *working_set, Editing_File *file){
    Assert(file->unique_name.name_size == 0);
    Assert(file->canon.name_size != 0);
    working_set_canon_remove(working_set, string_from_file_name(&file->canon));
    file->canon.name_size = 0;
}

internal b32
buffer_name_has_conflict(Working_Set *working_set, String_Const_u8 base_name){
    b32 hit_conflict = false;
    Node *used_nodes = &working_set->active_file_sentinel;
    for (Node *node = used_nodes->next;
         node != used_nodes;
         node = node->next){
        Editing_File *file_ptr = CastFromMember(Editing_File, main_chain_node, node);
        if (file_ptr && string_match(base_name, string_from_file_name(&file_ptr->unique_name))){
            hit_conflict = true;
            break;
        }
    }
    return(hit_conflict);
}

internal void
buffer_resolve_name_low_level(Arena *scratch, Working_Set *working_set, Editing_File_Name *name, String_Const_u8 base_name){
    u64 size = base_name.size;
    size = clamp_top(size, sizeof(name->name_space));
    block_copy(name->name_space, base_name.str, size);
    String_u8 string = Su8(name->name_space, size, sizeof(name->name_space));
    u64 original_size = string.size;
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
buffer_bind_name_low_level(Arena *scratch, Working_Set *working_set, Editing_File *file, String_Const_u8 base_name, String_Const_u8 name){
    Assert(file->base_name.name_size == 0);
    Assert(file->unique_name.name_size == 0);
    
    Editing_File_Name new_name = {};
    buffer_resolve_name_low_level(scratch, working_set, &new_name, name);
    
    {
        u64 size = base_name.size;
        size = clamp_top(size, sizeof(file->base_name.name_space));
        block_copy(file->base_name.name_space, base_name.str, size);
        file->base_name.name_size = size;
    }
    
    {
        u64 size = new_name.name_size;
        block_copy(file->unique_name.name_space, new_name.name_space, size);
        file->unique_name.name_size = size;
    }
    
    b32 result = working_set_add_name(working_set, file, string_from_file_name(&file->unique_name));
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
buffer_bind_name(Thread_Context *tctx, Models *models, Arena *scratch, Working_Set *working_set, Editing_File *file, String_Const_u8 base_name){
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
    
    Node *used_nodes = &working_set->active_file_sentinel;
    for (Node *node = used_nodes->next;
         node != used_nodes;
         node = node->next){
        Editing_File *file_ptr = CastFromMember(Editing_File, main_chain_node, node);
        if (file_ptr != 0 && string_match(base_name, string_from_file_name(&file_ptr->base_name))){
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
            entry->buffer_id = file_ptr->id;
            
            entry->file_name = push_string_copy(scratch, string_from_file_name(&file_ptr->canon));
            entry->base_name = push_string_copy(scratch, base_name);
            
            String_Const_u8 b = base_name;
            if (i > 0){
                b = string_from_file_name(&file_ptr->unique_name);
            }
            u64 unique_name_capacity = 256;
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
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
        models->buffer_name_resolver(&app, conflicts, conflict_count);
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
            buffer_bind_name_low_level(scratch, working_set, file_ptr, base_name, unique_name);
        }
    }
    
    end_temp(temp);
}

////////////////////////////////

internal void
file_touch(Working_Set *working_set, Editing_File *file){
    Assert(file != 0);
    dll_remove(&file->touch_node);
    dll_insert(&working_set->touch_order_sentinel, &file->touch_node);
}

internal Editing_File*
file_get_next(Working_Set *working_set, Editing_File *file){
    if (file != 0){
        Node *node = file->touch_node.next;
        file = CastFromMember(Editing_File, touch_node, node);
        if (node == &working_set->touch_order_sentinel){
            file = 0;
        }
    }
    else{
        if (working_set->active_file_count > 0){
            Node *node = working_set->touch_order_sentinel.next;
            file = CastFromMember(Editing_File, touch_node, node);
        }
    }
    return(file);
}

////////////////////////////////

internal Editing_File*
imp_get_file(Models *models, Buffer_ID buffer_id){
    Working_Set *working_set = &models->working_set;
    return(working_set_get_file(working_set, buffer_id));
}

// BOTTOM



