/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

// NOTE(allen|a4.0.14): This turned out to be a nasty little routine.  There might 
// be a better way to do it with just tokens that I didn't see the first time 
// through.  Once I build a real parser this should become almost just as easy as 
// iterating tokens is now.
//

static Get_Positions_Results
get_function_positions(Application_Links *app, Buffer_ID buffer, i32 first_token_index, Function_Positions *positions_array, i32 positions_max){
    Get_Positions_Results result = {};
    
    Token_Range token_range = buffer_get_token_range(app, buffer);
    if (token_range.first != 0){
        Token_Iterator token_it = make_token_iterator(token_range, first_token_index);
        
        i32 nest_level = 0;
        i32 paren_nest_level = 0;
        
        Cpp_Token *first_paren = 0;
        i32 first_paren_index = 0;
        i32 first_paren_position = 0;
        i32 last_paren_index = 0;
        
        // Look for the next token at global scope that might need to be printed.
        mode1:
        Assert(nest_level == 0);
        Assert(paren_nest_level == 0);
        first_paren_index = 0;
        first_paren_position = 0;
        last_paren_index = 0;
        for (Cpp_Token *token = token_iterator_current(&token_it);
             token != 0;
             token = token_iterator_goto_next(&token_it)){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                switch (token->type){
                    case CPP_TOKEN_BRACE_OPEN:
                    {
                        ++nest_level;
                    }break;
                    
                    case CPP_TOKEN_BRACE_CLOSE:
                    {
                        if (nest_level > 0){
                            --nest_level;
                        }
                    }break;
                    
                    case CPP_TOKEN_PARENTHESE_OPEN:
                    {
                        if (nest_level == 0){
                            first_paren = token;
                            first_paren_index = token_iterator_current_index(&token_it);
                            first_paren_position = token->start;
                            goto paren_mode1;
                        }
                    }break;
                }
            }
        }
        goto end;
        
        // Look for a closing parenthese to mark the end of a function signature.
        paren_mode1:
        paren_nest_level = 0;
        for (Cpp_Token *token = token_iterator_current(&token_it);
             token != 0;
             token = token_iterator_goto_next(&token_it)){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                switch (token->type){
                    case CPP_TOKEN_PARENTHESE_OPEN:
                    {
                        ++paren_nest_level;
                    }break;
                    
                    case CPP_TOKEN_PARENTHESE_CLOSE:
                    {
                        --paren_nest_level;
                        if (paren_nest_level == 0){
                            last_paren_index = token_iterator_current_index(&token_it);
                            goto paren_mode2;
                        }
                    }break;
                }
            }
        }
        goto end;
        
        // Look backwards from an open parenthese to find the start of a function signature.
        paren_mode2:
        {
            Cpp_Token *restore_point = token_iterator_current(&token_it);
            
            token_iterator_set(&token_it, first_paren);
            i32 signature_start_index = 0;
            for (Cpp_Token *token = token_iterator_current(&token_it);
                 token != 0;
                 token = token_iterator_goto_prev(&token_it)){
                if ((token->flags & CPP_TFLAG_PP_BODY) || (token->flags & CPP_TFLAG_PP_DIRECTIVE) ||
                    token->type == CPP_TOKEN_BRACE_CLOSE || token->type == CPP_TOKEN_SEMICOLON || token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                    token_iterator_goto_next(&token_it);
                    signature_start_index = token_iterator_current_index(&token_it);
                    if (signature_start_index == -1){
                        signature_start_index = first_paren_index;
                    }
                    goto paren_mode2_done;
                }
            }
            
            // When this loop ends by going all the way back to the beginning set the 
            // signature start to 0 and fall through to the printing phase.
            signature_start_index = 0;
            
            paren_mode2_done:;
            {
                Function_Positions positions;
                positions.sig_start_index = signature_start_index;
                positions.sig_end_index = last_paren_index;
                positions.open_paren_pos = first_paren_position;
                positions_array[result.positions_count++] = positions;
            }
            
            token_iterator_set(&token_it, restore_point);
            if (result.positions_count >= positions_max){
                result.next_token_index = token_iterator_current_index(&token_it);
                result.still_looping = true;
                goto end;
            }
            
            goto mode1;
        }
        end:;
        
    }
    
    return(result);
}

