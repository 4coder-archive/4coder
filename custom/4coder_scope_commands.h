/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

#if !defined(FCODER_SCOPE_COMMANDS_H)
#define FCODER_SCOPE_COMMANDS_H

typedef i32 Nest_Delimiter_Kind;
enum{
    NestDelim_None = 0,
    NestDelim_Open = 1,
    NestDelim_Close = 2,
};

typedef u32 Find_Nest_Flag;
enum{
    FindNest_Scope = 1,
    FindNest_Paren = 2,
    FindNest_EndOfToken = 4,
    FindNest_Balanced = 8,
};

#endif

// BOTTOM

