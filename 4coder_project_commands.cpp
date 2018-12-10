/*
4coder_project_commands.cpp - commands for loading and using a project.
*/

// TOP

static Project current_project = {};
static Partition current_project_arena = {};

///////////////////////////////

static Project_File_Pattern_Array
get_pattern_array_from_cstring_array(Partition *arena, CString_Array list){
    Project_File_Pattern_Array array = {};
    int32_t count = list.count;
    array.patterns = push_array(arena, Project_File_Pattern, count);
    array.count = count;
    for (int32_t i = 0; i < count; ++i){
        String base_str = make_string_slowly(list.strings[i]);
        String str = string_push(arena, base_str.size + 3);
        append(&str, "*.");
        append(&str, base_str);
        terminate_with_null(&str);
        get_absolutes(str, &array.patterns[i].absolutes, false, false);
    }
    return(array);
}

static Project_File_Pattern_Array
get_pattern_array_from_extension_list(Partition *arena, Extension_List extension_list){
    CString_Array list = {};
    list.strings = extension_list.exts;
    list.count = extension_list.count;
    return(get_pattern_array_from_cstring_array(arena, list));
}

///////////////////////////////

static void
close_all_files_with_extension(Application_Links *app, Partition *scratch_part,
                               CString_Array extension_array){
    Temp_Memory temp = begin_temp_memory(scratch_part);
    
    int32_t buffers_to_close_max = partition_remaining(scratch_part)/sizeof(int32_t);
    int32_t *buffers_to_close = push_array(scratch_part, int32_t, buffers_to_close_max);
    
    int32_t buffers_to_close_count = 0;
    bool32 do_repeat = 0;
    do{
        buffers_to_close_count = 0;
        do_repeat = 0;
        
        uint32_t access = AccessAll;
        Buffer_Summary buffer = {};
        for (buffer = get_buffer_first(app, access);
             buffer.exists;
             get_buffer_next(app, &buffer, access)){
            
            bool32 is_match = 1;
            if (extension_array.count > 0){
                is_match = 0;
                if (buffer.file_name != 0){
                    String extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
                    for (int32_t i = 0; i < extension_array.count; ++i){
                        if (match(extension, extension_array.strings[i])){
                            is_match = 1;
                            break;
                        }
                    }
                }
            }
            
            if (is_match){
                if (buffers_to_close_count >= buffers_to_close_max){
                    do_repeat = 1;
                    break;
                }
                buffers_to_close[buffers_to_close_count++] = buffer.buffer_id;
            }
        }
        
        for (int32_t i = 0; i < buffers_to_close_count; ++i){
            kill_buffer(app, buffer_identifier(buffers_to_close[i]), true, 0);
        }
    }while(do_repeat);
    
    end_temp_memory(temp);
}

static bool32
match_in_pattern_array(char *str, Project_File_Pattern_Array array){
    bool32 found_match = false;
    Project_File_Pattern *pattern = array.patterns;
    for (int32_t i = 0; i < array.count; ++i, ++pattern){
        if (wildcard_match_c(&pattern->absolutes, str, true)){
            found_match = true;
            break;
        }
    }
    return(found_match);
}

static void
open_all_files_in_directory_pattern_match__inner(Application_Links *app, String space,
                                                 Project_File_Pattern_Array whitelist,
                                                 Project_File_Pattern_Array blacklist,
                                                 uint32_t flags){
    File_List list = get_file_list(app, space.str, space.size);
    int32_t dir_size = space.size;
    
    File_Info *info = list.infos;
    for (uint32_t i = 0; i < list.count; ++i, ++info){
        if (info->folder){
            if ((flags & OpenAllFilesFlag_Recursive) == 0){
                continue;
            }
            if (match_in_pattern_array(info->filename, blacklist)){
                continue;
            }
            
            space.size = dir_size;
            String str = make_string(info->filename, info->filename_len);
            append(&space, str);
            append(&space, "/");
            open_all_files_in_directory_pattern_match__inner(app, space, whitelist, blacklist, flags);
        }
        else{
            if (!match_in_pattern_array(info->filename, whitelist)){
                continue;
            }
            if (match_in_pattern_array(info->filename, blacklist)){
                continue;
            }
            
            space.size = dir_size;
            String str = make_string(info->filename, info->filename_len);
            append(&space, str);
            create_buffer(app, space.str, space.size, 0);
        }
    }
    
    free_file_list(app, list);
}

static Project_File_Pattern_Array
get_standard_blacklist(Partition *arena){
    static char *dot_str = ".*";
    CString_Array black_array = {};
    black_array.strings = &dot_str;
    black_array.count = 1;
    return(get_pattern_array_from_cstring_array(arena, black_array));
}

static void
open_all_files_in_directory_pattern_match(Application_Links *app, Partition *scratch,
                                          String dir,
                                          Project_File_Pattern_Array whitelist,
                                          Project_File_Pattern_Array blacklist,
                                          uint32_t flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    int32_t size = 32 << 10;
    char *mem = push_array(scratch, char, size);
    String space = make_string_cap(mem, 0, size);
    append(&space, dir);
    if (space.size == 0 || !char_is_slash(space.str[space.size - 1])){
        append(&space, '/');
    }
    open_all_files_in_directory_pattern_match__inner(app, space, whitelist, blacklist, flags);
    end_temp_memory(temp);
}

static void
open_all_files_in_directory_with_extension(Application_Links *app, Partition *scratch,
                                           String dir, CString_Array array,
                                           uint32_t flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    Project_File_Pattern_Array whitelist = get_pattern_array_from_cstring_array(scratch, array);
    Project_File_Pattern_Array blacklist = get_standard_blacklist(scratch);
    open_all_files_in_directory_pattern_match(app, scratch, dir, whitelist, blacklist, flags);
    end_temp_memory(temp);
}

static void
open_all_files_in_hot_with_extension(Application_Links *app, Partition *scratch,
                                     CString_Array array,
                                     uint32_t flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    int32_t size = 32 << 10;
    char *mem = push_array(scratch, char, size);
    String space = make_string_cap(mem, 0, size);
    space.size = directory_get_hot(app, space.str, space.memory_size);
    if (space.size == 0 || !char_is_slash(space.str[space.size - 1])){
        append(&space, '/');
    }
    Project_File_Pattern_Array whitelist = get_pattern_array_from_cstring_array(scratch, array);
    Project_File_Pattern_Array blacklist = get_standard_blacklist(scratch);
    open_all_files_in_directory_pattern_match__inner(app, space, whitelist, blacklist, flags);
    end_temp_memory(temp);
}

