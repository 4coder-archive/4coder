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

internal i32
view_get_id(Live_Views *live_set, View *view){
    return((i32)(view - live_set->views) + 1);
}

internal View*
live_set_alloc_view(Heap *heap, Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, Panel *panel){
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    View *result = live_set->free_sentinel.next;
    dll_remove(result);
    block_zero_struct(result);
    
    result->in_use = true;
    init_query_set(&result->query_set);
    result->lifetime_object = lifetime_alloc_object(heap, lifetime_allocator, DynamicWorkspace_View, result);
    panel->view = result;
    result->panel = panel;
    
    return(result);
}

internal void
live_set_free_view(Heap *heap, Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    
    view->next = live_set->free_sentinel.next;
    view->prev = &live_set->free_sentinel;
    live_set->free_sentinel.next = view;
    view->next->prev = view;
    view->in_use = false;
    
    lifetime_free_object(heap, lifetime_allocator, view->lifetime_object);
}

////////////////////////////////

internal File_Edit_Positions
view_get_edit_pos(View *view){
    return(view->edit_pos_);
}

internal void
view_set_edit_pos(View *view, File_Edit_Positions edit_pos){
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

internal Vec2_i32
view_get_cursor_xy(Models *models, View *view){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    Full_Cursor cursor = file_compute_cursor(models, view->file, seek_pos(edit_pos.cursor_pos));
    Vec2_i32 result = {};
    if (view->file->settings.unwrapped_lines){
        result = V2i32((i32)cursor.unwrapped_x, (i32)cursor.unwrapped_y);
    }
    else{
        result = V2i32((i32)cursor.wrapped_x, (i32)cursor.wrapped_y);
    }
    return(result);
}

internal Cursor_Limits
view_cursor_limits(Models *models, View *view){
    Editing_File *file = view->file;
    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
    i32 line_height = (i32)face->height;
    i32 visible_height = (i32)view_height(models, view);
    Cursor_Limits limits = {};
    limits.min = line_height*2;
    limits.max = visible_height - line_height*3;
    if (limits.max - limits.min <= line_height){
        if (visible_height >= line_height){
            limits.max = visible_height - line_height;
        }
        else{
            limits.max = visible_height;
        }
        limits.min = 0;
    }
    limits.min = clamp(0, limits.min, limits.max);
    limits.max = clamp_bot(0, limits.max);
    limits.delta = clamp_top(line_height*5, (limits.max - limits.min + 1)/2);
    return(limits);
}

internal i32
view_compute_max_target_y_from_bottom_y(Models *models, View *view, f32 max_item_y, f32 line_height){
    f32 height = clamp_bot(line_height, view_height(models, view));
    f32 max_target_y = clamp_bot(0.f, max_item_y - height*0.5f);
    return(ceil32(max_target_y));
}

internal i32
view_compute_max_target_y(Models *models, View *view){
    Editing_File *file = view->file;
    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
    f32 line_height = face->height;
    Gap_Buffer *buffer = &file->state.buffer;
    i32 lowest_line = buffer->line_count;
    if (!file->settings.unwrapped_lines){
        lowest_line = file->state.wrap_line_index[buffer->line_count];
    }
    return(view_compute_max_target_y_from_bottom_y(models, view, (lowest_line + 0.5f)*line_height, line_height));
}

////////////////////////////////

internal b32
view_move_view_to_cursor(Models *models, View *view, GUI_Scroll_Vars *scroll){
    System_Functions *system = models->system;
    b32 result = false;
    i32 max_x = (i32)view_width(models, view);
    i32 max_y = view_compute_max_target_y(models, view);
    
    Vec2_i32 cursor = view_get_cursor_xy(models, view);
    
    GUI_Scroll_Vars scroll_vars = *scroll;
    i32 target_x = scroll_vars.target_x;
    i32 target_y = scroll_vars.target_y;
    
    Cursor_Limits limits = view_cursor_limits(models, view);
    if (target_y < cursor.y - limits.max){
        target_y = cursor.y - limits.max + limits.delta;
    }
    if (target_y > cursor.y - limits.min){
        target_y = cursor.y - limits.min - limits.delta;
    }
    
    target_y = clamp(0, target_y, max_y);
    
    if (cursor.x >= target_x + max_x){
        target_x = cursor.x - max_x/2;
    }
    else if (cursor.x < target_x){
        target_x = clamp_bot(0, cursor.x - max_x/2);
    }
    
    if (target_x != scroll_vars.target_x || target_y != scroll_vars.target_y){
        scroll->target_x = target_x;
        scroll->target_y = target_y;
        result = true;
    }
    
    return(result);
}

internal b32
view_move_cursor_to_view(Models *models, View *view, GUI_Scroll_Vars scroll, i32 *pos_in_out, f32 preferred_x){
    System_Functions *system = models->system;
    Editing_File *file = view->file;
    Full_Cursor cursor = file_compute_cursor(models, file, seek_pos(*pos_in_out));
    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
    f32 line_height = face->height;
    f32 old_cursor_y = 0.f;
    if (file->settings.unwrapped_lines){
        old_cursor_y = cursor.unwrapped_y;
    }
    else{
        old_cursor_y = cursor.wrapped_y;
    }
    f32 cursor_y = old_cursor_y;
    f32 target_y = (f32)scroll.target_y;
    
    Cursor_Limits limits = view_cursor_limits(models, view);
    
    if (cursor_y > target_y + limits.max){
        cursor_y = target_y + limits.max;
    }
    if (target_y != 0 && cursor_y < target_y + limits.min){
        cursor_y = target_y + limits.min;
    }
    
    b32 result = false;
    if (cursor_y != old_cursor_y){
        if (cursor_y > old_cursor_y){
            cursor_y += line_height;
        }
        else{
            cursor_y -= line_height;
        }
        Buffer_Seek seek = seek_xy(preferred_x, cursor_y, false, file->settings.unwrapped_lines);
        cursor = file_compute_cursor(models, file, seek);
        *pos_in_out = (i32)cursor.pos;
        result = true;
    }
    
    return(result);
}

internal b32
view_has_unwrapped_lines(View *view){
    return(view->file->settings.unwrapped_lines);
}

internal void
view_set_preferred_x(View *view, Full_Cursor cursor){
    if (view_has_unwrapped_lines(view)){
        view->preferred_x = cursor.unwrapped_x;
    }
    else{
        view->preferred_x = cursor.wrapped_x;
    }
}

internal void
view_set_preferred_x_to_current_position(Models *models, View *view){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    Full_Cursor cursor = file_compute_cursor(models, view->file, seek_pos(edit_pos.cursor_pos));
    view_set_preferred_x(view, cursor);
}

internal void
view_set_cursor(System_Functions *system, Models *models, View *view, Full_Cursor cursor, b32 set_preferred_x){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_cursor(&edit_pos, (i32)cursor.pos);
    if (set_preferred_x){
        view_set_preferred_x(view, cursor);
    }
    view_set_edit_pos(view, edit_pos);
    GUI_Scroll_Vars scroll = edit_pos.scroll;
    if (view_move_view_to_cursor(models, view, &scroll)){
        edit_pos.scroll = scroll;
        view_set_edit_pos(view, edit_pos);
    }
}

internal void
view_set_scroll(System_Functions *system, Models *models, View *view, GUI_Scroll_Vars scroll){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_scroll(&edit_pos, scroll, view_compute_max_target_y(models, view));
    view_set_edit_pos(view, edit_pos);
    i32 pos = (i32)edit_pos.cursor_pos;
    if (view_move_cursor_to_view(models, view, edit_pos.scroll, &pos, view->preferred_x)){
        Full_Cursor cursor = file_compute_cursor(models, view->file, seek_pos(pos));
        edit_pos.cursor_pos = cursor.pos;
        view_set_edit_pos(view, edit_pos);
    }
}

internal void
view_set_cursor_and_scroll(Models *models, View *view, Full_Cursor cursor, b32 set_preferred_x, GUI_Scroll_Vars scroll){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_cursor(&edit_pos, (i32)cursor.pos);
    if (set_preferred_x){
        view_set_preferred_x(view, cursor);
    }
    file_edit_positions_set_scroll(&edit_pos, scroll, view_compute_max_target_y(models, view));
    edit_pos.last_set_type = EditPos_None;
    view_set_edit_pos(view, edit_pos);
}

internal void
view_post_paste_effect(View *view, f32 seconds, i32 start, i32 size, u32 color){
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
    view_set_preferred_x_to_current_position(models, view);
    
    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
    Assert(face != 0);
    
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
            Full_Cursor cursor = file_compute_cursor(models, file, seek_pos(edit_pos.cursor_pos));
            view_set_cursor(system, models, view, cursor, true);
        }
    }
}

