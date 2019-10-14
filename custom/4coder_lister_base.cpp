/*
 * Helpers for ui data structures.
 */

// TOP

function Vec2_f32
panel_space_from_screen_space(Vec2_f32 p, Vec2_f32 file_region_p0){
    return(p - file_region_p0);
}

function Vec2_f32
get_mouse_position_in_panel_space(Mouse_State mouse, Vec2_f32 file_region_p0){
    return(panel_space_from_screen_space(V2f32(mouse.p), file_region_p0));
}

function Vec2_f32
get_mouse_position_in_panel_space(Application_Links *app, Vec2_f32 file_region_p0){
    return(get_mouse_position_in_panel_space(get_mouse_state(app), file_region_p0));
}

////////////////////////////////

function f32
lister_get_text_field_height(f32 line_height){
    return(line_height);
}

function f32
lister_get_block_height(f32 line_height){
    return(line_height*2);
}

function Lister_Top_Level_Layout
lister_get_top_level_layout(Rect_f32 rect, f32 text_field_height){
    Lister_Top_Level_Layout layout = {};
    layout.text_field_rect = Rf32(rect.x0, rect.y0,
                                  rect.x1, clamp_top(rect.y0 + text_field_height, rect.y1));
    layout.list_rect = Rf32(rect.x0, layout.text_field_rect.y1, rect.x1, rect.y1);
    return(layout);
}

////////////////////////////////

Lister *global_lister_state[16] = {};

function Lister*
view_get_lister(View_ID view){
    return(global_lister_state[view - 1]);
}

function Lister*
begin_lister(Application_Links *app, Arena *arena, View_ID view, void *user_data, umem user_data_size){
    Lister *lister = push_array_zero(arena, Lister, 1);
    lister->arena = arena;
    lister->data.query = Su8(lister->data.query_space, 0, sizeof(lister->data.query_space));
    lister->data.text_field = Su8(lister->data.text_field_space, 0, sizeof(lister->data.text_field_space));
    lister->data.key_string = Su8(lister->data.key_string_space, 0, sizeof(lister->data.key_string_space));
    lister->data.user_data_size = user_data_size;
    if (user_data_size > 0){
        lister->data.user_data = push_array(lister->arena, u8, user_data_size);
        if (user_data != 0){
            block_copy(lister->data.user_data, user_data, user_data_size);
        }
    }
    global_lister_state[view - 1] = lister;
    lister->restore_all_point = begin_temp(lister->arena);
    return(lister);
}

function void
lister_render(Application_Links *app, View_ID view, Frame_Info frame_info, Rect_f32 inner){
    Scratch_Block scratch(app);
    
    Lister *lister = view_get_lister(view);
    if (lister == 0){
        return;
    }
    
    Basic_Scroll scroll = lister->data.scroll;
    (void)scroll;
    Rect_f32 region = view_get_screen_rect(app, view);
    // TODO(allen): eliminate this. bad bad bad bad :(
    region = rect_inner(region, 3.f);
    Mouse_State mouse = get_mouse_state(app);
    Vec2_f32 m_p = V2f32(mouse.p);
    
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 block_height = lister_get_block_height(line_height);
    f32 text_field_height = lister_get_text_field_height(line_height);
    
    Lister_Top_Level_Layout layout = lister_get_top_level_layout(region, text_field_height);
    
    {
        Fancy_String_List text_field = {};
        push_fancy_string(scratch, &text_field, fancy_id(Stag_Pop1),
                          lister->data.query.string);
        push_fancy_stringf(scratch, &text_field, fancy_id(Stag_Pop1), " ");
        push_fancy_string(scratch, &text_field, fancy_id(Stag_Default),
                          lister->data.text_field.string);
        draw_fancy_string(app, face_id, text_field.first,
                          V2f32(layout.text_field_rect.x0 + 3.f, layout.text_field_rect.y0),
                          Stag_Default, Stag_Back, 0, V2f32(1.f, 0.f));
    }
    
    Range_f32 x = rect_range_x(layout.list_rect);
    f32 y_pos = layout.list_rect.y0;
    
    i32 count = lister->data.filtered.count;
    for (i32 i = 0; i < count; i += 1){
        Lister_Node *node = lister->data.filtered.node_ptrs[i];
        
        Range_f32 y = If32(y_pos, y_pos + block_height);
        y_pos = y.max;
        
        Rect_f32 item_rect = Rf32(x, y);
        Rect_f32 item_inner = rect_inner(item_rect, 3.f);
        
        b32 hovered = rect_contains_point(item_rect, m_p);
        UI_Highlight_Level highlight = UIHighlight_None;
        if (node == lister->data.highlighted_node){
            highlight = UIHighlight_Active;
        }
        else if (node->user_data == lister->data.hot_user_data){
            if (hovered){
                highlight = UIHighlight_Active;
            }
            else{
                highlight = UIHighlight_Hover;
            }
        }
        else if (hovered){
            highlight = UIHighlight_Hover;
        }
        
        draw_rectangle(app, item_rect, 3.f, get_margin_color(highlight));
        draw_rectangle(app, item_inner, 3.f, Stag_Back);
        
        Fancy_String_List line = {};
        push_fancy_string(scratch, &line, fancy_id(Stag_Default), node->string);
        push_fancy_stringf(scratch, &line, fancy_id(Stag_Default), " ");
        push_fancy_string(scratch, &line, fancy_id(Stag_Pop2), node->status);
        
        Vec2_f32 p = V2f32(item_inner.x0 + 3.f, item_inner.y0 + (block_height - line_height)*0.5f);
        draw_fancy_string(app, face_id, line.first, p, Stag_Default, 0, 0, V2(1.f, 0.f));
    }
}

