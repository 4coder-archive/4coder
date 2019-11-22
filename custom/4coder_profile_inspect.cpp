/*
 * 4coder_profile_inspect.cpp - Built in self profiling UI.
 */

// TOP

function Profile_Slot*
profile_parse_get_slot(Arena *arena, Profile_Inspection *insp,
                       String_Const_u8 loc, String_Const_u8 name){
    Profile_Slot *result = 0;
    for (Profile_Slot *node = insp->first_slot;
         node != 0;
         node = node->next){
        if (string_match(node->location, loc) &&
            string_match(node->name, name)){
            result = node;
            break;
        }
    }
    if (result == 0){
        result = push_array_zero(arena, Profile_Slot, 1);
        sll_queue_push(insp->first_slot, insp->last_slot, result);
        insp->slot_count += 1;
        result->location = loc;
        result->name = name;
    }
    return(result);
}

function void
profile_parse_error(Arena *arena, Profile_Inspection *insp, String_Const_u8 message,
                    String_Const_u8 location){
    Profile_Error *error = push_array(arena, Profile_Error, 1);
    sll_queue_push(insp->first_error, insp->last_error, error);
    insp->error_count += 1;
    error->message = message;
    error->location = location;
}

function Profile_Record*
profile_parse_record(Arena *arena, Profile_Inspection *insp,
                     Profile_Node *parent, Profile_Record *record,
                     Range_u64 *total_time_range){
    for (;record != 0;){
        if (record->id <= parent->id){
            break;
        }
        
        Profile_ID id = record->id;
        Profile_Node *node = push_array(arena, Profile_Node, 1);
        sll_queue_push(parent->first_child, parent->last_child, node);
        parent->child_count += 1;
        node->parent = parent;
        node->thread = parent->thread;
        
        String_Const_u8 location = record->location;
        String_Const_u8 name = record->name;
        
        node->time.min = record->time;
        node->time.max = max_u64;
        node->id = id;
        node->first_child = 0;
        node->last_child = 0;
        node->child_count = 0;
        node->closed = false;
        
        record = profile_parse_record(arena, insp, node, record->next,
                                      total_time_range);
        
        b32 quit_loop = false;
        Profile_Slot *slot = 0;
        if (record == 0 || record->id < id){
            if (record == 0){
#define M "List ended before all nodes closed"
                profile_parse_error(arena, insp, string_u8_litexpr(M), location);
#undef M
            }
            else{
#define M "Node '%.*s' closed by parent ending (or higher priority sibling starting)"
                String_Const_u8 str = push_u8_stringf(arena, M, string_expand(name));
                profile_parse_error(arena, insp, str, location);
#undef M
                if (parent->id != 0){
                    quit_loop = true;
                }
            }
            slot = profile_parse_get_slot(arena, insp, location, name);
        }
        else if (record->id == id){
            slot = profile_parse_get_slot(arena, insp, location, name);
            node->time.max = record->time;
            node->closed = true;
            total_time_range->min = min(total_time_range->min, node->time.min);
            total_time_range->max = max(total_time_range->max, node->time.max);
            record = record->next;
        }
        else{
            // NOTE(allen): This would mean that record exists and it's id
            // is greater than id, but then the sub-call should not have returned!
            InvalidPath;
        }
        
        node->slot = slot;
        if (!slot->corrupted_time){
            if (node->closed){
                slot->total_time += range_size(node->time);
            }
            else{
                slot->corrupted_time = true;
            }
        }
        {
            Profile_Node_Ptr *node_ptr = push_array(arena, Profile_Node_Ptr, 1);
            sll_queue_push(slot->first_hit, slot->last_hit, node_ptr);
            slot->hit_count += 1;
            node_ptr->ptr = node;
            node->unique_counter = (u64)slot->hit_count;
        }
        
        if (quit_loop){
            break;
        }
    }
    return(record);
}

