/* 
* Mr. 4th Dimention - Allen Webster
*  Four Tech
*
* public domain -- no warranty is offered or implied; use this code at your own risk
* 
* 24.10.2015
* 
*/

// TOP

#define Buffer_Init_Type cat_4tech(Buffer_Type, _Init)
#define Buffer_Stringify_Type cat_4tech(Buffer_Type, _Stringify_Loop)
#define Buffer_Backify_Type cat_4tech(Buffer_Type, _Backify_Loop)

inline_4tech void
buffer_stringify(Buffer_Type *buffer, int start, int end, char *out){
    for (Buffer_Stringify_Type loop = buffer_stringify_loop(buffer, start, end);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        memcpy_4tech(out, loop.data, loop.size);
        out += loop.size;
    }
}

internal_4tech int
buffer_convert_out(Buffer_Type *buffer, char *dest, int max){
    Buffer_Stringify_Type loop;
    int size, out_size, pos, result;
    
    size = buffer_size(buffer);
    assert_4tech(size + buffer->line_count < max);
    
    pos = 0;
    for (loop = buffer_stringify_loop(buffer, 0, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        result = eol_convert_out(dest + pos, max - pos, loop.data, loop.size, &out_size);
        assert_4tech(result);
        pos += out_size;
    }
    
    return(pos);
}

internal_4tech int
buffer_count_newlines(Buffer_Type *buffer, int start, int end){
    Buffer_Stringify_Type loop;
    int i;
    int count;
    
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end <= buffer_size(buffer));
    
    count = 0;
    
    for (loop = buffer_stringify_loop(buffer, start, end);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        for (i = 0; i < loop.size; ++i){
            count += (loop.data[i] == '\n');
        }
    }
    
    return(count);
}

#ifndef NON_ABSTRACT_4TECH
typedef struct Buffer_Measure_Starts{
    int i;
    int count;
    int start;
    float width;
} Buffer_Measure_Starts;
#endif

internal_4tech int
buffer_measure_starts_widths_(Buffer_Measure_Starts *state, Buffer_Type *buffer, float *advance_data){
    Buffer_Stringify_Type loop;
    int *start_ptr, *start_end;
    float *width_ptr;
    debug_4tech(int widths_max);
    debug_4tech(int max);
    char *data;
    int size, end;
    float width;
    int start, i;
    int result;
    char ch;
    
    size = buffer_size(buffer);
    
    debug_4tech(max = buffer->line_max);
    debug_4tech(widths_max = buffer->widths_max);
    assert_4tech(max == widths_max);
    
    result = 1;
    
    i = state->i;
    start = state->start;
    width = state->width;
    
    start_ptr = buffer->line_starts + state->count;
    width_ptr = buffer->line_widths + state->count;
    start_end = buffer->line_starts + buffer->line_max;
    
    for (loop = buffer_stringify_loop(buffer, i, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; i < end; ++i){
            ch = data[i];
            if (ch == '\n'){
                if (start_ptr == start_end) goto buffer_measure_starts_widths_end;
                
                *width_ptr++ = width;
                *start_ptr++ = start;
                start = i + 1;
                width = 0;
            }
            else{
                width += measure_character(advance_data, ch);
            }
        }
    }
    
    assert_4tech(i == size);
    
    if (start_ptr == start_end) goto buffer_measure_starts_widths_end;
    *start_ptr++ = start;
    *width_ptr++ = 0;
    result = 0;
    
    buffer_measure_starts_widths_end:
    state->i = i;
    state->count = (int)(start_ptr - buffer->line_starts);
    state->start = start;
    state->width = width;
    
    return(result);
}

