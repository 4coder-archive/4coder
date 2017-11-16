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
font_ft_flags(b32 use_hinting){
    u32 ft_flags = FT_LOAD_RENDER;
    
    if (use_hinting){
        // NOTE(inso): FT_LOAD_TARGET_LIGHT does hinting only vertically, which looks nicer imo
        // maybe it could be exposed as an option for hinting, instead of just on/off.
        ft_flags |= FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
    }
    else{
        ft_flags |= (FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_HINTING);
    }
    
    return(ft_flags);
}

internal void
font_load_page_layout(Font_Settings *settings, Font_Metrics *metrics, Glyph_Page *page, u32 page_number){
    Assert(page != 0);
    memset(page, 0, sizeof(*page));
    
    char *filename = settings->stub.name;
    u32 pt_size = settings->pt_size;
    b32 use_hinting = settings->use_hinting;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    page->page_number = page_number;
    
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
    } while(tex_height > tex_width);
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
            *advance_out = (f32)ceil32(face->glyph->advance.x / 64.0f);
            
            pen_x = ceil32(glyph_out->x1 + 1);
        }
    }
    
    // TODO(allen): Not sure setting tex_height here is right... double check.
    tex_height = round_up_pot_u32(pen_y + pen_y_descent);
    
    page->tex_width  = tex_width;
    page->tex_height = tex_height;
    page->has_layout = true;
    
    FT_Done_FreeType(ft);
}

internal u32*
font_load_page_pixels(Partition *part, Font_Settings *settings, Glyph_Page *page, u32 page_number, i32 *tex_width_out, i32 *tex_height_out){
    Assert(page != 0);
    Assert(page->has_layout);
    Assert(page->page_number == page_number);
    
    char *filename = settings->stub.name;
    i32 pt_size = settings->pt_size;
    b32 use_hinting = settings->use_hinting;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    page->page_number = page_number;
    
    // NOTE(allen): Prepare a pixel buffer.
    i32 tex_width   = page->tex_width;
    i32 tex_height  = page->tex_height;
    
    u32* pixels = push_array(part, u32, tex_width*tex_height);
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
            i32 pitch = face->glyph->bitmap.pitch;
            i32 end_x = x + w;
            i32 end_y = y + h;
            for (i32 Y = y, YY = 0; Y < end_y; ++Y, ++YY){
                for (i32 X = x, XX = 0; X < end_x; ++X, ++XX){
                    pixels[Y*tex_width + X] = 0x00FFFFFF + (0x01000000*src[YY*pitch + XX]);
                    //pixels[Y*tex_width + X] = (0x01010101*src[YY*pitch + XX]);
                }
            }
        }
    }
    
    *tex_width_out = tex_width;
    *tex_height_out = tex_height;
    
    FT_Done_FreeType(ft);
    
    return(pixels);
}

internal b32
font_load(System_Functions *system, Font_Settings *settings, Font_Metrics *metrics, Font_Page_Storage *pages){
    char *filename = settings->stub.name;
    i32 pt_size = settings->pt_size;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = (pt_size << 6);
    FT_Request_Size(face, &size);
    
    // NOTE(allen): Set size and metrics
    char *name = face->family_name;
    u32 name_len = 0;
    for (;name[name_len];++name_len);
    name_len = clamp_top(name_len, sizeof(metrics->name)-1);
    memcpy(metrics->name, name, name_len);
    metrics->name[name_len] = 0;
    metrics->name_len = name_len;
    
    metrics->ascent    = ceil32  (face->size->metrics.ascender    / 64.0f);
    metrics->descent   = floor32 (face->size->metrics.descender   / 64.0f);
    metrics->advance   = ceil32  (face->size->metrics.max_advance / 64.0f);
    metrics->height    = ceil32  (face->size->metrics.height      / 64.0f);
    metrics->line_skip = metrics->height - (metrics->ascent - metrics->descent);
    metrics->height   -= metrics->line_skip;
    metrics->line_skip = 0;
    
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
    
    FT_Done_FreeType(ft);
    
    return(true);
}

////////////////////////////////

internal
Sys_Font_Get_Count_Sig(system_font_get_count){
    return(fontvars.count);
}

internal
Sys_Font_Get_Name_By_ID_Sig(system_font_get_name_by_id, str_out, capacity){
    i32 length = 0;
    if (0 < font_id && font_id <= fontvars.count){
        u32 index = font_id - 1;
        Font_Slot *slot = &fontvars.slots[index];
        if (slot->is_active){
            Font_Metrics *metrics = &slot->metrics;
            length = metrics->name_len;
            copy_partial_cs(str_out, capacity, make_string(metrics->name, length));
        }
    }
    return(length);
}

internal
Sys_Font_Get_Pointers_By_ID_Sig(system_font_get_pointers_by_id, font_id){
    Font_Pointers font = {0};
    if (0 < font_id && font_id <= fontvars.count){
        u32 index = font_id - 1;
        Font_Slot *slot = &fontvars.slots[index];
        if (slot->is_active){
            font.valid    = true;
            font.settings = &slot->settings;
            font.metrics  = &slot->metrics;
            font.pages    = &slot->pages;
        }
    }
    return(font);
}

