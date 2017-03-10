/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.03.2017
 *
 * Font data type definitions.
 *
 */

// TOP

#if !defined(FCODER_FONT_DATA_H)
#define FCODER_FONT_DATA_H

#define ITEM_PER_FONT_PAGE 256

struct Glyph_Bounds{
    f32 x0, x1;
    f32 y0, y1;
    f32 xoff, yoff;
    f32 xoff2, yoff2;
};
global_const Glyph_Bounds null_glyph_bounds = {0};

struct Glyph_Data{
    Glyph_Bounds bounds;
    u32 tex;
    i32 tex_width, tex_height;
};

struct Glyph_Page{
    u32 page_number;
    u32 tex;
    i32 tex_width, tex_height;
    Glyph_Bounds glyphs[ITEM_PER_FONT_PAGE];
    f32 advance[ITEM_PER_FONT_PAGE];
};

struct Render_Font{
    char name_[24];
    String name;
    b32 loaded;
    
    Glyph_Page **pages;
    u32 page_count, page_max;
    
    f32 byte_advance;
    i32 height, ascent, descent, line_skip;
    i32 advance;
};

#endif

// BOTTOM


