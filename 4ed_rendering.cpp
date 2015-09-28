 /*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.17.2014
 *
 * Rendering layer for project codename "4ed"
 *
 */

// TOP

internal i32_Rect
rect_clamp_to_rect(i32_Rect rect, i32_Rect clamp_box){
	if (rect.x0 < clamp_box.x0) rect.x0 = clamp_box.x0;
	if (rect.y0 < clamp_box.y0) rect.y0 = clamp_box.y0;
	if (rect.x1 > clamp_box.x1) rect.x1 = clamp_box.x1;
	if (rect.y1 > clamp_box.y1) rect.y1 = clamp_box.y1;
    
	return rect;
}

inline i32_Rect
rect_clamp_to_rect(i32 left, i32 top, i32 right, i32 bottom, i32_Rect clamp_box){
	return rect_clamp_to_rect(i32R(left, top, right, bottom), clamp_box);
}

inline i32_Rect
rect_from_target(Render_Target *target){
	return i32R(0, 0, target->width, target->height);
}

#if SOFTWARE_RENDER
internal void
draw_clear(Render_Target *target, u32 color){
	u8 *pixel_line = (u8*)target->pixel_data;
	for (i32 pixel_y = 0; pixel_y < target->height; ++pixel_y){
		u32 *pixel = (u32*)pixel_line;
		for (i32 pixel_x = 0; pixel_x < target->width; ++pixel_x){
			*pixel++ = color;
		}
		pixel_line += target->pitch;
	}
}

internal void
draw_vertical_line(Render_Target *target,
				   i32 x, i32 top, i32 bottom,
				   u32 color){
	if (x >= 0 && x < target->width){
		if (top < 0){
			top = 0;
		}
		if (bottom >= target->height){
			bottom = target->height - 1;
		}
        
		if (top <= bottom){
			i32 y_range = bottom - top;
			u8 *pixel_line = (u8*)target->pixel_data + top*target->pitch;
			for (i32 pixel_y = 0; pixel_y <= y_range; ++pixel_y){
				u32 *pixel = (u32*)pixel_line + x;
				*pixel = color;
				pixel_line += target->pitch;
			}
		}
	}
}

internal void
draw_horizontal_line(Render_Target *target,
					 i32 y, i32 left, i32 right,
					 u32 color){
	if (y >= 0 && y < target->height){
		if (left < 0){
			left = 0;
		}
		if (right >= target->width){
			right = target->width - 1;
		}
		
		if (left <= right){
			i32 x_range = right - left;
			u8 *pixel_line = (u8*)target->pixel_data + y*target->pitch;
			u32 *pixel = (u32*)pixel_line + left;
			for (i32 pixel_x = 0; pixel_x <= x_range; ++pixel_x){
				*pixel = color;
				++pixel;
			}
		}
	}
}

internal void
draw_rectangle_blend_2corner_unclamped(Render_Target *target,
								 Blit_Rect rect, u32 color){
	if (rect.x_start < rect.x_end && rect.y_start < rect.y_end){
		i32 y_range = rect.y_end - rect.y_start;
		i32 x_range = rect.x_end - rect.x_start;
		
		u8 a,r,g,b;
		a = (u8)(color >> 24);
		r = (u8)(color >> 16);
		g = (u8)(color >> 8);
		b = (u8)(color >> 0);
		
		real32 blend = (a/255.f);
		real32 pr, pg, pb;
		pr = r*blend;
		pg = g*blend;
		pb = b*blend;
		
		real32 inv_blend = 1.f - blend;
		
		u8 *pixel_line = (u8*)target->pixel_data + rect.y_start*target->pitch;
		for (i32 pixel_y = 0; pixel_y < y_range; ++pixel_y){
			u32 *pixel = (u32*)pixel_line + rect.x_start;
			for (i32 pixel_x = 0; pixel_x < x_range; ++pixel_x){
				u8 dr,dg,db;
				dr = (u8)(*pixel >> 16);
				dg = (u8)(*pixel >> 8);
				db = (u8)(*pixel >> 0);
				
				dr = (u8)(dr*inv_blend + pr);
				dg = (u8)(dg*inv_blend + pg);
				db = (u8)(db*inv_blend + pb);
				
				*pixel = (dr << 16) | (dg << 8) | (db);
				++pixel;
			}
			pixel_line += target->pitch;
		}
	}
}

internal void
draw_rectangle_noblend_2corner_unclamped(Render_Target *target,
										   Blit_Rect rect, u32 color){
	if (rect.x_start < rect.x_end && rect.y_start < rect.y_end){
		i32 y_range = rect.y_end - rect.y_start;
		i32 x_range = rect.x_end - rect.x_start;
		
		u8 *pixel_line = (u8*)target->pixel_data + rect.y_start*target->pitch;
		for (i32 pixel_y = 0; pixel_y < y_range; ++pixel_y){
			u32 *pixel = (u32*)pixel_line + rect.x_start;
			for (i32 pixel_x = 0; pixel_x < x_range; ++pixel_x){
				*pixel++ = color;
			}
			pixel_line += target->pitch;
		}
	}
}

