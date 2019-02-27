/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render buffer fill helpers.
 *
 */

// TOP

internal void
draw_push_clip(Render_Target *target, i32_Rect clip_box){
    render_push_clip(target, clip_box);
}

internal i32_Rect
draw_pop_clip(Render_Target *target){
    i32_Rect result = render_pop_clip(target);
    return(result);
}

internal void
draw_change_clip(Render_Target *target, i32_Rect clip_box){
    render_change_clip(target, clip_box);
}

internal void
begin_frame(Render_Target *target){
    target->buffer.pos = 0;
}

internal void
begin_render_section(Render_Target *target, System_Functions *system,
                     i32 frame_index, f32 literal_dt, f32 animation_dt){
    target->clip_top = -1;
    
    i32_Rect clip;
    clip.x0 = 0;
    clip.y0 = 0;
    clip.x1 = target->width;
    clip.y1 = target->height;
    draw_push_clip(target, clip);
    
    target->frame_index = frame_index;
    target->literal_dt = literal_dt;
    target->animation_dt = animation_dt;
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

internal void
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

internal void
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

internal void
draw_margin(Render_Target *target, i32_Rect outer, i32 width, u32 color){
    i32_Rect inner = get_inner_rect(outer, width);
    draw_margin(target, outer, inner, color);
}

internal void
draw_font_glyph(Render_Target *target, Face_ID font_id, u32 codepoint, f32 x, f32 y, u32 color, u32 flags){
    Render_Command_Glyph cmd;
    CmdHeader(RenCom_Glyph);
    cmd.pos.x = x;
    cmd.pos.y = y;
    cmd.color = color;
    cmd.font_id = font_id;
    cmd.codepoint = codepoint;
    cmd.flags = flags;
    void *h = render_begin_push(target, &cmd, cmd.header.size);
    render_end_push(target, h);
}

internal Vec2
snap_point_to_boundary(Vec2 point){
    point.x = (f32)(floor32(point.x));
    point.y = (f32)(floor32(point.y));
    return(point);
}

internal f32
draw_string(System_Functions *system, Render_Target *target, Face_ID font_id, String string, Vec2 point,
            u32 color, u32 flags, Vec2 delta){
    f32 total_delta = 0.f;
    
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    if (font.valid != 0){
        point = snap_point_to_boundary(point);
        
        f32 byte_advance = font.metrics->byte_advance;
        f32 *sub_advances = font.metrics->sub_advances;
        
        u8 *str = (u8*)string.str;
        u8 *str_end = str + string.size;
        
        Translation_State tran = {};
        Translation_Emits emits = {};
        
        for (u32 i = 0; str < str_end; ++str, ++i){
            translating_fully_process_byte(system, font, &tran, *str, i, string.size, &emits);
            
            for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                TRANSLATION_DECL_GET_STEP(step, behavior, J, emits);
                
                if (behavior.do_codepoint_advance){
                    u32 codepoint = step.value;
                    if (color != 0){
                        draw_font_glyph(target, font_id, codepoint, point.x, point.y, color, flags);
                    }
                    f32 d = font_get_glyph_advance(system, font.settings, font.metrics, font.pages, codepoint);
                    point += d*delta;
                    total_delta += d;
                }
                else if (behavior.do_number_advance){
                    u8 n = (u8)(step.value);
                    if (color != 0){
                        u8 cs[3];
                        cs[0] = '\\';
                        byte_to_ascii(n, cs+1);
                        Vec2 pp = point;
                        for (u32 j = 0; j < 3; ++j){
                            draw_font_glyph(target, font_id, cs[j], pp.x, pp.y, color, flags);
                            pp += delta*sub_advances[j];
                        }
                    }
                    point += byte_advance*delta;
                    total_delta += byte_advance;
                }
            }
        }
    }
    
    return(total_delta);
}

internal f32
draw_string(System_Functions *system, Render_Target *target, Face_ID font_id, String string, Vec2 point,
            u32 color){
    return(draw_string(system, target, font_id, string, point, color, 0, V2(1.f, 0.f)));
}

internal f32
draw_string(System_Functions *system, Render_Target *target, Face_ID font_id, char *str, Vec2 point,
            u32 color, u32 flags, Vec2 delta){
    return(draw_string(system, target, font_id, make_string_slowly(str), point, color, flags, delta));
}

internal f32
draw_string(System_Functions *system, Render_Target *target, Face_ID font_id, char *str, Vec2 point,
            u32 color){
    return(draw_string(system, target, font_id, make_string_slowly(str), point, color, 0, V2(1.f, 0.f)));
}

internal f32
font_string_width(System_Functions *system, Render_Target *target, Face_ID font_id, String str){
    return(draw_string(system, target, font_id, str, V2(0, 0), 0, 0, V2(0, 0)));
}

internal f32
font_string_width(System_Functions *system, Render_Target *target, Face_ID font_id, char *str){
    return(draw_string(system, target, font_id, make_string_slowly(str), V2(0, 0), 0, 0, V2(0, 0)));
}

// BOTTOM

