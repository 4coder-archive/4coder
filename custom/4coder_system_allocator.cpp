/*
 * 4coder malloc base allocator
 */

// TOP

internal void*
base_reserve__system(void *user_data, u64 size, u64 *size_out, String_Const_u8 location){
    u64 extra_size = 128;
    u64 increased_size = size + extra_size;
    size = round_up_u64(increased_size, KB(4));
    *size_out = size - extra_size;
    void *ptr = system_memory_allocate(size, location);
    *(u64*)ptr = size;
    ptr = (u8*)ptr + extra_size;
    return(ptr);
}

internal void
base_free__system(void *user_data, void *ptr){
    u64 extra_size = 128;
    ptr = (u8*)ptr - extra_size;
    u64 size = *(u64*)ptr;
    system_memory_free(ptr, size);
}

internal Base_Allocator
make_base_allocator_system(void){
    return(make_base_allocator(base_reserve__system, 0, 0, base_free__system, 0, 0));
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
make_arena_system(u64 chunk_size, u64 align){
    return(make_arena(get_base_allocator_system(), chunk_size, align));
}

internal Arena
make_arena_system(u64 chunk_size){
    return(make_arena_system(chunk_size, 8));
}

internal Arena
make_arena_system(void){
    return(make_arena_system(KB(16), 8));
}

// BOTTOM

