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

internal Managed_Variable_ID
dynamic_variables_lookup(Dynamic_Variable_Layout *layout, String name){
    for (Dynamic_Variable_Slot *slot = layout->sentinel.next;
         slot != &layout->sentinel;
         slot = slot->next){
        if (match(slot->name, name)){
            return(slot->location);
        }
    }
    return(ManagedVariableIndex_ERROR);
}

internal Managed_Variable_ID
dynamic_variables_create__always(Heap *heap, Dynamic_Variable_Layout *layout, String name, u64 default_value){
    int32_t alloc_size = name.size + 1 + sizeof(Dynamic_Variable_Slot);
    void *ptr = heap_allocate(heap, alloc_size);
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
    return(ManagedVariableIndex_ERROR);
}

internal Managed_Variable_ID
dynamic_variables_lookup_or_create(Heap *heap, Dynamic_Variable_Layout *layout, String name, u64 default_value){
    Managed_Variable_ID lookup_id = dynamic_variables_lookup(layout, name);
    if (lookup_id != ManagedVariableIndex_ERROR){
        return(lookup_id);
    }
    return(dynamic_variables_create__always(heap, layout, name, default_value));
}

internal i32
dynamic_variables_create(Heap *heap, Dynamic_Variable_Layout *layout, String name, u64 default_value){
    Managed_Variable_ID lookup_id = dynamic_variables_lookup(layout, name);
    if (lookup_id == ManagedVariableIndex_ERROR){
        return(dynamic_variables_create__always(heap, layout, name, default_value));
    }
    return(ManagedVariableIndex_ERROR);
}

internal void
dynamic_variables_block_init(Heap *heap, Dynamic_Variable_Block *block){
    i32 max = 64;
    block->val_array = heap_array(heap, u64, max);
    block->count = 0;
    block->max = max;
}

internal void
dynamic_variables_block_free(Heap *heap, Dynamic_Variable_Block *block){
    heap_free(heap, block->val_array);
}

