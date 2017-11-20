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

struct Font_Slot_Page{
    Font_Slot_Page *next;
    Font_Slot_Page *prev;
    
    u64 *is_active;
    Font_Settings *settings;
    Font_Metrics *metrics;
    Font_Page_Storage *pages;
    
    i32 used_count;
    i32 fill_count;
    i32 max;
    Font_ID first_id;
};

struct Font_Slot_Page_And_Index{
    Font_Slot_Page *page;
    i32 index;
};

// NOTE(allen): SLOT_PER_PAGE must be >= 1
global int32_t SLOT_PER_PAGE = 32;
global int32_t SLOT_SIZE = sizeof(Font_Settings) + sizeof(Font_Metrics) + sizeof(Font_Page_Storage);
global int32_t SLOT_PAGE_SIZE = sizeof(Font_Slot_Page) + ((SLOT_PER_PAGE + 63)/64)*8 + SLOT_PER_PAGE*SLOT_SIZE;

struct Font_Vars{
    Font_Slot_Page slot_pages_sentinel;
    i32 used_slot_count;
    i32 max_slot_count;
    Font_ID largest_font_id;
    
    // HACK(allen): // HACK(allen): // HACK(allen): 
    // TODO(allen): Upgrade this to have "unlimited" resizable memory.
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

