/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * Parse context structures
 *
 */

// TOP

#if !defined(FRED_PARSE_CONTEXT_H)
#define FRED_PARSE_CONTEXT_H

struct Stored_Parse_Context{
    umem memsize;
    u64 *kw_keywords;
    u64 *pp_keywords;
    u32 kw_max;
    u32 pp_max;
};

struct Stored_Parse_Context_Slot{
    union{
        Stored_Parse_Context_Slot *next;
        Stored_Parse_Context *context;
    };
    b32 freed;
};

struct Parse_Context_Memory{
    Stored_Parse_Context_Slot *parse_context_array;
    u32 parse_context_counter;
    u32 parse_context_max;
    
    Stored_Parse_Context_Slot free_sentinel;
};

struct Parse_Context{
    b32 valid;
    Cpp_Keyword_Table kw_table;
    Cpp_Keyword_Table pp_table;
    umem memory_size;
};

#endif

// BOTTOM

