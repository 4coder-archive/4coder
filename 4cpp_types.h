/* "4cpp" Open C++ Parser v0.1: Types
   no warranty implied; use at your own risk
   
NOTES ON USE:
   This file is used to declare 4cpp fixed width integer and float types.
   It is not meant to be used directly.
*/

// TODO(allen):
// - create non stdint.h version in case someone wants to exclude that header

#include "4coder_config.h"

#ifndef FCPP_TYPES
#define FCPP_TYPES

#include <stdint.h>

typedef uint8_t fcpp_u8;
typedef uint64_t fcpp_u64;
typedef uint32_t fcpp_u32;
typedef uint16_t fcpp_u16;

typedef int8_t fcpp_i8;
typedef int64_t fcpp_i64;
typedef int32_t fcpp_i32;
typedef int16_t fcpp_i16;

typedef fcpp_i32 fcpp_bool32;
typedef fcpp_i8 fcpp_bool8;

typedef float fcpp_real32;
typedef double fcpp_real64;

#define FCPP_GLOBAL static

#define FCPP_COUNT(a) (sizeof(a)/sizeof(*(a)))

#endif
