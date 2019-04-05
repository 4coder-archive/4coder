/*
4coder_search.cpp - Commands that search accross buffers including word complete,
and list all locations.
*/

// TOP

//
// Search Iteration Systems
//

static void
search_key_alloc(Heap *heap, Search_Key *key, int32_t *size, int32_t count){
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
    
    if (key->base == 0){
        int32_t max_base_size = total_size*2;
        key->base  = heap_array(heap, char, max_base_size);
        key->base_size = max_base_size;
    }
    else if (key->base_size < total_size){
        int32_t max_base_size = total_size*2;
        heap_free(heap, key->base);
        key->base  = heap_array(heap, char, max_base_size);
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
search_set_init(Heap *heap, Search_Set *set, int32_t range_count){
    if (set->ranges == 0){
        int32_t max = range_count*2;
        set->ranges = heap_array(heap, Search_Range, max);
        set->max = max;
    }
    else if (set->max < range_count){
        int32_t max = range_count*2;
        heap_free(heap, set->ranges);
        set->ranges = heap_array(heap, Search_Range, max);
        set->max = max;
    }
    set->count = range_count;
}

static void
search_hits_table_alloc(Heap *heap, Table *hits, int32_t table_size){
    void *mem = 0;
    int32_t mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
    if (hits->hash_array == 0){
        mem = heap_allocate(heap, mem_size);
    }
    else{
        heap_free(heap, hits->hash_array);
        mem = heap_allocate(heap, mem_size);
    }
    table_init_memory(hits, mem, table_size, sizeof(Offset_String));
}

static void
search_hits_init(Heap *heap, Table *hits, String_Space *str, int32_t table_size, int32_t str_size){
    if (hits->hash_array == 0){
        search_hits_table_alloc(heap, hits, table_size);
    }
    else{
        int32_t mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
        heap_free(heap, hits->hash_array);
        void *mem = heap_allocate(heap, mem_size);
        table_init_memory(hits, mem, table_size, sizeof(Offset_String));
    }
    
    if (str->space == 0){
        str->space = heap_array(heap, char, str_size);
        str->max = str_size;
    }
    else if (str->max < str_size){
        heap_free(heap, str->space);
        str->space = heap_array(heap, char, str_size);
        str->max = str_size;
    }
    
    str->pos = str->new_pos = 0;
    table_clear(hits);
}

//
// Table Operations
//

static int32_t
search_hit_add(Heap *heap, Table *hits, String_Space *space, char *str, int32_t len){
    int32_t result = false;
    
    Assert(len != 0);
    
    Offset_String ostring = strspace_append(space, str, len);
    if (ostring.size == 0){
        int32_t new_size = space->max*2;
        if (new_size < space->max + len){
            new_size = space->max + len;
        }
        char *new_space = heap_array(heap, char, new_size);
        memcpy(new_space, space->space, space->new_pos);
        heap_free(heap, space->space);
        space->space = new_space;
        ostring = strspace_append(space, str, len);
    }
    
    Assert(ostring.size != 0);
    
    if (table_at_capacity(hits)){
        Table new_hits = {};
        search_hits_table_alloc(heap, &new_hits, hits->max*2);
        table_clear(&new_hits);
        table_rehash(hits, &new_hits, space->space, tbl_offset_string_hash, tbl_offset_string_compare);
        heap_free(heap, hits->hash_array);
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
    b32 case_insensitive = ((range->flags & SearchFlag_CaseInsensitive) != 0);
    b32 forward = (direction == SeekPotentialMatch_Forward);
#define OptFlag(b,f) ((b)?(f):(0))
    Buffer_Seek_String_Flags flags = 0
        | OptFlag(case_insensitive, BufferSeekString_CaseInsensitive)
        | OptFlag(!forward, BufferSeekString_Backward);
    result->buffer = range->buffer;
    
    int32_t best_pos = -1;
    if (forward){
        best_pos = end_pos;
    }
    
    for (int32_t i = 0; i < key.count; ++i){
        String word = key.words[i];
        int32_t new_pos = -1;
        buffer_seek_string(app, result->buffer, start_pos, end_pos, range->start, word.str, word.size, &new_pos, flags);
        
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

static i32
buffer_seek_alpha_numeric_end(Application_Links *app, Buffer_ID buffer_id, int32_t pos){
    char space[1024];
    Stream_Chunk chunk = {};
    if (init_stream_chunk(&chunk, app, buffer_id, pos, space, sizeof(space))){
        int32_t still_looping = true;
        do{
            for (; pos < chunk.end; ++pos){
                u8 at_pos = (u8)chunk.data[pos];
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
                    prev = buffer_get_char(app, result.buffer, result.start - 1);
                }
                
                if (!char_is_alpha_numeric_utf8(prev)){
                    result.end = result.start + word.size;
                    if (result.end <= end_pos){
                        char next = ' ';
                        if (char_is_alpha_numeric_utf8(word.str[word.size-1])){
                            next = buffer_get_char(app, result.buffer, result.end);
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
                char prev = buffer_get_char(app, result.buffer, result.start - 1);
                if (!char_is_alpha_numeric_utf8(prev)){
                    result.end = buffer_seek_alpha_numeric_end(app, result.buffer, result.start);
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
    Search_Match result = {};
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
                Search_Match forward_match = {};
                Search_Match backward_match = {};
                
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
initialize_generic_search_all_buffers(Application_Links *app, Heap *heap, String *strings, i32 count, Search_Range_Flag match_flags, Buffer_ID *skip_buffers, i32 skip_buffer_count, Search_Set *set, Search_Iter *iter){
    memset(set, 0, sizeof(*set));
    memset(iter, 0, sizeof(*iter));
    
    Search_Key key = {};
    i32 sizes[ArrayCount(key.words)];
    memset(sizes, 0, sizeof(sizes));
    
    if (count > ArrayCount(key.words)){
        count = ArrayCount(key.words);
    }
    for (i32 i = 0; i < count; ++i){
        sizes[i] = strings[i].size;
    }
    
    // TODO(allen): Why on earth am I allocating these separately in this case?  Upgrade to just use the string array on the stack!
    search_key_alloc(heap, &key, sizes, count);
    for (i32 i = 0; i < count; ++i){
        copy(&key.words[i], strings[i]);
    }
    
    search_iter_init(iter, key);
    
    i32 buffer_count = get_buffer_count(app);
    search_set_init(heap, set, buffer_count);
    
    Search_Range *ranges = set->ranges;
    
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    
    i32 j = 0;
    if (buffer_exists(app, buffer)){
        b32 skip = false;
        for (i32 i = 0; i < skip_buffer_count; ++i){
            if (buffer == skip_buffers[i]){
                skip = true;
                break;
            }
        }
        
        if (!skip){
            ranges[j].type = SearchRange_FrontToBack;
            ranges[j].flags = match_flags;
            ranges[j].buffer = buffer;
            ranges[j].start = 0;
            buffer_get_size(app, buffer, &ranges[j].size);
            ++j;
        }
    }
    
    Buffer_ID buffer_it = 0;
    for (get_buffer_next(app, 0, AccessAll, &buffer_it);
         buffer_exists(app, buffer_it);
         get_buffer_next(app, buffer_it, AccessAll, &buffer_it)){
        if (buffer_it == buffer){
            continue;
        }
        
        b32 skip = false;
        for (i32 i = 0; i < skip_buffer_count; ++i){
            if (buffer_it == skip_buffers[i]){
                skip = true;
                break;
            }
        }
        
        if (!skip){
            if (!buffer_has_name_with_star(app, buffer_it)){
                ranges[j].type = SearchRange_FrontToBack;
                ranges[j].flags = match_flags;
                ranges[j].buffer = buffer_it;
                ranges[j].start = 0;
                buffer_get_size(app, buffer_it, &ranges[j].size);
                ++j;
            }
        }
    }
    
    set->count = j;
}

////////////////////////////////

static String search_name = make_lit_string("*search*");

// TODO(allen): can this get merged with the insertion stuff???
struct Buffered_Printing{
    Partition *part;
    Temp_Memory temp;
    Buffer_ID buffer;
    i32 pos;
};

static Buffered_Printing
make_buffered_print(Application_Links *app, Partition *part, Buffer_ID buffer){
    Buffered_Printing buffered_print = {};
    buffered_print.part = part;
    buffered_print.temp = begin_temp_memory(part);
    buffered_print.buffer = buffer;
    buffer_get_size(app, buffer, &buffered_print.pos);
    return(buffered_print);
}

static void
buffered_print_flush(Application_Links *app, Buffered_Printing *out){
    i32 write_size = out->part->pos - out->temp.pos;
    char *str = out->part->base + out->temp.pos;
    buffer_replace_range(app, out->buffer, out->pos, out->pos, make_string(str, write_size));
    out->pos += write_size;
    end_temp_memory(out->temp);
}

static char*
buffered_memory_reserve(Application_Links *app, Buffered_Printing *out, i32 length, b32 *did_flush){
    char *mem = push_array(out->part, char, length);
    *did_flush = false;
    if (mem == 0){
        buffered_print_flush(app, out);
        mem = push_array(out->part, char, length);
        *did_flush = true;
    }
    Assert(mem != 0);
    return(mem);
}

static char*
buffered_memory_reserve(Application_Links *app, Buffered_Printing *out, i32 length){
    b32 ignore;
    return(buffered_memory_reserve(app, out, length, &ignore));
}

static i32
buffered_print_buffer_length(Buffered_Printing *out){
    return(out->part->pos - out->temp.pos);
}

static void
list__parameters_buffer(Application_Links *app, Heap *heap, Partition *scratch,
                        String *strings, i32 count, Search_Range_Flag match_flags,
                        Buffer_ID search_buffer_id){
    // Setup the search buffer for 'init' mode
    buffer_set_setting(app, search_buffer_id, BufferSetting_ReadOnly, true);
    buffer_set_setting(app, search_buffer_id, BufferSetting_RecordsHistory, false);
    
    // Initialize a generic search all buffers
    Search_Set set = {};
    Search_Iter iter = {};
    initialize_generic_search_all_buffers(app, heap, strings, count, match_flags, &search_buffer_id, 1, &set, &iter);
    
    // List all locations into search buffer
    Temp_Memory all_temp = begin_temp_memory(scratch);
    Partition line_part = part_sub_part(scratch, (4 << 10));
    
    // Setup buffered output
    Buffered_Printing out = make_buffered_print(app, scratch, search_buffer_id);
    
    Buffer_ID prev_match_id = 0;
    b32 no_matches = true;
    for (Search_Match match = search_next_match(app, &set, &iter);
         match.found_match;
         match = search_next_match(app, &set, &iter)){
        Partial_Cursor word_pos = {};
        if (buffer_compute_cursor(app, match.buffer, seek_pos(match.start), &word_pos)){
            if (prev_match_id != match.buffer){
                if (prev_match_id != 0){
                    char *newline = buffered_memory_reserve(app, &out, 1);
                    *newline = '\n';
                }
                prev_match_id = match.buffer;
            }
            
            char file_name_space[256];
            String file_name = make_fixed_width_string(file_name_space);
            buffer_get_file_name(app, match.buffer, &file_name, 0);
            
            i32 line_num_len = int_to_str_size(word_pos.line);
            i32 column_num_len = int_to_str_size(word_pos.character);
            
            Temp_Memory line_temp = begin_temp_memory(&line_part);
            Partial_Cursor line_start_cursor = {};
            Partial_Cursor line_one_past_last_cursor = {};
            String full_line_str = {};
            if (read_line(app, &line_part, match.buffer, word_pos.line, &full_line_str, &line_start_cursor, &line_one_past_last_cursor)){
                String line_str = skip_chop_whitespace(full_line_str);
                
                i32 str_len = file_name.size + 1 + line_num_len + 1 + column_num_len + 1 + 1 + line_str.size + 1;
                char *spare = buffered_memory_reserve(app, &out, str_len);
                
                String out_line = make_string_cap(spare, 0, str_len);
                append_ss(&out_line, file_name);
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
            
            no_matches = false;
        }
    }
    
    if (no_matches){
        char no_matches_message[] = "no matches\n";
        i32 no_matches_message_length = sizeof(no_matches_message) - 1;
        char *no_matches_message_out = buffered_memory_reserve(app, &out, no_matches_message_length);
        memcpy(no_matches_message_out, no_matches_message, no_matches_message_length);
    }
    
    buffered_print_flush(app, &out);
    
    end_temp_memory(all_temp);
    
    // Lock *search* as the jump buffer
    lock_jump_buffer(search_name.str, search_name.size);
}

static void
list__parameters(Application_Links *app, Heap *heap, Partition *scratch, String *strings, i32 count,
                 Search_Range_Flag match_flags, View_Summary default_target_view){
    // Open the search buffer
    Buffer_ID search_buffer_id = create_or_switch_to_buffer_by_name(app, search_name.str, search_name.size, default_target_view);
    list__parameters_buffer(app, heap, scratch, strings, count, match_flags, search_buffer_id);
}

static void
list_single__parameters(Application_Links *app, Heap *heap, Partition *scratch,
                        String str, b32 substrings, b32 case_insensitive,
                        View_Summary default_target_view){
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
    list__parameters(app, heap, scratch, &str, 1, flags, default_target_view);
}

static void
list_query__parameters(Application_Links *app, Heap *heap, Partition *scratch,
                       b32 substrings, b32 case_insensitive,
                       View_Summary default_target_view){
    char space[1024];
    String str = get_query_string(app, "List Locations For: ", space, sizeof(space));
    if (str.size > 0){
        list_single__parameters(app, heap, scratch, str, substrings, case_insensitive, default_target_view);
    }
}

static void
list_identifier__parameters(Application_Links *app, Heap *heap, Partition *scratch,
                            b32 substrings, b32 case_insensitive,
                            View_Summary default_target_view){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessProtected, &buffer);
    if (buffer_exists(app, buffer)){
        char space[512];
        String str = get_token_or_word_under_pos(app, buffer, view.cursor.pos, space, sizeof(space));
        if (str.size > 0){
            list_single__parameters(app, heap, scratch, str, substrings, case_insensitive, default_target_view);
        }
    }
}

static void
list_selected_range__parameters(Application_Links *app, Heap *heap, Partition *scratch,
                                b32 substrings, b32 case_insensitive,
                                View_Summary default_target_view){
    View_Summary view = get_active_view(app, AccessProtected);
    Temp_Memory temp = begin_temp_memory(scratch);
    String str = get_string_in_view_range(app, scratch, &view);
    if (str.size > 0){
        list_single__parameters(app, heap, scratch, str, substrings, case_insensitive, default_target_view);
    }
    end_temp_memory(temp);
}

static void
list_type_definition__parameters(Application_Links *app, Heap *heap, Partition *scratch,
                                 String str,
                                 View_Summary default_target_view){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    String match_strings[9];
    i32 i = 0;
    match_strings[i++] = string_push_f(scratch, "struct %.*s{"  , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "struct %.*s\n{", str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "struct %.*s {" , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "union %.*s{"   , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "union %.*s\n{" , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "union %.*s {"  , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "enum %.*s{"    , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "enum %.*s\n{"  , str.size, str.str);
    match_strings[i++] = string_push_f(scratch, "enum %.*s {"   , str.size, str.str);
    
    list__parameters(app, heap, scratch,
                     match_strings, ArrayCount(match_strings), 0,
                     default_target_view);
    
    end_temp_memory(temp);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(list_all_locations)
CUSTOM_DOC("Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, &global_part, false, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations)
CUSTOM_DOC("Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, &global_part, true, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, &global_part, false, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, &global_part, true, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_identifier__parameters(app, &global_heap, &global_part, false, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_identifier__parameters(app, &global_heap, &global_part, false, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_selected_range__parameters(app, &global_heap, &global_part, false, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.")
{
    View_Summary target_view = get_next_view_after_active(app, AccessAll);
    list_selected_range__parameters(app, &global_heap, &global_part, false, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition)
CUSTOM_DOC("Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.")
{
    char space[1024];
    String str = get_query_string(app, "List Definitions For: ", space, sizeof(space));
    if (str.size > 0){
        View_Summary target_view = get_next_view_after_active(app, AccessAll);
        list_type_definition__parameters(app, &global_heap, &global_part, str, target_view);
    }
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessProtected, &buffer);
    char space[512];
    String str = get_token_or_word_under_pos(app, buffer, view.cursor.pos, space, sizeof(space) - 1);
    if (str.size > 0){
        View_Summary target_view = get_next_view_after_active(app, AccessAll);
        list_type_definition__parameters(app, &global_heap, &global_part, str, target_view);
    }
}


//
// Word Complete Command
//

static Word_Complete_State complete_state = {};

CUSTOM_COMMAND_SIG(word_complete)
CUSTOM_DOC("Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = 0;
    if (view_get_buffer(app, view.view_id, AccessOpen, &buffer)){
        i32 do_init = false;
        
        Managed_Scope scope = view_get_managed_scope(app, view.view_id);
        
        u64 rewrite = 0;
        managed_variable_get(app, scope, view_rewrite_loc, &rewrite);
        if (rewrite != RewriteWordComplete){
            do_init = true;
        }
        managed_variable_set(app, scope, view_next_rewrite_loc, RewriteWordComplete);
        if (!complete_state.initialized){
            do_init = true;
        }
        
        i32 word_end = 0;
        i32 word_start = 0;
        i32 cursor_pos = 0;
        i32 size = 0;
        
        if (do_init){
            // NOTE(allen): Get the range where the
            // partial word is written.
            word_end = view.cursor.pos;
            word_start = word_end;
            cursor_pos = word_end - 1;
            
            char space[1024];
            Stream_Chunk chunk = {};
            if (init_stream_chunk(&chunk, app, buffer, cursor_pos, space, sizeof(space))){
                i32 still_looping = true;
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
            Search_Key key = {};
            search_key_alloc(&global_heap, &key, &size, 1);
            buffer_read_range(app, buffer, word_start, word_end, key.words[0].str);
            key.words[0].size = size;
            
            search_iter_init(&complete_state.iter, key);
            
            // NOTE(allen): Initialize the set of ranges to be searched.
            i32 buffer_count = get_buffer_count(app);
            search_set_init(&global_heap, &complete_state.set, buffer_count);
            
            Search_Range *ranges = complete_state.set.ranges;
            ranges[0].type = SearchRange_Wave;
            ranges[0].flags = SearchFlag_MatchWordPrefix;
            ranges[0].buffer = buffer;
            ranges[0].start = 0;
            buffer_get_size(app, buffer, &ranges[0].size);
            ranges[0].mid_start = word_start;
            ranges[0].mid_size = size;
            
            Buffer_ID buffer_it = 0;
            
            i32 j = 1;
            for (get_buffer_next(app, 0, AccessAll, &buffer_it);
                 buffer_it != 0;
                 get_buffer_next(app, buffer_it, AccessAll, &buffer_it)){
                if (buffer != buffer_it){
                    ranges[j].type = SearchRange_FrontToBack;
                    ranges[j].flags = SearchFlag_MatchWordPrefix;
                    ranges[j].buffer = buffer_it;
                    ranges[j].start = 0;
                    buffer_get_size(app, buffer_it, &ranges[j].size);
                    ++j;
                }
            }
            complete_state.set.count = j;
            
            // NOTE(allen): Initialize the search hit table.
            search_hits_init(&global_heap, &complete_state.hits, &complete_state.str, 100, (4 << 10));
            String word = complete_state.iter.key.words[0];
            search_hit_add(&global_heap, &complete_state.hits, &complete_state.str, word.str, word.size);
            
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
                i32 match_size = 0;
                Search_Match match = search_next_match(app, &complete_state.set, &complete_state.iter);
                
                if (match.found_match){
                    match_size = match.end - match.start;
                    Temp_Memory temp = begin_temp_memory(&global_part);
                    char *spare = push_array(&global_part, char, match_size);
                    
                    buffer_read_range(app, match.buffer, match.start, match.end, spare);
                    
                    if (search_hit_add(&global_heap, &complete_state.hits, &complete_state.str, spare, match_size)){
                        buffer_replace_range(app, buffer, word_start, word_end, make_string(spare, match_size));
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
                    
                    search_hits_init(&global_heap, &complete_state.hits, &complete_state.str, 100, (4 << 10));
                    String word = complete_state.iter.key.words[0];
                    search_hit_add(&global_heap, &complete_state.hits, &complete_state.str, word.str, word.size);
                    
                    match_size = word.size;
                    char *str = word.str;
                    buffer_replace_range(app, buffer, word_start, word_end, make_string(str, match_size));
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