internal void
dynamic_variables_block_grow_max_to(Heap *heap, i32 new_max, Dynamic_Variable_Block *block){
    u64 *new_array = heap_array(heap, u64, new_max);
    memcpy(new_array, block->val_array, sizeof(u64)*block->count);
    heap_free(heap, block->val_array);
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
dynamic_variables_get_ptr(Heap *heap,
                          Dynamic_Variable_Layout *layout, Dynamic_Variable_Block *block,
                          i32 location, u64 **ptr_out){
    b32 result = false;
    if (location > 0 && location < layout->location_counter){
        i32 index = location - 1;
        if (index >= block->count){
            i32 minimum_max = layout->location_counter - 1;
            if (block->max < minimum_max){
                dynamic_variables_block_grow_max_to(heap, minimum_max*2, block);
            }
            dynamic_variables_block_fill_unset_values(layout, block, index + 1);
        }
        *ptr_out = block->val_array + index;
        result = true;
    }
    return(result);
}

////////////////////////////////

internal void
dynamic_memory_bank_init(Heap *heap, Dynamic_Memory_Bank *mem_bank){
    heap_init(&mem_bank->heap);
    mem_bank->first = 0;
    mem_bank->last = 0;
}

internal void*
dynamic_memory_bank_allocate(Heap *heap, Dynamic_Memory_Bank *bank, i32 size){
    void *ptr = heap_allocate(&bank->heap, size);
    if (ptr == 0){
        i32 alloc_size = clamp_bottom(4096, size*4);
        void *new_block = heap_allocate(heap, alloc_size);
        if (new_block != 0){
            Dynamic_Memory_Header *header = (Dynamic_Memory_Header*)new_block;
            sll_push(bank->first, bank->last, header);
            heap_extend(&bank->heap, header + 1, alloc_size - sizeof(*header));
            ptr = heap_allocate(&bank->heap, size);
        }
    }
    return(ptr);
}

internal void
dynamic_memory_bank_free(Dynamic_Memory_Bank *bank, void *ptr){
    heap_free(&bank->heap, ptr);
}

internal void
dynamic_memory_bank_free_all(Heap *heap, Dynamic_Memory_Bank *mem_bank){
    for (Dynamic_Memory_Header *header = mem_bank->first, *next = 0;
         header != 0;
         header = next){
        next = header->next;
        heap_free(heap, header);
    }
}

////////////////////////////////

internal void
dynamic_workspace_init(Heap *heap, Lifetime_Allocator *lifetime_allocator,
                       i32 user_type, void *user_back_ptr,
                       Dynamic_Workspace *workspace){
    dynamic_variables_block_init(heap, &workspace->var_block);
    dynamic_memory_bank_init(heap, &workspace->mem_bank);
    if (lifetime_allocator->scope_id_counter == 0){
        lifetime_allocator->scope_id_counter = 1;
    }
    workspace->scope_id = lifetime_allocator->scope_id_counter++;
    insert_u32_Ptr_table(heap, &lifetime_allocator->scope_id_to_scope_ptr_table, workspace->scope_id, workspace);
    workspace->user_type = user_type;
    workspace->user_back_ptr = user_back_ptr;
}

internal void
dynamic_workspace_free(Heap *heap, Lifetime_Allocator *lifetime_allocator, Dynamic_Workspace *workspace){
    erase_u32_Ptr_table(&lifetime_allocator->scope_id_to_scope_ptr_table, workspace->scope_id);
    dynamic_variables_block_free(heap, &workspace->var_block);
    dynamic_memory_bank_free_all(heap, &workspace->mem_bank);
}

internal u32
dynamic_workspace_store_pointer(Heap *heap, Dynamic_Workspace *workspace, void *ptr){
    if (workspace->object_id_counter == 0){
        workspace->object_id_counter = 1;
    }
    u32 id = workspace->object_id_counter++;
    insert_u32_Ptr_table(heap, &workspace->object_id_to_object_ptr, id, ptr);
    return(id);
}

internal void
dynamic_workspace_erase_pointer(Dynamic_Workspace *workspace, u32 id){
    erase_u32_Ptr_table(&workspace->object_id_to_object_ptr, id);
}

internal void*
dynamic_workspace_get_pointer(Dynamic_Workspace *workspace, u32 id){
    u32_Ptr_Lookup_Result lookup = lookup_u32_Ptr_table(&workspace->object_id_to_object_ptr, id);
    if (lookup.success){
        return(*lookup.val);
    }
    return(0);
}

////////////////////////////////

internal u64
lifetime__key_hash(Lifetime_Object **object_ptr_array, i32 count){
    u64 hash = bit_1;
    for (i32 i = 0; i < count; i += 1){
        u64 x = (u64)(PtrAsInt(object_ptr_array[i]));
        x >>= 3;
        hash = (hash + ((hash << 37) ^ (((x) >> (x&1)))));
    }
    return(hash | bit_63);
}

internal Lifetime_Key*
lifetime__key_table_lookup(Lifetime_Key_Table *table, u64 hash, Lifetime_Object **object_ptr_array, i32 count){
    u32 max = table->max;
    if (max > 0 && table->count > 0){
        u32 first_index = hash%max;
        u32 index = first_index;
        u64 *hashes = table->hashes;
        umem set_size = count*sizeof(Lifetime_Object*);
        for (;;){
            if (hashes[index] == hash){
                Lifetime_Key *key = table->keys[index];
                if (key->count == count &&
                    memcmp(object_ptr_array, key->members, set_size) == 0){
                    return(key);
                }
            }
            else if (hashes[index] == LifetimeKeyHash_Empty){
                return(0);
            }
            index += 1;
            if (index == max){
                index = 0;
            }
            if (index == first_index){
                return(0);
            }
        }
    }
    return(0);
}

internal Lifetime_Key_Table
lifetime__key_table_copy(Heap *heap, Lifetime_Key_Table table, u32 new_max);

internal void
lifetime__key_table_insert(Heap *heap, Lifetime_Key_Table *table, u64 hash, Lifetime_Key *key){
    {
        u32 max = table->max;
        u32 count = table->count;
        if (max == 0 || (count + 1)*6 > max*5){
            Assert(heap != 0);
            Lifetime_Key_Table new_table = lifetime__key_table_copy(heap, *table, max*2);
            heap_free(heap, table->mem_ptr);
            *table = new_table;
        }
    }
    
    {
        u32 max = table->max;
        if (max > 0){
            u32 first_index = hash%max;
            u32 index = first_index;
            u64 *hashes = table->hashes;
            for (;;){
                if (hashes[index] == LifetimeKeyHash_Empty ||
                    hashes[index] == LifetimeKeyHash_Deleted){
                    hashes[index] = hash;
                    table->keys[index] = key;
                    table->count += 1;
                    return;
                }
                index += 1;
                if (index == max){
                    index = 0;
                }
                if (index == first_index){
                    return;
                }
            }
        }
    }
}

internal void
lifetime__key_table_erase(Lifetime_Key_Table *table, Lifetime_Key *erase_key){
    u32 max = table->max;
    if (max > 0 && table->count > 0){
        u64 hash = lifetime__key_hash(erase_key->members, erase_key->count);
        u32 first_index = hash%max;
        u32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == hash){
                Lifetime_Key *key = table->keys[index];
                if (erase_key == key){
                    hashes[index] = LifetimeKeyHash_Deleted;
                    table->keys[index] = 0;
                    return;
                }
            }
            else if (hashes[index] == LifetimeKeyHash_Empty){
                return;
            }
            index += 1;
            if (index == max){
                index = 0;
            }
            if (index == first_index){
                return;
            }
        }
    }
}

