/*
 * 4coder_profile.cpp - Built in self profiling UI.
 */

// TOP

struct Profile_Group_Ptr{
    Profile_Group_Ptr *next;
    Profile_Group *group;
};

struct Profile_Universal_Slot{
    Profile_Universal_Slot *next;
    
    String_Const_u8 source_location;
    u32 slot_index;
    
    String_Const_u8 name;
    
    u32 count;
    u64 total_time;
};

struct Profile_Thread{
    Profile_Thread *next;
    i32 thread_id;
    
    Profile_Group_Ptr *first_group;
    Profile_Group_Ptr *last_group;
    
    Profile_Universal_Slot *first_slot;
    Profile_Universal_Slot *last_slot;
    i32 slot_count;
    
    Profile_Universal_Slot **sorted_slots;
};

////////////////////////////////

function Profile_Thread*
get_column_from_thread_id(Profile_Thread *first, i32 thread_id){
    Profile_Thread *result = 0;
    for (Profile_Thread *node = first;
         node != 0;
         node = node->next){
        if (node->thread_id == thread_id){
            result = node;
            break;
        }
    }
    return(result);
}

function Profile_Universal_Slot*
get_universal_slot(Profile_Thread *column, String_Const_u8 source_location,
                   u32 slot_index){
    Profile_Universal_Slot *result = 0;
    for (Profile_Universal_Slot *node = column->first_slot;
         node != 0;
         node = node->next){
        if (node->slot_index == slot_index &&
            string_match(node->source_location, source_location)){
            result = node;
            break;
        }
    }
    return(result);
}

