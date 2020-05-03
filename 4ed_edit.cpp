/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.03.2018
 *
 * High level edit procedures
 *
 */

// TOP

function void
pre_edit_state_change(Models *models, Editing_File *file){
    file_add_dirty_flag(file, DirtyState_UnsavedChanges);
}

function void
pre_edit_history_prep(Editing_File *file, Edit_Behaviors behaviors){
    if (!behaviors.do_not_post_to_history){
        history_dump_records_after_index(&file->state.history,
                                         file->state.current_record_index);
    }
}

function void
post_edit_call_hook(Thread_Context *tctx, Models *models, Editing_File *file,
                    Range_i64 new_range, Range_Cursor old_cursor_range){
    // NOTE(allen): edit range hook
    if (models->buffer_edit_range != 0){
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
        models->buffer_edit_range(&app, file->id, new_range, old_cursor_range);
    }
}

function void
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

function void
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

function f32
edit_fix_markers__compute_scroll_y(i32 line_height, f32 old_y_val, f32 new_y_val_aligned){
    f32 y_offset = mod_f32(old_y_val, line_height);
    f32 y_position = new_y_val_aligned + y_offset;
    return(y_position);
}

function i32
edit_fix_markers__compute_scroll_y(i32 line_height, i32 old_y_val, f32 new_y_val_aligned){
    return((i32)edit_fix_markers__compute_scroll_y(line_height, (f32)old_y_val, new_y_val_aligned));
}

