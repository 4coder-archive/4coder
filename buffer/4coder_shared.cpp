/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 23.10.2015
 * 
 * Items shared by gap buffer types
 * 
 */

// TOP

#include "../4coder_seek_types.h"

#ifndef inline_4tech
#define inline_4tech inline
#endif

#ifndef internal_4tech
#define internal_4tech static
#endif

#ifndef memset_4tech
#define memset_4tech memset
#endif

#ifndef memzero_4tech
#define memzero_4tech(x) ((x) = {})
#endif

#ifndef memcpy_4tech
#define memcpy_4tech memcpy
#endif

#ifndef memmove_4tech
#define memmove_4tech memmove
#endif

#ifndef debug_4tech
#define debug_4tech(x) x
#endif

#ifndef assert_4tech
#define assert_4tech assert
#endif

#ifndef ceil_4tech
#define ceil_4tech CEIL32
#endif

#ifndef div_ceil_4tech
#define div_ceil_4tech DIVCEIL32
#endif

#ifndef cat_4tech
#define cat_4tech_(a,b) a##b
#define cat_4tech(a,b) cat_4tech_(a,b)
#endif

#ifndef round_up_4tech
internal_4tech int
lroundup_(int x, int granularity){
	int original_x;
    original_x = x;
	x /= granularity;
	x *= granularity;
	if (x < original_x) x += granularity;
	return x;
}
#define round_up_4tech(x,g) lroundup_(x,g)
#endif

#ifndef round_pot_4tech
#define round_pot_4tech ROUNDPOT32
#endif

#define measure_character(a,c) ((a)[c])

typedef struct Buffer_Batch_State{
    int i;
    int shift_total;
} Buffer_Batch_State;

inline_4tech Full_Cursor
make_cursor_hint(int line_index, int *starts, float *wrap_ys, float font_height){
    Full_Cursor hint;
    hint.pos = starts[line_index];
    hint.line = line_index + 1;
    hint.character = 1;
    hint.unwrapped_y = (float)(line_index * font_height);
    hint.unwrapped_x = 0;
    hint.wrapped_y = wrap_ys[line_index];
    hint.wrapped_x = 0;
    return(hint);
}

typedef struct Cursor_With_Index{
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
buffer_sort_cursors(Cursor_With_Index *positions, int count){
    assert_4tech(count > 0);
    buffer_quick_sort_cursors(positions, 0, count-1);
}

inline_4tech void
buffer_unsort_cursors(Cursor_With_Index *positions, int count){
    assert_4tech(count > 0);
    buffer_quick_unsort_cursors(positions, 0, count-1);
}

internal_4tech void
buffer_update_cursors(Cursor_With_Index *sorted_positions, int count, int start, int end, int len){
    int shift_amount = (len - (end - start));
    Cursor_With_Index *position = sorted_positions + count - 1;
    
    for (; position >= sorted_positions && position->pos > end; --position) position->pos += shift_amount;
    for (; position >= sorted_positions && position->pos >= start; --position) position->pos = start;
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

internal_4tech int
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
        
        for (; position->pos <= end && position < end_position; ++position){
            position->pos = start + shift_amount;
        }
        
        shift_amount += (edit->len - (end - start));
    }
    
    for (; position < end_position; ++position){
        position->pos += shift_amount;
    }
    
    for (; edit < end_edit; ++edit){
        shift_amount += (edit->len - (edit->end - edit->start));
    }
    
    return(shift_amount);
}

internal_4tech int
eol_convert_in(char *dest, char *src, int size){
    int i, j, k;

    i = 0;
    k = 0;
    j = 0;
    
    for (; j < size && src[j] != '\r'; ++j);
    memcpy_4tech(dest, src, j);
    
    if (j < size){
        k = 1;
        ++j;
        for (i = j; i < size; ++i){
            if (src[i] == '\r'){
                memcpy_4tech(dest + j - k, src + j, i - j);
                ++k;
                j = i+1;
            }
        }
        memcpy_4tech(dest + j - k, src + j, i - j);
        j = i - k;
    }
    
    return(j);
}

internal_4tech int
eol_in_place_convert_in(char *data, int size){
    int i, j, k;

    i = 0;
    k = 0;
    j = 0;
    
    for (; j < size && data[j] != '\r'; ++j);
    
    if (j < size){
        k = 1;
        ++j;
        for (i = j; i < size; ++i){
            if (data[i] == '\r'){
                memmove_4tech(data + j - k, data + j, i - j);
                ++k;
                j = i+1;
            }
        }
        memmove_4tech(data + j - k, data + j, i - j);
        j = i - k;
    }
    
    return(j);
}

internal_4tech int
eol_convert_out(char *dest, int max, char *src, int size, int *size_out){
    int result;
    int i, j;

    // TODO(allen): iterative memory check?
    result = 1;
    i = 0;
    j = 0;

    for (; i < size; ++i, ++j){
        if (src[i] == '\n'){
            dest[j] = '\r';
            ++j;
            dest[j] = '\n';
        }
        else dest[j] = src[i];
    }

    *size_out = j;
    return(result);
}

internal_4tech int
eol_in_place_convert_out(char *data, int size, int max, int *size_out){
    int result;
    int i;
    
    // TODO(allen): iterative memory check?
    result = 1;
    i = 0;
    
    for (; i < size; ++i){
        if (data[i] == '\n'){
            memmove_4tech(data + i + 1, data + i, size - i);
            data[i] = '\r';
            ++i;
            ++size;
        }
    }
    
    *size_out = size;
    return(result);
}

inline_4tech int
is_whitespace(char c){
    int result;
    result = (c == ' ' || c == '\n' || c == '\r'  || c == '\t' || c == '\f' || c == '\v');
    return(result);
}

inline_4tech int
is_alphanumeric_true(char c){
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9');
}

inline_4tech int
is_alphanumeric(char c){
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_');
}

inline_4tech int
is_upper(char c){
    return (c >= 'A' && c <= 'Z');
}

inline_4tech int
is_lower(char c){
    return (c >= 'a' && c <= 'z');
}

inline_4tech char
to_upper(char c){
    if (is_lower(c)){
        c += 'A' - 'a';
    }
    return(c);
}

internal_4tech int
is_match(char *a, char *b, int len){
    int result;

    result = 1;
    for (;len > 0; --len, ++a, ++b)
        if (*a != *b) { result = 0; break; }

    return(result);
}

internal_4tech int
is_match_insensitive(char *a, char *b, int len){
    int result;

    result = 1;
    for (;len > 0; --len, ++a, ++b)
        if (to_upper(*a) != to_upper(*b)) { result = 0; break; }

    return(result);
}

// BOTTOM

