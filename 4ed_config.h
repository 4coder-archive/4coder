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

// BOTTOM
