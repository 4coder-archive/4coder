/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.11.2015
 *
 * Application layer build target
 *
 */

// TOP

// TODO(allen): get away from assert.h
// TODO(allen): get away from string.h
#include <assert.h>
#include <string.h>

#include "4tech_defines.h"

#include "4coder_API/custom.h"

#include "4ed_math.h"
#include "4ed_system.h"

//#define USE_DEBUG_MEMORY

#define FSTRING_IMPLEMENTATION
#define FSTRING_C
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_mem.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_utf8.h"
#if defined(USE_DEBUG_MEMORY)
# include "4ed_debug_mem.h"
#endif

#include "4ed_rendering.h"
#include "4ed.h"

#define FCPP_FORBID_MALLOC
#include "4cpp/4cpp_lexer.h"

#include "4ed_doubly_linked_list.cpp"

#include "4ed_font_set.cpp"
#include "4ed_rendering_helper.cpp"

#include "4ed_style.h"
#include "4ed_style.cpp"
#include "4ed_command.cpp"

#include "file/4coder_buffer.cpp"
#include "file/4coder_undo.cpp"
#include "file/4coder_file.cpp"
#include "file/4coder_working_set.cpp"
#include "file/4coder_hot_directory.cpp"

#include "4ed_gui.h"
#include "4ed_gui.cpp"
#include "4ed_layout.cpp"
#include "4ed_app_models.h"
#include "4ed_file_view.cpp"
#include "4ed.cpp"

// BOTTOM

