/*
 * 4coder_profile_inspect.h - Built in self profiling UI.
 */

// TOP

#if !defined(FCODER_PROFILE_INSPECT_H)
#define FCODER_PROFILE_INSPECT_H

struct Profile_Group_Ptr{
    Profile_Group_Ptr *next;
    Profile_Group *group;
};

struct Profile_Universal_Slot{
    Profile_Universal_Slot *next;
    
    String_Const_u8 source_location;
    u32 slot_index;
    
    String_Const_u8 name;
    
    u32 count;
    u64 total_time;
};

struct Profile_Thread{
    Profile_Thread *next;
    i32 thread_id;
    
    Profile_Group_Ptr *first_group;
    Profile_Group_Ptr *last_group;
    
    Profile_Universal_Slot *first_slot;
    Profile_Universal_Slot *last_slot;
    i32 slot_count;
    
    Profile_Universal_Slot **sorted_slots;
};

struct Profile_Inspection{
    Profile_Thread *thread_first;
    Profile_Thread *thread_last;
    i32 thread_count;
};

global Profile_Inspection global_profile_inspection = {};

#endif

// TOP