function Profile_Inspection
profile_parse(Arena *arena, Profile_Global_List *src){
    Mutex_Lock lock(src->mutex);
    
    Profile_Inspection result = {};
    
    result.thread_count = src->thread_count;
    result.threads = push_array_zero(arena, Profile_Inspection_Thread,
                                     result.thread_count);
    
    i32 counter = 0;
    Profile_Inspection_Thread *insp_thread = result.threads;
    for (Profile_Thread *node = src->first_thread;
         node != 0;
         node = node->next, counter += 1, insp_thread += 1){
        insp_thread->thread_id = node->thread_id;
        insp_thread->name = node->name;
        
        // NOTE(allen): This is the "negative infinity" range.
        // We will be "maxing" it against all the ranges durring the parse,
        // to get the root range.
        Range_u64 time_range = {max_u64, 0};
        insp_thread->root.thread = insp_thread;
        profile_parse_record(arena, &result, &insp_thread->root, node->first_record,
                             &time_range);
        insp_thread->root.time = time_range;
        insp_thread->root.closed = true;
        
        for (Profile_Node *prof_node = insp_thread->root.first_child;
             prof_node != 0;
             prof_node = prof_node->next){
            insp_thread->active_time += range_size(prof_node->time);
        }
    }
    
    return(result);
}

////////////////////////////////

struct Tab_State{
    Vec2_f32 p;
    Range_f32 tabs_y;
    Face_ID face_id;
    f32 x_half_padding;
    Vec2_f32 m_p;
};

function void
profile_draw_tab(Application_Links *app, Tab_State *state, Profile_Inspection *insp,
                 String_Const_u8 string, Profile_Inspection_Tab tab_id){
    Scratch_Block scratch(app);
    
    state->p.x += state->x_half_padding;
    
    Fancy_String *fstring = push_fancy_string(scratch, 0, string);
    
    b32 hover = false;
    f32 width = get_fancy_string_width(app, state->face_id, fstring);
    Rect_f32 box = Rf32(If32_size(state->p.x, width), state->tabs_y);
    if (rect_contains_point(box, state->m_p)){
        hover = true;
        insp->tab_id_hovered = tab_id;
    }
    
    FColor text = fcolor_id(Stag_Default);
    if (insp->tab_id == tab_id){
        text = fcolor_id(Stag_Pop2);
    }
    else if (hover){
        text = fcolor_id(Stag_Pop1);
    }
    
    Vec2_f32 np = draw_fancy_string(app, state->face_id, text, fstring, state->p);
    state->p = np;
    state->p.x += state->x_half_padding;
}

function void
profile_select_thread(Profile_Inspection *inspect, Profile_Inspection_Thread *thread){
    inspect->tab_id = ProfileInspectTab_Selection;
    inspect->selected_thread = thread;
    inspect->selected_slot = 0;
    inspect->selected_node = 0;
}

function void
profile_select_slot(Profile_Inspection *inspect, Profile_Slot *slot){
    inspect->tab_id = ProfileInspectTab_Selection;
    inspect->selected_thread = 0;
    inspect->selected_slot = slot;
    inspect->selected_node = 0;
}

function void
profile_select_node(Profile_Inspection *inspect, Profile_Node *node){
    inspect->tab_id = ProfileInspectTab_Selection;
    inspect->selected_thread = 0;
    inspect->selected_slot = 0;
    inspect->selected_node = node;
}

function String_Const_u8
profile_node_thread_name(Profile_Node *node){
    String_Const_u8 result = {};
    if (node->thread != 0){
        result = node->thread->name;
    }
    return(result);
}

function String_Const_u8
profile_node_name(Profile_Node *node){
    String_Const_u8 result = string_u8_litexpr("*root*");
    if (node->slot != 0){
        result = node->slot->name;
    }
    return(result);
}

function String_Const_u8
profile_node_location(Profile_Node *node){
    String_Const_u8 result = {};
    if (node->slot != 0){
        result = node->slot->location;
    }
    return(result);
}

function void
profile_qsort_nodes(Profile_Node **nodes, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot_index = one_past_last - 1;
        Profile_Node *pivot = nodes[pivot_index];
        u64 pivot_time = range_size(pivot->time);
        i32 j = first;
        for (i32 i = first; i < one_past_last; i += 1){
            Profile_Node *node = nodes[i];
            u64 node_time = range_size(node->time);
            if (node_time > pivot_time){
                Swap(Profile_Node*, nodes[i], nodes[j]);
                j += 1;
            }
        }
        Swap(Profile_Node*, nodes[pivot_index], nodes[j]);
        profile_qsort_nodes(nodes, first, j);
        profile_qsort_nodes(nodes, j + 1, one_past_last);
    }
}

