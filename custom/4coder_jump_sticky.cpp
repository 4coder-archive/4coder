/*
4coder_jump_sticky.cpp - Commands and helpers for parsing jump locations from 
compiler errors, sticking markers on jump locations, and jumping to them.
*/

// TOP

global Marker_List_Node *marker_list_first = 0;
global Marker_List_Node *marker_list_last = 0;

////////////////////////////////

internal i32
binary_search(i64 *array, i32 stride, i32 count, i64 x){
    u8 *raw = (u8*)array;
    i32 i = 0;
    i32 first = 0;
    i32 last = count;
    if (first < last){
        for (;;){
            i = (first + last)/2;
            i64 k = *(i64*)(raw + stride*i);
            if (k < x){
                first = i;
            }
            else if (k > x){
                last = i;
            }
            else{ // NOTE(allen): array[i] == x
                break;
            }
            if (first + 1 >= last){
                i = first;
                break;
            }
        }
    }
    return(i);
}

internal Sticky_Jump_Array
parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_ID buffer){
    Sticky_Jump_Node *jump_first = 0;;
    Sticky_Jump_Node *jump_last = 0;
    i32 jump_count = 0;
    
    for (i32 line = 1;; line += 1){
        b32 output_jump = false;
        i32 colon_index = 0;
        b32 is_sub_error = false;
        Buffer_ID out_buffer_id = 0;
        i64 out_pos = 0;
        
        {
            Temp_Memory_Block line_auto_closer(arena);
            if (is_valid_line(app, buffer, line)){
                String_Const_u8 line_str = push_buffer_line(app, arena, buffer, line);
                Parsed_Jump parsed_jump = parse_jump_location(line_str);
                if (parsed_jump.success){
                    Buffer_ID jump_buffer = {};
                    if (open_file(app, &jump_buffer, parsed_jump.location.file, false, true)){
                        if (buffer_exists(app, jump_buffer)){
                            Buffer_Cursor cursor = buffer_compute_cursor(app, jump_buffer, seek_jump(parsed_jump));
                            if (cursor.line > 0){
                                out_buffer_id = jump_buffer;
                                out_pos = cursor.pos;
                                output_jump = true;
                            }
                        }
                    }
                }
            }
            else{
                break;
            }
        }
        
        if (output_jump){
            Sticky_Jump_Node *jump = push_array(arena, Sticky_Jump_Node, 1);
            sll_queue_push(jump_first, jump_last, jump);
            jump_count += 1;
            jump->jump.list_line = line;
            jump->jump.list_colon_index = colon_index;
            jump->jump.is_sub_error =  is_sub_error;
            jump->jump.jump_buffer_id = out_buffer_id;
            jump->jump.jump_pos = out_pos;
        }
    }
    
    Sticky_Jump_Array result = {};
    result.count = jump_count;
    result.jumps = push_array(arena, Sticky_Jump, result.count);
    i32 index = 0;
    for (Sticky_Jump_Node *node = jump_first;
         node != 0;
         node = node->next){
        result.jumps[index] = node->jump;
        index += 1;
    }
    
    return(result);
}

