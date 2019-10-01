/*
4coder_search.cpp - Commands that search accross buffers including word complete,
and list all locations.
*/

// TOP

//
// Search Iteration Systems
//

global String_Const_u8 search_name = string_u8_litexpr("*search*");

internal void
print_string_match_list_to_buffer(Application_Links *app, Buffer_ID out_buffer_id, String_Match_List matches){
    Scratch_Block scratch(app);
    clear_buffer(app, out_buffer_id);
    Buffer_Insertion out = begin_buffer_insertion_at_buffered(app, out_buffer_id, 0, scratch, KB(64));
    buffer_set_setting(app, out_buffer_id, BufferSetting_ReadOnly, true);
    buffer_set_setting(app, out_buffer_id, BufferSetting_RecordsHistory, false);
    
    Temp_Memory buffer_name_restore_point = begin_temp(scratch);
    String_Const_u8 current_file_name = {};
    Buffer_ID current_buffer = 0;
    
    if (matches.first != 0){
        for (String_Match *node = matches.first;
             node != 0;
             node = node->next){
            if (node->buffer != out_buffer_id){
                if (current_buffer != 0 && current_buffer != node->buffer){
                    insertc(&out, '\n');
                }
                if (current_buffer != node->buffer){
                    end_temp(buffer_name_restore_point);
                    current_buffer = node->buffer;
                    current_file_name = push_buffer_file_name(app, scratch, current_buffer);
                    if (current_file_name.size == 0){
                        current_file_name = push_buffer_unique_name(app, scratch, current_buffer);
                    }
                }
                
                Buffer_Cursor cursor = buffer_compute_cursor(app, current_buffer, seek_pos(node->range.first));
                Temp_Memory line_temp = begin_temp(scratch);
                String_Const_u8 full_line_str = push_buffer_line(app, scratch, current_buffer, cursor.line);
                String_Const_u8 line_str = string_skip_chop_whitespace(full_line_str);
                insertf(&out, "%.*s:%d:%d: %.*s\n",
                        string_expand(current_file_name), cursor.line, cursor.col,
                        string_expand(line_str));
                end_temp(line_temp);
            }
        }
    }
    else{
        insertf(&out, "no matches");
    }
    
    end_buffer_insertion(&out);
}

internal void
print_all_matches_all_buffers(Application_Links *app, String_Const_u8_Array match_patterns, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, Buffer_ID out_buffer_id){
    Scratch_Block scratch(app);
    String_Match_List matches = find_all_matches_all_buffers(app, scratch, match_patterns, must_have_flags, must_not_have_flags);
    string_match_list_filter_remove_buffer(&matches, out_buffer_id);
    string_match_list_filter_remove_buffer_predicate(app, &matches, buffer_has_name_with_star);
    print_string_match_list_to_buffer(app, out_buffer_id, matches);
}

internal void
print_all_matches_all_buffers(Application_Links *app, String_Const_u8 pattern, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, Buffer_ID out_buffer_id){
    String_Const_u8_Array array = {&pattern, 1};
    print_all_matches_all_buffers(app, array, must_have_flags, must_not_have_flags, out_buffer_id);
}

internal void
print_all_matches_all_buffers_to_search(Application_Links *app, String_Const_u8_Array match_patterns, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, View_ID default_target_view){
    Buffer_ID search_buffer = create_or_switch_to_buffer_and_clear_by_name(app, search_name, default_target_view);
    print_all_matches_all_buffers(app, match_patterns, must_have_flags, must_not_have_flags, search_buffer);
}

internal void
print_all_matches_all_buffers_to_search(Application_Links *app, String_Const_u8 pattern, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, View_ID default_target_view){
    String_Const_u8_Array array = {&pattern, 1};
    print_all_matches_all_buffers_to_search(app, array, must_have_flags, must_not_have_flags, default_target_view);
}

internal String_Const_u8
query_user_list_needle(Application_Links *app, Arena *arena){
    u8 *space = push_array(arena, u8, KB(1));
    return(get_query_string(app, "List Locations For: ", space, KB(1)));
}

