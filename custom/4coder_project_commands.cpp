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
// NOTE(allen): Project Files

function void
prj_stringize__string_list(Application_Links *app, Arena *arena, String8 name, Variable_Handle list, String8List *out){
    Scratch_Block scratch(app, arena);
    string_list_pushf(arena, out, "%.*s = {\n", string_expand(name));
    for (Vars_Children(child, list)){
        String8 child_string = vars_string_from_var(scratch, child);
        if (child_string.size > 0){
            // TODO(allen): escape child_string
            string_list_pushf(arena, out, "\"%.*s\",\n", string_expand(child_string));
        }
    }
    string_list_pushf(arena, out, "};\n");
}

function void
prj_stringize_project(Application_Links *app, Arena *arena, Variable_Handle project, String8List *out){
    Scratch_Block scratch(app, arena);
    
    // NOTE(allen): String IDs
    String_ID version_id = vars_save_string_lit("version");
    String_ID project_name_id = vars_save_string_lit("project_name");
    String_ID patterns_id = vars_save_string_lit("patterns");
    String_ID blacklist_patterns_id = vars_save_string_lit("blacklist_patterns");
    
    String_ID load_paths_id = vars_save_string_lit("load_paths");
    String_ID path_id = vars_save_string_lit("path");
    String_ID relative_id = vars_save_string_lit("relative");
    String_ID recursive_id = vars_save_string_lit("recursive");
    
    String_ID commands_id = vars_save_string_lit("commands");
    String_ID out_id = vars_save_string_lit("out");
    String_ID footer_panel_id = vars_save_string_lit("footer_panel");
    String_ID save_dirty_files_id = vars_save_string_lit("save_dirty_files");
    String_ID cursor_at_end_id = vars_save_string_lit("cursor_at_end");
    
    String_ID fkey_command_id = vars_save_string_lit("fkey_command");
    String_ID fkey_command_override_id = vars_save_string_lit("fkey_command_override");
    
    String8 os_strings[] = { str8_lit("win"), str8_lit("linux"), str8_lit("mac"), };
    local_const i32 os_string_count = ArrayCount(os_strings);
    String_ID os_string_ids[os_string_count];
    for (i32 i = 0; i < os_string_count; i += 1){
        os_string_ids[i] = vars_save_string(os_strings[i]);
    }
    
    
    // NOTE(allen): Stringizing...
    
    // NOTE(allen): Header Stuff
    u64 version = vars_u64_from_var(app, vars_read_key(project, version_id));
    version = clamp_bot(2, version);
    string_list_pushf(arena, out, "version(%llu);\n", version);
    
    String8 project_name = vars_string_from_var(scratch, vars_read_key(project, project_name_id));
    if (project_name.size > 0){
        // TODO(allen): escape project_name
        string_list_pushf(arena, out, "project_name = \"%.*s\";\n", string_expand(project_name));
    }
    
    string_list_push(arena, out, str8_lit("\n"));
    
    
    // NOTE(allen): File Match Patterns
    Variable_Handle patterns = vars_read_key(project, patterns_id);
    if (!vars_is_nil(patterns)){
        prj_stringize__string_list(app, arena, str8_lit("patterns"), patterns, out);
    }
    
    Variable_Handle blacklist_patterns = vars_read_key(project, blacklist_patterns_id);
    if (!vars_is_nil(blacklist_patterns)){
        prj_stringize__string_list(app, arena, str8_lit("blacklist_patterns"), blacklist_patterns, out);
    }
    
    string_list_push(arena, out, str8_lit("\n"));
    
    
    // NOTE(allen): Load Paths
    Variable_Handle load_paths = vars_read_key(project, load_paths_id);
    if (!vars_is_nil(load_paths)){
        string_list_push(arena, out, str8_lit("load_paths = {\n"));
        for (i32 i = 0; i < os_string_count; i += 1){
            Variable_Handle os_paths = vars_read_key(load_paths, os_string_ids[i]);
            if (!vars_is_nil(os_paths)){
                String8 os_string = os_strings[i];
                string_list_pushf(arena, out, ".%.*s = {\n", string_expand(os_string));
                for (Vars_Children(child, os_paths)){
                    Variable_Handle path_var = vars_read_key(child, path_id);
                    Variable_Handle recursive_var = vars_read_key(child, recursive_id);
                    Variable_Handle relative_var = vars_read_key(child, relative_id);
                    
                    // TODO(allen): escape path_string
                    String8 path_string = vars_string_from_var(scratch, path_var);
                    b32 recursive = vars_b32_from_var(recursive_var);
                    b32 relative = vars_b32_from_var(relative_var);
                    
                    string_list_push(arena, out, str8_lit("{ "));
                    string_list_pushf(arena, out, ".path = \"%.*s\", ", string_expand(path_string));
                    string_list_pushf(arena, out, ".recursive = %s, ", (recursive?"true":"false"));
                    string_list_pushf(arena, out, ".relative = %s, ", (relative?"true":"false"));
                    string_list_push(arena, out, str8_lit("},\n"));
                }
                string_list_push(arena, out, str8_lit("},\n"));
            }
        }
        string_list_push(arena, out, str8_lit("};\n\n"));
    }
    
    
    // NOTE(allen): Commands
    Variable_Handle commands = vars_read_key(project, commands_id);
    if (!vars_is_nil(commands)){
        string_list_push(arena, out, str8_lit("commands = {\n"));
        for (Vars_Children(command, commands)){
            String8 command_name = vars_key_from_var(scratch, command);
            string_list_pushf(arena, out, ".%.*s = {\n", string_expand(command_name));
            
            for (i32 i = 0; i < os_string_count; i += 1){
                Variable_Handle os_cmd_var = vars_read_key(command, os_string_ids[i]);
                if (!vars_is_nil(os_cmd_var)){
                    // TODO(allen): escape os_cmd_string
                    String8 os_cmd_string = vars_string_from_var(scratch, os_cmd_var);
                    string_list_pushf(arena, out, ".%.*s = \"%.*s\",\n", string_expand(os_strings[i]), string_expand(os_cmd_string));
                }
            }
            
            Variable_Handle out_var = vars_read_key(command, out_id);
            Variable_Handle footer_panel_var = vars_read_key(command, footer_panel_id);
            Variable_Handle save_dirty_files_var = vars_read_key(command, save_dirty_files_id);
            Variable_Handle cursor_at_end_var = vars_read_key(command, cursor_at_end_id);
            
            // TODO(allen): escape out_string
            String8 out_string = vars_string_from_var(scratch, out_var);
            b32 footer_panel = vars_b32_from_var(footer_panel_var);
            b32 save_dirty_files = vars_b32_from_var(save_dirty_files_var);
            b32 cursor_at_end = vars_b32_from_var(cursor_at_end_var);
            
            string_list_pushf(arena, out, ".out = \"%.*s\",\n", string_expand(out_string));
            string_list_pushf(arena, out, ".footer_panel = %s,\n", (footer_panel?"true":"false"));
            string_list_pushf(arena, out, ".save_dirty_files = %s,\n", (save_dirty_files?"true":"false"));
            string_list_pushf(arena, out, ".cursor_at_end = %s,\n", (cursor_at_end?"true":"false"));
            string_list_push(arena, out, str8_lit("},\n"));
        }
        string_list_push(arena, out, str8_lit("};\n\n"));
    }
    
    
    // NOTE(allen): FKey Command
    Variable_Handle fkey_commands = vars_read_key(project, fkey_command_id);
    if (!vars_is_nil(fkey_commands)){
        string_list_push(arena, out, str8_lit("fkey_command = {\n"));
        for (Vars_Children(child, fkey_commands)){
            String8 key = vars_key_from_var(scratch, child);
            String8 name = vars_string_from_var(scratch, child);
            string_list_pushf(arena, out, ".%.*s = \"%.*s\",\n",
                              string_expand(key), string_expand(name));
        }
        string_list_push(arena, out, str8_lit("};\n\n"));
    }
    
    
    // NOTE(allen): FKey Command Override
    Variable_Handle fkey_commands_overide = vars_read_key(project, fkey_command_override_id);
    if (!vars_is_nil(fkey_commands_overide)){
        string_list_push(arena, out, str8_lit("fkey_command_override = {\n"));
        for (Vars_Children(user_child, fkey_commands_overide)){
            String8 user_key = vars_key_from_var(scratch, user_child);
            string_list_pushf(arena, out, ".%.*s = {\n", string_expand(user_key));
            for (Vars_Children(child, user_child)){
                String8 key = vars_key_from_var(scratch, child);
                String8 name = vars_string_from_var(scratch, child);
                string_list_pushf(arena, out, ".%.*s = \"%.*s\",\n",
                                  string_expand(key), string_expand(name));
            }
            string_list_pushf(arena, out, "},\n");
        }
        string_list_push(arena, out, str8_lit("};\n\n"));
    }
}

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
        fprintf(out, "version(2);\n");
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
        fprintf(out, " .win = load_paths_base,\n");
        fprintf(out, " .linux = load_paths_base,\n");
        fprintf(out, " .mac = load_paths_base,\n");
        fprintf(out, "};\n");
        
        fprintf(out, "\n");
        
        fprintf(out, "commands = {\n");
        fprintf(out, " .build = { .out = \"*compilation*\", .footer_panel = true, .save_dirty_files = true,\n");
        fprintf(out, "   .win = \"%.*s.bat\",\n", string_expand(script_file));
        fprintf(out, "   .linux = \"./%.*s.sh\",\n", string_expand(script_file));
        fprintf(out, "   .mac = \"./%.*s.sh\", },\n", string_expand(script_file));
        fprintf(out, " .run = { .out = \"*run*\", .footer_panel = false, .save_dirty_files = false,\n");
        fprintf(out, "   .win = \"%.*s\\\\%.*s\",\n", string_expand(od_win), string_expand(bf_win));
        fprintf(out, "   .linux = \"%.*s/%.*s\",\n", string_expand(od), string_expand(bf));
        fprintf(out, "   .mac = \"%.*s/%.*s\", },\n", string_expand(od), string_expand(bf));
        fprintf(out, "};\n");
        
        fprintf(out, "fkey_command = {\n");
        fprintf(out, ".F1 = \"run\";\n");
        fprintf(out, ".F2 = \"run\";\n");
        fprintf(out, "};\n");
        
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
    // setup fkey string
    Scratch_Block scratch(app);
    String8 fkey_index_str = push_stringf(scratch, "F%d", fkey_index + 1);
    String_ID fkey_index_id = vars_save_string(fkey_index_str);
    
    // get command variable
    Variable_Handle cmd_name_var = vars_get_nil();
    
    // try user override
    {
        Variable_Handle fkey_override = 
            def_get_config_var(vars_save_string_lit("fkey_command_override"));
        if (!vars_is_nil(fkey_override)){
            String_Const_u8 name = def_get_config_string(scratch, vars_save_string_lit("user_name"));
            String_ID user_name_id = vars_save_string(name);
            Variable_Handle user_var = vars_read_key(fkey_override, user_name_id);
            cmd_name_var = vars_read_key(user_var, fkey_index_id);
        }
    }
    
    // try defaults
    if (vars_is_nil(cmd_name_var)){
        Variable_Handle fkeys = def_get_config_var(vars_save_string_lit("fkey_command"));
        cmd_name_var = vars_read_key(fkeys, fkey_index_id);
    }
    
    String8 cmd_name = vars_string_from_var(scratch, cmd_name_var);
    prj_exec_command_name(app, cmd_name);
}

