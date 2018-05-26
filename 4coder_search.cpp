/*
4coder_search.cpp - Commands that search accross buffers including word complete,
and list all locations.
*/

// TOP

//
// Search Iteration Systems
//

static void
search_key_alloc(General_Memory *general, Search_Key *key, int32_t *size, int32_t count){
    if (count > ArrayCount(key->words)){
        count = ArrayCount(key->words);
    }
    
    int32_t min_size = 0x7FFFFFFF;
    int32_t total_size = 0;
    for (int32_t i = 0; i < count; ++i){
        total_size += size[i];
        if (min_size > size[i]){
            min_size = size[i];
        }
    }
    
    int32_t max_base_size = total_size*2;
    
    if (key->base == 0){
        key->base = (char*)general_memory_allocate(general, max_base_size);
        key->base_size = max_base_size;
    }
    else if (key->base_size < total_size){
        key->base  = (char*)general_memory_reallocate_nocopy(general, key->base, max_base_size);
        key->base_size = max_base_size;
    }
    
    char *char_ptr = key->base;
    for (int32_t i = 0; i < count; ++i){
        key->words[i].str = char_ptr;
        key->words[i].size = 0;
        key->words[i].memory_size = size[i];
        char_ptr += size[i];
    }
    
    key->count = count;
    key->min_size = min_size;
}

static void
search_iter_init(Search_Iter *iter, Search_Key key){
    iter->key = key;
    iter->i = 0;
    iter->range_initialized = 0;
}

static void
search_set_init(General_Memory *general, Search_Set *set, int32_t range_count){
    int32_t max = range_count*2;
    
    if (set->ranges == 0){
        set->ranges = (Search_Range*)general_memory_allocate(general, sizeof(Search_Range)*max);
        set->max = max;
    }
    else if (set->max < range_count){
        set->ranges = (Search_Range*)general_memory_reallocate_nocopy(
            general, set->ranges, sizeof(Search_Range)*max);
        set->max = max;
    }
    
    set->count = range_count;
}

static void
search_hits_table_alloc(General_Memory *general, Table *hits, int32_t table_size){
    void *mem = 0;
    int32_t mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
    if (hits->hash_array == 0){
        mem = general_memory_allocate(general, mem_size);
    }
    else{
        mem = general_memory_reallocate_nocopy(general, hits->hash_array, mem_size);
    }
    table_init_memory(hits, mem, table_size, sizeof(Offset_String));
}

static void
search_hits_init(General_Memory *general, Table *hits, String_Space *str, int32_t table_size, int32_t str_size){
    if (hits->hash_array == 0){
        search_hits_table_alloc(general, hits, table_size);
    }
    else{
        int32_t mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
        void *mem = general_memory_reallocate_nocopy(general, hits->hash_array, mem_size);
        table_init_memory(hits, mem, table_size, sizeof(Offset_String));
    }
    
    if (str->space == 0){
        str->space = (char*)general_memory_allocate(general, str_size);
        str->max = str_size;
    }
    else if (str->max < str_size){
        str->space = (char*)general_memory_reallocate_nocopy(general, str->space, str_size);
        str->max = str_size;
    }
    
    str->pos = str->new_pos = 0;
    table_clear(hits);
}

//
// Table Operations
//

static int32_t
search_hit_add(General_Memory *general, Table *hits, String_Space *space, char *str, int32_t len){
    int32_t result = false;
    
    Assert(len != 0);
    
    Offset_String ostring = strspace_append(space, str, len);
    if (ostring.size == 0){
        int32_t new_size = space->max*2;
        if (new_size < space->max + len){
            new_size = space->max + len;
        }
        space->space = (char*)general_memory_reallocate(general, space->space, space->new_pos, new_size);
        ostring = strspace_append(space, str, len);
    }
    
    Assert(ostring.size != 0);
    
    if (table_at_capacity(hits)){
        Table new_hits = {0};
        search_hits_table_alloc(general, &new_hits, hits->max*2);
        table_clear(&new_hits);
        table_rehash(hits, &new_hits, space->space, tbl_offset_string_hash, tbl_offset_string_compare);
        general_memory_free(general, hits->hash_array);
        *hits = new_hits;
    }
    
    if (!table_add(hits, &ostring, space->space, tbl_offset_string_hash, tbl_offset_string_compare)){
        result = true;
        strspace_keep_prev(space);
    }
    else{
        strspace_discard_prev(space);
    }
    
    return(result);
}

