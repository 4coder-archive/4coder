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

#if SOFTWARE_RENDER
struct Render_Target{
	void *pixel_data;
	i32 width, height, pitch;
};
#else
struct Render_Target{
    void *handle;
    void *context;
    i32_Rect clip_boxes[5];
    i32 clip_top;
	i32 width, height;
    i32 bound_texture;
    u32 color;
};
#endif

struct Application_Memory{
    void *vars_memory;
    i32 vars_memory_size;
    void *target_memory;
    i32 target_memory_size;
};

#define KEY_INPUT_BUFFER_SIZE 4
#define KEY_INPUT_BUFFER_DSIZE (KEY_INPUT_BUFFER_SIZE << 1)

enum Key_Control{
	CONTROL_KEY_SHIFT,
	CONTROL_KEY_CONTROL,
	CONTROL_KEY_ALT,
	// always last
	CONTROL_KEY_COUNT
};

struct Key_Event_Data{
	u16 keycode;
	u16 loose_keycode;
	u16 character;
	u16 character_no_caps_lock;
};

struct Key_Input_Data{
	Key_Event_Data press[KEY_INPUT_BUFFER_SIZE];
	Key_Event_Data hold[KEY_INPUT_BUFFER_SIZE];
	i32 press_count;
    i32 hold_count;

	b8 control_keys[CONTROL_KEY_COUNT];
	b8 caps_lock;
};

struct Key_Summary{
    i32 count;
    Key_Event_Data keys[KEY_INPUT_BUFFER_DSIZE];
	bool8 modifiers[CONTROL_KEY_COUNT];
};

struct Key_Single{
    Key_Event_Data key;
    b8 *modifiers;
};

inline Key_Single
get_single_key(Key_Summary *summary, i32 index){
    Assert(index >= 0 && index < summary->count);
    Key_Single key;
    key.key = summary->keys[index];
    key.modifiers = summary->modifiers;
    return key;
}

struct Mouse_State{
	bool32 out_of_window;
	bool32 left_button, right_button;
	bool32 left_button_prev, right_button_prev;
	i32 x, y;
	i16 wheel;
};

struct Mouse_Summary{
    i32 mx, my;
    bool32 l, r;
    bool32 press_l, press_r;
    bool32 release_l, release_r;
    bool32 out_of_window;
    bool32 wheel_used;
    i16 wheel_amount;
};

struct Input_Summary{
    Mouse_Summary mouse;
    Key_Summary keys;
    Key_Codes *codes;
};

// TODO(allen): This can go, and we can just use a String for it.
struct Clipboard_Contents{
	u8 *str;
	i32 size;
};

struct Thread_Context;

#define App_Init_Sig(name)                                          \
    b32 name(System_Functions *system,                              \
             Thread_Context *thread, Application_Memory *memory,    \
             Key_Codes *loose_codes, Clipboard_Contents clipboard,  \
             Config_API api)

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
	bool32 redraw;
};

#define App_Step_Sig(name) Application_Step_Result          \
    name(System_Functions *system,                          \
         Thread_Context *thread, Key_Codes *codes,          \
         Key_Input_Data *input, Mouse_State *mouse,         \
         b32 time_step, Render_Target *target,              \
         Application_Memory *memory,                        \
         Clipboard_Contents clipboard,                      \
         b32 first_step, b32 force_redraw)

typedef App_Step_Sig(App_Step);

struct App_Functions{
    App_Init *init;
    App_Step *step;
};

#endif

// BOTTOM

