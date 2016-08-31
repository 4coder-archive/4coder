
#ifndef FCODER_JUMP_PARSING
#define FCODER_JUMP_PARSING

struct Jump_Location{
    String file;
    int32_t line;
    int32_t column;
};

static void
jump_to_location(Application_Links *app, View_Summary *view, Jump_Location *l){
    view_open_file(app, view, l->file.str, l->file.size, false);
    app->view_set_cursor(app, view, seek_line_char(l->line, l->column), true);
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
parse_error(String line, Jump_Location *location,
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
goto_error(Application_Links *app,
           Partition *part,
           View_Summary *view, int32_t line,
           Jump_Location *location,
           int32_t skip_sub_errors){
    
    int32_t result = false;
    String line_str = {0};
    Buffer_Summary buffer = app->get_buffer(app, view->buffer_id, AccessAll);
    if (read_line(app, part, &buffer, line, &line_str)){
        int32_t colon_char = 0;
        if (parse_error(line_str, location, skip_sub_errors, &colon_char)){
            result = true;
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = app->get_active_view(app, AccessProtected);
    
    Jump_Location location = {0};
    if (goto_error(app, &global_part,
                   &view, view.cursor.line,
                   &location, false)){
        
        exec_command(app, change_active_panel);
        view = app->get_active_view(app, AccessAll);
        jump_to_location(app, &view, &location);
    }
    
    end_temp_memory(temp);
        
}

//
// Error Jumping
//

struct Prev_Jump{
    int32_t buffer_id;
    int32_t line;
};

static Prev_Jump null_location = {0};
static Prev_Jump prev_location = {0};

// TODO(allen): GIVE THESE THINGS NAMES I CAN FUCKING UNDERSTAND
static int32_t
next_error(Application_Links *app,
           Partition *part,
           View_Summary *comp_out, int32_t *start_line,
           Jump_Location *location,
           int32_t skip_sub_errors,
           int32_t direction,
           int32_t *colon_char){
    
    int32_t result = false;
    int32_t line = *start_line + direction;
    String line_str = {0};
    Buffer_Summary buffer = app->get_buffer(app, comp_out->buffer_id, AccessAll);
    for (;;){
        if (read_line(app, part, &buffer, line, &line_str)){
            if (parse_error(line_str, location, skip_sub_errors, colon_char)){
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
    
    *start_line = line;
    
    return(result);
}

static Prev_Jump
jump_location_store(Application_Links *app, Jump_Location loc){
    Prev_Jump result = {0};
    Buffer_Summary buffer =
        app->get_buffer_by_name(app, loc.file.str, loc.file.size, AccessAll);
    
    if (buffer.exists){
        result.buffer_id = buffer.buffer_id;
        result.line = loc.line;
    }
    
    return(result);
}

static int32_t
seek_error_internal(Application_Links *app, Partition *part,
                    int32_t skip_sub_errors, int32_t dir, Jump_Location *loc){
    int32_t result = false;
    
    Jump_Location location = {0};
    Buffer_Summary buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    if (buffer.exists){
        View_Summary view = get_first_view_with_buffer(app, buffer.buffer_id);
        int32_t line = view.cursor.line;
        
        int32_t colon_char = 0;
        if (next_error(app, part, &view, &line, &location,
                       skip_sub_errors, dir, &colon_char)){
            
            View_Summary active_view = app->get_active_view(app, AccessAll);
            if (active_view.view_id == view.view_id){
                exec_command(app, change_active_panel_regular);
                active_view = app->get_active_view(app, AccessAll);
            }
            
            jump_to_location(app, &active_view, &location);
            app->view_set_cursor(app, &view, seek_line_char(line, colon_char+1), true);
            result = true;
            if (loc){
                *loc = location;
            }
        }
    }
    
    return(result);
}


static int32_t
skip_this_jump(Prev_Jump prev, Prev_Jump jump){
    int32_t result = false;
    if (prev.buffer_id != 0 && prev.buffer_id == jump.buffer_id &&
        prev.line == jump.line){
        result = true;
    }
    return(result);
}

static int32_t
seek_error_skip_repeats(Application_Links *app, Partition *part,
                        int32_t skip_sub_error, int32_t dir){
    int32_t result = true;
    Jump_Location location = {0};
    Prev_Jump jump = {0};
    do{
        Temp_Memory temp = begin_temp_memory(part);
        if (seek_error_internal(app, part, skip_sub_error, dir, &location)){
            jump = jump_location_store(app, location);
            result = true;
        }
        else{
            jump.buffer_id = 0;
            result = false;
        }
        end_temp_memory(temp);
    }while(skip_this_jump(prev_location, jump));
    prev_location = jump;
    return(result);
}

static int32_t
seek_error_no_skip(Application_Links *app, Partition *part,
                   int32_t skip_sub_error, int32_t dir){
    int32_t result = true;
    Jump_Location location = {0};
    Prev_Jump jump = {0};
    
    Temp_Memory temp = begin_temp_memory(part);
    if (seek_error_internal(app, part, skip_sub_error, dir, &location)){
        jump = jump_location_store(app, location);
        result = true;
    }
    else{
        result = false;
    }
    end_temp_memory(temp);
    
    prev_location = jump;
    return(result);
}

static int32_t
seek_error(Application_Links *app, Partition *part,
           int32_t skip_sub_error, int32_t skip_same_line, int32_t dir){
    if (skip_same_line){
        seek_error_skip_repeats(app, part, skip_sub_error, dir);
    }
    else{
        seek_error_no_skip(app, part, skip_sub_error, dir);
    }
}

CUSTOM_COMMAND_SIG(goto_next_error){
    seek_error_skip_repeats(app, &global_part, true, 1);
}

CUSTOM_COMMAND_SIG(goto_prev_error){
    seek_error_skip_repeats(app, &global_part, true, -1);
}

CUSTOM_COMMAND_SIG(goto_next_error_no_skips){
    seek_error_no_skip(app, &global_part, true, 1);
}

CUSTOM_COMMAND_SIG(goto_prev_error_no_skips){
    seek_error_no_skip(app, &global_part, true, -1);
}

CUSTOM_COMMAND_SIG(goto_first_error){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary active_view = app->get_active_view(app, AccessAll);
    app->view_set_cursor(app, &active_view, seek_pos(0), true);
    
    Jump_Location location = {0};
    prev_location = null_location;
    seek_error_internal(app, &global_part, true, 1, &location);
    prev_location = jump_location_store(app, location);
    end_temp_memory(temp);
}

#endif

