/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.01.2017
 *
 * The 4coder base buffer data structure.
 *
 */

// TOP

//
// Buffer low level operations
//

internal void
write_cursor_with_index(Cursor_With_Index *positions, i32 *count, i64 pos){
    positions[*count].index = *count;
    positions[*count].pos = pos;
    ++(*count);
}

internal void
buffer_quick_sort_cursors(Cursor_With_Index *positions, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot = one_past_last - 1;
        i64 pivot_pos = positions[pivot].pos;
        i32 j = first;
        for (i32 i = first; i < pivot; i += 1){
            i64 pos = positions[i].pos;
            if (pos < pivot_pos){
                Swap(Cursor_With_Index, positions[j], positions[i]);
                j += 1;
            }
        }
        Swap(Cursor_With_Index, positions[j], positions[pivot]);
        buffer_quick_sort_cursors(positions, first, j);
        buffer_quick_sort_cursors(positions, j + 1, one_past_last);
    }
}

internal void
buffer_sort_cursors(Cursor_With_Index *positions, i32 count){
    if (count > 0){
        buffer_quick_sort_cursors(positions, 0, count);
    }
}

internal void
buffer_unsort_cursors(Cursor_With_Index *positions, i32 count){
    if (count > 0){
        i32 i = 0;
        for (;;){
            if (positions[i].index == i){
                i += 1;
                if (i >= count){
                    break;
                }
            }
            else{
                i32 j = positions[i].index;
                Swap(Cursor_With_Index, positions[i], positions[j]);
            }
        }
    }
}

#if 0
function void
buffer_sort_batch(Edit *batch, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot = one_past_last - 1;
        i64 pivot_pos = batch[pivot].range.first;
        i32 j = first;
        for (i32 i = first; i < pivot; i += 1){
            i64 pos = batch[i].range.first;
            if (pos < pivot_pos){
                Swap(Edit, batch[j], batch[i]);
                j += 1;
            }
        }
        Swap(Edit, batch[j], batch[pivot]);
        buffer_sort_batch(batch, first, j);
        buffer_sort_batch(batch, j + 1, one_past_last);
    }
}

function Edit_Array
buffer_batch_array_from_linked_list(Arena *arena, Batch_Edit *batch, i32 count){
    Edit_Array result = {};
    result.count = count;
    result.vals = push_array(arena, Edit, count);
    i32 counter = 0;
    for (Batch_Edit *node = batch;
         counter < count && node != 0;
         node = node->next){
        result.vals[counter] = node->edit;
        counter += 1;
    }
    return(result);
}

function Edit_Array
buffer_sort_batch(Arena *arena, Batch_Edit *batch, i32 count){
    Edit_Array result = buffer_batch_array_from_linked_list(arena, batch, count);
    buffer_sort_batch(result.vals, 0, result.count);
    return(result);
}
#endif

internal void
buffer_update_cursors_lean_l(Cursor_With_Index *sorted_positions, i32 count,
                             Batch_Edit *batch){
    Cursor_With_Index *pos = sorted_positions;
    Cursor_With_Index *end_pos = sorted_positions + count;
    i64 shift_amount = 0;
    for (; batch != 0 && pos < end_pos;
         batch = batch->next){
        Range_i64 range = batch->edit.range;
        i64 len = batch->edit.text.size;
        if (shift_amount != 0){
            for (;pos < end_pos && pos->pos < range.first; pos += 1){
                pos->pos += shift_amount;
            }
        }
        else{
            for (;pos < end_pos && pos->pos < range.first; pos += 1);
        }
        i64 new_pos = range.first + shift_amount;
        for (;pos < end_pos && pos->pos <= range.one_past_last; pos += 1){
            pos->pos = new_pos;
        }
        shift_amount += len - (range.one_past_last - range.first);
    }
    if (shift_amount != 0){
        for (;pos < end_pos; pos += 1){
            pos->pos += shift_amount;
        }
    }
}

internal void
buffer_update_cursors_lean_r(Cursor_With_Index *sorted_positions, i32 count,
                             Batch_Edit *batch){
    Cursor_With_Index *pos = sorted_positions;
    Cursor_With_Index *end_pos = sorted_positions + count;
    i64 shift_amount = 0;
    for (; batch != 0 && pos < end_pos;
         batch = batch->next){
        Range_i64 range = batch->edit.range;
        i64 len = batch->edit.text.size;
        if (shift_amount != 0){
            for (;pos < end_pos && pos->pos < range.first; pos += 1){
                pos->pos += shift_amount;
            }
        }
        else{
            for (;pos < end_pos && pos->pos < range.first; pos += 1);
        }
        i64 new_pos = range.first + len + shift_amount;
        for (;pos < end_pos && pos->pos < range.one_past_last; pos += 1){
            pos->pos = new_pos;
        }
        shift_amount += len - (range.one_past_last - range.first);
    }
    if (shift_amount != 0){
        for (;pos < end_pos; pos += 1){
            pos->pos += shift_amount;
        }
    }
}

