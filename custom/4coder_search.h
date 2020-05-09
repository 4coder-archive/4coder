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

struct Word_Complete_Iterator{
    Application_Links *app;
    Arena *arena;
    
    Temp_Memory arena_restore;
    Buffer_ID first_buffer;
    Buffer_ID current_buffer;
    b32 scan_all_buffers;
    String_Const_u8 needle;
    
    List_String_Const_u8 list;
    Node_String_Const_u8 *node;
    Table_Data_u64 already_used_table;
};

struct Word_Complete_Menu{
    Render_Caller_Function *prev_render_caller;
    Word_Complete_Iterator *it;
    String_Const_u8 options[8];
    i32 count;
};

#endif

// BOTOTM

