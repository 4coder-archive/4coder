
// TOP

#include "4coder_default_include.cpp"

// NOTE(allen|a3.3): All of your custom ids should be enumerated
// as shown here, they may start at 0, and you can only have
// 2^24 of them so don't be wasteful!
enum My_Maps{
    my_code_map,
    // for testing
    my_html_map,
    my_empty_map1,
    my_empty_map2,
    my_maps_count
};

CUSTOM_COMMAND_SIG(write_allen_todo){
    write_string(app, make_lit_string("// TODO(allen): "));
}

CUSTOM_COMMAND_SIG(write_allen_note){
    write_string(app, make_lit_string("// NOTE(allen): "));
}

CUSTOM_COMMAND_SIG(write_zero_struct){
    write_string(app, make_lit_string(" = {0};"));
}

CUSTOM_COMMAND_SIG(write_capital){
    User_Input command_in = app->get_command_input(app);
    char c = command_in.key.character_no_caps_lock;
    if (c != 0){
        c = char_to_upper(c);
        write_string(app, make_string(&c, 1));
    }
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
        y = view.cursor.unwrapped_y;
    }
    else{
        y = view.cursor.wrapped_y;
    }

    y += 10*view.line_height;

    app->view_set_cursor(app, &view, seek_xy(x, y, 0, view.unwrapped_lines), 0);
}

