/*
 * Helpers for ui data structures.
 */

// TOP

// TODO(allen): documentation comment here

static UI_Item*
ui_list_add_item(Partition *arena, UI_List *list, UI_Item item){
    UI_Item_Node *node = push_array(arena, UI_Item_Node, 1);
    zdll_push_back(list->first, list->last, node);
    list->count += 1;
    node->fixed = item;
    return(&node->fixed);
}

static i32_Rect
ui__rect_union(i32_Rect a, i32_Rect b){
    if (b.x1 > b.x0 && b.y1 > b.y0){
        if (a.x0 > b.x0){
            a.x0 = b.x0;
        }
        if (a.x1 < b.x1){
            a.x1 = b.x1;
        }
        if (a.y0 > b.y0){
            a.y0 = b.y0;
        }
        if (a.y1 < b.y1){
            a.y1 = b.y1;
        }
    }
    return(a);
}

static UI_Control
ui_list_to_ui_control(Partition *arena, UI_List *list){
    UI_Control control = {};
    control.items = push_array(arena, UI_Item, list->count);
    i32_Rect neg_inf_rect = {};
    neg_inf_rect.x0 = INT32_MAX;
    neg_inf_rect.y0 = INT32_MAX;
    neg_inf_rect.x1 = INT32_MIN;
    neg_inf_rect.y1 = INT32_MIN;
    for (uint32_t i = 0; i < UICoordinates_COUNT; ++i){
        control.bounding_box[i] = neg_inf_rect;
    }
    for (UI_Item_Node *node = list->first;
         node != 0;
         node = node->next){
        UI_Item *item = &control.items[control.count++];
        *item = node->fixed;
        if (item->coordinates >= UICoordinates_COUNT){
            item->coordinates = UICoordinates_Scrolled;
        }
        control.bounding_box[item->coordinates] = ui__rect_union(control.bounding_box[item->coordinates], item->rectangle);
    }
    return(control);
}

static void
ui_control_set_top(UI_Control *control, int32_t top_y){
    control->bounding_box[UICoordinates_Scrolled].y0 = top_y;
}

static void
ui_control_set_bottom(UI_Control *control, int32_t bottom_y){
    control->bounding_box[UICoordinates_Scrolled].y1 = bottom_y;
}

static UI_Item*
ui_control_get_mouse_hit(UI_Control *control,
                         int32_t mx_scrolled, int32_t my_scrolled,
                         int32_t mx_unscrolled, int32_t my_unscrolled){
    int32_t count = control->count;
    UI_Item *item = control->items + count - 1;
    for (int32_t i = 0; i < count; ++i, item -= 1){
        i32_Rect r = item->rectangle;
        switch (item->coordinates){
            case UICoordinates_Scrolled:
            {
                if (r.x0 <= mx_scrolled && mx_scrolled < r.x1 && r.y0 <= my_scrolled && my_scrolled < r.y1){
                    return(item);
                }
            }break;
            
            case UICoordinates_ViewRelative:
            {
                if (r.x0 <= mx_unscrolled && mx_unscrolled < r.x1 && r.y0 <= my_unscrolled && my_unscrolled < r.y1){
                    return(item);
                }
            }break;
        }
    }
    return(0);
}

////////////////////////////////

static void
view_zero_scroll(Application_Links *app, View_Summary *view){
    GUI_Scroll_Vars zero_scroll = {};
    view_set_scroll(app, view, zero_scroll);
}

static void
view_set_vertical_focus(Application_Links *app, View_Summary *view,
                        int32_t y_top, int32_t y_bot){
    GUI_Scroll_Vars scroll = view->scroll_vars;
    int32_t view_y_top = scroll.target_y;
    int32_t view_y_dim = view->file_region.y1 - view->file_region.y0;
    int32_t view_y_bot = view_y_top + view_y_dim;
    int32_t line_dim = (int32_t)view->line_height;
    int32_t hot_y_top = view_y_top + line_dim*3;
    int32_t hot_y_bot = view_y_bot - line_dim*3;
    if (hot_y_bot - hot_y_top < line_dim*6){
        int32_t quarter_view_y_dim = view_y_dim/4;
        hot_y_top = view_y_top + quarter_view_y_dim;
        hot_y_bot = view_y_bot - quarter_view_y_dim;
    }
    int32_t hot_y_dim = hot_y_bot - hot_y_top;
    int32_t skirt_dim = hot_y_top - view_y_top;
    int32_t y_dim = y_bot - y_top;
    if (y_dim > hot_y_dim){
        scroll.target_y = y_top - skirt_dim;
        view_set_scroll(app, view, scroll);
    }
    else{
        if (y_top < hot_y_top){
            scroll.target_y = y_top - skirt_dim;
            view_set_scroll(app, view, scroll);
        }
        else if (y_bot > hot_y_bot){
            scroll.target_y = y_bot + skirt_dim - view_y_dim;
            view_set_scroll(app, view, scroll);
        }
    }
}

