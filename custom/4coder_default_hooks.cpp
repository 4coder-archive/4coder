/*
4coder_default_hooks.cpp - Sets up the hooks for the default framework.
*/

// TOP

START_HOOK_SIG(default_start){
    default_4coder_initialize(app, files, file_count);
    default_4coder_side_by_side_panels(app, files, file_count);
    if (global_config.automatically_load_project){
        load_project(app);
    }
    // no meaning for return
    return(0);
}

COMMAND_CALLER_HOOK(default_command_caller){
    // app
    
    // NOTE(allen): Get the binding from the buffer's current map
    User_Input input = get_command_input(app);
    View_ID view = get_active_view(app, AccessAll);
    
    Command_Map_ID map_id = 0;
    if (view_is_in_ui_mode(app, view)){
        view_get_setting(app, view, ViewSetting_UICommandMap, &map_id);
    }
    else{
        Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
        Managed_Scope buffer_scope = buffer_get_managed_scope(app, buffer);
        Command_Map_ID *map_id_ptr =
            scope_attachment(app, buffer_scope, buffer_map_id, Command_Map_ID);
        if (*map_id_ptr == 0){
            *map_id_ptr = mapid_file;
        }
        map_id = *map_id_ptr;
    }
    
    Command_Binding binding = map_get_binding_recursive(&framework_mapping, map_id, &input.event);
    
    if (binding.custom == 0){
        // NOTE(allen): we don't have anything to do with this input,
        // leave it marked unhandled so that if there's a follow up
        // event it is not blocked.
        leave_command_input_unhandled(app);
    }
    else{
        // NOTE(allen): before the command is called do some book keeping
        Managed_Scope scope = view_get_managed_scope(app, view);
        Rewrite_Type *next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
        *next_rewrite = Rewrite_None;
        if (fcoder_mode == FCoderMode_NotepadLike){
            for (View_ID view_it = get_view_next(app, 0, AccessAll);
                 view_it != 0;
                 view_it = get_view_next(app, view_it, AccessAll)){
                Managed_Scope scope_it = view_get_managed_scope(app, view_it);
                b32 *snap_mark_to_cursor = scope_attachment(app, scope_it, view_snap_mark_to_cursor, b32);
                *snap_mark_to_cursor = true;
            }
        }
        
        // NOTE(allen): call the command
        binding.custom(app);
        
        // NOTE(allen): after the command is called do some book keeping
        next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
        if (next_rewrite != 0){
            Rewrite_Type *rewrite = scope_attachment(app, scope, view_rewrite_loc, Rewrite_Type);
            *rewrite = *next_rewrite;
            if (fcoder_mode == FCoderMode_NotepadLike){
                for (View_ID view_it = get_view_next(app, 0, AccessAll);
                     view_it != 0;
                     view_it = get_view_next(app, view_it, AccessAll)){
                    Managed_Scope scope_it = view_get_managed_scope(app, view_it);
                    b32 *snap_mark_to_cursor = scope_attachment(app, scope_it, view_snap_mark_to_cursor, b32);
                    if (*snap_mark_to_cursor){
                        i64 pos = view_get_cursor_pos(app, view_it);
                        view_set_mark(app, view_it, seek_pos(pos));
                    }
                }
            }
        }
    }
    
    return(0);
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

typedef i32 Range_Highlight_Kind;
enum{
    RangeHighlightKind_LineHighlight,
    RangeHighlightKind_CharacterHighlight,
};

internal void
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

static argb_color default_colors[Stag_COUNT] = {};

MODIFY_COLOR_TABLE_SIG(default_modify_color_table){
    if (default_colors[Stag_NOOP] == 0){
        default_colors[Stag_NOOP]                  = 0xFFFF00FF;
        
        default_colors[Stag_Back]                  = 0xFF0C0C0C;
        default_colors[Stag_Margin]                = 0xFF181818;
        default_colors[Stag_Margin_Hover]          = 0xFF252525;
        default_colors[Stag_Margin_Active]         = 0xFF323232;
        default_colors[Stag_List_Item]             = default_colors[Stag_Margin];
        default_colors[Stag_List_Item_Hover]       = default_colors[Stag_Margin_Hover];
        default_colors[Stag_List_Item_Active]      = default_colors[Stag_Margin_Active];
        default_colors[Stag_Cursor]                = 0xFF00EE00;
        default_colors[Stag_Highlight]             = 0xFFDDEE00;
        default_colors[Stag_Mark]                  = 0xFF494949;
        default_colors[Stag_Default]               = 0xFF90B080;
        default_colors[Stag_At_Cursor]             = default_colors[Stag_Back];
        default_colors[Stag_Highlight_Cursor_Line] = 0xFF1E1E1E;
        default_colors[Stag_At_Highlight]          = 0xFFFF44DD;
        default_colors[Stag_Comment]               = 0xFF2090F0;
        default_colors[Stag_Keyword]               = 0xFFD08F20;
        default_colors[Stag_Str_Constant]          = 0xFF50FF30;
        default_colors[Stag_Char_Constant]         = default_colors[Stag_Str_Constant];
        default_colors[Stag_Int_Constant]          = default_colors[Stag_Str_Constant];
        default_colors[Stag_Float_Constant]        = default_colors[Stag_Str_Constant];
        default_colors[Stag_Bool_Constant]         = default_colors[Stag_Str_Constant];
        default_colors[Stag_Include]               = default_colors[Stag_Str_Constant];
        default_colors[Stag_Preproc]               = default_colors[Stag_Default];
        default_colors[Stag_Special_Character]     = 0xFFFF0000;
        default_colors[Stag_Ghost_Character]       = 0xFF4E5E46;
        
        default_colors[Stag_Paste] = 0xFFDDEE00;
        default_colors[Stag_Undo]  = 0xFF00DDEE;
        
        default_colors[Stag_Highlight_Junk]  = 0xFF3A0000;
        default_colors[Stag_Highlight_White] = 0xFF003A3A;
        
        default_colors[Stag_Bar]        = 0xFF888888;
        default_colors[Stag_Base]       = 0xFF000000;
        default_colors[Stag_Pop1]       = 0xFF3C57DC;
        default_colors[Stag_Pop2]       = 0xFFFF0000;
        
        default_colors[Stag_Back_Cycle_1] = 0x10A00000;
        default_colors[Stag_Back_Cycle_2] = 0x0C00A000;
        default_colors[Stag_Back_Cycle_3] = 0x0C0000A0;
        default_colors[Stag_Back_Cycle_4] = 0x0CA0A000;
        default_colors[Stag_Text_Cycle_1] = 0xFFA00000;
        default_colors[Stag_Text_Cycle_2] = 0xFF00A000;
        default_colors[Stag_Text_Cycle_3] = 0xFF0030B0;
        default_colors[Stag_Text_Cycle_4] = 0xFFA0A000;
        
        default_colors[Stag_Line_Numbers_Back] = 0xFF101010;
        default_colors[Stag_Line_Numbers_Text] = 0xFF404040;
    }
    
    Color_Table color_table = {};
    color_table.vals = default_colors;
    color_table.count = ArrayCount(default_colors);
    return(color_table);
}

GET_VIEW_BUFFER_REGION_SIG(default_view_buffer_region){
    Buffer_ID buffer = view_get_buffer(app, view_id, AccessAll);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    i32 line_height = i32_ceil32(metrics.line_height);
    
    // file bar
    {
        b64 showing_file_bar = false;
        if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar)){
            if (showing_file_bar){
                sub_region.y0 += line_height + 2;
            }
        }
    }
    
    // query bar
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            i32 widget_height = (line_height + 2)*query_bars.count;
            sub_region.y0 += widget_height;
        }
    }
    
    // line number margins
    if (global_config.show_line_number_margins){
        i64 line_count = buffer_get_line_count(app, buffer);
        i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
        i32 margin_width = i32_ceil32((f32)line_count_digit_count*metrics.normal_advance);
        sub_region.x0 += margin_width + 2;
    }
    
    return(sub_region);
}

