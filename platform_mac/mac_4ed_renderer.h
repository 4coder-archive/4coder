/* Mac Renderer Abstraction */

#if !defined(FRED_MAC_RENDERER_H)
#define FRED_MAC_RENDERER_H

////////////////////////////////

// TODO(yuval): This should be refactored into a platform independent renderer

struct Mac_Renderer;

#define mac_render_sig(name) void name(Mac_Renderer *renderer, Render_Target *target)
typedef mac_render_sig(mac_render_type);

#define mac_get_texture_sig(name) u32 name(Mac_Renderer *renderer, Vec3_i32 dim, Texture_Kind texture_kind)
typedef mac_get_texture_sig(mac_get_texture_type);

#define mac_fill_texture_sig(name) b32 name(Mac_Renderer *renderer, Texture_Kind texture_kind, u32 texture, Vec3_i32 p, Vec3_i32 dim, void* data)
typedef mac_fill_texture_sig(mac_fill_texture_type);

typedef i32 Mac_Renderer_Kind;
enum{
    MacRenderer_OpenGL,
    MacRenderer_Metal,
    //
    MacRenderer_COUNT
};

struct Mac_Renderer{
    mac_render_type *render;
    
    mac_get_texture_type *get_texture;
    mac_fill_texture_type *fill_texture;
};

////////////////////////////////

// NOTE(yuval): This is the actual platform dependent function that each renderer implementation implements and should be exported into a DLL
#define mac_load_renderer_sig(name) Mac_Renderer* name(NSWindow *window, Render_Target *target)
typedef mac_load_renderer_sig(mac_load_renderer_type);

////////////////////////////////

#endif