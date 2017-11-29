/*
4coder_jump_sticky.cpp - Commands and helpers for parsing jump locations from 
compiler errors, sticking markers on jump locations, and jumping to them.

TYPE: 'drop-in-command-pack'
*/

// TOP

//#if !defined(FCODER_STICKY_JUMP) && !defined(FCODER_JUMP_COMMANDS)
//#define FCODER_STICKY_JUMP
//#define FCODER_JUMP_COMMANDS

#if !defined(FCODER_STICKY_JUMP)
#define FCODER_STICKY_JUMP

#include "4coder_default_framework.h"
#include "4coder_helper/4coder_long_seek.h"
#include "4coder_helper/4coder_helper.h"

#include "4coder_lib/4coder_mem.h"
#include "4coder_jumping.h"

static uint32_t
binary_search(uint32_t *array, uint32_t count, uint32_t x){
    uint32_t i = 0;
    uint32_t first = 0;
    uint32_t last = count;
    if (first < last){
        for (;;){
            i = (first + last)/2;
            if (array[i] < x){
                first = i;
            }
            else if (array[i] > x){
                last = i;
            }
            else{ // NOTE(allen): array[i] == x
                break;
            }
            if (first+1 >= last){
                i = first;
                break;
            }
        }
    }
    return(i);
}

enum Jump_Location_Flag{
    JumpFlag_IsSubJump = 0x1,
};

struct Marker_List{
    uint32_t *handle_starts;
    Marker_Handle *handles;
    uint32_t *jump_line_numbers;
    uint32_t *jump_flags;
    uint32_t handle_count;
    uint32_t handle_max;
    int32_t jump_max;
    int32_t jump_count;
};

static void
double_jump_max(General_Memory *general, Marker_List *list){
    uint32_t new_jump_max = list->jump_max*2;
    list->jump_line_numbers = gen_realloc_array(general, uint32_t, list->jump_line_numbers, list->jump_max, new_jump_max);
    list->jump_flags = gen_realloc_array(general, uint32_t, list->jump_flags, list->jump_max, new_jump_max);
    list->jump_max = new_jump_max;
}

static void
double_handle_max(General_Memory *general, Marker_List *list){
    uint32_t new_handle_max = list->handle_max*2;
    list->handle_starts = gen_realloc_array(general, uint32_t, list->handle_starts, list->handle_max, new_handle_max);
    list->handles = gen_realloc_array(general, Marker_Handle, list->handles, list->handle_max, new_handle_max);
    list->handle_max = new_handle_max;
}

