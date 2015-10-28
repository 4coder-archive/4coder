/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 24.10.2015
 * 
 * Buffer data object
 *  type - Golden Array
 * 
 */

// TOP

#define Buffer_Stringify_Type cat_4tech(Buffer_Type, _Stringify_Loop)
#define Buffer_Backify_Type cat_4tech(Buffer_Type, _Backify_Loop)

inline_4tech void
buffer_stringify(Buffer_Type *buffer, int start, int end, char *out){
    for (Buffer_Stringify_Type loop = buffer_stringify_loop(buffer, start, end, end - start);
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
    for (loop = buffer_stringify_loop(buffer, 0, size, size);
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
    assert_4tech(end < buffer_size(buffer));
    
    count = 0;

    for (loop = buffer_stringify_loop(buffer, start, end, end - start);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        for (i = 0; i < loop.size; ++i){
            count += (loop.data[i] == '\n');
        }
    }
    
    return(count);
}

internal_4tech int
buffer_seek_whitespace_down(Buffer_Type *buffer, int pos){
    Buffer_Stringify_Type loop;
    char *data;
    int end;
    int size;
    int no_hard;
    int prev_endline;
    
    size = buffer_size(buffer);
    loop = buffer_stringify_loop(buffer, pos, size, size);
    
    for (;buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (;pos < end; ++pos){
            if (!is_whitespace(data[pos])) goto buffer_seek_whitespace_down_mid;
        }
    }

buffer_seek_whitespace_down_mid:
    no_hard = 0;
    prev_endline = -1;
    for (;buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; pos < end; ++pos){
            if (data[pos] == '\n'){
                if (no_hard) goto buffer_seek_whitespace_down_end;
                else{
                    no_hard = 1;
                    prev_endline = pos;
                }
            }
            else if (!is_whitespace(data[pos])){
                no_hard = 0;
            }
        }
    }
    
buffer_seek_whitespace_down_end:
    if (prev_endline == -1 || prev_endline+1 >= size) pos = size;
    else pos = prev_endline+1;

    return pos;
}

internal_4tech int
buffer_seek_whitespace_up(Buffer_Type *buffer, int pos){
    Buffer_Backify_Type loop;
    char *data;
    int end;
    int size;
    int no_hard;
    
    size = buffer_size(buffer);
    loop = buffer_backify_loop(buffer, pos, 0, size);
    
    for (;buffer_backify_good(&loop);
         buffer_backify_next(&loop)){
        end = loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (;pos >= end && pos > 0; --pos){
            if (!is_whitespace(data[pos])) goto buffer_seek_whitespace_up_mid;
        }
    }

buffer_seek_whitespace_up_mid:
    no_hard = 0;
    for (;buffer_backify_good(&loop);
         buffer_backify_next(&loop)){
        end = loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; pos >= end && pos > 0; --pos){
            if (data[pos] == '\n'){
                if (no_hard) goto buffer_seek_whitespace_up_end;
                else no_hard = 1;
            }
            else if (!is_whitespace(data[pos])){
                no_hard = 0;
            }
        }
    }
    
buffer_seek_whitespace_up_end:
    if (pos != 0) ++pos;

    return pos;
}

internal_4tech int
buffer_seek_whitespace_right(Buffer_Type *buffer, int pos){
    Buffer_Stringify_Type loop;
    char *data;
    int end;
    int size;
    
    size = buffer_size(buffer);
    loop = buffer_stringify_loop(buffer, pos, size, size);
    
    for (;buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; pos < end && is_whitespace(data[pos]); ++pos);
        if (!is_whitespace(data[pos])) break;
    }
    
    for (;buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; pos < end && !is_whitespace(data[pos]); ++pos);
        if (is_whitespace(data[pos])) break;
    }
    
    return(pos);
}

