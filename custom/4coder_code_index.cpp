/*
4coder_code_index.cpp - Generic code indexing system for layout, definition jumps, etc.
*/

// TOP

global Code_Index global_code_index = {};

function void
code_index_init(void){
    global_code_index.mutex = system_mutex_make();
    global_code_index.node_arena = make_arena_system(KB(4));
    global_code_index.buffer_to_index_file =
        make_table_u64_u64(global_code_index.node_arena.base_allocator, 500);
}

function Code_Index_File_Storage*
code_index__alloc_storage(void){
    Code_Index_File_Storage *result = global_code_index.free_storage;
    if (result == 0){
        result = push_array_zero(&global_code_index.node_arena,
                                 Code_Index_File_Storage, 1);
    }
    else{
        sll_stack_pop(global_code_index.free_storage);
    }
    zdll_push_back(global_code_index.storage_first, global_code_index.storage_last,
                   result);
    global_code_index.storage_count += 1;
    return(result);
}

function void
code_index__free_storage(Code_Index_File_Storage *storage){
    zdll_remove(global_code_index.storage_first, global_code_index.storage_last,
                storage);
    global_code_index.storage_count -= 1;
    sll_stack_push(global_code_index.free_storage, storage);
}

function void
code_index_push_nest(Code_Index_Nest_List *list, Code_Index_Nest *nest){
    sll_queue_push(list->first, list->last, nest);
    list->count += 1;
}

function Code_Index_Nest_Ptr_Array
code_index_nest_ptr_array_from_list(Arena *arena,  Code_Index_Nest_List *list){
    Code_Index_Nest_Ptr_Array array = {};
    array.ptrs = push_array_zero(arena, Code_Index_Nest*, list->count);
    array.count = list->count;
    i32 counter = 0;
    for (Code_Index_Nest *node = list->first;
         node != 0;
         node = node->next){
        array.ptrs[counter] = node;
        counter += 1;
    }
    return(array);
}

function void
code_index_lock(void){
    system_mutex_acquire(global_code_index.mutex);
}

function void
code_index_unlock(void){
    system_mutex_release(global_code_index.mutex);
}

function void
code_index_set_file(Buffer_ID buffer, Arena arena, Code_Index_File *index){
    Code_Index_File_Storage *storage = 0;
    
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file,
                                       buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        storage = (Code_Index_File_Storage*)IntAsPtr(val);
        linalloc_clear(&storage->arena);
    }
    else{
        storage = code_index__alloc_storage();
        table_insert(&global_code_index.buffer_to_index_file, buffer,
                     (u64)PtrAsInt(storage));
    }
    storage->arena = arena;
    storage->file = index;
}

function void
code_index_erase_file(Buffer_ID buffer){
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file,
                                       buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        Code_Index_File_Storage *storage = (Code_Index_File_Storage*)IntAsPtr(val);
        linalloc_clear(&storage->arena);
        table_erase(&global_code_index.buffer_to_index_file, lookup);
        code_index__free_storage(storage);
    }
}

function Code_Index_File*
code_index_get_file(Buffer_ID buffer){
    Code_Index_File *result = 0;
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file,
                                       buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        result = (Code_Index_File*)IntAsPtr(val);
    }
    return(result);
}

function Code_Index_Nest*
code_index_get_nest(Code_Index_Nest_Ptr_Array *array, i64 pos){
    Code_Index_Nest *result = 0;
    i32 count = array->count;
    Code_Index_Nest **nest_ptrs = array->ptrs;
    for (i32 i = 0; i < count; i += 1){
        Code_Index_Nest *nest = nest_ptrs[i];
        if (nest->open.max <= pos && pos < nest->close.min){
            Code_Index_Nest *sub_nest =
                code_index_get_nest(&nest->nest_array, pos);
            if (sub_nest != 0){
                result = sub_nest;
            }
            else{
                result = nest;
            }
            break;
        }
    }
    return(result);
}

function Code_Index_Nest*
code_index_get_nest(Code_Index_Nest *nest, i64 pos){
    return(code_index_get_nest(&nest->nest_array, pos));
}

