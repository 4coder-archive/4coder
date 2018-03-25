/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Viewing
 *
 */

// TOP

// TODO(allen): Switch over to using an i32 for these.
inline f32
view_width(View *view){
    i32_Rect file_rect = view->transient.file_region;
    f32 result = (f32)(file_rect.x1 - file_rect.x0);
    return (result);
}

inline f32
view_height(View *view){
    i32_Rect file_rect = view->transient.file_region;
    f32 result = (f32)(file_rect.y1 - file_rect.y0);
    return (result);
}

inline Vec2
view_get_cursor_xy(View *view){
    Full_Cursor *cursor = 0;
    if (view->transient.file_data.show_temp_highlight){
        cursor = &view->transient.file_data.temp_highlight;
    }
    else if (view->transient.edit_pos){
        cursor = &view->transient.edit_pos->cursor;
    }
    Assert(cursor != 0);
    Vec2 result;
    result.x = cursor->wrapped_x;
    result.y = cursor->wrapped_y;
    if (view->transient.file_data.file->settings.unwrapped_lines){
        result.x = cursor->unwrapped_x;
        result.y = cursor->unwrapped_y;
    }
    return(result);
}

inline Cursor_Limits
view_cursor_limits(View *view){
    Cursor_Limits limits = {0};
    
    f32 line_height = (f32)view->transient.line_height;
    f32 visible_height = view_height(view);
    
    limits.max = visible_height - line_height*3.f;
    limits.min = line_height * 2;
    
    if (limits.max - limits.min <= line_height){
        if (visible_height >= line_height){
            limits.max = visible_height - line_height;
            limits.min = -line_height;
        }
        else{
            limits.max = visible_height;
            limits.min = -line_height;
        }
    }
    
    limits.max = (limits.max > 0)?(limits.max):(0);
    limits.min = (limits.min > 0)?(limits.min):(0);
    
    limits.delta = clamp_top(line_height*3.f, (limits.max - limits.min)*.5f);
    
    return(limits);
}

inline i32
view_compute_max_target_y(View *view){
    i32 line_height = view->transient.line_height;
    Editing_File *file = view->transient.file_data.file;
    Gap_Buffer *buffer = &file->state.buffer;
    i32 lowest_line = buffer->line_count;
    if (!file->settings.unwrapped_lines){
        lowest_line = file->state.wrap_line_index[buffer->line_count];
    }
    f32 height = clamp_bottom((f32)line_height, view_height(view));
    f32 max_target_y = clamp_bottom(0.f, ((lowest_line + 0.5f)*line_height) - height*0.5f);
    return(ceil32(max_target_y));
}

inline u32
view_lock_flags(View *view){
    u32 result = AccessOpen;
    File_Viewing_Data *data = &view->transient.file_data;
    if (view->transient.showing_ui != VUI_None){
        result |= AccessHidden;
    }
    if (data->file_locked ||
        (data->file && data->file->settings.read_only)){
        result |= AccessProtected;
    }
    return(result);
}

////////////////////////////////

internal b32
view_move_view_to_cursor(View *view, GUI_Scroll_Vars *scroll, b32 center_view){
    b32 result = 0;
    f32 max_x = view_width(view);
    i32 max_y = view_compute_max_target_y(view);
    
    Vec2 cursor = view_get_cursor_xy(view);
    
    GUI_Scroll_Vars scroll_vars = *scroll;
    i32 target_x = scroll_vars.target_x;
    i32 target_y = scroll_vars.target_y;
    
    Cursor_Limits limits = view_cursor_limits(view);
    
    if (cursor.y > target_y + limits.max){
        if (center_view){
            target_y = round32(cursor.y - limits.max*.5f);
        }
        else{
            target_y = ceil32(cursor.y - limits.max + limits.delta);
        }
    }
    if (cursor.y < target_y + limits.min){
        if (center_view){
            target_y = round32(cursor.y - limits.max*.5f);
        }
        else{
            target_y = floor32(cursor.y - limits.delta - limits.min);
        }
    }
    
    target_y = clamp(0, target_y, max_y);
    
    if (cursor.x >= target_x + max_x){
        target_x = ceil32(cursor.x - max_x/2);
    }
    else if (cursor.x < target_x){
        target_x = floor32(Max(0, cursor.x - max_x/2));
    }
    
    if (target_x != scroll_vars.target_x || target_y != scroll_vars.target_y){
        scroll->target_x = target_x;
        scroll->target_y = target_y;
        result = 1;
    }
    
    return(result);
}

internal b32
view_move_cursor_to_view(System_Functions *system, View *view, GUI_Scroll_Vars scroll, Full_Cursor *cursor, f32 preferred_x){
    b32 result = false;
    
    if (view->transient.edit_pos){
        i32 line_height = view->transient.line_height;
        f32 old_cursor_y = cursor->wrapped_y;
        Editing_File *file = view->transient.file_data.file;
        if (file->settings.unwrapped_lines){
            old_cursor_y = cursor->unwrapped_y;
        }
        f32 cursor_y = old_cursor_y;
        f32 target_y = scroll.target_y + view->transient.widget_height;
        
        Cursor_Limits limits = view_cursor_limits(view);
        
        if (cursor_y > target_y + limits.max){
            cursor_y = target_y + limits.max;
        }
        if (target_y != 0 && cursor_y < target_y + limits.min){
            cursor_y = target_y + limits.min;
        }
        
        if (cursor_y != old_cursor_y){
            if (cursor_y > old_cursor_y){
                cursor_y += line_height;
            }
            else{
                cursor_y -= line_height;
            }
            
            Buffer_Seek seek = seek_xy(preferred_x, cursor_y, false, file->settings.unwrapped_lines);
            *cursor = file_compute_cursor(system, file, seek, false);
            
            result = true;
        }
    }
    
    return(result);
}

internal void
view_set_cursor(View *view, Full_Cursor cursor, b32 set_preferred_x, b32 unwrapped_lines){
    if (edit_pos_move_to_front(view->transient.file_data.file, view->transient.edit_pos)){
        edit_pos_set_cursor(view->transient.edit_pos, cursor, set_preferred_x, unwrapped_lines);
        GUI_Scroll_Vars scroll = view->transient.edit_pos->scroll;
        if (view_move_view_to_cursor(view, &scroll, 0)){
            view->transient.edit_pos->scroll = scroll;
        }
    }
}

internal void
view_set_scroll(System_Functions *system, View *view, GUI_Scroll_Vars scroll){
    if (edit_pos_move_to_front(view->transient.file_data.file, view->transient.edit_pos)){
        edit_pos_set_scroll(view->transient.edit_pos, scroll);
        Full_Cursor cursor = view->transient.edit_pos->cursor;
        if (view_move_cursor_to_view(system, view, view->transient.edit_pos->scroll, &cursor, view->transient.edit_pos->preferred_x)){
            view->transient.edit_pos->cursor = cursor;
        }
    }
}

internal void
view_set_cursor_and_scroll(View *view, Full_Cursor cursor, b32 set_preferred_x, b32 unwrapped_lines, GUI_Scroll_Vars scroll){
    File_Edit_Positions *edit_pos = view->transient.edit_pos;
    if (edit_pos_move_to_front(view->transient.file_data.file, edit_pos)){
        edit_pos_set_cursor(edit_pos, cursor, set_preferred_x, unwrapped_lines);
        edit_pos_set_scroll(edit_pos, scroll);
        edit_pos->last_set_type = EditPos_None;
    }
}

inline void
view_set_temp_highlight(System_Functions *system, View *view, i32 pos, i32 end_pos){
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    view->transient.file_data.temp_highlight = file_compute_cursor(system, file, seek_pos(pos), 0);
    view->transient.file_data.temp_highlight_end_pos = end_pos;
    view->transient.file_data.show_temp_highlight = true;
    view_set_cursor(view, view->transient.file_data.temp_highlight, 0, file->settings.unwrapped_lines);
}

inline void
view_cursor_move(System_Functions *system, View *view, i32 pos){
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    Full_Cursor cursor = file_compute_cursor(system, file, seek_pos(pos), 0);
    view_set_cursor(view, cursor, true, file->settings.unwrapped_lines);
    view->transient.file_data.show_temp_highlight = false;
}

inline void
view_post_paste_effect(View *view, f32 seconds, i32 start, i32 size, u32 color){
    Editing_File *file = view->transient.file_data.file;
    file->state.paste_effect.start = start;
    file->state.paste_effect.end = start + size;
    file->state.paste_effect.color = color;
    file->state.paste_effect.seconds_down = seconds;
    file->state.paste_effect.seconds_max = seconds;
}

////////////////////////////////

internal b32
file_is_viewed(Editing_Layout *layout, Editing_File *file){
    b32 is_viewed = false;
    for (Panel *panel = layout->used_sentinel.next;
         panel != &layout->used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file == file){
            is_viewed = true;
            break;
        }
    }
    return(is_viewed);
}

internal void
adjust_views_looking_at_file_to_new_cursor(System_Functions *system, Models *models, Editing_File *file){
    Editing_Layout *layout = &models->layout;
    
    for (Panel *panel = layout->used_sentinel.next;
         panel != &layout->used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file == file){
            if (!view->transient.file_data.show_temp_highlight){
                Assert(view->transient.edit_pos != 0);
                i32 pos = view->transient.edit_pos->cursor.pos;
                Full_Cursor cursor = file_compute_cursor(system, file, seek_pos(pos), 0);
                view_set_cursor(view, cursor, 1, file->settings.unwrapped_lines);
            }
            else{
                i32 pos = view->transient.file_data.temp_highlight.pos;
                i32 end = view->transient.file_data.temp_highlight_end_pos;
                view_set_temp_highlight(system, view, pos, end);
            }
        }
    }
}

internal void
file_full_remeasure(System_Functions *system, Models *models, Editing_File *file){
    Face_ID font_id = file->settings.font_id;
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    file_measure_wraps(system, &models->mem, file, font);
    adjust_views_looking_at_file_to_new_cursor(system, models, file);
    
    Editing_Layout *layout = &models->layout;
    
    for (Panel *panel = layout->used_sentinel.next;
         panel != &layout->used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file == file){
            view->transient.line_height = font.metrics->height;
        }
    }
}

internal void
file_set_font(System_Functions *system, Models *models, Editing_File *file, Face_ID font_id){
    file->settings.font_id = font_id;
    file_full_remeasure(system, models, file);
}

internal void
global_set_font_and_update_files(System_Functions *system, Models *models, Face_ID font_id){
    for (File_Node *node = models->working_set.used_sentinel.next;
         node != &models->working_set.used_sentinel;
         node = node->next){
        Editing_File *file = (Editing_File*)node;
        file_set_font(system, models, file, font_id);
    }
    models->global_font_id = font_id;
}

internal b32
alter_font_and_update_files(System_Functions *system, Models *models, Face_ID font_id, Font_Settings *new_settings){
    b32 success = false;
    if (system->font.face_change_settings(font_id, new_settings)){
        success = true;
        for (File_Node *node = models->working_set.used_sentinel.next;
             node != &models->working_set.used_sentinel;
             node = node->next){
            Editing_File *file = (Editing_File*)node;
            if (file->settings.font_id == font_id){
                file_full_remeasure(system, models, file);
            }
        }
    }
    return(success);
}

internal b32
release_font_and_update_files(System_Functions *system, Models *models, Face_ID font_id, Face_ID replacement_id){
    b32 success = false;
    if (system->font.face_release(font_id)){
        Font_Pointers font = system->font.get_pointers_by_id(replacement_id);
        if (!font.valid){
            Face_ID largest_id = system->font.get_largest_id();
            for (replacement_id = 1; replacement_id <= largest_id && replacement_id > 0; ++replacement_id){
                font = system->font.get_pointers_by_id(replacement_id);
                if (font.valid){
                    break;
                }
            }
            Assert(replacement_id <= largest_id && replacement_id > 0);
        }
        success = true;
        for (File_Node *node = models->working_set.used_sentinel.next;
             node != &models->working_set.used_sentinel;
             node = node->next){
            Editing_File *file = (Editing_File*)node;
            if (file->settings.font_id == font_id){
                file_set_font(system, models, file, replacement_id);
            }
        }
    }
    return(success);
}

////////////////////////////////

inline void
view_show_file(View *view){
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    view->transient.map = file->settings.base_map_id;
    if (view->transient.showing_ui != VUI_None){
        view->transient.showing_ui = VUI_None;
        view->transient.changed_context_in_step = 1;
    }
}

