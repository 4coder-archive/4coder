/*
4coder_code_index.cpp - Generic code indexing system for layout, definition jumps, etc.
*/

// TOP

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
code_index_set_file(Application_Links *app, Buffer_ID buffer, Arena arena, Code_Index_File *index){
    NotImplemented;
}

////////////////////////////////

function void
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens,
                   Generic_Parse_Comment_Function *handle_comment, Generic_Parse_State *state){
    state->app = app;
    state->arena = arena;
    state->contents = contents;
    state->it = token_iterator(0, tokens);
    state->handle_comment = handle_comment;
    
    Token *token = token_it_read(&state->it);
    if (token != 0 && token->kind == TokenBaseKind_Whitespace){
        token_it_inc_non_whitespace(&state->it);
    }
}

function Token*
generic_parse_read_token(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    for (;token != 0 && token->kind == TokenBaseKind_Comment;){
        state->handle_comment(state->app, state->arena, index, token, state->contents);
        token_it_inc_non_whitespace(&state->it);
        token = token_it_read(&state->it);
    }
    return(token);
}

function Code_Index_Nest*
generic_parse_parenthical(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    
    for (;;){
        token = generic_parse_read_token(index, state);
        if (token == 0){
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_parenthical(index, state);
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ScopeClose){
            result->close = Ii64(token);
            break;
        }
        else{
            token_it_inc_non_whitespace(&state->it);
        }
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    return(result);
}

function Code_Index_Nest*
generic_parse_parenthical(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Paren;
    result->open = Ii64(token);
    
    for (;;){
        token = generic_parse_read_token(index, state);
        if (token == 0){
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_parenthical(index, state);
            code_index_push_nest(&result->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalClose){
            result->close = Ii64(token);
            break;
        }
        else{
            token_it_inc_non_whitespace(&state->it);
        }
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    return(result);
}

function b32
generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit){
    b32 result = false;
    
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        Token *token = generic_parse_read_token(index, state);
        
        if (token == 0){
            result = true;
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_parenthical(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else{
            token_it_inc_non_whitespace(&state->it);
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

