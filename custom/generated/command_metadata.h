#if !defined(META_PASS)
#define command_id(c) (fcoder_metacmd_ID_##c)
#define command_metadata(c) (&fcoder_metacmd_table[command_id(c)])
#define command_metadata_by_id(id) (&fcoder_metacmd_table[id])
#define command_one_past_last_id 211
#if defined(CUSTOM_COMMAND_SIG)
#define PROC_LINKS(x,y) x
#else
#define PROC_LINKS(x,y) y
#endif
#if defined(CUSTOM_COMMAND_SIG)
CUSTOM_COMMAND_SIG(default_view_input_handler);
CUSTOM_COMMAND_SIG(profile_enable);
CUSTOM_COMMAND_SIG(profile_disable);
CUSTOM_COMMAND_SIG(profile_clear);
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
CUSTOM_COMMAND_SIG(write_text_input);
CUSTOM_COMMAND_SIG(write_space);
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
CUSTOM_COMMAND_SIG(default_file_externally_modified);
CUSTOM_COMMAND_SIG(set_eol_mode_to_crlf);
CUSTOM_COMMAND_SIG(set_eol_mode_to_lf);
CUSTOM_COMMAND_SIG(set_eol_mode_to_binary);
CUSTOM_COMMAND_SIG(set_eol_mode_from_contents);
CUSTOM_COMMAND_SIG(interactive_switch_buffer);
CUSTOM_COMMAND_SIG(interactive_kill_buffer);
CUSTOM_COMMAND_SIG(interactive_open_or_new);
CUSTOM_COMMAND_SIG(interactive_new);
CUSTOM_COMMAND_SIG(interactive_open);
CUSTOM_COMMAND_SIG(command_lister);
CUSTOM_COMMAND_SIG(auto_indent_whole_file);
CUSTOM_COMMAND_SIG(auto_indent_line_at_cursor);
CUSTOM_COMMAND_SIG(auto_indent_range);
CUSTOM_COMMAND_SIG(write_text_and_auto_indent);
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
CUSTOM_COMMAND_SIG(if_read_only_goto_position);
CUSTOM_COMMAND_SIG(if_read_only_goto_position_same_panel);
CUSTOM_COMMAND_SIG(view_jump_list_with_lister);
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
CUSTOM_COMMAND_SIG(select_next_scope_after_current);
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
CUSTOM_COMMAND_SIG(profile_inspect);
CUSTOM_COMMAND_SIG(default_startup);
CUSTOM_COMMAND_SIG(default_try_exit);
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
static Command_Metadata fcoder_metacmd_table[211] = {
{ PROC_LINKS(default_view_input_handler, 0), "default_view_input_handler", 26,  "Input consumption loop for default view behavior", 48, "c:\\4ed\\code\\custom\\4coder_default_hooks.cpp", 43, 56 },
{ PROC_LINKS(profile_enable, 0), "profile_enable", 14,  "Allow 4coder's self profiler to gather new profiling information.", 65, "c:\\4ed\\code\\custom\\4coder_profile.cpp", 37, 125 },
{ PROC_LINKS(profile_disable, 0), "profile_disable", 15,  "Prevent 4coder's self profiler from gathering new profiling information.", 72, "c:\\4ed\\code\\custom\\4coder_profile.cpp", 37, 131 },
{ PROC_LINKS(profile_clear, 0), "profile_clear", 13,  "Clear all profiling information from 4coder's self profiler.", 60, "c:\\4ed\\code\\custom\\4coder_profile.cpp", 37, 137 },
{ PROC_LINKS(seek_beginning_of_textual_line, 0), "seek_beginning_of_textual_line", 30,  "Seeks the cursor to the beginning of the line across all text.", 62, "c:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2095 },
{ PROC_LINKS(seek_end_of_textual_line, 0), "seek_end_of_textual_line", 24,  "Seeks the cursor to the end of the line across all text.", 56, "c:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2101 },
{ PROC_LINKS(seek_beginning_of_line, 0), "seek_beginning_of_line", 22,  "Seeks the cursor to the beginning of the visual line.", 53, "c:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2107 },
{ PROC_LINKS(seek_end_of_line, 0), "seek_end_of_line", 16,  "Seeks the cursor to the end of the visual line.", 47, "c:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2113 },
{ PROC_LINKS(goto_beginning_of_file, 0), "goto_beginning_of_file", 22,  "Sets the cursor to the beginning of the file.", 45, "c:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2119 },
{ PROC_LINKS(goto_end_of_file, 0), "goto_end_of_file", 16,  "Sets the cursor to the end of the file.", 39, "c:\\4ed\\code\\custom\\4coder_helper.cpp", 36, 2127 },
{ PROC_LINKS(change_active_panel, 0), "change_active_panel", 19,  "Change the currently active panel, moving to the panel with the next highest view_id.", 85, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 203 },
{ PROC_LINKS(change_active_panel_backwards, 0), "change_active_panel_backwards", 29,  "Change the currently active panel, moving to the panel with the next lowest view_id.", 84, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 213 },
{ PROC_LINKS(open_panel_vsplit, 0), "open_panel_vsplit", 17,  "Create a new panel by vertically splitting the active panel.", 60, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 223 },
{ PROC_LINKS(open_panel_hsplit, 0), "open_panel_hsplit", 17,  "Create a new panel by horizontally splitting the active panel.", 62, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 233 },
{ PROC_LINKS(suppress_mouse, 0), "suppress_mouse", 14,  "Hides the mouse and causes all mosue input (clicks, position, wheel) to be ignored.", 83, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 298 },
{ PROC_LINKS(allow_mouse, 0), "allow_mouse", 11,  "Shows the mouse and causes all mouse input to be processed normally.", 68, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 304 },
{ PROC_LINKS(toggle_mouse, 0), "toggle_mouse", 12,  "Toggles the mouse suppression mode, see suppress_mouse and allow_mouse.", 71, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 310 },
{ PROC_LINKS(set_mode_to_original, 0), "set_mode_to_original", 20,  "Sets the edit mode to 4coder original.", 38, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 316 },
{ PROC_LINKS(set_mode_to_notepad_like, 0), "set_mode_to_notepad_like", 24,  "Sets the edit mode to Notepad like.", 35, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 322 },
{ PROC_LINKS(toggle_highlight_line_at_cursor, 0), "toggle_highlight_line_at_cursor", 31,  "Toggles the line highlight at the cursor.", 41, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 328 },
{ PROC_LINKS(toggle_highlight_enclosing_scopes, 0), "toggle_highlight_enclosing_scopes", 33,  "In code files scopes surrounding the cursor are highlighted with distinguishing colors.", 87, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 334 },
{ PROC_LINKS(toggle_paren_matching_helper, 0), "toggle_paren_matching_helper", 28,  "In code files matching parentheses pairs are colored with distinguishing colors.", 80, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 340 },
{ PROC_LINKS(toggle_fullscreen, 0), "toggle_fullscreen", 17,  "Toggle fullscreen mode on or off.  The change(s) do not take effect until the next frame.", 89, "c:\\4ed\\code\\custom\\4coder_default_framework.cpp", 47, 346 },
{ PROC_LINKS(write_text_input, 0), "write_text_input", 16,  "Inserts whatever character was used to trigger this command.", 60, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 58 },
{ PROC_LINKS(write_space, 0), "write_space", 11,  "Inserts an underscore.", 22, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 66 },
{ PROC_LINKS(write_underscore, 0), "write_underscore", 16,  "Inserts an underscore.", 22, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 72 },
{ PROC_LINKS(delete_char, 0), "delete_char", 11,  "Deletes the character to the right of the cursor.", 49, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 78 },
{ PROC_LINKS(backspace_char, 0), "backspace_char", 14,  "Deletes the character to the left of the cursor.", 48, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 95 },
{ PROC_LINKS(set_mark, 0), "set_mark", 8,  "Sets the mark to the current position of the cursor.", 52, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 114 },
{ PROC_LINKS(cursor_mark_swap, 0), "cursor_mark_swap", 16,  "Swaps the position of the cursor and the mark.", 46, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 123 },
{ PROC_LINKS(delete_range, 0), "delete_range", 12,  "Deletes the text in the range between the cursor and the mark.", 62, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 133 },
{ PROC_LINKS(backspace_alpha_numeric_boundary, 0), "backspace_alpha_numeric_boundary", 32,  "Delete characters between the cursor position and the first alphanumeric boundary to the left.", 94, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 153 },
{ PROC_LINKS(delete_alpha_numeric_boundary, 0), "delete_alpha_numeric_boundary", 29,  "Delete characters between the cursor position and the first alphanumeric boundary to the right.", 95, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 161 },
{ PROC_LINKS(snipe_backward_whitespace_or_token_boundary, 0), "snipe_backward_whitespace_or_token_boundary", 43,  "Delete a single, whole token on or to the left of the cursor and post it to the clipboard.", 90, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 181 },
{ PROC_LINKS(snipe_forward_whitespace_or_token_boundary, 0), "snipe_forward_whitespace_or_token_boundary", 42,  "Delete a single, whole token on or to the right of the cursor and post it to the clipboard.", 91, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 189 },
{ PROC_LINKS(center_view, 0), "center_view", 11,  "Centers the view vertically on the line on which the cursor sits.", 65, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 199 },
{ PROC_LINKS(left_adjust_view, 0), "left_adjust_view", 16,  "Sets the left size of the view near the x position of the cursor.", 65, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 213 },
{ PROC_LINKS(click_set_cursor_and_mark, 0), "click_set_cursor_and_mark", 25,  "Sets the cursor position and mark to the mouse position.", 56, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 225 },
{ PROC_LINKS(click_set_cursor, 0), "click_set_cursor", 16,  "Sets the cursor position to the mouse position.", 47, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 235 },
{ PROC_LINKS(click_set_cursor_if_lbutton, 0), "click_set_cursor_if_lbutton", 27,  "If the mouse left button is pressed, sets the cursor position to the mouse position.", 84, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 245 },
{ PROC_LINKS(click_set_mark, 0), "click_set_mark", 14,  "Sets the mark position to the mouse position.", 45, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 257 },
{ PROC_LINKS(mouse_wheel_scroll, 0), "mouse_wheel_scroll", 18,  "Reads the scroll wheel value from the mouse state and scrolls accordingly.", 74, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 267 },
{ PROC_LINKS(move_up, 0), "move_up", 7,  "Moves the cursor up one line.", 29, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 321 },
{ PROC_LINKS(move_down, 0), "move_down", 9,  "Moves the cursor down one line.", 31, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 327 },
{ PROC_LINKS(move_up_10, 0), "move_up_10", 10,  "Moves the cursor up ten lines.", 30, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 333 },
{ PROC_LINKS(move_down_10, 0), "move_down_10", 12,  "Moves the cursor down ten lines.", 32, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 339 },
{ PROC_LINKS(move_down_textual, 0), "move_down_textual", 17,  "Moves down to the next line of actual text, regardless of line wrapping.", 72, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 345 },
{ PROC_LINKS(page_up, 0), "page_up", 7,  "Scrolls the view up one view height and moves the cursor up one view height.", 76, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 355 },
{ PROC_LINKS(page_down, 0), "page_down", 9,  "Scrolls the view down one view height and moves the cursor down one view height.", 80, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 363 },
{ PROC_LINKS(move_up_to_blank_line, 0), "move_up_to_blank_line", 21,  "Seeks the cursor up to the next blank line.", 43, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 393 },
{ PROC_LINKS(move_down_to_blank_line, 0), "move_down_to_blank_line", 23,  "Seeks the cursor down to the next blank line.", 45, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 399 },
{ PROC_LINKS(move_up_to_blank_line_skip_whitespace, 0), "move_up_to_blank_line_skip_whitespace", 37,  "Seeks the cursor up to the next blank line and places it at the end of the line.", 80, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 405 },
{ PROC_LINKS(move_down_to_blank_line_skip_whitespace, 0), "move_down_to_blank_line_skip_whitespace", 39,  "Seeks the cursor down to the next blank line and places it at the end of the line.", 82, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 411 },
{ PROC_LINKS(move_up_to_blank_line_end, 0), "move_up_to_blank_line_end", 25,  "Seeks the cursor up to the next blank line and places it at the end of the line.", 80, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 417 },
{ PROC_LINKS(move_down_to_blank_line_end, 0), "move_down_to_blank_line_end", 27,  "Seeks the cursor down to the next blank line and places it at the end of the line.", 82, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 423 },
{ PROC_LINKS(move_left, 0), "move_left", 9,  "Moves the cursor one character to the left.", 43, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 429 },
{ PROC_LINKS(move_right, 0), "move_right", 10,  "Moves the cursor one character to the right.", 44, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 441 },
{ PROC_LINKS(move_right_whitespace_boundary, 0), "move_right_whitespace_boundary", 30,  "Seek right for the next boundary between whitespace and non-whitespace.", 71, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 463 },
{ PROC_LINKS(move_left_whitespace_boundary, 0), "move_left_whitespace_boundary", 29,  "Seek left for the next boundary between whitespace and non-whitespace.", 70, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 471 },
{ PROC_LINKS(move_right_token_boundary, 0), "move_right_token_boundary", 25,  "Seek right for the next end of a token.", 39, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 479 },
{ PROC_LINKS(move_left_token_boundary, 0), "move_left_token_boundary", 24,  "Seek left for the next beginning of a token.", 44, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 487 },
{ PROC_LINKS(move_right_whitespace_or_token_boundary, 0), "move_right_whitespace_or_token_boundary", 39,  "Seek right for the next end of a token or boundary between whitespace and non-whitespace.", 89, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 495 },
{ PROC_LINKS(move_left_whitespace_or_token_boundary, 0), "move_left_whitespace_or_token_boundary", 38,  "Seek left for the next end of a token or boundary between whitespace and non-whitespace.", 88, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 503 },
{ PROC_LINKS(move_right_alpha_numeric_boundary, 0), "move_right_alpha_numeric_boundary", 33,  "Seek right for boundary between alphanumeric characters and non-alphanumeric characters.", 88, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 511 },
{ PROC_LINKS(move_left_alpha_numeric_boundary, 0), "move_left_alpha_numeric_boundary", 32,  "Seek left for boundary between alphanumeric characters and non-alphanumeric characters.", 87, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 519 },
{ PROC_LINKS(move_right_alpha_numeric_or_camel_boundary, 0), "move_right_alpha_numeric_or_camel_boundary", 42,  "Seek right for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.", 107, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 527 },
{ PROC_LINKS(move_left_alpha_numeric_or_camel_boundary, 0), "move_left_alpha_numeric_or_camel_boundary", 41,  "Seek left for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.", 106, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 535 },
{ PROC_LINKS(select_all, 0), "select_all", 10,  "Puts the cursor at the top of the file, and the mark at the bottom of the file.", 79, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 545 },
{ PROC_LINKS(to_uppercase, 0), "to_uppercase", 12,  "Converts all ascii text in the range between the cursor and the mark to uppercase.", 82, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 558 },
{ PROC_LINKS(to_lowercase, 0), "to_lowercase", 12,  "Converts all ascii text in the range between the cursor and the mark to lowercase.", 82, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 571 },
{ PROC_LINKS(clean_all_lines, 0), "clean_all_lines", 15,  "Removes trailing whitespace from all lines in the current buffer.", 65, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 584 },
{ PROC_LINKS(basic_change_active_panel, 0), "basic_change_active_panel", 25,  "Change the currently active panel, moving to the panel with the next highest view_id.  Will not skipe the build panel if it is open.", 132, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 618 },
{ PROC_LINKS(close_panel, 0), "close_panel", 11,  "Closes the currently active panel if it is not the only panel open.", 67, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 626 },
{ PROC_LINKS(show_scrollbar, 0), "show_scrollbar", 14,  "Sets the current view to show it's scrollbar.", 45, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 635 },
{ PROC_LINKS(hide_scrollbar, 0), "hide_scrollbar", 14,  "Sets the current view to hide it's scrollbar.", 45, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 642 },
{ PROC_LINKS(show_filebar, 0), "show_filebar", 12,  "Sets the current view to show it's filebar.", 43, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 649 },
{ PROC_LINKS(hide_filebar, 0), "hide_filebar", 12,  "Sets the current view to hide it's filebar.", 43, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 656 },
{ PROC_LINKS(toggle_filebar, 0), "toggle_filebar", 14,  "Toggles the visibility status of the current view's filebar.", 60, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 663 },
{ PROC_LINKS(toggle_fps_meter, 0), "toggle_fps_meter", 16,  "Toggles the visibility of the FPS performance meter", 51, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 672 },
{ PROC_LINKS(increase_face_size, 0), "increase_face_size", 18,  "Increase the size of the face used by the current buffer.", 57, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 678 },
{ PROC_LINKS(decrease_face_size, 0), "decrease_face_size", 18,  "Decrease the size of the face used by the current buffer.", 57, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 689 },
{ PROC_LINKS(mouse_wheel_change_face_size, 0), "mouse_wheel_change_face_size", 28,  "Reads the state of the mouse wheel and uses it to either increase or decrease the face size.", 92, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 700 },
{ PROC_LINKS(toggle_virtual_whitespace, 0), "toggle_virtual_whitespace", 25,  "Toggles the current buffer's virtual whitespace status.", 55, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 717 },
{ PROC_LINKS(toggle_show_whitespace, 0), "toggle_show_whitespace", 22,  "Toggles the current buffer's whitespace visibility status.", 58, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 726 },
{ PROC_LINKS(toggle_line_numbers, 0), "toggle_line_numbers", 19,  "Toggles the left margin line numbers.", 37, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 735 },
{ PROC_LINKS(exit_4coder, 0), "exit_4coder", 11,  "Attempts to close 4coder.", 25, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 741 },
{ PROC_LINKS(goto_line, 0), "goto_line", 9,  "Queries the user for a number, and jumps the cursor to the corresponding line.", 78, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 749 },
{ PROC_LINKS(search, 0), "search", 6,  "Begins an incremental search down through the current buffer for a user specified string.", 89, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 977 },
{ PROC_LINKS(reverse_search, 0), "reverse_search", 14,  "Begins an incremental search up through the current buffer for a user specified string.", 87, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 983 },
{ PROC_LINKS(search_identifier, 0), "search_identifier", 17,  "Begins an incremental search down through the current buffer for the word or token under the cursor.", 100, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 989 },
{ PROC_LINKS(reverse_search_identifier, 0), "reverse_search_identifier", 25,  "Begins an incremental search up through the current buffer for the word or token under the cursor.", 98, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 995 },
{ PROC_LINKS(replace_in_range, 0), "replace_in_range", 16,  "Queries the user for a needle and string. Replaces all occurences of needle with string in the range between cursor and the mark in the active buffer.", 150, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1042 },
{ PROC_LINKS(replace_in_buffer, 0), "replace_in_buffer", 17,  "Queries the user for a needle and string. Replaces all occurences of needle with string in the active buffer.", 109, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1051 },
{ PROC_LINKS(replace_in_all_buffers, 0), "replace_in_all_buffers", 22,  "Queries the user for a needle and string. Replaces all occurences of needle with string in all editable buffers.", 112, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1060 },
{ PROC_LINKS(query_replace, 0), "query_replace", 13,  "Queries the user for two strings, and incrementally replaces every occurence of the first string with the second string.", 120, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1150 },
{ PROC_LINKS(query_replace_identifier, 0), "query_replace_identifier", 24,  "Queries the user for a string, and incrementally replace every occurence of the word or token found at the cursor with the specified string.", 140, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1171 },
{ PROC_LINKS(query_replace_selection, 0), "query_replace_selection", 23,  "Queries the user for a string, and incrementally replace every occurence of the string found in the selected range with the specified string.", 141, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1187 },
{ PROC_LINKS(save_all_dirty_buffers, 0), "save_all_dirty_buffers", 22,  "Saves all buffers marked dirty (showing the '*' indicator).", 59, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1222 },
{ PROC_LINKS(delete_file_query, 0), "delete_file_query", 17,  "Deletes the file of the current buffer if 4coder has the appropriate access rights. Will ask the user for confirmation first.", 125, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1247 },
{ PROC_LINKS(save_to_query, 0), "save_to_query", 13,  "Queries the user for a file name and saves the contents of the current buffer, altering the buffer's name too.", 110, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1291 },
{ PROC_LINKS(rename_file_query, 0), "rename_file_query", 17,  "Queries the user for a new name and renames the file of the current buffer, altering the buffer's name too.", 107, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1324 },
{ PROC_LINKS(make_directory_query, 0), "make_directory_query", 20,  "Queries the user for a name and creates a new directory with the given name.", 76, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1362 },
{ PROC_LINKS(move_line_up, 0), "move_line_up", 12,  "Swaps the line under the cursor with the line above it, and moves the cursor up with it.", 88, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1396 },
{ PROC_LINKS(move_line_down, 0), "move_line_down", 14,  "Swaps the line under the cursor with the line below it, and moves the cursor down with it.", 90, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1402 },
{ PROC_LINKS(duplicate_line, 0), "duplicate_line", 14,  "Create a copy of the line on which the cursor sits.", 51, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1408 },
{ PROC_LINKS(delete_line, 0), "delete_line", 11,  "Delete the line the on which the cursor sits.", 45, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1422 },
{ PROC_LINKS(open_file_in_quotes, 0), "open_file_in_quotes", 19,  "Reads a filename from surrounding '\"' characters and attempts to open the corresponding file.", 94, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1487 },
{ PROC_LINKS(open_matching_file_cpp, 0), "open_matching_file_cpp", 22,  "If the current file is a *.cpp or *.h, attempts to open the corresponding *.h or *.cpp file in the other view.", 110, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1519 },
{ PROC_LINKS(view_buffer_other_panel, 0), "view_buffer_other_panel", 23,  "Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.", 104, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1532 },
{ PROC_LINKS(swap_buffers_between_panels, 0), "swap_buffers_between_panels", 27,  "Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.", 104, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1544 },
{ PROC_LINKS(kill_buffer, 0), "kill_buffer", 11,  "Kills the current buffer.", 25, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1578 },
{ PROC_LINKS(save, 0), "save", 4,  "Saves the current buffer.", 25, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1586 },
{ PROC_LINKS(reopen, 0), "reopen", 6,  "Reopen the current buffer from the hard drive.", 46, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1596 },
{ PROC_LINKS(undo, 0), "undo", 4,  "Advances backwards through the undo history of the current buffer.", 66, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1825 },
{ PROC_LINKS(redo, 0), "redo", 4,  "Advances forwards through the undo history of the current buffer.", 65, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1838 },
{ PROC_LINKS(undo_all_buffers, 0), "undo_all_buffers", 16,  "Advances backward through the undo history in the buffer containing the most recent regular edit.", 97, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1852 },
{ PROC_LINKS(redo_all_buffers, 0), "redo_all_buffers", 16,  "Advances forward through the undo history in the buffer containing the most recent regular edit.", 96, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 1923 },
{ PROC_LINKS(open_in_other, 0), "open_in_other", 13,  "Interactively opens a file in the other panel.", 46, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 2024 },
{ PROC_LINKS(default_file_externally_modified, 0), "default_file_externally_modified", 32,  "Notes the external modification of attached files by printing a message.", 72, "c:\\4ed\\code\\custom\\4coder_base_commands.cpp", 43, 2031 },
{ PROC_LINKS(set_eol_mode_to_crlf, 0), "set_eol_mode_to_crlf", 20,  "Puts the buffer in crlf line ending mode.", 41, "c:\\4ed\\code\\custom\\4coder_eol.cpp", 33, 80 },
{ PROC_LINKS(set_eol_mode_to_lf, 0), "set_eol_mode_to_lf", 18,  "Puts the buffer in lf line ending mode.", 39, "c:\\4ed\\code\\custom\\4coder_eol.cpp", 33, 91 },
{ PROC_LINKS(set_eol_mode_to_binary, 0), "set_eol_mode_to_binary", 22,  "Puts the buffer in bin line ending mode.", 40, "c:\\4ed\\code\\custom\\4coder_eol.cpp", 33, 102 },
{ PROC_LINKS(set_eol_mode_from_contents, 0), "set_eol_mode_from_contents", 26,  "Sets the buffer's line ending mode to match the contents of the buffer.", 71, "c:\\4ed\\code\\custom\\4coder_eol.cpp", 33, 113 },
{ PROC_LINKS(interactive_switch_buffer, 0), "interactive_switch_buffer", 25,  "Interactively switch to an open buffer.", 39, "c:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 487 },
{ PROC_LINKS(interactive_kill_buffer, 0), "interactive_kill_buffer", 23,  "Interactively kill an open buffer.", 34, "c:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 506 },
{ PROC_LINKS(interactive_open_or_new, 0), "interactive_open_or_new", 23,  "Interactively open a file out of the file system.", 49, "c:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 577 },
{ PROC_LINKS(interactive_new, 0), "interactive_new", 15,  "Interactively creates a new file.", 33, "c:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 616 },
{ PROC_LINKS(interactive_open, 0), "interactive_open", 16,  "Interactively opens a file.", 27, "c:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 649 },
{ PROC_LINKS(command_lister, 0), "command_lister", 14,  "Opens an interactive list of all registered commands.", 53, "c:\\4ed\\code\\custom\\4coder_lists.cpp", 35, 712 },
{ PROC_LINKS(auto_indent_whole_file, 0), "auto_indent_whole_file", 22,  "Audo-indents the entire current buffer.", 39, "c:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 356 },
{ PROC_LINKS(auto_indent_line_at_cursor, 0), "auto_indent_line_at_cursor", 26,  "Auto-indents the line on which the cursor sits.", 47, "c:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 365 },
{ PROC_LINKS(auto_indent_range, 0), "auto_indent_range", 17,  "Auto-indents the range between the cursor and the mark.", 55, "c:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 375 },
{ PROC_LINKS(write_text_and_auto_indent, 0), "write_text_and_auto_indent", 26,  "Inserts text and auto-indents the line on which the cursor sits if any of the text contains 'layout punctuation' such as ;:{}()[]# and new lines.", 145, "c:\\4ed\\code\\custom\\4coder_auto_indent.cpp", 41, 385 },
{ PROC_LINKS(list_all_locations, 0), "list_all_locations", 18,  "Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.", 99, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 166 },
{ PROC_LINKS(list_all_substring_locations, 0), "list_all_substring_locations", 28,  "Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.", 103, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 172 },
{ PROC_LINKS(list_all_locations_case_insensitive, 0), "list_all_locations_case_insensitive", 35,  "Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.", 101, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 178 },
{ PROC_LINKS(list_all_substring_locations_case_insensitive, 0), "list_all_substring_locations_case_insensitive", 45,  "Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.", 105, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 184 },
{ PROC_LINKS(list_all_locations_of_identifier, 0), "list_all_locations_of_identifier", 32,  "Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.", 102, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 190 },
{ PROC_LINKS(list_all_locations_of_identifier_case_insensitive, 0), "list_all_locations_of_identifier_case_insensitive", 49,  "Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.", 104, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 196 },
{ PROC_LINKS(list_all_locations_of_selection, 0), "list_all_locations_of_selection", 31,  "Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.", 102, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 202 },
{ PROC_LINKS(list_all_locations_of_selection_case_insensitive, 0), "list_all_locations_of_selection_case_insensitive", 48,  "Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.", 104, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 208 },
{ PROC_LINKS(list_all_locations_of_type_definition, 0), "list_all_locations_of_type_definition", 37,  "Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.", 121, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 214 },
{ PROC_LINKS(list_all_locations_of_type_definition_of_identifier, 0), "list_all_locations_of_type_definition_of_identifier", 51,  "Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.", 125, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 222 },
{ PROC_LINKS(word_complete, 0), "word_complete", 13,  "Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.", 130, "c:\\4ed\\code\\custom\\4coder_search.cpp", 36, 382 },
{ PROC_LINKS(goto_jump_at_cursor, 0), "goto_jump_at_cursor", 19,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.", 187, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 346 },
{ PROC_LINKS(goto_jump_at_cursor_same_panel, 0), "goto_jump_at_cursor_same_panel", 30,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list.", 167, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 373 },
{ PROC_LINKS(goto_next_jump, 0), "goto_next_jump", 14,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.", 123, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 462 },
{ PROC_LINKS(goto_prev_jump, 0), "goto_prev_jump", 14,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations.", 127, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 479 },
{ PROC_LINKS(goto_next_jump_no_skips, 0), "goto_next_jump_no_skips", 23,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.", 132, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 492 },
{ PROC_LINKS(goto_prev_jump_no_skips, 0), "goto_prev_jump_no_skips", 23,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.", 136, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 509 },
{ PROC_LINKS(goto_first_jump, 0), "goto_first_jump", 15,  "If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.", 95, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 523 },
{ PROC_LINKS(goto_first_jump_same_panel_sticky, 0), "goto_first_jump_same_panel_sticky", 33,  "If a buffer containing jump locations has been locked in, goes to the first jump in the buffer and views the buffer in the panel where the jump list was.", 153, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 540 },
{ PROC_LINKS(if_read_only_goto_position, 0), "if_read_only_goto_position", 26,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.", 106, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 562 },
{ PROC_LINKS(if_read_only_goto_position_same_panel, 0), "if_read_only_goto_position_same_panel", 37,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.", 117, "c:\\4ed\\code\\custom\\4coder_jump_sticky.cpp", 41, 579 },
{ PROC_LINKS(view_jump_list_with_lister, 0), "view_jump_list_with_lister", 26,  "When executed on a buffer with jumps, creates a persistent lister for all the jumps", 83, "c:\\4ed\\code\\custom\\4coder_jump_lister.cpp", 41, 104 },
{ PROC_LINKS(show_the_log_graph, 0), "show_the_log_graph", 18,  "Parses *log* and displays the 'log graph' UI", 44, "c:\\4ed\\code\\custom\\4coder_log_parser.cpp", 40, 990 },
{ PROC_LINKS(copy, 0), "copy", 4,  "Copy the text in the range from the cursor to the mark onto the clipboard.", 74, "c:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 19 },
{ PROC_LINKS(cut, 0), "cut", 3,  "Cut the text in the range from the cursor to the mark onto the clipboard.", 73, "c:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 28 },
{ PROC_LINKS(paste, 0), "paste", 5,  "At the cursor, insert the text at the top of the clipboard.", 59, "c:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 39 },
{ PROC_LINKS(paste_next, 0), "paste_next", 10,  "If the previous command was paste or paste_next, replaces the paste range with the next text down on the clipboard, otherwise operates as the paste command.", 156, "c:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 73 },
{ PROC_LINKS(paste_and_indent, 0), "paste_and_indent", 16,  "Paste from the top of clipboard and run auto-indent on the newly pasted text.", 77, "c:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 115 },
{ PROC_LINKS(paste_next_and_indent, 0), "paste_next_and_indent", 21,  "Paste the next item on the clipboard and run auto-indent on the newly pasted text.", 82, "c:\\4ed\\code\\custom\\4coder_clipboard.cpp", 39, 122 },
{ PROC_LINKS(execute_previous_cli, 0), "execute_previous_cli", 20,  "If the command execute_any_cli has already been used, this will execute a CLI reusing the most recent buffer name and command.", 126, "c:\\4ed\\code\\custom\\4coder_cli_command.cpp", 41, 7 },
{ PROC_LINKS(execute_any_cli, 0), "execute_any_cli", 15,  "Queries for an output buffer name and system command, runs the system command as a CLI and prints the output to the specified buffer.", 133, "c:\\4ed\\code\\custom\\4coder_cli_command.cpp", 41, 22 },
{ PROC_LINKS(build_search, 0), "build_search", 12,  "Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.", 153, "c:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 128 },
{ PROC_LINKS(build_in_build_panel, 0), "build_in_build_panel", 20,  "Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.  Puts the *compilation* buffer in a panel at the footer of the current view.", 230, "c:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 163 },
{ PROC_LINKS(close_build_panel, 0), "close_build_panel", 17,  "If the special build panel is open, closes it.", 46, "c:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 178 },
{ PROC_LINKS(change_to_build_panel, 0), "change_to_build_panel", 21,  "If the special build panel is open, makes the build panel the active panel.", 75, "c:\\4ed\\code\\custom\\4coder_build_commands.cpp", 44, 184 },
{ PROC_LINKS(close_all_code, 0), "close_all_code", 14,  "Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.", 107, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 917 },
{ PROC_LINKS(open_all_code, 0), "open_all_code", 13,  "Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.", 164, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 923 },
{ PROC_LINKS(open_all_code_recursive, 0), "open_all_code_recursive", 23,  "Works as open_all_code but also runs in all subdirectories.", 59, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 929 },
{ PROC_LINKS(load_project, 0), "load_project", 12,  "Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.", 167, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 937 },
{ PROC_LINKS(project_fkey_command, 0), "project_fkey_command", 20,  "Run an 'fkey command' configured in a project.4coder file.  Determines the index of the 'fkey command' by which function key or numeric key was pressed to trigger the command.", 175, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 944 },
{ PROC_LINKS(project_go_to_root_directory, 0), "project_go_to_root_directory", 28,  "Changes 4coder's hot directory to the root directory of the currently loaded project. With no loaded project nothing hapepns.", 125, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 969 },
{ PROC_LINKS(setup_new_project, 0), "setup_new_project", 17,  "Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.", 120, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1303 },
{ PROC_LINKS(setup_build_bat, 0), "setup_build_bat", 15,  "Queries the user for several configuration options and initializes a new build batch script.", 92, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1310 },
{ PROC_LINKS(setup_build_sh, 0), "setup_build_sh", 14,  "Queries the user for several configuration options and initializes a new build shell script.", 92, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1316 },
{ PROC_LINKS(setup_build_bat_and_sh, 0), "setup_build_bat_and_sh", 22,  "Queries the user for several configuration options and initializes a new build batch script.", 92, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1322 },
{ PROC_LINKS(project_command_lister, 0), "project_command_lister", 22,  "Open a lister of all commands in the currently loaded project.", 62, "c:\\4ed\\code\\custom\\4coder_project_commands.cpp", 46, 1338 },
{ PROC_LINKS(list_all_functions_current_buffer, 0), "list_all_functions_current_buffer", 33,  "Creates a jump list of lines of the current buffer that appear to define or declare functions.", 94, "c:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 267 },
{ PROC_LINKS(list_all_functions_current_buffer_lister, 0), "list_all_functions_current_buffer_lister", 40,  "Creates a lister of locations that look like function definitions and declarations in the buffer.", 97, "c:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 277 },
{ PROC_LINKS(list_all_functions_all_buffers, 0), "list_all_functions_all_buffers", 30,  "Creates a jump list of lines from all buffers that appear to define or declare functions.", 89, "c:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 289 },
{ PROC_LINKS(list_all_functions_all_buffers_lister, 0), "list_all_functions_all_buffers_lister", 37,  "Creates a lister of locations that look like function definitions and declarations all buffers.", 95, "c:\\4ed\\code\\custom\\4coder_function_list.cpp", 43, 295 },
{ PROC_LINKS(select_surrounding_scope, 0), "select_surrounding_scope", 24,  "Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.", 107, "c:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 7 },
{ PROC_LINKS(select_next_scope_absolute, 0), "select_next_scope_absolute", 26,  "Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.", 102, "c:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 32 },
{ PROC_LINKS(select_next_scope_after_current, 0), "select_next_scope_after_current", 31,  "Finds the first scope started by '{' after the mark and puts the cursor and mark on the '{' and '}'.  This command is meant to be used after a scope is already selected so that it will have the effect of selecting the next scope after the current scope.", 253, "c:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 41 },
{ PROC_LINKS(select_prev_scope_absolute, 0), "select_prev_scope_absolute", 26,  "Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.", 103, "c:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 50 },
{ PROC_LINKS(place_in_scope, 0), "place_in_scope", 14,  "Wraps the code contained in the range between cursor and mark with a new curly brace scope.", 91, "c:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 67 },
{ PROC_LINKS(delete_current_scope, 0), "delete_current_scope", 20,  "Deletes the braces surrounding the currently selected scope.  Leaves the contents within the scope.", 99, "c:\\4ed\\code\\custom\\4coder_scope_commands.cpp", 44, 73 },
{ PROC_LINKS(open_long_braces, 0), "open_long_braces", 16,  "At the cursor, insert a '{' and '}' separated by a blank line.", 62, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 46 },
{ PROC_LINKS(open_long_braces_semicolon, 0), "open_long_braces_semicolon", 26,  "At the cursor, insert a '{' and '};' separated by a blank line.", 63, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 54 },
{ PROC_LINKS(open_long_braces_break, 0), "open_long_braces_break", 22,  "At the cursor, insert a '{' and '}break;' separated by a blank line.", 68, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 62 },
{ PROC_LINKS(if0_off, 0), "if0_off", 7,  "Surround the range between the cursor and mark with an '#if 0' and an '#endif'", 78, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 70 },
{ PROC_LINKS(write_todo, 0), "write_todo", 10,  "At the cursor, insert a '// TODO' comment, includes user name if it was specified in config.4coder.", 99, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 76 },
{ PROC_LINKS(write_hack, 0), "write_hack", 10,  "At the cursor, insert a '// HACK' comment, includes user name if it was specified in config.4coder.", 99, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 82 },
{ PROC_LINKS(write_note, 0), "write_note", 10,  "At the cursor, insert a '// NOTE' comment, includes user name if it was specified in config.4coder.", 99, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 88 },
{ PROC_LINKS(write_block, 0), "write_block", 11,  "At the cursor, insert a block comment.", 38, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 94 },
{ PROC_LINKS(write_zero_struct, 0), "write_zero_struct", 17,  "At the cursor, insert a ' = {};'.", 33, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 100 },
{ PROC_LINKS(comment_line, 0), "comment_line", 12,  "Insert '//' at the beginning of the line after leading whitespace.", 66, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 125 },
{ PROC_LINKS(uncomment_line, 0), "uncomment_line", 14,  "If present, delete '//' at the beginning of the line after leading whitespace.", 78, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 137 },
{ PROC_LINKS(comment_line_toggle, 0), "comment_line_toggle", 19,  "Turns uncommented lines into commented lines and vice versa for comments starting with '//'.", 92, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 149 },
{ PROC_LINKS(snippet_lister, 0), "snippet_lister", 14,  "Opens a snippet lister for inserting whole pre-written snippets of text.", 72, "c:\\4ed\\code\\custom\\4coder_combined_write_commands.cpp", 53, 231 },
{ PROC_LINKS(miblo_increment_basic, 0), "miblo_increment_basic", 21,  "Increment an integer under the cursor by one.", 45, "c:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 29 },
{ PROC_LINKS(miblo_decrement_basic, 0), "miblo_decrement_basic", 21,  "Decrement an integer under the cursor by one.", 45, "c:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 44 },
{ PROC_LINKS(miblo_increment_time_stamp, 0), "miblo_increment_time_stamp", 26,  "Increment a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss", 81, "c:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 231 },
{ PROC_LINKS(miblo_decrement_time_stamp, 0), "miblo_decrement_time_stamp", 26,  "Decrement a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss", 81, "c:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 237 },
{ PROC_LINKS(miblo_increment_time_stamp_minute, 0), "miblo_increment_time_stamp_minute", 33,  "Increment a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss", 81, "c:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 243 },
{ PROC_LINKS(miblo_decrement_time_stamp_minute, 0), "miblo_decrement_time_stamp_minute", 33,  "Decrement a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss", 81, "c:\\4ed\\code\\custom\\4coder_miblo_numbers.cpp", 43, 249 },
{ PROC_LINKS(profile_inspect, 0), "profile_inspect", 15,  "Inspect all currently collected profiling information in 4coder's self profiler.", 80, "c:\\4ed\\code\\custom\\4coder_profile_inspect.cpp", 45, 306 },
{ PROC_LINKS(default_startup, 0), "default_startup", 15,  "Default command for responding to a startup event", 49, "c:\\4ed\\code\\custom\\4coder_default_hooks.cpp", 43, 7 },
{ PROC_LINKS(default_try_exit, 0), "default_try_exit", 16,  "Default command for responding to a try-exit event", 50, "c:\\4ed\\code\\custom\\4coder_default_hooks.cpp", 43, 21 },
};
static i32 fcoder_metacmd_ID_default_view_input_handler = 0;
static i32 fcoder_metacmd_ID_profile_enable = 1;
static i32 fcoder_metacmd_ID_profile_disable = 2;
static i32 fcoder_metacmd_ID_profile_clear = 3;
static i32 fcoder_metacmd_ID_seek_beginning_of_textual_line = 4;
static i32 fcoder_metacmd_ID_seek_end_of_textual_line = 5;
static i32 fcoder_metacmd_ID_seek_beginning_of_line = 6;
static i32 fcoder_metacmd_ID_seek_end_of_line = 7;
static i32 fcoder_metacmd_ID_goto_beginning_of_file = 8;
static i32 fcoder_metacmd_ID_goto_end_of_file = 9;
static i32 fcoder_metacmd_ID_change_active_panel = 10;
static i32 fcoder_metacmd_ID_change_active_panel_backwards = 11;
static i32 fcoder_metacmd_ID_open_panel_vsplit = 12;
static i32 fcoder_metacmd_ID_open_panel_hsplit = 13;
static i32 fcoder_metacmd_ID_suppress_mouse = 14;
static i32 fcoder_metacmd_ID_allow_mouse = 15;
static i32 fcoder_metacmd_ID_toggle_mouse = 16;
static i32 fcoder_metacmd_ID_set_mode_to_original = 17;
static i32 fcoder_metacmd_ID_set_mode_to_notepad_like = 18;
static i32 fcoder_metacmd_ID_toggle_highlight_line_at_cursor = 19;
static i32 fcoder_metacmd_ID_toggle_highlight_enclosing_scopes = 20;
static i32 fcoder_metacmd_ID_toggle_paren_matching_helper = 21;
static i32 fcoder_metacmd_ID_toggle_fullscreen = 22;
static i32 fcoder_metacmd_ID_write_text_input = 23;
static i32 fcoder_metacmd_ID_write_space = 24;
static i32 fcoder_metacmd_ID_write_underscore = 25;
static i32 fcoder_metacmd_ID_delete_char = 26;
static i32 fcoder_metacmd_ID_backspace_char = 27;
static i32 fcoder_metacmd_ID_set_mark = 28;
static i32 fcoder_metacmd_ID_cursor_mark_swap = 29;
static i32 fcoder_metacmd_ID_delete_range = 30;
static i32 fcoder_metacmd_ID_backspace_alpha_numeric_boundary = 31;
static i32 fcoder_metacmd_ID_delete_alpha_numeric_boundary = 32;
static i32 fcoder_metacmd_ID_snipe_backward_whitespace_or_token_boundary = 33;
static i32 fcoder_metacmd_ID_snipe_forward_whitespace_or_token_boundary = 34;
static i32 fcoder_metacmd_ID_center_view = 35;
static i32 fcoder_metacmd_ID_left_adjust_view = 36;
static i32 fcoder_metacmd_ID_click_set_cursor_and_mark = 37;
static i32 fcoder_metacmd_ID_click_set_cursor = 38;
static i32 fcoder_metacmd_ID_click_set_cursor_if_lbutton = 39;
static i32 fcoder_metacmd_ID_click_set_mark = 40;
static i32 fcoder_metacmd_ID_mouse_wheel_scroll = 41;
static i32 fcoder_metacmd_ID_move_up = 42;
static i32 fcoder_metacmd_ID_move_down = 43;
static i32 fcoder_metacmd_ID_move_up_10 = 44;
static i32 fcoder_metacmd_ID_move_down_10 = 45;
static i32 fcoder_metacmd_ID_move_down_textual = 46;
static i32 fcoder_metacmd_ID_page_up = 47;
static i32 fcoder_metacmd_ID_page_down = 48;
static i32 fcoder_metacmd_ID_move_up_to_blank_line = 49;
static i32 fcoder_metacmd_ID_move_down_to_blank_line = 50;
static i32 fcoder_metacmd_ID_move_up_to_blank_line_skip_whitespace = 51;
static i32 fcoder_metacmd_ID_move_down_to_blank_line_skip_whitespace = 52;
static i32 fcoder_metacmd_ID_move_up_to_blank_line_end = 53;
static i32 fcoder_metacmd_ID_move_down_to_blank_line_end = 54;
static i32 fcoder_metacmd_ID_move_left = 55;
static i32 fcoder_metacmd_ID_move_right = 56;
static i32 fcoder_metacmd_ID_move_right_whitespace_boundary = 57;
static i32 fcoder_metacmd_ID_move_left_whitespace_boundary = 58;
static i32 fcoder_metacmd_ID_move_right_token_boundary = 59;
static i32 fcoder_metacmd_ID_move_left_token_boundary = 60;
static i32 fcoder_metacmd_ID_move_right_whitespace_or_token_boundary = 61;
static i32 fcoder_metacmd_ID_move_left_whitespace_or_token_boundary = 62;
static i32 fcoder_metacmd_ID_move_right_alpha_numeric_boundary = 63;
static i32 fcoder_metacmd_ID_move_left_alpha_numeric_boundary = 64;
static i32 fcoder_metacmd_ID_move_right_alpha_numeric_or_camel_boundary = 65;
static i32 fcoder_metacmd_ID_move_left_alpha_numeric_or_camel_boundary = 66;
static i32 fcoder_metacmd_ID_select_all = 67;
static i32 fcoder_metacmd_ID_to_uppercase = 68;
static i32 fcoder_metacmd_ID_to_lowercase = 69;
static i32 fcoder_metacmd_ID_clean_all_lines = 70;
static i32 fcoder_metacmd_ID_basic_change_active_panel = 71;
static i32 fcoder_metacmd_ID_close_panel = 72;
static i32 fcoder_metacmd_ID_show_scrollbar = 73;
static i32 fcoder_metacmd_ID_hide_scrollbar = 74;
static i32 fcoder_metacmd_ID_show_filebar = 75;
static i32 fcoder_metacmd_ID_hide_filebar = 76;
static i32 fcoder_metacmd_ID_toggle_filebar = 77;
static i32 fcoder_metacmd_ID_toggle_fps_meter = 78;
static i32 fcoder_metacmd_ID_increase_face_size = 79;
static i32 fcoder_metacmd_ID_decrease_face_size = 80;
static i32 fcoder_metacmd_ID_mouse_wheel_change_face_size = 81;
static i32 fcoder_metacmd_ID_toggle_virtual_whitespace = 82;
static i32 fcoder_metacmd_ID_toggle_show_whitespace = 83;
static i32 fcoder_metacmd_ID_toggle_line_numbers = 84;
static i32 fcoder_metacmd_ID_exit_4coder = 85;
static i32 fcoder_metacmd_ID_goto_line = 86;
static i32 fcoder_metacmd_ID_search = 87;
static i32 fcoder_metacmd_ID_reverse_search = 88;
static i32 fcoder_metacmd_ID_search_identifier = 89;
static i32 fcoder_metacmd_ID_reverse_search_identifier = 90;
static i32 fcoder_metacmd_ID_replace_in_range = 91;
static i32 fcoder_metacmd_ID_replace_in_buffer = 92;
static i32 fcoder_metacmd_ID_replace_in_all_buffers = 93;
static i32 fcoder_metacmd_ID_query_replace = 94;
static i32 fcoder_metacmd_ID_query_replace_identifier = 95;
static i32 fcoder_metacmd_ID_query_replace_selection = 96;
static i32 fcoder_metacmd_ID_save_all_dirty_buffers = 97;
static i32 fcoder_metacmd_ID_delete_file_query = 98;
static i32 fcoder_metacmd_ID_save_to_query = 99;
static i32 fcoder_metacmd_ID_rename_file_query = 100;
static i32 fcoder_metacmd_ID_make_directory_query = 101;
static i32 fcoder_metacmd_ID_move_line_up = 102;
static i32 fcoder_metacmd_ID_move_line_down = 103;
static i32 fcoder_metacmd_ID_duplicate_line = 104;
static i32 fcoder_metacmd_ID_delete_line = 105;
static i32 fcoder_metacmd_ID_open_file_in_quotes = 106;
static i32 fcoder_metacmd_ID_open_matching_file_cpp = 107;
static i32 fcoder_metacmd_ID_view_buffer_other_panel = 108;
static i32 fcoder_metacmd_ID_swap_buffers_between_panels = 109;
static i32 fcoder_metacmd_ID_kill_buffer = 110;
static i32 fcoder_metacmd_ID_save = 111;
static i32 fcoder_metacmd_ID_reopen = 112;
static i32 fcoder_metacmd_ID_undo = 113;
static i32 fcoder_metacmd_ID_redo = 114;
static i32 fcoder_metacmd_ID_undo_all_buffers = 115;
static i32 fcoder_metacmd_ID_redo_all_buffers = 116;
static i32 fcoder_metacmd_ID_open_in_other = 117;
static i32 fcoder_metacmd_ID_default_file_externally_modified = 118;
static i32 fcoder_metacmd_ID_set_eol_mode_to_crlf = 119;
static i32 fcoder_metacmd_ID_set_eol_mode_to_lf = 120;
static i32 fcoder_metacmd_ID_set_eol_mode_to_binary = 121;
static i32 fcoder_metacmd_ID_set_eol_mode_from_contents = 122;
static i32 fcoder_metacmd_ID_interactive_switch_buffer = 123;
static i32 fcoder_metacmd_ID_interactive_kill_buffer = 124;
static i32 fcoder_metacmd_ID_interactive_open_or_new = 125;
static i32 fcoder_metacmd_ID_interactive_new = 126;
static i32 fcoder_metacmd_ID_interactive_open = 127;
static i32 fcoder_metacmd_ID_command_lister = 128;
static i32 fcoder_metacmd_ID_auto_indent_whole_file = 129;
static i32 fcoder_metacmd_ID_auto_indent_line_at_cursor = 130;
static i32 fcoder_metacmd_ID_auto_indent_range = 131;
static i32 fcoder_metacmd_ID_write_text_and_auto_indent = 132;
static i32 fcoder_metacmd_ID_list_all_locations = 133;
static i32 fcoder_metacmd_ID_list_all_substring_locations = 134;
static i32 fcoder_metacmd_ID_list_all_locations_case_insensitive = 135;
static i32 fcoder_metacmd_ID_list_all_substring_locations_case_insensitive = 136;
static i32 fcoder_metacmd_ID_list_all_locations_of_identifier = 137;
static i32 fcoder_metacmd_ID_list_all_locations_of_identifier_case_insensitive = 138;
static i32 fcoder_metacmd_ID_list_all_locations_of_selection = 139;
static i32 fcoder_metacmd_ID_list_all_locations_of_selection_case_insensitive = 140;
static i32 fcoder_metacmd_ID_list_all_locations_of_type_definition = 141;
static i32 fcoder_metacmd_ID_list_all_locations_of_type_definition_of_identifier = 142;
static i32 fcoder_metacmd_ID_word_complete = 143;
static i32 fcoder_metacmd_ID_goto_jump_at_cursor = 144;
static i32 fcoder_metacmd_ID_goto_jump_at_cursor_same_panel = 145;
static i32 fcoder_metacmd_ID_goto_next_jump = 146;
static i32 fcoder_metacmd_ID_goto_prev_jump = 147;
static i32 fcoder_metacmd_ID_goto_next_jump_no_skips = 148;
static i32 fcoder_metacmd_ID_goto_prev_jump_no_skips = 149;
static i32 fcoder_metacmd_ID_goto_first_jump = 150;
static i32 fcoder_metacmd_ID_goto_first_jump_same_panel_sticky = 151;
static i32 fcoder_metacmd_ID_if_read_only_goto_position = 152;
static i32 fcoder_metacmd_ID_if_read_only_goto_position_same_panel = 153;
static i32 fcoder_metacmd_ID_view_jump_list_with_lister = 154;
static i32 fcoder_metacmd_ID_show_the_log_graph = 155;
static i32 fcoder_metacmd_ID_copy = 156;
static i32 fcoder_metacmd_ID_cut = 157;
static i32 fcoder_metacmd_ID_paste = 158;
static i32 fcoder_metacmd_ID_paste_next = 159;
static i32 fcoder_metacmd_ID_paste_and_indent = 160;
static i32 fcoder_metacmd_ID_paste_next_and_indent = 161;
static i32 fcoder_metacmd_ID_execute_previous_cli = 162;
static i32 fcoder_metacmd_ID_execute_any_cli = 163;
static i32 fcoder_metacmd_ID_build_search = 164;
static i32 fcoder_metacmd_ID_build_in_build_panel = 165;
static i32 fcoder_metacmd_ID_close_build_panel = 166;
static i32 fcoder_metacmd_ID_change_to_build_panel = 167;
static i32 fcoder_metacmd_ID_close_all_code = 168;
static i32 fcoder_metacmd_ID_open_all_code = 169;
static i32 fcoder_metacmd_ID_open_all_code_recursive = 170;
static i32 fcoder_metacmd_ID_load_project = 171;
static i32 fcoder_metacmd_ID_project_fkey_command = 172;
static i32 fcoder_metacmd_ID_project_go_to_root_directory = 173;
static i32 fcoder_metacmd_ID_setup_new_project = 174;
static i32 fcoder_metacmd_ID_setup_build_bat = 175;
static i32 fcoder_metacmd_ID_setup_build_sh = 176;
static i32 fcoder_metacmd_ID_setup_build_bat_and_sh = 177;
static i32 fcoder_metacmd_ID_project_command_lister = 178;
static i32 fcoder_metacmd_ID_list_all_functions_current_buffer = 179;
static i32 fcoder_metacmd_ID_list_all_functions_current_buffer_lister = 180;
static i32 fcoder_metacmd_ID_list_all_functions_all_buffers = 181;
static i32 fcoder_metacmd_ID_list_all_functions_all_buffers_lister = 182;
static i32 fcoder_metacmd_ID_select_surrounding_scope = 183;
static i32 fcoder_metacmd_ID_select_next_scope_absolute = 184;
static i32 fcoder_metacmd_ID_select_next_scope_after_current = 185;
static i32 fcoder_metacmd_ID_select_prev_scope_absolute = 186;
static i32 fcoder_metacmd_ID_place_in_scope = 187;
static i32 fcoder_metacmd_ID_delete_current_scope = 188;
static i32 fcoder_metacmd_ID_open_long_braces = 189;
static i32 fcoder_metacmd_ID_open_long_braces_semicolon = 190;
static i32 fcoder_metacmd_ID_open_long_braces_break = 191;
static i32 fcoder_metacmd_ID_if0_off = 192;
static i32 fcoder_metacmd_ID_write_todo = 193;
static i32 fcoder_metacmd_ID_write_hack = 194;
static i32 fcoder_metacmd_ID_write_note = 195;
static i32 fcoder_metacmd_ID_write_block = 196;
static i32 fcoder_metacmd_ID_write_zero_struct = 197;
static i32 fcoder_metacmd_ID_comment_line = 198;
static i32 fcoder_metacmd_ID_uncomment_line = 199;
static i32 fcoder_metacmd_ID_comment_line_toggle = 200;
static i32 fcoder_metacmd_ID_snippet_lister = 201;
static i32 fcoder_metacmd_ID_miblo_increment_basic = 202;
static i32 fcoder_metacmd_ID_miblo_decrement_basic = 203;
static i32 fcoder_metacmd_ID_miblo_increment_time_stamp = 204;
static i32 fcoder_metacmd_ID_miblo_decrement_time_stamp = 205;
static i32 fcoder_metacmd_ID_miblo_increment_time_stamp_minute = 206;
static i32 fcoder_metacmd_ID_miblo_decrement_time_stamp_minute = 207;
static i32 fcoder_metacmd_ID_profile_inspect = 208;
static i32 fcoder_metacmd_ID_default_startup = 209;
static i32 fcoder_metacmd_ID_default_try_exit = 210;
#endif
