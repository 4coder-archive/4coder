/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 font rendering for nicer fonts
 *
 */

// TOP

struct Glyph_Bitmap{
    HBITMAP bitmap_handle;
    char *pixels;
};

void
win32_get_box(HDC dc, TCHAR character, int *x0, int *y0, int *x1, int *y1){
    SIZE size;
    GetTextExtentPoint32A(dc, &character, 1, &size);
    *x0 = 0;
    *y0 = 0;
    *x1 = size.cx;
    *y1 = size.cy;
}

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
            TEXTMETRIC metrics;
            GetTextMetrics(dc, &metrics);
            font_out->height = metrics.tmHeight + metrics.tmExternalLeading;
            font_out->ascent = metrics.tmAscent;
            font_out->descent = -metrics.tmDescent;
            font_out->line_skip = metrics.tmExternalLeading;
            font_out->advance = metrics.tmMaxCharWidth;
            
            result = 1;
            if (store_texture){
                Temp_Memory temp = begin_temp_memory(part);
                
                i32 tex_width = pt_size*16*oversample;
                i32 tex_height = pt_size*16*oversample;
                
                Glyph_Bitmap glyph_bitmap;
                glyph_bitmap.bitmap_handle = CreateCompatibleBitmap(dc, tex_width, tex_height);
                glyph_bitmap.pixels = push_array(part, char, tex_width*tex_height);
                
                SelectObject(dc, glyph_bitmap.bitmap_handle);
                SelectObject(dc, font_handle);
                
                SetBkColor(dc, RGB(0, 0, 0));
                
                stbtt_pack_context context = {0};
                stbtt_PackBegin(&context, (unsigned char*)glyph_bitmap.pixels, tex_width, tex_height, 0, 1, part);
                
                {
                    stbtt_pack_context *spc = &context;
                    int first_unicode_char_in_range = 0;
                    int num_chars_in_range = 128;
                    stbtt_packedchar *chardata_for_range = font_out->chardata;
                    
                    {
                        stbtt_pack_range range;
                        range.first_unicode_char_in_range = first_unicode_char_in_range;
                        range.num_chars_in_range          = num_chars_in_range;
                        range.chardata_for_range          = chardata_for_range;
                        
                        {
                            stbtt_pack_range *ranges = &range;
                            int num_ranges = 1;
                            
                            {
                                float recip_h = 1.0f / spc->h_oversample;
                                float recip_v = 1.0f / spc->v_oversample;
                                float sub_x = stbtt__oversample_shift(spc->h_oversample);
                                float sub_y = stbtt__oversample_shift(spc->v_oversample);
                                int i,j,k,n, return_value = 1;
                                stbrp_context *context = (stbrp_context *) spc->pack_info;
                                stbrp_rect    *rects;
                                
                                // flag all characters as NOT packed
                                for (i=0; i < num_ranges; ++i)
                                    for (j=0; j < ranges[i].num_chars_in_range; ++j)
                                    ranges[i].chardata_for_range[j].x0 =
                                    ranges[i].chardata_for_range[j].y0 =
                                    ranges[i].chardata_for_range[j].x1 =
                                    ranges[i].chardata_for_range[j].y1 = 0;
                                
                                n = 0;
                                for (i=0; i < num_ranges; ++i)
                                    n += ranges[i].num_chars_in_range;
                                
                                rects = (stbrp_rect *) STBTT_malloc(sizeof(*rects) * n, spc->user_allocator_context);
                                if (rects == NULL)
                                    return 0;
                                
                                k=0;
                                for (i=0; i < num_ranges; ++i) {
                                    for (j=0; j < ranges[i].num_chars_in_range; ++j) {
                                        int x0,y0,x1,y1;
                                        
                                        TCHAR character = (TCHAR)(ranges[i].first_unicode_char_in_range + j);
                                        win32_get_box(dc, character, &x0, &y0, &x1, &y1);
                                        
                                        rects[k].w = (stbrp_coord) (x1-x0 + spc->padding + spc->h_oversample-1);
                                        rects[k].h = (stbrp_coord) (y1-y0 + spc->padding + spc->v_oversample-1);
                                        ++k;
                                    }
                                }
                                
                                stbrp_pack_rects(context, rects, k);
                                
                                k = 0;
                                for (i=0; i < num_ranges; ++i) {
                                    for (j=0; j < ranges[i].num_chars_in_range; ++j) {
                                        stbrp_rect *r = &rects[k];
                                        if (r->was_packed) {
                                            stbtt_packedchar *bc = &ranges[i].chardata_for_range[j];
                                            int advance, x0,y0,x1,y1;
                                            int glyph = ranges[i].first_unicode_char_in_range + j;
                                            stbrp_coord pad = (stbrp_coord) spc->padding;
                                            
                                            GetCharWidth32W(dc, glyph, glyph, &advance);
                                            
                                            TCHAR character = (TCHAR)(ranges[i].first_unicode_char_in_range + j);
                                            win32_get_box(dc, character, &x0, &y0, &x1, &y1);
                                            
                                            // pad on left and top
                                            r->x += pad;
                                            r->y += pad;
                                            r->w -= pad;
                                            r->h -= pad;
                                            
                                            SetTextColor(dc, RGB(255, 255, 255));
                                            TextOutA(dc, r->x, r->y, &character, 1);
                                            
                                            if (spc->h_oversample > 1)
                                                stbtt__h_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                                                   r->w, r->h, spc->stride_in_bytes,
                                                                   spc->h_oversample);
                                            
                                            if (spc->v_oversample > 1)
                                                stbtt__v_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                                                   r->w, r->h, spc->stride_in_bytes,
                                                                   spc->v_oversample);
                                            
                                            bc->x0       = (stbtt_int16)  r->x;
                                            bc->y0       = (stbtt_int16)  r->y;
                                            bc->x1       = (stbtt_int16) (r->x + r->w);
                                            bc->y1       = (stbtt_int16) (r->y + r->h);
                                            bc->xadvance =                (float) advance;
                                            bc->xoff     =       (float)  x0 * recip_h + sub_x;
                                            bc->yoff     =       (float)  y0 * recip_v + sub_y;
                                            bc->xoff2    =                (x0 + r->w) * recip_h + sub_x;
                                            bc->yoff2    =                (y0 + r->h) * recip_v + sub_y;
                                        } else {
                                            return_value = 0; // if any fail, report failure
                                        }
                                        
                                        ++k;
                                    }
                                }
                                
                                STBTT_free(rects, spc->user_allocator_context);
                                
                                result = return_value;
                                
                                if (return_value){
                                    GLuint font_tex;
                                    glGenTextures(1, &font_tex);
                                    glBindTexture(GL_TEXTURE_2D, font_tex);
                                    
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                    
                                    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, glyph_bitmap.pixels);
                                    
                                    font_out->tex = font_tex;
                                    glBindTexture(GL_TEXTURE_2D, 0);
                                    
                                    font_out->chardata['\r'] = font_out->chardata[' '];
                                    font_out->chardata['\n'] = font_out->chardata[' '];
                                    font_out->chardata['\t'] = font_out->chardata[' '];
                                    font_out->chardata['\t'].xadvance *= tab_width;
                                }
                            }
                        }
                    }
                }
                
                stbtt_PackEnd(&context);
                
                end_temp_memory(temp);
            }
        }
        
        DeleteObject(font_handle);
    }
    
    return(result);
}

// BOTTOM


