/*
 * Helpers for ui data structures.
 */

// TOP

static Vec2_f32
panel_space_from_screen_space(Vec2_f32 p, Vec2_f32 file_region_p0){
    return(p - file_region_p0);
}

static Vec2_f32
get_mouse_position_in_panel_space(Mouse_State mouse, Vec2_f32 file_region_p0){
    return(panel_space_from_screen_space(V2f32(mouse.p), file_region_p0));
}

static Vec2_f32
get_mouse_position_in_panel_space(Application_Links *app, Vec2_f32 file_region_p0){
    return(get_mouse_position_in_panel_space(get_mouse_state(app), file_region_p0));
}

////////////////////////////////

#if 0
internal void
default_ui_render_caller(Application_Links *app, View_ID view_id, Rect_f32 rect, Face_ID face_id){
    UI_Data *ui_data = 0;
    Arena *ui_arena = 0;
    if (view_get_ui_data(app, view_id, ViewGetUIFlag_KeepDataAsIs, &ui_data, &ui_arena)){
        Basic_Scroll scroll = view_get_basic_scroll(app, view_id);
        
        for (UI_Item *item = ui_data->list.first;
             item != 0;
             item = item->next){
            Rect_f32 item_rect = Rf32(item->rect_outer);
            item_rect.p0 += rect.p0;
            item_rect.p1 += rect.p0;
            
            switch (item->coordinates){
                case UICoordinates_ViewSpace:
                {
                    item_rect.p0 -= scroll.position;
                    item_rect.p1 -= scroll.position;
                }break;
                case UICoordinates_PanelSpace:
                {}break;
            }
            
            if (rect_overlap(item_rect, rect)){
                Rect_f32 inner = rect_inner(item_rect, (f32)item->inner_margin);
                
                Face_Metrics metrics = get_face_metrics(app, face_id);
                f32 line_height = metrics.line_height;
                f32 info_height = (f32)item->line_count*line_height;
                
                draw_rectangle(app, inner, 0.f, Stag_Back);
                Vec2_f32 p = V2f32(inner.x0 + 3.f, f32_round32((inner.y0 + inner.y1 - info_height)*0.5f));
                for (i32 i = 0; i < item->line_count; i += 1){
                    draw_fancy_string(app, face_id, item->lines[i].first, p, Stag_Default, 0, 0, V2(1.f, 0));
                    p.y += line_height;
                }
                if (item->inner_margin > 0){
                    draw_margin(app, item_rect, inner, get_margin_color(item->activation_level));
                }
            }
        }
    }
}
internal void
default_ui_render_caller(Application_Links *app, View_ID view, Rect_f32 rect){
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    Face_ID face_id = get_face_id(app, buffer);
    default_ui_render_caller(app, view, rect, face_id);
}
#endif

////////////////////////////////

Lister_State global_lister_state_[16] = {};
Lister_State *global_lister_state = global_lister_state_ - 1;

static Lister_State*
view_get_lister_state(View_ID view){
    return(&global_lister_state[view]);
}

function void
init_lister_state(Application_Links *app, Lister_State *state, Heap *heap){
    state->initialized = true;
    state->set_view_vertical_focus_to_item = false;
    state->highlighted_node = 0;
    state->hot_user_data = 0;
    state->item_index = 0;
    state->filter_restore_point_is_set = false;
    block_zero_struct(&state->scroll);
}

function f32
lister_get_text_field_height(f32 line_height){
    return(line_height);
}

function f32
lister_get_block_height(f32 line_height){
    return(line_height*2);
}

function Lister_Top_Level_Layout
lister_get_top_level_layout(Rect_f32 rect, f32 line_height){
    f32 text_field_height = lister_get_text_field_height(line_height);
    Lister_Top_Level_Layout layout = {};
    layout.text_field_rect = Rf32(rect.x0, rect.y0,
                                  rect.x1, clamp_top(rect.y0 + text_field_height, rect.y1));
    layout.list_rect = Rf32(rect.x0, layout.text_field_rect.y0, rect.x1, rect.y1);
    return(layout);
}

