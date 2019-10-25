/*
 * 4coder_profile_static_enable.cpp - Macro interface for self profiler.
 */

// TOP

#if defined(ProfileBegin)
#undef ProfileBegin
#undef ProfileEnd
#undef ProfileBlock
#undef ProfileScope
#undef ProfileBlockNamed
#undef ProfileScopeNamed
#endif

#define ProfileBegin(T,N) \
thread_profile_record_push((T), system_now_time(), \
string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileEnd(T,I) thread_profile_record_pop((T), system_now_time(), (I))

#define ProfileBlock(T,N) \
Profile_Block glue(profile_block_, __LINE__) \
((T), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileScope(T,N) \
Profile_Scope_Block glue(profile_block_, __LINE__) \
((T), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileBlockNamed(T,N,M) \
Profile_Block M \
((T), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileScopeNamed(T,N,M) \
Profile_Scope_Block M \
((T), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

// BOTTOM

