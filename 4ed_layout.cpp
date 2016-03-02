/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Panel layout and general view functions for 4coder
 *
 */

// TOP

struct Panel_Divider{
    Panel_Divider *next;
    i32 parent;
    i32 which_child;
    i32 child1, child2;
    b32 v_divider;
    i32 pos;
};

struct Screen_Region{
    i32_Rect full;
    i32_Rect inner;
    i32_Rect prev_inner;
    i32 l_margin, r_margin;
    i32 t_margin, b_margin;
};

struct Panel{
    Panel *next;
    Panel *prev;

    struct View *view;
    i32 parent;
    i32 which_child;
    
    int ALLOCED;
    
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
};

struct Divider_And_ID{
    Panel_Divider* divider;
    i32 id;
};

struct Panel_And_ID{
    Panel* panel;
    i32 id;
};

internal void
panel_init(Panel *panel){
    panel->view = 0;
    panel->parent = -1;
    panel->which_child = 0;
    panel->screen_region.full = {};
    panel->screen_region.inner = {};
    panel->screen_region.prev_inner = {};
    panel->l_margin = 3;
    panel->r_margin = 3;
    panel->t_margin = 3;
    panel->b_margin = 3;
}

internal Divider_And_ID
layout_alloc_divider(Editing_Layout *layout){
    Divider_And_ID result;
    
    Assert(layout->free_divider);
    result.divider = layout->free_divider;
    layout->free_divider = result.divider->next;
    
    *result.divider = {};
    result.divider->parent = -1;
    result.divider->child1 = -1;
    result.divider->child2 = -1;
    result.id = (i32)(result.divider - layout->dividers);
    if (layout->panel_count == 1){
        layout->root = result.id;
    }
    
    return(result);
}

internal Divider_And_ID
layout_get_divider(Editing_Layout *layout, i32 id){
    Divider_And_ID result;
    
    Assert(id >= 0 && id < layout->panel_max_count-1);
    result.id = id;
    result.divider = layout->dividers + id;
    
    return(result);
}

internal void
layout_free_divider(Editing_Layout *layout, Panel_Divider *divider){
    divider->next = layout->free_divider;
    layout->free_divider = divider;
}

internal Panel_And_ID
layout_alloc_panel(Editing_Layout *layout){
    Panel_And_ID result = {};
    
    Assert(layout->panel_count < layout->panel_max_count);
    ++layout->panel_count;
    
    result.panel = layout->free_sentinel.next;
    dll_remove(result.panel);
    dll_insert(&layout->used_sentinel, result.panel);
    
    panel_init(result.panel);
    
    result.id = (i32)(result.panel - layout->panels);
    
    result.panel->ALLOCED = 1;
    
    return(result);
}

internal void
layout_free_panel(Editing_Layout *layout, Panel *panel){
    dll_remove(panel);
    dll_insert(&layout->free_sentinel, panel);
    --layout->panel_count;
    
    panel->ALLOCED = 0;
}

internal Divider_And_ID
layout_calc_divider_id(Editing_Layout *layout, Panel_Divider *divider){
    Divider_And_ID result;
    result.divider = divider;
    result.id = (i32)(divider - layout->dividers);
    return result;
}

struct Split_Result{
    Panel_Divider *divider;
    Panel *panel;
};

internal Split_Result
layout_split_panel(Editing_Layout *layout, Panel *panel, b32 vertical){
    Split_Result result = {};
    Divider_And_ID div = {}, parent_div = {};
    Panel_And_ID new_panel = {};
    
    div = layout_alloc_divider(layout);
    if (panel->parent != -1){
        parent_div = layout_get_divider(layout, panel->parent);
        if (panel->which_child == -1){
            parent_div.divider->child1 = div.id;
        }
        else{
            parent_div.divider->child2 = div.id;
        }
    }
    
    div.divider->parent = panel->parent;
    div.divider->which_child = panel->which_child;
    if (vertical){
        div.divider->v_divider = 1;
        div.divider->pos = (panel->full.x0 + panel->full.x1) / 2;
    }
    else{
        div.divider->v_divider = 0;
        div.divider->pos = (panel->full.y0 + panel->full.y1) / 2;
    }

    new_panel = layout_alloc_panel(layout);
    panel->parent = div.id;
    panel->which_child = -1;
    new_panel.panel->parent = div.id;
    new_panel.panel->which_child = 1;

    result.divider = div.divider;
    result.panel = new_panel.panel;
    
    return result;
}

internal void
panel_fix_internal_area(Panel *panel){
    panel->inner.x0 = panel->full.x0 + panel->l_margin;
    panel->inner.x1 = panel->full.x1 - panel->r_margin;
    panel->inner.y0 = panel->full.y0 + panel->t_margin;
    panel->inner.y1 = panel->full.y1 - panel->b_margin;
}

