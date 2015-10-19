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
#endif

#ifndef Assert
#define Assert
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
    char *data;
    int size;
} Buffer_Stringify_Loop;

inline_4tech Buffer_Stringify_Loop
buffer_stringify_loop(Buffer *buffer, int start, int end){
    Buffer_Stringify_Loop result;
    if (0 <= start && start <= end && end < buffer->size){
        result.buffer = buffer;
        result.data = buffer->data + start;
        result.size = end - start;
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

inline_4tech void
buffer_stringify(Buffer *buffer, int start, int end, char *out){
    for (Buffer_Stringify_Loop loop = buffer_stringify_loop(buffer, start, end);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        memcpy_4tech(out, loop.data, loop.size);
        out += loop.size;
    }
}

internal_4tech int
buffer_count_newlines(Buffer *buffer, int start, int end){
    int new_line, count;
    char *data;
    int i;
    
    Assert(0 <= start);
    Assert(start <= end);
    Assert(end < buffer->size);
    
    data = buffer->data;
    count = 0;
    
    for (i = start; i < end; ++i){
        new_line = (data[i] == '\n');
        count += new_line;
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
    
    result = 0;
    
    i = state->i;
    count = state->count;
    start = state->start;
    
    for (; i < size; ++i){
        if (data[i] == '\n'){
            if (count == max){
                result = 1;
                break;
            }
            
            starts[count++] = start;
            start = i + 1;
        }
    }
    
    if (i == size){
        if (count == max) result = 1;
        else starts[count++] = start;
    }
    
    state->i = i;
    state->count = count;
    state->start = start;
    
    return(result);
}

inline_4tech float
measure_character(void *advance_data, int offset, int stride, int *new_line, char character){
    char *advances;
    float width;
    
    advances = (char*)advance_data + offset;
    switch (character){
    case 0: width = 0; *new_line = 1; break;
    case '\n': width = *(float*)(advances + stride * '\n'); *new_line = 1; break;
    case '\r': width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * '\r'); break;
    default: width = *(float*)(advances + stride * character);
    }
    
    return(width);
}

internal_4tech void
buffer_measure_widths(Buffer *buffer, void *advance_data, int offset, int stride){
    float *widths;
    debug_4tech(int *starts);
    int line_count;
    char *data;
    int size;
    int i, j, new_line;
    float width;
    char ch, next;
    
    Assert(buffer->widths_max >= buffer->line_count);
    
    widths = buffer->line_widths;
    debug_4tech(starts = buffer->line_starts);
    line_count = buffer->line_count;
    
    data = buffer->data;
    size = buffer->size;
    
    Assert(size < buffer->max);
    data[size] = 0;
    
    for (i = 0, j = 0; i < line_count; ++i){
        Assert(j == starts[i]);
        new_line = 0;
        width = 0;
        ch = data[j];
        next = data[++j];
        
        while (new_line == 0){
            width += measure_character(advance_data, offset, stride, &new_line, ch);
            ch = next;
            next = data[++j];
        }
        
        --j;
        widths[i] = width;
    }
}

internal_4tech void
buffer_remeasure_starts(Buffer *buffer, int line_start, int line_end, int line_shift, int text_shift){
    int *lines;
    int line_count;
    char *data;
    int size;
    int line_i, char_i, start;
    char character;
    
    lines = buffer->line_starts;
    line_count = buffer->line_count;
    
    if (line_shift != 0){
        memmove_4tech(lines + line_end + line_shift + 1, lines + line_end + 1,
                sizeof(int)*(line_count - line_end - 1));
        line_count += line_shift;
    }
    
    if (text_shift != 0){
        line_i = line_end + 1;
        lines = lines + line_i;
        for (; line_i < line_count; ++line_i, ++lines){
            *lines += text_shift;
        }
        lines = buffer->line_starts;
    }
    
    size = buffer->size;
    data = buffer->data;
    char_i = lines[line_start];
    line_i = line_start;
    
    Assert(size < buffer->max);
    data[size] = '\n';
    
    start = char_i;
    for (; char_i <= size; ++char_i){
        character = data[char_i];
        if (character == '\n'){
            if (line_i > line_end && start == lines[line_i]) break;
            lines[line_i++] = start;
            start = char_i + 1;
        }
    }
    
    buffer->line_count = line_count;
}

internal_4tech int
buffer_get_line_index(Buffer *buffer, int pos, int l_bound, int u_bound){
    int *lines;
    int start, end;
    int i;
    
    Assert(0 <= l_bound);
    Assert(l_bound <= u_bound);
    Assert(u_bound <= buffer->line_count);
    
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
        Assert(start < end);
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

internal_4tech int
buffer_quick_partition_cursors(Cursor_With_Index *positions, int start, int pivot){
    int i;
    int pivot_pos;
    pivot_pos = positions[pivot].pos;
    for (i = start; i < pivot; ++i){
        if (positions[i].pos < pivot_pos){
            Swap(positions[start], positions[i]);
            ++start;
        }
    }
    Swap(positions[start], positions[pivot]);
    return start;
}

internal_4tech void
buffer_quick_sort_cursors(Cursor_With_Index *positions, int start, int pivot){
    int mid;
    mid = buffer_quick_partition_cursors(positions, start, pivot);
    if (start < mid - 1) buffer_quick_sort_cursors(positions, start, mid - 1);
    if (mid + 1 < pivot) buffer_quick_sort_cursors(positions, mid + 1, pivot);
}

inline_4tech void
buffer_sort_cursors(Cursor_With_Index *positions, int count){
    Assert(count > 0);
    buffer_quick_sort_cursors(positions, 0, count-1);
}

internal_4tech int
buffer_quick_unpartition_cursors(Cursor_With_Index *positions, int start, int pivot){
    int i;
    int pivot_index;
    pivot_index = positions[pivot].index;
    for (i = start; i < pivot; ++i){
        if (positions[i].index < pivot_index){
            Swap(positions[start], positions[i]);
            ++start;
        }
    }
    Swap(positions[start], positions[pivot]);
    return start;
}

internal_4tech void
buffer_quick_unsort_cursors(Cursor_With_Index *positions, int start, int pivot){
    int mid;
    mid = buffer_quick_unpartition_cursors(positions, start, pivot);
    if (start < mid - 1) buffer_quick_unsort_cursors(positions, start, mid - 1);
    if (mid + 1 < pivot) buffer_quick_unsort_cursors(positions, mid + 1, pivot);
}

inline_4tech void
buffer_unsort_cursors(Cursor_With_Index *positions, int count){
    Assert(count > 0);
    buffer_quick_unsort_cursors(positions, 0, count-1);
}

internal_4tech void
buffer_update_cursors(Cursor_With_Index *sorted_positions, int count, int start, int end, int len){
    Cursor_With_Index *position, *end_position;
    int shift_amount;
    
    shift_amount = (len - (end - start));
    
    position = sorted_positions;
    end_position = sorted_positions + count;
    for (; position < end_position && position->pos < start; ++position);
    for (; position < end_position && position->pos < end; ++position) position->pos = start;
    for (; position < end_position; ++position) position->pos += shift_amount;
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
buffer_cursor_seek(Buffer *buffer, Buffer_Seek seek,
                   float max_width, float font_height, void *width_data, int offset, int stride, Full_Cursor cursor){
    Full_Cursor prev_cursor;
    char *data, *data_;
    int size;
    int do_newline, do_slashr;
    char ch, next;
    float ch_width;
    
    int get_out;
    int xy_seek;
    float x, y, px;
    
    data = buffer->data;
    size = buffer->size;
    Assert(size < buffer->max);
    data[size] = 0;
    
    data_ = (char*)width_data + offset;
    
    xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
    
    for (;;){
        prev_cursor = cursor;
        ch_width = 0;
        ch = data[cursor.pos];
        next = data[cursor.pos+1];
        
        switch (ch){
        case '\r': do_newline = 0; do_slashr = 1; break;
        case '\n': do_newline = 1; do_slashr = 0; break;
            
        default:
            do_newline = 0; do_slashr = 0; 
            ++cursor.character;
            ch_width = *(float*)(data_ + stride * ch);
            break;
        }
        
        if (do_slashr){
            ++cursor.character;
            ch_width = *(float*)(data_ + stride * '\\') + *(float*)(data_ + stride * 'r');
        }
        
        if (cursor.wrapped_x + ch_width >= max_width){
            cursor.wrapped_y += font_height;
            cursor.wrapped_x = 0;
            prev_cursor = cursor;
        }
        
        cursor.unwrapped_x += ch_width;
        cursor.wrapped_x += ch_width;
        
        if (do_newline){
            ++cursor.line;
            cursor.unwrapped_y += font_height;
            cursor.wrapped_y += font_height;
            cursor.character = 0;
            cursor.unwrapped_x = 0;
            cursor.wrapped_x = 0;
        }
        
        ++cursor.pos;
        
        if (cursor.pos > size){
            cursor = prev_cursor;
            break;
        }
        
        get_out = 0;
        x = 0;
        y = 0;
        px = 0;
        
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
            
            if (seek.round_down){
                if (y > seek.y - font_height && x > seek.x){
                    cursor = prev_cursor;
                    break;
                }
            }
            else{
                if (y > seek.y - font_height && x >= seek.x){
                    if ((seek.x - px) < (x - seek.x)) cursor = prev_cursor;
                    break;
                }
            }
        }
    }
    
    return(cursor);
}

typedef struct{
    char *str;
    int len;
    int start, end;
} Buffer_Edit;

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
        start_point = Max(edit->end, edit->start + 1);
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
    
    return shift_max;
}

typedef struct{
    int i;
    int shift_total;
} Buffer_Batch_State;

internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Buffer *buffer, Buffer_Edit *sorted_edits, int edit_count){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_amount;
    
    result = 0;
    shift_total = state->shift_total;
    i = state->i;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      edit->str, edit->len, &shift_amount);
        if (result) break;
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return result;
}

internal_4tech void
buffer_batch_edit(Buffer *buffer, Buffer_Edit *sorted_edits, int edit_count){
    Buffer_Batch_State state;
    int result;
    
    state.i = 0;
    state.shift_total = 0;
    
    result = buffer_batch_edit_step(&state, buffer, sorted_edits, edit_count);
    Assert(result == 0);
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
    
    *all_space = 0;
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
    Assert(size < buffer->max);
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
    Assert(buffer_eol_convert_out_size(buffer) < buffer->max);
    data[size] = 0;
    
    for (i = 0; i < size; ++i){
        if (data[i] == '\n'){
            memmove_4tech(data + i, data + i + 1, size - i);
            data[i] = '\r';
            ++i;
        }
    }
    
    buffer->size = size;
}

// BOTTOM

