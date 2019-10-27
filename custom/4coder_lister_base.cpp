/*
 * Lister base
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

function Rect_f32_Pair
lister_get_top_level_layout(Rect_f32 rect, f32 text_field_height){
    return(rect_split_top_bottom(rect, text_field_height));
}

////////////////////////////////

Lister *global_lister_state[16] = {};

function Lister*
view_get_lister(View_ID view){
    return(global_lister_state[view - 1]);
}

function void
lister_set_map(Lister *lister, Mapping *mapping, Command_Map *map){
    lister->mapping = mapping;
    lister->map = map;
}

function void
lister_set_map(Lister *lister, Mapping *mapping, Command_Map_ID map){
    lister->mapping = mapping;
    lister->map = mapping_get_map(mapping, map);
}

function Lister*
begin_lister(Application_Links *app, Arena *arena){
    Lister *lister = push_array_zero(arena, Lister, 1);
    lister->arena = arena;
    lister->query = Su8(lister->query_space, 0, sizeof(lister->query_space));
    lister->text_field = Su8(lister->text_field_space, 0, sizeof(lister->text_field_space));
    lister->key_string = Su8(lister->key_string_space, 0, sizeof(lister->key_string_space));
    View_ID view = get_this_ctx_view(app, Access_Always);
    global_lister_state[view - 1] = lister;
    lister->restore_all_point = begin_temp(lister->arena);
    View_Context ctx = view_current_context(app, view);
    lister_set_map(lister, ctx.mapping, ctx.map_id);
    return(lister);
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
    lister_set_string(string, &lister->query);
}
function void
lister_set_query(Lister *lister, char *string){
    lister_set_string(SCu8(string), &lister->query);
}
function void
lister_set_text_field(Lister *lister, String_Const_u8 string){
    lister_set_string(string, &lister->text_field);
}
function void
lister_set_text_field(Lister *lister, char *string){
    lister_set_string(SCu8(string), &lister->text_field);
}
function void
lister_set_key(Lister *lister, String_Const_u8 string){
    lister_set_string(string, &lister->key_string);
}
function void
lister_set_key(Lister *lister, char *string){
    lister_set_string(SCu8(string), &lister->key_string);
}

function void
lister_append_query(Lister *lister, String_Const_u8 string){
    lister_append_string(string, &lister->query);
}
function void
lister_append_query(Lister *lister, char *string){
    lister_append_string(SCu8(string), &lister->query);
}
function void
lister_append_text_field(Lister *lister, String_Const_u8 string){
    lister_append_string(string, &lister->text_field);
}
function void
lister_append_text_field(Lister *lister, char *string){
    lister_append_string(SCu8(string), &lister->text_field);
}
function void
lister_append_key(Lister *lister, String_Const_u8 string){
    lister_append_string(string, &lister->key_string);
}
function void
lister_append_key(Lister *lister, char *string){
    lister_append_string(SCu8(string), &lister->key_string);
}

function void
lister_zero_scroll(Lister *lister){
    block_zero_struct(&lister->scroll);
}

function void
lister_render(Application_Links *app, Frame_Info frame_info, View_ID view){
    Scratch_Block scratch(app);
    
    Lister *lister = view_get_lister(view);
    if (lister == 0){
        return;
    }
    
    Rect_f32 region = draw_background_and_margin(app, view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 block_height = lister_get_block_height(line_height);
    f32 text_field_height = lister_get_text_field_height(line_height);
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view, ViewSetting_ShowFileBar, &showing_file_bar) &&
        showing_file_bar &&
        !global_config.hide_file_bar_in_ui){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
        draw_file_bar(app, view, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Mouse_State mouse = get_mouse_state(app);
    Vec2_f32 m_p = V2f32(mouse.p);
    
    lister->visible_count = (i32)((rect_height(region)/block_height)) - 3;
    lister->visible_count = clamp_bot(1, lister->visible_count);
    
    Rect_f32 text_field_rect = {};
    Rect_f32 list_rect = {};
    {
        Rect_f32_Pair pair = lister_get_top_level_layout(region, text_field_height);
        text_field_rect = pair.min;
        list_rect = pair.max;
    }
    
    {
        Fancy_Line text_field = {};
        push_fancy_string(scratch, &text_field, fcolor_id(Stag_Pop1),
                          lister->query.string);
        push_fancy_stringf(scratch, &text_field, " ");
        push_fancy_string(scratch, &text_field, fcolor_id(Stag_Default),
                          lister->text_field.string);
        draw_fancy_line(app, face_id, fcolor_zero(), &text_field,
                        V2f32(text_field_rect.x0 + 3.f, text_field_rect.y0));
    }
    
    
    Range_f32 x = rect_range_x(list_rect);
    draw_set_clip(app, list_rect);
    
    // NOTE(allen): auto scroll to the item if the flag is set.
    f32 scroll_y = lister->scroll.position.y;
    
    if (lister->set_vertical_focus_to_item){
        lister->set_vertical_focus_to_item = false;
        Range_f32 item_y = If32_size(lister->item_index*block_height, block_height);
        f32 view_h = rect_height(list_rect);
        Range_f32 view_y = If32_size(scroll_y, view_h);
        if (view_y.min > item_y.min || item_y.max > view_y.max){
            f32 item_center = (item_y.min + item_y.max)*0.5f;
            f32 view_center = (view_y.min + view_y.max)*0.5f;
            f32 margin = view_h*.3f;
            margin = clamp_top(margin, block_height*3.f);
            if (item_center < view_center){
                lister->scroll.target.y = item_y.min - margin;
            }
            else{
                f32 target_bot = item_y.max + margin;
                lister->scroll.target.y = target_bot - view_h;
            }
        }
    }
    
    // NOTE(allen): clamp scroll target and position; smooth scroll rule
    i32 count = lister->filtered.count;
    Range_f32 scroll_range = If32(0.f, clamp_bot(0.f, count*block_height - block_height));
    lister->scroll.target.y = clamp_range(scroll_range, lister->scroll.target.y);
    lister->scroll.target.x = 0.f;
    
    Vec2_f32_Delta_Result delta = delta_apply(app, view,
                                              frame_info.animation_dt, lister->scroll);
    lister->scroll.position = delta.p;
    if (delta.still_animating){
        animate_in_n_milliseconds(app, 0);
    }
    
    lister->scroll.position.y = clamp_range(scroll_range, lister->scroll.position.y);
    lister->scroll.position.x = 0.f;
    
    scroll_y = lister->scroll.position.y;
    f32 y_pos = list_rect.y0 - scroll_y;
    
    i32 first_index = (i32)(scroll_y/block_height);
    y_pos += first_index*block_height;
    
    for (i32 i = first_index; i < count; i += 1){
        Lister_Node *node = lister->filtered.node_ptrs[i];
        
        Range_f32 y = If32(y_pos, y_pos + block_height);
        y_pos = y.max;
        
        Rect_f32 item_rect = Rf32(x, y);
        Rect_f32 item_inner = rect_inner(item_rect, 3.f);
        
        b32 hovered = rect_contains_point(item_rect, m_p);
        UI_Highlight_Level highlight = UIHighlight_None;
        if (node == lister->highlighted_node){
            highlight = UIHighlight_Active;
        }
        else if (node->user_data == lister->hot_user_data){
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
        
        draw_rectangle(app, item_rect, 6.f, get_margin_color(highlight));
        draw_rectangle(app, item_inner, 6.f, fcolor_id(Stag_Back));
        
        Fancy_Line line = {};
        push_fancy_string(scratch, &line, fcolor_id(Stag_Default), node->string);
        push_fancy_stringf(scratch, &line, " ");
        push_fancy_string(scratch, &line, fcolor_id(Stag_Pop2), node->status);
        
        Vec2_f32 p = item_inner.p0 + V2f32(3.f, (block_height - line_height)*0.5f);
        draw_fancy_line(app, face_id, fcolor_zero(), &line, p);
    }
    
    draw_set_clip(app, prev_clip);
}

function void*
lister_get_user_data(Lister *lister, i32 index){
    void *result = 0;
    if (0 <= index && index < lister->options.count){
        i32 counter = 0;
        for (Lister_Node *node = lister->options.first;
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
    i32 node_count = lister->options.count;
    
    Lister_Filtered filtered = {};
    filtered.exact_matches.node_ptrs = push_array(arena, Lister_Node*, 1);
    filtered.before_extension_matches.node_ptrs = push_array(arena, Lister_Node*, node_count);
    filtered.substring_matches.node_ptrs = push_array(arena, Lister_Node*, node_count);
    
    Temp_Memory_Block temp(arena);
    
    String_Const_u8 key = lister->key_string.string;
    key = push_string_copy(arena, key);
    string_mod_replace_character(key, '_', '*');
    string_mod_replace_character(key, ' ', '*');
    
    List_String_Const_u8 absolutes = {};
    string_list_push(arena, &absolutes, string_u8_litexpr(""));
    List_String_Const_u8 splits = string_split(arena, key, (u8*)"*", 1);
    b32 has_wildcard = (splits.node_count > 1);
    string_list_push(&absolutes, &splits);
    string_list_push(arena, &absolutes, string_u8_litexpr(""));
    
    for (Lister_Node *node = lister->options.first;
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
lister_update_selection_values(Lister *lister){
    lister->raw_item_index = -1;
    lister->highlighted_node = 0;
    i32 count = lister->filtered.count;
    for (i32 i = 0; i < count; i += 1){
        Lister_Node *node = lister->filtered.node_ptrs[i];
        if (lister->item_index == i){
            lister->highlighted_node = node;
            lister->raw_item_index = node->raw_index;
        }
    }
}

function void
lister_update_filtered_list(Application_Links *app, Lister *lister){
    Scratch_Block scratch(app, Scratch_Share);
    
    Lister_Filtered filtered = lister_get_filtered(scratch, lister);
    
    Lister_Node_Ptr_Array node_ptr_arrays[] = {
        filtered.exact_matches,
        filtered.before_extension_matches,
        filtered.substring_matches,
    };
    
    Arena *arena = lister->arena;
    end_temp(lister->filter_restore_point);
    
    i32 total_count = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        total_count += node_ptr_array.count;
    }
    
    Lister_Node **node_ptrs = push_array(arena, Lister_Node*, total_count);
    lister->filtered.node_ptrs = node_ptrs;
    lister->filtered.count = total_count;
    i32 counter = 0;
    for (i32 array_index = 0; array_index < ArrayCount(node_ptr_arrays); array_index += 1){
        Lister_Node_Ptr_Array node_ptr_array = node_ptr_arrays[array_index];
        for (i32 node_index = 0; node_index < node_ptr_array.count; node_index += 1){
            Lister_Node *node = node_ptr_array.node_ptrs[node_index];
            node_ptrs[counter] = node;
            counter += 1;
        }
    }
    
    lister_update_selection_values(lister);
}

function void
lister_call_refresh_handler(Application_Links *app, Lister *lister){
    if (lister->handlers.refresh != 0){
        lister->handlers.refresh(app, lister);
        lister->filter_restore_point = begin_temp(lister->arena);
        lister_update_filtered_list(app, lister);
    }
}

function void
lister_activate(Application_Links *app, Lister *lister, void *user_data, b32 mouse){
    lister->out.activated_by_click = mouse;
    lister->out.text_field = lister->text_field.string;
    lister->out.user_data = user_data;
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
    
    b64 showing_file_bar = false;
    if (view_get_setting(app, view, ViewSetting_ShowFileBar, &showing_file_bar) &&
        showing_file_bar &&
        !global_config.hide_file_bar_in_ui){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        region = pair.max;
    }
    
    Rect_f32_Pair pair = lister_get_top_level_layout(region, text_field_height);
    Rect_f32 list_rect = pair.max;
    
    void *result = 0;
    if (rect_contains_point(list_rect, m_p)){
        f32 y = m_p.y - list_rect.y0 + lister->scroll.position.y;
        i32 index = (i32)(y/block_height);
        if (0 <= index && index < lister->filtered.count){
            Lister_Node *node = lister->filtered.node_ptrs[index];
            result = node->user_data;
        }
    }
    
    return(result);
}

function Lister_Result
run_lister(Application_Links *app, Lister *lister){
    lister->filter_restore_point = begin_temp(lister->arena);
    lister_update_filtered_list(app, lister);
    
    View_ID view = get_this_ctx_view(app, Access_Always);;
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = lister_render;
    ctx.hides_buffer = true;
    view_push_context(app, view, &ctx);
    
    for (;;){
        User_Input in = get_next_input(app, EventPropertyGroup_Any,
                                       EventProperty_Escape);
        if (in.abort){
            block_zero_struct(&lister->out);
            lister->out.canceled = true;
            break;
        }
        
        Lister_Activation_Code result = ListerActivation_Continue;
        b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_TextInsert:
            {
                if (lister->handlers.write_character != 0){
                    lister->handlers.write_character(app);
                }
            }break;
            
            case InputEventKind_KeyStroke:
            {
                switch (in.event.key.code){
                    case KeyCode_Return:
                    case KeyCode_Tab:
                    {
                        void *user_data = 0;
                        if (0 <= lister->raw_item_index &&
                            lister->raw_item_index < lister->options.count){
                            user_data = lister_get_user_data(lister, lister->raw_item_index);
                        }
                        lister_activate(app, lister, user_data, false);
                        result = ListerActivation_Finished;
                    }break;
                    
                    case KeyCode_Backspace:
                    {
                        if (lister->handlers.backspace != 0){
                            lister->handlers.backspace(app);
                        }
                        else if (lister->handlers.key_stroke != 0){
                            result = lister->handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_Up:
                    {
                        if (lister->handlers.navigate != 0){
                            lister->handlers.navigate(app, view, lister, -1);
                        }
                        else if (lister->handlers.key_stroke != 0){
                            result = lister->handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_Down:
                    {
                        if (lister->handlers.navigate != 0){
                            lister->handlers.navigate(app, view, lister, 1);
                        }
                        else if (lister->handlers.key_stroke != 0){
                            result = lister->handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_PageUp:
                    {
                        if (lister->handlers.navigate != 0){
                            lister->handlers.navigate(app, view, lister,
                                                      -lister->visible_count);
                        }
                        else if (lister->handlers.key_stroke != 0){
                            result = lister->handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_PageDown:
                    {
                        if (lister->handlers.navigate != 0){
                            lister->handlers.navigate(app, view, lister,
                                                      lister->visible_count);
                        }
                        else if (lister->handlers.key_stroke != 0){
                            result = lister->handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    default:
                    {
                        if (lister->handlers.key_stroke != 0){
                            result = lister->handlers.key_stroke(app);
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
                        lister->hot_user_data = clicked;
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
                        if (lister->hot_user_data != 0){
                            Vec2_f32 p = V2f32(in.event.mouse.p);
                            void *clicked = lister_user_data_at_p(app, view, lister, p);
                            if (lister->hot_user_data == clicked){
                                lister_activate(app, lister, clicked, true);
                                result = ListerActivation_Finished;
                            }
                        }
                        lister->hot_user_data = 0;
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
                lister->scroll.target.y += mouse.wheel;
                lister_update_filtered_list(app, lister);
            }break;
            
            case InputEventKind_MouseMove:
            case InputEventKind_Core:
            {
                lister_update_filtered_list(app, lister);
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
            Mapping *mapping = lister->mapping;
            Command_Map *map = lister->map;
            
            Fallback_Dispatch_Result disp_result =
                fallback_command_dispatch(app, mapping, map, &in);
            if (disp_result.code == FallbackDispatch_DelayedUICall){
                call_after_ctx_shutdown(app, view, disp_result.func);
                break;
            }
            if (disp_result.code == FallbackDispatch_Unhandled){
                leave_current_input_unhandled(app);
            }
            else{
                lister_call_refresh_handler(app, lister);
            }
        }
    }
    
    view_pop_context(app, view);
    
    return(lister->out);
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
    block_zero_struct(&lister->options);
    block_zero_struct(&lister->filtered);
}

function void*
lister_add_item(Lister *lister, Lister_Prealloced_String string,
                Lister_Prealloced_String status, void *user_data, umem extra_space){
    void *base_memory = push_array(lister->arena, u8, sizeof(Lister_Node) + extra_space);
    Lister_Node *node = (Lister_Node*)base_memory;
    node->string = string.string;
    node->status = status.string;
    node->user_data = user_data;
    node->raw_index = lister->options.count;
    zdll_push_back(lister->options.first, lister->options.last, node);
    lister->options.count += 1;
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

function void
lister__write_string__default(Application_Links *app){
    View_ID view = get_active_view(app, Access_Always);
    Lister *lister = view_get_lister(view);
    if (lister != 0){
        User_Input in = get_current_input(app);
        String_Const_u8 string = to_writable(&in);
        if (string.str != 0 && string.size > 0){
            lister_append_text_field(lister, string);
            lister_append_key(lister, string);
            lister->item_index = 0;
            lister_zero_scroll(lister);
            lister_update_filtered_list(app, lister);
        }
    }
}

function void
lister__backspace_text_field__default(Application_Links *app){
    View_ID view = get_active_view(app, Access_Always);
    Lister *lister = view_get_lister(view);
    if (lister != 0){
        lister->text_field.string = backspace_utf8(lister->text_field.string);
        lister->key_string.string = backspace_utf8(lister->key_string.string);
        lister->item_index = 0;
        lister_zero_scroll(lister);
        lister_update_filtered_list(app, lister);
    }
}

function void
lister__navigate__default(Application_Links *app, View_ID view, Lister *lister,
                          i32 delta){
    i32 new_index = lister->item_index + delta;
    if (new_index < 0 && lister->item_index == 0){
        lister->item_index = lister->filtered.count - 1;
    }
    else if (new_index >= lister->filtered.count &&
             lister->item_index == lister->filtered.count - 1){
        lister->item_index = 0;
    }
    else{
        lister->item_index = clamp(0, new_index, lister->filtered.count - 1);
    }
    lister->set_vertical_focus_to_item = true;
    lister_update_selection_values(lister);
}

function Lister_Handlers
lister_get_default_handlers(void){
    Lister_Handlers handlers = {};
    handlers.write_character = lister__write_string__default;
    handlers.backspace       = lister__backspace_text_field__default;
    handlers.navigate        = lister__navigate__default;
    return(handlers);
}

////////////////////////////////

function Lister_Result
run_lister_with_refresh_handler(Application_Links *app, Arena *arena,
                                String_Const_u8 query, Lister_Handlers handlers){
    Lister_Result result = {};
    if (handlers.refresh != 0){
        Lister *lister = begin_lister(app, arena);
        lister_set_query(lister, query);
        lister->handlers = handlers;
        handlers.refresh(app, lister);
        result = run_lister(app, lister);
    }
    else{
#define M "ERROR: No refresh handler specified for lister (query_string = \"%.*s\")\n"
        String_Const_u8 str = push_u8_stringf(arena, M, string_expand(query));
#undef M
        print_message(app, str);
        result.canceled = true;
    }
    return(result);
}

function Lister_Result
run_lister_with_refresh_handler(Application_Links *app, String_Const_u8 query, 
                                Lister_Handlers handlers){
    Scratch_Block scratch(app);
    return(run_lister_with_refresh_handler(app, scratch, query, handlers));
}

function Lister_Result
run_lister_with_refresh_handler(Application_Links *app, Arena *arena,
                                char *query, Lister_Handlers handlers){
    return(run_lister_with_refresh_handler(app, arena, SCu8(query), handlers));
}

function Lister_Result
run_lister_with_refresh_handler(Application_Links *app,
                                char *query, Lister_Handlers handlers){
    return(run_lister_with_refresh_handler(app, SCu8(query), handlers));
}

////////////////////////////////

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              String_Const_u8 string, String_Const_u8 status,
              Key_Code code, u64 user_data){
    Lister_Choice *choice = push_array(arena, Lister_Choice, 1);
    sll_queue_push(list->first, list->last, choice);
    choice->string = string;
    choice->status = status;
    choice->key_code = code;
    choice->user_data = user_data;
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              char *string, String_Const_u8 status,
              Key_Code code, u64 user_data){
    lister_choice(arena, list, SCu8(string), status, code, (u64)PtrAsInt(user_data));
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              String_Const_u8 string, char *status,
              Key_Code code, u64 user_data){
    lister_choice(arena, list, string, SCu8(status), code, (u64)PtrAsInt(user_data));
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              char *string, char *status,
              Key_Code code, u64 user_data){
    lister_choice(arena, list, SCu8(string), SCu8(status), code,
                  (u64)PtrAsInt(user_data));
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              String_Const_u8 string, String_Const_u8 status,
              Key_Code code, void *user_data){
    lister_choice(arena, list, string, status, code, (u64)PtrAsInt(user_data));
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              char *string, String_Const_u8 status,
              Key_Code code, void *user_data){
    lister_choice(arena, list, string, status, code, (u64)PtrAsInt(user_data));
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              String_Const_u8 string, char *status,
              Key_Code code, void *user_data){
    lister_choice(arena, list, string, status, code, (u64)PtrAsInt(user_data));
}

function void
lister_choice(Arena *arena, Lister_Choice_List *list,
              char *string, char *status,
              Key_Code code, void *user_data){
    lister_choice(arena, list, string, status, code, (u64)PtrAsInt(user_data));
}

function Lister_Activation_Code
lister__key_stroke__choice_list(Application_Links *app){
    Lister_Activation_Code result = ListerActivation_Continue;
    View_ID view = get_active_view(app, Access_Always);
    Lister *lister = view_get_lister(view);
    if (lister != 0){
        User_Input in = get_current_input(app);
        if (in.event.kind == InputEventKind_KeyStroke){
            void *user_data = 0;
            b32 did_shortcut_key = false;
            for (Lister_Node *node = lister->options.first;
                 node != 0;
                 node = node->next){
                Key_Code *key_code = (Key_Code*)(node + 1);
                if (*key_code == in.event.key.code){
                    user_data = node->user_data;
                    did_shortcut_key = true;
                    break;
                }
            }
            if (did_shortcut_key){
                lister_activate(app, lister, user_data, false);
                result = ListerActivation_Finished;
            }
        }
    }
    return(result);
}

function Lister_Choice*
get_choice_from_user(Application_Links *app, String_Const_u8 query,
                     Lister_Choice_List list){
    Scratch_Block scratch(app);
    Lister *lister = begin_lister(app, scratch);
    for (Lister_Choice *choice = list.first;
         choice != 0;
         choice = choice->next){
        umem code_size = sizeof(choice->key_code);
        void *extra = lister_add_item(lister, choice->string, choice->status,
                                      choice, code_size);
        block_copy(extra, &choice->key_code, code_size);
    }
    lister_set_query(lister, query);
    Lister_Handlers handlers = {};
    handlers.navigate        = lister__navigate__default;
    handlers.key_stroke      = lister__key_stroke__choice_list;
    lister->handlers = handlers;
    lister->handlers.refresh = 0;
    
    Lister_Result l_result = run_lister(app, lister);
    Lister_Choice *result = 0;
    if (!l_result.canceled){
        result = (Lister_Choice*)l_result.user_data;
    }
    return(result);
}

function Lister_Choice*
get_choice_from_user(Application_Links *app, char *query, Lister_Choice_List list){
    return(get_choice_from_user(app, SCu8(query), list));
}

// BOTTOM

