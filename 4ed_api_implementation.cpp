/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.????
 *
 * Implementation of the API functions.
 *
 */

// TOP

#define API_EXPORT

internal b32
access_test(u32 lock_flags, u32 access_flags){
    b32 result = ((lock_flags & ~access_flags) == 0);
    return(result);
}

internal void
fill_buffer_summary(Buffer_Summary *buffer, Editing_File *file, Working_Set *working_set){
    memset(buffer, 0, sizeof(*buffer));
    if (!file->is_dummy){
        buffer->exists = 1;
        buffer->ready = file_is_ready(file);
        
        buffer->buffer_id = file->id.id;
        buffer->size = buffer_size(&file->state.buffer);
        buffer->line_count = file->state.buffer.line_count;
        
        buffer->file_name_len = file->canon.name.size;
        buffer->file_name = file->canon.name.str;
        
        buffer->buffer_name_len = file->unique_name.name.size;
        buffer->buffer_name = file->unique_name.name.str;
        
        buffer->dirty = file->state.dirty;
        
        buffer->is_lexed = file->settings.tokens_exist;
        
        buffer->tokens_are_ready = (file->state.token_array.tokens && file->state.tokens_complete && !file->state.still_lexing);
        buffer->map_id = file->settings.base_map_id;
        buffer->unwrapped_lines = file->settings.unwrapped_lines;
        
        buffer->lock_flags = 0;
        if (file->settings.read_only){
            buffer->lock_flags |= AccessProtected;
        }
    }
}

internal void
fill_view_summary(System_Functions *system, View_Summary *view, View *vptr, Live_Views *live_set, Working_Set *working_set){
    File_Viewing_Data *data = &vptr->transient.file_data;
    
    memset(view, 0, sizeof(*view));
    
    if (vptr->transient.in_use){
        view->exists = true;
        view->view_id = (int32_t)(vptr - live_set->views) + 1;
        view->line_height = (f32)(vptr->transient.line_height);
        view->unwrapped_lines = data->file->settings.unwrapped_lines;
        view->show_whitespace = data->show_whitespace;
        view->lock_flags = view_lock_flags(vptr);
        
        view->buffer_id = vptr->transient.file_data.file->id.id;
        
        Assert(data->file != 0);
        File_Edit_Positions edit_pos = view_get_edit_pos(vptr);
        
        view->mark    = file_compute_cursor(system, data->file, seek_pos(edit_pos.mark));
        view->cursor  = file_compute_cursor(system, data->file, seek_pos(edit_pos.cursor_pos));
        
        view->preferred_x = edit_pos.preferred_x;
        
        view->view_region = vptr->transient.panel->rect_inner;
        view->file_region = vptr->transient.file_region;
        if (vptr->transient.ui_mode){
            view->scroll_vars = vptr->transient.ui_scroll;
        }
        else{
            view->scroll_vars = edit_pos.scroll;
        }
    }
}

internal void
fill_view_summary(System_Functions *system, View_Summary *view, View *vptr, Models *models){
    fill_view_summary(system, view, vptr, &models->live_set, &models->working_set);
}

internal void
view_quit_ui(System_Functions *system, Models *models, View *view){
    Assert(view != 0);
    view->transient.ui_mode = false;
    if (view->transient.ui_quit != 0){
        View_Summary view_summary = {};
        fill_view_summary(system, &view_summary, view, models);
        view->transient.ui_quit(&models->app_links, view_summary);
    }
}

internal Editing_File*
get_file_from_identifier(System_Functions *system, Working_Set *working_set, Buffer_Identifier buffer){
    Editing_File *file = 0;
    if (buffer.id){
        file = working_set_get_active_file(working_set, buffer.id);
    }
    else if (buffer.name != 0){
        String name = make_string(buffer.name, buffer.name_len);
        file = working_set_contains_name(working_set, name);
    }
    return(file);
}

internal Editing_File*
imp_get_file(Models *models, Buffer_ID buffer_id){
    Working_Set *working_set = &models->working_set;
    Editing_File *file = working_set_get_active_file(working_set, buffer_id);
    if (file != 0 && !file_is_ready(file)){
        file = 0;
    }
    return(file);
}

internal Editing_File*
imp_get_file(Models *models, Buffer_Summary *buffer){
    Editing_File *file = 0;
    if (buffer != 0 && buffer->exists){
        file = imp_get_file(models, buffer->buffer_id);
    }
    return(file);
}

internal View*
imp_get_view(Models *models, View_ID view_id){
    Live_Views *live_set = &models->live_set;
    View *vptr = 0;
    view_id = view_id - 1;
    if (0 <= view_id && view_id < live_set->max){
        vptr = live_set->views + view_id;
        if (!vptr->transient.in_use){
            vptr = 0;
        }
    }
    return(vptr);
}

internal View*
imp_get_view(Models *models, View_Summary *view){
    View *vptr = 0;
    if (view != 0 && view->exists){
        vptr = imp_get_view(models, view->view_id);
    }
    return(vptr);
}

API_EXPORT bool32
Global_Set_Setting(Application_Links *app, Global_Setting_ID setting, int32_t value)
/*
DOC_PARAM(setting, Which setting to change.)
DOC_PARAM(value, The new value to set ont he specified setting.)
DOC_SEE(Global_Setting_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    
    b32 result = true;
    switch (setting){
        case GlobalSetting_LAltLCtrlIsAltGr:
        {
            models->settings.lctrl_lalt_is_altgr = value;
        }break;
        
        default:
        {
            result = false;
        }break;
    }
    
    return(result);
}

API_EXPORT bool32
Global_Set_Mapping(Application_Links *app, void *data, int32_t size)
/*
DOC_PARAM(data, The beginning of a binding buffer.  Bind_Helper is designed to make it easy to produce such a buffer.)
DOC_PARAM(size, The size of the binding buffer in bytes.)
DOC_RETURN(Returns non-zero if no errors occurred while interpretting the binding buffer.  A return value of zero does not indicate that the old mappings are still in place.)
DOC(Dumps away the previous mappings and instantiates the mappings described in the binding buffer.  If any of the open buffers were bound to a command map that used to exist, but no command map with the same id exist after the new mappings are instantiated, the buffer's command map will be set to mapid_file and a warning will be posted to *messages*.)
*/{
    Models *models = (Models*)app->cmd_context;
    bool32 result = interpret_binding_buffer(models, data, size);
    return(result);
}

API_EXPORT bool32
Exec_System_Command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer_id, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags)
/*
DOC_PARAM(view, If the view parameter is non-null it specifies a view to display the command's output buffer, otherwise the command will still work but if there is a buffer capturing the output it will not automatically be displayed.)
DOC_PARAM(buffer_id, The buffer the command will output to is specified by the buffer parameter. See Buffer_Identifier for information on how this type specifies a buffer.  If output from the command should just be ignored, then buffer_identifier(0) can be specified to indicate no output buffer.)
DOC_PARAM(path, The path parameter specifies the current working directory in which the command shall be executed. The string need not be null terminated.)
DOC_PARAM(path_len, The parameter path_len specifies the length of the path string.)
DOC_PARAM(command, The command parameter specifies the command that shall be executed. The string need not be null terminated.)
DOC_PARAM(command_len, The parameter command_len specifies the length of the command string.)
DOC_PARAM(flags, Flags for the behavior of the call are specified in the flags parameter.)
DOC_RETURN(This call returns non-zero on success.)
DOC(A call to exec_system_command executes a command as if called from the command line, and sends the output to a buffer. To output to an existing buffer, the buffer identifier can name a new buffer that does not exist, or provide the id of a buffer that does exist.  If the buffer identifier uses a name for a buffer that does not exist, the buffer is created and used. If the buffer identifier uses an id that does not belong to an open buffer, then no buffer is used.

If there in an output buffer and it is not already in an open view and the view parameter is not NULL, then the provided view will display the output buffer.

If the view parameter is NULL, no view will switch to the output.)
DOC_SEE(Buffer_Identifier)
DOC_SEE(Command_Line_Interface_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    App_Vars *vars = models->vars;
    Partition *part = &models->mem.part;
    Heap *heap = &models->mem.heap;
    Working_Set *working_set = &models->working_set;
    
    bool32 result = true;
    char feedback_space[256];
    String feedback_str = make_fixed_width_string(feedback_space);
    
    Temp_Memory temp = begin_temp_memory(part);
    
    {
        // NOTE(allen): Check that it is possible to store a new child process.
        if (!cli_list_has_space(&vars->cli_processes)){
            append(&feedback_str, make_lit_string("ERROR: no available process slot\n"));
            result = false;
            goto done;
        }
        
        // NOTE(allen): Try to get the buffer that was specified if it exists.
        Editing_File *file = get_file_from_identifier(system, working_set, buffer_id);
        
        // NOTE(allen): If the file exists check that it is legal.
        if (file != 0){
            if (file->settings.read_only == 0){
                append(&feedback_str, make_lit_string("ERROR: "));
                append(&feedback_str, file->unique_name.name);
                append(&feedback_str, make_lit_string(" is not a read-only buffer\n"));
                result = false;
                goto done;
            }
            if (file->settings.never_kill){
                append(&feedback_str, make_lit_string("ERROR: The buffer "));
                append(&feedback_str, file->unique_name.name);
                append(&feedback_str, make_lit_string(" is not killable"));
                result = false;
                goto done;
            }
        }
        
        // NOTE(allen): If the buffer is specified by name but does not already exist, then create it.
        if (file == 0 && buffer_id.name != 0){
            file = working_set_alloc_always(working_set, heap, &models->lifetime_allocator);
            Assert(file != 0);
            
            String name = push_string(part, buffer_id.name, buffer_id.name_len);
            buffer_bind_name(models, heap, part, working_set, file, name);
            init_read_only_file(system, models, file);
        }
        
        // NOTE(allen): If there are conflicts in output buffer with an existing child process resolve it.
        if (file != 0){
            CLI_List *list = &vars->cli_processes;
            CLI_Process *proc_ptr = list->procs;
            for (u32 i = 0; i < list->count; ++i, ++proc_ptr){
                if (proc_ptr->out_file == file){
                    if (flags & CLI_OverlapWithConflict){
                        proc_ptr->out_file = 0;
                    }
                    else{
                        file = 0;
                    }
                    break;
                }
            }
            
            if (file == 0){
                append(&feedback_str, "did not begin command-line command because the target buffer is already in use\n");
                result = false;
                goto done;
            }
        }
        
        // NOTE(allen): If we have an output file, prepare it for child proc output.
        if (file != 0){
            edit_clear(system, models, file);
            file_set_unimportant(file, true);
        }
        
        // NOTE(allen): If we have an output file and we need to bring it up in a new view, do so.
        if (file != 0){
            b32 bind_to_new_view = true;
            if (!(flags & CLI_AlwaysBindToView)){
                if (file_is_viewed(&models->layout, file)){
                    bind_to_new_view = false;
                }
            }
            
            if (bind_to_new_view){
                View *vptr = imp_get_view(models, view);
                if (vptr != 0){
                    view_set_file(system, models, vptr, file);
                    view_quit_ui(system, models, vptr);
                }
            }
        }
        
        // NOTE(allen): Figure out the root path for the command.
        String path_string = {};
        if (path == 0){
            terminate_with_null(&models->hot_directory.string);
            path_string = models->hot_directory.string;
        }
        else{
            path_string = push_string(part, path, path_len);
        }
        
        // NOTE(allen): Figure out the command string.
        String command_string = {};
        if (command == 0){
            command_string = make_lit_string(" echo no script specified");
        }
        else{
            command_string = push_string(part, command, command_len);
        }
        
        // NOTE(allen): Attept to execute the command.
        char *path_str = path_string.str;
        char *command_str = command_string.str;
        b32 cursor_at_end = ((flags & CLI_CursorAtEnd) != 0);
        if (!cli_list_call(system, &vars->cli_processes, path_str, command_str, file, cursor_at_end)){
            append(&feedback_str, "ERROR: Failed to make the cli call\n");
            result = false;
        }
    }
    
    done:;
    if (!result){
        print_message(app, feedback_str.str, feedback_str.size);
    }
    
    end_temp_memory(temp);
    return(result);
}

API_EXPORT void
Clipboard_Post(Application_Links *app, int32_t clipboard_id, char *str, int32_t len)
/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC_PARAM(str, The str parameter specifies the string to be posted to the clipboard, it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the str string.)
DOC(Stores the string str in the clipboard initially with index 0. Also reports the copy to the operating system, so that it may be pasted into other applications.)
DOC_SEE(The_4coder_Clipboard)
*/{
    Models *models = (Models*)app->cmd_context;
    String *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, len);
    copy(dest, make_string(str, len));
    models->system->post_clipboard(*dest);
}

API_EXPORT int32_t
Clipboard_Count(Application_Links *app, int32_t clipboard_id)
/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC(This call returns the number of items in the clipboard.)
DOC_SEE(The_4coder_Clipboard)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working = &models->working_set;
    int32_t count = working->clipboard_size;
    return(count);
}

API_EXPORT int32_t
Clipboard_Index(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len)
/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC_PARAM(item_index, This parameter specifies which item to read, 0 is the most recent copy, 1 is the second most recent copy, etc.)
DOC_PARAM(out, This parameter provides a buffer where the clipboard contents are written.  This parameter may be NULL.)
DOC_PARAM(len, This parameter specifies the length of the out buffer.)
DOC_RETURN(This call returns the size of the item associated with item_index.)

DOC(This function always returns the size of the item even if the output buffer is NULL. If the output buffer is too small to contain the whole string, it is filled with the first len character of the clipboard contents.  The output string is not null terminated.)

DOC_SEE(The_4coder_Clipboard)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working = &models->working_set;
    int32_t size = 0;
    String *str = working_set_clipboard_index(working, item_index);
    if (str != 0){
        size = str->size;
        if (out != 0){
            String out_str = make_string_cap(out, 0, len);
            copy(&out_str, *str);
        }
    }
    return(size);
}

API_EXPORT Parse_Context_ID
Create_Parse_Context(Application_Links *app, Parser_String_And_Type *kw, uint32_t kw_count, Parser_String_And_Type *pp, uint32_t pp_count)
/*
DOC_PARAM(kw, The list of keywords and type of each.)
DOC_PARAM(kw_count, The number of keywords in the list.)
DOC_PARAM(pp, The list of preprocessor directives and the type of each.)
DOC_PARAM(pp_count, The number of preprocessor directives in the list.)
DOC_RETURN(On success returns an id for the new parse context.  If id == 0, then the maximum number of parse contexts has been reached.)
*/{
    Models *models = (Models*)app->cmd_context;
    Parse_Context_ID id = parse_context_add(&models->parse_context_memory, &models->mem.heap, kw, kw_count, pp, pp_count);
    return(id);
}

