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
    void *handle;
    void *context;
    i32_Rect clip_boxes[5];
    i32 clip_top;
    b32 clip_all;
    i32 width, height;
    i32 bound_texture;
    u32 color;
    
    // TODO(allen): change this to a Partition
    char *push_buffer;
    i32 size, max;
};

#endif

// BOTTOM

