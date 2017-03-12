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

struct Glyph_Page{
    u32 page_number;
    f32 advance[ITEM_PER_FONT_PAGE];
    Glyph_Bounds glyphs[ITEM_PER_FONT_PAGE];
    u32 tex;
    i32 tex_width, tex_height;
};

#define FONT_PAGE_EMPTY   ((Glyph_Page*)0)
#define FONT_PAGE_DELETED ((Glyph_Page*)(1))
#define FONT_PAGE_MAX     0x1100

struct Render_Font{
    Glyph_Page **pages;
    u32 page_count, page_max;
    f32 byte_advance;
    i32 height, ascent, descent, line_skip, advance;
    
    char filename[256];
    char name[256];
    u32 filename_len;
    u32 name_len;
};

struct Glyph_Data{
    Glyph_Bounds bounds;
    u32 tex;
    i32 tex_width, tex_height;
};

#endif

// BOTTOM