internal String_Const_u8_Array
user_list_definition_array(Application_Links *app, Arena *arena, String_Const_u8 base_needle){
    String_Const_u8_Array result = {};
    if (base_needle.size > 0){
        result.count = 9;
        result.vals = push_array(arena, String_Const_u8, result.count);
        i32 i = 0;
        result.vals[i++] = (push_u8_stringf(arena, "struct %.*s{"  , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "struct %.*s\n{", string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "struct %.*s {" , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "union %.*s{"   , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "union %.*s\n{" , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "union %.*s {"  , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "enum %.*s{"    , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "enum %.*s\n{"  , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "enum %.*s {"   , string_expand(base_needle)));
        Assert(i == result.count);
    }
    return(result);
}

internal String_Const_u8_Array
query_user_list_definition_needle(Application_Links *app, Arena *arena){
    u8 *space = push_array(arena, u8, KB(1));
    String_Const_u8 base_needle = get_query_string(app, "List Definitions For: ", space, KB(1));
    return(user_list_definition_array(app, arena, base_needle));
}

internal void
list_all_locations__generic(Application_Links *app, String_Const_u8_Array needle, List_All_Locations_Flag flags){
    if (needle.count > 0){
        View_ID target_view = get_next_view_after_active(app, AccessAll);
        String_Match_Flag must_have_flags = 0;
        String_Match_Flag must_not_have_flags = 0;
        if (HasFlag(flags, ListAllLocationsFlag_CaseSensitive)){
            AddFlag(must_have_flags, StringMatch_CaseSensitive);
        }
        if (!HasFlag(flags, ListAllLocationsFlag_MatchSubstring)){
            AddFlag(must_not_have_flags, StringMatch_LeftSideSloppy);
            AddFlag(must_not_have_flags, StringMatch_RightSideSloppy);
        }
        print_all_matches_all_buffers_to_search(app, needle, must_have_flags, must_not_have_flags, target_view);
    }
}

internal void
list_all_locations__generic(Application_Links *app, String_Const_u8 needle, List_All_Locations_Flag flags){
    String_Const_u8_Array array = {&needle, 1};
    list_all_locations__generic(app, array, flags);
}

internal void
list_all_locations__generic_query(Application_Links *app, List_All_Locations_Flag flags){
    Scratch_Block scratch(app);
    String_Const_u8 needle = query_user_list_needle(app, scratch);
    list_all_locations__generic(app, needle, flags);
}

internal void
list_all_locations__generic_identifier(Application_Links *app, List_All_Locations_Flag flags){
    Scratch_Block scratch(app);
    String_Const_u8 needle = push_token_or_word_under_active_cursor(app, scratch);
    list_all_locations__generic(app, needle, flags);
}

internal void
list_all_locations__generic_view_range(Application_Links *app, List_All_Locations_Flag flags){
    Scratch_Block scratch(app);
    String_Const_u8 needle = push_view_range_string(app, scratch);
    list_all_locations__generic(app, needle, flags);
}

CUSTOM_COMMAND_SIG(list_all_locations)
CUSTOM_DOC("Queries the user for a string and lists all exact case-sensitive matches found in all open buffers.")
{
    list_all_locations__generic_query(app, ListAllLocationsFlag_CaseSensitive);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations)
