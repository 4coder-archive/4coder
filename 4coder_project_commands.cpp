/*
4coder_project_commands.cpp - commands for loading and using a project.
*/

// TOP

static Project current_project = {0};
static Partition current_project_arena = {0};

///////////////////////////////

static Project_File_Pattern_Array
get_pattern_array_from_cstring_array(Partition *arena, CString_Array list){
    Project_File_Pattern_Array array = {0};
    int32_t count = list.count;
    array.patterns = push_array(arena, Project_File_Pattern, count);
    array.count = count;
    for (int32_t i = 0; i < count; ++i){
        String base_str = make_string_slowly(list.strings[i]);
        String str = push_string(arena, base_str.size + 3);
        append(&str, "*.");
        append(&str, base_str);
        terminate_with_null(&str);
        get_absolutes(str, &array.patterns[i].absolutes, false, false);
    }
    return(array);
}

static Project_File_Pattern_Array
get_pattern_array_from_extension_list(Partition *arena, Extension_List extension_list){
    CString_Array list = {0};
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
        Buffer_Summary buffer = {0};
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
    CString_Array black_array = {0};
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
    String str = {0};
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
            command->name = push_string(arena, 4);
            append_int_to_str(&command->name, j);
            terminate_with_null(&command->name);
            
            String cmd = {0};
            if (config_compound_string_member(parsed, compound, "cmd", 0, &cmd)){
                command->cmd = push_string_copy(arena, cmd);
            }
            
            String out = {0};
            if (config_compound_string_member(parsed, compound, "out", 1, &out)){
                command->out = push_string_copy(arena, out);
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
            command->name = push_string(arena, 4);
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
        int32_t count = typed_array_get_count(parsed, compound, ConfigRValueType_String);
        
        array_out->patterns = push_array(arena, Project_File_Pattern, count);
        array_out->count = count;
        
        for (int32_t i = 0, c = 0; c < count; ++i){
            String str = {0};
            if (config_compound_string_member(parsed, compound, "~", i, &str)){
                str = push_string_copy(arena, str);
                get_absolutes(str, &array_out->patterns[c].absolutes, false, false);
                ++c;
            }
        }
    }
}