static void
print_positions_buffered(Application_Links *app, Buffer_Insertion *out, Buffer_ID buffer, Function_Positions *positions_array, i32 positions_count){
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer);
    
    for (i32 i = 0; i < positions_count; ++i){
        Function_Positions *positions = &positions_array[i];
        
        i32 start_index = positions->sig_start_index;
        i32 end_index = positions->sig_end_index;
        i64 open_paren_pos = positions->open_paren_pos;
        i64 line_number = get_line_number_from_pos(app, buffer, open_paren_pos);
        
        Assert(end_index > start_index);
        
        Token_Range token_range = buffer_get_token_range(app, buffer);
        if (token_range.first != 0){
            insertf(out, "%.*s:%lld: ", string_expand(buffer_name), line_number);
            
            Cpp_Token prev_token = {};
            Token_Iterator token_it = make_token_iterator(token_range, start_index);
            for (Cpp_Token *token = token_iterator_current(&token_it);
                 token != 0 && token_iterator_current_index(&token_it) <= end_index;
                 token = token_iterator_goto_next_raw(&token_it)){
                if ((token->flags & CPP_TFLAG_PP_BODY) == 0 && token->type != CPP_TOKEN_COMMENT){
                    if ((prev_token.type == CPP_TOKEN_IDENTIFIER ||
                         prev_token.type == CPP_TOKEN_STAR ||
                         prev_token.type == CPP_TOKEN_COMMA ||
                         (prev_token.flags & CPP_TFLAG_IS_KEYWORD) != 0
                         ) &&
                        !(token->type == CPP_TOKEN_PARENTHESE_OPEN ||
                          token->type == CPP_TOKEN_PARENTHESE_CLOSE ||
                          token->type == CPP_TOKEN_COMMA
                          )
                        ){
                        insertc(out, ' ');
                    }
                    
                    Temp_Memory token_temp = begin_temp(scratch);
                    String_Const_u8 lexeme = push_token_lexeme(app, scratch, buffer, *token);
                    insert_string(out, lexeme);
                    end_temp(token_temp);
                    
                    prev_token = *token;
                }
            }
            
            insertc(out, '\n');
        }
    }
    
    end_temp(temp);
}

static void
list_all_functions(Application_Links *app, Buffer_ID optional_target_buffer){
    // TODO(allen): Use create or switch to buffer and clear here?
    String_Const_u8 decls_name = string_u8_litexpr("*decls*");
    Buffer_ID decls_buffer = get_buffer_by_name(app, decls_name, AccessAll);
    if (!buffer_exists(app, decls_buffer)){
        decls_buffer = create_buffer(app, decls_name, BufferCreate_AlwaysNew);
        buffer_set_setting(app, decls_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_WrapLine, false);
    }
    else{
        clear_buffer(app, decls_buffer);
        buffer_send_end_signal(app, decls_buffer);
    }
    
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    
    // TODO(allen): rewrite get_function_positions  to allocate on arena
    i32 positions_max = (4<<10)/sizeof(Function_Positions);
    Function_Positions *positions_array = push_array(scratch, Function_Positions, positions_max);
    
    Cursor insertion_cursor = make_cursor(push_array(scratch, u8, KB(256)), KB(256));
    Buffer_Insertion out = begin_buffer_insertion_at_buffered(app, decls_buffer, 0, &insertion_cursor);
    
    for (Buffer_ID buffer_it = get_buffer_next(app, 0, AccessAll);
         buffer_it != 0;
         buffer_it = get_buffer_next(app, buffer_it, AccessAll)){
        Buffer_ID buffer = buffer_it;
        if (optional_target_buffer != 0){
            buffer = optional_target_buffer;
        }
        
        if (buffer_tokens_are_ready(app, buffer)){
            i32 token_index = 0;
            b32 still_looping = false;
            do{
                Get_Positions_Results get_positions_results = get_function_positions(app, buffer, token_index, positions_array, positions_max);
                
                i32 positions_count = get_positions_results.positions_count;
                token_index = get_positions_results.next_token_index;
                still_looping = get_positions_results.still_looping;
                
                print_positions_buffered(app, &out, buffer, positions_array, positions_count);
            }while(still_looping);
            
            if (optional_target_buffer != 0){
                break;
            }
        }
        else{
            continue;
        }
    }
    
    end_buffer_insertion(&out);
    
    View_ID view = get_active_view(app, AccessAll);
    view_set_buffer(app, view, decls_buffer, 0);
    
    lock_jump_buffer(decls_name);
    
    end_temp(temp);
    
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer)
CUSTOM_DOC("Creates a jump list of lines of the current buffer that appear to define or declare functions.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    if (buffer != 0){
        list_all_functions(app, buffer);
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations in the buffer.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    if (buffer != 0){
        list_all_functions(app, buffer);
        view = get_active_view(app, AccessAll);
        open_jump_lister(app, &global_heap, view, buffer, JumpListerActivation_OpenInUIView, 0);
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_all_buffers)
CUSTOM_DOC("Creates a jump list of lines from all buffers that appear to define or declare functions.")
{
    list_all_functions(app, 0);
}

CUSTOM_COMMAND_SIG(list_all_functions_all_buffers_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations all buffers.")
{
    list_all_functions(app, 0);
    View_ID view = get_active_view(app, AccessAll);
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    open_jump_lister(app, &global_heap, view, buffer, JumpListerActivation_OpenInUIView, 0);
}

// BOTTOM