//
// Search Key Checking
//

static void
seek_potential_match(Application_Links *app, Search_Range *range, Search_Key key, Search_Match *result, Seek_Potential_Match_Direction direction, int32_t start_pos, int32_t end_pos){
    bool32 case_insensitive = ((range->flags & SearchFlag_CaseInsensitive) != 0);
    bool32 forward = (direction == SeekPotentialMatch_Forward);
#define OptFlag(b,f) ((b)?(f):(0))
    Buffer_Seek_String_Flags flags = 0
        | OptFlag(case_insensitive, BufferSeekString_CaseInsensitive)
        | OptFlag(!forward, BufferSeekString_Backward);
    result->buffer = get_buffer(app, range->buffer, AccessAll);
    
    int32_t best_pos = -1;
    if (forward){
        best_pos = end_pos;
    }
    
    for (int32_t i = 0; i < key.count; ++i){
        String word = key.words[i];
        int32_t new_pos = -1;
        buffer_seek_string(app, &result->buffer, start_pos, end_pos, range->start, word.str, word.size, &new_pos, flags);
        
        if (new_pos >= 0){
            if (forward){
                if (new_pos < best_pos){
                    best_pos = new_pos;
                }
            }
            else{
                if (new_pos > best_pos){
                    best_pos = new_pos;
                }
            }
        }
    }
    
    result->start = best_pos;
}

static int32_t
buffer_seek_alpha_numeric_end(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char space[1024];
    Stream_Chunk chunk = {0};
    if (init_stream_chunk(&chunk, app, buffer, pos, space, sizeof(space))){
        int32_t still_looping = true;
        do{
            for (; pos < chunk.end; ++pos){
                uint8_t at_pos = (uint8_t)chunk.data[pos];
                if (!char_is_alpha_numeric_utf8(at_pos)) goto double_break;
            }
            still_looping = forward_stream_chunk(&chunk);
        }while(still_looping);
    }
    double_break:;
    return(pos);
}

static int32_t
match_check(Application_Links *app, Search_Range *range, int32_t *pos, Search_Match *result_ptr, Search_Key key){
    int32_t result_code = FindResult_None;
    
    Search_Match result = *result_ptr;
    int32_t end_pos = range->start + range->size;
    
    int32_t type = (range->flags & SearchFlag_MatchMask);
    result.match_word_index = -1;
    
    for (int32_t i = 0; i < key.count; ++i){
        String word = key.words[i];
        
        int32_t found_match = FindResult_None;
        switch (type){
            case SearchFlag_MatchWholeWord:
            {
                char prev = ' ';
                if (char_is_alpha_numeric_utf8(word.str[0])){
                    prev = buffer_get_char(app, &result.buffer, result.start - 1);
                }
                
                if (!char_is_alpha_numeric_utf8(prev)){
                    result.end = result.start + word.size;
                    if (result.end <= end_pos){
                        char next = ' ';
                        if (char_is_alpha_numeric_utf8(word.str[word.size-1])){
                            next = buffer_get_char(app, &result.buffer, result.end);
                        }
                        
                        if (!char_is_alpha_numeric_utf8(next)){
                            result.found_match = true;
                            found_match = FindResult_FoundMatch;
                        }
                    }
                    else{
                        found_match = FindResult_PastEnd;
                    }
                }
            }break;
            
            case SearchFlag_MatchWordPrefix:
            {
                char prev = buffer_get_char(app, &result.buffer, result.start - 1);
                if (!char_is_alpha_numeric_utf8(prev)){
                    result.end =
                        buffer_seek_alpha_numeric_end(app, &result.buffer, result.start);
                    
                    if (result.end <= end_pos){
                        result.found_match = true;
                        found_match = FindResult_FoundMatch;
                    }
                    else{
                        found_match = FindResult_PastEnd;
                    }
                }
            }break;
            
            case SearchFlag_MatchSubstring:
            {
                result.end = result.start + word.size;
                if (result.end <= end_pos){
                    result.found_match = true;
                    found_match = FindResult_FoundMatch;
                }
                else{
                    found_match = FindResult_PastEnd;
                }
            }break;
        }
        
        if (found_match == FindResult_FoundMatch){
            result_code = FindResult_FoundMatch;
            result.match_word_index = i;
            break;
        }
        else if (found_match == FindResult_PastEnd){
            result_code = FindResult_PastEnd;
        }
    }
    
    *result_ptr = result;
    
    return(result_code);
}

