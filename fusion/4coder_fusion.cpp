/*
4coder_fusion.cpp - 4coder major mode
*/

// TOP

#include "4coder_default_include.cpp"

////////////////////////////////

// NOTE(allen): Users can declare their own managed IDs here.
CUSTOM_ID(command_map, fusion_map_command);
CUSTOM_ID(command_map, fusion_map_insert);

////////////////////////////////

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

////////////////////////////////

typedef i32 Fusion_Mode;
enum{
    FusionMode_Command,
    FusionMode_Insert,
};

Fusion_Mode fusion_mode = FusionMode_Command;

////////////////////////////////

function void
fusion_set_mode(Fusion_Mode mode){
    local_persist ARGB_Color col_margin = finalize_color(defcolor_margin, 0);
    local_persist ARGB_Color col_margin_hover = finalize_color(defcolor_margin_hover, 0);
    local_persist ARGB_Color col_margin_active = finalize_color(defcolor_margin_active, 0);
    local_persist ARGB_Color col_back = finalize_color(defcolor_back, 0);
    
    fusion_mode = mode;
    switch (mode){
        case FusionMode_Command:
        {
            col_margin = finalize_color(defcolor_margin, 0);
            col_margin_hover = finalize_color(defcolor_margin_hover, 0);
            col_margin_active = finalize_color(defcolor_margin_active, 0);
            
            global_config.highlight_line_at_cursor = true;
            global_config.mark_thickness = 2.f;
            
            set_single_active_color(defcolor_margin, col_back);
            set_single_active_color(defcolor_margin_hover, col_back);
            set_single_active_color(defcolor_margin_active, col_back);
        }break;
        
        case FusionMode_Insert:
        {
            global_config.highlight_line_at_cursor = false;
            global_config.mark_thickness = 0.f;
            
            set_single_active_color(defcolor_margin, col_margin);
            set_single_active_color(defcolor_margin_hover, col_margin_hover);
            set_single_active_color(defcolor_margin_active, col_margin_active);
        }break;
    }
}

CUSTOM_COMMAND_SIG(fusion_toggle_mode)
CUSTOM_DOC("TODO - document fusion mode")
{
    if (fusion_mode == FusionMode_Command){
        fusion_set_mode(FusionMode_Insert);
    }
    else{
        fusion_set_mode(FusionMode_Command);
    }
}

CUSTOM_COMMAND_SIG(fusion_input_handler)
CUSTOM_DOC("TODO - document fusion mode")
{
    Scratch_Block scratch(app);
    default_input_handler_init(app, scratch);
    
    View_ID view = get_this_ctx_view(app, Access_Always);
    Managed_Scope scope = view_get_managed_scope(app, view);
    
    for (;;){
        // NOTE(allen): Get input
        User_Input input = get_next_input(app, EventPropertyGroup_Any, 0);
        ProfileScopeNamed(app, "before view input", view_input_profile);
        if (input.abort){
            break;
        }
        
        // NOTE(allen): Get map_id
        Command_Map_ID map_id = fusion_map_command;
        if (fusion_mode == FusionMode_Insert){
            map_id = fusion_map_insert;
        }
        
        // NOTE(allen): Get binding
        Command_Binding binding = map_get_binding_recursive(&framework_mapping, map_id, &input.event);
        if (binding.custom == 0){
            leave_current_input_unhandled(app);
            continue;
        }
        
        // NOTE(allen): Run the command
        default_pre_command(app, scope);
        ProfileCloseNow(view_input_profile);
        
        binding.custom(app);
        
        ProfileScope(app, "after view input");
        
        default_post_command(app, scope);
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(fusion_return)
CUSTOM_DOC("TODO")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            goto_jump_at_cursor(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        write_text(app, string_u8_litexpr("\n"));
    }
}

CUSTOM_COMMAND_SIG(fusion_return_shift)
CUSTOM_DOC("TODO")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            goto_jump_at_cursor_same_panel(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        write_text(app, string_u8_litexpr("\n"));
    }
}

function b32
string_is_blank(String_Const_u8 string){
    b32 is_blank = true;
    for (u64 i = 0; i < string.size; i += 1){
        if (!character_is_whitespace(string.str[i])){
            is_blank = false;
            break;
        }
    }
    return(is_blank);
}

