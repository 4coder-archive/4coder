/*
 * 4coder_profile_inspect.h - Built in self profiling UI.
 */

// TOP

#if !defined(FCODER_PROFILE_INSPECT_H)
#define FCODER_PROFILE_INSPECT_H

struct Profile_Node_Ptr{
    Profile_Node_Ptr *next;
    struct Profile_Node *ptr;
};

struct Profile_Slot{
    Profile_Slot *next;
    String_Const_u8 location;
    String_Const_u8 name;
    
    u64 total_time;
    b32 corrupted_time;
    
    i32 hit_count;
    Profile_Node_Ptr *first_hit;
    Profile_Node_Ptr *last_hit;
};

struct Profile_Node{
    Profile_Node *next;
    Profile_Node *parent;
    Profile_Slot *slot;
    struct Profile_Inspection_Thread *thread;
    Range_u64 time;
    Profile_ID id;
    u64 unique_counter;
    
    Profile_Node *first_child;
    Profile_Node *last_child;
    i32 child_count;
    
    b32 closed;
};

struct Profile_Inspection_Thread{
    i32 thread_id;
    String_Const_u8 name;
    Profile_Node root;
    u64 active_time;
};

struct Profile_Error{
    Profile_Error *next;
    String_Const_u8 message;
    String_Const_u8 location;
};

typedef i32 Profile_Inspection_Tab;
enum{
    ProfileInspectTab_None,
    ProfileInspectTab_Threads,
    ProfileInspectTab_Blocks,
    ProfileInspectTab_Errors,
    ProfileInspectTab_Memory,
    ProfileInspectTab_Selection,
};

struct Profile_Inspection{
    Profile_Slot *first_slot;
    Profile_Slot *last_slot;
    Profile_Error *first_error;
    Profile_Error *last_error;
    Profile_Inspection_Thread *threads;
    i32 slot_count;
    i32 thread_count;
    i32 error_count;
    
    Profile_Inspection_Tab tab_id;
    Profile_Inspection_Thread *selected_thread;
    Profile_Slot *selected_slot;
    Profile_Node *selected_node;
    
    Profile_Inspection_Tab tab_id_hovered;
    String_Const_u8 full_name_hovered;
    u64 unique_counter_hovered;
    String_Const_u8 location_jump_hovered;
    Profile_Inspection_Thread *hover_thread;
    Profile_Slot *hover_slot;
    Profile_Node *hover_node;
};

global Profile_Inspection global_profile_inspection = {};

struct Memory_Bucket{
    Memory_Bucket *next;
    Memory_Annotation annotation;
    String_Const_u8 location;
    u64 total_memory;
};

#endif

// TOP

