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

internal Table_u32_u16
ft__get_codepoint_index_table(Base_Allocator *base_allocator, FT_Face face, u16 *maximum_index_out){
    FT_Long glyph_count = face->num_glyphs;
    
    Table_u32_u16 table = make_table_u32_u16(base_allocator, glyph_count*4);
    
    u16 maximum_index = 0;
    
    i32 counter = 0;
    FT_UInt index = 0;
    FT_ULong codepoint = FT_Get_First_Char(face, &index);
    table_insert(&table, (u32)codepoint, (u16)index);
    maximum_index = Max(maximum_index, (u16)index);
    counter += 1;
    for (;;){
        codepoint = FT_Get_Next_Char(face, codepoint, &index);
        table_insert(&table, (u32)codepoint, (u16)index);
        maximum_index = Max(maximum_index, (u16)index);
        counter += 1;
        if (counter == glyph_count){
            break;
        }
    }
    
    *maximum_index_out = maximum_index;
    
    return(table);
}

struct FT_Bad_Rect_Pack{
    Vec2_i32 max_dim;
    Vec3_i32 dim;
    Vec3_i32 p;
    i32 current_line_h;
};

internal void
ft__bad_rect_pack_init(FT_Bad_Rect_Pack *pack, Vec2_i32 max_dim){
    pack->max_dim = max_dim;
    pack->dim = V3i32(0, 0, 0);
    pack->p = V3i32(0, 0, 0);
    pack->current_line_h = 0;
}

internal void
ft__bad_rect_pack_end_line(FT_Bad_Rect_Pack *pack){
    pack->p.y += pack->current_line_h;
    pack->dim.y = Max(pack->dim.y, pack->p.y);
    pack->current_line_h = 0;
    pack->p.x = 0;
}

internal Vec3_i32
ft__bad_rect_pack_next(FT_Bad_Rect_Pack *pack, Vec2_i32 dim){
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
        }
    }
    return(result);
}

internal void
ft__glyph_bounds_store_uv_raw(Vec3_i32 p, Vec2_i32 dim, Glyph_Bounds *bounds){
    bounds->uv = Rf32((f32)p.x, (f32)p.y, (f32)dim.x, (f32)dim.y);
    bounds->w = (f32)p.z;
}

enum{
    TextureKind_Error,
    TextureKind_Mono,
};

