/*
4coder_command_map.cpp - Command management functions
*/

// TOP

#if !defined(MAP_METADATA_ONLY)
#define MAP_METADATA_ONLY 0
#endif

#if MAP_METADATA_ONLY
#define BindingGetPtr(b) ((b).name)
#else
#define BindingGetPtr(b) ((b).custom)
#endif

Command_Binding::Command_Binding(){
    block_zero_struct(this);
}
Command_Binding::Command_Binding(Custom_Command_Function *c){
    this->custom = c;
}
Command_Binding::Command_Binding(char *n){
    this->name = n;
}
Command_Binding::operator Custom_Command_Function*(){
    return(this->custom);
}
Command_Binding::operator char*(){
    return(this->name);
}

function u64
mapping__key(Input_Event_Kind kind, u32 sub_code){
    return((((u64)kind) << 32) | sub_code);
}

function Command_Map*
mapping__alloc_map(Mapping *mapping){
    Command_Map *result = mapping->free_maps;
    if (result != 0){
        sll_stack_pop(mapping->free_maps);
    }
    else{
        result = push_array(&mapping->node_arena, Command_Map, 1);
    }
    zdll_push_back(mapping->first_map, mapping->last_map, result);
    return(result);
}

function void
mapping__free_map(Mapping *mapping, Command_Map *map){
    zdll_remove(mapping->first_map, mapping->last_map, map);
    sll_stack_push(mapping->free_maps, map);
}

function Command_Modified_Binding*
mapping__alloc_modified_binding(Mapping *mapping){
    Command_Modified_Binding *result = mapping->free_bindings;
    if (result != 0){
        sll_stack_pop(mapping->free_bindings);
    }
    else{
        result = push_array(&mapping->node_arena, Command_Modified_Binding, 1);
    }
    return(result);
}

function void
mapping__free_modified_binding(Mapping *mapping, Command_Modified_Binding *binding){
    sll_stack_push(mapping->free_bindings, binding);
}

function Command_Binding_List*
mapping__alloc_binding_list(Mapping *mapping){
    Command_Binding_List *result = mapping->free_lists;
    if (result != 0){
        sll_stack_pop(mapping->free_lists);
    }
    else{
        result = push_array(&mapping->node_arena, Command_Binding_List, 1);
    }
    return(result);
}

function void
mapping__free_binding_list(Mapping *mapping, Command_Binding_List *binding_list){
    sll_stack_push(mapping->free_lists, binding_list);
}

function Command_Binding_List*
map__get_list(Command_Map *map, u64 key){
    Command_Binding_List *result = 0;
    Table_Lookup lookup = table_lookup(&map->event_code_to_binding_list, key);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&map->event_code_to_binding_list, lookup, &val);
        result = (Command_Binding_List*)IntAsPtr(val);
    }
    return(result);
}

function Command_Binding_List*
map__get_or_make_list(Mapping *mapping, Command_Map *map, u64 key){
    Command_Binding_List *result = map__get_list(map, key);
    if (result == 0){
        result = mapping__alloc_binding_list(mapping);
        block_zero_struct(result);
        sll_queue_push(map->list_first, map->list_last, result);
        table_insert(&map->event_code_to_binding_list, key, (u64)PtrAsInt(result));
    }
    return(result);
}

////////////////////////////////

function void
mapping_init(Thread_Context *tctx, Mapping *mapping){
    block_zero_struct(mapping);
    mapping->node_arena = make_arena_system();
    heap_init(&mapping->heap, &mapping->node_arena);
    mapping->heap_wrapper = base_allocator_on_heap(&mapping->heap);
    mapping->id_to_map = make_table_u64_u64(tctx->allocator, 10);
    mapping->id_counter = 1;
}

function void
mapping_release(Thread_Context *tctx, Mapping *mapping){
    linalloc_clear(&mapping->node_arena);
    table_free(&mapping->id_to_map);
}

function void
map__init(Mapping *mapping, Command_Map *map, Command_Map_ID id){
    block_zero_struct(map);
    map->id = id;
    map->node_arena = make_arena(&mapping->heap_wrapper, KB(2));
    map->event_code_to_binding_list = make_table_u64_u64(&mapping->heap_wrapper, 100);
    map->cmd_to_binding_trigger = make_table_u64_u64(&mapping->heap_wrapper, 100);
}

function Command_Map*
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

