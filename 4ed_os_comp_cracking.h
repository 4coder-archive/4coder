/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 31.03.2017
 *
 * OS & Compiler cracking for 4coder
 *
 */

// TOP

#if !defined(FRED_OS_COMP_CRACKING_H)
#define FRED_OS_COMP_CRACKING_H

#if defined(_MSC_VER) /* COMPILER */

# define IS_CL

// NOTE(allen): I suspect snprintf is defined in VS 2015 (I test on 2017)
#if _MSC_VER <= 1800
# define snprintf _snprintf
#endif

# if defined(_WIN32) /* OS */
#  define IS_WINDOWS
#  pragma comment(lib, "Kernel32.lib")
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(_M_AMD64) /* Arch */
#  define CALL_CONVENTION
#  define BUILD_X64
# elif defined(_M_IX86)
#  define CALL_CONVENTION __stdcall
#  define BUILD_X86
# else
#  error architecture not supported yet
# endif

#elif defined(__GNUC__) || defined(__GNUG__)

# define IS_GCC /* COMPILER */

# if defined(__gnu_linux__) /* OS */
#  define IS_LINUX
# elif defined(__APPLE__) && defined(__MACH__) /* OS */
#  define IS_MAC
#else
#  error This compiler/platform combo is not supported yet
# endif

#else
#error This compiler is not supported yet
#endif

#endif

// BOTTOM