internal_4tech int
buffer_seek_whitespace_left(Buffer_Type *buffer, int pos){
    Buffer_Backify_Type loop;
    char *data;
    int end;
    int size;

    --pos;
    if (pos > 0){
        size = buffer_size(buffer);
        loop = buffer_backify_loop(buffer, pos, 0, size);
    
        for (;buffer_backify_good(&loop);
             buffer_backify_next(&loop)){
            end = loop.absolute_pos;
            data = loop.data - loop.absolute_pos;
            for (; pos >= end && is_whitespace(data[pos]); --pos);
            if (!is_whitespace(data[pos])) break;
        }
    
        for (;buffer_backify_good(&loop);
             buffer_backify_next(&loop)){
            end = loop.absolute_pos;
            data = loop.data - loop.absolute_pos;
            for (; pos >= end && !is_whitespace(data[pos]); --pos);
            if (is_whitespace(data[pos])) break;
        }
        
        if (pos != 0) ++pos;
    }
    else{
        pos = 0;
    }
    
    return(pos);
}

internal_4tech int
buffer_seek_alphanumeric_right(Buffer_Type *buffer, int pos){
    Buffer_Stringify_Type loop;
    char *data;
    int end;
    int size;
    
    size = buffer_size(buffer);
    loop = buffer_stringify_loop(buffer, pos, size, size);
    
    for (;buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; pos < end && is_alphanumeric_true(data[pos]); ++pos);
        if (!is_alphanumeric_true(data[pos])) break;
    }
    
    for (;buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; pos < end && !is_alphanumeric_true(data[pos]); ++pos);
        if (is_alphanumeric_true(data[pos])) break;
    }
    
    return(pos);
}

internal_4tech int
buffer_seek_alphanumeric_left(Buffer_Type *buffer, int pos){
    Buffer_Backify_Type loop;
    char *data;
    int end;
    int size;
    
    --pos;
    if (pos >= 0){
        size = buffer_size(buffer);
        loop = buffer_backify_loop(buffer, pos, 0, size);
        
        for (;buffer_backify_good(&loop);
             buffer_backify_next(&loop)){
            end = loop.absolute_pos;
            data = loop.data - end;
            for (; pos >= end && !is_alphanumeric_true(data[pos]); --pos);
            if (is_alphanumeric_true(data[pos])) break;
        }
        
        for (;buffer_backify_good(&loop);
             buffer_backify_next(&loop)){
            end = loop.absolute_pos;
            data = loop.data - end;
            for (; pos >= end && is_alphanumeric_true(data[pos]); --pos);
            if (!is_alphanumeric_true(data[pos])) break;
        }
        
        ++pos;
    }
    else{
        pos = 0;
    }
    
    return(pos);
}

internal_4tech int
buffer_seek_alphanumeric_or_camel_right(Buffer_Type *buffer, int pos, int an_pos){
    Buffer_Stringify_Type loop;
    char *data;
    int end, size;
    char ch, prev_ch;

    size = buffer_size(buffer);
    assert_4tech(pos < an_pos);
    assert_4tech(an_pos < size);

    ++pos;
    if (pos < an_pos){
        loop = buffer_stringify_loop(buffer, pos, an_pos, size);
        if (buffer_stringify_good(&loop)){
            prev_ch = loop.data[0];
            ++pos;
            
            for (;buffer_stringify_good(&loop);
                 buffer_stringify_next(&loop)){
                end = loop.size + loop.absolute_pos;
                data = loop.data - loop.absolute_pos;
                for (; pos < end; ++pos){
                    ch = data[pos];
                    if (is_upper(ch) && is_lower(prev_ch)) goto buffer_seek_alphanumeric_or_camel_right_end;
                    prev_ch = ch;
                }
            }
        }
    }
    else{
        pos = an_pos;
    }
    
buffer_seek_alphanumeric_or_camel_right_end:
    return(pos);
}

