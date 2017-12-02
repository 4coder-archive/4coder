/*
4coder_search.h - Types that are used in the search accross all buffers procedures.

TYPE: 'types-header'
*/

// TOP

#if !defined(FCODER_SEARCH_H)
#define FCODER_SEARCH_H

enum Search_Range_Type{
    SearchRange_FrontToBack,
    SearchRange_BackToFront,
    SearchRange_Wave,
};

typedef uint32_t Search_Range_Flag;
enum{
    SearchFlag_MatchWholeWord  = 0x0000,
    SearchFlag_MatchWordPrefix = 0x0001,
    SearchFlag_MatchSubstring  = 0x0002,
    SearchFlag_MatchMask       = 0x00FF,
    SearchFlag_CaseInsensitive = 0x0100,
};

struct Search_Range{
    int32_t type;
    uint32_t flags;
    int32_t buffer;
    int32_t start;
    int32_t size;
    int32_t mid_start;
    int32_t mid_size;
};

struct Search_Set{
    Search_Range *ranges;
    int32_t count;
    int32_t max;
};

struct Search_Key{
    char *base;
    int32_t base_size;
    String words[16];
    int32_t count;
    int32_t min_size;
};

struct Search_Iter{
    Search_Key key;
    int32_t pos;
    int32_t back_pos;
    int32_t i;
    int32_t range_initialized;
};

struct Search_Match{
    Buffer_Summary buffer;
    int32_t start;
    int32_t end;
    int32_t match_word_index;
    int32_t found_match;
};

#endif

// BOTOTM

