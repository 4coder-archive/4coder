/*
 * 4coder_profile_static_disable.cpp - Statically removes all profile posting code
 * that follows until the next 4coder_profile_static_enable.cpp
 */

// TOP

#if defined(ProfileBegin)
#undef ProfileBegin
#undef ProfileEnd
#undef ProfileBlock
#undef ProfileScope
#endif

#define ProfileBegin(T,N)
#define ProfileEnd(T,I)
#define ProfileBlock(T,N)
#define ProfileScope(T,N)

// BOTTOM

