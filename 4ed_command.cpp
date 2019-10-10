/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Command management functions
 *
 */

// TOP

internal Command_Map*
mapping__alloc_map(Mapping *mapping){
    Command_Map *result = mapping->free_maps;
    if (result != 0){
        sll_stack_pop(mapping->free_maps);
    }
    else{
        result = push_array(mapping->node_arena, Command_Map, 1);
    }
    return(result);
}

internal void
mapping__free_map(Mapping *mapping, Command_Map *map){
    sll_stack_push(mapping->free_maps, map);
}

internal Command_Modified_Binding*
mapping__alloc_modified_binding(Mapping *mapping){
    Command_Modified_Binding *result = mapping->free_bindings;
    if (result != 0){
        sll_stack_pop(mapping->free_bindings);
    }
    else{
        result = push_array(mapping->node_arena, Command_Modified_Binding, 1);
    }
    return(result);
}

internal void
mapping__free_modified_binding(Mapping *mapping, Command_Modified_Binding *binding){
    sll_stack_push(mapping->free_bindings, binding);
}

internal Command_Binding_List*
mapping__alloc_binding_list(Mapping *mapping){
    Command_Binding_List *result = mapping->free_lists;
    if (result != 0){
        sll_stack_pop(mapping->free_lists);
    }
    else{
        result = push_array(mapping->node_arena, Command_Binding_List, 1);
    }
    return(result);
}

internal void
mapping__free_binding_list(Mapping *mapping, Command_Binding_List *binding_list){
    sll_stack_push(mapping->free_lists, binding_list);
}

internal Command_Binding_List*
map__get_list(Command_Map *map, Key_Code code){
    Command_Binding_List *result = 0;
    Table_Lookup lookup = table_lookup(&map->key_code_to_binding_list, code);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&map->key_code_to_binding_list, lookup, &val);
        result = (Command_Binding_List*)IntAsPtr(val);
    }
    return(result);
}

internal Command_Binding_List*
map__get_or_make_list(Mapping *mapping, Command_Map *map, Key_Code code){
    Command_Binding_List *result = map__get_list(map, code);
    if (result == 0){
        result = mapping__alloc_binding_list(mapping);
        block_zero_struct(result);
        sll_queue_push(map->list_first, map->list_last, result);
        table_insert(&map->key_code_to_binding_list, code, (u64)PtrAsInt(result));
    }
    return(result);
}

////////////////////////////////

internal void
mapping_init(Thread_Context *tctx, Mapping *mapping){
    block_zero_struct(mapping);
    mapping->node_arena = reserve_arena(tctx);
    heap_init(&mapping->heap, mapping->node_arena);
    mapping->heap_wrapper = base_allocator_on_heap(&mapping->heap);
    mapping->id_to_map = make_table_u64_u64(tctx->allocator, 100);
    mapping->id_counter = 1;
}

internal void
mapping_release(Thread_Context *tctx, Mapping *mapping){
    if (mapping->node_arena != 0){
        release_arena(tctx, mapping->node_arena);
        table_free(&mapping->id_to_map);
    }
}

#if 0
internal Command_Map*
mapping_begin_new_map(Mapping *mapping){
    Command_Map *map = mapping__alloc_map(mapping);
    block_zero_struct(map);
    map->id = mapping->id_counter;
    mapping->id_counter += 1;
    map->key_code_to_binding_list = make_table_u64_u64(&mapping->heap_wrapper, 8);
    table_insert(&mapping->id_to_map, map->id, (u64)PtrAsInt(map));
    return(map);
}
#endif

internal Command_Map*
mapping_get_map(Mapping *mapping, Command_Map_ID id){
    Command_Map *result = 0;
    Table_Lookup lookup = table_lookup(&mapping->id_to_map, id);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&mapping->id_to_map, lookup, &val);
        result = (Command_Map*)IntAsPtr(val);
    }
    return(result);
}

internal Command_Map_ID
mapping_validate_id(Mapping *mapping, Command_Map_ID id){
    Table_Lookup lookup = table_lookup(&mapping->id_to_map, id);
    if (!lookup.found_match){
        id = 0;
    }
    return(id);
}

internal Command_Map*
mapping_get_or_make_map(Mapping *mapping, Command_Map_ID id){
    Command_Map *result = mapping_get_map(mapping, id);
    if (result == 0){
        result = mapping__alloc_map(mapping);
        block_zero_struct(result);
        result->id = id;
        result->key_code_to_binding_list = make_table_u64_u64(&mapping->heap_wrapper, 8);
        table_insert(&mapping->id_to_map, result->id, (u64)PtrAsInt(result));
    }
    return(result);
}

internal void
mapping_release_map(Mapping *mapping, Command_Map *map){
    table_erase(&mapping->id_to_map, map->id);
    if (map->binding_last != 0){
        map->binding_last->next = mapping->free_bindings;
        mapping->free_bindings = map->binding_first;
    }
    if (map->list_last != 0){
        map->list_last->next = mapping->free_lists;
        mapping->free_lists = map->list_first;
    }
    table_free(&map->key_code_to_binding_list);
}

