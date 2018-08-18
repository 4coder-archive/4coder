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

// 4tech_standard_preamble.h
#if !defined(FTECH_INTEGERS)
#define FTECH_INTEGERS
#include <stdint.h>
typedef int8_t i8_4tech;
typedef int16_t i16_4tech;
typedef int32_t i32_4tech;
typedef int64_t i64_4tech;

typedef uint8_t u8_4tech;
typedef uint16_t u16_4tech;
typedef uint32_t u32_4tech;
typedef uint64_t u64_4tech;

#if defined(FTECH_32_BIT)
typedef u32_4tech umem_4tech;
#else
typedef u64_4tech umem_4tech;
#endif

typedef float f32_4tech;
typedef double f64_4tech;

typedef int8_t b8_4tech;
typedef int32_t b32_4tech;
#endif

#if !defined(Assert)
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif
// standard preamble end 

struct Partition{
    char *base;
    i32_4tech pos;
    i32_4tech max;
};

struct Temp_Memory{
    void *handle;
    i32_4tech pos;
};

struct Tail_Temp_Partition{
    Partition part;
    void *handle;
    i32_4tech old_max;
};

#endif

// BOTTOM

