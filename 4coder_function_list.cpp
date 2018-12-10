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
    Buffer_Summary buffer = get_buffer(app, stream->output_buffer_id, AccessProtected);
    int32_t buffer_size = (int32_t)(push_array(stream->buffering_arena, char, 0) - stream->buffer);
    buffer_replace_range(app, &buffer, buffer.size, buffer.size, stream->buffer, buffer_size);
    stream->buffering_arena->pos -= buffer_size;
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
            int32_t partial_size = partition_remaining(stream->buffering_arena);
            buffered = push_array(stream->buffering_arena, char, partial_size);
            Assert(partial_size < text.size);
            memcpy(buffered, text.str, partial_size);
            text.size -= partial_size;
            text.str += partial_size;
            buffered_write_stream_flush(app, stream);
        }
    }
}

static Get_Positions_Results
get_function_positions(Application_Links *app, Buffer_Summary *buffer, int32_t token_index, Function_Positions *positions_array, int32_t positions_max){
    Get_Positions_Results result = {};
    
    static const int32_t token_chunk_size = 512;
    Cpp_Token token_chunk[token_chunk_size];
    Stream_Tokens token_stream = {};
    
    if (init_stream_tokens(&token_stream, app, buffer, token_index, token_chunk, token_chunk_size)){
        int32_t nest_level = 0;
        int32_t paren_nest_level = 0;
        
        int32_t first_paren_index = 0;
        int32_t first_paren_position = 0;
        int32_t last_paren_index = 0;
        
        bool32 still_looping = false;
        
        // Look for the next token at global scope that might need to be printed.
        mode1:
        Assert(nest_level == 0);
        Assert(paren_nest_level == 0);
        first_paren_index = 0;
        first_paren_position = 0;
        last_paren_index = 0;
        
        do{
            for (; token_index < token_stream.end; ++token_index){
                Cpp_Token *token = &token_stream.tokens[token_index];
                
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
                                first_paren_index = token_index;
                                first_paren_position = token->start;
                                goto paren_mode1;
                            }
                        }break;
                    }
                }
            }
            still_looping = forward_stream_tokens(&token_stream);
        }while(still_looping);
        goto end;
        
        // Look for a closing parenthese to mark the end of a function signature.
        paren_mode1:
        paren_nest_level = 0;
        do{
            for (; token_index < token_stream.end; ++token_index){
                Cpp_Token *token = &token_stream.tokens[token_index];
                
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
                                last_paren_index = token_index;
                                goto paren_mode2;
                            }
                        }break;
                    }
                }
            }
            still_looping = forward_stream_tokens(&token_stream);
        }while(still_looping);
        goto end;
        
        // Look backwards from an open parenthese to find the start of a function signature.
        paren_mode2: {
            Stream_Tokens backward_stream_temp = begin_temp_stream_token(&token_stream);
            int32_t local_index = first_paren_index;
            int32_t signature_start_index = 0;
            
            do{
                for (; local_index >= token_stream.start; --local_index){
                    Cpp_Token *token = &token_stream.tokens[local_index];
                    if ((token->flags & CPP_TFLAG_PP_BODY) || (token->flags & CPP_TFLAG_PP_DIRECTIVE) || token->type == CPP_TOKEN_BRACE_CLOSE || token->type == CPP_TOKEN_SEMICOLON || token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                        ++local_index;
                        signature_start_index = local_index;
                        goto paren_mode2_done;
                    }
                }
                still_looping = backward_stream_tokens(&token_stream);
            }while(still_looping);
            // When this loop ends by going all the way back to the beginning set the signature start to 0 and fall through to the printing phase.
            signature_start_index = 0;
            
            paren_mode2_done:;
            {
                Function_Positions positions;
                positions.sig_start_index = signature_start_index;
                positions.sig_end_index = last_paren_index;
                positions.open_paren_pos = first_paren_position;
                positions_array[result.positions_count++] = positions;
            }
            
            end_temp_stream_token(&token_stream, backward_stream_temp);
            if (result.positions_count >= positions_max){
                result.next_token_index = token_index;
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
print_positions_buffered(Application_Links *app, Buffer_Summary *buffer, Function_Positions *positions_array, int32_t positions_count, Buffered_Write_Stream *stream){
    
    String buffer_name = make_string(buffer->buffer_name, buffer->buffer_name_len);
    
    for (int32_t i = 0; i < positions_count; ++i){
        Function_Positions *positions = &positions_array[i];
        
        int32_t local_index = positions->sig_start_index;
        int32_t end_index = positions->sig_end_index;
        int32_t open_paren_pos = positions->open_paren_pos;
        int32_t line_number = buffer_get_line_number(app, buffer, open_paren_pos);
        
        Assert(end_index > local_index);
        
        static const int32_t sig_chunk_size = 64;
        Cpp_Token sig_chunk[sig_chunk_size];
        Stream_Tokens sig_stream = {};
        if (init_stream_tokens(&sig_stream, app, buffer, local_index, sig_chunk, sig_chunk_size)){
            buffered_write_stream_write(app, stream, buffer_name);
            buffered_write_stream_write(app, stream, make_lit_string(":"));
            {
                char space[64];
                String integer_string = make_fixed_width_string(space);
                append_int_to_str(&integer_string, line_number);
                buffered_write_stream_write(app, stream, integer_string);
            }
            buffered_write_stream_write(app, stream, make_lit_string(": "));
            
            bool32 still_looping = false;
            do{
                Cpp_Token prev_token = {};
                for (; local_index < sig_stream.end; ++local_index){
                    Cpp_Token *token = &sig_stream.tokens[local_index];
                    if ((token->flags & CPP_TFLAG_PP_BODY) == 0 && token->type != CPP_TOKEN_COMMENT){
                        char space[2 << 10];
                        int32_t token_size = token->size;
                        if (token_size > sizeof(space)){
                            token_size = sizeof(space);
                        }
                        buffer_read_range(app, buffer, token->start, token->start + token_size, space);
                        
                        bool32 insert_space = (/**/
                                               (prev_token.type == CPP_TOKEN_IDENTIFIER ||
                                                prev_token.type == CPP_TOKEN_STAR ||
                                                prev_token.type == CPP_TOKEN_COMMA ||
                                                (prev_token.flags & CPP_TFLAG_IS_KEYWORD) != 0
                                                ) &&
                                               !(token->type == CPP_TOKEN_PARENTHESE_OPEN ||
                                                 token->type == CPP_TOKEN_PARENTHESE_CLOSE ||
                                                 token->type == CPP_TOKEN_COMMA
                                                 )
                                               );
                        
                        if (insert_space){
                            buffered_write_stream_write(app, stream, make_lit_string(" "));
                        }
                        buffered_write_stream_write(app, stream, make_string(space, token_size));
                        
                        prev_token = *token;
                    }
                    
                    if (local_index == end_index){
                        goto doublebreak;
                    }
                }
                still_looping = forward_stream_tokens(&sig_stream);
            }while(still_looping);
            doublebreak:;
            
            buffered_write_stream_write(app, stream, make_lit_string("\n"));
        }
    }
}

static void
list_all_functions(Application_Links *app, Partition *part, Buffer_Summary *optional_target_buffer){
    String search_name = make_lit_string("*decls*");
    Buffer_Summary decls_buffer = get_buffer_by_name(app, search_name.str, search_name.size, AccessAll);
    if (!decls_buffer.exists){
        decls_buffer = create_buffer(app, search_name.str, search_name.size, BufferCreate_AlwaysNew);
        buffer_set_setting(app, &decls_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, &decls_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, &decls_buffer, BufferSetting_WrapLine, false);
    }
    else{
        buffer_send_end_signal(app, &decls_buffer);
        buffer_replace_range(app, &decls_buffer, 0, decls_buffer.size, 0, 0);
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    
    int32_t positions_max = (4<<10)/sizeof(Function_Positions);
    Function_Positions *positions_array = push_array(part, Function_Positions, positions_max);
    
    Buffered_Write_Stream buffered_write_stream = make_buffered_write_stream(decls_buffer.buffer_id, part);
    
    for (Buffer_Summary buffer_it = get_buffer_first(app, AccessAll);
         buffer_it.exists;
         get_buffer_next(app, &buffer_it, AccessAll)){
        Buffer_Summary buffer = buffer_it;
        if (optional_target_buffer != 0){
            buffer = *optional_target_buffer;
        }
        
        if (!buffer.tokens_are_ready){
            continue;
        }
        
        int32_t token_index = 0;
        bool32 still_looping = false;
        do{
            Get_Positions_Results get_positions_results = get_function_positions(app, &buffer, token_index, positions_array, positions_max);
            
            int32_t positions_count = get_positions_results.positions_count;
            token_index = get_positions_results.next_token_index;
            still_looping = get_positions_results.still_looping;
            
            print_positions_buffered(app, &buffer, positions_array, positions_count, &buffered_write_stream);
            //print_positions(app, &buffer, positions_array, positions_count, &decls_buffer, part);
        }while(still_looping);
        
        if (optional_target_buffer != 0){
            break;
        }
    }
    
    buffered_write_stream_flush(app, &buffered_write_stream);
    
    View_Summary view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, decls_buffer.buffer_id, 0);
    
    lock_jump_buffer(search_name.str, search_name.size);
    
    end_temp_memory(temp);
    
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer)
CUSTOM_DOC("Creates a jump list of lines of the current buffer that appear to define or declare functions.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    if (buffer.exists){
        list_all_functions(app, &global_part, &buffer);
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer_lister)
CUSTOM_DOC("Creates a lister of locations that look like function definitions and declarations in the buffer.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    if (buffer.exists){
        list_all_functions(app, &global_part, &buffer);
        view = get_active_view(app, AccessAll);
        open_jump_lister(app, &global_part, &global_heap,
                         &view, view.buffer_id, JumpListerActivation_OpenInUIView, 0);
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