function Command_Map_ID
mapping_validate_id(Mapping *mapping, Command_Map_ID id){
    Table_Lookup lookup = table_lookup(&mapping->id_to_map, id);
    if (!lookup.found_match){
        id = 0;
    }
    return(id);
}

function Command_Map*
mapping_get_or_make_map(Mapping *mapping, Command_Map_ID id){
    Command_Map *result = mapping_get_map(mapping, id);
    if (result == 0){
        result = mapping__alloc_map(mapping);
        map__init(mapping, result, id);
        table_insert(&mapping->id_to_map, id, (u64)PtrAsInt(result));
    }
    return(result);
}

function void
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
    table_free(&map->event_code_to_binding_list);
    linalloc_clear(&map->node_arena);
}

////////////////////////////////

function b32
map_strict_match(Input_Modifier_Set *binding_mod_set, Input_Modifier_Set *event_mod_set, Key_Code skip_self_mod){
    b32 result = true;
    i32 binding_mod_count = binding_mod_set->count;
    Key_Code *binding_mods = binding_mod_set->mods;
    for (i32 i = 0; i < binding_mod_count; i += 1){
        if (!has_modifier(event_mod_set, binding_mods[i])){
            result = false;
            break;
        }
    }
    i32 mod_count = event_mod_set->count;
    Key_Code *mods = event_mod_set->mods;
    for (i32 i = 0; i < mod_count; i += 1){
        if (mods[i] != skip_self_mod && !has_modifier(binding_mod_set, mods[i])){
            result = false;
            break;
        }
    }
    return(result);
}

function b32
map_loose_match(Input_Modifier_Set *binding_mod_set, Input_Modifier_Set *event_mod_set){
    b32 result = true;
    i32 binding_mod_count = binding_mod_set->count;
    Key_Code *binding_mods = binding_mod_set->mods;
    for (i32 i = 0; i < binding_mod_count; i += 1){
        if (!has_modifier(event_mod_set, binding_mods[i])){
            result = false;
            break;
        }
    }
    return(result);
}

function Map_Event_Breakdown
map_get_event_breakdown(Input_Event *event){
    Map_Event_Breakdown result = {};
    
    switch (event->kind){
        case InputEventKind_KeyStroke:
        {
            result.key = mapping__key(InputEventKind_KeyStroke, event->key.code);
            result.mod_set = &event->key.modifiers;
            result.skip_self_mod = event->key.code;
        }break;
        
        case InputEventKind_MouseButton:
        {
            result.key = mapping__key(InputEventKind_MouseButton, event->mouse.code);
            result.mod_set = &event->mouse.modifiers;
        }break;
        
        case InputEventKind_MouseWheel:
        {
            result.key = mapping__key(InputEventKind_MouseWheel, 0);
            result.mod_set = &event->mouse_wheel.modifiers;
        }break;
        
        case InputEventKind_MouseMove:
        {
            result.key = mapping__key(InputEventKind_MouseMove, 0);
            result.mod_set = &event->mouse_move.modifiers;
        }break;
        
        case InputEventKind_Core:
        {
            result.key = mapping__key(InputEventKind_Core, event->core.code);
        }break;
    }
    
    return(result);
}

function Command_Binding
map_get_binding_non_recursive(Command_Map *map, Input_Event *event, Binding_Match_Rule rule){
    Command_Binding result = {};
    
    if (event->kind == InputEventKind_CustomFunction){
        result.custom = event->custom_func;
    }
    else if (map != 0){
        if (event->kind == InputEventKind_TextInsert){
            result = map->text_input_command;
        }
        else{
            Map_Event_Breakdown breakdown = map_get_event_breakdown(event);
            Table_Lookup lookup = table_lookup(&map->event_code_to_binding_list, breakdown.key);
            if (lookup.found_match){
                u64 val = 0;
                table_read(&map->event_code_to_binding_list, lookup, &val);
                Command_Binding_List *list = (Command_Binding_List*)IntAsPtr(val);
                if (breakdown.mod_set != 0){
                    switch (rule){
                        case BindingMatchRule_Strict:
                        {
                            for (SNode *node = list->first;
                                 node != 0;
                                 node = node->next){
                                Command_Modified_Binding *mod_binding = CastFromMember(Command_Modified_Binding, order_node, node);
                                Input_Modifier_Set *binding_mod_set = &mod_binding->mods;
                                if (map_strict_match(binding_mod_set, breakdown.mod_set, breakdown.skip_self_mod)){
                                    result = mod_binding->binding;
                                    goto done;
                                }
                            }
                        }break;
                        
                        case BindingMatchRule_Loose:
                        {
                            for (SNode *node = list->first;
                                 node != 0;
                                 node = node->next){
                                Command_Modified_Binding *mod_binding = CastFromMember(Command_Modified_Binding, order_node, node);
                                Input_Modifier_Set *binding_mod_set = &mod_binding->mods;
                                if (map_loose_match(binding_mod_set, breakdown.mod_set)){
                                    result = mod_binding->binding;
                                    goto done;
                                }
                            }
                        }break;
                    }
                    done:;
                }
                else{
                    Command_Modified_Binding *mod_binding = CastFromMember(Command_Modified_Binding, order_node, list->first);
                    result = mod_binding->binding;
                }
            }
        }
    }
    
    return(result);
}

