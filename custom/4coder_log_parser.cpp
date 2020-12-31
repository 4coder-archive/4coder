/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.08.2019
 *
 * Log parser.
 *
 */

// TOP

internal u64
log_parse__string_code(Log_Parse *parse, String_Const_u8 string, Log_String_Source string_source){
    u64 result = 0;
    if (string.size > 0){
        String_Const_u8 data = make_data(string.str, string.size);
        Table_Lookup lookup = table_lookup(&parse->string_to_id_table, data);
        if (lookup.found_match){
            table_read(&parse->string_to_id_table, lookup, &result);
        }
        else{
            if (string_source == LogParse_ExternalString){
                data = push_data_copy(parse->arena, data);
            }
            result = parse->string_id_counter;
            parse->string_id_counter += 1;
            table_insert(&parse->string_to_id_table, data, result);
            table_insert(&parse->id_to_string_table, result, data);
        }
    }
    return(result);
}

internal String_Const_u8
log_parse__get_string(Log_Parse *parse, u64 code){
    Table_Lookup lookup = table_lookup(&parse->id_to_string_table, code);
    String_Const_u8 result = {};
    if (lookup.found_match){
        table_read(&parse->id_to_string_table, lookup, &result);
    }
    return(result);
}

internal Log_Event*
log_parse__event(Log_Parse *parse,
                 String_Const_u8 file_name, String_Const_u8 line_number, String_Const_u8 event_name){
    Log_Event *new_event = push_array(parse->arena, Log_Event, 1);
    sll_queue_push(parse->first_event, parse->last_event, new_event);
    parse->event_count += 1;
    new_event->src_file_name = log_parse__string_code(parse, file_name, LogParse_ExternalString);
    new_event->event_name = log_parse__string_code(parse, event_name, LogParse_ExternalString);
    new_event->line_number = string_to_integer(line_number, 10);
    new_event->event_number = parse->event_count;
    return(new_event);
}

internal Log_Tag*
log_parse__tag(Log_Parse *parse, Log_Event *event, String_Const_u8 tag_name, String_Const_u8 tag_value){
    Log_Tag *new_tag = push_array(parse->arena, Log_Tag, 1);
    sll_queue_push(event->first_tag, event->last_tag, new_tag);
    event->tag_count += 1;
    new_tag->name = log_parse__string_code(parse, tag_name, LogParse_ExternalString);
    if (tag_value.size == 0){
        new_tag->value.kind = LogTagKind_String;
        new_tag->value.value = 0;
    }
    else{
        if (tag_value.str[0] == '"'){
            if (tag_value.size == 1){
                new_tag->value.kind = LogTagKind_String;
                new_tag->value.value = 0;
            }
            else{
                tag_value = string_skip(tag_value, 1);
                if (tag_value.str[tag_value.size - 1] == '"'){
                    tag_value = string_chop(tag_value, 1);
                }
                String_Const_u8 escape = string_interpret_escapes(parse->arena, tag_value);
                new_tag->value.kind = LogTagKind_String;
                new_tag->value.value = log_parse__string_code(parse, escape, LogParse_PreAllocatedString);
            }
        }
        else{
            new_tag->value.kind = LogTagKind_Integer;
            b32 is_negative = false;
            if (string_match(string_prefix(tag_value, 1), string_u8_litexpr("-"))){
                tag_value = string_skip(tag_value, 1);
                is_negative = true;
            }
            if (string_match(string_prefix(tag_value, 2), string_u8_litexpr("0x"))){
                tag_value = string_skip(tag_value, 2);
                new_tag->value.value_s = (i64)string_to_integer(tag_value, 16);
            }
            else{
                new_tag->value.value_s = (i64)string_to_integer(tag_value, 10);
            }
            if (is_negative){
                new_tag->value.value_s = -new_tag->value.value_s;
            }
        }
    }
    return(new_tag);
}

internal Log_Event_List*
log_parse_get_list_tag_value(Log_Parse *parse, u64 name, Log_Tag_Value value){
    Log_Event_List *result = 0;
    Log_Tag_Name_Value key = {name, value};
    Table_Lookup lookup = table_lookup(&parse->tag_value_to_event_list_table, make_data_struct(&key));
    if (lookup.found_match){
        u64 val = 0;
        table_read(&parse->tag_value_to_event_list_table, lookup, &val);
        result = (Log_Event_List*)IntAsPtr(val);
    }
    return(result);
}

internal Log_Event_List*
log_parse__get_or_make_list_tag_value(Log_Parse *parse, Log_Tag *tag){
    Log_Event_List *result = 0;
    Log_Tag_Name_Value key = {tag->name, tag->value};
    String_Const_u8 data_key = make_data_struct(&key);
    Table_Lookup lookup = table_lookup(&parse->tag_value_to_event_list_table, data_key);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&parse->tag_value_to_event_list_table, lookup, &val);
        result = (Log_Event_List*)IntAsPtr(val);
    }
    else{
        result = push_array_zero(parse->arena, Log_Event_List, 1);
        table_insert(&parse->tag_value_to_event_list_table, push_data_copy(parse->arena, data_key),
                     (u64)PtrAsInt(result));
    }
    return(result);
}

