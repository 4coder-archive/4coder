/*
 * 4coder base types
 */

// TOP

#if !defined(FCODER_BASE_TYPES)
#define FCODER_BASE_TYPES

////////////////////////////////

#if defined(_MSC_VER)

# define COMPILER_CL 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_IX86)
#  define ARCH_X86 1
# elif defined(_M_ARM64)
#  define ARCH_ARM64 1
# elif defined(_M_ARM)
#  define ARCH_ARM32 1
# else
#  error architecture not supported yet
# endif

#elif defined(__clang__)

# define COMPILER_CLANG 1

# if defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error architecture not supported yet
# endif

#elif defined(__GNUC__) || defined(__GNUG__)

# define COMPILER_GCC 1

# if defined(__gnu_linux__)
#  define OS_LINUX 1
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error architecture not supported yet
# endif

#else
# error This compiler is not supported yet
#endif

#if defined(ARCH_X64)
# define ARCH_64BIT 1
#elif defined(ARCH_X86)
# define ARCH_32BIT 1

#endif

// zeroify

#if !defined(ARCH_32BIT)
#define ARCH_32BIT 0
#endif
#if !defined(ARCH_64BIT)
#define ARCH_64BIT 0
#endif
#if !defined(ARCH_X64)
#define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
#define ARCH_X86 0
#endif
#if !defined(ARCH_ARM64)
#define ARCH_ARM64 0
#endif
#if !defined(ARCH_ARM32)
#define ARCH_ARM32 0
#endif
#if !defined(COMPILER_CL)
#define COMPILER_CL 0
#endif
#if !defined(COMPILER_GCC)
#define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
#define COMPILER_CLANG 0
#endif
#if !defined(OS_WINDOWS)
#define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
#define OS_LINUX 0
#endif
#if !defined(OS_MAC)
#define OS_MAC 0
#endif

#if !defined(SHIP_MODE)
#define SHIP_MODE 0
#else
#undef SHIP_MODE
#define SHIP_MODE 1
#endif

// names

#if COMPILER_CL
# define COMPILER_NAME "cl"
#elif COMPILER_CLANG
# define COMPILER_NAME "clang"
#elif COMPILER_GCC
# define COMPILER_NAME "gcc"
#else
# error no name for this compiler
#endif

#if OS_WINDOWS
# define OS_NAME "win"
#elif OS_LINUX
# define OS_NAME "linux"
#elif OS_MAC
# define OS_NAME "mac"
#else
# error no name for this operating system
#endif

#if ARCH_X86
# define ARCH_NAME "x86"
#elif ARCH_X64
# define ARCH_NAME "x64"
#elif ARCH_ARM64
# define ARCH_NAME "arm64"
#elif ARCH_ARM32
# define ARCH_NAME "arm32"
#else
# error no name for this architecture
#endif

////////////////////////////////

#if COMPILER_CL
# if _MSC_VER <= 1800
#  define snprintf _snprintf
# endif

# if (_MSC_VER <= 1500)
#  define JUST_GUESS_INTS
# endif
#endif

// NOTE(yuval): Changed this so that CALL_CONVENTION will be defined for all platforms
#if ARCH_32BIT && OS_WINDOWS
# define CALL_CONVENTION __stdcall
#else
# define CALL_CONVENTION
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
typedef i64 b64;

typedef float f32;
typedef double f64;

typedef void Void_Func(void);

typedef i32 Generated_Group;
enum{
    GeneratedGroup_Core,
    GeneratedGroup_Custom
};

#define glue_(a,b) a##b
#define glue(a,b) glue_(a,b)

#define stringify_(a) #a
#define stringify(a) stringify_(a)

#if COMPILER_CL
#define __VA_OPT__(x)
#endif

#define function static
#define api(x)

#define internal static
#define local_persist static
#define global static
#define local_const static const
#define global_const static const
#define external extern "C"

#define ArrayCount(a) ((sizeof(a))/(sizeof(*a)))
#define ArraySafe(a,i) ((a)[(i)%ArrayCount(a)])
#define ExpandArray(a) (a), (ArrayCount(a))
#define FixSize(s) struct{ u8 __size_fixer__[s]; }

#define PtrDif(a,b) ((u8*)(a) - (u8*)(b))
#define PtrAsInt(a) PtrDif(a,0)
#define HandleAsU64(a) (u64)(PtrAsInt(a))
#define Member(S,m) (((S*)0)->m)
#define NullMember(S,m) (&Member(S,m))
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#define OffsetOfMemberStruct(s,m) PtrDif(&(s)->m, (s))
#define SizeAfterMember(S,m) (sizeof(S) - OffsetOfMember(S,m))
#define CastFromMember(S,m,ptr) (S*)( (u8*)(ptr) - OffsetOfMember(S,m) )
#define IntAsPtr(a) (void*)(((u8*)0) + a)

#define Stmnt(s) do{ s }while(0)

// NOTE(allen): Assert notes:
// Break = the run time implementation of break
//                - replace this to get fancier behavior on assert
// Always = assert that is not compiled out in SHIP_MODE
//                - helpful for debugging specific issues
//                - used rarely in normal code
// Message = unconditional asserts with an attached message
//                - InvalidPath version for paths of a switch or if-else dispatch that should always be unreachable
//                - NotImplemented version for stubs functions that are not yet completed
// Static = asserts that contain only compile time constants and become compilation errors
// Disambiguate = for static asserts that happen to have name conflicts