internal void
layout_fix_all_panels(Editing_Layout *layout){
    Panel *panel;
    Panel_Divider *dividers = layout->dividers;
    i32 panel_count = layout->panel_count;
    i32_Rect r;
    i32 pos, which_child, action;
    Divider_And_ID div;
    
    if (panel_count > 1){
        for (panel = layout->used_sentinel.next;
            panel != &layout->used_sentinel;
            panel = panel->next){
            
            r.x0 = 0;
            r.x1 = r.x0 + layout->full_width;
            r.y0 = 0;
            r.y1 = r.y0 + layout->full_height;
            
            which_child = panel->which_child;
            
            div.id = panel->parent;
            
            for (;;){
                Assert(div.id != -1);
                div.divider = dividers + div.id;
                pos = div.divider->pos;
                
                action = (div.divider->v_divider << 1) | (which_child > 0);
                switch (action){
                case 0: // v_divider : 0, which_child : -1
                    if (pos < r.y1) r.y1 = pos;
                    break;
                case 1: // v_divider : 0, which_child : 1
                    if (pos > r.y0) r.y0 = pos;
                    break;
                case 2: // v_divider : 1, which_child : -1
                    if (pos < r.x1) r.x1 = pos;
                    break;
                case 3: // v_divider : 1, which_child : 1
                    if (pos > r.x0) r.x0 = pos;
                    break;
                }
                
                if (div.id != layout->root){
                    div.id = div.divider->parent;
                    which_child = div.divider->which_child;
                }
                else{
                    break;
                }
            }
            
            panel->full = r;
            panel_fix_internal_area(panel);
        }
    }

    else{
        panel = layout->used_sentinel.next;
        panel->full.x0 = 0;
        panel->full.y0 = 0;
        panel->full.x1 = layout->full_width;
        panel->full.y1 = layout->full_height;
        panel_fix_internal_area(panel);
    }
}

internal void
layout_refit(Editing_Layout *layout, i32 prev_width, i32 prev_height){
    
    Panel_Divider *dividers = layout->dividers;
    i32 max = layout->panel_max_count - 1;
    
    f32 h_ratio, v_ratio;

    Panel_Divider *divider = dividers;
    i32 i;

    if (layout->panel_count > 1){
        Assert(prev_width != 0 && prev_height != 0);

        h_ratio = ((f32)layout->full_width) / prev_width;
        v_ratio = ((f32)layout->full_height) / prev_height;

        for (i = 0; i < max; ++i, ++divider){
            if (divider->v_divider){
                divider->pos = ROUND32((divider->pos) * h_ratio);
            }
            else{
                divider->pos = ROUND32((divider->pos) * v_ratio);
            }
        }
    }

    layout_fix_all_panels(layout);
}

enum View_Message{
    VMSG_STEP,
    VMSG_DRAW,
    VMSG_RESIZE,
    VMSG_STYLE_CHANGE,
    VMSG_FREE
};

struct View;
#define Do_View_Sig(name)                                               \
    i32 (name)(System_Functions *system, Exchange *exchange,            \
               View *view, i32_Rect rect, View *active,                 \
               View_Message message, Render_Target *target,             \
               Input_Summary *user_input, Input_Summary *active_input)

typedef Do_View_Sig(Do_View_Function);

struct View{
    View *next, *prev;
    
    Panel *panel;
    Command_Map *map;
    Do_View_Function *do_view;
};

struct Live_Views{
    void *views;
    View free_sentinel;
    i32 count, max;
    i32 stride;
};

struct View_And_ID{
    View *view;
    i32 id;
};

internal View*
live_set_get_view(Live_Views *live_set, i32 id){
    void *result = ((char*)live_set->views + id);
    return (View*)result;
}

internal View_And_ID
live_set_alloc_view(Live_Views *live_set, Mem_Options *mem){
    View_And_ID result = {};
    
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    result.view = live_set->free_sentinel.next;
    result.id = (i32)((char*)result.view - (char*)live_set->views);
    
    dll_remove(result.view);
    memset(result.view, 0, live_set->stride);
    
    return(result);
}

inline void
live_set_free_view(System_Functions *system, Exchange *exchange, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    view->do_view(system, exchange, view, {}, 0, VMSG_FREE, 0, {}, 0);
    dll_insert(&live_set->free_sentinel, view);
}

inline void
view_set_first(View *new_view, Panel *panel){
    new_view->panel = panel;
    panel->view = new_view;
}

inline f32
view_base_compute_width(View *view){
    Panel *panel = view->panel;
    return (f32)(panel->inner.x1 - panel->inner.x0);
}

inline f32
view_base_compute_height(View *view){
    Panel *panel = view->panel;
    return (f32)(panel->inner.y1 - panel->inner.y0);
}

#define view_compute_width(view) (view_base_compute_width(&(view)->view_base))
#define view_compute_height(view) (view_base_compute_height(&(view)->view_base))

struct Interactive_Style{
    u32 bar_color;
    u32 bar_active_color;
    u32 base_color;
    u32 pop1_color;
    u32 pop2_color;
};

struct Interactive_Bar{
    Interactive_Style style;
    f32 pos_x, pos_y;
    f32 text_shift_x, text_shift_y;
    i32_Rect rect;
    i16 font_id;
};

internal void
intbar_draw_string(Render_Target *target,
                   Interactive_Bar *bar, u8 *str, u32 char_color){
    i16 font_id = bar->font_id;

    draw_string(target, font_id, (char*)str,
        (i32)(bar->pos_x + bar->text_shift_x),
        (i32)(bar->pos_y + bar->text_shift_y),
        char_color);
    bar->pos_x += font_string_width(target, font_id, (char*)str);
}

internal void
intbar_draw_string(Render_Target *target, Interactive_Bar *bar,
                   String str, u32 char_color){
    i16 font_id = bar->font_id;
    
    draw_string(target, font_id, str,
        (i32)(bar->pos_x + bar->text_shift_x),
        (i32)(bar->pos_y + bar->text_shift_y),
        char_color);
    bar->pos_x += font_string_width(target, font_id, str);
}

// BOTTOM

