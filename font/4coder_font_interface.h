/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Font system interface.
 *
 */

// TOP

#if !defined(FCODER_FONT_INTERFACE_H)
#define FCODER_FONT_INTERFACE_H

typedef u32 Font_ID;

struct Render_Font;
struct Glyph_Page;

#define Sys_Font_Get_Count_Sig(name_) u32 (name_)(void)
typedef Sys_Font_Get_Count_Sig(Font_Get_Count_Function);

#define Sys_Font_Get_IDs_By_Index_Sig(name_) b32 (name_)(u32 first_index, u32 index_count, u32 *id_out)
typedef Sys_Font_Get_IDs_By_Index_Sig(Font_Get_IDs_By_Index_Function);

#define Sys_Font_Get_Name_By_Index_Sig(name_) u32 (name_)(u32 font_index, char *str_out, u32 str_out_cap)
typedef Sys_Font_Get_Name_By_Index_Sig(Font_Get_Name_By_Index_Function);

#define Sys_Font_Get_Name_By_ID_Sig(name_) u32 (name_)(u32 font_id, char *str_out, u32 str_out_cap)
typedef Sys_Font_Get_Name_By_ID_Sig(Font_Get_Name_By_ID_Function);

#define Sys_Font_Get_Render_Data_By_ID_Sig(name_) Render_Font* (name_)(u32 font_id)
typedef Sys_Font_Get_Render_Data_By_ID_Sig(Font_Get_Render_Data_By_ID_Function);

#define Sys_Font_Load_Page_Sig(name_) void (name_)(Render_Font *font, Glyph_Page *page, u32 page_number)
typedef Sys_Font_Load_Page_Sig(Font_Load_Page_Function);

#define Sys_Font_Allocate_Sig(name_) void* (name_)(i32 size)
typedef Sys_Font_Allocate_Sig(Font_Allocate_Function);

#define Sys_Font_Free_Sig(name_) void (name_)(void *ptr)
typedef Sys_Font_Free_Sig(Font_Free_Function);

struct Font_Functions{
    Font_Get_Count_Function *get_count;
    Font_Get_IDs_By_Index_Function *get_ids_by_index;
    Font_Get_Name_By_Index_Function *get_name_by_index;
    Font_Get_Name_By_ID_Function *get_name_by_id;
    Font_Get_Render_Data_By_ID_Function *get_render_data_by_id;
    Font_Load_Page_Function *load_page;
    
    Font_Allocate_Function *allocate;
    Font_Free_Function *free;
};

internal f32 font_get_byte_advance(Render_Font *font);
internal f32*font_get_byte_sub_advances(Render_Font *font);
internal i32 font_get_height(Render_Font *font);
internal i32 font_get_ascent(Render_Font *font);
internal i32 font_get_descent(Render_Font *font);
internal i32 font_get_line_skip(Render_Font *font);
internal i32 font_get_advance(Render_Font *font);

internal b32 font_can_render(struct System_Functions *system, Render_Font *font, u32 codepoint);
internal f32 font_get_glyph_advance(struct System_Functions *system, Render_Font *font, u32 codepoint);

struct Glyph_Data;
internal Glyph_Data font_get_glyph(System_Functions *system, Render_Font *font, u32 codepoint);

internal Glyph_Page *font_get_or_make_page(struct System_Functions *system, Render_Font *font, u32 page_number);

#endif

// BOTTOM


