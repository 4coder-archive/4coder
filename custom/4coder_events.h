/*
 * 4coder event types
 */

// TOP

#if !defined(FCODER_EVENTS_H)
#define FCODER_EVENTS_H

typedef void Custom_Command_Function(struct Application_Links *app);

typedef u32 Key_Code;
typedef u32 Mouse_Code;
typedef u32 Core_Code;
#include "generated/4coder_event_codes.h"

typedef u32 Input_Event_Kind;
enum{
    InputEventKind_None,
    InputEventKind_TextInsert,
    InputEventKind_KeyStroke,
    InputEventKind_KeyRelease,
    InputEventKind_MouseButton,
    InputEventKind_MouseButtonRelease,
    InputEventKind_MouseWheel,
    InputEventKind_MouseMove,
    InputEventKind_Core,
    InputEventKind_CustomFunction,
    
    InputEventKind_COUNT,
};

typedef u32 Key_Flags;
enum{
    KeyFlag_IsDeadKey = (1 << 0),
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
    b32 virtual_event;
    union{
        struct{
            String_Const_u8 string;
            
            // used internally
            Input_Event *next_text;
            b32 blocked;
        } text;
        struct{
            Key_Code code;
            Key_Flags flags;
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
        Custom_Command_Function *custom_func;
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
    EventProperty_AnyKey         = 0x0001,
    EventProperty_Escape         = 0x0002,
    EventProperty_AnyKeyRelease  = 0x0004,
    EventProperty_MouseButton    = 0x0008,
    EventProperty_MouseRelease   = 0x0010,
    EventProperty_MouseWheel     = 0x0020,
    EventProperty_MouseMove      = 0x0040,
    EventProperty_Animate        = 0x0080,
    EventProperty_ViewActivation = 0x0100,
    EventProperty_TextInsert     = 0x0200,
    EventProperty_AnyFile        = 0x0400,
    EventProperty_Startup        = 0x0800,
    EventProperty_Exit           = 0x1000,
    EventProperty_Clipboard      = 0x2000,
    EventProperty_CustomFunction = 0x4000,
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
    EventPropertyGroup_AnyCore =
        EventProperty_Animate|
        EventProperty_ViewActivation|
        EventProperty_AnyFile|
        EventProperty_Startup|
        EventProperty_Exit|
        EventProperty_Clipboard|
        EventProperty_Animate,
    EventPropertyGroup_Any =
        EventPropertyGroup_AnyUserInput|
        EventPropertyGroup_AnyCore|
        EventProperty_CustomFunction,
};

#endif

// BOTTOM
