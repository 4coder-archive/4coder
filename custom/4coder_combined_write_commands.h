/*
4coder_combined_write_commands.cpp - Commands for writing text specialized for particular contexts.
*/

// TOP

struct Snippet{
    char *name;
    char *text;
    i32 cursor_offset;
    i32 mark_offset;
};

struct Snippet_Array{
    Snippet *snippets;
    i32 count;
};

// BOTTOM

