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
    if (view->ui_mode){
        return(view->ui_map_id);
    }
    else{
        return(view->file->settings.base_map_id);
    }
}

internal u32
view_get_access_flags(View *view){
    u32 result = AccessOpen;
    if (view->ui_mode){
        result |= AccessHidden;
    }
    result |= file_get_access_flags(view->file);
    return(result);
}

internal i32
view_get_index(Live_Views *live_set, View *view){
    return((i32)(view - live_set->views));
}

internal View_ID
view_get_id(Live_Views *live_set, View *view){
    return((View_ID)(view - live_set->views) + 1);
}

internal View*
live_set_alloc_view(Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, Panel *panel){
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    View *result = live_set->free_sentinel.next;
    dll_remove(result);
    block_zero_struct(result);
    
    result->in_use = true;
    init_query_set(&result->query_set);
    result->lifetime_object = lifetime_alloc_object(lifetime_allocator, DynamicWorkspace_View, result);
    panel->view = result;
    result->panel = panel;
    
    return(result);
}

internal void
live_set_free_view(Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    
    view->next = live_set->free_sentinel.next;
    view->prev = &live_set->free_sentinel;
    live_set->free_sentinel.next = view;
    view->next->prev = view;
    view->in_use = false;
    
    lifetime_free_object(lifetime_allocator, view->lifetime_object);
}

////////////////////////////////

internal File_Edit_Positions
view_get_edit_pos(View *view){
    return(view->edit_pos_);
}

internal void
view_set_edit_pos(View *view, File_Edit_Positions edit_pos){
    edit_pos.scroll.position.line_number = clamp_bot(1, edit_pos.scroll.position.line_number);
    edit_pos.scroll.target.line_number = clamp_bot(1, edit_pos.scroll.target.line_number);
    view->edit_pos_ = edit_pos;
    view->file->state.edit_pos_most_recent = edit_pos;
}

////////////////////////////////

internal Rect_f32
view_get_buffer_rect(Models *models, View *view){
    Rect_f32 region = {};
    if (models->get_view_buffer_region != 0){
        Rect_f32 rect = Rf32(view->panel->rect_inner);
        Rect_f32 sub_region = Rf32(V2(0, 0), rect_dim(rect));
        sub_region = models->get_view_buffer_region(&models->app_links, view_get_id(&models->live_set, view), sub_region);
        region.p0 = rect.p0 + sub_region.p0;
        region.p1 = rect.p0 + sub_region.p1;
        region.x1 = clamp_top(region.x1, rect.x1);
        region.y1 = clamp_top(region.y1, rect.y1);
        region.x0 = clamp_top(region.x0, region.x1);
        region.y0 = clamp_top(region.y0, region.y1);
    }
    else{
        region = Rf32(view->panel->rect_inner);
    }
    return(region);
}

internal f32
view_width(Models *models, View *view){
    return(rect_width(view_get_buffer_rect(models, view)));
}

internal f32
view_height(Models *models, View *view){
    return(rect_height(view_get_buffer_rect(models, view)));
}

////////////////////////////////

internal Buffer_Layout_Item_List
view_get_line_layout(Models *models, View *view, i64 line_number){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_get_line_layout(models, file, width, face, line_number));
}

internal Line_Shift_Vertical
view_line_shift_y(Models *models, View *view, i64 line_number, f32 y_delta){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_line_shift_y(models, file, width, face, line_number, y_delta));
}

internal f32
view_line_y_difference(Models *models, View *view, i64 line_a, i64 line_b){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_line_y_difference(models, file, width, face, line_a, line_b));
}

internal i64
view_pos_at_relative_xy(Models *models, View *view, i64 base_line, Vec2_f32 relative_xy){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_pos_at_relative_xy(models, file, width, face, base_line, relative_xy));
}

internal Vec2_f32
view_relative_xy_of_pos(Models *models, View *view, i64 base_line, i64 pos){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_relative_xy_of_pos(models, file, width, face, base_line, pos));
}

internal Buffer_Point
view_normalize_buffer_point(Models *models, View *view, Buffer_Point point){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_normalize_buffer_point(models, file, width, face, point));
}

internal Vec2_f32
view_buffer_point_difference(Models *models, View *view, Buffer_Point a, Buffer_Point b){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_buffer_point_difference(models, file, width, face, a, b));
}

