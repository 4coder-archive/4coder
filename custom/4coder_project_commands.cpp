/*
4coder_project_commands.cpp - commands for loading and using a project.
*/

// TOP

#if 1
global Project current_project = {};
global Arena current_project_arena = {};
#endif

///////////////////////////////

function Match_Pattern_List
prj_pattern_list_from_extension_array(Arena *arena, String_Const_u8_Array list){
    Match_Pattern_List result = {};
    for (i32 i = 0;
         i < list.count;
         ++i){
        Match_Pattern_Node *node = push_array(arena, Match_Pattern_Node, 1);
        sll_queue_push(result.first, result.last, node);
        result.count += 1;
        
        String_Const_u8 str = push_stringf(arena, "*.%.*s", string_expand(list.vals[i]));
        node->pattern.absolutes = string_split_wildcards(arena, str);
    }
    return(result);
}

function Match_Pattern_List
prj_pattern_list_from_var(Arena *arena, Variable_Handle var){
    Match_Pattern_List result = {};
    for (Vars_Children(child_var, var)){
        Match_Pattern_Node *node = push_array(arena, Match_Pattern_Node, 1);
        sll_queue_push(result.first, result.last, node);
        result.count += 1;
        
        String_Const_u8 str = vars_string_from_var(arena, child_var);
        node->pattern.absolutes = string_split_wildcards(arena, str);
    }
    return(result);
}

///////////////////////////////

function void
close_all_files_with_extension(Application_Links *app, String_Const_u8_Array extension_array){
    Scratch_Block scratch(app);
    
    i32 buffers_to_close_max = Thousand(100);
    Buffer_ID *buffers_to_close = push_array(scratch, Buffer_ID, buffers_to_close_max);
    
    b32 do_repeat = false;
    do{
        i32 buffers_to_close_count = 0;
        do_repeat = false;
        
        for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, Access_Always)){
            b32 is_match = true;
            
            if (extension_array.count > 0){
                Temp_Memory name_temp = begin_temp(scratch);
                String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer);
                is_match = false;
                if (file_name.size > 0){
                    String_Const_u8 extension = string_file_extension(file_name);
                    for (i32 i = 0; i < extension_array.count; ++i){
                        if (string_match(extension, extension_array.strings[i])){
                            is_match = true;
                            break;
                        }
                    }
                }
                end_temp(name_temp);
            }
            
            if (is_match){
                if (buffers_to_close_count >= buffers_to_close_max){
                    do_repeat = true;
                    break;
                }
                buffers_to_close[buffers_to_close_count++] = buffer;
            }
        }
        
        for (i32 i = 0; i < buffers_to_close_count; ++i){
            buffer_kill(app, buffers_to_close[i], BufferKill_AlwaysKill);
        }
    }while(do_repeat);
}

function b32
match_in_pattern_list(String_Const_u8 string, Match_Pattern_List list){
    b32 found_match = false;
    for (Match_Pattern_Node *node = list.first;
         node != 0;
         node = node->next){
        if (string_wildcard_match(node->pattern.absolutes, string, StringMatch_Exact)){
            found_match = true;
            break;
        }
    }
    return(found_match);
}

function void
open_files_pattern_match__recursive(Application_Links *app, String_Const_u8 path,
                                    Match_Pattern_List whitelist, Match_Pattern_List blacklist, u32 flags){
    Scratch_Block scratch(app);
    
    ProfileScopeNamed(app, "get file list", profile_get_file_list);
    File_List list = system_get_file_list(scratch, path);
    ProfileCloseNow(profile_get_file_list);
    
    File_Info **info = list.infos;
    for (u32 i = 0; i < list.count; ++i, ++info){
        String_Const_u8 file_name = (**info).file_name;
        if (HasFlag((**info).attributes.flags, FileAttribute_IsDirectory)){
            if ((flags & OpenAllFilesFlag_Recursive) == 0){
                continue;
            }
            if (match_in_pattern_list(file_name, blacklist)){
                continue;
            }
            String_Const_u8 new_path = push_u8_stringf(scratch, "%.*s%.*s/", string_expand(path), string_expand(file_name));
            open_files_pattern_match__recursive(app, new_path, whitelist, blacklist, flags);
        }
        else{
            if (!match_in_pattern_list(file_name, whitelist)){
                continue;
            }
            if (match_in_pattern_list(file_name, blacklist)){
                continue;
            }
            String_Const_u8 full_path = push_u8_stringf(scratch, "%.*s%.*s", string_expand(path), string_expand(file_name));
            create_buffer(app, full_path, 0);
        }
    }
}

// TODO(allen): dumb name
function Match_Pattern_List
get_standard_blacklist(Arena *arena){
    String_Const_u8 dot = string_u8_litexpr(".*");
    String_Const_u8_Array black_array = {};
    black_array.strings = &dot;
    black_array.count = 1;
    return(prj_pattern_list_from_extension_array(arena, black_array));
}

function void
open_files_pattern_match(Application_Links *app, String_Const_u8 dir,
                         Match_Pattern_List whitelist, Match_Pattern_List blacklist, u32 flags){
    ProfileScope(app, "open all files in directory pattern");
    Scratch_Block scratch(app);
    String_Const_u8 directory = dir;
    if (!character_is_slash(string_get_character(directory, directory.size - 1))){
        directory = push_u8_stringf(scratch, "%.*s/", string_expand(dir));
    }
    open_files_pattern_match__recursive(app, directory, whitelist, blacklist, flags);
}

function void
open_files_with_extension(Application_Links *app, String_Const_u8 dir, String_Const_u8_Array array, u32 flags){
    Scratch_Block scratch(app);
    Match_Pattern_List whitelist = prj_pattern_list_from_extension_array(scratch, array);
    Match_Pattern_List blacklist = get_standard_blacklist(scratch);
    open_files_pattern_match(app, dir, whitelist, blacklist, flags);
}

