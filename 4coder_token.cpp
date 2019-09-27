/*
 * 4coder token types
 */

// TOP

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

// BOTTOM

