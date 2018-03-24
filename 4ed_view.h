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

struct Debug_Vars{
    i32 mode;
    i32 inspecting_view_id;
};
global_const Debug_Vars null_debug_vars = {0};

struct View_Transient{
    struct View *next;
    struct View *prev;
    struct Panel *panel;
    b32 in_use;
    i32 map;
    
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

struct View{
    // TODO(allen): Why is this this way?
    View_Persistent persistent;
    View_Transient  transient;
};

struct Live_Views{
    View *views;
    View free_sentinel;
    i32 count, max;
};

struct Cursor_Limits{
    f32 min, max;
    f32 delta;
};

struct View_And_ID{
    View *view;
    i32 id;
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
    i32 start, end, amount;
};

struct Edit_Spec{
    u8 *str;
    Edit_Step step;
};

struct Relative_Scrolling{
    f32 scroll_x, scroll_y;
    f32 target_x, target_y;
};

struct Cursor_Fix_Descriptor{
    b32 is_batch;
    union{
        struct{
            Buffer_Edit *batch;
            i32 batch_size;
        };
        struct{
            i32 start, end;
            i32 shift_amount;
        };
    };
};

struct File_Bar{
    f32 pos_x, pos_y;
    f32 text_shift_x, text_shift_y;
    i32_Rect rect;
    Face_ID font_id;
};

struct Exhaustive_File_Loop{
    char front_name_[256];
    char full_path_[256];
    String front_name, full_path;
    
    Absolutes absolutes;
    
    File_Info *infos;
    i32 count, r;
};

struct Exhaustive_File_Info{
    File_Info *info;
    String message;
    b8 is_folder;
    b8 name_match;
    b8 is_loaded;
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
    b32 try_to_match;
    b32 case_sensitive;
};

struct View_Step_Result{
    b32 animating;
    b32 consume_keys;
    b32 consume_esc;
};

struct Input_Process_Result{
    GUI_Scroll_Vars vars;
    i32_Rect region;
    b32 is_animating;
    b32 consumed_l;
    b32 consumed_r;
    
    b32 has_max_y_suggestion;
    i32 max_y;
};

enum{
    FileCreateFlag_ReadOnly = 1,
};

enum History_Mode{
    hist_normal,
    hist_backward,
    hist_forward
};

enum Try_Kill_Result{
    TryKill_CannotKill,
    TryKill_NeedDialogue,
    TryKill_Success
};

#endif

// BOTTOM

