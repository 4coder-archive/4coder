/* "4cpp" Open C++ Parser v0.1: Clear Config
   no warranty implied; use at your own risk
   
NOTES ON USE:
   This file is used to clear options. The main use for this is for cases when you want
   it include different portions of the library with different settings.  So that the compiler
   does not complain about redifintion, and so that you do not have to undef everything yourself
   this is provided to undef everything at once.
*/

#ifdef FCPP_NO_CRT
#undef FCPP_NO_CRT
#endif

#ifdef FCPP_NO_MALLOC
#undef FCPP_NO_MALLOC
#endif

#ifdef FCPP_NO_ASSERT
#undef FCPP_NO_ASSERT
#endif

#ifdef FCPP_NO_STRING
#undef FCPP_NO_STRING
#endif

#ifdef FCPP_GET_MEMORY
#undef FCPP_GET_MEMORY
#endif

#ifdef FCPP_FREE_MEMORY
#undef FCPP_FREE_MEMORY
#endif

#ifdef FCPP_ASSERT
#undef FCPP_ASSERT
#endif

#ifdef FCPP_MEM_COPY
#undef FCPP_MEM_COPY
#endif

#ifdef FCPP_MEM_MOVE
#undef FCPP_MEM_MOVE
#endif

#ifdef FCPP_LINK
#undef FCPP_LINK
#endif

#ifdef FCPP_EXTERN
#undef FCPP_EXTERN
#endif

