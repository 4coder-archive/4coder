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
        Data data = make_data(string.str, string.size);
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
        Data val = {};
        table_read(&parse->id_to_string_table, lookup, &val);
        result = SCu8(val.data, val.size);
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
	Data data_key = make_data_struct(&key);
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
        umem end_of_line = string_find_first(source, '\n');
        String_Const_u8 line = string_prefix(source, end_of_line);
        line = string_skip_chop_whitespace(line);
        source = string_skip(source, end_of_line + 1);
        
        String_Const_u8 src_file_name = {};
        String_Const_u8 src_line_number = {};
        b32 got_source_position = false;
        
        String_Const_u8 whole_line = line;
        
        {
            umem colon1 = string_find_first(line, ':');
            src_file_name = string_prefix(line, colon1);
            line = string_skip(line, colon1 + 1);
            
            umem colon2 = string_find_first(line, ':');
            src_line_number = string_prefix(line, colon2);
            line = string_skip(line, colon2 + 1);
            
            if (string_is_integer(src_line_number, 10)){
                got_source_position = true;
            }
        }
        
        if (!got_source_position){
            line = whole_line;
            
            umem colon0 = string_find_first(line, ':');
            umem colon1 = string_find_first(line, colon0 + 1, ':');
            src_file_name = string_prefix(line, colon1);
            line = string_skip(line, colon1 + 1);
            
            umem colon2 = string_find_first(line, ':');
            src_line_number = string_prefix(line, colon2);
            line = string_skip(line, colon2 + 1);
            
            if (string_is_integer(src_line_number, 10)){
                got_source_position = true;
            }
        }
        
        if (got_source_position){
            umem bracket_open = string_find_first(line, '[');
            String_Const_u8 event_name = string_prefix(line, bracket_open);
            event_name = string_skip_chop_whitespace(event_name);
            line = string_skip(line, bracket_open + 1);
            
            Log_Event *event = log_parse__event(&parse,
                                                src_file_name, src_line_number, event_name);
            
            for (;line.size > 0;){
                umem bracket_close = string_find_first(line, ']');
                String_Const_u8 tag = string_prefix(line, bracket_close);
                line = string_skip(line, bracket_close + 1);
                bracket_open = string_find_first(line, '[');
                line = string_skip(line, bracket_open + 1);
                
                umem equal_sign = string_find_first(tag, '=');
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

CUSTOM_COMMAND_SIG(parse_the_log)
CUSTOM_DOC("Tests the log parser")
{
    Buffer_ID log_buffer = get_buffer_by_name(app, string_u8_litexpr("*log*"), AccessAll);
    Scratch_Block scratch(app);
    String_Const_u8 log_text = push_whole_buffer(app, scratch, log_buffer);
    Log_Parse parse = make_log_parse(scratch, log_text);
    
    u64 buffer_code = log_parse__string_code(&parse, string_u8_litexpr("buffer"),
                                             LogParse_ExternalString);
    u64 thread_code = log_parse__string_code(&parse, string_u8_litexpr("thread"),
                                             LogParse_ExternalString);
    
    Log_Tag_Value value = {};
    value.kind = LogTagKind_Integer;
    value.value_s = 10;
    Log_Event_List *list = log_parse_get_list_tag_value(&parse, buffer_code, value);
    
    Log_Event_Ptr_Array array = log_event_array_from_list(scratch, *list);
    log_events_sort_by_tag(scratch, array, thread_code);
    
    for (i32 i = 0; i < array.count; i += 1){
        Log_Event *event = array.events[i];
        String_Const_u8 src_name = log_parse__get_string(&parse, event->src_file_name);
        String_Const_u8 event_name = log_parse__get_string(&parse, event->event_name);
        u64 line_number = event->line_number;
        
        List_String_Const_u8 line = {};
        string_list_pushf(scratch, &line, "%.*s:%llu: %.*s",
                          string_expand(src_name), line_number, string_expand(event_name));
        
        for (Log_Tag *node = event->first_tag;
             node != 0;
             node = node->next){
            String_Const_u8 tag_name = log_parse__get_string(&parse, node->name);
            
            switch (node->value.kind){
                case LogTagKind_Integer:
                {
                    string_list_pushf(scratch, &line, " [%.*s:%lld]",
                                      string_expand(tag_name), node->value.value_s);
                }break;
                
                case LogTagKind_String:
                {
                    String_Const_u8 string_value = log_parse__get_string(&parse, node->value.value);
                    string_list_pushf(scratch, &line, " [%.*s:%.*s]",
                                      string_expand(tag_name), string_expand(string_value));
                }break;
            }
        }
        
        string_list_push(scratch, &line, string_u8_litexpr("\n"));
        
        String_Const_u8 line_string = string_list_flatten(scratch, line);
        print_message(app, line_string);
    }
}


// BOTTOM

