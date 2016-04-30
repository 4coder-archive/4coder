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

inline void
draw_change_clip(Render_Target *target, i32_Rect clip_box){
    target->pop_clip(target);
    target->push_clip(target, clip_box);
}

internal void
begin_render_section(Render_Target *target, System_Functions *system){
    Font_Set *font_set = &target->font_set;
    system->acquire_lock(RENDER_LOCK);
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
    system->release_lock(RENDER_LOCK);
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
font_set_tabwidth(Render_Font *font, i32 tab_width){
    font->chardata['\t'].xadvance *= font->chardata[' '].xadvance * tab_width;
}

internal void
font_draw_glyph_mono(Render_Target *target, i16 font_id,
                     u8 character, f32 x, f32 y, f32 advance, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_mono_glyph;
    piece.glyph.pos.x = x;
    piece.glyph.pos.y = y;
    piece.glyph.color = color;
    piece.glyph.font_id = font_id;
    piece.glyph.character = character;
    target->push_piece(target, piece);
    font_set_use(target->partition, &target->font_set, font_id);
}

inline void
font_draw_glyph_mono(Render_Target *target, i16 font_id,
                     u8 character, f32 x, f32 y, u32 color){
    f32 advance = (f32)get_font_info(&target->font_set, font_id)->advance;
    font_draw_glyph_mono(target, font_id, character, x, y, advance, color);
}

internal void
font_draw_glyph(Render_Target *target, i16 font_id,
                u8 character, f32 x, f32 y, u32 color){
    Render_Piece_Combined piece;
    piece.header.type = piece_type_glyph;
    piece.glyph.pos.x = x;
    piece.glyph.pos.y = y;
    piece.glyph.color = color;
    piece.glyph.font_id = font_id;
    piece.glyph.character = character;
    target->push_piece(target, piece);
    font_set_use(target->partition, &target->font_set, font_id);
}

inline f32
font_get_glyph_width(Render_Target *target, i16 font_id, u16 character){
    Render_Font *font = get_font_info(&target->font_set, font_id)->font;
    f32 result = 0.f;
    if (font) result = font->chardata[character].xadvance;
    return (result);
}

internal f32
font_string_width(Render_Target *target, i16 font_id, char *str){
    f32 x = 0;
    for (i32 i = 0; str[i]; ++i){
        u8 c = str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        x += font_get_glyph_width(target, font_id, c);
    }
    return x;
}

internal f32
font_string_width(Render_Target *target, i16 font_id, String str){
    f32 x = 0;
    for (i32 i = 0; i < str.size; ++i){
        u8 c = str.str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        x += font_get_glyph_width(target, font_id, c);
    }
    return x;
}

internal f32
draw_string(Render_Target *target, i16 font_id,
            char *str, i32 x_, i32 y, u32 color){
    real32 x = (real32)x_;
    for (i32 i = 0; str[i]; ++i){
        u8 c = str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        font_draw_glyph(target, font_id, c, x, (f32)y, color);
        x += font_get_glyph_width(target, font_id, c);
    }
    return x;
}

internal f32
draw_string_mono(Render_Target *target, i16 font_id,
                 char *str, f32 x, f32 y, f32 advance, u32 color){
    for (i32 i = 0; str[i]; ++i){
        u8 c = str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        font_draw_glyph_mono(target, font_id, c, x, y, advance, color);
        x += advance;
    }
    return x;
}

internal f32
draw_string(Render_Target *target, i16 font_id,
            String str, i32 x_, i32 y, u32 color){
    f32 x = (f32)x_;
    for (i32 i = 0; i < str.size; ++i){
        u8 c = str.str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        font_draw_glyph(target, font_id, c,
                        x, (f32)y, color);
        x += font_get_glyph_width(target, font_id, c);
    }
    return x;
}

internal f32
draw_string_mono(Render_Target *target, i16 font_id,
                 String str, f32 x, f32 y, f32 advance, u32 color){
    for (i32 i = 0; i < str.size; ++i){
        u8 c = str.str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        font_draw_glyph_mono(target, font_id, c, x, y, advance, color);
        x += advance;
    }
    return x;
}

internal f32
font_get_max_width(Font_Set *font_set, i16 font_id, char *characters){
    Render_Font *font = get_font_info(font_set, font_id)->font;
    f32 cx, x = 0;
    if (font){
        stbtt_packedchar *chardata = font->chardata;
        for (i32 i = 0; characters[i]; ++i){
            cx = chardata[characters[i]].xadvance;
            if (x < cx) x = cx;
        }
    }
    return x;
}

internal f32
font_get_string_width(Render_Target *target, i16 font_id, String string){
    f32 result = 0;
    for (i32 i = 0; i < string.size; ++i){
        u8 c = string.str[i];
        // TODO(allen): Someday let's not punt on the the unicode rendering
        c = c % 128;
        font_get_glyph_width(target, font_id, c);
    }
    return result;
}

// BOTTOM