//////////////////////////////////////

internal i32
eol_convert_in(char *dest, char *src, i32 size){
    i32 i = 0;
    i32 j = 0;
    i32 k = 0;
    
    for (; j < size && src[j] != '\r'; ++j);
    block_copy(dest, src, j);
    
    if (j < size){
        k = 1;
        ++j;
        for (i = j; i < size; ++i){
            if (src[i] == '\r'){
                block_copy(dest + j - k, src + j, i - j);
                ++k;
                j = i+1;
            }
        }
        block_copy(dest + j - k, src + j, i - j);
        j = i - k;
    }
    
    return(j);
}

internal i32
eol_in_place_convert_in(char *data, i32 size){
    i32 i = 0;
    i32 j = 0;
    i32 k = 0;
    
    for (; j < size && data[j] != '\r'; ++j);
    
    if (j < size){
        k = 1;
        ++j;
        for (i = j; i < size; ++i){
            if (data[i] == '\r'){
                block_copy(data + j - k, data + j, i - j);
                ++k;
                j = i+1;
            }
        }
        block_copy(data + j - k, data + j, i - j);
        j = i - k;
    }
    
    return(j);
}

// TODO(allen): iterative memory check?
internal b32
eol_convert_out(char *dest, i64 max, char *src, i64 size, i64 *size_out){
    i64 j = 0;
    for (i64 i = 0; i < size; ++i, ++j){
        if (src[i] == '\n'){
            dest[j] = '\r';
            ++j;
            dest[j] = '\n';
        }
        else{
            dest[j] = src[i];
        }
    }
    *size_out = j;
    return(true);
}

// TODO(allen): iterative memory check?
internal i32
eol_in_place_convert_out(char *data, i32 size, i32 max, i32 *size_out){
    i32 result = 1;
    i32 i = 0;
    
    for (; i < size; ++i){
        if (data[i] == '\n'){
            block_copy(data + i + 1, data + i, size - i);
            data[i] = '\r';
            ++i;
            ++size;
        }
    }
    
    *size_out = size;
    return(result);
}

//////////////////////////////////////

internal b32
buffer_good(Gap_Buffer *buffer){
    return(buffer->data != 0);
}

internal i64
buffer_size(Gap_Buffer *buffer){
    return(buffer->size1 + buffer->size2);
}

internal i64
buffer_line_count(Gap_Buffer *buffer){
    return(buffer->line_start_count - 1);
}

internal void
buffer_init(Gap_Buffer *buffer, u8 *data, umem size, Base_Allocator *allocator){
    block_zero_struct(buffer);
    
    buffer->allocator = allocator;
    
    umem capacity = round_up_umem(size*2, KB(4));
    Data memory = base_allocate(allocator, capacity);
    buffer->data = (u8*)memory.data;
    buffer->size1 = size/2;
    buffer->gap_size = capacity - size;
    buffer->size2 = size - buffer->size1;
    buffer->max = capacity;
    
    block_copy(buffer->data, data, buffer->size1);
    block_copy(buffer->data + buffer->size1 + buffer->gap_size, data + buffer->size1, buffer->size2);
}

internal b32
buffer_replace_range(Gap_Buffer *buffer, Range_i64 range, String_Const_u8 text, i64 shift_amount){
    i64 size = buffer_size(buffer);
    Assert(0 <= range.start);
    Assert(range.start <= range.end);
    Assert(range.end <= size);
    
    if (shift_amount + size > buffer->max){
        i64 new_max = round_up_i64(2*(shift_amount + size), KB(4));
        i64 new_gap_size = new_max - size;
        Data new_memory_data = base_allocate(buffer->allocator, new_max);
        u8 *new_memory = (u8*)new_memory_data.data;
        block_copy(new_memory, buffer->data, buffer->size1);
        block_copy(new_memory + buffer->size1 + new_gap_size, buffer->data + buffer->size1 + buffer->gap_size,
                   buffer->size2);
        base_free(buffer->allocator, buffer->data);
        buffer->data = new_memory;
        buffer->gap_size = new_gap_size;
        buffer->max = new_max;
    }
    
    Assert(shift_amount + size <= buffer->max);
    
    b32 result = false;
    
    if (range.end < buffer->size1){
        i64 move_size = buffer->size1 - range.end;
        block_copy(buffer->data + buffer->size1 + buffer->gap_size - move_size,
                   buffer->data + range.end,
                   move_size);
        buffer->size1 -= move_size;
        buffer->size2 += move_size;
    }
    if (range.start > buffer->size1){
        i64 move_size = range.start - buffer->size1;
        block_copy(buffer->data + buffer->size1,
                   buffer->data + buffer->size1 + buffer->gap_size,
                   move_size);
        buffer->size1 += move_size;
        buffer->size2 -= move_size;
    }
    
    block_copy(buffer->data + range.start, text.str, text.size);
    buffer->size2 = size - range.end;
    buffer->size1 = range.start + text.size;
    buffer->gap_size -= shift_amount;
    
    Assert(buffer->size1 + buffer->size2 == size + shift_amount);
    Assert(buffer->size1 + buffer->gap_size + buffer->size2 == buffer->max);
    
    return(result);
}

