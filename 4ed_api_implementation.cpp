/*
The implementation for the custom API
*/

// TOP

inline b32
access_test(u32 lock_flags, u32 access_flags){
    b32 result = 0;
    
    if ((lock_flags & ~access_flags) == 0){
        result = 1;
    }
    
    return(result);
}

internal void
fill_buffer_summary(Buffer_Summary *buffer, Editing_File *file, Working_Set *working_set){
    *buffer = buffer_summary_zero();
    if (!file->is_dummy){
        buffer->exists = 1;
        buffer->ready = file_is_ready(file);
        
        buffer->buffer_id = file->id.id;
        buffer->size = file->state.buffer.size;
        buffer->line_count = file->state.buffer.line_count;
        
        buffer->file_name_len = file->name.source_path.size;
        buffer->buffer_name_len = file->name.live_name.size;
        buffer->file_name = file->name.source_path.str;
        buffer->buffer_name = file->name.live_name.str;
        
        buffer->is_lexed = file->settings.tokens_exist;
        buffer->map_id = file->settings.base_map_id;
        buffer->unwrapped_lines = file->settings.unwrapped_lines;
        
        buffer->lock_flags = 0;
        if (file->settings.read_only){
            buffer->lock_flags |= AccessProtected;
        }
    }
}

internal void
fill_buffer_summary(Buffer_Summary *buffer, Editing_File *file, Command_Data *cmd){
    Working_Set *working_set = &cmd->models->working_set;
    fill_buffer_summary(buffer, file, working_set);
}

internal void
fill_view_summary(View_Summary *view, View *vptr, Live_Views *live_set, Working_Set *working_set){
    Buffer_ID buffer_id = 0;
    File_Viewing_Data *data = &vptr->file_data;
    
    *view = view_summary_zero();
    
    if (vptr->in_use){
        view->exists = 1;
        view->view_id = (int)(vptr - live_set->views) + 1;
        view->line_height = (float)(vptr->line_height);
        view->unwrapped_lines = vptr->file_data.unwrapped_lines;
        view->show_whitespace = vptr->file_data.show_whitespace;
        view->lock_flags = view_lock_flags(vptr);
        
        if (data->file){
            buffer_id = vptr->file_data.file->id.id;
            
            view->buffer_id = buffer_id;
            
            view->mark = view_compute_cursor_from_pos(vptr, vptr->edit_pos->mark);
            view->cursor = vptr->edit_pos->cursor;
            view->preferred_x = vptr->edit_pos->preferred_x;
            
            view->file_region = vptr->file_region;
            view->scroll_vars = vptr->edit_pos->scroll;
        }
    }
}

inline void
fill_view_summary(View_Summary *view, View *vptr, Command_Data *cmd){
    fill_view_summary(view, vptr, &cmd->vars->live_set, &cmd->models->working_set);
}

internal Editing_File*
get_file_from_identifier(System_Functions *system, Working_Set *working_set, Buffer_Identifier buffer){
    i32 buffer_id = buffer.id;
    i32 buffer_name_len = buffer.name_len;
    char *buffer_name = buffer.name;
    
    Editing_File *file = 0;
    
    if (buffer_id){
        file = working_set_get_active_file(working_set, buffer_id);
    }
    else if (buffer_name){
        file = working_set_contains(system, working_set, make_string(buffer_name, buffer_name_len));
    }
    
    return(file);
}

internal Editing_File*
imp_get_file(Command_Data *cmd, Buffer_Summary *buffer){
    Editing_File *file = 0;
    Working_Set *working_set = &cmd->models->working_set;;
    
    if (buffer->exists){
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file != 0 && !file_is_ready(file)){
            file = 0;
        }
    }
    
    return(file);
}

internal View*
imp_get_view(Command_Data *cmd, View_ID view_id){
    Live_Views *live_set = cmd->live_set;
    View *vptr = 0;
    
    view_id = view_id - 1;
    if (view_id >= 0 && view_id < live_set->max){
        vptr = live_set->views + view_id;
        if (!vptr->in_use){
            vptr = 0;
        }
    }
    
    return(vptr);
}

internal View*
imp_get_view(Command_Data *cmd, View_Summary *view){
    View *vptr = 0;
    
    if (view->exists){
        vptr = imp_get_view(cmd, view->view_id);
    }
    
    return(vptr);
}

#define API_EXPORT

API_EXPORT bool32
Exec_Command(Application_Links *app, Command_ID command_id)/*
DOC_PARAM(command_id, The command_id parameter specifies which internal command to execute.)
DOC_RETURN(This call returns non-zero if command_id named a valid internal command.)
DOC(A call to exec_command executes an internal command.
If command_id is invalid a warning is posted to *messages*.)
DOC_SEE(Command_ID)
*/{
    bool32 result = false;
    
    if (command_id < cmdid_count){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Command_Function function = command_table[command_id];
        Command_Binding binding = {};
        binding.function = function;
        if (function) function(cmd->system, cmd, binding);
        
        update_command_data(cmd->vars, cmd);
        
        result = true;
    }
    else{
        app->print_message(app, literal("CUSTOM WARNING: An invalid Command_ID was passed to exec_command."));
    }
    
    return(result);
}

// TODO(allen): This is a bit of a mess and needs to be fixed soon
API_EXPORT bool32
Exec_System_Command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Input_Flag flags)/*
DOC_PARAM(view, If the view parameter is non-null it specifies a view to display the command's output buffer.)
DOC_PARAM(buffer, The buffer the command will output to is specified by the buffer parameter.
See Buffer_Identifier for information on how this type specifies a buffer.)
DOC_PARAM(path, The path parameter specifies the path in which the command shall be executed. The string need not be null terminated.)
DOC_PARAM(path_len, The parameter path_len specifies the length of the path string.)
DOC_PARAM(command, The command parameter specifies the command that shall be executed. The string need not be null terminated.)
DOC_PARAM(command_len, The parameter command_len specifies the length of the command string.)
DOC_PARAM(flags, Flags for the behavior of the call are specified in the flags parameter.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
A call to exec_system_command executes a command as if called from the command line, and sends the output to
a buffer. The buffer identifier can either name a new buffer that does not exist, name a buffer that does
exist, or provide the id of a buffer that does exist.

If the buffer is not already in an open view and the view parameter is not NULL,
then the provided view will display the output buffer.

If the view parameter is NULL, no view will switch to the output.
)
DOC_SEE(Buffer_Identifier)
DOC_SEE(Command_Line_Input_Flag)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    App_Vars *vars = cmd->vars;
    Models *models = cmd->models;
    
    char feedback_space[256];
    String feedback_str = make_fixed_width_string(feedback_space);
    
    Working_Set *working_set = &models->working_set;
    CLI_Process *procs = vars->cli_processes.procs, *proc = 0;
    Editing_File *file = 0;
    b32 bind_to_new_view = true;
    General_Memory *general = &models->mem.general;
    
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    
    bool32 result = true;
    
    View *vptr = imp_get_view(cmd, view);
    
    if (vars->cli_processes.count < vars->cli_processes.max){
        file = get_file_from_identifier(system, working_set, buffer);
        if (file){
            if (file->settings.read_only == 0){
                append(&feedback_str, "ERROR: ");
                append(&feedback_str, file->name.live_name);
                append(&feedback_str, " is not a read-only buffer\n");
                do_feedback_message(system, models, feedback_str);
                result = false;
                goto done;
            }
            if (file->settings.never_kill){
                append(&feedback_str, "The buffer ");
                append(&feedback_str, file->name.live_name);
                append(&feedback_str, " is not killable");
                do_feedback_message(system, models, feedback_str);
                result = false;
                goto done;
            }
        }
        else if (buffer.name){
            file = working_set_alloc_always(working_set, general);
            if (file == 0){
                append(&feedback_str, "ERROR: unable to  allocate a new buffer\n");
                do_feedback_message(system, models, feedback_str);
                result = false;
                goto done;
            }
            file_create_read_only(system, models, file, buffer.name);
            working_set_add(system, working_set, file, general);
        }
        
        if (file){
            i32 proc_count = vars->cli_processes.count;
            for (i32 i = 0; i < proc_count; ++i){
                if (procs[i].out_file == file){
                    if (flags & CLI_OverlapWithConflict){
                        procs[i].out_file = 0;
                    }
                    else{
                        file = 0;
                    }
                    break;
                }
            }
            
            if (file){
                file_clear(system, models, file);
                file->settings.unimportant = 1;
                
                if (!(flags & CLI_AlwaysBindToView)){
                    View_Iter iter = file_view_iter_init(&models->layout, file, 0);
                    if (file_view_iter_good(iter)){
                        bind_to_new_view = false;
                    }
                }
            }
            else{
                append(&feedback_str, "did not begin command-line command because the target buffer is already in use\n");
                do_feedback_message(system, models, feedback_str);
                result = false;
                goto done;
            }
        }
        
        String path_string = {0};
        if (!path){
            terminate_with_null(&models->hot_directory.string);
            path_string = models->hot_directory.string;
        }
        else{
            path_string = make_string_terminated(part, path, path_len);
        }
        
        {
            String command_string = {0};
            
            if (!command){
#define NO_SCRIPT " echo no script specified"
                command_string.str = NO_SCRIPT;
                command_string.size = sizeof(NO_SCRIPT)-1;
#undef NO_SCRIPT
            }
            else{
                command_string = make_string_terminated(part, command, command_len);
            }
            
            if (vptr && bind_to_new_view){
                view_set_file(vptr, file, models);
                view_show_file(vptr);
            }
            
            proc = procs + vars->cli_processes.count++;
            proc->out_file = file;
            if (flags & CLI_CursorAtEnd){
                proc->cursor_at_end = 1;
            }
            else{
                proc->cursor_at_end = 0;
            }
            
            if (!system->cli_call(path_string.str, command_string.str, &proc->cli)){
                --vars->cli_processes.count;
            }
        }
    }
    else{
        append(&feedback_str, "ERROR: no available process slot\n");
        do_feedback_message(system, models, feedback_str);
        result = false;
        goto done;
    }
    
    done:
    end_temp_memory(temp);
    return(result);
}

API_EXPORT void
Clipboard_Post(Application_Links *app, int32_t clipboard_id, char *str, int32_t len)/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC_PARAM(str, The str parameter specifies the string to be posted to the clipboard, it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the str string.)
DOC
(
Stores the string str in the clipboard initially with index 0.
Also reports the copy to the operating system, so that it may
be pasted into other applications.
)
DOC_SEE(The_4coder_Clipboard)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    General_Memory *general = &models->mem.general;
    Working_Set *working = &models->working_set;
    
    String *dest = working_set_next_clipboard_string(general, working, len);
    copy(dest, make_string(str, len));
    system->post_clipboard(*dest);
}

API_EXPORT int32_t
Clipboard_Count(Application_Links *app, int32_t clipboard_id)/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC(This call returns the number of items in the clipboard.)
DOC_SEE(The_4coder_Clipboard)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working = &cmd->models->working_set;
    int32_t count = working->clipboard_size;
    return(count);
}

API_EXPORT int32_t
Clipboard_Index(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len)/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC_PARAM(item_index, This parameter specifies which item to read, 0 is the most recent copy, 1 is the second most recent copy, etc.)
DOC_PARAM(out, This parameter provides a buffer where the clipboard contents are written.  This parameter may be NULL.)
DOC_PARAM(len, This parameter specifies the length of the out buffer.)
DOC_RETURN(This call returns the size of the item associated with item_index.)
DOC
(
This function always returns the size of the item even if the output buffer is NULL.
If the output buffer is too small to contain the whole string, it is filled with the
first len character of the clipboard contents.  The output string is not null terminated.
)
DOC_SEE(The_4coder_Clipboard)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working = &cmd->models->working_set;
    
    int32_t size = 0;
    String *str = working_set_clipboard_index(working, item_index);
    if (str){
        size = str->size;
        if (out){
            String out_str = make_string(out, 0, len);
            copy(&out_str, *str);
        }
    }
    
    return(size);
}

API_EXPORT int32_t
Get_Buffer_Count(Application_Links *app)/*
DOC(TODO)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
    int32_t result = working_set->file_count;
    return(result);
}

internal void
internal_get_buffer_first(Working_Set *working_set, Buffer_Summary *buffer){
    if (working_set->file_count > 0){
        fill_buffer_summary(buffer, (Editing_File*)working_set->used_sentinel.next, working_set);
    }
}

internal void
internal_get_buffer_next(Working_Set *working_set, Buffer_Summary *buffer){
    Editing_File *file;
    
    file = working_set_get_active_file(working_set, buffer->buffer_id);
    if (file){
        file = (Editing_File*)file->node.next;
        fill_buffer_summary(buffer, file, working_set);
    }
    else{
        *buffer = buffer_summary_zero();
    }
}

API_EXPORT Buffer_Summary
Get_Buffer_First(Application_Links *app, Access_Flag access)/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns the summary of the first buffer in a buffer loop.)
DOC
(
This call begins a loop across all the buffers.

If the buffer returned does not exist, the loop is finished.
Buffers should not be killed durring a buffer loop.
)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
DOC_SEE(get_buffer_next)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
    Buffer_Summary result = {};
    
    internal_get_buffer_first(working_set, &result);
    while (result.exists && !access_test(result.lock_flags, access)){
        internal_get_buffer_next(working_set, &result);
    }
    
    return(result);
}

API_EXPORT void
Get_Buffer_Next(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access)/*
DOC_PARAM(buffer, The Buffer_Summary pointed to by buffer is iterated to the next buffer or to a null summary if this is the last buffer.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access. The buffer outputted will be the next buffer that is accessible.)
DOC
(
This call steps a Buffer_Summary to the next buffer in the global buffer order.
The global buffer order is kept roughly in the order of most recently used to least recently used.

If the buffer outputted does not exist, the loop is finished.
Buffers should not be killed or created durring a buffer loop.
)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
DOC_SEE(get_buffer_first)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
    
    internal_get_buffer_next(working_set, buffer);
    while (buffer->exists && !access_test(buffer->lock_flags, access)){
        internal_get_buffer_next(working_set, buffer);
    }
}

API_EXPORT Buffer_Summary
Get_Buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)/*
DOC_PARAM(buffer_id, The parameter buffer_id specifies which buffer to try to get.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
DOC_SEE(Buffer_ID)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
    Buffer_Summary buffer = {};
    Editing_File *file;
    
    file = working_set_get_active_file(working_set, buffer_id);
    if (file){
        fill_buffer_summary(&buffer, file, working_set);
        if (!access_test(buffer.lock_flags, access)){
            buffer = buffer_summary_zero();
        }
    }
    
    return(buffer);
}

API_EXPORT Buffer_Summary
Get_Buffer_By_Name(Application_Links *app, char *name, int32_t len, Access_Flag access)/*
DOC_PARAM(name, The name parameter specifies the buffer name to try to get. The string need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Buffer_Summary buffer = {};
    Editing_File *file;
    Working_Set *working_set = &cmd->models->working_set;
    
    file = working_set_contains(cmd->system, working_set, make_string(name, len));
    if (file && !file->is_dummy){
        fill_buffer_summary(&buffer, file, working_set);
        if (!access_test(buffer.lock_flags, access)){
            buffer = buffer_summary_zero();
        }
    }
    
    return(buffer);
}

internal i32
seek_token_left(Cpp_Token_Stack *tokens, i32 pos){
    Cpp_Get_Token_Result get = cpp_get_token(tokens, pos);
    if (get.token_index == -1){
        get.token_index = 0;
    }
    
    Cpp_Token *token = tokens->tokens + get.token_index;
    if (token->start == pos && get.token_index > 0){
        --token;
    }
    
    return token->start;
}

internal i32
seek_token_right(Cpp_Token_Stack *tokens, i32 pos){
    Cpp_Get_Token_Result get = cpp_get_token(tokens, pos);
    if (get.in_whitespace){
        ++get.token_index;
    }
    if (get.token_index >= tokens->count){
        get.token_index = tokens->count-1;
    }
    
    Cpp_Token *token = tokens->tokens + get.token_index;
    return token->start + token->size;
}

API_EXPORT int32_t
Buffer_Boundary_Seek(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, bool32 seek_forward, Seek_Boundary_Flag flags)/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer through which to seek.)
DOC_PARAM(start_pos, The beginning position of the seek is specified by start_pos measured in absolute position.)
DOC_PARAM(seek_forward, If this parameter is non-zero it indicates that the seek should move foward through the buffer.)
DOC_PARAM(flags, This field specifies the types of boundaries at which the seek should stop.)
DOC_RETURN(This call returns the absolute position where the seek stopped.
If the seek goes below 0 the returned value is -1.
If the seek goes past the end the returned value is the size of the buffer.)
DOC_SEE(Seek_Boundary_Flag)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file;
    int32_t result = 0;
    
    file = imp_get_file(cmd, buffer);
    
    if (file){
        // TODO(allen): reduce duplication?
        i32 size = buffer_size(&file->state.buffer);
        i32 pos[4] = {0};
        i32 new_pos = 0;
        
        if (start_pos < 0){
            start_pos = 0;
        }
        else if (start_pos > size){
            start_pos = size;
        }
        
        if (seek_forward){
            for (i32 i = 0; i < ArrayCount(pos); ++i) pos[i] = size;
            
            if (flags & (1)){
                pos[0] = buffer_seek_whitespace_right(&file->state.buffer, start_pos);
            }
            
            if (flags & (1 << 1)){
                if (file->state.tokens_complete){
                    pos[1] = seek_token_right(&file->state.token_stack, start_pos);
                }
                else{
                    pos[1] = buffer_seek_whitespace_right(&file->state.buffer, start_pos);
                }
            }
            
            if (flags & (1 << 2)){
                pos[2] = buffer_seek_alphanumeric_right(&file->state.buffer, start_pos);
                if (flags & (1 << 3)){
                    pos[3] = buffer_seek_range_camel_right(&file->state.buffer, start_pos, pos[2]);
                }
            }
            else{
                if (flags & (1 << 3)){
                    pos[3] = buffer_seek_alphanumeric_or_camel_right(&file->state.buffer, start_pos);
                }
            }
            
            new_pos = size;
            for (i32 i = 0; i < ArrayCount(pos); ++i){
                if (pos[i] < new_pos) new_pos = pos[i];
            }
        }
        else{
            if (flags & (1)){
                pos[0] = buffer_seek_whitespace_left(&file->state.buffer, start_pos);
            }
            
            if (flags & (1 << 1)){
                if (file->state.tokens_complete){
                    pos[1] = seek_token_left(&file->state.token_stack, start_pos);
                }
                else{
                    pos[1] = buffer_seek_whitespace_left(&file->state.buffer, start_pos);
                }
            }
            
            if (flags & (1 << 2)){
                pos[2] = buffer_seek_alphanumeric_left(&file->state.buffer, start_pos);
                if (flags & (1 << 3)){
                    pos[3] = buffer_seek_range_camel_left(&file->state.buffer, start_pos, pos[2]);
                }
            }
            else{
                if (flags & (1 << 3)){
                    pos[3] = buffer_seek_alphanumeric_or_camel_left(&file->state.buffer, start_pos);
                }
            }
            
            new_pos = 0;
            for (i32 i = 0; i < ArrayCount(pos); ++i){
                if (pos[i] > new_pos) new_pos = pos[i];
            }
        }
        result = new_pos;
        
        fill_buffer_summary(buffer, file, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Read_Range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *out)/*
DOC_PARAM(buffer, This parameter specifies the buffer to read.)
DOC_PARAM(start, This parameter specifies absolute position of the first character in the read range.)
DOC_PARAM(end, This parameter specifies the absolute position of the the character one past the end of the read range.)
DOC_PARAM(out, This paramter provides the output character buffer to fill with the result of the read.)
DOC_RETURN(This call returns non-zero if the read succeeds.)
DOC
(
The output buffer must have a capacity of at least (end - start).
The output is not null terminated.

This call fails if the buffer does not exist,
or if the read range is not within the bounds of the buffer.
)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file = imp_get_file(cmd, buffer);
    bool32 result = false;
    int32_t size = 0;
    
    if (file){
        size = buffer_size(&file->state.buffer);
        if (0 <= start && start <= end && end <= size){
            result = true;
            buffer_stringify(&file->state.buffer, start, end, out);
        }
        fill_buffer_summary(buffer, file, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Replace_Range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *str, int32_t len)/*
DOC_PARAM(buffer, This parameter specifies the buffer to edit.)
DOC_PARAM(start, This parameter specifies absolute position of the first character in the replace range.)
DOC_PARAM(end, This parameter specifies the absolute position of the the character one past the end of the replace range.)
DOC_PARAM(str, This parameter specifies the the string to write into the range; it need not be null terminated.)
DOC_PARAM(len, This parameter specifies the length of the str string.)
DOC_RETURN(This call returns non-zero if the replacement succeeds.)
DOC
(
If this call succeeds it deletes the range from start to end
and writes str in the same position.  If end == start then
this call is equivalent to inserting the string at start.
If len == 0 this call is equivalent to deleteing the range
from start to end.

This call fails if the buffer does not exist, or if the replace
range is not within the bounds of the buffer.
)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file = imp_get_file(cmd, buffer);
    
    bool32 result = false;
    int32_t size = 0;
    
    if (file){
        size = buffer_size(&file->state.buffer);
        if (0 <= start && start <= end && end <= size){
            result = true;
            
            file_replace_range(cmd->system, cmd->models,
                               file, start, end, str, len);
        }
        fill_buffer_summary(buffer, file, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Compute_Cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out)/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer on which to run the cursor computation.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(cursor_out, On success this struct is filled with the result of the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(Computes a Partial_Cursor for the given seek position with no side effects.
The seek position must be one of the types supported by Partial_Cursor.  Those
types are absolute position and line,column position.)
DOC_SEE(Buffer_Seek)
DOC_SEE(Partial_Cursor)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file = imp_get_file(cmd, buffer);
    bool32 result = false;
    
    if (file){
        if (file_compute_cursor(file, seek, cursor_out)){
            result = true;
            fill_buffer_summary(buffer, file, cmd);
        }
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Batch_Edit(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type)/*
DOC_PARAM(str, This parameter provides all of the source string for the edits in the batch.)
DOC_PARAM(str_len, This parameter specifies the length of the str string.)
DOC_PARAM(edits, This parameter provides about the source string and destination range of each edit as an array.)
DOC_PARAM(edit_count, This parameter specifies the number of Buffer_Edit structs in edits.)
DOC_PARAM(type, This prameter specifies what type of batch edit to execute.)
DOC_RETURN(This call returns non-zero if the batch edit succeeds.)
DOC(TODO)
DOC_SEE(Buffer_Edit)
DOC_SEE(Buffer_Batch_Edit_Type)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Models *models = cmd->models;
    Mem_Options *mem = &models->mem;
    Partition *part = &mem->part;
    Editing_File *file = imp_get_file(cmd, buffer);
    
    bool32 result = false;
    
    if (file){
        if (edit_count > 0){
            Temp_Memory temp = begin_temp_memory(part);
            Buffer_Edit *inverse_edits = push_array(part, Buffer_Edit, edit_count);
            Assert(inverse_edits);
            
            char *inv_str = (char*)part->base + part->pos;
            int inv_str_max = part->max - part->pos;
            
            Edit_Spec spec =
                file_compute_edit(mem, file,
                                  edits, str, str_len,
                                  inverse_edits, inv_str, inv_str_max,
                                  edit_count, type);
            
            file_do_batch_edit(cmd->system, models, file, spec, hist_normal, type);
            
            end_temp_memory(temp);
        }
        else{
            result = true;
        }
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Set_Setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value)/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer on which to set a setting.)
DOC_PARAM(setting, The setting parameter identifies the setting that shall be changed.)
DOC_PARAM(value, The value parameter specifies the value to which the setting shall be changed.)
DOC_SEE(Buffer_Setting_ID)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    Editing_File *file = imp_get_file(cmd, buffer);
    
    bool32 result = false;
    
    i32 new_mapid = 0;
    
    if (file){
        result = true;
        switch (setting){
            case BufferSetting_Lex:
            {
#if BUFFER_EXPERIMENT_SCALPEL <= 0
                if (file->settings.tokens_exist){
                    if (!value){
                        file_kill_tokens(system, &models->mem.general, file);
                    }
                }
                else{
                    if (value){
                        file_first_lex_parallel(system, &models->mem.general, file);
                    }
                }
#endif
            }break;
            
            case BufferSetting_WrapLine:
            {
                file->settings.unwrapped_lines = !value;
            }break;
            
            case BufferSetting_MapID:
            {
                if (value == mapid_global){
                    file->settings.base_map_id = mapid_global;
                }
                else if (value == mapid_file){
                    file->settings.base_map_id = mapid_file;
                }
                else if (value < mapid_global){
                    new_mapid = get_map_index(models, value);
                    if (new_mapid  < models->user_map_count){
                        file->settings.base_map_id = value;
                    }
                    else{
                        file->settings.base_map_id = mapid_file;
                    }
                }
                
                for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
                     file_view_iter_good(iter);
                     iter = file_view_iter_next(iter)){
                    iter.view->map = get_map(models, file->settings.base_map_id);
                }
            }break;
            
            case BufferSetting_Eol:
            {
                file->settings.dos_write_mode = value;
            }break;
            
            case BufferSetting_Unimportant:
            {
                if (value){
                    file->settings.unimportant = true;
                }
                else{
                    file->settings.unimportant = false;
                }
            }break;
            
            case BufferSetting_ReadOnly:
            {
                if (value){
                    file->settings.read_only = true;
                }
                else{
                    file->settings.read_only = false;
                }
            }break;
        }
        fill_buffer_summary(buffer, file, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Auto_Indent(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, int32_t tab_width, Auto_Indent_Flag flags)/*
DOC_PARAM(buffer, The buffer specifies the buffer in which to apply auto indentation.)
DOC_PARAM(start, This parameter specifies the absolute position of the start of the indentation range.)
DOC_PARAM(end, This parameter specifies the absolute position of the end of the indentation range.)
DOC_PARAM(tab_width, The tab_width parameter specifies how many spaces should be used for one indentation in space mode.)
DOC_PARAM(flags, This parameter specifies behaviors for the indentation.)
DOC_RETURN(This call returns non-zero when the call succeeds.)
DOC
(
Applies the built in auto-indentation rule to the code in the range from
start to end by inserting spaces or tabs at the beginning of the lines.
If the buffer does not have lexing enabled or the lexing job has not
completed this function will fail.
)
DOC_SEE(Auto_Indent_Flag)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    
    Indent_Options opts = {0};
    bool32 result = false;
    
    Editing_File *file = imp_get_file(cmd, buffer);
    if (file && file->state.token_stack.tokens &&
        file->state.tokens_complete && !file->state.still_lexing){
        result = true;
        
        opts.empty_blank_lines = (flags & AutoIndent_ClearLine);
        opts.use_tabs = (flags & AutoIndent_UseTab);
        opts.tab_width = tab_width;
        
        file_auto_tab_tokens(system, models, file, start, start, end, opts);
        
        fill_buffer_summary(buffer, file, cmd);
    }
    
    return(result);
}

API_EXPORT Buffer_Summary
Create_Buffer(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags)/*
DOC_PARAM(filename, The filename parameter specifies the name of the file to be opened or created; it need not be null terminated.)
DOC_PARAM(filename_len, The filename_len parameter spcifies the length of the filename string.)
DOC_PARAM(flags, The flags parameter specifies behaviors for buffer creation.)
DOC_RETURN(This call returns the summary of the created buffer.)
DOC
(
Tries to create a new buffer and associate it to the given filename.  If such a buffer already
exists the existing buffer is returned in the Buffer_Summary and no new buffer is created.
If the buffer does not exist a new buffer is created and named after the given filename.  If
the filename corresponds to a file on the disk that file is loaded and put into buffer, if
the filename does not correspond to a file on disk the buffer is created empty.
)
DOC_SEE(Buffer_Summary)
DOC_SEE(Buffer_Create_Flag)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    Working_Set *working_set = &models->working_set;
    General_Memory *general = &models->mem.general;
    Partition *part = &models->mem.part;
    
    Buffer_Summary result = {0};
    
    Temp_Memory temp = begin_temp_memory(part);
    if (filename != 0){
        String filename_string = make_string_terminated(part, filename, filename_len);
        Editing_File *file = working_set_contains(system, working_set, filename_string);
        
        if (file == 0){
            File_Loading loading = {0};
            
            b32 do_new_file = false;
            
            if (flags & BufferCreate_AlwaysNew){
                do_new_file = true;
            }
            else{
                loading = system->file_load_begin(filename_string.str);
                if (!loading.exists){
                    do_new_file = true;
                }
            }
            
            if (!do_new_file){
                b32 in_general_mem = false;
                char *buffer = push_array(part, char, loading.size);
                
                if (buffer == 0){
                    buffer = (char*)general_memory_allocate(general, loading.size);
                    if (buffer != 0){
                        in_general_mem = true;
                    }
                }
                
                if (system->file_load_end(loading, buffer)){
                    file = working_set_alloc_always(working_set, general);
                    if (file){
                        file_init_strings(file);
                        file_set_name(working_set, file, filename_string);
                        working_set_add(system, working_set, file, general);
                        init_normal_file(system, models, file,
                                         buffer, loading.size);
                        fill_buffer_summary(&result, file, cmd);
                    }
                }
                
                if (in_general_mem){
                    general_memory_free(general, buffer);
                }
                
            }
            else{
                file = working_set_alloc_always(working_set, general);
                if (file){
                    file_init_strings(file);
                    file_set_name(working_set, file, filename_string);
                    working_set_add(system, working_set, file, general);
                    init_normal_file(system, models, file, 0, 0);
                    fill_buffer_summary(&result, file, cmd);
                }
            }
        }
        else{
            fill_buffer_summary(&result, file, cmd);
        }
    }
    end_temp_memory(temp);
    
    return(result);
}

API_EXPORT bool32
Save_Buffer(Application_Links *app, Buffer_Summary *buffer, char *filename, int32_t filename_len, uint32_t flags)/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer to save to a file.)
DOC_PARAM(filename, The filename parameter specifies the name of the file to associated to the buffer; it need not be null terminated.)
DOC_PARAM(filename_len, The filename_len parameter specifies the length of the filename string.)
DOC_PARAM(flags, This parameter is not currently used and should be set to 0 for now.)
DOC_RETURN(This call returns non-zero on success.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    bool32 result = false;
    
    Editing_File *file = imp_get_file(cmd, buffer);
    if (file){
        result = true;
        String name = make_string(filename, filename_len);
        view_save_file(system, models, file, 0, name, false);
    }
    
    return(result);
}

API_EXPORT bool32
Kill_Buffer(Application_Links *app, Buffer_Identifier buffer, View_ID view_id, Buffer_Kill_Flag flags)/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer to try to kill.)
DOC_PARAM(view_id, The view_id parameter specifies the view that will contain the "are you sure" dialogue if the buffer is dirty.)
DOC_PARAM(flags, The flags parameter specifies behaviors for the buffer kill.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
Tries to kill the idenfied buffer.  If the buffer is dirty and the "are you sure"
dialogue needs to be displayed the provided view is used to show the dialogue.
If the view is not open the kill fails.
)
DOC_SEE(Buffer_Kill_Flag)
DOC_SEE(Buffer_Identifier)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    Working_Set *working_set = &models->working_set;
    View *vptr = imp_get_view(cmd, view_id);
    Editing_File *file = get_file_from_identifier(system, working_set, buffer);
    int result = false;
    
    if (file){
        if (flags & BufferKill_AlwaysKill){
            result = true;
            kill_file(system, models, file, string_zero());
        }
        else{
            if (vptr == 0){
                result = true;
                try_kill_file(system, models, file, vptr, string_zero());
            }
            else{
                app->print_message(app, literal("CUSTOM WARNING: the buffer is dirty and no view was specified for a dialogue."));
            }
        }
    }
    
    return(result);
}

internal void
internal_get_view_first(Command_Data *cmd, View_Summary *view){
    Editing_Layout *layout = &cmd->models->layout;
    Panel *panel = layout->used_sentinel.next;
    
    Assert(panel != &layout->used_sentinel);
    fill_view_summary(view, panel->view, cmd);
}

internal void
internal_get_view_next(Command_Data *cmd, View_Summary *view){
    Editing_Layout *layout = &cmd->models->layout;
    Live_Views *live_set = &cmd->vars->live_set;
    int index = view->view_id - 1;
    View *vptr = 0;
    Panel *panel = 0;
    
    if (index >= 0 && index < live_set->max){
        vptr = live_set->views + index;
        panel = vptr->panel;
        if (panel){
            panel = panel->next;
        }
        if (panel && panel != &layout->used_sentinel){
            fill_view_summary(view, panel->view, &cmd->vars->live_set, &cmd->models->working_set);
        }
        else{
            *view = view_summary_zero();
        }
    }
    else{
        *view = view_summary_zero();
    }
}

API_EXPORT View_Summary
Get_View_First(Application_Links *app, Access_Flag access)/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns the summary of the first view in a view loop.)
DOC
(
This call begins a loop across all the open views.

If the View_Summary returned is a null summary, the loop is finished.
Views should not be closed or opened durring a view loop.
)
DOC_SEE(Access_Flag)
DOC_SEE(get_view_next)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View_Summary view = {};
    
    internal_get_view_first(cmd, &view);
    while (view.exists && !access_test(view.lock_flags, access)){
        internal_get_view_next(cmd, &view);
    }
    
    return(view);
}

API_EXPORT void
Get_View_Next(Application_Links *app, View_Summary *view, Access_Flag access)/*
DOC_PARAM(view, The View_Summary pointed to by view is iterated to the next view or to a null summary if this is the last view.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access. The view outputted will be the next view that is accessible.)
DOC
(
This call steps a View_Summary to the next view in the global view order.

If the view outputted does not exist, the loop is finished.
Views should not be closed or opened durring a view loop.
)
DOC_SEE(Access_Flag)
DOC_SEE(get_view_first)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    
    internal_get_view_next(cmd, view);
    while (view->exists && !access_test(view->lock_flags, access)){
        internal_get_view_next(cmd, view);
    }
}

API_EXPORT View_Summary
Get_View(Application_Links *app, View_ID view_id, Access_Flag access)/*
DOC_PARAM(view_id, The view_id specifies the view to try to get.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated view if it is open and accessible.)
DOC_SEE(Access_Flag)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View_Summary view = {0};
    Live_Views *live_set = cmd->live_set;
    i32 max = live_set->max;
    View *vptr = 0;
    
    view_id -= 1;
    if (view_id >= 0 && view_id < max){
        vptr = live_set->views + view_id;
        fill_view_summary(&view, vptr, live_set, &cmd->models->working_set);
        if (!access_test(view.lock_flags, access)){
            view = view_summary_zero();
        }
    }
    
    return(view);
}

API_EXPORT View_Summary
Get_Active_View(Application_Links *app, Access_Flag access)/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the active view.)
DOC_SEE(set_active_view)
DOC_SEE(Access_Flag)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View_Summary view = {0};
    fill_view_summary(&view, cmd->view, &cmd->vars->live_set, &cmd->models->working_set);
    if (!access_test(view.lock_flags, access)){
        view = view_summary_zero();
    }
    return(view);
}

API_EXPORT View_Summary
Open_View(Application_Links *app, View_Summary *view_location, View_Split_Position position)/*
DOC_PARAM(view_location, The view_location parameter specifies the view to split to open the new view.)
DOC_PARAM(position, The position parameter specifies how to split the view and where to place the new view.)
DOC_RETURN(If this call succeeds it returns a View_Summary describing the newly created view, if it fails it
returns a null summary.)
DOC(4coder is built with a limit of 16 views.  If 16 views are already open when this is called the
call will fail.)
DOC_SEE(View_Split_Position)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    View *vptr = imp_get_view(cmd, view_location);
    Panel *panel = vptr->panel;
    View_Summary result = {0};
    
    if (models->layout.panel_count < models->layout.panel_max_count){
        b32 vsplit = ((position == ViewSplit_Left) || (position == ViewSplit_Right));
        b32 grtsplit = ((position == ViewSplit_Bottom) || (position == ViewSplit_Right));
        
        Split_Result split = layout_split_panel(&models->layout, panel, vsplit);
        
        Panel *grtpanel = split.panel;
        Panel *lsrpanel = panel;
        
        if (!grtsplit){
            Swap(i32, panel->which_child, split.panel->which_child);
            Swap(Panel*, grtpanel, lsrpanel);
        }
        
        split.panel->screen_region = panel->screen_region;
        if (vsplit){
            i32 x_pos = ROUND32(lerp((f32)lsrpanel->full.x0,
                                     split.divider->pos,
                                     (f32)lsrpanel->full.x1));
            
            grtpanel->full.x0 = x_pos;
            grtpanel->full.x1 = lsrpanel->full.x1;
            lsrpanel->full.x1 = x_pos;
        }
        else{
            i32 y_pos = ROUND32(lerp((f32)lsrpanel->full.y0,
                                     split.divider->pos,
                                     (f32)lsrpanel->full.y1));
            
            grtpanel->full.y0 = y_pos;
            grtpanel->full.y1 = lsrpanel->full.y1;
            lsrpanel->full.y1 = y_pos;
        }
        
        panel_fix_internal_area(panel);
        panel_fix_internal_area(split.panel);
        split.panel->prev_inner = split.panel->inner;
        
        models->layout.active_panel = (i32)(split.panel - models->layout.panels);
        panel_make_empty(system, cmd->vars, split.panel);
        
        fill_view_summary(&result, split.panel->view, cmd);
    }
    
    update_command_data(cmd->vars, cmd);
    
    return(result);
}

API_EXPORT bool32
Close_View(Application_Links *app, View_Summary *view)/*
DOC_PARAM(view, The view parameter specifies which view to close.)
DOC_RETURN(This call returns non-zero on success.)
DOC(
If the given view is open and is not the last view, it will be closed.
If the given view is the active view, the next active view in the global
order of view will be made active.
If the given view is the last open view in the system, the call will fail.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    View *vptr = imp_get_view(cmd, view);
    Panel *panel = vptr->panel;
    bool32 result = false;
    
    Divider_And_ID div, parent_div, child_div;
    i32 child;
    i32 parent;
    i32 which_child;
    i32 active;
    
    if (models->layout.panel_count > 1){
        live_set_free_view(system, models->live_set, vptr);
        panel->view = 0;
        
        div = layout_get_divider(&models->layout, panel->parent);
        
        // This divider cannot have two child dividers.
        Assert(div.divider->child1 == -1 || div.divider->child2 == -1);
        
        // Get the child who needs to fill in this node's spot
        child = div.divider->child1;
        if (child == -1) child = div.divider->child2;
        
        parent = div.divider->parent;
        which_child = div.divider->which_child;
        
        // Fill the child in the slot this node use to hold
        if (parent == -1){
            Assert(models->layout.root == div.id);
            models->layout.root = child;
        }
        else{
            parent_div = layout_get_divider(&models->layout, parent);
            if (which_child == -1){
                parent_div.divider->child1 = child;
            }
            else{
                parent_div.divider->child2 = child;
            }
        }
        
        // If there was a child divider, give it information about it's new parent.
        if (child != -1){
            child_div = layout_get_divider(&models->layout, child);
            child_div.divider->parent = parent;
            child_div.divider->which_child = div.divider->which_child;
        }
        
        // What is the new active panel?
        active = -1;
        if (child == -1){
            Panel *panel_ptr = 0;
            Panel *used_panels = &models->layout.used_sentinel;
            for (dll_items(panel_ptr, used_panels)){
                if (panel_ptr != panel && panel_ptr->parent == div.id){
                    panel_ptr->parent = parent;
                    panel_ptr->which_child = which_child;
                    active = (i32)(panel_ptr - models->layout.panels);
                    break;
                }
            }
        }
        else{
            Panel *panel_ptr = panel->next;
            if (panel_ptr == &models->layout.used_sentinel) panel_ptr = panel_ptr->next;
            Assert(panel_ptr != panel);
            active = (i32)(panel_ptr - models->layout.panels);
        }
        
        Assert(active != -1 && panel != models->layout.panels + active);
        models->layout.active_panel = active;
        
        layout_free_divider(&models->layout, div.divider);
        layout_free_panel(&models->layout, panel);
        layout_fix_all_panels(&models->layout);
    }
    
    return(result);
}

API_EXPORT bool32
Set_Active_View(Application_Links *app, View_Summary *view)/*
DOC_PARAM(view, The view parameter specifies which view to make active.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
If the given view is open, it is set as the
active view, and takes subsequent commands and is returned
from get_active_view.
)
DOC_SEE(get_active_view)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Models *models = cmd->models;
    View *vptr = imp_get_view(cmd, view);
    bool32 result = false;
    
    if (vptr){
        result = true;
        
        Panel *panel = vptr->panel;
        models->layout.active_panel = (i32)(panel - models->layout.panels);
        
        update_command_data(cmd->vars, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value)/*
DOC_PARAM(view, The view parameter specifies the view on which to set a setting.)
DOC_PARAM(setting, The setting parameter identifies the setting that shall be changed.)
DOC_PARAM(value, The value parameter specifies the value to which the setting shall be changed.)
DOC_RETURN(This call returns non-zero on success.)
DOC_SEE(View_Setting_ID)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    bool32 result = false;
    
    if (vptr){
        result = true;
        switch (setting){
            case ViewSetting_WrapLine:
            {
                Relative_Scrolling scrolling = view_get_relative_scrolling(vptr);
                if (value){
                    if (vptr->file_data.unwrapped_lines){
                        vptr->file_data.unwrapped_lines = 0;
                        vptr->edit_pos->scroll.target_x = 0;
                        view_cursor_move(vptr, vptr->edit_pos->cursor.pos);
                        view_set_relative_scrolling(vptr, scrolling);
                    }
                }
                else{
                    if (!vptr->file_data.unwrapped_lines){
                        vptr->file_data.unwrapped_lines = 1;
                        view_cursor_move(vptr, vptr->edit_pos->cursor.pos);
                        view_set_relative_scrolling(vptr, scrolling);
                    }
                }
            }break;
            
            case ViewSetting_ShowWhitespace:
            {
                vptr->file_data.show_whitespace = value;
            }break;
            
            case ViewSetting_ShowScrollbar:
            {
                vptr->hide_scrollbar = !value;
            }break;
            
            default:
            {
                result = false;
            }break;
        }
        
        fill_view_summary(view, vptr, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Split_Proportion(Application_Links *app, View_Summary *view, float t)/*
DOC_PARAM(view, The view parameter specifies which view shall have it's size adjusted.)
DOC_PARAM(t, The t parameter specifies the proportion of the containing box that the view should occupy. t should be in [0,1].)
DOC_RETURN(This call returns non-zero on success.)
*/{
    bool32 result = false;
    
    if (0 <= t && t <= 1.f){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Models *models = cmd->models;
        Editing_Layout *layout = &models->layout;
        View *vptr = imp_get_view(cmd, view);
        
        if (vptr){
            result = true;
            
            Panel *panel = vptr->panel;
            Panel_Divider *div = layout->dividers + panel->parent;
            
            if (panel->which_child == 1){
                t = 1-t;
            }
            
            div->pos = t;
            layout_fix_all_panels(layout);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Compute_Cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out)/*
DOC_PARAM(view, The view parameter specifies the view on which to run the cursor computation.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(cursor_out, On success this struct is filled with the result of the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(Computes a Full_Cursor for the given seek position with no side effects.)
DOC_SEE(Buffer_Seek)
DOC_SEE(Full_Cursor)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    Editing_File *file = 0;
    bool32 result = false;
    
    if (vptr){
        file = vptr->file_data.file;
        if (file && !file->is_loading){
            if (seek.type == buffer_seek_line_char && seek.character <= 0){
                seek.character = 1;
            }
            result = true;
            *cursor_out = view_compute_cursor(vptr, seek);
            fill_view_summary(view, vptr, cmd);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x)/*
DOC_PARAM(view, The view parameter specifies the view in which to set the cursor.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(set_preferred_x, If this parameter is true the preferred x is updated to match the new cursor x.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
This call sets the the view's cursor position.  set_preferred_x should usually be true
unless the change in cursor position is is a vertical motion that tries to keep the
cursor in the same column or x position.
)
DOC_SEE(Buffer_Seek)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    Editing_File *file = 0;
    bool32 result = false;
    
    if (vptr){
        file = vptr->file_data.file;
        if (file && !file->is_loading){
            result = true;
            if (seek.type == buffer_seek_line_char && seek.character <= 0){
                seek.character = 1;
            }
            Full_Cursor cursor = view_compute_cursor(vptr, seek);
            view_set_cursor(vptr, cursor, set_preferred_x,
                            vptr->file_data.unwrapped_lines);
            fill_view_summary(view, vptr, cmd);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll)/*
DOC(TODO)
DOC_SEE(GUI_Scroll_Vars)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    Editing_File *file = 0;
    bool32 result = false;
    
    if (vptr){
        file = vptr->file_data.file;
        if (file && !file->is_loading){
            result = true;
            view_set_scroll(vptr, scroll);
            fill_view_summary(view, vptr, cmd);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Mark(Application_Links *app, View_Summary *view, Buffer_Seek seek)/*
DOC_PARAM(view, The view parameter specifies the view in which to set the mark.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(This call sets the the view's mark position.)
DOC_SEE(Buffer_Seek)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    Editing_File *file = 0;
    Full_Cursor cursor = {0};
    bool32 result = false;
    
    if (vptr){
        file = vptr->file_data.file;
        if (file && !file->is_loading){
            result = true;
            if (seek.type != buffer_seek_pos){
                cursor = view_compute_cursor(vptr, seek);
                vptr->edit_pos->mark = cursor.pos;
            }
            else{
                vptr->edit_pos->mark = seek.pos;
            }
            fill_view_summary(view, vptr, cmd);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Highlight(Application_Links *app, View_Summary *view, int32_t start, int32_t end, bool32 turn_on)/*
DOC_PARAM(view, The view parameter specifies the view in which to set the highlight.)
DOC_PARAM(start, This parameter specifies the absolute position of the first character of the highlight range.)
DOC_PARAM(end, This parameter specifies the absolute position of the character one past the end of the highlight range.)
DOC_PARAM(turn_on, This parameter indicates whether the highlight is being turned on or off.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
The highlight is mutually exclusive to the cursor.  When the turn_on parameter
is set to true the highlight will be shown and the cursor will be hidden.  After
that either setting the with view_set_cursor or calling view_set_highlight and
the turn_on set to false, will switch back to showing the cursor.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    bool32 result = false;
    
    if (vptr){
        result = true;
        if (turn_on){
            view_set_temp_highlight(vptr, start, end);
        }
        else{
            vptr->file_data.show_temp_highlight = 0;
        }
        fill_view_summary(view, vptr, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags)/*
DOC_PARAM(view, The view parameter specifies the view in which to display the buffer.)
DOC_PARAM(buffer_id, The buffer_id parameter specifies which buffer to show in the view.)
DOC_PARAM(flags, The flags parameter specifies behaviors for setting the buffer.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
On success view_set_buffer sets the specified view's current buffer and
cancels and dialogue shown in the view and displays the file.
)
DOC_SEE(Set_Buffer_Flag)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    Models *models = cmd->models;
    Editing_File *file = 0;
    bool32 result = false;
    
    if (vptr){
        file = working_set_get_active_file(&models->working_set, buffer_id);
        
        if (file){
            result = true;
            if (file != vptr->file_data.file){
                view_set_file(vptr, file, models);
                if (!(flags & SetBuffer_KeepOriginalGUI)){
                    view_show_file(vptr);
                }
            }
        }
        
        fill_view_summary(view, vptr, cmd);
    }
    
    return(result);
}

API_EXPORT bool32
View_Post_Fade(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color)/*
DOC_PARAM(view, The view parameter specifies the view onto which the fade effect shall be posted.)
DOC_PARAM(seconds, This parameter specifies the number of seconds the fade effect should last.)
DOC_PARAM(start, This parameter specifies the absolute position of the first character of the fade range.)
DOC_PARAM(end, This parameter specifies the absolute position of the character one past the end of the fdae range.)
DOC_PARAM(color, The color parameter specifies the initial color of the text before it fades to it's natural color.)
DOC_RETURN(This call returns non-zero on success.)
DOC_SEE(int_color)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr = imp_get_view(cmd, view);
    
    bool32 result = false;
    
    int size = end - start;
    if (vptr){
        if (size > 0){
            result = true;
            view_post_paste_effect(vptr, seconds, start, size, color | 0xFF000000);
        }
    }
    
    return(result);
}

API_EXPORT User_Input
Get_User_Input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)/*
DOC_PARAM(get_type, The get_type parameter specifies the set of input types that should be returned.)
DOC_PARAM(abort_type, The get_type parameter specifies the set of input types that should trigger an abort signal.)
DOC_RETURN(This call returns a User_Input that describes a user input event.)
DOC
(
This call preempts the command. The command is resumed if either a get or abort condition
is met, or if another command is executed.  If either the abort condition is met or another
command is executed an abort signal is returned.  If an abort signal is ever returned the
command should finish execution without any more calls that preempt the command.
If a get condition is met the user input is returned.
)
DOC_SEE(Input_Type_Flag)
DOC_SEE(User_Input)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Coroutine *coroutine = (Coroutine*)app->current_coroutine;
    User_Input result = {0};
    
    if (app->type_coroutine == Co_Command){
        Assert(coroutine);
        *((u32*)coroutine->out+0) = get_type;
        *((u32*)coroutine->out+1) = abort_type;
        system->yield_coroutine(coroutine);
        result = *(User_Input*)coroutine->in;
    }
    
    return(result);
}

API_EXPORT User_Input
Get_Command_Input (Application_Links *app)/*
DOC_RETURN(This call returns the input that triggered the currently executing command.)
DOC_SEE(User_Input)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    User_Input result;
    
    result.type = UserInputKey;
    result.abort = 0;
    result.key = cmd->key;
    // TODO(allen): It would be nice to fill this.
    result.command.cmdid = 0;
    
    return(result);
}

API_EXPORT Mouse_State
Get_Mouse_State(Application_Links *app)/*
DOC_RETURN(This call returns the current mouse state as of the beginning of the frame.)
DOC_SEE(Mouse_State)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    App_Vars *vars = cmd->vars;
    Mouse_State mouse = direct_get_mouse_state(&vars->available_input);
    return(mouse);
}

/*
API_EXPORT Event_Message
Get_Event_Message (Application_Links *app){
    Event_Message message = {0};
    System_Functions *system = (System_Functions*)app->system_links;
    Coroutine *coroutine = (Coroutine*)app->current_coroutine;
    
    if (app->type_coroutine == Co_View){
        Assert(coroutine);
        system->yield_coroutine(coroutine);
        message = *(Event_Message*)coroutine->in;
    }
    
    return(message);
}
*/

API_EXPORT bool32
Start_Query_Bar(Application_Links *app, Query_Bar *bar, uint32_t flags)/*
DOC_PARAM(bar, This parameter provides a Query_Bar that should remain in valid memory
until end_query_bar or the end of the command.  It is commonly a good idea to make
this a pointer to a Query_Bar stored on the stack.)
DOC_PARAM(flags, This parameter is not currently used and should be 0 for now.)
DOC_RETURN(This call returns non-zero on success.)
DOC
(
This call tells the active view to begin displaying a "Query_Bar" which is a small
GUI element that can overlap a buffer or other 4coder GUI.  The contents of the bar
can be changed after the call to start_query_bar and the query bar shown by 4coder
will reflect the change.  Since the bar stops showing when the command exits the
only use for this call is in an interactive command that makes calls to get_user_input.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Query_Slot *slot = 0;
    View *vptr;
    
    vptr = cmd->view;
    
    slot = alloc_query_slot(&vptr->query_set);
    slot->query_bar = bar;
    
    bool32 result = (slot != 0);
    return(result);
}

API_EXPORT void
End_Query_Bar(Application_Links *app, Query_Bar *bar, uint32_t flags)/*
DOC_PARAM(bar, This parameter should be a bar pointer of a currently active query bar.)
DOC_PARAM(flags, This parameter is not currently used and should be 0 for now.)
DOC(Stops showing the particular query bar specified by the bar parameter.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr;
    vptr = cmd->view;
    free_query_slot(&vptr->query_set, bar);
}

API_EXPORT void
Print_Message(Application_Links *app, char *str, int32_t len)/*
DOC_PARAM(str, The str parameter specifies the string to post to *messages*; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the str string.)
DOC(This call posts a string to the *messages* buffer.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Models *models = cmd->models;
    do_feedback_message(cmd->system, models, make_string(str, len));
}

// TODO(allen): List the names of built in themes and fonts.

API_EXPORT void
Change_Theme(Application_Links *app, char *name, int32_t len)/*
DOC_PARAM(name, The name parameter specifies the name of the theme to begin using; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC(This call changes 4coder's color pallet to one of the built in themes.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Style_Library *styles = &cmd->models->styles;
    String theme_name = make_string(name, len);
    
    i32 i = 0;
    i32 count = styles->count;
    Style *s = styles->styles;
    
    for (i = 0; i < count; ++i, ++s){
        if (match(s->name, theme_name)){
            style_copy(main_style(cmd->models), s);
            break;
        }
    }
}

API_EXPORT void
Change_Font(Application_Links *app, char *name, int32_t len, bool32 apply_to_all_files)/*
DOC_PARAM(name, The name parameter specifies the name of the font to begin using; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC_PARAM(apply_to_all_files, If this is set all open files change to this font.  Usually this should be true
durring the start hook because several files already exist at that time.)
DOC(This call changes 4coder's default font to one of the built in fonts.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Font_Set *set = cmd->models->font_set;
    
    Style_Font *global_font = &cmd->models->global_font;
    String font_name = make_string(name, len);
    i16 font_id = 0;
    
    if (font_set_extract(set, font_name, &font_id)){
        if (apply_to_all_files){
            global_set_font(cmd->system, cmd->models, font_id);
        }
        else{
            global_font->font_id = font_id;
        }
    }
}

API_EXPORT void
Buffer_Set_Font(Application_Links *app, Buffer_Summary *buffer, char *name, int32_t len)/*
DOC_PARAM(buffer, This parameter the buffer that shall have it's font changed)
DOC_PARAM(name, The name parameter specifies the name of the font to begin using; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC(This call sets the display font of a particular buffer.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    Editing_File *file = imp_get_file(cmd, buffer);
    
    Font_Set *set = models->font_set;
    String font_name = make_string(name, len);
    i16 font_id = 0;
    
    if (font_set_extract(set, font_name, &font_id)){
        file_set_font(system, models, file, font_id);
    }
}

API_EXPORT void
Set_Theme_Colors(Application_Links *app, Theme_Color *colors, int32_t count)/*
DOC_PARAM(colors, The colors pointer provides an array of color structs pairing differet style tags to color codes.)
DOC_PARAM(count, The count parameter specifies the number of Theme_Color structs in the colors array.)
DOC
(
For each struct in the array, the slot in the main color pallet specified by the
struct's tag is set to the color code in the struct. If the tag value is invalid
no change is made to the color pallet.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Style *style = main_style(cmd->models);
    
    int_color *color = 0;
    i32 i = 0;
    Theme_Color *theme_color = colors;
    
    for (i = 0; i < count; ++i, ++theme_color){
        color = style_index_by_tag(&style->main, theme_color->tag);
        if (color){
            *color = theme_color->color | 0xFF000000;
        }
    }
}

API_EXPORT void
Get_Theme_Colors(Application_Links *app, Theme_Color *colors, int32_t count)/*
DOC_PARAM(colors, an array of color structs listing style tags to get color values for)
DOC_PARAM(count, the number of color structs in the colors array)
DOC
(
For each struct in the array, the color field of the struct is filled with the
color from the slot in the main color pallet specified by the tag.  If the tag
value is invalid the color is filled with black.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Style *style = main_style(cmd->models);
    
    u32 *color = 0;
    i32 i = 0;
    Theme_Color *theme_color = colors;
    
    for (i = 0; i < count; ++i, ++theme_color){
        color = style_index_by_tag(&style->main, theme_color->tag);
        if (color){
            theme_color->color = *color | 0xFF000000;
        }
        else{
            theme_color->color = 0xFF000000;
        }
    }
}

API_EXPORT int32_t
Directory_Get_Hot(Application_Links *app, char *out, int32_t capacity)/*
DOC_PARAM(out, This parameter provides a character buffer that receives the 4coder 'hot directory'.)
DOC_PARAM(capacity, This parameter specifies the maximum size to be output to the out buffer.)
DOC_RETURN(This call returns the size of the string written into the buffer.)
DOC
(
4coder has a concept of a 'hot directory' which is the directory most recently
accessed in the GUI.  Whenever the GUI is opened it shows the hot directory.

In the future this will be deprecated and eliminated in favor of more flexible
directories controlled on the custom side.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Hot_Directory *hot = &cmd->models->hot_directory;
    i32 copy_max = capacity - 1;
    hot_directory_clean_end(hot);
    if (copy_max > hot->string.size)
        copy_max = hot->string.size;
    memcpy(out, hot->string.str, copy_max);
    out[copy_max] = 0;
    return(hot->string.size);
}

#define Memory_Allocate system->memory_allocate
#define Memory_Set_Protection system->memory_set_protection
#define Memory_Free system->memory_free

#define Get_4ed_Path system->get_4ed_path
#define File_Exists system->file_exists
#define Directory_CD system->directory_cd
#define Show_Mouse_Cursor system->show_mouse_cursor

API_EXPORT File_List
Get_File_List(Application_Links *app, char *dir, int32_t len)/*
DOC_PARAM(dir, This parameter specifies the directory whose files will be enumerated in the returned list; it need not be null terminated.)
DOC_PARAM(len, This parameter the length of the dir string.)
DOC_RETURN
(
This call returns a File_List struct containing pointers to the names of the files in
the specified directory.  The File_List returned should be passed to free_file_list
when it is no longer in use.
)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    File_List result = {};
    system->set_file_list(&result, make_string(dir, len));
    return(result);
}

API_EXPORT void
Free_File_List(Application_Links *app, File_List list)/*
DOC_PARAM(list, This parameter provides the file list to be freed.)
DOC(After this call the file list passed in should not be read or written to.)
*/{
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    system->set_file_list(&list, make_string(0, 0));
}

// BOTTOM

