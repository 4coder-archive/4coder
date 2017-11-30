/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.05.2016 (dd.mm.yyyy)
 *
 * Global app level settings definition
 *
 */

// TOP

struct App_Settings{
    char *init_files[8];
    i32 init_files_count;
    i32 init_files_max;
    
    char **custom_flags;
    i32 custom_flags_count;
    
    i32 initial_line;
    b32 lctrl_lalt_is_altgr;
    
    i32 font_size;
    b32 use_hinting;
};
global_const App_Settings null_app_settings = {0};

struct Debug_Input_Event{
    Key_Code key;
    
    char consumer[32];
    
    b8 is_hold;
    b8 is_ctrl;
    b8 is_alt;
    b8 is_shift;
};

struct Debug_Data{
    Debug_Input_Event input_events[16];
    i32 this_frame_count;
};

struct Models{
    Mem_Options mem;
    App_Settings settings;
    
    Face_ID global_font_id;
    
    Mapping mapping;
    
    Command_Binding prev_command;
    
    Coroutine_Head *command_coroutine;
    u32 command_coroutine_flags[2];
    
    Hook_Function *hooks[hook_type_count];
    Application_Links app_links;
    
    Custom_API config_api;
    
    Start_Hook_Function *hook_start;
    Open_File_Hook_Function *hook_open_file;
    Open_File_Hook_Function *hook_new_file;
    Open_File_Hook_Function *hook_save_file;
    Open_File_Hook_Function *hook_end_file;
    Command_Caller_Hook_Function *command_caller;
    Input_Filter_Function *input_filter;
    Scroll_Rule_Function *scroll_rule;
    Buffer_Name_Resolver_Function *buffer_name_resolver;
    
    Style_Library styles;
    u32 *palette;
    
    Editing_Layout layout;
    Working_Set working_set;
    Live_Views live_set;
    Parse_Context_Memory parse_context_memory;
    
    Editing_File *message_buffer;
    Editing_File *scratch_buffer;
    
    Hot_Directory hot_directory;
    
    Panel *prev_mouse_panel;
    
    b32 keep_playing;
    
    Debug_Data debug;
    
    Key_Code user_up_key;
    Key_Code user_down_key;
    Key_Modifier user_up_key_modifier;
    Key_Modifier user_down_key_modifier;
    
    b32 has_new_title;
    char *title_space;
    i32 title_capacity;
};

// BOTTOM



