/*
 * 4coder malloc base allocator
 */

// TOP

internal void*
base_reserve__system(void *user_data, umem size, umem *size_out){
    System_Functions *system = (System_Functions*)user_data;
    umem extra_size = 128;
    umem increased_size = size + extra_size;
    size = round_up_umem(increased_size, KB(4));
    *size_out = size - extra_size;
    void *ptr = system->memory_allocate(size);
    *(umem*)ptr = size;
    ptr = (u8*)ptr + extra_size;
    return(ptr);
}

internal void
base_free__system(void *user_data, void *ptr){
    System_Functions *system = (System_Functions*)user_data;
    umem extra_size = 128;
    ptr = (u8*)ptr - extra_size;
    umem size = *(umem*)ptr;
    system->memory_free(ptr, size);
}

internal Base_Allocator
make_base_allocator_system(System_Functions *system){
    return(make_base_allocator(base_reserve__system, 0, 0,
                               base_free__system, 0, system));
}

global Base_Allocator base_allocator_system = {};

internal Base_Allocator*
get_base_allocator_system(System_Functions *system){
    if (base_allocator_system.reserve == 0){
        base_allocator_system = make_base_allocator_system(system);
    }
    return(&base_allocator_system);
}

internal Arena
make_arena_system(System_Functions *system, umem chunk_size, umem align){
    return(make_arena(get_base_allocator_system(system), chunk_size, align));
}

internal Arena
make_arena_system(System_Functions *system, umem chunk_size){
    return(make_arena_system(system, chunk_size, 8));
}

internal Arena
make_arena_system(System_Functions *system){
    return(make_arena_system(system, KB(16), 8));
}

// BOTTOM