////////////////////////////////

internal List_String_Const_u8
buffer_get_chunks(Arena *arena, Gap_Buffer *buffer){
    List_String_Const_u8 list = {};
    if (buffer->size1 > 0){
        string_list_push(arena, &list, SCu8(buffer->data, buffer->size1));
    }
    if (buffer->size2 > 0){
        umem gap_2_pos = buffer->size1 + buffer->gap_size;
        string_list_push(arena, &list, SCu8(buffer->data + gap_2_pos, buffer->size2));
    }
    return(list);
}

internal void
buffer_chunks_clamp(List_String_Const_u8 *chunks, Interval_i64 range){
    i64 p = 0;
    List_String_Const_u8 list = {};
    for (Node_String_Const_u8 *node = chunks->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        Interval_i64 node_range = Ii64(p, p + node->string.size);
        if (range_overlap(range, node_range)){
            i64 first = max(node_range.first, range.first) - node_range.first;
            i64 one_past_last = min(node_range.one_past_last, range.one_past_last) - node_range.first;
            String_Const_u8 s = string_prefix(node->string, one_past_last);
            node->string = string_skip(s, first);
            sll_queue_push(list.first, list.last, node);
            list.total_size += node->string.size;
            list.node_count += 1;
        }
        p = node_range.one_past_last;
    }
    *chunks = list;
}

internal String_Const_u8
buffer_stringify(Arena *arena, Gap_Buffer *buffer, Interval_i64 range){
    List_String_Const_u8 list = buffer_get_chunks(arena, buffer);
    buffer_chunks_clamp(&list, range);
    return(string_list_flatten(arena, list, StringFill_NullTerminate));
}

internal String_Const_u8
buffer_eol_convert_out(Arena *arena, Gap_Buffer *buffer, Interval_i64 range){
    List_String_Const_u8 list = buffer_get_chunks(arena, buffer);
    buffer_chunks_clamp(&list, range);
    umem cap = list.total_size*2;
    u8 *memory = push_array(arena, u8, cap);
    u8 *memory_opl = memory + cap;
    u8 *ptr = memory;
    for (Node_String_Const_u8 *node = list.first;
         node != 0;
         node = node->next){
        u8 *byte = node->string.str;
        u8 *byte_opl = byte + node->string.size;
        for (;byte < byte_opl; byte += 1){
            if (*byte == '\n'){
                *ptr = '\r';
                ptr += 1;
                *ptr = '\n';
                ptr += 1;
            }
            else{
                *ptr = *byte;
                ptr += 1;
            }
        }
    }
    linalloc_pop(arena, (memory_opl - ptr));
    push_align(arena, 8);
    return(SCu8(memory, ptr));
}

#if 0
internal i64
buffer_count_newlines(Arena *scratch, Gap_Buffer *buffer, i64 start, i64 end){
    Temp_Memory temp = begin_temp(scratch);
    List_String_Const_u8 list = buffer_get_chunks(scratch, buffer);
    buffer_chunks_clamp(&list, Ii64(start, end));
    
    i64 count = 0;
    for (Node_String_Const_u8 *node = list.first;
         node != 0;
         node = node->next){
        u8 *byte = node->string.str;
        u8 *byte_opl = byte + node->string.size;
        for (;byte < byte_opl; byte += 1){
            if (*byte == '\n'){
                count += 1;
            }
        }
    }
    
    end_temp(temp);
    
    return(count);
}
#endif

internal void
buffer_starts__ensure_max_size(Gap_Buffer *buffer, i64 max_size){
    if (max_size > buffer->line_start_max){
        i64 new_max = round_up_i64(max_size*2, KB(1));
        Data memory = base_allocate(buffer->allocator, sizeof(*buffer->line_starts)*new_max);
        i64 *new_line_starts = (i64*)memory.data;
        block_copy_dynamic_array(new_line_starts, buffer->line_starts, buffer->line_start_count);
        buffer->line_start_max = new_max;
        base_free(buffer->allocator, buffer->line_starts);
        buffer->line_starts = new_line_starts;
    }
}

internal void
buffer_measure_starts__write(Gap_Buffer *buffer, i64 pos){
    buffer_starts__ensure_max_size(buffer, buffer->line_start_count + 1);
    buffer->line_starts[buffer->line_start_count] = pos;
    buffer->line_start_count += 1;
}

