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
    char *user_file;
    b32 user_file_is_strict;
    
    char *init_files[8];
    i32 init_files_count;
    i32 init_files_max;
    
    i32 initial_line;
    b32 lctrl_lalt_is_altgr;
    
    i32 font_size;
    
    char *custom_font_file;
    char *custom_font_name;
    i32 custom_font_size;
    
    i32 custom_arg_start;
    i32 custom_arg_end;
};

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
    
    Command_Map map_top;
    Command_Map map_file;
    Command_Map map_ui;
    Command_Map *user_maps;
    i32 *map_id_table;
    i32 user_map_count;
    
    Command_Binding prev_command;
    
    Coroutine *command_coroutine;
    u32 command_coroutine_flags[2];
    
    Hook_Function *hooks[hook_type_count];
    Application_Links app_links;
    
    Custom_API config_api;
    
    Open_File_Hook_Function *hook_open_file;
    Open_File_Hook_Function *hook_new_file;
    Open_File_Hook_Function *hook_save_file;
    Command_Caller_Hook_Function *command_caller;
    Input_Filter_Function *input_filter;
    Scroll_Rule_Function *scroll_rule;
    
    Font_Set *font_set;
    Style_Font global_font;
    Style_Library styles;
    u32 *palette;
    i32 palette_size;
    
    Editing_Layout layout;
    Working_Set working_set;
    struct Live_Views *live_set;
    Editing_File *message_buffer;
    Editing_File *scratch_buffer;
    
    char hot_dir_base_[256];
    Hot_Directory hot_directory;
    
    Panel *prev_mouse_panel;
    
    b32 keep_playing;
    
    Debug_Data debug;
    
    i16 user_up_key;
    i16 user_down_key;
};

// BOTTOM



