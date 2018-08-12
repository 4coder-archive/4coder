/*
4coder_jump_sticky.cpp - Commands and helpers for parsing jump locations from 
compiler errors, sticking markers on jump locations, and jumping to them.
*/

// TOP

static Marker_List_Node *marker_list_first = 0;
static Marker_List_Node *marker_list_last = 0;

////////////////////////////////

static uint32_t
binary_search(uint32_t *array, int32_t stride, int32_t count, uint32_t x){
    uint8_t *raw = (uint8_t*)array;
    uint32_t i = 0;
    uint32_t first = 0;
    uint32_t last = count;
    if (first < last){
        for (;;){
            i = (first + last)/2;
            uint32_t k = *(uint32_t*)(raw + stride*i);
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

static Sticky_Jump_Destination_Array
make_sticky_jump_destination_array(uint32_t first_jump_index, Marker_Handle handle){
    Sticky_Jump_Destination_Array r;
    r.first_jump_index = first_jump_index;
    r.handle = handle;
    return(r);
}

static Sticky_Jump_Source
make_sticky_jump_source(uint32_t line_number, uint32_t flags){
    Sticky_Jump_Source r;
    r.line_number = line_number;
    r.flags = flags;
    return(r);
}

static void
double_dst_max(General_Memory *general, Marker_List *list){
    uint32_t new_dst_max = list->dst_max*2;
    list->dst = gen_realloc_array(general, Sticky_Jump_Destination_Array, list->dst, list->dst_max, new_dst_max);
    list->dst_max = new_dst_max;
}

static void
double_jump_max(General_Memory *general, Marker_List *list){
    uint32_t new_jump_max = list->jump_max*2;
    list->jumps = gen_realloc_array(general, Sticky_Jump_Source, list->jumps, list->jump_max, new_jump_max);
    list->jump_max = new_jump_max;
}

static void
marker_list_remove_references(Marker_List *list, Marker_Handle handle){
    int32_t count = list->dst_count;
    Sticky_Jump_Destination_Array *dst = list->dst;
    for (int32_t i = 0; i < count; ++i, ++dst){
        if (dst->handle == handle){
            dst->handle = 0;
        }
    }
}

static void
sticky_jump_markers_cleanup(Application_Links *app, Marker_Handle handle, void *user_data, uint32_t user_data_size){
    if (user_data_size == sizeof(Marker_List*)){
        Marker_List *list = *(Marker_List**)user_data;
        marker_list_remove_references(list, handle);
    }
}

static Sticky_Jump_Array
parse_buffer_to_jump_array(Application_Links *app, Partition *arena, Buffer_Summary buffer){
    Sticky_Jump_Array result = {0};
    result.jumps = push_array(arena, Sticky_Jump, 0);
    
    for (int32_t line = 1;; line += 1){
        bool32 output_jump = false;
        int32_t colon_index = 0;
        bool32 is_sub_error = false;
        Buffer_ID out_buffer_id = 0;
        int32_t out_pos = 0;
        
        Temp_Memory temp = begin_temp_memory(arena);
        String line_str = {0};
        if (read_line(app, arena, &buffer, line, &line_str)){
            Name_Line_Column_Location location = {0};
            if (parse_jump_location(line_str, &location, &colon_index, &is_sub_error)){
                Buffer_Summary jump_buffer = {0};
                if (open_file(app, &jump_buffer, location.file.str, location.file.size, false, true)){
                    if (jump_buffer.exists){
                        Partial_Cursor cursor = {0};
                        if (buffer_compute_cursor(app, &jump_buffer,
                                                  seek_line_char(location.line, location.column),
                                                  &cursor)){
                            out_buffer_id = jump_buffer.buffer_id;
                            out_pos = cursor.pos;
                            output_jump = true;
                        }
                    }
                }
            }
        }
        else{
            end_temp_memory(temp);
            break;
        }
        end_temp_memory(temp);
        
        if (output_jump){
            Sticky_Jump *jump = push_array(arena, Sticky_Jump, 1);
            jump->list_line = line;
            jump->list_colon_index = colon_index;
            jump->is_sub_error =  is_sub_error;
            jump->jump_buffer_id = out_buffer_id;
            jump->jump_pos = out_pos;
        }
    }
    
    result.count = (int32_t)(push_array(arena, Sticky_Jump, 0) - result.jumps);
    return(result);
}

static Range_Array
jump_array_mark_buffer_ranges(Partition *arena, Sticky_Jump_Array jumps){
    Range_Array result = {0};
    result.ranges = push_array(arena, Range, 0);
    int32_t start_i = 0;
    for (int32_t i = 1; i <= jumps.count; i += 1){
        bool32 is_end = false;
        if (i == jumps.count){
            is_end = true;
        }
        else if (jumps.jumps[i].jump_buffer_id != jumps.jumps[start_i].jump_buffer_id){
            is_end = true;
        }
        if (is_end){
            Range *new_range = push_array(arena, Range, 1);
            new_range->first = start_i;
            new_range->one_past_last = i;
            start_i = i;
        }
    }
    result.count = (int32_t)(push_array(arena, Range, 0) - result.ranges);
    return(result);
}

#if 1
static void
init_marker_list(Application_Links *app, Partition *part, General_Memory *general, Buffer_ID buffer_id,
                 Marker_List *list){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    
    Temp_Memory temp = begin_temp_memory(part);
    Sticky_Jump_Array jumps = parse_buffer_to_jump_array(app, part, buffer);
    Range_Array buffer_ranges = jump_array_mark_buffer_ranges(part, jumps);
    
    Dynamic_Scope scope_array[2];
    scope_array[0] = buffer_get_dynamic_scope(app, buffer_id);
    
    List_Node_Handle list_sentinel = managed_list_node_alloc(app, scope_array[0]);
    
    for (int32_t i = 0; i < buffer_ranges.count; i += 1){
        Range range = buffer_ranges.ranges[i];
        Buffer_ID target_buffer_id = jumps.jumps[range.first].jump_buffer_id;
        int32_t jump_count = range.one_past_last - range.first;
        
        scope_array[1] = buffer_get_dynamic_scope(app, target_buffer_id);
        Dynamic_Scope scope = get_intersected_dynamic_scope(app, scope_array, ArrayCount(scope_array));
        
        Marker_Handle marker_handle = buffer_add_markers(app, target_buffer_id, jump_count, &scope);
        Temp_Memory marker_temp = begin_temp_memory(part);
        Marker *markers = push_array(part, Marker, jump_count);
        for (int32_t j = 0; j < jump_count; j += 1){
            markers[j].pos = jumps.jumps[j + range.first].jump_pos;
            markers[j].lean_right = false;
        }
        buffer_set_markers(app, handle, 0, jump_count, markers);
        end_temp_memory(marker_temp);
        
        int32_t line_details_mem_size = sizeof(Sticky_Jump_Line_Details)*jump_count;
        Memory_Handle memory = managed_memory_alloc(app, scope, line_details_mem_size);
        Temp_Memory details_temp = begin_temp_memory(part);
        Sticky_Jump_Line_Details *details = push_array(part, Sticky_Jump_Line_Details, jump_count);
        for (int32_t j = 0; j < jump_count; j += 1){
            details[j].list_line        = jumps.jumps[j + range.first].list_line;
            details[j].list_colon_index = jumps.jumps[j + range.first].list_colon_index;
            details[j].is_sub_error     = jumps.jumps[j + range.first].is_sub_error;
        }
        managed_memory_set(app, memory, 0, details, line_details_mem_size);
        end_temp_memory(details_temp);
        
        List_Node_Handle node_handle = managed_list_node_alloc(app, scope, sizeof(Sticky_Jump_Node_Header));
        managed_list_node_insert(app, list_sentinel, node_handle, ListInsert_Back);
        Sticky_Jump_Node_Header node_header = {0};
        node_header.memory = memory;
        node_header.markers = marker_handle;
        node_handle.count = jump_count;
        managed_memory_set(app, node_handle, 0, &node_header, sizeof(node_header));
    }
    end_temp_memory(temp);
}

#else

static void
init_marker_list(Application_Links *app, Partition *part, General_Memory *general, Buffer_ID buffer_id,
                 Marker_List *list){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    
    Temp_Memory temp = begin_temp_memory(part);
    Sticky_Jump_Array jumps = parse_buffer_to_jump_array(app, part, buffer);
    Range_Array buffer_ranges = jump_array_mark_buffer_ranges(part, jumps);
    
    list->dst_max = 64;
    list->dst = gen_array(general, Sticky_Jump_Destination_Array, list->dst_max);
    
    list->jump_max = 64;
    list->jumps = gen_array(general, Sticky_Jump_Source, list->jump_max);
    
    list->previous_size = buffer.size;
    
    uint32_t prev_jump_count = 0;
    for (int32_t i = 0; i < buffer_ranges.count; i += 1){
        Range range = buffer_ranges.ranges[i];
        Buffer_ID target_buffer_id = jumps.jumps[range.first].jump_buffer_id;
        int32_t jump_count = range.one_past_last - range.first;
        
        if (list->dst_count >= list->dst_max){
            double_dst_max(general, list);
        }
        
        Buffer_Summary location_buffer = get_buffer(app, target_buffer_id, AccessAll);
        Marker_Handle new_handle = buffer_add_markers(app, &location_buffer, jump_count,
                                                      sticky_jump_markers_cleanup, &list, sizeof(list));
        
        list->dst[list->dst_count] = make_sticky_jump_destination_array(prev_jump_count, new_handle);
        ++list->dst_count;
        
        prev_jump_count = list->jump_count;
        
        Temp_Memory marker_temp = begin_temp_memory(part);
        Marker *markers = push_array(part, Marker, jump_count);
        for (int32_t j = 0; j < jump_count; j += 1){
            markers[j].pos = jumps.jumps[j + range.first].jump_pos;
            markers[j].lean_right = false;
        }
        buffer_set_markers(app, &location_buffer, new_handle, 0, jump_count, markers);
        end_temp_memory(marker_temp);
        
        for (int32_t j = range.first; j < range.one_past_last; j += 1){
            if (list->jump_count >= list->jump_max){
                double_jump_max(general, list);
            }
            uint32_t flags = 0;
            if (jumps.jumps[j].is_sub_error){
                flags |= JumpFlag_IsSubJump;
            }
            list->jumps[list->jump_count] = make_sticky_jump_source(jumps.jumps[j].list_line, flags);
            ++list->jump_count;
        }
    }
    end_temp_memory(temp);
}
#endif

static void
free_marker_list(General_Memory *general, Marker_List list){
    general_memory_free(general, list.dst);
    general_memory_free(general, list.jumps);
}

static void
delete_marker_list(Marker_List_Node *node){
    zdll_remove(marker_list_first, marker_list_last, node);
}

static void
delete_marker_list(Marker_List *list){
    delete_marker_list(CastFromMember(Marker_List_Node, list, list));
}

static Marker_List*
make_new_marker_list_for_buffer(General_Memory *general, int32_t buffer_id){
    Marker_List_Node *new_node = gen_array(general, Marker_List_Node, 1);
    zdll_push_back(marker_list_first, marker_list_last, new_node);
    new_node->buffer_id = buffer_id;
    memset(&new_node->list, 0, sizeof(new_node->list));
    Marker_List *result = &new_node->list;
    return(result);
}

static Marker_List*
get_marker_list_for_buffer(General_Memory *general, int32_t buffer_id){
    Marker_List *result = 0;
    for (Marker_List_Node *node = marker_list_first;
         node != 0;
         node = node->next){
        if (buffer_id == node->buffer_id){
            result = &node->list;
            break;
        }
    }
    return(result);
}

static Marker_List*
get_or_make_list_for_buffer(Application_Links *app, Partition *part, General_Memory *general, int32_t buffer_id){
    Marker_List *result = get_marker_list_for_buffer(general, buffer_id);
    if (result != 0){
        Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
        // TODO(allen): When buffers get an "edit sequence number" use that instead.
        if (result->previous_size != buffer.size){
            delete_marker_list(result);
            result = 0;
        }
    }
    if (result == 0){
        result = make_new_marker_list_for_buffer(general, buffer_id);
        init_marker_list(app, part, general, buffer_id, result);
    }
    return(result);
}

static bool32
get_jump_from_list(Application_Links *app, Marker_List *list, int32_t index, ID_Pos_Jump_Location *location){
    bool32 result = false;
    if (index >= 0 && index < list->jump_count){
        uint32_t handle_index = binary_search(&list->dst->first_jump_index, sizeof(*list->dst), list->dst_count, index);
        Sticky_Jump_Destination_Array destinations = list->dst[handle_index];
        uint32_t marker_index = index - destinations.first_jump_index;
        Buffer_Summary buffer = get_buffer_by_marker_handle(app, destinations.handle, AccessAll);
        if (buffer.exists){
            Marker marker;
            buffer_get_markers(app, &buffer, destinations.handle, marker_index, 1, &marker);
            location->buffer_id = buffer.buffer_id;
            location->pos = marker.pos;
            result = true;
        }
    }
    return(result);
}

static int32_t
get_index_exact_from_list(Marker_List *list, int32_t line){
    int32_t result = -1;
    uint32_t jump_index = binary_search(&list->jumps->line_number, sizeof(*list->jumps), list->jump_count, line);
    if (list->jumps[jump_index].line_number == (uint32_t)line){
        result = jump_index;
    }
    return(result);
}

static int32_t
get_index_nearest_from_list(Marker_List *list, int32_t line){
    int32_t result = binary_search(&list->jumps->line_number, sizeof(*list->jumps), list->jump_count, line);
    return(result);
}

static int32_t
get_line_from_list(Marker_List *list, int32_t index){
    int32_t result = 0;
    if (0 <= index && index < list->jump_count){
        result = list->jumps[index].line_number;
    }
    return(result);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor_sticky)
CUSTOM_DOC("If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in another view and changes the active panel to the view containing the jump.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    View_Summary view = get_active_view(app, AccessProtected);
    Marker_List *list = get_or_make_list_for_buffer(app, part, general, view.buffer_id);
    
    int32_t list_index = get_index_exact_from_list(list, view.cursor.line);
    
    if (list_index >= 0){
        ID_Pos_Jump_Location location = {0};
        if (get_jump_from_list(app, list, list_index, &location)){
            Buffer_Summary buffer = {0};
            if (get_jump_buffer(app, &buffer, &location)){
                change_active_panel(app);
                View_Summary target_view = get_active_view(app, AccessAll);
                switch_to_existing_view(app, &target_view, &buffer);
                jump_to_location(app, &target_view, &buffer, location);
            }
        }
    }
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(goto_jump_at_cursor_same_panel_sticky)
CUSTOM_DOC("If the cursor is found to be on a jump location, parses the jump location and brings up the file and position in this view, losing the compilation output or jump list.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    View_Summary view = get_active_view(app, AccessProtected);
    Marker_List *list = get_or_make_list_for_buffer(app, part, general, view.buffer_id);
    int32_t list_index = get_index_exact_from_list(list, view.cursor.line);
    
    if (list_index >= 0){
        ID_Pos_Jump_Location location = {0};
        if (get_jump_from_list(app, list, list_index, &location)){
            Buffer_Summary buffer = {0};
            if (get_jump_buffer(app, &buffer, &location)){
                View_Summary target_view = view;
                jump_to_location(app, &target_view, &buffer, location);
            }
        }
    }
    
    end_temp_memory(temp);
}

static void
goto_jump_in_order(Application_Links *app, Marker_List *list, View_Summary *jump_view, ID_Pos_Jump_Location location){
    Buffer_Summary buffer = {0};
    if (get_jump_buffer(app, &buffer, &location)){
        View_Summary target_view = get_active_view(app, AccessAll);
        if (target_view.view_id == jump_view->view_id){
            change_active_panel(app);
            target_view = get_active_view(app, AccessAll);
        }
        switch_to_existing_view(app, &target_view, &buffer);
        jump_to_location(app, &target_view, &buffer, location);
        prev_location.buffer_id = location.buffer_id;
        prev_location.line = location.pos;
        prev_location.column = 0;
    }
}

static bool32
jump_is_repeat(ID_Line_Column_Jump_Location prev, ID_Pos_Jump_Location location){
    bool32 skip = false;
    // NOTE(allen): This looks wrong, but it is correct.  The prev_location is a line column type
    // because that is how the old-style direct jumps worked, and they are still supported.  All code paths
    // in the sticky jump system treat line as the field for pos and ignore column.  When the time has
    // passed and the direct jump legacy system is gone then this can be corrected.
    if (prev.buffer_id == location.buffer_id && prev.line == location.pos){
        skip = true;
    }
    return(skip);
}

static void
goto_next_filtered_jump(Application_Links *app, Marker_List *list, View_Summary *jump_view, int32_t list_index, int32_t direction, bool32 skip_repeats, bool32 skip_sub_errors){
    Assert(direction == 1 || direction == -1);
    
    while (list_index >= 0 && list_index < list->jump_count){
        ID_Pos_Jump_Location location = {0};
        if (get_jump_from_list(app, list, list_index, &location)){
            bool32 skip_this = false;
            if (skip_repeats && jump_is_repeat(prev_location, location)){
                skip_this = true;
            }
            else if (skip_sub_errors && (list->jumps[list_index].flags & JumpFlag_IsSubJump)){
                skip_this = true;
            }
            
            if (!skip_this){
                goto_jump_in_order(app, list, jump_view, location);
                int32_t updated_line = get_line_from_list(list, list_index);
                view_set_cursor(app, jump_view, seek_line_char(updated_line, 1), true);
                break;
            }
        }
        
        list_index += direction;
    }
}

static Locked_Jump_State
get_locked_jump_state(Application_Links *app, Partition *part, General_Memory *general){
    Locked_Jump_State result = {0};
    result.view = get_view_for_locked_jump_buffer(app);
    if (result.view.exists){
        result.list = get_or_make_list_for_buffer(app, part, general, result.view.buffer_id);
        result.list_index = get_index_nearest_from_list(result.list, result.view.cursor.line);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(goto_next_jump_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, skipping sub jump locations.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, part, general);
    if (jump_state.view.exists){
        int32_t line = get_line_from_list(jump_state.list, jump_state.list_index);
        if (line <= jump_state.view.cursor.line){
            ++jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, &jump_state.view, jump_state.list_index, 1, true, true);
    }
}

CUSTOM_COMMAND_SIG(goto_prev_jump_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, skipping sub jump locations."){
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, part, general);
    if (jump_state.view.exists){
        if (jump_state.list_index > 0){
            --jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, &jump_state.view, jump_state.list_index, -1, true, true);
    }
}

CUSTOM_COMMAND_SIG(goto_next_jump_no_skips_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the next jump in the buffer, and does not skip sub jump locations.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, part, general);
    if (jump_state.view.exists){
        int32_t line = get_line_from_list(jump_state.list, jump_state.list_index);
        if (line <= jump_state.view.cursor.line){
            ++jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, &jump_state.view, jump_state.list_index, 1, true, false);
    }
}

CUSTOM_COMMAND_SIG(goto_prev_jump_no_skips_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the previous jump in the buffer, and does not skip sub jump locations.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, part, general);
    if (jump_state.view.exists){
        if (jump_state.list_index > 0){
            --jump_state.list_index;
        }
        goto_next_filtered_jump(app, jump_state.list, &jump_state.view, jump_state.list_index, -1, true, false);
    }
}

CUSTOM_COMMAND_SIG(goto_first_jump_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the first jump in the buffer.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, part, general);
    if (jump_state.view.exists){
        int32_t list_index = 0;
        ID_Pos_Jump_Location location = {0};
        if (get_jump_from_list(app, jump_state.list, list_index, &location)){
            goto_jump_in_order(app, jump_state.list, &jump_state.view, location);
            int32_t updated_line = get_line_from_list(jump_state.list, list_index);
            view_set_cursor(app, &jump_state.view, seek_line_char(updated_line, 1), true);
        }
    }
}

CUSTOM_COMMAND_SIG(goto_first_jump_same_panel_sticky)
CUSTOM_DOC("If a buffer containing jump locations has been locked in, goes to the first jump in the buffer and views the buffer in the panel where the jump list was.")
{
    General_Memory *general = &global_general;
    Partition *part = &global_part;
    
    Locked_Jump_State jump_state = get_locked_jump_state(app, part, general);
    if (jump_state.view.exists){
        int32_t list_index = 0;
        ID_Pos_Jump_Location location = {0};
        if (get_jump_from_list(app, jump_state.list, list_index, &location)){
            Buffer_Summary buffer = {0};
            if (get_jump_buffer(app, &buffer, &location)){
                jump_to_location(app, &jump_state.view, &buffer, location);
            }
        }
    }
}

//
// Insert Newline or Tigger Jump on Read Only Buffer
//

CUSTOM_COMMAND_SIG(newline_or_goto_position_sticky)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    if (buffer.lock_flags & AccessProtected){
        goto_jump_at_cursor_sticky(app);
        lock_jump_buffer(buffer);
    }
    else{
        write_character(app);
    }
}

CUSTOM_COMMAND_SIG(newline_or_goto_position_same_panel_sticky)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    if (buffer.lock_flags & AccessProtected){
        goto_jump_at_cursor_same_panel_sticky(app);
        lock_jump_buffer(buffer);
    }
    else{
        write_character(app);
    }
}

//
// End File Hook
//

OPEN_FILE_HOOK_SIG(default_end_file);
OPEN_FILE_HOOK_SIG(end_file_close_jump_list){
    General_Memory *general = &global_general;
    Marker_List *list = get_marker_list_for_buffer(general, buffer_id);
    if (list != 0){
        free_marker_list(general, *list);
        delete_marker_list(list);
    }
    default_end_file(app, buffer_id);
    return(0);
}

// BOTTOM

