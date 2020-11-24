/*
4coder_mac_map.cpp - Instantiate mac keyboard bindings.
*/

// TOP

function void
setup_mac_mapping(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(global_id);
    Bind(keyboard_macro_start_recording , KeyCode_U, KeyCode_Command);
    Bind(keyboard_macro_finish_recording, KeyCode_U, KeyCode_Command, KeyCode_Shift);
    Bind(keyboard_macro_replay,           KeyCode_U, KeyCode_Control);
    Bind(change_active_panel,           KeyCode_Comma, KeyCode_Command);
    Bind(change_active_panel_backwards, KeyCode_Comma, KeyCode_Command, KeyCode_Shift);
    Bind(interactive_new,               KeyCode_N, KeyCode_Command);
    Bind(interactive_open_or_new,       KeyCode_O, KeyCode_Command);
    Bind(open_in_other,                 KeyCode_O, KeyCode_Control);
    Bind(interactive_kill_buffer,       KeyCode_K, KeyCode_Command);
    Bind(interactive_switch_buffer,     KeyCode_I, KeyCode_Command);
    Bind(project_go_to_root_directory,  KeyCode_H, KeyCode_Command);
    Bind(save_all_dirty_buffers,        KeyCode_S, KeyCode_Command, KeyCode_Shift);
    Bind(change_to_build_panel,         KeyCode_Period, KeyCode_Control);
    Bind(close_build_panel,             KeyCode_Comma, KeyCode_Control);
    Bind(goto_next_jump,                KeyCode_N, KeyCode_Control);
    Bind(goto_prev_jump,                KeyCode_N, KeyCode_Control, KeyCode_Shift);
    Bind(build_in_build_panel,          KeyCode_M, KeyCode_Control);
    Bind(goto_first_jump,               KeyCode_M, KeyCode_Control, KeyCode_Shift);
    Bind(toggle_filebar,                KeyCode_B, KeyCode_Control);
    Bind(execute_any_cli,               KeyCode_Z, KeyCode_Control);
    Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Control, KeyCode_Shift);
    Bind(command_lister,                KeyCode_X, KeyCode_Control);
    Bind(project_command_lister,        KeyCode_X, KeyCode_Control, KeyCode_Shift);
    Bind(quick_swap_buffer,             KeyCode_BackwardSlash, KeyCode_Command);
    Bind(jump_to_last_point,            KeyCode_P, KeyCode_Command);
    Bind(list_all_functions_current_buffer, KeyCode_I, KeyCode_Command, KeyCode_Shift);
    Bind(project_fkey_command, KeyCode_F1);
    Bind(project_fkey_command, KeyCode_F2);
    Bind(project_fkey_command, KeyCode_F3);
    Bind(project_fkey_command, KeyCode_F4);
    Bind(project_fkey_command, KeyCode_F5);
    Bind(project_fkey_command, KeyCode_F6);
    Bind(project_fkey_command, KeyCode_F7);
    Bind(project_fkey_command, KeyCode_F8);
    Bind(project_fkey_command, KeyCode_F9);
    Bind(project_fkey_command, KeyCode_F10);
    Bind(project_fkey_command, KeyCode_F11);
    Bind(project_fkey_command, KeyCode_F12);
    Bind(project_fkey_command, KeyCode_F13);
    Bind(project_fkey_command, KeyCode_F14);
    Bind(project_fkey_command, KeyCode_F15);
    Bind(project_fkey_command, KeyCode_F16);
    Bind(exit_4coder,          KeyCode_F4, KeyCode_Alt);
    
    SelectMap(file_id);
    Bind(delete_char,            KeyCode_Delete);
    Bind(backspace_char,         KeyCode_Backspace);
    Bind(move_up,                KeyCode_Up);
    Bind(move_down,              KeyCode_Down);
    Bind(move_left,              KeyCode_Left);
    Bind(move_right,             KeyCode_Right);
    Bind(seek_end_of_line,       KeyCode_End);
    Bind(seek_beginning_of_line, KeyCode_Home);
    Bind(page_up,                KeyCode_PageUp);
    Bind(page_down,              KeyCode_PageDown);
    Bind(goto_beginning_of_file, KeyCode_PageUp, KeyCode_Command);
    Bind(goto_end_of_file,       KeyCode_PageDown, KeyCode_Command);
    Bind(move_up_to_blank_line_end,        KeyCode_Up, KeyCode_Command);
    Bind(move_down_to_blank_line_end,      KeyCode_Down, KeyCode_Command);
    Bind(move_left_whitespace_boundary,    KeyCode_Left, KeyCode_Command);
    Bind(move_right_whitespace_boundary,   KeyCode_Right, KeyCode_Command);
    Bind(move_line_up,                     KeyCode_Up, KeyCode_Alt);
    Bind(move_line_down,                   KeyCode_Down, KeyCode_Alt);
    Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Command);
    Bind(delete_alpha_numeric_boundary,    KeyCode_Delete, KeyCode_Command);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Control);
    Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Control);
    Bind(set_mark,                    KeyCode_Space, KeyCode_Control);
    Bind(set_mark,                    KeyCode_ForwardSlash, KeyCode_Command);
    Bind(replace_in_range,            KeyCode_A, KeyCode_Command);
    Bind(copy,                        KeyCode_C, KeyCode_Command);
    Bind(delete_range,                KeyCode_D, KeyCode_Command);
    Bind(delete_line,                 KeyCode_D, KeyCode_Command, KeyCode_Shift);
    Bind(center_view,                 KeyCode_E, KeyCode_Command);
    Bind(left_adjust_view,            KeyCode_E, KeyCode_Command, KeyCode_Shift);
    Bind(search,                      KeyCode_F, KeyCode_Command);
    Bind(list_all_locations,          KeyCode_F, KeyCode_Command, KeyCode_Shift);
    Bind(list_all_substring_locations_case_insensitive, KeyCode_F, KeyCode_Control);
    Bind(goto_line,                   KeyCode_G, KeyCode_Command);
    Bind(list_all_locations_of_selection,  KeyCode_G, KeyCode_Command, KeyCode_Shift);
    Bind(snippet_lister,              KeyCode_J, KeyCode_Command);
    Bind(kill_buffer,                 KeyCode_K, KeyCode_Command, KeyCode_Shift);
    Bind(duplicate_line,              KeyCode_L, KeyCode_Command);
    Bind(cursor_mark_swap,            KeyCode_M, KeyCode_Command);
    Bind(reopen,                      KeyCode_O, KeyCode_Command, KeyCode_Shift);
    Bind(query_replace,               KeyCode_Q, KeyCode_Command);
    Bind(query_replace_identifier,    KeyCode_Q, KeyCode_Command, KeyCode_Shift);
    Bind(query_replace_selection,     KeyCode_Q, KeyCode_Control);
    Bind(reverse_search,              KeyCode_R, KeyCode_Command);
    Bind(save,                        KeyCode_S, KeyCode_Command);
    Bind(save_all_dirty_buffers,      KeyCode_S, KeyCode_Command, KeyCode_Shift);
    Bind(search_identifier,           KeyCode_T, KeyCode_Command);
    Bind(list_all_locations_of_identifier, KeyCode_T, KeyCode_Command, KeyCode_Shift);
    Bind(paste_and_indent,            KeyCode_V, KeyCode_Command);
    Bind(paste_next_and_indent,       KeyCode_V, KeyCode_Command, KeyCode_Shift);
    Bind(cut,                         KeyCode_X, KeyCode_Command);
    Bind(redo,                        KeyCode_Y, KeyCode_Command);
    Bind(undo,                        KeyCode_Z, KeyCode_Command);
    Bind(view_buffer_other_panel,     KeyCode_1, KeyCode_Command);
    Bind(swap_panels,                 KeyCode_2, KeyCode_Command);
    Bind(if_read_only_goto_position,  KeyCode_Return);
    Bind(if_read_only_goto_position_same_panel, KeyCode_Return, KeyCode_Shift);
    Bind(view_jump_list_with_lister,  KeyCode_Period, KeyCode_Command, KeyCode_Shift);
    
    SelectMap(code_id);
    Bind(move_left_alpha_numeric_boundary,           KeyCode_Left, KeyCode_Command);
    Bind(move_right_alpha_numeric_boundary,          KeyCode_Right, KeyCode_Command);
    Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, KeyCode_Control);
    Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, KeyCode_Control);
    Bind(comment_line_toggle,        KeyCode_Semicolon, KeyCode_Command);
    Bind(word_complete,              KeyCode_Tab);
    Bind(auto_indent_range,          KeyCode_Tab, KeyCode_Command);
    Bind(auto_indent_line_at_cursor, KeyCode_Tab, KeyCode_Shift);
    Bind(word_complete_drop_down,    KeyCode_Tab, KeyCode_Shift, KeyCode_Command);
    Bind(write_block,                KeyCode_R, KeyCode_Control);
    Bind(write_todo,                 KeyCode_T, KeyCode_Control);
    Bind(write_note,                 KeyCode_Y, KeyCode_Control);
    Bind(list_all_locations_of_type_definition,               KeyCode_D, KeyCode_Control);
    Bind(list_all_locations_of_type_definition_of_identifier, KeyCode_T, KeyCode_Control, KeyCode_Shift);
    Bind(open_long_braces,           KeyCode_LeftBracket, KeyCode_Command);
    Bind(open_long_braces_semicolon, KeyCode_LeftBracket, KeyCode_Command, KeyCode_Shift);
    Bind(open_long_braces_break,     KeyCode_RightBracket, KeyCode_Command, KeyCode_Shift);
    Bind(select_surrounding_scope,   KeyCode_LeftBracket, KeyCode_Control);
    Bind(select_surrounding_scope_maximal, KeyCode_LeftBracket, KeyCode_Control, KeyCode_Shift);
    Bind(select_prev_scope_absolute, KeyCode_RightBracket, KeyCode_Control);
    Bind(select_prev_top_most_scope, KeyCode_RightBracket, KeyCode_Control, KeyCode_Shift);
    Bind(select_next_scope_absolute, KeyCode_Quote, KeyCode_Control);
    Bind(select_next_scope_after_current, KeyCode_Quote, KeyCode_Control, KeyCode_Shift);
    Bind(place_in_scope,             KeyCode_ForwardSlash, KeyCode_Control);
    Bind(delete_current_scope,       KeyCode_Minus, KeyCode_Control);
    Bind(if0_off,                    KeyCode_I, KeyCode_Control);
    Bind(open_file_in_quotes,        KeyCode_1, KeyCode_Control);
    Bind(open_matching_file_cpp,     KeyCode_2, KeyCode_Control);
    Bind(write_zero_struct,          KeyCode_0, KeyCode_Command);
    Bind(jump_to_definition_at_cursor, KeyCode_W, KeyCode_Command);
}

// BOTTOM