internal int_color
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

internal void
default_buffer_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id, Rect_f32 view_inner_rect){
    Buffer_ID buffer = view_get_buffer(app, view_id, AccessAll);
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    
    f32 line_height = face_metrics.line_height;
    
    Rect_f32 sub_region = default_view_buffer_region(app, view_id, view_inner_rect);
    Rect_f32 buffer_rect = rect_intersect(sub_region, view_inner_rect);
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, buffer_rect, buffer_point);
    Interval_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    
    View_ID active_view = get_active_view(app, AccessAll);
    b32 is_active_view = (active_view == view_id);
    
    Scratch_Block scratch(app);
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0){
        i64 first_index = token_index_from_pos(&token_array, visible_range.first);
        Token_Iterator_Array it = token_iterator_index(buffer, &token_array, first_index);
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
    
    // NOTE(allen): Scan for TODOs and NOTEs
    {
        Temp_Memory temp = begin_temp(scratch);
        String_Const_u8 tail = push_buffer_range(app, scratch, buffer, visible_range);
        i32 index = 0;
        for (;tail.size > 0; tail = string_skip(tail, 1), index += 1){
            if (string_match(string_prefix(tail, 4), string_u8_litexpr("NOTE"))){
                Interval_i64 range = Ii64_size(visible_range.first + index, 4);
                paint_text_color(app, text_layout_id, range, Stag_Text_Cycle_2);
                tail = string_skip(tail, 3);
                index += 3;
            }
            else if (string_match(string_prefix(tail, 4), string_u8_litexpr("TODO"))){
                Interval_i64 range = Ii64_size(visible_range.first + index, 4);
                paint_text_color(app, text_layout_id, range, Stag_Text_Cycle_1);
                tail = string_skip(tail, 3);
                index += 3;
            }
        }
        end_temp(temp);
    }
    
    i64 cursor_pos = view_get_cursor_pos(app, view_id);
    i64 mark_pos = view_get_mark_pos(app, view_id);
    
    // NOTE(allen): Scope highlight
    if (do_matching_enclosure_highlight){
        static const i32 color_count = 4;
        int_color colors[color_count];
        for (u16 i = 0; i < color_count; i += 1){
            colors[i] = Stag_Back_Cycle_1 + i;
        }
        draw_enclosures(app, text_layout_id, buffer,
                        cursor_pos, FindNest_Scope, RangeHighlightKind_LineHighlight,
                        colors, 0, color_count);
    }
    
    // NOTE(allen): Error highlight
    {
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, AccessAll);
        if (compilation_buffer != 0){
            Managed_Scope scopes[2];
            scopes[0] = buffer_get_managed_scope(app, compilation_buffer);
            scopes[1] = buffer_get_managed_scope(app, buffer);
            Managed_Scope scope = get_managed_scope_with_multiple_dependencies(app, scopes, ArrayCount(scopes));
            Managed_Object *markers_object = scope_attachment(app, scope, sticky_jump_marker_handle, Managed_Object);
            
            Temp_Memory temp = begin_temp(scratch);
            i32 count = managed_object_get_item_count(app, *markers_object);
            Marker *markers = push_array(scratch, Marker, count);
            managed_object_load_data(app, *markers_object, 0, count, markers);
            for (i32 i = 0; i < count; i += 1){
                i64 line_number = get_line_number_from_pos(app, buffer, markers[i].pos);
                draw_line_highlight(app, text_layout_id, line_number,
                                    Stag_Highlight_Junk);
            }
            end_temp(temp);
        }
    }
    
    // NOTE(allen): Color parens
    if (do_matching_paren_highlight){
        i64 pos = cursor_pos;
        if (buffer_get_char(app, buffer, pos) == '('){
            pos += 1;
        }
        else if (pos > 0){
            if (buffer_get_char(app, buffer, pos - 1) == ')'){
                pos -= 1;
            }
        }
        static const i32 color_count = 4;
        int_color colors[color_count];
        for (u16 i = 0; i < color_count; i += 1){
            colors[i] = Stag_Text_Cycle_1 + i;
        }
        draw_enclosures(app, text_layout_id, buffer,
                        cursor_pos, FindNest_Paren, RangeHighlightKind_CharacterHighlight,
                        0, colors, color_count);
    }
    
    // NOTE(allen): Line highlight
    if (highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number, Stag_Highlight_Cursor_Line);
    }
    
    // NOTE(allen): Roundness
    f32 cursor_roundness = 4.f;
    f32 mark_thickness = 2.f;
    
    // NOTE(allen): Highlight range
    b32 has_highlight_range = false;
    {
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
                    draw_character_block(app, text_layout_id, range, cursor_roundness, Stag_Highlight);
                    paint_text_color(app, text_layout_id, range, Stag_At_Highlight);
                }
            }
        }
    }
    
    // NOTE(allen): Cursor and mark
    b32 cursor_is_hidden_in_this_view = (cursor_is_hidden && is_active_view);
    if (!cursor_is_hidden_in_this_view){
        switch (fcoder_mode){
            case FCoderMode_Original:
            {
                if (is_active_view){
                    draw_character_block(app, text_layout_id, cursor_pos,
                                         cursor_roundness, Stag_Cursor);
                    paint_text_color_pos(app, text_layout_id, cursor_pos,
                                         Stag_At_Cursor);
                    draw_character_wire_frame(app, text_layout_id, mark_pos,
                                              cursor_roundness, mark_thickness,
                                              Stag_Mark);
                }
                else{
                    draw_character_wire_frame(app, text_layout_id, mark_pos,
                                              cursor_roundness, mark_thickness,
                                              Stag_Mark);
                    draw_character_wire_frame(app, text_layout_id, cursor_pos,
                                              cursor_roundness, mark_thickness,
                                              Stag_Cursor);
                }
            }break;
            
            case FCoderMode_NotepadLike:
            {
                if (cursor_pos != mark_pos){
                    Range_i64 range = Ii64(cursor_pos, mark_pos);
                    draw_character_block(app, text_layout_id, range,
                                         cursor_roundness, Stag_Highlight);
                    paint_text_color(app, text_layout_id, range, Stag_At_Highlight);
                }
                draw_character_i_bar(app, text_layout_id, cursor_pos, Stag_Cursor);
            }break;
        }
    }
    
    {
        Rect_f32 prev_clip = draw_set_clip(app, buffer_rect);
        draw_text_layout(app, text_layout_id);
        draw_set_clip(app, prev_clip);
    }
    
    // NOTE(allen): FPS HUD
    if (show_fps_hud){
        static const i32 history_depth = 10;
        static f32 history_literal_dt[history_depth] = {};
        static f32 history_animation_dt[history_depth] = {};
        static i32 history_frame_index[history_depth] = {};
        
        i32 wrapped_index = frame_info.index%history_depth;
        history_literal_dt[wrapped_index]   = frame_info.literal_dt;
        history_animation_dt[wrapped_index] = frame_info.animation_dt;
        history_frame_index[wrapped_index]  = frame_info.index;
        
        Rect_f32 hud_rect = view_get_screen_rect(app, view_id);
        hud_rect.y0 = hud_rect.y1 - line_height*(f32)(history_depth);
        draw_rectangle(app, hud_rect, 0.f, 0xFF000000);
        draw_rectangle_outline(app, hud_rect, 0.f, 1.f, 0xFFFFFFFF);
        
        Vec2 p = hud_rect.p0;
        
        Range ranges[2];
        ranges[0].first = wrapped_index;
        ranges[0].one_past_last = -1;
        ranges[1].first = history_depth - 1;
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
        
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): Frame
    Rect_f32 r_cursor = view_inner_rect;
    
    // NOTE(allen): Filebar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
        Rect_f32 bar = r_cursor;
        bar.y1 = bar.y0 + line_height + 2.f;
        r_cursor.y0 = bar.y1;
        
        draw_rectangle(app, bar, 0.f, Stag_Bar);
        
        Fancy_Color base_color = fancy_id(Stag_Base);
        Fancy_Color pop2_color = fancy_id(Stag_Pop2);
        
        Temp_Memory temp = begin_temp(scratch);
        
        i64 cursor_position = view_get_cursor_pos(app, view_id);
        Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));
        
        Fancy_String_List list = {};
        String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer);
        push_fancy_string(scratch, &list, base_color, unique_name);
        push_fancy_stringf(scratch, &list, base_color, " - Row: %3.lld Col: %3.lld -", cursor.line, cursor.col);
        
        b64 is_dos_mode = false;
        if (buffer_get_setting(app, buffer, BufferSetting_Eol, &is_dos_mode)){
            if (is_dos_mode){
                push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" dos"));
            }
            else{
                push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" nix"));
            }
        }
        else{
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" ???"));
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
        
        Vec2 p = bar.p0 + V2(0.f, 2.f);
        draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
        
        end_temp(temp);
    }
    
    // NOTE(allen): Query Bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            for (i32 i = 0; i < query_bars.count; i += 1){
                Query_Bar *query_bar = query_bars.ptrs[i];
                
                Rect_f32 bar = r_cursor;
                bar.y1 = bar.y0 + line_height + 2.f;
                r_cursor.y0 = bar.y1;
                
                Temp_Memory temp = begin_temp(scratch);
                Fancy_String_List list = {};
                
                Fancy_Color default_color = fancy_id(Stag_Default);
                Fancy_Color pop1_color = fancy_id(Stag_Pop1);
                
                push_fancy_string(scratch, &list, pop1_color   , query_bar->prompt);
                push_fancy_string(scratch, &list, default_color, query_bar->string);
                
                Vec2 p = bar.p0 + V2(0.f, 2.f);
                draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
                
                end_temp(temp);
            }
        }
    }
    
    // NOTE(allen): Line Numbers
    if (global_config.show_line_number_margins){
        i64 line_count = buffer_get_line_count(app, buffer);
        i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
        // TODO(allen): I need a "digit width"
        f32 zero = get_string_advance(app, face_id, string_u8_litexpr("0"));
        f32 margin_width = ((f32)line_count_digit_count)*zero;
        
        Rect_f32 left_margin = r_cursor;
        left_margin.x1 = left_margin.x0 + margin_width + 2.f;
        r_cursor.x0 = left_margin.x1;
        
        draw_rectangle(app, left_margin, 0.f, Stag_Line_Numbers_Back);
        
        Rect_f32 prev_clip = draw_set_clip(app, left_margin);
        
        Fancy_Color line_color = fancy_id(Stag_Line_Numbers_Text);
        
        Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(visible_range.first));
        i64 line_number = cursor.line;
        for (;cursor.pos <= visible_range.one_past_last;){
            if (line_number > line_count){
                break;
            }
            Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, line_number);
            Vec2_f32 p = V2f32(left_margin.x0, line_y.min);
            Temp_Memory_Block temp(scratch);
            Fancy_String *line_string = push_fancy_stringf(scratch, line_color, "%*lld", line_count_digit_count, line_number);
            draw_fancy_string(app, face_id, line_string, p, Stag_Margin_Active, 0);
            line_number += 1;
        }
        
        draw_set_clip(app, prev_clip);
    }
    
    text_layout_free(app, text_layout_id);
}

