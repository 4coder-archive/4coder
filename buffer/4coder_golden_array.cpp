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

#ifndef defines_4tech
#define inline_4tech inline
#define internal_4tech static
#define memset_4tech memset
#define memcpy_4tech memcpy
#define memmove_4tech memmove
#define defines_4tech 1
#define debug_4tech(x) x
#define assert_4tech assert
#endif

typedef struct{
    char *data;
    int size, max;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int line_max;
    int widths_max;
} Buffer;

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int size;
    int page_size;
} Buffer_Stringify_Loop;

inline_4tech Buffer_Stringify_Loop
buffer_stringify_loop(Buffer *buffer, int start, int end, int page_size){
    Buffer_Stringify_Loop result;
    if (0 <= start && start < end && end <= buffer->size){
        result.buffer = buffer;
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
        loop->size = (int)(loop->end - loop->data);
        if (loop->size > loop->page_size) loop->size = loop->page_size;
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
buffer_count_newlines(Buffer *buffer, int start, int end){
    char *data;
    int i;
    int count;
    
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end < buffer->size);
    
    data = buffer->data;
    count = 0;
    
    for (i = start; i < end; ++i){
        count += (data[i] == '\n');
    }
    
    return(count);
}

typedef struct{
    int i;
    int count;
    int start;
} Buffer_Measure_Starts;

internal_4tech int
buffer_measure_starts(Buffer_Measure_Starts *state, Buffer *buffer){
    int *starts;
    int max;
    char *data;
    int size;
    int start, count, i;
    int result;
    
    starts = buffer->line_starts;
    max = buffer->line_max;
    
    data = buffer->data;
    size = buffer->size;
    
    assert_4tech(size < buffer->max);
    data[size] = '\n';
    
    result = 0;
    
    i = state->i;
    count = state->count;
    start = state->start;
    
    for (; i <= size; ++i){
        if (data[i] == '\n'){
            if (count == max){
                result = 1;
                break;
            }
            
            starts[count++] = start;
            start = i + 1;
        }
    }
    
    state->i = i;
    state->count = count;
    state->start = start;
    
    return(result);
}

internal_4tech void
buffer_remeasure_starts(Buffer *buffer, int line_start, int line_end, int line_shift, int text_shift){
    int *starts;
    int line_count;
    char *data;
    int size;
    int line_i, char_i, start;
    
    starts = buffer->line_starts;
    line_count = buffer->line_count;
    
    assert_4tech(0 <= line_start);
    assert_4tech(line_start <= line_end);
    assert_4tech(line_end < line_count);
    assert_4tech(line_count + line_shift <= buffer->line_max);
    
    ++line_end;
    if (text_shift != 0){
        line_i = line_end;
        starts += line_i;
        for (; line_i < line_count; ++line_i, ++starts){
            *starts += text_shift;
        }
        starts = buffer->line_starts;
    }
    
    if (line_shift != 0){
        memmove_4tech(starts + line_end + line_shift, starts + line_end,
                      sizeof(int)*(line_count - line_end));
        line_count += line_shift;
    }
    
    size = buffer->size;
    data = buffer->data;
    char_i = starts[line_start];
    line_i = line_start;
    
    assert_4tech(size < buffer->max);
    data[size] = '\n';
    
    start = char_i;
    line_end += line_shift;
    for (; char_i <= size; ++char_i){
        if (data[char_i] == '\n'){
            starts[line_i++] = start;
            start = char_i + 1;
            if (line_i >= line_end && start == starts[line_i]) break;
        }
    }
    
    assert_4tech(line_count >= 1);
    buffer->line_count = line_count;
}

inline_4tech float
measure_character(void *advance_data, int stride, char character){
    char *advances;
    float width;
    
    advances = (char*)advance_data;
    switch (character){
    case 0: width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * '0'); break;
    case '\n': width = 0; break;
    case '\r': width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * '\r'); break;
    default: width = *(float*)(advances + stride * character);
    }
    
    return(width);
}