//
// Find Next Match
//

static int32_t
search_front_to_back(Application_Links *app, Search_Range *range, Search_Key key, int32_t *pos, Search_Match *result){
    int32_t found_match = FindResult_None;
    for (;found_match == FindResult_None;){
        found_match = FindResult_None;
        
        int32_t end_pos = range->start + range->size;
        if (*pos + key.min_size < end_pos){
            int32_t start_pos = *pos;
            if (start_pos < range->start){
                start_pos = range->start;
            }
            
            seek_potential_match(app, range, key, result, SeekPotentialMatch_Forward, start_pos, end_pos);
            
            if (result->start < end_pos){
                *pos = result->start + 1;
                found_match = match_check(app, range, pos, result, key);
                if (found_match == FindResult_FoundMatch){
                    *pos = result->end;
                }
            }
            else{
                found_match = FindResult_PastEnd;
                *pos = end_pos + 1;
            }
        }
        else{
            found_match = FindResult_PastEnd;
            *pos = end_pos + 1;
        }
    }
    return(found_match);
}

static int32_t
search_back_to_front(Application_Links *app, Search_Range *range, Search_Key key, int32_t *pos, Search_Match *result){
    int32_t found_match = FindResult_None;
    for (;found_match == FindResult_None;){
        found_match = FindResult_None;
        if (*pos > range->start){
            int32_t start_pos = *pos;
            
            seek_potential_match(app, range, key, result, SeekPotentialMatch_Backward, start_pos, 0);
            
            if (result->start >= range->start){
                *pos = result->start - 1;
                found_match = match_check(app, range, pos, result, key);
                if (found_match == FindResult_FoundMatch){
                    *pos = result->start - key.words[result->match_word_index].size;
                }
            }
            else{
                found_match = FindResult_PastEnd;
            }
        }
        else{
            found_match = FindResult_PastEnd;
        }
    }
    return(found_match);
}

static void
search_iter_next_range(Search_Iter *it){
    ++it->i;
    it->pos = 0;
    it->back_pos = 0;
    it->range_initialized = 0;
}

static Search_Match
search_next_match(Application_Links *app, Search_Set *set, Search_Iter *it_ptr){
    Search_Match result = {0};
    Search_Iter iter = *it_ptr;
    
    int32_t count = set->count;
    for (;iter.i < count;){
        Search_Range *range = set->ranges + iter.i;
        
        int32_t find_result = FindResult_None;
        
        if (!iter.range_initialized){
            iter.range_initialized = true;
            switch (range->type){
                case SearchRange_BackToFront:
                {
                    iter.back_pos = range->start + range->size-1;
                }break;
                
                case SearchRange_Wave:
                {
                    iter.back_pos = range->mid_start-1;
                    iter.pos = range->mid_start + range->mid_size;
                }break;
            }
        }
        
        switch (range->type){
            case SearchRange_FrontToBack:
            {
                find_result = search_front_to_back(app, range, iter.key, &iter.pos, &result);
            }break;
            
            case SearchRange_BackToFront:
            {
                find_result = search_back_to_front(app, range, iter.key, &iter.back_pos, &result);
            }break;
            
            case SearchRange_Wave:
            {
                Search_Match forward_match = {0};
                Search_Match backward_match = {0};
                
                int32_t forward_result = FindResult_PastEnd;
                int32_t backward_result = FindResult_PastEnd;
                
                if (iter.pos < range->start + range->size){
                    forward_result = search_front_to_back(app, range, iter.key, &iter.pos, &forward_match);
                }
                
                if (iter.back_pos > range->start){
                    backward_result = search_back_to_front(app, range, iter.key, &iter.back_pos, &backward_match);
                }
                
                if (forward_result == FindResult_FoundMatch){
                    if (backward_result == FindResult_FoundMatch){
                        find_result = FindResult_FoundMatch;
                        
                        int32_t forward_start = range->mid_start + range->mid_size;
                        int32_t forward_distance = (forward_match.start - forward_start);
                        int32_t backward_distance = (range->mid_start - backward_match.end);
                        
                        if (backward_distance < forward_distance){
                            iter.pos = forward_match.start;
                            result = backward_match;
                        }
                        else{
                            iter.back_pos = backward_match.start;
                            result = forward_match;
                        }
                    }
                    else{
                        find_result = FindResult_FoundMatch;
                        result = forward_match;
                    }
                }
                else{
                    if (backward_result == FindResult_FoundMatch){
                        find_result = FindResult_FoundMatch;
                        result = backward_match;
                        --iter.pos;
                    }
                    else{
                        find_result = FindResult_PastEnd;
                    }
                }
            }break;
        }
        
        if (find_result == FindResult_FoundMatch){
            goto double_break;
        }
        else if (find_result == FindResult_PastEnd){
            search_iter_next_range(&iter);
        }
    }
    double_break:;
    
    *it_ptr = iter;
    
    return(result);
} 