#define AssertBreak(m) (*((i32*)0) = 0xA11E)
#define AssertAlways(c) Stmnt( if (!(c)) { AssertBreak(c); } )
#define AssertMessageAlways(m) AssertBreak(m)
#define StaticAssertDisambiguateAlways(c,d) char glue(__ignore__, glue(__LINE__, d))[(c)?1:-1];
#define StaticAssertAlways(c) StaticAssertDisambiguateAlways(c,__default__)

#if !SHIP_MODE
#define Assert(c) AssertAlways(c)
#define AssertMessage(m) AssertMessageAlways(m)
#define StaticAssertDisambiguate(c,d) StaticAssertDisambiguateAlways(c,d)
#define StaticAssert(c) StaticAssertAlways(c)
#else
#define Assert(c)
#define AssertMessage(m)
#define StaticAssertDisambiguate(c,d)
#define StaticAssert(c)
#endif

#define AssertImplies(a,b) Assert(!(a) || (b))
#define InvalidPath AssertMessage("invalid path")
#define NotImplemented AssertMessage("not implemented")
#define DontCompile NoSeriouslyDontCompile

#define  B(x)  (x)
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) (((u64)x) << 40)

#define Thousand(x) ((x)*1000)
#define Million(x)  ((x)*1000000)
#define Billion(x)  ((x)*1000000000)

#define HasFlag(fi,fl) (((fi)&(fl))!=0)
#define HasAllFlag(fi,fl) (((fi)&(fl))==(fl))
#define AddFlag(fi,fl) ((fi)|=(fl))
#define RemFlag(fi,fl) ((fi)&=(~(fl)))
#define MovFlag(fi1,fl1,fi2,fl2) ((HasFlag(fi1,fl1))?(AddFlag(fi2,fl2)):(fi2))

#define Swap(t,a,b) do { t glue(hidden_temp_,__LINE__) = a; a = b; b = glue(hidden_temp_,__LINE__); } while(0)

#define div_round_up_positive_(n,d) (n + d - 1)/d
#define div_round_up_positive(n,d) (div_round_up_positive_((n),(d)))

#define DrCase(PC) case PC: goto resumespot_##PC
#define DrYield(PC, n) { *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }
#define DrReturn(n) { *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

#define Max(a,b) (((a)>(b))?(a):(b))
#define Min(a,b) (((a)<(b))?(a):(b))
#define clamp_top(a,b) Min(a,b)
#define clamp_bot(a,b) Max(a,b)
#define clamp_(a,x,b) ((a>x)?a:((b<x)?b:x))
#define clamp(a,x,b) clamp_((a),(x),(b))

#define array_initr(a) {(a), ArrayCount(a)}

global_const u8 max_u8 = 0xFF;
global_const u16 max_u16 = 0xFFFF;
global_const u32 max_u32 = 0xFFFFFFFF;
global_const u64 max_u64 = 0xFFFFFFFFFFFFFFFF;

global_const i8 max_i8 = 127;
global_const i16 max_i16 = 32767;
global_const i32 max_i32 = 2147483647;
global_const i64 max_i64 = 9223372036854775807;

global_const i8 min_i8   = -127 - 1;
global_const i16 min_i16 = -32767 - 1;
global_const i32 min_i32 = -2147483647 - 1;
global_const i64 min_i64 = -9223372036854775807 - 1;

global_const f32 max_f32 = 3.402823466e+38f;
global_const f32 min_f32 = -max_f32;
global_const f32 smallest_positive_f32 = 1.1754943508e-38f;
global_const f32 epsilon_f32 = 5.96046448e-8f;

global_const f32 pi_f32 = 3.14159265359f;
global_const f32 half_pi_f32 = 1.5707963267f;

global_const f64 max_f64 = 1.79769313486231e+308;
global_const f64 min_f64 = -max_f64;
global_const f64 smallest_positive_f64 = 4.94065645841247e-324;
global_const f64 epsilon_f64 = 1.11022302462515650e-16;

#define clamp_signed_to_i8(x) (i8)(clamp((i64)i8_min, (i64)(x), (i64)i8_max))
#define clamp_signed_to_i16(x) (i16)(clamp((i64)i16_min, (i64)(x), (i64)i16_max))
#define clamp_signed_to_i32(x) (i32)(clamp((i64)i32_min, (i64)(x), (i64)i32_max))
#define clamp_signed_to_i64(x) (i64)(clamp((i64)i64_min, (i64)(x), (i64)i64_max))
#define clamp_unsigned_to_i8(x) (i8)(clamp_top((u64)(x), (u64)i8_max))
#define clamp_unsigned_to_i16(x) (i16)(clamp_top((u64)(x), (u64)i16_max))
#define clamp_unsigned_to_i32(x) (i32)(clamp_top((u64)(x), (u64)i32_max))
#define clamp_unsigned_to_i64(x) (i64)(clamp_top((u64)(x), (u64)i64_max))
#define clamp_signed_to_u8(x) (u8)(clamp_top((u64)clamp_bot(0, (i64)(x)), (u64)u8_max))
#define clamp_signed_to_u16(x) (u16)(clamp_top((u64)clamp_bot(0, (i64)(x)), (u64)u16_max))
#define clamp_signed_to_u32(x) (u32)(clamp_top((u64)clamp_bot(0, (i64)(x)), (u64)u32_max))
#define clamp_signed_to_u64(x) (u64)(clamp_top((u64)clamp_bot(0, (i64)(x)), (u64)u64_max))
#define clamp_unsigned_to_u8(x) (u8)(clamp_top((u64)(x), (u64)u8_max))
#define clamp_unsigned_to_u16(x) (u16)(clamp_top((u64)(x), (u64)u16_max))
#define clamp_unsigned_to_u32(x) (u32)(clamp_top((u64)(x), (u64)u32_max))
#define clamp_unsigned_to_u64(x) (u64)(clamp_top((u64)(x), (u64)u64_max))

