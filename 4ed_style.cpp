/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.08.2015
 *
 * Styles for 4coder
 *
 */

// TOP

struct Style_Font{
    Font_ID font_id;
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
    i32 count, max;
};

internal void
style_set_colors(Style *style, Theme *theme){
    for (u32 i = 0; i < Stag_COUNT; ++i){
        u32 *color_ptr = style_index_by_tag(&style->main, i);
        *color_ptr = theme->colors[i];
    }
}

internal void
style_add(Style_Library *library, Theme *theme, String name){
    if (library->count < library->max){
        Style *style = &library->styles[library->count++];
        style_set_colors(style, theme);
        style_set_name(style, name);
    }
}

// BOTTOM

