/* macOS System/Graphics/Font API Implementations */

////////////////////////////////

function
system_get_path_sig(){
    String_Const_u8 result = {};
    
    switch (path_code){
        case SystemPath_CurrentDirectory:
        {
            char *working_dir = getcwd(NULL, 0);
            u64 working_dir_length = cstring_length(working_dir);
            
            // TODO(yuval): Maybe use push_string_copy instead
            u8 *out = push_array(arena, u8, working_dir_length);
            block_copy(out, working_dir, working_dir_length);
            
            free(working_dir);
            
            result = SCu8(out, working_dir_length);
        } break;
        
        case SystemPath_Binary:
        {
            local_persist b32 has_stashed_4ed_path = false;
            if (!has_stashed_4ed_path){
                local_const i32 binary_path_capacity = KB(32);
                u8 *memory = (u8*)system_memory_allocate(binary_path_capacity, string_u8_litexpr(file_name_line_number));
                i32 size = mac_get_binary_path(memory, binary_path_capacity);
                Assert(size <= binary_path_capacity - 1);
                
                mac_vars.binary_path = SCu8(memory, size);
                mac_vars.binary_path = string_remove_last_folder(mac_vars.binary_path);
                mac_vars.binary_path.str[mac_vars.binary_path.size] = 0;
                
                has_stashed_4ed_path = true;
            }
            
            result = push_string_copy(arena, mac_vars.binary_path);
        } break;
    }
    
    return(result);
}

function
system_get_canonical_sig(){
    String_Const_u8 result = mac_standardize_path(arena, name);
    return(result);
}

function
system_get_file_list_sig(){
    File_List result = {};
    
    u8* c_directory = push_array(arena, u8, directory.size + 1);
    block_copy(c_directory, directory.str, directory.size);
    c_directory[directory.size] = 0;
    
    DIR *dir = opendir((char*)c_directory);
    if (dir){
        File_Info* first = 0;
        File_Info* last = 0;
        i32 count = 0;
        
        for (struct dirent *entry = readdir(dir);
             entry;
             entry = readdir(dir)){
            char *c_file_name = entry->d_name;
            String_Const_u8 file_name = SCu8(c_file_name);
            
            if (string_match(file_name, string_u8_litexpr(".")) || string_match(file_name, string_u8_litexpr(".."))){
                continue;
            }
            
            File_Info* info = push_array(arena, File_Info, 1);
            sll_queue_push(first, last, info);
            count += 1;
            
            info->file_name = push_string_copy(arena, file_name);
            
            // NOTE(yuval): Get file attributes
            {
                Temp_Memory temp = begin_temp(arena);
                
                b32 append_slash = false;
                u64 file_path_size = directory.size + file_name.size;
                if (string_get_character(directory, directory.size - 1) != '/') {
                    append_slash = true;
                    file_path_size += 1;
                }
                
                char* file_path = push_array(arena, char, file_path_size + 1);
                char* file_path_at = file_path;
                
                block_copy(file_path_at, directory.str, directory.size);
                file_path_at += directory.size;
                
                if (append_slash) {
                    *file_path_at = '/';
                    file_path_at += 1;
                }
                
                block_copy(file_path_at, file_name.str, file_name.size);
                file_path_at += file_name.size;
                
                *file_path_at = 0;
                
                printf("File Path: %s  ", file_path);
                
                struct stat file_stat;
                if (stat(file_path, &file_stat) == 0){
                    info->attributes.size = file_stat.st_size;
                    info->attributes.last_write_time = ;
                    info->attributes.flags = ;
                } else {
                    char* error_message = strerror(errno);
                    printf("ERROR: stat failed with error message '%s'!\n", error_message);
                }
                
                end_temp(temp);
            }
            
            /*++file_count;
i32 size = 0;
for (; fname[size]; ++size);
character_count += size + 1;*/
        }
        
#if 0
        i32 required_size = character_count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_memory_free(file_list->block, file_list->block_size);
            file_list->block = system_memory_allocate(required_size);
            file_list->block_size = required_size;
        }
        
        file_list->infos = (File_Info*)file_list->block;
        char *cursor = (char*)(file_list->infos + file_count);
        
        if (file_list->block != 0){
            rewinddir(d);
            File_Info *info_ptr = file_list->infos;
            for (struct dirent *entry = readdir(d);
                 entry != 0;
                 entry = readdir(d)){
                char *fname = entry->d_name;
                if (match(fname, ".") || match(fname, "..")){
                    continue;
                }
                char *cursor_start = cursor;
                i32 length = copy_fast_unsafe_cc(cursor_start, fname);
                cursor += length;
                
                if (entry->d_type == DT_LNK){
                    struct stat st;
                    if (stat(entry->d_name, &st) != -1){
                        info_ptr->folder = S_ISDIR(st.st_mode);
                    }
                    else{
                        info_ptr->folder = false;
                    }
                }
                else{
                    info_ptr->folder = (entry->d_type == DT_DIR);
                }
                
                info_ptr->filename = cursor_start;
                info_ptr->filename_len = length;
                *cursor++ = 0;
                ++info_ptr;
            }
        }
        
        file_list->count = file_count;
        
#endif
        
        closedir(dir);
    }
    else{
        
        /*system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;*/
    }
    
    return(result);
}

