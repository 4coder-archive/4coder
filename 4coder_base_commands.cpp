/*
4coder_base_commands.cpp - Base commands such as inserting characters, and 
moving the cursor, which work even without the default 4coder framework.
*/

// TOP

static void
write_character_parameter(Application_Links *app, uint8_t *character, uint32_t length){
    if (length != 0){
        uint32_t access = AccessOpen;
        View_Summary view = get_active_view(app, access);
        
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
        int32_t pos = view.cursor.pos;
        
        Marker next_cursor_marker = {0};
        next_cursor_marker.pos = character_pos_to_pos(app, &view, &buffer, view.cursor.character_pos);
        next_cursor_marker.lean_right = true;
        
        Marker_Handle handle = buffer_add_markers(app, &buffer, 1, 0, 0, 0);
        buffer_set_markers(app, &buffer, handle, 0, 1, &next_cursor_marker);
        
        buffer_replace_range(app, &buffer, pos, pos, (char*)character, length);
        
        buffer_get_markers(app, &buffer, handle, 0, 1, &next_cursor_marker);
        buffer_remove_markers(app, &buffer, handle);
        
        view_set_cursor(app, &view, seek_pos(next_cursor_marker.pos), true);
    }
}

CUSTOM_COMMAND_SIG(write_character)
CUSTOM_DOC("Inserts whatever character was used to trigger this command.")
{
    User_Input in = get_command_input(app);
    uint8_t character[4];
    uint32_t length = to_writable_character(in, character);
    write_character_parameter(app, character, length);
    }

CUSTOM_COMMAND_SIG(write_underscore)
CUSTOM_DOC("Inserts an underscore.")
{
    uint8_t character = '_';
    write_character_parameter(app, &character, 1);
}