function void*
lister_user_data_at_p(Application_Links *app, View_ID view, Lister_State *state, Vec2_f32 p){
    Rect_f32 region = view_get_buffer_region(app, view);
    p -= region.p0;
    p += state->scroll.position;
    
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 block_height = lister_get_block_height(line_height);
    
    Lister_Top_Level_Layout layout = lister_get_top_level_layout(region, line_height);
    
    void *result = 0;
    if (rect_contains_point(layout.list_rect, p)){
        i32 index = (i32)((p.y - layout.list_rect.y0)/block_height);
        if (0 < index && index < state->filtered.count){
            Lister_Node *node = state->filtered.node_ptrs[index];
            result = node->user_data;
        }
    }
    
    return(result);
}

function Lister_Filtered
lister_get_filtered(Arena *arena, Lister_State *state){
    i32 node_count = state->lister.data.options.count;
    
    Lister_Filtered filtered = {};
    filtered.exact_matches.node_ptrs = push_array(arena, Lister_Node*, 1);
    filtered.before_extension_matches.node_ptrs = push_array(arena, Lister_Node*, node_count);
    filtered.substring_matches.node_ptrs = push_array(arena, Lister_Node*, node_count);
    
    Temp_Memory_Block temp(arena);
    
    String_Const_u8 key = state->lister.data.key_string.string;
    key = push_string_copy(arena, key);
    string_mod_replace_character(key, '_', '*');
    string_mod_replace_character(key, ' ', '*');
    
    List_String_Const_u8 absolutes = {};
    string_list_push(arena, &absolutes, string_u8_litexpr(""));
    List_String_Const_u8 splits = string_split(arena, key, (u8*)"*", 1);
    b32 has_wildcard = (splits.node_count > 1);
    string_list_push(&absolutes, &splits);
    string_list_push(arena, &absolutes, string_u8_litexpr(""));
    
    for (Lister_Node *node = state->lister.data.options.first;
         node != 0;
         node = node->next){
        String_Const_u8 node_string = node->string;
        if (key.size == 0 || string_wildcard_match_insensitive(absolutes, node_string)){
            if (string_match_insensitive(node_string, key) && filtered.exact_matches.count == 0){
                filtered.exact_matches.node_ptrs[filtered.exact_matches.count++] = node;
            }
            else if (!has_wildcard &&
                     string_match_insensitive(string_prefix(node_string, key.size), key) &&
                     node->string.size > key.size &&
                     node->string.str[key.size] == '.'){
                filtered.before_extension_matches.node_ptrs[filtered.before_extension_matches.count++] = node;
            }
            else{
                filtered.substring_matches.node_ptrs[filtered.substring_matches.count++] = node;
            }
        }
    }
    
    return(filtered);
}

static void
lister_update_filtered_list(Application_Links *app, View_ID view, Lister_State *state){
    Scratch_Block scratch(app, Scratch_Share);
    
    Lister_Filtered filtered = lister_get_filtered(scratch, state);
    
    Lister_Node_Ptr_Array node_ptr_arrays[] = {
        filtered.exact_matches,
        filtered.before_extension_matches,
        filtered.substring_matches,
    };
    
    Arena *arena = state->lister.arena;
    if (state->filter_restore_point_is_set){
        end_temp(state->filter_restore_point);
    }
    else{
        state->filter_restore_point = begin_temp(arena);
        state->filter_restore_point_is_set = true;
    }
    
    i32 total_count = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        total_count += node_ptr_array.count;
    }
    
    Lister_Node **node_ptrs = push_array(arena, Lister_Node*, total_count);
    state->filtered.node_ptrs = node_ptrs;
    state->filtered.count = total_count;
    
    state->raw_item_index = -1;
    state->highlighted_node = 0;
    
    i32 counter = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        for (i32 node_index = 0; node_index < node_ptr_array.count; node_index += 1){
            Lister_Node *node = node_ptr_array.node_ptrs[node_index];
            node_ptrs[counter] = node;
            if (state->item_index == counter){
                state->highlighted_node = node;
                state->raw_item_index = node->raw_index;
            }
            counter += 1;
        }
    }
}