function Command_Binding
map_get_binding_non_recursive(Command_Map *map, Input_Event *event){
    Command_Binding result = map_get_binding_non_recursive(map, event, BindingMatchRule_Strict);
    if (result.custom == 0){
        result = map_get_binding_non_recursive(map, event, BindingMatchRule_Loose);
    }
    return(result);
}

function Command_Binding
map_get_binding_recursive(Mapping *mapping, Command_Map *map, Input_Event *event, Binding_Match_Rule rule){
    Command_Binding result = {};
    for (i32 safety_counter = 0;
         map != 0 && safety_counter < 40;
         safety_counter += 1){
        result = map_get_binding_non_recursive(map, event, rule);
        if (result.custom != 0){
            break;
        }
        map = mapping_get_map(mapping, map->parent);
    }
    return(result);
}

function Command_Binding
map_get_binding_recursive(Mapping *mapping, Command_Map *map, Input_Event *event){
    Command_Binding result = map_get_binding_recursive(mapping, map, event, BindingMatchRule_Strict);
    if (result.custom == 0){
        result = map_get_binding_recursive(mapping, map, event, BindingMatchRule_Loose);
    }
    return(result);
}

function void
map_set_parent(Command_Map *map, Command_Map *parent){
    if (map != 0 && parent != 0){
        map->parent = parent->id;
    }
}

function void
map_null_parent(Command_Map *map){
    map->parent = 0;
}

function void
map__command_add_trigger(Command_Map *map, Command_Binding binding, Command_Trigger *trigger){
    if (map != 0){
        u64 key = (u64)(PtrAsInt(BindingGetPtr(binding)));
        Table_Lookup lookup = table_lookup(&map->cmd_to_binding_trigger, key);
        Command_Trigger_List *list = 0;
        if (!lookup.found_match){
            list = push_array_zero(&map->node_arena, Command_Trigger_List, 1);
            table_insert(&map->cmd_to_binding_trigger, key, (u64)(PtrAsInt(list)));
        }
        else{
            u64 val = 0;
            table_read(&map->cmd_to_binding_trigger, lookup, &val);
            list = (Command_Trigger_List*)IntAsPtr(val);
        }
        Command_Trigger *trigger_ptr = push_array(&map->node_arena, Command_Trigger, 1);
        block_copy_struct(trigger_ptr, trigger);
        sll_queue_push(list->first, list->last, trigger_ptr);
    }
}

function Input_Event
map_trigger_as_event(Command_Trigger *trigger){
    Input_Event result = {};
    result.kind = trigger->kind;
    switch (result.kind){
        case InputEventKind_TextInsert:
        {}break;
        
        case InputEventKind_KeyStroke:
        case InputEventKind_KeyRelease:
        {
            result.key.code = trigger->sub_code;
            result.key.modifiers = trigger->mods;
        }break;
        
        case InputEventKind_MouseButton:
        case InputEventKind_MouseButtonRelease:
        {
            result.mouse.code = trigger->sub_code;
            result.mouse.modifiers = trigger->mods;
        }break;
        
        case InputEventKind_MouseWheel:
        {
            result.mouse_wheel.modifiers = trigger->mods;
        }break;
        
        case InputEventKind_MouseMove:
        {
            result.mouse_move.modifiers = trigger->mods;
        }break;
        
        case InputEventKind_Core:
        {
            result.core.code = trigger->sub_code;
        }break;
    }
    return(result);
}