internal Lifetime_Key_Table
lifetime__key_table_copy(Heap *heap, Lifetime_Key_Table table, u32 new_max){
    Lifetime_Key_Table new_table = {0};
    new_table.max = clamp_bottom(table.max, new_max);
    new_table.max = clamp_bottom(307, new_table.max);
    i32 item_size = sizeof(*new_table.hashes) + sizeof(*new_table.keys);
    new_table.mem_ptr = heap_allocate(heap, item_size*new_table.max);
    memset(new_table.mem_ptr, 0, item_size*new_table.max);
    new_table.hashes = (u64*)(new_table.mem_ptr);
    new_table.keys = (Lifetime_Key**)(new_table.hashes + new_table.max);
    for (u32 i = 0; i < table.max; i += 1){
        if ((table.hashes[i]&bit_63) != 0){
            lifetime__key_table_insert(0, &new_table, table.hashes[i], table.keys[i]);
        }
    }
    return(new_table);
}

internal void
lifetime__free_key(Heap *heap, Lifetime_Allocator *lifetime_allocator,
                   Lifetime_Key *key, Lifetime_Object *skip_object){
    // Deinit
    dynamic_workspace_free(heap, lifetime_allocator, &key->dynamic_workspace);
    
    // Remove From Objects
    i32 count = key->count;
    Lifetime_Object **object_ptr = key->members;
    for (i32 i = 0; i < count; i += 1, object_ptr += 1){
        if (*object_ptr == skip_object) continue;
        
        Lifetime_Key_Ref_Node *delete_point_node = 0;
        i32 delete_point_i = 0;
        
        i32 key_i = 0;
        Lifetime_Object *object = *object_ptr;
        for (Lifetime_Key_Ref_Node *node = object->key_node_first;
             node != 0;
             node = node->next){
            i32 one_past_last = clamp_top(ArrayCount(node->keys), object->key_count - key_i);
            for (i32 j = 0; j < one_past_last; j += 1){
                if (node->keys[j] == key){
                    delete_point_node = node;
                    delete_point_i = j;
                    goto double_break;
                }
            }
            key_i += one_past_last;
        }
        double_break:;
        
        Assert(delete_point_node != 0);
        Lifetime_Key_Ref_Node *last_node = object->key_node_last;
        Lifetime_Key *last_key = last_node->keys[object->key_count % lifetime_key_reference_per_node];
        delete_point_node->keys[delete_point_i] = last_key;
        object->key_count -= 1;
        
        if ((object->key_count % lifetime_key_reference_per_node) == 0){
            zdll_remove(object->key_node_first, object->key_node_last, last_node);
            zdll_push_back(lifetime_allocator->free_key_references.first, lifetime_allocator->free_key_references.last, last_node);
        }
    }
    
    // Free
    lifetime__key_table_erase(&lifetime_allocator->key_table, key);
    erase_Ptr_table(&lifetime_allocator->key_check_table, key);
    heap_free(heap, key->members);
    zdll_push_back(lifetime_allocator->free_keys.first, lifetime_allocator->free_keys.last, key);
}

