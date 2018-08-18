/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * Undo
 *
 */

// TOP

internal void
undo_stack_grow_string(Heap *heap, Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = heap_array(heap, u8, new_max);
    memcpy(new_str, old_str, sizeof(*new_str)*old_max);
    heap_free(heap, old_str);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
undo_stack_grow_edits(Heap *heap, Edit_Stack *stack){
    i32 old_max = stack->edit_max;
    Edit_Step *old_eds = stack->edits;
    i32 new_max = old_max*2 + 2;
    Edit_Step *new_eds = heap_array(heap, Edit_Step, new_max);
    memcpy(new_eds, old_eds, sizeof(*new_eds)*old_max);
    heap_free(heap, old_eds);
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal void
child_stack_grow_string(Heap *heap, Small_Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = heap_array(heap, u8, new_max);
    memcpy(new_str, old_str, sizeof(*new_str)*old_max);
    heap_free(heap, old_str);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
child_stack_grow_edits(Heap *heap, Small_Edit_Stack *stack, i32 amount){
    i32 old_max = stack->edit_max;
    Buffer_Edit *old_eds = stack->edits;
    i32 new_max = old_max*2 + amount;
    Buffer_Edit *new_eds = heap_array(heap, Buffer_Edit, new_max);
    memcpy(new_eds, old_eds, sizeof(*new_eds)*new_max);
    heap_free(heap, old_eds);
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal i32
undo_children_push(Heap *heap, Small_Edit_Stack *children, Buffer_Edit *edits, i32 edit_count, u8 *strings, i32 string_size){
    i32 result = children->edit_count;
    if (children->edit_count + edit_count > children->edit_max){
        child_stack_grow_edits(heap, children, edit_count);
    }
    
    if (children->size + string_size > children->max){
        child_stack_grow_string(heap, children, string_size);
    }
    
    memcpy(children->edits + children->edit_count, edits, edit_count*sizeof(Buffer_Edit));
    memcpy(children->strings + children->size, strings, string_size);
    
    Buffer_Edit *edit = children->edits + children->edit_count;
    i32 start_pos = children->size;
    for (i32 i = 0; i < edit_count; ++i, ++edit){
        edit->str_start += start_pos;
    }
    
    children->edit_count += edit_count;
    children->size += string_size;
    
    return result;
}

internal Edit_Step*
file_post_undo(Heap *heap, Editing_File *file, Edit_Step step, b32 do_merge, b32 can_merge){
    if (step.type == ED_NORMAL){
        file->state.undo.redo.size = 0;
        file->state.undo.redo.edit_count = 0;
    }
    
    Edit_Stack *undo = &file->state.undo.undo;
    Edit_Step *result = 0;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + undo->size > undo->max){
            undo_stack_grow_string(heap, undo, step.edit.end - step.edit.start);
        }
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv, (char*)undo->strings, &undo->size, undo->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = ED_UNDO;
        
        b32 did_merge = 0;
        if (do_merge && undo->edit_count > 0){
            Edit_Step prev = undo->edits[undo->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                }
            }
        }
        
        if (did_merge){
            result = undo->edits + (undo->edit_count-1);
            *result = inv_step;
        }
        else{
            if (undo->edit_count == undo->edit_max){
                undo_stack_grow_edits(heap, undo);
            }
            
            result = undo->edits + (undo->edit_count++);
            *result = inv_step;
        }
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_UNDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (undo->edit_count == undo->edit_max){
            undo_stack_grow_edits(heap, undo);
        }
        result = undo->edits + (undo->edit_count++);
        *result = inv_step;
    }
    return result;
}

inline void
undo_stack_pop(Edit_Stack *stack){
    if (stack->edit_count > 0){
        Edit_Step *edit = stack->edits + (--stack->edit_count);
        if (edit->child_count == 0){
            stack->size -= edit->edit.len;
        }
    }
}

internal void
file_post_redo(Heap *heap, Editing_File *file, Edit_Step step){
    Edit_Stack *redo = &file->state.undo.redo;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + redo->size > redo->max){
            undo_stack_grow_string(heap, redo, step.edit.end - step.edit.start);
        }
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv, (char*)redo->strings, &redo->size, redo->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.type = ED_REDO;
        
        if (redo->edit_count == redo->edit_max){
            undo_stack_grow_edits(heap, redo);
        }
        redo->edits[redo->edit_count++] = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_REDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (redo->edit_count == redo->edit_max){
            undo_stack_grow_edits(heap, redo);
        }
        redo->edits[redo->edit_count++] = inv_step;
    }
}

