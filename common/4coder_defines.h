/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Meta setup for project codename "4ed"
 *
 */

// TOP

#if !defined(FRED_DEFINES_H)
#define FRED_DEFINES_H

#if !defined (FRED_TYPES)
#include <stdint.h>
#define FRED_TYPES
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 bool32;
typedef i8 bool8;
typedef i32 b32;
typedef i16 b16;
typedef i8 b8;

typedef float real32;
typedef double real64;
typedef float f32;
typedef double f64;

#define internal static
#define globalvar static
#define persist static

#define globalconst static const
#endif

#define COMP_ID_(a,b,c,d) (d << 24) | (c << 16) | (b << 8) | a
#define COMPOSE_ID(a,b,c,d) (COMP_ID_((a),(b),(c),(d)))

#define S(X) #X
#define S_(X) S(X)
#define S__LINE__ S_(__LINE__)

#if FRED_INTERNAL || FRED_KEEP_ASSERT
#  define Assert(c) assert(c)
#else
#  define Assert(c)
#endif
#define TentativeAssert(c) Assert(c)
#define NotImplemented Assert(!"This is not implemented yet!")
#define InvalidCodePath Assert(!"Invalid code path!")
#define InvalidPath InvalidCodePath

#define AllowLocal(name) (void)name
#ifndef ArrayCount
#  define ArrayCount(array) (sizeof(array)/sizeof(array[0]))
#endif

#define OffsetOfStruct(S,c) ((i64)(& ((S*)0)->c ))

#define Swap(T,a,b) do{ T t = a; a = b; b = t; } while(0)

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

#define TMin(t,v) globalconst t min_##t = ((t)(v))
TMin(i8,  -127-1);
TMin(i16, -32767-1);
TMin(i32, -2147483647-1);
TMin(i64, -9223372036854775807-1);
#undef TMin

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

#define bytes(n) (n)
#define Kbytes(n) ((n) << 10)
#define Mbytes(n) ((n) << 20)
#define Gbytes(n) (((u64)n) << 30)
#define Tbytes(n) (((u64)n) << 40)

//
// Rounding
//

internal u32
l_round_up_u32(u32 x, u32 granularity){
    u32 new_x = x + granularity - 1;
    new_x = new_x - (new_x % granularity);
    return(new_x);
}

internal i32
l_round_up_i32(i32 x, i32 granularity){
    i32 new_x = (i32)l_round_up_u32((u32)x, (u32)granularity);
    return(new_x);
}

inline i32
TRUNC32(real32 x) { return (i32)x; }

inline i32
FLOOR32(real32 x) { return (i32)(x)-((x!=(i32)(x) && x<0)?1:0); }

inline i32
CEIL32(real32 x) { return (i32)(x)+((x!=(i32)(x) && x>0)?1:0); }

inline i32
ROUND32(real32 x) { return FLOOR32(x + .5f); }

inline i32
DIVCEIL32(i32 n, i32 d) {
    i32 q = (n/d);
    return q + (q*d < n);
}

inline real32
FRACPART32(real32 x) { return x - (i32)x; }

inline u32
ROUNDPOT32(u32 v){
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

#endif

// BOTTOM

