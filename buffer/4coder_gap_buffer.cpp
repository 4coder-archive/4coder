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

typedef struct Gap_Buffer{
    char *data;
    int size1;
    int gap_size;
    int size2;
    int max;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int widths_count;
    int line_max;
    int widths_max;
} Gap_Buffer;

inline_4tech int
buffer_good(Gap_Buffer *buffer){
    int good = (buffer->data != 0);
    return(good);
}

inline_4tech int
buffer_size(Gap_Buffer *buffer){
    int size = buffer->size1 + buffer->size2;
    return(size);
}

typedef struct Gap_Buffer_Init{
    Gap_Buffer *buffer;
    char *data;
    int size;
} Gap_Buffer_Init;

internal_4tech Gap_Buffer_Init
buffer_begin_init(Gap_Buffer *buffer, char *data, int size){
    Gap_Buffer_Init init;
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    return(init);
}

internal_4tech int
buffer_init_need_more(Gap_Buffer_Init *init){
    int result = 1;
    if (init->buffer->data) result = 0;
    return(result);
}

internal_4tech int
buffer_init_page_size(Gap_Buffer_Init *init){
    int result = init->size * 2;
    return(result);
}

internal_4tech void
buffer_init_provide_page(Gap_Buffer_Init *init, void *page, int page_size){
    Gap_Buffer *buffer = init->buffer;
    buffer->data = (char*)page;
    buffer->max = page_size;
}

internal_4tech int
buffer_end_init(Gap_Buffer_Init *init, void *scratch, int scratch_size){
    Gap_Buffer *buffer = init->buffer;
    int osize1 = 0, size1 = 0, size2 = 0, size = init->size;
    int result = 0;
    
    if (buffer->data){
        if (buffer->max >= init->size){
            size2 = size >> 1;
            size1 = osize1 = size - size2;
            
            if (size1 > 0){
                size1 = eol_convert_in(buffer->data, init->data, size1);
                if (size2 > 0){
                    size2 = eol_convert_in(buffer->data + size1, init->data + osize1, size2);
                }
            }
            
            buffer->size1 = size1;
            buffer->size2 = size2;
            buffer->gap_size = buffer->max - size1 - size2;
            memmove_4tech(buffer->data + size1 + buffer->gap_size, buffer->data + size1, size2);
            
            result = 1;
        }
    }
    
    return(result);
}

typedef struct Gap_Buffer_Stringify_Loop{
    Gap_Buffer *buffer;
    char *data, *base;
    int absolute_pos;
    int pos, end;
    int size;
    int separated;
} Gap_Buffer_Stringify_Loop;

internal_4tech Gap_Buffer_Stringify_Loop
buffer_stringify_loop(Gap_Buffer *buffer, int start, int end){
    Gap_Buffer_Stringify_Loop result = {0};
    
    if (0 <= start && start < end && end <= buffer->size1 + buffer->size2){
        result.buffer = buffer;
        result.base = buffer->data;
        result.absolute_pos = start;
        
        if (end <= buffer->size1){
            result.end = end;
        }
        else{
            result.end = end + buffer->gap_size;
        }
        
        if (start < buffer->size1){
            if (end <= buffer->size1){
                result.separated = 0;
            }
            else{
                result.separated = 1;
            }
            result.pos = start;
        }
        else{
            result.separated = 0;
            result.pos = start + buffer->gap_size;
        }
        
        if (result.separated){
            result.size = buffer->size1 - start;
        }
        else{
            result.size = end - start;
        }
        
        result.data = buffer->data + result.pos;
    }
    
    return(result);
}

