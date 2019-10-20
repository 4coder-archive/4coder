/*
 * 4coder_profile_inspect.h - Built in self profiling UI.
 */

// TOP

#if !defined(FCODER_PROFILE_INSPECT_H)
#define FCODER_PROFILE_INSPECT_H

struct Profile_Slot{
    Profile_Slot *next;
    String_Const_u8 location;
    String_Const_u8 name;
    
    b32 corrupted_time;
    u64 total_time;
    i32 hit_count;
};

struct Profile_Node{
    Profile_Node *next;
    Profile_Slot *slot;
    Range_u64 time;
    Profile_ID id;
    
    Profile_Node *first_child;
    Profile_Node *last_child;
    i32 child_count;
    
    b32 closed;
};

struct Profile_Inspection_Thread{
    i32 thread_id;
    Profile_Node root;
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
    ProfileInspectTab_Slots,
    ProfileInspectTab_Errors
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
    Profile_Inspection_Tab tab_id_hovered;
    
    String_Const_u8 location_jump_hovered;
};

global Profile_Inspection global_profile_inspection = {};

#endif

// TOP

