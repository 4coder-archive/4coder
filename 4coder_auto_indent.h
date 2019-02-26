/*
4coder_auto_indent.h - Auto-indentation types.
*/

// TOP

#if !defined(FCODER_AUTO_INDENT_H)
#define FCODER_AUTO_INDENT_H

struct Hard_Start_Result{
    i32 char_pos;
    i32 indent_pos;
    i32 all_whitespace;
    i32 all_space;
};

struct Indent_Options{
    b32 empty_blank_lines;
    b32 use_tabs;
    i32 tab_width;
};

struct Indent_Parse_State{
    i32 current_indent;
    i32 previous_line_indent;
    i32 paren_nesting;
    i32 paren_anchor_indent[16];
    i32 comment_shift;
    i32 previous_comment_indent;
};

struct Indent_Anchor_Position{
    Cpp_Token *token;
    i32 indentation;
};

#endif

// BOTTOM

