/*
4coder_project_commands.cpp - Commands for loading and using a project.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_PROJECT_COMMANDS_CPP)
#define FCODER_PROJECT_COMMANDS_CPP

#include "4coder_default_framework.h"
#include "4coder_lib/4coder_mem.h"

#include "4coder_build_commands.cpp"

// TODO(allen): make this a string operation or a lexer operation or something
static void
interpret_escaped_string(char *dst, String src){
    int32_t mode = 0;
    int32_t j = 0;
    for (int32_t i = 0; i < src.size; ++i){
        switch (mode){
            case 0:
            {
                if (src.str[i] == '\\'){
                    mode = 1;
                }
                else{
                    dst[j++] = src.str[i];
                }
            }break;
            
            case 1:
            {
                switch (src.str[i]){
                    case '\\':{dst[j++] = '\\'; mode = 0;}break;
                    case 'n': {dst[j++] = '\n'; mode = 0;}break;
                    case 't': {dst[j++] = '\t'; mode = 0;}break;
                    case '"': {dst[j++] = '"';  mode = 0;}break;
                    case '0': {dst[j++] = '\0'; mode = 0;}break;
                }
            }break;
        }
    }
    dst[j] = 0;
}

static void
close_all_files_with_extension(Application_Links *app, Partition *scratch_part, char **extension_list, int32_t extension_count){
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
            if (extension_count > 0){
                is_match = 0;
                if (buffer.file_name != 0){
                    String extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
                    for (int32_t i = 0; i < extension_count; ++i){
                        if (match(extension, extension_list[i])){
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
    }
    while(do_repeat);
    
    end_temp_memory(temp);
}

static void
open_all_files_with_extension_internal(Application_Links *app, String dir, char **extension_list, int32_t extension_count, bool32 recursive){
    File_List list = get_file_list(app, dir.str, dir.size);
    int32_t dir_size = dir.size;
    
    for (int32_t i = 0; i < list.count; ++i){
        File_Info *info = list.infos + i;
        if (info->folder){
            if (recursive){
                dir.size = dir_size;
                append(&dir, info->filename);
                append(&dir, "/");
                open_all_files_with_extension_internal(app, dir, extension_list, extension_count, recursive);
            }
        }
        else{
            bool32 is_match = true;
            
            if (extension_count > 0){
                is_match = false;
                
                String extension = make_string_cap(info->filename, info->filename_len, info->filename_len+1);
                extension = file_extension(extension);
                for (int32_t j = 0; j < extension_count; ++j){
                    if (match(extension, extension_list[j])){
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
open_all_files_with_extension(Application_Links *app, Partition *scratch_part, char **extension_list, int32_t extension_count, bool32 recursive){
    Temp_Memory temp = begin_temp_memory(scratch_part);
    
    int32_t max_size = 4096;
    char *memory = push_array(scratch_part, char, max_size);
    
    String dir = make_string_cap(memory, 0, max_size);
    dir.size = directory_get_hot(app, dir.str, dir.memory_size);
    open_all_files_with_extension_internal(app, dir, extension_list, extension_count, recursive);
    
    end_temp_memory(temp);
}

// NOTE(allen|a4.0.14): open_all_code and close_all_code now use the extensions set in the loaded project.  If there is no project loaded the extensions ".cpp.hpp.c.h.cc" are used.
CUSTOM_COMMAND_SIG(open_all_code){
    int32_t extension_count = 0;
    char **extension_list = get_current_code_extensions(&extension_count);
    open_all_files_with_extension(app, &global_part, extension_list, extension_count, false);
}

CUSTOM_COMMAND_SIG(open_all_code_recursive){
    int32_t extension_count = 0;
    char **extension_list = get_current_code_extensions(&extension_count);
    open_all_files_with_extension(app, &global_part, extension_list, extension_count, true);
}

CUSTOM_COMMAND_SIG(close_all_code){
    int32_t extension_count = 0;
    char **extension_list = get_current_code_extensions(&extension_count);
    close_all_files_with_extension(app, &global_part, extension_list, extension_count);
}

CUSTOM_COMMAND_SIG(load_project){
    Partition *part = &global_part;
    
    char project_file_space[512];
    String project_name = make_fixed_width_string(project_file_space);
    project_name.size = directory_get_hot(app, project_name.str, project_name.memory_size);
    if (project_name.size >= project_name.memory_size){
        project_name.size = 0;
    }
    
    if (project_name.size != 0){
        int32_t original_size = project_name.size;
        append_sc(&project_name, "project.4coder");
        terminate_with_null(&project_name);
        
        FILE *file = fopen(project_name.str, "rb");
        if (file){
            project_name.size = original_size;
            terminate_with_null(&project_name);
            
            Temp_Memory temp = begin_temp_memory(part);
            
            char *mem = 0;
            int32_t size = 0;
            bool32 file_read_success = file_handle_dump(part, file, &mem, &size);
            if (file_read_success){
                fclose(file);
                
                Cpp_Token_Array array;
                array.count = 0;
                array.max_count = (1 << 20)/sizeof(Cpp_Token);
                array.tokens = push_array(&global_part, Cpp_Token, array.max_count);
                
                Cpp_Lex_Data S = cpp_lex_data_init();
                Cpp_Lex_Result result = cpp_lex_step(&S, mem, size+1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
                
                if (result == LexResult_Finished){
                    // Clear out current project
                    if (current_project.close_all_code_when_this_project_closes){
                        exec_command(app, close_all_code);
                    }
                    current_project = null_project;
                    
                    // Set new project directory
                    {
                        current_project.dir = current_project.dir_space;
                        String str = make_fixed_width_string(current_project.dir_space);
                        copy(&str, project_name);
                        terminate_with_null(&str);
                        current_project.dir_len = str.size;
                    }
                    
                    // Read the settings from project.4coder
                    for (int32_t i = 0; i < array.count; ++i){
                        Config_Line config_line = read_config_line(array, &i);
                        if (config_line.read_success){
                            Config_Item item = get_config_item(config_line, mem, array);
                            
                            {
                                char str_space[512];
                                String str = make_fixed_width_string(str_space);
                                if (config_string_var(item, "extensions", 0, &str)){
                                    if (str.size < sizeof(current_project.extension_space)){
                                        set_project_extensions(&current_project, str);
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
#if defined(_WIN32)
#define FKEY_COMMAND "fkey_command_win"
#elif defined(__linux__)
#define FKEY_COMMAND "fkey_command_linux"
#else
#error no project configuration names for this platform
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
                                                        interpret_escaped_string(dest_str, str);
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
                                                    if (dest_bool){
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
                    }
                    
                    if (current_project.close_all_files_when_project_opens){
                        close_all_files_with_extension(app, &global_part, 0, 0);
                    }
                    
                    // Open all project files
                    if (current_project.open_recursively){
                        exec_command(app, open_all_code_recursive);
                    }
                    else{
                        exec_command(app, open_all_code);
                    }
                }
            }
            
            end_temp_memory(temp);
        }
        else{
            char message_space[512];
            String message = make_fixed_width_string(message_space);
            append_sc(&message, "Did not find project.4coder.  ");
            if (current_project.dir != 0){
                append_sc(&message, "Continuing with: ");
                append_sc(&message, current_project.dir);
            }
            else{
                append_sc(&message, "Continuing without a project");
            }
            print_message(app, message.str, message.size);
        }
    }
    else{
        print_message(app, literal("Failed trying to get project file name"));
    }
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
        
        View_Summary view_ = {0};
        View_Summary *view = 0;
        Buffer_Identifier buffer_id = {0};
        uint32_t flags = 0;
        
        bool32 set_fancy_font = 0;
        
        if (out[0] != 0){
            int32_t out_len = str_size(out);
            buffer_id = buffer_identifier(out, out_len);
            
            view = &view_;
            
            if (use_build_panel){
                view_ = get_or_open_build_panel(app);
                if (match(out, "*compilation*")){
                    set_fancy_font = 1;
                }
            }
            else{
                view_ = get_active_view(app, AccessAll);
            }
            
            prev_location = null_location;
            lock_jump_buffer(out, out_len);
        }
        else{
            // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
            buffer_id = buffer_identifier(literal("*dump*"));
        }
        
        exec_system_command(app, view, buffer_id, current_project.dir, current_project.dir_len, command, command_len, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
}

CUSTOM_COMMAND_SIG(project_fkey_command){
    User_Input input = get_command_input(app);
    if (input.type == UserInputKey){
        if (input.key.keycode >= key_f1 && input.key.keycode <= key_f16){
            int32_t ind = (input.key.keycode - key_f1);
            exec_project_fkey_command(app, ind);
        }
    }
}

#endif

// BOTTOM

