/*
4coder_jumping.cpp - Routines commonly used when writing code to jump to locations and seek through jump lists.
*/

// TOP

static b32
ms_style_verify(String line, i32 left_paren_pos, i32 right_paren_pos){
    i32 result = false;
    String line_part = substr_tail(line, right_paren_pos);
    if (match_part_sc(line_part, ") : ")){
        result = true;
    }
    else if (match_part_sc(line_part, "): ")){
        result = true;
    }
    if (result){
        String number = substr(line, left_paren_pos + 1, right_paren_pos - left_paren_pos - 2);
        if (!str_is_int_s(number)){
            result = false;
            i32 comma_pos = find_s_char(number, 0, ',');
            if (comma_pos < number.size){
                String sub_number0 = substr(number, 0, comma_pos);
                String sub_number1 = substr(number, comma_pos + 1, number.size - comma_pos - 1);
                if (str_is_int_s(sub_number0) && str_is_int_s(sub_number1)){
                    result = true;
                }
            }
        }
    }
    return(result);
}

static i32
try_skip_rust_arrow(String line){
    i32 pos = 0;
    if (match_part(line, "-->")){
        String sub = substr_tail(line, 3);
        sub = skip_chop_whitespace(sub);
        pos = (i32)(sub.str - line.str);
    }
    return(pos);
}

