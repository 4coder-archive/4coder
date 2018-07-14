/*
 * Helpers for ui data structures.
 */

// TOP

static UI_Item*
ui_list_add_item(Partition *arena, UI_List *list, UI_Item item){
    UI_Item_Node *node = push_array(arena, UI_Item_Node, 1);
    zdll_push_back(list->first, list->last, node);
    list->count += 1;
    node->fixed = item;
    return(&node->fixed);
}

static UI_Control
ui_list_to_ui_control(Partition *arena, UI_List *list){
    UI_Control control = {0};
    control.items = push_array(arena, UI_Item, list->count);
    for (UI_Item_Node *node = list->first;
         node != 0;
         node = node->next){
        control.items[control.count++] = node->fixed;
    }
    return(control);
}

static UI_Item*
ui_control_get_mouse_hit(UI_Control *control, int32_t mx, int32_t my){
    int32_t count = control->count;
    UI_Item *item = control->items + count - 1;
    for (int32_t i = 0; i < count; ++i, item -= 1){
        i32_Rect r = item->rectangle;
        if (r.x0 <= mx && mx < r.x1 && r.y0 <= my && my < r.y1){
            return(item);
        }
    }
    return(0);
}

// BOTTOM

