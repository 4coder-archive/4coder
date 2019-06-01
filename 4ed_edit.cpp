/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.03.2018
 *
 * High level edit procedures
 *
 */

// TOP

internal void
edit_pre_state_change(System_Functions *system, Heap *heap, Models *models, Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            heap_free(heap, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
        file->state.still_lexing = 0;
    }
    file_add_dirty_flag(file, DirtyState_UnsavedChanges);
    file_unmark_edit_finished(&models->working_set, file);
    Layout *layout = &models->layout;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file == file){
            Full_Cursor render_cursor = view_get_render_cursor(system, view);
            Full_Cursor target_cursor = view_get_render_cursor_target(system, view);
            view->temp_view_top_left_pos        = render_cursor.pos;
            view->temp_view_top_left_target_pos = target_cursor.pos;
        }
    }
}

internal void
edit_fix_markers__write_workspace_markers(Dynamic_Workspace *workspace, Buffer_ID buffer_id,
                                          Cursor_With_Index *cursors, Cursor_With_Index *r_cursors, i32 *cursor_count, i32 *r_cursor_count){
    for (Managed_Buffer_Markers_Header *node = workspace->buffer_markers_list.first;
         node != 0;
         node = node->next){
        if (node->buffer_id != buffer_id) continue;
        Marker *markers = (Marker*)(node + 1);
        Assert(sizeof(*markers) == node->std_header.item_size);
        i32 count = node->std_header.count;
        for (i32 i = 0; i < count; i += 1){
            if (markers[i].lean_right){
                write_cursor_with_index(r_cursors, r_cursor_count, markers[i].pos);
            }
            else{
                write_cursor_with_index(cursors  , cursor_count  , markers[i].pos);
            }
        }
    }
}

internal void
edit_fix_markers__read_workspace_markers(Dynamic_Workspace *workspace, Buffer_ID buffer_id,
                                         Cursor_With_Index *cursors, Cursor_With_Index *r_cursors, i32 *cursor_count, i32 *r_cursor_count){
    for (Managed_Buffer_Markers_Header *node = workspace->buffer_markers_list.first;
         node != 0;
         node = node->next){
        if (node->buffer_id != buffer_id) continue;
        Marker *markers = (Marker*)(node + 1);
        Assert(sizeof(*markers) == node->std_header.item_size);
        i32 count = node->std_header.count;
        for (i32 i = 0; i < count; i += 1){
            if (markers[i].lean_right){
                markers[i].pos = r_cursors[(*r_cursor_count)++].pos;
            }
            else{
                markers[i].pos = cursors[(*cursor_count)++].pos;
            }
        }
    }
}

internal f32
edit_fix_markers__compute_scroll_y(i32 line_height, f32 old_y_val, f32 new_y_val_aligned){
    f32 y_offset = mod_f32(old_y_val, line_height);
    f32 y_position = new_y_val_aligned + y_offset;
    return(y_position);
}

internal i32
edit_fix_markers__compute_scroll_y(i32 line_height, i32 old_y_val, f32 new_y_val_aligned){
    return((i32)edit_fix_markers__compute_scroll_y(line_height, (f32)old_y_val, new_y_val_aligned));
}

