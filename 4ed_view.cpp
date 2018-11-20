/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Viewing
 *
 */

// TOP

internal i32
view_get_map(View *view){
    if (view->transient.ui_mode){
        return(view->transient.ui_map_id);
    }
    else{
        return(view->transient.file_data.file->settings.base_map_id);
    }
}

internal View_And_ID
live_set_alloc_view(Heap *heap, Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, Panel *panel){
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    View_And_ID result = {};
    result.view = live_set->free_sentinel.transient.next;
    result.id = (i32)(result.view - live_set->views);
    Assert(result.id == result.view->persistent.id);
    
    result.view->transient.next->transient.prev = result.view->transient.prev;
    result.view->transient.prev->transient.next = result.view->transient.next;
    memset(&result.view->transient, 0, sizeof(result.view->transient));
    
    result.view->transient.in_use = true;
    panel->view = result.view;
    result.view->transient.panel = panel;
    
    init_query_set(&result.view->transient.query_set);
    
    result.view->transient.lifetime_object = lifetime_alloc_object(heap, lifetime_allocator, DynamicWorkspace_View, result.view);
    
    return(result);
}

inline void
live_set_free_view(Heap *heap, Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    
    if (view->transient.ui_control.items != 0){
        heap_free(heap, view->transient.ui_control.items);
    }
    
    view->transient.next = live_set->free_sentinel.transient.next;
    view->transient.prev = &live_set->free_sentinel;
    live_set->free_sentinel.transient.next = view;
    view->transient.next->transient.prev = view;
    view->transient.in_use = false;
    
    lifetime_free_object(heap, lifetime_allocator, view->transient.lifetime_object);
}

////////////////////////////////

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
    Cursor_Limits limits = {};
    
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
view_compute_max_target_y_from_bottom_y(View *view, f32 max_item_y){
    i32 line_height = view->transient.line_height;
    f32 height = clamp_bottom((f32)line_height, view_height(view));
    f32 max_target_y = clamp_bottom(0.f, max_item_y - height*0.5f);
    return(ceil32(max_target_y));
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
    return(view_compute_max_target_y_from_bottom_y(view, (lowest_line + 0.5f)*(f32)line_height));
}