function void*
lister_get_user_data(Lister *lister, i32 index){
    void *result = 0;
    if (0 <= index && index < lister->data.options.count){
        i32 counter = 0;
        for (Lister_Node *node = lister->data.options.first;
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

function Lister_Filtered
lister_get_filtered(Arena *arena, Lister *lister){
    i32 node_count = lister->data.options.count;
    
    Lister_Filtered filtered = {};
    filtered.exact_matches.node_ptrs = push_array(arena, Lister_Node*, 1);
    filtered.before_extension_matches.node_ptrs = push_array(arena, Lister_Node*, node_count);
    filtered.substring_matches.node_ptrs = push_array(arena, Lister_Node*, node_count);
    
    Temp_Memory_Block temp(arena);
    
    String_Const_u8 key = lister->data.key_string.string;
    key = push_string_copy(arena, key);
    string_mod_replace_character(key, '_', '*');
    string_mod_replace_character(key, ' ', '*');
    
    List_String_Const_u8 absolutes = {};
    string_list_push(arena, &absolutes, string_u8_litexpr(""));
    List_String_Const_u8 splits = string_split(arena, key, (u8*)"*", 1);
    b32 has_wildcard = (splits.node_count > 1);
    string_list_push(&absolutes, &splits);
    string_list_push(arena, &absolutes, string_u8_litexpr(""));
    
    for (Lister_Node *node = lister->data.options.first;
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

function void
lister_update_filtered_list(Application_Links *app, View_ID view, Lister *lister){
    Scratch_Block scratch(app, Scratch_Share);
    
    Lister_Filtered filtered = lister_get_filtered(scratch, lister);
    
    Lister_Node_Ptr_Array node_ptr_arrays[] = {
        filtered.exact_matches,
        filtered.before_extension_matches,
        filtered.substring_matches,
    };
    
    Arena *arena = lister->arena;
    end_temp(lister->data.filter_restore_point);
    
    i32 total_count = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        total_count += node_ptr_array.count;
    }
    
    Lister_Node **node_ptrs = push_array(arena, Lister_Node*, total_count);
    lister->data.filtered.node_ptrs = node_ptrs;
    lister->data.filtered.count = total_count;
    
    lister->data.raw_item_index = -1;
    lister->data.highlighted_node = 0;
    
    i32 counter = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        for (i32 node_index = 0; node_index < node_ptr_array.count; node_index += 1){
            Lister_Node *node = node_ptr_array.node_ptrs[node_index];
            node_ptrs[counter] = node;
            if (lister->data.item_index == counter){
                lister->data.highlighted_node = node;
                lister->data.raw_item_index = node->raw_index;
            }
            counter += 1;
        }
    }
}

function void
lister_call_refresh_handler(Application_Links *app, View_ID view, Lister *lister){
    if (lister->data.handlers.refresh != 0){
        lister->data.handlers.refresh(app, lister);
        lister->data.filter_restore_point = begin_temp(lister->arena);
        lister_update_filtered_list(app, view, lister);
    }
}

function void
lister_default(Application_Links *app, View_ID view, Lister *lister, Lister_Activation_Code code){
    switch (code){
        case ListerActivation_Finished:
        {}break;
        
        case ListerActivation_Continue:
        {}break;
        
        case ListerActivation_ContinueAndRefresh:
        {
            lister->data.item_index = 0;
            lister_call_refresh_handler(app, view, lister);
        }break;
    }
}

function Lister_Activation_Code
lister_call_activate_handler(Application_Links *app, View_ID view, Lister *lister,
                             void *user_data, b32 activated_by_mouse){
    Lister_Activation_Code result = ListerActivation_Finished;
    if (lister->data.handlers.activate != 0){
        result = lister->data.handlers.activate(app, view, lister, lister->data.text_field.string,
                                                user_data, activated_by_mouse);
    }
    else{
        lister_default(app, view, lister, ListerActivation_Finished);
    }
    return(result);
}

function void*
lister_user_data_at_p(Application_Links *app, View_ID view, Lister *lister, Vec2_f32 m_p){
    Rect_f32 region = view_get_screen_rect(app, view);
    // TODO(allen): eliminate this. bad bad bad bad :(
    region = rect_inner(region, 3.f);
    
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 block_height = lister_get_block_height(line_height);
    f32 text_field_height = lister_get_text_field_height(line_height);
    
    Lister_Top_Level_Layout layout = lister_get_top_level_layout(region, text_field_height);
    
    void *result = 0;
    if (rect_contains_point(layout.list_rect, m_p)){
        f32 y = m_p.y - layout.list_rect.y0 + lister->data.scroll.position.y;
        i32 index = (i32)(y/block_height);
        if (0 < index && index < lister->data.filtered.count){
            Lister_Node *node = lister->data.filtered.node_ptrs[index];
            result = node->user_data;
        }
    }
    
    return(result);
}

function void
lister_run(Application_Links *app, View_ID view, Lister *lister){
    lister->data.filter_restore_point = begin_temp(lister->arena);
    lister_update_filtered_list(app, view, lister);
    
    Managed_Scope scope = view_get_managed_scope(app, view);
    View_Render_Hook **hook = scope_attachment(app, scope, view_render_hook, View_Render_Hook*);
    *hook = lister_render;
    
    for (;;){
        User_Input in = get_user_input(app, EventPropertyGroup_Any, EventProperty_Escape);
        if (in.abort){
            break;
        }
        
        Lister_Activation_Code result = ListerActivation_Continue;
        b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_TextInsert:
            {
                if (lister->data.handlers.write_character != 0){
                    lister->data.handlers.write_character(app);
                }
            }break;
            
            case InputEventKind_KeyStroke:
            {
                switch (in.event.key.code){
                    case KeyCode_Return:
                    case KeyCode_Tab:
                    {
                        void *user_data = 0;
                        if (0 <= lister->data.raw_item_index &&
                            lister->data.raw_item_index < lister->data.options.count){
                            user_data = lister_get_user_data(lister, lister->data.raw_item_index);
                        }
                        result = lister_call_activate_handler(app, view, lister,
                                                              user_data, false);
                    }break;
                    
                    case KeyCode_Backspace:
                    {
                        if (lister->data.handlers.backspace != 0){
                            lister->data.handlers.backspace(app);
                        }
                        else if (lister->data.handlers.key_stroke != 0){
                            result = lister->data.handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_Up:
                    {
                        if (lister->data.handlers.navigate_up != 0){
                            lister->data.handlers.navigate_up(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_Down:
                    {
                        if (lister->data.handlers.navigate_down != 0){
                            lister->data.handlers.navigate_down(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    default:
                    {
                        if (lister->data.handlers.key_stroke != 0){
                            result = lister->data.handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                }
            }break;
            
            case InputEventKind_MouseButton:
            {
                switch (in.event.mouse.code){
                    case MouseCode_Left:
                    {
                        Vec2_f32 p = V2f32(in.event.mouse.p);
                        void *clicked = lister_user_data_at_p(app, view, lister, p);
                        lister->data.hot_user_data = clicked;
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            case InputEventKind_MouseButtonRelease:
            {
                switch (in.event.mouse.code){
                    case MouseCode_Left:
                    {
                        if (lister->data.hot_user_data != 0){
                            Vec2_f32 p = V2f32(in.event.mouse.p);
                            void *clicked = lister_user_data_at_p(app, view, lister, p);
                            if (lister->data.hot_user_data == clicked){
                                result = lister_call_activate_handler(app, view, lister,
                                                                      clicked, true);
                            }
                        }
                        lister->data.hot_user_data = 0;
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            case InputEventKind_MouseWheel:
            {
                Mouse_State mouse = get_mouse_state(app);
                lister->data.scroll.target.y += mouse.wheel;
                lister_update_filtered_list(app, view, lister);
            }break;
            
            case InputEventKind_MouseMove:
            case InputEventKind_Core:
            {
                lister_update_filtered_list(app, view, lister);
            }break;
            
            default:
            {
                handled = false;
            }break;
        }
        
        if (result == ListerActivation_Finished){
            break;
        }
        if (!handled){
            leave_command_input_unhandled(app);
        }
    }
    
    hook = scope_attachment(app, scope, view_render_hook, View_Render_Hook*);
    *hook = 0;
}

function Lister_Prealloced_String
lister_prealloced(String_Const_u8 string){
    Lister_Prealloced_String result = {};
    result.string = string;
    return(result);
}

function void
lister_begin_new_item_set(Application_Links *app, Lister *lister){
    end_temp(lister->restore_all_point);
    block_zero_struct(&lister->data.options);
    block_zero_struct(&lister->data.filtered);
}

function void*
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

function void*
lister_add_item(Lister *lister, Lister_Prealloced_String string, String_Const_u8 status,
                void *user_data, umem  extra_space){
    return(lister_add_item(lister, string, lister_prealloced(push_string_copy(lister->arena, status)),
                           user_data, extra_space));
}

function void*
lister_add_item(Lister *lister, String_Const_u8 string, Lister_Prealloced_String status,
                void *user_data, umem extra_space){
    return(lister_add_item(lister, lister_prealloced(push_string_copy(lister->arena, string)), status,
                           user_data, extra_space));
}

function void*
lister_add_item(Lister *lister, String_Const_u8 string, String_Const_u8 status, void *user_data, umem extra_space){
    return(lister_add_item(lister,
                           lister_prealloced(push_string_copy(lister->arena, string)),
                           lister_prealloced(push_string_copy(lister->arena, status)),
                           user_data, extra_space));
}

function void*
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

function void*
lister_add_theme_item(Lister *lister, String_Const_u8 string, i32 index,
                      void *user_data, i32 extra_space){
    return(lister_add_theme_item(lister, lister_prealloced(push_string_copy(lister->arena, string)), index,
                                 user_data, extra_space));
}

function void
lister_set_string(String_Const_u8 string, String_u8 *target){
    target->size = 0;
    string_append(target, string);
}
function void
lister_append_string(String_Const_u8 string, String_u8 *target){
    string_append(target, string);
}

function void
lister_set_query(Lister *lister, String_Const_u8 string){
    lister_set_string(string, &lister->data.query);
}
function void
lister_set_query(Lister *lister, char *string){
    lister_set_string(SCu8(string), &lister->data.query);
}
function void
lister_set_text_field(Lister *lister, String_Const_u8 string){
    lister_set_string(string, &lister->data.text_field);
}
function void
lister_set_text_field(Lister *lister, char *string){
    lister_set_string(SCu8(string), &lister->data.text_field);
}
function void
lister_set_key(Lister *lister, String_Const_u8 string){
    lister_set_string(string, &lister->data.key_string);
}
function void
lister_set_key(Lister *lister, char *string){
    lister_set_string(SCu8(string), &lister->data.key_string);
}

function void
lister_append_query(Lister *lister, String_Const_u8 string){
    lister_append_string(string, &lister->data.query);
}
function void
lister_append_query(Lister *lister, char *string){
    lister_append_string(SCu8(string), &lister->data.query);
}
function void
lister_append_text_field(Lister *lister, String_Const_u8 string){
    lister_append_string(string, &lister->data.text_field);
}
function void
lister_append_text_field(Lister *lister, char *string){
    lister_append_string(SCu8(string), &lister->data.text_field);
}
function void
lister_append_key(Lister *lister, String_Const_u8 string){
    lister_append_string(string, &lister->data.key_string);
}
function void
lister_append_key(Lister *lister, char *string){
    lister_append_string(SCu8(string), &lister->data.key_string);
}

// BOTTOM

