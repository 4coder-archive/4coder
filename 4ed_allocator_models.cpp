/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 27.08.2019
 *
 * Models based arena constructors
 *
 */

// TOP

internal Arena*
reserve_arena(Models *models, umem chunk_size, umem align){
    Thread_Context *tctx = models->tctx;
    return(reserve_arena(tctx, chunk_size, align));
}

internal Arena*
reserve_arena(Models *models, umem chunk_size){
    Thread_Context *tctx = models->tctx;
    return(reserve_arena(tctx, chunk_size));
}

internal Arena*
reserve_arena(Models *models){
    Thread_Context *tctx = models->tctx;
    return(reserve_arena(tctx));
}

internal void
release_arena(Models *models, Arena *arena){
    Thread_Context *tctx = models->tctx;
    release_arena(tctx, arena);
}

// BOTTOM

