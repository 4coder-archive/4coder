/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * File layer for 4coder
 *
 */

// TOP

internal String_Const_u8
string_from_file_name(Editing_File_Name *name){
    return(SCu8(name->name_space, name->name_size));
}

////////////////////////////////

internal void
file_edit_positions_set_cursor(File_Edit_Positions *edit_pos, i64 pos){
    edit_pos->cursor_pos = pos;
    edit_pos->last_set_type = EditPos_CursorSet;
}

internal void
file_edit_positions_set_scroll(File_Edit_Positions *edit_pos, Buffer_Scroll scroll){
    edit_pos->scroll = scroll;
    edit_pos->last_set_type = EditPos_ScrollSet;
}

internal void
file_edit_positions_push(Editing_File *file, File_Edit_Positions edit_pos){
    if (file->state.edit_pos_stack_top + 1 < ArrayCount(file->state.edit_pos_stack)){
        file->state.edit_pos_stack_top += 1;
        file->state.edit_pos_stack[file->state.edit_pos_stack_top] = edit_pos;
    }
}

internal File_Edit_Positions
file_edit_positions_pop(Editing_File *file){
    File_Edit_Positions edit_pos = {};
    if (file->state.edit_pos_stack_top >= 0){
        edit_pos = file->state.edit_pos_stack[file->state.edit_pos_stack_top];
        file->state.edit_pos_stack_top -= 1;
    }
    else{
        edit_pos = file->state.edit_pos_most_recent;
    }
    return(edit_pos);
}

////////////////////////////////

internal Face*
file_get_face(Models *models, Editing_File *file){
    return(font_set_face_from_id(&models->font_set, file->settings.face_id));
}

internal u32
file_get_access_flags(Editing_File *file){
    u32 flags = 0;
    if (file->settings.read_only){
        flags |= AccessProtected;
    }
    return(flags);
}

internal b32
file_needs_save(Editing_File *file){
    b32 result = false;
    if (HasFlag(file->state.dirty, DirtyState_UnsavedChanges)){
        result = true;
    }
    return(result);
}

internal b32
file_can_save(Editing_File *file){
    b32 result = false;
    if (HasFlag(file->state.dirty, DirtyState_UnsavedChanges) ||
        HasFlag(file->state.dirty, DirtyState_UnloadedChanges)){
        result = true;
    }
    return(result);
}

internal b32
file_is_ready(Editing_File *file){
    b32 result = false;
    if (file != 0 && file->is_loading == 0){
        result = true;
    }
    return(result);
}

internal void
file_set_unimportant(Editing_File *file, b32 val){
    if (val){
        file->state.dirty = DirtyState_UpToDate;
    }
    file->settings.unimportant = (b8)(val);
}

internal void
file_set_to_loading(Editing_File *file){
    block_zero_struct(&file->state);
    block_zero_struct(&file->settings);
    file->is_loading = true;
}

internal void
file_add_dirty_flag(Editing_File *file, Dirty_State state){
    if (!file->settings.unimportant){
        file->state.dirty |= state;
    }
    else{
        file->state.dirty = DirtyState_UpToDate;
    }
}

internal void
file_clear_dirty_flags(Editing_File *file){
    file->state.dirty = DirtyState_UpToDate;
}

////////////////////////////////

internal void
file_name_terminate(Editing_File_Name *name){
    umem size = name->name_size;
    size = clamp_top(size, sizeof(name->name_space) - 1);
    name->name_space[size] = 0;
    name->name_size = size;
}

////////////////////////////////

