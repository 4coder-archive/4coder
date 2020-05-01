/*
 * 4coder app links base allocator
 */

// TOP

Scratch_Block::Scratch_Block(Application_Links *app){
    Thread_Context *t = this->tctx = get_thread_context(app);
    this->arena = tctx_reserve(t);
    this->temp = begin_temp(this->arena);
}

Scratch_Block::Scratch_Block(Application_Links *app, Arena *a1){
    Thread_Context *t = this->tctx = get_thread_context(app);
    this->arena = tctx_reserve(t, a1);
    this->temp = begin_temp(this->arena);
}

Scratch_Block::Scratch_Block(Application_Links *app, Arena *a1, Arena *a2){
    Thread_Context *t = this->tctx = get_thread_context(app);
    this->arena = tctx_reserve(t, a1, a2);
    this->temp = begin_temp(this->arena);
}

Scratch_Block::Scratch_Block(Application_Links *app, Arena *a1, Arena *a2, Arena *a3){
    Thread_Context *t = this->tctx = get_thread_context(app);
    this->arena = tctx_reserve(t, a1, a2, a3);
    this->temp = begin_temp(this->arena);
}

// BOTTOM

