/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * Panel layout structures
 *
 */

// TOP

#if !defined(FRED_LAYOUT_H)
#define FRED_LAYOUT_H

struct Panel_Divider{
    Panel_Divider *next;
    i32 parent;
    i32 which_child;
    i32 child1, child2;
    b32 v_divider;
    f32 pos;
};

struct Screen_Region{
    i32_Rect full;
    i32_Rect inner;
    i32 l_margin, r_margin;
    i32 t_margin, b_margin;
};

struct Panel{
    Panel *next;
    Panel *prev;
    
    struct View *view;
    i32 parent;
    i32 which_child;
    
    union{
        struct{
            i32_Rect full;
            i32_Rect inner;
            i32_Rect prev_inner;
            i32 l_margin, r_margin;
            i32 t_margin, b_margin;
        };
        Screen_Region screen_region;
    };
};

struct Editing_Layout{
    Panel *panels;
    Panel free_sentinel;
    Panel used_sentinel;
    Panel_Divider *dividers;
    Panel_Divider *free_divider;
    i32 panel_count, panel_max_count;
    i32 root;
    i32 active_panel;
    i32 full_width, full_height;
    b32 panel_state_dirty;
};

struct Divider_And_ID{
    Panel_Divider* divider;
    i32 id;
};

struct Panel_And_ID{
    Panel* panel;
    i32 id;
};

#endif

// BOTTOM

