/*
 * 4coder_async_tasks.cpp - Implementation of the custom layer asynchronous task system.
 */

// TOP

global Async_System async_system = {};

function Async_Node*
async_pop_node(void){
    system_mutex_acquire(async_system.mutex);
    for (;async_system.task_count == 0;){
        system_condition_variable_wait(async_system.cv, async_system.mutex);
    }
    Async_Node *node = async_system.task_first;
    sll_queue_pop(async_system.task_first, async_system.task_last);
    async_system.task_count -= 1;
    system_mutex_release(async_system.mutex);
    node->next = 0;
    return(node);
}

function Async_Task
async_push_node(Async_Task_Function_Type *func, Data data){
    Async_Task result = async_system.task_id_counter;
    async_system.task_id_counter += 1;
    
    system_mutex_acquire(async_system.mutex);
    Async_Node *node = async_system.free_nodes;
    if (node == 0){
        node = push_array(&async_system.node_arena, Async_Node, 1);
    }
    else{
        sll_stack_pop(async_system.free_nodes);
    }
    node->task = result;
    node->thread = 0;
    node->func = func;
    node->data.data = (u8*)heap_allocate(&async_system.node_heap, data.size);
    block_copy(node->data.data, data.data, data.size);
    node->data.size = data.size;
    sll_queue_push(async_system.task_first, async_system.task_last, node);
    async_system.task_count += 1;
    system_condition_variable_signal(async_system.cv);
    system_mutex_release(async_system.mutex);
    
    return(result);
}

function void
async_free_node(Async_Node *node){
    system_mutex_acquire(async_system.mutex);
    heap_free(&async_system.node_heap, node->data.data);
    sll_stack_push(async_system.free_nodes, node);
    system_mutex_release(async_system.mutex);
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
    
    Application_Links app = {};
    app.tctx = tctx;
    app.cmd_context = async_system.cmd_context;
    Async_Context ctx = {&app, thread};
    
    for (;;){
        Async_Node *node = async_pop_node();
        node->thread = thread;
        thread->node = node;
        thread->task = node->task;
        node->func(&ctx, node->data);
        thread->node = 0;
        thread->task = 0;
        async_free_node(node);
    }
}

function Async_Node*
async_get_pending_node(Async_Task task){
    Async_Node *result = 0;
    for (Async_Node *node = async_system.task_first;
         node != 0;
         node = node->next){
        if (node->task == task){
            result = node;
            break;
        }
    }
    return(result);
}

function Async_Node*
async_get_running_node(Async_Task task){
    Async_Node *result = 0;
    if (async_system.thread.task == task){
        
    }
    return(result);
}

////////////////////////////////

function void
async_task_handler_init(Application_Links *app){
    block_zero_struct(&async_system);
    async_system.cmd_context = app->cmd_context;
    async_system.node_arena = make_arena_system(KB(4));
    heap_init(&async_system.node_heap, &async_system.node_arena);
    async_system.mutex = system_mutex_make();
    async_system.cv = system_condition_variable_make();
    async_system.thread.thread = system_thread_launch(async_task_thread, &async_system.thread);
}

function Async_Task
async_task_no_dep(Async_Task_Function_Type *func, Data data){
    return(async_push_node(func, data));
}

function Async_Task
async_task_single_dep(Async_Task_Function_Type *func, Data data, Async_Task dependency){
    NotImplemented;
}

function b32
async_task_is_pending(Async_Task task){
    system_mutex_acquire(async_system.mutex);
    Async_Node *node = async_get_pending_node(task);
    system_mutex_release(async_system.mutex);
    return(node != 0);
}

function b32
async_task_is_running(Async_Task task){
    system_mutex_acquire(async_system.mutex);
    Async_Node *node = async_get_running_node(task);
    system_mutex_release(async_system.mutex);
    return(node != 0);
}

function b32
async_task_is_running_or_pending(Async_Task task){
    system_mutex_acquire(async_system.mutex);
    Async_Node *node = async_get_pending_node(task);
    if (node != 0){
        node = async_get_running_node(task);
    }
    system_mutex_release(async_system.mutex);
    return(node != 0);
}

function void
async_task_cancel(Async_Task task){
    system_mutex_acquire(async_system.mutex);
    NotImplemented;
    system_mutex_release(async_system.mutex);
}

function void
async_task_join(Async_Task task){
    NotImplemented;
}

// BOTTOM

