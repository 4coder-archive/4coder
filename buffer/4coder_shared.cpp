/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 23.10.2015
 *
 * Items shared by gap buffer types
 *
 */

// TOP

// TODO(allen): eliminate the extra defs and the extra include.

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
#define memzero_4tech(x) do{                      \
    char *p = (char*)&x; char *e = p + sizeof(x); \
    for (;p<e; ++p) {*p=0;} }while(0)             
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
internal_4tech i32
lroundup_(i32 x, i32 granularity){
	i32 original_x;
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

typedef struct Buffer_Batch_State{
    i32 i;
    i32 shift_total;
} Buffer_Batch_State;

typedef struct Cursor_With_Index{
    i32 pos;
    i32 index;
} Cursor_With_Index;

inline_4tech void
write_cursor_with_index(Cursor_With_Index *positions, i32 *count, i32 pos){
    positions[*count].index = *count;
    positions[*count].pos = pos;
    ++*count;
}

#define CursorSwap__(a,b) { Cursor_With_Index t = a; a = b; b = t; }

internal_4tech void
buffer_quick_sort_cursors(Cursor_With_Index *positions, i32 start, i32 pivot){
    i32 mid = start;
    i32 pivot_pos = positions[pivot].pos;
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
internal_4tech void
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

inline_4tech void
buffer_sort_cursors(Cursor_With_Index *positions, i32 count){
    assert_4tech(count > 0);
    buffer_quick_sort_cursors(positions, 0, count-1);
}

inline_4tech void
buffer_unsort_cursors(Cursor_With_Index *positions, i32 count){
    assert_4tech(count > 0);
    buffer_quick_unsort_cursors(positions, 0, count-1);
}

internal_4tech void
buffer_update_cursors(Cursor_With_Index *sorted_positions, i32 count, i32 start, i32 end, i32 len){
    i32 shift_amount = (len - (end - start));
    Cursor_With_Index *position = sorted_positions + count - 1;
    
    for (; position >= sorted_positions && position->pos > end; --position) position->pos += shift_amount;
    for (; position >= sorted_positions && position->pos >= start; --position) position->pos = start;
}

internal_4tech i32
buffer_batch_debug_sort_check(Buffer_Edit *sorted_edits, i32 edit_count){
    Buffer_Edit *edit = sorted_edits;
    i32 i = 0, start_point = 0;
    i32 result = 1; 
    
    for (i = 0; i < edit_count; ++i, ++edit){
        if (start_point > edit->start){
            result = 0; break;
        }
        start_point = (edit->end < edit->start + 1)?(edit->start + 1):(edit->end);
    }
    
    return(result);
}

internal_4tech i32
buffer_batch_edit_max_shift(Buffer_Edit *sorted_edits, i32 edit_count){
    Buffer_Edit *edit;
    i32 i, result;
    i32 shift_total, shift_max;
    
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

internal_4tech i32
buffer_batch_edit_update_cursors(Cursor_With_Index *sorted_positions, i32 count, Buffer_Edit *sorted_edits, i32 edit_count){
    Cursor_With_Index *position, *end_position;
    Buffer_Edit *edit, *end_edit;
    i32 start, end;
    i32 shift_amount;
    
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

internal_4tech i32
eol_convert_in(char *dest, char *src, i32 size){
    i32 i, j, k;

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

internal_4tech i32
eol_in_place_convert_in(char *data, i32 size){
    i32 i, j, k;

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

internal_4tech i32
eol_convert_out(char *dest, i32 max, char *src, i32 size, i32 *size_out){
    i32 result;
    i32 i, j;

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

internal_4tech i32
eol_in_place_convert_out(char *data, i32 size, i32 max, i32 *size_out){
    i32 result;
    i32 i;
    
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

inline_4tech i32
is_whitespace(char c){
    i32 result;
    result = (c == ' ' || c == '\n' || c == '\r'  || c == '\t' || c == '\f' || c == '\v');
    return(result);
}

inline_4tech i32
is_alphanumeric_true(char c){
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9');
}

inline_4tech i32
is_alphanumeric(char c){
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_');
}

inline_4tech i32
is_upper(char c){
    return (c >= 'A' && c <= 'Z');
}

inline_4tech i32
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

internal_4tech i32
is_match(char *a, char *b, i32 len){
    i32 result;

    result = 1;
    for (;len > 0; --len, ++a, ++b)
        if (*a != *b) { result = 0; break; }

    return(result);
}

internal_4tech i32
is_match_insensitive(char *a, char *b, i32 len){
    i32 result;

    result = 1;
    for (;len > 0; --len, ++a, ++b)
        if (to_upper(*a) != to_upper(*b)) { result = 0; break; }

    return(result);
}

// BOTTOM

