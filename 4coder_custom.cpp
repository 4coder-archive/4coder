/*
 * Example use of customization API
 */

// NOTE(allen): NEW THINGS TO LOOK FOR:
// MAPID_USER_CUSTOM - define maps other than the built in GLOBAL/FILE maps
// inherit_map
// 
// get_settings

#include "4coder_custom.h"
#include "4coder_helper.h"

#define exec_command app.exec_command
#define fulfill_interaction app.fulfill_interaction

extern "C" START_HOOK_SIG(start_hook){
    exec_command(cmd_context, cmdid_open_panel_vsplit);
    exec_command(cmd_context, cmdid_change_active_panel);
}

CUSTOM_COMMAND_SIG(open_in_other){
    exec_command(cmd_context, cmdid_change_active_panel);
    exec_command(cmd_context, cmdid_interactive_open);
}

extern "C" GET_BINDING_DATA(get_binding){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
    begin_settings_group(context);
    
    use_when(context, when_default, 1);
    
    set(context, set_lex_as_cpp_file, 0);
    set(context, set_wrap_lines, 1);
    set(context, set_key_mapid, MAPID_FILE);
    
    // NOTE(allen): options include EOL_USE_CRLF, EOL_USE_CR_USE_LF, EOL_SHOW_CR_USE_LF
    // EOL_USE_CRLF - treats a crlf and lf as newline markers, renders lone cr as special character "\r"
    // EOL_USE_CR_USE_LF - treats both as separate newline markers
    // EOL_SHOW_CR_USE_LF - treats lf as newline marker, renders cr as special character "\r"
    set(context, set_end_line_mode, EOL_USE_CRLF);
    
    end_group(context);
    
    
    begin_settings_group(context);
    
    use_when(context, when_extension, "cpp");
    use_when(context, when_extension, "hpp");
    use_when(context, when_extension, "c");
    use_when(context, when_extension, "h");
    
    set(context, set_lex_as_cpp_file, 1);
    set(context, set_key_mapid, MAPID_USER_CUSTOM + 0);
    
    begin_map(context, MAPID_GLOBAL);
    
    bind(context, 'p', MDFR_CTRL, cmdid_open_panel_vsplit);
    bind(context, '-', MDFR_CTRL, cmdid_open_panel_hsplit);
    bind(context, 'P', MDFR_CTRL, cmdid_close_panel);
    bind(context, 'n', MDFR_CTRL, cmdid_interactive_new);
    bind(context, 'o', MDFR_CTRL, cmdid_interactive_open);
    bind(context, ',', MDFR_CTRL, cmdid_change_active_panel);
    bind(context, 'k', MDFR_CTRL, cmdid_interactive_kill_buffer);
    bind(context, 'i', MDFR_CTRL, cmdid_interactive_switch_buffer);
    bind(context, 'c', MDFR_ALT, cmdid_open_color_tweaker);
    bind(context, 'x', MDFR_ALT, cmdid_open_menu);
    bind_me(context, 'o', MDFR_ALT, open_in_other);
    
    end_map(context);

    
    begin_map(context, MAPID_USER_CUSTOM + 0);

    // NOTE(allen): Set this map (MAPID_USER_CUSTOM + 0) to
    // inherit from MAPID_FILE.  When searching if a key is bound
    // in this map, if it is not found here it will then search MAPID_FILE.
    //
    // If this is not set, it defaults to MAPID_GLOBAL.
    inherit_map(context, MAPID_FILE);
    
    bind(context, codes->right, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_right);
    bind(context, codes->left, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_left);
    
    // NOTE(allen): Not currently functional
    bind(context, '\t', MDFR_CTRL, cmdid_auto_tab);
    
    end_map(context);

    
    begin_map(context, MAPID_FILE);
    
    // NOTE(allen): Binding this essentially binds all key combos that
    // would normally insert a character into a buffer.
    // Or apply this rule (which always works): if the code for the key
    // is not in the codes struct, it is a vanilla key.
    // It is possible to override this binding for individual keys.
    bind_vanilla_keys(context, cmdid_write_character);
    
    bind(context, codes->left, MDFR_NONE, cmdid_move_left);
    bind(context, codes->right, MDFR_NONE, cmdid_move_right);
    bind(context, codes->del, MDFR_NONE, cmdid_delete);
    bind(context, codes->back, MDFR_NONE, cmdid_backspace);
    bind(context, codes->up, MDFR_NONE, cmdid_move_up);
    bind(context, codes->down, MDFR_NONE, cmdid_move_down);
    bind(context, codes->end, MDFR_NONE, cmdid_seek_end_of_line);
    bind(context, codes->home, MDFR_NONE, cmdid_seek_beginning_of_line);
    bind(context, codes->page_up, MDFR_NONE, cmdid_page_up);
    bind(context, codes->page_down, MDFR_NONE, cmdid_page_down);
    
    bind(context, codes->right, MDFR_CTRL, cmdid_seek_whitespace_right);
    bind(context, codes->left, MDFR_CTRL, cmdid_seek_whitespace_left);
    bind(context, codes->up, MDFR_CTRL, cmdid_seek_whitespace_up);
    bind(context, codes->down, MDFR_CTRL, cmdid_seek_whitespace_down);
    
    bind(context, ' ', MDFR_CTRL, cmdid_set_mark);
    bind(context, 'm', MDFR_CTRL, cmdid_cursor_mark_swap);
    bind(context, 'c', MDFR_CTRL, cmdid_copy);
    bind(context, 'x', MDFR_CTRL, cmdid_cut);
    bind(context, 'v', MDFR_CTRL, cmdid_paste);
    bind(context, 'V', MDFR_CTRL, cmdid_paste_next);
    bind(context, 'z', MDFR_CTRL, cmdid_undo);
    bind(context, 'y', MDFR_CTRL, cmdid_redo);
    bind(context, 'd', MDFR_CTRL, cmdid_delete_chunk);
    bind(context, 'l', MDFR_CTRL, cmdid_toggle_line_wrap);
    bind(context, 'L', MDFR_CTRL, cmdid_toggle_endline_mode);
    bind(context, 'u', MDFR_CTRL, cmdid_to_uppercase);
    bind(context, 'j', MDFR_CTRL, cmdid_to_lowercase);
    bind(context, '?', MDFR_CTRL, cmdid_toggle_show_whitespace);

    // NOTE(allen): These whitespace manipulators are not currently functional
    bind(context, '`', MDFR_CTRL, cmdid_clean_line);
    bind(context, '~', MDFR_CTRL, cmdid_clean_all_lines);
    bind(context, '1', MDFR_CTRL, cmdid_eol_dosify);
    bind(context, '!', MDFR_CTRL, cmdid_eol_nixify);
    
    bind(context, 'f', MDFR_CTRL, cmdid_search);
    bind(context, 'r', MDFR_CTRL, cmdid_rsearch);
    bind(context, 'g', MDFR_CTRL, cmdid_goto_line);
    
    bind(context, 'K', MDFR_CTRL, cmdid_kill_buffer);
    bind(context, 'O', MDFR_CTRL, cmdid_reopen);
    bind(context, 'w', MDFR_CTRL, cmdid_interactive_save_as);
    bind(context, 's', MDFR_CTRL, cmdid_save);
    
    end_map(context);
    end_bind_helper(context);
    
    return context->write_total;
}

inline void
strset_(char *dst, char *src){
    do{
        *dst++ = *src++;
    }while (*src);
}

#define strset(d,s) if (sizeof(s) <= sizeof(d)) strset_(d,s)

extern "C" SET_EXTRA_FONT_SIG(set_extra_font){
    strset(font_out->file_name, "liberation-mono.ttf");
    strset(font_out->font_name, "BIG");
    font_out->size = 25;
}


