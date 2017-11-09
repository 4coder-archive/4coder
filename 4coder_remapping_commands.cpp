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
    B_None,
    B_Major,
    B_Minor,
    B_Both,
};

void
bind(Bind_Helper *context, 
     
     void
     default_keys(Bind_Helper *context){
     // NOTE(allen|a4.0.22): GLOBAL
     begin_map(context, mapid_global);
     
     bind(context, 'p', MDFR_CTRL, open_panel_vsplit);
     bind(context, '_', MDFR_CTRL, open_panel_hsplit);
     bind(context, 'P', MDFR_CTRL, close_panel);
     bind(context, ',', MDFR_CTRL, change_active_panel);
     bind(context, '<', MDFR_CTRL, change_active_panel_backwards);
     
     bind(context, 'n', MDFR_CTRL, interactive_new);
     bind(context, 'o', MDFR_CTRL, interactive_open_or_new);
     bind(context, 'o', MDFR_ALT, open_in_other);
     bind(context, 'k', MDFR_CTRL, interactive_kill_buffer);
     bind(context, 'i', MDFR_CTRL, interactive_switch_buffer);
     bind(context, 'w', MDFR_CTRL, save_as);
     bind(context, 'h', MDFR_CTRL, project_go_to_root_directory);
     bind(context, 'S', MDFR_CTRL, save_all_dirty_buffers);
     
     bind(context, 'c', MDFR_ALT, open_color_tweaker);
     bind(context, 'd', MDFR_ALT, open_debug);
     
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
     bind(context, 'b', MDFR_ALT, toggle_filebar);
     
     bind(context, '@', MDFR_ALT, toggle_mouse);
     bind(context, key_page_up, MDFR_CTRL, toggle_fullscreen);
     bind(context, 'E', MDFR_ALT, exit_4coder);
     
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
     bind(context, key_mouse_right, MDFR_NONE, click_set_mark);
     
     bind(context, key_mouse_left_release, MDFR_NONE, click_set_mark);
     
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
     
     bind(context, key_right, MDFR_CTRL, seek_whitespace_right);
     bind(context, key_left, MDFR_CTRL, seek_whitespace_left);
     bind(context, key_up, MDFR_CTRL, seek_whitespace_up_end_line);
     bind(context, key_down, MDFR_CTRL, seek_whitespace_down_end_line);
     
     bind(context, key_up, MDFR_ALT, move_up_10);
     bind(context, key_down, MDFR_ALT, move_down_10);
     
     bind(context, key_back, MDFR_CTRL, backspace_word);
     bind(context, key_del, MDFR_CTRL, delete_word);
     bind(context, key_back, MDFR_ALT, snipe_token_or_word);
     bind(context, key_del, MDFR_ALT, snipe_token_or_word_right);
     
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
     bind(context, 'j', MDFR_CTRL, to_lowercase);
     bind(context, 'K', MDFR_CTRL, kill_buffer);
     bind(context, 'l', MDFR_CTRL, toggle_line_wrap);
     bind(context, 'm', MDFR_CTRL, cursor_mark_swap);
     bind(context, 'O', MDFR_CTRL, reopen);
     bind(context, 'q', MDFR_CTRL, query_replace);
     bind(context, 'Q', MDFR_CTRL, query_replace_identifier);
     bind(context, 'r', MDFR_CTRL, reverse_search);
     bind(context, 's', MDFR_CTRL, save);
     bind(context, 't', MDFR_CTRL, search_identifier);
     bind(context, 'T', MDFR_CTRL, list_all_locations_of_identifier);
     bind(context, 'u', MDFR_CTRL, to_uppercase);
     bind(context, 'v', MDFR_CTRL, paste_and_indent);
     bind(context, 'v', MDFR_ALT, toggle_virtual_whitespace);
     bind(context, 'V', MDFR_CTRL, paste_next_and_indent);
     bind(context, 'x', MDFR_CTRL, cut);
     bind(context, 'y', MDFR_CTRL, redo);
     bind(context, 'z', MDFR_CTRL, undo);
     
     bind(context, '2', MDFR_CTRL, decrease_line_wrap);
     bind(context, '3', MDFR_CTRL, increase_line_wrap);
     
     bind(context, '?', MDFR_CTRL, toggle_show_whitespace);
     bind(context, '~', MDFR_CTRL, clean_all_lines);
     bind(context, '\n', MDFR_NONE, newline_or_goto_position);
     bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel);
     bind(context, ' ', MDFR_SHIFT, write_character);
     
     end_map(context);
     
     // NOTE(allen|a4.0.22): CODE
     begin_map(context, default_code_map);
     
     inherit_map(context, mapid_file);
     
     bind(context, key_right, MDFR_CTRL, seek_alphanumeric_or_camel_right);
     bind(context, key_left, MDFR_CTRL, seek_alphanumeric_or_camel_left);
     
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
     
     bind(context, 'h', MDFR_ALT, write_hack);
     bind(context, 'r', MDFR_ALT, write_block);
     bind(context, 't', MDFR_ALT, write_todo);
     bind(context, 'y', MDFR_ALT, write_note);
     bind(context, '[', MDFR_CTRL, open_long_braces);
     bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
     bind(context, '}', MDFR_CTRL, open_long_braces_break);
     bind(context, 'i', MDFR_ALT, if0_off);
     bind(context, '1', MDFR_ALT, open_file_in_quotes);
     bind(context, '2', MDFR_ALT, open_matching_file_cpp);
     bind(context, '0', MDFR_CTRL, write_zero_struct);
     bind(context, 'I', MDFR_CTRL, list_all_functions_current_buffer);
     
     end_map(context);
     }
     
     void
     bind_ctrl_and_cmnd(Bind_Helper *context, Key_Code code, int32_t command){
     bind(context, code, MDFR_CTRL, command);
     bind(context, code, MDFR_CMND, command);
     }
     
     void
     bind_ctrl_and_cmnd(Bind_Helper *context, Key_Code code, Custom_Command_Function *command){
     bind(context, code, MDFR_CTRL, command);
     bind(context, code, MDFR_CMND, command);
     bind(context, code, MDFR_CTRL|MDFR_CMND, command);
     }
     
     void
     mac_4coder_like_keys(Bind_Helper *context){
     // NOTE(allen|a4.0.22): GLOBAL
     begin_map(context, mapid_global);
     
     bind_ctrl_and_cmnd(context, 'p', open_panel_vsplit);
     bind_ctrl_and_cmnd(context, '_', open_panel_hsplit);
     bind_ctrl_and_cmnd(context, 'P', close_panel);
     bind_ctrl_and_cmnd(context, ',', change_active_panel);
     bind_ctrl_and_cmnd(context, '<', change_active_panel_backwards);
     
     bind_ctrl_and_cmnd(context, 'n', interactive_new);
     bind_ctrl_and_cmnd(context, 'o', interactive_open_or_new);
     bind(context, 'o', MDFR_ALT, open_in_other);
     bind_ctrl_and_cmnd(context, 'k', interactive_kill_buffer);
     bind_ctrl_and_cmnd(context, 'i', interactive_switch_buffer);
     bind_ctrl_and_cmnd(context, 'w', save_as);
     bind_ctrl_and_cmnd(context, 'h', project_go_to_root_directory);
     bind_ctrl_and_cmnd(context, 'S', save_all_dirty_buffers);
     
     bind(context, 'c', MDFR_ALT, open_color_tweaker);
     bind(context, 'd', MDFR_ALT, open_debug);
     
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
     bind(context, 'b', MDFR_ALT, toggle_filebar);
     
     bind(context, '@', MDFR_ALT, toggle_mouse);
     bind(context, key_page_up, MDFR_CTRL, toggle_fullscreen);
     bind(context, 'E', MDFR_ALT, exit_4coder);
     
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
     bind(context, key_mouse_right, MDFR_NONE, click_set_mark);
     
     bind(context, key_mouse_left_release, MDFR_NONE, click_set_mark);
     
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
     bind(context, key_left,  MDFR_CMND, seek_whitespace_left);
     bind(context, key_up,    MDFR_CMND, seek_whitespace_up_end_line);
     bind(context, key_down,  MDFR_CMND, seek_whitespace_down_end_line);
     
     bind(context, key_up, MDFR_ALT, move_up_10);
     bind(context, key_down, MDFR_ALT, move_down_10);
     
     bind_ctrl_and_cmnd(context, key_back, backspace_word);
     bind_ctrl_and_cmnd(context, key_del, delete_word);
     bind(context, key_back, MDFR_ALT, snipe_token_or_word);
     bind(context, key_del, MDFR_ALT, snipe_token_or_word_right);
     
     bind_ctrl_and_cmnd(context, ' ', set_mark);
     bind_ctrl_and_cmnd(context, 'a', replace_in_range);
     bind_ctrl_and_cmnd(context, 'c', copy);
     bind_ctrl_and_cmnd(context, 'd', delete_range);
     bind_ctrl_and_cmnd(context, 'e', center_view);
     bind_ctrl_and_cmnd(context, 'E', left_adjust_view);
     bind_ctrl_and_cmnd(context, 'f', search);
     bind_ctrl_and_cmnd(context, 'F', list_all_locations);
     bind(context, 'F', MDFR_ALT, list_all_substring_locations_case_insensitive);
     bind_ctrl_and_cmnd(context, 'g', goto_line);
     bind_ctrl_and_cmnd(context, 'j', to_lowercase);
     bind_ctrl_and_cmnd(context, 'K', kill_buffer);
     bind_ctrl_and_cmnd(context, 'l', toggle_line_wrap);
     bind_ctrl_and_cmnd(context, 'm', cursor_mark_swap);
     bind_ctrl_and_cmnd(context, 'O', reopen);
     bind_ctrl_and_cmnd(context, 'q', query_replace);
     bind_ctrl_and_cmnd(context, 'Q', query_replace_identifier);
     bind_ctrl_and_cmnd(context, 'r', reverse_search);
     bind_ctrl_and_cmnd(context, 's', save);
     bind_ctrl_and_cmnd(context, 't', search_identifier);
     bind_ctrl_and_cmnd(context, 'T', list_all_locations_of_identifier);
     bind_ctrl_and_cmnd(context, 'u', to_uppercase);
     bind_ctrl_and_cmnd(context, 'v', paste_and_indent);
     bind(context, 'v', MDFR_ALT, toggle_virtual_whitespace);
     bind_ctrl_and_cmnd(context, 'V', paste_next_and_indent);
     bind_ctrl_and_cmnd(context, 'x', cut);
     bind_ctrl_and_cmnd(context, 'y', redo);
     bind_ctrl_and_cmnd(context, 'z', undo);
     
     bind_ctrl_and_cmnd(context, '2', decrease_line_wrap);
     bind_ctrl_and_cmnd(context, '3', increase_line_wrap);
     
     bind_ctrl_and_cmnd(context, '?', toggle_show_whitespace);
     bind_ctrl_and_cmnd(context, '~', clean_all_lines);
     bind(context, '\n', MDFR_NONE, newline_or_goto_position);
     bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel);
     bind(context, ' ', MDFR_SHIFT, write_character);
     
     end_map(context);
     
     // NOTE(allen|a4.0.22): CODE
     begin_map(context, default_code_map);
     
     inherit_map(context, mapid_file);
     
     bind_ctrl_and_cmnd(context, key_right, seek_alphanumeric_or_camel_right);
     bind_ctrl_and_cmnd(context, key_left, seek_alphanumeric_or_camel_left);
     
     bind(context, '\n', MDFR_NONE, write_and_auto_tab);
     bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
     bind(context, '}', MDFR_NONE, write_and_auto_tab);
     bind(context, ')', MDFR_NONE, write_and_auto_tab);
     bind(context, ']', MDFR_NONE, write_and_auto_tab);
     bind(context, ';', MDFR_NONE, write_and_auto_tab);
     bind(context, '#', MDFR_NONE, write_and_auto_tab);
     
     bind(context, '\t', MDFR_NONE, word_complete);
     bind_ctrl_and_cmnd(context, '\t', auto_tab_range);
     bind(context, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
     
     bind(context, 'h', MDFR_ALT, write_hack);
     bind(context, 'r', MDFR_ALT, write_block);
     bind(context, 't', MDFR_ALT, write_todo);
     bind(context, 'y', MDFR_ALT, write_note);
     bind_ctrl_and_cmnd(context, '[', open_long_braces);
     bind_ctrl_and_cmnd(context, '{', open_long_braces_semicolon);
     bind_ctrl_and_cmnd(context, '}', open_long_braces_break);
     bind(context, 'i', MDFR_ALT, if0_off);
     bind(context, '1', MDFR_ALT, open_file_in_quotes);
     bind(context, '2', MDFR_ALT, open_matching_file_cpp);
     bind_ctrl_and_cmnd(context, '0', write_zero_struct);
     bind_ctrl_and_cmnd(context, 'I', list_all_functions_current_buffer);
     
     end_map(context);
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
     bind(context, key_mouse_right, MDFR_NONE, click_set_mark);
     
     bind(context, key_mouse_left_release, MDFR_NONE, click_set_mark);
     
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
     
     bind(context, key_up, MDFR_CTRL, move_up_10);
     bind(context, key_down, MDFR_CTRL, move_down_10);
     
     bind(context, key_back, MDFR_CMND, backspace_word);
     bind(context, key_del, MDFR_CMND, delete_word);
     bind(context, key_back, MDFR_CTRL, snipe_token_or_word);
     bind(context, key_del, MDFR_CTRL, snipe_token_or_word_right);
     
     bind(context, ' ', MDFR_CMND, set_mark);
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
     
     