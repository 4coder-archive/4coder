/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2018
 *
 * Ways of calling lexer for the text in Editing_File
 *
 */

// TOP

internal String_Array
file_lex_chunks(Partition *part, Gap_Buffer *buffer){
    String_Array result = {};
    result.vals = push_array(part, String, 0);
    buffer_get_chunks(part, buffer);
    {
        String *s = push_array(part, String, 1);
        block_zero(s, sizeof(*s));
    }
    result.count = (i32)(push_array(part, String, 0) - result.vals);
    return(result);
}

internal void
job_full_lex(System_Functions *system, Thread_Context *thread, Thread_Memory *memory, void *data[4]){
    Editing_File *file = (Editing_File*)data[0];
    Heap *heap = (Heap*)data[1];
    Models *models = (Models*)data[2];
    
    Parse_Context parse_context = parse_context_get(&models->parse_context_memory, file->settings.parse_context_id, memory->data, memory->size);
    if (!parse_context.valid){
        return;
    }
    
    Gap_Buffer *buffer = &file->state.buffer;
    i32 text_size = buffer_size(buffer);
    
    u32 aligned_buffer_size = (text_size + 3)&(~3);
    
    for (;memory->size < aligned_buffer_size + parse_context.memory_size;){
        void *old_base = memory->data;
        system->grow_thread_memory(memory);
        parse_context_rebase(&parse_context, old_base, memory->data);
    }
    
    u8 *data_ptr = (u8*)memory->data;
    umem data_size = memory->size;
    data_ptr += parse_context.memory_size;
    data_size -= parse_context.memory_size;
    
    Cpp_Token_Array tokens = {};
    tokens.tokens = (Cpp_Token*)(data_ptr);
    tokens.max_count = (u32)(data_size / sizeof(Cpp_Token));
    tokens.count = 0;
    
    b32 still_lexing = true;
    
    Cpp_Lex_Data lex = cpp_lex_data_init(file->settings.tokens_without_strings, parse_context.kw_table, parse_context.pp_table);
    
    String chunk_space[3];
    Partition chunk_part = make_part(chunk_space, sizeof(chunk_space));
    String_Array chunks = file_lex_chunks(&chunk_part, buffer);
    
    i32 chunk_index = 0;
    
    do{
        char *chunk = chunks.vals[chunk_index].str;
        i32 chunk_size = chunks.vals[chunk_index].size;
        
        i32 result = cpp_lex_step(&lex, chunk, chunk_size, text_size, &tokens, 2048);
        switch (result){
            case LexResult_NeedChunk:
            {
                ++chunk_index;
                Assert(chunk_index < chunks.count);
            }break;
            
            case LexResult_NeedTokenMemory:
            {
                if (system->check_cancel(thread)){
                    return;
                }
                
                void *old_base = memory->data;
                system->grow_thread_memory(memory);
                cpp_rebase_tables(&lex, old_base, memory->data);
                
                data_ptr = (u8*)memory->data;
                data_size = memory->size;
                data_ptr += parse_context.memory_size;
                data_size -= parse_context.memory_size;
                tokens.tokens = (Cpp_Token*)(data_ptr);
                tokens.max_count = (u32)(data_size / sizeof(Cpp_Token));
            }break;
            
            case LexResult_HitTokenLimit:
            {
                if (system->check_cancel(thread)){
                    return;
                }
            }break;
            
            case LexResult_Finished:
            {
                still_lexing = false;
            }break;
        }
    }while(still_lexing);
    
    i32 new_max = l_round_up_i32(tokens.count+1, KB(1));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Assert(file->state.swap_array.tokens == 0);
        file->state.swap_array.tokens = heap_array(heap, Cpp_Token, new_max);
    }
    system->release_lock(FRAME_LOCK);
    
    u8 *dest = (u8*)file->state.swap_array.tokens;
    u8 *src = (u8*)tokens.tokens;
    
    memcpy(dest, src, tokens.count*sizeof(Cpp_Token));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Cpp_Token_Array *file_token_array = &file->state.token_array;
        file_token_array->count = tokens.count;
        file_token_array->max_count = new_max;
        if (file_token_array->tokens){
            heap_free(heap, file_token_array->tokens);
        }
        file_token_array->tokens = file->state.swap_array.tokens;
        file->state.swap_array.tokens = 0;
    }
    file->state.tokens_complete = true;
    file->state.still_lexing = false;
    file_mark_edit_finished(&models->working_set, file);
    system->release_lock(FRAME_LOCK);
}

