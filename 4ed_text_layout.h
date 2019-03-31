/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 31.03.2019
 *
 * Implementation of the API functions.
 *
 */

// TOP

#if !defined(FRED_TEXT_LAYOUT_H)
#define FRED_TEXT_LAYOUT_H

struct Text_Layout{
    // NOTE(allen): This is not a _real_ text layout yet.
    // The eventual destiny of this type is that it will store the fairly
    // costly to generate results of the text layout engine.
    // For now, since the engine cannot be easily consolidated,
    // this just stores the parameters that should be handed to any
    // system that attempts to query the layout for hit testing.
    Buffer_ID buffer_id;
    i32 line;
    Vec2 pixel_shift;
};

#endif

// BOTTOM