function void
sort_universal_slots(Profile_Universal_Slot **slots, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot = one_past_last - 1;
        Profile_Universal_Slot *pivot_slot = slots[pivot];
        i32 j = first;
        for (i32 i = first; i < pivot; i += 1){
            Profile_Universal_Slot *slot = slots[i];
            b32 is_less = false;
            if (slot->total_time < pivot_slot->total_time){
                is_less = true;
            }
            else if (slot->total_time == pivot_slot->total_time){
                if (slot->count < pivot_slot->count){
                    is_less = true;
                }
                else if (slot->count == pivot_slot->count){
                    i32 comp = string_compare(slot->source_location,
                                              pivot_slot->source_location);
                    if (comp < 0){
                        is_less = true;
                    }
                    else if (comp == 0){
                        if (slot->slot_index < pivot_slot->slot_index){
                            is_less = true;
                        }
                    }
                }
            }
            if (is_less){
                Swap(Profile_Universal_Slot*, slots[i], slots[j]);
                j += 1;
            }
        }
        Swap(Profile_Universal_Slot*, slots[j], slots[pivot]);
        sort_universal_slots(slots, first, pivot);
        sort_universal_slots(slots, pivot + 1, one_past_last);
    }
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
    
    system_mutex_acquire(profile_history.mutex);
    
    // TODO(allen): cache this result!
    Profile_Thread *thread_first = 0;
    Profile_Thread *thread_last = 0;
    i32 thread_count = 0;
    for (Profile_Group *node = profile_history.first;
         node != 0;
         node = node->next){
        i32 thread_id = node->thread_id;
        Profile_Thread *column = get_column_from_thread_id(thread_first, thread_id);
        if (column == 0){
            column = push_array_zero(scratch, Profile_Thread, 1);
            sll_queue_push(thread_first, thread_last, column);
            thread_count += 1;
            column->thread_id = thread_id;
        }
        
        Profile_Group_Ptr *ptr = push_array(scratch, Profile_Group_Ptr, 1);
        sll_queue_push(column->first_group, column->last_group, ptr);
        ptr->group = node;
        
        String_Const_u8 source_location = node->source_location;
        for (Profile_Record *record = node->first;
             record != 0;
             record = record->next){
            Profile_Universal_Slot *univ_slot =
                get_universal_slot(column, source_location, record->slot_index);
            if (univ_slot == 0){
                univ_slot = push_array(scratch, Profile_Universal_Slot, 1);
                sll_queue_push(column->first_slot, column->last_slot, univ_slot);
                column->slot_count += 1;
                univ_slot->source_location = source_location;
                univ_slot->slot_index = record->slot_index;
                univ_slot->name = node->slot_names[univ_slot->slot_index];
                univ_slot->count = 0;
                univ_slot->total_time = 0;
            }
            univ_slot->count += 1;
            univ_slot->total_time += record->time;
        }
    }
    
    for (Profile_Thread *column = thread_first;
         column != 0;
         column = column->next){
        i32 count = column->slot_count;
        Profile_Universal_Slot **slots = push_array(scratch, Profile_Universal_Slot*, count);
        column->sorted_slots = slots;
        i32 counter = 0;
        for (Profile_Universal_Slot *node = column->first_slot;
             node != 0;
             node = node->next){
            slots[counter] = node;
            counter += 1;
        }
        sort_universal_slots(slots, 0, count);
    }
    
    f32 column_width = rect_width(region)/(f32)thread_count;
    
    Rect_f32_Pair header_body = rect_split_top_bottom(region, block_height);
    
    Range_f32 full_y = rect_range_y(region);
    Range_f32 header_y = rect_range_y(header_body.min);
    Range_f32 body_y = rect_range_y(header_body.max);
    
    f32 pos_x = region.x0;
    for (Profile_Thread *column = thread_first;
         column != 0;
         column = column->next){
        Range_f32 column_x = If32_size(pos_x, column_width);
        Range_f32 text_x = If32(column_x.min + 6.f, column_x.max - 6.f);
        f32 text_width = range_size(text_x);
        f32 count_width = normal_advance*6.f;
        f32 time_width = normal_advance*9.f;
        f32 half_padding = normal_advance*0.25f;
        f32 label_width = text_width - count_width - time_width;
        if (label_width < normal_advance*10.f){
            f32 count_ratio =  6.f/25.f;
            f32 time_ratio  =  9.f/25.f;
            f32 label_ratio = 10.f/25.f;
            count_width = text_width*count_ratio;
            time_width  = text_width*time_ratio;
            label_width = text_width*label_ratio;
        }
        
        i32 count = column->slot_count;
        
        draw_set_clip(app, Rf32(column_x, full_y));
        
        // NOTE(allen): header
        {
            Rect_f32 box = Rf32(column_x, header_y);
            draw_rectangle_outline(app, box, 6.f, 3.f, Stag_Margin_Active);
        }
        
        // NOTE(allen): list
        {        
            f32 pos_y = body_y.min;
            Profile_Universal_Slot **slot_ptr = column->sorted_slots;
            for (i32 i = 0; i < count; i += 1, slot_ptr += 1){
                Range_f32 slot_y = If32_size(pos_y, block_height);
                Rect_f32 box = Rf32(column_x, slot_y);
                draw_rectangle_outline(app, box, 6.f, 3.f, Stag_Margin);
                pos_y = slot_y.max;
            }
        }
        
        // NOTE(allen): header text
        {
            draw_set_clip(app, Rf32(text_x, full_y));
            
            Fancy_String_List list = {};
            push_fancy_stringf(scratch, &list, fancy_id(Stag_Keyword),
                               "%d", column->thread_id);
            f32 y = (header_y.min + header_y.max - line_height)*0.5f;
            draw_fancy_string(app, face_id, list.first, V2f32(text_x.min, y), Stag_Default, 0);
        }
        
        // NOTE(allen): list text counts
        {
            Range_f32 x = If32_size(text_x.min + label_width + time_width + half_padding, count_width);
            f32 pos_y = body_y.min;
            Profile_Universal_Slot **slot_ptr = column->sorted_slots;
            for (i32 i = 0; i < count; i += 1, slot_ptr += 1){
                Range_f32 slot_y = If32_size(pos_y, block_height);
                f32 y = (slot_y.min + slot_y.max - line_height)*0.5f;
                Profile_Universal_Slot *slot = *slot_ptr;
                Fancy_String_List list = {};
                push_fancy_stringf(scratch, &list, fancy_id(Stag_Pop1),
                                   "%5u", slot->count);
                draw_fancy_string(app, face_id, list.first, V2f32(x.min, y), Stag_Default, 0);
                pos_y = slot_y.max;
            }
        }
        
        // NOTE(allen): list text labels
        {
            Range_f32 x = If32_size(text_x.min + label_width + half_padding, time_width - half_padding);
            draw_set_clip(app, Rf32(x, full_y));
            
            f32 pos_y = body_y.min;
            Profile_Universal_Slot **slot_ptr = column->sorted_slots;
            for (i32 i = 0; i < count; i += 1, slot_ptr += 1){
                Range_f32 slot_y = If32_size(pos_y, block_height);
                f32 y = (slot_y.min + slot_y.max - line_height)*0.5f;
                Profile_Universal_Slot *slot = *slot_ptr;
                Fancy_String_List list = {};
                push_fancy_stringf(scratch, &list, fancy_id(Stag_Pop2),
                                   "%-8.6f", (f32)(slot->total_time)/1000000.f);
                draw_fancy_string(app, face_id, list.first, V2f32(x.min, y), Stag_Default, 0);
                pos_y = slot_y.max;
            }
        }
        
        // NOTE(allen): list text labels
        {
            Range_f32 x = If32_size(text_x.min, label_width - half_padding);
            draw_set_clip(app, Rf32(x, full_y));
            
            f32 pos_y = body_y.min;
            Profile_Universal_Slot **slot_ptr = column->sorted_slots;
            for (i32 i = 0; i < count; i += 1, slot_ptr += 1){
                Range_f32 slot_y = If32_size(pos_y, block_height);
                f32 y = (slot_y.min + slot_y.max - line_height)*0.5f;
                Profile_Universal_Slot *slot = *slot_ptr;
                Fancy_String_List list = {};
                push_fancy_stringf(scratch, &list, fancy_id(Stag_Default),
                                   "%.*s ", string_expand(slot->name));
                draw_fancy_string(app, face_id, list.first, V2f32(x.min, y), Stag_Default, 0);
                pos_y = slot_y.max;
            }
        }
        
        pos_x = column_x.max;
    }
    
    system_mutex_release(profile_history.mutex);
    
    draw_set_clip(app, prev_clip);
}

