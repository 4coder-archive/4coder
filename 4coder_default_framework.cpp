/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 
4coder behaviour.
*/

// TOP

static Partition global_part;
static Heap global_heap;

static void
unlock_jump_buffer(void){
    locked_buffer.size = 0;
}

static void
lock_jump_buffer(char *name, int32_t size){
    if (size <= locked_buffer.memory_size){
        copy(&locked_buffer, make_string(name, size));
    }
}

static void
lock_jump_buffer(Buffer_Summary buffer){
    lock_jump_buffer(buffer.buffer_name, buffer.buffer_name_len);
}

static View_Summary
get_view_for_locked_jump_buffer(Application_Links *app){
    View_Summary view = {0};
    if (locked_buffer.size > 0){
        Buffer_Summary buffer = get_buffer_by_name(app, locked_buffer.str, locked_buffer.size, AccessAll);
        if (buffer.exists){
            view = get_first_view_with_buffer(app, buffer.buffer_id);
        }
        else{
            unlock_jump_buffer();
        }
    }
    return(view);
}

////////////////////////////////

static void
new_view_settings(Application_Links *app, View_Summary *view){
    if (!global_config.use_scroll_bars){
        view_set_setting(app, view, ViewSetting_ShowScrollbar, false);
    }
    if (!global_config.use_file_bars){
        view_set_setting(app, view, ViewSetting_ShowFileBar, false);
    }
}

////////////////////////////////

static void
view_set_passive(Application_Links *app, View_Summary *view, bool32 value){
    Dynamic_Scope scope = view_get_dynamic_scope(app, view->view_id);
    managed_variable_set(app, scope, view_is_passive_loc, (uint64_t)value);
}

static bool32
view_get_is_passive(Application_Links *app, View_Summary *view){
    Dynamic_Scope scope = view_get_dynamic_scope(app, view->view_id);
    uint64_t is_passive = 0;
    managed_variable_get(app, scope, view_is_passive_loc, &is_passive);
    return(is_passive != 0);
}

static View_Summary
open_footer_panel(Application_Links *app, View_Summary *view){
    View_Summary special_view = open_view(app, view, ViewSplit_Bottom);
    new_view_settings(app, &special_view);
    view_set_split_proportion(app, &special_view, .2f);
    view_set_passive(app, &special_view, true);
    return(special_view);
}

////////////////////////////////

static void
close_build_footer_panel(Application_Links *app){
    View_Summary special_view = get_view(app, build_footer_panel_view_id, AccessAll);
    if (special_view.exists){
        close_view(app, &special_view);
    }
    build_footer_panel_view_id = 0;
}

static View_Summary
open_build_footer_panel(Application_Links *app, bool32 create_if_not_exist = true){
    View_Summary special_view = get_view(app, build_footer_panel_view_id, AccessAll);
    if (create_if_not_exist && !special_view.exists){
        View_Summary view = get_active_view(app, AccessAll);
        special_view = open_footer_panel(app, &view);
        set_active_view(app, &view);
        build_footer_panel_view_id = special_view.view_id;
    }
    return(special_view);
}

static View_Summary
get_next_view_looped_primary_panels(Application_Links *app, View_Summary *view_start, Access_Flag access){
    View_ID original_view_id = view_start->view_id;
    View_Summary view = *view_start;
    do{
        get_next_view_looped_all_panels(app, &view, access);
        if (!view_get_is_passive(app, &view)){
            break;
        }
    }while(view.view_id != original_view_id);
    if (!view.exists){
        memset(&view, 0, sizeof(view));
    }
    return(view);
}

static View_Summary
get_prev_view_looped_primary_panels(Application_Links *app, View_Summary *view_start, Access_Flag access){
    View_ID original_view_id = view_start->view_id;
    View_Summary view = *view_start;
    do{
        get_prev_view_looped_all_panels(app, &view, access);
        if (!view_get_is_passive(app, &view)){
            break;
        }
    }while(view.view_id != original_view_id);
    if (!view.exists){
        memset(&view, 0, sizeof(view));
    }
    return(view);
}

