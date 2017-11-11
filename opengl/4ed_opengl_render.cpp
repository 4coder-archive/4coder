/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * OpenGL render implementation
 *
 */

// TOP

// TODO(allen): If we don't actually need this then burn down 4ed_opengl_funcs.h
// Declare function types and function pointers
//#define GL_FUNC(N,R,P) typedef R (N##_Function) P; N##_Function *P = 0;
//#include "4ed_opengl_funcs.h"
#include "4ed_opengl_defines.h"

// OpenGL 2.1 implementation

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

internal void
interpret_render_buffer(System_Functions *system, Render_Target *t){
    local_persist b32 first_opengl_call = true;
    if (first_opengl_call){
        first_opengl_call = false;
        
        char *vendor   = (char *)glGetString(GL_VENDOR);
        char *renderer = (char *)glGetString(GL_RENDERER);
        char *version  = (char *)glGetString(GL_VERSION);
        
        LOGF("GL_VENDOR: %s\n", vendor);
        LOGF("GL_RENDERER: %s\n", renderer);
        LOGF("GL_VERSION: %s\n", version);
        
        // TODO(allen): Get this up and running for dev mode again.
#if (defined(BUILD_X64) && 0) || (defined(BUILD_X86) && 0)
        // NOTE(casey): This slows down GL but puts error messages to
        // the debug console immediately whenever you do something wrong
        
        void CALL_CONVENTION gl_dbg(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam);
        
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
    
    i32 width = t->width;
    i32 height = t->height;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glScissor(0, 0, width, height);
    glClearColor(1.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    t->bound_texture = 0;
    
    glColor4f(0.f, 0.f, 0.f, 0.f);
    t->color = 0;
    
    u8 *start = (u8*)t->buffer.base;
    u8 *end = (u8*)t->buffer.base + t->buffer.pos;
    Render_Command_Header *header = 0;
    for (u8 *p = start; p < end; p += header->size){
        header = (Render_Command_Header*)p;
        
        i32 type = header->type;
        switch (type){
            case RenCom_Rectangle:
            {
                Render_Command_Rectangle *rectangle = (Render_Command_Rectangle*)header;
                f32_Rect r = rectangle->rect;
                private_draw_set_color(t, rectangle->color);
                private_draw_bind_texture(t, 0);
                glBegin(GL_QUADS);
                {
                    glVertex2f(r.x0, r.y0);
                    glVertex2f(r.x0, r.y1);
                    glVertex2f(r.x1, r.y1);
                    glVertex2f(r.x1, r.y0);
                }
                glEnd();
            }break;
            
            case RenCom_Outline:
            {
                Render_Command_Rectangle *rectangle = (Render_Command_Rectangle*)header;
                f32_Rect r = get_inner_rect(rectangle->rect, .5f);
                private_draw_set_color(t, rectangle->color);
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
            }break;
            
            case RenCom_Glyph:
            {
                Render_Command_Glyph *glyph = (Render_Command_Glyph*)header;
                Render_Font *font = system->font.get_render_data_by_id(glyph->font_id);
                if (font == 0){
                    break;
                }
                
                Glyph_Data g = font_get_glyph(system, font, glyph->codepoint);
                if (g.tex == 0){
                    break;
                }
                
                f32 x = glyph->pos.x;
                f32 y = glyph->pos.y;
                
                f32_Rect xy = {0};
                xy.x0 = x + g.bounds.xoff;
                xy.y0 = y + g.bounds.yoff;
                xy.x1 = x + g.bounds.xoff2;
                xy.y1 = y + g.bounds.yoff2;
                
                // TODO(allen): Why aren't these baked in???
                f32 unit_u = 1.f/g.tex_width;
                f32 unit_v = 1.f/g.tex_height;
                
                f32_Rect uv = {0};
                uv.x0 = g.bounds.x0*unit_u;
                uv.y0 = g.bounds.y0*unit_v;
                uv.x1 = g.bounds.x1*unit_u;
                uv.y1 = g.bounds.y1*unit_v;
                
                private_draw_set_color(t, glyph->color);
                private_draw_bind_texture(t, g.tex);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(uv.x0, uv.y1); glVertex2f(xy.x0, xy.y1);
                    glTexCoord2f(uv.x1, uv.y1); glVertex2f(xy.x1, xy.y1);
                    glTexCoord2f(uv.x1, uv.y0); glVertex2f(xy.x1, xy.y0);
                    glTexCoord2f(uv.x0, uv.y0); glVertex2f(xy.x0, xy.y0);
                }
                glEnd();
            }break;
            
            case RenCom_ChangeClip:
            {
                Render_Command_Change_Clip *clip = (Render_Command_Change_Clip*)header;
                i32_Rect box = clip->box;
                glScissor(box.x0, height - box.y1, box.x1 - box.x0, box.y1 - box.y0);
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

