/*
 * 4coder malloc base allocator
 */

// TOP

internal void*
base_reserve__system(void *user_data, umem size, umem *size_out, String_Const_u8 location){
    umem extra_size = 128;
    umem increased_size = size + extra_size;
    size = round_up_umem(increased_size, KB(4));
    *size_out = size - extra_size;
    void *ptr = system_memory_allocate(size, location);
    *(umem*)ptr = size;
    ptr = (u8*)ptr + extra_size;
    return(ptr);
}

internal void
base_free__system(void *user_data, void *ptr){
    umem extra_size = 128;
    ptr = (u8*)ptr - extra_size;
    umem size = *(umem*)ptr;
    system_memory_free(ptr, size);
}

internal Base_Allocator
make_base_allocator_system(void){
    return(make_base_allocator(base_reserve__system, 0, 0,
                               base_free__system, 0, 0));
}

global Base_Allocator base_allocator_system = {};

internal Base_Allocator*
get_base_allocator_system(void){
    if (base_allocator_system.reserve == 0){
        base_allocator_system = make_base_allocator_system();
    }
    return(&base_allocator_system);
}

internal Arena
make_arena_system(umem chunk_size, umem align){
    return(make_arena(get_base_allocator_system(), chunk_size, align));
}

internal Arena
make_arena_system(umem chunk_size){
    return(make_arena_system(chunk_size, 8));
}

internal Arena
make_arena_system(void){
    return(make_arena_system(KB(16), 8));
}

// BOTTOM