internal Log_Event_List*
log_parse_get_list_tag_name(Log_Parse *parse, u64 name){
    Log_Event_List *result = 0;
    Table_Lookup lookup = table_lookup(&parse->tag_name_to_event_list_table, name);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&parse->tag_name_to_event_list_table, lookup, &val);
        result = (Log_Event_List*)IntAsPtr(val);
    }
    return(result);
}

internal Log_Event_List*
log_parse__get_or_make_list_tag_name(Log_Parse *parse, Log_Tag *tag){
    Log_Event_List *result = 0;
    Table_Lookup lookup = table_lookup(&parse->tag_name_to_event_list_table, tag->name);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&parse->tag_name_to_event_list_table, lookup, &val);
        result = (Log_Event_List*)IntAsPtr(val);
    }
    else{
        result = push_array_zero(parse->arena, Log_Event_List, 1);
        table_insert(&parse->tag_name_to_event_list_table, tag->name, (u64)PtrAsInt(result));
    }
    return(result);
}

internal Log_Parse
make_log_parse(Arena *arena, String_Const_u8 source){
    Log_Parse parse = {};
    parse.arena = arena;
    parse.string_id_counter = 1;
    parse.string_to_id_table = make_table_Data_u64(arena->base_allocator, 500);
    parse.id_to_string_table = make_table_u64_Data(arena->base_allocator, 500);
    
    for (;source.size > 0;){
        u64 end_of_line = string_find_first(source, '\n');
        String_Const_u8 line = string_prefix(source, end_of_line);
        line = string_skip_chop_whitespace(line);
        source = string_skip(source, end_of_line + 1);
        
        String_Const_u8 src_file_name = {};
        String_Const_u8 src_line_number = {};
        b32 got_source_position = false;
        
        String_Const_u8 whole_line = line;
        
        {
            u64 colon1 = string_find_first(line, ':');
            src_file_name = string_prefix(line, colon1);
            line = string_skip(line, colon1 + 1);
            
            u64 colon2 = string_find_first(line, ':');
            src_line_number = string_prefix(line, colon2);
            line = string_skip(line, colon2 + 1);
            
            if (string_is_integer(src_line_number, 10)){
                got_source_position = true;
            }
        }
        
        if (!got_source_position){
            line = whole_line;
            
            u64 colon0 = string_find_first(line, ':');
            u64 colon1 = string_find_first(line, colon0 + 1, ':');
            src_file_name = string_prefix(line, colon1);
            line = string_skip(line, colon1 + 1);
            
            u64 colon2 = string_find_first(line, ':');
            src_line_number = string_prefix(line, colon2);
            line = string_skip(line, colon2 + 1);
            
            if (string_is_integer(src_line_number, 10)){
                got_source_position = true;
            }
        }
        
        if (got_source_position){
            u64 bracket_open = string_find_first(line, '[');
            String_Const_u8 event_name = string_prefix(line, bracket_open);
            event_name = string_skip_chop_whitespace(event_name);
            line = string_skip(line, bracket_open + 1);
            
            Log_Event *event = log_parse__event(&parse,
                                                src_file_name, src_line_number, event_name);
            
            for (;line.size > 0;){
                u64 bracket_close = string_find_first(line, ']');
                String_Const_u8 tag = string_prefix(line, bracket_close);
                line = string_skip(line, bracket_close + 1);
                bracket_open = string_find_first(line, '[');
                line = string_skip(line, bracket_open + 1);
                
                u64 equal_sign = string_find_first(tag, '=');
                String_Const_u8 tag_name = string_prefix(tag, equal_sign);
                String_Const_u8 tag_contents = string_skip(tag, equal_sign + 1);
                
                log_parse__tag(&parse, event, tag_name, tag_contents);
            }
        }
    }
    
    ////////////////////////////////
    
    // NOTE(allen): fill acceleration structures
    
    parse.tag_value_to_event_list_table = make_table_Data_u64(arena->base_allocator, Thousand(1));
    parse.tag_name_to_event_list_table = make_table_u64_u64(arena->base_allocator, 100);
    
    for (Log_Event *event = parse.first_event;
         event != 0;
         event = event->next){
        for (Log_Tag *tag = event->first_tag;
             tag != 0;
             tag = tag->next){
            {
                Log_Event_List *list = log_parse__get_or_make_list_tag_value(&parse, tag);
                Log_Event_Ptr_Node *node = push_array(arena, Log_Event_Ptr_Node, 1);
                sll_queue_push(list->first, list->last, node);
                list->count += 1;
                node->event = event;
            }
            {
                Log_Event_List *list = log_parse__get_or_make_list_tag_name(&parse, tag);
                Log_Event_Ptr_Node *node = push_array(arena, Log_Event_Ptr_Node, 1);
                sll_queue_push(list->first, list->last, node);
                list->count += 1;
                node->event = event;
            }
        }
    }
    
    for (Log_Event *event = parse.first_event;
         event != 0;
         event = event->next){
        i32 slot_count = event->tag_count*3/2;
        event->tag_name_to_tag_ptr_table = make_table_u64_u64(arena->base_allocator, slot_count);
        for (Log_Tag *tag = event->first_tag;
             tag != 0;
             tag = tag->next){
            table_insert(&event->tag_name_to_tag_ptr_table, tag->name, (u64)PtrAsInt(tag));
        }
    }
    
    return(parse);
}

