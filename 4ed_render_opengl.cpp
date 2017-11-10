/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * OpenGL render implementation
 *
 */

// TOP

#define ExtractStruct(s) ((s*)cursor); cursor += sizeof(s)

inline void
private_draw_set_clip(Render_Target *t, i32_Rect clip_box){
    glScissor(clip_box.x0, t->height - clip_box.y1, clip_box.x1 - clip_box.x0, clip_box.y1 - clip_box.y0);
}

inline void
private_draw_bind_texture(Render_Target *t, i32 texid){
    if (t->bound_texture != texid){
        glBindTexture(GL_TEXTURE_2D, texid);
        t->bound_texture = texid;
    }
}

inline void
private_draw_set_color(Render_Target *t, u32 color){
    if (t->color != color){
        t->color = color;
        Vec4 c = unpack_color4(color);
        glColor4f(c.r, c.g, c.b, c.a);
    }
}

inline void
private_draw_rectangle(Render_Target *t, f32_Rect rect, u32 color){
    private_draw_set_color(t, color);
    private_draw_bind_texture(t, 0);
    glBegin(GL_QUADS);
    {
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x0, rect.y1);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x1, rect.y0);
    }
    glEnd();
}

inline void
private_draw_rectangle_outline(Render_Target *t, f32_Rect rect, u32 color){
    f32_Rect r = get_inner_rect(rect, .5f);
    private_draw_set_color(t, color);
    private_draw_bind_texture(t, 0);
    glBegin(GL_LINE_STRIP);
    {
        glVertex2f(r.x0, r.y0);
        glVertex2f(r.x1, r.y0);
        glVertex2f(r.x1, r.y1);
        glVertex2f(r.x0, r.y1);
        glVertex2f(r.x0, r.y0);
    }
    glEnd();
}

struct Render_Quad{
    f32 x0, y0, x1, y1;
    f32 s0, t0, s1, t1;
};

inline Render_Quad
get_render_quad(Glyph_Bounds *b, i32 pw, i32 ph, float xpos, float ypos){
    Render_Quad q;
    
    float ipw = 1.0f / pw, iph = 1.0f / ph;
    
    q.x0 = xpos + b->xoff;
    q.y0 = ypos + b->yoff;
    q.x1 = xpos + b->xoff2;
    q.y1 = ypos + b->yoff2;
    
    q.s0 = b->x0 * ipw;
    q.t0 = b->y0 * iph;
    q.s1 = b->x1 * ipw;
    q.t1 = b->y1 * iph;
    
    return(q);
}

inline void
private_draw_glyph(System_Functions *system, Render_Target *t, Render_Font *font, u32 codepoint, f32 x, f32 y, u32 color){
    Glyph_Data glyph = font_get_glyph(system, font, codepoint);
    if (glyph.tex != 0){
        Render_Quad q = get_render_quad(&glyph.bounds, glyph.tex_width, glyph.tex_height, x, y);
        
        private_draw_set_color(t, color);
        private_draw_bind_texture(t, glyph.tex);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
        }
        glEnd();
    }
}

internal void CALL_CONVENTION
opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam){
    // TODO(allen): Fill this in with my preferred thingy.
}

