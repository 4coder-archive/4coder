/*
4coder_direct_jump.cpp - Commands and helpers for parsing jump locations from 
compiler errors and jumping to them in the corresponding buffer.
*/

// TOP

CUSTOM_COMMAND_SIG(goto_jump_at_cursor_direct)
CUSTOM_DOC("If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.")
{
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = get_active_view(app, AccessProtected);
    
    Parsed_Jump jump = parse_jump_from_buffer_line(app, &global_part, view.buffer_id, view.cursor.line, false);
    if (jump.success){
        change_active_panel(app);
        View_ID target_view = 0;
        get_active_view(app, AccessAll, &target_view);
        Buffer_ID buffer = {};
        if (get_jump_buffer(app, &buffer, &jump.location)){
            switch_to_existing_view(app, target_view, buffer);
            jump_to_location(app, target_view, buffer, jump.location);
        }
    }
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel_direct)
CUSTOM_DOC("If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list..")
{
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_Summary view = get_active_view(app, AccessProtected);
    
    Parsed_Jump jump = parse_jump_from_buffer_line(app, &global_part, view.buffer_id, view.cursor.line, false);
    if (jump.success){
        View_Summary target_view = view;
        Buffer_ID buffer = {};
        if (get_jump_buffer(app, &buffer, &jump.location)){
            jump_to_location(app, target_view.view_id, buffer, jump.location);
        }
    }
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(goto_next_jump_direct)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.")
{
    b32 skip_repeats = true;
    b32 skip_sub_errors = true;
    i32 dir = 1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_jump_direct)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations.")
{
    b32 skip_repeats = true;
    b32 skip_sub_errors = true;
    i32 dir = -1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_next_jump_no_skips_direct)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.")
{
    b32 skip_repeats = false;
    b32 skip_sub_errors = true;
    i32 dir = 1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips_direct)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.")
{
    b32 skip_repeats = false;
    b32 skip_sub_errors = true;
    i32 dir = -1;
    seek_jump(app, &global_part, skip_repeats, skip_sub_errors, dir);
}

CUSTOM_COMMAND_SIG(goto_first_jump_direct)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.")
{
    Temp_Memory temp = begin_temp_memory(&global_part);
    View_ID view = get_view_for_locked_jump_buffer(app);
    if (view != 0){
        view_set_cursor(app, view, seek_pos(0), true);
        memset(&prev_location, 0, sizeof(prev_location));
        seek_jump(app, &global_part, false, true, 1);
    }
    end_temp_memory(temp);
}

//
// Insert Newline or Tigger Jump on Read Only Buffer
//

CUSTOM_COMMAND_SIG(newline_or_goto_position_direct)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = 0;
    if (view_get_buffer(app, view.view_id, AccessOpen, &buffer)){
        write_character(app);
    }
    else if (view_get_buffer(app, view.view_id, AccessProtected, &buffer)){
        goto_jump_at_cursor_direct(app);
        lock_jump_buffer(app, buffer);
    }
}

CUSTOM_COMMAND_SIG(newline_or_goto_position_same_panel_direct)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = 0;
    if (view_get_buffer(app, view.view_id, AccessOpen, &buffer)){
        write_character(app);
    }
    else if (view_get_buffer(app, view.view_id, AccessProtected, &buffer)){
        goto_jump_at_cursor_same_panel_direct(app);
        lock_jump_buffer(app, buffer);
    }
}

// BOTTOM

