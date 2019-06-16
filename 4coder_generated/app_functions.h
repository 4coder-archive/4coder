struct Application_Links;
#define GLOBAL_SET_SETTING_SIG(n) b32 n(Application_Links *app, Global_Setting_ID setting, i32 value)
#define GLOBAL_SET_MAPPING_SIG(n) b32 n(Application_Links *app, void *data, i32 size)
#define GLOBAL_GET_SCREEN_RECTANGLE_SIG(n) b32 n(Application_Links *app, Rect_f32 *rect_out)
#define CONTEXT_GET_ARENA_SIG(n) Arena* n(Application_Links *app)
#define CONTEXT_GET_BASE_ALLOCATOR_SIG(n) Base_Allocator* n(Application_Links *app)
#define CREATE_CHILD_PROCESS_SIG(n) b32 n(Application_Links *app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *child_process_id_out)
#define CHILD_PROCESS_SET_TARGET_BUFFER_SIG(n) b32 n(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags)
#define BUFFER_GET_ATTACHED_CHILD_PROCESS_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Child_Process_ID *child_process_id_out)
#define CHILD_PROCESS_GET_ATTACHED_BUFFER_SIG(n) b32 n(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID *buffer_id_out)
#define CHILD_PROCESS_GET_STATE_SIG(n) b32 n(Application_Links *app, Child_Process_ID child_process_id, Process_State *process_state_out)
#define CLIPBOARD_POST_SIG(n) b32 n(Application_Links *app, i32 clipboard_id, String_Const_u8 string)
#define CLIPBOARD_COUNT_SIG(n) b32 n(Application_Links *app, i32 clipboard_id, i32 *count_out)
#define CLIPBOARD_INDEX_SIG(n) b32 n(Application_Links *app, i32 clipboard_id, i32 item_index, Arena *out, String_Const_u8 *string_out)
#define CREATE_PARSE_CONTEXT_SIG(n) Parse_Context_ID n(Application_Links *app, Parser_String_And_Type *kw, u32 kw_count, Parser_String_And_Type *pp, u32 pp_count)
#define GET_BUFFER_COUNT_SIG(n) i32 n(Application_Links *app)
#define GET_BUFFER_NEXT_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Access_Flag access, Buffer_ID *buffer_id_out)
#define GET_BUFFER_BY_NAME_SIG(n) b32 n(Application_Links *app, String_Const_u8 name, Access_Flag access, Buffer_ID *buffer_id_out)
#define GET_BUFFER_BY_FILE_NAME_SIG(n) b32 n(Application_Links *app, String_Const_u8 file_name, Access_Flag access, Buffer_ID *buffer_id_out)
#define BUFFER_READ_RANGE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, i32 start, i32 one_past_last, char *out)
#define BUFFER_REPLACE_RANGE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Range range, String_Const_u8 string)
#define BUFFER_BATCH_EDIT_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, char *str, Buffer_Edit *edits, i32 edit_count)
#define BUFFER_SEEK_STRING_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i32 start_pos, i32 *pos_out, b32 *case_sensitive_out)
#define BUFFER_SEEK_CHARACTER_CLASS_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Character_Predicate *predicate, Scan_Direction direction, i32 start_pos, i32 *pos_out)
#define BUFFER_COMPUTE_CURSOR_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Buffer_Seek seek, Partial_Cursor *cursor_out)
#define BUFFER_EXISTS_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id)
#define BUFFER_READY_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id)
#define BUFFER_GET_ACCESS_FLAGS_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Access_Flag *access_flags_out)
#define BUFFER_GET_SIZE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, i32 *size_out)
#define BUFFER_GET_LINE_COUNT_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, i32 *line_count_out)
#define BUFFER_GET_BASE_BUFFER_NAME_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out)
#define BUFFER_GET_UNIQUE_BUFFER_NAME_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out)
#define BUFFER_GET_FILE_NAME_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out)
#define BUFFER_GET_DIRTY_STATE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Dirty_State *dirty_state_out)
#define BUFFER_DIRECTLY_SET_DIRTY_STATE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Dirty_State dirty_state)
#define BUFFER_TOKENS_ARE_READY_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id)
#define BUFFER_GET_SETTING_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 *value_out)
#define BUFFER_SET_SETTING_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 value)
#define BUFFER_GET_MANAGED_SCOPE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Managed_Scope *scope_out)
#define BUFFER_TOKEN_COUNT_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, i32 *count_out)
#define BUFFER_READ_TOKENS_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, i32 start_token, i32 end_token, Cpp_Token *tokens_out)
#define BUFFER_GET_TOKEN_RANGE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out)
#define BUFFER_GET_TOKEN_INDEX_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, i32 pos, Cpp_Get_Token_Result *get_result)
#define BUFFER_SEND_END_SIGNAL_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id)
#define CREATE_BUFFER_SIG(n) b32 n(Application_Links *app, String_Const_u8 file_name, Buffer_Create_Flag flags, Buffer_ID *new_buffer_id_out)
#define BUFFER_SAVE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, String_Const_u8 file_name, u32 flags)
#define BUFFER_KILL_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Buffer_Kill_Flag flags, Buffer_Kill_Result *result_out)
#define BUFFER_REOPEN_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags, Buffer_Reopen_Result *result_out)
#define BUFFER_GET_FILE_ATTRIBUTES_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, File_Attributes *attributes_out)
#define GET_VIEW_NEXT_SIG(n) b32 n(Application_Links *app, View_ID view_id, Access_Flag access, View_ID *view_id_out)
#define GET_VIEW_PREV_SIG(n) b32 n(Application_Links *app, View_ID view_id, Access_Flag access, View_ID *view_id_out)
#define GET_ACTIVE_VIEW_SIG(n) b32 n(Application_Links *app, Access_Flag access, View_ID *view_id_out)
#define GET_ACTIVE_PANEL_SIG(n) b32 n(Application_Links *app, Panel_ID *panel_id_out)
#define VIEW_EXISTS_SIG(n) b32 n(Application_Links *app, View_ID view_id)
#define VIEW_GET_BUFFER_SIG(n) b32 n(Application_Links *app, View_ID view_id, Access_Flag access, Buffer_ID *buffer_id_out)
#define VIEW_GET_CURSOR_POS_SIG(n) b32 n(Application_Links *app, View_ID view_id, i32 *pos_out)
#define VIEW_GET_MARK_POS_SIG(n) b32 n(Application_Links *app, View_ID view_id, i32 *pos_out)
#define VIEW_GET_PREFERRED_X_SIG(n) b32 n(Application_Links *app, View_ID view_id, f32 *preferred_x_out)
#define VIEW_GET_SCREEN_RECT_SIG(n) b32 n(Application_Links *app, View_ID view_id, Rect_f32 *rect_out)
#define VIEW_GET_PANEL_SIG(n) b32 n(Application_Links *app, View_ID view_id, Panel_ID *panel_id_out)
#define PANEL_GET_VIEW_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, View_ID *view_id_out)
#define PANEL_IS_SPLIT_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id)
#define PANEL_IS_LEAF_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id)
#define PANEL_SPLIT_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, Panel_Split_Orientation orientation)
#define PANEL_SET_SPLIT_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t)
#define PANEL_SWAP_CHILDREN_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t)
#define PANEL_GET_PARENT_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, Panel_ID *panel_id_out)
#define PANEL_GET_CHILD_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, Panel_Child which_child, Panel_ID *panel_id_out)
#define PANEL_GET_MAX_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, Panel_ID *panel_id_out)
#define PANEL_GET_MARGIN_SIG(n) b32 n(Application_Links *app, Panel_ID panel_id, i32_Rect *margins_out)
#define VIEW_CLOSE_SIG(n) b32 n(Application_Links *app, View_ID view_id)
#define VIEW_GET_REGION_SIG(n) b32 n(Application_Links *app, View_ID view_id, Rect_i32 *region_out)
#define VIEW_GET_BUFFER_REGION_SIG(n) b32 n(Application_Links *app, View_ID view_id, Rect_i32 *region_out)
#define VIEW_GET_SCROLL_VARS_SIG(n) b32 n(Application_Links *app, View_ID view_id, GUI_Scroll_Vars *scroll_vars_out)
#define VIEW_SET_ACTIVE_SIG(n) b32 n(Application_Links *app, View_ID view_id)
#define VIEW_GET_SETTING_SIG(n) b32 n(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 *value_out)
#define VIEW_SET_SETTING_SIG(n) b32 n(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 value)
#define VIEW_GET_MANAGED_SCOPE_SIG(n) b32 n(Application_Links *app, View_ID view_id, Managed_Scope *scope)
#define VIEW_COMPUTE_CURSOR_SIG(n) b32 n(Application_Links *app, View_ID view_id, Buffer_Seek seek, Full_Cursor *cursor_out)
#define VIEW_SET_CURSOR_SIG(n) b32 n(Application_Links *app, View_ID view_id, Buffer_Seek seek, b32 set_preferred_x)
#define VIEW_SET_SCROLL_SIG(n) b32 n(Application_Links *app, View_ID view_id, GUI_Scroll_Vars scroll)
#define VIEW_SET_MARK_SIG(n) b32 n(Application_Links *app, View_ID view_id, Buffer_Seek seek)
#define VIEW_SET_BUFFER_SIG(n) b32 n(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags)
#define VIEW_POST_FADE_SIG(n) b32 n(Application_Links *app, View_ID view_id, float seconds, i32 start, i32 end, int_color color)
#define VIEW_BEGIN_UI_MODE_SIG(n) b32 n(Application_Links *app, View_ID view_id)
#define VIEW_END_UI_MODE_SIG(n) b32 n(Application_Links *app, View_ID view_id)
#define VIEW_IS_IN_UI_MODE_SIG(n) b32 n(Application_Links *app, View_ID view_id)
#define VIEW_SET_QUIT_UI_HANDLER_SIG(n) b32 n(Application_Links *app, View_ID view_id, UI_Quit_Function_Type *quit_function)
#define VIEW_GET_QUIT_UI_HANDLER_SIG(n) b32 n(Application_Links *app, View_ID view_id, UI_Quit_Function_Type **quit_function_out)
#define CREATE_USER_MANAGED_SCOPE_SIG(n) Managed_Scope n(Application_Links *app)
#define DESTROY_USER_MANAGED_SCOPE_SIG(n) b32 n(Application_Links *app, Managed_Scope scope)
#define GET_GLOBAL_MANAGED_SCOPE_SIG(n) Managed_Scope n(Application_Links *app)
#define GET_MANAGED_SCOPE_WITH_MULTIPLE_DEPENDENCIES_SIG(n) Managed_Scope n(Application_Links *app, Managed_Scope *scopes, i32 count)
#define MANAGED_SCOPE_CLEAR_CONTENTS_SIG(n) b32 n(Application_Links *app, Managed_Scope scope)
#define MANAGED_SCOPE_CLEAR_SELF_ALL_DEPENDENT_SCOPES_SIG(n) b32 n(Application_Links *app, Managed_Scope scope)
#define MANAGED_VARIABLE_CREATE_SIG(n) Managed_Variable_ID n(Application_Links *app, char *null_terminated_name, u64 default_value)
#define MANAGED_VARIABLE_GET_ID_SIG(n) Managed_Variable_ID n(Application_Links *app, char *null_terminated_name)
#define MANAGED_VARIABLE_CREATE_OR_GET_ID_SIG(n) Managed_Variable_ID n(Application_Links *app, char *null_terminated_name, u64 default_value)
#define MANAGED_VARIABLE_SET_SIG(n) b32 n(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 value)
#define MANAGED_VARIABLE_GET_SIG(n) b32 n(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 *value_out)
#define ALLOC_MANAGED_MEMORY_IN_SCOPE_SIG(n) Managed_Object n(Application_Links *app, Managed_Scope scope, i32 item_size, i32 count)
#define ALLOC_BUFFER_MARKERS_ON_BUFFER_SIG(n) Managed_Object n(Application_Links *app, Buffer_ID buffer_id, i32 count, Managed_Scope *optional_extra_scope)
#define ALLOC_MANAGED_ARENA_IN_SCOPE_SIG(n) Managed_Object n(Application_Links *app, Managed_Scope scope, i32 page_size)
#define CREATE_MARKER_VISUAL_SIG(n) Marker_Visual n(Application_Links *app, Managed_Object object)
#define MARKER_VISUAL_SET_EFFECT_SIG(n) b32 n(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style)
#define MARKER_VISUAL_SET_TAKE_RULE_SIG(n) b32 n(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule)
#define MARKER_VISUAL_SET_PRIORITY_SIG(n) b32 n(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority)
#define MARKER_VISUAL_SET_VIEW_KEY_SIG(n) b32 n(Application_Links *app, Marker_Visual visual, View_ID key_view_id)
#define DESTROY_MARKER_VISUAL_SIG(n) b32 n(Application_Links *app, Marker_Visual visual)
#define BUFFER_MARKERS_GET_ATTACHED_VISUAL_COUNT_SIG(n) i32 n(Application_Links *app, Managed_Object object)
#define BUFFER_MARKERS_GET_ATTACHED_VISUAL_SIG(n) Marker_Visual* n(Application_Links *app, Arena *arena, Managed_Object object)
#define MANAGED_OBJECT_GET_ITEM_SIZE_SIG(n) u32 n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_GET_ITEM_COUNT_SIG(n) u32 n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_GET_TYPE_SIG(n) Managed_Object_Type n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_GET_CONTAINING_SCOPE_SIG(n) Managed_Scope n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_FREE_SIG(n) b32 n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_STORE_DATA_SIG(n) b32 n(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem)
#define MANAGED_OBJECT_LOAD_DATA_SIG(n) b32 n(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem_out)
#define GET_USER_INPUT_SIG(n) User_Input n(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
#define GET_COMMAND_INPUT_SIG(n) User_Input n(Application_Links *app)
#define SET_COMMAND_INPUT_SIG(n) void n(Application_Links *app, Key_Event_Data key_data)
#define GET_MOUSE_STATE_SIG(n) Mouse_State n(Application_Links *app)
#define GET_ACTIVE_QUERY_BARS_SIG(n) b32 n(Application_Links *app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array *array_out)
#define START_QUERY_BAR_SIG(n) b32 n(Application_Links *app, Query_Bar *bar, u32 flags)
#define END_QUERY_BAR_SIG(n) void n(Application_Links *app, Query_Bar *bar, u32 flags)
#define PRINT_MESSAGE_SIG(n) b32 n(Application_Links *app, String_Const_u8 message)
#define GET_LARGEST_FACE_ID_SIG(n) Face_ID n(Application_Links *app)
#define SET_GLOBAL_FACE_SIG(n) b32 n(Application_Links *app, Face_ID id, b32 apply_to_all_buffers)
#define BUFFER_HISTORY_GET_MAX_RECORD_INDEX_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, History_Record_Index *index_out)
#define BUFFER_HISTORY_GET_RECORD_INFO_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, Record_Info *record_out)
#define BUFFER_HISTORY_GET_GROUP_SUB_RECORD_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index, Record_Info *record_out)
#define BUFFER_HISTORY_GET_CURRENT_STATE_INDEX_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, History_Record_Index *index_out)
#define BUFFER_HISTORY_SET_CURRENT_STATE_INDEX_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index)
#define BUFFER_HISTORY_MERGE_RECORD_RANGE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags)
#define BUFFER_HISTORY_CLEAR_AFTER_CURRENT_STATE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id)
#define GLOBAL_HISTORY_EDIT_GROUP_BEGIN_SIG(n) void n(Application_Links *app)
#define GLOBAL_HISTORY_EDIT_GROUP_END_SIG(n) void n(Application_Links *app)
#define BUFFER_SET_FACE_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Face_ID id)
#define GET_FACE_DESCRIPTION_SIG(n) Face_Description n(Application_Links *app, Face_ID id)
#define GET_FACE_METRICS_SIG(n) b32 n(Application_Links *app, Face_ID face_id, Face_Metrics *metrics_out)
#define GET_FACE_ID_SIG(n) b32 n(Application_Links *app, Buffer_ID buffer_id, Face_ID *face_id_out)
#define TRY_CREATE_NEW_FACE_SIG(n) Face_ID n(Application_Links *app, Face_Description *description)
#define TRY_MODIFY_FACE_SIG(n) b32 n(Application_Links *app, Face_ID id, Face_Description *description)
#define TRY_RELEASE_FACE_SIG(n) b32 n(Application_Links *app, Face_ID id, Face_ID replacement_id)
#define GET_AVAILABLE_FONT_COUNT_SIG(n) i32 n(Application_Links *app)
#define GET_AVAILABLE_FONT_SIG(n) Available_Font n(Application_Links *app, i32 index)
#define SET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, i32 count)
#define GET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, i32 count)
#define FINALIZE_COLOR_SIG(n) argb_color n(Application_Links *app, int_color color)
#define GET_HOT_DIRECTORY_SIG(n) b32 n(Application_Links *app, Arena *out, String_Const_u8 *out_directory)
#define SET_HOT_DIRECTORY_SIG(n) b32 n(Application_Links *app, String_Const_u8 string)
#define GET_FILE_LIST_SIG(n) b32 n(Application_Links *app, String_Const_u8 directory, File_List *list_out)
#define FREE_FILE_LIST_SIG(n) void n(Application_Links *app, File_List list)
#define SET_GUI_UP_DOWN_KEYS_SIG(n) void n(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier)
#define MEMORY_ALLOCATE_SIG(n) void* n(Application_Links *app, i32 size)
#define MEMORY_SET_PROTECTION_SIG(n) b32 n(Application_Links *app, void *ptr, i32 size, Memory_Protect_Flags flags)
#define MEMORY_FREE_SIG(n) void n(Application_Links *app, void *ptr, i32 size)
#define FILE_GET_ATTRIBUTES_SIG(n) b32 n(Application_Links *app, String_Const_u8 file_name, File_Attributes *attributes_out)
#define GET_4ED_PATH_SIG(n) b32 n(Application_Links *app, Arena *out, String_Const_u8 *path_out)
#define SHOW_MOUSE_CURSOR_SIG(n) void n(Application_Links *app, Mouse_Cursor_Show_Type show)
#define SET_EDIT_FINISHED_HOOK_REPEAT_SPEED_SIG(n) b32 n(Application_Links *app, u32 milliseconds)
#define SET_FULLSCREEN_SIG(n) b32 n(Application_Links *app, b32 full_screen)
#define IS_FULLSCREEN_SIG(n) b32 n(Application_Links *app)
#define SEND_EXIT_SIGNAL_SIG(n) void n(Application_Links *app)
#define SET_WINDOW_TITLE_SIG(n) b32 n(Application_Links *app, String_Const_u8 title)
#define GET_MICROSECONDS_TIMESTAMP_SIG(n) Microsecond_Time_Stamp n(Application_Links *app)
#define DRAW_STRING_SIG(n) Vec2 n(Application_Links *app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta)
#define GET_STRING_ADVANCE_SIG(n) f32 n(Application_Links *app, Face_ID font_id, String_Const_u8 str)
#define DRAW_RECTANGLE_SIG(n) void n(Application_Links *app, Rect_f32 rect, int_color color)
#define DRAW_RECTANGLE_OUTLINE_SIG(n) void n(Application_Links *app, f32_Rect rect, int_color color)
#define DRAW_CLIP_PUSH_SIG(n) void n(Application_Links *app, f32_Rect clip_box)
#define DRAW_CLIP_POP_SIG(n) f32_Rect n(Application_Links *app)
#define DRAW_COORDINATE_CENTER_PUSH_SIG(n) void n(Application_Links *app, Vec2 point)
#define DRAW_COORDINATE_CENTER_POP_SIG(n) Vec2 n(Application_Links *app)
#define TEXT_LAYOUT_GET_BUFFER_SIG(n) b32 n(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID *buffer_id_out)
#define TEXT_LAYOUT_BUFFER_POINT_TO_LAYOUT_POINT_SIG(n) b32 n(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 buffer_relative_p, Vec2 *p_out)
#define TEXT_LAYOUT_LAYOUT_POINT_TO_BUFFER_POINT_SIG(n) b32 n(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 layout_relative_p, Vec2 *p_out)
#define TEXT_LAYOUT_GET_ON_SCREEN_RANGE_SIG(n) b32 n(Application_Links *app, Text_Layout_ID text_layout_id, Range *on_screen_range_out)
#define TEXT_LAYOUT_GET_HEIGHT_SIG(n) b32 n(Application_Links *app, Text_Layout_ID text_layout_id, f32 *height_out)
#define TEXT_LAYOUT_FREE_SIG(n) b32 n(Application_Links *app, Text_Layout_ID text_layout_id)
#define COMPUTE_RENDER_LAYOUT_SIG(n) b32 n(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Vec2 screen_p, Vec2 layout_dim, Buffer_Point buffer_point, i32 one_past_last, Text_Layout_ID *text_layout_id_out)
#define DRAW_RENDER_LAYOUT_SIG(n) void n(Application_Links *app, View_ID view_id)
#define OPEN_COLOR_PICKER_SIG(n) void n(Application_Links *app, color_picker *picker)
#define ANIMATE_IN_N_MILLISECONDS_SIG(n) void n(Application_Links *app, u32 n)
#define FIND_ALL_MATCHES_BUFFER_RANGE_SIG(n) String_Match_List n(Application_Links *app, Arena *arena, Buffer_ID buffer, Range range, String_Const_u8 needle, Character_Predicate *predicate, Scan_Direction direction)
#define GET_VIEW_VISIBLE_RANGE_SIG(n) Range n(Application_Links *app, View_ID view_id)
typedef GLOBAL_SET_SETTING_SIG(Global_Set_Setting_Function);
typedef GLOBAL_SET_MAPPING_SIG(Global_Set_Mapping_Function);
typedef GLOBAL_GET_SCREEN_RECTANGLE_SIG(Global_Get_Screen_Rectangle_Function);
typedef CONTEXT_GET_ARENA_SIG(Context_Get_Arena_Function);
typedef CONTEXT_GET_BASE_ALLOCATOR_SIG(Context_Get_Base_Allocator_Function);
typedef CREATE_CHILD_PROCESS_SIG(Create_Child_Process_Function);
typedef CHILD_PROCESS_SET_TARGET_BUFFER_SIG(Child_Process_Set_Target_Buffer_Function);
typedef BUFFER_GET_ATTACHED_CHILD_PROCESS_SIG(Buffer_Get_Attached_Child_Process_Function);
typedef CHILD_PROCESS_GET_ATTACHED_BUFFER_SIG(Child_Process_Get_Attached_Buffer_Function);
typedef CHILD_PROCESS_GET_STATE_SIG(Child_Process_Get_State_Function);
typedef CLIPBOARD_POST_SIG(Clipboard_Post_Function);
typedef CLIPBOARD_COUNT_SIG(Clipboard_Count_Function);
typedef CLIPBOARD_INDEX_SIG(Clipboard_Index_Function);
typedef CREATE_PARSE_CONTEXT_SIG(Create_Parse_Context_Function);
typedef GET_BUFFER_COUNT_SIG(Get_Buffer_Count_Function);
typedef GET_BUFFER_NEXT_SIG(Get_Buffer_Next_Function);
typedef GET_BUFFER_BY_NAME_SIG(Get_Buffer_By_Name_Function);
typedef GET_BUFFER_BY_FILE_NAME_SIG(Get_Buffer_By_File_Name_Function);
typedef BUFFER_READ_RANGE_SIG(Buffer_Read_Range_Function);
typedef BUFFER_REPLACE_RANGE_SIG(Buffer_Replace_Range_Function);
typedef BUFFER_BATCH_EDIT_SIG(Buffer_Batch_Edit_Function);
typedef BUFFER_SEEK_STRING_SIG(Buffer_Seek_String_Function);
typedef BUFFER_SEEK_CHARACTER_CLASS_SIG(Buffer_Seek_Character_Class_Function);
typedef BUFFER_COMPUTE_CURSOR_SIG(Buffer_Compute_Cursor_Function);
typedef BUFFER_EXISTS_SIG(Buffer_Exists_Function);
typedef BUFFER_READY_SIG(Buffer_Ready_Function);
typedef BUFFER_GET_ACCESS_FLAGS_SIG(Buffer_Get_Access_Flags_Function);
typedef BUFFER_GET_SIZE_SIG(Buffer_Get_Size_Function);
typedef BUFFER_GET_LINE_COUNT_SIG(Buffer_Get_Line_Count_Function);
typedef BUFFER_GET_BASE_BUFFER_NAME_SIG(Buffer_Get_Base_Buffer_Name_Function);
typedef BUFFER_GET_UNIQUE_BUFFER_NAME_SIG(Buffer_Get_Unique_Buffer_Name_Function);
typedef BUFFER_GET_FILE_NAME_SIG(Buffer_Get_File_Name_Function);
typedef BUFFER_GET_DIRTY_STATE_SIG(Buffer_Get_Dirty_State_Function);
typedef BUFFER_DIRECTLY_SET_DIRTY_STATE_SIG(Buffer_Directly_Set_Dirty_State_Function);
typedef BUFFER_TOKENS_ARE_READY_SIG(Buffer_Tokens_Are_Ready_Function);
typedef BUFFER_GET_SETTING_SIG(Buffer_Get_Setting_Function);
typedef BUFFER_SET_SETTING_SIG(Buffer_Set_Setting_Function);
typedef BUFFER_GET_MANAGED_SCOPE_SIG(Buffer_Get_Managed_Scope_Function);
typedef BUFFER_TOKEN_COUNT_SIG(Buffer_Token_Count_Function);
typedef BUFFER_READ_TOKENS_SIG(Buffer_Read_Tokens_Function);
typedef BUFFER_GET_TOKEN_RANGE_SIG(Buffer_Get_Token_Range_Function);
typedef BUFFER_GET_TOKEN_INDEX_SIG(Buffer_Get_Token_Index_Function);
typedef BUFFER_SEND_END_SIGNAL_SIG(Buffer_Send_End_Signal_Function);
typedef CREATE_BUFFER_SIG(Create_Buffer_Function);
typedef BUFFER_SAVE_SIG(Buffer_Save_Function);
typedef BUFFER_KILL_SIG(Buffer_Kill_Function);
typedef BUFFER_REOPEN_SIG(Buffer_Reopen_Function);
typedef BUFFER_GET_FILE_ATTRIBUTES_SIG(Buffer_Get_File_Attributes_Function);
typedef GET_VIEW_NEXT_SIG(Get_View_Next_Function);
typedef GET_VIEW_PREV_SIG(Get_View_Prev_Function);
typedef GET_ACTIVE_VIEW_SIG(Get_Active_View_Function);
typedef GET_ACTIVE_PANEL_SIG(Get_Active_Panel_Function);
typedef VIEW_EXISTS_SIG(View_Exists_Function);
typedef VIEW_GET_BUFFER_SIG(View_Get_Buffer_Function);
typedef VIEW_GET_CURSOR_POS_SIG(View_Get_Cursor_Pos_Function);
typedef VIEW_GET_MARK_POS_SIG(View_Get_Mark_Pos_Function);
typedef VIEW_GET_PREFERRED_X_SIG(View_Get_Preferred_X_Function);
typedef VIEW_GET_SCREEN_RECT_SIG(View_Get_Screen_Rect_Function);
typedef VIEW_GET_PANEL_SIG(View_Get_Panel_Function);
typedef PANEL_GET_VIEW_SIG(Panel_Get_View_Function);
typedef PANEL_IS_SPLIT_SIG(Panel_Is_Split_Function);
typedef PANEL_IS_LEAF_SIG(Panel_Is_Leaf_Function);
typedef PANEL_SPLIT_SIG(Panel_Split_Function);
typedef PANEL_SET_SPLIT_SIG(Panel_Set_Split_Function);
typedef PANEL_SWAP_CHILDREN_SIG(Panel_Swap_Children_Function);
typedef PANEL_GET_PARENT_SIG(Panel_Get_Parent_Function);
typedef PANEL_GET_CHILD_SIG(Panel_Get_Child_Function);
typedef PANEL_GET_MAX_SIG(Panel_Get_Max_Function);
typedef PANEL_GET_MARGIN_SIG(Panel_Get_Margin_Function);
typedef VIEW_CLOSE_SIG(View_Close_Function);
typedef VIEW_GET_REGION_SIG(View_Get_Region_Function);
typedef VIEW_GET_BUFFER_REGION_SIG(View_Get_Buffer_Region_Function);
typedef VIEW_GET_SCROLL_VARS_SIG(View_Get_Scroll_Vars_Function);
typedef VIEW_SET_ACTIVE_SIG(View_Set_Active_Function);
typedef VIEW_GET_SETTING_SIG(View_Get_Setting_Function);
typedef VIEW_SET_SETTING_SIG(View_Set_Setting_Function);
typedef VIEW_GET_MANAGED_SCOPE_SIG(View_Get_Managed_Scope_Function);
typedef VIEW_COMPUTE_CURSOR_SIG(View_Compute_Cursor_Function);
typedef VIEW_SET_CURSOR_SIG(View_Set_Cursor_Function);
typedef VIEW_SET_SCROLL_SIG(View_Set_Scroll_Function);
typedef VIEW_SET_MARK_SIG(View_Set_Mark_Function);
typedef VIEW_SET_BUFFER_SIG(View_Set_Buffer_Function);
typedef VIEW_POST_FADE_SIG(View_Post_Fade_Function);
typedef VIEW_BEGIN_UI_MODE_SIG(View_Begin_UI_Mode_Function);
typedef VIEW_END_UI_MODE_SIG(View_End_UI_Mode_Function);
typedef VIEW_IS_IN_UI_MODE_SIG(View_Is_In_UI_Mode_Function);
typedef VIEW_SET_QUIT_UI_HANDLER_SIG(View_Set_Quit_UI_Handler_Function);
typedef VIEW_GET_QUIT_UI_HANDLER_SIG(View_Get_Quit_UI_Handler_Function);
typedef CREATE_USER_MANAGED_SCOPE_SIG(Create_User_Managed_Scope_Function);
typedef DESTROY_USER_MANAGED_SCOPE_SIG(Destroy_User_Managed_Scope_Function);
typedef GET_GLOBAL_MANAGED_SCOPE_SIG(Get_Global_Managed_Scope_Function);
typedef GET_MANAGED_SCOPE_WITH_MULTIPLE_DEPENDENCIES_SIG(Get_Managed_Scope_With_Multiple_Dependencies_Function);
typedef MANAGED_SCOPE_CLEAR_CONTENTS_SIG(Managed_Scope_Clear_Contents_Function);
typedef MANAGED_SCOPE_CLEAR_SELF_ALL_DEPENDENT_SCOPES_SIG(Managed_Scope_Clear_Self_All_Dependent_Scopes_Function);
typedef MANAGED_VARIABLE_CREATE_SIG(Managed_Variable_Create_Function);
typedef MANAGED_VARIABLE_GET_ID_SIG(Managed_Variable_Get_ID_Function);
typedef MANAGED_VARIABLE_CREATE_OR_GET_ID_SIG(Managed_Variable_Create_Or_Get_ID_Function);
typedef MANAGED_VARIABLE_SET_SIG(Managed_Variable_Set_Function);
typedef MANAGED_VARIABLE_GET_SIG(Managed_Variable_Get_Function);
typedef ALLOC_MANAGED_MEMORY_IN_SCOPE_SIG(Alloc_Managed_Memory_In_Scope_Function);
typedef ALLOC_BUFFER_MARKERS_ON_BUFFER_SIG(Alloc_Buffer_Markers_On_Buffer_Function);
typedef ALLOC_MANAGED_ARENA_IN_SCOPE_SIG(Alloc_Managed_Arena_In_Scope_Function);
typedef CREATE_MARKER_VISUAL_SIG(Create_Marker_Visual_Function);
typedef MARKER_VISUAL_SET_EFFECT_SIG(Marker_Visual_Set_Effect_Function);
typedef MARKER_VISUAL_SET_TAKE_RULE_SIG(Marker_Visual_Set_Take_Rule_Function);
typedef MARKER_VISUAL_SET_PRIORITY_SIG(Marker_Visual_Set_Priority_Function);
typedef MARKER_VISUAL_SET_VIEW_KEY_SIG(Marker_Visual_Set_View_Key_Function);
typedef DESTROY_MARKER_VISUAL_SIG(Destroy_Marker_Visual_Function);
typedef BUFFER_MARKERS_GET_ATTACHED_VISUAL_COUNT_SIG(Buffer_Markers_Get_Attached_Visual_Count_Function);
typedef BUFFER_MARKERS_GET_ATTACHED_VISUAL_SIG(Buffer_Markers_Get_Attached_Visual_Function);
typedef MANAGED_OBJECT_GET_ITEM_SIZE_SIG(Managed_Object_Get_Item_Size_Function);
typedef MANAGED_OBJECT_GET_ITEM_COUNT_SIG(Managed_Object_Get_Item_Count_Function);
typedef MANAGED_OBJECT_GET_TYPE_SIG(Managed_Object_Get_Type_Function);
typedef MANAGED_OBJECT_GET_CONTAINING_SCOPE_SIG(Managed_Object_Get_Containing_Scope_Function);
typedef MANAGED_OBJECT_FREE_SIG(Managed_Object_Free_Function);
typedef MANAGED_OBJECT_STORE_DATA_SIG(Managed_Object_Store_Data_Function);
typedef MANAGED_OBJECT_LOAD_DATA_SIG(Managed_Object_Load_Data_Function);
typedef GET_USER_INPUT_SIG(Get_User_Input_Function);
typedef GET_COMMAND_INPUT_SIG(Get_Command_Input_Function);
typedef SET_COMMAND_INPUT_SIG(Set_Command_Input_Function);
typedef GET_MOUSE_STATE_SIG(Get_Mouse_State_Function);
typedef GET_ACTIVE_QUERY_BARS_SIG(Get_Active_Query_Bars_Function);
typedef START_QUERY_BAR_SIG(Start_Query_Bar_Function);
typedef END_QUERY_BAR_SIG(End_Query_Bar_Function);
typedef PRINT_MESSAGE_SIG(Print_Message_Function);
typedef GET_LARGEST_FACE_ID_SIG(Get_Largest_Face_ID_Function);
typedef SET_GLOBAL_FACE_SIG(Set_Global_Face_Function);
typedef BUFFER_HISTORY_GET_MAX_RECORD_INDEX_SIG(Buffer_History_Get_Max_Record_Index_Function);
typedef BUFFER_HISTORY_GET_RECORD_INFO_SIG(Buffer_History_Get_Record_Info_Function);
typedef BUFFER_HISTORY_GET_GROUP_SUB_RECORD_SIG(Buffer_History_Get_Group_Sub_Record_Function);
typedef BUFFER_HISTORY_GET_CURRENT_STATE_INDEX_SIG(Buffer_History_Get_Current_State_Index_Function);
typedef BUFFER_HISTORY_SET_CURRENT_STATE_INDEX_SIG(Buffer_History_Set_Current_State_Index_Function);
typedef BUFFER_HISTORY_MERGE_RECORD_RANGE_SIG(Buffer_History_Merge_Record_Range_Function);
typedef BUFFER_HISTORY_CLEAR_AFTER_CURRENT_STATE_SIG(Buffer_History_Clear_After_Current_State_Function);
typedef GLOBAL_HISTORY_EDIT_GROUP_BEGIN_SIG(Global_History_Edit_Group_Begin_Function);
typedef GLOBAL_HISTORY_EDIT_GROUP_END_SIG(Global_History_Edit_Group_End_Function);
typedef BUFFER_SET_FACE_SIG(Buffer_Set_Face_Function);
typedef GET_FACE_DESCRIPTION_SIG(Get_Face_Description_Function);
typedef GET_FACE_METRICS_SIG(Get_Face_Metrics_Function);
typedef GET_FACE_ID_SIG(Get_Face_ID_Function);
typedef TRY_CREATE_NEW_FACE_SIG(Try_Create_New_Face_Function);
typedef TRY_MODIFY_FACE_SIG(Try_Modify_Face_Function);
typedef TRY_RELEASE_FACE_SIG(Try_Release_Face_Function);
typedef GET_AVAILABLE_FONT_COUNT_SIG(Get_Available_Font_Count_Function);
typedef GET_AVAILABLE_FONT_SIG(Get_Available_Font_Function);
typedef SET_THEME_COLORS_SIG(Set_Theme_Colors_Function);
typedef GET_THEME_COLORS_SIG(Get_Theme_Colors_Function);
typedef FINALIZE_COLOR_SIG(Finalize_Color_Function);
typedef GET_HOT_DIRECTORY_SIG(Get_Hot_Directory_Function);
typedef SET_HOT_DIRECTORY_SIG(Set_Hot_Directory_Function);
typedef GET_FILE_LIST_SIG(Get_File_List_Function);
typedef FREE_FILE_LIST_SIG(Free_File_List_Function);
typedef SET_GUI_UP_DOWN_KEYS_SIG(Set_GUI_Up_Down_Keys_Function);
typedef MEMORY_ALLOCATE_SIG(Memory_Allocate_Function);
typedef MEMORY_SET_PROTECTION_SIG(Memory_Set_Protection_Function);
typedef MEMORY_FREE_SIG(Memory_Free_Function);
typedef FILE_GET_ATTRIBUTES_SIG(File_Get_Attributes_Function);
typedef GET_4ED_PATH_SIG(Get_4ed_Path_Function);
typedef SHOW_MOUSE_CURSOR_SIG(Show_Mouse_Cursor_Function);
typedef SET_EDIT_FINISHED_HOOK_REPEAT_SPEED_SIG(Set_Edit_Finished_Hook_Repeat_Speed_Function);
typedef SET_FULLSCREEN_SIG(Set_Fullscreen_Function);
typedef IS_FULLSCREEN_SIG(Is_Fullscreen_Function);
typedef SEND_EXIT_SIGNAL_SIG(Send_Exit_Signal_Function);
typedef SET_WINDOW_TITLE_SIG(Set_Window_Title_Function);
typedef GET_MICROSECONDS_TIMESTAMP_SIG(Get_Microseconds_Timestamp_Function);
typedef DRAW_STRING_SIG(Draw_String_Function);
typedef GET_STRING_ADVANCE_SIG(Get_String_Advance_Function);
typedef DRAW_RECTANGLE_SIG(Draw_Rectangle_Function);
typedef DRAW_RECTANGLE_OUTLINE_SIG(Draw_Rectangle_Outline_Function);
typedef DRAW_CLIP_PUSH_SIG(Draw_Clip_Push_Function);
typedef DRAW_CLIP_POP_SIG(Draw_Clip_Pop_Function);
typedef DRAW_COORDINATE_CENTER_PUSH_SIG(Draw_Coordinate_Center_Push_Function);
typedef DRAW_COORDINATE_CENTER_POP_SIG(Draw_Coordinate_Center_Pop_Function);
typedef TEXT_LAYOUT_GET_BUFFER_SIG(Text_Layout_Get_Buffer_Function);
typedef TEXT_LAYOUT_BUFFER_POINT_TO_LAYOUT_POINT_SIG(Text_Layout_Buffer_Point_To_Layout_Point_Function);
typedef TEXT_LAYOUT_LAYOUT_POINT_TO_BUFFER_POINT_SIG(Text_Layout_Layout_Point_To_Buffer_Point_Function);
typedef TEXT_LAYOUT_GET_ON_SCREEN_RANGE_SIG(Text_Layout_Get_On_Screen_Range_Function);
typedef TEXT_LAYOUT_GET_HEIGHT_SIG(Text_Layout_Get_Height_Function);
typedef TEXT_LAYOUT_FREE_SIG(Text_Layout_Free_Function);
typedef COMPUTE_RENDER_LAYOUT_SIG(Compute_Render_Layout_Function);
typedef DRAW_RENDER_LAYOUT_SIG(Draw_Render_Layout_Function);
typedef OPEN_COLOR_PICKER_SIG(Open_Color_Picker_Function);
typedef ANIMATE_IN_N_MILLISECONDS_SIG(Animate_In_N_Milliseconds_Function);
typedef FIND_ALL_MATCHES_BUFFER_RANGE_SIG(Find_All_Matches_Buffer_Range_Function);
typedef GET_VIEW_VISIBLE_RANGE_SIG(Get_View_Visible_Range_Function);
struct Application_Links{
#if defined(ALLOW_DEP_4CODER)
Global_Set_Setting_Function *global_set_setting;
Global_Set_Mapping_Function *global_set_mapping;
Global_Get_Screen_Rectangle_Function *global_get_screen_rectangle;
Context_Get_Arena_Function *context_get_arena;
Context_Get_Base_Allocator_Function *context_get_base_allocator;
Create_Child_Process_Function *create_child_process;
Child_Process_Set_Target_Buffer_Function *child_process_set_target_buffer;
Buffer_Get_Attached_Child_Process_Function *buffer_get_attached_child_process;
Child_Process_Get_Attached_Buffer_Function *child_process_get_attached_buffer;
Child_Process_Get_State_Function *child_process_get_state;
Clipboard_Post_Function *clipboard_post;
Clipboard_Count_Function *clipboard_count;
Clipboard_Index_Function *clipboard_index;
Create_Parse_Context_Function *create_parse_context;
Get_Buffer_Count_Function *get_buffer_count;
Get_Buffer_Next_Function *get_buffer_next;
Get_Buffer_By_Name_Function *get_buffer_by_name;
Get_Buffer_By_File_Name_Function *get_buffer_by_file_name;
Buffer_Read_Range_Function *buffer_read_range;
Buffer_Replace_Range_Function *buffer_replace_range;
Buffer_Batch_Edit_Function *buffer_batch_edit;
Buffer_Seek_String_Function *buffer_seek_string;
Buffer_Seek_Character_Class_Function *buffer_seek_character_class;
Buffer_Compute_Cursor_Function *buffer_compute_cursor;
Buffer_Exists_Function *buffer_exists;
Buffer_Ready_Function *buffer_ready;
Buffer_Get_Access_Flags_Function *buffer_get_access_flags;
Buffer_Get_Size_Function *buffer_get_size;
Buffer_Get_Line_Count_Function *buffer_get_line_count;
Buffer_Get_Base_Buffer_Name_Function *buffer_get_base_buffer_name;
Buffer_Get_Unique_Buffer_Name_Function *buffer_get_unique_buffer_name;
Buffer_Get_File_Name_Function *buffer_get_file_name;
Buffer_Get_Dirty_State_Function *buffer_get_dirty_state;
Buffer_Directly_Set_Dirty_State_Function *buffer_directly_set_dirty_state;
Buffer_Tokens_Are_Ready_Function *buffer_tokens_are_ready;
Buffer_Get_Setting_Function *buffer_get_setting;
Buffer_Set_Setting_Function *buffer_set_setting;
Buffer_Get_Managed_Scope_Function *buffer_get_managed_scope;
Buffer_Token_Count_Function *buffer_token_count;
Buffer_Read_Tokens_Function *buffer_read_tokens;
Buffer_Get_Token_Range_Function *buffer_get_token_range;
Buffer_Get_Token_Index_Function *buffer_get_token_index;
Buffer_Send_End_Signal_Function *buffer_send_end_signal;
Create_Buffer_Function *create_buffer;
Buffer_Save_Function *buffer_save;
Buffer_Kill_Function *buffer_kill;
Buffer_Reopen_Function *buffer_reopen;
Buffer_Get_File_Attributes_Function *buffer_get_file_attributes;
Get_View_Next_Function *get_view_next;
Get_View_Prev_Function *get_view_prev;
Get_Active_View_Function *get_active_view;
Get_Active_Panel_Function *get_active_panel;
View_Exists_Function *view_exists;
View_Get_Buffer_Function *view_get_buffer;
View_Get_Cursor_Pos_Function *view_get_cursor_pos;
View_Get_Mark_Pos_Function *view_get_mark_pos;
View_Get_Preferred_X_Function *view_get_preferred_x;
View_Get_Screen_Rect_Function *view_get_screen_rect;
View_Get_Panel_Function *view_get_panel;
Panel_Get_View_Function *panel_get_view;
Panel_Is_Split_Function *panel_is_split;
Panel_Is_Leaf_Function *panel_is_leaf;
Panel_Split_Function *panel_split;
Panel_Set_Split_Function *panel_set_split;
Panel_Swap_Children_Function *panel_swap_children;
Panel_Get_Parent_Function *panel_get_parent;
Panel_Get_Child_Function *panel_get_child;
Panel_Get_Max_Function *panel_get_max;
Panel_Get_Margin_Function *panel_get_margin;
View_Close_Function *view_close;
View_Get_Region_Function *view_get_region;
View_Get_Buffer_Region_Function *view_get_buffer_region;
View_Get_Scroll_Vars_Function *view_get_scroll_vars;
View_Set_Active_Function *view_set_active;
View_Get_Setting_Function *view_get_setting;
View_Set_Setting_Function *view_set_setting;
View_Get_Managed_Scope_Function *view_get_managed_scope;
View_Compute_Cursor_Function *view_compute_cursor;
View_Set_Cursor_Function *view_set_cursor;
View_Set_Scroll_Function *view_set_scroll;
View_Set_Mark_Function *view_set_mark;
View_Set_Buffer_Function *view_set_buffer;
View_Post_Fade_Function *view_post_fade;
View_Begin_UI_Mode_Function *view_begin_ui_mode;
View_End_UI_Mode_Function *view_end_ui_mode;
View_Is_In_UI_Mode_Function *view_is_in_ui_mode;
View_Set_Quit_UI_Handler_Function *view_set_quit_ui_handler;
View_Get_Quit_UI_Handler_Function *view_get_quit_ui_handler;
Create_User_Managed_Scope_Function *create_user_managed_scope;
Destroy_User_Managed_Scope_Function *destroy_user_managed_scope;
Get_Global_Managed_Scope_Function *get_global_managed_scope;
Get_Managed_Scope_With_Multiple_Dependencies_Function *get_managed_scope_with_multiple_dependencies;
Managed_Scope_Clear_Contents_Function *managed_scope_clear_contents;
Managed_Scope_Clear_Self_All_Dependent_Scopes_Function *managed_scope_clear_self_all_dependent_scopes;
Managed_Variable_Create_Function *managed_variable_create;
Managed_Variable_Get_ID_Function *managed_variable_get_id;
Managed_Variable_Create_Or_Get_ID_Function *managed_variable_create_or_get_id;
Managed_Variable_Set_Function *managed_variable_set;
Managed_Variable_Get_Function *managed_variable_get;
Alloc_Managed_Memory_In_Scope_Function *alloc_managed_memory_in_scope;
Alloc_Buffer_Markers_On_Buffer_Function *alloc_buffer_markers_on_buffer;
Alloc_Managed_Arena_In_Scope_Function *alloc_managed_arena_in_scope;
Create_Marker_Visual_Function *create_marker_visual;
Marker_Visual_Set_Effect_Function *marker_visual_set_effect;
Marker_Visual_Set_Take_Rule_Function *marker_visual_set_take_rule;
Marker_Visual_Set_Priority_Function *marker_visual_set_priority;
Marker_Visual_Set_View_Key_Function *marker_visual_set_view_key;
Destroy_Marker_Visual_Function *destroy_marker_visual;
Buffer_Markers_Get_Attached_Visual_Count_Function *buffer_markers_get_attached_visual_count;
Buffer_Markers_Get_Attached_Visual_Function *buffer_markers_get_attached_visual;
Managed_Object_Get_Item_Size_Function *managed_object_get_item_size;
Managed_Object_Get_Item_Count_Function *managed_object_get_item_count;
Managed_Object_Get_Type_Function *managed_object_get_type;
Managed_Object_Get_Containing_Scope_Function *managed_object_get_containing_scope;
Managed_Object_Free_Function *managed_object_free;
Managed_Object_Store_Data_Function *managed_object_store_data;
Managed_Object_Load_Data_Function *managed_object_load_data;
Get_User_Input_Function *get_user_input;
Get_Command_Input_Function *get_command_input;
Set_Command_Input_Function *set_command_input;
Get_Mouse_State_Function *get_mouse_state;
Get_Active_Query_Bars_Function *get_active_query_bars;
Start_Query_Bar_Function *start_query_bar;
End_Query_Bar_Function *end_query_bar;
Print_Message_Function *print_message;
Get_Largest_Face_ID_Function *get_largest_face_id;
Set_Global_Face_Function *set_global_face;
Buffer_History_Get_Max_Record_Index_Function *buffer_history_get_max_record_index;
Buffer_History_Get_Record_Info_Function *buffer_history_get_record_info;
Buffer_History_Get_Group_Sub_Record_Function *buffer_history_get_group_sub_record;
Buffer_History_Get_Current_State_Index_Function *buffer_history_get_current_state_index;
Buffer_History_Set_Current_State_Index_Function *buffer_history_set_current_state_index;
Buffer_History_Merge_Record_Range_Function *buffer_history_merge_record_range;
Buffer_History_Clear_After_Current_State_Function *buffer_history_clear_after_current_state;
Global_History_Edit_Group_Begin_Function *global_history_edit_group_begin;
Global_History_Edit_Group_End_Function *global_history_edit_group_end;
Buffer_Set_Face_Function *buffer_set_face;
Get_Face_Description_Function *get_face_description;
Get_Face_Metrics_Function *get_face_metrics;
Get_Face_ID_Function *get_face_id;
Try_Create_New_Face_Function *try_create_new_face;
Try_Modify_Face_Function *try_modify_face;
Try_Release_Face_Function *try_release_face;
Get_Available_Font_Count_Function *get_available_font_count;
Get_Available_Font_Function *get_available_font;
Set_Theme_Colors_Function *set_theme_colors;
Get_Theme_Colors_Function *get_theme_colors;
Finalize_Color_Function *finalize_color;
Get_Hot_Directory_Function *get_hot_directory;
Set_Hot_Directory_Function *set_hot_directory;
Get_File_List_Function *get_file_list;
Free_File_List_Function *free_file_list;
Set_GUI_Up_Down_Keys_Function *set_gui_up_down_keys;
Memory_Allocate_Function *memory_allocate;
Memory_Set_Protection_Function *memory_set_protection;
Memory_Free_Function *memory_free;
File_Get_Attributes_Function *file_get_attributes;
Get_4ed_Path_Function *get_4ed_path;
Show_Mouse_Cursor_Function *show_mouse_cursor;
Set_Edit_Finished_Hook_Repeat_Speed_Function *set_edit_finished_hook_repeat_speed;
Set_Fullscreen_Function *set_fullscreen;
Is_Fullscreen_Function *is_fullscreen;
Send_Exit_Signal_Function *send_exit_signal;
Set_Window_Title_Function *set_window_title;
Get_Microseconds_Timestamp_Function *get_microseconds_timestamp;
Draw_String_Function *draw_string;
Get_String_Advance_Function *get_string_advance;
Draw_Rectangle_Function *draw_rectangle;
Draw_Rectangle_Outline_Function *draw_rectangle_outline;
Draw_Clip_Push_Function *draw_clip_push;
Draw_Clip_Pop_Function *draw_clip_pop;
Draw_Coordinate_Center_Push_Function *draw_coordinate_center_push;
Draw_Coordinate_Center_Pop_Function *draw_coordinate_center_pop;
Text_Layout_Get_Buffer_Function *text_layout_get_buffer;
Text_Layout_Buffer_Point_To_Layout_Point_Function *text_layout_buffer_point_to_layout_point;
Text_Layout_Layout_Point_To_Buffer_Point_Function *text_layout_layout_point_to_buffer_point;
Text_Layout_Get_On_Screen_Range_Function *text_layout_get_on_screen_range;
Text_Layout_Get_Height_Function *text_layout_get_height;
Text_Layout_Free_Function *text_layout_free;
Compute_Render_Layout_Function *compute_render_layout;
Draw_Render_Layout_Function *draw_render_layout;
Open_Color_Picker_Function *open_color_picker;
Animate_In_N_Milliseconds_Function *animate_in_n_milliseconds;
Find_All_Matches_Buffer_Range_Function *find_all_matches_buffer_range;
Get_View_Visible_Range_Function *get_view_visible_range;
#else
Global_Set_Setting_Function *global_set_setting_;
Global_Set_Mapping_Function *global_set_mapping_;
Global_Get_Screen_Rectangle_Function *global_get_screen_rectangle_;
Context_Get_Arena_Function *context_get_arena_;
Context_Get_Base_Allocator_Function *context_get_base_allocator_;
Create_Child_Process_Function *create_child_process_;
Child_Process_Set_Target_Buffer_Function *child_process_set_target_buffer_;
Buffer_Get_Attached_Child_Process_Function *buffer_get_attached_child_process_;
Child_Process_Get_Attached_Buffer_Function *child_process_get_attached_buffer_;
Child_Process_Get_State_Function *child_process_get_state_;
Clipboard_Post_Function *clipboard_post_;
Clipboard_Count_Function *clipboard_count_;
Clipboard_Index_Function *clipboard_index_;
Create_Parse_Context_Function *create_parse_context_;
Get_Buffer_Count_Function *get_buffer_count_;
Get_Buffer_Next_Function *get_buffer_next_;
Get_Buffer_By_Name_Function *get_buffer_by_name_;
Get_Buffer_By_File_Name_Function *get_buffer_by_file_name_;
Buffer_Read_Range_Function *buffer_read_range_;
Buffer_Replace_Range_Function *buffer_replace_range_;
Buffer_Batch_Edit_Function *buffer_batch_edit_;
Buffer_Seek_String_Function *buffer_seek_string_;
Buffer_Seek_Character_Class_Function *buffer_seek_character_class_;
Buffer_Compute_Cursor_Function *buffer_compute_cursor_;
Buffer_Exists_Function *buffer_exists_;
Buffer_Ready_Function *buffer_ready_;
Buffer_Get_Access_Flags_Function *buffer_get_access_flags_;
Buffer_Get_Size_Function *buffer_get_size_;
Buffer_Get_Line_Count_Function *buffer_get_line_count_;
Buffer_Get_Base_Buffer_Name_Function *buffer_get_base_buffer_name_;
Buffer_Get_Unique_Buffer_Name_Function *buffer_get_unique_buffer_name_;
Buffer_Get_File_Name_Function *buffer_get_file_name_;
Buffer_Get_Dirty_State_Function *buffer_get_dirty_state_;
Buffer_Directly_Set_Dirty_State_Function *buffer_directly_set_dirty_state_;
Buffer_Tokens_Are_Ready_Function *buffer_tokens_are_ready_;
Buffer_Get_Setting_Function *buffer_get_setting_;
Buffer_Set_Setting_Function *buffer_set_setting_;
Buffer_Get_Managed_Scope_Function *buffer_get_managed_scope_;
Buffer_Token_Count_Function *buffer_token_count_;
Buffer_Read_Tokens_Function *buffer_read_tokens_;
Buffer_Get_Token_Range_Function *buffer_get_token_range_;
Buffer_Get_Token_Index_Function *buffer_get_token_index_;
Buffer_Send_End_Signal_Function *buffer_send_end_signal_;
Create_Buffer_Function *create_buffer_;
Buffer_Save_Function *buffer_save_;
Buffer_Kill_Function *buffer_kill_;
Buffer_Reopen_Function *buffer_reopen_;
Buffer_Get_File_Attributes_Function *buffer_get_file_attributes_;
Get_View_Next_Function *get_view_next_;
Get_View_Prev_Function *get_view_prev_;
Get_Active_View_Function *get_active_view_;
Get_Active_Panel_Function *get_active_panel_;
View_Exists_Function *view_exists_;
View_Get_Buffer_Function *view_get_buffer_;
View_Get_Cursor_Pos_Function *view_get_cursor_pos_;
View_Get_Mark_Pos_Function *view_get_mark_pos_;
View_Get_Preferred_X_Function *view_get_preferred_x_;
View_Get_Screen_Rect_Function *view_get_screen_rect_;
View_Get_Panel_Function *view_get_panel_;
Panel_Get_View_Function *panel_get_view_;
Panel_Is_Split_Function *panel_is_split_;
Panel_Is_Leaf_Function *panel_is_leaf_;
Panel_Split_Function *panel_split_;
Panel_Set_Split_Function *panel_set_split_;
Panel_Swap_Children_Function *panel_swap_children_;
Panel_Get_Parent_Function *panel_get_parent_;
Panel_Get_Child_Function *panel_get_child_;
Panel_Get_Max_Function *panel_get_max_;
Panel_Get_Margin_Function *panel_get_margin_;
View_Close_Function *view_close_;
View_Get_Region_Function *view_get_region_;
View_Get_Buffer_Region_Function *view_get_buffer_region_;
View_Get_Scroll_Vars_Function *view_get_scroll_vars_;
View_Set_Active_Function *view_set_active_;
View_Get_Setting_Function *view_get_setting_;
View_Set_Setting_Function *view_set_setting_;
View_Get_Managed_Scope_Function *view_get_managed_scope_;
View_Compute_Cursor_Function *view_compute_cursor_;
View_Set_Cursor_Function *view_set_cursor_;
View_Set_Scroll_Function *view_set_scroll_;
View_Set_Mark_Function *view_set_mark_;
View_Set_Buffer_Function *view_set_buffer_;
View_Post_Fade_Function *view_post_fade_;
View_Begin_UI_Mode_Function *view_begin_ui_mode_;
View_End_UI_Mode_Function *view_end_ui_mode_;
View_Is_In_UI_Mode_Function *view_is_in_ui_mode_;
View_Set_Quit_UI_Handler_Function *view_set_quit_ui_handler_;
View_Get_Quit_UI_Handler_Function *view_get_quit_ui_handler_;
Create_User_Managed_Scope_Function *create_user_managed_scope_;
Destroy_User_Managed_Scope_Function *destroy_user_managed_scope_;
Get_Global_Managed_Scope_Function *get_global_managed_scope_;
Get_Managed_Scope_With_Multiple_Dependencies_Function *get_managed_scope_with_multiple_dependencies_;
Managed_Scope_Clear_Contents_Function *managed_scope_clear_contents_;
Managed_Scope_Clear_Self_All_Dependent_Scopes_Function *managed_scope_clear_self_all_dependent_scopes_;
Managed_Variable_Create_Function *managed_variable_create_;
Managed_Variable_Get_ID_Function *managed_variable_get_id_;
Managed_Variable_Create_Or_Get_ID_Function *managed_variable_create_or_get_id_;
Managed_Variable_Set_Function *managed_variable_set_;
Managed_Variable_Get_Function *managed_variable_get_;
Alloc_Managed_Memory_In_Scope_Function *alloc_managed_memory_in_scope_;
Alloc_Buffer_Markers_On_Buffer_Function *alloc_buffer_markers_on_buffer_;
Alloc_Managed_Arena_In_Scope_Function *alloc_managed_arena_in_scope_;
Create_Marker_Visual_Function *create_marker_visual_;
Marker_Visual_Set_Effect_Function *marker_visual_set_effect_;
Marker_Visual_Set_Take_Rule_Function *marker_visual_set_take_rule_;
Marker_Visual_Set_Priority_Function *marker_visual_set_priority_;
Marker_Visual_Set_View_Key_Function *marker_visual_set_view_key_;
Destroy_Marker_Visual_Function *destroy_marker_visual_;
Buffer_Markers_Get_Attached_Visual_Count_Function *buffer_markers_get_attached_visual_count_;
Buffer_Markers_Get_Attached_Visual_Function *buffer_markers_get_attached_visual_;
Managed_Object_Get_Item_Size_Function *managed_object_get_item_size_;
Managed_Object_Get_Item_Count_Function *managed_object_get_item_count_;
Managed_Object_Get_Type_Function *managed_object_get_type_;
Managed_Object_Get_Containing_Scope_Function *managed_object_get_containing_scope_;
Managed_Object_Free_Function *managed_object_free_;
Managed_Object_Store_Data_Function *managed_object_store_data_;
Managed_Object_Load_Data_Function *managed_object_load_data_;
Get_User_Input_Function *get_user_input_;
Get_Command_Input_Function *get_command_input_;
Set_Command_Input_Function *set_command_input_;
Get_Mouse_State_Function *get_mouse_state_;
Get_Active_Query_Bars_Function *get_active_query_bars_;
Start_Query_Bar_Function *start_query_bar_;
End_Query_Bar_Function *end_query_bar_;
Print_Message_Function *print_message_;
Get_Largest_Face_ID_Function *get_largest_face_id_;
Set_Global_Face_Function *set_global_face_;
Buffer_History_Get_Max_Record_Index_Function *buffer_history_get_max_record_index_;
Buffer_History_Get_Record_Info_Function *buffer_history_get_record_info_;
Buffer_History_Get_Group_Sub_Record_Function *buffer_history_get_group_sub_record_;
Buffer_History_Get_Current_State_Index_Function *buffer_history_get_current_state_index_;
Buffer_History_Set_Current_State_Index_Function *buffer_history_set_current_state_index_;
Buffer_History_Merge_Record_Range_Function *buffer_history_merge_record_range_;
Buffer_History_Clear_After_Current_State_Function *buffer_history_clear_after_current_state_;
Global_History_Edit_Group_Begin_Function *global_history_edit_group_begin_;
Global_History_Edit_Group_End_Function *global_history_edit_group_end_;
Buffer_Set_Face_Function *buffer_set_face_;
Get_Face_Description_Function *get_face_description_;
Get_Face_Metrics_Function *get_face_metrics_;
Get_Face_ID_Function *get_face_id_;
Try_Create_New_Face_Function *try_create_new_face_;
Try_Modify_Face_Function *try_modify_face_;
Try_Release_Face_Function *try_release_face_;
Get_Available_Font_Count_Function *get_available_font_count_;
Get_Available_Font_Function *get_available_font_;
Set_Theme_Colors_Function *set_theme_colors_;
Get_Theme_Colors_Function *get_theme_colors_;
Finalize_Color_Function *finalize_color_;
Get_Hot_Directory_Function *get_hot_directory_;
Set_Hot_Directory_Function *set_hot_directory_;
Get_File_List_Function *get_file_list_;
Free_File_List_Function *free_file_list_;
Set_GUI_Up_Down_Keys_Function *set_gui_up_down_keys_;
Memory_Allocate_Function *memory_allocate_;
Memory_Set_Protection_Function *memory_set_protection_;
Memory_Free_Function *memory_free_;
File_Get_Attributes_Function *file_get_attributes_;
Get_4ed_Path_Function *get_4ed_path_;
Show_Mouse_Cursor_Function *show_mouse_cursor_;
Set_Edit_Finished_Hook_Repeat_Speed_Function *set_edit_finished_hook_repeat_speed_;
Set_Fullscreen_Function *set_fullscreen_;
Is_Fullscreen_Function *is_fullscreen_;
Send_Exit_Signal_Function *send_exit_signal_;
Set_Window_Title_Function *set_window_title_;
Get_Microseconds_Timestamp_Function *get_microseconds_timestamp_;
Draw_String_Function *draw_string_;
Get_String_Advance_Function *get_string_advance_;
Draw_Rectangle_Function *draw_rectangle_;
Draw_Rectangle_Outline_Function *draw_rectangle_outline_;
Draw_Clip_Push_Function *draw_clip_push_;
Draw_Clip_Pop_Function *draw_clip_pop_;
Draw_Coordinate_Center_Push_Function *draw_coordinate_center_push_;
Draw_Coordinate_Center_Pop_Function *draw_coordinate_center_pop_;
Text_Layout_Get_Buffer_Function *text_layout_get_buffer_;
Text_Layout_Buffer_Point_To_Layout_Point_Function *text_layout_buffer_point_to_layout_point_;
Text_Layout_Layout_Point_To_Buffer_Point_Function *text_layout_layout_point_to_buffer_point_;
Text_Layout_Get_On_Screen_Range_Function *text_layout_get_on_screen_range_;
Text_Layout_Get_Height_Function *text_layout_get_height_;
Text_Layout_Free_Function *text_layout_free_;
Compute_Render_Layout_Function *compute_render_layout_;
Draw_Render_Layout_Function *draw_render_layout_;
Open_Color_Picker_Function *open_color_picker_;
Animate_In_N_Milliseconds_Function *animate_in_n_milliseconds_;
Find_All_Matches_Buffer_Range_Function *find_all_matches_buffer_range_;
Get_View_Visible_Range_Function *get_view_visible_range_;
#endif
void *memory;
int32_t memory_size;
void *cmd_context;
void *system_links;
void *current_coroutine;
int32_t type_coroutine;
};
#define FillAppLinksAPI(app_links) do{\
app_links->global_set_setting_ = Global_Set_Setting;\
app_links->global_set_mapping_ = Global_Set_Mapping;\
app_links->global_get_screen_rectangle_ = Global_Get_Screen_Rectangle;\
app_links->context_get_arena_ = Context_Get_Arena;\
app_links->context_get_base_allocator_ = Context_Get_Base_Allocator;\
app_links->create_child_process_ = Create_Child_Process;\
app_links->child_process_set_target_buffer_ = Child_Process_Set_Target_Buffer;\
app_links->buffer_get_attached_child_process_ = Buffer_Get_Attached_Child_Process;\
app_links->child_process_get_attached_buffer_ = Child_Process_Get_Attached_Buffer;\
app_links->child_process_get_state_ = Child_Process_Get_State;\
app_links->clipboard_post_ = Clipboard_Post;\
app_links->clipboard_count_ = Clipboard_Count;\
app_links->clipboard_index_ = Clipboard_Index;\
app_links->create_parse_context_ = Create_Parse_Context;\
app_links->get_buffer_count_ = Get_Buffer_Count;\
app_links->get_buffer_next_ = Get_Buffer_Next;\
app_links->get_buffer_by_name_ = Get_Buffer_By_Name;\
app_links->get_buffer_by_file_name_ = Get_Buffer_By_File_Name;\
app_links->buffer_read_range_ = Buffer_Read_Range;\
app_links->buffer_replace_range_ = Buffer_Replace_Range;\
app_links->buffer_batch_edit_ = Buffer_Batch_Edit;\
app_links->buffer_seek_string_ = Buffer_Seek_String;\
app_links->buffer_seek_character_class_ = Buffer_Seek_Character_Class;\
app_links->buffer_compute_cursor_ = Buffer_Compute_Cursor;\
app_links->buffer_exists_ = Buffer_Exists;\
app_links->buffer_ready_ = Buffer_Ready;\
app_links->buffer_get_access_flags_ = Buffer_Get_Access_Flags;\
app_links->buffer_get_size_ = Buffer_Get_Size;\
app_links->buffer_get_line_count_ = Buffer_Get_Line_Count;\
app_links->buffer_get_base_buffer_name_ = Buffer_Get_Base_Buffer_Name;\
app_links->buffer_get_unique_buffer_name_ = Buffer_Get_Unique_Buffer_Name;\
app_links->buffer_get_file_name_ = Buffer_Get_File_Name;\
app_links->buffer_get_dirty_state_ = Buffer_Get_Dirty_State;\
app_links->buffer_directly_set_dirty_state_ = Buffer_Directly_Set_Dirty_State;\
app_links->buffer_tokens_are_ready_ = Buffer_Tokens_Are_Ready;\
app_links->buffer_get_setting_ = Buffer_Get_Setting;\
app_links->buffer_set_setting_ = Buffer_Set_Setting;\
app_links->buffer_get_managed_scope_ = Buffer_Get_Managed_Scope;\
app_links->buffer_token_count_ = Buffer_Token_Count;\
app_links->buffer_read_tokens_ = Buffer_Read_Tokens;\
app_links->buffer_get_token_range_ = Buffer_Get_Token_Range;\
app_links->buffer_get_token_index_ = Buffer_Get_Token_Index;\
app_links->buffer_send_end_signal_ = Buffer_Send_End_Signal;\
app_links->create_buffer_ = Create_Buffer;\
app_links->buffer_save_ = Buffer_Save;\
app_links->buffer_kill_ = Buffer_Kill;\
app_links->buffer_reopen_ = Buffer_Reopen;\
app_links->buffer_get_file_attributes_ = Buffer_Get_File_Attributes;\
app_links->get_view_next_ = Get_View_Next;\
app_links->get_view_prev_ = Get_View_Prev;\
app_links->get_active_view_ = Get_Active_View;\
app_links->get_active_panel_ = Get_Active_Panel;\
app_links->view_exists_ = View_Exists;\
app_links->view_get_buffer_ = View_Get_Buffer;\
app_links->view_get_cursor_pos_ = View_Get_Cursor_Pos;\
app_links->view_get_mark_pos_ = View_Get_Mark_Pos;\
app_links->view_get_preferred_x_ = View_Get_Preferred_X;\
app_links->view_get_screen_rect_ = View_Get_Screen_Rect;\
app_links->view_get_panel_ = View_Get_Panel;\
app_links->panel_get_view_ = Panel_Get_View;\
app_links->panel_is_split_ = Panel_Is_Split;\
app_links->panel_is_leaf_ = Panel_Is_Leaf;\
app_links->panel_split_ = Panel_Split;\
app_links->panel_set_split_ = Panel_Set_Split;\
app_links->panel_swap_children_ = Panel_Swap_Children;\
app_links->panel_get_parent_ = Panel_Get_Parent;\
app_links->panel_get_child_ = Panel_Get_Child;\
app_links->panel_get_max_ = Panel_Get_Max;\
app_links->panel_get_margin_ = Panel_Get_Margin;\
app_links->view_close_ = View_Close;\
app_links->view_get_region_ = View_Get_Region;\
app_links->view_get_buffer_region_ = View_Get_Buffer_Region;\
app_links->view_get_scroll_vars_ = View_Get_Scroll_Vars;\
app_links->view_set_active_ = View_Set_Active;\
app_links->view_get_setting_ = View_Get_Setting;\
app_links->view_set_setting_ = View_Set_Setting;\
app_links->view_get_managed_scope_ = View_Get_Managed_Scope;\
app_links->view_compute_cursor_ = View_Compute_Cursor;\
app_links->view_set_cursor_ = View_Set_Cursor;\
app_links->view_set_scroll_ = View_Set_Scroll;\
app_links->view_set_mark_ = View_Set_Mark;\
app_links->view_set_buffer_ = View_Set_Buffer;\
app_links->view_post_fade_ = View_Post_Fade;\
app_links->view_begin_ui_mode_ = View_Begin_UI_Mode;\
app_links->view_end_ui_mode_ = View_End_UI_Mode;\
app_links->view_is_in_ui_mode_ = View_Is_In_UI_Mode;\
app_links->view_set_quit_ui_handler_ = View_Set_Quit_UI_Handler;\
app_links->view_get_quit_ui_handler_ = View_Get_Quit_UI_Handler;\
app_links->create_user_managed_scope_ = Create_User_Managed_Scope;\
app_links->destroy_user_managed_scope_ = Destroy_User_Managed_Scope;\
app_links->get_global_managed_scope_ = Get_Global_Managed_Scope;\
app_links->get_managed_scope_with_multiple_dependencies_ = Get_Managed_Scope_With_Multiple_Dependencies;\
app_links->managed_scope_clear_contents_ = Managed_Scope_Clear_Contents;\
app_links->managed_scope_clear_self_all_dependent_scopes_ = Managed_Scope_Clear_Self_All_Dependent_Scopes;\
app_links->managed_variable_create_ = Managed_Variable_Create;\
app_links->managed_variable_get_id_ = Managed_Variable_Get_ID;\
app_links->managed_variable_create_or_get_id_ = Managed_Variable_Create_Or_Get_ID;\
app_links->managed_variable_set_ = Managed_Variable_Set;\
app_links->managed_variable_get_ = Managed_Variable_Get;\
app_links->alloc_managed_memory_in_scope_ = Alloc_Managed_Memory_In_Scope;\
app_links->alloc_buffer_markers_on_buffer_ = Alloc_Buffer_Markers_On_Buffer;\
app_links->alloc_managed_arena_in_scope_ = Alloc_Managed_Arena_In_Scope;\
app_links->create_marker_visual_ = Create_Marker_Visual;\
app_links->marker_visual_set_effect_ = Marker_Visual_Set_Effect;\
app_links->marker_visual_set_take_rule_ = Marker_Visual_Set_Take_Rule;\
app_links->marker_visual_set_priority_ = Marker_Visual_Set_Priority;\
app_links->marker_visual_set_view_key_ = Marker_Visual_Set_View_Key;\
app_links->destroy_marker_visual_ = Destroy_Marker_Visual;\
app_links->buffer_markers_get_attached_visual_count_ = Buffer_Markers_Get_Attached_Visual_Count;\
app_links->buffer_markers_get_attached_visual_ = Buffer_Markers_Get_Attached_Visual;\
app_links->managed_object_get_item_size_ = Managed_Object_Get_Item_Size;\
app_links->managed_object_get_item_count_ = Managed_Object_Get_Item_Count;\
app_links->managed_object_get_type_ = Managed_Object_Get_Type;\
app_links->managed_object_get_containing_scope_ = Managed_Object_Get_Containing_Scope;\
app_links->managed_object_free_ = Managed_Object_Free;\
app_links->managed_object_store_data_ = Managed_Object_Store_Data;\
app_links->managed_object_load_data_ = Managed_Object_Load_Data;\
app_links->get_user_input_ = Get_User_Input;\
app_links->get_command_input_ = Get_Command_Input;\
app_links->set_command_input_ = Set_Command_Input;\
app_links->get_mouse_state_ = Get_Mouse_State;\
app_links->get_active_query_bars_ = Get_Active_Query_Bars;\
app_links->start_query_bar_ = Start_Query_Bar;\
app_links->end_query_bar_ = End_Query_Bar;\
app_links->print_message_ = Print_Message;\
app_links->get_largest_face_id_ = Get_Largest_Face_ID;\
app_links->set_global_face_ = Set_Global_Face;\
app_links->buffer_history_get_max_record_index_ = Buffer_History_Get_Max_Record_Index;\
app_links->buffer_history_get_record_info_ = Buffer_History_Get_Record_Info;\
app_links->buffer_history_get_group_sub_record_ = Buffer_History_Get_Group_Sub_Record;\
app_links->buffer_history_get_current_state_index_ = Buffer_History_Get_Current_State_Index;\
app_links->buffer_history_set_current_state_index_ = Buffer_History_Set_Current_State_Index;\
app_links->buffer_history_merge_record_range_ = Buffer_History_Merge_Record_Range;\
app_links->buffer_history_clear_after_current_state_ = Buffer_History_Clear_After_Current_State;\
app_links->global_history_edit_group_begin_ = Global_History_Edit_Group_Begin;\
app_links->global_history_edit_group_end_ = Global_History_Edit_Group_End;\
app_links->buffer_set_face_ = Buffer_Set_Face;\
app_links->get_face_description_ = Get_Face_Description;\
app_links->get_face_metrics_ = Get_Face_Metrics;\
app_links->get_face_id_ = Get_Face_ID;\
app_links->try_create_new_face_ = Try_Create_New_Face;\
app_links->try_modify_face_ = Try_Modify_Face;\
app_links->try_release_face_ = Try_Release_Face;\
app_links->get_available_font_count_ = Get_Available_Font_Count;\
app_links->get_available_font_ = Get_Available_Font;\
app_links->set_theme_colors_ = Set_Theme_Colors;\
app_links->get_theme_colors_ = Get_Theme_Colors;\
app_links->finalize_color_ = Finalize_Color;\
app_links->get_hot_directory_ = Get_Hot_Directory;\
app_links->set_hot_directory_ = Set_Hot_Directory;\
app_links->get_file_list_ = Get_File_List;\
app_links->free_file_list_ = Free_File_List;\
app_links->set_gui_up_down_keys_ = Set_GUI_Up_Down_Keys;\
app_links->memory_allocate_ = Memory_Allocate;\
app_links->memory_set_protection_ = Memory_Set_Protection;\
app_links->memory_free_ = Memory_Free;\
app_links->file_get_attributes_ = File_Get_Attributes;\
app_links->get_4ed_path_ = Get_4ed_Path;\
app_links->show_mouse_cursor_ = Show_Mouse_Cursor;\
app_links->set_edit_finished_hook_repeat_speed_ = Set_Edit_Finished_Hook_Repeat_Speed;\
app_links->set_fullscreen_ = Set_Fullscreen;\
app_links->is_fullscreen_ = Is_Fullscreen;\
app_links->send_exit_signal_ = Send_Exit_Signal;\
app_links->set_window_title_ = Set_Window_Title;\
app_links->get_microseconds_timestamp_ = Get_Microseconds_Timestamp;\
app_links->draw_string_ = Draw_String;\
app_links->get_string_advance_ = Get_String_Advance;\
app_links->draw_rectangle_ = Draw_Rectangle;\
app_links->draw_rectangle_outline_ = Draw_Rectangle_Outline;\
app_links->draw_clip_push_ = Draw_Clip_Push;\
app_links->draw_clip_pop_ = Draw_Clip_Pop;\
app_links->draw_coordinate_center_push_ = Draw_Coordinate_Center_Push;\
app_links->draw_coordinate_center_pop_ = Draw_Coordinate_Center_Pop;\
app_links->text_layout_get_buffer_ = Text_Layout_Get_Buffer;\
app_links->text_layout_buffer_point_to_layout_point_ = Text_Layout_Buffer_Point_To_Layout_Point;\
app_links->text_layout_layout_point_to_buffer_point_ = Text_Layout_Layout_Point_To_Buffer_Point;\
app_links->text_layout_get_on_screen_range_ = Text_Layout_Get_On_Screen_Range;\
app_links->text_layout_get_height_ = Text_Layout_Get_Height;\
app_links->text_layout_free_ = Text_Layout_Free;\
app_links->compute_render_layout_ = Compute_Render_Layout;\
app_links->draw_render_layout_ = Draw_Render_Layout;\
app_links->open_color_picker_ = Open_Color_Picker;\
app_links->animate_in_n_milliseconds_ = Animate_In_N_Milliseconds;\
app_links->find_all_matches_buffer_range_ = Find_All_Matches_Buffer_Range;\
app_links->get_view_visible_range_ = Get_View_Visible_Range;} while(false)
#if defined(ALLOW_DEP_4CODER)
static b32 global_set_setting(Application_Links *app, Global_Setting_ID setting, i32 value){return(app->global_set_setting(app, setting, value));}
static b32 global_set_mapping(Application_Links *app, void *data, i32 size){return(app->global_set_mapping(app, data, size));}
static b32 global_get_screen_rectangle(Application_Links *app, Rect_f32 *rect_out){return(app->global_get_screen_rectangle(app, rect_out));}
static Arena* context_get_arena(Application_Links *app){return(app->context_get_arena(app));}
static Base_Allocator* context_get_base_allocator(Application_Links *app){return(app->context_get_base_allocator(app));}
static b32 create_child_process(Application_Links *app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *child_process_id_out){return(app->create_child_process(app, path, command, child_process_id_out));}
static b32 child_process_set_target_buffer(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags){return(app->child_process_set_target_buffer(app, child_process_id, buffer_id, flags));}
static b32 buffer_get_attached_child_process(Application_Links *app, Buffer_ID buffer_id, Child_Process_ID *child_process_id_out){return(app->buffer_get_attached_child_process(app, buffer_id, child_process_id_out));}
static b32 child_process_get_attached_buffer(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID *buffer_id_out){return(app->child_process_get_attached_buffer(app, child_process_id, buffer_id_out));}
static b32 child_process_get_state(Application_Links *app, Child_Process_ID child_process_id, Process_State *process_state_out){return(app->child_process_get_state(app, child_process_id, process_state_out));}
static b32 clipboard_post(Application_Links *app, i32 clipboard_id, String_Const_u8 string){return(app->clipboard_post(app, clipboard_id, string));}
static b32 clipboard_count(Application_Links *app, i32 clipboard_id, i32 *count_out){return(app->clipboard_count(app, clipboard_id, count_out));}
static b32 clipboard_index(Application_Links *app, i32 clipboard_id, i32 item_index, Arena *out, String_Const_u8 *string_out){return(app->clipboard_index(app, clipboard_id, item_index, out, string_out));}
static Parse_Context_ID create_parse_context(Application_Links *app, Parser_String_And_Type *kw, u32 kw_count, Parser_String_And_Type *pp, u32 pp_count){return(app->create_parse_context(app, kw, kw_count, pp, pp_count));}
static i32 get_buffer_count(Application_Links *app){return(app->get_buffer_count(app));}
static b32 get_buffer_next(Application_Links *app, Buffer_ID buffer_id, Access_Flag access, Buffer_ID *buffer_id_out){return(app->get_buffer_next(app, buffer_id, access, buffer_id_out));}
static b32 get_buffer_by_name(Application_Links *app, String_Const_u8 name, Access_Flag access, Buffer_ID *buffer_id_out){return(app->get_buffer_by_name(app, name, access, buffer_id_out));}
static b32 get_buffer_by_file_name(Application_Links *app, String_Const_u8 file_name, Access_Flag access, Buffer_ID *buffer_id_out){return(app->get_buffer_by_file_name(app, file_name, access, buffer_id_out));}
static b32 buffer_read_range(Application_Links *app, Buffer_ID buffer_id, i32 start, i32 one_past_last, char *out){return(app->buffer_read_range(app, buffer_id, start, one_past_last, out));}
static b32 buffer_replace_range(Application_Links *app, Buffer_ID buffer_id, Range range, String_Const_u8 string){return(app->buffer_replace_range(app, buffer_id, range, string));}
static b32 buffer_batch_edit(Application_Links *app, Buffer_ID buffer_id, char *str, Buffer_Edit *edits, i32 edit_count){return(app->buffer_batch_edit(app, buffer_id, str, edits, edit_count));}
static b32 buffer_seek_string(Application_Links *app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i32 start_pos, i32 *pos_out, b32 *case_sensitive_out){return(app->buffer_seek_string(app, buffer, needle, direction, start_pos, pos_out, case_sensitive_out));}
static b32 buffer_seek_character_class(Application_Links *app, Buffer_ID buffer_id, Character_Predicate *predicate, Scan_Direction direction, i32 start_pos, i32 *pos_out){return(app->buffer_seek_character_class(app, buffer_id, predicate, direction, start_pos, pos_out));}
static b32 buffer_compute_cursor(Application_Links *app, Buffer_ID buffer_id, Buffer_Seek seek, Partial_Cursor *cursor_out){return(app->buffer_compute_cursor(app, buffer_id, seek, cursor_out));}
static b32 buffer_exists(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_exists(app, buffer_id));}
static b32 buffer_ready(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_ready(app, buffer_id));}
static b32 buffer_get_access_flags(Application_Links *app, Buffer_ID buffer_id, Access_Flag *access_flags_out){return(app->buffer_get_access_flags(app, buffer_id, access_flags_out));}
static b32 buffer_get_size(Application_Links *app, Buffer_ID buffer_id, i32 *size_out){return(app->buffer_get_size(app, buffer_id, size_out));}
static b32 buffer_get_line_count(Application_Links *app, Buffer_ID buffer_id, i32 *line_count_out){return(app->buffer_get_line_count(app, buffer_id, line_count_out));}
static b32 buffer_get_base_buffer_name(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out){return(app->buffer_get_base_buffer_name(app, buffer_id, out, name_out));}
static b32 buffer_get_unique_buffer_name(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out){return(app->buffer_get_unique_buffer_name(app, buffer_id, out, name_out));}
static b32 buffer_get_file_name(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out){return(app->buffer_get_file_name(app, buffer_id, out, name_out));}
static b32 buffer_get_dirty_state(Application_Links *app, Buffer_ID buffer_id, Dirty_State *dirty_state_out){return(app->buffer_get_dirty_state(app, buffer_id, dirty_state_out));}
static b32 buffer_directly_set_dirty_state(Application_Links *app, Buffer_ID buffer_id, Dirty_State dirty_state){return(app->buffer_directly_set_dirty_state(app, buffer_id, dirty_state));}
static b32 buffer_tokens_are_ready(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_tokens_are_ready(app, buffer_id));}
static b32 buffer_get_setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 *value_out){return(app->buffer_get_setting(app, buffer_id, setting, value_out));}
static b32 buffer_set_setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 value){return(app->buffer_set_setting(app, buffer_id, setting, value));}
static b32 buffer_get_managed_scope(Application_Links *app, Buffer_ID buffer_id, Managed_Scope *scope_out){return(app->buffer_get_managed_scope(app, buffer_id, scope_out));}
static b32 buffer_token_count(Application_Links *app, Buffer_ID buffer_id, i32 *count_out){return(app->buffer_token_count(app, buffer_id, count_out));}
static b32 buffer_read_tokens(Application_Links *app, Buffer_ID buffer_id, i32 start_token, i32 end_token, Cpp_Token *tokens_out){return(app->buffer_read_tokens(app, buffer_id, start_token, end_token, tokens_out));}
static b32 buffer_get_token_range(Application_Links *app, Buffer_ID buffer_id, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out){return(app->buffer_get_token_range(app, buffer_id, first_token_out, one_past_last_token_out));}
static b32 buffer_get_token_index(Application_Links *app, Buffer_ID buffer_id, i32 pos, Cpp_Get_Token_Result *get_result){return(app->buffer_get_token_index(app, buffer_id, pos, get_result));}
static b32 buffer_send_end_signal(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_send_end_signal(app, buffer_id));}
static b32 create_buffer(Application_Links *app, String_Const_u8 file_name, Buffer_Create_Flag flags, Buffer_ID *new_buffer_id_out){return(app->create_buffer(app, file_name, flags, new_buffer_id_out));}
static b32 buffer_save(Application_Links *app, Buffer_ID buffer_id, String_Const_u8 file_name, u32 flags){return(app->buffer_save(app, buffer_id, file_name, flags));}
static b32 buffer_kill(Application_Links *app, Buffer_ID buffer_id, Buffer_Kill_Flag flags, Buffer_Kill_Result *result_out){return(app->buffer_kill(app, buffer_id, flags, result_out));}
static b32 buffer_reopen(Application_Links *app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags, Buffer_Reopen_Result *result_out){return(app->buffer_reopen(app, buffer_id, flags, result_out));}
static b32 buffer_get_file_attributes(Application_Links *app, Buffer_ID buffer_id, File_Attributes *attributes_out){return(app->buffer_get_file_attributes(app, buffer_id, attributes_out));}
static b32 get_view_next(Application_Links *app, View_ID view_id, Access_Flag access, View_ID *view_id_out){return(app->get_view_next(app, view_id, access, view_id_out));}
static b32 get_view_prev(Application_Links *app, View_ID view_id, Access_Flag access, View_ID *view_id_out){return(app->get_view_prev(app, view_id, access, view_id_out));}
static b32 get_active_view(Application_Links *app, Access_Flag access, View_ID *view_id_out){return(app->get_active_view(app, access, view_id_out));}
static b32 get_active_panel(Application_Links *app, Panel_ID *panel_id_out){return(app->get_active_panel(app, panel_id_out));}
static b32 view_exists(Application_Links *app, View_ID view_id){return(app->view_exists(app, view_id));}
static b32 view_get_buffer(Application_Links *app, View_ID view_id, Access_Flag access, Buffer_ID *buffer_id_out){return(app->view_get_buffer(app, view_id, access, buffer_id_out));}
static b32 view_get_cursor_pos(Application_Links *app, View_ID view_id, i32 *pos_out){return(app->view_get_cursor_pos(app, view_id, pos_out));}
static b32 view_get_mark_pos(Application_Links *app, View_ID view_id, i32 *pos_out){return(app->view_get_mark_pos(app, view_id, pos_out));}
static b32 view_get_preferred_x(Application_Links *app, View_ID view_id, f32 *preferred_x_out){return(app->view_get_preferred_x(app, view_id, preferred_x_out));}
static b32 view_get_screen_rect(Application_Links *app, View_ID view_id, Rect_f32 *rect_out){return(app->view_get_screen_rect(app, view_id, rect_out));}
static b32 view_get_panel(Application_Links *app, View_ID view_id, Panel_ID *panel_id_out){return(app->view_get_panel(app, view_id, panel_id_out));}
static b32 panel_get_view(Application_Links *app, Panel_ID panel_id, View_ID *view_id_out){return(app->panel_get_view(app, panel_id, view_id_out));}
static b32 panel_is_split(Application_Links *app, Panel_ID panel_id){return(app->panel_is_split(app, panel_id));}
static b32 panel_is_leaf(Application_Links *app, Panel_ID panel_id){return(app->panel_is_leaf(app, panel_id));}
static b32 panel_split(Application_Links *app, Panel_ID panel_id, Panel_Split_Orientation orientation){return(app->panel_split(app, panel_id, orientation));}
static b32 panel_set_split(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t){return(app->panel_set_split(app, panel_id, kind, t));}
static b32 panel_swap_children(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t){return(app->panel_swap_children(app, panel_id, kind, t));}
static b32 panel_get_parent(Application_Links *app, Panel_ID panel_id, Panel_ID *panel_id_out){return(app->panel_get_parent(app, panel_id, panel_id_out));}
static b32 panel_get_child(Application_Links *app, Panel_ID panel_id, Panel_Child which_child, Panel_ID *panel_id_out){return(app->panel_get_child(app, panel_id, which_child, panel_id_out));}
static b32 panel_get_max(Application_Links *app, Panel_ID panel_id, Panel_ID *panel_id_out){return(app->panel_get_max(app, panel_id, panel_id_out));}
static b32 panel_get_margin(Application_Links *app, Panel_ID panel_id, i32_Rect *margins_out){return(app->panel_get_margin(app, panel_id, margins_out));}
static b32 view_close(Application_Links *app, View_ID view_id){return(app->view_close(app, view_id));}
static b32 view_get_region(Application_Links *app, View_ID view_id, Rect_i32 *region_out){return(app->view_get_region(app, view_id, region_out));}
static b32 view_get_buffer_region(Application_Links *app, View_ID view_id, Rect_i32 *region_out){return(app->view_get_buffer_region(app, view_id, region_out));}
static b32 view_get_scroll_vars(Application_Links *app, View_ID view_id, GUI_Scroll_Vars *scroll_vars_out){return(app->view_get_scroll_vars(app, view_id, scroll_vars_out));}
static b32 view_set_active(Application_Links *app, View_ID view_id){return(app->view_set_active(app, view_id));}
static b32 view_get_setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 *value_out){return(app->view_get_setting(app, view_id, setting, value_out));}
static b32 view_set_setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 value){return(app->view_set_setting(app, view_id, setting, value));}
static b32 view_get_managed_scope(Application_Links *app, View_ID view_id, Managed_Scope *scope){return(app->view_get_managed_scope(app, view_id, scope));}
static b32 view_compute_cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek, Full_Cursor *cursor_out){return(app->view_compute_cursor(app, view_id, seek, cursor_out));}
static b32 view_set_cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek, b32 set_preferred_x){return(app->view_set_cursor(app, view_id, seek, set_preferred_x));}
static b32 view_set_scroll(Application_Links *app, View_ID view_id, GUI_Scroll_Vars scroll){return(app->view_set_scroll(app, view_id, scroll));}
static b32 view_set_mark(Application_Links *app, View_ID view_id, Buffer_Seek seek){return(app->view_set_mark(app, view_id, seek));}
static b32 view_set_buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags){return(app->view_set_buffer(app, view_id, buffer_id, flags));}
static b32 view_post_fade(Application_Links *app, View_ID view_id, float seconds, i32 start, i32 end, int_color color){return(app->view_post_fade(app, view_id, seconds, start, end, color));}
static b32 view_begin_ui_mode(Application_Links *app, View_ID view_id){return(app->view_begin_ui_mode(app, view_id));}
static b32 view_end_ui_mode(Application_Links *app, View_ID view_id){return(app->view_end_ui_mode(app, view_id));}
static b32 view_is_in_ui_mode(Application_Links *app, View_ID view_id){return(app->view_is_in_ui_mode(app, view_id));}
static b32 view_set_quit_ui_handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type *quit_function){return(app->view_set_quit_ui_handler(app, view_id, quit_function));}
static b32 view_get_quit_ui_handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type **quit_function_out){return(app->view_get_quit_ui_handler(app, view_id, quit_function_out));}
static Managed_Scope create_user_managed_scope(Application_Links *app){return(app->create_user_managed_scope(app));}
static b32 destroy_user_managed_scope(Application_Links *app, Managed_Scope scope){return(app->destroy_user_managed_scope(app, scope));}
static Managed_Scope get_global_managed_scope(Application_Links *app){return(app->get_global_managed_scope(app));}
static Managed_Scope get_managed_scope_with_multiple_dependencies(Application_Links *app, Managed_Scope *scopes, i32 count){return(app->get_managed_scope_with_multiple_dependencies(app, scopes, count));}
static b32 managed_scope_clear_contents(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_contents(app, scope));}
static b32 managed_scope_clear_self_all_dependent_scopes(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_self_all_dependent_scopes(app, scope));}
static Managed_Variable_ID managed_variable_create(Application_Links *app, char *null_terminated_name, u64 default_value){return(app->managed_variable_create(app, null_terminated_name, default_value));}
static Managed_Variable_ID managed_variable_get_id(Application_Links *app, char *null_terminated_name){return(app->managed_variable_get_id(app, null_terminated_name));}
static Managed_Variable_ID managed_variable_create_or_get_id(Application_Links *app, char *null_terminated_name, u64 default_value){return(app->managed_variable_create_or_get_id(app, null_terminated_name, default_value));}
static b32 managed_variable_set(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 value){return(app->managed_variable_set(app, scope, id, value));}
static b32 managed_variable_get(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 *value_out){return(app->managed_variable_get(app, scope, id, value_out));}
static Managed_Object alloc_managed_memory_in_scope(Application_Links *app, Managed_Scope scope, i32 item_size, i32 count){return(app->alloc_managed_memory_in_scope(app, scope, item_size, count));}
static Managed_Object alloc_buffer_markers_on_buffer(Application_Links *app, Buffer_ID buffer_id, i32 count, Managed_Scope *optional_extra_scope){return(app->alloc_buffer_markers_on_buffer(app, buffer_id, count, optional_extra_scope));}
static Managed_Object alloc_managed_arena_in_scope(Application_Links *app, Managed_Scope scope, i32 page_size){return(app->alloc_managed_arena_in_scope(app, scope, page_size));}
static Marker_Visual create_marker_visual(Application_Links *app, Managed_Object object){return(app->create_marker_visual(app, object));}
static b32 marker_visual_set_effect(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style){return(app->marker_visual_set_effect(app, visual, type, color, text_color, text_style));}
static b32 marker_visual_set_take_rule(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule){return(app->marker_visual_set_take_rule(app, visual, take_rule));}
static b32 marker_visual_set_priority(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority){return(app->marker_visual_set_priority(app, visual, priority));}
static b32 marker_visual_set_view_key(Application_Links *app, Marker_Visual visual, View_ID key_view_id){return(app->marker_visual_set_view_key(app, visual, key_view_id));}
static b32 destroy_marker_visual(Application_Links *app, Marker_Visual visual){return(app->destroy_marker_visual(app, visual));}
static i32 buffer_markers_get_attached_visual_count(Application_Links *app, Managed_Object object){return(app->buffer_markers_get_attached_visual_count(app, object));}
static Marker_Visual* buffer_markers_get_attached_visual(Application_Links *app, Arena *arena, Managed_Object object){return(app->buffer_markers_get_attached_visual(app, arena, object));}
static u32 managed_object_get_item_size(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_size(app, object));}
static u32 managed_object_get_item_count(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_count(app, object));}
static Managed_Object_Type managed_object_get_type(Application_Links *app, Managed_Object object){return(app->managed_object_get_type(app, object));}
static Managed_Scope managed_object_get_containing_scope(Application_Links *app, Managed_Object object){return(app->managed_object_get_containing_scope(app, object));}
static b32 managed_object_free(Application_Links *app, Managed_Object object){return(app->managed_object_free(app, object));}
static b32 managed_object_store_data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem){return(app->managed_object_store_data(app, object, first_index, count, mem));}
static b32 managed_object_load_data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem_out){return(app->managed_object_load_data(app, object, first_index, count, mem_out));}
static User_Input get_user_input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type){return(app->get_user_input(app, get_type, abort_type));}
static User_Input get_command_input(Application_Links *app){return(app->get_command_input(app));}
static void set_command_input(Application_Links *app, Key_Event_Data key_data){(app->set_command_input(app, key_data));}
static Mouse_State get_mouse_state(Application_Links *app){return(app->get_mouse_state(app));}
static b32 get_active_query_bars(Application_Links *app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array *array_out){return(app->get_active_query_bars(app, view_id, max_result_count, array_out));}
static b32 start_query_bar(Application_Links *app, Query_Bar *bar, u32 flags){return(app->start_query_bar(app, bar, flags));}
static void end_query_bar(Application_Links *app, Query_Bar *bar, u32 flags){(app->end_query_bar(app, bar, flags));}
static b32 print_message(Application_Links *app, String_Const_u8 message){return(app->print_message(app, message));}
static Face_ID get_largest_face_id(Application_Links *app){return(app->get_largest_face_id(app));}
static b32 set_global_face(Application_Links *app, Face_ID id, b32 apply_to_all_buffers){return(app->set_global_face(app, id, apply_to_all_buffers));}
static b32 buffer_history_get_max_record_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index *index_out){return(app->buffer_history_get_max_record_index(app, buffer_id, index_out));}
static b32 buffer_history_get_record_info(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, Record_Info *record_out){return(app->buffer_history_get_record_info(app, buffer_id, index, record_out));}
static b32 buffer_history_get_group_sub_record(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index, Record_Info *record_out){return(app->buffer_history_get_group_sub_record(app, buffer_id, index, sub_index, record_out));}
static b32 buffer_history_get_current_state_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index *index_out){return(app->buffer_history_get_current_state_index(app, buffer_id, index_out));}
static b32 buffer_history_set_current_state_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){return(app->buffer_history_set_current_state_index(app, buffer_id, index));}
static b32 buffer_history_merge_record_range(Application_Links *app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){return(app->buffer_history_merge_record_range(app, buffer_id, first_index, last_index, flags));}
static b32 buffer_history_clear_after_current_state(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_history_clear_after_current_state(app, buffer_id));}
static void global_history_edit_group_begin(Application_Links *app){(app->global_history_edit_group_begin(app));}
static void global_history_edit_group_end(Application_Links *app){(app->global_history_edit_group_end(app));}
static b32 buffer_set_face(Application_Links *app, Buffer_ID buffer_id, Face_ID id){return(app->buffer_set_face(app, buffer_id, id));}
static Face_Description get_face_description(Application_Links *app, Face_ID id){return(app->get_face_description(app, id));}
static b32 get_face_metrics(Application_Links *app, Face_ID face_id, Face_Metrics *metrics_out){return(app->get_face_metrics(app, face_id, metrics_out));}
static b32 get_face_id(Application_Links *app, Buffer_ID buffer_id, Face_ID *face_id_out){return(app->get_face_id(app, buffer_id, face_id_out));}
static Face_ID try_create_new_face(Application_Links *app, Face_Description *description){return(app->try_create_new_face(app, description));}
static b32 try_modify_face(Application_Links *app, Face_ID id, Face_Description *description){return(app->try_modify_face(app, id, description));}
static b32 try_release_face(Application_Links *app, Face_ID id, Face_ID replacement_id){return(app->try_release_face(app, id, replacement_id));}
static i32 get_available_font_count(Application_Links *app){return(app->get_available_font_count(app));}
static Available_Font get_available_font(Application_Links *app, i32 index){return(app->get_available_font(app, index));}
static void set_theme_colors(Application_Links *app, Theme_Color *colors, i32 count){(app->set_theme_colors(app, colors, count));}
static void get_theme_colors(Application_Links *app, Theme_Color *colors, i32 count){(app->get_theme_colors(app, colors, count));}
static argb_color finalize_color(Application_Links *app, int_color color){return(app->finalize_color(app, color));}
static b32 get_hot_directory(Application_Links *app, Arena *out, String_Const_u8 *out_directory){return(app->get_hot_directory(app, out, out_directory));}
static b32 set_hot_directory(Application_Links *app, String_Const_u8 string){return(app->set_hot_directory(app, string));}
static b32 get_file_list(Application_Links *app, String_Const_u8 directory, File_List *list_out){return(app->get_file_list(app, directory, list_out));}
static void free_file_list(Application_Links *app, File_List list){(app->free_file_list(app, list));}
static void set_gui_up_down_keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier){(app->set_gui_up_down_keys(app, up_key, up_key_modifier, down_key, down_key_modifier));}
static void* memory_allocate(Application_Links *app, i32 size){return(app->memory_allocate(app, size));}
static b32 memory_set_protection(Application_Links *app, void *ptr, i32 size, Memory_Protect_Flags flags){return(app->memory_set_protection(app, ptr, size, flags));}
static void memory_free(Application_Links *app, void *ptr, i32 size){(app->memory_free(app, ptr, size));}
static b32 file_get_attributes(Application_Links *app, String_Const_u8 file_name, File_Attributes *attributes_out){return(app->file_get_attributes(app, file_name, attributes_out));}
static b32 get_4ed_path(Application_Links *app, Arena *out, String_Const_u8 *path_out){return(app->get_4ed_path(app, out, path_out));}
static void show_mouse_cursor(Application_Links *app, Mouse_Cursor_Show_Type show){(app->show_mouse_cursor(app, show));}
static b32 set_edit_finished_hook_repeat_speed(Application_Links *app, u32 milliseconds){return(app->set_edit_finished_hook_repeat_speed(app, milliseconds));}
static b32 set_fullscreen(Application_Links *app, b32 full_screen){return(app->set_fullscreen(app, full_screen));}
static b32 is_fullscreen(Application_Links *app){return(app->is_fullscreen(app));}
static void send_exit_signal(Application_Links *app){(app->send_exit_signal(app));}
static b32 set_window_title(Application_Links *app, String_Const_u8 title){return(app->set_window_title(app, title));}
static Microsecond_Time_Stamp get_microseconds_timestamp(Application_Links *app){return(app->get_microseconds_timestamp(app));}
static Vec2 draw_string(Application_Links *app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta){return(app->draw_string(app, font_id, str, point, color, flags, delta));}
static f32 get_string_advance(Application_Links *app, Face_ID font_id, String_Const_u8 str){return(app->get_string_advance(app, font_id, str));}
static void draw_rectangle(Application_Links *app, Rect_f32 rect, int_color color){(app->draw_rectangle(app, rect, color));}
static void draw_rectangle_outline(Application_Links *app, f32_Rect rect, int_color color){(app->draw_rectangle_outline(app, rect, color));}
static void draw_clip_push(Application_Links *app, f32_Rect clip_box){(app->draw_clip_push(app, clip_box));}
static f32_Rect draw_clip_pop(Application_Links *app){return(app->draw_clip_pop(app));}
static void draw_coordinate_center_push(Application_Links *app, Vec2 point){(app->draw_coordinate_center_push(app, point));}
static Vec2 draw_coordinate_center_pop(Application_Links *app){return(app->draw_coordinate_center_pop(app));}
static b32 text_layout_get_buffer(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID *buffer_id_out){return(app->text_layout_get_buffer(app, text_layout_id, buffer_id_out));}
static b32 text_layout_buffer_point_to_layout_point(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 buffer_relative_p, Vec2 *p_out){return(app->text_layout_buffer_point_to_layout_point(app, text_layout_id, buffer_relative_p, p_out));}
static b32 text_layout_layout_point_to_buffer_point(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 layout_relative_p, Vec2 *p_out){return(app->text_layout_layout_point_to_buffer_point(app, text_layout_id, layout_relative_p, p_out));}
static b32 text_layout_get_on_screen_range(Application_Links *app, Text_Layout_ID text_layout_id, Range *on_screen_range_out){return(app->text_layout_get_on_screen_range(app, text_layout_id, on_screen_range_out));}
static b32 text_layout_get_height(Application_Links *app, Text_Layout_ID text_layout_id, f32 *height_out){return(app->text_layout_get_height(app, text_layout_id, height_out));}
static b32 text_layout_free(Application_Links *app, Text_Layout_ID text_layout_id){return(app->text_layout_free(app, text_layout_id));}
static b32 compute_render_layout(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Vec2 screen_p, Vec2 layout_dim, Buffer_Point buffer_point, i32 one_past_last, Text_Layout_ID *text_layout_id_out){return(app->compute_render_layout(app, view_id, buffer_id, screen_p, layout_dim, buffer_point, one_past_last, text_layout_id_out));}
static void draw_render_layout(Application_Links *app, View_ID view_id){(app->draw_render_layout(app, view_id));}
static void open_color_picker(Application_Links *app, color_picker *picker){(app->open_color_picker(app, picker));}
static void animate_in_n_milliseconds(Application_Links *app, u32 n){(app->animate_in_n_milliseconds(app, n));}
static String_Match_List find_all_matches_buffer_range(Application_Links *app, Arena *arena, Buffer_ID buffer, Range range, String_Const_u8 needle, Character_Predicate *predicate, Scan_Direction direction){return(app->find_all_matches_buffer_range(app, arena, buffer, range, needle, predicate, direction));}
static Range get_view_visible_range(Application_Links *app, View_ID view_id){return(app->get_view_visible_range(app, view_id));}
#else
static b32 global_set_setting(Application_Links *app, Global_Setting_ID setting, i32 value){return(app->global_set_setting_(app, setting, value));}
static b32 global_set_mapping(Application_Links *app, void *data, i32 size){return(app->global_set_mapping_(app, data, size));}
static b32 global_get_screen_rectangle(Application_Links *app, Rect_f32 *rect_out){return(app->global_get_screen_rectangle_(app, rect_out));}
static Arena* context_get_arena(Application_Links *app){return(app->context_get_arena_(app));}
static Base_Allocator* context_get_base_allocator(Application_Links *app){return(app->context_get_base_allocator_(app));}
static b32 create_child_process(Application_Links *app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *child_process_id_out){return(app->create_child_process_(app, path, command, child_process_id_out));}
static b32 child_process_set_target_buffer(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags){return(app->child_process_set_target_buffer_(app, child_process_id, buffer_id, flags));}
static b32 buffer_get_attached_child_process(Application_Links *app, Buffer_ID buffer_id, Child_Process_ID *child_process_id_out){return(app->buffer_get_attached_child_process_(app, buffer_id, child_process_id_out));}
static b32 child_process_get_attached_buffer(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID *buffer_id_out){return(app->child_process_get_attached_buffer_(app, child_process_id, buffer_id_out));}
static b32 child_process_get_state(Application_Links *app, Child_Process_ID child_process_id, Process_State *process_state_out){return(app->child_process_get_state_(app, child_process_id, process_state_out));}
static b32 clipboard_post(Application_Links *app, i32 clipboard_id, String_Const_u8 string){return(app->clipboard_post_(app, clipboard_id, string));}
static b32 clipboard_count(Application_Links *app, i32 clipboard_id, i32 *count_out){return(app->clipboard_count_(app, clipboard_id, count_out));}
static b32 clipboard_index(Application_Links *app, i32 clipboard_id, i32 item_index, Arena *out, String_Const_u8 *string_out){return(app->clipboard_index_(app, clipboard_id, item_index, out, string_out));}
static Parse_Context_ID create_parse_context(Application_Links *app, Parser_String_And_Type *kw, u32 kw_count, Parser_String_And_Type *pp, u32 pp_count){return(app->create_parse_context_(app, kw, kw_count, pp, pp_count));}
static i32 get_buffer_count(Application_Links *app){return(app->get_buffer_count_(app));}
static b32 get_buffer_next(Application_Links *app, Buffer_ID buffer_id, Access_Flag access, Buffer_ID *buffer_id_out){return(app->get_buffer_next_(app, buffer_id, access, buffer_id_out));}
static b32 get_buffer_by_name(Application_Links *app, String_Const_u8 name, Access_Flag access, Buffer_ID *buffer_id_out){return(app->get_buffer_by_name_(app, name, access, buffer_id_out));}
static b32 get_buffer_by_file_name(Application_Links *app, String_Const_u8 file_name, Access_Flag access, Buffer_ID *buffer_id_out){return(app->get_buffer_by_file_name_(app, file_name, access, buffer_id_out));}
static b32 buffer_read_range(Application_Links *app, Buffer_ID buffer_id, i32 start, i32 one_past_last, char *out){return(app->buffer_read_range_(app, buffer_id, start, one_past_last, out));}
static b32 buffer_replace_range(Application_Links *app, Buffer_ID buffer_id, Range range, String_Const_u8 string){return(app->buffer_replace_range_(app, buffer_id, range, string));}
static b32 buffer_batch_edit(Application_Links *app, Buffer_ID buffer_id, char *str, Buffer_Edit *edits, i32 edit_count){return(app->buffer_batch_edit_(app, buffer_id, str, edits, edit_count));}
static b32 buffer_seek_string(Application_Links *app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i32 start_pos, i32 *pos_out, b32 *case_sensitive_out){return(app->buffer_seek_string_(app, buffer, needle, direction, start_pos, pos_out, case_sensitive_out));}
static b32 buffer_seek_character_class(Application_Links *app, Buffer_ID buffer_id, Character_Predicate *predicate, Scan_Direction direction, i32 start_pos, i32 *pos_out){return(app->buffer_seek_character_class_(app, buffer_id, predicate, direction, start_pos, pos_out));}
static b32 buffer_compute_cursor(Application_Links *app, Buffer_ID buffer_id, Buffer_Seek seek, Partial_Cursor *cursor_out){return(app->buffer_compute_cursor_(app, buffer_id, seek, cursor_out));}
static b32 buffer_exists(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_exists_(app, buffer_id));}
static b32 buffer_ready(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_ready_(app, buffer_id));}
static b32 buffer_get_access_flags(Application_Links *app, Buffer_ID buffer_id, Access_Flag *access_flags_out){return(app->buffer_get_access_flags_(app, buffer_id, access_flags_out));}
static b32 buffer_get_size(Application_Links *app, Buffer_ID buffer_id, i32 *size_out){return(app->buffer_get_size_(app, buffer_id, size_out));}
static b32 buffer_get_line_count(Application_Links *app, Buffer_ID buffer_id, i32 *line_count_out){return(app->buffer_get_line_count_(app, buffer_id, line_count_out));}
static b32 buffer_get_base_buffer_name(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out){return(app->buffer_get_base_buffer_name_(app, buffer_id, out, name_out));}
static b32 buffer_get_unique_buffer_name(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out){return(app->buffer_get_unique_buffer_name_(app, buffer_id, out, name_out));}
static b32 buffer_get_file_name(Application_Links *app, Buffer_ID buffer_id, Arena *out, String_Const_u8 *name_out){return(app->buffer_get_file_name_(app, buffer_id, out, name_out));}
static b32 buffer_get_dirty_state(Application_Links *app, Buffer_ID buffer_id, Dirty_State *dirty_state_out){return(app->buffer_get_dirty_state_(app, buffer_id, dirty_state_out));}
static b32 buffer_directly_set_dirty_state(Application_Links *app, Buffer_ID buffer_id, Dirty_State dirty_state){return(app->buffer_directly_set_dirty_state_(app, buffer_id, dirty_state));}
static b32 buffer_tokens_are_ready(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_tokens_are_ready_(app, buffer_id));}
static b32 buffer_get_setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 *value_out){return(app->buffer_get_setting_(app, buffer_id, setting, value_out));}
static b32 buffer_set_setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 value){return(app->buffer_set_setting_(app, buffer_id, setting, value));}
static b32 buffer_get_managed_scope(Application_Links *app, Buffer_ID buffer_id, Managed_Scope *scope_out){return(app->buffer_get_managed_scope_(app, buffer_id, scope_out));}
static b32 buffer_token_count(Application_Links *app, Buffer_ID buffer_id, i32 *count_out){return(app->buffer_token_count_(app, buffer_id, count_out));}
static b32 buffer_read_tokens(Application_Links *app, Buffer_ID buffer_id, i32 start_token, i32 end_token, Cpp_Token *tokens_out){return(app->buffer_read_tokens_(app, buffer_id, start_token, end_token, tokens_out));}
static b32 buffer_get_token_range(Application_Links *app, Buffer_ID buffer_id, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out){return(app->buffer_get_token_range_(app, buffer_id, first_token_out, one_past_last_token_out));}
static b32 buffer_get_token_index(Application_Links *app, Buffer_ID buffer_id, i32 pos, Cpp_Get_Token_Result *get_result){return(app->buffer_get_token_index_(app, buffer_id, pos, get_result));}
static b32 buffer_send_end_signal(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_send_end_signal_(app, buffer_id));}
static b32 create_buffer(Application_Links *app, String_Const_u8 file_name, Buffer_Create_Flag flags, Buffer_ID *new_buffer_id_out){return(app->create_buffer_(app, file_name, flags, new_buffer_id_out));}
static b32 buffer_save(Application_Links *app, Buffer_ID buffer_id, String_Const_u8 file_name, u32 flags){return(app->buffer_save_(app, buffer_id, file_name, flags));}
static b32 buffer_kill(Application_Links *app, Buffer_ID buffer_id, Buffer_Kill_Flag flags, Buffer_Kill_Result *result_out){return(app->buffer_kill_(app, buffer_id, flags, result_out));}
static b32 buffer_reopen(Application_Links *app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags, Buffer_Reopen_Result *result_out){return(app->buffer_reopen_(app, buffer_id, flags, result_out));}
static b32 buffer_get_file_attributes(Application_Links *app, Buffer_ID buffer_id, File_Attributes *attributes_out){return(app->buffer_get_file_attributes_(app, buffer_id, attributes_out));}
static b32 get_view_next(Application_Links *app, View_ID view_id, Access_Flag access, View_ID *view_id_out){return(app->get_view_next_(app, view_id, access, view_id_out));}
static b32 get_view_prev(Application_Links *app, View_ID view_id, Access_Flag access, View_ID *view_id_out){return(app->get_view_prev_(app, view_id, access, view_id_out));}
static b32 get_active_view(Application_Links *app, Access_Flag access, View_ID *view_id_out){return(app->get_active_view_(app, access, view_id_out));}
static b32 get_active_panel(Application_Links *app, Panel_ID *panel_id_out){return(app->get_active_panel_(app, panel_id_out));}
static b32 view_exists(Application_Links *app, View_ID view_id){return(app->view_exists_(app, view_id));}
static b32 view_get_buffer(Application_Links *app, View_ID view_id, Access_Flag access, Buffer_ID *buffer_id_out){return(app->view_get_buffer_(app, view_id, access, buffer_id_out));}
static b32 view_get_cursor_pos(Application_Links *app, View_ID view_id, i32 *pos_out){return(app->view_get_cursor_pos_(app, view_id, pos_out));}
static b32 view_get_mark_pos(Application_Links *app, View_ID view_id, i32 *pos_out){return(app->view_get_mark_pos_(app, view_id, pos_out));}
static b32 view_get_preferred_x(Application_Links *app, View_ID view_id, f32 *preferred_x_out){return(app->view_get_preferred_x_(app, view_id, preferred_x_out));}
static b32 view_get_screen_rect(Application_Links *app, View_ID view_id, Rect_f32 *rect_out){return(app->view_get_screen_rect_(app, view_id, rect_out));}
static b32 view_get_panel(Application_Links *app, View_ID view_id, Panel_ID *panel_id_out){return(app->view_get_panel_(app, view_id, panel_id_out));}
static b32 panel_get_view(Application_Links *app, Panel_ID panel_id, View_ID *view_id_out){return(app->panel_get_view_(app, panel_id, view_id_out));}
static b32 panel_is_split(Application_Links *app, Panel_ID panel_id){return(app->panel_is_split_(app, panel_id));}
static b32 panel_is_leaf(Application_Links *app, Panel_ID panel_id){return(app->panel_is_leaf_(app, panel_id));}
static b32 panel_split(Application_Links *app, Panel_ID panel_id, Panel_Split_Orientation orientation){return(app->panel_split_(app, panel_id, orientation));}
static b32 panel_set_split(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t){return(app->panel_set_split_(app, panel_id, kind, t));}
static b32 panel_swap_children(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t){return(app->panel_swap_children_(app, panel_id, kind, t));}
static b32 panel_get_parent(Application_Links *app, Panel_ID panel_id, Panel_ID *panel_id_out){return(app->panel_get_parent_(app, panel_id, panel_id_out));}
static b32 panel_get_child(Application_Links *app, Panel_ID panel_id, Panel_Child which_child, Panel_ID *panel_id_out){return(app->panel_get_child_(app, panel_id, which_child, panel_id_out));}
static b32 panel_get_max(Application_Links *app, Panel_ID panel_id, Panel_ID *panel_id_out){return(app->panel_get_max_(app, panel_id, panel_id_out));}
static b32 panel_get_margin(Application_Links *app, Panel_ID panel_id, i32_Rect *margins_out){return(app->panel_get_margin_(app, panel_id, margins_out));}
static b32 view_close(Application_Links *app, View_ID view_id){return(app->view_close_(app, view_id));}
static b32 view_get_region(Application_Links *app, View_ID view_id, Rect_i32 *region_out){return(app->view_get_region_(app, view_id, region_out));}
static b32 view_get_buffer_region(Application_Links *app, View_ID view_id, Rect_i32 *region_out){return(app->view_get_buffer_region_(app, view_id, region_out));}
static b32 view_get_scroll_vars(Application_Links *app, View_ID view_id, GUI_Scroll_Vars *scroll_vars_out){return(app->view_get_scroll_vars_(app, view_id, scroll_vars_out));}
static b32 view_set_active(Application_Links *app, View_ID view_id){return(app->view_set_active_(app, view_id));}
static b32 view_get_setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 *value_out){return(app->view_get_setting_(app, view_id, setting, value_out));}
static b32 view_set_setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 value){return(app->view_set_setting_(app, view_id, setting, value));}
static b32 view_get_managed_scope(Application_Links *app, View_ID view_id, Managed_Scope *scope){return(app->view_get_managed_scope_(app, view_id, scope));}
static b32 view_compute_cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek, Full_Cursor *cursor_out){return(app->view_compute_cursor_(app, view_id, seek, cursor_out));}
static b32 view_set_cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek, b32 set_preferred_x){return(app->view_set_cursor_(app, view_id, seek, set_preferred_x));}
static b32 view_set_scroll(Application_Links *app, View_ID view_id, GUI_Scroll_Vars scroll){return(app->view_set_scroll_(app, view_id, scroll));}
static b32 view_set_mark(Application_Links *app, View_ID view_id, Buffer_Seek seek){return(app->view_set_mark_(app, view_id, seek));}
static b32 view_set_buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags){return(app->view_set_buffer_(app, view_id, buffer_id, flags));}
static b32 view_post_fade(Application_Links *app, View_ID view_id, float seconds, i32 start, i32 end, int_color color){return(app->view_post_fade_(app, view_id, seconds, start, end, color));}
static b32 view_begin_ui_mode(Application_Links *app, View_ID view_id){return(app->view_begin_ui_mode_(app, view_id));}
static b32 view_end_ui_mode(Application_Links *app, View_ID view_id){return(app->view_end_ui_mode_(app, view_id));}
static b32 view_is_in_ui_mode(Application_Links *app, View_ID view_id){return(app->view_is_in_ui_mode_(app, view_id));}
static b32 view_set_quit_ui_handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type *quit_function){return(app->view_set_quit_ui_handler_(app, view_id, quit_function));}
static b32 view_get_quit_ui_handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type **quit_function_out){return(app->view_get_quit_ui_handler_(app, view_id, quit_function_out));}
static Managed_Scope create_user_managed_scope(Application_Links *app){return(app->create_user_managed_scope_(app));}
static b32 destroy_user_managed_scope(Application_Links *app, Managed_Scope scope){return(app->destroy_user_managed_scope_(app, scope));}
static Managed_Scope get_global_managed_scope(Application_Links *app){return(app->get_global_managed_scope_(app));}
static Managed_Scope get_managed_scope_with_multiple_dependencies(Application_Links *app, Managed_Scope *scopes, i32 count){return(app->get_managed_scope_with_multiple_dependencies_(app, scopes, count));}
static b32 managed_scope_clear_contents(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_contents_(app, scope));}
static b32 managed_scope_clear_self_all_dependent_scopes(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_self_all_dependent_scopes_(app, scope));}
static Managed_Variable_ID managed_variable_create(Application_Links *app, char *null_terminated_name, u64 default_value){return(app->managed_variable_create_(app, null_terminated_name, default_value));}
static Managed_Variable_ID managed_variable_get_id(Application_Links *app, char *null_terminated_name){return(app->managed_variable_get_id_(app, null_terminated_name));}
static Managed_Variable_ID managed_variable_create_or_get_id(Application_Links *app, char *null_terminated_name, u64 default_value){return(app->managed_variable_create_or_get_id_(app, null_terminated_name, default_value));}
static b32 managed_variable_set(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 value){return(app->managed_variable_set_(app, scope, id, value));}
static b32 managed_variable_get(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 *value_out){return(app->managed_variable_get_(app, scope, id, value_out));}
static Managed_Object alloc_managed_memory_in_scope(Application_Links *app, Managed_Scope scope, i32 item_size, i32 count){return(app->alloc_managed_memory_in_scope_(app, scope, item_size, count));}
static Managed_Object alloc_buffer_markers_on_buffer(Application_Links *app, Buffer_ID buffer_id, i32 count, Managed_Scope *optional_extra_scope){return(app->alloc_buffer_markers_on_buffer_(app, buffer_id, count, optional_extra_scope));}
static Managed_Object alloc_managed_arena_in_scope(Application_Links *app, Managed_Scope scope, i32 page_size){return(app->alloc_managed_arena_in_scope_(app, scope, page_size));}
static Marker_Visual create_marker_visual(Application_Links *app, Managed_Object object){return(app->create_marker_visual_(app, object));}
static b32 marker_visual_set_effect(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style){return(app->marker_visual_set_effect_(app, visual, type, color, text_color, text_style));}
static b32 marker_visual_set_take_rule(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule){return(app->marker_visual_set_take_rule_(app, visual, take_rule));}
static b32 marker_visual_set_priority(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority){return(app->marker_visual_set_priority_(app, visual, priority));}
static b32 marker_visual_set_view_key(Application_Links *app, Marker_Visual visual, View_ID key_view_id){return(app->marker_visual_set_view_key_(app, visual, key_view_id));}
static b32 destroy_marker_visual(Application_Links *app, Marker_Visual visual){return(app->destroy_marker_visual_(app, visual));}
static i32 buffer_markers_get_attached_visual_count(Application_Links *app, Managed_Object object){return(app->buffer_markers_get_attached_visual_count_(app, object));}
static Marker_Visual* buffer_markers_get_attached_visual(Application_Links *app, Arena *arena, Managed_Object object){return(app->buffer_markers_get_attached_visual_(app, arena, object));}
static u32 managed_object_get_item_size(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_size_(app, object));}
static u32 managed_object_get_item_count(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_count_(app, object));}
static Managed_Object_Type managed_object_get_type(Application_Links *app, Managed_Object object){return(app->managed_object_get_type_(app, object));}
static Managed_Scope managed_object_get_containing_scope(Application_Links *app, Managed_Object object){return(app->managed_object_get_containing_scope_(app, object));}
static b32 managed_object_free(Application_Links *app, Managed_Object object){return(app->managed_object_free_(app, object));}
static b32 managed_object_store_data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem){return(app->managed_object_store_data_(app, object, first_index, count, mem));}
static b32 managed_object_load_data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem_out){return(app->managed_object_load_data_(app, object, first_index, count, mem_out));}
static User_Input get_user_input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type){return(app->get_user_input_(app, get_type, abort_type));}
static User_Input get_command_input(Application_Links *app){return(app->get_command_input_(app));}
static void set_command_input(Application_Links *app, Key_Event_Data key_data){(app->set_command_input_(app, key_data));}
static Mouse_State get_mouse_state(Application_Links *app){return(app->get_mouse_state_(app));}
static b32 get_active_query_bars(Application_Links *app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array *array_out){return(app->get_active_query_bars_(app, view_id, max_result_count, array_out));}
static b32 start_query_bar(Application_Links *app, Query_Bar *bar, u32 flags){return(app->start_query_bar_(app, bar, flags));}
static void end_query_bar(Application_Links *app, Query_Bar *bar, u32 flags){(app->end_query_bar_(app, bar, flags));}
static b32 print_message(Application_Links *app, String_Const_u8 message){return(app->print_message_(app, message));}
static Face_ID get_largest_face_id(Application_Links *app){return(app->get_largest_face_id_(app));}
static b32 set_global_face(Application_Links *app, Face_ID id, b32 apply_to_all_buffers){return(app->set_global_face_(app, id, apply_to_all_buffers));}
static b32 buffer_history_get_max_record_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index *index_out){return(app->buffer_history_get_max_record_index_(app, buffer_id, index_out));}
static b32 buffer_history_get_record_info(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, Record_Info *record_out){return(app->buffer_history_get_record_info_(app, buffer_id, index, record_out));}
static b32 buffer_history_get_group_sub_record(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index, Record_Info *record_out){return(app->buffer_history_get_group_sub_record_(app, buffer_id, index, sub_index, record_out));}
static b32 buffer_history_get_current_state_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index *index_out){return(app->buffer_history_get_current_state_index_(app, buffer_id, index_out));}
static b32 buffer_history_set_current_state_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){return(app->buffer_history_set_current_state_index_(app, buffer_id, index));}
static b32 buffer_history_merge_record_range(Application_Links *app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){return(app->buffer_history_merge_record_range_(app, buffer_id, first_index, last_index, flags));}
static b32 buffer_history_clear_after_current_state(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_history_clear_after_current_state_(app, buffer_id));}
static void global_history_edit_group_begin(Application_Links *app){(app->global_history_edit_group_begin_(app));}
static void global_history_edit_group_end(Application_Links *app){(app->global_history_edit_group_end_(app));}
static b32 buffer_set_face(Application_Links *app, Buffer_ID buffer_id, Face_ID id){return(app->buffer_set_face_(app, buffer_id, id));}
static Face_Description get_face_description(Application_Links *app, Face_ID id){return(app->get_face_description_(app, id));}
static b32 get_face_metrics(Application_Links *app, Face_ID face_id, Face_Metrics *metrics_out){return(app->get_face_metrics_(app, face_id, metrics_out));}
static b32 get_face_id(Application_Links *app, Buffer_ID buffer_id, Face_ID *face_id_out){return(app->get_face_id_(app, buffer_id, face_id_out));}
static Face_ID try_create_new_face(Application_Links *app, Face_Description *description){return(app->try_create_new_face_(app, description));}
static b32 try_modify_face(Application_Links *app, Face_ID id, Face_Description *description){return(app->try_modify_face_(app, id, description));}
static b32 try_release_face(Application_Links *app, Face_ID id, Face_ID replacement_id){return(app->try_release_face_(app, id, replacement_id));}
static i32 get_available_font_count(Application_Links *app){return(app->get_available_font_count_(app));}
static Available_Font get_available_font(Application_Links *app, i32 index){return(app->get_available_font_(app, index));}
static void set_theme_colors(Application_Links *app, Theme_Color *colors, i32 count){(app->set_theme_colors_(app, colors, count));}
static void get_theme_colors(Application_Links *app, Theme_Color *colors, i32 count){(app->get_theme_colors_(app, colors, count));}
static argb_color finalize_color(Application_Links *app, int_color color){return(app->finalize_color_(app, color));}
static b32 get_hot_directory(Application_Links *app, Arena *out, String_Const_u8 *out_directory){return(app->get_hot_directory_(app, out, out_directory));}
static b32 set_hot_directory(Application_Links *app, String_Const_u8 string){return(app->set_hot_directory_(app, string));}
static b32 get_file_list(Application_Links *app, String_Const_u8 directory, File_List *list_out){return(app->get_file_list_(app, directory, list_out));}
static void free_file_list(Application_Links *app, File_List list){(app->free_file_list_(app, list));}
static void set_gui_up_down_keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier){(app->set_gui_up_down_keys_(app, up_key, up_key_modifier, down_key, down_key_modifier));}
static void* memory_allocate(Application_Links *app, i32 size){return(app->memory_allocate_(app, size));}
static b32 memory_set_protection(Application_Links *app, void *ptr, i32 size, Memory_Protect_Flags flags){return(app->memory_set_protection_(app, ptr, size, flags));}
static void memory_free(Application_Links *app, void *ptr, i32 size){(app->memory_free_(app, ptr, size));}
static b32 file_get_attributes(Application_Links *app, String_Const_u8 file_name, File_Attributes *attributes_out){return(app->file_get_attributes_(app, file_name, attributes_out));}
static b32 get_4ed_path(Application_Links *app, Arena *out, String_Const_u8 *path_out){return(app->get_4ed_path_(app, out, path_out));}
static void show_mouse_cursor(Application_Links *app, Mouse_Cursor_Show_Type show){(app->show_mouse_cursor_(app, show));}
static b32 set_edit_finished_hook_repeat_speed(Application_Links *app, u32 milliseconds){return(app->set_edit_finished_hook_repeat_speed_(app, milliseconds));}
static b32 set_fullscreen(Application_Links *app, b32 full_screen){return(app->set_fullscreen_(app, full_screen));}
static b32 is_fullscreen(Application_Links *app){return(app->is_fullscreen_(app));}
static void send_exit_signal(Application_Links *app){(app->send_exit_signal_(app));}
static b32 set_window_title(Application_Links *app, String_Const_u8 title){return(app->set_window_title_(app, title));}
static Microsecond_Time_Stamp get_microseconds_timestamp(Application_Links *app){return(app->get_microseconds_timestamp_(app));}
static Vec2 draw_string(Application_Links *app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta){return(app->draw_string_(app, font_id, str, point, color, flags, delta));}
static f32 get_string_advance(Application_Links *app, Face_ID font_id, String_Const_u8 str){return(app->get_string_advance_(app, font_id, str));}
static void draw_rectangle(Application_Links *app, Rect_f32 rect, int_color color){(app->draw_rectangle_(app, rect, color));}
static void draw_rectangle_outline(Application_Links *app, f32_Rect rect, int_color color){(app->draw_rectangle_outline_(app, rect, color));}
static void draw_clip_push(Application_Links *app, f32_Rect clip_box){(app->draw_clip_push_(app, clip_box));}
static f32_Rect draw_clip_pop(Application_Links *app){return(app->draw_clip_pop_(app));}
static void draw_coordinate_center_push(Application_Links *app, Vec2 point){(app->draw_coordinate_center_push_(app, point));}
static Vec2 draw_coordinate_center_pop(Application_Links *app){return(app->draw_coordinate_center_pop_(app));}
static b32 text_layout_get_buffer(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID *buffer_id_out){return(app->text_layout_get_buffer_(app, text_layout_id, buffer_id_out));}
static b32 text_layout_buffer_point_to_layout_point(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 buffer_relative_p, Vec2 *p_out){return(app->text_layout_buffer_point_to_layout_point_(app, text_layout_id, buffer_relative_p, p_out));}
static b32 text_layout_layout_point_to_buffer_point(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 layout_relative_p, Vec2 *p_out){return(app->text_layout_layout_point_to_buffer_point_(app, text_layout_id, layout_relative_p, p_out));}
static b32 text_layout_get_on_screen_range(Application_Links *app, Text_Layout_ID text_layout_id, Range *on_screen_range_out){return(app->text_layout_get_on_screen_range_(app, text_layout_id, on_screen_range_out));}
static b32 text_layout_get_height(Application_Links *app, Text_Layout_ID text_layout_id, f32 *height_out){return(app->text_layout_get_height_(app, text_layout_id, height_out));}
static b32 text_layout_free(Application_Links *app, Text_Layout_ID text_layout_id){return(app->text_layout_free_(app, text_layout_id));}
static b32 compute_render_layout(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Vec2 screen_p, Vec2 layout_dim, Buffer_Point buffer_point, i32 one_past_last, Text_Layout_ID *text_layout_id_out){return(app->compute_render_layout_(app, view_id, buffer_id, screen_p, layout_dim, buffer_point, one_past_last, text_layout_id_out));}
static void draw_render_layout(Application_Links *app, View_ID view_id){(app->draw_render_layout_(app, view_id));}
static void open_color_picker(Application_Links *app, color_picker *picker){(app->open_color_picker_(app, picker));}
static void animate_in_n_milliseconds(Application_Links *app, u32 n){(app->animate_in_n_milliseconds_(app, n));}
static String_Match_List find_all_matches_buffer_range(Application_Links *app, Arena *arena, Buffer_ID buffer, Range range, String_Const_u8 needle, Character_Predicate *predicate, Scan_Direction direction){return(app->find_all_matches_buffer_range_(app, arena, buffer, range, needle, predicate, direction));}
static Range get_view_visible_range(Application_Links *app, View_ID view_id){return(app->get_view_visible_range_(app, view_id));}
#endif
