#undef internal
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#define internal static

//TODO(inso): put in linuxvars
static FcConfig* fc;

internal char*
linux_get_sys_font(char* name, i32 pt_size){
    char* result = 0;

    if(!fc){
        fc = FcInitLoadConfigAndFonts();
    }

    FcPattern* pat = FcPatternBuild(
        NULL,
        FC_POSTSCRIPT_NAME, FcTypeString, name,
        FC_SIZE,            FcTypeDouble, (double)pt_size,
        FC_FONTFORMAT,      FcTypeString, "TrueType",
        NULL
    );

    FcConfigSubstitute(fc, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult res;
    FcPattern* font = FcFontMatch(fc, pat, &res);
    FcChar8* fname = 0;

    if(font){
        FcPatternGetString(font, FC_FILE, 0, &fname);
        if(fname){
            result = strdup((char*)fname);
            fprintf(stderr, "Got system font from FontConfig: %s\n", result);
        }
        FcPatternDestroy(font);
    }

    FcPatternDestroy(pat);

    if(!result){
        char space[1024];
        String str = make_fixed_width_string(space);
        if(sysshared_to_binary_path(&str, name)){
            result =  strdup(space);
        } else {
            result = strdup(name);
        }
    }

    return result;
}

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
linux_font_load(Render_Font *rf, char *name, i32 pt_size, i32 tab_width){

#if 0
    char* filename = linux_get_sys_font(name, pt_size);
#else
    char* filename = (char*)malloc(256);
    String str = make_string(filename, 0, 256);
    sysshared_to_binary_path(&str, name);
#endif

    memset(rf, 0, sizeof(*rf));

    //TODO(inso): put stuff in linuxvars / init in main
    FT_Library ft;
    FT_Face face;
    b32 use_lcd_filter = 0;

    FT_Init_FreeType(&ft);

    //NOTE(inso): i'm not sure the LCD filter looks better, and it doesn't work perfectly with the coloring stuff
    // it will probably need shaders to work properly
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
    rf->ascent    = face->size->metrics.ascender    / 64.0f;
    rf->descent   = face->size->metrics.descender   / 64.0f;
    rf->advance   = face->size->metrics.max_advance / 64.0f;
    rf->height    = face->size->metrics.height      / 64.0f;
    rf->line_skip = rf->height - (rf->ascent - rf->descent);

    int max_glyph_w = face->size->metrics.x_ppem;
    int max_glyph_h = rf->height;
    int tex_width   = 64;
    int tex_height  = 0;

    // estimate upper bound on texture width
    do {
        tex_width *= 2;
        float glyphs_per_row = ceilf(tex_width / (float) max_glyph_w);
        float rows = ceilf(NUM_GLYPHS / glyphs_per_row);
        tex_height = rows * (max_glyph_h + 2);
    } while(tex_height > tex_width);

    tex_height = next_pow_of_2(tex_height);

    int pen_x = 0;
    int pen_y = 0;

    u32* pixels = (u32*) calloc(tex_width * tex_height, sizeof(u32));

    // XXX: test if AUTOHINT looks better or not
    const u32 ft_extra_flags = use_lcd_filter ? FT_LOAD_TARGET_LCD : 0; // FT_LOAD_FORCE_AUTOHINT;

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
        stbtt_packedchar* c = rf->chardata + i;

        c->x0 = pen_x;
        c->y0 = pen_y;
        c->x1 = pen_x + w;
        c->y1 = pen_y + h + 1;

        c->xoff = face->glyph->bitmap_left;
        c->yoff = -face->glyph->bitmap_top;

        c->xoff2 = w + c->xoff;
        c->yoff2 = h + c->yoff + 1;

        c->xadvance = face->glyph->advance.x >> 6;

        rf->advance_data[i] = c->xadvance;
        rf->glyphs[i].exists = 1;


        int pitch = face->glyph->bitmap.pitch;

        // write to texture atlas
        for(int j = 0; j < h; ++j){
            for(int i = 0; i < w; ++i){
                int x = pen_x + i;
                int y = pen_y + j;

                if(use_lcd_filter){
                    u8 a = face->glyph->bitmap.buffer[j * pitch + i * 3 + 1];
                    u8 r = face->glyph->bitmap.buffer[j * pitch + i * 3 + 0];
                    u8 b = face->glyph->bitmap.buffer[j * pitch + i * 3 + 2];

                    pixels[y * tex_width + x] = (a << 24) | (b << 16) | (a << 8) | r;
                } else {
                    pixels[y * tex_width + x] = face->glyph->bitmap.buffer[j * pitch + i] * 0x1010101;
                }
            }
        }

        pen_x = c->x1 + 1;
    }

    rf->chardata['\r'] = rf->chardata[' '];
    rf->chardata['\n'] = rf->chardata[' '];
    rf->chardata['\t'] = rf->chardata[' '];
    rf->chardata['\t'].xadvance *= tab_width;
    rf->advance_data['\t'] = rf->advance_data[' '] * tab_width;

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

    free(pixels);
    free(filename);

    return 1;
}
