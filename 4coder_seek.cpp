/*
4coder_seek.cpp - Procedures and commands for jumping through code to useful stop boundaries.
*/

// TOP

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_ID buffer, i32 pos, char delim, i32 *result){
    Character_Predicate predicate = character_predicate_from_character((u8)delim);
    buffer_seek_character_class(app, buffer, &predicate, Scan_Forward, pos, result);
}

void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_ID buffer, i32 pos, char delim, i32 *result){
    Character_Predicate predicate = character_predicate_from_character((u8)delim);
    buffer_seek_character_class(app, buffer, &predicate, Scan_Backward, pos, result);
}

static void
buffer_seek_string_forward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 end, String_Const_u8 needle, i32 *result){
    if (end == 0){
        buffer_get_size(app, buffer, &end);
    }
    b32 case_sensitive = false;
    b32 finding_matches = false;
    do{
        finding_matches = 
            buffer_seek_string(app, buffer, needle, Scan_Forward, pos, &pos, &case_sensitive);
    } while(!case_sensitive && pos < end && finding_matches);
    if (pos < end && finding_matches){
        *result = pos;
    }
    else{
        buffer_get_size(app, buffer, result);
    }
}

static void
buffer_seek_string_backward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 min, String_Const_u8 needle, i32 *result){
    b32 case_sensitive = false;
    b32 finding_matches = false;
    do{
        finding_matches = 
            buffer_seek_string(app, buffer, needle, Scan_Backward, pos, &pos, &case_sensitive);
    } while(!case_sensitive && pos >= min && finding_matches);
    if (pos >= min && finding_matches){
        *result = pos;
    }
    else{
        *result = -1;
    }
}

static void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 end, String_Const_u8 needle, i32 *result){
    if (end == 0){
        buffer_get_size(app, buffer, &end);
    }
    b32 finding_matches = false;
    b32 case_sensitive = false;
    finding_matches = 
        buffer_seek_string(app, buffer, needle, Scan_Forward, pos, &pos, &case_sensitive);
    if (pos < end && finding_matches){
        *result = pos;
    }
    else{
        buffer_get_size(app, buffer, result);
    }
}

static void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 min, String_Const_u8 needle, i32 *result){
    b32 finding_matches = false;
    b32 case_sensitive = false;
    finding_matches = 
        buffer_seek_string(app, buffer, needle, Scan_Backward, pos, &pos, &case_sensitive);
    if (pos >= min && finding_matches){
        *result = pos;
    }
    else{
        *result = -1;
    }
}

static void
buffer_seek_string(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 end, i32 min, String_Const_u8 str, i32 *result, Buffer_Seek_String_Flags flags){
    switch (flags & 3){
        case 0:
        {
            buffer_seek_string_forward(app, buffer_id, pos, end, str, result);
        }break;
        
        case BufferSeekString_Backward:
        {
            buffer_seek_string_backward(app, buffer_id, pos, min, str, result);
        }break;
        
        case BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_forward(app, buffer_id, pos, end, str, result);
        }break;
        
        case BufferSeekString_Backward|BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_backward(app, buffer_id, pos, min, str, result);
        }break;
    }
}

////////////////////////////////

internal void
seek_pos_of_textual_line(Application_Links *app, Side side){
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = get_line_side_pos_from_pos(app, buffer_id, pos, side);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

internal void
seek_pos_of_visual_line(Application_Links *app, Side side){
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(pos), &cursor);
    f32 y = cursor.wrapped_y;
    f32 x = (side == Side_Min)?(0.f):(max_f32);
    view_set_cursor(app, view, seek_wrapped_xy(x, y, true), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the line across all text.")
{
    seek_pos_of_textual_line(app, Side_Min);
}

CUSTOM_COMMAND_SIG(seek_end_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the end of the line across all text.")
{
    seek_pos_of_textual_line(app, Side_Max);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the visual line.")
{
    seek_pos_of_visual_line(app, Side_Min);
}

CUSTOM_COMMAND_SIG(seek_end_of_line)
CUSTOM_DOC("Seeks the cursor to the end of the visual line.")
{
    seek_pos_of_visual_line(app, Side_Max);
}

CUSTOM_COMMAND_SIG(goto_beginning_of_file)
CUSTOM_DOC("Sets the cursor to the beginning of the file.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    view_set_cursor(app, view, seek_pos(0), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(goto_end_of_file)
CUSTOM_DOC("Sets the cursor to the end of the file.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 size = 0;
    buffer_get_size(app, buffer_id, &size);
    view_set_cursor(app, view, seek_pos(size), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

// BOTTOM

