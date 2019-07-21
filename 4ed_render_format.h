/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Format for 4coder render commands.
 *
 */

// TOP

#if !defined(FRED_RENDER_FORMAT_H)
#define FRED_RENDER_FORMAT_H

enum Render_Command_Type{
    RenCom_Rectangle,
    RenCom_Glyph,
    RenCom_ChangeClip,
};

struct Render_Command_Header{
    union{
        struct{
            i32 size;
            i32 type;
        };
        u64 force_8_byte_align_;
    };
};

struct Render_Command_Rectangle{
    Render_Command_Header header;
    u32 color;
    Vec2 vertices[4];
};

struct Render_Command_Rectangle_Outline{
    Render_Command_Header header;
    u32 color;
    Vec2 vertices[5];
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
    Face_ID font_id;
    u32 codepoint;
    u32 flags;
};

struct Render_Command_Change_Clip{
    Render_Command_Header header;
    i32_Rect box;
};

struct Render_Pseudo_Command_Free_Texture{
    Render_Command_Header header;
    Render_Free_Texture free_texture_node;
};

#endif

// BOTTOM

