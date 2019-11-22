/*
* Mr. 4th Dimention - Allen Webster
*
* 23.07.2019
*
* Type for organizating the set of all loaded font faces.
*
*/

// TOP

internal Face_ID
font_set__alloc_face_id(Font_Set *set){
    Face_ID result = 0;
    if (set->free_ids != 0){
        Font_Face_ID_Node *node = set->free_ids;
        result = node->id;
        sll_stack_pop(set->free_ids);
        sll_stack_push(set->free_id_nodes, node);
    }
    else{
        result = set->next_id_counter;
        set->next_id_counter += 1;
    }
    return(result);
}

internal void
font_set__free_face_id(Font_Set *set, Face_ID id){
    if (id + 1 == set->next_id_counter){
        set->next_id_counter -= 1;
    }
    else{
        Font_Face_ID_Node *node = 0;
        if (set->free_id_nodes == 0){
            node = push_array(&set->arena, Font_Face_ID_Node, 1);
        }
        else{
            node = set->free_id_nodes;
            sll_stack_pop(set->free_id_nodes);
        }
        sll_stack_push(set->free_ids, node);
        node->id = id;
    }
}

internal Font_Face_Slot*
font_set__alloc_face_slot(Font_Set *set){
    Font_Face_Slot *result = 0;
    if (set->free_face_slots == 0){
        result = push_array(&set->arena, Font_Face_Slot, 1);
    }
    else{
        result = set->free_face_slots;
        sll_stack_pop(set->free_face_slots);
    }
    return(result);
}

internal void
font_set__free_face_slot(Font_Set *set, Font_Face_Slot *slot){
    if (slot->arena.base_allocator != 0){
        table_free(&slot->face->advance_map.codepoint_to_index.table);
        linalloc_clear(&slot->arena);
    }
    block_zero_struct(slot);
    sll_stack_push(set->free_face_slots, slot);
}

internal void
font_set_init(Font_Set *set){
    block_zero_struct(set);
    set->arena = make_arena_system();
    set->next_id_counter = 1;
    set->id_to_slot_table = make_table_u64_u64(set->arena.base_allocator, 40);
    set->scale_factor = system_get_screen_scale_factor();
}

internal Face*
font_set_new_face(Font_Set *set, Face_Description *description){
    Arena arena = make_arena_system();
    Face *face = font_make_face(&arena, description, set->scale_factor);
    if (face != 0){
        Font_Face_Slot *slot = font_set__alloc_face_slot(set);
        slot->arena = arena;
        slot->face = face;
        Face_ID new_id = font_set__alloc_face_id(set);
        face->id = new_id;
        table_insert(&set->id_to_slot_table, new_id, (u64)slot);
    }
    else{
        linalloc_clear(&arena);
    }
    return(face);
}

internal Font_Face_Slot*
font_set__get_face_slot(Font_Set *set, Face_ID id){
    Font_Face_Slot *result = 0;
    u64 slot_ptr_u64 = 0;
    if (table_read(&set->id_to_slot_table, id, &slot_ptr_u64)){
        result = (Font_Face_Slot*)slot_ptr_u64;
    }
    return(result);
}

internal b32
font_set_release_face(Font_Set *set, Face_ID id){
    b32 result = false;
    Font_Face_Slot *slot = font_set__get_face_slot(set, id);
    if (slot != 0){
        table_erase(&set->id_to_slot_table, id);
        font_set__free_face_slot(set, slot);
        font_set__free_face_id(set, id);
        result = true;
    }
    return(result);
}

internal Face*
font_set_face_from_id(Font_Set *set, Face_ID id){
    Face *result = 0;
    Font_Face_Slot *slot = font_set__get_face_slot(set, id);
    if (slot != 0){
        result = slot->face;
    }
    return(result);
}

internal Face_ID
font_set_get_fallback_face(Font_Set *set){
    Face_ID result = 0;
    for (Face_ID i = 1; i < set->next_id_counter; i += 1){
        if (font_set__get_face_slot(set, i) != 0){
            result = i;
            break;
        }
    }
    return(result);
}

internal Face_ID
font_set_get_largest_id(Font_Set *set){
    return(set->next_id_counter - 1);
}

internal b32
font_set_modify_face(Font_Set *set, Face_ID id, Face_Description *description){
    b32 result = false;
    Font_Face_Slot *slot = font_set__get_face_slot(set, id);
    if (slot != 0){
        i32 version_number = slot->face->version_number;
        Arena arena = make_arena_system();
        Face *face = font_make_face(&arena, description, set->scale_factor);
        if (face != 0){
            linalloc_clear(&slot->arena);
            slot->arena = arena;
            slot->face = face;
            face->version_number = version_number + 1;
            face->id = id;
            result = true;
        }
        else{
            linalloc_clear(&arena);
        }
    }
    return(result);
}

// BOTTOM

