
#ifndef FCODER_SEARCH
#define FCODER_SEARCH

enum Search_Range_Type{
    SearchRange_FrontToBack,
    SearchRange_BackToFront,
    SearchRange_Wave,
};

enum Search_Range_Flag{
    SearchFlag_MatchWholeWord  = 0x00,
    SearchFlag_MatchWordPrefix = 0x01,
    SearchFlag_MatchSubstring  = 0x02,
    SearchFlag_MatchMask       = 0xFF,
    SearchFlag_CaseInsensitive = 0x0100,
};

struct Search_Range{
    int32_t type;
    uint32_t flags;
    int32_t buffer;
    int32_t start;
    int32_t size;
    int32_t mid_start;
    int32_t mid_size;
};

struct Search_Set{
    Search_Range *ranges;
    int32_t count;
    int32_t max;
};

struct Search_Iter{
    String word;
    int32_t pos;
    int32_t back_pos;
    int32_t i;
    int32_t range_initialized;
};

struct Search_Match{
    Buffer_Summary buffer;
    int32_t start;
    int32_t end;
    int32_t found_match;
};

static void
search_iter_init(General_Memory *general, Search_Iter *iter, int32_t size){
    int32_t str_max = size*2;
    if (iter->word.str == 0){
        iter->word.str = (char*)general_memory_allocate(general, str_max);
        iter->word.memory_size = str_max;
    }
    else if (iter->word.memory_size < size){
        iter->word.str = (char*)general_memory_reallocate_nocopy(general, iter->word.str, str_max);
        iter->word.memory_size = str_max;
    }
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

static int32_t
search_hit_add(General_Memory *general, Table *hits, String_Space *space, char *str, int32_t len){
    int32_t result = false;
    
    assert(len != 0);
    
    Offset_String ostring = strspace_append(space, str, len);
    if (ostring.size == 0){
        int32_t new_size = space->max*2;
        if (new_size < space->max + len){
            new_size = space->max + len;
        }
        space->space = (char*)general_memory_reallocate(
            general, space->space, space->new_pos, new_size);
        ostring = strspace_append(space, str, len);
    }
    
    assert(ostring.size != 0);
    
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

static int32_t
buffer_seek_alpha_numeric_end(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char space[1024];
    Stream_Chunk chunk = {0};
    if (init_stream_chunk(&chunk, app, buffer, pos, space, sizeof(space))){
        int32_t still_looping = true;
        do{
            for (; pos < chunk.end; ++pos){
                char at_pos = chunk.data[pos];
                if (!char_is_alpha_numeric(at_pos)) goto double_break;
            }
            still_looping = forward_stream_chunk(&chunk);
        }while(still_looping);
    }
    double_break:;
    return(pos);
}

static void
search_iter_next_range(Search_Iter *it){
    ++it->i;
    it->pos = 0;
    it->back_pos = 0;
    it->range_initialized = 0;
}

enum{
    FindResult_None,
    FindResult_FoundMatch,
    FindResult_PastEnd
};

static int32_t
match_check(Application_Links *app, Search_Range *range, int32_t *pos, Search_Match *result_ptr, String word){
    int32_t found_match = FindResult_None;
    
    Search_Match result = *result_ptr;
    int32_t end_pos = range->start + range->size;
    
    int32_t type = (range->flags & SearchFlag_MatchMask);
    
    switch (type){
        case SearchFlag_MatchWholeWord:
        {
            char first = word.str[0];
            
            char prev = ' ';
            if (char_is_alpha_numeric(first)){
                prev = buffer_get_char(app, &result.buffer, result.start - 1);
            }
            
            if (!char_is_alpha_numeric(prev)){
                result.end = result.start + word.size;
                if (result.end <= end_pos){
                    char last = word.str[word.size-1];
                    
                    char next = ' ';
                    if (char_is_alpha_numeric(last)){
                        next = buffer_get_char(app, &result.buffer, result.end);
                    }
                    
                    if (!char_is_alpha_numeric(next)){
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
            if (!char_is_alpha_numeric(prev)){
                result.end =
                    buffer_seek_alpha_numeric_end(
                    app, &result.buffer, result.start);
                
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
    
    *result_ptr = result;
    
    return(found_match);
}

static int32_t
search_front_to_back_step(Application_Links *app, Search_Range *range, String word, int32_t *pos, Search_Match *result_ptr){
    int32_t found_match = FindResult_None;
    
    Search_Match result = *result_ptr;
    
    int32_t end_pos = range->start + range->size;
    if (*pos + word.size < end_pos){
        int32_t start_pos = *pos;
        if (start_pos < range->start){
            start_pos = range->start;
        }
        
        int32_t case_insensitive = (range->flags & SearchFlag_CaseInsensitive);
        
        result.buffer = get_buffer(app, range->buffer, AccessAll);
        if (case_insensitive){
            buffer_seek_string_insensitive_forward(app, &result.buffer, start_pos, end_pos, word.str, word.size, &result.start);
        }
        else{
            buffer_seek_string_forward(app, &result.buffer, start_pos, end_pos, word.str, word.size, &result.start);
        }
        
        if (result.start < end_pos){
            *pos = result.start + 1;
            found_match = match_check(app, range, pos, &result, word);
            if (found_match == FindResult_FoundMatch){
                *pos = result.end;
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
    
    *result_ptr = result;
    
    return(found_match);
}

static int32_t
search_front_to_back(Application_Links *app, Search_Range *range, String word, int32_t *pos, Search_Match *result_ptr){
    int32_t found_match = FindResult_None;
    for (;found_match == FindResult_None;){
        found_match = search_front_to_back_step(app, range, word, pos, result_ptr);
    }
    return(found_match);
}

static int32_t
search_back_to_front_step(Application_Links *app, Search_Range *range, String word, int32_t *pos, Search_Match *result_ptr){
    int32_t found_match = FindResult_None;
    
    Search_Match result = *result_ptr;
    
    if (*pos > range->start){
        int32_t start_pos = *pos;
        
        result.buffer = get_buffer(app, range->buffer, AccessAll);
        buffer_seek_string_backward(app, &result.buffer,
                                    start_pos, range->start,
                                    word.str, word.size,
                                    &result.start);
        
        // TODO(allen): deduplicate the match checking code.
        if (result.start >= range->start){
            *pos = result.start - 1;
            found_match = match_check(app, range, pos, &result, word);
            if (found_match == FindResult_FoundMatch){
                *pos = result.start - word.size;
            }
        }
        else{
            found_match = FindResult_PastEnd;
        }
    }
    else{
        found_match = FindResult_PastEnd;
    }
    
    *result_ptr = result;
    
    return(found_match);
}

static int32_t
search_back_to_front(Application_Links *app, Search_Range *range, String word, int32_t *pos, Search_Match *result_ptr){
    int32_t found_match = FindResult_None;
    for (;found_match == FindResult_None;){
        found_match = search_back_to_front_step(app, range, word, pos, result_ptr);
    }
    return(found_match);
}

static Search_Match
search_next_match(Application_Links *app, Search_Set *set, Search_Iter *it_ptr){
    Search_Match result = {0};
    Search_Iter iter = *it_ptr;
    
    int32_t count = set->count;
    for (; iter.i < count;){
        Search_Range *range = set->ranges + iter.i;
        
        int32_t find_result = FindResult_None;
        
        if (!iter.range_initialized){
            iter.range_initialized = true;
            switch (range->type){
                case SearchRange_BackToFront:
                {
                    iter.back_pos = range->start+range->size-1;
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
                find_result =
                    search_front_to_back(app, range,
                                         iter.word,
                                         &iter.pos,
                                         &result);
            }break;
            
            case SearchRange_BackToFront:
            {
                find_result =
                    search_back_to_front(app, range,
                                         iter.word,
                                         &iter.back_pos,
                                         &result);
            }break;
            
            case SearchRange_Wave:
            {
                Search_Match forward_match = {0};
                Search_Match backward_match = {0};
                
                int32_t forward_result = FindResult_PastEnd;
                int32_t backward_result = FindResult_PastEnd;
                
                if (iter.pos < range->start + range->size){
                    forward_result = search_front_to_back(app, range,
                                                          iter.word,
                                                          &iter.pos,
                                                          &forward_match);
                }
                
                if (iter.back_pos > range->start){
                    backward_result = search_back_to_front(app, range,
                                                           iter.word,
                                                           &iter.back_pos,
                                                           &backward_match);
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

#endif
