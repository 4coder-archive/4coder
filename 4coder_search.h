/*
4coder_search.h - Types that are used in the search accross all buffers procedures.
*/

// TOP

#if !defined(FCODER_SEARCH_H)
#define FCODER_SEARCH_H

/* TODO(allen): 

Time to rewrite _ALL_ of this s***f.
[x] 1. String matching optimization study
[x] 2. Redesign and reimplement find_all_in_range
[x] 3. Filtering and reversing string lists
[ ] 4. Clean prefix match iterator
[ ] 5. Reimplement commands in 4coder_search.cpp
[ ] 6. Ditch all the old s***f we don't want anymore
[ ] 7. Reorganize, rename, etc.

*/

////////////////////////////////



////////////////////////////////

// TODO(allen): deprecate all this
typedef i32 Seek_Potential_Match_Direction;
enum{
    SeekPotentialMatch_Forward = 0,
    SeekPotentialMatch_Backward = 1,
};

enum{
    FindResult_None,
    FindResult_FoundMatch,
    FindResult_PastEnd,
};

typedef i32 Search_Range_Type;
enum{
    SearchRange_FrontToBack = 0,
    SearchRange_BackToFront = 1,
    SearchRange_Wave = 2,
};

typedef u32 Search_Range_Flag;
enum{
    SearchFlag_MatchWholeWord  = 0x0000,
    SearchFlag_MatchWordPrefix = 0x0001,
    SearchFlag_MatchSubstring  = 0x0002,
    SearchFlag_MatchMask       = 0x00FF,
    SearchFlag_CaseInsensitive = 0x0100,
};

struct Search_Range{
    Search_Range_Type type;
    Search_Range_Flag flags;
    Buffer_ID buffer;
    i32 start;
    i32 size;
    i32 mid_start;
    i32 mid_size;
};

struct Search_Set{
    Search_Range *ranges;
    i32 count;
    i32 max;
};

struct Search_Key{
    u8 *base;
    umem base_size;
    umem min_size;
    String_Const_u8 words[16];
    i32 count;
};

struct Search_Iter{
    Search_Key key;
    i32 pos;
    i32 back_pos;
    i32 i;
    i32 range_initialized;
};

struct Search_Match{
    Buffer_ID buffer;
    i32 start;
    i32 end;
    i32 match_word_index;
    i32 found_match;
};

struct Word_Complete_State{
    Search_Set set;
    Search_Iter iter;
    Table hits;
    String_Space str;
    i32 word_start;
    i32 word_end;
    i32 initialized;
};

#endif

// BOTOTM

