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
    if (file->state.dirty == DirtyState_UpToDate){
        file_set_dirty_flag(file, DirtyState_UnsavedChanges);
    }
    file_unmark_edit_finished(file);
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

internal void
edit_fix_markers(System_Functions *system, Models *models, Editing_File *file, Editing_Layout *layout, Cursor_Fix_Descriptor desc){
    Partition *part = &models->mem.part;
    
    Temp_Memory cursor_temp = begin_temp_memory(part);
    
    Lifetime_Object *file_lifetime_object = file->lifetime_object;
    Buffer_ID file_id = file->id.id;
    Assert(file_lifetime_object != 0);
    
    i32 cursor_max = layout->panel_max_count * 3;
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
    Cursor_With_Index *cursors = push_array(part, Cursor_With_Index, cursor_max);
    Cursor_With_Index *r_cursors = push_array(part, Cursor_With_Index, cursor_max);
    i32 cursor_count = 0;
    i32 r_cursor_count = 0;
    Assert(cursors != 0);
    Assert(r_cursors != 0);
    
    for (Panel *panel = layout->used_sentinel.next;
         panel != &layout->used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file == file){
            write_cursor_with_index(cursors, &cursor_count, view->transient.edit_pos.cursor.pos);
            write_cursor_with_index(cursors, &cursor_count, view->transient.edit_pos.mark);
            write_cursor_with_index(cursors, &cursor_count, view->transient.edit_pos.scroll_i);
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
        buffer_sort_cursors(cursors, cursor_count);
        if (desc.is_batch){
            buffer_batch_edit_update_cursors(cursors, cursor_count, desc.batch, desc.batch_size, false);
            buffer_batch_edit_update_cursors(r_cursors, r_cursor_count, desc.batch, desc.batch_size, true);
        }
        else{
            buffer_update_cursors(cursors, cursor_count, desc.start, desc.end, desc.shift_amount + (desc.end - desc.start), false);
            buffer_update_cursors(r_cursors, r_cursor_count, desc.start, desc.end, desc.shift_amount + (desc.end - desc.start), true);
        }
        buffer_unsort_cursors(cursors, cursor_count);
        
        cursor_count = 0;
        r_cursor_count = 0;
        for (Panel *panel = layout->used_sentinel.next;
             panel != &layout->used_sentinel;
             panel = panel->next){
            View *view = panel->view;
            if (view->transient.file_data.file == file){
                i32 cursor_pos = cursors[cursor_count++].pos;
                Full_Cursor new_cursor = file_compute_cursor(system, file, seek_pos(cursor_pos), 0);
                
                GUI_Scroll_Vars scroll = view->transient.edit_pos.scroll;
                
                view->transient.edit_pos.mark = cursors[cursor_count++].pos;
                i32 new_scroll_i = cursors[cursor_count++].pos;
                if (view->transient.edit_pos.scroll_i != new_scroll_i){
                    view->transient.edit_pos.scroll_i = new_scroll_i;
                    
                    Full_Cursor temp_cursor = file_compute_cursor(system, file, seek_pos(view->transient.edit_pos.scroll_i), 0);
                    
                    f32 y_offset = MOD(view->transient.edit_pos.scroll.scroll_y, view->transient.line_height);
                    f32 y_position = temp_cursor.wrapped_y;
                    if (file->settings.unwrapped_lines){
                        y_position = temp_cursor.unwrapped_y;
                    }
                    y_position += y_offset;
                    
                    scroll.target_y += round32(y_position - scroll.scroll_y);
                    scroll.scroll_y = y_position;
                }
                
                view_set_cursor_and_scroll(view, new_cursor, 1, view->transient.file_data.file->settings.unwrapped_lines, scroll);
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
    
    end_temp_memory(cursor_temp);
}

internal void
edit_single__inner(System_Functions *system, Models *models, Editing_File *file, Edit_Spec spec, History_Mode history_mode){
    Mem_Options *mem = &models->mem;
    Editing_Layout *layout = &models->layout;
    Heap *heap = &mem->heap;
    Partition *part = &mem->part;
    
    // NOTE(allen): fixing stuff beforewards????
    file_update_history_before_edit(mem, file, spec.step, spec.str, history_mode);
    edit_pre_state_change(system, &mem->heap, models, file);
    
    // NOTE(allen): expand spec, compute shift
    char *str = (char*)spec.str;
    i32 start = spec.step.edit.start;
    i32 end = spec.step.edit.end;
    i32 str_len = spec.step.edit.len;
    i32 shift_amount = buffer_replace_range_compute_shift(start, end, str_len);
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {};
    desc.start = start;
    desc.end = end;
    desc.shift_amount = shift_amount;
    edit_fix_markers(system, models, file, layout, desc);
    
    // NOTE(allen): actual text replacement
    i32 scratch_size = partition_remaining(part);
    Assert(scratch_size > 0);
    i32 request_amount = 0;
    Assert(end <= buffer_size(&file->state.buffer));
    for (;buffer_replace_range(&file->state.buffer, start, end, str, str_len, shift_amount, part->base + part->pos, scratch_size, &request_amount);){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = heap_allocate(heap, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data){
            heap_free(heap, old_data);
        }
    }
    
    // NOTE(allen): line meta data
    Gap_Buffer *buffer = &file->state.buffer;
    i32 line_start = buffer_get_line_number(&file->state.buffer, start);
    i32 line_end = buffer_get_line_number(&file->state.buffer, end);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = buffer_count_newlines(&file->state.buffer, start, start+str_len);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    Assert(font.valid);
    
    file_grow_starts_as_needed(heap, buffer, line_shift);
    buffer_remeasure_starts(buffer, line_start, line_end, line_shift, shift_amount);
    
    file_allocate_character_starts_as_needed(heap, file);
    buffer_remeasure_character_starts(system, font, buffer, line_start, line_end, line_shift, file->state.character_starts, 0, file->settings.virtual_white);
    
    // NOTE(allen): token fixing
    if (file->settings.tokens_exist){
        file_relex(system, models, file, start, end, shift_amount);
    }
    
    // NOTE(allen): mark edit finished
    if (file->settings.tokens_exist){
        if (file->settings.virtual_white){
            file_mark_edit_finished(&models->working_set, file);
        }
    }
    else{
        file_mark_edit_finished(&models->working_set, file);
    }
    
    // NOTE(allen): wrap meta data
    file_measure_wraps(system, &models->mem, file, font);
}

internal void
edit_single(System_Functions *system, Models *models, Editing_File *file,
            i32 start, i32 end, char *str, i32 len){
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    spec.step.edit.len = len;
    spec.str = (u8*)str;
    edit_single__inner(system, models, file, spec, hist_normal);
}

internal Edit_Spec
edit_compute_batch_spec(Heap *heap,
                        Editing_File *file,
                        Buffer_Edit *edits, char *str_base, i32 str_size,
                        Buffer_Edit *inverse_array, char *inv_str, i32 inv_max, i32 edit_count, i32 batch_type){
    
    i32 inv_str_pos = 0;
    Buffer_Invert_Batch state = {};
    if (buffer_invert_batch(&state, &file->state.buffer, edits, edit_count,
                            inverse_array, inv_str, &inv_str_pos, inv_max)){
        InvalidCodePath;
    }
    
    i32 first_child = undo_children_push(heap, &file->state.undo.children, edits, edit_count, (u8*)(str_base), str_size);
    i32 inverse_first_child = undo_children_push(heap, &file->state.undo.children, inverse_array, edit_count, (u8*)(inv_str), inv_str_pos);
    
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.first_child = first_child;
    spec.step.inverse_first_child = inverse_first_child;
    spec.step.special_type = batch_type;
    spec.step.child_count = edit_count;
    spec.step.inverse_child_count = edit_count;
    return(spec);
}

internal void
edit_batch(System_Functions *system, Models *models, Editing_File *file, Edit_Spec spec, History_Mode history_mode, Buffer_Batch_Edit_Type batch_type){
    Mem_Options *mem = &models->mem;
    Heap *heap = &mem->heap;
    Partition *part = &mem->part;
    Editing_Layout *layout = &models->layout;
    
    // NOTE(allen): fixing stuff "beforewards"???
    Assert(spec.str == 0);
    file_update_history_before_edit(mem, file, spec.step, 0, history_mode);
    edit_pre_state_change(system, &mem->heap, models, file);
    
    // NOTE(allen): expand spec
    u8 *str_base = file->state.undo.children.strings;
    i32 batch_size = spec.step.child_count;
    Buffer_Edit *batch = file->state.undo.children.edits + spec.step.first_child;
    Assert(spec.step.first_child < file->state.undo.children.edit_count);
    Assert(batch_size >= 0);
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {};
    desc.is_batch = true;
    desc.batch = batch;
    desc.batch_size = batch_size;
    edit_fix_markers(system, models, file, layout, desc);
    
    // NOTE(allen): actual text replacement
    i32 scratch_size = partition_remaining(part);
    Buffer_Batch_State state = {};
    i32 request_amount = 0;
    for (;buffer_batch_edit_step(&state, &file->state.buffer, batch, (char*)str_base, batch_size, part->base + part->pos, scratch_size, &request_amount);){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = heap_allocate(heap, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data){
            heap_free(heap, old_data);
        }
    }
    i32 shift_total = state.shift_total;
    
    // NOTE(allen): line meta data
    // TODO(allen): Let's try to switch to remeasuring here moron!
    file_measure_starts(heap, &file->state.buffer);
    
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    Assert(font.valid);
    
    file_allocate_character_starts_as_needed(heap, file);
    buffer_measure_character_starts(system, font, &file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
    
    // NOTE(allen): token fixing
    switch (batch_type){
        case BatchEdit_Normal:
        {
            if (file->settings.tokens_exist){
                // TODO(allen): Write a smart fast one here someday.
                Buffer_Edit *first_edit = batch;
                Buffer_Edit *last_edit = batch + batch_size - 1;
                file_relex(system, models, file, first_edit->start, last_edit->end, shift_total);
            }
            else{
                file_mark_edit_finished(&models->working_set, file);
            }
        }break;
        
        case BatchEdit_PreserveTokens:
        {
            if (file->state.tokens_complete){
                Cpp_Token_Array tokens = file->state.token_array;
                Cpp_Token *token = tokens.tokens;
                Cpp_Token *end_token = tokens.tokens + tokens.count;
                Cpp_Token original = {};
                
                Buffer_Edit *edit = batch;
                Buffer_Edit *end_edit = batch + batch_size;
                
                i32 shift_amount = 0;
                i32 local_shift = 0;
                
                for (;token < end_token; ++token){
                    original = *token;
                    for (; edit < end_edit && edit->start <= original.start; ++edit){
                        local_shift = (edit->len - (edit->end - edit->start));
                        shift_amount += local_shift;
                    }
                    token->start += shift_amount;
                    local_shift = 0;
                    for (; edit < end_edit && edit->start < original.start + original.size; ++edit){
                        local_shift += (edit->len - (edit->end - edit->start));
                    }
                    token->size += local_shift;
                    shift_amount += local_shift;
                }
                file_mark_edit_finished(&models->working_set, file);
            }
        }break;
    }
    
    // NOTE(allen): mark edit finished
    if (file->settings.tokens_exist){
        if (file->settings.virtual_white){
            file_mark_edit_finished(&models->working_set, file);
        }
    }
    else{
        file_mark_edit_finished(&models->working_set, file);
    }
    
    // NOTE(allen): wrap meta data
    file_measure_wraps(system, &models->mem, file, font);
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
edit_clear(System_Functions *system, Models *models, Editing_File *file){
    file_end_file(models, file);
    
    b32 no_views_see_file = true;
    
    Editing_Layout *layout = &models->layout;
    for (Panel *panel = layout->used_sentinel.next;
         panel != &layout->used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file == file){
            Full_Cursor cursor = {};
            cursor.line = 1;
            cursor.character = 1;
            cursor.wrap_line = 1;
            view_set_cursor(view, cursor, true, file->settings.unwrapped_lines);
            no_views_see_file = false;
        }
    }
    
    if (no_views_see_file){
        block_zero(file->state.edit_pos_stack, sizeof(file->state.edit_pos_stack));
        file->state.edit_pos_stack_top = -1;
    }
    
    edit_single(system, models, file, 0, buffer_size(&file->state.buffer), 0, 0);
}

internal void
edit_historical(System_Functions *system, Models *models, Editing_File *file, View *view, Edit_Stack *stack,
                Edit_Step step, History_Mode history_mode){
    Edit_Spec spec = {};
    spec.step = step;
    
    if (step.child_count == 0){
        spec.step.edit.str_start = 0;
        spec.str = stack->strings + step.edit.str_start;
        
        edit_single__inner(system, models, file, spec, history_mode);
        
        if (view != 0){
            view_cursor_move(system, view, step.edit.start + step.edit.len);
            view->transient.edit_pos.mark = view->transient.edit_pos.cursor.pos;
            
            Style *style = &models->styles.styles[0];
            view_post_paste_effect(view, 0.333f, step.edit.start, step.edit.len, style->theme.colors[Stag_Undo]);
        }
    }
    else{
        edit_batch(system, models, view->transient.file_data.file, spec, hist_normal, spec.step.special_type);
    }
}

// BOTTOM

