function API_Definition*
custom_api_construct(Arena *arena){
API_Definition *result = begin_api(arena, "custom");
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("global_set_setting"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Global_Setting_ID", "setting");
api_param(arena, call, "i64", "value");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("global_get_screen_rectangle"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_thread_context"), string_u8_litexpr("Thread_Context*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("create_child_process"), string_u8_litexpr("Child_Process_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "path");
api_param(arena, call, "String_Const_u8", "command");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("child_process_set_target_buffer"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Child_Process_ID", "child_process_id");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Child_Process_Set_Target_Flags", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_attached_child_process"), string_u8_litexpr("Child_Process_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("child_process_get_attached_buffer"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Child_Process_ID", "child_process_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("child_process_get_state"), string_u8_litexpr("Process_State"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Child_Process_ID", "child_process_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("enqueue_virtual_event"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Input_Event*", "event");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_buffer_count"), string_u8_litexpr("i32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_buffer_next"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_buffer_by_name"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "name");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_buffer_by_file_name"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "file_name");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_read_range"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Range_i64", "range");
api_param(arena, call, "u8*", "out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_replace_range"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Range_i64", "range");
api_param(arena, call, "String_Const_u8", "string");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_batch_edit"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Batch_Edit*", "batch");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_seek_string"), string_u8_litexpr("String_Match"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer");
api_param(arena, call, "String_Const_u8", "needle");
api_param(arena, call, "Scan_Direction", "direction");
api_param(arena, call, "i64", "start_pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_seek_character_class"), string_u8_litexpr("String_Match"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer");
api_param(arena, call, "Character_Predicate*", "predicate");
api_param(arena, call, "Scan_Direction", "direction");
api_param(arena, call, "i64", "start_pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_line_y_difference"), string_u8_litexpr("f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "line_a");
api_param(arena, call, "i64", "line_b");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_line_shift_y"), string_u8_litexpr("Line_Shift_Vertical"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "line");
api_param(arena, call, "f32", "y_shift");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_pos_at_relative_xy"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "Vec2_f32", "relative_xy");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_relative_box_of_pos"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_padded_box_of_pos"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_relative_character_from_pos"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_pos_from_relative_character"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "f32", "width");
api_param(arena, call, "Face_ID", "face_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "relative_character");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_line_y_difference"), string_u8_litexpr("f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "line_a");
api_param(arena, call, "i64", "line_b");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_line_shift_y"), string_u8_litexpr("Line_Shift_Vertical"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "line");
api_param(arena, call, "f32", "y_shift");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_pos_at_relative_xy"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "Vec2_f32", "relative_xy");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_relative_box_of_pos"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_padded_box_of_pos"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_relative_character_from_pos"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_pos_from_relative_character"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i64", "base_line");
api_param(arena, call, "i64", "character");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_exists"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_access_flags"), string_u8_litexpr("Access_Flag"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_size"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_line_count"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("push_buffer_base_name"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("push_buffer_unique_name"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Arena*", "out");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("push_buffer_file_name"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_dirty_state"), string_u8_litexpr("Dirty_State"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_set_dirty_state"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Dirty_State", "dirty_state");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_set_layout"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Layout_Function*", "layout_func");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_clear_layout_cache"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_layout"), string_u8_litexpr("Layout_Function*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_setting"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Buffer_Setting_ID", "setting");
api_param(arena, call, "i64*", "value_out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_set_setting"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Buffer_Setting_ID", "setting");
api_param(arena, call, "i64", "value");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_managed_scope"), string_u8_litexpr("Managed_Scope"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_send_end_signal"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("create_buffer"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "file_name");
api_param(arena, call, "Buffer_Create_Flag", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_save"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "String_Const_u8", "file_name");
api_param(arena, call, "Buffer_Save_Flag", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_kill"), string_u8_litexpr("Buffer_Kill_Result"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Buffer_Kill_Flag", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_reopen"), string_u8_litexpr("Buffer_Reopen_Result"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Buffer_Reopen_Flag", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_get_file_attributes"), string_u8_litexpr("File_Attributes"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_view_next"), string_u8_litexpr("View_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_view_prev"), string_u8_litexpr("View_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_this_ctx_view"), string_u8_litexpr("View_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_active_view"), string_u8_litexpr("View_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_exists"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_buffer"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_cursor_pos"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_mark_pos"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_preferred_x"), string_u8_litexpr("f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_preferred_x"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "f32", "x");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_screen_rect"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_panel"), string_u8_litexpr("Panel_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_get_view"), string_u8_litexpr("View_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
api_param(arena, call, "Access_Flag", "access");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_is_split"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_is_leaf"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_split"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
api_param(arena, call, "Dimension", "split_dim");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_set_split"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
api_param(arena, call, "Panel_Split_Kind", "kind");
api_param(arena, call, "f32", "t");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_swap_children"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_get_root"), string_u8_litexpr("Panel_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_get_parent"), string_u8_litexpr("Panel_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("panel_get_child"), string_u8_litexpr("Panel_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Panel_ID", "panel_id");
api_param(arena, call, "Side", "which_child");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_close"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_buffer_region"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_buffer_scroll"), string_u8_litexpr("Buffer_Scroll"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_active"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_enqueue_command_function"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Custom_Command_Function*", "custom_func");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_setting"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "View_Setting_ID", "setting");
api_param(arena, call, "i64*", "value_out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_setting"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "View_Setting_ID", "setting");
api_param(arena, call, "i64", "value");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_managed_scope"), string_u8_litexpr("Managed_Scope"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_compute_cursor"), string_u8_litexpr("Buffer_Cursor"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer");
api_param(arena, call, "Buffer_Seek", "seek");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_compute_cursor"), string_u8_litexpr("Buffer_Cursor"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Buffer_Seek", "seek");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_camera_bounds"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Vec2_f32", "margin");
api_param(arena, call, "Vec2_f32", "push_in_multiplier");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_get_camera_bounds"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Vec2_f32*", "margin");
api_param(arena, call, "Vec2_f32*", "push_in_multiplier");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_cursor"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Buffer_Seek", "seek");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_buffer_scroll"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Buffer_Scroll", "scroll");
api_param(arena, call, "Set_Buffer_Scroll_Rule", "rule");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_mark"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Buffer_Seek", "seek");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_quit_ui"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_set_buffer"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Set_Buffer_Flag", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_push_context"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "View_Context*", "ctx");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_pop_context"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_alter_context"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "View_Context*", "ctx");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_current_context"), string_u8_litexpr("View_Context"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("view_current_context_hook_memory"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "Hook_ID", "hook_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("create_user_managed_scope"), string_u8_litexpr("Managed_Scope"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("destroy_user_managed_scope"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_global_managed_scope"), string_u8_litexpr("Managed_Scope"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_managed_scope_with_multiple_dependencies"), string_u8_litexpr("Managed_Scope"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope*", "scopes");
api_param(arena, call, "i32", "count");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_scope_clear_contents"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_scope_clear_self_all_dependent_scopes"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_scope_allocator"), string_u8_litexpr("Base_Allocator*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_id_group_highest_id"), string_u8_litexpr("u64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "group");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_id_declare"), string_u8_litexpr("Managed_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "group");
api_param(arena, call, "String_Const_u8", "name");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_id_get"), string_u8_litexpr("Managed_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "group");
api_param(arena, call, "String_Const_u8", "name");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_scope_get_attachment"), string_u8_litexpr("void*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
api_param(arena, call, "Managed_ID", "id");
api_param(arena, call, "u64", "size");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_scope_attachment_erase"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
api_param(arena, call, "Managed_ID", "id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("alloc_managed_memory_in_scope"), string_u8_litexpr("Managed_Object"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Scope", "scope");
api_param(arena, call, "i32", "item_size");
api_param(arena, call, "i32", "count");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("alloc_buffer_markers_on_buffer"), string_u8_litexpr("Managed_Object"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "i32", "count");
api_param(arena, call, "Managed_Scope*", "optional_extra_scope");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_get_item_size"), string_u8_litexpr("u32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_get_item_count"), string_u8_litexpr("u32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_get_pointer"), string_u8_litexpr("void*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_get_type"), string_u8_litexpr("Managed_Object_Type"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_get_containing_scope"), string_u8_litexpr("Managed_Scope"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_free"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_store_data"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
api_param(arena, call, "u32", "first_index");
api_param(arena, call, "u32", "count");
api_param(arena, call, "void*", "mem");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("managed_object_load_data"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Managed_Object", "object");
api_param(arena, call, "u32", "first_index");
api_param(arena, call, "u32", "count");
api_param(arena, call, "void*", "mem_out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_next_input_raw"), string_u8_litexpr("User_Input"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_current_input_sequence_number"), string_u8_litexpr("i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_current_input"), string_u8_litexpr("User_Input"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_current_input"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "User_Input*", "input");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("leave_current_input_unhandled"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_custom_hook"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Hook_ID", "hook_id");
api_param(arena, call, "Void_Func*", "func_ptr");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_custom_hook"), string_u8_litexpr("Void_Func*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Hook_ID", "hook_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_custom_hook_memory_size"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Hook_ID", "hook_id");
api_param(arena, call, "u64", "size");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_mouse_state"), string_u8_litexpr("Mouse_State"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_active_query_bars"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
api_param(arena, call, "i32", "max_result_count");
api_param(arena, call, "Query_Bar_Ptr_Array*", "array_out");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("start_query_bar"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Query_Bar*", "bar");
api_param(arena, call, "u32", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("end_query_bar"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Query_Bar*", "bar");
api_param(arena, call, "u32", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("clear_all_query_bars"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "View_ID", "view_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("print_message"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "message");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("log_string"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "str");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_largest_face_id"), string_u8_litexpr("Face_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_global_face"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_get_max_record_index"), string_u8_litexpr("History_Record_Index"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_get_record_info"), string_u8_litexpr("Record_Info"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "History_Record_Index", "index");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_get_group_sub_record"), string_u8_litexpr("Record_Info"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "History_Record_Index", "index");
api_param(arena, call, "i32", "sub_index");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_get_current_state_index"), string_u8_litexpr("History_Record_Index"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_set_current_state_index"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "History_Record_Index", "index");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_merge_record_range"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "History_Record_Index", "first_index");
api_param(arena, call, "History_Record_Index", "last_index");
api_param(arena, call, "Record_Merge_Flag", "flags");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_history_clear_after_current_state"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("global_history_edit_group_begin"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("global_history_edit_group_end"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_set_face"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Face_ID", "id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_face_description"), string_u8_litexpr("Face_Description"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "face_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_face_metrics"), string_u8_litexpr("Face_Metrics"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "face_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_face_advance_map"), string_u8_litexpr("Face_Advance_Map"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "face_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_face_id"), string_u8_litexpr("Face_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("try_create_new_face"), string_u8_litexpr("Face_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_Description*", "description");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("try_modify_face"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "id");
api_param(arena, call, "Face_Description*", "description");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("try_release_face"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "id");
api_param(arena, call, "Face_ID", "replacement_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("push_hot_directory"), string_u8_litexpr("String_Const_u8"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Arena*", "arena");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_hot_directory"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "string");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("send_exit_signal"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("hard_exit"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("set_window_title"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "String_Const_u8", "title");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("acquire_global_frame_mutex"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("release_global_frame_mutex"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("draw_string_oriented"), string_u8_litexpr("Vec2_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "font_id");
api_param(arena, call, "ARGB_Color", "color");
api_param(arena, call, "String_Const_u8", "str");
api_param(arena, call, "Vec2_f32", "point");
api_param(arena, call, "u32", "flags");
api_param(arena, call, "Vec2_f32", "delta");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_string_advance"), string_u8_litexpr("f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Face_ID", "font_id");
api_param(arena, call, "String_Const_u8", "str");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("draw_rectangle"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Rect_f32", "rect");
api_param(arena, call, "f32", "roundness");
api_param(arena, call, "ARGB_Color", "color");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("draw_rectangle_outline"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Rect_f32", "rect");
api_param(arena, call, "f32", "roundness");
api_param(arena, call, "f32", "thickness");
api_param(arena, call, "ARGB_Color", "color");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("draw_set_clip"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Rect_f32", "new_clip");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_create"), string_u8_litexpr("Text_Layout_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Buffer_ID", "buffer_id");
api_param(arena, call, "Rect_f32", "rect");
api_param(arena, call, "Buffer_Point", "buffer_point");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_region"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "text_layout_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_get_buffer"), string_u8_litexpr("Buffer_ID"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "text_layout_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_get_visible_range"), string_u8_litexpr("Range_i64"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "text_layout_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_line_on_screen"), string_u8_litexpr("Range_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "layout_id");
api_param(arena, call, "i64", "line_number");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_character_on_screen"), string_u8_litexpr("Rect_f32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "layout_id");
api_param(arena, call, "i64", "pos");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("paint_text_color"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "layout_id");
api_param(arena, call, "Range_i64", "range");
api_param(arena, call, "ARGB_Color", "color");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("paint_text_color_blend"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "layout_id");
api_param(arena, call, "Range_i64", "range");
api_param(arena, call, "ARGB_Color", "color");
api_param(arena, call, "f32", "blend");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("text_layout_free"), string_u8_litexpr("b32"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "text_layout_id");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("draw_text_layout"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Text_Layout_ID", "layout_id");
api_param(arena, call, "ARGB_Color", "special_color");
api_param(arena, call, "ARGB_Color", "ghost_color");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("open_color_picker"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Color_Picker*", "picker");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("animate_in_n_milliseconds"), string_u8_litexpr("void"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "u32", "n");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("buffer_find_all_matches"), string_u8_litexpr("String_Match_List"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Arena*", "arena");
api_param(arena, call, "Buffer_ID", "buffer");
api_param(arena, call, "i32", "string_id");
api_param(arena, call, "Range_i64", "range");
api_param(arena, call, "String_Const_u8", "needle");
api_param(arena, call, "Character_Predicate*", "predicate");
api_param(arena, call, "Scan_Direction", "direction");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_core_profile_list"), string_u8_litexpr("Profile_Global_List*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
}
{
API_Call *call = api_call_with_location(arena, result, string_u8_litexpr("get_custom_layer_boundary_docs"), string_u8_litexpr("Doc_Cluster*"), string_u8_litexpr(""));
api_param(arena, call, "Application_Links*", "app");
api_param(arena, call, "Arena*", "arena");
}
return(result);
}
