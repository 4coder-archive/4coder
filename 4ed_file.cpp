/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * File layer for 4coder
 *
 */

// TOP

inline Buffer_Slot_ID
to_file_id(i32 id){
    Buffer_Slot_ID result;
    result.id = id;
    return(result);
}

////////////////////////////////

internal void
edit_pos_set_cursor(File_Edit_Positions *edit_pos, Full_Cursor cursor, b32 set_preferred_x, b32 unwrapped_lines){
    edit_pos->cursor = cursor;
    if (set_preferred_x){
        edit_pos->preferred_x = cursor.wrapped_x;
        if (unwrapped_lines){
            edit_pos->preferred_x = cursor.unwrapped_x;
        }
    }
    edit_pos->last_set_type = EditPos_CursorSet;
}

internal void
edit_pos_set_scroll(File_Edit_Positions *edit_pos, GUI_Scroll_Vars scroll){
    edit_pos->scroll = scroll;
    edit_pos->last_set_type = EditPos_ScrollSet;
}

internal i32
edit_pos_get_index(Editing_File *file, File_Edit_Positions *edit_pos){
    i32 edit_pos_index = -1;
    
    i32 count = file->state.edit_poss_count;
    File_Edit_Positions **edit_poss = file->state.edit_poss;
    for (i32 i = 0; i < count; ++i){
        if (edit_poss[i] == edit_pos){
            edit_pos_index = i;
            break;
        }
    }
    
    return(edit_pos_index);
}

internal b32
edit_pos_move_to_front(Editing_File *file, File_Edit_Positions *edit_pos){
    b32 result = false;
    
    if (file && edit_pos){
        i32 edit_pos_index = edit_pos_get_index(file, edit_pos);
        Assert(edit_pos_index != -1);
        
        File_Edit_Positions **edit_poss = file->state.edit_poss;
        
        memmove(edit_poss + 1, edit_poss, edit_pos_index*sizeof(*edit_poss));
        
        edit_poss[0] = edit_pos;
        result = true;
    }
    
    return(result);
}

internal b32
edit_pos_unset(Editing_File *file, File_Edit_Positions *edit_pos){
    b32 result = false;
    
    if (file && edit_pos){
        i32 edit_pos_index = edit_pos_get_index(file, edit_pos);
        Assert(edit_pos_index != -1);
        
        i32 count = file->state.edit_poss_count;
        File_Edit_Positions **edit_poss = file->state.edit_poss;
        
        memmove(edit_poss + edit_pos_index,
                edit_poss + edit_pos_index + 1,
                (count - edit_pos_index - 1)*sizeof(*edit_poss));
        
        edit_pos->in_view = false;
        
        if (file->state.edit_poss_count > 1){
            file->state.edit_poss_count -= 1;
        }
        result = true;
    }
    
    return(result);
}

internal File_Edit_Positions*
edit_pos_get_new(Editing_File *file, i32 index){
    File_Edit_Positions *result = 0;
    
    if (file && 0 <= index && index < 16){
        result = file->state.edit_pos_space + index;
        i32 edit_pos_index = edit_pos_get_index(file, result);
        
        if (edit_pos_index == -1){
            File_Edit_Positions **edit_poss = file->state.edit_poss;
            i32 count = file->state.edit_poss_count;
            
            if (count > 0){
                if (edit_poss[0]->in_view){
                    memcpy(result, edit_poss[0], sizeof(*result));
                    memmove(edit_poss+1, edit_poss, sizeof(*edit_poss)*count);
                    file->state.edit_poss_count = count + 1;
                }
                else{
                    Assert(count == 1);
                    memcpy(result, edit_poss[0], sizeof(*result));
                }
            }
            else{
                memset(result, 0, sizeof(*result));
                file->state.edit_poss_count = 1;
            }
            
            edit_poss[0] = result;
        }
        
        result->in_view = true;
    }
    
    return(result);
}

////////////////////////////////