#define line_number_as_string stringify(__LINE__)
#define file_name_line_number __FILE__ ":" line_number_as_string ":"

#define require(c) Stmnt( if (!(c)){ return(0); } )

////////////////////////////////

global_const u32 bit_1  = 0x00000001;
global_const u32 bit_2  = 0x00000002;
global_const u32 bit_3  = 0x00000004;
global_const u32 bit_4  = 0x00000008;
global_const u32 bit_5  = 0x00000010;
global_const u32 bit_6  = 0x00000020;
global_const u32 bit_7  = 0x00000040;
global_const u32 bit_8  = 0x00000080;
global_const u32 bit_9  = 0x00000100;
global_const u32 bit_10 = 0x00000200;
global_const u32 bit_11 = 0x00000400;
global_const u32 bit_12 = 0x00000800;
global_const u32 bit_13 = 0x00001000;
global_const u32 bit_14 = 0x00002000;
global_const u32 bit_15 = 0x00004000;
global_const u32 bit_16 = 0x00008000;
global_const u32 bit_17 = 0x00010000;
global_const u32 bit_18 = 0x00020000;
global_const u32 bit_19 = 0x00040000;
global_const u32 bit_20 = 0x00080000;
global_const u32 bit_21 = 0x00100000;
global_const u32 bit_22 = 0x00200000;
global_const u32 bit_23 = 0x00400000;
global_const u32 bit_24 = 0x00800000;
global_const u32 bit_25 = 0x01000000;
global_const u32 bit_26 = 0x02000000;
global_const u32 bit_27 = 0x04000000;
global_const u32 bit_28 = 0x08000000;
global_const u32 bit_29 = 0x10000000;
global_const u32 bit_30 = 0x20000000;
global_const u32 bit_31 = 0x40000000;
global_const u32 bit_32 = 0x80000000;

global_const u64 bit_33 = 0x0000000100000000;
global_const u64 bit_34 = 0x0000000200000000;
global_const u64 bit_35 = 0x0000000400000000;
global_const u64 bit_36 = 0x0000000800000000;
global_const u64 bit_37 = 0x0000001000000000;
global_const u64 bit_38 = 0x0000002000000000;
global_const u64 bit_39 = 0x0000004000000000;
global_const u64 bit_40 = 0x0000008000000000;
global_const u64 bit_41 = 0x0000010000000000;
global_const u64 bit_42 = 0x0000020000000000;
global_const u64 bit_43 = 0x0000040000000000;
global_const u64 bit_44 = 0x0000080000000000;
global_const u64 bit_45 = 0x0000100000000000;
global_const u64 bit_46 = 0x0000200000000000;
global_const u64 bit_47 = 0x0000400000000000;
global_const u64 bit_48 = 0x0000800000000000;
global_const u64 bit_49 = 0x0001000000000000;
global_const u64 bit_50 = 0x0002000000000000;
global_const u64 bit_51 = 0x0004000000000000;
global_const u64 bit_52 = 0x0008000000000000;
global_const u64 bit_53 = 0x0010000000000000;
global_const u64 bit_54 = 0x0020000000000000;
global_const u64 bit_55 = 0x0040000000000000;
global_const u64 bit_56 = 0x0080000000000000;
global_const u64 bit_57 = 0x0100000000000000;
global_const u64 bit_58 = 0x0200000000000000;
global_const u64 bit_59 = 0x0400000000000000;
global_const u64 bit_60 = 0x0800000000000000;
global_const u64 bit_61 = 0x1000000000000000;
global_const u64 bit_62 = 0x2000000000000000;
global_const u64 bit_63 = 0x4000000000000000;
global_const u64 bit_64 = 0x8000000000000000;

global_const u32 bitmask_1  = 0x00000001;
global_const u32 bitmask_2  = 0x00000003;
global_const u32 bitmask_3  = 0x00000007;
global_const u32 bitmask_4  = 0x0000000f;
global_const u32 bitmask_5  = 0x0000001f;
global_const u32 bitmask_6  = 0x0000003f;
global_const u32 bitmask_7  = 0x0000007f;
global_const u32 bitmask_8  = 0x000000ff;
global_const u32 bitmask_9  = 0x000001ff;
global_const u32 bitmask_10 = 0x000003ff;
global_const u32 bitmask_11 = 0x000007ff;
global_const u32 bitmask_12 = 0x00000fff;
global_const u32 bitmask_13 = 0x00001fff;
global_const u32 bitmask_14 = 0x00003fff;
global_const u32 bitmask_15 = 0x00007fff;
global_const u32 bitmask_16 = 0x0000ffff;
global_const u32 bitmask_17 = 0x0001ffff;
global_const u32 bitmask_18 = 0x0003ffff;
global_const u32 bitmask_19 = 0x0007ffff;
global_const u32 bitmask_20 = 0x000fffff;
global_const u32 bitmask_21 = 0x001fffff;
global_const u32 bitmask_22 = 0x003fffff;
global_const u32 bitmask_23 = 0x007fffff;
global_const u32 bitmask_24 = 0x00ffffff;
global_const u32 bitmask_25 = 0x01ffffff;
global_const u32 bitmask_26 = 0x03ffffff;
global_const u32 bitmask_27 = 0x07ffffff;
global_const u32 bitmask_28 = 0x0fffffff;
global_const u32 bitmask_29 = 0x1fffffff;
global_const u32 bitmask_30 = 0x3fffffff;
global_const u32 bitmask_31 = 0x7fffffff;

