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

struct File_Viewing_Data{
    Editing_File *file;
    b32 show_whitespace;
    b32 file_locked;
};

struct View{
    struct View *next;
    struct View *prev;
    struct Panel *panel;
    b32 in_use;
    
    File_Viewing_Data file_data;
    Lifetime_Object *lifetime_object;
    
    i32_Rect file_region_prev;
    i32_Rect file_region;
    
    i32_Rect scroll_region;
    File_Edit_Positions edit_pos_;
    i32 mark;
    f32 preferred_x;
    
    i32 temp_view_top_left_pos;
    i32 temp_view_top_left_target_pos;
    
    b32 ui_mode;
    UI_Quit_Function_Type *ui_quit;
    UI_Control ui_control;
    GUI_Scroll_Vars ui_scroll;
    Vec2_i32 prev_target;
    i32 ui_map_id;
    
    b32 hide_scrollbar;
    b32 hide_file_bar;
    
    // misc
    
    // TODO(allen): Can we burn line_height to the ground now?
    // It's what I've always wanted!!!! :D
    i32 line_height;
    
    Query_Set query_set;
    f32 widget_height;
};

struct Live_Views{
    View *views;
    View free_sentinel;
    i32 count;
    i32 max;
};

struct Cursor_Limits{
    i32 min;
    i32 max;
    i32 delta;
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

struct Style_Color_Edit{
    Style_Tag target;
    Style_Tag fore;
    Style_Tag back;
    String text;
};

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
    String *string;
    Hot_Directory *hot_directory;
    b32 fast_folder_select;
};

struct View_Step_Result{
    b32 animating;
    b32 consume_keys;
    b32 consume_esc;
};

struct Input_Process_Result{
    GUI_Scroll_Vars scroll;
    b32 is_animating;
    b32 consumed_l;
    b32 consumed_r;
};

enum{
    FileCreateFlag_ReadOnly = 1,
};

struct Render_Marker_Brush{
    b8 color_noop;
    b8 text_color_noop;
    argb_color color;
    argb_color text_color;
};

struct Render_Marker{
    Marker_Visual_Type type;
    Render_Marker_Brush brush;
    i32 pos;
    i32 one_past_last;
    i32 priority;
};

struct Render_Range_Record{
    Render_Marker_Brush brush;
    i32 one_past_last;
    i32 priority;
};

struct Render_Marker_Array{
    Render_Marker *markers;
    i32 count;
};

#endif

// BOTTOM