//
// Generic Search All Buffers
//

static void
initialize_generic_search_all_buffers(Application_Links *app, General_Memory *general, String *strings, int32_t count, Search_Range_Flag match_flags, int32_t *skip_buffers, int32_t skip_buffer_count, Search_Set *set, Search_Iter *iter){
    memset(set, 0, sizeof(*set));
    memset(iter, 0, sizeof(*iter));
    
    Search_Key key = {0};
    int32_t sizes[ArrayCount(key.words)];
    memset(sizes, 0, sizeof(sizes));
    
    if (count > ArrayCount(key.words)){
        count = ArrayCount(key.words);
    }
    for (int32_t i = 0; i < count; ++i){
        sizes[i] = strings[i].size;
    }
    
    // TODO(allen): Why on earth am I allocating these separately in this case?  Upgrade to just use the string array on the stack!
    search_key_alloc(general, &key, sizes, count);
    for (int32_t i = 0; i < count; ++i){
        copy_ss(&key.words[i], strings[i]);
    }
    
    search_iter_init(iter, key);
    
    int32_t buffer_count = get_buffer_count(app);
    search_set_init(general, set, buffer_count);
    
    Search_Range *ranges = set->ranges;
    
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    int32_t j = 0;
    if (buffer.exists){
        bool32 skip = false;
        for (int32_t i = 0; i < skip_buffer_count; ++i){
            if (buffer.buffer_id == skip_buffers[i]){
                skip = true;
                break;
            }
        }
        
        if (!skip){
            ranges[j].type = SearchRange_FrontToBack;
            ranges[j].flags = match_flags;
            ranges[j].buffer = buffer.buffer_id;
            ranges[j].start = 0;
            ranges[j].size = buffer.size;
            ++j;
        }
    }
    
    for (Buffer_Summary buffer_it = get_buffer_first(app, AccessAll);
         buffer_it.exists;
         get_buffer_next(app, &buffer_it, AccessAll)){
        if (buffer_it.buffer_id == buffer.buffer_id){
            continue;
        }
        
        bool32 skip = false;
        for (int32_t i = 0; i < skip_buffer_count; ++i){
            if (buffer_it.buffer_id == skip_buffers[i]){
                skip = true;
                break;
            }
        }
        
        if (!skip){
            if (buffer_it.buffer_name[0] != '*'){
                ranges[j].type = SearchRange_FrontToBack;
                ranges[j].flags = match_flags;
                ranges[j].buffer = buffer_it.buffer_id;
                ranges[j].start = 0;
                ranges[j].size = buffer_it.size;
                ++j;
            }
        }
        
    }
    
    set->count = j;
}

////////////////////////////////

static void
buffered_print_flush(Application_Links *app, Partition *part, Temp_Memory temp, Buffer_Summary *output_buffer){
    int32_t size = output_buffer->size;
    int32_t write_size = part->pos - temp.pos;
    char *str = part->base + temp.pos;
    buffer_replace_range(app, output_buffer, size, size, str, write_size);
}