////////////////////////////////

internal void
log_events_sort_by_tag__inner(Log_Event **events, Log_Sort_Key *keys, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot_index = one_past_last - 1;
        Log_Sort_Key *pivot_key = keys + pivot_index;
        i32 j = first;
        for (i32 i = first; i < one_past_last; i += 1){
            Log_Sort_Key *key = keys + i;
            b32 key_is_less_than_pivot_key = false;
            if (key->value.kind < pivot_key->value.kind){
                key_is_less_than_pivot_key = true;
            }
            else if (key->value.kind == pivot_key->value.kind){
                if (key->value.value < pivot_key->value.value){
                    key_is_less_than_pivot_key = true;
                }
                else if (key->value.value == pivot_key->value.value){
                    if (key->number < pivot_key->number){
                        key_is_less_than_pivot_key = true;
                    }
                }
            }
            if (key_is_less_than_pivot_key){
                if (j < i){
                    Swap(Log_Event*, events[i], events[j]);
                    Swap(Log_Sort_Key, keys[i], keys[j]);
                }
                j += 1;
            }
        }
        Swap(Log_Event*, events[pivot_index], events[j]);
        Swap(Log_Sort_Key, keys[pivot_index], keys[j]);
        log_events_sort_by_tag__inner(events, keys, first, j);
        log_events_sort_by_tag__inner(events, keys, j + 1, one_past_last);
    }
}

internal void
log_events_sort_by_tag(Arena *scratch, Log_Event_Ptr_Array array, u64 tag_name){
    Temp_Memory temp = begin_temp(scratch);
    Log_Sort_Key *keys = push_array(scratch, Log_Sort_Key, array.count);
    for (i32 i = 0; i < array.count; i += 1){
        Log_Event *event = array.events[i];
        Table_Lookup lookup = table_lookup(&event->tag_name_to_tag_ptr_table, tag_name);
        if (lookup.found_match){
            u64 read_val = 0;
            table_read(&event->tag_name_to_tag_ptr_table, lookup, &read_val);
            Log_Tag *tag = (Log_Tag*)IntAsPtr(read_val);
            keys[i].value = tag->value;
        }
        else{
            keys[i].value.kind = LogTagKind_Null;
            keys[i].value.value = 0;
        }
        keys[i].number = event->event_number;
    }
    
    log_events_sort_by_tag__inner(array.events, keys, 0, array.count);
    
    end_temp(temp);
}

internal Log_Event_Ptr_Array
log_event_array_from_list(Arena *arena, Log_Event_List list){
    Log_Event_Ptr_Array array = {};
    array.count = list.count;
    array.events = push_array(arena, Log_Event*, array.count);
    i32 counter = 0;
    for (Log_Event_Ptr_Node *node = list.first;
         node != 0;
         node = node->next){
        array.events[counter] = node->event;
        counter += 1;
    }
    return(array);
}

////////////////////////////////

global View_ID log_view = 0;
global Arena log_arena = {};
global Log_Parse log_parse = {};
global Log_Graph log_graph = {};
global Log_Filter_Set log_filter_set = {};
global Log_Filter_Set log_preview_set = {};

internal void
log_filter_set_init(Log_Filter_Set *set){
    block_zero_struct(set);
    for (i32 i = ArrayCount(set->filters_memory) - 1; i >= 0; i -= 1){
        sll_stack_push(set->free_filters, &set->filters_memory[i]);
    }
}

internal Log_Filter_Set*
log_filter_set_from_tab(Log_Graph_List_Tab tab){
    Log_Filter_Set *result = 0;
    switch (tab){
        case LogTab_Filters:
        {
            result = &log_filter_set;
        }break;
        case LogTab_Previews:
        {
            result = &log_preview_set;
        }break;
    }
    return(result);
}

internal Log_Filter*
log_filter_set__new_filter(Log_Filter_Set *set, Log_Filter *prototype){
    Log_Filter *result = set->free_filters;
    if (result != 0){
        for (Log_Filter *filter = set->first;
             filter != 0;
             filter = filter->next){
            if (filter->kind == prototype->kind &&
                filter->tag_name_code == prototype->tag_name_code &&
                block_match_struct(&filter->tag_value, &prototype->tag_value)){
                result = 0;
                break;
            }
        }
        if (result != 0){
            sll_stack_pop(set->free_filters);
            block_copy_struct(result, prototype);
            zdll_push_back(set->first, set->last, result);
            set->count += 1;
            set->alter_counter += 1;
        }
    }
    return(result);
}

