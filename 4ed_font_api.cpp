/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.10.2019
 *
 * Font API definition program.
 *
 */

// TOP

#include "4ed_api_definition_main.cpp"

function API_Definition*
define_api(Arena *arena){
    API_Definition *api = begin_api(arena, "font");
    
    {
        API_Call *call = api_call(arena, api, "make_face", "Face*");
        api_param(arena, call, "Arena*", "arena");
        api_param(arena, call, "Face_Description*", "description");
        api_param(arena, call, "f32", "scale_factor");
    }
    
    return(api);
}

function Generated_Group
get_api_group(void){
    return(GeneratedGroup_Core);
}

// BOTTOM

