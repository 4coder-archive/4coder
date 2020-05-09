/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.07.2017
 *
 * File editing view for 4coder.
 *
 */

// TOP

#if !defined(FRED_VIEW_H)
#define FRED_VIEW_H

struct Co_In{
    union{
        struct{
            struct Models *models;
            Custom_Command_Function *event_context_base;
        };
        User_Input user_input;
        Face_ID face_id;
        b32 success;
    };
};

typedef i32 Co_Request;
enum{
    CoRequest_None = 0,
    CoRequest_NewFontFace = 1,
    CoRequest_ModifyFace = 2,
    CoRequest_AcquireGlobalFrameMutex = 3,
    CoRequest_ReleaseGlobalFrameMutex = 4,
};

struct Co_Out{
    Co_Request request;
    Face_Description *face_description;
    Face_ID face_id;
};

struct Query_Slot{
    Query_Slot *next;
    Query_Bar *query_bar;
};

struct Query_Set{
    Query_Slot slots[8];
    Query_Slot *free_slot;
    Query_Slot *used_slot;
};

struct View_Context_Node{
    View_Context_Node *next;
    Temp_Memory pop_me;
    View_Context ctx;
    void *delta_rule_memory;
};

struct View{
    View *next;
    View *prev;
    struct Panel *panel;
    b32 in_use;
    
    Editing_File *file;
    Lifetime_Object *lifetime_object;
    
    File_Edit_Positions edit_pos_;
    i64 mark;
    f32 preferred_x;
    Vec2_f32 cursor_margin;
    Vec2_f32 cursor_push_in_multiplier;
    
    b8 new_scroll_target;
    b8 hide_scrollbar;
    b8 hide_file_bar;
    b8 show_whitespace;
    
    Coroutine *co;
    Co_Out co_out;
    
    Arena node_arena;
    View_Context_Node *ctx;
    
    Query_Set query_set;
};

struct Live_Views{
    View *views;
    View free_sentinel;
    i32 count;
    i32 max;
};

#endif

// BOTTOM