internal void
log_filter_set__free_filter(Log_Filter_Set *set, Log_Filter *filter){
    zdll_remove(set->first, set->last, filter);
    set->count -= 1;
    set->alter_counter += 1;
    sll_stack_push(set->free_filters, filter);
}

internal void
log_graph_fill(Application_Links *app, Rect_f32 layout_region, Face_ID face_id){
    if (log_parse.arena != 0){
        if (log_graph.holding_temp){
            end_temp(log_graph.temp);
        }
        block_zero_struct(&log_graph);
        log_graph.holding_temp = true;
        log_graph.temp = begin_temp(&log_arena);
        log_graph.layout_region = layout_region;
        log_graph.face_id = face_id;
        log_graph.filter_alter_counter = log_filter_set.alter_counter;
        log_graph.preview_alter_counter = log_preview_set.alter_counter;
        log_graph.tab = LogTab_Filters;
        
        f32 details_h = rect_height(layout_region)*.22f;
        details_h = clamp_top(details_h, 250.f);
        
        Rect_f32 details_region = Rf32(layout_region.x0, layout_region.y0,
                                       layout_region.x1, layout_region.y0 + details_h);
        Rect_f32 event_list_region = Rf32(layout_region.x0, layout_region.y0 + details_h,
                                          layout_region.x1, layout_region.y1);
        
        log_graph.details_region = details_region;
        log_graph.details_region.p0 -= layout_region.p0;
        log_graph.details_region.p1 -= layout_region.p0;
        
        u64 thread_code = log_parse__string_code(&log_parse, string_u8_litexpr("thread"),
                                                 LogParse_ExternalString);
        
        if (log_filter_set.count == 0){
            // NOTE(allen): everything goes into the filtered list
            for (Log_Event *event = log_parse.first_event;
                 event != 0;
                 event = event->next){
                Log_Event_Ptr_Node *node = push_array(&log_arena, Log_Event_Ptr_Node, 1);
                node->event = event;
                sll_queue_push(log_graph.filtered_list.first, log_graph.filtered_list.last, node);
                log_graph.filtered_list.count += 1;
            }
        }
        else{
            for (Log_Filter *filter = log_filter_set.first;
                 filter != 0;
                 filter = filter->next){
                Log_Event_List *filter_list = 0;
                if (filter->kind == LogFilter_TagValue){
                    filter_list = log_parse_get_list_tag_value(&log_parse, filter->tag_name_code,
                                                               filter->tag_value);
                }
                else if (filter->kind == LogFilter_Tag){
                    filter_list = log_parse_get_list_tag_name(&log_parse, filter->tag_name_code);
                }
                
                // NOTE(allen): combine with existing result
                if (filter == log_filter_set.first){
                    for (Log_Event_Ptr_Node *node = filter_list->first;
                         node != 0;
                         node = node->next){
                        Log_Event_Ptr_Node *new_node = push_array(&log_arena, Log_Event_Ptr_Node, 1);
                        new_node->event = node->event;
                        sll_queue_push(log_graph.filtered_list.first, log_graph.filtered_list.last, new_node);
                        log_graph.filtered_list.count += 1;
                    }
                }
                else{
                    Log_Event_Ptr_Node **fixup_ptr = &log_graph.filtered_list.first;
                    log_graph.filtered_list.last = 0;
                    for (Log_Event_Ptr_Node *node_a = log_graph.filtered_list.first, *next = 0;
                         node_a != 0;
                         node_a = next){
                        next = node_a->next;
                        
                        b32 remove_node_a = true;
                        for (Log_Event_Ptr_Node *node_b = filter_list->first;
                             node_b != 0;
                             node_b = node_b->next){
                            if (node_a->event == node_b->event){
                                remove_node_a = false;
                                break;
                            }
                        }
                        
                        if (remove_node_a){
                            *fixup_ptr = next;
                        }
                        else{
                            fixup_ptr = &node_a->next;
                            log_graph.filtered_list.last = node_a;
                        }
                    }
                }
            }
        }
        
        log_graph.event_array = log_event_array_from_list(&log_arena, log_graph.filtered_list);
        log_events_sort_by_tag(&log_arena, log_graph.event_array, thread_code);
        
        b32 had_a_tag = true;
        u64 thread_id_value = 0;
        Log_Graph_Thread_Bucket *prev_bucket = 0;
        
        for (i32 i = 0; i < log_graph.event_array.count; i += 1){
            Table_u64_u64 *tag_table = &log_graph.event_array.events[i]->tag_name_to_tag_ptr_table;
            Table_Lookup lookup = table_lookup(tag_table, thread_code);
            
            b32 emit_next_bucket = false;
            if (!lookup.found_match){
                if (had_a_tag){
                    had_a_tag = false;
                    thread_id_value = 0;
                    emit_next_bucket = true;
                }
            }
            else{
                u64 read_val = 0;
                table_read(tag_table, lookup, &read_val);
                Log_Tag *tag = (Log_Tag*)IntAsPtr(read_val);
                if (!had_a_tag){
                    had_a_tag = true;
                    thread_id_value = tag->value.value;
                    emit_next_bucket = true;
                }
                else if (thread_id_value != tag->value.value){
                    thread_id_value = tag->value.value;
                    emit_next_bucket = true;
                }
            }
            
            if (emit_next_bucket){
                Log_Graph_Thread_Bucket *bucket = push_array(&log_arena, Log_Graph_Thread_Bucket, 1);
                sll_queue_push(log_graph.first_bucket, log_graph.last_bucket, bucket);
                log_graph.bucket_count += 1;
                bucket->range.first = i;
                bucket->had_a_tag = had_a_tag;
                bucket->thread_id_value = thread_id_value;
                if (prev_bucket != 0){
                    prev_bucket->range.one_past_last = i;
                }
                prev_bucket = bucket;
            }
        }
        if (prev_bucket != 0){
            prev_bucket->range.one_past_last = log_graph.event_array.count;
        }
        
        Face_Metrics metrics = get_face_metrics(app, face_id);
        f32 line_height = metrics.line_height;
        f32 box_h = f32_floor32(line_height*1.5f);
        f32 box_w = f32_floor32(rect_width(event_list_region)/log_graph.bucket_count);
        f32 y_cursor = event_list_region.y0 - layout_region.y0;
        
        if (log_graph.bucket_count > 0){
            f32 y_bottom = 0.f;
            
            for (;;){
                i32 smallest_event_number = max_i32;
                i32 bucket_with_next_event_index = -1;
                Log_Graph_Thread_Bucket *bucket_with_next_event = 0;
                Log_Event *next_event = 0;
                i32 iteration_counter = 0;
                for (Log_Graph_Thread_Bucket *bucket = log_graph.first_bucket;
                     bucket != 0;
                     bucket = bucket->next, iteration_counter += 1){
                    if (bucket->range.first < bucket->range.one_past_last){
                        Log_Event *event = log_graph.event_array.events[bucket->range.first];
                        if (event->event_number < smallest_event_number){
                            smallest_event_number = event->event_number;
                            bucket_with_next_event_index = iteration_counter;
                            bucket_with_next_event = bucket;
                            next_event = event;
                        }
                    }
                }
                
                if (bucket_with_next_event == 0){
                    break;
                }
                
                bucket_with_next_event->range.first += 1;
                
                Log_Graph_Box *box_node = push_array(&log_arena, Log_Graph_Box, 1);
                sll_queue_push(log_graph.first_box, log_graph.last_box, box_node);
                log_graph.box_count += 1;
                Rect_f32 rect = Rf32(box_w*bucket_with_next_event_index      , y_cursor,
                                     box_w*(bucket_with_next_event_index + 1), y_cursor + box_h);
                box_node->rect = rect;
                box_node->event = next_event;
                
                y_bottom = Max(y_bottom, rect.y1);
                
                y_cursor += box_h;
            }
            
            log_graph.max_y_scroll = clamp_bot(line_height, y_bottom - rect_height(event_list_region)*0.5f);
        }
    }
}