internal void
init_marker_list(Application_Links *app, Heap *heap, Buffer_ID buffer, Marker_List *list){
    Scratch_Block scratch(app);
    
    Sticky_Jump_Array jumps = parse_buffer_to_jump_array(app, scratch, buffer);
    Range_i32_Array buffer_ranges = get_ranges_of_duplicate_keys(scratch, &jumps.jumps->jump_buffer_id, sizeof(*jumps.jumps), jumps.count);
    Sort_Pair_i32 *range_index_buffer_id_pairs = push_array(scratch, Sort_Pair_i32, buffer_ranges.count);
    for (i32 i = 0; i < buffer_ranges.count; i += 1){
        range_index_buffer_id_pairs[i].index = i;
        range_index_buffer_id_pairs[i].key = jumps.jumps[buffer_ranges.ranges[i].first].jump_buffer_id;
    }
    sort_pairs_by_key(range_index_buffer_id_pairs, buffer_ranges.count);
    Range_i32_Array scoped_buffer_ranges = get_ranges_of_duplicate_keys(scratch,
                                                                        &range_index_buffer_id_pairs->key,
                                                                        sizeof(*range_index_buffer_id_pairs),
                                                                        buffer_ranges.count);
    
    Sticky_Jump_Stored *stored = push_array(scratch, Sticky_Jump_Stored, jumps.count);
    
    Managed_Scope scope_array[2] = {};
    scope_array[0] = buffer_get_managed_scope(app, buffer);
    
    for (i32 i = 0; i < scoped_buffer_ranges.count; i += 1){
        Range_i32 buffer_range_indices = scoped_buffer_ranges.ranges[i];
        
        u32 total_jump_count = 0;
        for (i32 j = buffer_range_indices.first;
             j < buffer_range_indices.one_past_last;
             j += 1){
            i32 range_index = range_index_buffer_id_pairs[j].index;
            Range_i32 range = buffer_ranges.ranges[range_index];
            total_jump_count += range_size(range);
        }
        
        Temp_Memory marker_temp = begin_temp(scratch);
        Marker *markers = push_array(scratch, Marker, total_jump_count);
        Buffer_ID target_buffer_id = 0;
        u32 marker_index = 0;
        for (i32 j = buffer_range_indices.first;
             j < buffer_range_indices.one_past_last;
             j += 1){
            i32 range_index = range_index_buffer_id_pairs[j].index;
            Range_i32 range = buffer_ranges.ranges[range_index];
            if (target_buffer_id == 0){
                target_buffer_id = jumps.jumps[range.first].jump_buffer_id;
            }
            for (i32 k = range.first; k < range.one_past_last; k += 1){
                markers[marker_index].pos = jumps.jumps[k].jump_pos;
                markers[marker_index].lean_right = false;
                stored[k].list_line        = jumps.jumps[k].list_line;
                stored[k].list_colon_index = jumps.jumps[k].list_colon_index;
                stored[k].is_sub_error     = jumps.jumps[k].is_sub_error;
                stored[k].jump_buffer_id   = jumps.jumps[k].jump_buffer_id;
                stored[k].index_into_marker_array = marker_index;
                marker_index += 1;
            }
        }
        
        scope_array[1] = buffer_get_managed_scope(app, target_buffer_id);
        Managed_Scope scope = get_managed_scope_with_multiple_dependencies(app, scope_array, ArrayCount(scope_array));
        Managed_Object marker_handle = alloc_buffer_markers_on_buffer(app, target_buffer_id, total_jump_count, &scope);
        managed_object_store_data(app, marker_handle, 0, total_jump_count, markers);
        
        end_temp(marker_temp);
        
        Assert(managed_object_get_item_size(app, marker_handle) == sizeof(Marker));
        Assert(managed_object_get_item_count(app, marker_handle) == total_jump_count);
        Assert(managed_object_get_type(app, marker_handle) == ManagedObjectType_Markers);
        
        Managed_Object *marker_handle_ptr = scope_attachment(app, scope, sticky_jump_marker_handle, Managed_Object);
        if (marker_handle_ptr != 0){
            *marker_handle_ptr = marker_handle;
        }
    }
    
    Managed_Object stored_jump_array = alloc_managed_memory_in_scope(app, scope_array[0], sizeof(Sticky_Jump_Stored), jumps.count);
    managed_object_store_data(app, stored_jump_array, 0, jumps.count, stored);
    
    list->jump_array = stored_jump_array;
    list->jump_count = jumps.count;
    list->previous_size = (i32)buffer_get_size(app, buffer);
    list->buffer_id = buffer;
}

internal void
delete_marker_list(Marker_List_Node *node){
    zdll_remove(marker_list_first, marker_list_last, node);
}

internal void
delete_marker_list(Marker_List *list){
    delete_marker_list(CastFromMember(Marker_List_Node, list, list));
}

