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

internal Rect_f32
draw_set_clip(Render_Target *target, Rect_f32 clip_box){
    Rect_f32 result = target->current_clip_box;
    if (target->current_clip_box != clip_box){
        target->current_clip_box = clip_box;
        draw__begin_new_group(target);
    }
    return(result);
}

internal void
begin_frame(Render_Target *target, void *font_set){
    linalloc_clear(&target->arena);
    target->group_first = 0;
    target->group_last = 0;
    target->current_face_id = 0;
    target->current_clip_box = Rf32(0, 0, (f32)target->width, (f32)target->height);
    target->font_set = font_set;
}

internal void
begin_render_section(Render_Target *target, i32 frame_index, f32 literal_dt, f32 animation_dt){
    target->frame_index = frame_index;
    target->literal_dt = literal_dt;
    target->animation_dt = animation_dt;
}

internal void
end_render_section(Render_Target *target){
}

////////////////////////////////

internal void
draw_rectangle_outline(Render_Target *target, Rect_f32 rect, f32 roundness, f32 thickness, u32 color){
    if (roundness < epsilon_f32){
        roundness = 0.f;
    }
    thickness = clamp_bot(1.f, thickness);
    f32 half_thickness = thickness*0.5f;
    
    Render_Vertex vertices[6] = {};
    vertices[0].xy = V2f32(rect.x0, rect.y0);
    vertices[1].xy = V2f32(rect.x1, rect.y0);
    vertices[2].xy = V2f32(rect.x0, rect.y1);
    vertices[3].xy = V2f32(rect.x1, rect.y0);
    vertices[4].xy = V2f32(rect.x0, rect.y1);
    vertices[5].xy = V2f32(rect.x1, rect.y1);
    
    Vec2_f32 center = rect_center(rect);
    for (i32 i = 0; i < ArrayCount(vertices); i += 1){
        vertices[i].uvw = V3f32(center.x, center.y, roundness);
        vertices[i].color = color;
        vertices[i].half_thickness = half_thickness;
    }
    draw__write_vertices_in_current_group(target, vertices, ArrayCount(vertices));
}

internal void
draw_rectangle(Render_Target *target, Rect_f32 rect, f32 roundness, u32 color){
    Vec2_f32 dim = rect_dim(rect);
    draw_rectangle_outline(target, rect, roundness, Max(dim.x, dim.y), color);
}

