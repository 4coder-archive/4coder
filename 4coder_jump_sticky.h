/*
4coder_jump_sticky.h - Types for persistant jump positions.
*/

// TOP

#if !defined(FCODER_JUMP_STICKY_H)
#define FCODER_JUMP_STICKY_H

enum Jump_Location_Flag{
    JumpFlag_IsSubJump = 0x1,
};

struct Sticky_Jump_Destination_Array{
    uint32_t first_jump_index;
    Marker_Handle handle;
};

struct Sticky_Jump_Source{
    uint32_t line_number;
    uint32_t flags;
};

struct Marker_List{
    Sticky_Jump_Destination_Array *dst;
    int32_t dst_count;
    int32_t dst_max;
    
    Sticky_Jump_Source *jumps;
    int32_t jump_count;
    int32_t jump_max;
    
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