static void
get_view_relative_points(int32_t x_window, int32_t y_window, View_Summary view,
                         int32_t *x_scrolled_out, int32_t *y_scrolled_out,
                         int32_t *x_view_out, int32_t *y_view_out){
    int32_t x = x_window - view.file_region.x0;
    int32_t y = y_window - view.file_region.y0;
    if (x_scrolled_out != 0 && y_scrolled_out != 0){
        *x_scrolled_out = x + (int32_t)view.scroll_vars.scroll_x;
        *y_scrolled_out = y + (int32_t)view.scroll_vars.scroll_y;
    }
    if (x_view_out != 0 && y_view_out != 0){
        *x_view_out = x;
        *y_view_out = y;
    }
}

static void
get_view_relative_points_x(int32_t x_window, View_Summary view, int32_t *x_scrolled_out, int32_t *x_view_out){
    int32_t y_ignore = 0;
    get_view_relative_points(x_window, y_ignore, view,
                             x_scrolled_out, &y_ignore, x_view_out, &y_ignore);
}

static void
get_view_relative_points_y(int32_t y_window, View_Summary view, int32_t *y_scrolled_out, int32_t *y_view_out){
    int32_t x_ignore = 0;
    get_view_relative_points(x_ignore, y_window, view,
                             &x_ignore, y_scrolled_out, &x_ignore, y_view_out);
}

static void
get_view_relative_mouse_positions(Mouse_State mouse, View_Summary view,
                                  int32_t *x_scrolled_out, int32_t *y_scrolled_out,
                                  int32_t *x_view_out, int32_t *y_view_out){
    get_view_relative_points(mouse.x, mouse.y, view,
                             x_scrolled_out, y_scrolled_out,
                             x_view_out, y_view_out);
}

static void
get_view_relative_mouse_positions(Application_Links *app, View_Summary view,
                                  int32_t *x_scrolled_out, int32_t *y_scrolled_out,
                                  int32_t *x_view_out, int32_t *y_view_out){
    get_view_relative_mouse_positions(get_mouse_state(app), view,
                                      x_scrolled_out, y_scrolled_out,
                                      x_view_out, y_view_out);
}

////////////////////////////////

Lister_State global_lister_state_[16] = {};
Lister_State *global_lister_state = global_lister_state_ - 1;

static Lister_State*
view_get_lister_state(View_Summary *view){
    return(&global_lister_state[view->view_id]);
}

static int32_t
lister_standard_arena_size_round_up(int32_t arena_size){
    if (arena_size < (64 << 10)){
        arena_size = (64 << 10);
    }
    else{
        arena_size += (4 << 10) - 1;
        arena_size -= arena_size%(4 << 10);
    }
    return(arena_size);
}

static void
lister_arena_clear_data_ensure_bytes(Application_Links *app, Lister *lister, int32_t minimum_bytes){
    if (lister->arena.base == 0 || lister->arena.max < minimum_bytes){
        int32_t rounded_user_data_size = lister->data.user_data_size;
        rounded_user_data_size += 7;
        rounded_user_data_size -= rounded_user_data_size%8;
        int32_t new_size = (lister->arena.max + rounded_user_data_size + minimum_bytes + 1)*2;
        new_size = lister_standard_arena_size_round_up(new_size);
        void *new_memory = memory_allocate(app, new_size);
        memcpy(new_memory, lister->data.user_data, lister->data.user_data_size);
        lister->data.user_data = new_memory;
        if (lister->arena.base != 0){
            memory_free(app, lister->arena.base, lister->arena.max);
        }
        lister->arena = make_part(new_memory, new_size);
        push_array(&lister->arena, char, lister->data.user_data_size);
    }
}

static void
init_lister_state(Application_Links *app, Lister_State *state, Heap *heap){
    state->initialized = true;
    state->hot_user_data = 0;
    state->item_index = 0;
    state->set_view_vertical_focus_to_item = false;
    state->item_count_after_filter = 0;
    if (state->lister.arena.base != 0){
        memory_free(app, state->lister.arena.base, state->lister.arena.max);
    }
    memset(&state->lister, 0, sizeof(state->lister));
}