internal void
default_ui_render_caller(Application_Links *app, View_ID view_id, Rect_f32 rect, Face_ID face_id){
    UI_Data *ui_data = 0;
    Arena *ui_arena = 0;
    if (view_get_ui_data(app, view_id, ViewGetUIFlag_KeepDataAsIs, &ui_data, &ui_arena)){
        Basic_Scroll scroll = view_get_basic_scroll(app, view_id);
        
        for (UI_Item *item = ui_data->list.first;
             item != 0;
             item = item->next){
            Rect_f32 item_rect = Rf32(item->rect_outer);
            item_rect.p0 += rect.p0;
            item_rect.p1 += rect.p0;
            
            switch (item->coordinates){
                case UICoordinates_ViewSpace:
                {
                    item_rect.p0 -= scroll.position;
                    item_rect.p1 -= scroll.position;
                }break;
                case UICoordinates_PanelSpace:
                {}break;
            }
            
            if (rect_overlap(item_rect, rect)){
                Rect_f32 inner = rect_inner(item_rect, (f32)item->inner_margin);
                
                Face_Metrics metrics = get_face_metrics(app, face_id);
                f32 line_height = metrics.line_height;
                f32 info_height = (f32)item->line_count*line_height;
                
                draw_rectangle(app, inner, 0.f, Stag_Back);
                Vec2_f32 p = V2f32(inner.x0 + 3.f, f32_round32((inner.y0 + inner.y1 - info_height)*0.5f));
                for (i32 i = 0; i < item->line_count; i += 1){
                    draw_fancy_string(app, face_id, item->lines[i].first, p, Stag_Default, 0, 0, V2(1.f, 0));
                    p.y += line_height;
                }
                if (item->inner_margin > 0){
                    draw_margin(app, item_rect, inner, get_margin_color(item->activation_level));
                }
            }
        }
    }
}
internal void
default_ui_render_caller(Application_Links *app, View_ID view, Rect_f32 rect){
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    Face_ID face_id = get_face_id(app, buffer);
    default_ui_render_caller(app, view, rect, face_id);
}

