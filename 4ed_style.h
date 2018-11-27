/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.08.2015
 *
 * Styles for 4coder
 *
 */

// TOP

#if !defined(FRED_STYLE_H)
#define FRED_STYLE_H

struct Style_Font{
    Face_ID font_id;
};

struct Style{
    char name_[32];
    String name;
    Theme theme;
};

struct Style_Library{
    Style styles[64];
    i32 count;
    i32 max;
};

#endif

// BOTTOM

