/*
4coder_open_all_close_all.cpp - commands for opening and closing lots of files together.

type: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_OPEN_ALL_CLOSE_ALL_CPP)
#define FCODER_OPEN_ALL_CLOSE_ALL_CPP

#include "4coder_default_framework.h"

enum{
    OpenAllFilesFlag_Recursive = 1,
};

///////////////////////////////

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
    }while(do_repeat);
    
    end_temp_memory(temp);
}

static void
open_all_files_in_directory_with_extension(Application_Links *app, String dir,
                                           char **extension_list, int32_t extension_count,
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
                open_all_files_in_directory_with_extension(app, dir,
                                                           extension_list, extension_count,
                                                           flags);
            }
        }
        else{
            bool32 is_match = true;
            if (extension_count > 0){
                is_match = false;
                String extension = make_string_cap(info->filename, info->filename_len, info->filename_len + 1);
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
open_all_files_with_extension_in_hot(Application_Links *app, Partition *scratch,
                                     char **extension_list, int32_t extension_count,
                                     uint32_t flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    int32_t max_size = 4096;
    char *memory = push_array(scratch, char, max_size);
    String dir = make_string_cap(memory, 0, max_size);
    dir.size = directory_get_hot(app, dir.str, dir.memory_size);
    open_all_files_in_directory_with_extension(app, dir,
                                               extension_list, extension_count,
                                               flags);
    end_temp_memory(temp);
}

static void
open_all_code_with_project_extensions_in_directory(Application_Links *app, String dir, uint32_t flags){
    int32_t extension_count = 0;
    char **extension_list = get_current_project_extensions(&extension_count);
    open_all_files_in_directory_with_extension(app, dir,
                                               extension_list, extension_count,
                                               flags);
}

CUSTOM_COMMAND_SIG(open_all_code)
CUSTOM_DOC("Open all code in the current directory. File types are determined by extensions. An extension is considered code based on the extensions specified in 4coder.config.")
{
    int32_t extension_count = 0;
    char **extension_list = get_current_project_extensions(&extension_count);
    open_all_files_with_extension_in_hot(app, &global_part, extension_list, extension_count, 0);
}

CUSTOM_COMMAND_SIG(open_all_code_recursive)
CUSTOM_DOC("Works as open_all_code but also runs in all subdirectories.")
{
    int32_t extension_count = 0;
    char **extension_list = get_current_project_extensions(&extension_count);
    open_all_files_with_extension_in_hot(app, &global_part, extension_list, extension_count, OpenAllFilesFlag_Recursive);
}

CUSTOM_COMMAND_SIG(close_all_code)
CUSTOM_DOC("Closes any buffer with a filename ending with an extension configured to be recognized as a code file type.")
{
    int32_t extension_count = 0;
    char **extension_list = get_current_project_extensions(&extension_count);
    close_all_files_with_extension(app, &global_part, extension_list, extension_count);
}

#endif

// BOTTOM