internal Face*
ft__font_make_face(Arena *arena, Face_Settings *settings, Get_GPU_Texture_Function *get_gpu_texture, Fill_GPU_Texture_Function *fill_gpu_texture){
    String_Const_u8 file_name = SCu8(settings->stub.name, settings->stub.len);
    
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face ft_face;
    FT_Error error = FT_New_Face(ft, (char*)file_name.str, 0, &ft_face);
    
    Face *face = push_array_zero(arena, Face, 1);;
    if (error == 0){
        u32 pt_size = settings->parameters.pt_size;
        b32 use_hinting = settings->parameters.use_hinting;
        
        FT_Size_RequestRec_ size = {};
        size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
        size.height = (pt_size << 6);
        FT_Request_Size(ft_face, &size);
        
        face->name = push_string_copy(arena, file_name);
        
        face->ascent    = f32_ceil32(ft_face->size->metrics.ascender/64.f);
        face->descent   = f32_floor32(ft_face->size->metrics.descender/64.f);
        face->advance   = f32_ceil32(ft_face->size->metrics.max_advance/64.f);
        face->height    = f32_ceil32(ft_face->size->metrics.height/64.f);
        face->line_skip = face->height - (face->ascent - face->descent);
        face->height   -= face->line_skip;
        
        {
            f32 real_over_notional = (f32)face->height/(f32)ft_face->height;
            f32 relative_center = -1.f*real_over_notional*ft_face->underline_position;
            f32 relative_thickness = real_over_notional*ft_face->underline_thickness;
            
            f32 center    = (f32)floor32(face->ascent + relative_center);
            f32 thickness = clamp_bot(1.f, relative_thickness);
            
            face->underline_yoff1 = center - thickness*0.5f;
            face->underline_yoff2 = center + thickness*0.5f;
        }
        
        u16 index_count = 0;
        face->codepoint_to_index_table = ft__get_codepoint_index_table(arena->base_allocator, ft_face, &index_count);
        face->bounds = push_array(arena, Glyph_Bounds, index_count);
        
        struct FT_Bitmap{
            Vec2_i32 dim;
            u8 *data;
        };
        FT_Bitmap *glyph_bitmaps = push_array(arena, FT_Bitmap, index_count);
        
        u32 load_flags = ft__load_flags(use_hinting);
        for (u16 i = 0; i < index_count; i += 1){
            
            FT_Bitmap *bitmap = &glyph_bitmaps[i];
            
            error = FT_Load_Glyph(ft_face, i, load_flags);
            if (error == 0){
                FT_GlyphSlot ft_glyph = ft_face->glyph;
                Vec2_i32 dim = V2i32(ft_glyph->bitmap.width, ft_glyph->bitmap.rows);
                bitmap->dim = dim;
                bitmap->data = push_array(arena, u8, dim.x*dim.y);
                
                face->bounds[i].xy_off.x0 = (f32)(ft_face->glyph->bitmap_left);
                face->bounds[i].xy_off.y0 = (f32)(face->ascent - ft_face->glyph->bitmap_top);
                face->bounds[i].xy_off.x1 = (f32)(face->bounds[i].xy_off.x0 + dim.x);
                face->bounds[i].xy_off.y1 = (f32)(face->bounds[i].xy_off.y0 + dim.y);
                
                switch (ft_glyph->bitmap.pixel_mode){
                    case FT_PIXEL_MODE_MONO:
                    {
                        NotImplemented;
                    }break;
                    
                    case FT_PIXEL_MODE_GRAY:
                    {
                        u8 *src_line = ft_glyph->bitmap.buffer;
                        if (ft_glyph->bitmap.pitch < 0){
                            src_line = ft_glyph->bitmap.buffer + (-ft_glyph->bitmap.pitch)*(dim.y - 1);
                        }
                        u8 *dst = bitmap->data;
                        for (i32 y = 0; y < dim.y; y += 1){
                            u8 *src_pixel = src_line;
                            for (i32 x = 0; x < dim.x; x += 1){
                                *dst = *src_pixel;
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
            }
        }
        
        u8 white_data[16] = {
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
        };
        
        FT_Bitmap white = {};
        white.dim = V2i32(4, 4);
        white.data = white_data;
        
        FT_Bad_Rect_Pack pack = {};
        ft__bad_rect_pack_init(&pack, V2i32(1024, 1024));
        ft__glyph_bounds_store_uv_raw(ft__bad_rect_pack_next(&pack, white.dim), white.dim, &face->white);
        for (u16 i = 0; i < index_count; i += 1){
            Vec2_i32 dim = glyph_bitmaps[i].dim;
            ft__glyph_bounds_store_uv_raw(ft__bad_rect_pack_next(&pack, dim), dim, &face->bounds[i]);
        }
        
        Texture_Kind texture_kind = TextureKind_Mono;
        u32 gpu_texture = get_gpu_texture(pack.dim, texture_kind);
        face->gpu_texture_kind = texture_kind;
        face->gpu_texture = gpu_texture;
        
        Vec3_f32 gpu_texture_dim = V3f32(pack.dim);
        face->gpu_texture_dim = gpu_texture_dim;
        for (u16 i = 0; i < index_count; i += 1){
            Vec3_i32 p = V3i32((i32)face->bounds[i].uv.x0, (i32)face->bounds[i].uv.y0, (i32)face->bounds[i].w);
            Vec3_i32 dim = V3i32(glyph_bitmaps[i].dim.x, glyph_bitmaps[i].dim.y, 1);
            fill_gpu_texture(texture_kind, gpu_texture, p, dim, glyph_bitmaps[i].data);
            
            face->bounds[i].uv.x1 = (face->bounds[i].uv.x0 + face->bounds[i].uv.x1)/gpu_texture_dim.x;
            face->bounds[i].uv.y1 = (face->bounds[i].uv.y0 + face->bounds[i].uv.y1)/gpu_texture_dim.y;
            face->bounds[i].uv.x0 =  face->bounds[i].uv.x0/gpu_texture_dim.x;
            face->bounds[i].uv.y0 =  face->bounds[i].uv.y0/gpu_texture_dim.y;
            face->bounds[i].w /= gpu_texture_dim.z;
        }
        
        face->settings = *settings;
    }
    
    FT_Done_FreeType(ft);
    
    return(face);
}

#if 0
internal b32
font_ft_get_face(FT_Library ft, Font_Loadable_Stub *stub, Font_Parameters *parameters, FT_Face *face){
    b32 success = true;
    b32 do_transform = false;
    if (stub->load_from_path){
        // TODO(allen): Look for italics/bold stuff?
        FT_Error error = FT_New_Face(ft, stub->name, 0, face);
        success = (error == 0);
        do_transform = success;
    }
    else{
        switch (system_font_method){
            case SystemFontMethod_FilePath:
            {
                Font_Path path = system_font_path(stub->name, parameters);
                if (path.len > 0){
                    FT_Error error = FT_New_Face(ft, path.name, 0, face);
                    success = (error == 0);
                    if (success){
                        success = match((*face)->family_name, stub->name);
                        do_transform = (success && path.used_base_file);
                    }
                }
                else{
                    success = false;
                }
            }break;
            
            case SystemFontMethod_RawData:
            {
                Font_Raw_Data data = system_font_data(stub->name, parameters);
                if (data.size > 0){
                    FT_Error error = FT_New_Memory_Face(ft, data.data, data.size, 0, face);
                    success = (error == 0);
                    if (success){
                        success = match((*face)->family_name, stub->name);
                        do_transform = (success && data.used_base_file);
                    }
                }
                else{
                    success = false;
                }
            }break;
        }
    }
    
    return(success);
}

internal b32
font_load_name(Font_Loadable_Stub *stub, char *buffer, i32 capacity){
    b32 success = false;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    if (font_ft_get_face(ft, stub, 0, &face)){
        char *name = face->family_name;
        i32 name_len = str_size(name);
        if (name_len < capacity){
            memcpy(buffer, name, name_len + 1);
            success = true;
        }
    }
    
    FT_Done_FreeType(ft);
    
    return(success);
}

internal b32
font_load_page_layout(Font_Settings *settings, Font_Metrics *metrics, Glyph_Page *page, u32 page_number){
    Assert(page != 0);
    memset(page, 0, sizeof(*page));
    page->page_number = page_number;
    page->has_layout = true;
    
    u32 pt_size = settings->parameters.pt_size;
    b32 use_hinting = settings->parameters.use_hinting;
    
    Temp_Memory temp = begin_temp(&shared_vars.font_scratch);
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    b32 has_a_good_face = font_ft_get_face(ft, &settings->stub, &settings->parameters, &face);
    
    if (has_a_good_face){
        FT_Size_RequestRec_ size = {};
        size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
        size.height = pt_size << 6;
        FT_Request_Size(face, &size);
        
        // NOTE(allen): Determine glyph layout dimensions
        i32 max_glyph_w = face->size->metrics.x_ppem;
        i32 max_glyph_h = metrics->height;
        i32 pen_y_descent = max_glyph_h + 2;
        i32 tex_width   = 64;
        i32 tex_height  = 0;
        
        do {
            tex_width *= 2;
            f32 glyphs_per_row = ceilf(tex_width/(f32)max_glyph_w);
            f32 rows = ceilf(GLYPHS_PER_PAGE/glyphs_per_row);
            tex_height = ceil32(rows*pen_y_descent);
        }
        while(tex_height > tex_width);
        tex_height = round_up_pot_u32(tex_height);
        
        i32 pen_x = 0;
        i32 pen_y = 0;
        
        // NOTE(allen): Fill the glyph bounds array
        u32 ft_flags = font_ft_flags(use_hinting);
        
        u32 codepoint = (page_number << 8);
        Glyph_Bounds *glyph_out = &page->glyphs[0];
        f32 *advance_out = &page->advance[0];
        for (u32 i = 0; i < GLYPHS_PER_PAGE; ++i, ++codepoint, ++glyph_out, ++advance_out){
            if (FT_Load_Char(face, codepoint, ft_flags) == 0){
                i32 w = face->glyph->bitmap.width;
                i32 h = face->glyph->bitmap.rows;
                i32 ascent = metrics->ascent;
                
                // NOTE(allen): Move to next line if necessary
                if (pen_x + w >= tex_width){
                    pen_x = 0;
                    pen_y += pen_y_descent;
                }
                
                // NOTE(allen): Set all this stuff the renderer needs
                glyph_out->x0 = (f32)(pen_x);
                glyph_out->y0 = (f32)(pen_y);
                glyph_out->x1 = (f32)(pen_x + w);
                glyph_out->y1 = (f32)(pen_y + h + 1);
                
                glyph_out->xoff = (f32)(face->glyph->bitmap_left);
                glyph_out->yoff = (f32)(ascent - face->glyph->bitmap_top);
                glyph_out->xoff2 = glyph_out->xoff + w;
                glyph_out->yoff2 = glyph_out->yoff + h + 1;
                
                // TODO(allen): maybe advance data should be integers?
                *advance_out = (f32)ceil32(face->glyph->advance.x/64.0f);
                
                pen_x = ceil32(glyph_out->x1 + 1);
            }
        }
        
        // TODO(allen): Not sure setting tex_height here is right... double check.
        tex_height = round_up_pot_u32(pen_y + pen_y_descent);
        
        page->tex_width  = tex_width;
        page->tex_height = tex_height;
    }
    else{
        page->tex_width  = 1;
        page->tex_height = 1;
    }
    
    FT_Done_FreeType(ft);
    
    end_temp(temp);
    
    return(has_a_good_face);
}

internal u32*
font_load_page_pixels(Arena *arena, Font_Settings *settings, Glyph_Page *page, u32 page_number, i32 *tex_width_out, i32 *tex_height_out){
    Assert(page != 0);
    Assert(page->has_layout);
    Assert(page->page_number == page_number);
    
    u32 pt_size = settings->parameters.pt_size;
    b32 use_hinting = settings->parameters.use_hinting;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    u32 *pixels = 0;
    
    FT_Face face;
    b32 has_a_good_face = font_ft_get_face(ft, &settings->stub, &settings->parameters, &face);
    if (has_a_good_face){
        FT_Size_RequestRec_ size = {};
        size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
        size.height = pt_size << 6;
        FT_Request_Size(face, &size);
        
        page->page_number = page_number;
        
        // NOTE(allen): Prepare a pixel buffer.
        i32 tex_width   = page->tex_width;
        i32 tex_height  = page->tex_height;
        
        pixels = push_array(arena, u32, tex_width*tex_height);
        
        if (pixels != 0){
            memset(pixels, 0, tex_width*tex_height*sizeof(u32));
            
            // NOTE(allen): Fill the texture
            u32 ft_flags = font_ft_flags(use_hinting);
            
            u32 codepoint = (page_number << 8);
            Glyph_Bounds *glyph_ptr = &page->glyphs[0];
            for (i32 i = 0; i < GLYPHS_PER_PAGE; ++i, ++codepoint, ++glyph_ptr){
                if (FT_Load_Char(face, codepoint, ft_flags) == 0){
                    // NOTE(allen): Extract this glyph's dimensions.
                    i32 x = (i32)glyph_ptr->x0;
                    i32 y = (i32)glyph_ptr->y0;
                    i32 w = (i32)(glyph_ptr->x1 - glyph_ptr->x0);
                    i32 h = (i32)(glyph_ptr->y1 - glyph_ptr->y0 - 1);
                    
                    // NOTE(allen): Write to the pixels.
                    u8 *src = face->glyph->bitmap.buffer;
                    i32 end_x = x + w;
                    i32 end_y = y + h;
                    if (src != 0){
                        i32 pitch = face->glyph->bitmap.pitch;
                        for (i32 Y = y, YY = 0; Y < end_y; ++Y, ++YY){
                            for (i32 X = x, XX = 0; X < end_x; ++X, ++XX){
                                //pixels[Y*tex_width + X] = (0x01010101*src[YY*pitch + XX]);
                                pixels[Y*tex_width + X] = 0x00FFFFFF + (0x01000000*src[YY*pitch + XX]);
                            }
                        }
                    }
                    else{
                        for (i32 Y = y, YY = 0; Y < end_y; ++Y, ++YY){
                            for (i32 X = x, XX = 0; X < end_x; ++X, ++XX){
                                pixels[Y*tex_width + X] = 0xFFFFFFFF;
                            }
                        }
                    }
                }
            }
            
            *tex_width_out = tex_width;
            *tex_height_out = tex_height;
        }
        else{
            pixels = 0;
            *tex_width_out = 1;
            *tex_height_out = 1;
        }
    }
    else{
        // TODO(allen): Fill in white boxes here.
        pixels = 0;
        *tex_width_out = 1;
        *tex_height_out = 1;
    }
    
    FT_Done_FreeType(ft);
    
    return(pixels);
}

internal void
font_release_pages(System_Functions *system, Font_Page_Storage *storage){
    u32 old_max = storage->page_max;
    Glyph_Page **old_pages = storage->pages;
    
    for (u32 i = 0; i < old_max; ++i){
        Glyph_Page *this_page = old_pages[i];
        if (this_page != FONT_PAGE_EMPTY && this_page != FONT_PAGE_DELETED){
            if (this_page->has_gpu_setup){
                Assert(sizeof(Render_Pseudo_Command_Free_Texture)%8 == 0);
                Render_Pseudo_Command_Free_Texture *c = push_array(&target.buffer, Render_Pseudo_Command_Free_Texture, 1);
                c->header.size = sizeof(*c);
                c->free_texture_node.tex_id = this_page->gpu_tex;
                sll_queue_push(target.free_texture_first, target.free_texture_last, &c->free_texture_node);
            }
            system->font.free(this_page);
        }
    }
    
    system->font.free(old_pages);
}

internal b32
font_load(System_Functions *system, Font_Settings *settings, Font_Metrics *metrics, Font_Page_Storage *pages){
    i32 pt_size = settings->parameters.pt_size;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    b32 success = font_ft_get_face(ft, &settings->stub, &settings->parameters, &face);
    
    if (success){    
        FT_Size_RequestRec_ size = {};
        size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
        size.height = (pt_size << 6);
        FT_Request_Size(face, &size);
        
        // NOTE(allen): Set size and metrics
        char *name = face->family_name;
        if (name != 0){
            i32 name_len = str_size(name);
            name_len = clamp_top(name_len, sizeof(metrics->name)-1);
            memcpy(metrics->name, name, name_len + 1);
            metrics->name_len = name_len;
        }
        else{
            if (!settings->stub.load_from_path){
                i32 name_len = settings->stub.len;
                memcpy(metrics->name, settings->stub.name, name_len + 1);
                metrics->name_len = name_len;
            }
        }
        
        metrics->ascent    = ceil32(face->size->metrics.ascender   /64.f);
        metrics->descent   = floor32(face->size->metrics.descender /64.f);
        metrics->advance   = ceil32(face->size->metrics.max_advance/64.f);
        metrics->height    = ceil32(face->size->metrics.height     /64.f);
        metrics->line_skip = metrics->height - (metrics->ascent - metrics->descent);
        metrics->height   -= metrics->line_skip;
        metrics->line_skip = 0;
        
        if (settings->parameters.underline){
            f32 notional_to_real_ratio = (f32)metrics->height/(f32)face->height;
            f32 relative_center = -1.f*notional_to_real_ratio*face->underline_position;
            f32 relative_thickness = notional_to_real_ratio*face->underline_thickness;
            
            f32 center    = (f32)floor32(metrics->ascent + relative_center);
            f32 thickness = clamp_bot(1.f, relative_thickness);
            
            metrics->underline_yoff1 = center - thickness*0.5f;
            metrics->underline_yoff2 = center + thickness*0.5f;
        }
        else{
            metrics->underline_yoff1 = 0.f;
            metrics->underline_yoff2 = 0.f;
        }
        
        if (metrics->height > pt_size*4 || metrics->height < 6){
            success = false;
        }
        else{
            // NOTE(allen): Set texture and glyph data.
            Assert(font_get_page(pages, 0) == 0);
            Glyph_Page *page = font_allocate_and_hash_new_page(system, pages, 0);
            font_load_page_layout(settings, metrics, page, 0);
            
            // NOTE(allen): Whitespace spacing stuff
            i32 tab_width = 4;
            
            f32 space_adv = page->advance[' '];
            f32 backslash_adv = page->advance['\\'];
            f32 r_adv = page->advance['r'];
            
            page->advance['\n'] = space_adv;
            page->advance['\r'] = backslash_adv + r_adv;
            page->advance['\t'] = space_adv*tab_width;
            
            // NOTE(allen): The rest of the metrics.
            f32 max_hex_advance = 0.f;
            for (u32 i = '0'; i <= '9'; ++i){
                f32 adv = page->advance[i];
                max_hex_advance = Max(max_hex_advance, adv);
            }
            for (u32 i = 'a'; i <= 'f'; ++i){
                f32 adv = page->advance[i];
                max_hex_advance = Max(max_hex_advance, adv);
            }
            for (u32 i = 'A'; i <= 'F'; ++i){
                f32 adv = page->advance[i];
                max_hex_advance = Max(max_hex_advance, adv);
            }
            
            metrics->byte_advance = backslash_adv + max_hex_advance*2;
            metrics->sub_advances[0] = backslash_adv;
            metrics->sub_advances[1] = max_hex_advance;
            metrics->sub_advances[2] = max_hex_advance;
        }
    }
    
    FT_Done_FreeType(ft);
    
    return(success);
}

////////////////////////////////

internal
Sys_Font_Allocate_Sig(system_font_allocate, size){
    umem *size_ptr = 0;
    void *result = system_memory_allocate(size + sizeof(*size_ptr));
    size_ptr = (umem*)result;
    *size_ptr = size + 4;
    return(size_ptr + 1);
}

internal
Sys_Font_Free_Sig(system_font_free, ptr){
    if (ptr != 0){
        umem *size_ptr = ((umem*)ptr) - 1;
        system_memory_free(size_ptr, *size_ptr);
    }
}

internal
Sys_Font_Get_Loadable_Count_Sig(system_font_get_loadable_count){
    return(fontvars.loadable_count);
}

internal
Sys_Font_Get_Loadable_Sig(system_font_get_loadable, i, out){
    if (0 <= i && i < fontvars.loadable_count){
        memcpy(out, &fontvars.loadables[i], sizeof(*out));
    }
    else{
        memset(out, 0, sizeof(*out));
    }
}

internal
Sys_Font_Face_Allocate_And_Init_Sig(system_font_face_allocate_and_init, new_settings){
    i32 slot_max = fontvars.max_slot_count;
    Font_Slot_Page *page_with_slot = 0;
    
    Assert(fontvars.used_slot_count <= slot_max);
    if (fontvars.used_slot_count == slot_max){
        i32 memsize = round_up_i32(SLOT_PAGE_SIZE, KB(4));
        i32 page_count = memsize/SLOT_PAGE_SIZE;
        void *ptr = system_font_allocate(memsize);
        memset(ptr, 0, memsize);
        
        page_with_slot = (Font_Slot_Page*)ptr;
        
        for (i32 i = 0; i < page_count; ++i){
            i32 page_slot_count = SLOT_PER_PAGE;
            
            Font_Slot_Page *page = (Font_Slot_Page*)ptr;
            page->is_active = (u64*)(page + 1);
            page->settings = (Font_Settings*)(page->is_active + (page_slot_count + 63)/64);
            page->metrics = (Font_Metrics*)(page->settings + page_slot_count);
            page->pages = (Font_Page_Storage*)(page->metrics + page_slot_count);
            ptr = page->pages + page_slot_count;
            
            page->used_count = 0;
            page->fill_count = 0;
            page->max = page_slot_count;
            
            dll_insert_back(&fontvars.slot_pages_sentinel, page);
            if (page->prev == &fontvars.slot_pages_sentinel){
                page->first_id = 1;
            }
            else{
                page->first_id = page->prev->first_id + page->prev->max;
            }
            
            // NOTE(allen): Some of the last 64 bits of the is_active array may not line up with actual slots.
            // Set such bits to 1 to simulate the "slot" being filled for allocation purposes.
            // TODO(allen): This could be O(log n) instead of O(n) if I end up making bit manipulation helpers someday.
            u64 last_mask_fill = 0;
            if (page_slot_count%64 != 0){
                last_mask_fill = (1ULL << 63);
                for (i32 spread_step = (page_slot_count%64) - 1;
                     spread_step > 0;
                     --spread_step){
                    last_mask_fill |= (last_mask_fill >> 1);
                }
            }
            
            i32 is_active_max = (page_with_slot->max + 63)/64;
            page->is_active[is_active_max - 1] = last_mask_fill;
        }
        
        fontvars.max_slot_count += page_count*SLOT_PER_PAGE;
    }
    else{
        for (Font_Slot_Page *page = fontvars.slot_pages_sentinel.next;
             page != &fontvars.slot_pages_sentinel;
             page = page->next){
            if (page->used_count < page->max){
                page_with_slot = page;
                break;
            }
        }
    }
    
    if (page_with_slot == 0){
        //LOG("Could not get a font slot while loading a font\n");
        return(0);
    }
    
    Assert(page_with_slot->used_count < page_with_slot->max);
    
    // Get a fillable index
    i32 index = -1;
    if (page_with_slot->fill_count < page_with_slot->max){
        index = page_with_slot->fill_count;
    }
    else{
        i32 is_active_max = (page_with_slot->max + 63)/64;
        u64 *is_active_ptr = page_with_slot->is_active;
        for (i32 i = 0; i < is_active_max; ++i){
            u64 is_active_v = is_active_ptr[i];
            if (is_active_v != (~0)){
                i32 j_stop = 64;
                if (i + 1 == is_active_max){
                    j_stop = SLOT_PER_PAGE%64;
                }
                for (i32 j = 0; j < j_stop; ++j){
                    if ((is_active_v & (1ULL << j)) == 0){
                        index = i*64 + j;
                        break;
                    }
                }
                break;
            }
        }
    }
    
    // Get the slot pointers.
    Assert(index != -1);
    
    u64 *is_active_flags = &page_with_slot->is_active[index/64];
    u64 is_active_mask = (1ULL << (index % 64));
    Font_Settings *settings = &page_with_slot->settings[index];
    Font_Metrics *metrics = &page_with_slot->metrics[index];
    Font_Page_Storage *pages = &page_with_slot->pages[index];
    Face_ID new_id = page_with_slot->first_id + index;
    
    Assert(((*is_active_flags) & is_active_mask) == 0);
    
    char *filename = new_settings->stub.name;
    i32 filename_len = 0;
    for (;filename[filename_len];++filename_len);
    
    // Initialize font settings.
    memcpy(settings, new_settings, sizeof(*new_settings));
    
    memset(metrics, 0, sizeof(*metrics));
    memset(pages, 0, sizeof(*pages));
    b32 success = font_load(&sysfunc, settings, metrics, pages);
    if (success){
        *is_active_flags |= is_active_mask;
        ++fontvars.used_slot_count;
        ++page_with_slot->used_count;
        if (index >= page_with_slot->fill_count){
            page_with_slot->fill_count = index + 1;
        }
    }
    else{
        new_id = 0;
    }
    
    if (new_id > fontvars.largest_font_id){
        fontvars.largest_font_id = new_id;
    }
    
    return(new_id);
}

internal
Sys_Font_Get_Largest_ID_Sig(system_font_get_largest_id){
    return(fontvars.largest_font_id);
}

internal
Sys_Font_Get_Count_Sig(system_font_get_count){
    return(fontvars.used_slot_count);
}

internal Font_Slot_Page_And_Index
system_font_get_active_location(Face_ID font_id){
    Font_Slot_Page_And_Index result = {};
    
    for (Font_Slot_Page *page = fontvars.slot_pages_sentinel.next;
         page != &fontvars.slot_pages_sentinel;
         page = page->next){
        if (page->first_id <= font_id && font_id < page->first_id + SLOT_PER_PAGE){
            i32 index = (i32)(font_id - page->first_id);
            u64 is_active_v = page->is_active[index/64];
            if ((is_active_v & (1ULL << (index%64))) != 0){
                result.page = page;
                result.index = index;
            }
            break;
        }
    }
    
    return(result);
}

internal
Sys_Font_Face_Change_Settings_Sig(system_font_face_change_settings, font_id, new_settings){
    if (font_id == 0){
        return(false);
    }
    
    Font_Slot_Page_And_Index page_and_index = system_font_get_active_location(font_id);
    if (page_and_index.page == 0){
        return(false);
    }
    
    Font_Settings *old_settings = &page_and_index.page->settings[page_and_index.index];
    if (memcmp(new_settings, old_settings, sizeof(*new_settings)) == 0){
        return(false);
    }
    
    b32 made_change = false;
    
    Font_Metrics temp_metrics = {};
    Font_Page_Storage temp_pages = {};
    
    if (font_load(&sysfunc, new_settings, &temp_metrics, &temp_pages)){
        Font_Metrics *metrics_ptr = &page_and_index.page->metrics[page_and_index.index];
        Font_Page_Storage *pages_ptr = &page_and_index.page->pages[page_and_index.index];
        font_release_pages(&sysfunc, pages_ptr);
        memcpy(old_settings, new_settings, sizeof(*old_settings));
        memcpy(metrics_ptr, &temp_metrics, sizeof(*metrics_ptr));
        memcpy(pages_ptr, &temp_pages, sizeof(*pages_ptr));
        made_change = true;
    }
    
    return(made_change);
}

internal
Sys_Font_Face_Release_Sig(system_font_face_release, font_id){
    if (fontvars.used_slot_count == 1){
        return(false);
    }
    
    if (font_id == 0){
        return(false);
    }
    
    Font_Slot_Page_And_Index page_and_index = system_font_get_active_location(font_id);
    if (page_and_index.page == 0){
        return(false);
    }
    
    Font_Page_Storage *pages_ptr = &page_and_index.page->pages[page_and_index.index];
    font_release_pages(&sysfunc, pages_ptr);
    
    u64 *is_active_ptr = &page_and_index.page->is_active[page_and_index.index/64];
    (*is_active_ptr) &= (~(1 << (page_and_index.index%64)));
    
    return(true);
}

internal
Sys_Font_Get_Name_By_ID_Sig(system_font_get_name_by_id, font_id, str_out, capacity){
    i32 length = 0;
    if (font_id == 0){
        return(length);
    }
    
    Font_Slot_Page_And_Index page_and_index = system_font_get_active_location(font_id);
    if (page_and_index.page != 0){
        Font_Metrics *metrics = &page_and_index.page->metrics[page_and_index.index];
        length = metrics->name_len;
        copy_partial_cs(str_out, capacity, make_string(metrics->name, length));
    }
    
    return(length);
}

internal
Sys_Font_Get_Pointers_By_ID_Sig(system_font_get_pointers_by_id, font_id){
    Font_Pointers font = {};
    if (font_id == 0){
        return(font);
    }
    
    Font_Slot_Page_And_Index page_and_index = system_font_get_active_location(font_id);
    if (page_and_index.page != 0){
        font.valid    = true;
        font.settings = &page_and_index.page->settings[page_and_index.index];
        font.metrics  = &page_and_index.page->metrics[page_and_index.index];
        font.pages    = &page_and_index.page->pages[page_and_index.index];
    }
    
    return(font);
}

internal
Sys_Font_Load_Page_Sig(system_font_load_page, settings, metrics, page, page_number){
    Assert(page_number != 0);
    font_load_page_layout(settings, metrics, page, page_number);
}

////////////////////////////////

internal Font_Setup_List
system_font_get_local_stubs(Arena *arena){
    Font_Setup_List list = {};
    
    u32 dir_max = KB(32);
    u8 *directory = push_array(arena, u8, dir_max);
    String dir_str = make_string_cap(directory, 0, dir_max);
    u32 dir_len = dir_str.size = system_get_4ed_path(dir_str.str, dir_str.memory_size);
    Assert(dir_len < dir_max);
    
    set_last_folder_sc(&dir_str, "fonts", SLASH);
    terminate_with_null(&dir_str);
    dir_len = dir_str.size;
    
    pop_array(arena, u8, dir_max - dir_len - 1);
    
    File_List file_list = {};
    system_set_file_list(&file_list, (char*)directory, 0, 0, 0);
    
    for (u32 i = 0; i < file_list.count; ++i){
        File_Info *info = &file_list.infos[i];
        
        char *filename = info->filename;
        u32 len = 0;
        for (;filename[len];++len);
        
        if (dir_len + len + 1 <= sizeof(list.first->stub.name)){
            Font_Setup *setup = push_array_zero(arena, Font_Setup, 1);
            sll_queue_push(list.first, list.last, setup);
            setup->stub.load_from_path = true;
            setup->stub.in_font_folder = true;
            memcpy(&setup->stub.name[0], directory, dir_len);
            memcpy(&setup->stub.name[dir_len], filename, len + 1);
            setup->stub.len = dir_len + len;
        }
    }
    
    system_set_file_list(&file_list, 0, 0, 0, 0);
    
    return(list);
}

internal void
system_font_init(Font_Functions *font_links, u32 pt_size, b32 use_hinting, Font_Setup_List list){
    // Linking
    font_links->get_loadable_count     = system_font_get_loadable_count;
    font_links->get_loadable           = system_font_get_loadable;
    font_links->face_allocate_and_init = system_font_face_allocate_and_init;
    font_links->face_change_settings   = system_font_face_change_settings;
    font_links->get_largest_id         = system_font_get_largest_id;
    font_links->get_count              = system_font_get_count;
    font_links->get_name_by_id         = system_font_get_name_by_id;
    font_links->get_pointers_by_id     = system_font_get_pointers_by_id;
    font_links->load_page              = system_font_load_page;
    font_links->allocate               = system_font_allocate;
    font_links->free                   = system_font_free;
    
    // Initialize fontvars
    memset(&fontvars, 0, sizeof(fontvars));
    dll_init_sentinel(&fontvars.slot_pages_sentinel);
    
    // TODO(allen): Eliminate from fontvars.
    fontvars.pt_size = pt_size;
    fontvars.use_hinting = use_hinting;
    
    // Filling loadable font descriptions
    for (Font_Setup *ptr = list.first;
         ptr != 0;
         ptr = ptr->next){
        Font_Loadable_Stub *stub = &ptr->stub;
        
        if (fontvars.loadable_count < ArrayCount(fontvars.loadables)){
            Font_Loadable_Description *loadable = &fontvars.loadables[fontvars.loadable_count];
            
            b32 name_good = false;
            i32 capacity = (i32)(sizeof(loadable->display_name));
            
            if (stub->load_from_path){
                if (ptr->has_display_name){
                    name_good = true;
                    memcpy(loadable->display_name, ptr->name, ptr->len);
                }
                else{
                    name_good = font_load_name(stub, loadable->display_name, capacity);
                }
                if (name_good){
                    loadable->display_len = str_size(loadable->display_name);
                }
            }
            else{
                i32 len = str_size(stub->name);
                if (len < capacity){
                    name_good = true;
                    memcpy(loadable->display_name, stub->name, len + 1);
                    loadable->display_len = len;
                }
            }
            
            if (name_good){
                if (stub->in_font_folder){
                    memcpy(&loadable->stub, stub, sizeof(*stub));
                }
                else{
                    stub->load_from_path = false;
                    stub->in_font_folder = false;
                    stub->len = loadable->display_len;
                    memset(loadable->stub.name, 0, sizeof(stub->name));
                    memcpy(loadable->stub.name, loadable->display_name, stub->len);
                }
                
                loadable->valid = true;
                ++fontvars.loadable_count;
            }
        }
    }
    
    // Force load one font.
    Font_Setup *first_setup = list.first;
    Font_Settings first_settings = {};
    memcpy(&first_settings.stub, &first_setup->stub, sizeof(first_setup->stub));
    first_settings.parameters.pt_size = pt_size;
    first_settings.parameters.use_hinting = use_hinting;
    system_font_face_allocate_and_init(&first_settings);
}
#endif

// BOTTOM