function Code_Index_Nest*
code_index_get_nest(Code_Index_File *file, i64 pos){
    return(code_index_get_nest(&file->nest_array, pos));
}

////////////////////////////////

function void
generic_parse_inc(Generic_Parse_State *state){
    if (!token_it_inc_all(&state->it)){
        state->finished = true;
    }
}

function void
generic_parse_skip_soft_tokens(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    for (;token != 0 && !state->finished;){
        if (token->kind == TokenBaseKind_Comment){
            state->handle_comment(state->app, state->arena, index, token, state->contents);
            token_it_inc_non_whitespace(&state->it);
            token = token_it_read(&state->it);
        }
        else if (token->kind == TokenBaseKind_Whitespace){
            Range_i64 range = Ii64(token);
            u8 *ptr = state->contents.str + range.one_past_last - 1;
            for (i64 i = range.one_past_last - 1;
                 i >= range.first;
                 i -= 1, ptr -= 1){
                if (*ptr == '\n'){
                    state->prev_line_start = ptr + 1;
                    break;
                }
            }
        }
        else{
            break;
        }
        generic_parse_inc(state);
        token = token_it_read(&state->it);
    }
}

function void
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens,
                   Generic_Parse_Comment_Function *handle_comment, Generic_Parse_State *state){
    state->app = app;
    state->arena = arena;
    state->contents = contents;
    state->it = token_iterator(0, tokens);
    state->handle_comment = handle_comment;
    state->prev_line_start = contents.str;
}

function Code_Index_Nest*
generic_parse_parenthical(Code_Index_File *index, Generic_Parse_State *state,
                          i64 indentation);

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state,
                    i64 indentation){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    result->file = index;
    
    result->interior_indentation = indentation + 4;
    result->close_indentation = indentation;
    indentation = result->interior_indentation;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_parenthical(index, state,
                                                              indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ScopeClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        else{
            generic_parse_inc(state);
        }
    }
    
    result->nest_array =
        code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    return(result);
}

function Code_Index_Nest*
generic_parse_parenthical(Code_Index_File *index, Generic_Parse_State *state,
                          i64 indentation){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Paren;
    result->open = Ii64(token);
    result->file = index;
    
    i64 manifested_characters_on_line = 0;
    {
        u8 *ptr = state->prev_line_start;
        u8 *end_ptr = state->contents.str + token->pos;
        // NOTE(allen): Initial whitespace
        for (;ptr < end_ptr; ptr += 1){
            if (!character_is_whitespace(*ptr)){
                break;
            }
        }
        // NOTE(allen): Manifested characters
        manifested_characters_on_line = (i64)(end_ptr - ptr) + token->size;
    }
    
    indentation += manifested_characters_on_line;
    result->interior_indentation = indentation;
    result->close_indentation = indentation;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_parenthical(index, state,
                                                              indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        else{
            generic_parse_inc(state);
        }
    }
    
    result->nest_array =
        code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    return(result);
}

function b32
generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit){
    b32 result = false;
    
    i64 indentation = 0;
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        Token *token = token_it_read(&state->it);
        
        if (token == 0 || state->finished){
            result = true;
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_parenthical(index, state, indentation);
            code_index_push_nest(&index->nest_list, nest);
        }
        else{
            generic_parse_inc(state);
        }
        
        i64 index = token_it_index(&state->it);
        if (index >= one_past_last_index){
            token = token_it_read(&state->it);
            if (token == 0){
                result = true;
            }
            break;
        }
    }
    
    if (result){
        index->nest_array =
            code_index_nest_ptr_array_from_list(state->arena, &index->nest_list);
    }
    
    return(result);
}

////////////////////////////////

function void
default_comment_index(Application_Links *app, Arena *arena, Code_Index_File *index,
                      Token *token, String_Const_u8 contents){
    
}

function void
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens,
                   Generic_Parse_State *state){
    generic_parse_init(app, arena, contents, tokens, default_comment_index, state);
}


// BOTTOM