// TODO(allen): file_name should be String_Const_u8
internal b32
save_file_to_name(Models *models, Editing_File *file, u8 *file_name){
    b32 result = false;
    b32 using_actual_file_name = false;
    
    if (file_name == 0){
        file_name_terminate(&file->canon);
        file_name = file->canon.name_space;
        using_actual_file_name = true;
    }
    
    if (file_name != 0){
        if (models->hook_save_file != 0){
            models->hook_save_file(&models->app_links, file->id);
        }
        
        Gap_Buffer *buffer = &file->state.buffer;
        b32 dos_write_mode = file->settings.dos_write_mode;
        
        Scratch_Block scratch(models->tctx, Scratch_Share);
        
        if (!using_actual_file_name){
            String_Const_u8 s_file_name = SCu8(file_name);
            String_Const_u8 canonical_file_name = system_get_canonical(scratch, s_file_name);
            if (string_match(canonical_file_name, string_from_file_name(&file->canon))){
                using_actual_file_name = true;
            }
        }
        
        String_Const_u8 saveable_string = buffer_stringify(scratch, buffer, Ii64(0, buffer_size(buffer)));
        
        File_Attributes new_attributes = system_save_file(scratch, (char*)file_name, saveable_string);
        if (new_attributes.last_write_time > 0){
            if (using_actual_file_name){
                file->state.ignore_behind_os = 1;
            }
            file->attributes = new_attributes;
        }
        LogEventF(log_string(M), scratch, file->id, 0, system_thread_get_id(),
                  "save file [last_write_time=0x%llx]", new_attributes.last_write_time);
        
        file_clear_dirty_flags(file);
    }
    
    return(result);
}

internal b32
save_file(Models *models, Editing_File *file){
    return(save_file_to_name(models, file, 0));
}

////////////////////////////////

internal Buffer_Cursor
file_compute_cursor(Editing_File *file, Buffer_Seek seek){
    Buffer_Cursor result = {};
    switch (seek.type){
        case buffer_seek_pos:
        {
            result = buffer_cursor_from_pos(&file->state.buffer, seek.pos);
        }break;
        case buffer_seek_line_col:
        {
            result = buffer_cursor_from_line_col(&file->state.buffer, seek.line, seek.col);
        }break;
    }
    return(result);
}

////////////////////////////////

internal void
file_create_from_string(Models *models, Editing_File *file, String_Const_u8 val, File_Attributes attributes){
    Thread_Context *tctx = models->tctx;
    Scratch_Block scratch(tctx, Scratch_Share);
    
    Base_Allocator *allocator = tctx->allocator;
    block_zero_struct(&file->state);
    buffer_init(&file->state.buffer, val.str, val.size, allocator);
    
    if (buffer_size(&file->state.buffer) < (i64)val.size){
        file->settings.dos_write_mode = true;
    }
    file_clear_dirty_flags(file);
    file->attributes = attributes;
    
    file->settings.face_id = models->global_face_id;
    
    buffer_measure_starts(scratch, &file->state.buffer);
    
    file->lifetime_object = lifetime_alloc_object(&models->lifetime_allocator, DynamicWorkspace_Buffer, file);
    history_init(models, &file->state.history);
    
    file->state.cached_layouts_arena = make_arena(allocator);
    file->state.line_layout_table = make_table_Data_u64(allocator, 500);
    
    file->settings.is_initialized = true;
    
    {
        Temp_Memory temp = begin_temp(scratch);
        String_Const_u8 name = SCu8(file->unique_name.name_space, file->unique_name.name_size);
        name = string_escape(scratch, name);
        LogEventF(log_string(M), scratch, file->id, 0, system_thread_get_id(),
                  "init file [lwt=0x%llx] [name=\"%.*s\"]",
                  attributes.last_write_time, string_expand(name));
        end_temp(temp);
    }
    
    ////////////////////////////////
    
    Buffer_Hook_Function *hook_open_file = models->hook_open_file;
    if (hook_open_file != 0){
        hook_open_file(&models->app_links, file->id);
    }
}

internal void
file_free(Models *models, Editing_File *file){
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    Working_Set *working_set = &models->working_set;
    
    lifetime_free_object(lifetime_allocator, file->lifetime_object);
    
    Gap_Buffer *buffer = &file->state.buffer;
    if (buffer->data){
        base_free(buffer->allocator, buffer->data);
        base_free(buffer->allocator, buffer->line_starts);
    }
    
    history_free(models, &file->state.history);
    
    linalloc_clear(&file->state.cached_layouts_arena);
    table_free(&file->state.line_layout_table);
}

////////////////////////////////

