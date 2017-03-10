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

//
// Render Commands
//

enum Render_Piece_Type{
    piece_type_rectangle,
    piece_type_outline,
    piece_type_gradient,
    piece_type_glyph,
    piece_type_mono_glyph,
    piece_type_mono_glyph_advance,
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
    i16 font_id;
    u8 character;
};

struct Render_Piece_Glyph_Advance{
    Vec2 pos;
    u32 color;
    f32 advance;
    i16 font_id;
    u8 character;
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

struct Render_Target;

#define Draw_Push_Clip_Sig(name) void name(Render_Target *target, i32_Rect clip_box)
typedef Draw_Push_Clip_Sig(Draw_Push_Clip);

#define Draw_Pop_Clip_Sig(name) i32_Rect name(Render_Target *target)
typedef Draw_Pop_Clip_Sig(Draw_Pop_Clip);

#define Draw_Push_Piece_Sig(name) void name(Render_Target *target, Render_Piece_Combined piece)
typedef Draw_Push_Piece_Sig(Draw_Push_Piece);

//
// Render target stuff
//

struct Render_Target{
    void *handle;
    void *context;
    i32_Rect clip_boxes[5];
    i32 clip_top;
    b32 clip_all;
    i32 width, height;
    i32 bound_texture;
    u32 color;
    
    // TODO(allen): change this to a Partition
    char *push_buffer;
    i32 size, max;
    
    Draw_Push_Clip *push_clip;
    Draw_Pop_Clip *pop_clip;
    Draw_Push_Piece *push_piece;
};

#define DpiMultiplier(n,dpi) ((n) * (dpi) / 96)

inline i32_Rect
rect_from_target(Render_Target *target){
    i32_Rect r;
    r.x0 = 0;
    r.y0 = 0;
    r.x1 = target->width;
    r.y1 = target->height;
    return(r);
}

#endif

// BOTTOM

