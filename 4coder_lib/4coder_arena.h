/*
4coder_arena.h - Preversioning
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.
*/

// TOP

#if !defined(FCODER_ARENA_H)
#define FCODER_ARENA_H

#if !defined(Migrating__Arena)
struct Partition{
    char *base;
    i32 pos;
    i32 max;
};

struct Temp_Memory{
    Partition *part;
    i32 pos;
};

#if defined(FCODER_CUSTOM_H)

struct Partition_Chained{
    Partition_Chained *prev;
    Partition part;
};

struct Arena{
    struct Application_Links *app;
    Partition_Chained *part;
    i32 chunk_size;
    i32 align;
};

struct Temp_Memory_Arena{
    Arena *arena;
    Partition_Chained *part;
    i32 pos;
};

struct Temp_Memory_Arena_Light{
    Partition_Chained *part;
    i32 pos;
};

struct Scratch_Block{
    Scratch_Block(Application_Links *app);
    ~Scratch_Block();
    operator Arena *();
    
    Arena *scratch;
    Temp_Memory_Arena temp;
};

#endif

#endif

#endif

// BOTTOM

