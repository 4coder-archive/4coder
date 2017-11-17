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

// NOTE(allen): Implemented by the freetype font provider.
struct Font_Slot{
    b32 is_active;
    Font_Settings settings;
    Font_Metrics metrics;
    Font_Page_Storage pages;
};

struct Font_Vars{
    Font_Slot slots[32];
    i32 count;
    
    Font_Loadable_Description loadables[4096];
    i32 loadable_count;
    
    u32 pt_size;
    b32 use_hinting;
};

global Font_Vars fontvars = {0};

struct Font_Setup{
    Font_Setup *next;
    Font_Loadable_Stub stub;
    
    b32 has_display_name;
    i32 len;
    char name[64];
};

struct Font_Setup_List{
    Font_Setup *first;
    Font_Setup *last;
};

// NOTE(allen): Procedures to be implemented per-OS for the freetype font provider.
struct Font_Raw_Data{
    Temp_Memory temp;
    u8 *data;
    i32 size;
};

#define Sys_Font_Data(name) Font_Raw_Data system_font_data(char *name)
internal Sys_Font_Data(name);

#define Sys_Font_Data_Not_Used \
internal Sys_Font_Data(name){Font_Raw_Data data = {0}; LOG("there is no font data retrieval procedure available\n"); return(data);}

#endif

// BOTTOM

