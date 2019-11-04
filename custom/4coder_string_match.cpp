/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.06.2019
 *
 * Routines for operating on the String_Match and String_Match_List types.
 *
 */

// TOP

internal void
string_match_list_push(Arena *arena, String_Match_List *list,
                       Buffer_ID buffer, i32 string_id, String_Match_Flag flags, Range_i64 range){
    String_Match *match = push_array(arena, String_Match, 1);
    sll_queue_push(list->first, list->last, match);
    list->count += 1;
    match->buffer = buffer;
    match->string_id = string_id;
    match->flags = flags;
    match->range = range;
}

internal void
string_match_list_push(Arena *arena, String_Match_List *list,
                       Buffer_ID buffer, i32 string_id, String_Match_Flag flags, i64 start, i64 length){
    string_match_list_push(arena, list, buffer, string_id, flags,
                           Ii64(start, start + length));
}

internal String_Match_List
string_match_list_join(String_Match_List *a, String_Match_List *b){
    String_Match_List list = *a;
    block_zero_struct(a);
    if (list.last != 0){
        list.last->next = b->first;
        if (b->last != 0){
            list.last = b->last;
        }
    }
    else{
        list.first = b->first;
        list.last = b->last;
    }
    list.count += b->count;
    block_zero_struct(b);
    return(list);
}

internal void
string_match_list_filter_flags(String_Match_List *list, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags){
    String_Match_List new_list = {};
    if ((must_have_flags & must_not_have_flags) == 0){
        for (String_Match *node = list->first, *next = 0;
             node != 0;
             node = next){
            next = node->next;
            if ((node->flags & must_have_flags) == must_have_flags && (node->flags & must_not_have_flags) == 0){
                sll_queue_push(new_list.first, new_list.last, node);
                new_list.count += 1;
            }
        }
    }
    *list = new_list;
}

internal void
string_match_list_filter_remove_buffer(String_Match_List *list, Buffer_ID buffer){
    String_Match_List new_list = {};
    for (String_Match *node = list->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        if (node->buffer != buffer){
            sll_queue_push(new_list.first, new_list.last, node);
            new_list.count += 1;
        }
    }
    *list = new_list;
}

internal void
string_match_list_filter_remove_buffer_predicate(Application_Links *app, String_Match_List *list, Buffer_Predicate *predicate){
    String_Match_List new_list = {};
    for (String_Match *node = list->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        if (!predicate(app, node->buffer)){
            sll_queue_push(new_list.first, new_list.last, node);
            new_list.count += 1;
        }
    }
    *list = new_list;
}

internal String_Match_List
string_match_list_merge_nearest(String_Match_List *a, String_Match_List *b, Range_i64 range){
    String_Match_List list = {};
    String_Match *node_a = a->first;
    String_Match *node_b = b->first;
    for (String_Match *next_a = node_a, *next_b = node_b;
         node_a != 0 && node_b != 0;
         node_a = next_a, node_b = next_b){
        i64 dist_a = range_distance(node_a->range, range);
        i64 dist_b = range_distance(node_b->range, range);
        if (dist_a <= dist_b){
            next_a = next_a->next;
            sll_queue_push(list.first, list.last, node_a);
            list.count += 1;
        }
        else{
            next_b = next_b->next;
            sll_queue_push(list.first, list.last, node_b);
            list.count += 1;
        }
    }
    Assert(node_a == 0 || node_b == 0);
    // TODO(allen): this is dumb O(n) work that could be O(1)
    String_Match *node = 0;
    if (node_a != 0){
        node = node_a;
    }
    else if (node_b != 0){
        node = node_b;
    }
    for (String_Match *next = 0;
         node != 0;
         node = next){
        next = node->next;
        sll_queue_push(list.first, list.last, node);
        list.count += 1;
    }
    block_zero_struct(a);
    block_zero_struct(b);
    return(list);
}

internal String_Match_List
string_match_list_merge_front_to_back(String_Match_List *a, String_Match_List *b){
    return(string_match_list_merge_nearest(a, b, Ii64((i64)0)));
}

// BOTTOM

