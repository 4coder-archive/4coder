/*
4coder_default_include.cpp - Default set of commands and setup used in 4coder.
*/

// TOP

#if !defined(FCODER_DEFAULT_INCLUDE_CPP)
#define FCODER_DEFAULT_INCLUDE_CPP

// NOTE(allen): Define USE_OLD_STYLE_JUMPS before 4coder_default_include.cpp to get
// the direct jumps (instead of sticky jumps).

#include "4coder_API/4coder_custom.h"
#include "4coder_os_comp_cracking.h"

#include "4coder_generated/command_metadata.h"

#include "4coder_lib/4coder_arena.cpp"
#include "4coder_lib/4coder_heap.cpp"

#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_utf8.h"
#include "4coder_lib/4cpp_lexer.h"

#include "4coder_api_transition_30_31.h"

#include "4coder_helper.h"
#include "4coder_fancy.h"
#include "4coder_ui_helper.h"
#include "4coder_default_framework.h"
#include "4coder_config.h"
#include "4coder_seek.h"
#include "4coder_auto_indent.h"
#include "4coder_search.h"
#include "4coder_build_commands.h"
#include "4coder_jump_sticky.h"
#include "4coder_jump_lister.h"
#include "4coder_project_commands.h"
#include "4coder_function_list.h"
#include "4coder_scope_commands.h"
#include "4coder_combined_write_commands.h"

#include "4coder_api_transition_30_31.cpp"

#include "4coder_default_framework_variables.cpp"
#include "4coder_buffer_seek_constructors.cpp"
#include "4coder_helper.cpp"
#include "4coder_fancy.cpp"
#include "4coder_ui_helper.cpp"
#include "4coder_font_helper.cpp"
#include "4coder_config.cpp"
#include "4coder_default_framework.cpp"
#include "4coder_insertion.cpp"
#include "4coder_seek.cpp"
#include "4coder_base_commands.cpp"
#include "4coder_lists.cpp"
#include "4coder_auto_indent.cpp"
#include "4coder_search.cpp"
#include "4coder_jumping.cpp"
#include "4coder_jump_direct.cpp"
#include "4coder_jump_sticky.cpp"
#include "4coder_jump_lister.cpp"
#include "4coder_clipboard.cpp"
#include "4coder_system_command.cpp"
#include "4coder_build_commands.cpp"
#include "4coder_project_commands.cpp"
#include "4coder_function_list.cpp"
#include "4coder_scope_commands.cpp"
#include "4coder_combined_write_commands.cpp"

#include "4coder_default_hooks.cpp"
#include "4coder_remapping_commands.cpp"

#include "4coder_api_transition_30_31_helpers.cpp"

#endif

// BOTTOM