CUSTOM_COMMAND_SIG(profile_inspect)
CUSTOM_DOC("Inspect all currently collected profiling information in 4coder's self profiler.")
{
    View_ID view = get_active_view(app, Access_Always);
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = profile_render;
    ctx.hides_buffer = true;
    view_push_context(app, view, &ctx);
    
    profile_history_set_enabled(false, ProfileEnable_InspectBit);
    
    for (;;){
        User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
        if (in.abort){
            break;
        }
        
#if 0
        b32 handled = true;
        switch (in.event.kind){
            default:
            {
                handled = false;
            }break;
        }
#else
        b32 handled = false;
#endif

        if (!handled){
            // TODO(allen): dedup this stuff.
            // TODO(allen): get mapping and map from a more flexible source.
            Mapping *mapping = &framework_mapping;
            Command_Map *map = mapping_get_map(mapping, mapid_global);
            if (mapping != 0 && map != 0){
                Command_Binding binding =
                    map_get_binding_recursive(mapping, map, &in.event);
                if (binding.custom != 0){
                    i64 old_num = get_current_input_sequence_number(app);
                    binding.custom(app);
                    i64 num = get_current_input_sequence_number(app);
                    if (old_num < num){
                        break;
                    }
                }
                else{
                    leave_current_input_unhandled(app);
                }
            }
            else{
                leave_current_input_unhandled(app);
            }
        }
    }
    
    profile_history_set_enabled(true, ProfileEnable_InspectBit);
    
    view_pop_context(app, view);
}

// BOTTOM
