/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.11.2014
 *
 * Application layer build target
 *
 */

// TOP

#ifdef FRED_NOT_PACKAGE

#define FRED_INTERNAL 1
#define FRED_SLOW 1

#define FRED_PRINT_DEBUG 1
#define FRED_PRINT_DEBUG_FILE_LINE 0
#define FRED_PROFILING 1
#define FRED_PROFILING_OS 0
#define FRED_FULL_ERRORS 0

#else

#define FRED_SLOW 0
#define FRED_INTERNAL 0

#define FRED_PRINT_DEBUG 0
#define FRED_PRINT_DEBUG_FILE_LINE 0
#define FRED_PROFILING 0
#define FRED_PROFILING_OS 0
#define FRED_FULL_ERRORS 0

#endif

#define SOFTWARE_RENDER 0

#if FRED_INTERNAL == 0
#undef FRED_PRINT_DEBUG
#define FRED_PRINT_DEBUG 0
#undef FRED_PROFILING
#define FRED_PROFILING 0
#undef FRED_PROFILING_OS
#define FRED_PROFILING_OS 0
#endif

#if FRED_PRINT_DEBUG == 0
#undef FRED_PRINT_DEBUG_FILE_LINE
#define FRED_PRINT_DEBUG_FILE_LINE 0
#undef FRED_PRINT_DEBUG_FILE_LINE
#define FRED_PROFILING_OS 0
#endif

#define FPS 30
#define FRAME_TIME (1000000 / FPS)

#define BUFFER_EXPERIMENT_SCALPEL 0

#include "4ed_meta.h"

#include "4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4cpp_string.h"

#include "4ed_mem.cpp"

#include "4ed_math.cpp"
#include "4coder_custom.h"
#include "4ed_system.h"
#include "4ed.h"
#include "4ed_rendering.h"

#if defined(_WIN32)
#  include <windows.h>
#  include <GL/gl.h>
#elif defined(__linux__)
#  include <SDL/SDL.h>
#  include <GL/gl.h>
#else
#  error UNSUPPORTED PLATFORM
#endif

#include "4ed_internal.h"

#define FCPP_LEXER_IMPLEMENTATION
#include "4cpp_lexer.h"

#include "4ed_rendering.cpp"
#include "4ed_command.cpp"
#include "4ed_layout.cpp"
#include "4ed_style.cpp"
#include "4ed_file_view.cpp"
#include "4ed_color_view.cpp"
#include "4ed_interactive_view.cpp"
#include "4ed_menu_view.cpp"
#include "4ed_debug_view.cpp"
#include "4ed.cpp"

// BOTTOM

