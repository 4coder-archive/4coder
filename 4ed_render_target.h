/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render target type definition
 *
 */

// TOP

#if !defined(FRED_RENDER_TARGET_H)
#define FRED_RENDER_TARGET_H

struct Render_Free_Texture{
    Render_Free_Texture *next;
    u32 tex_id;
};

struct Render_Target{
    i32_Rect clip_boxes[5];
    i32 clip_top;
    b8 clip_all;
    b8 out_of_memory;
    i32 width;
    i32 height;
    i32 bound_texture;
    u32 color;
    
    i32 frame_index;
    f32 literal_dt;
    f32 animation_dt;
    
    Render_Free_Texture *free_texture_first;
    Render_Free_Texture *free_texture_last;
    
    // TODO(allen): rewrite render system to work on an arena
    Cursor buffer;
};

#endif

// BOTTOM