internal void
buffer_measure_starts(Arena *scratch, Gap_Buffer *buffer){
    Temp_Memory temp = begin_temp(scratch);
    List_String_Const_u8 list = buffer_get_chunks(scratch, buffer);
    buffer->line_start_count = 0;
    buffer_measure_starts__write(buffer, 0);
    i64 index = 0;
    for (Node_String_Const_u8 *node = list.first;
         node != 0;
         node = node->next){
        u8 *byte = node->string.str;
        u8 *byte_opl = byte + node->string.size;
        for (;byte < byte_opl; byte += 1){
            index += 1;
            if (*byte == '\n'){
                buffer_measure_starts__write(buffer, index);
            }
        }
    }
    buffer_measure_starts__write(buffer, buffer_size(buffer));
    end_temp(temp);
}

internal i64
buffer_get_line_index(Gap_Buffer *buffer, i64 pos){
    i64 i = 0;
    if (buffer->line_start_count > 2){
        i64 start = 0;
        i64 one_past_last = buffer->line_start_count - 1;
        i64 *array = buffer->line_starts;
        pos = clamp_bot(0, pos);
        for (;;){
            i = (start + one_past_last) >> 1;
            if (array[i] < pos){
                start = i;
            }
            else if (array[i] > pos){
                one_past_last = i;
            }
            else{
                break;
            }
            if (start + 1 >= one_past_last){
                i = start;
                break;
            }
        }
    }
    return(i);
}

Line_Move*
push_line_move(Arena *arena, Line_Move *moves, i64 new_line_first,
               i64 old_line_first, i64 old_line_opl, i64 text_shift){
    Line_Move *move = push_array(arena, Line_Move, 1);
    move->next = moves;
    move->kind = LineMove_ShiftOldValues;
    move->new_line_first = new_line_first;
    move->old_line_first = old_line_first;
    move->old_line_opl = old_line_opl;
    move->text_shift = text_shift;
    return(move);
}

Line_Move*
push_line_move(Arena *arena, Line_Move *moves, i64 new_line_first,
               String_Const_u8 string, i64 text_base){
    Line_Move *move = push_array(arena, Line_Move, 1);
    move->next = moves;
    move->kind = LineMove_MeasureString;
    move->new_line_first = new_line_first;
    move->string = string;
    move->text_base = text_base;
    return(move);
}

function i64
count_lines(String_Const_u8 string){
    i64 result = 0;
    for (umem i = 0; i < string.size; i += 1){
        if (string.str[i] == '\n'){
            result += 1;
        }
    }
    return(result);
}

function void
fill_line_starts(i64 *lines_starts, String_Const_u8 string, i64 text_base){
    i64 *ptr = lines_starts;
    for (umem i = 0; i < string.size; i += 1){
        if (string.str[i] == '\n'){
            *ptr = text_base + i + 1;
            ptr += 1;
        }
    }
}

function void
buffer_remeasure_starts(Thread_Context *tctx, Gap_Buffer *buffer, Batch_Edit *batch){
    Scratch_Block scratch(tctx);
    
    i64 line_start_count = buffer_line_count(buffer) + 1;
    
    Line_Move *moves = 0;
    i64 current_line = 0;
    i64 text_shift = 0;
    i64 line_shift = 0;
    for (Batch_Edit *node = batch;
         node != 0;
         node = node->next){
        i64 first_line = buffer_get_line_index(buffer, node->edit.range.first);
        i64 opl_line = buffer_get_line_index(buffer, node->edit.range.one_past_last);
        i64 new_line_count = count_lines(node->edit.text);
        i64 deleted_line_count = opl_line - first_line;
        
        Assert(first_line <= opl_line);
        Assert(opl_line <= line_start_count);
        
        if (current_line <= first_line &&
            (text_shift != 0 || line_shift != 0)){
            moves = push_line_move(scratch, moves, current_line + line_shift,
                                   current_line, first_line + 1, text_shift);
        }
        
        if (new_line_count != 0){
            moves = push_line_move(scratch, moves, first_line + 1 + line_shift,
                                   node->edit.text, node->edit.range.first + text_shift);
        }
        
        text_shift += node->edit.text.size - range_size(node->edit.range);
        line_shift += new_line_count - deleted_line_count;
        current_line = opl_line + 1;
    }
    
    moves = push_line_move(scratch, moves, current_line + line_shift,
                           current_line, line_start_count, text_shift);
    line_start_count = line_start_count + line_shift;
    
    buffer_starts__ensure_max_size(buffer, line_start_count + 1);
    buffer->line_start_count = line_start_count;
    
    i64 *array = buffer->line_starts;
    
    for (Line_Move *node = moves;
         node != 0;
         node = node->next){
        if (node->kind == LineMove_ShiftOldValues){
            i64 line_index_shift = node->new_line_first - node->old_line_first;
            i64 move_text_shift = node->text_shift;
            if (line_index_shift > 0){
                for (i64 i = node->old_line_opl - 1;
                     i >= node->old_line_first;
                     i -= 1){
                    array[i + line_index_shift] = array[i] + move_text_shift;
                }
            }
            else{
                for (i64 i = node->old_line_first;
                     i < node->old_line_opl;
                     i += 1){
                    array[i + line_index_shift] = array[i] + move_text_shift;
                }
            }
        }
    }
    
    for (Line_Move *node = moves;
         node != 0;
         node = node->next){
        if (node->kind == LineMove_MeasureString){
            fill_line_starts(array + node->new_line_first, node->string, node->text_base);
        }
    }
}

