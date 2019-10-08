/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.10.2019
 *
 * Graphics API definition program.
 *
 */

// TOP

#include "4ed_api_definition_main.cpp"

function API_Definition*
define_api(Arena *arena){
    API_Definition *api = begin_api(arena, "graphics");
    
    {
        API_Call *call = api_call(arena, api, "get_texture", "u32");
        api_param(arena, call, "Vec3_i32", "dim");
        api_param(arena, call, "Texture_Kind", "texture_kind");
    }
    
    {
        API_Call *call = api_call(arena, api, "fill_texture", "b32");
        api_param(arena, call, "Texture_Kind", "texture_kind");
        api_param(arena, call, "u32", "texture");
        api_param(arena, call, "Vec3_i32", "p");
        api_param(arena, call, "Vec3_i32", "dim");
        api_param(arena, call, "void*", "data");
    }
    
    return(api);
}

function Generated_Group
get_api_group(void){
    return(GeneratedGroup_Core);
}

// BOTTOM

