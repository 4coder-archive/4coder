/*
 * 4coder token types
 */

// TOP

function Range_i64
Ii64(Token *token){
    return(Ii64_size(token->pos, token->size));
}

internal void
token_list_push(Arena *arena, Token_List *list, Token *token){
    Token_Block *block = list->last;
    if (block == 0 || block->count + 1 > block->max){
        block = push_array(arena, Token_Block, 1);
        block->next = 0;
        block->prev = 0;
        u32 new_max = round_up_u32(1, KB(4));
        block->tokens = push_array(arena, Token, new_max);
        block->count = 0;
        block->max = new_max;
        zdll_push_back(list->first, list->last, block);
        list->node_count += 1;
    }
    block_copy_struct(&block->tokens[block->count], token);
    block->count += 1;
    list->total_count += 1;
}

internal void
token_fill_memory_from_list(Token *dst, Token_List *list, i64 count){
    Token *ptr = dst;
    for (Token_Block *node = list->first;
         node != 0 && count > 0;
         node = node->next){
        i64 write_count = clamp_top(node->count, count);
        block_copy_dynamic_array(ptr, node->tokens, write_count);
        ptr += write_count;
        count -= write_count;
    }
}

internal void
token_fill_memory_from_list(Token *dst, Token_List *list){
    token_fill_memory_from_list(dst, list, list->total_count);
}

internal Token_Array
token_array_from_list_always_copy(Arena *arena, Token_List *list){
    Token_Array array = {};
    if (list->node_count >= 1){
        array.tokens = push_array(arena, Token, list->total_count);
        token_fill_memory_from_list(array.tokens, list);
        array.count = list->total_count;
        array.max = array.count;
    }
    return(array);
}

internal Token_Array
token_array_from_list(Arena *arena, Token_List *list){
    Token_Array array = {};
    if (list->node_count > 1){
        array.tokens = push_array(arena, Token, list->total_count);
        token_fill_memory_from_list(array.tokens, list);
        array.count = list->total_count;
        array.max = array.count;
    }
    else if (list->node_count == 1){
        array.tokens = list->first->tokens;
        array.count = list->first->count;
        array.max = list->first->max;
    }
    return(array);
}

internal i64
token_index_from_pos(Token *tokens, i64 count, i64 pos){
    i64 result = 0;
    if (count > 0){
        if (pos >= tokens[count - 1].pos){
            result = count - 1;
        }
        else if (pos <= tokens[0].pos){
            result = 0;
        }
        else{
            i64 first = 0;
            i64 one_past_last = count;
            for (;;){
                i64 index = (first + one_past_last) >> 1;
                i64 index_pos = tokens[index].pos;
                if (index_pos > pos){
                    one_past_last = index;
                }
                else if (index_pos + tokens[index].size <= pos){
                    first = index + 1;
                }
                else{
                    result = index;
                    break;
                }
            }
        }
    }
    return(result);
}

internal i64
token_index_from_pos(Token_Array *tokens, u64 pos){
    return(token_index_from_pos(tokens->tokens, tokens->count, pos));
}

internal Token*
token_from_pos(Token *tokens, i64 count, i64 pos){
    i64 index = token_index_from_pos(tokens, count, pos);
    return(tokens + index);
}

internal Token*
token_from_pos(Token_Array *tokens, u64 pos){
    i64 index = token_index_from_pos(tokens, pos);
    return(tokens->tokens + index);
}

////////////////////////////////

internal Token_Iterator_Array
token_iterator_index(u64 user_id, Token *tokens, i64 count, i64 token_index){
    Token_Iterator_Array it = {};
    if (tokens != 0){
        it.user_id = user_id;
        it.ptr = tokens + token_index;
        it.tokens = tokens;
        it.count = count;
    }
    return(it);
}

internal Token_Iterator_Array
token_iterator_index(u64 user_id, Token_Array *tokens, i64 token_index){
    return(token_iterator_index(user_id, tokens->tokens, tokens->count, token_index));
}

