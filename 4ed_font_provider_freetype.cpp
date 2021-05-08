/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Freetype implementation of the font provider interface.
 *
 */

// TOP

// NOTE(allen): Thanks to insofaras.  This is copy-pasted from some work he originally did to get free type working on Linux.

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

internal u32
ft__load_flags(b32 use_hinting){
    u32 ft_flags = FT_LOAD_RENDER;
    if (use_hinting){
        // NOTE(inso): FT_LOAD_TARGET_LIGHT does hinting only vertically, which looks nicer imo
        // maybe it could be exposed as an option for hinting, instead of just on/off.
        ft_flags |= (FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
    }
    else{
        ft_flags |= (FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_HINTING);
    }
    return(ft_flags);
}

internal FT_Codepoint_Index_Pair_Array
ft__get_codepoint_index_pairs(Arena *arena, FT_Face face, u16 *maximum_index_out){
    FT_Long glyph_count = face->num_glyphs;
    
    FT_Codepoint_Index_Pair_Array array = {};
    array.count = glyph_count;
    array.vals = push_array(arena, FT_Codepoint_Index_Pair, glyph_count);
    
    u16 maximum_index = 0;
    
    i32 counter = 0;
    FT_UInt index = 0;
    FT_ULong codepoint = FT_Get_First_Char(face, &index);
    array.vals[counter].codepoint = codepoint;
    array.vals[counter].index = (u16)index;
    maximum_index = Max(maximum_index, (u16)index);
    counter += 1;
    for (;;){
        codepoint = FT_Get_Next_Char(face, codepoint, &index);
        array.vals[counter].codepoint = codepoint;
        array.vals[counter].index = (u16)index;
        maximum_index = Max(maximum_index, (u16)index);
        counter += 1;
        if (counter == glyph_count){
            break;
        }
    }
    
    *maximum_index_out = maximum_index;
    
    return(array);
}

internal Codepoint_Index_Map
ft__get_codepoint_index_map(Base_Allocator *base_allocator, FT_Face face){
    FT_Long glyph_count = face->num_glyphs;
    
    Codepoint_Index_Map map = {};
    map.zero_index = max_u16;
    map.table = make_table_u32_u16(base_allocator, glyph_count*4);
    
    u16 maximum_index = 0;
    
    i32 counter = 0;
    FT_UInt index = 0;
    FT_ULong codepoint = FT_Get_First_Char(face, &index);
    table_insert(&map.table, (u32)codepoint, (u16)index);
    maximum_index = Max(maximum_index, (u16)index);
    counter += 1;
    for (;;){
        codepoint = FT_Get_Next_Char(face, codepoint, &index);
        if (codepoint == 0){
            map.has_zero_index = true;
            map.zero_index = (u16)(index);
        }
        else{
            table_insert(&map.table, (u32)codepoint, (u16)index);
        }
        maximum_index = Max(maximum_index, (u16)index);
        counter += 1;
        if (counter == glyph_count){
            break;
        }
    }
    
    map.max_index = maximum_index;
    
    return(map);
}

struct Bad_Rect_Pack{
    Vec2_i32 max_dim;
    Vec3_i32 dim;
    Vec3_i32 p;
    i32 current_line_h;
};

internal void
ft__bad_rect_pack_init(Bad_Rect_Pack *pack, Vec2_i32 max_dim){
    pack->max_dim = max_dim;
    pack->dim = V3i32(0, 0, 1);
    pack->p = V3i32(0, 0, 0);
    pack->current_line_h = 0;
}

internal void
ft__bad_rect_pack_end_line(Bad_Rect_Pack *pack){
    pack->p.y += pack->current_line_h;
    pack->dim.y = Max(pack->dim.y, pack->p.y);
    pack->current_line_h = 0;
    pack->p.x = 0;
}

internal Vec3_i32
ft__bad_rect_pack_next(Bad_Rect_Pack *pack, Vec2_i32 dim){
    Vec3_i32 result = {};
    if (dim.x <= pack->max_dim.x && dim.y <= pack->max_dim.y){
        if (pack->current_line_h < dim.y){
            pack->current_line_h = dim.y;
        }
        if (pack->current_line_h > pack->max_dim.y){
            ft__bad_rect_pack_end_line(pack);
            pack->p.y = 0;
            pack->dim.z += 1;
            pack->p.z += 1;
        }
        else{
            if (pack->p.x + dim.x > pack->max_dim.x){
                ft__bad_rect_pack_end_line(pack);
            }
            result = pack->p;
            pack->p.x += dim.x;
            pack->current_line_h = Max(pack->current_line_h, dim.y);
            pack->dim.x = clamp_bot(pack->dim.x, pack->p.x);
        }
    }
    return(result);
}

internal void
ft__bad_rect_store_finish(Bad_Rect_Pack *pack){
    ft__bad_rect_pack_end_line(pack);
}

internal void
ft__glyph_bounds_store_uv_raw(Vec3_i32 p, Vec2_i32 dim, Glyph_Bounds *bounds){
    bounds->uv = Rf32((f32)p.x, (f32)p.y, (f32)dim.x, (f32)dim.y);
    bounds->w = (f32)p.z;
}

internal Face*
ft__font_make_face(Arena *arena, Face_Description *description, f32 scale_factor){
    String_Const_u8 file_name = push_string_copy(arena, description->font.file_name);
    
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face ft_face;
    FT_Error error = FT_New_Face(ft, (char*)file_name.str, 0, &ft_face);
    
    Face *face = 0;
    if (error == 0){
        face = push_array_zero(arena, Face, 1);
        
        u32 pt_size_unscaled = Max(description->parameters.pt_size, 8); 
        u32 pt_size = (u32)(pt_size_unscaled*scale_factor);
        b32 hinting = description->parameters.hinting;
        
        FT_Size_RequestRec_ size = {};
        size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
        size.height = (pt_size << 6);
        FT_Request_Size(ft_face, &size);
        
        face->description.font.file_name = file_name;
        face->description.parameters = description->parameters;
        
        Face_Metrics *met = &face->metrics;
        
        met->max_advance = f32_ceil32(ft_face->size->metrics.max_advance/64.f);
        met->ascent      = f32_ceil32(ft_face->size->metrics.ascender/64.f);
        met->descent     = f32_floor32(ft_face->size->metrics.descender/64.f);
        met->text_height = f32_ceil32(ft_face->size->metrics.height/64.f);
        met->line_skip   = met->text_height - (met->ascent - met->descent);
        met->line_skip   = clamp_bot(1.f, met->line_skip);
        met->line_height = met->text_height + met->line_skip;
        
        {
            f32 real_over_notional = met->line_height/(f32)ft_face->height;
            f32 relative_center = -1.f*real_over_notional*ft_face->underline_position;
            f32 relative_thickness = real_over_notional*ft_face->underline_thickness;
            
            f32 center    = f32_floor32(met->ascent + relative_center);
            f32 thickness = clamp_bot(1.f, relative_thickness);
            
            met->underline_yoff1 = center - thickness*0.5f;
            met->underline_yoff2 = center + thickness*0.5f;
        }
        
        face->advance_map.codepoint_to_index =
            ft__get_codepoint_index_map(arena->base_allocator, ft_face);
        u16 index_count =
            codepoint_index_map_count(&face->advance_map.codepoint_to_index);
        face->advance_map.index_count = index_count;
        face->advance_map.advance = push_array_zero(arena, f32, index_count);
        face->bounds = push_array(arena, Glyph_Bounds, index_count);
        
        Temp_Memory_Block temp_memory(arena);
        struct Bitmap{
            Vec2_i32 dim;
            u8 *data;
        };
        Bitmap *glyph_bitmaps = push_array(arena, Bitmap, index_count);
        
        u32 load_flags = ft__load_flags(hinting);
        for (u16 i = 0; i < index_count; i += 1){
            Bitmap *bitmap = &glyph_bitmaps[i];
            
            error = FT_Load_Glyph(ft_face, i, load_flags);
            if (error == 0){
                FT_GlyphSlot ft_glyph = ft_face->glyph;
                Vec2_i32 dim = V2i32(ft_glyph->bitmap.width, ft_glyph->bitmap.rows);
                bitmap->dim = dim;
                bitmap->data = push_array(arena, u8, dim.x*dim.y);
                
                face->bounds[i].xy_off.x0 = (f32)(ft_face->glyph->bitmap_left);
                face->bounds[i].xy_off.y0 = (f32)(met->ascent - ft_face->glyph->bitmap_top);
                face->bounds[i].xy_off.x1 = (f32)(face->bounds[i].xy_off.x0 + dim.x);
                face->bounds[i].xy_off.y1 = (f32)(face->bounds[i].xy_off.y0 + dim.y);
                
                switch (ft_glyph->bitmap.pixel_mode){
                    case FT_PIXEL_MODE_MONO:
                    {
                        NotImplemented;
                    }break;
                    
                    case FT_PIXEL_MODE_GRAY:
                    {
                        b32 aa_1bit_mono = (description->parameters.aa_mode == FaceAntialiasingMode_1BitMono);
                        
                        u8 *src_line = ft_glyph->bitmap.buffer;
                        if (ft_glyph->bitmap.pitch < 0){
                            src_line = ft_glyph->bitmap.buffer + (-ft_glyph->bitmap.pitch)*(dim.y - 1);
                        }
                        u8 *dst = bitmap->data;
                        for (i32 y = 0; y < dim.y; y += 1){
                            u8 *src_pixel = src_line;
                            for (i32 x = 0; x < dim.x; x += 1){
                                if (aa_1bit_mono){
                                    u8 s = *src_pixel;
                                    if (s > 0){
                                        s = 255;
                                    }
                                    *dst = s;
                                }
                                else{
                                    *dst = *src_pixel;
                                }
                                dst += 1;
                                src_pixel += 1;
                            }
                            src_line += ft_glyph->bitmap.pitch;
                        }
                    }break;
                    
                    default:
                    {
                        NotImplemented;
                    }break;
                }
                
                face->advance_map.advance[i] = f32_ceil32(ft_glyph->advance.x/64.0f);
            }
        }
        
        u8 white_data[16] = {
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
        };
        
        Bitmap white = {};
        white.dim = V2i32(4, 4);
        white.data = white_data;
        
        Bad_Rect_Pack pack = {};
        ft__bad_rect_pack_init(&pack, V2i32(1024, 1024));
        ft__glyph_bounds_store_uv_raw(ft__bad_rect_pack_next(&pack, white.dim), white.dim, &face->white);
        for (u16 i = 0; i < index_count; i += 1){
            Vec2_i32 dim = glyph_bitmaps[i].dim;
            ft__glyph_bounds_store_uv_raw(ft__bad_rect_pack_next(&pack, dim), dim, &face->bounds[i]);
        }
        ft__bad_rect_store_finish(&pack);
        
        Texture_Kind texture_kind = TextureKind_Mono;
        u32 texture = graphics_get_texture(pack.dim, texture_kind);
        face->texture_kind = texture_kind;
        face->texture = texture;
        
        Vec3_f32 texture_dim = V3f32(pack.dim);
        face->texture_dim = texture_dim;
        
        {
            Vec3_i32 p = V3i32((i32)face->white.uv.x0, (i32)face->white.uv.y0, (i32)face->white.w);
            Vec3_i32 dim = V3i32(white.dim.x, white.dim.y, 1);
            graphics_fill_texture(texture_kind, texture, p, dim, white.data);
            face->white.uv.x1 = (face->white.uv.x0 + face->white.uv.x1)/texture_dim.x;
            face->white.uv.y1 = (face->white.uv.y0 + face->white.uv.y1)/texture_dim.y;
            face->white.uv.x0 =  face->white.uv.x0/texture_dim.x;
            face->white.uv.y0 =  face->white.uv.y0/texture_dim.y;
            face->white.w /= texture_dim.z;
        }
        
        for (u16 i = 0; i < index_count; i += 1){
            Vec3_i32 p = V3i32((i32)face->bounds[i].uv.x0, (i32)face->bounds[i].uv.y0, (i32)face->bounds[i].w);
            Vec3_i32 dim = V3i32(glyph_bitmaps[i].dim.x, glyph_bitmaps[i].dim.y, 1);
            graphics_fill_texture(texture_kind, texture, p, dim, glyph_bitmaps[i].data);
            face->bounds[i].uv.x1 = (face->bounds[i].uv.x0 + face->bounds[i].uv.x1)/texture_dim.x;
            face->bounds[i].uv.y1 = (face->bounds[i].uv.y0 + face->bounds[i].uv.y1)/texture_dim.y;
            face->bounds[i].uv.x0 =  face->bounds[i].uv.x0/texture_dim.x;
            face->bounds[i].uv.y0 =  face->bounds[i].uv.y0/texture_dim.y;
            face->bounds[i].w /= texture_dim.z;
        }
        
        {
            Face_Advance_Map *advance_map = &face->advance_map;
            
            met->space_advance = font_get_glyph_advance(advance_map, met, ' ', 0);
            met->decimal_digit_advance =
                font_get_max_glyph_advance_range(advance_map, met, '0', '9', 0);
            met->hex_digit_advance =
                font_get_max_glyph_advance_range(advance_map, met, 'A', 'F', 0);
            met->hex_digit_advance =
                Max(met->hex_digit_advance, met->decimal_digit_advance);
            met->byte_sub_advances[0] =
                font_get_glyph_advance(advance_map, met, '\\', 0);
            met->byte_sub_advances[1] = met->hex_digit_advance;
            met->byte_sub_advances[2] = met->hex_digit_advance;
            met->byte_advance =
                met->byte_sub_advances[0] +
                met->byte_sub_advances[1] +
                met->byte_sub_advances[2];
            met->normal_lowercase_advance =
                font_get_average_glyph_advance_range(advance_map, met, 'a', 'z', 0);
            met->normal_uppercase_advance =
                font_get_average_glyph_advance_range(advance_map, met, 'A', 'Z', 0);
            met->normal_advance = (26*met->normal_lowercase_advance +
                                   26*met->normal_uppercase_advance +
                                   10*met->decimal_digit_advance)/62.f;
        }
    }
    
    FT_Done_FreeType(ft);
    
    return(face);
}

// BOTTOM

