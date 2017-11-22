#define command_id(c) (fcoder_metacmd_ID_##c)
#define command_metadata(c) (&fcoder_metacmd_table[command_id(c)])
#define command_metadata_by_id(id) (&fcoder_metacmd_table[id])
#define command_one_past_last_id 185
#if defined(CUSTOM_COMMAND_SIG)
#define PROC_LINKS(x,y) x
#else
#define PROC_LINKS(x,y) y
#endif
#if defined(CUSTOM_COMMAND_SIG)
CUSTOM_COMMAND_SIG(auto_tab_whole_file);
CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor);
CUSTOM_COMMAND_SIG(auto_tab_range);
CUSTOM_COMMAND_SIG(write_and_auto_tab);
CUSTOM_COMMAND_SIG(write_character);
CUSTOM_COMMAND_SIG(write_underscore);
CUSTOM_COMMAND_SIG(delete_char);
CUSTOM_COMMAND_SIG(backspace_char);
CUSTOM_COMMAND_SIG(set_mark);
CUSTOM_COMMAND_SIG(cursor_mark_swap);
CUSTOM_COMMAND_SIG(delete_range);
CUSTOM_COMMAND_SIG(center_view);
CUSTOM_COMMAND_SIG(left_adjust_view);
CUSTOM_COMMAND_SIG(click_set_cursor);
CUSTOM_COMMAND_SIG(click_set_mark);
CUSTOM_COMMAND_SIG(move_up);
CUSTOM_COMMAND_SIG(move_down);
CUSTOM_COMMAND_SIG(move_up_10);
CUSTOM_COMMAND_SIG(move_down_10);
CUSTOM_COMMAND_SIG(page_up);
CUSTOM_COMMAND_SIG(page_down);
CUSTOM_COMMAND_SIG(move_left);
CUSTOM_COMMAND_SIG(move_right);
CUSTOM_COMMAND_SIG(select_all);
CUSTOM_COMMAND_SIG(seek_whitespace_up);
CUSTOM_COMMAND_SIG(seek_whitespace_down);
CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line);
CUSTOM_COMMAND_SIG(seek_end_of_textual_line);
CUSTOM_COMMAND_SIG(seek_beginning_of_line);
CUSTOM_COMMAND_SIG(seek_end_of_line);
CUSTOM_COMMAND_SIG(seek_whitespace_up_end_line);
CUSTOM_COMMAND_SIG(seek_whitespace_down_end_line);
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
CUSTOM_COMMAND_SIG(toggle_line_wrap);
CUSTOM_COMMAND_SIG(increase_line_wrap);
CUSTOM_COMMAND_SIG(decrease_line_wrap);
CUSTOM_COMMAND_SIG(increase_face_size);
CUSTOM_COMMAND_SIG(decrease_face_size);
CUSTOM_COMMAND_SIG(toggle_virtual_whitespace);
CUSTOM_COMMAND_SIG(toggle_show_whitespace);
CUSTOM_COMMAND_SIG(eol_dosify);
CUSTOM_COMMAND_SIG(eol_nixify);
CUSTOM_COMMAND_SIG(exit_4coder);
CUSTOM_COMMAND_SIG(goto_line);
CUSTOM_COMMAND_SIG(search);
CUSTOM_COMMAND_SIG(reverse_search);
CUSTOM_COMMAND_SIG(search_identifier);
CUSTOM_COMMAND_SIG(reverse_search_identifier);
CUSTOM_COMMAND_SIG(replace_in_range);
CUSTOM_COMMAND_SIG(query_replace);
CUSTOM_COMMAND_SIG(query_replace_identifier);
CUSTOM_COMMAND_SIG(save_all_dirty_buffers);
CUSTOM_COMMAND_SIG(delete_file_query);
CUSTOM_COMMAND_SIG(rename_file_query);
CUSTOM_COMMAND_SIG(make_directory_query);
CUSTOM_COMMAND_SIG(undo);
CUSTOM_COMMAND_SIG(redo);
CUSTOM_COMMAND_SIG(interactive_new);
CUSTOM_COMMAND_SIG(interactive_open);
CUSTOM_COMMAND_SIG(interactive_open_or_new);
CUSTOM_COMMAND_SIG(interactive_switch_buffer);
CUSTOM_COMMAND_SIG(interactive_kill_buffer);
CUSTOM_COMMAND_SIG(reopen);
CUSTOM_COMMAND_SIG(save);
CUSTOM_COMMAND_SIG(kill_buffer);
CUSTOM_COMMAND_SIG(open_color_tweaker);
CUSTOM_COMMAND_SIG(open_debug);
CUSTOM_COMMAND_SIG(build_search);
CUSTOM_COMMAND_SIG(build_in_build_panel);
CUSTOM_COMMAND_SIG(close_build_panel);
CUSTOM_COMMAND_SIG(change_to_build_panel);
CUSTOM_COMMAND_SIG(copy);
CUSTOM_COMMAND_SIG(cut);
CUSTOM_COMMAND_SIG(paste);
CUSTOM_COMMAND_SIG(paste_next);
CUSTOM_COMMAND_SIG(change_active_panel);
CUSTOM_COMMAND_SIG(change_active_panel_backwards);
CUSTOM_COMMAND_SIG(open_panel_vsplit);
CUSTOM_COMMAND_SIG(open_panel_hsplit);
CUSTOM_COMMAND_SIG(suppress_mouse);
CUSTOM_COMMAND_SIG(allow_mouse);
CUSTOM_COMMAND_SIG(toggle_mouse);
CUSTOM_COMMAND_SIG(toggle_fullscreen);
CUSTOM_COMMAND_SIG(remap_interactive);
CUSTOM_COMMAND_SIG(seek_whitespace_right);
CUSTOM_COMMAND_SIG(seek_whitespace_left);
CUSTOM_COMMAND_SIG(seek_token_right);
CUSTOM_COMMAND_SIG(seek_token_left);
CUSTOM_COMMAND_SIG(seek_white_or_token_right);
CUSTOM_COMMAND_SIG(seek_white_or_token_left);
CUSTOM_COMMAND_SIG(seek_alphanumeric_right);
CUSTOM_COMMAND_SIG(seek_alphanumeric_left);
CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_right);
CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_left);
CUSTOM_COMMAND_SIG(backspace_word);
CUSTOM_COMMAND_SIG(delete_word);
CUSTOM_COMMAND_SIG(snipe_token_or_word);
CUSTOM_COMMAND_SIG(snipe_token_or_word_right);
CUSTOM_COMMAND_SIG(query_replace_selection);
CUSTOM_COMMAND_SIG(move_line_up);
CUSTOM_COMMAND_SIG(move_down_textual);
CUSTOM_COMMAND_SIG(move_line_down);
CUSTOM_COMMAND_SIG(duplicate_line);
CUSTOM_COMMAND_SIG(delete_line);
CUSTOM_COMMAND_SIG(paste_and_indent);
CUSTOM_COMMAND_SIG(paste_next_and_indent);
CUSTOM_COMMAND_SIG(open_long_braces);
CUSTOM_COMMAND_SIG(open_long_braces_semicolon);
CUSTOM_COMMAND_SIG(open_long_braces_break);
CUSTOM_COMMAND_SIG(if0_off);
CUSTOM_COMMAND_SIG(write_todo);
CUSTOM_COMMAND_SIG(write_hack);
CUSTOM_COMMAND_SIG(write_note);
CUSTOM_COMMAND_SIG(write_block);
CUSTOM_COMMAND_SIG(write_zero_struct);
CUSTOM_COMMAND_SIG(open_file_in_quotes);
CUSTOM_COMMAND_SIG(open_in_other);
CUSTOM_COMMAND_SIG(open_matching_file_cpp);
CUSTOM_COMMAND_SIG(execute_arbitrary_command);
CUSTOM_COMMAND_SIG(list_all_functions_current_buffer);
CUSTOM_COMMAND_SIG(goto_jump_at_cursor_direct);
CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel_direct);
CUSTOM_COMMAND_SIG(goto_next_jump_direct);
CUSTOM_COMMAND_SIG(goto_prev_jump_direct);
CUSTOM_COMMAND_SIG(goto_next_jump_no_skips_direct);
CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips_direct);
CUSTOM_COMMAND_SIG(goto_first_jump_direct);
CUSTOM_COMMAND_SIG(newline_or_goto_position_direct);
CUSTOM_COMMAND_SIG(newline_or_goto_position_same_panel_direct);
CUSTOM_COMMAND_SIG(goto_jump_at_cursor_sticky);
CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel_sticky);
CUSTOM_COMMAND_SIG(goto_next_jump_sticky);
CUSTOM_COMMAND_SIG(goto_prev_jump_sticky);
CUSTOM_COMMAND_SIG(goto_next_jump_no_skips_sticky);
CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips_sticky);
CUSTOM_COMMAND_SIG(goto_first_jump_sticky);
CUSTOM_COMMAND_SIG(newline_or_goto_position_sticky);
CUSTOM_COMMAND_SIG(newline_or_goto_position_same_panel_sticky);
CUSTOM_COMMAND_SIG(open_all_code);
CUSTOM_COMMAND_SIG(open_all_code_recursive);
CUSTOM_COMMAND_SIG(close_all_code);
CUSTOM_COMMAND_SIG(load_project);
CUSTOM_COMMAND_SIG(reload_current_project);
CUSTOM_COMMAND_SIG(project_fkey_command);
CUSTOM_COMMAND_SIG(project_go_to_root_directory);
CUSTOM_COMMAND_SIG(setup_new_project);
CUSTOM_COMMAND_SIG(set_bindings_choose);
CUSTOM_COMMAND_SIG(set_bindings_default);
CUSTOM_COMMAND_SIG(set_bindings_mac_default);
CUSTOM_COMMAND_SIG(highlight_surrounding_scope);
CUSTOM_COMMAND_SIG(highlight_next_scope_absolute);
CUSTOM_COMMAND_SIG(highlight_prev_scope_absolute);
CUSTOM_COMMAND_SIG(place_in_scope);
CUSTOM_COMMAND_SIG(delete_current_scope);
CUSTOM_COMMAND_SIG(scope_absorb_down);
CUSTOM_COMMAND_SIG(list_all_locations);
CUSTOM_COMMAND_SIG(list_all_substring_locations);
CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_locations_of_identifier);
CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive);
CUSTOM_COMMAND_SIG(list_all_locations_of_selection);
CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive);
CUSTOM_COMMAND_SIG(word_complete);
CUSTOM_COMMAND_SIG(execute_previous_cli);
CUSTOM_COMMAND_SIG(execute_any_cli);
CUSTOM_COMMAND_SIG(kill_rect);
CUSTOM_COMMAND_SIG(multi_line_edit);
CUSTOM_COMMAND_SIG(rename_parameter);
CUSTOM_COMMAND_SIG(write_explicit_enum_values);
CUSTOM_COMMAND_SIG(miblo_increment_basic);
CUSTOM_COMMAND_SIG(miblo_decrement_basic);
CUSTOM_COMMAND_SIG(miblo_increment_time_stamp);
CUSTOM_COMMAND_SIG(miblo_decrement_time_stamp);
CUSTOM_COMMAND_SIG(miblo_increment_time_stamp_minute);
CUSTOM_COMMAND_SIG(miblo_decrement_time_stamp_minute);
#endif
struct Command_Metadata{
PROC_LINKS(Custom_Command_Function, void) *proc;
char *name;
int32_t name_len;
char *description;
int32_t description_len;
char *source_name;
int32_t source_name_len;
int32_t line_number;
};
static Command_Metadata fcoder_metacmd_table[185] = {
{ PROC_LINKS(auto_tab_whole_file, 0), "auto_tab_whole_file", 19,  "Audo-indents the entire current buffer.", 39, "C:\\work\\4ed\\code\\4coder_auto_indent.cpp", 43, 608 },
{ PROC_LINKS(auto_tab_line_at_cursor, 0), "auto_tab_line_at_cursor", 23,  "Auto-indents the line on which the cursor sits.", 47, "C:\\work\\4ed\\code\\4coder_auto_indent.cpp", 43, 618 },
{ PROC_LINKS(auto_tab_range, 0), "auto_tab_range", 14,  "Auto-indents the range between the cursor and the mark.", 55, "C:\\work\\4ed\\code\\4coder_auto_indent.cpp", 43, 629 },
{ PROC_LINKS(write_and_auto_tab, 0), "write_and_auto_tab", 18,  "Inserts a character and auto-indents the line on which the cursor sits.", 71, "C:\\work\\4ed\\code\\4coder_auto_indent.cpp", 43, 641 },
{ PROC_LINKS(write_character, 0), "write_character", 15,  "Inserts whatever character was used to trigger this command.", 60, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 47 },
{ PROC_LINKS(write_underscore, 0), "write_underscore", 16,  "Inserts an underscore.", 22, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 56 },
{ PROC_LINKS(delete_char, 0), "delete_char", 11,  "Deletes the character to the right of the cursor.", 49, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 63 },
{ PROC_LINKS(backspace_char, 0), "backspace_char", 14,  "Deletes the character to the left of the cursor.", 48, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 81 },
{ PROC_LINKS(set_mark, 0), "set_mark", 8,  "Sets the mark to the current position of the cursor.", 52, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 100 },
{ PROC_LINKS(cursor_mark_swap, 0), "cursor_mark_swap", 16,  "Swaps the position of the cursor and the mark.", 46, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 109 },
{ PROC_LINKS(delete_range, 0), "delete_range", 12,  "Deletes the text in the range between the cursor and the mark.", 62, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 121 },
{ PROC_LINKS(center_view, 0), "center_view", 11,  "Centers the view vertically on the line on which the cursor sits.", 65, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 136 },
{ PROC_LINKS(left_adjust_view, 0), "left_adjust_view", 16,  "Sets the left size of the view near the x position of the cursor.", 65, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 151 },
{ PROC_LINKS(click_set_cursor, 0), "click_set_cursor", 16,  "Sets the cursor position to the mouse position.", 47, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 190 },
{ PROC_LINKS(click_set_mark, 0), "click_set_mark", 14,  "Sets the mark position to the mouse position.", 45, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 203 },
{ PROC_LINKS(move_up, 0), "move_up", 7,  "Moves the cursor up one line.", 29, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 227 },
{ PROC_LINKS(move_down, 0), "move_down", 9,  "Moves the cursor down one line.", 31, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 233 },
{ PROC_LINKS(move_up_10, 0), "move_up_10", 10,  "Moves the cursor up ten lines.", 30, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 239 },
{ PROC_LINKS(move_down_10, 0), "move_down_10", 12,  "Moves the cursor down ten lines.", 32, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 245 },
{ PROC_LINKS(page_up, 0), "page_up", 7,  "Scrolls the view up one view height and moves the cursor up one view height.", 76, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 267 },
{ PROC_LINKS(page_down, 0), "page_down", 9,  "Scrolls the view down one view height and moves the cursor down one view height.", 80, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 276 },
{ PROC_LINKS(move_left, 0), "move_left", 9,  "Moves the cursor one character to the left.", 43, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 286 },
{ PROC_LINKS(move_right, 0), "move_right", 10,  "Moves the cursor one character to the right.", 44, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 295 },
{ PROC_LINKS(select_all, 0), "select_all", 10,  "Puts the cursor at the top of the file, and the mark at the bottom of the file.", 79, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 304 },
{ PROC_LINKS(seek_whitespace_up, 0), "seek_whitespace_up", 18,  "Seeks the cursor up to the next blank line.", 43, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 317 },
{ PROC_LINKS(seek_whitespace_down, 0), "seek_whitespace_down", 20,  "Seeks the cursor down to the next blank line.", 45, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 328 },
{ PROC_LINKS(seek_beginning_of_textual_line, 0), "seek_beginning_of_textual_line", 30,  "Seeks the cursor to the beginning of the line across all text.", 62, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 339 },
{ PROC_LINKS(seek_end_of_textual_line, 0), "seek_end_of_textual_line", 24,  "Seeks the cursor to the end of the line across all text.", 56, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 350 },
{ PROC_LINKS(seek_beginning_of_line, 0), "seek_beginning_of_line", 22,  "Seeks the cursor to the beginning of the visual line.", 53, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 361 },
{ PROC_LINKS(seek_end_of_line, 0), "seek_end_of_line", 16,  "Seeks the cursor to the end of the visual line.", 47, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 374 },
{ PROC_LINKS(seek_whitespace_up_end_line, 0), "seek_whitespace_up_end_line", 27,  "Seeks the cursor up to the next blank line and places it at the end of the line.", 80, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 387 },
{ PROC_LINKS(seek_whitespace_down_end_line, 0), "seek_whitespace_down_end_line", 29,  "Seeks the cursor down to the next blank line and places it at the end of the line.", 82, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 394 },
{ PROC_LINKS(to_uppercase, 0), "to_uppercase", 12,  "Converts all ascii text in the range between the cursor and the mark to uppercase.", 82, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 406 },
{ PROC_LINKS(to_lowercase, 0), "to_lowercase", 12,  "Converts all ascii text in the range between the cursor and the mark to lowercase.", 82, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 426 },
{ PROC_LINKS(clean_all_lines, 0), "clean_all_lines", 15,  "Removes trailing whitespace from all lines in the current buffer.", 65, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 446 },
{ PROC_LINKS(basic_change_active_panel, 0), "basic_change_active_panel", 25,  "Change the currently active panel, moving to the panel with the next highest view_id.  Will not skipe the build panel if it is open.", 132, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 514 },
{ PROC_LINKS(close_panel, 0), "close_panel", 11,  "Closes the currently active panel if it is not the only panel open.", 67, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 522 },
{ PROC_LINKS(show_scrollbar, 0), "show_scrollbar", 14,  "Sets the current view to show it's scrollbar.", 45, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 534 },
{ PROC_LINKS(hide_scrollbar, 0), "hide_scrollbar", 14,  "Sets the current view to hide it's scrollbar.", 45, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 541 },
{ PROC_LINKS(show_filebar, 0), "show_filebar", 12,  "Sets the current view to show it's filebar.", 43, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 548 },
{ PROC_LINKS(hide_filebar, 0), "hide_filebar", 12,  "Sets the current view to hide it's filebar.", 43, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 555 },
{ PROC_LINKS(toggle_filebar, 0), "toggle_filebar", 14,  "Toggles the visibility status of the current view's filebar.", 60, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 562 },
{ PROC_LINKS(toggle_line_wrap, 0), "toggle_line_wrap", 16,  "Toggles the current buffer's line wrapping status.", 50, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 571 },
{ PROC_LINKS(increase_line_wrap, 0), "increase_line_wrap", 18,  "Increases the current buffer's width for line wrapping.", 55, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 581 },
{ PROC_LINKS(decrease_line_wrap, 0), "decrease_line_wrap", 18,  "Decrases the current buffer's width for line wrapping.", 54, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 592 },
{ PROC_LINKS(increase_face_size, 0), "increase_face_size", 18,  "Increase the size of the face used by the current buffer.", 57, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 603 },
{ PROC_LINKS(decrease_face_size, 0), "decrease_face_size", 18,  "Decrease the size of the face used by the current buffer.", 57, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 615 },
{ PROC_LINKS(toggle_virtual_whitespace, 0), "toggle_virtual_whitespace", 25,  "Toggles the current buffer's virtual whitespace status.", 55, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 627 },
{ PROC_LINKS(toggle_show_whitespace, 0), "toggle_show_whitespace", 22,  "Toggles the current buffer's whitespace visibility status.", 58, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 638 },
{ PROC_LINKS(eol_dosify, 0), "eol_dosify", 10,  "Puts the buffer in DOS line ending mode.", 40, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 645 },
{ PROC_LINKS(eol_nixify, 0), "eol_nixify", 10,  "Puts the buffer in NIX line ending mode.", 40, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 653 },
{ PROC_LINKS(exit_4coder, 0), "exit_4coder", 11,  "Attempts to close 4coder.", 25, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 661 },
{ PROC_LINKS(goto_line, 0), "goto_line", 9,  "Queries the user for a number, and jumps the cursor to the corresponding line.", 78, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 671 },
{ PROC_LINKS(search, 0), "search", 6,  "Begins an incremental search down through the current buffer for a user specified string.", 89, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 844 },
{ PROC_LINKS(reverse_search, 0), "reverse_search", 14,  "Begins an incremental search up through the current buffer for a user specified string.", 87, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 851 },
{ PROC_LINKS(search_identifier, 0), "search_identifier", 17,  "Begins an incremental search down through the current buffer for the word or token under the cursor.", 100, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 858 },
{ PROC_LINKS(reverse_search_identifier, 0), "reverse_search_identifier", 25,  "Begins an incremental search up through the current buffer for the word or token under the cursor.", 98, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 869 },
{ PROC_LINKS(replace_in_range, 0), "replace_in_range", 16,  "Queries the user for two strings, and replaces all occurences of the first string in the range between the cursor and the mark with the second string.", 150, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 880 },
{ PROC_LINKS(query_replace, 0), "query_replace", 13,  "Queries the user for two strings, and incrementally replaces every occurence of the first string with the second string.", 120, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 982 },
{ PROC_LINKS(query_replace_identifier, 0), "query_replace_identifier", 24,  "Queries the user for a string, and incrementally replace every occurence of the word or token found at the cursor with the specified string.", 140, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1003 },
{ PROC_LINKS(save_all_dirty_buffers, 0), "save_all_dirty_buffers", 22,  "Saves all buffers marked dirty (showing the '*' indicator).", 59, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1026 },
{ PROC_LINKS(delete_file_query, 0), "delete_file_query", 17,  "Deletes the file of the current buffer if 4coder has the appropriate access rights. Will ask the user for confirmation first.", 125, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1061 },
{ PROC_LINKS(rename_file_query, 0), "rename_file_query", 17,  "Queries the user for a new name and renames the file of the current buffer, altering the buffer's name too.", 107, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1087 },
{ PROC_LINKS(make_directory_query, 0), "make_directory_query", 20,  "Queries the user for a name and creates a new directory with the given name.", 76, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1129 },
{ PROC_LINKS(undo, 0), "undo", 4,  "Advances backwards through the undo history.", 44, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1164 },
{ PROC_LINKS(redo, 0), "redo", 4,  "Advances forewards through the undo history.", 44, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1170 },
{ PROC_LINKS(interactive_new, 0), "interactive_new", 15,  "Interactively creates a new file.", 33, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1176 },
{ PROC_LINKS(interactive_open, 0), "interactive_open", 16,  "Interactively opens a file.", 27, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1182 },
{ PROC_LINKS(interactive_open_or_new, 0), "interactive_open_or_new", 23,  "Interactively opens or creates a new file.", 42, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1188 },
{ PROC_LINKS(interactive_switch_buffer, 0), "interactive_switch_buffer", 25,  "Interactively switch to an open buffer.", 39, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1194 },
{ PROC_LINKS(interactive_kill_buffer, 0), "interactive_kill_buffer", 23,  "Interactively kill an open buffer.", 34, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1200 },
{ PROC_LINKS(reopen, 0), "reopen", 6,  "Reopen the current buffer from the hard drive.", 46, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1206 },
{ PROC_LINKS(save, 0), "save", 4,  "Saves the current buffer.", 25, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1212 },
{ PROC_LINKS(kill_buffer, 0), "kill_buffer", 11,  "Kills the current buffer.", 25, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1218 },
{ PROC_LINKS(open_color_tweaker, 0), "open_color_tweaker", 18,  "Opens the 4coder colors and fonts selector menu.", 48, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1224 },
{ PROC_LINKS(open_debug, 0), "open_debug", 10,  "Opens a debug view for internal use.", 36, "C:\\work\\4ed\\code\\4coder_base_commands.cpp", 45, 1230 },
{ PROC_LINKS(build_search, 0), "build_search", 12,  "Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.", 153, "C:\\work\\4ed\\code\\4coder_build_commands.cpp", 46, 169 },
{ PROC_LINKS(build_in_build_panel, 0), "build_in_build_panel", 20,  "Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.  Puts the *compilation* buffer in a panel at the footer of the current view.", 230, "C:\\work\\4ed\\code\\4coder_build_commands.cpp", 46, 203 },
{ PROC_LINKS(close_build_panel, 0), "close_build_panel", 17,  "If the special build panel is open, closes it.", 46, "C:\\work\\4ed\\code\\4coder_build_commands.cpp", 46, 219 },
{ PROC_LINKS(change_to_build_panel, 0), "change_to_build_panel", 21,  "If the special build panel is open, makes the build panel the active panel.", 75, "C:\\work\\4ed\\code\\4coder_build_commands.cpp", 46, 225 },
{ PROC_LINKS(copy, 0), "copy", 4,  "Copy the text in the range from the cursor to the mark onto the clipboard.", 74, "C:\\work\\4ed\\code\\4coder_clipboard.cpp", 41, 52 },
{ PROC_LINKS(cut, 0), "cut", 3,  "Cut the text in the range from the cursor to the mark onto the clipboard.", 73, "C:\\work\\4ed\\code\\4coder_clipboard.cpp", 41, 61 },
{ PROC_LINKS(paste, 0), "paste", 5,  "At the cursor, insert the text at the top of the clipboard.", 59, "C:\\work\\4ed\\code\\4coder_clipboard.cpp", 41, 70 },
{ PROC_LINKS(paste_next, 0), "paste_next", 10,  "If the previous command was paste or paste_next, replaces the paste range with the next text down on the clipboard, otherwise operates as the paste command.", 156, "C:\\work\\4ed\\code\\4coder_clipboard.cpp", 41, 108 },
{ PROC_LINKS(change_active_panel, 0), "change_active_panel", 19,  "Change the currently active panel, moving to the panel with the next highest view_id.", 85, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 125 },
{ PROC_LINKS(change_active_panel_backwards, 0), "change_active_panel_backwards", 29,  "Change the currently active panel, moving to the panel with the next lowest view_id.", 84, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 143 },
{ PROC_LINKS(open_panel_vsplit, 0), "open_panel_vsplit", 17,  "Create a new panel by vertically splitting the active panel.", 60, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 161 },
{ PROC_LINKS(open_panel_hsplit, 0), "open_panel_hsplit", 17,  "Create a new panel by horizontally splitting the active panel.", 62, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 169 },
{ PROC_LINKS(suppress_mouse, 0), "suppress_mouse", 14,  "Hides the mouse and causes all mosue input (clicks, position, wheel) to be ignored.", 83, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 224 },
{ PROC_LINKS(allow_mouse, 0), "allow_mouse", 11,  "Shows the mouse and causes all mouse input to be processed normally.", 68, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 230 },
{ PROC_LINKS(toggle_mouse, 0), "toggle_mouse", 12,  "Toggles the mouse suppression mode, see suppress_mouse and allow_mouse.", 71, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 236 },
{ PROC_LINKS(toggle_fullscreen, 0), "toggle_fullscreen", 17,  "Toggle fullscreen mode on or off.  The change(s) do not take effect until the next frame.", 89, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 242 },
{ PROC_LINKS(remap_interactive, 0), "remap_interactive", 17,  "Switch to a named key binding map.", 34, "C:\\work\\4ed\\code\\4coder_default_framework.h", 47, 741 },
{ PROC_LINKS(seek_whitespace_right, 0), "seek_whitespace_right", 21,  "Seek right for the next boundary between whitespace and non-whitespace.", 71, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 100 },
{ PROC_LINKS(seek_whitespace_left, 0), "seek_whitespace_left", 20,  "Seek left for the next boundary between whitespace and non-whitespace.", 70, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 104 },
{ PROC_LINKS(seek_token_right, 0), "seek_token_right", 16,  "Seek right for the next end of a token.", 39, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 108 },
{ PROC_LINKS(seek_token_left, 0), "seek_token_left", 15,  "Seek left for the next beginning of a token.", 44, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 112 },
{ PROC_LINKS(seek_white_or_token_right, 0), "seek_white_or_token_right", 25,  "Seek right for the next end of a token or boundary between whitespace and non-whitespace.", 89, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 116 },
{ PROC_LINKS(seek_white_or_token_left, 0), "seek_white_or_token_left", 24,  "Seek left for the next end of a token or boundary between whitespace and non-whitespace.", 88, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 120 },
{ PROC_LINKS(seek_alphanumeric_right, 0), "seek_alphanumeric_right", 23,  "Seek right for boundary between alphanumeric characters and non-alphanumeric characters.", 88, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 124 },
{ PROC_LINKS(seek_alphanumeric_left, 0), "seek_alphanumeric_left", 22,  "Seek left for boundary between alphanumeric characters and non-alphanumeric characters.", 87, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 128 },
{ PROC_LINKS(seek_alphanumeric_or_camel_right, 0), "seek_alphanumeric_or_camel_right", 32,  "Seek right for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.", 107, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 132 },
{ PROC_LINKS(seek_alphanumeric_or_camel_left, 0), "seek_alphanumeric_or_camel_left", 31,  "Seek left for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.", 106, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 136 },
{ PROC_LINKS(backspace_word, 0), "backspace_word", 14,  "Delete characters between the cursor position and the first alphanumeric boundary to the left.", 94, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 147 },
{ PROC_LINKS(delete_word, 0), "delete_word", 11,  "Delete characters between the cursor position and the first alphanumeric boundary to the right.", 95, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 167 },
{ PROC_LINKS(snipe_token_or_word, 0), "snipe_token_or_word", 19,  "Delete a single, whole token on or to the left of the cursor.", 61, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 187 },
{ PROC_LINKS(snipe_token_or_word_right, 0), "snipe_token_or_word_right", 25,  "Delete a single, whole token on or to the right of the cursor.", 62, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 202 },
{ PROC_LINKS(query_replace_selection, 0), "query_replace_selection", 23,  "Queries the user for a string, and incrementally replace every occurence of the string found in the selected range with the specified string.", 141, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 222 },
{ PROC_LINKS(move_line_up, 0), "move_line_up", 12,  "Swaps the line under the cursor with the line above it, and moves the cursor up with it.", 88, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 253 },
{ PROC_LINKS(move_down_textual, 0), "move_down_textual", 17,  "Moves down to the next line of actual text, regardless of line wrapping.", 72, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 300 },
{ PROC_LINKS(move_line_down, 0), "move_line_down", 14,  "Swaps the line under the cursor with the line below it, and moves the cursor down with it.", 90, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 312 },
{ PROC_LINKS(duplicate_line, 0), "duplicate_line", 14,  "Create a copy of the line on which the cursor sits.", 51, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 329 },
{ PROC_LINKS(delete_line, 0), "delete_line", 11,  "Delete the line the on which the cursor sits.", 45, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 351 },
{ PROC_LINKS(paste_and_indent, 0), "paste_and_indent", 16,  "Paste from the top of clipboard and run auto-indent on the newly pasted text.", 77, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 373 },
{ PROC_LINKS(paste_next_and_indent, 0), "paste_next_and_indent", 21,  "Paste the next item on the clipboard and run auto-indent on the newly pasted text.", 82, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 380 },
{ PROC_LINKS(open_long_braces, 0), "open_long_braces", 16,  "At the cursor, insert a '{' and '}' separated by a blank line.", 62, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 420 },
{ PROC_LINKS(open_long_braces_semicolon, 0), "open_long_braces_semicolon", 26,  "At the cursor, insert a '{' and '};' separated by a blank line.", 63, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 428 },
{ PROC_LINKS(open_long_braces_break, 0), "open_long_braces_break", 22,  "At the cursor, insert a '{' and '}break;' separated by a blank line.", 68, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 436 },
{ PROC_LINKS(if0_off, 0), "if0_off", 7,  "Surround the range between the cursor and mark with an '#if 0' and an '#endif'", 78, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 444 },
{ PROC_LINKS(write_todo, 0), "write_todo", 10,  "At the cursor, insert a '// TODO' comment, includes user name if it was specified in config.4coder.", 99, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 516 },
{ PROC_LINKS(write_hack, 0), "write_hack", 10,  "At the cursor, insert a '// HACK' comment, includes user name if it was specified in config.4coder.", 99, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 522 },
{ PROC_LINKS(write_note, 0), "write_note", 10,  "At the cursor, insert a '// NOTE' comment, includes user name if it was specified in config.4coder.", 99, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 528 },
{ PROC_LINKS(write_block, 0), "write_block", 11,  "At the cursor, insert a block comment.", 38, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 534 },
{ PROC_LINKS(write_zero_struct, 0), "write_zero_struct", 17,  "At the cursor, insert a ' = {0};'.", 34, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 540 },
{ PROC_LINKS(open_file_in_quotes, 0), "open_file_in_quotes", 19,  "Reads a filename from surrounding '\"' characters and attempts to open the corresponding file.", 94, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 584 },
{ PROC_LINKS(open_in_other, 0), "open_in_other", 13,  "Reads a filename from surrounding '\"' characters and attempts to open the corresponding file, displaying it in the other view.", 127, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 597 },
{ PROC_LINKS(open_matching_file_cpp, 0), "open_matching_file_cpp", 22,  "If the current file is a *.cpp or *.h, attempts to open the corresponding *.h or *.cpp file in the other view.", 110, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 657 },
{ PROC_LINKS(execute_arbitrary_command, 0), "execute_arbitrary_command", 25,  "Execute a 'long form' command.", 30, "C:\\work\\4ed\\code\\4coder_default_include.cpp", 47, 676 },
{ PROC_LINKS(list_all_functions_current_buffer, 0), "list_all_functions_current_buffer", 33,  "Creates a jump list of lines of the current buffer that appear to define or declare functions.", 94, "C:\\work\\4ed\\code\\4coder_function_list.cpp", 45, 348 },
{ PROC_LINKS(goto_jump_at_cursor_direct, 0), "goto_jump_at_cursor_direct", 26,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.", 187, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 24 },
{ PROC_LINKS(goto_jump_at_cursor_same_panel_direct, 0), "goto_jump_at_cursor_same_panel_direct", 37,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list..", 168, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 45 },
{ PROC_LINKS(goto_next_jump_direct, 0), "goto_next_jump_direct", 21,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.", 123, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 64 },
{ PROC_LINKS(goto_prev_jump_direct, 0), "goto_prev_jump_direct", 21,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations.", 127, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 73 },
{ PROC_LINKS(goto_next_jump_no_skips_direct, 0), "goto_next_jump_no_skips_direct", 30,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.", 132, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 82 },
{ PROC_LINKS(goto_prev_jump_no_skips_direct, 0), "goto_prev_jump_no_skips_direct", 30,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.", 136, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 91 },
{ PROC_LINKS(goto_first_jump_direct, 0), "goto_first_jump_direct", 22,  "If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.", 95, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 100 },
{ PROC_LINKS(newline_or_goto_position_direct, 0), "newline_or_goto_position_direct", 31,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.", 106, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 117 },
{ PROC_LINKS(newline_or_goto_position_same_panel_direct, 0), "newline_or_goto_position_same_panel_direct", 42,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.", 117, "C:\\work\\4ed\\code\\4coder_jump_direct.cpp", 43, 132 },
{ PROC_LINKS(goto_jump_at_cursor_sticky, 0), "goto_jump_at_cursor_sticky", 26,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.", 187, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 360 },
{ PROC_LINKS(goto_jump_at_cursor_same_panel_sticky, 0), "goto_jump_at_cursor_same_panel_sticky", 37,  "If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list.", 167, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 388 },
{ PROC_LINKS(goto_next_jump_sticky, 0), "goto_next_jump_sticky", 21,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.", 123, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 483 },
{ PROC_LINKS(goto_prev_jump_sticky, 0), "goto_prev_jump_sticky", 21,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations.", 127, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 499 },
{ PROC_LINKS(goto_next_jump_no_skips_sticky, 0), "goto_next_jump_no_skips_sticky", 30,  "If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.", 132, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 513 },
{ PROC_LINKS(goto_prev_jump_no_skips_sticky, 0), "goto_prev_jump_no_skips_sticky", 30,  "If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.", 136, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 529 },
{ PROC_LINKS(goto_first_jump_sticky, 0), "goto_first_jump_sticky", 22,  "If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.", 95, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 544 },
{ PROC_LINKS(newline_or_goto_position_sticky, 0), "newline_or_goto_position_sticky", 31,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.", 106, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 566 },
{ PROC_LINKS(newline_or_goto_position_same_panel_sticky, 0), "newline_or_goto_position_same_panel_sticky", 42,  "If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.", 117, "C:\\work\\4ed\\code\\4coder_jump_sticky.cpp", 43, 581 },
{ PROC_LINKS(open_all_code, 0), "open_all_code", 13,  "Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.", 164, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 165 },
{ PROC_LINKS(open_all_code_recursive, 0), "open_all_code_recursive", 23,  "Works as open_all_code but also runs in all subdirectories.", 59, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 180 },
{ PROC_LINKS(close_all_code, 0), "close_all_code", 14,  "Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.", 107, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 188 },
{ PROC_LINKS(load_project, 0), "load_project", 12,  "Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.", 167, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 400 },
{ PROC_LINKS(reload_current_project, 0), "reload_current_project", 22,  "If a project file has already been loaded, reloads the same file.  Useful for when the project configuration is changed.", 120, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 471 },
{ PROC_LINKS(project_fkey_command, 0), "project_fkey_command", 20,  "Run an 'fkey command' configured in a project.4coder file.  Determines the index of the 'fkey command' by which function key or numeric key was pressed to trigger the command.", 175, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 567 },
{ PROC_LINKS(project_go_to_root_directory, 0), "project_go_to_root_directory", 28,  "Changes 4coder's hot directory to the root directory of the currently loaded project. With no loaded project nothing hapepns.", 125, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 593 },
{ PROC_LINKS(setup_new_project, 0), "setup_new_project", 17,  "Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.", 120, "C:\\work\\4ed\\code\\4coder_project_commands.cpp", 48, 650 },
{ PROC_LINKS(set_bindings_choose, 0), "set_bindings_choose", 19,  "Remap keybindings using the 'choose' mapping rule.", 50, "C:\\work\\4ed\\code\\4coder_remapping_commands.cpp", 50, 49 },
{ PROC_LINKS(set_bindings_default, 0), "set_bindings_default", 20,  "Remap keybindings using the 'default' mapping rule.", 51, "C:\\work\\4ed\\code\\4coder_remapping_commands.cpp", 50, 63 },
{ PROC_LINKS(set_bindings_mac_default, 0), "set_bindings_mac_default", 24,  "Remap keybindings using the 'mac-default' mapping rule.", 55, "C:\\work\\4ed\\code\\4coder_remapping_commands.cpp", 50, 77 },
{ PROC_LINKS(highlight_surrounding_scope, 0), "highlight_surrounding_scope", 27,  "Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.", 107, "C:\\work\\4ed\\code\\4coder_scope_commands.cpp", 46, 346 },
{ PROC_LINKS(highlight_next_scope_absolute, 0), "highlight_next_scope_absolute", 29,  "Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.", 102, "C:\\work\\4ed\\code\\4coder_scope_commands.cpp", 46, 367 },
{ PROC_LINKS(highlight_prev_scope_absolute, 0), "highlight_prev_scope_absolute", 29,  "Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.", 103, "C:\\work\\4ed\\code\\4coder_scope_commands.cpp", 46, 385 },
{ PROC_LINKS(place_in_scope, 0), "place_in_scope", 14,  "Wraps the code contained in the range between cursor and mark with a new curly brace scope.", 91, "C:\\work\\4ed\\code\\4coder_scope_commands.cpp", 46, 403 },
{ PROC_LINKS(delete_current_scope, 0), "delete_current_scope", 20,  "Deletes the braces surrounding the currently selected scope.  Leaves the contents within the scope.", 99, "C:\\work\\4ed\\code\\4coder_scope_commands.cpp", 46, 479 },
{ PROC_LINKS(scope_absorb_down, 0), "scope_absorb_down", 17,  "If a scope is currently selected, and a statement or block statement is present below the current scope, the statement is moved into the scope.", 143, "C:\\work\\4ed\\code\\4coder_scope_commands.cpp", 46, 738 },
{ PROC_LINKS(list_all_locations, 0), "list_all_locations", 18,  "Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.", 99, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 654 },
{ PROC_LINKS(list_all_substring_locations, 0), "list_all_substring_locations", 28,  "Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.", 103, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 663 },
{ PROC_LINKS(list_all_locations_case_insensitive, 0), "list_all_locations_case_insensitive", 35,  "Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.", 101, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 672 },
{ PROC_LINKS(list_all_substring_locations_case_insensitive, 0), "list_all_substring_locations_case_insensitive", 45,  "Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.", 105, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 681 },
{ PROC_LINKS(list_all_locations_of_identifier, 0), "list_all_locations_of_identifier", 32,  "Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.", 102, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 724 },
{ PROC_LINKS(list_all_locations_of_identifier_case_insensitive, 0), "list_all_locations_of_identifier_case_insensitive", 49,  "Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.", 104, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 730 },
{ PROC_LINKS(list_all_locations_of_selection, 0), "list_all_locations_of_selection", 31,  "Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.", 102, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 772 },
{ PROC_LINKS(list_all_locations_of_selection_case_insensitive, 0), "list_all_locations_of_selection_case_insensitive", 48,  "Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.", 104, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 778 },
{ PROC_LINKS(word_complete, 0), "word_complete", 13,  "Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.", 130, "C:\\work\\4ed\\code\\4coder_search.cpp", 38, 800 },
{ PROC_LINKS(execute_previous_cli, 0), "execute_previous_cli", 20,  "If the command execute_any_cli has already been used, this will execute a CLI reusing the most recent buffer name and command.", 126, "C:\\work\\4ed\\code\\4coder_system_command.cpp", 46, 14 },
{ PROC_LINKS(execute_any_cli, 0), "execute_any_cli", 15,  "Queries for an output buffer name and system command, runs the system command as a CLI and prints the output to the specified buffer.", 133, "C:\\work\\4ed\\code\\4coder_system_command.cpp", 46, 30 },
{ PROC_LINKS(kill_rect, 0), "kill_rect", 9,  "Delete characters in a rectangular region. Range testing is done by unwrapped-xy coordinates.", 93, "C:\\work\\4ed\\code\\power\\4coder_experiments.cpp", 50, 31 },
{ PROC_LINKS(multi_line_edit, 0), "multi_line_edit", 15,  "Begin multi-line mode.  In multi-line mode characters are inserted at every line between the mark and cursor.  All characters are inserted at the same character offset into the line.  This mode uses line_char coordinates.", 221, "C:\\work\\4ed\\code\\power\\4coder_experiments.cpp", 50, 122 },
{ PROC_LINKS(rename_parameter, 0), "rename_parameter", 16,  "If the cursor is found to be on the name of a function parameter in the signature of a function definition, all occurences within the scope of the function will be replaced with a new provided string.", 200, "C:\\work\\4ed\\code\\power\\4coder_experiments.cpp", 50, 343 },
{ PROC_LINKS(write_explicit_enum_values, 0), "write_explicit_enum_values", 26,  "If the cursor is found to be on the '{' of an enum definition, the values of the enum will be filled in sequentially starting from zero.  Existing values are overwritten.", 170, "C:\\work\\4ed\\code\\power\\4coder_experiments.cpp", 50, 496 },
{ PROC_LINKS(miblo_increment_basic, 0), "miblo_increment_basic", 21,  "Increment an integer under the cursor by one.", 45, "C:\\work\\4ed\\code\\power\\4coder_miblo_numbers.cpp", 52, 103 },
{ PROC_LINKS(miblo_decrement_basic, 0), "miblo_decrement_basic", 21,  "Decrement an integer under the cursor by one.", 45, "C:\\work\\4ed\\code\\power\\4coder_miblo_numbers.cpp", 52, 119 },
{ PROC_LINKS(miblo_increment_time_stamp, 0), "miblo_increment_time_stamp", 26,  "Increment a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss", 81, "C:\\work\\4ed\\code\\power\\4coder_miblo_numbers.cpp", 52, 386 },
{ PROC_LINKS(miblo_decrement_time_stamp, 0), "miblo_decrement_time_stamp", 26,  "Decrement a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss", 81, "C:\\work\\4ed\\code\\power\\4coder_miblo_numbers.cpp", 52, 392 },
{ PROC_LINKS(miblo_increment_time_stamp_minute, 0), "miblo_increment_time_stamp_minute", 33,  "Increment a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss", 81, "C:\\work\\4ed\\code\\power\\4coder_miblo_numbers.cpp", 52, 398 },
{ PROC_LINKS(miblo_decrement_time_stamp_minute, 0), "miblo_decrement_time_stamp_minute", 33,  "Decrement a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss", 81, "C:\\work\\4ed\\code\\power\\4coder_miblo_numbers.cpp", 52, 404 },
};
static int32_t fcoder_metacmd_ID_auto_tab_whole_file = 0;
static int32_t fcoder_metacmd_ID_auto_tab_line_at_cursor = 1;
static int32_t fcoder_metacmd_ID_auto_tab_range = 2;
static int32_t fcoder_metacmd_ID_write_and_auto_tab = 3;
static int32_t fcoder_metacmd_ID_write_character = 4;
static int32_t fcoder_metacmd_ID_write_underscore = 5;
static int32_t fcoder_metacmd_ID_delete_char = 6;
static int32_t fcoder_metacmd_ID_backspace_char = 7;
static int32_t fcoder_metacmd_ID_set_mark = 8;
static int32_t fcoder_metacmd_ID_cursor_mark_swap = 9;
static int32_t fcoder_metacmd_ID_delete_range = 10;
static int32_t fcoder_metacmd_ID_center_view = 11;
static int32_t fcoder_metacmd_ID_left_adjust_view = 12;
static int32_t fcoder_metacmd_ID_click_set_cursor = 13;
static int32_t fcoder_metacmd_ID_click_set_mark = 14;
static int32_t fcoder_metacmd_ID_move_up = 15;
static int32_t fcoder_metacmd_ID_move_down = 16;
static int32_t fcoder_metacmd_ID_move_up_10 = 17;
static int32_t fcoder_metacmd_ID_move_down_10 = 18;
static int32_t fcoder_metacmd_ID_page_up = 19;
static int32_t fcoder_metacmd_ID_page_down = 20;
static int32_t fcoder_metacmd_ID_move_left = 21;
static int32_t fcoder_metacmd_ID_move_right = 22;
static int32_t fcoder_metacmd_ID_select_all = 23;
static int32_t fcoder_metacmd_ID_seek_whitespace_up = 24;
static int32_t fcoder_metacmd_ID_seek_whitespace_down = 25;
static int32_t fcoder_metacmd_ID_seek_beginning_of_textual_line = 26;
static int32_t fcoder_metacmd_ID_seek_end_of_textual_line = 27;
static int32_t fcoder_metacmd_ID_seek_beginning_of_line = 28;
static int32_t fcoder_metacmd_ID_seek_end_of_line = 29;
static int32_t fcoder_metacmd_ID_seek_whitespace_up_end_line = 30;
static int32_t fcoder_metacmd_ID_seek_whitespace_down_end_line = 31;
static int32_t fcoder_metacmd_ID_to_uppercase = 32;
static int32_t fcoder_metacmd_ID_to_lowercase = 33;
static int32_t fcoder_metacmd_ID_clean_all_lines = 34;
static int32_t fcoder_metacmd_ID_basic_change_active_panel = 35;
static int32_t fcoder_metacmd_ID_close_panel = 36;
static int32_t fcoder_metacmd_ID_show_scrollbar = 37;
static int32_t fcoder_metacmd_ID_hide_scrollbar = 38;
static int32_t fcoder_metacmd_ID_show_filebar = 39;
static int32_t fcoder_metacmd_ID_hide_filebar = 40;
static int32_t fcoder_metacmd_ID_toggle_filebar = 41;
static int32_t fcoder_metacmd_ID_toggle_line_wrap = 42;
static int32_t fcoder_metacmd_ID_increase_line_wrap = 43;
static int32_t fcoder_metacmd_ID_decrease_line_wrap = 44;
static int32_t fcoder_metacmd_ID_increase_face_size = 45;
static int32_t fcoder_metacmd_ID_decrease_face_size = 46;
static int32_t fcoder_metacmd_ID_toggle_virtual_whitespace = 47;
static int32_t fcoder_metacmd_ID_toggle_show_whitespace = 48;
static int32_t fcoder_metacmd_ID_eol_dosify = 49;
static int32_t fcoder_metacmd_ID_eol_nixify = 50;
static int32_t fcoder_metacmd_ID_exit_4coder = 51;
static int32_t fcoder_metacmd_ID_goto_line = 52;
static int32_t fcoder_metacmd_ID_search = 53;
static int32_t fcoder_metacmd_ID_reverse_search = 54;
static int32_t fcoder_metacmd_ID_search_identifier = 55;
static int32_t fcoder_metacmd_ID_reverse_search_identifier = 56;
static int32_t fcoder_metacmd_ID_replace_in_range = 57;
static int32_t fcoder_metacmd_ID_query_replace = 58;
static int32_t fcoder_metacmd_ID_query_replace_identifier = 59;
static int32_t fcoder_metacmd_ID_save_all_dirty_buffers = 60;
static int32_t fcoder_metacmd_ID_delete_file_query = 61;
static int32_t fcoder_metacmd_ID_rename_file_query = 62;
static int32_t fcoder_metacmd_ID_make_directory_query = 63;
static int32_t fcoder_metacmd_ID_undo = 64;
static int32_t fcoder_metacmd_ID_redo = 65;
static int32_t fcoder_metacmd_ID_interactive_new = 66;
static int32_t fcoder_metacmd_ID_interactive_open = 67;
static int32_t fcoder_metacmd_ID_interactive_open_or_new = 68;
static int32_t fcoder_metacmd_ID_interactive_switch_buffer = 69;
static int32_t fcoder_metacmd_ID_interactive_kill_buffer = 70;
static int32_t fcoder_metacmd_ID_reopen = 71;
static int32_t fcoder_metacmd_ID_save = 72;
static int32_t fcoder_metacmd_ID_kill_buffer = 73;
static int32_t fcoder_metacmd_ID_open_color_tweaker = 74;
static int32_t fcoder_metacmd_ID_open_debug = 75;
static int32_t fcoder_metacmd_ID_build_search = 76;
static int32_t fcoder_metacmd_ID_build_in_build_panel = 77;
static int32_t fcoder_metacmd_ID_close_build_panel = 78;
static int32_t fcoder_metacmd_ID_change_to_build_panel = 79;
static int32_t fcoder_metacmd_ID_copy = 80;
static int32_t fcoder_metacmd_ID_cut = 81;
static int32_t fcoder_metacmd_ID_paste = 82;
static int32_t fcoder_metacmd_ID_paste_next = 83;
static int32_t fcoder_metacmd_ID_change_active_panel = 84;
static int32_t fcoder_metacmd_ID_change_active_panel_backwards = 85;
static int32_t fcoder_metacmd_ID_open_panel_vsplit = 86;
static int32_t fcoder_metacmd_ID_open_panel_hsplit = 87;
static int32_t fcoder_metacmd_ID_suppress_mouse = 88;
static int32_t fcoder_metacmd_ID_allow_mouse = 89;
static int32_t fcoder_metacmd_ID_toggle_mouse = 90;
static int32_t fcoder_metacmd_ID_toggle_fullscreen = 91;
static int32_t fcoder_metacmd_ID_remap_interactive = 92;
static int32_t fcoder_metacmd_ID_seek_whitespace_right = 93;
static int32_t fcoder_metacmd_ID_seek_whitespace_left = 94;
static int32_t fcoder_metacmd_ID_seek_token_right = 95;
static int32_t fcoder_metacmd_ID_seek_token_left = 96;
static int32_t fcoder_metacmd_ID_seek_white_or_token_right = 97;
static int32_t fcoder_metacmd_ID_seek_white_or_token_left = 98;
static int32_t fcoder_metacmd_ID_seek_alphanumeric_right = 99;
static int32_t fcoder_metacmd_ID_seek_alphanumeric_left = 100;
static int32_t fcoder_metacmd_ID_seek_alphanumeric_or_camel_right = 101;
static int32_t fcoder_metacmd_ID_seek_alphanumeric_or_camel_left = 102;
static int32_t fcoder_metacmd_ID_backspace_word = 103;
static int32_t fcoder_metacmd_ID_delete_word = 104;
static int32_t fcoder_metacmd_ID_snipe_token_or_word = 105;
static int32_t fcoder_metacmd_ID_snipe_token_or_word_right = 106;
static int32_t fcoder_metacmd_ID_query_replace_selection = 107;
static int32_t fcoder_metacmd_ID_move_line_up = 108;
static int32_t fcoder_metacmd_ID_move_down_textual = 109;
static int32_t fcoder_metacmd_ID_move_line_down = 110;
static int32_t fcoder_metacmd_ID_duplicate_line = 111;
static int32_t fcoder_metacmd_ID_delete_line = 112;
static int32_t fcoder_metacmd_ID_paste_and_indent = 113;
static int32_t fcoder_metacmd_ID_paste_next_and_indent = 114;
static int32_t fcoder_metacmd_ID_open_long_braces = 115;
static int32_t fcoder_metacmd_ID_open_long_braces_semicolon = 116;
static int32_t fcoder_metacmd_ID_open_long_braces_break = 117;
static int32_t fcoder_metacmd_ID_if0_off = 118;
static int32_t fcoder_metacmd_ID_write_todo = 119;
static int32_t fcoder_metacmd_ID_write_hack = 120;
static int32_t fcoder_metacmd_ID_write_note = 121;
static int32_t fcoder_metacmd_ID_write_block = 122;
static int32_t fcoder_metacmd_ID_write_zero_struct = 123;
static int32_t fcoder_metacmd_ID_open_file_in_quotes = 124;
static int32_t fcoder_metacmd_ID_open_in_other = 125;
static int32_t fcoder_metacmd_ID_open_matching_file_cpp = 126;
static int32_t fcoder_metacmd_ID_execute_arbitrary_command = 127;
static int32_t fcoder_metacmd_ID_list_all_functions_current_buffer = 128;
static int32_t fcoder_metacmd_ID_goto_jump_at_cursor_direct = 129;
static int32_t fcoder_metacmd_ID_goto_jump_at_cursor_same_panel_direct = 130;
static int32_t fcoder_metacmd_ID_goto_next_jump_direct = 131;
static int32_t fcoder_metacmd_ID_goto_prev_jump_direct = 132;
static int32_t fcoder_metacmd_ID_goto_next_jump_no_skips_direct = 133;
static int32_t fcoder_metacmd_ID_goto_prev_jump_no_skips_direct = 134;
static int32_t fcoder_metacmd_ID_goto_first_jump_direct = 135;
static int32_t fcoder_metacmd_ID_newline_or_goto_position_direct = 136;
static int32_t fcoder_metacmd_ID_newline_or_goto_position_same_panel_direct = 137;
static int32_t fcoder_metacmd_ID_goto_jump_at_cursor_sticky = 138;
static int32_t fcoder_metacmd_ID_goto_jump_at_cursor_same_panel_sticky = 139;
static int32_t fcoder_metacmd_ID_goto_next_jump_sticky = 140;
static int32_t fcoder_metacmd_ID_goto_prev_jump_sticky = 141;
static int32_t fcoder_metacmd_ID_goto_next_jump_no_skips_sticky = 142;
static int32_t fcoder_metacmd_ID_goto_prev_jump_no_skips_sticky = 143;
static int32_t fcoder_metacmd_ID_goto_first_jump_sticky = 144;
static int32_t fcoder_metacmd_ID_newline_or_goto_position_sticky = 145;
static int32_t fcoder_metacmd_ID_newline_or_goto_position_same_panel_sticky = 146;
static int32_t fcoder_metacmd_ID_open_all_code = 147;
static int32_t fcoder_metacmd_ID_open_all_code_recursive = 148;
static int32_t fcoder_metacmd_ID_close_all_code = 149;
static int32_t fcoder_metacmd_ID_load_project = 150;
static int32_t fcoder_metacmd_ID_reload_current_project = 151;
static int32_t fcoder_metacmd_ID_project_fkey_command = 152;
static int32_t fcoder_metacmd_ID_project_go_to_root_directory = 153;
static int32_t fcoder_metacmd_ID_setup_new_project = 154;
static int32_t fcoder_metacmd_ID_set_bindings_choose = 155;
static int32_t fcoder_metacmd_ID_set_bindings_default = 156;
static int32_t fcoder_metacmd_ID_set_bindings_mac_default = 157;
static int32_t fcoder_metacmd_ID_highlight_surrounding_scope = 158;
static int32_t fcoder_metacmd_ID_highlight_next_scope_absolute = 159;
static int32_t fcoder_metacmd_ID_highlight_prev_scope_absolute = 160;
static int32_t fcoder_metacmd_ID_place_in_scope = 161;
static int32_t fcoder_metacmd_ID_delete_current_scope = 162;
static int32_t fcoder_metacmd_ID_scope_absorb_down = 163;
static int32_t fcoder_metacmd_ID_list_all_locations = 164;
static int32_t fcoder_metacmd_ID_list_all_substring_locations = 165;
static int32_t fcoder_metacmd_ID_list_all_locations_case_insensitive = 166;
static int32_t fcoder_metacmd_ID_list_all_substring_locations_case_insensitive = 167;
static int32_t fcoder_metacmd_ID_list_all_locations_of_identifier = 168;
static int32_t fcoder_metacmd_ID_list_all_locations_of_identifier_case_insensitive = 169;
static int32_t fcoder_metacmd_ID_list_all_locations_of_selection = 170;
static int32_t fcoder_metacmd_ID_list_all_locations_of_selection_case_insensitive = 171;
static int32_t fcoder_metacmd_ID_word_complete = 172;
static int32_t fcoder_metacmd_ID_execute_previous_cli = 173;
static int32_t fcoder_metacmd_ID_execute_any_cli = 174;
static int32_t fcoder_metacmd_ID_kill_rect = 175;
static int32_t fcoder_metacmd_ID_multi_line_edit = 176;
static int32_t fcoder_metacmd_ID_rename_parameter = 177;
static int32_t fcoder_metacmd_ID_write_explicit_enum_values = 178;
static int32_t fcoder_metacmd_ID_miblo_increment_basic = 179;
static int32_t fcoder_metacmd_ID_miblo_decrement_basic = 180;
static int32_t fcoder_metacmd_ID_miblo_increment_time_stamp = 181;
static int32_t fcoder_metacmd_ID_miblo_decrement_time_stamp = 182;
static int32_t fcoder_metacmd_ID_miblo_increment_time_stamp_minute = 183;
static int32_t fcoder_metacmd_ID_miblo_decrement_time_stamp_minute = 184;
