/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.29.2017
 *
 * Types and functions for communication between C++ and Objective-C layers.
 *
 */

// TOP

#if !defined(OSX_OBJECTIVE_C_TO_CPP_LINKS_H)
#define OSX_OBJECTIVE_C_TO_CPP_LINKS_H

#include <stdio.h>

#if 0
#define DBG_POINT() fprintf(stdout, "%s\n", __FILE__ ":" LINE_STR ":")
#else
#define DBG_POINT()
#endif

typedef enum OSX_Mouse_Event_Type{
    MouseType_Move,
    MouseType_Press,
    MouseType_Release,
} OSX_Mouse_Event_Type;

typedef struct OSX_Keyboard_Modifiers{
    b32 shift;
    b32 command;
    b32 control;
    b32 option;
    b32 caps;
} OSX_Keyboard_Modifiers;

typedef struct OSX_Objective_C_Vars{
    i32 width, height;
    b32 running;
    u32 key_count;
    u32 keys[8];
    
    u32 prev_clipboard_change_count;
    b32 has_clipboard_item;
    void *clipboard_data;
    umem clipboard_size, clipboard_max;
    b32 just_posted_to_clipboard;
    
    char *clipboard_space;
    umem clipboard_space_max;
    
    b32 full_screen;
    b32 do_toggle;
    
    i32 argc;
    char **argv;
} OSX_Objective_C_Vars;

// In C++ layer.
extern OSX_Objective_C_Vars osx_objc;

external void*
osx_allocate(umem size);

external void
osx_free(void *ptr, umem size);

external void
osx_resize(int width, int height);

external void
osx_character_input(u32 code, OSX_Keyboard_Modifiers modifier_flags);

external void
osx_mouse(i32 mx, i32 my, u32 type);

external void
osx_mouse_wheel(float dx, float dy);

external void
osx_try_to_close(void);

external void
osx_step();

external void
osx_init();

// In Objective-C layer.
external void
osx_post_to_clipboard(char *str);

external void
osx_error_dialogue(char *str);

external void
osx_add_file_listener(char *file_name);

external void
osx_remove_file_listener(char *file_name);

external i32
osx_get_file_change_event(char *buffer, i32 max, i32 *size);

external void
osx_show_cursor(i32 show_inc, i32 cursor_type);

external void
osx_begin_render(void);

external void
osx_end_render(void);

external void
osx_schedule_step(void);

external void
osx_toggle_fullscreen(void);

external b32
osx_is_fullscreen(void);

external void
osx_close_app(void);

#endif

// BOTTOM

