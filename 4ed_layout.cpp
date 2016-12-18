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

internal void
panel_init(Panel *panel){
    panel->view = 0;
    panel->parent = -1;
    panel->which_child = 0;
    panel->screen_region.full = i32_rect_zero();
    panel->screen_region.inner = i32_rect_zero();
    panel->l_margin = 3;
    panel->r_margin = 3;
    panel->t_margin = 3;
    panel->b_margin = 3;
}

inline Panel_Divider
panel_divider_zero(){
    Panel_Divider divider={0};
    return(divider);
}

internal Divider_And_ID
layout_alloc_divider(Editing_Layout *layout){
    Divider_And_ID result;
    
    Assert(layout->free_divider);
    result.divider = layout->free_divider;
    layout->free_divider = result.divider->next;
    
    *result.divider = panel_divider_zero();
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
    
    return(result);
}

internal void
layout_free_panel(Editing_Layout *layout, Panel *panel){
    dll_remove(panel);
    dll_insert(&layout->free_sentinel, panel);
    --layout->panel_count;
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
    }
    else{
        div.divider->v_divider = 0;
    }
    div.divider->pos = 0.5f;
    
    new_panel = layout_alloc_panel(layout);
    panel->parent = div.id;
    panel->which_child = -1;
    new_panel.panel->parent = div.id;
    new_panel.panel->which_child = 1;
    
    result.divider = div.divider;
    result.panel = new_panel.panel;
    
    return(result);
}

internal void
panel_fix_internal_area(Panel *panel){
    panel->inner.x0 = panel->full.x0 + panel->l_margin;
    panel->inner.x1 = panel->full.x1 - panel->r_margin;
    panel->inner.y0 = panel->full.y0 + panel->t_margin;
    panel->inner.y1 = panel->full.y1 - panel->b_margin;
}

internal i32_Rect
layout_get_rect(Editing_Layout *layout, i32 id, i32 which_child){
    i32 divider_chain[MAX_VIEWS];
    i32 chain_count = 0;
    
    Panel_Divider *dividers = layout->dividers;
    Panel_Divider *original_div = dividers + id;
    i32 root = layout->root;
    
    Assert(0 <= id && id <= layout->panel_max_count - 1);
    
    divider_chain[chain_count++] = id;
    for (;id != root;){
        Panel_Divider *div = dividers + id;
        id = div->parent;
        divider_chain[chain_count++] = id;
    }
    
    i32_Rect r = i32R(0, 0, layout->full_width, layout->full_height);
    
    for (i32 i = chain_count-1; i > 0; --i){
        Panel_Divider *div = dividers + divider_chain[i];
        if (div->v_divider){
            if (div->child1 == divider_chain[i-1]){
                r.x1 = ROUND32(lerp((f32)r.x0, div->pos, (f32)r.x1));
            }
            else{
                r.x0 = ROUND32(lerp((f32)r.x0, div->pos, (f32)r.x1));
            }
        }
        else{
            if (div->child1 == divider_chain[i-1]){
                r.y1 = ROUND32(lerp((f32)r.y0, div->pos, (f32)r.y1));
            }
            else{
                r.y0 = ROUND32(lerp((f32)r.y0, div->pos, (f32)r.y1));
            }
        }
    }
    
    switch (which_child){
        case 1:
        {
            if (original_div->v_divider){
                r.x0 = ROUND32(lerp((f32)r.x0, original_div->pos, (f32)r.x1));
            }
            else{
                r.y0 = ROUND32(lerp((f32)r.y0, original_div->pos, (f32)r.y1));
            }
        }break;
        
        case -1:
        {
            if (original_div->v_divider){
                r.x1 = ROUND32(lerp((f32)r.x0, original_div->pos, (f32)r.x1));
            }
            else{
                r.y1 = ROUND32(lerp((f32)r.y0, original_div->pos, (f32)r.y1));
            }
        }break;
    }
    
    return(r);
}

