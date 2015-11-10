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

typedef struct Fixed_Width_Gap_Buffer{
    char *data;
    int size1, gap_size, size2;
    int start_pos;
} Fixed_Width_Gap_Buffer;

#define fixed_width_buffer_size (8 << 10)
#define fixed_width_buffer_half_size (4 << 10)

typedef struct Multi_Gap_Buffer{
    Fixed_Width_Gap_Buffer *gaps;
    int chunk_count;
    int chunk_alloced;
    int chunk_max;
    int size;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int widths_count;
    int line_max;
    int widths_max;
    
    int grow_gaps;
    int edit_stage;
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

typedef struct Multi_Gap_Buffer_Init{
    Multi_Gap_Buffer *buffer;
    char *data;
    int size;
    int chunk_i;
    int chunk_count;
    int chunk_alloc;
} Multi_Gap_Buffer_Init;

internal_4tech Multi_Gap_Buffer_Init
buffer_begin_init(Multi_Gap_Buffer *buffer, char *data, int size){
    Multi_Gap_Buffer_Init init;
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    init.chunk_i = 0;
    init.chunk_alloc = div_ceil_4tech(size, fixed_width_buffer_half_size);
    init.chunk_count = init.chunk_alloc;
    if (init.chunk_alloc < 4) init.chunk_alloc = 4;
    return(init);
}

inline_4tech int
buffer_init_need_more(Multi_Gap_Buffer_Init *init){
    int result;
    result = 1;
    if (init->buffer->gaps && init->chunk_i == init->chunk_alloc)
        result = 0;
    return(result);
}

inline_4tech int
buffer_init_page_size(Multi_Gap_Buffer_Init *init){
    Multi_Gap_Buffer *buffer;
    int result;
    buffer = init->buffer;
    if (buffer->gaps) result = fixed_width_buffer_size;
    else result = init->chunk_alloc * 2 * sizeof(*buffer->gaps);
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
buffer_end_init(Multi_Gap_Buffer_Init *init, void *scratch, int scratch_size){
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
            if (buffer->chunk_count == 0) buffer->chunk_count = 1;
            buffer->chunk_alloced = init->chunk_alloc;
            result = 1;
            
            data = init->data;
            total_size = init->size;
            gap = buffer->gaps;
            count = buffer->chunk_count;
            size = fixed_width_buffer_half_size;
            pos = 0;
            start_pos = 0;
            
            for (i = 0; i < count; ++i, ++gap, pos += size){
                assert_4tech(size == fixed_width_buffer_half_size);
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
        else{
            --m;
            if (m < 0) m = 0;
            break;
        }
        if (start+1 == end){
            m = start; break;
        }
        assert_4tech(start < end);
    }
    
    return(m);
}

typedef struct Multi_Gap_Buffer_Stringify_Loop{
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
        result.chunk_end = buffer_find_chunk(buffer, end);
        
        gap = result.gaps + result.chunk_end;
        end -= gap->start_pos;
        if (end <= gap->size1) result.end = end;
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
            if (gap->size1 == 0){
                loop->pos = gap->gap_size;
                temp_end = fixed_width_buffer_size;
            }
        }
        if (loop->chunk_i == loop->chunk_end && temp_end > loop->end) temp_end = loop->end;
        loop->size = temp_end - loop->pos;
        loop->data = gap->data + loop->pos;
    }
}

