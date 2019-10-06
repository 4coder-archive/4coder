/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render target function implementations.
 *
 */

// TOP

internal void
draw__begin_new_group(Render_Target *target){
    Render_Group *group = 0;
    if (target->group_last != 0){
        if (target->group_last->vertex_list.vertex_count == 0){
            group = target->group_last;
        }
    }
    if (group == 0){
        group = push_array_zero(&target->arena, Render_Group, 1);
        sll_queue_push(target->group_first, target->group_last, group);
    }
    group->face_id = target->current_face_id;
    group->clip_box = target->current_clip_box;
}

internal Render_Vertex_Array_Node*
draw__extend_group_vertex_memory(Arena *arena, Render_Vertex_List *list, i32 size){
    Render_Vertex_Array_Node *node = push_array_zero(arena, Render_Vertex_Array_Node, 1);
    sll_queue_push(list->first, list->last, node);
    node->vertices = push_array(arena, Render_Vertex, size);
    node->vertex_max = size;
    return(node);
}

internal void
draw__write_vertices_in_current_group(Render_Target *target, Render_Vertex *vertices, i32 count){
    if (count > 0){
        Render_Group *group = target->group_last;
        if (group == 0){
            draw__begin_new_group(target);
            group = target->group_last;
        }
        
        Render_Vertex_List *list = &group->vertex_list;
        
        Render_Vertex_Array_Node *last = list->last;
        
        Render_Vertex *tail_vertex = 0;
        i32 tail_count = 0;
        if (last != 0){
            tail_vertex = last->vertices + last->vertex_count;
            tail_count = last->vertex_max - last->vertex_count;
        }
        
        i32 base_vertex_max = 64;
        i32 transfer_count = clamp_top(count, tail_count);
        if (transfer_count > 0){
            block_copy_dynamic_array(tail_vertex, vertices, transfer_count);
            last->vertex_count += transfer_count;
            list->vertex_count += transfer_count;
            base_vertex_max = last->vertex_max;
        }
        
        i32 count_left_over = count - transfer_count;
        if (count_left_over > 0){
            Render_Vertex *vertices_left_over = vertices + transfer_count;
            
            i32 next_node_size = (base_vertex_max + count_left_over)*2;
            Render_Vertex_Array_Node *memory = draw__extend_group_vertex_memory(&target->arena, list, next_node_size);
            block_copy_dynamic_array(memory->vertices, vertices_left_over, count_left_over);
            memory->vertex_count += count_left_over;
            list->vertex_count += count_left_over;
        }
        
    }
}

internal void
draw__set_clip_box(Render_Target *target, Rect_i32 clip_box){
    if (target->current_clip_box != clip_box){
        target->current_clip_box = clip_box;
        draw__begin_new_group(target);
    }
}

internal void
draw__set_face_id(Render_Target *target, Face_ID face_id){
    if (target->current_face_id != face_id){
        if (target->current_face_id != 0){
            target->current_face_id = face_id;
            draw__begin_new_group(target);
        }
        else{
            target->current_face_id = face_id;
            for (Render_Group *group = target->group_first;
                 group != 0;
                 group = group->next){
                group->face_id = face_id;
            }
        }
    }
}

////////////////////////////////

internal void
draw_push_clip(Render_Target *target, Rect_i32 clip_box){
    if (target->clip_top != -1){
        clip_box = intersection_of(clip_box, target->clip_boxes[target->clip_top]);
    }
    Assert(target->clip_top + 1 < ArrayCount(target->clip_boxes));
    target->clip_boxes[++target->clip_top] = clip_box;
    draw__set_clip_box(target, clip_box);
}

internal Rect_i32
draw_pop_clip(Render_Target *target){
    Assert(target->clip_top > 0);
    Rect_i32 result = target->clip_boxes[target->clip_top];
    --target->clip_top;
    Rect_i32 clip_box = target->clip_boxes[target->clip_top];
    draw__set_clip_box(target, clip_box);
    return(result);
}

internal void
draw_change_clip(Render_Target *target, Rect_i32 clip_box){
    Assert(target->clip_top > -1);
    target->clip_boxes[target->clip_top] = clip_box;
    draw__set_clip_box(target, clip_box);
}

internal void
begin_frame(Render_Target *target, void *font_set){
    linalloc_clear(&target->arena);
    target->group_first = 0;
    target->group_last = 0;
    target->current_face_id = 0;
    target->current_clip_box = Ri32(0, 0, target->width, target->height);
    target->font_set = font_set;
}

