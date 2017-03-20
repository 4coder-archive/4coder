/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_FUNCTION_LIST_CPP)
#define FCODER_FUNCTION_LIST_CPP

#include "4coder_API/custom.h"

#include "4coder_default_framework.h"

#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_streaming.h"

#include "4coder_lib/4coder_mem.h"

// NOTE(allen|a4.0.14): This turned out to be a nasty little routine.  There might 
// be a better way to do it with just tokens that I didn't see the first time 
// through.  Once I build a real parser this should become almost just as easy as 
// iterating tokens is now.
//
// This version can be dropped anywhere underneath 4coder_default_include.cpp and
// will then provide the "list_all_functions_current_buffer" command.
//

//
// Declaration list
//

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
        buffer_replace_range(app, &decls_buffer, 0, decls_buffer.size, 0, 0);
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    
    struct Function_Positions{
        int32_t sig_start_index;
        int32_t sig_end_index;
        int32_t open_paren_pos;
    };
    
    Function_Positions *positions_array = push_array(part, Function_Positions, (4<<10)/sizeof(Function_Positions));
    int32_t positions_count = 0;
    
    Partition extra_memory_ = partition_sub_part(part, (4<<10));
    Partition *extra_memory = &extra_memory_;
    char *str = (char*)partition_current(part);
    size_t part_size = 0;
    size_t size = 0;
    
    static const int32_t token_chunk_size = 512;
    Cpp_Token token_chunk[token_chunk_size];
    Stream_Tokens token_stream = {0};
    
    if (init_stream_tokens(&token_stream, app, buffer, 0, token_chunk, token_chunk_size)){
        Stream_Tokens start_position_stream_temp = begin_temp_stream_token(&token_stream);
        
        int32_t token_index = 0;
        int32_t nest_level = 0;
        int32_t paren_nest_level = 0;
        
        int32_t first_paren_index = 0;
        int32_t first_paren_position = 0;
        int32_t last_paren_index = 0;
        
        bool32 still_looping = false;
        
        // Look for the next token at global scope that might need to be printed.
        mode1: do{
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
                positions_array[positions_count++] = positions;
            }
            
            end_temp_stream_token(&token_stream, backward_stream_temp);
            goto mode1;
        }
        
        end:;
        end_temp_stream_token(&token_stream, start_position_stream_temp);
        // Print the results
        String buffer_name = make_string(buffer->buffer_name, buffer->buffer_name_len);
        for (int32_t i = 0; i < positions_count; ++i){
            Function_Positions *positions = &positions_array[i];
            Temp_Memory extra_temp = begin_temp_memory(extra_memory);
            
            int32_t local_index = positions->sig_start_index;
            int32_t end_index = positions->sig_end_index;
            int32_t open_paren_pos = positions->open_paren_pos;
            
            do{
                for (; local_index < token_stream.end; ++local_index){
                    Cpp_Token *token = &token_stream.tokens[local_index];
                    if (!(token->flags & CPP_TFLAG_PP_BODY)){
                        if (token->type != CPP_TOKEN_COMMENT){
                            bool32 delete_space_before = false;
                            bool32 space_after = false;
                            
                            switch (token->type){
                                case CPP_TOKEN_COMMA:
                                case CPP_TOKEN_PARENTHESE_OPEN:
                                case CPP_TOKEN_PARENTHESE_CLOSE:
                                {
                                    delete_space_before = true;
                                }break;
                            }
                            
                            switch (token->type){
                                case CPP_TOKEN_IDENTIFIER:
                                case CPP_TOKEN_COMMA:
                                case CPP_TOKEN_STAR:
                                {
                                    space_after = true;
                                }break;
                            }
                            if (token->flags & CPP_TFLAG_IS_KEYWORD){
                                space_after = true;
                            }
                            
                            if (delete_space_before){
                                size_t pos = extra_memory->pos - 1;
                                char *base = ((char*)(extra_memory->base));
                                if (pos >= 0 && base[pos] == ' '){
                                    extra_memory->pos = pos;
                                }
                            }
                            
                            char *token_str = push_array(extra_memory, char, token->size + space_after);
                            
                            buffer_read_range(app, buffer, token->start, token->start + token->size, token_str);
                            if (space_after){
                                token_str[token->size] = ' ';
                            }
                        }
                    }
                    
                    if (local_index == end_index){
                        goto finish_print;
                    }
                }
                still_looping = forward_stream_tokens(&token_stream);
            }while(still_looping);
            
            finish_print:;
            {
                size_t sig_size = extra_memory->pos;
                String sig = make_string(extra_memory->base, (int32_t)sig_size);
                
                size_t line_number = buffer_get_line_index(app, buffer, open_paren_pos);
                size_t line_number_len = int_to_str_size((int32_t)line_number);
                
                size_t append_len = buffer_name.size + 1 + line_number_len + 1 + 1 + sig_size + 1;
                
                char *out_space = push_array(part, char, append_len);
                if (out_space == 0){
                    buffer_replace_range(app, &decls_buffer, size, size, str, part_size);
                    size += part_size;
                    
                    end_temp_memory(temp);
                    temp = begin_temp_memory(part);
                    
                    part_size = 0;
                    out_space = push_array(part, char, append_len);
                }
                
                part_size += append_len;
                String out = make_string(out_space, 0, (int32_t)append_len);
                append(&out, buffer_name);
                append(&out, ':');
                append_int_to_str(&out, (int32_t)line_number);
                append(&out, ':');
                append(&out, ' ');
                append(&out, sig);
                append(&out, '\n');
            }
            
            end_temp_memory(extra_temp);
        }
        
        buffer_replace_range(app, &decls_buffer, size, size, str, part_size);
        
        View_Summary view = get_active_view(app, AccessAll);
        view_set_buffer(app, &view, decls_buffer.buffer_id, 0);
        
        lock_jump_buffer(search_name.str, search_name.size);
        
        end_temp_memory(temp);
    }
}

CUSTOM_COMMAND_SIG(list_all_functions_current_buffer){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    list_all_functions(app, &global_part, &buffer);
}

#endif

// BOTTOM

