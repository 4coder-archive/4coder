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
draw_rectangle_sharp(Render_Target *target, Rect_f32 rect, u32 color){
    Render_Vertex vertices[6] = {};
    vertices[0].xy = V2(rect.x0, rect.y0);
    vertices[1].xy = V2(rect.x1, rect.y0);
    vertices[2].xy = V2(rect.x0, rect.y1);
    vertices[3].xy = V2(rect.x1, rect.y0);
    vertices[4].xy = V2(rect.x0, rect.y1);
    vertices[5].xy = V2(rect.x1, rect.y1);
	Vec4_f32 c = unpack_color4(color);
    for (i32 i = 0; i < 6; i += 1){
        vertices[i].color = c;
    }
    draw__write_vertices_in_current_group(target, vertices, 6);
}

global b32 filled_round_corner = false;
global_const i32 baked_tri_count = 12;
global Vec2_f32 baked_round_corner[baked_tri_count + 1] = {};

internal void
bake_round_corner(void){
    if (!filled_round_corner){
        filled_round_corner = true;
        Range_f32 theta = If32(0.f, half_pi_f32);
        for (i32 k = 0; k < baked_tri_count + 1; k += 1){
            f32 t = ((f32)k)/((f32)baked_tri_count);
            f32 theta1 = lerp(t, theta);
            baked_round_corner[k] = V2f32(cos_f32(theta1), sin_f32(theta1));
        }
    }
}

internal void
draw_round_corners(Render_Target *target, Rect_f32 mid, f32 roundness, u32 color){
    bake_round_corner();
    
    for (i32 i = 0; i < 2; i += 1){
        for (i32 j = 0; j < 2; j += 1){
            Vec2_f32 d = {};
            Vec2_f32 p = {};
            if (i == 0){
                if (j == 0){
                    p = V2f32(mid.x0, mid.y0);
                    d = V2f32(-1.f, -1.f);
                }
                else{
                    p = V2f32(mid.x1, mid.y0);
                    d = V2f32( 1.f, -1.f);
                }
            }
            else{
                if (j == 0){
                    p = V2f32(mid.x0, mid.y1);
                    d = V2f32(-1.f,  1.f);
                }
                else{
                    p = V2f32(mid.x1, mid.y1);
                    d = V2f32( 1.f,  1.f);
                }
            }
            d *= roundness;
            
            Render_Vertex vertices[baked_tri_count*3] = {};
            i32 m = 0;
            for (i32 k = 0; k < baked_tri_count; k += 1){
                Vec2_f32 p0 = baked_round_corner[k + 0];
                Vec2_f32 p1 = baked_round_corner[k + 1];
                vertices[m].xy = p;
                m += 1;
                vertices[m].xy = p + hadamard(p0, d);
                m += 1;
                vertices[m].xy = p + hadamard(p1, d);
                m += 1;
            }
			Vec4_f32 c = unpack_color4(color);
            for (i32 k = 0; k < ArrayCount(vertices); k += 1){
                vertices[k].color = c;
            }
            draw__write_vertices_in_current_group(target, vertices, ArrayCount(vertices));
        }   
    }
}

internal void
draw_rectangle(Render_Target *target, Rect_f32 rect, f32 roundness, u32 color){
    if (roundness < epsilon_f32){
        draw_rectangle_sharp(target, rect, color);
    }
    else{
        Vec2_f32 dim = rect_dim(rect);
        Vec2_f32 half_dim = dim*0.5f;
        roundness = min(roundness, half_dim.x);
        roundness = min(roundness, half_dim.y);
        
        Rect_f32 mid = rect_inner(rect, roundness);
        b32 has_x = (mid.x1 > mid.x0);
        b32 has_y = (mid.y1 > mid.y0);
        
        if (has_x && has_y){
            draw_rectangle_sharp(target, mid, color);
        }
        if (has_x){
            draw_rectangle_sharp(target,
                                 Rf32(mid.x0, rect.y0, mid.x1, mid.y0),
                                 color);
            draw_rectangle_sharp(target,
                                 Rf32(mid.x0, mid.y0, mid.x1, rect.y1),
                                 color);
        }
        if (has_y){
            draw_rectangle_sharp(target,
                                 Rf32(rect.x0, mid.y0, mid.x0, mid.y1),
                                 color);
            draw_rectangle_sharp(target,
                                 Rf32(mid.x1, mid.y0, rect.x1, mid.y1),
                                 color);
        }
        
        draw_round_corners(target, mid, roundness, color);
    }
}

