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
managed_ids_init(Base_Allocator *allocator, Managed_ID_Set *set){
    set->arena = make_arena(allocator, KB(4), 8);
    set->name_to_id_table = make_table_Data_u64(allocator, 40);
    set->id_counter = 1;
}

internal Managed_ID
managed_ids_declare(Managed_ID_Set *set, String_Const_u8 name){
    Managed_ID result = 0;
    Data data = make_data(name.str, name.size);
    Table_Lookup lookup = table_lookup(&set->name_to_id_table, data);
    if (lookup.found_match){
        table_read(&set->name_to_id_table, lookup, &result);
    }
    else{
        result = set->id_counter;
        set->id_counter += 1;
        data = push_data_copy(&set->arena, data);
        table_insert(&set->name_to_id_table, data, result);
    }
    return(result);
}

////////////////////////////////

internal void
dynamic_variable_block_init(Base_Allocator *allocator, Dynamic_Variable_Block *block){
    block->arena = make_arena(allocator, KB(4), 8);
    block->id_to_data_table = make_table_u64_Data(allocator, 20);
}

internal Data
dynamic_variable_get(Dynamic_Variable_Block *block, Managed_ID id, umem size){
    Data result = {};
    Table_Lookup lookup = table_lookup(&block->id_to_data_table, id);
    if (lookup.found_match){
        table_read(&block->id_to_data_table, lookup, &result);
    }
    else{
        result = push_data(&block->arena, size);
        block_zero(result);
        table_insert(&block->id_to_data_table, id, result);
    }
    return(result);
}

internal void
dynamic_variable_erase(Dynamic_Variable_Block *block, Managed_ID id){
    table_erase(&block->id_to_data_table, id);
}

////////////////////////////////

internal void
lifetime_allocator_init(Base_Allocator *base_allocator, Lifetime_Allocator *lifetime_allocator){
    block_zero_struct(lifetime_allocator);
    lifetime_allocator->allocator = base_allocator;
    lifetime_allocator->key_table = make_table_Data_u64(base_allocator, 100);
    lifetime_allocator->key_check_table = make_table_u64_u64(base_allocator, 100);
    lifetime_allocator->scope_id_to_scope_ptr_table = make_table_u64_u64(base_allocator, 100);
}

////////////////////////////////

internal void
dynamic_workspace_init(Lifetime_Allocator *lifetime_allocator, i32 user_type, void *user_back_ptr, Dynamic_Workspace *workspace){
    block_zero_struct(workspace);
    heap_init(&workspace->heap, lifetime_allocator->allocator);
    workspace->heap_wrapper = base_allocator_on_heap(&workspace->heap);
    workspace->object_id_to_object_ptr = make_table_u64_u64(&workspace->heap_wrapper, 10);
    dynamic_variable_block_init(&workspace->heap_wrapper, &workspace->var_block);
    if (lifetime_allocator->scope_id_counter == 0){
        lifetime_allocator->scope_id_counter = 1;
    }
    workspace->scope_id = lifetime_allocator->scope_id_counter++;
    table_insert(&lifetime_allocator->scope_id_to_scope_ptr_table,
                 workspace->scope_id, (u64)PtrAsInt(workspace));
    workspace->user_type = user_type;
    workspace->user_back_ptr = user_back_ptr;
}

internal void
dynamic_workspace_free(Lifetime_Allocator *lifetime_allocator, Dynamic_Workspace *workspace){
    table_erase(&lifetime_allocator->scope_id_to_scope_ptr_table, workspace->scope_id);
    heap_free_all(&workspace->heap);
}

internal void
dynamic_workspace_clear_contents(Dynamic_Workspace *workspace){
    Base_Allocator *base_allocator = heap_free_all(&workspace->heap);
    heap_init(&workspace->heap, base_allocator);
    workspace->heap_wrapper = base_allocator_on_heap(&workspace->heap);
    workspace->object_id_to_object_ptr = make_table_u64_u64(&workspace->heap_wrapper, 10);
    dynamic_variable_block_init(&workspace->heap_wrapper, &workspace->var_block);
    block_zero_struct(&workspace->buffer_markers_list);
    workspace->total_marker_count = 0;
}

internal u32
dynamic_workspace_store_pointer(Dynamic_Workspace *workspace, void *ptr){
    if (workspace->object_id_counter == 0){
        workspace->object_id_counter = 1;
    }
    u32 id = workspace->object_id_counter++;
    table_insert(&workspace->object_id_to_object_ptr, id, (u64)PtrAsInt(ptr));
    return(id);
}

internal void
dynamic_workspace_erase_pointer(Dynamic_Workspace *workspace, u32 id){
    table_erase(&workspace->object_id_to_object_ptr, id);
}

