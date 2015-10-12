/*
 * This is an example of how vim-keys might start
 * to work in 4coder, through the customization API.
 */

#include "4coder_custom.h"
#include "4coder_helper.h"

#define exec_command app.exec_command
#define fulfill_interaction app.fulfill_interaction

extern "C" START_HOOK_SIG(start_hook){
    exec_command(cmd_context, cmdid_open_panel_vsplit);
    exec_command(cmd_context, cmdid_change_active_panel);
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
    inherit_map(context, MAPID_FILE);
    end_map(context);

    int sub_id;
    begin_map(context, MAPID_FILE);
    
    sub_id = begin_sub_map(context);
    {
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
    }
    end_sub_map(context);
    
    bind_params(context, 'i', MDFR_NONE, cmdid_use_sub_map);
    fill_param(context, "sub_id", sub_id);
    end_params(context);
    
    bind_multi(context, 'a', MDFR_NONE);
    {
        bind(context, 'a', MDFR_NONE, cmdid_move_left);
        bind_params(context, 'a', MDFR_NONE, cmdid_use_sub_map);
        fill_param(context, "sub_id", sub_id);
        end_params(context);
    }
    end_multi(context);
    
    bind(context, 'b', cmdid_seek_alphanumeric_left);
    bind(context, 'w', cmdid_seek_alphanumeric_right);
    bind(context, 'e', cmdid_seek_white_or_token_right);

    // ???? this seems a bit off
    bind_compound(context, 'g');
    {
        bind(context, 'e', cmdid_seek_white_or_token_left);
    }
    end_compound(context);
    
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