internal i32
file_get_current_record_index(Editing_File *file){
    return(file->state.current_record_index);
}

internal Managed_Scope
file_get_managed_scope(Editing_File *file){
    Managed_Scope scope = 0;
    if (file != 0){
        Assert(file->lifetime_object != 0);
        scope = (Managed_Scope)file->lifetime_object->workspace.scope_id;
    }
    return(scope);
}

////////////////////////////////

internal Buffer_Layout_Item_List
file_get_line_layout(Models *models, Editing_File *file, f32 width, Face *face, i64 line_number){
    Buffer_Layout_Item_List result = {};
    
    i64 line_count = buffer_line_count(&file->state.buffer);
    if (1 <= line_number && line_number <= line_count){
        Line_Layout_Key key = {};
        key.face_id = face->id;
        key.face_version_number = face->version_number;
        key.width = width;
        key.line_number = line_number;
        
        Data key_data = make_data_struct(&key);
        
        Buffer_Layout_Item_List *list = 0;
        
        Table_Lookup lookup = table_lookup(&file->state.line_layout_table, key_data);
        if (lookup.found_match){
            u64 val = 0;
            table_read(&file->state.line_layout_table, lookup, &val);
            list = (Buffer_Layout_Item_List*)IntAsPtr(val);
        }
        else{
            list = push_array(&file->state.cached_layouts_arena, Buffer_Layout_Item_List, 1);
            Interval_i64 line_range = buffer_get_pos_range_from_line_number(&file->state.buffer, line_number);
            *list = buffer_layout(models->tctx, &file->state.cached_layouts_arena,
                                  &file->state.buffer, line_range, face, width);
            key_data = push_data_copy(&file->state.cached_layouts_arena, key_data);
            table_insert(&file->state.line_layout_table, key_data, (u64)PtrAsInt(list));
        }
        block_copy_struct(&result, list);
    }
    
    return(result);
}

internal void
file_clear_layout_cache(Editing_File *file){
    linalloc_clear(&file->state.cached_layouts_arena);
    table_clear(&file->state.line_layout_table);
}

internal Line_Shift_Vertical
file_line_shift_y(Models *models, Editing_File *file, f32 width, Face *face, i64 line_number, f32 y_delta){
    Line_Shift_Vertical result = {};
    
    f32 line_y = 0.f;
    
    if (y_delta < 0.f){
        // NOTE(allen): Iterating upward
        b32 has_result = false;
        for (;;){
            if (line_y <= y_delta){
                has_result = true;
                result.line = line_number;
                result.y_delta = line_y;
                break;
            }
            line_number -= 1;
            if (line_number <= 0){
                line_number = 1;
                break;
            }
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number);
            line_y -= line.height;
        }
        if (!has_result){
            result.line = line_number;
            result.y_delta = line_y;
        }
    }
    else{
        // NOTE(allen): Iterating downward
        b32 has_result = false;
        i64 line_count = buffer_line_count(&file->state.buffer);
        for (;;line_number += 1){
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number);
            f32 next_y = line_y + line.height;
            if (y_delta < next_y){
                has_result = true;
                result.line = line_number;
                result.y_delta = line_y;
                break;
            }
            if (line_number >= line_count){
                break;
            }
            line_y = next_y;
        }
        if (!has_result){
            result.line = line_number;
            result.y_delta = line_y;
        }
    }
    
    return(result);
}

internal f32
file_line_y_difference(Models *models, Editing_File *file, f32 width, Face *face, i64 line_a, i64 line_b){
    f32 result = 0.f;
    if (line_a != line_b){
        Interval_i64 line_range = Ii64(line_a, line_b);
        for (i64 i = line_range.min; i < line_range.max; i += 1){
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, i);
            result += line.height;
        }
        if (line_a < line_b){
            result *= -1.f;
        }
    }
    return(result);
}

internal i64
file_pos_at_relative_xy(Models *models, Editing_File *file, f32 width, Face *face, i64 base_line, Vec2_f32 relative_xy){
    Line_Shift_Vertical shift = file_line_shift_y(models, file, width, face, base_line, relative_xy.y);
    relative_xy.y -= shift.y_delta;
    Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, shift.line);
    return(buffer_layout_nearest_pos_to_xy(line, relative_xy));
}

