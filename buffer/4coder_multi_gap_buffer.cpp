/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 30.10.2015
 * 
 * Buffer data object
 *  type - Multi Gap Buffer
 *
 * This scheme was originally introduced to me by Martin Cohen,
 * who calls it a "Fixed Width Gap Buffer".
 * 
 */

// TOP

typedef struct{
    char *data;
    int size1, gap_size, size2;
    int start_pos;
} Fixed_Width_Gap_Buffer;

#define fixed_width_buffer_size Kbytes(8)
#define fixed_width_buffer_half_size Kbytes(4)

typedef struct{
    Fixed_Width_Gap_Buffer *gaps;
    int chunk_count;
    int chunk_max;
    int size;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int line_max;
    int widths_max;
} Multi_Gap_Buffer;

inline_4tech int
buffer_good(Multi_Gap_Buffer *buffer){
    int good;
    good = (buffer->gaps != 0);
    return(good);
}

inline_4tech int
buffer_size(Multi_Gap_Buffer *buffer){
    int size;
    size = buffer->size;
    return(size);
}

typedef struct{
    Multi_Gap_Buffer *buffer;
    char *data;
    int size;
    int chunk_i;
    int chunk_count;
} Multi_Gap_Buffer_Init;


internal_4tech Multi_Gap_Buffer_Init
buffer_begin_init(Multi_Gap_Buffer *buffer, char *data, int size){
    Multi_Gap_Buffer_Init init;
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    init.chunk_i = 0;
    init.chunk_count = div_ceil_4tech(size, fixed_width_buffer_half_size);
    return(init);
}

internal_4tech int
buffer_init_need_more(Multi_Gap_Buffer_Init *init){
    int result;
    result = 1;
    if (init->buffer->gaps && init->chunk_i == init->chunk_count)
        result = 0;
    return(result);
}

internal_4tech int
buffer_init_page_size(Multi_Gap_Buffer_Init *init){
    Multi_Gap_Buffer *buffer;
    int result;
    buffer = init->buffer;
    if (buffer->gaps) result = fixed_width_buffer_size;
    else result = init->chunk_count * 2 * sizeof(*buffer->gaps);
    return(result);
}

internal_4tech void
buffer_init_provide_page(Multi_Gap_Buffer_Init *init, void *page, int page_size){
    Multi_Gap_Buffer *buffer;
    buffer = init->buffer;
    
    if (buffer->gaps){
        assert_4tech(page_size >= fixed_width_buffer_size);
        buffer->gaps[init->chunk_i].data = (char*)page;
        ++init->chunk_i;
    }
    else{
        buffer->gaps = (Fixed_Width_Gap_Buffer*)page;
        buffer->chunk_max = page_size / sizeof(*buffer->gaps);
    }
}

internal_4tech int
buffer_end_init(Multi_Gap_Buffer_Init *init){
    Multi_Gap_Buffer *buffer;
    Fixed_Width_Gap_Buffer *gap;
    int result;
    int i, count;
    char *data;
    int pos, size, total_size, start_pos;
    int osize1, size1, size2;

    result = 0;
    buffer = init->buffer;
    if (buffer->gaps){
        if (buffer->chunk_max >= div_ceil_4tech(init->size, fixed_width_buffer_half_size)){
            buffer->chunk_count = init->chunk_count;
            result = 1;
            
            data = init->data;
            total_size = init->size;
            gap = buffer->gaps;
            count = init->chunk_count;
            size = fixed_width_buffer_half_size;
            pos = 0;
            start_pos = 0;
            
            for (i = 0; i < count; ++i, ++gap, pos += size){
                if (pos + size > total_size) size = total_size - pos;
                
                if (gap->data){
                    size2 = size >> 1;
                    size1 = osize1 = size - size2;
                    
                    if (size1 > 0){
                        size1 = eol_convert_in(gap->data, data + pos, size1);
                        if (size2 > 0){
                            size2 = eol_convert_in(gap->data + size1, data + pos + osize1, size2);
                        }
                    }
                    
                    gap->size1 = size1;
                    gap->size2 = size2;
                    gap->gap_size = fixed_width_buffer_size - size1 - size2;
                    memmove_4tech(gap->data + size1 + gap->gap_size, gap->data + size1, size2);

                    gap->start_pos = start_pos;
                    start_pos += size1 + size2;
                }
                else{
                    result = 0;
                    break;
                }
            }
            buffer->size = start_pos;
        }
    }
    
    return(result);
}

internal_4tech int
buffer_find_chunk(Multi_Gap_Buffer *buffer, int pos){
    Fixed_Width_Gap_Buffer *gaps;
    int start, end, m, this_pos;
    
    gaps = buffer->gaps;
    start = 0;
    end = buffer->chunk_count;
    for(;;){
        m = (start + end) / 2;
        this_pos = gaps[m].start_pos;
        if (this_pos < pos) start = m;
        else if (this_pos > pos) end = m;
        else break;
        if (start+1 == end){
            m = start; break;
        }
        assert_4tech(start < end);
    }
    return(m);
}

