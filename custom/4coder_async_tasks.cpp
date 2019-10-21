/*
 * 4coder_async_tasks.cpp - Implementation of the custom layer asynchronous task system.
 */

// TOP

Application_Links *dummy_async_app = 0;

function void
async_task_handler_init(Application_Links *app){
    //NotImplemented;
    dummy_async_app = app;
}

function Async_Task
async_task_no_dep(Async_Task_Function_Type *func, Data data){
    //NotImplemented;
    func(dummy_async_app, data);
    return(0);
}

function Async_Task
async_task_single_dep(Async_Task_Function_Type *func, Data data, Async_Task dependency){
    NotImplemented;
}

function b32
async_task_is_pending(Async_Task task){
    NotImplemented;
}

function b32
async_task_is_running(Async_Task task){
    NotImplemented;
}

function b32
async_task_is_running_or_pending(Async_Task task){
    NotImplemented;
}

function void
async_task_cancel(Async_Task task){
    NotImplemented;
}

function void
async_task_join(Async_Task task){
    NotImplemented;
}

// BOTTOM

