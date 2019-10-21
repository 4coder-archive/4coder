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
        slot->hit_count += 1;
        
        if (quit_loop){
            break;
        }
    }
    return(record);
}

function Profile_Inspection
profile_parse(Arena *arena){
    Mutex_Lock lock(global_prof_mutex);
    Profile_Global_List *src = &global_prof_list;
    
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
        
        // NOTE(allen): This is the "negative infinity" range.
        // We will be "maxing" it against all the ranges durring the parse,
        // to get the root range.
        Range_u64 time_range = {max_u64, 0};
        profile_parse_record(arena, &result, &insp_thread->root, node->first_record,
                             &time_range);
        insp_thread->root.time = time_range;
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
    
    Fancy_String_List list = {};
    push_fancy_string(scratch, &list, fancy_pass_through(), string);
    
    b32 hover = false;
    f32 width = get_fancy_string_advance(app, state->face_id, list.first);
    Rect_f32 box = Rf32(If32_size(state->p.x, width), state->tabs_y);
    if (rect_contains_point(box, state->m_p)){
        hover = true;
        insp->tab_id_hovered = tab_id;
    }
    
    int_color text = Stag_Default;
    int_color back = 0;
    if (insp->tab_id == tab_id){
        text = Stag_Pop2;
    }
    else if (hover){
        text = Stag_Pop1;
    }
    
    Vec2_f32 np = draw_fancy_string(app, state->face_id, list.first, state->p,
                                    text, back);
    state->p = np;
    state->p.x += state->x_half_padding;
}

