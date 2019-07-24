/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 16.11.2017
 *
 * Data types for the freetype font provider.
 *
 */

// TOP

#if !defined(FCODER_FONT_PROVIDER_FREETYPE_H)
#define FCODER_FONT_PROVIDER_FREETYPE_H

struct FT_Codepoint_Index_Pair{
    u32 codepoint;
    u16 index;
};

struct FT_Codepoint_Index_Pair_Array{
    FT_Codepoint_Index_Pair *vals;
    i32 count;
};

#endif

// BOTTOM