internal Marker_List*
make_new_marker_list_for_buffer(Heap *heap, i32 buffer_id){
    Marker_List_Node *new_node = heap_array(heap, Marker_List_Node, 1);
    zdll_push_back(marker_list_first, marker_list_last, new_node);
    new_node->buffer_id = buffer_id;
    block_zero_struct(&new_node->list);
    Marker_List *result = &new_node->list;
    return(result);
}

internal Marker_List*
get_marker_list_for_buffer(Buffer_ID buffer_id){
    for (Marker_List_Node *node = marker_list_first;
         node != 0;
         node = node->next){
        if (buffer_id == node->buffer_id){
            return(&node->list);
        }
    }
    return(0);
}

internal Marker_List*
get_or_make_list_for_buffer(Application_Links *app, Heap *heap, Buffer_ID buffer_id){
    Marker_List *result = get_marker_list_for_buffer(buffer_id);
    if (result != 0){
        i32 buffer_size = (i32)buffer_get_size(app, buffer_id);
        // TODO(allen):  // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): When buffers get an "edit sequence number" use that instead.
        if (result->previous_size != buffer_size){
            delete_marker_list(result);
            result = 0;
        }
    }
    if (result == 0){
        result = make_new_marker_list_for_buffer(heap, buffer_id);
        init_marker_list(app, heap, buffer_id, result);
        if (result->jump_count == 0){
            delete_marker_list(result);
            result = 0;
        }
    }
    return(result);
}

internal b32
get_stored_jump_from_list(Application_Links *app, Marker_List *list, i32 index,
                          Sticky_Jump_Stored *stored_out){
    Sticky_Jump_Stored stored = {};
    if (list != 0){
        if (managed_object_load_data(app, list->jump_array, index, 1, &stored)){
            *stored_out = stored;
            return(true);
        }
    }
    return(false);
}

internal Sticky_Jump_Stored*
get_all_stored_jumps_from_list(Application_Links *app, Arena *arena, Marker_List *list){
    Sticky_Jump_Stored *stored = 0;
    if (list != 0){
        Temp_Memory restore_point = begin_temp(arena);
        stored = push_array(arena, Sticky_Jump_Stored, list->jump_count);
        if (stored != 0){
            if (!managed_object_load_data(app, list->jump_array, 0, list->jump_count, stored)){
                stored = 0;
                end_temp(restore_point);
            }
        }
    }
    return(stored);
}

internal b32
get_jump_from_list(Application_Links *app, Marker_List *list, i32 index, ID_Pos_Jump_Location *location){
    b32 result = false;
    Sticky_Jump_Stored stored = {};
    if (get_stored_jump_from_list(app, list, index, &stored)){
        Buffer_ID target_buffer_id = stored.jump_buffer_id;
        
        Managed_Scope scope_array[2] = {};
        scope_array[0] = buffer_get_managed_scope(app, list->buffer_id);
        scope_array[1] = buffer_get_managed_scope(app, target_buffer_id);
        Managed_Scope scope = get_managed_scope_with_multiple_dependencies(app, scope_array, ArrayCount(scope_array));
        
        Managed_Object *marker_array = scope_attachment(app, scope, sticky_jump_marker_handle, Managed_Object);
        if (marker_array != 0 && *marker_array != 0){
            Marker marker = {};
            managed_object_load_data(app, *marker_array, stored.index_into_marker_array, 1, &marker);
            location->buffer_id = target_buffer_id;
            location->pos = marker.pos;
            result = true;
        }
    }
    return(result);
}

internal i64
get_line_from_list(Application_Links *app, Marker_List *list, i32 index){
    i64 result = 0;
    if (list != 0){
        Sticky_Jump_Stored stored = {};
        if (get_stored_jump_from_list(app, list, index, &stored)){
            result = stored.list_line;
        }
    }
    return(result);
}

internal b32
get_is_sub_error_from_list(Application_Links *app, Marker_List *list, i32 index){
    b32 result = false;
    if (list != 0){
        Sticky_Jump_Stored stored = {};
        if (get_stored_jump_from_list(app, list, index, &stored)){
            result = stored.is_sub_error;
        }
    }
    return(result);
}

