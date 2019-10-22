/*
 * 4coder_async_tasks.cpp - Types for the custom layer asynchronous task system.
 */

// TOP

#if !defined(FCODER_ASYNC_TASKS_H)
#define FCODER_ASYNC_TASKS_H

typedef void Async_Task_Function_Type(struct Async_Context *actx, Data data);
typedef u64 Async_Task;

struct Async_Thread{
    System_Thread thread;
    struct Async_Node *node;
    Async_Task task;
};

struct Async_Node{
    Async_Node *next;
    Async_Task task;
    Async_Thread *thread;
    Async_Task_Function_Type *func;
    Data data;
};

struct Async_System{
    void *cmd_context;
    
    Heap node_heap;
    Arena node_arena;
    System_Mutex mutex;
    System_Condition_Variable cv;
    Async_Task task_id_counter;
    Async_Node *free_nodes;
    Async_Node *task_first;
    Async_Node *task_last;
    i32 task_count;
    
    Async_Thread thread;
};

struct Async_Context{
    Application_Links *app;
    Async_Thread *thread;
};

#endif

// BOTTOM

