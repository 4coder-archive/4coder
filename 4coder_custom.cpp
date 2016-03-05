// Alternative customizations, set Custom_Current to select which to apply.
#define Custom_Default 0
#define Custom_HandmadeHero 1

#define Custom_Current Custom_HandmadeHero

#include "4coder_custom.h"

#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#define UseInterfacesThatArePhasingOut 0
#include "4coder_helper.h"

#ifndef literal
#define literal(s) (s), (sizeof(s)-1)
#endif

// NOTE(allen|a3.3): All of your custom ids should be enumerated
// as shown here, they may start at 0, and you can only have
// 2^24 of them so don't be wasteful!
enum My_Maps{
    my_code_map,
    my_html_map
};

HOOK_SIG(my_start){
    exec_command(app, cmdid_open_panel_vsplit);
    exec_command(app, cmdid_change_active_panel);
    
    app->change_theme(app, literal("4coder"));
    app->change_font(app, literal("liberation mono"));
    
    // Theme options:
    //  "4coder"
    //  "Handmade Hero"
    //  "Twilight"
    //  "Woverine"
    //  "stb"
    
    // Font options:
    //  "liberation sans"
    //  "liberation mono"
    //  "hack"
    //  "cutive mono"
    //  "inconsolata"
}

HOOK_SIG(my_file_settings){
    // NOTE(allen|a4): In hooks that want parameters, such as this file
    // created hook.  The file created hook is guaranteed to have only
    // and exactly one buffer parameter.  In normal command callbacks
    // there are no parameter buffers.
    Buffer_Summary buffer = app->get_parameter_buffer(app, 0);
    assert(buffer.exists);
    
    int treat_as_code = 0;

    if (buffer.file_name && buffer.size < (16 << 20)){
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match(ext, make_lit_string("cpp"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("h"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("c"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("hpp"))) treat_as_code = 1;
    }

    push_parameter(app, par_lex_as_cpp_file, treat_as_code);
    push_parameter(app, par_wrap_lines, !treat_as_code);
    push_parameter(app, par_key_mapid, (treat_as_code)?((int)my_code_map):((int)mapid_file));
    exec_command(app, cmdid_set_settings);
}

static void
write_string(Application_Links *app, String string){
    Buffer_Summary buffer = app->get_active_buffer(app);
    app->buffer_replace_range(app, &buffer, buffer.buffer_cursor_pos, buffer.buffer_cursor_pos, string.str, string.size);
}

CUSTOM_COMMAND_SIG(write_increment){
    write_string(app, make_lit_string("++"));
}

CUSTOM_COMMAND_SIG(write_decrement){
    write_string(app, make_lit_string("--"));
}

CUSTOM_COMMAND_SIG(write_allen_todo){
    write_string(app, make_lit_string("// TODO(allen): "));
}

CUSTOM_COMMAND_SIG(write_allen_note){
    write_string(app, make_lit_string("// NOTE(allen): "));
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

CUSTOM_COMMAND_SIG(paren_wrap){
    View_Summary view;
    Buffer_Summary buffer;

    char text1[] = "(";
    int size1 = sizeof(text1) - 1;

    char text2[] = ")";
    int size2 = sizeof(text2) - 1;

    Range range;
    int pos;

    view = app->get_active_view(app);
    buffer = app->get_active_buffer(app);

    range = get_range(&view);
    pos = range.max;
    app->buffer_replace_range(app, &buffer, pos, pos, text2, size2);

    pos = range.min;
    app->buffer_replace_range(app, &buffer, pos, pos, text1, size1);
}

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
    buffer = app->get_active_buffer(app);

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
    exec_command(app, cmdid_seek_alphanumeric_left);
    app->refresh_view(app, &view);
    pos1 = view.cursor.pos;
    
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

CUSTOM_COMMAND_SIG(switch_to_compilation){
    View_Summary view;
    Buffer_Summary buffer;
    
    char name[] = "*compilation*";
    int name_size = sizeof(name)-1;

    view = app->get_active_view(app);
    buffer = app->get_buffer_by_name(app, name, name_size);
    
    app->view_set_buffer(app, &view, buffer.buffer_id);
}

CUSTOM_COMMAND_SIG(move_up_10){
    View_Summary view;
    float x, y;

    view = app->get_active_view(app);
    x = view.preferred_x;
    
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    else{
        y = view.cursor.wrapped_y;
    }
    
    y -= 10*view.line_height;

    app->view_set_cursor(app, &view, seek_xy(x, y, 0, view.unwrapped_lines), 0);
}

CUSTOM_COMMAND_SIG(move_down_10){
    View_Summary view;
    float x, y;

    view = app->get_active_view(app);
    x = view.preferred_x;
    
    if (view.unwrapped_lines){
        y = view.cursor.wrapped_y;
    }
    else{
        y = view.cursor.wrapped_y;
    }
    
    y += 10*view.line_height;
    
    app->view_set_cursor(app, &view, seek_xy(x, y, 0, view.unwrapped_lines), 0);
}

CUSTOM_COMMAND_SIG(open_file_in_quotes){
    View_Summary view;
    Buffer_Summary buffer;
    char short_file_name[128];
    int pos, start, end, size;
    
    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);
    pos = view.cursor.pos;
    app->buffer_seek_delimiter(app, &buffer, pos, '"', 1, &end);
    app->buffer_seek_delimiter(app, &buffer, pos, '"', 0, &start);
    
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
    
    if (app->start_query_bar(app, &bar, 0) == 0) return;
    
    Range match;
    int reverse = start_reversed;
    int pos;
    
    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);
    
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
            --bar.string.size;
            made_change = 1;
        }
        
        int step_forward = 0;
        int step_backward = 0;
        
        if (CommandEqual(in.command, search) ||
                in.key.keycode == key_page_down || in.key.keycode == key_down) step_forward = 1;
        if (CommandEqual(in.command, reverse_search) ||
                in.key.keycode == key_page_down || in.key.keycode == key_up) step_backward = 1;
        
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
                app->buffer_seek_string(app, &buffer, start_pos - 1, bar.string.str, bar.string.size, 0, &new_pos);
                if (new_pos >= 0){
                    if (step_backward){
                        pos = new_pos;
                        start_pos = new_pos;
                        app->buffer_seek_string(app, &buffer, start_pos - 1, bar.string.str, bar.string.size, 0, &new_pos);
                        if (new_pos < 0) new_pos = start_pos;
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
            else{
                app->buffer_seek_string(app, &buffer, start_pos + 1, bar.string.str, bar.string.size, 1, &new_pos);
                if (new_pos < buffer.size){
                    if (step_forward){
                        pos = new_pos;
                        start_pos = new_pos;
                        app->buffer_seek_string(app, &buffer, start_pos + 1, bar.string.str, bar.string.size, 1, &new_pos);
                        if (new_pos >= buffer.size) new_pos = start_pos;
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
        }
        else{
            match.end = match.start + bar.string.size;
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

CUSTOM_COMMAND_SIG(rewrite_as_single_caps){
    View_Summary view;
    Buffer_Summary buffer;
    Range range;
    String string;
    int is_first, i;
    
    exec_command(app, cmdid_seek_token_left);
    view = app->get_active_view(app);
    range.min = view.cursor.pos;
    
    exec_command(app, cmdid_seek_token_right);
    app->refresh_view(app, &view);
    range.max = view.cursor.pos;
    
    string.str = (char*)app->memory;
    string.size = range.max - range.min;
    assert(string.size < app->memory_size);
    
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_read_range(app, &buffer, range.min, range.max, string.str);
    
    is_first = 1;
    for (i = 0; i < string.size; ++i){
        if (char_is_alpha_true(string.str[i])){
            if (is_first) is_first = 0;
            else string.str[i] = char_to_lower(string.str[i]);
        }
        else{
            is_first = 1;
        }
    }
    
    app->buffer_replace_range(app, &buffer, range.min, range.max, string.str, string.size);
}

// TODO(allen):
// get range by specific "word" type (for example "get token range")
// read range by specific "word" type
// Dream API for rewrite_as_single_caps:
#if 0
{
    rewrite = get_rewrite(app, ByToken);
    string = get_rewrite_string(app, &rewrite, app->memory, app->memory_size);
    
    is_first = 1;
    for (i = 0; i < string.size; ++i){
        if (char_is_alpha_true(string.str[i])){
            if (is_first) is_first = 0;
            else string.str[i] = char_to_lower(string.str[i]);
        }
        else{
            is_first = 1;
        }
    }
    
    do_rewrite(app, &rewrite, string);
}
#endif

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
    app->buffer_seek_string(app, &buffer, pos, r.str, r.size, 1, &new_pos);
    
    while (new_pos + r.size < range.end){
        app->buffer_replace_range(app, &buffer, new_pos, new_pos + r.size, w.str, w.size);
        range = get_range(&view);
        pos = new_pos + w.size;
        app->buffer_seek_string(app, &buffer, pos, r.str, r.size, 1, &new_pos);
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
    bar.string = {};
    
    app->start_query_bar(app, &bar, 0);
    
    view = app->get_active_view(app);
    buffer = app->get_buffer(app, view.buffer_id);

    pos = view.cursor.pos;
    app->buffer_seek_string(app, &buffer, pos, r.str, r.size, 1, &new_pos);
    
    User_Input in = {};
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
        
        app->buffer_seek_string(app, &buffer, pos, r.str, r.size, 1, &new_pos);
    }

    app->view_set_highlight(app, &view, 0, 0, 0);
    if (in.abort) return;

    app->view_set_cursor(app, &view, seek_pos(pos), 1);
}

CUSTOM_COMMAND_SIG(close_all_code){
    String extension;
    Buffer_Summary buffer;

    for (buffer = app->get_buffer_first(app);
        buffer.exists;
        app->get_buffer_next(app, &buffer)){
        
        extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match(extension, make_lit_string("cpp")) ||
                match(extension, make_lit_string("hpp")) ||
                match(extension, make_lit_string("c")) ||
                match(extension, make_lit_string("h"))){
            //
            push_parameter(app, par_buffer_id, buffer.buffer_id);
            exec_command(app, cmdid_kill_buffer);
        }
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
                push_parameter(app, par_do_in_background, 1);
                exec_command(app, cmdid_interactive_open);
            }
        }
    }
    
    app->free_file_list(app, list);
}

CUSTOM_COMMAND_SIG(execute_any_cli){
    Query_Bar bar_out, bar_cmd;
    String hot_directory;
    char space[1024], more_space[1024], even_more_space[1024];
    
    bar_out.prompt = make_lit_string("Output Buffer: ");
    bar_out.string = make_fixed_width_string(space);
    if (!query_user_string(app, &bar_out)) return;
    
    bar_cmd.prompt = make_lit_string("Command: ");
    bar_cmd.string = make_fixed_width_string(more_space);
    if (!query_user_string(app, &bar_cmd)) return;
    
    hot_directory = make_fixed_width_string(even_more_space);
    hot_directory.size = app->directory_get_hot(app, hot_directory.str, hot_directory.memory_size);
    
    push_parameter(app, par_flags, CLI_OverlapWithConflict);
    push_parameter(app, par_name, bar_out.string.str, bar_out.string.size);
    push_parameter(app, par_cli_path, hot_directory.str, hot_directory.size);
    push_parameter(app, par_cli_command, bar_cmd.string.str, bar_cmd.string.size);
    exec_command(app, cmdid_command_line);
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
    else{
        // TODO(allen): feedback message
    }
}

CUSTOM_COMMAND_SIG(open_in_other){
    exec_command(app, cmdid_change_active_panel);
    exec_command(app, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(open_my_files){
    // NOTE(allen|a3.1): EXAMPLE probably not useful in practice.
    // 
    // The command cmdid_interactive_open can now open
    // a file specified on the parameter stack.  If the file does not exist
    // cmdid_interactive_open behaves as usual.  If par_do_in_background
    // is set to true the command is prevented from changing the view under
    // any circumstance.
    push_parameter(app, par_name, literal("w:/4ed/data/test/basic.cpp"));
    exec_command(app, cmdid_interactive_open);

    exec_command(app, cmdid_change_active_panel);

    char my_file[256];
    int my_file_len;

    my_file_len = sizeof("w:/4ed/data/test/basic.txt") - 1;
    for (int i = 0; i < my_file_len; ++i){
        my_file[i] = ("w:/4ed/data/test/basic.txt")[i];
    }

    // NOTE(allen|a3.1): null terminators are not needed for strings.
    push_parameter(app, par_name, my_file, my_file_len);
    exec_command(app, cmdid_interactive_open);

    exec_command(app, cmdid_change_active_panel);
}

CUSTOM_COMMAND_SIG(build_at_launch_location){
    // NOTE(allen|a3.3):  EXAMPLE probably not all that useful in practice.
    // 
    // An example of calling build by setting all
    // parameters directly. This only works if build.bat can be called
    // from the directory the application is launched at.
    push_parameter(app, par_flags, CLI_OverlapWithConflict);
    push_parameter(app, par_name, literal("*compilation*"));
    push_parameter(app, par_cli_path, literal("."));
    push_parameter(app, par_cli_command, literal("build"));
    exec_command(app, cmdid_command_line);
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

    // TODO(allen): feedback message - couldn't find build.bat
}

CUSTOM_COMMAND_SIG(write_and_auto_tab){
    exec_command(app, cmdid_write_character);
    exec_command(app, cmdid_auto_tab_line_at_cursor);
}

// NOTE(allen|a4) See 4coder_styles.h for a list of available style tags.
// There are style tags corresponding to every color in the theme editor.
CUSTOM_COMMAND_SIG(improve_theme){
    Theme_Color colors[] = {
        {Stag_Bar, 0xFF0088},
        {Stag_Margin, 0x880088},
        {Stag_Margin_Hover, 0xAA0088},
        {Stag_Margin_Active, 0xDD0088},
        {Stag_Cursor, 0xFF0000},
    };
    
    int count = ArrayCount(colors);
    
    app->set_theme_colors(app, colors, count);
}

CUSTOM_COMMAND_SIG(ruin_theme){
    Theme_Color colors[] = {
        {Stag_Bar, 0x888888},
        {Stag_Margin, 0x181818},
        {Stag_Margin_Hover, 0x252525},
        {Stag_Margin_Active, 0x323232},
        {Stag_Cursor, 0x00EE00},
    };
    
    int count = ArrayCount(colors);
    
    app->set_theme_colors(app, colors, count);
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
Scroll_Velocity *scroll_velocity = scroll_velocity_;

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
    
    if (smooth_camera_step(target_y, scroll_y, &velocity->y, 40.f, 1.f/4.f)){
        result = 1;
    }
    if (smooth_camera_step(target_x, scroll_x, &velocity->x, 40.f, 1.f/4.f)){
        result = 1;
    }
    
    return(result);
}

#if Custom_Current == Custom_HandmadeHero
# include "4coder_handmade_hero.cpp"
#endif

extern "C" GET_BINDING_DATA(get_bindings){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
#if Custom_Current == Custom_HandmadeHero
    casey_get_bindings(context);
#else
    
    // NOTE(allen|a3.1): Right now hooks have no loyalties to maps, all hooks are
    // global and once set they always apply, regardless of what map is active.
    set_hook(context, hook_start, my_start);
    set_hook(context, hook_open_file, my_file_settings);
    set_scroll_rule(context, smooth_scroll_rule);
    
    begin_map(context, mapid_global);
    
    bind(context, 'p', MDFR_CTRL, cmdid_open_panel_vsplit);
    bind(context, '_', MDFR_CTRL, cmdid_open_panel_hsplit);
    bind(context, 'P', MDFR_CTRL, cmdid_close_panel);
    bind(context, 'n', MDFR_CTRL, cmdid_interactive_new);
    bind(context, 'o', MDFR_CTRL, cmdid_interactive_open);
    bind(context, ',', MDFR_CTRL, cmdid_change_active_panel);
    bind(context, 'k', MDFR_CTRL, cmdid_interactive_kill_buffer);
    bind(context, 'i', MDFR_CTRL, cmdid_interactive_switch_buffer);
    bind(context, 'c', MDFR_ALT, cmdid_open_color_tweaker);
    bind(context, 'o', MDFR_ALT, open_in_other);
    
    bind(context, 'm', MDFR_ALT, build_search);
    bind(context, ',', MDFR_ALT, switch_to_compilation);
    bind(context, 'x', MDFR_ALT, execute_arbitrary_command);
    bind(context, 'z', MDFR_ALT, execute_any_cli);
    
    // NOTE(allen): These callbacks may not actually be useful to you, but
    // go look at them and see what they do.
    bind(context, 'M', MDFR_ALT | MDFR_CTRL, open_my_files);
    bind(context, 'M', MDFR_ALT, build_at_launch_location);
    
    bind(context, '`', MDFR_ALT, improve_theme);
    bind(context, '~', MDFR_ALT, ruin_theme);

    end_map(context);


    begin_map(context, my_code_map);

    // NOTE(allen|a3.1): Set this map (my_code_map == mapid_user_custom) to
    // inherit from mapid_file.  When searching if a key is bound
    // in this map, if it is not found here it will then search mapid_file.
    //
    // If this is not set, it defaults to mapid_global.
    inherit_map(context, mapid_file);

    // NOTE(allen|a3.1): Children can override parent's bindings.
    bind(context, key_right, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_right);
    bind(context, key_left, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_left);

    // NOTE(allen|a3.2): Specific keys can override vanilla keys,
    // and write character writes whichever character corresponds
    // to the key that triggered the command.
    bind(context, '\n', MDFR_NONE, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, ')', MDFR_NONE, write_and_auto_tab);
    bind(context, ']', MDFR_NONE, write_and_auto_tab);
    bind(context, ';', MDFR_NONE, write_and_auto_tab);
    bind(context, '#', MDFR_NONE, write_and_auto_tab);
    
    bind(context, '\t', MDFR_NONE, cmdid_word_complete);
    bind(context, '\t', MDFR_CTRL, cmdid_auto_tab_range);
    bind(context, '\t', MDFR_SHIFT, cmdid_auto_tab_line_at_cursor);
    
    bind(context, '=', MDFR_CTRL, write_increment);
    bind(context, '-', MDFR_CTRL, write_decrement);
    bind(context, 't', MDFR_ALT, write_allen_todo);
    bind(context, 'n', MDFR_ALT, write_allen_note);
    bind(context, '[', MDFR_CTRL, open_long_braces);
    bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
    bind(context, '}', MDFR_CTRL, open_long_braces_break);
    bind(context, '9', MDFR_CTRL, paren_wrap);
    bind(context, 'i', MDFR_ALT, if0_off);
    
    end_map(context);
    
    
    begin_map(context, mapid_file);
    
    // NOTE(allen|a3.4.4): Binding this essentially binds
    // all key combos that would normally insert a character
    // into a buffer. If the code for the key is not an enum
    // value such as key_left or key_back then it is a vanilla key.
    // It is possible to override this binding for individual keys.
    bind_vanilla_keys(context, cmdid_write_character);
    
    bind(context, key_left, MDFR_NONE, cmdid_move_left);
    bind(context, key_right, MDFR_NONE, cmdid_move_right);
    bind(context, key_del, MDFR_NONE, cmdid_delete);
    bind(context, key_back, MDFR_NONE, cmdid_backspace);
    bind(context, key_up, MDFR_NONE, cmdid_move_up);
    bind(context, key_down, MDFR_NONE, cmdid_move_down);
    bind(context, key_end, MDFR_NONE, cmdid_seek_end_of_line);
    bind(context, key_home, MDFR_NONE, cmdid_seek_beginning_of_line);
    bind(context, key_page_up, MDFR_NONE, cmdid_page_up);
    bind(context, key_page_down, MDFR_NONE, cmdid_page_down);
    
    bind(context, key_right, MDFR_CTRL, cmdid_seek_whitespace_right);
    bind(context, key_left, MDFR_CTRL, cmdid_seek_whitespace_left);
    bind(context, key_up, MDFR_CTRL, cmdid_seek_whitespace_up);
    bind(context, key_down, MDFR_CTRL, cmdid_seek_whitespace_down);
    
    bind(context, key_up, MDFR_ALT, move_up_10);
    bind(context, key_down, MDFR_ALT, move_down_10);
    
    bind(context, key_back, MDFR_CTRL, backspace_word);
    bind(context, key_back, MDFR_ALT, snipe_token_or_word);
    
    bind(context, ' ', MDFR_CTRL, cmdid_set_mark);
    bind(context, 'm', MDFR_CTRL, cmdid_cursor_mark_swap);
    bind(context, 'c', MDFR_CTRL, cmdid_copy);
    bind(context, 'x', MDFR_CTRL, cmdid_cut);
    bind(context, 'v', MDFR_CTRL, cmdid_paste);
    bind(context, 'V', MDFR_CTRL, cmdid_paste_next);
    bind(context, 'Z', MDFR_CTRL, cmdid_timeline_scrub);
    bind(context, 'z', MDFR_CTRL, cmdid_undo);
    bind(context, 'y', MDFR_CTRL, cmdid_redo);
    bind(context, 'h', MDFR_CTRL, cmdid_history_backward);
    bind(context, 'H', MDFR_CTRL, cmdid_history_forward);
    bind(context, 'd', MDFR_CTRL, cmdid_delete_range);
    bind(context, 'l', MDFR_CTRL, cmdid_toggle_line_wrap);
    bind(context, 'L', MDFR_CTRL, cmdid_toggle_endline_mode);
    bind(context, 'u', MDFR_CTRL, cmdid_to_uppercase);
    bind(context, 'j', MDFR_CTRL, cmdid_to_lowercase);
    bind(context, '?', MDFR_CTRL, cmdid_toggle_show_whitespace);
    
    bind(context, '~', MDFR_CTRL, cmdid_clean_all_lines);
    bind(context, '1', MDFR_CTRL, cmdid_eol_dosify);
    bind(context, '!', MDFR_CTRL, cmdid_eol_nixify);
    
    bind(context, 'f', MDFR_CTRL, search);
    bind(context, 'r', MDFR_CTRL, reverse_search);
    bind(context, 'g', MDFR_CTRL, goto_line);
    bind(context, 'q', MDFR_CTRL, query_replace);
    bind(context, 'a', MDFR_CTRL, replace_in_range);
    bind(context, 's', MDFR_ALT, rewrite_as_single_caps);
    
    bind(context, 'K', MDFR_CTRL, cmdid_kill_buffer);
    bind(context, 'O', MDFR_CTRL, cmdid_reopen);
    bind(context, 'w', MDFR_CTRL, cmdid_interactive_save_as);
    bind(context, 's', MDFR_CTRL, cmdid_save);
    
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, ' ', MDFR_SHIFT, cmdid_write_character);
    
    end_map(context);
#endif

    end_bind_helper(context);
    
    return context->write_total;
}