inline b32
buffer_needs_save(Editing_File *file){
    b32 result = false;
    if (file->state.dirty == DirtyState_UnsavedChanges){
        result = true;
    }
    return(result);
}

inline b32
buffer_can_save(Editing_File *file){
    b32 result = false;
    if (file->state.dirty == DirtyState_UnsavedChanges ||
        file->state.dirty == DirtyState_UnloadedChanges){
        result = true;
    }
    return(result);
}

inline b32
file_is_ready(Editing_File *file){
    b32 result = false;
    if (file != 0 && file->is_loading == 0){
        result = true;
    }
    return(result);
}

inline void
file_set_unimportant(Editing_File *file, b32 val){
    if (val){
        file->state.dirty = DirtyState_UpToDate;
    }
    file->settings.unimportant = (b8)(val != false);
}

inline void
file_set_to_loading(Editing_File *file){
    memset(&file->state, 0, sizeof(file->state));
    memset(&file->settings, 0, sizeof(file->settings));
    file->is_loading = true;
}

inline void
file_set_dirty_flag(Editing_File *file, Dirty_State state){
    if (!file->settings.unimportant){
        file->state.dirty = state;
    }
    else{
        file->state.dirty = DirtyState_UpToDate;
    }
}

////////////////////////////////

internal b32
save_file_to_name(System_Functions *system, Models *models, Editing_File *file, char *filename){
    b32 result = false;
    b32 using_actual_filename = false;
    
    if (filename == 0){
        terminate_with_null(&file->canon.name);
        filename = file->canon.name.str;
        using_actual_filename = true;
    }
    
    if (filename != 0){
        Mem_Options *mem = &models->mem;
        if (models->hook_save_file != 0){
            models->hook_save_file(&models->app_links, file->id.id);
        }
        
        Gap_Buffer *buffer = &file->state.buffer;
        b32 dos_write_mode = file->settings.dos_write_mode;
        
        i32 max = 0;
        if (dos_write_mode){
            max = buffer_size(buffer) + buffer->line_count + 1;
        }
        else{
            max = buffer_size(buffer);
        }
        
        b32 used_heap = 0;
        Temp_Memory temp = begin_temp_memory(&mem->part);
        char empty = 0;
        char *data = 0;
        if (max == 0){
            data = &empty;
        }
        else{
            data = (char*)push_array(&mem->part, char, max);
            if (!data){
                used_heap = 1;
                data = heap_array(&mem->heap, char, max);
            }
        }
        Assert(data != 0);
        
        i32 size = 0;
        if (dos_write_mode){
            size = buffer_convert_out(buffer, data, max);
        }
        else{
            size = max;
            buffer_stringify(buffer, 0, size, data);
        }
        
        if (!using_actual_filename && file->canon.name.str != 0){
            char space[512];
            u32 length = str_size(filename);
            system->get_canonical(filename, length, space, sizeof(space));
            
            char *source_path = file->canon.name.str;
            if (match(space, source_path)){
                using_actual_filename = true;
            }
        }
        
        result = system->save_file(filename, data, size);
        
        if (result && using_actual_filename){
            file->state.ignore_behind_os = 1;
        }
        
        file_set_dirty_flag(file, DirtyState_UpToDate);
        
        if (used_heap){
            heap_free(&mem->heap, data);
        }
        end_temp_memory(temp);
        
    }
    
    return(result);
}

inline b32
save_file(System_Functions *system, Models *models, Editing_File *file){
    b32 result = save_file_to_name(system, models, file, 0);
    return(result);
}

////////////////////////////////

inline b32
file_compute_partial_cursor(Editing_File *file, Buffer_Seek seek, Partial_Cursor *cursor){
    b32 result = true;
    switch (seek.type){
        case buffer_seek_pos:
        {
            *cursor = buffer_partial_from_pos(&file->state.buffer, seek.pos);
        }break;
        
        case buffer_seek_line_char:
        {
            *cursor = buffer_partial_from_line_character(&file->state.buffer, seek.line, seek.character);
        }break;
        
        default:
        {
            result = false;
        }break;
    }
    return(result);
}

