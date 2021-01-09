/*
4coder_project_commands.cpp - commands for loading and using a project.
*/

// TOP

////////////////////////////////
// NOTE(allen): File Pattern Operators

function Prj_Pattern_List
prj_pattern_list_from_extension_array(Arena *arena, String8Array list){
    Prj_Pattern_List result = {};
    for (i32 i = 0;
         i < list.count;
         ++i){
        Prj_Pattern_Node *node = push_array(arena, Prj_Pattern_Node, 1);
        sll_queue_push(result.first, result.last, node);
        result.count += 1;
        
        String8 str = push_stringf(arena, "*.%.*s", string_expand(list.vals[i]));
        node->pattern.absolutes = string_split_wildcards(arena, str);
    }
    return(result);
}

function Prj_Pattern_List
prj_pattern_list_from_var(Arena *arena, Variable_Handle var){
    Prj_Pattern_List result = {};
    for (Vars_Children(child_var, var)){
        Prj_Pattern_Node *node = push_array(arena, Prj_Pattern_Node, 1);
        sll_queue_push(result.first, result.last, node);
        result.count += 1;
        
        String8 str = vars_string_from_var(arena, child_var);
        node->pattern.absolutes = string_split_wildcards(arena, str);
    }
    return(result);
}

function Prj_Pattern_List
prj_get_standard_blacklist(Arena *arena){
    String8 dot = string_u8_litexpr(".*");
    String8Array black_array = {};
    black_array.strings = &dot;
    black_array.count = 1;
    return(prj_pattern_list_from_extension_array(arena, black_array));
}

