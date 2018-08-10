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

#include "4ed_generated_style.h"

struct Style_Font{
    Face_ID font_id;
};

struct Style{
    char name_[24];
    String name;
    Style_Main_Data main;
};

internal void
style_copy(Style *dst, Style *src){
    *dst = *src;
    dst->name.str = dst->name_;
}

internal void
style_set_name(Style *style, String name){
    i32 count = ArrayCount(style->name_);
    style->name = make_string_cap(style->name_, 0, count - 1);
    copy_ss(&style->name, name);
    terminate_with_null(&style->name);
}

struct Style_Library{
    Style styles[64];
    i32 count;
    i32 max;
};

#endif

// BOTTOM

