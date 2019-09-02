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

typedef u32 Buffer_Layout_Flag;
enum{
    BRFlag_Special_Character = (1 << 0),
    BRFlag_Ghost_Character = (1 << 1)
};

struct Buffer_Layout_Item{
    i64 index;
    u32 codepoint;
    Buffer_Layout_Flag flags;
    Rect_f32 rect;
};

struct Buffer_Layout_Item_Block{
    Buffer_Layout_Item_Block *next;
    Buffer_Layout_Item *items;
    i64 count;
    i64 character_count;
};

struct Buffer_Layout_Item_List{
    Buffer_Layout_Item_Block *first;
    Buffer_Layout_Item_Block *last;
    i32 node_count;
    i32 total_count;
    f32 height;
    i64 character_count;
    Interval_i64 index_range;
};

#endif

// BOTTOM