internal Token_Iterator_Array
token_iterator(u64 user_id, Token *tokens, i64 count, Token *token){
    return(token_iterator_index(user_id, tokens, count, (i64)(token - tokens)));
}

internal Token_Iterator_Array
token_iterator(u64 user_id, Token_Array *tokens, Token *token){
    return(token_iterator_index(user_id, tokens->tokens, tokens->count, (i64)(token - tokens->tokens)));
}

internal Token_Iterator_Array
token_iterator(u64 user_id, Token *tokens, i64 count){
    return(token_iterator_index(user_id, tokens, count, 0));
}

internal Token_Iterator_Array
token_iterator(u64 user_id, Token_Array *tokens){
    return(token_iterator_index(user_id, tokens->tokens, tokens->count, 0));
}

internal Token_Iterator_Array
token_iterator_pos(u64 user_id, Token *tokens, i64 count, i64 pos){
    i64 index = token_index_from_pos(tokens, count, pos);
    return(token_iterator_index(user_id, tokens, count, index));
}

internal Token_Iterator_Array
token_iterator_pos(u64 user_id, Token_Array *tokens, i64 pos){
    i64 index = token_index_from_pos(tokens->tokens, tokens->count, pos);
    return(token_iterator_index(user_id, tokens->tokens, tokens->count, index));
}

internal Token*
token_it_read(Token_Iterator_Array *it){
    Token *result = 0;
    if (it->tokens != 0){
        result = it->ptr;
    }
    return(result);
}

internal i64
token_it_index(Token_Iterator_Array *it){
    return((i64)(it->ptr - it->tokens));
}

internal b32
token_it_inc_all(Token_Iterator_Array *it){
    b32 result = false;
    if (it->tokens != 0){
        if (it->ptr < it->tokens + it->count - 1){
            it->ptr += 1;
            result = true;
        }
    }
    return(result);
}

internal b32
token_it_dec_all(Token_Iterator_Array *it){
    b32 result = false;
    if (it->tokens != 0){
        if (it->ptr > it->tokens){
            it->ptr -= 1;
            result = true;
        }
    }
    return(result);
}