inline u32
view_lock_flags(View *view){
    u32 result = AccessOpen;
    File_Viewing_Data *data = &view->transient.file_data;
    if (view->transient.ui_mode){
        result |= AccessHidden;
    }
    if (data->file_locked || (data->file && data->file->settings.read_only)){
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

internal Relative_Scrolling
view_get_relative_scrolling(View *view){
    Relative_Scrolling result = {};
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

inline void
view_cursor_move(System_Functions *system, View *view, i32 pos){
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    Full_Cursor cursor = file_compute_cursor(system, file, seek_pos(pos), 0);
    view_set_cursor(view, cursor, true, file->settings.unwrapped_lines);
    view->transient.file_data.show_temp_highlight = false;
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
view_post_paste_effect(View *view, f32 seconds, i32 start, i32 size, u32 color){
    Editing_File *file = view->transient.file_data.file;
    file->state.paste_effect.start = start;
    file->state.paste_effect.end = start + size;
    file->state.paste_effect.color = color;
    file->state.paste_effect.seconds_down = seconds;
    file->state.paste_effect.seconds_max = seconds;
}

////////////////////////////////

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

internal void
get_visual_markers(Partition *arena, Dynamic_Workspace *workspace,
                   Range range, Buffer_ID buffer_id, i32 view_index,
                   Style_Main_Data *style_data){
    View_ID view_id = view_index + 1;
    for (Managed_Buffer_Markers_Header *node = workspace->buffer_markers_list.first;
         node != 0;
         node = node->next){
        if (node->buffer_id != buffer_id) continue;
        for (Marker_Visual_Data *data = node->visual_first;
             data != 0;
             data = data->next){
            if (data->type == VisualType_Invisible) continue;
            if (data->key_view_id != 0 && data->key_view_id != view_id) continue;
            
            Marker_Visual_Type type = data->type;
            u32 color = data->color;
            u32 text_color = data->text_color;
            i32 take_count_per_step = data->take_rule.take_count_per_step;
            i32 step_stride_in_marker_count = data->take_rule.step_stride_in_marker_count;
            i32 stride_size_from_last = step_stride_in_marker_count - take_count_per_step;
            i32 priority = data->priority;
            
            if ((color&SymbolicColor__StagColorFlag) && (color&0xFF000000) == 0){
                u32 *c = style_index_by_tag(style_data, color&0x007FFFFF);
                if (c != 0){
                    color = *c;
                }
                else{
                    color = 0;
                }
            }
            
            if ((text_color&SymbolicColor__StagColorFlag) && (text_color&0xFF000000) == 0){
                u32 *c = style_index_by_tag(style_data, text_color&0x007FFFFF);
                if (c != 0){
                    text_color = *c;
                }
                else{
                    text_color = 0;
                }
            }
            
            Marker *markers = (Marker*)(node + 1);
            Assert(sizeof(*markers) == node->std_header.item_size);
            i32 count = node->std_header.count;
            i32 one_past_last_index = clamp_top(data->one_past_last_take_index, count);
            
            Marker *marker_one_past_last = markers + one_past_last_index;
            Marker *marker = markers + data->take_rule.first_index;
            
            switch (type){
                default:
                {
                    for (;marker < marker_one_past_last; marker += stride_size_from_last){
                        for (i32 i = 0;
                             i < take_count_per_step && marker < marker_one_past_last;
                             marker += 1, i += 1){
                            if (range.first <= marker->pos && marker->pos <= range.one_past_last){
                                Render_Marker *render_marker = push_array(arena, Render_Marker, 1);
                                render_marker->type = type;
                                render_marker->pos = marker->pos;
                                render_marker->color = color;
                                render_marker->text_color = text_color;
                                render_marker->one_past_last = marker->pos;
                                render_marker->priority = priority;
                            }
                        }
                    }
                }break;
                
                case VisualType_CharacterHighlightRanges:
                case VisualType_LineHighlightRanges:
                {
                    i32 pos_pair[2] = {};
                    i32 pair_index = 0;
                    
                    for (;marker < marker_one_past_last; marker += stride_size_from_last){
                        for (i32 i = 0;
                             i < take_count_per_step && marker < marker_one_past_last;
                             marker += 1, i += 1){
                            pos_pair[pair_index++] = marker->pos;
                            if (pair_index == 2){
                                pair_index = 0;
                                
                                Range range_b = {};
                                range_b.first = pos_pair[0];
                                range_b.one_past_last = pos_pair[1];
                                
                                if (range_b.first == range_b.one_past_last) continue;
                                if (range_b.first > range_b.one_past_last){
                                    Swap(i32, range_b.first, range_b.one_past_last);
                                }
                                if (!((range.min - range_b.max <= 0) &&
                                      (range.max - range_b.min >= 0))) continue;
                                
                                Render_Marker *render_marker = push_array(arena, Render_Marker, 1);
                                render_marker->type = type;
                                render_marker->pos = range_b.min;
                                render_marker->color = color;
                                render_marker->text_color = text_color;
                                render_marker->one_past_last = range_b.max;
                                render_marker->priority = priority;
                            }
                        }
                    }
                }break;
            }
        }
    }
}

internal i32
marker_type_to_segment_rank(Marker_Visual_Type type){
    switch (type){
        case VisualType_LineHighlights:
        {
            return(1);
        }break;
        case VisualType_CharacterHighlightRanges:
        {
            return(2);
        }break;
        case VisualType_LineHighlightRanges:
        {
            return(3);
        }break;
    }
    return(0);
}
global_const i32 max_visual_type_rank = 3;

internal i32
split_sort(Render_Marker *markers, i32 first, i32 one_past_last, i32 min_in_high_segment){
    i32 i1 = first;
    i32 i0 = one_past_last - 1;
    for (;;){
        for (;i1 < one_past_last &&
             marker_type_to_segment_rank(markers[i1].type) < min_in_high_segment;
             i1 += 1);
        for (;i0 >= 0 &&
             marker_type_to_segment_rank(markers[i0].type) >= min_in_high_segment;
             i0 -= 1);
        if (i1 < i0){
            Swap(Render_Marker, markers[i0], markers[i1]);
        }
        else{
            break;
        }
    }
    return(i1);
}

internal void
visual_markers_segment_sort(Render_Marker *markers, i32 first, i32 one_past_last, Range *ranges_out){
    i32 pos = first;
    for (i32 i = 0; i < max_visual_type_rank; i += 1){
        ranges_out[i].first = pos;
        pos = split_sort(markers, pos, one_past_last, i + 1);
        ranges_out[i].one_past_last = pos;
    }
    ranges_out[max_visual_type_rank].first = pos;
    ranges_out[max_visual_type_rank].one_past_last = one_past_last;
}

internal void
visual_markers_quick_sort(Render_Marker *markers, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot_index = one_past_last - 1;
        {
            b32 go_left = false;
            i32 pivot_key = markers[pivot_index].pos;
            i32 j = first;
            for (i32 i = first; i < pivot_index; i += 1){
                i32 key = markers[i].pos;
                b32 swap_in = false;
                if (key < pivot_key){
                    swap_in = true;
                }
                else if (key == pivot_key){
                    if (go_left){
                        swap_in = true;
                    }
                    go_left = !go_left;
                }
                if (swap_in){
                    Swap(Render_Marker, markers[i], markers[j]);
                    j += 1;
                }
            }
            Swap(Render_Marker, markers[j], markers[pivot_index]);
            pivot_index = j;
        }
        visual_markers_quick_sort(markers, first, pivot_index);
        visual_markers_quick_sort(markers, pivot_index + 1, one_past_last);
    }
}

internal void
visual_markers_replace_pos_with_first_byte_of_line(Render_Marker_Array markers, i32 *line_starts, i32 line_count, i32 hint_line_index){
    if (markers.count > 0){
        Assert(0 <= hint_line_index && hint_line_index < line_count);
        i32 marker_scan_index = 0;
        for (i32 line_scan_index = hint_line_index;; line_scan_index += 1){
            Assert(line_scan_index < line_count);
            i32 look_ahead_line_index = line_scan_index + 1;
            i32 first_byte_index_of_next_line = 0;
            if (look_ahead_line_index < line_count){
                first_byte_index_of_next_line = line_starts[look_ahead_line_index];
            }
            else{
                first_byte_index_of_next_line = max_i32;
            }
            i32 first_byte_index_of_this_line = line_starts[line_scan_index];
            for (;(marker_scan_index < markers.count &&
                   markers.markers[marker_scan_index].pos < first_byte_index_of_next_line);
                 marker_scan_index += 1){
                markers.markers[marker_scan_index].pos = first_byte_index_of_this_line;
            }
            if (marker_scan_index == markers.count){
                break;
            }
        }
    }
}

internal i32
range_record_stack_get_insert_index(Render_Range_Record *records, i32 count, i32 priority){
    i32 insert_pos = 0;
    i32 first = 0;
    i32 one_past_last = count + 1;
    for (;;){
        if (first + 1 >= one_past_last){
            insert_pos = first;
            break;
        }
        i32 mid = (first + one_past_last)/2;
        // i - 1 is too big?
        b32 too_big = false;
        if (mid > 0){
            if (records[mid - 1].priority > priority){
                too_big = true;
            }
        }
        // i is too small?
        b32 too_small = false;
        if (!too_big && mid < count){
            if (records[mid].priority <= priority){
                too_small = true;
            }
        }
        // bisect
        if (too_big){
            one_past_last = mid;
        }
        else if (too_small){
            first = mid + 1;
        }
        else{
            insert_pos = mid;
            break;
        }
    }
    return(insert_pos);
}

internal void
render_loaded_file_in_view__inner(Models *models, Render_Target *target, View *view,
                                  i32_Rect rect, Full_Cursor render_cursor, Range on_screen_range,
                                  Buffer_Render_Item *items, i32 item_count){
    Editing_File *file = view->transient.file_data.file;
    Partition *part = &models->mem.part;
    Style *style = &models->styles.styles[0];
    
    Assert(file != 0);
    Assert(!file->is_dummy);
    Assert(buffer_good(&file->state.buffer));
    Assert(view->transient.edit_pos != 0);
    
    b32 tokens_use = file->state.tokens_complete && (file->state.token_array.count > 0);
    Cpp_Token_Array token_array = file->state.token_array;
    b32 wrapped = !file->settings.unwrapped_lines;
    Face_ID font_id = file->settings.font_id;
    
    // NOTE(allen): Get visual markers
    Render_Marker_Array markers = {};
    markers.markers = push_array(part, Render_Marker, 0);
    {
        Lifetime_Object *lifetime_object = file->lifetime_object;
        Buffer_ID buffer_id = file->id.id;
        i32 view_index = view->persistent.id;
        Style_Main_Data *style_data = &style->main;
        
        get_visual_markers(part, &lifetime_object->workspace, on_screen_range, buffer_id, view_index, style_data);
        
        i32 key_count = lifetime_object->key_count;
        i32 key_index = 0;
        for (Lifetime_Key_Ref_Node *node = lifetime_object->key_node_first;
             node != 0;
             node = node->next){
            i32 local_count = clamp_top(lifetime_key_reference_per_node, key_count - key_index);
            for (i32 i = 0; i < local_count; i += 1){
                Lifetime_Key *key = node->keys[i];
                get_visual_markers(part, &key->dynamic_workspace, on_screen_range, buffer_id, view_index, style_data);
            }
            key_index += local_count;
        }
    }
    markers.count = (i32)(push_array(part, Render_Marker, 0) - markers.markers);
    
    // NOTE(allen): Sort visual markers by position
    Range marker_segments[4];
    visual_markers_segment_sort(markers.markers, 0, markers.count, marker_segments);
    for (i32 i = 0; i < ArrayCount(marker_segments); i += 1){
        visual_markers_quick_sort(markers.markers, marker_segments[i].first, marker_segments[i].one_past_last);
    }
    
    Render_Marker_Array character_markers = {};
    character_markers.markers = markers.markers + marker_segments[0].first;
    character_markers.count = marker_segments[0].one_past_last - marker_segments[0].first;
    
    Render_Marker_Array line_markers = {};
    line_markers.markers = markers.markers + marker_segments[1].first;
    line_markers.count = marker_segments[1].one_past_last - marker_segments[1].first;
    
    Render_Marker_Array range_markers = {};
    range_markers.markers = markers.markers + marker_segments[2].first;
    range_markers.count = marker_segments[2].one_past_last - marker_segments[2].first;
    
    Render_Marker_Array line_range_markers = {};
    line_range_markers.markers = markers.markers + marker_segments[3].first;
    line_range_markers.count = marker_segments[3].one_past_last - marker_segments[3].first;
    
    Render_Range_Record *range_stack = push_array(part, Render_Range_Record, range_markers.count);
    i32 range_stack_top = -1;
    
    Render_Range_Record *line_range_stack = push_array(part, Render_Range_Record, line_range_markers.count);
    i32 line_range_stack_top = -1;
    
    i32 *line_starts = file->state.buffer.line_starts;
    i32 line_count = file->state.buffer.line_count;
    i32 line_scan_index = render_cursor.line - 1;
    i32 first_byte_index_of_next_line = 0;
    if (line_scan_index + 1 < line_count){
        first_byte_index_of_next_line = line_starts[line_scan_index + 1];
    }
    else{
        first_byte_index_of_next_line = max_i32;
    }
    
    i32 *wrap_starts = file->state.wrap_positions;
    i32 wrap_count = file->state.wrap_position_count;
    if (wrap_count > 0 && wrap_starts[wrap_count - 1] == buffer_size(&file->state.buffer)){
        wrap_count -= 1;
    }
    i32 wrap_scan_index = render_cursor.wrap_line - render_cursor.line;
    i32 first_byte_index_of_next_wrap = 0;
    if (wrap_scan_index + 1 < wrap_count){
        first_byte_index_of_next_wrap = wrap_starts[wrap_scan_index + 1];
    }
    else{
        first_byte_index_of_next_wrap = max_i32;
    }
    
    visual_markers_replace_pos_with_first_byte_of_line(line_markers, line_starts, line_count, line_scan_index);
    visual_markers_replace_pos_with_first_byte_of_line(line_range_markers, line_starts, line_count, line_scan_index);
    
    i32 visual_markers_scan_index = 0;
    
    i32 visual_line_markers_scan_index = 0;
    u32 visual_line_markers_color = 0;
    
    i32 visual_range_markers_scan_index = 0;
    i32 visual_line_range_markers_scan_index = 0;
    
    i32 token_i = 0;
    u32 main_color = style->main.default_color;
    u32 special_color = style->main.special_character_color;
    u32 ghost_color = style->main.ghost_character_color;
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(token_array, items->index);
        main_color = *style_get_color(style, token_array.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    Buffer_Render_Item *item = items;
    Buffer_Render_Item *item_end = item + item_count;
    i32 prev_ind = -1;
    u32 highlight_color = 0;
    
    for (; item < item_end; ++item){
        i32 ind = item->index;
        
        // NOTE(allen): Line scanning
        b32 is_new_line = false;
        for (; line_scan_index < line_count;){
            if (ind < first_byte_index_of_next_line){
                break;
            }
            line_scan_index += 1;
            is_new_line = true;
            if (line_scan_index + 1 < line_count){
                first_byte_index_of_next_line = line_starts[line_scan_index + 1];
            }
            else{
                first_byte_index_of_next_line = max_i32;
            }
        }
        if (item == items){
            is_new_line = true;
        }
        
        // NOTE(allen): Wrap scanning
        b32 is_new_wrap = false;
        if (wrapped){
            for (; wrap_scan_index < wrap_count;){
                if (ind < first_byte_index_of_next_wrap){
                    break;
                }
                wrap_scan_index += 1;
                is_new_wrap = true;
                if (wrap_scan_index + 1 < wrap_count){
                    first_byte_index_of_next_wrap = wrap_starts[wrap_scan_index + 1];
                }
                else{
                    first_byte_index_of_next_wrap = max_i32;
                }
            }
        }
        
        // NOTE(allen): Token scanning
        u32 highlight_this_color = 0;
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
        
        // NOTE(allen): Line marker color
        if (is_new_line){
            i32 visual_line_markers_best_priority = min_i32;
            visual_line_markers_color = 0;
            
            for (;visual_line_markers_scan_index < line_markers.count &&
                 line_markers.markers[visual_line_markers_scan_index].pos <= ind;
                 visual_line_markers_scan_index += 1){
                Render_Marker *marker = &line_markers.markers[visual_line_markers_scan_index];
                Assert(marker->type == VisualType_LineHighlights);
                if (marker->priority > visual_line_markers_best_priority){
                    visual_line_markers_color = marker->color;
                    visual_line_markers_best_priority = marker->priority;
                }
            }
            
            // NOTE(allen): Line range marker color
            for (;visual_line_range_markers_scan_index < line_range_markers.count &&
                 line_range_markers.markers[visual_line_range_markers_scan_index].pos <= ind;
                 visual_line_range_markers_scan_index += 1){
                Render_Marker *marker = &line_range_markers.markers[visual_line_range_markers_scan_index];
                Render_Range_Record range_record = {};
                range_record.color = marker->color;
                range_record.text_color = marker->text_color;
                range_record.one_past_last = marker->one_past_last;
                range_record.priority = marker->priority;
                i32 insert_pos = range_record_stack_get_insert_index(line_range_stack, line_range_stack_top + 1, range_record.priority);
                memmove(line_range_stack + insert_pos + 1,
                        line_range_stack + insert_pos,
                        sizeof(*line_range_stack)*(line_range_stack_top - insert_pos + 1));
                line_range_stack[insert_pos] = range_record;
                line_range_stack_top += 1;
            }
            for (;line_range_stack_top >= 0 && ind > line_range_stack[line_range_stack_top].one_past_last;
                 line_range_stack_top -= 1);
            if (visual_line_markers_color == 0 && line_range_stack_top >= 0){
                visual_line_markers_color = line_range_stack[line_range_stack_top].color;
            }
        }
        
        u32 marker_line_highlight = 0;
        if (is_new_line || is_new_wrap){
            marker_line_highlight = visual_line_markers_color;
        }
        
        // NOTE(allen): Visual marker colors
        i32 marker_highlight_best_priority = min_i32;
        u32 marker_highlight = 0;
        u32 marker_highlight_text = 0;
        
        i32 marker_wireframe_best_priority = min_i32;
        u32 marker_wireframe = 0;
        
        i32 marker_ibar_best_priority = min_i32;
        u32 marker_ibar = 0;
        
        for (;visual_markers_scan_index < character_markers.count &&
             character_markers.markers[visual_markers_scan_index].pos <= ind;
             visual_markers_scan_index += 1){
            Render_Marker *marker = &character_markers.markers[visual_markers_scan_index];
            switch (marker->type){
                case VisualType_CharacterBlocks:
                {
                    if (marker->priority > marker_highlight_best_priority){
                        marker_highlight = marker->color;
                        marker_highlight_text = marker->text_color;
                        marker_highlight_best_priority = marker->priority;
                    }
                }break;
                
                case VisualType_CharacterWireFrames:
                {
                    if (marker->priority > marker_wireframe_best_priority){
                        marker_wireframe = marker->color;
                        marker_wireframe_best_priority = marker->priority;
                    }
                }break;
                
                case VisualType_CharacterIBars:
                {
                    if (marker->priority > marker_ibar_best_priority){
                        marker_ibar = marker->color;
                        marker_ibar_best_priority = marker->priority;
                    }
                }break;
                
                default:
                {
                    InvalidCodePath;
                }break;
            }
        }
        
        // NOTE(allen): Highlight range marker color
        for (;visual_range_markers_scan_index < range_markers.count &&
             range_markers.markers[visual_range_markers_scan_index].pos <= ind;
             visual_range_markers_scan_index += 1){
            Render_Marker *marker = &range_markers.markers[visual_range_markers_scan_index];
            Render_Range_Record range_record = {};
            range_record.color = marker->color;
            range_record.text_color = marker->text_color;
            range_record.one_past_last = marker->one_past_last;
            range_record.priority = marker->priority;
            i32 insert_pos = range_record_stack_get_insert_index(range_stack, range_stack_top + 1, range_record.priority);
            memmove(range_stack + insert_pos + 1,
                    range_stack + insert_pos,
                    sizeof(*range_stack)*(range_stack_top - insert_pos + 1));
            range_stack[insert_pos] = range_record;
            range_stack_top += 1;
        }
        for (;range_stack_top >= 0 && ind >= range_stack[range_stack_top].one_past_last;
             range_stack_top -= 1);
        if (range_stack_top >= 0 &&
            range_stack[range_stack_top].priority > marker_highlight_best_priority){
            marker_highlight = range_stack[range_stack_top].color;
            marker_highlight_text = range_stack[range_stack_top].text_color;
        }
        
        // NOTE(allen): Perform highlight, wireframe, and ibar renders
        u32 color_highlight = 0;
        u32 color_wireframe = 0;
        u32 color_ibar = 0;
        
        if (marker_highlight != 0){
            if (color_highlight == 0){
                color_highlight = marker_highlight;
            }
        }
        if (marker_wireframe != 0){
            if (color_wireframe == 0){
                color_wireframe = marker_wireframe;
            }
        }
        if (marker_ibar != 0){
            if (color_ibar == 0){
                color_ibar = marker_ibar;
            }
        }
        if (highlight_this_color != 0){
            if (color_highlight == 0){
                color_highlight = highlight_this_color;
            }
        }
        
        if (marker_line_highlight != 0){
            f32_Rect line_rect = f32R((f32)rect.x0, char_rect.y0, (f32)rect.x1, char_rect.y1);
            draw_rectangle(target, line_rect, marker_line_highlight);
        }
        if (color_highlight != 0){
            draw_rectangle(target, char_rect, color_highlight);
        }
        
        if (marker_highlight_text != SymbolicColor_Default){
            char_color = marker_highlight_text;
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
        if (item->codepoint != 0){
            draw_font_glyph(target, font_id, item->codepoint, item->x0, item->y0, char_color);
        }
        
        if (color_wireframe != 0){
            draw_rectangle_outline(target, char_rect, color_wireframe);
        }
        if (color_ibar != 0){
            f32_Rect ibar_rect = f32R(char_rect.x0, char_rect.y0, char_rect.x0 + 1, char_rect.y1);
            draw_rectangle_outline(target, ibar_rect, color_ibar);
        }
        
        prev_ind = ind;
    }
}

internal void
do_core_render(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    Render_Target *target = models->target;
    View *view = models->render_view;
    i32_Rect rect = models->render_rect;
    Full_Cursor render_cursor = models->render_cursor;
    Range on_screen_range = models->render_range;
    Buffer_Render_Item *items = models->render_items;
    i32 item_count = models->render_item_count;
    render_loaded_file_in_view__inner(models, target, view,
                                      rect, render_cursor, on_screen_range,
                                      items, item_count);
}

internal void
render_loaded_file_in_view(System_Functions *system, View *view, Models *models,
                           i32_Rect rect, b32 is_active, Render_Target *target){
    Editing_File *file = view->transient.file_data.file;
    i32 line_height = view->transient.line_height;
    
    f32 max_x = (f32)file->settings.display_width;
    i32 max_y = rect.y1 - rect.y0 + line_height;
    
    Assert(file != 0);
    Assert(!file->is_dummy);
    Assert(buffer_good(&file->state.buffer));
    Assert(view->transient.edit_pos != 0);
    
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    
    partition_align(part, 4);
    
    f32 left_side_space = 0;
    
    i32 max = partition_remaining(part)/sizeof(Buffer_Render_Item);
    Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, 0);
    
    b32 wrapped = !file->settings.unwrapped_lines;
    Face_ID font_id = file->settings.font_id;
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    
    f32 scroll_x = view->transient.edit_pos->scroll.scroll_x;
    f32 scroll_y = view->transient.edit_pos->scroll.scroll_y;
    
    // NOTE(allen): For now we will temporarily adjust scroll_y to try
    // to prevent the view moving around until floating sections are added
    // to the gui system.
    scroll_y += view->transient.widget_height;
    
    Full_Cursor render_cursor = {};
    if (!file->settings.unwrapped_lines){
        render_cursor = file_compute_cursor(system, file, seek_wrapped_xy(0, scroll_y, 0), true);
    }
    else{
        render_cursor = file_compute_cursor(system, file, seek_unwrapped_xy(0, scroll_y, 0), true);
    }
    
    view->transient.edit_pos->scroll_i = render_cursor.pos;
    
    i32 item_count = 0;
    i32 end_pos = 0;
    {
        Buffer_Render_Params params;
        params.buffer        = &file->state.buffer;
        params.items         = items;
        params.max           = max;
        params.count         = &item_count;
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
        
        Buffer_Render_State state = {};
        Buffer_Layout_Stop stop = {};
        
        f32 line_shift = 0.f;
        b32 do_wrap = false;
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
                            do_wrap = true;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        else{
                            do_wrap = false;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        first_wrap_determination = 0;
                    }
                    else{
                        Assert(stop.pos == wrap_unit_end);
                        do_wrap = true;
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
        
        end_pos = state.i;
    }
    push_array(part, Buffer_Render_Item, item_count);
    
    Range on_screen_range = {};
    on_screen_range.first = render_cursor.pos;
    on_screen_range.one_past_last = end_pos;
    
    ////////////////////////////////
    
    if (models->render_caller != 0){
        models->target = target;
        models->render_view = view;
        models->render_rect = rect;
        models->render_cursor = render_cursor;
        models->render_range = on_screen_range;
        models->render_items = items;
        models->render_item_count = item_count;
        models->render_caller(&models->app_links, view->persistent.id + 1, on_screen_range, do_core_render);
    }
    else{
        render_loaded_file_in_view__inner(models, target, view, rect, render_cursor, on_screen_range, items, item_count);
    }
    
    end_temp_memory(temp);
}

// BOTTOM

