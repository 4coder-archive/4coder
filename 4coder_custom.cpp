/*
 * Example use of customization API
 */

// NOTE(allen): See exec_command and surrounding code in 4coder_helper.h
// to decide whether you want macro translations, without them you will
// have to manipulate the command and parameter stack through
// "app->" which may be more or less clear depending on your use.

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
}

HOOK_SIG(my_file_settings){
     Buffer_Summary buffer = app->get_active_buffer(app);
     
     // NOTE(allen|a3.4.2): Whenever you ask for a buffer, you can check that
     // the exists field is set to true.  Reasons why the buffer might not exist:
     //   -The active panel does not contain a buffer and get_active_buffer was used
     //   -The index provided to get_buffer was out of range [0,max) or that index is associated to a dummy buffer
     //   -The name provided to get_buffer_by_name did not match any of the existing buffers
     if (buffer.exists){
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
}

CUSTOM_COMMAND_SIG(write_increment){
    char text[] = "++";
    int size = sizeof(text) - 1;
    Buffer_Summary buffer = app->get_active_buffer(app);
    app->buffer_replace_range(app, &buffer, buffer.buffer_cursor_pos, buffer.buffer_cursor_pos, text, size);
}

CUSTOM_COMMAND_SIG(write_decrement){
    char text[] = "--";
    int size = sizeof(text) - 1;
    Buffer_Summary buffer = app->get_active_buffer(app);
    app->buffer_replace_range(app, &buffer, buffer.buffer_cursor_pos, buffer.buffer_cursor_pos, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces){
    File_View_Summary view;
    Buffer_Summary buffer;
    char text[] = "{\n\n}";
    int size = sizeof(text) - 1;
    int pos;
    
    view = app->get_active_file_view(app);
    buffer = app->get_buffer(app, view.buffer_id);
    
    pos = view.cursor.pos;
    app->buffer_replace_range(app, &buffer, pos, pos, text, size);
    app->view_set_cursor(app, &view, seek_pos(pos + 2), 1);
    
    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos + size);
    push_parameter(app, par_clear_blank_lines, 0);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(open_long_braces_semicolon){
    File_View_Summary view;
    Buffer_Summary buffer;
    char text[] = "{\n\n};";
    int size = sizeof(text) - 1;
    int pos;
    
    view = app->get_active_file_view(app);
    buffer = app->get_buffer(app, view.buffer_id);
    
    pos = view.cursor.pos;
    app->buffer_replace_range(app, &buffer, pos, pos, text, size);
    app->view_set_cursor(app, &view, seek_pos(pos + 2), 1);
    
    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos + size);
    push_parameter(app, par_clear_blank_lines, 0);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(paren_wrap){
    File_View_Summary view;
    Buffer_Summary buffer;

    char text1[] = "(";
    int size1 = sizeof(text1) - 1;

    char text2[] = ")";
    int size2 = sizeof(text2) - 1;

    Range range;
    int pos;

    view = app->get_active_file_view(app);
    buffer = app->get_active_buffer(app);

    range = get_range(&view);
    pos = range.max;
    app->buffer_replace_range(app, &buffer, pos, pos, text2, size2);

    pos = range.min;
    app->buffer_replace_range(app, &buffer, pos, pos, text1, size1);
}

CUSTOM_COMMAND_SIG(if0_off){
    File_View_Summary view;
    Buffer_Summary buffer;
    
    char text1[] = "#if 0\n";
    int size1 = sizeof(text1) - 1;
    
    char text2[] = "#endif\n";
    int size2 = sizeof(text2) - 1;
    
    Range range;
    int pos;

    view = app->get_active_file_view(app);
    buffer = app->get_active_buffer(app);

    range = get_range(&view);
    pos = range.min;

    app->buffer_replace_range(app, &buffer, pos, pos, text1, size1);

    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos);
    exec_command(app, cmdid_auto_tab_range);
    
    app->refresh_file_view(app, &view);
    range = get_range(&view);
    pos = range.max;
    
    app->buffer_replace_range(app, &buffer, pos, pos, text2, size2);
    
    push_parameter(app, par_range_start, pos);
    push_parameter(app, par_range_end, pos);
    exec_command(app, cmdid_auto_tab_range);
}

CUSTOM_COMMAND_SIG(backspace_word){
    File_View_Summary view;
    Buffer_Summary buffer;
    int pos2, pos1;
    
    view = app->get_active_file_view(app);
    
    pos2 = view.cursor.pos;
    exec_command(app, cmdid_seek_alphanumeric_left);
    app->refresh_file_view(app, &view);
    pos1 = view.cursor.pos;
    
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
}

CUSTOM_COMMAND_SIG(switch_to_compilation){
    File_View_Summary view;
    Buffer_Summary buffer;
    
    char name[] = "*compilation*";
    int name_size = sizeof(name)-1;

    // TODO(allen): This will only work for file views for now.  Extend the API
    // a bit to handle a general view type which can be manipulated at least enough
    // to change the specific type of view and set files even when the view didn't
    // contain a file.
    view = app->get_active_file_view(app);
    buffer = app->get_buffer_by_name(app, name, name_size);
    
    app->view_set_buffer(app, &view, buffer.buffer_id);
}

CUSTOM_COMMAND_SIG(move_up_10){
    File_View_Summary view;
    float x, y;

    view = app->get_active_file_view(app);
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
    File_View_Summary view;
    float x, y;

    view = app->get_active_file_view(app);
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

CUSTOM_COMMAND_SIG(switch_to_file_in_quotes){
    File_View_Summary view;
    Buffer_Summary buffer;
    char short_file_name[128];
    int pos, start, end, size;
    
    view = app->get_active_file_view(app);
    if (view.exists){
        buffer = app->get_buffer(app, view.buffer_id);
        if (buffer.ready){
            pos = view.cursor.pos;
            app->buffer_seek_delimiter(app, &buffer, pos, '"', 1, &end);
            app->buffer_seek_delimiter(app, &buffer, pos, '"', 0, &start);
            
            ++start;
            
            size = end - start;
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
    File_View_Summary view;
    Buffer_Summary buffer;
    User_Input in;
    Query_Bar bar;
    
    if (app->start_query_bar(app, &bar, 0) == 0) return;
    
    Range match;
    int reverse = start_reversed;
    int pos;
    
    view = app->get_active_file_view(app);
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
        
        if (CommandEqual(in.command, search)) step_forward = 1;
        if (CommandEqual(in.command, reverse_search)) step_backward = 1;
        
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
    if (!query_user_string(app, &with)) return;
    
    String r, w;
    r = replace.string;
    w = with.string;

    Buffer_Summary buffer;
    File_View_Summary view;

    view = app->get_active_file_view(app);
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
    if (!query_user_string(app, &with)) return;
    
    String r, w;
    r = replace.string;
    w = with.string;
    
    Query_Bar bar;
    Buffer_Summary buffer;
    File_View_Summary view;
    int pos, new_pos;
    
    bar.prompt = make_lit_string("Replace? (y)es, (n)ext, (esc)\n");
    bar.string = {};
    
    app->start_query_bar(app, &bar, 0);
    
    view = app->get_active_file_view(app);
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

CUSTOM_COMMAND_SIG(open_all_cpp_and_h){
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
    push_parameter(app, par_cli_overlap_with_conflict, 1);
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
    //   - par_cli_overlap_with_conflict: whether to launch this process if an existing process
    //     is already being used for output on the same buffer
    //
    //   - par_name: the name of the buffer to fill with the output from the process
    //
    //   - par_cli_path: sets the path from which the command is executed
    //
    //   - par_cli_command: sets the actual command to be executed, this can be almost any command
    //     that you could execute through a command line interface
    //
    //
    //     To set par_cli_path: push_parameter makes a copy of the dir string on the stack
    //         because the string allocated by push_directory is going to change again
    //     To set par_cli_command: app->push_parameter does not make a copy of the dir because
    //         dir isn't going to change again.
    // 
    // Step 3: If the batch file did not exist change the dir string to the  parent directory using
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
            
            push_parameter(app, par_cli_overlap_with_conflict, 0);
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

extern "C" GET_BINDING_DATA(get_bindings){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
    // NOTE(allen|a3.1): Right now hooks have no loyalties to maps, all hooks are
    // global and once set they always apply, regardless of what map is active.
    set_hook(context, hook_start, my_start);
    set_hook(context, hook_open_file, my_file_settings);
    
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
    bind(context, 'x', MDFR_ALT, cmdid_open_menu);
    bind(context, 'o', MDFR_ALT, open_in_other);
    
    bind(context, 'm', MDFR_ALT, build_search);
    bind(context, 'a', MDFR_ALT, open_all_cpp_and_h);
    
    // NOTE(allen): These callbacks may not actually be useful to you, but
    // go look at them and see what they do.
    bind(context, 'M', MDFR_ALT | MDFR_CTRL, open_my_files);
    bind(context, 'M', MDFR_ALT, build_at_launch_location);
    

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
    bind(context, '[', MDFR_CTRL, open_long_braces);
    bind(context, '{', MDFR_CTRL, open_long_braces);
    bind(context, '9', MDFR_CTRL, paren_wrap);
    bind(context, 'i', MDFR_ALT, if0_off);
    bind(context, '1', MDFR_ALT, switch_to_file_in_quotes);
    
    end_map(context);
    
    
    begin_map(context, mapid_file);
    
    // NOTE(allen|a3.4.4): Binding this essentially binds all key combos that
    // would normally insert a character into a buffer.
    // Or apply this rule: if the code for the key is not an enum value
    // such as key_left or key_back then it is a vanilla key.
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
    
    bind(context, 'K', MDFR_CTRL, cmdid_kill_buffer);
    bind(context, 'O', MDFR_CTRL, cmdid_reopen);
    bind(context, 'w', MDFR_CTRL, cmdid_interactive_save_as);
    bind(context, 's', MDFR_CTRL, cmdid_save);
    
    bind(context, ',', MDFR_ALT, switch_to_compilation);
    
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, ' ', MDFR_SHIFT, cmdid_write_character);
    
    end_map(context);
    
    end_bind_helper(context);
    
    return context->write_total;
}


