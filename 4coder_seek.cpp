/*
4coder_seek.cpp - Procedures and commands for jumping through code to useful stop boundaries.
*/

// TOP

static i32
seek_line_end(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    
    char chunk[1024];
    i32 chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, chunk_size)){
        b32  still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                char at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        if (pos > buffer_size){
            pos = buffer_size;
        }
    }
    return(pos);
}

static i32
seek_line_beginning(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    char chunk[1024];
    i32 chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    --pos;
    if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, chunk_size)){
        b32 still_looping = true;
        do{
            for (; pos >= stream.start; --pos){
                char at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        if (pos != 0){
            ++pos;
        }
        if (pos < 0){
            pos = 0;
        }
    }
    return(pos);
}

static void
move_past_lead_whitespace(Application_Links *app, View_ID view, Buffer_ID buffer_id){
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    
    i32 new_pos = seek_line_beginning(app, buffer_id, pos);
    char space[1024];
    Stream_Chunk chunk = {};
    i32 still_looping = false;
    
    i32 i = new_pos;
    if (init_stream_chunk(&chunk, app, buffer_id, i, space, sizeof(space))){
        do{
            for (; i < chunk.end; ++i){
                char at_pos = chunk.data[i];
                if (at_pos == '\n' || !character_is_whitespace(at_pos)){
                    goto break2;
                }
            }
            still_looping = forward_stream_chunk(&chunk);
        }while(still_looping);
        break2:;
        
        if (i > pos){
            view_set_cursor(app, view, seek_pos(i), true);
        }
    }
}

static i32
buffer_seek_whitespace_up(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    char chunk[1024];
    i32 chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, chunk_size)){
        // Step 1: Find the first non-whitespace character
        // behind the current position.
        b32 still_looping = true;
        for (;still_looping;){
            for (; pos >= stream.start; --pos){
                char at_pos = stream.data[pos];
                if (!character_is_whitespace(at_pos)){
                    goto double_break_1;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }
        double_break_1:;
        
        // Step 2: Continue scanning backward, at each '\n'
        // mark the beginning of another line by setting
        // no_hard to true, set it back to false if a
        // non-whitespace character is discovered before
        // the next '\n'
        i32 no_hard = false;
        for (;still_looping;){
            for (; pos >= stream.start; --pos){
                char at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                    }
                }
                else if (!character_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (pos != 0){
            ++pos;
        }
    }
    
    return(pos);
}

static i32
buffer_seek_whitespace_down(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    
    char chunk[1024];
    i32 chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    
    if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, chunk_size)){
        // step 1: find the first non-whitespace character
        // ahead of the current position.
        b32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                char at_pos = stream.data[pos];
                if (!character_is_whitespace(at_pos)){
                    goto double_break_1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        } while(still_looping);
        double_break_1:;
        
        // step 2: continue scanning forward, at each '\n'
        // mark it as the beginning of a new line by updating
        // the prev_endline value.  if another '\n' is found
        // with non-whitespace then the previous line was
        // all whitespace.
        b32 no_hard = false;
        i32 prev_endline = -1;
        while(still_looping){
            for (; pos < stream.end; ++pos){
                char at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                        prev_endline = pos;
                    }
                }
                else if (!character_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (prev_endline == -1 || prev_endline + 1 >= buffer_size){
            pos = buffer_size;
        }
        else{
            pos = prev_endline + 1;
        }
    }
    
    return(pos);
}

static i32
buffer_seek_whitespace_right(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    i32 result = buffer_size + 1;
    if (pos < 0){
        pos = 0;
    }
    pos += 1;
    if (pos < buffer_size){
        char data_chunk[1024];
        Stream_Chunk stream = {};
        stream.add_null = true;
        if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
            b32 still_looping = true;
            b32 is_whitespace_1 = character_is_whitespace(buffer_get_char(app, buffer_id, pos - 1));
            do{
                for (; pos < stream.end; ++pos){
                    char c2 = stream.data[pos];
                    b32 is_whitespace_2 = true;
                    if (c2 != 0){
                        is_whitespace_2 = character_is_whitespace(c2);
                    }
                    if (!is_whitespace_1 && is_whitespace_2){
                        result = pos;
                        goto double_break;
                    }
                    is_whitespace_1 = is_whitespace_2;
                }
                still_looping = forward_stream_chunk(&stream);
            }while(still_looping);
            double_break:;
        }
    }
    return(result);
}