static void
buffered_print_match_jump_line(Application_Links *app, Partition *part, Temp_Memory temp, Partition *line_part, Buffer_Summary *output_buffer, Buffer_Summary *match_buffer, Partial_Cursor word_pos){
    char *file_name = match_buffer->buffer_name;
    int32_t file_len = match_buffer->buffer_name_len;
    
    int32_t line_num_len = int_to_str_size(word_pos.line);
    int32_t column_num_len = int_to_str_size(word_pos.character);
    
    Temp_Memory line_temp = begin_temp_memory(line_part);
    String line_str = {0};
    if (read_line(app, line_part, match_buffer, word_pos.line, &line_str)){
        line_str = skip_chop_whitespace(line_str);
        
        int32_t str_len = file_len + 1 + line_num_len + 1 + column_num_len + 1 + 1 + line_str.size + 1;
        
        char *spare = push_array(part, char, str_len);
        
        if (spare == 0){
            buffered_print_flush(app, part, temp, output_buffer);
            
            end_temp_memory(temp);
            temp = begin_temp_memory(part);
            
            spare = push_array(part, char, str_len);
        }
        
        String out_line = make_string_cap(spare, 0, str_len);
        append_ss(&out_line, make_string(file_name, file_len));
        append_s_char(&out_line, ':');
        append_int_to_str(&out_line, word_pos.line);
        append_s_char(&out_line, ':');
        append_int_to_str(&out_line, word_pos.character);
        append_s_char(&out_line, ':');
        append_s_char(&out_line, ' ');
        append_ss(&out_line, line_str);
        append_s_char(&out_line, '\n');
        Assert(out_line.size == str_len);
    }
    
    end_temp_memory(line_temp);
}

static void
list__parameters(Application_Links *app, General_Memory *general, Partition *scratch,
                 String *strings, int32_t count, Search_Range_Flag match_flags){
    // Open the search buffer
    String search_name = make_lit_string("*search*");
    Buffer_Summary search_buffer = get_buffer_by_name(app, search_name.str, search_name.size, AccessAll);
    if (!search_buffer.exists){
        search_buffer = create_buffer(app, search_name.str, search_name.size, BufferCreate_AlwaysNew);
        buffer_set_setting(app, &search_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, &search_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, &search_buffer, BufferSetting_WrapLine, false);
    }
    else{
        buffer_send_end_signal(app, &search_buffer);
        buffer_replace_range(app, &search_buffer, 0, search_buffer.size, 0, 0);
    }
    
    // Initialize a generic search all buffers
    Search_Set set = {0};
    Search_Iter iter = {0};
    initialize_generic_search_all_buffers(app, general, strings, count, match_flags, &search_buffer.buffer_id, 1, &set, &iter);
    
    // List all locations into search buffer
    Temp_Memory all_temp = begin_temp_memory(scratch);
    Partition line_part = partition_sub_part(scratch, (4 << 10));
    Temp_Memory temp = begin_temp_memory(scratch);
    for (Search_Match match = search_next_match(app, &set, &iter);
         match.found_match;
         match = search_next_match(app, &set, &iter)){
        Partial_Cursor word_pos = {0};
        if (buffer_compute_cursor(app, &match.buffer, seek_pos(match.start), &word_pos)){
            buffered_print_match_jump_line(app, scratch, temp, &line_part, &search_buffer, &match.buffer, word_pos);
        }
    }
    
    buffered_print_flush(app, scratch, temp, &search_buffer);
    end_temp_memory(all_temp);
    
    // Lock this *search* as the jump buffer
    View_Summary view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, search_buffer.buffer_id, 0);
    lock_jump_buffer(search_name.str, search_name.size);
}

static void
list_single__parameters(Application_Links *app, General_Memory *general, Partition *scratch,
                        String str, bool32 substrings, bool32 case_insensitive){
    Search_Range_Flag flags = 0;
    if (substrings){
        flags |= SearchFlag_MatchSubstring;
    }
    else{
        flags |= SearchFlag_MatchWholeWord;
    }
    if (case_insensitive){
        flags |= SearchFlag_CaseInsensitive;
    }
    list__parameters(app, general, scratch, &str, 1, flags);
}

static void
list_query__parameters(Application_Links *app, General_Memory *general, Partition *scratch,
                       bool32 substrings, bool32 case_insensitive){
    char space[1024];
    String str = get_query_string(app, "List Locations For: ", space, sizeof(space));
    if (str.str != 0){
        change_active_panel(app);
        list_single__parameters(app, general, scratch, str, substrings, case_insensitive);
    }
}

