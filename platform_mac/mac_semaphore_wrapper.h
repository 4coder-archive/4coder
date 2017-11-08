/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 07.11.2017
 *
 * Mac semaphore wrapper
 *
 */

// TOP

union Semaphore{
    semaphore_t s;
    FixSize(SEMAPHORE_TYPE_SIZE);
};

internal void
system_init_semaphore(Semaphore *s, u32 count){
    task_t task = mach_task_self();
    semaphore_create(task, &s->s, SYNC_POLICY_FIFO, 0);
}

internal void
system_wait_on_semaphore(Semaphore *s){
    semaphore_wait(s->s);
}

internal void
system_release_semaphore(Semaphore *s){
    semaphore_signal(s->s);
}

// BOTTOM


