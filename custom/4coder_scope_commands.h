/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

#if !defined(FCODER_SCOPE_COMMANDS_H)
#define FCODER_SCOPE_COMMANDS_H

typedef u32 Find_Scope_Flag;
enum{
    FindScope_Parent = 1,
    FindScope_NextSibling = 2,
    FindScope_EndOfToken = 4,
    FindScope_Scope = 8,
    FindScope_Paren = 16,
};

typedef i32 Find_Scope_Token_Type;
enum{
    FindScopeTokenType_None = 0,
    FindScopeTokenType_Open = 1,
    FindScopeTokenType_Close = 2,
};

#endif

// BOTTOM

