/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 16.10.2015
 * 
 * Buffer data object
 *  type - Golden Array
 * 
 */

// TOP

typedef struct{
    char *data;
    int size, max;
    
    int *line_starts;
    float *line_widths;
    int line_count;
    int widths_count;
    int line_max;
    int widths_max;
} Buffer;

inline_4tech int
buffer_good(Buffer *buffer){
    int good;
    good = (buffer->data != 0);
    return(good);
}

inline_4tech int
buffer_size(Buffer *buffer){
    return buffer->size;
}

typedef struct{
    Buffer *buffer;
    char *data;
    int size;
} Buffer_Init;

internal_4tech Buffer_Init
buffer_begin_init(Buffer *buffer, char *data, int size){
    Buffer_Init init;
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    return(init);
}

inline_4tech int
buffer_init_need_more(Buffer_Init *init){
    int result;
    result = 1;
    if (init->buffer->data) result = 0;
    return(result);
}

inline_4tech int
buffer_init_page_size(Buffer_Init *init){
    int result;
    result = init->size * 2;
    return(result);
}

inline_4tech void
buffer_init_provide_page(Buffer_Init *init, void *page, int page_size){
    Buffer *buffer;
    buffer = init->buffer;
    buffer->data = (char*)page;
    buffer->max = page_size;
}

internal_4tech int
buffer_end_init(Buffer_Init *init){
    Buffer *buffer;
    int result;

    result = 0;
    buffer = init->buffer;
    if (buffer->data){
        if (buffer->max >= init->size){
            buffer->size = eol_convert_in(buffer->data, init->data, init->size);
            result = 1;
        }
    }
    
    return(result);
}

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int absolute_pos;
    int size;
} Buffer_Stringify_Loop;

inline_4tech Buffer_Stringify_Loop
buffer_stringify_loop(Buffer *buffer, int start, int end){
    Buffer_Stringify_Loop result;
    if (0 <= start && start < end && end <= buffer->size){
        result.buffer = buffer;
        result.absolute_pos = start;
        result.data = buffer->data + start;
        result.size = end - start;
        result.end = buffer->data + end;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_stringify_good(Buffer_Stringify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_stringify_next(Buffer_Stringify_Loop *loop){
    loop->buffer = 0;
}

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int absolute_pos;
    int size;
} Buffer_Backify_Loop;

inline_4tech Buffer_Backify_Loop
buffer_backify_loop(Buffer *buffer, int start, int end){
    Buffer_Backify_Loop result;
    
    ++start;
    if (0 <= end && end < start && start <= buffer->size){
        result.buffer = buffer;
        result.end = buffer->data + end;
        result.size = start - end;
        result.absolute_pos = start - result.size;
        result.data = buffer->data + result.absolute_pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_backify_good(Buffer_Backify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_backify_next(Buffer_Backify_Loop *loop){
    loop->buffer = 0;
}

internal_4tech int
buffer_replace_range(Buffer *buffer, int start, int end, char *str, int len, int *shift_amount, int *request_amount){
    char *data;
    int result;
    int size;
    
    size = buffer_size(buffer);
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end <= size);
    
    *shift_amount = (len - (end - start));
    if (*shift_amount + size <= buffer->max){
        data = (char*)buffer->data;
        memmove_4tech(data + end + *shift_amount, data + end, buffer->size - end);
        buffer->size += *shift_amount;
        if (len != 0) memcpy_4tech(data + start, str, len);
        
        result = 0;
    }
    else{
        *request_amount = round_up_4tech(2*(*shift_amount + size), 4 << 10);
        result = 1;
    }
    
    return(result);
}

internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Buffer *buffer, Buffer_Edit *sorted_edits, char *strings, int edit_count, int *request_amount){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_amount;
    
    result = 0;
    shift_total = state->shift_total;
    i = state->i;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      strings + edit->str_start, edit->len, &shift_amount, request_amount);
        if (result) break;
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

internal_4tech void*
buffer_edit_provide_memory(Buffer *buffer, void *new_data, int new_max){
    void *result;
    
    assert_4tech(new_max >= buffer->size);
    
    result = buffer->data;
    memcpy_4tech(new_data, buffer->data, buffer->size);
    buffer->data = (char*)new_data;
    buffer->max = new_max;
    
    return(result);
}

// BOTTOM

