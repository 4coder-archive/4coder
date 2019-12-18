/*
 * 4coder app links base allocator
 */

// TOP

Scratch_Block::Scratch_Block(Application_Links *app, Scratch_Share_Code share){
    scratch_block__init(this, get_thread_context(app), share);
}

Scratch_Block::Scratch_Block(Application_Links *app){
    scratch_block__init(this, get_thread_context(app), share_code_default);
}

////////////////////////////////

internal Arena*
reserve_arena(Application_Links *app, u64 chunk_size, u64 align){
    Thread_Context *tctx = get_thread_context(app);
    return(reserve_arena(tctx, chunk_size, align));
}

internal Arena*
reserve_arena(Application_Links *app, u64 chunk_size){
    Thread_Context *tctx = get_thread_context(app);
    return(reserve_arena(tctx, chunk_size));
}

internal Arena*
reserve_arena(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    return(reserve_arena(tctx));
}

internal void
release_arena(Application_Links *app, Arena *arena){
    Thread_Context *tctx = get_thread_context(app);
    release_arena(tctx, arena);
}

// BOTTOM