static Project*
parse_project__config_data__version_1(Partition *arena, String root_dir, Config *parsed){
    Project *project = push_array(arena, Project, 1);
    memset(project, 0, sizeof(*project));
    
    // Set new project directory
    {
        int32_t cap = root_dir.size + 1;
        project->dir = push_string(arena, cap);
        copy(&project->dir, root_dir);
        terminate_with_null(&project->dir);
    }
    
    // project_name
    {
        String str = {0};
        if (config_string_var(parsed, "project_name", 0, &str)){
            project->name = push_string_copy(arena, str);
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
            for (int32_t i = 0;; ++i){
                Config_Compound *paths_option = 0;
                Iteration_Step_Result step_result = typed_array_iteration_step(parsed, compound, ConfigRValueType_Compound, i, &paths_option);
                if (step_result == Iteration_Skip){
                    continue;
                }
                else if (step_result == Iteration_Quit){
                    break;
                }
                
                bool32 use_this_option = false;
                
                Config_Compound *paths = 0;
                if (config_compound_compound_member(parsed, paths_option, "paths", 0, &paths)){
                    String str = {0};
                    if (config_compound_string_member(parsed, paths_option, "os", 1, &str)){
                        if (match(str, make_lit_string(PlatformName))){
                            use_this_option = true;
                        }
                    }
                }
                
                if (use_this_option){
                    int32_t count = typed_array_get_count(parsed, paths, ConfigRValueType_Compound);
                    
                    project->load_path_array.paths = push_array(arena, Project_File_Load_Path, count);
                    project->load_path_array.count = count;
                    
                    Project_File_Load_Path *dst = project->load_path_array.paths;
                    for (int32_t j = 0, c = 0; c < count; ++j){
                        Config_Compound *src = 0;
                        if (config_compound_compound_member(parsed, paths, "~", j, &src)){
                            memset(dst, 0, sizeof(*dst));
                            dst->recursive = true;
                            dst->relative = true;
                            
                            String str = {0};
                            if (config_compound_string_member(parsed, src, "path", 0, &str)){
                                dst->path = push_string_copy(arena, str);
                            }
                            
                            config_compound_bool_member(parsed, src, "recursive", 1, &dst->recursive);
                            config_compound_bool_member(parsed, src, "relative", 1, &dst->relative);
                            
                            ++c;
                            ++dst;
                        }
                    }
                    
                    break;
                }
            }
        }
    }
    
    // command_list
    {
        Config_Compound *compound = 0;
        if (config_compound_var(parsed, "command_list", 0, &compound)){
            int32_t count = typed_array_get_count(parsed, compound, ConfigRValueType_Compound);
            
            project->command_array.commands = push_array(arena, Project_Command, count);
            project->command_array.count = count;
            
            Project_Command *dst = project->command_array.commands;
            for (int32_t i = 0, c = 0; c < count; ++i){
                
                Config_Compound *src = 0;
                if (config_compound_compound_member(parsed, compound, "~", i, &src)){
                    memset(dst, 0, sizeof(*dst));
                    
                    bool32 can_emit_command = true;
                    
                    String name = {0};
                    Config_Compound *cmd_set = 0;
                    String out = {0};
                    bool32 footer_panel = false;
                    bool32 save_dirty_files = true;
                    String cmd_str = {0};
                    
                    if (!config_compound_string_member(parsed, src, "name", 0, &name)){
                        can_emit_command = false;
                        goto finish_command;
                    }
                    
                    if (!config_compound_compound_member(parsed, src, "cmd", 1, &cmd_set)){
                        can_emit_command = false;
                        goto finish_command;
                    }
                    
                    can_emit_command = false;
                    for (int32_t j = 0;; ++j){
                        Config_Compound *cmd_option = 0;
                        Iteration_Step_Result step_result = typed_array_iteration_step(parsed, cmd_set, ConfigRValueType_Compound, j, &cmd_option);
                        if (step_result == Iteration_Skip){
                            continue;
                        }
                        else if (step_result == Iteration_Quit){
                            break;
                        }
                        
                        bool32 use_this_option = false;
                        
                        String cmd = {0};
                        if (config_compound_string_member(parsed, cmd_option, "cmd", 0, &cmd)){
                            String str = {0};
                            if (config_compound_string_member(parsed, cmd_option, "os", 1, &str)){
                                if (match(str, make_lit_string(PlatformName))){
                                    use_this_option = true;
                                }
                            }
                        }
                        
                        if (use_this_option){
                            can_emit_command = true;
                            cmd_str = cmd;
                            break;
                        }
                    }
                    
                    if (can_emit_command){
                        config_compound_string_member(parsed, src, "out", 2, &out);
                        config_compound_bool_member(parsed, src, "footer_panel", 3, &footer_panel);
                        config_compound_bool_member(parsed, src, "save_dirty_files", 4,
                                                    &save_dirty_files);
                        
                        dst->name = push_string_copy(arena, name);
                        dst->cmd = push_string_copy(arena, cmd_str);
                        dst->out = push_string_copy(arena, out);
                        dst->footer_panel = footer_panel;
                        dst->save_dirty_files = save_dirty_files;
                    }
                    
                    finish_command:;
                    ++dst;
                    ++c;
                }
            }
        }
    }
    
    // fkey_command
    {
        for (int32_t i = 1; i <= 16; ++i){
            String name = {0};
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
    Project_Parse_Result result = {0};
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
    Project_Parse_Result result = {0};
    
    Temp_Memory restore_point = begin_temp_memory(arena);
    String project_path = {0};
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
            dst->absolutes.a[j] = push_string_copy(arena, src->absolutes.a[j]);
            if (dst->absolutes.a[j].str == 0){
                return(false);
            }
        }
    }
    
    return(true);
}

