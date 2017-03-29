/*
4coder_jump_parsing.cpp - Commands and helpers for parsing jump locations from 
compiler errors and jumping to them in the corresponding buffer.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_JUMP_PARSING)
#define FCODER_JUMP_PARSING

#include "4coder_default_framework.h"
#include "4coder_helper/4coder_long_seek.h"
#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_jump_parsing.h"

#include "4coder_lib/4coder_mem.h"

CUSTOM_COMMAND_SIG(goto_jump_at_cursor){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = get_active_view(app, AccessProtected);
    
    Name_Based_Jump_Location location = {0};
    if (parse_jump_from_buffer_line(app, &global_part, view.buffer_id, view.cursor.line, false, &location)){
        
        exec_command(app, change_active_panel);
        view = get_active_view(app, AccessAll);
        jump_to_location(app, &view, &location);
    }
    
    end_temp_memory(temp);
}


//
// Error Jumping
//

static int32_t
seek_next_jump_in_buffer(Application_Links *app, Partition *part, int32_t buffer_id, int32_t first_line, bool32 skip_sub_errors, int32_t direction, int32_t *line_out, int32_t *colon_index_out, Name_Based_Jump_Location *location_out){
    
    Assert(direction == 1 || direction == -1);
    
    int32_t result = false;
    int32_t line = first_line;
    String line_str = {0};
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
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
        get_buffer_by_name(app, loc.file.str, loc.file.size, AccessAll);
    
    if (buffer.exists){
        result.buffer_id = buffer.buffer_id;
        result.line = loc.line;
        result.column = loc.column;
    }
    
    return(result);
}

static int32_t
seek_next_jump_in_view(Application_Links *app, Partition *part, View_Summary *view, int32_t skip_sub_errors, int32_t direction, int32_t *line_out, int32_t *colon_index_out, Name_Based_Jump_Location *location_out){
    int32_t result = false;
    
    Name_Based_Jump_Location location = {0};
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

static int32_t
advance_cursor_in_jump_view(Application_Links *app, Partition *part, View_Summary *view, int32_t skip_repeats, int32_t skip_sub_error, int32_t direction, Name_Based_Jump_Location *location_out){
    int32_t result = true;
    
    Name_Based_Jump_Location location = {0};
    ID_Based_Jump_Location jump = {0};
    int32_t line = 0, colon_index = 0;
    
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
        view_set_cursor(app, view, seek_line_char(line, colon_index+1), true);
    }
    
    prev_location = jump;
    
    return(result);
}

static int32_t
seek_jump(Application_Links *app, Partition *part, int32_t skip_repeats, int32_t skip_sub_errors, int32_t direction){
    int32_t result = 0;
    
    View_Summary view = get_view_for_locked_jump_buffer(app);
    if (view.exists){
        
        Name_Based_Jump_Location location = {0};
        if (advance_cursor_in_jump_view(app, &global_part, &view, skip_repeats, skip_sub_errors, direction, &location)){
            View_Summary active_view = get_active_view(app, AccessAll);
            if (active_view.view_id == view.view_id){
                exec_command(app, change_active_panel);
                active_view = get_active_view(app, AccessAll);
            }
            
            jump_to_location(app, &active_view, &location);
            result = 1;
        }
    }
    
    return(result);
}


CUSTOM_COMMAND_SIG(goto_next_jump){
    int32_t skip_repeats = true;
    int32_t skip_sub_errors = true;
    int32_t dir = 1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_jump){
    int32_t skip_repeats = true;
    int32_t skip_sub_errors = true;
    int32_t dir = -1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_next_jump_no_skips){
    int32_t skip_repeats = false;
    int32_t skip_sub_errors = true;
    int32_t dir = 1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips){
    int32_t skip_repeats = false;
    int32_t skip_sub_errors = true;
    int32_t dir = -1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_first_jump){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = get_view_for_locked_jump_buffer(app);
    if (view.exists){
        view_set_cursor(app, &view, seek_pos(0), true);
        prev_location = null_location;
        seek_jump(app, &global_part, false, true, 1);
    }
    end_temp_memory(temp);
}

//
// Insert Newline or Tigger Jump on Read Only Buffer
//

CUSTOM_COMMAND_SIG(newline_or_goto_position){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    if (buffer.lock_flags & AccessProtected){
        exec_command(app, goto_jump_at_cursor);
        lock_jump_buffer(buffer);
    }
    else{
        exec_command(app, write_character);
    }
}

#define seek_error                  seek_jump
#define goto_next_error             goto_next_jump
#define goto_prev_error             goto_prev_jump
#define goto_next_error_no_skips    goto_next_jump_no_skips
#define goto_first_error            goto_first_jump

#endif

// BOTTOM

