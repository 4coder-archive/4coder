/*
 * Example use of customization API
 */

#include "4coder_custom.h"

// NOTE(allen): All this helper stuff is here to make the new API
// work a lot like the old API
struct Bind_Helper{
    Binding_Unit *cursor, *start, *end;
    Binding_Unit *header, *map;
    int write_total;
    int error;
};

#define BH_ERR_NONE 0
#define BH_ERR_MISSING_END_MAP 1
#define BH_ERR_MISSING_BEGIN_MAP 2

inline Binding_Unit*
write_unit(Bind_Helper *helper, Binding_Unit unit){
    Binding_Unit *p = 0;
    helper->write_total += sizeof(Binding_Unit);
    if (helper->error == 0 && helper->cursor != helper->end){
        p = helper->cursor++;
        *p = unit;
    }
    return p;
}

inline Bind_Helper
begin_bind_helper(void *data, int size){
    Bind_Helper result;
    
    result.header = 0;
    result.map = 0;
    result.write_total = 0;
    result.error = 0;
    
    result.cursor = (Binding_Unit*)data;
    result.start = result.cursor;
    result.end = result.start + size / sizeof(Binding_Unit);
    
    Binding_Unit unit;
    unit.type = UNIT_HEADER;
    unit.header.total_size = sizeof(Binding_Unit);
    result.header = write_unit(&result, unit);
    
    return result;
}

inline void
begin_map(Bind_Helper *helper, int mapid){
    if (helper->map != 0 && helper->error == 0) helper->error = BH_ERR_MISSING_END_MAP;
    
    Binding_Unit unit;
    unit.type = UNIT_MAP_BEGIN;
    unit.map_begin.mapid = mapid;
    helper->map = write_unit(helper, unit);
}

inline void
end_map(Bind_Helper *helper){
    if (helper->map == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN_MAP;
    
    helper->map = 0;
}

inline void
bind(Bind_Helper *helper, short code, unsigned char modifiers, int cmdid){
    if (helper->map == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN_MAP;
    
    Binding_Unit unit;
    unit.type = UNIT_BINDING;
    unit.binding.command_id = cmdid;
    unit.binding.code = code;
    unit.binding.modifiers = modifiers;
    
    write_unit(helper, unit);
}

inline void
bind_me(Bind_Helper *helper, short code, unsigned char modifiers, Custom_Command_Function *func){
    if (helper->map == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN_MAP;
    
    Binding_Unit unit;
    unit.type = UNIT_CALLBACK;
    unit.callback.func = func;
    unit.callback.code = code;
    unit.callback.modifiers = modifiers;
    
    write_unit(helper, unit);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, int cmdid){
    bind(helper, 0, 0, cmdid);
}

inline void
bind_me_vanilla_keys(Bind_Helper *helper, Custom_Command_Function *func){
    bind_me(helper, 0, 0, func);
}

inline void
end_bind_helper(Bind_Helper *helper){
    if (helper->header){
        helper->header->header.total_size = (int)(helper->cursor - helper->start);
        helper->header->header.error = helper->error;
    }
}

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

extern "C" GET_BINDING_DATA(get_bindings){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
    begin_map(context, MAPID_GLOBAL);
    
    // NOTE(allen): Here put the contents of your set_global_bindings
    bind(context, 'p', MDFR_CTRL, cmdid_open_panel_vsplit);
    bind(context, '-', MDFR_CTRL, cmdid_open_panel_hsplit);
    bind(context, 'P', MDFR_CTRL, cmdid_close_panel);
    bind(context, 'n', MDFR_CTRL, cmdid_interactive_new);
    bind(context, 'o', MDFR_CTRL, cmdid_interactive_open);
    bind(context, ',', MDFR_CTRL, cmdid_change_active_panel);
    bind(context, 'k', MDFR_CTRL, cmdid_interactive_kill_file);
    bind(context, 'i', MDFR_CTRL, cmdid_interactive_switch_file);
    bind(context, 'c', MDFR_ALT, cmdid_open_color_tweaker);
    bind(context, 'x', MDFR_ALT, cmdid_open_menu);
    bind_me(context, 'o', MDFR_ALT, open_in_other);
    
    end_map(context);
    
    
    begin_map(context, MAPID_FILE);
    
    // NOTE(allen): This is a new concept in the API. Binding this can be thought of as binding
    // all combos which have an ascii code (shifted or not) and unmodified by CTRL or ALT.
    // As of now, if this is used it cannot be overriden for particular combos; this overrides
    // normal bindings.
    bind_vanilla_keys(context, cmdid_write_character);
    
    // NOTE(allen): Here put the contents of your set_file_bindings
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
    
    bind(context, codes->right, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_right);
    bind(context, codes->left, MDFR_CTRL, cmdid_seek_alphanumeric_or_camel_left);
    bind(context, codes->up, MDFR_CTRL, cmdid_seek_whitespace_up);
    bind(context, codes->down, MDFR_CTRL, cmdid_seek_whitespace_down);
    
    bind(context, ' ', MDFR_CTRL, cmdid_set_mark);
    bind(context, 'm', MDFR_CTRL, cmdid_cursor_mark_swap);
    bind(context, 'c', MDFR_CTRL, cmdid_copy);
    bind(context, 'x', MDFR_CTRL, cmdid_cut);
    bind(context, 'v', MDFR_CTRL, cmdid_paste);
    bind(context, 'V', MDFR_CTRL, cmdid_paste_next);
    bind(context, 'd', MDFR_CTRL, cmdid_delete_chunk);
    bind(context, 'l', MDFR_CTRL, cmdid_toggle_line_wrap);
    bind(context, 'L', MDFR_CTRL, cmdid_toggle_endline_mode);
    bind(context, 'u', MDFR_CTRL, cmdid_to_uppercase);
    bind(context, 'j', MDFR_CTRL, cmdid_to_lowercase);
    bind(context, '?', MDFR_CTRL, cmdid_toggle_show_whitespace);
    bind(context, '`', MDFR_CTRL, cmdid_clean_line);
    bind(context, '~', MDFR_CTRL, cmdid_clean_all_lines);
    bind(context, '1', MDFR_CTRL, cmdid_eol_dosify);
    bind(context, '!', MDFR_CTRL, cmdid_eol_nixify);
    bind(context, 'f', MDFR_CTRL, cmdid_search);
    bind(context, 'r', MDFR_CTRL, cmdid_rsearch);
    bind(context, 'g', MDFR_CTRL, cmdid_goto_line);
    
    bind(context, '\t', MDFR_CTRL, cmdid_auto_tab);
    
    bind(context, 'K', MDFR_CTRL, cmdid_kill_file);
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


