/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

#if !defined(FCODER_SYSTEM_SHARED_CPP)
#define FCODER_SYSTEM_SHARED_CPP

#include "font/4coder_font_data.h"

//
// Standard implementation of file system stuff based on the file track layer.
//

struct Shared_Vars{
    File_Track_System track;
    void *track_table;
    u32 track_table_size;
    u32 track_node_size;
};

static Shared_Vars shared_vars;

internal void
init_shared_vars(){
    shared_vars.track_table_size = (16 << 10);
    shared_vars.track_table = system_get_memory(shared_vars.track_table_size);
    
    shared_vars.track_node_size = (16 << 10);
    void *track_nodes = system_get_memory(shared_vars.track_node_size);
    
    i32 track_result = init_track_system(&shared_vars.track, shared_vars.track_table, shared_vars.track_table_size, track_nodes, shared_vars.track_node_size);
    
    if (track_result != FileTrack_Good){
        exit(1);
    }
}

internal b32
handle_track_out_of_memory(i32 val){
    b32 result = 0;
    
    switch (val){
        case FileTrack_OutOfTableMemory:
        {
            u32 new_table_size = shared_vars.track_table_size*2;
            void *new_table = system_get_memory(new_table_size);
            move_track_system(&shared_vars.track, new_table, new_table_size);
            system_free_memory(shared_vars.track_table);
            shared_vars.track_table_size = new_table_size;
            shared_vars.track_table = new_table;
        }break;
        
        case FileTrack_OutOfListenerMemory:
        {
            shared_vars.track_node_size *= 2;
            void *node_expansion = system_get_memory(shared_vars.track_node_size);
            expand_track_system_listeners(&shared_vars.track, node_expansion, shared_vars.track_node_size);
        }break;
        
        default: result = 1; break;
    }
    
    return(result);
}

internal
Sys_Add_Listener_Sig(system_add_listener){
    b32 result = 0;
    
    for (;;){
        i32 track_result = add_listener(&shared_vars.track, filename);
        if (handle_track_out_of_memory(track_result)){
            if (track_result == FileTrack_Good){
                result = 1;
            }
            break;
        }
    }
    
    return(result);
}

internal
Sys_Remove_Listener_Sig(system_remove_listener){
    i32 result = 0;
    i32 track_result = remove_listener(&shared_vars.track, filename);
    if (track_result == FileTrack_Good){
        result = 1;
    }
    return(result);
}

internal
Sys_Get_File_Change_Sig(system_get_file_change){
    i32 result = 0;
    
    i32 size = 0;
    i32 get_result = get_change_event(&shared_vars.track, buffer, max, &size);
    
    *required_size = size;
    *mem_too_small = 0;
    if (get_result == FileTrack_Good){
        result = 1;
    }
    else if (get_result == FileTrack_MemoryTooSmall){
        *mem_too_small = 1;
        result = 1;
    }
    
    return(result);
}

//
// General shared pieces
//

internal File_Data
sysshared_load_file(char *filename){
    File_Data result = {0};
    
    Plat_Handle handle = {0};
    if (system_load_handle(filename, &handle)){
        u32 size = system_load_size(handle);
        
        result.got_file = 1;
        if (size > 0){
            result.size = size;
            result.data = (char*)system_get_memory(size+1);
            
            if (!result.data){
                result = null_file_data;
            }
            else{
                if (!system_load_file(handle, result.data, size)){
                    system_free_memory(result.data);
                    result = null_file_data;
                }
            }
        }
        
        system_load_close(handle);
    }
    
    return(result);
}

internal b32
usable_ascii(char c){
    b32 result = 1;
    if ((c < ' ' || c > '~') && c != '\n' && c != '\r' && c != '\t'){
        result = 0;
    }
    return(result);
}

