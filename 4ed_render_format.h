/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Format for 4coder render commands.
 *
 */

// TOP

enum Render_Piece_Type{
    piece_type_rectangle,
    piece_type_outline,
    piece_type_glyph,
    piece_type_change_clip
};

struct Render_Piece_Header{
    i32 type;
};

struct Render_Piece_Rectangle{
    f32_Rect rect;
    u32 color;
};

struct Render_Piece_Gradient{
    f32_Rect rect;
    u32 left_color;
    u32 right_color;
};

struct Render_Piece_Glyph{
    Vec2 pos;
    u32 color;
    Font_ID font_id;
    u32 codepoint;
};

struct Render_Piece_Glyph_Advance{
    Vec2 pos;
    u32 color;
    f32 advance;
    Font_ID font_id;
    u32 codepoint;
};

struct Render_Piece_Change_Clip{
    i32_Rect box;
};

struct Render_Piece_Combined{
    Render_Piece_Header header;
    union{
        Render_Piece_Rectangle rectangle;
        Render_Piece_Gradient gradient;
        Render_Piece_Glyph glyph;
        Render_Piece_Glyph_Advance glyph_advance;
    };
};

// BOTTOM