API_EXPORT int32_t
Get_Buffer_Count(Application_Links *app)
/*
DOC(Gives the total number of buffers in the application.)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    int32_t result = working_set->file_count;
    return(result);
}

internal void
internal_get_buffer_first(Working_Set *working_set, Buffer_Summary *buffer){
    if (working_set->file_count > 0){
        Node *node = working_set->used_sentinel.next;
        Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
        fill_buffer_summary(buffer, file, working_set);
    }
}

internal void
get_buffer_next__internal(Working_Set *working_set, Buffer_Summary *buffer){
    Editing_File *file = working_set_get_active_file(working_set, buffer->buffer_id);
    if (file != 0){
        file = CastFromMember(Editing_File, main_chain_node, file->main_chain_node.next);
        Editing_File *sentinel_file_ptr = CastFromMember(Editing_File, main_chain_node, &working_set->used_sentinel);
        if (file != sentinel_file_ptr){
            fill_buffer_summary(buffer, file, working_set);
        }
        else{
            memset(buffer, 0, sizeof(*buffer));
        }
    }
    else{
        memset(buffer, 0, sizeof(*buffer));
    }
}

API_EXPORT Buffer_Summary
Get_Buffer_First(Application_Links *app, Access_Flag access)
/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns the summary of the first buffer in a buffer loop.)
DOC(
This call begins a loop across all the buffers.
If the buffer returned does not exist, the loop is finished.
Buffers should not be killed durring a buffer loop.
)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
DOC_SEE(get_buffer_next)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Buffer_Summary buffer = {};
    internal_get_buffer_first(working_set, &buffer);
    for (;buffer.exists && !access_test(buffer.lock_flags, access);){
        get_buffer_next__internal(working_set, &buffer);
    }
    return(buffer);
}

API_EXPORT void
Get_Buffer_Next(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access)
/*
DOC_PARAM(buffer, The Buffer_Summary pointed to by buffer is iterated to the next buffer or to a null summary if this is the last buffer.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access. The buffer outputted will be the next buffer that is accessible.)
DOC(
This call steps a Buffer_Summary to the next buffer in the global buffer order.
The global buffer order is kept roughly in the order of most recently used to least recently used.

If the buffer outputted does not exist, the loop is finished.
Buffers should not be killed or created durring a buffer loop.
)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
DOC_SEE(get_buffer_first)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    get_buffer_next__internal(working_set, buffer);
    for (;buffer->exists && !access_test(buffer->lock_flags, access);){
        get_buffer_next__internal(working_set, buffer);
    }
}

API_EXPORT Buffer_Summary
Get_Buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
/*
DOC_PARAM(buffer_id, The parameter buffer_id specifies which buffer to try to get.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
DOC_SEE(Buffer_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Buffer_Summary buffer = {};
    Editing_File *file = working_set_get_active_file(working_set, buffer_id);
    if (file != 0 && !file->is_dummy){
        fill_buffer_summary(&buffer, file, working_set);
        if (!access_test(buffer.lock_flags, access)){
            memset(&buffer, 0, sizeof(buffer));
        }
    }
    return(buffer);
}

API_EXPORT Buffer_Summary
Get_Buffer_By_Name(Application_Links *app, char *name, int32_t len, Access_Flag access)
/*
DOC_PARAM(name, The name parameter specifies the buffer name to try to get. The string need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)

DOC(This call searches the buffers by their buffer name.  The buffer name is the short name in the file bar.  The name must match exactly including any alterations put on the buffer name to avoid duplicates.)

DOC_SEE(get_buffer_by_file_name)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Buffer_Summary buffer = {};
    Editing_File *file = working_set_contains_name(working_set, make_string(name, len));
    if (file != 0 && !file->is_dummy){
        fill_buffer_summary(&buffer, file, working_set);
        if (!access_test(buffer.lock_flags, access)){
            memset(&buffer, 0, sizeof(buffer));
        }
    }
    return(buffer);
}

API_EXPORT Buffer_Summary
Get_Buffer_By_File_Name(Application_Links *app, char *name, int32_t len, Access_Flag access)
/*
DOC_PARAM(name, The name parameter specifies the buffer name to try to get. The string need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)

DOC(This call searches the buffers by their canonicalized file names.  Not all buffers have file names, only buffers that are tied to files.  For instance *scratch* does not have a file name.  Every file has one canonicalized file name.  For instance on windows this involves converting w:/a/b into W:\a\b.  If the name passed is not canonicalized a canonicalized copy is made first.  This includes turning relative paths to files that exist into full paths.  So the passed in name can be relative to the working directory.)

DOC_SEE(get_buffer_by_name)
DOC_SEE(Buffer_Summary)
DOC_SEE(Access_Flag)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Working_Set *working_set = &models->working_set;
    Buffer_Summary buffer = {};
    String fname = make_string(name, len);
    Editing_File_Name canon = {};
    if (get_canon_name(system, fname, &canon)){
        Editing_File *file = working_set_contains_canon(working_set, canon.name);
        if (file != 0){
            fill_buffer_summary(&buffer, file, working_set);
            if (!access_test(buffer.lock_flags, access)){
                memset(&buffer, 0, sizeof(buffer));
            }
        }
    }
    return(buffer);
}

API_EXPORT bool32
Buffer_Read_Range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *out)
/*
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
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    int32_t size = 0;
    if (file != 0){
        size = buffer_size(&file->state.buffer);
        if (0 <= start && start <= end && end <= size){
            result = true;
            buffer_stringify(&file->state.buffer, start, end, out);
        }
        fill_buffer_summary(buffer, file, &models->working_set);
    }
    return(result);
}

API_EXPORT bool32
Buffer_Replace_Range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *str, int32_t len)
/*
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
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    int32_t size = 0;
    if (file != 0){
        size = buffer_size(&file->state.buffer);
        if (0 <= start && start <= end && end <= size){
            Edit edit = {};
            edit.str = str;
            edit.length = len;
            edit.range.first = start;
            edit.range.one_past_last = end;
            Edit_Behaviors behaviors = {};
            edit_single(models->system, models, file, edit, behaviors);
            result = true;
        }
        fill_buffer_summary(buffer, file, &models->working_set);
    }
    return(result);
}

API_EXPORT bool32
Buffer_Compute_Cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer on which to run the cursor computation.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(cursor_out, On success this struct is filled with the result of the seek.)
DOC_RETURN(This call returns non-zero on success.  This call can fail if the buffer summary provided
does not summarize an actual buffer in 4coder, or if the provided seek format is invalid.  The valid
seek types are seek_pos and seek_line_char.)
DOC(Computes a Partial_Cursor for the given seek position with no side effects.
The seek position must be one of the types supported by Partial_Cursor.  Those
types are absolute position and line,column position.)
DOC_SEE(Buffer_Seek)
DOC_SEE(Partial_Cursor)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0){
        if (file_compute_partial_cursor(file, seek, cursor_out)){
            result = true;
            fill_buffer_summary(buffer, file, &models->working_set);
        }
    }
    return(result);
}

API_EXPORT bool32
Buffer_Batch_Edit(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type)
/*
DOC_PARAM(buffer, The buffer on which to apply the batch of edits.)
DOC_PARAM(str, This parameter provides all of the source string for the edits in the batch.)
DOC_PARAM(str_len, This parameter specifies the length of the str string.)
DOC_PARAM(edits, This parameter provides about the source string and destination range of each edit as an array.)
DOC_PARAM(edit_count, This parameter specifies the number of Buffer_Edit structs in edits.)
DOC_PARAM(type, This prameter specifies what type of batch edit to execute.)
DOC_RETURN(This call returns non-zero if the batch edit succeeds.  This call can fail if the provided buffer summary does not refer to an actual buffer in 4coder.)
DOC(Apply an array of edits all at once.  This combines all the edits into one undo operation.)
DOC_SEE(Buffer_Edit)
DOC_SEE(Buffer_Batch_Edit_Type)
*/{
    Models *models = (Models*)app->cmd_context;
    Mem_Options *mem = &models->mem;
    Partition *part = &mem->part;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0){
        if (edit_count > 0){
            Temp_Memory temp = begin_temp_memory(part);
            Edit_Array real_edits = {};
            real_edits.vals = push_array(part, Edit, edit_count);
            real_edits.count = edit_count;
            Edit *edit_out = real_edits.vals;
            Buffer_Edit *edit_in = edits;
            Edit *one_past_last_edit_out = real_edits.vals + edit_count;
            for (;edit_out < one_past_last_edit_out;
                 edit_out += 1, edit_in += 1){
                edit_out->str = str + edit_in->str_start;
                edit_out->length = edit_in->len;
                edit_out->range.first = edit_in->start;
                edit_out->range.one_past_last = edit_in->end;
            }
            Edit_Behaviors behaviors = {};
            behaviors.batch_type = type;
            edit_batch(system, models, file, real_edits, behaviors);
            end_temp_memory(temp);
        }
        result = true;
    }
    return(result);
}

API_EXPORT bool32
Buffer_Get_Setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out)
/*
DOC_PARAM(buffer, the buffer from which to read a setting)
DOC_PARAM(setting, the setting to read from the buffer)
DOC_PARAM(value_out, address to write the setting value on success)
DOC_RETURN(returns non-zero on success)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0){
        result = true;
        switch (setting){
            case BufferSetting_Lex:
            {
                *value_out = file->settings.tokens_exist;
            }break;
            
            case BufferSetting_LexWithoutStrings:
            {
                *value_out = file->settings.tokens_without_strings;
            }break;
            
            case BufferSetting_ParserContext:
            {
                *value_out = file->settings.parse_context_id;
            }break;
            
            case BufferSetting_WrapLine:
            {
                *value_out = !file->settings.unwrapped_lines;
            }break;
            
            case BufferSetting_WrapPosition:
            {
                *value_out = file->settings.display_width;
            }break;
            
            case BufferSetting_MinimumBaseWrapPosition:
            {
                *value_out = file->settings.minimum_base_display_width;
            }break;
            
            case BufferSetting_MapID:
            {
                *value_out = file->settings.base_map_id;
            }break;
            
            case BufferSetting_Eol:
            {
                *value_out = file->settings.dos_write_mode;
            }break;
            
            case BufferSetting_Unimportant:
            {
                *value_out = file->settings.unimportant;
            }break;
            
            case BufferSetting_ReadOnly:
            {
                *value_out = file->settings.read_only;
            }break;
            
            case BufferSetting_VirtualWhitespace:
            {
                *value_out = file->settings.virtual_white;
            }break;
            
            default:
            {
                result = 0;
            }break;
        }
    }
    return(result);
}

API_EXPORT bool32
Buffer_Set_Setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer on which to set a setting.)
DOC_PARAM(setting, The setting parameter identifies the setting that shall be changed.)
DOC_PARAM(value, The value parameter specifies the value to which the setting shall be changed.)
DOC_SEE(Buffer_Setting_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer);
    
    bool32 result = false;
    i32 new_mapid = 0;
    
    if (file != 0){
        result = true;
        switch (setting){
            case BufferSetting_Lex:
            {
                if (file->settings.tokens_exist){
                    if (!value){
                        file_kill_tokens(system, &models->mem.heap, file);
                    }
                }
                else{
                    if (value){
                        file_first_lex(system, models, file);
                    }
                }
            }break;
            
            case BufferSetting_LexWithoutStrings:
            {
                if (file->settings.tokens_exist){
                    if ((b8)value != file->settings.tokens_without_strings){
                        file_kill_tokens(system, &models->mem.heap, file);
                        file->settings.tokens_without_strings = (b8)value;
                        file_first_lex(system, models, file);
                    }
                }
                else{
                    file->settings.tokens_without_strings = (b8)value;
                }
            }break;
            
            case BufferSetting_ParserContext:
            {
                u32 fixed_value = parse_context_valid_id(&models->parse_context_memory, (u32)value);
                
                if (file->settings.tokens_exist){
                    if (fixed_value != file->settings.parse_context_id){
                        file_kill_tokens(system, &models->mem.heap, file);
                        file->settings.parse_context_id = fixed_value;
                        file_first_lex(system, models, file);
                    }
                }
                else{
                    file->settings.parse_context_id = fixed_value;
                }
            }break;
            
            case BufferSetting_WrapLine:
            {
                file->settings.unwrapped_lines = !value;
            }break;
            
            case BufferSetting_WrapPosition:
            {
                i32 new_value = value;
                if (new_value < 48){
                    new_value = 48;
                }
                if (new_value != file->settings.display_width){
                    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
                    file->settings.display_width = new_value;
                    file_measure_wraps(system, &models->mem, file, font);
                    adjust_views_looking_at_file_to_new_cursor(system, models, file);
                }
            }break;
            
            case BufferSetting_MinimumBaseWrapPosition:
            {
                i32 new_value = value;
                if (new_value < 0){
                    new_value = 0;
                }
                if (new_value != file->settings.minimum_base_display_width){
                    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
                    file->settings.minimum_base_display_width = new_value;
                    file_measure_wraps(system, &models->mem, file, font);
                    adjust_views_looking_at_file_to_new_cursor(system, models, file);
                }
            }break;
            
            case BufferSetting_WrapIndicator:
            {
                file->settings.wrap_indicator = value;
            }break;
            
            case BufferSetting_MapID:
            {
                if (value < mapid_global){
                    new_mapid = get_map_index(&models->mapping, value);
                    if (new_mapid < models->mapping.user_map_count){
                        file->settings.base_map_id = value;
                    }
                    else{
                        file->settings.base_map_id = mapid_file;
                    }
                }
                else if (value <= mapid_nomap){
                    file->settings.base_map_id = value;
                }
            }break;
            
            case BufferSetting_Eol:
            {
                file->settings.dos_write_mode = value;
            }break;
            
            case BufferSetting_Unimportant:
            {
                if (value != 0){
                    file_set_unimportant(file, true);
                }
                else{
                    file_set_unimportant(file, false);
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
            
            case BufferSetting_VirtualWhitespace:
            {
                b32 full_remeasure = false;
                if (value){
                    if (!file->settings.virtual_white){
                        if (!file->settings.tokens_exist){
                            file_first_lex_serial(system, models, file);
                        }
                        if (!file->state.still_lexing){
                            file->settings.virtual_white = true;
                            full_remeasure = true;
                        }
                        else{
                            result = false;
                        }
                    }
                }
                else{
                    if (file->settings.virtual_white){
                        file->settings.virtual_white = false;
                        full_remeasure = true;
                    }
                }
                
                if (full_remeasure){
                    Font_Pointers font = system->font.get_pointers_by_id(file->settings.font_id);
                    
                    file_allocate_character_starts_as_needed(&models->mem.heap, file);
                    buffer_measure_character_starts(system, font, &file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
                    file_measure_wraps(system, &models->mem, file, font);
                    adjust_views_looking_at_file_to_new_cursor(system, models, file);
                }
            }break;
            
            default: result = 0; break;
        }
        fill_buffer_summary(buffer, file, &models->working_set);
    }
    
    return(result);
}

internal Managed_Scope
buffer_get_managed_scope__inner(Editing_File *file){
    Managed_Scope lifetime = 0;
    if (file != 0){
        Assert(file->lifetime_object != 0);
        lifetime = (Managed_Scope)file->lifetime_object->workspace.scope_id;
    }
    return(lifetime);
}

API_EXPORT Managed_Scope
Buffer_Get_Managed_Scope(Application_Links *app, Buffer_ID buffer_id)
/*
DOC_PARAM(buffer_id, The id of the buffer from which to get a managed scope.)
DOC_RETURN(If the buffer_id specifies a valid buffer, the scope returned is the scope tied to the
lifetime of the buffer.  This is a 'basic scope' and is not considered a 'user managed scope'.
If the buffer_id does not specify a valid buffer, the returned scope is null.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    return(buffer_get_managed_scope__inner(file));
}

API_EXPORT int32_t
Buffer_Token_Count(Application_Links *app, Buffer_Summary *buffer)
/*
DOC_PARAM(buffer, Specifies the buffer from which to read the token count.)
DOC_RETURN(If tokens are available for the buffer, the number of tokens on the buffer is returned.
If the buffer does not exist or if it is not a lexed buffer, the return is zero.)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    int32_t count = 0;
    if (file != 0 && file->state.token_array.tokens && file->state.tokens_complete){
        count = file->state.token_array.count;
    }
    return(count);
}

API_EXPORT bool32
Buffer_Read_Tokens(Application_Links *app, Buffer_Summary *buffer, int32_t start_token, int32_t end_token, Cpp_Token *tokens_out)
/*
DOC_PARAM(buffer, Specifies the buffer from which to read tokens.)
DOC_PARAM(first_token, Specifies the index of the first token to read.)
DOC_PARAM(end_token, Specifies the token to stop reading at.)
DOC_PARAM(tokens_out, The memory that will store the tokens read from the buffer.)
DOC_RETURN(Returns non-zero on success.  This call can fail if the buffer doesn't
exist or doesn't have tokens ready, or if either the first or last index is out of bounds.)
DOC(Puts the data for the tokens with the indices [first_token,last_token) into the tokens_out array.
The number of output tokens will be end_token - start_token.)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    Cpp_Token_Array token_array = file->state.token_array;
    bool32 result = false;
    if (file != 0 && token_array.tokens != 0 && file->state.tokens_complete){
        if (0 <= start_token && start_token <= end_token && end_token <= token_array.count){
            result = true;
            memcpy(tokens_out, token_array.tokens + start_token, sizeof(Cpp_Token)*(end_token - start_token));
        }
    }
    return(result);
}

// TODO(allen): Include warning note about pointers and editing buffers.
API_EXPORT bool32
Buffer_Get_Token_Range(Application_Links *app, Buffer_Summary *buffer, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    Cpp_Token_Array token_array = file->state.token_array;
    bool32 result = false;
    if (file != 0 && token_array.tokens != 0 && file->state.tokens_complete){
        result = true;
        *first_token_out = token_array.tokens;
        *one_past_last_token_out = token_array.tokens + token_array.count;
    }
    return(result);
}

API_EXPORT bool32
Buffer_Get_Token_Index(Application_Links *app, Buffer_Summary *buffer, int32_t pos, Cpp_Get_Token_Result *get_result)
/*
DOC_PARAM(buffer, The buffer from which to get a token.)
DOC_PARAM(pos, The position in the buffer in absolute coordinates.)
DOC_PARAM(get_result, The output struct specifying which token contains pos.)
DOC_RETURN(Returns non-zero on success.  This call can fail if the buffer doesn't exist, or if the buffer doesn't have tokens ready.)
DOC(This call finds the token that contains a particular position, or if the position is in between tokens it finds the index of the token to the left of the position.  The returned index can be -1 if the position is before the first token.)
DOC_SEE(Cpp_Get_Token_Result)
DOC_SEE(cpp_get_token)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    Cpp_Token_Array token_array = file->state.token_array;
    
    bool32 result = false;
    if (file != 0 && token_array.tokens != 0 && file->state.tokens_complete){
        result = true;
        *get_result = cpp_get_token(token_array, pos);
    }
    
    return(result);
}

API_EXPORT bool32
Buffer_Send_End_Signal(Application_Links *app, Buffer_Summary *buffer)
/*
DOC_PARAM(buffer, The buffer to which to send the end signal.)
DOC_RETURN(Returns non-zero on success.  This call can fail if the buffer doesn't exist.)
DOC(Whenever a buffer is killed an end signal is sent which triggers the end file hook.  This call sends the end signal to the buffer without killing the buffer.  This is useful in cases such as clearing a buffer and refilling it with new contents.)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0){
        result = true;
        file_end_file(models, file);
    }
    return(result);
}

API_EXPORT Buffer_Summary
Create_Buffer(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags)
/*
DOC_PARAM(filename, The name of the file to associate to the new buffer.)
DOC_PARAM(filename_len, The length of the filename string.)
DOC_PARAM(flags, Flags controlling the buffer creation behavior.)
DOC_RETURN(Returns the newly created buffer or an already existing buffer with the given name.)
DOC(Try to create a new buffer.  This call first checks to see if a buffer already exists that goes by the given name, if so, that buffer is returned.

If no buffer exists with the given name, then a new buffer is created.  If a file that matches the given filename exists, the file is loaded as the contents of the new buffer.  Otherwise a buffer is created without a matching file until the buffer is saved and the buffer is left blank.)
DOC_SEE(Buffer_Create_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Working_Set *working_set = &models->working_set;
    Heap *heap = &models->mem.heap;
    Partition *part = &models->mem.part;
    
    Buffer_Summary result = {};
    b32 buffer_is_for_new_file = false;
    
    if (filename_len > 0){
        Temp_Memory temp = begin_temp_memory(part);
        
        String fname = make_string(filename, filename_len);
        Editing_File *file = 0;
        b32 do_empty_buffer = false;
        Editing_File_Name canon = {};
        b32 has_canon_name = false;
        
        // NOTE(allen): Try to get the file by canon name.
        if ((flags & BufferCreate_NeverAttachToFile) == 0){
            if (get_canon_name(system, fname, &canon)){
                has_canon_name = true;
                file = working_set_contains_canon(working_set, canon.name);
            }
            else{
                do_empty_buffer = true;
            }
        }
        
        // NOTE(allen): Try to get the file by buffer name.
        if ((flags & BufferCreate_MustAttachToFile) == 0){
            if (file == 0){
                file = working_set_contains_name(working_set, fname);
            }
        }
        
        // NOTE(allen): If there is still no file, create a new buffer.
        if (file == 0){
            Plat_Handle handle = {};
            
            // NOTE(allen): Figure out whether this is a new file, or an existing file.
            if (!do_empty_buffer){
                if ((flags & BufferCreate_AlwaysNew) != 0){
                    do_empty_buffer = true;
                }
                else{
                    if (!system->load_handle(canon.name.str, &handle)){
                        do_empty_buffer = true;
                    }
                }
            }
            
            if (do_empty_buffer){
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
                if ((flags & BufferCreate_NeverNew) == 0){
                    file = working_set_alloc_always(working_set, heap, &models->lifetime_allocator);
                    if (file != 0){
                        if (has_canon_name){
                            file_bind_filename(system, heap, working_set, file, canon.name);
                        }
                        buffer_bind_name(models, heap, part, working_set, file, front_of_directory(fname));
                        init_normal_file(system, models, 0, 0, file);
                        fill_buffer_summary(&result, file, &models->working_set);
                    }
                }
            }
            else{
                Assert(!handle_equal(handle, null_plat_handle));
                
                i32 size = system->load_size(handle);
                b32 in_heap_mem = false;
                char *buffer = push_array(part, char, size);
                
                if (buffer == 0){
                    buffer = heap_array(heap, char, size);
                    Assert(buffer != 0);
                    in_heap_mem = true;
                }
                
                if (system->load_file(handle, buffer, size)){
                    system->load_close(handle);
                    file = working_set_alloc_always(working_set, heap, &models->lifetime_allocator);
                    if (file != 0){
                        file_bind_filename(system, heap, working_set, file, canon.name);
                        buffer_bind_name(models, heap, part, working_set, file, front_of_directory(fname));
                        init_normal_file(system, models, buffer, size, file);
                        fill_buffer_summary(&result, file, &models->working_set);
                    }
                }
                else{
                    system->load_close(handle);
                }
                
                if (in_heap_mem){
                    heap_free(heap, buffer);
                }
            }
        }
        else{
            fill_buffer_summary(&result, file, &models->working_set);
        }
        
        if (file != 0 && (flags & BufferCreate_JustChangedFile) != 0){
            file->state.ignore_behind_os = 1;
        }
        
        if (file != 0 && (flags & BufferCreate_AlwaysNew) != 0){
            i32 size = buffer_size(&file->state.buffer);
            if (size > 0){
                Edit edit = {};
                edit.range.one_past_last = size;
                Edit_Behaviors behaviors = {};
                edit_single(system, models, file, edit, behaviors);
                if (has_canon_name){
                    buffer_is_for_new_file = true;
                }
            }
        }
        
        if (file != 0 && buffer_is_for_new_file &&
            (flags & BufferCreate_SuppressNewFileHook) == 0){
            if (models->hook_new_file != 0){
                models->hook_new_file(&models->app_links, file->id.id);
            }
        }
        
        end_temp_memory(temp);
    }
    
    return(result);
}

API_EXPORT bool32
Save_Buffer(Application_Links *app, Buffer_Summary *buffer, char *file_name, int32_t file_name_len, uint32_t flags)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer to save to a file.)
DOC_PARAM(file_name, The file_name parameter specifies the name of the file to write with the contents of the buffer; it need not be null terminated.)
DOC_PARAM(file_name_len, The file_name_len parameter specifies the length of the file_name string.)
DOC_PARAM(flags, Specifies special behaviors for the save routine.)
DOC_RETURN(This call returns non-zero on success.)
DOC(Often it will make sense to set file_name and file_name_len to buffer.file_name and buffer.file_name_len)
DOC_SEE(Buffer_Save_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    
    if (file != 0){
        b32 skip_save = false;
        if (!(flags & BufferSave_IgnoreDirtyFlag)){
            if (file->state.dirty == DirtyState_UpToDate){
                skip_save = true;
            }
        }
        
        if (!skip_save){
            result = true;
            
            Partition *part = &models->mem.part;
            Temp_Memory temp = begin_temp_memory(part);
            String name = push_string(part, file_name, file_name_len);
            save_file_to_name(system, models, file, name.str);
            end_temp_memory(temp);
        }
    }
    
    return(result);
}

API_EXPORT Buffer_Kill_Result
Kill_Buffer(Application_Links *app, Buffer_Identifier buffer, Buffer_Kill_Flag flags)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer to try to kill.)
DOC_PARAM(flags, The flags parameter specifies behaviors for the buffer kill.)
DOC_RETURN(This call returns BufferKillResult_Killed if the call successfully kills the buffer,
for extended information on other kill results see the Buffer_Kill_Result enumeration.)

DOC(Tries to kill the idenfied buffer, if the buffer is dirty or does not exist then nothing will
happen. The default rules about when to kill or not kill a buffer can be altered by the flags.)

DOC_SEE(Buffer_Kill_Result)
DOC_SEE(Buffer_Kill_Flag)
DOC_SEE(Buffer_Identifier)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Buffer_Kill_Result result = BufferKillResult_DoesNotExist;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = get_file_from_identifier(system, working_set, buffer);
    if (file != 0){
        result = BufferKillResult_Unkillable;
        if (!file->settings.never_kill){
            b32 needs_to_save = file_needs_save(file);
            if (!needs_to_save || (flags & BufferKill_AlwaysKill) != 0){
                if (models->hook_end_file != 0){
                    models->hook_end_file(&models->app_links, file->id.id);
                }
                
                buffer_unbind_name_low_level(working_set, file);
                if (file->canon.name.size != 0){
                    buffer_unbind_file(system, working_set, file);
                }
                file_free(system, &models->mem.heap, &models->lifetime_allocator, file);
                working_set_free_file(&models->mem.heap, working_set, file);
                
                Layout *layout = &models->layout;
                
                Node *used = &working_set->used_sentinel;
                Node *file_node = used->next;
                for (Panel *panel = layout_get_first_open_panel(layout);
                     panel != 0;
                     panel = layout_get_next_open_panel(layout, panel)){
                    View *view = panel->view;
                    if (view->transient.file_data.file == file){
                        Assert(file_node != used);
                        view->transient.file_data.file = 0;
                        Editing_File *new_file = CastFromMember(Editing_File, main_chain_node, file_node);
                        view_set_file(system, models, view, new_file);
                        if (file_node->next != used){
                            file_node = file_node->next;
                        }
                        else{
                            file_node = file_node->next->next;
                            Assert(file_node != used);
                        }
                    }
                }
                
                result = BufferKillResult_Killed;
            }
            else{
                result = BufferKillResult_Dirty;
            }
        }
    }
    return(result);
}

API_EXPORT Buffer_Reopen_Result
Reopen_Buffer(Application_Links *app, Buffer_Summary *buffer, Buffer_Reopen_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer);
    
    Buffer_Reopen_Result result = BufferReopenResult_Failed;
    
    if (file->canon.name.str != 0 && file->canon.name.size != 0){
        Plat_Handle handle = {};
        if (system->load_handle(file->canon.name.str, &handle)){
            i32 size = system->load_size(handle);
            
            Partition *part = &models->mem.part;
            Temp_Memory temp = begin_temp_memory(part);
            char *file_memory = push_array(part, char, size);
            
            if (file_memory != 0){
                if (system->load_file(handle, file_memory, size)){
                    system->load_close(handle);
                    
                    // TODO(allen): try(perform a diff maybe apply edits in reopen)
                    
                    int32_t line_numbers[16];
                    int32_t column_numbers[16];
                    View *vptrs[16];
                    i32 vptr_count = 0;
                    
                    Layout *layout = &models->layout;
                    for (Panel *panel = layout_get_first_open_panel(layout);
                         panel != 0;
                         panel = layout_get_next_open_panel(layout, panel)){
                        View *view_it = panel->view;
                        if (view_it->transient.file_data.file != file){
                            continue;
                        }
                        vptrs[vptr_count] = view_it;
                        File_Edit_Positions edit_pos = view_get_edit_pos(view_it);
                        Full_Cursor cursor = file_compute_cursor(system, view_it->transient.file_data.file, seek_pos(edit_pos.cursor_pos));
                        line_numbers[vptr_count]   = cursor.line;
                        column_numbers[vptr_count] = cursor.character;
                        view_it->transient.file_data.file = models->scratch_buffer;
                        ++vptr_count;
                    }
                    
                    file_free(system, &models->mem.heap, &models->lifetime_allocator, file);
                    working_set_file_default_settings(&models->working_set, file);
                    init_normal_file(system, models, file_memory, size, file);
                    
                    for (i32 i = 0; i < vptr_count; ++i){
                        view_set_file(system, models, vptrs[i], file);
                        
                        vptrs[i]->transient.file_data.file = file;
                        Full_Cursor cursor = file_compute_cursor(system, file, seek_line_char(line_numbers[i], column_numbers[i]));
                        
                        view_set_cursor(system, vptrs[i], cursor, true, file->settings.unwrapped_lines);
                    }
                    
                    result = BufferReopenResult_Reopened;
                }
                else{
                    system->load_close(handle);
                }
            }
            else{
                system->load_close(handle);
            }
            
            end_temp_memory(temp);
        }
    }
    
    return(result);
}

internal void
get_view_first__internal(Models *models, View_Summary *view){
    Panel *panel = layout_get_first_open_panel(&models->layout);
    fill_view_summary(models->system, view, panel->view, models);
}

internal void
get_view_next__internal(Models *models, View_Summary *view){
    System_Functions *system = models->system;
    Layout *layout = &models->layout;
    Live_Views *live_set = &models->live_set;
    i32 index = view->view_id - 1;
    if (index >= 0 && index < live_set->max){
        View *vptr = live_set->views + index;
        Panel *panel = vptr->transient.panel;
        if (panel != 0){
            panel = layout_get_next_open_panel(layout, panel);
        }
        if (panel != 0){
            fill_view_summary(system, view, panel->view, models);
        }
        else{
            block_zero_struct(view);
        }
    }
    else{
        block_zero_struct(view);
    }
}

API_EXPORT View_Summary
Get_View_First(Application_Links *app, Access_Flag access)
/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns the summary of the first view in a view loop.)
DOC(
This call begins a loop across all the open views.

If the View_Summary returned is a null summary, the loop is finished.
Views should not be closed or opened durring a view loop.
)
DOC_SEE(Access_Flag)
DOC_SEE(get_view_next)
*/{
    Models *models = (Models*)app->cmd_context;
    View_Summary view = {};
    get_view_first__internal(models, &view);
    while (view.exists && !access_test(view.lock_flags, access)){
        get_view_next__internal(models, &view);
    }
    return(view);
}

API_EXPORT void
Get_View_Next(Application_Links *app, View_Summary *view, Access_Flag access)
/*
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
    Models *models = (Models*)app->cmd_context;
    get_view_next__internal(models, view);
    while (view->exists && !access_test(view->lock_flags, access)){
        get_view_next__internal(models, view);
    }
}

API_EXPORT View_Summary
Get_View(Application_Links *app, View_ID view_id, Access_Flag access)
/*
DOC_PARAM(view_id, The view_id specifies the view to try to get.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated view if it is open and accessible.)
DOC_SEE(Access_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Live_Views *live_set = &models->live_set;
    View_Summary view = {};
    i32 max = live_set->max;
    view_id -= 1;
    if (view_id >= 0 && view_id < max){
        View *vptr = live_set->views + view_id;
        fill_view_summary(system, &view, vptr, models);
        if (!access_test(view.lock_flags, access)){
            memset(&view, 0, sizeof(view));
        }
    }
    return(view);
}

API_EXPORT View_Summary
Get_Active_View(Application_Links *app, Access_Flag access)
/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the active view.)
DOC_SEE(set_active_view)
DOC_SEE(Access_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Panel *panel = layout_get_active_panel(&models->layout);
    Assert(panel->view != 0);
    View_Summary view = {};
    fill_view_summary(system, &view, panel->view, &models->live_set, &models->working_set);
    if (!access_test(view.lock_flags, access)){
        block_zero_struct(&view);
    }
    return(view);
}

API_EXPORT View_Summary
Open_View(Application_Links *app, View_Summary *view_location, View_Split_Position position)
/*
DOC_PARAM(view_location, The view_location parameter specifies the view to split to open the new view.)
DOC_PARAM(position, The position parameter specifies how to split the view and where to place the new view.)
DOC_RETURN(If this call succeeds it returns a View_Summary describing the newly created view, if it fails it
returns a null summary.)
DOC(4coder is built with a limit of 16 views.  If 16 views are already open when this is called the call will fail.)
DOC_SEE(View_Split_Position)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Layout *layout = &models->layout;
    View *vptr = imp_get_view(models, view_location);
    Panel *panel = vptr->transient.panel;
    View_Summary result = {};
    b32 vertical_split = ((position == ViewSplit_Left) || (position == ViewSplit_Right));
    b32 br_split = ((position == ViewSplit_Bottom) || (position == ViewSplit_Right));
    Panel *new_panel  = layout_split_panel(layout, panel, vertical_split, br_split);
    if (new_panel != 0){
        View *new_view = live_set_alloc_view(&models->mem.heap, &models->lifetime_allocator, &models->live_set);
        new_panel->view = new_view;
        new_view->transient.panel = new_panel;
        view_set_file(system, models, new_view, models->scratch_buffer);
        fill_view_summary(system, &result, new_view, models);
    }
    return(result);
}

API_EXPORT bool32
Close_View(Application_Links *app, View_Summary *view)
/*
DOC_PARAM(view, The view parameter specifies which view to close.)
DOC_RETURN(This call returns non-zero on success.)

DOC(If the given view is open and is not the last view, it will be closed.
If the given view is the active view, the next active view in the global
order of view will be made active. If the given view is the last open view
in the system, the call will fail.)

*/{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *vptr = imp_get_view(models, view);
    
    bool32 result = false;
    if (vptr != 0){
        if (layout_close_panel(layout, vptr->transient.panel)){
            live_set_free_view(&models->mem.heap, &models->lifetime_allocator, &models->live_set, vptr);
            result = true;
        }
    }
        
    return(result);
}

API_EXPORT bool32
Set_Active_View(Application_Links *app, View_Summary *view)
/*
DOC_PARAM(view, The view parameter specifies which view to make active.)
DOC_RETURN(This call returns non-zero on success.)
DOC(If the given view is open, it is set as the
active view, and takes subsequent commands and is returned
from get_active_view.)
DOC_SEE(get_active_view)
*/{
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    if (vptr != 0){
        models->layout.active_panel = vptr->transient.panel;
        result = true;
    }
    return(result);
}

API_EXPORT bool32
View_Get_Setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out)
/*
DOC_PARAM(view, the view from which to read a setting)
DOC_PARAM(setting, the view setting to read)
DOC_PARAM(value_out, address to write the setting value on success)
DOC_RETURN(returns non-zero on success)
*/{
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    int32_t result = 0;
    
    if (vptr != 0){
        result = 1;
        switch (setting){
            case ViewSetting_ShowWhitespace:
            {
                *value_out = vptr->transient.file_data.show_whitespace;
            }break;
            
            case ViewSetting_ShowScrollbar:
            {
                *value_out = !vptr->transient.hide_scrollbar;
            }break;
            
            case ViewSetting_ShowFileBar:
            {
                *value_out = !vptr->transient.hide_file_bar;
            }break;
            
            case ViewSetting_UICommandMap:
            {
                *value_out = vptr->transient.ui_map_id;
            }break;
            
            default:
            {
                result = 0;
            }break;
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value)
/*
DOC_PARAM(view, The view parameter specifies the view on which to set a setting.)
DOC_PARAM(setting, The setting parameter identifies the setting that shall be changed.)
DOC_PARAM(value, The value parameter specifies the value to which the setting shall be changed.)
DOC_RETURN(This call returns non-zero on success.)
DOC_SEE(View_Setting_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    
    if (vptr != 0){
        result = true;
        switch (setting){
            case ViewSetting_ShowWhitespace:
            {
                vptr->transient.file_data.show_whitespace = value;
            }break;
            
            case ViewSetting_ShowScrollbar:
            {
                vptr->transient.hide_scrollbar = !value;
            }break;
            
            case ViewSetting_ShowFileBar:
            {
                vptr->transient.hide_file_bar = !value;
            }break;
            
            case ViewSetting_UICommandMap:
            {
                vptr->transient.ui_map_id = value;
            }break;
            
            default:
            {
                result = false;
            }break;
        }
        
        fill_view_summary(system, view, vptr, models);
    }
    
    return(result);
}

API_EXPORT Managed_Scope
View_Get_Managed_Scope(Application_Links *app, View_ID view_id)
/*
DOC_PARAM(view_id, The id of the view from which to get a managed scope.)
DOC_RETURN(If the view_id specifies a valid view, the scope returned is the scope tied to the
lifetime of the view.  This is a 'basic scope' and is not considered a 'user managed scope'.
If the view_id does not specify a valid view, the returned scope is null.)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Managed_Scope lifetime = 0;
    if (view != 0){
        Assert(view->transient.lifetime_object != 0);
        lifetime = (Managed_Scope)(view->transient.lifetime_object->workspace.scope_id);
    }
    return(lifetime);
}

API_EXPORT bool32
View_Set_Split(Application_Links *app, View_Summary *view, View_Split_Kind kind, float t)
/*
DOC_PARAM(view, The view parameter specifies which view shall have it's size adjusted.)
DOC_PARAM(kind, There are different kinds of split, see View_Split_Kind documentation for more information.)
DOC_PARAM(t, The t parameter specifies the proportion of the containing box that the view should occupy.

For proportion values, t will be clamped to [0,1].

For integer values, t will be rounded to the nearest integer.
)
DOC_SEE(View_Split_Kind)
DOC_RETURN(This call returns non-zero on success.)
*/{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    if (vptr != 0){
        Panel *panel = vptr->transient.panel;
        Panel *intermediate = panel->parent;
        if (intermediate != 0){
            Assert(intermediate->kind == PanelKind_Intermediate);
            switch (kind){
                case ViewSplitKind_Ratio:
                {
                    if (intermediate->br_panel == panel){
                        intermediate->split.kind = PanelSplitKind_Ratio_BR;
                    }
                    else{
                        intermediate->split.kind = PanelSplitKind_Ratio_TL;
                    }
                    intermediate->split.v_f32 = clamp(0.f, t, 1.f);
                }break;
                
                case ViewSplitKind_FixedPixels:
                {
                    if (intermediate->br_panel == panel){
                        intermediate->split.kind = PanelSplitKind_FixedPixels_BR;
                    }
                    else{
                        intermediate->split.kind = PanelSplitKind_FixedPixels_TL;
                    }
                    intermediate->split.v_i32 = round32(t);
                }break;
                
                default:
                {
                    print_message(app, literal("Invalid split kind passed to view_set_split, no change made to view layout"));
                }break;
            }
            layout_propogate_sizes_down_from_node(layout, intermediate);
            result = true;
        }
    }
    
    return(result);
}

API_EXPORT i32_Rect
View_Get_Enclosure_Rect(Application_Links *app, View_Summary *view)
/*
DOC_PARAM(view, The view whose parent rent will be returned.)
DOC_RETURN(The rectangle of the panel containing this view.)
*/{
    // TODO(allen): do(remove this from the API and put it in the custom helpers)
    // we should just have full tree traversal API for the splits between views.
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    i32_Rect result = {};
    if (vptr != 0){
        Panel *panel = vptr->transient.panel;
        Assert(panel != 0);
        Panel *parent = panel->parent;
        if (parent != 0){
            result = parent->rect_full;
        }
        else{
            result = panel->rect_full;
        }
    }
    return(result);
}

API_EXPORT bool32
View_Compute_Cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out)
/*
DOC_PARAM(view, The view parameter specifies the view on which to run the cursor computation.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(cursor_out, On success this struct is filled with the result of the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(Computes a Full_Cursor for the given seek position with no side effects.)
DOC_SEE(Buffer_Seek)
DOC_SEE(Full_Cursor)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    
    if (vptr != 0){
        Editing_File *file = vptr->transient.file_data.file;
        Assert(file != 0);
        if (!file->is_loading){
            result = true;
            *cursor_out = file_compute_cursor(system, file, seek);
            fill_view_summary(system, view, vptr, models);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x)
/*
DOC_PARAM(view, The view parameter specifies the view in which to set the cursor.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(set_preferred_x, If this parameter is true the preferred x is updated to match the new cursor x.)
DOC_RETURN(This call returns non-zero on success.)
DOC(This call sets the the view's cursor position.  set_preferred_x should usually be true unless the change in cursor position is is a vertical motion that tries to keep the cursor in the same column or x position.)
DOC_SEE(Buffer_Seek)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    
    if (vptr != 0){
        Editing_File *file = vptr->transient.file_data.file;
        if (!file->is_loading){
            result = true;
            Full_Cursor cursor = file_compute_cursor(system, file, seek);
            view_set_cursor(system, vptr, cursor, set_preferred_x, file->settings.unwrapped_lines);
            fill_view_summary(system, view, vptr, models);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll)
/*
DOC_PARAM(view, The view on which to change the scroll state.)
DOC_PARAM(scroll, The new scroll position for the view.)
DOC(Set the scrolling state of the view.)
DOC_SEE(GUI_Scroll_Vars)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    
    if (vptr != 0){
        Editing_File *file = vptr->transient.file_data.file;
        if (!file->is_loading){
            result = true;
            if (!vptr->transient.ui_mode){
                view_set_scroll(system, vptr, scroll);
            }
            else{
                vptr->transient.ui_scroll = scroll;
            }
            fill_view_summary(system, view, vptr, models);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Mark(Application_Links *app, View_Summary *view, Buffer_Seek seek)
/*
DOC_PARAM(view, The view parameter specifies the view in which to set the mark.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(This call sets the the view's mark position.)
DOC_SEE(Buffer_Seek)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    
    if (vptr != 0){
        Editing_File *file = vptr->transient.file_data.file;
        Assert(file != 0);
        if (!file->is_loading){
            if (seek.type != buffer_seek_pos){
                result = true;
                File_Edit_Positions edit_pos = view_get_edit_pos(vptr);
                Full_Cursor cursor = file_compute_cursor(system, file, seek);
                edit_pos.mark = cursor.pos;
                view_set_edit_pos(vptr, edit_pos);
            }
            else{
                result = true;
                File_Edit_Positions edit_pos = view_get_edit_pos(vptr);
                edit_pos.mark = seek.pos;
                view_set_edit_pos(vptr, edit_pos);
            }
            fill_view_summary(system, view, vptr, models);
        }
    }
    
    return(result);
}

API_EXPORT bool32
View_Set_Highlight(Application_Links *app, View_Summary *view, int32_t start, int32_t end, bool32 turn_on)/*
DOC(This feature has been removed.  Transition to the new highlighting system view markers which allow for
arbitrarily many highlights, and cursors, at the same time.)
*/{
    return(false);
}

API_EXPORT bool32
View_Set_Buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags)
/*
DOC_PARAM(view, The view parameter specifies the view in which to display the buffer.)
DOC_PARAM(buffer_id, The buffer_id parameter specifies which buffer to show in the view.)
DOC_PARAM(flags, The flags parameter specifies behaviors for setting the buffer.)
DOC_RETURN(This call returns non-zero on success.)
DOC(On success view_set_buffer sets the specified view's current buffer and cancels and dialogue shown in the view and displays the file.)
DOC_SEE(Set_Buffer_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    
    if (vptr != 0){
        Editing_File *file = working_set_get_active_file(&models->working_set, buffer_id);
        if (file != 0){
            result = true;
            if (file != vptr->transient.file_data.file){
                view_set_file(system, models, vptr, file);
                if (!(flags & SetBuffer_KeepOriginalGUI)){
                    view_quit_ui(system, models, vptr);
                }
            }
        }
        
        fill_view_summary(system, view, vptr, models);
    }
    
    return(result);
}

API_EXPORT bool32
View_Post_Fade(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color)
/*
DOC_PARAM(view, The view parameter specifies the view onto which the fade effect shall be posted.)
DOC_PARAM(seconds, This parameter specifies the number of seconds the fade effect should last.)
DOC_PARAM(start, This parameter specifies the absolute position of the first character of the fade range.)
DOC_PARAM(end, This parameter specifies the absolute position of the character one past the end of the fdae range.)
DOC_PARAM(color, The color parameter specifies the initial color of the text before it fades to it's natural color.)
DOC_RETURN(This call returns non-zero on success.)
DOC_SEE(int_color)
*/{
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    bool32 result = false;
    int32_t size = end - start;
    if (vptr){
        if (size > 0){
            result = true;
            view_post_paste_effect(vptr, seconds, start, size, color|0xFF000000);
        }
    }
    return(result);
}

API_EXPORT bool32
View_Begin_UI_Mode(Application_Links *app, View_Summary *view)
/*
DOC_PARAM(view, A summary for the view which is to be placed into UI mode.)
DOC_RETURN(This call returns non-zero on success.  It can fail if view is invalid, or if the view is already in UI mode.)
DOC(In UI mode, the view no longer renders it's buffer.  Instead it renders UI elements attached to the view.  The UI elements attached to a view can be modified by a view_set_ui call.)
DOC_SEE(view_end_ui_mode)
DOC_SEE(view_set_ui)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    if (vptr != 0){
        if (vptr->transient.ui_mode){
            return(false);
        }
        else{
            vptr->transient.ui_mode = true;
            return(true);
        }
    }
    else{
        return(false);
    }
}

API_EXPORT bool32
View_End_UI_Mode(Application_Links *app, View_Summary *view)
/*
DOC_PARAM(view, A summary for the view which is to be taken out of UI mode.)
DOC_RETURN(This call returns non-zero on success.  It can fail if view is invalid, or if the view is not in UI mode.)
DOC(Taking a view out of UI mode not only causes it to resume showing a buffer, but also sends a quit UI signal.
The exit signal calls the quit_function that is set by a view_set_ui call.)
DOC_SEE(view_begin_ui_mode)
DOC_SEE(view_set_ui)
*/
{
    bool32 result = false;
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    if (vptr != 0 && vptr->transient.ui_mode){
        view_quit_ui(models->system, models, vptr);
        vptr->transient.ui_mode = false;
        result = true;
    }
    return(result);
}

API_EXPORT bool32
View_Set_UI(Application_Links *app, View_Summary *view, UI_Control *control, UI_Quit_Function_Type *quit_function)
/*
DOC_PARAM(view, A summary for the view which is to get the new UI widget data.)
DOC_PARAM(control, A pointer to the baked UI widget data.  To get data in the UI_Control format see the helper called ui_list_to_ui_control.  More information about specifying widget data can be found in a comment in '4coder_ui_helper.cpp'.  If this pointer is null the view's widget data is cleared to zero.  The core maintains it's own copy of the widget data, so after this call the memory used to specify widget data may be freed.)
DOC_PARAM(quit_function, A function pointer to be called when a quit UI signal is sent to the view.  This pointer may be set to null.)
DOC_RETURN(This call returns non-zero on success.  It can fail if view is invalid.)
DOC(Setting the UI widget data determines what the view will look like in UI mode.  The UI widget data can be set no matter what the current mode is.  The UI widget data can be replaced any number of times regardless of whether the view is in UI mode or not.  See documentation on the UI widget data type UI_Control and the comments at the top of '4coder_ui_helper.cpp' for more information about the types of widgets that can be used.)
DOC_SEE(view_begin_ui_mode)
DOC_SEE(view_end_ui_mode)
DOC_SEE(UI_Control)
DOC_SEE(UI_Quit_Function_Type)
*/
{
    Models *models = (Models*)app->cmd_context;
    Heap *heap = &models->mem.heap;
    View *vptr = imp_get_view(models, view);
    if (vptr != 0){
        if (vptr->transient.ui_control.items != 0){
            heap_free(heap, vptr->transient.ui_control.items);
        }
        memset(&vptr->transient.ui_control, 0, sizeof(vptr->transient.ui_control));
        vptr->transient.ui_quit = quit_function;
        if (control != 0){
            if (control->count > 0){
                i32 string_size = 0;
                for (UI_Item *item = control->items, *one_past_last = control->items + control->count;
                     item < one_past_last;
                     item += 1){
                    switch (item->type){
                        case UIType_Option:
                        {
                            string_size += item->option.string.size;
                            string_size += item->option.status.size;
                        }break;
                        
                        case UIType_TextField:
                        {
                            string_size += item->text_field.query.size;
                            string_size += item->text_field.string.size;
                        }break;
                        
                        case UIType_ColorTheme:
                        {
                            string_size += item->color_theme.string.size;
                        }break;
                    }
                }
                
                i32 all_items_size = sizeof(UI_Item)*control->count;
                i32 memory_size = all_items_size + string_size;
                UI_Item *new_items = (UI_Item*)heap_allocate(heap, memory_size);
                if (new_items != 0){
                    char *string_space = (char*)(new_items + control->count);
                    Partition string_alloc = make_part(string_space, string_size);
                    i32 count = control->count;
                    memcpy(new_items, control->items, all_items_size);
                    for (UI_Item *item = new_items, *one_past_last = new_items + count;
                         item < one_past_last;
                         item += 1){
                        
                        String *fixup[2];
                        
                        int32_t fixup_count = 0;
                        switch (item->type){
                            case UIType_Option:
                            {
                                fixup[0] = &item->option.string;
                                fixup[1] = &item->option.status;
                                fixup_count = 2;
                            }break;
                            case UIType_TextField:
                            {
                                fixup[0] = &item->text_field.query;
                                fixup[1] = &item->text_field.string;
                                fixup_count = 2;
                            }break;
                            case UIType_ColorTheme:
                            {
                                fixup[0] = &item->color_theme.string;
                                fixup_count = 1;
                            }break;
                        }
                        
                        for (i32 i = 0; i < fixup_count; i += 1){
                            String old = *fixup[i];
                            char *new_str = push_array(&string_alloc, char, old.size);
                            fixup[i]->str = new_str;
                            fixup[i]->size = old.size;
                            fixup[i]->memory_size = old.size;
                            memcpy(new_str, old.str, old.size);
                        }
                    }
                    vptr->transient.ui_control.items = new_items;
                    vptr->transient.ui_control.count = count;
                }
                else{
                    return(false);
                }
            }
            memcpy(vptr->transient.ui_control.bounding_box, control->bounding_box,
                   sizeof(control->bounding_box));
        }
        return(true);
    }
    return(false);
}

API_EXPORT UI_Control
View_Get_UI_Copy(Application_Links *app, View_Summary *view, struct Partition *part)
/*
DOC_PARAM(view, A summary for the view which is to get the new UI widget data.)
DOC_PARAM(part, A memory arena onto which the UI widget data will be allocated.)
DOC_RETURN(If the view is valid, and there is enough space in part, then the UI_Control stucture returned points to a copy of the widget data currently attached to the view.)
DOC_SEE(view_set_ui)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *vptr = imp_get_view(models, view);
    UI_Control result = {};
    if (vptr != 0 && part != 0){
        UI_Control *control = &vptr->transient.ui_control;
        result.items = push_array(part, UI_Item, control->count);
        if (result.items != 0){
            result.count = control->count;
            // TODO(allen): do(fixup the pointers)
            memcpy(result.items, control->items, sizeof(*result.items)*result.count);
        }
        else{
            return(result);
        }
        memcpy(result.bounding_box, control->bounding_box, sizeof(result.bounding_box));
    }
    return(result);
}

internal Dynamic_Workspace*
get_dynamic_workspace(Models *models, Managed_Scope handle){
    u32_Ptr_Lookup_Result lookup_result = lookup_u32_Ptr_table(&models->lifetime_allocator.scope_id_to_scope_ptr_table, (u32)handle);
    if (!lookup_result.success){
        return(0);
    }
    return((Dynamic_Workspace*)*lookup_result.val);
}

API_EXPORT Managed_Scope
Create_User_Managed_Scope(Application_Links *app)
/*
DOC_RETURN(Always returns a newly created scope with a handle that is unique from all other scopes that have been created either by the core or the custom layer.  The new scope is a 'basic scope' and is a 'user managed scope'.)
DOC_SEE(destroy_user_managed_scope)
*/
{
    Models *models = (Models*)app->cmd_context;
    Heap *heap = &models->mem.heap;
    Lifetime_Object *object = lifetime_alloc_object(heap, &models->lifetime_allocator, DynamicWorkspace_Unassociated, 0);
    object->workspace.user_back_ptr = object;
    Managed_Scope scope = (Managed_Scope)object->workspace.scope_id;
    return(scope);
}