static i32
buffer_seek_whitespace_left(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    
    i32 result = -1;
    if (pos > buffer_size){
        pos = buffer_size;
    }
    pos -= 2;
    if (pos >= 0){
        char data_chunk[1024];
        Stream_Chunk stream = {};
        if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
            b32 still_looping = true;
            b32 is_whitespace_2 = character_is_whitespace(buffer_get_char(app, buffer_id, pos + 1));
            do{
                for (; pos >= stream.start; --pos){
                    char c1 = stream.data[pos];
                    b32 is_whitespace_1 = character_is_whitespace(c1);
                    if (is_whitespace_1 && !is_whitespace_2){
                        result = pos + 1;
                        goto double_break;
                    }
                    is_whitespace_2 = is_whitespace_1;
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break:;
        }
    }
    if (pos == -1){
        if (!character_is_whitespace(buffer_get_char(app, buffer_id, 0))){
            result = 0;
        }
    }
    return(result);
}

static i32
buffer_seek_alphanumeric_right(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
        b32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                u8 c = stream.data[pos];
                if (c != '_' && character_is_alpha_numeric_unicode(c)){
                    goto double_break1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break1:;
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                u8 c = stream.data[pos];
                if (!(c != '_' && character_is_alpha_numeric_unicode(c))){
                    goto double_break2;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break2:;
    }
    return(pos);
}

static i32
buffer_seek_alphanumeric_left(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    --pos;
    if (pos > 0){
        if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
            b32 still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    u8 c = stream.data[pos];
                    if (c != '_' && character_is_alpha_numeric_unicode(c)){
                        goto double_break1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
            still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    u8 c = stream.data[pos];
                    if (!(c != '_' && character_is_alpha_numeric_unicode(c))){
                        ++pos;
                        goto double_break2;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break2:;
        }
    }
    else{
        pos = 0;
    }
    return(pos);
}

static i32
buffer_seek_alphanumeric_or_underscore_right(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
        b32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                u8 c = stream.data[pos];
                if (character_is_alpha_numeric_unicode(c)){
                    goto double_break1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break1:;
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                u8 c = stream.data[pos];
                if (!character_is_alpha_numeric_unicode(c)){
                    goto double_break2;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break2:;
    }
    return(pos);
}

static i32
buffer_seek_alphanumeric_or_underscore_left(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    --pos;
    if (pos > 0){
        if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
            b32 still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    u8 c = stream.data[pos];
                    if (character_is_alpha_numeric_unicode(c)){
                        goto double_break1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
            still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    u8 c = stream.data[pos];
                    if (!character_is_alpha_numeric_unicode(c)){
                        ++pos;
                        goto double_break2;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break2:;
        }
    }
    else{
        pos = 0;
    }
    return(pos);
}

static i32
buffer_seek_range_camel_right(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 an_pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    ++pos;
    if (pos < an_pos){
        stream.max_end = an_pos;
        if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
            u8 c = 0;
            u8 pc = stream.data[pos];
            ++pos;
            b32 still_looping = false;
            do{
                for (; pos < stream.end; ++pos){
                    c = stream.data[pos];
                    if (character_is_upper(c) && character_is_lower_unicode(pc)){
                        goto double_break1;
                    }
                    pc = c;
                }
                still_looping = forward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
        }
    }
    else{
        pos = an_pos;
    }
    return(pos);
}

static i32
buffer_seek_range_camel_left(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 an_pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    --pos;
    if (pos > 0){
        stream.min_start = an_pos+1;
        if (init_stream_chunk(&stream, app, buffer_id, pos, data_chunk, sizeof(data_chunk))){
            u8 c = 0;
            u8 pc = stream.data[pos];
            b32 still_looping = false;
            do{
                for (; pos >= stream.start; --pos){
                    c = stream.data[pos];
                    if (character_is_upper(c) && character_is_lower_unicode(pc)){
                        goto double_break1;
                    }
                    pc = c;
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
        }
    }
    else{
        pos = 0;
    }
    return(pos);
}

static i32
buffer_seek_alphanumeric_or_camel_right(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 an_pos = buffer_seek_alphanumeric_right(app, buffer_id, pos);
    i32 result = buffer_seek_range_camel_right(app, buffer_id, pos, an_pos);
    return(result);
}

static i32
buffer_seek_alphanumeric_or_camel_left(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 an_pos = buffer_seek_alphanumeric_left(app, buffer_id, pos);
    i32 result = buffer_seek_range_camel_left(app, buffer_id, pos, an_pos);
    return(result);
}

static i32
seek_token_left(Cpp_Token_Array *tokens, i32 pos){
    i32 result = -1;
    i32 token_get_pos = pos - 1;
    Cpp_Get_Token_Result get = cpp_get_token(*tokens, token_get_pos);
    if (get.token_index >= 0){
        result = get.token_start;
    }
    return(result);
}

static i32
seek_token_right(Cpp_Token_Array *tokens, i32 pos, i32 buffer_end){
    i32 result = buffer_end + 1;
    Cpp_Get_Token_Result get = cpp_get_token(*tokens, pos);
    if (get.in_whitespace_after_token){
        get.token_index += 1;
        if (get.token_index < tokens->count){
            Cpp_Token *token = tokens->tokens + get.token_index;
            result = token->start + token->size;
        }
    }
    else{
        result = get.token_one_past_last;
    }
    return(result);
}

static Cpp_Token_Array
buffer_get_all_tokens(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Cpp_Token_Array array = {};
    if (buffer_exists(app, buffer_id)){
        b32 is_lexed = false;
        if (buffer_get_setting(app, buffer_id, BufferSetting_Lex, &is_lexed)){
            if (is_lexed){
                buffer_token_count(app, buffer_id, &array.count);
                array.max_count = array.count;
                array.tokens = push_array(arena, Cpp_Token, array.count);
                buffer_read_tokens(app, buffer_id, 0, array.count, array.tokens);
            }
        }
    }
    return(array);
}

static i32
buffer_boundary_seek(Application_Links *app, Buffer_ID buffer_id, Arena *scratch, i32 start_pos, b32 seek_forward, Seek_Boundary_Flag flags){
    i32 result = 0;
    
    // TODO(allen): reduce duplication?
    Temp_Memory temp = begin_temp(scratch);
    if (buffer_exists(app, buffer_id)){
        i32 pos[4];
        i32 size = 0;
        i32 new_pos = 0;
        buffer_get_size(app, buffer_id, &size);
        
        if (start_pos < 0){
            start_pos = 0;
        }
        else if (start_pos > size){
            start_pos = size;
        }
        
        if (seek_forward){
            for (i32 i = 0; i < ArrayCount(pos); ++i){
                pos[i] = size + 1;
            }
            
            if (flags & BoundaryWhitespace){
                pos[0] = buffer_seek_whitespace_right(app, buffer_id, start_pos);
            }
            
            if (flags & BoundaryToken){
                if (buffer_tokens_are_ready(app, buffer_id)){
                    Cpp_Token_Array array = buffer_get_all_tokens(app, scratch, buffer_id);
                    pos[1] = seek_token_right(&array, start_pos, size);
                }
                else{
                    pos[1] = buffer_seek_whitespace_right(app, buffer_id, start_pos);
                }
            }
            
            if (flags & BoundaryAlphanumeric){
                pos[2] = buffer_seek_alphanumeric_right(app, buffer_id, start_pos);
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_range_camel_right(app, buffer_id, start_pos, pos[2]);
                }
            }
            else{
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_alphanumeric_or_camel_right(app, buffer_id, start_pos);
                }
            }
            
            new_pos = size + 1;
            for (i32 i = 0; i < ArrayCount(pos); ++i){
                if (new_pos > pos[i]){
                    new_pos = pos[i];
                }
            }
        }
        else{
            for (i32 i = 0; i < ArrayCount(pos); ++i){
                pos[i] = -1;
            }
            
            if (flags & BoundaryWhitespace){
                pos[0] = buffer_seek_whitespace_left(app, buffer_id, start_pos);
            }
            
            if (flags & BoundaryToken){
                if (buffer_tokens_are_ready(app, buffer_id)){
                    Cpp_Token_Array array = buffer_get_all_tokens(app, scratch, buffer_id);
                    pos[1] = seek_token_left(&array, start_pos);
                }
                else{
                    pos[1] = buffer_seek_whitespace_left(app, buffer_id, start_pos);
                }
            }
            
            if (flags & BoundaryAlphanumeric){
                pos[2] = buffer_seek_alphanumeric_left(app, buffer_id, start_pos);
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_range_camel_left(app, buffer_id, start_pos, pos[2]);
                }
            }
            else{
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_alphanumeric_or_camel_left(app, buffer_id, start_pos);
                }
            }
            
            new_pos = -1;
            for (i32 i = 0; i < ArrayCount(pos); ++i){
                if (new_pos < pos[i]){
                    new_pos = pos[i];
                }
            }
        }
        result = new_pos;
    }
    end_temp(temp);
    
    return(result);
}

////////////////////////////////

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_ID buffer_id, i32 pos, char delim, i32 *result){
    if (buffer_exists(app, buffer_id)){
        char chunk[1024];
        i32 size = sizeof(chunk);
        Stream_Chunk stream = {};
        if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, size)){
            i32 still_looping = 1;
            do{
                for(; pos < stream.end; ++pos){
                    char at_pos = stream.data[pos];
                    if (at_pos == delim){
                        *result = pos;
                        goto finished;
                    }
                }
                still_looping = forward_stream_chunk(&stream);
            }while (still_looping);
        }
    }
    buffer_get_size(app, buffer_id, result);
    finished:;
}

