/*
4coder_project_commands.cpp - commands for loading and using a project.

type: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_PROJECT_COMMANDS_CPP)
#define FCODER_PROJECT_COMMANDS_CPP

#include "4coder_project_commands.h"

#include "4coder_build_commands.cpp"

///////////////////////////////

static Project current_project = {0};

///////////////////////////////

static CString_Array
get_project_extensions(Project *project){
    CString_Array array = {0};
    array.strings = default_extensions;
    array.count = ArrayCount(default_extensions);
    if (project->loaded){
        array.strings = project->extension_list.exts;
        array.count = project->extension_list.count;
    }
    return(array);
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

static void
open_all_files_in_directory_with_extension(Application_Links *app, String dir,
                                           CString_Array extension_array,
                                           uint32_t flags){
    File_List list = get_file_list(app, dir.str, dir.size);
    int32_t dir_size = dir.size;
    
    for (uint32_t i = 0; i < list.count; ++i){
        File_Info *info = list.infos + i;
        if (info->folder){
            if (((flags&OpenAllFilesFlag_Recursive) != 0) && info->filename[0] != '.'){
                dir.size = dir_size;
                append(&dir, info->filename);
                append(&dir, "/");
                open_all_files_in_directory_with_extension(app, dir, extension_array, flags);
            }
        }
        else{
            bool32 is_match = true;
            if (extension_array.count > 0){
                is_match = false;
                String ext = make_string_cap(info->filename, info->filename_len, info->filename_len + 1);
                ext = file_extension(ext);
                for (int32_t j = 0; j < extension_array.count; ++j){
                    if (match(ext, extension_array.strings[j])){
                        is_match = true;
                        break;
                    }
                }
            }
            
            if (is_match){
                dir.size = dir_size;
                append(&dir, info->filename);
                create_buffer(app, dir.str, dir.size, 0);
            }
        }
    }
    
    free_file_list(app, list);
}

static void
open_all_files_with_extension_in_hot(Application_Links *app, Partition *scratch,
                                     CString_Array extensions_array,
                                     uint32_t flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    int32_t max_size = 4096;
    char *memory = push_array(scratch, char, max_size);
    String dir = make_string_cap(memory, 0, max_size);
    dir.size = directory_get_hot(app, dir.str, dir.memory_size);
    open_all_files_in_directory_with_extension(app, dir, extensions_array, flags);
    end_temp_memory(temp);
}

static void
open_all_code_with_project_extensions_in_directory(Application_Links *app, String dir, uint32_t flags){
    CString_Array array = get_project_extensions(&current_project);
    open_all_files_in_directory_with_extension(app, dir, array, flags);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(close_all_code)
CUSTOM_DOC("Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.")
{
    CString_Array extensions = get_project_extensions(&current_project);
    close_all_files_with_extension(app, &global_part, extensions);
}

CUSTOM_COMMAND_SIG(open_all_code)
CUSTOM_DOC("Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.")
{
    CString_Array extensions = get_project_extensions(&current_project);
    open_all_files_with_extension_in_hot(app, &global_part, extensions, 0);
}

CUSTOM_COMMAND_SIG(open_all_code_recursive)
CUSTOM_DOC("Works as open_all_code but also runs in all subdirectories.")
{
    CString_Array extensions = get_project_extensions(&current_project);
    open_all_files_with_extension_in_hot(app, &global_part, extensions, OpenAllFilesFlag_Recursive);
}

///////////////////////////////

static void
load_project_from_data(Application_Links *app, Partition *scrtach,
                       char *config_data, int32_t config_data_size,
                       String project_dir){
    Temp_Memory temp = begin_temp_memory(scrtach);
    
    char *mem = config_data;
    int32_t size = config_data_size;
    
    Cpp_Token_Array array;
    array.count = 0;
    array.max_count = (1 << 20)/sizeof(Cpp_Token);
    array.tokens = push_array(scrtach, Cpp_Token, array.max_count);
    
    Cpp_Keyword_Table kw_table = {0};
    Cpp_Keyword_Table pp_table = {0};
    lexer_keywords_default_init(scrtach, &kw_table, &pp_table);
    
    Cpp_Lex_Data S = cpp_lex_data_init(false, kw_table, pp_table);
    Cpp_Lex_Result result = cpp_lex_step(&S, mem, size + 1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
    
    if (result == LexResult_Finished){
        // Clear out current project
        if (current_project.close_all_code_when_this_project_closes){
            exec_command(app, close_all_code);
        }
        memset(&current_project, 0, sizeof(current_project));
        current_project.loaded = true;
        
        // Set new project directory
        {
            current_project.dir = current_project.dir_space;
            String str = make_fixed_width_string(current_project.dir_space);
            copy(&str, project_dir);
            terminate_with_null(&str);
            current_project.dir_len = str.size;
        }
        
        // Read the settings from project.4coder
        for (int32_t i = 0; i < array.count; ++i){
            Config_Line config_line = read_config_line(array, &i, mem);
            if (config_line.read_success){
                Config_Item item = get_config_item(config_line, mem, array);
                
                {
                    char str_space[512];
                    String str = make_fixed_width_string(str_space);
                    if (config_string_var(item, "extensions", 0, &str)){
                        if (str.size < sizeof(current_project.extension_list.space)){
                            parse_extension_line_to_extension_list(str, &current_project.extension_list);
                            print_message(app, str.str, str.size);
                            print_message(app, "\n", 1);
                        }
                        else{
                            print_message(app, literal("STRING TOO LONG!\n"));
                        }
                    }
                }
                
                {
                    bool32 open_recursively = false;
                    if (config_bool_var(item, "open_recursively", 0, &open_recursively)){
                        current_project.open_recursively = open_recursively;
                    }
                }
                
                {
#if defined(IS_WINDOWS)
# define FKEY_COMMAND "fkey_command_win"
#elif defined(IS_LINUX)
# define FKEY_COMMAND "fkey_command_linux"
#elif defined(IS_MAC)
# define FKEY_COMMAND "fkey_command_mac"
#else
# error no project configuration names for this platform
#endif
                    
                    int32_t index = 0;
                    Config_Array_Reader array_reader = {0};
                    if (config_array_var(item, FKEY_COMMAND, &index, &array_reader)){
                        if (index >= 1 && index <= 16){
                            Config_Item array_item = {0};
                            int32_t item_index = 0;
                            
                            char space[256];
                            String msg = make_fixed_width_string(space);
                            append(&msg, FKEY_COMMAND"[");
                            append_int_to_str(&msg, index);
                            append(&msg, "] = {");
                            
                            for (config_array_next_item(&array_reader, &array_item);
                                 config_array_good(&array_reader);
                                 config_array_next_item(&array_reader, &array_item)){
                                
                                if (item_index >= 4){
                                    break;
                                }
                                
                                append(&msg, "[");
                                append_int_to_str(&msg, item_index);
                                append(&msg, "] = ");
                                
                                bool32 read_string = false;
                                bool32 read_bool = false;
                                
                                char *dest_str = 0;
                                int32_t dest_str_size = 0;
                                
                                bool32 *dest_bool = 0;
                                
                                switch (item_index){
                                    case 0:
                                    {
                                        dest_str = current_project.fkey_commands[index-1].command;
                                        dest_str_size = sizeof(current_project.fkey_commands[index-1].command);
                                        read_string = true;
                                    }break;
                                    
                                    case 1:
                                    {
                                        dest_str = current_project.fkey_commands[index-1].out;
                                        dest_str_size = sizeof(current_project.fkey_commands[index-1].out);
                                        read_string = true;
                                    }break;
                                    
                                    case 2:
                                    {
                                        dest_bool = &current_project.fkey_commands[index-1].use_build_panel;
                                        read_bool = true;
                                    }break;
                                    
                                    case 3:
                                    {
                                        dest_bool = &current_project.fkey_commands[index-1].save_dirty_buffers;
                                        read_bool = true;
                                    }break;
                                }
                                
                                if (read_string){
                                    if (config_int_var(array_item, 0, 0, 0)){
                                        append(&msg, "NULL, ");
                                        dest_str[0] = 0;
                                    }
                                    
                                    char str_space[512];
                                    String str = make_fixed_width_string(str_space);
                                    if (config_string_var(array_item, 0, 0, &str)){
                                        if (str.size < dest_str_size){
                                            string_interpret_escapes(str, dest_str);
                                            append(&msg, dest_str);
                                            append(&msg, ", ");
                                        }
                                        else{
                                            append(&msg, "STRING TOO LONG!, ");
                                        }
                                    }
                                }
                                
                                if (read_bool){
                                    if (config_bool_var(array_item, 0, 0, dest_bool)){
                                        if (*dest_bool){
                                            append(&msg, "true, ");
                                        }
                                        else{
                                            append(&msg, "false, ");
                                        }
                                    }
                                }
                                
                                item_index++;
                            }
                            
                            append(&msg, "}\n");
                            print_message(app, msg.str, msg.size);
                        }
                    }
                }
            }
            else if (config_line.error_str.str != 0){
                char space[2048];
                String str = make_fixed_width_string(space);
                copy(&str, "WARNING: bad syntax in 4coder.config at ");
                append(&str, config_line.error_str);
                append(&str, "\n");
                print_message(app, str.str, str.size);
            }
        }
        
        if (current_project.close_all_files_when_project_opens){
            CString_Array extension_array = {0};
            close_all_files_with_extension(app, scrtach, extension_array);
        }
        
        // Open all project files
        uint32_t flags = 0;
        if (current_project.open_recursively){
            flags |= OpenAllFilesFlag_Recursive;
        }
        open_all_code_with_project_extensions_in_directory(app, project_dir, flags);
        
        // Set window title
        char space[1024];
        String builder = make_fixed_width_string(space);
        append(&builder, "4coder: ");
        append(&builder, project_dir);
        terminate_with_null(&builder);
        set_window_title(app, builder.str);
    }
    
    end_temp_memory(temp);
}

static void
exec_project_fkey_command(Application_Links *app, int32_t command_ind){
    Fkey_Command *fkey = &current_project.fkey_commands[command_ind];
    char *command = fkey->command;
    
    if (command[0] != 0){
        char *out = fkey->out;
        bool32 use_build_panel = fkey->use_build_panel;
        bool32 save_dirty_buffers = fkey->save_dirty_buffers;
        
        if (save_dirty_buffers){
            save_all_dirty_buffers(app);
        }
        
        int32_t command_len = str_size(command);
        
        View_Summary view = {0};
        View_Summary *view_ptr = 0;
        Buffer_Identifier buffer_id = {0};
        uint32_t flags = CLI_OverlapWithConflict;
        
        bool32 set_fancy_font = false;
        if (out[0] != 0){
            int32_t out_len = str_size(out);
            buffer_id = buffer_identifier(out, out_len);
            
            if (use_build_panel){
                view = get_or_open_build_panel(app);
                if (match(out, "*compilation*")){
                    set_fancy_font = true;
                }
            }
            else{
                view = get_active_view(app, AccessAll);
            }
            view_ptr = &view;
            
            memset(&prev_location, 0, sizeof(prev_location));
            lock_jump_buffer(out, out_len);
        }
        else{
            // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
            buffer_id = buffer_identifier(literal("*dump*"));
        }
        
        exec_system_command(app, view_ptr, buffer_id, current_project.dir, current_project.dir_len, command, command_len, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
}

///////////////////////////////

CUSTOM_COMMAND_SIG(load_project)
CUSTOM_DOC("Looks for a project.4coder file in the current directory and tries to load it.  Looks in parent directories until a project file is found or there are no more parents.")
{
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    save_all_dirty_buffers(app);
    char space[512];
    String project_path = make_fixed_width_string(space);
    project_path.size = directory_get_hot(app, project_path.str, project_path.memory_size);
    if (project_path.size >= project_path.memory_size){
        print_message(app, literal("Hot directory longer than hard coded path buffer.\n"));
    }
    else if (project_path.size == 0){
        print_message(app, literal("The hot directory is empty, cannot search for a project.\n"));
    }
    else{
        String data = dump_file_search_up_path(part, project_path, make_lit_string("project.4coder"));
        if (data.str != 0){
            load_project_from_data(app, part, data.str, data.size, project_path);
        }
        else{
            char message_space[512];
            String message = make_fixed_width_string(message_space);
            append(&message, "Did not find project.4coder.  ");
            if (current_project.dir != 0){
                append(&message, "Continuing with: ");
                append(&message, current_project.dir);
            }
            else{
                append(&message, "Continuing without a project");
            }
            append(&message, '\n');
            print_message(app, message.str, message.size);
        }
    }
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(reload_current_project)
CUSTOM_DOC("If a project file has already been loaded, reloads the same file.  Useful for when the project configuration is changed.")
{
    Partition *part = &global_part;
    
    if (current_project.loaded){
        save_all_dirty_buffers(app);
        char space[512];
        String project_path = make_fixed_width_string(space);
        append(&project_path, make_string(current_project.dir, current_project.dir_len));
        if (project_path.size == 0 || !char_is_slash(project_path.str[project_path.size - 1])){
            append(&project_path, "/");
        }
        append(&project_path, "project.4coder");
        terminate_with_null(&project_path);
        
        FILE *file = fopen(project_path.str, "rb");
        if (file != 0){
            Temp_Memory temp = begin_temp_memory(part);
            String data = dump_file_handle(part, file);
            if (data.str != 0){
                load_project_from_data(app, part, data.str, data.size, project_path);
            }
            end_temp_memory(temp);
            fclose(file);
        }
        else{
            print_message(app, literal("project.4coder file not found. Project configuration unchanged."));
        }
    }
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
        directory_set_hot(app, current_project.dir, current_project.dir_len);
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
                fprintf(bat_script, "SET OPTS=%.*s\n", 
                        default_flags_bat.size, default_flags_bat.str);
                fprintf(bat_script, "SET CODE_HOME=%%cd%%\n");
                
                fprintf(bat_script, "pushd %.*s\n", 
                        output_dir.size, output_dir.str);
                fprintf(bat_script, "%.*s %%OPTS%% %%CODE_HOME%%\\%.*s -Fe%.*s\n",
                        default_compiler_bat.size, default_compiler_bat.str,
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
                
                fprintf(sh_script, "CODE_HOME=\"$PWD\"\n");
                
                fprintf(sh_script, "OPTS=%.*s\n", 
                        default_flags_sh.size, default_flags_sh.str);
                
                fprintf(sh_script, "cd %.*s > /dev/null\n", 
                        output_dir.size, output_dir.str);
                fprintf(sh_script, "%.*s $OPTS $CODE_HOME/%.*s -o %.*s\n",
                        default_compiler_sh.size, default_compiler_sh.str,
                        code_file.size, code_file.str,
                        binary_file.size, binary_file.str);
                fprintf(sh_script, "cd $CODE_HOME > /dev/null\n");
                
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

#endif

// BOTTOM

