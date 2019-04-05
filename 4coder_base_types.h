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
#define clamp(a,x,b) (((a)>(x))?(a):(((b)<(x))?(b):(x)))

#define HasFlag(field,flag) ((field)&(flag))

#define ArrayCount(a) ((sizeof(a))/(sizeof(*a)))
#define ExpandArray(a) (a), (ArrayCount(a))
#define AllowLocal(c) (void)(c)
#if !defined(Member)
# define Member(S,m) (((S*)0)->m)
#endif
#define PtrDif(a,b) ((u8*)(a) - (u8*)(b))
#define PtrAsInt(a) PtrDif(a,0)
#define HandleAsU64(a) (uint64_t)(PtrAsInt(a))
#define NullMember(S,m) (&Member(S,m))
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#define CastFromMember(S,m,ptr) (S*)( (u8*)(ptr) - OffsetOfMember(S,m) )
#define IntAsPtr(a) (void*)(((u8*)0) + a)

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

#define require(c) Stmnt( if (!(c)){ return(0); } )

#define Bytes(x) ((umem)x)
#define KB(x) (((umem)x) << 10)
#define MB(x) (((umem)x) << 20)
#define GB(x) (((umem)x) << 30)
#define TB(x) (((umem)x) << 40)

#define Thousand(x) (x*1000)
#define Million(x)  (x*1000000)
#define Billion(x)  (x*1000000000)

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

static const f32 max_f32 = 3.4028234e+38f;
static const f32 min_f32 = 1.1754943e-38f;

static const f64 max_f64 = 1.7976931348623157e+308;
static const f64 min_f64 = 2.2250738585072014e-308;

////////////////////////////////

static const u32 bit_0 = (((u32)1) << 0);
static const u32 bit_1 = (((u32)1) << 1);
static const u32 bit_2 = (((u32)1) << 2);
static const u32 bit_3 = (((u32)1) << 3);
static const u32 bit_4 = (((u32)1) << 4);
static const u32 bit_5 = (((u32)1) << 5);
static const u32 bit_6 = (((u32)1) << 6);
static const u32 bit_7 = (((u32)1) << 7);

static const u32 bit_8 = (((u32)1) << 8);
static const u32 bit_9 = (((u32)1) << 9);
static const u32 bit_10 = (((u32)1) << 10);
static const u32 bit_11 = (((u32)1) << 11);
static const u32 bit_12 = (((u32)1) << 12);
static const u32 bit_13 = (((u32)1) << 13);
static const u32 bit_14 = (((u32)1) << 14);
static const u32 bit_15 = (((u32)1) << 15);

static const u32 bit_16 = (((u32)1) << 16);
static const u32 bit_17 = (((u32)1) << 17);
static const u32 bit_18 = (((u32)1) << 18);
static const u32 bit_19 = (((u32)1) << 19);
static const u32 bit_20 = (((u32)1) << 20);
static const u32 bit_21 = (((u32)1) << 21);
static const u32 bit_22 = (((u32)1) << 22);
static const u32 bit_23 = (((u32)1) << 23);

static const u32 bit_24 = (((u32)1) << 24);
static const u32 bit_25 = (((u32)1) << 25);
static const u32 bit_26 = (((u32)1) << 26);
static const u32 bit_27 = (((u32)1) << 27);
static const u32 bit_28 = (((u32)1) << 28);
static const u32 bit_29 = (((u32)1) << 29);
static const u32 bit_30 = (((u32)1) << 30);
static const u32 bit_31 = (((u32)1) << 31);

static const u64 bit_32 = (((u64)1) << (0 + 32));
static const u64 bit_33 = (((u64)1) << (1 + 32));
static const u64 bit_34 = (((u64)1) << (2 + 32));
static const u64 bit_35 = (((u64)1) << (3 + 32));
static const u64 bit_36 = (((u64)1) << (4 + 32));
static const u64 bit_37 = (((u64)1) << (5 + 32));
static const u64 bit_38 = (((u64)1) << (6 + 32));
static const u64 bit_39 = (((u64)1) << (7 + 32));