function String8
prj_full_file_path_from_project(Arena *arena, Variable_Handle project){
    String8 project_full_path = vars_string_from_var(arena, project);
    return(project_full_path);
}

function String8
prj_path_from_project(Arena *arena, Variable_Handle project){
    String8 project_full_path = prj_full_file_path_from_project(arena, project);
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
    // TODO(allen): compress this _thoughtfully_
    
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
    Variable_Handle prj_var = vars_get_nil();
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
                        prj_var = prj_v1_to_v2(app, project_root, config_parse);
                    }break;
                    default:
                    {
                        prj_var = def_fill_var_from_config(app, vars_get_root(), vars_save_string_lit("prj_config"), config_parse);
                    }break;
                }
                
            }
        }
    }
    
    // NOTE(allen): Print Project
    if (!vars_is_nil(prj_var)){
        vars_print(app, prj_var);
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
    Variable_Handle load_paths_var = vars_read_key(prj_var, vars_save_string_lit("load_paths"));
    Variable_Handle load_paths_os_var = vars_read_key(load_paths_var, vars_save_string_lit(OS_NAME));
    
    String_ID path_id = vars_save_string_lit("path");
    String_ID recursive_id = vars_save_string_lit("recursive");
    String_ID relative_id = vars_save_string_lit("relative");
    
    Variable_Handle whitelist_var = vars_read_key(prj_var, vars_save_string_lit("patterns"));
    Variable_Handle blacklist_var = vars_read_key(prj_var, vars_save_string_lit("blacklist_patterns"));
    
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
            String8 prj_dir = prj_path_from_project(scratch, prj_var);
            
            String8List file_dir_list = {};
            string_list_push(scratch, &file_dir_list, prj_dir);
            string_list_push_overlap(scratch, &file_dir_list, '/', path);
            string_list_push_overlap(scratch, &file_dir_list, '/', SCu8());
            file_dir = string_list_flatten(scratch, file_dir_list, StringFill_NullTerminate);
        }
        
        prj_open_files_pattern_filter(app, file_dir, whitelist, blacklist, flags);
    }
    
    // NOTE(allen): Set Window Title
    Variable_Handle proj_name_var = vars_read_key(prj_var, vars_save_string_lit("project_name"));
    String_ID proj_name_id = vars_string_id_from_var(proj_name_var);
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

