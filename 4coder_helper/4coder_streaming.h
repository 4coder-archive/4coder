/*
 * Helpers for streaming over buffer data to avoid having to read it all at once.
 */

// TOP

#if !defined(FCODER_STREAMING_H)
#define FCODER_STREAMING_H

#include "4coder_helper/4coder_helper.h"

// TODO(allen): Rewrite the backward seek to get this on size_t instead of int64_t.
struct Stream_Chunk{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    char *base_data;
    int64_t start, end;
    int64_t min_start, max_end;
    int64_t data_size;
    bool32 add_null;
    
    char *data;
};

static uint64_t
round_down(uint64_t x, uint64_t b){
    uint64_t r = 0;
    if (x >= 0){
        r = x - (x % b);
    }
    return(r);
}

static uint64_t
round_up(uint64_t x, uint64_t b){
    uint64_t r = 0;
    if (x >= 0){
        r = x - (x % b) + b;
    }
    return(r);
}

static bool32
init_stream_chunk(Stream_Chunk *chunk, Application_Links *app, Buffer_Summary *buffer, size_t pos, char *data, size_t size){
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (pos >= 0 && pos < buffer->size && size > 0){
        chunk->app = app;
        chunk->buffer = buffer;
        chunk->base_data = data;
        chunk->data_size = size;
        chunk->start = round_down(pos, size);
        chunk->end = round_up(pos, size);
        
        if (chunk->max_end > buffer->size || chunk->max_end == 0){
            chunk->max_end = buffer->size;
        }
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = 1;
        }
    }
    
    return(result);
}

static bool32
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (chunk->end < buffer->size){
        chunk->start = chunk->end;
        chunk->end += chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = 1;
        }
    }
    
    else if (chunk->add_null && chunk->end + 1 < buffer->size){
        chunk->start = buffer->size;
        chunk->end = buffer->size + 1;
        chunk->base_data[0] = 0;
        chunk->data = chunk->base_data - chunk->start;
        result = 1;
    }
    
    return(result);
}

static bool32
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (chunk->start > 0){
        chunk->end = chunk->start;
        chunk->start -= chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = 1;
        }
    }
    
    else if (chunk->add_null && chunk->start > -1){
        chunk->start = -1;
        chunk->end = 0;
        chunk->base_data[0] = 0;
        chunk->data = chunk->base_data - chunk->start;
        result = 1;
    }
    
    return(result);
}

struct Stream_Tokens{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    Cpp_Token *base_tokens;
    Cpp_Token *tokens;
    int64_t start, end;
    int64_t count, token_count;
};

static bool32
init_stream_tokens(Stream_Tokens *stream, Application_Links *app, Buffer_Summary *buffer, size_t pos, Cpp_Token *data, size_t count){
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    
    int32_t token_count = buffer_token_count(app, buffer);
    if (buffer->tokens_are_ready && pos >= 0 && pos < token_count && count > 0){
        stream->app = app;
        stream->buffer = buffer;
        stream->base_tokens = data;
        stream->count = (int64_t)count;
        stream->start = (int64_t)round_down(pos, count);
        stream->end = (int64_t)round_up(pos, count);
        stream->token_count = token_count;
        
        if (stream->start < 0){
            stream->start = 0;
        }
        if (stream->end > stream->token_count){
            stream->end = stream->token_count;
        }
        
        buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
        stream->tokens = stream->base_tokens - stream->start;
        result = 1;
    }
    
    return(result);
}

static Stream_Tokens
begin_temp_stream_token(Stream_Tokens *stream){
    return(*stream);
}

static void
end_temp_stream_token(Stream_Tokens *stream, Stream_Tokens temp){
    if (stream->start != temp.start || stream->end != temp.end){
        Application_Links *app = stream->app;
        buffer_read_tokens(app, temp.buffer, temp.start, temp.end, temp.base_tokens);
        stream->tokens = stream->base_tokens - temp.start;
        stream->start = temp.start;
        stream->end = temp.end;
    }
}

static bool32
forward_stream_tokens(Stream_Tokens *stream){
    Application_Links *app = stream->app;
    Buffer_Summary *buffer = stream->buffer;
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (stream->end < stream->token_count){
        stream->start = stream->end;
        stream->end += stream->count;
        
        if (stream->end > stream->token_count){
            stream->end = stream->token_count;
        }
        
        if (stream->start < stream->end){
            buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = 1;
        }
    }
    
    return(result);
}

static bool32
backward_stream_tokens(Stream_Tokens *stream){
    Application_Links *app = stream->app;
    Buffer_Summary *buffer = stream->buffer;
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (stream->start > 0){
        stream->end = stream->start;
        stream->start -= stream->count;
        
        if (0 > stream->start){
            stream->start = 0;
        }
        
        if (stream->start < stream->end){
            buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = 1;
        }
    }
    
    return(result);
}

#endif

// BOTTOM
