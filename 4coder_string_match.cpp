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
                           make_range_i64(start, start + length));
}

internal void
string_match_list_join(String_Match_List *dst, String_Match_List *src){
    if (dst->last != 0){
        dst->last->next = src->first;
    }
    if (src->last != 0){
        dst->last = src->last;
    }
    dst->count += src->count;
    block_zero_struct(src);
}

// BOTTOM