inline void
view_show_interactive(System_Functions *system, View *view, Models *models, Interactive_Action action, Interactive_Interaction interaction, String query){
    view->transient.showing_ui = VUI_Interactive;
    view->transient.action = action;
    view->transient.interaction = interaction;
    view->transient.dest = make_fixed_width_string(view->transient.dest_);
    view->transient.list_i = 0;
    
    view->transient.map = mapid_ui;
    
    hot_directory_clean_end(&models->hot_directory);
    hot_directory_reload(system, &models->hot_directory);
    view->transient.changed_context_in_step = true;
}

internal void
view_set_file(System_Functions *system, Models *models, View *view, Editing_File *file){
    Assert(file != 0);
    
    if (view->transient.file_data.file != 0){
        touch_file(&models->working_set, view->transient.file_data.file);
    }
    
    File_Edit_Positions *edit_pos = view->transient.edit_pos;
    
    if (edit_pos != 0){
        edit_pos_unset(view->transient.file_data.file, edit_pos);
        edit_pos = 0;
    }
    
    memset(&view->transient.file_data, 0, sizeof(view->transient.file_data));
    view->transient.file_data.file = file;
    
    edit_pos = edit_pos_get_new(file, view->persistent.id);
    view->transient.edit_pos = edit_pos;
    
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    view->transient.line_height = font.metrics->height;
    
    if (edit_pos->cursor.line == 0){
        view_cursor_move(system, view, 0);
    }
    
    if (view->transient.showing_ui == VUI_None){
        view_show_file(view);
    }
}

internal Relative_Scrolling
view_get_relative_scrolling(View *view){
    Relative_Scrolling result = {0};
    if (view->transient.edit_pos != 0){
        Vec2 cursor = view_get_cursor_xy(view);
        result.scroll_y = cursor.y - view->transient.edit_pos->scroll.scroll_y;
        result.target_y = cursor.y - view->transient.edit_pos->scroll.target_y;
    }
    return(result);
}

internal void
view_set_relative_scrolling(View *view, Relative_Scrolling scrolling){
    Vec2 cursor = view_get_cursor_xy(view);
    if (view->transient.edit_pos != 0){
        view->transient.edit_pos->scroll.scroll_y = cursor.y - scrolling.scroll_y;
        view->transient.edit_pos->scroll.target_y = round32(clamp_bottom(0.f, cursor.y - scrolling.target_y));
    }
}

////////////////////////////////

inline void
edit_pre_maintenance(System_Functions *system, General_Memory *general, Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            general_memory_free(general, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
        file->state.still_lexing = 0;
    }
    if (file->state.dirty == DirtyState_UpToDate){
        file_set_dirty_flag(file, DirtyState_UnsavedChanges);
    }
}

