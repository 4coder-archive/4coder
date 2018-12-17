/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render buffer fill helpers.
 *
 */

// TOP

inline void
draw_push_clip(Render_Target *target, i32_Rect clip_box){
    render_push_clip(target, clip_box);
}

inline i32_Rect
draw_pop_clip(Render_Target *target){
    i32_Rect result = render_pop_clip(target);
    return(result);
}

inline void
draw_change_clip(Render_Target *target, i32_Rect clip_box){
    render_change_clip(target, clip_box);
}

internal void
begin_render_section(Render_Target *target, System_Functions *system){
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

#define CmdHeader(t) cmd.header.size = sizeof(cmd), cmd.header.type = t

internal void
draw_rectangle(Render_Target *target, f32_Rect rect, u32 color){
    Render_Command_Rectangle cmd = {};
    CmdHeader(RenCom_Rectangle);
    cmd.rect = rect;
    cmd.color = color;
    void *h = render_begin_push(target, &cmd, cmd.header.size);
    render_end_push(target, h);
}

internal void
draw_rectangle(Render_Target *target, i32_Rect rect, u32 color){
    draw_rectangle(target, f32R(rect), color);
}

internal void
draw_rectangle_outline(Render_Target *target, f32_Rect rect, u32 color){
    Render_Command_Rectangle cmd = {};
    CmdHeader(RenCom_Outline);
    cmd.rect = rect;
    cmd.color = color;
    void *h = render_begin_push(target, &cmd, cmd.header.size);
    render_end_push(target, h);
}

inline void
draw_rectangle_outline(Render_Target *target, i32_Rect rect, u32 color){
    draw_rectangle_outline(target, f32R(rect), color);
}

internal void
draw_margin(Render_Target *target, f32_Rect outer, f32_Rect inner, u32 color){
    draw_rectangle(target, f32R(outer.x0, outer.y0, outer.x1, inner.y0), color);
    draw_rectangle(target, f32R(outer.x0, inner.y1, outer.x1, outer.y1), color);
    draw_rectangle(target, f32R(outer.x0, inner.y0, inner.x0, inner.y1), color);
    draw_rectangle(target, f32R(inner.x1, inner.y0, outer.x1, inner.y1), color);
}

inline void
draw_margin(Render_Target *target, f32_Rect outer, f32 width, u32 color){
    f32_Rect inner = get_inner_rect(outer, width);
    draw_margin(target, outer, inner, color);
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

internal void
draw_font_glyph(Render_Target *target, Face_ID font_id, u32 codepoint, f32 x, f32 y, u32 color){
    Render_Command_Glyph cmd;
    CmdHeader(RenCom_Glyph);
    cmd.pos.x = x;
    cmd.pos.y = y;
    cmd.color = color;
    cmd.font_id = font_id;
    cmd.codepoint = codepoint;
    void *h = render_begin_push(target, &cmd, cmd.header.size);
    render_end_push(target, h);
}

internal f32
draw_string_base(System_Functions *system, Render_Target *target, Face_ID font_id, String str_, i32 x_, i32 y_, u32 color){
    f32 x = 0;
    
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    if (font.valid != 0){
        f32 y = (f32)y_;
        x = (f32)x_;
        
        f32 byte_advance = font.metrics->byte_advance;
        f32 *sub_advances = font.metrics->sub_advances;
        
        u8 *str = (u8*)str_.str;
        u8 *str_end = str + str_.size;
        
        Translation_State tran = {};
        Translation_Emits emits = {};
        
        for (u32 i = 0; str < str_end; ++str, ++i){
            translating_fully_process_byte(system, font, &tran, *str, i, str_.size, &emits);
            
            for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                TRANSLATION_DECL_GET_STEP(step, behavior, J, emits);
                
                if (behavior.do_codepoint_advance){
                    u32 codepoint = step.value;
                    if (color != 0){
                        draw_font_glyph(target, font_id, codepoint, x, y, color);
                    }
                    x += font_get_glyph_advance(system, font.settings, font.metrics, font.pages, codepoint);
                }
                else if (behavior.do_number_advance){
                    u8 n = (u8)(step.value);
                    if (color != 0){
                        u8 cs[3];
                        cs[0] = '\\';
                        byte_to_ascii(n, cs+1);
                        
                        f32 xx = x;
                        for (u32 j = 0; j < 3; ++j){
                            draw_font_glyph(target, font_id, cs[j], xx, y, color);
                            xx += sub_advances[j];
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
draw_string(System_Functions *system, Render_Target *target, Face_ID font_id, String str, i32 x, i32 y, u32 color){
    f32 w = draw_string_base(system, target, font_id, str, x, y, color);
    return(w);
}

internal f32
draw_string(System_Functions *system, Render_Target *target, Face_ID font_id, char *str, i32 x, i32 y, u32 color){
    String string = make_string_slowly(str);
    f32 w = draw_string_base(system, target, font_id, string, x, y, color);
    return(w);
}

internal f32
font_string_width(System_Functions *system, Render_Target *target, Face_ID font_id, String str){
    f32 w = draw_string_base(system, target, font_id, str, 0, 0, 0);
    return(w);
}

internal f32
font_string_width(System_Functions *system, Render_Target *target, Face_ID font_id, char *str){
    String string = make_string_slowly(str);
    f32 w = draw_string_base(system, target, font_id, string, 0, 0, 0);
    return(w);
}

// BOTTOM

