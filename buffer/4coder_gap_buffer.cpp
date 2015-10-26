/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 23.10.2015
 * 
 * Buffer data object
 *  type - Gap Buffer
 * 
 */

// TOP

typedef struct{
    char *data;
    int size1, gap_size, size2, max;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int line_max;
    int widths_max;
} Gap_Buffer;

inline_4tech void
gap_buffer_initialize(Gap_Buffer *buffer, char *data, int size){
    int size1, size2, gap_size;
    
    assert_4tech(buffer->max >= size);
    
    size1 = size >> 1;
    size2 = size - size1;
    gap_size = buffer->max - size1 - size2;
    buffer->size1 = size1;
    buffer->size2 = size2;
    buffer->gap_size = gap_size;
    
    if (buffer->size1 > 0){
        memcpy_4tech(buffer->data, data, size1);
    }
    
    if (buffer->size2 > 0){
        memcpy_4tech(buffer->data + size1 + gap_size, data + size1, size2);
    }
}

inline_4tech int
buffer_size(Gap_Buffer *buffer){
    int size;
    size = buffer->size1 + buffer->size2;
    return(size);
}

typedef struct{
    Gap_Buffer *buffer;
    char *data, *base;
    int absolute_pos;
    int pos, end;
    int size;
    int page_size;
    int separated;
} Gap_Buffer_Stringify_Loop;

inline_4tech Gap_Buffer_Stringify_Loop
buffer_stringify_loop(Gap_Buffer *buffer, int start, int end, int page_size){
    Gap_Buffer_Stringify_Loop result;
    if (0 <= start && start < end && end <= buffer->size1 + buffer->size2){
        result.buffer = buffer;
        result.base = buffer->data;
        result.page_size = page_size;
        result.absolute_pos = start;
        
        if (end <= buffer->size1) result.end = end;
        else result.end = end + buffer->gap_size;
        
        if (start < buffer->size1){
            if (end <= buffer->size1) result.separated = 0;
            else result.separated = 1;
            result.pos = start;
        }
        else{
            result.separated = 0;
            result.pos = start + buffer->gap_size;
        }
        if (result.separated) result.size = buffer->size1 - start;
        else result.size = end - start;
        if (result.size > page_size) result.size = page_size;
        result.data = buffer->data + result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_stringify_good(Gap_Buffer_Stringify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_stringify_next(Gap_Buffer_Stringify_Loop *loop){
    int size1, temp_end;
    if (loop->separated){
        size1 = loop->buffer->size1;
        if (loop->pos + loop->size == size1){
            loop->separated = 0;
            loop->pos = loop->buffer->gap_size + size1;
            loop->absolute_pos = size1;
            temp_end = loop->end;
        }
        else{
            loop->pos += loop->page_size;
            loop->absolute_pos += loop->page_size;
            temp_end = size1;
        }
    }
    else{
        if (loop->pos + loop->size == loop->end){
            loop->buffer = 0;
            temp_end = loop->pos;
        }
        else{
            loop->pos += loop->page_size;
            loop->absolute_pos += loop->page_size;
            temp_end = loop->end;
        }
    }
    loop->size = temp_end - loop->pos;
    if (loop->size > loop->page_size) loop->size = loop->page_size;
    loop->data = loop->base + loop->pos;
}

typedef struct{
    Gap_Buffer *buffer;
    char *data, *base;
    int pos, end;
    int size;
    int absolute_pos;
    int page_size;
    int separated;
} Gap_Buffer_Backify_Loop;

inline_4tech Gap_Buffer_Backify_Loop
buffer_backify_loop(Gap_Buffer *buffer, int start, int end, int page_size){
    Gap_Buffer_Backify_Loop result;
    int chunk2_start;
    
    if (0 <= end && end < start && start <= buffer->size1 + buffer->size2){
        chunk2_start = buffer->size1 + buffer->gap_size;
        
        result.buffer = buffer;
        result.base = buffer->data;
        result.page_size = page_size;
        
        if (end < buffer->size1) result.end = end;
        else result.end = end + buffer->gap_size;
        
        if (start <= buffer->size1){
            result.separated = 0;
            result.pos = start - page_size;
        }
        else{
            if (end < buffer->size1) result.separated = 1;
            else result.separated = 0;
            result.pos = start - page_size + buffer->gap_size;
        }
        if (result.separated){
            if (result.pos < chunk2_start) result.pos = chunk2_start;
        }
        else{
            if (result.pos < result.end) result.pos = result.end;
        }
        result.size = start - result.pos;
        result.absolute_pos = result.pos;
        if (result.absolute_pos > buffer->size1) result.absolute_pos -= buffer->gap_size;
        result.data = result.base + result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_backify_good(Gap_Buffer_Backify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_backify_next(Gap_Buffer_Backify_Loop *loop){
    Gap_Buffer *buffer;
    int temp_end;
    int chunk2_start;
    buffer = loop->buffer;
    chunk2_start = buffer->size1 + buffer->gap_size;
    if (loop->separated){
        if (loop->pos == chunk2_start){
            loop->separated = 0;
            temp_end = buffer->size1;
            loop->pos = temp_end - loop->page_size;
            loop->absolute_pos = loop->pos;
            if (loop->pos < loop->end){
                loop->absolute_pos += (loop->end - loop->pos);
                loop->pos = loop->end;
            }
        }
        else{
            temp_end = loop->pos;
            loop->pos -= loop->page_size;
            loop->absolute_pos -= loop->page_size;
            if (loop->pos < chunk2_start){
                loop->pos = chunk2_start;
                loop->absolute_pos = buffer->size1;
            }
        }
    }
    else{
        if (loop->pos == loop->end){
            temp_end = 0;
            loop->buffer = 0;
        }
        else{
            temp_end = loop->pos;
            loop->pos -= loop->page_size;
            loop->absolute_pos -= loop->page_size;
            if (loop->pos < loop->end){
                loop->absolute_pos += (loop->end - loop->pos);
                loop->pos = loop->end;
            }
        }
    }
    loop->size = temp_end - loop->pos;
    loop->data = loop->base + loop->pos;
}

inline_4tech void
buffer_stringify(Gap_Buffer *buffer, int start, int end, char *out){
    for (Gap_Buffer_Stringify_Loop loop = buffer_stringify_loop(buffer, start, end, end - start);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        memcpy_4tech(out, loop.data, loop.size);
        out += loop.size;
    }
}

internal_4tech Full_Cursor
buffer_cursor_seek(Gap_Buffer *buffer, Buffer_Seek seek, float max_width, float font_height,
                   void *advance_data, int stride, Full_Cursor cursor){
    char *data;
    int size1, size2, gap_size;
    int total_size, end, i;
    int step;
    int result;
    
    Seek_State state;
    char *advances;
    int xy_seek;
    char ch;
    
    data = buffer->data;
    size1 = buffer->size1;
    gap_size = buffer->gap_size;
    size2 = buffer->size2;
    total_size = size1 + gap_size + size2;
    
    advances = (char*)advance_data;
    xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
    state.cursor = cursor;
    
    result = 1;
    i = cursor.pos;
    end = size1;
    for (step = 0; step < 2; ++step){
        for (; i < end && result; ++i){
            ch = data[i];
            result = cursor_seek_step(&state, seek, xy_seek, max_width, font_height,
                                      advances, stride, size1 + size2, ch);
        }
        end = total_size;
        i += gap_size;
    }
    if (result){
        result = cursor_seek_step(&state, seek, xy_seek, max_width, font_height,
                                  advances, stride, size1 + size2, 0);
        assert_4tech(result == 0);
    }
    
    return(state.cursor);
}

// BOTTOM

