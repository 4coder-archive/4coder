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

inline void
draw_pop_clip(Render_Target *target){
    target->pop_clip(target);
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

inline internal i32
font_predict_size(i32 pt_size){
	return pt_size*pt_size*128;
}

internal i32
font_load(Render_Target *target, System_Functions *system,
          Render_Font *font_out, char *filename, i32 pt_size,
          void *font_block, i32 font_block_size,
          i32 *memory_used_out, i32 tab_width){
    i32 result =
        target->font_load(system, font_out, filename, pt_size,
                          font_block, font_block_size, memory_used_out, tab_width);
    return result;
}

internal void
font_set_tabwidth(Render_Font *font, i32 tab_width){
    font->chardata['\t'].xadvance *= font->chardata[' '].xadvance * tab_width;
}

// TODO(allen): Abstract Render_Font all the fucking way out of the app side code.
internal void
font_draw_glyph_mono(Render_Target *target, Render_Font *font, u16 character,
                     f32 x, f32 y, f32 advance, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_mono_glyph;
    piece.glyph.pos.x = x;
    piece.glyph.pos.y = y;
    piece.glyph.color = color;
    piece.glyph.font_id = font;
    piece.glyph.character = character;
    target->push_piece(target, piece);
}

inline void
font_draw_glyph_mono(Render_Target *target, Render_Font *font, u16 character,
                     real32 x, real32 y, u32 color){
    font_draw_glyph_mono(target, font, character, x, y, (real32)font->advance, color);
}

internal void
font_draw_glyph(Render_Target *target, Render_Font *font, u16 character,
                f32 x, f32 y, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_glyph;
    piece.glyph.pos.x = x;
    piece.glyph.pos.y = y;
    piece.glyph.color = color;
    piece.glyph.font_id = font;
    piece.glyph.character = character;
    target->push_piece(target, piece);
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

// BOTTOM
