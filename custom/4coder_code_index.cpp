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
index_shift(i64 *ptr, Range_i64 old_range, umem new_size){
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
                 Range_i64 old_range, umem new_size){
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
code_index_shift(Code_Index_File *file, Range_i64 old_range, umem new_size){
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
generic_parse_init(Application_Links *app, Arena *arena, String_Const_u8 contents,
                   Token_Array *tokens,
                   Generic_Parse_Comment_Function *handle_comment, Generic_Parse_State *state){
    state->app = app;
    state->arena = arena;
    state->contents = contents;
    state->it = token_iterator(0, tokens);
    state->handle_comment = handle_comment;
    state->prev_line_start = contents.str;
}

function Code_Index_Nest*
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state, i64 indentation);

function Code_Index_Nest*
generic_parse_paren(Code_Index_File *index, Generic_Parse_State *state, i64 indentation);

function Code_Index_Nest*
generic_parse_preprocessor(Code_Index_File *index, Generic_Parse_State *state, i64 indentation){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Preprocessor;
    result->open = Ii64(token->pos);
    result->close = Ii64(max_i64);
    result->file = index;
    
    result->interior_indentation = 0;
    result->close_indentation = 0;
    indentation = 0;
    
    state->in_preprocessor = true;
    
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
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state, indentation);
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
generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state,
                    i64 indentation){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
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
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state, indentation);
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
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        generic_parse_inc(state);
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    return(result);
}

function Code_Index_Nest*
generic_parse_paren(Code_Index_File *index, Generic_Parse_State *state,
                    i64 indentation){
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
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state, indentation);
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
        
        if (token->kind == TokenBaseKind_Preprocessor){
            Code_Index_Nest *nest = generic_parse_preprocessor(index, state, indentation);
            code_index_push_nest(&index->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state, indentation);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        generic_parse_inc(state);
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
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
        
        if (token->kind == TokenBaseKind_Preprocessor){
            Code_Index_Nest *nest = generic_parse_preprocessor(index, state, indentation);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state, indentation);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state, indentation);
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
        index->nest_array = code_index_nest_ptr_array_from_list(state->arena, &index->nest_list);
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

////////////////////////////////

function i64
layout_index_indent(Code_Index_File *file, i64 pos){
    i64 indent = 0;
    Code_Index_Nest *nest = code_index_get_nest(file, pos);
    if (nest != 0){
        if (pos == nest->close.min){
            indent = nest->close_indentation;
        }
        else{
            indent = nest->interior_indentation;
        }
    }
    return(indent);
}

function f32
layout_index_x_shift(Code_Index_File *file, i64 pos, f32 space_advance){
    i64 indent = layout_index_indent(file, pos);
    return(((f32)indent)*space_advance);
}