global_const u64 bitmask_32 = 0x00000000ffffffff;
global_const u64 bitmask_33 = 0x00000001ffffffff;
global_const u64 bitmask_34 = 0x00000003ffffffff;
global_const u64 bitmask_35 = 0x00000007ffffffff;
global_const u64 bitmask_36 = 0x0000000fffffffff;
global_const u64 bitmask_37 = 0x0000001fffffffff;
global_const u64 bitmask_38 = 0x0000003fffffffff;
global_const u64 bitmask_39 = 0x0000007fffffffff;
global_const u64 bitmask_40 = 0x000000ffffffffff;
global_const u64 bitmask_41 = 0x000001ffffffffff;
global_const u64 bitmask_42 = 0x000003ffffffffff;
global_const u64 bitmask_43 = 0x000007ffffffffff;
global_const u64 bitmask_44 = 0x00000fffffffffff;
global_const u64 bitmask_45 = 0x00001fffffffffff;
global_const u64 bitmask_46 = 0x00003fffffffffff;
global_const u64 bitmask_47 = 0x00007fffffffffff;
global_const u64 bitmask_48 = 0x0000ffffffffffff;
global_const u64 bitmask_49 = 0x0001ffffffffffff;
global_const u64 bitmask_50 = 0x0003ffffffffffff;
global_const u64 bitmask_51 = 0x0007ffffffffffff;
global_const u64 bitmask_52 = 0x000fffffffffffff;
global_const u64 bitmask_53 = 0x001fffffffffffff;
global_const u64 bitmask_54 = 0x003fffffffffffff;
global_const u64 bitmask_55 = 0x007fffffffffffff;
global_const u64 bitmask_56 = 0x00ffffffffffffff;
global_const u64 bitmask_57 = 0x01ffffffffffffff;
global_const u64 bitmask_58 = 0x03ffffffffffffff;
global_const u64 bitmask_59 = 0x07ffffffffffffff;
global_const u64 bitmask_60 = 0x0fffffffffffffff;
global_const u64 bitmask_61 = 0x1fffffffffffffff;
global_const u64 bitmask_62 = 0x3fffffffffffffff;
global_const u64 bitmask_63 = 0x7fffffffffffffff;

////////////////////////////////

struct Node{
    Node *next;
    Node *prev;
};
union SNode{
    SNode *next;
    SNode *prev;
};

#define dll_init_sentinel_NP_(s,next,prev) s->next=s,s->prev=s
#define dll_insert_NP_(p,n1,n2,next,prev) n2->next=p->next,n1->prev=p,p->next->prev=n2,p->next=n1
#define dll_remove_NP_(n1,n2,next,prev) n2->next->prev=n1->prev,n1->prev->next=n2->next,n2->next=n1->prev=0

#define dll_init_sentinel_(s) dll_init_sentinel_NP_(s,next,prev)
#define dll_insert_(p,n) dll_insert_NP_(p,n,n,next,prev)
#define dll_insert_multiple_(p,n1,n2) dll_insert_NP_(p,n1,n2,next,prev)
#define dll_insert_back_(p,n) dll_insert_NP_(p,n,n,prev,next)
#define dll_insert_multiple_back_(p,n1,n2) dll_insert_NP_(p,n2,n1,prev,next)
#define dll_remove_(n) dll_remove_NP_(n,n,next,prev)
#define dll_remove_multiple_(n1,n2) dll_remove_NP_(n1,n2,next,prev)

#define dll_init_sentinel(s) (dll_init_sentinel_((s)))
#define dll_insert(p,n) (dll_insert_((p),(n)))
#define dll_insert_multiple(p,n1,n2) (dll_insert_multiple_((p),(n1),(n2)))
#define dll_insert_back(p,n) (dll_insert_back_((p),(n)))
#define dll_insert_multiple_back(p,n1,n2) (dll_insert_multiple_back_((p),(n1),(n2)))
#define dll_remove(n) (dll_remove_((n)))
#define dll_remove_multiple(n1,n2) (dll_remove_multiple_((n1),(n2)))

#define sll_stack_push_(h,n) n->next=h,h=n
#define sll_stack_pop_(h) h=h=h->next
#define sll_queue_push_multiple_(f,l,ff,ll) if(ll){if(f){l->next=ff;}else{f=ff;}l=ll;l->next=0;}
#define sll_queue_push_(f,l,n) sll_queue_push_multiple_(f,l,n,n)
#define sll_queue_pop_(f,l) if (f==l) { f=l=0; } else { f=f->next; }

#define sll_stack_push(h,n) (sll_stack_push_((h),(n)))
#define sll_stack_pop(h) (sll_stack_pop_((h)))
#define sll_queue_push_multiple(f,l,ff,ll) Stmnt( sll_queue_push_multiple_((f),(l),(ff),(ll)) )
#define sll_queue_push(f,l,n) Stmnt( sll_queue_push_((f),(l),(n)) )
#define sll_queue_pop(f,l) Stmnt( sll_queue_pop_((f),(l)) )