internal void
edit_fix_marks(System_Functions *system, Models *models, Editing_File *file, Editing_Layout *layout, Cursor_Fix_Descriptor desc){
    
    Partition *part = &models->mem.part;
    
    Temp_Memory cursor_temp = begin_temp_memory(part);
    i32 cursor_max = layout->panel_max_count * 3;
    cursor_max += file->markers.marker_count;
    Cursor_With_Index *cursors = push_array(part, Cursor_With_Index, cursor_max);
    Cursor_With_Index *r_cursors = push_array(part, Cursor_With_Index, cursor_max);
    Assert(cursors != 0);
    
    i32 cursor_count = 0;
    i32 r_cursor_count = 0;
    
    for (Panel *panel = layout->used_sentinel.next;
         panel != &layout->used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file == file){
            Assert(view->transient.edit_pos != 0);
            write_cursor_with_index(cursors, &cursor_count, view->transient.edit_pos->cursor.pos);
            write_cursor_with_index(cursors, &cursor_count, view->transient.edit_pos->mark);
            write_cursor_with_index(cursors, &cursor_count, view->transient.edit_pos->scroll_i);
        }
    }
    
    for (Marker_Array *marker_it = file->markers.sentinel.next;
         marker_it != &file->markers.sentinel;
         marker_it = marker_it->next){
        u32 count = marker_it->count;
        Marker *markers = MarkerArrayBase(marker_it);
        for (u32 i = 0; i < count; ++i){
            if (markers[i].lean_right){
                write_cursor_with_index(r_cursors, &r_cursor_count, markers[i].pos);
            }
            else{
                write_cursor_with_index(cursors, &cursor_count, markers[i].pos);
            }
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
                Assert(view->transient.edit_pos != 0);
                
                i32 cursor_pos = cursors[cursor_count++].pos;
                Editing_File *file = view->transient.file_data.file;
                Full_Cursor new_cursor = file_compute_cursor(system, file, seek_pos(cursor_pos), 0);
                
                GUI_Scroll_Vars scroll = view->transient.edit_pos->scroll;
                
                view->transient.edit_pos->mark = cursors[cursor_count++].pos;
                i32 new_scroll_i = cursors[cursor_count++].pos;
                if (view->transient.edit_pos->scroll_i != new_scroll_i){
                    view->transient.edit_pos->scroll_i = new_scroll_i;
                    
                    Full_Cursor temp_cursor = file_compute_cursor(system, file, seek_pos(view->transient.edit_pos->scroll_i), 0);
                    
                    f32 y_offset = MOD(view->transient.edit_pos->scroll.scroll_y, view->transient.line_height);
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
        
        for (Marker_Array *marker_it = file->markers.sentinel.next;
             marker_it != &file->markers.sentinel;
             marker_it = marker_it->next){
            u32 count = marker_it->count;
            Marker *markers = MarkerArrayBase(marker_it);
            for (u32 i = 0; i < count; ++i){
                if (markers[i].lean_right){
                    markers[i].pos = r_cursors[r_cursor_count++].pos;
                }
                else{
                    markers[i].pos = cursors[cursor_count++].pos;
                }
            }
        }
    }
    
    end_temp_memory(cursor_temp);
}

internal void
edit_single__inner(System_Functions *system, Models *models, Editing_File *file,
                   Edit_Spec spec, History_Mode history_mode){
    
    Mem_Options *mem = &models->mem;
    Editing_Layout *layout = &models->layout;
    
    // NOTE(allen): fixing stuff beforewards????
    file_update_history_before_edit(mem, file, spec.step, spec.str, history_mode);
    edit_pre_maintenance(system, &mem->general, file);
    
    // NOTE(allen): actual text replacement
    i32 shift_amount = 0;
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    char *str = (char*)spec.str;
    i32 start = spec.step.edit.start;
    i32 end = spec.step.edit.end;
    i32 str_len = spec.step.edit.len;
    
    i32 scratch_size = partition_remaining(part);
    
    Assert(scratch_size > 0);
    i32 request_amount = 0;
    Assert(end <= buffer_size(&file->state.buffer));
    while (buffer_replace_range(&file->state.buffer, start, end, str, str_len, &shift_amount, part->base + part->pos, scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data){
            general_memory_free(general, old_data);
        }
    }
    
    // NOTE(allen): token fixing
    if (file->settings.tokens_exist){
        if (!file->settings.virtual_white){
            file_relex_parallel(system, models, file, start, end, shift_amount);
        }
        else{
            file_relex_serial(models, file, start, end, shift_amount);
        }
    }
    
    // NOTE(allen): meta data
    Gap_Buffer *buffer = &file->state.buffer;
    i32 line_start = buffer_get_line_number(&file->state.buffer, start);
    i32 line_end = buffer_get_line_number(&file->state.buffer, end);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = buffer_count_newlines(&file->state.buffer, start, start+str_len);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    Assert(font.valid);
    
    file_grow_starts_as_needed(general, buffer, line_shift);
    buffer_remeasure_starts(buffer, line_start, line_end, line_shift, shift_amount);
    
    file_allocate_character_starts_as_needed(general, file);
    buffer_remeasure_character_starts(system, font, buffer, line_start, line_end, line_shift, file->state.character_starts, 0, file->settings.virtual_white);
    
    file_measure_wraps(system, &models->mem, file, font);
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {0};
    desc.start = start;
    desc.end = end;
    desc.shift_amount = shift_amount;
    edit_fix_marks(system, models, file, layout, desc);
}

inline void
edit_single(System_Functions *system, Models *models, Editing_File *file,
            i32 start, i32 end, char *str, i32 len){
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    spec.step.edit.len = len;
    spec.str = (u8*)str;
    edit_single__inner(system, models, file, spec,
                       hist_normal);
}

internal Edit_Spec
edit_compute_batch_spec(General_Memory *general,
                        Editing_File *file,
                        Buffer_Edit *edits, char *str_base, i32 str_size,
                        Buffer_Edit *inverse_array, char *inv_str, i32 inv_max, i32 edit_count, i32 batch_type){
    
    i32 inv_str_pos = 0;
    Buffer_Invert_Batch state = {};
    if (buffer_invert_batch(&state, &file->state.buffer, edits, edit_count,
                            inverse_array, inv_str, &inv_str_pos, inv_max)){
        InvalidCodePath;
    }
    
    i32 first_child = undo_children_push(general, &file->state.undo.children, edits, edit_count, (u8*)(str_base), str_size);
    i32 inverse_first_child = undo_children_push(general, &file->state.undo.children, inverse_array, edit_count, (u8*)(inv_str), inv_str_pos);
    
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
edit_batch(System_Functions *system, Models *models, Editing_File *file,
           Edit_Spec spec, History_Mode history_mode, Buffer_Batch_Edit_Type batch_type){
    
    Mem_Options *mem = &models->mem;
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    Editing_Layout *layout = &models->layout;
    
    // NOTE(allen): fixing stuff "beforewards"???
    Assert(spec.str == 0);
    file_update_history_before_edit(mem, file, spec.step, 0, history_mode);
    edit_pre_maintenance(system, &mem->general, file);
    
    // NOTE(allen): actual text replacement
    u8 *str_base = file->state.undo.children.strings;
    i32 batch_size = spec.step.child_count;
    Buffer_Edit *batch = file->state.undo.children.edits + spec.step.first_child;
    
    Assert(spec.step.first_child < file->state.undo.children.edit_count);
    Assert(batch_size >= 0);
    
    i32 scratch_size = partition_remaining(part);
    Buffer_Batch_State state = {};
    i32 request_amount = 0;
    while (buffer_batch_edit_step(&state, &file->state.buffer, batch,
                                  (char*)str_base, batch_size, part->base + part->pos,
                                  scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data){
            general_memory_free(general, old_data);
        }
    }
    
    i32 shift_total = state.shift_total;
    
    // NOTE(allen): token fixing
    switch (batch_type){
        case BatchEdit_Normal:
        {
            if (file->settings.tokens_exist){
                // TODO(allen): Write a smart fast one here someday.
                Buffer_Edit *first_edit = batch;
                Buffer_Edit *last_edit = batch + batch_size - 1;
                
                if (!file->settings.virtual_white){
                    file_relex_parallel(system, models, file, first_edit->start, last_edit->end, shift_total);
                }
                else{
                    file_relex_serial(models, file, first_edit->start, last_edit->end, shift_total);
                }
            }
        }break;
        
        case BatchEdit_PreserveTokens:
        {
            if (file->state.tokens_complete){
                Cpp_Token_Array tokens = file->state.token_array;
                Cpp_Token *token = tokens.tokens;
                Cpp_Token *end_token = tokens.tokens + tokens.count;
                Cpp_Token original = {(Cpp_Token_Type)0};
                
                Buffer_Edit *edit = batch;
                Buffer_Edit *end_edit = batch + batch_size;
                
                i32 shift_amount = 0;
                i32 local_shift = 0;
                
                for (; token < end_token; ++token){
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
            }
        }break;
    }
    
    // TODO(allen): Let's try to switch to remeasuring here moron!
    // We'll need to get the total shift from the actual batch edit state
    // instead of from the cursor fixing.  The only reason we're getting
    // it from cursor fixing is because you're a lazy asshole.
    
    // NOTE(allen): meta data
    file_measure_starts(general, &file->state.buffer);
    
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    Assert(font.valid);
    
    // TODO(allen): write the remeasurement version
    file_allocate_character_starts_as_needed(general, file);
    buffer_measure_character_starts(system, font, &file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
    
    file_measure_wraps(system, &models->mem, file, font);
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {0};
    desc.is_batch = 1;
    desc.batch = batch;
    desc.batch_size = batch_size;
    edit_fix_marks(system, models, file, layout, desc);
}

inline void
edit_clear(System_Functions *system, Models *models, Editing_File *file){
    if (models->hook_end_file != 0){
        models->hook_end_file(&models->app_links, file->id.id);
    }
    edit_single(system, models, file,
                0, buffer_size(&file->state.buffer), 0, 0);
}

internal void
edit_historical(System_Functions *system, Models *models, Editing_File *file, View *view, Edit_Stack *stack,
                Edit_Step step, History_Mode history_mode){
    Edit_Spec spec = {};
    spec.step = step;
    
    if (step.child_count == 0){
        spec.step.edit.str_start = 0;
        spec.str = stack->strings + step.edit.str_start;
        
        edit_single__inner(system, models, file,
                           spec, history_mode);
        
        if (view != 0){
            view_cursor_move(system, view, step.edit.start + step.edit.len);
            view->transient.edit_pos->mark = view->transient.edit_pos->cursor.pos;
            
            Style *style = &models->styles.styles[0];
            view_post_paste_effect(view, 0.333f, step.edit.start, step.edit.len, style->main.undo_color);
        }
    }
    else{
        edit_batch(system, models, view->transient.file_data.file, spec, hist_normal, spec.step.special_type);
    }
}

////////////////////////////////

internal void
kill_file_and_update_views(System_Functions *system, Models *models, Editing_File *file){
    Working_Set *working_set = &models->working_set;
    
    if (file != 0 && !file->settings.never_kill){
        if (models->hook_end_file != 0){
            models->hook_end_file(&models->app_links, file->id.id);
        }
        
        buffer_unbind_name_low_level(working_set, file);
        if (file->canon.name.size != 0){
            buffer_unbind_file(system, working_set, file);
        }
        file_free(system, &models->mem.general, file);
        working_set_free_file(&models->mem.general, working_set, file);
        
        File_Node *used = &working_set->used_sentinel;
        File_Node *node = used->next;
        for (Panel *panel = models->layout.used_sentinel.next;
             panel != &models->layout.used_sentinel;
             panel = panel->next){
            View *view = panel->view;
            if (view->transient.file_data.file == file){
                Assert(node != used);
                view->transient.file_data.file = 0;
                view_set_file(system, models, view, (Editing_File*)node);
                if (node->next != used){
                    node = node->next;
                }
                else{
                    node = node->next->next;
                    Assert(node != used);
                }
            }
        }
    }
}

internal Try_Kill_Result
interactive_try_kill_file(System_Functions *system, Models *models, Editing_File *file){
    Try_Kill_Result result = TryKill_CannotKill;
    if (!file->settings.never_kill){
        if (buffer_needs_save(file)){
            result = TryKill_NeedDialogue;
        }
        else{
            kill_file_and_update_views(system, models, file);
            result = TryKill_Success;
        }
    }
    return(result);
}

internal void
interactive_begin_sure_to_kill(System_Functions *system, View *view, Models *models, Editing_File *file){
    view_show_interactive(system, view, models, IAct_Sure_To_Kill, IInt_Sure_To_Kill, make_lit_string("Are you sure?"));
    copy(&view->transient.dest, file->unique_name.name);
}

internal void
interactive_view_complete(System_Functions *system, View *view, Models *models, String dest, i32 user_action){
    switch (view->transient.action){
        case IAct_Open:
        {
            Editing_File *file = open_file(system, models, dest);
            if (file != 0){
                view_set_file(system, models, view, file);
            }
            view_show_file(view);
        }break;
        
        case IAct_New:
        {
            if (dest.size > 0 && !char_is_slash(dest.str[dest.size-1])){
                Editing_File *file = 0;
                Editing_File_Name canon_name = {0};
                
                if (terminate_with_null(&dest) &&
                    get_canon_name(system, dest, &canon_name)){
                    Working_Set *working_set = &models->working_set;
                    file = working_set_contains_canon(working_set, canon_name.name);
                    if (file == 0){
                        Mem_Options *mem = &models->mem;
                        General_Memory *general = &mem->general;
                        Partition *part = &mem->part;
                        
                        file = working_set_alloc_always(working_set, general);
                        buffer_bind_file(system, general, working_set, file, canon_name.name);
                        buffer_bind_name(models, general, part, working_set, file, front_of_directory(dest));
                        
                        init_normal_file(system, models, 0, 0, file);
                    }
                    else{
                        edit_clear(system, models, file);
                    }
                }
                if (file != 0){
                    view_set_file(system, models, view, file);
                }
                view_show_file(view);
                if (file != 0 && models->hook_new_file != 0){
                    models->hook_new_file(&models->app_links, file->id.id);
                }
            }
        }break;
        
        case IAct_Switch:
        {
            Editing_File *file = working_set_contains_name(&models->working_set, dest);
            if (file){
                view_set_file(system, models, view, file);
            }
            view_show_file(view);
        }break;
        
        case IAct_Kill:
        {
            b32 kill_dialogue = false;
            Editing_File *file = working_set_contains_name(&models->working_set, dest);
            if (file != 0){
                kill_dialogue = (interactive_try_kill_file(system, models, file) == TryKill_NeedDialogue);
                if (kill_dialogue){
                    interactive_begin_sure_to_kill(system, view, models, file);
                }
            }
            if (!kill_dialogue){
                view_show_file(view);
            }
        }break;
        
        case IAct_Sure_To_Close:
        {
            switch (user_action){
                case UnsavedChangesUserResponse_ContinueAnyway:
                {
                    models->keep_playing = false;
                }break;
                
                case UnsavedChangesUserResponse_Cancel:
                {
                    view_show_file(view);
                }break;
                
                // TODO(allen): Save all and close.
                case UnsavedChangesUserResponse_SaveAndContinue: InvalidCodePath;
                default: InvalidCodePath;
            }
        }break;
        
        case IAct_Sure_To_Kill:
        {
            switch (user_action){
                case UnsavedChangesUserResponse_ContinueAnyway:
                {
                    Editing_File *file = working_set_contains_name(&models->working_set, dest);
                    if (file != 0){
                        kill_file_and_update_views(system, models, file);
                    }
                    view_show_file(view);
                }break;
                
                case UnsavedChangesUserResponse_Cancel:
                {
                    view_show_file(view);
                }break;
                
                case UnsavedChangesUserResponse_SaveAndContinue:
                {
                    Editing_File *file = working_set_contains_name(&models->working_set, dest);
                    if (file != 0){
                        save_file(system, models, file);
                        kill_file_and_update_views(system, models, file);
                    }
                    view_show_file(view);
                }break;
                
                default: InvalidCodePath;
            }
        }break;
        
        case IAct_OpenOrNew: InvalidCodePath;
    }
}

////////////////////////////////

internal void
begin_exhaustive_loop(Exhaustive_File_Loop *loop, Hot_Directory *hdir){
    loop->front_name = make_fixed_width_string(loop->front_name_);
    loop->full_path = make_fixed_width_string(loop->full_path_);
    
    loop->infos = hdir->file_list.infos;
    loop->count = hdir->file_list.count;
    
    copy_ss(&loop->front_name, front_of_directory(hdir->string));
    get_absolutes(loop->front_name, &loop->absolutes, 1, 1);
    copy_ss(&loop->full_path, hdir->canon_dir);
    loop->r = loop->full_path.size;
}

internal Exhaustive_File_Info
get_exhaustive_info(System_Functions *system, Working_Set *working_set, Exhaustive_File_Loop *loop, i32 i){
    local_persist String message_loaded = make_lit_string(" LOADED");
    local_persist String message_unsaved = make_lit_string(" LOADED *");
    local_persist String message_unsynced = make_lit_string(" LOADED !");
    
    Exhaustive_File_Info result = {0};
    
    result.info = loop->infos + i;
    loop->full_path.size = loop->r;
    append_sc(&loop->full_path, result.info->filename);
    terminate_with_null(&loop->full_path);
    
    Editing_File *file = working_set_contains_canon(working_set, loop->full_path);
    
    String filename = make_string_cap(result.info->filename, result.info->filename_len, result.info->filename_len+1);
    
    result.is_folder = (result.info->folder != 0);
    result.name_match = (wildcard_match_s(&loop->absolutes, filename, 0) != 0);
    result.is_loaded = (file != 0 && file_is_ready(file));
    
    result.message = null_string;
    if (result.is_loaded){
        switch (file->state.dirty){
            case DirtyState_UpToDate: result.message = message_loaded; break;
            case DirtyState_UnsavedChanges: result.message = message_unsaved; break;
            case DirtyState_UnloadedChanges: result.message = message_unsynced; break;
        }
    }
    
    return(result);
}

static Style_Color_Edit colors_to_edit[] = {
    {Stag_Back, Stag_Default, Stag_Back, make_lit_string("Background")},
    {Stag_Margin, Stag_Default, Stag_Margin, make_lit_string("Margin")},
    {Stag_Margin_Hover, Stag_Default, Stag_Margin_Hover, make_lit_string("Margin Hover")},
    {Stag_Margin_Active, Stag_Default, Stag_Margin_Active, make_lit_string("Margin Active")},
    
    {Stag_Cursor, Stag_At_Cursor, Stag_Cursor, make_lit_string("Cursor")},
    {Stag_At_Cursor, Stag_At_Cursor, Stag_Cursor, make_lit_string("Text At Cursor")},
    {Stag_Mark, Stag_Mark, Stag_Back, make_lit_string("Mark")},
    
    {Stag_Highlight, Stag_At_Highlight, Stag_Highlight, make_lit_string("Highlight")},
    {Stag_At_Highlight, Stag_At_Highlight, Stag_Highlight, make_lit_string("Text At Highlight")},
    
    {Stag_Default, Stag_Default, Stag_Back, make_lit_string("Text Default")},
    {Stag_Comment, Stag_Comment, Stag_Back, make_lit_string("Comment")},
    {Stag_Keyword, Stag_Keyword, Stag_Back, make_lit_string("Keyword")},
    {Stag_Str_Constant, Stag_Str_Constant, Stag_Back, make_lit_string("String Constant")},
    {Stag_Char_Constant, Stag_Char_Constant, Stag_Back, make_lit_string("Character Constant")},
    {Stag_Int_Constant, Stag_Int_Constant, Stag_Back, make_lit_string("Integer Constant")},
    {Stag_Float_Constant, Stag_Float_Constant, Stag_Back, make_lit_string("Float Constant")},
    {Stag_Bool_Constant, Stag_Bool_Constant, Stag_Back, make_lit_string("Boolean Constant")},
    {Stag_Preproc, Stag_Preproc, Stag_Back, make_lit_string("Preprocessor")},
    {Stag_Special_Character, Stag_Special_Character, Stag_Back, make_lit_string("Special Character")},
    
    {Stag_Highlight_Junk, Stag_Default, Stag_Highlight_Junk, make_lit_string("Junk Highlight")},
    {Stag_Highlight_White, Stag_Default, Stag_Highlight_White, make_lit_string("Whitespace Highlight")},
    
    {Stag_Paste, Stag_Paste, Stag_Back, make_lit_string("Paste Color")},
    
    {Stag_Bar, Stag_Base, Stag_Bar, make_lit_string("Bar")},
    {Stag_Base, Stag_Base, Stag_Bar, make_lit_string("Bar Text")},
    {Stag_Pop1, Stag_Pop1, Stag_Bar, make_lit_string("Bar Pop 1")},
    {Stag_Pop2, Stag_Pop2, Stag_Bar, make_lit_string("Bar Pop 2")},
};

internal Single_Line_Input_Step
app_single_line_input__inner(System_Functions *system, Working_Set *working_set, Key_Event_Data key, Single_Line_Mode mode){
    Single_Line_Input_Step result = {0};
    
    b8 ctrl = key.modifiers[MDFR_CONTROL_INDEX];
    b8 cmnd = key.modifiers[MDFR_COMMAND_INDEX];
    b8 alt  = key.modifiers[MDFR_ALT_INDEX];
    
    if (key.keycode == key_back){
        result.hit_backspace = true;
        if (mode.string->size > 0){
            result.made_a_change = true;
            --mode.string->size;
            switch (mode.type){
                case SINGLE_LINE_STRING:
                {
                    mode.string->str[mode.string->size] = 0;
                }break;
                
                case SINGLE_LINE_FILE:
                {
                    if (!ctrl && !cmnd && !alt){
                        char end_character = mode.string->str[mode.string->size];
                        if (char_is_slash(end_character)){
                            mode.string->size = reverse_seek_slash(*mode.string) + 1;
                            mode.string->str[mode.string->size] = 0;
                            hot_directory_set(system, mode.hot_directory, *mode.string);
                        }
                        else{
                            mode.string->str[mode.string->size] = 0;
                        }
                    }
                    else{
                        mode.string->str[mode.string->size] = 0;
                    }
                }break;
            }
        }
    }
    
    else if (key.character == '\n' || key.character == '\t'){
        // NOTE(allen): do nothing!
    }
    
    else if (key.keycode == key_esc){
        result.hit_esc = true;
        result.made_a_change = true;
    }
    
    else if (key.character){
        result.hit_a_character = true;
        if (!ctrl && !cmnd && !alt){
            if (mode.string->size + 1 < mode.string->memory_size){
                u8 new_character = (u8)key.character;
                mode.string->str[mode.string->size] = new_character;
                mode.string->size++;
                mode.string->str[mode.string->size] = 0;
                if (mode.type == SINGLE_LINE_FILE && char_is_slash(new_character)){
                    hot_directory_set(system, mode.hot_directory, *mode.string);
                }
                result.made_a_change = true;
            }
        }
        else{
            result.did_command = true;
        }
    }
    
    return(result);
}

inline Single_Line_Input_Step
app_single_line_input_step(System_Functions *system, Key_Event_Data key, String *string){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_STRING;
    mode.string = string;
    return(app_single_line_input__inner(system, 0, key, mode));
}

inline Single_Line_Input_Step
app_single_file_input_step(System_Functions *system,
                           Working_Set *working_set, Key_Event_Data key,
                           String *string, Hot_Directory *hot_directory,
                           b32 try_to_match, b32 case_sensitive){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_FILE;
    mode.string = string;
    mode.hot_directory = hot_directory;
    mode.try_to_match = try_to_match;
    mode.case_sensitive = case_sensitive;
    return(app_single_line_input__inner(system, working_set, key, mode));
}

internal View_Step_Result
step_file_view(System_Functions *system, View *view, Models *models, View *active_view, Input_Summary input){
    View_Step_Result result = {0};
    GUI_Target *target = &view->transient.gui_target;
    Key_Input_Data keys = input.keys;
    
    b32 show_scrollbar = !view->transient.hide_scrollbar;
    
    if (view->transient.showing_ui != VUI_None){
        b32 did_esc = false;
        for (i32 i = 0; i < keys.count; ++i){
            Key_Event_Data key = keys.keys[i];
            if (key.keycode == key_esc){
                did_esc = 1;
                break;
            }
        }
        
        if (did_esc){
            view_show_file(view);
            result.consume_esc = true;
        }
    }
    
    gui_begin_top_level(target, input);
    {
        if (!view->transient.hide_file_bar){
            gui_do_top_bar(target);
        }
        
        // NOTE(allen): A temporary measure... although in
        // general we maybe want the user to be able to ask
        // how large a particular section of the GUI turns
        // out to be after layout?
        i32 bar_count = 0;
        for (Query_Slot *slot = view->transient.query_set.used_slot;
             slot != 0;
             slot = slot->next, ++bar_count){
            Query_Bar *bar = slot->query_bar;
            gui_do_text_field(target, bar->prompt, bar->string);
        }
        view->transient.widget_height = (f32)bar_count*(view->transient.line_height + 2);
        
        if (view->transient.showing_ui == VUI_None){
            gui_begin_serial_section(target);
            
            i32 delta = 9*view->transient.line_height;
            GUI_id scroll_context = {0};
            scroll_context.id[1] = view->transient.showing_ui;
            scroll_context.id[0] = (u64)(view->transient.file_data.file);
            
            Assert(view->transient.file_data.file != 0);
            Assert(view->transient.edit_pos != 0);
            
            GUI_Scroll_Vars *scroll = &view->transient.edit_pos->scroll;
            gui_begin_scrollable(target, scroll_context, *scroll,
                                 delta, show_scrollbar);
            
            gui_do_file(target);
            gui_end_scrollable(target);
            
            gui_end_serial_section(target);
        }
        else{
            switch (view->transient.showing_ui){
                case VUI_Theme:
                {
                    if (view != active_view){
                        view->transient.hot_file_view = active_view;
                    }
                    
                    String message = {0};
                    String empty_string = {0};
                    
                    GUI_id id = {0};
                    id.id[1] = VUI_Theme + ((u64)view->transient.color_mode << 32);
                    
                    GUI_id scroll_context = {0};
                    scroll_context.id[0] = 0;
                    scroll_context.id[1] = VUI_Theme + ((u64)view->transient.color_mode << 32);
                    
                    switch (view->transient.color_mode){
                        case CV_Mode_Library:
                        {
                            message = make_lit_string("Current Theme - Click to Edit");
                            gui_do_text_field(target, message, empty_string);
                            
                            id.id[0] = (u64)(&models->styles.styles[0]);
                            if (gui_do_style_preview(target, id, 0)){
                                view->transient.color_mode = CV_Mode_Adjusting;
                            }
                            
                            Assert(view->transient.file_data.file != 0);
                            
                            message = make_lit_string("Set Font");
                            id.id[0] = (u64)(&view->transient.file_data.file->settings.font_id);
                            if (gui_do_button(target, id, message)){
                                view->transient.color_mode = CV_Mode_Font;
                            }
                            
                            
                            message = make_lit_string("Set Global Font");
                            id.id[0] = (u64)(&models->global_font_id);
                            
                            if (gui_do_button(target, id, message)){
                                view->transient.color_mode = CV_Mode_Global_Font;
                            }
                            
                            message = make_lit_string("Theme Library - Click to Select");
                            gui_do_text_field(target, message, empty_string);
                            
                            gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll,
                                                 9*view->transient.line_height, show_scrollbar);
                            
                            {
                                i32 count = models->styles.count;
                                for (i32 i = 1; i < count; ++i){
                                    Style *style = &models->styles.styles[i];
                                    id.id[0] = (u64)(style);
                                    if (gui_do_style_preview(target, id, i)){
                                        style_copy(&models->styles.styles[0], style);
                                    }
                                }
                            }
                            
                            gui_end_scrollable(target);
                        }
                        break;
                        
                        case CV_Mode_Font:
                        case CV_Mode_Global_Font:
                        {
                            Editing_File *file = view->transient.file_data.file;
                            Assert(file != 0);
                            
                            id.id[0] = 0;
                            if (gui_do_button(target, id, make_lit_string("Back"))){
                                view->transient.color_mode = CV_Mode_Library;
                            }
                            
                            gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                            
                            Face_ID font_id = file->settings.font_id;
                            Face_ID new_font_id = 0;
                            
                            Face_ID largest_id = system->font.get_largest_id();
                            for (Face_ID i = 1; i <= largest_id; ++i){
                                Font_Pointers font = system->font.get_pointers_by_id(i);
                                if (font.valid){
                                    Font_Settings *settings = font.settings;
                                    Font_Metrics *metrics = font.metrics;
                                    
                                    char space[512];
                                    String m = make_fixed_width_string(space);
                                    if (i == font_id){
                                        append(&m, "*");
                                    }
                                    
                                    append(&m, " \"");
                                    append(&m, make_string(metrics->name, metrics->name_len));
                                    append(&m, "\" ");
                                    append_int_to_str(&m, settings->parameters.pt_size);
                                    append(&m, " ");
                                    append(&m, (char*)(settings->parameters.italics?"italics ":""));
                                    append(&m, (char*)(settings->parameters.bold?"bold ":""));
                                    append(&m, (char*)(settings->parameters.underline?"underline ":""));
                                    append(&m, (char*)(settings->parameters.use_hinting?"hinting ":""));
                                    
                                    if (i == font_id){
                                        append(&m, "*");
                                    }
                                    
                                    id.id[0] = i*2 + 0;
                                    if (gui_do_button(target, id, m)){
                                        if (new_font_id == 0){
                                            new_font_id = i;
                                        }
                                    }
                                    
                                    id.id[0] = i*2 + 1;
                                    if (gui_do_button(target, id, make_lit_string("edit"))){
                                        view->transient.font_edit_id = i;
                                        if (view->transient.color_mode == CV_Mode_Font){
                                            view->transient.color_mode = CV_Mode_Font_Editing;
                                        }
                                        else{
                                            view->transient.color_mode = CV_Mode_Global_Font_Editing;
                                        }
                                    }
                                }
                            }
                            
                            id.id[0] = largest_id*2 + 2;
                            if (gui_do_button(target, id, make_lit_string("new face"))){
                                if (new_font_id == 0){
                                    Font_Pointers font = system->font.get_pointers_by_id(font_id);
                                    view->transient.font_edit_id = system->font.face_allocate_and_init(font.settings);
                                    if (view->transient.color_mode == CV_Mode_Font){
                                        view->transient.color_mode = CV_Mode_Font_Editing;
                                    }
                                    else{
                                        view->transient.color_mode = CV_Mode_Global_Font_Editing;
                                    }
                                }
                            }
                            
                            if (new_font_id != 0){
                                if (view->transient.color_mode == CV_Mode_Font && new_font_id != font_id){
                                    file_set_font(system, models, file, new_font_id);
                                }
                                else if (new_font_id != font_id || new_font_id != models->global_font_id){
                                    global_set_font_and_update_files(system, models, new_font_id);
                                }
                            }
                            
                            gui_end_scrollable(target);
                        }break;
                        
                        case CV_Mode_Font_Editing:
                        case CV_Mode_Global_Font_Editing:
                        {
                            id.id[0] = 0;
                            if (gui_do_button(target, id, make_lit_string("Back"))){
                                if (view->transient.color_mode == CV_Mode_Font_Editing){
                                    view->transient.color_mode = CV_Mode_Font;
                                }
                                else{
                                    view->transient.color_mode = CV_Mode_Global_Font;
                                }
                            }
                            
                            gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                            
                            Face_ID font_edit_id = view->transient.font_edit_id;
                            Font_Pointers font = system->font.get_pointers_by_id(font_edit_id);
                            Font_Settings *settings = font.settings;
                            Font_Metrics *metrics = font.metrics;
                            Font_Settings new_settings = *settings;
                            b32 has_new_settings = false;
                            
                            char space[128];
                            String m = make_fixed_width_string(space);
                            copy(&m, "Size Up (");
                            append_int_to_str(&m, settings->parameters.pt_size);
                            append(&m, ")");
                            ++id.id[0];
                            if (gui_do_button(target, id, m)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    ++new_settings.parameters.pt_size;
                                }
                            }
                            
                            copy(&m, "Size Down (");
                            append_int_to_str(&m, settings->parameters.pt_size);
                            append(&m, ")");
                            ++id.id[0];
                            if (gui_do_button(target, id, m)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    --new_settings.parameters.pt_size;
                                }
                            }
                            
                            copy(&m, "Italics [");
                            append(&m, (char*)(settings->parameters.italics?"+":" "));
                            append(&m, "]");
                            ++id.id[0];
                            if (gui_do_button(target, id, m)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    new_settings.parameters.italics = !new_settings.parameters.italics;
                                }
                            }
                            
                            copy(&m, "Bold [");
                            append(&m, (char*)(settings->parameters.bold?"+":" "));
                            append(&m, "]");
                            ++id.id[0];
                            if (gui_do_button(target, id, m)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    new_settings.parameters.bold = !new_settings.parameters.bold;
                                }
                            }
                            
                            copy(&m, "Underline [");
                            append(&m, (char*)(settings->parameters.underline?"+":" "));
                            append(&m, "]");
                            ++id.id[0];
                            if (gui_do_button(target, id, m)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    new_settings.parameters.underline = !new_settings.parameters.underline;
                                }
                            }
                            
                            copy(&m, "Hinting [");
                            append(&m, (char*)(settings->parameters.use_hinting?"+":" "));
                            append(&m, "]");
                            ++id.id[0];
                            if (gui_do_button(target, id, m)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    new_settings.parameters.use_hinting = !new_settings.parameters.use_hinting;
                                }
                            }
                            
                            copy(&m, "Current Family: ");
                            append(&m, make_string(metrics->name, metrics->name_len));
                            ++id.id[0];
                            gui_do_button(target, id, m);
                            
                            i32 total_count = system->font.get_loadable_count();
                            for (i32 i = 0; i < total_count; ++i){
                                Font_Loadable_Description loadable = {0};
                                system->font.get_loadable(i, &loadable);
                                
                                if (loadable.valid){
                                    String name = make_string(loadable.display_name, loadable.display_len);
                                    ++id.id[0];
                                    if (gui_do_button(target, id, name)){
                                        if (!has_new_settings){
                                            has_new_settings = true;
                                            memcpy(&new_settings.stub, &loadable.stub, sizeof(loadable.stub));
                                        }
                                    }
                                }
                            }
                            
                            gui_end_scrollable(target);
                            
                            if (has_new_settings){
                                alter_font_and_update_files(system, models, font_edit_id, &new_settings);
                            }
                        }break;
                        
                        case CV_Mode_Adjusting:
                        {
                            Style *style = &models->styles.styles[0];
                            u32 *edit_color = 0;
                            u32 *fore = 0, *back = 0;
                            i32 i = 0;
                            
                            message = make_lit_string("Back");
                            
                            id.id[0] = 0;
                            if (gui_do_button(target, id, message)){
                                view->transient.color_mode = CV_Mode_Library;
                            }
                            
                            gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                            
                            i32 next_color_editing = view->transient.current_color_editing;
                            
                            for (i = 0; i < ArrayCount(colors_to_edit); ++i){
                                edit_color = style_index_by_tag(&style->main, colors_to_edit[i].target);
                                id.id[0] = (u64)(edit_color);
                                
                                fore = style_index_by_tag(&style->main, colors_to_edit[i].fore);
                                back = style_index_by_tag(&style->main, colors_to_edit[i].back);
                                
                                if (gui_do_color_button(target, id, *fore, *back, colors_to_edit[i].text)){
                                    next_color_editing = i;
                                    view->transient.color_cursor = 0;
                                }
                                
                                if (view->transient.current_color_editing == i){
                                    GUI_Item_Update update = {0};
                                    char text_space[7];
                                    String text = make_fixed_width_string(text_space);
                                    
                                    color_to_hexstr(&text, *edit_color);
                                    if (gui_do_text_with_cursor(target, view->transient.color_cursor, text, &update)){
                                        b32 r = false;
                                        i32 j = 0;
                                        
                                        for (j = 0; j < keys.count; ++j){
                                            Key_Code key = keys.keys[j].keycode;
                                            switch (key){
                                                case key_left:
                                                {
                                                    --view->transient.color_cursor;
                                                    r = true;
                                                    result.consume_keys = 1;
                                                }break;
                                                case key_right:
                                                {
                                                    ++view->transient.color_cursor;
                                                    r = true;
                                                    result.consume_keys = 1;
                                                }break;
                                                
                                                case key_up:
                                                {
                                                    if (next_color_editing > 0){
                                                        --next_color_editing;
                                                    }
                                                    result.consume_keys = 1;
                                                }break;
                                                
                                                case key_down:
                                                {
                                                    if (next_color_editing <= ArrayCount(colors_to_edit)-1){
                                                        ++next_color_editing;
                                                    }
                                                    result.consume_keys = 1;
                                                }break;
                                                
                                                default:
                                                {
                                                    if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f') || (key >= 'A' && key <= 'F')){
                                                        text.str[view->transient.color_cursor] = (char)key;
                                                        r = true;
                                                        result.consume_keys = 1;
                                                    }
                                                }break;
                                            }
                                            
                                            if (view->transient.color_cursor < 0){
                                                view->transient.color_cursor = 0;
                                            }
                                            if (view->transient.color_cursor >= 6){
                                                view->transient.color_cursor = 5;
                                            }
                                        }
                                        
                                        if (r){
                                            hexstr_to_color(text, edit_color);
                                            gui_rollback(target, &update);
                                            gui_do_text_with_cursor(target, view->transient.color_cursor, text, 0);
                                        }
                                    }
                                }
                            }
                            
                            if (view->transient.current_color_editing != next_color_editing){
                                view->transient.current_color_editing = next_color_editing;
                                view->transient.color_cursor = 0;
                            }
                            
                            gui_end_scrollable(target);
                        }break;
                    }
                }break;
                
                case VUI_Interactive:
                {
                    b32 complete = 0;
                    char comp_dest_space[1024];
                    String comp_dest = make_fixed_width_string(comp_dest_space);
                    i32 comp_action = 0;
                    
                    GUI_id id = {0};
                    id.id[1] = VUI_Interactive + ((u64)view->transient.interaction << 32);
                    
                    GUI_id scroll_context = {0};
                    scroll_context.id[1] = VUI_Interactive + ((u64)view->transient.interaction << 32);
                    
                    Key_Code user_up_key = models->user_up_key;
                    Key_Code user_down_key = models->user_down_key;
                    Key_Modifier user_up_key_modifier = models->user_up_key_modifier;
                    Key_Modifier user_down_key_modifier = models->user_down_key_modifier;
                    
                    switch (view->transient.interaction){
                        case IInt_Sys_File_List:
                        {
                            b32 autocomplete_with_enter = true;
                            b32 activate_directly = false;
                            
                            if (view->transient.action == IAct_New){
                                autocomplete_with_enter = false;
                            }
                            
                            String message = null_string;
                            switch (view->transient.action){
                                case IAct_OpenOrNew:
                                case IAct_Open:
                                {
                                    message = make_lit_string("Open: ");
                                }break;
                                
                                case IAct_New:
                                {
                                    message = make_lit_string("New: ");
                                }break;
                            }
                            
                            GUI_Item_Update update = {0};
                            Hot_Directory *hdir = &models->hot_directory;
                            
                            b32 do_open_or_new = false;
                            for (i32 i = 0; i < keys.count; ++i){
                                Key_Event_Data key = keys.keys[i];
                                Single_Line_Input_Step step = app_single_file_input_step(system, &models->working_set, key,
                                                                                         &hdir->string, hdir, 1, 0);
                                
                                if (step.made_a_change){
                                    view->transient.list_i = 0;
                                    result.consume_keys = true;
                                }
                                
                                if (key.keycode == '\n'){
                                    if (!autocomplete_with_enter){
                                        activate_directly = true;
                                        result.consume_keys = true;
                                    }
                                    else if (view->transient.action == IAct_OpenOrNew){
                                        do_open_or_new = true;
                                        result.consume_keys = true;
                                    }
                                }
                            }
                            
                            gui_do_text_field(target, message, hdir->string);
                            
                            b32 snap_into_view = false;
                            scroll_context.id[0] = (u64)(hdir);
                            if (gui_scroll_was_activated(target, scroll_context)){
                                snap_into_view = true;
                            }
                            gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                            
                            id.id[0] = (u64)(hdir) + 1;
                            
                            if (gui_begin_list(target, id, view->transient.list_i, 0, snap_into_view, &update)){
                                // TODO(allen): Allow me to handle key consumption correctly here!
                                gui_standard_list(target, id, &view->transient.gui_scroll, view->transient.scroll_region, &keys, &view->transient.list_i, &update, user_up_key, user_up_key_modifier, user_down_key, user_down_key_modifier);
                            }
                            
                            b32 do_new_directory = false;
                            Exhaustive_File_Loop loop;
                            begin_exhaustive_loop(&loop, hdir);
                            for (i32 i = 0; i < loop.count; ++i){
                                Exhaustive_File_Info file_info = get_exhaustive_info(system, &models->working_set, &loop, i);
                                
                                if (file_info.name_match){
                                    id.id[0] = (u64)(file_info.info);
                                    
                                    char *str = file_info.info->filename;
                                    i32 len = file_info.info->filename_len;
                                    String filename = make_string_cap(str, len, len + 1);
                                    
                                    if (gui_do_file_option(target, id, filename, file_info.is_folder, file_info.message)){
                                        if (file_info.is_folder){
                                            set_last_folder_sc(&hdir->string, file_info.info->filename, '/');
                                            do_new_directory = true;
                                        }
                                        
                                        else if (autocomplete_with_enter){
                                            complete = true;
                                            copy_ss(&comp_dest, loop.full_path);
                                            if (view->transient.action == IAct_OpenOrNew){
                                                view->transient.action = IAct_Open;
                                            }
                                        }
                                    }
                                    
                                    if (do_open_or_new){
                                        do_open_or_new = false;
                                    }
                                }
                            }
                            
                            gui_end_list(target);
                            
                            if (activate_directly || do_open_or_new){
                                complete = true;
                                copy_ss(&comp_dest, hdir->string);
                                if (do_open_or_new){
                                    view->transient.action = IAct_New;
                                }
                            }
                            
                            if (do_new_directory){
                                hot_directory_reload(system, hdir);
                            }
                            
                            gui_end_scrollable(target);
                        }break;
                        
                        case IInt_Live_File_List:
                        {
                            local_persist String message_unsaved = make_lit_string(" *");
                            local_persist String message_unsynced = make_lit_string(" !");
                            
                            String message = null_string;
                            switch (view->transient.action){
                                case IAct_Switch:
                                {
                                    message = make_lit_string("Switch: ");
                                }break;
                                
                                case IAct_Kill:
                                {
                                    message = make_lit_string("Kill: ");
                                }break;
                            }
                            
                            Working_Set *working_set = &models->working_set;
                            Editing_Layout *layout = &models->layout;
                            GUI_Item_Update update = {0};
                            
                            for (i32 i = 0; i < keys.count; ++i){
                                Key_Event_Data key = keys.keys[i];
                                Single_Line_Input_Step step = app_single_line_input_step(system, key, &view->transient.dest);
                                if (step.made_a_change){
                                    view->transient.list_i = 0;
                                    result.consume_keys = 1;
                                }
                            }
                            
                            Absolutes absolutes = {0};
                            get_absolutes(view->transient.dest, &absolutes, 1, 1);
                            
                            gui_do_text_field(target, message, view->transient.dest);
                            
                            b32 snap_into_view = 0;
                            scroll_context.id[0] = (u64)(working_set);
                            if (gui_scroll_was_activated(target, scroll_context)){
                                snap_into_view = 1;
                            }
                            gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                            
                            id.id[0] = (u64)(working_set) + 1;
                            if (gui_begin_list(target, id, view->transient.list_i, 0, snap_into_view, &update)){
                                gui_standard_list(target, id, &view->transient.gui_scroll, view->transient.scroll_region, &keys, &view->transient.list_i, &update, user_up_key, user_up_key_modifier, user_down_key, user_down_key_modifier);
                            }
                            
                            {
                                Partition *part = &models->mem.part;
                                Temp_Memory temp = begin_temp_memory(part);
                                i32 reserved_top = 0, i = 0;
                                
                                partition_align(part, 8);
                                Editing_File **reserved_files = push_array(part, Editing_File*, 0);
                                
                                for (File_Node *node = working_set->used_sentinel.next;
                                     node != &working_set->used_sentinel;
                                     node = node->next){
                                    Editing_File *file = (Editing_File*)node;
                                    Assert(!file->is_dummy);
                                    
                                    if (wildcard_match_s(&absolutes, file->unique_name.name, 0) != 0){
                                        b32 is_viewed = file_is_viewed(layout, file);
                                        
                                        if (is_viewed){
                                            reserved_files[reserved_top++] = file;
                                        }
                                        else{
                                            if (file->unique_name.name.str[0] == '*'){
                                                reserved_files[reserved_top++] = file;
                                            }
                                            else{
                                                message = null_string;
                                                if (!file->settings.unimportant){
                                                    switch (file->state.dirty){
                                                        case DirtyState_UnloadedChanges: message = message_unsynced; break;
                                                        case DirtyState_UnsavedChanges: message = message_unsaved; break;
                                                    }
                                                }
                                                
                                                id.id[0] = (u64)(file);
                                                if (gui_do_file_option(target, id, file->unique_name.name, 0, message)){
                                                    complete = 1;
                                                    copy_ss(&comp_dest, file->unique_name.name);
                                                }
                                            }
                                        }
                                    }
                                }
                                
                                for (i = 0; i < reserved_top; ++i){
                                    Editing_File *file = reserved_files[i];
                                    
                                    message = null_string;
                                    if (!file->settings.unimportant){
                                        switch (file->state.dirty){
                                            case DirtyState_UnloadedChanges: message = message_unsynced; break;
                                            case DirtyState_UnsavedChanges: message = message_unsaved; break;
                                        }
                                    }
                                    
                                    id.id[0] = (u64)(file);
                                    if (gui_do_file_option(target, id, file->unique_name.name, 0, message)){
                                        complete = 1;
                                        copy_ss(&comp_dest, file->unique_name.name);
                                    }
                                }
                                
                                end_temp_memory(temp);
                            }
                            
                            gui_end_list(target);
                            
                            gui_end_scrollable(target);
                        }break;
                        
                        case IInt_Sure_To_Close:
                        {
                            i32 action = -1;
                            
                            String empty_str = {0};
                            String message = make_lit_string("There is one or more files unsaved changes, close anyway?");
                            
                            gui_do_text_field(target, message, empty_str);
                            
                            id.id[0] = (u64)('y');
                            message = make_lit_string("(Y)es");
                            if (gui_do_fixed_option(target, id, message, 'y')){
                                action = 0;
                            }
                            
                            id.id[0] = (u64)('n');
                            message = make_lit_string("(N)o");
                            if (gui_do_fixed_option(target, id, message, 'n')){
                                action = 1;
                            }
                            
                            if (action != -1){
                                complete = 1;
                                copy_ss(&comp_dest, view->transient.dest);
                                comp_action = action;
                            }
                        }break;
                        
                        case IInt_Sure_To_Kill:
                        {
                            i32 action = -1;
                            
                            String empty_str = {0};
                            String message = make_lit_string("There are unsaved changes, close anyway?");
                            
                            gui_do_text_field(target, message, empty_str);
                            
                            id.id[0] = (u64)('y');
                            message = make_lit_string("(Y)es");
                            if (gui_do_fixed_option(target, id, message, 'y')){
                                action = 0;
                            }
                            
                            id.id[0] = (u64)('n');
                            message = make_lit_string("(N)o");
                            if (gui_do_fixed_option(target, id, message, 'n')){
                                action = 1;
                            }
                            
                            id.id[0] = (u64)('s');
                            message = make_lit_string("(S)ave and kill");
                            if (gui_do_fixed_option(target, id, message, 's')){
                                action = 2;
                            }
                            
                            if (action != -1){
                                complete = 1;
                                copy_ss(&comp_dest, view->transient.dest);
                                comp_action = action;
                            }
                        }break;
                    }
                    
                    if (complete){
                        terminate_with_null(&comp_dest);
                        interactive_view_complete(system, view, models, comp_dest, comp_action);
                    }
                }break;
            }
        }
    }
    gui_end_top_level(target);
    
    result.animating = target->animating;
    return(result);
}