internal_4tech int
buffer_seek_alphanumeric_or_camel_left(Buffer_Type *buffer, int pos, int an_pos){
    Buffer_Backify_Type loop;
    char *data;
    int end, size;
    char ch, prev_ch;

    size = buffer_size(buffer);
    assert_4tech(an_pos < pos);
    assert_4tech(0 <= an_pos);
    
    loop = buffer_backify_loop(buffer, pos, an_pos, size);
    if (buffer_backify_good(&loop)){
        prev_ch = loop.data[0];
        --pos;
        
        for (;buffer_backify_good(&loop);
             buffer_backify_next(&loop)){
            end = loop.absolute_pos;
            data = loop.data - loop.absolute_pos;
            for (; pos >= end && pos > 0; --pos){
                ch = data[pos];
                if (is_upper(ch) && is_lower(prev_ch)) goto buffer_seek_alphanumeric_or_camel_left_end;
                prev_ch = ch;
            }
        }
    }
    
buffer_seek_alphanumeric_or_camel_left_end:
    return(pos);
}

typedef struct{
    int i;
    int count;
    int start;
} Buffer_Measure_Starts;

internal_4tech int
buffer_measure_starts(Buffer_Measure_Starts *state, Buffer_Type *buffer){
    Buffer_Stringify_Type loop;
    int *starts;
    int max;
    char *data;
    int size, end;
    int start, count, i;
    int result;
    
    size = buffer_size(buffer);
    starts = buffer->line_starts;
    max = buffer->line_max;
    
    result = 0;
    
    i = state->i;
    count = state->count;
    start = state->start;
    
    for (loop = buffer_stringify_loop(buffer, i, size, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; i < end; ++i){
            if (data[i] == '\n'){
                if (count == max){
                    result = 1;
                    goto buffer_measure_starts_end;
                }
                
                starts[count++] = start;
                start = i + 1;
            }
        }
    }
    
    if (i == size){
        starts[count++] = start;
    }
    
buffer_measure_starts_end:
    state->i = i;
    state->count = count;
    state->start = start;
    
    return(result);
}

internal_4tech void
buffer_remeasure_starts(Buffer_Type *buffer, int line_start, int line_end, int line_shift, int text_shift){
    Buffer_Stringify_Type loop;
    int *starts;
    int line_count;
    char *data;
    int size, end;
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
    
    line_end += line_shift;
    size = buffer_size(buffer);
    char_i = starts[line_start];
    line_i = line_start;
    start = char_i;

    for (loop = buffer_stringify_loop(buffer, char_i, size, size);
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
buffer_remeasure_widths(Buffer_Type *buffer, void *advance_data, int stride,
                        int line_start, int line_end, int line_shift){
    Buffer_Stringify_Type loop;
    int *starts;
    float *widths;
    int line_count;
    char *data;
    int size, end;
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
    }
    
    line_end += line_shift;    
    i = line_start;
    j = starts[i];

    if (line_end == line_count) size = buffer_size(buffer);
    else size = starts[line_end];
    
    width = 0;
    
    for (loop = buffer_stringify_loop(buffer, j, size, size);
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
                width += measure_character(advance_data, stride, ch);
            }
        }
    }
}

inline_4tech void
buffer_measure_widths(Buffer_Type *buffer, void *advance_data, int stride){
    assert_4tech(buffer->line_count >= 1);
    buffer_remeasure_widths(buffer, advance_data, stride, 0, buffer->line_count-1, 0);
}

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
    int result;
    result = buffer_get_line_index_range(buffer, pos, 0, buffer->line_count);
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

typedef struct{
    Full_Cursor cursor, prev_cursor;
} Seek_State;

