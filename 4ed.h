/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application Layer for 4coder
 *
 */

// TOP

#ifndef FRED_H
#define FRED_H

#define MAX_VIEWS 16

struct Application_Memory{
    void *vars_memory;
    i32 vars_memory_size;
    void *target_memory;
    i32 target_memory_size;
    void *user_memory;
    i32 user_memory_size;
    void *debug_memory;
    i32 debug_memory_size;
};

#define KEY_INPUT_BUFFER_SIZE 8
#define KEY_EXTRA_SIZE 2

struct Key_Input_Data{
    Key_Event_Data keys[KEY_INPUT_BUFFER_SIZE + KEY_EXTRA_SIZE];
    i32 count;
};

struct Input_Summary{
    Mouse_State mouse;
    Key_Input_Data keys;
    f32 dt;
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
    b8 use_test_input;
    char *test_input;
    
    i32 window_w, window_h;
    i32 window_x, window_y;
    b8 set_window_pos;
    b8 set_window_size;
    b8 maximize_window;
    
    b8 use_hinting;
    i32 font_size;
};

#define App_Read_Command_Line_Sig(name)             \
i32 name(System_Functions *system, Application_Memory *memory, String current_directory, Plat_Settings *plat_settings, char ***files, i32 **file_count, i32  argc, char **argv)

typedef App_Read_Command_Line_Sig(App_Read_Command_Line);

struct Custom_API{
    Get_Binding_Data_Function *get_bindings;
    _Get_Version_Function *get_alpha_4coder_version;
};

#define App_Init_Sig(name) \
void name(System_Functions *system, Render_Target *target, Application_Memory *memory, String clipboard, String current_directory, Custom_API api)

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
    String clipboard;
    b32 trying_to_kill;
    u32 debug_number;
};

#define App_Step_Sig(name) Application_Step_Result \
name(System_Functions *system,             \
Render_Target *target,                \
Application_Memory *memory,           \
Application_Step_Input *input)

typedef App_Step_Sig(App_Step);


struct App_Functions{
    App_Read_Command_Line *read_command_line;
    App_Init *init;
    App_Step *step;
};

#define App_Get_Functions_Sig(name) App_Functions name()
typedef App_Get_Functions_Sig(App_Get_Functions);

#endif

// BOTTOM