function void
lister_render(Application_Links *app, View_ID view, Frame_Info frame_info, Rect_f32 inner){
    Scratch_Block scratch(app);
    
    Lister_State *state = view_get_lister_state(view);
    
    Basic_Scroll scroll = state->scroll;
    Rect_f32 region = view_get_buffer_region(app, view);
    Vec2_f32 view_m = get_mouse_position_in_panel_space(app, region.p0);
    view_m += scroll.position;
    
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 block_height = lister_get_block_height(line_height);
    f32 text_field_height = lister_get_text_field_height(line_height);
    
    Range_f32 x = rect_range_x(region);
    f32 y_pos = region.y0;
    
    {
        Fancy_String_List text_field = {};
        push_fancy_string(scratch, &text_field, fancy_id(Stag_Pop1),
                          state->lister.data.query.string);
        push_fancy_stringf(scratch, &text_field, fancy_id(Stag_Pop1), " ");
        push_fancy_string(scratch, &text_field, fancy_id(Stag_Default),
                          state->lister.data.text_field.string);
        draw_fancy_string(app, face_id, text_field.first, V2f32(x.min, y_pos),
                          Stag_Default, Stag_Back, 0, V2f32(1.f, 0.f));
        y_pos += text_field_height;
    }
    
    i32 count = state->filtered.count;
    for (i32 i = 0; i < count; i += 1){
        Lister_Node *node = state->filtered.node_ptrs[i];
        (void)node;
        
        Range_f32 y = If32(y_pos, y_pos + block_height);
        y_pos = y.max;
        
        Rect_f32 item_rect = Rf32(x, y);
        Rect_f32 item_inner = rect_inner(item_rect, 3.f);
        
        UI_Highlight_Level highlight = UIHighlight_None;
        draw_rectangle(app, item_rect, 3.f, get_margin_color(highlight));
        draw_rectangle(app, item_inner, 3.f, Stag_Back);
        
        
    }
    
#if 0
    i32 item_index_counter = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        for (i32 node_index = 0; node_index < node_ptr_array.count; node_index += 1){
            Lister_Node *node = node_ptr_array.node_ptrs[node_index];
            
            Rect_f32 item_rect = Rf32(x0, y_pos, x1, y_pos + block_height);
            y_pos = item_rect.y1;
            
            UI_Item item = {};
            item.activation_level = UIActivation_None;
            item.coordinates = UICoordinates_ViewSpace;
            item.rect_outer = Ri32(item_rect);
            item.inner_margin = 3;
            
            if (!is_theme_list){
                Fancy_String_List list = {};
                push_fancy_string (ui_arena, &list, fancy_id(Stag_Default), node->string);
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Default), " ");
                push_fancy_string (ui_arena, &list, fancy_id(Stag_Pop2   ), node->status);
                item.lines[0] = list;
                item.line_count = 1;
            }
            else{
                //i32 style_index = node->index;
                
                String_Const_u8 name = string_u8_litexpr("name");
                item.lines[0] = fancy_string_list_single(push_fancy_string(ui_arena, fancy_id(Stag_Default), name));
                
                Fancy_String_List list = {};
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Keyword     ), "if ");
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Default     ), "(x < ");
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Int_Constant), "0");
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Default     ), ") { x = ");
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Int_Constant), "0");
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Default     ), "; } ");
                push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Comment     ), "// comment");
                item.lines[1] = list;
                
                item.line_count = 2;
            }
            
            item.user_data = node->user_data;
            
            UI_Item *item_ptr = ui_list_add_item(ui_arena, &ui_data->list, item);
            if (rect_contains_point(item_rect, view_m)){
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
            view_set_vertical_focus_basic(app, view,
                                          (f32)highlighted_item->rect_outer.y0,
                                          (f32)highlighted_item->rect_outer.y1);
        }
        state->set_view_vertical_focus_to_item = false;
    }
    
    {
        // TODO(allen): switch to float
        Rect_i32 item_rect = {};
        item_rect.x0 = (i32)x0;
        item_rect.y0 = 0;
        item_rect.x1 = (i32)x1;
        item_rect.y1 = item_rect.y0 + (i32)text_field_height;
        y_pos = (f32)item_rect.y1;
        
        UI_Item item = {};
        item.activation_level = UIActivation_Active;
        item.coordinates = UICoordinates_PanelSpace;
        item.rect_outer = item_rect;
        item.inner_margin = 0;
        {
            Fancy_String_List list = {};
            push_fancy_string (ui_arena, &list, fancy_id(Stag_Pop1   ), state->lister.data.query.string);
            push_fancy_stringf(ui_arena, &list, fancy_id(Stag_Pop1   ), " ");
            push_fancy_string (ui_arena, &list, fancy_id(Stag_Default), state->lister.data.text_field.string);
            item.lines[0] = list;
            item.line_count = 1;
        }
        item.user_data = 0;
        
        
        ui_list_add_item(ui_arena, &ui_data->list, item);
    }
    
    ui_data_compute_bounding_boxes(ui_data);
