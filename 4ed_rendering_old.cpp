/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.17.2014
 *
 * Rendering layer for project codename "4ed"
 *
 */

// TOP

#if 0
inline void
draw_set_clip(Render_Target *target, i32_Rect clip_box){
    glScissor(clip_box.x0,
              target->height - clip_box.y1,
              clip_box.x1 - clip_box.x0,
              clip_box.y1 - clip_box.y0);
}

inline void
draw_push_clip(Render_Target *target, i32_Rect clip_box){
    Assert(target->clip_top == -1 ||
           fits_inside(clip_box, target->clip_boxes[target->clip_top]));
    Assert(target->clip_top+1 < ArrayCount(target->clip_boxes));
    target->clip_boxes[++target->clip_top] = clip_box;
    draw_set_clip(target, clip_box);
}

inline void
draw_pop_clip(Render_Target *target){
    Assert(target->clip_top > 0);
    --target->clip_top;
    draw_set_clip(target, target->clip_boxes[target->clip_top]);
}

inline void
draw_bind_texture(Render_Target *target, i32 texid){
    if (target->bound_texture != texid){
        glBindTexture(GL_TEXTURE_2D, texid);
        target->bound_texture = texid;
    }
}

internal void
draw_set_color(Render_Target *target, u32 color){
    if (target->color != color){
        target->color = color;
        Vec4 c = unpack_color4(color);
        glColor4f(c.r, c.g, c.b, c.a);
    }
}

internal void
draw_rectangle(Render_Target *target, i32_Rect rect, u32 color){
    draw_set_color(target, color);
    draw_bind_texture(target, 0);
	glBegin(GL_QUADS);
    {
        glVertex2i(rect.x0, rect.y0);
        glVertex2i(rect.x0, rect.y1);
        glVertex2i(rect.x1, rect.y1);
        glVertex2i(rect.x1, rect.y0);
    }
	glEnd();
}

internal void
draw_rectangle(Render_Target *target, real32_Rect rect, u32 color){
    draw_set_color(target, color);
    draw_bind_texture(target, 0);
	glBegin(GL_QUADS);
    {
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x0, rect.y1);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x1, rect.y0);
    }
	glEnd();
}

internal void
draw_triangle_3corner(Render_Target *target,
                      real32 x0, real32 y0,
                      real32 x1, real32 y1,
                      real32 x2, real32 y2,
                      u32 color){
    draw_set_color(target, color);
    draw_bind_texture(target, 0);
	glBegin(GL_TRIANGLES);
    {
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
	glEnd();
}

internal void
draw_gradient_2corner_clipped(Render_Target *target, real32_Rect rect,
                              Vec4 color_left, Vec4 color_right){
    Vec4 cl = color_left;
    Vec4 cr = color_right;
    
    draw_bind_texture(target, 0);
	glBegin(GL_QUADS);
    {
        glColor4f(cl.r, cl.g, cl.b, cl.a);
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x0, rect.y1);
        
        glColor4f(cr.r, cr.g, cr.b, cr.a);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x1, rect.y0);
    }
	glEnd();
}

inline void
draw_gradient_2corner_clipped(Render_Target *target, real32 l, real32 t, real32 r, real32 b,
                              Vec4 color_left, Vec4 color_right){
    draw_gradient_2corner_clipped(target, f32R(l,t,r,b), color_left, color_right);
}

