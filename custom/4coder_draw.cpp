/*
4coder_draw.cpp - Layout and rendering implementation of standard UI pieces (including buffers)
*/

// TOP

function void
draw_text_layout_default(Application_Links *app, Text_Layout_ID layout_id){
    ARGB_Color special_color = finalize_color(defcolor_special_character, 0);
    ARGB_Color ghost_color = finalize_color(defcolor_ghost_character, 0);
    draw_text_layout(app, layout_id, special_color, ghost_color);
}

function FColor
get_item_margin_color(i32 level, i32 sub_id){
    FColor margin = fcolor_zero();
    switch (level){
        default:
        case UIHighlight_None:
        {
            margin = fcolor_id(defcolor_list_item, sub_id);
        }break;
        case UIHighlight_Hover:
        {
            margin = fcolor_id(defcolor_list_item_hover, sub_id);
        }break;
        case UIHighlight_Active:
        {
            margin = fcolor_id(defcolor_list_item_active, sub_id);
        }break;
    }
    return(margin);
}
function FColor
get_item_margin_color(i32 level){
    return(get_item_margin_color(level, 0));
}
function FColor
get_panel_margin_color(i32 level){
    FColor margin = fcolor_zero();
    switch (level){
        default:
        case UIHighlight_None:
        {
            margin = fcolor_id(defcolor_margin);
        }break;
        case UIHighlight_Hover:
        {
            margin = fcolor_id(defcolor_margin_hover);
        }break;
        case UIHighlight_Active:
        {
            margin = fcolor_id(defcolor_margin_active);
        }break;
    }
    return(margin);
}

function Vec2_f32
draw_string(Application_Links *app, Face_ID font_id, String_Const_u8 string, Vec2_f32 p, ARGB_Color color){
    return(draw_string_oriented(app, font_id, color, string, p, 0, V2f32(1.f, 0.f)));
}

function Vec2_f32
draw_string(Application_Links *app, Face_ID font_id, String_Const_u8 string, Vec2_f32 p, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    return(draw_string(app, font_id, string, p, argb));
}

function void
draw_rectangle_fcolor(Application_Links *app, Rect_f32 rect, f32 roundness, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_rectangle(app, rect, roundness, argb);
}

function void
draw_rectangle_outline_fcolor(Application_Links *app, Rect_f32 rect, f32 roundness, f32 thickness, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_rectangle_outline(app, rect, roundness, thickness, argb);
}

function void
draw_margin(Application_Links *app, Rect_f32 outer, Rect_f32 inner, ARGB_Color color){
    draw_rectangle(app, Rf32(outer.x0, outer.y0, outer.x1, inner.y0), 0.f, color);
    draw_rectangle(app, Rf32(outer.x0, inner.y1, outer.x1, outer.y1), 0.f, color);
    draw_rectangle(app, Rf32(outer.x0, inner.y0, inner.x0, inner.y1), 0.f, color);
    draw_rectangle(app, Rf32(inner.x1, inner.y0, outer.x1, inner.y1), 0.f, color);
}

function void
draw_margin(Application_Links *app, Rect_f32 outer, Rect_f32 inner, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_margin(app, outer, inner, argb);
}

function void
draw_character_block(Application_Links *app, Text_Layout_ID layout, i64 pos, f32 roundness, ARGB_Color color){
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    draw_rectangle(app, rect, roundness, color);
}

function void
draw_character_block(Application_Links *app, Text_Layout_ID layout, i64 pos, f32 roundness, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_character_block(app, layout, pos, roundness, argb);
}

function void
draw_character_block(Application_Links *app, Text_Layout_ID layout, Range_i64 range, f32 roundness, ARGB_Color color){
    if (range.first < range.one_past_last){
        i64 i = range.first;
        Rect_f32 first_rect = text_layout_character_on_screen(app, layout, i);
        i += 1;
        Range_f32 y = rect_range_y(first_rect);
        Range_f32 x = rect_range_x(first_rect);
        for (;i < range.one_past_last; i += 1){
            Rect_f32 rect = text_layout_character_on_screen(app, layout, i);
            if (rect.x0 < rect.x1 && rect.y0 < rect.y1){
                Range_f32 new_y = rect_range_y(rect);
                Range_f32 new_x = rect_range_x(rect);
                b32 joinable = false;
                if (new_y == y && (range_overlap(x, new_x) || x.max == new_x.min || new_x.max == x.min)){
                    joinable = true;
                }
                
                if (!joinable){
                    draw_rectangle(app, Rf32(x, y), roundness, color);
                    y = new_y;
                    x = new_x;
                }
                else{
                    x = range_union(x, new_x);
                }
            }
        }
        draw_rectangle(app, Rf32(x, y), roundness, color);
    }
}