CUSTOM_COMMAND_SIG(project_reprint)
CUSTOM_DOC("Prints the current project to the file it was loaded from; prints in the most recent project file version")
{
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    if (!vars_is_nil(prj_var)){
        Scratch_Block scratch(app);
        String8 prj_full_path = prj_full_file_path_from_project(scratch, prj_var);
        prj_full_path = push_string_copy(scratch, prj_full_path);
        String8 message = push_stringf(scratch, "Reprinting project file: %.*s\n", string_expand(prj_full_path));
        print_message(app, message);
        
        String8List prj_string = {};
        prj_stringize_project(app, scratch, prj_var, &prj_string);
        
        FILE *file = fopen((char*)prj_full_path.str, "wb");
        if (file == 0){
            print_message(app, str8_lit("Could not open project file\n"));
        }
        else{
            for (String8Node *node = prj_string.first;
                 node != 0;
                 node = node->next){
                fwrite(node->string.str, 1, (size_t)node->string.size, file);
            }
            fclose(file);
            print_message(app, str8_lit("Reloading project buffer\n"));
            
            Buffer_ID buffer = get_buffer_by_file_name(app, prj_full_path, Access_Always);
            if (buffer != 0){
                buffer_reopen(app, buffer, 0);
            }
            else{
                create_buffer(app, prj_full_path, 0);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(project_command_F1)
CUSTOM_DOC("Run the command with index 1")
{
    prj_exec_command_fkey_index(app, 0);
}

CUSTOM_COMMAND_SIG(project_command_F2)
CUSTOM_DOC("Run the command with index 2")
{
    prj_exec_command_fkey_index(app, 1);
}

CUSTOM_COMMAND_SIG(project_command_F3)
CUSTOM_DOC("Run the command with index 3")
{
    prj_exec_command_fkey_index(app, 2);
}

CUSTOM_COMMAND_SIG(project_command_F4)
CUSTOM_DOC("Run the command with index 4")
{
    prj_exec_command_fkey_index(app, 3);
}

CUSTOM_COMMAND_SIG(project_command_F5)
CUSTOM_DOC("Run the command with index 5")
{
    prj_exec_command_fkey_index(app, 4);
}

CUSTOM_COMMAND_SIG(project_command_F6)
CUSTOM_DOC("Run the command with index 6")
{
    prj_exec_command_fkey_index(app, 5);
}

CUSTOM_COMMAND_SIG(project_command_F7)
CUSTOM_DOC("Run the command with index 7")
{
    prj_exec_command_fkey_index(app, 6);
}

CUSTOM_COMMAND_SIG(project_command_F8)
CUSTOM_DOC("Run the command with index 8")
{
    prj_exec_command_fkey_index(app, 7);
}

CUSTOM_COMMAND_SIG(project_command_F9)
CUSTOM_DOC("Run the command with index 9")
{
    prj_exec_command_fkey_index(app, 8);
}

CUSTOM_COMMAND_SIG(project_command_F10)
CUSTOM_DOC("Run the command with index 10")
{
    prj_exec_command_fkey_index(app, 9);
}

CUSTOM_COMMAND_SIG(project_command_F11)
CUSTOM_DOC("Run the command with index 11")
{
    prj_exec_command_fkey_index(app, 10);
}

CUSTOM_COMMAND_SIG(project_command_F12)
CUSTOM_DOC("Run the command with index 12")
{
    prj_exec_command_fkey_index(app, 11);
}

CUSTOM_COMMAND_SIG(project_command_F13)
CUSTOM_DOC("Run the command with index 13")
{
    prj_exec_command_fkey_index(app, 12);
}

CUSTOM_COMMAND_SIG(project_command_F14)
CUSTOM_DOC("Run the command with index 14")
{
    prj_exec_command_fkey_index(app, 13);
}

CUSTOM_COMMAND_SIG(project_command_F15)
CUSTOM_DOC("Run the command with index 15")
{
    prj_exec_command_fkey_index(app, 14);
}

CUSTOM_COMMAND_SIG(project_command_F16)
CUSTOM_DOC("Run the command with index 16")
{
    prj_exec_command_fkey_index(app, 15);
}

// BOTTOM

