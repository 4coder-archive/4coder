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

struct Heap_Basic_Node{
    Heap_Basic_Node *next;
    Heap_Basic_Node *prev;
};

struct Heap_Node{
    union{
        struct{
            Heap_Basic_Node order;
            Heap_Basic_Node alloc;
            i32 size;
        };
        u8 force_size__[64];
    };
};

struct Heap{
    Heap_Basic_Node in_order;
    Heap_Basic_Node free_nodes;
    i32 used_space;
    i32 total_space;
};

//#define DO_HEAP_CHECKS

#endif

// BOTTOM

