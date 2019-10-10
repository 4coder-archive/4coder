/*
 * nonsense remapping.h file gotta remove it before I ship hopefully
 */

#if defined(CUSTOM_COMMAND_SIG)

function void
setup_default_mapping(Mapping *mapping){
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(mapid_global);
    Bind(change_active_panel,           KeyCode_Comma, MDFR_CTRL);
    Bind(change_active_panel_backwards, KeyCode_Comma, MDFR_CTRL, MDFR_SHIFT);
    Bind(interactive_new,               KeyCode_N, MDFR_CTRL);
    Bind(interactive_open_or_new,       KeyCode_O, MDFR_CTRL);
    Bind(open_in_other,                 KeyCode_O, MDFR_ALT);
    Bind(interactive_kill_buffer,       KeyCode_K, MDFR_CTRL);
    Bind(interactive_switch_buffer,     KeyCode_I, MDFR_CTRL);
    Bind(project_go_to_root_directory,  KeyCode_H, MDFR_CTRL);
    Bind(save_all_dirty_buffers,        KeyCode_S, MDFR_CTRL, MDFR_SHIFT);
    Bind(change_to_build_panel,         KeyCode_Period, MDFR_ALT);
    Bind(close_build_panel,             KeyCode_Comma, MDFR_ALT);
    Bind(goto_next_jump,                KeyCode_N, MDFR_ALT);
    Bind(goto_prev_jump,                KeyCode_N, MDFR_ALT, MDFR_SHIFT);
    Bind(goto_first_jump,               KeyCode_M, MDFR_ALT, MDFR_SHIFT);
    Bind(build_in_build_panel,          KeyCode_M, MDFR_ALT);
    Bind(toggle_filebar,                KeyCode_B, MDFR_ALT);
    Bind(execute_any_cli,               KeyCode_Z, MDFR_ALT);
    Bind(execute_previous_cli,          KeyCode_Z, MDFR_ALT, MDFR_SHIFT);
    Bind(command_lister,                KeyCode_X, MDFR_ALT);
    Bind(project_command_lister,        KeyCode_X, MDFR_ALT, MDFR_SHIFT);
    Bind(list_all_functions_all_buffers_lister, KeyCode_I, MDFR_CTRL, MDFR_SHIFT);
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
    Bind(exit_4coder,          KeyCode_F4, MDFR_ALT);
    //Bind(KeyCodeExt_MouseWheel, mouse_wheel_scroll);
    //Bind(KeyCodeExt_MouseWheel, MDFR_CTRL, mouse_wheel_change_face_size);
    
    SelectMap(mapid_file);
    ParentMap(mapid_global);
    BindTextInput(write_text_input);
    //Bind(KeyCodeExt_MouseLeft, click_set_cursor_and_mark);
    //Bind(KeyCodeExt_ClickActivateView, click_set_cursor_and_mark);
    //Bind(KeyCodeExt_MouseLeftRelease, click_set_cursor);
    //Bind(KeyCodeExt_MouseMove, click_set_cursor_if_lbutton);
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
    Bind(goto_beginning_of_file, KeyCode_PageUp, MDFR_CTRL);
    Bind(goto_end_of_file,       KeyCode_PageDown, MDFR_CTRL);
    Bind(move_up_to_blank_line_skip_whitespace, KeyCode_Up, MDFR_CTRL);
    Bind(move_down_to_blank_line_end,      KeyCode_Down, MDFR_CTRL);
    Bind(move_left_whitespace_boundary,    KeyCode_Left, MDFR_CTRL);
    Bind(move_right_whitespace_boundary,   KeyCode_Right, MDFR_CTRL);
    Bind(move_line_up,                     KeyCode_Up, MDFR_ALT);
    Bind(move_line_down,                   KeyCode_Down, MDFR_ALT);
    Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, MDFR_CTRL);
    Bind(delete_alpha_numeric_boundary,    KeyCode_Delete, MDFR_CTRL);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, MDFR_ALT);
    Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, MDFR_ALT);
    Bind(set_mark,                    KeyCode_Space, MDFR_CTRL);
    Bind(replace_in_range,            KeyCode_A, MDFR_CTRL);
    Bind(copy,                        KeyCode_C, MDFR_CTRL);
    Bind(delete_range,                KeyCode_D, MDFR_CTRL);
    Bind(delete_line,                 KeyCode_D, MDFR_CTRL, MDFR_SHIFT);
    Bind(center_view,                 KeyCode_E, MDFR_CTRL);
    Bind(left_adjust_view,            KeyCode_E, MDFR_CTRL, MDFR_SHIFT);
    Bind(search,                      KeyCode_F, MDFR_CTRL);
    Bind(list_all_locations,          KeyCode_F, MDFR_CTRL, MDFR_SHIFT);
    Bind(list_all_substring_locations_case_insensitive, KeyCode_F, MDFR_ALT);
    Bind(goto_line,                   KeyCode_G, MDFR_CTRL);
    Bind(list_all_locations_of_selection,  KeyCode_G, MDFR_CTRL, MDFR_SHIFT);
    Bind(snippet_lister,              KeyCode_J, MDFR_CTRL);
    Bind(kill_buffer,                 KeyCode_K, MDFR_CTRL, MDFR_SHIFT);
    Bind(duplicate_line,              KeyCode_L, MDFR_CTRL);
    Bind(cursor_mark_swap,            KeyCode_M, MDFR_CTRL);
    Bind(reopen,                      KeyCode_O, MDFR_CTRL, MDFR_SHIFT);
    Bind(query_replace,               KeyCode_Q, MDFR_CTRL);
    Bind(query_replace_identifier,    KeyCode_Q, MDFR_CTRL, MDFR_SHIFT);
    Bind(query_replace_selection,     KeyCode_Q, MDFR_ALT);
    Bind(reverse_search,              KeyCode_R, MDFR_CTRL);
    Bind(save,                        KeyCode_S, MDFR_CTRL);
    Bind(search_identifier,           KeyCode_T, MDFR_CTRL);
    Bind(list_all_locations_of_identifier, KeyCode_T, MDFR_CTRL, MDFR_SHIFT);
    Bind(paste_and_indent,            KeyCode_V, MDFR_CTRL);
    Bind(paste_next_and_indent,       KeyCode_V, MDFR_CTRL, MDFR_SHIFT);
    Bind(cut,                         KeyCode_X, MDFR_CTRL);
    Bind(redo,                        KeyCode_Y, MDFR_CTRL);
    Bind(undo,                        KeyCode_Z, MDFR_CTRL);
    Bind(view_buffer_other_panel,     KeyCode_1, MDFR_CTRL);
    Bind(swap_buffers_between_panels, KeyCode_2, MDFR_CTRL);
    Bind(if_read_only_goto_position,  KeyCode_Return);
    Bind(if_read_only_goto_position_same_panel, KeyCode_Return, MDFR_SHIFT);
    Bind(view_jump_list_with_lister,  KeyCode_Period, MDFR_CTRL, MDFR_SHIFT);
    
    SelectMap(default_code_map);
    ParentMap(mapid_file);
    BindTextInput(write_text_and_auto_indent);
    Bind(move_left_alpha_numeric_boundary,           KeyCode_Left, MDFR_CTRL);
    Bind(move_right_alpha_numeric_boundary,          KeyCode_Right, MDFR_CTRL);
    Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, MDFR_ALT);
    Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, MDFR_ALT);
    Bind(comment_line_toggle,        KeyCode_Semicolon, MDFR_CTRL);
    Bind(word_complete,              KeyCode_Tab);
    Bind(auto_indent_range,          KeyCode_Tab, MDFR_CTRL);
    Bind(auto_indent_line_at_cursor, KeyCode_Tab, MDFR_SHIFT);
    Bind(write_block,                KeyCode_R, MDFR_ALT);
    Bind(write_todo,                 KeyCode_T, MDFR_ALT);
    Bind(write_note,                 KeyCode_Y, MDFR_ALT);
    Bind(list_all_locations_of_type_definition,               KeyCode_D, MDFR_ALT);
    Bind(list_all_locations_of_type_definition_of_identifier, KeyCode_T, MDFR_ALT);
    Bind(open_long_braces,           KeyCode_LeftBracket, MDFR_CTRL);
    Bind(open_long_braces_semicolon, KeyCode_LeftBracket, MDFR_CTRL, MDFR_SHIFT);
    Bind(open_long_braces_break,     KeyCode_RightBracket, MDFR_CTRL, MDFR_SHIFT);
    Bind(select_surrounding_scope,   KeyCode_LeftBracket, MDFR_ALT);
    Bind(select_prev_scope_absolute, KeyCode_RightBracket, MDFR_ALT);
    Bind(select_next_scope_absolute, KeyCode_Quote, MDFR_ALT);
    Bind(select_next_scope_after_current, KeyCode_Quote, MDFR_ALT, MDFR_SHIFT);
    Bind(place_in_scope,             KeyCode_ForwardSlash, MDFR_ALT);
    Bind(delete_current_scope,       KeyCode_Minus, MDFR_ALT);
    Bind(if0_off,                    KeyCode_I, MDFR_ALT);
    Bind(open_file_in_quotes,        KeyCode_1, MDFR_ALT);
    Bind(open_matching_file_cpp,     KeyCode_2, MDFR_ALT);
    Bind(write_zero_struct,          KeyCode_0, MDFR_CTRL);
    
    SelectMap(default_lister_ui_map);
    ParentMap(mapid_global);
    BindTextInput(lister__write_character);
    Bind(lister__quit,     KeyCode_Escape);
    Bind(lister__activate, KeyCode_Return);
    Bind(lister__activate, KeyCode_Tab);
    Bind(lister__backspace_text_field, KeyCode_Backspace);
    Bind(lister__move_up, KeyCode_Up);
    Bind(lister__move_down, KeyCode_Down);
    //Bind(KeyCodeExt_MouseWheel, MDFR_NONE, lister__wheel_scroll);
    //Bind(KeyCodeExt_MouseLeft, MDFR_NONE, lister__mouse_press);
    //Bind(KeyCodeExt_MouseLeftRelease, MDFR_NONE, lister__mouse_release);
    //Bind(KeyCodeExt_MouseMove, MDFR_NONE, lister__repaint);
    //Bind(KeyCodeExt_Animate, MDFR_NONE, lister__repaint);
}

#endif

// BOTTOM

