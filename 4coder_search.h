/*
4coder_search.h - Types that are used in the search accross all buffers procedures.
*/

// TOP

#if !defined(FCODER_SEARCH_H)
#define FCODER_SEARCH_H

typedef u32 List_All_Locations_Flag;
enum{
    ListAllLocationsFlag_CaseSensitive = 1,
    ListAllLocationsFlag_MatchSubstring = 2,
};

struct Word_Complete_State{
    b32 initialized;
    String_Const_u8 needle;
    Range_i64 range;
    List_String_Const_u8 list;
    Node_String_Const_u8 *iterator;
};

#endif

// BOTOTM

