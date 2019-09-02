/*
4coder_seek.cpp - Procedures and commands for jumping through code to useful stop boundaries.
*/

// TOP

internal void
seek_pos_of_textual_line(Application_Links *app, Side side){
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    i64 new_pos = get_line_side_pos_from_pos(app, buffer, pos, side);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

internal void
seek_pos_of_visual_line(Application_Links *app, Side side){
    View_ID view = get_active_view(app, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    Vec2_f32 p = view_relative_xy_of_pos(app, view, cursor.line, pos);
    p.x = (side == Side_Min)?(0.f):(max_f32);
    i64 new_pos = view_pos_at_relative_xy(app, view, cursor.line, p);
    view_set_cursor(app, view, seek_pos(new_pos), true);
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
    View_ID view = get_active_view(app, AccessProtected);
    view_set_cursor(app, view, seek_pos(0), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(goto_end_of_file)
CUSTOM_DOC("Sets the cursor to the end of the file.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer_id = view_get_buffer(app, view, AccessProtected);
    i32 size = (i32)buffer_get_size(app, buffer_id);
    view_set_cursor(app, view, seek_pos(size), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

// BOTTOM

