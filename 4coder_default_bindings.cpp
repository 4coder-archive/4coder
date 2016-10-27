
// TOP

#ifndef FCODER_DEFAULT_BINDINGS
#define FCODER_DEFAULT_BINDINGS

#include "4coder_default_include.cpp"

// NOTE(allen|a3.3): All of your custom ids should be enumerated
// as shown here, they may start at 0, and you can only have
// 2^24 of them so don't be wasteful!
enum My_Maps{
    my_code_map,
    
    my_maps_count
};

CUSTOM_COMMAND_SIG(write_allen_todo){
    write_string(app, make_lit_string("// TODO(allen): "));
}

CUSTOM_COMMAND_SIG(write_allen_note){
    write_string(app, make_lit_string("// NOTE(allen): "));
}

CUSTOM_COMMAND_SIG(write_allen_doc){
    write_string(app, make_lit_string("/* DOC() */"));
}

CUSTOM_COMMAND_SIG(write_zero_struct){
    write_string(app, make_lit_string(" = {0};"));
}

CUSTOM_COMMAND_SIG(write_capital){
    User_Input command_in = get_command_input(app);
    char c = command_in.key.character_no_caps_lock;
    if (c != 0){
        c = char_to_upper(c);
        write_string(app, make_string(&c, 1));
    }
}

CUSTOM_COMMAND_SIG(switch_to_compilation){
    
    char name[] = "*compilation*";
    int32_t name_size = sizeof(name)-1;
    
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer_by_name(app, name, name_size, access);
    
    view_set_buffer(app, &view, buffer.buffer_id, 0);
}

CUSTOM_COMMAND_SIG(rewrite_as_single_caps){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Full_Cursor cursor = view.cursor;
    
    // TODO(allen): This can be rewritten now without moving the
    // cursor around, instead just calling the boundary seek.
    Range range = {0};
    exec_command(app, seek_token_left);
    refresh_view(app, &view);
    range.min = view.cursor.pos;
    
    exec_command(app, seek_token_right);
    refresh_view(app, &view);
    range.max = view.cursor.pos;
    
    String string = {0};
    string.str = (char*)app->memory;
    string.size = range.max - range.min;
    assert(string.size < app->memory_size);
    
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    buffer_read_range(app, &buffer, range.min, range.max, string.str);
    
    int32_t is_first = true;
    for (int32_t i = 0; i < string.size; ++i){
        if (char_is_alpha_true(string.str[i])){
            if (is_first){
                is_first = false;
            }
            else{
                string.str[i] = char_to_lower(string.str[i]);
            }
        }
        else{
            is_first = true;
        }
    }
    
    buffer_replace_range(app, &buffer, range.min, range.max, string.str, string.size);
    
    view_set_cursor(app, &view,
                         seek_line_char(cursor.line+1, cursor.character),
                         true);
}

CUSTOM_COMMAND_SIG(open_my_files){
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    view_open_file(app, &view, literal("w:/4ed/data/test/basic.cpp"), true);
}

CUSTOM_COMMAND_SIG(build_at_launch_location){
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    exec_system_command(app, &view,
                             buffer_identifier(literal("*compilation*")),
                             literal("."),
                             literal("build"),
                             CLI_OverlapWithConflict);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up_end_line){
    exec_command(app, seek_whitespace_up);
    exec_command(app, seek_end_of_line);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down_end_line){
    exec_command(app, seek_whitespace_down);
    exec_command(app, seek_end_of_line);
}

static bool32 enable_code_wrapping = 1;
static int32_t default_wrap_width = 672;

