/*
4coder_remapping_commands.cpp - Commands that remap all of the keys to one of the maps
in the set of default maps.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_REMAPPING_COMMANDS_CPP)
#define FCODER_REMAPPING_COMMANDS_CPP

//
// Buffer Filling Helpers
//

enum{
    B_None  = 0x0,
    B_Major = 0x1,
    B_Minor = 0x2,
    B_Both  = 0x3,
    B_MASK  = 0x3,
    B_Shift = 0x4,
};

void
bind(Bind_Helper *context, Key_Code code, int32_t m, int32_t t, Generic_Command cmd){
    uint8_t f = 0;
    if (m & B_Shift){
        f |= MDFR_SHIFT;
    }
    
    switch (m & B_MASK){
        case B_None:
        {
            bind(context, code, MDFR_NONE|f, cmd);
        }break;
        
        case B_Major:
        {
            if (t == 0){
                bind(context, code, MDFR_CTRL|f, cmd);
            }
            else if (t == 1){
                bind(context, code, MDFR_CTRL|f, cmd);
                bind(context, code, MDFR_ALT|f, cmd);
            }
        }break;
        
        case B_Minor:
        {
            if (t == 0){
                bind(context, code, MDFR_ALT|f, cmd);
            }
            else if (t == 1){
                bind(context, code, MDFR_CMND|f, cmd);
            }
        }break;
        
        case B_Both:
        {
            if (t == 0){
                bind(context, code, MDFR_CTRL|MDFR_ALT|f, cmd);
            }
            else if (t == 1){
                bind(context, code, MDFR_CTRL|MDFR_CMND|f, cmd);
                bind(context, code, MDFR_ALT|f, cmd);
            }
        }break;
    }
}

void
bind(Bind_Helper *context, Key_Code code, int32_t m, int32_t t, Command_ID cmdid){
    Generic_Command cmd = {0};
    cmd.cmdid = cmdid;
    bind(context, code, m, t, cmd);
}

void
bind(Bind_Helper *context, Key_Code code, int32_t m, int32_t t, Custom_Command_Function *func){
    Generic_Command cmd = {0};
    cmd.command = func;
    bind(context, code, m, t, cmd);
}

void
default_key_template(Bind_Helper *context, int32_t t){
    // NOTE(allen|a4.0.22): GLOBAL
    begin_map(context, mapid_global);
    
    bind(context, 'p', B_Major, t, open_panel_vsplit);
    bind(context, '_', B_Major, t, open_panel_hsplit);
    bind(context, 'P', B_Major, t, close_panel);
    bind(context, ',', B_Major, t, change_active_panel);
    bind(context, '<', B_Major, t, change_active_panel_backwards);
    
    bind(context, 'n', B_Major, t, interactive_new);
    bind(context, 'o', B_Major, t, interactive_open_or_new);
    bind(context, 'o', B_Minor, t, open_in_other);
    bind(context, 'k', B_Major, t, interactive_kill_buffer);
    bind(context, 'i', B_Major, t, interactive_switch_buffer);
    bind(context, 'w', B_Major, t, save_as);
    bind(context, 'h', B_Major, t, project_go_to_root_directory);
    bind(context, 'S', B_Major, t, save_all_dirty_buffers);
    
    bind(context, 'c', B_Minor, t, open_color_tweaker);
    bind(context, 'd', B_Minor, t, open_debug);
    
    bind(context, '.', B_Minor, t, change_to_build_panel);
    bind(context, ',', B_Minor, t, close_build_panel);
    bind(context, 'n', B_Minor, t, goto_next_error);
    bind(context, 'N', B_Minor, t, goto_prev_error);
    bind(context, 'M', B_Minor, t, goto_first_error);
    bind(context, 'm', B_Minor, t, build_in_build_panel);
    
    bind(context, 'z', B_Minor, t, execute_any_cli);
    bind(context, 'Z', B_Minor, t, execute_previous_cli);
    
    bind(context, 'x', B_Minor, t, execute_arbitrary_command);
    
    bind(context, 's', B_Minor, t, show_scrollbar);
    bind(context, 'w', B_Minor, t, hide_scrollbar);
    bind(context, 'b', B_Minor, t, toggle_filebar);
    
    bind(context, '@', B_Minor, t, toggle_mouse);
    bind(context, key_page_up, B_Major, t, toggle_fullscreen);
    bind(context, 'E', B_Minor, t, exit_4coder);
    
    bind(context, key_f1, B_None, t, project_fkey_command);
    bind(context, key_f2, B_None, t, project_fkey_command);
    bind(context, key_f3, B_None, t, project_fkey_command);
    bind(context, key_f4, B_None, t, project_fkey_command);
    
    bind(context, key_f5, B_None, t, project_fkey_command);
    bind(context, key_f6, B_None, t, project_fkey_command);
    bind(context, key_f7, B_None, t, project_fkey_command);
    bind(context, key_f8, B_None, t, project_fkey_command);
    
    bind(context, key_f9, B_None, t, project_fkey_command);
    bind(context, key_f10, B_None, t, project_fkey_command);
    bind(context, key_f11, B_None, t, project_fkey_command);
    bind(context, key_f12, B_None, t, project_fkey_command);
    
    bind(context, key_f13, B_None, t, project_fkey_command);
    bind(context, key_f14, B_None, t, project_fkey_command);
    bind(context, key_f15, B_None, t, project_fkey_command);
    bind(context, key_f16, B_None, t, project_fkey_command);
    
    end_map(context);
    
    // NOTE(allen|a4.0.22): FILE
    begin_map(context, mapid_file);
    
    bind_vanilla_keys(context, write_character);
    
    bind(context, key_mouse_left, B_None, t, click_set_cursor);
    bind(context, key_mouse_left_release, B_None, t, click_set_mark);
    bind(context, key_mouse_right, B_None, t, click_set_mark);
    
    bind(context, key_left,      B_None,  t, move_left);
    bind(context, key_right,     B_None,  t, move_right);
    bind(context, key_del,       B_None,  t, delete_char);
    bind(context, key_del,       B_Shift, t, delete_char);
    bind(context, key_back,      B_None,  t, backspace_char);
    bind(context, key_back,      B_Shift, t, backspace_char);
    bind(context, key_up,        B_None,  t, move_up);
    bind(context, key_down,      B_None,  t, move_down);
    bind(context, key_end,       B_None,  t, seek_end_of_line);
    bind(context, key_home,      B_None,  t, seek_beginning_of_line);
    bind(context, key_page_up,   B_None,  t, page_up);
    bind(context, key_page_down, B_None,  t, page_down);
    
    bind(context, key_right, B_Major, t, seek_whitespace_right);
    bind(context, key_left,  B_Major, t, seek_whitespace_left);
    bind(context, key_up,    B_Major, t, seek_whitespace_up_end_line);
    bind(context, key_down,  B_Major, t, seek_whitespace_down_end_line);
    
    bind(context, key_up, B_Minor, t, move_up_10);
    bind(context, key_down, B_Minor, t, move_down_10);
    
    bind(context, key_back, B_Major, t, backspace_word);
    bind(context, key_del,  B_Major, t, delete_word);
    bind(context, key_back, B_Minor, t, snipe_token_or_word);
    bind(context, key_del,  B_Minor, t, snipe_token_or_word_right);
    
    bind(context, ' ', B_Major, t, set_mark);
    bind(context, 'a', B_Major, t, replace_in_range);
    bind(context, 'c', B_Major, t, copy);
    bind(context, 'd', B_Major, t, delete_range);
    bind(context, 'e', B_Major, t, center_view);
    bind(context, 'E', B_Major, t, left_adjust_view);
    bind(context, 'f', B_Major, t, search);
    bind(context, 'F', B_Major, t, list_all_locations);
    bind(context, 'F', B_Minor, t, list_all_substring_locations_case_insensitive);
    bind(context, 'g', B_Major, t, goto_line);
    bind(context, 'j', B_Major, t, to_lowercase);
    bind(context, 'K', B_Major, t, kill_buffer);
    bind(context, 'l', B_Major, t, toggle_line_wrap);
    bind(context, 'm', B_Major, t, cursor_mark_swap);
    bind(context, 'O', B_Major, t, reopen);
    bind(context, 'q', B_Major, t, query_replace);
    bind(context, 'Q', B_Major, t, query_replace_identifier);
    bind(context, 'r', B_Major, t, reverse_search);
    bind(context, 's', B_Major, t, save);
    bind(context, 't', B_Major, t, search_identifier);
    bind(context, 'T', B_Major, t, list_all_locations_of_identifier);
    bind(context, 'u', B_Major, t, to_uppercase);
    bind(context, 'v', B_Major, t, paste_and_indent);
    bind(context, 'v', B_Minor, t, toggle_virtual_whitespace);
    bind(context, 'V', B_Major, t, paste_next_and_indent);
    bind(context, 'x', B_Major, t, cut);
    bind(context, 'y', B_Major, t, redo);
    bind(context, 'z', B_Major, t, undo);
    
    bind(context, '2', B_Major, t, decrease_line_wrap);
    bind(context, '3', B_Major, t, increase_line_wrap);
    
    bind(context, '?', B_Major, t, toggle_show_whitespace);
    bind(context, '~', B_Major, t, clean_all_lines);
    bind(context, '\n', B_None, t, newline_or_goto_position);
    bind(context, '\n', B_Shift, t, newline_or_goto_position_same_panel);
    bind(context, ' ', B_Shift, t, write_character);
    
    end_map(context);
    
    // NOTE(allen|a4.0.22): CODE
    begin_map(context, default_code_map);
    
    inherit_map(context, mapid_file);
    
    bind(context, key_right, B_Major, t, seek_alphanumeric_or_camel_right);
    bind(context, key_left, B_Major, t, seek_alphanumeric_or_camel_left);
    
    bind(context, '\n', B_None, t, write_and_auto_tab);
    bind(context, '\n', B_Shift, t, write_and_auto_tab);
    bind(context, '}', B_None, t, write_and_auto_tab);
    bind(context, ')', B_None, t, write_and_auto_tab);
    bind(context, ']', B_None, t, write_and_auto_tab);
    bind(context, ';', B_None, t, write_and_auto_tab);
    bind(context, '#', B_None, t, write_and_auto_tab);
    
    bind(context, '\t', B_None, t, word_complete);
    bind(context, '\t', B_Major, t, auto_tab_range);
    bind(context, '\t', B_Shift, t, auto_tab_line_at_cursor);
    
    bind(context, 'h', B_Minor, t, write_hack);
    bind(context, 'r', B_Minor, t, write_block);
    bind(context, 't', B_Minor, t, write_todo);
    bind(context, 'y', B_Minor, t, write_note);
    bind(context, '[', B_Major, t, open_long_braces);
    bind(context, '{', B_Major, t, open_long_braces_semicolon);
    bind(context, '}', B_Major, t, open_long_braces_break);
    bind(context, 'i', B_Minor, t, if0_off);
    bind(context, '1', B_Minor, t, open_file_in_quotes);
    bind(context, '2', B_Minor, t, open_matching_file_cpp);
    bind(context, '0', B_Major, t, write_zero_struct);
    bind(context, 'I', B_Major, t, list_all_functions_current_buffer);
    
    end_map(context);
}

void
default_keys(Bind_Helper *context){
    default_key_template(context, 0);
}

void
mac_4coder_like_keys(Bind_Helper *context){
    default_key_template(context, 1);
}

void
mac_default_keys(Bind_Helper *context){
    // NOTE(allen|a4.0.22): GLOBAL
    begin_map(context, mapid_global);
    
    bind(context, 'p', MDFR_CMND, open_panel_vsplit);
    bind(context, '_', MDFR_CMND, open_panel_hsplit);
    bind(context, 'P', MDFR_CMND, close_panel);
    bind(context, ',', MDFR_CMND, change_active_panel);
    bind(context, '<', MDFR_CMND, change_active_panel_backwards);
    
    bind(context, 'n', MDFR_CMND, interactive_new);
    bind(context, 'o', MDFR_CMND, interactive_open_or_new);
    bind(context, 'o', MDFR_CTRL, open_in_other);
    bind(context, 'k', MDFR_CMND, interactive_kill_buffer);
    bind(context, 'i', MDFR_CMND, interactive_switch_buffer);
    bind(context, 'w', MDFR_CMND, save_as);
    bind(context, 'h', MDFR_CMND, project_go_to_root_directory);
    bind(context, 'S', MDFR_CMND, save_all_dirty_buffers);
    
    bind(context, 'c', MDFR_CTRL, open_color_tweaker);
    bind(context, 'd', MDFR_CTRL, open_debug);
    
    bind(context, '.', MDFR_CTRL, change_to_build_panel);
    bind(context, ',', MDFR_CTRL, close_build_panel);
    bind(context, 'n', MDFR_CTRL, goto_next_error);
    bind(context, 'N', MDFR_CTRL, goto_prev_error);
    bind(context, 'M', MDFR_CTRL, goto_first_error);
    bind(context, 'm', MDFR_CTRL, build_in_build_panel);
    
    bind(context, 'z', MDFR_CTRL, execute_any_cli);
    bind(context, 'Z', MDFR_CTRL, execute_previous_cli);
    
    bind(context, 'x', MDFR_CTRL, execute_arbitrary_command);
    
    bind(context, 's', MDFR_CTRL, show_scrollbar);
    bind(context, 'w', MDFR_CTRL, hide_scrollbar);
    bind(context, 'b', MDFR_CTRL, toggle_filebar);
    
    bind(context, '@', MDFR_CTRL, toggle_mouse);
    bind(context, key_page_up, MDFR_CMND, toggle_fullscreen);
    bind(context, 'E', MDFR_CTRL, exit_4coder);
    
    bind(context, key_f1, MDFR_NONE, project_fkey_command);
    bind(context, key_f2, MDFR_NONE, project_fkey_command);
    bind(context, key_f3, MDFR_NONE, project_fkey_command);
    bind(context, key_f4, MDFR_NONE, project_fkey_command);
    
    bind(context, key_f5, MDFR_NONE, project_fkey_command);
    bind(context, key_f6, MDFR_NONE, project_fkey_command);
    bind(context, key_f7, MDFR_NONE, project_fkey_command);
    bind(context, key_f8, MDFR_NONE, project_fkey_command);
    
    bind(context, key_f9, MDFR_NONE, project_fkey_command);
    bind(context, key_f10, MDFR_NONE, project_fkey_command);
    bind(context, key_f11, MDFR_NONE, project_fkey_command);
    bind(context, key_f12, MDFR_NONE, project_fkey_command);
    
    bind(context, key_f13, MDFR_NONE, project_fkey_command);
    bind(context, key_f14, MDFR_NONE, project_fkey_command);
    bind(context, key_f15, MDFR_NONE, project_fkey_command);
    bind(context, key_f16, MDFR_NONE, project_fkey_command);
    
    end_map(context);
    
    // NOTE(allen|a4.0.22): FILE
    begin_map(context, mapid_file);
    
    bind_vanilla_keys(context, write_character);
    
    bind(context, key_mouse_left, MDFR_NONE, click_set_cursor);
    bind(context, key_mouse_left_release, MDFR_NONE, click_set_mark);
    bind(context, key_mouse_right, MDFR_NONE, click_set_mark);
    
    bind(context, key_left, MDFR_NONE, move_left);
    bind(context, key_right, MDFR_NONE, move_right);
    bind(context, key_del, MDFR_NONE, delete_char);
    bind(context, key_del, MDFR_SHIFT, delete_char);
    bind(context, key_back, MDFR_NONE, backspace_char);
    bind(context, key_back, MDFR_SHIFT, backspace_char);
    bind(context, key_up, MDFR_NONE, move_up);
    bind(context, key_down, MDFR_NONE, move_down);
    bind(context, key_end, MDFR_NONE, seek_end_of_line);
    bind(context, key_home, MDFR_NONE, seek_beginning_of_line);
    bind(context, key_page_up, MDFR_NONE, page_up);
    bind(context, key_page_down, MDFR_NONE, page_down);
    
    bind(context, key_right, MDFR_CMND, seek_whitespace_right);
    bind(context, key_left, MDFR_CMND, seek_whitespace_left);
    bind(context, key_up, MDFR_CMND, seek_whitespace_up_end_line);
    bind(context, key_down, MDFR_CMND, seek_whitespace_down_end_line);
    
    bind(context, key_up, MDFR_ALT, move_up_10);
    bind(context, key_down, MDFR_ALT, move_down_10);
    
    bind(context, key_back, MDFR_CMND, backspace_word);
    bind(context, key_del, MDFR_CMND, delete_word);
    bind(context, key_back, MDFR_CTRL, snipe_token_or_word);
    bind(context, key_del, MDFR_CTRL, snipe_token_or_word_right);
    
    bind(context, ' ', MDFR_CTRL, set_mark);
    bind(context, 'a', MDFR_CMND, replace_in_range);
    bind(context, 'c', MDFR_CMND, copy);
    bind(context, 'd', MDFR_CMND, delete_range);
    bind(context, 'e', MDFR_CMND, center_view);
    bind(context, 'E', MDFR_CMND, left_adjust_view);
    bind(context, 'f', MDFR_CMND, search);
    bind(context, 'F', MDFR_CMND, list_all_locations);
    bind(context, 'F', MDFR_CTRL, list_all_substring_locations_case_insensitive);
    bind(context, 'g', MDFR_CMND, goto_line);
    bind(context, 'j', MDFR_CMND, to_lowercase);
    bind(context, 'K', MDFR_CMND, kill_buffer);
    bind(context, 'l', MDFR_CMND, toggle_line_wrap);
    bind(context, 'm', MDFR_CMND, cursor_mark_swap);
    bind(context, 'O', MDFR_CMND, reopen);
    bind(context, 'q', MDFR_CMND, query_replace);
    bind(context, 'Q', MDFR_CMND, query_replace_identifier);
    bind(context, 'r', MDFR_CMND, reverse_search);
    bind(context, 's', MDFR_CMND, save);
    bind(context, 't', MDFR_CMND, search_identifier);
    bind(context, 'T', MDFR_CMND, list_all_locations_of_identifier);
    bind(context, 'u', MDFR_CMND, to_uppercase);
    bind(context, 'v', MDFR_CMND, paste_and_indent);
    bind(context, 'v', MDFR_CTRL, toggle_virtual_whitespace);
    bind(context, 'V', MDFR_CMND, paste_next_and_indent);
    bind(context, 'x', MDFR_CMND, cut);
    bind(context, 'y', MDFR_CMND, redo);
    bind(context, 'z', MDFR_CMND, undo);
    
    bind(context, '2', MDFR_CMND, decrease_line_wrap);
    bind(context, '3', MDFR_CMND, increase_line_wrap);
    
    bind(context, '?', MDFR_CMND, toggle_show_whitespace);
    bind(context, '~', MDFR_CMND, clean_all_lines);
    bind(context, '\n', MDFR_NONE, newline_or_goto_position);
    bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel);
    bind(context, ' ', MDFR_SHIFT, write_character);
    
    end_map(context);
    
    // NOTE(allen|a4.0.22): CODE
    begin_map(context, default_code_map);
    
    inherit_map(context, mapid_file);
    
    bind(context, key_right, MDFR_CMND, seek_alphanumeric_or_camel_right);
    bind(context, key_left, MDFR_CMND, seek_alphanumeric_or_camel_left);
    
    bind(context, '\n', MDFR_NONE, write_and_auto_tab);
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, ')', MDFR_NONE, write_and_auto_tab);
    bind(context, ']', MDFR_NONE, write_and_auto_tab);
    bind(context, ';', MDFR_NONE, write_and_auto_tab);
    bind(context, '#', MDFR_NONE, write_and_auto_tab);
    
    bind(context, '\t', MDFR_NONE, word_complete);
    bind(context, '\t', MDFR_CMND, auto_tab_range);
    bind(context, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
    
    bind(context, 'h', MDFR_CTRL, write_hack);
    bind(context, 'r', MDFR_CTRL, write_block);
    bind(context, 't', MDFR_CTRL, write_todo);
    bind(context, 'y', MDFR_CTRL, write_note);
    bind(context, '[', MDFR_CMND, open_long_braces);
    bind(context, '{', MDFR_CMND, open_long_braces_semicolon);
    bind(context, '}', MDFR_CMND, open_long_braces_break);
    bind(context, 'i', MDFR_CTRL, if0_off);
    bind(context, '1', MDFR_CTRL, open_file_in_quotes);
    bind(context, '2', MDFR_CTRL, open_matching_file_cpp);
    bind(context, '0', MDFR_CMND, write_zero_struct);
    bind(context, 'I', MDFR_CMND, list_all_functions_current_buffer);
    
    end_map(context);
}


//
// Remapping Commands
//

static Bind_Helper
get_context_on_global_part(void){
    Bind_Helper result = {0};
    int32_t size = (1 << 20);
    for (;;){
        void *data = push_array(&global_part, char, size);
        if (data != 0){
            result = begin_bind_helper(data, size);
            break;
        }
        size = (size >> 1);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(set_bindings_choose){
#if defined(_WIN32) || defined(__linux__)
    
    set_bindings_default(app);
    
#elif defined(__APPLE__) && defined(__MACH__)
    
    set_bindings_mac_default(app);
    
#endif
}

CUSTOM_COMMAND_SIG(set_bindings_default){
    Temp_Memory temp = begin_temp_memory(&global_part);
    
    Bind_Helper context = get_context_on_global_part();
    set_all_default_hooks(&context);
    default_keys(&context);
    Bind_Buffer result = end_bind_helper_get_buffer(&context);
    global_set_mapping(app, result.data, result.size);
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(set_bindings_mac_4coder_like){
    Temp_Memory temp = begin_temp_memory(&global_part);
    
    Bind_Helper context = get_context_on_global_part();
    set_all_default_hooks(&context);
    mac_4coder_like_keys(&context);
    Bind_Buffer result = end_bind_helper_get_buffer(&context);
    global_set_mapping(app, result.data, result.size);
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(set_bindings_mac_default){
    Temp_Memory temp = begin_temp_memory(&global_part);
    
    Bind_Helper context = get_context_on_global_part();
    set_all_default_hooks(&context);
    mac_default_keys(&context);
    Bind_Buffer result = end_bind_helper_get_buffer(&context);
    global_set_mapping(app, result.data, result.size);
    
    end_temp_memory(temp);
}

#endif

// BOTTOM

