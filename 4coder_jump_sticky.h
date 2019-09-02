/*
4coder_jump_sticky.h - Types for persistant jump positions.
*/

// TOP

#if !defined(FCODER_JUMP_STICKY_H)
#define FCODER_JUMP_STICKY_H

struct Sticky_Jump{
    i64 list_line;
    i64 list_colon_index;
    b32 is_sub_error;
    Buffer_ID jump_buffer_id;
    i64 jump_pos;
};

struct Sticky_Jump_Stored{
    i64 list_line;
    i64 list_colon_index;
    b32 is_sub_error;
    Buffer_ID jump_buffer_id;
    i32 index_into_marker_array;
};

struct Sticky_Jump_Node{
    Sticky_Jump_Node *next;
    Sticky_Jump jump;
};

struct Sticky_Jump_Array{
    Sticky_Jump *jumps;
    i32 count;
};

struct Sticky_Jump_Node_Header{
    Managed_Object memory;
    Managed_Object markers;
    i32 first_index;
    i32 count;
};

enum Jump_Location_Flag{
    JumpFlag_IsSubJump = 0x1,
};

struct Marker_List{
    Managed_Object jump_array;
    i32 jump_count;
    i32 previous_size;
    Buffer_ID buffer_id;
};

struct Marker_List_Node{
    Marker_List_Node *next;
    Marker_List_Node *prev;
    Marker_List list;
    Buffer_ID buffer_id;
};

struct Locked_Jump_State{
    View_ID view;
    Marker_List *list;
    i32 list_index;
};

#endif

// BOTTOM

