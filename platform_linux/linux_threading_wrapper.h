/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Linux threading wrapper
 *
 */

// TOP

#if !defined(LINUX_THREADING_WRAPPER)
#define LINUX_THREADING_WRAPPER

#define PLAT_THREAD_SIG(n) void* n(void *ptr)
typedef PLAT_THREAD_SIG(Thread_Function);

union Thread{
    pthread_t t;
    FixSize(THREAD_TYPE_SIZE);
};

union Mutex{
    pthread_mutex_t crit;
    FixSize(MUTEX_TYPE_SIZE);
};

union Condition_Variable{
    pthread_cond_t cv;
    FixSize(CONDITION_VARIABLE_TYPE_SIZE);
};

union Semaphore{
    sem_t s;
    FixSize(SEMAPHORE_TYPE_SIZE);
};


internal void
system_init_and_launch_thread(Thread *t, Thread_Function *proc, void *ptr){
    pthread_create(&t->t, 0, proc, ptr);
}

internal void
system_init_lock(Mutex *m){
    pthread_mutex_init(&m->crit, NULL);
}

internal void
system_acquire_lock(Mutex *m){
    pthread_mutex_lock(&m->crit);
}

internal void
system_release_lock(Mutex *m){
    pthread_mutex_unlock(&m->crit);
}

internal void
system_init_cv(Condition_Variable *cv){
    pthread_cond_init(&cv->cv, NULL);
}

internal void
system_wait_cv(Condition_Variable *cv, Mutex *m){
    pthread_cond_wait(&cv->cv, &m->crit);
}

internal void
system_signal_cv(Condition_Variable *cv, Mutex *m){
    pthread_cond_signal(&cv->cv);
}

internal void
system_init_semaphore(Semaphore *s, u32 count){
    sem_init(&s->s, 0, 0);
}

internal void
system_wait_on_semaphore(Semaphore *s){
    sem_wait(&s->s);
}

internal void
system_release_semaphore(Semaphore *s){
    sem_post(&s->s);
}

#endif

// BOTTOM

