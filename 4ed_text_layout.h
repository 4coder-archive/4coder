/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 31.03.2019
 *
 * Text layout representation
 *
 */

// TOP

#if !defined(FRED_TEXT_LAYOUT_H)
#define FRED_TEXT_LAYOUT_H

union Text_Layout{
    Text_Layout *next;
    struct{
        Arena *arena;
        Buffer_ID buffer_id;
        Buffer_Point point;
        Range_i64 visible_range;
        Range_i64 visible_line_number_range;
        Rect_f32 rect;
        ARGB_Color *item_colors;
        Layout_Function *layout_func;
    };
};

struct Text_Layout_Container{
    Arena node_arena;
    Text_Layout *free_nodes;
    Table_u64_u64 table;
    Text_Layout_ID id_counter;
};

#endif

// BOTTOM

