/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.07.2017
 *
 * Coroutine implementation from thread+mutex+cv
 *
 */

// TOP

internal void
coroutine__pass_control(Coroutine *me, Coroutine *other,
                        Coroutine_State my_new_state, Coroutine_Pass_Control control){
    Assert(me->state == CoroutineState_Active);
    Assert(me->sys == other->sys);
    
    me->state = my_new_state;
    other->state = CoroutineState_Active;
    me->sys->active = other;
    system_condition_variable_signal(other->cv);
    if (control == CoroutinePassControl_BlockMe){
        for (;me->state != CoroutineState_Active;){
            system_condition_variable_wait(me->cv, me->sys->lock);
        }
    }
}

internal void
coroutine_main(void *ptr){
    Coroutine *me = (Coroutine*)ptr;
    
    Thread_Context_Extra_Info tctx_info = {};
    tctx_info.coroutine = me;
    
    Thread_Context tctx_ = {};
    thread_ctx_init(&tctx_, ThreadKind_MainCoroutine,
                    get_base_allocator_system(), get_base_allocator_system());
    tctx_.user_data = &tctx_info;
    me->tctx = &tctx_;
    
    // NOTE(allen): Init handshake
    Assert(me->state == CoroutineState_Dead);
    system_mutex_acquire(me->sys->lock);
    me->sys->did_init = true;
    system_condition_variable_signal(me->sys->init_cv);
    
    for (;;){
        // NOTE(allen): Wait until someone wakes us up, then go into our procedure.
        for (;me->state != CoroutineState_Active;){
            system_condition_variable_wait(me->cv, me->sys->lock);
        }
        Assert(me->type != CoroutineType_Root);
        Assert(me->yield_ctx != 0);
        Assert(me->func != 0);
        
        me->func(me);
        
        // NOTE(allen): Wake up the caller and set this coroutine back to being dead.
        Coroutine *other = me->yield_ctx;
        Assert(other != 0);
        Assert(other->state == CoroutineState_Waiting);
        
        coroutine__pass_control(me, other, CoroutineState_Dead, CoroutinePassControl_ExitMe);
        me->func = 0;
    }
}

internal void
coroutine_sub_init(Coroutine *co, Coroutine_Group *sys){
    block_zero_struct(co);
    co->sys = sys;
    co->state = CoroutineState_Dead;
    co->type = CoroutineType_Sub;
    co->cv = system_condition_variable_make();
    sys->did_init = false;
    co->thread = system_thread_launch(coroutine_main, co);
    for (;!sys->did_init;){
        system_condition_variable_wait(sys->init_cv, sys->lock);
    }
}

internal void
coroutine_system_init(Coroutine_Group *sys){
    sys->arena = make_arena_system();
    
    Coroutine *root = &sys->root;
    
    sys->lock = system_mutex_make();
    sys->init_cv = system_condition_variable_make();
    sys->active = root;
    
    block_zero_struct(root);
    root->sys = sys;
    root->state = CoroutineState_Active;
    root->type = CoroutineType_Root;
    root->cv = system_condition_variable_make();
    
    sys->unused = 0;
    
    system_mutex_acquire(sys->lock);
}

internal Coroutine*
coroutine_system_alloc(Coroutine_Group *sys){
    Coroutine *result = sys->unused;
    if (result != 0){
        sll_stack_pop(sys->unused);
    }
    else{
        result = push_array(&sys->arena, Coroutine, 1);
        coroutine_sub_init(result, sys);
    }
    result->next = 0;
    return(result);
}

internal void
coroutine_system_free(Coroutine_Group *sys, Coroutine *coroutine){
    sll_stack_push(sys->unused, coroutine);
}

////////////////////////////////

internal Coroutine*
coroutine_create(Coroutine_Group *coroutines, Coroutine_Function *func){
    Coroutine *result = coroutine_system_alloc(coroutines);
    Assert(result->state == CoroutineState_Dead);
    result->func = func;
    return(result);
}

internal Coroutine*
coroutine_run(Coroutine_Group *sys, Coroutine *other, void *in, void *out){
    other->in = in;
    other->out = out;
    
    Coroutine *me = other->sys->active;
    Assert(me != 0);
    Assert(me->sys == other->sys);
    Assert(other->state == CoroutineState_Dead || other->state == CoroutineState_Inactive);
    other->yield_ctx = me;
    coroutine__pass_control(me, other, CoroutineState_Waiting, CoroutinePassControl_BlockMe);
    Assert(me == other->sys->active);
    
    Coroutine *result = other;
    if (other->state == CoroutineState_Dead){
        coroutine_system_free(sys, other);
        result = 0;
    }
    return(result);
}

internal void
coroutine_yield(Coroutine *me){
    Coroutine *other = me->yield_ctx;
    Assert(other != 0);
    Assert(me->sys == other->sys);
    Assert(other->state == CoroutineState_Waiting);
    coroutine__pass_control(me, other, CoroutineState_Inactive, CoroutinePassControl_BlockMe);
}

// BOTTOM

