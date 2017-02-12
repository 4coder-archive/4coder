/*
4coder_windows_bindings.cpp - Supplies bindings very similar to the bindings in modern style editors like notepad.

TYPE: 'build-target'
*/

// TOP

#if !defined(FCODER_WINDOWS_BINDINGS_CPP)
#define FCODER_WINDOWS_BINDINGS_CPP

#include "4coder_default_include.cpp"

void
default_keys(Bind_Helper *context){
    begin_map(context, mapid_global);
    
    bind(context, 't', MDFR_CTRL, open_panel_vsplit);
    bind(context, 'T', MDFR_CTRL, open_panel_hsplit);
    bind(context, 'w', MDFR_CTRL, close_panel);
    bind(context, '\t', MDFR_CTRL, change_active_panel);
    bind(context, '\t', MDFR_CTRL | MDFR_SHIFT, change_active_panel_backwards);
    
    bind(context, 'n', MDFR_CTRL, interactive_new);
    bind(context, 'n', MDFR_ALT, new_in_other);
    bind(context, 'o', MDFR_CTRL, interactive_open);
    bind(context, 'o', MDFR_ALT, open_in_other);
    bind(context, 'k', MDFR_CTRL, interactive_kill_buffer);
    bind(context, 'i', MDFR_CTRL, interactive_switch_buffer);
    bind(context, 'S', MDFR_CTRL, save_all_dirty_buffers);
    
    bind(context, 'c', MDFR_ALT, open_color_tweaker);
    bind(context, 'd', MDFR_ALT, open_debug);
    
    bind(context, '\\', MDFR_CTRL, change_to_build_panel);
    bind(context, '|', MDFR_CTRL, close_build_panel);
    bind(context, key_down, MDFR_ALT, goto_next_error);
    bind(context, key_up, MDFR_ALT, goto_prev_error);
    bind(context, key_up, MDFR_ALT | MDFR_SHIFT, goto_first_error);
    
    bind(context, 'z', MDFR_ALT, execute_any_cli);
    bind(context, 'Z', MDFR_ALT, execute_previous_cli);
    bind(context, 'x', MDFR_ALT, execute_arbitrary_command);
    bind(context, 's', MDFR_ALT, show_scrollbar);
    bind(context, 'w', MDFR_ALT, hide_scrollbar);
    
    bind(context, '@', MDFR_ALT, toggle_mouse);
    bind(context, key_page_up, MDFR_CTRL, toggle_fullscreen);
    bind(context, 'W', MDFR_CTRL, exit_4coder);
    bind(context, key_f4, MDFR_ALT, exit_4coder);
    
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
    
    
    begin_map(context, default_code_map);
    inherit_map(context, mapid_file);
    
    bind(context, key_right, MDFR_CTRL, seek_alphanumeric_or_camel_right);
    bind(context, key_left, MDFR_CTRL, seek_alphanumeric_or_camel_left);
    bind(context, key_right, MDFR_ALT, seek_whitespace_right);
    bind(context, key_left, MDFR_ALT, seek_whitespace_left);
    
    bind(context, '\n', MDFR_NONE, write_and_auto_tab);
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, ')', MDFR_NONE, write_and_auto_tab);
    bind(context, ']', MDFR_NONE, write_and_auto_tab);
    bind(context, ';', MDFR_NONE, write_and_auto_tab);
    bind(context, '#', MDFR_NONE, write_and_auto_tab);
    
    bind(context, ' ', MDFR_ALT, word_complete);
    bind(context, '\t', MDFR_NONE, auto_tab_line_at_cursor);
    bind(context, '\t', MDFR_SHIFT, auto_tab_range);
    
    bind(context, 't', MDFR_ALT, write_todo);
    bind(context, 'y', MDFR_ALT, write_note);
    bind(context, 'r', MDFR_ALT, write_block);
    bind(context, '[', MDFR_CTRL, open_long_braces);
    bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
    bind(context, '}', MDFR_CTRL, open_long_braces_break);
    bind(context, 'i', MDFR_ALT, if0_off);
    bind(context, '1', MDFR_ALT, open_file_in_quotes);
    bind(context, '2', MDFR_ALT, open_matching_file_cpp);
    bind(context, '0', MDFR_CTRL, write_zero_struct);
    bind(context, 'I', MDFR_CTRL, list_all_functions_current_buffer);
    
    end_map(context);
    
    
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
    
    bind(context, key_right, MDFR_CTRL, seek_alphanumeric_or_camel_right);
    bind(context, key_left, MDFR_CTRL, seek_alphanumeric_or_camel_left);
    bind(context, key_right, MDFR_ALT, seek_whitespace_right);
    bind(context, key_left, MDFR_ALT, seek_whitespace_left);
    bind(context, key_up, MDFR_CTRL, seek_whitespace_up_end_line);
    bind(context, key_down, MDFR_CTRL, seek_whitespace_down_end_line);
    
    bind(context, key_back, MDFR_CTRL, backspace_word);
    bind(context, key_del, MDFR_CTRL, delete_word);
    bind(context, key_back, MDFR_ALT, snipe_token_or_word);
    
    bind(context, ' ', MDFR_CTRL, set_mark);
    bind(context, 'a', MDFR_CTRL, select_all);
    bind(context, 'c', MDFR_CTRL, copy);
    bind(context, 'd', MDFR_CTRL, duplicate_line);
    bind(context, 'f', MDFR_ALT, list_all_locations);
    bind(context, 'f', MDFR_CTRL, list_all_substring_locations_case_insensitive);
    bind(context, 'F', MDFR_CTRL, list_all_locations_of_identifier);
    bind(context, 'F', MDFR_ALT, list_all_locations_of_identifier_case_insensitive);
    bind(context, 'e', MDFR_CTRL, center_view);
    bind(context, 'E', MDFR_CTRL, left_adjust_view);
    bind(context, 'g', MDFR_CTRL, goto_line);
    bind(context, 'h', MDFR_CTRL, query_replace);
    bind(context, 'H', MDFR_CTRL, replace_in_range);
    bind(context, 'i', MDFR_CTRL, search);
    bind(context, 'I', MDFR_CTRL, reverse_search);
    bind(context, 'K', MDFR_CTRL, kill_buffer);
    bind(context, 'l', MDFR_CTRL, delete_line);
    bind(context, 'L', MDFR_ALT, toggle_line_wrap);
    bind(context, 'O', MDFR_CTRL, reopen);
    bind(context, 's', MDFR_CTRL, save);
    bind(context, 'S', MDFR_ALT, save_as);
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
    bind(context, '\n', MDFR_SHIFT, newline_or_goto_position);
    bind(context, ' ', MDFR_SHIFT, write_character);
    
    end_map(context);
}

#ifndef NO_BINDING
extern "C" int32_t
get_bindings(void *data, int32_t size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_all_default_hooks(context);
    default_keys(context);
    
    int32_t result = end_bind_helper(context);
    return(result);
}
#endif //NO_BINDING

#endif

// BOTTOM

