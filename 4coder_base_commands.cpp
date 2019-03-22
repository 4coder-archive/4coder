/*
4coder_base_commands.cpp - Base commands such as inserting characters, and 
moving the cursor, which work even without the default 4coder framework.
*/

// TOP

static void
write_character_parameter(Application_Links *app, u8 *character, u32 length){
    if (length != 0){
        View_Summary view = get_active_view(app, AccessOpen);
        if_view_has_highlighted_range_delete_range(app, view.view_id);
        view = get_view(app, view.view_id, AccessAll);
        
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
        i32 pos = view.cursor.pos;
        
        // NOTE(allen): setup markers to figure out the new position of cursor after the insert
        Marker next_cursor_marker = {};
        next_cursor_marker.pos = character_pos_to_pos(app, &view, &buffer, view.cursor.character_pos);
        next_cursor_marker.lean_right = true;
        Managed_Object handle = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, 1, 0);
        managed_object_store_data(app, handle, 0, 1, &next_cursor_marker);
        
        // NOTE(allen): consecutive inserts merge logic
        History_Record_Index first_index = 0;
        buffer_history_get_current_state_index(app, buffer.buffer_id, &first_index);
        b32 do_merge = false;
        if (character[0] != '\n'){
            Record_Info record = get_single_record(app, buffer.buffer_id, first_index);
            if (record.error == RecordError_NoError && record.kind == RecordKind_Single){
                String string = record.single.string_forward;
                i32 last_end = record.single.first + string.size;
                if (last_end == pos && string.size > 0){
                    char c = string.str[string.size - 1];
                    if (c != '\n'){
                        if (char_is_whitespace(character[0]) && char_is_whitespace(c)){
                            do_merge = true;
                        }
                        else if (char_is_alpha_numeric(character[0]) && char_is_alpha_numeric(c)){
                            do_merge = true;
                        }
                    }
                }
            }
        }
        
        // NOTE(allen): perform the edit
        b32 edit_success = buffer_replace_range(app, &buffer, pos, pos, (char*)character, length);
        
        // NOTE(allen): finish merging records if necessary
        if (do_merge){
            History_Record_Index last_index = 0;
            buffer_history_get_current_state_index(app, buffer.buffer_id, &last_index);
            buffer_history_merge_record_range(app, buffer.buffer_id, first_index, last_index, RecordMergeFlag_StateInRange_MoveStateForward);
        }
        
        // NOTE(allen): finish updating the cursor
        managed_object_load_data(app, handle, 0, 1, &next_cursor_marker);
        managed_object_free(app, handle);
        if (edit_success){
            view_set_cursor(app, &view, seek_pos(next_cursor_marker.pos), true);
        }
    }
}

CUSTOM_COMMAND_SIG(write_character)
CUSTOM_DOC("Inserts whatever character was used to trigger this command.")
{
    User_Input in = get_command_input(app);
    u8 character[4];
    u32 length = to_writable_character(in, character);
    write_character_parameter(app, character, length);
}

CUSTOM_COMMAND_SIG(write_underscore)
CUSTOM_DOC("Inserts an underscore.")
{
    u8 character = '_';
    write_character_parameter(app, &character, 1);
}

CUSTOM_COMMAND_SIG(delete_char)
CUSTOM_DOC("Deletes the character to the right of the cursor.")
{
    u32 access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    if (!if_view_has_highlighted_range_delete_range(app, view.view_id)){
        view = get_view(app, view.view_id, AccessAll);
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
        i32 start = view.cursor.pos;
        if (0 <= start && start < buffer.size){
            Full_Cursor cursor = {};
            view_compute_cursor(app, &view, seek_character_pos(view.cursor.character_pos + 1), &cursor);
            i32 end = cursor.pos;
            buffer_replace_range(app, &buffer, start, end, 0, 0);
        }
    }
}

