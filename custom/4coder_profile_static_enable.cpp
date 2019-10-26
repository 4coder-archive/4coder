/*
 * 4coder_profile_static_enable.cpp - Macro interface for self profiler.
 */

// TOP

#if defined(ProfileBlock)
#undef ProfileBlock
#undef ProfileScope
#undef ProfileBlockNamed
#undef ProfileScopeNamed

#undef ProfileTLBlock
#undef ProfileTLScope
#undef ProfileTLBlockNamed
#undef ProfileTLScopeNamed

#undef ProfileCloseNow
#endif

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



#define ProfileTLBlock(T,L,N) \
Profile_Block glue(profile_block_, __LINE__) \
((T), (L), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileTLScope(T,L,N) \
Profile_Scope_Block glue(profile_block_, __LINE__) \
((T), (L), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileTLBlockNamed(T,L,N,M) \
Profile_Block M \
((T), (L), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))

#define ProfileTLScopeNamed(T,L,N,M) \
Profile_Scope_Block M \
((T), (L), string_u8_litexpr(N), string_u8_litexpr(file_name_line_number))



#define ProfileCloseNow(B) ((B).close_now())

// BOTTOM

