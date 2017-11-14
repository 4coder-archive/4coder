/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.11.2017
 *
 * FreeType font loader implementation
 *
 */

// TOP

// NOTE(allen): Thanks to insofaras.  This is copy-pasted from some work he originally did to get free type working on Linux.

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

internal u32
font_ft_flags(b32 use_hinting){
    u32 ft_flags = FT_LOAD_RENDER;
    
    if (use_hinting){
        // NOTE(inso): FT_LOAD_TARGET_LIGHT does hinting only vertically, which looks nicer imo
        // maybe it could be exposed as an option for hinting, instead of just on/off.
        ft_flags |= FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
    }
    else{
        ft_flags |= (FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_HINTING);
    }
    
    return(ft_flags);
}

internal void
font_load_page_layout(Render_Font *font, Glyph_Page *page, u32 page_number, u32 pt_size, b32 use_hinting){
    Assert(page != 0);
    memset(page, 0, sizeof(*page));
    
    char *filename = font->filename;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    page->page_number = page_number;
    
    i32 tab_width = 4;
    
    // NOTE(allen): Determine glyph layout dimensions
    i32 max_glyph_w = face->size->metrics.x_ppem;
    i32 max_glyph_h = font_get_height(font);
    i32 pen_y_descent = max_glyph_h + 2;
    i32 tex_width   = 64;
    i32 tex_height  = 0;
    
    do {
        tex_width *= 2;
        f32 glyphs_per_row = ceilf(tex_width/(f32)max_glyph_w);
        f32 rows = ceilf(ITEM_PER_FONT_PAGE/glyphs_per_row);
        tex_height = ceil32(rows*pen_y_descent);
    } while(tex_height > tex_width);
    tex_height = round_up_pot_u32(tex_height);
    
    i32 pen_x = 0;
    i32 pen_y = 0;
    
    // NOTE(allen): Fill the glyph bounds array
    u32 ft_flags = font_ft_flags(use_hinting);
    
    u32 codepoint = (page_number << 8);
    Glyph_Bounds *glyph_out = &page->glyphs[0];
    f32 *advance_out = &page->advance[0];
    for (u32 i = 0; i < ITEM_PER_FONT_PAGE; ++i, ++codepoint, ++glyph_out, ++advance_out){
        if (FT_Load_Char(face, codepoint, ft_flags) == 0){
            i32 w = face->glyph->bitmap.width;
            i32 h = face->glyph->bitmap.rows;
            i32 ascent = font_get_ascent(font);
            
            // NOTE(allen): Move to next line if necessary
            if (pen_x + w >= tex_width){
                pen_x = 0;
                pen_y += pen_y_descent;
            }
            
            // NOTE(allen): Set all this stuff the renderer needs
            glyph_out->x0 = (f32)(pen_x);
            glyph_out->y0 = (f32)(pen_y);
            glyph_out->x1 = (f32)(pen_x + w);
            glyph_out->y1 = (f32)(pen_y + h + 1);
            
            glyph_out->xoff = (f32)(face->glyph->bitmap_left);
            glyph_out->yoff = (f32)(ascent - face->glyph->bitmap_top);
            glyph_out->xoff2 = glyph_out->xoff + w;
            glyph_out->yoff2 = glyph_out->yoff + h + 1;
            
            // TODO(allen): maybe advance data should be integers?
            *advance_out = (f32)ceil32(face->glyph->advance.x / 64.0f);
            
            pen_x = ceil32(glyph_out->x1 + 1);
        }
    }
    
    // TODO(allen): Not sure setting tex_height here is right... double check.
    tex_height = round_up_pot_u32(pen_y + pen_y_descent);
    
    page->tex_width  = tex_width;
    page->tex_height = tex_height;
    page->has_layout = true;
    
    // HACK(allen): Put this somewhere else!
    // NOTE(allen): whitespace spacing stuff
    if (page_number == 0){
        f32 space_adv = page->advance[' '];
        f32 backslash_adv = page->advance['\\'];
        f32 r_adv = page->advance['r'];
        
        page->advance['\n'] = space_adv;
        page->advance['\r'] = backslash_adv + r_adv;
        page->advance['\t'] = space_adv*tab_width;
    }
    
    FT_Done_FreeType(ft);
}

