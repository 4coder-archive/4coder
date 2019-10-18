/*
4coder_draw.cpp - Layout and rendering implementation of standard UI pieces (including buffers)
*/

// TOP

function int_color
get_margin_color(i32 level){
    int_color margin = 0;
    switch (level){
        default:
        case UIHighlight_None:
        {
            margin = Stag_List_Item;
        }break;
        case UIHighlight_Hover:
        {
            margin = Stag_List_Item_Hover;
        }break;
        case UIHighlight_Active:
        {
            margin = Stag_List_Item_Active;
        }break;
    }
    return(margin);
}

internal Vec2
draw_string(Application_Links *app, Face_ID font_id, String_Const_u8 string, Vec2 p, int_color color){
    return(draw_string_oriented(app, font_id, string, p, color, 0, V2(1.f, 0.f)));
}

internal void
draw_margin(Application_Links *app, Rect_f32 outer, Rect_f32 inner, int_color color){
    draw_rectangle(app, Rf32(outer.x0, outer.y0, outer.x1, inner.y0), 0.f, color);
    draw_rectangle(app, Rf32(outer.x0, inner.y1, outer.x1, outer.y1), 0.f, color);
    draw_rectangle(app, Rf32(outer.x0, inner.y0, inner.x0, inner.y1), 0.f, color);
    draw_rectangle(app, Rf32(inner.x1, inner.y0, outer.x1, inner.y1), 0.f, color);
}

internal void
draw_character_block(Application_Links *app, Text_Layout_ID layout, i64 pos, f32 roundness, int_color color){
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    draw_rectangle(app, rect, roundness, color);
}

internal void
draw_character_block(Application_Links *app, Text_Layout_ID layout, Range_i64 range, f32 roundness, int_color color){
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
    for (i64 i = range.first; i < range.one_past_last; i += 1){
        draw_character_block(app, layout, i, roundness, color);
    }
}

internal void
draw_character_wire_frame(Application_Links *app, Text_Layout_ID layout, i64 pos, f32 roundness, f32 thickness, int_color color){
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    draw_rectangle_outline(app, rect, roundness, thickness, color);
}

internal void
draw_character_wire_frame(Application_Links *app, Text_Layout_ID layout, Range_i64 range, f32 roundness, f32 thickness,  int_color color){
    for (i64 i = range.first; i < range.one_past_last; i += 1){
        draw_character_wire_frame(app, layout, i, roundness, thickness, color);
    }
}

internal void
draw_character_i_bar(Application_Links *app, Text_Layout_ID layout, i64 pos, int_color color){
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    rect.x1 = rect.x0 + 1.f;
    draw_rectangle(app, rect, 0.f, color);
}

internal void
draw_line_highlight(Application_Links *app, Text_Layout_ID layout, Range_i64 line_range, int_color color){
    Range_f32 y1 = text_layout_line_on_screen(app, layout, line_range.min);
    Range_f32 y2 = text_layout_line_on_screen(app, layout, line_range.max);
    Range_f32 y = range_union(y1, y2);
    if (range_size(y) > 0.f){
        Rect_f32 region = text_layout_region(app, layout);
        draw_rectangle(app, Rf32(rect_range_x(region), y), 0.f, color);
    }
}

internal void
draw_line_highlight(Application_Links *app, Text_Layout_ID layout, i64 line, int_color color){
    draw_line_highlight(app, layout, Ii64(line), color);
}

internal void
paint_text_color_pos(Application_Links *app, Text_Layout_ID layout, i64 pos, int_color color){
    paint_text_color(app, layout, Ii64(pos, pos + 1), color);
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
draw_background_and_margin(Application_Links *app, View_ID view, b32 is_active_view){
    Rect_f32 view_rect = view_get_screen_rect(app, view);
    Rect_f32 inner = rect_inner(view_rect, 3.f);
    int_color margin_color = get_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    draw_rectangle(app, inner, 0.f, Stag_Back);
    draw_margin(app, view_rect, inner, margin_color);
    return(inner);
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
    
    draw_rectangle(app, bar, 0.f, Stag_Bar);
    
    Fancy_Color base_color = fancy_id(Stag_Base);
    Fancy_Color pop2_color = fancy_id(Stag_Pop2);
    
    i64 cursor_position = view_get_cursor_pos(app, view_id);
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));
    
    Fancy_String_List list = {};
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
    
    {
        Dirty_State dirty = buffer_get_dirty_state(app, buffer);
        u8 space[3];
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
    
    Vec2 p = bar.p0 + V2(2.f, 2.f);
    draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
}

