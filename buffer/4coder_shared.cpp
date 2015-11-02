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

#ifndef inline_4tech
#define inline_4tech inline
#endif

#ifndef internal_4tech
#define internal_4tech static
#endif

#ifndef memset_4tech
#define memset_4tech memset
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

typedef struct{
    int str_start, len;
    int start, end;
} Buffer_Edit;

typedef struct{
    int i;
    int shift_total;
} Buffer_Batch_State;

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

typedef struct{
    int index;
    int glyphid;
    float x0, y0;
    float x1, y1;
} Buffer_Render_Item;

inline_4tech void
write_render_item(Buffer_Render_Item *item, int index, int glyphid,
                  float x, float y, float w, float h){
    item->index = index;
    item->glyphid = glyphid;
    item->x0 = x;
    item->y0 = y;
    item->x1 = x + w;
    item->y1 = y + h;
}

inline_4tech float
write_render_item_inline(Buffer_Render_Item *item, int index, int glyphid,
                         float x, float y, void *advance_data, int stride, float h){
    float ch_width;
    ch_width = measure_character(advance_data, stride, (char)glyphid);
    write_render_item(item, index, glyphid, x, y, ch_width, h);
    return(ch_width);
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
    Cursor_With_Index *position;
    int shift_amount;
    
    shift_amount = (len - (end - start));
    
    position = sorted_positions + count - 1;
    for (; position >= sorted_positions && position->pos >= end; --position) position->pos += shift_amount;
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
eol_convert_in(char *dest, char *src, int size){
    int i, j;
    
    for (i = 0, j = 0; i < size; ++i, ++j){
        if (src[i] == '\r' && i+1 < size && src[i+1] == '\n') ++i;
        dest[j] = src[i];
    }
    
    return(j);
}

internal_4tech int
eol_in_place_convert_in(char *data, int size){
    int i;
    
    for (i = 0; i < size; ++i){
        if (data[i] == '\r' && i+1 < size && data[i+1] == '\n'){
            memmove_4tech(data + i, data + i + 1, size - i);
            size -= 1;
        }
    }
    
    return(size);
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
is_upper(char c){
    return (c >= 'A' && c <= 'Z');
}

inline_4tech int
is_lower(char c){
    return (c >= 'a' && c <= 'z');
}

internal_4tech int
is_match(char *a, char *b, int len){
    int result;

    result = 1;
    for (;len > 0; --len, ++a, ++b)
        if (*a != *b) { result = 0; break; }

    return(result);
}

// BOTTOM

