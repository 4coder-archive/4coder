/*
4coder_code_index.cpp - Generic code indexing system for layout, definition jumps, etc.
*/

// TOP

global Code_Index global_code_index = {};

function void
code_index_init(void){
    global_code_index.mutex = system_mutex_make();
    global_code_index.node_arena = make_arena_system(KB(4));
    global_code_index.buffer_to_index_file = make_table_u64_u64(global_code_index.node_arena.base_allocator, 500);
}

function Code_Index_File_Storage*
code_index__alloc_storage(void){
    Code_Index_File_Storage *result = global_code_index.free_storage;
    if (result == 0){
        result = push_array_zero(&global_code_index.node_arena, Code_Index_File_Storage, 1);
    }
    else{
        sll_stack_pop(global_code_index.free_storage);
    }
    zdll_push_back(global_code_index.storage_first, global_code_index.storage_last, result);
    global_code_index.storage_count += 1;
    return(result);
}

function void
code_index__free_storage(Code_Index_File_Storage *storage){
    zdll_remove(global_code_index.storage_first, global_code_index.storage_last, storage);
    global_code_index.storage_count -= 1;
    sll_stack_push(global_code_index.free_storage, storage);
}

function void
code_index_push_nest(Code_Index_Nest_List *list, Code_Index_Nest *nest){
    sll_queue_push(list->first, list->last, nest);
    list->count += 1;
}

function Code_Index_Nest_Ptr_Array
code_index_nest_ptr_array_from_list(Arena *arena, Code_Index_Nest_List *list){
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

function Code_Index_Note_Ptr_Array
code_index_note_ptr_array_from_list(Arena *arena, Code_Index_Note_List *list){
    Code_Index_Note_Ptr_Array array = {};
    array.ptrs = push_array_zero(arena, Code_Index_Note*, list->count);
    array.count = list->count;
    i32 counter = 0;
    for (Code_Index_Note *node = list->first;
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
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file, buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        storage = (Code_Index_File_Storage*)IntAsPtr(val);
        linalloc_clear(&storage->arena);
    }
    else{
        storage = code_index__alloc_storage();
        table_insert(&global_code_index.buffer_to_index_file, buffer, (u64)PtrAsInt(storage));
    }
    storage->arena = arena;
    storage->file = index;
}

function void
code_index_erase_file(Buffer_ID buffer){
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file, buffer);
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
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file, buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        Code_Index_File_Storage *storage = (Code_Index_File_Storage*)IntAsPtr(val);
        result = storage->file;
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
        if (nest->open.max <= pos && pos <= nest->close.min){
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

function void
index_shift(i64 *ptr, Range_i64 old_range, u64 new_size){
    i64 i = *ptr;
    if (old_range.min <= i && i < old_range.max){
        *ptr = old_range.first;
    }
    else if (old_range.max <= i){
        *ptr = i + new_size - (old_range.max - old_range.min);
    }
}

function void
code_index_shift(Code_Index_Nest_Ptr_Array *array,
                 Range_i64 old_range, u64 new_size){
    i32 count = array->count;
    Code_Index_Nest **nest_ptr = array->ptrs;
    for (i32 i = 0; i < count; i += 1, nest_ptr += 1){
        Code_Index_Nest *nest = *nest_ptr;
        index_shift(&nest->open.min, old_range, new_size);
        index_shift(&nest->open.max, old_range, new_size);
        if (nest->is_closed){
            index_shift(&nest->close.min, old_range, new_size);
            index_shift(&nest->close.max, old_range, new_size);
        }
        code_index_shift(&nest->nest_array, old_range, new_size);
    }
}

function void
code_index_shift(Code_Index_File *file, Range_i64 old_range, u64 new_size){
    code_index_shift(&file->nest_array, old_range, new_size);
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
        if (state->in_preprocessor && !HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
            break;
        }
        if (token->kind == TokenBaseKind_Comment){
            state->handle_comment(state->app, state->arena, index, token, state->contents);
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
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens, Generic_Parse_Comment_Function *handle_comment, Generic_Parse_State *state){
    state->app = app;
    state->arena = arena;
    state->contents = contents;
    state->it = token_iterator(0, tokens);
    state->handle_comment = handle_comment;
    state->prev_line_start = contents.str;
}

////////////////////////////////

#if 0
/*
// NOTE(allen): grammar syntax
(X) = X
X Y = X and then Y
X? = zero or one X
$X = check for X but don't consume
[X] = zero or more Xs
X | Y = either X or Y
* = anything that does not match previous options in a X | Y | ... chain
* - X = anything that does not match X or previous options in a Y | Z | ... chain
<X> = a token of type X
"X" = literally the string "X"
X{Y} = X with flag Y

// NOTE(allen): grammar of code index parse
file: [preprocessor | scope | parens | function | type | * - <end-of-file>] <end-of-file>
preprocessor: <preprocessor> [scope | parens | stmnt]{pp-body}
scope: <scope-open> [preprocessor | scope | parens | * - <scope-close>] <scope-close>
paren: <paren-open> [preprocessor | scope | parens | * - <paren-close>] <paren-close>
stmnt-close-pattern: <scope-open> | <scope-close> | <paren-open> | <paren-close> | <stmnt-close> | <preprocessor>
stmnt: [type | * - stmnt-close-pattern] stmnt-close-pattern
type: struct | union | enum | typedef
struct: "struct" <identifier> $(";" | "{")
union: "union" <identifier> $(";" | "{")
enum: "enum" <identifier> $(";" | "{")
typedef: "typedef" [* - (<identifier> (";" | "("))] <identifier> $(";" | "(")
function: <identifier> >"(" ["(" ")" | * - ("(" | ")")] ")" ("{" | ";")
*/
#endif

////////////////////////////////

function Code_Index_Note*
index_new_note(Code_Index_File *index, Generic_Parse_State *state, Range_i64 range, Code_Index_Note_Kind kind, Code_Index_Nest *parent){
    Code_Index_Note *result = push_array(state->arena, Code_Index_Note, 1);
    sll_queue_push(index->note_list.first, index->note_list.last, result);
    index->note_list.count += 1;
    result->note_kind = kind;
    result->pos = range;
    result->text = push_string_copy(state->arena, string_substring(state->contents, range));
    result->file = index;
    result->parent = parent;
    return(result);
}

function void
cpp_parse_type_structure(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent){
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    if (state->finished){
        return;
    }
    Token *token = token_it_read(&state->it);
    if (token != 0 && token->kind == TokenBaseKind_Identifier){
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        Token *peek = token_it_read(&state->it);
        if (peek != 0 && peek->kind == TokenBaseKind_StatementClose ||
            peek->kind == TokenBaseKind_ScopeOpen){
            index_new_note(index, state, Ii64(token), CodeIndexNote_Type, parent);
        }
    }
}

function void
cpp_parse_type_def(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent){
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    for (;;){
        b32 did_advance = false;
        Token *token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        if (token->kind == TokenBaseKind_Identifier){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            did_advance = true;
            Token *peek = token_it_read(&state->it);
            if (peek != 0 && peek->kind == TokenBaseKind_StatementClose ||
                peek->kind == TokenBaseKind_ParentheticalOpen){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Type, parent);
                break;
            }
        }
        else if (token->kind == TokenBaseKind_StatementClose ||
                 token->kind == TokenBaseKind_ScopeOpen ||
                 token->kind == TokenBaseKind_ScopeClose ||
                 token->kind == TokenBaseKind_ScopeOpen ||
                 token->kind == TokenBaseKind_ScopeClose){
            break;
        }
        else if (token->kind == TokenBaseKind_Keyword){
            String_Const_u8 lexeme = string_substring(state->contents, Ii64(token));
            if (string_match(lexeme, string_u8_litexpr("struct")) ||
                string_match(lexeme, string_u8_litexpr("union")) ||
                string_match(lexeme, string_u8_litexpr("enum"))){
                break;
            }
        }
        if (!did_advance){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
        }
    }
}

function void
cpp_parse_function(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent){
    Token *token = token_it_read(&state->it);
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    if (state->finished){
        return;
    }
    Token *peek = token_it_read(&state->it);
    Token *reset_point = peek;
    if (peek != 0 && peek->sub_kind == TokenCppKind_ParenOp){
        b32 at_paren_close = false;
        i32 paren_nest_level = 0;
        for (; peek != 0;){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            peek = token_it_read(&state->it);
            if (peek == 0 || state->finished){
                break;
            }
            
            if (peek->kind == TokenBaseKind_ParentheticalOpen){
                paren_nest_level += 1;
            }
            else if (peek->kind == TokenBaseKind_ParentheticalClose){
                if (paren_nest_level > 0){
                    paren_nest_level -= 1;
                }
                else{
                    at_paren_close = true;
                    break;
                }
            }
        }
        
        if (at_paren_close){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            peek = token_it_read(&state->it);
            if (peek != 0 &&
                peek->kind == TokenBaseKind_ScopeOpen ||
                peek->kind == TokenBaseKind_StatementClose){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Function, parent);
            }
        }
    }
    state->it = token_iterator(state->it.user_id, state->it.tokens, state->it.count, reset_point);
}

function Code_Index_Nest*
generic_parse_statement(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_preprocessor(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_paren(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_statement(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Statement;
    result->open = Ii64(token->pos);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->in_statement = true;
    
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                result->is_closed = true;
                result->close = Ii64(token->pos);
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                result->is_closed = true;
                result->close = Ii64(token->pos);
                break;
            }
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen ||
            token->kind == TokenBaseKind_ScopeClose ||
            token->kind == TokenBaseKind_ParentheticalOpen){
            result->is_closed = true;
            result->close = Ii64(token->pos);
            break;
        }
        
        if (token->kind == TokenBaseKind_StatementClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        generic_parse_inc(state);
    }
    
    state->in_statement = false;
    
    return(result);
}

function Code_Index_Nest*
generic_parse_preprocessor(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Preprocessor;
    result->open = Ii64(token->pos);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->in_preprocessor = true;
    
    b32 potential_macro  = false;
    if (state->do_cpp_parse){
        if (token->sub_kind == TokenCppKind_PPDefine){
            potential_macro = true;
        }
    }
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
            token->kind == TokenBaseKind_Preprocessor){
            result->is_closed = true;
            result->close = Ii64(token->pos);
            break;
        }
        
        if (state->do_cpp_parse && potential_macro){
            if (token->sub_kind == TokenCppKind_Identifier){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Macro, result);
            }
            potential_macro = false;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        generic_parse_inc(state);
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->in_preprocessor = false;
    
    return(result);
}

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->scope_counter += 1;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
                code_index_push_nest(&index->nest_list, nest);
                continue;
            }
        }
        
        if (token->kind == TokenBaseKind_ScopeClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalClose){
            generic_parse_inc(state);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            
            // NOTE(allen): after a parenthetical group we consider ourselves immediately
            // transitioning into a statement
            nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            
            continue;
        }
        
        {
            Code_Index_Nest *nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->scope_counter -= 1;
    
    return(result);
}

