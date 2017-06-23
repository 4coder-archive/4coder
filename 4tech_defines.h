/*
4tech_defines.h - Standard defines across all 4tech projects.
By Allen Webster
Created 21.01.2017 (dd.mm.yyyy)
*/

// TOP

#if !defined(FTECH_DEFINES)
#define FTECH_DEFINES

#include "4ed_os_comp_cracking.h"

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8 b8;
typedef i32 b32;

#if defined(FTECH_32_BIT)
typedef u32 umem;
typedef i32 imem;
#else
typedef u64 umem;
typedef i64 imem;
#endif

typedef float f32;
typedef double f64;

#define internal static
#define local_persist static
#define global static
#define local_const static const
#define global_const static const

#define ArrayCount(a) ((sizeof(a))/(sizeof(*a)))
#define ExpandArray(a) (a), (ArrayCount(a))
#define AllowLocal(c) (void)(c)
#define Member(T, m) (((T*)0)->m)

#if defined(Assert)
# undef Assert
#endif

#define Assert(c) do { if (!(c)) *((i32*)0) = 0xA11E; } while(0)
#define TentativeAssert(c) Assert(c)
#define InvalidCodePath Assert(!"Invalid Code Path!")
#define NotImplemented Assert(!"Not Implemented!")

#define Swap(t,a,b) do { t x = a; a = b; b = x; } while(0)

#define Max(a,b) (((a)>(b))?(a):(b))
#define Min(a,b) (((a)<(b))?(a):(b))

inline i32 ceil32(f32 v){
    return(((v)>0)?( (v == (i32)(v))?((i32)(v)):((i32)((v)+1.f)) ):( ((i32)(v)) ));
}

inline i32 floor32(f32 v){
    return(((v)<0)?( (v == (i32)(v))?((i32)(v)):((i32)((v)-1.f)) ):( ((i32)(v)) ));
}

inline i32 round32(f32 v){
    return(floor32(v + 0.5f));
}

inline i32 trun32(f32 v){
    return((i32)(v));
}

inline i32 div_ceil(i32 n, i32 d){
    return( ((n) % (d) != 0) + ((n) / (d)) );
}

inline i32 l_round_up_i32(i32 x, i32 b){
    return( ((x)+(b)-1) - (((x)+(b)-1)%(b)) );
}

inline u32 l_round_up_u32(u32 x, u32 b){
    return( ((x)+(b)-1) - (((x)+(b)-1)%(b)) );
}

inline u32 round_up_pot_u32(u32 x){
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    ++x;
    return(x);
}

#define STR__(s) #s
#define STR_(s) STR__(s)

#define DrCase(PC) case PC: goto resumespot_##PC
#define DrYield(PC, n) { *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }
#define DrReturn(n) { *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

#define Bytes(x) ((umem)x)
#define KB(x) (((umem)x) << 10)
#define MB(x) (((umem)x) << 20)
#define GB(x) (((umem)x) << 30)
#define TB(x) (((umem)x) << 40)

#define max_i8  ((i8)0x7F)
#define max_i16 ((i16)0x7FFF)
#define max_i32 ((i32)0x7FFFFFFF)
#define max_i64 ((i64)0x7FFFFFFFFFFFFFFF)

#define min_i8  ((i8)0x80)
#define min_i16 ((i16)0x8000)
#define min_i32 ((i32)0x80000000)
#define min_i64 ((i64)0x8000000000000000)

#define max_u8  ((u8)0xFF)
#define max_u16 ((u16)0xFFFF)
#define max_u32 ((u32)0xFFFFFFFF)
#define max_u64 ((u64)0xFFFFFFFFFFFFFFFF)

#define min_u8  ((u8)0)
#define min_u16 ((u16)0)
#define min_u32 ((u32)0)
#define min_u64 ((u64)0)

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

#endif

// BOTTOM

