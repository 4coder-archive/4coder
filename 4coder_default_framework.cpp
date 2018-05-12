/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 
4coder behaviour.
*/

// TOP

static Partition global_part;
static General_Memory global_general;


#if !defined(AUTO_CENTER_AFTER_JUMPS)
#define AUTO_CENTER_AFTER_JUMPS true
#endif
static bool32 auto_center_after_jumps = AUTO_CENTER_AFTER_JUMPS;
static char locked_buffer_space[256];
static String locked_buffer = make_fixed_width_string(locked_buffer_space);


static View_ID special_note_view_id = 0;


static bool32 default_use_scrollbars = false;
static bool32 default_use_file_bars = true;


View_Paste_Index view_paste_index_[16];
View_Paste_Index *view_paste_index = view_paste_index_ - 1;


static char out_buffer_space[1024];
static char command_space[1024];
static char hot_directory_space[1024];


static bool32 suppressing_mouse = false;


static ID_Based_Jump_Location prev_location = {0};


static Config_Data global_config = {0};

////////////////////////////////

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
    if (!default_use_scrollbars){
        view_set_setting(app, view, ViewSetting_ShowScrollbar, false);
    }
    if (!default_use_file_bars){
        view_set_setting(app, view, ViewSetting_ShowFileBar, false);
    }
}

static void
close_special_note_view(Application_Links *app){
    View_Summary special_view = get_view(app, special_note_view_id, AccessAll);
    if (special_view.exists){
        close_view(app, &special_view);
    }
    special_note_view_id = 0;
}

static View_Summary
open_special_note_view(Application_Links *app, bool32 create_if_not_exist = true){
    View_Summary special_view = get_view(app, special_note_view_id, AccessAll);
    if (create_if_not_exist && !special_view.exists){
        View_Summary view = get_active_view(app, AccessAll);
        special_view = open_view(app, &view, ViewSplit_Bottom);
        new_view_settings(app, &special_view);
        view_set_split_proportion(app, &special_view, .2f);
        set_active_view(app, &view);
        special_note_view_id = special_view.view_id;
    }
    return(special_view);
}

static View_Summary
get_next_active_panel(Application_Links *app, View_Summary *view_start){
    View_ID original_view_id = view_start->view_id;
    View_Summary view = *view_start;
    do{
        get_view_next_looped(app, &view, AccessAll);
        if (view.view_id != special_note_view_id){
            break;
        }
    }while(view.view_id != original_view_id);
    if (!view.exists){
        memset(&view, 0, sizeof(view));
    }
    return(view);
}