CUSTOM_COMMAND_SIG(delete_char)
CUSTOM_DOC("Deletes the character to the right of the cursor.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t start = view.cursor.pos;
    
    Full_Cursor cursor;
    view_compute_cursor(app, &view, seek_character_pos(view.cursor.character_pos+1), &cursor);
    int32_t end = cursor.pos;
    
    if (0 <= start && start < buffer.size){
        buffer_replace_range(app, &buffer, start, end, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(backspace_char)
CUSTOM_DOC("Deletes the character to the left of the cursor.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t end = view.cursor.pos;
    
    Full_Cursor cursor;
    view_compute_cursor(app, &view, seek_character_pos(view.cursor.character_pos-1), &cursor);
    int32_t start = cursor.pos;
    
    if (0 < end && end <= buffer.size){
        buffer_replace_range(app, &buffer, start, end, 0, 0);
        view_set_cursor(app, &view, seek_character_pos(view.cursor.character_pos-1), true);
    }
}

CUSTOM_COMMAND_SIG(set_mark)
CUSTOM_DOC("Sets the mark to the current position of the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    view_set_mark(app, &view, seek_pos(view.cursor.pos));
    view_set_cursor(app, &view, seek_pos(view.cursor.pos), 1);
}

CUSTOM_COMMAND_SIG(cursor_mark_swap)
CUSTOM_DOC("Swaps the position of the cursor and the mark.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    int32_t cursor = view.cursor.pos;
    int32_t mark = view.mark.pos;
    
    view_set_cursor(app, &view, seek_pos(mark), true);
    view_set_mark(app, &view, seek_pos(cursor));
}

CUSTOM_COMMAND_SIG(delete_range)
CUSTOM_DOC("Deletes the text in the range between the cursor and the mark.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Range range = get_view_range(&view);
    buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(center_view)
CUSTOM_DOC("Centers the view vertically on the line on which the cursor sits.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    i32_Rect region = view.file_region;
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    float h = (float)(region.y1 - region.y0);
    float y = get_view_y(&view);
    y = y - h*.5f;
    scroll.target_y = (int32_t)(y + .5f);
    view_set_scroll(app, &view, scroll);
}

CUSTOM_COMMAND_SIG(left_adjust_view)
CUSTOM_DOC("Sets the left size of the view near the x position of the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    float x = get_view_x(&view) - 30.f;
    if (x < 0){
        x = 0.f;
    }
    
    scroll.target_x = (int32_t)(x + .5f);
    view_set_scroll(app, &view, scroll);
}

bool32
global_point_to_view_point(View_Summary *view, int32_t x, int32_t y, float *x_out, float *y_out){
    bool32 result = false;
    
    i32_Rect region = view->file_region;
    
    int32_t max_x = (region.x1 - region.x0);
    int32_t max_y = (region.y1 - region.y0);
    GUI_Scroll_Vars scroll_vars = view->scroll_vars;
    
    int32_t rx = x - region.x0;
    int32_t ry = y - region.y0;
    
    if (ry >= 0 && rx >= 0 && rx < max_x && ry < max_y){
        result = 1;
    }
    
    *x_out = (float)rx + scroll_vars.scroll_x;
    *y_out = (float)ry + scroll_vars.scroll_y;
    
    return(result);
}

CUSTOM_COMMAND_SIG(click_set_cursor)
CUSTOM_DOC("Sets the cursor position to the mouse position.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    
    Mouse_State mouse = get_mouse_state(app);
    float rx = 0, ry = 0;
    if (global_point_to_view_point(&view, mouse.x, mouse.y, &rx, &ry)){
        view_set_cursor(app, &view, seek_xy(rx, ry, 1, view.unwrapped_lines), 1);
    }
}

CUSTOM_COMMAND_SIG(click_set_mark)
CUSTOM_DOC("Sets the mark position to the mouse position.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    
    Mouse_State mouse = get_mouse_state(app);
    float rx = 0, ry = 0;
    if (global_point_to_view_point(&view, mouse.x, mouse.y, &rx, &ry)){
        view_set_mark(app, &view, seek_xy(rx, ry, 1, view.unwrapped_lines));
    }
}

////////////////////////////////

inline void
move_vertical(Application_Links *app, float line_multiplier){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    
    float delta_y = line_multiplier*view.line_height;
    float new_y = get_view_y(&view) + delta_y;
    float x = view.preferred_x;
    
    view_set_cursor(app, &view, seek_xy(x, new_y, 0, view.unwrapped_lines), 0);
    float actual_new_y = get_view_y(&view);
    if (actual_new_y < new_y){
        i32_Rect file_region = view.file_region;
        int32_t height = file_region.y1 - file_region.y0;
        int32_t full_scroll_y = (int32_t)actual_new_y - height/2;
        if (view.scroll_vars.target_y < full_scroll_y){
            GUI_Scroll_Vars new_scroll_vars = view.scroll_vars;
            new_scroll_vars.target_y += (int32_t)delta_y;
            if (new_scroll_vars.target_y > full_scroll_y){
                new_scroll_vars.target_y = full_scroll_y;
            }
            view_set_scroll(app, &view, new_scroll_vars);
        }
    }
}

static float
get_page_jump(View_Summary *view){
    i32_Rect region = view->file_region;
    float page_jump = 1.f;
    if (view->line_height > 0.f){
        int32_t height = region.y1 - region.y0;
        float line_count = (float)(height)/view->line_height;
        int32_t line_count_rounded = (int32_t)line_count;
        page_jump = (float)line_count_rounded - 3.f;
        if (page_jump <= 1.f){
            page_jump = 1.f;
        }
    }
    return(page_jump);
}

CUSTOM_COMMAND_SIG(move_up)
CUSTOM_DOC("Moves the cursor up one line.")
{
    move_vertical(app, -1.f);
}

CUSTOM_COMMAND_SIG(move_down)
CUSTOM_DOC("Moves the cursor down one line.")
{
    move_vertical(app, 1.f);
}

CUSTOM_COMMAND_SIG(move_up_10)
CUSTOM_DOC("Moves the cursor up ten lines.")
{
    move_vertical(app, -10.f);
}

CUSTOM_COMMAND_SIG(move_down_10)
CUSTOM_DOC("Moves the cursor down ten lines.")
{
    move_vertical(app, 10.f);
}

CUSTOM_COMMAND_SIG(move_down_textual)
CUSTOM_DOC("Moves down to the next line of actual text, regardless of line wrapping.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    if (!view.exists){
        return;
    }
    int32_t next_line = view.cursor.line + 1;
    view_set_cursor(app, &view, seek_line_char(next_line, 1), true);
}

CUSTOM_COMMAND_SIG(page_up)
CUSTOM_DOC("Scrolls the view up one view height and moves the cursor up one view height.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    float page_jump = get_page_jump(&view);
    move_vertical(app, -page_jump);
}

CUSTOM_COMMAND_SIG(page_down)
CUSTOM_DOC("Scrolls the view down one view height and moves the cursor down one view height.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    float page_jump = get_page_jump(&view);
    move_vertical(app, page_jump);
}

////////////////

CUSTOM_COMMAND_SIG(move_left)
CUSTOM_DOC("Moves the cursor one character to the left.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    int32_t new_pos = view.cursor.character_pos - 1;
    view_set_cursor(app, &view, seek_character_pos(new_pos), 1);
}

CUSTOM_COMMAND_SIG(move_right)
CUSTOM_DOC("Moves the cursor one character to the right.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    int32_t new_pos = view.cursor.character_pos + 1;
    view_set_cursor(app, &view, seek_character_pos(new_pos), 1);
}

CUSTOM_COMMAND_SIG(select_all)
CUSTOM_DOC("Puts the cursor at the top of the file, and the mark at the bottom of the file.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    view_set_cursor(app, &view, seek_pos(0), true);
    view_set_mark(app, &view, seek_pos(buffer.size));
}

////////////////////////////////

CUSTOM_COMMAND_SIG(to_uppercase)
CUSTOM_DOC("Converts all ascii text in the range between the cursor and the mark to uppercase.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_view_range(&view);
    int32_t size = range.max - range.min;
    if (size <= app->memory_size){
        char *mem = (char*)app->memory;
        
        buffer_read_range(app, &buffer, range.min, range.max, mem);
        for (int32_t i = 0; i < size; ++i){
            mem[i] = char_to_upper(mem[i]);
        }
        buffer_replace_range(app, &buffer, range.min, range.max, mem, size);
        view_set_cursor(app, &view, seek_pos(range.max), true);
    }
}

CUSTOM_COMMAND_SIG(to_lowercase)
CUSTOM_DOC("Converts all ascii text in the range between the cursor and the mark to lowercase.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_view_range(&view);
    int32_t size = range.max - range.min;
    if (size <= app->memory_size){
        char *mem = (char*)app->memory;
        
        buffer_read_range(app, &buffer, range.min, range.max, mem);
        for (int32_t i = 0; i < size; ++i){
            mem[i] = char_to_lower(mem[i]);
        }
        buffer_replace_range(app, &buffer, range.min, range.max, mem, size);
        view_set_cursor(app, &view, seek_pos(range.max), true);
    }
}

CUSTOM_COMMAND_SIG(clean_all_lines)
CUSTOM_DOC("Removes trailing whitespace from all lines in the current buffer.")
{
    // TODO(allen): This command always iterates accross the entire
    // buffer, so streaming it is actually the wrong call.  Rewrite this
    // to minimize calls to buffer_read_range.
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    int32_t line_count = buffer.line_count;
    int32_t edit_max = line_count;
    
    if (edit_max*(int32_t)sizeof(Buffer_Edit) < app->memory_size){
        Buffer_Edit *edits = (Buffer_Edit*)app->memory;
        
        char data[1024];
        Stream_Chunk chunk = {0};
        
        int32_t i = 0;
        if (init_stream_chunk(&chunk, app, &buffer, i, data, sizeof(data))){
            Buffer_Edit *edit = edits;
            
            int32_t buffer_size = buffer.size;
            int32_t still_looping = true;
            int32_t last_hard = buffer_size;
            do{
                for (; i < chunk.end; ++i){
                    char at_pos = chunk.data[i];
                    if (at_pos == '\n'){
                        if (last_hard + 1 < i){
                            edit->str_start = 0;
                            edit->len = 0;
                            edit->start = last_hard + 1;
                            edit->end = i;
                            ++edit;
                        }
                        last_hard = buffer_size;
                    }
                    else if (char_is_whitespace(at_pos)){
                        // NOTE(allen): do nothing
                    }
                    else{
                        last_hard = i;
                    }
                }
                
                still_looping = forward_stream_chunk(&chunk);
            }while(still_looping);
            
            if (last_hard + 1 < buffer_size){
                edit->str_start = 0;
                edit->len = 0;
                edit->start = last_hard + 1;
                edit->end = buffer_size;
                ++edit;
            }
            
            int32_t edit_count = (int32_t)(edit - edits);
            buffer_batch_edit(app, &buffer, 0, 0, edits, edit_count, BatchEdit_PreserveTokens);
        }
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(basic_change_active_panel)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next highest view_id.  Will not skipe the build panel if it is open.")
{
    View_Summary view = get_active_view(app, AccessAll);
    get_view_next_looped(app, &view, AccessAll);
    set_active_view(app, &view);
}

CUSTOM_COMMAND_SIG(close_panel)
CUSTOM_DOC("Closes the currently active panel if it is not the only panel open.")
{
    View_Summary view = get_active_view(app, AccessAll);
    close_view(app, &view);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(show_scrollbar)
CUSTOM_DOC("Sets the current view to show it's scrollbar.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_set_setting(app, &view, ViewSetting_ShowScrollbar, true);
}

CUSTOM_COMMAND_SIG(hide_scrollbar)
CUSTOM_DOC("Sets the current view to hide it's scrollbar.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_set_setting(app, &view, ViewSetting_ShowScrollbar, false);
}

CUSTOM_COMMAND_SIG(show_filebar)
CUSTOM_DOC("Sets the current view to show it's filebar.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_set_setting(app, &view, ViewSetting_ShowFileBar, true);
}

CUSTOM_COMMAND_SIG(hide_filebar)
CUSTOM_DOC("Sets the current view to hide it's filebar.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_set_setting(app, &view, ViewSetting_ShowFileBar, false);
}

CUSTOM_COMMAND_SIG(toggle_filebar)
CUSTOM_DOC("Toggles the visibility status of the current view's filebar.")
{
    View_Summary view = get_active_view(app, AccessAll);
    bool32 value;
    view_get_setting(app, &view, ViewSetting_ShowFileBar, &value);
    view_set_setting(app, &view, ViewSetting_ShowFileBar, !value);
}

CUSTOM_COMMAND_SIG(toggle_line_wrap)
CUSTOM_DOC("Toggles the current buffer's line wrapping status.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    bool32 unwrapped = view.unwrapped_lines;
    buffer_set_setting(app, &buffer, BufferSetting_WrapLine, unwrapped);
}

CUSTOM_COMMAND_SIG(increase_line_wrap)
CUSTOM_DOC("Increases the current buffer's width for line wrapping.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t wrap = 0;
    buffer_get_setting(app, &buffer, BufferSetting_WrapPosition, &wrap);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap + 10);
}

CUSTOM_COMMAND_SIG(decrease_line_wrap)
CUSTOM_DOC("Decrases the current buffer's width for line wrapping.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t wrap = 0;
    buffer_get_setting(app, &buffer, BufferSetting_WrapPosition, &wrap);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap - 10);
}

CUSTOM_COMMAND_SIG(increase_face_size)
CUSTOM_DOC("Increase the size of the face used by the current buffer.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    Face_ID face_id = get_face_id(app, &buffer);
    Face_Description description = get_face_description(app, face_id);
    ++description.pt_size;
    try_modify_face(app, face_id, &description);
}

CUSTOM_COMMAND_SIG(decrease_face_size)
CUSTOM_DOC("Decrease the size of the face used by the current buffer.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    Face_ID face_id = get_face_id(app, &buffer);
    Face_Description description = get_face_description(app, face_id);
    --description.pt_size;
    try_modify_face(app, face_id, &description);
}

CUSTOM_COMMAND_SIG(toggle_virtual_whitespace)
CUSTOM_DOC("Toggles the current buffer's virtual whitespace status.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t vwhite = 0;
    buffer_get_setting(app, &buffer, BufferSetting_VirtualWhitespace, &vwhite);
    buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, !vwhite);
}

CUSTOM_COMMAND_SIG(toggle_show_whitespace)
CUSTOM_DOC("Toggles the current buffer's whitespace visibility status.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    view_set_setting(app, &view, ViewSetting_ShowWhitespace, !view.show_whitespace);
}

CUSTOM_COMMAND_SIG(eol_dosify)
CUSTOM_DOC("Puts the buffer in DOS line ending mode.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    buffer_set_setting(app, &buffer, BufferSetting_Eol, 1);
}

CUSTOM_COMMAND_SIG(eol_nixify)
CUSTOM_DOC("Puts the buffer in NIX line ending mode.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    buffer_set_setting(app, &buffer, BufferSetting_Eol, 0);
}

CUSTOM_COMMAND_SIG(exit_4coder)
CUSTOM_DOC("Attempts to close 4coder.")
{
    send_exit_signal(app);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(goto_line)
CUSTOM_DOC("Queries the user for a number, and jumps the cursor to the corresponding line.")
{
    uint32_t access = AccessProtected;
    
    Query_Bar bar = {0};
    char string_space[256];
    
    bar.prompt = make_lit_string("Goto Line: ");
    bar.string = make_fixed_width_string(string_space);
    
    if (query_user_number(app, &bar)){
        int32_t line_number = str_to_int_s(bar.string);
        
        View_Summary view = get_active_view(app, access);
        view_set_cursor(app, &view, seek_line_char(line_number, 0), true);
    }
}

CUSTOM_COMMAND_SIG(search);
CUSTOM_COMMAND_SIG(reverse_search);

static void
isearch(Application_Links *app, int32_t start_reversed, String query_init){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    if (!buffer.exists) return;
    
    Query_Bar bar = {0};
    if (start_query_bar(app, &bar, 0) == 0) return;
    
    bool32 reverse = start_reversed;
    int32_t first_pos = view.cursor.pos;
    
    int32_t pos = first_pos;
    if (query_init.size != 0){
        pos += 2;
    }
    
    int32_t start_pos = pos;
    Range match = make_range(pos, pos);
    
    char bar_string_space[256];
    bar.string = make_fixed_width_string(bar_string_space);
    copy_ss(&bar.string, query_init);
    
    String isearch_str = make_lit_string("I-Search: ");
    String rsearch_str = make_lit_string("Reverse-I-Search: ");
    
    bool32 first_step = true;
    
    User_Input in = {0};
    for (;;){
        view_set_highlight(app, &view, match.start, match.end, true);
        
        // NOTE(allen): Change the bar's prompt to match the current direction.
        if (reverse){
            bar.prompt = rsearch_str;
        }
        else{
            bar.prompt = isearch_str;
        }
        
        bool32 step_forward = false;
        bool32 step_backward = false;
        bool32 backspace = false;
        
        if (!first_step){
            //in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
            in = get_user_input(app, EventOnAnyKey, EventOnEsc);
            if (in.abort) break;
            
            // NOTE(allen): If we're getting mouse events here it's a 4coder bug, because we only asked to intercept key events.
            Assert(in.type == UserInputKey);
            
            uint8_t character[4];
            uint32_t length = to_writable_character(in, character);
            
            bool32 made_change = false;
            if (in.key.keycode == '\n' || in.key.keycode == '\t'){
                break;
            }
            else if (length != 0 && key_is_unmodified(&in.key)){
                append_ss(&bar.string, make_string(character, length));
                made_change = true;
            }
            else if (in.key.keycode == key_back){
                made_change = backspace_utf8(&bar.string);
                backspace = true;
            }
            
            if ((in.command.command == search) || in.key.keycode == key_page_down || in.key.keycode == key_down){
                step_forward = true;
            }
            
            if ((in.command.command == reverse_search) || in.key.keycode == key_page_up || in.key.keycode == key_up){
                step_backward = true;
            }
        }
        else{
            if (bar.string.size != 0){
                step_backward = true;
            }
            first_step = false;
        }
        
        start_pos = pos;
        if (step_forward && reverse){
            start_pos = match.start + 1;
            pos = start_pos;
            reverse = false;
            step_forward = false;
        }
        if (step_backward && !reverse){
            start_pos = match.start - 1;
            pos = start_pos;
            reverse = true;
            step_backward = false;
        }
        
        if (!backspace){
            if (reverse){
                int32_t new_pos = 0;
                buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1, 0, bar.string.str, bar.string.size, &new_pos);
                if (new_pos >= 0){
                    if (step_backward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1, 0, bar.string.str, bar.string.size, &new_pos);
                        if (new_pos < 0){
                            new_pos = start_pos;
                        }
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
            else{
                int32_t new_pos = 0;
                buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1, 0, bar.string.str, bar.string.size, &new_pos);
                if (new_pos < buffer.size){
                    if (step_forward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1, 0, bar.string.str, bar.string.size, &new_pos);
                        if (new_pos >= buffer.size){
                            new_pos = start_pos;
                        }
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
        }
        else{
            if (match.end > match.start + bar.string.size){
                match.end = match.start + bar.string.size;
            }
        }
    }
    
    view_set_highlight(app, &view, 0, 0, false);
    if (in.abort){
        view_set_cursor(app, &view, seek_pos(first_pos), true);
        return;
    }
    
    view_set_cursor(app, &view, seek_pos(match.min), true);
}

CUSTOM_COMMAND_SIG(search)
CUSTOM_DOC("Begins an incremental search down through the current buffer for a user specified string.")
{
    String query = {0};
    isearch(app, false, query);
}

CUSTOM_COMMAND_SIG(reverse_search)
CUSTOM_DOC("Begins an incremental search up through the current buffer for a user specified string.")
{
    String query = {0};
    isearch(app, true, query);
}

CUSTOM_COMMAND_SIG(search_identifier)
CUSTOM_DOC("Begins an incremental search down through the current buffer for the word or token under the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    char space[256];
    String query = read_identifier_at_pos(app, &buffer, view.cursor.pos, space, sizeof(space), 0);
    isearch(app, false, query);
}

CUSTOM_COMMAND_SIG(reverse_search_identifier)
CUSTOM_DOC("Begins an incremental search up through the current buffer for the word or token under the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    char space[256];
    String query = read_identifier_at_pos(app, &buffer, view.cursor.pos, space, sizeof(space), 0);
    isearch(app, true, query);
}

CUSTOM_COMMAND_SIG(replace_in_range)
CUSTOM_DOC("Queries the user for two strings, and replaces all occurences of the first string in the range between the cursor and the mark with the second string.")
{
    Query_Bar replace;
    char replace_space[1024];
    replace.prompt = make_lit_string("Replace: ");
    replace.string = make_fixed_width_string(replace_space);
    
    Query_Bar with;
    char with_space[1024];
    with.prompt = make_lit_string("With: ");
    with.string = make_fixed_width_string(with_space);
    
    if (!query_user_string(app, &replace)) return;
    if (replace.string.size == 0) return;
    
    if (!query_user_string(app, &with)) return;
    
    String r = replace.string;
    String w = with.string;
    
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Range range = get_view_range(&view);
    
    int32_t pos = range.min;
    int32_t new_pos;
    buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    
    while (new_pos + r.size <= range.end){
        buffer_replace_range(app, &buffer, new_pos, new_pos + r.size, w.str, w.size);
        refresh_view(app, &view);
        range = get_view_range(&view);
        pos = new_pos + w.size;
        buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    }
}

static void
query_replace_base(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, int32_t pos, String r, String w){
    int32_t new_pos = 0;
    buffer_seek_string_forward(app, buffer, pos, 0, r.str, r.size, &new_pos);
    
    User_Input in = {0};
    for (;new_pos < buffer->size;){
        Range match = make_range(new_pos, new_pos + r.size);
        view_set_highlight(app, view, match.min, match.max, 1);
        
        in = get_user_input(app, EventOnAnyKey, EventOnButton);
        if (in.abort || in.key.keycode == key_esc || !key_is_unmodified(&in.key))  break;
        
        if (in.key.character == 'y' || in.key.character == 'Y' || in.key.character == '\n' || in.key.character == '\t'){
            buffer_replace_range(app, buffer, match.min, match.max, w.str, w.size);
            pos = match.start + w.size;
        }
        else{
            pos = match.max;
        }
        
        buffer_seek_string_forward(app, buffer, pos, 0, r.str, r.size, &new_pos);
    }
    
    view_set_highlight(app, view, 0, 0, 0);
    if (in.abort) return;
    
    view_set_cursor(app, view, seek_pos(pos), true);
}

static void
query_replace_parameter(Application_Links *app, String replace_str, int32_t start_pos, bool32 add_replace_query_bar){
    Query_Bar replace;
    replace.prompt = make_lit_string("Replace: ");
    replace.string = replace_str;
    
    if (add_replace_query_bar){
        start_query_bar(app, &replace, 0);
    }
    
    Query_Bar with;
    char with_space[1024];
    with.prompt = make_lit_string("With: ");
    with.string = make_fixed_width_string(with_space);
    
    if (!query_user_string(app, &with)) return;
    
    String r = replace.string;
    String w = with.string;
    
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    int32_t pos = start_pos;
    
    Query_Bar bar;
    bar.prompt = make_lit_string("Replace? (y)es, (n)ext, (esc)\n");
    bar.string = null_string;
    start_query_bar(app, &bar, 0);
    
    query_replace_base(app, &view, &buffer, pos, r, w);
}

CUSTOM_COMMAND_SIG(query_replace)
CUSTOM_DOC("Queries the user for two strings, and incrementally replaces every occurence of the first string with the second string.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    Query_Bar replace;
    char replace_space[1024];
    replace.prompt = make_lit_string("Replace: ");
    replace.string = make_fixed_width_string(replace_space);
    
    if (!query_user_string(app, &replace)) return;
    if (replace.string.size == 0) return;
    
    query_replace_parameter(app, replace.string, view.cursor.pos, false);
}

CUSTOM_COMMAND_SIG(query_replace_identifier)
CUSTOM_DOC("Queries the user for a string, and incrementally replace every occurence of the word or token found at the cursor with the specified string.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    Range range = {0};
    char space[256];
    String replace = read_identifier_at_pos(app, &buffer, view.cursor.pos, space, sizeof(space), &range);
    
    if (replace.size != 0){
        query_replace_parameter(app, replace, range.min, true);
    }
}

CUSTOM_COMMAND_SIG(query_replace_selection)
CUSTOM_DOC("Queries the user for a string, and incrementally replace every occurence of the string found in the selected range with the specified string.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    Partition *part = &global_part;
    Temp_Memory temp = begin_temp_memory(part);
    
    Range range = get_view_range(&view);
    int32_t replace_length = range.max - range.min;
    if (replace_length != 0){
        char *replace_space = push_array(part, char, replace_length);
        if (buffer_read_range(app, &buffer, range.min, range.max, replace_space)){
            String replace = make_string(replace_space, replace_length);
            query_replace_parameter(app, replace, range.min, true);
        }
    }
    
    end_temp_memory(temp);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(save_all_dirty_buffers)
CUSTOM_DOC("Saves all buffers marked dirty (showing the '*' indicator).")
{
    for (Buffer_Summary buffer = get_buffer_first(app, AccessOpen);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessOpen)){
        if (buffer.dirty == DirtyState_UnsavedChanges){
            save_buffer(app, &buffer, buffer.file_name, buffer.file_name_len, 0);
        }
    }
}

static void
delete_file_base(Application_Links *app, String file_name, Buffer_ID buffer_id){
    String path = path_of_directory(file_name);
    
    char space[4096];
    String cmd = make_fixed_width_string(space);
    
#if defined(IS_WINDOWS)
    append(&cmd, "del ");
#elif defined(IS_LINUX) || defined(IS_MAC)
    append(&cmd, "rm ");
#else
# error no delete file command for this platform
#endif
    append(&cmd, '"');
    append(&cmd, front_of_directory(file_name));
    append(&cmd, '"');
    
    exec_system_command(app, 0, buffer_identifier(0), path.str, path.size, cmd.str, cmd.size, 0);
    
    kill_buffer(app, buffer_identifier(buffer_id), 0, BufferKill_AlwaysKill);
}

CUSTOM_COMMAND_SIG(delete_file_query)
CUSTOM_DOC("Deletes the file of the current buffer if 4coder has the appropriate access rights. Will ask the user for confirmation first.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    if (buffer.file_name != 0){
        String file_name = {0};
        file_name = make_string(buffer.file_name, buffer.file_name_len);
        
        char space[4096];
        Query_Bar bar;
        bar.prompt = make_fixed_width_string(space);
        append(&bar.prompt, "Delete '");
        append(&bar.prompt, file_name);
        append(&bar.prompt, "' (Y)es, (n)o");
        bar.string = null_string;
        if (start_query_bar(app, &bar, 0) == 0) return;
        
        User_Input in = get_user_input(app, EventOnAnyKey, 0);
        if (in.key.keycode != 'Y') return;
        
        delete_file_base(app, file_name, buffer.buffer_id);
    }
}

CUSTOM_COMMAND_SIG(save_to_query)
CUSTOM_DOC("Queries the user for a name and saves the contents of the current buffer, altering the buffer's name too.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    // Query the user
    Query_Bar bar;
    
    char prompt_space[4096];
    bar.prompt = make_fixed_width_string(prompt_space);
    append(&bar.prompt, "Save '");
    append(&bar.prompt, make_string(buffer.buffer_name, buffer.buffer_name_len));
    append(&bar.prompt, "' to: ");
    
    char name_space[4096];
    bar.string = make_fixed_width_string(name_space);
    if (!query_user_string(app, &bar)) return;
    if (bar.string.size == 0) return;
    
    char new_file_name_space[4096];
    String new_file_name = make_fixed_width_string(new_file_name_space);
    int32_t hot_dir_size = directory_get_hot(app, 0, 0);
    if (new_file_name.size + hot_dir_size <= new_file_name.memory_size){
        new_file_name.size += directory_get_hot(app, new_file_name.str + new_file_name.size, new_file_name.memory_size - new_file_name.size);
        //append(&new_file_name, "/");
        if (append(&new_file_name, bar.string)){
            if (save_buffer(app, &buffer, new_file_name.str, new_file_name.size, BufferSave_IgnoreDirtyFlag)){
                Buffer_Summary new_buffer = create_buffer(app, new_file_name.str, new_file_name.size, BufferCreate_NeverNew|BufferCreate_JustChangedFile);
                if (new_buffer.exists){
                    if (new_buffer.buffer_id != buffer.buffer_id){
                        kill_buffer(app, buffer_identifier(buffer.buffer_id), 0, BufferKill_AlwaysKill);
                        view_set_buffer(app, &view, new_buffer.buffer_id, 0);
                    }
                }
            }
        }
    }
}

CUSTOM_COMMAND_SIG(rename_file_query)
CUSTOM_DOC("Queries the user for a new name and renames the file of the current buffer, altering the buffer's name too.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    if (buffer.file_name != 0){
        char file_name_space[4096];
        String file_name = make_fixed_width_string(file_name_space);
        if (copy_checked(&file_name, make_string(buffer.file_name, buffer.file_name_len))){
            // Query the user
            Query_Bar bar;
            
            char prompt_space[4096];
            bar.prompt = make_fixed_width_string(prompt_space);
            append(&bar.prompt, "Rename '");
            append(&bar.prompt, front_of_directory(file_name));
            append(&bar.prompt, "' to: ");
            
            char name_space[4096];
            bar.string = make_fixed_width_string(name_space);
            if (!query_user_string(app, &bar)) return;
            if (bar.string.size == 0) return;
            
            // TODO(allen): There should be a way to say, "detach a buffer's file" and "attach this file to a buffer"
            
            char new_file_name_space[4096];
            String new_file_name = make_fixed_width_string(new_file_name_space);
            copy(&new_file_name, file_name);
            remove_last_folder(&new_file_name);
            append(&new_file_name, bar.string);
            terminate_with_null(&new_file_name);
            
            if (save_buffer(app, &buffer, new_file_name.str, new_file_name.size, BufferSave_IgnoreDirtyFlag)){
                delete_file_base(app, file_name, buffer.buffer_id);
                Buffer_Summary new_buffer = create_buffer(app, new_file_name.str, new_file_name.size, BufferCreate_NeverNew|BufferCreate_JustChangedFile);
                view_set_buffer(app, &view, new_buffer.buffer_id, 0);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(make_directory_query)
CUSTOM_DOC("Queries the user for a name and creates a new directory with the given name.")
{
    char hot_space[2048];
    int32_t hot_length = directory_get_hot(app, hot_space, sizeof(hot_space));
    if (hot_length < sizeof(hot_space)){
        String hot = make_string(hot_space, hot_length);
        
        // Query the user
        Query_Bar bar;
        
        char prompt_space[4096];
        bar.prompt = make_fixed_width_string(prompt_space);
        append(&bar.prompt, "Make directory at '");
        append(&bar.prompt, hot);
        append(&bar.prompt, "': ");
        
        char name_space[4096];
        bar.string = make_fixed_width_string(name_space);
        if (!query_user_string(app, &bar)) return;
        if (bar.string.size == 0) return;
        
        char cmd_space[4096];
        String cmd = make_fixed_width_string(cmd_space);
        append(&cmd, "mkdir ");
        if (append_checked(&cmd, bar.string)){
            exec_system_command(app, 0, buffer_identifier(0), hot.str, hot.size, cmd.str, cmd.size, 0);
        }
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(move_line_up)
CUSTOM_DOC("Swaps the line under the cursor with the line above it, and moves the cursor up with it.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    
    if (view.cursor.line <= 1){
        return;
    }
    
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    Full_Cursor prev_line_cursor = {0};
    Full_Cursor this_line_cursor = {0};
    Full_Cursor next_line_cursor = {0};
    
    int32_t this_line = view.cursor.line;
    int32_t prev_line = this_line - 1;
    int32_t next_line = this_line +1;
    
    if (view_compute_cursor(app, &view, seek_line_char(prev_line, 1), &prev_line_cursor) &&
        view_compute_cursor(app, &view, seek_line_char(this_line, 1), &this_line_cursor) &&
        view_compute_cursor(app, &view, seek_line_char(next_line, 1), &next_line_cursor)){
        
        int32_t prev_line_pos = prev_line_cursor.pos;
        int32_t this_line_pos = this_line_cursor.pos;
        int32_t next_line_pos = next_line_cursor.pos;
        
        Partition *part = &global_part;
        Temp_Memory temp = begin_temp_memory(part);
        
        int32_t length = next_line_pos - prev_line_pos;
        char *swap = push_array(part, char, length + 1);
        int32_t first_len = next_line_pos - this_line_pos;
        
        if (buffer_read_range(app, &buffer, this_line_pos, next_line_pos, swap)){
            bool32 second_line_didnt_have_newline = true;
            for (int32_t i = first_len - 1; i >= 0; --i){
                if (swap[i] == '\n'){
                    second_line_didnt_have_newline = false;
                    break;
                }
            }
            
            if (second_line_didnt_have_newline){
                swap[first_len] = '\n';
                first_len += 1;
                // NOTE(allen): Don't increase "length" because then we will be including
                // the original newline and addignt this new one, making the file longer
                // which shouldn't be possible for this command!
            }
            
            if (buffer_read_range(app, &buffer, prev_line_pos, this_line_pos, swap + first_len)){
                buffer_replace_range(app, &buffer, prev_line_pos, next_line_pos, swap, length);
                view_set_cursor(app, &view, seek_line_char(prev_line, 1), true);
            }
        }
        
        end_temp_memory(temp);
    }
}

CUSTOM_COMMAND_SIG(move_line_down)
CUSTOM_DOC("Swaps the line under the cursor with the line below it, and moves the cursor down with it.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    if (!view.exists){
        return;
    }
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    int32_t next_line = view.cursor.line + 1;
    Full_Cursor new_cursor = {0};
    if (view_compute_cursor(app, &view, seek_line_char(next_line, 1), &new_cursor)){
        if (new_cursor.line == next_line){
            view_set_cursor(app, &view, seek_pos(new_cursor.pos), true);
            move_line_up(app);
            move_down_textual(app);
        }
    }
}

CUSTOM_COMMAND_SIG(duplicate_line)
CUSTOM_DOC("Create a copy of the line on which the cursor sits.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    String line_string = {0};
    char *before_line = push_array(part, char, 1);
    if (read_line(app, part, &buffer, view.cursor.line, &line_string)){
        *before_line = '\n';
        line_string.str = before_line;
        line_string.size += 1;
        
        int32_t pos = buffer_get_line_end(app, &buffer, view.cursor.line);
        buffer_replace_range(app, &buffer, pos, pos, line_string.str, line_string.size);
    }
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(delete_line)
CUSTOM_DOC("Delete the line the on which the cursor sits.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    int32_t start = buffer_get_line_start(app, &buffer, view.cursor.line);
    int32_t end = buffer_get_line_end(app, &buffer, view.cursor.line) + 1;
    if (end > buffer.size){
        end = buffer.size;
    }
    if (start == end || buffer_get_char(app, &buffer, end - 1) != '\n'){
        start -= 1;
        if (start < 0){
            start = 0;
        }
    }
    
    buffer_replace_range(app, &buffer, start, end, 0, 0);
    
    end_temp_memory(temp);
}

////////////////////////////////

static bool32
get_cpp_matching_file(Application_Links *app, Buffer_Summary buffer, Buffer_Summary *buffer_out){
    bool32 result = false;
    
    if (buffer.file_name != 0){
        char space[512];
        String file_name = make_string_cap(space, 0, sizeof(space));
        append(&file_name, make_string(buffer.file_name, buffer.file_name_len));
        
        String extension = file_extension(file_name);
        String new_extensions[2] = {0};
        int32_t new_extensions_count = 0;
        
        if (match(extension, "cpp") || match(extension, "cc")){
            new_extensions[0] = make_lit_string("h");
            new_extensions[1] = make_lit_string("hpp");
            new_extensions_count = 2;
        }
        else if (match(extension, "c")){
            new_extensions[0] = make_lit_string("h");
            new_extensions_count = 1;
        }
        else if (match(extension, "h")){
            new_extensions[0] = make_lit_string("c");
            new_extensions[1] = make_lit_string("cpp");
            new_extensions_count = 2;
        }
        else if (match(extension, "hpp")){
            new_extensions[0] = make_lit_string("cpp");
            new_extensions_count = 1;
        }
        
        remove_extension(&file_name);
        int32_t base_pos = file_name.size;
        for (int32_t i = 0; i < new_extensions_count; ++i){
            String ext = new_extensions[i];
            file_name.size = base_pos;
            append(&file_name, ext);
            
            if (open_file(app, buffer_out, file_name.str, file_name.size, false, true)){
                result = true;
                break;
            }
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(open_file_in_quotes)
CUSTOM_DOC("Reads a filename from surrounding '\"' characters and attempts to open the corresponding file.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    if (!buffer.exists) return;
    
    char file_name_[256];
    String file_name = make_fixed_width_string(file_name_);
    
    int32_t pos = view.cursor.pos;
    int32_t start = 0;
    int32_t end = 0;
    buffer_seek_delimiter_forward(app, &buffer, pos, '"', &end);
    buffer_seek_delimiter_backward(app, &buffer, pos, '"', &start);
    ++start;
    
    int32_t size = end - start;
    
    char short_file_name[128];
    if (size < sizeof(short_file_name)){
        if (buffer_read_range(app, &buffer, start, end, short_file_name)){
            copy(&file_name, make_string(buffer.file_name, buffer.file_name_len));
            remove_last_folder(&file_name);
            append(&file_name, make_string(short_file_name, size));
            
            view = get_next_active_panel(app, &view);
            if (view.exists){
                if (view_open_file(app, &view, file_name.str, file_name.size, true)){
                    set_active_view(app, &view);
                }
            }
        }
    }
}

CUSTOM_COMMAND_SIG(open_matching_file_cpp)
CUSTOM_DOC("If the current file is a *.cpp or *.h, attempts to open the corresponding *.h or *.cpp file in the other view.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    Buffer_Summary new_buffer = {0};
    if (get_cpp_matching_file(app, buffer, &new_buffer)){
        get_view_next_looped(app, &view, AccessAll);
        view_set_buffer(app, &view, new_buffer.buffer_id, 0);
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(view_buffer_other_panel)
CUSTOM_DOC("Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.")
{
    View_Summary view = get_active_view(app, AccessAll);
    int32_t buffer_id = view.buffer_id;
    change_active_panel(app);
    view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, buffer_id, 0);
}

CUSTOM_COMMAND_SIG(swap_buffers_between_panels)
CUSTOM_DOC("Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.")
{
    View_Summary view1 = get_active_view(app, AccessAll);
    change_active_panel(app);
    View_Summary view2 = get_active_view(app, AccessAll);
    
    if (view1.view_id != view2.view_id){
        int32_t buffer_id1 = view1.buffer_id;
        int32_t buffer_id2 = view2.buffer_id;
        view_set_buffer(app, &view1, buffer_id2, 0);
        view_set_buffer(app, &view2, buffer_id1, 0);
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(undo)
CUSTOM_DOC("Advances backwards through the undo history.")
{
    exec_command(app, cmdid_undo);
}

CUSTOM_COMMAND_SIG(redo)
CUSTOM_DOC("Advances forewards through the undo history.")
{
    exec_command(app, cmdid_redo);
}

CUSTOM_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    exec_command(app, cmdid_interactive_new);
}

CUSTOM_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    exec_command(app, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively opens or creates a new file.")
{
    exec_command(app, cmdid_interactive_open_or_new);
}

CUSTOM_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    exec_command(app, cmdid_interactive_switch_buffer);
}

CUSTOM_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    exec_command(app, cmdid_interactive_kill_buffer);
}

CUSTOM_COMMAND_SIG(reopen)
CUSTOM_DOC("Reopen the current buffer from the hard drive.")
{
    exec_command(app, cmdid_reopen);
}

CUSTOM_COMMAND_SIG(save)
CUSTOM_DOC("Saves the current buffer.")
{
    exec_command(app, cmdid_save);
}

CUSTOM_COMMAND_SIG(kill_buffer)
CUSTOM_DOC("Kills the current buffer.")
{
    exec_command(app, cmdid_kill_buffer);
}

CUSTOM_COMMAND_SIG(open_color_tweaker)
CUSTOM_DOC("Opens the 4coder colors and fonts selector menu.")
{
    exec_command(app, cmdid_open_color_tweaker);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(open_in_other)
CUSTOM_DOC("Switches to the next active panel and begins an open file dialogue.")
{
    change_active_panel(app);
    interactive_open_or_new(app);
}

// BOTTOM