internal u32*
font_load_page_pixels(Partition *part, Render_Font *font, Glyph_Page *page, u32 page_number, i32 *tex_width_out, i32 *tex_height_out){
    Assert(page != 0);
    Assert(page->has_layout);
    Assert(page->page_number == page_number);
    
    char *filename = font->filename;
    i32 pt_size = font->pt_size;
    b32 use_hinting = font->use_hinting;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    page->page_number = page_number;
    
    // NOTE(allen): Prepare a pixel buffer.
    i32 tex_width   = page->tex_width;
    i32 tex_height  = page->tex_height;
    
    u32* pixels = push_array(part, u32, tex_width*tex_height);
    memset(pixels, 0, tex_width*tex_height*sizeof(u32));
    
    // NOTE(allen): Fill the texture
    u32 ft_flags = font_ft_flags(use_hinting);
    
    u32 codepoint = (page_number << 8);
    Glyph_Bounds *glyph_ptr = &page->glyphs[0];
    for (u32 i = 0; i < ITEM_PER_FONT_PAGE; ++i, ++codepoint, ++glyph_ptr){
        if (FT_Load_Char(face, codepoint, ft_flags) == 0){
            // NOTE(allen): Extract this glyph's dimensions.
            i32 x = (i32)glyph_ptr->x0;
            i32 y = (i32)glyph_ptr->y0;
            i32 w = (i32)(glyph_ptr->x1 - glyph_ptr->x0);
            i32 h = (i32)(glyph_ptr->y1 - glyph_ptr->y0 - 1);
            
            // NOTE(allen): Write to the pixels.
            u8 *src = face->glyph->bitmap.buffer;
            i32 pitch = face->glyph->bitmap.pitch;
            i32 end_x = x + w;
            i32 end_y = y + h;
            for (i32 Y = y, YY = 0; Y < end_y; ++Y, ++YY){
                for (i32 X = x, XX = 0; X < end_x; ++X, ++XX){
                    pixels[Y*tex_width + X] = 0x00FFFFFF + (0x01000000*src[YY*pitch + XX]);
                }
            }
        }
    }
    
    *tex_width_out = tex_width;
    *tex_height_out = tex_height;
    
    FT_Done_FreeType(ft);
    
    return(pixels);
}

internal b32
font_load(System_Functions *system, Partition *part, Render_Font *font, i32 pt_size, b32 use_hinting){
    char *filename = font->filename;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    // NOTE(allen): Set size and metrics
    char *name = face->family_name;
    u32 name_len = 0;
    for (;name[name_len];++name_len);
    name_len = clamp_top(name_len, sizeof(font->name)-1);
    memcpy(font->name, name, name_len);
    font->name[name_len] = 0;
    font->name_len = name_len;
    
    font->ascent    = ceil32  (face->size->metrics.ascender    / 64.0f);
    font->descent   = floor32 (face->size->metrics.descender   / 64.0f);
    font->advance   = ceil32  (face->size->metrics.max_advance / 64.0f);
    font->height    = ceil32  (face->size->metrics.height      / 64.0f);
    font->line_skip = font->height - (font->ascent - font->descent);
    
    font->height -= font->line_skip;
    font->line_skip = 0;
    
    font->pt_size = pt_size;
    font->use_hinting = use_hinting;
    
    // NOTE(allen): Set texture and glyph data.
    Glyph_Page *page = font_get_or_make_page(system, font, 0);
    
    // NOTE(allen): Setup some basic spacing stuff.
    f32 backslash_adv = page->advance['\\'];
    f32 max_hex_advance = 0.f;
    for (u32 i = '0'; i <= '9'; ++i){
        f32 adv = page->advance[i];
        max_hex_advance = Max(max_hex_advance, adv);
    }
    for (u32 i = 'a'; i <= 'f'; ++i){
        f32 adv = page->advance[i];
        max_hex_advance = Max(max_hex_advance, adv);
    }
    for (u32 i = 'A'; i <= 'F'; ++i){
        f32 adv = page->advance[i];
        max_hex_advance = Max(max_hex_advance, adv);
    }
    
    font->byte_advance = backslash_adv + max_hex_advance*2;
    font->byte_sub_advances[0] = backslash_adv;
    font->byte_sub_advances[1] = max_hex_advance;
    font->byte_sub_advances[2] = max_hex_advance;
    
    FT_Done_FreeType(ft);
    
    return(true);
}

// TODO(allen): Remove Partition
internal void
system_set_page(System_Functions *system, Partition *part, Render_Font *font, Glyph_Page *page, u32 page_number, u32 pt_size, b32 use_hinting){
    Assert(pt_size >= 8);
    font_load_page_layout(font, page, page_number, pt_size, use_hinting);
}

internal void
system_set_font(System_Functions *system, Partition *part, Render_Font *font, char *filename, u32 pt_size, b32 use_hinting){
    memset(font, 0, sizeof(*font));
    
    u32 filename_len = 0;
    for (;filename[filename_len];++filename_len);
    
    if (filename_len <= sizeof(font->filename) - 1){
        memcpy(font->filename, filename, filename_len);
        font->filename[filename_len] = 0;
        font->filename_len = filename_len;
        
        if (part->base == 0){
            *part = sysshared_scratch_partition(MB(8));
        }
        
        b32 success = false;
        for (u32 R = 0; R < 3; ++R){
            success = font_load(system, part, font, pt_size, use_hinting);
            if (success){
                break;
            }
            else{
                sysshared_partition_double(part);
            }
        }
    }
    else{
        LOGF("font \"%.*s\" name is too long to load in current build (max %u)\n", filename_len, filename, sizeof(font->filename) - 1);
    }
}

// BOTTOM

