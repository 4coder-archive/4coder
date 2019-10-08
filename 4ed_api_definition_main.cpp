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

function Generated_Group
get_api_group(void);

int
main(void){
    Arena arena = make_arena_malloc();
    API_Definition *api = define_api(&arena);
    if (!api_definition_generate_api_includes(&arena, api, get_api_group(), 0)){
        return(1);
    }
    return(0);
}

// BOTTOM