function void
profile_draw_node(Application_Links *app, View_ID view, Face_ID face_id,
                  Profile_Node *node, Rect_f32 rect,
                  Profile_Inspection *insp, Vec2_f32 m_p){
    Range_f32 x = rect_range_x(rect);
    Range_f32 y = rect_range_y(rect);
    
    // TODO(allen): share this shit
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 normal_advance = metrics.normal_advance;
    f32 block_height = line_height*2.f;
    f32 x_padding = normal_advance*1.5f;
    f32 x_half_padding = x_padding*0.5f;
    
    FColor colors[] = {
        fcolor_id(Stag_Back_Cycle_1), fcolor_id(Stag_Back_Cycle_2),
        fcolor_id(Stag_Back_Cycle_3), fcolor_id(Stag_Back_Cycle_4),
    };
    
    Scratch_Block scratch(app);
    
    f32 x_pos = x.min + x_half_padding;
    f32 nav_bar_w = 0.f;
    Range_f32 nav_bar_y = {};
    nav_bar_y.min = y.min;
    
    String_Const_u8 thread_name = profile_node_thread_name(node);
    if (thread_name.size > 0){
        Fancy_String *fstr =
            push_fancy_string(scratch, 0, fcolor_id(Stag_Pop1), thread_name);
        Vec2_f32 p = V2f32(x_pos, y.min + 1.f);
        draw_fancy_string(app, face_id, fcolor_zero(), fstr, p);
        f32 w = get_fancy_string_width(app, face_id, fstr);
        nav_bar_w = max(nav_bar_w, w);
    }
    y.min += line_height + 2.f;
    
    String_Const_u8 name = profile_node_name(node);
    if (name.size > 0){
        Fancy_String *fstr =
            push_fancy_string(scratch, 0, fcolor_id(Stag_Default), name);
        Vec2_f32 p = V2f32(x_pos, y.min + 1.f);
        draw_fancy_string(app, face_id, fcolor_zero(), fstr, p);
        f32 w = get_fancy_string_width(app, face_id, fstr);
        nav_bar_w = max(nav_bar_w, w);
    }
    y.min += line_height + 2.f;
    
    nav_bar_y.max = y.min;
    
    x_pos += nav_bar_w + x_half_padding;
    if (node->parent != 0){
        Fancy_String *fstr = push_fancy_string(scratch, 0, fcolor_zero(),
                                               string_u8_litexpr("to parent"));
        f32 w = get_fancy_string_width(app, face_id, fstr) + x_padding;
        Range_f32 btn_x = If32_size(x_pos, w);
        Rect_f32 box = Rf32(btn_x, nav_bar_y);
        
        FColor color = fcolor_id(Stag_Default);
        if (rect_contains_point(box, m_p)){
            draw_rectangle(app, box, 0.f, fcolor_id(Stag_Margin));
            color = fcolor_id(Stag_Pop1);
            insp->hover_node = node->parent;
        }
        
        Vec2_f32 p = V2f32(box.x0 + x_half_padding,
                           (box.y0 + box.y1 - line_height)*0.5f);
        draw_fancy_string(app, face_id, color, fstr, p);
        
        x_pos = btn_x.max;
    }
    
    Range_u64 top_time = node->time;
    
    Rect_f32_Pair side_by_side = rect_split_left_right_lerp(Rf32(x, y), 0.5f);
    
    Rect_f32 time_slice_box = side_by_side.min;
    time_slice_box = rect_inner(time_slice_box, 3.f);
    draw_rectangle_outline(app, time_slice_box, 0.f, 3.f, f_white);
    time_slice_box = rect_inner(time_slice_box, 3.f);
    
    if (node->closed){
        draw_set_clip(app, time_slice_box);
        
        x = rect_range_x(time_slice_box);
        y = rect_range_y(time_slice_box);
        
        i32 cycle_counter = 0;
        i32 count = ArrayCount(colors);
        for (Profile_Node *child = node->first_child;
             child != 0;
             child = child->next){
            if (!child->closed){
                continue;
            }
            
            Range_u64 child_time = child->time;
            Range_f32 child_y = {};
            child_y.min = unlerp(top_time.min, child_time.min, top_time.max);
            child_y.max = unlerp(top_time.min, child_time.max, top_time.max);
            child_y.min = lerp(y.min, child_y.min, y.max);
            child_y.max = lerp(y.min, child_y.max, y.max);
            
            Rect_f32 box = Rf32(x, child_y);
            draw_rectangle(app, box, 0.f, colors[cycle_counter%count]);
            cycle_counter += 1;
            
            if (rect_contains_point(box, m_p)){
                insp->full_name_hovered = profile_node_name(child);
                insp->unique_counter_hovered = child->unique_counter;
                insp->location_jump_hovered = profile_node_location(child);
                insp->hover_node = child;
            }
            
            if (range_size(child_y) >= line_height){
                String_Const_u8 child_name = profile_node_name(child);
                Fancy_Line line = {};
                push_fancy_string(scratch, &line, fcolor_id(Stag_Pop1),
                                  child_name);
                push_fancy_stringf(scratch, &line, fcolor_id(Stag_Default),
                                   0.5f, 0.f, "#%4llu", child->unique_counter);
                
                Vec2_f32 p = V2f32(x.min + x_half_padding,
                                   child_y.min);
                draw_fancy_line(app, face_id, fcolor_zero(),
                                &line, p);
            }
        }
    }
    
    Rect_f32 info_box = side_by_side.max;
    
    {
        draw_set_clip(app, info_box);
        
        x = rect_range_x(info_box);
        
        x_pos = x.min + x_half_padding;
        f32 y_pos = info_box.y0;
        
        // NOTE(allen): duration
        {
            f32 duration = ((f32)range_size(node->time))/1000000.f;
            Fancy_Line list = {};
            push_fancy_stringf(scratch, &list, fcolor_id(Stag_Default),
                               "time: %11.9f", duration);
            draw_fancy_line(app, face_id, fcolor_zero(),
                            &list, V2f32(x_pos, y_pos + 1.f));
            y_pos += line_height + 2.f;
        }
        
        i32 child_count = node->child_count;
        Profile_Node **children_array = push_array(scratch, Profile_Node*, child_count);
        i32 counter = 0;
        for (Profile_Node *child = node->first_child;
             child != 0;
             child = child->next){
            children_array[counter] = child;
            counter += 1;
        }
        
        profile_qsort_nodes(children_array, 0, child_count);
        
        Profile_Node **child_ptr = children_array;
        for (i32 i = 0; i < child_count; i += 1, child_ptr += 1){
            Profile_Node *child = *child_ptr;
            y = If32_size(y_pos, block_height);
            
            f32 child_duration = ((f32)range_size(child->time))/1000000.f;
            
            String_Const_u8 child_name = profile_node_name(child);
            Fancy_Line line = {};
            push_fancy_string_trunc(scratch, &line, child_name, 20);
            push_fancy_stringf(scratch, &line, fcolor_id(Stag_Default), 0.5f, 0.f,
                               "#%4llu", child->unique_counter);
            push_fancy_stringf(scratch, &line, fcolor_id(Stag_Pop2),
                               0.5f, 0.f, "%6.4f", child_duration);
            
            Vec2_f32 p = V2f32(x.min + x_half_padding,
                               (y.min + y.max - line_height)*0.5f);
            draw_fancy_line(app, face_id, fcolor_id(Stag_Pop1), &line, p);
            
            Rect_f32 box = Rf32(x, y);
            FColor margin = fcolor_id(Stag_Margin);
            if (rect_contains_point(box, m_p)){
                insp->full_name_hovered = child_name;
                insp->unique_counter_hovered = child->unique_counter;
                insp->location_jump_hovered = profile_node_location(child);
                insp->hover_node = child;
                margin = fcolor_id(Stag_Margin_Hover);
            }
            draw_rectangle_outline(app, box, 6.f, 3.f, margin);
            
            y_pos = y.max;
            if (y_pos >= info_box.y1){
                break;
            }
        }
    }
}

