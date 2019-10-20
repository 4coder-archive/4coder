/*
 * 4coder_profile_static_enable.cpp - Macro interface for self profiler.
 */

// TOP

#if defined(ProfileBegin)
#undef ProfileBegin
#undef ProfileEnd
#undef ProfileBlock
#undef ProfileScope
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

// BOTTOM