function Command_Trigger_List
map_get_triggers_non_recursive(Mapping *mapping, Command_Map *map, Command_Binding binding){
    Command_Trigger_List *result_ptr = 0;
    if (map != 0){
        u64 key = (u64)(PtrAsInt(BindingGetPtr(binding)));
        Table_Lookup lookup = table_lookup(&map->cmd_to_binding_trigger, key);
        if (lookup.found_match){
            u64 val = 0;
            table_read(&map->cmd_to_binding_trigger, lookup, &val);
            result_ptr = (Command_Trigger_List*)IntAsPtr(val);
            
            Command_Trigger_List list = {};
            for (Command_Trigger *node = result_ptr->first, *next = 0;
                 node != 0;
                 node = next){
                next = node->next;
                Input_Event event = map_trigger_as_event(node);
                Command_Binding this_binding = {};
                if (mapping != 0){
                    this_binding = map_get_binding_recursive(mapping, map, &event);
                }
                else{
                    this_binding = map_get_binding_non_recursive(map, &event);
                }
                if (BindingGetPtr(this_binding) == BindingGetPtr(binding)){
                    sll_queue_push(list.first, list.last, node);
                }
            }
            *result_ptr = list;
        }
    }
    Command_Trigger_List result = {};
    if (result_ptr != 0){
        result = *result_ptr;
    }
    return(result);
}

function Command_Trigger_List
map_get_triggers_non_recursive(Command_Map *map, Command_Binding binding){
    return(map_get_triggers_non_recursive(0, map, binding));
}

function Command_Trigger_List
map_get_triggers_recursive(Arena *arena, Mapping *mapping, Command_Map *map, Command_Binding binding){
    Command_Trigger_List result = {};
    if (mapping != 0){
        for (i32 safety_counter = 0;
             map != 0 && safety_counter < 40;
             safety_counter += 1){
            Command_Trigger_List list = map_get_triggers_non_recursive(mapping, map, binding);
            
            for (Command_Trigger *node = list.first, *next = 0;
                 node != 0;
                 node = next){
                next = node->next;
                Command_Trigger *nnode = push_array_write(arena, Command_Trigger, 1, node);
                sll_queue_push(result.first, result.last, nnode);
            }
            
            map = mapping_get_map(mapping, map->parent);
        }
    }
    return(result);
}

function Command_Binding_List*
map_get_binding_list_on_key(Command_Map *map, Key_Code code){
    Command_Binding_List *result = 0;
    if (map != 0){
        u64 key = mapping__key(InputEventKind_KeyStroke, code);
        result = map__get_list(map, key);
    }
    return(result);
}

function Command_Binding_List*
map_get_binding_list_on_mouse_button(Command_Map *map, Mouse_Code code){
    Command_Binding_List *result = 0;
    if (map != 0){
        u64 key = mapping__key(InputEventKind_MouseButton, code);
        result = map__get_list(map, key);
    }
    return(result);
}

function Command_Binding_List*
map_get_binding_list_on_core(Command_Map *map, Core_Code code){
    Command_Binding_List *result = 0;
    if (map != 0){
        u64 key = mapping__key(InputEventKind_Core, code);
        result = map__get_list(map, key);
    }
    return(result);
}

////////////////////////////////

function void
map_set_binding(Mapping *mapping, Command_Map *map, Command_Binding binding, u32 code1, u32 code2, Input_Modifier_Set *mods){
    if (map != 0){
        u64 key = mapping__key(code1, code2);
        Command_Binding_List *list = map__get_or_make_list(mapping, map, key);
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
        mod_binding->mods = copy_modifier_set(&map->node_arena, mods);
        mod_binding->binding = binding;
        
        Command_Trigger trigger = {};
        trigger.kind = code1;
        trigger.sub_code = code2;
        trigger.mods = mod_binding->mods;
        map__command_add_trigger(map, binding, &trigger);
    }
}

function void
map_set_binding_key(Mapping *mapping, Command_Map *map, Command_Binding binding, Key_Code code, Input_Modifier_Set *modifiers){
    map_set_binding(mapping, map, binding, InputEventKind_KeyStroke, code, modifiers);
}

function void
map_set_binding_mouse(Mapping *mapping, Command_Map *map, Command_Binding binding, Mouse_Code code, Input_Modifier_Set *modifiers){
    map_set_binding(mapping, map, binding, InputEventKind_MouseButton, code, modifiers);
}

function void
map_set_binding_core(Mapping *mapping, Command_Map *map, Command_Binding binding, Core_Code code, Input_Modifier_Set *modifiers){
    map_set_binding(mapping, map, binding, InputEventKind_Core, code, modifiers);
}

