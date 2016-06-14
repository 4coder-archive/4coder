/*
The implementation for the custom API
*/

// TOP

internal void
fill_buffer_summary(Buffer_Summary *buffer, Editing_File *file, Working_Set *working_set){
    *buffer = buffer_summary_zero();
    if (!file->is_dummy){
        buffer->exists = 1;
        buffer->ready = file_is_ready(file);
        
        buffer->is_lexed = file->settings.tokens_exist;
        buffer->buffer_id = file->id.id;
        buffer->size = file->state.buffer.size;
        buffer->buffer_cursor_pos = file->state.cursor_pos;
        
        buffer->file_name_len = file->name.source_path.size;
        buffer->buffer_name_len = file->name.live_name.size;
        buffer->file_name = file->name.source_path.str;
        buffer->buffer_name = file->name.live_name.str;
        
        buffer->map_id = file->settings.base_map_id;
    }
}

internal void
fill_view_summary(View_Summary *view, View *vptr, Live_Views *live_set, Working_Set *working_set){
    i32 lock_level;
    int buffer_id;
    *view = view_summary_zero();
    
    if (vptr->in_use){
        view->exists = 1;
        view->view_id = (int)(vptr - live_set->views) + 1;
        view->line_height = (float)(vptr->line_height);
        view->unwrapped_lines = vptr->file_data.unwrapped_lines;
        view->show_whitespace = vptr->file_data.show_whitespace;
        
        
        if (vptr->file_data.file){
            lock_level = view_lock_level(vptr);
            buffer_id = vptr->file_data.file->id.id;
            
            if (lock_level <= 0){
                view->buffer_id = buffer_id;
            }
            else{
                view->buffer_id = 0;
            }
            
            if (lock_level <= 1){
                view->locked_buffer_id = buffer_id;
            }
            else{
                view->locked_buffer_id = 0;
            }
            
            if (lock_level <= 2){
                view->hidden_buffer_id = buffer_id;
            }
            else{
                view->hidden_buffer_id = 0;
            }
            
            view->mark = view_compute_cursor_from_pos(vptr, vptr->recent->mark);
            view->cursor = vptr->recent->cursor;
            view->preferred_x = vptr->recent->preferred_x;
            
            view->file_region = vptr->file_region;
            view->scroll_vars = *vptr->current_scroll;
        }
    }
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

String
make_string_terminated(Partition *part, char *str, int len){
    char *space = (char*)push_array(part, char, len + 1);
    String string = make_string(str, len, len+1);
    copy_fast_unsafe(space, string);
    string.str = space;
    terminate_with_null(&string);
    return(string);
}

EXEC_COMMAND_SIG(external_exec_command){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Command_Function function = command_table[command_id];
    Command_Binding binding = {};
    binding.function = function;
    if (function) function(cmd->system, cmd, binding);
    
    update_command_data(cmd->vars, cmd);
}

// TODO(allen): This is a bit of a mess and needs to be fixed soon
EXEC_SYSTEM_COMMAND_SIG(external_exec_system_command){
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
    
    View *vptr = 0;
    
    int result = true;
    
    if (view->exists){
        Live_Views *live_set = cmd->live_set;
        i32 view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
        }
    }
    
    if (vptr == 0){
        result = false;
        goto done;
    }
    
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
            View_Iter iter;
            i32 i;
            
            for (i = 0; i < proc_count; ++i){
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
                file_clear(system, models, file, 1);
                file->settings.unimportant = 1;
                
                if (!(flags & CLI_AlwaysBindToView)){
                    iter = file_view_iter_init(&models->layout, file, 0);
                    if (file_view_iter_good(iter)){
                        bind_to_new_view = 0;
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
            
            if (bind_to_new_view){
                view_set_file(vptr, file, models);
                view_show_file(vptr);
            }
            
            proc = procs + vars->cli_processes.count++;
            proc->out_file = file;
            
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

DIRECTORY_GET_HOT_SIG(external_directory_get_hot){
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

#define external_get_4ed_path system->get_4ed_path

#define external_file_exists  system->file_exists

#define external_directory_cd system->directory_cd

GET_FILE_LIST_SIG(external_get_file_list){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    File_List result = {};
    system->set_file_list(&result, make_string(dir, len));
    return(result);
}

FREE_FILE_LIST_SIG(external_free_file_list){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    system->set_file_list(&list, make_string(0, 0));
}

CLIPBOARD_POST_SIG(external_clipboard_post){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    General_Memory *general = &models->mem.general;
    Working_Set *working = &models->working_set;
    int result = false;
    
    String *dest = working_set_next_clipboard_string(general, working, len);
    copy(dest, make_string(str, len));
    system->post_clipboard(*dest);
    
    return(result);
}

CLIPBOARD_COUNT_SIG(external_clipboard_count){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working = &cmd->models->working_set;
    int count = working->clipboard_size;
    return(count);
}

CLIPBOARD_INDEX_SIG(external_clipboard_index){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working = &cmd->models->working_set;
    
    int size = 0;
    String *str = working_set_clipboard_index(working, index);
    if (str){
        size = str->size;
        if (out){
            String out_str = make_string(out, 0, len);
            copy(&out_str, *str);
        }
    }
    
    return(size);
}

GET_BUFFER_FIRST_SIG(external_get_buffer_first){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
    Buffer_Summary result = {};
    if (working_set->file_count > 0){
        fill_buffer_summary(&result, (Editing_File*)working_set->used_sentinel.next, working_set);
    }
    return(result);
}

GET_BUFFER_NEXT_SIG(external_get_buffer_next){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
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

GET_BUFFER_SIG(external_get_buffer){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Working_Set *working_set = &cmd->models->working_set;
    Buffer_Summary buffer = {};
    Editing_File *file;
    
    file = working_set_get_active_file(working_set, index);
    if (file){
        fill_buffer_summary(&buffer, file, working_set);
    }
    
    return(buffer);
}

GET_PARAMETER_BUFFER_SIG(external_get_parameter_buffer){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Models *models = cmd->models;
    Buffer_Summary buffer = {};
    
    if (param_index >= 0 && param_index < models->buffer_param_count){
        buffer = external_get_buffer(app, models->buffer_param_indices[param_index]);
    }
    
    return(buffer);
}

GET_BUFFER_BY_NAME_SIG(external_get_buffer_by_name){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Buffer_Summary buffer = {};
    Editing_File *file;
    Working_Set *working_set = &cmd->models->working_set;
    
    file = working_set_contains(cmd->system, working_set, make_string(filename, len));
    if (file && !file->is_dummy){
        fill_buffer_summary(&buffer, file, working_set);
    }
    
    return(buffer);
}

REFRESH_BUFFER_SIG(external_refresh_buffer){
    int result;
    *buffer = external_get_buffer(app, buffer->buffer_id);
    result = buffer->exists;
    return(result);
}

BUFFER_SEEK_SIG(external_buffer_seek){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file;
    Working_Set *working_set;
    int result = 0;
    
    if (buffer->exists){
        working_set = &cmd->models->working_set;
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file && file_is_ready(file)){
            // TODO(allen): reduce duplication?
            {
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
            }
            
            fill_buffer_summary(buffer, file, working_set);
        }
    }
    
    return(result);
}

BUFFER_READ_RANGE_SIG(external_buffer_read_range){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file;
    Working_Set *working_set;
    int result = 0;
    int size;
    
    if (buffer->exists){
        working_set = &cmd->models->working_set;
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file && file_is_ready(file)){
            size = buffer_size(&file->state.buffer);
            if (0 <= start && start <= end && end <= size){
                result = 1;
                buffer_stringify(&file->state.buffer, start, end, out);
            }
            fill_buffer_summary(buffer, file, working_set);
        }
    }
    
    return(result);
}

BUFFER_REPLACE_RANGE_SIG(external_buffer_replace_range){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file;
    Working_Set *working_set;
    
    Models *models;
    
    int result = 0;
    int size;
    int next_cursor, pos;
    
    if (buffer->exists){
        models = cmd->models;
        working_set = &models->working_set;
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file && file_is_ready(file)){
            size = buffer_size(&file->state.buffer);
            if (0 <= start && start <= end && end <= size){
                result = 1;
                
                pos = file->state.cursor_pos;
                if (pos < start) next_cursor = pos;
                else if (pos < end) next_cursor = start;
                else next_cursor = pos + end - start - len;
                
                file_replace_range(cmd->system, models, file, start, end, str, len, next_cursor);
            }
            fill_buffer_summary(buffer, file, working_set);
        }
    }
    
    return(result);
}

#if 0
BUFFER_SET_POS_SIG(external_buffer_set_pos){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_File *file;
    Working_Set *working_set;
    
    int result = 0;
    int size;
    
    if (buffer->exists){
        working_set = &cmd->models->working_set;
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file && file_is_ready(file)){
            result = 1;
            size = buffer_size(&file->state.buffer);
            if (pos < 0) pos = 0;
            if (pos > size) pos = size;
            file->state.cursor_pos = pos;
            fill_buffer_summary(buffer, file, working_set);
        }
    }
    
    return(result);
}
#endif

BUFFER_SET_SETTING_SIG(external_buffer_set_setting){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    
    Editing_File *file;
    Working_Set *working_set;
    
    int result = false;
    
    i32 new_mapid = 0;
    
    if (buffer->exists){
        working_set = &models->working_set;
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file && file_is_ready(file)){
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
            }
        }
        
        fill_buffer_summary(buffer, file, working_set);
    }
    
    return(result);
}