#define zdll_push_back_NP_(f,l,n,next,prev) ((f==0)?(n->next=n->prev=0,f=l=n):(n->prev=l,n->next=0,l->next=n,l=n))
#define zdll_remove_back_NP_(f,l,next,prev) ((f==l)?(f=l=0):(l->prev->next=0,l=l->prev))
#define zdll_remove_NP_(f,l,n,next,prev)       \
((l==n)?(zdll_remove_back_NP_(f,l,next,prev))  \
:(f==n)?(zdll_remove_back_NP_(l,f,prev,next)) \
:       (dll_remove_NP_(n,n,next,prev)))

#define zdll_push_back(f,l,n) zdll_push_back_NP_((f),(l),(n),next,prev)
#define zdll_push_front(f,l,n) zdll_push_back_NP_((l),(f),(n),prev,next)
#define zdll_remove_back(f,l) zdll_remove_back_NP_((f),(l),next,prev)
#define zdll_remove_front(f,l) zdll_remove_back_NP_((l),(f),prev,next)
#define zdll_remove(f,l,n) zdll_remove_NP_((f),(l),(n),next,prev)

#define zdll_assert_good(T,f) Stmnt( if (f != 0){ Assert(f->prev == 0); \
for(T *p_ = f; p_ != 0; p_ = p_->next){ Assert(p_->prev == 0 || p_->prev->next == p_); Assert(p_->next == 0 || p_->next->prev == p_); }  } )

////////////////////////////////

