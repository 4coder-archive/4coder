/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.17.2014
 *
 * Rendering layer for project codename "4ed"
 *
 */

#ifndef FRED_RENDERING_H
#define FRED_RENDERING_H

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#if SOFTWARE_RENDER
struct Glyph_Data{
	void *data;
	i32 width, height;
	i32 minx, maxx, miny, maxy;
    i32 left_shift;
    bool32 exists;
};

struct Font{
	Glyph_Data glyphs[128];
	i32 height, ascent, descent, line_skip;
    i32 advance;
};
#else
struct Glyph_Data{
#if 0
	i32 width, height;
	i32 minx, maxx, miny, maxy;
    i32 left_shift;
#endif
    bool32 exists;
};

struct Font{
    char name_[24];
    String name;
    bool32 loaded;
    
	Glyph_Data glyphs[256];
    stbtt_bakedchar chardata[256];
    float advance_data[256];
	i32 height, ascent, descent, line_skip;
    i32 advance;
    u32 tex;
    i32 tex_width, tex_height;
};
#endif

#endif