static b32
check_is_note(String line, i32 colon_pos){
    b32 is_note = false;
    i32 note_pos = find_substr(line, colon_pos, make_lit_string("note"));
    if (note_pos < line.size){
        b32 is_all_whitespace = true;
        for (i32 i = colon_pos + 1; i < note_pos; i += 1){
            if (!char_is_whitespace(line.str[i])){
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

static Parsed_Jump
parse_jump_location(String line){
    Parsed_Jump jump = {};
    jump.sub_jump_indented = (line.str[0] == ' ');
    
    i32 whitespace_length = 0;
    line = skip_chop_whitespace(line, &whitespace_length);
    
    i32 left_paren_pos = find_s_char(line, 0, '(');
    i32 right_paren_pos = find_s_char(line, left_paren_pos, ')');
    for (;!jump.is_ms_style && right_paren_pos < line.size;){
        if (ms_style_verify(line, left_paren_pos, right_paren_pos)){
            jump.is_ms_style = true;
            jump.colon_position = find_s_char(line, right_paren_pos, ':');
            if (jump.colon_position < line.size){
                if (check_is_note(line, jump.colon_position)){
                    jump.sub_jump_note = true;
                }
                
                String location_str = substr(line, 0, jump.colon_position);
                
                location_str = skip_chop_whitespace(location_str);
                
                i32 close_pos = right_paren_pos;
                i32 open_pos = left_paren_pos;
                
                if (0 < open_pos && open_pos < location_str.size){
                    String file = substr(location_str, 0, open_pos);
                    file = skip_chop_whitespace(file);
                    
                    if (file.size > 0){
                        String line_number = substr(location_str, open_pos + 1, close_pos-open_pos - 1);
                        line_number = skip_chop_whitespace(line_number);
                        
                        if (line_number.size > 0){
                            i32 comma_pos = find_s_char(line_number, 0, ',');
                            if (comma_pos < line_number.size){
                                i32 start = comma_pos + 1;
                                String column_number = substr(line_number, start, line_number.size-start);
                                line_number = substr(line_number, 0, comma_pos);
                                jump.location.line = str_to_int_s(line_number);
                                jump.location.column = str_to_int_s(column_number);
                            }
                            else{
                                jump.location.line = str_to_int_s(line_number);
                                jump.location.column = 0;
                            }
                            jump.location.file = file;
                            jump.colon_position = jump.colon_position + whitespace_length;
                            jump.success = true;
                        }
                    }
                }
            }
        }
        else{
            left_paren_pos = find_s_char(line, left_paren_pos + 1, '(');
            right_paren_pos = find_s_char(line, left_paren_pos, ')');
        }
    }
    
    if (!jump.is_ms_style){
        i32 start = try_skip_rust_arrow(line);
        if (start != 0){
            jump.has_rust_arrow = true;
        }
        
        i32 colon_pos1 = find_s_char(line, start, ':');
        if (line.size > colon_pos1 + 1){
            if (char_is_slash(line.str[colon_pos1 + 1])){
                colon_pos1 = find_s_char(line, colon_pos1 + 1, ':');
            }
        }
        
        i32 colon_pos2 = find_s_char(line, colon_pos1 + 1, ':');
        i32 colon_pos3 = find_s_char(line, colon_pos2 + 1, ':');
        
        if (colon_pos3 < line.size){
            if (check_is_note(line, colon_pos3)){
                jump.sub_jump_note = true;
            }
            
            String filename = substr(line, start, colon_pos1 - start);
            String line_number = substr(line, colon_pos1 + 1, colon_pos2 - colon_pos1 - 1);
            String column_number = substr(line, colon_pos2 + 1, colon_pos3 - colon_pos2 - 1);
            
            if (filename.size > 0 &&
                line_number.size > 0 &&
                column_number.size > 0){
                jump.location.file = filename;
                jump.location.line = str_to_int_s(line_number);
                jump.location.column = str_to_int_s(column_number);
                jump.colon_position = colon_pos3 + whitespace_length;
                jump.success = true;
            }
        }
        else{
            if (colon_pos2 < line.size){
                if (check_is_note(line, colon_pos2)){
                    jump.sub_jump_note = true;
                }
                
                String filename = substr(line, 0, colon_pos1);
                String line_number = substr(line, colon_pos1 + 1, colon_pos2 - colon_pos1 - 1);
                
                if (str_is_int_s(line_number)){
                    if (filename.size > 0 && line_number.size > 0){
                        jump.location.file = filename;
                        jump.location.line = str_to_int_s(line_number);
                        jump.location.column = 0;
                        jump.colon_position = colon_pos3 + whitespace_length;
                        jump.success = true;
                    }
                }
            }
        }
    }
    
    if (!jump.success){
        memset(&jump, 0, sizeof(jump));
    }
    else{
        jump.is_sub_jump = (jump.sub_jump_indented || jump.sub_jump_note);
    }
    return(jump);
}

static Parsed_Jump
parse_jump_location(String line, b32 skip_sub_jump){
    Parsed_Jump jump = parse_jump_location(line);
    if (jump.is_sub_jump && skip_sub_jump){
        memset(&jump, 0, sizeof(jump));
    }
    return(jump);
}

static Parsed_Jump
parse_jump_from_buffer_line(Application_Links *app, Partition *arena, Buffer_ID buffer_id, i32 line, b32 skip_sub_errors){
    Parsed_Jump jump = {};
    String line_str = {};
    if (read_line(app, arena, buffer_id, line, &line_str)){
        jump = parse_jump_location(line_str, skip_sub_errors);
    }
    return(jump);
}

////////////////////////////////

static b32
get_jump_buffer(Application_Links *app, Buffer_ID *buffer, Name_Line_Column_Location *location){
    return(open_file(app, buffer, location->file.str, location->file.size, false, true));
}

static b32
get_jump_buffer(Application_Links *app, Buffer_ID *buffer, ID_Pos_Jump_Location *location, Access_Flag access){
    *buffer = location->buffer_id;
    return(buffer_exists(app, *buffer));
}

static b32
get_jump_buffer(Application_Links *app, Buffer_ID *buffer, ID_Pos_Jump_Location *location){
    return(get_jump_buffer(app, buffer, location, AccessAll));
}

static View_ID
switch_to_existing_view(Application_Links *app, View_ID view, Buffer_ID buffer){
    Buffer_ID current_buffer = 0;
    view_get_buffer(app, view, AccessAll, &current_buffer);
    if (view != 0 || current_buffer != buffer){
        View_ID existing_view = get_first_view_with_buffer(app, buffer);
        if (existing_view != 0){
            view = existing_view;
        }
    }
    return(view);
}

static void
set_view_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, Buffer_Seek seek){
    Buffer_ID current_buffer = 0;
    view_get_buffer(app, view, AccessAll, &current_buffer);
    if (current_buffer != buffer){
        view_set_buffer(app, view, buffer, 0);
    }
    view_set_cursor(app, view, seek, true);
}

static void
jump_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, Name_Line_Column_Location location){
    view_set_active(app, view);
    set_view_to_location(app, view, buffer, seek_line_char(location.line, location.column));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

static void
jump_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, ID_Pos_Jump_Location location){
    view_set_active(app, view);
    set_view_to_location(app, view, buffer, seek_pos(location.pos));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

////////////////////////////////

static Parsed_Jump
seek_next_jump_in_buffer(Application_Links *app, Partition *part,
                         i32 buffer_id, i32 first_line, b32 skip_sub_errors, i32 direction,
                         i32 *line_out){
    Assert(direction == 1 || direction == -1);
    Parsed_Jump jump = {};
    i32 line = first_line;
    String line_str = {};
    for (;;){
        if (read_line(app, part, buffer_id, line, &line_str)){
            jump = parse_jump_location(line_str, skip_sub_errors);
            if (jump.success){
                break;
            }
            line += direction;
        }
        else{
            break;
        }
    }
    if (line < 0){
        line = 0;
    }
    if (jump.success){
        *line_out = line;
    }
    return(jump);
}

static ID_Line_Column_Jump_Location
convert_name_based_to_id_based(Application_Links *app, Name_Line_Column_Location loc){
    ID_Line_Column_Jump_Location result = {};
    Buffer_ID buffer = 0;
    get_buffer_by_name(app, loc.file, AccessAll, &buffer);
    if (buffer != 0){
        result.buffer_id = buffer;
        result.line = loc.line;
        result.column = loc.column;
    }
    return(result);
}

static Parsed_Jump
seek_next_jump_in_view(Application_Links *app, Partition *part, View_ID view, i32 skip_sub_errors, i32 direction, i32 *line_out){
    i32 cursor_position = 0;
    view_get_cursor_pos(app, view, &cursor_position);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(cursor_position), &cursor);
    i32 line = cursor.line;
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessAll, &buffer);
    Parsed_Jump jump = seek_next_jump_in_buffer(app, part, buffer, line + direction, skip_sub_errors, direction, &line);
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

static b32
advance_cursor_in_jump_view(Application_Links *app, Partition *part, View_ID view, i32 skip_repeats, i32 skip_sub_error, i32 direction, Name_Line_Column_Location *location_out){
    b32 result = true;
    
    Name_Line_Column_Location location = {};
    ID_Line_Column_Jump_Location jump = {};
    i32 line = 0;
    i32 colon_index = 0;
    
    do{
        Temp_Memory temp = begin_temp_memory(part);
        Parsed_Jump parsed_jump = seek_next_jump_in_view(app, part, view, skip_sub_error, direction, &line);
        if (parsed_jump.success){
            jump = convert_name_based_to_id_based(app, parsed_jump.location);
            view_set_cursor(app, view, seek_line_char(line, parsed_jump.colon_position + 1), true);
            result = true;
        }
        else{
            jump.buffer_id = 0;
            result = false;
        }
        end_temp_memory(temp);
    }while(skip_repeats && skip_this_jump(prev_location, jump));
    
    if (result){
        *location_out = location;
        view_set_cursor(app, view, seek_line_char(line, colon_index + 1), true);
    }
    
    prev_location = jump;
    
    return(result);
}

static b32
seek_jump(Application_Links *app, Partition *part, b32 skip_repeats, b32 skip_sub_errors, i32 direction){
    b32 result = false;
    
    View_ID view = get_view_for_locked_jump_buffer(app);
    if (view != 0){
        Name_Line_Column_Location location = {};
        if (advance_cursor_in_jump_view(app, &global_part, view, skip_repeats, skip_sub_errors, direction, &location)){
            Buffer_ID buffer = {};
            if (get_jump_buffer(app, &buffer, &location)){
                View_ID target_view = 0;
                get_active_view(app, AccessAll, &target_view);
                if (target_view == view){
                    change_active_panel(app);
                    get_active_view(app, AccessAll, &target_view);
                }
                switch_to_existing_view(app, target_view, buffer);
                jump_to_location(app, target_view, buffer, location);
                result = true;
            }
        }
    }
    
    return(result);
}

////////////////////////////////

#if defined(USE_OLD_STYLE_JUMPS)

#define goto_jump_at_cursor                 CUSTOM_ALIAS(goto_jump_at_cursor_direct)
#define goto_jump_at_cursor_same_panel      CUSTOM_ALIAS(goto_jump_at_cursor_same_panel_direct)
#define goto_next_jump                      CUSTOM_ALIAS(goto_next_jump_direct)
#define goto_prev_jump                      CUSTOM_ALIAS(goto_prev_jump_direct)
#define goto_next_jump_no_skips             CUSTOM_ALIAS(goto_next_jump_no_skips_direct)
#define goto_prev_jump_no_skips             CUSTOM_ALIAS(goto_prev_jump_no_skips_direct)
#define goto_first_jump                     CUSTOM_ALIAS(goto_first_jump_direct)
#define newline_or_goto_position            CUSTOM_ALIAS(newline_or_goto_position_direct)
#define newline_or_goto_position_same_panel CUSTOM_ALIAS(newline_or_goto_position_same_panel_direct)

#else

#define goto_jump_at_cursor                 CUSTOM_ALIAS(goto_jump_at_cursor_sticky)
#define goto_jump_at_cursor_same_panel      CUSTOM_ALIAS(goto_jump_at_cursor_same_panel_sticky)
#define goto_next_jump                      CUSTOM_ALIAS(goto_next_jump_sticky)
#define goto_prev_jump                      CUSTOM_ALIAS(goto_prev_jump_sticky)
#define goto_next_jump_no_skips             CUSTOM_ALIAS(goto_next_jump_no_skips_sticky)
#define goto_prev_jump_no_skips             CUSTOM_ALIAS(goto_prev_jump_no_skips_sticky)
#define goto_first_jump                     CUSTOM_ALIAS(goto_first_jump_sticky)
#define newline_or_goto_position            CUSTOM_ALIAS(newline_or_goto_position_sticky)
#define newline_or_goto_position_same_panel CUSTOM_ALIAS(newline_or_goto_position_same_panel_sticky)

#endif

#define seek_error               CUSTOM_ALIAS(seek_jump)
#define goto_next_error          CUSTOM_ALIAS(goto_next_jump)
#define goto_prev_error          CUSTOM_ALIAS(goto_prev_jump)
#define goto_next_error_no_skips CUSTOM_ALIAS(goto_next_jump_no_skips)
#define goto_prev_error_no_skips CUSTOM_ALIAS(goto_prev_jump_no_skips)
#define goto_first_error         CUSTOM_ALIAS(goto_first_jump)

// BOTTOM

