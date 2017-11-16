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

struct Font_Slot{
    b32 is_active;
    Font_Settings settings;
    Font_Metrics metrics;
    Font_Page_Storage pages;
};

struct Font_Vars{
    Font_Slot slots[32];
    u32 count;
};

global Font_Vars fontvars = {0};

struct Font_Setup{
    Font_Setup *next_font;
    Font_Loadable_Stub stub;
};

#endif

// BOTTOM

