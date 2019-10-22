/*
 * 4coder_async_tasks.cpp - Implementation of the custom layer asynchronous task system.
 */

// TOP

// TODO(allen): put atomic wrappers in base type layer
#define atomic_write_b32(p,v) (*(p)=(v))
#define atomic_read_b32(p) (*(p))

global Async_System global_async_system = {};

function Async_Node*
async_pop_node(Async_System *async_system){
    for (;async_system->task_count == 0;){
        system_condition_variable_wait(async_system->cv, async_system->mutex);
    }
    Node *node = async_system->task_sent.next;
    Assert(node != &async_system->task_sent);
    dll_remove(node);
    async_system->task_count -= 1;
    Async_Node *a_node = CastFromMember(Async_Node, node, node);
    a_node->next = 0;
    return(a_node);
}

function Async_Task
async_push_node(Async_System *async_system, Async_Task_Function_Type *func, Data data){
    Async_Task result = async_system->task_id_counter;
    async_system->task_id_counter += 1;
    
    Async_Node *node = async_system->free_nodes;
    if (node == 0){
        node = push_array(&async_system->node_arena, Async_Node, 1);
    }
    else{
        sll_stack_pop(async_system->free_nodes);
    }
    node->task = result;
    node->thread = 0;
    node->func = func;
    node->data.data = (u8*)heap_allocate(&async_system->node_heap, data.size);
    block_copy(node->data.data, data.data, data.size);
    node->data.size = data.size;
    dll_insert_back(&async_system->task_sent, &node->node);
    async_system->task_count += 1;
    system_condition_variable_signal(async_system->cv);
    
    return(result);
}

function void
async_free_node(Async_System *async_system, Async_Node *node){
    heap_free(&async_system->node_heap, node->data.data);
    sll_stack_push(async_system->free_nodes, node);
}

function void
async_task_thread(void *thread_ptr){
    Base_Allocator *allocator = get_base_allocator_system();
    
    Thread_Context_Extra_Info tctx_info = {};
    tctx_info.async_thread = thread_ptr;
    
    Thread_Context tctx_ = {};
    Thread_Context *tctx = &tctx_;
    thread_ctx_init(tctx, ThreadKind_AsyncTasks, allocator, allocator);
    
    Async_Thread *thread = (Async_Thread*)thread_ptr;
    Async_System *async_system = thread->async_system;
    
    Application_Links app = {};
    app.tctx = tctx;
    app.cmd_context = async_system->cmd_context;
    Async_Context ctx = {&app, thread};
    
    for (;;){
        system_mutex_acquire(async_system->mutex);
        Async_Node *node = async_pop_node(async_system);
        node->thread = thread;
        thread->node = node;
        thread->task = node->task;
        thread->cancel_signal = false;
        thread->join_signal = false;
        system_mutex_release(async_system->mutex);
        
        node->func(&ctx, node->data);
        
        system_mutex_acquire(async_system->mutex);
        if (thread->join_signal){
            system_condition_variable_signal(async_system->join_cv);
        }
        node->thread = 0;
        thread->node = 0;
        thread->task = 0;
        thread->cancel_signal = false;
        thread->join_signal = false;
        async_free_node(async_system, node);
        system_mutex_release(async_system->mutex);
    }
}

function Async_Node*
async_get_pending_node(Async_System *async_system, Async_Task task){
    Async_Node *result = 0;
    if (task != 0){
        for (Node *node = async_system->task_sent.next;
             node != &async_system->task_sent;
             node = node->next){
            Async_Node *a_node = CastFromMember(Async_Node, node, node);
            if (a_node->task == task){
                result = a_node;
                break;
            }
        }
    }
    return(result);
}

function Async_Node*
async_get_running_node(Async_System *async_system, Async_Task task){
    Async_Node *result = 0;
    if (task != 0 && async_system->thread.task == task){
        result = async_system->thread.node;
    }
    return(result);
}

////////////////////////////////

function void
async_task_handler_init(Application_Links *app, Async_System *async_system){
    block_zero_struct(async_system);
    async_system->cmd_context = app->cmd_context;
    async_system->node_arena = make_arena_system(KB(4));
    heap_init(&async_system->node_heap, &async_system->node_arena);
    async_system->mutex = system_mutex_make();
    async_system->cv = system_condition_variable_make();
    async_system->join_cv = system_condition_variable_make();
    dll_init_sentinel(&async_system->task_sent);
    async_system->thread.thread = system_thread_launch(async_task_thread, &async_system->thread);
}

function Async_Task
async_task_no_dep(Async_System *async_system, Async_Task_Function_Type *func, Data data){
    system_mutex_acquire(async_system->mutex);
    Async_Task result = async_push_node(async_system, func, data);
    system_mutex_release(async_system->mutex);
    return(result);
}

function Async_Task
async_task_single_dep(Async_System *async_system, Async_Task_Function_Type *func, Data data, Async_Task dependency){
    NotImplemented;
}

function b32
async_task_is_pending(Async_System *async_system, Async_Task task){
    system_mutex_acquire(async_system->mutex);
    Async_Node *node = async_get_pending_node(async_system, task);
    system_mutex_release(async_system->mutex);
    return(node != 0);
}

function b32
async_task_is_running(Async_System *async_system, Async_Task task){
    system_mutex_acquire(async_system->mutex);
    Async_Node *node = async_get_running_node(async_system, task);
    system_mutex_release(async_system->mutex);
    return(node != 0);
}

function b32
async_task_is_running_or_pending(Async_System *async_system, Async_Task task){
    system_mutex_acquire(async_system->mutex);
    Async_Node *node = async_get_pending_node(async_system, task);
    if (node != 0){
        node = async_get_running_node(async_system, task);
    }
    system_mutex_release(async_system->mutex);
    return(node != 0);
}

function void
async_task_cancel(Async_System *async_system, Async_Task task){
    system_mutex_acquire(async_system->mutex);
    Async_Node *node = async_get_pending_node(async_system, task);
    if (node != 0){
        dll_remove(&node->node);
        async_system->task_count -= 1;
        async_free_node(async_system, node);
    }
    else{
        node = async_get_running_node(async_system, task);
        if (node != 0){
            b32 *cancel_signal = &node->thread->cancel_signal;
            atomic_write_b32(cancel_signal, true);
        }
    }
    system_mutex_release(async_system->mutex);
}

function void
async_task_join(Async_System *async_system, Async_Task task){
    system_mutex_acquire(async_system->mutex);
    Async_Node *node = async_get_pending_node(async_system, task);
    b32 wait_for_join = false;
    if (node != 0){
        dll_remove(&node->node);
        dll_insert(&async_system->task_sent, &node->node);
        node->thread->join_signal = true;
        wait_for_join = true;
    }
    else{
        node = async_get_running_node(async_system, task);
        if (node != 0){
            node->thread->join_signal = true;
            wait_for_join = true;
        }
    }
    if (wait_for_join){
        system_condition_variable_wait(async_system->join_cv, async_system->mutex);
    }
    system_mutex_release(async_system->mutex);
}

function b32
async_check_canceled(Async_Context *actx){
    b32 *cancel_signal = &actx->thread->cancel_signal;
    b32 result = atomic_read_b32(cancel_signal);
    return(result);
}

// BOTTOM

