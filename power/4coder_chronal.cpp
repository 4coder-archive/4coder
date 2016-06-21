
// TOP
#include "4coder_default.cpp"

//#include "chr_winutils.h"

#ifndef literal
#define literal(s) (s), (sizeof(s)-1)
#endif

#define rgb_color(r, g, b) (r << 16 + g << 8 + b << 0)
#define hex_color(hex) hex

const int color_margin_normal = 0x341313;
const int color_margin_insert = 0x5a3619;

enum Vim_Maps {
    mapid_normal = mapid_global,
    mapid_insert = 0,
    mapid_replace,
    mapid_visual,

    // There are a bunch of different chord "starters" that result in keys having
    // different behaviors. There's no better way to handle this right now than
    // just explicitly creating maps for each one.

    //TODO(chronister): Chords can be built up, so this can have potentially huge
    //combinatorics... what I *want* is a way to build up an actual stack of commands
    //...

    mapid_chord_delete,
    mapid_chord_yank,
    mapid_chord_g,
};

HOOK_SIG(chronal_init){
    exec_command(app, cmdid_open_panel_vsplit);
    exec_command(app, cmdid_change_active_panel);

    app->change_theme(app, literal("4coder"));
    app->change_font(app, literal("Hack"));

    const int color_bg = 0x15100f;
    const int color_bar = 0x1c1212;
    const int color_bar_hover = 0x261414;
    const int color_bar_active = 0x341313;
    const int color_text = 0x916550;
    const int color_comment = 0x9d5b25;
    const int color_string_literal = 0x9c2d21;
    const int color_num_literals = 0xc56211;
    const int color_keyword = 0xf74402;
    Theme_Color colors[] = {
        { Stag_Back, color_bg },
        { Stag_Margin, color_bar },
        { Stag_Margin_Hover, color_bar_hover },
        { Stag_Margin_Active, color_margin_normal },
        { Stag_Bar, color_bar },
        { Stag_Bar_Active, color_bar_active },
        { Stag_Base, color_text },
        { Stag_Default, color_text },
        { Stag_Comment, color_comment },
        { Stag_Int_Constant, color_num_literals },
        { Stag_Float_Constant, color_num_literals },
        { Stag_Str_Constant, color_string_literal },
        { Stag_Char_Constant, color_string_literal },
        { Stag_Bool_Constant, color_keyword },
        { Stag_Keyword, color_keyword },
        { Stag_Special_Character, color_keyword },
        { Stag_Preproc, color_keyword },
    };

    app->set_theme_colors(app, colors, ArrayCount(colors));

    push_parameter(app, par_key_mapid, mapid_normal);
    exec_command(app, cmdid_set_settings);

    // no meaning for return
    return(0);
}

