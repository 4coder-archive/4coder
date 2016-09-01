
#ifndef FCODER_JUMP_PARSING
#define FCODER_JUMP_PARSING

typedef struct Name_Based_Jump_Location{
    String file;
    int32_t line;
    int32_t column;
} Name_Based_Jump_Location;

typedef struct ID_Based_Jump_Location{
    int32_t buffer_id;
    int32_t line;
    int32_t column;
} ID_Based_Jump_Location;
static ID_Based_Jump_Location null_location = {0};

static void
jump_to_location(Application_Links *app, View_Summary *view, Name_Based_Jump_Location *l){
    if (view_open_file(app, view, l->file.str, l->file.size, true)){
        app->view_set_cursor(app, view, seek_line_char(l->line, l->column), true);
    }
}

static int32_t
ms_style_verify(String line, int32_t paren_pos){
    int32_t result = false;
    
    String line_part = substr_tail(line, paren_pos);
    if (match_part_sc(line_part, ") : ")){
        result = true;
    }
    else if (match_part_sc(line_part, "): ")){
        result = true;
    }
    
    return(result);
}

static int32_t
parse_jump_location(String line, Name_Based_Jump_Location *location,
                    int32_t skip_sub_errors, int32_t *colon_char){
    int32_t result = false;
    
    String original_line = line;
    line = skip_chop_whitespace(line);
    
    int32_t colon_pos = 0;
    int32_t is_ms_style = 0;
    
    int32_t paren_pos = find_s_char(line, 0, ')');
    while (!is_ms_style && paren_pos < line.size){
        if (ms_style_verify(line, paren_pos)){
            is_ms_style = 1;
            colon_pos = find_s_char(line, paren_pos, ':');
            if (colon_pos < line.size){
                String location_str = substr(line, 0, colon_pos);
                
                if (!(skip_sub_errors && original_line.str[0] == ' ')){
                    location_str = skip_chop_whitespace(location_str);
                    
                    int32_t close_pos = paren_pos;
                    int32_t open_pos = rfind_s_char(location_str, close_pos, '(');
                    
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
                                
                                *colon_char = colon_pos;
                                result = true;
                            }
                        }
                    }
                }
            }
        }
        else{
            paren_pos = find_s_char(line, paren_pos+1, ')');
        }
    }
    
    if (!is_ms_style){
        int32_t colon_pos1 = find_s_char(line, 0, ':');
        if (line.size > colon_pos1+1){
            if (char_is_slash(line.str[colon_pos1+1])){
                colon_pos1 = find_s_char(line, colon_pos1+1, ':');
            }
        }
        
        int32_t colon_pos2 = find_s_char(line, colon_pos1+1, ':');
        int32_t colon_pos3 = find_s_char(line, colon_pos2+1, ':');
        
        if (colon_pos3+1 < line.size && line.str[colon_pos3+1] == ' '){
            String filename = substr(line, 0, colon_pos1);
            String line_number = substr(line, colon_pos1+1, colon_pos2 - colon_pos1 - 1);
            String column_number = substr(line, colon_pos2+1, colon_pos3 - colon_pos2 - 1);
            
            if (filename.size > 0 &&
                line_number.size > 0 &&
                column_number.size > 0){
                location->file = filename;
                location->line = str_to_int_s(line_number);
                location->column = str_to_int_s(column_number);
                *colon_char = colon_pos3;
                result = true;
            }
        }
        else{
            colon_pos1 = find_s_char(line, 0, ':');
            if (line.size > colon_pos1+1){
                if (char_is_slash(line.str[colon_pos1+1])){
                    colon_pos1 = find_s_char(line, colon_pos1+1, ':');
                }
            }
            
            colon_pos2 = find_s_char(line, colon_pos1+1, ':');
            
            if (colon_pos2+1 < line.size && line.str[colon_pos2+1] == ' '){
                String filename = substr(line, 0, colon_pos1);
                String line_number = substr(line, colon_pos1+1, colon_pos2 - colon_pos1 - 1);
                
                if (str_is_int_s(line_number)){
                    if (filename.size > 0 && line_number.size > 0){
                        location->file = filename;
                        location->line = str_to_int_s(line_number);
                        location->column = 0;
                        *colon_char = colon_pos2;
                        result = true;
                    }
                }
            }
        }
    }
    
    return(result);
}