union Vec2_i8{
    struct{
        i8 x;
        i8 y;
    };
    i8 v[2];
};
union Vec3_i8{
    struct{
        i8 x;
        i8 y;
        i8 z;
    };
    struct{
        i8 r;
        i8 g;
        i8 b;
    };
    i8 v[3];
};
union Vec4_i8{
    struct{
        i8 x;
        i8 y;
        i8 z;
        i8 w;
    };
    struct{
        i8 r;
        i8 g;
        i8 b;
        i8 a;
    };
    i8 v[4];
};
union Vec2_i16{
    struct{
        i16 x;
        i16 y;
    };
    i16 v[2];
};
union Vec3_i16{
    struct{
        i16 x;
        i16 y;
        i16 z;
    };
    struct{
        i16 r;
        i16 g;
        i16 b;
    };
    i16 v[3];
};
union Vec4_i16{
    struct{
        i16 x;
        i16 y;
        i16 z;
        i16 w;
    };
    struct{
        i16 r;
        i16 g;
        i16 b;
        i16 a;
    };
    i16 v[4];
};
union Vec2_i32{
    struct{
        i32 x;
        i32 y;
    };
    i32 v[2];
};
union Vec3_i32{
    struct{
        i32 x;
        i32 y;
        i32 z;
    };
    struct{
        i32 r;
        i32 g;
        i32 b;
    };
    i32 v[3];
};
union Vec4_i32{
    struct{
        i32 x;
        i32 y;
        i32 z;
        i32 w;
    };
    struct{
        i32 r;
        i32 g;
        i32 b;
        i32 a;
    };
    i32 v[4];
};
union Vec2_f32{
    struct{
        f32 x;
        f32 y;
    };
    f32 v[2];
};
union Vec3_f32{
    struct{
        f32 x;
        f32 y;
        f32 z;
    };
    struct{
        f32 r;
        f32 g;
        f32 b;
    };
    f32 v[3];
};
union Vec4_f32{
    struct{
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct{
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    struct{
        f32 h;
        f32 s;
        f32 l;
        f32 __a;
    };
    f32 v[4];
};

union Range_i32{
    struct{
        i32 min;
        i32 max;
    };
    struct{
        i32 start;
        i32 end;
    };
    struct{
        i32 first;
        i32 one_past_last;
    };
};
union Range_i64{
    struct{
        i64 min;
        i64 max;
    };
    struct{
        i64 start;
        i64 end;
    };
    struct{
        i64 first;
        i64 one_past_last;
    };
};
union Range_u64{
    struct{
        u64 min;
        u64 max;
    };
    struct{
        u64 start;
        u64 end;
    };
    struct{
        u64 first;
        u64 one_past_last;
    };
};
union Range_f32{
    struct{
        f32 min;
        f32 max;
    };
    struct{
        f32 start;
        f32 end;
    };
    struct{
        f32 first;
        f32 one_past_last;
    };
};

struct Range_i32_Array{
    Range_i32 *ranges;
    i32 count;
};
struct Range_i64_Array{
    Range_i64 *ranges;
    i32 count;
};
struct Range_u64_Array{
    Range_u64 *ranges;
    i32 count;
};
struct Range_f32_Array{
    Range_f32 *ranges;
    i32 count;
};

union Rect_i32{
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
    Vec2_i32 p[2];
};
union Rect_f32{
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
    Vec2_f32 p[2];
};

union Rect_f32_Pair{
    struct{
        Rect_f32 a;
        Rect_f32 b;
    };
    struct{
        Rect_f32 min;
        Rect_f32 max;
    };
    struct{
        Rect_f32 e[2];
    };
};

typedef u32 ARGB_Color;

////////////////////////////////

struct i8_Array{
    i8 *vals;
    i32 count;
};
struct i16_Array{
    i16 *vals;
    i32 count;
};
struct i32_Array{
    i32 *vals;
    i32 count;
};
struct i64_Array{
    i64 *vals;
    i32 count;
};

struct u8_Array{
    u8 *vals;
    i32 count;
};
struct u16_Array{
    u16 *vals;
    i32 count;
};
struct u32_Array{
    u32 *vals;
    i32 count;
};
struct u64_Array{
    u64 *vals;
    i32 count;
};

////////////////////////////////

typedef i32 String_Fill_Terminate_Rule;
enum{
    StringFill_NoTerminate = 0,
    StringFill_NullTerminate = 1,
};
typedef u32 String_Separator_Flag;
enum{
    StringSeparator_NoFlags = 0,
};
enum{
    StringSeparator_BeforeFirst = 1,
    StringSeparator_AfterLast = 2,
};
typedef i32 String_Match_Rule;
enum{
    StringMatch_Exact = 0,
    StringMatch_CaseInsensitive = 1,
};

struct String_Const_char{
    char *str;
    u64 size;
};
struct String_Const_u8{
    union{
        void *data;
        u8 *str;
    };
    u64 size;
};
struct String_Const_u16{
    u16 *str;
    u64 size;
};
struct String_Const_u32{
    u32 *str;
    u64 size;
};

struct String_Const_char_Array{
    union{
        String_Const_char *strings;
        String_Const_char *vals;
    };
    i32 count;
};
struct String_Const_u8_Array{
    union{
        String_Const_u8 *strings;
        String_Const_u8 *vals;
    };
    i32 count;
};
struct String_Const_u16_Array{
    union{
        String_Const_u16 *strings;
        String_Const_u16 *vals;
    };
    i32 count;
};
struct String_Const_u32_Array{
    union{
        String_Const_u32 *strings;
        String_Const_u32 *vals;
    };
    i32 count;
};

typedef i32 String_Encoding;
enum{
    StringEncoding_ASCII = 0,
    StringEncoding_UTF8  = 1,
    StringEncoding_UTF16 = 2,
    StringEncoding_UTF32 = 3,
};

struct String_Const_Any{
    String_Encoding encoding;
    union{
        struct{
            void *str;
            u64 size;
        };
        String_Const_char s_char;
        String_Const_u8 s_u8;
        String_Const_u16 s_u16;
        String_Const_u32 s_u32;
    };
};

#define string_litinit(s) {(s), sizeof(s) - 1}
#define string_u8_litinit(s) {(u8*)(s), sizeof(s) - 1}

struct Node_String_Const_char{
    Node_String_Const_char *next;
    String_Const_char string;
};
struct Node_String_Const_u8{
    Node_String_Const_u8 *next;
    String_Const_u8 string;
};
struct Node_String_Const_u16{
    Node_String_Const_u16 *next;
    String_Const_u16 string;
};
struct Node_String_Const_u32{
    Node_String_Const_u32 *next;
    String_Const_u32 string;
};
struct List_String_Const_char{
    Node_String_Const_char *first;
    Node_String_Const_char *last;
    u64 total_size;
    i32 node_count;
};
struct List_String_Const_u8{
    Node_String_Const_u8 *first;
    Node_String_Const_u8 *last;
    u64 total_size;
    i32 node_count;
};
struct List_String_Const_u16{
    Node_String_Const_u16 *first;
    Node_String_Const_u16 *last;
    u64 total_size;
    i32 node_count;
};
struct List_String_Const_u32{
    Node_String_Const_u32 *first;
    Node_String_Const_u32 *last;
    u64 total_size;
    i32 node_count;
};

struct Node_String_Const_Any{
    Node_String_Const_Any *next;
    String_Const_Any string;
};
struct List_String_Const_Any{
    Node_String_Const_Any *first;
    Node_String_Const_Any *last;
    u64 total_size;
    i32 node_count;
};

struct String_char{
    union{
        String_Const_char string;
        struct{
            char *str;
            u64 size;
        };
    };
    u64 cap;
};
struct String_u8{
    union{
        String_Const_u8 string;
        struct{
            u8 *str;
            u64 size;
        };
    };
    u64 cap;
};
struct String_u16{
    union{
        String_Const_u16 string;
        struct{
            u16 *str;
            u64 size;
        };
    };
    u64 cap;
};
struct String_u32{
    union{
        String_Const_u32 string;
        struct{
            u32 *str;
            u64 size;
        };
    };
    u64 cap;
};

struct String_Any{
    String_Encoding encoding;
    union{
        struct{
            void *str;
            u64 size;
            u64 cap;
        };
        String_char s_char;
        String_u8 s_u8;
        String_u16 s_u16;
        String_u32 s_u32;
    };
};

typedef i32 Line_Ending_Kind;
enum{
    LineEndingKind_Binary,
    LineEndingKind_LF,
    LineEndingKind_CRLF,
};

struct Character_Consume_Result{
    u32 inc;
    u32 codepoint;
};

global u32 surrogate_min = 0xD800;
global u32 surrogate_max = 0xDFFF;

global u32 nonchar_min = 0xFDD0;
global u32 nonchar_max = 0xFDEF;

struct Data{
    u8 *data;
    u64 size;
};

////////////////////////////////

typedef u32 Access_Flag;
enum{
    AccessFlag_Read  = 1,
    AccessFlag_Write = 2,
    AccessFlag_Exec  = 4,
};

typedef i32 Dimension;
enum{
    Dimension_X = 0,
    Dimension_Y = 1,
    Dimension_Z = 2,
    Dimension_W = 3,
};

typedef i32 Coordinate;
enum{
    Coordinate_X = 0,
    Coordinate_Y = 1,
    Coordinate_Z = 2,
    Coordinate_W = 3,
};

typedef i32 Side;
enum{
    Side_Min = 0,
    Side_Max = 1,
};

typedef i32 Scan_Direction;
enum{
    Scan_Backward = -1,
    Scan_Forward  =  1,
};

////////////////////////////////

struct Date_Time{
    u32 year; // Real year, no adjustment
    u8 mon;   // [0,11]
    u8 day;   // [0,30]
    u8 hour;  // [0,23]
    u8 min;   // [0,59]
    u8 sec;   // [0,60]
    u16 msec; // [0,999]
};

global String_Const_u8 month_full_name[] = {
    string_u8_litinit("January"),
    string_u8_litinit("February"),
    string_u8_litinit("March"),
    string_u8_litinit("April"),
    string_u8_litinit("May"),
    string_u8_litinit("June"),
    string_u8_litinit("July"),
    string_u8_litinit("August"),
    string_u8_litinit("September"),
    string_u8_litinit("October"),
    string_u8_litinit("November"),
    string_u8_litinit("December"),
};

global String_Const_u8 month_abrev_name[] = {
    string_u8_litinit("Jan"),
    string_u8_litinit("Feb"),
    string_u8_litinit("Mar"),
    string_u8_litinit("Apr"),
    string_u8_litinit("May"),
    string_u8_litinit("Jun"),
    string_u8_litinit("Jul"),
    string_u8_litinit("Aug"),
    string_u8_litinit("Sep"),
    string_u8_litinit("Oct"),
    string_u8_litinit("Nov"),
    string_u8_litinit("Dec"),
};

global String_Const_u8 ordinal_numeric_name[] = {
    string_u8_litinit("1st"),
    string_u8_litinit("2nd"),
    string_u8_litinit("3rd"),
    string_u8_litinit("4th"),
    string_u8_litinit("5th"),
    string_u8_litinit("6th"),
    string_u8_litinit("7th"),
    string_u8_litinit("8th"),
    string_u8_litinit("9th"),
    string_u8_litinit("10th"),
    string_u8_litinit("11th"),
    string_u8_litinit("12th"),
    string_u8_litinit("13th"),
    string_u8_litinit("14th"),
    string_u8_litinit("15th"),
    string_u8_litinit("16th"),
    string_u8_litinit("17th"),
    string_u8_litinit("18th"),
    string_u8_litinit("19th"),
    string_u8_litinit("20th"),
    string_u8_litinit("21st"),
    string_u8_litinit("22nd"),
    string_u8_litinit("23rd"),
    string_u8_litinit("24th"),
    string_u8_litinit("25th"),
    string_u8_litinit("26th"),
    string_u8_litinit("27th"),
    string_u8_litinit("28th"),
    string_u8_litinit("29th"),
    string_u8_litinit("30th"),
    string_u8_litinit("31st"),
    string_u8_litinit("32nd"),
    string_u8_litinit("33rd"),
    string_u8_litinit("34th"),
    string_u8_litinit("35th"),
    string_u8_litinit("36th"),
    string_u8_litinit("37th"),
    string_u8_litinit("38th"),
    string_u8_litinit("39th"),
    string_u8_litinit("40th"),
    string_u8_litinit("41st"),
    string_u8_litinit("42nd"),
    string_u8_litinit("43rd"),
    string_u8_litinit("44th"),
    string_u8_litinit("45th"),
    string_u8_litinit("46th"),
    string_u8_litinit("47th"),
    string_u8_litinit("48th"),
    string_u8_litinit("49th"),
    string_u8_litinit("50th"),
    string_u8_litinit("51st"),
    string_u8_litinit("52nd"),
    string_u8_litinit("53rd"),
    string_u8_litinit("54th"),
    string_u8_litinit("55th"),
    string_u8_litinit("56th"),
    string_u8_litinit("57th"),
    string_u8_litinit("58th"),
    string_u8_litinit("59th"),
    string_u8_litinit("60th"),
    string_u8_litinit("61st"),
    string_u8_litinit("62nd"),
    string_u8_litinit("63rd"),
    string_u8_litinit("64th"),
    string_u8_litinit("65th"),
    string_u8_litinit("66th"),
    string_u8_litinit("67th"),
    string_u8_litinit("68th"),
    string_u8_litinit("69th"),
    string_u8_litinit("70th"),
    string_u8_litinit("71st"),
    string_u8_litinit("72nd"),
    string_u8_litinit("73rd"),
    string_u8_litinit("74th"),
    string_u8_litinit("75th"),
    string_u8_litinit("76th"),
    string_u8_litinit("77th"),
    string_u8_litinit("78th"),
    string_u8_litinit("79th"),
    string_u8_litinit("80th"),
    string_u8_litinit("81st"),
    string_u8_litinit("82nd"),
    string_u8_litinit("83rd"),
    string_u8_litinit("84th"),
    string_u8_litinit("85th"),
    string_u8_litinit("86th"),
    string_u8_litinit("87th"),
    string_u8_litinit("88th"),
    string_u8_litinit("89th"),
    string_u8_litinit("90th"),
    string_u8_litinit("91st"),
    string_u8_litinit("92nd"),
    string_u8_litinit("93rd"),
    string_u8_litinit("94th"),
    string_u8_litinit("95th"),
    string_u8_litinit("96th"),
    string_u8_litinit("97th"),
    string_u8_litinit("98th"),
    string_u8_litinit("99th"),
    string_u8_litinit("100th"),
};

////////////////////////////////

typedef void *Base_Allocator_Reserve_Signature(void *user_data, u64 size, u64 *size_out, String_Const_u8 location);
typedef void  Base_Allocator_Commit_Signature(void *user_data, void *ptr, u64 size);
typedef void  Base_Allocator_Uncommit_Signature(void *user_data, void *ptr, u64 size);
typedef void  Base_Allocator_Free_Signature(void *user_data, void *ptr);
typedef void  Base_Allocator_Set_Access_Signature(void *user_data, void *ptr, u64 size, Access_Flag flags);
struct Base_Allocator{
    Base_Allocator_Reserve_Signature *reserve;
    Base_Allocator_Commit_Signature *commit;
    Base_Allocator_Uncommit_Signature *uncommit;
    Base_Allocator_Free_Signature *free;
    Base_Allocator_Set_Access_Signature *set_access;
    void *user_data;
};

struct Cursor{
    u8 *base;
    u64 pos;
    u64 cap;
};
struct Temp_Memory_Cursor{
    Cursor *cursor;
    u64 pos;
};
struct Cursor_Node{
    union{
        Cursor_Node *next;
        Cursor_Node *prev;
    };
    Cursor cursor;
};
struct Arena{
    Base_Allocator *base_allocator;
    Cursor_Node *cursor_node;
    u64 chunk_size;
    u64 alignment;
};
struct Temp_Memory_Arena{
    Arena *arena;
    Cursor_Node *cursor_node;
    u64 pos;
};
typedef i32 Linear_Allocator_Kind;
enum{
    LinearAllocatorKind_Cursor,
    LinearAllocatorKind_Arena,
};
struct Temp_Memory{
    Linear_Allocator_Kind kind;
    union{
        Temp_Memory_Cursor temp_memory_cursor;
        Temp_Memory_Arena temp_memory_arena;
    };
};

////////////////////////////////

typedef u64 Profile_ID;
struct Profile_Record{
    Profile_Record *next;
    Profile_ID id;
    u64 time;
    String_Const_u8 location;
    String_Const_u8 name;
};

struct Profile_Thread{
    Profile_Thread *next;
    Profile_Record *first_record;
    Profile_Record *last_record;
    i32 record_count;
    i32 thread_id;
    String_Const_u8 name;
};

typedef u32 Profile_Enable_Flag;
enum{
    ProfileEnable_UserBit    = 0x1,
    ProfileEnable_InspectBit = 0x2,
};

// NOTE(allen): full definition in 4coder_profile.h, due to dependency on System_Mutex.
struct Profile_Global_List;

////////////////////////////////

typedef i32 Thread_Kind;
enum{
    ThreadKind_Main,
    ThreadKind_MainCoroutine,
    ThreadKind_AsyncTasks,
};

struct Arena_Node{
    Arena_Node *next;
    Arena_Node *prev;
    Arena arena;
    i32 ref_counter;
};

struct Thread_Context{
    Thread_Kind kind;
    Base_Allocator *allocator;
    Arena node_arena;
    Arena_Node *used_first;
    Arena_Node *used_last;
    Arena_Node *free_arenas;
    