internal void
draw_rectangle_outline(Render_Target *target, real32_Rect rect, u32 color){
    real32_Rect r;
    r.x0 = rect.x0 + .5f;
    r.y0 = rect.y0 + .5f;
    r.x1 = rect.x1 - .5f;
    r.y1 = rect.y1 - .5f;
    
    draw_set_color(target, color);
    draw_bind_texture(target, 0);
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

inline void
draw_rectangle_outline(Render_Target *target, i32_Rect rect, u32 color){
    draw_rectangle_outline(target, f32R(rect), color);
}

internal void
draw_margin(Render_Target *target, i32_Rect outer, i32_Rect inner, u32 color){
    draw_rectangle(target, i32R(outer.x0, outer.y0, outer.x1, inner.y0), color);
    draw_rectangle(target, i32R(outer.x0, inner.y1, outer.x1, outer.y1), color);
    draw_rectangle(target, i32R(outer.x0, inner.y0, inner.x0, inner.y1), color);
    draw_rectangle(target, i32R(inner.x1, inner.y0, outer.x1, inner.y1), color);
}

// TODO(allen): eliminate this?
internal i32
font_init(){
    return 1;
}

inline internal i32
font_predict_size(i32 pt_size){
	return pt_size*pt_size*128;
}

internal i32
font_load(System_Functions *system,
          Render_Font *font_out, char *filename, i32 pt_size,
          void *font_block, i32 font_block_size,
          i32 *memory_used_out, i32 tab_width){
    i32 result = 1;
    File_Data file;
    file = system->load_file(filename);
    
    if (!file.data){
        result = 0;
    }
    
    else{
        stbtt_fontinfo font;
        if (!stbtt_InitFont(&font, (u8*)file.data, 0)){
            result = 0;
        }
        else{
            i32 ascent, descent, line_gap;
            real32 scale;
            
            stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
            scale = stbtt_ScaleForPixelHeight(&font, (real32)pt_size);
            
            real32 scaled_ascent, scaled_descent, scaled_line_gap;
            
            scaled_ascent = scale*ascent;
            scaled_descent = scale*descent;
            scaled_line_gap = scale*line_gap;
            
            font_out->height = (i32)(scaled_ascent - scaled_descent + scaled_line_gap);
            font_out->ascent = (i32)(scaled_ascent);
            font_out->descent = (i32)(scaled_descent);
            font_out->line_skip = (i32)(scaled_line_gap);
            
            u8 *memory_cursor = (u8*)font_block;
            Assert(pt_size*pt_size*128 <= font_block_size);
            
            i32 tex_width, tex_height;
            tex_width = pt_size*128;
            tex_height = pt_size*2;
            
            font_out->tex_width = tex_width;
            font_out->tex_height = tex_height;
            
            if (stbtt_BakeFontBitmap((u8*)file.data, 0, (real32)pt_size,
                                     memory_cursor, tex_width, tex_height, 0, 128, font_out->chardata) <= 0){
                result = 0;
            }
            
            else{
                GLuint font_tex;
                glGenTextures(1, &font_tex);
                glBindTexture(GL_TEXTURE_2D, font_tex);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, memory_cursor);
                
                font_out->tex = font_tex;
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            
            font_out->chardata['\r'] = font_out->chardata[' '];
            font_out->chardata['\n'] = font_out->chardata[' '];
            font_out->chardata['\t'] = font_out->chardata[' '];
            font_out->chardata['\t'].xadvance *= tab_width;
            
            i32 max_advance = 0;
            for (u16 code_point = 0; code_point < 128; ++code_point){
                if (stbtt_FindGlyphIndex(&font, code_point) != 0){
                    font_out->glyphs[code_point].exists = 1;
                    i32 advance = CEIL32(font_out->chardata[code_point].xadvance);
                    if (max_advance < advance) max_advance = advance;
                    font_out->advance_data[code_point] = font_out->chardata[code_point].xadvance;
                }
            }
            font_out->advance = max_advance - 1;
        }
        system->free_file(file);
    }
    
    return result;
}

internal void
font_set_tabwidth(Render_Font *font, i32 tab_width){
    font->chardata['\t'].xadvance *= font->chardata[' '].xadvance * tab_width;
}

internal void
font_draw_glyph_mono(Render_Target *target, Render_Font *font, u16 character,
                     real32 x, real32 y, real32 advance, u32 color){
    real32 x_shift, y_shift;
    i32 left = font->chardata[character].x0;
    i32 right = font->chardata[character].x1;
    i32 width = (right - left);
    x_shift = (real32)(advance - width) * .5f - font->chardata[character].xoff;
    y_shift = (real32)font->ascent;
    
    x += x_shift;
    y += y_shift;
    
    stbtt_aligned_quad q;
    stbtt_GetBakedQuadUnrounded(font->chardata, font->tex_width, font->tex_height, character, &x, &y, &q, 1);
    
    draw_set_color(target, color);
    draw_bind_texture(target, font->tex);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
        glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
        glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
        glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
    }
    glEnd();
}

inline void
font_draw_glyph_mono(Render_Target *target, Render_Font *font, u16 character,
                     real32 x, real32 y, u32 color){
    font_draw_glyph_mono(target, font, character, x, y, (real32)font->advance, color);
}

internal void
font_draw_glyph(Render_Target *target, Render_Font *font, u16 character,
                real32 x, real32 y, u32 color){
    real32 x_shift, y_shift;
    x_shift = 0;
    y_shift = (real32)font->ascent;
    
    x += x_shift;
    y += y_shift;
    
    stbtt_aligned_quad q;
    stbtt_GetBakedQuadUnrounded(font->chardata, font->tex_width, font->tex_height, character, &x, &y, &q, 1);
    
    draw_set_color(target, color);
    draw_bind_texture(target, font->tex);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
        glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
        glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
        glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
    }
    glEnd();
}

inline real32
font_get_glyph_width(Render_Font *font, u16 character){
    return font->chardata[character].xadvance;
}

internal real32
font_string_width(Render_Font *font, char *str){
    real32 x = 0;
    for (i32 i = 0; str[i]; ++i){
        x += font_get_glyph_width(font, str[i]);
    }
    return x;
}

internal i32
draw_string(Render_Target *target, Render_Font *font, char *str,
            i32 x_, i32 y, u32 color){
    real32 x = (real32)x_;
    for (i32 i = 0; str[i]; ++i){
        char c = str[i];
        font_draw_glyph(target, font, c,
                        x, (real32)y, color);
        x += font_get_glyph_width(font, c);
    }
    return CEIL32(x);
}

internal real32
draw_string_mono(Render_Target *target, Render_Font *font, char *str,
                 real32 x, real32 y, real32 advance, u32 color){
    for (i32 i = 0; str[i]; ++i){
        font_draw_glyph_mono(target, font, str[i],
                             x, y, advance, color);
        x += advance;
    }
    return x;
}

internal i32
draw_string(Render_Target *target, Render_Font *font, String str,
            i32 x_, i32 y, u32 color){
    real32 x = (real32)x_;
    for (i32 i = 0; i < str.size; ++i){
        char c = str.str[i];
        font_draw_glyph(target, font, c,
                        x, (real32)y, color);
        x += font_get_glyph_width(font, c);
    }
    return CEIL32(x);
}

internal real32
draw_string_mono(Render_Target *target, Render_Font *font, String str,
                 real32 x, real32 y, real32 advance, u32 color){
    for (i32 i = 0; i < str.size; ++i){
        font_draw_glyph_mono(target, font, str.str[i],
                             x, y, advance, color);
        x += advance;
    }
    return x;
}

internal real32
font_get_max_width(Render_Font *font, char *characters){
    stbtt_bakedchar *chardata = font->chardata;
    real32 cx, x = 0;
    for (i32 i = 0; characters[i]; ++i){
        cx = chardata[characters[i]].xadvance;
        if (x < cx) x = cx;
    }
    return x;
}

internal real32
font_get_string_width(Render_Font *font, String string){
    real32 result = 0;
    for (i32 i = 0; i < string.size; ++i){
        font_get_glyph_width(font, string.str[i]);
    }
    return result;
}
#endif

// BOTTOM