typedef struct{
    Multi_Gap_Buffer *buffer;
    Fixed_Width_Gap_Buffer *gaps;
    char *data;
    int absolute_pos;
    int size;
    int chunk_i;
    int chunk_end;
    int pos, end;
} Multi_Gap_Buffer_Stringify_Loop;

internal_4tech Multi_Gap_Buffer_Stringify_Loop
buffer_stringify_loop(Multi_Gap_Buffer *buffer, int start, int end){
    Multi_Gap_Buffer_Stringify_Loop result;
    Fixed_Width_Gap_Buffer *gap;
    int temp_end;
    
    if (0 <= start && start < end && end <= buffer->size){
        result.buffer = buffer;
        result.gaps = buffer->gaps;
        result.absolute_pos = start;
        
        result.chunk_i = buffer_find_chunk(buffer, start);
        result.chunk_end = buffer_find_chunk(buffer, end-1);
        
        gap = result.gaps + result.chunk_end;
        end -= gap->start_pos;
        if (end < gap->size1) result.end = end;
        else result.end = end + gap->gap_size;
        
        gap = result.gaps + result.chunk_i;
        start -= gap->start_pos;
        if (start < gap->size1){
            result.pos = start;
            temp_end = gap->size1;
        }
        else{
            result.pos = start + gap->gap_size;
            temp_end = fixed_width_buffer_size;
        }
        
        if (result.chunk_i == result.chunk_end && temp_end > result.end) temp_end = result.end;
        result.size = temp_end - result.pos;
        result.data = gap->data + result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_stringify_good(Multi_Gap_Buffer_Stringify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_stringify_next(Multi_Gap_Buffer_Stringify_Loop *loop){
    Fixed_Width_Gap_Buffer *gap;
    int temp_end;

    gap = loop->gaps + loop->chunk_i;
    if (loop->chunk_i == loop->chunk_end && loop->pos + loop->size == loop->end){
        loop->buffer = 0;
    }
    else{
        if (loop->pos < gap->size1){
            loop->pos = gap->size1 + gap->gap_size;
            loop->absolute_pos = gap->start_pos + gap->size1;
            temp_end = fixed_width_buffer_size;
        }
        else{
            ++loop->chunk_i;
            ++gap;
            loop->pos = 0;
            loop->absolute_pos = gap->start_pos;
            temp_end = gap->size1;
        }
        if (loop->chunk_i == loop->chunk_end && temp_end > loop->end) temp_end = loop->end;
        loop->size = temp_end - loop->pos;
        loop->data = gap->data + loop->pos;
    }
}

typedef struct{
    Multi_Gap_Buffer *buffer;
    Fixed_Width_Gap_Buffer *gaps;
    char *data;
    int absolute_pos;
    int size;
    int chunk_i;
    int chunk_end;
    int pos, end;
} Multi_Gap_Buffer_Backify_Loop;

internal_4tech Multi_Gap_Buffer_Backify_Loop
buffer_backify_loop(Multi_Gap_Buffer *buffer, int start, int end){
    Multi_Gap_Buffer_Backify_Loop result;
    Fixed_Width_Gap_Buffer *gap;
    int temp_end, temp_start;
    
    ++start;
    if (0 <= end && end < start && start <= buffer->size){
        result.buffer = buffer;
        result.gaps = buffer->gaps;
        
        result.chunk_i = buffer_find_chunk(buffer, start);
        result.chunk_end = buffer_find_chunk(buffer, end);
        
        gap = result.gaps + result.chunk_end;
        end -= gap->start_pos;
        if (end < gap->size1) result.end = end;
        else result.end = end + gap->gap_size;
        
        gap = result.gaps + result.chunk_i;
        start -= gap->start_pos;
        if (start < gap->size1){
            temp_end = start;
            temp_start = 0;
        }
        else{
            temp_end = start + gap->gap_size;
            temp_start = gap->size1 + gap->gap_size;
        }
        
        if (result.chunk_i == result.chunk_end && temp_start < result.end) temp_start = result.end;
        result.pos = temp_start;
        result.absolute_pos = temp_start + gap->start_pos;
        if (temp_start >= gap->size1) result.absolute_pos -= gap->gap_size;
        result.size = temp_end - temp_start;
        result.data = gap->data + result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_backify_good(Multi_Gap_Buffer_Backify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_backify_next(Multi_Gap_Buffer_Backify_Loop *loop){
    Fixed_Width_Gap_Buffer *gap;
    int temp_end, temp_start;
    
    gap = loop->gaps + loop->chunk_i;
    if (loop->chunk_i == loop->chunk_end && loop->pos == loop->end){
        loop->buffer = 0;
    }
    else{
        if (loop->pos < gap->size1){
            --gap;
            --loop->chunk_i;
            temp_start = gap->size1 + gap->gap_size;
            temp_end = fixed_width_buffer_size;
        }
        else{
            temp_start = 0;
            temp_end = gap->size1;
        }
        if (loop->chunk_i == loop->chunk_end && temp_start < loop->end) temp_start = loop->end;
        loop->absolute_pos = temp_start + gap->start_pos;
        if (temp_start >= gap->size1) loop->absolute_pos -= gap->gap_size;
        loop->pos = temp_start;
        loop->size = temp_end - temp_start;
        loop->data = gap->data + loop->pos;
    }
}


// BOTTOM


