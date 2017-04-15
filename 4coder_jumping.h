/*
4coder_jumping.h - Routines commonly used when writing code to jump to locations and seek through jump lists.

TYPE: 'helper-routines'
*/

// TOP

#if !defined(FCODER_JUMPING)
#define FCODER_JUMPING

static bool32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, Name_Based_Jump_Location *location){
    bool32 result = open_file(app, buffer, location->file.str, location->file.size, false, true);
    return(result);
}

static bool32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, ID_Pos_Jump_Location *location){
    *buffer = get_buffer(app, location->buffer_id, AccessAll);
    bool32 result = false;
    if (buffer->exists){
        result = true;
    }
    return(result);
}

static void
switch_to_existing_view(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    if (!(view->exists && view->buffer_id == buffer->buffer_id)){
        View_Summary existing_view = get_first_view_with_buffer(app, buffer->buffer_id);
        if (existing_view.exists){
            *view = existing_view;
        }
    }
}

static void
set_view_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, Buffer_Seek seek){
    if (view->buffer_id != buffer->buffer_id){
        view_set_buffer(app, view, buffer->buffer_id, 0);
    }
    view_set_cursor(app, view, seek, true);
}

static void
jump_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, Name_Based_Jump_Location location){
    set_active_view(app, view);
    set_view_to_location(app, view, buffer, seek_line_char(location.line, location.column));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

static void
jump_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, ID_Pos_Jump_Location location){
    set_active_view(app, view);
    set_view_to_location(app, view, buffer, seek_pos(location.pos));
    if (auto_center_after_jumps){
        center_view(app);
    }
}

//
// Error Jumping
//

static bool32
seek_next_jump_in_buffer(Application_Links *app, Partition *part, int32_t buffer_id, int32_t first_line, bool32 skip_sub_errors, int32_t direction, int32_t *line_out, int32_t *colon_index_out, Name_Based_Jump_Location *location_out){
    
    Assert(direction == 1 || direction == -1);
    
    bool32 result = false;
    int32_t line = first_line;
    String line_str = {0};
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    for (;;){
        if (read_line(app, part, &buffer, line, &line_str)){
            if (parse_jump_location(line_str, location_out, skip_sub_errors, colon_index_out)){
                result = true;
                break;
            }
            line += direction;
        }
        else{
            break;
        }
    }
    
    if (line < 0){
        line = 0;
    }
    
    *line_out = line;
    
    return(result);
}

static ID_Based_Jump_Location
convert_name_based_to_id_based(Application_Links *app, Name_Based_Jump_Location loc){
    ID_Based_Jump_Location result = {0};
    Buffer_Summary buffer = get_buffer_by_name(app, loc.file.str, loc.file.size, AccessAll);
    
    if (buffer.exists){
        result.buffer_id = buffer.buffer_id;
        result.line = loc.line;
        result.column = loc.column;
    }
    
    return(result);
}

static int32_t
seek_next_jump_in_view(Application_Links *app, Partition *part, View_Summary *view, int32_t skip_sub_errors, int32_t direction, int32_t *line_out, int32_t *colon_index_out, Name_Based_Jump_Location *location_out){
    int32_t result = false;
    
    Name_Based_Jump_Location location = {0};
    int32_t line = view->cursor.line;
    int32_t colon_index = 0;
    if (seek_next_jump_in_buffer(app, part, view->buffer_id, line+direction, skip_sub_errors, direction, &line, &colon_index, &location)){
        result = true;
        *line_out = line;
        *colon_index_out = colon_index;
        *location_out = location;
    }
    
    return(result);
}

static bool32
skip_this_jump(ID_Based_Jump_Location prev, ID_Based_Jump_Location jump){
    bool32 result = false;
    if (prev.buffer_id != 0 && prev.buffer_id == jump.buffer_id && prev.line == jump.line && prev.column <= jump.column){
        result = true;
    }
    return(result);
}

static bool32
advance_cursor_in_jump_view(Application_Links *app, Partition *part, View_Summary *view, int32_t skip_repeats, int32_t skip_sub_error, int32_t direction, Name_Based_Jump_Location *location_out){
    bool32 result = true;
    
    Name_Based_Jump_Location location = {0};
    ID_Based_Jump_Location jump = {0};
    int32_t line = 0, colon_index = 0;
    
    do{
        Temp_Memory temp = begin_temp_memory(part);
        if (seek_next_jump_in_view(app, part, view, skip_sub_error, direction, &line, &colon_index, &location)){
            jump = convert_name_based_to_id_based(app, location);
            view_set_cursor(app, view, seek_line_char(line, colon_index+1), true);
            result = true;
        }
        else{
            jump.buffer_id = 0;
            result = false;
        }
        end_temp_memory(temp);
    }while(skip_repeats && skip_this_jump(prev_location, jump));
    
    if (result){
        *location_out = location;
        view_set_cursor(app, view, seek_line_char(line, colon_index+1), true);
    }
    
    prev_location = jump;
    
    return(result);
}

static bool32
seek_jump(Application_Links *app, Partition *part, bool32 skip_repeats, bool32 skip_sub_errors, int32_t direction){
    bool32 result = false;
    
    View_Summary view = get_view_for_locked_jump_buffer(app);
    if (view.exists){
        Name_Based_Jump_Location location = {0};
        if (advance_cursor_in_jump_view(app, &global_part, &view, skip_repeats, skip_sub_errors, direction, &location)){
            
            Buffer_Summary buffer = {0};
            if (get_jump_buffer(app, &buffer, &location)){
                View_Summary target_view = get_active_view(app, AccessAll);
                if (target_view.view_id == view.view_id){
                    change_active_panel(app);
                    target_view = get_active_view(app, AccessAll);
                }
                switch_to_existing_view(app, &target_view, &buffer);
                jump_to_location(app, &target_view, &buffer, location);
                result = true;
            }
        }
    }
    
    return(result);
}

#endif

// BOTTOM

