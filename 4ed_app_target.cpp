/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.11.2015
 *
 * Application layer build target
 *
 */

// TOP

// TODO(allen): can I get away from this one?
#include <assert.h>
#include "4ed_defines.h"

#include "4coder_custom.h"

#define FSTRING_IMPLEMENTATION
#define FSTRING_C
#include "4coder_string.h"

#define BUFFER_EXPERIMENT_SCALPEL 0

#include "4ed_math.h"

#include "4ed_system.h"
#include "4ed_rendering.h"

#include "4ed.h"

#include "4cpp_lexer.h"

#include "4coder_table.cpp"

#include "4ed_doubly_linked_list.cpp"

#include "4ed_font_set.cpp"
#include "4ed_rendering_helper.cpp"

#include "4ed_style.h"
#include "4ed_style.cpp"
#include "4ed_command.cpp"
#include "4ed_file.cpp"
#include "4ed_gui.cpp"
#include "4ed_layout.cpp"
#include "4ed_app_models.h"
#include "4ed_file_view.cpp"
#include "4ed.cpp"

// BOTTOM

