
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

#endif

