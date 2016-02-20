/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.17.2014
 *
 * Rendering layer for project codename "4ed"
 *
 */

// TOP

inline void
draw_set_clip(Render_Target *target, i32_Rect clip_box){
    glScissor(clip_box.x0,
              target->height - clip_box.y1,
              clip_box.x1 - clip_box.x0,
              clip_box.y1 - clip_box.y0);
}

inline void
draw_bind_texture(Render_Target *target, i32 texid){
    if (target->bound_texture != texid){
        glBindTexture(GL_TEXTURE_2D, texid);
        target->bound_texture = texid;
    }
}

inline void
draw_set_color(Render_Target *target, u32 color){
    if (target->color != color){
        target->color = color;
        Vec4 c = unpack_color4(color);
        glColor4f(c.r, c.g, c.b, c.a);
    }
}

#define PutStruct(s,x) *(s*)(target->push_buffer + target->size) = x; target->size += sizeof(s)

internal void
draw_push_piece(Render_Target *target, Render_Piece_Combined piece){
    PutStruct(Render_Piece_Header, piece.header);
    
    switch (piece.header.type){
    case piece_type_rectangle:
    case piece_type_outline:
        PutStruct(Render_Piece_Rectangle, piece.rectangle);
        break;
        
    case piece_type_gradient:
        PutStruct(Render_Piece_Gradient, piece.gradient);
        break;
        
    case piece_type_glyph:
    case piece_type_mono_glyph:
        PutStruct(Render_Piece_Glyph, piece.glyph);
        break;
        
    case piece_type_mono_glyph_advance:
        PutStruct(Render_Piece_Glyph_Advance, piece.glyph_advance);
        break;
    }
    
    Assert(target->size <= target->max);
}

internal void
draw_push_piece_clip(Render_Target *target, i32_Rect clip_box){
    // TODO(allen): optimize out if there are two clip box changes in a row
    Render_Piece_Change_Clip clip;
    Render_Piece_Header header;
    
    header.type = piece_type_change_clip;
    clip.box = clip_box;
    
    PutStruct(Render_Piece_Header, header);
    PutStruct(Render_Piece_Change_Clip, clip);
}

internal void
draw_push_clip(Render_Target *target, i32_Rect clip_box){
    Assert(target->clip_top == -1 ||
           fits_inside(clip_box, target->clip_boxes[target->clip_top]));
    Assert(target->clip_top+1 < ArrayCount(target->clip_boxes));
    target->clip_boxes[++target->clip_top] = clip_box;

    draw_push_piece_clip(target, clip_box);
}

internal void
draw_pop_clip(Render_Target *target){
    i32_Rect clip_box;
    Assert(target->clip_top > 0);
    --target->clip_top;
    clip_box = target->clip_boxes[target->clip_top];
    
    draw_push_piece_clip(target, clip_box);
}

#define ExtractStruct(s) ((s*)cursor); cursor += sizeof(s)