function void
draw_character_block(Application_Links *app, Text_Layout_ID layout, Range_i64 range, f32 roundness, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_character_block(app, layout, range, roundness, argb);
}

function void
draw_character_wire_frame(Application_Links *app, Text_Layout_ID layout, i64 pos, f32 roundness, f32 thickness, ARGB_Color color){
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    draw_rectangle_outline(app, rect, roundness, thickness, color);
}

function void
draw_character_wire_frame(Application_Links *app, Text_Layout_ID layout, i64 pos, f32 roundness, f32 thickness, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_character_wire_frame(app, layout, pos, roundness, thickness, argb);
}

function void
draw_character_wire_frame(Application_Links *app, Text_Layout_ID layout, Range_i64 range, f32 roundness, f32 thickness, FColor color){
    for (i64 i = range.first; i < range.one_past_last; i += 1){
        draw_character_wire_frame(app, layout, i, roundness, thickness, color);
    }
}

function void
draw_character_i_bar(Application_Links *app, Text_Layout_ID layout, i64 pos, ARGB_Color color){
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    rect.x1 = rect.x0 + 1.f;
    draw_rectangle(app, rect, 0.f, color);
}

function void
draw_character_i_bar(Application_Links *app, Text_Layout_ID layout, i64 pos, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_character_i_bar(app, layout, pos, argb);
}

function void
draw_line_highlight(Application_Links *app, Text_Layout_ID layout, Range_i64 line_range, ARGB_Color color){
    Range_f32 y1 = text_layout_line_on_screen(app, layout, line_range.min);
    Range_f32 y2 = text_layout_line_on_screen(app, layout, line_range.max);
    Range_f32 y = range_union(y1, y2);
    if (range_size(y) > 0.f){
        Rect_f32 region = text_layout_region(app, layout);
        draw_rectangle(app, Rf32(rect_range_x(region), y), 0.f, color);
    }
}

function void
draw_line_highlight(Application_Links *app, Text_Layout_ID layout, Range_i64 line_range, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    draw_line_highlight(app, layout, line_range, argb);
}

function void
draw_line_highlight(Application_Links *app, Text_Layout_ID layout, i64 line, ARGB_Color color){
    draw_line_highlight(app, layout, Ii64(line), color);
}

function void
draw_line_highlight(Application_Links *app, Text_Layout_ID layout, i64 line, FColor color){
    draw_line_highlight(app, layout, Ii64(line), color);
}

function void
paint_text_color_fcolor(Application_Links *app, Text_Layout_ID layout, Range_i64 pos, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    paint_text_color(app, layout, pos, argb);
}

function void
paint_text_color_pos(Application_Links *app, Text_Layout_ID layout, i64 pos, ARGB_Color color){
    paint_text_color(app, layout, Ii64(pos, pos + 1), color);
}

function void
paint_text_color_pos(Application_Links *app, Text_Layout_ID layout, i64 pos, FColor color){
    ARGB_Color argb = fcolor_resolve(color);
    paint_text_color_pos(app, layout, pos, argb);
}

////////////////////////////////

function Rect_f32_Pair
layout_file_bar_on_top(Rect_f32 rect, f32 line_height){
    return(rect_split_top_bottom(rect, line_height + 2.f));
}

function Rect_f32_Pair
layout_file_bar_on_bot(Rect_f32 rect, f32 line_height){
    return(rect_split_top_bottom_neg(rect, line_height + 2.f));
}

function Rect_f32_Pair
layout_query_bar_on_top(Rect_f32 rect, f32 line_height, i32 bar_count){
    return(rect_split_top_bottom(rect, (line_height + 2.f)*bar_count));
}

function Rect_f32_Pair
layout_query_bar_on_bot(Rect_f32 rect, f32 line_height, i32 bar_count){
    return(rect_split_top_bottom_neg(rect, (line_height + 2.f)*bar_count));
}