internal void
log_parse_fill(Application_Links *app, Buffer_ID buffer){
    if (log_arena.base_allocator == 0){
        log_arena = make_arena_system();
    }
    
    linalloc_clear(&log_arena);
    block_zero_struct(&log_graph);
    log_filter_set_init(&log_filter_set);
    log_filter_set_init(&log_preview_set);
    
    String_Const_u8 log_text = push_whole_buffer(app, &log_arena, buffer);
    log_parse = make_log_parse(&log_arena, log_text);
}

internal void
log_graph_render__tag(Arena *arena, Fancy_Line *line,
                      Log_Parse *log, Log_Tag *tag){
    String_Const_u8 tag_name = log_parse__get_string(log, tag->name);
    push_fancy_stringf(arena, line, f_white, "[");
    push_fancy_string(arena, line, f_green, tag_name);
    push_fancy_stringf(arena, line, f_white, "=");
    if (tag->value.kind == LogTagKind_Integer){
        push_fancy_stringf(arena, line, f_pink, "0x%llx", tag->value.value_s);
    }
    else if (tag->value.kind == LogTagKind_String){
        String_Const_u8 value = log_parse__get_string(log, tag->value.value);
        push_fancy_string(arena, line, f_pink, value);
    }
    push_fancy_stringf(arena, line, f_white, "]");
}