internal void
default_render_view(Application_Links *app, Frame_Info frame_info, View_ID view, b32 is_active){
    Rect_f32 view_rect = view_get_screen_rect(app, view);
    Rect_f32 inner = rect_inner(view_rect, 3);
    draw_rectangle(app, view_rect, 0.f, get_margin_color(is_active?UIActivation_Active:UIActivation_None));
    draw_rectangle(app, inner, 0.f, Stag_Back);
    Rect_f32 prev_clip = draw_set_clip(app, inner);
    
    Managed_Scope scope = view_get_managed_scope(app, view);
    View_Render_Hook **hook_ptr = scope_attachment(app, scope, view_render_hook, View_Render_Hook*);
    
    if (*hook_ptr == 0){
        if (view_is_in_ui_mode(app, view)){
            default_ui_render_caller(app, view, inner);
        }
        else{
            default_buffer_render_caller(app, frame_info, view, inner);
        }
    }
    else{
        View_Render_Hook *hook = *hook_ptr;
        hook(app, view, frame_info, inner);
    }
    
    draw_set_clip(app, prev_clip);
}

RENDER_CALLER_SIG(default_render_caller){
    View_ID active_view_id = get_active_view(app, AccessAll);
    for (View_ID view_id = get_view_next(app, 0, AccessAll);
         view_id != 0;
         view_id = get_view_next(app, view_id, AccessAll)){
        default_render_view(app, frame_info, view_id, (active_view_id == view_id));
    }
}

