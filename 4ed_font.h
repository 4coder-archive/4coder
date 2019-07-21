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

struct Font_Loadable_Stub{
    b32 load_from_path;
    b32 in_font_folder;
    i32 len;
    char name[256];
};

struct Face_Parameters{
    i32 pt_size;
    b32 italics;
    b32 bold;
    b32 underline;
    b32 use_hinting;
};

struct Face_Settings{
    Font_Loadable_Stub stub;
    Face_Parameters parameters;
};

struct Glyph_Bounds{
    Rect_f32 uv;
    f32 w;
    Rect_f32 xy_off;
};

typedef i32 Texture_Kind;

struct Face{
    Face_Settings settings;
    
    // NOTE(allen): Metrics
    String_Const_u8 name;
    
    f32 height;
    f32 ascent;
    f32 descent;
    f32 line_skip;
    f32 advance;
    
    f32 underline_yoff1;
    f32 underline_yoff2;
    
    f32 byte_advance;
    f32 sub_advances[3];
    
    // NOTE(allen): Glyph data
    Table_u32_u16 codepoint_to_index_table;
    i32 index_count;
    Glyph_Bounds *bounds;
    Glyph_Bounds white;
    
    Texture_Kind gpu_texture_kind;
    u32 gpu_texture;
    Vec3_f32 gpu_texture_dim;
};

////////////////////////////////

typedef u32 Get_GPU_Texture_Function(Vec3_i32 dim, Texture_Kind texture_kind);
typedef b32 Fill_GPU_Texture_Function(Texture_Kind texture_kind, u32 gpu_texture,
                                      Vec3_i32 p, Vec3_i32 dim, void *data);

// NOTE(allen): Platform layer calls - implemented in a "font provider"
typedef Face Font_Make_Face_Function(Arena *arena, Face_Settings *settings, 
                                     Get_GPU_Texture_Function *get_gpu_texture,
                                     Fill_GPU_Texture_Function *fill_gpu_texture);

#endif

// BOTTOM


