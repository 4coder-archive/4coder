/*
4coder_jump_parsing.cpp - Commands and helpers for parsing jump locations from 
compiler errors and jumping to them in the corresponding buffer.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_JUMP_PARSING) && !defined(FCODER_JUMP_COMMANDS)
#define FCODER_JUMP_PARSING

#define FCODER_JUMP_COMMANDS

#include "4coder_default_framework.h"
#include "4coder_helper/4coder_long_seek.h"
#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_jump_parsing.h"

#include "4coder_lib/4coder_mem.h"
#include "4coder_jumping.h"

CUSTOM_COMMAND_SIG(goto_jump_at_cursor){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = get_active_view(app, AccessProtected);
    
    Name_Based_Jump_Location location = {0};
    if (parse_jump_from_buffer_line(app, &global_part, view.buffer_id, view.cursor.line, false, &location)){
        change_active_panel(app);
        View_Summary target_view = get_active_view(app, AccessAll);
        
        Buffer_Summary buffer = {0};
        if (get_jump_buffer(app, &buffer, &location)){
            switch_to_existing_view(app, &target_view, &buffer);
            jump_to_location(app, &target_view, &buffer, location);
        }
    }
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel){
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = get_active_view(app, AccessProtected);
    
    Name_Based_Jump_Location location = {0};
    if (parse_jump_from_buffer_line(app, &global_part, view.buffer_id, view.cursor.line, false, &location)){
        View_Summary target_view = view;
        
        Buffer_Summary buffer = {0};
        if (get_jump_buffer(app, &buffer, &location)){
            jump_to_location(app, &target_view, &buffer, location);
        }
    }
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(goto_next_jump){
    bool32 skip_repeats = true;
    bool32 skip_sub_errors = true;
    int32_t dir = 1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_jump){
    bool32 skip_repeats = true;
    bool32 skip_sub_errors = true;
    int32_t dir = -1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_next_jump_no_skips){
    bool32 skip_repeats = false;
    bool32 skip_sub_errors = true;
    int32_t dir = 1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips){
    bool32 skip_repeats = false;
    bool32 skip_sub_errors = true;
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
        goto_jump_at_cursor(app);
        lock_jump_buffer(buffer);
    }
    else{
        write_character(app);
    }
}

CUSTOM_COMMAND_SIG(newline_or_goto_position_same_panel){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    if (buffer.lock_flags & AccessProtected){
        goto_jump_at_cursor_same_panel(app);
        lock_jump_buffer(buffer);
    }
    else{
        write_character(app);
    }
}

#define seek_error                  seek_jump
#define goto_next_error             goto_next_jump
#define goto_prev_error             goto_prev_jump
#define goto_next_error_no_skips    goto_next_jump_no_skips
#define goto_first_error            goto_first_jump

#endif

// BOTTOM