internal void
sysshared_filter_real_files(char **files, i32 *file_count){
    i32 i, j;
    i32 end;
    
    end = *file_count;
    for (i = 0, j = 0; i < end; ++i){
        if (system_file_can_be_made(files[i])){
            files[j] = files[i];
            ++j;
        }
    }
    *file_count = j;
}

internal Partition
sysshared_scratch_partition(i32 size){
    void *data = system_get_memory(size);
    Partition part = make_part(data, size);
    return(part);
}

internal void
sysshared_partition_grow(Partition *part, i32 new_size){
    void *data = 0;
    if (new_size > part->max){
        // TODO(allen): attempt to grow in place by just acquiring next vpages?!
        data = system_get_memory(new_size);
        memcpy(data, part->base, part->pos);
        system_free_memory(part->base);
        part->base = (char*)data;
    }
}

internal void
sysshared_partition_double(Partition *part){
    sysshared_partition_grow(part, part->max*2);
}

internal void*
sysshared_push_block(Partition *part, i32 size){
    void *result = 0;
    result = push_block(part, size);
    if (!result){
        sysshared_partition_grow(part, size+part->max);
        result = push_block(part, size);
    }
    return(result);
}

internal b32
sysshared_to_binary_path(String *out_filename, char *filename){
    b32 translate_success = 0;
    i32 max = out_filename->memory_size;
    i32 size = system_get_binary_path(out_filename);
    if (size > 0 && size < max-1){
        out_filename->size = size;
        if (append_sc(out_filename, filename) && terminate_with_null(out_filename)){
            translate_success = 1;
        }
    }
    return(translate_success);
}

//
// Rendering
//

inline void
draw_set_clip(Render_Target *target, i32_Rect clip_box){
    glScissor(clip_box.x0, target->height - clip_box.y1, clip_box.x1 - clip_box.x0, clip_box.y1 - clip_box.y0);
}

inline void
draw_bind_texture(Render_Target *target, i32 texid){
    if (target->bound_texture != texid){
        glBindTexture(GL_TEXTURE_2D, texid);
        target->bound_texture = texid;
    }
}

inline void
draw_set_color(Render_Target *target, u32 color){
    if (target->color != color){
        target->color = color;
        Vec4 c = unpack_color4(color);
        glColor4f(c.r, c.g, c.b, c.a);
    }
}

inline void
draw_safe_push(Render_Target *target, i32 size, void *x){
    if (size + target->size <= target->max){
        memcpy(target->push_buffer + target->size, x, size);
        target->size += size;
    }
}

#define PutStruct(s,x) draw_safe_push(target, sizeof(s), &x)

internal void
draw_push_piece(Render_Target *target, Render_Piece_Combined piece){
    if (!target->clip_all){
        PutStruct(Render_Piece_Header, piece.header);
        
        switch (piece.header.type){
            case piece_type_rectangle: case piece_type_outline:
            {
                PutStruct(Render_Piece_Rectangle, piece.rectangle);
            }break;
            
            case piece_type_gradient:
            {
                PutStruct(Render_Piece_Gradient, piece.gradient);
            }break;
            
            case piece_type_glyph: case piece_type_mono_glyph:
            {
                PutStruct(Render_Piece_Glyph, piece.glyph);
            }break;
            
            case piece_type_mono_glyph_advance:
            {
                PutStruct(Render_Piece_Glyph_Advance, piece.glyph_advance);
            }break;
        }
        
        Assert(target->size <= target->max);
    }
}

internal void
draw_push_piece_clip(Render_Target *target, i32_Rect clip_box){
    if (!target->clip_all){
        // TODO(allen): optimize out if there are two clip box changes in a row
        Render_Piece_Change_Clip clip;
        Render_Piece_Header header;
        
        header.type = piece_type_change_clip;
        clip.box = clip_box;
        
        PutStruct(Render_Piece_Header, header);
        PutStruct(Render_Piece_Change_Clip, clip);
    }
}

