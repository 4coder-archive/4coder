/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Panel layout and general view functions for 4coder
 *
 */

// TOP

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

// TODO(allen): this shouldn't exist
enum View_Type{
    VIEW_TYPE_NONE,
    VIEW_TYPE_FILE,
    VIEW_TYPE_COLOR,
    VIEW_TYPE_INTERACTIVE,
    VIEW_TYPE_MENU,
    VIEW_TYPE_CONFIG
};

struct Panel;
struct View{
    union{
        View *next_free;
        View *major;
        View *minor;
    };
    Panel *panel;
    Command_Map *map;
    Do_View_Function *do_view;
    Mem_Options *mem;
    i32 type;
    i32 block_size;
    Application_Mouse_Cursor mouse_cursor_type;
    b32 is_active;
    b32 is_minor;
};

struct Live_Views{
    void *views;
    View *free_view;
    i32 count, max;
    i32 stride;
};

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
    View *view;
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
    Panel_Divider *dividers;
    Panel_Divider *free_divider;
    i32 panel_count, panel_max_count;
    i32 root;
    i32 active_panel;
    i32 full_width, full_height;
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

internal void
panel_init(Panel *panel){
    *panel = {};
    panel->parent = -1;
    panel->l_margin = 3;
    panel->r_margin = 3;
    panel->t_margin = 3;
    panel->b_margin = 3;
}

internal View*
live_set_get_view(Live_Views *live_set, i32 i){
    void *result = ((char*)live_set->views + i*live_set->stride);
    return (View*)result;
}

internal View*
live_set_alloc_view(Live_Views *live_set, Mem_Options *mem){
    Assert(live_set->count < live_set->max);
    View *result = 0;
    result = live_set->free_view;
    live_set->free_view = result->next_free;
    memset(result, 0, live_set->stride);
    ++live_set->count;
    result->is_active = 1;
    result->mem = mem;
    return result;
}

inline void
live_set_free_view(System_Functions *system, Exchange *exchange, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    view->do_view(system, exchange, view, {}, 0, VMSG_FREE, 0, {}, 0);
    view->next_free = live_set->free_view;
    live_set->free_view = view;
    --live_set->count;
    view->is_active = 0;
}

inline void
view_set_first(View *new_view, Panel *panel){
    new_view->panel = panel;
    new_view->minor = 0;
    panel->view = new_view;
}

inline void
view_replace_major(System_Functions *system, Exchange *exchange,
    View *new_view, Panel *panel, Live_Views *live_set){
    View *view = panel->view;
    if (view->is_minor && view->major){
        live_set_free_view(system, exchange, live_set, view->major);
    }
    live_set_free_view(system, exchange, live_set, view);
    new_view->panel = panel;
    new_view->minor = 0;
    panel->view = new_view;
}

inline void
view_replace_minor(System_Functions *system, Exchange *exchange,
    View *new_view, Panel *panel, Live_Views *live_set){
    View *view = panel->view;
    
    new_view->is_minor = 1;
    if (view->is_minor){
        new_view->major = view->major;
        live_set_free_view(system, exchange, live_set, view);
    }
    else{
        new_view->major = view;
        view->is_active = 0;
    }
    new_view->panel = panel;
    panel->view = new_view;
}

inline void
view_remove_minor(System_Functions *system, Exchange *exchange,
    Panel *panel, Live_Views *live_set){
    View *view = panel->view;
    View *major = view;
    if (view->is_minor){
        major = view->major;
        live_set_free_view(system, exchange, live_set, view);
    }
    Assert(major);
    panel->view = major;
    major->is_active = 1;
}

struct Divider_And_ID{
    Panel_Divider* divider;
    i32 id;
};

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
    Assert(id >= 0 && id < layout->panel_max_count-1);
    Divider_And_ID result;
    result.id = id;
    result.divider = layout->dividers + id;
    return result;
}

