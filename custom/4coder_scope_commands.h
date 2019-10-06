/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

#if !defined(FCODER_SCOPE_COMMANDS_H)
#define FCODER_SCOPE_COMMANDS_H

typedef i32 Nest_Delimiter_Kind;
enum{
    NestDelimiterKind_None = 0,
    NestDelimiterKind_Open = 1,
    NestDelimiterKind_Close = 2,
};

typedef u32 Find_Scope_Flag;
enum{
    FindScope_Parent = 1,
    FindScope_NextSibling = 2,
    FindScope_EndOfToken = 4,
    FindScope_Scope = 8,
    FindScope_Paren = 16,
};

#endif

// BOTTOM