internal void
file_kill_tokens(System_Functions *system, Heap *heap, Editing_File *file){
    file->settings.tokens_exist = false;
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            heap_free(heap, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
    }
    if (file->state.token_array.tokens){
        heap_free(heap, file->state.token_array.tokens);
    }
    file->state.tokens_complete = false;
    file->state.token_array = null_cpp_token_array;
}

internal void
file_first_lex_parallel(System_Functions *system, Models *models, Editing_File *file){
    Heap *heap = &models->mem.heap;
    file->settings.tokens_exist = true;
    
    if (!file->is_loading && !file->state.still_lexing){
        Assert(file->state.token_array.tokens == 0);
        
        file->state.tokens_complete = false;
        file->state.still_lexing = true;
        
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = heap;
        job.data[2] = models;
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
    }
}

internal void
file_first_lex_serial(System_Functions *system, Models *models, Editing_File *file){
    Mem_Options *mem = &models->mem;
    Partition *part = &mem->part;
    Heap *heap = &mem->heap;
    file->settings.tokens_exist = true;
    
    Assert(!file->state.still_lexing);
    
    if (file->is_loading == 0){
        Assert(file->state.token_array.tokens == 0);
        
        Temp_Memory temp = begin_temp_memory(part);
        
        Parse_Context parse_context = parse_context_get(&models->parse_context_memory, file->settings.parse_context_id, push_array(part, u8, 0), part_remaining(part));
        Assert(parse_context.valid);
        push_array(part, char, (i32)parse_context.memory_size);
        
        Gap_Buffer *buffer = &file->state.buffer;
        i32 text_size = buffer_size(buffer);
        
        i32 mem_size = part_remaining(part);
        
        Cpp_Token_Array new_tokens;
        new_tokens.max_count = mem_size/sizeof(Cpp_Token);
        new_tokens.count = 0;
        new_tokens.tokens = push_array(part, Cpp_Token, new_tokens.max_count);
        
        b32 still_lexing = true;
        
        Cpp_Lex_Data lex = cpp_lex_data_init(file->settings.tokens_without_strings, parse_context.kw_table, parse_context.pp_table);
        
        String chunk_space[3];
        Partition chunk_part = make_part(chunk_space, sizeof(chunk_space));
        String_Array chunks = file_lex_chunks(&chunk_part, buffer);
        
        i32 chunk_index = 0;
        
        Cpp_Token_Array *swap_array = &file->state.swap_array;
        
        do{
            char *chunk = chunks.vals[chunk_index].str;
            i32 chunk_size = chunks.vals[chunk_index].size;
            
            i32 result = cpp_lex_step(&lex, chunk, chunk_size, text_size, &new_tokens, NO_OUT_LIMIT);
            
            switch (result){
                case LexResult_NeedChunk:
                {
                    ++chunk_index;
                    Assert(chunk_index < chunks.count);
                }break;
                
                case LexResult_Finished:
                case LexResult_NeedTokenMemory:
                {
                    u32 new_max = l_round_up_u32(swap_array->count + new_tokens.count + 1, KB(1));
                    if (swap_array->tokens == 0){
                        swap_array->tokens = heap_array(heap, Cpp_Token, new_max);
                    }
                    else{
                        u32 old_count = swap_array->count;
                        Cpp_Token *new_token_mem = heap_array(heap, Cpp_Token, new_max);
                        memcpy(new_token_mem, swap_array->tokens, sizeof(*new_token_mem)*old_count);
                        heap_free(heap, swap_array->tokens);
                        swap_array->tokens = new_token_mem;
                    }
                    swap_array->max_count = new_max;
                    
                    Assert(swap_array->count + new_tokens.count <= swap_array->max_count);
                    memcpy(swap_array->tokens + swap_array->count, new_tokens.tokens, new_tokens.count*sizeof(Cpp_Token));
                    swap_array->count += new_tokens.count;
                    new_tokens.count = 0;
                    
                    if (result == LexResult_Finished){
                        still_lexing = false;
                    }
                }break;
                
                case LexResult_HitTokenLimit: InvalidCodePath;
            }
        } while (still_lexing);
        
        Cpp_Token_Array *token_array = &file->state.token_array;
        token_array->count = swap_array->count;
        token_array->max_count = swap_array->max_count;
        if (token_array->tokens != 0){
            heap_free(heap, token_array->tokens);
        }
        token_array->tokens = swap_array->tokens;
        
        swap_array->tokens = 0;
        swap_array->count = 0;
        swap_array->max_count = 0;
        
        file->state.tokens_complete = true;
        file->state.still_lexing = false;
        
        end_temp_memory(temp);
        
        file->state.tokens_complete = true;
        file_mark_edit_finished(&models->working_set, file);
    }
}

