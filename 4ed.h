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

struct Application_Memory{
    void *vars_memory;
    i32 vars_memory_size;
    void *target_memory;
    i32 target_memory_size;
};

#define KEY_INPUT_BUFFER_SIZE 4
#define KEY_INPUT_BUFFER_DSIZE (KEY_INPUT_BUFFER_SIZE << 1)

struct Key_Input_Data{
	Key_Event_Data press[KEY_INPUT_BUFFER_SIZE];
	Key_Event_Data hold[KEY_INPUT_BUFFER_SIZE];
	i32 press_count;
    i32 hold_count;
};

struct Key_Summary{
    i32 count;
    Key_Event_Data keys[KEY_INPUT_BUFFER_DSIZE];
};

inline Key_Event_Data
get_single_key(Key_Summary *summary, i32 index){
    Assert(index >= 0 && index < summary->count);
    Key_Event_Data key;
    key = summary->keys[index];
    return key;
}

struct Input_Summary{
    Mouse_State mouse;
    Key_Summary keys;
};

struct Command_Line_Parameters{
    char **argv;
    int argc;
};

struct Plat_Settings{
    char *custom_dll;
    b32 custom_dll_is_strict;

    i32 window_w, window_h;
    i32 window_x, window_y;
    b8 set_window_pos, set_window_size;
    b8 maximize_window;
};

#define App_Read_Command_Line_Sig(name)                 \
    i32 name(System_Functions *system,                  \
             Application_Memory *memory,                \
             String current_directory,                  \
             Plat_Settings *plat_settings,                 \
             char ***files, i32 **file_count,                   \
             Command_Line_Parameters clparams           \
             )

typedef App_Read_Command_Line_Sig(App_Read_Command_Line);

#define App_Init_Sig(name) void                                         \
name(System_Functions *system,                                      \
    Render_Target *target,                                         \
    Application_Memory *memory,                                    \
    Exchange *exchange,                                            \
    String clipboard,                                  \
    String current_directory,                                      \
    Custom_API api)

typedef App_Init_Sig(App_Init);

enum Application_Mouse_Cursor{
	APP_MOUSE_CURSOR_DEFAULT,
	APP_MOUSE_CURSOR_ARROW,
	APP_MOUSE_CURSOR_IBEAM,
	APP_MOUSE_CURSOR_LEFTRIGHT,
	APP_MOUSE_CURSOR_UPDOWN,
	// never below this
	APP_MOUSE_CURSOR_COUNT
};

struct Application_Step_Result{
	Application_Mouse_Cursor mouse_cursor_type;
    b32 redraw;
    b32 lctrl_lalt_is_altgr;
};

#define App_Step_Sig(name) void          \
    name(System_Functions *system,                          \
         Key_Input_Data *input,                             \
         Mouse_State *mouse,                                \
         Render_Target *target,                             \
         Application_Memory *memory,                        \
         Exchange *exchange,                                \
         String clipboard,                      \
         b32 time_step, b32 first_step, b32 force_redraw,   \
         Application_Step_Result *result)

typedef App_Step_Sig(App_Step);

#define App_Alloc_Sig(name) void *name(void *handle, i32 size)
typedef App_Alloc_Sig(App_Alloc);

#define App_Free_Sig(name) void name(void *handle, void *block)
typedef App_Free_Sig(App_Free);

struct App_Functions{
    App_Read_Command_Line *read_command_line;
    App_Init *init;
    App_Step *step;

    App_Alloc *alloc;
    App_Free *free;
};

#define App_Get_Functions_Sig(name) App_Functions name()
typedef App_Get_Functions_Sig(App_Get_Functions);

#endif

// BOTTOM