internal void
interpret_render_buffer(System_Functions *system, Render_Target *t){
    local_persist b32 first_opengl_call = true;
    if (first_opengl_call){
        first_opengl_call = false;
        
        // TODO(allen): Get logging working everywhere
#if 0
        //TODO(inso): glGetStringi is required in core profile if the GL version is >= 3.0
        char *Vendor   = (char *)glGetString(GL_VENDOR);
        char *Renderer = (char *)glGetString(GL_RENDERER);
        char *Version  = (char *)glGetString(GL_VERSION);
        
        LOGF("GL_VENDOR: %s\n", Vendor);
        LOGF("GL_RENDERER: %s\n", Renderer);
        LOGF("GL_VERSION: %s\n", Version);
#endif
        
        // TODO(allen): Get this up and running for dev mode again.
#if (defined(BUILD_X64) && 0) || (defined(BUILD_X86) && 0)
        // NOTE(casey): This slows down GL but puts error messages to
        // the debug console immediately whenever you do something wrong
        glDebugMessageCallback_type *glDebugMessageCallback = 
            (glDebugMessageCallback_type *)win32_load_gl_always("glDebugMessageCallback", module);
        glDebugMessageControl_type *glDebugMessageControl = 
            (glDebugMessageControl_type *)win32_load_gl_always("glDebugMessageControl", module);
        if(glDebugMessageCallback != 0 && glDebugMessageControl != 0)
        {
            glDebugMessageCallback(opengl_debug_callback, 0);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
#endif
        
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    char *cursor = t->push_buffer;
    char *cursor_end = cursor + t->size;
    
    for (; cursor < cursor_end;){
        Render_Piece_Header *header = ExtractStruct(Render_Piece_Header);
        
        i32 type = header->type;
        switch (type){
            case piece_type_rectangle:
            {
                Render_Piece_Rectangle *rectangle = ExtractStruct(Render_Piece_Rectangle);
                private_draw_rectangle(t, rectangle->rect, rectangle->color);
            }break;
            
            case piece_type_outline:
            {
                Render_Piece_Rectangle *rectangle = ExtractStruct(Render_Piece_Rectangle);
                private_draw_rectangle_outline(t, rectangle->rect, rectangle->color);
            }break;
            
            case piece_type_glyph:
            {
                Render_Piece_Glyph *glyph = ExtractStruct(Render_Piece_Glyph);
                
                Render_Font *font = system->font.get_render_data_by_id(glyph->font_id);
                Assert(font != 0);
                private_draw_glyph(system, t, font, glyph->codepoint, glyph->pos.x, glyph->pos.y, glyph->color);
            }break;
            
            case piece_type_change_clip:
            {
                Render_Piece_Change_Clip *clip = ExtractStruct(Render_Piece_Change_Clip);
                private_draw_set_clip(t, clip->box);
            }break;
        }
    }
    
    glFlush();
}

#undef ExtractStruct

// NOTE(allen): Thanks to insofaras.  This is copy-pasted from some work he originally did to get free type working on Linux.

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

internal void
font_load_page_inner(Partition *part, Render_Font *font, FT_Library ft, FT_Face face, b32 use_hinting, Glyph_Page *page, u32 page_number, i32 tab_width){
    Temp_Memory temp = begin_temp_memory(part);
    Assert(page != 0);
    page->page_number = page_number;
    
    // prepare to read glyphs into a temporary texture buffer
    i32 max_glyph_w = face->size->metrics.x_ppem;
    
    i32 max_glyph_h = font_get_height(font);
    i32 tex_width   = 64;
    i32 tex_height  = 0;
    
    do {
        tex_width *= 2;
        float glyphs_per_row = ceilf(tex_width / (float) max_glyph_w);
        float rows = ceilf(ITEM_PER_FONT_PAGE / glyphs_per_row);
        tex_height = ceil32(rows * (max_glyph_h + 2));
    } while(tex_height > tex_width);
    
    tex_height = round_up_pot_u32(tex_height);
    
    i32 pen_x = 0;
    i32 pen_y = 0;
    
    u32* pixels = push_array(part, u32, tex_width * tex_height);
    memset(pixels, 0, tex_width * tex_height * sizeof(u32));
    
    u32 ft_flags = FT_LOAD_RENDER;
    if (use_hinting){
        // NOTE(inso): FT_LOAD_TARGET_LIGHT does hinting only vertically, which looks nicer imo
        // maybe it could be exposed as an option for hinting, instead of just on/off.
        ft_flags |= FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
    }
    else{
        ft_flags |= (FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_HINTING);
    }
    
    // fill the texture
    u32 base_codepoint = (page_number << 8);
    Glyph_Bounds *glyphs = &page->glyphs[0];
    Glyph_Bounds *glyph_ptr = glyphs;
    
    f32 *advances = &page->advance[0];
    f32 *advance_ptr = advances;
    for(u32 i = 0; i < ITEM_PER_FONT_PAGE; ++i, ++glyph_ptr, ++advance_ptr){
        u32 codepoint = i + base_codepoint;
        
        if(FT_Load_Char(face, codepoint, ft_flags) == 0){
            i32 w = face->glyph->bitmap.width;
            i32 h = face->glyph->bitmap.rows;
            
            i32 ascent = font_get_ascent(font);
            
            // move to next line if necessary
            if(pen_x + w >= tex_width){
                pen_x = 0;
                pen_y += (max_glyph_h + 2);
            }
            
            // set all this stuff the renderer needs
            glyph_ptr->x0 = (f32)(pen_x);
            glyph_ptr->y0 = (f32)(pen_y);
            glyph_ptr->x1 = (f32)(pen_x + w);
            glyph_ptr->y1 = (f32)(pen_y + h + 1);
            
            glyph_ptr->xoff = (f32)(face->glyph->bitmap_left);
            glyph_ptr->yoff = (f32)(ascent - face->glyph->bitmap_top);
            glyph_ptr->xoff2 = glyph_ptr->xoff + w;
            glyph_ptr->yoff2 = glyph_ptr->yoff + h + 1;
            
            // TODO(allen): maybe advance data should be integers?
            *advance_ptr = (f32)ceil32(face->glyph->advance.x / 64.0f);
            
            // write to texture atlas
            i32 pitch = face->glyph->bitmap.pitch;
            for(i32 Y = 0; Y < h; ++Y){
                for(i32 X = 0; X < w; ++X){
                    i32 x = pen_x + X;
                    i32 y = pen_y + Y;
                    
                    pixels[y * tex_width + x] = face->glyph->bitmap.buffer[Y * pitch + X] * 0x01010101;
                }
            }
            
            pen_x = ceil32(glyph_ptr->x1 + 1);
        }
    }
    
    // upload texture
    tex_height = round_up_pot_u32(pen_y + max_glyph_h + 2);
    
    page->tex_width  = tex_width;
    page->tex_height = tex_height;
    
    glGenTextures(1, &page->tex);
    glBindTexture(GL_TEXTURE_2D, page->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_INT, pixels);
    
    end_temp_memory(temp);
    
    // whitespace spacing stuff
    if (page_number == 0){
        f32 space_adv = advances[' '];
        f32 backslash_adv = advances['\\'];
        f32 r_adv = advances['r'];
        
        advances['\n'] = space_adv;
        advances['\r'] = backslash_adv + r_adv;
        advances['\t'] = space_adv*tab_width;
    }
}

internal b32
font_load_page(System_Functions *system, Partition *part, Render_Font *font, Glyph_Page *page, u32 page_number, u32 pt_size,  b32 use_hinting){
    
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
    
    // NOTE(allen): set texture and glyph data.
    font_load_page_inner(part, font, ft, face, use_hinting, page, page_number, 4);
    
    FT_Done_FreeType(ft);
    
    return(true);
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
    
    // set size & metrics
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
    
    // NOTE(allen): set texture and glyph data.
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

internal void
system_set_page(System_Functions *system, Partition *part, Render_Font *font, Glyph_Page *page, u32 page_number, u32 pt_size, b32 use_hinting){
    Assert(pt_size >= 8);
    
    memset(page, 0, sizeof(*page));
    
    if (part->base == 0){
        *part = sysshared_scratch_partition(MB(8));
    }
    
    b32 success = false;
    for (u32 R = 0; R < 3; ++R){
        success = font_load_page(system, part, font, page, page_number, pt_size, use_hinting);
        if (success){
            break;
        }
        else{
            sysshared_partition_double(part);
        }
    }
}

internal void
system_set_font(System_Functions *system, Partition *part, Render_Font *font, char *filename, u32 pt_size, b32 use_hinting){
    memset(font, 0, sizeof(*font));
    
    u32 filename_len = 0;
    for (;filename[filename_len];++filename_len);
    
    if (filename_len <= sizeof(font->filename)-1){
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
}

// BOTTOM