internal Lifetime_Key_Ref_Node*
lifetime__alloc_key_reference_node(Heap *heap, Lifetime_Allocator *lifetime_allocator){
    Lifetime_Key_Ref_Node *result = lifetime_allocator->free_key_references.first;
    if (result == 0){
        i32 new_node_count = 32;
        Lifetime_Key_Ref_Node *new_nodes = heap_array(heap, Lifetime_Key_Ref_Node, new_node_count);
        Lifetime_Key_Ref_Node *new_node_ptr = new_nodes;
        for (i32 i = 0; i < new_node_count; i += 1, new_node_ptr += 1){
            zdll_push_back(lifetime_allocator->free_key_references.first,
                           lifetime_allocator->free_key_references.last,
                           new_node_ptr);
        }
        lifetime_allocator->free_key_references.count += new_node_count;
        result = lifetime_allocator->free_key_references.first;
    }
    zdll_remove(lifetime_allocator->free_key_references.first, lifetime_allocator->free_key_references.last,
                result);
    return(result);
}

internal void
lifetime__object_add_key(Heap *heap, Lifetime_Allocator *lifetime_allocator,
                         Lifetime_Object *object, Lifetime_Key *key){
    Lifetime_Key_Ref_Node *last_node = object->key_node_last;
    b32 insert_on_new_node = false;
    if (last_node == 0){
        insert_on_new_node = true;
    }
    else{
        i32 next_insert_slot = object->key_count%ArrayCount(last_node->keys);
        if (next_insert_slot != 0){
            last_node->keys[next_insert_slot] = key;
        }
        else{
            insert_on_new_node = true;
        }
    }
    if (insert_on_new_node){
        Lifetime_Key_Ref_Node *new_node = lifetime__alloc_key_reference_node(heap, lifetime_allocator);
        zdll_push_back(object->key_node_first, object->key_node_last, new_node);
        memset(new_node->keys, 0, sizeof(new_node->keys));
        new_node->keys[0] = key;
        object->key_count += 1;
    }
}

internal Lifetime_Object*
lifetime_alloc_object(Heap *heap, Lifetime_Allocator *lifetime_allocator, i32 user_type, void *user_back_ptr){
    Lifetime_Object *object = lifetime_allocator->free_objects.first;
    if (object == 0){
        i32 new_object_count = 256;
        Lifetime_Object *new_objects = heap_array(heap, Lifetime_Object, new_object_count);
        Lifetime_Object *new_object_ptr = new_objects;
        for (i32 i = 0; i < new_object_count; i += 1, new_object_ptr += 1){
            zdll_push_back(lifetime_allocator->free_objects.first, lifetime_allocator->free_objects.last, new_object_ptr);
        }
        lifetime_allocator->free_objects.count += new_object_count;
        object = lifetime_allocator->free_objects.first;
    }
    
    zdll_remove(lifetime_allocator->free_objects.first, lifetime_allocator->free_objects.last, object);
    lifetime_allocator->free_objects.count -= 1;
    
    memset(object, 0, sizeof(*object));
    dynamic_workspace_init(heap, lifetime_allocator, user_type, user_back_ptr, &object->workspace);
    
    return(object);
}

internal void
lifetime_free_object(Heap *heap, Lifetime_Allocator *lifetime_allocator,
                     Lifetime_Object *lifetime_object){
    dynamic_workspace_free(heap, lifetime_allocator, &lifetime_object->workspace);
    
    i32 key_i = 0;
    for (Lifetime_Key_Ref_Node *node = lifetime_object->key_node_first;
         node != 0;
         node = node->next){
        i32 one_past_last = clamp_top(ArrayCount(node->keys), lifetime_object->key_count - key_i);
        for (i32 i = 0; i < one_past_last; i += 1){
            lifetime__free_key(heap, lifetime_allocator, node->keys[i], lifetime_object);
        }
        key_i += one_past_last;
    }
    
    if (lifetime_object->key_count > 0){
        lifetime_object->key_node_last->next = lifetime_allocator->free_key_references.first;
        lifetime_allocator->free_key_references.first = lifetime_object->key_node_first;
        i32 node_count = (lifetime_object->key_count + (lifetime_key_reference_per_node - 1))/lifetime_key_reference_per_node;
        lifetime_allocator->free_key_references.count += node_count;
    }
    
    zdll_push_back(lifetime_allocator->free_objects.first, lifetime_allocator->free_objects.last, lifetime_object);
}

