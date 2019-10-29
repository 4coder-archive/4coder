/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.01.2018
 *
 * Buffer types
 *
 */

// TOP

#if !defined(FRED_BUFFER_H)
#define FRED_BUFFER_H

struct Cursor_With_Index{
    i64 pos;
    i32 index;
};

struct Gap_Buffer{
    Base_Allocator *allocator;
    
    u8 *data;
    i64 size1;
    i64 gap_size;
    i64 size2;
    i64 max;
    
    // NOTE(allen): If there are N lines I store N + 1 slots in this array with
    // line_starts[N] = size of the buffer.
    //    The variable line_start_count stores N + 1; call buffer_line_count(buffer)
    // to get "N" the actual number of lines.
    i64 *line_starts;
    i64 line_start_count;
    i64 line_start_max;
};

struct Buffer_Chunk_Position{
    i64 real_pos;
    i64 chunk_pos;
    i64 chunk_index;
};

typedef i32 Line_Move_Kind;
enum{
    LineMove_ShiftOldValues,
    LineMove_MeasureString,
};
struct Line_Move{
    Line_Move *next;
    Line_Move_Kind kind;
    i64 new_line_first;
    union{
        struct{
            i64 old_line_first;
            i64 old_line_opl;
            i64 text_shift;
        };
        struct{
            String_Const_u8 string;
            i64 text_base;
        };
    };
};

#endif

// BOTTOM