BUFFER_SAVE_SIG(external_buffer_save){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    
    Editing_File *file;
    Working_Set *working_set;
    
    int result = false;
    
    if (buffer->exists){
        working_set = &models->working_set;
        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file && !file->is_dummy && file_is_ready(file)){
            result = true;
            String name = make_string(filename, filename_len);
            view_save_file(system, models, file, 0, name, 0);
        }
    }
    
    return(result);
}

GET_VIEW_FIRST_SIG(external_get_view_first){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_Layout *layout = &cmd->models->layout;
    View_Summary view = {};
    
    Panel *panel = layout->used_sentinel.next;
    
    Assert(panel != &layout->used_sentinel);
    fill_view_summary(&view, panel->view, &cmd->vars->live_set, &cmd->models->working_set);
    
    return(view);
}

GET_VIEW_NEXT_SIG(external_get_view_next){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Editing_Layout *layout = &cmd->models->layout;
    Live_Views *live_set = &cmd->vars->live_set;
    View *vptr;
    Panel *panel;
    int index = view->view_id - 1;
    
    if (index >= 0 && index < live_set->max){
        vptr = live_set->views + index;
        panel = vptr->panel;
        if (panel) panel = panel->next;
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

GET_VIEW_SIG(external_get_view){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View_Summary view = {};
    Live_Views *live_set = cmd->live_set;
    int max = live_set->max;
    View *vptr;
    
    index -= 1;
    if (index >= 0 && index < max){
        vptr = live_set->views + index;
        fill_view_summary(&view, vptr, live_set, &cmd->models->working_set);
    }
    
    return(view);
}

GET_ACTIVE_VIEW_SIG(external_get_active_view){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View_Summary view = {};
    fill_view_summary(&view, cmd->view, &cmd->vars->live_set, &cmd->models->working_set);
    return(view);
}

REFRESH_VIEW_SIG(external_refresh_view){
    int result;
    *view = external_get_view(app, view->view_id);
    result = view->exists;
    return(result);
}

VIEW_AUTO_TAB_SIG(external_view_auto_tab){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    
    int result = false;
    
    Live_Views *live_set;
    Editing_File *file;
    View *vptr;
    int view_id;
    
    if (view->exists){
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            file = vptr->file_data.file;
            
            if (file && file->state.token_stack.tokens &&
                file->state.tokens_complete && !file->state.still_lexing){
                result = true;
                
                Indent_Options opts;
                opts.empty_blank_lines = (flags & AutoTab_ClearLine);
                opts.use_tabs = (flags & AutoTab_UseTab);
                opts.tab_width = tab_width;
                
                view_auto_tab_tokens(system, models, vptr, start, end, opts);
            }
        }
    }
    
    return(result);
}

VIEW_COMPUTE_CURSOR_SIG(external_view_compute_cursor){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Live_Views *live_set;
    View *vptr;
    Editing_File *file;
    Full_Cursor result = {0};
    int view_id;
    
    if (view->exists){
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            file = vptr->file_data.file;
            if (file && !file->is_loading){
                if (seek.type == buffer_seek_line_char && seek.character <= 0){
                    seek.character = 1;
                }
                result = view_compute_cursor(vptr, seek);
                fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
            }
        }
    }
    
    return(result);
}

VIEW_SET_CURSOR_SIG(external_view_set_cursor){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Live_Views *live_set;
    View *vptr;
    Editing_File *file;
    int result = 0;
    int view_id;
    
    if (view->exists){
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            file = vptr->file_data.file;
            if (file && !file->is_loading){
                result = 1;
                if (seek.type == buffer_seek_line_char && seek.character <= 0){
                    seek.character = 1;
                }
                vptr->recent->cursor = view_compute_cursor(vptr, seek);
                if (set_preferred_x){
                    vptr->recent->preferred_x = view_get_cursor_x(vptr);
                }
                fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
                file->state.cursor_pos = vptr->recent->cursor.pos;
            }
        }
    }
    
    return(result);
}

VIEW_SET_MARK_SIG(external_view_set_mark){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Live_Views *live_set;
    View *vptr;
    Full_Cursor cursor;
    int result = 0;
    int view_id;
    
    if (view->exists){
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            result = 1;
            if (seek.type != buffer_seek_pos){
                cursor = view_compute_cursor(vptr, seek);
                vptr->recent->mark = cursor.pos;
            }
            else{
                vptr->recent->mark = seek.pos;
            }
            fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
        }
    }
    
    return(result);
}

VIEW_SET_HIGHLIGHT_SIG(external_view_set_highlight){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Live_Views *live_set;
    View *vptr;
    int result = 0;
    int view_id;
    
    if (view->exists){
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            result = 1;
            if (turn_on){
                view_set_temp_highlight(vptr, start, end);
            }
            else{
                vptr->file_data.show_temp_highlight = 0;
            }
            fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
        }
    }
    
    return(result);
}

VIEW_SET_BUFFER_SIG(external_view_set_buffer){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Live_Views *live_set;
    View *vptr;
    Editing_File *file;
    Working_Set *working_set;
    Models *models;
    int result = 0;
    int view_id;
    
    if (view->exists){
        models = cmd->models;
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            working_set = &models->working_set;
            file = working_set_get_active_file(working_set, buffer_id);
            
            if (file){
                result = 1;
                if (file != vptr->file_data.file){
                    view_set_file(vptr, file, models);
                    view_show_file(vptr);
                }
            }
            
            fill_view_summary(view, vptr, live_set, working_set);
        }
    }
    
    return(result);
}

VIEW_POST_FADE_SIG(external_view_post_fade){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    
    Live_Views *live_set;
    View *vptr;
    Models *models;
    int view_id;
    
    int result = false;
    
    if (view->exists){
        models = cmd->models;
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            
            if (end > start){
                int size = end - start;
                result = true;
                view_post_paste_effect(vptr, ticks, start, size, color);
            }
        }
    }
    
    return(result);
}

// TODO(allen): standardize the safe get view/buffer code
VIEW_SET_PASTE_REWRITE__SIG(external_view_set_paste_rewrite_){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    
    Live_Views *live_set;
    View *vptr;
    Models *models;
    int view_id;
    
    if (view->exists){
        models = cmd->models;
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            vptr->next_mode.rewrite = true;
        }
    }
}

VIEW_GET_PASTE_REWRITE__SIG(external_view_get_paste_rewrite_){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    
    Live_Views *live_set;
    View *vptr;
    Models *models;
    int view_id;
    
    int result = false;
    
    if (view->exists){
        models = cmd->models;
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            result = vptr->mode.rewrite;
        }
    }
    
    return(result);
}

VIEW_OPEN_FILE_SIG(external_view_open_file){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    
    System_Functions *system = cmd->system;
    Models *models = cmd->models;
    
    Working_Set *working_set = &models->working_set;
    
    Live_Views *live_set = cmd->live_set;
    
    int result = false;
    
    // TODO(allen): do in background option
    
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    String string = make_string_terminated(part, filename, filename_len);
    
    if (do_in_background){
        result = true;
        view_open_file(system, models, 0, string);
    }
    else if (view){
        View *vptr = 0;
        int view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            result = true;
            
            view_open_file(system, models, 0, string);
            
            fill_view_summary(view, vptr, live_set, working_set);
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

VIEW_KILL_BUFFER_SIG(external_view_kill_buffer){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    System_Functions *system = cmd->system;
    Live_Views *live_set;
    View *vptr;
    Editing_File *file;
    Working_Set *working_set;
    Models *models;
    int result = false;
    int view_id;
    
    if (view->exists){
        models = cmd->models;
        live_set = cmd->live_set;
        view_id = view->view_id - 1;
        if (view_id >= 0 && view_id < live_set->max){
            vptr = live_set->views + view_id;
            working_set = &models->working_set;
            file = get_file_from_identifier(system, working_set, buffer);
            
            if (file){
                result = true;
                try_kill_file(system, models, file, vptr, string_zero());
                fill_view_summary(view, vptr, live_set, working_set);
            }
        }
    }
    
    return(result);
}

GET_USER_INPUT_SIG(external_get_user_input){
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

GET_COMMAND_INPUT_SIG(external_get_command_input){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    User_Input result;
    
    result.type = UserInputKey;
    result.abort = 0;
    result.key = cmd->key;
    result.command = 0;
    
    return(result);
}

GET_MOUSE_STATE_SIG(external_get_mouse_state){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    App_Vars *vars = cmd->vars;
    Mouse_State mouse = direct_get_mouse_state(&vars->available_input);
    return(mouse);
}

GET_EVENT_MESSAGE_SIG(external_get_event_message){
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

START_QUERY_BAR_SIG(external_start_query_bar){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Query_Slot *slot = 0;
    View *vptr;
    
    vptr = cmd->view;
    
    slot = alloc_query_slot(&vptr->query_set);
    slot->query_bar = bar;
    
    return(slot != 0);
}

END_QUERY_BAR_SIG(external_end_query_bar){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    View *vptr;
    vptr = cmd->view;
    free_query_slot(&vptr->query_set, bar);
}

PRINT_MESSAGE_SIG(external_print_message){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Models *models = cmd->models;
    do_feedback_message(cmd->system, models, make_string(string, len));
}

#if 0
GET_GUI_FUNCTIONS_SIG(external_get_gui_functions){
    GUI_Functions *guifn = 0;
    NotImplemented;
    return(guifn);
}

GET_GUI_SIG(external_get_gui){
    GUI *gui = 0;
    NotImplemented;
    return(gui);
}
#endif

CHANGE_THEME_SIG(external_change_theme){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Style_Library *styles = &cmd->models->styles;
    String theme_name = make_string(name, len);
    Style *s;
    i32 i, count;
    
    count = styles->count;
    s = styles->styles;
    for (i = 0; i < count; ++i, ++s){
        if (match(s->name, theme_name)){
            style_copy(main_style(cmd->models), s);
            break;
        }
    }
}

CHANGE_FONT_SIG(external_change_font){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Font_Set *set = cmd->models->font_set;
    Style_Font *global_font = &cmd->models->global_font;
    String font_name = make_string(name, len);
    i16 font_id;
    
    if (font_set_extract(set, font_name, &font_id)){
        global_font->font_id = font_id;
        global_font->font_changed = 1;
    }
}

SET_THEME_COLORS_SIG(external_set_theme_colors){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Style *style = main_style(cmd->models);
    Theme_Color *theme_color;
    u32 *color;
    i32 i;
    
    theme_color = colors;
    for (i = 0; i < count; ++i, ++theme_color){
        color = style_index_by_tag(&style->main, theme_color->tag);
        if (color) *color = theme_color->color | 0xFF000000;
    }
}

GET_THEME_COLORS_SIG(external_get_theme_colors){
    Command_Data *cmd = (Command_Data*)app->cmd_context;
    Style *style = main_style(cmd->models);
    Theme_Color *theme_color;
    u32 *color;
    i32 i;
    
    theme_color = colors;
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

// BOTTOM

