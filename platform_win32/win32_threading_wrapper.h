/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.07.2017
 *
 * Win32 threading wrapper
 *
 */

// TOP

#if !defined(WIN32_THREADING_WRAPPER)
#define WIN32_THREADING_WRAPPER

#define PLAT_THREAD_SIG(n) DWORD CALL_CONVENTION n(LPVOID ptr)
typedef PLAT_THREAD_SIG(Thread_Function);

union Thread{
    HANDLE t;
    FixSize(THREAD_TYPE_SIZE);
};

union Mutex{
    CRITICAL_SECTION crit;
    FixSize(MUTEX_TYPE_SIZE);
};

union Condition_Variable{
    CONDITION_VARIABLE cv;
    FixSize(CONDITION_VARIABLE_TYPE_SIZE);
};

union Semaphore{
    HANDLE s;
    FixSize(SEMAPHORE_TYPE_SIZE);
};


internal void
system_init_and_launch_thread(Thread *t, Thread_Function *proc, void *ptr){
    t->t = CreateThread(0, 0, proc, ptr, 0, 0);
}

internal void
system_init_lock(Mutex *m){
    InitializeCriticalSection(&m->crit);
}

internal void
system_acquire_lock(Mutex *m){
    EnterCriticalSection(&m->crit);
}

internal void
system_release_lock(Mutex *m){
    LeaveCriticalSection(&m->crit);
}

internal void
system_init_cv(Condition_Variable *cv){
    InitializeConditionVariable(&cv->cv);
}

internal void
system_wait_cv(Condition_Variable *cv, Mutex *lock){
    SleepConditionVariableCS(&cv->cv, &lock->crit, INFINITE);
}

internal void
system_signal_cv(Condition_Variable *cv, Mutex *lock){
    WakeConditionVariable(&cv->cv);
}

internal void
system_init_semaphore(Semaphore *s, u32 max){
    s->s = CreateSemaphore(0, 0, max, 0);
}

internal void
system_wait_on_semaphore(Semaphore *s){
    WaitForSingleObject(s->s, INFINITE);
}

internal void
system_release_semaphore(Semaphore *s){
    ReleaseSemaphore(s->s, 1, 0);
}

#endif

// BOTTOM