function void
profile_render(Application_Links *app, Frame_Info frame_info, View_ID view){
    Scratch_Block scratch(app);
    
    Rect_f32 region = draw_background_and_margin(app, view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 normal_advance = metrics.normal_advance;
    f32 block_height = line_height*2.f;
    
    Mouse_State mouse = get_mouse_state(app);
    Vec2_f32 m_p = V2f32(mouse.p);
    
    Profile_Inspection *inspect = &global_profile_inspection;
    
    if (inspect->thread_count == 0){
        Fancy_String_List text = {};
        push_fancy_string(scratch, &text, fancy_id(Stag_Pop2),
                          string_u8_litexpr("no profile data"));
        f32 width = get_fancy_string_advance(app, face_id, text.first);
        Vec2_f32 view_center = (region.p0 + region.p1)*0.5f;
        Vec2_f32 half_dim = V2f32(width, line_height)*0.5f;
        Vec2_f32 p = view_center - half_dim;
        draw_fancy_string(app, face_id, text.first, p, Stag_Default, 0);
    }
    else{
        Rect_f32_Pair tabs_body = rect_split_top_bottom(region, line_height + 2.f);
        Range_f32 tabs_y = rect_range_y(tabs_body.min);
        
        f32 x_padding = normal_advance*1.5f;
        f32 x_half_padding = x_padding*0.5f;
        
        inspect->tab_id_hovered = ProfileInspectTab_None;
        block_zero_struct(&inspect->location_jump_hovered);
        
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
            
            draw_rectangle(app, tabs_body.min, 0.f, Stag_Margin_Hover);
            
            if (inspect->tab_id == ProfileInspectTab_None){
                inspect->tab_id = ProfileInspectTab_Threads;
            }
            
            profile_draw_tab(app, &tab_state, inspect,
                             string_u8_litexpr("threads"),
                             ProfileInspectTab_Threads);
            
            if (inspect->slot_count > 0){
                profile_draw_tab(app, &tab_state, inspect,
                                 string_u8_litexpr("slots"),
                                 ProfileInspectTab_Slots);
            }
            
            if (inspect->error_count > 0){
                profile_draw_tab(app, &tab_state, inspect,
                                 string_u8_litexpr("errors"),
                                 ProfileInspectTab_Errors);
            }
        }
        
        switch (inspect->tab_id){
            case ProfileInspectTab_Slots:
            {
                draw_set_clip(app, tabs_body.max);
                Range_f32 x = rect_range_x(tabs_body.max);
                f32 y_pos = tabs_body.max.y0;
                for (Profile_Slot *node = inspect->first_slot;
                     node != 0;
                     node = node->next){
                    Range_f32 y = If32_size(y_pos, block_height);
                    
                    i32 name_width = 30;
                    Fancy_String_List list = {};
                    if (node->name.size > name_width){
                        push_fancy_stringf(scratch, &list, fancy_id(Stag_Pop1),
                                           "%.*s... ",
                                           name_width - 3, node->name.str);
                    }
                    else{
                        push_fancy_stringf(scratch, &list, fancy_id(Stag_Pop1),
                                           "%-*.*s ",
                                           name_width,
                                           string_expand(node->name));
                    }
                    
                    if (node->corrupted_time){
                        push_fancy_string(scratch, &list, fancy_id(Stag_Pop2),
                                          string_u8_litexpr("timing error "));
                    }
                    else{
                        push_fancy_stringf(scratch, &list, fancy_id(Stag_Pop2),
                                           "%11.8fs ",
                                           ((f32)node->total_time)/1000000.f);
                    }
                    
                    push_fancy_stringf(scratch, &list, fancy_id(Stag_Keyword),
                                       "hit # %5d", node->hit_count);
                    
                    Vec2_f32 p = V2f32(x.min + x_half_padding,
                                       (y.min + y.max - line_height)*0.5f);
                    draw_fancy_string(app, face_id, list.first, p, 0, 0);
                    
                    Rect_f32 box = Rf32(x, y);
                    int_color margin = Stag_Margin;
                    if (rect_contains_point(box, m_p)){
                        inspect->location_jump_hovered = node->location;
                        margin = Stag_Margin_Hover;
                    }
                    draw_rectangle_outline(app, box, 6.f, 3.f, margin);
                    
                    y_pos = y.max;
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
                    
                    Fancy_String_List list = {};
                    push_fancy_string(scratch, &list, fancy_id(Stag_Pop2),
                                      node->message);
                    
                    Vec2_f32 p = V2f32(x.min + x_half_padding,
                                       (y.min + y.max - line_height)*0.5f);
                    draw_fancy_string(app, face_id, list.first, p, 0, 0);
                    
                    Rect_f32 box = Rf32(x, y);
                    int_color margin = Stag_Margin;
                    if (rect_contains_point(box, m_p)){
                        inspect->location_jump_hovered = node->location;
                        margin = Stag_Margin_Hover;
                    }
                    draw_rectangle_outline(app, box, 6.f, 3.f, margin);
                    
                    y_pos = y.max;
                }
            }break;
        }
        
        if (inspect->tab_id_hovered != ProfileInspectTab_None){
            // NOTE(allen): no tool tip for tabs
        }
        else if (inspect->location_jump_hovered.size > 0){
            draw_set_clip(app, region);
            
            Fancy_String_List list = {};
            push_fancy_stringf(scratch, &list, fancy_rgba(1.f, 1.f, 1.f, 0.5f),
                               "jump to: '%.*s'",
                               string_expand(inspect->location_jump_hovered));
            f32 width = get_fancy_string_advance(app, face_id, list.first);
            Vec2_f32 dims = V2f32(width + x_padding, line_height + 2.f);
            Rect_f32 box = get_tool_tip_box(region, m_p, dims);
            if (rect_area(box) > 0.f){
                draw_rectangle(app, box, 6.f, 0x80000000);
                draw_fancy_string(app, face_id, list.first,
                                  V2f32(box.x0 + x_half_padding, box.y0 + 1.f),
                                  0, 0);
            }
        }
    }
    
    draw_set_clip(app, prev_clip);
}

CUSTOM_UI_COMMAND_SIG(profile_inspect)
CUSTOM_DOC("Inspect all currently collected profiling information in 4coder's self profiler.")
{
    if (HasFlag(global_prof_list.disable_bits, ProfileEnable_InspectBit)){
        return;
    }
    
    global_prof_set_enabled(false, ProfileEnable_InspectBit);
    
    Scratch_Block scratch(app);
    global_profile_inspection = profile_parse(scratch);
    Profile_Inspection *insp = &global_profile_inspection;
    
    View_ID view = get_active_view(app, Access_Always);
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = profile_render;
    ctx.hides_buffer = true;
    view_push_context(app, view, &ctx);
    
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
                        if (insp->tab_id_hovered != ProfileInspectTab_None){
                            insp->tab_id = insp->tab_id_hovered;
                        }
                        else if (insp->location_jump_hovered.size != 0){
                            View_ID target_view = view;
                            target_view = get_next_view_looped_primary_panels(app, target_view, Access_Always);
                            String_Const_u8 location = insp->location_jump_hovered;
                            jump_to_location(app, target_view, location);
                        }
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
    
    global_prof_set_enabled(true, ProfileEnable_InspectBit);
    
    view_pop_context(app, view);
}

// BOTTOM