UI_QUIT_FUNCTION(lister_quit_function){
    Lister_State *state = view_get_lister_state(&view);
    state->initialized = false;
}

static UI_Item
lister_get_clicked_item(Application_Links *app, View_Summary *view, Partition *scratch){
    Temp_Memory temp = begin_temp_memory(scratch);
    UI_Control control = view_get_ui_copy(app, view, scratch);
    Mouse_State mouse = get_mouse_state(app);
    int32_t mxs = 0;
    int32_t mys = 0;
    int32_t mxu = 0;
    int32_t myu = 0;
    get_view_relative_mouse_positions(mouse, *view, &mxs, &mys, &mxu, &myu);
    UI_Item *clicked = ui_control_get_mouse_hit(&control, mxs, mys, mxu, myu);
    UI_Item result = {};
    if (clicked != 0){
        result = *clicked;
    }
    end_temp_memory(temp);
    return(result);
}

static int32_t
lister_get_line_height(View_Summary *view){
    return((int32_t)view->line_height);
}

static int32_t
lister_get_text_field_height(View_Summary *view){
    return((int32_t)view->line_height);
}

static int32_t
lister_get_block_height(int32_t line_height, bool32 is_theme_list){
    int32_t block_height = 0;
    if (is_theme_list){
        block_height = line_height*3 + 6;
    }
    else{
        block_height = line_height*2;
    }
    return(block_height);
}

static void
lister_update_ui(Application_Links *app, Partition *scratch, View_Summary *view,
                 Lister_State *state){
    bool32 is_theme_list = state->lister.data.theme_list;
    
    int32_t x0 = 0;
    int32_t x1 = view->view_region.x1 - view->view_region.x0;
    int32_t line_height = lister_get_line_height(view);
    int32_t block_height = lister_get_block_height(line_height, is_theme_list);
    int32_t text_field_height = lister_get_text_field_height(view);
    
    Temp_Memory full_temp = begin_temp_memory(scratch);
    
    int32_t mx = 0;
    int32_t my = 0;
    refresh_view(app, view);
    get_view_relative_mouse_positions(app, *view, &mx, &my, 0, 0);
    
    int32_t y_pos = text_field_height;
    
    state->raw_item_index = -1;
    
    int32_t node_count = state->lister.data.options.count;
    Lister_Node_Ptr_Array exact_matches = {};
    exact_matches.node_ptrs = push_array(scratch, Lister_Node*, 1);
    Lister_Node_Ptr_Array before_extension_matches = {};
    before_extension_matches.node_ptrs = push_array(scratch, Lister_Node*, node_count);
    Lister_Node_Ptr_Array substring_matches = {};
    substring_matches.node_ptrs = push_array(scratch, Lister_Node*, node_count);
    
    String key = state->lister.data.key_string;
    Absolutes absolutes = {};
    get_absolutes(key, &absolutes, true, true);
    bool32 has_wildcard = (absolutes.count > 3);
    
    for (Lister_Node *node = state->lister.data.options.first;
         node != 0;
         node = node->next){
        if (key.size == 0 ||
            wildcard_match_s(&absolutes, node->string, false)){
            if (match_insensitive(node->string, key) && exact_matches.count == 0){
                exact_matches.node_ptrs[exact_matches.count++] = node;
            }
            else if (!has_wildcard &&
                     match_part_insensitive(node->string, key) &&
                     node->string.size > key.size &&
                     node->string.str[key.size] == '.'){
                before_extension_matches.node_ptrs[before_extension_matches.count++] = node;
            }
            else{
                substring_matches.node_ptrs[substring_matches.count++] = node;
            }
        }
    }
    
    Lister_Node_Ptr_Array node_ptr_arrays[] = {
        exact_matches,
        before_extension_matches,
        substring_matches,
    };
    
    UI_List list = {};
    UI_Item *highlighted_item = 0;
    UI_Item *hot_item = 0;
    UI_Item *hovered_item = 0;
    int32_t item_index_counter = 0;
    for (int32_t array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        for (int32_t node_index = 0; node_index < node_ptr_array.count; node_index += 1){
            Lister_Node *node = node_ptr_array.node_ptrs[node_index];
            
            i32_Rect item_rect = {};
            item_rect.x0 = x0;
            item_rect.y0 = y_pos;
            item_rect.x1 = x1;
            item_rect.y1 = y_pos + block_height;
            y_pos = item_rect.y1;
            
            UI_Item item = {};
            if (!is_theme_list){
                item.type = UIType_Option;
                item.option.string = node->string;
                item.option.status = node->status;
            }
            else{
                item.type = UIType_ColorTheme;
                item.color_theme.string = node->string;
                item.color_theme.index = node->index;
            }
            item.activation_level = UIActivation_None;
            item.coordinates = UICoordinates_Scrolled;
            item.user_data = node->user_data;
            item.rectangle = item_rect;
            
            UI_Item *item_ptr = ui_list_add_item(scratch, &list, item);
            if (item_rect.x0 <= mx && mx < item_rect.x1 &&
                item_rect.y0 <= my && my < item_rect.y1){
                hovered_item = item_ptr;
            }
            if (state->item_index == item_index_counter){
                highlighted_item = item_ptr;
                state->raw_item_index = node->raw_index;
            }
            item_index_counter += 1;
            if (node->user_data == state->hot_user_data && hot_item != 0){
                hot_item = item_ptr;
            }
        }
    }
    state->item_count_after_filter = item_index_counter;
    
    if (hovered_item != 0){
        hovered_item->activation_level = UIActivation_Hover;
    }
    if (hot_item != 0){
        if (hot_item == hovered_item){
            hot_item->activation_level = UIActivation_Active;
        }
        else{
            hot_item->activation_level = UIActivation_Hover;
        }
    }
    if (highlighted_item != 0){
        highlighted_item->activation_level = UIActivation_Active;
    }
    
    if (state->set_view_vertical_focus_to_item){
        if (highlighted_item != 0){
            view_set_vertical_focus(app, view, highlighted_item->rectangle.y0, highlighted_item->rectangle.y1);
        }
        state->set_view_vertical_focus_to_item = false;
    }
    
    {
        i32_Rect item_rect = {};
        item_rect.x0 = x0;
        item_rect.y0 = 0;
        item_rect.x1 = x1;
        item_rect.y1 = item_rect.y0 + text_field_height;
        y_pos = item_rect.y1;
        
        UI_Item item = {};
        item.type = UIType_TextField;
        item.activation_level = UIActivation_Active;
        item.coordinates = UICoordinates_ViewRelative;
        item.text_field.query = state->lister.data.query;
        item.text_field.string = state->lister.data.text_field;
        item.user_data = 0;
        item.rectangle = item_rect;
        ui_list_add_item(scratch, &list, item);
    }
    
    UI_Control control = ui_list_to_ui_control(scratch, &list);
    view_set_ui(app, view, &control, lister_quit_function);
    
    end_temp_memory(full_temp);
}

