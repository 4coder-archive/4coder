/*
 * 4coder_profile.cpp - Built in self profiling report.
 */

// TOP

function Profile_Group*
make_profile_group__inner(u32 slot_count, u32 stack_size, char *source_location){
    Arena arena = make_arena_system(KB(4));
    Profile_Group *group = push_array(&arena, Profile_Group, 1);
    group->arena = arena;
    group->thread_id = system_thread_get_id();
    group->source_location = SCu8(source_location);
    group->slot_names = push_array_zero(&group->arena, String_Const_u8, slot_count);
    group->timer_stack = push_array_zero(&group->arena, Profile_Slot, stack_size);
    group->slot_count = slot_count;
    group->stack_size = stack_size;
    group->stack_top = 0;
    group->first = 0;
    group->last = 0;
    return(group);
}

#define make_profile_group(L,T) make_profile_group__inner((L),(T), file_name_line_number)

function void
profile_define(Profile_Group *group, u32 slot_index, char *name){
    if (slot_index < group->slot_count){
        group->slot_names[slot_index] = SCu8(name);
    }
}

function void
profile__record(Profile_Group *group, u32 slot_index, u64 time){
    Profile_Record *record = push_array(&group->arena, Profile_Record, 1);
    record->slot_index = slot_index;
    record->time = time;
    sll_queue_push(group->first, group->last, record);
}

function void
profile_begin_range(Profile_Group *group, u32 slot_index){
    Assert(group->stack_top < group->stack_size);
    Profile_Slot *slot = &group->timer_stack[group->stack_top];
    Profile_Slot *prev = 0;
    if (group->stack_top > 0){
        prev = &group->timer_stack[group->stack_top - 1];
    }
    slot->slot_index = slot_index;
    group->stack_top += 1;
    slot->accumulated_time = 0;
    u64 time = system_now_time();
    slot->start_time = time;
    if (prev != 0){
        prev->accumulated_time += time - prev->start_time;
    }
}

function void
profile_end_range(Profile_Group *group){
    u64 time = system_now_time();
    Assert(group->stack_top > 0);
    group->stack_top -= 1;
    if (group->stack_top > 0){
        Profile_Slot *prev = &group->timer_stack[group->stack_top - 1];
        prev->start_time = time;
    }
    Profile_Slot *slot = &group->timer_stack[group->stack_top];
    u64 accumulated = time - slot->start_time + slot->accumulated_time;
    profile__record(group, slot->slot_index, accumulated);
}

function void
profile_add_note(Profile_Group *group, u32 slot_index){
    profile__record(group, slot_index, 0);
}

function void
profile_group_post(Profile_Group *group){
    Assert(group->stack_top == 0);
    system_mutex_acquire(profile_history.mutex);
    if (profile_history.disable_bits == 0){
        sll_queue_push(profile_history.first, profile_history.last, group);
    }
    else{
        Arena *arena = &group->arena;
        linalloc_clear(arena);
    }
    system_mutex_release(profile_history.mutex);
}

function void
profile_history_set_enabled(b32 value, Profile_Enable_Flag flag){
    system_mutex_acquire(profile_history.mutex);
    if (value){
        RemFlag(profile_history.disable_bits, flag);
    }
    else{
        AddFlag(profile_history.disable_bits, flag);
    }
    system_mutex_release(profile_history.mutex);
}

function void
profile_history_clear(void){
    system_mutex_acquire(profile_history.mutex);
    for (Profile_Group *node = profile_history.first, *next = 0;
         node != 0;
         node = next){
        next  = node->next;
        Arena *arena = &node->arena;
        linalloc_clear(arena);
    }
    system_mutex_release(profile_history.mutex);
}

function void
profile_history_init(void){
    profile_history.mutex = system_mutex_make();
    profile_history.disable_bits = 0;
}

////////////////////////////////

CUSTOM_COMMAND_SIG(profile_enable)
CUSTOM_DOC("Allow 4coder's self profiler to gather new profiling information.")
{
    profile_history_set_enabled(true, ProfileEnable_UserBit);
}

CUSTOM_COMMAND_SIG(profile_disable)
CUSTOM_DOC("Prevent 4coder's self profiler from gathering new profiling information.")
{
    profile_history_set_enabled(false, ProfileEnable_UserBit);
}

CUSTOM_COMMAND_SIG(profile_clear)
CUSTOM_DOC("Clear all profiling information from 4coder's self profiler.")
{
    profile_history_clear();
}

// BOTTOM