internal void
begin_render_section(Render_Target *target, i32 frame_index, f32 literal_dt, f32 animation_dt){
    target->clip_top = -1;
    Rect_i32 clip = Ri32(0, 0, target->width, target->height);
    draw_push_clip(target, clip);
    target->frame_index = frame_index;
    target->literal_dt = literal_dt;
    target->animation_dt = animation_dt;
}

internal void
end_render_section(Render_Target *target){
    Assert(target->clip_top == 0);
}

////////////////////////////////

internal void
draw_rectangle(Render_Target *target, Rect_f32 rect, u32 color){
    Render_Vertex vertices[6] = {};
    vertices[0].xy = V2(rect.x0, rect.y0);
    vertices[1].xy = V2(rect.x1, rect.y0);
    vertices[2].xy = V2(rect.x0, rect.y1);
    vertices[3].xy = V2(rect.x1, rect.y0);
    vertices[4].xy = V2(rect.x0, rect.y1);
    vertices[5].xy = V2(rect.x1, rect.y1);
    Vec4 c = unpack_color4(color);
    for (i32 i = 0; i < 6; i += 1){
        vertices[i].color = c;
    }
    draw__write_vertices_in_current_group(target, vertices, 6);
}

internal void
draw_font_glyph(Render_Target *target, Face *face, u32 codepoint, f32 x, f32 y, u32 color, u32 flags){
    draw__set_face_id(target, face->id);
    
    u16 glyph_index = 0;
    if (!codepoint_index_map_read(&face->codepoint_to_index_map, codepoint, &glyph_index)){
        glyph_index = 0;
    }
    Glyph_Bounds bounds = face->bounds[glyph_index];
    Vec3_f32 texture_dim = face->texture_dim;
    
    Rect_f32 uv = bounds.uv;
    
    Render_Vertex vertices[6] = {};
    if (!HasFlag(flags, GlyphFlag_Rotate90)){
        Rect_f32 xy = Rf32(x + bounds.xy_off.x0, y + bounds.xy_off.y0,
                           x + bounds.xy_off.x1, y + bounds.xy_off.y1);
        
        vertices[0].xy = V2(xy.x0, xy.y1); vertices[0].uvw = V3(uv.x0, uv.y1, bounds.w);
        vertices[1].xy = V2(xy.x1, xy.y1); vertices[1].uvw = V3(uv.x1, uv.y1, bounds.w);
        vertices[2].xy = V2(xy.x0, xy.y0); vertices[2].uvw = V3(uv.x0, uv.y0, bounds.w);
        vertices[5].xy = V2(xy.x1, xy.y0); vertices[5].uvw = V3(uv.x1, uv.y0, bounds.w);
    }
    else{
        Rect_f32 xy = Rf32(x - bounds.xy_off.y1, y + bounds.xy_off.x0,
                           x - bounds.xy_off.y0, y + bounds.xy_off.x1);
        
        vertices[0].xy = V2(xy.x0, xy.y1); vertices[0].uvw = V3(uv.x1, uv.y1, bounds.w);
        vertices[1].xy = V2(xy.x1, xy.y1); vertices[1].uvw = V3(uv.x1, uv.y0, bounds.w);
        vertices[2].xy = V2(xy.x0, xy.y0); vertices[2].uvw = V3(uv.x0, uv.y1, bounds.w);
        vertices[5].xy = V2(xy.x1, xy.y0); vertices[5].uvw = V3(uv.x0, uv.y0, bounds.w);
    }
    
    vertices[3] = vertices[1];
    vertices[4] = vertices[2];
    
    Vec4 c = unpack_color4(color);
    for (i32 i = 0; i < 6; i += 1){
        vertices[i].color = c;
    }
    
    draw__write_vertices_in_current_group(target, vertices, 6);
}

////////////////////////////////

internal void
draw_rectangle_outline(Render_Target *target, Rect_f32 rect, u32 color){
    draw_rectangle(target, Rf32(rect.x0, rect.y0, rect.x1, rect.y0 + 1), color);
    draw_rectangle(target, Rf32(rect.x1 - 1, rect.y0, rect.x1, rect.y1), color);
    draw_rectangle(target, Rf32(rect.x0, rect.y1 - 1, rect.x1, rect.y1), color);
    draw_rectangle(target, Rf32(rect.x0, rect.y0, rect.x0 + 1, rect.y1), color);
}

////////////////////////////////

internal void
draw_rectangle(Render_Target *target, Rect_i32 rect, u32 color){
    draw_rectangle(target, Rf32(rect), color);
}

internal void
draw_rectangle_outline(Render_Target *target, Rect_i32 rect, u32 color){
    draw_rectangle_outline(target, Rf32(rect), color);
}