internal void
draw_font_glyph(Render_Target *target, Face *face, u32 codepoint, Vec2_f32 p,
                ARGB_Color color, Glyph_Flag flags, Vec2_f32 x_axis){
    draw__set_face_id(target, face->id);
    
    u16 glyph_index = 0;
    if (!codepoint_index_map_read(&face->advance_map.codepoint_to_index,
                                  codepoint, &glyph_index)){
        glyph_index = 0;
    }
    Glyph_Bounds bounds = face->bounds[glyph_index];
    Vec3_f32 texture_dim = face->texture_dim;
    
    Render_Vertex vertices[6] = {};
    
    Rect_f32 uv = bounds.uv;
    vertices[0].uvw = V3f32(uv.x0, uv.y0, bounds.w);
    vertices[1].uvw = V3f32(uv.x1, uv.y0, bounds.w);
    vertices[2].uvw = V3f32(uv.x0, uv.y1, bounds.w);
    vertices[5].uvw = V3f32(uv.x1, uv.y1, bounds.w);
    
    Vec2_f32 y_axis = V2f32(-x_axis.y, x_axis.x);
    Vec2_f32 x_min = bounds.xy_off.x0*x_axis;
    Vec2_f32 x_max = bounds.xy_off.x1*x_axis;
    Vec2_f32 y_min = bounds.xy_off.y0*y_axis;
    Vec2_f32 y_max = bounds.xy_off.y1*y_axis;
    Vec2_f32 p_x_min = p + x_min;
    Vec2_f32 p_x_max = p + x_max;
    vertices[0].xy = p_x_min + y_min;
    vertices[1].xy = p_x_max + y_min;
    vertices[2].xy = p_x_min + y_max;
    vertices[5].xy = p_x_max + y_max;
    
#if 0    
    Vec2_f32 xy_min = p + bounds.xy_off.x0*x_axis + bounds.xy_off.y0*y_axis;
    Vec2_f32 xy_max = p + bounds.xy_off.x1*x_axis + bounds.xy_off.y1*y_axis;
    
    vertices[0].xy = V2f32(xy_min.x, xy_min.y);
    vertices[1].xy = V2f32(xy_max.x, xy_min.y);
    vertices[2].xy = V2f32(xy_min.x, xy_max.y);
    vertices[5].xy = V2f32(xy_max.x, xy_max.y);
#endif
    
#if 0    
    if (!HasFlag(flags, GlyphFlag_Rotate90)){
        Rect_f32 xy = Rf32(p + bounds.xy_off.p0, p + bounds.xy_off.p1);
        
        vertices[0].xy  = V2f32(xy.x0, xy.y1);
        vertices[0].uvw = V3f32(uv.x0, uv.y1, bounds.w);
        vertices[1].xy  = V2f32(xy.x1, xy.y1);
        vertices[1].uvw = V3f32(uv.x1, uv.y1, bounds.w);
        vertices[2].xy  = V2f32(xy.x0, xy.y0);
        vertices[2].uvw = V3f32(uv.x0, uv.y0, bounds.w);
        vertices[5].xy  = V2f32(xy.x1, xy.y0);
        vertices[5].uvw = V3f32(uv.x1, uv.y0, bounds.w);
    }
    else{
        Rect_f32 xy = Rf32(p.x - bounds.xy_off.y1, p.y + bounds.xy_off.x0,
                           p.x - bounds.xy_off.y0, p.y + bounds.xy_off.x1);
        
        vertices[0].xy  = V2f32(xy.x0, xy.y1);
        vertices[0].uvw = V3f32(uv.x1, uv.y1, bounds.w);
        vertices[1].xy  = V2f32(xy.x1, xy.y1);
        vertices[1].uvw = V3f32(uv.x1, uv.y0, bounds.w);
        vertices[2].xy  = V2f32(xy.x0, xy.y0);
        vertices[2].uvw = V3f32(uv.x0, uv.y1, bounds.w);
        vertices[5].xy  = V2f32(xy.x1, xy.y0);
        vertices[5].uvw = V3f32(uv.x0, uv.y0, bounds.w);
    }
#endif
    
    vertices[3] = vertices[1];
    vertices[4] = vertices[2];
    
    for (i32 i = 0; i < ArrayCount(vertices); i += 1){
        vertices[i].color = color;
        vertices[i].half_thickness = 0.f;
    }
    
    draw__write_vertices_in_current_group(target, vertices, ArrayCount(vertices));
}

////////////////////////////////

internal Vec2_f32
floor32(Vec2_f32 point){
    point.x = f32_floor32(point.x);
    point.y = f32_floor32(point.y);
    return(point);
}

internal f32
draw_string(Render_Target *target, Face *face, String_Const_u8 string, Vec2_f32 point,
            ARGB_Color color, u32 flags, Vec2_f32 delta){
    f32 total_delta = 0.f;
    if (face != 0){
        point = floor32(point);
        
        f32 byte_advance = face->metrics.byte_advance;
        f32 *byte_sub_advances = face->metrics.byte_sub_advances;
        
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
                        u32 draw_codepoint = step.value;
                        if (draw_codepoint == '\t'){
                            draw_codepoint = ' ';
                        }
                        draw_font_glyph(target, face, draw_codepoint, point, color, flags, delta);
                    }
                    local_const f32 internal_tab_width = 4.f;
                    f32 d = font_get_glyph_advance(&face->advance_map, &face->metrics, codepoint, internal_tab_width);
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
                        
                        Vec2_f32 pp = point;
                        for (u32 j = 0; j < 3; ++j){
                            draw_font_glyph(target, face, cs[j], pp, color, flags, delta);
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
draw_string(Render_Target *target, Face *face, String_Const_u8 string, Vec2_f32 point, u32 color){
    return(draw_string(target, face, string, point, color, 0, V2f32(1.f, 0.f)));
}

internal f32
draw_string(Render_Target *target, Face *face, u8 *str, Vec2_f32 point, u32 color, u32 flags, Vec2_f32 delta){
    return(draw_string(target, face, SCu8(str), point, color, flags, delta));
}

internal f32
draw_string(Render_Target *target, Face *face, u8 *str, Vec2_f32 point, u32 color){
    return(draw_string(target, face, SCu8(str), point, color, 0, V2f32(1.f, 0.f)));
}

internal f32
font_string_width(Render_Target *target, Face *face, String_Const_u8 str){
    return(draw_string(target, face, str, V2f32(0, 0), 0, 0, V2f32(0, 0)));
}

internal f32
font_string_width(Render_Target *target, Face *face, u8 *str){
    return(draw_string(target, face, SCu8(str), V2f32(0, 0), 0, 0, V2f32(0, 0)));
}

// BOTTOM

