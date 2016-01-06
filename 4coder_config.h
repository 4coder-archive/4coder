/* "4cpp" Open C++ Parser v0.1: Config
   no warranty implied; use at your own risk
   
NOTES ON USE:
   This file is used to configure 4cpp options at the begining of 4cpp files.
   It is not meant to be used directly.
*/

#ifdef FCPP_NO_CRT
# ifndef FCPP_NO_MALLOC
# define FCPP_NO_MALLOC
# endif
# ifndef FCPP_NO_ASSERT
# define FCPP_NO_ASSERT
# endif
# ifndef FCPP_NO_STRING
# define FCPP_NO_STRING
# endif
#endif

#ifdef FCPP_FORBID_MALLOC
# define FCPP_NO_MALLOC
#endif

#ifndef FCPP_NO_MALLOC
# include <stdlib.h>
#endif

#ifndef FCPP_NO_ASSERT
# include <assert.h>
#endif

#ifndef FCPP_NO_STRING
# include <string.h>
#endif

#ifndef FCPP_NO_MALLOC
# ifndef FCPP_GET_MEMORY
# define FCPP_GET_MEMORY malloc
# endif
# ifndef FCPP_FREE_MEMORY
# define FCPP_FREE_MEMORY free
# endif
#else
# ifndef FCPP_FORBID_MALLOC
#  ifndef FCPP_GET_MEMORY
#   error Missing definition for FCPP_GET_MEMORY
#  endif
#  ifndef FCPP_FREE_MEMORY
#   error Missing definition for FCPP_FREE_MEMORY
#  endif
# endif
#endif

#ifndef FCPP_NO_ASSERT
# ifndef FCPP_ASSERT
# define FCPP_ASSERT assert
# endif
#else
# ifndef FCPP_ASSERT
# define FCPP_ASSERT(x)
# endif
#endif

#ifndef FCPP_NO_STRING
# ifndef FCPP_MEM_COPY
# define FCPP_MEM_COPY memcpy
# endif
# ifndef FCPP_MEM_MOVE
# define FCPP_MEM_MOVE memmove
# endif
#endif

#ifndef FCPP_LINK
# ifdef FCPP_EXTERN
#  define FCPP_LINK extern
# else
#  define FCPP_LINK static
# endif
#endif

