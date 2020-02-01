/*
 * 4coder malloc base allocator
 */

// TOP

#include <stdlib.h>

#if !OS_MAC
# include <malloc.h>
#endif

internal void*
base_reserve__malloc(void *user_data, u64 size, u64 *size_out, String_Const_u8 location){
    *size_out = size;
    return(malloc((size_t)size));
}

internal void
base_free__malloc(void *user_data, void *ptr){
    free(ptr);
}

internal Base_Allocator
make_malloc_base_allocator(void){
    return(make_base_allocator(base_reserve__malloc, 0, 0,
                               base_free__malloc, 0, 0));
}

global Base_Allocator malloc_base_allocator = {};

internal Base_Allocator*
get_allocator_malloc(void){
    if (malloc_base_allocator.reserve == 0){
        malloc_base_allocator = make_malloc_base_allocator();
    }
    return(&malloc_base_allocator);
}

internal Arena
make_arena_malloc(u64 chunk_size, u64 align){
    Base_Allocator *allocator = get_allocator_malloc();
    return(make_arena(allocator, chunk_size, align));
}

internal Arena
make_arena_malloc(u64 chunk_size){
    return(make_arena_malloc(chunk_size, 8));
}

internal Arena
make_arena_malloc(void){
    return(make_arena_malloc(KB(16), 8));
}

// BOTTOM