    Base_Allocator *prof_allocator;
    Profile_ID prof_id_counter;
    Arena prof_arena;
    Profile_Record *prof_first;
    Profile_Record *prof_last;
    i32 prof_record_count;
    
    void *user_data;
};

struct Scratch_Block{
    Thread_Context *tctx;
    Arena *arena;
    Temp_Memory temp;
    
    Scratch_Block(struct Thread_Context *tctx);
    Scratch_Block(struct Thread_Context *tctx, Arena *a1);
    Scratch_Block(struct Thread_Context *tctx, Arena *a1, Arena *a2);
    Scratch_Block(struct Thread_Context *tctx, Arena *a1, Arena *a2, Arena *a3);
    Scratch_Block(struct Application_Links *app);
    Scratch_Block(struct Application_Links *app, Arena *a1);
    Scratch_Block(struct Application_Links *app, Arena *a1, Arena *a2);
    Scratch_Block(struct Application_Links *app, Arena *a1, Arena *a2, Arena *a3);
    ~Scratch_Block();
    operator Arena*();
    void restore(void);
};

struct Temp_Memory_Block{
    Temp_Memory temp;
    Temp_Memory_Block(Temp_Memory temp);
    Temp_Memory_Block(Arena *arena);
    ~Temp_Memory_Block();
    void restore(void);
};

////////////////////////////////

struct Heap_Basic_Node{
    Heap_Basic_Node *next;
    Heap_Basic_Node *prev;
};

struct Heap_Node{
    union{
        struct{
            Heap_Basic_Node order;
            Heap_Basic_Node alloc;
            u64 size;
        };
        u8 force_size__[64];
    };
};

struct Heap{
    Arena arena_;
    Arena *arena;
    Heap_Basic_Node in_order;
    Heap_Basic_Node free_nodes;
    u64 used_space;
    u64 total_space;
};

#endif

// BOTTOM

