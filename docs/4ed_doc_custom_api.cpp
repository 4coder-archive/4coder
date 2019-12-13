/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 04.12.2019
 *
 * Documentation of the custom layer's primary api.
 *
 */

// TOP

function void
doc_custom_app_ptr(Arena *arena, Doc_Function *func){
    doc_function_param(arena, func, "app");
    doc_text(arena, func->params, "the standard custom layer context pointer");
}

#include "4ed_doc_custom_api_global.cpp"
#include "4ed_doc_custom_api_buffer.cpp"
#include "4ed_doc_custom_api_view.cpp"
#include "4ed_doc_custom_api_draw.cpp"

function Doc_Cluster*
doc_custom_api(Arena *arena, API_Definition *api_def){
    Doc_Cluster *cluster = new_doc_cluster(arena, "Custom Layer Boundary API", "custom_api");
    
    doc_custom_api__global(arena, api_def, cluster);
    doc_custom_api__buffer(arena, api_def, cluster);
    doc_custom_api__view(arena, api_def, cluster);
    doc_custom_api__draw(arena, api_def, cluster);
    
    return(cluster);
}

// BOTTOM

