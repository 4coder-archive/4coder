/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.03.2016
 *
 * Templated code.
 *
 */

// TOP

// NOTE(allen): This is an experiment, BUT remember a lot of people shit on templates.
// So if you start getting a wiff of stupidity from this back out immediately!
//
// experience 1: no badness, haven't seen any anoying template errors
// ...

template<typename T>
inline void
dll_init_sentinel(T *sentinel){
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
}

template<typename T>
inline void
dll_insert(T *pos, T *v){
    v->next = pos->next;
    v->prev = pos;    
    pos->next = v;
    v->next->prev = v;
}

template<typename T>
inline void
dll_insert_back(T *pos, T *v){
    v->prev = pos->prev;
    v->next = pos;
    pos->prev = v;
    v->prev->next = v;
}

template<typename T>
inline void
dll_remove(T *v){
    v->next->prev = v->prev;
    v->prev->next = v->next;
}

// for(dll_items(iterator, sentinel_ptr)){...}
#define dll_items(it, st) ((it) = (st)->next); ((it) != (st)); ((it) = (it)->next)

// BOTTOM

