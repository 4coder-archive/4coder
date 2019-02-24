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

#include "4coder_os_comp_cracking.h"

#if defined(IS_CL)
#if (_MSC_VER == 1500)
#define JUST_GUESS_INTS
#endif
#endif

#if defined(JUST_GUESS_INTS)
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
#else
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

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
#define external extern "C"

#define ArrayCount(a) ((sizeof(a))/(sizeof(*a)))
#define ExpandArray(a) (a), (ArrayCount(a))
#define AllowLocal(c) (void)(c)
#if !defined(Member)
# define Member(T, m) (((T*)0)->m)
#endif
#define PtrDif(a,b) ((uint8_t*)(a) - (uint8_t*)(b))
#define PtrAsInt(a) PtrDif(a,0)
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#define CastFromMember(S,m,ptr) (S*)( (uint8_t*)(ptr) - OffsetOfMember(S,m) )
#define IntAsPtr(a) (void*)(((uint8_t*)0) + a)

#define Stmnt(s) do{ s }while(0)

#define STR__(s) #s
#define STR_(s) STR__(s)
#define LINE_STR STR_(__LINE__)
#define FNLN __FILE__ ":" LINE_STR ":"

#if defined(Assert)
# undef Assert
#endif

#define Assert(c) do { if (!(c)) *((i32*)0) = 0xA11E; } while(0)
#define TentativeAssert(c) Assert(c)
#define InvalidCodePath Assert(!"Invalid Code Path!")
#define NotImplemented Assert(!"Not Implemented!")
#define AssertImplies(a,b) Assert(!(a) || (b))

#define Swap(t,a,b) do { t x = a; a = b; b = x; } while(0)

#define Max(a,b) (((a)>(b))?(a):(b))
#define Min(a,b) (((a)<(b))?(a):(b))

#define FixSize(s) struct{ u8 __size_fixer__[s]; }

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

#if !defined(max_f32)
internal f32
max_f32_proc(void){
    union{
        u32 x;
        f32 f;
    } c;
    c.x = 0x7f800000;
    return(c.f);
}

#define max_f32 max_f32_proc()
#endif

global_const u32 bit_0 = (((u32)1) << 0);
global_const u32 bit_1 = (((u32)1) << 1);
global_const u32 bit_2 = (((u32)1) << 2);
global_const u32 bit_3 = (((u32)1) << 3);
global_const u32 bit_4 = (((u32)1) << 4);
global_const u32 bit_5 = (((u32)1) << 5);
global_const u32 bit_6 = (((u32)1) << 6);
global_const u32 bit_7 = (((u32)1) << 7);

global_const u32 bit_8 = (((u32)1) << 8);
global_const u32 bit_9 = (((u32)1) << 9);
global_const u32 bit_10 = (((u32)1) << 10);
global_const u32 bit_11 = (((u32)1) << 11);
global_const u32 bit_12 = (((u32)1) << 12);
global_const u32 bit_13 = (((u32)1) << 13);
global_const u32 bit_14 = (((u32)1) << 14);
global_const u32 bit_15 = (((u32)1) << 15);

global_const u32 bit_16 = (((u32)1) << 16);
global_const u32 bit_17 = (((u32)1) << 17);
global_const u32 bit_18 = (((u32)1) << 18);
global_const u32 bit_19 = (((u32)1) << 19);
global_const u32 bit_20 = (((u32)1) << 20);
global_const u32 bit_21 = (((u32)1) << 21);
global_const u32 bit_22 = (((u32)1) << 22);
global_const u32 bit_23 = (((u32)1) << 23);

global_const u32 bit_24 = (((u32)1) << 24);
global_const u32 bit_25 = (((u32)1) << 25);
global_const u32 bit_26 = (((u32)1) << 26);
global_const u32 bit_27 = (((u32)1) << 27);
global_const u32 bit_28 = (((u32)1) << 28);
global_const u32 bit_29 = (((u32)1) << 29);
global_const u32 bit_30 = (((u32)1) << 30);
global_const u32 bit_31 = (((u32)1) << 31);

global_const u64 bit_32 = (((u64)1) << (0 + 32));
global_const u64 bit_33 = (((u64)1) << (1 + 32));
global_const u64 bit_34 = (((u64)1) << (2 + 32));
global_const u64 bit_35 = (((u64)1) << (3 + 32));
global_const u64 bit_36 = (((u64)1) << (4 + 32));
global_const u64 bit_37 = (((u64)1) << (5 + 32));
global_const u64 bit_38 = (((u64)1) << (6 + 32));
global_const u64 bit_39 = (((u64)1) << (7 + 32));

global_const u64 bit_40 = (((u64)1) << (8 + 32));
global_const u64 bit_41 = (((u64)1) << (9 + 32));
global_const u64 bit_42 = (((u64)1) << (10 + 32));
global_const u64 bit_43 = (((u64)1) << (11 + 32));
global_const u64 bit_44 = (((u64)1) << (12 + 32));
global_const u64 bit_45 = (((u64)1) << (13 + 32));
global_const u64 bit_46 = (((u64)1) << (14 + 32));
global_const u64 bit_47 = (((u64)1) << (15 + 32));

global_const u64 bit_48 = (((u64)1) << (16 + 32));
global_const u64 bit_49 = (((u64)1) << (17 + 32));
global_const u64 bit_50 = (((u64)1) << (18 + 32));
global_const u64 bit_51 = (((u64)1) << (19 + 32));
global_const u64 bit_52 = (((u64)1) << (20 + 32));
global_const u64 bit_53 = (((u64)1) << (21 + 32));
global_const u64 bit_54 = (((u64)1) << (22 + 32));
global_const u64 bit_55 = (((u64)1) << (23 + 32));

global_const u64 bit_56 = (((u64)1) << (24 + 32));
global_const u64 bit_57 = (((u64)1) << (25 + 32));
global_const u64 bit_58 = (((u64)1) << (26 + 32));
global_const u64 bit_59 = (((u64)1) << (27 + 32));
global_const u64 bit_60 = (((u64)1) << (28 + 32));
global_const u64 bit_61 = (((u64)1) << (29 + 32));
global_const u64 bit_62 = (((u64)1) << (30 + 32));
global_const u64 bit_63 = (((u64)1) << (31 + 32));

#endif

// BOTTOM