#endif
    
}

static Lister_Prealloced_String
lister_prealloced(String_Const_u8 string){
    Lister_Prealloced_String result = {};
    result.string = string;
    return(result);
}

static void
lister_first_init(Application_Links *app, Lister *lister, void *user_data, i32 user_data_size){
	if (lister->arena == 0) {
		lister->arena = reserve_arena(app, KB(16));
	}
	else{
		linalloc_clear(lister->arena);
	}
	block_zero_struct(&lister->data);
    lister->data.query = Su8(lister->data.query_space, 0, sizeof(lister->data.query_space));
    lister->data.text_field = Su8(lister->data.text_field_space, 0, sizeof(lister->data.text_field_space));
    lister->data.key_string = Su8(lister->data.key_string_space, 0, sizeof(lister->data.key_string_space));
    lister->data.user_data = push_array(lister->arena, char, user_data_size);
    lister->data.user_data_size = user_data_size;
    if (user_data != 0){
        block_copy(lister->data.user_data, user_data, user_data_size);
    }
}

static void
lister_begin_new_item_set(Application_Links *app, Lister *lister){
    linalloc_clear(lister->arena);
    block_zero_struct(&lister->data.options);
}

static void*
lister_add_item(Lister *lister, Lister_Prealloced_String string, Lister_Prealloced_String status,
                void *user_data, umem extra_space){
    void *base_memory = push_array(lister->arena, u8, sizeof(Lister_Node) + extra_space);
    Lister_Node *node = (Lister_Node*)base_memory;
    node->string = string.string;
    node->status = status.string;
    node->user_data = user_data;
    node->raw_index = lister->data.options.count;
    zdll_push_back(lister->data.options.first, lister->data.options.last, node);
    lister->data.options.count += 1;
    void *result = (node + 1);
    return(result);
}

static void*
lister_add_item(Lister *lister, Lister_Prealloced_String string, String_Const_u8 status,
                void *user_data, umem  extra_space){
    return(lister_add_item(lister, string, lister_prealloced(push_string_copy(lister->arena, status)),
                           user_data, extra_space));
}

static void*
lister_add_item(Lister *lister, String_Const_u8 string, Lister_Prealloced_String status,
                void *user_data, umem extra_space){
    return(lister_add_item(lister, lister_prealloced(push_string_copy(lister->arena, string)), status,
                           user_data, extra_space));
}

static void*
lister_add_item(Lister *lister, String_Const_u8 string, String_Const_u8 status, void *user_data, umem extra_space){
    return(lister_add_item(lister,
                           lister_prealloced(push_string_copy(lister->arena, string)),
                           lister_prealloced(push_string_copy(lister->arena, status)),
                           user_data, extra_space));
}

static void*
lister_add_theme_item(Lister *lister,
                      Lister_Prealloced_String string, i32 index,
                      void *user_data, i32 extra_space){
    Lister_Node *node = push_array(lister->arena, Lister_Node, 1);
    node->string = string.string;
    node->index = index;
    node->user_data = user_data;
    node->raw_index = lister->data.options.count;
    zdll_push_back(lister->data.options.first, lister->data.options.last, node);
    lister->data.options.count += 1;
    void *result = push_array(lister->arena, char, extra_space);
    push_align(lister->arena, 8);
    return(result);
}

static void*
lister_add_theme_item(Lister *lister, String_Const_u8 string, i32 index,
                      void *user_data, i32 extra_space){
    return(lister_add_theme_item(lister, lister_prealloced(push_string_copy(lister->arena, string)), index,
                                 user_data, extra_space));
}

static void*
lister_get_user_data(Lister_Data *lister_data, i32 index){
    void *result = 0;
    if (0 <= index && index < lister_data->options.count){
        i32 counter = 0;
        for (Lister_Node *node = lister_data->options.first;
             node != 0;
             node = node->next, counter += 1){
            if (counter == index){
                result = node->user_data;
                break;
            }
        }
    }
    return(result);
}