internal void*
dynamic_workspace_get_pointer(Dynamic_Workspace *workspace, u32 id){
    void *result = 0;
    Table_Lookup lookup = table_lookup(&workspace->object_id_to_object_ptr, id);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&workspace->object_id_to_object_ptr, lookup, &val);
        result = IntAsPtr(val);
    }
    return(result);
}

////////////////////////////////

internal Data
lifetime__key_as_data(Lifetime_Object **members, i32 count){
    return(make_data(members, sizeof(*members)*count));
}

internal Data
lifetime__key_as_data(Lifetime_Key *key){
    return(lifetime__key_as_data(key->members, key->count));
}

internal void
lifetime__free_key(Lifetime_Allocator *lifetime_allocator, Lifetime_Key *key, Lifetime_Object *skip_object){
    // Deinit
    dynamic_workspace_free(lifetime_allocator, &key->dynamic_workspace);
    
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
        Lifetime_Key *last_key = last_node->keys[(object->key_count - 1) % ArrayCount(last_node->keys)];
        Assert(last_key != 0);
        delete_point_node->keys[delete_point_i] = last_key;
        object->key_count -= 1;
        
        if ((object->key_count % lifetime_key_reference_per_node) == 0){
            zdll_remove(object->key_node_first, object->key_node_last, last_node);
            zdll_push_back(lifetime_allocator->free_key_references.first, lifetime_allocator->free_key_references.last, last_node);
        }
    }
    
    // Free
    Data key_data = lifetime__key_as_data(key);
    table_erase(&lifetime_allocator->key_table, key_data);
    table_erase(&lifetime_allocator->key_check_table, (u64)PtrAsInt(key));
    base_free(lifetime_allocator->allocator, key->members);
    zdll_push_back(lifetime_allocator->free_keys.first, lifetime_allocator->free_keys.last, key);
}

internal Lifetime_Key_Ref_Node*
lifetime__alloc_key_reference_node(Lifetime_Allocator *lifetime_allocator){
    Assert(lifetime_allocator != 0);
    Lifetime_Key_Ref_Node *result = lifetime_allocator->free_key_references.first;
    if (result == 0){
        i32 new_node_count = 32;
        umem new_memory_size = new_node_count*sizeof(Lifetime_Key_Ref_Node);
        Data new_memory = base_allocate(lifetime_allocator->allocator, new_memory_size);
        Lifetime_Key_Ref_Node *new_nodes = (Lifetime_Key_Ref_Node*)new_memory.data;
        Lifetime_Key_Ref_Node *new_node_ptr = new_nodes;
        for (i32 i = 0; i < new_node_count; i += 1, new_node_ptr += 1){
            zdll_push_back(lifetime_allocator->free_key_references.first,
                           lifetime_allocator->free_key_references.last,
                           new_node_ptr);
        }
        lifetime_allocator->free_key_references.count += new_node_count;
        result = lifetime_allocator->free_key_references.first;
    }
    zdll_remove(lifetime_allocator->free_key_references.first, lifetime_allocator->free_key_references.last, result);
    return(result);
}

internal void
lifetime__object_add_key(Lifetime_Allocator *lifetime_allocator, Lifetime_Object *object, Lifetime_Key *key){
    Lifetime_Key_Ref_Node *last_node = object->key_node_last;
    b32 insert_on_new_node = false;
    if (last_node == 0){
        insert_on_new_node = true;
    }
    else{
        i32 next_insert_slot = object->key_count%ArrayCount(last_node->keys);
        if (next_insert_slot != 0){
            last_node->keys[next_insert_slot] = key;
            object->key_count += 1;
        }
        else{
            insert_on_new_node = true;
        }
    }
    if (insert_on_new_node){
        Lifetime_Key_Ref_Node *new_node = lifetime__alloc_key_reference_node(lifetime_allocator);
        zdll_push_back(object->key_node_first, object->key_node_last, new_node);
        block_zero_struct(new_node->keys);
        new_node->keys[0] = key;
        object->key_count += 1;
    }
}

internal Lifetime_Object*
lifetime_alloc_object(Lifetime_Allocator *lifetime_allocator, i32 user_type, void *user_back_ptr){
    Lifetime_Object *object = lifetime_allocator->free_objects.first;
    if (object == 0){
        i32 new_object_count = 256;
        umem new_memory_size = new_object_count*sizeof(Lifetime_Object);
        Data new_memory = base_allocate(lifetime_allocator->allocator, new_memory_size);
        Lifetime_Object *new_objects = (Lifetime_Object*)new_memory.data;
        Lifetime_Object *new_object_ptr = new_objects;
        for (i32 i = 0; i < new_object_count; i += 1, new_object_ptr += 1){
            zdll_push_back(lifetime_allocator->free_objects.first, lifetime_allocator->free_objects.last, new_object_ptr);
        }
        lifetime_allocator->free_objects.count += new_object_count;
        object = lifetime_allocator->free_objects.first;
    }
    
    zdll_remove(lifetime_allocator->free_objects.first, lifetime_allocator->free_objects.last, object);
    lifetime_allocator->free_objects.count -= 1;
    
    block_zero_struct(object);
    dynamic_workspace_init(lifetime_allocator, user_type, user_back_ptr, &object->workspace);
    
    return(object);
}

