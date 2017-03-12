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

#define Sys_Font_Get_Count_Sig(name_) u32 (name_)(void)
typedef Sys_Font_Get_Count_Sig(Font_Get_Count_Function);

#define Sys_Font_Get_IDs_By_Index_Sig(name_) b32 (name_)(u32 first_index, u32 index_count, u32 *id_out)
typedef Sys_Font_Get_IDs_By_Index_Sig(Font_Get_IDs_By_Index_Function);

#define Sys_Font_Get_Name_By_Index_Sig(name_) u32 (name_)(u32 font_index, char *str_out, u32 str_out_cap)
typedef Sys_Font_Get_Name_By_Index_Sig(Font_Get_Name_By_Index_Function);

#define Sys_Font_Get_Name_By_ID_Sig(name_) u32 (name_)(u32 font_index, char *str_out, u32 str_out_cap)
typedef Sys_Font_Get_Name_By_ID_Sig(Font_Get_Name_By_ID_Function);

#define Sys_Font_Get_Render_Data_By_ID_Sig(name_) Render_Font* (name_)(u32 font_id)
typedef Sys_Font_Get_Render_Data_By_ID_Sig(Font_Get_Render_Data_By_ID_Function);

struct Font_Functions{
    Font_Get_Count_Function *get_count;
    Font_Get_IDs_By_Index_Function *get_ids_by_index;
    Font_Get_Name_By_Index_Function *get_name_by_index;
    Font_Get_Name_By_ID_Function *get_name_by_id;
    Font_Get_Render_Data_By_ID_Function *get_render_data_by_id;
};

internal f32 font_get_byte_advance(Render_Font *font);
internal i32 font_get_height(Render_Font *font);
internal i32 font_get_ascent(Render_Font *font);
internal i32 font_get_descent(Render_Font *font);
internal i32 font_get_line_skip(Render_Font *font);
internal i32 font_get_advance(Render_Font *font);

#endif

// BOTTOM