function void
edit_fix_markers(Thread_Context *tctx, Models *models, Editing_File *file, Batch_Edit *batch){
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
    
    Scratch_Block scratch(tctx);
    
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
            Buffer_Cursor pos_cursor = file_compute_cursor(file, seek_line_col(edit_pos.scroll.position.line_number, 1));
            Buffer_Cursor targ_cursor = file_compute_cursor(file, seek_line_col(edit_pos.scroll.target.line_number, 1));
            write_cursor_with_index(cursors, &cursor_count, pos_cursor.pos);
            write_cursor_with_index(cursors, &cursor_count, targ_cursor.pos);
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
    
    buffer_remeasure_starts(tctx, &file->state.buffer, batch);
    
    if (cursor_count > 0 || r_cursor_count > 0){
        buffer_sort_cursors(  cursors,   cursor_count);
        buffer_sort_cursors(r_cursors, r_cursor_count);
        
        buffer_update_cursors_lean_l(  cursors,   cursor_count, batch);
        buffer_update_cursors_lean_r(r_cursors, r_cursor_count, batch);
        
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
                
                i64 scroll_pos = cursors[cursor_count++].pos;
                i64 scroll_targ = cursors[cursor_count++].pos;
                Buffer_Cursor pos_cursor = file_compute_cursor(file, seek_pos(scroll_pos));
                Buffer_Cursor targ_cursor = file_compute_cursor(file, seek_pos(scroll_targ));
                edit_pos.scroll.position.line_number = pos_cursor.line;
                edit_pos.scroll.target.line_number = targ_cursor.line;
                
                view_set_cursor_and_scroll(tctx, models, view, cursor_pos, edit_pos.scroll);
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
}

function void
file_end_file(Thread_Context *tctx, Models *models, Editing_File *file){
    if (models->end_buffer != 0){
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
        models->end_buffer(&app, file->id);
    }
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    lifetime_free_object(lifetime_allocator, file->lifetime_object);
    file->lifetime_object = lifetime_alloc_object(lifetime_allocator, DynamicWorkspace_Buffer, file);
}

function void
edit__apply(Thread_Context *tctx, Models *models, Editing_File *file, Range_i64 range, String_Const_u8 string, Edit_Behaviors behaviors){
    Edit edit = {};
    edit.text = string;
    edit.range = range;
    
    Gap_Buffer *buffer = &file->state.buffer;
    Assert(0 <= edit.range.first);
    Assert(edit.range.first <= edit.range.one_past_last);
    Assert(edit.range.one_past_last <= buffer_size(buffer));
    
    // NOTE(allen): history update
    if (!behaviors.do_not_post_to_history){
        ProfileTLBlock(tctx, &models->profile_list, "edit apply history");
        history_record_edit(&models->global_history, &file->state.history, buffer,
                            behaviors.pos_before_edit, edit);
        file->state.current_record_index =
            history_get_record_count(&file->state.history);
    }
    
    {
        ProfileTLBlock(tctx, &models->profile_list, "edit apply replace range");
        i64 shift_amount = replace_range_shift(edit.range, (i64)edit.text.size);
        buffer_replace_range(buffer, edit.range, edit.text, shift_amount);
    }
}

function void
edit_single(Thread_Context *tctx, Models *models, Editing_File *file,
            Range_i64 range, String_Const_u8 string, Edit_Behaviors behaviors){
    Range_Cursor cursor_range = {};
    cursor_range.min = file_compute_cursor(file, seek_pos(range.min));
    cursor_range.max = file_compute_cursor(file, seek_pos(range.max));
    
    pre_edit_state_change(models, file);
    pre_edit_history_prep(file, behaviors);
    
    edit__apply(tctx, models, file, range, string, behaviors);
    
    file_clear_layout_cache(file);
    
    Batch_Edit batch = {};
    batch.edit.text = string;
    batch.edit.range = range;
    
    edit_fix_markers(tctx, models, file, &batch);
    post_edit_call_hook(tctx, models, file, Ii64_size(range.first, string.size), cursor_range);
}

function void
edit__apply_record_forward(Thread_Context *tctx, Models *models, Editing_File *file, Record *record, Edit_Behaviors behaviors_prototype){
    // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen):
    // Whenever you change this also change the backward version!
    
    switch (record->kind){
        case RecordKind_Single:
        {
            String_Const_u8 str = record->single.forward_text;
            Range_i64 range = Ii64(record->single.first, record->single.first + record->single.backward_text.size);
            edit_single(tctx, models, file, range, str, behaviors_prototype);
        }break;
        
        case RecordKind_Group:
        {
            Node *sentinel = &record->group.children;
            for (Node *node = sentinel->next;
                 node != sentinel;
                 node = node->next){
                Record *sub_record = CastFromMember(Record, node, node);
                edit__apply_record_forward(tctx, models, file, sub_record, behaviors_prototype);
            }
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
}

function void
edit__apply_record_backward(Thread_Context *tctx, Models *models, Editing_File *file, Record *record, Edit_Behaviors behaviors_prototype){
    // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen): // NOTE(allen):
    // Whenever you change this also change the forward version!
    
    switch (record->kind){
        case RecordKind_Single:
        {
            String_Const_u8 str = record->single.backward_text;
            Range_i64 range = Ii64(record->single.first, record->single.first + record->single.forward_text.size);
            edit_single(tctx, models, file, range, str, behaviors_prototype);
        }break;
        
        case RecordKind_Group:
        {
            Node *sentinel = &record->group.children;
            for (Node *node = sentinel->prev;
                 node != sentinel;
                 node = node->prev){
                Record *sub_record = CastFromMember(Record, node, node);
                edit__apply_record_backward(tctx, models, file, sub_record, behaviors_prototype);
            }
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
}

function void
edit_change_current_history_state(Thread_Context *tctx, Models *models, Editing_File *file, i32 target_index){
    History *history = &file->state.history;
    if (history->activated && file->state.current_record_index != target_index){
        Assert(0 <= target_index && target_index <= history->record_count);
        
        i32 current = file->state.current_record_index;
        Record *record = history_get_record(history, current);
        Assert(record != 0);
        Record *dummy_record = history_get_dummy_record(history);
        
        Edit_Behaviors behaviors_prototype = {};
        behaviors_prototype.do_not_post_to_history = true;
        behaviors_prototype.pos_before_edit = -1;
        
        if (current < target_index){
            do{
                current += 1;
                record = CastFromMember(Record, node, record->node.next);
                Assert(record != dummy_record);
                edit__apply_record_forward(tctx, models, file, record, behaviors_prototype);
            } while (current != target_index);
        }
        else{
            do{
                Assert(record != dummy_record);
                edit__apply_record_backward(tctx, models, file, record, behaviors_prototype);
                current -= 1;
                record = CastFromMember(Record, node, record->node.prev);
            } while (current != target_index);
        }
        
        file->state.current_record_index = current;
    }
}

function b32
edit_merge_history_range(Thread_Context *tctx, Models *models, Editing_File *file, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){
    b32 result = false;
    History *history = &file->state.history;
    if (history_is_activated(history)){
        i32 max_index = history_get_record_count(history);
        first_index = clamp_bot(1, first_index);
        if (first_index <= last_index && last_index <= max_index){
            if (first_index < last_index){
                i32 current_index = file->state.current_record_index;
                if (first_index <= current_index && current_index < last_index){
                    u32 in_range_handler = (flags & bitmask_2);
                    switch (in_range_handler){
                        case RecordMergeFlag_StateInRange_MoveStateForward:
                        {
                            edit_change_current_history_state(tctx, models, file, last_index);
                            current_index = last_index;
                        }break;
                        
                        case RecordMergeFlag_StateInRange_MoveStateBackward:
                        {
                            edit_change_current_history_state(tctx, models, file, first_index);
                            current_index = first_index;
                        }break;
                        
                        case RecordMergeFlag_StateInRange_ErrorOut:
                        {
                            goto done;
                        }break;
                    }
                }
                Scratch_Block scratch(tctx);
                history_merge_records(scratch, history, first_index, last_index);
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

function b32
edit_batch_check(Thread_Context *tctx, Profile_Global_List *list, Batch_Edit *batch){
    ProfileTLScope(tctx, list, "batch check");
    b32 result = true;
    Range_i64 prev_range = Ii64(-1, 0);
    for (;batch != 0;
         batch = batch->next){
        if (batch->edit.range.first <= prev_range.first ||
            batch->edit.range.first < prev_range.one_past_last){
            result = false;
            break;
        }
    }
    return(result);
}

function b32
edit_batch(Thread_Context *tctx, Models *models, Editing_File *file,
           Batch_Edit *batch, Edit_Behaviors behaviors){
    b32 result = true;
    if (batch != 0){
        if (!edit_batch_check(tctx, &models->profile_list, batch)){
            result = false;
        }
        else{
            ProfileTLScope(tctx, &models->profile_list, "batch apply");
            
            pre_edit_state_change(models, file);
            pre_edit_history_prep(file, behaviors);
            
            History_Record_Index start_index = 0;
            if (history_is_activated(&file->state.history)){
                start_index = file->state.current_record_index;
            }
            
            ProfileTLBlockNamed(tctx, &models->profile_list, "batch text edits", profile_edits);
            
            Range_i64 old_range = {};
            old_range.min = batch->edit.range.min;
            for (Batch_Edit *edit = batch;
                 edit != 0;
                 edit = edit->next){
                if (edit->next == 0){
                    old_range.max = edit->edit.range.max;
                }
            }
            Range_Cursor cursor_range = {};
            cursor_range.min = file_compute_cursor(file, seek_pos(old_range.min));
            cursor_range.max = file_compute_cursor(file, seek_pos(old_range.max));
            
            Range_i64 new_range = Ii64_neg_inf;
            Gap_Buffer *buffer = &file->state.buffer;
            
            i32 batch_count = 0;
            i64 shift = 0;
            for (Batch_Edit *edit = batch;
                 edit != 0;
                 edit = edit->next){
                String_Const_u8 insert_string = edit->edit.text;
                
                Range_i64 edit_range = edit->edit.range;
                edit_range.first += shift;
                edit_range.one_past_last += shift;
                
                new_range.min = Min(new_range.min, edit_range.min);
                i64 new_max = (i64)(edit_range.min + insert_string.size);
                new_range.max = Max(new_range.max, new_max);
                
                i64 size = buffer_size(buffer);
                if (0 <= edit_range.first &&
                    edit_range.first <= edit_range.one_past_last &&
                    edit_range.one_past_last <= size){
                    edit__apply(tctx, models, file, edit_range, insert_string,
                                behaviors);
                    shift += replace_range_shift(edit_range, insert_string.size);
                    batch_count += 1;
                }
                else{
                    result = false;
                    break;
                }
            }
            ProfileCloseNow(profile_edits);
            
            if (history_is_activated(&file->state.history)){
                History_Record_Index last_index = file->state.current_record_index;
                if (start_index + 1 < last_index){
                    edit_merge_history_range(tctx, models, file,
                                             start_index + 1, last_index,
                                             RecordMergeFlag_StateInRange_ErrorOut);
                }
            }
            
            file_clear_layout_cache(file);
            
            edit_fix_markers(tctx, models, file, batch);
            
            post_edit_call_hook(tctx, models, file, new_range, cursor_range);
        }
    }
    
    return(result);
}

////////////////////////////////

function Editing_File*
create_file(Thread_Context *tctx, Models *models, String_Const_u8 file_name, Buffer_Create_Flag flags){
    Editing_File *result = 0;
    
    if (file_name.size > 0){
        Working_Set *working_set = &models->working_set;
        Heap *heap = &models->heap;
        
        Scratch_Block scratch(tctx);
        
        Editing_File *file = 0;
        b32 do_empty_buffer = false;
        Editing_File_Name canon = {};
        b32 has_canon_name = false;
        b32 buffer_is_for_new_file = false;
        
        // NOTE(allen): Try to get the file by canon name.
        if (HasFlag(flags, BufferCreate_NeverAttachToFile) == 0){
            if (get_canon_name(scratch, file_name, &canon)){
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
                    if (!system_load_handle(scratch, (char*)canon.name_space, &handle)){
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
                            file_bind_file_name(working_set, file, string_from_file_name(&canon));
                        }
                        String_Const_u8 front = string_front_of_path(file_name);
                        buffer_bind_name(tctx, models, scratch, working_set, file, front);
                        File_Attributes attributes = {};
                        file_create_from_string(tctx, models, file, SCu8(""), attributes);
                        result = file;
                    }
                }
            }
            else{
                File_Attributes attributes = system_load_attributes(handle);
                b32 in_heap_mem = false;
                char *buffer = push_array(scratch, char, (i32)attributes.size);
                
                if (buffer == 0){
                    buffer = heap_array(heap, char, (i32)attributes.size);
                    Assert(buffer != 0);
                    in_heap_mem = true;
                }
                
                if (system_load_file(handle, buffer, (i32)attributes.size)){
                    system_load_close(handle);
                    file = working_set_allocate_file(working_set, &models->lifetime_allocator);
                    if (file != 0){
                        file_bind_file_name(working_set, file, string_from_file_name(&canon));
                        String_Const_u8 front = string_front_of_path(file_name);
                        buffer_bind_name(tctx, models, scratch, working_set, file, front);
                        file_create_from_string(tctx, models, file, SCu8(buffer, (i32)attributes.size), attributes);
                        result = file;
                    }
                }
                else{
                    system_load_close(handle);
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
            file->state.save_state = FileSaveState_SavedWaitingForNotification;
        }
        
        if (file != 0 && HasFlag(flags, BufferCreate_AlwaysNew)){
            i64 size = buffer_size(&file->state.buffer);
            if (size > 0){
                Edit_Behaviors behaviors = {};
                edit_single(tctx, models, file, Ii64(0, size), string_u8_litexpr(""), behaviors);
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
            }
        }
        
        if (file != 0 && buffer_is_for_new_file &&
            !HasFlag(flags, BufferCreate_SuppressNewFileHook) &&
            models->new_file != 0){
            Application_Links app = {};
            app.tctx = tctx;
            app.cmd_context = models;
            models->new_file(&app, file->id);
        }
    }
    
    return(result);
}

// BOTTOM

