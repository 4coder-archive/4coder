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

struct Render_Target{
    i32_Rect clip_boxes[5];
    i32 clip_top;
    b32 clip_all;
    i32 width, height;
    i32 bound_texture;
    u32 color;
    
    Partition buffer;
};

#endif

// BOTTOM