CUSTOM_COMMAND_SIG(fusion_backspace)
CUSTOM_DOC("TODO")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    if (view != 0){
        Scratch_Block scratch(app);
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWrite);
        i64 pos = view_get_cursor_pos(app, view);
        i64 line_number = get_line_number_from_pos(app, buffer, pos);
        Buffer_Cursor beginning_of_line = get_line_side(app, buffer, line_number, Side_Min);
        Range_i64 range = Ii64(beginning_of_line.pos, pos);
        String_Const_u8 string = push_buffer_range(app, scratch, buffer, range);
        if (string_is_blank(string)){
            if (line_number > 1){
                Buffer_Cursor end_of_prev_line = get_line_side(app, buffer, line_number - 1, Side_Max);
                range.min = end_of_prev_line.pos;
            }
            buffer_replace_range(app, buffer, range, string_u8_litexpr(""));
        }
        else{
            current_view_boundary_delete(app, Scan_Backward,
                                         push_boundary_list(scratch, boundary_alpha_numeric, boundary_token,
                                                            boundary_non_whitespace));
        }
    }
}

CUSTOM_COMMAND_SIG(fusion_delete)
CUSTOM_DOC("TODO")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    if (view != 0){
        Scratch_Block scratch(app);
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWrite);
        i64 pos = view_get_cursor_pos(app, view);
        i64 line_number = get_line_number_from_pos(app, buffer, pos);
        Buffer_Cursor end_of_line = get_line_side(app, buffer, line_number, Side_Max);
        Range_i64 range = Ii64(pos, end_of_line.pos);
        String_Const_u8 string = push_buffer_range(app, scratch, buffer, range);
        if (string_is_blank(string)){
            i64 line_count = buffer_get_line_count(app, buffer);
            if (line_number < line_count){
                Buffer_Cursor beginning_of_prev_line = get_line_side(app, buffer, line_number + 1, Side_Min);
                range.max = beginning_of_prev_line.pos;
            }
            buffer_replace_range(app, buffer, range, string_u8_litexpr(""));
        }
        else{
            current_view_boundary_delete(app, Scan_Forward,
                                         push_boundary_list(scratch, boundary_alpha_numeric, boundary_token,
                                                            boundary_non_whitespace));
        }
    }
}

////////////////////////////////