// TODO(allen): what to do when a push returns 0?
static Marker_List
make_marker_list(Application_Links *app, Partition *part, General_Memory *general, int32_t buffer_id){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    
    Marker_List list = {0};
    
    Temp_Memory temp = begin_temp_memory(part);
    ID_Based_Jump_Location *location_list = (ID_Based_Jump_Location*)partition_current(part);
    uint32_t location_count = 0;
    
    list.handle_max = 64;
    list.handle_starts = gen_array(general, uint32_t, list.handle_max);
    list.handles = gen_array(general, Marker_Handle, list.handle_max);
    
    list.jump_max = 64;
    list.jump_line_numbers = gen_array(general, uint32_t, list.jump_max);
    list.jump_flags = gen_array(general, uint32_t, list.jump_max);
    
    uint32_t prev_jump_count = 0;
    for (int32_t this_jump_line = 1;; ++this_jump_line){
        bool32 output_jump = false;
        Name_Based_Jump_Location location = {0};
        bool32 is_sub_error = false;
        
        Temp_Memory temp_name = begin_temp_memory(part);
        String line_str = {0};
        if (read_line(app, part, &buffer, this_jump_line, &line_str)){
            int32_t colon_index = 0;
            if (parse_jump_location(line_str, &location, &colon_index, &is_sub_error)){
                output_jump = true;
            }
            else{
                end_temp_memory(temp_name);
            }
        }
        else{
            end_temp_memory(temp_name);
            break;
        }
        
        if (output_jump){
            Buffer_Summary jump_buffer = {0};
            if (open_file(app, &jump_buffer, location.file.str, location.file.size, false, true)){
                ID_Based_Jump_Location id_location = {0};
                end_temp_memory(temp_name);
                id_location.buffer_id = jump_buffer.buffer_id;
                id_location.line = location.line;
                id_location.column = location.column;
                
                if (id_location.buffer_id != 0){
                    if (location_count > 0){
                        ID_Based_Jump_Location *prev_parsed_loc = &location_list[location_count-1];
                        if (prev_parsed_loc->buffer_id != id_location.buffer_id){
                            Buffer_Summary location_buffer = get_buffer(app, prev_parsed_loc->buffer_id, AccessAll);
                            
                            if (location_buffer.exists){
                                if (list.handle_count >= list.handle_max){
                                    double_handle_max(general, &list);
                                }
                                
                                Marker_Handle new_handle = buffer_add_markers(app, &location_buffer, location_count);
                                
                                list.handle_starts[list.handle_count] = prev_jump_count;
                                list.handles[list.handle_count] = new_handle;
                                ++list.handle_count;
                                
                                prev_jump_count = list.jump_count;
                                
                                Marker *markers = push_array(part, Marker, location_count);
                                for (uint32_t i = 0; i < location_count; ++i){
                                    ID_Based_Jump_Location *write_loc = &location_list[i];
                                    Partial_Cursor cursor = {0};
                                    Buffer_Seek seek = seek_line_char(write_loc->line, write_loc->column);
                                    if (buffer_compute_cursor(app, &location_buffer, seek, &cursor)){
                                        markers[i].pos = cursor.pos;
                                        markers[i].lean_right = false;
                                    }
                                }
                                
                                buffer_set_markers(app, &location_buffer, new_handle, 0, location_count, markers);
                                
                                location_count = 0;
                                reset_temp_memory(temp);
                            }
                        }
                    }
                    
                    ID_Based_Jump_Location *new_id_location = push_struct(part, ID_Based_Jump_Location);
                    *new_id_location = id_location;
                    ++location_count;
                    
                    if (list.jump_count >= list.jump_max){
                        double_jump_max(general, &list);
                    }
                    list.jump_line_numbers[list.jump_count] = this_jump_line;
                    
                    uint32_t flags = 0;
                    if (is_sub_error){
                        flags |= JumpFlag_IsSubJump;
                    }
                    
                    list.jump_flags[list.jump_count] = flags;
                    ++list.jump_count;
                }
            }
            else{
                end_temp_memory(temp_name);
            }
        }
    }
    
    if (location_count > 0){
        ID_Based_Jump_Location *prev_parsed_loc = &location_list[location_count-1];
        Buffer_Summary location_buffer = get_buffer(app, prev_parsed_loc->buffer_id, AccessAll);
        
        if (list.handle_count >= list.handle_max){
            double_handle_max(general, &list);
        }
        
        Marker_Handle new_handle = buffer_add_markers(app, &location_buffer, location_count);
        
        list.handle_starts[list.handle_count] = prev_jump_count;
        list.handles[list.handle_count] = new_handle;
        ++list.handle_count;
        
        prev_jump_count = list.jump_count;
        
        Marker *markers = push_array(part, Marker, location_count);
        for (uint32_t i = 0; i < location_count; ++i){
            ID_Based_Jump_Location *location = &location_list[i];
            Partial_Cursor cursor = {0};
            Buffer_Seek seek = seek_line_char(location->line, location->column);
            if (buffer_compute_cursor(app, &location_buffer, seek, &cursor)){
                markers[i].pos = cursor.pos;
                markers[i].lean_right = false;
            }
        }
        
        buffer_set_markers(app, &location_buffer, new_handle, 0, location_count, markers);
        
        location_count = 0;
        reset_temp_memory(temp);
    }
    
    end_temp_memory(temp);
    
    return(list);
}

static void
free_marker_list(General_Memory *general, Marker_List list){
    general_memory_free(general, list.handle_starts);
    general_memory_free(general, list.handles);
    general_memory_free(general, list.jump_line_numbers);
}

struct Marker_List_Slot{
    Marker_List list;
    int32_t buffer_id;
};

static Marker_List_Slot *marker_list_slots = 0;
static uint32_t marker_list_slot_count = 0;
static uint32_t marker_list_slot_max = 0;

