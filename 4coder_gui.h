
#ifndef FCODER_GUI_H
#define FCODER_GUI_H

#include <stdint.h>

struct GUI_id{
    uint64_t id[2];
};
inline GUI_id
gui_id_zero(){
    GUI_id id = {0};
    return(id);
}

typedef struct GUI GUI;

#define GUI_BEGIN_SIG(n) void n(GUI *gui)
#define GUI_END_SIG(n) void n(GUI *gui)

#define GUI_TOP_BAR_SIG(n) void n(GUI *gui)

// TODO(allen): Do we want to break this call
// down a little more?  I think maybe we do.
#define GUI_GET_SCROLL_VARS_SIG(n) void n(GUI *gui, GUI_id scroll_id, GUI_Scroll_Vars *vars, i32_Rect *region)

#define GUI_BEGIN_SCROLLABLE_SIG(n) int n(GUI *gui, GUI_id scroll_id, GUI_Scroll_Vars vars, float delta, int show_scrollbar)
#define GUI_END_SCROLLABLE_SIG(n) void n(GUI *gui)

#define GUI_FILE_SIG(n) void n(GUI *gui, int buffer_id)

typedef GUI_BEGIN_SIG(GUI_Begin_Function);
typedef GUI_END_SIG(GUI_End_Function);

typedef GUI_TOP_BAR_SIG(GUI_Top_Bar_Function);

typedef GUI_GET_SCROLL_VARS_SIG(GUI_Get_Scroll_Vars_Function);

typedef GUI_BEGIN_SCROLLABLE_SIG(GUI_Begin_Scrollable_Function);
typedef GUI_END_SCROLLABLE_SIG(GUI_End_Scrollable_Function);

typedef GUI_FILE_SIG(GUI_File_Function);

struct GUI_Functions{
    GUI_Begin_Function *begin;
    GUI_End_Function *end;
    
    GUI_Top_Bar_Function *top_bar;
    
    GUI_Get_Scroll_Vars_Function *get_scroll_vars;
    
    GUI_Begin_Scrollable_Function *begin_scrollable;
    GUI_End_Scrollable_Function *end_scrollable;
    
    GUI_File_Function *file;
};

#endif