internal i32_Rect
layout_get_panel_rect(Editing_Layout *layout, Panel *panel){
    Assert(layout->panel_count > 1);
    
    i32_Rect r = layout_get_rect(layout, panel->parent, panel->which_child);
    
    return(r);
}

internal void
layout_fix_all_panels(Editing_Layout *layout){
    Panel *panel;
    Panel_Divider *dividers = layout->dividers; AllowLocal(dividers);
    i32 panel_count = layout->panel_count;
    
    if (panel_count > 1){
        for (panel = layout->used_sentinel.next;
             panel != &layout->used_sentinel;
             panel = panel->next){
            panel->full = layout_get_panel_rect(layout, panel);
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
    
    layout->panel_state_dirty = 1;
}

internal void
layout_refit(Editing_Layout *layout, i32 prev_width, i32 prev_height){
    Panel_Divider *dividers = layout->dividers;
    i32 max = layout->panel_max_count - 1;
    
    Panel_Divider *divider = dividers;
    
    if (layout->panel_count > 1){
        Assert(prev_width != 0 && prev_height != 0);
        
        for (i32 i = 0; i < max; ++i, ++divider){
            if (divider->v_divider){
                divider->pos = divider->pos;
            }
            else{
                divider->pos = divider->pos;
            }
        }
    }
    
    layout_fix_all_panels(layout);
}

internal f32
layout_get_position(Editing_Layout *layout, i32 id){
    Panel_Divider *dividers = layout->dividers;
    Panel_Divider *original_div = dividers + id;
    
    i32_Rect r = layout_get_rect(layout, id, 0);
    f32 pos = 0;
    if (original_div->v_divider){
        pos = lerp((f32)r.x0, original_div->pos, (f32)r.x1);
    }
    else{
        pos = lerp((f32)r.y0, original_div->pos, (f32)r.y1);
    }
    
    return(pos);
}

internal f32
layout_compute_position(Editing_Layout *layout, Panel_Divider *divider, i32 pos){
    Panel_Divider *dividers = layout->dividers;
    Panel_Divider *original_div = divider;
    i32 id = (i32)(divider - dividers);
    
    i32_Rect r = layout_get_rect(layout, id, 0);
    f32 l = 0;
    if (original_div->v_divider){
        l = unlerp((f32)r.x0, (f32)pos, (f32)r.x1);
    }
    else{
        l = unlerp((f32)r.y0, (f32)pos, (f32)r.y1);
    }
    
    Assert(0.f <= l && l <= 1.f);
    
    return(l);
}

internal void
layout_compute_abs_step(Editing_Layout *layout, i32 divider_id, i32_Rect rect, i32 *abs_pos){
    Panel_Divider *div = layout->dividers + divider_id;
    i32 p0 = 0, p1 = 0;
    
    if (div->v_divider){
        p0 = rect.x0; p1 = rect.x1;
    }
    else{
        p0 = rect.y0; p1 = rect.y1;
    }
    
    i32 pos = lerp(p0, div->pos, p1);
    i32_Rect r1 = rect, r2 = rect;
    
    abs_pos[divider_id] = pos;
    
    if (div->v_divider){
        r1.x1 = pos; r2.x0 = pos;
    }
    else{
        r1.y1 = pos; r2.y0 = pos;
    }
    
    if (div->child1 != -1){
        layout_compute_abs_step(layout, div->child1, r1, abs_pos);
    }
    
    if (div->child2 != -1){
        layout_compute_abs_step(layout, div->child2, r2, abs_pos);
    }
}

internal void
layout_compute_absolute_positions(Editing_Layout *layout, i32 *abs_pos){
    i32_Rect r;
    r.x0 = 0;
    r.y0 = 0;
    r.x1 = layout->full_width;
    r.y1 = layout->full_height;
    if (layout->panel_count > 1){
        layout_compute_abs_step(layout, layout->root, r, abs_pos);
    }
}

internal void
layout_get_min_max_step_up(Editing_Layout *layout, b32 v, i32 divider_id, i32 which_child,
                           i32 *abs_pos, i32 *min_out, i32 *max_out){
    Panel_Divider *divider = layout->dividers + divider_id;
    
    if (divider->v_divider == v){
        if (which_child == -1){
            if (*max_out > abs_pos[divider_id]){
                *max_out = abs_pos[divider_id];
            }
        }
        else{
            if (*min_out < abs_pos[divider_id]){
                *min_out = abs_pos[divider_id];
            }
        }
    }
    
    if (divider->parent != -1){
        layout_get_min_max_step_up(layout, v, divider->parent, divider->which_child,
                                   abs_pos, min_out, max_out);
    }
}

internal void
layout_get_min_max_step_down(Editing_Layout *layout, b32 v, i32 divider_id, i32 which_child,
                             i32 *abs_pos, i32 *min_out, i32 *max_out){
    Panel_Divider *divider = layout->dividers + divider_id;
    
    // NOTE(allen): The min/max is switched here, because children on the -1 side
    // effect min, while if you are on the -1 side your parent effects max.
    if (divider->v_divider == v){
        if (which_child == -1){
            if (*min_out < abs_pos[divider_id]){
                *min_out = abs_pos[divider_id];
            }
        }
        else{
            if (*max_out > abs_pos[divider_id]){
                *max_out = abs_pos[divider_id];
            }
        }
    }
    
    if (divider->child1 != -1){
        layout_get_min_max_step_down(layout, v, divider->child1, which_child,
                                     abs_pos, min_out, max_out);
    }
    
    if (divider->child2 != -1){
        layout_get_min_max_step_down(layout, v, divider->child2, which_child,
                                     abs_pos, min_out, max_out);
    }
}

internal void
layout_get_min_max(Editing_Layout *layout, Panel_Divider *divider, i32 *abs_pos, i32 *min_out, i32 *max_out){
    *min_out = 0;
    *max_out = max_i32;
    
    if (layout->panel_count > 1){
        if (divider->parent != -1){
            layout_get_min_max_step_up(layout, divider->v_divider, divider->parent, divider->which_child,
                                       abs_pos, min_out, max_out);
        }
        
        if (divider->child1 != -1){
            layout_get_min_max_step_down(layout, divider->v_divider, divider->child1, -1,
                                         abs_pos, min_out, max_out);
        }
        
        if (divider->child2 != -1){
            layout_get_min_max_step_down(layout, divider->v_divider, divider->child2, 1,
                                         abs_pos, min_out, max_out);
        }
    }
    else{
        if (divider->v_divider){
            *max_out = layout->full_width;
        }
        else{
            *max_out = layout->full_height;
        }
    }
}

internal void
layout_update_pos_step(Editing_Layout *layout, i32 divider_id, i32_Rect rect, i32 *abs_pos){
    Panel_Divider *div = layout->dividers + divider_id;
    i32 p0 = 0, p1 = 0;
    
    if (div->v_divider){
        p0 = rect.x0; p1 = rect.x1;
    }
    else{
        p0 = rect.y0; p1 = rect.y1;
    }
    
    i32 pos = abs_pos[divider_id];
    i32_Rect r1 = rect, r2 = rect;
    f32 lpos = unlerp((f32)p0, (f32)pos, (f32)p1);
    
    div->pos = lpos;
    
    if (div->v_divider){
        pos = clamp(r1.x0, pos, r2.x1);
        r1.x1 = pos; r2.x0 = pos;
    }
    else{
        pos = clamp(r1.y0, pos, r2.y1);
        r1.y1 = pos; r2.y0 = pos;
    }
    
    if (div->child1 != -1){
        layout_update_pos_step(layout, div->child1, r1, abs_pos);
    }
    
    if (div->child2 != -1){
        layout_update_pos_step(layout, div->child2, r2, abs_pos);
    }
}

internal void
layout_update_all_positions(Editing_Layout *layout, i32 *abs_pos){
    i32_Rect r;
    r.x0 = 0;
    r.y0 = 0;
    r.x1 = layout->full_width;
    r.y1 = layout->full_height;
    if (layout->panel_count > 1){
        layout_update_pos_step(layout, layout->root, r, abs_pos);
    }
}

// BOTTOM

