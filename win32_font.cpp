/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 font rendering for nicer fonts
 *
 */

// TOP

struct GMetrics{
    f32 advance;
    f32 xoff;
    f32 xoff2;
};

internal b32
win32_glyph_metrics(HDC dc, int code, GMetrics *gmetrics){
    b32 result = false;
    ABCFLOAT abc = {0};
    INT width;
    
    if (GetCharWidth32W(dc, code, code, &width)){
        gmetrics->advance = (f32)width;
        if (GetCharABCWidthsFloat(dc, code, code, &abc)){
            gmetrics->xoff = abc.abcfA;
            gmetrics->xoff2 = (abc.abcfA + abc.abcfB);
            
            result = true;
        }
    }
    
    return(result);
}

internal i32
win32_draw_font_load(Partition *part,
                     Render_Font *font_out,
                     char *filename_untranslated,
                     char *fontname,
                     i32 pt_size,
                     i32 tab_width,
                     i32 oversample,
                     b32 store_texture){
    
    char space_[1024];
    String filename = make_fixed_width_string(space_);
    b32 translate_success = sysshared_to_binary_path(&filename, filename_untranslated);
    
    i32 result = 0;
    
    if (translate_success){
        HDC dc = GetDC(win32vars.window_handle);
        
        AddFontResourceEx(filename.str, FR_PRIVATE, 0);
        
        HFONT font_handle = CreateFont(
            pt_size,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            FF_DONTCARE | DEFAULT_PITCH,
            fontname
            );
        HBITMAP bmp_handle = CreateBitmap(
            pt_size*2, pt_size*2,
            4, 32,
            0
            );
        
        
        if (font_handle != 0 && bmp_handle != 0){
            SelectObject(dc, font_handle);
            SelectObject(dc, bmp_handle);
            
            memset(font_out, 0, sizeof(*font_out));
            
            TEXTMETRIC metric;
            if (GetTextMetrics(dc, &metric)){
                
                font_out->height = (i32)(metric.tmHeight + metric.tmExternalLeading);
                font_out->ascent = (i32)(metric.tmAscent);
                font_out->descent = (i32)(metric.tmDescent);
                font_out->line_skip = (i32)(metric.tmExternalLeading);
                
                if (!store_texture){
                    result = 1;
                }
                else{
                    Temp_Memory temp = begin_temp_memory(part);
                    
                    i32 tex_width = pt_size*16;
                    i32 tex_height = pt_size*16;
                    void *block = sysshared_push_block(part, tex_width * tex_height);
                    
                    font_out->tex_width = tex_width;
                    font_out->tex_height = tex_height;
                    
                    result = 1;
                    /////////////////////////////////////////////////////////////////
                    stbtt_pack_context spc_;
                    
                    int pack_result;
                    {
                        stbtt_pack_context *spc = &spc_;
                        unsigned char  *pixels = (u8*)block;
                        int pw = tex_width;
                        int ph = tex_height;
                        int stride_in_bytes = tex_width;
                        int padding = 1;
                        void *alloc_context = part;
                        
                        {
                            stbrp_context *context = (stbrp_context *) STBTT_malloc(sizeof(*context)            ,alloc_context);
                            int            num_nodes = pw - padding;
                            stbrp_node    *nodes   = (stbrp_node    *) STBTT_malloc(sizeof(*nodes  ) * num_nodes,alloc_context);
                            
                            if (context == NULL || nodes == NULL) {
                                if (context != NULL) STBTT_free(context, alloc_context);
                                if (nodes   != NULL) STBTT_free(nodes  , alloc_context);
                                pack_result = 0;
                                goto packbegin_end;
                            }
                            
                            spc->user_allocator_context = alloc_context;
                            spc->width = pw;
                            spc->height = ph;
                            spc->pixels = pixels;
                            spc->pack_info = context;
                            spc->nodes = nodes;
                            spc->padding = padding;
                            spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
                            spc->h_oversample = 1;
                            spc->v_oversample = 1;
                            
                            stbrp_init_target(context, pw-padding, ph-padding, nodes, num_nodes);
                            
                            STBTT_memset(pixels, 0, pw*ph); // background of 0 around pixels
                            
                            pack_result = 1;
                            goto packbegin_end;
                        }
                    }
                    packbegin_end:;
                    
                    if (pack_result){
                        
                        int pack_font_range_result;
                        
                        {
                            stbtt_pack_range range;
                            range.first_unicode_char_in_range = 0;
                            range.num_chars_in_range          = 128;
                            range.chardata_for_range          = font_out->chardata;
                            range.font_size                   = STBTT_POINT_SIZE((f32)pt_size);
                            
                            stbtt_pack_context *spc = &spc_;
                            stbtt_pack_range *ranges = &range;
                            int num_ranges = 1;
                            
                            {
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
                                if (rects == NULL){
                                    pack_font_range_result = 0;
                                    goto pack_font_range_end;
                                }
                                
                                //info.userdata = spc->user_allocator_context;
                                //stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata,font_index));
                                k=0;
                                for (i=0; i < num_ranges; ++i) {
                                    //float fh = ranges[i].font_size;
                                    //float scale = fh > 0 ? stbtt_ScaleForPixelHeight(&info, fh) : stbtt_ScaleForMappingEmToPixels(&info, -fh);
                                    for (j=0; j < ranges[i].num_chars_in_range; ++j) {
                                        int w,h;
                                        
                                        GMetrics gmetrics;
                                        
                                        if (!win32_glyph_metrics(dc, ranges[i].first_unicode_char_in_range + j, &gmetrics)){
                                            result = 0;
                                            break;
                                        }
                                        
                                        w = CEIL32(gmetrics.xoff2 - gmetrics.xoff);
                                        h = font_out->ascent + font_out->descent;
                                        
                                        rects[k].w = (stbrp_coord) (w + spc->padding);
                                        rects[k].h = (stbrp_coord) (h + spc->padding);
                                        ++k;
                                    }
                                }
                                
                                stbrp_pack_rects(context, rects, k);
                                
                                k = 0;
                                for (i=0; i < num_ranges; ++i) {
                                    //float fh = ranges[i].font_size;
                                    //float scale = fh > 0 ? stbtt_ScaleForPixelHeight(&info, fh) : stbtt_ScaleForMappingEmToPixels(&info, -fh);
                                    for (j=0; j < ranges[i].num_chars_in_range; ++j) {
                                        stbrp_rect *r = &rects[k];
                                        if (r->was_packed) {
                                            stbtt_packedchar *bc = &ranges[i].chardata_for_range[j];
                                            //int glyph = stbtt_FindGlyphIndex(&info, ranges[i].first_unicode_char_in_range + j);
                                            int code = ranges[i].first_unicode_char_in_range + j;
                                            stbrp_coord pad = (stbrp_coord) spc->padding;
                                            
                                            // pad on left and top
                                            r->x += pad;
                                            r->y += pad;
                                            r->w -= pad;
                                            r->h -= pad;
                                            
#if 0
                                            int advance, lsb, x0,y0,x1,y1;
                                            
                                            stbtt_GetGlyphHMetrics(&info, glyph, &advance, &lsb);
                                            stbtt_GetGlyphBitmapBox(&info, glyph,
                                                                    scale * spc->h_oversample,
                                                                    scale * spc->v_oversample,
                                                                    &x0,&y0,&x1,&y1);
                                            stbtt_MakeGlyphBitmapSubpixel(&info,
                                                                          spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                                                          r->w - spc->h_oversample+1,
                                                                          r->h - spc->v_oversample+1,
                                                                          spc->stride_in_bytes,
                                                                          scale * spc->h_oversample,
                                                                          scale * spc->v_oversample,
                                                                          0,0,
                                                                          glyph);
#else
                                            float advance, x0, y0, x1, y1;
                                            
                                            GMetrics gmetrics;
                                            if (!win32_glyph_metrics(dc, code, &gmetrics)){
                                                result = false;
                                                break;
                                            }
                                            
                                            advance = gmetrics.advance;
                                            x0 = gmetrics.xoff;
                                            y0 = -font_out->ascent;
                                            x1 = gmetrics.xoff2;
                                            y1 = font_out->descent;
                                            
#endif
                                            
                                            bc->x0       = (stbtt_int16)  r->x;
                                            bc->y0       = (stbtt_int16)  r->y;
                                            bc->x1       = (stbtt_int16) (r->x + r->w);
                                            bc->y1       = (stbtt_int16) (r->y + r->h);
                                            bc->xadvance =       (float)  advance;
                                            bc->xoff     =       (float)  x0;
                                            bc->yoff     =       (float)  y0;
                                            bc->xoff2    =       (float)  (x0 + r->w);
                                            bc->yoff2    =       (float)  (y0 + r->h);
                                        } else {
                                            return_value = 0; // if any fail, report failure
                                        }
                                        
                                        ++k;
                                    }
                                }
                                
                                STBTT_free(rects, spc->user_allocator_context);
                                pack_font_range_result =  return_value;
                                goto pack_font_range_end;
                            }
                        }
                        pack_font_range_end:;
                        
                        
                        if (!pack_font_range_result){
                            result = 0;
                        }
                        
                        {
                            stbtt_pack_context *spc = &spc_;
                            {
                                STBTT_free(spc->nodes    , spc->user_allocator_context);
                                STBTT_free(spc->pack_info, spc->user_allocator_context);
                            }
                        }
                    }
                    else{
                        result = 0;
                    }
                    /////////////////////////////////////////////////////////////////
                    
                    if (result){
                        GLuint font_tex;
                        glGenTextures(1, &font_tex);
                        glBindTexture(GL_TEXTURE_2D, font_tex);
                        
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        
                        memset(block, 0xFF, tex_width*tex_height);
                        
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, block);
                        
                        font_out->tex = font_tex;
                        glBindTexture(GL_TEXTURE_2D, 0);
                        
                        font_out->chardata['\r'] = font_out->chardata[' '];
                        font_out->chardata['\n'] = font_out->chardata[' '];
                        font_out->chardata['\t'] = font_out->chardata[' '];
                        font_out->chardata['\t'].xadvance *= tab_width;
                        
                        for (u8 code_point = 0; code_point < 128; ++code_point){
                            font_out->advance_data[code_point] = font_out->chardata[code_point].xadvance;
                        }
                        
                        i32 max_advance = metric.tmMaxCharWidth;
                        font_out->advance = max_advance - 1;
                    }
                    
                    end_temp_memory(temp);
                }
            }
            
            DeleteObject(font_handle);
            DeleteObject(bmp_handle);
        }
        
        ReleaseDC(win32vars.window_handle, dc);
    }
    
    return(result);
}

// BOTTOM