internal Buffer_Point
view_move_buffer_point(Models *models, View *view, Buffer_Point buffer_point, Vec2_f32 delta){
    delta += buffer_point.pixel_shift;
    Line_Shift_Vertical shift = view_line_shift_y(models, view, buffer_point.line_number, delta.y);
    buffer_point.line_number = shift.line;
    buffer_point.pixel_shift = V2f32(delta.x, delta.y - shift.y_delta);
    return(buffer_point);
}

internal Line_Shift_Character
view_line_shift_characters(Models *models, View *view, i64 line_number, i64 character_delta){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_line_shift_characters(models, file, width, face, line_number, character_delta));
}

internal i64
view_line_character_difference(Models *models, View *view, i64 line_a, i64 line_b){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_line_character_difference(models, file, width, face, line_a, line_b));
}

internal i64
view_pos_from_relative_character(Models *models, View *view, i64 base_line, i64 relative_character){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_pos_from_relative_character(models, file, width, face, base_line, relative_character));
}

internal i64
view_relative_character_from_pos(Models *models, View *view, i64 base_line, i64 pos){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(models, view);
    return(file_relative_character_from_pos(models, file, width, face, base_line, pos));
}

internal Buffer_Cursor
view_compute_cursor(View *view, Buffer_Seek seek){
    Editing_File *file = view->file;
    return(file_compute_cursor(file, seek));
}

////////////////////////////////

internal Interval_f32
view_acceptable_y(f32 view_height, f32 line_height){
    Interval_f32 acceptable_y = {};
    if (view_height <= line_height*5.f){
        if (view_height < line_height){
            acceptable_y.max = view_height;
        }
        else{
            acceptable_y.max = view_height - line_height;
        }
    }
    else{
        acceptable_y = If32(line_height*2.f, view_height - line_height*2.f);
    }
    return(acceptable_y);
}

internal Vec2_f32
view_safety_margin(f32 view_width, f32 acceptable_y_height, f32 line_height, f32 typical_advance){
    Vec2_f32 safety = {};
    safety.y = min(line_height*5.f, (acceptable_y_height + 1.f)*0.5f);
    safety.x = min(view_width*0.5f, typical_advance*8.f);
    return(safety);
}

internal b32
view_move_view_to_cursor(Models *models, View *view, Buffer_Scroll *scroll){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    Rect_f32 rect = view_get_buffer_rect(models, view);
    Vec2_f32 view_dim = rect_dim(rect);
    
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    Vec2_f32 p = file_relative_xy_of_pos(models, file, view_dim.x, face, scroll->target.line_number, 
                                         edit_pos.cursor_pos);
    p -= scroll->target.pixel_shift;
    
    f32 line_height = face->height;
    f32 typical_advance = face->typical_advance;
    Interval_f32 acceptable_y = view_acceptable_y(view_dim.y, line_height);
    Vec2_f32 safety = view_safety_margin(view_dim.x, range_size(acceptable_y), line_height, typical_advance);
    
    Vec2_f32 target_p_relative = {};
    if (p.y < acceptable_y.min){
        target_p_relative.y = p.y - safety.y;
    }
    else if (p.y > acceptable_y.max){
        target_p_relative.y = (p.y + safety.y) - view_dim.y;
    }
    if (p.x < 0.f){
        target_p_relative.x = p.x - safety.x;
    }
    else if (p.x > view_dim.x){
        target_p_relative.x = (p.x + safety.x) - view_dim.x;
    }
    scroll->target.pixel_shift = target_p_relative;
    scroll->target = view_normalize_buffer_point(models, view, scroll->target);
    scroll->target.pixel_shift.x = f32_round32(scroll->target.pixel_shift.x);
    scroll->target.pixel_shift.y = f32_round32(scroll->target.pixel_shift.y);
    
    return(target_p_relative != V2f32(0.f, 0.f));
}

