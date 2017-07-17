// 4ed_standard_preamble.h
#if !defined(FTECH_INTEGERS)
#define FTECH_INTEGERS
#include <stdint.h>
typedef int8_t i8_4ed;
typedef int16_t i16_4ed;
typedef int32_t i32_4ed;
typedef int64_t i64_4ed;

typedef uint8_t u8_4ed;
typedef uint16_t u16_4ed;
typedef uint32_t u32_4ed;
typedef uint64_t u64_4ed;

#if defined(FTECH_32_BIT)
typedef u32_4ed umem_4ed;
#else
typedef u64_4ed umem_4ed;
#endif

typedef float f32_4ed;
typedef double f64_4ed;

typedef int8_t b8_4ed;
typedef int32_t b32_4ed;
#endif

#if !defined(Assert)
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif
// standard preamble end 
