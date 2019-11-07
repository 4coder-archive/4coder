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

enum App_State{
    APP_STATE_EDIT,
    APP_STATE_RESIZING,
    // never below this
    APP_STATE_COUNT
};

struct Model_View_Command_Function{
    Model_View_Command_Function *next;
    Custom_Command_Function *custom_func;
    View_ID view_id;
};

struct Models{
    Arena arena_;
    Arena *arena;
    Heap heap;
    
    App_Settings settings;
    App_State state;
    
    Face_ID global_face_id;
    
    Coroutine_Group coroutines;
    
    Child_Process_Container child_processes;
    Custom_API config_api;
    
    Tick_Function *tick;
    Render_Caller_Function *render_caller;
    Delta_Rule_Function *delta_rule;
    umem delta_rule_memory_size;
    
    Hook_Function *buffer_viewer_update;
    Custom_Command_Function *view_event_handler;
    Buffer_Name_Resolver_Function *buffer_name_resolver;
    Buffer_Hook_Function *begin_buffer;
    Buffer_Hook_Function *end_buffer;
    Buffer_Hook_Function *new_file;
    Buffer_Hook_Function *save_file;
    Buffer_Edit_Range_Function *buffer_edit_range;
    Buffer_Region_Function *buffer_region;
    Layout_Function *layout_func;
    
    Color_Table fallback_color_table;
    Color_Table color_table;
    
    Model_View_Command_Function *free_view_cmd_funcs;
    Model_View_Command_Function *first_view_cmd_func;
    Model_View_Command_Function *last_view_cmd_func;
    
    Layout layout;
    Working_Set working_set;
    Live_Views view_set;
    Global_History global_history;
    Text_Layout_Container text_layouts;
    Font_Set font_set;
    
    Managed_ID_Set managed_id_set;
    Dynamic_Workspace dynamic_workspace;
    Lifetime_Allocator lifetime_allocator;
    
    Editing_File *message_buffer;
    Editing_File *scratch_buffer;
    Editing_File *log_buffer;
    
    Hot_Directory hot_directory;
    
    b32 keep_playing;
    
    b32 has_new_title;
    char *title_space;
    i32 title_capacity;
    
    Panel *resizing_intermediate_panel;
    
    Plat_Handle period_wakeup_timer;
    i32 frame_counter;
    u32 next_animate_delay;
    b32 animate_next_frame;
    
    Profile_Global_List profile_list;
    
    // Last frame state
    Vec2_i32 prev_p;
    Panel *prev_mouse_panel;
    b32 animated_last_frame;
    u64 last_render_usecond_stamp;
    
    // Event Context
    Application_Step_Input *input;
    i64 current_input_sequence_number;
    User_Input current_input;
    b8 current_input_unhandled;
    
    b8 in_render_mode;
    Render_Target *target;
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

struct File_Init{
    String_Const_u8 name;
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
    //
    CLAct_COUNT,
};

enum Command_Line_Mode{
    CLMode_App,
    CLMode_Custom
};

#endif

// BOTTOM

