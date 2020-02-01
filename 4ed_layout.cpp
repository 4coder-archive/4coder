/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Panel layout functions
 *
 */

// TOP

internal Panel_Split
make_panel_split(Panel_Split_Kind kind, i32 v){
    Panel_Split split = {};
    split.kind = kind;
    split.v_i32 = v;
    return(split);
}

internal Panel_Split
make_panel_split(Panel_Split_Kind kind, f32 v){
    Panel_Split split = {};
    split.kind = kind;
    split.v_f32 = v;
    return(split);
}

internal Panel_Split
make_panel_split_50_50(void){
    return(make_panel_split(PanelSplitKind_Ratio_Min, 0.5f));
}

internal Panel*
layout__alloc_panel(Layout *layout){
    Panel *panel = 0;
    Node *node = layout->free_panels.next;
    if (node != &layout->free_panels){
        dll_remove(node);
        panel = CastFromMember(Panel, node, node);
    }
    return(panel);
}

internal void
layout__free_panel(Layout *layout, Panel *panel){
    Assert(panel != layout->active_panel);
    Assert(panel != layout->root);
    dll_remove(&panel->node);
    dll_insert(&layout->free_panels, &panel->node);
    panel->kind = PanelKind_Unused;
}

internal void
layout__set_panel_rectangle(Layout *layout, Panel *panel, Rect_i32 rect){
    panel->rect_full = rect;
    panel->rect_inner = rect_inner(rect, layout->margin);
}

internal i32
layout__evaluate_split(Panel_Split split, i32 v0, i32 v1){
    i32 v = 0;
    switch (split.kind){
        case PanelSplitKind_Ratio_Min:
        {
            v = i32_round32(lerp((f32)v0, split.v_f32, (f32)v1));
        }break;
        case PanelSplitKind_Ratio_Max:
        {
            v = i32_round32(lerp((f32)v1, split.v_f32, (f32)v0));
        }break;
        case PanelSplitKind_FixedPixels_Min:
        {
            v = clamp_top(v0 + split.v_i32, v1);
        }break;
        case PanelSplitKind_FixedPixels_Max:
        {
            v = clamp_bot(v0, v1 - split.v_i32);
        }break;
    }
    return(v);
}

internal void
layout_propogate_sizes_down_from_node(Layout *layout, Panel *panel){
    if (panel->kind == PanelKind_Intermediate){
        Panel *tl_panel = panel->tl_panel;
        Panel *br_panel = panel->br_panel;
        
        Rect_i32 r1 = panel->rect_full;
        Rect_i32 r2 = panel->rect_full;
        
        if (panel->vertical_split){
            i32 x_pos = layout__evaluate_split(panel->split, r1.x0, r1.x1);
            r1.x1 = x_pos;
            r2.x0 = x_pos;
        }
        else{
            i32 y_pos = layout__evaluate_split(panel->split, r1.y0, r1.y1);
            r1.y1 = y_pos;
            r2.y0 = y_pos;
        }
        
        layout__set_panel_rectangle(layout, tl_panel, r1);
        layout__set_panel_rectangle(layout, br_panel, r2);
        
        layout_propogate_sizes_down_from_node(layout, tl_panel);
        layout_propogate_sizes_down_from_node(layout, br_panel);
    }
}

internal i32
layout_get_open_panel_count(Layout *layout){
    return(layout->open_panel_count);
}

internal Panel*
layout_get_first_open_panel(Layout *layout){
    Panel *panel = CastFromMember(Panel, node, layout->open_panels.next);
    if (panel != 0 && &panel->node == &layout->open_panels){
        panel = 0;
    }
    AssertImplies(panel != 0, panel->kind == PanelKind_Final);
    return(panel);
}

internal Panel*
layout_get_last_open_panel(Layout *layout){
    Panel *panel = CastFromMember(Panel, node, layout->open_panels.prev);
    if (panel != 0 && &panel->node == &layout->open_panels){
        panel = 0;
    }
    AssertImplies(panel != 0, panel->kind == PanelKind_Final);
    return(panel);
}