internal void
draw_push_clip(Render_Target *target, i32_Rect clip_box){
    Assert(target->clip_top == -1 || fits_inside(clip_box, target->clip_boxes[target->clip_top]));
    Assert(target->clip_top+1 < ArrayCount(target->clip_boxes));
    target->clip_boxes[++target->clip_top] = clip_box;
    
    target->clip_all = (clip_box.x0 >= clip_box.x1 || clip_box.y0 >= clip_box.y1);
    draw_push_piece_clip(target, clip_box);
}

internal i32_Rect
draw_pop_clip(Render_Target *target){
    Assert(target->clip_top > 0);
    i32_Rect result = target->clip_boxes[target->clip_top];
    --target->clip_top;
    i32_Rect clip_box = target->clip_boxes[target->clip_top];
    
    target->clip_all = (clip_box.x0 >= clip_box.x1 || clip_box.y0 >= clip_box.y1);
    draw_push_piece_clip(target, clip_box);
    
    return(result);
}

#define ExtractStruct(s) ((s*)cursor); cursor += sizeof(s)

inline void
private_draw_rectangle(Render_Target *target, f32_Rect rect, u32 color){
    draw_set_color(target, color);
    draw_bind_texture(target, 0);
    glBegin(GL_QUADS);
    {
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x0, rect.y1);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x1, rect.y0);
    }
    glEnd();
}

inline void
private_draw_rectangle_outline(Render_Target *target, f32_Rect rect, u32 color){
    f32_Rect r = get_inner_rect(rect, .5f);
    draw_set_color(target, color);
    draw_bind_texture(target, 0);
    glBegin(GL_LINE_STRIP);
    {
        glVertex2f(r.x0, r.y0);
        glVertex2f(r.x1, r.y0);
        glVertex2f(r.x1, r.y1);
        glVertex2f(r.x0, r.y1);
        glVertex2f(r.x0, r.y0);
    }
    glEnd();
}

inline void
private_draw_gradient(Render_Target *target, f32_Rect rect, Vec4 color_left, Vec4 color_right){
    Vec4 cl = color_left;
    Vec4 cr = color_right;
    
    draw_bind_texture(target, 0);
    glBegin(GL_QUADS);
    {
        glColor4f(cl.r, cl.g, cl.b, cl.a);
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x0, rect.y1);
        
        glColor4f(cr.r, cr.g, cr.b, cr.a);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x1, rect.y0);
    }
    glEnd();
}

struct Render_Quad{
    f32 x0, y0, x1, y1;
    f32 s0, t0, s1, t1;
};

inline Render_Quad
get_render_quad(Glyph_Bounds *b, i32 pw, i32 ph, float xpos, float ypos){
    Render_Quad q;
    
    float ipw = 1.0f / pw, iph = 1.0f / ph;
    
    q.x0 = xpos + b->xoff;
    q.y0 = ypos + b->yoff;
    q.x1 = xpos + b->xoff2;
    q.y1 = ypos + b->yoff2;
    
    q.s0 = b->x0 * ipw;
    q.t0 = b->y0 * iph;
    q.s1 = b->x1 * ipw;
    q.t1 = b->y1 * iph;
    
    return(q);
}

inline Render_Quad
get_exact_render_quad(Glyph_Bounds *b, i32 pw, i32 ph, float xpos, float ypos){
    Render_Quad q;
    
    float ipw = 1.0f / pw, iph = 1.0f / ph;
    
    q.x0 = xpos;
    q.y0 = ypos + b->yoff;
    q.x1 = xpos + (b->xoff2 - b->xoff);
    q.y1 = ypos + b->yoff2;
    
    q.s0 = b->x0 * ipw;
    q.t0 = b->y0 * iph;
    q.s1 = b->x1 * ipw;
    q.t1 = b->y1 * iph;
    
    return(q);
}

