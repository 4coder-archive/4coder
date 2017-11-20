/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.07.2017
 *
 * File editing view for 4coder.
 *
 */

// TOP

#if !defined(FRED_VIEW_CPP)
#define FRED_VIEW_CPP

struct View_Persistent{
    i32 id;
    Coroutine_Head *coroutine;
    Event_Message message_passing_slot;
};

struct File_Viewing_Data{
    Editing_File *file;
    
    Full_Cursor temp_highlight;
    i32 temp_highlight_end_pos;
    b32 show_temp_highlight;
    
    b32 show_whitespace;
    b32 file_locked;
};
global File_Viewing_Data null_file_viewing_data = {0};

enum Interactive_Action{
    IAct_Open,
    IAct_Save_As,
    IAct_New,
    IAct_OpenOrNew,
    IAct_Switch,
    IAct_Kill,
    IAct_Sure_To_Kill,
    IAct_Sure_To_Close
};

enum Interactive_Interaction{
    IInt_Sys_File_List,
    IInt_Live_File_List,
    IInt_Sure_To_Kill,
    IInt_Sure_To_Close
};

enum View_UI{
    VUI_None,
    VUI_Theme,
    VUI_Interactive,
    VUI_Debug
};

enum Debug_Mode{
    DBG_Input,
    DBG_Threads_And_Memory,
    DBG_View_Inspection
};

enum Color_View_Mode{
    CV_Mode_Library,
    CV_Mode_Font,
    CV_Mode_Global_Font,
    CV_Mode_Font_Editing,
    CV_Mode_Global_Font_Editing,
    CV_Mode_Adjusting,
};

struct Scroll_Context{
    Editing_File *file;
    GUI_id scroll;
    View_UI mode;
};
inline b32
context_eq(Scroll_Context a, Scroll_Context b){
    b32 result = false;
    if (gui_id_eq(a.scroll, b.scroll)){
        if (a.file == b.file){
            if (a.mode == b.mode){
                result = true;
            }
        }
    }
    return(result);
}

struct Debug_Vars{
    i32 mode;
    i32 inspecting_view_id;
};
global_const Debug_Vars null_debug_vars = {0};

struct View{
    View_Persistent persistent;
    
    View *next, *prev;
    Panel *panel;
    b32 in_use;
    i32 map;
    //Command_Map *map;
    
    File_Viewing_Data file_data;
    
    i32_Rect file_region_prev;
    i32_Rect file_region;
    
    i32_Rect scroll_region;
    File_Edit_Positions *edit_pos;
    
    View_UI showing_ui;
    GUI_Target gui_target;
    void *gui_mem;
    GUI_Scroll_Vars gui_scroll;
    i32 gui_max_y;
    i32 list_i;
    
    b32 hide_scrollbar;
    b32 hide_file_bar;
    
    // interactive stuff
    Interactive_Interaction interaction;
    Interactive_Action action;
    
    char dest_[256];
    String dest;
    
    b32 changed_context_in_step;
    
    // theme stuff
    View *hot_file_view;
    u32 *palette;
    Color_View_Mode color_mode;
    Face_ID font_edit_id;
    Super_Color color;
    b32 p4c_only;
    Style_Library inspecting_styles;
    b8 import_export_check[64];
    i32 import_file_id;
    i32 current_color_editing;
    i32 color_cursor;
    
    // misc
    
    // TODO(allen): Can we burn line_height to the ground now?
    // It's what I've always wanted!!!! :D
    i32 line_height;
    
    // TODO(allen): Do I still use mode?
    Query_Set query_set;
    f32 widget_height;
    
    b32 reinit_scrolling;
    
    Debug_Vars debug_vars;
};

struct Live_Views{
    View *views;
    View free_sentinel;
    i32 count, max;
};

#endif

// BOTTOM

