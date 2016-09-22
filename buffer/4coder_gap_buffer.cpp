/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 23.10.2015
 *
 * An implementation of a gap buffer.
 *
 */

// TOP

typedef struct Gap_Buffer{
    char *data;
    i32 size1;
    i32 gap_size;
    i32 size2;
    i32 max;
    
    i32 *line_starts;
    i32 line_count;
    i32 line_max;
    
    f32 *wraps;
    i32 wrap_max;
} Gap_Buffer;

inline_4tech i32
buffer_good(Gap_Buffer *buffer){
    i32 good = (buffer->data != 0);
    return(good);
}

inline_4tech i32
buffer_size(Gap_Buffer *buffer){
    i32 size = buffer->size1 + buffer->size2;
    return(size);
}

typedef struct Gap_Buffer_Init{
    Gap_Buffer *buffer;
    char *data;
    i32 size;
} Gap_Buffer_Init;

internal_4tech Gap_Buffer_Init
buffer_begin_init(Gap_Buffer *buffer, char *data, i32 size){
    Gap_Buffer_Init init;
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    return(init);
}

internal_4tech i32
buffer_init_need_more(Gap_Buffer_Init *init){
    i32 result = 1;
    if (init->buffer->data) result = 0;
    return(result);
}

internal_4tech i32
buffer_init_page_size(Gap_Buffer_Init *init){
    i32 result = init->size * 2;
    return(result);
}

internal_4tech void
buffer_init_provide_page(Gap_Buffer_Init *init, void *page, i32 page_size){
    Gap_Buffer *buffer = init->buffer;
    buffer->data = (char*)page;
    buffer->max = page_size;
}

internal_4tech i32
buffer_end_init(Gap_Buffer_Init *init, void *scratch, i32 scratch_size){
    Gap_Buffer *buffer = init->buffer;
    i32 osize1 = 0, size1 = 0, size2 = 0, size = init->size;
    i32 result = 0;
    
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
            
            buffer->wraps = 0;
            
            result = 1;
        }
    }
    
    return(result);
}

typedef struct Gap_Buffer_Stringify_Loop{
    Gap_Buffer *buffer;
    char *data, *base;
    i32 absolute_pos;
    i32 pos, end;
    i32 size;
    i32 separated;
} Gap_Buffer_Stringify_Loop;

internal_4tech Gap_Buffer_Stringify_Loop
buffer_stringify_loop(Gap_Buffer *buffer, i32 start, i32 end){
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

inline_4tech i32
buffer_stringify_good(Gap_Buffer_Stringify_Loop *loop){
    i32 result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_stringify_next(Gap_Buffer_Stringify_Loop *loop){
    i32 size1 = 0, temp_end = 0;
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

internal_4tech i32
buffer_replace_range(Gap_Buffer *buffer, i32 start, i32 end, char *str, i32 len, i32 *shift_amount,
                     void *scratch, i32 scratch_memory, i32 *request_amount){
    char *data = buffer->data;
    i32 size = buffer_size(buffer);
    i32 result = 0;
    i32 move_size = 0;
    
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
internal_4tech i32
buffer_batch_edit_step(Buffer_Batch_State *state, Gap_Buffer *buffer, Buffer_Edit *sorted_edits,
                       char *strings, i32 edit_count, void *scratch, i32 scratch_size, i32 *request_amount){
    Buffer_Edit *edit = 0;
    i32 i = state->i;
    i32 shift_total = state->shift_total;
    i32 shift_amount = 0;
    i32 result = 0;
    
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
buffer_edit_provide_memory(Gap_Buffer *buffer, void *new_data, i32 new_max){
    void *result = buffer->data;
    i32 size = buffer_size(buffer);
    i32 new_gap_size = new_max - size;
    
    assert_4tech(new_max >= size);
    
    memcpy_4tech(new_data, buffer->data, buffer->size1);
    memcpy_4tech((char*)new_data + buffer->size1 + new_gap_size, buffer->data + buffer->size1 + buffer->gap_size, buffer->size2);
    
    buffer->data = (char*)new_data;
    buffer->gap_size = new_gap_size;
    buffer->max = new_max;
    
    return(result);
}

// BOTTOM

