/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

// NOTE(allen|a4.0.14): This turned out to be a nasty little routine.  There might
// be a better way to do it with just tokens that I didn't see the first time
// through.  Once I build a real parser this should become almost just as easy as
// iterating tokens is now.
//
// NOTE(allen|b4.1.0): This routine assumes C++ sub_kinds in the tokens of the buffer.

function Get_Positions_Results
get_function_positions(Application_Links *app, Buffer_ID buffer, i64 first_token_index, Function_Positions *positions_array, i64 positions_max){
    Get_Positions_Results result = {};
    
    Token_Array array = get_token_array_from_buffer(app, buffer);
    if (array.tokens != 0){
        Token_Iterator_Array it = token_iterator_index(buffer, &array, first_token_index);
        
        i32 nest_level = 0;
        i32 paren_nest_level = 0;
        
        Token_Iterator_Array first_paren_it = {};
        i64 first_paren_index = 0;
        i64 first_paren_position = 0;
        i64 last_paren_index = 0;
        
        // Look for the next token at global scope that might need to be printed.
        mode1:
        Assert(nest_level == 0);
        Assert(paren_nest_level == 0);
        first_paren_index = 0;
        first_paren_position = 0;
        last_paren_index = 0;
        for (;;){
            Token *token = token_it_read(&it);
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
                switch (token->sub_kind){
                    case TokenCppKind_BraceOp:
                    {
                        ++nest_level;
                    }break;
                    
                    case TokenCppKind_BraceCl:
                    {
                        if (nest_level > 0){
                            --nest_level;
                        }
                    }break;
                    
                    case TokenCppKind_ParenOp:
                    {
                        if (nest_level == 0){
                            first_paren_it = it;
                            first_paren_index = token_it_index(&it);
                            first_paren_position = token->pos;
                            goto paren_mode1;
                        }
                    }break;
                }
            }
            if (!token_it_inc(&it)){
                goto end;
            }
        }
        
        // Look for a closing parenthese to mark the end of a function signature.
        paren_mode1:
        paren_nest_level = 0;
        for (;;){
            Token *token = token_it_read(&it);
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
                switch (token->sub_kind){
                    case TokenCppKind_ParenOp:
                    {
                        ++paren_nest_level;
                    }break;
                    
                    case TokenCppKind_ParenCl:
                    {
                        --paren_nest_level;
                        if (paren_nest_level == 0){
                            last_paren_index = token_it_index(&it);
                            goto paren_mode2;
                        }
                    }break;
                }
            }
            if (!token_it_inc(&it)){
                goto end;
            }
        }
        
        // Look backwards from an open parenthese to find the start of a function signature.
        paren_mode2:
        {
            Token_Iterator_Array restore_point = it;
            it = first_paren_it;
            i64 signature_start_index = 0;
            for (;;){
                Token *token = token_it_read(&it);
                if (HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                    token->sub_kind == TokenCppKind_BraceCl ||
                    token->sub_kind == TokenCppKind_Semicolon ||
                    token->sub_kind == TokenCppKind_ParenCl){
                    if (!token_it_inc(&it)){
                        signature_start_index = first_paren_index;
                    }
                    else{
                        signature_start_index = token_it_index(&it);
                    }
                    goto paren_mode2_done;
                }
                if (!token_it_dec(&it)){
                    break;
                }
            }
            
            // When this loop ends by going all the way back to the beginning set the
            // signature start to 0 and fall through to the printing phase.
            signature_start_index = 0;
            
            paren_mode2_done:;
            {
                Function_Positions positions = {};
                positions.sig_start_index = signature_start_index;
                positions.sig_end_index = last_paren_index;
                positions.open_paren_pos = first_paren_position;
                positions_array[result.positions_count++] = positions;
            }
            
            it = restore_point;
            if (result.positions_count >= positions_max){
                result.next_token_index = token_it_index(&it);
                result.still_looping = true;
                goto end;
            }
            
            goto mode1;
        }
        end:;
    }
    
    return(result);
}