#if 0
internal void
buffer_remeasure_starts(Arena *scratch, Gap_Buffer *buffer,
                        Range_i64 old_line_indexes, i64 line_shift, i64 text_shift){
    Temp_Memory temp = begin_temp(scratch);
    buffer_starts__ensure_max_size(buffer, buffer->line_start_count + line_shift);
    
    i64 new_line_indexes_opl = old_line_indexes.one_past_last + line_shift;
    i64 old_line_start_count = buffer->line_start_count;
    i64 *line_start_ptr = buffer->line_starts + old_line_indexes.one_past_last;
    for (i64 i = old_line_indexes.one_past_last;
         i < old_line_start_count;
         i += 1, line_start_ptr += 1){
        *line_start_ptr += text_shift;
    }
    block_copy_dynamic_array(buffer->line_starts + new_line_indexes_opl,
                             buffer->line_starts + old_line_indexes.one_past_last,
                             buffer->line_start_count - old_line_indexes.one_past_last);
    
    i64 first_pos = buffer->line_starts[old_line_indexes.first];
    i64 write_counter = old_line_indexes.first + 1;
    i64 pos = first_pos;
    List_String_Const_u8 list = buffer_get_chunks(scratch, buffer);
    buffer_chunks_clamp(&list, Ii64(first_pos, buffer_size(buffer)));
    for (Node_String_Const_u8 *node = list.first;
         node != 0;
         node = node->next){
        u8 *byte = node->string.str;
        u8 *byte_opl = byte + node->string.size;
        for (;byte < byte_opl; byte += 1){
            pos += 1;
            if (*byte == '\n'){
                buffer->line_starts[write_counter] = pos;
                write_counter += 1;
                if (write_counter == new_line_indexes_opl){
                    goto double_break;
                }
            }
        }
    }
    double_break:;
    
    buffer->line_start_count += line_shift;
    end_temp(temp);
}
#endif

internal Interval_i64
buffer_get_pos_range_from_line_number(Gap_Buffer *buffer, i64 line_number){
    Interval_i64 result = {};
    if (1 <= line_number && line_number < buffer->line_start_count){
        result.first = buffer->line_starts[line_number - 1];
        result.one_past_last = buffer->line_starts[line_number];
    }
    return(result);
}

internal i64
buffer_get_first_pos_from_line_number(Gap_Buffer *buffer, i64 line_number){
    i64 result = {};
    if (line_number < 1){
        result = 0;
    }
    else if (line_number >= buffer->line_start_count){
        result = buffer_size(buffer);
    }
    else{
        result = buffer->line_starts[line_number - 1];
    }
    return(result);
}

internal i64
buffer_get_last_pos_from_line_number(Gap_Buffer *buffer, i64 line_number){
    i64 result = {};
    if (line_number < 1){
        result = 0;
    }
    else if (line_number >= buffer->line_start_count - 1){
        result = buffer_size(buffer);
    }
    else{
        result = buffer->line_starts[line_number] - 1;
    }
    return(result);
}

internal Buffer_Cursor
buffer_cursor_from_pos(Gap_Buffer *buffer, i64 pos){
    i64 size = buffer_size(buffer);
    pos = clamp(0, pos, size);
    i64 line_index = buffer_get_line_index(buffer, pos);
    
    Buffer_Cursor result = {};
    result.pos = pos;
    result.line = line_index + 1;
    result.col = pos - buffer->line_starts[line_index] + 1;
    return(result);
}

internal Buffer_Cursor
buffer_cursor_from_line_col(Gap_Buffer *buffer, i64 line, i64 col){
    i64 size = buffer_size(buffer);
    i64 line_index = line - 1;
    i64 line_count = buffer_line_count(buffer);
    line_index = clamp(0, line_index, line_count - 1);
    
    i64 this_start = buffer->line_starts[line_index];
    i64 max_col = (buffer->line_starts[line_index + 1] - this_start);
    if (line_index + 1 == line_count){
        max_col += 1;
    }
    max_col = clamp_bot(1, max_col);
    
    if (col < 0){
        if (-col > max_col){
            col = 1;
        }
        else{
            col = max_col + col + 1;
        }
    }
    else if (col == 0){
        col = 1;
    }
    else{
        col = clamp_top(col, max_col);
    }
    Assert(col > 0);
    i64 adjusted_pos = col - 1;
    
    i64 pos = this_start + adjusted_pos;
    
    Buffer_Cursor result = {};
    result.pos = pos;
    result.line = line_index + 1;
    result.col = col;
    return(result);
}