HOOK_SIG(default_exit){
    // If this returns false it cancels the exit.
    b32 result = true;
    
    if (!allow_immediate_close_without_checking_for_changes){
        b32 has_unsaved_changes = false;
        for (Buffer_ID buffer = get_buffer_next(app, 0, AccessAll);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, AccessAll)){
            Dirty_State dirty = buffer_get_dirty_state(app, buffer);
            if (HasFlag(dirty, DirtyState_UnsavedChanges)){
                has_unsaved_changes = true;
                break;
            }
        }
        if (has_unsaved_changes){
            View_ID view = get_active_view(app, AccessAll);
            do_gui_sure_to_close_4coder(app, view);
            result = false;
        }
    }
    
    return(result);
}

HOOK_SIG(default_view_adjust){
    // NOTE(allen): Called whenever the view layout/sizes have been modified, including
    // by full window resize.
    return(0);
}

BUFFER_NAME_RESOLVER_SIG(default_buffer_name_resolution){
    if (conflict_count > 1){
        // List of unresolved conflicts
        Scratch_Block scratch(app);
        
        i32 *unresolved = push_array(scratch, i32, conflict_count);
        i32 unresolved_count = conflict_count;
        for (i32 i = 0; i < conflict_count; ++i){
            unresolved[i] = i;
        }
        
        // Resolution Loop
        i32 x = 0;
        for (;;){
            // Resolution Pass
            ++x;
            for (i32 i = 0; i < unresolved_count; ++i){
                i32 conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                
                umem size = conflict->base_name.size;
                size = clamp_top(size, conflict->unique_name_capacity);
                conflict->unique_name_len_in_out = size;
                block_copy(conflict->unique_name_in_out, conflict->base_name.str, size);
                
                if (conflict->file_name.str != 0){
                    Temp_Memory_Block temp(scratch);
                    String_Const_u8 uniqueifier = {};
                    
                    String_Const_u8 file_name = string_remove_last_folder(conflict->file_name);
                    if (file_name.size > 0){
                        file_name = string_chop(file_name, 1);
                        u8 *end = file_name.str + file_name.size;
                        b32 past_the_end = false;
                        for (i32 j = 0; j < x; ++j){
                            file_name = string_remove_last_folder(file_name);
                            if (j + 1 < x){
                                file_name = string_chop(file_name, 1);
                            }
                            if (file_name.size == 0){
                                if (j + 1 < x){
                                    past_the_end = true;
                                }
                                break;
                            }
                        }
                        u8 *start = file_name.str + file_name.size;
                        
                        uniqueifier = SCu8(start, end);
                        if (past_the_end){
                            uniqueifier = push_u8_stringf(scratch, "%.*s~%d",
                                                          string_expand(uniqueifier), i);
                        }
                    }
                    else{
                        uniqueifier = push_u8_stringf(scratch, "%d", i);
                    }
                    
                    String_u8 builder = Su8(conflict->unique_name_in_out,
                                            conflict->unique_name_len_in_out,
                                            conflict->unique_name_capacity);
                    string_append(&builder, string_u8_litexpr(" <"));
                    string_append(&builder, uniqueifier);
                    string_append(&builder, string_u8_litexpr(">"));
                    conflict->unique_name_len_in_out = builder.size;
                }
            }
            
            // Conflict Check Pass
            b32 has_conflicts = false;
            for (i32 i = 0; i < unresolved_count; ++i){
                i32 conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                String_Const_u8 conflict_name = SCu8(conflict->unique_name_in_out,
                                                     conflict->unique_name_len_in_out);
                
                b32 hit_conflict = false;
                if (conflict->file_name.str != 0){
                    for (i32 j = 0; j < unresolved_count; ++j){
                        if (i == j) continue;
                        
                        i32 conflict_j_index = unresolved[j];
                        Buffer_Name_Conflict_Entry *conflict_j = &conflicts[conflict_j_index];
                        
                        String_Const_u8 conflict_name_j = SCu8(conflict_j->unique_name_in_out,
                                                               conflict_j->unique_name_len_in_out);
                        
                        if (string_match(conflict_name, conflict_name_j)){
                            hit_conflict = true;
                            break;
                        }
                    }
                }
                
                if (hit_conflict){
                    has_conflicts = true;
                }
                else{
                    --unresolved_count;
                    unresolved[i] = unresolved[unresolved_count];
                    --i;
                }
            }
            
            if (!has_conflicts){
                break;
            }
        }
    }
}

