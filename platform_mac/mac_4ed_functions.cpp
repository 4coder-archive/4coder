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
    String_Const_u8 result = {};
    
    NotImplemented;
    
    return(result);
}

function
system_get_file_list_sig(){
    File_List result = {};
    
    NotImplemented;
    
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