internal Panel*
layout_get_next_open_panel(Layout *layout, Panel *panel){
    panel = CastFromMember(Panel, node, panel->node.next);
    if (&panel->node == &layout->open_panels){
        panel = 0;
    }
    AssertImplies(panel != 0, panel->kind == PanelKind_Final);
    return(panel);
}

internal Panel*
layout_get_prev_open_panel(Layout *layout, Panel *panel){
    panel = CastFromMember(Panel, node, panel->node.prev);
    if (&panel->node == &layout->open_panels){
        panel = 0;
    }
    AssertImplies(panel != 0, panel->kind == PanelKind_Final);
    return(panel);
}

internal Panel*
layout_get_active_panel(Layout *layout){
    return(layout->active_panel);
}

internal b32
layout_split_panel(Layout *layout, Panel *panel, b32 vertical_split, Panel **new_panel_out){
    b32 result = false;
    if (layout->open_panel_count < layout->open_panel_max_count){
        Panel *min_panel = layout__alloc_panel(layout);
        Panel *max_panel = layout__alloc_panel(layout);
        
        if (panel->kind == PanelKind_Final){
            dll_remove(&panel->node);
            dll_insert(&layout->intermediate_panels, &panel->node);
            
            // init min_panel
            dll_insert(&layout->open_panels, &min_panel->node);
            
            panel->view->panel = min_panel;
            min_panel->parent = panel;
            min_panel->kind = PanelKind_Final;
            min_panel->view = panel->view;
        }
        else{
            // init min_panel
            dll_insert(&layout->intermediate_panels, &min_panel->node);
            
            panel->tl_panel->parent = min_panel;
            panel->br_panel->parent = min_panel;
            min_panel->parent = panel;
            min_panel->kind = PanelKind_Intermediate;
            min_panel->tl_panel = panel->tl_panel;
            min_panel->br_panel = panel->br_panel;
            min_panel->vertical_split = panel->vertical_split;
            min_panel->split = panel->split;
        }
        
        // init max_panel
        dll_insert(&layout->open_panels, &max_panel->node);
        
        *new_panel_out = max_panel;
        max_panel->parent = panel;
        max_panel->kind = PanelKind_Final;
        max_panel->view = 0;
        
        // modify panel
        panel->kind = PanelKind_Intermediate;
        panel->tl_panel = min_panel;
        panel->br_panel = max_panel;
        panel->vertical_split = vertical_split;
        panel->split = make_panel_split_50_50();
        
        // propogate rectangle sizes down from the new intermediate to
        // resize the panel and the new panel.
        layout_propogate_sizes_down_from_node(layout, panel);
        
        // update layout state
        layout->open_panel_count += 1;
        layout->active_panel = max_panel;
        layout->panel_state_dirty = true;
        
        result = true;
    }
    return(result);
}

internal b32
layout_close_panel(Layout *layout, Panel *panel){
    b32 result = false;
    if (layout->open_panel_count > 1){
        Panel *parent = panel->parent;
        Assert(parent != 0);
        
        // find sibling
        Panel *sibling = 0;
        if (parent->tl_panel == panel){
            sibling = parent->br_panel;
        }
        else{
            Assert(parent->br_panel == panel);
            sibling = parent->tl_panel;
        }
        
        // update layout state
        if (layout->active_panel == panel){
            Panel *new_active = sibling;
            for (;new_active->kind == PanelKind_Intermediate;){
                new_active = new_active->br_panel;
            }
            layout->active_panel = new_active;
        }
        layout->panel_state_dirty = true;
        layout->open_panel_count -= 1;
        
        // link grand parent and sibling
        Panel *g_parent = parent->parent;
        sibling->parent = g_parent;
        if (g_parent != 0){
            if (g_parent->tl_panel == parent){
                g_parent->tl_panel = sibling;
            }
            else{
                Assert(g_parent->br_panel == parent);
                g_parent->br_panel = sibling;
            }
        }
        else{
            Assert(parent == layout->root);
            layout->root = sibling;
        }
        
        // set sibling's size
        sibling->screen_region = parent->screen_region;
        
        // set the sizes down stream of sibling
        layout_propogate_sizes_down_from_node(layout, sibling);
        
        // free panel and parent
        layout__free_panel(layout, panel);
        layout__free_panel(layout, parent);
        
        result = true;
    }
    return(result);
}

