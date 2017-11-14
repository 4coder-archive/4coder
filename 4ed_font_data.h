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
    
    b32 has_layout;
    f32 advance[ITEM_PER_FONT_PAGE];
    Glyph_Bounds glyphs[ITEM_PER_FONT_PAGE];
    i32 tex_width, tex_height;
    
    b32 has_gpu_setup;
    u32 gpu_tex;
};

#define FONT_PAGE_EMPTY   ((Glyph_Page*)0)
#define FONT_PAGE_DELETED ((Glyph_Page*)(1))
#define FONT_PAGE_MAX     0x1100

struct Render_Font{
    Glyph_Page **pages;
    u32 page_count, page_max;
    f32 byte_advance;
    f32 byte_sub_advances[3];
    i32 height, ascent, descent, line_skip, advance;
    i32 pt_size;
    b32 use_hinting;
    
    u32 filename_len;
    u32 name_len;
    char filename[256];
    char name[256];
    
    // Hack optimizations
    struct Page_Cache{
        u32 page_number;
        Glyph_Page *page;
    };
    
    Page_Cache cache[16];
};

struct Glyph_Data{
    Glyph_Bounds bounds;
    b32 has_gpu_setup;
    u32 tex;
    i32 tex_width, tex_height;
};

#endif

// BOTTOM

