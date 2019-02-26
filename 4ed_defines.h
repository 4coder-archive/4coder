/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2017
 *
 * Standard defines across 4coder code base.
 *
 */

// TOP

#if !defined(FTECH_DEFINES)
#define FTECH_DEFINES

#define internal static
#define local_persist static
#define global static
#define local_const static const
#define global_const static const
#define external extern "C"

#define FixSize(s) struct{ u8 __size_fixer__[s]; }

#define DrCase(PC) case PC: goto resumespot_##PC
#define DrYield(PC, n) { *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }
#define DrReturn(n) { *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

#endif

// BOTTOM