internal i32
get_index_nearest_from_list(Application_Links *app, Marker_List *list, i64 line){
    i32 result = -1;
    if (list != 0){
        Scratch_Block scratch(app);
        Sticky_Jump_Stored *stored = get_all_stored_jumps_from_list(app, scratch, list);
        if (stored != 0){
            result = binary_search((i64*)&stored->list_line, sizeof(*stored), list->jump_count, line);
        }
    }
    return(result);
}

internal i32
get_index_exact_from_list(Application_Links *app, Marker_List *list, i64 line){
    i32 result = -1;
    if (list != 0){
        Scratch_Block scratch(app);
        Sticky_Jump_Stored *stored = get_all_stored_jumps_from_list(app, scratch, list);
        if (stored != 0){
            i32 index = binary_search((i64*)&stored->list_line, sizeof(*stored), list->jump_count, line);
            if (stored[index].list_line == line){
                result = index;
            }
        }
    }
    return(result);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor)
CUSTOM_DOC("If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.")
{
    Heap *heap = &global_heap;
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    Marker_List *list = get_or_make_list_for_buffer(app, heap, buffer);
    
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));
    
    i32 list_index = get_index_exact_from_list(app, list, cursor.line);
    
    if (list_index >= 0){
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, list, list_index, &location)){
            if (get_jump_buffer(app, &buffer, &location)){
                change_active_panel(app);
                View_ID target_view = get_active_view(app, Access_Always);
                switch_to_existing_view(app, target_view, buffer);
                jump_to_location(app, target_view, buffer, location);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel)
CUSTOM_DOC("If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list.")
{
    Heap *heap = &global_heap;
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    
    Marker_List *list = get_or_make_list_for_buffer(app, heap, buffer);
    
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));
    
    i32 list_index = get_index_exact_from_list(app, list, cursor.line);
    
    if (list_index >= 0){
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, list, list_index, &location)){
            if (get_jump_buffer(app, &buffer, &location)){
                jump_to_location(app, view, buffer, location);
            }
        }
    }
}

internal void
goto_jump_in_order(Application_Links *app, Marker_List *list, View_ID jump_view, ID_Pos_Jump_Location location){
    Buffer_ID buffer = {};
    if (get_jump_buffer(app, &buffer, &location)){
        View_ID target_view = get_active_view(app, Access_Always);
        if (target_view == jump_view){
            change_active_panel(app);
            target_view = get_active_view(app, Access_Always);
        }
        switch_to_existing_view(app, target_view, buffer);
        jump_to_location(app, target_view, buffer, location);
        prev_location = location;
    }
}

internal b32
jump_is_repeat(ID_Pos_Jump_Location prev, ID_Pos_Jump_Location location){
    return(prev.buffer_id == location.buffer_id && prev.pos == location.pos);
}

internal void
goto_next_filtered_jump(Application_Links *app, Marker_List *list, View_ID jump_view, i32 list_index, i32 direction, b32 skip_repeats, b32 skip_sub_errors){
    Assert(direction == 1 || direction == -1);
    
    if (list != 0){
        for (;list_index >= 0 && list_index < list->jump_count;){
            ID_Pos_Jump_Location location = {};
            if (get_jump_from_list(app, list, list_index, &location)){
                b32 skip_this = false;
                if (skip_repeats && jump_is_repeat(prev_location, location)){
                    skip_this = true;
                }
                else if (skip_sub_errors && get_is_sub_error_from_list(app, list, list_index)){
                    skip_this = true;
                }
                
                if (!skip_this){
                    goto_jump_in_order(app, list, jump_view, location);
                    i64 updated_line = get_line_from_list(app, list, list_index);
                    view_set_cursor_and_preferred_x(app, jump_view, seek_line_col(updated_line, 1));
                    break;
                }
            }
            
            list_index += direction;
        }
    }
}

