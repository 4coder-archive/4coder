/*
4coder_layout_rule.h - Built in layout rule types.
*/

// TOP

#if !defined(FCODER_LAYOUT_RULE_H)
#define FCODER_LAYOUT_RULE_H

struct Newline_Layout_Vars{
    i64 newline_character_index;
    b32 consuming_newline_characters;
    b32 prev_did_emit_newline;
};

struct LefRig_TopBot_Layout_Vars{
    Face_Advance_Map  *advance_map;
    Face_Metrics *metrics;
    f32 tab_width;
    f32 line_to_text_shift;
    
    Vec2_f32 blank_dim;
    
    Vec2_f32 p;
    f32 line_y;
    f32 text_y;
    f32 width;
};

struct Layout_Reflex{
    Layout_Item_List *list;
    Buffer_ID buffer;
    f32 width;
    Face_ID face;
};

typedef i32 Layout_Wrap_Kind;
enum{
    Layout_Unwrapped,
    Layout_Wrapped,
};

typedef i32 Layout_Virtual_Indent;
enum{
    LayoutVirtualIndent_Off,
    LayoutVirtualIndent_On,
};

#endif

// BOTTOM

