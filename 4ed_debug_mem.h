/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.14.2017
 *
 * 
 *
 */

// TOP

#ifndef FED_DEBUG_MEM_H
#define FED_DEBUG_MEM_H

// NOTE(allen): revent any future instantiation of 4coder_mem.h
#define FCODER_MEM_H

#if !defined(OS_PAGE_SIZE)
#define OS_PAGE_SIZE 4096
#endif

struct Debug_GM{
    System_Functions *system;
};

static void
debug_gm_open(System_Functions *system, Debug_GM *general, void *memory, i32 size){
    general->system = system;
}

static void*
debug_gm_allocate(Debug_GM *general, int32_t size){
    System_Functions *system = general->system;
    persist u32 round_val = OS_PAGE_SIZE-1;
    size = (size + round_val) & (~round_val);
    void *result = system->memory_allocate(0, size + OS_PAGE_SIZE);
    system->memory_set_protection(0, (u8*)result + size, OS_PAGE_SIZE, 0);
    return(result);
}

static void
debug_gm_free(Debug_GM *general, void *memory){
    System_Functions *system = general->system;
    system->memory_free(0, memory, 0);
}

static void*
debug_gm_reallocate(Debug_GM *general, void *old, int32_t old_size, int32_t size){
    void *result = debug_gm_allocate(general, size);
    memcpy(result, old, old_size);
    debug_gm_free(general, old);
    return(result);
}

inline void*
debug_gm_reallocate_nocopy(Debug_GM *general, void *old, int32_t size){
    debug_gm_free(general, old);
    void *result = debug_gm_allocate(general, size);
    return(result);
}

// Redefine the normal general memory names to go to these debug names.
#define General_Memory Debug_GM
struct Debug_Mem_Options{
    Partition part;
    General_Memory general;
};
#define Mem_Options Debug_Mem_Options
#define general_memory_open(sys,gen,mem,siz) debug_gm_open(sys,gen,mem,siz)
#define general_memory_allocate(gen,siz) debug_gm_allocate(gen,siz)
#define general_memory_free(gen,siz) debug_gm_free(gen,siz)
#define general_memory_reallocate(gen,old,old_size,new_size) debug_gm_reallocate(gen,old,old_size,new_size)
#define general_memory_reallocate_nocopy(gen,old,new_size) debug_gm_reallocate_nocopy(gen,old,new_size)

#endif

// BOTTOM