static Marker_List*
set_marker_list_for_buffer(int32_t buffer_id, Marker_List *list){
    Marker_List *result = 0;
    General_Memory *general = &global_general;
    
    bool32 found_slot = false;
    for (uint32_t i = 0; i < marker_list_slot_count; ++i){
        if (buffer_id == marker_list_slots[i].buffer_id){
            if (list != 0){
                marker_list_slots[i].list = *list;
                result = &marker_list_slots[i].list;
            }
            else{
                void *dst = marker_list_slots+i;
                void *src = marker_list_slots+i+1;
                size_t amount = (marker_list_slot_count-i-1)*sizeof(Marker_List_Slot);
                memmove(dst, src, amount);
                --marker_list_slot_count;
            }
            found_slot = true;
            break;
        }
    }
    
    if (!found_slot && list != 0){
        if (marker_list_slot_count >= marker_list_slot_max){
            if (marker_list_slots == 0){
                uint32_t new_max = 64;
                marker_list_slots = gen_array(general, Marker_List_Slot, new_max);
                marker_list_slot_max = new_max;
            }
            else{
                uint32_t new_max = 2*marker_list_slot_max;
                marker_list_slots = gen_realloc_array(general, Marker_List_Slot, marker_list_slots, marker_list_slot_count, new_max);
                marker_list_slot_max = new_max;
            }
        }
        
        marker_list_slots[marker_list_slot_count].buffer_id = buffer_id;
        marker_list_slots[marker_list_slot_count].list = *list;
        result = &marker_list_slots[marker_list_slot_count].list;
        ++marker_list_slot_count;
    }
    return(result);
}

static Marker_List*
get_marker_list_for_buffer(int32_t buffer_id){
    Marker_List *result = 0;
    for (uint32_t i = 0; i < marker_list_slot_count; ++i){
        if (buffer_id == marker_list_slots[i].buffer_id){
            result = &marker_list_slots[i].list;
            break;
        }
    }
    return(result);
}

static Marker_List*
get_or_make_list_for_buffer(Application_Links *app, Partition *part, General_Memory *general, int32_t buffer_id){
    Marker_List *result = get_marker_list_for_buffer(buffer_id);
    if (result == 0){
        Marker_List new_list = make_marker_list(app, part, general, buffer_id);
        result = set_marker_list_for_buffer(buffer_id, &new_list);
    }
    return(result);
}

static bool32
get_jump_from_list(Application_Links *app, Marker_List *list, int32_t index, ID_Pos_Jump_Location *location){
    bool32 result = false;
    if (index >= 0 && index < list->jump_count){
        uint32_t handle_index = binary_search(list->handle_starts, list->handle_count, index);
        uint32_t handle_start = list->handle_starts[handle_index];
        uint32_t marker_index = index - handle_start;
        Marker_Handle handle = list->handles[handle_index];
        Buffer_Summary buffer = get_buffer_by_marker_handle(app, handle, AccessAll);
        if (buffer.exists){
            Marker marker;
            buffer_get_markers(app, &buffer, handle, marker_index, 1, &marker);
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
    uint32_t jump_index = binary_search(list->jump_line_numbers, list->jump_count, line);
    if (list->jump_line_numbers[jump_index] == (uint32_t)line){
        result = jump_index;
    }
    return(result);
}

static int32_t
get_index_nearest_from_list(Marker_List *list, int32_t line){
    int32_t result = binary_search(list->jump_line_numbers, list->jump_count, line);
    return(result);
}

static int32_t
get_line_from_list(Marker_List *list, int32_t index){
    int32_t result = 0;
    if (index >= 0 && index < list->jump_count){
        result = list->jump_line_numbers[index];
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
skip_this_jump(ID_Based_Jump_Location prev, ID_Pos_Jump_Location location){
    bool32 skip = false;
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
            if (skip_repeats && skip_this_jump(prev_location, location)){
                skip_this = true;
            }
            else if (skip_sub_errors && (list->jump_flags[list_index] & JumpFlag_IsSubJump)){
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

struct Locked_Jump_State{
    View_Summary view;
    Marker_List *list;
    int32_t list_index;
};

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
    Marker_List *list = get_marker_list_for_buffer(buffer_id);
    if (list != 0){
        free_marker_list(&global_general, *list);
        set_marker_list_for_buffer(buffer_id, 0);
    }
    
    default_end_file(app, buffer_id);
    
    return(0);
}

#endif

// BOTTOM