function Code_Index_Nest*
generic_parse_paren(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Paren;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
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
    
    state->paren_counter += 1;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
                code_index_push_nest(&index->nest_list, nest);
                continue;
            }
        }
        
        if (token->kind == TokenBaseKind_ParentheticalClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeClose){
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        generic_parse_inc(state);
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->paren_counter -= 1;
    
    return(result);
}

function b32
generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit){
    b32 result = false;
    
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        Token *token = token_it_read(&state->it);
        
        if (token == 0 || state->finished){
            result = true;
            break;
        }
        
        if (token->kind == TokenBaseKind_Preprocessor){
            Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (state->do_cpp_parse){
            if (token->sub_kind == TokenCppKind_Struct ||
                token->sub_kind == TokenCppKind_Union ||
                token->sub_kind == TokenCppKind_Enum){
                cpp_parse_type_structure(index, state, 0);
            }
            else if (token->sub_kind == TokenCppKind_Typedef){
                cpp_parse_type_def(index, state, 0);
            }
            else if (token->sub_kind == TokenCppKind_Identifier){
                cpp_parse_function(index, state, 0);
            }
            else{
                generic_parse_inc(state);
            }
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
        index->nest_array = code_index_nest_ptr_array_from_list(state->arena, &index->nest_list);
        index->note_array = code_index_note_ptr_array_from_list(state->arena, &index->note_list);
    }
    
    return(result);
}

////////////////////////////////

function void
default_comment_index(Application_Links *app, Arena *arena, Code_Index_File *index, Token *token, String_Const_u8 contents){
    
}

function void
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens, Generic_Parse_State *state){
    generic_parse_init(app, arena, contents, tokens, default_comment_index, state);
}

////////////////////////////////

