/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.07.2017
 *
 * File editing view for 4coder.
 *
 */

// TOP

#if !defined(FRED_VIEW_H)
#define FRED_VIEW_H

struct View{
    struct View *next;
    struct View *prev;
    struct Panel *panel;
    b32 in_use;
    
    Editing_File *file;
    Lifetime_Object *lifetime_object;
    
    File_Edit_Positions edit_pos_;
    i64 mark;
    f32 preferred_x;
    
    // TODO(allen): ELIMINATE THESE
    i32 temp_view_top_left_pos;
    i32 temp_view_top_left_target_pos;
    
    b8 new_scroll_target;
    
    b8 ui_mode;
    i32 ui_map_id;
    Basic_Scroll ui_scroll;
    UI_Quit_Function_Type *ui_quit;
    
    b8 hide_scrollbar;
    b8 hide_file_bar;
    b8 show_whitespace;
    
    // misc
    
    Query_Set query_set;
};

struct Live_Views{
    View *views;
    View free_sentinel;
    i32 count;
    i32 max;
};

enum{
    GROW_FAILED,
    GROW_NOT_NEEDED,
    GROW_SUCCESS,
};

struct Wrap_Indent_Pair{
    i32 wrap_position;
    f32 line_shift;
};

struct Potential_Wrap_Indent_Pair{
    i32 wrap_position;
    f32 line_shift;
    
    f32 wrap_x;
    i32 wrappable_score;
    
    b32 adjust_top_to_this;
};

struct Shift_Information{
    i32 start;
    i32 end;
    i32 amount;
};

struct File_Bar{
    Vec2 pos;
    Vec2 text_shift;
    i32_Rect rect;
    Face_ID font_id;
};

#if 0
struct Style_Color_Edit{
    Style_Tag target;
    Style_Tag fore;
    Style_Tag back;
    String_Const_u8 text;
};
#endif

struct Single_Line_Input_Step{
    b8 hit_newline;
    b8 hit_ctrl_newline;
    b8 hit_a_character;
    b8 hit_backspace;
    b8 hit_esc;
    b8 made_a_change;
    b8 did_command;
    b8 no_file_match;
};

enum Single_Line_Input_Type{
    SINGLE_LINE_STRING,
    SINGLE_LINE_FILE
};

struct Single_Line_Mode{
    Single_Line_Input_Type type;
    String_Const_u8 *string;
    Hot_Directory *hot_directory;
    b32 fast_folder_select;
};

struct View_Step_Result{
    b32 animating;
    b32 consume_keys;
    b32 consume_esc;
};

#endif

// BOTTOM

