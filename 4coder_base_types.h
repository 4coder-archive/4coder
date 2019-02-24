/*
 * 4coder base types
 */

// TOP

#if !defined(FCODER_BASE_TYPES)
#define FCODER_BASE_TYPES

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

#define Swap(t,a,b) do { t x = a; a = b; b = x; } while(0)

#define Max(a,b) (((a)>(b))?(a):(b))
#define Min(a,b) (((a)<(b))?(a):(b))

#define clamp_top(a,b) Min(a,b)
#define clamp_bottom(a,b) Max(a,b)
//#define clamp(a,x,b) Min(

////////////////////////////////

struct Vec2_f32{
    union{
        struct{
            f32 x, y;
        };
        struct{
            f32 v[2];
        };
    };
};

struct Vec3_f32{
    union{
        struct{
            f32 x, y, z;
        };
        struct{
            f32 r, g, b;
        };
        struct{
            Vec2_f32 xy;
            f32 _z;
        };
        struct{
            f32 _x;
            Vec2_f32 yz;
        };
        struct{
            f32 v[3];
        };
    };
};

struct Vec4_f32{
    union{
        struct{
            f32 r, g, b, a;
        };
        struct{
            f32 h, s, l, __a;
        };
        struct{
            f32 x, y, z, w;
        };
        struct{
            Vec3_f32 xy;
            Vec3_f32 zw;
        };
        struct{
            Vec3_f32 _x;
            Vec3_f32 yz;
            Vec3_f32 _w;
        };
        struct{
            Vec3_f32 rgb;
            f32 __a;
        };
        struct{
            Vec3_f32 xyz;
            f32 __w;
        };
        struct{
            f32 __x;
            Vec3_f32 yzw;
        };
        struct{
            f32 v[4];
        };
    };
};

struct Vec2_i32{
    union{
        struct{
            i32 x, y;
        };
        struct{
            i32 v[2];
        };
    };
};

struct Vec3_i32{
    union{
        struct{
            i32 x, y, z;
        };
        struct{
            i32 r, g, b;
        };
        struct{
            Vec2_i32 xy;
            i32 _z;
        };
        struct{
            i32 _x;
            Vec2_i32 yz;
        };
        struct{
            i32 v[3];
        };
    };
};

struct Vec4_i32{
    union{
        struct{
            i32 r, g, b, a;
        };
        struct{
            i32 h, s, l, __a;
        };
        struct{
            i32 x, y, z, w;
        };
        struct{
            Vec3_i32 xy;
            Vec3_i32 zw;
        };
        struct{
            Vec3_i32 _x;
            Vec3_i32 yz;
            Vec3_i32 _w;
        };
        struct{
            Vec3_i32 rgb;
            i32 __a;
        };
        struct{
            Vec3_i32 xyz;
            i32 __w;
        };
        struct{
            i32 __x;
            Vec3_i32 yzw;
        };
        struct{
            i32 v[4];
        };
    };
};

typedef Vec2_f32 Vec2;
typedef Vec3_f32 Vec3;
typedef Vec4_f32 Vec4;

////////////////////////////////

struct Rect_f32{
    union{
        struct{
            f32 x0;
            f32 y0;
            f32 x1;
            f32 y1;
        };
        struct{
            Vec2_f32 p0;
            Vec2_f32 p1;
        };
    };
};

struct Rect_i32{
    union{
        struct{
            i32 x0;
            i32 y0;
            i32 x1;
            i32 y1;
        };
        struct{
            Vec2_i32 p0;
            Vec2_i32 p1;
        };
    };
};

typedef Rect_f32 f32_Rect;
typedef Rect_i32 i32_Rect;

#endif

// BOTTOM