inline void
private_draw_glyph(System_Functions *system, Render_Target *target, Render_Font *font, u32 codepoint, f32 x, f32 y, u32 color){
    Glyph_Data glyph = font_get_glyph(system, font, codepoint);
    if (glyph.tex != 0){
        Render_Quad q = get_render_quad(&glyph.bounds, glyph.tex_width, glyph.tex_height, x, y);
        
        draw_set_color(target, color);
        draw_bind_texture(target, glyph.tex);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
        }
        glEnd();
    }
}

inline void
private_draw_glyph_mono(System_Functions *system, Render_Target *target, Render_Font *font, u32 codepoint, f32 x, f32 y, f32 advance, u32 color){
    Glyph_Data glyph = font_get_glyph(system, font, codepoint);
    if (glyph.tex != 0){
        f32 left = glyph.bounds.x0;
        f32 right = glyph.bounds.x1;
        f32 width = (right - left);
        f32 x_shift = (advance - width) * .5f;
        
        x += x_shift;
        
        Render_Quad q = get_exact_render_quad(&glyph.bounds, glyph.tex_width, glyph.tex_height, x, y);
        
        draw_set_color(target, color);
        draw_bind_texture(target, glyph.tex);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
        }
        glEnd();
    }
}

inline void
private_draw_glyph_mono(System_Functions *system, Render_Target *target, Render_Font *font, u32 character, f32 x, f32 y, u32 color){
    f32 advance = (f32)font_get_advance(font);
    private_draw_glyph_mono(system, target, font, character, x, y, advance, color);
}

internal void
launch_rendering(System_Functions *system, Render_Target *target){
    char *cursor = target->push_buffer;
    char *cursor_end = cursor + target->size;
    
    for (; cursor < cursor_end;){
        Render_Piece_Header *header = ExtractStruct(Render_Piece_Header);
        
        i32 type = header->type;
        switch (type){
            case piece_type_rectangle:
            {
                Render_Piece_Rectangle *rectangle = ExtractStruct(Render_Piece_Rectangle);
                private_draw_rectangle(target, rectangle->rect, rectangle->color);
            }break;
            
            case piece_type_outline:
            {
                Render_Piece_Rectangle *rectangle = ExtractStruct(Render_Piece_Rectangle);
                private_draw_rectangle_outline(target, rectangle->rect, rectangle->color);
            }break;
            
            case piece_type_gradient:
            {
                Render_Piece_Gradient *gradient = ExtractStruct(Render_Piece_Gradient);
                private_draw_gradient(target, gradient->rect, unpack_color4(gradient->left_color), unpack_color4(gradient->right_color));
            }break;
            
            case piece_type_glyph:
            {
                Render_Piece_Glyph *glyph = ExtractStruct(Render_Piece_Glyph);
                
                Render_Font *font = system->font.get_render_data_by_id(glyph->font_id);
                Assert(font != 0);
                private_draw_glyph(system, target, font, glyph->codepoint, glyph->pos.x, glyph->pos.y, glyph->color);
            }break;
            
            case piece_type_mono_glyph:
            {
                Render_Piece_Glyph *glyph = ExtractStruct(Render_Piece_Glyph);
                
                Render_Font *font = system->font.get_render_data_by_id(glyph->font_id);
                Assert(font != 0);
                private_draw_glyph_mono(system, target, font, glyph->codepoint, glyph->pos.x, glyph->pos.y, glyph->color);
            }break;
            
            case piece_type_mono_glyph_advance:
            {
                Render_Piece_Glyph_Advance *glyph = ExtractStruct(Render_Piece_Glyph_Advance);
                
                Render_Font *font = system->font.get_render_data_by_id(glyph->font_id);
                Assert(font != 0);
                private_draw_glyph_mono(system, target, font, glyph->codepoint, glyph->pos.x, glyph->pos.y, glyph->advance, glyph->color);
            }break;
            
            case piece_type_change_clip:
            {
                Render_Piece_Change_Clip *clip = ExtractStruct(Render_Piece_Change_Clip);
                draw_set_clip(target, clip->box);
            }break;
        }
    }
}

#undef ExtractStruct