CUSTOM_COMMAND_SIG(change_active_panel)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next highest view_id.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view = get_next_view_looped_primary_panels(app, &view, AccessAll);
    if (view.exists){
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel_backwards)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next lowest view_id.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view = get_prev_view_looped_primary_panels(app, &view, AccessAll);
    if (view.exists){
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(open_panel_vsplit)
CUSTOM_DOC("Create a new panel by vertically splitting the active panel.")
{
    View_Summary view = get_active_view(app, AccessAll);
    View_Summary new_view = open_view(app, &view, ViewSplit_Right);
    new_view_settings(app, &new_view);
    view_set_buffer(app, &new_view, view.buffer_id, 0);
}

CUSTOM_COMMAND_SIG(open_panel_hsplit)
CUSTOM_DOC("Create a new panel by horizontally splitting the active panel.")
{
    View_Summary view = get_active_view(app, AccessAll);
    View_Summary new_view = open_view(app, &view, ViewSplit_Bottom);
    new_view_settings(app, &new_view);
    view_set_buffer(app, &new_view, view.buffer_id, 0);
}

////////////////////////////////

static void
set_mouse_suppression(Application_Links *app, int32_t suppress){
    if (suppress){
        suppressing_mouse = 1;
        show_mouse_cursor(app, MouseCursorShow_Never);
    }
    else{
        suppressing_mouse = 0;
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

CUSTOM_COMMAND_SIG(toggle_fullscreen)
CUSTOM_DOC("Toggle fullscreen mode on or off.  The change(s) do not take effect until the next frame.")
{
    set_fullscreen(app, !is_fullscreen(app));
}

////////////////////////////////

CUSTOM_COMMAND_SIG(remap_interactive)
CUSTOM_DOC("Switch to a named key binding map.")
{
    Query_Bar bar = {0};
    char space[1024];
    bar.prompt = make_lit_string("Map Name: ");
    bar.string = make_fixed_width_string(space);
    if (!query_user_string(app, &bar)) return;
    change_mapping(app, bar.string);
}

////////////////////////////////

static void
default_4coder_initialize(Application_Links *app, int32_t override_font_size, bool32 override_hinting){
    int32_t part_size = (32 << 20);
    int32_t heap_size = ( 4 << 20);
    
    void *part_mem = memory_allocate(app, part_size);
    global_part = make_part(part_mem, part_size);
    
    void *heap_mem = memory_allocate(app, heap_size);
    heap_init(&global_heap);
    heap_extend(&global_heap, heap_mem, heap_size);
    
    static const char message[] = ""
        "Welcome to " VERSION "\n"
        "If you're new to 4coder there are some tutorials at http://4coder.net/tutorials.html\n"
        "Direct bug reports to editor@4coder.net for maximum reply speed\n"
        "Questions or requests can go to editor@4coder.net or to 4coder.handmade.network\n"
        "The change log can be found in CHANGES.txt\n"
        "\n";
    String msg = make_lit_string(message);
    print_message(app, msg.str, msg.size);
    
    load_folder_of_themes_into_live_set(app, &global_part, "themes");
    load_config_and_apply(app, &global_part, &global_config, override_font_size, override_hinting);
    
    view_rewrite_loc      = managed_variable_create_or_get_id(app, "DEFAULT.rewrite"     , (uint64_t)0);
    view_next_rewrite_loc = managed_variable_create_or_get_id(app, "DEFAULT.next_rewrite", (uint64_t)0);
    view_paste_index_loc  = managed_variable_create_or_get_id(app, "DEFAULT.paste_index" , (uint64_t)0);
    view_is_passive_loc   = managed_variable_create_or_get_id(app, "DEFAULT.is_passive"  , (uint64_t)false);
}

static void
default_4coder_initialize(Application_Links *app){
    Face_Description command_line_description = get_face_description(app, 0);
    default_4coder_initialize(app, command_line_description.pt_size, command_line_description.hinting);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, Buffer_Identifier left_buffer, Buffer_Identifier right_buffer){
    Buffer_ID left_id = buffer_identifier_to_id(app, left_buffer);
    Buffer_ID right_id = buffer_identifier_to_id(app, right_buffer);
    
    // Left Panel
    View_Summary view = get_active_view(app, AccessAll);
    new_view_settings(app, &view);
    view_set_buffer(app, &view, left_id, 0);
    
    // Right Panel
    open_panel_vsplit(app);
    View_Summary right_view = get_active_view(app, AccessAll);
    view_set_buffer(app, &right_view, right_id, 0);
    
    // Restore Active to Left
    set_active_view(app, &view);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, char **command_line_files, int32_t file_count){
    Buffer_Identifier left = buffer_identifier(literal("*scratch*"));
    Buffer_Identifier right = buffer_identifier(literal("*messages*"));
    
    if (file_count > 0){
        char *left_name = command_line_files[0];
        int32_t left_len = str_size(left_name);
        left = buffer_identifier(left_name, left_len);
        
        if (file_count > 1){
            char *right_name = command_line_files[1];
            int32_t right_len = str_size(right_name);
            right = buffer_identifier(right_name, right_len);
        }
    }
    
    default_4coder_side_by_side_panels(app, left, right);
}

static void
default_4coder_side_by_side_panels(Application_Links *app){
    default_4coder_side_by_side_panels(app, 0, 0);
}

static void
default_4coder_one_panel(Application_Links *app, Buffer_Identifier buffer){
    Buffer_ID id = buffer_identifier_to_id(app, buffer);
    
    View_Summary view = get_active_view(app, AccessAll);
    new_view_settings(app, &view);
    view_set_buffer(app, &view, id, 0);
}

static void
default_4coder_one_panel(Application_Links *app, char **command_line_files, int32_t file_count){
    Buffer_Identifier buffer = buffer_identifier(literal("*messages*"));
    
    if (file_count > 0){
        char *name = command_line_files[0];
        int32_t len = str_size(name);
        buffer = buffer_identifier(name, len);
    }
    
    default_4coder_one_panel(app, buffer);
}

static void
default_4coder_one_panel(Application_Links *app){
    default_4coder_one_panel(app, 0, 0);
}

// BOTTOM