internal Panel*
layout_alloc_panel(Editing_Layout *layout){
    Assert(layout->panel_count < layout->panel_max_count);
    Panel *result = layout->panels + layout->panel_count;
    *result = {};
    ++layout->panel_count;
    return result;
}

internal void
layout_free_divider(Editing_Layout *layout, Panel_Divider *divider){
    divider->next = layout->free_divider;
    layout->free_divider = divider;
}

internal void
layout_free_panel(Editing_Layout *layout, Panel *panel){
    Panel *p, *panels;
    i32 panel_count, i;
    
    panels = layout->panels;
    panel_count = --layout->panel_count;

    p = panel;
    for (i = (i32)(panel - layout->panels); i < panel_count; ++i, ++p){
        *p = panels[i+1];
        p->view->panel = p;
    }
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
    Panel *new_panel = 0;
    
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
    new_panel->parent = div.id;
    new_panel->which_child = 1;

    result.divider = div.divider;
    result.panel = new_panel;
    return result;
}

internal void
panel_fix_internal_area(Panel *panel){
    i32 left, right, top, bottom;
    left = panel->l_margin;
    right = panel->r_margin;
    top = panel->t_margin;
    bottom = panel->b_margin;

    panel->inner.x0 = panel->full.x0 + left;
    panel->inner.x1 = panel->full.x1 - right;
    panel->inner.y0 = panel->full.y0 + top;
    panel->inner.y1 = panel->full.y1 - bottom;
}

internal void
layout_fix_all_panels(Editing_Layout *layout){
    Panel *panels = layout->panels;
    if (layout->panel_count > 1){
        Panel_Divider *dividers = layout->dividers;
        int panel_count = layout->panel_count;

        Panel *panel = panels;
        for (i32 i = 0; i < panel_count; ++i){
            i32 x0, x1, y0, y1;
            x0 = 0;
            x1 = x0 + layout->full_width;
            y0 = 0;
            y1 = y0 + layout->full_height;

            i32 pos;
            i32 which_child = panel->which_child;
            Divider_And_ID div;
            div.id = panel->parent;

            for (;;){
                div.divider = dividers + div.id;
                pos = div.divider->pos;
                div.divider = dividers + div.id;
                // NOTE(allen): sorry if this is hard to read through, there are
                // two binary conditionals that combine into four possible cases.
                // Why am I appologizing to you? IF YOU CANT HANDLE MY CODE GET OUT!
                i32 action = (div.divider->v_divider << 1) | (which_child > 0);
                switch (action){
                case 0: // v_divider : 0, which_child : -1
                    if (pos < y1) y1 = pos;
                    break;
                case 1: // v_divider : 0, which_child : 1
                    if (pos > y0) y0 = pos;
                    break;
                case 2: // v_divider : 1, which_child : -1
                    if (pos < x1) x1 = pos;
                    break;
                case 3: // v_divider : 1, which_child : 1
                    if (pos > x0) x0 = pos;
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

            panel->full.x0 = x0;
            panel->full.y0 = y0;
            panel->full.x1 = x1;
            panel->full.y1 = y1;
            panel_fix_internal_area(panel);
            ++panel;
        }
    }

    else{
        panels[0].full.x0 = 0;
        panels[0].full.y0 = 0;
        panels[0].full.x1 = layout->full_width;
        panels[0].full.y1 = layout->full_height;
        panel_fix_internal_area(panels);
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

inline real32
view_base_compute_width(View *view){
    Panel *panel = view->panel;
    return (real32)(panel->inner.x1 - panel->inner.x0);
}

inline real32
view_base_compute_height(View *view){
    Panel *panel = view->panel;
    return (real32)(panel->inner.y1 - panel->inner.y0);
}

#define view_compute_width(view) (view_base_compute_width(&(view)->view_base))
#define view_compute_height(view) (view_base_compute_height(&(view)->view_base))

// BOTTOM

