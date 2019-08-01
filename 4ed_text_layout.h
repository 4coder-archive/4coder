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

struct Text_Layout{
    // NOTE(allen): This is not a _real_ text layout yet.
    // The eventual destiny of this type is that it will store the fairly
    // costly to generate results of the text layout engine.
    // For now, since the engine cannot be easily consolidated,
    // this just stores the parameters that should be handed to any
    // system that attempts to query the layout for hit testing.
    View_ID view_id;
    Buffer_ID buffer_id;
    Buffer_Point point;
    Range on_screen_range;
    f32 height;
    
    Text_Layout_Coordinates coordinates;
};

union Text_Layout_Node{
    Text_Layout_Node *next;
    Text_Layout layout;
};

struct Text_Layout_Container{
    Arena node_arena;
    Text_Layout_Node *free_nodes;
    u32_Ptr_Table table;
    u32 id_counter;
};

#endif

// BOTTOM