internal Panel*
layout_initialize(Arena *arena, Layout *layout){
    i32 panel_alloc_count = MAX_VIEWS*2 - 1;
    Panel *panels = push_array(arena, Panel, panel_alloc_count);
    block_zero(panels, sizeof(*panels)*panel_alloc_count);
    
    layout->panel_first = panels;
    layout->panel_one_past_last = panels + panel_alloc_count;
    
    layout->margin = 3;
    layout->open_panel_count = 0;
    layout->open_panel_max_count = MAX_VIEWS;
    
    dll_init_sentinel(&layout->open_panels);
    dll_init_sentinel(&layout->intermediate_panels);
    
    Panel *panel = panels;
    layout->free_panels.next = &panel->node;
    panel->node.prev = &layout->free_panels;
    for (i32 i = 1; i < panel_alloc_count; i += 1, panel += 1){
        panel[1].node.prev = &panel[0].node;
        panel[0].node.next = &panel[1].node;
    }
    panel->node.next = &layout->free_panels;
    layout->free_panels.prev = &panel->node;
    
    panel = layout__alloc_panel(layout);
    panel->parent = 0;
    panel->kind = PanelKind_Final;
    panel->view = 0;
    block_zero_struct(&panel->screen_region);
    
    dll_insert(&layout->open_panels, &panel->node);
    layout->open_panel_count += 1;
    layout->root = panel;
    layout->active_panel = panel;
    layout->panel_state_dirty = true;
    
    return(panel);
}

internal void
layout_set_margin(Layout *layout, i32 margin){
    if (layout->margin != margin){
        layout->margin = margin;
        layout__set_panel_rectangle(layout, layout->root, Ri32(0, 0, layout->full_dim.x, layout->full_dim.y));
        layout_propogate_sizes_down_from_node(layout, layout->root);
        layout->panel_state_dirty = true;
    }
}

internal void
layout_set_root_size(Layout *layout, Vec2_i32 dim){
    if (layout->full_dim != dim){
        layout->full_dim = dim;
        layout__set_panel_rectangle(layout, layout->root, Ri32(0, 0, dim.x, dim.y));
        layout_propogate_sizes_down_from_node(layout, layout->root);
        layout->panel_state_dirty = true;
    }
}

internal Vec2_i32
layout_get_root_size(Layout *layout){
    return(layout->full_dim);
}

internal i32
layout_get_absolute_position_of_split(Panel *panel){
    i32 pos = 0;
    if (panel->vertical_split){
        pos = layout__evaluate_split(panel->split, panel->rect_full.x0, panel->rect_full.x1);
    }
    else{
        pos = layout__evaluate_split(panel->split, panel->rect_full.y0, panel->rect_full.y1);
    }
    return(pos);
}

internal Range_i32
layout__get_limiting_range_on_split_children(i32 mid, b32 vertical_split, Panel *panel, Range_i32 range){
    if (panel->kind == PanelKind_Intermediate){
        if (vertical_split == panel->vertical_split){
            i32 pos = layout_get_absolute_position_of_split(panel);
            if (mid < pos && pos < range.max){
                range.max = pos;
            }
            else if (range.min < pos && pos < mid){
                range.min = pos;
            }
        }
        range = layout__get_limiting_range_on_split_children(mid, vertical_split, panel->tl_panel, range);
        range = layout__get_limiting_range_on_split_children(mid, vertical_split, panel->br_panel, range);
    }
    return(range);
}

