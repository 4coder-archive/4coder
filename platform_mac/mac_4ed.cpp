/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.28.2017
 *
 * Mac C++ layer for 4coder
 *
 */

// TOP

#include "4tech_defines.h"
#include "4coder_API/version.h"

#include "osx_objective_c_to_cpp_links.h"

OSX_Vars osx;

// TODO(allen): Implement a real allocate
#include <stdlib.h>
void*
osx_allocate(umem size){
    void *result = malloc(size);
    return(result);
}

void
osx_resize(int width, int height){
    osx.width = width;
    osx.height = height;
    // TODO
}

void
osx_character_input(u32 code, OSX_Keyboard_Modifiers modifier_flags){
    // TODO
}

void
osx_mouse(i32 mx, i32 my, u32 type){
    // TODO
}

void
osx_mouse_wheel(float dx, float dy){
    // TODO
}

void
osx_step(){
    // TODO
}

void
osx_init(){
    // TODO
}

// BOTTOM