internal b32
file_relex_parallel(System_Functions *system, Models *models, Editing_File *file, i32 start_i, i32 end_i, i32 shift_amount){
    Mem_Options *mem = &models->mem;
    Heap *heap = &mem->heap;
    Partition *part = &mem->part;
    
    if (file->state.token_array.tokens == 0){
        file_first_lex_parallel(system, models, file);
        return(true);
    }
    
    b32 result = true;
    b32 internal_lex = !file->state.still_lexing;
    if (internal_lex){
        Gap_Buffer *buffer = &file->state.buffer;
        i32 extra_tolerance = 100;
        
        Cpp_Token_Array *array = &file->state.token_array;
        Cpp_Relex_Range relex_range = cpp_get_relex_range(array, start_i, end_i);
        
        i32 relex_space_size =
            relex_range.end_token_index - relex_range.start_token_index + extra_tolerance;
        
        Temp_Memory temp = begin_temp_memory(part);
        Parse_Context parse_context = parse_context_get(&models->parse_context_memory, file->settings.parse_context_id, push_array(part, u8, 0), part_remaining(part));
        Assert(parse_context.valid);
        push_array(part, char, (i32)parse_context.memory_size);
        
        Cpp_Token_Array relex_array;
        relex_array.count = 0;
        relex_array.max_count = relex_space_size;
        relex_array.tokens = push_array(part, Cpp_Token, relex_array.max_count);
        
        i32 size = buffer_size(buffer);
        
        Cpp_Relex_Data state = cpp_relex_init(array, start_i, end_i, shift_amount, file->settings.tokens_without_strings, parse_context.kw_table, parse_context.pp_table);
        
        String chunk_space[3];
        Partition chunk_part = make_part(chunk_space, sizeof(chunk_space));
        String_Array chunks = file_lex_chunks(&chunk_part, buffer);
        
        i32 chunk_index = 0;
        char *chunk = chunks.vals[chunk_index].str;
        i32 chunk_size = chunks.vals[chunk_index].size;
        
        for (;!cpp_relex_is_start_chunk(&state, chunk, chunk_size);){
            ++chunk_index;
            Assert(chunk_index < chunks.count);
            chunk = chunks.vals[chunk_index].str;
            chunk_size = chunks.vals[chunk_index].size;
        }
        
        for (;;){
            Cpp_Lex_Result lex_result =
                cpp_relex_step(&state, chunk, chunk_size, size, array, &relex_array);
            
            switch (lex_result){
                case LexResult_NeedChunk:
                {
                    ++chunk_index;
                    Assert(chunk_index < chunks.count);
                    chunk = chunks.vals[chunk_index].str;
                    chunk_size = chunks.vals[chunk_index].size;
                }break;
                
                case LexResult_NeedTokenMemory:
                {
                    internal_lex = false;
                }goto doublebreak;
                
                case LexResult_Finished: goto doublebreak;
            }
        }
        doublebreak:;
        
        if (internal_lex){
            i32 new_count = cpp_relex_get_new_count(&state, array->count, &relex_array);
            if (new_count > array->max_count){
                i32 new_max = l_round_up_i32(new_count, KB(1));
                void *memory = heap_allocate(heap, new_max*sizeof(Cpp_Token));
                memcpy(memory, array->tokens, array->count*sizeof(Cpp_Token));
                heap_free(heap, array->tokens);
                array->tokens = (Cpp_Token*)memory;
                array->max_count = new_max;
            }
            
            cpp_relex_complete(&state, array, &relex_array);
            file_mark_edit_finished(&models->working_set, file);
        }
        else{
            cpp_relex_abort(&state, array);
        }
        
        end_temp_memory(temp);
    }
    
    if (!internal_lex){
        Cpp_Token_Array *array = &file->state.token_array;
        Cpp_Get_Token_Result get_token_result = cpp_get_token(*array, end_i);
        i32 end_token_i = get_token_result.token_index;
        
        if (end_token_i < 0){
            end_token_i = 0;
        }
        else if (end_i > array->tokens[end_token_i].start){
            ++end_token_i;
        }
        
        cpp_shift_token_starts(array, end_token_i, shift_amount);
        --end_token_i;
        if (end_token_i >= 0){
            Cpp_Token *token = array->tokens + end_token_i;
            if (token->start < end_i && token->start + token->size > end_i){
                token->size += shift_amount;
            }
        }
        
        file->state.still_lexing = true;
        
        Job_Data job = {};
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = heap;
        job.data[2] = models;
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
        result = false;
    }
    
    return(result);
}

