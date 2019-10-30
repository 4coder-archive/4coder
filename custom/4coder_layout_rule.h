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
    f32 line_to_text_shift;
    
    Vec2_f32 blank_dim;
    
    Vec2_f32 p;
    f32 line_y;
    f32 text_y;
    f32 width;
};

#endif

// BOTTOM