internal void
lifetime__object_free_all_keys(Lifetime_Allocator *lifetime_allocator, Lifetime_Object *lifetime_object){
    i32 key_i = 0;
    for (Lifetime_Key_Ref_Node *node = lifetime_object->key_node_first;
         node != 0;
         node = node->next){
        i32 one_past_last = clamp_top(ArrayCount(node->keys), lifetime_object->key_count - key_i);
        for (i32 i = 0; i < one_past_last; i += 1){
            lifetime__free_key(lifetime_allocator, node->keys[i], lifetime_object);
        }
        key_i += one_past_last;
    }
    
    if (lifetime_object->key_count > 0){
        lifetime_object->key_node_last->next = lifetime_allocator->free_key_references.first;
        lifetime_allocator->free_key_references.first = lifetime_object->key_node_first;
        i32 node_count = (lifetime_object->key_count + (lifetime_key_reference_per_node - 1))/lifetime_key_reference_per_node;
        lifetime_allocator->free_key_references.count += node_count;
    }
}

internal void
lifetime__object_clear_all_keys(Lifetime_Allocator *lifetime_allocator, Lifetime_Object *lifetime_object){
    i32 key_i = 0;
    for (Lifetime_Key_Ref_Node *node = lifetime_object->key_node_first;
         node != 0;
         node = node->next){
        i32 one_past_last = clamp_top(ArrayCount(node->keys), lifetime_object->key_count - key_i);
        Lifetime_Key **key_ptr = node->keys;
        for (i32 i = 0; i < one_past_last; i += 1, key_ptr += 1){
            dynamic_workspace_clear_contents(&(*key_ptr)->dynamic_workspace);
        }
        key_i += one_past_last;
    }
}

internal void
lifetime_free_object(Lifetime_Allocator *lifetime_allocator, Lifetime_Object *lifetime_object){
    lifetime__object_free_all_keys(lifetime_allocator, lifetime_object);
    dynamic_workspace_free(lifetime_allocator, &lifetime_object->workspace);
    zdll_push_back(lifetime_allocator->free_objects.first, lifetime_allocator->free_objects.last, lifetime_object);
}

