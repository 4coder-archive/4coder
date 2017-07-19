/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.07.2017
 *
 * Coroutine implementation from thread+mutex+cv
 *
 */

// TOP

typedef u32 Coroutine_State;
enum{
    CoroutineState_Dead,
    CoroutineState_Active,
    CoroutineState_Inactive,
    CoroutineState_Waiting,
};

typedef u32 Coroutine_Type;
enum{
    CoroutineType_Uninitialized,
    CoroutineType_Root,
    CoroutineType_Sub,
};

struct Coroutine{
    Coroutine_Head head;
    
    Thread thread;
    Condition_Variable cv;
    struct Coroutine_System *sys;
    Coroutine_Function *function;
    Coroutine *yield_ctx;
    Coroutine_State state;
    Coroutine_Type type;
};

struct Coroutine_System{
    Mutex lock;
    Condition_Variable init_cv;
    b32 did_init;
    Coroutine *active;
};

#define COROUTINE_INT_SKIP_SLEEP false

internal void
coroutine_internal_pass_control(Coroutine *me, Coroutine *other, Coroutine_State my_new_state, b32 do_sleep_loop = true){
    Assert(me->state == CoroutineState_Active);
    Assert(me->sys == other->sys);
    
    me->state = my_new_state;
    other->state = CoroutineState_Active;
    me->sys->active = other;
    system_signal_cv(&other->cv, &me->sys->lock);
    if (do_sleep_loop){
        for (;me->state != CoroutineState_Active;){
            system_wait_cv(&me->cv, &me->sys->lock);
        }
    }
}

internal
PLAT_THREAD_SIG(coroutine_main){
    Coroutine *me = (Coroutine*)ptr;
    
    // NOTE(allen): Init handshake
    Assert(me->state == CoroutineState_Dead);
    system_acquire_lock(&me->sys->lock);
    me->sys->did_init = true;
    system_signal_cv(&me->sys->init_cv, &me->sys->lock);
    
    for (;;){
        // NOTE(allen): Wait until someone wakes us up, then go into our procedure.
        for (;me->state != CoroutineState_Active;){
            system_wait_cv(&me->cv, &me->sys->lock);
        }
        Assert(me->type != CoroutineType_Root);
        Assert(me->yield_ctx != 0);
        Assert(me->function != 0);
        
        me->function(&me->head);
        
        // NOTE(allen): Wake up the caller and set this coroutine back to being dead.
        Coroutine *other = me->yield_ctx;
        Assert(other != 0);
        Assert(other->state == CoroutineState_Waiting);
        
        coroutine_internal_pass_control(me, other, CoroutineState_Dead, COROUTINE_INT_SKIP_SLEEP);
        me->function = 0;
    }
}

internal void
init_coroutine_system(Coroutine *root, Coroutine_System *sys){
    system_init_lock(&sys->lock);
    system_init_cv(&sys->init_cv);
    sys->active = root;
    
    memset(root, 0, sizeof(*root));
    
    root->sys = sys;
    root->state = CoroutineState_Active;
    root->type = CoroutineType_Root;
    
    system_init_cv(&root->cv);
}

internal void
init_coroutine_sub(Coroutine *co, Coroutine_System *sys){
    memset(co, 0, sizeof(*co));
    
    co->sys = sys;
    co->state = CoroutineState_Dead;
    co->type = CoroutineType_Sub;
    
    system_init_cv(&co->cv);
    
    sys->did_init = false;
    system_init_and_launch_thread(&co->thread, coroutine_main, co);
    for (;!sys->did_init;){
        system_wait_cv(&sys->init_cv, &sys->lock);
    }
}

// HACK(allen): I want to bundle this with launch!
internal void
coroutine_set_function(Coroutine *me, Coroutine_Function *function){
    Assert(me->state == CoroutineState_Dead);
    me->function = function;
}

internal void
coroutine_launch(Coroutine *me, Coroutine *other){
    Assert(me->sys == other->sys);
    Assert(other->state == CoroutineState_Dead);
    Assert(other->function != 0);
    
    other->yield_ctx = me;
    coroutine_internal_pass_control(me, other, CoroutineState_Waiting);
}

internal void
coroutine_yield(Coroutine *me){
    Coroutine *other = me->yield_ctx;
    Assert(other != 0);
    Assert(me->sys == other->sys);
    Assert(other->state == CoroutineState_Waiting);
    
    coroutine_internal_pass_control(me, other, CoroutineState_Inactive);
}

internal void
coroutine_resume(Coroutine *me, Coroutine *other){
    Assert(me->sys == other->sys);
    Assert(other->state == CoroutineState_Inactive);
    
    other->yield_ctx = me;
    coroutine_internal_pass_control(me, other, CoroutineState_Waiting);
}

////////////////////////////////

struct Coroutine_Alloc_Block{
    Coroutine coroutine;
    Coroutine_Alloc_Block *next;
};

#define COROUTINE_SLOT_SIZE sizeof(Coroutine_Alloc_Block)

struct Coroutine_System_Auto_Alloc{
    Coroutine_System sys;
    
    Coroutine root;
    Coroutine_Alloc_Block *head_free_uninit;
    Coroutine_Alloc_Block *head_free_inited;
};

internal void
init_coroutine_system(Coroutine_System_Auto_Alloc *sys){
    init_coroutine_system(&sys->root, &sys->sys);
    sys->head_free_uninit = 0;
    sys->head_free_inited = 0;
}

internal void
coroutine_system_provide_memory(Coroutine_System_Auto_Alloc *sys, void *memory, umem size){
    memset(memory, 0, size);
    
    Coroutine_Alloc_Block *blocks = (Coroutine_Alloc_Block*)memory;
    umem count = (size / sizeof(*blocks));
    
    Coroutine_Alloc_Block *old_head = sys->head_free_uninit;
    sys->head_free_uninit = &blocks[0];
    Coroutine_Alloc_Block *block = blocks;
    for (u32 i = 1; i < count; ++i, ++block){
        block->next = block + 1;
    }
    block->next = old_head;
}

internal Coroutine_Alloc_Block*
coroutine_system_pop(Coroutine_Alloc_Block **head){
    Coroutine_Alloc_Block *result = *head;
    if (result != 0){
        *head = result->next;
    }
    result->next = 0;
    return(result);
}

internal void
coroutine_system_push(Coroutine_Alloc_Block **head, Coroutine_Alloc_Block *block){
    block->next = *head;
    *head = block;
}

internal void
coroutine_system_force_init(Coroutine_System_Auto_Alloc *sys, u32 count){
    for (u32 i = 0; i < count; ++i){
        Coroutine_Alloc_Block *block = coroutine_system_pop(&sys->head_free_uninit);
        if (block == 0){
            break;
        }
        init_coroutine_sub(&block->coroutine, &sys->sys);
        coroutine_system_push(&sys->head_free_inited, block);
    }
}

internal Coroutine*
coroutine_system_alloc(Coroutine_System_Auto_Alloc *sys){
    Coroutine_Alloc_Block *block = coroutine_system_pop(&sys->head_free_inited);
    if (block == 0){
        coroutine_system_force_init(sys, 1);
        block = coroutine_system_pop(&sys->head_free_inited);
    }
    
    Coroutine *result = 0;
    if (block != 0){
        result = &block->coroutine;
    }
    return(result);
}

internal void
coroutine_system_free(Coroutine_System_Auto_Alloc *sys, Coroutine *co){
    Coroutine_Alloc_Block *block = (Coroutine_Alloc_Block*)co;
    coroutine_system_push(&sys->head_free_inited, block);
}

// BOTTOM

