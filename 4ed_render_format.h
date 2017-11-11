/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Format for 4coder render commands.
 *
 */

// TOP

enum Render_Command_Type{
    RenCom_Rectangle,
    RenCom_Outline,
    RenCom_Glyph,
    RenCom_ChangeClip
};

struct Render_Command_Header{
    i32 size;
    i32 type;
};

struct Render_Command_Rectangle{
    Render_Command_Header header;
    f32_Rect rect;
    u32 color;
};

struct Render_Command_Gradient{
    Render_Command_Header header;
    f32_Rect rect;
    u32 left_color;
    u32 right_color;
};

struct Render_Command_Glyph{
    Render_Command_Header header;
    Vec2 pos;
    u32 color;
    Font_ID font_id;
    u32 codepoint;
};

struct Render_Command_Change_Clip{
    Render_Command_Header header;
    i32_Rect box;
};

// BOTTOM