function Token_Pair
layout_token_pair(Token_Array *tokens, i64 pos){
    Token_Pair result = {};
    Token_Iterator_Array it = token_iterator_pos(0, tokens, pos);
    Token *b = token_it_read(&it);
    if (b != 0){
        if (b->kind == TokenBaseKind_Whitespace){
            token_it_inc_non_whitespace(&it);
            b = token_it_read(&it);
        }
    }
    token_it_dec_non_whitespace(&it);
    Token *a = token_it_read(&it);
    if (a != 0){
        result.a = *a;
    }
    if (b != 0){
        result.b = *b;
    }
    return(result);
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_Nest *nest, i64 pos, f32 space_advance, b32 *unresolved_dependence){
    f32 result = 0.f;
    if (nest != 0){
        switch (nest->kind){
            case CodeIndexNest_Scope:
            case CodeIndexNest_Preprocessor:
            {
                result = layout_index_x_shift(app, reflex, nest->parent, pos, space_advance, unresolved_dependence);
                if (nest->open.min < pos && nest->open.max <= pos &&
                    (!nest->is_closed || pos < nest->close.min)){
                    result += 4.f*space_advance;
                }
            }break;
            
            case CodeIndexNest_Statement:
            {
                result = layout_index_x_shift(app, reflex, nest->parent, pos, space_advance, unresolved_dependence);
                if (nest->open.min < pos && nest->open.max <= pos &&
                    (!nest->is_closed || pos < nest->close.min)){
                    result += 4.f*space_advance;
                }
            }break;
            
            case CodeIndexNest_Paren:
            {
                Rect_f32 box = layout_reflex_get_rect(app, reflex, nest->open.max - 1, unresolved_dependence);
                result = box.x1;
            }break;
        }
    }
    return(result);
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_Nest *nest, i64 pos, f32 space_advance){
    b32 ignore;
    return(layout_index_x_shift(app, reflex, nest, pos, space_advance, &ignore));
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_File *file, i64 pos, f32 space_advance, b32 *unresolved_dependence){
    f32 indent = 0;
    Code_Index_Nest *nest = code_index_get_nest(file, pos);
    if (nest != 0){
        indent = layout_index_x_shift(app, reflex, nest, pos, space_advance, unresolved_dependence);
    }
    return(indent);
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_File *file, i64 pos, f32 space_advance){
    b32 ignore;
    return(layout_index_x_shift(app, reflex, file, pos, space_advance, &ignore));
}

function void
layout_index__emit_chunk(LefRig_TopBot_Layout_Vars *pos_vars, Face_ID face, Arena *arena, u8 *text_str, i64 range_first, u8 *ptr, u8 *end, Layout_Item_List *list){
    for (;ptr < end;){
        Character_Consume_Result consume = utf8_consume(ptr, (u64)(end - ptr));
        if (consume.codepoint != '\r'){
            i64 index = layout_index_from_ptr(ptr, text_str, range_first);
            if (consume.codepoint != max_u32){
                lr_tb_write(pos_vars, face, arena, list, index, consume.codepoint);
            }
            else{
                lr_tb_write_byte(pos_vars, face, arena, list, index, *ptr);
            }
		}
        ptr += consume.inc;
    }
}

function i32
layout_token_score_wrap_token(Token_Pair *pair, Token_Cpp_Kind kind){
    i32 result = 0;
    if (pair->a.sub_kind != kind && pair->b.sub_kind == kind){
        result -= 1;
    }
    else if (pair->a.sub_kind == kind && pair->b.sub_kind != kind){
        result += 1;
    }
    return(result);
}