inline void
file_post_history_block(Editing_File *file, i32 pos){
    Assert(file->state.undo.history_head_block < pos);
    Assert(pos < file->state.undo.history.edit_count);
    
    Edit_Step *history = file->state.undo.history.edits;
    Edit_Step *step = history + file->state.undo.history_head_block;
    step->next_block = pos;
    step = history + pos;
    step->prev_block = file->state.undo.history_head_block;
    file->state.undo.history_head_block = pos;
    ++file->state.undo.history_block_count;
}

inline void
file_unpost_history_block(Editing_File *file){
    Assert(file->state.undo.history_block_count > 1);
    --file->state.undo.history_block_count;
    Edit_Step *old_head = file->state.undo.history.edits + file->state.undo.history_head_block;
    file->state.undo.history_head_block = old_head->prev_block;
}

internal Edit_Step*
file_post_history(Heap *heap, Editing_File *file, Edit_Step step, b32 do_merge, b32 can_merge){
    Edit_Stack *history = &file->state.undo.history;
    Edit_Step *result = 0;
    
    local_persist Edit_Type reverse_types[4];
    if (reverse_types[ED_UNDO] == 0){
        reverse_types[ED_NORMAL] = ED_REVERSE_NORMAL;
        reverse_types[ED_REVERSE_NORMAL] = ED_NORMAL;
        reverse_types[ED_UNDO] = ED_REDO;
        reverse_types[ED_REDO] = ED_UNDO;
    }
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + history->size > history->max){
            undo_stack_grow_string(heap, history, step.edit.end - step.edit.start);
        }
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)history->strings, &history->size, history->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = reverse_types[step.type];
        
        b32 did_merge = 0;
        if (do_merge && history->edit_count > 0){
            Edit_Step prev = history->edits[history->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                }
            }
        }
        
        if (did_merge){
            result = history->edits + (history->edit_count-1);
        }
        else{
            if (history->edit_count == history->edit_max){
                undo_stack_grow_edits(heap, history);
            }
            result = history->edits + (history->edit_count++);
        }
        
        *result = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = reverse_types[step.type];
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.inverse_child_count = step.child_count;
        inv_step.child_count = step.inverse_child_count;
        
        if (history->edit_count == history->edit_max){
            undo_stack_grow_edits(heap, history);
        }
        result = history->edits + (history->edit_count++);
        *result = inv_step;
    }
    
    return(result);
}

