/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 
4coder behaviour.
*/

// TOP

static void
unlock_jump_buffer(void){
    locked_buffer.size = 0;
}

static void
lock_jump_buffer(String_Const_u8 name){
    if (name.size < sizeof(locked_buffer_space)){
        block_copy(locked_buffer_space, name.str, name.size);
        locked_buffer = SCu8(locked_buffer_space, name.size);
    }
}

static void
lock_jump_buffer(char *name, i32 size){
    lock_jump_buffer(SCu8(name, size));
}

static void
lock_jump_buffer(Application_Links *app, Buffer_ID buffer_id){
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer_id);
    lock_jump_buffer(buffer_name);
    end_temp(temp);
}

static View_ID
get_view_for_locked_jump_buffer(Application_Links *app){
    View_ID view = 0;
    if (locked_buffer.size > 0){
        Buffer_ID buffer = get_buffer_by_name(app, locked_buffer, AccessAll);
        if (buffer != 0){
            view = get_first_view_with_buffer(app, buffer);
        }
        else{
            unlock_jump_buffer();
        }
    }
    return(view);
}

////////////////////////////////

static void
new_view_settings(Application_Links *app, View_ID view){
    if (!global_config.use_scroll_bars){
        view_set_setting(app, view, ViewSetting_ShowScrollbar, false);
    }
    if (!global_config.use_file_bars){
        view_set_setting(app, view, ViewSetting_ShowFileBar, false);
    }
}

////////////////////////////////

static void
view_set_passive(Application_Links *app, View_ID view_id, b32 value){
    Managed_Scope scope = 0;
    view_get_managed_scope(app, view_id, &scope);
    managed_variable_set(app, scope, view_is_passive_loc, (u64)value);
}

static b32
view_get_is_passive(Application_Links *app, View_ID view_id){
    Managed_Scope scope = 0;
    view_get_managed_scope(app, view_id, &scope);
    u64 is_passive = 0;
    managed_variable_get(app, scope, view_is_passive_loc, &is_passive);
    return(is_passive != 0);
}

static View_ID
open_footer_panel(Application_Links *app, View_ID view){
    View_ID special_view = open_view(app, view, ViewSplit_Bottom);
    new_view_settings(app, special_view);
    Buffer_ID buffer = view_get_buffer(app, special_view, AccessAll);
    Face_ID face_id = 0;
    get_face_id(app, buffer, &face_id);
    Face_Metrics metrics = {};
    get_face_metrics(app, face_id, &metrics);
    view_set_split_pixel_size(app, special_view, (i32)(metrics.line_height*20.f));
    view_set_passive(app, special_view, true);
    return(special_view);
}

static void
close_build_footer_panel(Application_Links *app){
    if (build_footer_panel_view_id != 0){
        view_close(app, build_footer_panel_view_id);
        build_footer_panel_view_id = 0;
    }
}

static View_ID
open_build_footer_panel(Application_Links *app){
    if (build_footer_panel_view_id == 0){
        View_ID view = get_active_view(app, AccessAll);
        build_footer_panel_view_id = open_footer_panel(app, view);
        view_set_active(app, view);
    }
    return(build_footer_panel_view_id);
}

static View_ID
get_next_view_looped_primary_panels(Application_Links *app, View_ID start_view_id, Access_Flag access){
    View_ID view_id = start_view_id;
    do{
        view_id = get_next_view_looped_all_panels(app, view_id, access);
        if (!view_get_is_passive(app, view_id)){
            break;
        }
    }while(view_id != start_view_id);
    return(view_id);
}

static View_ID
get_prev_view_looped_primary_panels(Application_Links *app, View_ID start_view_id, Access_Flag access){
    View_ID view_id = start_view_id;
    do{
        view_id = get_prev_view_looped_all_panels(app, view_id, access);
        if (!view_get_is_passive(app, view_id)){
            break;
        }
    }while(view_id != start_view_id);
    return(view_id);
}

static View_ID
get_next_view_after_active(Application_Links *app, Access_Flag access){
    View_ID view = get_active_view(app, access);
    if (view != 0){
        view = get_next_view_looped_primary_panels(app, view, access);
    }
    return(view);
}

////////////////////////////////

