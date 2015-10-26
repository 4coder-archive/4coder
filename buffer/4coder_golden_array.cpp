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
    int line_max;
    int widths_max;
} Buffer;

inline_4tech int
buffer_size(Buffer *buffer){
    return buffer->size;
}

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int absolute_pos;
    int size;
    int page_size;
} Buffer_Stringify_Loop;

inline_4tech Buffer_Stringify_Loop
buffer_stringify_loop(Buffer *buffer, int start, int end, int page_size){
    Buffer_Stringify_Loop result;
    if (0 <= start && start < end && end <= buffer->size){
        result.buffer = buffer;
        result.absolute_pos = start;
        result.data = buffer->data + start;
        result.size = end - start;
        result.end = buffer->data + end;
        result.page_size = page_size;
        if (result.size > page_size) result.size = page_size;
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
    if (loop->data + loop->size == loop->end) loop->buffer = 0;
    else{
        loop->data += loop->page_size;
        loop->absolute_pos += loop->page_size;
        loop->size = (int)(loop->end - loop->data);
        if (loop->size > loop->page_size) loop->size = loop->page_size;
    }
}

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int absolute_pos;
    int size;
    int page_size;
} Buffer_Backify_Loop;

inline_4tech Buffer_Backify_Loop
buffer_backify_loop(Buffer *buffer, int start, int end, int page_size){
    Buffer_Backify_Loop result;
    if (0 <= end && end < start && start <= buffer->size){
        result.buffer = buffer;
        result.end = buffer->data + end;
        result.page_size = page_size;
        result.size = start - end;
        if (result.size > page_size) result.size = page_size;
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
    char *old_data;
    if (loop->data == loop->end) loop->buffer = 0;
    else{
        old_data = loop->data;
        loop->data -= loop->page_size;
        loop->absolute_pos -= loop->page_size;
        if (loop->data < loop->end){
            loop->size = (int)(old_data - loop->end);
            loop->data = loop->end;
            loop->absolute_pos = 0;
        }
    }
}

inline_4tech void
buffer_stringify(Buffer *buffer, int start, int end, char *out){
    for (Buffer_Stringify_Loop loop = buffer_stringify_loop(buffer, start, end, end - start);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        memcpy_4tech(out, loop.data, loop.size);
        out += loop.size;
    }
}

internal_4tech int
buffer_replace_range(Buffer *buffer, int start, int end, char *str, int len, int *shift_amount){
    char *data;
    int result;
    
    *shift_amount = (len - (end - start));
    if (*shift_amount + buffer->size + 1 <= buffer->max){
        data = (char*)buffer->data;
        memmove_4tech(data + end + *shift_amount, data + end, buffer->size - end);
        buffer->size += *shift_amount;
        data[buffer->size] = 0;
        if (len != 0) memcpy_4tech(data + start, str, len);
        
        result = 0;
    }
    else{
        result = *shift_amount + buffer->size + 1;
    }
    
    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_seek(Buffer *buffer, Buffer_Seek seek, float max_width, float font_height,
                   void *advance_data, int stride, Full_Cursor cursor){
    char *data;
    int size;
    
    Seek_State state;
    char *advances;
    char ch;
    int xy_seek;
    int result;
    
    data = buffer->data;
    size = buffer->size;
    assert_4tech(size < buffer->max);
    data[size] = 0;
    
    advances = (char*)advance_data;
    xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
    state.cursor = cursor;
    
    do{
        ch = data[state.cursor.pos];
        result = cursor_seek_step(&state, seek, xy_seek, max_width, font_height,
                                  advances, stride, size, ch);
    }while(result);
    
    return(state.cursor);
}

internal_4tech void
buffer_invert_edit_shift(Buffer *buffer, Buffer_Edit edit, Buffer_Edit *inverse, char *strings, int *str_pos, int max, int shift_amount){
    int pos;
    int len;
    
    pos = *str_pos;
    len = edit.end - edit.start;
    assert_4tech(pos + len <= max);
    *str_pos = pos + len;
    
    inverse->str_start = pos;
    inverse->len = len;
    inverse->start = edit.start + shift_amount;
    inverse->end = edit.start + edit.len + shift_amount;
    memcpy_4tech(strings + pos, buffer->data + edit.start, len);
}

inline_4tech void
buffer_invert_edit(Buffer *buffer, Buffer_Edit edit, Buffer_Edit *inverse, char *strings, int *str_pos, int max){
    buffer_invert_edit_shift(buffer, edit, inverse, strings, str_pos, max, 0);
}

typedef struct{
    int i;
    int shift_amount;
    int len;
} Buffer_Invert_Batch;

internal_4tech int
buffer_invert_batch(Buffer_Invert_Batch *state, Buffer *buffer, Buffer_Edit *edits, int count,
                    Buffer_Edit *inverse, char *strings, int *str_pos, int max){
    Buffer_Edit *edit, *inv_edit;
    int shift_amount;
    int result;
    int i;
    
    result = 0;
    i = state->i;
    shift_amount = state->shift_amount;
    
    edit = edits + i;
    inv_edit = inverse + i;
    
    for (; i < count; ++i, ++edit, ++inv_edit){
        if (*str_pos + edit->end - edit->start <= max){
            buffer_invert_edit_shift(buffer, *edit, inv_edit, strings, str_pos, max, shift_amount);
            shift_amount += (edit->len - (edit->end - edit->start));
        }
        else{
            result = 1;
            state->len = edit->end - edit->start;
        }
    }
    
    state->i = i;
    state->shift_amount = shift_amount;
    
    return(result);
}

typedef struct{
    int i;
    int shift_total;
} Buffer_Batch_State;

internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Buffer *buffer, Buffer_Edit *sorted_edits, char *strings, int edit_count){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_amount;
    
    result = 0;
    shift_total = state->shift_total;
    i = state->i;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      strings + edit->str_start, edit->len, &shift_amount);
        if (result) break;
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

internal_4tech void
buffer_batch_edit(Buffer *buffer, Buffer_Edit *sorted_edits, char *strings, int edit_count){
    Buffer_Batch_State state;
    debug_4tech(int result);
    
    state.i = 0;
    state.shift_total = 0;
    
    debug_4tech(result =)
        buffer_batch_edit_step(&state, buffer, sorted_edits, strings, edit_count);
    assert_4tech(result == 0);
}

internal_4tech int
buffer_find_hard_start(Buffer *buffer, int line_start, int *all_whitespace, int *all_space,
                       int *preferred_indent, int tab_width){
    char *data;
    int size;
    int result;
    char c;
    
    *all_space = 1;
    *preferred_indent = 0;
    
    data = buffer->data;
    size = buffer->size;
    
    tab_width -= 1;
    
    for (result = line_start; result < size; ++result){
        c = data[result];
        if (c == '\n' || c == 0){
            *all_whitespace = 1;
            break;
        }
        if (c >= '!' && c <= '~') break;
        if (c == '\t') *preferred_indent += tab_width;
        if (c != ' ') *all_space = 0;
        *preferred_indent += 1;
    }
    
    return(result);
}

internal_4tech void
buffer_eol_convert_in(Buffer *buffer){
    buffer->size = eol_convert_in(buffer->data, buffer->size);
}

inline_4tech int
buffer_eol_convert_out_size(Buffer *buffer){
    int size;
    size = buffer->size + buffer->line_count;
    return(size);
}

internal_4tech void
buffer_eol_convert_out(Buffer *buffer){
    int size;
    assert_4tech(buffer_eol_convert_out_size(buffer) < buffer->max);
    eol_convert_out(buffer->data, buffer->size, buffer->max, &size);
    buffer->size = size;
}

// BOTTOM

