
#ifndef FCODER_MEM_H
#define FCODER_MEM_H

struct Partition{
    char *base;
    int pos, max;
};

struct Temp_Memory{
    void *handle;
    int pos;
};

inline Partition
make_part(void *memory, int size){
    Partition partition;
    partition.base = (char*)memory;
    partition.pos = 0;
    partition.max = size;
    return partition;
}

inline void*
partition_allocate(Partition *data, int size){
    void *ret = 0;
    if (size > 0 && data->pos + size <= data->max){
        ret = data->base + data->pos;
        data->pos += size;
    }
    return ret;
}

inline void
partition_align(Partition *data, unsigned int boundary){
    --boundary;
    data->pos = (data->pos + boundary) & (~boundary);
}

inline void*
partition_current(Partition *data){
    return data->base + data->pos;
}

inline int
partition_remaining(Partition *data){
    return data->max - data->pos;
}

inline Partition
partition_sub_part(Partition *data, int size){
    Partition result = {};
    void *d = partition_allocate(data, size);
    if (d) result = make_part(d, size);
    return result;
}

#define push_struct(part, T) (T*)partition_allocate(part, sizeof(T))
#define push_array(part, T, size) (T*)partition_allocate(part, sizeof(T)*(size))
#define push_block(part, size) partition_allocate(part, size)

inline Temp_Memory
begin_temp_memory(Partition *data){
    Temp_Memory result;
    result.handle = data;
    result.pos = data->pos;
    return result;
}

inline void
end_temp_memory(Temp_Memory temp){
    ((Partition*)temp.handle)->pos = temp.pos;
}

/*
NOTE(allen):
This is a very week general purpose allocator system.
It should only be used for infrequent large allocations (4K+).
*/

#include <stdint.h>
#include <assert.h>
#include <string.h>

enum{
    MEM_BUBBLE_FLAG_INIT = 0x0,
    MEM_BUBBLE_USED = 0x1,
};

struct Bubble{
    Bubble *prev;
    Bubble *next;
    int32_t size;
    uint32_t flags;
    uint32_t _unused_[2];
};

struct General_Memory{
    Bubble sentinel;
};

struct Mem_Options{
    Partition part;
    General_Memory general;
};

inline void
insert_bubble(Bubble *prev, Bubble *bubble){
    bubble->prev = prev;
    bubble->next = prev->next;
    bubble->prev->next = bubble;
    bubble->next->prev = bubble;
}

inline void
remove_bubble(Bubble *bubble){
    bubble->prev->next = bubble->next;
    bubble->next->prev = bubble->prev;
}

static void
general_memory_open(General_Memory *general, void *memory, int32_t size){
    general->sentinel.prev = &general->sentinel;
    general->sentinel.next = &general->sentinel;
    general->sentinel.flags = MEM_BUBBLE_USED;
    general->sentinel.size = 0;
    
    Bubble *first = (Bubble*)memory;
    first->flags = (uint32_t)MEM_BUBBLE_FLAG_INIT;
    first->size = size - sizeof(Bubble);
    insert_bubble(&general->sentinel, first);
}

static int32_t
general_memory_check(General_Memory *general){
    Bubble *sentinel = &general->sentinel;
    for (Bubble *bubble = sentinel->next;
         bubble != sentinel;
         bubble = bubble->next){
        assert(bubble);
        
        Bubble *next = bubble->next;
        assert(bubble == next->prev);
        if (next != sentinel && bubble->prev != sentinel){
            assert(bubble->next > bubble);
            assert(bubble > bubble->prev);
            
            char *end_ptr = (char*)(bubble + 1) + bubble->size;
            char *next_ptr = (char*)next;
            (void)(end_ptr);
            (void)(next_ptr);
            assert(end_ptr == next_ptr);
        }
    }
    return(1);
}

#define BUBBLE_MIN_SIZE 1024

static void
general_memory_attempt_split(Bubble *bubble, int32_t wanted_size){
    int32_t remaining_size = bubble->size - wanted_size;
    if (remaining_size >= BUBBLE_MIN_SIZE){
        bubble->size = wanted_size;
        Bubble *new_bubble = (Bubble*)((char*)(bubble + 1) + wanted_size);
        new_bubble->flags = (uint32_t)MEM_BUBBLE_FLAG_INIT;
        new_bubble->size = remaining_size - sizeof(Bubble);
        insert_bubble(bubble, new_bubble);
    }
}

static void*
general_memory_allocate(General_Memory *general, int32_t size){
    void *result = 0;
    for (Bubble *bubble = general->sentinel.next;
         bubble != &general->sentinel;
         bubble = bubble->next){
        if (!(bubble->flags & MEM_BUBBLE_USED)){
            if (bubble->size >= size){
                result = bubble + 1;
                bubble->flags |= MEM_BUBBLE_USED;
                general_memory_attempt_split(bubble, size);
                break;
            }
        }
    }
    return result;
}

inline void
general_memory_do_merge(Bubble *left, Bubble *right){
    assert(left->next == right);
    assert(right->prev == left);
    left->size += sizeof(Bubble) + right->size;
    remove_bubble(right);
}

inline void
general_memory_attempt_merge(Bubble *left, Bubble *right){
    if (!(left->flags & MEM_BUBBLE_USED) &&
        !(right->flags & MEM_BUBBLE_USED)){
        general_memory_do_merge(left, right);
    }
}

static void
general_memory_free(General_Memory *general, void *memory){
    Bubble *bubble = ((Bubble*)memory) - 1;
    bubble->flags &= ~MEM_BUBBLE_USED;
    Bubble *prev, *next;
    prev = bubble->prev;
    next = bubble->next;
    general_memory_attempt_merge(bubble, next);
    general_memory_attempt_merge(prev, bubble);
}

static void*
general_memory_reallocate(General_Memory *general, void *old, int32_t old_size, int32_t size){
    void *result = old;
    Bubble *bubble = ((Bubble*)old) - 1;
    int32_t additional_space = size - bubble->size;
    if (additional_space > 0){
        Bubble *next = bubble->next;
        if (!(next->flags & MEM_BUBBLE_USED) &&
            next->size + sizeof(Bubble) >= additional_space){
            general_memory_do_merge(bubble, next);
            general_memory_attempt_split(bubble, size);
        }
        else{
            result = general_memory_allocate(general, size);
            if (old_size) memcpy(result, old, old_size);
            general_memory_free(general, old);
        }
    }
    return result;
}

inline void*
general_memory_reallocate_nocopy(General_Memory *general, void *old, int32_t size){
    return general_memory_reallocate(general, old, 0, size);
}

#define reset_temp_memory end_temp_memory

#define gen_struct(g, T) (T*)general_memory_allocate(g, sizeof(T), 0)
#define gen_array(g, T, size) (T*)general_memory_allocate(g, sizeof(T)*(size))
#define gen_block(g, size) general_memory_open(g, size, 0)

#endif