internal Locked_Jump_State
get_locked_jump_state(Application_Links *app, Heap *heap){
    Locked_Jump_State result = {};
    result.view = get_view_for_locked_jump_buffer(app);
    if (result.view != 0){
        Buffer_ID buffer = view_get_buffer(app, result.view, Access_Always);
        result.list = get_or_make_list_for_buffer(app, heap, buffer);
        
        i64 cursor_position = view_get_cursor_pos(app, result.view);
        Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(cursor_position));
        result.list_index = get_index_nearest_from_list(app, result.list, cursor.line);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(goto_next_jump)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.")
{
    Heap *heap = &global_heap;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, heap);
    if (jump_state.view != 0){
        i64 cursor_position = view_get_cursor_pos(app, jump_state.view);
        Buffer_Cursor cursor = view_compute_cursor(app, jump_state.view, seek_pos(cursor_position));
        i64 line = get_line_from_list(app, jump_state.list, jump_state.list_index);
        if (line <= cursor.line){
            jump_state.list_index += 1;
        }
        goto_next_filtered_jump(app, jump_state.list, jump_state.view, jump_state.list_index, 1, true, true);
    }
}

CUSTOM_COMMAND_SIG(goto_prev_jump)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations."){
    Heap *heap = &global_heap;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, heap);
    if (jump_state.view != 0){
        if (jump_state.list_index > 0){
            --jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, jump_state.view, jump_state.list_index, -1, true, true);
    }
}

CUSTOM_COMMAND_SIG(goto_next_jump_no_skips)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.")
{
    Heap *heap = &global_heap;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, heap);
    if (jump_state.view != 0){
        i64 cursor_position = view_get_cursor_pos(app, jump_state.view);
        Buffer_Cursor cursor = view_compute_cursor(app, jump_state.view, seek_pos(cursor_position));
        i64 line = get_line_from_list(app, jump_state.list, jump_state.list_index);
        if (line <= cursor.line){
            ++jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, jump_state.view, jump_state.list_index, 1, true, false);
    }
}

CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.")
{
    Heap *heap = &global_heap;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, heap);
    if (jump_state.view != 0){
        if (jump_state.list_index > 0){
            --jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, jump_state.view, jump_state.list_index, -1, true, false);
    }
}

CUSTOM_COMMAND_SIG(goto_first_jump)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.")
{
    Heap *heap = &global_heap;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, heap);
    if (jump_state.view != 0){
        i32 list_index = 0;
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, jump_state.list, list_index, &location)){
            goto_jump_in_order(app, jump_state.list, jump_state.view, location);
            i64 updated_line = get_line_from_list(app, jump_state.list, list_index);
            view_set_cursor_and_preferred_x(app, jump_state.view, seek_line_col(updated_line, 1));
        }
    }
}

CUSTOM_COMMAND_SIG(goto_first_jump_same_panel_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the first jump in the buffer and views the buffer in the panel where the jump list was.")
{
    Heap *heap = &global_heap;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, heap);
    if (jump_state.view != 0){
        i32 list_index = 0;
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, jump_state.list, list_index, &location)){
            Buffer_ID buffer = {};
            if (get_jump_buffer(app, &buffer, &location)){
                jump_to_location(app, jump_state.view, buffer, location);
            }
        }
    }
}

//
// Insert Newline or Tigger Jump on Read Only Buffer
//

CUSTOM_COMMAND_SIG(if_read_only_goto_position)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            goto_jump_at_cursor(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        leave_current_input_unhandled(app);
    }
}

CUSTOM_COMMAND_SIG(if_read_only_goto_position_same_panel)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            goto_jump_at_cursor_same_panel(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        leave_current_input_unhandled(app);
    }
}

//
// End File Hook
//

BUFFER_HOOK_SIG(default_end_buffer);
BUFFER_HOOK_SIG(end_buffer_close_jump_list){
    Marker_List *list = get_marker_list_for_buffer(buffer_id);
    if (list != 0){
        delete_marker_list(list);
    }
    default_end_buffer(app, buffer_id);
    return(0);
}

// BOTTOM