HOOK_SIG(my_start){
    init_memory(app);
    
    {
        FILE *file = fopen("config.4coder", "rb");
        if (file){
            Temp_Memory temp = begin_temp_memory(&global_part);
            
            fseek(file, 0, SEEK_END);
            int32_t size = ftell(file);
            char *mem = (char*)push_block(&global_part, size+1);
            fseek(file, 0, SEEK_SET);
            fread(mem, 1, size+1, file);
            fclose(file);
            
            Cpp_Token_Array array;
            array.count = 0;
            array.max_count = (1 << 20)/sizeof(Cpp_Token);
            array.tokens = push_array(&global_part, Cpp_Token, array.max_count);
            
            Cpp_Lex_Data S = cpp_lex_data_init();
            Cpp_Lex_Result result = cpp_lex_step(&S, mem, size, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
            
            if (result == LexResult_Finished){
                
                for (int32_t i = 0; i < array.count; ++i){
                int32_t read_setting_failed = 1;
                    Cpp_Token id_token = array.tokens[i];
                if (id_token.type == CPP_TOKEN_IDENTIFIER){
                    ++i;
                    if (i < array.count){
                    Cpp_Token eq_token = array.tokens[i];
                        if (eq_token.type == CPP_TOKEN_EQEQ){
                            ++i;
                            if (i < array.count){
                                Cpp_Token val_token = array.tokens[i];
                                {
                                ++i;
                                    if (i < array.count){
                                        Cpp_Token semicolon_token = array.tokens[i];
                                        if (semicolon_token.type == CPP_TOKEN_SEMICOLON){
                                            read_setting_failed = 0;
                                            
                                            String id = make_string(mem + id_token.start, id_token.size);
                                            
                                            if (match(id, "enable_code_wrapping")){
                                                if (val_token.type == CPP_TOKEN_BOOLEAN_CONSTANT){
                                                    String val = make_string(mem + val_token.start, val_token.size);
                                                    if (val.str[0] == 't'){
                                                        enable_code_wrapping = 1;
                                                    }
                                                    else{
                                                        enable_code_wrapping = 0;
                                                    }
                                                }
                                                }
                                        }
                                }
                            }
                            }
                        }
                    }
                }
                
                if (read_setting_failed){
                    for (; i < array.count; ++i){
                        Cpp_Token token = array.tokens[i];
                        if (token.type == CPP_TOKEN_SEMICOLON){
                            break;
                        }
                    }
                }
            }
            }
            
            end_temp_memory(temp);
        }
    }
    
    change_theme(app, literal("4coder"));
    change_font(app, literal("Liberation Sans"), true);
    
    exec_command(app, open_panel_vsplit);
    exec_command(app, hide_scrollbar);
    exec_command(app, change_active_panel);
    exec_command(app, hide_scrollbar);
    
    {
        View_Summary view = get_active_view(app, AccessAll);
         int32_t width = view.file_region.x1 - view.file_region.x0;
        default_wrap_width = width - 40;
    }
    
    // Theme options:
    //  "4coder"
    //  "Handmade Hero"
    //  "Twilight"
    //  "Woverine"
    //  "stb"
    
    // Font options:
    //  "Liberation Sans"
    //  "Liberation Mono"
    //  "Hack"
    //  "Cutive Mono"
    //  "Inconsolata"
    
    // no meaning for return
    return(0);
}

HOOK_SIG(my_exit){
    // if this returns zero it cancels the exit.
    return(1);
}

// NOTE(allen|a4.0.12): This is for testing it may be removed and replaced with a better test for the buffer_get_font when you eventally read this and wonder what it's about.
CUSTOM_COMMAND_SIG(write_name_of_font){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    char font_name[256];
    int32_t font_max = 256;
    int32_t font_len = buffer_get_font(app, &buffer, font_name, font_max);
    
    if (font_len != 0){
    write_string(app, &view, &buffer, make_string(font_name, font_len));
    }
}

CUSTOM_COMMAND_SIG(newline_or_goto_position){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    if (buffer.lock_flags & AccessProtected){
        exec_command(app, goto_jump_at_cursor);
        lock_jump_buffer(buffer);
    }
    else{
        exec_command(app, write_character);
    }
}

// TODO(allen): Eliminate this hook if you can.
OPEN_FILE_HOOK_SIG(my_file_settings){
    // NOTE(allen|a4.0.8): The get_parameter_buffer was eliminated
    // and instead the buffer is passed as an explicit parameter through
    // the function call.  That is where buffer_id comes from here.
    uint32_t access = AccessAll;
    Buffer_Summary buffer = get_buffer(app, buffer_id, access);
    assert(buffer.exists);
    
    int32_t treat_as_code = 0;
    int32_t wrap_lines = 1;
    
    if (buffer.file_name && buffer.size < (16 << 20)){
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match_ss(ext, make_lit_string("cpp"))) treat_as_code = 1;
        else if (match_ss(ext, make_lit_string("h"))) treat_as_code = 1;
        else if (match_ss(ext, make_lit_string("c"))) treat_as_code = 1;
        else if (match_ss(ext, make_lit_string("hpp"))) treat_as_code = 1;
    }
    
    if (treat_as_code){
        wrap_lines = 0;
    }
    if (buffer.file_name[0] == '*'){
        wrap_lines = 0;
    }
    
    buffer_set_setting(app, &buffer, BufferSetting_Lex, treat_as_code);
    buffer_set_setting(app, &buffer, BufferSetting_WrapLine, wrap_lines);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, default_wrap_width);
    buffer_set_setting(app, &buffer, BufferSetting_MapID, (treat_as_code)?((int32_t)my_code_map):((int32_t)mapid_file));
    
    if (treat_as_code && enable_code_wrapping && buffer.size < (1 << 20)){
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, 1);
        buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, 1);
    }
    
    // no meaning for return
    return(0);
}