static Lister_Prealloced_String
lister_prealloced(String string){
    Lister_Prealloced_String result = {};
    result.string = string;
    return(result);
}

static void
lister_first_init(Application_Links *app, Lister *lister, void *user_data, int32_t user_data_size, int32_t arena_size){
    memset(lister, 0, sizeof(*lister));
    lister->data.query      = make_fixed_width_string(lister->data.query_space);
    lister->data.text_field = make_fixed_width_string(lister->data.text_field_space);
    lister->data.key_string = make_fixed_width_string(lister->data.key_string_space);
    if (arena_size < user_data_size){
        arena_size = user_data_size;
    }
    lister->data.user_data_size = 0;
    if (arena_size != 0){
        lister_arena_clear_data_ensure_bytes(app, lister, arena_size);
        lister->data.user_data = push_array(&lister->arena, char, user_data_size);
        push_align(&lister->arena, 8);
        if (user_data != 0){
            memcpy(lister->data.user_data, user_data, user_data_size);
        }
        lister->data.user_data_size = user_data_size;
    }
}

static void
lister_begin_new_item_set(Application_Links *app, Lister *lister, int32_t list_memory_size){
    lister_arena_clear_data_ensure_bytes(app, lister, list_memory_size);
    memset(&lister->data.options, 0, sizeof(lister->data.options));
}

static void*
lister_add_item(Lister *lister, Lister_Prealloced_String string, Lister_Prealloced_String status,
                void *user_data, int32_t extra_space){
    Lister_Node *node = push_array(&lister->arena, Lister_Node, 1);
    node->string = string.string;
    node->status = status.string;
    node->user_data = user_data;
    node->raw_index = lister->data.options.count;
    zdll_push_back(lister->data.options.first, lister->data.options.last, node);
    lister->data.options.count += 1;
    void *result = push_array(&lister->arena, char, extra_space);
    push_align(&lister->arena, 8);
    return(result);
}