internal void
log_graph_render(Application_Links *app, Frame_Info frame_info, View_ID view){
    if (log_parse.arena != 0){
        ////////////////////////////////
        View_ID active_view = get_active_view(app, Access_Always);
        b32 is_active_view = (active_view == view);
        
        Rect_f32 view_rect = view_get_screen_rect(app, view);
        Rect_f32 inner = rect_inner(view_rect, 3);
        draw_rectangle_fcolor(app, view_rect, 0.f,
                              get_item_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None));
        draw_rectangle_fcolor(app, inner, 0.f, fcolor_id(defcolor_back));
        
        Rect_f32 prev_clip = draw_set_clip(app, inner);
        ////////////////////////////////
        
        Face_ID face_id = get_face_id(app, 0);
        f32 y_scroll = log_graph.y_scroll;
        Log_Event *selected_event = log_graph.selected_event;
        if (!log_graph.holding_temp ||
            inner != log_graph.layout_region ||
            face_id != log_graph.face_id ||
            log_filter_set.alter_counter != log_graph.filter_alter_counter){
            log_graph_fill(app, inner, face_id);
        }
        log_graph.y_scroll = clamp(0.f, y_scroll, log_graph.max_y_scroll);
        log_graph.selected_event = selected_event;
        
        Mouse_State mouse = get_mouse_state(app);
        Vec2_f32 m_p = V2f32(mouse.p) - inner.p0;
        
        Face_Metrics metrics = get_face_metrics(app, log_graph.face_id);
        f32 line_height = metrics.line_height;
        
        Log_Event *hover_event = 0;
        
        b32 in_details_region = (rect_contains_point(log_graph.details_region, m_p));
        
        for (Log_Graph_Box *box_node = log_graph.first_box;
             box_node != 0;
             box_node = box_node->next){
            Scratch_Block scratch(app);
            
            Rect_f32 box = box_node->rect;
            box.y0 -= log_graph.y_scroll;
            box.y1 -= log_graph.y_scroll;
            
            Rect_f32 box_inner = rect_inner(box, 3.f);
            
            FColor margin_color = f_dark_gray;
            if (!in_details_region && hover_event == 0 && rect_contains_point(box, m_p)){
                margin_color = f_gray;
                hover_event = box_node->event;
            }
            if (box_node->event == log_graph.selected_event){
                margin_color = f_light_gray;
            }
            
            draw_rectangle_fcolor(app, box      , 0.f, margin_color);
            draw_rectangle_fcolor(app, box_inner, 0.f, f_black     );
            
            Log_Event *event = box_node->event;
            
            String_Const_u8 event_name = log_parse__get_string(&log_parse, event->event_name);
            Fancy_Line line = {};
            push_fancy_string(scratch, &line, f_white, event_name);
            
            for (Log_Filter *filter = log_preview_set.first;
                 filter != 0;
                 filter = filter->next){
                Table_u64_u64 *table = &event->tag_name_to_tag_ptr_table;
                Table_Lookup lookup = table_lookup(table, filter->tag_name_code);
                if (lookup.found_match){
                    u64 val = 0;
                    table_read(table, lookup, &val);
                    Log_Tag *tag = (Log_Tag*)IntAsPtr(val);
                    push_fancy_string(scratch, &line, string_u8_litexpr(" "));
                    log_graph_render__tag(scratch, &line, &log_parse, tag);
                }
            }
            
            
            Vec2_f32 p = V2f32(box_inner.x0 + 3.f,
                               (f32_round32((box_inner.y0 + box_inner.y1 - line_height)*0.5f)));
            draw_fancy_line(app, log_graph.face_id, fcolor_zero(), &line, p);
        }
        
        {
            Scratch_Block scratch(app);
            
            Rect_f32 box = log_graph.details_region;
            Rect_f32 box_inner = rect_inner(box, 3.f);
            
            Log_Graph_List_Tab current_tab = log_graph.tab;
            Log_Filter_Set *viewing_filter_set = log_filter_set_from_tab(current_tab);
            
            draw_rectangle_fcolor(app, box      , 0.f, f_dark_gray);
            draw_rectangle_fcolor(app, box_inner, 0.f, f_black    );
            
            {
                f32 y_cursor = box_inner.y0 + 3.f;
                if (y_cursor + line_height > box_inner.y1) goto finish_list_display;
                
                {
                    f32 x_cursor = box_inner.x0 + 3.f;
                    for (i32 i = LogTab_ERROR + 1; i < LogTab_COUNT; i += 1){
                        FColor color = (i == current_tab)?f_white:f_gray;
                        Fancy_Line line = {};
                        switch (i){
                            case LogTab_Filters:
                            {
                                push_fancy_stringf(scratch, &line, color, "filters");
                            }break;
                            case LogTab_Previews:
                            {
                                push_fancy_stringf(scratch, &line, color, "previews");
                            }break;
                        }
                        
                        Vec2_f32 p = V2f32(x_cursor, y_cursor);
                        f32 width = get_fancy_line_width(app, log_graph.face_id,
                                                         &line);
                        draw_fancy_line(app, log_graph.face_id, fcolor_zero(),
                                        &line, p);
                        x_cursor += width + metrics.normal_advance;
                        
                        if (log_graph.has_unused_click){
                            Rect_f32 click_rect = Rf32_xy_wh(p.x, p.y,
                                                             width, line_height);
                            if (rect_contains_point(click_rect, log_graph.unused_click)){
                                log_graph.has_unused_click = false;
                                log_graph.tab = i;
                            }
                        }
                    }
                }
                
                if (viewing_filter_set != 0){
                    for (Log_Filter *filter = viewing_filter_set->first, *next = 0;
                         filter != 0;
                         filter = next){
                        next = filter->next;
                        
                        y_cursor += line_height;
                        if (y_cursor + line_height > box_inner.y1) goto finish_list_display;
                        
                        Fancy_Line line = {};
                        
                        if (filter->kind == LogFilter_TagValue){
                            push_fancy_stringf(scratch, &line, f_white, "val  [");
                            String_Const_u8 tag_name = log_parse__get_string(&log_parse, filter->tag_name_code);
                            push_fancy_stringf(scratch, &line, f_green, "%.*s", string_expand(tag_name));
                            push_fancy_stringf(scratch, &line, f_white, "=");
                            if (filter->tag_value.kind == LogTagKind_Integer){
                                push_fancy_stringf(scratch, &line, f_pink, "0x%llx", filter->tag_value.value_s);
                            }
                            else if (filter->tag_value.kind == LogTagKind_String){
                                String_Const_u8 value = log_parse__get_string(&log_parse, filter->tag_value.value);
                                push_fancy_stringf(scratch, &line, f_pink, "%.*s", string_expand(value));
                            }
                            push_fancy_stringf(scratch, &line, f_white, "]");
                        }
                        else{
                            push_fancy_stringf(scratch, &line, f_white, "name [");
                            String_Const_u8 tag_name = log_parse__get_string(&log_parse, filter->tag_name_code);
                            push_fancy_stringf(scratch, &line, f_green, "%.*s", string_expand(tag_name));
                            push_fancy_stringf(scratch, &line, f_white, "]");
                        }
                        
                        Vec2_f32 p = V2f32(box_inner.x0 + 3.f, y_cursor);
                        f32 width = get_fancy_line_width(app, log_graph.face_id,
                                                         &line);
                        draw_fancy_line(app, log_graph.face_id, fcolor_zero(),
                                        &line, p);
                        
                        if (log_graph.has_unused_click){
                            Rect_f32 click_rect = Rf32_xy_wh(p.x, p.y,
                                                             width, line_height);
                            if (rect_contains_point(click_rect, log_graph.unused_click)){
                                log_graph.has_unused_click = false;
                                log_filter_set__free_filter(viewing_filter_set, filter);
                            }
                        }
                    }
                }
                
                finish_list_display:;
            }
            
            Log_Event *view_event = (hover_event!=0)?hover_event:log_graph.selected_event;
            if (view_event != 0){
                f32 y_cursor = box_inner.y0 + 3.f;
                if (y_cursor + line_height > box_inner.y1) goto finish_event_display;
                
                {
                    Fancy_Line line = {};
                    String_Const_u8 file_name = log_parse__get_string(&log_parse, view_event->src_file_name);
                    push_fancy_stringf(scratch, &line, f_green, "[%d]  ", view_event->event_number);
                    push_fancy_stringf(scratch, &line, f_white, "%.*s:", string_expand(file_name));
                    push_fancy_stringf(scratch, &line, f_pink, "%llu", view_event->line_number);
                    
                    Vec2_f32 right_p = V2f32(box_inner.x1 - 3.f, y_cursor);
                    f32 width = get_fancy_line_width(app, log_graph.face_id, &line);
                    Vec2_f32 p = V2f32(right_p.x - width, right_p.y);
                    draw_fancy_line(app, log_graph.face_id, fcolor_zero(), &line, p);
                }
                
                for (Log_Tag *tag = view_event->first_tag;
                     tag != 0;
                     tag = tag->next){
                    y_cursor += line_height;
                    if (y_cursor + line_height > box_inner.y1) goto finish_event_display;
                    
                    {
                        Fancy_Line line = {};
                        log_graph_render__tag(scratch, &line, &log_parse, tag);
                        
                        Vec2_f32 right_p = V2f32(box_inner.x1 - 3.f, y_cursor);
                        f32 width = get_fancy_line_width(app, log_graph.face_id, &line);
                        Vec2_f32 p = V2f32(right_p.x - width, right_p.y);
                        draw_fancy_line(app, log_graph.face_id, fcolor_zero(),
                                        &line, p);
                        
                        if (log_graph.has_unused_click){
                            Rect_f32 click_rect = Rf32(p.x, p.y, right_p.x, p.y + line_height);
                            if (rect_contains_point(click_rect, log_graph.unused_click)){
                                log_graph.has_unused_click = false;
                                Log_Filter filter = {};
                                switch (log_graph.tab){
                                    case LogTab_Filters:
                                    {
                                        filter.kind = LogFilter_TagValue;
                                        filter.tag_name_code = tag->name;
                                        filter.tag_value = tag->value;
                                    }break;
                                    case LogTab_Previews:
                                    {
                                        filter.kind = LogFilter_Tag;
                                        filter.tag_name_code = tag->name;
                                    }break;
                                }
                                if (filter.kind != LogTab_ERROR){
                                    log_filter_set__new_filter(viewing_filter_set, &filter);
                                    animate_in_n_milliseconds(app, 0);
                                }
                            }
                        }
                    }
                }
                
                finish_event_display:;
            }
        }
        
        log_graph.has_unused_click = false;
        draw_set_clip(app, prev_clip);
    }
}