function Layout_Item_List
layout_index__inner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Code_Index_File *file, Layout_Wrap_Kind kind){
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens_ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
    
    Layout_Item_List list = get_empty_item_list(range);
    String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
    
    Face_Advance_Map advance_map = get_face_advance_map(app, face);
    Face_Metrics metrics = get_face_metrics(app, face);
    LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, width);
    
    f32 wrap_align_x = width - metrics.normal_advance;
    
    Layout_Reflex reflex = get_layout_reflex(&list, buffer, width, face);
    
    if (text.size == 0){
        lr_tb_write_blank(&pos_vars, face, arena, &list, range.start);
    }
    else{
        b32 first_of_the_line = true;
        Newline_Layout_Vars newline_vars = get_newline_layout_vars();
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        u8 *word_ptr = ptr;
        
        u8 *pending_wrap_ptr = ptr;
        f32 pending_wrap_x = 0.f;
        i32 pending_wrap_paren_nest_count = 0;
        i32 pending_wrap_token_score = 0;
        f32 pending_wrap_accumulated_w = 0.f;
        
        start:
        if (ptr == end_ptr){
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
            lr_tb_advance_x_without_item(&pos_vars, shift);
            goto finish;
        }
        
        if (!character_is_whitespace(*ptr)){
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
            lr_tb_advance_x_without_item(&pos_vars, shift);
            goto consuming_non_whitespace;
        }
        
        {
            for (;ptr < end_ptr; ptr += 1){
                if (!character_is_whitespace(*ptr)){
                    pending_wrap_ptr = ptr;
                    word_ptr = ptr;
                    i64 index = layout_index_from_ptr(ptr, text.str, range.first);
                    f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                    lr_tb_advance_x_without_item(&pos_vars, shift);
                    goto consuming_non_whitespace;
                }
                if (*ptr == '\n'){
                    pending_wrap_ptr = ptr;
                    i64 index = layout_index_from_ptr(ptr, text.str, range.first);
                    f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                    lr_tb_advance_x_without_item(&pos_vars, shift);
                    goto consuming_normal_whitespace;
                }
            }
            
            if (ptr == end_ptr){
                pending_wrap_ptr = ptr;
                i64 index = layout_index_from_ptr(ptr - 1, text.str, range.first);
                f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                lr_tb_advance_x_without_item(&pos_vars, shift);
                goto finish;
            }
        }
        
        consuming_non_whitespace:
        {
            for (;ptr <= end_ptr; ptr += 1){
                if (ptr == end_ptr || character_is_whitespace(*ptr)){
                    break;
                }
            }
            
            // NOTE(allen): measure this word
            newline_layout_consume_default(&newline_vars);
            String_Const_u8 word = SCu8(word_ptr, ptr);
            u8 *word_end = ptr;
            {
                f32 word_advance = 0.f;
                ptr = word.str;
                for (;ptr < word_end;){
                    Character_Consume_Result consume = utf8_consume(ptr, (u64)(word_end - ptr));
                    if (consume.codepoint != max_u32){
                        word_advance += lr_tb_advance(&pos_vars, face, consume.codepoint);
                    }
                    else{
                        word_advance += lr_tb_advance_byte(&pos_vars);
                    }
                    ptr += consume.inc;
                }
                pending_wrap_accumulated_w += word_advance;
            }
            
            if (!first_of_the_line && (kind == Layout_Wrapped) && lr_tb_crosses_width(&pos_vars, pending_wrap_accumulated_w)){
                i64 index = layout_index_from_ptr(pending_wrap_ptr, text.str, range.first);
                lr_tb_align_rightward(&pos_vars, wrap_align_x);
                lr_tb_write_ghost(&pos_vars, face, arena, &list, index, '\\');
                
                lr_tb_next_line(&pos_vars);
#if 0
                f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                lr_tb_advance_x_without_item(&pos_vars, shift);
#endif
                
                ptr = pending_wrap_ptr;
                pending_wrap_accumulated_w = 0.f;
                first_of_the_line = true;
                goto start;
            }
        }
        
        consuming_normal_whitespace:
        for (; ptr < end_ptr; ptr += 1){
            if (!character_is_whitespace(*ptr)){
                u8 *new_wrap_ptr = ptr;
                
                i64 index = layout_index_from_ptr(new_wrap_ptr, text.str, range.first);
                Code_Index_Nest *new_wrap_nest = code_index_get_nest(file, index);
                b32 invalid_wrap_x = false;
                f32 new_wrap_x = layout_index_x_shift(app, &reflex, new_wrap_nest, index, metrics.space_advance, &invalid_wrap_x);
                if (invalid_wrap_x){
                    new_wrap_x = max_f32;
                }
                
                i32 new_wrap_paren_nest_count = 0;
                for (Code_Index_Nest *nest = new_wrap_nest;
                     nest != 0;
                     nest = nest->parent){
                    if (nest->kind == CodeIndexNest_Paren){
                        new_wrap_paren_nest_count += 1;
                    }
                }
                
                Token_Pair new_wrap_token_pair = layout_token_pair(tokens_ptr, index);
                
                // TODO(allen): pull out the token scoring part and make it replacable for other
                // language's token based wrap scoring needs.
                i32 token_score = 0;
                if (new_wrap_token_pair.a.kind == TokenBaseKind_Keyword){
                    if (new_wrap_token_pair.b.kind == TokenBaseKind_ParentheticalOpen ||
                        new_wrap_token_pair.b.kind == TokenBaseKind_Keyword){
                        token_score -= 2;
                    }
                }
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Eq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_PlusEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_MinusEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_StarEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_DivEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_ModEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_LeftLeftEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_RightRightEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Comma);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_AndAnd);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_OrOr);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Ternary);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Colon);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Semicolon);
                
                i32 new_wrap_token_score = token_score;
                
                b32 new_wrap_ptr_is_better = false;
                if (first_of_the_line){
                    new_wrap_ptr_is_better = true;
                }
                else{
                    if (new_wrap_token_score > pending_wrap_token_score){
                        new_wrap_ptr_is_better = true;
                    }
                    else if (new_wrap_token_score == pending_wrap_token_score){
                        f32 new_score = new_wrap_paren_nest_count*10.f + new_wrap_x;
                        f32 old_score = pending_wrap_paren_nest_count*10.f + pending_wrap_x + metrics.normal_advance*4.f + pending_wrap_accumulated_w*0.5f;
                        
                        if (new_score < old_score){
                            new_wrap_ptr_is_better = true;
                        }
                    }
                }
                
                if (new_wrap_ptr_is_better){
                    layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, new_wrap_ptr, &list);
                    first_of_the_line = false;
                    
                    pending_wrap_ptr = new_wrap_ptr;
                    pending_wrap_paren_nest_count = new_wrap_paren_nest_count;
                    pending_wrap_x = layout_index_x_shift(app, &reflex, new_wrap_nest, index, metrics.space_advance);
                    pending_wrap_paren_nest_count = new_wrap_paren_nest_count;
                    pending_wrap_token_score = new_wrap_token_score;
                    pending_wrap_accumulated_w = 0.f;
                }
                
                word_ptr = ptr;
                goto consuming_non_whitespace;
            }
            
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            switch (*ptr){
                default:
                {
                    newline_layout_consume_default(&newline_vars);
                    pending_wrap_accumulated_w += lr_tb_advance(&pos_vars, face, *ptr);
                }break;
                
                case '\r':
                {
                    newline_layout_consume_CR(&newline_vars, index);
                }break;
                
                case '\n':
                {
                    layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, ptr, &list);
                    pending_wrap_ptr = ptr + 1;
                    pending_wrap_accumulated_w = 0.f;
                    
                    u64 newline_index = newline_layout_consume_LF(&newline_vars, index);
                    lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
                    lr_tb_next_line(&pos_vars);
                    first_of_the_line = true;
                    ptr += 1;
                    goto start;
                }break;
            }
        }
        
        finish:
        if (newline_layout_consume_finish(&newline_vars)){
            layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, ptr, &list);
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            lr_tb_write_blank(&pos_vars, face, arena, &list, index);
        }
    }
    
    layout_item_list_finish(&list, -pos_vars.line_to_text_shift);
    
    return(list);
}

function Layout_Item_List
layout_virt_indent_index(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Wrap_Kind kind){
    Layout_Item_List result = {};
    
    if (global_config.enable_virtual_whitespace){
        code_index_lock();
        Code_Index_File *file = code_index_get_file(buffer);
        if (file != 0){
            result = layout_index__inner(app, arena, buffer, range, face, width, file, kind);
        }
        code_index_unlock();
        if (file == 0){
            result = layout_virt_indent_literal(app, arena, buffer, range, face, width, kind);
        }
    }
    else{
        result = layout_basic(app, arena, buffer, range, face, width, kind);
    }
    
    return(result);
}

function Layout_Item_List
layout_virt_indent_index_unwrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
    return(layout_virt_indent_index(app, arena, buffer, range, face, width, Layout_Unwrapped));
}

function Layout_Item_List
layout_virt_indent_index_wrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
    return(layout_virt_indent_index(app, arena, buffer, range, face, width, Layout_Wrapped));
}

function Layout_Item_List
layout_virt_indent_index_generic(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
    b32 wrap_lines = (wrap_lines_ptr != 0 && *wrap_lines_ptr);
    return(layout_virt_indent_index(app, arena, buffer, range, face, width, wrap_lines?Layout_Wrapped:Layout_Unwrapped));
}

CUSTOM_COMMAND_SIG(toggle_virtual_whitespace)
CUSTOM_DOC("Toggles the current buffer's virtual whitespace status.")
{
    global_config.enable_virtual_whitespace = !global_config.enable_virtual_whitespace;
    
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always)){
        buffer_clear_layout_cache(app, buffer);
    }
}

// BOTTOM

#if 0
=======
/*
4coder_code_index.cpp - Generic code indexing system for layout, definition jumps, etc.
*/

// TOP

global Code_Index global_code_index = {};

