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
edit_pre_state_change(Models *models, Editing_File *file){
    System_Functions *system = models->system;
    file_add_dirty_flag(file, DirtyState_UnsavedChanges);
    file_unmark_edit_finished(&models->working_set, file);
}

internal void
edit_fix_markers__write_workspace_markers(Dynamic_Workspace *workspace, Buffer_ID buffer_id,
                                          Cursor_With_Index *cursors, Cursor_With_Index *r_cursors,
                                          i32 *cursor_count, i32 *r_cursor_count){
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
edit_fix_markers(Models *models, Editing_File *file, Edit edit){
    Layout *layout = &models->layout;
    
    Lifetime_Object *file_lifetime_object = file->lifetime_object;
    Buffer_ID file_id = file->id;
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
            write_cursor_with_index(cursors, &cursor_count, (i32)edit_pos.cursor_pos);
            write_cursor_with_index(cursors, &cursor_count, (i32)view->mark);
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
        buffer_update_cursors(  cursors,   cursor_count, edit.range.first, edit.range.one_past_last, edit.text.size, false);
        buffer_update_cursors(r_cursors, r_cursor_count, edit.range.first, edit.range.one_past_last, edit.text.size, true);
        buffer_unsort_cursors(  cursors,   cursor_count);
        buffer_unsort_cursors(r_cursors, r_cursor_count);
        
        Face *face = file_get_face(models, file);
        
        cursor_count = 0;
        r_cursor_count = 0;
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            View *view = panel->view;
            if (view->file == file){
                i64 cursor_pos = cursors[cursor_count++].pos;
                view->mark = cursors[cursor_count++].pos;
                File_Edit_Positions edit_pos = view_get_edit_pos(view);
                view_set_cursor_and_scroll(models, view, cursor_pos, true, edit_pos.scroll);
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
edit_single(Models *models, Editing_File *file, Interval_i64 range, String_Const_u8 string, Edit_Behaviors behaviors){
    Edit edit = {};
    edit.text = string;
    edit.range = range;
    
    Gap_Buffer *buffer = &file->state.buffer;
    Assert(0 <= edit.range.first);
    Assert(edit.range.first <= edit.range.one_past_last);
    Assert(edit.range.one_past_last <= buffer_size(buffer));
    
    Arena *scratch = &models->mem.arena;
    
    // NOTE(allen): history update
    if (!behaviors.do_not_post_to_history){
        // TODO(allen): if the edit number counter is not updated, maybe auto-merge edits?  Wouldn't that just work?
        history_dump_records_after_index(&file->state.history, file->state.current_record_index);
        history_record_edit(&models->global_history, &file->state.history, buffer, edit);
        file->state.current_record_index = history_get_record_count(&file->state.history);
    }
    
    // NOTE(allen): fixing stuff beforewards????
    edit_pre_state_change(models, file);
    
    // NOTE(allen): edit range hook
    if (models->hook_file_edit_range != 0){
        models->hook_file_edit_range(&models->app_links, file->id, edit.range, edit.text);
    }
    
    // NOTE(allen): expand spec, compute shift
    i64 shift_amount = replace_range_shift(edit.range, (i64)edit.text.size);
    
    // NOTE(allen): actual text replacement
    buffer_replace_range(buffer, edit.range, edit.text, shift_amount);
    
    // NOTE(allen): line meta data
    i64 line_start = buffer_get_line_index(buffer, edit.range.first);
    i64 line_end = buffer_get_line_index(buffer, edit.range.one_past_last);
    i64 replaced_line_count = line_end - line_start;
    i64 new_line_count = buffer_count_newlines(scratch, buffer, edit.range.first, edit.range.first + edit.text.size);
    i64 line_shift =  new_line_count - replaced_line_count;
    
    file_clear_layout_cache(file);
    buffer_remeasure_starts(scratch, buffer, Ii64(line_start, line_end + 1), line_shift, shift_amount);
    
    // NOTE(allen): cursor fixing
    edit_fix_markers(models, file, edit);
    
    // NOTE(allen): mark edit finished
    file_mark_edit_finished(&models->working_set, file);
}

internal void
file_end_file(Models *models, Editing_File *file){
    if (models->hook_end_file != 0){
        models->hook_end_file(&models->app_links, file->id);
    }
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    lifetime_free_object(lifetime_allocator, file->lifetime_object);
    file->lifetime_object = lifetime_alloc_object(lifetime_allocator, DynamicWorkspace_Buffer, file);
}

internal void
edit__apply_record_forward(System_Functions *system, Models *models, Editing_File *file, Record *record, Edit_Behaviors behaviors_prototype){
    // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): 
    // Whenever you change this also change the backward version!
    
    switch (record->kind){
        case RecordKind_Single:
        {
            String_Const_u8 str = record->single.forward_text;
            Interval_i64 range = Ii64(record->single.first, record->single.first + record->single.backward_text.size);
            edit_single(models, file, range, str, behaviors_prototype);
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
            String_Const_u8 str = record->single.backward_text;
            Interval_i64 range = Ii64(record->single.first, record->single.first + record->single.forward_text.size);
            edit_single(models, file, range, str, behaviors_prototype);
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

internal b32
edit_merge_history_range(Models *models, Editing_File *file, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){
    b32 result = false;
    History *history = &file->state.history;
    if (history_is_activated(history)){
        i32 max_index = history_get_record_count(history);
        first_index = clamp_bot(1, first_index);
        if (first_index <= last_index && last_index <= max_index){
            if (first_index < last_index){
                i32 current_index = file->state.current_record_index;
                if (first_index <= current_index && current_index < last_index){
                    System_Functions *system = models->system;
                    u32 in_range_handler = (flags & bitmask_2);
                    switch (in_range_handler){
                        case RecordMergeFlag_StateInRange_MoveStateForward:
                        {
                            edit_change_current_history_state(system, models, file, last_index);
                            current_index = last_index;
                        }break;
                        
                        case RecordMergeFlag_StateInRange_MoveStateBackward:
                        {
                            edit_change_current_history_state(system, models, file, first_index);
                            current_index = first_index;
                        }break;
                        
                        case RecordMergeFlag_StateInRange_ErrorOut:
                        {
                            goto done;
                        }break;
                    }
                }
                history_merge_records(&models->mem.arena, history, first_index, last_index);
                if (current_index >= last_index){
                    current_index -= (last_index - first_index);
                }
                file->state.current_record_index = current_index;
            }
            result = true;
        }
    }
    done:;
    return(result);
}

internal b32
edit_batch(Models *models, Editing_File *file, Batch_Edit *batch, Edit_Behaviors behaviors){
    b32 result = true;
    if (batch != 0){
        History_Record_Index start_index = 0;
        if (history_is_activated(&file->state.history)){
            start_index = file->state.current_record_index;
        }
        
        i64 shift = 0;
        for (Batch_Edit *edit = batch;
             edit != 0;
             edit = edit->next){
            String_Const_u8 insert_string = edit->edit.text;
            Interval_i64 edit_range = edit->edit.range;
            edit_range.first += shift;
            edit_range.one_past_last += shift;
            i64 size = buffer_size(&file->state.buffer);
            if (0 <= edit_range.first &&
                edit_range.first <= edit_range.one_past_last &&
                edit_range.one_past_last <= size){
                edit_single(models, file, edit_range, insert_string, behaviors);
                shift += replace_range_shift(edit_range, insert_string.size);
            }
            else{
                result = false;
                break;
            }
        }
        
        if (history_is_activated(&file->state.history)){
            History_Record_Index last_index = file->state.current_record_index;
            if (start_index + 1 < last_index){
                edit_merge_history_range(models, file, start_index + 1, last_index, RecordMergeFlag_StateInRange_ErrorOut);
            }
        }
    }
    return(result);
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
            if (get_canon_name(system, scratch, file_name, &canon)){
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
                    if (!system->load_handle(scratch, (char*)canon.name_space, &handle)){
                        do_empty_buffer = true;
                    }
                }
            }
            
            if (do_empty_buffer){
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
                if (!HasFlag(flags, BufferCreate_NeverNew)){
                    file = working_set_allocate_file(working_set, &models->lifetime_allocator);
                    if (file != 0){
                        if (has_canon_name){
                            file_bind_file_name(system, working_set, file, string_from_file_name(&canon));
                        }
                        String_Const_u8 front = string_front_of_path(file_name);
                        buffer_bind_name(models, scratch, working_set, file, front);
                        File_Attributes attributes = {};
                        file_create_from_string(models, file, SCu8(""), attributes);
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
                    file = working_set_allocate_file(working_set, &models->lifetime_allocator);
                    if (file != 0){
                        file_bind_file_name(system, working_set, file, string_from_file_name(&canon));
                        String_Const_u8 front = string_front_of_path(file_name);
                        buffer_bind_name(models, scratch, working_set, file, front);
                        file_create_from_string(models, file, SCu8(buffer, (i32)attributes.size), attributes);
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
            i64 size = buffer_size(&file->state.buffer);
            if (size > 0){
                Edit_Behaviors behaviors = {};
                edit_single(models, file, Ii64(0, size), string_u8_litexpr(""), behaviors);
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
            }
        }
        
        if (file != 0 && buffer_is_for_new_file &&
            !HasFlag(flags, BufferCreate_SuppressNewFileHook) &&
            models->hook_new_file != 0){
            models->hook_new_file(&models->app_links, file->id);
        }
        
        end_temp(temp);
    }
    
    return(result);
}

// BOTTOM