///////////////////////////////

#if defined(IS_WINDOWS)
#define PlatformName "win"
#elif defined(IS_LINUX)
#define PlatformName "linux"
#elif defined(IS_MAC)
#define PlatformName "mac"
#else
# error no project configuration names for this platform
#endif

static Project*
parse_project__config_data__version_0(Partition *arena, String file_dir, Config *parsed){
    Project *project = push_array(arena, Project, 1);
    memset(project, 0, sizeof(*project));
    
    // Set new project directory
    {
        int32_t cap = file_dir.size + 1;
        char *mem = push_array(arena, char, cap);
        project->dir = make_string(mem, 0, cap);
        copy(&project->dir, file_dir);
        terminate_with_null(&project->dir);
        
        project->load_path_array.paths = push_array(arena, Project_File_Load_Path, 1);
        project->load_path_array.count = 1;
        
        project->load_path_array.paths[0].path = project->dir;
        project->load_path_array.paths[0].recursive = false;
        project->load_path_array.paths[0].relative = false;
        
        project->name = project->dir;
    }
    
    // Read the settings from project.4coder
    String str = {};
    if (config_string_var(parsed, "extensions", 0, &str)){
        Extension_List extension_list;
        parse_extension_line_to_extension_list(str, &extension_list);
        project->pattern_array = get_pattern_array_from_extension_list(arena, extension_list);
        project->blacklist_pattern_array = get_standard_blacklist(arena);
    }
    
    bool32 open_recursively = false;
    if (config_bool_var(parsed, "open_recursively", 0, &open_recursively)){
        project->load_path_array.paths[0].recursive = open_recursively;
    }
    
    char fkey_command_name[] = "fkey_command_" PlatformName;
    
    project->command_array.commands = push_array(arena, Project_Command, 16);
    project->command_array.count = 16;
    
    Project_Command *command = project->command_array.commands;
    for (int32_t j = 1; j <= 16; ++j, ++command){
        project->fkey_commands[j - 1] = j - 1;
        memset(command, 0, sizeof(*command));
        
        Config_Compound *compound = 0;
        if (config_compound_var(parsed, fkey_command_name, j, &compound)){
            command->name = string_push(arena, 4);
            append_int_to_str(&command->name, j);
            terminate_with_null(&command->name);
            
            String cmd = {};
            if (config_compound_string_member(parsed, compound, "cmd", 0, &cmd)){
                command->cmd = string_push_copy(arena, cmd);
            }
            
            String out = {};
            if (config_compound_string_member(parsed, compound, "out", 1, &out)){
                command->out = string_push_copy(arena, out);
            }
            
            bool32 footer_panel = false;
            if (config_compound_bool_member(parsed, compound, "footer_panel", 2, &footer_panel)){
                command->footer_panel = footer_panel;
            }
            
            bool32 save_dirty_files = false;
            if (config_compound_bool_member(parsed, compound, "save_dirty_files", 3, &save_dirty_files)){
                command->save_dirty_files = save_dirty_files;
            }
        }
        else{
            command->name = string_push(arena, 4);
            append_int_to_str(&command->name, j);
            terminate_with_null(&command->name);
        }
    }
    
    project->loaded = true;
    return(project);
}

static void
parse_project__extract_pattern_array(Partition *arena, Config *parsed,
                                     char *root_variable_name,
                                     Project_File_Pattern_Array *array_out){
    Config_Compound *compound = 0;
    if (config_compound_var(parsed, root_variable_name, 0, &compound)){
        Config_Get_Result_List list = typed_string_array_reference_list(arena, parsed, compound);
        
        array_out->patterns = push_array(arena, Project_File_Pattern, list.count);
        array_out->count = list.count;
        
        int32_t i = 0;
        for (Config_Get_Result_Node *node = list.first;
             node != 0;
             node = node->next, i += 1){
            String str = string_push_copy(arena, node->result.string);
            get_absolutes(str, &array_out->patterns[i].absolutes, false, false);
        }
    }
}

static Project_OS_Match_Level
parse_project__version_1__os_match(String str, String this_os_str){
    if (match(str, this_os_str)){
        return(ProjectOSMatchLevel_ActiveMatch);
    }
    else if (match(str, make_lit_string("all"))){
        return(ProjectOSMatchLevel_ActiveMatch);
    }
    else if (match(str, make_lit_string("default"))){
        return(ProjectOSMatchLevel_PassiveMatch);
    }
    return(ProjectOSMatchLevel_NoMatch);
}