// NOTE(allen): uses of this can be replaced with draw_rectangle_2corner_unclamped
// if it is guaranteed that clip_box will be within the target area.
inline void
draw_rectangle_2corner_clipped(Render_Target *target,
							   i32 left, i32 top, i32 right, i32 bottom,
							   u32 color, Blit_Rect clip_box){
	clip_box = rect_clamp_to_rect(clip_box, rect_from_target(target));
	Blit_Rect rect = rect_clamp_to_rect(left, top, right, bottom, clip_box);
	draw_rectangle_noblend_2corner_unclamped(target, rect, color);
}

inline void
draw_rectangle_2corner(Render_Target *target,
					   i32 left, i32 top, i32 right, i32 bottom,
					   u32 color){
	Blit_Rect rect = rect_clamp_to_rect(left, top, right, bottom, rect_from_target(target));
	draw_rectangle_noblend_2corner_unclamped(target, rect, color);
}

inline void
draw_rectangle_clipped(Render_Target *target,
					   i32 x, i32 y, i32 w, i32 h,
					   u32 color, Blit_Rect clip_box){
	draw_rectangle_2corner_clipped(target, x, y, x+w, y+h, color, clip_box);
}

inline void
draw_rectangle(Render_Target *target,
			   i32 x, i32 y, i32 w, i32 h,
			   u32 color){
	draw_rectangle_2corner(target, x, y, x+w, y+h, color);
}

internal void
draw_rectangle_outline_unclamped(Render_Target *target,
								 i32 x, i32 y, i32 w, i32 h,
								 u32 color, Blit_Rect rect){
	if (rect.x_start <= rect.x_end && rect.y_start <= rect.y_end){
		if (rect.y_start == y){
			draw_horizontal_line(target,
								 rect.y_start, rect.x_start, rect.x_end-1,
								 color);
		}
		
		if (rect.y_end == y+h){
			draw_horizontal_line(target,
								 rect.y_end-1, rect.x_start, rect.x_end-1,
								 color);
		}
		
		if (rect.x_start == x){
			draw_vertical_line(target,
							   rect.x_start, rect.y_start, rect.y_end-1,
							   color);
		}
		
		if (rect.x_end == x+w){
			draw_vertical_line(target,
							   rect.x_end-1, rect.y_start, rect.y_end-1,
							   color);
		}
	}
}

internal void
draw_rectangle_outline_clipped(Render_Target *target,
							   i32 x, i32 y, i32 w, i32 h,
							   u32 color, Blit_Rect clip_box){
	clip_box = rect_clamp_to_rect(clip_box, rect_from_target(target));
	Blit_Rect rect = rect_clamp_to_rect(x, y, x+w, y+h, clip_box);
	draw_rectangle_outline_unclamped(target, x, y, w, h, color, rect);
}

internal void
draw_rectangle_outline(Render_Target *target,
					   i32 x, i32 y, i32 w, i32 h,
					   u32 color){
	Blit_Rect rect = rect_clamp_to_rect(x, y, x+w, y+h, rect_from_target(target));
	draw_rectangle_outline_unclamped(target, x, y, w, h, color, rect);
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
font_load(Font *font_out, i32 pt_size,
          void *font_block, i32 font_block_size,
          i32 *memory_used_out){
    i32 result = 1;
    File_Data file;
    file = system_load_file((u8*)"liberation-mono.ttf");
    
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
            
            i32 max_advance = 0;
            
            bool32 block_full = 0, block_overfull = 0;
            u8 *memory_cursor = (u8*)font_block;
            for (u16 code_point = 0; code_point < 128; ++code_point){
                i32 glyph_index;
                
                if (block_full){
                    block_overfull = 1;
                    font_out->glyphs[code_point] = {};
                    continue;
                }
                
                else{
                    glyph_index = stbtt_FindGlyphIndex(&font, code_point);
                    if (glyph_index != 0){
                        font_out->glyphs[code_point].exists = 1;
                        
                        i32 left, right, top, bottom;
                        stbtt_GetGlyphBitmapBox(&font, glyph_index, scale, scale, &left, &top, &right, &bottom);
                        
                        i32 glyph_width, glyph_height;
                        i32 data_width, data_height;
                        glyph_width = right - left;
                        glyph_height = bottom - top;
                        data_width = glyph_width + 2;
                        data_height = glyph_height + 2;
                        
                        font_out->glyphs[code_point].minx = left;
                        font_out->glyphs[code_point].maxx = right;
                        font_out->glyphs[code_point].miny = top;
                        font_out->glyphs[code_point].maxy = bottom;
                        font_out->glyphs[code_point].width = data_width;
                        font_out->glyphs[code_point].height = data_height;
                        
                        i32 advance, left_bearing;
                        stbtt_GetGlyphHMetrics(&font, glyph_index, &advance, &left_bearing);
                        
                        font_out->glyphs[code_point].left_shift = (i32)(scale*left_bearing);
                        
                        if (advance > max_advance){
                            max_advance = advance;
                        }
                        
                        u8 *data = memory_cursor;
                        memory_cursor = memory_cursor + data_width*data_height;
                        
                        i32 size_after_glyph = (i32)((u8*)memory_cursor - (u8*)font_block);
                        if (size_after_glyph >= font_block_size){
                            block_full = 1;
                            if (size_after_glyph > font_block_size){
                                block_overfull = 1;
                            }
                        }
                        else{
                            font_out->glyphs[code_point].data = data;
                            u8 *data_start = data + data_width + 1;
                            stbtt_MakeGlyphBitmap(&font, data_start,
                                                  glyph_width, glyph_height, data_width,
                                                  scale, scale,
                                                  glyph_index);
                        }
                    }
                    else{
                        font_out->glyphs[code_point] = {};
                    }
                }
            }
            
            font_out->advance = Ceil(max_advance*scale);
        }
        system_free_file(file);
    }
    
    return result;
}

