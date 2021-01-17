function API_Definition*
system_api_construct(Arena *arena){
API_Definition *result = begin_api(arena, "system");
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("error_box"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "char*", "msg");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_path"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "System_Path_Code", "path_code");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_canonical"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "String_Const_u8", "name");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_file_list"), string_u8_litexpr("File_List"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "String_Const_u8", "directory");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("quick_file_attributes"), string_u8_litexpr("File_Attributes"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "scratch");
api_param(arena, call, "String_Const_u8", "file_name");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("load_handle"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "scratch");
api_param(arena, call, "char*", "file_name");
api_param(arena, call, "Plat_Handle*", "out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("load_attributes"), string_u8_litexpr("File_Attributes"), string_u8_litexpr(""));
api_param(arena, call, "Plat_Handle", "handle");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("load_file"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Plat_Handle", "handle");
api_param(arena, call, "char*", "buffer");
api_param(arena, call, "u32", "size");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("load_close"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Plat_Handle", "handle");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("save_file"), string_u8_litexpr("File_Attributes"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "scratch");
api_param(arena, call, "char*", "file_name");
api_param(arena, call, "String_Const_u8", "data");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("load_library"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "scratch");
api_param(arena, call, "String_Const_u8", "file_name");
api_param(arena, call, "System_Library*", "out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("release_library"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "System_Library", "handle");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_proc"), string_u8_litexpr("Void_Func*"), string_u8_litexpr(""));
api_param(arena, call, "System_Library", "handle");
api_param(arena, call, "char*", "proc_name");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("now_time"), string_u8_litexpr("u64"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("now_date_time_universal"), string_u8_litexpr("Date_Time"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("local_date_time_from_universal"), string_u8_litexpr("Date_Time"), string_u8_litexpr(""));
api_param(arena, call, "Date_Time*", "date_time");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("universal_date_time_from_local"), string_u8_litexpr("Date_Time"), string_u8_litexpr(""));
api_param(arena, call, "Date_Time*", "date_time");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("wake_up_timer_create"), string_u8_litexpr("Plat_Handle"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("wake_up_timer_release"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Plat_Handle", "handle");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("wake_up_timer_set"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Plat_Handle", "handle");
api_param(arena, call, "u32", "time_milliseconds");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("signal_step"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "u32", "code");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("sleep"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "u64", "microseconds");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_clipboard"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "i32", "index");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("post_clipboard"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "String_Const_u8", "str");
api_param(arena, call, "i32", "index");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_clipboard_catch_all"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "b32", "enabled");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_clipboard_catch_all"), string_u8_litexpr("b32"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("cli_call"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "scratch");
api_param(arena, call, "char*", "path");
api_param(arena, call, "char*", "script");
api_param(arena, call, "CLI_Handles*", "cli_out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("cli_begin_update"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "CLI_Handles*", "cli");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("cli_update_step"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "CLI_Handles*", "cli");
api_param(arena, call, "char*", "dest");
api_param(arena, call, "u32", "max");
api_param(arena, call, "u32*", "amount");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("cli_end_update"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "CLI_Handles*", "cli");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("open_color_picker"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Color_Picker*", "picker");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_screen_scale_factor"), string_u8_litexpr("f32"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("thread_launch"), string_u8_litexpr("System_Thread"), string_u8_litexpr(""));
api_param(arena, call, "Thread_Function*", "proc");
api_param(arena, call, "void*", "ptr");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("thread_join"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Thread", "thread");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("thread_free"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Thread", "thread");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("thread_get_id"), string_u8_litexpr("i32"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("acquire_global_frame_mutex"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Thread_Context*", "tctx");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("release_global_frame_mutex"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Thread_Context*", "tctx");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("mutex_make"), string_u8_litexpr("System_Mutex"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("mutex_acquire"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Mutex", "mutex");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("mutex_release"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Mutex", "mutex");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("mutex_free"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Mutex", "mutex");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("condition_variable_make"), string_u8_litexpr("System_Condition_Variable"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("condition_variable_wait"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Condition_Variable", "cv");
api_param(arena, call, "System_Mutex", "mutex");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("condition_variable_signal"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Condition_Variable", "cv");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("condition_variable_free"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "System_Condition_Variable", "cv");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("memory_allocate"), string_u8_litexpr("void*"), string_u8_litexpr(""));
api_param(arena, call, "u64", "size");
api_param(arena, call, "String_Const_u8", "location");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("memory_set_protection"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "void*", "ptr");
api_param(arena, call, "u64", "size");
api_param(arena, call, "u32", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("memory_free"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "void*", "ptr");
api_param(arena, call, "u64", "size");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("memory_annotation"), string_u8_litexpr("Memory_Annotation"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "arena");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("show_mouse_cursor"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "i32", "show");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_fullscreen"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "b32", "full_screen");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("is_fullscreen"), string_u8_litexpr("b32"), string_u8_litexpr(""));
(void)call;
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_keyboard_modifiers"), string_u8_litexpr("Input_Modifier_Set"), string_u8_litexpr(""));
api_param(arena, call, "Arena*", "arena");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_key_mode"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Key_Mode", "mode");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_source_mixer"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "void*", "ctx");
api_param(arena, call, "Audio_Mix_Sources_Function*", "mix_func");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_destination_mixer"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Audio_Mix_Destination_Function*", "mix_func");
}
return(result);
}