function void
code_index_init(void){
    global_code_index.mutex = system_mutex_make();
    global_code_index.node_arena = make_arena_system(KB(4));
    global_code_index.buffer_to_index_file = make_table_u64_u64(global_code_index.node_arena.base_allocator, 500);
}

function Code_Index_File_Storage*
code_index__alloc_storage(void){
    Code_Index_File_Storage *result = global_code_index.free_storage;
    if (result == 0){
        result = push_array_zero(&global_code_index.node_arena, Code_Index_File_Storage, 1);
    }
    else{
        sll_stack_pop(global_code_index.free_storage);
    }
    zdll_push_back(global_code_index.storage_first, global_code_index.storage_last, result);
    global_code_index.storage_count += 1;
    return(result);
}

function void
code_index__free_storage(Code_Index_File_Storage *storage){
    zdll_remove(global_code_index.storage_first, global_code_index.storage_last, storage);
    global_code_index.storage_count -= 1;
    sll_stack_push(global_code_index.free_storage, storage);
}

function void
code_index_push_nest(Code_Index_Nest_List *list, Code_Index_Nest *nest){
    sll_queue_push(list->first, list->last, nest);
    list->count += 1;
}

function Code_Index_Nest_Ptr_Array
code_index_nest_ptr_array_from_list(Arena *arena, Code_Index_Nest_List *list){
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

function Code_Index_Note_Ptr_Array
code_index_note_ptr_array_from_list(Arena *arena, Code_Index_Note_List *list){
    Code_Index_Note_Ptr_Array array = {};
    array.ptrs = push_array_zero(arena, Code_Index_Note*, list->count);
    array.count = list->count;
    i32 counter = 0;
    for (Code_Index_Note *node = list->first;
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
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file, buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        storage = (Code_Index_File_Storage*)IntAsPtr(val);
        linalloc_clear(&storage->arena);
    }
    else{
        storage = code_index__alloc_storage();
        table_insert(&global_code_index.buffer_to_index_file, buffer, (u64)PtrAsInt(storage));
    }
    storage->arena = arena;
    storage->file = index;
}

function void
code_index_erase_file(Buffer_ID buffer){
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file, buffer);
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
    Table_Lookup lookup = table_lookup(&global_code_index.buffer_to_index_file, buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&global_code_index.buffer_to_index_file, lookup, &val);
        Code_Index_File_Storage *storage = (Code_Index_File_Storage*)IntAsPtr(val);
        result = storage->file;
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
        if (nest->open.max <= pos && pos <= nest->close.min){
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

function void
index_shift(i64 *ptr, Range_i64 old_range, u64 new_size){
    i64 i = *ptr;
    if (old_range.min <= i && i < old_range.max){
        *ptr = old_range.first;
    }
    else if (old_range.max <= i){
        *ptr = i + new_size - (old_range.max - old_range.min);
    }
}

function void
code_index_shift(Code_Index_Nest_Ptr_Array *array,
                 Range_i64 old_range, u64 new_size){
    i32 count = array->count;
    Code_Index_Nest **nest_ptr = array->ptrs;
    for (i32 i = 0; i < count; i += 1, nest_ptr += 1){
        Code_Index_Nest *nest = *nest_ptr;
        index_shift(&nest->open.min, old_range, new_size);
        index_shift(&nest->open.max, old_range, new_size);
        if (nest->is_closed){
            index_shift(&nest->close.min, old_range, new_size);
            index_shift(&nest->close.max, old_range, new_size);
        }
        code_index_shift(&nest->nest_array, old_range, new_size);
    }
}

function void
code_index_shift(Code_Index_File *file, Range_i64 old_range, u64 new_size){
    code_index_shift(&file->nest_array, old_range, new_size);
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
        if (state->in_preprocessor && !HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
            break;
        }
        if (token->kind == TokenBaseKind_Comment){
            state->handle_comment(state->app, state->arena, index, token, state->contents);
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
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens, Generic_Parse_Comment_Function *handle_comment, Generic_Parse_State *state){
    state->app = app;
    state->arena = arena;
    state->contents = contents;
    state->it = token_iterator(0, tokens);
    state->handle_comment = handle_comment;
    state->prev_line_start = contents.str;
}

////////////////////////////////

#if 0
// NOTE(allen): grammar syntax
(X) = X
X Y = X and then Y
X? = zero or one X
$X = check for X but dont consume // NOTE(yuval): Removed apostrophe as it was causing a warning when compiling with gcc
[X] = zero or more Xs
X | Y = either X or Y
* = anything that does not match previous options in a X | Y | ... chain
* - X = anything that does not match X or previous options in a Y | Z | ... chain
<X> = a token of type X
"X" = literally the string "X"
X{Y} = X with flag Y

// NOTE(allen): grammar of code index parse
file: [preprocessor | scope | parens | function | type | * - <end-of-file>] <end-of-file>
preprocessor: <preprocessor> [scope | parens | stmnt]{pp-body}
scope: <scope-open> [preprocessor | scope | parens | * - <scope-close>] <scope-close>
paren: <paren-open> [preprocessor | scope | parens | * - <paren-close>] <paren-close>
stmnt-close-pattern: <scope-open> | <scope-close> | <paren-open> | <paren-close> | <stmnt-close> | <preprocessor>
stmnt: [type | * - stmnt-close-pattern] stmnt-close-pattern
type: struct | union | enum | typedef
struct: "struct" <identifier> $(";" | "{")
union: "union" <identifier> $(";" | "{")
enum: "enum" <identifier> $(";" | "{")
typedef: "typedef" [* - (<identifier> (";" | "("))] <identifier> $(";" | "(")
function: <identifier> >"(" [* - ("(" | ")" | "{" | "}" | ";")] ")" ("{" | ";")

#endif

////////////////////////////////

function Code_Index_Note*
index_new_note(Code_Index_File *index, Generic_Parse_State *state, Range_i64 range, Code_Index_Note_Kind kind, Code_Index_Nest *parent){
    Code_Index_Note *result = push_array(state->arena, Code_Index_Note, 1);
    sll_queue_push(index->note_list.first, index->note_list.last, result);
    index->note_list.count += 1;
    result->note_kind = kind;
    result->pos = range;
    result->text = push_string_copy(state->arena, string_substring(state->contents, range));
    result->file = index;
    result->parent = parent;
    return(result);
}

function void
cpp_parse_type_structure(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent){
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    if (state->finished){
        return;
    }
    Token *token = token_it_read(&state->it);
    if (token != 0 && token->kind == TokenBaseKind_Identifier){
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        Token *peek = token_it_read(&state->it);
        if (peek != 0 && peek->kind == TokenBaseKind_StatementClose ||
            peek->kind == TokenBaseKind_ScopeOpen){
            index_new_note(index, state, Ii64(token), CodeIndexNote_Type, parent);
        }
    }
}

function void
cpp_parse_type_def(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent){
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    for (;;){
        b32 did_advance = false;
        Token *token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        if (token->kind == TokenBaseKind_Identifier){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            did_advance = true;
            Token *peek = token_it_read(&state->it);
            if (peek != 0 && peek->kind == TokenBaseKind_StatementClose ||
                peek->kind == TokenBaseKind_ParentheticalOpen){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Type, parent);
                break;
            }
        }
        else if (token->kind == TokenBaseKind_StatementClose ||
                 token->kind == TokenBaseKind_ScopeOpen ||
                 token->kind == TokenBaseKind_ScopeClose ||
                 token->kind == TokenBaseKind_ScopeOpen ||
                 token->kind == TokenBaseKind_ScopeClose){
            break;
        }
        else if (token->kind == TokenBaseKind_Keyword){
            String_Const_u8 lexeme = string_substring(state->contents, Ii64(token));
            if (string_match(lexeme, string_u8_litexpr("struct")) ||
                string_match(lexeme, string_u8_litexpr("union")) ||
                string_match(lexeme, string_u8_litexpr("enum"))){
                break;
            }
        }
        if (!did_advance){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
        }
    }
}

function void
cpp_parse_function(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent){
    Token *token = token_it_read(&state->it);
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    if (state->finished){
        return;
    }
    Token *peek = token_it_read(&state->it);
    Token *reset_point = peek;
    if (peek != 0 && peek->sub_kind == TokenCppKind_ParenOp){
        b32 at_paren_close = false;
        for (; peek != 0;){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            peek = token_it_read(&state->it);
            if (peek == 0 || state->finished){
                break;
            }
            
            if (peek->kind == TokenBaseKind_ParentheticalOpen ||
                peek->kind == TokenBaseKind_ScopeOpen ||
                peek->kind == TokenBaseKind_ScopeClose ||
                peek->kind == TokenBaseKind_StatementClose){
                break;
            }
            if (peek->kind == TokenBaseKind_ParentheticalClose){
                at_paren_close = true;
                break;
            }
        }
        
        if (at_paren_close){
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            peek = token_it_read(&state->it);
            if (peek != 0 &&
                peek->kind == TokenBaseKind_ScopeOpen ||
                peek->kind == TokenBaseKind_StatementClose){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Function, parent);
            }
        }
    }
    state->it = token_iterator(state->it.user_id, state->it.tokens, state->it.count, reset_point);
}

function Code_Index_Nest*
generic_parse_statement(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_preprocessor(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_paren(Code_Index_File *index, Generic_Parse_State *state);

function Code_Index_Nest*
generic_parse_statement(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Statement;
    result->open = Ii64(token->pos);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->in_statement = true;
    
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                result->is_closed = true;
                result->close = Ii64(token->pos);
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                result->is_closed = true;
                result->close = Ii64(token->pos);
                break;
            }
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen ||
            token->kind == TokenBaseKind_ScopeClose ||
            token->kind == TokenBaseKind_ParentheticalOpen){
            result->is_closed = true;
            result->close = Ii64(token->pos);
            break;
        }
        
        if (token->kind == TokenBaseKind_StatementClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        generic_parse_inc(state);
    }
    
    state->in_statement = false;
    
    return(result);
}

function Code_Index_Nest*
generic_parse_preprocessor(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Preprocessor;
    result->open = Ii64(token->pos);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->in_preprocessor = true;
    
    b32 potential_macro  = false;
    if (state->do_cpp_parse){
        if (token->sub_kind == TokenCppKind_PPDefine){
            potential_macro = true;
        }
    }
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
            token->kind == TokenBaseKind_Preprocessor){
            result->is_closed = true;
            result->close = Ii64(token->pos);
            break;
        }
        
        if (state->do_cpp_parse && potential_macro){
            if (token->sub_kind == TokenCppKind_Identifier){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Macro, result);
            }
            potential_macro = false;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        generic_parse_inc(state);
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->in_preprocessor = false;
    
    return(result);
}

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->scope_counter += 1;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
                code_index_push_nest(&index->nest_list, nest);
                continue;
            }
        }
        
        if (token->kind == TokenBaseKind_ScopeClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalClose){
            generic_parse_inc(state);
            continue;
        }
        
        {
            Code_Index_Nest *nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->scope_counter -= 1;
    
    return(result);
}

