/*
4coder_keyboard_macro.cpp - Keyboard macro recording and replaying commands.
*/

// TOP

function Buffer_ID
get_keyboard_log_buffer(Application_Links *app){
    return(get_buffer_by_name(app, string_u8_litexpr("*keyboard*"), Access_Always));
}

function void
keyboard_macro_play_single_line(Application_Links *app, String_Const_u8 macro_line){
    Scratch_Block scratch(app);
    Input_Event event = parse_keyboard_event(scratch, macro_line);
    if (event.kind != InputEventKind_None){
        enqueue_virtual_event(app, &event);
    }
}

function void
keyboard_macro_play(Application_Links *app, String_Const_u8 macro){
    Scratch_Block scratch(app);
    List_String_Const_u8 lines = string_split(scratch, macro, (u8*)"\n", 1);
    for (Node_String_Const_u8 *node = lines.first;
         node != 0;
         node = node->next){
        String_Const_u8 line = string_skip_chop_whitespace(node->string);
        keyboard_macro_play_single_line(app, line);
    }
}

function b32
get_current_input_is_virtual(Application_Links *app){
    User_Input input = get_current_input(app);
    return(input.event.virtual_event);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(keyboard_macro_start_recording)
CUSTOM_DOC("Start macro recording, do nothing if macro recording is already started")
{
    if (global_keyboard_macro_is_recording ||
        get_current_input_is_virtual(app)){
        return;
    }
    
    Buffer_ID buffer = get_keyboard_log_buffer(app);
    global_keyboard_macro_is_recording = true;
    global_keyboard_macro_range.first = buffer_get_size(app, buffer);
}

CUSTOM_COMMAND_SIG(keyboard_macro_finish_recording)
CUSTOM_DOC("Stop macro recording, do nothing if macro recording is not already started")
{
    if (!global_keyboard_macro_is_recording ||
        get_current_input_is_virtual(app)){
        return;
    }
    
    Buffer_ID buffer = get_keyboard_log_buffer(app);
    global_keyboard_macro_is_recording = false;
    i64 end = buffer_get_size(app, buffer);
    Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(end));
    Buffer_Cursor back_cursor = buffer_compute_cursor(app, buffer, seek_line_col(cursor.line - 1, 1));
    global_keyboard_macro_range.one_past_last = back_cursor.pos;
    
#if 0
    Scratch_Block scratch(app);
    String_Const_u8 macro = push_buffer_range(app, scratch, buffer, global_keyboard_macro_range);
    print_message(app, string_u8_litexpr("recorded:\n"));
    print_message(app, macro);
#endif
}

CUSTOM_COMMAND_SIG(keyboard_macro_replay)
CUSTOM_DOC("Replay the most recently recorded keyboard macro")
{
    if (global_keyboard_macro_is_recording ||
        get_current_input_is_virtual(app)){
        return;
    }
    
    Buffer_ID buffer = get_keyboard_log_buffer(app);
    Scratch_Block scratch(app);
    String_Const_u8 macro = push_buffer_range(app, scratch, buffer, global_keyboard_macro_range);
    keyboard_macro_play(app, macro);
}

// BOTTOM

