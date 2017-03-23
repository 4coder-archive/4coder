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

#include "4ed_font_data.h"
#include "4coder_helper/4coder_seek_types.h"

typedef struct Cursor_With_Index{
    u32 pos;
    i32 index;
} Cursor_With_Index;

inline void
write_cursor_with_index(Cursor_With_Index *positions, u32 *count, u32 pos){
    positions[(*count)].index = *count;
    positions[(*count)].pos = pos;
    ++(*count);
}

#define CursorSwap__(a,b) { Cursor_With_Index t = a; a = b; b = t; }

internal void
buffer_quick_sort_cursors(Cursor_With_Index *positions, i32 start, i32 pivot){
    i32 mid = start;
    u32 pivot_pos = positions[pivot].pos;
    for (i32 i = mid; i < pivot; ++i){
        if (positions[i].pos < pivot_pos){
            CursorSwap__(positions[mid], positions[i]);
            ++mid;
        }
    }
    CursorSwap__(positions[mid], positions[pivot]);
    
    if (start < mid - 1) buffer_quick_sort_cursors(positions, start, mid - 1);
    if (mid + 1 < pivot) buffer_quick_sort_cursors(positions, mid + 1, pivot);
}

// TODO(allen): Rewrite this without being a dumbass.
internal void
buffer_quick_unsort_cursors(Cursor_With_Index *positions, i32 start, i32 pivot){
    i32 mid = start;
    i32 pivot_index = positions[pivot].index;
    for (i32 i = mid; i < pivot; ++i){
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

inline void
buffer_sort_cursors(Cursor_With_Index *positions, i32 count){
    assert(count > 0);
    buffer_quick_sort_cursors(positions, 0, count-1);
}

inline void
buffer_unsort_cursors(Cursor_With_Index *positions, i32 count){
    assert(count > 0);
    buffer_quick_unsort_cursors(positions, 0, count-1);
}

internal void
buffer_update_cursors(Cursor_With_Index *sorted_positions, u32 count, u32 start, u32 end, u32 len, b32 lean_right){
    i32 shift_amount = (len - (end - start));
    Cursor_With_Index *position = sorted_positions + count - 1;
    
    if (lean_right){
        for (; position >= sorted_positions && position->pos > end; --position){
            position->pos += shift_amount;
        }
        for (; position >= sorted_positions && position->pos >= start; --position){
            position->pos = start + len;
        }
    }
    else{
        for (; position >= sorted_positions && position->pos > end; --position){
            position->pos += shift_amount;
        }
        for (; position >= sorted_positions && position->pos >= start; --position){
            position->pos = start;
        }
    }
}

internal b32
buffer_batch_debug_sort_check(Buffer_Edit *sorted_edits, u32 edit_count){
    b32 result = true; 
    
    Buffer_Edit *edit = sorted_edits;
    size_t start_point = 0;
    for (u32 i = 0; i < edit_count; ++i, ++edit){
        if (start_point > edit->start){
            result = false; break;
        }
        start_point = (edit->end < edit->start + 1)?(edit->start + 1):(edit->end);
    }
    
    return(result);
}

internal i32
buffer_batch_edit_max_shift(Buffer_Edit *sorted_edits, u32 edit_count){
    Buffer_Edit *edit = sorted_edits;
    i32 shift_total = 0, shift_max = 0;
    for (u32 i = 0; i < edit_count; ++i, ++edit){
        u32 target_length = (u32)(edit->end - edit->start);
        u32 edit_length = (u32)edit->len;
        if (edit_length > target_length){
            shift_total += (i32)(edit_length - target_length);
        }
        else{
            shift_total -= (i32)(target_length - edit_length);
        }
        if (shift_total > shift_max){
            shift_max = shift_total;
        }
    }
    
    return(shift_max);
}

internal i32
buffer_batch_edit_update_cursors(Cursor_With_Index *sorted_positions, u32 count, Buffer_Edit *sorted_edits, u32 edit_count, b32 lean_right){
    Cursor_With_Index *position = sorted_positions;
    Cursor_With_Index *end_position = sorted_positions + count;
    Buffer_Edit *edit = sorted_edits;
    Buffer_Edit *end_edit = sorted_edits + edit_count;
    i32 shift_amount = 0;
    
    if (lean_right){
        for (; edit < end_edit && position < end_position; ++edit){
            u32 start = (u32)edit->start;
            u32 end = (u32)edit->end;
            
            for (; position->pos < start && position < end_position; ++position){
                position->pos += shift_amount;
            }
            
            u32 new_end = start + (u32)edit->len + shift_amount;
            for (; position->pos <= end && position < end_position; ++position){
                position->pos = new_end;
            }
            
            u32 target_length = end - start;
            u32 edit_length = (u32)edit->len;
            if (edit_length > target_length){
                shift_amount += (i32)(edit_length - target_length);
            }
            else{
                shift_amount -= (i32)(target_length - edit_length);
            }
        }
    }
    else{
        for (; edit < end_edit && position < end_position; ++edit){
            u32 start = (u32)edit->start;
            u32 end = (u32)edit->end;
            
            for (; position->pos < start && position < end_position; ++position){
                position->pos += shift_amount;
            }
            
            u32 new_end = start + shift_amount;
            for (; position->pos <= end && position < end_position; ++position){
                position->pos = new_end;
            }
            
            u32 target_length = end - start;
            u32 edit_length = (u32)edit->len;
            if (edit_length > target_length){
                shift_amount += (i32)(edit_length - target_length);
            }
            else{
                shift_amount -= (i32)(target_length - edit_length);
            }
        }
    }
    
    for (; position < end_position; ++position){
        position->pos += shift_amount;
    }
    
    for (; edit < end_edit; ++edit){
        u32 target_length = (u32)(edit->end - edit->start);
        u32 edit_length = (u32)edit->len;
        if (edit_length > target_length){
            shift_amount += (i32)(edit_length - target_length);
        }
        else{
            shift_amount -= (i32)(target_length - edit_length);
        }
    }
    
    return(shift_amount);
}

//////////////////////////////////////

internal u32
eol_convert_in(char *dest, char *src, u32 size){
    u32 i = 0, j = 0, k = 0;
    
    for (; j < size && src[j] != '\r'; ++j);
    memcpy(dest, src, j);
    
    if (j < size){
        k = 1;
        ++j;
        for (i = j; i < size; ++i){
            if (src[i] == '\r'){
                memcpy(dest + j - k, src + j, i - j);
                ++k;
                j = i+1;
            }
        }
        memcpy(dest + j - k, src + j, i - j);
        j = i - k;
    }
    
    return(j);
}

internal u32
eol_in_place_convert_in(char *data, u32 size){
    u32 i = 0, j = 0, k = 0;
    
    for (; j < size && data[j] != '\r'; ++j);
    
    if (j < size){
        k = 1;
        ++j;
        for (i = j; i < size; ++i){
            if (data[i] == '\r'){
                memmove(data + j - k, data + j, i - j);
                ++k;
                j = i+1;
            }
        }
        memmove(data + j - k, data + j, i - j);
        j = i - k;
    }
    
    return(j);
}

// TODO(allen): iterative memory check?
internal b32
eol_convert_out(char *dest, u32 max, char *src, u32 size, u32 *size_out){
    b32 result = true;
    u32 i = 0, j = 0;
    
    for (; i < size; ++i, ++j){
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
    return(result);
}

// TODO(allen): iterative memory check?
internal b32
eol_in_place_convert_out(char *data, u32 size, u32 max, u32 *size_out){
    b32 result = true;
    u32 i = 0;
    
    for (; i < size; ++i){
        if (data[i] == '\n'){
            memmove(data + i + 1, data + i, size - i);
            data[i] = '\r';
            ++i;
            ++size;
        }
    }
    
    *size_out = size;
    return(result);
}

//////////////////////////////////////

//
// Implementation of the gap buffer
//

typedef struct Gap_Buffer{
    char *data;
    u32 size1;
    u32 gap_size;
    u32 size2;
    u32 max;
    
    i32 *line_starts;
    i32 line_count;
    i32 line_max;
} Gap_Buffer;

inline b32
buffer_good(Gap_Buffer *buffer){
    b32 good = (buffer->data != 0);
    return(good);
}

inline u32
buffer_size(Gap_Buffer *buffer){
    u32 size = buffer->size1 + buffer->size2;
    return(size);
}

typedef struct Gap_Buffer_Init{
    Gap_Buffer *buffer;
    char *data;
    u32 size;
} Gap_Buffer_Init;

internal Gap_Buffer_Init
buffer_begin_init(Gap_Buffer *buffer, char *data, u32 size){
    Gap_Buffer_Init init;
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    return(init);
}

internal b32
buffer_init_need_more(Gap_Buffer_Init *init){
    b32 result = true;
    if (init->buffer->data != 0){
        result = false;
    }
    return(result);
}

internal u32
buffer_init_page_size(Gap_Buffer_Init *init){
    u32 result = init->size * 2;
    return(result);
}

internal void
buffer_init_provide_page(Gap_Buffer_Init *init, void *page, u32 page_size){
    Gap_Buffer *buffer = init->buffer;
    buffer->data = (char*)page;
    buffer->max = page_size;
}

internal b32
buffer_end_init(Gap_Buffer_Init *init){
    Gap_Buffer *buffer = init->buffer;
    b32 result = false;
    
    if (buffer->data != 0 && buffer->max >= init->size){
        i32 size = init->size;
        i32 size2 = size*2;
        i32 osize1 = size - size2;
        i32 size1 = osize1;
        
        if (size1 > 0){
            size1 = eol_convert_in(buffer->data, init->data, size1);
            if (size2 > 0){
                size2 = eol_convert_in(buffer->data + size1, init->data + osize1, size2);
            }
        }
        
        buffer->size1 = size1;
        buffer->size2 = size2;
        buffer->gap_size = buffer->max - size1 - size2;
        memmove(buffer->data + size1 + buffer->gap_size, buffer->data + size1, size2);
        
        result = true;
    }
    
    return(result);
}

typedef struct Gap_Buffer_Stream{
    Gap_Buffer *buffer;
    char *data;
    i32 end;
    i32 absolute_end;
    b32 separated;
    b32 use_termination_character;
    char terminator;
} Gap_Buffer_Stream;
static Gap_Buffer_Stream null_buffer_stream = {0};

internal b32
buffer_stringify_loop(Gap_Buffer_Stream *stream, Gap_Buffer *buffer, u32 start, u32 end){
    b32 result = false;
    
    if (0 <= start && start < end && end <= buffer->size1 + buffer->size2){
        stream->buffer = buffer;
        stream->absolute_end = end;
        
        if (start < buffer->size1){
            if (buffer->size1 < end){
                stream->separated = true;
            }
            else{
                stream->separated = false;
            }
            stream->data = buffer->data;
        }
        else{
            stream->separated = false;
            stream->data = buffer->data + buffer->gap_size;
        }
        
        if (stream->separated){
            stream->end = buffer->size1;
        }
        else{
            stream->end = end;
        }
        
        if (stream->end > stream->absolute_end){
            stream->end = stream->absolute_end;
        }
        
        result = true;
    }
    
    if (!result){
        if (stream->use_termination_character){
            stream->buffer = buffer;
            stream->absolute_end = end;
            stream->use_termination_character = false;
            stream->data = (&stream->terminator) - buffer->size1 - buffer->size2;
            stream->end = stream->absolute_end + 1;
            result = true;
        }
    }
    
    return(result);
}

internal b32
buffer_stringify_next(Gap_Buffer_Stream *stream){
    b32 result = false;
    Gap_Buffer *buffer = stream->buffer;
    if (stream->separated){
        stream->data = buffer->data + buffer->gap_size;
        stream->end = stream->absolute_end;
        stream->separated = false;
        result = true;
    }
    
    if (!result){
        if (stream->use_termination_character){
            stream->use_termination_character = false;
            stream->data = (&stream->terminator) - buffer->size1 - buffer->size2;
            stream->end = stream->absolute_end + 1;
            result = true;
        }
    }
    
    return(result);
}

internal b32
buffer_replace_range(Gap_Buffer *buffer, u32 start, u32 end, char *str, u32 len, i32 *shift_amount, void *scratch, u32 scratch_memory, u32 *request_amount){
    char *data = buffer->data;
    u32 size = buffer_size(buffer);
    b32 result = false;
    
    assert(0 <= start);
    assert(start <= end);
    assert(end <= size);
    
    u32 target_length = end - start;
    
    if (target_length <= max_i32 && len <= max_i32){
        if (len >= target_length){
            *shift_amount = (i32)(len - target_length);
        }
        else{
            *shift_amount = -(i32)(target_length - len);
        }
        
        if (*shift_amount + size <= buffer->max){
            u32 move_size = 0;
            if (end < buffer->size1){
                move_size = buffer->size1 - end;
                memmove(data + buffer->size1 + buffer->gap_size - move_size, data + end, move_size);
                buffer->size1 -= move_size;
                buffer->size2 += move_size;
            }
            if (start > buffer->size1){
                move_size = start - buffer->size1;
                memmove(data + buffer->size1, data + buffer->size1 + buffer->gap_size, move_size);
                buffer->size1 += move_size;
                buffer->size2 -= move_size;
            }
            
            memcpy(data + start, str, len);
            buffer->size2 = size - end;
            buffer->size1 = start + len;
            buffer->gap_size -= *shift_amount;
            
            assert(buffer->size1 + buffer->size2 == size + *shift_amount);
            assert(buffer->size1 + buffer->gap_size + buffer->size2 == buffer->max);
        }
        else{
            *request_amount = l_round_up_u32(2*(*shift_amount + size), KB(4));
            result = true;
        }
    }
    
    return(result);
}

typedef struct Buffer_Batch_State{
    u32 i;
    i32 shift_total;
} Buffer_Batch_State;

// TODO(allen): Now that we are just using Gap_Buffer we could afford to improve this for the Gap_Buffer's behavior.
internal b32
buffer_batch_edit_step(Buffer_Batch_State *state, Gap_Buffer *buffer, Buffer_Edit *sorted_edits, char *strings, u32 edit_count, void *scratch, u32 scratch_size, u32 *request_amount){
    Buffer_Edit *edit = 0;
    u32 i = state->i;
    i32 shift_total = state->shift_total;
    i32 shift_amount = 0;
    b32 result = false;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        u32 start = (u32)(edit->start + shift_total);
        u32 end = (u32)(edit->end + shift_total);
        u32 len = (u32)(edit->len);
        
        result = buffer_replace_range(buffer, start, end, strings + edit->str_start, len, &shift_amount, scratch, scratch_size, request_amount);
        if (result){
            break;
        }
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

internal void*
buffer_edit_provide_memory(Gap_Buffer *buffer, void *new_data, u32 new_max){
    void *result = buffer->data;
    u32 size = buffer_size(buffer);
    u32 new_gap_size = new_max - size;
    
    assert(new_max >= size);
    
    memcpy(new_data, buffer->data, buffer->size1);
    memcpy((char*)new_data + buffer->size1 + new_gap_size, buffer->data + buffer->size1 + buffer->gap_size, buffer->size2);
    
    buffer->data = (char*)new_data;
    buffer->gap_size = new_gap_size;
    buffer->max = new_max;
    
    return(result);
}

//
// High level buffer operations
//

inline void
buffer_stringify(Gap_Buffer *buffer, u32 start, u32 end, char *out){
    Gap_Buffer_Stream stream = {0};
    
    u32 i = start;
    if (buffer_stringify_loop(&stream, buffer, i, end)){
        b32 still_looping = false;
        do{
            u32 size = stream.end - i;
            memcpy(out, stream.data + i, size);
            i = stream.end;
            out += size;
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
}

internal u32
buffer_convert_out(Gap_Buffer *buffer, char *dest, u32 max){
    Gap_Buffer_Stream stream = {0};
    u32 size = buffer_size(buffer);
    assert(size + buffer->line_count <= max);
    
    u32 pos = 0;
    if (buffer_stringify_loop(&stream, buffer, 0, size)){
        u32 i = 0;
        b32 still_looping = false;
        do{
            u32 chunk_size = stream.end - i;
            u32 out_size = 0;
            b32 result = eol_convert_out(dest + pos, max - pos, stream.data + i, chunk_size, &out_size);
            assert(result);
            i = stream.end;
            pos += out_size;
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    
    return(pos);
}

internal i32
buffer_count_newlines(Gap_Buffer *buffer, i32 start, i32 end){
    Gap_Buffer_Stream stream = {0};
    i32 i = start;
    i32 count = 0;
    
    assert(0 <= start);
    assert(start <= end);
    assert(end <= (i32)buffer_size(buffer));
    
    if (buffer_stringify_loop(&stream, buffer, i, end)){
        b32 still_looping = 0;
        do{
            for (; i < stream.end; ++i){
                count += (stream.data[i] == '\n');
            }
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    
    return(count);
}

typedef struct Buffer_Measure_Starts{
    i32 i;
    i32 count;
    i32 start;
} Buffer_Measure_Starts;

// TODO(allen): Rewrite this with a duff routine
// Also make it so that the array goes one past the end
// and stores the size in the extra spot.
internal b32
buffer_measure_starts(Buffer_Measure_Starts *state, Gap_Buffer *buffer){
    Gap_Buffer_Stream stream = {0};
    i32 size = (i32)buffer_size(buffer);
    i32 start = state->start, i = state->i;
    i32 *start_ptr = buffer->line_starts + state->count;
    i32 *start_end = buffer->line_starts + buffer->line_max;
    b32 result = true;
    
    if (buffer_stringify_loop(&stream, buffer, i, size)){
        b32 still_looping = 0;
        do{
            for (; i < stream.end; ++i){
                char ch = stream.data[i];
                if (ch == '\n'){
                    if (start_ptr == start_end){
                        goto buffer_measure_starts_widths_end;
                    }
                    
                    *start_ptr++ = start;
                    start = i + 1;
                }
            }
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    assert(i == size);
    if (start_ptr == start_end){
        goto buffer_measure_starts_widths_end;
    }
    *start_ptr++ = start;
    result = false;
    
    buffer_measure_starts_widths_end:;
    state->i = i;
    state->count = (i32)(start_ptr - buffer->line_starts);
    state->start = start;
    
    if (!result){
        buffer->line_count = state->count;
    }
    
    return(result);
}

internal void
buffer_measure_character_starts(System_Functions *system, Render_Font *font, Gap_Buffer *buffer, i32 *character_starts, i32 mode, b32 virtual_white){
    assert(mode == 0);
    
    Gap_Buffer_Stream stream = {0};
    
    i32 line_index = 0;
    i32 character_index = 0;
    
    character_starts[line_index++] = character_index;
    
    b32 skipping_whitespace = false;
    if (virtual_white){
        skipping_whitespace = true;
    }
    
    Translation_State tran = {0};
    Translation_Emits emits = {0};
    
    stream.use_termination_character = 1;
    stream.terminator = '\n';
    
    i32 size = (i32)buffer_size(buffer);
    i32 i = 0;
    if (buffer_stringify_loop(&stream, buffer, i, size)){
        b32 still_looping = false;
        do{
            for (; i < stream.end; ++i){
                u8 ch = (u8)stream.data[i];
                
                translating_fully_process_byte(system, font, &tran, ch, i, size, &emits);
                
                for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                    TRANSLATION_DECL_GET_STEP(step, behavior, J, emits);
                    
                    if (behavior.do_newline){
                        ++character_index;
                        character_starts[line_index++] = character_index;
                        if (virtual_white){
                            skipping_whitespace = true;
                        }
                    }
                    else if (behavior.do_codepoint_advance || behavior.do_number_advance){
                        if (ch != ' ' && ch != '\t'){
                            skipping_whitespace = false;
                        }
                        
                        if (!skipping_whitespace){
                            ++character_index;
                        }
                    }
                }
            }
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    
    assert(line_index-1 == buffer->line_count);
}

enum{
    BLStatus_Finished,
    BLStatus_NeedWrapLineShift,
    BLStatus_NeedLineShift,
    BLStatus_NeedWrapDetermination,
};

struct Buffer_Layout_Stop{
    i32 status;
    i32 line_index;
    i32 wrap_line_index;
    i32 pos;
    i32 next_line_pos;
    f32 x;
};

struct Buffer_Measure_Wrap_Params{
    Gap_Buffer *buffer;
    i32 *wrap_line_index;
    System_Functions *system;
    Render_Font *font;
    b32 virtual_white;
};

struct Buffer_Measure_Wrap_State{
    Gap_Buffer_Stream stream;
    i32 i;
    i32 size;
    b32 still_looping;
    
    i32 line_index;
    
    i32 current_wrap_index;
    f32 current_adv;
    f32 x;
    
    i32 wrap_unit_end;
    b32 skipping_whitespace;
    b32 did_wrap;
    b32 first_of_the_line;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
    
    i32 __pc__;
};

// duff-routine defines
#define DrCase(PC) case PC: goto resumespot_##PC
#define DrYield(PC, n) { *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }
#define DrReturn(n) { *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

internal Buffer_Layout_Stop
buffer_measure_wrap_y(Buffer_Measure_Wrap_State *S_ptr, Buffer_Measure_Wrap_Params params, f32 line_shift, b32 do_wrap, u32 wrap_unit_end){
    Buffer_Measure_Wrap_State S = *S_ptr;
    Buffer_Layout_Stop S_stop;
    
    switch (S.__pc__){
        DrCase(1);
        DrCase(2);
        DrCase(3);
        DrCase(4);
    }
    
    S.size = (u32)buffer_size(params.buffer);
    
    if (params.virtual_white){
        S_stop.status = BLStatus_NeedLineShift;
        S_stop.line_index = S.line_index;
        S_stop.wrap_line_index = S.current_wrap_index;
        S_stop.pos = S.i;
        DrYield(1, S_stop);
    }
    
    S.x = line_shift;
    params.wrap_line_index[S.line_index++] = 0;
    
    if (params.virtual_white){
        S.skipping_whitespace = true;
    }
    
    S.first_of_the_line = 1;
    if (buffer_stringify_loop(&S.stream, params.buffer, S.i, S.size)){
        S.still_looping = 0;
        do{
            for (; S.i < S.stream.end; ++S.i){
                {
                    u8 ch = (u8)S.stream.data[S.i];
                    
                    if (ch != ' ' && ch != '\t'){
                        S.skipping_whitespace = false;
                    }
                    
                    translating_fully_process_byte(params.system, params.font, &S.tran, ch, S.i, S.size, &S.emits);
                }
                
                for (TRANSLATION_EMIT_LOOP(S.J, S.emits)){
                    TRANSLATION_GET_STEP(S.step, S.behavior, S.J, S.emits);
                    
                    if (S.behavior.do_newline){
                        ++S.current_wrap_index;
                        params.wrap_line_index[S.line_index++] = S.current_wrap_index;
                        
                        if (params.virtual_white){
                            S_stop.status          = BLStatus_NeedLineShift;
                            S_stop.line_index      = S.line_index - 1;
                            S_stop.wrap_line_index = S.current_wrap_index;
                            S_stop.pos             = S.i+1;
                            DrYield(2, S_stop);
                        }
                        
                        S.x = line_shift;
                        
                        if (params.virtual_white){
                            S.skipping_whitespace = 1;
                        }
                        S.first_of_the_line = 1;
                    }
                    else if (S.behavior.do_number_advance || S.behavior.do_codepoint_advance){
                        if (!S.skipping_whitespace){
                            if (S.behavior.do_codepoint_advance){
                                S.current_adv = font_get_glyph_advance(params.system, params.font, S.step.value);
                            }
                            else{
                                S.current_adv = font_get_byte_advance(params.font);
                            }
                            
                            S.did_wrap = false;
                            if (S.i >= S.wrap_unit_end){
                                S_stop.status          = BLStatus_NeedWrapDetermination;
                                S_stop.line_index      = S.line_index - 1;
                                S_stop.wrap_line_index = S.current_wrap_index;
                                S_stop.pos             = S.i;
                                S_stop.x               = S.x;
                                DrYield(4, S_stop);
                                
                                S.wrap_unit_end = wrap_unit_end;
                                
                                if (do_wrap && !S.first_of_the_line){
                                    S.did_wrap = true;
                                    ++S.current_wrap_index;
                                    
                                    if (params.virtual_white){
                                        S_stop.status          = BLStatus_NeedWrapLineShift;
                                        S_stop.line_index      = S.line_index - 1;
                                        S_stop.wrap_line_index = S.current_wrap_index;
                                        S_stop.pos             = S.i;
                                        DrYield(3, S_stop);
                                    }
                                    
                                    S.x = line_shift + S.current_adv;
                                }
                            }
                            
                            if (!S.did_wrap){
                                S.x += S.current_adv;
                            }
                            
                            S.first_of_the_line = 0;
                        }
                    }
                }
            }
            S.still_looping = buffer_stringify_next(&S.stream);
        }while(S.still_looping);
    }
    
    ++S.current_wrap_index;
    params.wrap_line_index[S.line_index++] = S.current_wrap_index;
    
    assert(S.line_index-1 == params.buffer->line_count);
    
    S_stop.status = BLStatus_Finished;
    DrReturn(S_stop);
}

#undef DrCase
#undef DrYield
#undef DrReturn

internal void
buffer_remeasure_starts(Gap_Buffer *buffer, i32 start_line, i32 end_line, i32 line_shift, i32 text_shift){
    i32 *starts = buffer->line_starts;
    i32 line_count = buffer->line_count;
    
    assert(0 <= start_line);
    assert(start_line <= end_line);
    assert(end_line < line_count);
    assert(line_count + line_shift <= buffer->line_max);
    
    ++end_line;
    
    // Adjust
    if (text_shift != 0){
        i32 line_i = end_line;
        starts += line_i;
        for (; line_i < line_count; ++line_i, ++starts){
            *starts += text_shift;
        }
        starts = buffer->line_starts;
    }
    
    // Shift
    i32 new_line_count = line_count;
    i32 new_end_line = end_line;
    if (line_shift != 0){
        new_line_count += line_shift;
        new_end_line += line_shift;
        
        memmove(starts + end_line + line_shift, starts + end_line,
                sizeof(i32)*(line_count - end_line));
    }
    
    // Iteration data (yikes! Need better loop system)
    Gap_Buffer_Stream stream = {0};
    i32 size = buffer_size(buffer);
    i32 char_i = starts[start_line];
    i32 line_i = start_line;
    
    // Line start measurement
    i32 start = char_i;
    
    if (buffer_stringify_loop(&stream, buffer, char_i, size)){
        b32 still_looping = 0;
        do{
            for (; char_i < stream.end; ++char_i){
                u8 ch = (u8)stream.data[char_i];
                
                if (ch == '\n'){
                    starts[line_i] = start;
                    ++line_i;
                    start = char_i + 1;
                    
                    // TODO(allen): I would like to know that I am not guessing here, so let's try to turn the && into an Assert.
                    if (line_i >= new_end_line && (line_i >= new_line_count || start == starts[line_i])){
                        goto buffer_remeasure_starts_end;
                    }
                }
            }
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    
    // TODO(allen): I suspect this can just go away.
    if (char_i == size){
        starts[line_i++] = start;
    }
    
    buffer_remeasure_starts_end:;
    assert(line_count >= 1);
    buffer->line_count = new_line_count;
}

internal void
buffer_remeasure_character_starts(System_Functions *system, Render_Font *font, Gap_Buffer *buffer, i32 start_line, i32 end_line, i32 line_shift, i32 *character_starts, i32 mode, b32 virtual_whitespace){
    assert(mode == 0);
    
    i32 new_line_count = buffer->line_count;
    
    assert(0 <= start_line);
    assert(start_line <= end_line);
    assert(end_line < new_line_count - line_shift);
    
    ++end_line;
    
    // Shift
    i32 line_count = new_line_count;
    i32 new_end_line = end_line;
    if (line_shift != 0){
        line_count -= line_shift;
        new_end_line += line_shift;
        memmove(character_starts + end_line + line_shift, character_starts + end_line, sizeof(i32)*(line_count - end_line + 1));
    }
    
    // Iteration data
    Gap_Buffer_Stream stream = {0};
    i32 size = buffer_size(buffer);
    i32 char_i = buffer->line_starts[start_line];
    i32 line_i = start_line;
    
    // Character measurement
    i32 last_char_start = character_starts[line_i];
    i32 current_char_start = last_char_start;
    
    b32 skipping_whitespace = false;
    if (virtual_whitespace){
        skipping_whitespace = true;
    }
    
    // Translation
    Translation_State tran = {0};
    Translation_Emits emits = {0};
    
    stream.use_termination_character = true;
    stream.terminator = '\n';
    if (buffer_stringify_loop(&stream, buffer, char_i, size)){
        b32 still_looping = 0;
        do{
            for (; char_i < stream.end; ++char_i){
                u8 ch = (u8)stream.data[char_i];
                translating_fully_process_byte(system, font, &tran, ch, char_i, size, &emits);
                
                for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                    TRANSLATION_DECL_GET_STEP(step, behavior, J, emits);
                    
                    if (behavior.do_newline){
                        character_starts[line_i++] = last_char_start;
                        ++current_char_start;
                        last_char_start = current_char_start;
                        if (virtual_whitespace){
                            skipping_whitespace = 1;
                        }
                        
                        if (line_i >= new_end_line){
                            goto buffer_remeasure_character_starts_end;
                        }
                    }
                    else if (behavior.do_codepoint_advance || behavior.do_number_advance){
                        if (ch != ' ' && ch != '\t'){
                            skipping_whitespace = 0;
                        }
                        
                        if (!skipping_whitespace){
                            ++current_char_start;
                        }
                    }
                }
            }
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    assert(line_i >= new_end_line);
    buffer_remeasure_character_starts_end:;
    
    // Adjust
    if (line_i <= new_end_line){
        u32 old_value = character_starts[line_i];
        u32 new_value = current_char_start;
        i32 character_shift = 0;
        if (new_value > old_value){
            character_shift = (i32)(new_value - old_value);
        }
        else{
            character_shift = -(i32)(old_value - new_value);
        }
        
        if (character_shift != 0){
            character_starts += line_i;
            for (; line_i <= new_line_count; ++line_i, ++character_starts){
                *character_starts += character_shift;
            }
        }
    }
}

internal void
buffer_remeasure_wrap_y(Gap_Buffer *buffer, i32 start_line, i32 end_line, i32 line_shift, f32 *wraps, f32 font_height, f32 *adv, f32 max_width){
    i32 new_line_count = buffer->line_count;
    
    assert(0 <= start_line);
    assert(start_line <= end_line);
    assert(end_line < new_line_count - line_shift);
    
    ++end_line;
    
    // Shift
    i32 line_count = new_line_count;
    i32 new_end_line = end_line;
    if (line_shift != 0){
        line_count -= line_shift;
        new_end_line += line_shift;
        
        memmove(wraps + end_line + line_shift, wraps + end_line, sizeof(i32)*(line_count - end_line + 1));
    }
    
    // Iteration data (yikes! Need better loop system)
    Gap_Buffer_Stream stream = {0};
    i32 size = buffer_size(buffer);
    i32 char_i = buffer->line_starts[start_line];
    i32 line_i = start_line;
    
    // Line wrap measurement
    f32 last_wrap = wraps[line_i];
    f32 current_wrap = last_wrap;
    f32 x = 0.f;
    
    if (buffer_stringify_loop(&stream, buffer, char_i, size)){
        b32 still_looping = false;
        do{
            for (; char_i < stream.end; ++char_i){
                u8 ch = (u8)stream.data[char_i];
                
                if (ch == '\n'){
                    wraps[line_i] = last_wrap;
                    ++line_i;
                    current_wrap += font_height;
                    last_wrap = current_wrap;
                    x = 0.f;
                    
                    // TODO(allen): I would like to know that I am not guessing here.
                    if (line_i >= new_end_line){
                        goto buffer_remeasure_wraps_end;
                    }
                }
                else{
                    f32 current_adv = adv[ch];
                    if (x + current_adv > max_width){
                        current_wrap += font_height;
                        x = current_adv;
                    }
                    else{
                        x += current_adv;
                    }
                }
            }
            still_looping = buffer_stringify_next(&stream);
        }while(still_looping);
    }
    
    wraps[line_i++] = last_wrap;
    
    buffer_remeasure_wraps_end:;
    
    // Adjust
    if (line_i <= new_end_line){
        f32 y_shift = current_wrap - wraps[line_i];
        
        if (y_shift != 0){
            wraps += line_i;
            for (; line_i <= new_line_count; ++line_i, ++wraps){
                *wraps += y_shift;
            }
        }
    }
}

internal i32
binary_search(i32 *array, i32 value, i32 l_bound, i32 u_bound){
    value = clamp_bottom(0, value);
    i32 start = l_bound, end = u_bound, i = 0;
    for (;;){
        i = (start + end) >> 1;
        if (array[i] < value){
            start = i;
        }
        else if (array[i] > value){
            end = i;
        }
        else{
            break;
        }
        assert(start < end);
        if (start == end - 1){
            i = start;
            break;
        }
    }
    return(i);
}

inline i32
buffer_get_line_index_range(Gap_Buffer *buffer, i32 pos, i32 l_bound, i32 u_bound){
    assert(0 <= l_bound);
    assert(l_bound <= u_bound);
    assert(u_bound <= buffer->line_count);
    
    assert(buffer->line_starts != 0);
    
    i32 i = binary_search(buffer->line_starts, pos, l_bound, u_bound);
    return(i);
}

inline i32
buffer_get_line_index(Gap_Buffer *buffer, i32 pos){
    i32 result = buffer_get_line_index_range(buffer, pos, 0, buffer->line_count);
    return(result);
}

inline i32
buffer_get_line_index_from_character_pos(i32 *character_starts, i32 pos, i32 l_bound, i32 u_bound){
    i32 i = binary_search(character_starts, pos, l_bound, u_bound);
    return(i);
}

inline i32
buffer_get_line_index_from_wrapped_y(i32 *wrap_line_index, f32 y, i32 line_height, i32 l_bound, i32 u_bound){
    i32 wrap_index = floor32(y/line_height);
    i32 i = binary_search(wrap_line_index, wrap_index, l_bound, u_bound);
    return(i);
}

internal Partial_Cursor
buffer_partial_from_pos(Gap_Buffer *buffer, u32 pos){
    Partial_Cursor result = {0};
    
    u32 size = buffer_size(buffer);
    pos = clamp_u32(0, pos, size);
    
    u32 line_index = buffer_get_line_index_range(buffer, (u32)pos, 0, (u32)buffer->line_count);
    result.pos = pos;
    result.line = line_index+1;
    result.character = pos - buffer->line_starts[line_index] + 1;
    
    return(result);
}

internal Partial_Cursor
buffer_partial_from_line_character(Gap_Buffer *buffer, i32 line, i32 character, b32 reversed){
    Partial_Cursor result = {0};
    
    i32 line_index = line - 1;
    if (line_index >= buffer->line_count){
        line_index = buffer->line_count - 1;
    }
    if (line_index < 0){
        line_index = 0;
    }
    
    i32 size = buffer_size(buffer);
    
    i32 this_start = buffer->line_starts[line_index];
    i32 max_character = (size-this_start) + 1;
    if (line_index+1 < buffer->line_count){
        i32 next_start = buffer->line_starts[line_index+1];
        max_character = (next_start-this_start);
    }
    
    i32 adjusted_pos = 0;
    if (character > 0){
        if (reversed){
            if (character > max_character){
                adjusted_pos = 0;
            }
            else{
                adjusted_pos = max_character + character;
            }
        }
        else{
            if (character > max_character){
                adjusted_pos = max_character - 1;
            }
            else{
                adjusted_pos = character - 1;
            }
        }
    }
    else if (character == 0){
        adjusted_pos = 0;
    }
    
    result.pos = this_start + adjusted_pos;
    result.line = line_index + 1;
    result.character = character;
    
    return(result);
}

struct Buffer_Cursor_Seek_Params{
    Gap_Buffer *buffer;
    Buffer_Seek seek;
    System_Functions *system;
    Render_Font *font;
    i32 *wrap_line_index;
    i32 *character_starts;
    b32 virtual_white;
    b32 return_hint;
    Full_Cursor *cursor_out;
};

struct Buffer_Cursor_Seek_State{
    Full_Cursor next_cursor;
    Full_Cursor this_cursor;
    Full_Cursor prev_cursor;
    
    Gap_Buffer_Stream stream;
    b32 still_looping;
    i32 i;
    i32 size;
    i32 wrap_unit_end;
    
    b32 first_of_the_line;
    b32 xy_seek;
    f32 ch_width;
    
    i32 font_height;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
    
    
    i32 __pc__;
};

// dialogical-routine defines
#define DrCase(PC) case PC: goto resumespot_##PC
#define DrYield(PC, n) { *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }
#define DrReturn(n) { *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

internal Buffer_Layout_Stop
buffer_cursor_seek(Buffer_Cursor_Seek_State *S_ptr, Buffer_Cursor_Seek_Params params, f32 line_shift, b32 do_wrap, i32 wrap_unit_end){
    Buffer_Cursor_Seek_State S = *S_ptr;
    Buffer_Layout_Stop S_stop;
    
    switch (S.__pc__){
        DrCase(1);
        DrCase(2);
        DrCase(3);
        DrCase(4);
    }
    
    S.font_height = font_get_height(params.font);
    
    S.xy_seek = (params.seek.type == buffer_seek_wrapped_xy || params.seek.type == buffer_seek_unwrapped_xy);
    S.size = (i32)buffer_size(params.buffer);
    
    // Get cursor hint
    {
        i32 line_index = 0;
        switch (params.seek.type){
            case buffer_seek_pos:
            {
                params.seek.pos = clamp_u32(0, (u32)params.seek.pos, S.size);
                
                line_index = buffer_get_line_index(params.buffer, (u32)params.seek.pos);
            }break;
            
            case buffer_seek_character_pos:
            {
                i32 line_count = params.buffer->line_count;
                i32 max_character = params.character_starts[line_count] - 1;
                params.seek.pos = clamp_i32(0, (i32)params.seek.pos, max_character);
                
                i32 *char_starts = params.character_starts;
                
                i32 pos = (i32)params.seek.pos;
                line_index = buffer_get_line_index_from_character_pos(char_starts, pos, 0, line_count);
            }break;
            
            case buffer_seek_line_char:
            {
                line_index = params.seek.line - 1;
                line_index = clamp_bottom(0, line_index);
            }break;
            
            case buffer_seek_unwrapped_xy:
            {
                line_index = (i32)(params.seek.y / S.font_height);
                line_index = clamp_bottom(0, line_index);
            }break;
            
            case buffer_seek_wrapped_xy:
            {
                line_index = buffer_get_line_index_from_wrapped_y(params.wrap_line_index, params.seek.y, S.font_height, 0, params.buffer->line_count);
            }break;
            
            default: InvalidCodePath;
        }
        
        i32 safe_line_index = line_index;
        if (line_index >= params.buffer->line_count){
            safe_line_index = params.buffer->line_count-1;
        }
        
        // Build the cursor hint
        S.next_cursor.pos = params.buffer->line_starts[safe_line_index];
        S.next_cursor.character_pos = params.character_starts[safe_line_index];
        S.next_cursor.line = line_index + 1;
        S.next_cursor.character = 1;
        S.next_cursor.wrap_line = params.wrap_line_index[safe_line_index] + 1;
        S.next_cursor.unwrapped_y = (f32)(safe_line_index * S.font_height);
        S.next_cursor.unwrapped_x = 0;
        S.next_cursor.wrapped_y = (f32)(params.wrap_line_index[safe_line_index] * S.font_height);
        S.next_cursor.wrapped_x = 0;
    }
    
    // Get the initial line shift.
    // Adjust the non-screen based coordinates to point to the first
    // non-virtual character of the line.
    if (params.virtual_white){
        S_stop.status          = BLStatus_NeedLineShift;
        S_stop.line_index      = (u32)(S.next_cursor.line-1);
        S_stop.wrap_line_index = (u32)(S.next_cursor.wrap_line-1);
        DrYield(1, S_stop);
        
        S.next_cursor.unwrapped_x += line_shift;
        S.next_cursor.wrapped_x += line_shift;
        
        S.stream.use_termination_character = true;
        S.stream.terminator = '\n';
        if (buffer_stringify_loop(&S.stream, params.buffer, (u32)S.next_cursor.pos, S.size)){
            do{
                for (; S.next_cursor.pos < S.stream.end; ++S.next_cursor.pos){
                    u8 ch = (u8)S.stream.data[S.next_cursor.pos];
                    
                    if (ch != ' ' && ch != '\t'){
                        goto double_break_vwhite;
                    }
                    else{
                        ++S.next_cursor.character;
                    }
                }
                S.still_looping = buffer_stringify_next(&S.stream);
            }while(S.still_looping);
        }
        InvalidCodePath;
        double_break_vwhite:;
    }
    
    // If the caller just wants the hint, return that now.
    if (params.return_hint){
        *params.cursor_out = S.next_cursor;
        S_stop.status = BLStatus_Finished;
        DrReturn(S_stop);
    }
    
    // If we are already passed the point we want to be at, then just take this.
    S.this_cursor = S.next_cursor;
    switch (params.seek.type){
        case buffer_seek_pos:
        {
            if (S.next_cursor.pos >= params.seek.pos){
                goto buffer_cursor_seek_end;
            }
        }break;
        
        case buffer_seek_character_pos:
        {
            if (S.next_cursor.character_pos >= params.seek.pos){
                goto buffer_cursor_seek_end;
            }
        }break;
        
        case buffer_seek_line_char:
        {
            if ((S.next_cursor.line == params.seek.line &&
                 S.next_cursor.character >= params.seek.character) ||
                S.next_cursor.line > params.seek.line){
                goto buffer_cursor_seek_end;
            }
        }break;
        
        case buffer_seek_unwrapped_xy:
        {
            if (S.next_cursor.unwrapped_y > params.seek.y){
                goto buffer_cursor_seek_end;
            }
        }break;
        
        case buffer_seek_wrapped_xy:
        {
            if (S.next_cursor.wrapped_y > params.seek.y){
                goto buffer_cursor_seek_end;
            }
        }break;
    }
    
    // Main seek loop
    S.i = (u32)S.next_cursor.pos;
    
    S.stream = null_buffer_stream;
    S.stream.use_termination_character = 1;
    S.stream.terminator = 0;
    
    S.first_of_the_line = 1;
    if (buffer_stringify_loop(&S.stream, params.buffer, S.i, S.size)){
        S.still_looping = 0;
        do{
            for (; S.i < S.stream.end; ++S.i){
                {
                    u8 ch = (u8)S.stream.data[S.i];
                    translating_fully_process_byte(params.system, params.font, &S.tran, ch, S.i, S.size, &S.emits);
                }
                
                for (TRANSLATION_EMIT_LOOP(S.J, S.emits)){
                    TRANSLATION_GET_STEP(S.step, S.behavior, S.J, S.emits);
                    
                    S.prev_cursor = S.this_cursor;
                    S.this_cursor = S.next_cursor;
                    
                    if (S.behavior.do_newline){
                        ++S.next_cursor.character_pos;
                        ++S.next_cursor.line;
                        ++S.next_cursor.wrap_line;
                        S.next_cursor.unwrapped_y += S.font_height;
                        S.next_cursor.wrapped_y += S.font_height;
                        S.next_cursor.character = 1;
                        S.next_cursor.unwrapped_x = 0;
                        
                        if (params.virtual_white){
                            S_stop.status          = BLStatus_NeedLineShift;
                            S_stop.line_index      = (u32)(S.next_cursor.line-1);
                            S_stop.wrap_line_index = (u32)(S.next_cursor.wrap_line-1);
                            DrYield(2, S_stop);
                        }
                        
                        S.next_cursor.wrapped_x = line_shift;
                        S.first_of_the_line = 1;
                    }
                    else if (S.behavior.do_number_advance || S.behavior.do_codepoint_advance){
                        
                        if (S.behavior.do_codepoint_advance){
                            S.ch_width = font_get_glyph_advance(params.system, params.font, S.step.value);
                        }
                        else{
                            S.ch_width = font_get_byte_advance(params.font);
                        }
                        
                        if (S.step.i >= S.wrap_unit_end){
                            S_stop.status          = BLStatus_NeedWrapDetermination;
                            S_stop.line_index      = (u32)(S.next_cursor.line-1);
                            S_stop.wrap_line_index = (u32)(S.next_cursor.wrap_line-1);
                            S_stop.pos             = (u32)(S.step.i);
                            S_stop.x               = S.next_cursor.wrapped_x;
                            DrYield(4, S_stop);
                            
                            S.wrap_unit_end = wrap_unit_end;
                            
                            if (do_wrap && !S.first_of_the_line){
                                S.next_cursor.wrapped_y += S.font_height;
                                
                                ++S.next_cursor.wrap_line;
                                if (params.virtual_white){
                                    S_stop.status          = BLStatus_NeedWrapLineShift;
                                    S_stop.line_index      = (u32)(S.next_cursor.line-1);
                                    S_stop.wrap_line_index = (u32)(S.next_cursor.wrap_line-1);
                                    DrYield(3, S_stop);
                                }
                                
                                S.next_cursor.wrapped_x = line_shift;
                                S.this_cursor = S.next_cursor;
                            }
                        }
                        
                        ++S.next_cursor.character_pos;
                        ++S.next_cursor.character;
                        S.next_cursor.unwrapped_x += S.ch_width;
                        S.next_cursor.wrapped_x += S.ch_width;
                        
                        S.first_of_the_line = 0;
                    }
                    
                    S.next_cursor.pos += S.step.byte_length;
                    
                    f32 x = 0, px = 0, y = 0, py = 0;
                    switch (params.seek.type){
                        case buffer_seek_pos:
                        if (S.this_cursor.pos >= params.seek.pos){
                            goto buffer_cursor_seek_end;
                        }break;
                        
                        case buffer_seek_character_pos:
                        if (S.this_cursor.character_pos >= params.seek.pos){
                            goto buffer_cursor_seek_end;
                        }break;
                        
                        case buffer_seek_wrapped_xy:
                        {
                            x = S.this_cursor.wrapped_x;
                            px = S.prev_cursor.wrapped_x;
                            y = S.this_cursor.wrapped_y;
                            py = S.prev_cursor.wrapped_y;
                        }break;
                        
                        case buffer_seek_unwrapped_xy:
                        {
                            x = S.this_cursor.unwrapped_x;
                            px = S.prev_cursor.unwrapped_x;
                            y = S.this_cursor.unwrapped_y;
                            py = S.prev_cursor.wrapped_y;
                        }break;
                        
                        case buffer_seek_line_char:
                        if (S.this_cursor.line == params.seek.line && S.this_cursor.character >= params.seek.character){
                            goto buffer_cursor_seek_end;
                        }
                        else if (S.this_cursor.line > params.seek.line){
                            S.this_cursor = S.prev_cursor;
                            goto buffer_cursor_seek_end;
                        }break;
                    }
                    
                    if (S.xy_seek){
                        if (y > params.seek.y){
                            S.this_cursor = S.prev_cursor;
                            goto buffer_cursor_seek_end;
                        }
                        
                        if (y > params.seek.y - S.font_height && x >= params.seek.x){
                            if (!params.seek.round_down){
                                if (py >= y && !S.behavior.do_newline && (params.seek.x - px) < (x - params.seek.x)){
                                    S.this_cursor = S.prev_cursor;
                                }
                                goto buffer_cursor_seek_end;
                            }
                            
                            if (py >= y){
                                S.this_cursor = S.prev_cursor;
                            }
                            goto buffer_cursor_seek_end;
                        }
                    }
                    
                    if (S.next_cursor.pos > S.size){
                        goto buffer_cursor_seek_end;
                    }
                }
            }
            S.still_looping = buffer_stringify_next(&S.stream);
        }while(S.still_looping);
    }
    
    InvalidCodePath;
    
    buffer_cursor_seek_end:;
    *params.cursor_out = S.this_cursor;
    S_stop.status = BLStatus_Finished;
    DrReturn(S_stop);
}

#undef DrCase
#undef DrYield
#undef DrReturn

internal void
buffer_invert_edit_shift(Gap_Buffer *buffer, Buffer_Edit edit, Buffer_Edit *inverse, char *strings, u32 *str_pos, u32 max, i32 shift_amount){
    u32 pos = *str_pos;
    u32 len = (u32)(edit.end - edit.start);
    assert(pos + len <= max);
    *str_pos = pos + len;
    
    inverse->str_start = pos;
    inverse->len = len;
    inverse->start = edit.start + shift_amount;
    inverse->end = edit.start + edit.len + shift_amount;
    buffer_stringify(buffer, (u32)edit.start, (u32)edit.end, strings + pos);
}

inline void
buffer_invert_edit(Gap_Buffer *buffer, Buffer_Edit edit, Buffer_Edit *inverse, char *strings, u32 *str_pos, u32 max){
    buffer_invert_edit_shift(buffer, edit, inverse, strings, str_pos, max, 0);
}

typedef struct Buffer_Invert_Batch{
    u32 i;
    i32 shift_amount;
    u32 len;
} Buffer_Invert_Batch;

internal b32
buffer_invert_batch(Buffer_Invert_Batch *state, Gap_Buffer *buffer, Buffer_Edit *edits, u32 count, Buffer_Edit *inverse, char *strings, u32 *str_pos, u32 max){
    i32 shift_amount = state->shift_amount;
    u32 i = state->i;
    Buffer_Edit *edit = edits + i;
    Buffer_Edit *inv_edit = inverse + i;
    b32 result = false;
    
    for (; i < count; ++i, ++edit, ++inv_edit){
        if (*str_pos + edit->end - edit->start <= max){
            buffer_invert_edit_shift(buffer, *edit, inv_edit, strings, str_pos, max, shift_amount);
            
            u32 target_length = (u32)(edit->end - edit->start);
            u32 edit_length = (u32)edit->len;
            if (edit_length > target_length){
                shift_amount += (i32)(edit_length - target_length);
            }
            else{
                shift_amount -= (i32)(target_length - edit_length);
            }
        }
        else{
            result = true;
            state->len = (u32)(edit->end - edit->start);
        }
    }
    
    state->i = i;
    state->shift_amount = shift_amount;
    
    return(result);
}

enum Buffer_Render_Flag{
    BRFlag_Special_Character = (1 << 0),
    BRFlag_Ghost_Character = (1 << 1)
};

typedef struct Buffer_Render_Item{
    u32 index;
    u32 codepoint;
    u32 flags;
    f32 x0, y0;
    f32 x1, y1;
} Buffer_Render_Item;

typedef struct Render_Item_Write{
    Buffer_Render_Item *item;
    f32 x, y;
    System_Functions *system;
    Render_Font *font;
    i32 font_height;
    f32 x_min;
    f32 x_max;
} Render_Item_Write;

inline Render_Item_Write
write_render_item(Render_Item_Write write, u32 index, u32 codepoint, u32 flags){
    f32 ch_width = font_get_glyph_advance(write.system, write.font, codepoint);
    
    if (write.x <= write.x_max && write.x + ch_width >= write.x_min){
        write.item->index = index;
        write.item->codepoint = codepoint;
        write.item->flags = flags;
        write.item->x0 = write.x;
        write.item->y0 = write.y;
        write.item->x1 = write.x + ch_width;
        write.item->y1 = write.y + write.font_height;
        
        ++write.item;
    }
    
    write.x += ch_width;
    
    return(write);
}

struct Buffer_Render_Params{
    Gap_Buffer *buffer;
    Buffer_Render_Item *items;
    u32 max;
    u32 *count;
    f32 port_x;
    f32 port_y;
    f32 clip_w;
    f32 scroll_x;
    f32 scroll_y;
    f32 width;
    f32 height;
    Full_Cursor start_cursor;
    i32 wrapped;
    System_Functions *system;
    Render_Font *font;
    b32 virtual_white;
    i32 wrap_slashes;
};

struct Buffer_Render_State{
    Gap_Buffer_Stream stream;
    b32 still_looping;
    i32 i;
    i32 size;
    
    f32 shift_x;
    f32 shift_y;
    
    f32 ch_width;
    
    Render_Item_Write write;
    f32 byte_advance;
    
    i32 line;
    i32 wrap_line;
    i32 wrap_unit_end;
    b32 skipping_whitespace;
    b32 first_of_the_line;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
    
    i32 __pc__;
};

// duff-routine defines
#define DrCase(PC) case PC: goto resumespot_##PC
#define DrYield(PC, n) { *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }
#define DrReturn(n) { *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

internal Buffer_Layout_Stop
buffer_render_data(Buffer_Render_State *S_ptr, Buffer_Render_Params params, f32 line_shift, b32 do_wrap, i32 wrap_unit_end){
    Buffer_Render_State S = *S_ptr;
    Buffer_Layout_Stop S_stop;
    
    Buffer_Render_Item *item_end = params.items + params.max;
    
    switch (S.__pc__){
        DrCase(1);
        DrCase(2);
        DrCase(3);
        DrCase(4);
    }
    
    S.size = buffer_size(params.buffer);
    S.shift_x = params.port_x - params.scroll_x;
    S.shift_y = params.port_y - params.scroll_y;
    
    if (params.wrapped){
        S.shift_y += params.start_cursor.wrapped_y;
    }
    else{
        S.shift_y += params.start_cursor.unwrapped_y;
    }
    
    S.line = (u32)params.start_cursor.line - 1;
    S.wrap_line = (u32)params.start_cursor.wrap_line - 1;
    
    if (params.virtual_white){
        S_stop.status          = BLStatus_NeedLineShift;
        S_stop.line_index      = (u32)(S.line);
        S_stop.wrap_line_index = (u32)(S.wrap_line);
        DrYield(1, S_stop);
    }
    
    S.write.item        = params.items;
    S.write.x           = S.shift_x + line_shift;
    S.write.y           = S.shift_y;
    S.write.system      = params.system;
    S.write.font        = params.font;
    S.write.font_height = font_get_height(params.font);
    S.write.x_min       = params.port_x;
    S.write.x_max       = params.port_x + params.clip_w;
    
    S.byte_advance = font_get_byte_advance(params.font);
    
    if (params.virtual_white){
        S.skipping_whitespace = 1;
    }
    
    S.first_of_the_line = 1;
    S.i = (u32)params.start_cursor.pos;
    if (buffer_stringify_loop(&S.stream, params.buffer, S.i, S.size)){
        do{
            for (; S.i < S.stream.end; ++S.i){
                {
                    u8 ch = (u8)S.stream.data[S.i];
                    translating_fully_process_byte(params.system, params.font, &S.tran, ch, S.i, S.size, &S.emits);
                }
                
                for (TRANSLATION_EMIT_LOOP(S.J, S.emits)){
                    TRANSLATION_GET_STEP(S.step, S.behavior, S.J, S.emits);
                    
                    if (!S.behavior.do_newline && S.step.i >= S.wrap_unit_end){
                        S_stop.status          = BLStatus_NeedWrapDetermination;
                        S_stop.line_index      = (u32)(S.line);
                        S_stop.wrap_line_index = (u32)(S.wrap_line);
                        S_stop.pos             = (u32)(S.step.i);
                        S_stop.x               = S.write.x - S.shift_x;
                        DrYield(4, S_stop);
                        
                        S.wrap_unit_end = wrap_unit_end;
                        
                        if (do_wrap && !S.first_of_the_line){
                            if (params.virtual_white){
                                S_stop.status          = BLStatus_NeedWrapLineShift;
                                S_stop.line_index      = (u32)(S.line);
                                S_stop.wrap_line_index = (u32)(S.wrap_line + 1);
                                DrYield(2, S_stop);
                            }
                            
                            ++S.wrap_line;
                            
                            if (params.wrapped){
                                switch (params.wrap_slashes){
                                    case WrapIndicator_Show_After_Line:
                                    {
                                        S.write = write_render_item(S.write, S.step.i-1, '\\', BRFlag_Ghost_Character);
                                    }break;
                                    
                                    case WrapIndicator_Show_At_Wrap_Edge:
                                    {
                                        if (S.write.x < S.shift_x + params.width){
                                            S.write.x = S.shift_x + params.width;
                                        }
                                        S.write = write_render_item(S.write, S.step.i-1, '\\', BRFlag_Ghost_Character);
                                    }break;
                                }
                                
                                S.write.x = S.shift_x + line_shift;
                                S.write.y += S.write.font_height;
                            }
                        }
                    }
                    
                    if (S.write.y > params.height + params.port_y || S.write.item >= item_end){
                        goto buffer_get_render_data_end;
                    }
                    
                    S.first_of_the_line = false;
                    if (S.behavior.do_newline){
                        S.write = write_render_item(S.write, S.step.i, ' ', 0);
                        
                        if (params.virtual_white){
                            S_stop.status          = BLStatus_NeedLineShift;
                            S_stop.line_index      = (u32)(S.line+1);
                            S_stop.wrap_line_index = (u32)(S.wrap_line+1);
                            DrYield(3, S_stop);
                            
                            S.skipping_whitespace = 1;
                        }
                        
                        ++S.line;
                        ++S.wrap_line;
                        
                        S.write.x = S.shift_x + line_shift;
                        S.write.y += S.write.font_height;
                        
                        S.first_of_the_line = true;
                    }
                    else if (S.behavior.do_codepoint_advance){
                        u32 n = S.step.value;
                        if (n != ' ' && n != '\t'){
                            S.skipping_whitespace = false;
                        }
                        
                        if (!S.skipping_whitespace){
                            u32 I = S.step.i;
                            switch (n){
                                case '\r':
                                {
                                    S.write = write_render_item(S.write, I, '\\', BRFlag_Special_Character);
                                    if (S.write.item < item_end){
                                        S.write = write_render_item(S.write, I, 'r', BRFlag_Special_Character);
                                    }
                                }break;
                                
                                case '\t':
                                {
                                    S.ch_width = font_get_glyph_advance(params.system, params.font, '\t');
                                    
                                    f32 new_x = S.write.x + S.ch_width;
                                    S.write = write_render_item(S.write, I, ' ', 0);
                                    S.write.x = new_x;
                                }break;
                                
                                default:
                                {
                                    S.write = write_render_item(S.write, I, n, 0);
                                }break;
                            }
                        }
                    }
                    else if (S.behavior.do_number_advance){
                        u8 n = (u8)S.step.value;
                        u32 I = S.step.i;
                        S.skipping_whitespace = false;
                        
                        S.ch_width = S.byte_advance;
                        f32 new_x = S.write.x + S.ch_width;
                        
                        u8 cs[3];
                        cs[0] = '\\';
                        byte_to_ascii(n, cs+1);
                        
                        if (S.write.item < item_end){
                            S.write = write_render_item(S.write, I, cs[0], BRFlag_Special_Character);
                            if (S.write.item < item_end){
                                S.write = write_render_item(S.write, I, cs[1], BRFlag_Special_Character);
                                if (S.write.item < item_end){
                                    S.write = write_render_item(S.write, I, cs[2], BRFlag_Special_Character);
                                }
                            }
                        }
                        Assert(S.write.x <= new_x);
                        S.write.x = new_x;
                    }
                    
                    if (!S.skipping_whitespace && !S.behavior.do_newline){
                        S.first_of_the_line = false;
                    }
                }
            }
            S.still_looping = buffer_stringify_next(&S.stream);
        }while(S.still_looping);
    }
    
    buffer_get_render_data_end:;
    if (S.write.y <= params.height + S.shift_y || S.write.item == params.items){
        if (S.write.item < item_end){
            S.write = write_render_item(S.write, S.size, ' ', 0);
        }
    }
    
    *params.count = (i32)(S.write.item - params.items);
    assert(*params.count <= params.max);
    
    S_stop.status = BLStatus_Finished;
    DrReturn(S_stop);
}

#undef DrYield
#undef DrReturn
#undef DrCase

// BOTTOM

