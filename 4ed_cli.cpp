/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.07.2017
 *
 * CLI handling code.
 *
 */

// TOP

internal void
child_process_container_init(Base_Allocator *allocator, Child_Process_Container *container){
    container->arena = make_arena(allocator);
    dll_init_sentinel(&container->child_process_active_list);
    dll_init_sentinel(&container->child_process_free_list);
    container->active_child_process_count = 0;
    container->child_process_id_counter = 0;
    container->id_to_ptr_table = make_table_u64_u64(allocator, 10);
    container->id_to_return_code_table = make_table_u64_u64(allocator, 10);
}

internal void
child_process_container_release(Child_Process_Container *container, Models *models){
    linalloc_clear(&container->arena);
    table_free(&container->id_to_ptr_table);
    block_zero_struct(container);
}

internal Child_Process_And_ID
child_process_alloc_new(Models *models, Child_Process_Container *container){
    Child_Process_And_ID result = {};
    Child_Process *new_process = 0;
    if (container->child_process_free_list.next != &container->child_process_free_list){
        Node *new_node = container->child_process_free_list.next;
        dll_remove(new_node);
        new_process = CastFromMember(Child_Process, node, new_node);
    }
    else{
        new_process = push_array(&container->arena, Child_Process, 1);
    }
    
    u32 new_id = ++container->child_process_id_counter;
    block_zero_struct(new_process);
    dll_insert_back(&container->child_process_active_list, &new_process->node);
    new_process->id = new_id;
    table_insert(&container->id_to_ptr_table, new_id, (u64)PtrAsInt(new_process));
    container->active_child_process_count += 1;
    
    result.process = new_process;
    result.id = new_id;
    return(result);
}

internal Child_Process*
child_process_from_id(Child_Process_Container *container, Child_Process_ID id){
    Table_Lookup lookup = table_lookup(&container->id_to_ptr_table, id);
    Child_Process *process = 0;
    if (lookup.found_match){
        u64 val = 0;
        table_read(&container->id_to_ptr_table, lookup, &val);
        process = (Child_Process*)IntAsPtr(val);
    }
    return(process);
}

internal b32
child_process_free(Child_Process_Container *container, Child_Process_ID id){
    b32 result = false;
    Child_Process *process = child_process_from_id(container, id);
    if (process != 0){
        table_erase(&container->id_to_ptr_table, id);
        dll_remove(&process->node);
        dll_insert(&container->child_process_free_list, &process->node);
        container->active_child_process_count -= 1;
        result = true;
    }
    return(result);
}

internal b32
child_process_set_return_code(Models *models, Child_Process_Container *container, Child_Process_ID id, i64 val){
    table_insert(&container->id_to_return_code_table, id, val);
    return(true);
}

internal b32
child_process_lookup_return_code(Child_Process_Container *container, Child_Process_ID id, i64 *out){
    b32 result = false;
    Table_Lookup lookup = table_lookup(&container->id_to_return_code_table, id);
    if (lookup.found_match){
        table_read(&container->id_to_return_code_table, lookup, (u64*)out);
        result = true;
    }
    return(result);
}

////////////////////////////////

internal b32
child_process_call(Thread_Context *tctx, Models *models, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *id_out){
    b32 result = false;
    Scratch_Block scratch(tctx);
    String_Const_u8 path_n = push_string_copy(scratch, path);
    String_Const_u8 command_n = push_string_copy(scratch, command);
    CLI_Handles cli_handles = {};
    if (system_cli_call(scratch, (char*)path_n.str, (char*)command_n.str, &cli_handles)){
        Child_Process_And_ID new_process = child_process_alloc_new(models, &models->child_processes);
        *id_out = new_process.id;
        new_process.process->cli = cli_handles;
        result = true;
    }
    return(result);
}

internal b32
child_process_set_target_buffer(Models *models, Child_Process *child_process, Editing_File *file, Child_Process_Set_Target_Flags flags){
    b32 result = false;
    b32 fail_if_process_has_buffer = HasFlag(flags, ChildProcessSet_FailIfProcessAlreadyAttachedToABuffer);
    b32 fail_if_buffer_has_process = HasFlag(flags, ChildProcessSet_FailIfBufferAlreadyAttachedToAProcess);
    b32 process_has_buffer = (child_process->out_file != 0);
    b32 buffer_has_process = (file->state.attached_child_process != 0);
    b32 fail = ((process_has_buffer && fail_if_process_has_buffer) ||
                (buffer_has_process && fail_if_buffer_has_process));
    if (!fail){
        if (process_has_buffer){
            child_process->out_file->state.attached_child_process = 0;
        }
        if (buffer_has_process){
            Child_Process *attached_child_process = child_process_from_id(&models->child_processes, file->state.attached_child_process);
            if (attached_child_process != 0){
                attached_child_process->out_file = 0;
            }
        }
        child_process->out_file = file;
        child_process->cursor_at_end = HasFlag(flags, ChildProcessSet_CursorAtEnd);
        file->state.attached_child_process = child_process->id;
        result = true;
    }
    return(result);
}

internal Process_State
child_process_get_state(Child_Process_Container *child_processes, Child_Process_ID child_process_id){
    Child_Process *child_process = child_process_from_id(child_processes, child_process_id);
    Process_State result = {};
    if (child_processes != 0){
        result.valid = true;
        result.is_updating = true;
    }
    else if (child_process_lookup_return_code(child_processes, child_process_id, &result.return_code)){
        result.valid = true;
    }
    return(result);
}

// BOTTOM