internal_4tech int
buffer_measure_starts_zero_widths_(Buffer_Measure_Starts *state, Buffer_Type *buffer){
    Buffer_Stringify_Type loop;
    int *start_ptr, *start_end;
    float *width_ptr;
    debug_4tech(int widths_max);
    debug_4tech(int max);
    char *data;
    int size, end;
    int start, i;
    int result;
    char ch;
    
    size = buffer_size(buffer);
    
    debug_4tech(max = buffer->line_max);
    debug_4tech(widths_max = buffer->widths_max);
    assert_4tech(max == widths_max);
    
    result = 1;
    
    i = state->i;
    start = state->start;
    
    start_ptr = buffer->line_starts + state->count;
    width_ptr = buffer->line_widths + state->count;
    start_end = buffer->line_starts + buffer->line_max;
    
    for (loop = buffer_stringify_loop(buffer, i, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; i < end; ++i){
            ch = data[i];
            if (ch == '\n'){
                if (start_ptr == start_end) goto buffer_measure_starts_zero_widths_end;
                
                *width_ptr++ = 0;
                *start_ptr++ = start;
                start = i + 1;
            }
        }
    }
    
    assert_4tech(i == size);
    
    if (start_ptr == start_end) goto buffer_measure_starts_zero_widths_end;
    *start_ptr++ = start;
    *width_ptr++ = 0;
    result = 0;
    
    buffer_measure_starts_zero_widths_end:
    state->i = i;
    state->count = (int)(start_ptr - buffer->line_starts);
    state->start = start;
    
    return(result);
}

internal_4tech int
buffer_measure_starts_widths(Buffer_Measure_Starts *state, Buffer_Type *buffer, float *advance_data){
    int result = 0;
    
    if (advance_data){
        result = buffer_measure_starts_widths_(state, buffer, advance_data);
    }
    else{
        result = buffer_measure_starts_zero_widths_(state, buffer);
    }
    
    return(result);
}

internal_4tech void
buffer_remeasure_starts(Buffer_Type *buffer, int line_start, int line_end, int line_shift, int text_shift){
    Buffer_Stringify_Type loop;
    int *starts = buffer->line_starts;
    int line_count = buffer->line_count;
    char *data = 0;
    int size = 0, end = 0;
    int line_i = 0, char_i = 0, start = 0;
    
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
    
    line_end += line_shift;
    size = buffer_size(buffer);
    char_i = starts[line_start];
    line_i = line_start;
    start = char_i;
    
    for (loop = buffer_stringify_loop(buffer, char_i, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; char_i < end; ++char_i){
            if (data[char_i] == '\n'){
                starts[line_i++] = start;
                start = char_i + 1;
                if (line_i >= line_end && start == starts[line_i]) goto buffer_remeasure_starts_end;
            }
        }
    }
    
    if (char_i == size){
        starts[line_i++] = start;
    }
    
    buffer_remeasure_starts_end:    
    assert_4tech(line_count >= 1);
    buffer->line_count = line_count;
}

