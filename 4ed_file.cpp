/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * File layer for 4coder
 *
 */

// TOP

//
// Edit Position Basics
//

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

//
// Editing_File
//

inline Buffer_Slot_ID
to_file_id(i32 id){
    Buffer_Slot_ID result;
    result.id = id;
    return(result);
}

//
// Handling a file's Marker Arrays
//

internal void
init_file_markers_state(Editing_File_Markers *markers){
    Marker_Array *sentinel = &markers->sentinel;
    dll_init_sentinel(sentinel);
    markers->array_count = 0;
    markers->marker_count = 0;
}

internal void
clear_file_markers_state(General_Memory *general, Editing_File_Markers *markers){
    Marker_Array *sentinel = &markers->sentinel;
    for (Marker_Array *marker_array = sentinel->next;
         marker_array != sentinel;
         marker_array = sentinel->next){
        dll_remove(marker_array);
        general_memory_free(general, marker_array);
    }
    Assert(sentinel->next == sentinel);
    Assert(sentinel->prev == sentinel);
    markers->array_count = 0;
    markers->marker_count = 0;
}

internal void*
allocate_markers_state(General_Memory *general, Editing_File *file, u32 new_array_max){
    u32 memory_size = sizeof(Marker_Array) + sizeof(Marker)*new_array_max;
    memory_size = l_round_up_u32(memory_size, KB(4));
    u32 real_max = (memory_size - sizeof(Marker_Array))/sizeof(Marker);
    Marker_Array *array = (Marker_Array*)general_memory_allocate(general, memory_size);
    
    dll_insert_back(&file->markers.sentinel, array);
    array->buffer_id = file->id;
    array->count = 0;
    array->sim_max = new_array_max;
    array->max = real_max;
    
    ++file->markers.array_count;
    
    return(array);
}

internal Buffer_ID
get_buffer_id_from_marker_handle(void *handle){
    Marker_Array *markers = (Marker_Array*)handle;
    Buffer_Slot_ID result = markers->buffer_id;
    return(result.id);
}

internal b32
markers_set(Editing_File *file, void *handle, u32 first_index, u32 count, Marker *source){
    Assert(file != 0);
    b32 result = false;
    if (handle != 0){
        Marker_Array *markers = (Marker_Array*)handle;
        if (markers->buffer_id.id == file->id.id){
            if (first_index + count <= markers->sim_max){
                u32 new_count = first_index + count;
                if (new_count > markers->count){
                    file->markers.marker_count += new_count - markers->count;
                    markers->count = new_count;
                }
                Marker *dst = MarkerArrayBase(markers);
                memcpy(dst + first_index, source, sizeof(Marker)*count);
                result = true;
            }
        }
    }
    return(result);
}

internal b32
markers_get(Editing_File *file, void *handle, u32 first_index, u32 count, Marker *output){
    Assert(file != 0);
    b32 result = false;
    if (handle != 0){
        Marker_Array *markers = (Marker_Array*)handle;
        if (markers->buffer_id.id == file->id.id){
            if (first_index + count <= markers->count){
                Marker *src = MarkerArrayBase(markers);
                memcpy(output, src + first_index, sizeof(Marker)*count);
                result = true;
            }
        }
    }
    return(result);
}

internal b32
markers_free(General_Memory *general, Editing_File *file, void *handle){
    Assert(file != 0);
    b32 result = false;
    if (handle != 0){
        Marker_Array *markers = (Marker_Array*)handle;
        if (markers->buffer_id.id == file->id.id){
            dll_remove(markers);
            file->markers.marker_count -= markers->count;
            --file->markers.array_count;
            general_memory_free(general, markers);
        }
    }
    return(result);
}

//
// Manipulating a file's Edit_Pos array
//

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


//
// Cursor Seeking
//

inline Partial_Cursor
file_compute_cursor_from_pos(Editing_File *file, i32 pos){
    Partial_Cursor result = buffer_partial_from_pos(&file->state.buffer, pos);
    return(result);
}

inline Partial_Cursor
file_compute_cursor_from_line_character(Editing_File *file, i32 line, i32 character){
    Partial_Cursor result = buffer_partial_from_line_character(&file->state.buffer, line, character);
    return(result);
}

inline b32
file_compute_partial_cursor(Editing_File *file, Buffer_Seek seek, Partial_Cursor *cursor){
    b32 result = true;
    switch (seek.type){
        case buffer_seek_pos:
        {
            *cursor = file_compute_cursor_from_pos(file, seek.pos);
        }break;
        
        case buffer_seek_line_char:
        {
            *cursor = file_compute_cursor_from_line_character(file, seek.line, seek.character);
        }break;
        
        default:
        {
            result = false;
        }break;
    }
    return(result);
}


//
// Dirty Flags
//

inline b32
buffer_needs_save(Editing_File *file){
    b32 result = 0;
    if (!file->settings.unimportant){
        if (file->state.dirty == DirtyState_UnsavedChanges){
            result = 1;
        }
    }
    return(result);
}

inline b32
buffer_can_save(Editing_File *file){
    b32 result = 0;
    if (!file->settings.unimportant){
        if (file->state.dirty == DirtyState_UnsavedChanges ||
            file->state.dirty == DirtyState_UnloadedChanges){
            result = 1;
        }
    }
    return(result);
}

inline b32
file_is_ready(Editing_File *file){
    b32 result = 0;
    if (file && file->is_loading == 0){
        result = 1;
    }
    return(result);
}

inline void
file_set_to_loading(Editing_File *file){
    memset(&file->state, 0, sizeof(file->state));
    memset(&file->settings, 0, sizeof(file->settings));
    file->is_loading = true;
}

inline void
file_mark_clean(Editing_File *file){
    if (file->settings.unimportant){
        file->state.dirty = DirtyState_UpToDate;
    }
    else{
        if (file->state.dirty != DirtyState_UnloadedChanges){
            file->state.dirty = DirtyState_UpToDate;
        }
    }
}

inline void
file_mark_dirty(Editing_File *file){
    if (file->settings.unimportant){
        file->state.dirty = DirtyState_UpToDate;
    }
    else{
        if (file->state.dirty != DirtyState_UnloadedChanges){
            file->state.dirty = DirtyState_UnsavedChanges;
        }
    }
}

inline void
file_mark_behind_os(Editing_File *file){
    if (file->settings.unimportant){
        file->state.dirty = DirtyState_UpToDate;
    }
    else{
        file->state.dirty = DirtyState_UnloadedChanges;
    }
}

inline Dirty_State
file_get_sync(Editing_File *file){
    return (file->state.dirty);
}


// BOTTOM

