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

typedef i32 Input_Event_Kind;
enum{
    InputEventKind_TextInsert,
    InputEventKind_KeyStroke,
    InputEventKind_MouseButton,
    InputEventKind_MouseWheel,
    InputEventKind_MouseMove,
    InputEventKind_Core,
};

typedef i32 Key_Modifier_Index;
enum{
    MDFR_SHIFT_INDEX,
    MDFR_CONTROL_INDEX,
    MDFR_ALT_INDEX,
    MDFR_COMMAND_INDEX,
    
    MDFR_INDEX_BINDABLE_COUNT,
    
    MDFR_CAPS_INDEX = MDFR_INDEX_BINDABLE_COUNT,
    MDFR_HOLD_INDEX,
    
    MDFR_INDEX_COUNT,
};

struct Key_Modifiers{
    b8 modifiers[MDFR_INDEX_COUNT];
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
            Key_Modifiers modifiers;
            
            // used internally
            Input_Event *first_dependent_text;
        } key;
        struct{
            Mouse_Code code;
            Key_Modifiers modifiers;
            Vec2_i32 p;
            b32 release;
        } mouse;
        struct{
            f32 value;
            Vec2_i32 p;
        } mouse_wheel;
        struct{
            Vec2_i32 p;
        } mouse_move;
        struct{
            Core_Code code;
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
    EventProperty_MouseLeft      = 0x4,
    EventProperty_MouseMiddle    = 0x8,
    EventProperty_MouseRight     = 0x10,
    EventProperty_MouseWheel     = 0x20,
    EventProperty_MouseMove      = 0x40,
    EventProperty_Animate        = 0x80,
    EventProperty_ViewActivation = 0x100,
    EventProperty_TextInsert     = 0x200,
};
enum{
    EventPropertyGroup_AnyKeyboardEvent =
        EventProperty_AnyKey|
        EventProperty_Escape|
        EventProperty_TextInsert,
    EventPropertyGroup_AnyMouseButton =
        EventProperty_MouseLeft|
        EventProperty_MouseMiddle|
        EventProperty_MouseRight,
    EventPropertyGroup_AnyMouseEvent =
        EventPropertyGroup_AnyMouseButton|
        EventProperty_MouseWheel|
        EventProperty_MouseMove,
    EventPropertyGroup_AnyUserInput =
        EventPropertyGroup_AnyKeyboardEvent|
        EventPropertyGroup_AnyMouseEvent,
};

#endif

// BOTTOM