internal void
font_draw_glyph_subpixel(Render_Target *target,
                         Font *font, u16 character,
                         real32 x, real32 y, u32 color,
                         Blit_Rect clip){
	if (clip.x_start <= clip.x_end && clip.y_start <= clip.y_end){
        Glyph_Data glyph = font->glyphs[character];
        
        Vec2 s_origin = V2(x - 1.f, y - 1.f);
        
        i32 left_most = (i32)(x);
        i32 top_most = (i32)(y);
        
        real32 xe = x + glyph.width - 2;
        real32 ye = y + glyph.height - 2;
        i32 right_most = (i32)(xe)+(xe>0)*((i32)xe != xe);
        i32 bottom_most = (i32)(ye)+(ye>0)*((i32)ye != ye);
        
        if (left_most < clip.x_start){
            left_most = clip.x_start;
        }
        
        if (top_most < clip.y_start){
            top_most = clip.y_start;
        }
        
        if (right_most >= clip.x_end){
            right_most = clip.x_end - 1;
        }
        
        if (bottom_most >= clip.y_end){
            bottom_most = clip.y_end - 1;
        }
        
        u8 *dest_data = (u8*)target->pixel_data;
        u8 *src_data = (u8*)glyph.data;
        
        i32 width = glyph.width;
        i32 target_pitch = target->pitch;
        
        real32 inv_255 = 1.f / 255.f;
        
        real32 cr,cg,cb,ca;
        cb = (real32)((color) & 0xFF);
        cg = (real32)((color >> 8) & 0xFF);
        cr = (real32)((color >> 16) & 0xFF);
        ca = (real32)((color >> 24) & 0xFF);
        
        i32 lox_start = (i32)(left_most - s_origin.x);
        i32 loy_start = (i32)(top_most - s_origin.y);
        
        real32 lX = left_most - s_origin.x - lox_start;
        real32 lY = top_most - s_origin.y - loy_start;
        
        real32 inv_lX = 1.f - lX;
        real32 inv_lY = 1.f - lY;
        
        i32 loy = loy_start * width;
        
        u8 *dest_line = (dest_data + top_most*target_pitch);
        for (i32 y = top_most; y <= bottom_most; ++y){
            u8 src_a[4];
            
            i32 lox_loy = lox_start + loy;
            src_a[0] = *(src_data + (lox_loy));
            src_a[2] = *(src_data + (lox_loy + width));
            
            u32 *dest_pixel = (u32*)(dest_line) + left_most;
            
            for (i32 x = left_most; x <= right_most; ++x){
                src_a[1] = *(src_data + (lox_loy + 1));
                src_a[3] = *(src_data + (lox_loy + 1 + width));
                
                real32 alpha = (src_a[0]*(inv_lX) + src_a[1]*(lX))*(inv_lY) + 
                    (src_a[2]*(inv_lX) + src_a[3]*(lX))*(lY);
                alpha *= inv_255;
                
                u32 dp = *dest_pixel;
                
                real32 dr,dg,db,da;
                db = (real32)((dp) & 0xFF);
                dg = (real32)((dp >> 8) & 0xFF);
                dr = (real32)((dp >> 16) & 0xFF);
                da = (real32)((dp >> 24) & 0xFF);
                
                db = db + (cb - db)*alpha;
                dg = dg + (cg - dg)*alpha;
                dr = dr + (cr - dr)*alpha;
                da = da + (ca - da)*alpha;
                
                *dest_pixel = ((u8)db) | (((u8)dg) << 8) | (((u8)dr) << 16) | (((u8)da) << 24);
                
                ++dest_pixel;
                ++lox_loy;
                
                src_a[0] = src_a[1];
                src_a[2] = src_a[3];
            }
            loy += width;
            dest_line += target_pitch;
        }
    }
}

