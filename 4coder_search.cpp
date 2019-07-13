/*
4coder_search.cpp - Commands that search accross buffers including word complete,
and list all locations.
*/

// TOP

//
// Search Iteration Systems
//

#if Migrate__Match_Iterator


CUSTOM_COMMAND_SIG(list_all_locations)
CUSTOM_DOC("Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_substring_locations)
CUSTOM_DOC("Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition)
CUSTOM_DOC("Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.")
{
    NotImplemented;
}

CUSTOM_COMMAND_SIG(word_complete)
CUSTOM_DOC("Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.")
{
    NotImplemented;
}

#else

static void
search_key_alloc(Heap *heap, Search_Key *key, umem *size, i32 count){
    count = clamp_top(count, ArrayCount(key->words));
    
    umem min_size = 0x7FFFFFFF;
    umem total_size = 0;
    for (i32 i = 0; i < count; ++i){
        total_size += size[i];
        if (min_size > size[i]){
            min_size = size[i];
        }
    }
    
    if (key->base == 0){
        umem max_base_size = total_size*2;
        key->base  = heap_array(heap, u8, (i32)max_base_size);
        key->base_size = max_base_size;
    }
    else if (key->base_size < total_size){
        umem max_base_size = total_size*2;
        heap_free(heap, key->base);
        key->base  = heap_array(heap, u8, (i32)max_base_size);
        key->base_size = max_base_size;
    }
    
    u8 *char_ptr = key->base;
    for (i32 i = 0; i < count; ++i){
        key->words[i].str = char_ptr;
        key->words[i].size = 0;
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
search_set_init(Heap *heap, Search_Set *set, i32 range_count){
    if (set->ranges == 0){
        i32 max = range_count*2;
        set->ranges = heap_array(heap, Search_Range, max);
        set->max = max;
    }
    else if (set->max < range_count){
        i32 max = range_count*2;
        heap_free(heap, set->ranges);
        set->ranges = heap_array(heap, Search_Range, max);
        set->max = max;
    }
    set->count = range_count;
}

static void
search_hits_table_alloc(Heap *heap, Table *hits, i32 table_size){
    void *mem = 0;
    i32 mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
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
search_hits_init(Heap *heap, Table *hits, String_Space *str, i32 table_size, i32 str_size){
    if (hits->hash_array == 0){
        search_hits_table_alloc(heap, hits, table_size);
    }
    else{
        i32 mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
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

static b32
search_hit_add(Heap *heap, Table *hits, String_Space *space, char *str, i32 len){
    b32 result = false;
    Assert(len != 0);
    Offset_String ostring = strspace_append(space, str, len);
    if (ostring.size == 0){
        i32 new_size = space->max*2;
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

static b32
search_hit_add(Heap *heap, Table *hits, String_Space *space, String_Const_u8 string){
    return(search_hit_add(heap, hits, space, (char*)string.str, (i32)string.size));
}

//
// Search Key Checking
//

static void
seek_potential_match(Application_Links *app, Search_Range *range, Search_Key key, Search_Match *result, Seek_Potential_Match_Direction direction, i64 start_pos, i64 end_pos){
    b32 case_insensitive = ((range->flags & SearchFlag_CaseInsensitive) != 0);
    b32 forward = (direction == SeekPotentialMatch_Forward);
#define OptFlag(b,f) ((b)?(f):(0))
    Buffer_Seek_String_Flags flags = 0
        | OptFlag(case_insensitive, BufferSeekString_CaseInsensitive)
        | OptFlag(!forward, BufferSeekString_Backward);
    result->buffer = range->buffer;
    
    i64 best_pos = -1;
    if (forward){
        best_pos = end_pos;
    }
    
    for (i32 i = 0; i < key.count; ++i){
        String_Const_u8 word = key.words[i];
        i64 new_pos = -1;
        buffer_seek_string(app, result->buffer, start_pos, end_pos, range->start, word, &new_pos, flags);
        
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
    
    result->start = (i32)best_pos;
}

static i32
match_check(Application_Links *app, Search_Range *range, i32 *pos, Search_Match *result_ptr, Search_Key key){
    i32 result_code = FindResult_None;
    
    Search_Match result = *result_ptr;
    i32 end_pos = range->start + range->size;
    
    i32 type = (range->flags & SearchFlag_MatchMask);
    result.match_word_index = -1;
    
    for (i32 i = 0; i < key.count; ++i){
        String_Const_u8 word = key.words[i];
        
        i32 found_match = FindResult_None;
        switch (type){
            case SearchFlag_MatchWholeWord:
            {
                u8 prev = ' ';
                if (character_is_alpha_numeric(string_get_character(word, 0))){
                    prev = buffer_get_char(app, result.buffer, result.start - 1);
                }
                
                if (!character_is_alpha_numeric(prev)){
                    result.end = result.start + (i32)word.size;
                    if (result.end <= end_pos){
                        u8 next = ' ';
                        if (character_is_alpha_numeric_unicode(string_get_character(word, word.size - 1))){
                            next = buffer_get_char(app, result.buffer, result.end);
                        }
                        
                        if (!character_is_alpha_numeric_unicode(next)){
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
                u8 prev = buffer_get_char(app, result.buffer, result.start - 1);
                if (!character_is_alpha_numeric_unicode(prev)){
                    result.end = (i32)scan(app, boundary_alpha_numeric_unicode, result.buffer, Scan_Forward, result.start);
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
                result.end = result.start + (i32)word.size;
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

static i32
search_front_to_back(Application_Links *app, Search_Range *range, Search_Key key, i32 *pos, Search_Match *result){
    i32 found_match = FindResult_None;
    for (;found_match == FindResult_None;){
        found_match = FindResult_None;
        
        i32 end_pos = range->start + range->size;
        if (*pos + key.min_size < end_pos){
            i32 start_pos = *pos;
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

static i32
search_back_to_front(Application_Links *app, Search_Range *range, Search_Key key, i32 *pos, Search_Match *result){
    i32 found_match = FindResult_None;
    for (;found_match == FindResult_None;){
        found_match = FindResult_None;
        if (*pos > range->start){
            i32 start_pos = *pos;
            
            seek_potential_match(app, range, key, result, SeekPotentialMatch_Backward, start_pos, 0);
            
            if (result->start >= range->start){
                *pos = result->start - 1;
                found_match = match_check(app, range, pos, result, key);
                if (found_match == FindResult_FoundMatch){
                    *pos = result->start - (i32)key.words[result->match_word_index].size;
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
    
    i32 count = set->count;
    for (;iter.i < count;){
        Search_Range *range = set->ranges + iter.i;
        
        i32 find_result = FindResult_None;
        
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
                
                i32 forward_result = FindResult_PastEnd;
                i32 backward_result = FindResult_PastEnd;
                
                if (iter.pos < range->start + range->size){
                    forward_result = search_front_to_back(app, range, iter.key, &iter.pos, &forward_match);
                }
                
                if (iter.back_pos > range->start){
                    backward_result = search_back_to_front(app, range, iter.key, &iter.back_pos, &backward_match);
                }
                
                if (forward_result == FindResult_FoundMatch){
                    if (backward_result == FindResult_FoundMatch){
                        find_result = FindResult_FoundMatch;
                        
                        i32 forward_start = range->mid_start + range->mid_size;
                        i32 forward_distance = (forward_match.start - forward_start);
                        i32 backward_distance = (range->mid_start - backward_match.end);
                        
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
initialize_generic_search_all_buffers(Application_Links *app, Heap *heap, String_Const_u8 *strings, i32 count, Search_Range_Flag match_flags, Buffer_ID *skip_buffers, i32 skip_buffer_count, Search_Set *set, Search_Iter *iter){
    block_zero_struct(set);
    block_zero_struct(iter);
    
    Search_Key key = {};
    umem sizes[ArrayCount(key.words)] = {};
    count = clamp_top(count, ArrayCount(key.words));
    for (i32 i = 0; i < count; ++i){
        sizes[i] = strings[i].size;
    }
    
    // TODO(allen): Why on earth am I allocating these separately in this case?  Upgrade to just use the string array on the stack!
    // NOTE(allen): ?? What did that TODO mean!?
    search_key_alloc(heap, &key, sizes, count);
    for (i32 i = 0; i < count; ++i){
        key.words[i].size = strings[i].size;
        block_copy(key.words[i].str, strings[i].str, strings[i].size);
    }
    
    search_iter_init(iter, key);
    
    i32 buffer_count = get_buffer_count(app);
    search_set_init(heap, set, buffer_count);
    
    Search_Range *ranges = set->ranges;
    
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    
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
            ranges[j].size = (i32)buffer_get_size(app, buffer);
            ++j;
        }
    }
    
    for (Buffer_ID buffer_it = get_buffer_next(app, 0, AccessAll);
         buffer_exists(app, buffer_it);
         buffer_it = get_buffer_next(app, buffer_it, AccessAll)){
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
                ranges[j].size = (i32)buffer_get_size(app, buffer_it);
                ++j;
            }
        }
    }
    
    set->count = j;
}

////////////////////////////////

static String_Const_u8 search_name = string_u8_litexpr("*search*");

static void
list__parameters_buffer(Application_Links *app, Heap *heap,
                        String_Const_u8 *strings, i32 count, Search_Range_Flag match_flags,
                        Buffer_ID search_buffer_id){
    Arena *scratch = context_get_arena(app);
    
    // Setup the search buffer for 'init' mode
    buffer_set_setting(app, search_buffer_id, BufferSetting_ReadOnly, true);
    buffer_set_setting(app, search_buffer_id, BufferSetting_RecordsHistory, false);
    
    // Initialize a generic search all buffers
    Search_Set set = {};
    Search_Iter iter = {};
    initialize_generic_search_all_buffers(app, heap, strings, count, match_flags, &search_buffer_id, 1, &set, &iter);
    
    // List all locations into search buffer
    Temp_Memory all_temp = begin_temp(scratch);
    
    // Setup buffered output
    Cursor buffering_cursor = make_cursor(push_array(scratch, u8, KB(64)), KB(64));
    Buffer_Insertion out = begin_buffer_insertion_at_buffered(app, search_buffer_id, 0, &buffering_cursor);
    
    
    Buffer_ID prev_match_id = 0;
    b32 no_matches = true;
    for (Search_Match match = search_next_match(app, &set, &iter);
         match.found_match;
         match = search_next_match(app, &set, &iter)){
        Partial_Cursor word_pos = buffer_compute_cursor(app, match.buffer, seek_pos(match.start));
        if (word_pos.line > 0){
            if (prev_match_id != match.buffer){
                if (prev_match_id != 0){
                    insertc(&out, '\n');
                }
                prev_match_id = match.buffer;
            }
            
            Temp_Memory line_temp = begin_temp(scratch);
            String_Const_u8 file_name = push_buffer_file_name(app, scratch, match.buffer);
            String_Const_u8 full_line_str = push_buffer_line(app, scratch, match.buffer, word_pos.line);
            if (full_line_str.size > 0){
                String_Const_u8 line_str = string_skip_chop_whitespace(full_line_str);
                insertf(&out, "%.*s:%d:%d: %.*s\n", string_expand(file_name), word_pos.line, word_pos.character, string_expand(line_str));
            }
            end_temp(line_temp);
            
            no_matches = false;
        }
    }
    
    if (no_matches){
        insert_string(&out, string_u8_litexpr("no matches\n"));
    }
    
    end_buffer_insertion(&out);
    
    end_temp(all_temp);
    
    // Lock *search* as the jump buffer
    lock_jump_buffer(search_name);
}

static void
list__parameters(Application_Links *app, Heap *heap, String_Const_u8 *strings, i32 count, Search_Range_Flag match_flags, View_ID default_target_view){
    // Open the search buffer
    Buffer_ID search_buffer_id = create_or_switch_to_buffer_and_clear_by_name(app, search_name, default_target_view);
    list__parameters_buffer(app, heap, strings, count, match_flags, search_buffer_id);
}

static void
list_single__parameters(Application_Links *app, Heap *heap, String_Const_u8 str, b32 substrings, b32 case_insensitive, View_ID default_target_view){
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
    list__parameters(app, heap, &str, 1, flags, default_target_view);
}

static void
list_query__parameters(Application_Links *app, Heap *heap, b32 substrings, b32 case_insensitive, View_ID default_target_view){
    char space[1024];
    String_Const_u8 str = get_query_string(app, "List Locations For: ", space, sizeof(space));
    if (str.size > 0){
        list_single__parameters(app, heap, str, substrings, case_insensitive, default_target_view);
    }
}

static void
list_identifier__parameters(Application_Links *app, Heap *heap, b32 substrings, b32 case_insensitive, View_ID default_target_view){
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    if (buffer != 0){
        i64 pos = view_get_cursor_pos(app, view);
        Scratch_Block scratch(app);
        String_Const_u8 str = get_token_or_word_under_pos(app, scratch, buffer, pos);
        if (str.size > 0){
            list_single__parameters(app, heap, str, substrings, case_insensitive, default_target_view);
        }
    }
}

static void
list_selected_range__parameters(Application_Links *app, Heap *heap, b32 substrings, b32 case_insensitive, View_ID default_target_view){
    View_ID view = get_active_view(app, AccessProtected);
    Scratch_Block scratch(app);
    String_Const_u8 str = push_view_range_string(app, scratch, view);
    if (str.size > 0){
        list_single__parameters(app, heap, str, substrings, case_insensitive, default_target_view);
    }
}

static void
list_type_definition__parameters(Application_Links *app, Heap *heap, String_Const_u8 str, View_ID default_target_view){
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_u8 match_strings[9];
    i32 i = 0;
    match_strings[i++] = (push_u8_stringf(scratch, "struct %.*s{"  , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "struct %.*s\n{", string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "struct %.*s {" , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "union %.*s{"   , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "union %.*s\n{" , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "union %.*s {"  , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "enum %.*s{"    , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "enum %.*s\n{"  , string_expand(str)));
    match_strings[i++] = (push_u8_stringf(scratch, "enum %.*s {"   , string_expand(str)));
    
    list__parameters(app, heap, match_strings, ArrayCount(match_strings), 0, default_target_view);
    
    end_temp(temp);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(list_all_locations)
CUSTOM_DOC("Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, false, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations)
CUSTOM_DOC("Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, true, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, false, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_query__parameters(app, &global_heap, true, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_identifier__parameters(app, &global_heap, false, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_identifier__parameters(app, &global_heap, false, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_selected_range__parameters(app, &global_heap, false, false, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.")
{
    View_ID target_view = get_next_view_after_active(app, AccessAll);
    list_selected_range__parameters(app, &global_heap, false, true, target_view);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition)
CUSTOM_DOC("Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.")
{
    char space[1024];
    String_Const_u8 str = get_query_string(app, "List Definitions For: ", space, sizeof(space));
    if (str.size > 0){
        View_ID target_view = get_next_view_after_active(app, AccessAll);
        list_type_definition__parameters(app, &global_heap, str, target_view);
    }
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.")
{
    View_ID target_view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, target_view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, target_view);
    Scratch_Block scratch(app);
    String_Const_u8 str = get_token_or_word_under_pos(app, scratch, buffer, pos);
    if (str.size > 0){
        target_view = get_next_view_after_active(app, AccessAll);
        list_type_definition__parameters(app, &global_heap, str, target_view);
    }
}

//
// Word Complete Command
//

static Word_Complete_State complete_state = {};

CUSTOM_COMMAND_SIG(word_complete)
CUSTOM_DOC("Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    if (buffer != 0){
        Managed_Scope scope = view_get_managed_scope(app, view);
        
        i32 do_init = false;
        u64 rewrite = 0;
        managed_variable_get(app, scope, view_rewrite_loc, &rewrite);
        if (rewrite != RewriteWordComplete){
            do_init = true;
        }
        managed_variable_set(app, scope, view_next_rewrite_loc, RewriteWordComplete);
        if (!complete_state.initialized){
            do_init = true;
        }
        
        i64 word_end = 0;
        i64 word_start = 0;
        i64 cursor_pos = 0;
        umem size = 0;
        
        if (do_init){
            // NOTE(allen): Get the range where the
            // partial word is written.
            word_end = view_get_cursor_pos(app, view);
            word_start = word_end;
            cursor_pos = word_end - 1;
            
            char space[1024];
            Stream_Chunk chunk = {};
            if (init_stream_chunk(&chunk, app, buffer, (i32)cursor_pos, space, sizeof(space))){
                i32 still_looping = true;
                do{
                    for (; cursor_pos >= chunk.start; --cursor_pos){
                        u8 c = chunk.data[cursor_pos];
                        if (character_is_alpha_unicode(c)){
                            word_start = cursor_pos;
                        }
                        else if (!character_is_base10(c)){
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
            buffer_read_range(app, buffer, Ii64(word_start, word_end), (char*)key.words[0].str);
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
            ranges[0].size = (i32)buffer_get_size(app, buffer);
            ranges[0].mid_start = (i32)word_start;
            ranges[0].mid_size = (i32)size;
            
            Buffer_ID buffer_it = 0;
            
            i32 j = 1;
            for (buffer_it = get_buffer_next(app, 0, AccessAll);
                 buffer_it != 0;
                 buffer_it = get_buffer_next(app, buffer_it, AccessAll)){
                if (buffer != buffer_it){
                    ranges[j].type = SearchRange_FrontToBack;
                    ranges[j].flags = SearchFlag_MatchWordPrefix;
                    ranges[j].buffer = buffer_it;
                    ranges[j].start = 0;
                    ranges[j].size = (i32)buffer_get_size(app, buffer_it);
                    ++j;
                }
            }
            complete_state.set.count = j;
            
            // NOTE(allen): Initialize the search hit table.
            search_hits_init(&global_heap, &complete_state.hits, &complete_state.str, 100, (4 << 10));
            String_Const_u8 word = complete_state.iter.key.words[0];
            search_hit_add(&global_heap, &complete_state.hits, &complete_state.str, word);
            
            complete_state.word_start = (i32)word_start;
            complete_state.word_end = (i32)word_end;
        }
        else{
            word_start = complete_state.word_start;
            word_end = complete_state.word_end;
            size = (i32)complete_state.iter.key.words[0].size;
        }
        
        // NOTE(allen): Iterate through matches.
        if (size > 0){
            for (;;){
                Search_Match match = search_next_match(app, &complete_state.set, &complete_state.iter);
                
                if (match.found_match){
                    i32 match_size = match.end - match.start;
                    Arena *scratch = context_get_arena(app);
                    Scratch_Block temp_auto_closer(scratch);
                    String_Const_u8 spare = push_buffer_range(app, scratch, match.buffer, Ii64(match.start, match.end));
                    if (search_hit_add(&global_heap, &complete_state.hits, &complete_state.str, (char*)spare.str, (i32)spare.size)){
                        buffer_replace_range(app, buffer, Ii64(word_start, word_end), spare);
                        view_set_cursor(app, view, seek_pos(word_start + match_size), true);
                        
                        complete_state.word_end = (i32)(word_start + match_size);
                        complete_state.set.ranges[0].mid_size = match_size;
                        break;
                    }
                }
                else{
                    complete_state.iter.pos = 0;
                    complete_state.iter.i = 0;
                    
                    search_hits_init(&global_heap, &complete_state.hits, &complete_state.str, 100, (4 << 10));
                    String_Const_u8 word = complete_state.iter.key.words[0];
                    search_hit_add(&global_heap, &complete_state.hits, &complete_state.str, word);
                    
                    i32 match_size = (i32)word.size;
                    buffer_replace_range(app, buffer, Ii64(word_start, word_end), word);
                    view_set_cursor(app, view, seek_pos(word_start + match_size), true);
                    
                    complete_state.word_end = (i32)(word_start + match_size);
                    complete_state.set.ranges[0].mid_size = match_size;
                    break;
                }
            }
        }
    }
}

#endif

// BOTTOM

