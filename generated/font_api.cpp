function void
font_api_fill_vtable(API_VTable_font *vtable){
vtable->make_face = font_make_face;
}
#if defined(DYNAMIC_LINK_API)
function void
font_api_read_vtable(API_VTable_font *vtable){
font_make_face = vtable->make_face;
}
#undef DYNAMIC_LINK_API
#endif