API_EXPORT bool32
Destroy_User_Managed_Scope(Application_Links *app, Managed_Scope scope)
/*
DOC_PARAM(scope, The handle for the scope which is to be destroyed)
DOC_RETURN(If the scope is a valid 'user managed scope' this function returns non-zero, otherwise it returns zero.)
DOC(This call only operates on 'user managed scopes'.  'User managed scopes' are those scopes created by create_user_managed_scope.  Scopes tied to views, and buffers, and the global scope are not 'user managed scopes'.)
DOC_SEE(create_user_managed_scope)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    if (workspace != 0 && workspace->user_type == DynamicWorkspace_Unassociated){
        Lifetime_Object *lifetime_object = (Lifetime_Object*)workspace->user_back_ptr;
        lifetime_free_object(&models->mem.heap, &models->lifetime_allocator, lifetime_object);
        return(true);
    }
    return(false);
}

API_EXPORT Managed_Scope
Get_Global_Managed_Scope(Application_Links *app)
/*
DOC_RETURN(Always returns the handle to the 'global scope'.  The 'global scope' is unique in that it is not dependent on any object at all.  The handle for the 'global scope' never changes in a 4coder session, and when used as a dependency in get_managed_scope_with_multiple_dependencies it has no effect.)
*/
{
    Models *models = (Models*)app->cmd_context;
    return((Managed_Scope)models->dynamic_workspace.scope_id);
}

internal Lifetime_Object*
get_lifetime_object_from_workspace(Dynamic_Workspace *workspace){
    Lifetime_Object *result = 0;
    switch (workspace->user_type){
        case DynamicWorkspace_Unassociated:
        {
            result = (Lifetime_Object*)workspace->user_back_ptr;
        }break;
        case DynamicWorkspace_Buffer:
        {
            Editing_File *file = (Editing_File*)workspace->user_back_ptr;
            result = file->lifetime_object;
        }break;
        case DynamicWorkspace_View:
        {
            View *vptr = (View*)workspace->user_back_ptr;
            result = vptr->transient.lifetime_object;
        }break;
        default:
        {
            InvalidCodePath;
        }break;
    }
    return(result);
}

API_EXPORT Managed_Scope
Get_Managed_Scope_With_Multiple_Dependencies(Application_Links *app, Managed_Scope *scopes, int32_t count)
/*
DOC_PARAM(scopes, An array of scope handles which are to represent the various dependencies that the returned scope will have.)
DOC_PARAM(count, The number of scopes in the scopes array)
DOC_RETURN(Always returns the handle to a scope with the appropriate dependencies.)
DOC(Scopes with multiple dependencies are a subtle topic, for a full treatment please read
https://4coder.handmade.network/blogs/p/3412-new_features_p3__memory_management_scopes)
*/
{
    Models *models = (Models*)app->cmd_context;
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    Partition *scratch = &models->mem.part;
    
    Temp_Memory temp = begin_temp_memory(scratch);
    
    b32 filled_array = true;
    Lifetime_Object **object_ptr_array = push_array(scratch, Lifetime_Object*, 0);
    for (i32 i = 0; i < count; i += 1){
        Dynamic_Workspace *workspace = get_dynamic_workspace(models, scopes[i]);
        if (workspace == 0){
            filled_array = false;
            break;
        }
        
        switch (workspace->user_type){
            case DynamicWorkspace_Global:
            {
                // NOTE(allen): (global_scope INTERSECT X) == X for all X, therefore we emit nothing when a global group is in the key list.
            }break;
            
            case DynamicWorkspace_Unassociated:
            case DynamicWorkspace_Buffer:
            case DynamicWorkspace_View:
            {
                Lifetime_Object *object = get_lifetime_object_from_workspace(workspace);
                Assert(object != 0);
                Lifetime_Object **new_object_ptr = push_array(scratch, Lifetime_Object*, 1);
                *new_object_ptr = object;
            }break;
            
            case DynamicWorkspace_Intersected:
            {
                Lifetime_Key *key = (Lifetime_Key*)workspace->user_back_ptr;
                if (lifetime_key_check(lifetime_allocator, key)){
                    i32 member_count = key->count;
                    Lifetime_Object **key_member_ptr = key->members;
                    for (i32 j = 0; j < member_count; j += 1, key_member_ptr += 1){
                        Lifetime_Object **new_object_ptr = push_array(scratch, Lifetime_Object*, 1);
                        *new_object_ptr = *key_member_ptr;
                    }
                }
            }break;
            
            default:
            {
                InvalidCodePath;
            }break;
        }
    }
    
    Managed_Scope result = 0;
    if (filled_array){
        i32 member_count = (i32)(push_array(scratch, Lifetime_Object*, 0) - object_ptr_array);
        member_count = lifetime_sort_and_dedup_object_set(object_ptr_array, member_count);
        Heap *heap = &models->mem.heap;
        Lifetime_Key *key = lifetime_get_or_create_intersection_key(heap, lifetime_allocator, object_ptr_array, member_count);
        result = (Managed_Scope)key->dynamic_workspace.scope_id;
    }
    
    end_temp_memory(temp);
    
    return(result);
}

API_EXPORT bool32
Managed_Scope_Clear_Contents(Application_Links *app, Managed_Scope scope)
/*
DOC_PARAM(scope, A handle to the scope which is to be cleared.)
DOC_RETURN(Returns non-zero on succes, and zero on failure.  This call fails when the scope handle does not refer to a valid scope.)
DOC(Clearing the contents of a scope resets all managed variables to have their default values, and frees all managed objects.  The scope handle remains valid and refers to the same scope which still has the same dependencies.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    if (workspace != 0){
        dynamic_workspace_clear_contents(&models->mem.heap, workspace);
        return(true);
    }
    return(false);
}

API_EXPORT bool32
Managed_Scope_Clear_Self_All_Dependent_Scopes(Application_Links *app, Managed_Scope scope)
/*
DOC_PARAM(scope, A handle to the 'basic scope' which is to be cleared.)
DOC_RETURN(Returns non-zero on succes, and zero on failure.  This call fails when the scope handle does not refer to a valid 'basic scope'.)
DOC(The parameter scope must be a 'basic scope' which means a scope with a single dependency.  Scopes tied to buffers and view, and 'user managed scopes' are the 'basic scopes'.  This call treats the scope as an identifier for the single object up which the scope depends, and clears all scopes that depend on that object, this includes clearing 'scope' itself.  The scopes returned from get_managed_scope_with_multiple_dependencies that included 'scope' via the union rule are the only other scopes that can be cleared by this call.
For more on the union rule read the full treatment of managed scopes
https://4coder.handmade.network/blogs/p/3412-new_features_p3__memory_management_scopes)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    if (workspace != 0 && workspace->user_type != DynamicWorkspace_Global && workspace->user_type != DynamicWorkspace_Intersected){
        Lifetime_Object *object = get_lifetime_object_from_workspace(workspace);
        Assert(object != 0);
        lifetime_object_reset(&models->mem.heap, &models->lifetime_allocator, object);
        return(true);
    }
    return(false);
}

API_EXPORT Managed_Variable_ID
Managed_Variable_Create(Application_Links *app, char *null_terminated_name, uint64_t default_value)
/*
DOC_PARAM(null_terminated_name, The unique name for this managed variable.)
DOC_PARAM(default_value, The default value that this variable will have in a scope that has not set this variable.)
DOC_RETURN(Returns the Managed_Variable_ID for the new variable on success and zero on failure.  This call fails when a variable with the given name alraedy exists.)
DOC(Variables are stored in scopes but creating a variable does not mean that all scopes will allocate space for this variable.  Space for variables is allocated sparsly on demand in each scope.  Once a variable exists it will exist for the entire duration of the 4coder session and will never have a different id.  The id will be used to set and get the value of the variable in managed scopes.)
DOC_SEE(managed_variable_get_id)
DOC_SEE(managed_variable_create_or_get_id)
*/
{
    Models *models = (Models*)app->cmd_context;
    String name = make_string_slowly(null_terminated_name);
    Heap *heap = &models->mem.heap;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    return(dynamic_variables_create(heap, layout, name, default_value));
}

API_EXPORT Managed_Variable_ID
Managed_Variable_Get_ID(Application_Links *app, char *null_terminated_name)
/*
DOC_PARAM(null_terminated_name, The unique name for this managed variable.)
DOC_RETURN(Returns the Managed_Variable_ID for the variable on success and zero on failure.  This call fails when no variable that already exists has the given name.)
DOC_SEE(managed_variable_create)
DOC_SEE(managed_variable_create_or_get_id)
*/
{
    Models *models = (Models*)app->cmd_context;
    String name = make_string_slowly(null_terminated_name);
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    return(dynamic_variables_lookup(layout, name));
}

API_EXPORT Managed_Variable_ID
Managed_Variable_Create_Or_Get_ID(Application_Links *app, char *null_terminated_name, uint64_t default_value)
/*
DOC_PARAM(null_terminated_name, The unique name for this managed variable.)
DOC_PARAM(default_value, The default value that this variable will have in a scope that has not set this variable.  This parameter is ignored if the variable already exists.)
DOC_RETURN(Returns the Managed_Variable_ID for the variable on success, this call never fails.)
DOC(This call first tries to get the variable id in the same way that managed_variable_get_id would, if this fails it creates the variable and returns the new id in the way that managed_variable_create would. )
DOC_SEE(managed_variable_create)
DOC_SEE(managed_variable_get_id)
*/
{
    Models *models = (Models*)app->cmd_context;
    String name = make_string_slowly(null_terminated_name);
    Heap *heap = &models->mem.heap;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    return(dynamic_variables_lookup_or_create(heap, layout, name, default_value));
}

internal bool32
get_dynamic_variable__internal(Models *models, Managed_Scope handle, int32_t location, uint64_t **ptr_out){
    Heap *heap = &models->mem.heap;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, handle);
    bool32 result = false;
    if (workspace != 0){
        if (dynamic_variables_get_ptr(heap, &workspace->mem_bank, layout, &workspace->var_block, location, ptr_out)){
            result = true;
        }
    }
    return(result);
}

API_EXPORT bool32
Managed_Variable_Set(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t value)
/*
DOC_PARAM(scope, A handle to the scope in which the value of the given variable will be set.)
DOC_PARAM(id, The id of the variable to set.)
DOC_PARAM(value, The new value of the variable.)
DOC_RETURN(Returns non-zero on success.  This call fails if scope does not refer to a valid managed scope, or id does not refer to an existing managed variable.)
*/
{
    Models *models = (Models*)app->cmd_context;
    u64 *ptr = 0;
    if (get_dynamic_variable__internal(models, scope, id, &ptr)){
        *ptr = value;
        return(true);
    }
    return(false);
}

API_EXPORT bool32
Managed_Variable_Get(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t *value_out)
/*
DOC_PARAM(scope, A handle to the scope from which the value of the given variable will be queried.)
DOC_PARAM(id, The id of the variable to get.)
DOC_PARAM(value_out, An address where the value of the given variable in the given scope will be stored.)
DOC_RETURN(Returns non-zero on success.  This call fails if scope does not refer to a valid managed scope, or id does not refer to an existing managed variable.  If the managed scope and managed variable both exist, but the variable has never been set, then the default value for the variable that was determined when the variable was created is used to fill value_out, this is treated as a success and returns non-zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    u64 *ptr = 0;
    if (get_dynamic_variable__internal(models, scope, id, &ptr)){
        *value_out = *ptr;
        return(true);
    }
    return(false);
}

API_EXPORT Managed_Object
Alloc_Managed_Memory_In_Scope(Application_Links *app, Managed_Scope scope, int32_t item_size, int32_t count)
/*
DOC_PARAM(scope, A handle to the scope in which the new object will be allocated.)
DOC_PARAM(item_size, The size, in bytes, of a single 'item' in this memory object.  This effects the size of the allocation, and the indexing of the memory in the store and load calls.)
DOC_PARAM(count, The number of 'items' allocated for this memory object.  The total memory size is item_size*count.)
DOC_RETURN(Returns the handle to the new object on success, and zero on failure.  This call fails if scope does not refer to a valid managed scope.)
DOC(Managed objects allocate memory that is tied to the scope.  When the scope is cleared or destroyed all of the memory allocated in it is freed in bulk and the handles to the objects never again become valid.  Thus the handle returned by this call will only ever refer to this memory allocation.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Heap *heap = &models->mem.heap;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    Managed_Object result = 0;
    if (workspace != 0){
        int32_t size = count*item_size;
        void *ptr = memory_bank_allocate(heap, &workspace->mem_bank, size + sizeof(Managed_Memory_Header));
        Managed_Memory_Header *header = (Managed_Memory_Header*)ptr;
        header->std_header.type = ManagedObjectType_Memory;
        header->std_header.item_size = item_size;
        header->std_header.count = count;
        u32 id = dynamic_workspace_store_pointer(heap, workspace, ptr);
        result = ((u64)scope << 32) | (u64)id;
    }
    return(result);
}

API_EXPORT Managed_Object
Alloc_Buffer_Markers_On_Buffer(Application_Links *app, Buffer_ID buffer_id, int32_t count, Managed_Scope *optional_extra_scope)
/*
DOC_PARAM(buffer_id, The id for the buffer onto which these markers will be attached.  The markers will live in the scope of the buffer, or in another scope dependent on this buffer, thus guaranteeing that when the buffer is closed, all attached markers are freed in bulk with it.)
DOC_PARAM(count, The number of Marker items allocated in this object.  The total memory size is sizeof(Marker)*count.)
DOC_PARAM(optional_extra_scope, If this pointer is non-null, then it is treated as a scope with additional dependencies for the allocated markers.  In this case, the scope of buffer and extra scope are unioned via get_managed_scope_with_multiple_dependencies and marker object lives in the resulting scope.)
DOC_RETURN(Returns the handle to the new object on succes, and zero on failure.  This call fails if buffer_id does not refer to a valid buffer, or optional_extra_scope does not refer to a valid scope.)
DOC(The created managed object is essentially a memory object with item size equal to sizeof(Marker).  The primary difference is that if the buffer referred to by buffer_id is edited, the position of all markers attached to that buffer can be changed by the core.  Thus this not a memory storage so much as position marking and tracking in a buffer.)
DOC_SEE(alloc_managed_memory_in_scope)
DOC_SEE(Marker)
*/
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Managed_Scope markers_scope = buffer_get_managed_scope__inner(file);
    if (optional_extra_scope != 0){
        Managed_Object scope_array[2];
        scope_array[0] = markers_scope;
        scope_array[1] = *optional_extra_scope;
        markers_scope = Get_Managed_Scope_With_Multiple_Dependencies(app, scope_array, 2);
    }
    Heap *heap = &models->mem.heap;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, markers_scope);
    Managed_Object result = 0;
    if (workspace != 0){
        i32 size = count*sizeof(Marker);
		void *ptr = memory_bank_allocate(heap, &workspace->mem_bank, size + sizeof(Managed_Buffer_Markers_Header));
        Managed_Buffer_Markers_Header *header = (Managed_Buffer_Markers_Header*)ptr;
        header->std_header.type = ManagedObjectType_Markers;
        header->std_header.item_size = sizeof(Marker);
        header->std_header.count = count;
        zdll_push_back(workspace->buffer_markers_list.first, workspace->buffer_markers_list.last, header);
        workspace->buffer_markers_list.count += 1;
        workspace->total_marker_count += count;
        header->buffer_id = buffer_id;
        header->visual_first = 0;
        header->visual_last = 0;
        header->visual_count = 0;
        u32 id = dynamic_workspace_store_pointer(heap, workspace, ptr);
        result = ((u64)markers_scope << 32) | (u64)id;
    }
    return(result);
}

internal Managed_Object_Ptr_And_Workspace
get_dynamic_object_ptrs(Models *models, Managed_Object object){
    Managed_Object_Ptr_And_Workspace result = {};
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    if (workspace != 0){
        u32 lo_id = object&max_u32;
        Managed_Object_Standard_Header *header = (Managed_Object_Standard_Header*)dynamic_workspace_get_pointer(workspace, lo_id);
        if (header != 0){
            result.workspace = workspace;
            result.header = header;
            return(result);
        }
    }
    return(result);
}

API_EXPORT Marker_Visual
Create_Marker_Visual(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, A handle to the marker object on which the new visual will be attached.)
DOC_RETURN(Returns the handle to the newly created marker visual on success, and zero on failure.  This call fails when object does not refer to a valid marker object.)
DOC(A marker visual adds graphical effects to markers such as cursors, highlight ranges, text colors, etc.  A marker object can have any number of attached visuals.  The memory in the 4coder core for visuals is stored in the same scope as the object.)
DOC_SEE(destroy_marker_visuals)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    Marker_Visual visual = {};
    if (object_ptrs.header != 0 && object_ptrs.header->type == ManagedObjectType_Markers){
        Heap *heap = &models->mem.heap;
        Dynamic_Workspace *workspace = object_ptrs.workspace;
        Marker_Visual_Data *data = dynamic_workspace_alloc_visual(heap, &workspace->mem_bank, workspace);
        
        Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)object_ptrs.header;
        zdll_push_back(markers->visual_first, markers->visual_last, data);
        markers->visual_count += 1;
        data->owner_object = object;
        marker_visual_defaults(data);
        
        visual.scope = workspace->scope_id;
        visual.slot_id = data->slot_id;
        visual.gen_id = data->gen_id;
    }
    return(visual);
}