function void
setup_fusion_mapping(Mapping *mapping){
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(mapid_global);
    BindCore(default_startup, CoreCode_Startup);
    BindCore(default_try_exit, CoreCode_TryExit);
    BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
    Bind(exit_4coder,          KeyCode_F4, KeyCode_Alt);
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
    Bind(fusion_toggle_mode, KeyCode_Insert);
    Bind(fusion_toggle_mode, KeyCode_Escape);
    
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    
    SelectMap(fusion_map_command);
    ParentMap(mapid_global);
    Bind(set_mark,                        KeyCode_Space);
    
    Bind(move_up,                         KeyCode_Up);
    Bind(move_down,                       KeyCode_Down);
    Bind(move_line_up,                    KeyCode_Up, KeyCode_Shift);
    Bind(move_line_down,                  KeyCode_Down, KeyCode_Shift);
    Bind(move_left_alpha_numeric_boundary, KeyCode_Left);
    Bind(move_right_alpha_numeric_boundary, KeyCode_Right);
    Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, KeyCode_Shift);
    Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, KeyCode_Shift);
    Bind(seek_end_of_line,                KeyCode_End);
    Bind(seek_beginning_of_line,          KeyCode_Home);
    Bind(page_up,                         KeyCode_PageUp);
    Bind(page_down,                       KeyCode_PageDown);
    
    Bind(fusion_backspace, KeyCode_Backspace);
    Bind(fusion_delete,    KeyCode_Delete);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Shift);
    Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Shift);
    
    Bind(fusion_return,            KeyCode_Return);
    Bind(fusion_return_shift,      KeyCode_Return, KeyCode_Shift);
    
    Bind(change_active_panel,             KeyCode_Comma);
    Bind(change_active_panel_backwards,   KeyCode_Comma, KeyCode_Shift);
    Bind(change_to_build_panel,           KeyCode_Period);
    Bind(close_build_panel,               KeyCode_Period, KeyCode_Shift);
    
    Bind(comment_line_toggle,             KeyCode_Semicolon);
    
    Bind(keyboard_macro_start_recording , KeyCode_Control);
    Bind(keyboard_macro_finish_recording, KeyCode_Alt);
    Bind(keyboard_macro_replay,           KeyCode_U);
    Bind(interactive_open_or_new,         KeyCode_O);
    Bind(interactive_switch_buffer,       KeyCode_I);
    Bind(goto_next_jump,                  KeyCode_N);
    Bind(goto_prev_jump,                  KeyCode_N, KeyCode_Shift);
    Bind(goto_first_jump,                 KeyCode_M);
    Bind(command_lister,                  KeyCode_X);
    Bind(jump_to_last_point,              KeyCode_P);
    Bind(replace_in_range,                KeyCode_A);
    Bind(copy,                            KeyCode_C);
    Bind(paste_next_and_indent,           KeyCode_V);
    Bind(delete_range,                    KeyCode_D);
    Bind(delete_line,                     KeyCode_D, KeyCode_Shift);
    Bind(center_view,                     KeyCode_E);
    Bind(search,                          KeyCode_F);
    Bind(list_all_substring_locations_case_insensitive, KeyCode_S);
    Bind(goto_line,                       KeyCode_G);
    Bind(snippet_lister,                  KeyCode_J);
    Bind(kill_buffer,                     KeyCode_K, KeyCode_Shift);
    Bind(duplicate_line,                  KeyCode_L);
    Bind(query_replace,                   KeyCode_Q);
    Bind(query_replace_identifier,        KeyCode_Q, KeyCode_Shift);
    Bind(list_all_locations_of_identifier, KeyCode_T);
    Bind(redo,                            KeyCode_Y);
    Bind(undo,                            KeyCode_Z);
    Bind(jump_to_definition_at_cursor,    KeyCode_W);
    
    Bind(view_buffer_other_panel,     KeyCode_1);
    Bind(swap_panels,                 KeyCode_2);
    Bind(open_matching_file_cpp,      KeyCode_3);
    
    Bind(write_zero_struct,          KeyCode_0);
    Bind(open_long_braces,           KeyCode_LeftBracket);
    Bind(open_long_braces_semicolon, KeyCode_LeftBracket, KeyCode_Shift);
    Bind(open_long_braces_break,     KeyCode_RightBracket, KeyCode_Shift);
    
    Bind(select_surrounding_scope,   KeyCode_RightBracket);
    Bind(select_prev_top_most_scope, KeyCode_Quote);
    Bind(select_next_scope_after_current, KeyCode_ForwardSlash);
    Bind(place_in_scope,             KeyCode_ForwardSlash, KeyCode_Shift);
    Bind(delete_current_scope,       KeyCode_Minus);
    Bind(if0_off,                    KeyCode_9, KeyCode_Alt);
    
    SelectMap(fusion_map_insert);
    ParentMap(mapid_global);
    BindTextInput(write_text_and_auto_indent);
    Bind(move_up,                KeyCode_Up);
    Bind(move_down,              KeyCode_Down);
    Bind(move_left,              KeyCode_Left);
    Bind(move_right,             KeyCode_Right);
    Bind(delete_char,            KeyCode_Delete);
    Bind(backspace_char,         KeyCode_Backspace);
    Bind(seek_end_of_line,       KeyCode_End);
    Bind(seek_beginning_of_line, KeyCode_Home);
    Bind(move_up_to_blank_line_end,   KeyCode_PageUp);
    Bind(move_down_to_blank_line_end, KeyCode_PageDown);
    Bind(word_complete,          KeyCode_Tab);
}

void
custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    
    // NOTE(allen): setup for default framework
    default_framework_init(app);
    
    // NOTE(allen): default hooks and command maps
    set_all_default_hooks(app);
    set_custom_hook(app, HookID_ViewEventHandler, fusion_input_handler);
    
    mapping_init(tctx, &framework_mapping);
    setup_fusion_mapping(&framework_mapping);
}

// BOTTOM

