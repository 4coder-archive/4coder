/*
 * 4coder malloc base allocator
 */

// TOP

#include <stdlib.h>
#include <malloc.h>

internal void*
base_reserve__malloc(void *user_data, umem size, umem *size_out){
    *size_out = size;
    return(malloc(size));
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

internal Arena
make_arena_malloc(umem chunk_size, umem align){
    if (malloc_base_allocator.reserve == 0){
        malloc_base_allocator = make_malloc_base_allocator();
    }
    return(make_arena(&malloc_base_allocator, chunk_size, align));
}

internal Arena
make_arena_malloc(umem chunk_size){
    return(make_arena_malloc(chunk_size, 8));
}

internal Arena
make_arena_malloc(void){
    return(make_arena_malloc(KB(16), 8));
}

// BOTTOM

