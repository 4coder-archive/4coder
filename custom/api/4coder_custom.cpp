/*
4coder_custom.cpp
*/

// TOP

#define DYNAMIC_LINK_API
#include "generated/custom_api.cpp"

extern "C" b32
get_version(i32 maj, i32 min, i32 patch){
    return(maj == MAJOR && min == MINOR && patch == PATCH);
}

extern "C" void
init_apis(API_VTable_custom *custom_vtable){
    custom_api_read_vtable(custom_vtable);
}

// BOTTOM