internal f32
view_get_scroll_y(View *view){
    f32 v = 0;
    if (view->transient.showing_ui == VUI_None){
        File_Edit_Positions *edit_pos = view->transient.edit_pos;
        TentativeAssert(edit_pos != 0);
        if (edit_pos != 0){
            v = edit_pos->scroll.scroll_y;
        }
    }
    else{
        v = view->transient.gui_scroll.scroll_y;
    }
    return(v);
}

internal b32
click_button_input(GUI_Target *target, GUI_Session *session, b32 in_scroll, i32_Rect scroll_rect, Input_Summary *user_input, GUI_Interactive *b, b32 *is_animating){
    b32 result = 0;
    i32 mx = user_input->mouse.x;
    i32 my = user_input->mouse.y;
    
    b32 in_sub_region = true;
    if (in_scroll && !hit_check(mx, my, scroll_rect)){
        in_sub_region = false;
    }
    
    if (in_sub_region && hit_check(mx, my, session->rect)){
        target->hover = b->id;
        if (user_input->mouse.press_l){
            target->mouse_hot = b->id;
            *is_animating = 1;
            result = 1;
        }
        if (user_input->mouse.release_l && gui_id_eq(target->mouse_hot, b->id)){
            target->active = b->id;
            target->mouse_hot = gui_id_zero();
            *is_animating = 1;
        }
    }
    else if (gui_id_eq(target->hover, b->id)){
        target->hover = gui_id_zero();
    }
    
    return(result);
}

