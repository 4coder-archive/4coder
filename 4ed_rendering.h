/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.12.2014
 *
 * Rendering layer for project codename "4ed"
 *
 */

// TOP

#ifndef FRED_RENDERING_H
#define FRED_RENDERING_H

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Glyph_Data{
    b32 exists;
};

struct Render_Font{
    char name_[24];
    String name;
    b32 loaded;
    
	Glyph_Data glyphs[256];
    stbtt_bakedchar chardata[256];
    float advance_data[256];
	i32 height, ascent, descent, line_skip;
    i32 advance;
    u32 tex;
    i32 tex_width, tex_height;
};

struct Render_Target;

#define Draw_Push_Clip_Sig(name) void name(Render_Target *target, i32_Rect clip_box)
typedef Draw_Push_Clip_Sig(Draw_Push_Clip);

#define Draw_Pop_Clip_Sig(name) void name(Render_Target *target)
typedef Draw_Pop_Clip_Sig(Draw_Pop_Clip);

enum Render_Piece_Type{
    piece_type_rectangle,
    piece_type_outline,
    piece_type_gradient,
    piece_type_glyph,
    piece_type_mono_glyph,
    piece_type_mono_glyph_advance
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
    u32 left_color, right_color;
};

struct Render_Piece_Glyph{
    Vec2 pos;
    u32 color;
    Render_Font *font_id;
    u16 character;
};

struct Render_Piece_Glyph_Advance{
    Vec2 pos;
    u32 color;
    f32 advance;
    Render_Font *font_id;
    u16 character;
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

#define Draw_Push_Piece_Sig(name) void name(Render_Target *target, Render_Piece_Combined piece)
typedef Draw_Push_Piece_Sig(Draw_Push_Piece);

#define Font_Load_Sig(name) i32 name(                                   \
        System_Functions *system,                                       \
        Render_Font *font_out,                                          \
        char *filename,                                                 \
        i32 pt_size,                                                    \
        void *font_block,                                               \
        i32 font_block_size,                                            \
        i32 *memory_used_out,                                           \
        i32 tab_width)
typedef Font_Load_Sig(Font_Load);

struct Render_Target{
    void *handle;
    void *context;
    i32_Rect clip_boxes[5];
    i32 clip_top;
	i32 width, height;
    i32 bound_texture;
    u32 color;
    
    byte *push_buffer;
    i32 size, max;
    
    Draw_Push_Clip *push_clip;
    Draw_Pop_Clip *pop_clip;
    Draw_Push_Piece *push_piece;
    Font_Load *font_load;
};

inline i32_Rect
rect_from_target(Render_Target *target){
	return i32R(0, 0, target->width, target->height);
}

#endif

// BOTTOM