function void
map_set_binding_text_input(Command_Map *map, Command_Binding binding){
    if (map != 0){
        map->text_input_command = binding;
        Command_Trigger trigger = {};
        trigger.kind = InputEventKind_TextInsert;
        map__command_add_trigger(map, binding, &trigger);
    }
}

////////////////////////////////

function Command_Binding_List*
map_get_binding_list_on_key(Mapping *mapping, Command_Map_ID map_id, Key_Code code){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_binding_list_on_key(map, code));
}

function Command_Binding
map_get_binding_non_recursive(Mapping *mapping, Command_Map_ID map_id, Input_Event *event){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_binding_non_recursive(map, event));
}

function Command_Binding
map_get_binding_recursive(Mapping *mapping, Command_Map_ID map_id, Input_Event *event){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_binding_recursive(mapping, map, event));
}

function Command_Trigger_List
map_get_triggers_non_recursive(Mapping *mapping, Command_Map_ID map_id, Command_Binding binding){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_triggers_non_recursive(map, binding));
}

function Command_Trigger_List
map_get_triggers_recursive(Arena *arena, Mapping *mapping, Command_Map_ID map_id, Command_Binding binding){
    Command_Map *map = mapping_get_map(mapping, map_id);
    return(map_get_triggers_recursive(arena, mapping, map, binding));
}

function void
map_set_parent(Mapping *mapping, Command_Map_ID map_id, Command_Map_ID parent_id){
    Command_Map *map = mapping_get_map(mapping, map_id);
    Command_Map *parent = mapping_get_map(mapping, parent_id);
    map_set_parent(map, parent);
}

function void
map_set_parent(Mapping *mapping, Command_Map *map, Command_Map_ID parent_id){
    Command_Map *parent = mapping_get_map(mapping, parent_id);
    map_set_parent(map, parent);
}

function void
map_null_parent(Mapping *mapping, Command_Map_ID map_id){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_null_parent(map);
}

function void
map_set_binding(Mapping *mapping, Command_Map_ID map_id, Command_Binding binding,
                u32 code1, u32 code2, Input_Modifier_Set *modifiers){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding(mapping, map, binding, code1, code2, modifiers);
}

function void
map_set_binding_key(Mapping *mapping, Command_Map_ID map_id, Command_Binding binding,
                    Key_Code code, Input_Modifier_Set *modifiers){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding_key(mapping, map, binding, code, modifiers);
}

function void
map_set_binding_mouse(Mapping *mapping, Command_Map_ID map_id, Command_Binding binding,
                      Mouse_Code code, Input_Modifier_Set *modifiers){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding_mouse(mapping, map, binding, code, modifiers);
}

function void
map_set_binding_core(Mapping *mapping, Command_Map_ID map_id, Command_Binding binding,
                     Core_Code code, Input_Modifier_Set *modifiers){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding_core(mapping, map, binding, code, modifiers);
}

function void
map_set_binding_text_input(Mapping *mapping, Command_Map_ID map_id, Command_Binding binding){
    Command_Map *map = mapping_get_map(mapping, map_id);
    map_set_binding_text_input(map, binding);
}

////////////////////////////////

function void
command_trigger_stringize_mods(Arena *arena, List_String_Const_u8 *list, Input_Modifier_Set *modifiers){
    if (modifiers->count > 0){
        string_list_push(arena, list, string_u8_litexpr(" holding:"));
        i32 count = modifiers->count;
        Key_Code *mods = modifiers->mods;
        for (i32 i = 0; i < count; i += 1){
            string_list_pushf(arena, list, " %s", ArraySafe(key_code_name, mods[i]));
        }
    }
}

