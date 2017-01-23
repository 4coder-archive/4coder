
#ifndef FCODER_CUSTOM_H
#define FCODER_CUSTOM_H

#include <stdint.h>

#include "version.h"
#include "keycodes.h"
#include "style.h"
// TODO(allen): I don't like having to pull in the types from my standalone libraries to define the API.
// What to do??? Hmmm....
#include "4coder_lib/4coder_string.h"
#include "4cpp/4cpp_lexer_types.h"
#include "types.h"
#include "app_functions.h"

extern "C" _GET_VERSION_SIG(get_alpha_4coder_version){
    int32_t result = (maj == MAJOR && min == MINOR && patch == PATCH);
    return(result);
}

#endif