internal b32
scroll_button_input(GUI_Target *target, GUI_Session *session, Input_Summary *user_input, GUI_id id, b32 *is_animating){
    b32 result = 0;
    i32 mx = user_input->mouse.x;
    i32 my = user_input->mouse.y;
    
    if (hit_check(mx, my, session->rect)){
        target->hover = id;
        if (user_input->mouse.l){
            target->mouse_hot = id;
            gui_activate_scrolling(target);
            *is_animating = 1;
            result = 1;
        }
    }
    else if (gui_id_eq(target->hover, id)){
        target->hover = gui_id_zero();
    }
    return(result);
}

static u32
to_writable_character(Key_Code long_character, u8 *character){
    u32 result = 0;
    if (long_character != 0){
        u32_to_utf8_unchecked(long_character, character, &result);
    }
    return(result);
}

internal Input_Process_Result
do_step_file_view(System_Functions *system, View *view, Models *models, i32_Rect rect, b32 is_active, Input_Summary *user_input, GUI_Scroll_Vars vars, i32_Rect region, i32 max_y){
    Input_Process_Result result = {0};
    b32 is_file_scroll = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *target = &view->transient.gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    vars.target_y = clamp(0, vars.target_y, max_y);
    
    result.vars = vars;
    result.region = region;
    
    target->active = gui_id_zero();
    
    // HACK(allen): UI sucks!  Now just forcing it to 
    // not have the bug where it clicks buttons behind the 
    // header buttons before the scrollable section.
    b32 in_scroll = false;
    i32_Rect scroll_rect = {0};
    i32 prev_bottom = 0;
    
    if (target->push.pos > 0){
        gui_session_init(&gui_session, target, rect, view->transient.line_height);
        
        for (h = (GUI_Header*)target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(target, &gui_session, h,
                                             result.vars, result.region, max_y);
            
            if (interpret_result.has_region){
                result.region = interpret_result.region;
            }
            
            switch (h->type){
                case guicom_file_option:
                case guicom_fixed_option:
                case guicom_fixed_option_checkbox:
                {
                    GUI_Interactive *b = (GUI_Interactive*)h;
                    
                    if (interpret_result.auto_activate){
                        target->auto_hot = gui_id_zero();
                        target->active = b->id;
                        result.is_animating = 1;
                    }
                    else if (interpret_result.auto_hot){
                        if (!gui_id_eq(target->auto_hot, b->id)){
                            target->auto_hot = b->id;
                            result.is_animating = 1;
                        }
                    }
                }break;
            }
            
            if (interpret_result.has_info){
                switch (h->type){
                    case guicom_top_bar: break;
                    
                    case guicom_file:
                    {
                        // NOTE(allen): Set the file region first because the
                        // computation of view_compute_max_target_y depends on it.
                        view->transient.file_region = gui_session.rect;
                        
                        Editing_File *file = view->transient.file_data.file;
                        Assert(file != 0);
                        
                        if (view->transient.reinit_scrolling){
                            view->transient.reinit_scrolling = false;
                            result.is_animating = true;
                            
                            i32 target_x = 0;
                            i32 target_y = 0;
                            if (file_is_ready(file)){
                                Vec2 cursor = view_get_cursor_xy(view);
                                
                                f32 w = view_width(view);
                                f32 h = view_height(view);
                                
                                if (cursor.x >= target_x + w){
                                    target_x = round32(cursor.x - w*.35f);
                                }
                                
                                target_y = clamp_bottom(0, floor32(cursor.y - h*.5f));
                            }
                            
                            result.vars.target_y = target_y;
                            result.vars.scroll_y = (f32)target_y;
                            result.vars.prev_target_y = -1000;
                            
                            result.vars.target_x = target_x;
                            result.vars.scroll_x = (f32)target_x;
                            result.vars.prev_target_x = -1000;
                        }
                        
                        if (!file->is_loading && file->state.paste_effect.seconds_down > 0.f){
                            file->state.paste_effect.seconds_down -= user_input->dt;
                            result.is_animating = true;
                        }
                        
                        is_file_scroll = true;
                    }break;
                    
                    case guicom_color_button:
                    case guicom_font_button:
                    case guicom_button:
                    case guicom_file_option:
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, in_scroll, scroll_rect, user_input, b, &result.is_animating)){
                            result.consumed_l = true;
                        }
                        
                        prev_bottom = gui_session.rect.y1;
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, in_scroll, scroll_rect, user_input, b, &result.is_animating)){
                            result.consumed_l = true;
                        }
                        
                        Key_Input_Data *keys = &user_input->keys;
                        
                        void *ptr = (b + 1);
                        String string = gui_read_string(&ptr);
                        AllowLocal(string);
                        
                        char activation_key = *(char*)ptr;
                        activation_key = char_to_upper(activation_key);
                        
                        if (activation_key != 0){
                            i32 count = keys->count;
                            for (i32 i = 0; i < count; ++i){
                                Key_Event_Data key = keys->keys[i];
                                
                                u8 character[4];
                                u32 length = to_writable_character(key.character, character);
                                if (length == 1){
                                    if (char_to_upper(character[0]) == activation_key){
                                        target->active = b->id;
                                        result.is_animating = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }break;
                    
                    case guicom_scrollable_slider:
                    {
                        GUI_id id = gui_id_scrollbar_slider();
                        i32 mx = user_input->mouse.x;
                        i32 my = user_input->mouse.y;
                        f32 v = 0;
                        
                        if (hit_check(mx, my, gui_session.rect)){
                            target->hover = id;
                            if (user_input->mouse.press_l){
                                target->mouse_hot = id;
                                result.is_animating = 1;
                                result.consumed_l = 1;
                            }
                        }
                        else if (gui_id_eq(target->hover, id)){
                            target->hover = gui_id_zero();
                        }
                        
                        if (gui_id_eq(target->mouse_hot, id)){
                            v = unlerp(gui_session.scroll_top, (f32)my,
                                       gui_session.scroll_bottom);
                            v = clamp(0.f, v, 1.f);
                            result.vars.target_y = round32(lerp(0.f, v, (f32)max_y));
                            
                            gui_activate_scrolling(target);
                            result.is_animating = 1;
                        }
                    }
                    // NOTE(allen): NO BREAK HERE!!
                    
                    case guicom_scrollable_invisible:
                    {
                        if (user_input->mouse.wheel != 0){
                            result.vars.target_y += user_input->mouse.wheel;
                            
                            result.vars.target_y =
                                clamp(0, result.vars.target_y, max_y);
                            gui_activate_scrolling(target);
                            result.is_animating = 1;
                        }
                    }break;
                    
                    case guicom_scrollable_top:
                    {
                        GUI_id id = gui_id_scrollbar_top();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y -= clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_bottom(0, result.vars.target_y);
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id = gui_id_scrollbar_bottom();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y += clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_top(result.vars.target_y, max_y);
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_begin_scrollable_section:
                    {
                        in_scroll = true;
                        scroll_rect.x0 = region.x0;
                        scroll_rect.y0 = prev_bottom;
                        scroll_rect.x1 = region.x1;
                        scroll_rect.y1 = region.y1;
                    }break;
                    
                    case guicom_end_scrollable_section:
                    {
                        in_scroll = false;
                        if (!is_file_scroll){
                            result.has_max_y_suggestion = 1;
                            result.max_y = gui_session.suggested_max_y;
                        }
                    }break;
                }
            }
        }
        
        if (!user_input->mouse.l){
            if (!gui_id_is_null(target->mouse_hot)){
                target->mouse_hot = gui_id_zero();
                result.is_animating = 1;
            }
        }
        
        {
            GUI_Scroll_Vars scroll_vars = result.vars;
            b32 is_new_target = 0;
            if (scroll_vars.target_x != scroll_vars.prev_target_x ||
                scroll_vars.target_y != scroll_vars.prev_target_y){
                is_new_target = 1;
            }
            
            f32 target_x = (f32)scroll_vars.target_x;
            f32 target_y = (f32)scroll_vars.target_y;
            
            if (models->scroll_rule(target_x, target_y, &scroll_vars.scroll_x, &scroll_vars.scroll_y, (view->persistent.id) + 1, is_new_target, user_input->dt)){
                result.is_animating = true;
            }
            
            scroll_vars.prev_target_x = scroll_vars.target_x;
            scroll_vars.prev_target_y = scroll_vars.target_y;
            
            result.vars = scroll_vars;
        }
    }
    
    return(result);
}