internal String_Const_u8
buffer_invert_edit_shift(Arena *arena, Gap_Buffer *buffer, Edit edit, Edit *inv, i64 shift_amount){
    String_Const_u8 string = buffer_stringify(arena, buffer, edit.range);
    inv->text = string;
    inv->range = Ii64(edit.range.start + shift_amount, edit.range.start + edit.text.size + shift_amount);
    return(string);
}

internal b32
buffer_invert_batch(Arena *arena, Gap_Buffer *buffer, Edit *edits, Edit *inverse, i64 count){
    b32 result = false;
    i64 pos = 0;
    i64 shift_amount = 0;
    Edit *edit = edits;
    Edit *inv_edit = inverse;
    for (i64 i = 0; i < count; i += 1, edit += 1, inv_edit += 1){
        String_Const_u8 inv_str = buffer_invert_edit_shift(arena, buffer, *edit, inv_edit, shift_amount);
        shift_amount += replace_range_shift(edit->range, edit->text.size);
        pos += inv_str.size;
    }
    return(result);
}

internal Buffer_Chunk_Position
buffer_get_chunk_position(String_Const_u8_Array chunks, i64 buffer_size, i64 real_pos){
    Buffer_Chunk_Position pos = {};
    pos.real_pos = real_pos;
    pos.chunk_pos = real_pos;
    if (pos.real_pos != buffer_size){
        for (;(imem)(chunks.vals[pos.chunk_index].size) <= pos.chunk_pos;){
            Assert(pos.chunk_index < chunks.count);
            pos.chunk_pos -= (i32)chunks.vals[pos.chunk_index].size;
            pos.chunk_index += 1;
        }
    }
    else{
        pos.chunk_index = chunks.count - 1;
        pos.chunk_pos = (i32)chunks.vals[pos.chunk_index].size;
    }
    return(pos);
}

internal i32
buffer_chunk_position_iterate(String_Const_u8_Array chunks, Buffer_Chunk_Position *pos, Scan_Direction direction){
    i32 past_end = 0;
    pos->real_pos += direction;
    pos->chunk_pos += direction;
    if (pos->chunk_pos < 0){
        if (pos->chunk_index == 0){
            past_end = -1;
        }
        else{
            pos->chunk_index -= 1;
            pos->chunk_pos = (i32)chunks.vals[pos->chunk_index].size - 1;
        }
    }
    else if (pos->chunk_pos >= (imem)(chunks.vals[pos->chunk_index].size)){
        pos->chunk_index += 1;
        if (pos->chunk_index == chunks.count){
            past_end = 1;
        }
        else{
            pos->chunk_pos = 0;
        }
    }
    return(past_end);
}

////////////////////////////////

internal void
buffer_layout__write(Arena *arena, Buffer_Layout_Item_List *list, i64 index, u32 codepoint,
                     Buffer_Layout_Flag flags, Rect_f32 rect){
    Temp_Memory restore_point = begin_temp(arena);
    Buffer_Layout_Item *item = push_array(arena, Buffer_Layout_Item, 1);
    
    Buffer_Layout_Item_Block *block = list->first;
    if (block != 0){
        if (block->items + block->count == item){
            block->count += 1;
        }
        else{
            block = 0;
        }
    }
    if (block == 0){
        end_temp(restore_point);
        block = push_array(arena, Buffer_Layout_Item_Block, 1);
        item = push_array(arena, Buffer_Layout_Item, 1);
        sll_queue_push(list->first, list->last, block);
        list->node_count += 1;
        block->items = item;
        block->count = 1;
    }
    list->total_count += 1;
    
    if (index > list->index_range.max){
        block->character_count += 1;
        list->character_count += 1;
        list->index_range.max = index;
    }
    
    item->index = index;
    item->codepoint = codepoint;
    item->flags = flags;
    item->rect = rect;
    list->height = max(list->height, rect.y1);
}

