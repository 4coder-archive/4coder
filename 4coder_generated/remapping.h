#if defined(CUSTOM_COMMAND_SIG)
void fill_keys_default(Bind_Helper *context){
begin_map(context, mapid_global);
bind(context, ',', MDFR_CTRL, change_active_panel);
bind(context, '<', MDFR_CTRL, change_active_panel_backwards);
bind(context, 'n', MDFR_CTRL, interactive_new);
bind(context, 'o', MDFR_CTRL, interactive_open_or_new);
bind(context, 'o', MDFR_ALT, open_in_other);
bind(context, 'k', MDFR_CTRL, interactive_kill_buffer);
bind(context, 'i', MDFR_CTRL, interactive_switch_buffer);
bind(context, 'h', MDFR_CTRL, project_go_to_root_directory);
bind(context, 'S', MDFR_CTRL, save_all_dirty_buffers);
bind(context, '.', MDFR_ALT, change_to_build_panel);
bind(context, ',', MDFR_ALT, close_build_panel);
bind(context, 'n', MDFR_ALT, goto_next_jump_no_skips_sticky);
bind(context, 'N', MDFR_ALT, goto_prev_jump_no_skips_sticky);
bind(context, 'M', MDFR_ALT, goto_first_jump_sticky);
bind(context, 'm', MDFR_ALT, build_in_build_panel);
bind(context, 'b', MDFR_ALT, toggle_filebar);
bind(context, 'z', MDFR_ALT, execute_any_cli);
bind(context, 'Z', MDFR_ALT, execute_previous_cli);
bind(context, 'x', MDFR_ALT, command_lister);
bind(context, 'X', MDFR_ALT, project_command_lister);
bind(context, 'I', MDFR_CTRL, list_all_functions_current_buffer_lister);
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
bind(context, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
end_map(context);
begin_map(context, mapid_file);
bind_vanilla_keys(context, write_character);
bind(context, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
bind(context, key_click_activate_view, MDFR_NONE, click_set_cursor_and_mark);
bind(context, key_mouse_left_release, MDFR_NONE, click_set_cursor);
bind(context, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
bind(context, key_del, MDFR_NONE, delete_char);
bind(context, key_del, MDFR_SHIFT, delete_char);
bind(context, key_back, MDFR_NONE, backspace_char);
bind(context, key_back, MDFR_SHIFT, backspace_char);
bind(context, key_up, MDFR_NONE, move_up);
bind(context, key_down, MDFR_NONE, move_down);
bind(context, key_left, MDFR_NONE, move_left);
bind(context, key_right, MDFR_NONE, move_right);
bind(context, key_up, MDFR_SHIFT, move_up);
bind(context, key_down, MDFR_SHIFT, move_down);
bind(context, key_left, MDFR_SHIFT, move_left);
bind(context, key_right, MDFR_SHIFT, move_right);
bind(context, 'k', MDFR_ALT, move_up);
bind(context, 'j', MDFR_ALT, move_down);
bind(context, 'h', MDFR_ALT, move_left);
bind(context, 'l', MDFR_ALT, move_right);
bind(context, key_end, MDFR_NONE, seek_end_of_line);
bind(context, key_home, MDFR_NONE, seek_beginning_of_line);
bind(context, key_page_up, MDFR_CTRL, goto_beginning_of_file);
bind(context, key_page_down, MDFR_CTRL, goto_end_of_file);
bind(context, key_page_up, MDFR_NONE, page_up);
bind(context, key_page_down, MDFR_NONE, page_down);
bind(context, key_end, MDFR_SHIFT, seek_end_of_line);
bind(context, key_home, MDFR_SHIFT, seek_beginning_of_line);
bind(context, key_page_up, MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
bind(context, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
bind(context, key_page_up, MDFR_SHIFT, page_up);
bind(context, key_page_down, MDFR_SHIFT, page_down);
bind(context, key_up, MDFR_CTRL, seek_whitespace_up_end_line);
bind(context, key_down, MDFR_CTRL, seek_whitespace_down_end_line);
bind(context, key_left, MDFR_CTRL, seek_whitespace_left);
bind(context, key_right, MDFR_CTRL, seek_whitespace_right);
bind(context, key_up, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_up_end_line);
bind(context, key_down, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_down_end_line);
bind(context, key_left, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_left);
bind(context, key_right, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_right);
bind(context, 'k', MDFR_CTRL|MDFR_ALT, seek_whitespace_up_end_line);
bind(context, 'j', MDFR_CTRL|MDFR_ALT, seek_whitespace_down_end_line);
bind(context, 'h', MDFR_CTRL|MDFR_ALT, seek_whitespace_left);
bind(context, 'l', MDFR_CTRL|MDFR_ALT, seek_whitespace_right);
bind(context, key_up, MDFR_ALT, move_line_up);
bind(context, key_down, MDFR_ALT, move_line_down);
bind(context, 'K', MDFR_ALT, move_line_up);
bind(context, 'J', MDFR_ALT, move_line_down);
bind(context, key_back, MDFR_CTRL, backspace_word);
bind(context, key_del, MDFR_CTRL, delete_word);
bind(context, key_back, MDFR_ALT, snipe_token_or_word);
bind(context, key_del, MDFR_ALT, snipe_token_or_word_right);
bind(context, ' ', MDFR_CTRL, set_mark);
bind(context, 'a', MDFR_CTRL, replace_in_range);
bind(context, 'c', MDFR_CTRL, copy);
bind(context, 'd', MDFR_CTRL, delete_range);
bind(context, 'D', MDFR_CTRL, delete_line);
bind(context, 'e', MDFR_CTRL, center_view);
bind(context, 'E', MDFR_CTRL, left_adjust_view);
bind(context, 'f', MDFR_CTRL, search);
bind(context, 'F', MDFR_CTRL, list_all_locations);
bind(context, 'F', MDFR_ALT, list_all_substring_locations_case_insensitive);
bind(context, 'g', MDFR_CTRL, goto_line);
bind(context, 'G', MDFR_CTRL, list_all_locations_of_selection);
bind(context, 'j', MDFR_CTRL, snippet_lister);
bind(context, 'K', MDFR_CTRL, kill_buffer);
bind(context, 'L', MDFR_CTRL, duplicate_line);
bind(context, 'm', MDFR_CTRL, cursor_mark_swap);
bind(context, 'O', MDFR_CTRL, reopen);
bind(context, 'q', MDFR_CTRL, query_replace);
bind(context, 'Q', MDFR_CTRL, query_replace_identifier);
bind(context, 'q', MDFR_ALT, query_replace_selection);
bind(context, 'r', MDFR_CTRL, reverse_search);
bind(context, 's', MDFR_CTRL, save);
bind(context, 't', MDFR_CTRL, search_identifier);
bind(context, 'T', MDFR_CTRL, list_all_locations_of_identifier);
bind(context, 'v', MDFR_CTRL, paste_and_indent);
bind(context, 'V', MDFR_CTRL, paste_next_and_indent);
bind(context, 'x', MDFR_CTRL, cut);
bind(context, 'y', MDFR_CTRL, redo);
bind(context, 'z', MDFR_CTRL, undo);
bind(context, '1', MDFR_CTRL, view_buffer_other_panel);
bind(context, '2', MDFR_CTRL, swap_buffers_between_panels);
bind(context, '\n', MDFR_NONE, newline_or_goto_position_sticky);
bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
bind(context, '>', MDFR_CTRL, view_jump_list_with_lister);
bind(context, ' ', MDFR_SHIFT, write_character);
end_map(context);
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
bind(context, 'r', MDFR_ALT, write_block);
bind(context, 't', MDFR_ALT, write_todo);
bind(context, 'y', MDFR_ALT, write_note);
bind(context, 'D', MDFR_ALT, list_all_locations_of_type_definition);
bind(context, 'T', MDFR_ALT, list_all_locations_of_type_definition_of_identifier);
bind(context, '[', MDFR_CTRL, open_long_braces);
bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
bind(context, '}', MDFR_CTRL, open_long_braces_break);
bind(context, '[', MDFR_ALT, select_surrounding_scope);
bind(context, ']', MDFR_ALT, select_prev_scope_absolute);
bind(context, '\'', MDFR_ALT, select_next_scope_absolute);
bind(context, '/', MDFR_ALT, place_in_scope);
bind(context, '-', MDFR_ALT, delete_current_scope);
bind(context, '}', MDFR_ALT, scope_absorb_down);
bind(context, 'i', MDFR_ALT, if0_off);
bind(context, '1', MDFR_ALT, open_file_in_quotes);
bind(context, '2', MDFR_ALT, open_matching_file_cpp);
bind(context, '0', MDFR_CTRL, write_zero_struct);
end_map(context);
begin_map(context, default_lister_ui_map);
bind_vanilla_keys(context, lister__write_character);
bind(context, key_esc, MDFR_NONE, lister__quit);
bind(context, '\n', MDFR_NONE, lister__activate);
bind(context, '\t', MDFR_NONE, lister__activate);
bind(context, key_back, MDFR_NONE, lister__backspace_text_field);
bind(context, key_up, MDFR_NONE, lister__move_up);
bind(context, 'k', MDFR_ALT, lister__move_up);
bind(context, key_page_up, MDFR_NONE, lister__move_up);
bind(context, key_down, MDFR_NONE, lister__move_down);
bind(context, 'j', MDFR_ALT, lister__move_up);
bind(context, key_page_down, MDFR_NONE, lister__move_down);
bind(context, key_mouse_wheel, MDFR_NONE, lister__wheel_scroll);
bind(context, key_mouse_left, MDFR_NONE, lister__mouse_press);
bind(context, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
bind(context, key_mouse_move, MDFR_NONE, lister__repaint);
bind(context, key_animate, MDFR_NONE, lister__repaint);
end_map(context);
}
void fill_keys_mac_default(Bind_Helper *context){
begin_map(context, mapid_global);
bind(context, ',', MDFR_CMND, change_active_panel);
bind(context, '<', MDFR_CMND, change_active_panel_backwards);
bind(context, 'n', MDFR_CMND, interactive_new);
bind(context, 'o', MDFR_CMND, interactive_open_or_new);
bind(context, 'o', MDFR_CTRL, open_in_other);
bind(context, 'k', MDFR_CMND, interactive_kill_buffer);
bind(context, 'i', MDFR_CMND, interactive_switch_buffer);
bind(context, 'h', MDFR_CMND, project_go_to_root_directory);
bind(context, 'S', MDFR_CMND, save_all_dirty_buffers);
bind(context, '.', MDFR_CTRL, change_to_build_panel);
bind(context, ',', MDFR_CTRL, close_build_panel);
bind(context, 'n', MDFR_CTRL, goto_next_jump_sticky);
bind(context, 'N', MDFR_CTRL, goto_prev_jump_sticky);
bind(context, 'M', MDFR_CTRL, goto_first_jump_sticky);
bind(context, 'm', MDFR_CTRL, build_in_build_panel);
bind(context, 'b', MDFR_CTRL, toggle_filebar);
bind(context, 'z', MDFR_CTRL, execute_any_cli);
bind(context, 'Z', MDFR_CTRL, execute_previous_cli);
bind(context, 'x', MDFR_CTRL, command_lister);
bind(context, 'X', MDFR_CTRL, project_command_lister);
bind(context, 'I', MDFR_CMND, list_all_functions_current_buffer_lister);
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
bind(context, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
end_map(context);
begin_map(context, mapid_file);
bind_vanilla_keys(context, write_character);
bind_vanilla_keys(context, MDFR_ALT, write_character);
bind(context, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
bind(context, key_click_activate_view, MDFR_NONE, click_set_cursor_and_mark);
bind(context, key_mouse_left_release, MDFR_NONE, click_set_cursor);
bind(context, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
bind(context, key_del, MDFR_NONE, delete_char);
bind(context, key_del, MDFR_SHIFT, delete_char);
bind(context, key_back, MDFR_NONE, backspace_char);
bind(context, key_back, MDFR_SHIFT, backspace_char);
bind(context, key_up, MDFR_NONE, move_up);
bind(context, key_down, MDFR_NONE, move_down);
bind(context, key_left, MDFR_NONE, move_left);
bind(context, key_right, MDFR_NONE, move_right);
bind(context, key_up, MDFR_SHIFT, move_up);
bind(context, key_down, MDFR_SHIFT, move_down);
bind(context, key_left, MDFR_SHIFT, move_left);
bind(context, key_right, MDFR_SHIFT, move_right);
bind(context, 'k', MDFR_ALT, move_up);
bind(context, 'j', MDFR_ALT, move_down);
bind(context, 'h', MDFR_ALT, move_left);
bind(context, 'l', MDFR_ALT, move_right);
bind(context, key_end, MDFR_NONE, seek_end_of_line);
bind(context, key_home, MDFR_NONE, seek_beginning_of_line);
bind(context, key_page_up, MDFR_CTRL, goto_beginning_of_file);
bind(context, key_page_down, MDFR_CTRL, goto_end_of_file);
bind(context, key_page_up, MDFR_NONE, page_up);
bind(context, key_page_down, MDFR_NONE, page_down);
bind(context, key_end, MDFR_SHIFT, seek_end_of_line);
bind(context, key_home, MDFR_SHIFT, seek_beginning_of_line);
bind(context, key_page_up, MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
bind(context, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
bind(context, key_page_up, MDFR_SHIFT, page_up);
bind(context, key_page_down, MDFR_SHIFT, page_down);
bind(context, key_up, MDFR_CMND, seek_whitespace_up_end_line);
bind(context, key_down, MDFR_CMND, seek_whitespace_down_end_line);
bind(context, key_left, MDFR_CMND, seek_whitespace_left);
bind(context, key_right, MDFR_CMND, seek_whitespace_right);
bind(context, key_up, MDFR_CMND|MDFR_SHIFT, seek_whitespace_up_end_line);
bind(context, key_down, MDFR_CMND|MDFR_SHIFT, seek_whitespace_down_end_line);
bind(context, key_left, MDFR_CMND|MDFR_SHIFT, seek_whitespace_left);
bind(context, key_right, MDFR_CMND|MDFR_SHIFT, seek_whitespace_right);
bind(context, 'k', MDFR_CTRL|MDFR_CMND, seek_whitespace_up_end_line);
bind(context, 'j', MDFR_CTRL|MDFR_CMND, seek_whitespace_down_end_line);
bind(context, 'h', MDFR_CTRL|MDFR_CMND, seek_whitespace_left);
bind(context, 'l', MDFR_CTRL|MDFR_CMND, seek_whitespace_right);
bind(context, key_up, MDFR_ALT, move_line_up);
bind(context, key_down, MDFR_ALT, move_line_down);
bind(context, 'k', MDFR_CTRL|MDFR_ALT, move_line_up);
bind(context, 'j', MDFR_CTRL|MDFR_ALT, move_line_down);
bind(context, key_back, MDFR_CMND, backspace_word);
bind(context, key_del, MDFR_CMND, delete_word);
bind(context, key_back, MDFR_CTRL, snipe_token_or_word);
bind(context, key_del, MDFR_CTRL, snipe_token_or_word_right);
bind(context, '/', MDFR_CMND, set_mark);
bind(context, 'a', MDFR_CMND, replace_in_range);
bind(context, 'c', MDFR_CMND, copy);
bind(context, 'd', MDFR_CMND, delete_range);
bind(context, 'D', MDFR_CMND, delete_line);
bind(context, 'e', MDFR_CMND, center_view);
bind(context, 'E', MDFR_CMND, left_adjust_view);
bind(context, 'f', MDFR_CMND, search);
bind(context, 'F', MDFR_CMND, list_all_locations);
bind(context, 'F', MDFR_CTRL, list_all_substring_locations_case_insensitive);
bind(context, 'g', MDFR_CMND, goto_line);
bind(context, 'G', MDFR_CMND, list_all_locations_of_selection);
bind(context, 'K', MDFR_CMND, kill_buffer);
bind(context, 'L', MDFR_CMND, duplicate_line);
bind(context, 'm', MDFR_CMND, cursor_mark_swap);
bind(context, 'O', MDFR_CMND, reopen);
bind(context, 'q', MDFR_CMND, query_replace);
bind(context, 'Q', MDFR_CMND, query_replace_identifier);
bind(context, 'r', MDFR_CMND, reverse_search);
bind(context, 's', MDFR_CMND, save);
bind(context, 't', MDFR_CMND, search_identifier);
bind(context, 'T', MDFR_CMND, list_all_locations_of_identifier);
bind(context, 'v', MDFR_CMND, paste_and_indent);
bind(context, 'V', MDFR_CMND, paste_next_and_indent);
bind(context, 'x', MDFR_CMND, cut);
bind(context, 'y', MDFR_CMND, redo);
bind(context, 'z', MDFR_CMND, undo);
bind(context, '1', MDFR_CMND, view_buffer_other_panel);
bind(context, '2', MDFR_CMND, swap_buffers_between_panels);
bind(context, '\n', MDFR_NONE, newline_or_goto_position_sticky);
bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
bind(context, '>', MDFR_CMND, view_jump_list_with_lister);
bind(context, ' ', MDFR_SHIFT, write_character);
end_map(context);
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
bind(context, 'r', MDFR_CTRL, write_block);
bind(context, 't', MDFR_CTRL, write_todo);
bind(context, 'y', MDFR_CTRL, write_note);
bind(context, 'D', MDFR_CTRL, list_all_locations_of_type_definition);
bind(context, 'T', MDFR_CTRL, list_all_locations_of_type_definition_of_identifier);
bind(context, '[', MDFR_CMND, open_long_braces);
bind(context, '{', MDFR_CMND, open_long_braces_semicolon);
bind(context, '}', MDFR_CMND, open_long_braces_break);
bind(context, '[', MDFR_CTRL, select_surrounding_scope);
bind(context, ']', MDFR_CTRL, select_prev_scope_absolute);
bind(context, '\'', MDFR_CTRL, select_next_scope_absolute);
bind(context, '/', MDFR_CTRL, place_in_scope);
bind(context, '-', MDFR_CTRL, delete_current_scope);
bind(context, '}', MDFR_CTRL, scope_absorb_down);
bind(context, 'i', MDFR_CTRL, if0_off);
bind(context, '1', MDFR_CTRL, open_file_in_quotes);
bind(context, '2', MDFR_CTRL, open_matching_file_cpp);
bind(context, '0', MDFR_CMND, write_zero_struct);
end_map(context);
begin_map(context, default_lister_ui_map);
bind_vanilla_keys(context, lister__write_character);
bind(context, key_esc, MDFR_NONE, lister__quit);
bind(context, '\n', MDFR_NONE, lister__activate);
bind(context, '\t', MDFR_NONE, lister__activate);
bind(context, key_back, MDFR_NONE, lister__backspace_text_field);
bind(context, key_up, MDFR_NONE, lister__move_up);
bind(context, key_page_up, MDFR_NONE, lister__move_up);
bind(context, key_down, MDFR_NONE, lister__move_down);
bind(context, key_page_down, MDFR_NONE, lister__move_down);
bind(context, key_mouse_wheel, MDFR_NONE, lister__wheel_scroll);
bind(context, key_mouse_left, MDFR_NONE, lister__mouse_press);
bind(context, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
bind(context, key_mouse_move, MDFR_NONE, lister__repaint);
bind(context, key_animate, MDFR_NONE, lister__repaint);
end_map(context);
}
#endif
#if defined(CUSTOM_COMMAND_SIG)
#define LINK_PROCS(x) x
#else
#define LINK_PROCS(x)
#endif
struct Meta_Key_Bind{
int32_t vanilla;
uint32_t keycode;
uint32_t modifiers;
char *command;
int32_t command_len;
LINK_PROCS(Custom_Command_Function *proc;)
};
struct Meta_Sub_Map{
char *name;
int32_t name_len;
char *description;
int32_t description_len;
char *parent;
int32_t parent_len;
Meta_Key_Bind *binds;
int32_t bind_count;
};
struct Meta_Mapping{
char *name;
int32_t name_len;
char *description;
int32_t description_len;
Meta_Sub_Map *sub_maps;
int32_t sub_map_count;
LINK_PROCS(void (*fill_keys_proc)(Bind_Helper *context);)
};
static Meta_Key_Bind fcoder_binds_for_default_mapid_global[39] = {
{0, 44, 1, "change_active_panel", 19, LINK_PROCS(change_active_panel)},
{0, 60, 1, "change_active_panel_backwards", 29, LINK_PROCS(change_active_panel_backwards)},
{0, 110, 1, "interactive_new", 15, LINK_PROCS(interactive_new)},
{0, 111, 1, "interactive_open_or_new", 23, LINK_PROCS(interactive_open_or_new)},
{0, 111, 2, "open_in_other", 13, LINK_PROCS(open_in_other)},
{0, 107, 1, "interactive_kill_buffer", 23, LINK_PROCS(interactive_kill_buffer)},
{0, 105, 1, "interactive_switch_buffer", 25, LINK_PROCS(interactive_switch_buffer)},
{0, 104, 1, "project_go_to_root_directory", 28, LINK_PROCS(project_go_to_root_directory)},
{0, 83, 1, "save_all_dirty_buffers", 22, LINK_PROCS(save_all_dirty_buffers)},
{0, 46, 2, "change_to_build_panel", 21, LINK_PROCS(change_to_build_panel)},
{0, 44, 2, "close_build_panel", 17, LINK_PROCS(close_build_panel)},
{0, 110, 2, "goto_next_jump_no_skips_sticky", 30, LINK_PROCS(goto_next_jump_no_skips_sticky)},
{0, 78, 2, "goto_prev_jump_no_skips_sticky", 30, LINK_PROCS(goto_prev_jump_no_skips_sticky)},
{0, 77, 2, "goto_first_jump_sticky", 22, LINK_PROCS(goto_first_jump_sticky)},
{0, 109, 2, "build_in_build_panel", 20, LINK_PROCS(build_in_build_panel)},
{0, 98, 2, "toggle_filebar", 14, LINK_PROCS(toggle_filebar)},
{0, 122, 2, "execute_any_cli", 15, LINK_PROCS(execute_any_cli)},
{0, 90, 2, "execute_previous_cli", 20, LINK_PROCS(execute_previous_cli)},
{0, 120, 2, "command_lister", 14, LINK_PROCS(command_lister)},
{0, 88, 2, "project_command_lister", 22, LINK_PROCS(project_command_lister)},
{0, 73, 1, "list_all_functions_current_buffer_lister", 40, LINK_PROCS(list_all_functions_current_buffer_lister)},
{0, 69, 2, "exit_4coder", 11, LINK_PROCS(exit_4coder)},
{0, 55317, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55318, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55319, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55320, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55321, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55322, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55323, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55324, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55325, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55326, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55327, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55328, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55329, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55330, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55331, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55332, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55312, 0, "mouse_wheel_scroll", 18, LINK_PROCS(mouse_wheel_scroll)},
};
static Meta_Key_Bind fcoder_binds_for_default_mapid_file[88] = {
{1, 0, 0, "write_character", 15, LINK_PROCS(write_character)},
{0, 55308, 0, "click_set_cursor_and_mark", 25, LINK_PROCS(click_set_cursor_and_mark)},
{0, 55315, 0, "click_set_cursor_and_mark", 25, LINK_PROCS(click_set_cursor_and_mark)},
{0, 55310, 0, "click_set_cursor", 16, LINK_PROCS(click_set_cursor)},
{0, 55313, 0, "click_set_cursor_if_lbutton", 27, LINK_PROCS(click_set_cursor_if_lbutton)},
{0, 55301, 0, "delete_char", 11, LINK_PROCS(delete_char)},
{0, 55301, 8, "delete_char", 11, LINK_PROCS(delete_char)},
{0, 55296, 0, "backspace_char", 14, LINK_PROCS(backspace_char)},
{0, 55296, 8, "backspace_char", 14, LINK_PROCS(backspace_char)},
{0, 55297, 0, "move_up", 7, LINK_PROCS(move_up)},
{0, 55298, 0, "move_down", 9, LINK_PROCS(move_down)},
{0, 55299, 0, "move_left", 9, LINK_PROCS(move_left)},
{0, 55300, 0, "move_right", 10, LINK_PROCS(move_right)},
{0, 55297, 8, "move_up", 7, LINK_PROCS(move_up)},
{0, 55298, 8, "move_down", 9, LINK_PROCS(move_down)},
{0, 55299, 8, "move_left", 9, LINK_PROCS(move_left)},
{0, 55300, 8, "move_right", 10, LINK_PROCS(move_right)},
{0, 107, 2, "move_up", 7, LINK_PROCS(move_up)},
{0, 106, 2, "move_down", 9, LINK_PROCS(move_down)},
{0, 104, 2, "move_left", 9, LINK_PROCS(move_left)},
{0, 108, 2, "move_right", 10, LINK_PROCS(move_right)},
{0, 55304, 0, "seek_end_of_line", 16, LINK_PROCS(seek_end_of_line)},
{0, 55303, 0, "seek_beginning_of_line", 22, LINK_PROCS(seek_beginning_of_line)},
{0, 55305, 1, "goto_beginning_of_file", 22, LINK_PROCS(goto_beginning_of_file)},
{0, 55306, 1, "goto_end_of_file", 16, LINK_PROCS(goto_end_of_file)},
{0, 55305, 0, "page_up", 7, LINK_PROCS(page_up)},
{0, 55306, 0, "page_down", 9, LINK_PROCS(page_down)},
{0, 55304, 8, "seek_end_of_line", 16, LINK_PROCS(seek_end_of_line)},
{0, 55303, 8, "seek_beginning_of_line", 22, LINK_PROCS(seek_beginning_of_line)},
{0, 55305, 9, "goto_beginning_of_file", 22, LINK_PROCS(goto_beginning_of_file)},
{0, 55306, 9, "goto_end_of_file", 16, LINK_PROCS(goto_end_of_file)},
{0, 55305, 8, "page_up", 7, LINK_PROCS(page_up)},
{0, 55306, 8, "page_down", 9, LINK_PROCS(page_down)},
{0, 55297, 1, "seek_whitespace_up_end_line", 27, LINK_PROCS(seek_whitespace_up_end_line)},
{0, 55298, 1, "seek_whitespace_down_end_line", 29, LINK_PROCS(seek_whitespace_down_end_line)},
{0, 55299, 1, "seek_whitespace_left", 20, LINK_PROCS(seek_whitespace_left)},
{0, 55300, 1, "seek_whitespace_right", 21, LINK_PROCS(seek_whitespace_right)},
{0, 55297, 9, "seek_whitespace_up_end_line", 27, LINK_PROCS(seek_whitespace_up_end_line)},
{0, 55298, 9, "seek_whitespace_down_end_line", 29, LINK_PROCS(seek_whitespace_down_end_line)},
{0, 55299, 9, "seek_whitespace_left", 20, LINK_PROCS(seek_whitespace_left)},
{0, 55300, 9, "seek_whitespace_right", 21, LINK_PROCS(seek_whitespace_right)},
{0, 107, 3, "seek_whitespace_up_end_line", 27, LINK_PROCS(seek_whitespace_up_end_line)},
{0, 106, 3, "seek_whitespace_down_end_line", 29, LINK_PROCS(seek_whitespace_down_end_line)},
{0, 104, 3, "seek_whitespace_left", 20, LINK_PROCS(seek_whitespace_left)},
{0, 108, 3, "seek_whitespace_right", 21, LINK_PROCS(seek_whitespace_right)},
{0, 55297, 2, "move_line_up", 12, LINK_PROCS(move_line_up)},
{0, 55298, 2, "move_line_down", 14, LINK_PROCS(move_line_down)},
{0, 75, 2, "move_line_up", 12, LINK_PROCS(move_line_up)},
{0, 74, 2, "move_line_down", 14, LINK_PROCS(move_line_down)},
{0, 55296, 1, "backspace_word", 14, LINK_PROCS(backspace_word)},
{0, 55301, 1, "delete_word", 11, LINK_PROCS(delete_word)},
{0, 55296, 2, "snipe_token_or_word", 19, LINK_PROCS(snipe_token_or_word)},
{0, 55301, 2, "snipe_token_or_word_right", 25, LINK_PROCS(snipe_token_or_word_right)},
{0, 32, 1, "set_mark", 8, LINK_PROCS(set_mark)},
{0, 97, 1, "replace_in_range", 16, LINK_PROCS(replace_in_range)},
{0, 99, 1, "copy", 4, LINK_PROCS(copy)},
{0, 100, 1, "delete_range", 12, LINK_PROCS(delete_range)},
{0, 68, 1, "delete_line", 11, LINK_PROCS(delete_line)},
{0, 101, 1, "center_view", 11, LINK_PROCS(center_view)},
{0, 69, 1, "left_adjust_view", 16, LINK_PROCS(left_adjust_view)},
{0, 102, 1, "search", 6, LINK_PROCS(search)},
{0, 70, 1, "list_all_locations", 18, LINK_PROCS(list_all_locations)},
{0, 70, 2, "list_all_substring_locations_case_insensitive", 45, LINK_PROCS(list_all_substring_locations_case_insensitive)},
{0, 103, 1, "goto_line", 9, LINK_PROCS(goto_line)},
{0, 71, 1, "list_all_locations_of_selection", 31, LINK_PROCS(list_all_locations_of_selection)},
{0, 106, 1, "snippet_lister", 14, LINK_PROCS(snippet_lister)},
{0, 75, 1, "kill_buffer", 11, LINK_PROCS(kill_buffer)},
{0, 76, 1, "duplicate_line", 14, LINK_PROCS(duplicate_line)},
{0, 109, 1, "cursor_mark_swap", 16, LINK_PROCS(cursor_mark_swap)},
{0, 79, 1, "reopen", 6, LINK_PROCS(reopen)},
{0, 113, 1, "query_replace", 13, LINK_PROCS(query_replace)},
{0, 81, 1, "query_replace_identifier", 24, LINK_PROCS(query_replace_identifier)},
{0, 113, 2, "query_replace_selection", 23, LINK_PROCS(query_replace_selection)},
{0, 114, 1, "reverse_search", 14, LINK_PROCS(reverse_search)},
{0, 115, 1, "save", 4, LINK_PROCS(save)},
{0, 116, 1, "search_identifier", 17, LINK_PROCS(search_identifier)},
{0, 84, 1, "list_all_locations_of_identifier", 32, LINK_PROCS(list_all_locations_of_identifier)},
{0, 118, 1, "paste_and_indent", 16, LINK_PROCS(paste_and_indent)},
{0, 86, 1, "paste_next_and_indent", 21, LINK_PROCS(paste_next_and_indent)},
{0, 120, 1, "cut", 3, LINK_PROCS(cut)},
{0, 121, 1, "redo", 4, LINK_PROCS(redo)},
{0, 122, 1, "undo", 4, LINK_PROCS(undo)},
{0, 49, 1, "view_buffer_other_panel", 23, LINK_PROCS(view_buffer_other_panel)},
{0, 50, 1, "swap_buffers_between_panels", 27, LINK_PROCS(swap_buffers_between_panels)},
{0, 10, 0, "newline_or_goto_position_sticky", 31, LINK_PROCS(newline_or_goto_position_sticky)},
{0, 10, 8, "newline_or_goto_position_same_panel_sticky", 42, LINK_PROCS(newline_or_goto_position_same_panel_sticky)},
{0, 62, 1, "view_jump_list_with_lister", 26, LINK_PROCS(view_jump_list_with_lister)},
{0, 32, 8, "write_character", 15, LINK_PROCS(write_character)},
};
static Meta_Key_Bind fcoder_binds_for_default_default_code_map[30] = {
{0, 55300, 1, "seek_alphanumeric_or_camel_right", 32, LINK_PROCS(seek_alphanumeric_or_camel_right)},
{0, 55299, 1, "seek_alphanumeric_or_camel_left", 31, LINK_PROCS(seek_alphanumeric_or_camel_left)},
{0, 10, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 10, 8, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 125, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 41, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 93, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 59, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 35, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 9, 0, "word_complete", 13, LINK_PROCS(word_complete)},
{0, 9, 1, "auto_tab_range", 14, LINK_PROCS(auto_tab_range)},
{0, 9, 8, "auto_tab_line_at_cursor", 23, LINK_PROCS(auto_tab_line_at_cursor)},
{0, 114, 2, "write_block", 11, LINK_PROCS(write_block)},
{0, 116, 2, "write_todo", 10, LINK_PROCS(write_todo)},
{0, 121, 2, "write_note", 10, LINK_PROCS(write_note)},
{0, 68, 2, "list_all_locations_of_type_definition", 37, LINK_PROCS(list_all_locations_of_type_definition)},
{0, 84, 2, "list_all_locations_of_type_definition_of_identifier", 51, LINK_PROCS(list_all_locations_of_type_definition_of_identifier)},
{0, 91, 1, "open_long_braces", 16, LINK_PROCS(open_long_braces)},
{0, 123, 1, "open_long_braces_semicolon", 26, LINK_PROCS(open_long_braces_semicolon)},
{0, 125, 1, "open_long_braces_break", 22, LINK_PROCS(open_long_braces_break)},
{0, 91, 2, "select_surrounding_scope", 24, LINK_PROCS(select_surrounding_scope)},
{0, 93, 2, "select_prev_scope_absolute", 26, LINK_PROCS(select_prev_scope_absolute)},
{0, 39, 2, "select_next_scope_absolute", 26, LINK_PROCS(select_next_scope_absolute)},
{0, 47, 2, "place_in_scope", 14, LINK_PROCS(place_in_scope)},
{0, 45, 2, "delete_current_scope", 20, LINK_PROCS(delete_current_scope)},
{0, 125, 2, "scope_absorb_down", 17, LINK_PROCS(scope_absorb_down)},
{0, 105, 2, "if0_off", 7, LINK_PROCS(if0_off)},
{0, 49, 2, "open_file_in_quotes", 19, LINK_PROCS(open_file_in_quotes)},
{0, 50, 2, "open_matching_file_cpp", 22, LINK_PROCS(open_matching_file_cpp)},
{0, 48, 1, "write_zero_struct", 17, LINK_PROCS(write_zero_struct)},
};
static Meta_Key_Bind fcoder_binds_for_default_default_lister_ui_map[16] = {
{1, 0, 0, "lister__write_character", 23, LINK_PROCS(lister__write_character)},
{0, 55307, 0, "lister__quit", 12, LINK_PROCS(lister__quit)},
{0, 10, 0, "lister__activate", 16, LINK_PROCS(lister__activate)},
{0, 9, 0, "lister__activate", 16, LINK_PROCS(lister__activate)},
{0, 55296, 0, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
{0, 55297, 0, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
{0, 107, 2, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
{0, 55305, 0, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
{0, 55298, 0, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
{0, 106, 2, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
{0, 55306, 0, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
{0, 55312, 0, "lister__wheel_scroll", 20, LINK_PROCS(lister__wheel_scroll)},
{0, 55308, 0, "lister__mouse_press", 19, LINK_PROCS(lister__mouse_press)},
{0, 55310, 0, "lister__mouse_release", 21, LINK_PROCS(lister__mouse_release)},
{0, 55313, 0, "lister__repaint", 15, LINK_PROCS(lister__repaint)},
{0, 55314, 0, "lister__repaint", 15, LINK_PROCS(lister__repaint)},
};
static Meta_Sub_Map fcoder_submaps_for_default[4] = {
{"mapid_global", 12, "The following bindings apply in all situations.", 47, 0, 0, fcoder_binds_for_default_mapid_global, 39},
{"mapid_file", 10, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.", 145, 0, 0, fcoder_binds_for_default_mapid_file, 88},
{"default_code_map", 16, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.", 94, "mapid_file", 10, fcoder_binds_for_default_default_code_map, 30},
{"default_lister_ui_map", 21, "These commands apply in 'lister mode' such as when you open a file.", 67, 0, 0, fcoder_binds_for_default_default_lister_ui_map, 16},
};
static Meta_Key_Bind fcoder_binds_for_mac_default_mapid_global[39] = {
{0, 44, 4, "change_active_panel", 19, LINK_PROCS(change_active_panel)},
{0, 60, 4, "change_active_panel_backwards", 29, LINK_PROCS(change_active_panel_backwards)},
{0, 110, 4, "interactive_new", 15, LINK_PROCS(interactive_new)},
{0, 111, 4, "interactive_open_or_new", 23, LINK_PROCS(interactive_open_or_new)},
{0, 111, 1, "open_in_other", 13, LINK_PROCS(open_in_other)},
{0, 107, 4, "interactive_kill_buffer", 23, LINK_PROCS(interactive_kill_buffer)},
{0, 105, 4, "interactive_switch_buffer", 25, LINK_PROCS(interactive_switch_buffer)},
{0, 104, 4, "project_go_to_root_directory", 28, LINK_PROCS(project_go_to_root_directory)},
{0, 83, 4, "save_all_dirty_buffers", 22, LINK_PROCS(save_all_dirty_buffers)},
{0, 46, 1, "change_to_build_panel", 21, LINK_PROCS(change_to_build_panel)},
{0, 44, 1, "close_build_panel", 17, LINK_PROCS(close_build_panel)},
{0, 110, 1, "goto_next_jump_sticky", 21, LINK_PROCS(goto_next_jump_sticky)},
{0, 78, 1, "goto_prev_jump_sticky", 21, LINK_PROCS(goto_prev_jump_sticky)},
{0, 77, 1, "goto_first_jump_sticky", 22, LINK_PROCS(goto_first_jump_sticky)},
{0, 109, 1, "build_in_build_panel", 20, LINK_PROCS(build_in_build_panel)},
{0, 98, 1, "toggle_filebar", 14, LINK_PROCS(toggle_filebar)},
{0, 122, 1, "execute_any_cli", 15, LINK_PROCS(execute_any_cli)},
{0, 90, 1, "execute_previous_cli", 20, LINK_PROCS(execute_previous_cli)},
{0, 120, 1, "command_lister", 14, LINK_PROCS(command_lister)},
{0, 88, 1, "project_command_lister", 22, LINK_PROCS(project_command_lister)},
{0, 73, 4, "list_all_functions_current_buffer_lister", 40, LINK_PROCS(list_all_functions_current_buffer_lister)},
{0, 69, 1, "exit_4coder", 11, LINK_PROCS(exit_4coder)},
{0, 55317, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55318, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55319, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55320, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55321, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55322, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55323, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55324, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55325, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55326, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55327, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55328, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55329, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55330, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55331, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55332, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
{0, 55312, 0, "mouse_wheel_scroll", 18, LINK_PROCS(mouse_wheel_scroll)},
};
static Meta_Key_Bind fcoder_binds_for_mac_default_mapid_file[87] = {
{1, 0, 0, "write_character", 15, LINK_PROCS(write_character)},
{1, 0, 2, "write_character", 15, LINK_PROCS(write_character)},
{0, 55308, 0, "click_set_cursor_and_mark", 25, LINK_PROCS(click_set_cursor_and_mark)},
{0, 55315, 0, "click_set_cursor_and_mark", 25, LINK_PROCS(click_set_cursor_and_mark)},
{0, 55310, 0, "click_set_cursor", 16, LINK_PROCS(click_set_cursor)},
{0, 55313, 0, "click_set_cursor_if_lbutton", 27, LINK_PROCS(click_set_cursor_if_lbutton)},
{0, 55301, 0, "delete_char", 11, LINK_PROCS(delete_char)},
{0, 55301, 8, "delete_char", 11, LINK_PROCS(delete_char)},
{0, 55296, 0, "backspace_char", 14, LINK_PROCS(backspace_char)},
{0, 55296, 8, "backspace_char", 14, LINK_PROCS(backspace_char)},
{0, 55297, 0, "move_up", 7, LINK_PROCS(move_up)},
{0, 55298, 0, "move_down", 9, LINK_PROCS(move_down)},
{0, 55299, 0, "move_left", 9, LINK_PROCS(move_left)},
{0, 55300, 0, "move_right", 10, LINK_PROCS(move_right)},
{0, 55297, 8, "move_up", 7, LINK_PROCS(move_up)},
{0, 55298, 8, "move_down", 9, LINK_PROCS(move_down)},
{0, 55299, 8, "move_left", 9, LINK_PROCS(move_left)},
{0, 55300, 8, "move_right", 10, LINK_PROCS(move_right)},
{0, 107, 2, "move_up", 7, LINK_PROCS(move_up)},
{0, 106, 2, "move_down", 9, LINK_PROCS(move_down)},
{0, 104, 2, "move_left", 9, LINK_PROCS(move_left)},
{0, 108, 2, "move_right", 10, LINK_PROCS(move_right)},
{0, 55304, 0, "seek_end_of_line", 16, LINK_PROCS(seek_end_of_line)},
{0, 55303, 0, "seek_beginning_of_line", 22, LINK_PROCS(seek_beginning_of_line)},
{0, 55305, 1, "goto_beginning_of_file", 22, LINK_PROCS(goto_beginning_of_file)},
{0, 55306, 1, "goto_end_of_file", 16, LINK_PROCS(goto_end_of_file)},
{0, 55305, 0, "page_up", 7, LINK_PROCS(page_up)},
{0, 55306, 0, "page_down", 9, LINK_PROCS(page_down)},
{0, 55304, 8, "seek_end_of_line", 16, LINK_PROCS(seek_end_of_line)},
{0, 55303, 8, "seek_beginning_of_line", 22, LINK_PROCS(seek_beginning_of_line)},
{0, 55305, 9, "goto_beginning_of_file", 22, LINK_PROCS(goto_beginning_of_file)},
{0, 55306, 9, "goto_end_of_file", 16, LINK_PROCS(goto_end_of_file)},
{0, 55305, 8, "page_up", 7, LINK_PROCS(page_up)},
{0, 55306, 8, "page_down", 9, LINK_PROCS(page_down)},
{0, 55297, 4, "seek_whitespace_up_end_line", 27, LINK_PROCS(seek_whitespace_up_end_line)},
{0, 55298, 4, "seek_whitespace_down_end_line", 29, LINK_PROCS(seek_whitespace_down_end_line)},
{0, 55299, 4, "seek_whitespace_left", 20, LINK_PROCS(seek_whitespace_left)},
{0, 55300, 4, "seek_whitespace_right", 21, LINK_PROCS(seek_whitespace_right)},
{0, 55297, 12, "seek_whitespace_up_end_line", 27, LINK_PROCS(seek_whitespace_up_end_line)},
{0, 55298, 12, "seek_whitespace_down_end_line", 29, LINK_PROCS(seek_whitespace_down_end_line)},
{0, 55299, 12, "seek_whitespace_left", 20, LINK_PROCS(seek_whitespace_left)},
{0, 55300, 12, "seek_whitespace_right", 21, LINK_PROCS(seek_whitespace_right)},
{0, 107, 5, "seek_whitespace_up_end_line", 27, LINK_PROCS(seek_whitespace_up_end_line)},
{0, 106, 5, "seek_whitespace_down_end_line", 29, LINK_PROCS(seek_whitespace_down_end_line)},
{0, 104, 5, "seek_whitespace_left", 20, LINK_PROCS(seek_whitespace_left)},
{0, 108, 5, "seek_whitespace_right", 21, LINK_PROCS(seek_whitespace_right)},
{0, 55297, 2, "move_line_up", 12, LINK_PROCS(move_line_up)},
{0, 55298, 2, "move_line_down", 14, LINK_PROCS(move_line_down)},
{0, 107, 3, "move_line_up", 12, LINK_PROCS(move_line_up)},
{0, 106, 3, "move_line_down", 14, LINK_PROCS(move_line_down)},
{0, 55296, 4, "backspace_word", 14, LINK_PROCS(backspace_word)},
{0, 55301, 4, "delete_word", 11, LINK_PROCS(delete_word)},
{0, 55296, 1, "snipe_token_or_word", 19, LINK_PROCS(snipe_token_or_word)},
{0, 55301, 1, "snipe_token_or_word_right", 25, LINK_PROCS(snipe_token_or_word_right)},
{0, 47, 4, "set_mark", 8, LINK_PROCS(set_mark)},
{0, 97, 4, "replace_in_range", 16, LINK_PROCS(replace_in_range)},
{0, 99, 4, "copy", 4, LINK_PROCS(copy)},
{0, 100, 4, "delete_range", 12, LINK_PROCS(delete_range)},
{0, 68, 4, "delete_line", 11, LINK_PROCS(delete_line)},
{0, 101, 4, "center_view", 11, LINK_PROCS(center_view)},
{0, 69, 4, "left_adjust_view", 16, LINK_PROCS(left_adjust_view)},
{0, 102, 4, "search", 6, LINK_PROCS(search)},
{0, 70, 4, "list_all_locations", 18, LINK_PROCS(list_all_locations)},
{0, 70, 1, "list_all_substring_locations_case_insensitive", 45, LINK_PROCS(list_all_substring_locations_case_insensitive)},
{0, 103, 4, "goto_line", 9, LINK_PROCS(goto_line)},
{0, 71, 4, "list_all_locations_of_selection", 31, LINK_PROCS(list_all_locations_of_selection)},
{0, 75, 4, "kill_buffer", 11, LINK_PROCS(kill_buffer)},
{0, 76, 4, "duplicate_line", 14, LINK_PROCS(duplicate_line)},
{0, 109, 4, "cursor_mark_swap", 16, LINK_PROCS(cursor_mark_swap)},
{0, 79, 4, "reopen", 6, LINK_PROCS(reopen)},
{0, 113, 4, "query_replace", 13, LINK_PROCS(query_replace)},
{0, 81, 4, "query_replace_identifier", 24, LINK_PROCS(query_replace_identifier)},
{0, 114, 4, "reverse_search", 14, LINK_PROCS(reverse_search)},
{0, 115, 4, "save", 4, LINK_PROCS(save)},
{0, 116, 4, "search_identifier", 17, LINK_PROCS(search_identifier)},
{0, 84, 4, "list_all_locations_of_identifier", 32, LINK_PROCS(list_all_locations_of_identifier)},
{0, 118, 4, "paste_and_indent", 16, LINK_PROCS(paste_and_indent)},
{0, 86, 4, "paste_next_and_indent", 21, LINK_PROCS(paste_next_and_indent)},
{0, 120, 4, "cut", 3, LINK_PROCS(cut)},
{0, 121, 4, "redo", 4, LINK_PROCS(redo)},
{0, 122, 4, "undo", 4, LINK_PROCS(undo)},
{0, 49, 4, "view_buffer_other_panel", 23, LINK_PROCS(view_buffer_other_panel)},
{0, 50, 4, "swap_buffers_between_panels", 27, LINK_PROCS(swap_buffers_between_panels)},
{0, 10, 0, "newline_or_goto_position_sticky", 31, LINK_PROCS(newline_or_goto_position_sticky)},
{0, 10, 8, "newline_or_goto_position_same_panel_sticky", 42, LINK_PROCS(newline_or_goto_position_same_panel_sticky)},
{0, 62, 4, "view_jump_list_with_lister", 26, LINK_PROCS(view_jump_list_with_lister)},
{0, 32, 8, "write_character", 15, LINK_PROCS(write_character)},
};
static Meta_Key_Bind fcoder_binds_for_mac_default_default_code_map[30] = {
{0, 55300, 4, "seek_alphanumeric_or_camel_right", 32, LINK_PROCS(seek_alphanumeric_or_camel_right)},
{0, 55299, 4, "seek_alphanumeric_or_camel_left", 31, LINK_PROCS(seek_alphanumeric_or_camel_left)},
{0, 10, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 10, 8, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 125, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 41, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 93, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 59, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 35, 0, "write_and_auto_tab", 18, LINK_PROCS(write_and_auto_tab)},
{0, 9, 0, "word_complete", 13, LINK_PROCS(word_complete)},
{0, 9, 4, "auto_tab_range", 14, LINK_PROCS(auto_tab_range)},
{0, 9, 8, "auto_tab_line_at_cursor", 23, LINK_PROCS(auto_tab_line_at_cursor)},
{0, 114, 1, "write_block", 11, LINK_PROCS(write_block)},
{0, 116, 1, "write_todo", 10, LINK_PROCS(write_todo)},
{0, 121, 1, "write_note", 10, LINK_PROCS(write_note)},
{0, 68, 1, "list_all_locations_of_type_definition", 37, LINK_PROCS(list_all_locations_of_type_definition)},
{0, 84, 1, "list_all_locations_of_type_definition_of_identifier", 51, LINK_PROCS(list_all_locations_of_type_definition_of_identifier)},
{0, 91, 4, "open_long_braces", 16, LINK_PROCS(open_long_braces)},
{0, 123, 4, "open_long_braces_semicolon", 26, LINK_PROCS(open_long_braces_semicolon)},
{0, 125, 4, "open_long_braces_break", 22, LINK_PROCS(open_long_braces_break)},
{0, 91, 1, "select_surrounding_scope", 24, LINK_PROCS(select_surrounding_scope)},
{0, 93, 1, "select_prev_scope_absolute", 26, LINK_PROCS(select_prev_scope_absolute)},
{0, 39, 1, "select_next_scope_absolute", 26, LINK_PROCS(select_next_scope_absolute)},
{0, 47, 1, "place_in_scope", 14, LINK_PROCS(place_in_scope)},
{0, 45, 1, "delete_current_scope", 20, LINK_PROCS(delete_current_scope)},
{0, 125, 1, "scope_absorb_down", 17, LINK_PROCS(scope_absorb_down)},
{0, 105, 1, "if0_off", 7, LINK_PROCS(if0_off)},
{0, 49, 1, "open_file_in_quotes", 19, LINK_PROCS(open_file_in_quotes)},
{0, 50, 1, "open_matching_file_cpp", 22, LINK_PROCS(open_matching_file_cpp)},
{0, 48, 4, "write_zero_struct", 17, LINK_PROCS(write_zero_struct)},
};
static Meta_Key_Bind fcoder_binds_for_mac_default_default_lister_ui_map[14] = {
{1, 0, 0, "lister__write_character", 23, LINK_PROCS(lister__write_character)},
{0, 55307, 0, "lister__quit", 12, LINK_PROCS(lister__quit)},
{0, 10, 0, "lister__activate", 16, LINK_PROCS(lister__activate)},
{0, 9, 0, "lister__activate", 16, LINK_PROCS(lister__activate)},
{0, 55296, 0, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
{0, 55297, 0, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
{0, 55305, 0, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
{0, 55298, 0, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
{0, 55306, 0, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
{0, 55312, 0, "lister__wheel_scroll", 20, LINK_PROCS(lister__wheel_scroll)},
{0, 55308, 0, "lister__mouse_press", 19, LINK_PROCS(lister__mouse_press)},
{0, 55310, 0, "lister__mouse_release", 21, LINK_PROCS(lister__mouse_release)},
{0, 55313, 0, "lister__repaint", 15, LINK_PROCS(lister__repaint)},
{0, 55314, 0, "lister__repaint", 15, LINK_PROCS(lister__repaint)},
};
static Meta_Sub_Map fcoder_submaps_for_mac_default[4] = {
{"mapid_global", 12, "The following bindings apply in all situations.", 47, 0, 0, fcoder_binds_for_mac_default_mapid_global, 39},
{"mapid_file", 10, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.", 145, 0, 0, fcoder_binds_for_mac_default_mapid_file, 87},
{"default_code_map", 16, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.", 94, "mapid_file", 10, fcoder_binds_for_mac_default_default_code_map, 30},
{"default_lister_ui_map", 21, "These commands apply in 'lister mode' such as when you open a file.", 67, 0, 0, fcoder_binds_for_mac_default_default_lister_ui_map, 14},
};
static Meta_Mapping fcoder_meta_maps[2] = {
{"default", 7, "The default 4coder bindings - typically good for Windows and Linux", 66, fcoder_submaps_for_default, 4, LINK_PROCS(fill_keys_default)},
{"mac_default", 11, "Default 4coder bindings on a Mac keyboard", 41, fcoder_submaps_for_mac_default, 4, LINK_PROCS(fill_keys_mac_default)},
};
