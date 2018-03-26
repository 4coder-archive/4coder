/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.11.2015
 *
 * Application layer build target
 *
 */

// TOP

// TODO(allen): get away from string.h
#include <string.h>

#include "4ed_defines.h"

#include "4coder_API/custom.h"

#include "4ed_math.h"
#include "4ed_font.h"
#include "4ed_system.h"
#include "4ed_input_simulation_event.h"

#define PREFERRED_ALIGNMENT 8
#define USE_DEBUG_MEMORY

#define FSTRING_IMPLEMENTATION
#define FSTRING_C
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_mem.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_utf8.h"
#if defined(USE_DEBUG_MEMORY)
# include "4ed_debug_mem.h"
#endif

#include "4ed_render_target.h"
#include "4ed_render_format.h"
#include "4ed.h"
#include "4ed_buffer_model.h"

#define FCPP_FORBID_MALLOC
#include "4coder_lib/4cpp_lexer.h"

#include "4ed_linked_node_macros.h"
#include "4ed_log.h"

#include "4ed_buffer_model.h"
#include "4ed_translation.h"
#include "4ed_command.h"
#include "4ed_buffer.h"
#include "4ed_undo.h"
#include "4ed_file.h"
#include "4ed_code_wrap.h"

#include "4ed_working_set.h"
#include "4ed_style.h"
#include "4ed_hot_directory.h"
#include "4ed_parse_context.h"
#include "4ed_cli.h"
#include "4ed_gui.h"
#include "4ed_layout.h"
#include "4ed_view.h"
#include "4ed_app_models.h"

#include "4ed_parse_context.cpp"
#include "4ed_font.cpp"
#include "4ed_translation.cpp"
#include "4ed_render_target.cpp"
#include "4ed_render_format.cpp"
#include "4ed_command.cpp"
#include "4ed_buffer.cpp"
#include "4ed_undo.cpp"
#include "4ed_file_lex.cpp"
#include "4ed_file.cpp"
#include "4ed_code_wrap.cpp"
#include "4ed_working_set.cpp"
#include "4ed_style.cpp"
#include "4ed_hot_directory.cpp"
#include "4ed_cli.cpp"
#include "4ed_gui.cpp"
#include "4ed_layout.cpp"
#include "4ed_view.cpp"
#include "4ed_edit.cpp"
#include "4ed_view_ui.cpp"
#include "4ed.cpp"

// BOTTOM

