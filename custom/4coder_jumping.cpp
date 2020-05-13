/*
4coder_jumping.cpp - Routines commonly used when writing code to jump to locations and seek through jump lists.
*/

// TOP

function b32
ms_style_verify(String_Const_u8 line, u64 left_paren_pos, u64 right_paren_pos){
    i32 result = false;
    String_Const_u8 line_part = string_skip(line, right_paren_pos);
    if (string_match(string_prefix(line_part, 4), string_u8_litexpr(") : ")) ||
        string_match(string_prefix(line_part, 3), string_u8_litexpr("): "))){
        result = true;
    }
    if (result){
        String_Const_u8 number = string_skip(string_prefix(line, right_paren_pos), left_paren_pos + 1);
        if (!string_is_integer(number, 10)){
            result = false;
            u64 comma_pos = string_find_first(number, ',');
            if (comma_pos < number.size){
                String_Const_u8 sub_number0 = string_prefix(number, comma_pos);
                String_Const_u8 sub_number1 = string_skip(number, comma_pos + 1);
                if (string_is_integer(sub_number0, 10) && string_is_integer(sub_number1, 10)){
                    result = true;
                }
            }
        }
    }
    return(result);
}

function u64
try_skip_rust_arrow(String_Const_u8 line){
    u64 pos = 0;
    if (string_match(string_prefix(line, 3), string_u8_litexpr("-->"))){
        String_Const_u8 sub = string_skip(line, 3);
        sub = string_skip_chop_whitespace(sub);
        pos = (u64)(sub.str - line.str);
    }
    return(pos);
}

function b32
check_is_note(String_Const_u8 line, u64 colon_pos){
    b32 is_note = false;
    u64 note_pos = colon_pos + string_find_first(string_skip(line, colon_pos), string_u8_litexpr("note"));
    if (note_pos < line.size){
        b32 is_all_whitespace = true;
        for (u64 i = colon_pos + 1; i < note_pos; i += 1){
            if (!character_is_whitespace(line.str[i])){
                is_all_whitespace = false;
                break;
            }
        }
        if (is_all_whitespace){
            is_note = true;
        }
    }
    return(is_note);
}

function Parsed_Jump
parse_jump_location(String_Const_u8 line){
    Parsed_Jump jump = {};
    jump.sub_jump_indented = (string_get_character(line, 0) == ' ');
    
    String_Const_u8 reduced_line = string_skip_chop_whitespace(line);
    u64 whitespace_length = (u64)(reduced_line.str - line.str);
    line = reduced_line;
    
    u64 left_paren_pos = string_find_first(line, '(');
    u64 right_paren_pos = left_paren_pos + string_find_first(string_skip(line, left_paren_pos), ')');
    for (;!jump.is_ms_style && right_paren_pos < line.size;){
        if (ms_style_verify(line, left_paren_pos, right_paren_pos)){
            jump.is_ms_style = true;
            jump.colon_position = (i32)(right_paren_pos + string_find_first(string_skip(line, right_paren_pos), ':'));
            if (jump.colon_position < line.size){
                if (check_is_note(line, jump.colon_position)){
                    jump.sub_jump_note = true;
                }
                
                String_Const_u8 location_str = string_prefix(line, jump.colon_position);
                location_str = string_skip_chop_whitespace(location_str);
                
                i32 close_pos = (i32)right_paren_pos;
                i32 open_pos = (i32)left_paren_pos;
                
                if (0 < open_pos && open_pos < location_str.size){
                    String_Const_u8 file = SCu8(location_str.str, open_pos);
                    file = string_skip_chop_whitespace(file);
                    
                    if (file.size > 0){
                        String_Const_u8 line_number = string_skip(string_prefix(location_str, close_pos), open_pos + 1);
                        line_number = string_skip_chop_whitespace(line_number);
                        
                        if (line_number.size > 0){
                            u64 comma_pos = string_find_first(line_number, ',');
                            if (comma_pos < line_number.size){
                                String_Const_u8 column_number = string_skip(line_number, comma_pos + 1);
                                line_number = string_prefix(line_number, comma_pos);
                                jump.location.line = (i32)string_to_integer(line_number, 10);
                                jump.location.column = (i32)string_to_integer(column_number, 10);
                            }
                            else{
                                jump.location.line = (i32)string_to_integer(line_number, 10);
                                jump.location.column = 0;
                            }
                            jump.location.file = file;
                            jump.colon_position = jump.colon_position + (i32)whitespace_length;
                            jump.success = true;
                        }
                    }
                }
            }
        }
        else{
            left_paren_pos = string_find_first(string_skip(line, left_paren_pos + 1), '(') + left_paren_pos + 1;
            right_paren_pos = string_find_first(string_skip(line, left_paren_pos), ')') + left_paren_pos;
        }
    }
    
    if (!jump.is_ms_style){
        i32 start = (i32)try_skip_rust_arrow(line);
        if (start != 0){
            jump.has_rust_arrow = true;
        }
        
        u64 colon_pos1 = string_find_first(string_skip(line, start), ':') + start;
        if (line.size > colon_pos1 + 1){
            if (character_is_slash(string_get_character(line, colon_pos1 + 1))){
                colon_pos1 = string_find_first(string_skip(line, colon_pos1 + 1), ':') + colon_pos1 + 1;
            }
        }
        
        u64 colon_pos2 = string_find_first(string_skip(line, colon_pos1 + 1), ':') + colon_pos1 + 1;
        u64 colon_pos3 = string_find_first(string_skip(line, colon_pos2 + 1), ':') + colon_pos2 + 1;
        
        if (colon_pos3 < line.size){
            if (check_is_note(line, colon_pos3)){
                jump.sub_jump_note = true;
            }
            
            String_Const_u8 file_name = string_skip(string_prefix(line, colon_pos1), start);
            String_Const_u8 line_number = string_skip(string_prefix(line, colon_pos2), colon_pos1 + 1);
            String_Const_u8 column_number = string_skip(string_prefix(line, colon_pos3), colon_pos2 + 1);
            
            if (file_name.size > 0 && line_number.size > 0 && column_number.size > 0){
                jump.location.file = file_name;
                jump.location.line = (i32)string_to_integer(line_number, 10);
                jump.location.column = (i32)string_to_integer(column_number, 10);
                jump.colon_position = (i32)(colon_pos3 + whitespace_length);
                jump.success = true;
            }
        }
        else{
            if (colon_pos2 < line.size){
                if (check_is_note(line, colon_pos2)){
                    jump.sub_jump_note = true;
                }
                
                String_Const_u8 file_name = string_prefix(line, colon_pos1);
                String_Const_u8 line_number = string_skip(string_prefix(line, colon_pos2), colon_pos1 + 1);
                
                if (string_is_integer(line_number, 10)){
                    if (file_name.size > 0 && line_number.size > 0){
                        jump.location.file = file_name;
                        jump.location.line = (i32)string_to_integer(line_number, 10);
                        jump.location.column = 0;
                        jump.colon_position = (i32)(colon_pos3 + whitespace_length);
                        jump.success = true;
                    }
                }
            }
        }
    }
    
    if (!jump.success){
        block_zero_struct(&jump);
    }
    else{
        jump.is_sub_jump = (jump.sub_jump_indented || jump.sub_jump_note);
    }
    return(jump);
}