function void
profile_memory_sort_by_count(Memory_Bucket **buckets, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot = one_past_last - 1;
        i32 pivot_key = buckets[pivot]->annotation.count;
        i32 j = first;
        for (i32 i = first; i < pivot; i += 1){
            i32 key = buckets[i]->annotation.count;
            if (key <= pivot_key){
                Swap(Memory_Bucket*, buckets[j], buckets[i]);
                j += 1;
            }
        }
        Swap(Memory_Bucket*, buckets[j], buckets[pivot]);
        profile_memory_sort_by_count(buckets, first, j);
        profile_memory_sort_by_count(buckets, j + 1, one_past_last);
    }
}

function void
profile_render(Application_Links *app, Frame_Info frame_info, View_ID view){
    Scratch_Block scratch(app);
    
    Rect_f32 region = draw_background_and_margin(app, view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Face_ID face_id = get_face_id(app, 0);
    // TODO(allen): share this shit
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 normal_advance = metrics.normal_advance;
    f32 block_height = line_height*2.f;
    f32 x_padding = normal_advance*1.5f;
    f32 x_half_padding = x_padding*0.5f;
    
    Mouse_State mouse = get_mouse_state(app);
    Vec2_f32 m_p = V2f32(mouse.p);
    
    Profile_Inspection *inspect = &global_profile_inspection;
    
    if (inspect->thread_count == 0){
        Fancy_String *fstr = push_fancy_string(scratch, 0, fcolor_id(Stag_Pop2),
                                               string_u8_litexpr("no profile data"));
        f32 width = get_fancy_string_width(app, face_id, fstr);
        Vec2_f32 view_center = (region.p0 + region.p1)*0.5f;
        Vec2_f32 half_dim = V2f32(width, line_height)*0.5f;
        Vec2_f32 p = view_center - half_dim;
        draw_fancy_string(app, face_id, fcolor_zero(), fstr, p);
    }
    else{
        Rect_f32_Pair tabs_body = rect_split_top_bottom(region, line_height + 2.f);
        Range_f32 tabs_y = rect_range_y(tabs_body.min);
        
        inspect->tab_id_hovered = ProfileInspectTab_None;
        block_zero_struct(&inspect->full_name_hovered);
        inspect->unique_counter_hovered = 0;
        block_zero_struct(&inspect->location_jump_hovered);
        inspect->hover_thread = 0;
        inspect->hover_slot = 0;
        inspect->hover_node = 0;
        
        // NOTE(allen): tabs
        {
            f32 y = (tabs_y.min + tabs_y.max - line_height)*0.5f;
            f32 x = region.x0;
            
            Tab_State tab_state = {};
            tab_state.p = V2f32(x, y);
            tab_state.tabs_y = tabs_y;
            tab_state.face_id = face_id;
            tab_state.x_half_padding = x_half_padding;
            tab_state.m_p = m_p;
            
            draw_rectangle(app, tabs_body.min, 0.f, fcolor_id(Stag_Margin_Hover));
            
            if (inspect->tab_id == ProfileInspectTab_None){
                inspect->tab_id = ProfileInspectTab_Threads;
            }
            
            profile_draw_tab(app, &tab_state, inspect,
                             string_u8_litexpr("threads"),
                             ProfileInspectTab_Threads);
            
            if (inspect->slot_count > 0){
                profile_draw_tab(app, &tab_state, inspect,
                                 string_u8_litexpr("blocks"),
                                 ProfileInspectTab_Blocks);
            }
            
            if (inspect->error_count > 0){
                profile_draw_tab(app, &tab_state, inspect,
                                 string_u8_litexpr("errors"),
                                 ProfileInspectTab_Errors);
            }
            
                profile_draw_tab(app, &tab_state, inspect,
                                 string_u8_litexpr("memory"),
                                 ProfileInspectTab_Memory);
            
            if (inspect->tab_id == ProfileInspectTab_Selection){
                String_Const_u8 string = {};
                if (inspect->selected_thread != 0){
                    String_Const_u8 name = inspect->selected_thread->name;
                    string = push_u8_stringf(scratch, "%.*s (%d)",
                                             string_expand(name),
                                             inspect->selected_thread->thread_id);
                }
                else if (inspect->selected_slot != 0){
                    String_Const_u8 name = inspect->selected_slot->name;
                    string = push_u8_stringf(scratch, "block %.*s",
                                             string_expand(name));
                }
                else if (inspect->selected_node != 0){
                    String_Const_u8 name = profile_node_name(inspect->selected_node);
                    string = push_u8_stringf(scratch, "node %.*s",
                                             string_expand(name));
                }
                else{
                    inspect->tab_id = ProfileInspectTab_Threads;
                }
                if (string.str != 0){
                    profile_draw_tab(app, &tab_state, inspect,
                                     string, ProfileInspectTab_Selection);
                }
            }
        }
        
        draw_set_clip(app, tabs_body.max);
        switch (inspect->tab_id){
            case ProfileInspectTab_Threads:
            {
                Range_f32 x = rect_range_x(tabs_body.max);
                f32 y_pos = tabs_body.max.y0;
                i32 count = inspect->thread_count;
                Profile_Inspection_Thread *thread = inspect->threads;
                for (i32 i = 0; i < count; i += 1, thread += 1){
                    Range_f32 y = If32_size(y_pos, block_height);
                    
                    Fancy_Line list = {};
                    push_fancy_stringf(scratch, &list, fcolor_id(Stag_Pop1),
                                       "%-20.*s (%6d) ",
                                       string_expand(thread->name),
                                       thread->thread_id);
                    
                    f32 active_time = ((f32)thread->active_time)/1000000.f;
                    push_fancy_stringf(scratch, &list, fcolor_id(Stag_Pop2),
                                       "active time %11.9f",
                                       active_time);
                    
                    Vec2_f32 p = V2f32(x.min + x_half_padding,
                                       (y.min + y.max - line_height)*0.5f);
                    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
                    
                    Rect_f32 box = Rf32(x, y);
                    FColor margin = fcolor_id(Stag_Margin);
                    if (rect_contains_point(box, m_p)){
                        inspect->hover_thread = thread;
                        margin = fcolor_id(Stag_Margin_Hover);
                    }
                    draw_rectangle_outline(app, box, 6.f, 3.f, margin);
                    
                    y_pos = y.max;
                    if (y_pos >= tabs_body.max.y1){
                        break;
                    }
                }
            }break;
            
            case ProfileInspectTab_Blocks:
            {
                Range_f32 x = rect_range_x(tabs_body.max);
                f32 y_pos = tabs_body.max.y0;
                for (Profile_Slot *node = inspect->first_slot;
                     node != 0;
                     node = node->next){
                    Range_f32 y = If32_size(y_pos, block_height);
                    
                    u32 name_width = 45;
                    b32 name_too_long = (node->name.size > name_width);
                    Fancy_Line list = {};
                    push_fancy_string_fixed(scratch, &list, fcolor_id(Stag_Pop1),
                                            node->name, name_width);
                    
                    if (node->corrupted_time){
                        push_fancy_string(scratch, &list, fcolor_id(Stag_Pop2),
                                          string_u8_litexpr("timing error "));
                    }
                    else{
                        push_fancy_stringf(scratch, &list, fcolor_id(Stag_Pop2),
                                           "%11.9fs ",
                                           ((f32)node->total_time)/1000000.f);
                    }
                    
                    push_fancy_stringf(scratch, &list, fcolor_id(Stag_Keyword),
                                       "hit # %5d", node->hit_count);
                    
                    Vec2_f32 p = V2f32(x.min + x_half_padding,
                                       (y.min + y.max - line_height)*0.5f);
                    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
                    
                    Rect_f32 box = Rf32(x, y);
                    FColor margin = fcolor_id(Stag_Margin);
                    if (rect_contains_point(box, m_p)){
                        if (name_too_long){
                            inspect->full_name_hovered = node->name;
                        }
                        inspect->location_jump_hovered = node->location;
                        inspect->hover_slot = node;
                        margin = fcolor_id(Stag_Margin_Hover);
                    }
                    draw_rectangle_outline(app, box, 6.f, 3.f, margin);
                    
                    y_pos = y.max;
                    if (y_pos >= tabs_body.max.y1){
                        break;
                    }
                }
            }break;
            
            case ProfileInspectTab_Errors:
            {
                draw_set_clip(app, tabs_body.max);
                Range_f32 x = rect_range_x(tabs_body.max);
                f32 y_pos = tabs_body.max.y0;
                for (Profile_Error *node = inspect->first_error;
                     node != 0;
                     node = node->next){
                    Range_f32 y = If32_size(y_pos, block_height);
                    
                    Fancy_Line list = {};
                    push_fancy_string(scratch, &list, fcolor_id(Stag_Pop2),
                                      node->message);
                    
                    Vec2_f32 p = V2f32(x.min + x_half_padding,
                                       (y.min + y.max - line_height)*0.5f);
                    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
                    
                    Rect_f32 box = Rf32(x, y);
                    FColor margin = fcolor_id(Stag_Margin);
                    if (rect_contains_point(box, m_p)){
                        inspect->location_jump_hovered = node->location;
                        margin = fcolor_id(Stag_Margin_Hover);
                    }
                    draw_rectangle_outline(app, box, 6.f, 3.f, margin);
                    
                    y_pos = y.max;
                    if (y_pos >= tabs_body.max.y1){
                        break;
                    }
                }
            }break;
            
            case ProfileInspectTab_Memory:
            {
                draw_set_clip(app, tabs_body.max);
                Range_f32 x = rect_range_x(tabs_body.max);
                f32 y_pos = tabs_body.max.y0;
                Memory_Annotation annotation = system_memory_annotation(scratch);
                
                Base_Allocator *allocator = get_base_allocator_system();
                
                Memory_Bucket *first_bucket = 0;
                Memory_Bucket *last_bucket = 0;
                i32 bucket_count = 0;
                Table_Data_u64 table = make_table_Data_u64(allocator, 100);
                
                for (Memory_Annotation_Node *node = annotation.first, *next = 0;
                     node != 0;
                     node = next){
                    next = node->next;
                    Data key = make_data(node->location.str, node->location.size);
                    Table_Lookup lookup = table_lookup(&table, key);
                    Memory_Bucket *bucket = 0;
                    if (lookup.found_match){
                        u64 val = 0;
                        table_read(&table, lookup, &val);
                        bucket = (Memory_Bucket*)IntAsPtr(val);
                    }
                    else{
                        bucket = push_array_zero(scratch, Memory_Bucket, 1);
                        sll_queue_push(first_bucket, last_bucket, bucket);
                        bucket_count += 1;
                        bucket->location = node->location;
                        table_insert(&table, key, PtrAsInt(bucket));
                    }
                    sll_queue_push(bucket->annotation.first, bucket->annotation.last, node);
                    bucket->annotation.count += 1;
                    bucket->total_memory += node->size;
                }
                
                Memory_Bucket **buckets = push_array(scratch, Memory_Bucket*, bucket_count);
                i32 counter = 0;
                for (Memory_Bucket *node = first_bucket;
                     node != 0;
                     node = node->next){
                    buckets[counter] = node;
                    counter += 1;
                }
                
                profile_memory_sort_by_count(buckets, 0, bucket_count);
                
                for (i32 i = bucket_count - 1; i >= 0; i -= 1){
                    Memory_Bucket *node = buckets[i];
                    Range_f32 y = If32_size(y_pos, block_height);
                    
                    Fancy_Line list = {};
                    push_fancy_stringf(scratch, &list, fcolor_id(Stag_Pop2), "[%12llu] / %6d ",
                                       node->total_memory, node->annotation.count);
                    push_fancy_stringf(scratch, &list, fcolor_id(Stag_Pop1), "%.*s",
                                       string_expand(node->location));
                    
                    Vec2_f32 p = V2f32(x.min + x_half_padding,
                                       (y.min + y.max - line_height)*0.5f);
                    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
                    
                    Rect_f32 box = Rf32(x, y);
                    FColor margin = fcolor_id(Stag_Margin);
                    if (rect_contains_point(box, m_p)){
                        inspect->location_jump_hovered = node->location;
                        margin = fcolor_id(Stag_Margin_Hover);
                    }
                    
                    y_pos = y.max;
                    if (y_pos >= tabs_body.max.y1){
                        break;
                    }
                }
                
                table_free(&table);
            }break;
            
            case ProfileInspectTab_Selection:
            {
                if (inspect->selected_thread != 0){
                    profile_draw_node(app, view, face_id,
                                      &inspect->selected_thread->root, tabs_body.max,
                                      inspect, m_p);
                }
                else if (inspect->selected_slot != 0){
                    
                }
                else if (inspect->selected_node != 0){
                    profile_draw_node(app, view, face_id,
                                      inspect->selected_node, tabs_body.max,
                                      inspect, m_p);
                }
            }break;
        }
        
        if (!rect_contains_point(region, m_p)){
            // NOTE(allen): don't draw tool tip when the mouse doesn't hover in our view
        }
        else if (inspect->tab_id_hovered != ProfileInspectTab_None){
            // NOTE(allen): no tool tip for tabs
        }
        else{
            Fancy_Block block = {};
            FColor text_color = fcolor_change_alpha(app, f_white, 0.5f);
            FColor back_color = fcolor_change_alpha(app, f_black, 0.5f);
            
            if (inspect->full_name_hovered.size > 0){
                Fancy_Line *line = push_fancy_line(scratch, &block, text_color);
                push_fancy_stringf(scratch, line, "%.*s",
                                   string_expand(inspect->full_name_hovered));
                if (inspect->unique_counter_hovered > 0){
                    push_fancy_stringf(scratch, line, text_color, 0.5f, 0.f,
                                       "#%4llu", inspect->unique_counter_hovered);
                }
            }
            if (inspect->location_jump_hovered.size > 0){
                Fancy_Line *line = push_fancy_line(scratch, &block, text_color);
                push_fancy_stringf(scratch, line, "[shift] '%.*s'",
                                   string_expand(inspect->location_jump_hovered));
            }
            
            draw_tool_tip(app, face_id, &block, m_p, region,
                          x_padding, x_half_padding, back_color);
        }
    }
    
    draw_set_clip(app, prev_clip);
}

function void
profile_inspect__left_click(Application_Links *app, View_ID view,
                            Profile_Inspection *insp, Input_Event *event){
    if (has_modifier(event, KeyCode_Shift)){
        if (insp->location_jump_hovered.size != 0){
            View_ID target_view = view;
            target_view = get_next_view_looped_primary_panels(app, target_view,
                                                              Access_Always);
            String_Const_u8 location = insp->location_jump_hovered;
            jump_to_location(app, target_view, location);
        }
    }
    else{
        if (insp->tab_id_hovered != ProfileInspectTab_None){
            insp->tab_id = insp->tab_id_hovered;
        }
        else if (insp->hover_thread != 0){
            profile_select_thread(insp, insp->hover_thread);
        }
        else if (insp->hover_slot != 0){
            profile_select_slot(insp, insp->hover_slot);
        }
        else if (insp->hover_node != 0){
            profile_select_node(insp, insp->hover_node);
        }
    }
}

CUSTOM_UI_COMMAND_SIG(profile_inspect)
CUSTOM_DOC("Inspect all currently collected profiling information in 4coder's self profiler.")
{
    Profile_Global_List *list = get_core_profile_list(app);
    if (HasFlag(list->disable_bits, ProfileEnable_InspectBit)){
        return;
    }
    
    profile_set_enabled(list, false, ProfileEnable_InspectBit);
    
    Scratch_Block scratch(app);
    global_profile_inspection = profile_parse(scratch, list);
    Profile_Inspection *insp = &global_profile_inspection;
    
    View_ID view = get_active_view(app, Access_Always);
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = profile_render;
    ctx.hides_buffer = true;
    View_Context_Block ctx_block(app, view, &ctx);
    
    for (;;){
        User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
        if (in.abort){
            break;
        }
        
        b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_MouseButton:
            {
                switch (in.event.mouse.code){
                    case MouseCode_Left:
                    {
                        profile_inspect__left_click(app, view, insp, &in.event);
                    }break;
                }
            }break;
            
            default:
            {
                handled = false;
            }break;
        }
        
        if (!handled){
            if (ui_fallback_command_dispatch(app, view, &in)){
                break;
            }
        }
    }
    
    profile_set_enabled(list, true, ProfileEnable_InspectBit);
}

// BOTTOM