function void
print_positions_buffered(Application_Links *app, Buffer_Insertion *out, Buffer_ID buffer, Function_Positions *positions_array, i64 positions_count){
    Scratch_Block scratch(app);
    
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer);
    
    for (i32 i = 0; i < positions_count; ++i){
        Function_Positions *positions = &positions_array[i];
        
        i64 start_index = positions->sig_start_index;
        i64 end_index = positions->sig_end_index;
        i64 open_paren_pos = positions->open_paren_pos;
        i64 line_number = get_line_number_from_pos(app, buffer, open_paren_pos);
        
        Assert(end_index > start_index);
        
        Token_Array array = get_token_array_from_buffer(app, buffer);
        if (array.tokens != 0){
            insertf(out, "%.*s:%lld: ", string_expand(buffer_name), line_number);
            
            Token prev_token = {};
            Token_Iterator_Array it = token_iterator_index(buffer, &array, start_index);
            for (;;){
                Token *token = token_it_read(&it);
                if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) &&
                    token->kind != TokenBaseKind_Comment &&
                    token->kind != TokenBaseKind_Whitespace){
                    if ((prev_token.sub_kind == TokenCppKind_Identifier ||
                         prev_token.sub_kind == TokenCppKind_Star ||
                         prev_token.sub_kind == TokenCppKind_Comma ||
                         prev_token.kind == TokenBaseKind_Keyword) &&
                        !(token->sub_kind == TokenCppKind_ParenOp ||
                          token->sub_kind == TokenCppKind_ParenCl ||
                          token->sub_kind == TokenCppKind_Comma)){
                        insertc(out, ' ');
                    }
                    
                    Temp_Memory token_temp = begin_temp(scratch);
                    String_Const_u8 lexeme = push_token_lexeme(app, scratch, buffer, token);
                    insert_string(out, lexeme);
                    end_temp(token_temp);
                    
                    prev_token = *token;
                }
                if (!token_it_inc(&it)){
                    break;
                }
                i64 index = token_it_index(&it);
                if (index > end_index){
                    break;
                }
            }
            
            insertc(out, '\n');
        }
    }
}

function void
list_all_functions(Application_Links *app, Buffer_ID optional_target_buffer){
    // TODO(allen): Use create or switch to buffer and clear here?
    String_Const_u8 decls_name = string_u8_litexpr("*decls*");
    Buffer_ID decls_buffer = get_buffer_by_name(app, decls_name, Access_Always);
    if (!buffer_exists(app, decls_buffer)){
        decls_buffer = create_buffer(app, decls_name, BufferCreate_AlwaysNew);
        buffer_set_setting(app, decls_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_ReadOnly, true);
        //buffer_set_setting(app, decls_buffer, BufferSetting_WrapLine, false);
    }
    else{
        clear_buffer(app, decls_buffer);
        buffer_send_end_signal(app, decls_buffer);
    }
    
    Scratch_Block scratch(app);
    
    // TODO(allen): rewrite get_function_positions to allocate on arena
    i32 positions_max = KB(4)/sizeof(Function_Positions);
    Function_Positions *positions_array = push_array(scratch, Function_Positions, positions_max);
    
    Cursor insertion_cursor = make_cursor(push_array(scratch, u8, KB(256)), KB(256));
    Buffer_Insertion out = begin_buffer_insertion_at_buffered(app, decls_buffer, 0, &insertion_cursor);
    
    for (Buffer_ID buffer_it = get_buffer_next(app, 0, Access_Always);
         buffer_it != 0;
         buffer_it = get_buffer_next(app, buffer_it, Access_Always)){
        Buffer_ID buffer = buffer_it;
        if (optional_target_buffer != 0){
            buffer = optional_target_buffer;
        }
        
        Token_Array array = get_token_array_from_buffer(app, buffer);
        if (array.tokens != 0){
            i64 token_index = 0;
            b32 still_looping = false;
            do{
                Get_Positions_Results get_positions_results = get_function_positions(app, buffer, token_index, positions_array, positions_max);
                
                i64 positions_count = get_positions_results.positions_count;
                token_index = get_positions_results.next_token_index;
                still_looping = get_positions_results.still_looping;
                
                print_positions_buffered(app, &out, buffer, positions_array, positions_count);
            }while(still_looping);
            
            if (optional_target_buffer != 0){
                break;
            }
        }
    }
    
    end_buffer_insertion(&out);
    
    View_ID view = get_active_view(app, Access_Always);
    view_set_buffer(app, view, decls_buffer, 0);
    
    lock_jump_buffer(app, decls_name);
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer)
CUSTOM_DOC("Creates a jump list of lines of the current buffer that appear to define or declare functions.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    if (buffer != 0){
        list_all_functions(app, buffer);
    }
}

CUSTOM_UI_COMMAND_SIG(list_all_functions_current_buffer_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations in the buffer.")
{
    Heap *heap = &global_heap;
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    if (buffer != 0){
        list_all_functions(app, buffer);
        view = get_active_view(app, Access_Always);
        buffer = view_get_buffer(app, view, Access_Always);
        Marker_List *list = get_or_make_list_for_buffer(app, heap, buffer);
        if (list != 0){
            Jump_Lister_Result jump = get_jump_index_from_user(app, list, "Function:");
            jump_to_jump_lister_result(app, view, list, &jump);
        }
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_all_buffers)
CUSTOM_DOC("Creates a jump list of lines from all buffers that appear to define or declare functions.")
{
    list_all_functions(app, 0);
}

CUSTOM_UI_COMMAND_SIG(list_all_functions_all_buffers_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations all buffers.")
{
    Heap *heap = &global_heap;
    list_all_functions(app, 0);
    View_ID view = get_active_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Marker_List *list = get_or_make_list_for_buffer(app, heap, buffer);
    if (list != 0){
        Jump_Lister_Result jump = get_jump_index_from_user(app, list, "Function:");
        jump_to_jump_lister_result(app, view, list, &jump);
    }
}

// BOTTOM