internal Log_Graph_Box*
log_graph__get_box_at_point(Log_Graph *graph, Vec2_f32 p){
    Log_Graph_Box *result = 0;
    if (!rect_contains_point(graph->details_region, p)){
        for (Log_Graph_Box *box_node = graph->first_box;
             box_node != 0;
             box_node = box_node->next){
            Rect_f32 box = box_node->rect;
            box.y0 -= graph->y_scroll;
            box.y1 -= graph->y_scroll;
            if (rect_contains_point(box, p)){
                result = box_node;
                break;
            }
        }
    }
    return(result);
}

internal Log_Graph_Box*
log_graph__get_box_at_mouse_point(Application_Links *app, Log_Graph *graph){
    Mouse_State mouse = get_mouse_state(app);
    Vec2_f32 m_p = V2f32(mouse.p) - graph->layout_region.p0;
    return(log_graph__get_box_at_point(graph, m_p));
}

function void
log_graph__click_select_event(Application_Links *app, Vec2_f32 m_p)
{
    if (log_view != 0 && log_graph.holding_temp){
        Log_Graph_Box *box_node = log_graph__get_box_at_point(&log_graph, m_p);
        if (box_node != 0){
            log_graph.selected_event = box_node->event;
        }
        else{
            log_graph.has_unused_click = true;
            log_graph.unused_click = m_p;
        }
    }
}

