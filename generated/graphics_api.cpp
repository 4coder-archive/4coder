function void
graphics_api_fill_vtable(API_VTable_graphics *vtable){
vtable->get_texture = graphics_get_texture;
vtable->fill_texture = graphics_fill_texture;
}
#if defined(DYNAMIC_LINK_API)
function void
graphics_api_read_vtable(API_VTable_graphics *vtable){
graphics_get_texture = vtable->get_texture;
graphics_fill_texture = vtable->fill_texture;
}
#undef DYNAMIC_LINK_API
#endif
