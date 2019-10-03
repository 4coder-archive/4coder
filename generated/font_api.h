#define font_make_face_sig() Face* font_make_face(Arena* arena, Face_Description* description, f32 scale_factor)
typedef Face* font_make_face_type(Arena* arena, Face_Description* description, f32 scale_factor);
struct API_VTable_font{
font_make_face_type *make_face;
};
#if defined(STATIC_LINK_API)
internal Face* font_make_face(Arena* arena, Face_Description* description, f32 scale_factor);
#undef STATIC_LINK_API
#elif defined(DYNAMIC_LINK_API)
global font_make_face_type *font_make_face = 0;
#undef DYNAMIC_LINK_API
#endif
