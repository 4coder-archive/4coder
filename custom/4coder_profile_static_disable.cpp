/*
 * 4coder_profile_static_disable.cpp - Statically removes all profile posting code
 * that follows until the next 4coder_profile_static_enable.cpp
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

#define ProfileBlock(T,N)
#define ProfileScope(T,N)
#define ProfileBlockNamed(T,N,M)
#define ProfileScopeNamed(T,N,M)

#define ProfileTLBlock(T,L,N)
#define ProfileTLScope(T,L,N)
#define ProfileTLBlockNamed(T,L,N,M)
#define ProfileTLScopeNamed(T,L,N,M)

#define ProfileCloseNow(O)

// BOTTOM

