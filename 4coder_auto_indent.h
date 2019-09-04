/*
4coder_auto_indent.h - Auto-indentation types.
*/

// TOP

#if !defined(FCODER_AUTO_INDENT_H)
#define FCODER_AUTO_INDENT_H

struct Indent_Options{
    b32 empty_blank_lines;
    b32 use_tabs;
    i32 tab_width;
};

struct Indent_Parse_State{
    i64 current_indent;
    i64 previous_line_indent;
    i64 paren_nesting;
    i64 paren_anchor_indent[16];
    i64 comment_shift;
    i64 previous_comment_indent;
};

struct Indent_Anchor_Position{
    Token *token;
    i32 indentation;
};

#endif

// BOTTOM

