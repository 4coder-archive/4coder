/*
4coder_prj_v1.cpp - parsing and converting v0 and v1 projects
*/

// TOP

////////////////////////////////
// NOTE(allen): Project v0-v1 Functions

function void
prj_v1_parse_pattern_list(Arena *arena, Config *parsed, char *name, Prj_Pattern_List *list_out){
    Config_Compound *compound = 0;
    if (config_compound_var(parsed, name, 0, &compound)){
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

function Prj_V1_OS_Match_Level
prj_v1_parse_os_match(String8 str, String8 this_os_str){
    Prj_V1_OS_Match_Level result = PrjV1OSMatchLevel_NoMatch;
    if (string_match(str, this_os_str)){
        result = PrjV1OSMatchLevel_ActiveMatch;
    }
    else if (string_match(str, string_u8_litexpr("all"))){
        result = PrjV1OSMatchLevel_ActiveMatch;
    }
    else if (string_match(str, string_u8_litexpr("default"))){
        result = PrjV1OSMatchLevel_PassiveMatch;
    }
    return(result);
}

function Prj_V1*
prj_v1_parse_from_config(Application_Links *app, Arena *arena, String8 dir, Config *parsed){
    Prj_V1 *project = push_array_zero(arena, Prj_V1, 1);
    
    // Set new project directory
    project->dir = push_string_copy(arena, dir);
    
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
    prj_v1_parse_pattern_list(arena, parsed, "patterns", &project->pattern_list);
    
    // blacklist_patterns
    prj_v1_parse_pattern_list(arena, parsed, "blacklist_patterns", &project->blacklist_pattern_list);
    
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
                        Prj_V1_OS_Match_Level r = prj_v1_parse_os_match(str, string_u8_litexpr(OS_NAME));
                        if (r == PrjV1OSMatchLevel_ActiveMatch){
                            found_match = true;
                            best_paths = paths;
                            break;
                        }
                        else if (r == PrjV1OSMatchLevel_PassiveMatch){
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
                
                project->load_path_array.paths = push_array(arena, Prj_V1_File_Load_Path, list.count);
                project->load_path_array.count = list.count;
                
                Prj_V1_File_Load_Path *dst = project->load_path_array.paths;
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
            
            project->command_array.commands = push_array(arena, Prj_V1_Command, list.count);
            project->command_array.count = list.count;
            
            Prj_V1_Command *dst = project->command_array.commands;
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
                            Prj_V1_OS_Match_Level r = prj_v1_parse_os_match(str, string_u8_litexpr(OS_NAME));
                            if (r == PrjV1OSMatchLevel_ActiveMatch){
                                can_emit_command = true;
                                cmd_str = cmd;
                                break;
                            }
                            else if (r == PrjV1OSMatchLevel_PassiveMatch){
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
                Prj_V1_Command *command_ptr = project->command_array.commands;
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

// NOTE(allen): string list join ("flatten") doesn't really work... :(
function String8
prj_v1_join_pattern_string(Arena *arena, String8List list){
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
prj_v1_sanitize_string(Arena *arena, String8 string){
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
prj_v1_to_v2(Application_Links *app, String8 dir, Config *parsed){
    Scratch_Block scratch(app);
    
    Prj_V1 *project = prj_v1_parse_from_config(app, scratch, dir, parsed);
    
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
            String8 pattern_string = prj_v1_join_pattern_string(scratch, node->pattern.absolutes);
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
        Prj_V1_File_Load_Path *load_path = project->load_path_array.paths;
        for (i32 i = 0; i < count; i += 1, load_path += 1){
            String_ID key = vars_save_string(push_stringf(scratch, "%d", i));
            Variable_Handle path_var = vars_new_variable(os_var, key);
            vars_new_variable(path_var, path_id, vars_save_string(load_path->path));
            vars_new_variable(path_var, recursive_id, load_path->recursive?true_id:false_id);
            vars_new_variable(path_var, relative_id, load_path->relative?true_id:false_id);
        }
    }
    
    // NOTE(allen): Commands
    {
        Variable_Handle cmd_list_var = vars_new_variable(proj_var, commands_id);
        i32 count = project->command_array.count;
        Prj_V1_Command *cmd = project->command_array.commands;
        for (i32 i = 0; i < count; i += 1, cmd += 1){
            String8 cmd_name = prj_v1_sanitize_string(scratch, cmd->name);
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
                Prj_V1_Command *cmd = project->command_array.commands + cmd_index;
                if (cmd->name.size > 0){
                    String8 cmd_name = prj_v1_sanitize_string(scratch, cmd->name);
                    String_ID key = vars_save_string(push_stringf(scratch, "F%d", i + 1));
                    String_ID val = vars_save_string(cmd_name);
                    vars_new_variable(fkeys_var, key, val);
                }
            }
        }
    }
    
    return(proj_var);
}

// BOTTOM
