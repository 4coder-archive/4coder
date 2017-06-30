/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.06.2017
 *
 * Mac C++ layer for 4coder
 *
 */

// TOP

#define IS_PLAT_LAYER

#include "4tech_defines.h"
#include "4coder_API/version.h"

#include "4coder_lib/4coder_utf8.h"

#if defined(FRED_SUPER)
# include "4coder_API/keycodes.h"
# include "4coder_API/style.h"

# define FSTRING_IMPLEMENTATION
# define FSTRING_C
# include "4coder_lib/4coder_string.h"
# include "4coder_lib/4coder_mem.h"

# include "4coder_API/types.h"
# include "4ed_os_custom_api.h"

#else
# include "4coder_default_bindings.cpp"
#endif

#include "4ed_math.h"

#include "4ed_system.h"
#include "4ed_log.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include "4ed_file_track.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_system_shared.h"

#include "unix_4ed_functions.cpp"

#include "osx_objective_c_to_cpp_links.h"
OSX_Vars osx;

external void*
osx_allocate(umem size){
    void *result = system_memory_allocate(size);
    return(result);
}

external void
osx_resize(int width, int height){
    osx.width = width;
    osx.height = height;
    // TODO
}

external void
osx_character_input(u32 code, OSX_Keyboard_Modifiers modifier_flags){
    // TODO
}

external void
osx_mouse(i32 mx, i32 my, u32 type){
    // TODO
}

external void
osx_mouse_wheel(float dx, float dy){
    // TODO
}

external void
osx_step(){
    // TODO
}

external void
osx_init(){
    // TODO
}

// BOTTOM