internal void
file_full_remeasure(System_Functions *system, Models *models, Editing_File *file){
    Face_ID font_id = file->settings.font_id;
    Face *face = font_set_face_from_id(&models->font_set, font_id);
    file_measure_wraps(system, &models->mem, file, face);
    adjust_views_looking_at_file_to_new_cursor(system, models, file);
}

internal void
file_set_font(System_Functions *system, Models *models, Editing_File *file, Face_ID font_id){
    file->settings.font_id = font_id;
    file_full_remeasure(system, models, file);
}

internal void
global_set_font_and_update_files(System_Functions *system, Models *models, Face_ID font_id){
    for (Node *node = models->working_set.used_sentinel.next;
         node != &models->working_set.used_sentinel;
         node = node->next){
        Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
        file_set_font(system, models, file, font_id);
    }
    models->global_font_id = font_id;
}

internal b32
alter_font_and_update_files(System_Functions *system, Models *models, Face_ID face_id, Face_Description *new_description){
    b32 success = false;
    if (font_set_modify_face(&models->font_set, face_id, new_description)){
        success = true;
        for (Node *node = models->working_set.used_sentinel.next;
             node != &models->working_set.used_sentinel;
             node = node->next){
            Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
            if (file->settings.font_id == face_id){
                file_full_remeasure(system, models, file);
            }
        }
    }
    return(success);
}

