/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.28.2017
 *
 * Mac C layer for 4coder
 *
 */

// TOP

#define WINDOW_TITLE "4coder"

#include "4tech_defines.h"
#include "4coder_API/version.h"

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
} OSX_Keyboard_Modifiers;

typedef struct OSX_Vars{
	i32 width, height;
	b32 running;
	u32 key_count;
	u32 keys[8];

	u32 prev_clipboard_change_count;
	b32 has_clipboard_item;
	void *clipboard_data;
	umem clipboard_size, clipboard_max;
	b32 just_posted_to_clipboard;
} OSX_Vars;

internal OSX_Vars osx;

internal void
osx_post_to_clipboard(char *str);

// TODO(allen): Implement a real allocate
#include <stdlib.h>
internal void*
osx_allocate(umem size){
	void *result = malloc(size);
	return(result);
}

internal void
osx_resize(int width, int height){
	osx.width = width;
	osx.height = height;
	// TODO
}

internal void
osx_character_input(u32 code, OSX_Keyboard_Modifiers modifier_flags){
	// TODO
}

internal void
osx_mouse(i32 mx, i32 my, u32 type){
	// TODO
}

internal void
osx_mouse_wheel(float dx, float dy){
	// TODO
}

internal void
osx_step(){
	// TODO
}

internal void
osx_init(){
	// TODO
}

// BOTTOM

