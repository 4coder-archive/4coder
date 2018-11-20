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
    b32 load_from_path;
    b32 in_font_folder;
    i32 len;
    char name[256];
};

struct Font_Loadable_Description{
    b32 valid;
    Font_Loadable_Stub stub;
    i32 display_len;
    char display_name[64];
};

// NOTE(allen): Settings that the are specified that determine how a font should be loaded and rendered.
struct Font_Parameters{
    i32 pt_size;
    b32 italics;
    b32 bold;
    b32 underline;
    b32 use_hinting;
};

struct Font_Settings{
    Font_Loadable_Stub stub;
    Font_Parameters parameters;
};

// NOTE(allen): Data about the font true for the entire font as a whole.
struct Font_Metrics{
    i32 name_len;
    char name[256];
    
    i32 height;
    i32 ascent;
    i32 descent;
    i32 line_skip;
    i32 advance;
    
    f32 underline_yoff1;
    f32 underline_yoff2;
    
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
global_const Glyph_Bounds null_glyph_bounds = {};

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

// NOTE(allen): Platform layer calls - implemented in a "font provider"
#define Sys_Font_Get_Loadable_Count_Sig(n) i32 (n)(void)
typedef Sys_Font_Get_Loadable_Count_Sig(Font_Get_Loadable_Count_Function);

#define Sys_Font_Get_Loadable_Sig(n,i,o) void (n)(i32 i, Font_Loadable_Description *o)
typedef Sys_Font_Get_Loadable_Sig(Font_Get_Loadable_Function, index, out);

#define Sys_Font_Face_Allocate_And_Init_Sig(n,s) Face_ID (n)(Font_Settings *s)
typedef Sys_Font_Face_Allocate_And_Init_Sig(Font_Face_Allocate_And_Init_Function, settings);

#define Sys_Font_Face_Change_Settings_Sig(n,id,s) b32 (n)(Face_ID id, Font_Settings *s)
typedef Sys_Font_Face_Change_Settings_Sig(Font_Face_Change_Settings_Function, font_id, new_settings);

#define Sys_Font_Face_Release_Sig(n,id) b32 (n)(Face_ID id)
typedef Sys_Font_Face_Release_Sig(Font_Face_Release_Function, font_id);

#define Sys_Font_Get_Largest_ID_Sig(n) Face_ID (n)(void)
typedef Sys_Font_Get_Largest_ID_Sig(Font_Get_Largest_ID_Function);

#define Sys_Font_Get_Count_Sig(n) i32 (n)(void)
typedef Sys_Font_Get_Count_Sig(Font_Get_Count_Function);

#define Sys_Font_Get_Name_By_ID_Sig(n, font_id, out, cap) i32 (n)(Face_ID font_id, char *out, u32 cap)
typedef Sys_Font_Get_Name_By_ID_Sig(Font_Get_Name_By_ID_Function, font_id, out, cap);

#define Sys_Font_Get_Pointers_By_ID_Sig(n, font_id) Font_Pointers (n)(Face_ID font_id)
typedef Sys_Font_Get_Pointers_By_ID_Sig(Font_Get_Pointers_By_ID_Function, font_id);

#define Sys_Font_Load_Page_Sig(n,s,m,p,pn) void (n)(Font_Settings *s, Font_Metrics *m, Glyph_Page *p, u32 pn)
typedef Sys_Font_Load_Page_Sig(Font_Load_Page_Function, settings, metrics, page, page_number);

#define Sys_Font_Allocate_Sig(n,size) void* (n)(i32 size)
typedef Sys_Font_Allocate_Sig(Font_Allocate_Function,size);

#define Sys_Font_Free_Sig(n,ptr) void (n)(void *ptr)
typedef Sys_Font_Free_Sig(Font_Free_Function,ptr);

struct Font_Functions{
    Font_Get_Loadable_Count_Function *get_loadable_count;
    Font_Get_Loadable_Function *get_loadable;
    Font_Face_Allocate_And_Init_Function *face_allocate_and_init;
    Font_Face_Change_Settings_Function *face_change_settings;
    Font_Face_Release_Function *face_release;
    Font_Get_Largest_ID_Function *get_largest_id;
    Font_Get_Count_Function *get_count;
    Font_Get_Name_By_ID_Function *get_name_by_id;
    Font_Get_Pointers_By_ID_Function *get_pointers_by_id;
    Font_Load_Page_Function *load_page;
    Font_Allocate_Function *allocate;
    Font_Free_Function *free;
};

#endif

// BOTTOM


