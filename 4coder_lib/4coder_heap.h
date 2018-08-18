/*
4coder_heap.h - Preversioning
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.
*/

// TOP

#if !defined(FCODER_HEAP_H)
#define FCODER_HEAP_H

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

#if !defined(Member)
# define Member(T, m) (((T*)0)->m)
#endif

#if !defined(PtrDif)
#define PtrDif(a,b) ((uint8_t*)(a) - (uint8_t*)(b))
#endif

#if !defined(PtrAsInt)
#define PtrAsInt(a) PtrDif(a,0)
#endif

#if !defined(OffsetOfMember)
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#endif

#if !defined(CastFromMember)
#define CastFromMember(S,m,ptr) (S*)( (uint8_t*)(ptr) - OffsetOfMember(S,m) )
#endif
// standard preamble end 

struct Heap_Basic_Node{
    Heap_Basic_Node *next;
    Heap_Basic_Node *prev;
};

struct Heap_Node{
    union{
        struct{
            Heap_Basic_Node order;
            Heap_Basic_Node alloc;
            i32_4tech size;
        };
        u8_4tech force_size__[64];
    };
};

struct Heap{
    Heap_Basic_Node in_order;
    Heap_Basic_Node free_nodes;
};

#endif

// BOTTOM