inline_4tech int
buffer_stringify_good(Gap_Buffer_Stringify_Loop *loop){
    int result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_stringify_next(Gap_Buffer_Stringify_Loop *loop){
    int size1 = 0, temp_end = 0;
    if (loop->separated){
        loop->separated = 0;
        size1 = loop->buffer->size1;
        loop->pos = loop->buffer->gap_size + size1;
        loop->absolute_pos = size1;
        temp_end = loop->end;
    }
    else{
        loop->buffer = 0;
        temp_end = loop->pos;
    }
    loop->size = temp_end - loop->pos;
    loop->data = loop->base + loop->pos;
}

typedef struct Gap_Buffer_Backify_Loop{
    Gap_Buffer *buffer;
    char *data, *base;
    int pos, end;
    int size;
    int absolute_pos;
    int separated;
} Gap_Buffer_Backify_Loop;

internal_4tech Gap_Buffer_Backify_Loop
buffer_backify_loop(Gap_Buffer *buffer, int start, int end){
    Gap_Buffer_Backify_Loop result = {0};
    
    ++start;
    if (0 <= end && end < start && start <= buffer->size1 + buffer->size2){
        result.buffer = buffer;
        result.base = buffer->data;
        
        if (end < buffer->size1){
            result.end = end;
        }
        else{
            result.end = end + buffer->gap_size;
        }
        
        if (start <= buffer->size1){
            result.separated = 0;
            result.pos = 0;
        }
        else{
            if (end < buffer->size1){
                result.separated = 1;
            }
            else{
                result.separated = 0;
            }
            result.pos = buffer->size1 + buffer->gap_size;
        }
        
        if (!result.separated && result.pos < result.end){
            result.pos = result.end;
        }
        
        result.size = start - result.pos;
        result.absolute_pos = result.pos;
        if (result.absolute_pos > buffer->size1){
            result.absolute_pos -= buffer->gap_size;
        }
        result.data = result.base + result.pos;
    }
    
    return(result);
}

inline_4tech int
buffer_backify_good(Gap_Buffer_Backify_Loop *loop){
    int result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_backify_next(Gap_Buffer_Backify_Loop *loop){
    Gap_Buffer *buffer = loop->buffer;
    int temp_end = 0;
    
    if (loop->separated){
        loop->separated = 0;
        temp_end = buffer->size1;
        loop->pos = 0;
        loop->absolute_pos = 0;
        if (loop->pos < loop->end){
            loop->absolute_pos = loop->end;
            loop->pos = loop->end;
        }
    }
    else{
        temp_end = 0;
        loop->buffer = 0;
    }
    
    loop->size = temp_end - loop->pos;
    loop->data = loop->base + loop->pos;
}

internal_4tech int
buffer_replace_range(Gap_Buffer *buffer, int start, int end, char *str, int len, int *shift_amount,
                     void *scratch, int scratch_memory, int *request_amount){
    char *data = buffer->data;
    int size = buffer_size(buffer);
    int result = 0;
    int move_size = 0;
    
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end <= size);
    
    *shift_amount = (len - (end - start));
    if (*shift_amount + size <= buffer->max){
        if (end < buffer->size1){
            move_size = buffer->size1 - end;
            memmove_4tech(data + buffer->size1 + buffer->gap_size - move_size, data + end, move_size);
            buffer->size1 -= move_size;
            buffer->size2 += move_size;
        }
        if (start > buffer->size1){
            move_size = start - buffer->size1;
            memmove_4tech(data + buffer->size1, data + buffer->size1 + buffer->gap_size, move_size);
            buffer->size1 += move_size;
            buffer->size2 -= move_size;
        }
        
        memcpy_4tech(data + start, str, len);
        buffer->size2 = size - end;
        buffer->size1 = start + len;
        buffer->gap_size -= *shift_amount;
        
        assert_4tech(buffer->size1 + buffer->size2 == size + *shift_amount);
        assert_4tech(buffer->size1 + buffer->gap_size + buffer->size2 == buffer->max);
    }
    else{
        *request_amount = round_up_4tech(2*(*shift_amount + size), 4 << 10);
        result = 1;
    }

    return(result);
}

// NOTE(allen): This could should be optimized for Gap_Buffer
internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Gap_Buffer *buffer, Buffer_Edit *sorted_edits,
                       char *strings, int edit_count, void *scratch, int scratch_size, int *request_amount){
    Buffer_Edit *edit = 0;
    int i = state->i;
    int shift_total = state->shift_total;
    int shift_amount = 0;
    int result = 0;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      strings + edit->str_start, edit->len, &shift_amount,
                                      scratch, scratch_size, request_amount);
        if (result) break;
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

internal_4tech void*
buffer_edit_provide_memory(Gap_Buffer *buffer, void *new_data, int new_max){
    void *result = buffer->data;
    int size = buffer_size(buffer);
    int new_gap_size = new_max - size;
    
    assert_4tech(new_max >= size);
    
    memcpy_4tech(new_data, buffer->data, buffer->size1);
    memcpy_4tech((char*)new_data + buffer->size1 + new_gap_size, buffer->data + buffer->size1 + buffer->gap_size, buffer->size2);
    
    buffer->data = (char*)new_data;
    buffer->gap_size = new_gap_size;
    buffer->max = new_max;
    
    return(result);
}

// BOTTOM