internal u32*
style_get_color(Style *style, Cpp_Token token){
    u32 *result = 0;
    if ((token.flags & CPP_TFLAG_IS_KEYWORD) != 0){
        if (token.type == CPP_TOKEN_BOOLEAN_CONSTANT){
            result = &style->main.bool_constant_color;
        }
        else{
            result = &style->main.keyword_color;
        }
    }
    else if ((token.flags & CPP_TFLAG_PP_DIRECTIVE) != 0){
        result = &style->main.preproc_color;
    }
    else{
        switch (token.type){
            case CPP_TOKEN_COMMENT:
            {
                result = &style->main.comment_color;
            }break;
            
            case CPP_TOKEN_STRING_CONSTANT:
            {
                result = &style->main.str_constant_color;
            }break;
            
            case CPP_TOKEN_CHARACTER_CONSTANT:
            {
                result = &style->main.char_constant_color;
            }break;
            
            case CPP_TOKEN_INTEGER_CONSTANT:
            {
                result = &style->main.int_constant_color;
            }break;
            
            case CPP_TOKEN_FLOATING_CONSTANT:
            {
                result = &style->main.float_constant_color;
            }break;
            
            case CPP_PP_INCLUDE_FILE:
            {
                result = &style->main.include_color;
            }break;
            
            default:
            {
                result = &style->main.default_color;
            }break;
        }
    }
    Assert(result != 0);
    return(result);
}