function
system_quick_file_attributes_sig(){
    File_Attributes result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_load_handle_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_load_attributes_sig(){
    File_Attributes result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_load_file_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_load_close_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_save_file_sig(){
    File_Attributes result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_load_library_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_release_library_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_get_proc_sig(){
    Void_Func* result = 0;
    
    NotImplemented;
    
    return(result);
}

function
system_now_time_sig(){
    u64 result = 0;
    
    NotImplemented;
    
    return(result);
}

function
system_wake_up_timer_create_sig(){
    Plat_Handle result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_wake_up_timer_release_sig(){
    NotImplemented;
}

function
system_wake_up_timer_set_sig(){
    NotImplemented;
}

function
system_signal_step_sig(){
    NotImplemented;
}

function
system_sleep_sig(){
    NotImplemented;
}

function
system_post_clipboard_sig(){
    NotImplemented;
}

function
system_cli_call_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_cli_begin_update_sig(){
    NotImplemented;
}

function
system_cli_update_step_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_cli_end_update_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_open_color_picker_sig(){
    NotImplemented;
}

function
system_get_screen_scale_factor_sig(){
    f32 result = 0.0f;
    
    NotImplemented;
    
    return(result);
}

function
system_thread_launch_sig(){
    System_Thread result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_thread_join_sig(){
    NotImplemented;
}

function
system_thread_free_sig(){
    NotImplemented;
}

function
system_thread_get_id_sig(){
    i32 result = 0;
    
    NotImplemented;
    
    return(result);
}

function
system_acquire_global_frame_mutex_sig(){
    NotImplemented;
}

function
system_release_global_frame_mutex_sig(){
    NotImplemented;
}

function
system_mutex_make_sig(){
    System_Mutex result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_mutex_acquire_sig(){
    NotImplemented;
}

function
system_mutex_release_sig(){
    NotImplemented;
}

function
system_mutex_free_sig(){
    NotImplemented;
}

function
system_condition_variable_make_sig(){
    System_Condition_Variable result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_condition_variable_wait_sig(){
    NotImplemented;
}

function
system_condition_variable_signal_sig(){
    NotImplemented;
}

function
system_condition_variable_free_sig(){
    NotImplemented;
}

function
system_memory_allocate_sig(){
    void* result = 0;
    
    NotImplemented;
    
    return(result);
}

function
system_memory_set_protection_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_memory_free_sig(){
    NotImplemented;
}

function
system_memory_annotation_sig(){
    Memory_Annotation result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_show_mouse_cursor_sig(){
    NotImplemented;
}

function
system_set_fullscreen_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_is_fullscreen_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

function
system_get_keyboard_modifiers_sig(){
    Input_Modifier_Set result = {};
    
    NotImplemented;
    
    return(result);
}

////////////////////////////////

function
graphics_get_texture_sig(){
    u32 result = 0;
    
    NotImplemented;
    
    return(result);
}

function
graphics_fill_texture_sig(){
    b32 result = false;
    
    NotImplemented;
    
    return(result);
}

////////////////////////////////

function
font_make_face_sig(){
    Face* result = 0;
    
    NotImplemented;
    
    return(result);
}

////////////////////////////////