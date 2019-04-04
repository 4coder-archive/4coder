/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

// NOTE(allen|a4.0.14): This turned out to be a nasty little routine.  There might 
// be a better way to do it with just tokens that I didn't see the first time 
// through.  Once I build a real parser this should become almost just as easy as 
// iterating tokens is now.
//

static Buffered_Write_Stream
make_buffered_write_stream(Buffer_ID output_buffer_id, Partition *buffering_arena){
    Buffered_Write_Stream stream = {};
    stream.output_buffer_id = output_buffer_id;
    stream.buffering_arena = buffering_arena;
    stream.buffer = push_array(buffering_arena, char, 0);
    return(stream);
}

static void
buffered_write_stream_flush(Application_Links *app, Buffered_Write_Stream *stream){
    Buffer_ID buffer = stream->output_buffer_id;
    i32 buffer_size = 0;
    buffer_get_size(app, buffer, &buffer_size);
    i32 stream_size = (i32)(push_array(stream->buffering_arena, char, 0) - stream->buffer);
    buffer_replace_range(app, buffer, buffer_size, buffer_size, make_string(stream->buffer, stream_size));
    stream->buffering_arena->pos -= stream_size;
}

static void
buffered_write_stream_write(Application_Links *app, Buffered_Write_Stream *stream, String text){
    for (;text.size > 0;){
        char *buffered = push_array(stream->buffering_arena, char, text.size);
        if (buffered != 0){
            memcpy(buffered, text.str, text.size);
            text.size = 0;
        }
        else{
            i32 partial_size = part_remaining(stream->buffering_arena);
            buffered = push_array(stream->buffering_arena, char, partial_size);
            Assert(partial_size < text.size);
            memcpy(buffered, text.str, partial_size);
            text.size -= partial_size;
            text.str += partial_size;
            buffered_write_stream_flush(app, stream);
        }
    }
}

static void
buffered_write_stream_write_int(Application_Links *app, Buffered_Write_Stream *stream, i32 x){
    char space[128];
    String integer_string = make_fixed_width_string(space);
    append_int_to_str(&integer_string, x);
    buffered_write_stream_write(app, stream, integer_string);
}

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
print_positions_buffered(Application_Links *app, Buffer_ID buffer, Function_Positions *positions_array, i32 positions_count, Buffered_Write_Stream *stream){
    Arena *scratch = context_get_arena(app);
    Temp_Memory_Arena temp = begin_temp_memory(scratch);
    
    String buffer_name = buffer_push_unique_buffer_name(app, buffer, scratch);
    
    for (i32 i = 0; i < positions_count; ++i){
        Function_Positions *positions = &positions_array[i];
        
        i32 start_index = positions->sig_start_index;
        i32 end_index = positions->sig_end_index;
        i32 open_paren_pos = positions->open_paren_pos;
        i32 line_number = buffer_get_line_number(app, buffer, open_paren_pos);
        
        Assert(end_index > start_index);
        
        Token_Range token_range = buffer_get_token_range(app, buffer);
        if (token_range.first != 0){
            buffered_write_stream_write(app, stream, buffer_name);
            buffered_write_stream_write(app, stream, make_lit_string(":"));
            buffered_write_stream_write_int(app, stream, line_number);
            buffered_write_stream_write(app, stream, make_lit_string(": "));
            
            Cpp_Token prev_token = {};
            Token_Iterator token_it = make_token_iterator(token_range, start_index);
            for (Cpp_Token *token = token_iterator_current(&token_it);
                 token != 0 && token_iterator_current_index(&token_it) <= end_index;
                 token = token_iterator_goto_next_raw(&token_it)){
                if ((token->flags & CPP_TFLAG_PP_BODY) == 0 && token->type != CPP_TOKEN_COMMENT){
                    char space[2 << 10];
                    i32 token_size = token->size;
                    if (token_size > sizeof(space)){
                        token_size = sizeof(space);
                    }
                    buffer_read_range(app, buffer, token->start, token->start + token_size, space);
                    
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
                        buffered_write_stream_write(app, stream, make_lit_string(" "));
                    }
                    buffered_write_stream_write(app, stream, make_string(space, token_size));
                    
                    prev_token = *token;
                }
            }
            
            buffered_write_stream_write(app, stream, make_lit_string("\n"));
        }
    }
    
    end_temp_memory(temp);
}

static void
list_all_functions(Application_Links *app, Partition *part, Buffer_ID optional_target_buffer){
    String decls_name = make_lit_string("*decls*");
    Buffer_ID decls_buffer = 0;
    get_buffer_by_name(app, decls_name, AccessAll, &decls_buffer);
    if (!buffer_exists(app, decls_buffer)){
        create_buffer(app, decls_name, BufferCreate_AlwaysNew, &decls_buffer);
        buffer_set_setting(app, decls_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_WrapLine, false);
    }
    else{
        buffer_send_end_signal(app, decls_buffer);
        i32 size = 0;
        buffer_get_size(app, decls_buffer, &size);
        buffer_replace_range(app, decls_buffer, 0, size, make_lit_string(""));
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    
    i32 positions_max = (4<<10)/sizeof(Function_Positions);
    Function_Positions *positions_array = push_array(part, Function_Positions, positions_max);
    
    Buffered_Write_Stream buffered_write_stream = make_buffered_write_stream(decls_buffer, part);
    
    Buffer_ID buffer_it = 0;
    for (get_buffer_next(app, 0, AccessAll, &buffer_it);
         buffer_it != 0;
         get_buffer_next(app, buffer_it, AccessAll, &buffer_it)){
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
                
                print_positions_buffered(app, buffer, positions_array, positions_count, &buffered_write_stream);
                //print_positions(app, &buffer, positions_array, positions_count, &decls_buffer, part);
            }while(still_looping);
            
            if (optional_target_buffer != 0){
                break;
            }
        }
        else{
            continue;
        }
    }
    
    buffered_write_stream_flush(app, &buffered_write_stream);
    
    View_Summary view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, decls_buffer, 0);
    
    lock_jump_buffer(decls_name.str, decls_name.size);
    
    end_temp_memory(temp);
    
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer)
CUSTOM_DOC("Creates a jump list of lines of the current buffer that appear to define or declare functions.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessProtected, &buffer);
    if (buffer_exists(app, buffer)){
        list_all_functions(app, &global_part, buffer);
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations in the buffer.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessProtected, &buffer);
    if (buffer_exists(app, buffer)){
        list_all_functions(app, &global_part, buffer);
        view = get_active_view(app, AccessAll);
        open_jump_lister(app, &global_part, &global_heap, &view, buffer, JumpListerActivation_OpenInUIView, 0);
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_all_buffers)
CUSTOM_DOC("Creates a jump list of lines from all buffers that appear to define or declare functions.")
{
    list_all_functions(app, &global_part, 0);
}

CUSTOM_COMMAND_SIG(list_all_functions_all_buffers_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations all buffers.")
{
    list_all_functions(app, &global_part, 0);
    View_Summary view = get_active_view(app, AccessAll);
    open_jump_lister(app, &global_part, &global_heap,
                     &view, view.buffer_id, JumpListerActivation_OpenInUIView, 0);
}

// BOTTOM