function Rect_f32_Pair
layout_line_number_margin(Rect_f32 rect, f32 digit_advance, i64 digit_count){
    f32 margin_width = (f32)digit_count*digit_advance + 2.f;
    return(rect_split_left_right(rect, margin_width));
}

function Rect_f32_Pair
layout_line_number_margin(Application_Links *app, Buffer_ID buffer, Rect_f32 rect, f32 digit_advance){
    i64 line_count = buffer_get_line_count(app, buffer);
    i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
    return(layout_line_number_margin(rect, digit_advance, line_count_digit_count));
}

global_const i32 fps_history_depth = 10;
function Rect_f32_Pair
layout_fps_hud_on_bottom(Rect_f32 rect, f32 line_height){
    return(rect_split_top_bottom_neg(rect, line_height*fps_history_depth));
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view, ARGB_Color margin, ARGB_Color back, f32 width){
    Rect_f32 view_rect = view_get_screen_rect(app, view);
    Rect_f32 inner = rect_inner(view_rect, width);
    draw_rectangle(app, inner, 0.f, back);
    if (width > 0.f){
        draw_margin(app, view_rect, inner, margin);
    }
    return(inner);
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view, ARGB_Color margin, ARGB_Color back){
    return(draw_background_and_margin(app, view, margin, back, 3.f));
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view, FColor margin, FColor back, f32 width){
    ARGB_Color margin_argb = fcolor_resolve(margin);
    ARGB_Color back_argb = fcolor_resolve(back);
    return(draw_background_and_margin(app, view, margin_argb, back_argb, width));
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view, FColor margin, FColor back){
    ARGB_Color margin_argb = fcolor_resolve(margin);
    ARGB_Color back_argb = fcolor_resolve(back);
    return(draw_background_and_margin(app, view, margin_argb, back_argb, 3.f));
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view, b32 is_active_view, f32 width){
    FColor margin_color = get_panel_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    return(draw_background_and_margin(app, view, margin_color, fcolor_id(defcolor_back), width));
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view, b32 is_active_view){
    FColor margin_color = get_panel_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    return(draw_background_and_margin(app, view, margin_color, fcolor_id(defcolor_back), 3.f));
}

function Rect_f32
draw_background_and_margin(Application_Links *app, View_ID view){
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view);
    return(draw_background_and_margin(app, view, is_active_view));
}

function void
draw_file_bar(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Rect_f32 bar){
    Scratch_Block scratch(app);
    
    draw_rectangle_fcolor(app, bar, 0.f, fcolor_id(defcolor_bar));
    
    FColor base_color = fcolor_id(defcolor_base);
    FColor pop2_color = fcolor_id(defcolor_pop2);
    
    i64 cursor_position = view_get_cursor_pos(app, view_id);
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));
    
    Fancy_Line list = {};
    String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer);
    push_fancy_string(scratch, &list, base_color, unique_name);
    push_fancy_stringf(scratch, &list, base_color, " - Row: %3.lld Col: %3.lld -", cursor.line, cursor.col);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    switch (*eol_setting){
        case LineEndingKind_Binary:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" bin"));
        }break;
        
        case LineEndingKind_LF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" lf"));
        }break;
        
        case LineEndingKind_CRLF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" crlf"));
        }break;
    }
    
    u8 space[3];
    {
        Dirty_State dirty = buffer_get_dirty_state(app, buffer);
        String_u8 str = Su8(space, 0, 3);
        if (dirty != 0){
            string_append(&str, string_u8_litexpr(" "));
        }
        if (HasFlag(dirty, DirtyState_UnsavedChanges)){
            string_append(&str, string_u8_litexpr("*"));
        }
        if (HasFlag(dirty, DirtyState_UnloadedChanges)){
            string_append(&str, string_u8_litexpr("!"));
        }
        push_fancy_string(scratch, &list, pop2_color, str.string);
    }
    
    Vec2_f32 p = bar.p0 + V2f32(2.f, 2.f);
    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
}

function void
draw_query_bar(Application_Links *app, Query_Bar *query_bar, Face_ID face_id, Rect_f32 bar){
    Scratch_Block scratch(app);
    Fancy_Line list = {};
    push_fancy_string(scratch, &list, fcolor_id(defcolor_pop1)        , query_bar->prompt);
    push_fancy_string(scratch, &list, fcolor_id(defcolor_text_default), query_bar->string);
    Vec2_f32 p = bar.p0 + V2f32(2.f, 2.f);
    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
}

