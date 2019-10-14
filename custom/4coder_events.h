/*
 * 4coder event types
 */

// TOP

#if !defined(FCODER_EVENTS_H)
#define FCODER_EVENTS_H

typedef u32 Key_Code;
typedef u32 Mouse_Code;
typedef u32 Core_Code;
#include "generated/4coder_event_codes.h"

typedef u32 Input_Event_Kind;
enum{
    InputEventKind_TextInsert,
    InputEventKind_KeyStroke,
    InputEventKind_KeyRelease,
    InputEventKind_MouseButton,
    InputEventKind_MouseButtonRelease,
    InputEventKind_MouseWheel,
    InputEventKind_MouseMove,
    InputEventKind_Core,
};

global_const i32 Input_MaxModifierCount = 8;

struct Input_Modifier_Set{
    Key_Code *mods;
    i32 count;
};

struct Input_Modifier_Set_Fixed{
    Key_Code mods[Input_MaxModifierCount];
    i32 count;
};

struct Input_Event{
    Input_Event_Kind kind;
    union{
        struct{
            String_Const_u8 string;
            
            // used internally
            Input_Event *next_text;
            b32 blocked;
        } text;
        struct{
            Key_Code code;
            Input_Modifier_Set modifiers;
            
            // used internally
            Input_Event *first_dependent_text;
        } key;
        struct{
            Mouse_Code code;
            Vec2_i32 p;
            Input_Modifier_Set modifiers;
        } mouse;
        struct{
            f32 value;
            Vec2_i32 p;
            Input_Modifier_Set modifiers;
        } mouse_wheel;
        struct{
            Vec2_i32 p;
            Input_Modifier_Set modifiers;
        } mouse_move;
        struct{
            Core_Code code;
            union{
                String_Const_u8 string;
                i32 id;
                struct{
                    String_Const_u8_Array flag_strings;
                    String_Const_u8_Array file_names;
                };
            };
        } core;
    };
};

struct Input_Event_Node{
    Input_Event_Node *next;
    Input_Event event;
};

struct Input_List{
    Input_Event_Node *first;
    Input_Event_Node *last;
    i32 count;
};

typedef u32 Event_Property;
enum{
    EventProperty_AnyKey         = 0x1,
    EventProperty_Escape         = 0x2,
    EventProperty_AnyKeyRelease  = 0x4,
    EventProperty_MouseButton    = 0x8,
    EventProperty_MouseRelease   = 0x10,
    EventProperty_MouseWheel     = 0x20,
    EventProperty_MouseMove      = 0x40,
    EventProperty_Animate        = 0x80,
    EventProperty_ViewActivation = 0x100,
    EventProperty_TextInsert     = 0x200,
    EventProperty_AnyFile        = 0x400,
    EventProperty_Startup        = 0x800,
    EventProperty_Exit           = 0x1000,
    EventProperty_Clipboard      = 0x2000,
};
enum{
    EventPropertyGroup_AnyKeyboardEvent =
        EventProperty_AnyKey|
        EventProperty_Escape|
        EventProperty_AnyKeyRelease|
        EventProperty_TextInsert,
    EventPropertyGroup_AnyMouseEvent =
        EventProperty_MouseButton|
        EventProperty_MouseRelease|
        EventProperty_MouseWheel|
        EventProperty_MouseMove,
    EventPropertyGroup_AnyUserInput =
        EventPropertyGroup_AnyKeyboardEvent|
        EventPropertyGroup_AnyMouseEvent,
    EventPropertyGroup_Any =
        EventPropertyGroup_AnyUserInput|
        EventProperty_Animate|
        EventProperty_ViewActivation|
        EventProperty_AnyFile|
        EventProperty_Startup|
        EventProperty_Exit|
        EventProperty_Clipboard,
};

#endif

// BOTTOM