function void
log_graph__click_jump_to_event_source(Application_Links *app, Vec2_f32 m_p){
    if (log_view != 0 && log_graph.holding_temp){
        Log_Graph_Box *box_node = log_graph__get_box_at_point(&log_graph, m_p);
        if (box_node != 0){
            Log_Event *event = box_node->event;
            log_graph.selected_event = event;
            
            View_ID target_view = get_next_view_looped_primary_panels(app, log_view,
                                                                      Access_ReadVisible);
            if (target_view != 0){
                String_Const_u8 file_name = log_parse__get_string(&log_parse, event->src_file_name);
                Buffer_ID target_buffer = get_buffer_by_file_name(app, file_name, Access_Always);
                if (target_buffer == 0){
                    target_buffer = get_buffer_by_name(app, file_name, Access_Always);
                }
                if (target_buffer != 0){
                    set_view_to_location(app, target_view, target_buffer,
                                         seek_line_col(event->line_number, 1));
                }
            }
        }
        else{
            log_graph.has_unused_click = true;
            log_graph.unused_click = m_p;
        }
    }
}

CUSTOM_UI_COMMAND_SIG(show_the_log_graph)
CUSTOM_DOC("Parses *log* and displays the 'log graph' UI")
{
    if (log_view != 0){
        return;
    }
    
    Buffer_ID log_buffer = get_buffer_by_name(app, string_u8_litexpr("*log*"), Access_Always);
    log_parse_fill(app, log_buffer);
    
    log_view = get_this_ctx_view(app, Access_Always);
    View_ID view = log_view;
    
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = log_graph_render;
    View_Context_Block ctx_block(app, view, &ctx);
    
    for (;;){
        User_Input in = get_next_input(app, EventPropertyGroup_AnyUserInput, KeyCode_Escape);
        if (in.abort){
            view = 0;
            break;
        }
        
        b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_KeyStroke:
            {
                switch (in.event.key.code){
                    case KeyCode_PageUp:
                    {
                        log_graph.y_scroll -= get_page_jump(app, view);
                    }break;
                    
                    case KeyCode_PageDown:
                    {
                        log_graph.y_scroll += get_page_jump(app, view);
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            case InputEventKind_MouseButton:
            {
                Vec2_f32 m_p = V2f32(in.event.mouse.p) - log_graph.layout_region.p0;
                switch (in.event.mouse.code){
                    case MouseCode_Left:
                    {
                        log_graph__click_jump_to_event_source(app, m_p);
                    }break;
                    
                    case MouseCode_Right:
                    {
                        log_graph__click_select_event(app, m_p);
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            case InputEventKind_MouseWheel:
            {
                f32 value = in.event.mouse_wheel.value;
                log_graph.y_scroll += f32_round32(value);
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
    
    log_view = 0;
}

// BOTTOM