static void
list_identifier__parameters(Application_Links *app, General_Memory *general, Partition *scratch,
                            bool32 substrings, bool32 case_insensitive){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    if (!buffer.exists) return;
    char space[512];
    String str = get_token_or_word_under_pos(app, &buffer, view.cursor.pos, space, sizeof(space));
    if (str.str != 0){
        change_active_panel(app);
        list_single__parameters(app, general, scratch, str, substrings, case_insensitive);
    }
}

static void
list_selected_range__parameters(Application_Links *app, General_Memory *general, Partition *scratch,
                                bool32 substrings, bool32 case_insensitive){
    View_Summary view = get_active_view(app, AccessProtected);
    Temp_Memory temp = begin_temp_memory(scratch);
    String str = get_string_in_view_range(app, scratch, &view);
    if (str.str != 0){
        change_active_panel(app);
        list_single__parameters(app, general, scratch, str, substrings, case_insensitive);
    }
    end_temp_memory(temp);
}

static void
list_type_definition__parameters(Application_Links *app, General_Memory *general, Partition *scratch,
                                 String str){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    String match_strings[6];
    match_strings[0] = build_string(scratch, "struct ", str, "{");
    match_strings[1] = build_string(scratch, "struct ", str, "\n{");
    match_strings[2] = build_string(scratch, "union " , str, "{");
    match_strings[3] = build_string(scratch, "union " , str, "\n{");
    match_strings[4] = build_string(scratch, "enum "  , str, "{");
    match_strings[5] = build_string(scratch, "enum "  , str, "\n{");
    
    list__parameters(app, general, scratch,
                     match_strings, ArrayCount(match_strings), 0);
    
    end_temp_memory(temp);
    
#if 0    
    Buffer_Summary buffer = get_buffer_by_name(app, literal("*search*"), AccessAll);
    if (buffer.line_count == 2){
        goto_first_jump_same_panel_sticky(app);
    }
#endif
}

////////////////////////////////

CUSTOM_COMMAND_SIG(list_all_locations)
CUSTOM_DOC("Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.")
{
    list_query__parameters(app, &global_general, &global_part, false, false);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations)