internal Buffer_Layout_Item_List
buffer_layout(Thread_Context *tctx, Arena *arena, Gap_Buffer *buffer, Interval_i64 range, Face *face, f32 width){
    Scratch_Block scratch(tctx);
    
    Buffer_Layout_Item_List list = {};
    list.index_range.first = range.first;
    list.index_range.one_past_last = range.first - 1;
    
    List_String_Const_u8 chunks = buffer_get_chunks(scratch, buffer);
    buffer_chunks_clamp(&chunks, range);
    
    f32 line_height = face->line_height;
    f32 text_height = face->text_height;
    f32 line_to_text_shift = text_height - line_height;
    f32 space_advance = face->space_advance;
    
    if (chunks.node_count == 0){
        f32 next_x = space_advance;
        buffer_layout__write(arena, &list, range.first, ' ', 0, 
                             Rf32(V2(0.f, 0.f), V2f32(next_x, text_height)));
    }
    else{
        Vec2_f32 p = {};
        f32 line_y = line_height;
        f32 text_y = text_height;
        
        String_Const_u8 text = {};
        if (chunks.node_count == 1){
            text = chunks.first->string;
        }
        else{
            text = string_list_flatten(scratch, chunks);
        }
        
        i64 index = range.first;
        b32 first_of_the_line = true;
        
        b32 consuming_newline_characters = false;
        i64 newline_character_index = -1;
        
        b32 prev_did_emit_newline = false;
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        for (;ptr < end_ptr;){
            Character_Consume_Result consume = utf8_consume(ptr, (umem)(end_ptr - ptr));
            u32 render_codepoint = consume.codepoint;
            b32 emit_newline = false;
            switch (consume.codepoint){
                case '\t':
                {
                    render_codepoint = ' ';
                }//fallthrough;
                default:
                {
                    f32 advance = font_get_glyph_advance(face, consume.codepoint);
                    f32 next_x = p.x + advance;
                    if (!first_of_the_line && next_x >= width){
                        p.y = line_y;
                        p.x = 0.f;
                        line_y += line_height;
                        text_y = line_y + line_to_text_shift;
                        next_x = p.x + advance;
                    }
                    buffer_layout__write(arena, &list, index, render_codepoint, 0, 
                                         Rf32(p, V2f32(next_x, text_y)));
                    p.x = next_x;
                    ptr += consume.inc;
                    index += consume.inc;
                    first_of_the_line = false;
                }break;
                
                case '\r':
                {
                    if (!consuming_newline_characters){
                        consuming_newline_characters = true;
                        newline_character_index = index;
                    }
                    ptr += 1;
                    index += 1;
                }break;
                
                case '\n':
                {
                    if (!consuming_newline_characters){
                        consuming_newline_characters = true;
                        newline_character_index = index;
                    }
                    emit_newline = true;
                    ptr += 1;
                    index += 1;
                }break;
                
                case max_u32:
                {
                    f32 next_x = p.x + face->byte_advance;
                    if (!first_of_the_line && next_x >= width){
                        p.y = line_y;
                        p.x = 0.f;
                        line_y += line_height;
                        text_y = line_y + line_to_text_shift;
                        next_x = p.x + face->byte_advance;
                    }
                    u32 v = *ptr;
                    u32 lo = v&0xF;
                    u32 hi = (v >> 4)&0xF;
                    f32 advance = face->byte_sub_advances[0];
                    buffer_layout__write(arena, &list, index, '\\', 0,
                                         Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                    advance = face->byte_sub_advances[1];
                    buffer_layout__write(arena, &list, index, integer_symbols[lo], 0,
                                         Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                    face->byte_sub_advances[2];
                    buffer_layout__write(arena, &list, index, integer_symbols[hi], 0,
                                         Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x = next_x;
                    ptr += 1;
                    index += 1;
                    first_of_the_line = false;
                }break;
            }
            prev_did_emit_newline = false;
            if (emit_newline){
                f32 next_x = p.x + space_advance;
                buffer_layout__write(arena, &list, newline_character_index, ' ', 0,
                                     Rf32(p, V2f32(next_x, text_y)));
                p.y = line_y;
                p.x = 0.f;
                line_y += line_height;
                text_y = line_y + line_to_text_shift;
                first_of_the_line = true;
                prev_did_emit_newline = true;
            }
        }
        if (!prev_did_emit_newline){
            f32 next_x = p.x + space_advance;
            buffer_layout__write(arena, &list, index, ' ', 0,  Rf32(p, V2f32(next_x, text_y)));
        }
    }
    list.bottom_extension = -line_to_text_shift;
    list.height += list.bottom_extension;
    
    return(list);
}

internal i64
buffer_layout_nearest_pos_to_xy(Buffer_Layout_Item_List list, Vec2_f32 p){
    i64 closest_match = 0;
    if (p.y < 0.f){
        closest_match = list.index_range.min;
    }
    else if (p.y >= list.height){
        closest_match = list.index_range.max + 1;
    }
    else{
        if (0.f < p.x && p.x < max_f32){
            f32 bottom_extension = list.bottom_extension;
            f32 closest_x = -max_f32;
            for (Buffer_Layout_Item_Block *block = list.first;
                 block != 0;
                 block = block->next){
                i64 count = block->count;
                Buffer_Layout_Item *item = block->items;
                for (i32 i = 0; i < count; i += 1, item += 1){
                    // NOTE(allen): This only works if we build layouts in y-sorted order.
                    if (p.y < item->rect.y0){
                        goto double_break;
                    }
                    if (item->rect.y1 + bottom_extension <= p.y){
                        continue;
                    }
                    f32 dist0 = p.x - item->rect.x0;
                    f32 dist1 = item->rect.x1 - p.x;
                    if (dist0 >= 0.f && dist1 > 0.f){
                        closest_match = item->index;
                        goto double_break;
                    }
                    // NOTE(allen): One of dist0 and dist1 are negative, but certainly not both.
                    // 1. Take the negative one.
                    // 2. If the negative distance is larger than closest_x, then this is closer.
                    f32 neg_dist = min(dist0, dist1);
                    if (closest_x < neg_dist){
                        closest_x = neg_dist;
                        closest_match = item->index;
                    }
                }
            }
            double_break:;
        }
        else{
            if (p.x == max_f32){
                Buffer_Layout_Item *prev_item = 0;
                for (Buffer_Layout_Item_Block *block = list.first;
                     block != 0;
                     block = block->next){
                    i64 count = block->count;
                    Buffer_Layout_Item *item = block->items;
                    for (i32 i = 0; i < count; i += 1, item += 1){
                        if (p.y < item->rect.y0){
                            goto double_break_2;
                        }
                        prev_item = item;
                        if (item->rect.y1 <= p.y){
                            continue;
                        }
                    }
                }
                double_break_2:;
                if (prev_item != 0){
                    closest_match = prev_item->index;
                }
                else{
                    closest_match = list.index_range.max;
                }
            }
            else{
                Buffer_Layout_Item *closest_item = 0;
                for (Buffer_Layout_Item_Block *block = list.first;
                     block != 0;
                     block = block->next){
                    i64 count = block->count;
                    Buffer_Layout_Item *item = block->items;
                    for (i32 i = 0; i < count; i += 1, item += 1){
                        // NOTE(allen): This only works if we build layouts in y-sorted order.
                        if (p.y < item->rect.y0){
                            goto double_break_3;
                        }
                        if (item->rect.y1 <= p.y){
                            continue;
                        }
                        closest_item = item;
                        goto double_break_3;
                    }
                }
                double_break_3:;
                if (closest_item != 0){
                    closest_match = closest_item->index;
                }
                else{
                    closest_match = list.index_range.min;
                }
            }
        }
    }
    return(closest_match);
}

internal i64
buffer_layout_get_pos_at_character(Buffer_Layout_Item_List list, i64 character){
    i64 result = 0;
    if (character <= 0){
        result = list.index_range.min;
    }
    else if (character >= list.character_count){
        result = list.index_range.max + 1;
    }
    else{
        i64 counter = 0;
        for (Buffer_Layout_Item_Block *node = list.first;
             node != 0;
             node = node->next){
            i64 next_counter = counter + node->character_count;
            if (character < next_counter){
                i64 count = node->count;
                i64 relative_character = character - counter;
                i64 relative_character_counter = 0;
                i64 prev_index = -1;
                Buffer_Layout_Item *item = node->items;
                for (i64 i = 0; i < count; i += 1, item += 1){
                    if (prev_index != item->index){
                        prev_index = item->index;
                        if (relative_character_counter == relative_character){
                            result = prev_index;
                            break;
                        }
                        relative_character_counter += 1;
                    }
                }
                break;
            }
            counter = next_counter;
        }
    }
    return(result);
}

internal Buffer_Layout_Item*
buffer_layout_get_first_with_index(Buffer_Layout_Item_List list, i64 index){
    Buffer_Layout_Item *result = 0;
    Buffer_Layout_Item *prev = 0;
    for (Buffer_Layout_Item_Block *block = list.first;
         block != 0;
         block = block->next){
        i64 count = block->count;
        Buffer_Layout_Item *item = block->items;
        for (i32 i = 0; i < count; i += 1, item += 1){
            if (item->index > index){
                result = prev;
                goto done;
            }
            if (item->index == index){
                result = item;
                goto done;
            }
            prev = item;
        }
    }
    if (result == 0){
        result = prev;
    }
    done:;
    return(result);
}

internal Vec2_f32
buffer_layout_xy_center_of_pos(Buffer_Layout_Item_List list, i64 index){
    Vec2_f32 result = {};
    Buffer_Layout_Item *item = buffer_layout_get_first_with_index(list, index);
    if (item != 0){
        result = rect_center(item->rect);
    }
    return(result);
}

internal i64
buffer_layout_character_from_pos(Buffer_Layout_Item_List list, i64 index){
    i64 result = 0;
    i64 character_count = 0;
    i64 prev_index = -1;
    if (index <= list.index_range.first){
        result = 0;
    }
    else if (index > list.index_range.one_past_last){
        result = list.character_count - 1;
    }
    else{
        for (Buffer_Layout_Item_Block *node = list.first;
             node != 0;
             node = node->next){
            Buffer_Layout_Item *item = node->items;
            i64 count = node->count;
            for (i64 i = 0; i < count; i += 1, item += 1){
                if (item->index == index){
                    result = character_count;
                    goto double_break;
                }
                if (item->index != prev_index){
                    prev_index = item->index;
                    character_count += 1;
                }
            }
        }
    }
    double_break:;
    return(result);
}

// BOTTOM

