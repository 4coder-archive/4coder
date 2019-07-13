/*
4coder_custom.h
*/

// TOP

#ifndef FCODER_CUSTOM_H
#define FCODER_CUSTOM_H

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "4coder_base_types.h"
#include "4coder_lib/4coder_heap.h"
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4cpp_lexer_types.h"
#include "4coder_API/4coder_version.h"
#include "4coder_API/4coder_keycodes.h"
#include "4coder_API/4coder_default_colors.h"
#include "4coder_API/4coder_types.h"
#include "4coder_generated/app_functions.h"

extern "C" _GET_VERSION_SIG(get_alpha_4coder_version){
    return((maj == MAJOR && min == MINOR && patch == PATCH));
}

#endif

// BOTTOM

