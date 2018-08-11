/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.05.2016 (dd.mm.yyyy)
 *
 * Global app level settings definition
 *
 */

// TOP

#if !defined(FRED_APP_MODELS_H)
#define FRED_APP_MODELS_H

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

struct Models{
    Mem_Options mem;
    App_Settings settings;
    
    Face_ID global_font_id;
    
    Mapping mapping;
    
    Command_Binding prev_command;
    
    i32 prev_x;
    i32 prev_y;
    b32 animated_last_frame;
    
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
    
    Dynamic_Variable_Layout buffer_variable_layout;
    Dynamic_Variable_Layout view_variable_layout;
    
    Editing_File *message_buffer;
    Editing_File *scratch_buffer;
    
    Hot_Directory hot_directory;
    
    Panel *prev_mouse_panel;
    
    b32 keep_playing;
    
    Key_Code user_up_key;
    Key_Code user_down_key;
    Key_Modifier user_up_key_modifier;
    Key_Modifier user_down_key_modifier;
    
    b32 has_new_title;
    char *title_space;
    i32 title_capacity;
    
    i32 frame_counter;
    
    i32 previous_mouse_x;
    i32 previous_mouse_y;
};

////////////////////////////////

enum App_State{
    APP_STATE_EDIT,
    APP_STATE_RESIZING,
    // never below this
    APP_STATE_COUNT
};

struct App_State_Resizing{
    Panel_Divider *divider;
};

struct Command_Data{
    Models *models;
    struct App_Vars *vars;
    System_Functions *system;
    Live_Views *live_set;
    
    i32 screen_width;
    i32 screen_height;
    
    Key_Event_Data key;
};

enum Input_Types{
    Input_AnyKey,
    Input_Esc,
    Input_MouseMove,
    Input_MouseLeftButton,
    Input_MouseRightButton,
    Input_MouseWheel,
    Input_Count
};

struct Consumption_Record{
    b32 consumed;
    char consumer[32];
};

struct Available_Input{
    Key_Input_Data *keys;
    Mouse_State *mouse;
    Consumption_Record records[Input_Count];
};

struct App_Vars{
    Models models;
    
    CLI_List cli_processes;
    
    App_State state;
    App_State_Resizing resizing;
    
    Command_Data command_data;
    
    Available_Input available_input;
};

enum Coroutine_Type{
    Co_View,
    Co_Command
};
struct App_Coroutine_State{
    void *co;
    i32 type;
};

struct Command_In{
    Command_Data *cmd;
    Command_Binding bind;
};

struct File_Init{
    String name;
    Editing_File **ptr;
    b32 read_only;
};

enum{
    Event_Keyboard,
    Event_Mouse,
};
struct Coroutine_Event{
    u32 type;
    u32 key_i;
};

enum Command_Line_Action{
    CLAct_Nothing,
    CLAct_Ignore,
    CLAct_UserFile,
    CLAct_CustomDLL,
    CLAct_InitialFilePosition,
    CLAct_WindowSize,
    CLAct_WindowMaximize,
    CLAct_WindowPosition,
    CLAct_WindowFullscreen,
    CLAct_FontSize,
    CLAct_FontUseHinting,
    CLAct_LogStdout,
    CLAct_LogFile,
    //
    CLAct_COUNT,
};

enum Command_Line_Mode{
    CLMode_App,
    CLMode_Custom
};

#endif

// BOTTOM

