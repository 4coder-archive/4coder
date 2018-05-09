/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

// NOTE(allen|a4.0.14): This turned out to be a nasty little routine.  There might 
// be a better way to do it with just tokens that I didn't see the first time 
// through.  Once I build a real parser this should become almost just as easy as 
// iterating tokens is now.
//
// This version can be dropped anywhere underneath 4coder_default_include.cpp and
// will then provide the "list_all_functions_current_buffer" command.
//

static Get_Positions_Results
get_function_positions(Application_Links *app, Buffer_Summary *buffer, int32_t token_index, Function_Positions *positions_array, int32_t positions_max){
    Get_Positions_Results result = {0};
    
    static const int32_t token_chunk_size = 512;
    Cpp_Token token_chunk[token_chunk_size];
    Stream_Tokens token_stream = {0};
    
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
print_positions(Application_Links *app, Buffer_Summary *buffer, Function_Positions *positions_array, int32_t positions_count, Buffer_Summary *output_buffer, Partition *part){
    
    Temp_Memory temp = begin_temp_memory(part);
    
    Partition extra_memory_ = partition_sub_part(part, (4<<10));
    Partition *extra_memory = &extra_memory_;
    
    char *str_ptr = (char*)partition_current(part);
    int32_t part_size = 0;
    
    String buffer_name = make_string(buffer->buffer_name, buffer->buffer_name_len);
    int32_t size = output_buffer->size;
    
    for (int32_t i = 0; i < positions_count; ++i){
        Function_Positions *positions = &positions_array[i];
        Temp_Memory extra_temp = begin_temp_memory(extra_memory);
        
        int32_t local_index = positions->sig_start_index;
        int32_t end_index = positions->sig_end_index;
        int32_t open_paren_pos = positions->open_paren_pos;
        
        Assert(end_index > local_index);
        
        static const int32_t sig_chunk_size = 64;
        Cpp_Token sig_chunk[sig_chunk_size];
        Stream_Tokens sig_stream = {0};
        if (init_stream_tokens(&sig_stream, app, buffer, local_index, sig_chunk, sig_chunk_size)){
            bool32 still_looping = false;
            do{
                for (; local_index < sig_stream.end; ++local_index){
                    Cpp_Token *token = &sig_stream.tokens[local_index];
                    if (!(token->flags & CPP_TFLAG_PP_BODY) && token->type != CPP_TOKEN_COMMENT){
                        bool32 delete_space_before = false;
                        bool32 space_after = false;
                        
                        switch (token->type){
                            case CPP_TOKEN_PARENTHESE_OPEN:
                            case CPP_TOKEN_PARENTHESE_CLOSE:
                            {
                                delete_space_before = true;
                            }break;
                            
                            case CPP_TOKEN_IDENTIFIER:
                            case CPP_TOKEN_STAR:
                            {
                                space_after = true;
                            }break;
                            
                            case CPP_TOKEN_COMMA:
                            {
                                delete_space_before = true;
                                space_after = true;
                            }break;
                        }
                        
                        if (token->flags & CPP_TFLAG_IS_KEYWORD){
                            space_after = true;
                        }
                        
                        if (delete_space_before){
                            int32_t pos = extra_memory->pos - 1;
                            char *base = ((char*)(extra_memory->base));
                            if (pos >= 0 && base[pos] == ' '){
                                extra_memory->pos = pos;
                            }
                        }
                        
                        char *token_str = push_array(extra_memory, char, token->size + space_after);
                        if (token_str != 0){
                            buffer_read_range(app, buffer, token->start, token->start + token->size, token_str);
                            if (space_after){
                                token_str[token->size] = ' ';
                            }
                        }
                        else{
                            goto finish_print;
                        }
                        
                    }
                    
                    if (local_index == end_index){
                        goto finish_print;
                    }
                }
                still_looping = forward_stream_tokens(&sig_stream);
            }while(still_looping);
            
            finish_print:;
            {
                int32_t sig_size = extra_memory->pos;
                String sig = make_string(extra_memory->base, sig_size);
                
                int32_t line_number = buffer_get_line_number(app, buffer, open_paren_pos);
                int32_t line_number_len = int_to_str_size(line_number);
                
                int32_t append_len = buffer_name.size + 1 + line_number_len + 1 + 1 + sig_size + 1;
                
                char *out_space = push_array(part, char, append_len);
                if (out_space == 0){
                    buffer_replace_range(app, output_buffer, size, size, str_ptr, part_size);
                    size += part_size;
                    
                    end_temp_memory(temp);
                    temp = begin_temp_memory(part);
                    
                    part_size = 0;
                    out_space = push_array(part, char, append_len);
                }
                
                part_size += append_len;
                String out = make_string(out_space, 0, append_len);
                append(&out, buffer_name);
                append(&out, ':');
                append_int_to_str(&out, line_number);
                append(&out, ':');
                append(&out, ' ');
                append(&out, sig);
                append(&out, '\n');
            }
        }
        
        end_temp_memory(extra_temp);
    }
    
    buffer_replace_range(app, output_buffer, size, size, str_ptr, part_size);
    end_temp_memory(temp);
}

static void
list_all_functions(Application_Links *app, Partition *part, Buffer_Summary *buffer){
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
    
    int32_t token_index = 0;
    bool32 still_looping = false;
    do{
        Get_Positions_Results get_positions_results = get_function_positions(app, buffer, token_index, positions_array, positions_max);
        
        int32_t positions_count = get_positions_results.positions_count;
        token_index = get_positions_results.next_token_index;
        still_looping = get_positions_results.still_looping;
        
        print_positions(app, buffer, positions_array, positions_count, &decls_buffer, part);
    }while(still_looping);
    
    View_Summary view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, decls_buffer.buffer_id, 0);
    
    lock_jump_buffer(search_name.str, search_name.size);
    
    end_temp_memory(temp);
    
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer)
CUSTOM_DOC("Creates a jump list of lines of the current buffer that appear to define or declare functions.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    list_all_functions(app, &global_part, &buffer);
}

// BOTTOM

