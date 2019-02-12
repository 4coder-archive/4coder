/*
* Mr. 4th Dimention - Allen Webster
*
* 07.02.2019
*
* Memory bank wrapper for heap
*
*/

// TOP

#if !defined(FRED_MEMORY_BANK_H)
#define FRED_MEMORY_BANK_H

struct Memory_Header{
    Memory_Header *next;
};

struct Memory_Bank{
    Heap heap;
    Memory_Header *first;
    Memory_Header *last;
    umem total_memory_size;
};

#endif

// BOTTOM