internal void
file_update_history_before_edit(Mem_Options *mem, Editing_File *file, Edit_Step step, u8 *str, History_Mode history_mode){
    if (!file->state.undo.undo.edits) return;
    Heap *heap = &mem->heap;
    
    b32 can_merge = 0, do_merge = 0;
    switch (step.type){
        case ED_NORMAL:
        {
            if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)){
                can_merge = 1;
            }
            if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))){
                do_merge = 1;
            }
            
            if (history_mode != hist_forward){
                file_post_history(heap, file, step, do_merge, can_merge);
            }
            
            file_post_undo(heap, file, step, do_merge, can_merge);
        }break;
        
        case ED_REVERSE_NORMAL:
        {
            if (history_mode != hist_forward){
                file_post_history(heap, file, step, do_merge, can_merge);
            }
            
            undo_stack_pop(&file->state.undo.undo);
            
            b32 restore_redos = 0;
            Edit_Step *redo_end = 0;
            
            if (history_mode == hist_backward && file->state.undo.edit_history_cursor > 0){
                restore_redos = 1;
                redo_end = file->state.undo.history.edits + (file->state.undo.edit_history_cursor - 1);
            }
            else if (history_mode == hist_forward && file->state.undo.history.edit_count > 0){
                restore_redos = 1;
                redo_end = file->state.undo.history.edits + (file->state.undo.history.edit_count - 1);
            }
            
            if (restore_redos){
                Edit_Step *redo_start = redo_end;
                i32 steps_of_redo = 0;
                i32 strings_of_redo = 0;
                {
                    i32 undo_count = 0;
                    while (redo_start->type == ED_REDO || redo_start->type == ED_UNDO){
                        if (redo_start->type == ED_REDO){
                            if (undo_count > 0){
                                --undo_count;
                            }
                            else{
                                ++steps_of_redo;
                                strings_of_redo += redo_start->edit.len;
                            }
                        }
                        else{
                            ++undo_count;
                        }
                        --redo_start;
                    }
                }
                
                if (redo_start < redo_end){
                    ++redo_start;
                    ++redo_end;
                    
                    if (file->state.undo.redo.edit_count + steps_of_redo > file->state.undo.redo.edit_max)
                        undo_stack_grow_edits(heap, &file->state.undo.redo);
                    
                    if (file->state.undo.redo.size + strings_of_redo > file->state.undo.redo.max)
                        undo_stack_grow_string(heap, &file->state.undo.redo, strings_of_redo);
                    
                    u8 *str_src = file->state.undo.history.strings + redo_end->edit.str_start;
                    u8 *str_dest_base = file->state.undo.redo.strings;
                    i32 str_redo_pos = file->state.undo.redo.size + strings_of_redo;
                    
                    Edit_Step *edit_src = redo_end;
                    Edit_Step *edit_dest = file->state.undo.redo.edits + file->state.undo.redo.edit_count + steps_of_redo;
                    
                    {
                        i32 undo_count = 0;
                        for (i32 i = 0; i < steps_of_redo;){
                            --edit_src;
                            str_src -= edit_src->edit.len;
                            if (edit_src->type == ED_REDO){
                                if (undo_count > 0){
                                    --undo_count;
                                }
                                else{
                                    ++i;
                                    
                                    --edit_dest;
                                    *edit_dest = *edit_src;
                                    
                                    str_redo_pos -= edit_dest->edit.len;
                                    edit_dest->edit.str_start = str_redo_pos;
                                    
                                    memcpy(str_dest_base + str_redo_pos, str_src, edit_dest->edit.len);
                                }
                            }
                            else{
                                ++undo_count;
                            }
                        }
                        Assert(undo_count == 0);
                    }
                    
                    file->state.undo.redo.size += strings_of_redo;
                    file->state.undo.redo.edit_count += steps_of_redo;
                }
            }
        }break;
        
        case ED_UNDO:
        {
            if (history_mode != hist_forward){
                file_post_history(heap, file, step, do_merge, can_merge);
            }
            file_post_redo(heap, file, step);
            undo_stack_pop(&file->state.undo.undo);
        }break;
        
        case ED_REDO:
        {
            if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
            if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
            
            if (history_mode != hist_forward){
                file_post_history(heap, file, step, do_merge, can_merge);
            }
            
            file_post_undo(heap, file, step, do_merge, can_merge);
            undo_stack_pop(&file->state.undo.redo);
        }break;
    }
    
    if (history_mode != hist_forward){
        if (step.type == ED_UNDO || step.type == ED_REDO){
            if (file->state.undo.current_block_normal){
                file_post_history_block(file, file->state.undo.history.edit_count - 1);
                file->state.undo.current_block_normal = 0;
            }
        }
        else{
            if (!file->state.undo.current_block_normal){
                file_post_history_block(file, file->state.undo.history.edit_count - 1);
                file->state.undo.current_block_normal = 1;
            }
        }
    }
    else{
        if (file->state.undo.history_head_block == file->state.undo.history.edit_count){
            file_unpost_history_block(file);
            file->state.undo.current_block_normal = !file->state.undo.current_block_normal;
        }
    }
    
    if (history_mode == hist_normal){
        file->state.undo.edit_history_cursor = file->state.undo.history.edit_count;
    }
}

// BOTTOM