static View_Summary
get_prev_active_panel(Application_Links *app, View_Summary *view_start){
    View_ID original_view_id = view_start->view_id;
    View_Summary view = *view_start;
    do{
        get_view_prev_looped(app, &view, AccessAll);
        if (view.view_id != special_note_view_id){
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
    view = get_next_active_panel(app, &view);
    if (view.exists){
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel_backwards)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next lowest view_id.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view = get_prev_active_panel(app, &view);
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

#if 0
static bool32
get_current_name(char **name_out, int32_t *len_out){
    bool32 result = false;
    *name_out = 0;
    if (user_name.str[0] != 0){
        *name_out = user_name.str;
        *len_out = user_name.size;
        result = true;
    }
    return(result);
}

static String
get_default_theme_name(void){
    String str = default_theme_name;
    if (str.size == 0){
        str = make_lit_string("4coder");
    }
    return(str);
}

static String
get_default_font_name(void){
    String str = default_font_name;
    if (str.size == 0){
        str = make_lit_string("Liberation Mono");
    }
    return(str);
}
#endif

////////////////////////////////

static bool32
descriptions_match(Face_Description *a, Face_Description *b){
    bool32 result = false;
    if (match(a->font.name, b->font.name) && a->font.in_local_font_folder == b->font.in_local_font_folder){
        if (memcmp((&a->pt_size), (&b->pt_size), sizeof(*a) - sizeof(a->font)) == 0){
            result = true;
        }
    }
    return(result);
}

static Face_ID
get_existing_face_id_matching_name(Application_Links *app, char *name, int32_t len){
    String name_str = make_string(name, len);
    Face_ID largest_id = get_largest_face_id(app);
    Face_ID result = 0;
    for (Face_ID id = 1; id <= largest_id; ++id){
        Face_Description compare = get_face_description(app, id);
        if (match(compare.font.name, name_str)){
            result = id;
            break;
        }
    }
    return(result);
}

static Face_ID
get_existing_face_id_matching_description(Application_Links *app, Face_Description *description){
    Face_ID largest_id = get_largest_face_id(app);
    Face_ID result = 0;
    for (Face_ID id = 1; id <= largest_id; ++id){
        Face_Description compare = get_face_description(app, id);
        if (descriptions_match(&compare, description)){
            result = id;
            break;
        }
    }
    return(result);
}

static Face_ID
get_face_id_by_name(Application_Links *app, char *name, int32_t len, Face_Description *base_description){
    Face_ID new_id = 0;
    
    String str = make_string(name, len);
    if (!match(str, base_description->font.name)){
        new_id = get_existing_face_id_matching_name(app, name, len);
        if (new_id == 0){
            Face_Description description = *base_description;
            copy_fast_unsafe_cs(description.font.name, str);
            description.font.name[str.size] = 0;
            
            description.font.in_local_font_folder = false;
            new_id = try_create_new_face(app, &description);
            if (new_id == 0){
                description.font.in_local_font_folder = true;
                new_id = try_create_new_face(app, &description);
            }
        }
    }
    
    return(new_id);
}

static void
change_font(Application_Links *app, char *name, int32_t len, bool32 apply_to_all_buffers){
    Face_ID global_face_id = get_face_id(app, 0);
    Face_Description description = get_face_description(app, global_face_id);
    Face_ID new_id = get_face_id_by_name(app, name, len, &description);
    if (new_id != 0){
        set_global_face(app, new_id, apply_to_all_buffers);
    }
}

static void
buffer_set_font(Application_Links *app, Buffer_Summary *buffer, char *name, int32_t len){
    Face_ID current_id = get_face_id(app, buffer);
    if (current_id != 0){
        Face_Description description = get_face_description(app, current_id);
        Face_ID new_id = get_face_id_by_name(app, name, len, &description);
        if (new_id != 0){
            buffer_set_face(app, buffer, new_id);
        }
    }
}

static Face_ID
get_face_id_by_description(Application_Links *app, Face_Description *description, Face_Description *base_description){
    Face_ID new_id = 0;
    
    if (!descriptions_match(description, base_description)){
        new_id = get_existing_face_id_matching_description(app, description);
        if (new_id == 0){
            new_id = try_create_new_face(app, description);
        }
    }
    
    return(new_id);
}

static void
change_face_description(Application_Links *app, Face_Description *new_description, bool32 apply_to_all_buffers){
    Face_ID global_face_id = get_face_id(app, 0);
    Face_Description old_description = get_face_description(app, global_face_id);
    Face_ID new_id = get_face_id_by_description(app, new_description, &old_description);
    if (new_id != 0){
        set_global_face(app, new_id, apply_to_all_buffers);
    }
}

static void
buffer_set_face_description(Application_Links *app, Buffer_Summary *buffer, Face_Description *new_description){
    Face_ID current_id = get_face_id(app, buffer);
    if (current_id != 0){
        Face_Description old_description = get_face_description(app, current_id);
        Face_ID new_id = get_face_id_by_description(app, new_description, &old_description);
        if (new_id != 0){
            buffer_set_face(app, buffer, new_id);
        }
    }
}

static Face_Description
get_buffer_face_description(Application_Links *app, Buffer_Summary *buffer){
    Face_ID current_id = get_face_id(app, buffer);
    Face_Description description = {0};
    if (current_id != 0){
        description = get_face_description(app, current_id);
    }
    return(description);
}

static Face_Description
get_global_face_description(Application_Links *app){
    Face_ID current_id = get_face_id(app, 0);
    Face_Description description = get_face_description(app, current_id);
    return(description);
}

////////////////////////////////

static void
init_memory(Application_Links *app){
    int32_t part_size = (32 << 20);
    int32_t general_size = (4 << 20);
    
    void *part_mem = memory_allocate(app, part_size);
    global_part = make_part(part_mem, part_size);
    
    void *general_mem = memory_allocate(app, general_size);
    general_memory_open(&global_general, general_mem, general_size);
}

static void
default_4coder_initialize(Application_Links *app, bool32 use_scrollbars, bool32 use_file_bars){
    init_memory(app);
    load_config_and_apply(app, &global_part, &global_config);
    load_folder_of_themes_into_live_set(app, &global_part, "themes");
    
    String theme = global_config.default_theme_name;
    String font = global_config.default_font_name;
    
    change_theme(app, theme.str, theme.size);
    change_font(app, font.str, font.size, true);
    
    default_use_scrollbars = use_scrollbars;
    default_use_file_bars = use_file_bars;
}

static void
default_4coder_initialize(Application_Links *app){
    default_4coder_initialize(app, false, true);
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

