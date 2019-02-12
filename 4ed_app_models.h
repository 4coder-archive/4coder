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
    
    Custom_API config_api;
    
    Application_Links app_links;
    
    Hook_Function *hooks[hook_type_count];
    Start_Hook_Function *hook_start;
    Open_File_Hook_Function *hook_open_file;
    Open_File_Hook_Function *hook_new_file;
    Open_File_Hook_Function *hook_save_file;
    Open_File_Hook_Function *hook_end_file;
    File_Edit_Finished_Function *hook_file_edit_finished;
    Command_Caller_Hook_Function *command_caller;
    Render_Caller_Function *render_caller;
    Input_Filter_Function *input_filter;
    Scroll_Rule_Function *scroll_rule;
    Buffer_Name_Resolver_Function *buffer_name_resolver;
    
    Style_Library styles;
    u32 *palette;
    
    Layout layout;
    Working_Set working_set;
    Live_Views live_set;
    Parse_Context_Memory parse_context_memory;
    Global_History global_history;
    
    Dynamic_Variable_Layout variable_layout;
    Dynamic_Workspace dynamic_workspace;
    Lifetime_Allocator lifetime_allocator;
    
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
    
    Panel *resizing_intermediate_panel;
    
    // System Context
    System_Functions *system;
    struct App_Vars *vars;
    
    // Event Context
    Application_Step_Input *input;
    Key_Event_Data key;
    
    // Render Context
    Render_Target *target;
    View *render_view;
    i32_Rect render_rect;
    Full_Cursor render_cursor;
    Range render_range;
    Buffer_Render_Item *render_items;
    i32 render_item_count;
};

////////////////////////////////

typedef i32 Dynamic_Workspace_Type;
enum{
    DynamicWorkspace_Global = 0,
    DynamicWorkspace_Unassociated = 1,
    DynamicWorkspace_Buffer = 2,
    DynamicWorkspace_View = 3,
    DynamicWorkspace_Intersected = 4,
};

enum App_State{
    APP_STATE_EDIT,
    APP_STATE_RESIZING,
    // never below this
    APP_STATE_COUNT
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

struct App_Vars{
    Models models;
    CLI_List cli_processes;
    App_State state;
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
    Models *models;
    Command_Binding bind;
};

struct File_Init{
    String name;
    Editing_File **ptr;
    b32 read_only;
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