inline void
private_draw_rectangle(Render_Target *target, f32_Rect rect, u32 color){
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

inline void
private_draw_rectangle_outline(Render_Target *target, f32_Rect rect, u32 color){
    f32_Rect r;
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
private_draw_gradient(Render_Target *target, f32_Rect rect,
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
private_draw_glyph(Render_Target *target, Render_Font *font,
                   u8 character, f32 x, f32 y, u32 color){
    f32 x_shift, y_shift;
    x_shift = 0;
    y_shift = (f32)font->ascent;
    
    x += x_shift;
    y += y_shift;
    
    stbtt_aligned_quad q;
    stbtt_GetPackedQuad(font->chardata, font->tex_width, font->tex_height,
                        character, &x, &y, &q, 0);
    
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
private_draw_glyph_mono(Render_Target *target, Render_Font *font, u8 character,
                        f32 x, f32 y, f32 advance, u32 color){
    f32 x_shift, y_shift;
    i32 left = font->chardata[character].x0;
    i32 right = font->chardata[character].x1;
    i32 width = (right - left);
    x_shift = (f32)(advance - width) * .5f - font->chardata[character].xoff;
    y_shift = (f32)font->ascent;
    
    x += x_shift;
    y += y_shift;
    
    stbtt_aligned_quad q;
    stbtt_GetPackedQuad(font->chardata, font->tex_width, font->tex_height,
                        character, &x, &y, &q, 0);
    
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
private_draw_glyph_mono(Render_Target *target, Render_Font *font, u8 character,
                        real32 x, real32 y, u32 color){
    private_draw_glyph_mono(target, font, character, x, y, (f32)font->advance, color);
}

internal void
launch_rendering(Render_Target *target){
    byte *cursor = target->push_buffer;
    byte *cursor_end = cursor + target->size;

    for (; cursor < cursor_end;){
        Render_Piece_Header *header = ExtractStruct(Render_Piece_Header);
        
        i32 type = header->type;
        switch (type){
        case piece_type_rectangle:
        {
            Render_Piece_Rectangle *rectangle =
                ExtractStruct(Render_Piece_Rectangle);
            private_draw_rectangle(target, rectangle->rect, rectangle->color);
        }break;
        
        case piece_type_outline:
        {
            Render_Piece_Rectangle *rectangle =
                ExtractStruct(Render_Piece_Rectangle);
            private_draw_rectangle_outline(target, rectangle->rect, rectangle->color);
        }break;
        
        case piece_type_gradient:
        {
            Render_Piece_Gradient *gradient =
                ExtractStruct(Render_Piece_Gradient);
            private_draw_gradient(target, gradient->rect,
                                  unpack_color4(gradient->left_color),
                                  unpack_color4(gradient->right_color));
        }break;
        
        case piece_type_glyph:
        {
            Render_Piece_Glyph *glyph =
                ExtractStruct(Render_Piece_Glyph);
            
            Render_Font *font = get_font_info(&target->font_set, glyph->font_id)->font;
            if (font)
                private_draw_glyph(target, font, glyph->character,
                                   glyph->pos.x, glyph->pos.y, glyph->color);
        }break;
        
        case piece_type_mono_glyph:
        {
            Render_Piece_Glyph *glyph =
                ExtractStruct(Render_Piece_Glyph);
            
            Render_Font *font = get_font_info(&target->font_set, glyph->font_id)->font;
            if (font)            
                private_draw_glyph_mono(target, font, glyph->character,
                                        glyph->pos.x, glyph->pos.y, glyph->color);
        }break;
        
        case piece_type_mono_glyph_advance:
        {
            Render_Piece_Glyph_Advance *glyph =
                ExtractStruct(Render_Piece_Glyph_Advance);
            
            Render_Font *font = get_font_info(&target->font_set, glyph->font_id)->font;
            if (font)
                private_draw_glyph_mono(target, font, glyph->character,
                                        glyph->pos.x, glyph->pos.y,
                                        glyph->advance, glyph->color);
        }break;

        case piece_type_change_clip:
        {
            Render_Piece_Change_Clip *clip =
                ExtractStruct(Render_Piece_Change_Clip);
            draw_set_clip(target, clip->box);
        }break;
        }
    }
}

#undef ExtractStruct

internal i32
draw_font_info_load(Partition *partition,
    char *filename_untranslated,
    i32 pt_size, i32 *height, i32 *advance){
    
    char space_[1024];
    String filename = make_fixed_width_string(space_);
    b32 translate_success = system_to_binary_path(&filename, filename_untranslated);
    if (!translate_success) return 0;
    
    i32 result = 1;
    Data file;
    file = system_load_file(filename.str);
    
    Temp_Memory temp = begin_temp_memory(partition);
    stbtt_packedchar *chardata = push_array(partition, stbtt_packedchar, 256);

    i32 oversample = 2;
    
    i32 tex_width, tex_height;
    tex_width = pt_size*128*oversample;
    tex_height = pt_size*2*oversample;
    void *block = push_block(partition, tex_width * tex_height);
    
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
            f32 scale;
            
            stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
            scale = stbtt_ScaleForPixelHeight(&font, (f32)pt_size);
            
            f32 scaled_ascent, scaled_descent, scaled_line_gap;
            
            scaled_ascent = scale*ascent;
            scaled_descent = scale*descent;
            scaled_line_gap = scale*line_gap;

            i32 font_height = (i32)(scaled_ascent - scaled_descent + scaled_line_gap);

            stbtt_pack_context spc;
            if (stbtt_PackBegin(&spc, (u8*)block, tex_width, tex_height, tex_width, 1, partition)){
                stbtt_PackSetOversampling(&spc, oversample, oversample);
                if (stbtt_PackFontRange(&spc, (u8*)file.data, 0,
                                        STBTT_POINT_SIZE((f32)pt_size), 0, 128, chardata)){
                    // do nothing
                }
                else{
                    result = 0;
                }
                
                stbtt_PackEnd(&spc);
            }
            else{
                result = 0;
            }
            
            if (result){
                i32 max_advance = 0;
                for (u8 code_point = 0; code_point < 128; ++code_point){
                    if (stbtt_FindGlyphIndex(&font, code_point) != 0){
                        i32 adv = CEIL32(chardata[code_point].xadvance);
                        if (max_advance < adv){
                            max_advance = adv;
                        }
                    }
                }
                
                *height = font_height;
                *advance = max_advance - 1;
            }
        }
        
        system_free_memory(file.data);
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal i32
draw_font_load(void *base_block, i32 size,
    Render_Font *font_out,
    char *filename_untranslated,
    i32 pt_size,
    i32 tab_width){

    char space_[1024];
    String filename = make_fixed_width_string(space_);
    b32 translate_success = system_to_binary_path(&filename, filename_untranslated);
    if (!translate_success) return 0;
        
    i32 result = 1;
    Data file;
    file = system_load_file(filename.str);
    
    Partition partition_ = partition_open(base_block, size);
    Partition *partition = &partition_;
    
    stbtt_packedchar *chardata = font_out->chardata;

    i32 oversample = 2;
    
    i32 tex_width, tex_height;
    tex_width = pt_size*128*oversample;
    tex_height = pt_size*2*oversample;
    void *block = push_block(partition, tex_width * tex_height);
    
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
            f32 scale;
            
            stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
            scale = stbtt_ScaleForPixelHeight(&font, (f32)pt_size);
            
            f32 scaled_ascent, scaled_descent, scaled_line_gap;
            
            scaled_ascent = scale*ascent;
            scaled_descent = scale*descent;
            scaled_line_gap = scale*line_gap;
            
            font_out->height = (i32)(scaled_ascent - scaled_descent + scaled_line_gap);
            font_out->ascent = (i32)(scaled_ascent);
            font_out->descent = (i32)(scaled_descent);
            font_out->line_skip = (i32)(scaled_line_gap);
            
            font_out->tex_width = tex_width;
            font_out->tex_height = tex_height;

            stbtt_pack_context spc;

            if (stbtt_PackBegin(&spc, (u8*)block, tex_width, tex_height, tex_width, 1, partition)){
                stbtt_PackSetOversampling(&spc, oversample, oversample);
                if (stbtt_PackFontRange(&spc, (u8*)file.data, 0,
                                        STBTT_POINT_SIZE((f32)pt_size), 0, 128, chardata)){
                    // do nothing
                }
                else{
                    result = 0;
                }

                stbtt_PackEnd(&spc);
            }
            else{
                result = 0;
            }

            if (result){
                GLuint font_tex;
                glGenTextures(1, &font_tex);
                glBindTexture(GL_TEXTURE_2D, font_tex);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, block);
                
                font_out->tex = font_tex;
                glBindTexture(GL_TEXTURE_2D, 0);
                
                font_out->chardata['\r'] = font_out->chardata[' '];
                font_out->chardata['\n'] = font_out->chardata[' '];
                font_out->chardata['\t'] = font_out->chardata[' '];
                font_out->chardata['\t'].xadvance *= tab_width;
            
                i32 max_advance = 0;
                for (u8 code_point = 0; code_point < 128; ++code_point){
                    if (stbtt_FindGlyphIndex(&font, code_point) != 0){
                        font_out->glyphs[code_point].exists = 1;
                        i32 advance = CEIL32(font_out->chardata[code_point].xadvance);
                        if (max_advance < advance) max_advance = advance;
                        font_out->advance_data[code_point] = font_out->chardata[code_point].xadvance;
                    }
                    else if (code_point == '\r' || code_point == '\n' || code_point == '\t'){
                        font_out->advance_data[code_point] = font_out->chardata[code_point].xadvance;
                    }
                }
                font_out->advance = max_advance - 1;
            }
            
        }
        system_free_memory(file.data);
    }
    
    return result;
}

internal
Release_Font_Sig(draw_release_font){
    glDeleteTextures(1, &font->tex);
    font->tex = 0;
}

// BOTTOM