function void
command_trigger_stringize(Arena *arena, List_String_Const_u8 *list, Command_Trigger *trigger){
    string_list_push(arena, list, string_u8_litexpr("<"));
    switch (trigger->kind){
        case InputEventKind_TextInsert:
        {
            string_list_push(arena, list, string_u8_litexpr("TextInsert"));
        }break;
        
        case InputEventKind_KeyStroke:
        {
            String_Const_u8 key_name = SCu8(ArraySafe(key_code_name, trigger->sub_code));
            string_list_push(arena, list, key_name);
            command_trigger_stringize_mods(arena, list, &trigger->mods);
        }break;
        
        case InputEventKind_KeyRelease:
        {
            string_list_pushf(arena, list, "Release %s", ArraySafe(key_code_name, trigger->sub_code));
            command_trigger_stringize_mods(arena, list, &trigger->mods);
        }break;
        
        case InputEventKind_MouseButton:
        {
            string_list_pushf(arena, list, "Mouse %s", ArraySafe(mouse_code_name, trigger->sub_code));
            command_trigger_stringize_mods(arena, list, &trigger->mods);
        }break;
        
        case InputEventKind_MouseButtonRelease:
        {
            string_list_pushf(arena, list, "Release Mouse %s", ArraySafe(mouse_code_name, trigger->sub_code));
            command_trigger_stringize_mods(arena, list, &trigger->mods);
        }break;
        
        case InputEventKind_MouseWheel:
        {
            string_list_push(arena, list, string_u8_litexpr("MouseWheel"));
            command_trigger_stringize_mods(arena, list, &trigger->mods);
        }break;
        
        case InputEventKind_MouseMove:
        {
            string_list_push(arena, list, string_u8_litexpr("MouseMove"));
            command_trigger_stringize_mods(arena, list, &trigger->mods);
        }break;
        
        case InputEventKind_Core:
        {
            string_list_pushf(arena, list, "Core %s", ArraySafe(core_code_name, trigger->sub_code));
        }break;
        
        default:
        {
            string_list_push(arena, list, string_u8_litexpr("ERROR unexpected trigger kind"));
        }break;
    }
    string_list_push(arena, list, string_u8_litexpr(">"));
}

////////////////////////////////

function void
map_set_binding_lv(Mapping *mapping, Command_Map *map,
                   Command_Binding binding, u32 code1, u32 code2, va_list args){
    Input_Modifier_Set mods = {};
    Key_Code mods_array[Input_MaxModifierCount];
    mods.mods = mods_array;
    for (;mods.count < ArrayCount(mods_array);){
        i32 v = va_arg(args, i32);
        if (v <= 0){
            break;
        }
        mods.mods[mods.count] = v;
        mods.count += 1;
    }
    return(map_set_binding(mapping, map, binding, code1, code2, &mods));
}

#if MAP_METADATA_ONLY
function void
map_set_binding_l(Mapping *mapping, Command_Map *map, char *name, u32 code1, u32 code2, ...){
    va_list args;
    va_start(args, code2);
    Command_Binding binding = {};
    binding.name = name;
    map_set_binding_lv(mapping, map, binding, code1, code2, args);
    va_end(args);
}
#else
function void
map_set_binding_l(Mapping *mapping, Command_Map *map, Custom_Command_Function *custom, u32 code1, u32 code2, ...){
    va_list args;
    va_start(args, code2);
    Command_Binding binding = {};
    binding.custom = custom;
    map_set_binding_lv(mapping, map, binding, code1, code2, args);
    va_end(args);
}
#endif

#if MAP_METADATA_ONLY
# define BindFWrap_(F) stringify(F)
#else
# define BindFWrap_(F) F
#endif

#define MappingScope() Mapping *m = 0; Command_Map *map = 0
#define SelectMapping(N) m = (N)
#define SelectMap(ID) map = mapping_get_or_make_map(m, (ID))
#define ParentMap(ID) map_set_parent(m, map, (ID))
#define BindTextInput(F) map_set_binding_text_input(map, BindFWrap_(F))

#if COMPILER_CL

#define Bind(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_KeyStroke, (K), __VA_ARGS__, 0)
#define BindRelease(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_KeyRelease, (K), __VA_ARGS__, 0)
#define BindMouse(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseButton, (K), __VA_ARGS__, 0)
#define BindMouseRelease(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseButtonRelease, (K), __VA_ARGS__, 0)
#define BindMouseWheel(F, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseWheel, 0, __VA_ARGS__, 0)
#define BindMouseMove(F, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseMove, 0, __VA_ARGS__, 0)
#define BindCore(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_Core, (K), __VA_ARGS__, 0)

#elif COMPILER_GCC | COMPILER_CLANG

#define Bind(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_KeyStroke, (K), ##__VA_ARGS__, 0)
#define BindRelease(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_KeyRelease, (K), ##__VA_ARGS__, 0)
#define BindMouse(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseButton, (K), ##__VA_ARGS__, 0)
#define BindMouseRelease(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseButtonRelease, (K), ##__VA_ARGS__, 0)
#define BindMouseWheel(F, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseWheel, 0, ##__VA_ARGS__, 0)
#define BindMouseMove(F, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_MouseMove, 0, ##__VA_ARGS__, 0)
#define BindCore(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_Core, (K), ##__VA_ARGS__, 0)

#else
#error "Unsupported compiler"
#endif

// BOTTOM

