#define custom_global_set_setting_sig() b32 custom_global_set_setting(Application_Links* app, Global_Setting_ID setting, i64 value)
#define custom_global_get_screen_rectangle_sig() Rect_f32 custom_global_get_screen_rectangle(Application_Links* app)
#define custom_get_thread_context_sig() Thread_Context* custom_get_thread_context(Application_Links* app)
#define custom_create_child_process_sig() b32 custom_create_child_process(Application_Links* app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID* child_process_id_out)
#define custom_child_process_set_target_buffer_sig() b32 custom_child_process_set_target_buffer(Application_Links* app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags)
#define custom_buffer_get_attached_child_process_sig() Child_Process_ID custom_buffer_get_attached_child_process(Application_Links* app, Buffer_ID buffer_id)
#define custom_child_process_get_attached_buffer_sig() Buffer_ID custom_child_process_get_attached_buffer(Application_Links* app, Child_Process_ID child_process_id)
#define custom_child_process_get_state_sig() Process_State custom_child_process_get_state(Application_Links* app, Child_Process_ID child_process_id)
#define custom_clipboard_post_sig() b32 custom_clipboard_post(Application_Links* app, i32 clipboard_id, String_Const_u8 string)
#define custom_clipboard_count_sig() i32 custom_clipboard_count(Application_Links* app, i32 clipboard_id)
#define custom_push_clipboard_index_sig() String_Const_u8 custom_push_clipboard_index(Application_Links* app, Arena* arena, i32 clipboard_id, i32 item_index)
#define custom_get_buffer_count_sig() i32 custom_get_buffer_count(Application_Links* app)
#define custom_get_buffer_next_sig() Buffer_ID custom_get_buffer_next(Application_Links* app, Buffer_ID buffer_id, Access_Flag access)
#define custom_get_buffer_by_name_sig() Buffer_ID custom_get_buffer_by_name(Application_Links* app, String_Const_u8 name, Access_Flag access)
#define custom_get_buffer_by_file_name_sig() Buffer_ID custom_get_buffer_by_file_name(Application_Links* app, String_Const_u8 file_name, Access_Flag access)
#define custom_buffer_read_range_sig() b32 custom_buffer_read_range(Application_Links* app, Buffer_ID buffer_id, Range_i64 range, u8* out)
#define custom_buffer_replace_range_sig() b32 custom_buffer_replace_range(Application_Links* app, Buffer_ID buffer_id, Range_i64 range, String_Const_u8 string)
#define custom_buffer_batch_edit_sig() b32 custom_buffer_batch_edit(Application_Links* app, Buffer_ID buffer_id, Batch_Edit* batch)
#define custom_buffer_seek_string_sig() String_Match custom_buffer_seek_string(Application_Links* app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i64 start_pos)
#define custom_buffer_seek_character_class_sig() String_Match custom_buffer_seek_character_class(Application_Links* app, Buffer_ID buffer, Character_Predicate* predicate, Scan_Direction direction, i64 start_pos)
#define custom_buffer_line_y_difference_sig() f32 custom_buffer_line_y_difference(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line_a, i64 line_b)
#define custom_buffer_line_shift_y_sig() Line_Shift_Vertical custom_buffer_line_shift_y(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line, f32 y_shift)
#define custom_buffer_pos_at_relative_xy_sig() i64 custom_buffer_pos_at_relative_xy(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, Vec2_f32 relative_xy)
#define custom_buffer_relative_xy_of_pos_sig() Vec2_f32 custom_buffer_relative_xy_of_pos(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos)
#define custom_buffer_relative_character_from_pos_sig() i64 custom_buffer_relative_character_from_pos(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos)
#define custom_buffer_pos_from_relative_character_sig() i64 custom_buffer_pos_from_relative_character(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 relative_character)
#define custom_view_line_y_difference_sig() f32 custom_view_line_y_difference(Application_Links* app, View_ID view_id, i64 line_a, i64 line_b)
#define custom_view_line_shift_y_sig() Line_Shift_Vertical custom_view_line_shift_y(Application_Links* app, View_ID view_id, i64 line, f32 y_shift)
#define custom_view_pos_at_relative_xy_sig() i64 custom_view_pos_at_relative_xy(Application_Links* app, View_ID view_id, i64 base_line, Vec2_f32 relative_xy)
#define custom_view_relative_xy_of_pos_sig() Vec2_f32 custom_view_relative_xy_of_pos(Application_Links* app, View_ID view_id, i64 base_line, i64 pos)
#define custom_view_relative_character_from_pos_sig() i64 custom_view_relative_character_from_pos(Application_Links* app, View_ID view_id, i64 base_line, i64 pos)
#define custom_view_pos_from_relative_character_sig() i64 custom_view_pos_from_relative_character(Application_Links* app, View_ID view_id, i64 base_line, i64 character)
#define custom_buffer_exists_sig() b32 custom_buffer_exists(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_ready_sig() b32 custom_buffer_ready(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_get_access_flags_sig() Access_Flag custom_buffer_get_access_flags(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_get_size_sig() i64 custom_buffer_get_size(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_get_line_count_sig() i64 custom_buffer_get_line_count(Application_Links* app, Buffer_ID buffer_id)
#define custom_push_buffer_base_name_sig() String_Const_u8 custom_push_buffer_base_name(Application_Links* app, Arena* arena, Buffer_ID buffer_id)
#define custom_push_buffer_unique_name_sig() String_Const_u8 custom_push_buffer_unique_name(Application_Links* app, Arena* out, Buffer_ID buffer_id)
#define custom_push_buffer_file_name_sig() String_Const_u8 custom_push_buffer_file_name(Application_Links* app, Arena* arena, Buffer_ID buffer_id)
#define custom_buffer_get_dirty_state_sig() Dirty_State custom_buffer_get_dirty_state(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_set_dirty_state_sig() b32 custom_buffer_set_dirty_state(Application_Links* app, Buffer_ID buffer_id, Dirty_State dirty_state)
#define custom_buffer_get_setting_sig() b32 custom_buffer_get_setting(Application_Links* app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64* value_out)
#define custom_buffer_set_setting_sig() b32 custom_buffer_set_setting(Application_Links* app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64 value)
#define custom_buffer_get_managed_scope_sig() Managed_Scope custom_buffer_get_managed_scope(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_send_end_signal_sig() b32 custom_buffer_send_end_signal(Application_Links* app, Buffer_ID buffer_id)
#define custom_create_buffer_sig() Buffer_ID custom_create_buffer(Application_Links* app, String_Const_u8 file_name, Buffer_Create_Flag flags)
#define custom_buffer_save_sig() b32 custom_buffer_save(Application_Links* app, Buffer_ID buffer_id, String_Const_u8 file_name, Buffer_Save_Flag flags)
#define custom_buffer_kill_sig() Buffer_Kill_Result custom_buffer_kill(Application_Links* app, Buffer_ID buffer_id, Buffer_Kill_Flag flags)
#define custom_buffer_reopen_sig() Buffer_Reopen_Result custom_buffer_reopen(Application_Links* app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags)
#define custom_buffer_get_file_attributes_sig() File_Attributes custom_buffer_get_file_attributes(Application_Links* app, Buffer_ID buffer_id)
#define custom_get_file_attributes_sig() File_Attributes custom_get_file_attributes(Application_Links* app, String_Const_u8 file_name)
#define custom_get_view_next_sig() View_ID custom_get_view_next(Application_Links* app, View_ID view_id, Access_Flag access)
#define custom_get_view_prev_sig() View_ID custom_get_view_prev(Application_Links* app, View_ID view_id, Access_Flag access)
#define custom_get_active_view_sig() View_ID custom_get_active_view(Application_Links* app, Access_Flag access)
#define custom_get_active_panel_sig() Panel_ID custom_get_active_panel(Application_Links* app)
#define custom_view_exists_sig() b32 custom_view_exists(Application_Links* app, View_ID view_id)
#define custom_view_get_buffer_sig() Buffer_ID custom_view_get_buffer(Application_Links* app, View_ID view_id, Access_Flag access)
#define custom_view_get_cursor_pos_sig() i64 custom_view_get_cursor_pos(Application_Links* app, View_ID view_id)
#define custom_view_get_mark_pos_sig() i64 custom_view_get_mark_pos(Application_Links* app, View_ID view_id)
#define custom_view_get_preferred_x_sig() f32 custom_view_get_preferred_x(Application_Links* app, View_ID view_id)
#define custom_view_set_preferred_x_sig() b32 custom_view_set_preferred_x(Application_Links* app, View_ID view_id, f32 x)
#define custom_view_get_screen_rect_sig() Rect_f32 custom_view_get_screen_rect(Application_Links* app, View_ID view_id)
#define custom_view_get_panel_sig() Panel_ID custom_view_get_panel(Application_Links* app, View_ID view_id)
#define custom_panel_get_view_sig() View_ID custom_panel_get_view(Application_Links* app, Panel_ID panel_id)
#define custom_panel_is_split_sig() b32 custom_panel_is_split(Application_Links* app, Panel_ID panel_id)
#define custom_panel_is_leaf_sig() b32 custom_panel_is_leaf(Application_Links* app, Panel_ID panel_id)
#define custom_panel_split_sig() b32 custom_panel_split(Application_Links* app, Panel_ID panel_id, Panel_Split_Orientation orientation)
#define custom_panel_set_split_sig() b32 custom_panel_set_split(Application_Links* app, Panel_ID panel_id, Panel_Split_Kind kind, float t)
#define custom_panel_swap_children_sig() b32 custom_panel_swap_children(Application_Links* app, Panel_ID panel_id, Panel_Split_Kind kind, float t)
#define custom_panel_get_parent_sig() Panel_ID custom_panel_get_parent(Application_Links* app, Panel_ID panel_id)
#define custom_panel_get_child_sig() Panel_ID custom_panel_get_child(Application_Links* app, Panel_ID panel_id, Panel_Child which_child)
#define custom_panel_get_max_sig() Panel_ID custom_panel_get_max(Application_Links* app, Panel_ID panel_id)
#define custom_panel_get_margin_sig() Rect_i32 custom_panel_get_margin(Application_Links* app, Panel_ID panel_id)
#define custom_view_close_sig() b32 custom_view_close(Application_Links* app, View_ID view_id)
#define custom_view_get_buffer_region_sig() Rect_f32 custom_view_get_buffer_region(Application_Links* app, View_ID view_id)
#define custom_view_get_buffer_scroll_sig() Buffer_Scroll custom_view_get_buffer_scroll(Application_Links* app, View_ID view_id)
#define custom_view_set_active_sig() b32 custom_view_set_active(Application_Links* app, View_ID view_id)
#define custom_view_get_setting_sig() b32 custom_view_get_setting(Application_Links* app, View_ID view_id, View_Setting_ID setting, i64* value_out)
#define custom_view_set_setting_sig() b32 custom_view_set_setting(Application_Links* app, View_ID view_id, View_Setting_ID setting, i64 value)
#define custom_view_get_managed_scope_sig() Managed_Scope custom_view_get_managed_scope(Application_Links* app, View_ID view_id)
#define custom_buffer_compute_cursor_sig() Buffer_Cursor custom_buffer_compute_cursor(Application_Links* app, Buffer_ID buffer, Buffer_Seek seek)
#define custom_view_compute_cursor_sig() Buffer_Cursor custom_view_compute_cursor(Application_Links* app, View_ID view_id, Buffer_Seek seek)
#define custom_view_set_cursor_sig() b32 custom_view_set_cursor(Application_Links* app, View_ID view_id, Buffer_Seek seek)
#define custom_view_set_buffer_scroll_sig() b32 custom_view_set_buffer_scroll(Application_Links* app, View_ID view_id, Buffer_Scroll scroll, Set_Buffer_Scroll_Rule rule)
#define custom_view_set_mark_sig() b32 custom_view_set_mark(Application_Links* app, View_ID view_id, Buffer_Seek seek)
#define custom_view_set_buffer_sig() b32 custom_view_set_buffer(Application_Links* app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags)
#define custom_view_post_fade_sig() b32 custom_view_post_fade(Application_Links* app, View_ID view_id, f32 seconds, Range_i64 range, int_color color)
#define custom_view_push_context_sig() b32 custom_view_push_context(Application_Links* app, View_ID view_id, View_Context* ctx)
#define custom_view_pop_context_sig() b32 custom_view_pop_context(Application_Links* app, View_ID view_id)
#define custom_view_current_context_sig() View_Context custom_view_current_context(Application_Links* app, View_ID view_id)
#define custom_view_current_context_hook_memory_sig() Data custom_view_current_context_hook_memory(Application_Links* app, View_ID view_id, Hook_ID hook_id)
#define custom_create_user_managed_scope_sig() Managed_Scope custom_create_user_managed_scope(Application_Links* app)
#define custom_destroy_user_managed_scope_sig() b32 custom_destroy_user_managed_scope(Application_Links* app, Managed_Scope scope)
#define custom_get_global_managed_scope_sig() Managed_Scope custom_get_global_managed_scope(Application_Links* app)
#define custom_get_managed_scope_with_multiple_dependencies_sig() Managed_Scope custom_get_managed_scope_with_multiple_dependencies(Application_Links* app, Managed_Scope* scopes, i32 count)
#define custom_managed_scope_clear_contents_sig() b32 custom_managed_scope_clear_contents(Application_Links* app, Managed_Scope scope)
#define custom_managed_scope_clear_self_all_dependent_scopes_sig() b32 custom_managed_scope_clear_self_all_dependent_scopes(Application_Links* app, Managed_Scope scope)
#define custom_managed_scope_allocator_sig() Base_Allocator* custom_managed_scope_allocator(Application_Links* app, Managed_Scope scope)
#define custom_managed_id_declare_sig() Managed_ID custom_managed_id_declare(Application_Links* app, String_Const_u8 name)
#define custom_managed_scope_get_attachment_sig() void* custom_managed_scope_get_attachment(Application_Links* app, Managed_Scope scope, Managed_ID id, umem size)
#define custom_managed_scope_attachment_erase_sig() void* custom_managed_scope_attachment_erase(Application_Links* app, Managed_Scope scope, Managed_ID id)
#define custom_alloc_managed_memory_in_scope_sig() Managed_Object custom_alloc_managed_memory_in_scope(Application_Links* app, Managed_Scope scope, i32 item_size, i32 count)
#define custom_alloc_buffer_markers_on_buffer_sig() Managed_Object custom_alloc_buffer_markers_on_buffer(Application_Links* app, Buffer_ID buffer_id, i32 count, Managed_Scope* optional_extra_scope)
#define custom_managed_object_get_item_size_sig() u32 custom_managed_object_get_item_size(Application_Links* app, Managed_Object object)
#define custom_managed_object_get_item_count_sig() u32 custom_managed_object_get_item_count(Application_Links* app, Managed_Object object)
#define custom_managed_object_get_pointer_sig() void* custom_managed_object_get_pointer(Application_Links* app, Managed_Object object)
#define custom_managed_object_get_type_sig() Managed_Object_Type custom_managed_object_get_type(Application_Links* app, Managed_Object object)
#define custom_managed_object_get_containing_scope_sig() Managed_Scope custom_managed_object_get_containing_scope(Application_Links* app, Managed_Object object)
#define custom_managed_object_free_sig() b32 custom_managed_object_free(Application_Links* app, Managed_Object object)
#define custom_managed_object_store_data_sig() b32 custom_managed_object_store_data(Application_Links* app, Managed_Object object, u32 first_index, u32 count, void* mem)
#define custom_managed_object_load_data_sig() b32 custom_managed_object_load_data(Application_Links* app, Managed_Object object, u32 first_index, u32 count, void* mem_out)
#define custom_get_next_input_sig() User_Input custom_get_next_input(Application_Links* app, Event_Property get_properties, Event_Property abort_properties)
#define custom_get_current_input_sequence_number_sig() i64 custom_get_current_input_sequence_number(Application_Links* app)
#define custom_get_current_input_sig() User_Input custom_get_current_input(Application_Links* app)
#define custom_set_current_input_sig() void custom_set_current_input(Application_Links* app, User_Input* input)
#define custom_leave_current_input_unhandled_sig() void custom_leave_current_input_unhandled(Application_Links* app)
#define custom_set_custom_hook_sig() void custom_set_custom_hook(Application_Links* app, Hook_ID hook_id, Void_Func* func_ptr)
#define custom_set_custom_hook_memory_size_sig() b32 custom_set_custom_hook_memory_size(Application_Links* app, Hook_ID hook_id, umem size)
#define custom_get_mouse_state_sig() Mouse_State custom_get_mouse_state(Application_Links* app)
#define custom_get_active_query_bars_sig() b32 custom_get_active_query_bars(Application_Links* app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array* array_out)
#define custom_start_query_bar_sig() b32 custom_start_query_bar(Application_Links* app, Query_Bar* bar, u32 flags)
#define custom_end_query_bar_sig() void custom_end_query_bar(Application_Links* app, Query_Bar* bar, u32 flags)
#define custom_clear_all_query_bars_sig() void custom_clear_all_query_bars(Application_Links* app, View_ID view_id)
#define custom_print_message_sig() b32 custom_print_message(Application_Links* app, String_Const_u8 message)
#define custom_log_string_sig() b32 custom_log_string(Application_Links* app, String_Const_u8 str)
#define custom_thread_get_id_sig() i32 custom_thread_get_id(Application_Links* app)
#define custom_get_largest_face_id_sig() Face_ID custom_get_largest_face_id(Application_Links* app)
#define custom_set_global_face_sig() b32 custom_set_global_face(Application_Links* app, Face_ID id, b32 apply_to_all_buffers)
#define custom_buffer_history_get_max_record_index_sig() History_Record_Index custom_buffer_history_get_max_record_index(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_history_get_record_info_sig() Record_Info custom_buffer_history_get_record_info(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index)
#define custom_buffer_history_get_group_sub_record_sig() Record_Info custom_buffer_history_get_group_sub_record(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index)
#define custom_buffer_history_get_current_state_index_sig() History_Record_Index custom_buffer_history_get_current_state_index(Application_Links* app, Buffer_ID buffer_id)
#define custom_buffer_history_set_current_state_index_sig() b32 custom_buffer_history_set_current_state_index(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index)
#define custom_buffer_history_merge_record_range_sig() b32 custom_buffer_history_merge_record_range(Application_Links* app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags)
#define custom_buffer_history_clear_after_current_state_sig() b32 custom_buffer_history_clear_after_current_state(Application_Links* app, Buffer_ID buffer_id)
#define custom_global_history_edit_group_begin_sig() void custom_global_history_edit_group_begin(Application_Links* app)
#define custom_global_history_edit_group_end_sig() void custom_global_history_edit_group_end(Application_Links* app)
#define custom_buffer_set_face_sig() b32 custom_buffer_set_face(Application_Links* app, Buffer_ID buffer_id, Face_ID id)
#define custom_get_face_description_sig() Face_Description custom_get_face_description(Application_Links* app, Face_ID face_id)
#define custom_get_face_metrics_sig() Face_Metrics custom_get_face_metrics(Application_Links* app, Face_ID face_id)
#define custom_get_face_id_sig() Face_ID custom_get_face_id(Application_Links* app, Buffer_ID buffer_id)
#define custom_try_create_new_face_sig() Face_ID custom_try_create_new_face(Application_Links* app, Face_Description* description)
#define custom_try_modify_face_sig() b32 custom_try_modify_face(Application_Links* app, Face_ID id, Face_Description* description)
#define custom_try_release_face_sig() b32 custom_try_release_face(Application_Links* app, Face_ID id, Face_ID replacement_id)
#define custom_set_theme_colors_sig() void custom_set_theme_colors(Application_Links* app, Theme_Color* colors, i32 count)
#define custom_get_theme_colors_sig() void custom_get_theme_colors(Application_Links* app, Theme_Color* colors, i32 count)
#define custom_finalize_color_sig() argb_color custom_finalize_color(Application_Links* app, int_color color)
#define custom_push_hot_directory_sig() String_Const_u8 custom_push_hot_directory(Application_Links* app, Arena* arena)
#define custom_set_hot_directory_sig() b32 custom_set_hot_directory(Application_Links* app, String_Const_u8 string)
#define custom_set_gui_up_down_keys_sig() void custom_set_gui_up_down_keys(Application_Links* app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier)
#define custom_send_exit_signal_sig() void custom_send_exit_signal(Application_Links* app)
#define custom_set_window_title_sig() b32 custom_set_window_title(Application_Links* app, String_Const_u8 title)
#define custom_draw_string_oriented_sig() Vec2 custom_draw_string_oriented(Application_Links* app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta)
#define custom_get_string_advance_sig() f32 custom_get_string_advance(Application_Links* app, Face_ID font_id, String_Const_u8 str)
#define custom_draw_rectangle_sig() void custom_draw_rectangle(Application_Links* app, Rect_f32 rect, f32 roundness, int_color color)
#define custom_draw_rectangle_outline_sig() void custom_draw_rectangle_outline(Application_Links* app, Rect_f32 rect, f32 roundness, f32 thickness, int_color color)
#define custom_draw_set_clip_sig() Rect_f32 custom_draw_set_clip(Application_Links* app, Rect_f32 new_clip)
#define custom_text_layout_create_sig() Text_Layout_ID custom_text_layout_create(Application_Links* app, Buffer_ID buffer_id, Rect_f32 rect, Buffer_Point buffer_point)
#define custom_text_layout_region_sig() Rect_f32 custom_text_layout_region(Application_Links* app, Text_Layout_ID text_layout_id)
#define custom_text_layout_get_buffer_sig() Buffer_ID custom_text_layout_get_buffer(Application_Links* app, Text_Layout_ID text_layout_id)
#define custom_text_layout_get_visible_range_sig() Interval_i64 custom_text_layout_get_visible_range(Application_Links* app, Text_Layout_ID text_layout_id)
#define custom_text_layout_line_on_screen_sig() Range_f32 custom_text_layout_line_on_screen(Application_Links* app, Text_Layout_ID layout_id, i64 line_number)
#define custom_text_layout_character_on_screen_sig() Rect_f32 custom_text_layout_character_on_screen(Application_Links* app, Text_Layout_ID layout_id, i64 pos)
#define custom_paint_text_color_sig() void custom_paint_text_color(Application_Links* app, Text_Layout_ID layout_id, Interval_i64 range, int_color color)
#define custom_text_layout_free_sig() b32 custom_text_layout_free(Application_Links* app, Text_Layout_ID text_layout_id)
#define custom_draw_text_layout_sig() void custom_draw_text_layout(Application_Links* app, Text_Layout_ID layout_id)
#define custom_open_color_picker_sig() void custom_open_color_picker(Application_Links* app, Color_Picker* picker)
#define custom_animate_in_n_milliseconds_sig() void custom_animate_in_n_milliseconds(Application_Links* app, u32 n)
#define custom_buffer_find_all_matches_sig() String_Match_List custom_buffer_find_all_matches(Application_Links* app, Arena* arena, Buffer_ID buffer, i32 string_id, Range_i64 range, String_Const_u8 needle, Character_Predicate* predicate, Scan_Direction direction)
typedef b32 custom_global_set_setting_type(Application_Links* app, Global_Setting_ID setting, i64 value);
typedef Rect_f32 custom_global_get_screen_rectangle_type(Application_Links* app);
typedef Thread_Context* custom_get_thread_context_type(Application_Links* app);
typedef b32 custom_create_child_process_type(Application_Links* app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID* child_process_id_out);
typedef b32 custom_child_process_set_target_buffer_type(Application_Links* app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags);
typedef Child_Process_ID custom_buffer_get_attached_child_process_type(Application_Links* app, Buffer_ID buffer_id);
typedef Buffer_ID custom_child_process_get_attached_buffer_type(Application_Links* app, Child_Process_ID child_process_id);
typedef Process_State custom_child_process_get_state_type(Application_Links* app, Child_Process_ID child_process_id);
typedef b32 custom_clipboard_post_type(Application_Links* app, i32 clipboard_id, String_Const_u8 string);
typedef i32 custom_clipboard_count_type(Application_Links* app, i32 clipboard_id);
typedef String_Const_u8 custom_push_clipboard_index_type(Application_Links* app, Arena* arena, i32 clipboard_id, i32 item_index);
typedef i32 custom_get_buffer_count_type(Application_Links* app);
typedef Buffer_ID custom_get_buffer_next_type(Application_Links* app, Buffer_ID buffer_id, Access_Flag access);
typedef Buffer_ID custom_get_buffer_by_name_type(Application_Links* app, String_Const_u8 name, Access_Flag access);
typedef Buffer_ID custom_get_buffer_by_file_name_type(Application_Links* app, String_Const_u8 file_name, Access_Flag access);
typedef b32 custom_buffer_read_range_type(Application_Links* app, Buffer_ID buffer_id, Range_i64 range, u8* out);
typedef b32 custom_buffer_replace_range_type(Application_Links* app, Buffer_ID buffer_id, Range_i64 range, String_Const_u8 string);
typedef b32 custom_buffer_batch_edit_type(Application_Links* app, Buffer_ID buffer_id, Batch_Edit* batch);
typedef String_Match custom_buffer_seek_string_type(Application_Links* app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i64 start_pos);
typedef String_Match custom_buffer_seek_character_class_type(Application_Links* app, Buffer_ID buffer, Character_Predicate* predicate, Scan_Direction direction, i64 start_pos);
typedef f32 custom_buffer_line_y_difference_type(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line_a, i64 line_b);
typedef Line_Shift_Vertical custom_buffer_line_shift_y_type(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line, f32 y_shift);
typedef i64 custom_buffer_pos_at_relative_xy_type(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, Vec2_f32 relative_xy);
typedef Vec2_f32 custom_buffer_relative_xy_of_pos_type(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos);
typedef i64 custom_buffer_relative_character_from_pos_type(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos);
typedef i64 custom_buffer_pos_from_relative_character_type(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 relative_character);
typedef f32 custom_view_line_y_difference_type(Application_Links* app, View_ID view_id, i64 line_a, i64 line_b);
typedef Line_Shift_Vertical custom_view_line_shift_y_type(Application_Links* app, View_ID view_id, i64 line, f32 y_shift);
typedef i64 custom_view_pos_at_relative_xy_type(Application_Links* app, View_ID view_id, i64 base_line, Vec2_f32 relative_xy);
typedef Vec2_f32 custom_view_relative_xy_of_pos_type(Application_Links* app, View_ID view_id, i64 base_line, i64 pos);
typedef i64 custom_view_relative_character_from_pos_type(Application_Links* app, View_ID view_id, i64 base_line, i64 pos);
typedef i64 custom_view_pos_from_relative_character_type(Application_Links* app, View_ID view_id, i64 base_line, i64 character);
typedef b32 custom_buffer_exists_type(Application_Links* app, Buffer_ID buffer_id);
typedef b32 custom_buffer_ready_type(Application_Links* app, Buffer_ID buffer_id);
typedef Access_Flag custom_buffer_get_access_flags_type(Application_Links* app, Buffer_ID buffer_id);
typedef i64 custom_buffer_get_size_type(Application_Links* app, Buffer_ID buffer_id);
typedef i64 custom_buffer_get_line_count_type(Application_Links* app, Buffer_ID buffer_id);
typedef String_Const_u8 custom_push_buffer_base_name_type(Application_Links* app, Arena* arena, Buffer_ID buffer_id);
typedef String_Const_u8 custom_push_buffer_unique_name_type(Application_Links* app, Arena* out, Buffer_ID buffer_id);
typedef String_Const_u8 custom_push_buffer_file_name_type(Application_Links* app, Arena* arena, Buffer_ID buffer_id);
typedef Dirty_State custom_buffer_get_dirty_state_type(Application_Links* app, Buffer_ID buffer_id);
typedef b32 custom_buffer_set_dirty_state_type(Application_Links* app, Buffer_ID buffer_id, Dirty_State dirty_state);
typedef b32 custom_buffer_get_setting_type(Application_Links* app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64* value_out);
typedef b32 custom_buffer_set_setting_type(Application_Links* app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64 value);
typedef Managed_Scope custom_buffer_get_managed_scope_type(Application_Links* app, Buffer_ID buffer_id);
typedef b32 custom_buffer_send_end_signal_type(Application_Links* app, Buffer_ID buffer_id);
typedef Buffer_ID custom_create_buffer_type(Application_Links* app, String_Const_u8 file_name, Buffer_Create_Flag flags);
typedef b32 custom_buffer_save_type(Application_Links* app, Buffer_ID buffer_id, String_Const_u8 file_name, Buffer_Save_Flag flags);
typedef Buffer_Kill_Result custom_buffer_kill_type(Application_Links* app, Buffer_ID buffer_id, Buffer_Kill_Flag flags);
typedef Buffer_Reopen_Result custom_buffer_reopen_type(Application_Links* app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags);
typedef File_Attributes custom_buffer_get_file_attributes_type(Application_Links* app, Buffer_ID buffer_id);
typedef File_Attributes custom_get_file_attributes_type(Application_Links* app, String_Const_u8 file_name);
typedef View_ID custom_get_view_next_type(Application_Links* app, View_ID view_id, Access_Flag access);
typedef View_ID custom_get_view_prev_type(Application_Links* app, View_ID view_id, Access_Flag access);
typedef View_ID custom_get_active_view_type(Application_Links* app, Access_Flag access);
typedef Panel_ID custom_get_active_panel_type(Application_Links* app);
typedef b32 custom_view_exists_type(Application_Links* app, View_ID view_id);
typedef Buffer_ID custom_view_get_buffer_type(Application_Links* app, View_ID view_id, Access_Flag access);
typedef i64 custom_view_get_cursor_pos_type(Application_Links* app, View_ID view_id);
typedef i64 custom_view_get_mark_pos_type(Application_Links* app, View_ID view_id);
typedef f32 custom_view_get_preferred_x_type(Application_Links* app, View_ID view_id);
typedef b32 custom_view_set_preferred_x_type(Application_Links* app, View_ID view_id, f32 x);
typedef Rect_f32 custom_view_get_screen_rect_type(Application_Links* app, View_ID view_id);
typedef Panel_ID custom_view_get_panel_type(Application_Links* app, View_ID view_id);
typedef View_ID custom_panel_get_view_type(Application_Links* app, Panel_ID panel_id);
typedef b32 custom_panel_is_split_type(Application_Links* app, Panel_ID panel_id);
typedef b32 custom_panel_is_leaf_type(Application_Links* app, Panel_ID panel_id);
typedef b32 custom_panel_split_type(Application_Links* app, Panel_ID panel_id, Panel_Split_Orientation orientation);
typedef b32 custom_panel_set_split_type(Application_Links* app, Panel_ID panel_id, Panel_Split_Kind kind, float t);
typedef b32 custom_panel_swap_children_type(Application_Links* app, Panel_ID panel_id, Panel_Split_Kind kind, float t);
typedef Panel_ID custom_panel_get_parent_type(Application_Links* app, Panel_ID panel_id);
typedef Panel_ID custom_panel_get_child_type(Application_Links* app, Panel_ID panel_id, Panel_Child which_child);
typedef Panel_ID custom_panel_get_max_type(Application_Links* app, Panel_ID panel_id);
typedef Rect_i32 custom_panel_get_margin_type(Application_Links* app, Panel_ID panel_id);
typedef b32 custom_view_close_type(Application_Links* app, View_ID view_id);
typedef Rect_f32 custom_view_get_buffer_region_type(Application_Links* app, View_ID view_id);
typedef Buffer_Scroll custom_view_get_buffer_scroll_type(Application_Links* app, View_ID view_id);
typedef b32 custom_view_set_active_type(Application_Links* app, View_ID view_id);
typedef b32 custom_view_get_setting_type(Application_Links* app, View_ID view_id, View_Setting_ID setting, i64* value_out);
typedef b32 custom_view_set_setting_type(Application_Links* app, View_ID view_id, View_Setting_ID setting, i64 value);
typedef Managed_Scope custom_view_get_managed_scope_type(Application_Links* app, View_ID view_id);
typedef Buffer_Cursor custom_buffer_compute_cursor_type(Application_Links* app, Buffer_ID buffer, Buffer_Seek seek);
typedef Buffer_Cursor custom_view_compute_cursor_type(Application_Links* app, View_ID view_id, Buffer_Seek seek);
typedef b32 custom_view_set_cursor_type(Application_Links* app, View_ID view_id, Buffer_Seek seek);
typedef b32 custom_view_set_buffer_scroll_type(Application_Links* app, View_ID view_id, Buffer_Scroll scroll, Set_Buffer_Scroll_Rule rule);
typedef b32 custom_view_set_mark_type(Application_Links* app, View_ID view_id, Buffer_Seek seek);
typedef b32 custom_view_set_buffer_type(Application_Links* app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags);
typedef b32 custom_view_post_fade_type(Application_Links* app, View_ID view_id, f32 seconds, Range_i64 range, int_color color);
typedef b32 custom_view_push_context_type(Application_Links* app, View_ID view_id, View_Context* ctx);
typedef b32 custom_view_pop_context_type(Application_Links* app, View_ID view_id);
typedef View_Context custom_view_current_context_type(Application_Links* app, View_ID view_id);
typedef Data custom_view_current_context_hook_memory_type(Application_Links* app, View_ID view_id, Hook_ID hook_id);
typedef Managed_Scope custom_create_user_managed_scope_type(Application_Links* app);
typedef b32 custom_destroy_user_managed_scope_type(Application_Links* app, Managed_Scope scope);
typedef Managed_Scope custom_get_global_managed_scope_type(Application_Links* app);
typedef Managed_Scope custom_get_managed_scope_with_multiple_dependencies_type(Application_Links* app, Managed_Scope* scopes, i32 count);
typedef b32 custom_managed_scope_clear_contents_type(Application_Links* app, Managed_Scope scope);
typedef b32 custom_managed_scope_clear_self_all_dependent_scopes_type(Application_Links* app, Managed_Scope scope);
typedef Base_Allocator* custom_managed_scope_allocator_type(Application_Links* app, Managed_Scope scope);
typedef Managed_ID custom_managed_id_declare_type(Application_Links* app, String_Const_u8 name);
typedef void* custom_managed_scope_get_attachment_type(Application_Links* app, Managed_Scope scope, Managed_ID id, umem size);
typedef void* custom_managed_scope_attachment_erase_type(Application_Links* app, Managed_Scope scope, Managed_ID id);
typedef Managed_Object custom_alloc_managed_memory_in_scope_type(Application_Links* app, Managed_Scope scope, i32 item_size, i32 count);
typedef Managed_Object custom_alloc_buffer_markers_on_buffer_type(Application_Links* app, Buffer_ID buffer_id, i32 count, Managed_Scope* optional_extra_scope);
typedef u32 custom_managed_object_get_item_size_type(Application_Links* app, Managed_Object object);
typedef u32 custom_managed_object_get_item_count_type(Application_Links* app, Managed_Object object);
typedef void* custom_managed_object_get_pointer_type(Application_Links* app, Managed_Object object);
typedef Managed_Object_Type custom_managed_object_get_type_type(Application_Links* app, Managed_Object object);
typedef Managed_Scope custom_managed_object_get_containing_scope_type(Application_Links* app, Managed_Object object);
typedef b32 custom_managed_object_free_type(Application_Links* app, Managed_Object object);
typedef b32 custom_managed_object_store_data_type(Application_Links* app, Managed_Object object, u32 first_index, u32 count, void* mem);
typedef b32 custom_managed_object_load_data_type(Application_Links* app, Managed_Object object, u32 first_index, u32 count, void* mem_out);
typedef User_Input custom_get_next_input_type(Application_Links* app, Event_Property get_properties, Event_Property abort_properties);
typedef i64 custom_get_current_input_sequence_number_type(Application_Links* app);
typedef User_Input custom_get_current_input_type(Application_Links* app);
typedef void custom_set_current_input_type(Application_Links* app, User_Input* input);
typedef void custom_leave_current_input_unhandled_type(Application_Links* app);
typedef void custom_set_custom_hook_type(Application_Links* app, Hook_ID hook_id, Void_Func* func_ptr);
typedef b32 custom_set_custom_hook_memory_size_type(Application_Links* app, Hook_ID hook_id, umem size);
typedef Mouse_State custom_get_mouse_state_type(Application_Links* app);
typedef b32 custom_get_active_query_bars_type(Application_Links* app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array* array_out);
typedef b32 custom_start_query_bar_type(Application_Links* app, Query_Bar* bar, u32 flags);
typedef void custom_end_query_bar_type(Application_Links* app, Query_Bar* bar, u32 flags);
typedef void custom_clear_all_query_bars_type(Application_Links* app, View_ID view_id);
typedef b32 custom_print_message_type(Application_Links* app, String_Const_u8 message);
typedef b32 custom_log_string_type(Application_Links* app, String_Const_u8 str);
typedef i32 custom_thread_get_id_type(Application_Links* app);
typedef Face_ID custom_get_largest_face_id_type(Application_Links* app);
typedef b32 custom_set_global_face_type(Application_Links* app, Face_ID id, b32 apply_to_all_buffers);
typedef History_Record_Index custom_buffer_history_get_max_record_index_type(Application_Links* app, Buffer_ID buffer_id);
typedef Record_Info custom_buffer_history_get_record_info_type(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index);
typedef Record_Info custom_buffer_history_get_group_sub_record_type(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index);
typedef History_Record_Index custom_buffer_history_get_current_state_index_type(Application_Links* app, Buffer_ID buffer_id);
typedef b32 custom_buffer_history_set_current_state_index_type(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index);
typedef b32 custom_buffer_history_merge_record_range_type(Application_Links* app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags);
typedef b32 custom_buffer_history_clear_after_current_state_type(Application_Links* app, Buffer_ID buffer_id);
typedef void custom_global_history_edit_group_begin_type(Application_Links* app);
typedef void custom_global_history_edit_group_end_type(Application_Links* app);
typedef b32 custom_buffer_set_face_type(Application_Links* app, Buffer_ID buffer_id, Face_ID id);
typedef Face_Description custom_get_face_description_type(Application_Links* app, Face_ID face_id);
typedef Face_Metrics custom_get_face_metrics_type(Application_Links* app, Face_ID face_id);
typedef Face_ID custom_get_face_id_type(Application_Links* app, Buffer_ID buffer_id);
typedef Face_ID custom_try_create_new_face_type(Application_Links* app, Face_Description* description);
typedef b32 custom_try_modify_face_type(Application_Links* app, Face_ID id, Face_Description* description);
typedef b32 custom_try_release_face_type(Application_Links* app, Face_ID id, Face_ID replacement_id);
typedef void custom_set_theme_colors_type(Application_Links* app, Theme_Color* colors, i32 count);
typedef void custom_get_theme_colors_type(Application_Links* app, Theme_Color* colors, i32 count);
typedef argb_color custom_finalize_color_type(Application_Links* app, int_color color);
typedef String_Const_u8 custom_push_hot_directory_type(Application_Links* app, Arena* arena);
typedef b32 custom_set_hot_directory_type(Application_Links* app, String_Const_u8 string);
typedef void custom_set_gui_up_down_keys_type(Application_Links* app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier);
typedef void custom_send_exit_signal_type(Application_Links* app);
typedef b32 custom_set_window_title_type(Application_Links* app, String_Const_u8 title);
typedef Vec2 custom_draw_string_oriented_type(Application_Links* app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta);
typedef f32 custom_get_string_advance_type(Application_Links* app, Face_ID font_id, String_Const_u8 str);
typedef void custom_draw_rectangle_type(Application_Links* app, Rect_f32 rect, f32 roundness, int_color color);
typedef void custom_draw_rectangle_outline_type(Application_Links* app, Rect_f32 rect, f32 roundness, f32 thickness, int_color color);
typedef Rect_f32 custom_draw_set_clip_type(Application_Links* app, Rect_f32 new_clip);
typedef Text_Layout_ID custom_text_layout_create_type(Application_Links* app, Buffer_ID buffer_id, Rect_f32 rect, Buffer_Point buffer_point);
typedef Rect_f32 custom_text_layout_region_type(Application_Links* app, Text_Layout_ID text_layout_id);
typedef Buffer_ID custom_text_layout_get_buffer_type(Application_Links* app, Text_Layout_ID text_layout_id);
typedef Interval_i64 custom_text_layout_get_visible_range_type(Application_Links* app, Text_Layout_ID text_layout_id);
typedef Range_f32 custom_text_layout_line_on_screen_type(Application_Links* app, Text_Layout_ID layout_id, i64 line_number);
typedef Rect_f32 custom_text_layout_character_on_screen_type(Application_Links* app, Text_Layout_ID layout_id, i64 pos);
typedef void custom_paint_text_color_type(Application_Links* app, Text_Layout_ID layout_id, Interval_i64 range, int_color color);
typedef b32 custom_text_layout_free_type(Application_Links* app, Text_Layout_ID text_layout_id);
typedef void custom_draw_text_layout_type(Application_Links* app, Text_Layout_ID layout_id);
typedef void custom_open_color_picker_type(Application_Links* app, Color_Picker* picker);
typedef void custom_animate_in_n_milliseconds_type(Application_Links* app, u32 n);
typedef String_Match_List custom_buffer_find_all_matches_type(Application_Links* app, Arena* arena, Buffer_ID buffer, i32 string_id, Range_i64 range, String_Const_u8 needle, Character_Predicate* predicate, Scan_Direction direction);
struct API_VTable_custom{
custom_global_set_setting_type *global_set_setting;
custom_global_get_screen_rectangle_type *global_get_screen_rectangle;
custom_get_thread_context_type *get_thread_context;
custom_create_child_process_type *create_child_process;
custom_child_process_set_target_buffer_type *child_process_set_target_buffer;
custom_buffer_get_attached_child_process_type *buffer_get_attached_child_process;
custom_child_process_get_attached_buffer_type *child_process_get_attached_buffer;
custom_child_process_get_state_type *child_process_get_state;
custom_clipboard_post_type *clipboard_post;
custom_clipboard_count_type *clipboard_count;
custom_push_clipboard_index_type *push_clipboard_index;
custom_get_buffer_count_type *get_buffer_count;
custom_get_buffer_next_type *get_buffer_next;
custom_get_buffer_by_name_type *get_buffer_by_name;
custom_get_buffer_by_file_name_type *get_buffer_by_file_name;
custom_buffer_read_range_type *buffer_read_range;
custom_buffer_replace_range_type *buffer_replace_range;
custom_buffer_batch_edit_type *buffer_batch_edit;
custom_buffer_seek_string_type *buffer_seek_string;
custom_buffer_seek_character_class_type *buffer_seek_character_class;
custom_buffer_line_y_difference_type *buffer_line_y_difference;
custom_buffer_line_shift_y_type *buffer_line_shift_y;
custom_buffer_pos_at_relative_xy_type *buffer_pos_at_relative_xy;
custom_buffer_relative_xy_of_pos_type *buffer_relative_xy_of_pos;
custom_buffer_relative_character_from_pos_type *buffer_relative_character_from_pos;
custom_buffer_pos_from_relative_character_type *buffer_pos_from_relative_character;
custom_view_line_y_difference_type *view_line_y_difference;
custom_view_line_shift_y_type *view_line_shift_y;
custom_view_pos_at_relative_xy_type *view_pos_at_relative_xy;
custom_view_relative_xy_of_pos_type *view_relative_xy_of_pos;
custom_view_relative_character_from_pos_type *view_relative_character_from_pos;
custom_view_pos_from_relative_character_type *view_pos_from_relative_character;
custom_buffer_exists_type *buffer_exists;
custom_buffer_ready_type *buffer_ready;
custom_buffer_get_access_flags_type *buffer_get_access_flags;
custom_buffer_get_size_type *buffer_get_size;
custom_buffer_get_line_count_type *buffer_get_line_count;
custom_push_buffer_base_name_type *push_buffer_base_name;
custom_push_buffer_unique_name_type *push_buffer_unique_name;
custom_push_buffer_file_name_type *push_buffer_file_name;
custom_buffer_get_dirty_state_type *buffer_get_dirty_state;
custom_buffer_set_dirty_state_type *buffer_set_dirty_state;
custom_buffer_get_setting_type *buffer_get_setting;
custom_buffer_set_setting_type *buffer_set_setting;
custom_buffer_get_managed_scope_type *buffer_get_managed_scope;
custom_buffer_send_end_signal_type *buffer_send_end_signal;
custom_create_buffer_type *create_buffer;
custom_buffer_save_type *buffer_save;
custom_buffer_kill_type *buffer_kill;
custom_buffer_reopen_type *buffer_reopen;
custom_buffer_get_file_attributes_type *buffer_get_file_attributes;
custom_get_file_attributes_type *get_file_attributes;
custom_get_view_next_type *get_view_next;
custom_get_view_prev_type *get_view_prev;
custom_get_active_view_type *get_active_view;
custom_get_active_panel_type *get_active_panel;
custom_view_exists_type *view_exists;
custom_view_get_buffer_type *view_get_buffer;
custom_view_get_cursor_pos_type *view_get_cursor_pos;
custom_view_get_mark_pos_type *view_get_mark_pos;
custom_view_get_preferred_x_type *view_get_preferred_x;
custom_view_set_preferred_x_type *view_set_preferred_x;
custom_view_get_screen_rect_type *view_get_screen_rect;
custom_view_get_panel_type *view_get_panel;
custom_panel_get_view_type *panel_get_view;
custom_panel_is_split_type *panel_is_split;
custom_panel_is_leaf_type *panel_is_leaf;
custom_panel_split_type *panel_split;
custom_panel_set_split_type *panel_set_split;
custom_panel_swap_children_type *panel_swap_children;
custom_panel_get_parent_type *panel_get_parent;
custom_panel_get_child_type *panel_get_child;
custom_panel_get_max_type *panel_get_max;
custom_panel_get_margin_type *panel_get_margin;
custom_view_close_type *view_close;
custom_view_get_buffer_region_type *view_get_buffer_region;
custom_view_get_buffer_scroll_type *view_get_buffer_scroll;
custom_view_set_active_type *view_set_active;
custom_view_get_setting_type *view_get_setting;
custom_view_set_setting_type *view_set_setting;
custom_view_get_managed_scope_type *view_get_managed_scope;
custom_buffer_compute_cursor_type *buffer_compute_cursor;
custom_view_compute_cursor_type *view_compute_cursor;
custom_view_set_cursor_type *view_set_cursor;
custom_view_set_buffer_scroll_type *view_set_buffer_scroll;
custom_view_set_mark_type *view_set_mark;
custom_view_set_buffer_type *view_set_buffer;
custom_view_post_fade_type *view_post_fade;
custom_view_push_context_type *view_push_context;
custom_view_pop_context_type *view_pop_context;
custom_view_current_context_type *view_current_context;
custom_view_current_context_hook_memory_type *view_current_context_hook_memory;
custom_create_user_managed_scope_type *create_user_managed_scope;
custom_destroy_user_managed_scope_type *destroy_user_managed_scope;
custom_get_global_managed_scope_type *get_global_managed_scope;
custom_get_managed_scope_with_multiple_dependencies_type *get_managed_scope_with_multiple_dependencies;
custom_managed_scope_clear_contents_type *managed_scope_clear_contents;
custom_managed_scope_clear_self_all_dependent_scopes_type *managed_scope_clear_self_all_dependent_scopes;
custom_managed_scope_allocator_type *managed_scope_allocator;
custom_managed_id_declare_type *managed_id_declare;
custom_managed_scope_get_attachment_type *managed_scope_get_attachment;
custom_managed_scope_attachment_erase_type *managed_scope_attachment_erase;
custom_alloc_managed_memory_in_scope_type *alloc_managed_memory_in_scope;
custom_alloc_buffer_markers_on_buffer_type *alloc_buffer_markers_on_buffer;
custom_managed_object_get_item_size_type *managed_object_get_item_size;
custom_managed_object_get_item_count_type *managed_object_get_item_count;
custom_managed_object_get_pointer_type *managed_object_get_pointer;
custom_managed_object_get_type_type *managed_object_get_type;
custom_managed_object_get_containing_scope_type *managed_object_get_containing_scope;
custom_managed_object_free_type *managed_object_free;
custom_managed_object_store_data_type *managed_object_store_data;
custom_managed_object_load_data_type *managed_object_load_data;
custom_get_next_input_type *get_next_input;
custom_get_current_input_sequence_number_type *get_current_input_sequence_number;
custom_get_current_input_type *get_current_input;
custom_set_current_input_type *set_current_input;
custom_leave_current_input_unhandled_type *leave_current_input_unhandled;
custom_set_custom_hook_type *set_custom_hook;
custom_set_custom_hook_memory_size_type *set_custom_hook_memory_size;
custom_get_mouse_state_type *get_mouse_state;
custom_get_active_query_bars_type *get_active_query_bars;
custom_start_query_bar_type *start_query_bar;
custom_end_query_bar_type *end_query_bar;
custom_clear_all_query_bars_type *clear_all_query_bars;
custom_print_message_type *print_message;
custom_log_string_type *log_string;
custom_thread_get_id_type *thread_get_id;
custom_get_largest_face_id_type *get_largest_face_id;
custom_set_global_face_type *set_global_face;
custom_buffer_history_get_max_record_index_type *buffer_history_get_max_record_index;
custom_buffer_history_get_record_info_type *buffer_history_get_record_info;
custom_buffer_history_get_group_sub_record_type *buffer_history_get_group_sub_record;
custom_buffer_history_get_current_state_index_type *buffer_history_get_current_state_index;
custom_buffer_history_set_current_state_index_type *buffer_history_set_current_state_index;
custom_buffer_history_merge_record_range_type *buffer_history_merge_record_range;
custom_buffer_history_clear_after_current_state_type *buffer_history_clear_after_current_state;
custom_global_history_edit_group_begin_type *global_history_edit_group_begin;
custom_global_history_edit_group_end_type *global_history_edit_group_end;
custom_buffer_set_face_type *buffer_set_face;
custom_get_face_description_type *get_face_description;
custom_get_face_metrics_type *get_face_metrics;
custom_get_face_id_type *get_face_id;
custom_try_create_new_face_type *try_create_new_face;
custom_try_modify_face_type *try_modify_face;
custom_try_release_face_type *try_release_face;
custom_set_theme_colors_type *set_theme_colors;
custom_get_theme_colors_type *get_theme_colors;
custom_finalize_color_type *finalize_color;
custom_push_hot_directory_type *push_hot_directory;
custom_set_hot_directory_type *set_hot_directory;
custom_set_gui_up_down_keys_type *set_gui_up_down_keys;
custom_send_exit_signal_type *send_exit_signal;
custom_set_window_title_type *set_window_title;
custom_draw_string_oriented_type *draw_string_oriented;
custom_get_string_advance_type *get_string_advance;
custom_draw_rectangle_type *draw_rectangle;
custom_draw_rectangle_outline_type *draw_rectangle_outline;
custom_draw_set_clip_type *draw_set_clip;
custom_text_layout_create_type *text_layout_create;
custom_text_layout_region_type *text_layout_region;
custom_text_layout_get_buffer_type *text_layout_get_buffer;
custom_text_layout_get_visible_range_type *text_layout_get_visible_range;
custom_text_layout_line_on_screen_type *text_layout_line_on_screen;
custom_text_layout_character_on_screen_type *text_layout_character_on_screen;
custom_paint_text_color_type *paint_text_color;
custom_text_layout_free_type *text_layout_free;
custom_draw_text_layout_type *draw_text_layout;
custom_open_color_picker_type *open_color_picker;
custom_animate_in_n_milliseconds_type *animate_in_n_milliseconds;
custom_buffer_find_all_matches_type *buffer_find_all_matches;
};
#if defined(STATIC_LINK_API)
internal b32 global_set_setting(Application_Links* app, Global_Setting_ID setting, i64 value);
internal Rect_f32 global_get_screen_rectangle(Application_Links* app);
internal Thread_Context* get_thread_context(Application_Links* app);
internal b32 create_child_process(Application_Links* app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID* child_process_id_out);
internal b32 child_process_set_target_buffer(Application_Links* app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags);
internal Child_Process_ID buffer_get_attached_child_process(Application_Links* app, Buffer_ID buffer_id);
internal Buffer_ID child_process_get_attached_buffer(Application_Links* app, Child_Process_ID child_process_id);
internal Process_State child_process_get_state(Application_Links* app, Child_Process_ID child_process_id);
internal b32 clipboard_post(Application_Links* app, i32 clipboard_id, String_Const_u8 string);
internal i32 clipboard_count(Application_Links* app, i32 clipboard_id);
internal String_Const_u8 push_clipboard_index(Application_Links* app, Arena* arena, i32 clipboard_id, i32 item_index);
internal i32 get_buffer_count(Application_Links* app);
internal Buffer_ID get_buffer_next(Application_Links* app, Buffer_ID buffer_id, Access_Flag access);
internal Buffer_ID get_buffer_by_name(Application_Links* app, String_Const_u8 name, Access_Flag access);
internal Buffer_ID get_buffer_by_file_name(Application_Links* app, String_Const_u8 file_name, Access_Flag access);
internal b32 buffer_read_range(Application_Links* app, Buffer_ID buffer_id, Range_i64 range, u8* out);
internal b32 buffer_replace_range(Application_Links* app, Buffer_ID buffer_id, Range_i64 range, String_Const_u8 string);
internal b32 buffer_batch_edit(Application_Links* app, Buffer_ID buffer_id, Batch_Edit* batch);
internal String_Match buffer_seek_string(Application_Links* app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i64 start_pos);
internal String_Match buffer_seek_character_class(Application_Links* app, Buffer_ID buffer, Character_Predicate* predicate, Scan_Direction direction, i64 start_pos);
internal f32 buffer_line_y_difference(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line_a, i64 line_b);
internal Line_Shift_Vertical buffer_line_shift_y(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line, f32 y_shift);
internal i64 buffer_pos_at_relative_xy(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, Vec2_f32 relative_xy);
internal Vec2_f32 buffer_relative_xy_of_pos(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos);
internal i64 buffer_relative_character_from_pos(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos);
internal i64 buffer_pos_from_relative_character(Application_Links* app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 relative_character);
internal f32 view_line_y_difference(Application_Links* app, View_ID view_id, i64 line_a, i64 line_b);
internal Line_Shift_Vertical view_line_shift_y(Application_Links* app, View_ID view_id, i64 line, f32 y_shift);
internal i64 view_pos_at_relative_xy(Application_Links* app, View_ID view_id, i64 base_line, Vec2_f32 relative_xy);
internal Vec2_f32 view_relative_xy_of_pos(Application_Links* app, View_ID view_id, i64 base_line, i64 pos);
internal i64 view_relative_character_from_pos(Application_Links* app, View_ID view_id, i64 base_line, i64 pos);
internal i64 view_pos_from_relative_character(Application_Links* app, View_ID view_id, i64 base_line, i64 character);
internal b32 buffer_exists(Application_Links* app, Buffer_ID buffer_id);
internal b32 buffer_ready(Application_Links* app, Buffer_ID buffer_id);
internal Access_Flag buffer_get_access_flags(Application_Links* app, Buffer_ID buffer_id);
internal i64 buffer_get_size(Application_Links* app, Buffer_ID buffer_id);
internal i64 buffer_get_line_count(Application_Links* app, Buffer_ID buffer_id);
internal String_Const_u8 push_buffer_base_name(Application_Links* app, Arena* arena, Buffer_ID buffer_id);
internal String_Const_u8 push_buffer_unique_name(Application_Links* app, Arena* out, Buffer_ID buffer_id);
internal String_Const_u8 push_buffer_file_name(Application_Links* app, Arena* arena, Buffer_ID buffer_id);
internal Dirty_State buffer_get_dirty_state(Application_Links* app, Buffer_ID buffer_id);
internal b32 buffer_set_dirty_state(Application_Links* app, Buffer_ID buffer_id, Dirty_State dirty_state);
internal b32 buffer_get_setting(Application_Links* app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64* value_out);
internal b32 buffer_set_setting(Application_Links* app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64 value);
internal Managed_Scope buffer_get_managed_scope(Application_Links* app, Buffer_ID buffer_id);
internal b32 buffer_send_end_signal(Application_Links* app, Buffer_ID buffer_id);
internal Buffer_ID create_buffer(Application_Links* app, String_Const_u8 file_name, Buffer_Create_Flag flags);
internal b32 buffer_save(Application_Links* app, Buffer_ID buffer_id, String_Const_u8 file_name, Buffer_Save_Flag flags);
internal Buffer_Kill_Result buffer_kill(Application_Links* app, Buffer_ID buffer_id, Buffer_Kill_Flag flags);
internal Buffer_Reopen_Result buffer_reopen(Application_Links* app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags);
internal File_Attributes buffer_get_file_attributes(Application_Links* app, Buffer_ID buffer_id);
internal File_Attributes get_file_attributes(Application_Links* app, String_Const_u8 file_name);
internal View_ID get_view_next(Application_Links* app, View_ID view_id, Access_Flag access);
internal View_ID get_view_prev(Application_Links* app, View_ID view_id, Access_Flag access);
internal View_ID get_active_view(Application_Links* app, Access_Flag access);
internal Panel_ID get_active_panel(Application_Links* app);
internal b32 view_exists(Application_Links* app, View_ID view_id);
internal Buffer_ID view_get_buffer(Application_Links* app, View_ID view_id, Access_Flag access);
internal i64 view_get_cursor_pos(Application_Links* app, View_ID view_id);
internal i64 view_get_mark_pos(Application_Links* app, View_ID view_id);
internal f32 view_get_preferred_x(Application_Links* app, View_ID view_id);
internal b32 view_set_preferred_x(Application_Links* app, View_ID view_id, f32 x);
internal Rect_f32 view_get_screen_rect(Application_Links* app, View_ID view_id);
internal Panel_ID view_get_panel(Application_Links* app, View_ID view_id);
internal View_ID panel_get_view(Application_Links* app, Panel_ID panel_id);
internal b32 panel_is_split(Application_Links* app, Panel_ID panel_id);
internal b32 panel_is_leaf(Application_Links* app, Panel_ID panel_id);
internal b32 panel_split(Application_Links* app, Panel_ID panel_id, Panel_Split_Orientation orientation);
internal b32 panel_set_split(Application_Links* app, Panel_ID panel_id, Panel_Split_Kind kind, float t);
internal b32 panel_swap_children(Application_Links* app, Panel_ID panel_id, Panel_Split_Kind kind, float t);
internal Panel_ID panel_get_parent(Application_Links* app, Panel_ID panel_id);
internal Panel_ID panel_get_child(Application_Links* app, Panel_ID panel_id, Panel_Child which_child);
internal Panel_ID panel_get_max(Application_Links* app, Panel_ID panel_id);
internal Rect_i32 panel_get_margin(Application_Links* app, Panel_ID panel_id);
internal b32 view_close(Application_Links* app, View_ID view_id);
internal Rect_f32 view_get_buffer_region(Application_Links* app, View_ID view_id);
internal Buffer_Scroll view_get_buffer_scroll(Application_Links* app, View_ID view_id);
internal b32 view_set_active(Application_Links* app, View_ID view_id);
internal b32 view_get_setting(Application_Links* app, View_ID view_id, View_Setting_ID setting, i64* value_out);
internal b32 view_set_setting(Application_Links* app, View_ID view_id, View_Setting_ID setting, i64 value);
internal Managed_Scope view_get_managed_scope(Application_Links* app, View_ID view_id);
internal Buffer_Cursor buffer_compute_cursor(Application_Links* app, Buffer_ID buffer, Buffer_Seek seek);
internal Buffer_Cursor view_compute_cursor(Application_Links* app, View_ID view_id, Buffer_Seek seek);
internal b32 view_set_cursor(Application_Links* app, View_ID view_id, Buffer_Seek seek);
internal b32 view_set_buffer_scroll(Application_Links* app, View_ID view_id, Buffer_Scroll scroll, Set_Buffer_Scroll_Rule rule);
internal b32 view_set_mark(Application_Links* app, View_ID view_id, Buffer_Seek seek);
internal b32 view_set_buffer(Application_Links* app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags);
internal b32 view_post_fade(Application_Links* app, View_ID view_id, f32 seconds, Range_i64 range, int_color color);
internal b32 view_push_context(Application_Links* app, View_ID view_id, View_Context* ctx);
internal b32 view_pop_context(Application_Links* app, View_ID view_id);
internal View_Context view_current_context(Application_Links* app, View_ID view_id);
internal Data view_current_context_hook_memory(Application_Links* app, View_ID view_id, Hook_ID hook_id);
internal Managed_Scope create_user_managed_scope(Application_Links* app);
internal b32 destroy_user_managed_scope(Application_Links* app, Managed_Scope scope);
internal Managed_Scope get_global_managed_scope(Application_Links* app);
internal Managed_Scope get_managed_scope_with_multiple_dependencies(Application_Links* app, Managed_Scope* scopes, i32 count);
internal b32 managed_scope_clear_contents(Application_Links* app, Managed_Scope scope);
internal b32 managed_scope_clear_self_all_dependent_scopes(Application_Links* app, Managed_Scope scope);
internal Base_Allocator* managed_scope_allocator(Application_Links* app, Managed_Scope scope);
internal Managed_ID managed_id_declare(Application_Links* app, String_Const_u8 name);
internal void* managed_scope_get_attachment(Application_Links* app, Managed_Scope scope, Managed_ID id, umem size);
internal void* managed_scope_attachment_erase(Application_Links* app, Managed_Scope scope, Managed_ID id);
internal Managed_Object alloc_managed_memory_in_scope(Application_Links* app, Managed_Scope scope, i32 item_size, i32 count);
internal Managed_Object alloc_buffer_markers_on_buffer(Application_Links* app, Buffer_ID buffer_id, i32 count, Managed_Scope* optional_extra_scope);
internal u32 managed_object_get_item_size(Application_Links* app, Managed_Object object);
internal u32 managed_object_get_item_count(Application_Links* app, Managed_Object object);
internal void* managed_object_get_pointer(Application_Links* app, Managed_Object object);
internal Managed_Object_Type managed_object_get_type(Application_Links* app, Managed_Object object);
internal Managed_Scope managed_object_get_containing_scope(Application_Links* app, Managed_Object object);
internal b32 managed_object_free(Application_Links* app, Managed_Object object);
internal b32 managed_object_store_data(Application_Links* app, Managed_Object object, u32 first_index, u32 count, void* mem);
internal b32 managed_object_load_data(Application_Links* app, Managed_Object object, u32 first_index, u32 count, void* mem_out);
internal User_Input get_next_input(Application_Links* app, Event_Property get_properties, Event_Property abort_properties);
internal i64 get_current_input_sequence_number(Application_Links* app);
internal User_Input get_current_input(Application_Links* app);
internal void set_current_input(Application_Links* app, User_Input* input);
internal void leave_current_input_unhandled(Application_Links* app);
internal void set_custom_hook(Application_Links* app, Hook_ID hook_id, Void_Func* func_ptr);
internal b32 set_custom_hook_memory_size(Application_Links* app, Hook_ID hook_id, umem size);
internal Mouse_State get_mouse_state(Application_Links* app);
internal b32 get_active_query_bars(Application_Links* app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array* array_out);
internal b32 start_query_bar(Application_Links* app, Query_Bar* bar, u32 flags);
internal void end_query_bar(Application_Links* app, Query_Bar* bar, u32 flags);
internal void clear_all_query_bars(Application_Links* app, View_ID view_id);
internal b32 print_message(Application_Links* app, String_Const_u8 message);
internal b32 log_string(Application_Links* app, String_Const_u8 str);
internal i32 thread_get_id(Application_Links* app);
internal Face_ID get_largest_face_id(Application_Links* app);
internal b32 set_global_face(Application_Links* app, Face_ID id, b32 apply_to_all_buffers);
internal History_Record_Index buffer_history_get_max_record_index(Application_Links* app, Buffer_ID buffer_id);
internal Record_Info buffer_history_get_record_info(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index);
internal Record_Info buffer_history_get_group_sub_record(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index);
internal History_Record_Index buffer_history_get_current_state_index(Application_Links* app, Buffer_ID buffer_id);
internal b32 buffer_history_set_current_state_index(Application_Links* app, Buffer_ID buffer_id, History_Record_Index index);
internal b32 buffer_history_merge_record_range(Application_Links* app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags);
internal b32 buffer_history_clear_after_current_state(Application_Links* app, Buffer_ID buffer_id);
internal void global_history_edit_group_begin(Application_Links* app);
internal void global_history_edit_group_end(Application_Links* app);
internal b32 buffer_set_face(Application_Links* app, Buffer_ID buffer_id, Face_ID id);
internal Face_Description get_face_description(Application_Links* app, Face_ID face_id);
internal Face_Metrics get_face_metrics(Application_Links* app, Face_ID face_id);
internal Face_ID get_face_id(Application_Links* app, Buffer_ID buffer_id);
internal Face_ID try_create_new_face(Application_Links* app, Face_Description* description);
internal b32 try_modify_face(Application_Links* app, Face_ID id, Face_Description* description);
internal b32 try_release_face(Application_Links* app, Face_ID id, Face_ID replacement_id);
internal void set_theme_colors(Application_Links* app, Theme_Color* colors, i32 count);
internal void get_theme_colors(Application_Links* app, Theme_Color* colors, i32 count);
internal argb_color finalize_color(Application_Links* app, int_color color);
internal String_Const_u8 push_hot_directory(Application_Links* app, Arena* arena);
internal b32 set_hot_directory(Application_Links* app, String_Const_u8 string);
internal void set_gui_up_down_keys(Application_Links* app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier);
internal void send_exit_signal(Application_Links* app);
internal b32 set_window_title(Application_Links* app, String_Const_u8 title);
internal Vec2 draw_string_oriented(Application_Links* app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta);
internal f32 get_string_advance(Application_Links* app, Face_ID font_id, String_Const_u8 str);
internal void draw_rectangle(Application_Links* app, Rect_f32 rect, f32 roundness, int_color color);
internal void draw_rectangle_outline(Application_Links* app, Rect_f32 rect, f32 roundness, f32 thickness, int_color color);
internal Rect_f32 draw_set_clip(Application_Links* app, Rect_f32 new_clip);
internal Text_Layout_ID text_layout_create(Application_Links* app, Buffer_ID buffer_id, Rect_f32 rect, Buffer_Point buffer_point);
internal Rect_f32 text_layout_region(Application_Links* app, Text_Layout_ID text_layout_id);
internal Buffer_ID text_layout_get_buffer(Application_Links* app, Text_Layout_ID text_layout_id);
internal Interval_i64 text_layout_get_visible_range(Application_Links* app, Text_Layout_ID text_layout_id);
internal Range_f32 text_layout_line_on_screen(Application_Links* app, Text_Layout_ID layout_id, i64 line_number);
internal Rect_f32 text_layout_character_on_screen(Application_Links* app, Text_Layout_ID layout_id, i64 pos);
internal void paint_text_color(Application_Links* app, Text_Layout_ID layout_id, Interval_i64 range, int_color color);
internal b32 text_layout_free(Application_Links* app, Text_Layout_ID text_layout_id);
internal void draw_text_layout(Application_Links* app, Text_Layout_ID layout_id);
internal void open_color_picker(Application_Links* app, Color_Picker* picker);
internal void animate_in_n_milliseconds(Application_Links* app, u32 n);
internal String_Match_List buffer_find_all_matches(Application_Links* app, Arena* arena, Buffer_ID buffer, i32 string_id, Range_i64 range, String_Const_u8 needle, Character_Predicate* predicate, Scan_Direction direction);
#undef STATIC_LINK_API
#elif defined(DYNAMIC_LINK_API)
global custom_global_set_setting_type *global_set_setting = 0;
global custom_global_get_screen_rectangle_type *global_get_screen_rectangle = 0;
global custom_get_thread_context_type *get_thread_context = 0;
global custom_create_child_process_type *create_child_process = 0;
global custom_child_process_set_target_buffer_type *child_process_set_target_buffer = 0;
global custom_buffer_get_attached_child_process_type *buffer_get_attached_child_process = 0;
global custom_child_process_get_attached_buffer_type *child_process_get_attached_buffer = 0;
global custom_child_process_get_state_type *child_process_get_state = 0;
global custom_clipboard_post_type *clipboard_post = 0;
global custom_clipboard_count_type *clipboard_count = 0;
global custom_push_clipboard_index_type *push_clipboard_index = 0;
global custom_get_buffer_count_type *get_buffer_count = 0;
global custom_get_buffer_next_type *get_buffer_next = 0;
global custom_get_buffer_by_name_type *get_buffer_by_name = 0;
global custom_get_buffer_by_file_name_type *get_buffer_by_file_name = 0;
global custom_buffer_read_range_type *buffer_read_range = 0;
global custom_buffer_replace_range_type *buffer_replace_range = 0;
global custom_buffer_batch_edit_type *buffer_batch_edit = 0;
global custom_buffer_seek_string_type *buffer_seek_string = 0;
global custom_buffer_seek_character_class_type *buffer_seek_character_class = 0;
global custom_buffer_line_y_difference_type *buffer_line_y_difference = 0;
global custom_buffer_line_shift_y_type *buffer_line_shift_y = 0;
global custom_buffer_pos_at_relative_xy_type *buffer_pos_at_relative_xy = 0;
global custom_buffer_relative_xy_of_pos_type *buffer_relative_xy_of_pos = 0;
global custom_buffer_relative_character_from_pos_type *buffer_relative_character_from_pos = 0;
global custom_buffer_pos_from_relative_character_type *buffer_pos_from_relative_character = 0;
global custom_view_line_y_difference_type *view_line_y_difference = 0;
global custom_view_line_shift_y_type *view_line_shift_y = 0;
global custom_view_pos_at_relative_xy_type *view_pos_at_relative_xy = 0;
global custom_view_relative_xy_of_pos_type *view_relative_xy_of_pos = 0;
global custom_view_relative_character_from_pos_type *view_relative_character_from_pos = 0;
global custom_view_pos_from_relative_character_type *view_pos_from_relative_character = 0;
global custom_buffer_exists_type *buffer_exists = 0;
global custom_buffer_ready_type *buffer_ready = 0;
global custom_buffer_get_access_flags_type *buffer_get_access_flags = 0;
global custom_buffer_get_size_type *buffer_get_size = 0;
global custom_buffer_get_line_count_type *buffer_get_line_count = 0;
global custom_push_buffer_base_name_type *push_buffer_base_name = 0;
global custom_push_buffer_unique_name_type *push_buffer_unique_name = 0;
global custom_push_buffer_file_name_type *push_buffer_file_name = 0;
global custom_buffer_get_dirty_state_type *buffer_get_dirty_state = 0;
global custom_buffer_set_dirty_state_type *buffer_set_dirty_state = 0;
global custom_buffer_get_setting_type *buffer_get_setting = 0;
global custom_buffer_set_setting_type *buffer_set_setting = 0;
global custom_buffer_get_managed_scope_type *buffer_get_managed_scope = 0;
global custom_buffer_send_end_signal_type *buffer_send_end_signal = 0;
global custom_create_buffer_type *create_buffer = 0;
global custom_buffer_save_type *buffer_save = 0;
global custom_buffer_kill_type *buffer_kill = 0;
global custom_buffer_reopen_type *buffer_reopen = 0;
global custom_buffer_get_file_attributes_type *buffer_get_file_attributes = 0;
global custom_get_file_attributes_type *get_file_attributes = 0;
global custom_get_view_next_type *get_view_next = 0;
global custom_get_view_prev_type *get_view_prev = 0;
global custom_get_active_view_type *get_active_view = 0;
global custom_get_active_panel_type *get_active_panel = 0;
global custom_view_exists_type *view_exists = 0;
global custom_view_get_buffer_type *view_get_buffer = 0;
global custom_view_get_cursor_pos_type *view_get_cursor_pos = 0;
global custom_view_get_mark_pos_type *view_get_mark_pos = 0;
global custom_view_get_preferred_x_type *view_get_preferred_x = 0;
global custom_view_set_preferred_x_type *view_set_preferred_x = 0;
global custom_view_get_screen_rect_type *view_get_screen_rect = 0;
global custom_view_get_panel_type *view_get_panel = 0;
global custom_panel_get_view_type *panel_get_view = 0;
global custom_panel_is_split_type *panel_is_split = 0;
global custom_panel_is_leaf_type *panel_is_leaf = 0;
global custom_panel_split_type *panel_split = 0;
global custom_panel_set_split_type *panel_set_split = 0;
global custom_panel_swap_children_type *panel_swap_children = 0;
global custom_panel_get_parent_type *panel_get_parent = 0;
global custom_panel_get_child_type *panel_get_child = 0;
global custom_panel_get_max_type *panel_get_max = 0;
global custom_panel_get_margin_type *panel_get_margin = 0;
global custom_view_close_type *view_close = 0;
global custom_view_get_buffer_region_type *view_get_buffer_region = 0;
global custom_view_get_buffer_scroll_type *view_get_buffer_scroll = 0;
global custom_view_set_active_type *view_set_active = 0;
global custom_view_get_setting_type *view_get_setting = 0;
global custom_view_set_setting_type *view_set_setting = 0;
global custom_view_get_managed_scope_type *view_get_managed_scope = 0;
global custom_buffer_compute_cursor_type *buffer_compute_cursor = 0;
global custom_view_compute_cursor_type *view_compute_cursor = 0;
global custom_view_set_cursor_type *view_set_cursor = 0;
global custom_view_set_buffer_scroll_type *view_set_buffer_scroll = 0;
global custom_view_set_mark_type *view_set_mark = 0;
global custom_view_set_buffer_type *view_set_buffer = 0;
global custom_view_post_fade_type *view_post_fade = 0;
global custom_view_push_context_type *view_push_context = 0;
global custom_view_pop_context_type *view_pop_context = 0;
global custom_view_current_context_type *view_current_context = 0;
global custom_view_current_context_hook_memory_type *view_current_context_hook_memory = 0;
global custom_create_user_managed_scope_type *create_user_managed_scope = 0;
global custom_destroy_user_managed_scope_type *destroy_user_managed_scope = 0;
global custom_get_global_managed_scope_type *get_global_managed_scope = 0;
global custom_get_managed_scope_with_multiple_dependencies_type *get_managed_scope_with_multiple_dependencies = 0;
global custom_managed_scope_clear_contents_type *managed_scope_clear_contents = 0;
global custom_managed_scope_clear_self_all_dependent_scopes_type *managed_scope_clear_self_all_dependent_scopes = 0;
global custom_managed_scope_allocator_type *managed_scope_allocator = 0;
global custom_managed_id_declare_type *managed_id_declare = 0;
global custom_managed_scope_get_attachment_type *managed_scope_get_attachment = 0;
global custom_managed_scope_attachment_erase_type *managed_scope_attachment_erase = 0;
global custom_alloc_managed_memory_in_scope_type *alloc_managed_memory_in_scope = 0;
global custom_alloc_buffer_markers_on_buffer_type *alloc_buffer_markers_on_buffer = 0;
global custom_managed_object_get_item_size_type *managed_object_get_item_size = 0;
global custom_managed_object_get_item_count_type *managed_object_get_item_count = 0;
global custom_managed_object_get_pointer_type *managed_object_get_pointer = 0;
global custom_managed_object_get_type_type *managed_object_get_type = 0;
global custom_managed_object_get_containing_scope_type *managed_object_get_containing_scope = 0;
global custom_managed_object_free_type *managed_object_free = 0;
global custom_managed_object_store_data_type *managed_object_store_data = 0;
global custom_managed_object_load_data_type *managed_object_load_data = 0;
global custom_get_next_input_type *get_next_input = 0;
global custom_get_current_input_sequence_number_type *get_current_input_sequence_number = 0;
global custom_get_current_input_type *get_current_input = 0;
global custom_set_current_input_type *set_current_input = 0;
global custom_leave_current_input_unhandled_type *leave_current_input_unhandled = 0;
global custom_set_custom_hook_type *set_custom_hook = 0;
global custom_set_custom_hook_memory_size_type *set_custom_hook_memory_size = 0;
global custom_get_mouse_state_type *get_mouse_state = 0;
global custom_get_active_query_bars_type *get_active_query_bars = 0;
global custom_start_query_bar_type *start_query_bar = 0;
global custom_end_query_bar_type *end_query_bar = 0;
global custom_clear_all_query_bars_type *clear_all_query_bars = 0;
global custom_print_message_type *print_message = 0;
global custom_log_string_type *log_string = 0;
global custom_thread_get_id_type *thread_get_id = 0;
global custom_get_largest_face_id_type *get_largest_face_id = 0;
global custom_set_global_face_type *set_global_face = 0;
global custom_buffer_history_get_max_record_index_type *buffer_history_get_max_record_index = 0;
global custom_buffer_history_get_record_info_type *buffer_history_get_record_info = 0;
global custom_buffer_history_get_group_sub_record_type *buffer_history_get_group_sub_record = 0;
global custom_buffer_history_get_current_state_index_type *buffer_history_get_current_state_index = 0;
global custom_buffer_history_set_current_state_index_type *buffer_history_set_current_state_index = 0;
global custom_buffer_history_merge_record_range_type *buffer_history_merge_record_range = 0;
global custom_buffer_history_clear_after_current_state_type *buffer_history_clear_after_current_state = 0;
global custom_global_history_edit_group_begin_type *global_history_edit_group_begin = 0;
global custom_global_history_edit_group_end_type *global_history_edit_group_end = 0;
global custom_buffer_set_face_type *buffer_set_face = 0;
global custom_get_face_description_type *get_face_description = 0;
global custom_get_face_metrics_type *get_face_metrics = 0;
global custom_get_face_id_type *get_face_id = 0;
global custom_try_create_new_face_type *try_create_new_face = 0;
global custom_try_modify_face_type *try_modify_face = 0;
global custom_try_release_face_type *try_release_face = 0;
global custom_set_theme_colors_type *set_theme_colors = 0;
global custom_get_theme_colors_type *get_theme_colors = 0;
global custom_finalize_color_type *finalize_color = 0;
global custom_push_hot_directory_type *push_hot_directory = 0;
global custom_set_hot_directory_type *set_hot_directory = 0;
global custom_set_gui_up_down_keys_type *set_gui_up_down_keys = 0;
global custom_send_exit_signal_type *send_exit_signal = 0;
global custom_set_window_title_type *set_window_title = 0;
global custom_draw_string_oriented_type *draw_string_oriented = 0;
global custom_get_string_advance_type *get_string_advance = 0;
global custom_draw_rectangle_type *draw_rectangle = 0;
global custom_draw_rectangle_outline_type *draw_rectangle_outline = 0;
global custom_draw_set_clip_type *draw_set_clip = 0;
global custom_text_layout_create_type *text_layout_create = 0;
global custom_text_layout_region_type *text_layout_region = 0;
global custom_text_layout_get_buffer_type *text_layout_get_buffer = 0;
global custom_text_layout_get_visible_range_type *text_layout_get_visible_range = 0;
global custom_text_layout_line_on_screen_type *text_layout_line_on_screen = 0;
global custom_text_layout_character_on_screen_type *text_layout_character_on_screen = 0;
global custom_paint_text_color_type *paint_text_color = 0;
global custom_text_layout_free_type *text_layout_free = 0;
global custom_draw_text_layout_type *draw_text_layout = 0;
global custom_open_color_picker_type *open_color_picker = 0;
global custom_animate_in_n_milliseconds_type *animate_in_n_milliseconds = 0;
global custom_buffer_find_all_matches_type *buffer_find_all_matches = 0;
#undef DYNAMIC_LINK_API
#endif