function void
draw_line_number_margin(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Text_Layout_ID text_layout_id, Rect_f32 margin){
    ProfileScope(app, "draw line number margin");
    
    Scratch_Block scratch(app);
    FColor line_color = fcolor_id(defcolor_line_numbers_text);
    
    Rect_f32 prev_clip = draw_set_clip(app, margin);
    draw_rectangle_fcolor(app, margin, 0.f, fcolor_id(defcolor_line_numbers_back));
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 line_count = buffer_get_line_count(app, buffer);
    i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
    
    Fancy_String fstring = {};
    u8 *digit_buffer = push_array(scratch, u8, line_count_digit_count);
    String_Const_u8 digit_string = SCu8(digit_buffer, line_count_digit_count);
    for (i32 i = 0; i < line_count_digit_count; i += 1){
        digit_buffer[i] = ' ';
    }
    
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(visible_range.first));
    i64 line_number = cursor.line;
    
    Buffer_Cursor cursor_opl = view_compute_cursor(app, view_id, seek_pos(visible_range.one_past_last));
    i64 one_past_last_line_number = cursor_opl.line + 1;
    
    u8 *small_digit = digit_buffer + line_count_digit_count - 1;
    {
        u8 *ptr = small_digit;
        if (line_number == 0){
            *ptr = '0';
        }
        else{
            for (u64 X = line_number; X > 0; X /= 10){
                *ptr = '0' + (X%10);
                ptr -= 1;
            }
        }
    }
    
    for (;line_number < one_past_last_line_number &&
         line_number < line_count;){
        Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, line_number);
        Vec2_f32 p = V2f32(margin.x0, line_y.min);
        
        fill_fancy_string(&fstring, 0, line_color, 0, 0, digit_string);
        draw_fancy_string(app, face_id, fcolor_zero(), &fstring, p);
        
        line_number += 1;
        {
            u8 *ptr = small_digit;
            for (;;){
                if (ptr < digit_buffer){
                    break;
                }
                if (*ptr == ' '){
                    *ptr = '0';
                }
                if (*ptr == '9'){
                    *ptr = '0';
                    ptr -= 1;
                }
                else{
                    *ptr += 1;
                    break;
                }
            }
        }
    }
    
    draw_set_clip(app, prev_clip);
}

function void
draw_fps_hud(Application_Links *app, Frame_Info frame_info, Face_ID face_id, Rect_f32 rect){
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    
    local_persist f32 history_literal_dt[fps_history_depth] = {};
    local_persist f32 history_animation_dt[fps_history_depth] = {};
    local_persist i32 history_frame_index[fps_history_depth] = {};
    
    i32 wrapped_index = frame_info.index%fps_history_depth;
    history_literal_dt[wrapped_index]   = frame_info.literal_dt;
    history_animation_dt[wrapped_index] = frame_info.animation_dt;
    history_frame_index[wrapped_index]  = frame_info.index;
    
    draw_rectangle_fcolor(app, rect, 0.f, f_black);
    draw_rectangle_outline_fcolor(app, rect, 0.f, 1.f, f_white);
    
    Vec2_f32 p = rect.p0;
    
    Scratch_Block scratch(app);
    
    Range_i32 ranges[2] = {};
    ranges[0].first = wrapped_index;
    ranges[0].one_past_last = -1;
    ranges[1].first = fps_history_depth - 1;
    ranges[1].one_past_last = wrapped_index;
    for (i32 i = 0; i < 2; i += 1){
        Range_i32 r = ranges[i];
        for (i32 j = r.first; j > r.one_past_last; j -= 1, p.y += line_height){
            f32 dts[2];
            dts[0] = history_literal_dt[j];
            dts[1] = history_animation_dt[j];
            i32 frame_index = history_frame_index[j];
            
            Fancy_Line list = {};
            push_fancy_stringf(scratch, &list, f_pink , "FPS: ");
            push_fancy_stringf(scratch, &list, f_green, "[");
            push_fancy_stringf(scratch, &list, f_white, "%5d", frame_index);
            push_fancy_stringf(scratch, &list, f_green, "]: ");
            
            for (i32 k = 0; k < 2; k += 1){
                f32 dt = dts[k];
                if (dt == 0.f){
                    push_fancy_stringf(scratch, &list, f_white, "----------");
                }
                else{
                    push_fancy_stringf(scratch, &list, f_white, "%10.6f", dt);
                }
                push_fancy_stringf(scratch, &list, f_green, " | ");
            }
            
            draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
        }
    }
}