CUSTOM_DOC("Queries the user for a string and lists all case-sensitive substring matches found in all open buffers.")
{
    list_all_locations__generic_query(app, ListAllLocationsFlag_CaseSensitive|ListAllLocationsFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.")
{
    list_all_locations__generic_query(app, 0);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive)
CUSTOM_DOC("Queries the user for a string and lists all case-insensitive substring matches found in all open buffers.")
{
    list_all_locations__generic_query(app, ListAllLocationsFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-sensitive mathces in all open buffers.")
{
    list_all_locations__generic_identifier(app, ListAllLocationsFlag_CaseSensitive);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_identifier_case_insensitive)
CUSTOM_DOC("Reads a token or word under the cursor and lists all exact case-insensitive mathces in all open buffers.")
{
    list_all_locations__generic_identifier(app, ListAllLocationsFlag_CaseSensitive|ListAllLocationsFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-sensitive mathces in all open buffers.")
{
    list_all_locations__generic_view_range(app, ListAllLocationsFlag_CaseSensitive);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_selection_case_insensitive)
CUSTOM_DOC("Reads the string in the selected range and lists all exact case-insensitive mathces in all open buffers.")
{
    list_all_locations__generic_view_range(app, 0);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition)
CUSTOM_DOC("Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.")
{
    Scratch_Block scratch(app);
    String_Const_u8_Array array = query_user_list_definition_needle(app, scratch);
    list_all_locations__generic(app, array, ListAllLocationsFlag_CaseSensitive|ListAllLocationsFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.")
{
    Scratch_Block scratch(app);
    String_Const_u8 base_needle = push_token_or_word_under_active_cursor(app, scratch);
    String_Const_u8_Array array = user_list_definition_array(app, scratch, base_needle);
    list_all_locations__generic(app, array, ListAllLocationsFlag_CaseSensitive|ListAllLocationsFlag_MatchSubstring);
}

internal Range_i64
get_word_complete_needle_range(Application_Links *app, Buffer_ID buffer, i64 pos){
    Range_i64 needle_range = {};
    needle_range.max = pos;
    needle_range.min = scan(app, boundary_alpha_numeric_underscore_utf8, buffer, Scan_Backward, pos);
    i64 e = scan(app, boundary_alpha_numeric_underscore_utf8, buffer, Scan_Forward, needle_range.min);
    if (needle_range.max > e){
        needle_range = make_range_i64(pos);
    }
    return(needle_range);
}

internal void
string_match_list_enclose_all(Application_Links *app, String_Match_List list, Enclose_Function *enclose){
    for (String_Match *node = list.first;
         node != 0;
         node = node->next){
        node->range = enclose(app, node->buffer, node->range);
    }
}

internal List_String_Const_u8
string_match_list_deduplicated_strings(Application_Links *app, Arena *arena, String_Match_List list){
    List_String_Const_u8 extension_list = {};
    Thread_Context *tctx = get_thread_context(app);
    Table_Data_u64 table = make_table_Data_u64(tctx->allocator, 128);
    for (String_Match *node = list.first;
         node != 0;
         node = node->next){
        Temp_Memory restore_point = begin_temp(arena);
        String_Const_u8 str = push_buffer_range(app, arena, node->buffer, node->range);
        if (table_insert(&table, make_data(str.str, str.size), 1)){
            string_list_push(arena, &extension_list, str);
        }
        else{
            end_temp(restore_point);
        }
    }
    table_free(&table);
    return(extension_list);
}

internal void
string_match_list_deduplicate(Application_Links *app, String_Match_List *list){
    String_Match_List new_list = {};
    Thread_Context *tctx = get_thread_context(app);
    Scratch_Block scratch(tctx);
    Table_Data_u64 table = make_table_Data_u64(tctx->allocator, 128);
    for (String_Match *node = list->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        Temp_Memory restore_point = begin_temp(scratch);
        String_Const_u8 str = push_buffer_range(app, scratch, node->buffer, node->range);
        if (table_insert(&table, make_data(str.str, str.size), 1)){
            sll_queue_push(new_list.first, new_list.last, node);
            new_list.count += 1;
        }
        else{
            end_temp(restore_point);
        }
    }
    table_free(&table);
    *list = new_list;
}

global String_Match_Flag word_complete_must = StringMatch_CaseSensitive|StringMatch_RightSideSloppy;
global String_Match_Flag word_complete_must_not = StringMatch_LeftSideSloppy;

internal String_Match_List
get_word_complete_match_list__unreduced(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 needle_range, String_Const_u8 needle){
    if (needle.size == 0){
        needle = push_buffer_range(app, arena, buffer, needle_range);
    }
    local_persist Character_Predicate *pred = &character_predicate_alpha_numeric_underscore_utf8;
    
    i64 size = buffer_get_size(app, buffer);
    String_Match_List up = buffer_find_all_matches(app, arena, buffer, 0,
                                                   make_range_i64(0, needle_range.min),
                                                   needle, pred, Scan_Backward);
    String_Match_List down = buffer_find_all_matches(app, arena, buffer, 0,
                                                     make_range_i64(needle_range.max, size),
                                                     needle, pred, Scan_Forward);
    
    string_match_list_filter_flags(&up, word_complete_must, word_complete_must_not);
    string_match_list_filter_flags(&down, word_complete_must, word_complete_must_not);
    String_Match_List here = string_match_list_merge_nearest(&up, &down, needle_range);
    
    String_Match_List everywhere = find_all_matches_all_buffers(app, arena, needle, word_complete_must, word_complete_must_not);
    string_match_list_filter_remove_buffer(&everywhere, buffer);
    
    String_Match_List whole_list = string_match_list_join(&here, &everywhere);
    string_match_list_enclose_all(app, whole_list, enclose_alpha_numeric_underscore_utf8);
    
    return(whole_list);
}

internal String_Match_List
get_word_complete_match_list__unreduced(Application_Links *app, Arena *arena, String_Const_u8 needle){
    String_Match_List whole_list = find_all_matches_all_buffers(app, arena, needle, word_complete_must, word_complete_must_not);
    string_match_list_enclose_all(app, whole_list, enclose_alpha_numeric_underscore_utf8);
    return(whole_list);
}

internal String_Match_List
get_word_complete_match_list(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 needle_range, String_Const_u8 needle){
    String_Match_List whole_list = get_word_complete_match_list__unreduced(app, arena, buffer, needle_range, needle);
    string_match_list_deduplicate(app, &whole_list);
    return(whole_list);
}

internal String_Match_List
get_word_complete_match_list(Application_Links *app, Arena *arena, String_Const_u8 needle){
    String_Match_List whole_list = get_word_complete_match_list__unreduced(app, arena, needle);
    string_match_list_deduplicate(app, &whole_list);
    return(whole_list);
}

internal Word_Complete_State
get_word_complete_state(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 needle_range){
    String_Const_u8 needle = push_buffer_range(app, arena, buffer, needle_range);
    
    Scratch_Block scratch(app);
    String_Match_List whole_list = get_word_complete_match_list__unreduced(app, arena, buffer, needle_range, needle);
    
    Word_Complete_State state = {};
    state.initialized = true;
    state.needle = needle;
    state.range = needle_range;
    state.list = string_match_list_deduplicated_strings(app, arena, whole_list);
    state.iterator = 0;
    return(state);
}

internal List_String_Const_u8
get_word_complete_list(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 needle_range){
    Word_Complete_State state = get_word_complete_state(app, arena, buffer, needle_range);
    return(state.list);
}

internal List_String_Const_u8
get_word_complete_list(Application_Links *app, Arena *arena, String_Const_u8 needle){
    Scratch_Block scratch(app);
    String_Match_List whole_list = get_word_complete_match_list__unreduced(app, scratch, needle);
    return(string_match_list_deduplicated_strings(app, arena, whole_list));
}

CUSTOM_COMMAND_SIG(word_complete)
CUSTOM_DOC("Iteratively tries completing the word to the left of the cursor with other words in open buffers that have the same prefix string.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    if (buffer != 0){
        Managed_Scope scope = view_get_managed_scope(app, view);
        
        b32 first_completion = false;
        Rewrite_Type *rewrite = scope_attachment(app, scope, view_rewrite_loc, Rewrite_Type);
        if (*rewrite != Rewrite_WordComplete){
            first_completion = true;
        }
        
        Rewrite_Type *next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
        *next_rewrite = Rewrite_WordComplete;
        
        local_persist Arena *completion_arena = {};
        if (completion_arena == 0){
            completion_arena = reserve_arena(app);
        }
        local_persist Word_Complete_State state = {};
        
        if (first_completion || !state.initialized){
            block_zero_struct(&state);
            linalloc_clear(completion_arena);
            
            i64 pos = view_get_cursor_pos(app, view);
            Range_i64 needle_range = get_word_complete_needle_range(app, buffer, pos);
            if (range_size(needle_range) > 0){
                state = get_word_complete_state(app, completion_arena, buffer, needle_range);
            }
        }
        
        if (state.initialized){
            if (state.iterator == 0){
                state.iterator = state.list.first;
            }
            else{
                state.iterator = state.iterator->next;
            }
            
            String_Const_u8 str = {};
            if (state.iterator == 0){
                str = state.needle;
            }
            else{
                str = state.iterator->string;
            }
            
            buffer_replace_range(app, buffer, state.range, str);
            state.range.max = state.range.min + str.size;
            view_set_cursor_and_preferred_x(app, view, seek_pos(state.range.max));
        }
    }
}

// BOTTOM

