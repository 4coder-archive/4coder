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

#define FONT_PAGE_ITEMS 256

struct Glyph_Bounds{
    b32 exists;
    
    f32 x0, x1;
    f32 y0, y1;
    
    f32 xoff, yoff;
    f32 xoff2, yoff2;
};

struct Glyph_Page{
    Glyph_Bounds glyphs[256];
    u32 tex;
    i32 tex_width, tex_height;
    b32 exists;
};

struct Glyph_Data{
    Glyph_Bounds bounds;
    u32 tex;
    i32 tex_width, tex_height;
};

struct Advance_Page{
    f32 advance[256];
};

struct Render_Font{
    char name_[24];
    String name;
    b32 loaded;
    
    Glyph_Page glyph_pages[1];
    Advance_Page advance_pages[1];
    
    f32 byte_advance;
    i32 height, ascent, descent, line_skip;
    i32 advance;
};

internal b32
get_codepoint_can_render(Render_Font *font, u32 codepoint){
    b32 exists = false;
    if (codepoint < FONT_PAGE_ITEMS){
        exists = true;
    }
    return(exists);
}

internal u32
get_codepoint_page_index(Render_Font *font, u32 codepoint, u32 *page_base_codepoint){
    *page_base_codepoint = 0;
    return(0);
}

internal void
get_codepoint_memory(Render_Font *font, u32 codepoint, Glyph_Bounds **bounds_mem_out, f32 **advance_mem_out){
    Glyph_Bounds *bounds = 0;
    f32 *advance = 0;
    
    if (get_codepoint_can_render(font, codepoint)){
        u32 base_codepoint = 0;
        u32 page_index = get_codepoint_page_index(font, codepoint, &base_codepoint);
        Glyph_Page *bounds_page = &font->glyph_pages[page_index];
        Advance_Page *advance_page = &font->advance_pages[page_index];
        u32 glyph_index = codepoint - base_codepoint;
        
        bounds = &bounds_page->glyphs[glyph_index];
        advance = &advance_page->advance[glyph_index];
    }
    
    *bounds_mem_out = bounds;
    *advance_mem_out = advance;
}

internal b32
get_codepoint_glyph_data(Render_Font *font, u32 codepoint, Glyph_Data *data_out){
    b32 success = false;
    if (get_codepoint_can_render(font, codepoint)){
        u32 base_codepoint = 0;
        u32 page_index = get_codepoint_page_index(font, codepoint, &base_codepoint);
        Glyph_Page *page = &font->glyph_pages[page_index];
        data_out->bounds = page->glyphs[codepoint - base_codepoint];
        data_out->tex = page->tex;
        data_out->tex_width = page->tex_width;
        data_out->tex_height = page->tex_height;
        success = true;
    }
    return(success);
}

internal f32
get_codepoint_advance(Render_Font *font, u32 codepoint){
    f32 advance = (f32)font->advance;
    if (get_codepoint_can_render(font, codepoint)){
        u32 base_codepoint = 0;
        u32 page_index = get_codepoint_page_index(font, codepoint, &base_codepoint);
        Advance_Page *page = &font->advance_pages[page_index];
        advance = page->advance[codepoint - base_codepoint];
    }
    return(advance);
}

internal b32
set_codepoint_advance(Render_Font *font, u32 codepoint, f32 value){
    b32 success = false;
    if (get_codepoint_can_render(font, codepoint)){
        u32 base_codepoint = 0;
        u32 page_index = get_codepoint_page_index(font, codepoint, &base_codepoint);
        Advance_Page *page = &font->advance_pages[page_index];
        page->advance[codepoint - base_codepoint] = value;
        success = true;
    }
    return(success);
}

#endif

// BOTTOM