internal_4tech void
buffer_remeasure_widths(Buffer *buffer, void *advance_data, int stride,
                        int line_start, int line_end, int line_shift){
    int *starts;
    float *widths;
    int line_count;
    char *data;
    int i, j;
    float width;
    char ch;
    
    starts = buffer->line_starts;
    widths = buffer->line_widths;
    line_count = buffer->line_count;
    
    assert_4tech(0 <= line_start);
    assert_4tech(line_start <= line_end);
    assert_4tech(line_end < line_count);
    assert_4tech(line_count <= buffer->widths_max);

    ++line_end;
    if (line_shift != 0){
        memmove_4tech(widths + line_end + line_shift, widths + line_end,
                      sizeof(float)*(line_count - line_end));
        line_count += line_shift;
    }
    
    data = buffer->data;
    
    assert_4tech(buffer->size < buffer->max);
    data[buffer->size] = '\n';
    
    i = line_start;
    j = starts[i];
    
    line_end += line_shift;    
    for (; i < line_end; ++i){
        assert_4tech(j == starts[i]);
        width = 0;
        
        for (ch = data[j]; ch != '\n'; ch = data[++j]){
            width += measure_character(advance_data, stride, ch);
        }
        ++j;
        
        widths[i] = width;
    }
}

inline_4tech void
buffer_measure_widths(Buffer *buffer, void *advance_data, int stride){
    assert_4tech(buffer->line_count >= 1);
    buffer_remeasure_widths(buffer, advance_data, stride, 0, buffer->line_count-1, 0);
}

