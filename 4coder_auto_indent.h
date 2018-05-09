/*
4coder_auto_indent.h - Auto-indentation types.
*/

// TOP

#if !defined(FCODER_AUTO_INDENT_H)
#define FCODER_AUTO_INDENT_H

struct Hard_Start_Result{
    int32_t char_pos;
    int32_t indent_pos;
    int32_t all_whitespace;
    int32_t all_space;
};

struct Indent_Options{
    bool32 empty_blank_lines;
    bool32 use_tabs;
    int32_t tab_width;
};

struct Indent_Parse_State{
    int32_t current_indent;
    int32_t previous_line_indent;
    int32_t paren_nesting;
    int32_t paren_anchor_indent[16];
    int32_t comment_shift;
    int32_t previous_comment_indent;
};

struct Indent_Anchor_Position{
    Cpp_Token *token;
    int32_t indentation;
};

#endif

// BOTTOM

