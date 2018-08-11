/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 22.06.2018
 *
 * Dynamic variable system
 *
 */

// TOP

internal void
dynamic_variables_init(Dynamic_Variable_Layout *layout){
    dll_init_sentinel(&layout->sentinel);
    layout->location_counter = 1;
}

internal i32
dynamic_variables_lookup_or_create(General_Memory *general, Dynamic_Variable_Layout *layout,
                                   String name, u64 default_value){
    for (Dynamic_Variable_Slot *slot = layout->sentinel.next;
         slot != &layout->sentinel;
         slot = slot->next){
        if (match(slot->name, name)){
            return(slot->location);
        }
    }
    int32_t alloc_size = name.size + 1 + sizeof(Dynamic_Variable_Slot);
    void *ptr = general_memory_allocate(general, alloc_size);
    if (ptr != 0){
        Dynamic_Variable_Slot *new_slot = (Dynamic_Variable_Slot*)ptr;
        char *c_str = (char*)(new_slot + 1);
        String str = make_string_cap(c_str, 0, name.size + 1);
        copy(&str, name);
        terminate_with_null(&str);
        new_slot->name = str;
        new_slot->default_value = default_value;
        new_slot->location = layout->location_counter++;
        dll_insert_back(&layout->sentinel, new_slot);
        return(new_slot->location);
    }
    return(0);
}

internal void
dynamic_variables_block_init(General_Memory *general, Dynamic_Variable_Block *block){
    i32 max = 64;
    block->val_array = (u64*)general_memory_allocate(general, sizeof(u64)*max);
    block->count = 0;
    block->max = max;
}

internal void
dynamic_variables_block_free(General_Memory *general, Dynamic_Variable_Block *block){
    general_memory_free(general, block->val_array);
}

internal void
dynamic_variables_block_grow_max_to(General_Memory *general, i32 new_max, Dynamic_Variable_Block *block){
    u64 *new_array = (u64*)general_memory_allocate(general, sizeof(u64)*new_max);
    memcpy(new_array, block->val_array, sizeof(u64)*block->count);
    general_memory_free(general, block->val_array);
    block->val_array = new_array;
}

internal void
dynamic_variables_block_fill_unset_values(Dynamic_Variable_Layout *layout, Dynamic_Variable_Block *block,
                                          i32 one_past_last_index){
    i32 first_location = block->count + 1;
    i32 one_past_last_location = one_past_last_index + 1;
    block->count = one_past_last_index;
    for (Dynamic_Variable_Slot *slot = layout->sentinel.next;
         slot != &layout->sentinel;
         slot = slot->next){
        if (first_location <= slot->location && slot->location < one_past_last_location){
            block->val_array[slot->location - 1] = slot->default_value;
        }
    }
}

internal b32
dynamic_variables_get_ptr(General_Memory *general,
                          Dynamic_Variable_Layout *layout, Dynamic_Variable_Block *block,
                          i32 location, u64 **ptr_out){
    b32 result = false;
    if (location > 0 && location < layout->location_counter){
        i32 index = location - 1;
        if (index >= block->count){
            i32 minimum_max = layout->location_counter - 1;
            if (block->max < minimum_max){
                dynamic_variables_block_grow_max_to(general, minimum_max*2, block);
            }
            dynamic_variables_block_fill_unset_values(layout, block, index + 1);
        }
        *ptr_out = block->val_array + index;
        result = true;
    }
    return(result);
}

////////////////////////////////

// TODO(allen): // TODO(allen): // TODO(allen): 

// BOTTOM