function Parsed_Jump
parse_jump_location(String_Const_u8 line, Jump_Flag flags){
    Parsed_Jump jump = parse_jump_location(line);
    if (HasFlag(flags, JumpFlag_SkipSubs) && jump.is_sub_jump){
        block_zero_struct(&jump);
    }
    return(jump);
}

function Parsed_Jump
parse_jump_from_buffer_line(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 line, Jump_Flag flags){
    Parsed_Jump jump = {};
    String_Const_u8 line_str = push_buffer_line(app, arena, buffer, line);
    if (line_str.size > 0){
        jump = parse_jump_location(line_str, flags);
    }
    return(jump);
}

////////////////////////////////

function b32
get_jump_buffer(Application_Links *app, Buffer_ID *buffer, Name_Line_Column_Location *location){
    return(open_file(app, buffer, location->file, false, true));
}

function b32
get_jump_buffer(Application_Links *app, Buffer_ID *buffer, ID_Pos_Jump_Location *location, Access_Flag access){
    *buffer = location->buffer_id;
    return(buffer_exists(app, *buffer));
}

function b32
get_jump_buffer(Application_Links *app, Buffer_ID *buffer, ID_Pos_Jump_Location *location){
    return(get_jump_buffer(app, buffer, location, Access_Always));
}

function View_ID
switch_to_existing_view(Application_Links *app, View_ID view, Buffer_ID buffer){
    Buffer_ID current_buffer = view_get_buffer(app, view, Access_Always);
    if (view != 0 || current_buffer != buffer){
        View_ID existing_view = get_first_view_with_buffer(app, buffer);
        if (existing_view != 0){
            view = existing_view;
        }
    }
    return(view);
}

function void
set_view_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, Buffer_Seek seek){
    Buffer_ID current_buffer = view_get_buffer(app, view, Access_Always);
    if (current_buffer != buffer){
        view_set_buffer(app, view, buffer, 0);
    }
    view_set_cursor_and_preferred_x(app, view, seek);
}