CUSTOM_COMMAND_SIG(backspace_char)
CUSTOM_DOC("Deletes the character to the left of the cursor.")
{
    u32 access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    if (!if_view_has_highlighted_range_delete_range(app, view.view_id)){
        view = get_view(app, view.view_id, AccessAll);
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
        i32 end = view.cursor.pos;
        if (0 < end && end <= buffer.size){
            Full_Cursor cursor = {};
            view_compute_cursor(app, &view, seek_character_pos(view.cursor.character_pos - 1), &cursor);
            i32 start = cursor.pos;
            if (buffer_replace_range(app, &buffer, start, end, 0, 0)){
                view_set_cursor(app, &view, seek_character_pos(view.cursor.character_pos - 1), true);
            }
        }
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
    i32 cursor = view.cursor.pos;
    i32 mark = view.mark.pos;
    view_set_cursor(app, &view, seek_pos(mark), true);
    view_set_mark(app, &view, seek_pos(cursor));
}

CUSTOM_COMMAND_SIG(delete_range)
CUSTOM_DOC("Deletes the text in the range between the cursor and the mark.")
{
    u32 access = AccessOpen;
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
    
    Rect_i32 region = {};
    view_get_buffer_region(app, view.view_id, &region);
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    f32 h = (f32)(rect_height(region));
    f32 y = get_view_y(&view);
    y = y - h*.5f;
    scroll.target_y = (i32)(y + .5f);
    view_set_scroll(app, &view, scroll);
}

CUSTOM_COMMAND_SIG(left_adjust_view)
CUSTOM_DOC("Sets the left size of the view near the x position of the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    f32 x = get_view_x(&view) - 30.f;
    if (x < 0){
        x = 0.f;
    }
    
    scroll.target_x = (i32)(x + .5f);
    view_set_scroll(app, &view, scroll);
}

static b32
view_space_from_screen_space_checked(Vec2_i32 p, Rect_i32 file_region, Vec2 scroll_p, Vec2 *p_out){
    b32 result = false;
    if (hit_check(file_region, p)){
        *p_out = view_space_from_screen_space(V2(p), V2(file_region.p0), scroll_p);
        result = true;
    }
    else{
        *p_out = V2(0.f, 0.f);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(click_set_cursor_and_mark)
CUSTOM_DOC("Sets the cursor position and mark to the mouse position.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Mouse_State mouse = get_mouse_state(app);
    Vec2 p = {};
    Rect_i32 region = {};
    view_get_buffer_region(app, view.view_id, &region);
    if (view_space_from_screen_space_checked(mouse.p, region, view.scroll_vars.scroll_p, &p)){
        view_set_cursor(app, &view, seek_xy(p.x, p.y, true, view.unwrapped_lines), true);
        view_set_mark(app, &view, seek_pos(view.cursor.pos));
    }
}

CUSTOM_COMMAND_SIG(click_set_cursor)
CUSTOM_DOC("Sets the cursor position to the mouse position.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Mouse_State mouse = get_mouse_state(app);
    Vec2 p = {};
    Rect_i32 region = {};
    view_get_buffer_region(app, view.view_id, &region);
    if (view_space_from_screen_space_checked(mouse.p, region, view.scroll_vars.scroll_p, &p)){
        view_set_cursor(app, &view, seek_xy(p.x, p.y, true, view.unwrapped_lines), true);
    }
    no_mark_snap_to_cursor(app, view.view_id);
}

CUSTOM_COMMAND_SIG(click_set_cursor_if_lbutton)
CUSTOM_DOC("If the mouse left button is pressed, sets the cursor position to the mouse position.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Mouse_State mouse = get_mouse_state(app);
    if (mouse.l){
        Vec2 p = {};
        Rect_i32 region = {};
        view_get_buffer_region(app, view.view_id, &region);
        if (view_space_from_screen_space_checked(mouse.p, region, view.scroll_vars.scroll_p, &p)){
            view_set_cursor(app, &view, seek_xy(p.x, p.y, true, view.unwrapped_lines), true);
        }
    }
    no_mark_snap_to_cursor(app, view.view_id);
}

CUSTOM_COMMAND_SIG(click_set_mark)
CUSTOM_DOC("Sets the mark position to the mouse position.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Mouse_State mouse = get_mouse_state(app);
    Vec2 p = {};
    Rect_i32 region = {};
    view_get_buffer_region(app, view.view_id, &region);
    if (view_space_from_screen_space_checked(mouse.p, region, view.scroll_vars.scroll_p, &p)){
        view_set_mark(app, &view, seek_xy(p.x, p.y, true, view.unwrapped_lines));
    }
    no_mark_snap_to_cursor(app, view.view_id);
}

CUSTOM_COMMAND_SIG(mouse_wheel_scroll)
CUSTOM_DOC("Reads the scroll wheel value from the mouse state and scrolls accordingly.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Mouse_State mouse = get_mouse_state(app);
    if (mouse.wheel != 0){
        GUI_Scroll_Vars scroll = view.scroll_vars;
        scroll.target_y += mouse.wheel;
        view_set_scroll(app, &view, scroll);
    }
}

////////////////////////////////

static void
move_vertical(Application_Links *app, f32 line_multiplier){
    u32 access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    
    f32 delta_y = line_multiplier*view.line_height;
    f32 new_y = get_view_y(&view) + delta_y;
    f32 x = view.preferred_x;
    
    view_set_cursor(app, &view, seek_xy(x, new_y, 0, view.unwrapped_lines), 0);
    f32 actual_new_y = get_view_y(&view);
    if (actual_new_y < new_y){
        Rect_i32 file_region = {};
        view_get_buffer_region(app, view.view_id, &file_region);
        i32 height = rect_height(file_region);
        i32 full_scroll_y = (i32)actual_new_y - height/2;
        if (view.scroll_vars.target_y < full_scroll_y){
            GUI_Scroll_Vars new_scroll_vars = view.scroll_vars;
            new_scroll_vars.target_y += (i32)delta_y;
            if (new_scroll_vars.target_y > full_scroll_y){
                new_scroll_vars.target_y = full_scroll_y;
            }
            view_set_scroll(app, &view, new_scroll_vars);
        }
    }
    
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

static f32
get_page_jump(Application_Links *app, View_Summary *view){
    Rect_i32 region = {};
    view_get_buffer_region(app, view->view_id, &region);
    f32 page_jump = 1.f;
    if (view->line_height > 0.f){
        i32 height = region.y1 - region.y0;
        f32 line_count = (f32)(height)/view->line_height;
        i32 line_count_rounded = (i32)line_count;
        page_jump = (f32)line_count_rounded - 3.f;
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
    i32 next_line = view.cursor.line + 1;
    view_set_cursor(app, &view, seek_line_char(next_line, 1), true);
}

CUSTOM_COMMAND_SIG(page_up)
CUSTOM_DOC("Scrolls the view up one view height and moves the cursor up one view height.")
{
    u32 access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    f32 page_jump = get_page_jump(app, &view);
    move_vertical(app, -page_jump);
}

CUSTOM_COMMAND_SIG(page_down)
CUSTOM_DOC("Scrolls the view down one view height and moves the cursor down one view height.")
{
    u32 access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    f32 page_jump = get_page_jump(app, &view);
    move_vertical(app, page_jump);
}

////////////////

CUSTOM_COMMAND_SIG(move_left)
CUSTOM_DOC("Moves the cursor one character to the left.")
{
    u32 access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    i32 new_pos = view.cursor.character_pos - 1;
    view_set_cursor(app, &view, seek_character_pos(new_pos), 1);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(move_right)
CUSTOM_DOC("Moves the cursor one character to the right.")
{
    u32 access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    i32 new_pos = view.cursor.character_pos + 1;
    view_set_cursor(app, &view, seek_character_pos(new_pos), 1);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(select_all)
CUSTOM_DOC("Puts the cursor at the top of the file, and the mark at the bottom of the file.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    view_set_cursor(app, &view, seek_pos(0), true);
    view_set_mark(app, &view, seek_pos(buffer.size));
    no_mark_snap_to_cursor(app, view.view_id);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(to_uppercase)
CUSTOM_DOC("Converts all ascii text in the range between the cursor and the mark to uppercase.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_view_range(&view);
    i32 size = range.max - range.min;
    if (size <= app->memory_size){
        char *mem = (char*)app->memory;
        
        buffer_read_range(app, &buffer, range.min, range.max, mem);
        for (i32 i = 0; i < size; ++i){
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
    i32 size = range.max - range.min;
    if (size <= app->memory_size){
        char *mem = (char*)app->memory;
        
        buffer_read_range(app, &buffer, range.min, range.max, mem);
        for (i32 i = 0; i < size; ++i){
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
    
    i32 line_count = buffer.line_count;
    i32 edit_max = line_count;
    
    if (edit_max*(i32)sizeof(Buffer_Edit) < app->memory_size){
        Buffer_Edit *edits = (Buffer_Edit*)app->memory;
        
        char data[1024];
        Stream_Chunk chunk = {};
        
        i32 i = 0;
        if (init_stream_chunk(&chunk, app, &buffer, i, data, sizeof(data))){
            Buffer_Edit *edit = edits;
            
            i32 buffer_size = buffer.size;
            i32 still_looping = true;
            i32 last_hard = buffer_size;
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
            
            i32 edit_count = (i32)(edit - edits);
            buffer_batch_edit(app, &buffer, 0, 0, edits, edit_count, BatchEdit_PreserveTokens);
        }
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(basic_change_active_panel)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next highest view_id.  Will not skipe the build panel if it is open.")
{
    View_Summary view = get_active_view(app, AccessAll);
    get_next_view_looped_all_panels(app, &view, AccessAll);
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
    b32 value;
    view_get_setting(app, &view, ViewSetting_ShowFileBar, &value);
    view_set_setting(app, &view, ViewSetting_ShowFileBar, !value);
}

CUSTOM_COMMAND_SIG(toggle_line_wrap)
CUSTOM_DOC("Toggles the current buffer's line wrapping status.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    b32 unwrapped = view.unwrapped_lines;
    buffer_set_setting(app, &buffer, BufferSetting_WrapLine, unwrapped);
}

CUSTOM_COMMAND_SIG(toggle_fps_meter)
CUSTOM_DOC("Toggles the visibility of the FPS performance meter")
{
    show_fps_hud = !show_fps_hud;
}

CUSTOM_COMMAND_SIG(increase_line_wrap)
CUSTOM_DOC("Increases the current buffer's width for line wrapping.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    i32 wrap = 0;
    buffer_get_setting(app, &buffer, BufferSetting_WrapPosition, &wrap);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap + 10);
}

CUSTOM_COMMAND_SIG(decrease_line_wrap)
CUSTOM_DOC("Decrases the current buffer's width for line wrapping.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    i32 wrap = 0;
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

CUSTOM_COMMAND_SIG(mouse_wheel_change_face_size)
CUSTOM_DOC("Reads the state of the mouse wheel and uses it to either increase or decrease the face size.")
{
    static Microsecond_Time_Stamp next_resize_time = 0;
    Microsecond_Time_Stamp now = get_microseconds_timestamp(app);
    if (now >= next_resize_time){
        next_resize_time = now + 50*1000;
        Mouse_State mouse = get_mouse_state(app);
        if (mouse.wheel > 0){
            decrease_face_size(app);
        }
        else if (mouse.wheel < 0){
            increase_face_size(app);
        }
    }
}

CUSTOM_COMMAND_SIG(toggle_virtual_whitespace)
CUSTOM_DOC("Toggles the current buffer's virtual whitespace status.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    i32 vwhite = 0;
    buffer_get_setting(app, &buffer, BufferSetting_VirtualWhitespace, &vwhite);
    buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, !vwhite);
}

CUSTOM_COMMAND_SIG(toggle_show_whitespace)
CUSTOM_DOC("Toggles the current buffer's whitespace visibility status.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    view_set_setting(app, &view, ViewSetting_ShowWhitespace, !view.show_whitespace);
}

CUSTOM_COMMAND_SIG(toggle_line_numbers)
CUSTOM_DOC("Toggles the left margin line numbers.")
{
    global_config.show_line_number_margins = !global_config.show_line_number_margins;
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
    u32 access = AccessProtected;
    
    Query_Bar bar = {};
    char string_space[256];
    
    bar.prompt = make_lit_string("Goto Line: ");
    bar.string = make_fixed_width_string(string_space);
    
    if (query_user_number(app, &bar)){
        i32 line_number = str_to_int_s(bar.string);
        
        View_Summary view = get_active_view(app, access);
        view_set_cursor(app, &view, seek_line_char(line_number, 0), true);
    }
}

CUSTOM_COMMAND_SIG(search);
CUSTOM_COMMAND_SIG(reverse_search);

static void
isearch__update_highlight(Application_Links *app, View_Summary *view, Managed_Object highlight,
                          i32 start, i32 end){
    Marker markers[4] = {};
    markers[0].pos = start;
    markers[1].pos = end;
    managed_object_store_data(app, highlight, 0, 2, markers);
    view_set_cursor(app, view, seek_pos(start), false);
}

static void
isearch(Application_Links *app, b32 start_reversed, String query_init, b32 on_the_query_init_string){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    if (!buffer.exists){
        return;
    }
    
    Query_Bar bar = {};
    if (start_query_bar(app, &bar, 0) == 0){
        return;
    }
    
    b32 reverse = start_reversed;
    i32 first_pos = view.cursor.pos;
    
    i32 pos = first_pos;
    if (query_init.size != 0){
        pos += 2;
    }
    
    i32 start_pos = pos;
    Range match = make_range(pos, pos);
    
    char bar_string_space[256];
    bar.string = make_fixed_width_string(bar_string_space);
    copy(&bar.string, query_init);
    
    String isearch_str = make_lit_string("I-Search: ");
    String rsearch_str = make_lit_string("Reverse-I-Search: ");
    
    b32 first_step = true;
    
    Managed_Scope view_scope = view_get_managed_scope(app, view.view_id);
    Managed_Object highlight = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, 2, &view_scope);
    Marker_Visual visual = create_marker_visual(app, highlight);
    marker_visual_set_effect(app, visual,
                             VisualType_CharacterHighlightRanges,
                             Stag_Highlight,
                             Stag_At_Highlight, 0);
    marker_visual_set_view_key(app, visual, view.view_id);
    marker_visual_set_priority(app, visual, VisualPriority_Default + 1);
    isearch__update_highlight(app, &view, highlight, match.start, match.end);
    cursor_is_hidden = true;
    
    User_Input in = {};
    for (;;){
        // NOTE(allen): Change the bar's prompt to match the current direction.
        if (reverse){
            bar.prompt = rsearch_str;
        }
        else{
            bar.prompt = isearch_str;
        }
        
        b32 step_forward = false;
        b32 step_backward = false;
        b32 backspace = false;
        b32 suppress_highligh_update = false;
        
        if (!first_step){
            in = get_user_input(app, EventOnAnyKey, EventOnEsc);
            if (in.abort) break;
            
            u8 character[4];
            u32 length = to_writable_character(in, character);
            
            b32 made_change = false;
            if (in.key.keycode == '\n' || in.key.keycode == '\t'){
                if (in.key.modifiers[MDFR_CONTROL_INDEX]){
                    copy(&bar.string, previous_isearch_query);
                }
                else{
                    String previous_isearch_query_str = make_fixed_width_string(previous_isearch_query);
                    append(&previous_isearch_query_str, bar.string);
                    terminate_with_null(&previous_isearch_query_str);
                    break;
                }
            }
            else if (length != 0 && key_is_unmodified(&in.key)){
                append(&bar.string, make_string(character, length));
                made_change = true;
            }
            else if (in.key.keycode == key_back){
                if (key_is_unmodified(&in.key)){
                    made_change = backspace_utf8(&bar.string);
                    backspace = true;
                }
                else if (in.key.modifiers[MDFR_CONTROL_INDEX]){
                    if (bar.string.size > 0){
                        made_change = true;
                        bar.string.size = 0;
                        backspace = true;
                    }
                }
            }
            
            if ((in.command.command == search) || in.key.keycode == key_page_down || in.key.keycode == key_down){
                step_forward = true;
            }
            
            if (in.command.command == mouse_wheel_scroll){
                mouse_wheel_scroll(app);
                suppress_highligh_update = true;
            }
            
            if ((in.command.command == reverse_search) || in.key.keycode == key_page_up || in.key.keycode == key_up){
                step_backward = true;
            }
        }
        else{
            if (query_init.size != 0 && on_the_query_init_string){
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
                i32 new_pos = 0;
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
                i32 new_pos = 0;
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
        
        if (!suppress_highligh_update){
            isearch__update_highlight(app, &view, highlight, match.start, match.end);
        }
    }
    
    managed_object_free(app, highlight);
    cursor_is_hidden = false;
    
    if (in.abort){
        String previous_isearch_query_str = make_fixed_width_string(previous_isearch_query);
        append(&previous_isearch_query_str, bar.string);
        terminate_with_null(&previous_isearch_query_str);
        view_set_cursor(app, &view, seek_pos(first_pos), true);
    }
}

CUSTOM_COMMAND_SIG(search)
CUSTOM_DOC("Begins an incremental search down through the current buffer for a user specified string.")
{
    String query = {};
    isearch(app, false, query, false);
}

CUSTOM_COMMAND_SIG(reverse_search)
CUSTOM_DOC("Begins an incremental search up through the current buffer for a user specified string.")
{
    String query = {};
    isearch(app, true, query, false);
}

CUSTOM_COMMAND_SIG(search_identifier)
CUSTOM_DOC("Begins an incremental search down through the current buffer for the word or token under the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    char space[256];
    String query = read_identifier_at_pos(app, &buffer, view.cursor.pos, space, sizeof(space), 0);
    isearch(app, false, query, true);
}

CUSTOM_COMMAND_SIG(reverse_search_identifier)
CUSTOM_DOC("Begins an incremental search up through the current buffer for the word or token under the cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    char space[256];
    String query = read_identifier_at_pos(app, &buffer, view.cursor.pos, space, sizeof(space), 0);
    isearch(app, true, query, true);
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
    
    u32 access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Range range = get_view_range(&view);
    
    i32 pos = range.min;
    i32 new_pos;
    buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    
    global_history_edit_group_begin(app);
    for (;new_pos + r.size <= range.end;){
        buffer_replace_range(app, &buffer, new_pos, new_pos + r.size, w.str, w.size);
        refresh_view(app, &view);
        range = get_view_range(&view);
        pos = new_pos + w.size;
        buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    }
    global_history_edit_group_end(app);
}

static void
query_replace_base(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 pos, String r, String w){
    i32 new_pos = 0;
    buffer_seek_string_forward(app, buffer, pos, 0, r.str, r.size, &new_pos);
    
    Managed_Scope view_scope = view_get_managed_scope(app, view->view_id);
    Managed_Object highlight = alloc_buffer_markers_on_buffer(app, buffer->buffer_id, 2, &view_scope);
    Marker_Visual visual = create_marker_visual(app, highlight);
    marker_visual_set_effect(app, visual,
                             VisualType_CharacterHighlightRanges,
                             Stag_Highlight,
                             Stag_At_Highlight, 0);
    marker_visual_set_view_key(app, visual, view->view_id);
    cursor_is_hidden = true;
    
    User_Input in = {};
    for (;new_pos < buffer->size;){
        Range match = make_range(new_pos, new_pos + r.size);
        isearch__update_highlight(app, view, highlight, match.min, match.max);
        
        in = get_user_input(app, EventOnAnyKey, EventOnMouseLeftButton|EventOnMouseRightButton);
        if (in.abort || in.key.keycode == key_esc || !key_is_unmodified(&in.key)) break;
        
        if (in.key.character == 'y' || in.key.character == 'Y' ||
            in.key.character == '\n' || in.key.character == '\t'){
            buffer_replace_range(app, buffer, match.min, match.max, w.str, w.size);
            pos = match.start + w.size;
        }
        else{
            pos = match.max;
        }
        
        buffer_seek_string_forward(app, buffer, pos, 0, r.str, r.size, &new_pos);
    }
    
    managed_object_free(app, highlight);
    cursor_is_hidden = false;
    
    if (in.abort){
        return;
    }
    
    view_set_cursor(app, view, seek_pos(pos), true);
}

static void
query_replace_parameter(Application_Links *app, String replace_str, i32 start_pos, b32 add_replace_query_bar){
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
    
    if (!query_user_string(app, &with)){
        return;
    }
    
    String r = replace.string;
    String w = with.string;
    
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    i32 pos = start_pos;
    
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
    
    Query_Bar replace = {};
    char replace_space[1024];
    replace.prompt = make_lit_string("Replace: ");
    replace.string = make_fixed_width_string(replace_space);
    
    if (!query_user_string(app, &replace)){
        return;
    }
    if (replace.string.size == 0){
        return;
    }
    
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
    
    Range range = {};
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
    i32 replace_length = range.max - range.min;
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

static void
save_all_dirty_buffers_with_postfix(Application_Links *app, String postfix){
    for (Buffer_Summary buffer = get_buffer_first(app, AccessOpen);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessOpen)){
        if (buffer.dirty == DirtyState_UnsavedChanges){
            String file_name = make_string(buffer.file_name, buffer.file_name_len);
            if (file_name.size >= postfix.size){
                String file_name_post = substr_tail(file_name, file_name.size - postfix.size);
                if (match(file_name_post, postfix)){
                    save_buffer(app, &buffer, buffer.file_name, buffer.file_name_len, 0);
                }
            }
        }
    }
}

CUSTOM_COMMAND_SIG(save_all_dirty_buffers)
CUSTOM_DOC("Saves all buffers marked dirty (showing the '*' indicator).")
{
    String empty = {};
    save_all_dirty_buffers_with_postfix(app, empty);
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
        String file_name = {};
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
CUSTOM_DOC("Queries the user for a file name and saves the contents of the current buffer, altering the buffer's name too.")
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
    i32 hot_dir_size = directory_get_hot(app, 0, 0);
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
                Buffer_Summary new_buffer = create_buffer(app, new_file_name.str, new_file_name.size, BufferCreate_NeverNew|BufferCreate_JustChangedFile);
                if (new_buffer.exists && new_buffer.buffer_id != buffer.buffer_id){
                    delete_file_base(app, file_name, buffer.buffer_id);
                    view_set_buffer(app, &view, new_buffer.buffer_id, 0);
                }
            }
        }
    }
}

CUSTOM_COMMAND_SIG(make_directory_query)
CUSTOM_DOC("Queries the user for a name and creates a new directory with the given name.")
{
    char hot_space[2048];
    i32 hot_length = directory_get_hot(app, hot_space, sizeof(hot_space));
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
    
    Full_Cursor prev_line_cursor = {};
    Full_Cursor this_line_cursor = {};
    Full_Cursor next_line_cursor = {};
    
    i32 this_line = view.cursor.line;
    i32 prev_line = this_line - 1;
    i32 next_line = this_line +1;
    
    if (view_compute_cursor(app, &view, seek_line_char(prev_line, 1), &prev_line_cursor) &&
        view_compute_cursor(app, &view, seek_line_char(this_line, 1), &this_line_cursor) &&
        view_compute_cursor(app, &view, seek_line_char(next_line, 1), &next_line_cursor)){
        
        i32 prev_line_pos = prev_line_cursor.pos;
        i32 this_line_pos = this_line_cursor.pos;
        i32 next_line_pos = next_line_cursor.pos;
        
        Partition *part = &global_part;
        Temp_Memory temp = begin_temp_memory(part);
        
        i32 length = next_line_pos - prev_line_pos;
        char *swap = push_array(part, char, length + 1);
        i32 first_len = next_line_pos - this_line_pos;
        
        if (buffer_read_range(app, &buffer, this_line_pos, next_line_pos, swap)){
            b32 second_line_didnt_have_newline = true;
            for (i32 i = first_len - 1; i >= 0; --i){
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
    
    i32 next_line = view.cursor.line + 1;
    Full_Cursor new_cursor = {};
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
    String line_string = {};
    char *before_line = push_array(part, char, 1);
    if (read_line(app, part, &buffer, view.cursor.line, &line_string)){
        *before_line = '\n';
        line_string.str = before_line;
        line_string.size += 1;
        
        i32 pos = buffer_get_line_end(app, &buffer, view.cursor.line);
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
    i32 start = buffer_get_line_start(app, &buffer, view.cursor.line);
    i32 end = buffer_get_line_end(app, &buffer, view.cursor.line) + 1;
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

static b32
get_cpp_matching_file(Application_Links *app, Buffer_Summary buffer, Buffer_Summary *buffer_out){
    b32 result = false;
    
    if (buffer.file_name != 0){
        char space[512];
        String file_name = make_string_cap(space, 0, sizeof(space));
        append(&file_name, make_string(buffer.file_name, buffer.file_name_len));
        
        String extension = file_extension(file_name);
        String new_extensions[2] = {};
        i32 new_extensions_count = 0;
        
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
        i32 base_pos = file_name.size;
        for (i32 i = 0; i < new_extensions_count; ++i){
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
    
    i32 pos = view.cursor.pos;
    i32 start = 0;
    i32 end = 0;
    buffer_seek_delimiter_forward(app, &buffer, pos, '"', &end);
    buffer_seek_delimiter_backward(app, &buffer, pos, '"', &start);
    ++start;
    
    i32 size = end - start;
    
    char short_file_name[128];
    if (size < sizeof(short_file_name)){
        if (buffer_read_range(app, &buffer, start, end, short_file_name)){
            copy(&file_name, make_string(buffer.file_name, buffer.file_name_len));
            remove_last_folder(&file_name);
            append(&file_name, make_string(short_file_name, size));
            
            get_next_view_looped_primary_panels(app, &view, AccessAll);
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
    
    Buffer_Summary new_buffer = {};
    if (get_cpp_matching_file(app, buffer, &new_buffer)){
        get_next_view_looped_primary_panels(app, &view, AccessAll);
        view_set_buffer(app, &view, new_buffer.buffer_id, 0);
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(view_buffer_other_panel)
CUSTOM_DOC("Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.")
{
    View_Summary view = get_active_view(app, AccessAll);
    i32 buffer_id = view.buffer_id;
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
        i32 buffer_id1 = view1.buffer_id;
        i32 buffer_id2 = view2.buffer_id;
        if (buffer_id1 != buffer_id2){
            view_set_buffer(app, &view1, buffer_id2, 0);
            view_set_buffer(app, &view2, buffer_id1, 0);
        }
        else{
            Full_Cursor v1_c = view1.cursor;
            Full_Cursor v1_m = view1.mark;
            GUI_Scroll_Vars v1_r = view1.scroll_vars;
            Full_Cursor v2_c = view2.cursor;
            Full_Cursor v2_m = view2.mark;
            GUI_Scroll_Vars v2_r = view2.scroll_vars;
            view_set_cursor(app, &view1, seek_pos(v2_c.pos), true);
            view_set_mark  (app, &view1, seek_pos(v2_m.pos));
            view_set_scroll(app, &view1, v2_r);
            view_set_cursor(app, &view2, seek_pos(v1_c.pos), true);
            view_set_mark  (app, &view2, seek_pos(v1_m.pos));
            view_set_scroll(app, &view2, v1_r);
        }
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(kill_buffer)
CUSTOM_DOC("Kills the current buffer.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    kill_buffer(app, buffer_identifier(view.buffer_id), view.view_id, 0);
}

CUSTOM_COMMAND_SIG(save)
CUSTOM_DOC("Saves the current buffer.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    save_buffer(app, &buffer, buffer.file_name, buffer.file_name_len, 0);
}

CUSTOM_COMMAND_SIG(reopen)
CUSTOM_DOC("Reopen the current buffer from the hard drive.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    reopen_buffer(app, &buffer, 0);
}

////////////////////////////////

static i32
record_get_new_cursor_position_undo(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, Record_Info record){
    i32 new_edit_position = 0;
    switch (record.kind){
        default:
        case RecordKind_Single:
        {
            new_edit_position = record.single.first + record.single.string_backward.size;
        }break;
        case RecordKind_Group:
        {
            Record_Info sub_record = {};
            buffer_history_get_group_sub_record(app, buffer_id, index, 0, &sub_record);
            new_edit_position = sub_record.single.first + sub_record.single.string_backward.size;
        }break;
    }
    return(new_edit_position);
}

static i32
record_get_new_cursor_position_undo(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Record_Info record = {};
    buffer_history_get_record_info(app, buffer_id, index, &record);
    return(record_get_new_cursor_position_undo(app, buffer_id, index, record));
}

static i32
record_get_new_cursor_position_redo(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, Record_Info record){
    i32 new_edit_position = 0;
    switch (record.kind){
        default:
        case RecordKind_Single:
        {
            new_edit_position = record.single.first + record.single.string_forward.size;
        }break;
        case RecordKind_Group:
        {
            Record_Info sub_record = {};
            buffer_history_get_group_sub_record(app, buffer_id, index, record.group.count - 1, &sub_record);
            new_edit_position = sub_record.single.first + sub_record.single.string_forward.size;
        }break;
    }
    return(new_edit_position);
}

static i32
record_get_new_cursor_position_redo(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Record_Info record = {};
    buffer_history_get_record_info(app, buffer_id, index, &record);
    return(record_get_new_cursor_position_redo(app, buffer_id, index, record));
}

CUSTOM_COMMAND_SIG(undo_this_buffer)
CUSTOM_DOC("Advances backwards through the undo history of the current buffer.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_ID buffer_id = view.buffer_id;
    History_Record_Index current = 0;
    buffer_history_get_current_state_index(app, buffer_id, &current);
    if (current > 0){
        i32 new_position = record_get_new_cursor_position_undo(app, buffer_id, current);
        buffer_history_set_current_state_index(app, buffer_id, current - 1);
        view_set_cursor(app, view.view_id, seek_pos(new_position), true);
    }
}

CUSTOM_COMMAND_SIG(redo_this_buffer)
CUSTOM_DOC("Advances forwards through the undo history of the current buffer.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_ID buffer_id = view.buffer_id;
    History_Record_Index current = 0;
    History_Record_Index max_index = 0;
    buffer_history_get_current_state_index(app, buffer_id, &current);
    buffer_history_get_max_record_index(app, buffer_id, &max_index);
    if (current < max_index){
        i32 new_position = record_get_new_cursor_position_redo(app, buffer_id, current + 1);
        buffer_history_set_current_state_index(app, buffer_id, current + 1);
        view_set_cursor(app, view.view_id, seek_pos(new_position), true);
    }
}

CUSTOM_COMMAND_SIG(undo)
CUSTOM_DOC("Advances backward through the undo history in the buffer containing the most recent regular edit.")
{
    Partition *scratch = &global_part;
    i32 highest_edit_number = -1;
    Buffer_ID first_buffer_match = 0;
    Buffer_ID last_buffer_match = 0;
    i32 match_count = 0;
    
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        History_Record_Index index = 0;
        buffer_history_get_current_state_index(app, buffer.buffer_id, &index);
        if (index > 0){
            Record_Info record = {};
            buffer_history_get_record_info(app, buffer.buffer_id, index, &record);
            if (record.edit_number > highest_edit_number){
                highest_edit_number = record.edit_number;
                first_buffer_match = buffer.buffer_id;
                last_buffer_match = buffer.buffer_id;
                match_count = 1;
            }
            else if (record.edit_number == highest_edit_number){
                last_buffer_match = buffer.buffer_id;
                match_count += 1;
            }
        }
    }
    
    Temp_Memory temp = begin_temp_memory(scratch);
    Buffer_ID *match_buffers = push_array(scratch, Buffer_ID, match_count);
    i32 *new_positions = push_array(scratch, i32, match_count);
    match_count = 0;
    
    if (highest_edit_number != -1){
        for (Buffer_Summary buffer = get_buffer(app, first_buffer_match, AccessAll);
             buffer.exists;
             get_buffer_next(app, &buffer, AccessAll)){
            b32 did_match = false;
            i32 new_edit_position = 0;
            for (;;){
                History_Record_Index index = 0;
                buffer_history_get_current_state_index(app, buffer.buffer_id, &index);
                if (index > 0){
                    Record_Info record = {};
                    buffer_history_get_record_info(app, buffer.buffer_id, index, &record);
                    if (record.edit_number == highest_edit_number){
                        did_match = true;
                        new_edit_position = record_get_new_cursor_position_undo(app, buffer.buffer_id, index, record);
                        buffer_history_set_current_state_index(app, buffer.buffer_id, index - 1);
                    }
                    else{
                        break;
                    }
                }
                else{
                    break;
                }
            }
            if (did_match){
                match_buffers[match_count] = buffer.buffer_id;
                new_positions[match_count] = new_edit_position;
                match_count += 1;
            }
            if (buffer.buffer_id == last_buffer_match){
                break;
            }
        }
    }
    
    view_buffer_set(app, match_buffers, new_positions, match_count);
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(redo)
CUSTOM_DOC("Advances forward through the undo history in the buffer containing the most recent regular edit.")
{
    Partition *scratch = &global_part;
    i32 lowest_edit_number = 0x7FFFFFFF;
    Buffer_ID first_buffer_match = 0;
    Buffer_ID last_buffer_match = 0;
    i32 match_count = 0;
    
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        History_Record_Index max_index = 0;
        History_Record_Index index = 0;
        buffer_history_get_max_record_index(app, buffer.buffer_id, &max_index);
        buffer_history_get_current_state_index(app, buffer.buffer_id, &index);
        if (index < max_index){
            Record_Info record = {};
            buffer_history_get_record_info(app, buffer.buffer_id, index + 1, &record);
            if (record.edit_number < lowest_edit_number){
                lowest_edit_number = record.edit_number;
                first_buffer_match = buffer.buffer_id;
                last_buffer_match = buffer.buffer_id;
                match_count = 1;
            }
            else if (record.edit_number == lowest_edit_number){
                last_buffer_match = buffer.buffer_id;
                match_count += 1;
            }
        }
    }
    
    Temp_Memory temp = begin_temp_memory(scratch);
    Buffer_ID *match_buffers = push_array(scratch, Buffer_ID, match_count);
    i32 *new_positions = push_array(scratch, i32, match_count);
    match_count = 0;
    
    if (lowest_edit_number != -1){
        for (Buffer_Summary buffer = get_buffer(app, first_buffer_match, AccessAll);
             buffer.exists;
             get_buffer_next(app, &buffer, AccessAll)){
            b32 did_match = false;
            i32 new_edit_position = 0;
            History_Record_Index max_index = 0;
            buffer_history_get_max_record_index(app, buffer.buffer_id, &max_index);
            for (;;){
                History_Record_Index index = 0;
                buffer_history_get_current_state_index(app, buffer.buffer_id, &index);
                if (index < max_index){
                    Record_Info record = {};
                    buffer_history_get_record_info(app, buffer.buffer_id, index + 1, &record);
                    if (record.edit_number == lowest_edit_number){
                        did_match = true;
                        new_edit_position = record_get_new_cursor_position_redo(app, buffer.buffer_id, index + 1, record);
                        buffer_history_set_current_state_index(app, buffer.buffer_id, index + 1);
                    }
                    else{
                        break;
                    }
                }
                else{
                    break;
                }
            }
            if (did_match){
                match_buffers[match_count] = buffer.buffer_id;
                new_positions[match_count] = new_edit_position;
                match_count += 1;
            }
            if (buffer.buffer_id == last_buffer_match){
                break;
            }
        }
    }
    
    view_buffer_set(app, match_buffers, new_positions, match_count);
    
    end_temp_memory(temp);
}

////////////////////////////////

#if 0
CUSTOM_COMMAND_SIG(reload_themes)
CUSTOM_DOC("Loads all the theme files in the theme folder, replacing duplicates with the new theme data.")
{
    String fcoder_extension = make_lit_string(".4coder");
    save_all_dirty_buffers_with_postfix(app, fcoder_extension);
    
    Partition *scratch = &global_part;
    Temp_Memory temp = begin_temp_memory(scratch);
    load_folder_of_themes_into_live_set(app, scratch, "themes");
    String name = get_theme_name(app, scratch, 0);
    i32 theme_count = get_theme_count(app);
    for (i32 i = 1; i < theme_count; i += 1){
        Temp_Memory sub_temp = begin_temp_memory(scratch);
        String style_name = get_theme_name(app, scratch, i);
        if (match(name, style_name)){
            change_theme_by_index(app, i);
            break;
        }
        end_temp_memory(sub_temp);
    }
    end_temp_memory(temp);
}
#endif

CUSTOM_COMMAND_SIG(open_in_other)
CUSTOM_DOC("Interactively opens a file in the other panel.")
{
    change_active_panel(app);
    interactive_open_or_new(app);
}

// BOTTOM