internal void
draw_margin(Render_Target *target, Rect_f32 outer, Rect_f32 inner, u32 color){
    draw_rectangle(target, Rf32(outer.x0, outer.y0, outer.x1, inner.y0), color);
    draw_rectangle(target, Rf32(outer.x0, inner.y1, outer.x1, outer.y1), color);
    draw_rectangle(target, Rf32(outer.x0, inner.y0, inner.x0, inner.y1), color);
    draw_rectangle(target, Rf32(inner.x1, inner.y0, outer.x1, inner.y1), color);
}

internal void
draw_margin(Render_Target *target, Rect_f32 outer, f32 width, u32 color){
    Rect_f32 inner = rect_inner(outer, width);
    draw_margin(target, outer, inner, color);
}

internal void
draw_margin(Render_Target *target, Rect_i32 outer, Rect_i32 inner, u32 color){
    draw_rectangle(target, Ri32(outer.x0, outer.y0, outer.x1, inner.y0), color);
    draw_rectangle(target, Ri32(outer.x0, inner.y1, outer.x1, outer.y1), color);
    draw_rectangle(target, Ri32(outer.x0, inner.y0, inner.x0, inner.y1), color);
    draw_rectangle(target, Ri32(inner.x1, inner.y0, outer.x1, inner.y1), color);
}

internal void
draw_margin(Render_Target *target, Rect_i32 outer, i32 width, u32 color){
    Rect_i32 inner = rect_inner(outer, width);
    draw_margin(target, outer, inner, color);
}

internal Vec2
snap_point_to_boundary(Vec2 point){
    point.x = (f32)(floor32(point.x));
    point.y = (f32)(floor32(point.y));
    return(point);
}

internal f32
draw_string(Render_Target *target, Face *face, String_Const_u8 string, Vec2 point, u32 color, u32 flags, Vec2 delta){
    f32 total_delta = 0.f;
    if (face != 0){
        point = snap_point_to_boundary(point);
        
        f32 byte_advance = face->byte_advance;
        f32 *byte_sub_advances = face->byte_sub_advances;
        
        u8 *str = (u8*)string.str;
        u8 *str_end = str + string.size;
        
        Translation_State tran = {};
        Translation_Emits emits = {};
        
        for (u32 i = 0; str < str_end; ++str, ++i){
            translating_fully_process_byte(&tran, *str, i, (i32)string.size, &emits);
            
            for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                TRANSLATION_DECL_GET_STEP(step, behavior, J, emits);
                
                if (behavior.do_codepoint_advance){
                    u32 codepoint = step.value;
                    if (color != 0){
                        draw_font_glyph(target, face, codepoint, point.x, point.y, color, flags);
                    }
                    f32 d = font_get_glyph_advance(face, codepoint);
                    point += d*delta;
                    total_delta += d;
                }
                else if (behavior.do_number_advance){
                    u8 n = (u8)(step.value);
                    if (color != 0){
                        u8 cs[3];
                        cs[0] = '\\';
                        u8 nh = (n >> 4);
                        u8 nl = (n & 0xF);
                        u8 ch = '0' + nh;
                        u8 cl = '0' + nl;
                        if (nh > 0x9){
                            ch = ('A' - 0xA) + nh;
                        }
                        if (nl > 0x9){
                            cl = ('A' - 0xA) + nl;
                        }
                        cs[1] = ch;
                        cs[2] = cl;
                        
                        Vec2 pp = point;
                        for (u32 j = 0; j < 3; ++j){
                            draw_font_glyph(target, face, cs[j], pp.x, pp.y, color, flags);
                            pp += delta*byte_sub_advances[j];
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
draw_string(Render_Target *target, Face *face, String_Const_u8 string, Vec2 point, u32 color){
    return(draw_string(target, face, string, point, color, 0, V2(1.f, 0.f)));
}

internal f32
draw_string(Render_Target *target, Face *face, u8 *str, Vec2 point,
            u32 color, u32 flags, Vec2 delta){
    return(draw_string(target, face, SCu8(str), point, color, flags, delta));
}

internal f32
draw_string(Render_Target *target, Face *face, u8 *str, Vec2 point,
            u32 color){
    return(draw_string(target, face, SCu8(str), point, color, 0, V2(1.f, 0.f)));
}

internal f32
font_string_width(Render_Target *target, Face *face, String_Const_u8 str){
    return(draw_string(target, face, str, V2(0, 0), 0, 0, V2(0, 0)));
}

internal f32
font_string_width(Render_Target *target, Face *face, u8 *str){
    return(draw_string(target, face, SCu8(str), V2(0, 0), 0, 0, V2(0, 0)));
}

// BOTTOM

