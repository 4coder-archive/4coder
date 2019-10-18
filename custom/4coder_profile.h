/*
 * 4coder_profile.h - Types for built in self profiling report.
 */

// TOP

#if !defined(FCODER_PROFILE_H)
#define FCODER_PROFILE_H

struct Profile_Slot{
    u64 start_time;
    u64 accumulated_time;
    u32 slot_index;
};

struct Profile_Record{
    Profile_Record *next;
    struct Profile_Group *group;
    u64 time;
    u32 slot_index;
};

struct Profile_Group{
    Profile_Group *next;
    
    Arena arena;
    
    i32 thread_id;
    String_Const_u8 source_location;
    String_Const_u8 *slot_names;
    Profile_Slot *timer_stack;
    u32 slot_count;
    u32 stack_size;
    u32 stack_top;
    
    Profile_Record *first;
    Profile_Record *last;
};

////////////////////////////////

typedef u32 Profile_Enable_Flag;
enum{
    ProfileEnable_UserBit    = 0x1,
    ProfileEnable_InspectBit = 0x2
};

struct Profile_History{
    System_Mutex mutex;
    Profile_Enable_Flag disable_bits;
    Profile_Group *first;
    Profile_Group *last;
};

global Profile_History profile_history = {};

#endif

// BOTTOM

