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
    if (view->transient.ui_mode_counter > 0){
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
    
    dynamic_workspace_init(heap, &result.view->transient.dynamic_workspace);
    result.view->transient.lifetime_object = lifetime_alloc_object(heap, lifetime_allocator, LifetimeObject_View, result.view);
    
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
    
    dynamic_workspace_free(heap, &view->transient.dynamic_workspace);
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

internal f32
view_get_scroll_y(View *view){
    f32 v = 0;
    if (view->transient.ui_mode_counter == 0){
        File_Edit_Positions *edit_pos = view->transient.edit_pos;
        TentativeAssert(edit_pos != 0);
        if (edit_pos != 0){
            v = edit_pos->scroll.scroll_y;
        }
    }
    else{
#if 0
        v = view->transient.gui_scroll.scroll_y;
#endif
    }
    return(v);
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
    if (view->transient.ui_mode_counter > 0){
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

internal i32
render_loaded_file_in_view(System_Functions *system, View *view, Models *models,
                           i32_Rect rect, b32 is_active, Render_Target *target){
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

// BOTTOM

