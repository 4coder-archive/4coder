// NOTE(allen): Thanks to insofaras.
// This is copy-pasted from some work he
// did to get free type working on linux.
// Once it is working on both sides it might
// be possible to pull some parts out as
// portable FT rendering.


#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#define internal static

internal u32
next_pow_of_2(u32 v){
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}

#define NUM_GLYPHS 128
#define ENABLE_LCD_FILTER 0

internal b32
win32_ft_font_load(Partition *part, Render_Font *rf, char *name, i32 pt_size, i32 tab_width){
    
    Temp_Memory temp = begin_temp_memory(part);
    
    char* filename = push_array(part, char, 256);
    String str = make_string(filename, 0, 256);
    sysshared_to_binary_path(&str, name);
    
    memset(rf, 0, sizeof(*rf));
    
    //TODO(inso): put stuff in linuxvars / init in main
    FT_Library ft;
    FT_Face face;
    b32 use_lcd_filter = 0;
    
    FT_Init_FreeType(&ft);
    
    //NOTE(inso): i'm not sure the LCD filter looks better, and it doesn't work perfectly with the coloring stuff
#if ENABLE_LCD_FILTER
    if(FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_DEFAULT) == 0){
        puts("LCD Filter on");
        use_lcd_filter = 1;
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
#endif
    
    FT_New_Face(ft, filename, 0, &face);
    
    // set size & metrics
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    rf->loaded    = 1;
    rf->ascent    = CEIL32  (face->size->metrics.ascender    / 64.0f);
    rf->descent   = FLOOR32 (face->size->metrics.descender   / 64.0f);
    rf->advance   = CEIL32  (face->size->metrics.max_advance / 64.0f);
    rf->height    = CEIL32  (face->size->metrics.height      / 64.0f);
    rf->line_skip = rf->height - (rf->ascent - rf->descent);
    
    rf->height -= rf->line_skip;
    rf->line_skip = 0;
    
    int max_glyph_w = face->size->metrics.x_ppem;
    int max_glyph_h = rf->height;
    int tex_width   = 64;
    int tex_height  = 0;
    
    // estimate upper bound on texture width
    do {
        tex_width *= 2;
        float glyphs_per_row = ceilf(tex_width / (float) max_glyph_w);
        float rows = ceilf(NUM_GLYPHS / glyphs_per_row);
        tex_height = CEIL32(rows * (max_glyph_h + 2));
    } while(tex_height > tex_width);
    
    tex_height = next_pow_of_2(tex_height);
    
    int pen_x = 0;
    int pen_y = 0;
    
    u32* pixels = push_array(part, u32, tex_width * tex_height);
    memset(pixels, 0, tex_width * tex_height * sizeof(u32));
    
    // XXX: test if AUTOHINT looks better or not
    const u32 ft_extra_flags = use_lcd_filter ? FT_LOAD_TARGET_LCD : FT_LOAD_FORCE_AUTOHINT;
    
    for(int i = 0; i < NUM_GLYPHS; ++i){
        if(FT_Load_Char(face, i, FT_LOAD_RENDER | ft_extra_flags) != 0) continue;
        
        int w = face->glyph->bitmap.width;
        int h = face->glyph->bitmap.rows;
        
        // lcd filter produces RGB bitmaps, need to account for the extra components
        if(use_lcd_filter){
            w /= 3;
        }
        
        // move to next line if necessary
        if(pen_x + w >= tex_width){
            pen_x = 0;
            pen_y += (max_glyph_h + 2);
        }
        
        // set all this stuff the renderer needs
        Glyph_Data* c = rf->glyphs + i;
        
        c->x0 = (f32)(pen_x);
        c->y0 = (f32)(pen_y);
        c->x1 = (f32)(pen_x + w);
        c->y1 = (f32)(pen_y + h + 1);
        
        c->xoff = (f32)(face->glyph->bitmap_left);
        c->yoff = (f32)(rf->ascent - face->glyph->bitmap_top);
        
        c->xoff2 = w + c->xoff;
        c->yoff2 = h + c->yoff + 1;
        
        rf->advance_data[i] = (f32)(face->glyph->advance.x >> 6);
        
        rf->glyphs[i].exists = 1;
        
        
        int pitch = face->glyph->bitmap.pitch;
        
        // write to texture atlas
        for(int j = 0; j < h; ++j){
            for(int i = 0; i < w; ++i){
                int x = pen_x + i;
                int y = pen_y + j;
                
                if(use_lcd_filter){
                    u8 r = face->glyph->bitmap.buffer[j * pitch + i * 3];
                    u8 g = face->glyph->bitmap.buffer[j * pitch + i * 3 + 1];
                    u8 b = face->glyph->bitmap.buffer[j * pitch + i * 3 + 2];
                    u8 a = (u8)ROUND32((r + g + b) / 3.0f);
                    
                    pixels[y * tex_width + x] = (a << 24) | (r << 16) | (g << 8) | b;
                } else {
                    pixels[y * tex_width + x] = face->glyph->bitmap.buffer[j * pitch + i] * 0x1010101;
                }
            }
        }
        
        pen_x = CEIL32(c->x1 + 1);
    }
    
    Glyph_Data space_glyph = rf->glyphs[' '];
    f32 space_width = rf->advance_data[' '];
    
    rf->glyphs['\r'] = space_glyph;
    rf->advance_data['\r'] = space_width*tab_width;
    
    rf->glyphs['\n'] = space_glyph;
    rf->advance_data['\n'] = space_width*tab_width;
    
    rf->glyphs['\t'] = space_glyph;
    rf->advance_data['\t'] = space_width*tab_width;
    
    FT_Done_FreeType(ft);
    
    tex_height = next_pow_of_2(pen_y + max_glyph_h + 2);
    
    rf->tex_width  = tex_width;
    rf->tex_height = tex_height;
    
    // upload texture
    glGenTextures(1, &rf->tex);
    glBindTexture(GL_TEXTURE_2D, rf->tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    
    if(use_lcd_filter){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_INT, pixels);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    end_temp_memory(temp);
    
    return 1;
}