internal b32
file_relex_serial(System_Functions *system, Models *models, Editing_File *file, i32 start_i, i32 end_i, i32 shift_amount){
    Mem_Options *mem = &models->mem;
    Heap *heap = &mem->heap;
    Partition *part = &mem->part;
    
    if (file->state.token_array.tokens == 0){
        file_first_lex_serial(system, models, file);
        return(true);
    }
    
    Assert(!file->state.still_lexing);
    
    Gap_Buffer *buffer = &file->state.buffer;
    Cpp_Token_Array *array = &file->state.token_array;
    
    Temp_Memory temp = begin_temp_memory(part);
    Parse_Context parse_context = parse_context_get(&models->parse_context_memory, file->settings.parse_context_id, push_array(part, u8, 0), part_remaining(part));
    Assert(parse_context.valid);
    push_array(part, char, (i32)parse_context.memory_size);
    
    Cpp_Token_Array relex_array;
    relex_array.count = 0;
    relex_array.max_count = part_remaining(part) / sizeof(Cpp_Token);
    relex_array.tokens = push_array(part, Cpp_Token, relex_array.max_count);
    
    i32 size = buffer_size(buffer);
    
    Cpp_Relex_Data state = cpp_relex_init(array, start_i, end_i, shift_amount, file->settings.tokens_without_strings, parse_context.kw_table, parse_context.pp_table);
    
    String chunk_space[3];
    Partition chunk_part = make_part(chunk_space, sizeof(chunk_space));
    String_Array chunks = file_lex_chunks(&chunk_part, buffer);
    
    i32 chunk_index = 0;
    char *chunk = chunks.vals[chunk_index].str;
    i32 chunk_size = chunks.vals[chunk_index].size;
    
    for (;!cpp_relex_is_start_chunk(&state, chunk, chunk_size);){
        ++chunk_index;
        Assert(chunk_index < chunks.count);
        chunk = chunks.vals[chunk_index].str;
        chunk_size = chunks.vals[chunk_index].size;
    }
    
    for(;;){
        Cpp_Lex_Result lex_result = cpp_relex_step(&state, chunk, chunk_size, size, array, &relex_array);
        
        switch (lex_result){
            case LexResult_NeedChunk:
            {
                ++chunk_index;
                Assert(chunk_index < chunks.count);
                chunk = chunks.vals[chunk_index].str;
                chunk_size = chunks.vals[chunk_index].size;
            }break;
            
            case LexResult_NeedTokenMemory: InvalidCodePath;
            
            case LexResult_Finished: goto doublebreak;
        }
    }
    doublebreak:;
    
    i32 new_count = cpp_relex_get_new_count(&state, array->count, &relex_array);
    if (new_count > array->max_count){
        i32 new_max = l_round_up_i32(new_count, KB(1));
        Cpp_Token *new_tokens = heap_array(heap, Cpp_Token, new_max);
        memcpy(new_tokens, array->tokens, array->count*sizeof(Cpp_Token));
        heap_free(heap, array->tokens);
        array->tokens = new_tokens;
        array->max_count = new_max;
    }
    
    cpp_relex_complete(&state, array, &relex_array);
    
    end_temp_memory(temp);
    
    return(true);
}

internal void
file_first_lex(System_Functions *system, Models *models, Editing_File *file){
    if (!file->settings.virtual_white){
        file_first_lex_parallel(system, models, file);
    }
    else{
        file_first_lex_serial(system, models, file);
    }
}

internal void
file_relex(System_Functions *system, Models *models, Editing_File *file, i32 start, i32 end, i32 shift_amount){
    if (!file->settings.virtual_white){
        file_relex_parallel(system, models, file, start, end, shift_amount);
    }
    else{
        file_relex_serial(system, models, file, start, end, shift_amount);
    }
}

// BOTTOM