internal void
lifetime_object_reset(Lifetime_Allocator *lifetime_allocator, Lifetime_Object *lifetime_object){
    lifetime__object_clear_all_keys(lifetime_allocator, lifetime_object);
    dynamic_workspace_clear_contents(&lifetime_object->workspace);
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
lifetime_get_or_create_intersection_key(Lifetime_Allocator *lifetime_allocator, Lifetime_Object **object_ptr_array, i32 count){
    {
        Data key_data = lifetime__key_as_data(object_ptr_array, count);
        Table_Lookup lookup = table_lookup(&lifetime_allocator->key_table, key_data);
        if (lookup.found_match){
            u64 val = 0;
            table_read(&lifetime_allocator->key_table, lookup, &val);
            return((Lifetime_Key*)IntAsPtr(val));
        }
    }
    
    // Allocate
    Lifetime_Key *new_key = lifetime_allocator->free_keys.first;
    if (new_key == 0){
        i32 new_key_count = 256;
        umem new_memory_size = new_key_count*sizeof(Lifetime_Key);
        Data new_memory = base_allocate(lifetime_allocator->allocator, new_memory_size);
        Lifetime_Key *new_keys = (Lifetime_Key*)new_memory.data;
        Lifetime_Key *new_key_ptr = new_keys;
        for (i32 i = 0; i < new_key_count; i += 1, new_key_ptr += 1){
            zdll_push_back(lifetime_allocator->free_keys.first, lifetime_allocator->free_keys.last, new_key_ptr);
        }
        lifetime_allocator->free_keys.count += new_key_count;
        new_key = lifetime_allocator->free_keys.first;
    }
    zdll_remove(lifetime_allocator->free_keys.first, lifetime_allocator->free_keys.last, new_key);
    block_zero_struct(new_key);
    
    // Add to Objects
    Lifetime_Object **object_ptr = object_ptr_array;
    for (i32 i = 0; i < count; i += 1, object_ptr += 1){
        Lifetime_Object *object = *object_ptr;
        lifetime__object_add_key(lifetime_allocator, object, new_key);
    }
    
    // Initialize
    umem new_memory_size = sizeof(Lifetime_Object*)*count;
    Data new_memory = base_allocate(lifetime_allocator->allocator, new_memory_size);
    new_key->members = (Lifetime_Object**)new_memory.data;
    block_copy_dynamic_array(new_key->members, object_ptr_array, count);
    new_key->count = count;
    dynamic_workspace_init(lifetime_allocator,
                           DynamicWorkspace_Intersected, new_key,
                           &new_key->dynamic_workspace);
    
    {
        Data key_data = lifetime__key_as_data(new_key);
        u64 new_key_val = (u64)PtrAsInt(new_key);
        table_insert(&lifetime_allocator->key_table, key_data, new_key_val);
        table_insert(&lifetime_allocator->key_check_table, new_key_val, new_key_val);
    }
    
    return(new_key);
}

internal b32
lifetime_key_check(Lifetime_Allocator *lifetime_allocator, Lifetime_Key *key){
    Table_Lookup lookup = table_lookup(&lifetime_allocator->key_check_table, (u64)PtrAsInt(key));
    return(lookup.found_match);
}

////////////////////////////////

// TODO(allen): move this shit somewhere real, clean up all object creation functions to be more cleanly layered.
internal u8*
get_dynamic_object_memory_ptr(Managed_Object_Standard_Header *header){
    u8 *ptr = 0;
    if (header != 0){
        switch (header->type){
            case ManagedObjectType_Memory:
            case ManagedObjectType_Markers:
            {
                ptr = ((u8*)header) + managed_header_type_sizes[header->type];
            }break;
        }
    }
    return(ptr);
}

internal Managed_Object
managed_object_alloc_managed_memory(Dynamic_Workspace *workspace, i32 item_size, i32 count, void **ptr_out){
    i32 size = item_size*count;
    Data new_memory = base_allocate(&workspace->heap_wrapper, sizeof(Managed_Memory_Header) + size);
    void *ptr = new_memory.data;
    Managed_Memory_Header *header = (Managed_Memory_Header*)ptr;
    header->std_header.type = ManagedObjectType_Memory;
    header->std_header.item_size = item_size;
    header->std_header.count = count;
    if (ptr_out != 0){
        *ptr_out = get_dynamic_object_memory_ptr(&header->std_header);
    }
    u32 id = dynamic_workspace_store_pointer(workspace, ptr);
    return(((u64)workspace->scope_id << 32) | (u64)id);
}

internal Managed_Object
managed_object_alloc_buffer_markers(Dynamic_Workspace *workspace, Buffer_ID buffer_id, i32 count, Marker **markers_out){
    i32 size = count*sizeof(Marker);
    Data new_memory = base_allocate(&workspace->heap_wrapper, size + sizeof(Managed_Buffer_Markers_Header));
    void *ptr = new_memory.data;
    Managed_Buffer_Markers_Header *header = (Managed_Buffer_Markers_Header*)ptr;
    header->std_header.type = ManagedObjectType_Markers;
    header->std_header.item_size = sizeof(Marker);
    header->std_header.count = count;
    zdll_push_back(workspace->buffer_markers_list.first, workspace->buffer_markers_list.last, header);
    workspace->buffer_markers_list.count += 1;
    workspace->total_marker_count += count;
    header->buffer_id = buffer_id;
    if (markers_out != 0){
        *markers_out = (Marker*)get_dynamic_object_memory_ptr(&header->std_header);
    }
    u32 id = dynamic_workspace_store_pointer(workspace, ptr);
    return(((u64)workspace->scope_id << 32) | (u64)id);
}

internal b32
managed_object_free(Dynamic_Workspace *workspace, Managed_Object object){
    b32 result = false;
    u32 lo_id = object&max_u32;
    u8 *object_ptr = (u8*)dynamic_workspace_get_pointer(workspace, lo_id);
    if (object_ptr != 0){
        Managed_Object_Type *type = (Managed_Object_Type*)object_ptr;
        switch (*type){
            case ManagedObjectType_Markers:
            {
                Managed_Buffer_Markers_Header *header = (Managed_Buffer_Markers_Header*)object_ptr;
                workspace->total_marker_count -= header->std_header.count;
                zdll_remove(workspace->buffer_markers_list.first, workspace->buffer_markers_list.last, header);
                workspace->buffer_markers_list.count -= 1;
            }break;
        }
        dynamic_workspace_erase_pointer(workspace, lo_id);
        base_free(&workspace->heap_wrapper, object_ptr);
        result = true;
    }
    return(result);
}

// BOTTOM

