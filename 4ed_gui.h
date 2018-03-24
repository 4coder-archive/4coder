/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.02.2016
 *
 * GUI system for 4coder
 *
 */

// TOP

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

#define GUI_BEGIN_SCROLLABLE_SIG(n) int32_t n(GUI *gui, GUI_id scroll_id, GUI_Scroll_Vars vars, float delta, int32_t show_scrollbar)
#define GUI_END_SCROLLABLE_SIG(n) void n(GUI *gui)

#define GUI_FILE_SIG(n) void n(GUI *gui, int32_t buffer_id)

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

struct Query_Slot{
    Query_Slot *next;
    Query_Bar *query_bar;
};

struct Query_Set{
    Query_Slot slots[8];
    Query_Slot *free_slot;
    Query_Slot *used_slot;
};

struct Super_Color{
    Vec4 hsla;
    Vec4 rgba;
    u32 *out;
};

struct GUI_Target{
    Partition push;
    
    GUI_id active;
    GUI_id mouse_hot;
    GUI_id auto_hot;
    GUI_id hover;
    
    // TODO(allen): Can we remove original yet?
    GUI_Scroll_Vars scroll_original;
    i32_Rect region_original;
    
    //GUI_Scroll_Vars scroll_updated;
    i32_Rect region_updated;
    
    // TODO(allen): Would rather have a way of tracking this
    // for more than one list.  Perhaps just throw in a hash table?
    // Or maybe this only needs to be tracked for the active list.
    i32 list_max;
    b32 has_list_index_position;
    i32_Rect list_index_position;
    i32 list_view_min;
    i32 list_view_max;
    
    GUI_id scroll_id; 
    // TODO(allen): is currently ignored in the wheel code, reevaluate?
    i32 delta;
    b32 has_keys;
    b32 animating;
    b32 did_file;
};

struct GUI_Item_Update{
    i32 partition_point;
    
    b32 has_adjustment;
    i32 adjustment_value;
    
    b32 has_index_position;
    i32_Rect index_position;
};

struct GUI_Header{
    i32 type;
    i32 size;
};

struct GUI_Interactive{
    GUI_Header h;
    GUI_id id;
};

struct GUI_Edit{
    GUI_Header h;
    GUI_id id;
    void *out;
};

enum GUI_Command_Type{
    guicom_null,
    guicom_begin_serial,
    guicom_end_serial,
    guicom_top_bar,
    guicom_file,
    guicom_text_field,
    guicom_color_button,
    guicom_font_button,
    guicom_text_with_cursor,
    guicom_begin_list,
    guicom_end_list,
    guicom_file_option,
    guicom_fixed_option,
    guicom_button,
    guicom_fixed_option_checkbox,
    guicom_style_preview,
    guicom_scrollable,
    guicom_scrollable_bar,
    guicom_scrollable_top,
    guicom_scrollable_slider,
    guicom_scrollable_bottom,
    guicom_scrollable_invisible,
    guicom_begin_scrollable_section,
    guicom_end_scrollable_section,
};

struct GUI_Section{
    i32 max_v, v, top_v;
};

struct GUI_List_Vars{
    b32 in_list;
    i32 index;
    i32 auto_hot;
    i32 auto_activate;
};

struct GUI_Session{
    i32_Rect full_rect;
    i32_Rect rect;
    
    i32 suggested_max_y;
    i32 clip_y;
    
    i32 line_height;
    b32 is_scrollable;
    i32 scrollable_items_bottom;
    
    i32_Rect scroll_region;
    i32_Rect scroll_rect;
    f32 scroll_top, scroll_bottom;
    
    GUI_List_Vars list;
    
    GUI_Section sections[64];
    i32 t;
};

struct GUI_Interpret_Result{
    b32 has_info;
    b32 auto_hot;
    b32 auto_activate;
    i32 screen_orientation;
    
    b32 has_region;
    i32_Rect region;
};

struct GUI_View_Jump{
    i32 view_min;
    i32 view_max;
};

#endif

// BOTTOM