internal Vec2_f32
file_relative_xy_of_pos(Models *models, Editing_File *file, f32 width, Face *face, i64 base_line, i64 pos){
    i64 line_number = buffer_get_line_index(&file->state.buffer, pos) + 1;
    Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number);
    Vec2_f32 result = buffer_layout_xy_center_of_pos(line, pos);
    result.y += file_line_y_difference(models, file, width, face, line_number, base_line);
    return(result);
}

internal Buffer_Point
file_normalize_buffer_point(Models *models, Editing_File *file, f32 width, Face *face, Buffer_Point point){
    Line_Shift_Vertical shift = file_line_shift_y(models, file, width, face, point.line_number, point.pixel_shift.y);
    point.line_number = shift.line;
    point.pixel_shift.y -= shift.y_delta;
    point.pixel_shift.x = clamp_bot(0.f, point.pixel_shift.x);
    point.pixel_shift.y = clamp_bot(0.f, point.pixel_shift.y);
    return(point);
}

internal Vec2_f32
file_buffer_point_difference(Models *models, Editing_File *file, f32 width, Face *face, Buffer_Point a, Buffer_Point b){
    f32 y_difference = file_line_y_difference(models, file, width, face, a.line_number, b.line_number);
    Vec2_f32 result = a.pixel_shift - b.pixel_shift;
    result.y += y_difference;
    return(result);
}

internal Line_Shift_Character
file_line_shift_characters(Models *models, Editing_File *file, f32 width, Face *face, i64 line_number, i64 character_delta){
    Line_Shift_Character result = {};
    
    i64 line_character = 0;
    
    if (character_delta < 0){
        // NOTE(allen): Iterating upward
        b32 has_result = false;
        for (;;){
            if (line_character <= character_delta){
                has_result = true;
                result.line = line_number;
                result.character_delta = line_character;
                break;
            }
            line_number -= 1;
            if (line_number <= 0){
                line_number = 1;
                break;
            }
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number);
            line_character -= line.character_count;
        }
        if (!has_result){
            result.line = line_number;
            result.character_delta = line_character;
        }
    }
    else{
        // NOTE(allen): Iterating downward
        b32 has_result = false;
        i64 line_count = buffer_line_count(&file->state.buffer);
        for (;;line_number += 1){
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number);
            i64 next_character = line_character + line.character_count;
            if (character_delta < next_character){
                has_result = true;
                result.line = line_number;
                result.character_delta = line_character;
                break;
            }
            if (line_number >= line_count){
                break;
            }
            line_character = next_character;
        }
        if (!has_result){
            result.line = line_number;
            result.character_delta = line_character;
        }
    }
    
    return(result);
}

internal i64
file_line_character_difference(Models *models, Editing_File *file, f32 width, Face *face, i64 line_a, i64 line_b){
    i64 result = 0;
    if (line_a != line_b){
        Interval_i64 line_range = Ii64(line_a, line_b);
        for (i64 i = line_range.min; i < line_range.max; i += 1){
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, i);
            result += line.character_count;
        }
        if (line_a < line_b){
            result *= -1;
        }
    }
    return(result);
}

internal i64
file_pos_from_relative_character(Models *models, Editing_File *file, f32 width, Face *face, i64 base_line, i64 relative_character){
    Line_Shift_Character shift = file_line_shift_characters(models, file, width, face, base_line, relative_character);
    relative_character -= shift.character_delta;
    Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, shift.line);
    return(buffer_layout_get_pos_at_character(line, relative_character));
}

internal i64
file_relative_character_from_pos(Models *models, Editing_File *file, f32 width, Face *face, i64 base_line, i64 pos){
    i64 line_number = buffer_get_line_index(&file->state.buffer, pos) + 1;
    Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number);
    i64 result = buffer_layout_character_from_pos(line, pos);
    result += file_line_character_difference(models, file, width, face, line_number, base_line);
    return(result);
}

// BOTTOM