internal_4tech int
cursor_seek_step(Seek_State *state, Buffer_Seek seek, int xy_seek, float max_width, float font_height,
                 char *advances, int stride, int size, char ch){
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
buffer_cursor_seek(Buffer_Type *buffer, Buffer_Seek seek, float max_width, float font_height,
                   void *advance_data, int stride, Full_Cursor cursor){
    Buffer_Stringify_Type loop;
    char *data;
    int size, end;
    int i;
    int result;
    
    Seek_State state;
    char *advances;
    int xy_seek;

    size = buffer_size(buffer);
    advances = (char*)advance_data;
    xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
    state.cursor = cursor;
    
    result = 1;
    i = cursor.pos;
    for (loop = buffer_stringify_loop(buffer, i, size, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        for (; i < end && result; ++i){
            result = cursor_seek_step(&state, seek, xy_seek, max_width, font_height,
                                      advances, stride, size, data[i]);
        }
    }
    if (result){
        result = cursor_seek_step(&state, seek, xy_seek, max_width, font_height,
                                  advances, stride, size, 0);
        assert_4tech(result == 0);
    }
    
    return(state.cursor);
}

internal_4tech Full_Cursor
buffer_cursor_from_pos(Buffer_Type *buffer, int pos, float *wraps,
                       float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    int line_index;

    line_index = buffer_get_line_index_range(buffer, pos, 0, buffer->line_count);
    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_pos(pos), max_width, font_height,
                                advance_data, stride, result);

    return(result);
}

internal_4tech Full_Cursor
buffer_cursor_from_unwrapped_xy(Buffer_Type *buffer, float x, float y, int round_down, float *wraps,
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
buffer_cursor_from_wrapped_xy(Buffer_Type *buffer, float x, float y, int round_down, float *wraps,
                              float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    int line_index;

    line_index = buffer_get_line_index_from_wrapped_y(wraps, y, font_height, 0, buffer->line_count);
    result = make_cursor_hint(line_index, buffer->line_starts, wraps, font_height);
    result = buffer_cursor_seek(buffer, seek_wrapped_xy(x, y, round_down), max_width, font_height,
                                advance_data, stride, result);

    return(result);
}

internal_4tech void
buffer_get_render_data(Buffer_Type *buffer, float *wraps, Buffer_Render_Item *items, int max, int *count,
                       float port_x, float port_y, float scroll_x, float scroll_y, int wrapped,
                       float width, float height, void *advance_data, int stride, float font_height){
    Buffer_Stringify_Type loop;
    Full_Cursor start_cursor;
    Buffer_Render_Item *item;
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
    
    x = shift_x;
    y = shift_y;
    item_i = 0;
    item = items + item_i;
    
    for (loop = buffer_stringify_loop(buffer, start_cursor.pos, size, size);
         buffer_stringify_good(&loop);
         buffer_stringify_next(&loop)){
        
        end = loop.size + loop.absolute_pos;
        data = loop.data - loop.absolute_pos;
        
        for (i = loop.absolute_pos; i < end; ++i){
            ch = data[i];
            ch_width = measure_character(advance_data, stride, ch);
            
            if (ch_width + x > width + shift_x && wrapped){
                x = shift_x;
                y += font_height;
            }
            if (y > height + shift_y) goto buffer_get_render_data_end;
            
            switch (ch){
            case '\n':
                write_render_item_inline(item, i, ' ', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                
                x = shift_x;
                y += font_height;
                break;

            case 0:
                ch_width = write_render_item_inline(item, i, '\\', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;

                ch_width = write_render_item_inline(item, i, '0', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;

            case '\r':
                ch_width = write_render_item_inline(item, i, '\\', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;

                ch_width = write_render_item_inline(item, i, 'r', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;

            case '\t':
                ch_width_sub = write_render_item_inline(item, i, '\\', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;

                write_render_item_inline(item, i, 't', x + ch_width_sub, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;

            default:
                write_render_item(item, i, ch, x, y, ch_width, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;
            }
            if (y > height + shift_y) goto buffer_get_render_data_end;
        }
    }
    
buffer_get_render_data_end:
    // TODO(allen): handle this with a control state
    assert_4tech(item_i <= max);
    *count = item_i;
}

#ifndef NON_ABSTRACT_4TECH
#define NON_ABSTRACT_4TECH 1
#endif
// BOTTOM

