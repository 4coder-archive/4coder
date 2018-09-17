/*
4coder_combined_write_commands.cpp - Commands for writing text specialized for particular contexts.
*/

// TOP

struct Snippet{
    char *name;
    char *text;
    int32_t cursor_offset;
    int32_t mark_offset;
};

struct Snippet_Array{
    Snippet *snippets;
    int32_t count;
};

// BOTTOM