function b32
prj_match_in_pattern_list(String8 string, Prj_Pattern_List list){
    b32 found_match = false;
    for (Prj_Pattern_Node *node = list.first;
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
prj_close_files_with_ext(Application_Links *app, String8Array extension_array){
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
                String8 file_name = push_buffer_file_name(app, scratch, buffer);
                is_match = false;
                if (file_name.size > 0){
                    String8 extension = string_file_extension(file_name);
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

function void
prj_open_files_pattern_filter__rec(Application_Links *app, String8 path, Prj_Pattern_List whitelist, Prj_Pattern_List blacklist, Prj_Open_File_Flags flags){
    Scratch_Block scratch(app);
    
    ProfileScopeNamed(app, "get file list", profile_get_file_list);
    File_List list = system_get_file_list(scratch, path);
    ProfileCloseNow(profile_get_file_list);
    
    File_Info **info = list.infos;
    for (u32 i = 0; i < list.count; ++i, ++info){
        String8 file_name = (**info).file_name;
        if (HasFlag((**info).attributes.flags, FileAttribute_IsDirectory)){
            if ((flags & PrjOpenFileFlag_Recursive) == 0){
                continue;
            }
            if (prj_match_in_pattern_list(file_name, blacklist)){
                continue;
            }
            String8 new_path = push_u8_stringf(scratch, "%.*s%.*s/", string_expand(path), string_expand(file_name));
            prj_open_files_pattern_filter__rec(app, new_path, whitelist, blacklist, flags);
        }
        else{
            if (!prj_match_in_pattern_list(file_name, whitelist)){
                continue;
            }
            if (prj_match_in_pattern_list(file_name, blacklist)){
                continue;
            }
            String8 full_path = push_u8_stringf(scratch, "%.*s%.*s", string_expand(path), string_expand(file_name));
            create_buffer(app, full_path, 0);
        }
    }
}

function void
prj_open_files_pattern_filter(Application_Links *app, String8 dir, Prj_Pattern_List whitelist, Prj_Pattern_List blacklist, Prj_Open_File_Flags flags){
    ProfileScope(app, "open all files in directory pattern");
    Scratch_Block scratch(app);
    String8 directory = dir;
    if (!character_is_slash(string_get_character(directory, directory.size - 1))){
        directory = push_u8_stringf(scratch, "%.*s/", string_expand(dir));
    }
    prj_open_files_pattern_filter__rec(app, directory, whitelist, blacklist, flags);
}

function void
prj_open_all_files_with_ext_in_hot(Application_Links *app, String8Array array, Prj_Open_File_Flags flags){
    Scratch_Block scratch(app);
    String8 hot = push_hot_directory(app, scratch);
    String8 directory = hot;
    if (!character_is_slash(string_get_character(hot, hot.size - 1))){
        directory = push_u8_stringf(scratch, "%.*s/", string_expand(hot));
    }
    Prj_Pattern_List whitelist = prj_pattern_list_from_extension_array(scratch, array);
    Prj_Pattern_List blacklist = prj_get_standard_blacklist(scratch);
    prj_open_files_pattern_filter(app, hot, whitelist, blacklist, flags);
}

////////////////////////////////
// NOTE(allen): Project Parse

function void
prj_parse_pattern_list(Arena *arena, Config *parsed, char *root_variable_name, Prj_Pattern_List *list_out){
    Config_Compound *compound = 0;
    if (config_compound_var(parsed, root_variable_name, 0, &compound)){
        Config_Get_Result_List list = typed_string_array_reference_list(arena, parsed, compound);
        for (Config_Get_Result_Node *cfg_node = list.first;
             cfg_node != 0;
             cfg_node = cfg_node->next){
            Prj_Pattern_Node *node = push_array(arena, Prj_Pattern_Node, 1);
            sll_queue_push(list_out->first, list_out->last, node);
            list_out->count += 1;
            String8 str = push_string_copy(arena, cfg_node->result.string);
            node->pattern.absolutes = string_split_wildcards(arena, str);
        }
    }
}

function Prj_OS_Match_Level
prj_parse_v1_os_match(String8 str, String8 this_os_str){
    Prj_OS_Match_Level result = PrjOSMatchLevel_NoMatch;
    if (string_match(str, this_os_str)){
        result = PrjOSMatchLevel_ActiveMatch;
    }
    else if (string_match(str, string_u8_litexpr("all"))){
        result = PrjOSMatchLevel_ActiveMatch;
    }
    else if (string_match(str, string_u8_litexpr("default"))){
        result = PrjOSMatchLevel_PassiveMatch;
    }
    return(result);
}

function Prj*
prj_parse_from_v1_config_data(Application_Links *app, Arena *arena, String8 root_dir, Config *parsed){
    Prj *project = push_array_zero(arena, Prj, 1);
    
    // Set new project directory
    project->dir = push_string_copy(arena, root_dir);
    
    // project_name
    {
        String8 str = {};
        if (config_string_var(parsed, "project_name", 0, &str)){
            project->name = push_string_copy(arena, str);
        }
        else{
            project->name = SCu8("");
        }
    }
    
    // patterns
    prj_parse_pattern_list(arena, parsed, "patterns", &project->pattern_list);
    
    // blacklist_patterns
    prj_parse_pattern_list(arena, parsed, "blacklist_patterns", &project->blacklist_pattern_list);
    
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
                    String8 str = {};
                    if (config_compound_string_member(parsed, paths_option, "os", 1, &str)){
                        Prj_OS_Match_Level r = prj_parse_v1_os_match(str, string_u8_litexpr(OS_NAME));
                        if (r == PrjOSMatchLevel_ActiveMatch){
                            found_match = true;
                            best_paths = paths;
                            break;
                        }
                        else if (r == PrjOSMatchLevel_PassiveMatch){
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
                
                project->load_path_array.paths = push_array(arena, Prj_File_Load_Path, list.count);
                project->load_path_array.count = list.count;
                
                Prj_File_Load_Path *dst = project->load_path_array.paths;
                for (Config_Get_Result_Node *node = list.first;
                     node != 0;
                     node = node->next, ++dst){
                    Config_Compound *src = node->result.compound;
                    block_zero_struct(dst);
                    dst->recursive = true;
                    dst->relative = true;
                    
                    String8 str = {};
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
            
            project->command_array.commands = push_array(arena, Prj_Command, list.count);
            project->command_array.count = list.count;
            
            Prj_Command *dst = project->command_array.commands;
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
                String8 cmd_str = {};
                String8 out = {};
                b32 footer_panel = false;
                b32 save_dirty_files = true;
                b32 cursor_at_end = false;
                String8 name = {};
                
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
                    
                    String8 cmd = {};
                    if (config_compound_string_member(parsed, cmd_option, "cmd", 0, &cmd)){
                        String8 str = {};
                        if (config_compound_string_member(parsed, cmd_option, "os", 1, &str)){
                            Prj_OS_Match_Level r = prj_parse_v1_os_match(str, string_u8_litexpr(OS_NAME));
                            if (r == PrjOSMatchLevel_ActiveMatch){
                                can_emit_command = true;
                                cmd_str = cmd;
                                break;
                            }
                            else if (r == PrjOSMatchLevel_PassiveMatch){
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
            String8 name = {};
            i32 index = -1;
            if (config_string_var(parsed, "fkey_command", i, &name)){
                i32 count = project->command_array.count;
                Prj_Command *command_ptr = project->command_array.commands;
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
project_deep_copy__pattern_list(Arena *arena, Prj_Pattern_List *src_list, Prj_Pattern_List *dst_list){
    for (Prj_Pattern_Node *src_node = src_list->first;
         src_node != 0;
         src_node = src_node->next){
        Prj_Pattern_Node *dst_node = push_array(arena, Prj_Pattern_Node, 1);
        sll_queue_push(dst_list->first, dst_list->last, dst_node);
        dst_list->count += 1;
        
        for (String8Node *node = src_node->pattern.absolutes.first;
             node != 0;
             node = node->next){
            String8 string = push_string_copy(arena, node->string);
            string_list_push(arena, &dst_node->pattern.absolutes, string);
        }
    }
}

function Prj
project_deep_copy__inner(Arena *arena, Prj *project){
    Prj result = {};
    result.dir = push_string_copy(arena, project->dir);
    result.name = push_string_copy(arena, project->name);
    project_deep_copy__pattern_list(arena, &project->pattern_list, &result.pattern_list);
    project_deep_copy__pattern_list(arena, &project->blacklist_pattern_list, &result.blacklist_pattern_list);
    
    {
        i32 path_count = project->load_path_array.count;
        result.load_path_array.paths = push_array(arena, Prj_File_Load_Path, path_count);
        result.load_path_array.count = path_count;
        
        Prj_File_Load_Path *dst = result.load_path_array.paths;
        Prj_File_Load_Path *src = project->load_path_array.paths;
        for (i32 i = 0; i < path_count; ++i, ++dst, ++src){
            dst->path = push_string_copy(arena, src->path);
            dst->recursive = src->recursive;
            dst->relative = src->relative;
        }
    }
    
    {
        i32 command_count = project->command_array.count;
        result.command_array.commands = push_array_zero(arena, Prj_Command, command_count);
        result.command_array.count = command_count;
        
        Prj_Command *dst = result.command_array.commands;
        Prj_Command *src = project->command_array.commands;
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

function Prj
project_deep_copy(Arena *arena, Prj *project){
    Temp_Memory restore_point = begin_temp(arena);
    Prj result = project_deep_copy__inner(arena, project);
    if (!result.loaded){
        end_temp(restore_point);
        block_zero_struct(&result);
    }
    return(result);
}

// NOTE(allen): string list join ("flatten") doesn't really work... :(
function String8
prj_join_pattern_string(Arena *arena, String8List list){
    String8 pattern_string = {};
    pattern_string.size = list.total_size + list.node_count - 1;
    pattern_string.str = push_array(arena, u8, pattern_string.size + 1);
    u8 *ptr = pattern_string.str;
    String8Node *node = list.first;
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

function String8
prj_sanitize_string(Arena *arena, String8 string){
    String8 result = {};
    
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
prj_version_1_to_version_2(Application_Links *app, Config *parsed, Prj *project){
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
    String_ID commands_id = vars_save_string(str8_lit("commands"));
    
    String_ID out_id = vars_save_string(str8_lit("out"));
    String_ID footer_panel_id = vars_save_string(str8_lit("footer_panel"));
    String_ID save_dirty_files_id = vars_save_string(str8_lit("save_dirty_files"));
    String_ID cursor_at_end_id = vars_save_string(str8_lit("cursor_at_end"));
    
    String_ID fkey_command_id = vars_save_string(str8_lit("fkey_command"));
    
    String_ID os_id = vars_save_string(str8_lit(OS_NAME));;
    
    Variable_Handle proj_var = vars_new_variable(vars_get_root(), project_id, vars_save_string(parsed->file_name));
    
    if (parsed->version != 0){
        String8 version_str = push_stringf(scratch, "%d", *parsed->version);
        vars_new_variable(proj_var, version_id, vars_save_string(version_str));
    }
    
    vars_new_variable(proj_var, project_name_id, vars_save_string(project->name));
    
    // NOTE(allen): Load Pattern
    struct PatternVars{
        String_ID id;
        Prj_Pattern_List list;
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
        for (Prj_Pattern_Node *node = pattern_var->list.first;
             node != 0;
             node = node->next, i += 1){
            String8 pattern_string = prj_join_pattern_string(scratch, node->pattern.absolutes);
            String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
            String_ID pattern_id = vars_save_string(pattern_string);
            vars_new_variable(patterns, key, pattern_id);
        }
    }
    
    // NOTE(allen): Load Paths
    {
        Variable_Handle load_paths = vars_new_variable(proj_var, load_paths_id);
        Variable_Handle os_var = vars_new_variable(load_paths, os_id);
        i32 count = project->load_path_array.count;
        Prj_File_Load_Path *load_path = project->load_path_array.paths;
        for (i32 i = 0; i < count; i += 1, load_path += 1){
            String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
            Variable_Handle path_var = vars_new_variable(os_var, key);
            vars_new_variable(path_var, path_id, vars_save_string(load_path->path));
            vars_new_variable(path_var, recursive_id, load_path->recursive?true_id:false_id);
            vars_new_variable(path_var, relative_id, load_path->recursive?true_id:false_id);
        }
    }
    
    // NOTE(allen): Commands
    {
        Variable_Handle cmd_list_var = vars_new_variable(proj_var, commands_id);
        i32 count = project->command_array.count;
        Prj_Command *cmd = project->command_array.commands;
        for (i32 i = 0; i < count; i += 1, cmd += 1){
            String8 cmd_name = prj_sanitize_string(scratch, cmd->name);
            Variable_Handle cmd_var = vars_new_variable(cmd_list_var, vars_save_string(cmd_name));
            vars_new_variable(cmd_var, os_id, vars_save_string(cmd->cmd));
            vars_new_variable(cmd_var, out_id, vars_save_string(cmd->out));
            vars_new_variable(cmd_var, footer_panel_id, cmd->footer_panel?true_id:false_id);
            vars_new_variable(cmd_var, save_dirty_files_id, cmd->save_dirty_files?true_id:false_id);
            vars_new_variable(cmd_var, cursor_at_end_id, cmd->cursor_at_end?true_id:false_id);
        }
    }
    
    // NOTE(allen): FKey Commands
    {
        Variable_Handle fkeys_var = vars_new_variable(proj_var, fkey_command_id);
        for (i32 i = 0; i < 16; i += 1){
            i32 cmd_index = project->fkey_commands[i];
            if (0 <= cmd_index && cmd_index < project->command_array.count){
                Prj_Command *cmd = project->command_array.commands + cmd_index;
                if (cmd->name.size > 0){
                    String8 cmd_name = prj_sanitize_string(scratch, cmd->name);
                    String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
                    String_ID val = vars_save_string(cmd_name);
                    vars_new_variable(fkeys_var, key, val);
                }
            }
        }
    }
    
    return(proj_var);
}

////////////////////////////////
// NOTE(allen): Project Files

function Prj_Setup_Status
prj_file_is_setup(Application_Links *app, String8 script_path, String8 script_file){
    Prj_Setup_Status result = {};
    {
        Scratch_Block scratch(app);
        String8 bat_path = push_u8_stringf(scratch, "%.*s/%.*s.bat",
                                           string_expand(script_path),
                                           string_expand(script_file));
        result.bat_exists = file_exists(app, bat_path);
    }
    {
        Scratch_Block scratch(app);
        String8 sh_path = push_u8_stringf(scratch, "%.*s/%.*s.sh",
                                          string_expand(script_path),
                                          string_expand(script_file));
        result.sh_exists = file_exists(app, sh_path);
    }
    {
        Scratch_Block scratch(app);
        String8 project_path = push_u8_stringf(scratch, "%.*s/project.4coder",
                                               string_expand(script_path));
        result.sh_exists = file_exists(app, project_path);
    }
    result.everything_exists = (result.bat_exists && result.sh_exists && result.project_exists);
    return(result);
}

function b32
prj_generate_bat(Arena *scratch, String8 opts, String8 compiler, String8 script_path, String8 script_file,
                 String8 code_file, String8 output_dir, String8 binary_file){
    b32 success = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    String8 cf = push_string_copy(scratch, code_file);
    String8 od = push_string_copy(scratch, output_dir);
    String8 bf = push_string_copy(scratch, binary_file);
    
    cf = string_mod_replace_character(cf, '/', '\\');
    od = string_mod_replace_character(od, '/', '\\');
    bf = string_mod_replace_character(bf, '/', '\\');
    
    String8 file_name = push_u8_stringf(scratch, "%.*s/%.*s.bat",
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
prj_generate_sh(Arena *scratch, String8 opts, String8 compiler, String8 script_path, String8 script_file, String8 code_file, String8 output_dir, String8 binary_file){
    b32 success = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    String8 cf = code_file;
    String8 od = output_dir;
    String8 bf = binary_file;
    
    String8 file_name = push_u8_stringf(scratch, "%.*s/%.*s.sh",
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
prj_generate_project(Arena *scratch, String8 script_path, String8 script_file, String8 output_dir, String8 binary_file){
    b32 success = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    String8 od = output_dir;
    String8 bf = binary_file;
    
    String8 od_win = string_replace(scratch, od,
                                    string_u8_litexpr("/"), string_u8_litexpr("\\"));
    String8 bf_win = string_replace(scratch, bf,
                                    string_u8_litexpr("/"), string_u8_litexpr("\\"));
    
    String8 file_name = push_u8_stringf(scratch, "%.*s/project.4coder", string_expand(script_path));
    
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
prj_setup_scripts(Application_Links *app, Prj_Setup_Script_Flags flags){
    Scratch_Block scratch(app);
    String8 script_path = push_hot_directory(app, scratch);
    
    b32 do_project_file = (flags & PrjSetupScriptFlag_Project);
    b32 do_bat_script   = (flags & PrjSetupScriptFlag_Bat);
    b32 do_sh_script    = (flags & PrjSetupScriptFlag_Sh);
    
    b32 needs_to_do_work = false;
    Prj_Setup_Status status = {};
    if (do_project_file){
        status = prj_file_is_setup(app, script_path, string_u8_litexpr("build"));
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
        
        b32 finished_queries = false;
        local_const i32 text_field_cap = 1024;
        
        String8 script_file = {};
        String8 code_file = {};
        String8 output_dir = {};
        String8 binary_file = {};
        
        {
            Query_Bar_Group bar_group(app);
            
            Query_Bar script_file_bar = {};
            Query_Bar code_file_bar = {};
            Query_Bar output_dir_bar = {};
            Query_Bar binary_file_bar = {};
            
            b32 get_script_file = !do_project_file;
            b32 get_code_file = ((do_bat_script && !status.bat_exists) || (do_sh_script && !status.sh_exists));
            
            if (get_script_file){
                script_file_bar.prompt = string_u8_litexpr("Script Name: ");
                script_file_bar.string.str = push_array(scratch, u8, text_field_cap);
                script_file_bar.string_capacity = text_field_cap;
                if (!query_user_string(app, &script_file_bar) ||
                    script_file_bar.string.size == 0){
                    goto fail_out;
                }
            }
            
            if (get_code_file){
                code_file_bar.prompt = string_u8_litexpr("Build Target: ");
                code_file_bar.string.str = push_array(scratch, u8, text_field_cap);
                code_file_bar.string_capacity = text_field_cap;
                if (!query_user_string(app, &code_file_bar) ||
                    code_file_bar.string.size == 0){
                    goto fail_out;
                }
            }
            
            output_dir_bar.prompt = string_u8_litexpr("Output Directory: ");
            output_dir_bar.string.str = push_array(scratch, u8, text_field_cap);
            output_dir_bar.string_capacity = text_field_cap;
            if (!query_user_string(app, &output_dir_bar)){
                goto fail_out;
            }
            if (output_dir_bar.string.size == 0){
                output_dir_bar.string.str[0] = '.';
                output_dir_bar.string.size = 1;
            }
            
            binary_file_bar.prompt = string_u8_litexpr("Binary Output: ");
            binary_file_bar.string.str = push_array(scratch, u8, text_field_cap);
            binary_file_bar.string_capacity = text_field_cap;
            if (!query_user_string(app, &binary_file_bar) ||
                binary_file_bar.string.size == 0){
                goto fail_out;
            }
            
            finished_queries = true;
            script_file = script_file_bar.string;
            code_file   = code_file_bar.string;
            output_dir  = output_dir_bar.string;
            binary_file = binary_file_bar.string;
            
            fail_out:;
        }
        
        if (!finished_queries){
            return;
        }
        
        if (do_project_file){
            script_file = string_u8_litexpr("build");
        }
        
        if (!do_project_file){
            status = prj_file_is_setup(app, script_path, script_file);
        }
        
        // Generate Scripts
        if (do_bat_script){
            if (!status.bat_exists){
                String8 default_flags_bat = def_get_config_string(scratch, vars_save_string_lit("default_flags_bat"));
                String8 default_compiler_bat = def_get_config_string(scratch, vars_save_string_lit("default_compiler_bat"));
                
                if (!prj_generate_bat(scratch, default_flags_bat, default_compiler_bat, script_path,
                                      script_file, code_file, output_dir, binary_file)){
                    print_message(app, string_u8_litexpr("could not create build.bat for new project\n"));
                }
            }
            else{
                print_message(app, string_u8_litexpr("the batch script already exists, no changes made to it\n"));
            }
        }
        
        if (do_sh_script){
            if (!status.bat_exists){
                String8 default_flags_sh = def_get_config_string(scratch, vars_save_string_lit("default_flags_sh"));
                String8 default_compiler_sh = def_get_config_string(scratch, vars_save_string_lit("default_compiler_sh"));
                if (!prj_generate_sh(scratch, default_flags_sh, default_compiler_sh,
                                     script_path, script_file, code_file, output_dir, binary_file)){
                    print_message(app, string_u8_litexpr("could not create build.sh for new project\n"));
                }
            }
            else{
                print_message(app, string_u8_litexpr("the shell script already exists, no changes made to it\n"));
            }
        }
        
        if (do_project_file){
            if (!status.project_exists){
                if (!prj_generate_project(scratch, script_path, script_file, output_dir, binary_file)){
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

////////////////////////////////
// NOTE(allen): Project Operations

function void
prj_exec_command(Application_Links *app, Variable_Handle cmd_var){
    Scratch_Block scratch(app);
    
    String_ID os_id = vars_save_string_lit(OS_NAME);
    
    String8 cmd = vars_string_from_var(scratch, vars_read_key(cmd_var, os_id));
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
        String8 out = vars_string_from_var(scratch, vars_read_key(cmd_var, out_id));
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
        
        Variable_Handle command_list_var = vars_parent(cmd_var);
        Variable_Handle prj_var = vars_parent(command_list_var);
        String8 prj_dir = prj_path_from_project(scratch, prj_var);
        exec_system_command(app, view, buffer_id, prj_dir, cmd, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
}

function Variable_Handle
prj_command_from_name(Application_Links *app, String8 cmd_name){
    Scratch_Block scratch(app);
    // TODO(allen): fallback for multiple stages of reading
    Variable_Handle cmd_list = def_get_config_var(vars_save_string_lit("commands"));
    Variable_Handle cmd = vars_read_key(cmd_list, vars_save_string(cmd_name));
    return(cmd);
}

function void
prj_exec_command_name(Application_Links *app, String8 cmd_name){
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
    String8 fkey_index_str = push_stringf(scratch, "%d", fkey_index);
    String_ID fkey_index_id = vars_save_string(fkey_index_str);
    Variable_Handle cmd_name_var = vars_read_key(fkeys, fkey_index_id);
    String8 cmd_name = vars_string_from_var(scratch, cmd_name_var);
    prj_exec_command_name(app, cmd_name);
}

function String8
prj_path_from_project(Arena *arena, Variable_Handle project){
    String8 project_full_path = vars_string_from_var(arena, project);
    String8 project_dir = string_remove_last_folder(project_full_path);
    return(project_dir);
}

function Variable_Handle
prj_cmd_from_user(Application_Links *app, Variable_Handle prj_var, String8 query){
    Scratch_Block scratch(app);
    Lister_Block lister(app, scratch);
    lister_set_query(lister, query);
    lister_set_default_handlers(lister);
    
    Variable_Handle cmd_list_var = vars_read_key(prj_var, vars_save_string_lit("commands"));
    String_ID os_id = vars_save_string_lit(OS_NAME);
    
    for (Variable_Handle cmd = vars_first_child(cmd_list_var);
         !vars_is_nil(cmd);
         cmd = vars_next_sibling(cmd)){
        Variable_Handle os_cmd = vars_read_key(cmd, os_id);
        if (!vars_is_nil(os_cmd)){
            String8 cmd_name = vars_key_from_var(scratch, cmd);
            String8 os_cmd_str = vars_string_from_var(scratch, os_cmd);
            lister_add_item(lister, cmd_name, os_cmd_str, cmd.ptr, 0);
        }
    }
    
    Variable_Handle result = vars_get_nil();
    Lister_Result l_result = run_lister(app, lister);
    if (!l_result.canceled){
        if (l_result.user_data != 0){
            result.ptr = (Variable*)l_result.user_data;
        }
    }
    
    return(result);
}

////////////////////////////////
// NOTE(allen): Commands

CUSTOM_COMMAND_SIG(close_all_code)
CUSTOM_DOC("Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.")
{
    Scratch_Block scratch(app);
    String8 treat_as_code = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
    String8Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code);
    prj_close_files_with_ext(app, extensions);
}

CUSTOM_COMMAND_SIG(open_all_code)
CUSTOM_DOC("Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.")
{
    Scratch_Block scratch(app);
    String8 treat_as_code = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
    String8Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code);
    prj_open_all_files_with_ext_in_hot(app, extensions, 0);
}

CUSTOM_COMMAND_SIG(open_all_code_recursive)
CUSTOM_DOC("Works as open_all_code but also runs in all subdirectories.")
{
    Scratch_Block scratch(app);
    String8 treat_as_code = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
    String8Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code);
    prj_open_all_files_with_ext_in_hot(app, extensions, PrjOpenFileFlag_Recursive);
}

CUSTOM_COMMAND_SIG(load_project)
CUSTOM_DOC("Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.")
{
    ProfileScope(app, "load project");
    save_all_dirty_buffers(app);
    Scratch_Block scratch(app);
    
    // NOTE(allen): Load the project file from the hot directory
    String8 project_path = push_hot_directory(app, scratch);
    File_Name_Data dump = dump_file_search_up_path(app, scratch, project_path, string_u8_litexpr("project.4coder"));
    String8 project_root = string_remove_last_folder(dump.file_name);
    
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
                        Prj *project = prj_parse_from_v1_config_data(app, scratch, project_root, config_parse);
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
        String8 error_text = config_stringize_errors(app, scratch, config_parse);
        if (error_text.size > 0){
            print_message(app, string_u8_litexpr("Project errors:\n"));
            print_message(app, error_text);
            print_message(app, string_u8_litexpr("\n"));
        }
    }
    
    // NOTE(allen): Open All Project Files
    Variable_Handle load_paths_var = vars_read_key(proj_var, vars_save_string_lit("load_paths"));
    Variable_Handle load_paths_os_var = vars_read_key(load_paths_var, vars_save_string_lit(OS_NAME));
    
    String_ID path_id = vars_save_string_lit("path");
    String_ID recursive_id = vars_save_string_lit("recursive");
    String_ID relative_id = vars_save_string_lit("relative");
    
    Variable_Handle whitelist_var = vars_read_key(proj_var, vars_save_string_lit("patterns"));
    Variable_Handle blacklist_var = vars_read_key(proj_var, vars_save_string_lit("blacklist_patterns"));
    
    Prj_Pattern_List whitelist = prj_pattern_list_from_var(scratch, whitelist_var);
    Prj_Pattern_List blacklist = prj_pattern_list_from_var(scratch, blacklist_var);
    
    for (Variable_Handle load_path_var = vars_first_child(load_paths_os_var);
         !vars_is_nil(load_path_var);
         load_path_var = vars_next_sibling(load_path_var)){
        Variable_Handle path_var = vars_read_key(load_path_var, path_id);
        Variable_Handle recursive_var = vars_read_key(load_path_var, recursive_id);
        Variable_Handle relative_var = vars_read_key(load_path_var, relative_id);
        
        String8 path = vars_string_from_var(scratch, path_var);
        b32 recursive = vars_b32_from_var(recursive_var);
        b32 relative = vars_b32_from_var(relative_var);
        
        
        u32 flags = 0;
        if (recursive){
            flags |= PrjOpenFileFlag_Recursive;
        }
        
        String8 file_dir = path;
        if (relative){
            String8 prj_dir = prj_path_from_project(scratch, proj_var);
            
            String8List file_dir_list = {};
            string_list_push(scratch, &file_dir_list, prj_dir);
            string_list_push_overlap(scratch, &file_dir_list, '/', path);
            string_list_push_overlap(scratch, &file_dir_list, '/', SCu8());
            file_dir = string_list_flatten(scratch, file_dir_list, StringFill_NullTerminate);
        }
        
        prj_open_files_pattern_filter(app, file_dir, whitelist, blacklist, flags);
    }
    
    // NOTE(allen): Set Window Title
    Variable_Handle proj_name_var = vars_read_key(proj_var, vars_save_string_lit("project_name"));
    String_ID proj_name_id = vars_key_id_from_var(proj_var);
    if (proj_name_id != 0){
        String8 proj_name = vars_read_string(scratch, proj_name_id);
        String8 title = push_u8_stringf(scratch, "4coder project: %.*s", string_expand(proj_name));
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
    Scratch_Block scratch(app);
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    String8 prj_dir = prj_path_from_project(scratch, prj_var);
    if (prj_dir.size > 0){
        set_hot_directory(app, prj_dir);
    }
}

CUSTOM_COMMAND_SIG(setup_new_project)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new 4coder project with build scripts for every OS.")
{
    prj_setup_scripts(app, PrjSetupScriptFlag_Project|PrjSetupScriptFlag_Bat|PrjSetupScriptFlag_Sh);
    load_project(app);
}

CUSTOM_COMMAND_SIG(setup_build_bat)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build batch script.")
{
    prj_setup_scripts(app, PrjSetupScriptFlag_Bat);
}

CUSTOM_COMMAND_SIG(setup_build_sh)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build shell script.")
{
    prj_setup_scripts(app, PrjSetupScriptFlag_Sh);
}

CUSTOM_COMMAND_SIG(setup_build_bat_and_sh)
CUSTOM_DOC("Queries the user for several configuration options and initializes a new build batch script.")
{
    prj_setup_scripts(app, PrjSetupScriptFlag_Bat|PrjSetupScriptFlag_Sh);
}

CUSTOM_COMMAND_SIG(project_command_lister)
CUSTOM_DOC("Open a lister of all commands in the currently loaded project.")
{
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    Variable_Handle prj_cmd = prj_cmd_from_user(app, prj_var, string_u8_litexpr("Command:"));
    if (!vars_is_nil(prj_cmd)){
        prj_exec_command(app, prj_cmd);
    }
}

// BOTTOM