static void
lister_call_refresh_handler(Application_Links *app, Lister *lister){
    if (lister->data.handlers.refresh != 0){
        lister->data.handlers.refresh(app, lister);
    }
}

static void
lister_default(Application_Links *app, Heap *heap, View_ID view, Lister_State *state, Lister_Activation_Code code){
    switch (code){
        case ListerActivation_Finished:
        {
            
        }break;
        
        case ListerActivation_Continue:
        {}break;
        
        case ListerActivation_ContinueAndRefresh:
        {
            state->item_index = 0;
            lister_call_refresh_handler(app, &state->lister);
            lister_update_filtered_list(app, view, state);
        }break;
    }
}

static Lister_Activation_Code
lister_call_activate_handler(Application_Links *app, Heap *heap, View_ID view, Lister_State *state, void *user_data, b32 activated_by_mouse){
    Lister_Activation_Code result = ListerActivation_Finished;
    Lister_Data *lister = &state->lister.data;
    if (lister->handlers.activate != 0){
        result = lister->handlers.activate(app, heap, view, state, lister->text_field.string, user_data, activated_by_mouse);
    }
    else{
        lister_default(app, heap, view, state, ListerActivation_Finished);
    }
    return(result);
}

static void
lister_set_string(String_Const_u8 string, String_u8 *target){
    target->size = 0;
    string_append(target, string);
}
static void
lister_append_string(String_Const_u8 string, String_u8 *target){
    string_append(target, string);
}

static void
lister_set_query(Lister_Data *lister, String_Const_u8 string){
    lister_set_string(string, &lister->query);
}
static void
lister_set_query(Lister_Data *lister, char *string){
    lister_set_string(SCu8(string), &lister->query);
}
static void
lister_set_text_field(Lister_Data *lister, String_Const_u8 string){
    lister_set_string(string, &lister->text_field);
}
static void
lister_set_text_field(Lister_Data *lister, char *string){
    lister_set_string(SCu8(string), &lister->text_field);
}
static void
lister_set_key(Lister_Data *lister, String_Const_u8 string){
    lister_set_string(string, &lister->key_string);
}
static void
lister_set_key(Lister_Data *lister, char *string){
    lister_set_string(SCu8(string), &lister->key_string);
}

static void
lister_set_query(Lister *lister, String_Const_u8 string){
    lister_set_query(&lister->data, string);
}
static void
lister_set_query(Lister *lister, char *string){
    lister_set_query(&lister->data, string);
}
static void
lister_set_text_field(Lister *lister, String_Const_u8 string){
    lister_set_text_field(&lister->data, string);
}
static void
lister_set_text_field(Lister *lister, char *string){
    lister_set_text_field(&lister->data, string);
}
static void
lister_set_key(Lister *lister, String_Const_u8 string){
    lister_set_key(&lister->data, string);
}
static void
lister_set_key(Lister *lister, char *string){
    lister_set_key(&lister->data, string);
}

static void
lister_append_query(Lister_Data *lister, String_Const_u8 string){
    lister_append_string(string, &lister->query);
}
static void
lister_append_query(Lister_Data *lister, char *string){
    lister_append_string(SCu8(string), &lister->query);
}
static void
lister_append_text_field(Lister_Data *lister, String_Const_u8 string){
    lister_append_string(string, &lister->text_field);
}
static void
lister_append_text_field(Lister_Data *lister, char *string){
    lister_append_string(SCu8(string), &lister->text_field);
}
static void
lister_append_key(Lister_Data *lister, String_Const_u8 string){
    lister_append_string(string, &lister->key_string);
}
static void
lister_append_key(Lister_Data *lister, char *string){
    lister_append_string(SCu8(string), &lister->key_string);
}

static void
lister_append_query(Lister *lister, String_Const_u8 string){
    lister_append_query(&lister->data, string);
}
static void
lister_append_query(Lister *lister, char *string){
    lister_append_query(&lister->data, string);
}
static void
lister_append_text_field(Lister *lister, String_Const_u8 string){
    lister_append_text_field(&lister->data, string);
}
static void
lister_append_text_field(Lister *lister, char *string){
    lister_append_text_field(&lister->data, string);
}
static void
lister_append_key(Lister *lister, String_Const_u8 string){
    lister_append_key(&lister->data, string);
}
static void
lister_append_key(Lister *lister, char *string){
    lister_append_key(&lister->data, string);
}

// BOTTOM