function FColor
get_token_color_cpp(Token token){
    Managed_ID color = defcolor_text_default;
    switch (token.kind){
        case TokenBaseKind_Preprocessor:
        {
            color = defcolor_preproc;
        }break;
        case TokenBaseKind_Keyword:
        {
            color = defcolor_keyword;
        }break;
        case TokenBaseKind_Comment:
        {
            color = defcolor_comment;
        }break;
        case TokenBaseKind_LiteralString:
        {
            color = defcolor_str_constant;
        }break;
        case TokenBaseKind_LiteralInteger:
        {
            color = defcolor_int_constant;
        }break;
        case TokenBaseKind_LiteralFloat:
        {
            color = defcolor_float_constant;
        }break;
    }
    // specifics override generals
    switch (token.sub_kind){
        case TokenCppKind_LiteralTrue:
        case TokenCppKind_LiteralFalse:
        {
            color = defcolor_bool_constant;
        }break;
        case TokenCppKind_LiteralCharacter:
        case TokenCppKind_LiteralCharacterWide:
        case TokenCppKind_LiteralCharacterUTF8:
        case TokenCppKind_LiteralCharacterUTF16:
        case TokenCppKind_LiteralCharacterUTF32:
        {
            color = defcolor_char_constant;
        }break;
        case TokenCppKind_PPIncludeFile:
        {
            color = defcolor_include;
        }break;
    }
    FColor result = fcolor_id(color);
    return(result);
}

function void
draw_cpp_token_colors(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array){
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 first_index = token_index_from_pos(array, visible_range.first);
    Token_Iterator_Array it = token_iterator_index(0, array, first_index);
    for (;;){
        Token *token = token_it_read(&it);
        if (token->pos >= visible_range.one_past_last){
            break;
        }
        FColor color = get_token_color_cpp(*token);
        ARGB_Color argb = fcolor_resolve(color);
        paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), argb);
        if (!token_it_inc_all(&it)){
            break;
        }
    }
}

function void
draw_whitespace_highlight(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array, f32 roundness){
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 first_index = token_index_from_pos(array, visible_range.first);
    Token_Iterator_Array it = token_iterator_index(0, array, first_index);
    for (;;){
        Token *token = token_it_read(&it);
        if (token->pos >= visible_range.one_past_last){
            break;
        }
        if (token->kind == TokenBaseKind_Whitespace){
            Range_i64 range = Ii64(token);
            draw_character_block(app, text_layout_id, range, roundness,
                                 fcolor_id(defcolor_highlight_white));
        }
        if (!token_it_inc_all(&it)){
            break;
        }
    }
}

function void
draw_whitespace_highlight(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, f32 roundness){
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    for (i64 i = visible_range.first; i < visible_range.one_past_last;){
        u8 c = buffer_get_char(app, buffer, i);
        if (character_is_whitespace(c)){
            i64 s = i;
            i += 1;
            for (; i < visible_range.one_past_last; i += 1){
                c = buffer_get_char(app, buffer, i);
                if (!character_is_whitespace(c)){
                    break;
                }
            }
            Range_i64 range = Ii64(s, i);
            draw_character_block(app, text_layout_id, range, roundness,
                                 fcolor_id(defcolor_highlight_white));
        }
        else{
            i += 1;
        }
    }
}

