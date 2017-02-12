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

//
// Fundamental Editing Commands
//

CUSTOM_COMMAND_SIG(write_character){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    
    User_Input in = get_command_input(app);
    
    char character = 0;
    if (in.type == UserInputKey){
        character = to_writable_char(in.key.character);
    }
    
    if (character != 0){
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
        int32_t pos = view.cursor.pos;
        buffer_replace_range(app, &buffer, pos, pos, &character, 1);
        view_set_cursor(app, &view, seek_pos(view.cursor.pos + 1), true);
    }
}

CUSTOM_COMMAND_SIG(delete_char){
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

CUSTOM_COMMAND_SIG(backspace_char){
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

CUSTOM_COMMAND_SIG(set_mark){
    View_Summary view = get_active_view(app, AccessProtected);
    
    view_set_mark(app, &view, seek_pos(view.cursor.pos));
    view_set_cursor(app, &view, seek_pos(view.cursor.pos), 1);
}

CUSTOM_COMMAND_SIG(cursor_mark_swap){
    View_Summary view = get_active_view(app, AccessProtected);
    
    int32_t cursor = view.cursor.pos;
    int32_t mark = view.mark.pos;
    
    view_set_cursor(app, &view, seek_pos(mark), true);
    view_set_mark(app, &view, seek_pos(cursor));
}

CUSTOM_COMMAND_SIG(delete_range){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Range range = get_range(&view);
    buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
}

//
// Basic Navigation Commands
//

CUSTOM_COMMAND_SIG(center_view){
    View_Summary view = get_active_view(app, AccessProtected);
    
    i32_Rect region = view.file_region;
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    float h = (float)(region.y1 - region.y0);
    float y = get_view_y(&view);
    y = y - h*.5f;
    scroll.target_y = (int32_t)(y + .5f);
    view_set_scroll(app, &view, scroll);
}

CUSTOM_COMMAND_SIG(left_adjust_view){
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

CUSTOM_COMMAND_SIG(click_set_cursor){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    
    Mouse_State mouse = get_mouse_state(app);
    float rx = 0, ry = 0;
    if (global_point_to_view_point(&view, mouse.x, mouse.y, &rx, &ry)){
        view_set_cursor(app, &view, seek_xy(rx, ry, 1, view.unwrapped_lines), 1);
    }
}

CUSTOM_COMMAND_SIG(click_set_mark){
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

CUSTOM_COMMAND_SIG(move_up){
    move_vertical(app, -1.f);
}

CUSTOM_COMMAND_SIG(move_down){
    move_vertical(app, 1.f);
}

CUSTOM_COMMAND_SIG(move_up_10){
    move_vertical(app, -10.f);
}

CUSTOM_COMMAND_SIG(move_down_10){
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

CUSTOM_COMMAND_SIG(page_up){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    float page_jump = get_page_jump(&view);
    move_vertical(app, -page_jump);
}

CUSTOM_COMMAND_SIG(page_down){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    float page_jump = get_page_jump(&view);
    move_vertical(app, page_jump);
}


CUSTOM_COMMAND_SIG(move_left){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    int32_t new_pos = view.cursor.character_pos - 1;
    view_set_cursor(app, &view, seek_character_pos(new_pos), 1);
}

CUSTOM_COMMAND_SIG(move_right){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    int32_t new_pos = view.cursor.character_pos + 1;
    view_set_cursor(app, &view, seek_character_pos(new_pos), 1);
}

//
// Long Seeks
//

CUSTOM_COMMAND_SIG(seek_whitespace_up){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = buffer_seek_whitespace_up(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = buffer_seek_whitespace_down(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_end_of_textual_line){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = seek_line_end(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = seek_line_beginning(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line){
    View_Summary view = get_active_view(app, AccessProtected);
    
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    
    view_set_cursor(app, &view, seek_xy(0, y, 1, view.unwrapped_lines), 1);
}

CUSTOM_COMMAND_SIG(seek_end_of_line){
    View_Summary view = get_active_view(app, AccessProtected);
    
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    
    view_set_cursor(app, &view, seek_xy(100000.f, y, 1, view.unwrapped_lines), 1);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up_end_line){
    exec_command(app, seek_whitespace_up);
    exec_command(app, seek_end_of_line);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down_end_line){
    exec_command(app, seek_whitespace_down);
    exec_command(app, seek_end_of_line);
}


//
// Fancy Editing
//

CUSTOM_COMMAND_SIG(to_uppercase){
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

CUSTOM_COMMAND_SIG(to_lowercase){
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

CUSTOM_COMMAND_SIG(clean_all_lines){
    // TODO(allen): This command always iterates accross the entire
    // buffer, so streaming it is actually the wrong call.  Rewrite this
    // to minimize calls to buffer_read_range.
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    int32_t line_count = buffer.line_count;
    int32_t edit_max = line_count;
    
    if (edit_max*sizeof(Buffer_Edit) < app->memory_size){
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
                        if (last_hard+1 < i){
                            edit->str_start = 0;
                            edit->len = 0;
                            edit->start = last_hard+1;
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
            
            if (last_hard+1 < buffer_size){
                edit->str_start = 0;
                edit->len = 0;
                edit->start = last_hard+1;
                edit->end = buffer_size;
                ++edit;
            }
            
            int32_t edit_count = (int32_t)(edit - edits);
            buffer_batch_edit(app, &buffer, 0, 0, edits, edit_count, BatchEdit_PreserveTokens);
        }
    }
}


//
// Scroll Bar Controlling
//

CUSTOM_COMMAND_SIG(show_scrollbar){
    View_Summary view = get_active_view(app, AccessProtected);
    view_set_setting(app, &view, ViewSetting_ShowScrollbar, true);
}

CUSTOM_COMMAND_SIG(hide_scrollbar){
    View_Summary view = get_active_view(app, AccessProtected);
    view_set_setting(app, &view, ViewSetting_ShowScrollbar, false);
}


//
// Basic Panel Management
//

CUSTOM_COMMAND_SIG(basic_change_active_panel){
    View_Summary view = get_active_view(app, AccessAll);
    get_view_next_looped(app, &view, AccessAll);
    set_active_view(app, &view);
}

CUSTOM_COMMAND_SIG(close_panel){
    View_Summary view = get_active_view(app, AccessAll);
    close_view(app, &view);
}

CUSTOM_COMMAND_SIG(open_panel_vsplit){
    View_Summary view = get_active_view(app, AccessAll);
    View_Summary new_view = open_view(app, &view, ViewSplit_Right);
    view_set_setting(app, &new_view, ViewSetting_ShowScrollbar, false);
}

CUSTOM_COMMAND_SIG(open_panel_hsplit){
    View_Summary view = get_active_view(app, AccessAll);
    View_Summary new_view = open_view(app, &view, ViewSplit_Bottom);
    view_set_setting(app, &new_view, ViewSetting_ShowScrollbar, false);
}


//
// Common Settings Commands
//

//toggle_fullscreen can be used as a command

CUSTOM_COMMAND_SIG(toggle_line_wrap){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    bool32 unwrapped = view.unwrapped_lines;
    buffer_set_setting(app, &buffer, BufferSetting_WrapLine, unwrapped);
}

CUSTOM_COMMAND_SIG(increase_line_wrap){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t wrap = 0;
    buffer_get_setting(app, &buffer, BufferSetting_WrapPosition, &wrap);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap + 10);
}

CUSTOM_COMMAND_SIG(decrease_line_wrap){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t wrap = 0;
    buffer_get_setting(app, &buffer, BufferSetting_WrapPosition, &wrap);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap - 10);
}

CUSTOM_COMMAND_SIG(toggle_virtual_whitespace){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t vwhite = 0;
    buffer_get_setting(app, &buffer, BufferSetting_VirtualWhitespace, &vwhite);
    buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, !vwhite);
}

CUSTOM_COMMAND_SIG(toggle_show_whitespace){
    View_Summary view = get_active_view(app, AccessProtected);
    view_set_setting(app, &view, ViewSetting_ShowWhitespace, !view.show_whitespace);
}

CUSTOM_COMMAND_SIG(eol_dosify){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    buffer_set_setting(app, &buffer, BufferSetting_Eol, 1);
}

CUSTOM_COMMAND_SIG(eol_nixify){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    buffer_set_setting(app, &buffer, BufferSetting_Eol, 0);
}

CUSTOM_COMMAND_SIG(exit_4coder){
    send_exit_signal(app);
}

//
// Interactive Commands
//

CUSTOM_COMMAND_SIG(goto_line){
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
isearch(Application_Links *app, int32_t start_reversed){
    uint32_t access = AccessProtected;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    if (!buffer.exists) return;
    
    Query_Bar bar = {0};
    if (start_query_bar(app, &bar, 0) == 0) return;
    
    int32_t reverse = start_reversed;
    int32_t pos = view.cursor.pos;
    int32_t start_pos = pos;
    int32_t first_pos = pos;
    Range match = make_range(pos, pos);
    
    char bar_string_space[256];
    bar.string = make_fixed_width_string(bar_string_space);
    
    String isearch_str = make_lit_string("I-Search: ");
    String rsearch_str = make_lit_string("Reverse-I-Search: ");
    
    User_Input in = {0};
    for (;;){
        view_set_highlight(app, &view, match.start, match.end, true);
        
        // NOTE(allen): Change the bar's prompt to match the current direction.
        if (reverse) bar.prompt = rsearch_str;
        else bar.prompt = isearch_str;
        
        in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        if (in.abort) break;
        
        // NOTE(allen): If we're getting mouse events here it's a 4coder bug, because we
        // only asked to intercept key events.
        Assert(in.type == UserInputKey);
        
        char character = to_writable_char(in.key.character);
        int32_t made_change = 0;
        if (in.key.keycode == '\n' || in.key.keycode == '\t'){
            break;
        }
        else if (character && key_is_unmodified(&in.key)){
            append_s_char(&bar.string, character);
            made_change = 1;
        }
        else if (in.key.keycode == key_back){
            if (bar.string.size > 0){
                --bar.string.size;
                made_change = 1;
            }
        }
        
        int32_t step_forward = 0;
        int32_t step_backward = 0;
        
        if ((in.command.command == search) ||
            in.key.keycode == key_page_down || in.key.keycode == key_down) step_forward = 1;
        if ((in.command.command == reverse_search) ||
            in.key.keycode == key_page_up || in.key.keycode == key_up) step_backward = 1;
        
        start_pos = pos;
        if (step_forward && reverse){
            start_pos = match.start + 1;
            pos = start_pos;
            reverse = 0;
            step_forward = 0;
        }
        if (step_backward && !reverse){
            start_pos = match.start - 1;
            pos = start_pos;
            reverse = 1;
            step_backward = 0;
        }
        
        if (in.key.keycode != key_back){
            int32_t new_pos;
            if (reverse){
                buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1, 0,
                                                        bar.string.str, bar.string.size, &new_pos);
                if (new_pos >= 0){
                    if (step_backward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1, 0,
                                                                bar.string.str, bar.string.size, &new_pos);
                        if (new_pos < 0) new_pos = start_pos;
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
            else{
                buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1, 0,
                                                       bar.string.str, bar.string.size, &new_pos);
                if (new_pos < buffer.size){
                    if (step_forward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1, 0,
                                                               bar.string.str, bar.string.size, &new_pos);
                        if (new_pos >= buffer.size) new_pos = start_pos;
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

CUSTOM_COMMAND_SIG(search){
    isearch(app, false);
}

CUSTOM_COMMAND_SIG(reverse_search){
    isearch(app, true);
}

CUSTOM_COMMAND_SIG(replace_in_range){
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
    
    String r = replace.string, w = with.string;
    
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

CUSTOM_COMMAND_SIG(query_replace){
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
    
    String r, w;
    r = replace.string;
    w = with.string;
    
    Query_Bar bar;
    Buffer_Summary buffer;
    View_Summary view;
    int32_t pos, new_pos;
    
    bar.prompt = make_lit_string("Replace? (y)es, (n)ext, (esc)\n");
    bar.string = null_string;
    
    start_query_bar(app, &bar, 0);
    
    uint32_t access = AccessOpen;
    view = get_active_view(app, access);
    buffer = get_buffer(app, view.buffer_id, access);
    
    pos = view.cursor.pos;
    buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    
    User_Input in = {0};
    while (new_pos < buffer.size){
        Range match = make_range(new_pos, new_pos + r.size);
        view_set_highlight(app, &view, match.min, match.max, 1);
        
        in = get_user_input(app, EventOnAnyKey, EventOnButton);
        if (in.abort || in.key.keycode == key_esc || !key_is_unmodified(&in.key))  break;
        
        if (in.key.character == 'y' ||
            in.key.character == 'Y' ||
            in.key.character == '\n' ||
            in.key.character == '\t'){
            buffer_replace_range(app, &buffer, match.min, match.max, w.str, w.size);
            pos = match.start + w.size;
        }
        else{
            pos = match.max;
        }
        
        buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    }
    
    view_set_highlight(app, &view, 0, 0, 0);
    if (in.abort) return;
    
    view_set_cursor(app, &view, seek_pos(pos), 1);
}


//
// cmdid wrappers
//

CUSTOM_COMMAND_SIG(undo){
    exec_command(app, cmdid_undo);
}

CUSTOM_COMMAND_SIG(redo){
    exec_command(app, cmdid_redo);
}

CUSTOM_COMMAND_SIG(interactive_new){
    exec_command(app, cmdid_interactive_new);
}

CUSTOM_COMMAND_SIG(interactive_open){
    exec_command(app, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(save_as){
    exec_command(app, cmdid_save_as);
}

CUSTOM_COMMAND_SIG(interactive_switch_buffer){
    exec_command(app, cmdid_interactive_switch_buffer);
}

CUSTOM_COMMAND_SIG(interactive_kill_buffer){
    exec_command(app, cmdid_interactive_kill_buffer);
}

CUSTOM_COMMAND_SIG(reopen){
    exec_command(app, cmdid_reopen);
}

CUSTOM_COMMAND_SIG(save){
    exec_command(app, cmdid_save);
}

CUSTOM_COMMAND_SIG(kill_buffer){
    exec_command(app, cmdid_kill_buffer);
}

CUSTOM_COMMAND_SIG(open_color_tweaker){
    exec_command(app, cmdid_open_color_tweaker);
}

CUSTOM_COMMAND_SIG(open_config){
    exec_command(app, cmdid_open_config);
}

CUSTOM_COMMAND_SIG(open_menu){
    exec_command(app, cmdid_open_menu);
}

CUSTOM_COMMAND_SIG(open_debug){
    exec_command(app, cmdid_open_debug);
}

#endif

// BOTTOM

