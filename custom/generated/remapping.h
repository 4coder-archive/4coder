#if defined(CUSTOM_COMMAND_SIG)
void fill_keys_default(Bind_Helper *context){
    begin_map(context, mapid_global);
    bind(context, KeyCode_Comma, MDFR_CTRL, change_active_panel);
    bind(context, KeyCode_Comma, MDFR_CTRL|MDFR_SHIFT, change_active_panel_backwards);
    bind(context, KeyCode_N, MDFR_CTRL, interactive_new);
    bind(context, KeyCode_O, MDFR_CTRL, interactive_open_or_new);
    bind(context, KeyCode_O, MDFR_ALT, open_in_other);
    bind(context, KeyCode_K, MDFR_CTRL, interactive_kill_buffer);
    bind(context, KeyCode_I, MDFR_CTRL, interactive_switch_buffer);
    bind(context, KeyCode_H, MDFR_CTRL, project_go_to_root_directory);
    bind(context, KeyCode_S, MDFR_CTRL|MDFR_SHIFT, save_all_dirty_buffers);
    bind(context, KeyCode_Period, MDFR_ALT, change_to_build_panel);
    bind(context, KeyCode_Comma, MDFR_ALT, close_build_panel);
    bind(context, KeyCode_N, MDFR_ALT, goto_next_jump);
    bind(context, KeyCode_N, MDFR_ALT|MDFR_SHIFT, goto_prev_jump);
    bind(context, KeyCode_M, MDFR_ALT|MDFR_SHIFT, goto_first_jump);
    bind(context, KeyCode_M, MDFR_ALT, build_in_build_panel);
    bind(context, KeyCode_B, MDFR_ALT, toggle_filebar);
    bind(context, KeyCode_Z, MDFR_ALT, execute_any_cli);
    bind(context, KeyCode_Z, MDFR_ALT|MDFR_SHIFT, execute_previous_cli);
    bind(context, KeyCode_X, MDFR_ALT, command_lister);
    bind(context, KeyCode_X, MDFR_ALT|MDFR_SHIFT, project_command_lister);
    bind(context, KeyCode_I, MDFR_CTRL|MDFR_SHIFT, list_all_functions_all_buffers_lister);
    bind(context, KeyCode_F1, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F2, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F3, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F4, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F5, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F6, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F7, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F8, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F9, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F10, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F11, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F12, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F13, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F14, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F15, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F16, MDFR_NONE, project_fkey_command);
    bind(context, KeyCode_F4, MDFR_ALT, exit_4coder);
    //bind(context, KeyCodeExt_MouseWheel, MDFR_NONE, mouse_wheel_scroll);
    //bind(context, KeyCodeExt_MouseWheel, MDFR_CTRL, mouse_wheel_change_face_size);
    end_map(context);
    begin_map(context, mapid_file);
    bind_text_input(context, write_text_input);
    //bind(context, KeyCodeExt_MouseLeft, MDFR_NONE, click_set_cursor_and_mark);
    //bind(context, KeyCodeExt_ClickActivateView, MDFR_NONE, click_set_cursor_and_mark);
    //bind(context, KeyCodeExt_MouseLeftRelease, MDFR_NONE, click_set_cursor);
    //bind(context, KeyCodeExt_MouseMove, MDFR_NONE, click_set_cursor_if_lbutton);
    bind(context, KeyCode_Delete, MDFR_NONE, delete_char);
    bind(context, KeyCode_Delete, MDFR_SHIFT, delete_char);
    bind(context, KeyCode_Backspace, MDFR_NONE, backspace_char);
    bind(context, KeyCode_Backspace, MDFR_SHIFT, backspace_char);
    bind(context, KeyCode_Up, MDFR_NONE, move_up);
    bind(context, KeyCode_Down, MDFR_NONE, move_down);
    bind(context, KeyCode_Left, MDFR_NONE, move_left);
    bind(context, KeyCode_Right, MDFR_NONE, move_right);
    bind(context, KeyCode_Up, MDFR_SHIFT, move_up);
    bind(context, KeyCode_Down, MDFR_SHIFT, move_down);
    bind(context, KeyCode_Left, MDFR_SHIFT, move_left);
    bind(context, KeyCode_Right, MDFR_SHIFT, move_right);
    bind(context, KeyCode_End, MDFR_NONE, seek_end_of_line);
    bind(context, KeyCode_Home, MDFR_NONE, seek_beginning_of_line);
    bind(context, KeyCode_PageUp, MDFR_CTRL, goto_beginning_of_file);
    bind(context, KeyCode_PageDown, MDFR_CTRL, goto_end_of_file);
    bind(context, KeyCode_PageUp, MDFR_NONE, page_up);
    bind(context, KeyCode_PageDown, MDFR_NONE, page_down);
    bind(context, KeyCode_End, MDFR_SHIFT, seek_end_of_line);
    bind(context, KeyCode_Home, MDFR_SHIFT, seek_beginning_of_line);
    bind(context, KeyCode_PageUp, MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
    bind(context, KeyCode_PageDown, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
    bind(context, KeyCode_PageUp, MDFR_SHIFT, page_up);
    bind(context, KeyCode_PageDown, MDFR_SHIFT, page_down);
    bind(context, KeyCode_Up, MDFR_CTRL, move_up_to_blank_line_skip_whitespace);
    bind(context, KeyCode_Down, MDFR_CTRL, move_down_to_blank_line_end);
    bind(context, KeyCode_Left, MDFR_CTRL, move_left_whitespace_boundary);
    bind(context, KeyCode_Right, MDFR_CTRL, move_right_whitespace_boundary);
    bind(context, KeyCode_Up, MDFR_CTRL|MDFR_SHIFT, move_up_to_blank_line_skip_whitespace);
    bind(context, KeyCode_Down, MDFR_CTRL|MDFR_SHIFT, move_down_to_blank_line_end);
    bind(context, KeyCode_Left, MDFR_CTRL|MDFR_SHIFT, move_left_whitespace_boundary);
    bind(context, KeyCode_Right, MDFR_CTRL|MDFR_SHIFT, move_right_whitespace_boundary);
    bind(context, KeyCode_Up, MDFR_ALT, move_line_up);
    bind(context, KeyCode_Down, MDFR_ALT, move_line_down);
    bind(context, KeyCode_Backspace, MDFR_CTRL, backspace_alpha_numeric_boundary);
    bind(context, KeyCode_Delete, MDFR_CTRL, delete_alpha_numeric_boundary);
    bind(context, KeyCode_Backspace, MDFR_ALT, snipe_backward_whitespace_or_token_boundary);
    bind(context, KeyCode_Delete, MDFR_ALT, snipe_forward_whitespace_or_token_boundary);
    bind(context, KeyCode_Space, MDFR_CTRL, set_mark);
    bind(context, KeyCode_A, MDFR_CTRL, replace_in_range);
    bind(context, KeyCode_C, MDFR_CTRL, copy);
    bind(context, KeyCode_D, MDFR_CTRL, delete_range);
    bind(context, KeyCode_D, MDFR_CTRL|MDFR_SHIFT, delete_line);
    bind(context, KeyCode_E, MDFR_CTRL, center_view);
    bind(context, KeyCode_E, MDFR_CTRL|MDFR_SHIFT, left_adjust_view);
    bind(context, KeyCode_F, MDFR_CTRL, search);
    bind(context, KeyCode_F, MDFR_CTRL|MDFR_SHIFT, list_all_locations);
    bind(context, KeyCode_F, MDFR_ALT, list_all_substring_locations_case_insensitive);
    bind(context, KeyCode_G, MDFR_CTRL, goto_line);
    bind(context, KeyCode_G, MDFR_CTRL|MDFR_SHIFT, list_all_locations_of_selection);
    bind(context, KeyCode_J, MDFR_CTRL, snippet_lister);
    bind(context, KeyCode_K, MDFR_CTRL|MDFR_SHIFT, kill_buffer);
    bind(context, KeyCode_L, MDFR_CTRL, duplicate_line);
    bind(context, KeyCode_M, MDFR_CTRL, cursor_mark_swap);
    bind(context, KeyCode_O, MDFR_CTRL|MDFR_SHIFT, reopen);
    bind(context, KeyCode_Q, MDFR_CTRL, query_replace);
    bind(context, KeyCode_Q, MDFR_CTRL|MDFR_SHIFT, query_replace_identifier);
    bind(context, KeyCode_Q, MDFR_ALT, query_replace_selection);
    bind(context, KeyCode_R, MDFR_CTRL, reverse_search);
    bind(context, KeyCode_S, MDFR_CTRL, save);
    bind(context, KeyCode_T, MDFR_CTRL, search_identifier);
    bind(context, KeyCode_T, MDFR_CTRL|MDFR_SHIFT, list_all_locations_of_identifier);
    bind(context, KeyCode_V, MDFR_CTRL, paste_and_indent);
    bind(context, KeyCode_V, MDFR_CTRL|MDFR_SHIFT, paste_next_and_indent);
    bind(context, KeyCode_X, MDFR_CTRL, cut);
    bind(context, KeyCode_Y, MDFR_CTRL, redo);
    bind(context, KeyCode_Z, MDFR_CTRL, undo);
    bind(context, KeyCode_1, MDFR_CTRL, view_buffer_other_panel);
    bind(context, KeyCode_2, MDFR_CTRL, swap_buffers_between_panels);
    bind(context, KeyCode_Return, MDFR_NONE, if_read_only_goto_position);
    bind(context, KeyCode_Return, MDFR_SHIFT, if_read_only_goto_position_same_panel);
    bind(context, KeyCode_Period, MDFR_CTRL|MDFR_SHIFT, view_jump_list_with_lister);
    end_map(context);
    begin_map(context, default_code_map);
    inherit_map(context, mapid_file);
    bind_text_input(context, write_text_and_auto_indent);
    bind(context, KeyCode_Left, MDFR_CTRL, move_left_alpha_numeric_boundary);
    bind(context, KeyCode_Left, MDFR_CTRL|MDFR_SHIFT, move_left_alpha_numeric_boundary);
    bind(context, KeyCode_Right, MDFR_CTRL, move_right_alpha_numeric_boundary);
    bind(context, KeyCode_Right, MDFR_CTRL|MDFR_SHIFT, move_right_alpha_numeric_boundary);
    bind(context, KeyCode_Left, MDFR_ALT, move_left_alpha_numeric_or_camel_boundary);
    bind(context, KeyCode_Left, MDFR_ALT|MDFR_SHIFT, move_left_alpha_numeric_or_camel_boundary);
    bind(context, KeyCode_Right, MDFR_ALT|MDFR_SHIFT, move_right_alpha_numeric_or_camel_boundary);
    bind(context, KeyCode_Semicolon, MDFR_CTRL, comment_line_toggle);
    bind(context, KeyCode_Tab, MDFR_NONE, word_complete);
    bind(context, KeyCode_Tab, MDFR_CTRL, auto_indent_range);
    bind(context, KeyCode_Tab, MDFR_SHIFT, auto_indent_line_at_cursor);
    bind(context, KeyCode_R, MDFR_ALT, write_block);
    bind(context, KeyCode_T, MDFR_ALT, write_todo);
    bind(context, KeyCode_Y, MDFR_ALT, write_note);
    bind(context, KeyCode_D, MDFR_ALT, list_all_locations_of_type_definition);
    bind(context, KeyCode_T, MDFR_ALT, list_all_locations_of_type_definition_of_identifier);
    bind(context, KeyCode_LeftBracket, MDFR_CTRL, open_long_braces);
    bind(context, KeyCode_LeftBracket, MDFR_CTRL|MDFR_SHIFT, open_long_braces_semicolon);
    bind(context, KeyCode_RightBracket, MDFR_CTRL|MDFR_SHIFT, open_long_braces_break);
    bind(context, KeyCode_LeftBracket, MDFR_ALT, select_surrounding_scope);
    bind(context, KeyCode_RightBracket, MDFR_ALT, select_prev_scope_absolute);
    bind(context, KeyCode_Quote, MDFR_ALT, select_next_scope_absolute);
    bind(context, KeyCode_Quote, MDFR_ALT|MDFR_SHIFT, select_next_scope_after_current);
    bind(context, KeyCode_ForwardSlash, MDFR_ALT, place_in_scope);
    bind(context, KeyCode_Minus, MDFR_ALT, delete_current_scope);
    bind(context, KeyCode_I, MDFR_ALT, if0_off);
    bind(context, KeyCode_1, MDFR_ALT, open_file_in_quotes);
    bind(context, KeyCode_2, MDFR_ALT, open_matching_file_cpp);
    bind(context, KeyCode_0, MDFR_CTRL, write_zero_struct);
    end_map(context);
    begin_map(context, default_lister_ui_map);
    bind_text_input(context, lister__write_character);
    bind(context, KeyCode_Escape, MDFR_NONE, lister__quit);
    bind(context, KeyCode_Return, MDFR_NONE, lister__activate);
    bind(context, KeyCode_Tab, MDFR_NONE, lister__activate);
    bind(context, KeyCode_Backspace, MDFR_NONE, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_CTRL, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_ALT, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_CMND, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_CTRL|MDFR_ALT, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_ALT|MDFR_CMND, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_CTRL|MDFR_CMND, lister__backspace_text_field);
    bind(context, KeyCode_Backspace, MDFR_CTRL|MDFR_ALT|MDFR_CMND, lister__backspace_text_field);
    bind(context, KeyCode_Up, MDFR_NONE, lister__move_up);
    bind(context, KeyCode_K, MDFR_ALT, lister__move_up);
    bind(context, KeyCode_PageUp, MDFR_NONE, lister__move_up);
    bind(context, KeyCode_Down, MDFR_NONE, lister__move_down);
    bind(context, KeyCode_J, MDFR_ALT, lister__move_down);
    bind(context, KeyCode_PageDown, MDFR_NONE, lister__move_down);
    //bind(context, KeyCodeExt_MouseWheel, MDFR_NONE, lister__wheel_scroll);
    //bind(context, KeyCodeExt_MouseLeft, MDFR_NONE, lister__mouse_press);
    //bind(context, KeyCodeExt_MouseLeftRelease, MDFR_NONE, lister__mouse_release);
    //bind(context, KeyCodeExt_MouseMove, MDFR_NONE, lister__repaint);
    //bind(context, KeyCodeExt_Animate, MDFR_NONE, lister__repaint);
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
static Meta_Key_Bind fcoder_binds_for_default_mapid_global[43] = {
    {0, 44, 1, "change_active_panel", 19, LINK_PROCS(change_active_panel)},
    {0, 60, 1, "change_active_panel_backwards", 29, LINK_PROCS(change_active_panel_backwards)},
    {0, 110, 1, "interactive_new", 15, LINK_PROCS(interactive_new)},
    {0, 111, 1, "interactive_open_or_new", 23, LINK_PROCS(interactive_open_or_new)},
    {0, 111, 2, "open_in_other", 13, LINK_PROCS(open_in_other)},
    {0, 107, 1, "interactive_kill_buffer", 23, LINK_PROCS(interactive_kill_buffer)},
    {0, 105, 1, "interactive_switch_buffer", 25, LINK_PROCS(interactive_switch_buffer)},
    {0, 104, 1, "project_go_to_root_directory", 28, LINK_PROCS(project_go_to_root_directory)},
    {0, 83, 1, "save_all_dirty_buffers", 22, LINK_PROCS(save_all_dirty_buffers)},
    {0, 55315, 0, "toggle_filebar", 14, LINK_PROCS(toggle_filebar)},
    {0, 55308, 0, "toggle_filebar", 14, LINK_PROCS(toggle_filebar)},
    {0, 55313, 0, "toggle_filebar", 14, LINK_PROCS(toggle_filebar)},
    {0, 46, 2, "change_to_build_panel", 21, LINK_PROCS(change_to_build_panel)},
    {0, 44, 2, "close_build_panel", 17, LINK_PROCS(close_build_panel)},
    {0, 110, 2, "goto_next_jump", 14, LINK_PROCS(goto_next_jump)},
    {0, 78, 2, "goto_prev_jump", 14, LINK_PROCS(goto_prev_jump)},
    {0, 77, 2, "goto_first_jump", 15, LINK_PROCS(goto_first_jump)},
    {0, 109, 2, "build_in_build_panel", 20, LINK_PROCS(build_in_build_panel)},
    {0, 98, 2, "toggle_filebar", 14, LINK_PROCS(toggle_filebar)},
    {0, 122, 2, "execute_any_cli", 15, LINK_PROCS(execute_any_cli)},
    {0, 90, 2, "execute_previous_cli", 20, LINK_PROCS(execute_previous_cli)},
    {0, 120, 2, "command_lister", 14, LINK_PROCS(command_lister)},
    {0, 88, 2, "project_command_lister", 22, LINK_PROCS(project_command_lister)},
    {0, 73, 1, "list_all_functions_all_buffers_lister", 37, LINK_PROCS(list_all_functions_all_buffers_lister)},
    {0, 69, 2, "exit_4coder", 11, LINK_PROCS(exit_4coder)},
    {0, 55326, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55327, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55328, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55329, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55330, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55331, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55332, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55333, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55334, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55335, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55336, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55337, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55338, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55339, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55340, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55341, 0, "project_fkey_command", 20, LINK_PROCS(project_fkey_command)},
    {0, 55321, 0, "mouse_wheel_scroll", 18, LINK_PROCS(mouse_wheel_scroll)},
    {0, 55321, 1, "mouse_wheel_change_face_size", 28, LINK_PROCS(mouse_wheel_change_face_size)},
};
static Meta_Key_Bind fcoder_binds_for_default_mapid_file[78] = {
    {1, 0, 0, "write_text_input", 15, LINK_PROCS(write_text_input)},
    {0, 55317, 0, "click_set_cursor_and_mark", 25, LINK_PROCS(click_set_cursor_and_mark)},
    {0, 55324, 0, "click_set_cursor_and_mark", 25, LINK_PROCS(click_set_cursor_and_mark)},
    {0, 55319, 0, "click_set_cursor", 16, LINK_PROCS(click_set_cursor)},
    {0, 55322, 0, "click_set_cursor_if_lbutton", 27, LINK_PROCS(click_set_cursor_if_lbutton)},
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
    {0, 55297, 1, "move_up_to_blank_line_skip_whitespace", 37, LINK_PROCS(move_up_to_blank_line_skip_whitespace)},
    {0, 55298, 1, "move_down_to_blank_line_end", 27, LINK_PROCS(move_down_to_blank_line_end)},
    {0, 55299, 1, "move_left_whitespace_boundary", 29, LINK_PROCS(move_left_whitespace_boundary)},
    {0, 55300, 1, "move_right_whitespace_boundary", 30, LINK_PROCS(move_right_whitespace_boundary)},
    {0, 55297, 9, "move_up_to_blank_line_skip_whitespace", 37, LINK_PROCS(move_up_to_blank_line_skip_whitespace)},
    {0, 55298, 9, "move_down_to_blank_line_end", 27, LINK_PROCS(move_down_to_blank_line_end)},
    {0, 55299, 9, "move_left_whitespace_boundary", 29, LINK_PROCS(move_left_whitespace_boundary)},
    {0, 55300, 9, "move_right_whitespace_boundary", 30, LINK_PROCS(move_right_whitespace_boundary)},
    {0, 55297, 2, "move_line_up", 12, LINK_PROCS(move_line_up)},
    {0, 55298, 2, "move_line_down", 14, LINK_PROCS(move_line_down)},
    {0, 55296, 1, "backspace_alpha_numeric_boundary", 32, LINK_PROCS(backspace_alpha_numeric_boundary)},
    {0, 55301, 1, "delete_alpha_numeric_boundary", 29, LINK_PROCS(delete_alpha_numeric_boundary)},
    {0, 55296, 2, "snipe_backward_whitespace_or_token_boundary", 43, LINK_PROCS(snipe_backward_whitespace_or_token_boundary)},
    {0, 55301, 2, "snipe_forward_whitespace_or_token_boundary", 42, LINK_PROCS(snipe_forward_whitespace_or_token_boundary)},
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
    {0, 10, 0, "newline_or_goto_position", 24, LINK_PROCS(if_read_only_goto_position)},
    {0, 10, 8, "newline_or_goto_position_same_panel", 35, LINK_PROCS(if_read_only_goto_position_same_panel)},
    {0, 62, 1, "view_jump_list_with_lister", 26, LINK_PROCS(view_jump_list_with_lister)},
    {0, 32, 8, "write_text_input", 15, LINK_PROCS(write_text_input)},
};
static Meta_Key_Bind fcoder_binds_for_default_default_code_map[33] = {
    {0, 55299, 1, "move_left_alpha_numeric_or_camel_boundary", 41, LINK_PROCS(move_left_alpha_numeric_or_camel_boundary)},
    {0, 55300, 1, "move_right_alpha_numeric_or_camel_boundary", 42, LINK_PROCS(move_right_alpha_numeric_or_camel_boundary)},
    {0, 55299, 2, "move_left_alpha_numeric_boundary", 32, LINK_PROCS(move_left_alpha_numeric_boundary)},
    {0, 55300, 2, "move_right_alpha_numeric_boundary", 33, LINK_PROCS(move_right_alpha_numeric_boundary)},
    {0, 10, 0, "write_and_auto_tab", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 10, 8, "write_and_auto_tab", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 125, 0, "write_text_and_auto_indent", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 41, 0, "write_text_and_auto_indent", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 93, 0, "write_text_and_auto_indent", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 59, 0, "write_text_and_auto_indent", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 35, 0, "write_text_and_auto_indent", 18, LINK_PROCS(write_text_and_auto_indent)},
    {0, 59, 1, "comment_line_toggle", 19, LINK_PROCS(comment_line_toggle)},
    {0, 9, 0, "word_complete", 13, LINK_PROCS(word_complete)},
    {0, 9, 1, "auto_tab_range", 14, LINK_PROCS(auto_indent_range)},
    {0, 9, 8, "auto_tab_line_at_cursor", 23, LINK_PROCS(auto_indent_line_at_cursor)},
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
    {0, 105, 2, "if0_off", 7, LINK_PROCS(if0_off)},
    {0, 49, 2, "open_file_in_quotes", 19, LINK_PROCS(open_file_in_quotes)},
    {0, 50, 2, "open_matching_file_cpp", 22, LINK_PROCS(open_matching_file_cpp)},
    {0, 48, 1, "write_zero_struct", 17, LINK_PROCS(write_zero_struct)},
};
static Meta_Key_Bind fcoder_binds_for_default_default_lister_ui_map[23] = {
    {1, 0, 0, "lister__write_character", 23, LINK_PROCS(lister__write_character)},
    {0, 55307, 0, "lister__quit", 12, LINK_PROCS(lister__quit)},
    {0, 10, 0, "lister__activate", 16, LINK_PROCS(lister__activate)},
    {0, 9, 0, "lister__activate", 16, LINK_PROCS(lister__activate)},
    {0, 55296, 0, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 1, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 2, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 4, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 3, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 6, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 5, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55296, 7, "lister__backspace_text_field", 28, LINK_PROCS(lister__backspace_text_field)},
    {0, 55297, 0, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
    {0, 107, 2, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
    {0, 55305, 0, "lister__move_up", 15, LINK_PROCS(lister__move_up)},
    {0, 55298, 0, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
    {0, 106, 2, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
    {0, 55306, 0, "lister__move_down", 17, LINK_PROCS(lister__move_down)},
    {0, 55321, 0, "lister__wheel_scroll", 20, LINK_PROCS(lister__wheel_scroll)},
    {0, 55317, 0, "lister__mouse_press", 19, LINK_PROCS(lister__mouse_press)},
    {0, 55319, 0, "lister__mouse_release", 21, LINK_PROCS(lister__mouse_release)},
    {0, 55322, 0, "lister__repaint", 15, LINK_PROCS(lister__repaint)},
    {0, 55323, 0, "lister__repaint", 15, LINK_PROCS(lister__repaint)},
};
static Meta_Sub_Map fcoder_submaps_for_default[4] = {
    {"mapid_global", 12, "The following bindings apply in all situations.", 47, 0, 0, fcoder_binds_for_default_mapid_global, 43},
    {"mapid_file", 10, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.", 145, 0, 0, fcoder_binds_for_default_mapid_file, 78},
    {"default_code_map", 16, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.", 94, "mapid_file", 10, fcoder_binds_for_default_default_code_map, 33},
    {"default_lister_ui_map", 21, "These commands apply in 'lister mode' such as when you open a file.", 67, 0, 0, fcoder_binds_for_default_default_lister_ui_map, 23},
};
static Meta_Mapping fcoder_meta_maps[2] = {
    {"default", 7, "The default 4coder bindings - typically good for Windows and Linux", 66, fcoder_submaps_for_default, 4, LINK_PROCS(fill_keys_default)},
};
