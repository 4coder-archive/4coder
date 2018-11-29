/*
4coder_jumping.cpp - Routines commonly used when writing code to jump to locations and seek through jump lists.
*/

// TOP

static bool32
ms_style_verify(String line, int32_t left_paren_pos, int32_t right_paren_pos){
    int32_t result = false;
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
            int32_t comma_pos = find_s_char(number, 0, ',');
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

static int32_t
try_skip_rust_arrow(String line){
    int32_t pos = 0;
    if (match_part(line, "-->")){
        String sub = substr_tail(line, 3);
        sub = skip_chop_whitespace(sub);
        pos = (int32_t)(sub.str - line.str);
    }
    return(pos);
}

static bool32
check_is_note(String line, int32_t colon_pos){
    bool32 is_note = false;
    int32_t note_pos = find_substr(line, colon_pos, make_lit_string("note"));
    if (note_pos < line.size){
        bool32 is_all_whitespace = true;
        for (int32_t i = colon_pos + 1; i < note_pos; i += 1){
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

static bool32
parse_jump_location(String line, Name_Line_Column_Location *location, int32_t *colon_char, bool32 *is_sub_error){
    bool32 result = false;
    *is_sub_error = (line.str[0] == ' ');
    
    int32_t whitespace_length = 0;
    line = skip_chop_whitespace(line, &whitespace_length);
    
    int32_t colon_pos = 0;
    bool32 is_ms_style = false;
    
    int32_t left_paren_pos = find_s_char(line, 0, '(');
    int32_t right_paren_pos = find_s_char(line, left_paren_pos, ')');
    while (!is_ms_style && right_paren_pos < line.size){
        if (ms_style_verify(line, left_paren_pos, right_paren_pos)){
            is_ms_style = true;
            colon_pos = find_s_char(line, right_paren_pos, ':');
            if (colon_pos < line.size){
                if (check_is_note(line, colon_pos)){
                    *is_sub_error = true;
                }
                
                String location_str = substr(line, 0, colon_pos);
                
                location_str = skip_chop_whitespace(location_str);
                
                int32_t close_pos = right_paren_pos;
                int32_t open_pos = left_paren_pos;
                
                if (0 < open_pos && open_pos < location_str.size){
                    String file = substr(location_str, 0, open_pos);
                    file = skip_chop_whitespace(file);
                    
                    if (file.size > 0){
                        String line_number = substr(location_str,
                                                    open_pos+1,
                                                    close_pos-open_pos-1);
                        line_number = skip_chop_whitespace(line_number);
                        
                        if (line_number.size > 0){
                            location->file = file;
                            
                            int32_t comma_pos = find_s_char(line_number, 0, ',');
                            if (comma_pos < line_number.size){
                                int32_t start = comma_pos+1;
                                String column_number = substr(line_number, start, line_number.size-start);
                                line_number = substr(line_number, 0, comma_pos);
                                
                                location->line = str_to_int_s(line_number);
                                location->column = str_to_int_s(column_number);
                            }
                            else{
                                location->line = str_to_int_s(line_number);
                                location->column = 1;
                            }
                            
                            *colon_char = colon_pos + whitespace_length;
                            result = true;
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
    
    if (!is_ms_style){
        int32_t start = try_skip_rust_arrow(line);
        
        int32_t colon_pos1 = find_s_char(line, start, ':');
        if (line.size > colon_pos1 + 1){
            if (char_is_slash(line.str[colon_pos1 + 1])){
                colon_pos1 = find_s_char(line, colon_pos1 + 1, ':');
            }
        }
        
        int32_t colon_pos2 = find_s_char(line, colon_pos1 + 1, ':');
        int32_t colon_pos3 = find_s_char(line, colon_pos2 + 1, ':');
        
        if (colon_pos3 < line.size){
            if (check_is_note(line, colon_pos3)){
                *is_sub_error = true;
            }
            
            String filename = substr(line, start, colon_pos1 - start);
            String line_number = substr(line, colon_pos1 + 1, colon_pos2 - colon_pos1 - 1);
            String column_number = substr(line, colon_pos2 + 1, colon_pos3 - colon_pos2 - 1);
            
            if (filename.size > 0 &&
                line_number.size > 0 &&
                column_number.size > 0){
                location->file = filename;
                location->line = str_to_int_s(line_number);
                location->column = str_to_int_s(column_number);
                *colon_char = colon_pos3 + whitespace_length;
                result = true;
            }
        }
        else{
            if (colon_pos2 < line.size){
                if (check_is_note(line, colon_pos2)){
                    *is_sub_error = true;
                }
                
                String filename = substr(line, 0, colon_pos1);
                String line_number = substr(line, colon_pos1 + 1, colon_pos2 - colon_pos1 - 1);
                
                if (str_is_int_s(line_number)){
                    if (filename.size > 0 && line_number.size > 0){
                        location->file = filename;
                        location->line = str_to_int_s(line_number);
                        location->column = 0;
                        *colon_char = colon_pos2 + whitespace_length;
                        result = true;
                    }
                }
            }
        }
    }
    
    if (!result){
        *is_sub_error = false;
    }
    
    return(result);
}

static bool32
parse_jump_location(String line, bool32 skip_sub_error, Name_Line_Column_Location *location, int32_t *colon_char){
    bool32 is_sub_error = false;
    bool32 result = parse_jump_location(line, location, colon_char, &is_sub_error);
    if (is_sub_error && skip_sub_error){
        result = false;
    }
    return(result);
}

static int32_t
parse_jump_from_buffer_line(Application_Links *app, Partition *arena,
                            int32_t buffer_id, int32_t line,
                            bool32 skip_sub_errors, Name_Line_Column_Location *location){
    int32_t result = false;
    String line_str = {};
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    if (read_line(app, arena, &buffer, line, &line_str)){
        int32_t colon_char = 0;
        if (parse_jump_location(line_str, skip_sub_errors, location, &colon_char)){
            result = true;
        }
    }
    return(result);
}

////////////////////////////////

static bool32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, Name_Line_Column_Location *location){
    bool32 result = open_file(app, buffer, location->file.str, location->file.size, false, true);
    return(result);
}

static bool32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, ID_Pos_Jump_Location *location){
    *buffer = get_buffer(app, location->buffer_id, AccessAll);
    bool32 result = false;
    if (buffer->exists){
        result = true;
    }
    return(result);
}

static void
switch_to_existing_view(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    if (!(view->exists && view->buffer_id == buffer->buffer_id)){
        View_Summary existing_view = get_first_view_with_buffer(app, buffer->buffer_id);
        if (existing_view.exists){
            *view = existing_view;
        }
    }
}

static void
set_view_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, Buffer_Seek seek){
    if (view->buffer_id != buffer->buffer_id){
        view_set_buffer(app, view, buffer->buffer_id, 0);
    }
    view_set_cursor(app, view, seek, true);
}

static void
jump_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, Name_Line_Column_Location location){
    set_active_view(app, view);
    set_view_to_location(app, view, buffer, seek_line_char(location.line, location.column));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

static void
jump_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, ID_Pos_Jump_Location location){
    set_active_view(app, view);
    set_view_to_location(app, view, buffer, seek_pos(location.pos));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

////////////////////////////////

static bool32
seek_next_jump_in_buffer(Application_Links *app, Partition *part,
                         int32_t buffer_id, int32_t first_line, bool32 skip_sub_errors,
                         int32_t direction,
                         int32_t *line_out, int32_t *colon_index_out, Name_Line_Column_Location *location_out){
    
    Assert(direction == 1 || direction == -1);
    
    bool32 result = false;
    int32_t line = first_line;
    String line_str = {};
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    for (;;){
        if (read_line(app, part, &buffer, line, &line_str)){
            if (parse_jump_location(line_str, skip_sub_errors, location_out, colon_index_out)){
                result = true;
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
    
    *line_out = line;
    
    return(result);
}

static ID_Line_Column_Jump_Location
convert_name_based_to_id_based(Application_Links *app, Name_Line_Column_Location loc){
    ID_Line_Column_Jump_Location result = {};
    Buffer_Summary buffer = get_buffer_by_name(app, loc.file.str, loc.file.size, AccessAll);
    
    if (buffer.exists){
        result.buffer_id = buffer.buffer_id;
        result.line = loc.line;
        result.column = loc.column;
    }
    
    return(result);
}

static int32_t
seek_next_jump_in_view(Application_Links *app, Partition *part, View_Summary *view, int32_t skip_sub_errors, int32_t direction, int32_t *line_out, int32_t *colon_index_out, Name_Line_Column_Location *location_out){
    int32_t result = false;
    
    Name_Line_Column_Location location = {};
    int32_t line = view->cursor.line;
    int32_t colon_index = 0;
    if (seek_next_jump_in_buffer(app, part, view->buffer_id, line+direction, skip_sub_errors, direction, &line, &colon_index, &location)){
        result = true;
        *line_out = line;
        *colon_index_out = colon_index;
        *location_out = location;
    }
    
    return(result);
}

static bool32
skip_this_jump(ID_Line_Column_Jump_Location prev, ID_Line_Column_Jump_Location jump){
    bool32 result = false;
    if (prev.buffer_id != 0 && prev.buffer_id == jump.buffer_id && prev.line == jump.line && prev.column <= jump.column){
        result = true;
    }
    return(result);
}

static bool32
advance_cursor_in_jump_view(Application_Links *app, Partition *part, View_Summary *view, int32_t skip_repeats, int32_t skip_sub_error, int32_t direction, Name_Line_Column_Location *location_out){
    bool32 result = true;
    
    Name_Line_Column_Location location = {};
    ID_Line_Column_Jump_Location jump = {};
    int32_t line = 0;
    int32_t colon_index = 0;
    
    do{
        Temp_Memory temp = begin_temp_memory(part);
        if (seek_next_jump_in_view(app, part, view, skip_sub_error, direction, &line, &colon_index, &location)){
            jump = convert_name_based_to_id_based(app, location);
            view_set_cursor(app, view, seek_line_char(line, colon_index+1), true);
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

static bool32
seek_jump(Application_Links *app, Partition *part, bool32 skip_repeats, bool32 skip_sub_errors, int32_t direction){
    bool32 result = false;
    
    View_Summary view = get_view_for_locked_jump_buffer(app);
    if (view.exists){
        Name_Line_Column_Location location = {};
        if (advance_cursor_in_jump_view(app, &global_part, &view, skip_repeats, skip_sub_errors, direction, &location)){
            
            Buffer_Summary buffer = {};
            if (get_jump_buffer(app, &buffer, &location)){
                View_Summary target_view = get_active_view(app, AccessAll);
                if (target_view.view_id == view.view_id){
                    change_active_panel(app);
                    target_view = get_active_view(app, AccessAll);
                }
                switch_to_existing_view(app, &target_view, &buffer);
                jump_to_location(app, &target_view, &buffer, location);
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