internal b32
token_it_inc_non_whitespace(Token_Iterator_Array *it){
    b32 result = false;
    repeat:
    if (token_it_inc_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && token->kind == TokenBaseKind_Whitespace){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal b32
token_it_dec_non_whitespace(Token_Iterator_Array *it){
    b32 result = false;
    repeat:
    if (token_it_dec_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && token->kind == TokenBaseKind_Whitespace){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal b32
token_it_inc(Token_Iterator_Array *it){
    b32 result = false;
    repeat:
    if (token_it_inc_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && (token->kind == TokenBaseKind_Whitespace ||
                           token->kind == TokenBaseKind_Comment)){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal b32
token_it_dec(Token_Iterator_Array *it){
    b32 result = false;
    repeat:
    if (token_it_dec_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && (token->kind == TokenBaseKind_Whitespace ||
                           token->kind == TokenBaseKind_Comment)){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal Token_Iterator_List
token_iterator_index(u64 user_id, Token_List *list, i64 index){
    Token_Iterator_List it = {};
    if (list->first != 0){
        index = clamp(0, index, list->total_count - 1);
        i64 base_index = 0;
        Token_Block *block = 0;
        for (Token_Block *node = list->first;
             node != 0;
             node = node->next){
            if (index < base_index + node->count){
                block = node;
                break;
            }
            base_index += node->count;
        }
        Assert(block != 0);
        it.user_id = user_id;
        it.index = index;
        it.ptr = block->tokens + (index - base_index);
        it.block = block;
        it.first = list->first;
        it.last = list->last;
        it.node_count = list->node_count;
        it.total_count = list->total_count;
    }
    return(it);
}

internal Token_Iterator_List
token_iterator(u64 user_id, Token_List *list){
    return(token_iterator_index(user_id, list, 0));
}

internal Token_Iterator_List
token_iterator_pos(u64 user_id, Token_List *list, i64 pos){
    Token_Iterator_List it = {};
    if (list->first != 0){
        Token_Block *block = list->last;
        Token *token = &block->tokens[block->count - 1];
        i64 size = token->pos + token->size;
        pos = clamp(0, pos, size);
        i64 base_index = 0;
        block = 0;
        for (Token_Block *node = list->first;
             node != 0;
             node = node->next){
            Token *last_token = &node->tokens[node->count - 1];
            i64 one_past_last = last_token->pos + last_token->size;
            if (pos < one_past_last ||
                (node->next == 0 && pos == one_past_last)){
                block = node;
                break;
            }
            base_index += node->count;
        }
        Assert(block != 0);
        i64 sub_index = token_index_from_pos(block->tokens, block->count, pos);
        it.user_id = user_id;
        it.index = base_index + sub_index;
        it.ptr = block->tokens + sub_index;
        it.block = block;
        it.first = list->first;
        it.last = list->last;
        it.node_count = list->node_count;
        it.total_count = list->total_count;
    }
    return(it);
}

internal Token*
token_it_read(Token_Iterator_List *it){
    Token *result = 0;
    if (it->block != 0){
        result = it->ptr;
    }
    return(result);
}

internal i64
token_it_index(Token_Iterator_List *it){
    return(it->index);
}

internal b32
token_it_inc_all(Token_Iterator_List *it){
    b32 result = false;
    if (it->block != 0){
        i64 sub_index = (i64)(it->ptr - it->block->tokens);
        if (sub_index + 1 < it->block->count){
            it->index += 1;
            it->ptr += 1;
            result = true;
        }
        else{
            if (it->block->next != 0){
                it->block = it->block->next;
                it->index += 1;
                it->ptr = it->block->tokens;
                result = true;
            }
        }
    }
    return(result);
}

internal b32
token_it_dec_all(Token_Iterator_List *it){
    b32 result = false;
    if (it->block != 0){
        i64 sub_index = (i64)(it->ptr - it->block->tokens);
        if (sub_index > 0){
            it->index -= 1;
            it->ptr -= 1;
            result = true;
        }
        else{
            if (it->block->prev != 0){
                it->block = it->block->prev;
                it->index -= 1;
                it->ptr = it->block->tokens + it->block->count - 1;
                result = true;
            }
        }
    }
    return(result);
}

internal b32
token_it_inc_non_whitespace(Token_Iterator_List *it){
    b32 result = false;
    repeat:
    if (token_it_inc_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && token->kind == TokenBaseKind_Whitespace){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal b32
token_it_dec_non_whitespace(Token_Iterator_List *it){
    b32 result = false;
    repeat:
    if (token_it_dec_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && token->kind == TokenBaseKind_Whitespace){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal b32
token_it_inc(Token_Iterator_List *it){
    b32 result = false;
    repeat:
    if (token_it_inc_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && (token->kind == TokenBaseKind_Whitespace ||
                           token->kind == TokenBaseKind_Comment)){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal b32
token_it_dec(Token_Iterator_List *it){
    b32 result = false;
    repeat:
    if (token_it_dec_all(it)){
        Token *token = token_it_read(it);
        if (token != 0 && (token->kind == TokenBaseKind_Whitespace ||
                           token->kind == TokenBaseKind_Comment)){
            goto repeat;
        }
        result = true;
    }
    return(result);
}

internal Token_Iterator
token_iterator(Token_Iterator_Array it){
    Token_Iterator result = {};
    result.kind = TokenIterator_Array;
    result.array = it;
    return(result);
}

internal Token_Iterator
token_iterator(Token_Iterator_List it){
    Token_Iterator result = {};
    result.kind = TokenIterator_List;
    result.list = it;
    return(result);
}

internal Token*
token_it_read(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_read(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_read(&it->list));
        }break;
    }
    return(0);
}

internal i64
token_it_index(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_index(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_index(&it->list));
        }break;
    }
    return(0);
}

internal b32
token_it_inc_all(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_inc_all(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_inc_all(&it->list));
        }break;
    }
    return(0);
}

internal b32
token_it_dec_all(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_dec_all(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_dec_all(&it->list));
        }break;
    }
    return(0);
}

internal b32
token_it_inc_non_whitespace(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_inc_non_whitespace(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_inc_non_whitespace(&it->list));
        }break;
    }
    return(0);
}

internal b32
token_it_dec_non_whitespace(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_dec_non_whitespace(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_dec_non_whitespace(&it->list));
        }break;
    }
    return(0);
}

internal b32
token_it_inc(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_inc(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_inc(&it->list));
        }break;
    }
    return(0);
}