function void
draw_comment_highlights(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                        Token_Array *array, Comment_Highlight_Pair *pairs, i32 pair_count){
    Scratch_Block scratch(app);
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 first_index = token_index_from_pos(array, visible_range.first);
    Token_Iterator_Array it = token_iterator_index(buffer, array, first_index);
    for (;;){
        Temp_Memory_Block temp(scratch);
        Token *token = token_it_read(&it);
        if (token->pos >= visible_range.one_past_last){
            break;
        }
        String_Const_u8 tail = {};
        if (token_it_check_and_get_lexeme(app, scratch, &it, TokenBaseKind_Comment, &tail)){
            for (i64 index = token->pos;
                 tail.size > 0;
                 tail = string_skip(tail, 1), index += 1){
                Comment_Highlight_Pair *pair = pairs;
                for (i32 i = 0; i < pair_count; i += 1, pair += 1){
                    u64 needle_size = pair->needle.size;
                    if (needle_size == 0){
                        continue;
                    }
                    String_Const_u8 prefix = string_prefix(tail, needle_size);
                    if (string_match(prefix, pair->needle)){
                        Range_i64 range = Ii64_size(index, needle_size);
                        paint_text_color(app, text_layout_id, range, pair->color);
                        tail = string_skip(tail, needle_size - 1);
                        index += needle_size - 1;
                        break;
                    }
                }
            }
        }
        if (!token_it_inc_non_whitespace(&it)){
            break;
        }
    }
}

function Range_i64_Array
get_enclosure_ranges(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 pos, u32 flags){
    Range_i64_Array array = {};
    i32 max = 100;
    array.ranges = push_array(arena, Range_i64, max);
    for (;;){
        Range_i64 range = {};
        if (find_surrounding_nest(app, buffer, pos, flags, &range)){
            array.ranges[array.count] = range;
            array.count += 1;
            pos = range.first;
            if (array.count >= max){
                break;
            }
        }
        else{
            break;
        }
    }
    return(array);
}

function void
draw_enclosures(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID buffer,
                i64 pos, u32 flags, Range_Highlight_Kind kind,
                ARGB_Color *back_colors, i32 back_count,
                ARGB_Color *fore_colors, i32 fore_count){
    Scratch_Block scratch(app);
    Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, flags);
    
    i32 color_index = 0;
    for (i32 i = ranges.count - 1; i >= 0; i -= 1){
        Range_i64 range = ranges.ranges[i];
        if (kind == RangeHighlightKind_LineHighlight){
            Range_i64 r[2] = {};
            if (i > 0){
                Range_i64 inner_range = ranges.ranges[i - 1];
                Range_i64 lines = get_line_range_from_pos_range(app, buffer, range);
                Range_i64 inner_lines = get_line_range_from_pos_range(app, buffer, inner_range);
                inner_lines.min = clamp_bot(lines.min, inner_lines.min);
                inner_lines.max = clamp_top(inner_lines.max, lines.max);
                inner_lines.min -= 1;
                inner_lines.max += 1;
                if (lines.min <= inner_lines.min){
                    r[0] = Ii64(lines.min, inner_lines.min);
                }
                if (inner_lines.max <= lines.max){
                    r[1] = Ii64(inner_lines.max, lines.max);
                }
            }
            else{
                r[0] = get_line_range_from_pos_range(app, buffer, range);
            }
            for (i32 j = 0; j < 2; j += 1){
                if (r[j].min == 0){
                    continue;
                }
                Range_i64 line_range = r[j];
                if (back_colors != 0){
                    i32 back_index = color_index%back_count;
                    draw_line_highlight(app, text_layout_id, line_range, back_colors[back_index]);
                }
                if (fore_colors != 0){
                    i32 fore_index = color_index%fore_count;
                    Range_i64 pos_range = get_pos_range_from_line_range(app, buffer, line_range);
                    paint_text_color(app, text_layout_id, pos_range, fore_colors[fore_index]);
                }
            }
        }
        else{
            if (back_colors != 0){
                i32 back_index = color_index%back_count;
                draw_character_block(app, text_layout_id, range.min, 0.f, back_colors[back_index]);
                draw_character_block(app, text_layout_id, range.max - 1, 0.f, back_colors[back_index]);
            }
            if (fore_colors != 0){
                i32 fore_index = color_index%fore_count;
                paint_text_color_pos(app, text_layout_id, range.min, fore_colors[fore_index]);
                paint_text_color_pos(app, text_layout_id, range.max - 1, fore_colors[fore_index]);
            }
        }
        color_index += 1;
    }
}

function void
draw_scope_highlight(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     i64 pos, ARGB_Color *colors, i32 color_count){
    draw_enclosures(app, text_layout_id, buffer,
                    pos, FindNest_Scope, RangeHighlightKind_LineHighlight,
                    colors, color_count, 0, 0);
}

