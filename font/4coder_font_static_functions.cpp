/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Implements some basic getters for fonts set up to make the font type opaque.
 *
 */

// TOP

#include "4coder_font_data.h"

internal f32
font_get_byte_advance(Render_Font *font){
    return(font->byte_advance);
}

internal i32
font_get_height(Render_Font *font){
    return(font->height);
}

internal i32
font_get_ascent(Render_Font *font){
    return(font->ascent);
}

internal i32
font_get_descent(Render_Font *font){
    return(font->descent);
}

internal i32
font_get_line_skip(Render_Font *font){
    return(font->line_skip);
}

internal i32
font_get_advance(Render_Font *font){
    return(font->advance);
}

internal b32
font_can_render(System_Functions *system, Render_Font *font, u32 codepoint){
    b32 result = false;
    u32 page_number = (codepoint >> 8);
    u32 glyph_index = codepoint & 0xFF;
    
    Glyph_Page *page = font_get_or_make_page(system, font, page_number);
    if (page != 0 && page->advance[glyph_index] > 0.f){
        result = true;
    }
    
    return(result);
}

internal Glyph_Page**
font_page_lookup(Render_Font *font, u32 page_number, b32 get_empty_slot){
    Glyph_Page **result = 0;
    
    if (font->page_max > 0){
        u32 first_index = page_number % font->page_max;
        
        u32 range_count = 0;
        u32 ranges[4];
        if (first_index == 0){
            ranges[0] = 0;
            ranges[1] = font->page_max;
            range_count = 2;
        }
        else{
            ranges[0] = first_index;
            ranges[1] = font->page_max;
            ranges[2] = 0;
            ranges[3] = first_index;
            range_count = 4;
        }
        
        Glyph_Page **pages = font->pages;
        if (get_empty_slot){
            for (u32 j = 0; j < range_count; j += 2){
                u32 stop = ranges[j+1];
                for (u32 i = ranges[j]; i < stop; ++i){
                    if (pages[i] == FONT_PAGE_EMPTY || pages[i] == FONT_PAGE_DELETED){
                        result = &pages[i];
                        goto break2;
                    }
                    if (pages[i]->page_number == page_number){
                        goto break2;
                    }
                }
            }
        }
        else{
            for (u32 j = 0; j < range_count; j += 2){
                u32 stop = ranges[j+1];
                for (u32 i = ranges[j]; i < stop; ++i){
                    if (pages[i] == FONT_PAGE_EMPTY){
                        goto break2;
                    }
                    if (pages[i] != FONT_PAGE_DELETED && pages[i]->page_number == page_number){
                        result = &pages[i];
                        goto break2;
                    }
                }
            }
        }
        
        break2:;
    }
    
    return(result);
}

internal Glyph_Page*
font_get_or_make_page(System_Functions *system, Render_Font *font, u32 page_number){
    Glyph_Page *result = 0;
    if (page_number <= 0x10FF){
        Glyph_Page **page_get_result = font_page_lookup(font, page_number, false);
        
        if (page_get_result == 0){
            b32 has_space = true;
            u32 new_page_count = 1;
            u32 new_max = (font->page_count+new_page_count)*3;
            if (font->page_max < FONT_PAGE_MAX && new_max > font->page_max*2){
                Glyph_Page **pages = (Glyph_Page**)system->font.allocate(sizeof(Glyph_Page*)*new_max);
                has_space = false;
                if (pages != 0){
                    memset(pages, 0, sizeof(*pages)*new_max);
                    u32 old_max = font->page_max;
                    for (u32 i = 0; i < old_max; ++i){
                        Glyph_Page *this_page = pages[i];
                        if (this_page != FONT_PAGE_EMPTY && this_page != FONT_PAGE_DELETED){
                            u32 this_page_number = this_page->page_number;
                            Glyph_Page **dest = font_page_lookup(font, this_page_number, true);
                            Assert(dest != 0);
                            *dest = this_page;
                        }
                    }
                    system->font.free(font->pages);
                    font->pages = pages;
                    font->page_max = new_max;
                    has_space = true;
                }
            }
            
            if (has_space){
                Glyph_Page *new_page = (Glyph_Page*)system->font.allocate(sizeof(Glyph_Page));
                if (new_page != 0){
                    Glyph_Page **dest = font_page_lookup(font, page_number, true);
                    Assert(dest != 0);
                    *dest = new_page;
                    font->page_count += new_page_count;
                    result = new_page;
                }
            }
        }
    }
    return(result);
}

// BOTTOM