static void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_ID buffer_id, i32 pos, char delim, i32 *result){
    if (buffer_exists(app, buffer_id)){
        char chunk[1024];
        i32 size = sizeof(chunk);
        Stream_Chunk stream = {};
        if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, size)){
            i32 still_looping = 1;
            do{
                for(; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (at_pos == delim){
                        *result = pos;
                        goto finished;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while (still_looping);
        }
    }
    *result = 0;
    finished:;
}

static void
buffer_seek_string_forward(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 end, String_Const_u8 needle_string, i32 *result){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    if (buffer_size > end){
        *result = buffer_size;
    }
    else{
        *result = end;
    }
    
    Scratch_Block scratch(app);
    if (0 < needle_string.size){
        if (buffer_exists(app, buffer_id)){
            u8 first_char = string_get_character(needle_string, 0);
            
            u8 *read_buffer = push_array(scratch, u8, needle_string.size);
            String_Const_u8 read_str = SCu8(read_buffer, needle_string.size);
            
            char chunk[1024];
            Stream_Chunk stream = {};
            stream.max_end = end;
            
            if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, sizeof(chunk))){
                b32 still_looping = true;
                do{
                    for(; pos < stream.end; ++pos){
                        u8 at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            buffer_read_range(app, buffer_id, pos, (i32)(pos + needle_string.size), (char*)read_buffer);
                            if (string_match(needle_string, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        if (end == 0){
            *result = buffer_size;
        }
        else{
            *result = end;
        }
        
        finished:;
    }
}

static void
buffer_seek_string_backward(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 min, String_Const_u8 needle_string, i32 *result){
    Scratch_Block scratch(app);
    *result = min - 1;
    if (0 < needle_string.size && buffer_exists(app, buffer_id)){
        u8 first_char = string_get_character(needle_string, 0);
        
        u8 *read_buffer = push_array(scratch, u8, needle_string.size);
        String_Const_u8 read_str = SCu8(read_buffer, needle_string.size);
        
        char chunk[1024];
        Stream_Chunk stream = {};
        stream.min_start = min;
        
        if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, sizeof(chunk))){
            b32 still_looping = true;
            do{
                for(; pos >= stream.start; --pos){
                    u8 at_pos = stream.data[pos];
                    if (at_pos == first_char){
                        buffer_read_range(app, buffer_id, pos, (i32)(pos + needle_string.size), (char*)read_buffer);
                        if (string_match(needle_string, read_str)){
                            *result = pos;
                            goto finished;
                        }
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while (still_looping);
        }
        finished:;
    }
}

static void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 end, String_Const_u8 needle_string, i32 *result){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    if (buffer_size > end){
        *result = buffer_size;
    }
    else{
        *result = end;
    }
    
    Scratch_Block scratch(app);
    if (0 < needle_string.size && buffer_exists(app, buffer_id)){
        u8 first_char = character_to_upper(string_get_character(needle_string, 0));
        
        u8 *read_buffer = push_array(scratch, u8, needle_string.size);
        String_Const_u8 read_str = SCu8(read_buffer, needle_string.size);
        
        char chunk[1024];
        Stream_Chunk stream = {};
        stream.max_end = end;
        
        if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, sizeof(chunk))){
            b32 still_looping = true;
            do{
                for(; pos < stream.end; ++pos){
                    u8 at_pos = character_to_upper(stream.data[pos]);
                    if (at_pos == first_char){
                        buffer_read_range(app, buffer_id, pos, (i32)(pos + needle_string.size), (char*)read_buffer);
                        if (string_match_insensitive(needle_string, read_str)){
                            *result = pos;
                            goto finished;
                        }
                    }
                }
                still_looping = forward_stream_chunk(&stream);
            }while (still_looping);
        }
        finished:;
    }
}

static void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 min, String_Const_u8 needle_string, i32 *result){
    Scratch_Block scratch(app);
    *result = min - 1;
    if (0 < needle_string.size && buffer_exists(app, buffer_id)){
        u8 first_char = character_to_upper(string_get_character(needle_string, 0));
        
        u8 *read_buffer = push_array(scratch, u8, needle_string.size);
        String_Const_u8 read_str = SCu8(read_buffer, needle_string.size);
        
        char chunk[1024];
        Stream_Chunk stream = {};
        stream.min_start = min;
        
        if (init_stream_chunk(&stream, app, buffer_id, pos, chunk, sizeof(chunk))){
            b32 still_looping = true;
            do{
                for(; pos >= stream.start; --pos){
                    u8 at_pos = character_to_upper(stream.data[pos]);
                    if (at_pos == first_char){
                        buffer_read_range(app, buffer_id, pos, (i32)(pos + needle_string.size), (char*)read_buffer);
                        if (string_match_insensitive(needle_string, read_str)){
                            *result = pos;
                            goto finished;
                        }
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while (still_looping);
        }
        finished:;
    }
}

////////////////////////////////

static void
buffer_seek_string(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 end, i32 min, String_Const_u8 str, i32 *result, Buffer_Seek_String_Flags flags){
    switch (flags & 3){
        case 0:
        {
            buffer_seek_string_forward(app, buffer_id, pos, end, str, result);
        }break;
        
        case BufferSeekString_Backward:
        {
            buffer_seek_string_backward(app, buffer_id, pos, min, str, result);
        }break;
        
        case BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_forward(app, buffer_id, pos, end, str, result);
        }break;
        
        case BufferSeekString_Backward|BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_backward(app, buffer_id, pos, min, str, result);
        }break;
    }
}

////////////////////////////////

static b32
buffer_line_is_blank(Application_Links *app, Buffer_ID buffer_id, i32 line){
    Partial_Cursor start = {};
    Partial_Cursor end = {};
    b32 result = false;
    i32 line_count = 0;
    buffer_get_line_count(app, buffer_id, &line_count);
    if (line <= line_count){
        buffer_compute_cursor(app, buffer_id, seek_line_char(line,  1), &start);
        buffer_compute_cursor(app, buffer_id, seek_line_char(line, -1), &end);
        
        static const i32 chunk_size = 1024;
        char chunk[chunk_size];
        Stream_Chunk stream = {};
        i32 i = start.pos;
        stream.max_end = end.pos;
        
        result = true;
        if (init_stream_chunk(&stream, app, buffer_id, i, chunk, chunk_size)){
            b32 still_looping = false;
            do{
                for (;i < stream.end; ++i){
                    char c = stream.data[i];
                    if (!character_is_whitespace(c)){
                        result = false;
                        goto double_break;
                    }
                }
                still_looping = forward_stream_chunk(&stream);
            }while(still_looping);
        }
        double_break:;
    }
    return(result);
}

static String_Const_u8
read_identifier_at_pos(Application_Links *app, Arena *arena, Buffer_ID buffer_id, i32 pos, Range *range_out){
    String_Const_u8 result = {};
    
    i32 start = buffer_seek_alphanumeric_or_underscore_left(app, buffer_id, pos);
    i32 end = buffer_seek_alphanumeric_or_underscore_right(app, buffer_id, start);
    
    if (!(start <= pos && pos < end)){
        end = buffer_seek_alphanumeric_or_underscore_right(app, buffer_id, pos);
        start = buffer_seek_alphanumeric_or_underscore_left(app, buffer_id, end);
    }
    
    if (start <= pos && pos < end){
        if (range_out != 0){
            *range_out = make_range(start, end);
        }
        result = scratch_read(app, arena, buffer_id, start, end);
    }
    
    return(result);
}

////////////////////////////////

static i32
flip_dir(i32 dir){
    if (dir == DirLeft){
        dir = DirRight;
    }
    else{
        dir = DirLeft;
    }
    return(dir);
}

static i32
buffer_boundary_seek(Application_Links *app, Buffer_ID buffer_id, i32 start_pos, i32 dir, Seek_Boundary_Flag flags){
    b32 forward = (dir != DirLeft);
    Arena *scratch = context_get_arena(app);
    return(buffer_boundary_seek(app, buffer_id, scratch, start_pos, forward, flags));
}

static void
view_buffer_boundary_seek_set_pos(Application_Links *app, View_ID view, Buffer_ID buffer_id, i32 dir, u32 flags){
    i32 cursor_pos = 0;
    view_get_cursor_pos(app, view, &cursor_pos);
    Arena *scratch = context_get_arena(app);
    i32 pos = buffer_boundary_seek(app, buffer_id, scratch, cursor_pos, dir, flags);
    view_set_cursor(app, view, seek_pos(pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

static void
view_boundary_seek_set_pos(Application_Links *app, View_ID view, i32 dir, u32 flags){
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    view_buffer_boundary_seek_set_pos(app, view, buffer_id, dir, flags);
}

static void
current_view_boundary_seek_set_pos(Application_Links *app, i32 dir, u32 flags){
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    view_boundary_seek_set_pos(app, view, dir, flags);
}

static Range
view_buffer_boundary_range(Application_Links *app, View_ID view, Buffer_ID buffer_id, i32 dir, u32 flags){
    i32 pos1 = 0;
    view_get_cursor_pos(app, view, &pos1);
    i32 pos2 = buffer_boundary_seek(app, buffer_id, pos1, dir, flags);
    return(make_range(pos1, pos2));
}

static Range
view_buffer_snipe_range(Application_Links *app, View_ID view, Buffer_ID buffer_id, i32 dir, u32 flags){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    Range result = {};
    i32 pos0 = 0;
    view_get_cursor_pos(app, view, &pos0);
    i32 pos1 = buffer_boundary_seek(app, buffer_id, pos0, dir, flags);
    if (0 <= pos1 && pos1 <= buffer_size){
        i32 pos2 = buffer_boundary_seek(app, buffer_id, pos1, flip_dir(dir), flags);
        if (0 <= pos2 && pos2 <= buffer_size){
            if (dir == DirLeft){
                pos2 = clamp_bot(pos2, pos0);
            }
            else{
                pos2 = clamp_top(pos2, pos0);
            }
            result = make_range(pos1, pos2);
        }
    }
    return(result);
}

static void
current_view_boundary_delete(Application_Links *app, i32 dir, u32 flags){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessOpen, &buffer_id);
    Range range = view_buffer_boundary_range(app, view, buffer_id, dir, flags);
    buffer_replace_range(app, buffer_id, range, string_u8_litexpr(""));
}

static void
current_view_snipe_delete(Application_Links *app, i32 dir, u32 flags){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessOpen, &buffer_id);
    Range range = view_buffer_snipe_range(app, view, buffer_id, dir, flags);
    buffer_replace_range(app, buffer_id, range, string_u8_litexpr(""));
}

////////////////////////////////

CUSTOM_COMMAND_SIG(seek_whitespace_up)
CUSTOM_DOC("Seeks the cursor up to the next blank line.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = buffer_seek_whitespace_up(app, buffer_id, pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down)
CUSTOM_DOC("Seeks the cursor down to the next blank line.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = buffer_seek_whitespace_down(app, buffer_id, pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the line across all text.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = seek_line_beginning(app, buffer_id, pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_end_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the end of the line across all text.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = seek_line_end(app, buffer_id, pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the visual line.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(pos), &cursor);
    f32 y = cursor.wrapped_y;
    view_set_cursor(app, view, seek_wrapped_xy(0.f, y, true), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_end_of_line)
CUSTOM_DOC("Seeks the cursor to the end of the visual line.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(pos), &cursor);
    f32 y = cursor.wrapped_y;
    view_set_cursor(app, view, seek_wrapped_xy(max_f32, y, true), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up_end_line)
CUSTOM_DOC("Seeks the cursor up to the next blank line and places it at the end of the line.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = buffer_seek_whitespace_up(app, buffer_id, pos);
    new_pos = seek_line_end(app, buffer_id, new_pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down_end_line)
CUSTOM_DOC("Seeks the cursor down to the next blank line and places it at the end of the line.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = buffer_seek_whitespace_down(app, buffer_id, pos);
    new_pos = seek_line_end(app, buffer_id, new_pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(goto_beginning_of_file)
CUSTOM_DOC("Sets the cursor to the beginning of the file.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    view_set_cursor(app, view, seek_pos(0), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

CUSTOM_COMMAND_SIG(goto_end_of_file)
CUSTOM_DOC("Sets the cursor to the end of the file.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view, AccessProtected, &buffer_id);
    i32 size = 0;
    buffer_get_size(app, buffer_id, &size);
    view_set_cursor(app, view, seek_pos(size), true);
    no_mark_snap_to_cursor_if_shift(app, view);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(seek_whitespace_right)
CUSTOM_DOC("Seek right for the next boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_whitespace_left)
CUSTOM_DOC("Seek left for the next boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_token_right)
CUSTOM_DOC("Seek right for the next end of a token.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryToken);
}

CUSTOM_COMMAND_SIG(seek_token_left)
CUSTOM_DOC("Seek left for the next beginning of a token.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryToken);
}

CUSTOM_COMMAND_SIG(seek_white_or_token_right)
CUSTOM_DOC("Seek right for the next end of a token or boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryToken|BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_white_or_token_left)
CUSTOM_DOC("Seek left for the next end of a token or boundary between whitespace and non-whitespace.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryToken|BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_right)
CUSTOM_DOC("Seek right for boundary between alphanumeric characters and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_left)
CUSTOM_DOC("Seek left for boundary between alphanumeric characters and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_right)
CUSTOM_DOC("Seek right for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirRight, BoundaryAlphanumeric|BoundaryCamelCase);
}

CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_left)
CUSTOM_DOC("Seek left for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.")
{
    current_view_boundary_seek_set_pos(app, DirLeft, BoundaryAlphanumeric|BoundaryCamelCase);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(backspace_word)
CUSTOM_DOC("Delete characters between the cursor position and the first alphanumeric boundary to the left.")
{
    current_view_boundary_delete(app, DirLeft, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(delete_word)
CUSTOM_DOC("Delete characters between the cursor position and the first alphanumeric boundary to the right.")
{
    current_view_boundary_delete(app, DirRight, BoundaryAlphanumeric);
}

CUSTOM_COMMAND_SIG(snipe_token_or_word)
CUSTOM_DOC("Delete a single, whole token on or to the left of the cursor and post it to the clipboard.")
{
    current_view_snipe_delete(app, DirLeft, BoundaryToken|BoundaryWhitespace);
}

CUSTOM_COMMAND_SIG(snipe_token_or_word_right)
CUSTOM_DOC("Delete a single, whole token on or to the right of the cursor and post it to the clipboard.")
{
    current_view_snipe_delete(app, DirRight, BoundaryToken|BoundaryWhitespace);
}

// BOTTOM


