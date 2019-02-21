/*
4coder_seek.cpp - Procedures and commands for jumping through code to useful stop boundaries.
*/

// TOP

static int32_t
seek_line_end(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    
    int32_t still_looping;
    char at_pos;
    
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        still_looping = 1;
        do{
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        
        if (pos > buffer->size){
            pos = buffer->size;
        }
    }
    
    return(pos);
}

static int32_t
seek_line_beginning(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    
    int32_t still_looping;
    char at_pos;
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        still_looping = 1;
        do{
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
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
move_past_lead_whitespace(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    refresh_view(app, view);
    
    int32_t new_pos = seek_line_beginning(app, buffer, view->cursor.pos);
    char space[1024];
    Stream_Chunk chunk = {};
    int32_t still_looping = false;
    
    int32_t i = new_pos;
    if (init_stream_chunk(&chunk, app, buffer, i, space, sizeof(space))){
        do{
            for (; i < chunk.end; ++i){
                char at_pos = chunk.data[i];
                if (at_pos == '\n' || !char_is_whitespace(at_pos)){
                    goto break2;
                }
            }
            still_looping = forward_stream_chunk(&chunk);
        }while(still_looping);
        break2:;
        
        if (i > view->cursor.pos){
            view_set_cursor(app, view, seek_pos(i), true);
        }
    }
}

static int32_t
buffer_seek_whitespace_up(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    
    char at_pos;
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        // Step 1: Find the first non-whitespace character
        // behind the current position.
        int32_t still_looping = 1;
        while (still_looping){
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (!char_is_whitespace(at_pos)){
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
        int32_t no_hard = false;
        while (still_looping){
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                    }
                }
                else if (!char_is_whitespace(at_pos)){
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

static int32_t
buffer_seek_whitespace_down(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        // step 1: find the first non-whitespace character
        // ahead of the current position.
        bool32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                char at_pos = stream.data[pos];
                if (!char_is_whitespace(at_pos)){
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
        bool32 no_hard = false;
        int32_t prev_endline = -1;
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
                else if (!char_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (prev_endline == -1 || prev_endline+1 >= buffer->size){
            pos = buffer->size;
        }
        else{
            pos = prev_endline+1;
        }
    }
    
    return(pos);
}

static int32_t
buffer_seek_whitespace_right(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
        
        bool32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                if (!char_is_whitespace(stream.data[pos])){
                    goto double_break1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break1:;
        
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                if (char_is_whitespace(stream.data[pos])){
                    goto double_break2;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break2:;
    }
    
    return(pos);
}

static int32_t
buffer_seek_whitespace_left(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    --pos;
    if (pos > 0){
        if (init_stream_chunk(&stream, app, buffer,
                              pos, data_chunk, sizeof(data_chunk))){
            
            bool32 still_looping = 1;
            do{
                for (; pos >= stream.start; --pos){
                    if (!char_is_whitespace(stream.data[pos])){
                        goto double_break1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
            
            still_looping = 1;
            do{
                for (; pos >= stream.start; --pos){
                    if (char_is_whitespace(stream.data[pos])){
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

static int32_t
buffer_seek_alphanumeric_right(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
        
        bool32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                if (char_is_alpha_numeric_true_utf8(stream.data[pos])){
                    goto double_break1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break1:;
        
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                if (!char_is_alpha_numeric_true_utf8(stream.data[pos])){
                    goto double_break2;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break2:;
    }
    
    return(pos);
}

static int32_t
buffer_seek_alphanumeric_left(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    --pos;
    if (pos > 0){
        if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
            bool32 still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    if (char_is_alpha_numeric_true_utf8(stream.data[pos])){
                        goto double_break1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
            
            still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    if (!char_is_alpha_numeric_true_utf8(stream.data[pos])){
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

static int32_t
buffer_seek_alphanumeric_or_underscore_right(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
        bool32 still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                if (char_is_alpha_numeric_utf8(stream.data[pos])){
                    goto double_break1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break1:;
        
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                if (!char_is_alpha_numeric_utf8(stream.data[pos])){
                    goto double_break2;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break2:;
    }
    
    return(pos);
}

static int32_t
buffer_seek_alphanumeric_or_underscore_left(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    --pos;
    if (pos > 0){
        if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
            
            bool32 still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    if (char_is_alpha_numeric_utf8(stream.data[pos])){
                        goto double_break1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while(still_looping);
            double_break1:;
            
            still_looping = true;
            do{
                for (; pos >= stream.start; --pos){
                    if (!char_is_alpha_numeric_utf8(stream.data[pos])){
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

static int32_t
buffer_seek_range_camel_right(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t an_pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    ++pos;
    if (pos < an_pos){
        stream.max_end = an_pos;
        if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
            
            uint8_t c = 0, pc = stream.data[pos];
            ++pos;
            
            bool32 still_looping = false;
            do{
                for (; pos < stream.end; ++pos){
                    c = stream.data[pos];
                    if (char_is_upper(c) && char_is_lower_utf8(pc)){
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

static int32_t
buffer_seek_range_camel_left(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t an_pos){
    char data_chunk[1024];
    Stream_Chunk stream = {};
    
    --pos;
    if (pos > 0){
        stream.min_start = an_pos+1;
        if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
            
            char c = 0, pc = stream.data[pos];
            
            bool32 still_looping = false;
            do{
                for (; pos >= stream.start; --pos){
                    c = stream.data[pos];
                    if (char_is_upper(c) && char_is_lower_utf8(pc)){
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

static int32_t
buffer_seek_alphanumeric_or_camel_right(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    int32_t an_pos = buffer_seek_alphanumeric_right(app, buffer, pos);
    int32_t result = buffer_seek_range_camel_right(app, buffer, pos, an_pos);
    return(result);
}

static int32_t
buffer_seek_alphanumeric_or_camel_left(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    int32_t an_pos = buffer_seek_alphanumeric_left(app, buffer, pos);
    int32_t result = buffer_seek_range_camel_left(app, buffer, pos, an_pos);
    return(result);
}

static int32_t
seek_token_left(Cpp_Token_Array *tokens, int32_t pos){
    int32_t result = 0;
    int32_t token_get_pos = (pos > 0)?(pos - 1):0;
    Cpp_Get_Token_Result get = cpp_get_token(*tokens, token_get_pos);
    if (get.token_index >= 0){
        result = get.token_start;
    }
    return(result);
}

static int32_t
seek_token_right(Cpp_Token_Array *tokens, int32_t pos, int32_t buffer_end){
    int32_t result = 0;
    Cpp_Get_Token_Result get = cpp_get_token(*tokens, pos);
    if (get.in_whitespace){
        get.token_index += 1;
        if (get.token_index < tokens->count){
            Cpp_Token *token = tokens->tokens + get.token_index;
            result = token->start + token->size;
        }
        else{
            result = buffer_end;
        }
    }
    else{
        result = get.token_end;
    }
    return(result);
}

static Cpp_Token_Array
buffer_get_all_tokens(Application_Links *app, Partition *part, Buffer_Summary *buffer){
    Cpp_Token_Array array = {};
    
    if (buffer->exists && buffer->is_lexed){
        array.count = buffer_token_count(app, buffer);
        array.max_count = array.count;
        array.tokens = push_array(part, Cpp_Token, array.count);
        buffer_read_tokens(app, buffer, 0, array.count, array.tokens);
    }
    
    return(array);
}

static int32_t
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer, Partition *part, int32_t start_pos, bool32 seek_forward, Seek_Boundary_Flag flags)/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer through which to seek.)
DOC_PARAM(start_pos, The beginning position of the seek is specified by start_pos measured in absolute position.)
DOC_PARAM(seek_forward, If this parameter is non-zero it indicates that the seek should move foward through the buffer.)
DOC_PARAM(flags, This field specifies the types of boundaries at which the seek should stop.)

DOC_RETURN(This call returns the absolute position where the seek stopped.
If the seek goes below 0 the returned value is -1.
If the seek goes past the end the returned value is the size of the buffer.)

DOC_SEE(Seek_Boundary_Flag)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    int32_t result = 0;
    
    // TODO(allen): reduce duplication?
    Temp_Memory temp = begin_temp_memory(part);
    if (buffer->exists){
        int32_t pos[4];
        int32_t size = buffer->size;
        int32_t new_pos = 0;
        
        if (start_pos < 0){
            start_pos = 0;
        }
        else if (start_pos > size){
            start_pos = size;
        }
        
        if (seek_forward){
            for (int32_t i = 0; i < ArrayCount(pos); ++i){
                pos[i] = size;
            }
            
            if (flags & BoundaryWhitespace){
                pos[0] = buffer_seek_whitespace_right(app, buffer, start_pos);
            }
            
            if (flags & BoundaryToken){
                if (buffer->tokens_are_ready){
                    Cpp_Token_Array array = buffer_get_all_tokens(app, part, buffer);
                    pos[1] = seek_token_right(&array, start_pos, size);
                }
                else{
                    pos[1] = buffer_seek_whitespace_right(app, buffer, start_pos);
                }
            }
            
            if (flags & BoundaryAlphanumeric){
                pos[2] = buffer_seek_alphanumeric_right(app, buffer, start_pos);
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_range_camel_right(app, buffer, start_pos, pos[2]);
                }
            }
            else{
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_alphanumeric_or_camel_right(app, buffer, start_pos);
                }
            }
            
            new_pos = size;
            for (int32_t i = 0; i < ArrayCount(pos); ++i){
                if (pos[i] < new_pos){
                    new_pos = pos[i];
                }
            }
        }
        else{
            for (int32_t i = 0; i < ArrayCount(pos); ++i){
                pos[i] = 0;
            }
            
            if (flags & BoundaryWhitespace){
                pos[0] = buffer_seek_whitespace_left(app, buffer, start_pos);
            }
            
            if (flags & BoundaryToken){
                if (buffer->tokens_are_ready){
                    Cpp_Token_Array array = buffer_get_all_tokens(app, part, buffer);
                    pos[1] = seek_token_left(&array, start_pos);
                }
                else{
                    pos[1] = buffer_seek_whitespace_left(app, buffer, start_pos);
                }
            }
            
            if (flags & BoundaryAlphanumeric){
                pos[2] = buffer_seek_alphanumeric_left(app, buffer, start_pos);
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_range_camel_left(app, buffer, start_pos, pos[2]);
                }
            }
            else{
                if (flags & BoundaryCamelCase){
                    pos[3] = buffer_seek_alphanumeric_or_camel_left(app, buffer, start_pos);
                }
            }
            
            new_pos = 0;
            for (int32_t i = 0; i < ArrayCount(pos); ++i){
                if (pos[i] > new_pos){
                    new_pos = pos[i];
                }
            }
        }
        result = new_pos;
    }
    end_temp_memory(temp);
    
    return(result);
}

////////////////////////////////

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_Summary *buffer, int32_t pos, char delim, int32_t *result){
    if (buffer->exists){
        char chunk[1024];
        int32_t size = sizeof(chunk);
        Stream_Chunk stream = {};
        
        if (init_stream_chunk(&stream, app, buffer, pos, chunk, size)){
            int32_t still_looping = 1;
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
    
    *result = buffer->size;
    
    finished:;
}

static void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_Summary *buffer, int32_t pos, char delim, int32_t *result){
    if (buffer->exists){
        char chunk[1024];
        int32_t size = sizeof(chunk);
        Stream_Chunk stream = {};
        
        if (init_stream_chunk(&stream, app, buffer, pos, chunk, size)){
            int32_t still_looping = 1;
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

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
static void
buffer_seek_string_forward(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t end, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    
    if (buffer->size > end){
        *result = buffer->size;
    }
    else{
        *result = end;
    }
    
    if (size > 0 && size <= sizeof(read_buffer)){
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = str[0];
            
            read_str.size = size;
            
            char chunk[1024];
            Stream_Chunk stream = {};
            stream.max_end = end;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, sizeof(chunk))){
                int32_t still_looping = 1;
                do{
                    for(; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_ss(needle_str, read_str)){
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
            *result = buffer->size;
        }
        else{
            *result = end;
        }
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
static void
buffer_seek_string_backward(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t min, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    
    *result = min-1;
    if (size > 0 && size <= sizeof(read_buffer)){
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = str[0];
            
            read_str.size = size;
            
            char chunk[1024];
            Stream_Chunk stream = {};
            stream.min_start = min;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, sizeof(chunk))){
                int32_t still_looping = 1;
                do{
                    for(; pos >= stream.start; --pos){
                        char at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = backward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
static void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t end, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    stream.max_end = end;
    
    if (buffer->size > end){
        *result = buffer->size;
    }
    else{
        *result = end;
    }
    
    if (size > 0 && size <= sizeof(read_buffer)){
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = char_to_upper(str[0]);
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int32_t still_looping = 1;
                do{
                    for(; pos < stream.end; ++pos){
                        char at_pos = char_to_upper(stream.data[pos]);
                        if (at_pos == first_char){
                            buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_insensitive_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
static void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t min, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {};
    stream.min_start = min;
    
    *result = min-1;
    if (size > 0 && size <= sizeof(read_buffer)){
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = char_to_upper(str[0]);
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int32_t still_looping = 1;
                do{
                    for(; pos >= stream.start; --pos){
                        char at_pos = char_to_upper(stream.data[pos]);
                        if (at_pos == first_char){
                            buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_insensitive_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = backward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        finished:;
    }
}

////////////////////////////////

static void
buffer_seek_string(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t end, int32_t min, char *str, int32_t size, int32_t *result, Buffer_Seek_String_Flags flags){
    switch (flags & 3){
        case 0:
        {
            buffer_seek_string_forward(app, buffer, pos, end, str, size, result);
        }break;
        
        case BufferSeekString_Backward:
        {
            buffer_seek_string_backward(app, buffer, pos, min, str, size, result);
        }break;
        
        case BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_forward(app, buffer, pos, end, str, size, result);
        }break;
        
        case BufferSeekString_Backward|BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_backward(app, buffer, pos, min, str, size, result);
        }break;
    }
}

////////////////////////////////

static bool32
buffer_line_is_blank(Application_Links *app, Buffer_Summary *buffer, int32_t line){
    Partial_Cursor start = {};
    Partial_Cursor end = {};
    bool32 result = false;
    if (line <= buffer->line_count){
        buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &start);
        buffer_compute_cursor(app, buffer, seek_line_char(line, -1), &end);
        
        static const int32_t chunk_size = 1024;
        char chunk[chunk_size];
        Stream_Chunk stream = {};
        int32_t i = start.pos;
        stream.max_end = end.pos;
        
        result = true;
        if (init_stream_chunk(&stream, app, buffer, i, chunk, chunk_size)){
            bool32 still_looping = false;
            do{
                for (;i < stream.end; ++i){
                    char c = stream.data[i];
                    if (!char_is_whitespace(c)){
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

static String
read_identifier_at_pos(Application_Links *app, Buffer_Summary *buffer, int32_t pos, char *space, int32_t max, Range *range_out){
    String query = {};
    
    int32_t start = buffer_seek_alphanumeric_or_underscore_left(app, buffer, pos);
    int32_t end = buffer_seek_alphanumeric_or_underscore_right(app, buffer, start);
    
    if (!(start <= pos && pos < end)){
        end = buffer_seek_alphanumeric_or_underscore_right(app, buffer, pos);
        start = buffer_seek_alphanumeric_or_underscore_left(app, buffer, end);
    }
    
    if (start <= pos && pos < end){
        int32_t size = end - start;
        if (size <= max){
            if (range_out != 0){
                *range_out = make_range(start, end);
            }
            buffer_read_range(app, buffer, start, end, space);
            query = make_string_cap(space, size, max);
        }
    }
    
    return(query);
}

////////////////////////////////

static int32_t
flip_dir(int32_t dir){
    if (dir == DirLeft){
        return(DirRight);
    }
    else{
        return(DirLeft);
    }
}

static int32_t
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer,
                     int32_t start_pos, int32_t dir, Seek_Boundary_Flag flags){
    bool32 forward = (dir == DirRight);
    return(buffer_boundary_seek(app, buffer, &global_part, start_pos, forward, flags));
}

static void
view_buffer_boundary_seek_set_pos(Application_Links *app, View_Summary *view, Buffer_Summary *buffer,
                                  int32_t dir, uint32_t flags){
    int32_t pos = buffer_boundary_seek(app, buffer, &global_part, view->cursor.pos, dir, flags);
    view_set_cursor(app, view, seek_pos(pos), true);
    no_mark_snap_to_cursor_if_shift(app, view->view_id);
}

static void
view_boundary_seek_set_pos(Application_Links *app, View_Summary *view,
                           int32_t dir, uint32_t flags){
    Buffer_Summary buffer = get_buffer(app, view->buffer_id, AccessProtected);
    view_buffer_boundary_seek_set_pos(app, view, &buffer, dir, flags);
}

static void
current_view_boundary_seek_set_pos(Application_Links *app, int32_t dir, uint32_t flags){
    View_Summary view = get_active_view(app, AccessProtected);
    view_boundary_seek_set_pos(app, &view, dir, flags);
}

static Range
view_buffer_boundary_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer,
                           int32_t dir, uint32_t flags){
    int32_t pos1 = view->cursor.pos;
    int32_t pos2 = buffer_boundary_seek(app, buffer, pos1, dir, flags);
    return(make_range(pos1, pos2));
}

static Range
view_buffer_snipe_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer,
                        int32_t dir, uint32_t flags){
    int32_t pos0 = view->cursor.pos;
    int32_t pos1 = buffer_boundary_seek(app, buffer, pos0, dir          , flags);
    int32_t pos2 = buffer_boundary_seek(app, buffer, pos1, flip_dir(dir), flags);
    if (dir == DirLeft){
        if (pos2 < pos0){
            pos2 = pos0;
        }
    }
    else{
        if (pos2 > pos0){
            pos2 = pos0;
        }
    }
    return(make_range(pos1, pos2));
}

static void
current_view_boundary_delete(Application_Links *app, int32_t dir, uint32_t flags){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (buffer.exists){
        Range range = view_buffer_boundary_range(app, &view, &buffer, dir, flags);
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    }
}

static void
current_view_snipe_delete(Application_Links *app, int32_t dir, uint32_t flags){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (buffer.exists){
        Range range = view_buffer_snipe_range(app, &view, &buffer, dir, flags);
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(seek_whitespace_up)
CUSTOM_DOC("Seeks the cursor up to the next blank line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t new_pos = buffer_seek_whitespace_up(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down)
CUSTOM_DOC("Seeks the cursor down to the next blank line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t new_pos = buffer_seek_whitespace_down(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the line across all text.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t new_pos = seek_line_beginning(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_end_of_textual_line)
CUSTOM_DOC("Seeks the cursor to the end of the line across all text.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t new_pos = seek_line_end(app, &buffer, view.cursor.pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line)
CUSTOM_DOC("Seeks the cursor to the beginning of the visual line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    view_set_cursor(app, &view, seek_xy(0, y, 1, view.unwrapped_lines), 1);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_end_of_line)
CUSTOM_DOC("Seeks the cursor to the end of the visual line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    view_set_cursor(app, &view, seek_xy(max_f32, y, 1, view.unwrapped_lines), 1);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up_end_line)
CUSTOM_DOC("Seeks the cursor up to the next blank line and places it at the end of the line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t new_pos = buffer_seek_whitespace_up(app, &buffer, view.cursor.pos);
    new_pos = seek_line_end(app, &buffer, new_pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down_end_line)
CUSTOM_DOC("Seeks the cursor down to the next blank line and places it at the end of the line.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t new_pos = buffer_seek_whitespace_down(app, &buffer, view.cursor.pos);
    new_pos = seek_line_end(app, &buffer, new_pos);
    view_set_cursor(app, &view, seek_pos(new_pos), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(goto_beginning_of_file)
CUSTOM_DOC("Sets the cursor to the beginning of the file.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    view_set_cursor(app, &view, seek_pos(0), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
}

CUSTOM_COMMAND_SIG(goto_end_of_file)
CUSTOM_DOC("Sets the cursor to the end of the file.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    view_set_cursor(app, &view, seek_pos(buffer.size), true);
    no_mark_snap_to_cursor_if_shift(app, view.view_id);
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