// NOTE(allen|a4.0.9): The input filter allows you to modify the input
// to a frame before 4coder starts processing it at all.
//
// Right now it only has access to the mouse state, but it will be
// extended to have access to the key presses soon.
static int32_t suppressing_mouse = false;

INPUT_FILTER_SIG(my_suppress_mouse_filter){
    if (suppressing_mouse){
        *mouse = null_mouse_state;
        mouse->x = -100;
        mouse->y = -100;
    }
}

static void
set_mouse_suppression(Application_Links *app, int32_t suppress){
    if (suppress){
        suppressing_mouse = true;
        show_mouse_cursor(app, MouseCursorShow_Never);
    }
    else{
        suppressing_mouse = false;
        show_mouse_cursor(app, MouseCursorShow_Always);
    }
}

CUSTOM_COMMAND_SIG(suppress_mouse){
    set_mouse_suppression(app, true);
}

CUSTOM_COMMAND_SIG(allow_mouse){
    set_mouse_suppression(app, false);
}

CUSTOM_COMMAND_SIG(toggle_mouse){
    set_mouse_suppression(app, !suppressing_mouse);
}

void
default_keys(Bind_Helper *context){
    begin_map(context, mapid_global);
    
    bind(context, 'p', MDFR_CTRL, open_panel_vsplit);
    bind(context, '_', MDFR_CTRL, open_panel_hsplit);
    bind(context, 'P', MDFR_CTRL, close_panel);
    bind(context, ',', MDFR_CTRL, change_active_panel);
    
    bind(context, 'n', MDFR_CTRL, cmdid_interactive_new);
    bind(context, 'o', MDFR_CTRL, cmdid_interactive_open);
    bind(context, 'o', MDFR_ALT, open_in_other);
    bind(context, 'k', MDFR_CTRL, cmdid_interactive_kill_buffer);
    bind(context, 'i', MDFR_CTRL, cmdid_interactive_switch_buffer);
    bind(context, 'w', MDFR_CTRL, save_as);
    
    bind(context, 'c', MDFR_ALT, cmdid_open_color_tweaker);
    bind(context, 'd', MDFR_ALT, cmdid_open_debug);
    
    bind(context, '.', MDFR_ALT, change_to_build_panel);
    bind(context, ',', MDFR_ALT, close_build_panel);
    bind(context, 'n', MDFR_ALT, goto_next_error);
    bind(context, 'N', MDFR_ALT, goto_prev_error);
    bind(context, 'M', MDFR_ALT, goto_first_error);
    bind(context, 'm', MDFR_ALT, build_in_build_panel);
    
    bind(context, 'z', MDFR_ALT, execute_any_cli);
    bind(context, 'Z', MDFR_ALT, execute_previous_cli);
    
    bind(context, 'x', MDFR_ALT, execute_arbitrary_command);
    
    bind(context, 's', MDFR_ALT, show_scrollbar);
    bind(context, 'w', MDFR_ALT, hide_scrollbar);
    
    bind(context, key_f2, MDFR_NONE, toggle_mouse);
    bind(context, key_page_up, MDFR_CTRL, toggle_fullscreen);
    bind(context, 'E', MDFR_ALT, exit_4coder);
    
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
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, ')', MDFR_NONE, write_and_auto_tab);
    bind(context, ']', MDFR_NONE, write_and_auto_tab);
    bind(context, ';', MDFR_NONE, write_and_auto_tab);
    bind(context, '#', MDFR_NONE, write_and_auto_tab);
    
    bind(context, '\t', MDFR_NONE, word_complete);
    bind(context, '\t', MDFR_CTRL, auto_tab_range);
    bind(context, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
    
    bind(context, 't', MDFR_ALT, write_allen_todo);
    bind(context, 'y', MDFR_ALT, write_allen_note);
    bind(context, 'r', MDFR_ALT, write_allen_doc);
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
    bind_vanilla_keys(context, write_character);
    
    // NOTE(allen|a4.0.7): You can now bind left and right clicks.
    // They only trigger on mouse presses.  Modifiers do work
    // so control+click shift+click etc can now have special meanings.
    bind(context, key_mouse_left, MDFR_NONE, click_set_cursor);
    bind(context, key_mouse_right, MDFR_NONE, click_set_mark);
    
    // NOTE(allen|a4.0.11): You can now bind left and right mouse
    // button releases.  Modifiers do work so control+click shift+click
    // etc can now have special meanings.
    bind(context, key_mouse_left_release, MDFR_NONE, click_set_mark);
    
    bind(context, key_left, MDFR_NONE, move_left);
    bind(context, key_right, MDFR_NONE, move_right);
    bind(context, key_del, MDFR_NONE, delete_char);
    bind(context, key_back, MDFR_NONE, backspace_char);
    bind(context, key_up, MDFR_NONE, move_up);
    bind(context, key_down, MDFR_NONE, move_down);
    bind(context, key_end, MDFR_NONE, seek_end_of_line);
    bind(context, key_home, MDFR_NONE, seek_beginning_of_line);
    bind(context, key_page_up, MDFR_NONE, page_up);
    bind(context, key_page_down, MDFR_NONE, page_down);
    
    bind(context, key_right, MDFR_CTRL, seek_whitespace_right);
    bind(context, key_left, MDFR_CTRL, seek_whitespace_left);
    bind(context, key_up, MDFR_CTRL, seek_whitespace_up_end_line);
    bind(context, key_down, MDFR_CTRL, seek_whitespace_down_end_line);
    
    bind(context, key_up, MDFR_ALT, move_up_10);
    bind(context, key_down, MDFR_ALT, move_down_10);
    
    bind(context, key_back, MDFR_CTRL, backspace_word);
    bind(context, key_del, MDFR_CTRL, delete_word);
    bind(context, key_back, MDFR_ALT, snipe_token_or_word);
    
    bind(context, ' ', MDFR_CTRL, set_mark);
    bind(context, 'a', MDFR_CTRL, replace_in_range);
    bind(context, 'c', MDFR_CTRL, copy);
    bind(context, 'd', MDFR_CTRL, delete_range);
    bind(context, 'e', MDFR_CTRL, center_view);
    bind(context, 'E', MDFR_CTRL, left_adjust_view);
    bind(context, 'f', MDFR_CTRL, search);
    bind(context, 'F', MDFR_CTRL, list_all_locations);
    bind(context, 'F', MDFR_ALT, list_all_substring_locations_case_insensitive);
    bind(context, 'g', MDFR_CTRL, goto_line);
    bind(context, 'h', MDFR_CTRL, cmdid_history_backward);
    bind(context, 'H', MDFR_CTRL, cmdid_history_forward);
    bind(context, 'j', MDFR_CTRL, to_lowercase);
    bind(context, 'K', MDFR_CTRL, cmdid_kill_buffer);
    bind(context, 'l', MDFR_CTRL, toggle_line_wrap);
    bind(context, 'm', MDFR_CTRL, cursor_mark_swap);
    bind(context, 'O', MDFR_CTRL, cmdid_reopen);
    bind(context, 'q', MDFR_CTRL, query_replace);
    bind(context, 'r', MDFR_CTRL, reverse_search);
    bind(context, 's', MDFR_CTRL, cmdid_save);
    bind(context, 'T', MDFR_CTRL, list_all_locations_of_identifier);
    bind(context, 'u', MDFR_CTRL, to_uppercase);
    bind(context, 'U', MDFR_CTRL, rewrite_as_single_caps);
    bind(context, 'v', MDFR_CTRL, paste_and_indent);
    bind(context, 'v', MDFR_ALT, toggle_virtual_whitespace);
    bind(context, 'V', MDFR_CTRL, paste_next_and_indent);
    bind(context, 'x', MDFR_CTRL, cut);
    bind(context, 'y', MDFR_CTRL, cmdid_redo);
    bind(context, 'z', MDFR_CTRL, cmdid_undo);
    
    bind(context, '1', MDFR_CTRL, eol_dosify);
    bind(context, '!', MDFR_CTRL, eol_nixify);
    
    bind(context, '2', MDFR_CTRL, decrease_line_wrap);
    bind(context, '3', MDFR_CTRL, increase_line_wrap);
    
    bind(context, '?', MDFR_CTRL, toggle_show_whitespace);
    bind(context, '~', MDFR_CTRL, clean_all_lines);
    bind(context, '\n', MDFR_NONE, newline_or_goto_position);
    bind(context, '\n', MDFR_SHIFT, newline_or_goto_position);
    bind(context, ' ', MDFR_SHIFT, write_character);
    
    bind(context, ';', MDFR_ALT, write_name_of_font);
    
    end_map(context);
}



#ifndef NO_BINDING

extern "C" int32_t
get_bindings(void *data, int32_t size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    // NOTE(allen|a3.1): Hooks have no loyalties to maps. All hooks are global
    // and once set they always apply, regardless of what map is active.
    set_hook(context, hook_start, my_start);
    set_hook(context, hook_exit, my_exit);
    
    set_open_file_hook(context, my_file_settings);
    set_command_caller(context, default_command_caller);
    set_input_filter(context, my_suppress_mouse_filter);
    set_scroll_rule(context, smooth_scroll_rule);
    
    default_keys(context);
    
    int32_t result = end_bind_helper(context);
    return(result);
}

#endif //NO_BINDING

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM
