/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.07.2017
 *
 * CLI handling code.
 *
 */

// TOP

internal CLI_List
make_cli_list(Partition *part, u32 max){
    CLI_List list = {};
    push_align(part, 8);
    list.procs = push_array(part, CLI_Process, max);
    list.max = max;
    return(list);
}

internal b32
cli_list_call(System_Functions *system, CLI_List *list, char *path, char *command, Editing_File *file, b32 cursor_at_end){
    b32 result = false;
    
    if (list->count < list->max){
        CLI_Process *proc = &list->procs[list->count++];
        file->is_updating = true;
        proc->out_file = file;
        proc->cursor_at_end = cursor_at_end;
        result = system->cli_call(path, command, &proc->cli);
        if (!result){
            --list->count;
        }
    }
    
    return(result);
}

internal void
cli_list_free_proc(CLI_List *list, CLI_Process *proc){
    Assert(proc >= list->procs);
    Assert(proc < list->procs + list->count);
    *proc = list->procs[--list->count];
}

internal b32
cli_list_has_space(CLI_List *list){
    b32 has_space = (list->count < list->max);
    return(has_space);
}

// BOTTOM