function void
draw_query_bar(Application_Links *app, Query_Bar *query_bar, Face_ID face_id, Rect_f32 bar){
    Scratch_Block scratch(app);
    Fancy_String_List list = {};
    push_fancy_string(scratch, &list, fancy_id(Stag_Pop1)   , query_bar->prompt);
    push_fancy_string(scratch, &list, fancy_id(Stag_Default), query_bar->string);
    Vec2_f32 p = bar.p0 + V2(2.f, 2.f);
    draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
}

function void
draw_line_number_margin(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id,
                        Text_Layout_ID text_layout_id, Rect_f32 margin){
    Rect_f32 prev_clip = draw_set_clip(app, margin);
    draw_rectangle(app, margin, 0.f, Stag_Line_Numbers_Back);
    
    Interval_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    
    Fancy_Color line_color = fancy_id(Stag_Line_Numbers_Text);
    
    i64 line_count = buffer_get_line_count(app, buffer);
    i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
    
    Scratch_Block scratch(app, Scratch_Share);
    
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(visible_range.first));
    i64 line_number = cursor.line;
    for (;cursor.pos <= visible_range.one_past_last;){
        if (line_number > line_count){
            break;
        }
        Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, line_number);
        Vec2_f32 p = V2f32(margin.x0, line_y.min);
        Temp_Memory_Block temp(scratch);
        Fancy_String *line_string = push_fancy_stringf(scratch, line_color, "%*lld", line_count_digit_count, line_number);
        draw_fancy_string(app, face_id, line_string, p, Stag_Margin_Active, 0);
        line_number += 1;
    }
    
    draw_set_clip(app, prev_clip);
}

function void
draw_fps_hud(Application_Links *app, Frame_Info frame_info,
             Face_ID face_id, Rect_f32 rect){
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    
    local_persist f32 history_literal_dt[fps_history_depth] = {};
    local_persist f32 history_animation_dt[fps_history_depth] = {};
    local_persist i32 history_frame_index[fps_history_depth] = {};
    
    i32 wrapped_index = frame_info.index%fps_history_depth;
    history_literal_dt[wrapped_index]   = frame_info.literal_dt;
    history_animation_dt[wrapped_index] = frame_info.animation_dt;
    history_frame_index[wrapped_index]  = frame_info.index;
    
    draw_rectangle(app, rect, 0.f, 0xFF000000);
    draw_rectangle_outline(app, rect, 0.f, 1.f, 0xFFFFFFFF);
    
    Vec2_f32 p = rect.p0;
    
    Scratch_Block scratch(app);
    
    Range ranges[2];
    ranges[0].first = wrapped_index;
    ranges[0].one_past_last = -1;
    ranges[1].first = fps_history_depth - 1;
    ranges[1].one_past_last = wrapped_index;
    for (i32 i = 0; i < 2; i += 1){
        Range r = ranges[i];
        for (i32 j = r.first; j > r.one_past_last; j -= 1, p.y += line_height){
            f32 dts[2];
            dts[0] = history_literal_dt[j];
            dts[1] = history_animation_dt[j];
            i32 frame_index = history_frame_index[j];
            
            Fancy_String_List list = {};
            push_fancy_stringf(scratch, &list, pink , "FPS: ");
            push_fancy_stringf(scratch, &list, green, "[");
            push_fancy_stringf(scratch, &list, white, "%5d", frame_index);
            push_fancy_stringf(scratch, &list, green, "]: ");
            
            for (i32 k = 0; k < 2; k += 1){
                f32 dt = dts[k];
                if (dt == 0.f){
                    push_fancy_stringf(scratch, &list, white, "----------");
                }
                else{
                    push_fancy_stringf(scratch, &list, white, "%10.6f", dt);
                }
                push_fancy_stringf(scratch, &list, green, " | ");
            }
            
            draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0, 0, V2(1.f, 0.f));
        }
    }
}

function int_color
get_token_color_cpp(Token token){
    int_color result = Stag_Default;
    switch (token.kind){
        case TokenBaseKind_Preprocessor:
        {
            result = Stag_Preproc;
        }break;
        case TokenBaseKind_Keyword:
        {            
            result = Stag_Keyword;
        }break;
        case TokenBaseKind_Comment:
        {
            result = Stag_Comment;
        }break;
        case TokenBaseKind_LiteralString:
        {
            result = Stag_Str_Constant;
        }break;
        case TokenBaseKind_LiteralInteger:
        {
            result = Stag_Int_Constant;
        }break;
        case TokenBaseKind_LiteralFloat:
        {
            result = Stag_Float_Constant;
        }break;
        default:
        {
            switch (token.sub_kind){
                case TokenCppKind_LiteralTrue:
                case TokenCppKind_LiteralFalse:
                {
                    result = Stag_Bool_Constant;
                }break;
                case TokenCppKind_LiteralCharacter:
                case TokenCppKind_LiteralCharacterWide:
                case TokenCppKind_LiteralCharacterUTF8:
                case TokenCppKind_LiteralCharacterUTF16:
                case TokenCppKind_LiteralCharacterUTF32:
                {
                    result = Stag_Char_Constant;
                }break;
                case TokenCppKind_PPIncludeFile:
                {
                    result = Stag_Include;
                }break;
            }
        }break;
    }
    return(result);
}

