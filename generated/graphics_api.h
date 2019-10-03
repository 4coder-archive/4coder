#define graphics_get_texture_sig() u32 graphics_get_texture(Vec3_i32 dim, Texture_Kind texture_kind)
#define graphics_fill_texture_sig() b32 graphics_fill_texture(Texture_Kind texture_kind, u32 texture, Vec3_i32 p, Vec3_i32 dim, void* data)
typedef u32 graphics_get_texture_type(Vec3_i32 dim, Texture_Kind texture_kind);
typedef b32 graphics_fill_texture_type(Texture_Kind texture_kind, u32 texture, Vec3_i32 p, Vec3_i32 dim, void* data);
struct API_VTable_graphics{
graphics_get_texture_type *get_texture;
graphics_fill_texture_type *fill_texture;
};
#if defined(STATIC_LINK_API)
internal u32 graphics_get_texture(Vec3_i32 dim, Texture_Kind texture_kind);
internal b32 graphics_fill_texture(Texture_Kind texture_kind, u32 texture, Vec3_i32 p, Vec3_i32 dim, void* data);
#undef STATIC_LINK_API
#elif defined(DYNAMIC_LINK_API)
global graphics_get_texture_type *graphics_get_texture = 0;
global graphics_fill_texture_type *graphics_fill_texture = 0;
#undef DYNAMIC_LINK_API
#endif