CUSTOM_COMMAND_SIG(rewrite_as_single_caps){
    View_Summary view;
    Buffer_Summary buffer;
    Full_Cursor cursor;
    Range range;
    String string;
    int is_first, i;
    
    view = app->get_active_view(app);
    cursor = view.cursor;
    
    exec_command(app, seek_token_left);
    app->refresh_view(app, &view);
    range.min = view.cursor.pos;
    
    exec_command(app, seek_token_right);
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
    
    app->view_set_cursor(app, &view,
                         seek_line_char(cursor.line+1, cursor.character),
                         1);
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

#if 0
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
#endif

HOOK_SIG(my_start){
    exec_command(app, cmdid_open_panel_vsplit);
    exec_command(app, cmdid_change_active_panel);
    
    app->change_theme(app, literal("4coder"));
    app->change_font(app, literal("liberation sans"));
    
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

    // no meaning for return
    return(0);
}

HOOK_SIG(my_file_settings){
    // NOTE(allen|a4): In hooks that want parameters, such as this file
    // opened hook.  The file created hook is guaranteed to have only
    // and exactly one buffer parameter.  In normal command callbacks
    // there are no parameter buffers.
    Buffer_Summary buffer = app->get_parameter_buffer(app, 0);
    assert(buffer.exists);

    int treat_as_code = 0;
    int wrap_lines = 1;

    if (buffer.file_name && buffer.size < (16 << 20)){
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match(ext, make_lit_string("cpp"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("h"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("c"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("hpp"))) treat_as_code = 1;
    }

    if (treat_as_code){
        wrap_lines = 0;
    }
    if (buffer.file_name[0] == '*'){
        wrap_lines = 0;
    }

    // NOTE(allen|a4.0.5): Unlike previous versions the command cmdid_set_settings
    // no longer automatically effects the active buffer.  This command will actually be
    // phased out in favor of an app call soon.
    push_parameter(app, par_buffer_id, buffer.buffer_id);
    
    push_parameter(app, par_lex_as_cpp_file, treat_as_code);
    push_parameter(app, par_wrap_lines, wrap_lines);
    push_parameter(app, par_key_mapid, (treat_as_code)?((int)my_code_map):((int)mapid_file));
    exec_command(app, cmdid_set_settings);

    // no meaning for return
    return(0);
}

void
default_keys(Bind_Helper *context){
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
    bind(context, 'w', MDFR_CTRL, save_as);

    bind(context, 'm', MDFR_ALT, build_search);
    bind(context, 'x', MDFR_ALT, execute_arbitrary_command);
    bind(context, 'z', MDFR_ALT, execute_any_cli);
    bind(context, 'Z', MDFR_ALT, execute_previous_cli);

    // NOTE(allen): These callbacks may not actually be useful to you, but
    // go look at them and see what they do.
    bind(context, 'M', MDFR_ALT | MDFR_CTRL, open_my_files);
    bind(context, 'M', MDFR_ALT, build_at_launch_location);

    end_map(context);
    
    begin_map(context, my_empty_map1);
    inherit_map(context, mapid_nomap);
    end_map(context);

    begin_map(context, my_empty_map2);
    inherit_map(context, mapid_nomap);
    end_map(context);

    begin_map(context, my_code_map);

    // NOTE(allen|a3.1): Set this map (my_code_map == mapid_user_custom) to
    // inherit from mapid_file.  When searching if a key is bound
    // in this map, if it is not found here it will then search mapid_file.
    //
    // If this is not set, it defaults to mapid_global.
    inherit_map(context, mapid_file);

    // NOTE(allen|a3.1): Children can override parent's bindings.
    bind(context, key_right, MDFR_CTRL, seek_alphanumeric_or_camel_right);
    bind(context, key_left, MDFR_CTRL, seek_alphanumeric_or_camel_left);

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
    bind(context, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);

    bind(context, '=', MDFR_CTRL, write_increment);
    bind(context, 't', MDFR_ALT, write_allen_todo);
    bind(context, 'n', MDFR_ALT, write_allen_note);
    bind(context, '[', MDFR_CTRL, open_long_braces);
    bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
    bind(context, '}', MDFR_CTRL, open_long_braces_break);
    bind(context, 'i', MDFR_ALT, if0_off);
    bind(context, '1', MDFR_ALT, open_file_in_quotes);
    bind(context, '0', MDFR_CTRL, write_zero_struct);

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

    bind(context, key_right, MDFR_CTRL, seek_whitespace_right);
    bind(context, key_left, MDFR_CTRL, seek_whitespace_left);
    bind(context, key_up, MDFR_CTRL, cmdid_seek_whitespace_up);
    bind(context, key_down, MDFR_CTRL, cmdid_seek_whitespace_down);

    bind(context, key_up, MDFR_ALT, move_up_10);
    bind(context, key_down, MDFR_ALT, move_down_10);

    bind(context, key_back, MDFR_CTRL, backspace_word);
    bind(context, key_back, MDFR_ALT, snipe_token_or_word);

    bind(context, ' ', MDFR_CTRL, cmdid_set_mark);
    bind(context, 'a', MDFR_CTRL, replace_in_range);
    bind(context, 'c', MDFR_CTRL, cmdid_copy);
    bind(context, 'd', MDFR_CTRL, cmdid_delete_range);
    bind(context, 'e', MDFR_CTRL, cmdid_center_view);
    bind(context, 'f', MDFR_CTRL, search);
    bind(context, 'g', MDFR_CTRL, goto_line);
    bind(context, 'h', MDFR_CTRL, cmdid_history_backward);
    bind(context, 'H', MDFR_CTRL, cmdid_history_forward);
    bind(context, 'l', MDFR_CTRL, cmdid_toggle_line_wrap);
    bind(context, 'j', MDFR_CTRL, cmdid_to_lowercase);
    bind(context, 'K', MDFR_CTRL, cmdid_kill_buffer);
    bind(context, 'L', MDFR_CTRL, cmdid_toggle_endline_mode);
    bind(context, 'm', MDFR_CTRL, cmdid_cursor_mark_swap);
    bind(context, 'O', MDFR_CTRL, cmdid_reopen);
    bind(context, 'q', MDFR_CTRL, query_replace);
    bind(context, 'r', MDFR_CTRL, reverse_search);
    bind(context, 's', MDFR_ALT, cmdid_show_scrollbar);
    bind(context, 's', MDFR_CTRL, cmdid_save);
    bind(context, 'u', MDFR_CTRL, cmdid_to_uppercase);
    bind(context, 'U', MDFR_CTRL, rewrite_as_single_caps);
    bind(context, 'v', MDFR_CTRL, cmdid_paste);
    bind(context, 'V', MDFR_CTRL, cmdid_paste_next);
    bind(context, 'w', MDFR_ALT, cmdid_hide_scrollbar);
    bind(context, 'x', MDFR_CTRL, cmdid_cut);
    bind(context, 'y', MDFR_CTRL, cmdid_redo);
    bind(context, 'z', MDFR_CTRL, cmdid_undo);
    
    
    bind(context, '1', MDFR_CTRL, cmdid_eol_dosify);
    
    bind(context, '?', MDFR_CTRL, cmdid_toggle_show_whitespace);
    bind(context, '~', MDFR_CTRL, cmdid_clean_all_lines);
    bind(context, '!', MDFR_CTRL, cmdid_eol_nixify);
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, ' ', MDFR_SHIFT, cmdid_write_character);
    
    end_map(context);
}

#ifndef NO_BINDING

extern "C" int
get_bindings(void *data, int size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    // NOTE(allen|a3.1): Hooks have no loyalties to maps. All hooks are global
    // and once set they always apply, regardless of what map is active.
    set_hook(context, hook_start, my_start);
    set_hook(context, hook_open_file, my_file_settings);

    set_scroll_rule(context, smooth_scroll_rule);
    
    default_keys(context);
    
    int result = end_bind_helper(context);
    return(result);
}

struct Custom_Vars{
    int initialized;
    Partition part;
};

enum View_Mode{
    ViewMode_File,
};

struct View_Vars{
    int id;
    View_Mode mode;
    
    GUI_Scroll_Vars scroll;
    i32_Rect scroll_region;
    
    int buffer_id;
};
inline View_Vars
view_vars_zero(){
    View_Vars vars = {0};
    return(vars);
}

extern "C" void
view_routine(Application_Links *app, int view_id){
    Custom_Vars *vars = (Custom_Vars*)app->memory;
    View_Vars view = {0};
    view.id = view_id;
    
    int show_scrollbar = 1;
    
    if (!vars->initialized){
        vars->initialized = 1;
        vars->part = make_part(app->memory, app->memory_size);
        push_struct(&vars->part, Custom_Vars);
    }
    
    for(;;){
        Event_Message message = {0};
        message = app->get_event_message(app);
        
        switch (message.type){
            case EM_Open_View:
            {
                view = view_vars_zero();
                view.id = view_id;
            }break;
            
            case EM_Frame:
            {
                GUI_Functions *guifn = app->get_gui_functions(app);
                GUI *gui = app->get_gui(app, view_id);
                
                guifn->begin(gui);
                guifn->top_bar(gui);
                
                switch (view.mode){
                    case ViewMode_File:
                    // TODO(allen): Overlapped widget
                    GUI_id scroll_id;
                    scroll_id.id[1] = view.mode;
                    scroll_id.id[0] = view.buffer_id;
                    
                    guifn->get_scroll_vars(gui, scroll_id, &view.scroll,
                                           &view.scroll_region);
                    guifn->begin_scrollable(gui, scroll_id, view.scroll,
                                            144.f, show_scrollbar);
                    guifn->file(gui, view.buffer_id);
                    guifn->end_scrollable(gui);
                    break;
                }
                
                guifn->end(gui);
                
                // TODO(allen): Put this code in charge of dispatching
                // to the command or command coroutine or whatever.
                
                // TODO(allen): Put this code in charge of when to process
                // the GUI with input and retrieve new layout data.
            }break;
            
            case EM_Close_View:
            {}break;
        }
    }
}

#endif

// BOTTOM