internal Full_Cursor
file_compute_cursor(System_Functions *system, Editing_File *file, Buffer_Seek seek, b32 return_hint){
    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
    Assert(font.valid);
    
    Full_Cursor result = {0};
    
    Buffer_Cursor_Seek_Params params;
    params.buffer           = &file->state.buffer;
    params.seek             = seek;
    params.system           = system;
    params.font             = font;
    params.wrap_line_index  = file->state.wrap_line_index;
    params.character_starts = file->state.character_starts;
    params.virtual_white    = file->settings.virtual_white;
    params.return_hint      = return_hint;
    params.cursor_out       = &result;
    
    Buffer_Cursor_Seek_State state = {0};
    Buffer_Layout_Stop stop = {0};
    
    i32 size = buffer_size(params.buffer);
    
    f32 line_shift = 0.f;
    b32 do_wrap = 0;
    i32 wrap_unit_end = 0;
    
    b32 first_wrap_determination = 1;
    i32 wrap_array_index = 0;
    
    do{
        stop = buffer_cursor_seek(&state, params, line_shift, do_wrap, wrap_unit_end);
        switch (stop.status){
            case BLStatus_NeedWrapDetermination:
            {
                if (stop.pos >= size){
                    do_wrap = 0;
                    wrap_unit_end = max_i32;
                }
                else{
                    if (first_wrap_determination){
                        wrap_array_index = binary_search(file->state.wrap_positions, stop.pos, 0, file->state.wrap_position_count);
                        ++wrap_array_index;
                        if (file->state.wrap_positions[wrap_array_index] == stop.pos){
                            do_wrap = 1;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        else{
                            do_wrap = 0;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        first_wrap_determination = 0;
                    }
                    else{
                        Assert(stop.pos == wrap_unit_end);
                        do_wrap = 1;
                        ++wrap_array_index;
                        wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                    }
                }
            }break;
            
            case BLStatus_NeedWrapLineShift:
            case BLStatus_NeedLineShift:
            {
                line_shift = file->state.line_indents[stop.wrap_line_index];
            }break;
        }
    }while(stop.status != BLStatus_Finished);
    
    return(result);
}

////////////////////////////////

internal i32
file_grow_starts_as_needed(Heap *heap, Gap_Buffer *buffer, i32 additional_lines){
    b32 result = GROW_NOT_NEEDED;
    i32 max = buffer->line_max;
    i32 count = buffer->line_count;
    i32 target_lines = count + additional_lines;
    if (target_lines > max || max == 0){
        max = l_round_up_i32(target_lines + max, KB(1));
        i32 *new_lines = heap_array(heap, i32, max);
        if (new_lines != 0){
            result = GROW_SUCCESS;
            memcpy(new_lines, buffer->line_starts, sizeof(*new_lines)*count);
            heap_free(heap, buffer->line_starts);
            buffer->line_starts = new_lines;
            buffer->line_max = max;
        }
        else{
            result = GROW_FAILED;
        }
    }
    return(result);
}

internal void
file_measure_starts(Heap *heap, Gap_Buffer *buffer){
    if (buffer->line_starts == 0){
        i32 max = buffer->line_max = KB(1);
        buffer->line_starts = heap_array(heap, i32, max);
        Assert(buffer->line_starts != 0);
    }
    
    Buffer_Measure_Starts state = {0};
    for (;buffer_measure_starts(&state, buffer);){
        i32 count = state.count;
        i32 max = ((buffer->line_max + 1) << 1);
        i32 *new_lines = heap_array(heap, i32, max);
        Assert(new_lines != 0);
        memcpy(new_lines, buffer->line_starts, sizeof(*new_lines)*count);
        heap_free(heap, buffer->line_starts);
        buffer->line_starts = new_lines;
        buffer->line_max = max;
    }
}

internal void
file_allocate_metadata_as_needed(Heap *heap, Gap_Buffer *buffer, void **mem, i32 *mem_max_count, i32 count, i32 item_size){
    if (*mem == 0){
        i32 max = l_round_up_i32(((count + 1)*2), KB(1));
        *mem = heap_allocate(heap, max*item_size);
        *mem_max_count = max;
        Assert(*mem != 0);
    }
    else if (*mem_max_count < count){
        i32 old_max = *mem_max_count;
        i32 max = l_round_up_i32(((count + 1)*2), KB(1));
        void *new_mem = heap_allocate(heap, item_size*max);
        memcpy(new_mem, *mem, item_size*old_max);
        heap_free(heap, *mem);
        *mem = new_mem;
        *mem_max_count = max;
        Assert(*mem != 0);
    }
}

inline void
file_allocate_character_starts_as_needed(Heap *heap, Editing_File *file){
    file_allocate_metadata_as_needed(heap,
                                     &file->state.buffer, (void**)&file->state.character_starts,
                                     &file->state.character_start_max, file->state.buffer.line_count + 1, sizeof(i32));
}

internal void
file_allocate_indents_as_needed(Heap *heap, Editing_File *file, i32 min_last_index){
    i32 min_amount = min_last_index + 1;
    file_allocate_metadata_as_needed(heap,
                                     &file->state.buffer, (void**)&file->state.line_indents,
                                     &file->state.line_indent_max, min_amount, sizeof(f32));
}

inline void
file_allocate_wraps_as_needed(Heap *heap, Editing_File *file){
    file_allocate_metadata_as_needed(heap,
                                     &file->state.buffer, (void**)&file->state.wrap_line_index,
                                     &file->state.wrap_max, file->state.buffer.line_count + 1, sizeof(f32));
}

inline void
file_allocate_wrap_positions_as_needed(Heap *heap, Editing_File *file, i32 min_last_index){
    i32 min_amount = min_last_index + 1;
    file_allocate_metadata_as_needed(heap,
                                     &file->state.buffer, (void**)&file->state.wrap_positions,
                                     &file->state.wrap_position_max, min_amount, sizeof(f32));
}

////////////////////////////////

internal void
file_create_from_string(System_Functions *system, Models *models, Editing_File *file, String val, u32 flags){
    Heap *heap = &models->mem.heap;
    Partition *part = &models->mem.part;
    Open_File_Hook_Function *hook_open_file = models->hook_open_file;
    Application_Links *app_links = &models->app_links;
    
    memset(&file->state, 0, sizeof(file->state));
    Gap_Buffer_Init init = buffer_begin_init(&file->state.buffer, val.str, val.size);
    for (; buffer_init_need_more(&init); ){
        i32 page_size = buffer_init_page_size(&init);
        page_size = l_round_up_i32(page_size, KB(4));
        if (page_size < KB(4)){
            page_size = KB(4);
        }
        void *data = heap_allocate(heap, page_size);
        buffer_init_provide_page(&init, data, page_size);
    }
    
    i32 scratch_size = partition_remaining(part);
    Assert(scratch_size > 0);
    b32 init_success = buffer_end_init(&init, part->base + part->pos, scratch_size);
    AllowLocal(init_success); Assert(init_success);
    
    if (buffer_size(&file->state.buffer) < val.size){
        file->settings.dos_write_mode = 1;
    }
    file_set_dirty_flag(file, DirtyState_UpToDate);
    
    Face_ID font_id = models->global_font_id;
    file->settings.font_id = font_id;
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    Assert(font.valid);
    
    {
        file_measure_starts(heap, &file->state.buffer);
        
        file_allocate_character_starts_as_needed(heap, file);
        buffer_measure_character_starts(system, font, &file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
        
        file_measure_wraps(system, &models->mem, file, font);
        //adjust_views_looking_at_files_to_new_cursor(system, models, file);
    }
    
    file->lifetime_object = lifetime_alloc_object(heap, &models->lifetime_allocator, DynamicWorkspace_Buffer, file);
    
    file->settings.read_only = ((flags & FileCreateFlag_ReadOnly) != 0);
    if (!file->settings.read_only){
        // TODO(allen): Redo undo system (if you don't mind the pun)
        i32 request_size = KB(64);
        file->state.undo.undo.max = request_size;
        file->state.undo.undo.strings = (u8*)heap_allocate(heap, request_size);
        file->state.undo.undo.edit_max = request_size/sizeof(Edit_Step);
        file->state.undo.undo.edits = (Edit_Step*)heap_allocate(heap, request_size);
        
        file->state.undo.redo.max = request_size;
        file->state.undo.redo.strings = (u8*)heap_allocate(heap, request_size);
        file->state.undo.redo.edit_max = request_size/sizeof(Edit_Step);
        file->state.undo.redo.edits = (Edit_Step*)heap_allocate(heap, request_size);
        
        file->state.undo.history.max = request_size;
        file->state.undo.history.strings = (u8*)heap_allocate(heap, request_size);
        file->state.undo.history.edit_max = request_size/sizeof(Edit_Step);
        file->state.undo.history.edits = (Edit_Step*)heap_allocate(heap, request_size);
        
        file->state.undo.children.max = request_size;
        file->state.undo.children.strings = (u8*)heap_allocate(heap, request_size);
        file->state.undo.children.edit_max = request_size/sizeof(Buffer_Edit);
        file->state.undo.children.edits = (Buffer_Edit*)heap_allocate(heap, request_size);
        
        file->state.undo.history_block_count = 1;
        file->state.undo.history_head_block = 0;
        file->state.undo.current_block_normal = 1;
    }
    
    if (hook_open_file != 0){
        hook_open_file(app_links, file->id.id);
    }
    file->settings.is_initialized = true;
}

internal void
file_free(System_Functions *system, Application_Links *app, Heap *heap, Lifetime_Allocator *lifetime_allocator,
          Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            heap_free(heap, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
    }
    if (file->state.token_array.tokens){
        heap_free(heap, file->state.token_array.tokens);
    }
    
    lifetime_free_object(heap, lifetime_allocator, file->lifetime_object);
    
    Gap_Buffer *buffer = &file->state.buffer;
    if (buffer->data){
        heap_free(heap, buffer->data);
        heap_free(heap, buffer->line_starts);
    }
    
    heap_free(heap, file->state.wrap_line_index);
    heap_free(heap, file->state.character_starts);
    heap_free(heap, file->state.line_indents);
    
    if (file->state.undo.undo.edits){
        heap_free(heap, file->state.undo.undo.strings);
        heap_free(heap, file->state.undo.undo.edits);
        
        heap_free(heap, file->state.undo.redo.strings);
        heap_free(heap, file->state.undo.redo.edits);
        
        heap_free(heap, file->state.undo.history.strings);
        heap_free(heap, file->state.undo.history.edits);
        
        heap_free(heap, file->state.undo.children.strings);
        heap_free(heap, file->state.undo.children.edits);
    }
}

internal void
init_normal_file(System_Functions *system, Models *models,
                 char *buffer, i32 size,
                 Editing_File *file){
    String val = make_string(buffer, size);
    file_create_from_string(system, models, file, val, 0);
    
    if (file->settings.tokens_exist && file->state.token_array.tokens == 0){
        if (!file->settings.virtual_white){
            file_first_lex_parallel(system, models, file);
        }
        else{
            file_first_lex_serial(models, file);
        }
    }
}

internal void
init_read_only_file(System_Functions *system, Models *models, Editing_File *file){
    String val = null_string;
    file_create_from_string(system, models, file, val, FileCreateFlag_ReadOnly);
    
    if (file->settings.tokens_exist && file->state.token_array.tokens == 0){
        if (!file->settings.virtual_white){
            file_first_lex_parallel(system, models, file);
        }
        else{
            file_first_lex_serial(models, file);
        }
    }
}

// BOTTOM