internal void
do_full_lex(Application_Links *app, Buffer_ID buffer_id){
    Scratch_Block scratch(app);
    String_Const_u8 contents = push_whole_buffer(app, scratch, buffer_id);
    Token_List list = lex_full_input_cpp(scratch, contents);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Base_Allocator *allocator = managed_scope_allocator(app, scope);
    Token_Array tokens = {};
    tokens.tokens = base_array(allocator, Token, list.total_count);
    tokens.count = list.total_count;
    tokens.max = list.total_count;
    token_fill_memory_from_list(tokens.tokens, &list);
    
    Token_Array *tokens_ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
    block_copy_struct(tokens_ptr, &tokens);
}

BUFFER_HOOK_SIG(default_file_settings){
    b32 treat_as_code = false;
    b32 lex_without_strings = false;
    (void)(lex_without_strings);
    
    String_Const_u8_Array extensions = global_config.code_exts;
    
    Scratch_Block scratch(app);
    
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    
    if (file_name.size > 0){
        String_Const_u8 ext = string_file_extension(file_name);
        for (i32 i = 0; i < extensions.count; ++i){
            if (string_match(ext, extensions.strings[i])){
                
                if (string_match(ext, string_u8_litexpr("cpp")) || 
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc"))){
                    treat_as_code = true;
                }
                
#if 0                
                treat_as_code = true;
                
                if (string_match(ext, string_u8_litexpr("cs"))){
                    if (parse_context_language_cs == 0){
                        init_language_cs(app);
                    }
                    parse_context_id = parse_context_language_cs;
                }
                
                if (string_match(ext, string_u8_litexpr("java"))){
                    if (parse_context_language_java == 0){
                        init_language_java(app);
                    }
                    parse_context_id = parse_context_language_java;
                }
                
                if (string_match(ext, string_u8_litexpr("rs"))){
                    if (parse_context_language_rust == 0){
                        init_language_rust(app);
                    }
                    parse_context_id = parse_context_language_rust;
                    lex_without_strings = true;
                }
                
                if (string_match(ext, string_u8_litexpr("cpp")) || 
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real GLSL highlighting
                if (string_match(ext, string_u8_litexpr("glsl"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real Objective-C highlighting
                if (string_match(ext, string_u8_litexpr("m"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
#endif
                
                break;
            }
        }
    }
    
    Command_Map_ID map_id = (treat_as_code)?(default_code_map):(mapid_file);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_virtual_whitespace = false;
    b32 use_lexer = false;
    if (treat_as_code){
        wrap_lines = global_config.enable_code_wrapping;
        use_virtual_whitespace = global_config.enable_virtual_whitespace;
        use_lexer = true;
    }
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(buffer_name, string_u8_litexpr("*compilation*"))){
        wrap_lines = false;
    }
    
    if (use_lexer){
        do_full_lex(app, buffer_id);
    }
    
    // no meaning for return
    return(0);
}

BUFFER_HOOK_SIG(default_new_file){
    // buffer_id
    // no meaning for return
    return(0);
}

BUFFER_HOOK_SIG(default_file_save){
    b32 is_virtual = false;
    if (global_config.automatically_indent_text_on_save &&
        is_virtual){ 
        auto_indent_buffer(app, buffer_id, buffer_range(app, buffer_id));
    }
    // no meaning for return
    return(0);
}

FILE_EDIT_RANGE_SIG(default_file_edit_range){
    // buffer_id, range, text
    
    Interval_i64 replace_range = Ii64(range.first, range.first + text.size);
    i64 insert_size = range_size(range);
    i64 text_shift = replace_range_shift(replace_range, insert_size);
    
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Token_Array *ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
    if (ptr != 0 && ptr->tokens != 0){
        i64 token_index_first = token_relex_first(ptr, range.first, 1);
        i64 token_index_resync_guess = token_relex_resync(ptr, range.one_past_last, 16);
        
        Token *token_first = ptr->tokens + token_index_first;
        Token *token_resync = ptr->tokens + token_index_resync_guess;
        
        Interval_i64 relex_range = Ii64(token_first->pos,
                                        token_resync->pos + token_resync->size + text_shift);
        String_Const_u8 partial_text = push_buffer_range(app, scratch, buffer_id, relex_range);
        
        Token_List relex_list = lex_full_input_cpp(scratch, partial_text);
        if (relex_range.one_past_last < buffer_get_size(app, buffer_id)){
            token_drop_eof(&relex_list);
        }
        
        Token_Relex relex = token_relex(relex_list, relex_range.first - text_shift,
                                        ptr->tokens, token_index_first, token_index_resync_guess);
        
        Base_Allocator *allocator = managed_scope_allocator(app, scope);
        
        if (relex.successful_resync){
            i64 token_index_resync = relex.first_resync_index;
            
            Interval_i64 head = Ii64(0, token_index_first);
            Interval_i64 replaced = Ii64(token_index_first, token_index_resync);
            Interval_i64 tail = Ii64(token_index_resync, ptr->count);
            i64 resynced_count = (token_index_resync_guess + 1) - token_index_resync;
            i64 relexed_count = relex_list.total_count - resynced_count;
            i64 tail_shift = relexed_count - (token_index_resync - token_index_first);
            
            i64 new_tokens_count = ptr->count + tail_shift;
            Token *new_tokens = base_array(allocator, Token, new_tokens_count);
            
            Token *old_tokens = ptr->tokens;
            block_copy_array_shift(new_tokens, old_tokens, head, 0);
            token_fill_memory_from_list(new_tokens + replaced.first, &relex_list, relexed_count);
            for (i64 i = 0, index = replaced.first; i < relexed_count; i += 1, index += 1){
                new_tokens[index].pos += relex_range.first;
            }
            for (i64 i = tail.first; i < tail.one_past_last; i += 1){
                old_tokens[i].pos += text_shift;
            }
            block_copy_array_shift(new_tokens, ptr->tokens, tail, tail_shift);
            
            base_free(allocator, ptr->tokens);
            
            ptr->tokens = new_tokens;
            ptr->count = new_tokens_count;
            ptr->max = new_tokens_count;
        }
        else{
            scratch.restore();
            base_free(allocator, ptr->tokens);
            do_full_lex(app, buffer_id);
        }
        
#if 0
        // NOTE(allen): Assert correctness of relex results. Enable this code
        // to track down corruption of the token data.
        {
            String_Const_u8 full = push_whole_buffer(app, scratch, buffer_id);
            Token_List list = lex_full_input_cpp(scratch, full);
            Token_Array array = token_array_from_list(scratch, &list);
            Assert(array.count == ptr->count);
            Token *token_a = array.tokens;
            Token *token_b = ptr->tokens;
            for (i64 i = 0; i < array.count; i += 1, token_a += 1, token_b += 1){
                Assert(block_match_struct(token_a, token_b));
            }
        }
#endif
    }
    
    // no meaning for return
    return(0);
}

FILE_EXTERNALLY_MODIFIED_SIG(default_file_externally_modified){
    // buffer_id
    Scratch_Block scratch(app);
    String_Const_u8 name = push_buffer_unique_name(app, scratch, buffer_id);
    String_Const_u8 str = push_u8_stringf(scratch, "Modified externally: %s\n", name.str);
    print_message(app, str);
    // no meaning for return
    return(0);
}

BUFFER_HOOK_SIG(default_end_file){
    Scratch_Block scratch(app);
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer_id);
    String_Const_u8 str = push_u8_stringf(scratch, "Ending file: %s\n", buffer_name.str);
    print_message(app, str);
    // no meaning for return
    return(0);
}

// NOTE(allen|a4.0.9): The input filter allows you to modify the input
// to a frame before 4coder starts processing it at all.
//
// Right now it only has access to the mouse state, but it will be
// extended to have access to the key presses soon.
INPUT_FILTER_SIG(default_suppress_mouse_filter){
    if (suppressing_mouse){
        block_zero_struct(mouse);
        mouse->p.x = -100;
        mouse->p.y = -100;
    }
}

// TODO(allen): FIX FIX FIX FIX
// NOTE(allen|a4): scroll rule information
//
// The parameters:
// target_x, target_y
//  This is where the view would like to be for the purpose of
// following the cursor, doing mouse wheel work, etc.
//
// scroll_x, scroll_y
//  These are pointers to where the scrolling actually is. If you bind
// the scroll rule it is you have to update these in some way to move
// the actual location of the scrolling.
//
// view_id
//  This corresponds to which view is computing it's new scrolling position.
// This id DOES correspond to the views that View_ _Summary contains.
// This will always be between 1 and 16 (0 is a null id).
// See below for an example of having state that carries across scroll udpates.
//
// is_new_target
//  If the target of the view is different from the last target in either x or y
// this is true, otherwise it is false.
//
// The return:
//  Should be true if and only if scroll_x or scroll_y are changed.
//
// Don't try to use the app pointer in a scroll rule, you're asking for trouble.
//
// If you don't bind scroll_rule, nothing bad will happen, yo will get default
// 4coder scrolling behavior.
//

Vec2_f32 scroll_velocity_[16] = {};
Vec2_f32 *scroll_velocity = scroll_velocity_ - 1;

struct Smooth_Step{
    f32 p;
    f32 v;
};

internal Smooth_Step
smooth_camera_step(f32 target, f32 v, f32 S, f32 T){
    Smooth_Step step = {};
    step.v = v;
    if (step.p != target){
        if (step.p > target - .1f && step.p < target + .1f){
            step.p = target;
            step.v = 1.f;
        }
        else{
            f32 L = step.p + T*(target - step.p);
            i32 sign = (target > step.p) - (target < step.p);
            f32 V = step.p + sign*step.v;
            if (sign > 0){
                step.p = (L<V)?(L):(V);
            }
            else{
                step.p = (L>V)?(L):(V);
            }
            if (step.p == V){
                step.v *= S;
            }
        }
    }
    return(step);
}

DELTA_RULE_SIG(smooth_scroll_rule){
    Vec2_f32 *velocity = scroll_velocity + view_id;
    if (velocity->x == 0.f){
        velocity->x = 1.f;
        velocity->y = 1.f;
    }
    Smooth_Step step_x = smooth_camera_step(pending_delta.x, velocity->x, 80.f, 1.f/2.f);
    Smooth_Step step_y = smooth_camera_step(pending_delta.y, velocity->y, 80.f, 1.f/2.f);
    *velocity = V2f32(step_x.v, step_y.v);
    return(V2f32(step_x.p, step_y.p));
}

internal void
set_all_default_hooks(Application_Links *app){
    set_custom_hook(app, HookID_Exit, default_exit);
    set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);
    
    set_custom_hook(app, HookID_Start, default_start);
    set_custom_hook(app, HookID_OpenFile, default_file_settings);
    set_custom_hook(app, HookID_NewFile, default_new_file);
    set_custom_hook(app, HookID_SaveFile, default_file_save);
    set_custom_hook(app, HookID_FileEditRange, default_file_edit_range);
    set_custom_hook(app, HookID_FileExternallyModified, default_file_externally_modified);
    set_custom_hook(app, HookID_EndFile, end_file_close_jump_list);
    
    set_custom_hook(app, HookID_CommandCaller, default_command_caller);
    set_custom_hook(app, HookID_RenderCaller, default_render_caller);
    set_custom_hook(app, HookID_InputFilter, default_suppress_mouse_filter);
    set_custom_hook(app, HookID_ScrollRule, smooth_scroll_rule);
    set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
    set_custom_hook(app, HookID_ModifyColorTable, default_modify_color_table);
    set_custom_hook(app, HookID_GetViewBufferRegion, default_view_buffer_region);
}

// BOTTOM

