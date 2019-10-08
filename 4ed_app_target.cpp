/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.11.2015
 *
 * Application layer build target
 *
 */

// TOP

#define REMOVE_OLD_STRING

#include "4coder_base_types.h"
#include "4coder_version.h"
#include "4coder_keycodes.h"
#include "4coder_default_colors.h"
#include "4coder_types.h"
#define STATIC_LINK_API
#include "generated/custom_api.h"

#include "4coder_table.h"

#include "4coder_system_types.h"
#define DYNAMIC_LINK_API
#include "generated/system_api.h"

#include "4ed_font_interface.h"
#define DYNAMIC_LINK_API
#include "generated/graphics_api.h"
#define DYNAMIC_LINK_API
#include "generated/font_api.h"

#include "4coder_string_match.h"

#include "4coder_base_types.cpp"
#include "4coder_string_match.cpp"
#include "4coder_stringf.cpp"
#include "4coder_app_links_allocator.cpp"
#include "4coder_system_allocator.cpp"
#include "4coder_hash_functions.cpp"
#include "4coder_table.cpp"

#include "4ed_render_target.h"
#include "4ed.h"
#include "4ed_buffer_model.h"
#include "4ed_coroutine.h"

#include "4coder_token.h"

#include "4ed_dynamic_variables.h"

#include "4ed_buffer_model.h"
#include "4ed_translation.h"
#include "4ed_command.h"
#include "4ed_buffer.h"
#include "4ed_history.h"
#include "4ed_file.h"

#include "4ed_working_set.h"
#include "4ed_hot_directory.h"
#include "4ed_cli.h"
#include "4ed_gui.h"
#include "4ed_layout.h"
#include "4ed_view.h"
#include "4ed_edit.h"
#include "4ed_text_layout.h"
#include "4ed_font_set.h"
#include "4ed_log.h"
#include "4ed_app_models.h"

#include "generated/custom_api.cpp"
#define DYNAMIC_LINK_API
#include "generated/system_api.cpp"
#define DYNAMIC_LINK_API
#include "generated/graphics_api.cpp"
#define DYNAMIC_LINK_API
#include "generated/font_api.cpp"

#include "4coder_system_helpers.cpp"
#include "4ed_allocator_models.cpp"
#include "4ed_log.cpp"
#include "4coder_log.cpp"
#include "4ed_coroutine.cpp"
#include "4ed_mem.cpp"
#include "4ed_dynamic_variables.cpp"
#include "4ed_font_face.cpp"
#include "4ed_font_set.cpp"
#include "4ed_translation.cpp"
#include "4ed_render_target.cpp"
#include "4ed_command.cpp"
#include "4ed_buffer.cpp"
#include "4ed_string_matching.cpp"
#include "4ed_history.cpp"
#include "4ed_file.cpp"
#include "4ed_working_set.cpp"
#include "4ed_hot_directory.cpp"
#include "4ed_cli.cpp"
#include "4ed_gui.cpp"
#include "4ed_layout.cpp"
#include "4coder_buffer_seek_constructors.cpp"
#include "4ed_view.cpp"
#include "4ed_edit.cpp"
#include "4ed_text_layout.cpp"
#include "4ed.cpp"

// BOTTOM