internal b32
view_move_cursor_to_view(Models *models, View *view, Buffer_Scroll scroll, i64 *pos_in_out, f32 preferred_x){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    Rect_f32 rect = view_get_buffer_rect(models, view);
    Vec2_f32 view_dim = rect_dim(rect);
    
    Vec2_f32 p = file_relative_xy_of_pos(models, file, view_dim.x, face, scroll.target.line_number, *pos_in_out);
    p -= scroll.target.pixel_shift;
    
    f32 line_height = face->height;
    Interval_f32 acceptable_y = view_acceptable_y(view_dim.y, line_height);
    Vec2_f32 safety = view_safety_margin(view_dim.x, range_size(acceptable_y),
                                         line_height, face->typical_advance);
    
    b32 adjusted_y = true;
    if (p.y < acceptable_y.min){
        p.y = acceptable_y.min + safety.y;
    }
    else if (p.y > acceptable_y.max){
        p.y = acceptable_y.max - safety.y;
    }
    else{
        adjusted_y = false;
    }
    
    b32 result = false;
    if (adjusted_y){
        p += scroll.target.pixel_shift;
        *pos_in_out = file_pos_at_relative_xy(models, file, view_dim.x, face, scroll.target.line_number, p);
        result = true;
    }
    
    return(result);
}

internal void
view_set_cursor(Models *models, View *view, i64 pos){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_cursor(&edit_pos, pos);
    view_set_edit_pos(view, edit_pos);
    Buffer_Scroll scroll = edit_pos.scroll;
    if (view_move_view_to_cursor(models, view, &scroll)){
        edit_pos.scroll = scroll;
        view_set_edit_pos(view, edit_pos);
    }
}

internal void
view_set_scroll(Models *models, View *view, Buffer_Scroll scroll){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_scroll(&edit_pos, scroll);
    view_set_edit_pos(view, edit_pos);
    if (view_move_cursor_to_view(models, view, edit_pos.scroll, &edit_pos.cursor_pos, view->preferred_x)){
        view_set_edit_pos(view, edit_pos);
    }
}

internal void
view_set_cursor_and_scroll(Models *models, View *view, i64 pos, b32 set_preferred_x, Buffer_Scroll scroll){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_cursor(&edit_pos, pos);
    view->preferred_x = 0.f;
    file_edit_positions_set_scroll(&edit_pos, scroll);
    edit_pos.last_set_type = EditPos_None;
    view_set_edit_pos(view, edit_pos);
}

internal void
view_post_paste_effect(View *view, f32 seconds, i64 start, i64 size, u32 color){
    Editing_File *file = view->file;
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
    
    Editing_File *old_file = view->file;
    if (old_file != 0){
        file_touch(&models->working_set, old_file);
        file_edit_positions_push(old_file, view_get_edit_pos(view));
    }
    
    view->file = file;
    
    File_Edit_Positions edit_pos = file_edit_positions_pop(file);
    view_set_edit_pos(view, edit_pos);
    view->mark = edit_pos.cursor_pos;
    Buffer_Cursor cursor = view_compute_cursor(view, seek_pos(edit_pos.cursor_pos));
    Vec2_f32 p = view_relative_xy_of_pos(models, view, cursor.line, edit_pos.cursor_pos);
    view->preferred_x = p.x;
    
    models->layout.panel_state_dirty = true;
}

////////////////////////////////

internal b32
file_is_viewed(Layout *layout, Editing_File *file){
    b32 is_viewed = false;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file == file){
            is_viewed = true;
            break;
        }
    }
    return(is_viewed);
}

internal void
adjust_views_looking_at_file_to_new_cursor(System_Functions *system, Models *models, Editing_File *file){
    Layout *layout = &models->layout;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file == file){
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            view_set_cursor(models, view, edit_pos.cursor_pos);
        }
    }
}

internal void
global_set_font_and_update_files(System_Functions *system, Models *models, Face *new_global_face){
    for (Node *node = models->working_set.active_file_sentinel.next;
         node != &models->working_set.active_file_sentinel;
         node = node->next){
        Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
        file->settings.face_id = new_global_face->id;
    }
    models->global_face_id = new_global_face->id;
}

internal b32
release_font_and_update(System_Functions *system, Models *models, Face *face, Face *replacement_face){
    b32 success = false;
    Assert(replacement_face != 0 && replacement_face != face);
    if (font_set_release_face(&models->font_set, face->id)){
        for (Node *node = models->working_set.active_file_sentinel.next;
             node != &models->working_set.active_file_sentinel;
             node = node->next){
            Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
            if (file->settings.face_id == face->id){
                file->settings.face_id = replacement_face->id;
            }
        }
        if (models->global_face_id == face->id){
            models->global_face_id = replacement_face->id;
        }
        success = true;
    }
    return(success);
}

