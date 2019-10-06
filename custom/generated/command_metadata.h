#if !defined(META_PASS)
#define command_id(c) (fcoder_metacmd_ID_##c)
#define command_metadata(c) (&fcoder_metacmd_table[command_id(c)])
#define command_metadata_by_id(id) (&fcoder_metacmd_table[id])
#define command_one_past_last_id 226
#if defined(CUSTOM_COMMAND_SIG)
#define PROC_LINKS(x,y) x
#else
#define PROC_LINKS(x,y) y
#endif
#if defined(CUSTOM_COMMAND_SIG)
CUSTOM_COMMAND_SIG(set_bindings_mac_default);
CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line);
CUSTOM_COMMAND_SIG(seek_end_of_textual_line);
CUSTOM_COMMAND_SIG(seek_beginning_of_line);
CUSTOM_COMMAND_SIG(seek_end_of_line);
CUSTOM_COMMAND_SIG(goto_beginning_of_file);
CUSTOM_COMMAND_SIG(goto_end_of_file);
CUSTOM_COMMAND_SIG(change_active_panel);
CUSTOM_COMMAND_SIG(change_active_panel_backwards);
CUSTOM_COMMAND_SIG(open_panel_vsplit);
CUSTOM_COMMAND_SIG(open_panel_hsplit);
CUSTOM_COMMAND_SIG(suppress_mouse);
CUSTOM_COMMAND_SIG(allow_mouse);
CUSTOM_COMMAND_SIG(toggle_mouse);
CUSTOM_COMMAND_SIG(set_mode_to_original);
CUSTOM_COMMAND_SIG(set_mode_to_notepad_like);
CUSTOM_COMMAND_SIG(toggle_highlight_line_at_cursor);
CUSTOM_COMMAND_SIG(toggle_highlight_enclosing_scopes);
CUSTOM_COMMAND_SIG(toggle_paren_matching_helper);
CUSTOM_COMMAND_SIG(toggle_fullscreen);
CUSTOM_COMMAND_SIG(remap_interactive);
CUSTOM_COMMAND_SIG(write_character);
CUSTOM_COMMAND_SIG(write_underscore);
CUSTOM_COMMAND_SIG(delete_char);
CUSTOM_COMMAND_SIG(backspace_char);
CUSTOM_COMMAND_SIG(set_mark);
CUSTOM_COMMAND_SIG(cursor_mark_swap);
CUSTOM_COMMAND_SIG(delete_range);
CUSTOM_COMMAND_SIG(backspace_alpha_numeric_boundary);
CUSTOM_COMMAND_SIG(delete_alpha_numeric_boundary);
CUSTOM_COMMAND_SIG(snipe_backward_whitespace_or_token_boundary);
CUSTOM_COMMAND_SIG(snipe_forward_whitespace_or_token_boundary);
CUSTOM_COMMAND_SIG(center_view);
CUSTOM_COMMAND_SIG(left_adjust_view);
CUSTOM_COMMAND_SIG(click_set_cursor_and_mark);
CUSTOM_COMMAND_SIG(click_set_cursor);
CUSTOM_COMMAND_SIG(click_set_cursor_if_lbutton);
CUSTOM_COMMAND_SIG(click_set_mark);
CUSTOM_COMMAND_SIG(mouse_wheel_scroll);
CUSTOM_COMMAND_SIG(move_up);
CUSTOM_COMMAND_SIG(move_down);
CUSTOM_COMMAND_SIG(move_up_10);
CUSTOM_COMMAND_SIG(move_down_10);
CUSTOM_COMMAND_SIG(move_down_textual);
CUSTOM_COMMAND_SIG(page_up);
CUSTOM_COMMAND_SIG(page_down);
CUSTOM_COMMAND_SIG(move_up_to_blank_line);
CUSTOM_COMMAND_SIG(move_down_to_blank_line);
CUSTOM_COMMAND_SIG(move_up_to_blank_line_skip_whitespace);
CUSTOM_COMMAND_SIG(move_down_to_blank_line_skip_whitespace);
CUSTOM_COMMAND_SIG(move_up_to_blank_line_end);
CUSTOM_COMMAND_SIG(move_down_to_blank_line_end);
CUSTOM_COMMAND_SIG(move_left);
CUSTOM_COMMAND_SIG(move_right);
CUSTOM_COMMAND_SIG(move_right_whitespace_boundary);
CUSTOM_COMMAND_SIG(move_left_whitespace_boundary);
CUSTOM_COMMAND_SIG(move_right_token_boundary);
CUSTOM_COMMAND_SIG(move_left_token_boundary);
CUSTOM_COMMAND_SIG(move_right_whitespace_or_token_boundary);
CUSTOM_COMMAND_SIG(move_left_whitespace_or_token_boundary);
CUSTOM_COMMAND_SIG(move_right_alpha_numeric_boundary);
CUSTOM_COMMAND_SIG(move_left_alpha_numeric_boundary);
CUSTOM_COMMAND_SIG(move_right_alpha_numeric_or_camel_boundary);
CUSTOM_COMMAND_SIG(move_left_alpha_numeric_or_camel_boundary);
CUSTOM_COMMAND_SIG(select_all);
CUSTOM_COMMAND_SIG(to_uppercase);
CUSTOM_COMMAND_SIG(to_lowercase);
CUSTOM_COMMAND_SIG(clean_all_lines);
CUSTOM_COMMAND_SIG(basic_change_active_panel);
CUSTOM_COMMAND_SIG(close_panel);
CUSTOM_COMMAND_SIG(show_scrollbar);
CUSTOM_COMMAND_SIG(hide_scrollbar);
CUSTOM_COMMAND_SIG(show_filebar);
CUSTOM_COMMAND_SIG(hide_filebar);
CUSTOM_COMMAND_SIG(toggle_filebar);
CUSTOM_COMMAND_SIG(toggle_fps_meter);
CUSTOM_COMMAND_SIG(increase_face_size);
CUSTOM_COMMAND_SIG(decrease_face_size);
CUSTOM_COMMAND_SIG(mouse_wheel_change_face_size);
CUSTOM_COMMAND_SIG(toggle_virtual_whitespace);
CUSTOM_COMMAND_SIG(toggle_show_whitespace);
CUSTOM_COMMAND_SIG(toggle_line_numbers);
CUSTOM_COMMAND_SIG(eol_dosify);
CUSTOM_COMMAND_SIG(eol_nixify);
CUSTOM_COMMAND_SIG(exit_4coder);
CUSTOM_COMMAND_SIG(goto_line);
CUSTOM_COMMAND_SIG(search);
CUSTOM_COMMAND_SIG(reverse_search);
CUSTOM_COMMAND_SIG(search_identifier);
CUSTOM_COMMAND_SIG(reverse_search_identifier);
CUSTOM_COMMAND_SIG(replace_in_range);
CUSTOM_COMMAND_SIG(replace_in_buffer);
CUSTOM_COMMAND_SIG(replace_in_all_buffers);
CUSTOM_COMMAND_SIG(query_replace);
CUSTOM_COMMAND_SIG(query_replace_identifier);
CUSTOM_COMMAND_SIG(query_replace_selection);
CUSTOM_COMMAND_SIG(save_all_dirty_buffers);
CUSTOM_COMMAND_SIG(delete_file_query);
CUSTOM_COMMAND_SIG(save_to_query);
CUSTOM_COMMAND_SIG(rename_file_query);
CUSTOM_COMMAND_SIG(make_directory_query);
CUSTOM_COMMAND_SIG(move_line_up);
CUSTOM_COMMAND_SIG(move_line_down);
CUSTOM_COMMAND_SIG(duplicate_line);
CUSTOM_COMMAND_SIG(delete_line);
CUSTOM_COMMAND_SIG(open_file_in_quotes);
CUSTOM_COMMAND_SIG(open_matching_file_cpp);
CUSTOM_COMMAND_SIG(view_buffer_other_panel);
CUSTOM_COMMAND_SIG(swap_buffers_between_panels);
CUSTOM_COMMAND_SIG(kill_buffer);
CUSTOM_COMMAND_SIG(save);
CUSTOM_COMMAND_SIG(reopen);
CUSTOM_COMMAND_SIG(undo);
CUSTOM_COMMAND_SIG(redo);
CUSTOM_COMMAND_SIG(undo_all_buffers);
CUSTOM_COMMAND_SIG(redo_all_buffers);
CUSTOM_COMMAND_SIG(open_in_other);
CUSTOM_COMMAND_SIG(lister__quit);
CUSTOM_COMMAND_SIG(lister__activate);
CUSTOM_COMMAND_SIG(lister__write_character);
CUSTOM_COMMAND_SIG(lister__backspace_text_field);
CUSTOM_COMMAND_SIG(lister__move_up);
CUSTOM_COMMAND_SIG(lister__move_down);
CUSTOM_COMMAND_SIG(lister__wheel_scroll);
CUSTOM_COMMAND_SIG(lister__mouse_press);
CUSTOM_COMMAND_SIG(lister__mouse_release);
CUSTOM_COMMAND_SIG(lister__repaint);
CUSTOM_COMMAND_SIG(lister__write_character__default);
CUSTOM_COMMAND_SIG(lister__backspace_text_field__default);
CUSTOM_COMMAND_SIG(lister__move_up__default);
CUSTOM_COMMAND_SIG(lister__move_down__default);
CUSTOM_COMMAND_SIG(lister__write_character__file_path);
CUSTOM_COMMAND_SIG(lister__backspace_text_field__file_path);
CUSTOM_COMMAND_SIG(lister__write_character__fixed_list);
CUSTOM_COMMAND_SIG(interactive_switch_buffer);
CUSTOM_COMMAND_SIG(interactive_kill_buffer);
CUSTOM_COMMAND_SIG(interactive_open_or_new);
CUSTOM_COMMAND_SIG(interactive_new);
CUSTOM_COMMAND_SIG(interactive_open);
CUSTOM_COMMAND_SIG(command_lister);
CUSTOM_COMMAND_SIG(auto_tab_whole_file);
CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor);
CUSTOM_COMMAND_SIG(auto_tab_range);
CUSTOM_COMMAND_SIG(write_and_auto_tab);
CUSTOM_COMMAND_SIG(list_all_locations);
CUSTOM_COMMAND_SIG(list_all_substring_locations);
CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_locations_of_identifier);
CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_locations_of_selection);
CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition);
CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier);
CUSTOM_COMMAND_SIG(word_complete);
CUSTOM_COMMAND_SIG(goto_jump_at_cursor);
CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel);
CUSTOM_COMMAND_SIG(goto_next_jump);
CUSTOM_COMMAND_SIG(goto_prev_jump);
CUSTOM_COMMAND_SIG(goto_next_jump_no_skips);
CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips);
CUSTOM_COMMAND_SIG(goto_first_jump);
CUSTOM_COMMAND_SIG(goto_first_jump_same_panel_sticky);
CUSTOM_COMMAND_SIG(newline_or_goto_position);
CUSTOM_COMMAND_SIG(newline_or_goto_position_same_panel);
CUSTOM_COMMAND_SIG(view_jump_list_with_lister);
CUSTOM_COMMAND_SIG(log_graph__escape);
CUSTOM_COMMAND_SIG(log_graph__scroll_wheel);
CUSTOM_COMMAND_SIG(log_graph__page_up);
CUSTOM_COMMAND_SIG(log_graph__page_down);
CUSTOM_COMMAND_SIG(log_graph__click_select_event);
CUSTOM_COMMAND_SIG(log_graph__click_jump_to_event_source);
CUSTOM_COMMAND_SIG(show_the_log_graph);
CUSTOM_COMMAND_SIG(copy);
CUSTOM_COMMAND_SIG(cut);
CUSTOM_COMMAND_SIG(paste);
CUSTOM_COMMAND_SIG(paste_next);
CUSTOM_COMMAND_SIG(paste_and_indent);
CUSTOM_COMMAND_SIG(paste_next_and_indent);
CUSTOM_COMMAND_SIG(execute_previous_cli);
CUSTOM_COMMAND_SIG(execute_any_cli);
CUSTOM_COMMAND_SIG(build_search);
CUSTOM_COMMAND_SIG(build_in_build_panel);
CUSTOM_COMMAND_SIG(close_build_panel);
CUSTOM_COMMAND_SIG(change_to_build_panel);
CUSTOM_COMMAND_SIG(close_all_code);
CUSTOM_COMMAND_SIG(open_all_code);
CUSTOM_COMMAND_SIG(open_all_code_recursive);
CUSTOM_COMMAND_SIG(load_project);
CUSTOM_COMMAND_SIG(project_fkey_command);
CUSTOM_COMMAND_SIG(project_go_to_root_directory);
CUSTOM_COMMAND_SIG(setup_new_project);
CUSTOM_COMMAND_SIG(setup_build_bat);
CUSTOM_COMMAND_SIG(setup_build_sh);
CUSTOM_COMMAND_SIG(setup_build_bat_and_sh);
CUSTOM_COMMAND_SIG(project_command_lister);
CUSTOM_COMMAND_SIG(list_all_functions_current_buffer);
CUSTOM_COMMAND_SIG(list_all_functions_current_buffer_lister);
CUSTOM_COMMAND_SIG(list_all_functions_all_buffers);
CUSTOM_COMMAND_SIG(list_all_functions_all_buffers_lister);
CUSTOM_COMMAND_SIG(select_surrounding_scope);
CUSTOM_COMMAND_SIG(select_next_scope_absolute);
CUSTOM_COMMAND_SIG(select_prev_scope_absolute);
CUSTOM_COMMAND_SIG(place_in_scope);
CUSTOM_COMMAND_SIG(delete_current_scope);
CUSTOM_COMMAND_SIG(open_long_braces);
CUSTOM_COMMAND_SIG(open_long_braces_semicolon);
CUSTOM_COMMAND_SIG(open_long_braces_break);
CUSTOM_COMMAND_SIG(if0_off);
CUSTOM_COMMAND_SIG(write_todo);
CUSTOM_COMMAND_SIG(write_hack);
CUSTOM_COMMAND_SIG(write_note);
CUSTOM_COMMAND_SIG(write_block);
CUSTOM_COMMAND_SIG(write_zero_struct);
CUSTOM_COMMAND_SIG(comment_line);
CUSTOM_COMMAND_SIG(uncomment_line);
CUSTOM_COMMAND_SIG(comment_line_toggle);
CUSTOM_COMMAND_SIG(snippet_lister);
CUSTOM_COMMAND_SIG(miblo_increment_basic);
CUSTOM_COMMAND_SIG(miblo_decrement_basic);
CUSTOM_COMMAND_SIG(miblo_increment_time_stamp);
CUSTOM_COMMAND_SIG(miblo_decrement_time_stamp);
CUSTOM_COMMAND_SIG(miblo_increment_time_stamp_minute);
CUSTOM_COMMAND_SIG(miblo_decrement_time_stamp_minute);
CUSTOM_COMMAND_SIG(set_bindings_choose);
CUSTOM_COMMAND_SIG(set_bindings_default);
#endif
struct Command_Metadata{
PROC_LINKS(Custom_Command_Function, void) *proc;
char *name;
i32 name_len;
char *description;
i32 description_len;
char *source_name;
i32 source_name_len;
i32 line_number;
};
static Command_Metadata fcoder_metacmd_table[226] = {
{ PROC_LINKS(set_bindings_mac_default, 0), "set_bindings_mac_default", 24,  "Remap keybindings using the 'mac-default' mapping rule.", 55, "w:\\4ed\\code\\custom\\4coder_remapping_commands.cpp", 48, 62 },
{ PROC_LINKS(seek_beginning_of_textual_line, 0), "seek_beginning_of_textual_line", 30,  "Seeks the cursor to the beginning of the line across all text.", 62, "w:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2123 },
{ PROC_LINKS(seek_end_of_textual_line, 0), "seek_end_of_textual_line", 24,  "Seeks the cursor to the end of the line across all text.", 56, "w:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2129 },
{ PROC_LINKS(seek_beginning_of_line, 0), "seek_beginning_of_line", 22,  "Seeks the cursor to the beginning of the visual line.", 53, "w:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2135 },
{ PROC_LINKS(seek_end_of_line, 0), "seek_end_of_line", 16,  "Seeks the cursor to the end of the visual line.", 47, "w:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2141 },
{ PROC_LINKS(goto_beginning_of_file, 0), "goto_beginning_of_file", 22,  "Sets the cursor to the beginning of the file.", 45, "w:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2147 },
{ PROC_LINKS(goto_end_of_file, 0), "goto_end_of_file", 16,  "Sets the cursor to the end of the file.", 39, "w:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2155 },
{ PROC_LINKS(change_active_panel, 0), "change_active_panel", 19,  "Change the currently active panel, moving to the panel with the next highest view_id.", 85, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 196 },
{ PROC_LINKS(change_active_panel_backwards, 0), "change_active_panel_backwards", 29,  "Change the currently active panel, moving to the panel with the next lowest view_id.", 84, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 206 },
{ PROC_LINKS(open_panel_vsplit, 0), "open_panel_vsplit", 17,  "Create a new panel by vertically splitting the active panel.", 60, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 216 },
{ PROC_LINKS(open_panel_hsplit, 0), "open_panel_hsplit", 17,  "Create a new panel by horizontally splitting the active panel.", 62, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 226 },
{ PROC_LINKS(suppress_mouse, 0), "suppress_mouse", 14,  "Hides the mouse and causes all mosue input (clicks, position, wheel) to be ignored.", 83, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 289 },
{ PROC_LINKS(allow_mouse, 0), "allow_mouse", 11,  "Shows the mouse and causes all mouse input to be processed normally.", 68, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 295 },
{ PROC_LINKS(toggle_mouse, 0), "toggle_mouse", 12,  "Toggles the mouse suppression mode, see suppress_mouse and allow_mouse.", 71, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 301 },
{ PROC_LINKS(set_mode_to_original, 0), "set_mode_to_original", 20,  "Sets the edit mode to 4coder original.", 38, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 307 },
{ PROC_LINKS(set_mode_to_notepad_like, 0), "set_mode_to_notepad_like", 24,  "Sets the edit mode to Notepad like.", 35, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 313 },
{ PROC_LINKS(toggle_highlight_line_at_cursor, 0), "toggle_highlight_line_at_cursor", 31,  "Toggles the line highlight at the cursor.", 41, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 319 },
{ PROC_LINKS(toggle_highlight_enclosing_scopes, 0), "toggle_highlight_enclosing_scopes", 33,  "In code files scopes surrounding the cursor are highlighted with distinguishing colors.", 87, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 325 },
{ PROC_LINKS(toggle_paren_matching_helper, 0), "toggle_paren_matching_helper", 28,  "In code files matching parentheses pairs are colored with distinguishing colors.", 80, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 331 },
{ PROC_LINKS(toggle_fullscreen, 0), "toggle_fullscreen", 17,  "Toggle fullscreen mode on or off.  The change(s) do not take effect until the next frame.", 89, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 337 },
{ PROC_LINKS(remap_interactive, 0), "remap_interactive", 17,  "Switch to a named key binding map.", 34, "w:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 345 },
{ PROC_LINKS(write_character, 0), "write_character", 15,  "Inserts whatever character was used to trigger this command.", 60, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 57 },
{ PROC_LINKS(write_underscore, 0), "write_underscore", 16,  "Inserts an underscore.", 22, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 66 },
{ PROC_LINKS(delete_char, 0), "delete_char", 11,  "Deletes the character to the right of the cursor.", 49, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 73 },
{ PROC_LINKS(backspace_char, 0), "backspace_char", 14,  "Deletes the character to the left of the cursor.", 48, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 90 },
{ PROC_LINKS(set_mark, 0), "set_mark", 8,  "Sets the mark to the current position of the cursor.", 52, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 109 },
{ PROC_LINKS(cursor_mark_swap, 0), "cursor_mark_swap", 16,  "Swaps the position of the cursor and the mark.", 46, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 118 },
{ PROC_LINKS(delete_range, 0), "delete_range", 12,  "Deletes the text in the range between the cursor and the mark.", 62, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 128 },
{ PROC_LINKS(backspace_alpha_numeric_boundary, 0), "backspace_alpha_numeric_boundary", 32,  "Delete characters between the cursor position and the first alphanumeric boundary to the left.", 94, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 148 },
{ PROC_LINKS(delete_alpha_numeric_boundary, 0), "delete_alpha_numeric_boundary", 29,  "Delete characters between the cursor position and the first alphanumeric boundary to the right.", 95, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 156 },
{ PROC_LINKS(snipe_backward_whitespace_or_token_boundary, 0), "snipe_backward_whitespace_or_token_boundary", 43,  "Delete a single, whole token on or to the left of the cursor and post it to the clipboard.", 90, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 176 },
{ PROC_LINKS(snipe_forward_whitespace_or_token_boundary, 0), "snipe_forward_whitespace_or_token_boundary", 42,  "Delete a single, whole token on or to the right of the cursor and post it to the clipboard.", 91, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 184 },
{ PROC_LINKS(center_view, 0), "center_view", 11,  "Centers the view vertically on the line on which the cursor sits.", 65, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 194 },
{ PROC_LINKS(left_adjust_view, 0), "left_adjust_view", 16,  "Sets the left size of the view near the x position of the cursor.", 65, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 208 },
{ PROC_LINKS(click_set_cursor_and_mark, 0), "click_set_cursor_and_mark", 25,  "Sets the cursor position and mark to the mouse position.", 56, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 220 },
{ PROC_LINKS(click_set_cursor, 0), "click_set_cursor", 16,  "Sets the cursor position to the mouse position.", 47, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 232 },
{ PROC_LINKS(click_set_cursor_if_lbutton, 0), "click_set_cursor_if_lbutton", 27,  "If the mouse left button is pressed, sets the cursor position to the mouse position.", 84, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 244 },
{ PROC_LINKS(click_set_mark, 0), "click_set_mark", 14,  "Sets the mark position to the mouse position.", 45, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 258 },
{ PROC_LINKS(mouse_wheel_scroll, 0), "mouse_wheel_scroll", 18,  "Reads the scroll wheel value from the mouse state and scrolls accordingly.", 74, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 270 },
{ PROC_LINKS(move_up, 0), "move_up", 7,  "Moves the cursor up one line.", 29, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 331 },
{ PROC_LINKS(move_down, 0), "move_down", 9,  "Moves the cursor down one line.", 31, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 337 },
{ PROC_LINKS(move_up_10, 0), "move_up_10", 10,  "Moves the cursor up ten lines.", 30, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 343 },
{ PROC_LINKS(move_down_10, 0), "move_down_10", 12,  "Moves the cursor down ten lines.", 32, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 349 },
{ PROC_LINKS(move_down_textual, 0), "move_down_textual", 17,  "Moves down to the next line of actual text, regardless of line wrapping.", 72, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 355 },
{ PROC_LINKS(page_up, 0), "page_up", 7,  "Scrolls the view up one view height and moves the cursor up one view height.", 76, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 365 },
{ PROC_LINKS(page_down, 0), "page_down", 9,  "Scrolls the view down one view height and moves the cursor down one view height.", 80, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 373 },
{ PROC_LINKS(move_up_to_blank_line, 0), "move_up_to_blank_line", 21,  "Seeks the cursor up to the next blank line.", 43, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 403 },
{ PROC_LINKS(move_down_to_blank_line, 0), "move_down_to_blank_line", 23,  "Seeks the cursor down to the next blank line.", 45, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 409 },
{ PROC_LINKS(move_up_to_blank_line_skip_whitespace, 0), "move_up_to_blank_line_skip_whitespace", 37,  "Seeks the cursor up to the next blank line and places it at the end of the line.", 80, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 415 },
{ PROC_LINKS(move_down_to_blank_line_skip_whitespace, 0), "move_down_to_blank_line_skip_whitespace", 39,  "Seeks the cursor down to the next blank line and places it at the end of the line.", 82, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 421 },
{ PROC_LINKS(move_up_to_blank_line_end, 0), "move_up_to_blank_line_end", 25,  "Seeks the cursor up to the next blank line and places it at the end of the line.", 80, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 427 },
{ PROC_LINKS(move_down_to_blank_line_end, 0), "move_down_to_blank_line_end", 27,  "Seeks the cursor down to the next blank line and places it at the end of the line.", 82, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 433 },
{ PROC_LINKS(move_left, 0), "move_left", 9,  "Moves the cursor one character to the left.", 43, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 439 },
{ PROC_LINKS(move_right, 0), "move_right", 10,  "Moves the cursor one character to the right.", 44, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 451 },
{ PROC_LINKS(move_right_whitespace_boundary, 0), "move_right_whitespace_boundary", 30,  "Seek right for the next boundary between whitespace and non-whitespace.", 71, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 473 },
{ PROC_LINKS(move_left_whitespace_boundary, 0), "move_left_whitespace_boundary", 29,  "Seek left for the next boundary between whitespace and non-whitespace.", 70, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 481 },
{ PROC_LINKS(move_right_token_boundary, 0), "move_right_token_boundary", 25,  "Seek right for the next end of a token.", 39, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 489 },
{ PROC_LINKS(move_left_token_boundary, 0), "move_left_token_boundary", 24,  "Seek left for the next beginning of a token.", 44, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 497 },
{ PROC_LINKS(move_right_whitespace_or_token_boundary, 0), "move_right_whitespace_or_token_boundary", 39,  "Seek right for the next end of a token or boundary between whitespace and non-whitespace.", 89, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 505 },
{ PROC_LINKS(move_left_whitespace_or_token_boundary, 0), "move_left_whitespace_or_token_boundary", 38,  "Seek left for the next end of a token or boundary between whitespace and non-whitespace.", 88, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 513 },
{ PROC_LINKS(move_right_alpha_numeric_boundary, 0), "move_right_alpha_numeric_boundary", 33,  "Seek right for boundary between alphanumeric characters and non-alphanumeric characters.", 88, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 521 },
{ PROC_LINKS(move_left_alpha_numeric_boundary, 0), "move_left_alpha_numeric_boundary", 32,  "Seek left for boundary between alphanumeric characters and non-alphanumeric characters.", 87, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 529 },
{ PROC_LINKS(move_right_alpha_numeric_or_camel_boundary, 0), "move_right_alpha_numeric_or_camel_boundary", 42,  "Seek right for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.", 107, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 537 },
{ PROC_LINKS(move_left_alpha_numeric_or_camel_boundary, 0), "move_left_alpha_numeric_or_camel_boundary", 41,  "Seek left for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.", 106, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 545 },
{ PROC_LINKS(select_all, 0), "select_all", 10,  "Puts the cursor at the top of the file, and the mark at the bottom of the file.", 79, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 555 },
{ PROC_LINKS(to_uppercase, 0), "to_uppercase", 12,  "Converts all ascii text in the range between the cursor and the mark to uppercase.", 82, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 568 },
{ PROC_LINKS(to_lowercase, 0), "to_lowercase", 12,  "Converts all ascii text in the range between the cursor and the mark to lowercase.", 82, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 581 },
{ PROC_LINKS(clean_all_lines, 0), "clean_all_lines", 15,  "Removes trailing whitespace from all lines in the current buffer.", 65, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 594 },
{ PROC_LINKS(basic_change_active_panel, 0), "basic_change_active_panel", 25,  "Change the currently active panel, moving to the panel with the next highest view_id.  Will not skipe the build panel if it is open.", 132, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 628 },
{ PROC_LINKS(close_panel, 0), "close_panel", 11,  "Closes the currently active panel if it is not the only panel open.", 67, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 636 },
{ PROC_LINKS(show_scrollbar, 0), "show_scrollbar", 14,  "Sets the current view to show it's scrollbar.", 45, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 645 },
{ PROC_LINKS(hide_scrollbar, 0), "hide_scrollbar", 14,  "Sets the current view to hide it's scrollbar.", 45, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 652 },
{ PROC_LINKS(show_filebar, 0), "show_filebar", 12,  "Sets the current view to show it's filebar.", 43, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 659 },
{ PROC_LINKS(hide_filebar, 0), "hide_filebar", 12,  "Sets the current view to hide it's filebar.", 43, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 666 },
{ PROC_LINKS(toggle_filebar, 0), "toggle_filebar", 14,  "Toggles the visibility status of the current view's filebar.", 60, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 673 },
{ PROC_LINKS(toggle_fps_meter, 0), "toggle_fps_meter", 16,  "Toggles the visibility of the FPS performance meter", 51, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 682 },
{ PROC_LINKS(increase_face_size, 0), "increase_face_size", 18,  "Increase the size of the face used by the current buffer.", 57, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 688 },
{ PROC_LINKS(decrease_face_size, 0), "decrease_face_size", 18,  "Decrease the size of the face used by the current buffer.", 57, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 699 },
{ PROC_LINKS(mouse_wheel_change_face_size, 0), "mouse_wheel_change_face_size", 28,  "Reads the state of the mouse wheel and uses it to either increase or decrease the face size.", 92, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 710 },
{ PROC_LINKS(toggle_virtual_whitespace, 0), "toggle_virtual_whitespace", 25,  "Toggles the current buffer's virtual whitespace status.", 55, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 727 },
{ PROC_LINKS(toggle_show_whitespace, 0), "toggle_show_whitespace", 22,  "Toggles the current buffer's whitespace visibility status.", 58, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 737 },
{ PROC_LINKS(toggle_line_numbers, 0), "toggle_line_numbers", 19,  "Toggles the left margin line numbers.", 37, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 746 },
{ PROC_LINKS(eol_dosify, 0), "eol_dosify", 10,  "Puts the buffer in DOS line ending mode.", 40, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 752 },
{ PROC_LINKS(eol_nixify, 0), "eol_nixify", 10,  "Puts the buffer in NIX line ending mode.", 40, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 760 },
{ PROC_LINKS(exit_4coder, 0), "exit_4coder", 11,  "Attempts to close 4coder.", 25, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 768 },
{ PROC_LINKS(goto_line, 0), "goto_line", 9,  "Queries the user for a number, and jumps the cursor to the corresponding line.", 78, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 776 },
{ PROC_LINKS(search, 0), "search", 6,  "Begins an incremental search down through the current buffer for a user specified string.", 89, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 967 },
{ PROC_LINKS(reverse_search, 0), "reverse_search", 14,  "Begins an incremental search up through the current buffer for a user specified string.", 87, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 973 },
{ PROC_LINKS(search_identifier, 0), "search_identifier", 17,  "Begins an incremental search down through the current buffer for the word or token under the cursor.", 100, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 979 },
{ PROC_LINKS(reverse_search_identifier, 0), "reverse_search_identifier", 25,  "Begins an incremental search up through the current buffer for the word or token under the cursor.", 98, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 990 },
{ PROC_LINKS(replace_in_range, 0), "replace_in_range", 16,  "Queries the user for a needle and string. Replaces all occurences of needle with string in the range between cursor and the mark in the active buffer.", 150, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1041 },
{ PROC_LINKS(replace_in_buffer, 0), "replace_in_buffer", 17,  "Queries the user for a needle and string. Replaces all occurences of needle with string in the active buffer.", 109, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1050 },
{ PROC_LINKS(replace_in_all_buffers, 0), "replace_in_all_buffers", 22,  "Queries the user for a needle and string. Replaces all occurences of needle with string in all editable buffers.", 112, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1059 },
{ PROC_LINKS(query_replace, 0), "query_replace", 13,  "Queries the user for two strings, and incrementally replaces every occurence of the first string with the second string.", 120, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1147 },
{ PROC_LINKS(query_replace_identifier, 0), "query_replace_identifier", 24,  "Queries the user for a string, and incrementally replace every occurence of the word or token found at the cursor with the specified string.", 140, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1167 },
{ PROC_LINKS(query_replace_selection, 0), "query_replace_selection", 23,  "Queries the user for a string, and incrementally replace every occurence of the string found in the selected range with the specified string.", 141, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1183 },
{ PROC_LINKS(save_all_dirty_buffers, 0), "save_all_dirty_buffers", 22,  "Saves all buffers marked dirty (showing the '*' indicator).", 59, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1218 },
{ PROC_LINKS(delete_file_query, 0), "delete_file_query", 17,  "Deletes the file of the current buffer if 4coder has the appropriate access rights. Will ask the user for confirmation first.", 125, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1243 },
{ PROC_LINKS(save_to_query, 0), "save_to_query", 13,  "Queries the user for a file name and saves the contents of the current buffer, altering the buffer's name too.", 110, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1281 },
{ PROC_LINKS(rename_file_query, 0), "rename_file_query", 17,  "Queries the user for a new name and renames the file of the current buffer, altering the buffer's name too.", 107, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1313 },
{ PROC_LINKS(make_directory_query, 0), "make_directory_query", 20,  "Queries the user for a name and creates a new directory with the given name.", 76, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1350 },
{ PROC_LINKS(move_line_up, 0), "move_line_up", 12,  "Swaps the line under the cursor with the line above it, and moves the cursor up with it.", 88, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1383 },
{ PROC_LINKS(move_line_down, 0), "move_line_down", 14,  "Swaps the line under the cursor with the line below it, and moves the cursor down with it.", 90, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1389 },
{ PROC_LINKS(duplicate_line, 0), "duplicate_line", 14,  "Create a copy of the line on which the cursor sits.", 51, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1395 },
{ PROC_LINKS(delete_line, 0), "delete_line", 11,  "Delete the line the on which the cursor sits.", 45, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1409 },
{ PROC_LINKS(open_file_in_quotes, 0), "open_file_in_quotes", 19,  "Reads a filename from surrounding '\"' characters and attempts to open the corresponding file.", 94, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1474 },
{ PROC_LINKS(open_matching_file_cpp, 0), "open_matching_file_cpp", 22,  "If the current file is a *.cpp or *.h, attempts to open the corresponding *.h or *.cpp file in the other view.", 110, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1506 },
{ PROC_LINKS(view_buffer_other_panel, 0), "view_buffer_other_panel", 23,  "Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.", 104, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1519 },
{ PROC_LINKS(swap_buffers_between_panels, 0), "swap_buffers_between_panels", 27,  "Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.", 104, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1531 },
{ PROC_LINKS(kill_buffer, 0), "kill_buffer", 11,  "Kills the current buffer.", 25, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1567 },
{ PROC_LINKS(save, 0), "save", 4,  "Saves the current buffer.", 25, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1575 },
{ PROC_LINKS(reopen, 0), "reopen", 6,  "Reopen the current buffer from the hard drive.", 46, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1585 },
{ PROC_LINKS(undo, 0), "undo", 4,  "Advances backwards through the undo history of the current buffer.", 66, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1812 },
{ PROC_LINKS(redo, 0), "redo", 4,  "Advances forwards through the undo history of the current buffer.", 65, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1825 },
{ PROC_LINKS(undo_all_buffers, 0), "undo_all_buffers", 16,  "Advances backward through the undo history in the buffer containing the most recent regular edit.", 97, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1839 },
{ PROC_LINKS(redo_all_buffers, 0), "redo_all_buffers", 16,  "Advances forward through the undo history in the buffer containing the most recent regular edit.", 96, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1910 },
{ PROC_LINKS(open_in_other, 0), "open_in_other", 13,  "Interactively opens a file in the other panel.", 46, "w:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 2011 },
{ PROC_LINKS(lister__quit, 0), "lister__quit", 12,  "A lister mode command that quits the list without executing any actions.", 72, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 8 },
{ PROC_LINKS(lister__activate, 0), "lister__activate", 16,  "A lister mode command that activates the list's action on the highlighted item.", 79, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 15 },
{ PROC_LINKS(lister__write_character, 0), "lister__write_character", 23,  "A lister mode command that dispatches to the lister's write character handler.", 78, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 30 },
{ PROC_LINKS(lister__backspace_text_field, 0), "lister__backspace_text_field", 28,  "A lister mode command that dispatches to the lister's backspace text field handler.", 83, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 40 },
{ PROC_LINKS(lister__move_up, 0), "lister__move_up", 15,  "A lister mode command that dispatches to the lister's navigate up handler.", 74, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 50 },
{ PROC_LINKS(lister__move_down, 0), "lister__move_down", 17,  "A lister mode command that dispatches to the lister's navigate down handler.", 76, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 60 },
{ PROC_LINKS(lister__wheel_scroll, 0), "lister__wheel_scroll", 20,  "A lister mode command that scrolls the list in response to the mouse wheel.", 75, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 70 },
{ PROC_LINKS(lister__mouse_press, 0), "lister__mouse_press", 19,  "A lister mode command that beings a click interaction with a list item under the mouse.", 87, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 84 },
{ PROC_LINKS(lister__mouse_release, 0), "lister__mouse_release", 21,  "A lister mode command that ends a click interaction with a list item under the mouse, possibly activating it.", 109, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 95 },
{ PROC_LINKS(lister__repaint, 0), "lister__repaint", 15,  "A lister mode command that updates the lists UI data.", 53, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 110 },
{ PROC_LINKS(lister__write_character__default, 0), "lister__write_character__default", 32,  "A lister mode command that inserts a new character to the text field.", 69, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 120 },
{ PROC_LINKS(lister__backspace_text_field__default, 0), "lister__backspace_text_field__default", 37,  "A lister mode command that backspaces one character from the text field.", 72, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 139 },
{ PROC_LINKS(lister__move_up__default, 0), "lister__move_up__default", 24,  "A lister mode command that moves the highlighted item one up in the list.", 73, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 153 },
{ PROC_LINKS(lister__move_down__default, 0), "lister__move_down__default", 26,  "A lister mode command that moves the highlighted item one down in the list.", 75, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 168 },
{ PROC_LINKS(lister__write_character__file_path, 0), "lister__write_character__file_path", 34,  "A lister mode command that inserts a character into the text field of a file system list.", 89, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 183 },
{ PROC_LINKS(lister__backspace_text_field__file_path, 0), "lister__backspace_text_field__file_path", 39,  "A lister mode command that backspaces one character from the text field of a file system list.", 94, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 208 },
{ PROC_LINKS(lister__write_character__fixed_list, 0), "lister__write_character__fixed_list", 35,  "A lister mode command that handles input for the fixed sure to kill list.", 73, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 249 },
{ PROC_LINKS(interactive_switch_buffer, 0), "interactive_switch_buffer", 25,  "Interactively switch to an open buffer.", 39, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 718 },
{ PROC_LINKS(interactive_kill_buffer, 0), "interactive_kill_buffer", 23,  "Interactively kill an open buffer.", 34, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 737 },
{ PROC_LINKS(interactive_open_or_new, 0), "interactive_open_or_new", 23,  "Interactively open a file out of the file system.", 49, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 808 },
{ PROC_LINKS(interactive_new, 0), "interactive_new", 15,  "Interactively creates a new file.", 33, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 847 },
{ PROC_LINKS(interactive_open, 0), "interactive_open", 16,  "Interactively opens a file.", 27, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 880 },
{ PROC_LINKS(command_lister, 0), "command_lister", 14,  "Opens an interactive list of all registered commands.", 53, "w:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 960 },
{ PROC_LINKS(auto_tab_whole_file, 0), "auto_tab_whole_file", 19,  "Audo-indents the entire current buffer.", 39, "w:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 322 },
{ PROC_LINKS(auto_tab_line_at_cursor, 0), "auto_tab_line_at_cursor", 23,  "Auto-indents the line on which the cursor sits.", 47, "w:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 331 },
{ PROC_LINKS(auto_tab_range, 0), "auto_tab_range", 14,  "Auto-indents the range between the cursor and the mark.", 55, "w:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 341 },
{ PROC_LINKS(write_and_auto_tab, 0), "write_and_auto_tab", 18,  "Inserts a character and auto-indents the line on which the cursor sits.", 71, "w:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 351 },
{ PROC_LINKS(list_all_locations, 0), "list_all_locations", 18,  "Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.", 99, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 166 },
{ PROC_LINKS(list_all_substring_locations, 0), "list_all_substring_locations", 28,  "Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.", 103, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 172 },
{ PROC_LINKS(list_all_locations_case_insensitive, 0), "list_all_locations_case_insensitive", 35,  "Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.", 101, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 178 },
{ PROC_LINKS(list_all_substring_locations_case_insensitive, 0), "list_all_substring_locations_case_insensitive", 45,  "Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.", 105, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 184 },
{ PROC_LINKS(list_all_locations_of_identifier, 0), "list_all_locations_of_identifier", 32,  "Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.", 102, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 190 },
{ PROC_LINKS(list_all_locations_of_identifier_case_insensitive, 0), "list_all_locations_of_identifier_case_insensitive", 49,  "Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.", 104, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 196 },
{ PROC_LINKS(list_all_locations_of_selection, 0), "list_all_locations_of_selection", 31,  "Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.", 102, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 202 },
{ PROC_LINKS(list_all_locations_of_selection_case_insensitive, 0), "list_all_locations_of_selection_case_insensitive", 48,  "Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.", 104, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 208 },
{ PROC_LINKS(list_all_locations_of_type_definition, 0), "list_all_locations_of_type_definition", 37,  "Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.", 121, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 214 },
{ PROC_LINKS(list_all_locations_of_type_definition_of_identifier, 0), "list_all_locations_of_type_definition_of_identifier", 51,  "Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.", 125, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 222 },
{ PROC_LINKS(word_complete, 0), "word_complete", 13,  "Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.", 130, "w:\\4ed\\code\\custom\\4coder_search.cpp", 36, 378 },
{ PROC_LINKS(goto_jump_at_cursor, 0), "goto_jump_at_cursor", 19,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.", 187, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 346 },
{ PROC_LINKS(goto_jump_at_cursor_same_panel, 0), "goto_jump_at_cursor_same_panel", 30,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list.", 167, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 373 },
{ PROC_LINKS(goto_next_jump, 0), "goto_next_jump", 14,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.", 123, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 462 },
{ PROC_LINKS(goto_prev_jump, 0), "goto_prev_jump", 14,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations.", 127, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 479 },
{ PROC_LINKS(goto_next_jump_no_skips, 0), "goto_next_jump_no_skips", 23,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.", 132, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 492 },
{ PROC_LINKS(goto_prev_jump_no_skips, 0), "goto_prev_jump_no_skips", 23,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.", 136, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 509 },
{ PROC_LINKS(goto_first_jump, 0), "goto_first_jump", 15,  "If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.", 95, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 523 },
{ PROC_LINKS(goto_first_jump_same_panel_sticky, 0), "goto_first_jump_same_panel_sticky", 33,  "If a buffer containing jump locations has been locked in, goes to the first jump in the buffer and views the buffer in the panel where the jump list was.", 153, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 540 },
{ PROC_LINKS(newline_or_goto_position, 0), "newline_or_goto_position", 24,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.", 106, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 562 },
{ PROC_LINKS(newline_or_goto_position_same_panel, 0), "newline_or_goto_position_same_panel", 35,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.", 117, "w:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 579 },
{ PROC_LINKS(view_jump_list_with_lister, 0), "view_jump_list_with_lister", 26,  "When executed on a buffer with jumps, creates a persistent lister for all the jumps", 83, "w:\\4ed\\code\\custom\\4coder_jump_lister.cpp", 41, 102 },
{ PROC_LINKS(log_graph__escape, 0), "log_graph__escape", 17,  "Ends the log grapher", 20, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 906 },
{ PROC_LINKS(log_graph__scroll_wheel, 0), "log_graph__scroll_wheel", 23,  "Scrolls the log graph", 21, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 915 },
{ PROC_LINKS(log_graph__page_up, 0), "log_graph__page_up", 18,  "Scroll the log graph up one whole page", 38, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 926 },
{ PROC_LINKS(log_graph__page_down, 0), "log_graph__page_down", 20,  "Scroll the log graph down one whole page", 40, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 934 },
{ PROC_LINKS(log_graph__click_select_event, 0), "log_graph__click_select_event", 29,  "Select the event record at the mouse point in the log graph", 59, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 968 },
{ PROC_LINKS(log_graph__click_jump_to_event_source, 0), "log_graph__click_jump_to_event_source", 37,  "Jump to the code that logged the event record at the mouse point in the log graph", 81, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 987 },
{ PROC_LINKS(show_the_log_graph, 0), "show_the_log_graph", 18,  "Parser *log* and displays the 'log graph' UI", 44, "w:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 1035 },
{ PROC_LINKS(copy, 0), "copy", 4,  "Copy the text in the range from the cursor to the mark onto the clipboard.", 74, "w:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 19 },
{ PROC_LINKS(cut, 0), "cut", 3,  "Cut the text in the range from the cursor to the mark onto the clipboard.", 73, "w:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 28 },
{ PROC_LINKS(paste, 0), "paste", 5,  "At the cursor, insert the text at the top of the clipboard.", 59, "w:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 39 },
{ PROC_LINKS(paste_next, 0), "paste_next", 10,  "If the previous command was paste or paste_next, replaces the paste range with the next text down on the clipboard, otherwise operates as the paste command.", 156, "w:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 73 },
{ PROC_LINKS(paste_and_indent, 0), "paste_and_indent", 16,  "Paste from the top of clipboard and run auto-indent on the newly pasted text.", 77, "w:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 115 },
{ PROC_LINKS(paste_next_and_indent, 0), "paste_next_and_indent", 21,  "Paste the next item on the clipboard and run auto-indent on the newly pasted text.", 82, "w:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 122 },
{ PROC_LINKS(execute_previous_cli, 0), "execute_previous_cli", 20,  "If the command execute_any_cli has already been used, this will execute a CLI reusing the most recent buffer name and command.", 126, "w:\\4ed\\code\\custom\\4coder_system_command.cpp", 44, 7 },
{ PROC_LINKS(execute_any_cli, 0), "execute_any_cli", 15,  "Queries for an output buffer name and system command, runs the system command as a CLI and prints the output to the specified buffer.", 133, "w:\\4ed\\code\\custom\\4coder_system_command.cpp", 44, 22 },
{ PROC_LINKS(build_search, 0), "build_search", 12,  "Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.", 153, "w:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 128 },
{ PROC_LINKS(build_in_build_panel, 0), "build_in_build_panel", 20,  "Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.  Puts the *compilation* buffer in a panel at the footer of the current view.", 230, "w:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 163 },
{ PROC_LINKS(close_build_panel, 0), "close_build_panel", 17,  "If the special build panel is open, closes it.", 46, "w:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 178 },
{ PROC_LINKS(change_to_build_panel, 0), "change_to_build_panel", 21,  "If the special build panel is open, makes the build panel the active panel.", 75, "w:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 184 },
{ PROC_LINKS(close_all_code, 0), "close_all_code", 14,  "Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.", 107, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 917 },
{ PROC_LINKS(open_all_code, 0), "open_all_code", 13,  "Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.", 164, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 923 },
{ PROC_LINKS(open_all_code_recursive, 0), "open_all_code_recursive", 23,  "Works as open_all_code but also runs in all subdirectories.", 59, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 929 },
{ PROC_LINKS(load_project, 0), "load_project", 12,  "Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.", 167, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 937 },
{ PROC_LINKS(project_fkey_command, 0), "project_fkey_command", 20,  "Run an 'fkey command' configured in a project.4coder file.  Determines the index of the 'fkey command' by which function key or numeric key was pressed to trigger the command.", 175, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 944 },
{ PROC_LINKS(project_go_to_root_directory, 0), "project_go_to_root_directory", 28,  "Changes 4coder's hot directory to the root directory of the currently loaded project. With no loaded project nothing hapepns.", 125, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 967 },
{ PROC_LINKS(setup_new_project, 0), "setup_new_project", 17,  "Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.", 120, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1299 },
{ PROC_LINKS(setup_build_bat, 0), "setup_build_bat", 15,  "Queries the user for several configuration options and initializes a new build batch script.", 92, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1306 },
{ PROC_LINKS(setup_build_sh, 0), "setup_build_sh", 14,  "Queries the user for several configuration options and initializes a new build shell script.", 92, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1312 },
{ PROC_LINKS(setup_build_bat_and_sh, 0), "setup_build_bat_and_sh", 22,  "Queries the user for several configuration options and initializes a new build batch script.", 92, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1318 },
{ PROC_LINKS(project_command_lister, 0), "project_command_lister", 22,  "Open a lister of all commands in the currently loaded project.", 62, "w:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1333 },
{ PROC_LINKS(list_all_functions_current_buffer, 0), "list_all_functions_current_buffer", 33,  "Creates a jump list of lines of the current buffer that appear to define or declare functions.", 94, "w:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 267 },
{ PROC_LINKS(list_all_functions_current_buffer_lister, 0), "list_all_functions_current_buffer_lister", 40,  "Creates a lister of locations that look like function definitions and declarations in the buffer.", 97, "w:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 277 },
{ PROC_LINKS(list_all_functions_all_buffers, 0), "list_all_functions_all_buffers", 30,  "Creates a jump list of lines from all buffers that appear to define or declare functions.", 89, "w:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 289 },
{ PROC_LINKS(list_all_functions_all_buffers_lister, 0), "list_all_functions_all_buffers_lister", 37,  "Creates a lister of locations that look like function definitions and declarations all buffers.", 95, "w:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 295 },
{ PROC_LINKS(select_surrounding_scope, 0), "select_surrounding_scope", 24,  "Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.", 107, "w:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 321 },
{ PROC_LINKS(select_next_scope_absolute, 0), "select_next_scope_absolute", 26,  "Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.", 102, "w:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 336 },
{ PROC_LINKS(select_prev_scope_absolute, 0), "select_prev_scope_absolute", 26,  "Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.", 103, "w:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 355 },
{ PROC_LINKS(place_in_scope, 0), "place_in_scope", 14,  "Wraps the code contained in the range between cursor and mark with a new curly brace scope.", 91, "w:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 429 },
{ PROC_LINKS(delete_current_scope, 0), "delete_current_scope", 20,  "Deletes the braces surrounding the currently selected scope.  Leaves the contents within the scope.", 99, "w:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 435 },
{ PROC_LINKS(open_long_braces, 0), "open_long_braces", 16,  "At the cursor, insert a '{' and '}' separated by a blank line.", 62, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 46 },
{ PROC_LINKS(open_long_braces_semicolon, 0), "open_long_braces_semicolon", 26,  "At the cursor, insert a '{' and '};' separated by a blank line.", 63, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 54 },
{ PROC_LINKS(open_long_braces_break, 0), "open_long_braces_break", 22,  "At the cursor, insert a '{' and '}break;' separated by a blank line.", 68, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 62 },
{ PROC_LINKS(if0_off, 0), "if0_off", 7,  "Surround the range between the cursor and mark with an '#if 0' and an '#endif'", 78, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 70 },
{ PROC_LINKS(write_todo, 0), "write_todo", 10,  "At the cursor, insert a '// TODO' comment, includes user name if it was specified in config.4coder.", 99, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 76 },
{ PROC_LINKS(write_hack, 0), "write_hack", 10,  "At the cursor, insert a '// HACK' comment, includes user name if it was specified in config.4coder.", 99, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 82 },
{ PROC_LINKS(write_note, 0), "write_note", 10,  "At the cursor, insert a '// NOTE' comment, includes user name if it was specified in config.4coder.", 99, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 88 },
{ PROC_LINKS(write_block, 0), "write_block", 11,  "At the cursor, insert a block comment.", 38, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 94 },
{ PROC_LINKS(write_zero_struct, 0), "write_zero_struct", 17,  "At the cursor, insert a ' = {};'.", 33, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 100 },
{ PROC_LINKS(comment_line, 0), "comment_line", 12,  "Insert '//' at the beginning of the line after leading whitespace.", 66, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 125 },
{ PROC_LINKS(uncomment_line, 0), "uncomment_line", 14,  "If present, delete '//' at the beginning of the line after leading whitespace.", 78, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 137 },
{ PROC_LINKS(comment_line_toggle, 0), "comment_line_toggle", 19,  "Turns uncommented lines into commented lines and vice versa for comments starting with '//'.", 92, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 149 },
{ PROC_LINKS(snippet_lister, 0), "snippet_lister", 14,  "Opens a snippet lister for inserting whole pre-written snippets of text.", 72, "w:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 233 },
{ PROC_LINKS(miblo_increment_basic, 0), "miblo_increment_basic", 21,  "Increment an integer under the cursor by one.", 45, "w:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 29 },
{ PROC_LINKS(miblo_decrement_basic, 0), "miblo_decrement_basic", 21,  "Decrement an integer under the cursor by one.", 45, "w:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 44 },
{ PROC_LINKS(miblo_increment_time_stamp, 0), "miblo_increment_time_stamp", 26,  "Increment a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss", 81, "w:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 231 },
{ PROC_LINKS(miblo_decrement_time_stamp, 0), "miblo_decrement_time_stamp", 26,  "Decrement a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss", 81, "w:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 237 },
{ PROC_LINKS(miblo_increment_time_stamp_minute, 0), "miblo_increment_time_stamp_minute", 33,  "Increment a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss", 81, "w:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 243 },
{ PROC_LINKS(miblo_decrement_time_stamp_minute, 0), "miblo_decrement_time_stamp_minute", 33,  "Decrement a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss", 81, "w:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 249 },
{ PROC_LINKS(set_bindings_choose, 0), "set_bindings_choose", 19,  "Remap keybindings using the 'choose' mapping rule.", 50, "w:\\4ed\\code\\custom\\4coder_remapping_commands.cpp", 48, 41 },
{ PROC_LINKS(set_bindings_default, 0), "set_bindings_default", 20,  "Remap keybindings using the 'default' mapping rule.", 51, "w:\\4ed\\code\\custom\\4coder_remapping_commands.cpp", 48, 51 },
};
static i32 fcoder_metacmd_ID_set_bindings_mac_default = 0;
static i32 fcoder_metacmd_ID_seek_beginning_of_textual_line = 1;
static i32 fcoder_metacmd_ID_seek_end_of_textual_line = 2;
static i32 fcoder_metacmd_ID_seek_beginning_of_line = 3;
static i32 fcoder_metacmd_ID_seek_end_of_line = 4;
static i32 fcoder_metacmd_ID_goto_beginning_of_file = 5;
static i32 fcoder_metacmd_ID_goto_end_of_file = 6;
static i32 fcoder_metacmd_ID_change_active_panel = 7;
static i32 fcoder_metacmd_ID_change_active_panel_backwards = 8;
static i32 fcoder_metacmd_ID_open_panel_vsplit = 9;
static i32 fcoder_metacmd_ID_open_panel_hsplit = 10;
static i32 fcoder_metacmd_ID_suppress_mouse = 11;
static i32 fcoder_metacmd_ID_allow_mouse = 12;
static i32 fcoder_metacmd_ID_toggle_mouse = 13;
static i32 fcoder_metacmd_ID_set_mode_to_original = 14;
static i32 fcoder_metacmd_ID_set_mode_to_notepad_like = 15;
static i32 fcoder_metacmd_ID_toggle_highlight_line_at_cursor = 16;
static i32 fcoder_metacmd_ID_toggle_highlight_enclosing_scopes = 17;
static i32 fcoder_metacmd_ID_toggle_paren_matching_helper = 18;
static i32 fcoder_metacmd_ID_toggle_fullscreen = 19;
static i32 fcoder_metacmd_ID_remap_interactive = 20;
static i32 fcoder_metacmd_ID_write_character = 21;
static i32 fcoder_metacmd_ID_write_underscore = 22;
static i32 fcoder_metacmd_ID_delete_char = 23;
static i32 fcoder_metacmd_ID_backspace_char = 24;
static i32 fcoder_metacmd_ID_set_mark = 25;
static i32 fcoder_metacmd_ID_cursor_mark_swap = 26;
static i32 fcoder_metacmd_ID_delete_range = 27;
static i32 fcoder_metacmd_ID_backspace_alpha_numeric_boundary = 28;
static i32 fcoder_metacmd_ID_delete_alpha_numeric_boundary = 29;
static i32 fcoder_metacmd_ID_snipe_backward_whitespace_or_token_boundary = 30;
static i32 fcoder_metacmd_ID_snipe_forward_whitespace_or_token_boundary = 31;
static i32 fcoder_metacmd_ID_center_view = 32;
static i32 fcoder_metacmd_ID_left_adjust_view = 33;
static i32 fcoder_metacmd_ID_click_set_cursor_and_mark = 34;
static i32 fcoder_metacmd_ID_click_set_cursor = 35;
static i32 fcoder_metacmd_ID_click_set_cursor_if_lbutton = 36;
static i32 fcoder_metacmd_ID_click_set_mark = 37;
static i32 fcoder_metacmd_ID_mouse_wheel_scroll = 38;
static i32 fcoder_metacmd_ID_move_up = 39;
static i32 fcoder_metacmd_ID_move_down = 40;
static i32 fcoder_metacmd_ID_move_up_10 = 41;
static i32 fcoder_metacmd_ID_move_down_10 = 42;
static i32 fcoder_metacmd_ID_move_down_textual = 43;
static i32 fcoder_metacmd_ID_page_up = 44;
static i32 fcoder_metacmd_ID_page_down = 45;
static i32 fcoder_metacmd_ID_move_up_to_blank_line = 46;
static i32 fcoder_metacmd_ID_move_down_to_blank_line = 47;
static i32 fcoder_metacmd_ID_move_up_to_blank_line_skip_whitespace = 48;
static i32 fcoder_metacmd_ID_move_down_to_blank_line_skip_whitespace = 49;
static i32 fcoder_metacmd_ID_move_up_to_blank_line_end = 50;
static i32 fcoder_metacmd_ID_move_down_to_blank_line_end = 51;
static i32 fcoder_metacmd_ID_move_left = 52;
static i32 fcoder_metacmd_ID_move_right = 53;
static i32 fcoder_metacmd_ID_move_right_whitespace_boundary = 54;
static i32 fcoder_metacmd_ID_move_left_whitespace_boundary = 55;
static i32 fcoder_metacmd_ID_move_right_token_boundary = 56;
static i32 fcoder_metacmd_ID_move_left_token_boundary = 57;
static i32 fcoder_metacmd_ID_move_right_whitespace_or_token_boundary = 58;
static i32 fcoder_metacmd_ID_move_left_whitespace_or_token_boundary = 59;
static i32 fcoder_metacmd_ID_move_right_alpha_numeric_boundary = 60;
static i32 fcoder_metacmd_ID_move_left_alpha_numeric_boundary = 61;
static i32 fcoder_metacmd_ID_move_right_alpha_numeric_or_camel_boundary = 62;
static i32 fcoder_metacmd_ID_move_left_alpha_numeric_or_camel_boundary = 63;
static i32 fcoder_metacmd_ID_select_all = 64;
static i32 fcoder_metacmd_ID_to_uppercase = 65;
static i32 fcoder_metacmd_ID_to_lowercase = 66;
static i32 fcoder_metacmd_ID_clean_all_lines = 67;
static i32 fcoder_metacmd_ID_basic_change_active_panel = 68;
static i32 fcoder_metacmd_ID_close_panel = 69;
static i32 fcoder_metacmd_ID_show_scrollbar = 70;
static i32 fcoder_metacmd_ID_hide_scrollbar = 71;
static i32 fcoder_metacmd_ID_show_filebar = 72;
static i32 fcoder_metacmd_ID_hide_filebar = 73;
static i32 fcoder_metacmd_ID_toggle_filebar = 74;
static i32 fcoder_metacmd_ID_toggle_fps_meter = 75;
static i32 fcoder_metacmd_ID_increase_face_size = 76;
static i32 fcoder_metacmd_ID_decrease_face_size = 77;
static i32 fcoder_metacmd_ID_mouse_wheel_change_face_size = 78;
static i32 fcoder_metacmd_ID_toggle_virtual_whitespace = 79;
static i32 fcoder_metacmd_ID_toggle_show_whitespace = 80;
static i32 fcoder_metacmd_ID_toggle_line_numbers = 81;
static i32 fcoder_metacmd_ID_eol_dosify = 82;
static i32 fcoder_metacmd_ID_eol_nixify = 83;
static i32 fcoder_metacmd_ID_exit_4coder = 84;
static i32 fcoder_metacmd_ID_goto_line = 85;
static i32 fcoder_metacmd_ID_search = 86;
static i32 fcoder_metacmd_ID_reverse_search = 87;
static i32 fcoder_metacmd_ID_search_identifier = 88;
static i32 fcoder_metacmd_ID_reverse_search_identifier = 89;
static i32 fcoder_metacmd_ID_replace_in_range = 90;
static i32 fcoder_metacmd_ID_replace_in_buffer = 91;
static i32 fcoder_metacmd_ID_replace_in_all_buffers = 92;
static i32 fcoder_metacmd_ID_query_replace = 93;
static i32 fcoder_metacmd_ID_query_replace_identifier = 94;
static i32 fcoder_metacmd_ID_query_replace_selection = 95;
static i32 fcoder_metacmd_ID_save_all_dirty_buffers = 96;
static i32 fcoder_metacmd_ID_delete_file_query = 97;
static i32 fcoder_metacmd_ID_save_to_query = 98;
static i32 fcoder_metacmd_ID_rename_file_query = 99;
static i32 fcoder_metacmd_ID_make_directory_query = 100;
static i32 fcoder_metacmd_ID_move_line_up = 101;
static i32 fcoder_metacmd_ID_move_line_down = 102;
static i32 fcoder_metacmd_ID_duplicate_line = 103;
static i32 fcoder_metacmd_ID_delete_line = 104;
static i32 fcoder_metacmd_ID_open_file_in_quotes = 105;
static i32 fcoder_metacmd_ID_open_matching_file_cpp = 106;
static i32 fcoder_metacmd_ID_view_buffer_other_panel = 107;
static i32 fcoder_metacmd_ID_swap_buffers_between_panels = 108;
static i32 fcoder_metacmd_ID_kill_buffer = 109;
static i32 fcoder_metacmd_ID_save = 110;
static i32 fcoder_metacmd_ID_reopen = 111;
static i32 fcoder_metacmd_ID_undo = 112;
static i32 fcoder_metacmd_ID_redo = 113;
static i32 fcoder_metacmd_ID_undo_all_buffers = 114;
static i32 fcoder_metacmd_ID_redo_all_buffers = 115;
static i32 fcoder_metacmd_ID_open_in_other = 116;
static i32 fcoder_metacmd_ID_lister__quit = 117;
static i32 fcoder_metacmd_ID_lister__activate = 118;
static i32 fcoder_metacmd_ID_lister__write_character = 119;
static i32 fcoder_metacmd_ID_lister__backspace_text_field = 120;
static i32 fcoder_metacmd_ID_lister__move_up = 121;
static i32 fcoder_metacmd_ID_lister__move_down = 122;
static i32 fcoder_metacmd_ID_lister__wheel_scroll = 123;
static i32 fcoder_metacmd_ID_lister__mouse_press = 124;
static i32 fcoder_metacmd_ID_lister__mouse_release = 125;
static i32 fcoder_metacmd_ID_lister__repaint = 126;
static i32 fcoder_metacmd_ID_lister__write_character__default = 127;
static i32 fcoder_metacmd_ID_lister__backspace_text_field__default = 128;
static i32 fcoder_metacmd_ID_lister__move_up__default = 129;
static i32 fcoder_metacmd_ID_lister__move_down__default = 130;
static i32 fcoder_metacmd_ID_lister__write_character__file_path = 131;
static i32 fcoder_metacmd_ID_lister__backspace_text_field__file_path = 132;
static i32 fcoder_metacmd_ID_lister__write_character__fixed_list = 133;
static i32 fcoder_metacmd_ID_interactive_switch_buffer = 134;
static i32 fcoder_metacmd_ID_interactive_kill_buffer = 135;
static i32 fcoder_metacmd_ID_interactive_open_or_new = 136;
static i32 fcoder_metacmd_ID_interactive_new = 137;
static i32 fcoder_metacmd_ID_interactive_open = 138;
static i32 fcoder_metacmd_ID_command_lister = 139;
static i32 fcoder_metacmd_ID_auto_tab_whole_file = 140;
static i32 fcoder_metacmd_ID_auto_tab_line_at_cursor = 141;
static i32 fcoder_metacmd_ID_auto_tab_range = 142;
static i32 fcoder_metacmd_ID_write_and_auto_tab = 143;
static i32 fcoder_metacmd_ID_list_all_locations = 144;
static i32 fcoder_metacmd_ID_list_all_substring_locations = 145;
static i32 fcoder_metacmd_ID_list_all_locations_case_insensitive = 146;
static i32 fcoder_metacmd_ID_list_all_substring_locations_case_insensitive = 147;
static i32 fcoder_metacmd_ID_list_all_locations_of_identifier = 148;
static i32 fcoder_metacmd_ID_list_all_locations_of_identifier_case_insensitive = 149;
static i32 fcoder_metacmd_ID_list_all_locations_of_selection = 150;
static i32 fcoder_metacmd_ID_list_all_locations_of_selection_case_insensitive = 151;
static i32 fcoder_metacmd_ID_list_all_locations_of_type_definition = 152;
static i32 fcoder_metacmd_ID_list_all_locations_of_type_definition_of_identifier = 153;
static i32 fcoder_metacmd_ID_word_complete = 154;
static i32 fcoder_metacmd_ID_goto_jump_at_cursor = 155;
static i32 fcoder_metacmd_ID_goto_jump_at_cursor_same_panel = 156;
static i32 fcoder_metacmd_ID_goto_next_jump = 157;
static i32 fcoder_metacmd_ID_goto_prev_jump = 158;
static i32 fcoder_metacmd_ID_goto_next_jump_no_skips = 159;
static i32 fcoder_metacmd_ID_goto_prev_jump_no_skips = 160;
static i32 fcoder_metacmd_ID_goto_first_jump = 161;
static i32 fcoder_metacmd_ID_goto_first_jump_same_panel_sticky = 162;
static i32 fcoder_metacmd_ID_newline_or_goto_position = 163;
static i32 fcoder_metacmd_ID_newline_or_goto_position_same_panel = 164;
static i32 fcoder_metacmd_ID_view_jump_list_with_lister = 165;
static i32 fcoder_metacmd_ID_log_graph__escape = 166;
static i32 fcoder_metacmd_ID_log_graph__scroll_wheel = 167;
static i32 fcoder_metacmd_ID_log_graph__page_up = 168;
static i32 fcoder_metacmd_ID_log_graph__page_down = 169;
static i32 fcoder_metacmd_ID_log_graph__click_select_event = 170;
static i32 fcoder_metacmd_ID_log_graph__click_jump_to_event_source = 171;
static i32 fcoder_metacmd_ID_show_the_log_graph = 172;
static i32 fcoder_metacmd_ID_copy = 173;
static i32 fcoder_metacmd_ID_cut = 174;
static i32 fcoder_metacmd_ID_paste = 175;
static i32 fcoder_metacmd_ID_paste_next = 176;
static i32 fcoder_metacmd_ID_paste_and_indent = 177;
static i32 fcoder_metacmd_ID_paste_next_and_indent = 178;
static i32 fcoder_metacmd_ID_execute_previous_cli = 179;
static i32 fcoder_metacmd_ID_execute_any_cli = 180;
static i32 fcoder_metacmd_ID_build_search = 181;
static i32 fcoder_metacmd_ID_build_in_build_panel = 182;
static i32 fcoder_metacmd_ID_close_build_panel = 183;
static i32 fcoder_metacmd_ID_change_to_build_panel = 184;
static i32 fcoder_metacmd_ID_close_all_code = 185;
static i32 fcoder_metacmd_ID_open_all_code = 186;
static i32 fcoder_metacmd_ID_open_all_code_recursive = 187;
static i32 fcoder_metacmd_ID_load_project = 188;
static i32 fcoder_metacmd_ID_project_fkey_command = 189;
static i32 fcoder_metacmd_ID_project_go_to_root_directory = 190;
static i32 fcoder_metacmd_ID_setup_new_project = 191;
static i32 fcoder_metacmd_ID_setup_build_bat = 192;
static i32 fcoder_metacmd_ID_setup_build_sh = 193;
static i32 fcoder_metacmd_ID_setup_build_bat_and_sh = 194;
static i32 fcoder_metacmd_ID_project_command_lister = 195;
static i32 fcoder_metacmd_ID_list_all_functions_current_buffer = 196;
static i32 fcoder_metacmd_ID_list_all_functions_current_buffer_lister = 197;
static i32 fcoder_metacmd_ID_list_all_functions_all_buffers = 198;
static i32 fcoder_metacmd_ID_list_all_functions_all_buffers_lister = 199;
static i32 fcoder_metacmd_ID_select_surrounding_scope = 200;
static i32 fcoder_metacmd_ID_select_next_scope_absolute = 201;
static i32 fcoder_metacmd_ID_select_prev_scope_absolute = 202;
static i32 fcoder_metacmd_ID_place_in_scope = 203;
static i32 fcoder_metacmd_ID_delete_current_scope = 204;
static i32 fcoder_metacmd_ID_open_long_braces = 205;
static i32 fcoder_metacmd_ID_open_long_braces_semicolon = 206;
static i32 fcoder_metacmd_ID_open_long_braces_break = 207;
static i32 fcoder_metacmd_ID_if0_off = 208;
static i32 fcoder_metacmd_ID_write_todo = 209;
static i32 fcoder_metacmd_ID_write_hack = 210;
static i32 fcoder_metacmd_ID_write_note = 211;
static i32 fcoder_metacmd_ID_write_block = 212;
static i32 fcoder_metacmd_ID_write_zero_struct = 213;
static i32 fcoder_metacmd_ID_comment_line = 214;
static i32 fcoder_metacmd_ID_uncomment_line = 215;
static i32 fcoder_metacmd_ID_comment_line_toggle = 216;
static i32 fcoder_metacmd_ID_snippet_lister = 217;
static i32 fcoder_metacmd_ID_miblo_increment_basic = 218;
static i32 fcoder_metacmd_ID_miblo_decrement_basic = 219;
static i32 fcoder_metacmd_ID_miblo_increment_time_stamp = 220;
static i32 fcoder_metacmd_ID_miblo_decrement_time_stamp = 221;
static i32 fcoder_metacmd_ID_miblo_increment_time_stamp_minute = 222;
static i32 fcoder_metacmd_ID_miblo_decrement_time_stamp_minute = 223;
static i32 fcoder_metacmd_ID_set_bindings_choose = 224;
static i32 fcoder_metacmd_ID_set_bindings_default = 225;
#endif
