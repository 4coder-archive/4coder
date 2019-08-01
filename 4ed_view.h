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
    
    i32 temp_view_top_left_pos;
    i32 temp_view_top_left_target_pos;
    
    b32 ui_mode;
    i32 ui_map_id;
    GUI_Scroll_Vars ui_scroll;
    UI_Quit_Function_Type *ui_quit;
    
    Vec2_i32 prev_target;
    
    b32 hide_scrollbar;
    b32 hide_file_bar;
    b32 show_whitespace;
    
    // misc
    
    Query_Set query_set;
    
    // Render Context
    Arena layout_arena;
    
    struct{    
        Rect_i32 view_rect;
        Rect_i32 buffer_rect;
        Full_Cursor cursor;
        Range range;
        Buffer_Render_Item *items;
        int_color *item_colors;
        i32 item_count;
    } render;
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

struct Render_Marker_Node{
    Render_Marker_Node *next;
    Render_Marker render_marker;
};

struct Render_Marker_List{
    Render_Marker_Node *first;
    Render_Marker_Node *last;
    i32 count;
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