internal Command_Binding
map_get_binding_non_recursive(Command_Map *map, Input_Event *event){
    Command_Binding result = {};
    
    if (map != 0){
        switch (event->kind){
            case InputEventKind_TextInsert:
            {
                result = map->text_input_command;
            }break;
            
            case InputEventKind_KeyStroke:
            {
                Table_Lookup lookup = table_lookup(&map->key_code_to_binding_list, event->key.code);
                if (lookup.found_match){
                    u64 val = 0;
                    table_read(&map->key_code_to_binding_list, lookup, &val);
                    Command_Binding_List *list = (Command_Binding_List*)IntAsPtr(val);
                    Key_Modifiers *event_mods = &event->key.modifiers;
                    for (SNode *node = list->first;
                         node != 0;
                         node = node->next){
                        Command_Modified_Binding *mod_binding = CastFromMember(Command_Modified_Binding, order_node, node);
                        Key_Modifiers *binding_mods = &mod_binding->modifiers;
                        b32 is_a_match = true;
                        for (i32 i = 0; i < ArrayCount(binding_mods->modifiers); i += 1){
                            if (binding_mods->modifiers[i] &&
                                !event_mods->modifiers[i]){
                                is_a_match = false;
                                break;
                            }
                        }
                        if (is_a_match){
                            result = mod_binding->binding;
                            break;
                        }
                    }
                    
                }
            }break;
        }
    }
    
    return(result);
}

internal Command_Binding
map_get_binding_recursive(Mapping *mapping, Command_Map *map, Input_Event *event){
    Command_Binding result = {};
    for (i32 safety_counter = 0;
         map != 0 && safety_counter < 40;
         safety_counter += 1){
        result = map_get_binding_non_recursive(map, event);
        if (result.custom != 0){
            break;
        }
        map = mapping_get_map(mapping, map->parent);
    }
    return(result);
}

internal void
map_set_parent(Command_Map *map, Command_Map *parent){
    if (map != 0 && parent != 0){
        map->parent = parent->id;
    }
}

internal void
map_set_binding_text_input(Command_Map *map, Custom_Command_Function *custom){
    if (map != 0){
        map->text_input_command.custom = custom;
    }
}

internal void
map_set_binding_key(Mapping *mapping, Command_Map *map, Custom_Command_Function *custom,
                    Key_Code code, Key_Modifiers *modifiers){
    if (map != 0){
        Command_Binding_List *list = map__get_or_make_list(mapping, map, code);
        Command_Modified_Binding *mod_binding = mapping__alloc_modified_binding(mapping);
        sll_stack_push(map->binding_first, mod_binding);
        if (map->binding_last == 0){
            map->binding_last = map->binding_first;
        }
        sll_stack_push(list->first, &mod_binding->order_node);
        if (list->last == 0){
            list->last= list->first;
        }
        list->count += 1;
        block_copy_struct(&mod_binding->modifiers, modifiers);
        mod_binding->binding.custom = custom;
    }
}

internal Command_Binding_List*
map_get_binding_list_on_key(Command_Map *map, Key_Code code){
    Command_Binding_List *result = 0;
    if (map != 0){
        result = map__get_list(map, code);
    }
    return(result);
}

////////////////////////////////

internal void
map_set_parent(Mapping *mapping, Command_Map_ID map_id, Command_Map_ID parent_id){
    Command_Map *map = mapping_get_map(mapping, map_id);
    Command_Map *parent = mapping_get_map(mapping, parent_id);
    map_set_parent(map, parent);
}

internal void
map_set_parent(Mapping *mapping, Command_Map *map, Command_Map_ID parent_id){
    Command_Map *parent = mapping_get_map(mapping, parent_id);
    map_set_parent(map, parent);
}

internal void
map_set_binding_text_input(Mapping *mapping, Command_Map_ID map_id, Custom_Command_Function *custom){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding_text_input(map, custom);
}

internal void
map_set_binding_key(Mapping *mapping, Command_Map_ID map_id, Custom_Command_Function *custom,
                    Key_Code code, Key_Modifiers *modifiers){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding_key(mapping, map, custom, code, modifiers);
}

internal Command_Binding_List*
map_get_binding_list_on_key(Mapping *mapping, Command_Map_ID map_id, Key_Code code){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_binding_list_on_key(map, code));
}

internal Command_Binding
map_get_binding_non_recursive(Mapping *mapping, Command_Map_ID map_id, Input_Event *event){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_binding_non_recursive(map, event));
}

internal Command_Binding
map_get_binding_recursive(Mapping *mapping, Command_Map_ID map_id, Input_Event *event){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_binding_recursive(mapping, map, event));
}

// BOTTOM

