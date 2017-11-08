/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 07.11.2017
 *
 * Linux semaphore wrapper
 *
 */

// TOP

union Semaphore{
    sem_t s;
    FixSize(SEMAPHORE_TYPE_SIZE);
};

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

// BOTTOM


