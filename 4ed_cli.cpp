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
child_process_container_init(Child_Process_Container *container, Models *models){
    container->arena = make_arena_app_links(&models->app_links);
    dll_init_sentinel(&container->child_process_active_list);
    dll_init_sentinel(&container->child_process_free_list);
    container->child_process_id_counter = 0;
    block_zero_struct(&container->id_to_ptr_table);
}

internal void
child_process_container_release(Child_Process_Container *container, Models *models){
    linalloc_clear(&container->arena);
    heap_free(&models->mem.heap, container->id_to_ptr_table.mem);
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
    insert_u32_Ptr_table(&models->mem.heap, &container->id_to_ptr_table, new_id, new_process);
    container->active_child_process_count += 1;
    
    result.process = new_process;
    result.id = new_id;
    return(result);
}

internal Child_Process*
child_process_from_id(Child_Process_Container *container, Child_Process_ID id){
    Child_Process *process = 0;
    lookup_u32_Ptr_table(&container->id_to_ptr_table, &id, (void**)&process);
    return(process);
}

internal b32
child_process_free(Child_Process_Container *container, Child_Process_ID id){
    b32 result = false;
    Child_Process *process = child_process_from_id(container, id);
    if (process != 0){
        erase_u32_Ptr_table(&container->id_to_ptr_table, &id);
        dll_remove(&process->node);
        dll_insert(&container->child_process_free_list, &process->node);
        container->active_child_process_count -= 1;
        result = true;
    }
    return(result);
}

internal b32
child_process_set_return_code(Models *models, Child_Process_Container *container, Child_Process_ID id, i64 val){
    void *val_as_ptr = IntAsPtr(val);
    insert_u32_Ptr_table(&models->mem.heap, &container->id_to_return_code_table, id, val_as_ptr);
    return(true);
}

internal b32
child_process_lookup_return_code(Child_Process_Container *container, Child_Process_ID id, i64 *out){
    b32 result = false;
    void *val_as_ptr = 0;
    if (lookup_u32_Ptr_table(&container->id_to_return_code_table, &id, &val_as_ptr)){
        *out = (u64)PtrAsInt(val_as_ptr);
        result = true;
    }
    return(result);
}

////////////////////////////////

internal b32
child_process_call(Models *models, System_Functions *system, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *id_out){
    b32 result = false;
    Scratch_Block scratch(&models->app_links);
    String_Const_u8 path_n = push_string_copy(scratch, path);
    String_Const_u8 command_n = push_string_copy(scratch, command);
    CLI_Handles cli_handles = {};
    if (system->cli_call(scratch, (char*)path_n.str, (char*)command_n.str, &cli_handles)){
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