function void
open_all_files_in_hot_with_extension(Application_Links *app, String_Const_u8_Array array, u32 flags){
    Scratch_Block scratch(app);
    String_Const_u8 hot = push_hot_directory(app, scratch);
    String_Const_u8 directory = hot;
    if (!character_is_slash(string_get_character(hot, hot.size - 1))){
        directory = push_u8_stringf(scratch, "%.*s/", string_expand(hot));
    }
    open_files_with_extension(app, hot, array, flags);
}

///////////////////////////////

function void
parse_project__extract_pattern_list(Arena *arena, Config *parsed, char *root_variable_name, Match_Pattern_List *list_out){
    Config_Compound *compound = 0;
    if (config_compound_var(parsed, root_variable_name, 0, &compound)){
        Config_Get_Result_List list = typed_string_array_reference_list(arena, parsed, compound);
        for (Config_Get_Result_Node *cfg_node = list.first;
             cfg_node != 0;
             cfg_node = cfg_node->next){
            Match_Pattern_Node *node = push_array(arena, Match_Pattern_Node, 1);
            sll_queue_push(list_out->first, list_out->last, node);
            list_out->count += 1;
            String_Const_u8 str = push_string_copy(arena, cfg_node->result.string);
            node->pattern.absolutes = string_split_wildcards(arena, str);
        }
    }
}

function Project_OS_Match_Level
parse_project__version_1__os_match(String_Const_u8 str, String_Const_u8 this_os_str){
    if (string_match(str, this_os_str)){
        return(ProjectOSMatchLevel_ActiveMatch);
    }
    else if (string_match(str, string_u8_litexpr("all"))){
        return(ProjectOSMatchLevel_ActiveMatch);
    }
    else if (string_match(str, string_u8_litexpr("default"))){
        return(ProjectOSMatchLevel_PassiveMatch);
    }
    return(ProjectOSMatchLevel_NoMatch);
}

