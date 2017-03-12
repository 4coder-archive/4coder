/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Implements some basic getters for fonts set up to make the font type opaque.
 *
 */

// TOP

#include "4coder_font_data.h"

internal f32
font_get_byte_advance(Render_Font *font){
    return(font->byte_advance);
}

internal i32
font_get_height(Render_Font *font){
    return(font->height);
}

internal i32
font_get_ascent(Render_Font *font){
    return(font->ascent);
}

internal i32
font_get_descent(Render_Font *font){
    return(font->descent);
}

internal i32
font_get_line_skip(Render_Font *font){
    return(font->line_skip);
}

internal i32
font_get_advance(Render_Font *font){
    return(font->advance);
}

// BOTTOM



