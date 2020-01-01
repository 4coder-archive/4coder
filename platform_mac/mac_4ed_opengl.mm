/* Mac OpenGL layer for 4coder */

#include "opengl/4ed_opengl_defines.h"

#define GL_FUNC(N,R,P) typedef R (CALL_CONVENTION N##_Function)P; N##_Function *N = 0;
#include "mac_4ed_opengl_funcs.h"

#include "opengl/4ed_opengl_render.cpp"

function b32
mac_gl_load_functions(){
    b32 result = true;
    
    // NOTE(yuval): Open the gl dynamic library
    void* gl_image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
    
    // NOTE(yuval): Load gl functions
#define GL_FUNC(f,R,P) Stmnt((f) = (f##_Function*)dlsym(gl_image, #f); \
    (result) &= (f != 0););
#include "mac_4ed_opengl_funcs.h"
    
    return result;
}