function Project*
parse_project__config_data__version_1(Application_Links *app, Arena *arena, String_Const_u8 root_dir, Config *parsed){
    Project *project = push_array_zero(arena, Project, 1);
    
    // Set new project directory
    project->dir = push_string_copy(arena, root_dir);
    
    // project_name
    {
        String_Const_u8 str = {};
        if (config_string_var(parsed, "project_name", 0, &str)){
            project->name = push_string_copy(arena, str);
        }
        else{
            project->name = SCu8("");
        }
    }
    
    // patterns
    parse_project__extract_pattern_list(arena, parsed, "patterns", &project->pattern_list);
    
    // blacklist_patterns
    parse_project__extract_pattern_list(arena, parsed, "blacklist_patterns", &project->blacklist_pattern_list);
    
    // load_paths
    {
        Config_Compound *compound = 0;
        if (config_compound_var(parsed, "load_paths", 0, &compound)){
            b32 found_match = false;
            Config_Compound *best_paths = 0;
            
            for (i32 i = 0;; ++i){
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
                    String_Const_u8 str = {};
                    if (config_compound_string_member(parsed, paths_option, "os", 1, &str)){
                        Project_OS_Match_Level r = parse_project__version_1__os_match(str, string_u8_litexpr(OS_NAME));
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
                    block_zero_struct(dst);
                    dst->recursive = true;
                    dst->relative = true;
                    
                    String_Const_u8 str = {};
                    if (config_compound_string_member(parsed, src, "path", 0, &str)){
                        dst->path = push_string_copy(arena, str);
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
                u8 *pos = node->result.pos;
                Config_Compound *src = node->result.compound;
                block_zero_struct(dst);
                
                b32 can_emit_command = true;
                
                Config_Get_Result cmd_result = {};
                Config_Compound *cmd_set = 0;
                u8 *cmd_pos = 0;
                String_Const_u8 cmd_str = {};
                String_Const_u8 out = {};
                b32 footer_panel = false;
                b32 save_dirty_files = true;
                b32 cursor_at_end = false;
                String_Const_u8 name = {};
                
                if (!config_compound_string_member(parsed, src, "name", 0, &name)){
                    can_emit_command = false;
                    def_config_push_error(arena, parsed, pos, "a command must have a string type name member");
                    goto finish_command;
                }
                
                cmd_result = config_compound_member(parsed, src, string_u8_litexpr("cmd"), 1);
                if (cmd_result.success && cmd_result.type == ConfigRValueType_Compound){
                    cmd_set = cmd_result.compound;
                    cmd_pos = cmd_result.pos;
                }
                else{
                    can_emit_command = false;
                    def_config_push_error(arena, parsed, pos, "a command must have an array type cmd member");
                    goto finish_command;
                }
                
                can_emit_command = false;
                for (i32 j = 0;; ++j){
                    Config_Iteration_Step_Result result = typed_array_iteration_step(parsed, cmd_set, ConfigRValueType_Compound, j);
                    if (result.step == Iteration_Skip){
                        continue;
                    }
                    else if (result.step == Iteration_Quit){
                        break;
                    }
                    Config_Compound *cmd_option = result.get.compound;
                    
                    String_Const_u8 cmd = {};
                    if (config_compound_string_member(parsed, cmd_option, "cmd", 0, &cmd)){
                        String_Const_u8 str = {};
                        if (config_compound_string_member(parsed, cmd_option, "os", 1, &str)){
                            Project_OS_Match_Level r = parse_project__version_1__os_match(str, string_u8_litexpr(OS_NAME));
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
                    def_config_push_error(arena, parsed, cmd_pos, "no usable command strings found in cmd");
                    goto finish_command;
                }
                
                config_compound_string_member(parsed, src, "out", 2, &out);
                config_compound_bool_member(parsed, src, "footer_panel", 3, &footer_panel);
                config_compound_bool_member(parsed, src, "save_dirty_files", 4,
                                            &save_dirty_files);
                config_compound_bool_member(parsed, src, "cursor_at_end", 5,
                                            &cursor_at_end);
                
                dst->name = push_string_copy(arena, name);
                dst->cmd = push_string_copy(arena, cmd_str);
                dst->out = push_string_copy(arena, out);
                dst->footer_panel = footer_panel;
                dst->save_dirty_files = save_dirty_files;
                dst->cursor_at_end = cursor_at_end;
                
                finish_command:;
            }
        }
    }
    
    // fkey_command
    {
        for (i32 i = 1; i <= 16; ++i){
            String_Const_u8 name = {};
            i32 index = -1;
            if (config_string_var(parsed, "fkey_command", i, &name)){
                i32 count = project->command_array.count;
                Project_Command *command_ptr = project->command_array.commands;
                for (i32 j = 0; j < count; ++j, ++command_ptr){
                    if (string_match(command_ptr->name, name)){
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

function void
project_deep_copy__pattern_list(Arena *arena, Match_Pattern_List *src_list, Match_Pattern_List *dst_list){
    for (Match_Pattern_Node *src_node = src_list->first;
         src_node != 0;
         src_node = src_node->next){
        Match_Pattern_Node *dst_node = push_array(arena, Match_Pattern_Node, 1);
        sll_queue_push(dst_list->first, dst_list->last, dst_node);
        dst_list->count += 1;
        
        for (Node_String_Const_u8 *node = src_node->pattern.absolutes.first;
             node != 0;
             node = node->next){
            String_Const_u8 string = push_string_copy(arena, node->string);
            string_list_push(arena, &dst_node->pattern.absolutes, string);
        }
    }
}

function Project
project_deep_copy__inner(Arena *arena, Project *project){
    Project result = {};
    result.dir = push_string_copy(arena, project->dir);
    result.name = push_string_copy(arena, project->name);
    project_deep_copy__pattern_list(arena, &project->pattern_list, &result.pattern_list);
    project_deep_copy__pattern_list(arena, &project->blacklist_pattern_list, &result.blacklist_pattern_list);
    
    {
        i32 path_count = project->load_path_array.count;
        result.load_path_array.paths = push_array(arena, Project_File_Load_Path, path_count);
        result.load_path_array.count = path_count;
        
        Project_File_Load_Path *dst = result.load_path_array.paths;
        Project_File_Load_Path *src = project->load_path_array.paths;
        for (i32 i = 0; i < path_count; ++i, ++dst, ++src){
            dst->path = push_string_copy(arena, src->path);
            dst->recursive = src->recursive;
            dst->relative = src->relative;
        }
    }
    
    {
        i32 command_count = project->command_array.count;
        result.command_array.commands = push_array_zero(arena, Project_Command, command_count);
        result.command_array.count = command_count;
        
        Project_Command *dst = result.command_array.commands;
        Project_Command *src = project->command_array.commands;
        for (i32 i = 0; i < command_count; ++i, ++dst, ++src){
            if (src->name.str != 0){
                dst->name = push_string_copy(arena, src->name);
            }
            if (src->cmd.str != 0){
                dst->cmd = push_string_copy(arena, src->cmd);
            }
            if (src->out.str != 0){
                dst->out = push_string_copy(arena, src->out);
            }
            dst->footer_panel = src->footer_panel;
            dst->save_dirty_files = src->save_dirty_files;
            dst->cursor_at_end = src->cursor_at_end;
        }
    }
    
    block_copy_array(result.fkey_commands, project->fkey_commands);
    
    result.loaded = true;
    return(result);
}

function Project
project_deep_copy(Arena *arena, Project *project){
    Temp_Memory restore_point = begin_temp(arena);
    Project result = project_deep_copy__inner(arena, project);
    if (!result.loaded){
        end_temp(restore_point);
        block_zero_struct(&result);
    }
    return(result);
}

// NOTE(allen): string list join ("flatten") doesn't really work... :(
function String_Const_u8
prj_join_pattern_string(Arena *arena, List_String_Const_u8 list){
    String_Const_u8 pattern_string = {};
    pattern_string.size = list.total_size + list.node_count - 1;
    pattern_string.str = push_array(arena, u8, pattern_string.size + 1);
    u8 *ptr = pattern_string.str;
    Node_String_Const_u8 *node = list.first;
    if (node != 0){
        for (;;){
            block_copy(ptr, node->string.str, node->string.size);
            ptr += node->string.size;
            node = node->next;
            if (node == 0){
                break;
            }
            *ptr = '*';
            ptr += 1;
        }
    }
    pattern_string.str[pattern_string.size] = 0;
    return(pattern_string);
}

function String_Const_u8
prj_sanitize_string(Arena *arena, String_Const_u8 string){
    String_Const_u8 result = {};
    
    if (string.size > 0){
        result.size = string.size;
        result.str = push_array(arena, u8, result.size + 2);
        
        u8 *in = string.str;
        u8 *out = result.str;
        
        if (character_is_base10(*in)){
            *out = '_';
            out += 1;
            result.size += 1;
        }
        
        for (u64 i = 0; i < string.size; i += 1, in += 1, out += 1){
            u8 c = *in;
            if (!character_is_alpha_numeric(c)){
                c = '_';
            }
            *out = c;
        }
        
        result.str[result.size] = 0;
    }
    
    return(result);
}

function Variable_Handle
prj_version_1_to_version_2(Application_Links *app, Config *parsed, Project *project){
    Scratch_Block scratch(app);
    
    String_ID project_id = vars_save_string_lit("prj_config");
    String_ID version_id = vars_save_string(str8_lit("version"));
    String_ID project_name_id = vars_save_string(str8_lit("project_name"));
    String_ID patterns_id = vars_save_string(str8_lit("patterns"));
    String_ID blacklist_patterns_id = vars_save_string(str8_lit("blacklist_patterns"));
    
    String_ID load_paths_id = vars_save_string(str8_lit("load_paths"));
    String_ID path_id = vars_save_string(str8_lit("path"));
    String_ID recursive_id = vars_save_string(str8_lit("recursive"));
    String_ID relative_id = vars_save_string(str8_lit("relative"));
    String_ID true_id = vars_save_string(str8_lit("true"));
    String_ID false_id = vars_save_string(str8_lit("false"));
    
    String_ID out_id = vars_save_string(str8_lit("out"));
    String_ID footer_panel_id = vars_save_string(str8_lit("footer_panel"));
    String_ID save_dirty_files_id = vars_save_string(str8_lit("save_dirty_files"));
    String_ID cursor_at_end_id = vars_save_string(str8_lit("cursor_at_end"));
    
    String_ID fkey_command_id = vars_save_string(str8_lit("fkey_command"));
    
    String_ID os_id = vars_save_string(str8_lit(OS_NAME));;
    
    Variable_Handle proj_var = vars_new_variable(vars_get_root(), project_id, vars_save_string(parsed->file_name));
    
    if (parsed->version != 0){
        String_Const_u8 version_str = push_stringf(scratch, "%d", *parsed->version);
        vars_new_variable(proj_var, version_id, vars_save_string(version_str));
    }
    
    vars_new_variable(proj_var, project_name_id, vars_save_string(project->name));
    
    // load paterns
    struct PatternVars{
        String_ID id;
        Match_Pattern_List list;
    };
    PatternVars pattern_vars[] = {
        {          patterns_id, project->          pattern_list},
        {blacklist_patterns_id, project->blacklist_pattern_list},
    };
    
    PatternVars *pattern_var = pattern_vars;
    PatternVars *opl = pattern_vars + ArrayCount(pattern_vars);
    for (; pattern_var < opl; pattern_var += 1){
        Variable_Handle patterns = vars_new_variable(proj_var, pattern_var->id);
        
        i32 i = 0;
        for (Match_Pattern_Node *node = pattern_var->list.first;
             node != 0;
             node = node->next, i += 1){
            String_Const_u8 pattern_string = prj_join_pattern_string(scratch, node->pattern.absolutes);
            String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
            String_ID pattern_id = vars_save_string(pattern_string);
            vars_new_variable(patterns, key, pattern_id);
        }
    }
    
    // load paths
    {
        Variable_Handle load_paths = vars_new_variable(proj_var, load_paths_id);
        Variable_Handle os_var = vars_new_variable(load_paths, os_id);
        i32 count = project->load_path_array.count;
        Project_File_Load_Path *load_path = project->load_path_array.paths;
        for (i32 i = 0; i < count; i += 1, load_path += 1){
            String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
            Variable_Handle path_var = vars_new_variable(os_var, key);
            vars_new_variable(path_var, path_id, vars_save_string(load_path->path));
            vars_new_variable(path_var, recursive_id, load_path->recursive?true_id:false_id);
            vars_new_variable(path_var, relative_id, load_path->recursive?true_id:false_id);
        }
    }
    
    // commands
    {
        i32 count = project->command_array.count;
        Project_Command *cmd = project->command_array.commands;
        for (i32 i = 0; i < count; i += 1, cmd += 1){
            String_Const_u8 cmd_name = prj_sanitize_string(scratch, cmd->name);
            Variable_Handle cmd_var = vars_new_variable(proj_var, vars_save_string(cmd_name));
            vars_new_variable(cmd_var, os_id, vars_save_string(cmd->cmd));
            vars_new_variable(cmd_var, out_id, vars_save_string(cmd->out));
            vars_new_variable(cmd_var, footer_panel_id, cmd->footer_panel?true_id:false_id);
            vars_new_variable(cmd_var, save_dirty_files_id, cmd->save_dirty_files?true_id:false_id);
            vars_new_variable(cmd_var, cursor_at_end_id, cmd->cursor_at_end?true_id:false_id);
        }
    }
    
    // fkey_commands
    {
        Variable_Handle fkeys_var = vars_new_variable(proj_var, fkey_command_id);
        for (i32 i = 0; i < 16; i += 1){
            i32 cmd_index = project->fkey_commands[i];
            if (0 <= cmd_index && cmd_index < project->command_array.count){
                Project_Command *cmd = project->command_array.commands + cmd_index;
                if (cmd->name.size > 0){
                    String_Const_u8 cmd_name = prj_sanitize_string(scratch, cmd->name);
                    String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
                    String_ID val = vars_save_string(cmd_name);
                    vars_new_variable(fkeys_var, key, val);
                }
            }
        }
    }
    
	return(proj_var);
}

function String_Const_u8
prj_path_from_project(Arena *arena, Variable_Handle project){
    String_Const_u8 project_full_path = vars_string_from_var(arena, project);
    String_Const_u8 project_dir = string_remove_last_folder(project_full_path);
    return(project_dir);
}

function void
prj_exec_command(Application_Links *app, Variable_Handle cmd_var){
    Scratch_Block scratch(app);
    
    String_ID os_id = vars_save_string_lit(OS_NAME);
    
    String_Const_u8 cmd = vars_string_from_var(scratch, vars_read_key(cmd_var, os_id));
    if (cmd.size > 0){
        String_ID out_id = vars_save_string_lit("out");
        String_ID footer_panel_id = vars_save_string_lit("footer_panel");
        String_ID save_dirty_files_id = vars_save_string_lit("save_dirty_files");
        String_ID cursor_at_end_id = vars_save_string_lit("cursor_at_end");
        
        b32 save_dirty_files = vars_b32_from_var(vars_read_key(cmd_var, save_dirty_files_id));
        if (save_dirty_files){
            save_all_dirty_buffers(app);
        }
        
        u32 flags = CLI_OverlapWithConflict|CLI_SendEndSignal;
        b32 cursor_at_end = vars_b32_from_var(vars_read_key(cmd_var, cursor_at_end_id));
        if (cursor_at_end){
            flags |= CLI_CursorAtEnd;
        }
        
        View_ID view = 0;
        Buffer_Identifier buffer_id = {};
        b32 set_fancy_font = false;
        String_Const_u8 out = vars_string_from_var(scratch, vars_read_key(cmd_var, out_id));
        if (out.size > 0){
            buffer_id = buffer_identifier(out);
            
            b32 footer_panel = vars_b32_from_var(vars_read_key(cmd_var, footer_panel_id));
            if (footer_panel){
                view = get_or_open_build_panel(app);
                if (string_match(out, string_u8_litexpr("*compilation*"))){
                    set_fancy_font = true;
                }
            }
            else{
                Buffer_ID buffer = buffer_identifier_to_id(app, buffer_id);
                view = get_first_view_with_buffer(app, buffer);
                if (view == 0){
                    view = get_active_view(app, Access_Always);
                }
            }
            
            block_zero_struct(&prev_location);
            lock_jump_buffer(app, out);
        }
        else{
            // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
            buffer_id = buffer_identifier(string_u8_litexpr("*dump*"));
        }
        
        Variable_Handle parent = vars_parent(cmd_var);
        String_Const_u8 project_dir = prj_path_from_project(scratch, parent);
        exec_system_command(app, view, buffer_id, project_dir, cmd, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
}

function Variable_Handle
prj_command_from_name(Application_Links *app, String_Const_u8 cmd_name){
    Scratch_Block scratch(app);
    String_ID cmd_name_id = vars_save_string(cmd_name);
    Variable_Handle cmd = def_get_config_var(cmd_name_id);
    return(cmd);
}

function void
prj_exec_command_name(Application_Links *app, String_Const_u8 cmd_name){
    Scratch_Block scratch(app);
    Variable_Handle cmd = prj_command_from_name(app, cmd_name);
    prj_exec_command(app, cmd);
}

function void
prj_exec_command_fkey_index(Application_Links *app, i32 fkey_index){
    // TODO(allen): ideally if one fkey_command is missing this index the fallback
    // can be continued.
    Variable_Handle fkeys = def_get_config_var(vars_save_string_lit("fkey_command"));
    
    // TODO(allen): Ideally I could just pass fkey_index right to vars_read_key
    // in a case like this.
    Scratch_Block scratch(app);
    String_Const_u8 fkey_index_str = push_stringf(scratch, "%d", fkey_index);
    String_ID fkey_index_id = vars_save_string(fkey_index_str);
    Variable_Handle cmd_name_var = vars_read_key(fkeys, fkey_index_id);
    String_Const_u8 cmd_name = vars_string_from_var(scratch, cmd_name_var);
    prj_exec_command_name(app, cmd_name);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(close_all_code)
CUSTOM_DOC("Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.")
{
    Scratch_Block scratch(app);
    String_Const_u8 treat_as_code = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
    String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code);
    close_all_files_with_extension(app, extensions);
}

CUSTOM_COMMAND_SIG(open_all_code)
CUSTOM_DOC("Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.")
{
    Scratch_Block scratch(app);
    String_Const_u8 treat_as_code = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
    String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code);
    open_all_files_in_hot_with_extension(app, extensions, 0);
}

CUSTOM_COMMAND_SIG(open_all_code_recursive)
CUSTOM_DOC("Works as open_all_code but also runs in all subdirectories.")
{
    Scratch_Block scratch(app);
    String_Const_u8 treat_as_code = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
    String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code);
    open_all_files_in_hot_with_extension(app, extensions, OpenAllFilesFlag_Recursive);
}

///////////////////////////////

CUSTOM_COMMAND_SIG(load_project)
CUSTOM_DOC("Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.")
{
    ProfileScope(app, "load project");
    save_all_dirty_buffers(app);
    Scratch_Block scratch(app);
    
    // NOTE(allen): Load the project file from the hot directory
    String_Const_u8 project_path = push_hot_directory(app, scratch);
    File_Name_Data dump = dump_file_search_up_path(app, scratch, project_path, string_u8_litexpr("project.4coder"));
    String_Const_u8 project_root = string_remove_last_folder(dump.file_name);
    
    if (dump.data.str == 0){
        print_message(app, string_u8_litexpr("Did not find project.4coder.\n"));
    }
    
    // NOTE(allen): Parse config data out of project file
    Config *config_parse = 0;
    Variable_Handle proj_var = vars_get_nil();
    if (dump.data.str != 0){
        Token_Array array = token_array_from_text(app, scratch, dump.data);
        if (array.tokens != 0){
            config_parse = def_config_parse(app, scratch, dump.file_name, dump.data, array);
            if (config_parse != 0){
                i32 version = 0;
                if (config_parse->version != 0){
                    version = *config_parse->version;
                }
                
                switch (version){
                    case 0:
                    case 1:
                    {
                        Project *project = parse_project__config_data__version_1(app, scratch, project_root, config_parse);
                        proj_var = prj_version_1_to_version_2(app, config_parse, project);
                    }break;
                    default:
                    {
                        proj_var = def_fill_var_from_config(app, vars_get_root(), vars_save_string_lit("prj_config"), config_parse);
                    }break;
                }
                
            }
        }
    }
    
    // NOTE(allen): Print Project
    if (!vars_is_nil(proj_var)){
        vars_print(app, proj_var);
        print_message(app, string_u8_litexpr("\n"));
    }
    
    // NOTE(allen): Print Errors
    if (config_parse != 0){
        String_Const_u8 error_text = config_stringize_errors(app, scratch, config_parse);
        if (error_text.size > 0){
            print_message(app, string_u8_litexpr("Project errors:\n"));
            print_message(app, error_text);
            print_message(app, string_u8_litexpr("\n"));
        }
    }
    
    // TODO(allen): this is dummy dumb dumb and don't need to be like this.
    // NOTE(allen): Set the normal search list's project slot
    String_Const_u8 project_dir = prj_path_from_project(scratch, proj_var);
    
    if (current_project_arena.base_allocator == 0){
        current_project_arena = make_arena_system();
    }
    linalloc_clear(&current_project_arena);
    def_search_project_path = push_string_copy(&current_project_arena, project_dir);
    
    // NOTE(allen): Open All Project Files
    Variable_Handle load_paths_var = vars_read_key(proj_var, vars_save_string_lit("load_paths"));
    Variable_Handle load_paths_os_var = vars_read_key(load_paths_var, vars_save_string_lit(OS_NAME));
    
    String_ID path_id = vars_save_string_lit("path");
    String_ID recursive_id = vars_save_string_lit("recursive");
    String_ID relative_id = vars_save_string_lit("relative");
    
    Variable_Handle whitelist_var = vars_read_key(proj_var, vars_save_string_lit("patterns"));
    Variable_Handle blacklist_var = vars_read_key(proj_var, vars_save_string_lit("blacklist_patterns"));
    
    Match_Pattern_List whitelist = prj_pattern_list_from_var(scratch, whitelist_var);
    Match_Pattern_List blacklist = prj_pattern_list_from_var(scratch, blacklist_var);
    
    for (Variable_Handle load_path_var = vars_first_child(load_paths_os_var);
         !vars_is_nil(load_path_var);
         load_path_var = vars_next_sibling(load_path_var)){
        Variable_Handle path_var = vars_read_key(load_path_var, path_id);
        Variable_Handle recursive_var = vars_read_key(load_path_var, recursive_id);
        Variable_Handle relative_var = vars_read_key(load_path_var, relative_id);
        
        String_Const_u8 path = vars_string_from_var(scratch, path_var);
        b32 recursive = vars_b32_from_var(recursive_var);
        b32 relative = vars_b32_from_var(relative_var);
        
        
        u32 flags = 0;
        if (recursive){
            flags |= OpenAllFilesFlag_Recursive;
        }
        
        String_Const_u8 file_dir = path;
        if (relative){
            List_String_Const_u8 file_dir_list = {};
            string_list_push(scratch, &file_dir_list, project_dir);
            string_list_push_overlap(scratch, &file_dir_list, '/', path);
            string_list_push_overlap(scratch, &file_dir_list, '/', SCu8());
            file_dir = string_list_flatten(scratch, file_dir_list, StringFill_NullTerminate);
        }
        
        open_files_pattern_match(app, file_dir, whitelist, blacklist, flags);
    }
    
    // NOTE(allen): Set Window Title
    Variable_Handle proj_name_var = vars_read_key(proj_var, vars_save_string_lit("project_name"));
    String_ID proj_name_id = vars_key_id_from_var(proj_var);
    if (proj_name_id != 0){
        String_Const_u8 proj_name = vars_read_string(scratch, proj_name_id);
        String_Const_u8 title = push_u8_stringf(scratch, "4coder project: %.*s", string_expand(proj_name));
        set_window_title(app, title);
    }
}

CUSTOM_COMMAND_SIG(project_fkey_command)
CUSTOM_DOC("Run an 'fkey command' configured in a project.4coder file.  Determines the index of the 'fkey command' by which function key or numeric key was pressed to trigger the command.")
{
    ProfileScope(app, "project fkey command");
    User_Input input = get_current_input(app);
    b32 got_ind = false;
    i32 ind = 0;
    if (input.event.kind == InputEventKind_KeyStroke){
        if (KeyCode_F1 <= input.event.key.code && input.event.key.code <= KeyCode_F16){
            ind = (input.event.key.code - KeyCode_F1);
            got_ind = true;
        }
        else if (KeyCode_1 <= input.event.key.code && input.event.key.code <= KeyCode_9){
            ind = (input.event.key.code - '1');
            got_ind = true;
        }
        else if (input.event.key.code == KeyCode_0){
            ind = 9;
            got_ind = true;
        }
        if (got_ind){
            prj_exec_command_fkey_index(app, ind);
        }
    }
}

CUSTOM_COMMAND_SIG(project_go_to_root_directory)
CUSTOM_DOC("Changes 4coder's hot directory to the root directory of the currently loaded project. With no loaded project nothing hapepns.")
{
    if (current_project.loaded){
        set_hot_directory(app, current_project.dir);
    }
}

///////////////////////////////

function Project_Setup_Status
project_is_setup(Application_Links *app, String_Const_u8 script_path, String_Const_u8 script_file){
    Project_Setup_Status result = {};
    {
        Scratch_Block scratch(app);
        String_Const_u8 bat_path = push_u8_stringf(scratch, "%.*s/%.*s.bat",
                                                   string_expand(script_path),
                                                   string_expand(script_file));
        result.bat_exists = file_exists(app, bat_path);
    }
    {
        Scratch_Block scratch(app);
        String_Const_u8 sh_path = push_u8_stringf(scratch, "%.*s/%.*s.sh",
                                                  string_expand(script_path),
                                                  string_expand(script_file));
        result.sh_exists = file_exists(app, sh_path);
    }
    {
        Scratch_Block scratch(app);
        String_Const_u8 project_path = push_u8_stringf(scratch, "%.*s/project.4coder",
                                                       string_expand(script_path));
        result.sh_exists = file_exists(app, project_path);
    }
    result.everything_exists = (result.bat_exists && result.sh_exists && result.project_exists);
    return(result);
}

function Project_Key_Strings
project_key_strings_query_user(Application_Links *app,
                               b32 get_script_file, b32 get_code_file,
                               u8 *script_file_space, i32 script_file_cap,
                               u8 *code_file_space, i32 code_file_cap,
                               u8 *output_dir_space, i32 output_dir_cap,
                               u8 *binary_file_space, i32 binary_file_cap){
    Project_Key_Strings keys = {};
    
    Query_Bar_Group bar_group(app);
    
    Query_Bar script_file_bar = {};
    Query_Bar code_file_bar = {};
    Query_Bar output_dir_bar = {};
    Query_Bar binary_file_bar = {};
    
    if (get_script_file){
        script_file_bar.prompt = string_u8_litexpr("Script Name: ");
        script_file_bar.string = SCu8(script_file_space, (u64)0);
        script_file_bar.string_capacity = script_file_cap;
        if (!query_user_string(app, &script_file_bar)) return(keys);
        if (script_file_bar.string.size == 0) return(keys);
    }
    
    if (get_code_file){
        code_file_bar.prompt = string_u8_litexpr("Build Target: ");
        code_file_bar.string = SCu8(code_file_space, (u64)0);
        code_file_bar.string_capacity = code_file_cap;
        if (!query_user_string(app, &code_file_bar)) return(keys);
        if (code_file_bar.string.size == 0) return(keys);
    }
    
    output_dir_bar.prompt = string_u8_litexpr("Output Directory: ");
    output_dir_bar.string = SCu8(output_dir_space, (u64)0);
    output_dir_bar.string_capacity = output_dir_cap;
    if (!query_user_string(app, &output_dir_bar)) return(keys);
    if (output_dir_bar.string.size == 0 && output_dir_cap > 0){
        output_dir_bar.string.str[0] = '.';
        output_dir_bar.string.size = 1;
    }
    
    binary_file_bar.prompt = string_u8_litexpr("Binary Output: ");
    binary_file_bar.string = SCu8(binary_file_space, (u64)0);
    binary_file_bar.string_capacity = binary_file_cap;
    if (!query_user_string(app, &binary_file_bar)) return(keys);
    if (binary_file_bar.string.size == 0) return(keys);
    
    keys.success = true;
    keys.script_file = script_file_bar.string;
    keys.code_file = code_file_bar.string;
    keys.output_dir = output_dir_bar.string;
    keys.binary_file = binary_file_bar.string;
    
    return(keys);
}

function b32
project_generate_bat_script(Arena *scratch, String_Const_u8 opts, String_Const_u8 compiler,
                            String_Const_u8 script_path, String_Const_u8 script_file,
                            String_Const_u8 code_file, String_Const_u8 output_dir, String_Const_u8 binary_file){
    b32 success = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_u8 cf = push_string_copy(scratch, code_file);
    String_Const_u8 od = push_string_copy(scratch, output_dir);
    String_Const_u8 bf = push_string_copy(scratch, binary_file);
    
    cf = string_mod_replace_character(cf, '/', '\\');
    od = string_mod_replace_character(od, '/', '\\');
    bf = string_mod_replace_character(bf, '/', '\\');
    
    String_Const_u8 file_name = push_u8_stringf(scratch, "%.*s/%.*s.bat",
                                                string_expand(script_path),
                                                string_expand(script_file));
    
    FILE *bat_script = fopen((char*)file_name.str, "wb");
    if (bat_script != 0){
        fprintf(bat_script, "@echo off\n\n");
        fprintf(bat_script, "set opts=%.*s\n", (i32)opts.size, opts.str);
        fprintf(bat_script, "set code=%%cd%%\n");
        fprintf(bat_script, "pushd %.*s\n", (i32)od.size, od.str);
        fprintf(bat_script, "%.*s %%opts%% %%code%%\\%.*s -Fe%.*s\n",
                (i32)compiler.size, compiler.str, (i32)cf.size, cf.str, (i32)bf.size, bf.str);
        fprintf(bat_script, "popd\n");
        fclose(bat_script);
        success = true;
    }
    
    end_temp(temp);
    
    return(success);
}

function b32
project_generate_sh_script(Arena *scratch, String_Const_u8 opts, String_Const_u8 compiler,
                           String_Const_u8 script_path, String_Const_u8 script_file,
                           String_Const_u8 code_file, String_Const_u8 output_dir, String_Const_u8 binary_file){
    b32 success = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_u8 cf = code_file;
    String_Const_u8 od = output_dir;
    String_Const_u8 bf = binary_file;
    
    String_Const_u8 file_name = push_u8_stringf(scratch, "%.*s/%.*s.sh",
                                                string_expand(script_path),
                                                string_expand(script_file));
    
    FILE *sh_script = fopen((char*)file_name.str, "wb");
    if (sh_script != 0){
        fprintf(sh_script, "#!/bin/bash\n\n");
        fprintf(sh_script, "code=\"$PWD\"\n");
        fprintf(sh_script, "opts=%.*s\n", string_expand(opts));
        fprintf(sh_script, "cd %.*s > /dev/null\n", string_expand(od));
        fprintf(sh_script, "%.*s $opts $code/%.*s -o %.*s\n", string_expand(compiler), string_expand(cf), string_expand(bf));
        fprintf(sh_script, "cd $code > /dev/null\n");
        fclose(sh_script);
        success = true;
    }
    
    end_temp(temp);
    
    return(success);
}

function b32
project_generate_project_4coder_file(Arena *scratch, String_Const_u8 script_path, String_Const_u8 script_file, String_Const_u8 output_dir, String_Const_u8 binary_file){
    b32 success = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_u8 od = output_dir;
    String_Const_u8 bf = binary_file;
    
    String_Const_u8 od_win = string_replace(scratch, od,
                                            string_u8_litexpr("/"), string_u8_litexpr("\\"));
    String_Const_u8 bf_win = string_replace(scratch, bf,
                                            string_u8_litexpr("/"), string_u8_litexpr("\\"));
    
    String_Const_u8 file_name = push_u8_stringf(scratch, "%.*s/project.4coder", string_expand(script_path));
    
    FILE *out = fopen((char*)file_name.str, "wb");
    if (out != 0){
        fprintf(out, "version(1);\n");
        fprintf(out, "project_name = \"%.*s\";\n", string_expand(binary_file));
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
        fprintf(out, "   .cmd = { { \"%.*s.bat\" , .os = \"win\"   },\n", string_expand(script_file));
        fprintf(out, "            { \"./%.*s.sh\", .os = \"linux\" },\n", string_expand(script_file));
        fprintf(out, "            { \"./%.*s.sh\", .os = \"mac\"   }, }, },\n", string_expand(script_file));
        fprintf(out, " { .name = \"run\",\n");
        fprintf(out, "   .out = \"*run*\", .footer_panel = false, .save_dirty_files = false,\n");
        fprintf(out, "   .cmd = { { \"%.*s\\\\%.*s\", .os = \"win\"   },\n", string_expand(od_win), string_expand(bf_win));
        fprintf(out, "            { \"%.*s/%.*s\" , .os = \"linux\" },\n", string_expand(od), string_expand(bf));
        fprintf(out, "            { \"%.*s/%.*s\" , .os = \"mac\"   }, }, },\n", string_expand(od), string_expand(bf));
        fprintf(out, "};\n");
        
        fprintf(out, "fkey_command[1] = \"build\";\n");
        fprintf(out, "fkey_command[2] = \"run\";\n");
        
        fclose(out);
        success = true;
    }
    
    end_temp(temp);
    
    return(success);
}

function void
project_setup_scripts__generic(Application_Links *app, b32 do_project_file, b32 do_bat_script, b32 do_sh_script){
    Scratch_Block scratch(app);
    String_Const_u8 script_path = push_hot_directory(app, scratch);
    
    b32 needs_to_do_work = false;
    Project_Setup_Status status = {};
    if (do_project_file){
        status = project_is_setup(app, script_path, string_u8_litexpr("build"));
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
        u8 script_file_space[1024];
        u8 code_file_space  [1024];
        u8 output_dir_space [1024];
        u8 binary_file_space[1024];
        
        b32 get_script_file = !do_project_file;
        b32 get_code_file =
            (do_bat_script && !status.bat_exists) ||
            (do_sh_script && !status.sh_exists);
        
        Project_Key_Strings keys = {};
        keys = project_key_strings_query_user(app, get_script_file, get_code_file,
                                              script_file_space, sizeof(script_file_space),
                                              code_file_space, sizeof(code_file_space),
                                              output_dir_space, sizeof(output_dir_space),
                                              binary_file_space, sizeof(binary_file_space));
        
        if (!keys.success){
            return;
        }
        
        if (do_project_file){
            keys.script_file = string_u8_litexpr("build");
        }
        
        if (!do_project_file){
            status = project_is_setup(app, script_path, keys.script_file);
        }
        
        // Generate Scripts
        if (do_bat_script){
            if (!status.bat_exists){
                String_Const_u8 default_flags_bat = def_get_config_string(scratch, vars_save_string_lit("default_flags_bat"));
                String_Const_u8 default_compiler_bat = def_get_config_string(scratch, vars_save_string_lit("default_compiler_bat"));
                
                if (!project_generate_bat_script(scratch, default_flags_bat, default_compiler_bat,
                                                 script_path, keys.script_file,
                                                 keys.code_file, keys.output_dir, keys.binary_file)){
                    print_message(app, string_u8_litexpr("could not create build.bat for new project\n"));
                }
            }
            else{
                print_message(app, string_u8_litexpr("the batch script already exists, no changes made to it\n"));
            }
        }
        
        if (do_sh_script){
            if (!status.bat_exists){
                String_Const_u8 default_flags_sh = def_get_config_string(scratch, vars_save_string_lit("default_flags_sh"));
                String_Const_u8 default_compiler_sh = def_get_config_string(scratch, vars_save_string_lit("default_compiler_sh"));
                if (!project_generate_sh_script(scratch, default_flags_sh, default_compiler_sh,
                                                script_path, keys.script_file,
                                                keys.code_file, keys.output_dir, keys.binary_file)){
                    print_message(app, string_u8_litexpr("could not create build.sh for new project\n"));
                }
            }
            else{
                print_message(app, string_u8_litexpr("the shell script already exists, no changes made to it\n"));
            }
        }
        
        if (do_project_file){
            if (!status.project_exists){
                if (!project_generate_project_4coder_file(scratch,
                                                          script_path,
                                                          keys.script_file,
                                                          keys.output_dir,
                                                          keys.binary_file)){
                    print_message(app, string_u8_litexpr("could not create project.4coder for new project\n"));
                }
            }
            else{
                print_message(app, string_u8_litexpr("project.4coder already exists, no changes made to it\n"));
            }
        }
    }
    else{
        if (do_project_file){
            print_message(app, string_u8_litexpr("project already setup, no changes made\n"));
        }
    }
}

CUSTOM_COMMAND_SIG(setup_new_project)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.")
{
    project_setup_scripts__generic(app, true, true, true);
    load_project(app);
}

CUSTOM_COMMAND_SIG(setup_build_bat)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build batch script.")
{
    project_setup_scripts__generic(app, false, true, false);
}

CUSTOM_COMMAND_SIG(setup_build_sh)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build shell script.")
{
    project_setup_scripts__generic(app, false, false, true);
}

CUSTOM_COMMAND_SIG(setup_build_bat_and_sh)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build batch script.")
{
    project_setup_scripts__generic(app, false, true, true);
}

///////////////////////////////

function Variable_Handle
get_project_command_from_user(Application_Links *app, Project *project, String_Const_u8 query){
    Variable_Handle result = vars_get_nil();
    if (project != 0){
        Scratch_Block scratch(app);
        Lister_Block lister(app, scratch);
        lister_set_query(lister, query);
        lister_set_default_handlers(lister);
        
        Project_Command *proj_cmd = project->command_array.commands;
        i32 count = project->command_array.count;
        for (i32 i = 0; i < count; i += 1, proj_cmd += 1){
            lister_add_item(lister, proj_cmd->name, proj_cmd->cmd, proj_cmd, 0);
        }
        
        Lister_Result l_result = run_lister(app, lister);
        if (!l_result.canceled){
            Project_Command *result_proj_cmd = (Project_Command*)l_result.user_data;
            if (result_proj_cmd != 0){
                String_Const_u8 cmd_name = prj_sanitize_string(scratch, result_proj_cmd->name);
                result = prj_command_from_name(app, cmd_name);
            }
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(project_command_lister)
CUSTOM_DOC("Open a lister of all commands in the currently loaded project.")
{
    if (current_project.loaded){
        Variable_Handle proj_cmd =
            get_project_command_from_user(app, &current_project, string_u8_litexpr("Command:"));
        if (!vars_is_nil(proj_cmd)){
            prj_exec_command(app, proj_cmd);
        }
    }
}

// BOTTOM

