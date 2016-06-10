
#include "4coder_custom.h"

#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4coder_helper.h"

#include <assert.h>

inline float
get_view_y(View_Summary view){
    float y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    else{
        y = view.cursor.wrapped_y;
    }
    return(y);
}

inline float
get_view_x(View_Summary view){
    float x;
    if (view.unwrapped_lines){
        x = view.cursor.unwrapped_x;
    }
    else{
        x = view.cursor.wrapped_x;
    }
    return(x);
}

//
// Fundamental Editing
//

CUSTOM_COMMAND_SIG(write_character){
    View_Summary view = app->get_active_view(app);
    
    User_Input in = app->get_command_input(app);
    char character = 0;
    
    if (in.type == UserInputKey){
        character = in.key.character;
    }
    
    if (character != 0){
        Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
        int pos = view.cursor.pos;
        int next_pos = pos + 1;
        app->buffer_replace_range(app, &buffer,
                                  pos, pos, &character, 1);
        app->view_set_cursor(app, &view, seek_pos(next_pos), true);
    }
}

CUSTOM_COMMAND_SIG(delete_char){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    int pos = view.cursor.pos;
    if (0 < buffer.size && pos < buffer.size){
        app->buffer_replace_range(app, &buffer,
                                  pos, pos+1, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(backspace_char){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    int pos = view.cursor.pos;
    if (0 < pos && pos <= buffer.size){
        app->buffer_replace_range(app, &buffer,
                                  pos-1, pos, 0, 0);
        
        app->view_set_cursor(app, &view, seek_pos(pos-1), true);
    }
}

CUSTOM_COMMAND_SIG(set_mark){
    View_Summary view = app->get_active_view(app);
    
    app->view_set_mark(app, &view, seek_pos(view.cursor.pos));
    // TODO(allen): Just expose the preferred_x seperately
    app->view_set_cursor(app, &view, seek_pos(view.cursor.pos), true);
}

CUSTOM_COMMAND_SIG(cursor_mark_swap){
    View_Summary view = app->get_active_view(app);
    
    int cursor = view.cursor.pos;
    int mark = view.mark.pos;
    
    app->view_set_cursor(app, &view, seek_pos(mark), true);
    app->view_set_mark(app, &view, seek_pos(cursor));
}

CUSTOM_COMMAND_SIG(delete_range){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    Range range = get_range(&view);
    
    app->buffer_replace_range(app, &buffer,
                              range.min, range.max,
                              0, 0);
}

//
// Basic Navigation
//

int
get_relative_xy(View_Summary *view, int x, int y, float *x_out, float *y_out){
    int result = false;
    
    i32_Rect region = view->file_region;
    
    int max_x = (region.x1 - region.x0);
    int max_y = (region.y1 - region.y0);
    GUI_Scroll_Vars scroll_vars = view->scroll_vars;
    
    int rx = x - region.x0;
    int ry = y - region.y0;
    
    if (ry >= 0){
        if (rx >= 0 && rx < max_x && ry >= 0 && ry < max_y){
            result = 1;
        }
    }
    
    *x_out = (float)rx + scroll_vars.scroll_x;
    *y_out = (float)ry + scroll_vars.scroll_y;
    
    return(result);
}

CUSTOM_COMMAND_SIG(click_set_cursor){
    View_Summary view = app->get_active_view(app);
    
    // TODO(allen): Need a better system for
    // weeding out views in a hidden state.
    if (view.locked_buffer_id != 0){
        Mouse_State mouse = app->get_mouse_state(app);
        float rx = 0, ry = 0;
        if (get_relative_xy(&view, mouse.x, mouse.y, &rx, &ry)){
            app->view_set_cursor(app, &view,
                                 seek_xy(rx, ry, true,
                                         view.unwrapped_lines),
                                 true);
        }
    }
}

CUSTOM_COMMAND_SIG(click_set_mark){
    View_Summary view = app->get_active_view(app);
    
    if (view.locked_buffer_id != 0){
        Mouse_State mouse = app->get_mouse_state(app);
        float rx = 0, ry = 0;
        if (get_relative_xy(&view, mouse.x, mouse.y, &rx, &ry)){
            app->view_set_mark(app, &view,
                               seek_xy(rx, ry, true,
                                       view.unwrapped_lines)
                               );
        }
    }
}

inline void
move_vertical(Application_Links *app, float line_multiplier){
    View_Summary view = app->get_active_view(app);
    
    float new_y = get_view_y(view) + line_multiplier*view.line_height;
    float x = view.preferred_x;
    
    app->view_set_cursor(app, &view,
                         seek_xy(x, new_y, false, view.unwrapped_lines),
                         false);
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


CUSTOM_COMMAND_SIG(move_left){
    View_Summary view = app->get_active_view(app);
    int new_pos = view.cursor.pos - 1;
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

CUSTOM_COMMAND_SIG(move_right){
    View_Summary view = app->get_active_view(app);
    int new_pos = view.cursor.pos + 1;
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

//
// Various Forms of Seek
//

static int
buffer_seek_whitespace_up(Application_Links *app, Buffer_Summary *buffer, int pos){
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int no_hard;
    int still_looping;
    char at_pos;
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        // Step 1: Find the first non-whitespace character
        // behind the current position.
        still_looping = true;
        do{
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (!char_is_whitespace(at_pos)){
                    goto double_break_1;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        } while(still_looping);
        double_break_1:;
        
        // Step 2: Continue scanning backward, at each '\n'
        // mark the beginning of another line by setting
        // no_hard to true, set it back to false if a
        // non-whitespace character is discovered before
        // the next '\n'
        no_hard = false;
        while (still_looping){
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                    }
                }
                else if (!char_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (pos != 0){
            ++pos;
        }
    }

    return(pos);
}

static int
buffer_seek_whitespace_down(Application_Links *app, Buffer_Summary *buffer, int pos){
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int no_hard;
    int prev_endline;
    int still_looping;
    char at_pos;
    
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        // Step 1: Find the first non-whitespace character
        // ahead of the current position.
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (!char_is_whitespace(at_pos)){
                    goto double_break_1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        } while(still_looping);
        double_break_1:;
        
        // Step 2: Continue scanning forward, at each '\n'
        // mark it as the beginning of a new line by updating
        // the prev_endline value.  If another '\n' is found
        // with non-whitespace then the previous line was
        // all whitespace.
        no_hard = false;
        prev_endline = -1;
        while(still_looping){
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                        prev_endline = pos;
                    }
                }
                else if (!char_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (prev_endline == -1 || prev_endline+1 >= buffer->size){
            pos = buffer->size;
        }
        else{
            pos = prev_endline+1;
        }
    }
    
    return(pos);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.locked_buffer_id);
    
    int new_pos = buffer_seek_whitespace_up(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.locked_buffer_id);
    
    int new_pos = buffer_seek_whitespace_down(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

static int
seek_line_end(Application_Links *app, Buffer_Summary *buffer, int pos){
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int still_looping;
    char at_pos;
    
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        still_looping = 1;
        do{
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        
        if (pos > buffer->size){
            pos = buffer->size;
        }
    }
    
    return(pos);
}

static int
seek_line_beginning(Application_Links *app, Buffer_Summary *buffer, int pos){
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int still_looping;
    char at_pos;
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        still_looping = 1;
        do{
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        
        if (pos != 0){
            ++pos;
        }
        if (pos < 0){
            pos = 0;
        }
    }
    
    return(pos);
}

CUSTOM_COMMAND_SIG(seek_end_of_line){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.locked_buffer_id);
    
    int new_pos = seek_line_end(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.locked_buffer_id);
    
    int new_pos = seek_line_beginning(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view, seek_pos(new_pos), true);
}

static void
basic_seek(Application_Links *app, Command_ID seek_type, unsigned int flags){
    push_parameter(app, par_flags, flags);
    exec_command(app, seek_type);
}

#define SEEK_COMMAND(n, dir, flags)\
CUSTOM_COMMAND_SIG(seek_##n##_##dir){ basic_seek(app, cmdid_seek_##dir, flags); }

SEEK_COMMAND(whitespace, right, BoundryWhitespace)
SEEK_COMMAND(whitespace, left, BoundryWhitespace)
SEEK_COMMAND(token, right, BoundryToken)
SEEK_COMMAND(token, left, BoundryToken)
SEEK_COMMAND(white_or_token, right, BoundryToken | BoundryWhitespace)
SEEK_COMMAND(white_or_token, left, BoundryToken | BoundryWhitespace)
SEEK_COMMAND(alphanumeric, right, BoundryAlphanumeric)
SEEK_COMMAND(alphanumeric, left, BoundryAlphanumeric)
SEEK_COMMAND(alphanumeric_or_camel, right, BoundryAlphanumeric | BoundryCamelCase)
SEEK_COMMAND(alphanumeric_or_camel, left, BoundryAlphanumeric | BoundryCamelCase)


//
// Special string writing commands
//

static void
write_string(Application_Links *app, String string){
    Buffer_Summary buffer = get_active_buffer(app);
    app->buffer_replace_range(app, &buffer,
                              buffer.buffer_cursor_pos, buffer.buffer_cursor_pos,
                              string.str, string.size);
}

CUSTOM_COMMAND_SIG(write_increment){
    write_string(app, make_lit_string("++"));
}

static void
long_braces(Application_Links *app, char *text, int size){
    View_Summary view;
    Buffer_Summary buffer;
    int pos;

    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);

    pos = view.cursor.pos;
    app->buffer_replace_range(app, &buffer, pos, pos, text, size);
    app->view_set_cursor(app, &view, seek_pos(pos + 2), 1);

    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos + size);
    push_parameter(app, par_clear_blank_lines, 0);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(open_long_braces){
    char text[] = "{\n\n}";
    int size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_semicolon){
    char text[] = "{\n\n};";
    int size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_break){
    char text[] = "{\n\n}break;";
    int size = sizeof(text) - 1;
    long_braces(app, text, size);
}

// TODO(allen): Have this thing check if it is on
// a blank line and insert newlines as needed.
CUSTOM_COMMAND_SIG(if0_off){
    View_Summary view;
    Buffer_Summary buffer;

    char text1[] = "\n#if 0";
    int size1 = sizeof(text1) - 1;

    char text2[] = "#endif\n";
    int size2 = sizeof(text2) - 1;

    Range range;
    int pos;

    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);
    
    range = get_range(&view);
    pos = range.min;
    
    app->buffer_replace_range(app, &buffer, pos, pos, text1, size1);
    
    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos);
    exec_command(app, cmdid_auto_tab_range);
    
    app->refresh_view(app, &view);
    range = get_range(&view);
    pos = range.max;
    
    app->buffer_replace_range(app, &buffer, pos, pos, text2, size2);
    
    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(backspace_word){
    View_Summary view;
    Buffer_Summary buffer;
    int pos2, pos1;
    
    view = app->get_active_view(app);
    
    pos2 = view.cursor.pos;
    exec_command(app, seek_alphanumeric_left);
    app->refresh_view(app, &view);
    pos1 = view.cursor.pos;
    
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
}

CUSTOM_COMMAND_SIG(delete_word){
    View_Summary view;
    Buffer_Summary buffer;
    int pos2, pos1;
    
    view = app->get_active_view(app);
    
    pos1 = view.cursor.pos;
    exec_command(app, seek_alphanumeric_right);
    app->refresh_view(app, &view);
    pos2 = view.cursor.pos;
    
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
}

CUSTOM_COMMAND_SIG(snipe_token_or_word){
    View_Summary view;
    Buffer_Summary buffer;
    int pos1, pos2;
    
    view = app->get_active_view(app);
    
    push_parameter(app, par_flags, BoundryToken | BoundryWhitespace);
    exec_command(app, cmdid_seek_left);
    app->refresh_view(app, &view);
    pos1 = view.cursor.pos;
    
    push_parameter(app, par_flags, BoundryToken | BoundryWhitespace);
    exec_command(app, cmdid_seek_right);
    app->refresh_view(app, &view);
    pos2 = view.cursor.pos;
    
    Range range = make_range(pos1, pos2);
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
}

CUSTOM_COMMAND_SIG(open_file_in_quotes){
    View_Summary view;
    Buffer_Summary buffer;
    char short_file_name[128];
    int pos, start, end, size;
    
    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);
    pos = view.cursor.pos;
    buffer_seek_delimiter_forward(app, &buffer, pos, '"', &end);
    buffer_seek_delimiter_backward(app, &buffer, pos, '"', &start);
    
    ++start;
    size = end - start;
    
    // NOTE(allen): This check is necessary because app->buffer_read_range
    // requiers that the output buffer you provide is at least (end - start) bytes long.
    if (size < sizeof(short_file_name)){
        char file_name_[256];
        String file_name = make_fixed_width_string(file_name_);
        
        app->buffer_read_range(app, &buffer, start, end, short_file_name);
        
        copy(&file_name, make_string(buffer.file_name, buffer.file_name_len));
        remove_last_folder(&file_name);
        append(&file_name, make_string(short_file_name, size));
        
        exec_command(app, cmdid_change_active_panel);
        push_parameter(app, par_name, expand_str(file_name));
        exec_command(app, cmdid_interactive_open);
    }
}

CUSTOM_COMMAND_SIG(save_as){
    push_parameter(app, par_save_update_name, 1);
    exec_command(app, cmdid_save);
}

CUSTOM_COMMAND_SIG(goto_line){
    int line_number;
    Query_Bar bar;
    char string_space[256];

    bar.prompt = make_lit_string("Goto Line: ");
    bar.string = make_fixed_width_string(string_space);

    if (query_user_number(app, &bar)){
        line_number = str_to_int(bar.string);
        active_view_to_line(app, line_number);
    }
}

CUSTOM_COMMAND_SIG(search);
CUSTOM_COMMAND_SIG(reverse_search);

static void
isearch(Application_Links *app, int start_reversed){
    View_Summary view;
    Buffer_Summary buffer;
    User_Input in;
    Query_Bar bar;
    
    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.locked_buffer_id);
    
    if (!buffer.exists) return;
    
    if (app->start_query_bar(app, &bar, 0) == 0) return;
    
    Range match;
    int reverse = start_reversed;
    int pos;
    
    pos = view.cursor.pos;
    match = make_range(pos, pos);
    
    char bar_string_space[256];
    bar.string = make_fixed_width_string(bar_string_space);
    
    String isearch = make_lit_string("I-Search: ");
    String rsearch = make_lit_string("Reverse-I-Search: ");
    
    while (1){
        // NOTE(allen): Change the bar's prompt to match the current direction.
        if (reverse) bar.prompt = rsearch;
        else bar.prompt = isearch;
        
        in = app->get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        if (in.abort) break;
        
        // NOTE(allen): If we're getting mouse events here it's a 4coder bug, because we
        // only asked to intercept key events.
        assert(in.type == UserInputKey);
        
        int made_change = 0;
        if (in.key.keycode == '\n' || in.key.keycode == '\t'){
            break;
        }
        else if (in.key.character && key_is_unmodified(&in.key)){
            append(&bar.string, in.key.character);
            made_change = 1;
        }
        else if (in.key.keycode == key_back){
            if (bar.string.size > 0){
                --bar.string.size;
                made_change = 1;
            }
        }
        
        int step_forward = 0;
        int step_backward = 0;
        
        if (CommandEqual(in.command, search) ||
            in.key.keycode == key_page_down || in.key.keycode == key_down) step_forward = 1;
        if (CommandEqual(in.command, reverse_search) ||
            in.key.keycode == key_page_up || in.key.keycode == key_up) step_backward = 1;
        
        int start_pos = pos;
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
            int new_pos;
            if (reverse){
                buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1,
                                                        bar.string.str, bar.string.size, &new_pos);
                if (new_pos >= 0){
                    if (step_backward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1,
                                                                bar.string.str, bar.string.size, &new_pos);
                        if (new_pos < 0) new_pos = start_pos;
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
            else{
                buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1,
                                                       bar.string.str, bar.string.size, &new_pos);
                if (new_pos < buffer.size){
                    if (step_forward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1,
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
        
        app->view_set_highlight(app, &view, match.start, match.end, 1);
    }
    app->view_set_highlight(app, &view, 0, 0, 0);
    if (in.abort) return;
    
    app->view_set_cursor(app, &view, seek_pos(match.min), 1);
}

CUSTOM_COMMAND_SIG(search){
    isearch(app, 0);
}

CUSTOM_COMMAND_SIG(reverse_search){
    isearch(app, 1);
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

    String r, w;
    r = replace.string;
    w = with.string;

    Buffer_Summary buffer;
    View_Summary view;

    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);

    Range range = get_range(&view);

    int pos, new_pos;
    pos = range.min;
    buffer_seek_string_forward(app, &buffer, pos, r.str, r.size, &new_pos);

    while (new_pos + r.size <= range.end){
        app->buffer_replace_range(app, &buffer, new_pos, new_pos + r.size, w.str, w.size);
        app->refresh_view(app, &view);
        range = get_range(&view);
        pos = new_pos + w.size;
        buffer_seek_string_forward(app, &buffer, pos, r.str, r.size, &new_pos);
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
    int pos, new_pos;

    bar.prompt = make_lit_string("Replace? (y)es, (n)ext, (esc)\n");
    bar.string = empty_string();

    app->start_query_bar(app, &bar, 0);

    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);

    pos = view.cursor.pos;
    buffer_seek_string_forward(app, &buffer, pos, r.str, r.size, &new_pos);

    User_Input in = {0};
    while (new_pos < buffer.size){
        Range match = make_range(new_pos, new_pos + r.size);
        app->view_set_highlight(app, &view, match.min, match.max, 1);

        in = app->get_user_input(app, EventOnAnyKey, EventOnButton);
        if (in.abort || in.key.keycode == key_esc || !key_is_unmodified(&in.key))  break;

        if (in.key.character == 'y' || in.key.character == 'Y' || in.key.character == '\n' || in.key.character == '\t'){
            app->buffer_replace_range(app, &buffer, match.min, match.max, w.str, w.size);
            pos = match.start + w.size;
        }
        else{
            pos = match.max;
        }

        buffer_seek_string_forward(app, &buffer, pos, r.str, r.size, &new_pos);
    }

    app->view_set_highlight(app, &view, 0, 0, 0);
    if (in.abort) return;

    app->view_set_cursor(app, &view, seek_pos(pos), 1);
}

CUSTOM_COMMAND_SIG(close_all_code){
    String extension;
    Buffer_Summary buffer;
    
    // TODO(allen): Get better memory constructs to the custom layer
    // so that it doesn't have to rely on arbitrary limits like this one.
    int buffers_to_close[2048];
    int buffers_to_close_count = 0;
    
    for (buffer = app->get_buffer_first(app);
         buffer.exists;
         app->get_buffer_next(app, &buffer)){
        
        extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match(extension, make_lit_string("cpp")) ||
            match(extension, make_lit_string("hpp")) ||
            match(extension, make_lit_string("c")) ||
            match(extension, make_lit_string("h"))){
            
            buffers_to_close[buffers_to_close_count++] = buffer.buffer_id;
        }
    }
    
    for (int i = 0; i < buffers_to_close_count; ++i){
        push_parameter(app, par_buffer_id, buffers_to_close[i]);
        exec_command(app, cmdid_kill_buffer);
    }
}

CUSTOM_COMMAND_SIG(open_all_code){
    // NOTE(allen|a3.4.4): This method of getting the hot directory works
    // because this custom.cpp gives no special meaning to app->memory
    // and doesn't set up a persistent allocation system within app->memory.
    // push_directory isn't a very good option since it's tied to the parameter
    // stack, so I am phasing that idea out now.
    String dir = make_string(app->memory, 0, app->memory_size);
    dir.size = app->directory_get_hot(app, dir.str, dir.memory_size);
    int dir_size = dir.size;

    // NOTE(allen|a3.4.4): Here we get the list of files in this directory.
    // Notice that we free_file_list at the end.
    File_List list = app->get_file_list(app, dir.str, dir.size);

    for (int i = 0; i < list.count; ++i){
        File_Info *info = list.infos + i;
        if (!info->folder){
            String extension = file_extension(info->filename);
            if (match(extension, make_lit_string("cpp")) ||
                    match(extension, make_lit_string("hpp")) ||
                    match(extension, make_lit_string("c")) ||
                    match(extension, make_lit_string("h"))){
                // NOTE(allen): There's no way in the 4coder API to use relative
                // paths at the moment, so everything should be full paths.  Which is
                // managable.  Here simply set the dir string size back to where it
                // was originally, so that new appends overwrite old ones.
                dir.size = dir_size;
                append(&dir, info->filename);
                push_parameter(app, par_name, dir.str, dir.size);
                //push_parameter(app, par_do_in_background, 1);
                exec_command(app, cmdid_interactive_open);
            }
        }
    }

    app->free_file_list(app, list);
}

char out_buffer_space[1024], command_space[1024], hot_directory_space[1024];

CUSTOM_COMMAND_SIG(execute_any_cli){
    Query_Bar bar_out, bar_cmd;
    String hot_directory;

    bar_out.prompt = make_lit_string("Output Buffer: ");
    bar_out.string = make_fixed_width_string(out_buffer_space);
    if (!query_user_string(app, &bar_out)) return;

    bar_cmd.prompt = make_lit_string("Command: ");
    bar_cmd.string = make_fixed_width_string(command_space);
    if (!query_user_string(app, &bar_cmd)) return;

    hot_directory = make_fixed_width_string(hot_directory_space);
    hot_directory.size = app->directory_get_hot(app, hot_directory.str, hot_directory.memory_size);

    push_parameter(app, par_flags, CLI_OverlapWithConflict);
    push_parameter(app, par_name, bar_out.string.str, bar_out.string.size);
    push_parameter(app, par_cli_path, hot_directory.str, hot_directory.size);
    push_parameter(app, par_cli_command, bar_cmd.string.str, bar_cmd.string.size);
    exec_command(app, cmdid_command_line);
    
    terminate_with_null(&bar_out.string);
    terminate_with_null(&bar_cmd.string);
    terminate_with_null(&hot_directory);
}

CUSTOM_COMMAND_SIG(execute_previous_cli){
    String out_buffer, cmd, hot_directory;
    
    out_buffer = make_string_slowly(out_buffer_space);
    cmd = make_string_slowly(command_space);
    hot_directory = make_string_slowly(hot_directory_space);
    
    if (out_buffer.size > 0 && cmd.size > 0 && hot_directory.size > 0){
        push_parameter(app, par_flags, CLI_OverlapWithConflict);
        push_parameter(app, par_name, out_buffer.str, out_buffer.size);
        push_parameter(app, par_cli_path, hot_directory.str, hot_directory.size);
        push_parameter(app, par_cli_command, cmd.str, cmd.size);
        exec_command(app, cmdid_command_line);
    }
}

CUSTOM_COMMAND_SIG(execute_arbitrary_command){
    // NOTE(allen): This isn't a super powerful version of this command, I will expand
    // upon it so that it has all the cmdid_* commands by default.  However, with this
    // as an example you have everything you need to make it work already. You could
    // even use app->memory to create a hash table in the start hook.
    Query_Bar bar;
    char space[1024];
    bar.prompt = make_lit_string("Command: ");
    bar.string = make_fixed_width_string(space);

    if (!query_user_string(app, &bar)) return;

    // NOTE(allen): Here I chose to end this query bar because when I call another
    // command it might ALSO have query bars and I don't want this one hanging
    // around at that point.  Since the bar exists on my stack the result of the query
    // is still available in bar.string though.
    app->end_query_bar(app, &bar, 0);

    if (match(bar.string, make_lit_string("open all code"))){
        exec_command(app, open_all_code);
    }
    else if(match(bar.string, make_lit_string("close all code"))){
        exec_command(app, close_all_code);
    }
    else if (match(bar.string, make_lit_string("open menu"))){
        exec_command(app, cmdid_open_menu);
    }
    else if (match(bar.string, make_lit_string("dos lines"))){
        exec_command(app, cmdid_eol_dosify);
    }
    else if (match(bar.string, make_lit_string("nix lines"))){
        exec_command(app, cmdid_eol_nixify);
    }
    else{
        app->print_message(app, literal("unrecognized command\n"));
    }
}

CUSTOM_COMMAND_SIG(open_in_other){
    exec_command(app, cmdid_change_active_panel);
    exec_command(app, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(build_search){
    // NOTE(allen|a3.3): An example of traversing the filesystem through parent
    // directories looking for a file, in this case a batch file to execute.
    //
    //
    // Step 1: Grab all of the user memory (or, you know, less if you've got better
    //     thing to do with some of it).  Make a string and store the hot directory in it.
    //
    // Step 2: app->file_exists queries the file system to see if "<somedir>/build.bat" exists.
    // If it does exist several parameters are pushed and cmdid_command_line is executed:
    //   - par_flags: flags for specifiying behaviors
    //        CLI_OverlapWithConflict - (on by default) if another CLI is still using the output buffer
    //        that process is detached from the buffer and this process executes outputing to the buffer
    //        CLI_AlwaysBindToView - if set, the current view always switches to the output buffer
    //        even if the output buffer is open in another view
    //
    //   - par_name: the name of the buffer to fill with the output from the process
    //   - par_buffer_id: the buffer_id of the buffer to to fill with output
    //     If both are set buffer_id is used and the name is ignored.
    //     If neither is set the command runs without storing output anywhere.
    //
    //   - par_cli_path: sets the path from which the command is executed
    //     If this parameter is unset the command runs from the hot directory.
    //
    //   - par_cli_command: sets the actual command to be executed, this can be almost any
    //     command that you could execute through a command line interface.
    //     If this parameter is unset the command get's it's command from the range between
    //     the mark and cursor.
    // 
    // Step 3: If the batch file did not exist change the dir string to the parent directory using
    // app->directory_cd. The cd function can also be used to navigate to subdirectories.
    // It returns true if it can actually move in the specified direction, and false otherwise.
    // 
    // This doesn't actually change the hot directory of 4coder, it's only effect is to
    // modify the string you passed in to reflect the change in directory if that change was possible.

    int keep_going = 1;
    int old_size;
    String dir = make_string(app->memory, 0, app->memory_size);
    dir.size = app->directory_get_hot(app, dir.str, dir.memory_size);

    while (keep_going){
        old_size = dir.size;
        append(&dir, "build.bat");

        if (app->file_exists(app, dir.str, dir.size)){
            dir.size = old_size;

            push_parameter(app, par_flags, CLI_OverlapWithConflict);
            push_parameter(app, par_name, literal("*compilation*"));
            push_parameter(app, par_cli_path, dir.str, dir.size);

            if (append(&dir, "build")){
                push_parameter(app, par_cli_command, dir.str, dir.size);
                exec_command(app, cmdid_command_line);
            }
            else{
                app->clear_parameters(app);
            }

            return;
        }
        dir.size = old_size;

        if (app->directory_cd(app, dir.str, &dir.size, dir.memory_size, literal("..")) == 0){
            keep_going = 0;
        }
    }

    app->print_message(app, literal("couldn't find build.bat\n"));
}

CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor){
    View_Summary view = app->get_active_view(app);
    push_parameter(app, par_range_start, view.cursor.pos);
    push_parameter(app, par_range_end, view.cursor.pos);
    push_parameter(app, par_clear_blank_lines, 0);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(auto_tab_whole_file){
    Buffer_Summary buffer = get_active_buffer(app);
    push_parameter(app, par_range_start, 0);
    push_parameter(app, par_range_end, buffer.size);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(write_and_auto_tab){
    exec_command(app, write_character);
    exec_command(app, auto_tab_line_at_cursor);
}

// NOTE(allen|a4): scroll rule information
//
// The parameters:
// target_x, target_y
//  This is where the view would like to be for the purpose of
// following the cursor, doing mouse wheel work, etc.
//
// scroll_x, scroll_y
//  These are pointers to where the scrolling actually is. If you bind
// the scroll rule it is you have to update these in some way to move
// the actual location of the scrolling.
//
// view_id
//  This corresponds to which view is computing it's new scrolling position.
// This id DOES correspond to the views that View_Summary contains.
// This will always be between 1 and 16 (0 is a null id).
// See below for an example of having state that carries across scroll udpates.
//
// is_new_target
//  If the target of the view is different from the last target in either x or y
// this is true, otherwise it is false.
//
// The return:
//  Should be true if and only if scroll_x or scroll_y are changed.
//
// Don't try to use the app pointer in a scroll rule, you're asking for trouble.
//
// If you don't bind scroll_rule, nothing bad will happen, yo will get default
// 4coder scrolling behavior.
//

struct Scroll_Velocity{
    float x, y;
};

Scroll_Velocity scroll_velocity_[16] = {0};
Scroll_Velocity *scroll_velocity = scroll_velocity_ - 1;

static int
smooth_camera_step(float target, float *current, float *vel, float S, float T){
    int result = 0;
    float curr = *current;
    float v = *vel;
    if (curr != target){
        if (curr > target - .1f && curr < target + .1f){
            curr = target;
            v = 1.f;
        }
        else{
            float L = curr + T*(target - curr);

            int sign = (target > curr) - (target < curr);
            float V = curr + sign*v;

            if (sign > 0) curr = (L<V)?(L):(V);
            else curr = (L>V)?(L):(V);

            if (curr == V){
                v *= S;
            }
        }

        *current = curr;
        *vel = v;
        result = 1;
    }
    return result;
}

SCROLL_RULE_SIG(smooth_scroll_rule){
    Scroll_Velocity *velocity = scroll_velocity + view_id;
    int result = 0;
    if (velocity->x == 0.f){
        velocity->x = 1.f;
        velocity->y = 1.f;
    }

    if (smooth_camera_step(target_y, scroll_y, &velocity->y, 80.f, 1.f/2.f)){
        result = 1;
    }
    if (smooth_camera_step(target_x, scroll_x, &velocity->x, 80.f, 1.f/2.f)){
        result = 1;
    }

    return(result);
}


