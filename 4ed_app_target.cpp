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

#define FSTRING_IMPLEMENTATION
#define FSTRING_C
#include "4coder_string.h"

#include "4coder_custom.h"

#include "4ed_math.h"

#include "4ed_system.h"
#include "4ed_rendering.h"

#include "4ed.h"
#include "4ed_mem.h"

#define FCPP_FORBID_MALLOC
#include "4cpp_lexer.h"

#include "4coder_table.cpp"

#include "4ed_doubly_linked_list.cpp"

#include "4ed_font_set.cpp"
#include "4ed_rendering_helper.cpp"

#include "4ed_style.h"
#include "4ed_style.cpp"
#include "4ed_command.cpp"

#include "buffer/4coder_shared.cpp"
#include "buffer/4coder_gap_buffer.cpp"
#define Buffer_Type Gap_Buffer
#include "buffer/4coder_buffer_abstract.cpp"

#include "file/4coder_file.cpp"
#include "file/4coder_working_set.cpp"
#include "file/4coder_hot_directory.cpp"

#include "4ed_gui.cpp"
#include "4ed_layout.cpp"
#include "4ed_app_models.h"
#include "4ed_file_view.cpp"
#include "4ed.cpp"

// BOTTOM

