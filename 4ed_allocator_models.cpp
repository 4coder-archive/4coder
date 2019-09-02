/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 27.08.2019
 *
 * Models based arena constructors
 *
 */

// TOP

internal Arena
make_arena_models(Models *models, umem chunk_size, umem align){
    return(make_arena(models->base_allocator, chunk_size, align));
}

internal Arena
make_arena_models(Models *models, umem chunk_size){
    return(make_arena(models->base_allocator, chunk_size, 8));
}

internal Arena
make_arena_models(Models *models){
    return(make_arena(models->base_allocator, KB(16), 8));
}

// BOTTOM