static void*
lister_add_item(Lister *lister, Lister_Prealloced_String string, String status,
                void *user_data, int32_t extra_space){
    return(lister_add_item(lister, string, lister_prealloced(string_push_copy(&lister->arena, status)),
                           user_data, extra_space));
}

static void*
lister_add_item(Lister *lister, String string, Lister_Prealloced_String status,
                void *user_data, int32_t extra_space){
    return(lister_add_item(lister, lister_prealloced(string_push_copy(&lister->arena, string)), status,
                           user_data, extra_space));
}

static void*
lister_add_item(Lister *lister, String string, String status, void *user_data, int32_t extra_space){
    return(lister_add_item(lister,
                           lister_prealloced(string_push_copy(&lister->arena, string)),
                           lister_prealloced(string_push_copy(&lister->arena, status)),
                           user_data, extra_space));
}

static void*
lister_add_theme_item(Lister *lister,
                      Lister_Prealloced_String string, int32_t index,
                      void *user_data, int32_t extra_space){
    Lister_Node *node = push_array(&lister->arena, Lister_Node, 1);
    node->string = string.string;
    node->index = index;
    node->user_data = user_data;
    node->raw_index = lister->data.options.count;
    zdll_push_back(lister->data.options.first, lister->data.options.last, node);
    lister->data.options.count += 1;
    void *result = push_array(&lister->arena, char, extra_space);
    push_align(&lister->arena, 8);
    return(result);
}

static void*
lister_add_theme_item(Lister *lister, String string, int32_t index,
                      void *user_data, int32_t extra_space){
    return(lister_add_theme_item(lister, lister_prealloced(string_push_copy(&lister->arena, string)), index,
                                 user_data, extra_space));
}

static void*
lister_get_user_data(Lister_Data *lister_data, int32_t index){
    if (0 <= index && index < lister_data->options.count){
        int32_t counter = 0;
        for (Lister_Node *node = lister_data->options.first;
             node != 0;
             node = node->next){
            if (counter == index){
                return(node->user_data);
            }
            counter += 1;
        }
    }
    return(0);
}

static void
lister_call_refresh_handler(Application_Links *app, Lister *lister){
    if (lister->data.handlers.refresh != 0){
        lister->data.handlers.refresh(app, lister);
    }
}

static void
lister_default(Application_Links *app, Partition *scratch, Heap *heap,
               View_Summary *view, Lister_State *state,
               Lister_Activation_Code code){
    switch (code){
        case ListerActivation_Finished:
        {
            view_end_ui_mode(app, view);
            state->initialized = false;
            if (state->lister.arena.base != 0){
                memory_free(app, state->lister.arena.base, state->lister.arena.max);
                memset(&state->lister.arena, 0, sizeof(state->lister.arena));
            }
        }break;
        
        case ListerActivation_Continue:
        {
            view_begin_ui_mode(app, view);
        }break;
        
        case ListerActivation_ContinueAndRefresh:
        {
            view_begin_ui_mode(app, view);
            state->item_index = 0;
            lister_call_refresh_handler(app, &state->lister);
            lister_update_ui(app, scratch, view, state);
        }break;
    }
}

static void
lister_call_activate_handler(Application_Links *app, Partition *scratch, Heap *heap,
                             View_Summary *view, Lister_State *state,
                             void *user_data, bool32 activated_by_mouse){
    Lister_Data *lister = &state->lister.data;
    if (lister->handlers.activate != 0){
        lister->handlers.activate(app, scratch, heap, view, state,
                                  lister->text_field, user_data, activated_by_mouse);
    }
    else{
        lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
    }
}

static void
lister_set_query_string(Lister_Data *lister, char *string){
    copy(&lister->query, string);
}

static void
lister_set_query_string(Lister_Data *lister, String string){
    copy(&lister->query, string);
}

static void
lister_set_text_field_string(Lister_Data *lister, char *string){
    copy(&lister->text_field, string);
}

static void
lister_set_text_field_string(Lister_Data *lister, String string){
    copy(&lister->text_field, string);
}

static void
lister_set_key_string(Lister_Data *lister, char *string){
    copy(&lister->key_string, string);
}

static void
lister_set_key_string(Lister_Data *lister, String string){
    copy(&lister->key_string, string);
}

// BOTTOM

