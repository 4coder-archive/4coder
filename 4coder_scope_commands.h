/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

#if !defined(FCODER_SCOPE_COMMANDS_H)
#define FCODER_SCOPE_COMMANDS_H

enum{
    FindScope_Parent = 0x1,
    FindScope_NextSibling = 0x1,
    FindScope_EndOfToken = 0x2,
};

struct Statement_Parser{
    Stream_Tokens stream;
    int32_t token_index;
    Buffer_Summary *buffer;
};

#endif

// BOTTOM

