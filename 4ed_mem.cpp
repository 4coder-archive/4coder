/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.11.2015
 *
 * Memory utils for 4coder
 *
 */

// TOP

struct Partition{
    u8 *base;
    i32 pos, max;
};

struct Temp_Memory{
    void *handle;
    int pos;
};

enum Memory_Bubble_Flag{
    MEM_BUBBLE_USED = 0x1,
    MEM_BUBBLE_DEBUG = 0xD3000000,
    MEM_BUBBLE_SYS_DEBUG = 0x5D000000,
    MEM_BUBBLE_DEBUG_MASK = 0xFF000000
};

struct Bubble{
    Bubble *prev;
    Bubble *next;
    u32 flags;
    i32 size;
    u32 type;
    u32 _unused_;
};

struct General_Memory{
    Bubble sentinel;
};

struct Mem_Options{
    Partition part;
    General_Memory general;
};

inline Partition
partition_open(void *memory, i32 size){
    Partition partition;
    partition.base = (u8*)memory;;
    partition.pos = 0;
    partition.max = size;
    return partition;
}

inline void*
partition_allocate(Partition *data, i32 size){
    void *ret = 0;
    if (size > 0 && data->pos + size <= data->max){
        ret = data->base + data->pos;
        data->pos += size;
    }
    return ret;
}

inline void
partition_align(Partition *data, u32 boundary){
    data->pos = (data->pos + (boundary - 1)) & (~boundary);
}

inline void*
partition_current(Partition *data){
    return data->base + data->pos;
}

inline i32
partition_remaining(Partition *data){
    return data->max - data->pos;
}

inline Partition
partition_sub_part(Partition *data, i32 size){
    Partition result = {};
    void *d = partition_allocate(data, size);
    if (d) result = partition_open(d, size);
    return result;
}

#define push_struct(part, T) (T*)partition_allocate(part, sizeof(T))
#define push_array(part, T, size) (T*)partition_allocate(part, sizeof(T)*(size))
#define push_block(part, size) partition_allocate(part, size)

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

#if FRED_INTERNAL
#define MEM_BUBBLE_FLAG_INIT MEM_BUBBLE_DEBUG
#else
#define MEM_BUBBLE_FLAG_INIT 0
#endif

internal void
general_memory_open(General_Memory *general, void *memory, i32 size){
    general->sentinel.prev = &general->sentinel;
    general->sentinel.next = &general->sentinel;
    general->sentinel.flags = MEM_BUBBLE_USED;
    general->sentinel.size = 0;
    
    Bubble *first = (Bubble*)memory;
    first->flags = (u32)MEM_BUBBLE_FLAG_INIT;
    first->size = size - sizeof(Bubble);
    insert_bubble(&general->sentinel, first);
}

internal void
general_memory_check(General_Memory *general){
    Bubble *sentinel = &general->sentinel;
    for (Bubble *bubble = sentinel->next;
         bubble != sentinel;
         bubble = bubble->next){
        Assert(bubble);
        
        Bubble *next = bubble->next;
        Assert(bubble == next->prev);
        if (next != sentinel){
            Assert(bubble->next > bubble);
            Assert(bubble > bubble->prev);
            
            char *end_ptr = (char*)(bubble + 1) + bubble->size;
            char *next_ptr = (char*)next;
            AllowLocal(end_ptr);
            AllowLocal(next_ptr);
            Assert(end_ptr == next_ptr);
        }
    }
}

#define BUBBLE_MIN_SIZE 1024

internal void
general_memory_attempt_split(Bubble *bubble, i32 wanted_size){
    i32 remaining_size = bubble->size - wanted_size;
    if (remaining_size >= BUBBLE_MIN_SIZE){
        bubble->size = wanted_size;
        Bubble *new_bubble = (Bubble*)((u8*)(bubble + 1) + wanted_size);
        new_bubble->flags = (u32)MEM_BUBBLE_FLAG_INIT;
        new_bubble->size = remaining_size - sizeof(Bubble);
        insert_bubble(bubble, new_bubble);
    }
}

internal void*
general_memory_allocate(General_Memory *general, i32 size, u32 type = 0){
    void *result = 0;
    for (Bubble *bubble = general->sentinel.next;
         bubble != &general->sentinel;
         bubble = bubble->next){
        if (!(bubble->flags & MEM_BUBBLE_USED)){
            if (bubble->size >= size){
                result = bubble + 1;
                bubble->flags |= MEM_BUBBLE_USED;
                bubble->type = type;
                general_memory_attempt_split(bubble, size);
                break;
            }
        }
    }
    return result;
}

inline void
general_memory_do_merge(Bubble *left, Bubble *right){
    Assert(left->next == right);
    Assert(right->prev == left);
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

internal void
general_memory_free(General_Memory *general, void *memory){
    Bubble *bubble = ((Bubble*)memory) - 1;
    Assert((!FRED_INTERNAL) || (bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_DEBUG);
    bubble->flags &= ~MEM_BUBBLE_USED;
    bubble->type = 0;
    Bubble *prev, *next;
    prev = bubble->prev;
    next = bubble->next;
    general_memory_attempt_merge(bubble, next);
    general_memory_attempt_merge(prev, bubble);
}

internal void*
general_memory_reallocate(General_Memory *general, void *old, i32 old_size, i32 size, u32 type = 0){
    void *result = old;
    Bubble *bubble = ((Bubble*)old) - 1;
    bubble->type = type;
    Assert((!FRED_INTERNAL) || (bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_DEBUG);
    i32 additional_space = size - bubble->size;
    if (additional_space > 0){
        Bubble *next = bubble->next;
        if (!(next->flags & MEM_BUBBLE_USED) &&
            next->size + sizeof(Bubble) >= additional_space){
            general_memory_do_merge(bubble, next);
            general_memory_attempt_split(bubble, size);
        }
        else{
            result = general_memory_allocate(general, size, type);
            if (old_size) memcpy(result, old, old_size);
            general_memory_free(general, old);
        }
    }
    return result;
}

inline void*
general_memory_reallocate_nocopy(General_Memory *general, void *old, i32 size, u32 type = 0){
    return general_memory_reallocate(general, old, 0, size, type);
}

internal Temp_Memory
begin_temp_memory(Partition *data){
    Temp_Memory result;
    result.handle = data;
    result.pos = data->pos;
    return result;
}

internal void
end_temp_memory(Temp_Memory temp){
    ((Partition*)temp.handle)->pos = temp.pos;
}

// BOTTOM

