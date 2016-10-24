
#ifndef FED_MEM_H
#define FED_MEM_H

#include "4coder_mem.h"

//#define MEMORY_DEBUG

static void
general_memory_open(System_Functions *system, General_Memory *general, void *memory, int32_t size){
#if defined(MEMORY_DEBUG)
    general_memory_open(general, memory, size);
    #else
    general_memory_open(general, memory, size);
    #endif
}

#if defined(Assert) && !defined(MEMORY_DEBUG)
static int32_t
general_memory_check(System_Functions *system, General_Memory *general){
    Bubble *sentinel = &general->sentinel;
    for (Bubble *bubble = sentinel->next;
         bubble != sentinel;
         bubble = bubble->next){
        Assert(bubble);
        
        Bubble *next = bubble->next;
        Assert(bubble == next->prev);
        if (next != sentinel && bubble->prev != sentinel){
            Assert(bubble->next > bubble);
            Assert(bubble > bubble->prev);
            
            char *end_ptr = (char*)(bubble + 1) + bubble->size;
            char *next_ptr = (char*)next;
            (void)(end_ptr);
            (void)(next_ptr);
            Assert(end_ptr == next_ptr);
        }
    }
    return(1);
}
#else
static int32_t
general_memory_check(System_Functions *system, General_Memory *general){return(1);}
#endif

#define OS_PAGE_SIZE 4096

static void*
general_memory_allocate(System_Functions *system, General_Memory *general, int32_t size){
#if defined(MEMORY_DEBUG)
    {
        persist u32 round_val = OS_PAGE_SIZE-1;
        size = (size + round_val) & (~round_val);
        void *result = system->memory_allocate(0, size + OS_PAGE_SIZE);
        system->memory_set_protection(0, (u8*)result + size, OS_PAGE_SIZE, 0);
    return(result);
    }
#else
    return general_memory_allocate(general, size);
#endif
}

static void
general_memory_free(System_Functions *system, General_Memory *general, void *memory){
#if defined(MEMORY_DEBUG)
    {
        system->memory_free(0, memory, 0);
    }
#else
    return general_memory_free(general, memory);
#endif
}

static void*
general_memory_reallocate(System_Functions *system, General_Memory *general, void *old, int32_t old_size, int32_t size){
#if defined(MEMORY_DEBUG)
    {
    void *result = general_memory_allocate(system, general, size);
        memcpy(result, old, old_size);
        general_memory_free(system, general, old);
    return(result);
}
#else
    return general_memory_reallocate(general, old, old_size, size);
#endif
}

inline void*
general_memory_reallocate_nocopy(System_Functions *system, General_Memory *general, void *old, int32_t size){
#if defined(MEMORY_DEBUG)
    {
    general_memory_free(system, general, old);
    return general_memory_allocate(system, general, size);
    }
#else
    return general_memory_reallocate_nocopy(general, old, size);
#endif
}

#define reset_temp_memory end_temp_memory
#define gen_struct(s, g, T) (T*)general_memory_allocate(s, g, sizeof(T), 0)
#define gen_array(s, g, T, size) (T*)general_memory_allocate(s, g, sizeof(T)*(size))
#define gen_block(s, g, size) general_memory_open(s, g, size, 0)

#endif

