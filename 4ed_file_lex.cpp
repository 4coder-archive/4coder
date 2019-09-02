/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2018
 *
 * Ways of calling lexer for the text in Editing_File
 *
 */

// TOP

internal void
file_kill_tokens(System_Functions *system, Heap *heap, Editing_File *file){
    file->settings.tokens_exist = false;
    if (file->state.token_array.tokens != 0){
        heap_free(heap, file->state.token_array.tokens);
    }
    block_zero_struct(&file->state.token_array);
}

internal void
file_first_lex_serial(System_Functions *system, Models *models, Editing_File *file){
    Mem_Options *mem = &models->mem;
    Arena *scratch = &mem->arena;
    Heap *heap = &mem->heap;
    file->settings.tokens_exist = true;
    
    if (file->is_loading == 0){
        Assert(file->state.token_array.tokens == 0);
        
        Temp_Memory temp = begin_temp(scratch);
        
        Parse_Context parse_context = parse_context_get(scratch, &models->parse_context_memory, file->settings.parse_context_id);
        Assert(parse_context.valid);
        
        Gap_Buffer *buffer = &file->state.buffer;
        i64 text_size = buffer_size(buffer);
        
        // TODO(allen): REWRITE REWRITE REWRITE
        Cpp_Token_Array new_tokens = {};
        new_tokens.max_count = Million(1);
        new_tokens.count = 0;
        new_tokens.tokens = push_array(scratch, Cpp_Token, new_tokens.max_count);
        
        b32 still_lexing = true;
        
        Cpp_Lex_Data lex = cpp_lex_data_init(file->settings.tokens_without_strings, parse_context.kw_table, parse_context.pp_table);
        
        List_String_Const_u8 chunks = buffer_get_chunks(scratch, buffer);
        u8 null_terminator = 0;
        string_list_push(scratch, &chunks, SCu8(&null_terminator, 1));
        
        Cpp_Token_Array new_array = {};
        
        for (Node_String_Const_u8 *node = chunks.first;
             node != 0;){
            u8 *chunk = node->string.str;
            umem chunk_size = node->string.size;
            
            i32 result = cpp_lex_step(&lex, (char*)chunk, (i32)chunk_size, (i32)text_size, &new_tokens, NO_OUT_LIMIT);
            
            switch (result){
                case LexResult_NeedChunk:
                {
                    node = node->next;
                }break;
                
                case LexResult_Finished:
                case LexResult_NeedTokenMemory:
                {
                    u32 new_max = round_up_u32(new_array.count + new_tokens.count + 1, KB(1));
                    if (new_array.tokens == 0){
                        new_array.tokens = heap_array(heap, Cpp_Token, new_max);
                    }
                    else{
                        u32 old_count = new_array.count;
                        Cpp_Token *new_token_mem = heap_array(heap, Cpp_Token, new_max);
                        memcpy(new_token_mem, new_array.tokens, sizeof(*new_token_mem)*old_count);
                        heap_free(heap, new_array.tokens);
                        new_array.tokens = new_token_mem;
                    }
                    new_array.max_count = new_max;
                    
                    Assert(new_array.count + new_tokens.count <= new_array.max_count);
                    memcpy(new_array.tokens + new_array.count, new_tokens.tokens, new_tokens.count*sizeof(Cpp_Token));
                    new_array.count += new_tokens.count;
                    new_tokens.count = 0;
                    
                    if (result == LexResult_Finished){
                        still_lexing = false;
                    }
                }break;
                
                case LexResult_HitTokenLimit: InvalidPath;
            }
            
            if (!still_lexing){
                break;
            }
        }
        
        Cpp_Token_Array *token_array = &file->state.token_array;
        token_array->count = new_array.count;
        token_array->max_count = new_array.max_count;
        if (token_array->tokens != 0){
            heap_free(heap, token_array->tokens);
        }
        token_array->tokens = new_array.tokens;
        
        new_array.tokens = 0;
        new_array.count = 0;
        new_array.max_count = 0;
        
        end_temp(temp);
        
        file_mark_edit_finished(&models->working_set, file);
    }
}

internal b32
file_relex_serial(System_Functions *system, Models *models, Editing_File *file, i64 start_i, i64 end_i, i64 shift_amount){
    Mem_Options *mem = &models->mem;
    Arena *scratch = &mem->arena;
    Heap *heap = &mem->heap;
    
    if (file->state.token_array.tokens == 0){
        file_first_lex_serial(system, models, file);
    }
    else{
        Gap_Buffer *buffer = &file->state.buffer;
        Cpp_Token_Array *array = &file->state.token_array;
        
        Temp_Memory temp = begin_temp(scratch);
        Parse_Context parse_context = parse_context_get(scratch, &models->parse_context_memory, file->settings.parse_context_id);
        Assert(parse_context.valid);
        
        Cpp_Token_Array relex_array = {};
        relex_array.count = 0;
        relex_array.max_count = Million(1);
        relex_array.tokens = push_array(scratch, Cpp_Token, relex_array.max_count);
        
        i64 size = buffer_size(buffer);
        
        Cpp_Relex_Data state = cpp_relex_init(array, (i32)start_i, (i32)end_i, (i32)shift_amount, file->settings.tokens_without_strings, parse_context.kw_table, parse_context.pp_table);
        
        List_String_Const_u8 chunks = buffer_get_chunks(scratch, buffer);
        u8 null_terminator = 0;
        string_list_push(scratch, &chunks, SCu8(&null_terminator, 1));
        Node_String_Const_u8 *node = chunks.first;
        
        i32 chunk_index = 0;
        u8 *chunk = 0;
        umem chunk_size = 0;
        if (node != 0){
            chunk = node->string.str;
            chunk_size = node->string.size;
        }
        for (;!cpp_relex_is_start_chunk(&state, (char*)chunk, (i32)chunk_size);){
            node = node->next;
            Assert(node != 0);
            chunk = node->string.str;
            chunk_size = node->string.size;
        }
        for(;;){
            Cpp_Lex_Result lex_result = cpp_relex_step(&state, (char*)chunk, (i32)chunk_size, (i32)size, array, &relex_array);
            
            switch (lex_result){
                case LexResult_NeedChunk:
                {
                    node = node->next;
                    Assert(node != 0);
                    chunk = node->string.str;
                    chunk_size = node->string.size;
                }break;
                
                case LexResult_NeedTokenMemory: InvalidPath;
                
                case LexResult_Finished: goto doublebreak;
            }
        }
        doublebreak:;
        
        i32 new_count = cpp_relex_get_new_count(&state, array->count, &relex_array);
        if (new_count > array->max_count){
            i32 new_max = round_up_i32(new_count, KB(1));
            Cpp_Token *new_tokens = heap_array(heap, Cpp_Token, new_max);
            memcpy(new_tokens, array->tokens, array->count*sizeof(Cpp_Token));
            heap_free(heap, array->tokens);
            array->tokens = new_tokens;
            array->max_count = new_max;
        }
        
        cpp_relex_complete(&state, array, &relex_array);
        
        end_temp(temp);
    }
    
    return(true);
}

internal void
file_first_lex(System_Functions *system, Models *models, Editing_File *file){
    file_first_lex_serial(system, models, file);
}

internal void
file_relex(System_Functions *system, Models *models, Editing_File *file, i64 start, i64 end, i64  shift_amount){
    file_relex_serial(system, models, file, start, end, shift_amount);
}

// BOTTOM