function Layout_Item_List
layout_index_unwrapped__inner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Code_Index_File *file){
    Scratch_Block scratch(app);
    
    Layout_Item_List list = get_empty_item_list(range);
    String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
    
    Face_Advance_Map advance_map = get_face_advance_map(app, face);
    Face_Metrics metrics = get_face_metrics(app, face);
    LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, width);
    
    f32 wrap_align_x = width - metrics.normal_advance;
    
    if (text.size == 0){
        lr_tb_write_blank(&pos_vars, arena, &list, range.start);
    }
    else{
        b32 first_of_the_line = true;
        Newline_Layout_Vars newline_vars = get_newline_layout_vars();
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        u8 *word_ptr = ptr;
        
        if (!character_is_whitespace(*ptr)){
            goto consuming_non_whitespace;
        }
        
        skipping_leading_whitespace:
        for (;ptr < end_ptr; ptr += 1){
            if (!character_is_whitespace(*ptr)){
                word_ptr = ptr;
                goto consuming_non_whitespace;
            }
            if (*ptr == '\n'){
                goto consuming_normal_whitespace;
            }
        }
        
        if (ptr == end_ptr){
            goto finish;
        }
        
        consuming_non_whitespace:
        for (;ptr <= end_ptr; ptr += 1){
            if (ptr == end_ptr || character_is_whitespace(*ptr)){
                break;
            }
        }
        
        {
            newline_layout_consume_default(&newline_vars);
            
            String_Const_u8 word = SCu8(word_ptr, ptr);
            u8 *word_end = ptr;
            
            if (!first_of_the_line){
                f32 total_advance = 0.f;
                ptr = word.str;
                for (;ptr < word_end;){
                    Character_Consume_Result consume =
                        utf8_consume(ptr, (umem)(word_end - ptr));
                    if (consume.codepoint != max_u32){
                        total_advance += lr_tb_advance(&pos_vars, consume.codepoint);
                    }
                    else{
                        total_advance += lr_tb_advance_byte(&pos_vars);
                    }
                    ptr += consume.inc;
                }
                
                if (lr_tb_crosses_width(&pos_vars, total_advance)){
                    i64 index = layout_index_from_ptr(word.str, text.str, range.first);
                    lr_tb_align_rightward(&pos_vars, wrap_align_x);
                    lr_tb_write_ghost(&pos_vars, arena, &list, index, '\\');
                    
                    lr_tb_next_line(&pos_vars);
                    f32 shift = layout_index_x_shift(file, index, metrics.space_advance);
                    lr_tb_advance_x_without_item(&pos_vars, shift);
                }
            }
            else{
                i64 index = layout_index_from_ptr(word.str, text.str, range.first);
                f32 shift = layout_index_x_shift(file, index, metrics.space_advance);
                lr_tb_advance_x_without_item(&pos_vars, shift);
            }
            
            ptr = word.str;
            
            for (;ptr < word_end;){
                Character_Consume_Result consume =
                    utf8_consume(ptr, (umem)(word_end - ptr));
                i64 index = layout_index_from_ptr(ptr, text.str, range.first);
                
                if (consume.codepoint != max_u32){
                    lr_tb_write(&pos_vars, arena, &list, index, consume.codepoint);
                }
                else{
                    lr_tb_write_byte(&pos_vars, arena, &list, index, *ptr);
                }
                
                ptr += consume.inc;
            }
            
            first_of_the_line = false;
        }
        
        consuming_normal_whitespace:
        for (; ptr < end_ptr; ptr += 1){
            if (!character_is_whitespace(*ptr)){
                word_ptr = ptr;
                goto consuming_non_whitespace;
            }
            
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            switch (*ptr){
                default:
                {
                    newline_layout_consume_default(&newline_vars);
                    lr_tb_write(&pos_vars, arena, &list, index, *ptr);
                    first_of_the_line = false;
                }break;
                
                case '\r':
                {
                    newline_layout_consume_CR(&newline_vars, index);
                }break;
                
                case '\n':
                {
                    if (first_of_the_line){
                        f32 shift = layout_index_x_shift(file, index,
                                                         metrics.space_advance);
                        lr_tb_advance_x_without_item(&pos_vars, shift);
                    }
                    
                    u64 newline_index = newline_layout_consume_LF(&newline_vars, index);
                    lr_tb_write_blank(&pos_vars, arena, &list, newline_index);
                    lr_tb_next_line(&pos_vars);
                    first_of_the_line = true;
                    ptr += 1;
                    goto skipping_leading_whitespace;
                }break;
            }
        }
        
        finish:
        if (newline_layout_consume_finish(&newline_vars)){
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            if (first_of_the_line){
                f32 shift = layout_index_x_shift(file, index,
                                                 metrics.space_advance);
                lr_tb_advance_x_without_item(&pos_vars, shift);
            }
            lr_tb_write_blank(&pos_vars, arena, &list, index);
        }
    }
    
    layout_item_list_finish(&list, -pos_vars.line_to_text_shift);
    
    return(list);
}

function Layout_Item_List
layout_virt_indent_index_unwrapped(Application_Links *app, Arena *arena,
                                   Buffer_ID buffer, Range_i64 range, Face_ID face,
                                   f32 width){
    Layout_Item_List result = {};
    
    code_index_lock();
    Code_Index_File *file = code_index_get_file(buffer);
    if (file != 0){
        result = layout_index_unwrapped__inner(app, arena, buffer, range, face, width, file);
    }
    code_index_unlock();
    if (file == 0){
        result = layout_virt_indent_literal_unwrapped(app, arena, buffer, range, face, width);
    }
    
    return(result);
}

// BOTTOM

