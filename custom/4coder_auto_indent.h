/*
4coder_auto_indent.h - Auto-indentation types.
*/

// TOP

#if !defined(FCODER_AUTO_INDENT_H)
#define FCODER_AUTO_INDENT_H

typedef u32 Indent_Flag;
enum{
    Indent_ClearLine = 0x1,
    Indent_UseTab    = 0x2,
    Indent_FullTokens = 0x4,
};

struct Nest{
    Nest *next;
    Token_Base_Kind kind;
    i64 indent;
};

struct Nest_Alloc{
    Nest *free_nest;
};

struct Indent_Line_Cache{
    i64 where_token_starts;
    i64 line_number_for_cached_indent;
    i64 start_pos;
    i64 one_past_last_pos;
    Indent_Info indent_info;
};

#endif

// BOTTOM

