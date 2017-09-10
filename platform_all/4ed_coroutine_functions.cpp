/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.09.2017
 *
 * Mac C++ layer for 4coder
 *
 */

// TOP

#if !defined(FRED_COROUTINE_FUNCTIONS_CPP)
#define FRED_COROUTINE_FUNCTIONS_CPP

//
// Coroutine
//

internal
Sys_Create_Coroutine_Sig(system_create_coroutine){
    Coroutine *coroutine = coroutine_system_alloc(&coroutines);
    Coroutine_Head *result = 0;
    if (coroutine != 0){
        coroutine_set_function(coroutine, func);
        result = &coroutine->head;
    }
    return(result);
}

internal
Sys_Launch_Coroutine_Sig(system_launch_coroutine){
    Coroutine *coroutine = (Coroutine*)head;
    coroutine->head.in = in;
    coroutine->head.out = out;
    
    Coroutine *active = coroutine->sys->active;
    Assert(active != 0);
    coroutine_launch(active, coroutine);
    Assert(active == coroutine->sys->active);
    
    Coroutine_Head *result = &coroutine->head;
    if (coroutine->state == CoroutineState_Dead){
        coroutine_system_free(&coroutines, coroutine);
        result = 0;
    }
    return(result);
}

Sys_Resume_Coroutine_Sig(system_resume_coroutine){
    Coroutine *coroutine = (Coroutine*)head;
    coroutine->head.in = in;
    coroutine->head.out = out;
    
    Coroutine *active = coroutine->sys->active;
    Assert(active != 0);
    coroutine_resume(active, coroutine);
    Assert(active == coroutine->sys->active);
    
    Coroutine_Head *result = &coroutine->head;
    if (coroutine->state == CoroutineState_Dead){
        coroutine_system_free(&coroutines, coroutine);
        result = 0;
    }
    return(result);
}

Sys_Yield_Coroutine_Sig(system_yield_coroutine){
    Coroutine *coroutine = (Coroutine*)head;
    coroutine_yield(coroutine);
}

#endif

// BOTTOM


