
#ifndef FCODER_JUMP_PARSING
#define FCODER_JUMP_PARSING

struct Jump_Location{
    String file;
    int line;
    int column;
};

static void
jump_to_location(Application_Links *app, View_Summary *view, Jump_Location *l){
    view_open_file(app, view, l->file.str, l->file.size, false);
    app->view_set_cursor(app, view, seek_line_char(l->line, l->column), true);
}

static int
gcc_style_verify(String line, int colon_pos){
    int result = false;
    if (colon_pos < line.size){
        result = true;
    }
    return(result);
}

static int
ms_style_verify(String line, int paren_pos){
    int result = false;
    
    String line_part = substr(line, paren_pos);
    if (match_part(line_part, ") : ")){
        result = true;
    }
    
    return(result);
}

static int
parse_error(String line, Jump_Location *location,
            int skip_sub_errors, int *colon_char){
    int result = false;
    
    int colon_pos = find(line, 0, ')');
    if (ms_style_verify(line, colon_pos)){
        colon_pos = find(line, colon_pos, ':');
        if (colon_pos < line.size){
            String location_str = substr(line, 0, colon_pos);
            
            if (!(skip_sub_errors && line.str[0] == ' ')){
                location_str = skip_chop_whitespace(location_str);
                
                int paren_pos = find(location_str, 0, '(');
                if (paren_pos < location_str.size){
                    String file = substr(location_str, 0, paren_pos);
                    file = skip_chop_whitespace(file);
                    
                    int close_pos = find(location_str, 0, ')') + 1;
                    if (close_pos == location_str.size && file.size > 0){
                        String line_number = substr(location_str,
                                                    paren_pos+1,
                                                    close_pos-paren_pos-2);
                        line_number = skip_chop_whitespace(line_number);
                        
                        if (line_number.size > 0){
                            location->file = file;
                            
                            int comma_pos = find(line_number, 0, ',');
                            if (comma_pos < line_number.size){
                                int start = comma_pos+1;
                                String column_number = substr(line_number, start, line_number.size-start);
                                line_number = substr(line_number, 0, comma_pos);
                                
                                location->line = str_to_int(line_number);
                                location->column = str_to_int(column_number);
                            }
                            else{
                                location->line = str_to_int(line_number);
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
        int colon_pos1 = find(line, 0, ':');
        if (colon_pos1 == 1){
            if (line.size > colon_pos1+1){
                if (char_is_slash(line.str[colon_pos1+1])){
                    colon_pos1 = find(line, colon_pos1+1, ':');
                }
            }
        }
        
        int colon_pos2 = find(line, colon_pos1+1, ':');
        int colon_pos3 = find(line, colon_pos2+1, ':');
        
        if (gcc_style_verify(line, colon_pos3)){
            String filename = substr(line, 0, colon_pos1);
            String line_number = substr(line, colon_pos1+1, colon_pos2 - colon_pos1 - 1);
            String column_number = substr(line, colon_pos2+1, colon_pos3 - colon_pos2 - 1);
            
            if (filename.size > 0 &&
                line_number.size > 0 &&
                column_number.size > 0){
                location->file = filename;
                location->line = str_to_int(line_number);
                location->column = str_to_int(column_number);
                *colon_char = colon_pos3;
                result = true;
            }
        }
        else{
            int colon_pos1 = find(line, 0, ':');
            int colon_pos2 = find(line, colon_pos1+1, ':');
            
            if (colon_pos2 < line.size){
                String filename = substr(line, 0, colon_pos1);
                String line_number = substr(line, colon_pos1+1, colon_pos2 - colon_pos1 - 1);
                
                if (str_is_int(line_number)){
                    if (filename.size > 0 && line_number.size > 0){
                        location->file = filename;
                        location->line = str_to_int(line_number);
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

static int
goto_error(Application_Links *app,
           Partition *part,
           View_Summary *view, int line,
           Jump_Location *location,
           int skip_sub_errors){
    
    int result = false;
    String line_str = {0};
    Buffer_Summary buffer = app->get_buffer(app, view->buffer_id, AccessAll);
    if (read_line(app, part, &buffer, line, &line_str)){
        int colon_char = 0;
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
    int buffer_id;
    int line;
};

static Prev_Jump null_location = {0};
static Prev_Jump prev_location = {0};

// TODO(allen): GIVE THESE THINGS NAMES I CAN FUCKING UNDERSTAND
static int
next_error(Application_Links *app,
           Partition *part,
           View_Summary *comp_out, int *start_line,
           Jump_Location *location,
           int direction,
           int skip_sub_errors,
           int *colon_char){
    
    int result = false;
    int line = *start_line + direction;
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

static int
seek_error(Application_Links *app, Partition *part, int direction, int skip_sub_errors, Jump_Location *loc){
    int result = false;
    
    Jump_Location location = {0};
    Buffer_Summary buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    if (buffer.exists){
        View_Summary view = get_first_view_with_buffer(app, buffer.buffer_id);
        int line = view.cursor.line;
        
        int colon_char = 0;
        if (next_error(app, part, &view, &line, &location,
                       skip_sub_errors, direction, &colon_char)){
            
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

static int
skip_this_jump(Prev_Jump prev, Prev_Jump jump){
    int result = false;
    if (prev.buffer_id != 0 && prev.buffer_id == jump.buffer_id &&
        prev.line == jump.line){
        result = true;
    }
    return(result);
}

static int
seek_error_skip_repeats(Application_Links *app, Partition *part,
                        int skip_sub_error, int dir){
    int result = true;
    Jump_Location location = {0};
    Prev_Jump jump = {0};
    do{
        Temp_Memory temp = begin_temp_memory(part);
        if (seek_error(app, part, skip_sub_error, dir, &location)){
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

CUSTOM_COMMAND_SIG(goto_next_error){
    seek_error_skip_repeats(app, &global_part, true, 1);
}

CUSTOM_COMMAND_SIG(goto_prev_error){
    seek_error_skip_repeats(app, &global_part, true, -1);
}

CUSTOM_COMMAND_SIG(goto_first_error){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary active_view = app->get_active_view(app, AccessAll);
    app->view_set_cursor(app, &active_view, seek_pos(0), true);
    
    Jump_Location location = {0};
    seek_error(app, &global_part, true, 1, &location);
    prev_location = jump_location_store(app, location);
    end_temp_memory(temp);
}

#endif