function void
jump_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, i64 pos){
    view_set_active(app, view);
    set_view_to_location(app, view, buffer, seek_pos(pos));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

function void
jump_to_location(Application_Links *app, View_ID view, Buffer_ID buffer,
                 Name_Line_Column_Location location){
    view_set_active(app, view);
    set_view_to_location(app, view, buffer, seek_line_col(location.line, location.column));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

function void
jump_to_location(Application_Links *app, View_ID view,
                 Name_Line_Column_Location location){
    Buffer_ID buffer = 0;
    if (get_jump_buffer(app, &buffer, &location)){
        jump_to_location(app, view, buffer, location);
    }
}

function void
jump_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, ID_Pos_Jump_Location location){
    view_set_active(app, view);
    set_view_to_location(app, view, buffer, seek_pos(location.pos));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

function void
jump_to_location(Application_Links *app, View_ID view, String_Const_u8 location){
    Parsed_Jump jump = parse_jump_location(location);
    if (jump.success){
        jump_to_location(app, view, jump.location);
    }
}

////////////////////////////////

// TODO(allen): rewrite
static Parsed_Jump
seek_next_jump_in_buffer(Application_Links *app, Arena *arena,
                         Buffer_ID buffer, i64 first_line, Jump_Flag flags, Scan_Direction direction,
                         i64 *line_out){
    Assert(direction == 1 || direction == -1);
    Parsed_Jump jump = {};
    i64 line = first_line;
    for (;;){
        if (is_valid_line(app, buffer, line)){
            String_Const_u8 line_str = push_buffer_line(app, arena, buffer, line);
            jump = parse_jump_location(line_str, flags);
            if (jump.success){
                break;
            }
            line += direction;
        }
        else{
            break;
        }
    }
    if (jump.success){
        *line_out = clamp_bot(line, 0);
    }
    return(jump);
}

static ID_Line_Column_Jump_Location
convert_name_based_to_id_based(Application_Links *app, Name_Line_Column_Location loc){
    ID_Line_Column_Jump_Location result = {};
    Buffer_ID buffer = get_buffer_by_name(app, loc.file, Access_Always);
    if (buffer != 0){
        result.buffer_id = buffer;
        result.line = loc.line;
        result.column = loc.column;
    }
    return(result);
}

static Parsed_Jump
seek_next_jump_in_view(Application_Links *app, Arena *arena, View_ID view, i32 skip_sub_errors, Scan_Direction direction, i64 *line_out){
    i64 cursor_position = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(cursor_position));
    i64 line = cursor.line;
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Parsed_Jump jump = seek_next_jump_in_buffer(app, arena, buffer, line + direction, skip_sub_errors, direction, &line);
    if (jump.success){
        *line_out = line;
    }
    return(jump);
}

static b32
skip_this_jump(ID_Line_Column_Jump_Location prev, ID_Line_Column_Jump_Location jump){
    b32 result = false;
    if (prev.buffer_id != 0 && prev.buffer_id == jump.buffer_id && prev.line == jump.line && prev.column <= jump.column){
        result = true;
    }
    return(result);
}

#if 0
static b32
advance_cursor_in_jump_view(Application_Links *app, View_ID view, b32 skip_repeats, b32 skip_sub_error, Scan_Direction direction, Name_Line_Column_Location *location_out){
    b32 result = true;
    
    Name_Line_Column_Location location = {};
    ID_Line_Column_Jump_Location jump = {};
    i64 line = 0;
    i64 colon_index = 0;
    
    do{
        Arena *scratch = context_get_arena(app);
        Temp_Memory temp = begin_temp(scratch);
        Parsed_Jump parsed_jump = seek_next_jump_in_view(app, scratch, view, skip_sub_error, direction, &line);
        if (parsed_jump.success){
            jump = convert_name_based_to_id_based(app, parsed_jump.location);
            view_set_cursor(app, view, seek_line_char(line, parsed_jump.colon_position + 1), true);
            result = true;
        }
        else{
            jump.buffer_id = 0;
            result = false;
        }
        end_temp(temp);
    }while(skip_repeats && skip_this_jump(prev_location, jump));
    
    if (result){
        *location_out = location;
        view_set_cursor(app, view, seek_line_char(line, colon_index + 1), true);
    }
    
    prev_location = jump;
    
    return(result);
}

static b32
seek_jump_(Application_Links *app, b32 skip_repeats, b32 skip_sub_errors, i32 direction){
    b32 result = false;
    
    View_ID view = get_view_for_locked_jump_buffer(app);
    if (view != 0){
        Name_Line_Column_Location location = {};
        if (advance_cursor_in_jump_view(app, view, skip_repeats, skip_sub_errors, direction, &location)){
            Buffer_ID buffer = {};
            if (get_jump_buffer(app, &buffer, &location)){
                View_ID target_view = get_active_view(app, Access_Always);
                if (target_view == view){
                    change_active_panel(app);
                    target_view = get_active_view(app, Access_Always);
                }
                switch_to_existing_view(app, target_view, buffer);
                jump_to_location(app, target_view, buffer, location);
                result = true;
            }
        }
    }
    
    return(result);
}
#endif

// BOTTOM