static int32_t
parse_jump_from_buffer_line(Application_Links *app,
                            Partition *part,
                            int32_t buffer_id,
                            int32_t line,
                            int32_t skip_sub_errors,
                            Name_Based_Jump_Location *location){
    
    int32_t result = false;
    String line_str = {0};
    Buffer_Summary buffer = app->get_buffer(app, buffer_id, AccessAll);
    if (read_line(app, part, &buffer, line, &line_str)){
        int32_t colon_char = 0;
        if (parse_jump_location(line_str, location, skip_sub_errors, &colon_char)){
            result = true;
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = app->get_active_view(app, AccessProtected);
    
    Name_Based_Jump_Location location = {0};
    if (parse_jump_from_buffer_line(app, &global_part,
                                    view.buffer_id, view.cursor.line, false,
                                    &location)){
        
        exec_command(app, change_active_panel);
        view = app->get_active_view(app, AccessAll);
        jump_to_location(app, &view, &location);
    }
    
    end_temp_memory(temp);
}


//
// Error Jumping
//

static int32_t
seek_next_jump_in_buffer(Application_Links *app,
                         Partition *part,
                         int32_t buffer_id,
                         int32_t first_line,
                         bool32 skip_sub_errors,
                         int32_t direction,
                         int32_t *line_out,
                         int32_t *colon_index_out,
                         Name_Based_Jump_Location *location_out){
    
    Assert(direction == 1 || direction == -1);
    
    int32_t result = false;
    int32_t line = first_line;
    String line_str = {0};
    Buffer_Summary buffer = app->get_buffer(app, buffer_id, AccessAll);
    for (;;){
        if (read_line(app, part, &buffer, line, &line_str)){
            if (parse_jump_location(line_str, location_out, skip_sub_errors, colon_index_out)){
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

static ID_Based_Jump_Location
convert_name_based_to_id_based(Application_Links *app, Name_Based_Jump_Location loc){
    ID_Based_Jump_Location result = {0};
    Buffer_Summary buffer =
        app->get_buffer_by_name(app, loc.file.str, loc.file.size, AccessAll);
    
    if (buffer.exists){
        result.buffer_id = buffer.buffer_id;
        result.line = loc.line;
        result.column = loc.column;
    }
    
    return(result);
}

static int32_t
seek_next_jump_in_view(Application_Links *app,
                       Partition *part,
                       View_Summary *view,
                       int32_t skip_sub_errors,
                       int32_t direction,
                       int32_t *line_out,
                       int32_t *colon_index_out,
                       Name_Based_Jump_Location *location_out){
    int32_t result = false;
    
    Name_Based_Jump_Location location = {0};
    int32_t line = view->cursor.line;
    int32_t colon_index = 0;
    if (seek_next_jump_in_buffer(app, part, view->buffer_id,
                                 line+direction, skip_sub_errors, direction,
                                 &line, &colon_index, &location)){
        result = true;
        *line_out = line;
        *colon_index_out = colon_index;
        *location_out = location;
    }
    
    return(result);
}

static int32_t
skip_this_jump(ID_Based_Jump_Location prev, ID_Based_Jump_Location jump){
    int32_t result = false;
    if (prev.buffer_id != 0 &&
        prev.buffer_id == jump.buffer_id &&
        prev.line == jump.line &&
        prev.column <= jump.column){
        result = true;
    }
    return(result);
}

static ID_Based_Jump_Location prev_location = {0};

static int32_t
advance_cursor_in_jump_view(Application_Links *app,
                            Partition *part,
                            View_Summary *view,
                            int32_t skip_repeats,
                            int32_t skip_sub_error,
                            int32_t direction,
                            Name_Based_Jump_Location *location_out){
    int32_t result = true;
    
    Name_Based_Jump_Location location = {0};
    ID_Based_Jump_Location jump = {0};
    int32_t line = 0, colon_index = 0;
    
    do{
        Temp_Memory temp = begin_temp_memory(part);
        if (seek_next_jump_in_view(app, part, view,
                                   skip_sub_error, direction,
                                   &line, &colon_index, &location)){
            jump = convert_name_based_to_id_based(app, location);
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
        app->view_set_cursor(app, view, seek_line_char(line, colon_index+1), true);
    }
    
    prev_location = jump;
    
    return(result);
}

static char locked_buffer_space[256];
static String locked_buffer = make_fixed_width_string(locked_buffer_space);

static void
unlock_jump_buffer(){
    locked_buffer.size = 0;
}

static void
lock_jump_buffer(char *name, int32_t size){
    copy(&locked_buffer, make_string(name, size));
}

static void
lock_jump_buffer(Buffer_Summary buffer){
    copy(&locked_buffer, make_string(buffer.buffer_name, buffer.buffer_name_len));
}

static View_Summary
get_view_for_locked_jump_buffer(Application_Links *app){
    View_Summary view = {0};
    
    if (locked_buffer.size > 0){
        Buffer_Summary buffer = app->get_buffer_by_name(app, locked_buffer.str, locked_buffer.size, AccessAll);
        if (buffer.exists){
            view = get_first_view_with_buffer(app, buffer.buffer_id);
        }
        else{
            unlock_jump_buffer();
        }
    }
    
    return(view);
}

static int32_t
seek_error(Application_Links *app,
           Partition *part,
           int32_t skip_repeats,
           int32_t skip_sub_errors,
           int32_t direction){
    int32_t result = 0;
    
    View_Summary view = get_view_for_locked_jump_buffer(app);
    if (view.exists){
        
        Name_Based_Jump_Location location = {0};
        if (advance_cursor_in_jump_view(app, &global_part, &view,
                                        skip_repeats, skip_sub_errors, direction,
                                        &location)){
            View_Summary active_view = app->get_active_view(app, AccessAll);
            if (active_view.view_id == view.view_id){
                exec_command(app, change_active_panel);
                active_view = app->get_active_view(app, AccessAll);
            }
            
            jump_to_location(app, &active_view, &location);
            result = 1;
        }
    }
    
    return(result);
}


CUSTOM_COMMAND_SIG(goto_next_error){
    int32_t skip_repeats = true;
    int32_t skip_sub_errors = true;
    int32_t dir = 1;
    seek_error(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_error){
    int32_t skip_repeats = true;
    int32_t skip_sub_errors = true;
    int32_t dir = -1;
    seek_error(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_next_error_no_skips){
    int32_t skip_repeats = false;
    int32_t skip_sub_errors = true;
    int32_t dir = 1;
    seek_error(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_error_no_skips){
    int32_t skip_repeats = false;
    int32_t skip_sub_errors = true;
    int32_t dir = -1;
    seek_error(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_first_error){
    Temp_Memory temp = begin_temp_memory(&global_part);
    
    View_Summary view = get_view_for_locked_jump_buffer(app);
    if (view.exists){
        app->view_set_cursor(app, &view, seek_pos(0), true);
        
        prev_location = null_location;
        seek_error(app, &global_part, false, true, 1);
    }
    end_temp_memory(temp);
}

#endif