internal b32
token_it_dec(Token_Iterator *it){
    switch (it->kind){
        case TokenIterator_Array:
        {
            return(token_it_dec(&it->array));
        }break;
        case TokenIterator_List:
        {
            return(token_it_dec(&it->list));
        }break;
    }
    return(0);
}

////////////////////////////////

internal void
token_drop_eof(Token_List *list){
    if (list->last != 0){
        Token_Block *block = list->last;
        if (block->tokens[block->count - 1].kind == TokenBaseKind_EOF){
            list->total_count -= 1;
            block->count -= 1;
            if (block->count == 0){
                zdll_remove(list->first, list->last, block);
                list->node_count -= 1;
            }
        }
    }
}

////////////////////////////////

internal i64
token_relex_first(Token_Array *tokens, i64 edit_range_first, i64 backup_repeats){
    Token_Iterator_Array it = token_iterator_pos(0, tokens, edit_range_first);
    b32 good_status = true;
    for (i64 i = 0; i < backup_repeats && good_status; i += 1){
        good_status = token_it_dec(&it);
    }
    if (good_status){
        for (;;){
            Token *token = token_it_read(&it);
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
                break;
            }
            if (!token_it_dec(&it)){
                break;
            }
        }
    }
    return(token_it_index(&it));
}

internal i64
token_relex_resync(Token_Array *tokens, i64 edit_range_first, i64 look_ahead_repeats){
    Token_Iterator_Array it = token_iterator_pos(0, tokens, edit_range_first);
    b32 good_status = true;
    for (i64 i = 0; (i < look_ahead_repeats) && good_status; i += 1){
        good_status = token_it_inc(&it);
    }
    if (good_status){
        for (;;){
            Token *token = token_it_read(&it);
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
                break;
            }
            if (!token_it_inc(&it)){
                break;
            }
        }
    }
    return(token_it_index(&it));
}

internal Token_Relex
token_relex(Token_List relex_list, i64 new_pos_to_old_pos_shift, Token *tokens, i64 relex_first, i64 relex_last){
    Token_Relex relex = {};
    if (relex_list.total_count > 0){
        Token_Array relexed_tokens = {tokens + relex_first, relex_last - relex_first + 1};
        Token_Iterator_List it = token_iterator_index(0, &relex_list, relex_list.total_count - 1);
        for (;;){
            Token *token = token_it_read(&it);
            i64 new_pos_rebased = token->pos + new_pos_to_old_pos_shift;
            i64 old_token_index = token_index_from_pos(&relexed_tokens, new_pos_rebased);
            Token *old_token = relexed_tokens.tokens + old_token_index;
            if (new_pos_rebased == old_token->pos &&
                token->size == old_token->size &&
                token->kind == old_token->kind &&
                token->sub_kind == old_token->sub_kind &&
                token->flags == old_token->flags &&
                token->sub_flags == old_token->sub_flags){
                relex.successful_resync = true;
                relex.first_resync_index = relex_first + old_token_index;
            }
            else{
                break;
            }
            if (!token_it_dec_all(&it)){
                break;
            }
        }
    }
    return(relex);
}

// BOTTOM

