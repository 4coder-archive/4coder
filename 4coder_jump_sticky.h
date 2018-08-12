/*
4coder_jump_sticky.h - Types for persistant jump positions.
*/

// TOP

#if !defined(FCODER_JUMP_STICKY_H)
#define FCODER_JUMP_STICKY_H

struct Sticky_Jump_Line_Details{
    int32_t list_line;
    int32_t list_colon_index;
    bool32 is_sub_error;
};

struct Sticky_Jump{
    int32_t list_line;
    int32_t list_colon_index;
    bool32 is_sub_error;
    Buffer_ID jump_buffer_id;
    int32_t jump_pos;
};

struct Sticky_Jump_Array{
    struct Sticky_Jump *jumps;
    int32_t count;
};

struct Sticky_Jump_Node_Header{
    Managed_Object memory;
    Managed_Object markers;
    int32_t first_index;
    int32_t count;
};

enum Jump_Location_Flag{
    JumpFlag_IsSubJump = 0x1,
};

struct Marker_List{
    int32_t previous_size;
};

struct Marker_List_Node{
    Marker_List_Node *next;
    Marker_List_Node *prev;
    Marker_List list;
    int32_t buffer_id;
};

struct Locked_Jump_State{
    View_Summary view;
    Marker_List *list;
    int32_t list_index;
};

#endif

// BOTTOM

