/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.10.2019
 *
 * System API definition program.
 *
 */

// TOP

#include "4ed_api_definition.cpp"

function API_Definition*
define_api(Arena *arena){
    API_Definition *api = begin_api(arena, "system");
    
    {
        API_Call *call = api_call(arena, api, "get_canonical", "String_Const_u8");
        api_param(arena, call, "Arena*", "arena");
        api_param(arena, call, "String_Const_u8", "name");
    }
    
    {
        API_Call *call = api_call(arena, api, "get_file_list", "File_List");
        api_param(arena, call, "Arena*", "arena");
        api_param(arena, call, "String_Const_u8", "directory");
    }
    
    {
        API_Call *call = api_call(arena, api, "quick_file_attributes", "File_Attributes");
        api_param(arena, call, "Arena*", "scratch");
        api_param(arena, call, "String_Const_u8", "file_name");
    }
    
    {
        API_Call *call = api_call(arena, api, "load_handle", "b32");
        api_param(arena, call, "Arena*", "scratch");
        api_param(arena, call, "char*", "file_name");
        api_param(arena, call, "Plat_Handle*", "out");
    }
    
    {
        API_Call *call = api_call(arena, api, "load_attributes", "File_Attributes");
        api_param(arena, call, "Plat_Handle", "handle");
    }
    
    {
        API_Call *call = api_call(arena, api, "load_file", "b32");
        api_param(arena, call, "Plat_Handle", "handle");
        api_param(arena, call, "char*", "buffer");
        api_param(arena, call, "u32", "size");
    }
    
    {
        API_Call *call = api_call(arena, api, "load_close", "b32");
        api_param(arena, call, "Plat_Handle", "handle");
    }
    
    {
        API_Call *call = api_call(arena, api, "save_file", "File_Attributes");
        api_param(arena, call, "Arena*", "scratch");
        api_param(arena, call, "char*", "file_name");
        api_param(arena, call, "String_Const_u8", "data");
    }
    
    {
        API_Call *call = api_call(arena, api, "load_library", "b32");
        api_param(arena, call, "Arena*", "scratch");
        api_param(arena, call, "String_Const_u8", "file_name");
        api_param(arena, call, "System_Library*", "out");
    }
    
    {
        API_Call *call = api_call(arena, api, "release_library", "b32");
        api_param(arena, call, "System_Library", "handle");
    }
    
    {
        API_Call *call = api_call(arena, api, "get_proc", "Void_Func");
        api_param(arena, call, "System_Library", "handle");
        api_param(arena, call, "char*", "proc_name");
    }
    
    {
        API_Call *call = api_call(arena, api, "now_time", "u64");
    }
    
    {
        API_Call *call = api_call(arena, api, "wake_up_timer_create", "Plat_Handle");
    }
    
    {
        API_Call *call = api_call(arena, api, "wake_up_timer_release", "void");
        api_param(arena, call, "Plat_Handle", "handle");
    }
    
    {
        API_Call *call = api_call(arena, api, "wake_up_timer_set", "void");
        api_param(arena, call, "Plat_Handle", "handle");
        api_param(arena, call, "u32", "time_milliseconds");
    }
    
    {
        API_Call *call = api_call(arena, api, "signal_step", "void");
        api_param(arena, call, "u32", "code");
    }
    
    {
        API_Call *call = api_call(arena, api, "sleep", "void");
        api_param(arena, call, "u64", "microseconds");
    }
    
    {
        API_Call *call = api_call(arena, api, "post_clipboard", "void");
        api_param(arena, call, "String_Const_u8", "str");
    }
    
    {
        API_Call *call = api_call(arena, api, "cli_call", "b32");
        api_param(arena, call, "Arena*", "scratch");
        api_param(arena, call, "char*", "path");
        api_param(arena, call, "char*", "script");
        api_param(arena, call, "CLI_Handles*", "cli_out");
    }
    
    {
        API_Call *call = api_call(arena, api, "cli_begin_update", "void");
        api_param(arena, call, "CLI_Handles*", "cli");
    }
    
    {
        API_Call *call = api_call(arena, api, "cli_update_step", "b32");
        api_param(arena, call, "CLI_Handles*", "cli");
        api_param(arena, call, "char*", "dest");
        api_param(arena, call, "u32", "max");
        api_param(arena, call, "u32*", "amount");
    }
    
    {
        API_Call *call = api_call(arena, api, "cli_end_update", "b32");
        api_param(arena, call, "CLI_Handles*", "cli");
    }
    
    {
        API_Call *call = api_call(arena, api, "open_color_picker", "void");
        api_param(arena, call, "Color_Picker*", "picker");
    }
    
    {
        API_Call *call = api_call(arena, api, "get_screen_scale_factor", "f32");
    }
    
    {
        API_Call *call = api_call(arena, api, "thread_launch", "System_Thread");
        api_param(arena, call, "Thread_Function*", "proc");
        api_param(arena, call, "void*", "ptr");
    }
    
    {
        API_Call *call = api_call(arena, api, "thread_join", "void");
        api_param(arena, call, "System_Thread", "thread");
    }
    
    {
        API_Call *call = api_call(arena, api, "thread_free", "void");
        api_param(arena, call, "System_Thread", "thread");
    }
    
    {
        API_Call *call = api_call(arena, api, "thread_get_id", "i32");
    }
    
    {
        API_Call *call = api_call(arena, api, "mutex_make", "System_Mutex");
    }
    
    {
        API_Call *call = api_call(arena, api, "mutex_acquire", "void");
        api_param(arena, call, "System_Mutex", "mutex");
    }
    
    {
        API_Call *call = api_call(arena, api, "mutex_release", "void");
        api_param(arena, call, "System_Mutex", "mutex");
    }
    
    {
        API_Call *call = api_call(arena, api, "mutex_free", "void");
        api_param(arena, call, "System_Mutex", "mutex");
    }
    
    {
        API_Call *call = api_call(arena, api, "condition_variable_make",
                                  "System_Condition_Variable");
    }
    
    {
        API_Call *call = api_call(arena, api, "condition_variable_wait", "void");
        api_param(arena, call, "System_Condition_Variable", "cv");
        api_param(arena, call, "System_Mutex", "mutex");
    }
    
    {
        API_Call *call = api_call(arena, api, "condition_variable_signal", "void");
        api_param(arena, call, "System_Condition_Variable", "cv");
    }
    
    {
        API_Call *call = api_call(arena, api, "condition_variable_free", "void");
        api_param(arena, call, "System_Condition_Variable", "cv");
    }
    
    {
        API_Call *call = api_call(arena, api, "memory_allocate", "void*");
        api_param(arena, call, "umem", "size");
    }
    
    {
        API_Call *call = api_call(arena, api, "memory_set_protection", "b32");
        api_param(arena, call, "void*", "ptr");
        api_param(arena, call, "umem", "size");
        api_param(arena, call, "u32", "flags");
    }
    
    {
        API_Call *call = api_call(arena, api, "memory_free", "void");
        api_param(arena, call, "void*", "ptr");
        api_param(arena, call, "umem", "size");
    }
    
    {
        API_Call *call = api_call(arena, api, "show_mouse_cursor", "void");
        api_param(arena, call, "i32", "show");
    }
    
    {
        API_Call *call = api_call(arena, api, "set_fullscreen", "b32");
        api_param(arena, call, "b32", "full_screen");
    }
    
    {
        API_Call *call = api_call(arena, api, "is_fullscreen", "b32");
    }
    
    return(api);
}

// BOTTOM