internal Range_i32
layout_get_limiting_range_on_split(Layout *layout, Panel *panel){
    // root level min max
    Range_i32 range = {};
    if (panel->vertical_split){
        range.max = layout->full_dim.x;
    }
    else{
        range.max = layout->full_dim.y;
    }
    
    // get mid
    i32 mid = layout_get_absolute_position_of_split(panel);
    
    // parents min max
    for (Panel *panel_it = panel;
         panel_it != 0;
         panel_it = panel_it->parent){
        if (panel->vertical_split == panel_it->vertical_split){
            i32 pos = layout_get_absolute_position_of_split(panel_it);
            if (mid < pos && pos < range.max){
                range.max = pos;
            }
            else if (range.min < pos && pos < mid){
                range.min = pos;
            }
        }
    }
    
    // children min max
    if (panel->kind == PanelKind_Intermediate){
        range = layout__get_limiting_range_on_split_children(mid, panel->vertical_split, panel->tl_panel, range);
        range = layout__get_limiting_range_on_split_children(mid, panel->vertical_split, panel->br_panel, range);
    }
    
    return(range);
}

internal void
layout__reverse_evaluate_panel_split(Panel *panel, i32 position){
    i32 v0 = 0;
    i32 v1 = 0;
    if (panel->vertical_split){
        v0 = panel->rect_full.x0;
        v1 = panel->rect_full.x1;
    }
    else{
        v0 = panel->rect_full.y0;
        v1 = panel->rect_full.y1;
    }
    switch (panel->split.kind){
        case PanelSplitKind_Ratio_Min:
        {
            panel->split.v_f32 = unlerp((f32)v0, (f32)position, (f32)v1);
        }break;
        case PanelSplitKind_Ratio_Max:
        {
            panel->split.v_f32 = unlerp((f32)v1, (f32)position, (f32)v0);
        }break;
        case PanelSplitKind_FixedPixels_Min:
        {
            panel->split.v_i32 = clamp(v0, position, v1) - v0;
        }break;
        case PanelSplitKind_FixedPixels_Max:
        {
            panel->split.v_i32 = v1 - clamp(v0, position, v1);
        }break;
    }
}

internal void
layout__set_split_absolute_position_inner(Panel *panel){
    if (panel->kind == PanelKind_Intermediate){
        Rect_i32 r = panel->rect_full;
        i32 position = 0;
        if (panel->vertical_split){
            position = layout__evaluate_split(panel->split, r.x0, r.x1);
        }
        else{
            position = layout__evaluate_split(panel->split, r.y0, r.y1);
        }
        layout__reverse_evaluate_panel_split(panel, position);
    }
}

internal void
layout_set_split_absolute_position(Layout *layout, Panel *panel, i32 absolute_position){
    if (panel->kind == PanelKind_Intermediate){
        layout__reverse_evaluate_panel_split(panel, absolute_position);
        layout__set_split_absolute_position_inner(panel->tl_panel);
        layout__set_split_absolute_position_inner(panel->br_panel);
        layout_propogate_sizes_down_from_node(layout, panel);
        layout->panel_state_dirty = true;
    }
}

internal Panel_ID
panel_get_id(Layout *layout, Panel *panel){
    Panel_ID id = 0;
    if (layout->panel_first <= panel && panel < layout->panel_one_past_last){
        id = (Panel_ID)(panel - layout->panel_first) + 1;
    }
    return(id);
}

////////////////////////////////

internal Panel*
imp_get_panel(Models *models, Panel_ID panel_id){
    Layout *layout = &models->layout;
    Panel *panel = layout->panel_first + panel_id - 1;
    if (!(layout->panel_first <= panel && panel < layout->panel_one_past_last)){
        panel = 0;
    }
    return(panel);
}

// BOTTOM