function void
draw_buffer_add_cpp_token_colors(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array){
    Interval_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 first_index = token_index_from_pos(array, visible_range.first);
    Token_Iterator_Array it = token_iterator_index(0, array, first_index);
    for (;;){
        Token *token = token_it_read(&it);
        if (token->pos >= visible_range.one_past_last){
            break;
        }
        int_color color = get_token_color_cpp(*token);
        paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), color);
        if (!token_it_inc_non_whitespace(&it)){
            break;
        }
    }
}

function void
draw_comment_highlights(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                        Token_Array *array, Comment_Highlight_Pair *pairs, i32 pair_count){
    Scratch_Block scratch(app);
    Interval_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
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
                    umem needle_size = pair->needle.size;
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

internal Range_i64_Array
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
                int_color *back_colors, int_color *fore_colors, i32 color_count){
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
                    draw_line_highlight(app, text_layout_id, line_range, back_colors[color_index]);
                }
                if (fore_colors != 0){
                    Range_i64 pos_range = get_pos_range_from_line_range(app, buffer, line_range);
                    paint_text_color(app, text_layout_id, pos_range, fore_colors[color_index]);
                }
            }
        }
        else{
            if (back_colors != 0){
                draw_character_block(app, text_layout_id, range.min, 0.f, back_colors[color_index]);
                draw_character_block(app, text_layout_id, range.max - 1, 0.f, back_colors[color_index]);
            }
            if (fore_colors != 0){
                paint_text_color_pos(app, text_layout_id, range.min, fore_colors[color_index]);
                paint_text_color_pos(app, text_layout_id, range.max - 1, fore_colors[color_index]);
            }
        }
        color_index += 1;
        color_index = (color_index%color_count);
    }
}

function void
draw_scope_highlight(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     i64 pos, int_color *colors, i32 color_count){
    draw_enclosures(app, text_layout_id, buffer,
                    pos, FindNest_Scope, RangeHighlightKind_LineHighlight,
                    colors, 0, color_count);
}

function void
draw_paren_highlight(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     i64 pos, int_color *colors, i32 color_count){
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
                    0, colors, color_count);
}

function void
draw_jump_highlights(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     Buffer_ID jump_buffer, int_color line_color){
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
                draw_character_block(app, text_layout_id, range, roundness, Stag_Highlight);
                paint_text_color(app, text_layout_id, range, Stag_At_Highlight);
            }
        }
    }
    return(has_highlight_range);
}

function void
draw_original_4coder_style_cursor_mark_highlight(Application_Links *app, View_ID view_id, b32 is_active_view,
                                                 Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                                 f32 roundness, f32 outline_thickness){
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    if (!has_highlight_range){
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        i64 mark_pos = view_get_mark_pos(app, view_id);
        if (is_active_view){
            draw_character_block(app, text_layout_id, cursor_pos, roundness, Stag_Cursor);
            paint_text_color_pos(app, text_layout_id, cursor_pos, Stag_At_Cursor);
            draw_character_wire_frame(app, text_layout_id, mark_pos, roundness, outline_thickness, Stag_Mark);
        }
        else{
            draw_character_wire_frame(app, text_layout_id, mark_pos, roundness, outline_thickness, Stag_Mark);
            draw_character_wire_frame(app, text_layout_id, cursor_pos, roundness, outline_thickness, Stag_Cursor);
        }
    }
}

function void
draw_notepad_style_cursor_highlight(Application_Links *app, View_ID view_id,
                                    Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                    f32 roundness){
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    if (!has_highlight_range){
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        i64 mark_pos = view_get_mark_pos(app, view_id);
        if (cursor_pos != mark_pos){
            Range_i64 range = Ii64(cursor_pos, mark_pos);
            draw_character_block(app, text_layout_id, range, roundness, Stag_Highlight);
            paint_text_color(app, text_layout_id, range, Stag_At_Highlight);
        }
        draw_character_i_bar(app, text_layout_id, cursor_pos, Stag_Cursor);
    }
}

// BOTTOM

