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
    Partition part = {};
    part.base = (char*)memory;
    part.pos = 0;
    part.max = size;
    return(part);
}

static void*
part_allocate(Partition *data, i32_4tech size){
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
part_reduce(Partition *data, i32_4tech size){
    if (size > 0 && size <= data->pos){
        data->pos -= size;
    }
}

static void
part_align(Partition *data, i32_4tech boundary){
    i32_4tech p = data->pos;
    p += boundary - 1;
    data->pos = p - p%boundary;
}

static void*
part_current(Partition *data){
    return(data->base + data->pos);
}

static i32_4tech
part_remaining(Partition *data){
    return(data->max - data->pos);
}

static Partition
part_sub_part(Partition *data, i32_4tech size){
    Partition result = {};
    void *d = part_allocate(data, size);
    if (d != 0){
        result = make_part(d, size);
    }
    return(result);
}

static Temp_Memory
begin_temp_memory(Partition *data){
    Temp_Memory result = {};
    result.part = data;
    result.pos = data->pos;
    return(result);
}

static void
end_temp_memory(Temp_Memory temp){
    temp.part->pos = temp.pos;
}

static void*
push_allocator_allocate(Partition *part, i32_4tech size){
    return(part_allocate(part, size));
}

static void
push_allocator_align(Partition *part, i32_4tech b){
    part_align(part, b);
}

////////////////////////////////

#if defined(FCODER_CUSTOM_H)

static Arena
make_arena(Application_Links *app, i32_4tech chunk_size, i32_4tech initial_align){
    Arena arena = {};
    arena.app = app;
    arena.chunk_size = chunk_size;
    arena.align = initial_align;
    return(arena);
}

static Arena
make_arena(Application_Links *app, i32_4tech chunk_size){
    return(make_arena(app, chunk_size, 1));
}

static Arena
make_arena(Application_Links *app){
    return(make_arena(app, 4096, 1));
}

static void
arena_release_all(Arena *arena){
    Application_Links *app = arena->app;
    for (Partition_Chained *part = arena->part, *prev = 0;
         part != 0;
         part = prev){
        prev = part->prev;
        memory_free(app, part, part->part.max);
    }
    arena->part = 0;
}

static void
arena_change_chunk_size(Arena *arena, i32_4tech chunk_size){
    arena->chunk_size = chunk_size;
}

static Partition_Chained*
arena__new_part(Arena *arena, i32_4tech new_chunk_size){
    Application_Links *app = arena->app;
    i32_4tech memory_size = new_chunk_size + sizeof(Partition_Chained);
    i32_4tech boundardy = 4096;
    if (memory_size < boundardy){
        memory_size = boundardy;
    }
    else{
        memory_size = memory_size + boundardy - 1;
        memory_size = memory_size - memory_size%boundardy;
    }
    void *memory = memory_allocate(app, memory_size);
    Partition_Chained *part = (Partition_Chained*)memory;
    part->part = make_part(memory, memory_size);
    part_allocate(&part->part, sizeof(*part));
    part->prev = arena->part;
    arena->part = part;
    return(part);
}

static Partition_Chained*
arena__new_part(Arena *arena){
    return(arena__new_part(arena, arena->chunk_size));
}

static void
arena__align_inner(Arena *arena, i32_4tech b){
    if (arena->align != 1){
        Partition_Chained *part = arena->part;
        if (part == 0){
            part = arena__new_part(arena);
        }
        part_align(&part->part, b);
    }
}

static void*
arena_allocate(Arena *arena, i32_4tech size){
    void *result = 0;
    Partition_Chained *part = arena->part;
    if (part == 0){
        part = arena__new_part(arena);
    }
    arena__align_inner(arena, arena->align);
    result = part_allocate(&part->part, size);
    if (result == 0){
        i32_4tech new_chunk_size = arena->chunk_size;
        if (size > new_chunk_size){
            new_chunk_size = size;
        }
        part = arena__new_part(arena, new_chunk_size);
        arena__align_inner(arena, arena->align);
        result = part_allocate(&part->part, size);
    }
    arena->align = 1;
    return(result);
}

static void
arena_align(Arena *arena, i32_4tech b){
    arena__align_inner(arena, b);
    arena->align = b;
}

static Partition*
arena_use_as_part(Arena *arena, i32_4tech minimum_part_size){
    if (minimum_part_size < arena->chunk_size){
        minimum_part_size = arena->chunk_size;
    }
    Partition_Chained *part = arena->part;
    if (part == 0){
        part = arena__new_part(arena, minimum_part_size);
    }
    else{
        if (part_remaining(&part->part) < minimum_part_size){
            part = arena__new_part(arena, minimum_part_size);
        }
    }
    return(&part->part);
}

static Temp_Memory_Arena
begin_temp_memory(Arena *arena){
    Temp_Memory_Arena result = {};
    result.arena = arena;
    result.part = arena->part;
    if (result.part != 0){
        result.pos = result.part->part.pos;
    }
    return(result);
}

static void
end_temp_memory(Temp_Memory_Arena temp){
    Application_Links *app = temp.arena->app;
    for (Partition_Chained *part = temp.arena->part, *prev = 0;
         part != temp.part;
         part = prev){
        prev = part->prev;
        memory_free(app, part, part->part.max);
    }
    temp.arena->part = temp.part;
    if (temp.part != 0){
        temp.part->part.pos = temp.pos;
    }
}

static Temp_Memory_Arena_Light
temp_memory_light(Temp_Memory_Arena temp){
    Temp_Memory_Arena_Light light = {};
    light.part = temp.part;
    light.pos = temp.pos;
    return(light);
}

static void
end_temp_memory(Arena *arena, Temp_Memory_Arena_Light temp){
    Temp_Memory_Arena full_temp = {};
    full_temp.arena = arena;
    full_temp.part = temp.part;
    full_temp.pos = temp.pos;
    end_temp_memory(full_temp);
}

static void*
push_allocator_allocate(Arena *arena, i32_4tech size){
    return(arena_allocate(arena, size));
}

static void
push_allocator_align(Arena *arena, i32_4tech b){
    arena_align(arena, b);
}

////////////////////////////////

Scratch_Block::Scratch_Block(Application_Links *app){
    scratch = context_get_arena(app);
    temp = begin_temp_memory(scratch);
}

Scratch_Block::~Scratch_Block(){
    end_temp_memory(temp);
}

Scratch_Block::operator Arena*(){
    return(scratch);
}

#endif

////////////////////////////////

#define push_array(A, T, size) (T*)push_allocator_allocate((A), sizeof(T)*(size))
#define push_align(A, b) push_allocator_align((A), (b))
#define reset_temp_memory end_temp_memory

// BOTTOM