function Code_Index_Nest*
generic_parse_paren(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Paren;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
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
    
    state->paren_counter += 1;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
                code_index_push_nest(&index->nest_list, nest);
                continue;
            }
        }
        
        if (token->kind == TokenBaseKind_ParentheticalClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeClose){
            break;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        generic_parse_inc(state);
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->paren_counter -= 1;
    
    return(result);
}

function b32
generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit){
    b32 result = false;
    
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        Token *token = token_it_read(&state->it);
        
        if (token == 0 || state->finished){
            result = true;
            break;
        }
        
        if (token->kind == TokenBaseKind_Preprocessor){
            Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (state->do_cpp_parse){
            if (token->sub_kind == TokenCppKind_Struct ||
                token->sub_kind == TokenCppKind_Union ||
                token->sub_kind == TokenCppKind_Enum){
                cpp_parse_type_structure(index, state, 0);
            }
            else if (token->sub_kind == TokenCppKind_Typedef){
                cpp_parse_type_def(index, state, 0);
            }
            else if (token->sub_kind == TokenCppKind_Identifier){
                cpp_parse_function(index, state, 0);
            }
            else{
                generic_parse_inc(state);
            }
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
        index->nest_array = code_index_nest_ptr_array_from_list(state->arena, &index->nest_list);
        index->note_array = code_index_note_ptr_array_from_list(state->arena, &index->note_list);
    }
    
    return(result);
}

////////////////////////////////

function void
default_comment_index(Application_Links *app, Arena *arena, Code_Index_File *index, Token *token, String_Const_u8 contents){
    
}

function void
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents, Token_Array *tokens, Generic_Parse_State *state){
    generic_parse_init(app, arena, contents, tokens, default_comment_index, state);
}

////////////////////////////////