CUSTOM_DOC("Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.")
{
    list_query__parameters(app, &global_general, &global_part, true, false);
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.")
{
    list_query__parameters(app, &global_general, &global_part, false, true);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.")
{
    list_query__parameters(app, &global_general, &global_part, true, true);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.")
{
    list_identifier__parameters(app, &global_general, &global_part, false, false);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.")
{
    list_identifier__parameters(app, &global_general, &global_part, false, true);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.")
{
    list_selected_range__parameters(app, &global_general, &global_part, false, false);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.")
{
    list_selected_range__parameters(app, &global_general, &global_part, false, true);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition)
CUSTOM_DOC("Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.")
{
    char space[1024];
    String str = get_query_string(app, "List Definitions For: ", space, sizeof(space));
    if (str.str != 0){
        change_active_panel(app);
        list_type_definition__parameters(app, &global_general, &global_part, str);
    }
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    char space[512];
    String str = get_token_or_word_under_pos(app, &buffer, view.cursor.pos, space, sizeof(space) - 1);
    if (str.str != 0){
        change_active_panel(app);
        list_type_definition__parameters(app, &global_general, &global_part, str);
    }
}


//
// Word Complete Command
//

static Word_Complete_State complete_state = {0};

CUSTOM_COMMAND_SIG(word_complete)
CUSTOM_DOC("Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    // NOTE(allen): I just do this because this command is a lot of work
    // and there is no point in doing any of it if nothing will happen anyway.
    if (buffer.exists){
        int32_t do_init = false;
        
        if (view_paste_index[view.view_id].rewrite != RewriteWordComplete){
            do_init = true;
        }
        view_paste_index[view.view_id].next_rewrite = RewriteWordComplete;
        if (!complete_state.initialized){
            do_init = true;
        }
        
        int32_t word_end = 0;
        int32_t word_start = 0;
        int32_t cursor_pos = 0;
        int32_t size = 0;
        
        if (do_init){
            // NOTE(allen): Get the range where the
            // partial word is written.
            word_end = view.cursor.pos;
            word_start = word_end;
            cursor_pos = word_end - 1;
            
            char space[1024];
            Stream_Chunk chunk = {0};
            if (init_stream_chunk(&chunk, app, &buffer, cursor_pos, space, sizeof(space))){
                int32_t still_looping = true;
                do{
                    for (; cursor_pos >= chunk.start; --cursor_pos){
                        char c = chunk.data[cursor_pos];
                        if (char_is_alpha_utf8(c)){
                            word_start = cursor_pos;
                        }
                        else if (!char_is_numeric(c)){
                            goto double_break;
                        }
                    }
                    still_looping = backward_stream_chunk(&chunk);
                }while(still_looping);
            }
            double_break:;
            
            size = word_end - word_start;
            
            if (size == 0){
                complete_state.initialized = false;
                return;
            }
            
            // NOTE(allen): Initialize the search iterator with the partial word.
            complete_state.initialized = true;
            Search_Key key = {0};
            search_key_alloc(&global_general, &key, &size, 1);
            buffer_read_range(app, &buffer, word_start, word_end, key.words[0].str);
            key.words[0].size = size;
            
            search_iter_init(&complete_state.iter, key);
            
            // NOTE(allen): Initialize the set of ranges to be searched.
            int32_t buffer_count = get_buffer_count(app);
            search_set_init(&global_general, &complete_state.set, buffer_count);
            
            Search_Range *ranges = complete_state.set.ranges;
            ranges[0].type = SearchRange_Wave;
            ranges[0].flags = SearchFlag_MatchWordPrefix;
            ranges[0].buffer = buffer.buffer_id;
            ranges[0].start = 0;
            ranges[0].size = buffer.size;
            ranges[0].mid_start = word_start;
            ranges[0].mid_size = size;
            
            int32_t j = 1;
            for (Buffer_Summary buffer_it = get_buffer_first(app, AccessAll);
                 buffer_it.exists;
                 get_buffer_next(app, &buffer_it, AccessAll)){
                if (buffer.buffer_id != buffer_it.buffer_id){
                    ranges[j].type = SearchRange_FrontToBack;
                    ranges[j].flags = SearchFlag_MatchWordPrefix;
                    ranges[j].buffer = buffer_it.buffer_id;
                    ranges[j].start = 0;
                    ranges[j].size = buffer_it.size;
                    ++j;
                }
            }
            complete_state.set.count = j;
            
            // NOTE(allen): Initialize the search hit table.
            search_hits_init(&global_general, &complete_state.hits, &complete_state.str, 100, (4 << 10));
            String word = complete_state.iter.key.words[0];
            search_hit_add(&global_general, &complete_state.hits, &complete_state.str, word.str, word.size);
            
            complete_state.word_start = word_start;
            complete_state.word_end = word_end;
        }
        else{
            word_start = complete_state.word_start;
            word_end = complete_state.word_end;
            size = complete_state.iter.key.words[0].size;
        }
        
        // NOTE(allen): Iterate through matches.
        if (size > 0){
            for (;;){
                int32_t match_size = 0;
                Search_Match match = search_next_match(app, &complete_state.set, &complete_state.iter);
                
                if (match.found_match){
                    match_size = match.end - match.start;
                    Temp_Memory temp = begin_temp_memory(&global_part);
                    char *spare = push_array(&global_part, char, match_size);
                    
                    buffer_read_range(app, &match.buffer, match.start, match.end, spare);
                    
                    if (search_hit_add(&global_general, &complete_state.hits, &complete_state.str, spare, match_size)){
                        buffer_replace_range(app, &buffer, word_start, word_end, spare, match_size);
                        view_set_cursor(app, &view, seek_pos(word_start + match_size), true);
                        
                        complete_state.word_end = word_start + match_size;
                        complete_state.set.ranges[0].mid_size = match_size;
                        end_temp_memory(temp);
                        break;
                    }
                    end_temp_memory(temp);
                }
                else{
                    complete_state.iter.pos = 0;
                    complete_state.iter.i = 0;
                    
                    search_hits_init(&global_general, &complete_state.hits, &complete_state.str, 100, (4 << 10));
                    String word = complete_state.iter.key.words[0];
                    search_hit_add(&global_general, &complete_state.hits, &complete_state.str, word.str, word.size);
                    
                    match_size = word.size;
                    char *str = word.str;
                    buffer_replace_range(app, &buffer, word_start, word_end, str, match_size);
                    view_set_cursor(app, &view, seek_pos(word_start + match_size), true);
                    
                    complete_state.word_end = word_start + match_size;
                    complete_state.set.ranges[0].mid_size = match_size;
                    break;
                }
            }
        }
    }
}

// BOTTOM