internal i32
draw_file_loaded(System_Functions *system, View *view, Models *models, i32_Rect rect, b32 is_active, Render_Target *target){
    Editing_File *file = view->transient.file_data.file;
    Style *style = &models->styles.styles[0];
    i32 line_height = view->transient.line_height;
    
    f32 max_x = (f32)file->settings.display_width;
    i32 max_y = rect.y1 - rect.y0 + line_height;
    
    Assert(file != 0);
    Assert(!file->is_dummy);
    Assert(buffer_good(&file->state.buffer));
    Assert(view->transient.edit_pos != 0);
    
    b32 tokens_use = 0;
    Cpp_Token_Array token_array = {};
    if (file){
        tokens_use = file->state.tokens_complete && (file->state.token_array.count > 0);
        token_array = file->state.token_array;
    }
    
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    
    partition_align(part, 4);
    
    f32 left_side_space = 0;
    
    i32 max = partition_remaining(part) / sizeof(Buffer_Render_Item);
    Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, max);
    
    Face_ID font_id = file->settings.font_id;
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    
    f32 scroll_x = view->transient.edit_pos->scroll.scroll_x;
    f32 scroll_y = view->transient.edit_pos->scroll.scroll_y;
    
    // NOTE(allen): For now we will temporarily adjust scroll_y to try
    // to prevent the view moving around until floating sections are added
    // to the gui system.
    scroll_y += view->transient.widget_height;
    
    Full_Cursor render_cursor;
    if (!file->settings.unwrapped_lines){
        render_cursor = file_compute_cursor(system, file, seek_wrapped_xy(0, scroll_y, 0), true);
    }
    else{
        render_cursor = file_compute_cursor(system, file, seek_unwrapped_xy(0, scroll_y, 0), true);
    }
    
    view->transient.edit_pos->scroll_i = render_cursor.pos;
    
    i32 count = 0;
    {
        b32 wrapped = !file->settings.unwrapped_lines;
        
        Buffer_Render_Params params;
        params.buffer        = &file->state.buffer;
        params.items         = items;
        params.max           = max;
        params.count         = &count;
        params.port_x        = (f32)rect.x0 + left_side_space;
        params.port_y        = (f32)rect.y0;
        params.clip_w        = view_width(view) - left_side_space;
        params.scroll_x      = scroll_x;
        params.scroll_y      = scroll_y;
        params.width         = max_x;
        params.height        = (f32)max_y;
        params.start_cursor  = render_cursor;
        params.wrapped       = wrapped;
        params.system        = system;
        params.font          = font;
        params.virtual_white = file->settings.virtual_white;
        params.wrap_slashes  = file->settings.wrap_indicator;
        
        Buffer_Render_State state = {0};
        Buffer_Layout_Stop stop = {0};
        
        f32 line_shift = 0.f;
        b32 do_wrap = 0;
        i32 wrap_unit_end = 0;
        
        b32 first_wrap_determination = 1;
        i32 wrap_array_index = 0;
        
        do{
            stop = buffer_render_data(&state, params, line_shift, do_wrap, wrap_unit_end);
            switch (stop.status){
                case BLStatus_NeedWrapDetermination:
                {
                    if (first_wrap_determination){
                        wrap_array_index = binary_search(file->state.wrap_positions, stop.pos, 0, file->state.wrap_position_count);
                        ++wrap_array_index;
                        if (file->state.wrap_positions[wrap_array_index] == stop.pos){
                            do_wrap = 1;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        else{
                            do_wrap = 0;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        first_wrap_determination = 0;
                    }
                    else{
                        Assert(stop.pos == wrap_unit_end);
                        do_wrap = 1;
                        ++wrap_array_index;
                        wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                    }
                }break;
                
                case BLStatus_NeedWrapLineShift:
                case BLStatus_NeedLineShift:
                {
                    line_shift = file->state.line_indents[stop.wrap_line_index];
                }break;
            }
        }while(stop.status != BLStatus_Finished);
    }
    
    i32 cursor_begin = 0, cursor_end = 0;
    u32 cursor_color = 0, at_cursor_color = 0;
    if (view->transient.file_data.show_temp_highlight){
        cursor_begin = view->transient.file_data.temp_highlight.pos;
        cursor_end = view->transient.file_data.temp_highlight_end_pos;
        cursor_color = style->main.highlight_color;
        at_cursor_color = style->main.at_highlight_color;
    }
    else{
        cursor_begin = view->transient.edit_pos->cursor.pos;
        cursor_end = cursor_begin + 1;
        cursor_color = style->main.cursor_color;
        at_cursor_color = style->main.at_cursor_color;
    }
    
    i32 token_i = 0;
    u32 main_color = style->main.default_color;
    u32 special_color = style->main.special_character_color;
    u32 ghost_color = style->main.ghost_character_color;
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(token_array, items->index);
        main_color = *style_get_color(style, token_array.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    u32 mark_color = style->main.mark_color;
    Buffer_Render_Item *item = items;
    Buffer_Render_Item *item_end = item + count;
    i32 prev_ind = -1;
    u32 highlight_color = 0;
    u32 highlight_this_color = 0;
    
    for (; item < item_end; ++item){
        i32 ind = item->index;
        highlight_this_color = 0;
        if (tokens_use && ind != prev_ind){
            Cpp_Token current_token = token_array.tokens[token_i-1];
            
            if (token_i < token_array.count){
                if (ind >= token_array.tokens[token_i].start){
                    for (;token_i < token_array.count && ind >= token_array.tokens[token_i].start; ++token_i){
                        main_color = *style_get_color(style, token_array.tokens[token_i]);
                        current_token = token_array.tokens[token_i];
                    }
                }
                else if (ind >= current_token.start + current_token.size){
                    main_color = style->main.default_color;
                }
            }
            
            if (current_token.type == CPP_TOKEN_JUNK && ind >= current_token.start && ind < current_token.start + current_token.size){
                highlight_color = style->main.highlight_junk_color;
            }
            else{
                highlight_color = 0;
            }
        }
        
        u32 char_color = main_color;
        if (item->flags & BRFlag_Special_Character){
            char_color = special_color;
        }
        else if (item->flags & BRFlag_Ghost_Character){
            char_color = ghost_color;
        }
        
        f32_Rect char_rect = f32R(item->x0, item->y0, item->x1,  item->y1);
        
        if (view->transient.file_data.show_whitespace && highlight_color == 0 && codepoint_is_whitespace(item->codepoint)){
            highlight_this_color = style->main.highlight_white_color;
        }
        else{
            highlight_this_color = highlight_color;
        }
        
        if (cursor_begin <= ind && ind < cursor_end && (ind != prev_ind || cursor_begin < ind)){
            if (is_active){
                draw_rectangle(target, char_rect, cursor_color);
                char_color = at_cursor_color;
            }
            else{
                if (!view->transient.file_data.show_temp_highlight){
                    draw_rectangle_outline(target, char_rect, cursor_color);
                }
            }
        }
        else if (highlight_this_color){
            draw_rectangle(target, char_rect, highlight_this_color);
        }
        
        u32 fade_color = 0xFFFF00FF;
        f32 fade_amount = 0.f;
        
        if (file->state.paste_effect.seconds_down > 0.f &&
            file->state.paste_effect.start <= ind &&
            ind < file->state.paste_effect.end){
            fade_color = file->state.paste_effect.color;
            fade_amount = file->state.paste_effect.seconds_down;
            fade_amount /= file->state.paste_effect.seconds_max;
        }
        
        char_color = color_blend(char_color, fade_amount, fade_color);
        
        if (ind == view->transient.edit_pos->mark && prev_ind != ind){
            draw_rectangle_outline(target, char_rect, mark_color);
        }
        if (item->codepoint != 0){
            draw_font_glyph(target, font_id, item->codepoint, item->x0, item->y0, char_color);
        }
        prev_ind = ind;
    }
    
    end_temp_memory(temp);
    
    return(0);
}

internal void
draw_text_field(System_Functions *system, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, String p, String t){
    Style *style = &models->styles.styles[0];
    
    u32 back_color = style->main.margin_color;
    u32 text1_color = style->main.default_color;
    u32 text2_color = style->main.file_info_style.pop1_color;
    
    i32 x = rect.x0;
    i32 y = rect.y0 + 2;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        x = ceil32(draw_string(system, target, font_id, p, x, y, text2_color));
        draw_string(system, target, font_id, t, x, y, text1_color);
    }
}

internal void
draw_text_with_cursor(System_Functions *system, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, String s, i32 pos){
    Style *style = &models->styles.styles[0];
    
    u32 back_color = style->main.margin_color;
    u32 text_color = style->main.default_color;
    u32 cursor_color = style->main.cursor_color;
    u32 at_cursor_color = style->main.at_cursor_color;
    
    f32 x = (f32)rect.x0;
    i32 y = rect.y0 + 2;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        
        if (pos >= 0 && pos <  s.size){
            Font_Pointers font = system->font.get_pointers_by_id(font_id);
            
            String part1 = substr(s, 0, pos);
            String part2 = substr(s, pos, 1);
            String part3 = substr(s, pos+1, s.size-pos-1);
            
            x = draw_string(system, target, font_id, part1, floor32(x), y, text_color);
            
            f32 adv = font_get_glyph_advance(system, font.settings, font.metrics, font.pages, s.str[pos]);
            i32_Rect cursor_rect;
            cursor_rect.x0 = floor32(x);
            cursor_rect.x1 = floor32(x) + ceil32(adv);
            cursor_rect.y0 = y;
            cursor_rect.y1 = y + view->transient.line_height;
            draw_rectangle(target, cursor_rect, cursor_color);
            x = draw_string(system, target, font_id, part2, floor32(x), y, at_cursor_color);
            
            draw_string(system, target, font_id, part3, floor32(x), y, text_color);
        }
        else{
            draw_string(system, target, font_id, s, floor32(x), y, text_color);
        }
    }
}

internal void
intbar_draw_string(System_Functions *system, Render_Target *target, File_Bar *bar, String str, u32 char_color){
    draw_string(system, target, bar->font_id, str, (i32)(bar->pos_x + bar->text_shift_x), (i32)(bar->pos_y + bar->text_shift_y), char_color);
    bar->pos_x += font_string_width(system, target, bar->font_id, str);
}

internal void
draw_file_bar(System_Functions *system, Render_Target *target, View *view, Models *models, Editing_File *file, i32_Rect rect){
    File_Bar bar;
    Style *style = &models->styles.styles[0];
    Interactive_Style bar_style = style->main.file_info_style;
    
    u32 back_color = bar_style.bar_color;
    u32 base_color = bar_style.base_color;
    u32 pop1_color = bar_style.pop1_color;
    u32 pop2_color = bar_style.pop2_color;
    
    bar.rect = rect;
    
    if (target){
        bar.font_id = file->settings.font_id;
        bar.pos_x = (f32)bar.rect.x0;
        bar.pos_y = (f32)bar.rect.y0;
        bar.text_shift_y = 2;
        bar.text_shift_x = 0;
        
        draw_rectangle(target, bar.rect, back_color);
        
        Assert(file);
        
        intbar_draw_string(system, target, &bar, file->unique_name.name, base_color);
        intbar_draw_string(system, target, &bar, make_lit_string(" -"), base_color);
        
        if (file->is_loading){
            intbar_draw_string(system, target, &bar, make_lit_string(" loading"), base_color);
        }
        else{
            char bar_space[526];
            String bar_text = make_fixed_width_string(bar_space);
            append_ss        (&bar_text, make_lit_string(" L#"));
            append_int_to_str(&bar_text, view->transient.edit_pos->cursor.line);
            append_ss        (&bar_text, make_lit_string(" C#"));
            append_int_to_str(&bar_text, view->transient.edit_pos->cursor.character);
            
            append_ss(&bar_text, make_lit_string(" -"));
            
            if (file->settings.dos_write_mode){
                append_ss(&bar_text, make_lit_string(" dos"));
            }
            else{
                append_ss(&bar_text, make_lit_string(" nix"));
            }
            
            intbar_draw_string(system, target, &bar, bar_text, base_color);
            
            
            if (file->state.still_lexing){
                intbar_draw_string(system, target, &bar, make_lit_string(" parsing"), pop1_color);
            }
            
            if (!file->settings.unimportant){
                switch (file->state.dirty){
                    case DirtyState_UnloadedChanges:
                    {
                        local_persist String out_of_sync = make_lit_string(" !");
                        intbar_draw_string(system, target, &bar, out_of_sync, pop2_color);
                    }break;
                    
                    case DirtyState_UnsavedChanges:
                    {
                        local_persist String out_of_sync = make_lit_string(" *");
                        intbar_draw_string(system, target, &bar, out_of_sync, pop2_color);
                    }break;
                }
            }
        }
    }
}