function Token_Pair
layout_token_pair(Token_Array *tokens, i64 pos){
    Token_Pair result = {};
    Token_Iterator_Array it = token_iterator_pos(0, tokens, pos);
    Token *b = token_it_read(&it);
    if (b != 0){
        if (b->kind == TokenBaseKind_Whitespace){
            token_it_inc_non_whitespace(&it);
            b = token_it_read(&it);
        }
    }
    token_it_dec_non_whitespace(&it);
    Token *a = token_it_read(&it);
    if (a != 0){
        result.a = *a;
    }
    if (b != 0){
        result.b = *b;
    }
    return(result);
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_Nest *nest, i64 pos, f32 space_advance, b32 *unresolved_dependence){
    f32 result = 0.f;
    if (nest != 0){
        switch (nest->kind){
            case CodeIndexNest_Scope:
            case CodeIndexNest_Preprocessor:
            {
                result = layout_index_x_shift(app, reflex, nest->parent, pos, space_advance, unresolved_dependence);
                if (nest->open.min < pos && nest->open.max <= pos &&
                    (!nest->is_closed || pos < nest->close.min)){
                    result += 4.f*space_advance;
                }
            }break;
            
            case CodeIndexNest_Statement:
            {
                result = layout_index_x_shift(app, reflex, nest->parent, pos, space_advance, unresolved_dependence);
                if (nest->open.min < pos && nest->open.max <= pos &&
                    (!nest->is_closed || pos < nest->close.min)){
                    result += 4.f*space_advance;
                }
            }break;
            
            case CodeIndexNest_Paren:
            {
                Rect_f32 box = layout_reflex_get_rect(app, reflex, nest->open.max - 1, unresolved_dependence);
                result = box.x1;
            }break;
        }
    }
    return(result);
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_Nest *nest, i64 pos, f32 space_advance){
    b32 ignore;
    return(layout_index_x_shift(app, reflex, nest, pos, space_advance, &ignore));
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_File *file, i64 pos, f32 space_advance, b32 *unresolved_dependence){
    f32 indent = 0;
    Code_Index_Nest *nest = code_index_get_nest(file, pos);
    if (nest != 0){
        indent = layout_index_x_shift(app, reflex, nest, pos, space_advance, unresolved_dependence);
    }
    return(indent);
}

function f32
layout_index_x_shift(Application_Links *app, Layout_Reflex *reflex, Code_Index_File *file, i64 pos, f32 space_advance){
    b32 ignore;
    return(layout_index_x_shift(app, reflex, file, pos, space_advance, &ignore));
}

function void
layout_index__emit_chunk(LefRig_TopBot_Layout_Vars *pos_vars, Face_ID face, Arena *arena, u8 *text_str, i64 range_first, u8 *ptr, u8 *end, Layout_Item_List *list){
    for (;ptr < end;){
        Character_Consume_Result consume = utf8_consume(ptr, (u64)(end - ptr));
        if (consume.codepoint != '\r'){
            i64 index = layout_index_from_ptr(ptr, text_str, range_first);
            if (consume.codepoint != max_u32){
                lr_tb_write(pos_vars, face, arena, list, index, consume.codepoint);
            }
            else{
                lr_tb_write_byte(pos_vars, face, arena, list, index, *ptr);
            }
		}
        ptr += consume.inc;
    }
}

function i32
layout_token_score_wrap_token(Token_Pair *pair, Token_Cpp_Kind kind){
    i32 result = 0;
    if (pair->a.sub_kind != kind && pair->b.sub_kind == kind){
        result -= 1;
    }
    else if (pair->a.sub_kind == kind && pair->b.sub_kind != kind){
        result += 1;
    }
    return(result);
}

