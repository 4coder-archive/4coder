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
draw_push_clip(Render_Target *target, i32_Rect clip_box){
    target->push_clip(target, clip_box);
}

inline i32_Rect
draw_pop_clip(Render_Target *target){
    i32_Rect result = target->pop_clip(target);
    return(result);
}

inline void
draw_change_clip(Render_Target *target, i32_Rect clip_box){
    target->pop_clip(target);
    target->push_clip(target, clip_box);
}

internal void
begin_render_section(Render_Target *target, System_Functions *system){
    Font_Set *font_set = &target->font_set;
    font_set->used_this_frame = 0;
    memset(font_set->font_used_flags, 0, font_set->max);
    target->size = 0;
    target->clip_top = -1;
    
    i32_Rect clip;
    clip.x0 = 0;
    clip.y0 = 0;
    clip.x1 = target->width;
    clip.y1 = target->height;
    draw_push_clip(target, clip);
}

internal void
end_render_section(Render_Target *target, System_Functions *system){
    Assert(target->clip_top == 0);
}

internal void
draw_rectangle(Render_Target *target, i32_Rect rect, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_rectangle;
    piece.rectangle.rect = f32R(rect);
    piece.rectangle.color = color;
    target->push_piece(target, piece);
}

internal void
draw_rectangle(Render_Target *target, f32_Rect rect, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_rectangle;
    piece.rectangle.rect = rect;
    piece.rectangle.color = color;
    target->push_piece(target, piece);
}

internal void
draw_gradient_2corner_clipped(Render_Target *target, f32_Rect rect,
                              Vec4 left_color, Vec4 right_color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_gradient;
    piece.gradient.rect = rect;
    piece.gradient.left_color = pack_color4(left_color);
    piece.gradient.right_color = pack_color4(right_color);
    target->push_piece(target, piece);
}

inline void
draw_gradient_2corner_clipped(Render_Target *target, f32 l, f32 t, f32 r, f32 b,
                              Vec4 color_left, Vec4 color_right){
    draw_gradient_2corner_clipped(target, f32R(l,t,r,b), color_left, color_right);
}

internal void
draw_rectangle_outline(Render_Target *target, f32_Rect rect, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_outline;
    piece.rectangle.rect = rect;
    piece.rectangle.color = color;
    target->push_piece(target, piece);
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

inline void
draw_margin(Render_Target *target, i32_Rect outer, i32 width, u32 color){
    i32_Rect inner = get_inner_rect(outer, width);
    draw_margin(target, outer, inner, color);
}

inline internal i32
font_predict_size(i32 pt_size){
    return pt_size*pt_size*128;
}

internal void
font_draw_glyph(Render_Target *target, i16 font_id, i32 type, u8 character, f32 x, f32 y, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = type;
    piece.glyph.pos.x = x;
    piece.glyph.pos.y = y;
    piece.glyph.color = color;
    piece.glyph.font_id = font_id;
    piece.glyph.character = character;
    target->push_piece(target, piece);
    font_set_use(&target->font_set, font_id);
}

internal void
font_draw_glyph(Render_Target *target, i16 font_id, u8 character, f32 x, f32 y, u32 color){
    font_draw_glyph(target, font_id, piece_type_glyph, character, x, y, color);
}

internal f32
draw_string_base(Render_Target *target, i16 font_id, i32 type, String str_, i32 x_, i32 y_, u32 color){
    Font_Info *font_info = get_font_info(&target->font_set, font_id);
    Render_Font *font = font_info->font;
    f32 x = 0;
    
    if (font){
        f32 y = (f32)y_;
        x = (f32)x_;
        
        f32 byte_advance = font->byte_advance;
        
        u8 *str = (u8*)str_.str;
        u8 *str_end = str + str_.size;
        
        for (;str < str_end;){
            u8 *byte = str;
            u32 codepoint = utf8_to_u32(&str, str_end);
            
            b32 do_codepoint = false;
            b32 do_numbers = false;
            if (codepoint){
                if (codepoint >= ' ' && codepoint <= 0xFF && codepoint != 127){
                    do_codepoint = true;
                }
                else{
                    do_numbers = true;
                }
            }
            else{
                do_numbers = true;
            }
            
            if (do_codepoint){
                if (color != 0){
                    font_draw_glyph(target, font_id, type, (u8)codepoint, x, y, color);
                }
                x += get_codepoint_advance(font, codepoint);
            }
            else if (do_numbers){
                for (;byte < str; ++byte){
                    u8_4tech n = *byte;
                    if (color != 0){
                        u8 cs[3];
                        cs[0] = '\\';
                        byte_to_ascii(n, cs+1);
                        
                        f32 xx = x;
                        for (u32 j = 0; j < 3; ++j){
                            font_draw_glyph(target, font_id, type, cs[j], xx, y, color);
                            xx += byte_advance;
                        }
                    }
                    
                    x += byte_advance;
                }
            }
        }
    }
    
    return(x);
}

internal f32
draw_string(Render_Target *target, i16 font_id, String str, i32 x, i32 y, u32 color){
    f32 w = draw_string_base(target, font_id, piece_type_glyph, str, x, y, color);
    return(w);
}

internal f32
draw_string(Render_Target *target, i16 font_id, char *str, i32 x, i32 y, u32 color){
    String string = make_string_slowly(str);
    f32 w = draw_string_base(target, font_id, piece_type_glyph, string, x, y, color);
    return(w);
}

internal f32
draw_string_mono(Render_Target *target, i16 font_id, String str, i32 x, i32 y, f32 advance, u32 color){
    f32 w = draw_string_base(target, font_id, piece_type_mono_glyph, str, x, y, color);
    return(w);
}

internal f32
draw_string_mono(Render_Target *target, i16 font_id, char *str, i32 x, i32 y, f32 advance, u32 color){
    String string = make_string_slowly(str);
    f32 w = draw_string_base(target, font_id, piece_type_mono_glyph, string, x, y, color);
    return(w);
}

internal f32
font_string_width(Render_Target *target, i16 font_id, String str){
    f32 w = draw_string_base(target, font_id, piece_type_glyph, str, 0, 0, 0);
    return(w);
}

internal f32
font_string_width(Render_Target *target, i16 font_id, char *str){
    String string = make_string_slowly(str);
    f32 w = draw_string_base(target, font_id, piece_type_glyph, string, 0, 0, 0);
    return(w);
}

// BOTTOM