u32
get_margin_color(i32 active_level, Style *style){
    u32 margin = 0xFFFFFFFF;
    
    switch (active_level){
        default:
        margin = style->main.list_item_color;
        break;
        
        case 1: case 2:
        margin = style->main.list_item_hover_color;
        break;
        
        case 3: case 4:
        margin = style->main.list_item_active_color;
        break;
    }
    
    return(margin);
}

internal void
draw_color_button(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Face_ID font_id, i32_Rect rect, GUI_id id, u32 fore, u32 back, String text){
    i32 active_level = gui_active_level(gui_target, id);
    
    if (active_level > 0){
        Swap(u32, back, fore);
    }
    
    draw_rectangle(target, rect, back);
    draw_string(system, target, font_id, text, rect.x0, rect.y0 + 1, fore);
}

internal void
draw_font_button(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, i32_Rect rect, GUI_id id, Face_ID font_id, String text){
    Style *style = &models->styles.styles[0];
    
    i32 active_level = gui_active_level(gui_target, id);
    
    u32 margin_color = get_margin_color(active_level, style);
    u32 back_color = style->main.back_color;
    u32 text_color = style->main.default_color;
    
    draw_rectangle(target, rect, back_color);
    draw_rectangle_outline(target, rect, margin_color);
    draw_string(system, target, font_id, text, rect.x0, rect.y0 + 1, text_color);
}

internal void
draw_fat_option_block(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, GUI_id id, String text, String pop, i8 checkbox = -1){
    Style *style = &models->styles.styles[0];
    
    i32 active_level = gui_active_level(gui_target, id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 pop_color = style->main.file_info_style.pop2_color;
    
    i32 h = view->transient.line_height;
    i32 x = inner.x0 + 3;
    i32 y = inner.y0 + h/2 - 1;
    
    draw_rectangle(target, inner, back);
    draw_margin(target, rect, inner, margin);
    
    if (checkbox != -1){
        u32 checkbox_color = style->main.margin_active_color;
        i32_Rect checkbox_rect = get_inner_rect(inner, (inner.y1 - inner.y0 - h)/2);
        checkbox_rect.x1 = checkbox_rect.x0 + (checkbox_rect.y1 - checkbox_rect.y0);
        
        if (checkbox == 0){
            draw_rectangle_outline(target, checkbox_rect, checkbox_color);
        }
        else{
            draw_rectangle(target, checkbox_rect, checkbox_color);
        }
        
        x = checkbox_rect.x1 + 3;
    }
    
    x = ceil32(draw_string(system, target, font_id, text, x, y, text_color));
    draw_string(system, target, font_id, pop, x, y, pop_color);
}

internal void
draw_button(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, GUI_id id, String text){
    Style *style = &models->styles.styles[0];
    
    i32 active_level = gui_active_level(gui_target, id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = style->main.default_color;
    u32 back = get_margin_color(active_level, style);
    u32 text_color = style->main.default_color;
    
    i32 h = view->transient.line_height;
    i32 y = inner.y0 + h/2 - 1;
    
    i32 w = (i32)font_string_width(system, target, font_id, text);
    i32 x = (inner.x1 + inner.x0 - w)/2;
    
    draw_rectangle(target, inner, back);
    draw_rectangle_outline(target, inner, margin);
    
    draw_string(system, target, font_id, text, x, y, text_color);
}

internal void
draw_style_preview(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, GUI_id id, Style *style){
    i32 active_level = gui_active_level(gui_target, id);
    char font_name_space[256];
    String font_name = make_fixed_width_string(font_name_space);
    font_name.size = system->font.get_name_by_id(font_id, font_name.str, font_name.memory_size);
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin_color = get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 keyword_color = style->main.keyword_color;
    u32 int_constant_color = style->main.int_constant_color;
    u32 comment_color = style->main.comment_color;
    
    draw_margin(target, rect, inner, margin_color);
    draw_rectangle(target, inner, back);
    
    i32 y = inner.y0;
    i32 x = inner.x0;
    x = ceil32(draw_string(system, target, font_id, style->name.str, x, y, text_color));
    i32 font_x = (i32)(inner.x1 - font_string_width(system, target, font_id, font_name));
    if (font_x > x + 10){
        draw_string(system, target, font_id, font_name, font_x, y, text_color);
    }
    
    i32 height = font.metrics->height;
    x = inner.x0;
    y += height;
    x = ceil32(draw_string(system, target, font_id, "if", x, y, keyword_color));
    x = ceil32(draw_string(system, target, font_id, "(x < ", x, y, text_color));
    x = ceil32(draw_string(system, target, font_id, "0", x, y, int_constant_color));
    x = ceil32(draw_string(system, target, font_id, ") { x = ", x, y, text_color));
    x = ceil32(draw_string(system, target, font_id, "0", x, y, int_constant_color));
    x = ceil32(draw_string(system, target, font_id, "; } ", x, y, text_color));
    x = ceil32(draw_string(system, target, font_id, "// comment", x, y, comment_color));
    
    x = inner.x0;
    y += height;
    draw_string(system, target, font_id, "[] () {}; * -> +-/ <>= ! && || % ^", x, y, text_color);
}

internal i32
do_render_file_view(System_Functions *system, View *view, Models *models, GUI_Scroll_Vars *scroll, View *active, i32_Rect rect, b32 is_active, Render_Target *target, Input_Summary *user_input){
    
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    
    i32 result = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *gui_target = &view->transient.gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    f32 v = {0};
    i32 max_y = view_compute_max_target_y(view);
    
    Face_ID font_id = file->settings.font_id;
    if (gui_target->push.pos > 0){
        gui_session_init(&gui_session, gui_target, rect, view->transient.line_height);
        
        v = view_get_scroll_y(view);
        
        i32_Rect clip_rect = rect;
        draw_push_clip(target, clip_rect);
        
        for (h = (GUI_Header*)gui_target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(gui_target, &gui_session, h,
                                             *scroll, view->transient.scroll_region, max_y);
            
            if (interpret_result.has_info){
                if (gui_session.clip_y > clip_rect.y0){
                    clip_rect.y0 = gui_session.clip_y;
                    draw_change_clip(target, clip_rect);
                }
                
                switch (h->type){
                    case guicom_top_bar:
                    {
                        draw_file_bar(system, target, view, models, file, gui_session.rect);
                    }break;
                    
                    case guicom_file:
                    {
                        if (file_is_ready(file)){
                            result = draw_file_loaded(system, view, models, gui_session.rect, is_active, target);
                        }
                    }break;
                    
                    case guicom_text_field:
                    {
                        void *ptr = (h+1);
                        String p = gui_read_string(&ptr);
                        String t = gui_read_string(&ptr);
                        draw_text_field(system, target, view, models, font_id, gui_session.rect, p, t);
                    }break;
                    
                    case guicom_text_with_cursor:
                    {
                        void *ptr = (h+1);
                        String s = gui_read_string(&ptr);
                        i32 pos = gui_read_integer(&ptr);
                        
                        draw_text_with_cursor(system, target, view, models, font_id, gui_session.rect, s, pos);
                    }break;
                    
                    case guicom_color_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        u32 fore = (u32)gui_read_integer(&ptr);
                        u32 back = (u32)gui_read_integer(&ptr);
                        String t = gui_read_string(&ptr);
                        
                        draw_color_button(system, gui_target, target, view, font_id, gui_session.rect, b->id, fore, back, t);
                    }break;
                    
                    case guicom_font_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        Face_ID this_font_id = (Face_ID)gui_read_integer(&ptr);
                        String t = gui_read_string(&ptr);
                        
                        draw_font_button(system, gui_target, target, view, models, gui_session.rect, b->id, this_font_id, t);
                    }break;
                    
                    case guicom_file_option:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        b32 folder = gui_read_integer(&ptr);
                        String f = gui_read_string(&ptr);
                        String m = gui_read_string(&ptr);
                        
                        if (folder){
                            append_s_char(&f, '/');
                        }
                        
                        draw_fat_option_block(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, f, m);
                    }break;
                    
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        i32 style_index = *(i32*)(b + 1);
                        Style *style = &models->styles.styles[style_index];
                        
                        draw_style_preview(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, style);
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String f = gui_read_string(&ptr);
                        String m = {0};
                        i8 status = -1;
                        if (h->type == guicom_fixed_option_checkbox){
                            gui_read_byte(&ptr);
                            status = (i8)gui_read_byte(&ptr);
                        }
                        
                        draw_fat_option_block(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, f, m, status);
                    }break;
                    
                    case guicom_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String t = gui_read_string(&ptr);
                        
                        draw_button(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, t);
                    }break;
                    
                    case guicom_scrollable_bar:
                    {
                        Style *style = &models->styles.styles[0];
                        
                        u32 back;
                        u32 outline;
                        
                        i32_Rect bar = gui_session.rect;
                        
                        back = style->main.back_color;
                        if (is_active){
                            outline = style->main.margin_active_color;
                        }
                        else{
                            outline = style->main.margin_color;
                        }
                        
                        draw_rectangle(target, bar, back);
                        draw_rectangle_outline(target, bar, outline);
                    }break;
                    
                    case guicom_scrollable_top:
                    case guicom_scrollable_slider:
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id;
                        Style *style = &models->styles.styles[0];
                        i32_Rect box = gui_session.rect;
                        
                        i32 active_level;
                        
                        u32 back;
                        u32 outline;
                        
                        switch (h->type){
                            case guicom_scrollable_top: id = gui_id_scrollbar_top(); break;
                            case guicom_scrollable_bottom: id = gui_id_scrollbar_bottom(); break;
                            default: id = gui_id_scrollbar_slider(); break;
                        }
                        
                        active_level = gui_active_level(gui_target, id);
                        
                        switch (active_level){
                            case 0: back = style->main.back_color; break;
                            case 1: back = style->main.margin_hover_color; break;
                            default: back = style->main.margin_active_color; break;
                        }
                        
                        if (is_active){
                            outline = style->main.margin_active_color;
                        }
                        else{
                            outline = style->main.margin_color;
                        }
                        
                        draw_rectangle(target, box, back);
                        draw_margin(target, box, get_inner_rect(box, 2), outline);
                    }break;
                    
                    case guicom_begin_scrollable_section:
                    clip_rect.x1 = Min(gui_session.scroll_region.x1, clip_rect.x1);
                    draw_push_clip(target, clip_rect);
                    break;
                    
                    case guicom_end_scrollable_section:
                    clip_rect = draw_pop_clip(target);
                    break;
                }
            }
        }
        
        draw_pop_clip(target);
    }
    
    return(result);
}

inline void
file_view_free_buffers(View *view, Models *models){
    General_Memory *general = &models->mem.general;
    general_memory_free(general, view->transient.gui_mem);
    view->transient.gui_mem = 0;
}

internal View_And_ID
live_set_alloc_view(Live_Views *live_set, Panel *panel, Models *models){
    View_And_ID result = {};
    
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    result.view = live_set->free_sentinel.transient.next;
    result.id = (i32)(result.view - live_set->views);
    Assert(result.id == result.view->persistent.id);
    
    //dll_remove(&result.view->transient));
    result.view->transient.next->transient.prev = result.view->transient.prev;
    result.view->transient.prev->transient.next = result.view->transient.next;
    memset(&result.view->transient, 0, sizeof(result.view->transient));
    
    result.view->transient.in_use = true;
    panel->view = result.view;
    result.view->transient.panel = panel;
    
    init_query_set(&result.view->transient.query_set);
    
    i32 gui_mem_size = KB(512);
    void *gui_mem = general_memory_allocate(&models->mem.general, gui_mem_size + 8);
    result.view->transient.gui_mem = gui_mem;
    gui_mem = advance_to_alignment(gui_mem);
    result.view->transient.gui_target.push = make_part(gui_mem, gui_mem_size);
    
    return(result);
}

inline void
live_set_free_view(Live_Views *live_set, View *view, Models *models){
    Assert(live_set->count > 0);
    --live_set->count;
    file_view_free_buffers(view, models);
    //dll_insert(&live_set->free_sentinel, view);
    view->transient.next = live_set->free_sentinel.transient.next;
    view->transient.prev = &live_set->free_sentinel;
    live_set->free_sentinel.transient.next = view;
    view->transient.next->transient.prev = view;
    view->transient.in_use = false;
}

// BOTTOM