function void
draw_paren_highlight(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     i64 pos, ARGB_Color *colors, i32 color_count){
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0){
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, pos);
        Token *token = token_it_read(&it);
        if (token != 0 && token->kind == TokenBaseKind_ParentheticalOpen){
            pos = token->pos + token->size;
        }
        else{
            if (token_it_dec_all(&it)){
                token = token_it_read(&it);
                if (token->kind == TokenBaseKind_ParentheticalClose &&
                    pos == token->pos + token->size){
                    pos = token->pos;
                }
            }
        }
    }
    draw_enclosures(app, text_layout_id, buffer,
                    pos, FindNest_Paren, RangeHighlightKind_CharacterHighlight,
                    0, 0, colors, color_count);
}

function void
draw_jump_highlights(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     Buffer_ID jump_buffer, FColor line_color){
    Scratch_Block scratch(app);
    if (jump_buffer != 0){
        Managed_Scope scopes[2];
        scopes[0] = buffer_get_managed_scope(app, jump_buffer);
        scopes[1] = buffer_get_managed_scope(app, buffer);
        Managed_Scope comp_scope = get_managed_scope_with_multiple_dependencies(app, scopes, ArrayCount(scopes));
        Managed_Object *markers_object = scope_attachment(app, comp_scope, sticky_jump_marker_handle, Managed_Object);
        
        i32 count = managed_object_get_item_count(app, *markers_object);
        Marker *markers = push_array(scratch, Marker, count);
        managed_object_load_data(app, *markers_object, 0, count, markers);
        for (i32 i = 0; i < count; i += 1){
            i64 line_number = get_line_number_from_pos(app, buffer, markers[i].pos);
            draw_line_highlight(app, text_layout_id, line_number, line_color);
        }
    }
}

function b32
draw_highlight_range(Application_Links *app, View_ID view_id,
                     Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     f32 roundness){
    b32 has_highlight_range = false;
    Managed_Scope scope = view_get_managed_scope(app, view_id);
    Buffer_ID *highlight_buffer = scope_attachment(app, scope, view_highlight_buffer, Buffer_ID);
    if (*highlight_buffer != 0){
        if (*highlight_buffer != buffer){
            view_disable_highlight_range(app, view_id);
        }
        else{
            has_highlight_range = true;
            Managed_Object *highlight = scope_attachment(app, scope, view_highlight_range, Managed_Object);
            Marker marker_range[2];
            if (managed_object_load_data(app, *highlight, 0, 2, marker_range)){
                Range_i64 range = Ii64(marker_range[0].pos, marker_range[1].pos);
                draw_character_block(app, text_layout_id, range, roundness,
                                     fcolor_id(defcolor_highlight));
                paint_text_color_fcolor(app, text_layout_id, range,
                                        fcolor_id(defcolor_at_highlight));
            }
        }
    }
    return(has_highlight_range);
}

function i32
default_cursor_sub_id(void){
    i32 result = 0;
    if (global_keyboard_macro_is_recording){
        result = 1;
    }
    return(result);
}

function void
draw_original_4coder_style_cursor_mark_highlight(Application_Links *app, View_ID view_id, b32 is_active_view,
                                                 Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                                 f32 roundness, f32 outline_thickness){
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    if (!has_highlight_range){
        i32 cursor_sub_id = default_cursor_sub_id();
        
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        i64 mark_pos = view_get_mark_pos(app, view_id);
        if (is_active_view){
            draw_character_block(app, text_layout_id, cursor_pos, roundness,
                                 fcolor_id(defcolor_cursor, cursor_sub_id));
            paint_text_color_pos(app, text_layout_id, cursor_pos,
                                 fcolor_id(defcolor_at_cursor));
            draw_character_wire_frame(app, text_layout_id, mark_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_mark));
        }
        else{
            draw_character_wire_frame(app, text_layout_id, mark_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_mark));
            draw_character_wire_frame(app, text_layout_id, cursor_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_cursor, cursor_sub_id));
        }
    }
}

function void
draw_notepad_style_cursor_highlight(Application_Links *app, View_ID view_id,
                                    Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                    f32 roundness){
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    if (!has_highlight_range){
        i32 cursor_sub_id = default_cursor_sub_id();
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        i64 mark_pos = view_get_mark_pos(app, view_id);
        if (cursor_pos != mark_pos){
            Range_i64 range = Ii64(cursor_pos, mark_pos);
            draw_character_block(app, text_layout_id, range, roundness, fcolor_id(defcolor_highlight));
            paint_text_color_fcolor(app, text_layout_id, range, fcolor_id(defcolor_at_highlight));
        }
        draw_character_i_bar(app, text_layout_id, cursor_pos, fcolor_id(defcolor_cursor, cursor_sub_id));
    }
}

