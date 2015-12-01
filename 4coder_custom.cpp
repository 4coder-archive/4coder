/*
 * Example use of customization API
 */

#include "4coder_custom.h"
#include "4coder_helper.h"

#define exec_command_keep_stack app.exec_command_keep_stack
#define clear_parameters app.clear_parameters
#define get_active_buffer app.get_active_buffer

#define exec_command(cmd_context, id)           \
    exec_command_keep_stack(cmd_context, id);   \
    clear_parameters(cmd_context)
#define push_parameter(cmd_context, ...) push_parameter_helper(cmd_context, app, __VA_ARGS__)
#define push_memory(cmd_context, len) app.push_memory(cmd_context, len)

#define literal(s) s, (sizeof(s)-1)

// NOTE(allen|a3.1): All of your custom ids should be >= mapid_user_custom.
// I recommend enumerating your own map ids as shown here.
enum My_Maps{
    my_code_map = mapid_user_custom,
};

HOOK_SIG(my_start){
    exec_command(cmd_context, cmdid_open_panel_vsplit);
    exec_command(cmd_context, cmdid_change_active_panel);
}

char *get_extension(const char *filename, int len, int *extension_len){
    char *c = (char*)(filename + len - 1);
    char *end = c;
    while (*c != '.' && c > filename) --c;
    *extension_len = (int)(end - c);
    return c+1;
}

bool str_match(const char *a, int len_a, const char *b, int len_b){
    bool result = 0;
    if (len_a == len_b){
        char *end = (char*)(a + len_a);
        while (a < end && *a == *b){
            ++a; ++b;
        }
        if (a == end) result = 1;
    }
    return result;
}

HOOK_SIG(my_file_settings){
    Buffer_Summary buffer = get_active_buffer(cmd_context);
    
    int treat_as_code = 0;
    
    // NOTE(allen|a3.1): This checks buffer.file_name just in case get_active_buffer returns back
    // a null buffer (where every member is 0).
    if (buffer.file_name && buffer.size < (16 << 20)){
        int extension_len;
        char *extension = get_extension(buffer.file_name, buffer.file_name_len, &extension_len);
        if (str_match(extension, extension_len, literal("cpp"))) treat_as_code = 1;
        else if (str_match(extension, extension_len, literal("h"))) treat_as_code = 1;
        else if (str_match(extension, extension_len, literal("c"))) treat_as_code = 1;
        else if (str_match(extension, extension_len, literal("hpp"))) treat_as_code = 1;
    }
    
    push_parameter(cmd_context, par_lex_as_cpp_file, treat_as_code);
    push_parameter(cmd_context, par_wrap_lines, !treat_as_code);
    push_parameter(cmd_context, par_key_mapid, (treat_as_code)?(my_code_map):(mapid_file));
    exec_command(cmd_context, cmdid_set_settings);
}

CUSTOM_COMMAND_SIG(open_in_other){
    exec_command(cmd_context, cmdid_change_active_panel);
    exec_command(cmd_context, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(open_my_files){
    // NOTE(allen|a3.1): The command cmdid_interactive_open can now open
    // a file specified on the parameter stack.  If the file does not
    // exist cmdid_interactive_open behaves as usual.
    push_parameter(cmd_context, par_name, literal("w:/4ed/data/test/basic.cpp"));
    exec_command(cmd_context, cmdid_interactive_open);
    
    exec_command(cmd_context, cmdid_change_active_panel);
    
    char my_file[256];
    int my_file_len;
    
    my_file_len = sizeof("w:/4ed/data/test/basic.txt") - 1;
    for (int i = 0; i < my_file_len; ++i){
        my_file[i] = ("w:/4ed/data/test/basic.txt")[i];
    }
    
    // NOTE(allen|a3.1): null terminators are not needed for strings.
    push_parameter(cmd_context, par_name, my_file, my_file_len);
    exec_command(cmd_context, cmdid_interactive_open);
    
    exec_command(cmd_context, cmdid_change_active_panel);
}

CUSTOM_COMMAND_SIG(write_and_auto_tab){
    exec_command(cmd_context, cmdid_write_character);
    exec_command(cmd_context, cmdid_auto_tab_line_at_cursor);
}

extern "C" GET_BINDING_DATA(get_bindings){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
    // NOTE(allen|a3.1): Right now hooks have no loyalties to maps, all hooks are
    // global and once set they always apply, regardless of what map is active.
    set_hook(context, hook_start, my_start);
    set_hook(context, hook_open_file, my_file_settings);
    
    begin_map(context, mapid_global);
    
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
    // NOTE(allen|a3.1): Go look at open_my_files, that's the only point of this being here,
    // it won't actually be useful for you.
    bind_me(context, 'M', MDFR_ALT | MDFR_CTRL, open_my_files);
    
    end_map(context);
    
    
    begin_map(context, my_code_map);
    
    // NOTE(allen|a3.1): Set this map (my_code_map == mapid_user_custom) to
    // inherit from mapid_file.  When searching if a key is bound
    // in this map, if it is not found here it will then search mapid_file.
    //
    // If this is not set, it defaults to mapid_global.
    inherit_map(context, mapid_file);
    
    // NOTE(allen|a3.1): Children can override parent's bindings.
    bind(context, codes->right, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_right);
    bind(context, codes->left, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_left);
    
    // NOTE(allen|a3.2): Specific keys can override vanilla keys,
    // and write character writes whichever character corresponds
    // to the key that triggered the command.
    bind_me(context, '\n', MDFR_NONE, write_and_auto_tab);
    bind_me(context, '}', MDFR_NONE, write_and_auto_tab);
    bind_me(context, ')', MDFR_NONE, write_and_auto_tab);
    bind_me(context, ']', MDFR_NONE, write_and_auto_tab);
    bind_me(context, ';', MDFR_NONE, write_and_auto_tab);
    
    bind(context, '\t', MDFR_NONE, cmdid_auto_tab_line_at_cursor);
    bind(context, '\t', MDFR_CTRL, cmdid_auto_tab_range);
    bind(context, '\t', MDFR_CTRL | MDFR_SHIFT, cmdid_write_character);
    
    end_map(context);
    
    
    begin_map(context, mapid_file);
    
    // NOTE(allen|a3.1): Binding this essentially binds all key combos that
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
    bind(context, 'Z', MDFR_CTRL, cmdid_timeline_scrub);
    bind(context, 'z', MDFR_CTRL, cmdid_undo);
    bind(context, 'y', MDFR_CTRL, cmdid_redo);
    bind(context, codes->left, MDFR_ALT, cmdid_increase_rewind_speed);
    bind(context, codes->right, MDFR_ALT, cmdid_increase_fastforward_speed);
    bind(context, codes->down, MDFR_ALT, cmdid_stop_rewind_fastforward);
    bind(context, 'h', MDFR_CTRL, cmdid_history_backward);
    bind(context, 'H', MDFR_CTRL, cmdid_history_forward);
    bind(context, 'd', MDFR_CTRL, cmdid_delete_range);
    bind(context, 'l', MDFR_CTRL, cmdid_toggle_line_wrap);
    bind(context, 'L', MDFR_CTRL, cmdid_toggle_endline_mode);
    bind(context, 'u', MDFR_CTRL, cmdid_to_uppercase);
    bind(context, 'j', MDFR_CTRL, cmdid_to_lowercase);
    bind(context, '?', MDFR_CTRL, cmdid_toggle_show_whitespace);
    
    bind(context, '~', MDFR_CTRL, cmdid_clean_all_lines);
    // NOTE(allen|a3.2): These now only set the mode of the file for writing to disk
    // they do no longer effect the internal representation.
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


