/*
4coder_seek_commands.cpp - Commands for jumping through code to useful stop boundaries.
*/

// TOP

enum{
    DirLeft = 0,
    DirRight = 1,
};

static int32_t
flip_dir(int32_t dir){
    if (dir == DirLeft){
        return(DirRight);
    }
    else{
        return(DirLeft);
    }
}

static int32_t
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer,
                     int32_t start_pos, int32_t dir, Seek_Boundary_Flag flags){
    bool32 forward = (dir == DirRight);
    return(buffer_boundary_seek(app, buffer, &global_part, start_pos, forward, flags));
}

static void
view_buffer_boundary_seek_set_pos(Application_Links *app, View_Summary *view, Buffer_Summary *buffer,
                                  int32_t dir, uint32_t flags){
    int32_t pos = buffer_boundary_seek(app, buffer, &global_part, view->cursor.pos, dir, flags);
    view_set_cursor(app, view, seek_pos(pos), true);
}

static void
view_boundary_seek_set_pos(Application_Links *app, View_Summary *view,
                           int32_t dir, uint32_t flags){
    Buffer_Summary buffer = get_buffer(app, view->buffer_id, AccessProtected);
    view_buffer_boundary_seek_set_pos(app, view, &buffer, dir, flags);
}

static void
current_view_boundary_seek_set_pos(Application_Links *app, int32_t dir, uint32_t flags){
    View_Summary view = get_active_view(app, AccessProtected);
    view_boundary_seek_set_pos(app, &view, dir, flags);
}

static Range
view_buffer_boundary_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer,
                           int32_t dir, uint32_t flags){
    int32_t pos1 = view->cursor.pos;
    int32_t pos2 = buffer_boundary_seek(app, buffer, pos1, dir, flags);
    return(make_range(pos1, pos2));
}

static Range
view_buffer_snipe_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer,
                        int32_t dir, uint32_t flags){
    int32_t pos0 = view->cursor.pos;
    int32_t pos1 = buffer_boundary_seek(app, buffer, pos0, dir          , flags);
    int32_t pos2 = buffer_boundary_seek(app, buffer, pos1, flip_dir(dir), flags);
    if (dir == DirLeft){
        if (pos2 < pos0){
            pos2 = pos0;
        }
    }
    else{
        if (pos2 > pos0){
            pos2 = pos0;
        }
    }
    return(make_range(pos1, pos2));
}

static void
current_view_boundary_delete(Application_Links *app, int32_t dir, uint32_t flags){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (buffer.exists){
        Range range = view_buffer_boundary_range(app, &view, &buffer, dir, flags);
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    }
}

static void
current_view_snipe_delete(Application_Links *app, int32_t dir, uint32_t flags){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (buffer.exists){
        Range range = view_buffer_snipe_range(app, &view, &buffer, dir, flags);
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(seek_whitespace_right)
CUSTOM_DOC("Seek right for the next boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_whitespace_left)
CUSTOM_DOC("Seek left for the next boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_token_right)
CUSTOM_DOC("Seek right for the next end of a token.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryToken);
}

CUSTOM_COMMAND_SIG(seek_token_left)
CUSTOM_DOC("Seek left for the next beginning of a token.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryToken);
}

CUSTOM_COMMAND_SIG(seek_white_or_token_right)
CUSTOM_DOC("Seek right for the next end of a token or boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryToken|BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_white_or_token_left)
CUSTOM_DOC("Seek left for the next end of a token or boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryToken|BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_right)
CUSTOM_DOC("Seek right for boundary between alphanumeric characters and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_left)
CUSTOM_DOC("Seek left for boundary between alphanumeric characters and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_right)
CUSTOM_DOC("Seek right for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryAlphanumeric|BoundaryCamelCase);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_left)
CUSTOM_DOC("Seek left for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryAlphanumeric|BoundaryCamelCase);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(backspace_word)
CUSTOM_DOC("Delete characters between the cursor position and the first alphanumeric boundary to the left.")
{
    current_view_boundary_delete(app, DirLeft, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(delete_word)
CUSTOM_DOC("Delete characters between the cursor position and the first alphanumeric boundary to the right.")
{
    current_view_boundary_delete(app, DirRight, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(snipe_token_or_word)
CUSTOM_DOC("Delete a single, whole token on or to the left of the cursor and post it to the clipboard.")
{
    current_view_snipe_delete(app, DirLeft, BoundaryToken|BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(snipe_token_or_word_right)
CUSTOM_DOC("Delete a single, whole token on or to the right of the cursor and post it to the clipboard.")
{
    current_view_snipe_delete(app, DirRight, BoundaryToken|BoundaryWhitespace);
}

// BOTTOM

