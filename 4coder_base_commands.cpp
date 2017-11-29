/*
4coder_base_commands.cpp - Base commands such as inserting characters, and 
moving the cursor, which work even without the default 4coder framework.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_BASE_COMMANDS_CPP)
#define FCODER_BASE_COMMANDS_CPP

#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_long_seek.h"

#include "4coder_lib/4coder_utf8.h"

//
// Fundamental Editing Commands
//

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
        
        Marker_Handle handle = buffer_add_markers(app, &buffer, 1);
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
    
    Range range = get_range(&view);
    buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
}

//
// Basic Navigation Commands
//

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

inline void
move_vertical(Application_Links *app, float line_multiplier){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    
    float new_y = get_view_y(&view) + line_multiplier*view.line_height;
    float x = view.preferred_x;
    
    view_set_cursor(app, &view, seek_xy(x, new_y, 0, view.unwrapped_lines), 0);
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

static float
get_page_jump(View_Summary *view){
    i32_Rect region = view->file_region;
    float page_jump = 1;
    
    if (view->line_height > 0){
        page_jump = (float)(region.y1 - region.y0) / view->line_height;
        page_jump -= 3.f;
        if (page_jump <= 0){
            page_jump = 1.f;
        }
    }
    
    return(page_jump);
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
    view_set_cursor(app, &view, seek_character_pos(0), true);
    view_set_mark(app, &view, seek_character_pos(buffer.size));
}

//
// Long Seeks
//

CUSTOM_COMMAND_SIG(seek_whitespace_up)
CUSTOM_DOC("Seeks the cursor up to the next blank line.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = buffer_seek_whitespace_up(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down)
CUSTOM_DOC("Seeks the cursor down to the next blank line.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = buffer_seek_whitespace_down(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the line across all text.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = seek_line_beginning(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_end_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the end of the line across all text.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = seek_line_end(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the visual line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    
    view_set_cursor(app, &view, seek_xy(0, y, 1, view.unwrapped_lines), 1);
}

CUSTOM_COMMAND_SIG(seek_end_of_line)
CUSTOM_DOC("Seeks the cursor to the end of the visual line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    
    view_set_cursor(app, &view, seek_xy(100000.f, y, 1, view.unwrapped_lines), 1);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up_end_line)
CUSTOM_DOC("Seeks the cursor up to the next blank line and places it at the end of the line.")
{
    exec_command(app, seek_whitespace_up);
    exec_command(app, seek_end_of_line);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down_end_line)
CUSTOM_DOC("Seeks the cursor down to the next blank line and places it at the end of the line.")
{
    exec_command(app, seek_whitespace_down);
    exec_command(app, seek_end_of_line);
}


//
// Fancy Editing
//

CUSTOM_COMMAND_SIG(to_uppercase)
CUSTOM_DOC("Converts all ascii text in the range between the cursor and the mark to uppercase.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_range(&view);
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
    
    Range range = get_range(&view);
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


//
// Basic Panel Management
//

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


//
// Common Settings Commands
//

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

//
// Interactive Commands
//

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
    uint32_t access = AccessProtected;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
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
    
    Range range = get_range(&view);
    
    int32_t pos, new_pos;
    pos = range.min;
    
    buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    
    while (new_pos + r.size <= range.end){
        buffer_replace_range(app, &buffer, new_pos, new_pos + r.size, w.str, w.size);
        refresh_view(app, &view);
        range = get_range(&view);
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

//
// File Handling Commands
//

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

//
// cmdid wrappers
//

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

CUSTOM_COMMAND_SIG(open_debug)
CUSTOM_DOC("Opens a debug view for internal use.")
{
    exec_command(app, cmdid_open_debug);
}

#endif

// BOTTOM

