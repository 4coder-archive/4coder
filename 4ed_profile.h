/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 29.03.2017
 *
 * Really basic profiling primitives. (not meant to stay for long)
 *
 */

// TOP

#if !defined(FRED_PROFILE_H)
#define FRED_PROFILE_H

#if FRED_INTERNAL && IS_MSVC

#include <intrin.h>
#include <stdio.h>

struct Profile_Group{
    char name[104];
    u64 cycle_start;
    u64 cycle_count;
    void *end;
};

struct Profile{
    u8 *buffer;
    u32 pos, max;
    
    Profile_Group *stack[64];
    u32 stack_top;
};
global Profile global_profile = {0};

internal Profile_Group*
push_group(char *name){
    Profile_Group *result = 0;
    if (global_profile.pos + sizeof(Profile_Group) <= global_profile.max){
        result = (Profile_Group*)(global_profile.buffer + global_profile.pos);
        global_profile.pos += sizeof(Profile_Group);
        u32 i = 0;
        for (; name[i] && i < sizeof(result->name)-1; ++i){
            result->name[i] = name[i];
        }
        result->name[i] = 0;
        result->cycle_start = __rdtsc();
    }
    return(result);
}

internal void
profile_init(void *mem, u32 max){
    global_profile.buffer = (u8*)mem;
    global_profile.max = max;
}

internal void
profile_begin_group(char *name){
    u32 stack_pos = global_profile.stack_top;
    if (stack_pos < ArrayCount(global_profile.stack)){
        global_profile.stack[stack_pos] = push_group(name);
        Assert(global_profile.stack[stack_pos] != 0);
        ++global_profile.stack_top;
    }
}

internal void
profile_end_group(){
    Assert(global_profile.stack_top > 0);
    u32 stack_pos = --global_profile.stack_top;
    if (stack_pos < ArrayCount(global_profile.stack)){
        Profile_Group *group = global_profile.stack[stack_pos];
        Assert(group != 0);
        group->cycle_count = __rdtsc() - group->cycle_start;
        group->end = global_profile.buffer + global_profile.pos;
    }
}

internal void
profile_begin_frame(){
    global_profile.pos = 0;
    global_profile.stack_top = 0;
    profile_begin_group("*frame*");
}

internal void
profile_end_frame(char *filename){
    profile_end_group();
    Assert(global_profile.stack_top == 0);
    
    FILE *file = fopen(filename, "ab");
    Assert(file != 0);
    fwrite(&global_profile.buffer, 8, 1, file);
    
    Profile_Group *first_group = (Profile_Group*)global_profile.buffer;
    fwrite(&first_group->end, 8, 1, file);
    
    fwrite(global_profile.buffer, 1, global_profile.pos, file);
    fclose(file);
}

internal Profile_Group*
profile_begin_resumable(char *name){
    Profile_Group *result = push_group(name);
    Assert(result != 0);
    result->cycle_count = 0;
    result->end = result + 1;
    return(result);
}

struct profile_scope{
    profile_scope(char *name){
        profile_begin_group(name);
    }
    
    ~profile_scope(){
        profile_end_group();
    }
};

#define PRFL_INIT(m,s) profile_init(m,s)

#define PRFL_BEGIN_FRAME() profile_begin_frame()
#define PRFL_END_FRAME(n) profile_end_frame(n)

#define PRFL_BEGIN_GROUP(n) profile_begin_group(#n)
#define PRFL_END_GROUP() profile_end_group()
#define PRFL_SCOPE_GROUP(n) profile_scope SCOPE_##n(#n)

#define PRFL_FUNC_GROUP() profile_scope SCOPE_FUNC(__FUNCTION__)

#define PRFL_BEGIN_RESUMABLE(n) Profile_Group *PRFLGRP_##n = profile_begin_resumable(#n)
#define PRFL_START_RESUMABLE(n) PRFLGRP_##n->cycle_start = __rdtsc()
#define PRFL_STOP_RESUMABLE(n) PRFLGRP_##n->cycle_count += __rdtsc() - PRFLGRP_##n->cycle_start
#define PRFL_END_RESUMABLE(n)

#else

#define PRFL_INIT(m,s)

#define PRFL_BEGIN_FRAME()
#define PRFL_END_FRAME(n)

#define PRFL_BEGIN_GROUP(n)
#define PRFL_END_GROUP()
#define PRFL_SCOPE_GROUP(n)

#define PRFL_FUNC_GROUP()

#define PRFL_BEGIN_RESUMABLE(n)
#define PRFL_START_RESUMABLE(n)
#define PRFL_STOP_RESUMABLE(n)
#define PRFL_END_RESUMABLE(n)

#endif

#endif

// BOTTOM


