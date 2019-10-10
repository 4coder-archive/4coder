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
    View *next;
    View *prev;
    struct Panel *panel;
    b32 in_use;
    
    Editing_File *file;
    Lifetime_Object *lifetime_object;
    
    File_Edit_Positions edit_pos_;
    i64 mark;
    f32 preferred_x;
    
    b8 new_scroll_target;
    
    b8 ui_mode;
    Command_Map_ID ui_map_id;
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

#endif

// BOTTOM