static Project*
parse_project__config_data__version_1(Partition *arena, String root_dir, Config *parsed){
    Project *project = push_array(arena, Project, 1);
    memset(project, 0, sizeof(*project));
    
    // Set new project directory
    {
        int32_t cap = root_dir.size + 1;
        project->dir = string_push(arena, cap);
        copy(&project->dir, root_dir);
        terminate_with_null(&project->dir);
    }
    
    // project_name
    {
        String str = {};
        if (config_string_var(parsed, "project_name", 0, &str)){
            project->name = string_push_copy(arena, str);
        }
        else{
            project->name = make_lit_string("");
        }
    }
    
    // patterns
    parse_project__extract_pattern_array(arena, parsed, "patterns", &project->pattern_array);
    
    // blacklist_patterns
    parse_project__extract_pattern_array(arena, parsed, "blacklist_patterns", &project->blacklist_pattern_array);
    
    // load_paths
    {
        Config_Compound *compound = 0;
        if (config_compound_var(parsed, "load_paths", 0, &compound)){
            bool32 found_match = false;
            Config_Compound *best_paths = 0;
            
            for (int32_t i = 0;; ++i){
                Config_Iteration_Step_Result result = typed_array_iteration_step(parsed, compound, ConfigRValueType_Compound, i);
                if (result.step == Iteration_Skip){
                    continue;
                }
                else if (result.step == Iteration_Quit){
                    break;
                }
                Config_Compound *paths_option = result.get.compound;
                
                Config_Compound *paths = 0;
                if (config_compound_compound_member(parsed, paths_option, "paths", 0, &paths)){
                    String str = {};
                    if (config_compound_string_member(parsed, paths_option, "os", 1, &str)){
                        Project_OS_Match_Level r = parse_project__version_1__os_match(str, make_lit_string(PlatformName));
                        if (r == ProjectOSMatchLevel_ActiveMatch){
                            found_match = true;
                            best_paths = paths;
                            break;
                        }
                        else if (r == ProjectOSMatchLevel_PassiveMatch){
                            if (!found_match){
                                found_match = true;
                                best_paths = paths;
                            }
                        }
                    }
                }
            }
            
            if (found_match){
                Config_Get_Result_List list = typed_compound_array_reference_list(arena, parsed, best_paths);
                
                project->load_path_array.paths = push_array(arena, Project_File_Load_Path, list.count);
                project->load_path_array.count = list.count;
                
                Project_File_Load_Path *dst = project->load_path_array.paths;
                for (Config_Get_Result_Node *node = list.first;
                     node != 0;
                     node = node->next, ++dst){
                    Config_Compound *src = node->result.compound;
                    memset(dst, 0, sizeof(*dst));
                    dst->recursive = true;
                    dst->relative = true;
                    
                    String str = {};
                    if (config_compound_string_member(parsed, src, "path", 0, &str)){
                        dst->path = string_push_copy(arena, str);
                    }
                    
                    config_compound_bool_member(parsed, src, "recursive", 1, &dst->recursive);
                    config_compound_bool_member(parsed, src, "relative", 2, &dst->relative);
                }
            }
        }
    }
    
    // command_list
    {
        Config_Compound *compound = 0;
        if (config_compound_var(parsed, "command_list", 0, &compound)){
            Config_Get_Result_List list = typed_compound_array_reference_list(arena, parsed, compound);
            
            project->command_array.commands = push_array(arena, Project_Command, list.count);
            project->command_array.count = list.count;
            
            Project_Command *dst = project->command_array.commands;
            for (Config_Get_Result_Node *node = list.first;
                 node != 0;
                 node = node->next, ++dst){
                char *pos = node->result.pos;
                Config_Compound *src = node->result.compound;
                memset(dst, 0, sizeof(*dst));
                
                bool32 can_emit_command = true;
                
                String name = {};
                Config_Get_Result cmd_result = {};
                Config_Compound *cmd_set = 0;
                char *cmd_pos = 0;
                String cmd_str = {};
                String out = {};
                bool32 footer_panel = false;
                bool32 save_dirty_files = true;
                bool32 cursor_at_end = false;
                
                if (!config_compound_string_member(parsed, src, "name", 0, &name)){
                    can_emit_command = false;
                    config_add_error(arena, parsed, pos, "a command must have a string type name member");
                    goto finish_command;
                }
                
                cmd_result = config_compound_member(parsed, src,
                                                    make_lit_string("cmd"), 1);
                if (cmd_result.success){
                    cmd_set = cmd_result.compound;
                    cmd_pos = cmd_result.pos;
                }
                else{
                    can_emit_command = false;
                    config_add_error(arena, parsed, pos, "a command must have an array type cmd member");
                    goto finish_command;
                }
                
                can_emit_command = false;
                for (int32_t j = 0;; ++j){
                    Config_Iteration_Step_Result result = typed_array_iteration_step(parsed, cmd_set, ConfigRValueType_Compound, j);
                    if (result.step == Iteration_Skip){
                        continue;
                    }
                    else if (result.step == Iteration_Quit){
                        break;
                    }
                    Config_Compound *cmd_option = result.get.compound;
                    
                    String cmd = {};
                    if (config_compound_string_member(parsed, cmd_option, "cmd", 0, &cmd)){
                        String str = {};
                        if (config_compound_string_member(parsed, cmd_option, "os", 1, &str)){
                            Project_OS_Match_Level r = parse_project__version_1__os_match(str, make_lit_string(PlatformName));
                            if (r == ProjectOSMatchLevel_ActiveMatch){
                                can_emit_command = true;
                                cmd_str = cmd;
                                break;
                            }
                            else if (r == ProjectOSMatchLevel_PassiveMatch){
                                if (!can_emit_command){
                                    can_emit_command = true;
                                    cmd_str = cmd;
                                }
                            }
                        }
                    }
                }
                
                if (!can_emit_command){
                    config_add_error(arena, parsed, cmd_pos, "no usable command strings found in cmd");
                    goto finish_command;
                }
                
                config_compound_string_member(parsed, src, "out", 2, &out);
                config_compound_bool_member(parsed, src, "footer_panel", 3, &footer_panel);
                config_compound_bool_member(parsed, src, "save_dirty_files", 4,
                                            &save_dirty_files);
                config_compound_bool_member(parsed, src, "cursor_at_end", 5,
                                            &cursor_at_end);
                
                dst->name = string_push_copy(arena, name);
                dst->cmd = string_push_copy(arena, cmd_str);
                dst->out = string_push_copy(arena, out);
                dst->footer_panel = footer_panel;
                dst->save_dirty_files = save_dirty_files;
                dst->cursor_at_end = cursor_at_end;
                
                finish_command:;
            }
        }
    }
    
    // fkey_command
    {
        for (int32_t i = 1; i <= 16; ++i){
            String name = {};
            int32_t index = -1;
            if (config_string_var(parsed, "fkey_command", i, &name)){
                int32_t count = project->command_array.count;
                Project_Command *command_ptr = project->command_array.commands;
                for (int32_t j = 0; j < count; ++j, ++command_ptr){
                    if (match(command_ptr->name, name)){
                        index = j;
                        break;
                    }
                }
            }
            project->fkey_commands[i - 1] = index;
        }
    }
    
    project->loaded = true;
    return(project);
}

static Project*
parse_project__config_data(Partition *arena, String file_dir, Config *parsed){
    int32_t version = 0;
    if (parsed->version != 0){
        version = *parsed->version;
    }
    
    switch (version){
        case 0:
        {
            return(parse_project__config_data__version_0(arena, file_dir, parsed));
        }break;
        
        case 1:
        {
            return(parse_project__config_data__version_1(arena, file_dir, parsed));
        }break;
        
        default:
        {
            return(0);
        }break;
    }
}

static Project_Parse_Result
parse_project__data(Partition *arena, String file_name, String data, String file_dir){
    Project_Parse_Result result = {};
    Cpp_Token_Array array = text_data_to_token_array(arena, data);
    if (array.tokens != 0){
        result.parsed = text_data_and_token_array_to_parse_data(arena, file_name, data, array);
        if (result.parsed != 0){
            result.project = parse_project__config_data(arena, file_dir, result.parsed);
        }
    }
    return(result);
}

static Project_Parse_Result
parse_project__nearest_file(Application_Links *app, Partition *arena){
    Project_Parse_Result result = {};
    
    Temp_Memory restore_point = begin_temp_memory(arena);
    String project_path = {};
    int32_t size = 32 << 10;
    char *space = push_array(arena, char, size);
    if (space != 0){
        project_path = make_string_cap(space, 0, size);
        project_path.size = directory_get_hot(app, project_path.str, project_path.memory_size);
        end_temp_memory(restore_point);
        push_array(arena, char, project_path.size);
        project_path.memory_size = project_path.size;
        if (project_path.size == 0){
            print_message(app, literal("The hot directory is empty, cannot search for a project.\n"));
        }
        else{
            File_Name_Path_Data dump = dump_file_search_up_path(arena, project_path, make_lit_string("project.4coder"));
            if (dump.data.str != 0){
                String project_root = dump.path;
                remove_last_folder(&project_root);
                result = parse_project__data(arena, dump.file_name, dump.data, project_root);
            }
            else{
                char message_space[512];
                String message = make_fixed_width_string(message_space);
                append(&message, "Did not find project.4coder.  ");
                if (current_project.loaded){
                    if (current_project.name.size > 0){
                        append(&message, "Continuing with: ");
                        append(&message, current_project.name);
                    }
                    else{
                        append(&message, "Continuing with: ");
                        append(&message, current_project.dir);
                    }
                }
                else{
                    append(&message, "Continuing without a project");
                }
                append(&message, '\n');
                print_message(app, message.str, message.size);
            }
        }
    }
    
    return(result);
}

static bool32
project_deep_copy__pattern_array(Partition *arena, Project_File_Pattern_Array *src_array,
                                 Project_File_Pattern_Array *dst_array){
    int32_t pattern_count = src_array->count;
    dst_array->patterns = push_array(arena, Project_File_Pattern, pattern_count);
    if (dst_array->patterns == 0){
        return(false);
    }
    dst_array->count = pattern_count;
    
    Project_File_Pattern *dst = dst_array->patterns;
    Project_File_Pattern *src = src_array->patterns;
    for (int32_t i = 0; i < pattern_count; ++i, ++dst, ++src){
        int32_t abs_count = src->absolutes.count;
        dst->absolutes.count = abs_count;
        for (int32_t j = 0; j < abs_count; ++j){
            dst->absolutes.a[j] = string_push_copy(arena, src->absolutes.a[j]);
            if (dst->absolutes.a[j].str == 0){
                return(false);
            }
        }
    }
    
    return(true);
}

static Project
project_deep_copy__inner(Partition *arena, Project *project){
    Project result = {};
    
    result.dir = string_push_copy(arena, project->dir);
    if (result.dir.str == 0){
        return(result);
    }
    
    result.name = string_push_copy(arena, project->name);
    if (result.name.str == 0){
        return(result);
    }
    
    if (!project_deep_copy__pattern_array(arena, &project->pattern_array, &result.pattern_array)){
        return(result);
    }
    if (!project_deep_copy__pattern_array(arena, &project->blacklist_pattern_array, &result.blacklist_pattern_array)){
        return(result);
    }
    
    {
        int32_t path_count = project->load_path_array.count;
        result.load_path_array.paths = push_array(arena, Project_File_Load_Path, path_count);
        if (result.load_path_array.paths == 0){
            return(result);
        }
        result.load_path_array.count = path_count;
        
        Project_File_Load_Path *dst = result.load_path_array.paths;
        Project_File_Load_Path *src = project->load_path_array.paths;
        for (int32_t i = 0; i < path_count; ++i, ++dst, ++src){
            dst->path = string_push_copy(arena, src->path);
            if (dst->path.str == 0){
                return(result);
            }
            dst->recursive = src->recursive;
            dst->relative = src->relative;
        }
    }
    
    {
        int32_t command_count = project->command_array.count;
        result.command_array.commands = push_array(arena, Project_Command, command_count);
        if (result.command_array.commands == 0){
            return(result);
        }
        result.command_array.count = command_count;
        
        Project_Command *dst = result.command_array.commands;
        Project_Command *src = project->command_array.commands;
        for (int32_t i = 0; i < command_count; ++i, ++dst, ++src){
            memset(dst, 0, sizeof(*dst));
            if (src->name.str != 0){
                dst->name = string_push_copy(arena, src->name);
                if (dst->name.str == 0){
                    return(result);
                }
            }
            if (src->cmd.str != 0){
                dst->cmd = string_push_copy(arena, src->cmd);
                if (dst->cmd.str == 0){
                    return(result);
                }
            }
            if (src->out.str != 0){
                dst->out = string_push_copy(arena, src->out);
                if (dst->out.str == 0){
                    return(result);
                }
            }
            dst->footer_panel = src->footer_panel;
            dst->save_dirty_files = src->save_dirty_files;
            dst->cursor_at_end = src->cursor_at_end;
        }
    }
    
    {
        memcpy(result.fkey_commands, project->fkey_commands, sizeof(result.fkey_commands));
    }
    
    result.loaded = true;
    return(result);
}

static Project
project_deep_copy(Partition *arena, Project *project){
    Temp_Memory temp = begin_temp_memory(arena);
    Project result = project_deep_copy__inner(arena, project);
    if (!result.loaded){
        end_temp_memory(temp);
        memset(&result, 0, sizeof(result));
    }
    return(result);
}

static void
config_feedback_file_pattern_array(String *space, char *name, Project_File_Pattern_Array *array){
    append(space, name);
    append(space, " = {\n");
    Project_File_Pattern *pattern = array->patterns;
    for (int32_t i = 0; i < array->count; ++i, ++pattern){
        append(space, "\"");
        String *absolute = pattern->absolutes.a;
        for (int32_t j = 0; j < pattern->absolutes.count; ++j, ++absolute){
            if (j != 0){
                append(space, "*");
            }
            append(space, *absolute);
        }
        append(space, "\",\n");
    }
    append(space, "};\n");
}

static void
config_feedback_file_load_path_array(String *space, char *name, Project_File_Load_Path_Array *array){
    append(space, name);
    append(space, " = {\n");
    Project_File_Load_Path *path = array->paths;
    for (int32_t i = 0; i < array->count; ++i, ++path){
        append(space, "{ ");
        append(space, ".path = \"");
        append(space, path->path);
        append(space, "\", ");
        append(space, ".recursive = ");
        append(space, (char*)(path->recursive?"true":"false"));
        append(space, ", ");
        append(space, ".relative = ");
        append(space, (char*)(path->relative?"true":"false"));
        append(space, ", ");
        append(space, "},\n");
    }
    append(space, "};\n");
}

static void
config_feedback_command_array(String *space, char *name, Project_Command_Array *array){
    append(space, name);
    append(space, " = {\n");
    Project_Command *command = array->commands;
    for (int32_t i = 0; i < array->count; ++i, ++command){
        append(space, "{ ");
        append(space, ".name = \"");
        append(space, command->name);
        append(space, "\", ");
        append(space, ".cmd = \"");
        append(space, command->cmd);
        append(space, "\", ");
        append(space, ".out = \"");
        append(space, command->out);
        append(space, "\", ");
        append(space, ".footer_panel = ");
        append(space, (char*)(command->footer_panel?"true":"false"));
        append(space, ", ");
        append(space, ".save_dirty_files = ");
        append(space, (char*)(command->save_dirty_files?"true":"false"));
        append(space, ", ");
        append(space, ".cursor_at_end = ");
        append(space, (char*)(command->cursor_at_end?"true":"false"));
        append(space, ", ");
        append(space, "},\n");
    }
    append(space, "};\n");
}

static void
set_current_project(Application_Links *app, Partition *scratch, Project *project, Config *parsed){
    bool32 print_feedback = false;
    
    if (parsed != 0 && project != 0){
        if (current_project_arena.base == 0){
            int32_t project_mem_size = (1 << 20);
            void *project_mem = memory_allocate(app, project_mem_size);
            current_project_arena = make_part(project_mem, project_mem_size);
        }
        
        // Copy project to current_project
        current_project_arena.pos = 0;
        Project new_project = project_deep_copy(&current_project_arena, project);
        if (new_project.loaded){
            current_project = new_project;
            
            print_feedback = true;
            
            // Open all project files
            for (int32_t i = 0; i < current_project.load_path_array.count; ++i){
                Project_File_Load_Path *load_path = &current_project.load_path_array.paths[i];
                uint32_t flags = 0;
                if (load_path->recursive){
                    flags |= OpenAllFilesFlag_Recursive;
                }
                
                Temp_Memory temp = begin_temp_memory(scratch);
                String path_str = load_path->path;
                String file_dir = path_str;
                if (load_path->relative){
                    String project_dir = current_project.dir;
                    int32_t cap = path_str.size + project_dir.size + 2;
                    char *mem = push_array(scratch, char, cap);
                    push_align(scratch, 8);
                    if (mem != 0){
                        file_dir = make_string_cap(mem, 0, cap);
                        append(&file_dir, project_dir);
                        if (file_dir.size == 0 || file_dir.str[file_dir.size - 1] != '/'){
                            append(&file_dir, "/");
                        }
                        append(&file_dir, path_str);
                        if (file_dir.size == 0 || file_dir.str[file_dir.size - 1] != '/'){
                            append(&file_dir, "/");
                        }
                        terminate_with_null(&file_dir);
                    }
                }
                
                Project_File_Pattern_Array whitelist = current_project.pattern_array;
                Project_File_Pattern_Array blacklist = current_project.blacklist_pattern_array;
                open_all_files_in_directory_pattern_match(app, scratch, file_dir,
                                                          whitelist, blacklist,
                                                          flags);
                
                end_temp_memory(temp);
            }
            
            // Set window title
            if (project->name.size > 0){
                char space[1024];
                String builder = make_fixed_width_string(space);
                append(&builder, "4coder project: ");
                append(&builder, project->name);
                terminate_with_null(&builder);
                set_window_title(app, builder.str);
            }
        }
        else{
#define M "Failed to initialize new project; need more memory dedicated to the project system.\n"
            print_message(app, literal(M));
#undef M
        }
    }
    else if (parsed != 0){
        print_feedback = true;
    }
    
    if (print_feedback){
        Temp_Memory temp = begin_temp_memory(scratch);
        
        // Top
        print_message(app, literal("Loaded project file:\n"));
        
        // Errors
        String error_text = config_stringize_errors(scratch, parsed);
        print_message(app, error_text.str, error_text.size);
        
        // Values
        if (project == 0){
            print_message(app, literal("Could not instantiate project\n"));
        }
        else{
            Temp_Memory temp2 = begin_temp_memory(scratch);
            String space = string_push(scratch, partition_remaining(scratch));
            
            {
                config_feedback_string(&space, "'root_directory'", project->dir);
                config_feedback_string(&space, "project_name", project->name);
                
                config_feedback_file_pattern_array(&space, "patterns", &project->pattern_array);
                config_feedback_file_pattern_array(&space, "blacklist_patterns", &project->blacklist_pattern_array);
                config_feedback_file_load_path_array(&space, "load_paths", &project->load_path_array);
                config_feedback_command_array(&space, "command_list", &project->command_array);
            }
            
            append(&space, "\n");
            print_message(app, space.str, space.size);
            end_temp_memory(temp2);
        }
        
        end_temp_memory(temp);
    }
}

static void
set_current_project_from_data(Application_Links *app, Partition *scratch,
                              String file_name, String data, String file_dir){
    Temp_Memory temp = begin_temp_memory(scratch);
    Project_Parse_Result project_parse = parse_project__data(scratch, file_name, data, file_dir);
    set_current_project(app, scratch, project_parse.project, project_parse.parsed);
    end_temp_memory(temp);
}

static void
set_current_project_from_nearest_project_file(Application_Links *app, Partition *scratch){
    Temp_Memory temp = begin_temp_memory(scratch);
    Project_Parse_Result project_parse = parse_project__nearest_file(app, scratch);
    set_current_project(app, scratch, project_parse.project, project_parse.parsed);
    end_temp_memory(temp);
}

static void
exec_project_command(Application_Links *app, Project_Command *command){
    if (command->cmd.size > 0){
        bool32 footer_panel = command->footer_panel;
        bool32 save_dirty_files = command->save_dirty_files;
        bool32 cursor_at_end = command->cursor_at_end;
        
        if (save_dirty_files){
            save_all_dirty_buffers(app);
        }
        
        View_Summary view = {};
        View_Summary *view_ptr = 0;
        Buffer_Identifier buffer_id = {};
        uint32_t flags = CLI_OverlapWithConflict;
        if (cursor_at_end){
            flags |= CLI_CursorAtEnd;
        }
        
        bool32 set_fancy_font = false;
        if (command->out.size > 0){
            buffer_id = buffer_identifier(command->out.str, command->out.size);
            
            if (footer_panel){
                view = get_or_open_build_panel(app);
                if (match(command->out, "*compilation*")){
                    set_fancy_font = true;
                }
            }
            else{
                view = get_active_view(app, AccessAll);
            }
            view_ptr = &view;
            
            memset(&prev_location, 0, sizeof(prev_location));
            lock_jump_buffer(command->out.str, command->out.size);
        }
        else{
            // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
            buffer_id = buffer_identifier(literal("*dump*"));
        }
        
        String dir = current_project.dir;
        String cmd = command->cmd;
        exec_system_command(app, view_ptr, buffer_id, dir.str, dir.size, cmd.str, cmd.size, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
}

static void
exec_project_command_by_index(Application_Links *app, int32_t command_index){
    if (!current_project.loaded){
        return;
    }
    if (command_index < 0 || command_index >= current_project.command_array.count){
        return;
    }
    Project_Command *command = &current_project.command_array.commands[command_index];
    exec_project_command(app, command);
}

static void
exec_project_fkey_command(Application_Links *app, int32_t fkey_index){
    if (!current_project.loaded){
        return;
    }
    int32_t command_index = current_project.fkey_commands[fkey_index];
    if (command_index < 0 || command_index >= current_project.command_array.count){
        return;
    }
    Project_Command *command = &current_project.command_array.commands[command_index];
    exec_project_command(app, command);
}

static void
exec_project_command_by_name(Application_Links *app, String name){
    if (!current_project.loaded){
        return;
    }
    Project_Command *command = current_project.command_array.commands;
    for (int32_t i = 0; i < current_project.command_array.count; ++i, ++command){
        if (match(command->name, name)){
            exec_project_command(app, command);
            break;
        }
    }
}

static void
exec_project_command_by_name(Application_Links *app, char *name){
    exec_project_command_by_name(app, make_string_slowly(name));
}

////////////////////////////////

CUSTOM_COMMAND_SIG(close_all_code)
CUSTOM_DOC("Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.")
{
    CString_Array extensions = get_code_extensions(&global_config.code_exts);
    close_all_files_with_extension(app, &global_part, extensions);
}

CUSTOM_COMMAND_SIG(open_all_code)
CUSTOM_DOC("Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.")
{
    CString_Array extensions = get_code_extensions(&global_config.code_exts);
    open_all_files_in_hot_with_extension(app, &global_part, extensions, 0);
}

CUSTOM_COMMAND_SIG(open_all_code_recursive)
CUSTOM_DOC("Works as open_all_code but also runs in all subdirectories.")
{
    CString_Array extensions = get_code_extensions(&global_config.code_exts);
    open_all_files_in_hot_with_extension(app, &global_part, extensions, OpenAllFilesFlag_Recursive);
}

///////////////////////////////

CUSTOM_COMMAND_SIG(load_project)
CUSTOM_DOC("Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.")
{
    save_all_dirty_buffers(app);
    set_current_project_from_nearest_project_file(app, &global_part);
}

CUSTOM_COMMAND_SIG(project_fkey_command)
CUSTOM_DOC("Run an 'fkey command' configured in a project.4coder file.  Determines the index of the 'fkey command' by which function key or numeric key was pressed to trigger the command.")
{
    User_Input input = get_command_input(app);
    bool32 got_ind = false;
    int32_t ind = 0;
    if (input.key.keycode >= key_f1 && input.key.keycode <= key_f16){
        ind = (input.key.keycode - key_f1);
        got_ind = true;
    }
    else if (input.key.character_no_caps_lock >= '1' && input.key.character_no_caps_lock >= '9'){
        ind = (input.key.character_no_caps_lock - '1');
        got_ind = true;
    }
    else if (input.key.character_no_caps_lock == '0'){
        ind = 9;
        got_ind = true;
    }
    if (got_ind){
        exec_project_fkey_command(app, ind);
    }
}

CUSTOM_COMMAND_SIG(project_go_to_root_directory)
CUSTOM_DOC("Changes 4coder's hot directory to the root directory of the currently loaded project. With no loaded project nothing hapepns.")
{
    if (current_project.loaded){
        String dir = current_project.dir;
        directory_set_hot(app, dir.str, dir.size);
    }
}

///////////////////////////////

static Project_Setup_Status
project_is_setup(Application_Links *app, Partition *scratch, String script_path, String script_file){
    Project_Setup_Status result = {};
    
    Temp_Memory temp = begin_temp_memory(scratch);
    
    static int32_t needed_extra_space = 15;
    char *space = push_array(&global_part, char, script_path.size + needed_extra_space);
    String str = make_string_cap(space, 0, script_path.size + needed_extra_space);
    copy(&str, script_path);
    
    int32_t dir_len = str.size;
    append(&str, "/");
    append(&str, script_file);
    append(&str, ".bat");
    result.bat_exists = file_exists(app, str.str, str.size);
    
    str.size = dir_len;
    append(&str, "/");
    append(&str, script_file);
    append(&str, ".sh");
    result.sh_exists = file_exists(app, str.str, str.size);
    
    str.size = dir_len;
    append(&str, "/project.4coder");
    result.project_exists = file_exists(app, str.str, str.size);
    
    result.everything_exists = result.bat_exists && result.sh_exists && result.project_exists;
    
    end_temp_memory(temp);
    
    return(result);
}

static Project_Key_Strings
project_key_strings_query_user(Application_Links *app,
                               bool32 get_script_file, bool32 get_code_file,
                               char *script_file_space, int32_t script_file_cap,
                               char *code_file_space, int32_t code_file_cap,
                               char *output_dir_space, int32_t output_dir_cap,
                               char *binary_file_space, int32_t binary_file_cap){
    Project_Key_Strings keys = {};
    
    Query_Bar script_file_bar = {};
    Query_Bar code_file_bar = {};
    Query_Bar output_dir_bar = {};
    Query_Bar binary_file_bar = {};
    
    if (get_script_file){
        script_file_bar.prompt = make_lit_string("Script Name: ");
        script_file_bar.string = make_string_cap(script_file_space, 0, script_file_cap);
        
        if (!query_user_string(app, &script_file_bar)) return(keys);
        if (script_file_bar.string.size == 0) return(keys);
    }
    
    if (get_code_file){
        code_file_bar.prompt = make_lit_string("Build Target: ");
        code_file_bar.string = make_string_cap(code_file_space, 0, code_file_cap);
        
        if (!query_user_string(app, &code_file_bar)) return(keys);
        if (code_file_bar.string.size == 0) return(keys);
    }
    
    output_dir_bar.prompt = make_lit_string("Output Directory: ");
    output_dir_bar.string = make_string_cap(output_dir_space, 0, output_dir_cap);
    
    if (!query_user_string(app, &output_dir_bar)) return(keys);
    if (output_dir_bar.string.size == 0){
        copy(&output_dir_bar.string, ".");
    }
    
    
    binary_file_bar.prompt = make_lit_string("Binary Output: ");
    binary_file_bar.string = make_string_cap(binary_file_space, 0, binary_file_cap);
    
    if (!query_user_string(app, &binary_file_bar)) return(keys);
    if (binary_file_bar.string.size == 0) return(keys);
    
    
    keys.success = true;
    keys.script_file = script_file_bar.string;
    keys.code_file = code_file_bar.string;
    keys.output_dir = output_dir_bar.string;
    keys.binary_file = binary_file_bar.string;
    
    return(keys);
}

static bool32
project_generate_bat_script(Partition *scratch, String opts, String compiler,
                            String script_path, String script_file,
                            String code_file, String output_dir, String binary_file){
    bool32 success = false;
    
    Temp_Memory temp = begin_temp_memory(scratch);
    
    String cf = string_push_copy(scratch, code_file);
    String od = string_push_copy(scratch, output_dir);
    String bf = string_push_copy(scratch, binary_file);
    
    replace_char(&cf, '/', '\\');
    replace_char(&od, '/', '\\');
    replace_char(&bf, '/', '\\');
    
    int32_t space_cap = partition_remaining(scratch);
    char *space = push_array(scratch, char, space_cap);
    String file_name = make_string_cap(space, 0, space_cap);
    append(&file_name, script_path);
    append(&file_name, "/");
    append(&file_name, script_file);
    append(&file_name, ".bat");
    terminate_with_null(&file_name);
    
    FILE *bat_script = fopen(file_name.str, "wb");
    if (bat_script != 0){
        fprintf(bat_script, "@echo off\n\n");
        fprintf(bat_script, "set opts=%.*s\n", opts.size, opts.str);
        fprintf(bat_script, "set code=%%cd%%\n");
        
        fprintf(bat_script, "pushd %.*s\n", od.size, od.str);
        fprintf(bat_script, "%.*s %%opts%% %%code%%\\%.*s -Fe%.*s\n",
                compiler.size, compiler.str, cf.size, cf.str, bf.size, bf.str);
        fprintf(bat_script, "popd\n");
        
        fclose(bat_script);
        success = true;
    }
    
    end_temp_memory(temp);
    
    return(success);
}

static bool32
project_generate_sh_script(Partition *scratch, String opts, String compiler,
                           String script_path, String script_file,
                           String code_file, String output_dir, String binary_file){
    bool32 success = false;
    
    Temp_Memory temp = begin_temp_memory(scratch);
    
    String cf = code_file;
    String od = output_dir;
    String bf = binary_file;
    
    int32_t space_cap = partition_remaining(scratch);
    char *space = push_array(scratch, char, space_cap);
    String file_name = make_string_cap(space, 0, space_cap);
    append(&file_name, script_path);
    append(&file_name, "/");
    append(&file_name, script_file);
    append(&file_name, ".sh");
    terminate_with_null(&file_name);
    
    FILE *sh_script = fopen(file_name.str, "wb");
    if (sh_script != 0){
        fprintf(sh_script, "#!/bin/bash\n\n");
        
        fprintf(sh_script, "code=\"$PWD\"\n");
        
        fprintf(sh_script, "opts=%.*s\n", opts.size, opts.str);
        
        fprintf(sh_script, "cd %.*s > /dev/null\n", od.size, od.str);
        fprintf(sh_script, "%.*s $opts $code/%.*s -o %.*s\n",
                compiler.size, compiler.str, cf.size, cf.str, bf.size, bf.str);
        fprintf(sh_script, "cd $code > /dev/null\n");
        
        fclose(sh_script);
        success = true;
    }
    
    end_temp_memory(temp);
    
    return(success);
}

static bool32
project_generate_project_4coder_file(Partition *scratch,
                                     String script_path, String script_file,
                                     String output_dir, String binary_file){
    bool32 success = false;
    
    Temp_Memory temp = begin_temp_memory(scratch);
    
    String od = output_dir;
    String bf = binary_file;
    
    String od_win = string_push(scratch, od.size*2);
    String bf_win = string_push(scratch, bf.size*2);
    
    append(&od_win, od);
    append(&bf_win, bf);
    
    replace_str(&od_win, "/", "\\\\");
    replace_str(&bf_win, "/", "\\\\");
    
    int32_t space_cap = partition_remaining(scratch);
    char *space = push_array(scratch, char, space_cap);
    String file_name = make_string_cap(space, 0, space_cap);
    append(&file_name, script_path);
    append(&file_name, "/project.4coder");
    terminate_with_null(&file_name);
    
    FILE *out = fopen(file_name.str, "wb");
    if (out != 0){
        fprintf(out, "version(1);\n");
        fprintf(out, "project_name = \"%.*s\";\n", binary_file.size, binary_file.str);
        fprintf(out, "patterns = {\n");
        fprintf(out, "\"*.c\",\n");
        fprintf(out, "\"*.cpp\",\n");
        fprintf(out, "\"*.h\",\n");
        fprintf(out, "\"*.m\",\n");
        fprintf(out, "\"*.bat\",\n");
        fprintf(out, "\"*.sh\",\n");
        fprintf(out, "\"*.4coder\",\n");
        fprintf(out, "};\n");
        fprintf(out, "blacklist_patterns = {\n");
        fprintf(out, "\".*\",\n");
        fprintf(out, "};\n");
        fprintf(out, "load_paths_base = {\n");
        fprintf(out, " { \".\", .relative = true, .recursive = true, },\n");
        fprintf(out, "};\n");
        fprintf(out, "load_paths = {\n");
        fprintf(out, " { load_paths_base, .os = \"win\", },\n");
        fprintf(out, " { load_paths_base, .os = \"linux\", },\n");
        fprintf(out, " { load_paths_base, .os = \"mac\", },\n");
        fprintf(out, "};\n");
        
        fprintf(out, "\n");
        
        fprintf(out, "command_list = {\n");
        fprintf(out, " { .name = \"build\",\n");
        fprintf(out, "   .out = \"*compilation*\", .footer_panel = true, .save_dirty_files = true,\n");
        fprintf(out, "   .cmd = { { \"%.*s.bat\" , .os = \"win\"   },\n",
                script_file.size, script_file.str);
        fprintf(out, "            { \"./%.*s.sh\", .os = \"linux\" },\n",
                script_file.size, script_file.str);
        fprintf(out, "            { \"./%.*s.sh\", .os = \"mac\"   }, }, },\n",
                script_file.size, script_file.str);
        fprintf(out, " { .name = \"run\",\n");
        fprintf(out, "   .out = \"*run*\", .footer_panel = false, .save_dirty_files = false,\n");
        fprintf(out, "   .cmd = { { \"%.*s\\\\%.*s\", .os = \"win\"   },\n",
                od_win.size, od_win.str, bf_win.size, bf_win.str);
        fprintf(out, "            { \"%.*s/%.*s\" , .os = \"linux\" },\n",
                od.size, od.str, bf.size, bf.str);
        fprintf(out, "            { \"%.*s/%.*s\" , .os = \"mac\"   }, }, },\n",
                od.size, od.str, bf.size, bf.str);
        fprintf(out, "};\n");
        
        fprintf(out, "fkey_command[1] = \"build\";\n");
        fprintf(out, "fkey_command[2] = \"run\";\n");
        
        fclose(out);
        success = true;
    }
    
    end_temp_memory(temp);
    
    return(success);
}

static void
project_setup_scripts__generic(Application_Links *app, Partition *scratch,
                               bool32 do_project_file,
                               bool32 do_bat_script,
                               bool32 do_sh_script){
    Temp_Memory temp = begin_temp_memory(scratch);
    String script_path = get_hot_directory(app, scratch);
    
    bool32 needs_to_do_work = false;
    Project_Setup_Status status = {};
    if (do_project_file){
        status = project_is_setup(app, scratch, script_path, make_lit_string("build"));
        needs_to_do_work =
            !status.project_exists ||
            (do_bat_script && !status.bat_exists) ||
            (do_sh_script && !status.sh_exists);
    }
    else{
        needs_to_do_work = true;
    }
    
    if (needs_to_do_work){
        // Query the User for Key File Names
        char script_file_space[1024];
        char code_file_space  [1024];
        char output_dir_space [1024];
        char binary_file_space[1024];
        
        bool32 get_script_file = !do_project_file;
        bool32 get_code_file =
            (do_bat_script && !status.bat_exists) ||
            (do_sh_script && !status.sh_exists);
        
        Project_Key_Strings keys = 
            project_key_strings_query_user(app,
                                           get_script_file, get_code_file,
                                           script_file_space, sizeof(script_file_space),
                                           code_file_space, sizeof(code_file_space),
                                           output_dir_space, sizeof(output_dir_space),
                                           binary_file_space, sizeof(binary_file_space));
        
        if (!keys.success){
            return;
        }
        
        if (do_project_file){
            keys.script_file = make_lit_string("build");
        }
        
        if (!do_project_file){
            status = project_is_setup(app, scratch, script_path, keys.script_file);
        }
        
        // Generate Scripts
        if (do_bat_script){
            if (!status.bat_exists){
                if (!project_generate_bat_script(scratch,
                                                 global_config.default_flags_bat,
                                                 global_config.default_compiler_bat,
                                                 script_path, keys.script_file,
                                                 keys.code_file, keys.output_dir, keys.binary_file)){
                    print_message(app, literal("could not create build.bat for new project\n"));
                }
            }
            else{
                print_message(app, literal("the batch script already exists, no changes made to it\n"));
            }
        }
        
        if (do_sh_script){
            if (!status.bat_exists){
                if (!project_generate_sh_script(scratch,
                                                global_config.default_flags_sh,
                                                global_config.default_compiler_sh,
                                                script_path, keys.script_file,
                                                keys.code_file, keys.output_dir, keys.binary_file)){
                    print_message(app, literal("could not create build.sh for new project\n"));
                }
            }
            else{
                print_message(app, literal("the shell script already exists, no changes made to it\n"));
            }
        }
        
        if (do_project_file){
            if (!status.project_exists){
                if (!project_generate_project_4coder_file(scratch, script_path, keys.script_file,
                                                          keys.output_dir, keys.binary_file)){
                    print_message(app, literal("could not create project.4coder for new project\n"));
                }
            }
            else{
                print_message(app, literal("project.4coder already exists, no changes made to it\n"));
            }
        }
    }
    else{
        if (do_project_file){
            print_message(app, literal("project already setup, no changes made\n"));
        }
    }
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(setup_new_project)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.")
{
    project_setup_scripts__generic(app, &global_part, true, true, true);
    load_project(app);
}

CUSTOM_COMMAND_SIG(setup_build_bat)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build batch script.")
{
    project_setup_scripts__generic(app, &global_part, false, true, false);
}

CUSTOM_COMMAND_SIG(setup_build_sh)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build shell script.")
{
    project_setup_scripts__generic(app, &global_part, false, false, true);
}

CUSTOM_COMMAND_SIG(setup_build_bat_and_sh)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build batch script.")
{
    project_setup_scripts__generic(app, &global_part, false, true, true);
}

///////////////////////////////

static void
activate_project_command(Application_Links *app, Partition *scratch, Heap *heap,
                         View_Summary *view, Lister_State *state,
                         String text_field, void *user_data, bool32 activated_by_mouse){
    int32_t command_index = (int32_t)PtrAsInt(user_data);
    exec_project_command_by_index(app, command_index);
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(project_command_lister)
CUSTOM_DOC("Open a lister of all commands in the currently loaded project.")
{
    if (!current_project.loaded){
        return;
    }
    
    Partition *arena = &global_part;
    
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    Temp_Memory temp = begin_temp_memory(arena);
    int32_t option_count = current_project.command_array.count;
    Lister_Option *options = push_array(arena, Lister_Option, option_count);
    for (int32_t i = 0; i < current_project.command_array.count; i += 1){
        options[i].string = string_push_copy(arena, current_project.command_array.commands[i].name);
        options[i].status = string_push_copy(arena, current_project.command_array.commands[i].cmd);
        options[i].user_data = IntAsPtr(i);
    }
    begin_integrated_lister__basic_list(app, "Command:", activate_project_command, 0, 0,
                                        options, option_count,
                                        0,
                                        &view);
    end_temp_memory(temp);
}

// BOTTOM