static void
view_buffer_set(Application_Links *app, Buffer_ID *buffers, i32 *positions, i32 count){
    if (count > 0){
        Arena *arena = context_get_arena(app);
        Temp_Memory temp = begin_temp(arena);
        
        struct View_Node{
            View_Node *next;
            View_ID view_id;
        };
        
        View_ID active_view_id = get_active_view(app, AccessAll);
        View_ID first_view_id = active_view_id;
        if (view_get_is_passive(app, active_view_id)){
            first_view_id = get_next_view_looped_primary_panels(app, active_view_id, AccessAll);
        }
        
        View_ID view_id = first_view_id;
        
        View_Node *primary_view_first = 0;
        View_Node *primary_view_last = 0;
        i32 available_view_count = 0;
        
        primary_view_first = primary_view_last = push_array(arena, View_Node, 1);
        primary_view_last->next = 0;
        primary_view_last->view_id = view_id;
        available_view_count += 1;
        for (;;){
            view_id = get_next_view_looped_primary_panels(app, view_id, AccessAll);
            if (view_id == first_view_id){
                break;
            }
            View_Node *node = push_array(arena, View_Node, 1);
            primary_view_last->next = node;
            node->next = 0;
            node->view_id = view_id;
            primary_view_last = node;
            available_view_count += 1;
        }
        
        i32 buffer_set_count = clamp_top(count, available_view_count);
        View_Node *node = primary_view_first;
        for (i32 i = 0; i < buffer_set_count; i += 1, node = node->next){
            if (view_set_buffer(app, node->view_id, buffers[i], 0)){
                view_set_cursor(app, node->view_id, seek_pos(positions[i]), true);
            }
        }
        
        end_temp(temp);
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(change_active_panel)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next highest view_id.")
{
    View_ID view = get_active_view(app, AccessAll);
    view = get_next_view_looped_primary_panels(app, view, AccessAll);
    if (view != 0){
        view_set_active(app, view);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel_backwards)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next lowest view_id.")
{
    View_ID view = get_active_view(app, AccessAll);
    view = get_prev_view_looped_primary_panels(app, view, AccessAll);
    if (view != 0){
        view_set_active(app, view);
    }
}

CUSTOM_COMMAND_SIG(open_panel_vsplit)
CUSTOM_DOC("Create a new panel by vertically splitting the active panel.")
{
    View_ID view = get_active_view(app, AccessAll);
    View_ID new_view = open_view(app, view, ViewSplit_Right);
    new_view_settings(app, new_view);
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    view_set_buffer(app, new_view, buffer, 0);
}

CUSTOM_COMMAND_SIG(open_panel_hsplit)
CUSTOM_DOC("Create a new panel by horizontally splitting the active panel.")
{
    View_ID view = get_active_view(app, AccessAll);
    View_ID new_view = open_view(app, view, ViewSplit_Bottom);
    new_view_settings(app, new_view);
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    view_set_buffer(app, new_view, buffer, 0);
}

////////////////////////////////

// NOTE(allen): Credits to nj/FlyingSolomon for authoring the original version of this helper.

static Buffer_ID
create_or_switch_to_buffer_and_clear_by_name(Application_Links *app, String_Const_u8 name_string, View_ID default_target_view){
    Buffer_ID search_buffer = get_buffer_by_name(app, name_string, AccessAll);
    if (search_buffer != 0){
        buffer_set_setting(app, search_buffer, BufferSetting_ReadOnly, true);
        
        View_ID target_view = default_target_view;
        
        View_ID view_with_buffer_already_open = get_first_view_with_buffer(app, search_buffer);
        if (view_with_buffer_already_open != 0){
            target_view = view_with_buffer_already_open;
            view_end_ui_mode(app, target_view);
        }
        else{
            view_set_buffer(app, target_view, search_buffer, 0);
        }
        view_set_active(app, target_view);
        
        i32 buffer_size = (i32)buffer_get_size(app, search_buffer);
        
        buffer_send_end_signal(app, search_buffer);
        buffer_replace_range(app, search_buffer, make_range(0, buffer_size), string_u8_litexpr(""));
    }
    else{
        search_buffer = create_buffer(app, name_string, BufferCreate_AlwaysNew);
        buffer_set_setting(app, search_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, search_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, search_buffer, BufferSetting_WrapLine, false);
        view_set_buffer(app, default_target_view, search_buffer, 0);
        view_set_active(app, default_target_view);
    }
    
    return(search_buffer);
}

////////////////////////////////

static void
set_mouse_suppression(Application_Links *app, b32 suppress){
    if (suppress){
        suppressing_mouse = true;
        show_mouse_cursor(app, MouseCursorShow_Never);
    }
    else{
        suppressing_mouse = false;
        show_mouse_cursor(app, MouseCursorShow_Always);
    }
}

CUSTOM_COMMAND_SIG(suppress_mouse)
CUSTOM_DOC("Hides the mouse and causes all mosue input (clicks, position, wheel) to be ignored.")
{
    set_mouse_suppression(app, true);
}

CUSTOM_COMMAND_SIG(allow_mouse)
CUSTOM_DOC("Shows the mouse and causes all mouse input to be processed normally.")
{
    set_mouse_suppression(app, false);
}

CUSTOM_COMMAND_SIG(toggle_mouse)
CUSTOM_DOC("Toggles the mouse suppression mode, see suppress_mouse and allow_mouse.")
{
    set_mouse_suppression(app, !suppressing_mouse);
}

CUSTOM_COMMAND_SIG(set_mode_to_original)
CUSTOM_DOC("Sets the edit mode to 4coder original.")
{
    fcoder_mode = FCoderMode_Original;
}

CUSTOM_COMMAND_SIG(set_mode_to_notepad_like)
CUSTOM_DOC("Sets the edit mode to Notepad like.")
{
    begin_notepad_mode(app);
}

CUSTOM_COMMAND_SIG(toggle_highlight_line_at_cursor)
CUSTOM_DOC("Toggles the line highlight at the cursor.")
{
    highlight_line_at_cursor = !highlight_line_at_cursor;
}

CUSTOM_COMMAND_SIG(toggle_highlight_enclosing_scopes)
CUSTOM_DOC("In code files scopes surrounding the cursor are highlighted with distinguishing colors.")
{
    do_matching_enclosure_highlight = !do_matching_enclosure_highlight;
}

CUSTOM_COMMAND_SIG(toggle_paren_matching_helper)
CUSTOM_DOC("In code files matching parentheses pairs are colored with distinguishing colors.")
{
    do_matching_paren_highlight = !do_matching_paren_highlight;
}

CUSTOM_COMMAND_SIG(toggle_fullscreen)
CUSTOM_DOC("Toggle fullscreen mode on or off.  The change(s) do not take effect until the next frame.")
{
    set_fullscreen(app, !is_fullscreen(app));
}

////////////////////////////////

CUSTOM_COMMAND_SIG(remap_interactive)
CUSTOM_DOC("Switch to a named key binding map.")
{
    Query_Bar bar = {};
    u8 space[1024];
    bar.prompt = string_u8_litexpr("Map Name: ");
    bar.string = SCu8(space, (umem)0);
    bar.string_capacity = sizeof(space);
    if (!query_user_string(app, &bar)) return;
    change_mapping(app, bar.string);
}

////////////////////////////////

static void
default_4coder_initialize(Application_Links *app, char **command_line_files, i32 file_count, i32 override_font_size, b32 override_hinting){
    i32 heap_size = (4 << 20);
    void *heap_mem = memory_allocate(app, heap_size);
    heap_init(&global_heap);
    heap_extend(&global_heap, heap_mem, heap_size);
    
#define M \
    "Welcome to " VERSION "\n" \
    "If you're new to 4coder there are some tutorials at http://4coder.net/tutorials.html\n" \
    "Direct bug reports and feature requests to https://github.com/4coder-editor/4coder/issues\n" \
    "Other questions and discussion can be directed to editor@4coder.net or 4coder.handmade.network\n" \
    "The change log can be found in CHANGES.txt\n" \
    "\n"
    print_message(app, string_u8_litexpr(M));
#undef M
    
#if 0
    load_folder_of_themes_into_live_set(app, &global_part, "themes");
#endif
    global_config_arena = make_arena_app_links(app);
    load_config_and_apply(app, &global_config_arena, &global_config, override_font_size, override_hinting);
    
    view_rewrite_loc         = managed_variable_create_or_get_id(app, "DEFAULT.rewrite"       , 0);
    view_next_rewrite_loc    = managed_variable_create_or_get_id(app, "DEFAULT.next_rewrite"  , 0);
    view_paste_index_loc     = managed_variable_create_or_get_id(app, "DEFAULT.paste_index"   , 0);
    view_is_passive_loc      = managed_variable_create_or_get_id(app, "DEFAULT.is_passive"    , 0);
    view_snap_mark_to_cursor = managed_variable_create_or_get_id(app, "DEFAULT.mark_to_cursor", 0);
    view_ui_data             = managed_variable_create_or_get_id(app, "DEFAULT.ui_data"       , 0);
    
    // open command line files
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 hot_directory = push_hot_directory(app, scratch);
    for (i32 i = 0; i < file_count; i += 1){
        Temp_Memory temp2 = begin_temp(scratch);
        String_Const_u8 input_name = SCu8(command_line_files[i]);
        String_Const_u8 file_name = push_u8_stringf(scratch, "%.*s/%.*s", string_expand(hot_directory), string_expand(input_name));
        Buffer_ID new_buffer = create_buffer(app, file_name, BufferCreate_NeverNew|BufferCreate_MustAttachToFile);
        if (new_buffer == 0){
            create_buffer(app, input_name, 0);
        }
        end_temp(temp2);
    }
    end_temp(temp);
}

static void
default_4coder_initialize(Application_Links *app, i32 override_font_size, b32 override_hinting){
    default_4coder_initialize(app, 0, 0, override_font_size, override_hinting);
}

static void
default_4coder_initialize(Application_Links *app, char **command_line_files, i32 file_count){
    Face_Description command_line_description = get_face_description(app, 0);
    default_4coder_initialize(app, command_line_files, file_count, command_line_description.pt_size, command_line_description.hinting);
}

static void
default_4coder_initialize(Application_Links *app){
    Face_Description command_line_description = get_face_description(app, 0);
    default_4coder_initialize(app, 0, 0, command_line_description.pt_size, command_line_description.hinting);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, Buffer_Identifier left_buffer, Buffer_Identifier right_buffer){
    Buffer_ID left_id = buffer_identifier_to_id(app, left_buffer);
    Buffer_ID right_id = buffer_identifier_to_id(app, right_buffer);
    
    // Left Panel
    View_ID view = get_active_view(app, AccessAll);
    new_view_settings(app, view);
    view_set_buffer(app, view, left_id, 0);
    
    // Right Panel
    open_panel_vsplit(app);
    View_ID right_view = get_active_view(app, AccessAll);
    view_set_buffer(app, right_view, right_id, 0);
    
    // Restore Active to Left
    view_set_active(app, view);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, Buffer_Identifier left, Buffer_Identifier right, char **command_line_files, i32 file_count){
    if (file_count > 0){
        left = buffer_identifier(SCu8(command_line_files[0]));
        if (file_count > 1){
            right = buffer_identifier(SCu8(command_line_files[1]));
        }
    }
    default_4coder_side_by_side_panels(app, left, right);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, char **command_line_files, i32 file_count){
    Buffer_Identifier left = buffer_identifier(string_u8_litexpr("*scratch*"));
    Buffer_Identifier right = buffer_identifier(string_u8_litexpr("*messages*"));
    default_4coder_side_by_side_panels(app, left, right, command_line_files, file_count);
}

static void
default_4coder_side_by_side_panels(Application_Links *app){
    default_4coder_side_by_side_panels(app, 0, 0);
}

static void
default_4coder_one_panel(Application_Links *app, Buffer_Identifier buffer){
    Buffer_ID id = buffer_identifier_to_id(app, buffer);
    View_ID view = get_active_view(app, AccessAll);
    new_view_settings(app, view);
    view_set_buffer(app, view, id, 0);
}

static void
default_4coder_one_panel(Application_Links *app, char **command_line_files, i32 file_count){
    Buffer_Identifier buffer = buffer_identifier(string_u8_litexpr("*messages*"));
    if (file_count > 0){
        buffer = buffer_identifier(SCu8(command_line_files[0]));
    }
    default_4coder_one_panel(app, buffer);
}

static void
default_4coder_one_panel(Application_Links *app){
    default_4coder_one_panel(app, 0, 0);
}

// BOTTOM