// NOTE(allen): Thanks to insofaras.  This is copy-pasted from some work he originally did to get free type working on Linux.

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

internal void
font_load_page_inner(Partition *part, Render_Font *font, FT_Library ft, FT_Face face, b32 use_hinting, Glyph_Page *page, u32 page_number, i32 tab_width){
    Temp_Memory temp = begin_temp_memory(part);
    Assert(page != 0);
    page->page_number = page_number;
    
    // prepare to read glyphs into a temporary texture buffer
    i32 max_glyph_w = face->size->metrics.x_ppem;
    i32 max_glyph_h = font_get_height(font);
    i32 tex_width   = 64;
    i32 tex_height  = 0;
    
    do {
        tex_width *= 2;
        float glyphs_per_row = ceilf(tex_width / (float) max_glyph_w);
        float rows = ceilf(ITEM_PER_FONT_PAGE / glyphs_per_row);
        tex_height = ceil32(rows * (max_glyph_h + 2));
    } while(tex_height > tex_width);
    
    tex_height = round_up_pot_u32(tex_height);
    
    i32 pen_x = 0;
    i32 pen_y = 0;
    
    u32* pixels = push_array(part, u32, tex_width * tex_height);
    memset(pixels, 0, tex_width * tex_height * sizeof(u32));
    
    u32 ft_flags = FT_LOAD_RENDER;
    if (use_hinting){
        // NOTE(inso): FT_LOAD_TARGET_LIGHT does hinting only vertically, which looks nicer imo
        // maybe it could be exposed as an option for hinting, instead of just on/off.
        ft_flags |= FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
    }
    else{
        ft_flags |= (FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_HINTING);
    }
    
    // fill the texture
    u32 base_codepoint = (page_number << 8);
    Glyph_Bounds *glyphs = &page->glyphs[0];
    Glyph_Bounds *glyph_ptr = glyphs;
    
    f32 *advances = &page->advance[0];
    f32 *advance_ptr = advances;
    for(u32 i = 0; i < ITEM_PER_FONT_PAGE; ++i, ++glyph_ptr, ++advance_ptr){
        u32 codepoint = i + base_codepoint;
        
        if(FT_Load_Char(face, codepoint, ft_flags) == 0){
            i32 w = face->glyph->bitmap.width;
            i32 h = face->glyph->bitmap.rows;
            
            i32 ascent = font_get_ascent(font);
            
            // move to next line if necessary
            if(pen_x + w >= tex_width){
                pen_x = 0;
                pen_y += (max_glyph_h + 2);
            }
            
            // set all this stuff the renderer needs
            glyph_ptr->x0 = (f32)(pen_x);
            glyph_ptr->y0 = (f32)(pen_y);
            glyph_ptr->x1 = (f32)(pen_x + w);
            glyph_ptr->y1 = (f32)(pen_y + h + 1);
            
            glyph_ptr->xoff = (f32)(face->glyph->bitmap_left);
            glyph_ptr->yoff = (f32)(ascent - face->glyph->bitmap_top);
            glyph_ptr->xoff2 = glyph_ptr->xoff + w;
            glyph_ptr->yoff2 = glyph_ptr->yoff + h + 1;
            
            // TODO(allen): maybe advance data should be integers?
            *advance_ptr = (f32)ceil32(face->glyph->advance.x / 64.0f);
            
            // write to texture atlas
            i32 pitch = face->glyph->bitmap.pitch;
            for(i32 Y = 0; Y < h; ++Y){
                for(i32 X = 0; X < w; ++X){
                    i32 x = pen_x + X;
                    i32 y = pen_y + Y;
                    
                    pixels[y * tex_width + x] = face->glyph->bitmap.buffer[Y * pitch + X] * 0x01010101;
                }
            }
            
            pen_x = ceil32(glyph_ptr->x1 + 1);
        }
    }
    
    // upload texture
    tex_height = round_up_pot_u32(pen_y + max_glyph_h + 2);
    
    page->tex_width  = tex_width;
    page->tex_height = tex_height;
    
    glGenTextures(1, &page->tex);
    glBindTexture(GL_TEXTURE_2D, page->tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_INT, pixels);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    end_temp_memory(temp);
    
    // whitespace spacing stuff
    if (page_number == 0){
        f32 space_adv = advances[' '];
        f32 backslash_adv = advances['\\'];
        f32 r_adv = advances['r'];
        
        advances['\n'] = space_adv;
        advances['\r'] = backslash_adv + r_adv;
        advances['\t'] = space_adv*tab_width;
    }
}

