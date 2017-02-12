/*
4coder_search.cpp - Commands that search accross buffers including word complete,
and list all locations.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_SEARCH)
#define FCODER_SEARCH

#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_mem.h"

#include "4coder_helper/4coder_streaming.h"
#include "4coder_helper/4coder_long_seek.h"

#include "4coder_default_framework.h"

//
// Search Iteration Systems
//

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
    
    Assert(len != 0);
    
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

//
// Generic Search All Buffers
//

static void
get_search_all_string(Application_Links *app, Query_Bar *bar){
    char string_space[1024];
    bar->prompt = make_lit_string("List Locations For: ");
    bar->string = make_fixed_width_string(string_space);
    
    if (!query_user_string(app, bar)){
        bar->string.size = 0;
    }
}

static void
generic_search_all_buffers(Application_Links *app, General_Memory *general, Partition *part, String string, uint32_t match_flags){
    Search_Set set = {0};
    Search_Iter iter = {0};
    
    search_iter_init(general, &iter, string.size);
    copy_ss(&iter.word, string);
    
    int32_t buffer_count = get_buffer_count(app);
    search_set_init(general, &set, buffer_count);
    
    Search_Range *ranges = set.ranges;
    
    String search_name = make_lit_string("*search*");
    Buffer_Summary search_buffer = get_buffer_by_name(app, search_name.str, search_name.size, AccessAll);
    if (!search_buffer.exists){
        search_buffer = create_buffer(app, search_name.str, search_name.size, BufferCreate_AlwaysNew);
        buffer_set_setting(app, &search_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, &search_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, &search_buffer, BufferSetting_WrapLine, false);
    }
    else{
        buffer_replace_range(app, &search_buffer, 0, search_buffer.size, 0, 0);
    }
    
    {
        View_Summary view = get_active_view(app, AccessProtected);
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
        
        int32_t j = 0;
        if (buffer.exists){
            if (buffer.buffer_id != search_buffer.buffer_id){
                ranges[0].type = SearchRange_FrontToBack;
                ranges[0].flags = match_flags;
                ranges[0].buffer = buffer.buffer_id;
                ranges[0].start = 0;
                ranges[0].size = buffer.size;
                j = 1;
            }
        }
        
        for (Buffer_Summary buffer_it = get_buffer_first(app, AccessAll);
             buffer_it.exists;
             get_buffer_next(app, &buffer_it, AccessAll)){
            if (buffer.buffer_id != buffer_it.buffer_id){
                if (search_buffer.buffer_id != buffer_it.buffer_id){
                    ranges[j].type = SearchRange_FrontToBack;
                    ranges[j].flags = match_flags;
                    ranges[j].buffer = buffer_it.buffer_id;
                    ranges[j].start = 0;
                    ranges[j].size = buffer_it.size;
                    ++j;
                }
            }
        }
        set.count = j;
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    Partition line_part = partition_sub_part(part, (4 << 10));
    char *str = (char*)partition_current(part);
    int32_t part_size = 0;
    int32_t size = 0;
    for (;;){
        Search_Match match = search_next_match(app, &set, &iter);
        if (match.found_match){
            Partial_Cursor word_pos = {0};
            if (buffer_compute_cursor(app, &match.buffer, seek_pos(match.start), &word_pos)){
                char *file_name = match.buffer.file_name;
                int32_t file_len = match.buffer.file_name_len;
                if (file_name != 0){
                    file_name = match.buffer.buffer_name;
                    file_len = match.buffer.buffer_name_len;
                }
                
                int32_t line_num_len = int_to_str_size(word_pos.line);
                int32_t column_num_len = int_to_str_size(word_pos.character);
                
                Temp_Memory line_temp = begin_temp_memory(&line_part);
                String line_str = {0};
                read_line(app, &line_part, &match.buffer, word_pos.line, &line_str);
                line_str = skip_chop_whitespace(line_str);
                
                int32_t str_len = file_len + 1 + line_num_len + 1 + column_num_len + 1 + 1 + line_str.size + 1;
                
                char *spare = push_array(part, char, str_len);
                
                if (spare == 0){
                    buffer_replace_range(app, &search_buffer, size, size, str, part_size);
                    size += part_size;
                    
                    end_temp_memory(temp);
                    temp = begin_temp_memory(part);
                    
                    part_size = 0;
                    spare = push_array(part, char, str_len);
                }
                
                part_size += str_len;
                
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
                
                end_temp_memory(line_temp);
            }
        }
        else{
            break;
        }
    }
    
    buffer_replace_range(app, &search_buffer, size, size, str, part_size);
    
    View_Summary view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, search_buffer.buffer_id, 0);
    
    lock_jump_buffer(search_name.str, search_name.size);
    
    end_temp_memory(temp);
}

//
// List Commands
//

CUSTOM_COMMAND_SIG(list_all_locations){
    Query_Bar bar;
    get_search_all_string(app, &bar);
    if (bar.string.size == 0) return;
    generic_search_all_buffers(app, &global_general, &global_part, bar.string, SearchFlag_MatchWholeWord);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations){
    Query_Bar bar;
    get_search_all_string(app, &bar);
    if (bar.string.size == 0) return;
    generic_search_all_buffers(app, &global_general, &global_part, bar.string, SearchFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive){
    Query_Bar bar;
    get_search_all_string(app, &bar);
    if (bar.string.size == 0) return;
    generic_search_all_buffers(app, &global_general, &global_part, bar.string, SearchFlag_CaseInsensitive | SearchFlag_MatchWholeWord);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive){
    Query_Bar bar;
    get_search_all_string(app, &bar);
    if (bar.string.size == 0) return;
    generic_search_all_buffers(app, &global_general, &global_part, bar.string, SearchFlag_CaseInsensitive | SearchFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    Cpp_Get_Token_Result get_result = {0};
    bool32 success = buffer_get_token_index(app, &buffer, view.cursor.pos, &get_result);
    
    if (success && !get_result.in_whitespace){
        char space[128];
        int32_t size = get_result.token_end - get_result.token_start;
        
        if (size > 0 && size <= sizeof(space)){
            success = buffer_read_range(app, &buffer, get_result.token_start, get_result.token_end, space);
            if (success){
                String str = make_string(space, size);
                exec_command(app, change_active_panel);
                generic_search_all_buffers(app, &global_general, &global_part, str, SearchFlag_MatchWholeWord);
            }
        }
    }
}

//
// Word Complete Command
//

struct Word_Complete_State{
    Search_Set set;
    Search_Iter iter;
    Table hits;
    String_Space str;
    int32_t word_start;
    int32_t word_end;
    int32_t initialized;
};

static Word_Complete_State complete_state = {0};

CUSTOM_COMMAND_SIG(word_complete){
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
                        if (char_is_alpha(c)){
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
            
            // NOTE(allen): Initialize the search iterator
            // with the partial word.
            complete_state.initialized = true;
            search_iter_init(&global_general, &complete_state.iter, size);
            buffer_read_range(app, &buffer, word_start, word_end,
                              complete_state.iter.word.str);
            complete_state.iter.word.size = size;
            
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
            search_hits_init(&global_general, &complete_state.hits, &complete_state.str,
                             100, (4 << 10));
            search_hit_add(&global_general, &complete_state.hits, &complete_state.str,
                           complete_state.iter.word.str,
                           complete_state.iter.word.size);
            
            complete_state.word_start = word_start;
            complete_state.word_end = word_end;
        }
        else{
            word_start = complete_state.word_start;
            word_end = complete_state.word_end;
            size = complete_state.iter.word.size;
        }
        
        // NOTE(allen): Iterate through matches.
        if (size > 0){
            for (;;){
                int32_t match_size = 0;
                Search_Match match =
                    search_next_match(app, &complete_state.set,
                                      &complete_state.iter);
                
                if (match.found_match){
                    match_size = match.end - match.start;
                    Temp_Memory temp = begin_temp_memory(&global_part);
                    char *spare = push_array(&global_part, char, match_size);
                    
                    buffer_read_range(app, &match.buffer,
                                      match.start, match.end, spare);
                    
                    if (search_hit_add(&global_general, &complete_state.hits, &complete_state.str,
                                       spare, match_size)){
                        buffer_replace_range(app, &buffer, word_start, word_end,
                                             spare, match_size);
                        view_set_cursor(app, &view,
                                        seek_pos(word_start + match_size),
                                        true);
                        
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
                    
                    search_hits_init(&global_general, &complete_state.hits, &complete_state.str,
                                     100, (4 << 10));
                    search_hit_add(&global_general, &complete_state.hits, &complete_state.str,
                                   complete_state.iter.word.str,
                                   complete_state.iter.word.size);
                    
                    match_size = complete_state.iter.word.size;
                    char *str = complete_state.iter.word.str;
                    buffer_replace_range(app, &buffer, word_start, word_end,
                                         str, match_size);
                    view_set_cursor(app, &view,
                                    seek_pos(word_start + match_size),
                                    true);
                    
                    complete_state.word_end = word_start + match_size;
                    complete_state.set.ranges[0].mid_size = match_size;
                    break;
                }
            }
        }
    }
}

#endif

// BOTTOM

