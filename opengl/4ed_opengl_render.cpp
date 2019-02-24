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

internal GLuint
private_texture_initialize(GLint tex_width, GLint tex_height, u32 *pixels){
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_INT, pixels);
    return(tex);
}

internal void
private_draw_bind_texture(Render_Target *t, i32 texid){
    if (t->bound_texture != texid){
        glBindTexture(GL_TEXTURE_2D, texid);
        t->bound_texture = texid;
    }
}

internal void
private_draw_set_color(Render_Target *t, u32 color){
    if (t->color != color){
        t->color = color;
        Vec4 c = unpack_color4(color);
        glColor4f(c.r, c.g, c.b, c.a);
    }
}

internal void
interpret_render_buffer(Render_Target *t, Partition *growable_scratch){
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
    {
        Vec4 color = unpack_color4(t->clear_color);
        glClearColor(color.r, color.g, color.b, color.a);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    t->bound_texture = 0;
    
    glColor4f(0.f, 0.f, 0.f, 0.f);
    t->color = 0;
    
    for (Render_Free_Texture *free_texture = t->free_texture_first;
         free_texture != 0;
         free_texture = free_texture->next){
        glDeleteTextures(1, &free_texture->tex_id);
    }
    sll_clear(t->free_texture_first, t->free_texture_last);
    
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
                Font_Pointers font = system_font_get_pointers_by_id(glyph->font_id);
                if (!font.valid){
                    break;
                }
                
                u32 codepoint = glyph->codepoint;
                u32 page_number = codepoint/GLYPHS_PER_PAGE;
                Glyph_Page *page = font_cached_get_page(font.pages, page_number);
                if (page == 0){
                    break;
                }
                
                if (!page->has_gpu_setup){
                    Temp_Memory temp = begin_temp_memory(growable_scratch);
                    i32 tex_width = 0;
                    i32 tex_height = 0;
                    u32 *pixels = font_load_page_pixels(growable_scratch, font.settings, page, page_number, &tex_width, &tex_height);
                    page->has_gpu_setup = true;
                    page->gpu_tex = private_texture_initialize(tex_width, tex_height, pixels);
                    end_temp_memory(temp);
                }
                
                if (page->gpu_tex == 0){
                    break;
                }
                
                u32 glyph_index = codepoint%GLYPHS_PER_PAGE;
                Glyph_Bounds bounds = page->glyphs[glyph_index];
                GLuint tex = page->gpu_tex;
                i32 tex_width = page->tex_width;
                i32 tex_height = page->tex_height;
                
                f32 x = glyph->pos.x;
                f32 y = glyph->pos.y;
                
                f32_Rect uv = {};
                
                // TODO(allen): do(think about baking unit_u/unit_v into font data)
                f32 unit_u = 1.f/tex_width;
                f32 unit_v = 1.f/tex_height;
                
                uv.x0 = bounds.x0*unit_u;
                uv.y0 = bounds.y0*unit_v;
                uv.x1 = bounds.x1*unit_u;
                uv.y1 = bounds.y1*unit_v;
                
                private_draw_set_color(t, glyph->color);
                private_draw_bind_texture(t, tex);
                glBegin(GL_QUADS);
                if ((glyph->flags & GlyphFlag_Rotate90) == 0){
                    glTexCoord2f(uv.x0, uv.y1); glVertex2f(x + bounds.xoff , y + bounds.yoff2);
                    glTexCoord2f(uv.x1, uv.y1); glVertex2f(x + bounds.xoff2, y + bounds.yoff2);
                    glTexCoord2f(uv.x1, uv.y0); glVertex2f(x + bounds.xoff2, y + bounds.yoff );
                    glTexCoord2f(uv.x0, uv.y0); glVertex2f(x + bounds.xoff , y + bounds.yoff );
                }
                else{
                    glTexCoord2f(uv.x0, uv.y1); glVertex2f(x + bounds.yoff2, y + bounds.xoff2);
                    glTexCoord2f(uv.x1, uv.y1); glVertex2f(x + bounds.yoff2, y + bounds.xoff );
                    glTexCoord2f(uv.x1, uv.y0); glVertex2f(x + bounds.yoff , y + bounds.xoff );
                    glTexCoord2f(uv.x0, uv.y0); glVertex2f(x + bounds.yoff , y + bounds.xoff2);
                }
                glEnd();
                
                if (codepoint != ' ' && font.settings->parameters.underline){
                    glDisable(GL_TEXTURE_2D);
                    
                    f32 x0 = x;
                    f32 x1 = x + page->advance[glyph_index];
                    f32 yoff1 = y + font.metrics->underline_yoff1;
                    f32 yoff2 = y + font.metrics->underline_yoff2;
                    
                    glBegin(GL_QUADS);
                    {
                        glVertex2f(x0, yoff1);
                        glVertex2f(x1, yoff1);
                        glVertex2f(x1, yoff2);
                        glVertex2f(x0, yoff2);
                    }
                    glEnd();
                    
                    glEnable(GL_TEXTURE_2D);
                }
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

// BOTTOM