internal void
font_draw_glyph(Render_Target *target, Font *font, u16 character,
                real32 x, real32 y, u32 color){
	Glyph_Data glyph = font->glyphs[character];
    
    i32 left = glyph.minx;
    i32 right = glyph.maxx;
    i32 width = right - left;
    
    real32 x_shift, y_shift;
    x_shift = glyph.left_shift + (real32)width / font->advance;
    y_shift = (real32)font->ascent + glyph.miny;
    
    x += x_shift;
    y += y_shift;
    
    i32 xi, yi;
    xi = (i32)(x);
    yi = (i32)(y);
    
	Blit_Rect rect = rect_clamp_to_rect(xi, yi, xi+glyph.width-1, yi+glyph.height-1, rect_from_target(target));
    font_draw_glyph_subpixel(target, font, character, x, y, color, rect);
}

internal void
font_draw_glyph_clipped(Render_Target *target,
						Font *font, u16 character,
						real32 x, real32 y, u32 color,
						Blit_Rect clip_box){
	Glyph_Data glyph = font->glyphs[character];
    
    i32 left = glyph.minx;
    i32 right = glyph.maxx;
    i32 width = right - left;
    
    real32 x_shift, y_shift;
    x_shift = glyph.left_shift + (real32)width / font->advance;
    y_shift = (real32)font->ascent + glyph.miny;
    
    x += x_shift;
    y += y_shift;
    
    i32 xi, yi;
    xi = (i32)(x);
    yi = (i32)(y);
    
	clip_box = rect_clamp_to_rect(clip_box, rect_from_target(target));
	Blit_Rect rect = rect_clamp_to_rect(xi, yi, xi+glyph.width-1, yi+glyph.height-1, clip_box);
    font_draw_glyph_subpixel(target, font, character, x, y, color, rect);
}

#else

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
    draw_gradient_2corner_clipped(target, real32R(l,t,r,b), color_left, color_right);
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
    draw_rectangle_outline(target, real32R(rect), color);
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
font_load(Font *font_out, char *filename, i32 pt_size,
          void *font_block, i32 font_block_size,
          i32 *memory_used_out, i32 tab_width){
    i32 result = 1;
    File_Data file;
    file = system_load_file((u8*)filename);
    
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
                    if (max_advance < advance){
                        max_advance = advance;
                    }
                }
            }
            font_out->advance = max_advance - 1;
        }
        system_free_file(file);
    }
    
    return result;
}

internal void
font_set_tabwidth(Font *font, i32 tab_width){
    font->chardata['\t'].xadvance *= font->chardata[' '].xadvance * tab_width;
}

internal void
font_draw_glyph_mono(Render_Target *target, Font *font, u16 character,
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
font_draw_glyph_mono(Render_Target *target, Font *font, u16 character,
                     real32 x, real32 y, u32 color){
    font_draw_glyph_mono(target, font, character, x, y, (real32)font->advance, color);
}

internal void
font_draw_glyph(Render_Target *target, Font *font, u16 character,
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
font_get_glyph_width(Font *font, u16 character){
    return font->chardata[character].xadvance;
}

internal i32
draw_string(Render_Target *target, Font *font, char *str,
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
draw_string_mono(Render_Target *target, Font *font, char *str,
                 real32 x, real32 y, real32 advance, u32 color){
    for (i32 i = 0; str[i]; ++i){
        font_draw_glyph_mono(target, font, str[i],
                             x, y, advance, color);
        x += advance;
    }
    return x;
}

internal i32
draw_string(Render_Target *target, Font *font, String str,
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
draw_string_mono(Render_Target *target, Font *font, String str,
                 real32 x, real32 y, real32 advance, u32 color){
    for (i32 i = 0; i < str.size; ++i){
        font_draw_glyph_mono(target, font, str.str[i],
                             x, y, advance, color);
        x += advance;
    }
    return x;
}

internal real32
font_get_max_width(Font *font, char *characters){
    stbtt_bakedchar *chardata = font->chardata;
    real32 cx, x = 0;
    for (i32 i = 0; characters[i]; ++i){
        cx = chardata[characters[i]].xadvance;
        if (x < cx) x = cx;
    }
    return x;
}

internal real32
font_get_string_width(Font *font, String string){
    real32 result = 0;
    for (i32 i = 0; i < string.size; ++i){
        font_get_glyph_width(font, string.str[i]);
    }
    return result;
}

#endif

// BOTTOM
