/*
 * 4coder_async_tasks.cpp - Types for the custom layer asynchronous task system.
 */

// TOP

#if !defined(FCODER_ASYNC_TASKS_H)
#define FCODER_ASYNC_TASKS_H

typedef void Async_Task_Function_Type(struct Async_Context *actx, String_Const_u8 data);
typedef u64 Async_Task;

struct Async_Thread{
    struct Async_System *async_system;
    System_Thread thread;
    struct Async_Node *node;
    Async_Task task;
    b32 cancel_signal;
};

struct Async_Node{
    union{
        Async_Node *next;
        Node node;
    };
    Async_Task task;
    Async_Thread *thread;
    Async_Task_Function_Type *func;
    String_Const_u8 data;
};

struct Async_System{
    void *cmd_context;
    
    Heap node_heap;
    Arena node_arena;
    System_Mutex mutex;
    System_Condition_Variable cv;
    System_Condition_Variable join_cv;
    Async_Task task_id_counter;
    Async_Node *free_nodes;
    Node task_sent;
    i32 task_count;
    
    Async_Thread thread;
};

struct Async_Context{
    Application_Links *app;
    Async_Thread *thread;
};

#endif

// BOTTOM