internal
Sys_Font_Load_Page_Sig(system_font_load_page, settings, metrics, page, page_number){
    Assert(page_number != 0);
    font_load_page_layout(settings, metrics, page, page_number);
}

internal
Sys_Font_Allocate_Sig(system_font_allocate){
    i64 *size_ptr = 0;
    void *result = system_memory_allocate(size + sizeof(*size_ptr));
    size_ptr = (i64*)result;
    *size_ptr = size + 4;
    return(size_ptr + 1);
}

internal
Sys_Font_Free_Sig(system_font_free){
    if (ptr != 0){
        i64 *size_ptr = ((i64*)ptr) - 1;
        system_memory_free(size_ptr, *size_ptr);
    }
}

////////////////////////////////

internal Font_Setup*
system_font_get_stubs(Partition *part){
    Font_Setup *first_setup = 0;
    Font_Setup *head_setup = 0;
    
    u32 dir_max = KB(32);
    u8 *directory = push_array(part, u8, dir_max);
    String dir_str = make_string_cap(directory, 0, dir_max);
    u32 dir_len = dir_str.size = system_get_4ed_path(dir_str.str, dir_str.memory_size);
    Assert(dir_len < dir_max);
    
    set_last_folder_sc(&dir_str, "fonts", SLASH);
    terminate_with_null(&dir_str);
    dir_len = dir_str.size;
    
    partition_reduce(part, dir_max - dir_len - 1);
    partition_align(part, 8);
    
    File_List file_list = {0};
    system_set_file_list(&file_list, (char*)directory, 0, 0, 0);
    
    for (u32 i = 0; i < file_list.count; ++i){
        File_Info *info = &file_list.infos[i];
        
        char *filename = info->filename;
        u32 len = 0;
        for (;filename[len];++len);
        
        if (dir_len + len + 1 <= sizeof(head_setup->stub.name)){
            if (first_setup == 0){
                first_setup = push_struct(part, Font_Setup);
                head_setup = first_setup;
            }
            else{
                head_setup->next_font = push_struct(part, Font_Setup);
                head_setup = head_setup->next_font;
            }
            head_setup->next_font = 0;
            
            head_setup->stub.in_local_folder = true;
            memcpy(&head_setup->stub.name[0], directory, dir_len);
            memcpy(&head_setup->stub.name[dir_len], filename, len + 1);
            head_setup->stub.len = dir_len + len;
            
            partition_align(part, 8);
        }
        
        
    }
    
    system_set_file_list(&file_list, 0, 0, 0, 0);
    
    return(first_setup);
}

internal void
system_font_init(Font_Functions *font, u32 pt_size, b32 use_hinting, Font_Setup *font_setup_head){
    // Linking
    font->get_count = system_font_get_count;
    font->get_name_by_id = system_font_get_name_by_id;
    font->get_pointers_by_id = system_font_get_pointers_by_id;
    font->load_page = system_font_load_page;
    font->allocate = system_font_allocate;
    font->free = system_font_free;
    
    // Filling initial fonts
    //i32 font_count_max = ArrayCount(fontvars.slots);
    i32 font_count_max = 1;
    i32 font_count = 0;
    i32 i = 0;
    for (Font_Setup *ptr = font_setup_head;
         ptr != 0;
         ptr = ptr->next_font){
        char *filename = ptr->stub.name;
        
        if (i < font_count_max){
            Font_Slot *slot = &fontvars.slots[i];
            Font_Settings *settings = &slot->settings;
            Font_Metrics *metrics = &slot->metrics;
            Font_Page_Storage *pages = &slot->pages;
            
            Assert(!slot->is_active);
            
            i32 filename_len = 0;
            for (;filename[filename_len];++filename_len);
            
            if (filename_len <= sizeof(settings->stub.name) - 1){
                memset(settings, 0, sizeof(*settings));
                memset(metrics, 0, sizeof(*metrics));
                memset(pages, 0, sizeof(*pages));
                
                // Initialize Font Parameters
                memcpy(&settings->stub, &ptr->stub, sizeof(ptr->stub));
                settings->pt_size = pt_size;
                settings->use_hinting = use_hinting;
                
                b32 success = font_load(&sysfunc, settings, metrics, pages);
                if (!success){
                    memset(font, 0, sizeof(*font));
                    LOGF("font \"%.*s\" failed to load, unknown error\n", filename_len, filename);
                }
                else{
                    slot->is_active = true;
                    ++i;
                }
            }
            else{
                LOGF("font \"%.*s\" name is too long to load in current build (max %d)\n", filename_len, filename, (i32)(sizeof(settings->stub.name) - 1));
            }
        }
        else{
            LOGF("Exceeded maximum font slots (%d) skipping %s\n", font_count_max, filename);
        }
        
        ++font_count;
    }
    
    fontvars.count = clamp_top(font_count, font_count_max);
}

// BOTTOM

