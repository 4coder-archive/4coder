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

struct Glyph_Data{
    b32 exists;
    
    f32 x0, x1;
    f32 y0, y1;
    
    f32 xoff, yoff;
    f32 xoff2, yoff2;
};

struct Render_Font{
    char name_[24];
    String name;
    b32 loaded;
    
    Glyph_Data glyphs[256];
    f32 byte_advance;
    f32 codepoint_advance_data[256];
    i32 height, ascent, descent, line_skip;
    i32 advance;
    u32 tex;
    i32 tex_width, tex_height;
};

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

#define Font_Load_Sig(name) i32 name(Render_Font *font_out, char *filename, char *fontname, i32 pt_size, i32 tab_width, b32 store_texture)
typedef Font_Load_Sig(Font_Load);

#define Release_Font_Sig(name) void name(Render_Font *font)
typedef Release_Font_Sig(Release_Font);

struct Font_Table_Entry{
    u32 hash;
    String name;
    i16 font_id;
};

struct Font_Info{
    Render_Font *font;
    String filename;
    String name;
    i32 height, advance;
    i32 pt_size;
};

struct Font_Slot{
    Font_Slot *next, *prev;
    i16 font_id;
    u8 padding[14];
};

struct Font_Set{
    Font_Info *info;
    Font_Table_Entry *entries;
    u32 count, max;
    
    void *font_block;
    Font_Slot free_slots;
    Font_Slot used_slots;
    
    //Font_Info_Load *font_info_load;
    Font_Load *font_load;
    Release_Font *release_font;
    
    b8 *font_used_flags;
    i16 used_this_frame;
    i16 live_max;
};

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
    
    // TODO(allen): rename this to font_partition
    Font_Set font_set;
    Partition *partition;
    
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

inline Font_Info*
get_font_info(Font_Set *set, i16 font_id){
    Font_Info *result = set->info + font_id - 1;
    return(result);
}

#endif

// BOTTOM