internal Marker_Visual_Data*
get_marker_visual_pointer(Models *models, Marker_Visual visual){
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, visual.scope);
    if (workspace != 0){
        return(dynamic_workspace_get_visual_pointer(workspace, visual.slot_id, visual.gen_id));
    }
    return(0);
}

API_EXPORT bool32
Marker_Visual_Set_Effect(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(type, The new type of visual effect this marker visual will create.)
DOC_PARAM(color, The new color aspect of the effect, exact meaning depends on the type.)
DOC_PARAM(text_color, The new text color aspect of the effect, exact meaning depends on the type.)
DOC_PARAM(text_style, This feature is not yet implemented and the parameter should always be 0.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
DOC(Each effect type uses the color and text_color aspects differently.  These aspects can be specified as 32-bit colors, or as "symbolic coloes" which are special values small enough that their alpha channels would be zero as 32-bit color codes.  Valid symbolic color values have special rules for evaluation, and sometimes their meaning depends on the effect type too.)
DOC_SEE(Marker_Visuals_Type)
DOC_SEE(Marker_Visuals_Symbolic_Color)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    if (data != 0){
        data->type = type;
        data->color = color;
        data->text_color = text_color;
        data->text_style = text_style;
        return(true);
    }
    return(false);
}

API_EXPORT bool32
Marker_Visual_Set_Take_Rule(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(take_rule, The new take rule for the marker visual.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
DOC(Marker visuals have take rules so that they do not necessarily effect every marker in the marker object they were created to visualize.  The take rule can effect the start of the run of markers, the total number of markers, and the stride of the markers, "taken" by this visual when applying it's effect.  The word "take" should not be thought of as reserving a marker to the particular marker visual, multiple visuals may add effects to a single marker.  See the documentation for Marker_Visual_Take_Rule for specifics about how the take rule can be configured.)
DOC_SEE(Marker_Visual_Take_Rule)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    if (data != 0){
        Assert(take_rule.take_count_per_step != 0);
        take_rule.first_index = clamp_bottom(0, take_rule.first_index);
        take_rule.take_count_per_step = clamp_bottom(1, take_rule.take_count_per_step);
        take_rule.step_stride_in_marker_count = clamp_bottom(take_rule.take_count_per_step, take_rule.step_stride_in_marker_count);
        data->take_rule = take_rule;
        if (data->take_rule.maximum_number_of_markers != 0){
            i32 whole_steps = take_rule.maximum_number_of_markers/take_rule.take_count_per_step;
            i32 extra_in_last = take_rule.maximum_number_of_markers%take_rule.take_count_per_step;
            whole_steps = whole_steps + (extra_in_last > 0?1:0);
            data->one_past_last_take_index = take_rule.first_index + whole_steps*take_rule.step_stride_in_marker_count + extra_in_last;
        }
        else{
            data->one_past_last_take_index = max_i32;
        }
        return(true);
    }
    return(false);
}

API_EXPORT bool32
Marker_Visual_Set_Priority(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(priority, The new priority level for this marker visual.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
DOC(Multiple visuals effecting the same position, whether they are on the same marker object, or different marker objects, are sorted by their priority level, so that higher priority levels are displayed when they are in conflict with lower priority levels.  Some effects have implicit priorities over other effects which does not take priority level into account, other effects may occur in the same position without being considered "in conflict".  See the documentation for each effect for more information these relationships.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    if (data != 0){
        data->priority = priority;
        return(true);
    }
    return(false);
}

API_EXPORT bool32
Marker_Visual_Set_View_Key(Application_Links *app, Marker_Visual visual, View_ID key_view_id)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(key_view_id, The new value of the marker visual's view keying.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does notrefer to a valid marker visual.)
DOC(View keying allows a marker visual to declare that it only appears in one view.  For instance, if a buffer is opened in two views side-by-side, and each view has it's own cursor position, this can be used to make sure that the cursor for one view does not appear in the other view.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    if (data != 0){
        data->key_view_id = key_view_id;
        return(true);
    }
    return(false);
}

API_EXPORT bool32
Destroy_Marker_Visual(Application_Links *app, Marker_Visual visual)
/*
DOC_PARAM(visual, A handle to the marker visual to be destroyed.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, visual.scope);
    if (workspace != 0){
        Marker_Visual_Data *data = dynamic_workspace_get_visual_pointer(workspace, visual.slot_id, visual.gen_id);
        if (data != 0){
            void *ptr = dynamic_workspace_get_pointer(workspace, data->owner_object&max_u32);
            Managed_Object_Standard_Header *header = (Managed_Object_Standard_Header*)ptr;
            if (header != 0){
                Assert(header->type == ManagedObjectType_Markers);
                Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)header;
                zdll_remove(markers->visual_first, markers->visual_last, data);
                markers->visual_count -= 1;
                marker_visual_free(&workspace->visual_allocator, data);
                return(true);
            }
        }
    }
    return(false);
}

API_EXPORT int32_t
Buffer_Markers_Get_Attached_Visual_Count(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the marker object to be queried.)
DOC_RETURN(Returns the number of marker visuals that are currently attached to the given object.  If the object handle does not refer to a valid marker object, then this call returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0 && object_ptrs.header->type == ManagedObjectType_Markers){
        Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)object_ptrs.header;
        return(markers->visual_count);
    }
    return(0);
}

API_EXPORT Marker_Visual*
Buffer_Markers_Get_Attached_Visual(Application_Links *app, Partition *part, Managed_Object object)
/*
DOC_PARAM(part, The arena to be used to allocate the returned array.)
DOC_PARAM(object, The handle to the marker object to be queried.)
DOC_RETURN(Pushes an array onto part containing the handle to every marker visual attached to this object, and returns the pointer to it's base.  If the object does not refer to a valid marker object or there is not enough space in part to allocate the array, then a null pointer is returned.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0 && object_ptrs.header->type == ManagedObjectType_Markers){
        Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)object_ptrs.header;
        i32 count = markers->visual_count;
        Marker_Visual *visual = push_array(part, Marker_Visual, count);
        if (visual != 0){
            Marker_Visual *v = visual;
            Managed_Scope scope = object_ptrs.workspace->scope_id;
            for (Marker_Visual_Data *data = markers->visual_first;
                 data != 0;
                 data = data->next, v += 1){
                v->scope = scope;
                v->slot_id = data->slot_id;
                v->gen_id = data->gen_id;
            }
            Assert(v == visual + count);
        }
        return(visual);
    }
    return(0);
}

internal u8*
get_dynamic_object_memory_ptr(Managed_Object_Standard_Header *header){
    if (header != 0){
        if (0 < header->type && header->type < ManagedObjectType_COUNT){
            return(((u8*)header) + managed_header_type_sizes[header->type]);
        }
    }
    return(0);
}

API_EXPORT uint32_t
Managed_Object_Get_Item_Size(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns the size, in bytes, of a single item in the managed object.  Item size is multiplied by the indices in store and load calls, and it is multiplied by item count to discover the total memory size of the managed object.  If object does not refer to a valid managed object, this call returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0){
        return(object_ptrs.header->item_size);
    }
    return(0);
}

API_EXPORT uint32_t
Managed_Object_Get_Item_Count(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns the count of items this object can store, this count is used to range check the indices in store and load calls.  If object does not refer to a valid managed object, this call returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0){
        return(object_ptrs.header->count);
    }
    return(0);
}

API_EXPORT Managed_Object_Type
Managed_Object_Get_Type(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns the type of the managed object, see Managed_Object_Type for the enumeration of possible values and their special meanings.  If object does not refer to a valid managed object, this call returns ManagedObjectType_Error.)
DOC_SEE(Managed_Object_Type)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0){
        Managed_Object_Type type = object_ptrs.header->type;
        if (type < 0 || ManagedObjectType_COUNT <= type){
            type = ManagedObjectType_Error;
        }
        return(type);
    }
    return(ManagedObjectType_Error);
}

API_EXPORT Managed_Scope
Managed_Object_Get_Containing_Scope(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns a handle to the managed scope in which this object is allocated, or zero if object does not refer to a valid managed object.)
*/
{
    Models *models = (Models*)app->cmd_context;
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    if (workspace != 0){
        return((Managed_Scope)hi_id);
    }
    return(0);
}

API_EXPORT bool32
Managed_Object_Free(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be freed.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when object does not refer to a valid managed object.)
DOC(Permanently frees the specified object.  Not only does this free up the memory this object had allocated, but it also triggers cleanup for some types of managed objects.  For instance after markers are freed, any visual effects from the markers are removed as well.  See Managed_Object_Type for more information about what cleanup each type performs when it is freed.)
 */
{
    Models *models = (Models*)app->cmd_context;
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    if (workspace != 0){
        u32 lo_id = object&max_u32;
        u8 *object_ptr = (u8*)dynamic_workspace_get_pointer(workspace, lo_id);
        if (object_ptr != 0){
            Managed_Object_Type *type = (Managed_Object_Type*)object_ptr;
            if (*type == ManagedObjectType_Markers){
                Managed_Buffer_Markers_Header *header = (Managed_Buffer_Markers_Header*)object_ptr;
                workspace->total_marker_count -= header->std_header.count;
                if (header->visual_count > 0){
                    marker_visual_free_chain(&workspace->visual_allocator, header->visual_first, header->visual_last, header->visual_count);
                }
                zdll_remove(workspace->buffer_markers_list.first, workspace->buffer_markers_list.last, header);
                workspace->buffer_markers_list.count -= 1;
            }
            dynamic_workspace_erase_pointer(workspace, lo_id);
            memory_bank_free(&workspace->mem_bank, object_ptr);
            return(true);
        }
    }
    return(false);
}

API_EXPORT bool32
Managed_Object_Store_Data(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem)
/*
DOC_PARAM(object, The handle to the managed object in which data will be stored.)
DOC_PARAM(first_index, The first index of the range in the managed object to be stored.  Managed object indics are zero based.)
DOC_PARAM(count, The number of items in the managed object to be stored.)
DOC_PARAM(mem, A pointer to the data to be stored, it is expected that the size of this memory is item_size*count, item_size can be queried with managed_object_get_item_size.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when object does not refer to a valid managed object, and when the range of indices overflows the range of this managed object.  The range of the managed object can be queried with managed_object_get_item_count.)
DOC(All managed objects, in addition to whatever special behaviors they have, have the ability to store and load data.  This storage can have special properties in certain managed object types, for instance, the data stored in marker objects are edited by the core when the buffer to which they are attached is edited.  This call stores the data pointed to by mem into the item range specified by first_index and count.)
DOC_SEE(managed_object_get_item_size)
DOC_SEE(managed_object_get_item_count)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u8 *ptr = get_dynamic_object_memory_ptr(object_ptrs.header);
    if (ptr != 0){
        u32 item_count = object_ptrs.header->count;
        if (0 <= first_index && first_index + count <= item_count){
            u32 item_size = object_ptrs.header->item_size;
            memcpy(ptr + first_index*item_size, mem, count*item_size);
            heap_assert_good(&object_ptrs.workspace->mem_bank.heap);
            return(true);
        }
    }
    return(false);
}

API_EXPORT bool32
Managed_Object_Load_Data(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem_out)
/*
DOC_PARAM(object, The handle to the managed object  from which data will be loaded.)
DOC_PARAM(first_index, The first index of the range in the managed object to be loaded.  Managed object indics are zero based.)
DOC_PARAM(count, The number of items in the managed object to be loaded.)
DOC_PARAM(mem_out, A pointer to the memory where loaded data will be written, it is expected that the size of this memory is item_size*count, item_size can be queried with managed_object_get_item_size.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when object does not refer to a valid managed object, and when the range of indices overflows the range of this managed object.  The range of the managed object can be queried with managed_object_get_item_count.)
DOC(All managed objects, in addition to whatever special behaviors they have, have the ability to store and load data.  This storage can have special properties in certain managed object types, for instance, the data stored in marker objects are edited by the core when the buffer to which they are attached is edited.  This call loads the data from the item range specified by first_index and count into mem_out.)
DOC_SEE(managed_object_get_item_size)
DOC_SEE(managed_object_get_item_count)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u8 *ptr = get_dynamic_object_memory_ptr(object_ptrs.header);
    if (ptr != 0){
        u32 item_count = object_ptrs.header->count;
        if (0 <= first_index && first_index + count <= item_count){
            u32 item_size = object_ptrs.header->item_size;
            memcpy(mem_out, ptr + first_index*item_size, count*item_size);
            heap_assert_good(&object_ptrs.workspace->mem_bank.heap);
            return(true);
        }
    }
    return(false);
}

API_EXPORT User_Input
Get_User_Input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
/*
DOC_PARAM(get_type, The get_type parameter specifies the set of input types that should be returned.)
DOC_PARAM(abort_type, The get_type parameter specifies the set of input types that should trigger an abort signal.)
DOC_RETURN(This call returns a User_Input that describes a user input event.)
DOC(
This call preempts the command. The command is resumed if either a get or abort condition
is met, or if another command is executed.  If either the abort condition is met or another
command is executed an abort signal is returned.  If an abort signal is ever returned the
command should finish execution without any more calls that preempt the command.
If a get condition is met the user input is returned.
)
DOC_SEE(Input_Type_Flag)
DOC_SEE(User_Input)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Coroutine_Head *coroutine = (Coroutine_Head*)app->current_coroutine;
    User_Input result = {};
    if (app->type_coroutine == Co_Command){
        Assert(coroutine != 0);
        *((u32*)coroutine->out + 0) = get_type;
        *((u32*)coroutine->out + 1) = abort_type;
        system->yield_coroutine(coroutine);
        result = *(User_Input*)coroutine->in;
    }
    return(result);
}

API_EXPORT User_Input
Get_Command_Input(Application_Links *app)
/*
DOC_RETURN(This call returns the input that triggered the currently executing command.)
DOC_SEE(User_Input)
*/{
    Models *models = (Models*)app->cmd_context;
    User_Input result = {};
    result.key = models->key;
    return(result);
}

API_EXPORT void
Set_Command_Input(Application_Links *app, Key_Event_Data key_data)
/*
DOC_PARAM(key_data, The new value of the "command input". Setting this effects the result returned by get_command_input until the end of this command.)
*/{
    Models *models = (Models*)app->cmd_context;
    models->key = key_data;
}

API_EXPORT Mouse_State
Get_Mouse_State(Application_Links *app)
/*
DOC_RETURN(This call returns the current mouse state as of the beginning of the frame.)
DOC_SEE(Mouse_State)
*/{
    Models *models = (Models*)app->cmd_context;
    return(models->input->mouse);
}

API_EXPORT int32_t
Get_Active_Query_Bars(Application_Links *app, View_ID view_id, int32_t max_result_count, Query_Bar **result_array)
/*
DOC_PARAM(view_id, Specifies the view for which query bars should be retrieved.)
DOC_PARAM(max_result_count, Specifies the number of Query_Bar pointers available in result_array.)
DOC_PARAM(result_array, User-supplied empty array of max_result_count Query_Bar pointers.)
DOC_RETURN(This call returns the number of Query_Bar pointers successfully placed in result_array.)
DOC
(
This call allows the customization layer to inspect the set of active Query_Bar slots for a given
view_id.  By convention, the most recent query will be entry 0, the next most recent 1, etc., such
that if you only care about the most recent query bar, you can call Get_Active_Query_Bars with a
max_result_count of 1 and be assured you will get the most recent bar if any exist.
)
*/{
    int32_t result = 0;
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    if (view != 0){
        for (Query_Slot *slot = view->transient.query_set.used_slot;
             slot != 0 && (result < max_result_count);
             slot = slot->next){
            if (slot->query_bar != 0){
                result_array[result++] = slot->query_bar;
            }
        }
    }
    return(result);
}

API_EXPORT bool32
Start_Query_Bar(Application_Links *app, Query_Bar *bar, uint32_t flags)
/*
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
    Models *models = (Models*)app->cmd_context;
    Panel *active_panel = layout_get_active_panel(&models->layout);
    View *active_view = active_panel->view;
    Query_Slot *slot = alloc_query_slot(&active_view->transient.query_set);
    bool32 result = (slot != 0);
    if (result){
        slot->query_bar = bar;
    }
    return(result);
}

API_EXPORT void
End_Query_Bar(Application_Links *app, Query_Bar *bar, uint32_t flags)
/*
DOC_PARAM(bar, This parameter should be a bar pointer of a currently active query bar.)
DOC_PARAM(flags, This parameter is not currently used and should be 0 for now.)
DOC(Stops showing the particular query bar specified by the bar parameter.)
*/{
    Models *models = (Models*)app->cmd_context;
    Panel *active_panel = layout_get_active_panel(&models->layout);
    View *active_view = active_panel->view;
    free_query_slot(&active_view->transient.query_set, bar);
}

API_EXPORT void
Print_Message(Application_Links *app, char *str, int32_t len)
/*
DOC_PARAM(str, The str parameter specifies the string to post to *messages*; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the str string.)
DOC(This call posts a string to the *messages* buffer.)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = models->message_buffer;
    if (file != 0){
        output_file_append(system, models, file, make_string(str, len));
        file_cursor_to_end(system, models, file);
    }
}

API_EXPORT int32_t
Get_Theme_Count(Application_Links *app)
/*
DOC_RETURN(Returns the number of themes that currently exist in the core.)
*/
{
    Models *models = (Models*)app->cmd_context;
    return(models->styles.count);
}

API_EXPORT String
Get_Theme_Name(Application_Links *app, struct Partition *arena, int32_t index)
/*
DOC_PARAM(arena, The arena which will be used to allocate the returned string.)
DOC_PARAM(index, The index of the theme to query.  Index zero always refers to the active theme, all other indices refer to the static copies of available themes.)
DOC_RETURN(On success this call returns a string allocated on arena that is the name of the queried theme, on failure a null string is returned.  This call fails when index is not less than the total number of themes, and when there is not enough space in arena to allocate the return string.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Style_Library *library = &models->styles;
    String str = {};
    if (0 <= index && index < library->count){
        Style *style = &library->styles[index];
        char *mem = push_array(arena, char, style->name.size + 1);
        if (mem != 0){
            str.str = mem;
            str.size = style->name.size;
            str.memory_size = str.size + 1;
            memcpy(str.str, style->name.str, str.size);
            str.str[str.size] = 0;
        }
    }
    return(str);
}

API_EXPORT void
Create_Theme(Application_Links *app, Theme *theme, char *name, int32_t len)
/*
DOC_PARAM(theme, The color data of the new theme.)
DOC_PARAM(name, The name of the new theme. This string need not be null terminated.)
DOC_PARAM(len, The length of the name string.)
DOC(This call creates a new theme.  If the given name is already the name of a string, the old string will be replaced with the new one.  This call does not set the current theme.)
*/{
    Models *models = (Models*)app->cmd_context;
    Style_Library *library = &models->styles;
    String theme_name = make_string(name, len);
    
    i32 count = library->count;
    Style *destination_style = 0;
    Style *style = library->styles + 1;
    for (i32 i = 1; i < count; ++i, ++style){
        if (match(style->name, theme_name)){
            destination_style = style;
            break;
        }
    }
    
    if (destination_style == 0 && library->count < library->max){
        destination_style = &library->styles[library->count++];
        destination_style->name = make_fixed_width_string(destination_style->name_);
        copy(&destination_style->name, make_string(name, len));
        terminate_with_null(&destination_style->name);
    }
    
    memcpy(&destination_style->theme, theme, sizeof(*theme));
}

API_EXPORT void
Change_Theme(Application_Links *app, char *name, int32_t len)
/*
DOC_PARAM(name, The name parameter specifies the name of the theme to begin using; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC(This call changes 4coder's color pallet to one of the built in themes.)
*/{
    Models *models = (Models*)app->cmd_context;
    Style_Library *styles = &models->styles;
    String theme_name = make_string(name, len);
    
    i32 count = styles->count;
    Style *s = styles->styles + 1;
    for (i32 i = 1; i < count; ++i, ++s){
        if (match(s->name, theme_name)){
            styles->styles[0] = *s;
            styles->styles[0].name.str = styles->styles[0].name_;
            break;
        }
    }
}

API_EXPORT bool32
Change_Theme_By_Index(Application_Links *app, int32_t index)
/*
DOC_PARAM(index, The index parameter specifies the index of theme to begin using.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when index is not less than the total number of themes.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Style_Library *styles = &models->styles;
    i32 count = styles->count;
    if (0 <= index && index < count){
        styles->styles[0] = styles->styles[index];
        styles->styles[0].name.str = styles->styles[0].name_;
        return(true);
    }
    return(false);
}

API_EXPORT Face_ID
Get_Largest_Face_ID(Application_Links *app)
/*
DOC_RETURN(Returns the largest face ID that could be valid.  There is no guarantee that the returned value is a valid face, or that every face less than the returned value is valid.  The guarantee is that all valid face ids are in the range between 1 and the return value.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Face_ID result = models->system->font.get_largest_id();
    return(result);
}

API_EXPORT bool32
Set_Global_Face(Application_Links *app, Face_ID id, bool32 apply_to_all_buffers)
/*
DOC_PARAM(id, The id of the face to try to make the global face.)
DOC_PARAM(apply_to_all_buffers, If the face is valid, apply the face to change to all open buffers as well as setting the global default.)
DOC(Tries to set the global default face, which new buffers will use upon creation.)
DOC_RETURN(Returns true if the given id was a valid face and the change was made successfully.)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    
    bool32 did_change = false;
    
    Font_Pointers font = system->font.get_pointers_by_id(id);
    if (font.valid){
        did_change = true;
        
        if (apply_to_all_buffers){
            global_set_font_and_update_files(system, models, id);
        }
        else{
            models->global_font_id = id;
        }
    }
    
    return(did_change);
}

API_EXPORT bool32
Buffer_Set_Face(Application_Links *app, Buffer_Summary *buffer, Face_ID id)
/*
DOC_PARAM(buffer, The buffer on which to change the face.)
DOC_PARAM(id, The id of the face to try to set the buffer to use.)
DOC(Tries to set the buffer's face.)
DOC_RETURN(Returns true if the given id was a valid face and the change was made successfully.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    
    bool32 did_change = false;
    
    if (file != 0){
        System_Functions *system = models->system;
        Font_Pointers font = system->font.get_pointers_by_id(id);
        if (font.valid){
            did_change = true;
            file_set_font(system, models, file, id);
        }
    }
    
    return(did_change);
}

API_EXPORT History_Record_Index
Buffer_History_Newest_Record_Index(Application_Links *app, Buffer_Summary *buffer){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    History_Record_Index result = 0;
    if (file != 0 && history_is_activated(&file->state.history)){
        result = history_get_record_count(&file->state.history);
    }
    return(result);
}

internal void
buffer_history__fill_record_info(Record *record, Record_Info *out){
    out->kind = record->kind;
    out->edit_number = record->edit_number;
    switch (out->kind){
        case RecordKind_Single:
        {
            out->single.string_forward  = make_string(record->single.str_forward , record->single.length_forward );
            out->single.string_backward = make_string(record->single.str_backward, record->single.length_backward);
            out->single.first = record->single.first;
        }break;
        case RecordKind_Batch:
        {
            out->batch.type = record->batch.type;
            out->batch.count = record->batch.count;
        }break;
        case RecordKind_Group:
        {
            out->group.count = record->group.count;
        }break;
        default:
        {
            InvalidCodePath;
        }break;
    }
}

API_EXPORT Record_Info
Buffer_History_Get_Record_Info(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    Record_Info result = {};
    if (file != 0){
        History *history = &file->state.history;
        if (history_is_activated(history)){
            i32 max_index = history_get_record_count(history);
            if (0 <= index && index <= max_index){
                if (0 < index){
                    Record *record = history_get_record(history, index);
                    buffer_history__fill_record_info(record, &result);
                }
                else{
                    result.error = RecordError_InitialStateDummyRecord;
                }
            }
            else{
                result.error = RecordError_IndexOutOfBounds;
            }
        }
        else{
            result.error = RecordError_NoHistoryAttached;
        }
    }
    else{
        result.error = RecordError_InvalidBuffer;
    }
    return(result);
}

API_EXPORT Record_Info
Buffer_History_Get_Group_Sub_Record(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index, int32_t sub_index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    Record_Info result = {};
    if (file != 0){
        History *history = &file->state.history;
        if (history_is_activated(history)){
            i32 max_index = history_get_record_count(history);
            if (0 <= index && index <= max_index){
                if (0 < index){
                    Record *record = history_get_record(history, index);
                    if (record->kind == RecordKind_Group){
                        record = history_get_sub_record(record, sub_index);
                        buffer_history__fill_record_info(record, &result);
                    }
                    else{
                        result.error = RecordError_WrongRecordTypeAtIndex;
                    }
                }
                else{
                    result.error = RecordError_InitialStateDummyRecord;
                }
            }
            else{
                result.error = RecordError_IndexOutOfBounds;
            }
        }
        else{
            result.error = RecordError_NoHistoryAttached;
        }
    }
    else{
        result.error = RecordError_InvalidBuffer;
    }
    return(result);
}

API_EXPORT History_Record_Index
Buffer_History_Get_Current_State_Index(Application_Links *app, Buffer_Summary *buffer){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    History_Record_Index result = 0;
    if (file != 0 && history_is_activated(&file->state.history)){
        result = file_get_current_record_index(file);
    }
    return(result);
}

API_EXPORT bool32
Buffer_History_Set_Current_State_Index(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0 && history_is_activated(&file->state.history)){
        i32 max_index = history_get_record_count(&file->state.history);
        if (0 <= index && index <= max_index){
            System_Functions *system = models->system;
            edit_change_current_history_state(system, models, file, index);
            result = true;
        }
    }
    return(result);
}

API_EXPORT bool32
Buffer_History_Merge_Record_Range(Application_Links *app, Buffer_Summary *buffer, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0 && history_is_activated(&file->state.history)){
        History *history = &file->state.history;
        i32 max_index = history_get_record_count(history);
        first_index = clamp_bottom(1, first_index);
        if (first_index <= last_index && last_index <= max_index){
            i32 current_index = file->state.current_record_index;
            if (first_index <= current_index && current_index < last_index){
                System_Functions *system = models->system;
                u32 in_range_handler = flags & (bit_0 | bit_1);
                switch (in_range_handler){
                    case RecordMergeFlag_StateInRange_MoveStateForward:
                    {
                        edit_change_current_history_state(system, models, file, last_index);
                        current_index = last_index;
                    }break;
                    
                    case RecordMergeFlag_StateInRange_MoveStateBackward:
                    {
                        edit_change_current_history_state(system, models, file, first_index);
                        current_index = first_index;
                    }break;
                    
                    case RecordMergeFlag_StateInRange_ErrorOut:
                    {
                        goto done;
                    }break;
                }
            }
            if (first_index < last_index){
                history_merge_records(&models->mem.part, &models->mem.heap, history, first_index, last_index);
            }
            if (current_index >= last_index){
                current_index -= (last_index - first_index);
            }
            file->state.current_record_index = current_index;
            result = true;
        }
    }
    done:;
    return(result);
}

API_EXPORT bool32
Buffer_History_Clear_After_Current_State(Application_Links *app, Buffer_Summary *buffer){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    bool32 result = false;
    if (file != 0 && history_is_activated(&file->state.history)){
        history_dump_records_after_index(&file->state.history, file->state.current_record_index);
        result = true;
    }
    return(result);
}

API_EXPORT void
Global_History_Edit_Group_Begin(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, 1);
}

API_EXPORT void
Global_History_Edit_Group_End(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, -1);
}

internal void
font_pointers_to_face_description(Font_Pointers font, Face_Description *description){
    Font_Metrics *metrics = font.metrics;
    i32 len = str_size(metrics->name);
    memcpy(description->font.name, metrics->name, len);
    
    Font_Settings *settings = font.settings;
    description->font.in_local_font_folder = settings->stub.in_font_folder;
    description->pt_size = settings->parameters.pt_size;
    description->bold = settings->parameters.bold;
    description->italic = settings->parameters.italics;
    description->underline = settings->parameters.underline;
    description->hinting = settings->parameters.use_hinting;
}

internal b32
face_description_to_settings(System_Functions *system, Face_Description description, Font_Settings *settings){
    b32 success = false;
    
    if (description.font.in_local_font_folder){
        i32 count = system->font.get_loadable_count();
        for (i32 i = 0; i < count; ++i){
            Font_Loadable_Description loadable = {};
            system->font.get_loadable(i, &loadable);
            
            if (loadable.valid){
                if (!loadable.stub.in_font_folder){
                    break;
                }
                
                if (match(make_string(loadable.display_name, loadable.display_len), description.font.name)){
                    success = true;
                    memcpy(&settings->stub, &loadable.stub, sizeof(settings->stub));
                    break;
                }
            }
        }
    }
    else{
        success = true;
        
        settings->stub.load_from_path = false;
        settings->stub.in_font_folder = false;
        settings->stub.len = str_size(description.font.name);
        memcpy(settings->stub.name, description.font.name, settings->stub.len + 1);
    }
    
    if (success){
        settings->parameters.pt_size = description.pt_size;
        settings->parameters.italics = description.italic;
        settings->parameters.bold = description.bold;
        settings->parameters.underline = description.underline;
        settings->parameters.use_hinting = description.hinting;
    }
    
    return(success);
}

API_EXPORT Face_Description
Get_Face_Description(Application_Links *app, Face_ID id)
/*
DOC_PARAM(id, The face slot from which to read a description.  If zero gets default values.)
DOC(Fills out the values of a Face_Description struct, which includes all the information that determines the appearance of the face.  If the id does not specify a valid face the description will be invalid.  An invalid description has a zero length string in it's font.name field (i.e. description.font.name[0] == 0), and a valid description always contains a non-zero length string in the font.name field (i.e. description.font.name[0] != 0)

If the input id is zero, the description returned will be invalid, but the pt_size and hinting fields will reflect the default values for those fields as specified on the command line.  The default values, if unspecified, are pt_size=0 and hinting=false.  These default values are overriden by config.4coder when instantiating fonts at startup, but the original values of pt_size=0 and hinting=false from the command line are preserved and returned here for the lifetime of the program.

Note that the id of zero is reserved and is never a valid face.)
DOC_RETURN(Returns a Face_Description that is valid if the id references a valid face slot and is filled with the description of the face.  Otherwise returns an invalid Face_Description.)
DOC_SEE(Face_Description)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Face_Description description = {};
    if (id != 0){
        Font_Pointers font = system->font.get_pointers_by_id(id);
        if (font.valid){
            font_pointers_to_face_description(font, &description);
            Assert(description.font.name[0] != 0);
        }
    }
    else{
        description.pt_size = models->settings.font_size;
        description.hinting = models->settings.use_hinting;
    }
    return(description);
}

API_EXPORT Face_ID
Get_Face_ID(Application_Links *app, Buffer_Summary *buffer)
/*
DOC_PARAM(buffer, The buffer from which to get a face id.  If NULL gets global face id.)
DOC(Retrieves a face id if buffer is a valid Buffer_Summary.  If buffer is set to NULL, the parameter is ignored and the global default face is returned.)
DOC_RETURN(On success a valid Face_ID, otherwise returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Face_ID id = 0;
    if (buffer != 0){
        Editing_File *file = imp_get_file(models, buffer);
        if (file != 0){
            id = file->settings.font_id;
        }
    }
    else{
        id = models->global_font_id;
    }
    return(id);
}

API_EXPORT Face_ID
Try_Create_New_Face(Application_Links *app, Face_Description *description)
/*
DOC_PARAM(description, A description of the new face to try to create.)
DOC(Attempts to create a new face and configure it with the provided description.  This call can fail for a number of reasons:

- If the description's font field is not one of the available fonts no face is created.

- If the specified font cannot actually be loaded by the 4coder font system no face is created.

- If the specified font with the specified configuration is too large or too small no face is created.

Note, not all fonts will support all styles.  The fields for italic, bold, underline, and hinting, are only requests that the system try to apply these configurations, but if any cannot be done the face will still be created but without the unsupported configurations.  4coder does not try to simulate the effects of missing styles.)
DOC_RETURN(Returns a new valid face id if the font system successfully instanatiates the new face, otherwise returns zero.  Note that zero is a reserved id and is never a valid id.)
DOC_SEE(Face_Description)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Face_ID id = 0;
    Font_Settings settings;
    if (face_description_to_settings(system, *description, &settings)){
        id = system->font.face_allocate_and_init(&settings);
    }
    return(id);
}

API_EXPORT bool32
Try_Modify_Face(Application_Links *app, Face_ID id, Face_Description *description)
/*
DOC_PARAM(id, The id of the face slot to try to modify.)
DOC_PARAM(description, The new description for the face slot to use.)
DOC(Attempts to modify the face in a particular face slot.  If successful all buffers using the face will continue using the same face slot, and will therefore change appearance to the new configuration of the face slot.

This call can fail for all the same reasons that try_create_new_face can fail, and the same rules about failure to apply specific styles also apply.  If the call does fail, the original configuration of the face slot stays in place.  A valid face slot should never become invalid except by releasing it.

Performance Warning: Modifying a face slot should only be done a couple of times per frame in most cases.  Not only does this call reconfigure a slot, it also recomputes the layout for all buffers that use this face slot.)
DOC_RETURN(Returns true on success and false on failure.)
DOC_SEE(try_create_new_face)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    bool32 success = false;
    Font_Settings settings;
    if (face_description_to_settings(system, *description, &settings)){
        if (alter_font_and_update_files(system, models, id, &settings)){
            success = true;
        }
    }
    return(success);
}

API_EXPORT bool32
Try_Release_Face(Application_Links *app, Face_ID id, Face_ID replacement_id)
/*
DOC_PARAM(id, The id of the face slot to release.)
DOC_PARAM(replacement_id, Optional.  Specifies what buffers that were using id should switch to after the release.)
DOC(Attempts to release the slot referred to by id.  If successful all buffers using the face will switch to using a new valid face, and will therefore change appearance to the new face slot.  If replacement_id refers to a valid face slot, it will be used for the new slot, otherwise the slot is chosen arbitrarily out of the remaining valid faces.

This call can fail if the id does not name a valid face slot, or if there is only one face slot left in the system.

Performance Warning: Releasing a face slot should only be done a couple of times per frame in most cases.  Not only does it release all the resources used by the slot, it also recomputes the layout for all buffers that used the released slot.  If no buffers use the slots that are released, it is generally okay to use it more frequently.)
DOC_RETURN(Returns true on success and zero on failure.)
*/
{
    Models *models = (Models*)app->cmd_context;
    bool32 success = release_font_and_update_files(models->system, models, id, replacement_id);
    return(success);
}

API_EXPORT int32_t
Get_Available_Font_Count(Application_Links *app)
/*
DOC(An available font is a font that the 4coder font system detected on initialization.  Available fonts either come from the font folder in the same path as the 4ed executable, or from the system fonts.  Attempting to load fonts not in returned by available fonts will likely fail, but is permitted.  Available fonts are not updated after initialization.  Just because a font is returned by the available font system does not necessarily mean that it can be loaded.)
DOC_RETURN(Returns the number of available fonts that the user can query.)
*/
{
    Models *models = (Models*)app->cmd_context;
    i32 count = models->system->font.get_loadable_count();
    return(count);
}

API_EXPORT Available_Font
Get_Available_Font(Application_Links *app, int32_t index)
/*
DOC_PARAM(index, The index of the available font to retrieve.  Must be in the range [0,count-1] where count is the value returned by get_available_font_count.)
DOC_RETURN(Returns a valid Available_Font if index is in the required range.  Otherwise returns an invalid Available_Font.  An Available_Font is valid if and only if it's name field contains a string with a non-zero length (i.e. font.name[0] != 0))
DOC_SEE(get_available_font_count)
*/
{
    Models *models = (Models*)app->cmd_context;
    Available_Font available = {};
    Font_Loadable_Description description = {};
    models->system->font.get_loadable(index, &description);
    if (description.valid){
        memcpy(available.name, description.display_name, description.display_len);
        available.in_local_font_folder = description.stub.in_font_folder;
    }
    return(available);
}

API_EXPORT void
Set_Theme_Colors(Application_Links *app, Theme_Color *colors, int32_t count)
/*
DOC_PARAM(colors, The colors pointer provides an array of color structs pairing differet style tags to color codes.)
DOC_PARAM(count, The count parameter specifies the number of Theme_Color structs in the colors array.)
DOC(For each struct in the array, the slot in the main color pallet specified by the struct's tag is set to the color code in the struct. If the tag value is invalid no change is made to the color pallet.)
DOC_SEE(Theme_Color)
*/{
    Models *models = (Models*)app->cmd_context;
    Style *style = &models->styles.styles[0];
    Theme_Color *theme_color = colors;
    for (i32 i = 0; i < count; ++i, ++theme_color){
        if (theme_color->tag < Stag_COUNT){
            style->theme.colors[theme_color->tag] = theme_color->color;
        }
    }
}

API_EXPORT void
Get_Theme_Colors(Application_Links *app, Theme_Color *colors, int32_t count)
/*
DOC_PARAM(colors, an array of color structs listing style tags to get color values for)
DOC_PARAM(count, the number of color structs in the colors array)
DOC(For each struct in the array, the color field of the struct is filled with the color from the slot in the main color pallet specified by the tag.  If the tag value is invalid the color is filled with black.)
DOC_SEE(Theme_Color)
*/{
    Models *models = (Models*)app->cmd_context;
    Style *style = &models->styles.styles[0];
    Theme_Color *theme_color = colors;
    for (i32 i = 0; i < count; ++i, ++theme_color){
        if (theme_color->tag < Stag_COUNT){
            theme_color->color = style->theme.colors[theme_color->tag];
        }
        else{
            theme_color->color = 0xFF000000;
        }
    }
}

API_EXPORT int32_t
Directory_Get_Hot(Application_Links *app, char *out, int32_t capacity)
/*
DOC_PARAM(out, On success this character buffer is filled with the 4coder 'hot directory'.)
DOC_PARAM(capacity, Specifies the capacity in bytes of the of the out buffer.)
DOC(4coder has a concept of a 'hot directory' which is the directory most recently accessed in the GUI.  Whenever the GUI is opened it shows the hot directory. In the future this will be deprecated and eliminated in favor of more flexible hot directories created and controlled in the custom layer.)
DOC_RETURN(This call returns the length of the hot directory string whether or not it was successfully copied into the output buffer.  The call is successful if and only if capacity is greater than or equal to the return value.)
DOC_SEE(directory_set_hot)
*/{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    hot_directory_clean_end(hot);
    if (capacity >= hot->string.size){
        memcpy(out, hot->string.str, hot->string.size);
        if (capacity > hot->string.size){
            out[hot->string.size] = 0;
        }
    }
    return(hot->string.size);
}

API_EXPORT bool32
Directory_Set_Hot(Application_Links *app, char *str, int32_t len)
/*
DOC_PARAM(str, The new value of the hot directory.  This does not need to be a null terminated string.)
DOC_PARAM(len, The length of str in bytes.)
DOC_RETURN(Returns non-zero on success.)
DOC(4coder has a concept of a 'hot directory' which is the directory most recently accessed in the GUI.  Whenever the GUI is opened it shows the hot directory. In the future this will be deprecated and eliminated in favor of more flexible directories controlled on the custom side.)
DOC_SEE(directory_get_hot)
*/{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    b32 success = false;
    if (len < hot->string.memory_size){
        hot_directory_set(models->system, hot, make_string(str, len));
        success = true;
    }
    return(success);
}

API_EXPORT File_List
Get_File_List(Application_Links *app, char *dir, int32_t len)
/*
DOC_PARAM(dir, This parameter specifies the directory whose files will be enumerated in the returned list; it need not be null terminated.)
DOC_PARAM(len, This parameter the length of the dir string.)
DOC_RETURN(This call returns a File_List struct containing pointers to the names of the files in the specified directory.  The File_List returned should be passed to free_file_list when it is no longer in use.)
DOC_SEE(File_List)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Partition *part = &models->mem.part;
    File_List result = {};
    Editing_File_Name canon = {};
    if (get_canon_name(system, make_string(dir, len), &canon)){
        Temp_Memory temp = begin_temp_memory(part);
        String str = push_string(part, canon.name.str, canon.name.size);
        system->set_file_list(&result, str.str, 0, 0, 0);
        end_temp_memory(temp);
    }
    return(result);
}

API_EXPORT void
Free_File_List(Application_Links *app, File_List list)
/*
DOC_PARAM(list, This parameter provides the file list to be freed.)
DOC(After this call the file list passed in should not be read or written to.)
DOC_SEE(File_List)
*/{
    Models *models = (Models*)app->cmd_context;
    models->system->set_file_list(&list, 0, 0, 0, 0);
}

API_EXPORT void
Set_GUI_Up_Down_Keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier)
/*
DOC_PARAM(up_key, the code of the key that should be interpreted as an up key)
DOC_PARAM(up_key_modifier, the modifier for the key that should be interpreted as an up key)
DOC_PARAM(down_key, the code of the key that should be interpreted as a down key)
DOC_PARAM(down_key_modifier, the modifier for the key that should be interpreted as a down key)

DOC(This is a temporary ad-hoc solution to allow some customization of the behavior of the built in GUI. There is a high chance that it will be removed and not replaced at some point, so it is not recommended that it be heavily used.) */
{
    Models *models = (Models*)app->cmd_context;
    models->user_up_key = up_key;
    models->user_up_key_modifier = up_key_modifier;
    models->user_down_key = down_key;
    models->user_down_key_modifier = down_key_modifier;
}

API_EXPORT void*
Memory_Allocate(Application_Links *app, int32_t size)
/*
DOC_PARAM(size, The size in bytes of the block that should be returned.)
DOC(This calls to a low level OS allocator which means it is best used for infrequent, large allocations.  The size of the block must be remembered if it will be freed or if it's mem protection status will be changed.)
DOC_SEE(memory_free)
*/{
    Models *models = (Models*)app->cmd_context;
    void *result = models->system->memory_allocate(size);
    return(result);
}

API_EXPORT bool32
Memory_Set_Protection(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags)
/*
DOC_PARAM(ptr, The base of the block on which to set memory protection flags.)
DOC_PARAM(size, The size that was originally used to allocate this block.)
DOC_PARAM(flags, The new memory protection flags.)
DOC(This call sets the memory protection flags of a block of memory that was previously allocate by memory_allocate.)
DOC_SEE(memory_allocate)
DOC_SEE(Memory_Protect_Flags)
*/{
    Models *models = (Models*)app->cmd_context;
    bool32 result = models->system->memory_set_protection(ptr, size, flags);
    return(result);
}

API_EXPORT void
Memory_Free(Application_Links *app, void *ptr, int32_t size)
/*
DOC_PARAM(mem, The base of a block to free.)
DOC_PARAM(size, The size that was originally used to allocate this block.)
DOC(This call frees a block of memory that was previously allocated by memory_allocate.)
DOC_SEE(memory_allocate)
*/{
    Models *models = (Models*)app->cmd_context;
    models->system->memory_free(ptr, size);
}

API_EXPORT bool32
File_Exists(Application_Links *app, char *filename, int32_t len)
/*
DOC_PARAM(filename, This parameter specifies the full path to a file; it need not be null terminated.)
DOC_PARAM(len, This parameter specifies the length of the filename string.)
DOC_RETURN(This call returns non-zero if and only if the file exists.)
*/{
    Models *models = (Models*)app->cmd_context;
    bool32 result = models->system->file_exists(filename, len);
    return(result);
}

API_EXPORT bool32
Directory_CD(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len)
/*
DOC_PARAM(dir, This parameter provides a character buffer that stores a directory; it need not be null terminated.)
DOC_PARAM(len, This parameter specifies the length of the dir string.)
DOC_PARAM(capacity, This parameter specifies the maximum size of the dir string.)
DOC_PARAM(rel_path, This parameter specifies the path to change to, may include '.' or '..'; it need not be null terminated.)
DOC_PARAM(rel_len, This parameter specifies the length of the rel_path string.)
DOC_RETURN(This call returns non-zero if the call succeeds.)
DOC(
This call succeeds if the new directory exists and it fits inside the dir buffer. If the call succeeds the dir buffer is filled with the new directory and len is overwritten with the length of the new string in the buffer.

For instance if dir contains "C:/Users/MySelf" and rel is "Documents" the buffer will contain "C:/Users/MySelf/Documents" and len will contain the length of that string.  This call can also be used with rel set to ".." to traverse to parent folders.
)*/{
    Models *models = (Models*)app->cmd_context;
    bool32 result = models->system->directory_cd(dir, len, capacity, rel_path, rel_len);
    return(result);
}

API_EXPORT int32_t
Get_4ed_Path(Application_Links *app, char *out, int32_t capacity)
/*
DOC_PARAM(out, This parameter provides a character buffer that receives the path to the 4ed executable file.)
DOC_PARAM(capacity, This parameter specifies the maximum capacity of the out buffer.)
DOC_RETURN(This call returns non-zero on success.)
*/{
    Models *models = (Models*)app->cmd_context;
    int32_t result = models->system->get_4ed_path(out, capacity);
    return(result);
}

// TODO(allen): do(add a "shown but auto-hides on timer" setting for cursor show type)
API_EXPORT void
Show_Mouse_Cursor(Application_Links *app, Mouse_Cursor_Show_Type show)
/*
DOC_PARAM(show, This parameter specifies the new state of the mouse cursor.)
DOC_SEE(Mouse_Cursor_Show_Type)
*/{
    Models *models = (Models*)app->cmd_context;
    models->system->show_mouse_cursor(show);
}

API_EXPORT bool32
Set_Fullscreen(Application_Links *app, bool32 full_screen)
/*
DOC_PARAM(full_screen, The new value of the global full_screen setting.)
DOC(This call tells 4coder to set the full_screen mode.  The change to full screen mode does not take effect until the end of the current frame.  But is_fullscreen does report the new state.)
*/{
    Models *models = (Models*)app->cmd_context;
    bool32 success = models->system->set_fullscreen(full_screen);
    if (!success){
        print_message(app, literal("ERROR: Failed to go fullscreen.\n"));
    }
    return(success);
}

API_EXPORT bool32
Is_Fullscreen(Application_Links *app)
/*
DOC(This call returns true if the 4coder is in full screen mode.  This call takes toggles that have already occured this frame into account.  So it may return true even though the frame has not ended and actually put 4coder into full screen. If it returns true though, 4coder will definitely be full screen by the beginning of the next frame if the state is not changed.)
*/{
    Models *models = (Models*)app->cmd_context;
    bool32 result = models->system->is_fullscreen();
    return(result);
}

API_EXPORT void
Send_Exit_Signal(Application_Links *app)
/*
DOC(This call sends a signal to 4coder to attempt to exit, which will trigger the exit hook before the end of the frame.  That hook will have the chance to cancel the exit.

In the default behavior of the exit hook, the exit is cancelled if there are unsaved changes, and instead a UI querying the user for permission to close without saving is presented, if the user confirms the UI sends another exit signal that will not be canceled.

To make send_exit_signal exit no matter what, setup your hook in such a way that it knows when you are trying to exit no matter what, such as with a global variable that you set before calling send_exit_signal.)
*/{
    Models *models = (Models*)app->cmd_context;
    models->keep_playing = false;
}

API_EXPORT void
Set_Window_Title(Application_Links *app, char *title)
/*
DOC_PARAM(title, A null terminated string indicating the new title for the 4coder window.)
DOC(Sets 4coder's window title to the specified title string.)
*/{
    Models *models = (Models*)app->cmd_context;
    models->has_new_title = true;
    String dst = make_string_cap(models->title_space, 0, models->title_capacity);
    append(&dst, title);
    terminate_with_null(&dst);
}

API_EXPORT Microsecond_Time_Stamp
Get_Microseconds_Timestamp(Application_Links *app)
/*
DOC(Returns a microsecond resolution timestamp.)
*/
{
    // TODO(allen): do(decrease indirection in API calls)
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(system->now_time());
}

internal void
draw_helper__view_space_to_screen_space(Models *models, i32 *x, i32 *y){
    i32_Rect region = models->render_rect;
    *x += region.x0;
    *y += region.y0;
}

internal void
draw_helper__view_space_to_screen_space(Models *models, f32_Rect *rect){
    i32_Rect region = models->render_rect;
    f32 x_corner = (f32)region.x0;
    f32 y_corner = (f32)region.y0;
    rect->x0 += x_corner;
    rect->y0 += y_corner;
    rect->x1 += x_corner;
    rect->y1 += y_corner;
}

// TODO(allen): do(Documentation for draw_* related calls)

API_EXPORT float
Draw_String(Application_Links *app, Face_ID font_id, String str, int32_t x, int32_t y, int_color color, uint32_t flags, float dx, float dy)
{
    Models *models = (Models*)app->cmd_context;
    if (models->render_view == 0){
        float w = font_string_width(models->system, models->target, font_id, str);
        return(w);
    }
    Style *style = &models->styles.styles[0];
    Theme *theme_data = &style->theme;
    
    draw_helper__view_space_to_screen_space(models, &x, &y);
    
    u32 actual_color = finalize_color(theme_data, color);
    float w = draw_string_base(models->system, models->target, font_id, str, x, y,
                               actual_color, flags, dx, dy);
    return(w);
}

API_EXPORT float
Get_String_Advance(Application_Links *app, Face_ID font_id, String str)
{
    Models *models = (Models*)app->cmd_context;
    float w = font_string_width(models->system, models->target, font_id, str);
    return(w);
}

API_EXPORT void
Draw_Rectangle(Application_Links *app, f32_Rect rect, int_color color)
{
    Models *models = (Models*)app->cmd_context;
    if (models->render_view == 0){
        return;
    }
    
    Style *style = &models->styles.styles[0];
    Theme *theme_data = &style->theme;
    
    draw_helper__view_space_to_screen_space(models, &rect);
    
    u32 actual_color = finalize_color(theme_data, color);
    draw_rectangle(models->target, rect, actual_color);
}

API_EXPORT void
Draw_Rectangle_Outline(Application_Links *app, f32_Rect rect, int_color color)
{
    Models *models = (Models*)app->cmd_context;
    if (models->render_view == 0){
        return;
    }
    
    Style *style = &models->styles.styles[0];
    Theme *theme_data = &style->theme;
    
    draw_helper__view_space_to_screen_space(models, &rect);
    
    u32 actual_color = finalize_color(theme_data, color);
    draw_rectangle_outline(models->target, rect, actual_color);
}

API_EXPORT Face_ID
Get_Default_Font_For_View(Application_Links *app, View_ID view_id)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    Face_ID face_id = file->settings.font_id;
    return(face_id);
}

// BOTTOM
