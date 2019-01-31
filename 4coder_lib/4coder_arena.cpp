/*
4coder_arena.cpp - Preversioning
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.
*/

// TOP

static Partition
make_part(void *memory, i32_4tech size){
    Partition partition = {};
    partition.base = (char*)memory;
    partition.pos = 0;
    partition.max = size;
    return partition;
}

static void*
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

static void
partition_reduce(Partition *data, i32_4tech size){
    if (size > 0 && size <= data->pos){
        data->pos -= size;
    }
}

static void*
partition_align(Partition *data, u32_4tech boundary){
    --boundary;
    data->pos = (data->pos + boundary) & (~boundary);
    return(data->base + data->pos);
}

static void*
partition_current(Partition *data){
    return(data->base + data->pos);
}

static i32_4tech
partition_remaining(Partition *data){
    return(data->max - data->pos);
}

static Partition
partition_sub_part(Partition *data, i32_4tech size){
    Partition result = {};
    void *d = partition_allocate(data, size);
    if (d != 0){
        result = make_part(d, size);
    }
    return(result);
}

#define push_array(part, T, size) (T*)partition_allocate(part, sizeof(T)*(size))
#define push_align(part, b) partition_align(part, b)

static Temp_Memory
begin_temp_memory(Partition *data){
    Temp_Memory result;
    result.handle = data;
    result.pos = data->pos;
    return(result);
}

static void
end_temp_memory(Temp_Memory temp){
    ((Partition*)temp.handle)->pos = temp.pos;
}

#define reset_temp_memory end_temp_memory

// BOTTOM