typedef struct Multi_Gap_Buffer_Backify_Loop{
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

internal_4tech int
buffer_replace_range(Multi_Gap_Buffer *buffer, int start, int end, char *str, int len, int *shift_amount_out,
                     void *scratch, int scratch_size, int *request_amount){
    Fixed_Width_Gap_Buffer *gaps, *gap, *dgap;
    char *data;
    int move_size;
    int gap_start, gap_end;
    int result;
    int size;
    int local_end;
    int shift_amount;
    int required_empty_buffers;
    int supplanted_gaps;
    int dpos;
    int head_size, middle_size, tail_size;
    int mem_pos, local_start_pos;
    debug_4tech(int osize);
    
    size = buffer_size(buffer);
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end <= size);

    *shift_amount_out = (len - (end - start));
    
    gaps = buffer->gaps;
    gap_start = buffer_find_chunk(buffer, start);
    gap_end = buffer_find_chunk(buffer, end);
    
    if (buffer->edit_stage == 0){
        buffer->size += *shift_amount_out;
        for (gap = gaps + gap_end + 1;
             gap < gaps + buffer->chunk_count;
             ++gap){
            gap->start_pos += *shift_amount_out;
        }
        buffer->edit_stage = 1;
    }
    
    gap = gaps + gap_start;
    if (buffer->edit_stage == 1){
        if (gap_start < gap_end && gap_start+1 < buffer->chunk_count){
            supplanted_gaps = gap_end - gap_start - 1;
            if (buffer->chunk_max - buffer->chunk_alloced >= supplanted_gaps){
                ++gap;
                memcpy_4tech(gaps + buffer->chunk_alloced, gap, sizeof(*gaps)*supplanted_gaps);
                memmove_4tech(gap, gaps + gap_end, sizeof(*gaps)*(buffer->chunk_alloced - gap_start - 1));
                buffer->chunk_count -= (gap_end - gap_start - 1);
        
                local_end = end - gap->start_pos;
        
                if (local_end >= gap->size1){
                    gap->size2 -= (local_end - gap->size1);
                    gap->size1 = 0;
                }
                else{
                    memmove_4tech(gap->data, gap->data + local_end, gap->size1 - local_end);
                    gap->size1 -= local_end;
                }
                gap->gap_size = fixed_width_buffer_size - gap->size2 - gap->size1;
        
                --gap;
            }
            else{
                buffer->grow_gaps = 1;
                *request_amount = round_up_4tech(sizeof(*gaps)*(supplanted_gaps+buffer->chunk_max*2), 4<<10);
                result = 1;
                goto mugab_replace_range_end;
            }
        }
        buffer->edit_stage = 2;
    }
    
    start -= gap->start_pos;
    end -= gap->start_pos;
    assert_4tech(start >= 0 && end >= 0);
    if (end > gap->size1 + gap->size2) end = gap->size1 + gap->size2;
    shift_amount = (len - (end - start));
    
    if (shift_amount + gap->size1 + gap->size2 <= fixed_width_buffer_size){
        data = gap->data;
        debug_4tech(osize = gap->size1 + gap->size2);
        if (end < gap->size1){
            move_size = gap->size1 - end;
            memmove_4tech(data + gap->size1 + gap->gap_size - move_size, data + end, move_size);
            gap->size1 -= move_size;
            gap->size2 += move_size;
        }
        if (start > gap->size1){
            move_size = start - gap->size1;
            memmove_4tech(data + gap->size1, data + gap->size1 + gap->gap_size, move_size);
            gap->size1 += move_size;
            gap->size2 -= move_size;
        }
        
        memcpy_4tech(data + start, str, len);
        gap->size2 = fixed_width_buffer_size - (end + gap->gap_size);
        gap->size1 = start + len;
        gap->gap_size -= shift_amount;
        
        assert_4tech(gap->size1 + gap->size2 == osize + shift_amount);
        assert_4tech(gap->size1 + gap->gap_size + gap->size2 == fixed_width_buffer_size);
        
        result = 0;
        buffer->edit_stage = 0;

        if (gap_start < gap_end){
            ++gap;
            gap->start_pos += shift_amount;
        }
    }
    else{
        required_empty_buffers = div_ceil_4tech(shift_amount, fixed_width_buffer_half_size);
        if (buffer->chunk_alloced - buffer->chunk_count >= required_empty_buffers){
            if (buffer->chunk_max - buffer->chunk_alloced >= required_empty_buffers){
                memcpy_4tech(gaps + buffer->chunk_alloced, gaps + buffer->chunk_count, sizeof(*gaps)*required_empty_buffers);
                memmove_4tech(gap + required_empty_buffers + 1, gap + 1, sizeof(*gaps)*(buffer->chunk_count - gap_start - 1));
                memcpy_4tech(gap + 1, gaps + buffer->chunk_alloced, sizeof(*gaps)*required_empty_buffers);
                
                data = gap->data;
                if (end < gap->size1){
                    move_size = gap->size1 - end;
                    memmove_4tech(data + gap->size1 + gap->gap_size - move_size, data + end, move_size);
                    gap->size1 -= move_size + (end - start);
                    gap->size2 += move_size;
                }
                else if (start > gap->size1){
                    move_size = start - gap->size1;
                    memmove_4tech(data + gap->size1, data + gap->size1 + gap->gap_size, move_size);
                    gap->size1 += move_size;
                    gap->size2 -= move_size + (end - start);
                }
                else{
                    if (end > gap->size1){
                        gap->size2 -= (end - gap->size1);
                    }
                    gap->size1 = start;
                }
                
                dgap = gap + required_empty_buffers;
                dpos = gap->size1 + gap->gap_size;
                memcpy_4tech(dgap->data + dpos, data + dpos, gap->size2);
                dgap->size2 = gap->size2;
                gap->size2 = 0;
                
                tail_size = fixed_width_buffer_half_size - dgap->size2;
                if (tail_size < 0) tail_size = 0;

                if (tail_size < len){
                    middle_size = div_ceil_4tech(len - tail_size, (required_empty_buffers * 2));
                
                    head_size = middle_size;
                
                    if (head_size + gap->size1 + 256 > fixed_width_buffer_size){
                        head_size = fixed_width_buffer_size - gap->size1 - 256;
                        if (head_size < 0) head_size = 0;
                    }

                    if (required_empty_buffers-1 > 0){
                        middle_size = div_ceil_4tech(len - head_size - tail_size, (required_empty_buffers-1)*2);
                    }
                    else{
                        middle_size = 0;
                        head_size = len - tail_size;
                        assert_4tech(head_size + gap->size1 <= fixed_width_buffer_size);
                    }
                }
                else{
                    middle_size = 0;
                    head_size = 0;
                }

                mem_pos = 0;
                if (head_size > len - mem_pos) head_size = len - mem_pos;
                
                gap->size2 = head_size;
                gap->gap_size = fixed_width_buffer_size - gap->size1 - gap->size2;
                memcpy_4tech(gap->data + fixed_width_buffer_size - head_size, str + mem_pos, head_size);
                mem_pos += head_size;
                local_start_pos = gap->start_pos + gap->size1 + gap->size2;
                
                ++gap;
                for (;gap < dgap; ++gap){
                    gap->start_pos = local_start_pos;
                    
                    if (middle_size > len - mem_pos) middle_size = len - mem_pos;
                    gap->size1 = middle_size;
                    memcpy_4tech(gap->data, str + mem_pos, middle_size);
                    mem_pos += middle_size;
                    
                    if (middle_size > len - mem_pos) middle_size = len - mem_pos;
                    gap->size2 = middle_size;
                    memcpy_4tech(gap->data + fixed_width_buffer_size - middle_size, str + mem_pos, middle_size);
                    mem_pos += middle_size;
                    
                    gap->gap_size = fixed_width_buffer_size - (gap->size1 + gap->size2);
                    local_start_pos += gap->size1 + gap->size2;
                }
                
                if (tail_size > len - mem_pos) tail_size = len - mem_pos;
                gap->start_pos = local_start_pos;
                gap->size1 = tail_size;
                gap->gap_size = fixed_width_buffer_size - gap->size1 - gap->size2;
                memcpy_4tech(gap->data, str + mem_pos, tail_size);
                mem_pos += tail_size;
                assert_4tech(mem_pos == len);
                debug_4tech(local_start_pos += gap->size1 + gap->size2);
                //assert_4tech(local_start_pos == buffer->size);

                buffer->chunk_count += required_empty_buffers;

                result = 0;
                buffer->edit_stage = 0;
            }
            else{
                buffer->grow_gaps = 1;
                *request_amount = round_up_4tech(sizeof(*gaps)*(required_empty_buffers+buffer->chunk_max*2), 4<<10);
                result = 1;
                goto mugab_replace_range_end;
            }
        }
        else{
            if (buffer->chunk_alloced < buffer->chunk_max){
                *request_amount = fixed_width_buffer_size;
                result = 1;
            }
            else{
                buffer->grow_gaps = 1;
                *request_amount = round_up_4tech(sizeof(*gaps)*(1+buffer->chunk_max*2), 4<<10);
                result = 1;
            }
        }
    }

mugab_replace_range_end:
    return(result);
}

