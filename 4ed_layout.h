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

typedef i32 Panel_Split_Kind;
enum{
    PanelSplitKind_Ratio_TL = 0,
    PanelSplitKind_Ratio_BR = 1,
    PanelSplitKind_FixedPixels_TL = 2,
    PanelSplitKind_FixedPixels_BR = 3,
};

struct Panel_Split{
    Panel_Split_Kind kind;
    union{
        f32 v_f32;
        i32 v_i32;
    };
};

typedef i32 Panel_Kind;
enum{
    PanelKind_Intermediate = 0,
    PanelKind_Final = 1,
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
            i32_Rect rect_full;
            i32_Rect rect_inner;
        } screen_region;
        struct{
            i32_Rect rect_full;
            i32_Rect rect_inner;
        };
    };
};

struct Layout{
    Node free_panels;
    Node open_panels;
    Node intermediate_panels;
    
    Panel *root;
    Panel *active_panel;
    
    i32 margin;
    i32 open_panel_count;
    i32 open_panel_max_count;
    Vec2_i32 full_dim;
    b32 panel_state_dirty;
};

#endif

// BOTTOM