static Project
project_deep_copy__inner(Partition *arena, Project *project){
    Project result = {0};
    
    result.dir = push_string_copy(arena, project->dir);
    if (result.dir.str == 0){
        return(result);
    }
    
    result.name = push_string_copy(arena, project->name);
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
            dst->path = push_string_copy(arena, src->path);
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
                dst->name = push_string_copy(arena, src->name);
                if (dst->name.str == 0){
                    return(result);
                }
            }
            if (src->cmd.str != 0){
                dst->cmd = push_string_copy(arena, src->cmd);
                if (dst->cmd.str == 0){
                    return(result);
                }
            }
            if (src->out.str != 0){
                dst->out = push_string_copy(arena, src->out);
                if (dst->out.str == 0){
                    return(result);
                }
            }
            dst->footer_panel = src->footer_panel;
            dst->save_dirty_files = src->save_dirty_files;
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
set_current_project(Application_Links *app, Partition *scratch, Project *project, Config *parsed){
    bool32 print_errors = false;
    
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
            
            print_errors = true;
            
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
        print_errors = true;
    }
    
    if (print_errors){
        Temp_Memory temp = begin_temp_memory(scratch);
        String error_text = config_stringize_errors(scratch, parsed);
        print_message(app, error_text.str, error_text.size);
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
exec_project_fkey_command(Application_Links *app, int32_t fkey_index){
    if (!current_project.loaded){
        return;
    }
    
    int32_t command_index = current_project.fkey_commands[fkey_index];
    if (command_index < 0 || command_index >= current_project.command_array.count){
        return;
    }
    
    Project_Command *fkey = &current_project.command_array.commands[command_index];
    if (fkey->cmd.size > 0){
        bool32 footer_panel = fkey->footer_panel;
        bool32 save_dirty_files = fkey->save_dirty_files;
        
        if (save_dirty_files){
            save_all_dirty_buffers(app);
        }
        
        View_Summary view = {0};
        View_Summary *view_ptr = 0;
        Buffer_Identifier buffer_id = {0};
        uint32_t flags = CLI_OverlapWithConflict;
        
        bool32 set_fancy_font = false;
        if (fkey->out.size > 0){
            buffer_id = buffer_identifier(fkey->out.str, fkey->out.size);
            
            if (footer_panel){
                view = get_or_open_build_panel(app);
                if (match(fkey->out, "*compilation*")){
                    set_fancy_font = true;
                }
            }
            else{
                view = get_active_view(app, AccessAll);
            }
            view_ptr = &view;
            
            memset(&prev_location, 0, sizeof(prev_location));
            lock_jump_buffer(fkey->out.str, fkey->out.size);
        }
        else{
            // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
            buffer_id = buffer_identifier(literal("*dump*"));
        }
        
        String dir = current_project.dir;
        String cmd = fkey->cmd;
        exec_system_command(app, view_ptr, buffer_id, dir.str, dir.size, cmd.str, cmd.size, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
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
    if (input.type == UserInputKey){
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
project_is_setup(Application_Links *app, char *dir, int32_t dir_len, int32_t dir_capacity){
    Project_Setup_Status result = {0};
    
    Temp_Memory temp = begin_temp_memory(&global_part);
    
    static int32_t needed_extra_space = 15;
    
    String str = {0};
    if (dir_capacity >= dir_len + needed_extra_space){
        str = make_string_cap(dir, dir_len, dir_capacity);
    }
    else{
        char *space = push_array(&global_part, char, dir_len + needed_extra_space);
        str = make_string_cap(space, 0, dir_len + needed_extra_space);
        copy(&str, make_string(dir, dir_len));
    }
    
    str.size = dir_len;
    append(&str, "/build.bat");
    result.bat_exists = file_exists(app, str.str, str.size);
    
    str.size = dir_len;
    append(&str, "/build.sh");
    result.sh_exists = file_exists(app, str.str, str.size);
    
    str.size = dir_len;
    append(&str, "/project.4coder");
    result.project_exists = file_exists(app, str.str, str.size);
    
    result.everything_exists = result.bat_exists && result.sh_exists && result.project_exists;
    
    end_temp_memory(temp);
    
    return(result);
}

CUSTOM_COMMAND_SIG(setup_new_project)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.")
{
    char space[4096];
    String str = make_fixed_width_string(space);
    str.size = directory_get_hot(app, str.str, str.memory_size);
    int32_t dir_size = str.size;
    
    Project_Setup_Status status = project_is_setup(app, str.str, dir_size, str.memory_size);
    if (!status.everything_exists){
        // Query the User for Key File Names
        Query_Bar code_file_bar = {0};
        Query_Bar output_dir_bar = {0};
        Query_Bar binary_file_bar = {0};
        char code_file_space[1024];
        char output_dir_space[1024];
        char binary_file_space[1024];
        
        if (!status.bat_exists || !status.sh_exists){
            code_file_bar.prompt = make_lit_string("Build Target: ");
            code_file_bar.string = make_fixed_width_string(code_file_space);
            
            if (!query_user_string(app, &code_file_bar)) return;
            if (code_file_bar.string.size == 0) return;
        }
        
        output_dir_bar.prompt = make_lit_string("Output Directory: ");
        output_dir_bar.string = make_fixed_width_string(output_dir_space);
        
        if (!query_user_string(app, &output_dir_bar)) return;
        if (output_dir_bar.string.size == 0){
            copy(&output_dir_bar.string, ".");
        }
        
        
        binary_file_bar.prompt = make_lit_string("Binary Output: ");
        binary_file_bar.string = make_fixed_width_string(binary_file_space);
        
        if (!query_user_string(app, &binary_file_bar)) return;
        if (binary_file_bar.string.size == 0) return;
        
        
        String code_file = code_file_bar.string;
        String output_dir = output_dir_bar.string;
        String binary_file = binary_file_bar.string;
        
        // Generate Scripts
        if (!status.bat_exists){
            replace_char(&code_file, '/', '\\');
            replace_char(&output_dir, '/', '\\');
            replace_char(&binary_file, '/', '\\');
            
            str.size = dir_size;
            append(&str, "/build.bat");
            terminate_with_null(&str);
            FILE *bat_script = fopen(str.str, "wb");
            if (bat_script != 0){
                fprintf(bat_script, "@echo off\n\n");
                fprintf(bat_script, "set opts=%.*s\n", 
                        global_config.default_flags_bat.size,
                        global_config.default_flags_bat.str);
                fprintf(bat_script, "set code=%%cd%%\n");
                
                fprintf(bat_script, "pushd %.*s\n", 
                        output_dir.size, output_dir.str);
                fprintf(bat_script, "%.*s %%opts%% %%code%%\\%.*s -Fe%.*s\n",
                        global_config.default_compiler_bat.size,
                        global_config.default_compiler_bat.str,
                        code_file.size, code_file.str,
                        binary_file.size, binary_file.str);
                fprintf(bat_script, "popd\n");
                
                fclose(bat_script);
            }
            else{
                print_message(app, literal("could not create build.bat for new project\n"));
            }
            
            replace_char(&code_file, '\\', '/');
            replace_char(&output_dir, '\\', '/');
            replace_char(&binary_file, '\\', '/');
        }
        else{
            print_message(app, literal("build.bat already exists, no changes made to it\n"));
        }
        
        if (!status.sh_exists){
            str.size = dir_size;
            append(&str, "/build.sh");
            terminate_with_null(&str);
            FILE *sh_script = fopen(str.str, "wb");
            if (sh_script != 0){
                fprintf(sh_script, "#!/bin/bash\n\n");
                
                fprintf(sh_script, "code=\"$PWD\"\n");
                
                fprintf(sh_script, "opts=%.*s\n", 
                        global_config.default_flags_sh.size,
                        global_config.default_flags_sh.str);
                
                fprintf(sh_script, "cd %.*s > /dev/null\n", 
                        output_dir.size, output_dir.str);
                fprintf(sh_script, "%.*s $opts $code/%.*s -o %.*s\n",
                        global_config.default_compiler_sh.size,
                        global_config.default_compiler_sh.str,
                        code_file.size, code_file.str,
                        binary_file.size, binary_file.str);
                fprintf(sh_script, "cd $code > /dev/null\n");
                
                fclose(sh_script);
            }
            else{
                print_message(app, literal("could not create build.sh for new project\n"));
            }
        }
        else{
            print_message(app, literal("build.sh already exists, no changes made to it\n"));
        }
        
        if (!status.project_exists){
            str.size = dir_size;
            append(&str, "/project.4coder");
            terminate_with_null(&str);
            FILE *project_script = fopen(str.str, "wb");
            if (project_script != 0){
                fprintf(project_script, "extensions = \".c.cpp.h.m.bat.sh.4coder\";\n");
                fprintf(project_script, "open_recursively = true;\n\n");
                
                replace_str(&code_file, "/", "\\\\");
                replace_str(&output_dir, "/", "\\\\");
                replace_str(&binary_file, "/", "\\\\");
                fprintf(project_script, 
                        "fkey_command_win[1] = {\"build.bat\", \"*compilation*\", true , true };\n");
                fprintf(project_script, 
                        "fkey_command_win[2] = {\"%.*s\\\\%.*s\", \"*run*\", false , true };\n", 
                        output_dir.size, output_dir.str,
                        binary_file.size, binary_file.str);
                replace_str(&code_file, "\\\\", "/");
                replace_str(&output_dir, "\\\\", "/");
                replace_str(&binary_file, "\\\\", "/");
                
                fprintf(project_script, "fkey_command_linux[1] = {\"./build.sh\", \"*compilation*\", true , true };\n");
                fprintf(project_script, 
                        "fkey_command_linux[2] = {\"%.*s/%.*s\", \"*run*\", false , true };\n", 
                        output_dir.size, output_dir.str,
                        binary_file.size, binary_file.str);
                
                fprintf(project_script, "fkey_command_mac[1] = {\"./build.sh\", \"*compilation*\", true , true };\n");
                fprintf(project_script, 
                        "fkey_command_mac[2] = {\"%.*s/%.*s\", \"*run*\", false , true };\n", 
                        output_dir.size, output_dir.str,
                        binary_file.size, binary_file.str);
                fclose(project_script);
            }
            else{
                print_message(app, literal("could not create project.4coder for new project\n"));
            }
        }
        else{
            print_message(app, literal("project.4coder already exists, no changes made to it\n"));
        }
        
    }
    else{
        print_message(app, literal("project already setup, no changes made\n"));
    }
    
    load_project(app);
}

// BOTTOM

