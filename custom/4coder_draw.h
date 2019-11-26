/*
4coder_draw.h - Layout and rendering types of standard UI pieces (including buffers)
*/

// TOP

#if !defined(FCODER_DRAW_H)
#define FCODER_DRAW_H

struct Comment_Highlight_Pair{
    String_Const_u8 needle;
     ARGB_Color color;
};

typedef i32 Range_Highlight_Kind;
enum{
    RangeHighlightKind_LineHighlight,
    RangeHighlightKind_CharacterHighlight,
};

#endif

// BOTTOM