////////////////////////////////

function Rect_f32
get_contained_box_near_point(Rect_f32 container, Vec2_f32 p, Vec2_f32 box_dims){
    Vec2_f32 container_dims = rect_dim(container);
    box_dims.x = clamp_top(box_dims.x, container_dims.x);
    box_dims.y = clamp_top(box_dims.y, container_dims.y);
    Vec2_f32 q = p + V2f32(-20.f, 22.f);
    if (q.x + box_dims.x > container.x1){
        q.x = container.x1 - box_dims.x;
    }
    if (q.y + box_dims.y > container.y1){
        q.y = p.y - box_dims.y - 2.f;
        if (q.y < container.y0){
            q.y = (container.y0 + container.y1 - box_dims.y)*0.5f;
        }
    }
    return(Rf32_xy_wh(q, box_dims));
}

function Rect_f32
draw_tool_tip(Application_Links *app, Face_ID face, Fancy_Block *block,
              Vec2_f32 p, Rect_f32 region, f32 x_padding, f32 x_half_padding,
              FColor back_color){
    Rect_f32 box = Rf32(p, p);
    if (block->line_count > 0){
        Vec2_f32 dims = get_fancy_block_dim(app, face, block);
        dims += V2f32(x_padding, 2.f);
        box = get_contained_box_near_point(region, p, dims);
        box.x0 = f32_round32(box.x0);
        box.y0 = f32_round32(box.y0);
        box.x1 = f32_round32(box.x1);
        box.y1 = f32_round32(box.y1);
        Rect_f32 prev_clip = draw_set_clip(app, box);
        draw_rectangle_fcolor(app, box, 6.f, back_color);
        draw_fancy_block(app, face, fcolor_zero(), block,
                         box.p0 + V2f32(x_half_padding, 1.f));
        draw_set_clip(app, prev_clip);
    }
    return(box);
}

function Rect_f32
draw_drop_down(Application_Links *app, Face_ID face, Fancy_Block *block,
               Vec2_f32 p, Rect_f32 region, f32 x_padding, f32 x_half_padding,
               FColor outline_color, FColor back_color){
    Rect_f32 box = Rf32(p, p);
    if (block->line_count > 0){
        Vec2_f32 dims = get_fancy_block_dim(app, face, block);
        dims += V2f32(x_padding, 4.f);
        box = get_contained_box_near_point(region, p, dims);
        box.x0 = f32_round32(box.x0);
        box.y0 = f32_round32(box.y0);
        box.x1 = f32_round32(box.x1);
        box.y1 = f32_round32(box.y1);
        Rect_f32 prev_clip = draw_set_clip(app, box);
        draw_rectangle_fcolor(app, box, 0.f, back_color);
        draw_margin(app, box, rect_inner(box, 1.f), outline_color);
        draw_fancy_block(app, face, fcolor_zero(), block,
                         box.p0 + V2f32(x_half_padding, 2.f));
        draw_set_clip(app, prev_clip);
    }
    return(box);
}

function b32
draw_button(Application_Links *app, Rect_f32 rect, Vec2_f32 mouse_p, Face_ID face, String_Const_u8 text){
    b32 hovered = false;
    if (rect_contains_point(rect, mouse_p)){
        hovered = true;
    }
    
    UI_Highlight_Level highlight = hovered?UIHighlight_Active:UIHighlight_None;
    draw_rectangle_fcolor(app, rect, 3.f, get_item_margin_color(highlight));
    rect = rect_inner(rect, 3.f);
    draw_rectangle_fcolor(app, rect, 3.f, get_item_margin_color(highlight, 1));
    
    Scratch_Block scratch(app);
    Fancy_String *fancy = push_fancy_string(scratch, 0, face, fcolor_id(defcolor_text_default), text);
    Vec2_f32 dim = get_fancy_string_dim(app, 0, fancy);
    Vec2_f32 p = (rect.p0 + rect.p1 - dim)*0.5f;
    draw_fancy_string(app, fancy, p);
    
    return(hovered);
}

// BOTTOM

