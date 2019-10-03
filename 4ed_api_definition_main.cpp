/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.10.2019
 *
 * System API definition program.
 *
 */

// TOP

#include "4coder_base_types.h"
#include "4ed_api_definition.h"

#include "4coder_base_types.cpp"
#include "4ed_api_definition.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"

#include <stdio.h>

////////////////////////////////

function API_Definition*
define_api(Arena *arena);

int
main(void){
    Arena arena = make_arena_malloc();
    API_Definition *api = define_api(&arena);
    
    ////////////////////////////////
    
    // NOTE(allen): Arrange output files
    
    String_Const_u8 path_to_self = string_u8_litexpr(__FILE__);
    path_to_self = string_remove_last_folder(path_to_self);
    
    String_Const_u8 fname_h = push_u8_stringf(&arena, "%.*sgenerated/%.*s_api.h",
                                              string_expand(path_to_self),
                                              string_expand(api->name));
    
    String_Const_u8 fname_cpp = push_u8_stringf(&arena, "%.*sgenerated/%.*s_api.cpp",
                                                string_expand(path_to_self),
                                                string_expand(api->name));
    
    FILE *out_file_h = fopen((char*)fname_h.str, "wb");
    if (out_file_h == 0){
        printf("could not open output file: '%s'\n", fname_h.str);
        exit(1);
    }
    
    FILE *out_file_cpp = fopen((char*)fname_cpp.str, "wb");
    if (out_file_cpp == 0){
        printf("could not open output file: '%s'\n", fname_cpp.str);
        exit(1);
    }
    
    printf("%s:1:\n", fname_h.str);
    printf("%s:1:\n", fname_cpp.str);
    
    ////////////////////////////////
    
    // NOTE(allen): Generate output
    
    generate_header(&arena, api, out_file_h);
    generate_cpp(&arena, api, out_file_cpp);
    
    ////////////////////////////////
    
    fclose(out_file_h);
    fclose(out_file_cpp);
    
    return(0);
}

// BOTTOM

