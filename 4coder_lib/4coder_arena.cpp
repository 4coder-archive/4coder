/*
4coder_arena.cpp - Preversioning
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.
*/

// TOP

inline Partition
make_part(void *memory, i32_4tech size){
    Partition partition = {0};
    partition.base = (char*)memory;
    partition.pos = 0;
    partition.max = size;
    return partition;
}

inline void*
partition_allocate(Partition *data, i32_4tech size){
    void *ret = 0;
    if (size < 0){
        size = 0;
    }
    if (data->pos + size <= data->max){
        ret = data->base + data->pos;
        data->pos += size;
    }
    return(ret);
}

inline void
partition_reduce(Partition *data, i32_4tech size){
    if (size > 0 && size <= data->pos){
        data->pos -= size;
    }
}

inline void*
partition_align(Partition *data, u32_4tech boundary){
    --boundary;
    data->pos = (data->pos + boundary) & (~boundary);
    return(data->base + data->pos);
}

inline void*
partition_current(Partition *data){
    return(data->base + data->pos);
}

inline i32_4tech
partition_remaining(Partition *data){
    return(data->max - data->pos);
}

inline Partition
partition_sub_part(Partition *data, i32_4tech size){
    Partition result = {};
    void *d = partition_allocate(data, size);
    if (d != 0){
        result = make_part(d, size);
    }
    return(result);
}

#define push_struct(part, T) (T*)partition_allocate(part, sizeof(T))
#define push_array(part, T, size) (T*)partition_allocate(part, sizeof(T)*(size))
#define push_block(part, size) partition_allocate(part, size)
#define push_align(part, b) partition_align(part, b)

inline Temp_Memory
begin_temp_memory(Partition *data){
    Temp_Memory result;
    result.handle = data;
    result.pos = data->pos;
    return(result);
}

inline void
end_temp_memory(Temp_Memory temp){
    ((Partition*)temp.handle)->pos = temp.pos;
}

inline Tail_Temp_Partition
begin_tail_part(Partition *data, i32_4tech size){
    Tail_Temp_Partition result = {0};
    if (data->pos + size <= data->max){
        result.handle = data;
        result.old_max = data->max;
        data->max -= size;
        result.part = make_part(data->base + data->max, size);
    }
    return(result);
}

inline void
end_tail_part(Tail_Temp_Partition temp){
    if (temp.handle){
        Partition *part = (Partition*)temp.handle;
        part->max = temp.old_max;
    }
}

#define reset_temp_memory end_temp_memory

// BOTTOM