static const u64 bit_40 = (((u64)1) << (8 + 32));
static const u64 bit_41 = (((u64)1) << (9 + 32));
static const u64 bit_42 = (((u64)1) << (10 + 32));
static const u64 bit_43 = (((u64)1) << (11 + 32));
static const u64 bit_44 = (((u64)1) << (12 + 32));
static const u64 bit_45 = (((u64)1) << (13 + 32));
static const u64 bit_46 = (((u64)1) << (14 + 32));
static const u64 bit_47 = (((u64)1) << (15 + 32));

static const u64 bit_48 = (((u64)1) << (16 + 32));
static const u64 bit_49 = (((u64)1) << (17 + 32));
static const u64 bit_50 = (((u64)1) << (18 + 32));
static const u64 bit_51 = (((u64)1) << (19 + 32));
static const u64 bit_52 = (((u64)1) << (20 + 32));
static const u64 bit_53 = (((u64)1) << (21 + 32));
static const u64 bit_54 = (((u64)1) << (22 + 32));
static const u64 bit_55 = (((u64)1) << (23 + 32));

static const u64 bit_56 = (((u64)1) << (24 + 32));
static const u64 bit_57 = (((u64)1) << (25 + 32));
static const u64 bit_58 = (((u64)1) << (26 + 32));
static const u64 bit_59 = (((u64)1) << (27 + 32));
static const u64 bit_60 = (((u64)1) << (28 + 32));
static const u64 bit_61 = (((u64)1) << (29 + 32));
static const u64 bit_62 = (((u64)1) << (30 + 32));
static const u64 bit_63 = (((u64)1) << (31 + 32));

////////////////////////////////

#define dll_init_sentinel(s) (s)->next=(s),(s)->prev=(s)
#define dll_insert(p,n)      (n)->next=(p)->next,(n)->prev=(p),(p)->next=(n),(n)->next->prev=(n)
#define dll_insert_back(p,n) (n)->prev=(p)->prev,(n)->next=(p),(p)->prev=(n),(n)->prev->next=(n)
#define dll_remove(n)        (n)->next->prev=(n)->prev,(n)->prev->next=(n)->next,(n)->next=(n)->prev=0

#define zdll_push_back_(f,l,n) if(f==0){n->next=n->prev=0;f=l=n;}else{n->prev=l;n->next=0;l->next=n;l=n;}
#define zdll_push_front_(f,l,n) if(f==0){n->prev=n->next=0;f=l=n;}else{n->next=l;n->prev=0;l->prev=n;l=n;}
#define zdll_remove_front_(f,l,n) if(f==l){f=l=0;}else{f=f->next;f->prev=0;}
#define zdll_remove_(f,l,n) if(f==n){zdll_remove_front_(f,l,n);}else if(l==n){zdll_remove_back_(f,l,n);}else{dll_remove(n);}

#define zdll_push_back(f,l,n) Stmnt( zdll_push_back_((f),(l),(n)) )
#define zdll_push_front(f,l,n) Stmnt( zdll_push_front_((f),(l),(n)) )
#define zdll_remove_back_(f,l,n) if(f==l){f=l=0;}else{l=l->prev;l->next=0;}
#define zdll_remove(f,l,n) Stmnt( zdll_remove_((f),(l),(n)) )

#define sll_clear(f,l) (f)=(l)=0
#define sll_push(f,l,n) Stmnt( if((f)==0||(l)==0){(f)=(l)=(n);}else{(l)->next=(n);(l)=(n);}(n)->next=0; )
#define sll_pop(f,l) Stmnt( if((f)!=(l)){(f)=(f)->next;}else{(f)=(l)=0;} )

#define sll_init_sentinel(s) Stmnt( (s)->next=(s); )
#define sll_insert(p,v) Stmnt( (v)->next=(p)->next; (p)->next = (v); )
#define sll_remove(p,v) Stmnt( Assert((p)->next == (v)); (p)->next = (v)->next; )

struct Node{
    Node *next;
    Node *prev;
};

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

struct f32_Rect_Pair
{
    f32_Rect E[2];
};

typedef f32_Rect_Pair Rect_f32_Pair;


typedef i32 Coordinate;
typedef i32 Side;

enum{
    Coordinate_X = 0,
    Coordinate_Y = 1,
    Coordinate_Z = 2,
    Coordinate_W = 3,
};

enum{
    Side_Min = 0,
    Side_Max = 1,
};

#endif

// BOTTOM

