/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Font system interface.
 *
 */

// TOP

#if !defined(FCODER_FONT_H)
#define FCODER_FONT_H

// NOTE(allen): A description of an available font.
struct Font_Loadable_Stub{
    b32 in_local_folder;
    i32 len;
    char name[256];
};

// NOTE(allen): Settings that the are specified that determine how a font should be loaded and rendered.
struct Font_Settings{
    Font_Loadable_Stub stub;
    i32 pt_size;
    b32 use_hinting;
};

// NOTE(allen): Results about the font true for the entire font as a whole.
struct Font_Metrics{
    i32 name_len;
    char name[256];
    
    i32 height;
    i32 ascent;
    i32 descent;
    i32 line_skip;
    i32 advance;
    
    f32 byte_advance;
    f32 sub_advances[3];
};

// NOTE(allen): The pages of glyph data.
#define GLYPHS_PER_PAGE 256

struct Glyph_Bounds{
    f32 x0, x1;
    f32 y0, y1;
    f32 xoff, yoff;
    f32 xoff2, yoff2;
};
global_const Glyph_Bounds null_glyph_bounds = {0};

struct Glyph_Page{
    u32 page_number;
    
    b32 has_layout;
    f32 advance[GLYPHS_PER_PAGE];
    Glyph_Bounds glyphs[GLYPHS_PER_PAGE];
    i32 tex_width, tex_height;
    
    b32 has_gpu_setup;
    u32 gpu_tex;
};

#define FONT_PAGE_EMPTY   ((Glyph_Page*)0)
#define FONT_PAGE_DELETED ((Glyph_Page*)(1))
#define FONT_PAGE_MAX     0x1100

struct Font_Page_Storage{
    Glyph_Page **pages;
    u32 page_count;
    u32 page_max;
    
    // Hack optimizations
    struct Page_Cache{
        u32 page_number;
        Glyph_Page *page;
    };
    
    Page_Cache cache[16];
};

// NOTE(allen): Types of refernces fonts.
struct Font_Pointers{
    b32 valid;
    Font_Settings *settings;
    Font_Metrics *metrics;
    Font_Page_Storage *pages;
};

typedef u32 Font_ID;

// NOTE(allen): Platform layer calls - implemented in a "font provider"
#define Sys_Font_Get_Count_Sig(n) u32 (n)(void)
typedef Sys_Font_Get_Count_Sig(Font_Get_Count_Function);

#define Sys_Font_Get_Name_By_ID_Sig(n, str_out, capacity) i32 (n)(Font_ID font_id, char *str_out, u32 capacity)
typedef Sys_Font_Get_Name_By_ID_Sig(Font_Get_Name_By_ID_Function, str_out, capacity);

#define Sys_Font_Get_Pointers_By_ID_Sig(n,font_id) Font_Pointers (n)(Font_ID font_id)
typedef Sys_Font_Get_Pointers_By_ID_Sig(Font_Get_Pointers_By_ID_Function, font_id);

#define Sys_Font_Load_Page_Sig(n,s,m,p,pn) void (n)(Font_Settings *s, Font_Metrics *m, Glyph_Page *p, u32 pn)
typedef Sys_Font_Load_Page_Sig(Font_Load_Page_Function, settings, metrics, page, page_number);

#define Sys_Font_Allocate_Sig(n) void* (n)(i32 size)
typedef Sys_Font_Allocate_Sig(Font_Allocate_Function);

#define Sys_Font_Free_Sig(n) void (n)(void *ptr)
typedef Sys_Font_Free_Sig(Font_Free_Function);

struct Font_Functions{
    Font_Get_Count_Function *get_count;
    Font_Get_Name_By_ID_Function *get_name_by_id;
    Font_Get_Pointers_By_ID_Function *get_pointers_by_id;
    Font_Load_Page_Function *load_page;
    Font_Allocate_Function *allocate;
    Font_Free_Function *free;
};

#endif

// BOTTOM