HOOK_SIG(chronal_file_settings){
    // NOTE(allen|a4): In hooks that want parameters, such as this file
    // created hook.  The file created hook is guaranteed to have only
    // and exactly one buffer parameter.  In normal command callbacks
    // there are no parameter buffers.
    Buffer_Summary buffer = app->get_parameter_buffer(app, 0);
    assert(buffer.exists);

    int treat_as_code = 0;

    if (buffer.file_name && buffer.size < (16 << 20)){
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match(ext, make_lit_string("cpp"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("h"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("c"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("hpp"))) treat_as_code = 1;
    }

    push_parameter(app, par_lex_as_cpp_file, treat_as_code);
    push_parameter(app, par_wrap_lines, !treat_as_code);
    push_parameter(app, par_key_mapid, mapid_normal);
    exec_command(app, cmdid_set_settings);

    // no meaning for return
    return(0);
}

/*                 *
 * Custom commands *
 *                 */



CUSTOM_COMMAND_SIG(do_nothing){
}

CUSTOM_COMMAND_SIG(enter_insert_mode){
    push_parameter(app, par_key_mapid, mapid_insert);
    exec_command(app, cmdid_set_settings);

    Theme_Color colors[] = {
        { Stag_Bar_Active, color_margin_insert },
        { Stag_Margin_Active, color_margin_insert },
    };
    app->set_theme_colors(app, colors, ArrayCount(colors));
}

CUSTOM_COMMAND_SIG(enter_normal_mode){
    push_parameter(app, par_key_mapid, mapid_normal);
    exec_command(app, cmdid_set_settings);

    Theme_Color colors[] = {
        { Stag_Bar_Active, color_margin_normal },
        { Stag_Margin_Active, color_margin_normal },
    };
    app->set_theme_colors(app, colors, ArrayCount(colors));
}

CUSTOM_COMMAND_SIG(seek_forward_word_start){
    View_Summary view;
    view = app->get_active_view(app);
    push_parameter(app, par_flags, BoundryToken);
    exec_command(app, cmdid_seek_right);
    app->refresh_view(app, &view);
}

CUSTOM_COMMAND_SIG(seek_backward_word_start){
    View_Summary view;
    view = app->get_active_view(app);
    push_parameter(app, par_flags, BoundryToken | BoundryWhitespace);
    exec_command(app, cmdid_seek_left);
    app->refresh_view(app, &view);
}

CUSTOM_COMMAND_SIG(seek_forward_word_end){
    View_Summary view;
    view = app->get_active_view(app);
    push_parameter(app, par_flags, BoundryToken | BoundryWhitespace);
    exec_command(app, cmdid_seek_right);
    app->refresh_view(app, &view);
}

CUSTOM_COMMAND_SIG(newline_then_insert_before){
    exec_command(app, cmdid_seek_beginning_of_line);
    write_string(app, make_lit_string("\n"));
    exec_command(app, cmdid_move_left);
    exec_command(app, enter_insert_mode);
}

CUSTOM_COMMAND_SIG(newline_then_insert_after){
    exec_command(app, cmdid_seek_end_of_line);
    write_string(app, make_lit_string("\n"));
    exec_command(app, enter_insert_mode);
}

CUSTOM_COMMAND_SIG(begin_chord_delete){
    push_parameter(app, par_key_mapid, mapid_chord_delete);
    exec_command(app, cmdid_set_settings);
}

CUSTOM_COMMAND_SIG(delete_line){
    View_Summary view;
    Buffer_Summary buffer;
    int pos1, pos2;
    view = app->get_active_view(app);

    exec_command(app, cmdid_seek_beginning_of_line);
    app->refresh_view(app, &view);
    pos1 = view.cursor.pos;

    exec_command(app, cmdid_seek_end_of_line);
    app->refresh_view(app, &view);
    pos2 = view.cursor.pos;
    
    Range range = make_range(pos1, pos2);
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
}

CUSTOM_COMMAND_SIG(delete_word){
    View_Summary view;
    Buffer_Summary buffer;
    int pos1, pos2;
    view = app->get_active_view(app);

    exec_command(app, seek_backward_word_start);
    app->refresh_view(app, &view);
    pos1 = view.cursor.pos;

    exec_command(app, seek_forward_word_end);
    app->refresh_view(app, &view);
    pos2 = view.cursor.pos;
    
    Range range = make_range(pos1, pos2);
    buffer = app->get_buffer(app, view.buffer_id);
    app->buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
    
}

void
chronal_get_bindings(Bind_Helper *context){
    set_hook(context, hook_start, chronal_init);
    set_hook(context, hook_open_file, chronal_file_settings);

    set_scroll_rule(context, smooth_scroll_rule);

    /*                          *
     * SECTION: Vim keybindings *
     *                          */

    /* Normal mode.
     * aka "It's eating all my input, help!" mode.
     * Shortcuts for navigation, entering various modes,
     * dealing with the editor.
     */
    begin_map(context, mapid_normal);

    bind_vanilla_keys(context, do_nothing);

    bind(context, 'w', MDFR_NONE, seek_forward_word_start);
    bind(context, 'e', MDFR_NONE, seek_forward_word_end);
    bind(context, 'b', MDFR_NONE, seek_backward_word_start);
    bind(context, '$', MDFR_NONE, cmdid_seek_end_of_line);
    bind(context, '0', MDFR_NONE, cmdid_seek_beginning_of_line);

    bind(context, 'h', MDFR_NONE, cmdid_move_left);
    bind(context, 'j', MDFR_NONE, cmdid_move_down);
    bind(context, 'k', MDFR_NONE, cmdid_move_up);
    bind(context, 'l', MDFR_NONE, cmdid_move_right);

    bind(context, 'u', MDFR_CTRL, cmdid_page_up);
    bind(context, 'd', MDFR_CTRL, cmdid_page_down);

    bind(context, 'x', MDFR_NONE, cmdid_delete);

    bind(context, 'u', MDFR_NONE, cmdid_undo);
    bind(context, 'r', MDFR_CTRL, cmdid_redo);

    bind(context, '/', MDFR_NONE, search);

    bind(context, 'i', MDFR_NONE, enter_insert_mode);
    bind(context, 'o', MDFR_NONE, newline_then_insert_after);
    bind(context, 'O', MDFR_NONE, newline_then_insert_before);

    bind(context, 'n', MDFR_CTRL, cmdid_word_complete);

    // TEMP (will be replaced later by :statusbar commands)
    bind(context, 'o', MDFR_CTRL, cmdid_interactive_open);
    bind(context, 'c', MDFR_CTRL, cmdid_open_color_tweaker);
    end_map(context);

    /* Insert mode
     * You type and it goes into the buffer. Nice and simple.
     * Escape to exit.
     */
    begin_map(context, mapid_insert);
    inherit_map(context, mapid_nomap);

    bind_vanilla_keys(context, cmdid_write_character);
    bind(context, key_back, MDFR_NONE, cmdid_backspace);

    bind(context, key_esc, MDFR_NONE, enter_normal_mode);

    end_map(context);

#if 1

    /* Chord "modes".
     * They're not really an explicit mode per-say, but the meaning of key presses
     * does change once a chord starts, and is context-dependent.
     * TODO(chronister): I want these to properly build on each other.
     */
    begin_map(context, mapid_chord_delete);
    inherit_map(context, mapid_nomap);

    bind(context, 'd', MDFR_NONE, delete_line);
    bind(context, 'w', MDFR_NONE, delete_word);

    end_map(context);

#endif
}