////////////////////////////////

internal argb_color
finalize_color(Color_Table color_table, int_color color){
    argb_color color_argb = color;
    if ((color & 0xFF000000) == 0){
        color_argb = color_table.vals[color % color_table.count];
    }
    return(color_argb);
}

internal void
view_quit_ui(System_Functions *system, Models *models, View *view){
    Assert(view != 0);
    view->ui_mode = false;
    if (view->ui_quit != 0){
        view->ui_quit(&models->app_links, view_get_id(&models->live_set, view));
        view->ui_quit = 0;
    }
}

////////////////////////////////

internal View*
imp_get_view(Models *models, View_ID view_id){
    Live_Views *live_set = &models->live_set;
    View *view = 0;
    view_id -= 1;
    if (0 <= view_id && view_id < live_set->max){
        view = live_set->views + view_id;
        if (!view->in_use){
            view = 0;
        }
    }
    return(view);
}

////////////////////////////////

#if 0
internal void
render_loaded_file_in_view__inner(Models *models, Render_Target *target, View *view,
                                  Rect_i32 rect,
                                  Full_Cursor render_cursor,
                                  Interval_i64 on_screen_range,
                                  Buffer_Layout_Item_List list,
                                  int_color *item_colors){
    Cpp_Token_Array token_array = file->state.token_array;
    b32 tokens_use = (token_array.tokens != 0);
    i64 token_i = 0;
    
    u32 main_color    = color_table.vals[Stag_Default];
    u32 special_color = color_table.vals[Stag_Special_Character];
    u32 ghost_color   = color_table.vals[Stag_Ghost_Character];
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(token_array, (i32)items->index);
        main_color = get_token_color(color_table, token_array.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    {
        i64 ind = item->index;
        
        // NOTE(allen): Token scanning
        u32 highlight_this_color = 0;
        if (tokens_use && ind != prev_ind){
            Token current_token = token_array.tokens[token_i-1];
            
            if (token_i < token_array.count){
                if (ind >= token_array.tokens[token_i].start){
                    for (;token_i < token_array.count && ind >= token_array.tokens[token_i].start; ++token_i){
                        main_color = get_token_color(color_table, token_array.tokens[token_i]);
                        current_token = token_array.tokens[token_i];
                    }
                }
                else if (ind >= current_token.start + current_token.size){
                    main_color = color_table.vals[Stag_Default];
                }
            }
            
            if (current_token.type == CPP_TOKEN_JUNK && ind >= current_token.start && ind < current_token.start + current_token.size){
                highlight_color = color_table.vals[Stag_Highlight_Junk];
            }
            else{
                highlight_color = 0;
            }
        }
        
        u32 char_color = main_color;
        if (on_screen_range.min <= ind && ind < on_screen_range.max){
            i64 index_shifted = ind - on_screen_range.min;
            if (item_colors[index_shifted] != 0){
                char_color = finalize_color(color_table, item_colors[index_shifted]);
            }
        }
        if (HasFlag(item->flags, BRFlag_Special_Character)){
            char_color = special_color;
        }
        else if (HasFlag(item->flags, BRFlag_Ghost_Character)){
            char_color = ghost_color;
        }
        
        if (view->show_whitespace && highlight_color == 0 && codepoint_is_whitespace(item->codepoint)){
            highlight_this_color = color_table.vals[Stag_Highlight_White];
        }
        else{
            highlight_this_color = highlight_color;
        }
        
        // NOTE(allen): Perform highlight, wireframe, and ibar renders
        u32 color_highlight = 0;
        u32 color_wireframe = 0;
        u32 color_ibar = 0;
        
        if (highlight_this_color != 0){
            if (color_highlight == 0){
                color_highlight = highlight_this_color;
            }
        }
        
        if (color_highlight != 0){
            draw_rectangle(target, char_rect, color_highlight);
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
        
        if (color_wireframe != 0){
            draw_rectangle_outline(target, char_rect, color_wireframe);
        }
    }
    
}

#endif

// BOTTOM

