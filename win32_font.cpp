/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 font rendering for nicer fonts
 *
 */

// TOP

internal i32
win32_draw_font_load(Partition *part,
                     Render_Font *font_out,
                     char *filename_untranslated,
                     i32 pt_size,
                     i32 tab_width,
                     i32 oversample,
                     b32 store_texture){
    
    char space_[1024];
    String filename = make_fixed_width_string(space_);
    b32 translate_success = sysshared_to_binary_path(&filename, filename_untranslated);
    if (!translate_success) return 0;
    
    i32 result = 0;
    
    AddFontResourceEx(filename.str, FR_PRIVATE, 0);
    
    HFONT font_handle =
        CreateFontA(pt_size, 0, 0, 0,
                    FW_NORMAL, // WEIGHT
                    FALSE,     // ITALICS
                    FALSE,     // UNDERLINE
                    FALSE,     // STRIKE-OUT
                    ANSI_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    ANTIALIASED_QUALITY,
                    DEFAULT_PITCH|FF_DONTCARE,
                    filename.str);
    
    if (font_handle){
        HDC dc = CreateCompatibleDC(0);
        
        if (dc){
            // TODO(allen): Have to get metrics
            
            result = 1;
            
            if (store_texture){
                i32 tex_width = pt_size*16*oversample;
                i32 tex_height = pt_size*16*oversample;
                
                HBITAMP bitmap = CreateCompatibleBitmap(dc, tex_width, tex_height);
                
                // TODO(allen): pack each glyph into a texture
                // and generate the equivalent data output by stb
                // in the stbtt_packedchar array.
                
            }
        }
        
        DeleteObject(font_handle);
    }
    
    return(result);
}

// BOTTOM