internal i32
lifetime_sort_object_set__part(Lifetime_Object **ptr_array, i32 first, i32 one_past_last){
    i32 pivot_index = one_past_last - 1;
    Lifetime_Object *pivot = ptr_array[pivot_index];
    i32 j = first;
    for (i32 i = first; i < pivot_index; i += 1){
        Lifetime_Object *object = ptr_array[i];
        if (object < pivot){
            Swap(Lifetime_Object*, ptr_array[i], ptr_array[j]);
            j += 1;
        }
    }
    Swap(Lifetime_Object*, ptr_array[j], ptr_array[pivot_index]);
    return(j);
}

internal void
lifetime_sort_object_set__quick(Lifetime_Object **ptr_array, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot = lifetime_sort_object_set__part(ptr_array, first, one_past_last);
        lifetime_sort_object_set__quick(ptr_array, first, pivot);
        lifetime_sort_object_set__quick(ptr_array, pivot + 1, one_past_last);
    }
}

internal i32
lifetime_sort_and_dedup_object_set(Lifetime_Object **ptr_array, i32 count){
    lifetime_sort_object_set__quick(ptr_array, 0, count);
    Lifetime_Object **ptr_write = ptr_array + 1;
    Lifetime_Object **ptr_read  = ptr_array + 1;
    for (i32 i = 1; i < count; i += 1, ptr_read += 1){
        if (ptr_read[-1] < ptr_read[0]){
            *ptr_write = *ptr_read;
            ptr_write += 1;
        }
    }
    return((i32)(ptr_write - ptr_array));
}

internal Lifetime_Key*
lifetime_get_or_create_intersection_key(Heap *heap, Lifetime_Allocator *lifetime_allocator, Lifetime_Object **object_ptr_array, i32 count){
    u64 hash = lifetime__key_hash(object_ptr_array, count);
    
    // Lookup
    Lifetime_Key *existing_key = lifetime__key_table_lookup(&lifetime_allocator->key_table, hash,
                                                            object_ptr_array, count);
    if (existing_key != 0){
        return(existing_key);
    }
    
    // Allocate
    Lifetime_Key *new_key = lifetime_allocator->free_keys.first;
    if (new_key == 0){
        i32 new_key_count = 256;
        Lifetime_Key *new_keys = heap_array(heap, Lifetime_Key, new_key_count);
        Lifetime_Key *new_key_ptr = new_keys;
        for (i32 i = 0; i < new_key_count; i += 1, new_key_ptr += 1){
            zdll_push_back(lifetime_allocator->free_keys.first, lifetime_allocator->free_keys.last, new_key_ptr);
        }
        lifetime_allocator->free_keys.count += new_key_count;
        new_key = lifetime_allocator->free_keys.first;
    }
    zdll_remove(lifetime_allocator->free_keys.first, lifetime_allocator->free_keys.last, new_key);
    
    // Add to Objects
    Lifetime_Object **object_ptr = object_ptr_array;
    for (i32 i = 0; i < count; i += 1, object_ptr += 1){
        Lifetime_Object *object = *object_ptr;
        lifetime__object_add_key(heap, lifetime_allocator, object, new_key);
    }
    
    // Initialize
    new_key->members = heap_array(heap, Lifetime_Object*, count);
    memcpy(new_key->members, object_ptr_array, sizeof(*new_key->members)*count);
    new_key->count = count;
    dynamic_workspace_init(heap, lifetime_allocator,
                           DynamicWorkspace_Intersected, new_key,
                           &new_key->dynamic_workspace);
    
    lifetime__key_table_insert(heap, &lifetime_allocator->key_table, hash, new_key);
    insert_Ptr_table(heap, &lifetime_allocator->key_check_table, new_key);
    
    return(new_key);
}

internal b32
lifetime_key_check(Lifetime_Allocator *lifetime_allocator, Lifetime_Key *key){
    return(lookup_Ptr_table(&lifetime_allocator->key_check_table, key));
}

// BOTTOM

