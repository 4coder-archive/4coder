/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render target type definition
 *
 */

// TOP

#if !defined(FRED_RENDER_TARGET_H)
#define FRED_RENDER_TARGET_H

struct Render_Free_Texture{
    Render_Free_Texture *next;
    u32 tex_id;
};

struct Render_Vertex{
    Vec2_f32 xy;
    Vec3_f32 uvw;
    u32 color;
    f32 half_thickness;
};

struct Render_Vertex_Array_Node{
    Render_Vertex_Array_Node *next;
    Render_Vertex *vertices;
    i32 vertex_count;
    i32 vertex_max;
};

struct Render_Vertex_List{
    Render_Vertex_Array_Node *first;
    Render_Vertex_Array_Node *last;
    i32 vertex_count;
};

struct Render_Group{
    Render_Group *next;
    Render_Vertex_List vertex_list;
    // parameters
    Face_ID face_id;
    Rect_f32 clip_box;
};

struct Render_Target{
    b8 clip_all;
    i32 width;
    i32 height;
    i32 bound_texture;
    u32 color;
    
    i32 frame_index;
    f32 literal_dt;
    f32 animation_dt;
    
    Render_Free_Texture *free_texture_first;
    Render_Free_Texture *free_texture_last;
    
    Arena arena;
    Render_Group *group_first;
    Render_Group *group_last;
    i32 group_count;
    
    Face_ID current_face_id;
    Rect_f32 current_clip_box;
    void *font_set;
    u32 fallback_texture_id;
};

#endif

// BOTTOM