internal_4tech void
buffer_remeasure_widths(Buffer_Type *buffer, float *advance_data,
                        int line_start, int line_end, int line_shift){
    Buffer_Stringify_Type loop;
    int *starts = buffer->line_starts;
    float *widths = buffer->line_widths;
    int line_count = buffer->line_count;
    int widths_count = buffer->widths_count;
    char *data = 0;
    int size = 0, end = 0;
    int i = 0, j = 0;
    float width = 0;
    char ch = 0;
    
    assert_4tech(0 <= line_start);
    assert_4tech(line_start <= line_end);
    assert_4tech(line_count <= buffer->widths_max);
    
    ++line_end;
    if (line_shift != 0){
        memmove_4tech(widths + line_end + line_shift, widths + line_end,
                      sizeof(float)*(widths_count - line_end));
    }
    buffer->widths_count = line_count;
    
    line_end += line_shift;    
    i = line_start;
    j = starts[i];
    
    if (line_end == line_count) size = buffer_size(buffer);
    else size = starts[line_end];
    
    width = 0;
    
    for (loop = buffer_stringify_loop(buffer, j, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        
        for (; j < end; ++j){
            ch = data[j];
            if (ch == '\n'){
                widths[i] = width;
                ++i;
                assert_4tech(j + 1 == starts[i]);
                width = 0;
            }
            else{
                width += measure_character(advance_data, ch);
            }
        }
    }
    
    if (j == buffer_size(buffer)){
        widths[i] = width;
        assert_4tech(i+1 == line_count);
    }
}

#if 0
inline_4tech void
buffer_measure_widths(Buffer_Type *buffer, void *advance_data){
    assert_4tech(buffer->line_count >= 1);
    buffer_remeasure_widths(buffer, advance_data, 0, buffer->line_count-1, 0);
}
#endif

internal_4tech void
buffer_measure_wrap_y(Buffer_Type *buffer, float *wraps,
                      float font_height, float max_width){
    float *widths;
    float y_pos;
    int i, line_count;
    
    line_count = buffer->line_count;
    widths = buffer->line_widths;
    y_pos = 0;
    
    for (i = 0; i < line_count; ++i){
        wraps[i] = y_pos;
        if (widths[i] == 0) y_pos += font_height;
        else y_pos += font_height*ceil_4tech(widths[i]/max_width);
    }
}

internal_4tech int
buffer_get_line_index_range(Buffer_Type *buffer, int pos, int l_bound, int u_bound){
    int *lines;
    int start, end;
    int i;
    
    assert_4tech(0 <= l_bound);
    assert_4tech(l_bound <= u_bound);
    assert_4tech(u_bound <= buffer->line_count);
    
    lines = buffer->line_starts;
    
    assert_4tech(lines != 0);
    
    start = l_bound;
    end = u_bound;
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

inline_4tech int
buffer_get_line_index(Buffer_Type *buffer, int pos){
    int result = buffer_get_line_index_range(buffer, pos, 0, buffer->line_count);
    return(result);
}

#ifndef NON_ABSTRACT_4TECH
internal_4tech int
buffer_get_line_index_from_wrapped_y(float *wraps, float y, float font_height, int l_bound, int u_bound){
    int start, end, i, result;
    start = l_bound;
    end = u_bound;
    for (;;){
        i = (start + end) / 2;
        if (wraps[i]+font_height <= y) start = i;
        else if (wraps[i] > y) end = i;
        else{
            result = i;
            break;
        }
        if (start >= end - 1){
            result = start;
            break;
        }
    }
    return(result);
}
#endif

#ifndef NON_ABSTRACT_4TECH
typedef struct Seek_State{
    Full_Cursor cursor;
    Full_Cursor prev_cursor;
} Seek_State;

internal_4tech int
cursor_seek_step(Seek_State *state, Buffer_Seek seek, int xy_seek, float max_width,
                 float font_height, float *advances, int size, char ch){
    Full_Cursor cursor, prev_cursor;
    float ch_width;
    int result;
    float x, px, y;
    
    cursor = state->cursor;
    prev_cursor = state->prev_cursor;
    
    result = 1;
    prev_cursor = cursor;
    switch (ch){
        case '\n':
        ++cursor.line;
        cursor.unwrapped_y += font_height;
        cursor.wrapped_y += font_height;
        cursor.character = 1;
        cursor.unwrapped_x = 0;
        cursor.wrapped_x = 0;
        break;
        
        default:
        ++cursor.character;
        if (ch == '\r') ch_width = *(float*)(advances + '\\') + *(float*)(advances + 'r');
        else ch_width = *(float*)(advances + ch);
        
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
        result = 0;
        goto cursor_seek_step_end;
    }
    
    x = y = px = 0;
    
    switch (seek.type){
        case buffer_seek_pos:
        if (cursor.pos > seek.pos){
            cursor = prev_cursor;
            result = 0;
            goto cursor_seek_step_end;
        }break;
        
        case buffer_seek_wrapped_xy:
        x = cursor.wrapped_x; px = prev_cursor.wrapped_x;
        y = cursor.wrapped_y; break;
        
        case buffer_seek_unwrapped_xy:
        x = cursor.unwrapped_x; px = prev_cursor.unwrapped_x;
        y = cursor.unwrapped_y; break;
        
        case buffer_seek_line_char:
        if (cursor.line == seek.line && cursor.character >= seek.character){
            result = 0;
            goto cursor_seek_step_end;
        }
        else if (cursor.line > seek.line){
            cursor = prev_cursor;
            result = 0;
            goto cursor_seek_step_end;
        }break;
    }
    
    if (xy_seek){
        if (y > seek.y){
            cursor = prev_cursor;
            result = 0;
            goto cursor_seek_step_end;
        }
        
        if (y > seek.y - font_height && x >= seek.x){
            if (!seek.round_down){
                if (ch != '\n' && (seek.x - px) < (x - seek.x)) cursor = prev_cursor;
                result = 0;
                goto cursor_seek_step_end;
            }
            
            if (x > seek.x){
                cursor = prev_cursor;
                result = 0;
                goto cursor_seek_step_end;
            }
        }
    }
    
    cursor_seek_step_end:
    state->cursor = cursor;
    state->prev_cursor = prev_cursor;
    return(result);
}
#endif

internal_4tech Full_Cursor
buffer_cursor_seek(Buffer_Type *buffer, Buffer_Seek seek, float max_width,
                   float font_height, float *advance_data, Full_Cursor cursor){
    Buffer_Stringify_Type loop;
    char *data;
    int size, end;
    int i;
    int result;
    
    Seek_State state;
    int xy_seek;
    
    state.cursor = cursor;
    
    switch(seek.type){
        case buffer_seek_pos:
        if (cursor.pos >= seek.pos) goto buffer_cursor_seek_end;
        break;
        
        case buffer_seek_wrapped_xy:
        if (seek.x == 0 && cursor.wrapped_y >= seek.y) goto buffer_cursor_seek_end;
        break;
        
        case buffer_seek_unwrapped_xy:
        if (seek.x == 0 && cursor.unwrapped_y >= seek.y) goto buffer_cursor_seek_end;
        break;
        
        case buffer_seek_line_char:
        if (cursor.line >= seek.line && cursor.character >= seek.character) goto buffer_cursor_seek_end;
        break;
    }
    
    if (advance_data){
        size = buffer_size(buffer);
        xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
        
        result = 1;
        i = cursor.pos;
        for (loop = buffer_stringify_loop(buffer, i, size);
             buffer_stringify_good(&loop);
             buffer_stringify_next(&loop)){
            end = loop.size + loop.absolute_pos;
            data = loop.data - loop.absolute_pos;
            for (; i < end; ++i){
                result = cursor_seek_step(&state, seek, xy_seek, max_width,
                                          font_height, advance_data, size, data[i]);
                if (!result) goto buffer_cursor_seek_end;
            }
        }
        if (result){
            result = cursor_seek_step(&state, seek, xy_seek, max_width,
                                      font_height, advance_data, size, 0);
            assert_4tech(result == 0);
        }
    }
    
    buffer_cursor_seek_end:    
    return(state.cursor);
}

internal_4tech Partial_Cursor
buffer_partial_from_pos(Buffer_Type *buffer, int pos){
    Partial_Cursor result = {0};
    
    int32_t size = buffer_size(buffer);
    if (pos > size){
        pos = size;
    }
    if (pos < 0){
        pos = 0;
    }
    
    int line_index = buffer_get_line_index_range(buffer, pos, 0, buffer->line_count);
    result.pos = pos;
    result.line = line_index+1;
    result.character = pos - buffer->line_starts[line_index] + 1;
    
    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_pos(Buffer_Type *buffer, int pos, float *wraps,
                       float max_width, float font_height, float *advance_data){
    Full_Cursor result;
    int line_index;
    
    int32_t size = buffer_size(buffer);
    if (pos > size){
        pos = size;
    }
    if (pos < 0){
        pos = 0;
    }
    
    line_index = buffer_get_line_index_range(buffer, pos, 0, buffer->line_count);
    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_pos(pos), max_width, font_height,
                                advance_data, result);
    
    return(result);
}

internal_4tech Partial_Cursor
buffer_partial_from_line_character(Buffer_Type *buffer, int line, int character){
    Partial_Cursor result = {0};
    
    int line_index = line - 1;
    if (line_index >= buffer->line_count) line_index = buffer->line_count - 1;
    if (line_index < 0) line_index = 0;
    
    int32_t size = buffer_size(buffer);
    int this_start = buffer->line_starts[line_index];
    int max_character = (size-this_start) + 1;
    if (line_index+1 < buffer->line_count){
        int next_start = buffer->line_starts[line_index+1];
        max_character = (next_start-this_start);
    }
    
    if (character <= 0) character = 1;
    if (character > max_character) character = max_character;
    
    result.pos = this_start + character - 1;
    result.line = line_index+1;
    result.character = character;
    
    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_line_character(Buffer_Type *buffer, int line, int character, float *wraps,
                                  float max_width, float font_height, float *advance_data){
    Full_Cursor result = {0};
    
    int line_index = line - 1;
    if (line_index >= buffer->line_count) line_index = buffer->line_count - 1;
    if (line_index < 0) line_index = 0;
    
    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_line_char(line, character),
                                max_width, font_height, advance_data, result);
    
    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_unwrapped_xy(Buffer_Type *buffer, float x, float y, int round_down, float *wraps,
                                float max_width, float font_height, float *advance_data){
    Full_Cursor result;
    int line_index;

    line_index = (int)(y / font_height);
    if (line_index >= buffer->line_count) line_index = buffer->line_count - 1;
    if (line_index < 0) line_index = 0;

    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_unwrapped_xy(x, y, round_down),
                                max_width, font_height, advance_data, result);

    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_wrapped_xy(Buffer_Type *buffer, float x, float y, int round_down, float *wraps,
                              float max_width, float font_height, float *advance_data){
    Full_Cursor result;
    int line_index;

    line_index = buffer_get_line_index_from_wrapped_y(wraps, y, font_height, 0, buffer->line_count);
    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_wrapped_xy(x, y, round_down),
                                max_width, font_height, advance_data, result);

    return(result);
}

internal_4tech void
buffer_invert_edit_shift(Buffer_Type *buffer, Buffer_Edit edit, Buffer_Edit *inverse, char *strings,
                         int *str_pos, int max, int shift_amount){
    int pos = *str_pos;
    int len = edit.end - edit.start;
    assert_4tech(pos >= 0);
    assert_4tech(pos + len <= max);
    *str_pos = pos + len;
    
    inverse->str_start = pos;
    inverse->len = len;
    inverse->start = edit.start + shift_amount;
    inverse->end = edit.start + edit.len + shift_amount;
    buffer_stringify(buffer, edit.start, edit.end, strings + pos);
}

inline_4tech void
buffer_invert_edit(Buffer_Type *buffer, Buffer_Edit edit, Buffer_Edit *inverse, char *strings,
                   int *str_pos, int max){
    buffer_invert_edit_shift(buffer, edit, inverse, strings, str_pos, max, 0);
}

#ifndef NON_ABSTRACT_4TECH
typedef struct Buffer_Invert_Batch{
    int i;
    int shift_amount;
    int len;
} Buffer_Invert_Batch;
#endif

internal_4tech int
buffer_invert_batch(Buffer_Invert_Batch *state, Buffer_Type *buffer, Buffer_Edit *edits, int count,
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

struct Buffer_Render_Options{
    b8 show_slash_t;
};

internal_4tech Full_Cursor
buffer_get_start_cursor(Buffer_Type *buffer, float *wraps, float scroll_y,
                        int wrapped, float width, float *advance_data, float font_height){
    Full_Cursor result;
    
    if (wrapped){
        result = buffer_cursor_from_wrapped_xy(buffer, 0, scroll_y, 0, wraps,
                                                     width, font_height, advance_data);
    }
    else{
        result = buffer_cursor_from_unwrapped_xy(buffer, 0, scroll_y, 0, wraps,
                                                       width, font_height, advance_data);
    }
    
    return(result);
}

#define BRFlag_Special_Character (1 << 0)

typedef struct Buffer_Render_Item{
    int index;
    unsigned short glyphid;
    unsigned short flags;
    float x0, y0;
    float x1, y1;
} Buffer_Render_Item;

inline_4tech void
write_render_item(Buffer_Render_Item *item,
                  int index,
                  unsigned short glyphid,
                  float x, float y,
                  float w, float h){
    item->index = index;
    item->glyphid = glyphid;
    item->x0 = x;
    item->y0 = y;
    item->x1 = x + w;
    item->y1 = y + h;
}

inline_4tech float
write_render_item_inline(Buffer_Render_Item *item,
                         int index,
                         unsigned short glyphid,
                         float x, float y,
                         float *advance_data, float h){
    float ch_width;
    ch_width = measure_character(advance_data, (char)glyphid);
    write_render_item(item, index, glyphid, x, y, ch_width, h);
    return(ch_width);
}

internal_4tech void
buffer_get_render_data(Buffer_Type *buffer, Buffer_Render_Item *items, int max, int *count,
                       float port_x, float port_y,
                       float scroll_x, float scroll_y, Full_Cursor start_cursor,
                       int wrapped,
                       float width, float height,
                       float *advance_data, float font_height,
                       Buffer_Render_Options opts){
    
    Buffer_Stringify_Type loop;
    Buffer_Render_Item *item;
    Buffer_Render_Item *item_end;
    char *data;
    int size, end;
    float shift_x, shift_y;
    float x, y;
    int i, item_i;
    float ch_width, ch_width_sub;
    char ch;
    
    size = buffer_size(buffer);
    
    shift_x = port_x - scroll_x;
    shift_y = port_y - scroll_y;
    if (wrapped) shift_y += start_cursor.wrapped_y;
    else shift_y += start_cursor.unwrapped_y;
    
    x = shift_x;
    y = shift_y;
    item_i = 0;
    item = items + item_i;
    item_end = items + max;
    
    // TODO(allen): What's the plan for when there is not enough space to store
    // more render items?  It seems like we should be able to use the view_x
    // to skip items that are not in view right?  That way I think it would
    // just always fit in the buffer.
    if (advance_data){
        for (loop = buffer_stringify_loop(buffer, start_cursor.pos, size);
             buffer_stringify_good(&loop) && item < item_end;
             buffer_stringify_next(&loop)){
            
            end = loop.size + loop.absolute_pos;
            data = loop.data - loop.absolute_pos;
            
            for (i = loop.absolute_pos; i < end; ++i){
                ch = data[i];
                ch_width = measure_character(advance_data, ch);
                
                if (ch_width + x > width + shift_x && wrapped && ch != '\n'){
                    x = shift_x;
                    y += font_height;
                }
                if (y > height + shift_y) goto buffer_get_render_data_end;
                
                switch (ch){
                    case '\n':
                    if (item < item_end){
                        write_render_item_inline(item, i, ' ', x, y, advance_data, font_height);
                        item->flags = 0;
                        ++item_i;
                        ++item;
                        
                        x = shift_x;
                        y += font_height;
                    }
                    break;
                    
                    case 0:
                    if (item < item_end){
                        ch_width = write_render_item_inline(item, i, '\\', x, y, advance_data, font_height);
                        item->flags = BRFlag_Special_Character;
                        ++item_i;
                        ++item;
                        x += ch_width;
                        
                        if (item < item_end){
                            ch_width = write_render_item_inline(item, i, '0', x, y, advance_data, font_height);
                            item->flags = BRFlag_Special_Character;
                            ++item_i;
                            ++item;
                            x += ch_width;
                        }
                    }
                    break;
                    
                    case '\r':
                    if (item < item_end){
                        ch_width = write_render_item_inline(item, i, '\\', x, y, advance_data, font_height);
                        item->flags = BRFlag_Special_Character;
                        ++item_i;
                        ++item;
                        x += ch_width;
                        
                        if (item < item_end){
                            ch_width = write_render_item_inline(item, i, 'r', x, y, advance_data, font_height);
                            item->flags = BRFlag_Special_Character;
                            ++item_i;
                            ++item;
                            x += ch_width;
                        }
                    }
                    break;
                    
                    case '\t':
                    if (opts.show_slash_t){
                        if (item < item_end){
                            ch_width_sub = write_render_item_inline(item, i, '\\', x, y, advance_data, font_height);
                            item->flags = BRFlag_Special_Character;
                            ++item_i;
                            ++item;
                            if (item < item_end){
                                write_render_item_inline(item, i, 't', x + ch_width_sub, y, advance_data, font_height);
                                item->flags = BRFlag_Special_Character;
                                ++item_i;
                                ++item;
                            }
                        }
                    }
                    else{
                        if (item < item_end){
                            write_render_item_inline(item, i, ' ', x, y, advance_data, font_height);
                            item->flags = 0;
                            ++item_i;
                            ++item;
                        }
                    }
                    x += ch_width;
                    break;
                    
                    default:
                    if (item < item_end){
                        write_render_item(item, i, ch, x, y, ch_width, font_height);
                        item->flags = 0;
                        ++item_i;
                        ++item;
                        x += ch_width;
                    }
                    break;
                }
                if (y > height + shift_y) goto buffer_get_render_data_end;
            }
        }
        
        buffer_get_render_data_end:
        if (y <= height + shift_y || item == items){
            if (item < item_end){
                ch = 0;
                ch_width = measure_character(advance_data, ' ');
                write_render_item(item, size, ch, x, y, ch_width, font_height);
                ++item_i;
                ++item;
                x += ch_width;
            }
        }
    }
    else{
        if (item < item_end){
            ch = 0;
            ch_width = 0;
            write_render_item(item, size, ch, x, y, ch_width, font_height);
            ++item_i;
            ++item;
            x += ch_width;
        }
    }
    
    // TODO(allen): handle this with a control state
    assert_4tech(item_i <= max);
    *count = item_i;
}

#ifndef NON_ABSTRACT_4TECH
#define NON_ABSTRACT_4TECH 1
#endif
// BOTTOM