internal_4tech int
buffer_mugab_check(Multi_Gap_Buffer *buffer){
    Fixed_Width_Gap_Buffer *gap;
    int result;
    int total_size;
    int i, count;
    
    assert_4tech(buffer->chunk_count <= buffer->chunk_alloced);
    assert_4tech(buffer->chunk_alloced <= buffer->chunk_max);
    
    count = buffer->chunk_count;
    
    total_size = 0;
    gap = buffer->gaps;
    for (i = 0; i < count; ++i, ++gap){
        assert_4tech(gap->size1 + gap->size2 + gap->gap_size == fixed_width_buffer_size);
        assert_4tech(gap->start_pos == total_size);
        total_size += gap->size1 + gap->size2;
    }
    assert_4tech(total_size == buffer->size);
    
    assert_4tech(buffer->edit_stage == 0);
    assert_4tech(buffer->grow_gaps == 0);
    
    result = 1;
    return(result);
}

// NOTE(allen): This could should be optimized for Multi_Gap_Buffer
internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Multi_Gap_Buffer *buffer, Buffer_Edit *sorted_edits,
                       char *strings, int edit_count, void *scratch, int scratch_size, int *request_amount){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_amount;
    
    result = 0;
    shift_total = state->shift_total;
    i = state->i;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      strings + edit->str_start, edit->len, &shift_amount,
                                      scratch, scratch_size, request_amount);
        if (result) break;
        buffer_mugab_check(buffer);
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

internal_4tech void*
buffer_edit_provide_memory(Multi_Gap_Buffer *buffer, void *new_data, int size){
    void *result;
    Fixed_Width_Gap_Buffer *gap;

    if (buffer->grow_gaps){
        assert_4tech(size >= buffer->chunk_max*sizeof(*buffer->gaps));
        
        result = buffer->gaps;
        memcpy_4tech(new_data, buffer->gaps, buffer->chunk_alloced*sizeof(*buffer->gaps));
        buffer->gaps = (Fixed_Width_Gap_Buffer*)new_data;
        buffer->chunk_max = size / sizeof(*buffer->gaps);
        buffer->grow_gaps = 0;
    }
    
    else{
        assert_4tech(buffer->chunk_max > buffer->chunk_alloced);
        assert_4tech(size >= fixed_width_buffer_size);
        
        gap = &buffer->gaps[buffer->chunk_alloced++];
        memzero_4tech(*gap);
        gap->data = (char*)new_data;
        result = 0;
    }
    
    return(result);
}

// BOTTOM


