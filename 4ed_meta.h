/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Meta setup for project codename "4ed"
 *
 */

#ifndef FRED_META_H
#define FRED_META_H

#include <string.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;

typedef int8_t i8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;

typedef i32 bool32;
typedef i8 bool8;
typedef i32 b32;
typedef i8 b8;

typedef uint8_t byte;

typedef float real32;
typedef double real64;
typedef float f32;
typedef double f64;

struct Data{
    byte *data;
    i32 size;
};

#define external extern "C"
#define internal static
#define globalvar static
#define persist static

#define globalconst static const

inline i32
raw_ptr_dif(void *a, void *b) { return (i32)((u8*)a - (u8*)b); }

#define COMP_ID_(a,b,c,d) (d << 24) | (c << 16) | (b << 8) | a
#define COMPOSE_ID(a,b,c,d) (COMP_ID_((a),(b),(c),(d)))

#define S(X) #X
#define S_(X) S(X)
#define S__LINE__ S_(__LINE__)

#if FRED_PRINT_DEBUG == 1
internal void
_OutDbgStr(u8*);
#  include <stdio.h>
#  if FRED_PRINT_DEBUG_FILE_LINE
#    define FredDbg(con, size, ...) {_OutDbgStr((u8*)("FILE:"__FILE__"LINE:"S__LINE__"\n")); char msg[size]; sprintf(msg, __VA_ARGS__); _OutDbgStr((u8*)msg);}
#  else
#    define FredDbg(con, size, ...) {char msg[size]; sprintf(msg, __VA_ARGS__); _OutDbgStr((u8*)msg);}
#  endif
#elif FRED_PRINT_DEBUG == 2
#  include <stdio.h>
#  if FRED_PRINT_DEBUG_FILE_LINE
#    define FredDbg(con, size, ...) {fprintf((con)->log, ("FILE:"__FILE__"LINE:"S__LINE__"\n")); fprintf(__VA_ARGS__);}
#  else
#    define FredDbg(con, size, ...) {fprintf((con)->log, __VA_ARGS__);}
#  endif
#else
#  define FredDbg(con, size, ...)
#endif

#if FRED_INTERNAL && FRED_FULL_ERRORS
#  include <stdio.h>
#  define FatalErrorFormat(alt, size, ...) {char msg[size]; sprintf(msg, __VA_ARGS__); FatalError(msg);}
#else
#  define FatalErrorFormat(alt, size, ...) {FatalError(alt);}
#endif

#if FRED_SLOW
#  define Assert(c) assert(c)
#else
#  define Assert(c)
#endif

#define TentativeAssert(c) Assert(c)

#define FatalError(message) system_fatal_error((u8*)message)

#define AllowLocal(name) (void)name
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))
#define OffsetOfStruct(S,c) ((i64)(& ((S*)0)->c ))
#define OffsetOfPtr(s,c) ((i64)((char*)(&(s)->c) - (char*)(s)))

#define Swap(T,a,b) do{ T t = a; a = b; b = t; } while(0)

#ifndef literal
#define literal(s) s, (sizeof(s)-1)
#endif

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))

#define TMax(t,v) globalconst t max_##t = v
TMax(u8,  255);
TMax(u16, 65535);
TMax(u32, 4294967295);
TMax(u64, 18446744073709551615U);

TMax(i8,  127);
TMax(i16, 32767);
TMax(i32, 2147483647);
TMax(i64, 9223372036854775807);
#undef TMax

#define TMin(t) globalconst t min_##t = 0
TMin(u8);
TMin(u16);
TMin(u32);
TMin(u64);
#undef TMin

#define TMin(t,v) globalconst t min_##t = ((t)v)
TMin(i8,  -0xF0);
TMin(i16, -0xF000);
TMin(i32, -0xF00000);
TMin(i64, -0xF0000000LL);
#undef TMin

internal i32
LargeRoundUp(i32 x, i32 granularity){
	i32 original_x = x;
	x /= granularity;
	x *= granularity;
	if (x < original_x){
		x += granularity;
	}
	return x;
}

#define Bit_0 (1 << 0)
#define Bit_1 (1 << 1)
#define Bit_2 (1 << 2)
#define Bit_3 (1 << 3)
#define Bit_4 (1 << 4)
#define Bit_5 (1 << 5)
#define Bit_6 (1 << 6)
#define Bit_7 (1 << 7)

#define Bit_8 (1 << 8)
#define Bit_9 (1 << 9)
#define Bit_10 (1 << 10)
#define Bit_11 (1 << 11)
#define Bit_12 (1 << 12)
#define Bit_13 (1 << 13)
#define Bit_14 (1 << 14)
#define Bit_15 (1 << 15)

#define Bit_16 (1 << 16)
#define Bit_17 (1 << 17)
#define Bit_18 (1 << 18)
#define Bit_19 (1 << 19)
#define Bit_20 (1 << 20)
#define Bit_21 (1 << 21)
#define Bit_22 (1 << 22)
#define Bit_23 (1 << 23)

#define Bit_24 (1 << 24)
#define Bit_25 (1 << 25)
#define Bit_26 (1 << 26)
#define Bit_27 (1 << 27)
#define Bit_28 (1 << 28)
#define Bit_29 (1 << 29)
#define Bit_30 (1 << 30)
#define Bit_31 (1 << 31)

#define Byte_0 (0xFFU)
#define Byte_1 (0xFFU << 8)
#define Byte_2 (0xFFU << 16)
#define Byte_3 (0xFFU << 24)
#define Byte_4 (0xFFU << 32)
#define Byte_5 (0xFFU << 40)
#define Byte_6 (0xFFU << 48)
#define Byte_7 (0xFFU << 56)

#define bytes(n) (n)
#define Kbytes(n) ((n) << 10)
#define Mbytes(n) ((n) << 20)
#define Gbytes(n) (((u64)n) << 30)
#define Tbytes(n) (((u64)n) << 40)

#endif