internal void
draw_rectangle_outline(Render_Target *target, Rect_f32 rect, f32 roundness, f32 thickness, u32 color){
    Vec2_f32 dim = rect_dim(rect);
    Vec2_f32 half_dim = dim*0.5f;
    f32 max_thickness = min(half_dim.x, half_dim.y);
    thickness = clamp(1.f, thickness, max_thickness);
    Rect_f32 inner = rect_inner(rect, thickness);
    
    if (roundness < epsilon_f32){
        Render_Vertex vertices[24] = {};
        vertices[ 0].xy = V2(rect.x0, rect.y0);
        vertices[ 1].xy = V2(inner.x0, inner.y0);
        vertices[ 2].xy = V2(rect.x1, rect.y0);
        vertices[ 3].xy = V2(inner.x0, inner.y0);
        vertices[ 4].xy = V2(rect.x1, rect.y0);
        vertices[ 5].xy = V2(inner.x1, inner.y0);
        vertices[ 6].xy = V2(rect.x1, rect.y0);
        vertices[ 7].xy = V2(inner.x1, inner.y0);
        vertices[ 8].xy = V2(rect.x1, rect.y1);
        vertices[ 9].xy = V2(inner.x1, inner.y0);
        vertices[10].xy = V2(rect.x1, rect.y1);
        vertices[11].xy = V2(inner.x1, inner.y1);
        vertices[12].xy = V2(rect.x1, rect.y1);
        vertices[13].xy = V2(inner.x1, inner.y1);
        vertices[14].xy = V2(rect.x0, rect.y1);
        vertices[15].xy = V2(inner.x1, inner.y1);
        vertices[16].xy = V2(rect.x0, rect.y1);
        vertices[17].xy = V2(inner.x0, inner.y1);
        vertices[18].xy = V2(rect.x0, rect.y1);
        vertices[19].xy = V2(inner.x0, inner.y1);
        vertices[20].xy = V2(rect.x0, rect.y0);
        vertices[21].xy = V2(inner.x0, inner.y1);
        vertices[22].xy = V2(rect.x0, rect.y0);
        vertices[23].xy = V2(inner.x0, inner.y0);
		Vec4_f32 c = unpack_color4(color);
        for (i32 i = 0; i < ArrayCount(vertices); i += 1){
            vertices[i].color = c;
        }
        draw__write_vertices_in_current_group(target, vertices, ArrayCount(vertices));
    }
    else{
        roundness = min(roundness, half_dim.x);
        roundness = min(roundness, half_dim.y);
        
        Rect_f32 mid = rect_inner(rect, roundness);
        b32 has_x = (mid.x1 > mid.x0);
        b32 has_y = (mid.y1 > mid.y0);
        
        if (has_x){
            draw_rectangle_sharp(target,
                                 Rf32(mid.x0, rect.y0, mid.x1, inner.y0),
                                 color);
            draw_rectangle_sharp(target,
                                 Rf32(mid.x0, inner.y1, mid.x1, rect.y1),
                                 color);
        }
        if (has_y){
            draw_rectangle_sharp(target,
                                 Rf32(rect.x0, mid.y0, inner.x0, mid.y1),
                                 color);
            draw_rectangle_sharp(target,
                                 Rf32(inner.x1, mid.y0, rect.x1, mid.y1),
                                 color);
        }
        
        if (roundness <= thickness){
            draw_round_corners(target, mid, roundness, color);
        }
        else{
            bake_round_corner();
            
            for (i32 i = 0; i < 2; i += 1){
                for (i32 j = 0; j < 2; j += 1){
                    Vec2_f32 d = {};
                    Vec2_f32 p = {};
                    if (i == 0){
                        if (j == 0){
                            p = V2f32(mid.x0, mid.y0);
                            d = V2f32(-1.f, -1.f);
                        }
                        else{
                            p = V2f32(mid.x1, mid.y0);
                            d = V2f32( 1.f, -1.f);
                        }
                    }
                    else{
                        if (j == 0){
                            p = V2f32(mid.x0, mid.y1);
                            d = V2f32(-1.f,  1.f);
                        }
                        else{
                            p = V2f32(mid.x1, mid.y1);
                            d = V2f32( 1.f,  1.f);
                        }
                    }
                    Vec2_f32 d1 = d*roundness;
                    Vec2_f32 d2 = d*(roundness - thickness);
                    
                    Render_Vertex vertices[baked_tri_count*6] = {};
                    i32 m = 0;
                    for (i32 k = 0; k < baked_tri_count; k += 1){
                        Vec2_f32 p0 = baked_round_corner[k + 0];
                        Vec2_f32 p1 = baked_round_corner[k + 1];
                        vertices[m].xy = p + hadamard(p0, d1);
                        m += 1;
                        vertices[m].xy = p + hadamard(p0, d2);
                        m += 1;
                        vertices[m].xy = p + hadamard(p1, d1);
                        m += 1;
                        vertices[m].xy = p + hadamard(p0, d2);
                        m += 1;
                        vertices[m].xy = p + hadamard(p1, d1);
                        m += 1;
                        vertices[m].xy = p + hadamard(p1, d2);
                        m += 1;
                    }
					Vec4_f32 c = unpack_color4(color);
                    for (i32 k = 0; k < ArrayCount(vertices); k += 1){
                        vertices[k].color = c;
                    }
                    draw__write_vertices_in_current_group(target, vertices, ArrayCount(vertices));
                }   
            }
        }
    }
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

