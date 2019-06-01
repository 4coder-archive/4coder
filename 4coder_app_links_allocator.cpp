/*
 * 4coder app links base allocator
 */

// TOP

internal void
scratch_block__init(Scratch_Block *block, Arena *arena){
    block->temp = begin_temp(arena);
}

Scratch_Block::Scratch_Block(Temp_Memory t){
    this->temp = t;
}

Scratch_Block::Scratch_Block(Arena *arena){
    scratch_block__init(this, arena);
}

Scratch_Block::Scratch_Block(Application_Links *app){
    scratch_block__init(this, context_get_arena(app));
}

Scratch_Block::~Scratch_Block(){
    end_temp(this->temp);
}

Scratch_Block::operator Arena*(){
    return(this->temp.temp_memory_arena.arena);
}

void Scratch_Block::restore(void){
    end_temp(this->temp);
}

////////////////////////////////

internal Arena
make_arena_app_links(Application_Links *app, umem chunk_size, umem align){
    return(make_arena(context_get_base_allocator(app), chunk_size, align));
}

internal Arena
make_arena_app_links(Application_Links *app, umem chunk_size){
    return(make_arena_app_links(app, chunk_size, 8));
}

internal Arena
make_arena_app_links(Application_Links *app){
    return(make_arena_app_links(app, KB(16), 8));
}

// BOTTOM

