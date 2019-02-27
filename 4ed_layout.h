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

struct Panel_Split{
    Panel_Split_Kind kind;
    union{
        f32 v_f32;
        i32 v_i32;
    };
};

typedef i32 Panel_Kind;
enum{
    PanelKind_Unused = 0,
    PanelKind_Intermediate = 1,
    PanelKind_Final = 2,
};

struct Panel{
    Node node;
    
    Panel *parent;
    Panel_Kind kind;
    union{
        struct View *view;
        struct{
            struct Panel *tl_panel;
            struct Panel *br_panel;
            b32 vertical_split;
            Panel_Split split;
        };
    };
    
    union{
        struct{
            Rect_i32 rect_full;
            Rect_i32 rect_inner;
        } screen_region;
        struct{
            Rect_i32 rect_full;
            Rect_i32 rect_inner;
        };
    };
};

struct Layout{
    Node free_panels;
    Node open_panels;
    Node intermediate_panels;
    
    Panel *root;
    Panel *active_panel;
    Panel *panel_first;
    Panel *panel_one_past_last;
    
    i32 margin;
    i32 open_panel_count;
    i32 open_panel_max_count;
    Vec2_i32 full_dim;
    b32 panel_state_dirty;
};

#endif

// BOTTOM