internal_4tech int
buffer_get_line_index(Buffer *buffer, int pos, int l_bound, int u_bound){
    int *lines;
    int start, end;
    int i;
    
    assert_4tech(0 <= l_bound);
    assert_4tech(l_bound <= u_bound);
    assert_4tech(u_bound <= buffer->line_count);
    
    start = l_bound;
    end = u_bound;
    lines = buffer->line_starts;
    for (;;){
        i = (start + end) >> 1;
        if (lines[i] < pos) start = i;
        else if (lines[i] > pos) end = i;
        else{
            start = i;
            break;
        }
        assert_4tech(start < end);
        if (start == end - 1) break;
    }
    
    return(start);
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

typedef struct{
    int pos, index;
} Cursor_With_Index;

inline_4tech void
write_cursor_with_index(Cursor_With_Index *positions, int *count, int pos){
    positions[*count].index = *count;
    positions[*count].pos = pos;
    ++*count;
}

#define CursorSwap__(a,b) { Cursor_With_Index t = a; a = b; b = t; }

internal_4tech void
buffer_quick_sort_cursors(Cursor_With_Index *positions, int start, int pivot){
    int i, mid;
    int pivot_pos;
    
    mid = start;
    pivot_pos = positions[pivot].pos;
    for (i = mid; i < pivot; ++i){
        if (positions[i].pos < pivot_pos){
            CursorSwap__(positions[mid], positions[i]);
            ++mid;
        }
    }
    CursorSwap__(positions[mid], positions[pivot]);
    
    if (start < mid - 1) buffer_quick_sort_cursors(positions, start, mid - 1);
    if (mid + 1 < pivot) buffer_quick_sort_cursors(positions, mid + 1, pivot);
}

inline_4tech void
buffer_sort_cursors(Cursor_With_Index *positions, int count){
    assert_4tech(count > 0);
    buffer_quick_sort_cursors(positions, 0, count-1);
}

internal_4tech void
buffer_quick_unsort_cursors(Cursor_With_Index *positions, int start, int pivot){
    int i, mid;
    int pivot_index;
    
    mid = start;
    pivot_index = positions[pivot].index;
    for (i = mid; i < pivot; ++i){
        if (positions[i].index < pivot_index){
            CursorSwap__(positions[mid], positions[i]);
            ++mid;
        }
    }
    CursorSwap__(positions[mid], positions[pivot]);
    
    if (start < mid - 1) buffer_quick_unsort_cursors(positions, start, mid - 1);
    if (mid + 1 < pivot) buffer_quick_unsort_cursors(positions, mid + 1, pivot);
}

#undef CursorSwap__

inline_4tech void
buffer_unsort_cursors(Cursor_With_Index *positions, int count){
    assert_4tech(count > 0);
    buffer_quick_unsort_cursors(positions, 0, count-1);
}

internal_4tech void
buffer_update_cursors(Cursor_With_Index *sorted_positions, int count, int start, int end, int len){
    Cursor_With_Index *position;
    int shift_amount;
    
    shift_amount = (len - (end - start));
    
    position = sorted_positions + count - 1;
    for (; position >= sorted_positions && position->pos >= end; --position) position->pos += shift_amount;
    for (; position >= sorted_positions && position->pos >= start; --position) position->pos = start;
}

typedef enum{
    buffer_seek_pos,
    buffer_seek_wrapped_xy,
    buffer_seek_unwrapped_xy,
    buffer_seek_line_char
} Buffer_Seek_Type;

typedef struct{
    Buffer_Seek_Type type;
    union{
        struct { int pos; };
        struct { int round_down; float x, y; };
        struct { int line, character; };
    };
} Buffer_Seek;

inline_4tech Buffer_Seek
seek_pos(int pos){
    Buffer_Seek result;
    result.type = buffer_seek_pos;
    result.pos = pos;
    return(result);
}

inline_4tech Buffer_Seek
seek_wrapped_xy(float x, float y, int round_down){
    Buffer_Seek result;
    result.type = buffer_seek_wrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

inline_4tech Buffer_Seek
seek_unwrapped_xy(float x, float y, int round_down){
    Buffer_Seek result;
    result.type = buffer_seek_unwrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

inline_4tech Buffer_Seek
seek_line_char(int line, int character){
    Buffer_Seek result;
    result.type = buffer_seek_line_char;
    result.line = line;
    result.character = character;
    return(result);
}

typedef struct{
    int pos;
    int line, character;
    float unwrapped_x, unwrapped_y;
    float wrapped_x, wrapped_y;
} Full_Cursor;

internal_4tech Full_Cursor
buffer_cursor_seek(Buffer *buffer, Buffer_Seek seek, float max_width, float font_height,
                   void *advance_data, int stride, Full_Cursor cursor){
    Full_Cursor prev_cursor;
    char *data, *advances;
    int size;
    char ch;
    float ch_width;
    
    int get_out;
    int xy_seek;
    float x, y, px;
    
    data = buffer->data;
    size = buffer->size;
    assert_4tech(size < buffer->max);
    data[size] = 0;
    
    advances = (char*)advance_data;
    
    x = 0;
    y = 0;
    px = 0;
    xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
    
    for (;;){
        prev_cursor = cursor;
        ch = data[cursor.pos];
        
        switch (ch){
        case '\n':
            ++cursor.line;
            cursor.unwrapped_y += font_height;
            cursor.wrapped_y += font_height;
            cursor.character = 0;
            cursor.unwrapped_x = 0;
            cursor.wrapped_x = 0;
            break;
            
        default:
            ++cursor.character;
            if (ch == '\r') ch_width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * 'r');
            else ch_width = *(float*)(advances + stride * ch);
            
            if (cursor.wrapped_x + ch_width >= max_width){
                cursor.wrapped_y += font_height;
                cursor.wrapped_x = 0;
                prev_cursor = cursor;
            }
            
            cursor.unwrapped_x += ch_width;
            cursor.wrapped_x += ch_width;
            
            break;
        }
        
        ++cursor.pos;
        
        if (cursor.pos > size){
            cursor = prev_cursor;
            break;
        }
        
        get_out = 0;
        
        switch (seek.type){
        case buffer_seek_pos:
            if (cursor.pos > seek.pos){
                cursor = prev_cursor;
                get_out = 1;
            }break;
            
        case buffer_seek_wrapped_xy:
            x = cursor.wrapped_x; px = prev_cursor.wrapped_x;
            y = cursor.wrapped_y; break;
            
        case buffer_seek_unwrapped_xy:
            x = cursor.unwrapped_x; px = prev_cursor.unwrapped_x;
            y = cursor.unwrapped_y; break;
                
        case buffer_seek_line_char:
            if (cursor.line == seek.line && cursor.character >= seek.character){
                get_out = 1;
            }
            else if (cursor.line > seek.line){
                cursor = prev_cursor;
                get_out = 1;
            }break;
        }
        
        if (get_out) break;
        if (xy_seek){
            if (y > seek.y){
                cursor = prev_cursor;
                break;
            }
            
            if (y > seek.y - font_height && x >= seek.x){
                if (!seek.round_down){
                    if ((seek.x - px) < (x - seek.x)) cursor = prev_cursor;
                    break;
                }
                
                if (x > seek.x){
                    cursor = prev_cursor;
                    break;
                }
            }
        }
    }
    
    return(cursor);
}

inline_4tech Full_Cursor
make_cursor_hint(int line_index, int *starts, float *wrap_ys, float font_height){
    Full_Cursor hint;
    hint.pos = starts[line_index];
    hint.line = line_index + 1;
    hint.character = 1;
    hint.unwrapped_y = (f32)(line_index * font_height);
    hint.unwrapped_x = 0;
    hint.wrapped_y = wrap_ys[line_index];
    hint.wrapped_x = 0;
    return(hint);
}

internal_4tech Full_Cursor
buffer_cursor_from_pos(Buffer *buffer, int pos, float *wraps,
                       float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    int line_index;

    line_index = buffer_get_line_index(buffer, pos, 0, buffer->line_count);
    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_pos(pos), max_width, font_height,
                                advance_data, stride, result);

    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_unwrapped_xy(Buffer *buffer, float x, float y, int round_down, float *wraps,
                                float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    int line_index;

    line_index = (int)(y / font_height);
    if (line_index >= buffer->line_count) line_index = buffer->line_count - 1;
    if (line_index < 0) line_index = 0;

    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_unwrapped_xy(x, y, round_down), max_width, font_height,
                                advance_data, stride, result);

    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_wrapped_xy(Buffer *buffer, float x, float y, int round_down, float *wraps,
                              float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    int line_index;
    int start, end, i;

    // NOTE(allen): binary search lines on wrapped y position
    // TODO(allen): pull this out once other wrap handling code is ready
    start = 0;
    end = buffer->line_count;
    for (;;){
        i = (start + end) / 2;
        if (wraps[i]+font_height <= y) start = i;
        else if (wraps[i] > y) end = i;
        else{
            line_index = i;
            break;
        }
        if (start >= end - 1){
            line_index = start;
            break;
        }
    }

    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_wrapped_xy(x, y, round_down), max_width, font_height,
                                advance_data, stride, result);

    return(result);
}

typedef struct{
    int str_start, len;
    int start, end;
} Buffer_Edit;

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

internal_4tech int
buffer_batch_debug_sort_check(Buffer_Edit *sorted_edits, int edit_count){
    Buffer_Edit *edit;
    int i, result, start_point;
    
    result = 1;
    start_point = 0;
    
    edit = sorted_edits;
    for (i = 0; i < edit_count; ++i, ++edit){
        if (start_point > edit->start){
            result = 0; break;
        }
        start_point = (edit->end < edit->start + 1)?edit->start + 1:edit->end;
    }
    
    return(result);
}

internal_4tech int
buffer_batch_edit_max_shift(Buffer_Edit *sorted_edits, int edit_count){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_max;
    
    result = 0;
    shift_total = 0;
    shift_max = 0;
    
    edit = sorted_edits;
    for (i = 0; i < edit_count; ++i, ++edit){
        shift_total += (edit->len - (edit->end - edit->start));
        if (shift_total > shift_max) shift_max = shift_total;
    }
    
    return(shift_max);
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

internal_4tech void
buffer_batch_edit_update_cursors(Cursor_With_Index *sorted_positions, int count, Buffer_Edit *sorted_edits, int edit_count){
    Cursor_With_Index *position, *end_position;
    Buffer_Edit *edit, *end_edit;
    int start, end;
    int shift_amount;
    
    position = sorted_positions;
    end_position = sorted_positions + count;
    
    edit = sorted_edits;
    end_edit = sorted_edits + edit_count;
    
    shift_amount = 0;
    
    for (; edit < end_edit && position < end_position; ++edit){
        start = edit->start;
        end = edit->end;
        
        for (; position->pos < start && position < end_position; ++position){
            position->pos += shift_amount;
        }
        
        for (; position->pos < end && position < end_position; ++position){
            position->pos = start + shift_amount;
        }
        
        shift_amount += (edit->len - (end - start));
    }
    
    for (; position < end_position; ++position){
        position->pos += shift_amount;
    }
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
    char *data;
    int size;
    int i;
    
    data = buffer->data;
    size = buffer->size;
    assert_4tech(size < buffer->max);
    data[size] = 0;
    
    for (i = 0; i < size; ++i){
        if (data[i] == '\r' && data[i+1] == '\n'){
            memmove_4tech(data + i, data + i + 1, size - i);
            size -= 1;
        }
    }
    
    buffer->size = size;
}

inline_4tech int
buffer_eol_convert_out_size(Buffer *buffer){
    int size;
    size = buffer->size + buffer->line_count;
    return(size);
}

internal_4tech void
buffer_eol_convert_out(Buffer *buffer){
    char *data;
    int size;
    int i;
    
    data = buffer->data;
    size = buffer->size;
    assert_4tech(buffer_eol_convert_out_size(buffer) < buffer->max);
    
    for (i = 0; i < size; ++i){
        if (data[i] == '\n'){
            memmove_4tech(data + i + 1, data + i, size - i);
            data[i] = '\r';
            ++i;
            ++size;
        }
    }
    
    buffer->size = size;
}

typedef struct{
    int index;
    int glyphid;
    float x0, y0;
    float x1, y1;
} Buffer_Render_Item;

internal_4tech void
buffer_get_render_data(Buffer *buffer, float *wraps, Buffer_Render_Item *items, int max, int *count,
                       float port_x, float port_y, float scroll_x, float scroll_y, int wrapped,
                       float width, float height, void *advance_data, int stride, float font_height){
    Full_Cursor start_cursor;
    Buffer_Render_Item *item;
    char *data;
    int size;
    float shift_x, shift_y;
    float x, y;
    int i, item_i;
    float ch_width;
    char ch;

    data = buffer->data;
    size = buffer->size;
    assert_4tech(size < buffer->max);
    data[size] = 0;

    shift_x = port_x - scroll_x;
    shift_y = port_y - scroll_y;
    if (wrapped){
        start_cursor = buffer_cursor_from_wrapped_xy(buffer, 0, scroll_y, 0, wraps,
                                                     width, font_height, advance_data, stride);
        shift_y += start_cursor.wrapped_y;
    }
    else{
        start_cursor = buffer_cursor_from_unwrapped_xy(buffer, 0, scroll_y, 0, wraps,
                                                       width, font_height, advance_data, stride);
        shift_y += start_cursor.unwrapped_y;
    }

    i = start_cursor.pos;

    x = shift_x;
    y = shift_y;
    item_i = 0;
    item = items + item_i;

    for (; i <= size; ++i){
        ch = data[i];
        ch_width = measure_character(advance_data, stride, ch);

        if (ch_width + x > width + shift_x && wrapped){
            x = shift_x;
            y += font_height;
        }
        if (y > height + shift_y) break;

        switch (ch){
        case '\n':
            ch_width = measure_character(advance_data, stride, ' ');
            item->index = i;
            item->glyphid = ' ';
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + ch_width;
            item->y1 = y + font_height;
            ++item_i;
            ++item;

            x = shift_x;
            y += font_height;
            break;

        case 0:
            ch_width = measure_character(advance_data, stride, '\\');
            item->index = i;
            item->glyphid = '\\';
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + ch_width;
            item->y1 = y + font_height;
            ++item_i;
            ++item;
            x += ch_width;

            ch_width = measure_character(advance_data, stride, '0');
            item->index = i;
            item->glyphid = '0';
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + ch_width;
            item->y1 = y + font_height;
            ++item_i;
            ++item;
            x += ch_width;
            break;

        case '\r':
            ch_width = measure_character(advance_data, stride, '\\');
            item->index = i;
            item->glyphid = '\\';
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + ch_width;
            item->y1 = y + font_height;
            ++item_i;
            ++item;
            x += ch_width;

            ch_width = measure_character(advance_data, stride, 'r');
            item->index = i;
            item->glyphid = 'r';
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + ch_width;
            item->y1 = y + font_height;
            ++item_i;
            ++item;
            x += ch_width;
            break;

        case '\t':
            item->index = i;
            item->glyphid = '\\';
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + measure_character(advance_data, stride, '\\');
            item->y1 = y + font_height;
            ++item_i;
            ++item;

            item->index = i;
            item->glyphid = 't';
            item->x0 = (item-1)->x1;
            item->y0 = y;
            item->x1 = item->x0 + measure_character(advance_data, stride, 't');
            item->y1 = y + font_height;
            ++item_i;
            ++item;
            x += ch_width;
            break;

        default:
            item->index = i;
            item->glyphid = ch;
            item->x0 = x;
            item->y0 = y;
            item->x1 = x + ch_width;
            item->y1 = y + font_height;
            ++item_i;
            ++item;
            x += ch_width;
            break;
        }
        if (y > height + shift_y) break;
    }

    // TODO(allen): handle this with a control state
    assert_4tech(item_i <= max);
    *count = item_i;
}

// BOTTOM