internal b32
font_load_page(System_Functions *system, Partition *part, Render_Font *font, Glyph_Page *page, u32 page_number, u32 pt_size,  b32 use_hinting){
    
    char *filename = font->filename;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    // NOTE(allen): set texture and glyph data.
    font_load_page_inner(part, font, ft, face, use_hinting, page, page_number, 4);
    
    FT_Done_FreeType(ft);
    
    return(true);
}

internal b32
font_load(System_Functions *system, Partition *part, Render_Font *font, i32 pt_size, b32 use_hinting){
    
    char *filename = font->filename;
    
    // TODO(allen): Stop redoing all this init for each call.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);
    
    // set size & metrics
    FT_Size_RequestRec_ size = {};
    size.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = pt_size << 6;
    FT_Request_Size(face, &size);
    
    font->ascent    = ceil32  (face->size->metrics.ascender    / 64.0f);
    font->descent   = floor32 (face->size->metrics.descender   / 64.0f);
    font->advance   = ceil32  (face->size->metrics.max_advance / 64.0f);
    font->height    = ceil32  (face->size->metrics.height      / 64.0f);
    font->line_skip = font->height - (font->ascent - font->descent);
    
    font->height -= font->line_skip;
    font->line_skip = 0;
    
    // NOTE(allen): set texture and glyph data.
    Glyph_Page *page = font_get_or_make_page(system, font, 0);
    
    // NOTE(allen): Setup some basic spacing stuff.
    f32 backslash_adv = page->advance['\\'];
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
    
    font->byte_advance = backslash_adv + max_hex_advance*2;
    font->byte_sub_advances[0] = backslash_adv;
    font->byte_sub_advances[1] = max_hex_advance;
    font->byte_sub_advances[2] = max_hex_advance;
    
    FT_Done_FreeType(ft);
    
    return(true);
}

internal void
system_set_page(System_Functions *system, Partition *part, Render_Font *font, Glyph_Page *page, u32 page_number, u32 pt_size, b32 use_hinting){
    memset(page, 0, sizeof(*page));
    
    if (part->base == 0){
        *part = sysshared_scratch_partition(MB(8));
    }
    
    b32 success = false;
    for (u32 R = 0; R < 3; ++R){
        success = font_load_page(system, part, font, page, page_number, pt_size, use_hinting);
        if (success){
            break;
        }
        else{
            sysshared_partition_double(part);
        }
    }
}

internal void
system_set_font(System_Functions *system, Partition *part, Render_Font *font, String filename, String name, u32 pt_size, b32 use_hinting){
    memset(font, 0, sizeof(*font));
    
    copy_partial_cs(font->filename, sizeof(font->filename)-1, filename);
    font->filename_len = filename.size;
    font->filename[font->filename_len] = 0;
    copy_partial_cs(font->name, sizeof(font->name)-1, name);
    font->name_len = name.size;
    font->name[font->name_len] = 0;
    
    if (part->base == 0){
        *part = sysshared_scratch_partition(MB(8));
    }
    
    b32 success = false;
    for (u32 R = 0; R < 3; ++R){
        success = font_load(system, part, font, pt_size, use_hinting);
        if (success){
            break;
        }
        else{
            sysshared_partition_double(part);
        }
    }
}


#endif

// BOTTOM

