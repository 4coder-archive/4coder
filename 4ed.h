/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application Layer for 4coder
 *
 */

// TOP

#if !defined(FRED_H)
#define FRED_H

#define MAX_VIEWS 16

// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): Fix this nonsense
#define KEY_INPUT_BUFFER_SIZE 8
#define KEY_EXTRA_SIZE 5

struct Key_Input_Data{
    Key_Event_Data keys[KEY_INPUT_BUFFER_SIZE + KEY_EXTRA_SIZE];
    i32 count;
    b8 modifiers[MDFR_INDEX_COUNT];
};

typedef u8 Log_To_Type;
enum{
    LogTo_Nothing,
    LogTo_Stdout,
    LogTo_LogFile,
    LogTo_COUNT
};

struct Plat_Settings{
    char *custom_dll;
    b8 custom_dll_is_strict;
    b8 fullscreen_window;
    
    u8 use_log;
    
    i32 window_w;
    i32 window_h;
    i32 window_x;
    i32 window_y;
    b8 set_window_pos;
    b8 set_window_size;
    b8 maximize_window;
    
    b8 use_hinting;
};

#define App_Read_Command_Line_Sig(name)             \
void *name(Thread_Context *tctx,     \
           System_Functions *system, \
           String_Const_u8 current_directory,\
           Plat_Settings *plat_settings,\
           char ***files,   \
           i32 **file_count,\
           i32 argc,        \
           char **argv)

typedef App_Read_Command_Line_Sig(App_Read_Command_Line);

struct Custom_API{
    Get_Binding_Data_Function *get_bindings;
    _Get_Version_Function *get_alpha_4coder_version;
};

#define App_Init_Sig(name) \
void name(System_Functions *system, \
          Render_Target *target,    \
          void *base_ptr,           \
          String_Const_u8 clipboard,\
          String_Const_u8 current_directory,\
          Custom_API api)

typedef App_Init_Sig(App_Init);

#include "4ed_cursor_codes.h"

struct Application_Step_Result{
    Application_Mouse_Cursor mouse_cursor_type;
    b32 lctrl_lalt_is_altgr;
    b32 perform_kill;
    b32 animating;
    b32 has_new_title;
    char *title_string;
};

struct Application_Step_Input{
    b32 first_step;
    f32 dt;
    Key_Input_Data keys;
    Mouse_State mouse;
    String_Const_u8 clipboard;
    b32 clipboard_changed;
    b32 trying_to_kill;
};

#define App_Step_Sig(name) Application_Step_Result \
name(System_Functions *system,             \
Render_Target *target,                \
void *base_ptr,                       \
Application_Step_Input *input)

typedef App_Step_Sig(App_Step);

typedef b32 Log_Function(String_Const_u8 str);
typedef Log_Function *App_Get_Logger(System_Functions *system);

struct App_Functions{
    App_Get_Logger *get_logger;
    App_Read_Command_Line *read_command_line;
    App_Init *init;
    App_Step *step;
};

#define App_Get_Functions_Sig(name) App_Functions name()
typedef App_Get_Functions_Sig(App_Get_Functions);

#endif

// BOTTOM