internal void
edit_fix_markers(System_Functions *system, Models *models, Editing_File *file, Edit edit){
    Layout *layout = &models->layout;
    
    Lifetime_Object *file_lifetime_object = file->lifetime_object;
    Buffer_ID file_id = file->id.id;
    Assert(file_lifetime_object != 0);
    
    i32 cursor_max = layout_get_open_panel_count(layout)*4;
    i32 total_marker_count = 0;
    {
        total_marker_count += file_lifetime_object->workspace.total_marker_count;
        
        i32 key_count = file_lifetime_object->key_count;
        i32 key_index = 0;
        for (Lifetime_Key_Ref_Node *key_node = file_lifetime_object->key_node_first;
             key_node != 0;
             key_node = key_node->next){
            i32 count = clamp_top(lifetime_key_reference_per_node, key_count - key_index);
            for (i32 i = 0; i < count; i += 1){
                Lifetime_Key *key = key_node->keys[i];
                total_marker_count += key->dynamic_workspace.total_marker_count;
            }
            key_index += count;
        }
    }
    cursor_max += total_marker_count;
    
    Arena *scratch = &models->mem.arena;
    Temp_Memory cursor_temp = begin_temp(scratch);
    Cursor_With_Index *cursors = push_array(scratch, Cursor_With_Index, cursor_max);
    Cursor_With_Index *r_cursors = push_array(scratch, Cursor_With_Index, cursor_max);
    i32 cursor_count = 0;
    i32 r_cursor_count = 0;
    Assert(cursors != 0);
    Assert(r_cursors != 0);
    
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file == file){
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            write_cursor_with_index(cursors, &cursor_count, edit_pos.cursor_pos);
            write_cursor_with_index(cursors, &cursor_count, view->mark);
            write_cursor_with_index(cursors, &cursor_count, view->temp_view_top_left_pos);
            write_cursor_with_index(cursors, &cursor_count, view->temp_view_top_left_target_pos);
        }
    }
    
    edit_fix_markers__write_workspace_markers(&file_lifetime_object->workspace, file_id, cursors, r_cursors, &cursor_count, &r_cursor_count);
    
    {
        i32 key_count = file_lifetime_object->key_count;
        i32 key_index = 0;
        for (Lifetime_Key_Ref_Node *key_node = file_lifetime_object->key_node_first;
             key_node != 0;
             key_node = key_node->next){
            i32 count = clamp_top(lifetime_key_reference_per_node, key_count - key_index);
            for (i32 i = 0; i < count; i += 1){
                Lifetime_Key *key = key_node->keys[i];
                edit_fix_markers__write_workspace_markers(&key->dynamic_workspace, file_id, cursors, r_cursors, &cursor_count, &r_cursor_count);
            }
            key_index += count;
        }
    }
    
    if (cursor_count > 0 || r_cursor_count > 0){
        buffer_sort_cursors(  cursors,   cursor_count);
        buffer_sort_cursors(r_cursors, r_cursor_count);
        buffer_update_cursors(  cursors,   cursor_count, edit.range.first, edit.range.one_past_last, edit.length, false);
        buffer_update_cursors(r_cursors, r_cursor_count, edit.range.first, edit.range.one_past_last, edit.length, true);
        buffer_unsort_cursors(  cursors,   cursor_count);
        buffer_unsort_cursors(r_cursors, r_cursor_count);
        
        cursor_count = 0;
        r_cursor_count = 0;
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            View *view = panel->view;
            if (view->file == file){
                i32 cursor_pos = cursors[cursor_count++].pos;
                Full_Cursor new_cursor = file_compute_cursor(system, file, seek_pos(cursor_pos));
                
                File_Edit_Positions edit_pos = view_get_edit_pos(view);
                GUI_Scroll_Vars scroll = edit_pos.scroll;
                
                view->mark = cursors[cursor_count++].pos;
                i32 line_height = view->line_height;
                i32 top_left_pos = cursors[cursor_count++].pos;
                i32 top_left_target_pos = cursors[cursor_count++].pos;
                f32 new_y_val_aligned = 0;
                if (view->temp_view_top_left_pos != top_left_pos){
                    Full_Cursor new_position_cursor = file_compute_cursor(system, file, seek_pos(top_left_pos));
                    if (file->settings.unwrapped_lines){
                        new_y_val_aligned = new_position_cursor.unwrapped_y;
                    }
                    else{
                        new_y_val_aligned = new_position_cursor.wrapped_y;
                    }
                    scroll.scroll_y = edit_fix_markers__compute_scroll_y(line_height, scroll.scroll_y, new_y_val_aligned);
                }
                if (view->temp_view_top_left_target_pos != top_left_target_pos){
                    if (top_left_target_pos != top_left_pos){
                        Full_Cursor new_position_cursor = file_compute_cursor(system, file, seek_pos(top_left_target_pos));
                        if (file->settings.unwrapped_lines){
                            new_y_val_aligned = new_position_cursor.unwrapped_y;
                        }
                        else{
                            new_y_val_aligned = new_position_cursor.wrapped_y;
                        }
                    }
                    scroll.target_y = edit_fix_markers__compute_scroll_y(line_height, scroll.target_y, new_y_val_aligned);
                }
                
                view_set_cursor_and_scroll(models, view, new_cursor, true, scroll);
            }
        }
        
        edit_fix_markers__read_workspace_markers(&file_lifetime_object->workspace, file_id, cursors, r_cursors, &cursor_count, &r_cursor_count);
        
        i32 key_count = file_lifetime_object->key_count;
        i32 key_index = 0;
        for (Lifetime_Key_Ref_Node *key_node = file_lifetime_object->key_node_first;
             key_node != 0;
             key_node = key_node->next){
            i32 count = clamp_top(lifetime_key_reference_per_node, key_count - key_index);
            for (i32 i = 0; i < count; i += 1){
                Lifetime_Key *key = key_node->keys[i];
                edit_fix_markers__read_workspace_markers(&key->dynamic_workspace, file_id, cursors, r_cursors, &cursor_count, &r_cursor_count);
            }
            key_index += count;
        }
    }
    
    end_temp(cursor_temp);
}