function Layout_Item_List
layout_index__inner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Code_Index_File *file, Layout_Wrap_Kind kind){
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens_ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
    
    Layout_Item_List list = get_empty_item_list(range);
    String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
    
    Face_Advance_Map advance_map = get_face_advance_map(app, face);
    Face_Metrics metrics = get_face_metrics(app, face);
    LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, width);
    
    f32 wrap_align_x = width - metrics.normal_advance;
    
    Layout_Reflex reflex = get_layout_reflex(&list, buffer, width, face);
    
    if (text.size == 0){
        lr_tb_write_blank(&pos_vars, face, arena, &list, range.start);
    }
    else{
        b32 first_of_the_line = true;
        Newline_Layout_Vars newline_vars = get_newline_layout_vars();
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        u8 *word_ptr = ptr;
        
        u8 *pending_wrap_ptr = ptr;
        f32 pending_wrap_x = 0.f;
        i32 pending_wrap_paren_nest_count = 0;
        i32 pending_wrap_token_score = 0;
        f32 pending_wrap_accumulated_w = 0.f;
        
        start:
        if (ptr == end_ptr){
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
            lr_tb_advance_x_without_item(&pos_vars, shift);
            goto finish;
        }
        
        if (!character_is_whitespace(*ptr)){
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
            lr_tb_advance_x_without_item(&pos_vars, shift);
            goto consuming_non_whitespace;
        }
        
        {
            for (;ptr < end_ptr; ptr += 1){
                if (!character_is_whitespace(*ptr)){
                    pending_wrap_ptr = ptr;
                    word_ptr = ptr;
                    i64 index = layout_index_from_ptr(ptr, text.str, range.first);
                    f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                    lr_tb_advance_x_without_item(&pos_vars, shift);
                    goto consuming_non_whitespace;
                }
                if (*ptr == '\n'){
                    pending_wrap_ptr = ptr;
                    i64 index = layout_index_from_ptr(ptr, text.str, range.first);
                    f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                    lr_tb_advance_x_without_item(&pos_vars, shift);
                    goto consuming_normal_whitespace;
                }
            }
            
            if (ptr == end_ptr){
                pending_wrap_ptr = ptr;
                i64 index = layout_index_from_ptr(ptr - 1, text.str, range.first);
                f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                lr_tb_advance_x_without_item(&pos_vars, shift);
                goto finish;
            }
        }
        
        consuming_non_whitespace:
        {
            for (;ptr <= end_ptr; ptr += 1){
                if (ptr == end_ptr || character_is_whitespace(*ptr)){
                    break;
                }
            }
            
            // NOTE(allen): measure this word
            newline_layout_consume_default(&newline_vars);
            String_Const_u8 word = SCu8(word_ptr, ptr);
            u8 *word_end = ptr;
            {
                f32 word_advance = 0.f;
                ptr = word.str;
                for (;ptr < word_end;){
                    Character_Consume_Result consume = utf8_consume(ptr, (u64)(word_end - ptr));
                    if (consume.codepoint != max_u32){
                        word_advance += lr_tb_advance(&pos_vars, face, consume.codepoint);
                    }
                    else{
                        word_advance += lr_tb_advance_byte(&pos_vars);
                    }
                    ptr += consume.inc;
                }
                pending_wrap_accumulated_w += word_advance;
            }
            
            if (!first_of_the_line && (kind == Layout_Wrapped) && lr_tb_crosses_width(&pos_vars, pending_wrap_accumulated_w)){
                i64 index = layout_index_from_ptr(pending_wrap_ptr, text.str, range.first);
                lr_tb_align_rightward(&pos_vars, wrap_align_x);
                lr_tb_write_ghost(&pos_vars, face, arena, &list, index, '\\');
                
                lr_tb_next_line(&pos_vars);
#if 0
                f32 shift = layout_index_x_shift(app, &reflex, file, index, metrics.space_advance);
                lr_tb_advance_x_without_item(&pos_vars, shift);
#endif
                
                ptr = pending_wrap_ptr;
                pending_wrap_accumulated_w = 0.f;
                first_of_the_line = true;
                goto start;
            }
        }
        
        consuming_normal_whitespace:
        for (; ptr < end_ptr; ptr += 1){
            if (!character_is_whitespace(*ptr)){
                u8 *new_wrap_ptr = ptr;
                
                i64 index = layout_index_from_ptr(new_wrap_ptr, text.str, range.first);
                Code_Index_Nest *new_wrap_nest = code_index_get_nest(file, index);
                b32 invalid_wrap_x = false;
                f32 new_wrap_x = layout_index_x_shift(app, &reflex, new_wrap_nest, index, metrics.space_advance, &invalid_wrap_x);
                if (invalid_wrap_x){
                    new_wrap_x = max_f32;
                }
                
                i32 new_wrap_paren_nest_count = 0;
                for (Code_Index_Nest *nest = new_wrap_nest;
                     nest != 0;
                     nest = nest->parent){
                    if (nest->kind == CodeIndexNest_Paren){
                        new_wrap_paren_nest_count += 1;
                    }
                }
                
                Token_Pair new_wrap_token_pair = layout_token_pair(tokens_ptr, index);
                
                // TODO(allen): pull out the token scoring part and make it replacable for other
                // language's token based wrap scoring needs.
                i32 token_score = 0;
                if (new_wrap_token_pair.a.kind == TokenBaseKind_Keyword){
                    if (new_wrap_token_pair.b.kind == TokenBaseKind_ParentheticalOpen ||
                        new_wrap_token_pair.b.kind == TokenBaseKind_Keyword){
                        token_score -= 2;
                    }
                }
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Eq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_PlusEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_MinusEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_StarEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_DivEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_ModEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_LeftLeftEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_RightRightEq);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Comma);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_AndAnd);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_OrOr);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Ternary);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Colon);
                token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Semicolon);
                
                i32 new_wrap_token_score = token_score;
                
                b32 new_wrap_ptr_is_better = false;
                if (first_of_the_line){
                    new_wrap_ptr_is_better = true;
                }
                else{
                    if (new_wrap_token_score > pending_wrap_token_score){
                        new_wrap_ptr_is_better = true;
                    }
                    else if (new_wrap_token_score == pending_wrap_token_score){
                        f32 new_score = new_wrap_paren_nest_count*10.f + new_wrap_x;
                        f32 old_score = pending_wrap_paren_nest_count*10.f + pending_wrap_x + metrics.normal_advance*4.f + pending_wrap_accumulated_w*0.5f;
                        
                        if (new_score < old_score){
                            new_wrap_ptr_is_better = true;
                        }
                    }
                }
                
                if (new_wrap_ptr_is_better){
                    layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, new_wrap_ptr, &list);
                    first_of_the_line = false;
                    
                    pending_wrap_ptr = new_wrap_ptr;
                    pending_wrap_paren_nest_count = new_wrap_paren_nest_count;
                    pending_wrap_x = layout_index_x_shift(app, &reflex, new_wrap_nest, index, metrics.space_advance);
                    pending_wrap_paren_nest_count = new_wrap_paren_nest_count;
                    pending_wrap_token_score = new_wrap_token_score;
                    pending_wrap_accumulated_w = 0.f;
                }
                
                word_ptr = ptr;
                goto consuming_non_whitespace;
            }
            
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            switch (*ptr){
                default:
                {
                    newline_layout_consume_default(&newline_vars);
                    pending_wrap_accumulated_w += lr_tb_advance(&pos_vars, face, *ptr);
                }break;
                
                case '\r':
                {
                    newline_layout_consume_CR(&newline_vars, index);
                }break;
                
                case '\n':
                {
                    layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, ptr, &list);
                    pending_wrap_ptr = ptr + 1;
                    pending_wrap_accumulated_w = 0.f;
                    
                    u64 newline_index = newline_layout_consume_LF(&newline_vars, index);
                    lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
                    lr_tb_next_line(&pos_vars);
                    first_of_the_line = true;
                    ptr += 1;
                    goto start;
                }break;
            }
        }
        
        finish:
        if (newline_layout_consume_finish(&newline_vars)){
            layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, ptr, &list);
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            lr_tb_write_blank(&pos_vars, face, arena, &list, index);
        }
    }
    
    layout_item_list_finish(&list, -pos_vars.line_to_text_shift);
    
    return(list);
}

function Layout_Item_List
layout_virt_indent_index(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Wrap_Kind kind){
    Layout_Item_List result = {};
    
    if (global_config.enable_virtual_whitespace){
        code_index_lock();
        Code_Index_File *file = code_index_get_file(buffer);
        if (file != 0){
            result = layout_index__inner(app, arena, buffer, range, face, width, file, kind);
        }
        code_index_unlock();
        if (file == 0){
            result = layout_virt_indent_literal(app, arena, buffer, range, face, width, kind);
        }
    }
    else{
        result = layout_basic(app, arena, buffer, range, face, width, kind);
    }
    
    return(result);
}

function Layout_Item_List
layout_virt_indent_index_unwrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
    return(layout_virt_indent_index(app, arena, buffer, range, face, width, Layout_Unwrapped));
}

function Layout_Item_List
layout_virt_indent_index_wrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
    return(layout_virt_indent_index(app, arena, buffer, range, face, width, Layout_Wrapped));
}

function Layout_Item_List
layout_virt_indent_index_generic(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
    b32 wrap_lines = (wrap_lines_ptr != 0 && *wrap_lines_ptr);
    return(layout_virt_indent_index(app, arena, buffer, range, face, width, wrap_lines?Layout_Wrapped:Layout_Unwrapped));
}

CUSTOM_COMMAND_SIG(toggle_virtual_whitespace)
CUSTOM_DOC("Toggles the current buffer's virtual whitespace status.")
{
    global_config.enable_virtual_whitespace = !global_config.enable_virtual_whitespace;
    
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always)){
        buffer_clear_layout_cache(app, buffer);
    }
}

// BOTTOM

>>>>>>> yuval_macos_platform_layer
#endif