internal b32
release_font_and_update_files(System_Functions *system, Models *models, Face_ID font_id, Face_ID replacement_id){
    b32 success = false;
    if (font_set_release_face(&models->font_set, font_id)){
        Face *face = font_set_face_from_id(&models->font_set, replacement_id);
        if (face == 0){
            replacement_id = font_set_get_fallback_face(&models->font_set);
            Assert(font_set_face_from_id(&models->font_set, replacement_id) != 0);
        }
        for (Node *node = models->working_set.used_sentinel.next;
             node != &models->working_set.used_sentinel;
             node = node->next){
            Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
            if (file->settings.font_id == font_id){
                file_set_font(system, models, file, replacement_id);
            }
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

internal u32
get_token_color(Color_Table color_table, Cpp_Token token){
    u32 result = 0;
    if ((token.flags & CPP_TFLAG_IS_KEYWORD) != 0){
        if (cpp_token_category_from_type(token.type) == CPP_TOKEN_CAT_BOOLEAN_CONSTANT){
            result = color_table.vals[Stag_Bool_Constant];
        }
        else{
            result = color_table.vals[Stag_Keyword];
        }
    }
    else if ((token.flags & CPP_TFLAG_PP_DIRECTIVE) != 0){
        result = color_table.vals[Stag_Preproc];
    }
    else{
        switch (token.type){
            case CPP_TOKEN_COMMENT:
            {
                result = color_table.vals[Stag_Comment];
            }break;
            case CPP_TOKEN_STRING_CONSTANT:
            {
                result = color_table.vals[Stag_Str_Constant];
            }break;
            case CPP_TOKEN_CHARACTER_CONSTANT:
            {
                result = color_table.vals[Stag_Char_Constant];
            }break;
            case CPP_TOKEN_INTEGER_CONSTANT:
            {
                result = color_table.vals[Stag_Int_Constant];
            }break;
            case CPP_TOKEN_FLOATING_CONSTANT:
            {
                result = color_table.vals[Stag_Float_Constant];
            }break;
            case CPP_PP_INCLUDE_FILE:
            {
                result = color_table.vals[Stag_Include];
            }break;
            default:
            {
                result = color_table.vals[Stag_Default];
            }break;
        }
    }
    return(result);
}

internal void
render_loaded_file_in_view__inner(Models *models, Render_Target *target, View *view,
                                  Rect_i32 rect, Full_Cursor render_cursor, Range on_screen_range,
                                  Buffer_Render_Item *items, int_color *item_colors, i32 item_count){
    Editing_File *file = view->file;
    Arena *scratch = &models->mem.arena;
    Color_Table color_table = models->color_table;
    Font_Set *font_set = &models->font_set;
    
    Assert(file != 0);
    Assert(!file->is_dummy);
    Assert(buffer_good(&file->state.buffer));
    
    Cpp_Token_Array token_array = file->state.token_array;
    b32 tokens_use = (token_array.tokens != 0);
    b32 wrapped = (!file->settings.unwrapped_lines);
    Face_ID font_id = file->settings.font_id;
    
    i32 *line_starts = file->state.buffer.line_starts;
    i32 line_count = file->state.buffer.line_count;
    i32 line_scan_index = (i32)render_cursor.line - 1;
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
    i32 wrap_scan_index = (i32)render_cursor.wrap_line - (i32)render_cursor.line;
    i32 first_byte_index_of_next_wrap = 0;
    if (wrap_scan_index + 1 < wrap_count){
        first_byte_index_of_next_wrap = wrap_starts[wrap_scan_index + 1];
    }
    else{
        first_byte_index_of_next_wrap = max_i32;
    }
    
    i32 visual_markers_scan_index = 0;
    
    i32 visual_line_markers_scan_index = 0;
    u32 visual_line_markers_color = 0;
    
    i32 visual_range_markers_scan_index = 0;
    i32 visual_line_range_markers_scan_index = 0;
    
    i32 token_i = 0;
    u32 main_color    = color_table.vals[Stag_Default];
    u32 special_color = color_table.vals[Stag_Special_Character];
    u32 ghost_color   = color_table.vals[Stag_Ghost_Character];
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(token_array, items->index);
        main_color = get_token_color(color_table, token_array.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    Buffer_Render_Item *item = items;
    Buffer_Render_Item *item_end = item + item_count;
    i32 prev_ind = -1;
    u32 highlight_color = 0;
    
    for (i32 i = 0; item < item_end; item += 1, i += 1){
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
        
        if (item->y1 > 0){
            Rect_f32 char_rect = f32R(item->x0, item->y0, item->x1, item->y1);
            
            u32 char_color = main_color;
            if (on_screen_range.min <= ind && ind < on_screen_range.max){
                i32 index_shifted = ind - on_screen_range.min;
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
            if (item->codepoint != 0){
                draw_font_glyph(target, font_set, font_id, item->codepoint, item->x0, item->y0, char_color, GlyphFlag_None);
            }
            
            if (color_wireframe != 0){
                draw_rectangle_outline(target, char_rect, color_wireframe);
            }
        }
        
        prev_ind = ind;
    }
}

internal Full_Cursor
file_get_render_cursor(Models *models, Editing_File *file, f32 scroll_y){
    Full_Cursor result = {};
    if (file->settings.unwrapped_lines){
        result = file_compute_cursor_hint(models, file, seek_unwrapped_xy(0, scroll_y, false));
    }
    else{
        result = file_compute_cursor(models, file, seek_wrapped_xy(0, scroll_y, false));
    }
    return(result);
}

internal Full_Cursor
view_get_render_cursor(Models *models, View *view, f32 scroll_y){
    return(file_get_render_cursor(models, view->file, scroll_y));
}

internal Full_Cursor
view_get_render_cursor(Models *models, View *view){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    f32 scroll_y = edit_pos.scroll.scroll_y;
    //scroll_y += view->widget_height;
    return(view_get_render_cursor(models, view, scroll_y));
}

internal Full_Cursor
view_get_render_cursor_target(Models *models, View *view){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    f32 scroll_y = (f32)edit_pos.scroll.target_y;
    //scroll_y += view->widget_height;
    return(view_get_render_cursor(models, view, scroll_y));
}

internal void
view_quit_ui(System_Functions *system, Models *models, View *view){
    Assert(view != 0);
    view->ui_mode = false;
    if (view->ui_quit != 0){
        view->ui_quit(&models->app_links, view_get_id(&models->live_set, view));
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

// BOTTOM