internal void
edit_single(System_Functions *system, Models *models, Editing_File *file, Range range, String string, Edit_Behaviors behaviors){
    Edit edit = {};
    edit.str = string.str;
    edit.length = string.size;
    edit.range = range;
    
    
    Gap_Buffer *buffer = &file->state.buffer;
    Assert(0 <= edit.range.first);
    Assert(edit.range.first <= edit.range.one_past_last);
    Assert(edit.range.one_past_last <= buffer_size(buffer));
    
    Heap *heap = &models->mem.heap;
    
    // NOTE(allen): history update
    if (!behaviors.do_not_post_to_history){
        // TODO(allen): if the edit number counter is not updated, maybe auto-merge edits?  Wouldn't that just work?
        history_dump_records_after_index(&file->state.history, file->state.current_record_index);
        history_record_edit(heap, &models->global_history, &file->state.history, buffer, edit);
        file->state.current_record_index = history_get_record_count(&file->state.history);
    }
    
    // NOTE(allen): fixing stuff beforewards????
    edit_pre_state_change(system, heap, models, file);
    
    // NOTE(allen): edit range hook
    if (models->hook_file_edit_range != 0){
        models->hook_file_edit_range(&models->app_links, file->id.id, edit.range, SCu8(edit.str, edit.length));
    }
    
    // NOTE(allen): expand spec, compute shift
    i32 shift_amount = buffer_replace_range_compute_shift(edit.range.first, edit.range.one_past_last, edit.length);
    
    // NOTE(allen): actual text replacement
    i32 request_amount = 0;
    for (;buffer_replace_range(buffer, edit.range.first, edit.range.one_past_last, edit.str, edit.length, shift_amount, &request_amount);){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = heap_allocate(heap, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(buffer, new_data, request_amount);
        if (old_data != 0){
            heap_free(heap, old_data);
        }
    }
    
    // NOTE(allen): line meta data
    i32 line_start = buffer_get_line_number(buffer, edit.range.first);
    i32 line_end = buffer_get_line_number(buffer, edit.range.one_past_last);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = buffer_count_newlines(buffer, edit.range.first, edit.range.first + edit.length);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    Assert(font.valid);
    
    file_grow_starts_as_needed(heap, buffer, line_shift);
    buffer_remeasure_starts(buffer, line_start, line_end, line_shift, shift_amount);
    
    file_allocate_character_starts_as_needed(heap, file);
    buffer_remeasure_character_starts(system, font, buffer, line_start, line_end, line_shift, file->state.character_starts, 0, file->settings.virtual_white);
    
    // NOTE(allen): token fixing
    if (file->settings.tokens_exist){
        file_relex(system, models, file, edit.range.first, edit.range.one_past_last, shift_amount);
    }
    
    // NOTE(allen): wrap meta data
    file_measure_wraps(system, &models->mem, file, font);
    
    // NOTE(allen): cursor fixing
    edit_fix_markers(system, models, file, edit);
    
    // NOTE(allen): mark edit finished
    if (file->settings.tokens_exist){
        if (file->settings.virtual_white){
            file_mark_edit_finished(&models->working_set, file);
        }
    }
    else{
        file_mark_edit_finished(&models->working_set, file);
    }
}

internal b32
edit_batch(System_Functions *system, Models *models, Editing_File *file, char *str, Buffer_Edit *edits, i32 edit_count, Edit_Behaviors behaviors){
    b32 result = true;
    if (edit_count > 0){
        global_history_adjust_edit_grouping_counter(&models->global_history, 1);
        
        Buffer_Edit *edit_in = edits;
        Buffer_Edit *one_past_last = edits + edit_count;
        i32 shift = 0;
        for (;edit_in < one_past_last; edit_in += 1){
            String insert_string = make_string(str + edit_in->str_start, edit_in->len);
            Range edit_range = {edit_in->start, edit_in->end};
            edit_range.first += shift;
            edit_range.one_past_last += shift;
            i32 size = buffer_size(&file->state.buffer);
            if (0 <= edit_range.first && edit_range.first <= edit_range.one_past_last && edit_range.one_past_last <= size){
                edit_single(system, models, file, edit_range, insert_string, behaviors);
                shift += replace_range_compute_shift(edit_range, insert_string.size);
            }
            else{
                result = false;
                break;
            }
        }
        
        global_history_adjust_edit_grouping_counter(&models->global_history, -1);
    }
    return(result);
}

internal void
file_end_file(Models *models, Editing_File *file){
    if (models->hook_end_file != 0){
        models->hook_end_file(&models->app_links, file->id.id);
    }
    Heap *heap = &models->mem.heap;
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    lifetime_free_object(heap, lifetime_allocator, file->lifetime_object);
    file->lifetime_object = lifetime_alloc_object(heap, lifetime_allocator, DynamicWorkspace_Buffer, file);
}

internal void
edit__apply_record_forward(System_Functions *system, Models *models, Editing_File *file, Record *record, Edit_Behaviors behaviors_prototype){
    // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): 
    // Whenever you change this also change the backward version!
    
    switch (record->kind){
        case RecordKind_Single:
        {
            String str = make_string(record->single.str_forward, record->single.length_forward);
            Range range = {record->single.first, record->single.first + record->single.length_backward};
            edit_single(system, models, file, range, str, behaviors_prototype);
        }break;
        
        case RecordKind_Group:
        {
            Node *sentinel = &record->group.children;
            for (Node *node = sentinel->next;
                 node != sentinel;
                 node = node->next){
                Record *sub_record = CastFromMember(Record, node, node);
                edit__apply_record_forward(system, models, file, sub_record, behaviors_prototype);
            }
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
}

internal void
edit__apply_record_backward(System_Functions *system, Models *models, Editing_File *file, Record *record, Edit_Behaviors behaviors_prototype){
    // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): 
    // Whenever you change this also change the forward version!
    
    switch (record->kind){
        case RecordKind_Single:
        {
            String str = make_string(record->single.str_backward, record->single.length_backward);
            Range range = {record->single.first, record->single.first + record->single.length_forward};
            edit_single(system, models, file, range, str, behaviors_prototype);
        }break;
        
        case RecordKind_Group:
        {
            Node *sentinel = &record->group.children;
            for (Node *node = sentinel->prev;
                 node != sentinel;
                 node = node->prev){
                Record *sub_record = CastFromMember(Record, node, node);
                edit__apply_record_backward(system, models, file, sub_record, behaviors_prototype);
            }
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
}

internal void
edit_change_current_history_state(System_Functions *system, Models *models, Editing_File *file, i32 target_index){
    History *history = &file->state.history;
    if (history->activated && file->state.current_record_index != target_index){
        Assert(0 <= target_index && target_index <= history->record_count);
        
        i32 current = file->state.current_record_index;
        Record *record = history_get_record(history, current);
        Assert(record != 0);
        Record *dummy_record = history_get_dummy_record(history);
        
        Edit_Behaviors behaviors_prototype = {};
        behaviors_prototype.do_not_post_to_history = true;
        
        if (current < target_index){
            do{
                current += 1;
                record = CastFromMember(Record, node, record->node.next);
                Assert(record != dummy_record);
                edit__apply_record_forward(system, models, file, record, behaviors_prototype);
            } while (current != target_index);
        }
        else{
            do{
                Assert(record != dummy_record);
                edit__apply_record_backward(system, models, file, record, behaviors_prototype);
                current -= 1;
                record = CastFromMember(Record, node, record->node.prev);
            } while (current != target_index);
        }
        
        file->state.current_record_index = current;
    }
}

////////////////////////////////

internal Editing_File*
create_file(Models *models, String_Const_u8 file_name, Buffer_Create_Flag flags){
    Editing_File *result = 0;
    
    if (file_name.size > 0){
        System_Functions *system = models->system;
        Working_Set *working_set = &models->working_set;
        Heap *heap = &models->mem.heap;
        
        Arena *scratch = &models->mem.arena;
        Temp_Memory temp = begin_temp(scratch);
        
        Editing_File *file = 0;
        b32 do_empty_buffer = false;
        Editing_File_Name canon = {};
        b32 has_canon_name = false;
        b32 buffer_is_for_new_file = false;
        
        // NOTE(allen): Try to get the file by canon name.
        if (HasFlag(flags, BufferCreate_NeverAttachToFile) == 0){
            if (get_canon_name(system, file_name, &canon)){
                has_canon_name = true;
                file = working_set_contains_canon(working_set, string_from_file_name(&canon));
            }
            else{
                do_empty_buffer = true;
            }
        }
        
        // NOTE(allen): Try to get the file by buffer name.
        if ((flags & BufferCreate_MustAttachToFile) == 0){
            if (file == 0){
                file = working_set_contains_name(working_set, file_name);
            }
        }
        
        // NOTE(allen): If there is still no file, create a new buffer.
        if (file == 0){
            Plat_Handle handle = {};
            
            // NOTE(allen): Figure out whether this is a new file, or an existing file.
            if (!do_empty_buffer){
                if ((flags & BufferCreate_AlwaysNew) != 0){
                    do_empty_buffer = true;
                }
                else{
                    if (!system->load_handle((char*)canon.name_space, &handle)){
                        do_empty_buffer = true;
                    }
                }
            }
            
            if (do_empty_buffer){
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
                if (!HasFlag(flags, BufferCreate_NeverNew)){
                    file = working_set_alloc_always(working_set, heap, &models->lifetime_allocator);
                    if (file != 0){
                        if (has_canon_name){
                            file_bind_file_name(system, heap, working_set, file, string_from_file_name(&canon));
                        }
                        String_Const_u8 front = string_front_of_path(file_name);
                        buffer_bind_name(models, heap, scratch, working_set, file, front);
                        File_Attributes attributes = {};
                        file_create_from_string(system, models, file, SCu8(""), attributes);
                        result = file;
                    }
                }
            }
            else{
                File_Attributes attributes = system->load_attributes(handle);
                b32 in_heap_mem = false;
                char *buffer = push_array(scratch, char, (i32)attributes.size);
                
                if (buffer == 0){
                    buffer = heap_array(heap, char, (i32)attributes.size);
                    Assert(buffer != 0);
                    in_heap_mem = true;
                }
                
                if (system->load_file(handle, buffer, (i32)attributes.size)){
                    system->load_close(handle);
                    file = working_set_alloc_always(working_set, heap, &models->lifetime_allocator);
                    if (file != 0){
                        file_bind_file_name(system, heap, working_set, file, string_from_file_name(&canon));
                        String_Const_u8 front = string_front_of_path(file_name);
                        buffer_bind_name(models, heap, scratch, working_set, file, front);
                        file_create_from_string(system, models, file, SCu8(buffer, (i32)attributes.size), attributes);
                        result = file;
                    }
                }
                else{
                    system->load_close(handle);
                }
                
                if (in_heap_mem){
                    heap_free(heap, buffer);
                }
            }
        }
        else{
            result = file;
        }
        
        if (file != 0 && HasFlag(flags, BufferCreate_JustChangedFile)){
            file->state.ignore_behind_os = 1;
        }
        
        if (file != 0 && HasFlag(flags, BufferCreate_AlwaysNew)){
            i32 size = buffer_size(&file->state.buffer);
            if (size > 0){
                Edit_Behaviors behaviors = {};
                edit_single(system, models, file, make_range(0, size), make_lit_string(""), behaviors);
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
            }
        }
        
        if (file != 0 && buffer_is_for_new_file && !HasFlag(flags, BufferCreate_SuppressNewFileHook) && models->hook_new_file != 0){
            models->hook_new_file(&models->app_links, file->id.id);
        }
        
        end_temp(temp);
    }
    
    return(result);
}

// BOTTOM

