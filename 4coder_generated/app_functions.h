struct Application_Links;
#define GLOBAL_SET_SETTING_SIG(n) bool32 n(Application_Links *app, Global_Setting_ID setting, int32_t value)
#define GLOBAL_SET_MAPPING_SIG(n) bool32 n(Application_Links *app, void *data, int32_t size)
#define EXEC_SYSTEM_COMMAND_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Identifier buffer_id, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags)
#define CLIPBOARD_POST_SIG(n) void n(Application_Links *app, int32_t clipboard_id, char *str, int32_t len)
#define CLIPBOARD_COUNT_SIG(n) int32_t n(Application_Links *app, int32_t clipboard_id)
#define CLIPBOARD_INDEX_SIG(n) int32_t n(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len)
#define CREATE_PARSE_CONTEXT_SIG(n) Parse_Context_ID n(Application_Links *app, Parser_String_And_Type *kw, uint32_t kw_count, Parser_String_And_Type *pp, uint32_t pp_count)
#define GET_BUFFER_COUNT_SIG(n) int32_t n(Application_Links *app)
#define GET_BUFFER_FIRST_SIG(n) Buffer_Summary n(Application_Links *app, Access_Flag access)
#define GET_BUFFER_NEXT_SIG(n) void n(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access)
#define GET_BUFFER_SIG(n) Buffer_Summary n(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
#define GET_BUFFER_BY_NAME_SIG(n) Buffer_Summary n(Application_Links *app, char *name, int32_t len, Access_Flag access)
#define GET_BUFFER_BY_FILE_NAME_SIG(n) Buffer_Summary n(Application_Links *app, char *name, int32_t len, Access_Flag access)
#define BUFFER_READ_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *out)
#define BUFFER_REPLACE_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *str, int32_t len)
#define BUFFER_COMPUTE_CURSOR_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out)
#define BUFFER_BATCH_EDIT_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type)
#define BUFFER_GET_SETTING_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out)
#define BUFFER_SET_SETTING_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value)
#define BUFFER_GET_MANAGED_SCOPE_SIG(n) Managed_Scope n(Application_Links *app, Buffer_ID buffer_id)
#define BUFFER_TOKEN_COUNT_SIG(n) int32_t n(Application_Links *app, Buffer_Summary *buffer)
#define BUFFER_READ_TOKENS_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t start_token, int32_t end_token, Cpp_Token *tokens_out)
#define BUFFER_GET_TOKEN_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out)
#define BUFFER_GET_TOKEN_INDEX_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t pos, Cpp_Get_Token_Result *get_result)
#define BUFFER_SEND_END_SIGNAL_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer)
#define CREATE_BUFFER_SIG(n) Buffer_Summary n(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags)
#define SAVE_BUFFER_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *file_name, int32_t file_name_len, uint32_t flags)
#define KILL_BUFFER_SIG(n) Buffer_Kill_Result n(Application_Links *app, Buffer_Identifier buffer, Buffer_Kill_Flag flags)
#define REOPEN_BUFFER_SIG(n) Buffer_Reopen_Result n(Application_Links *app, Buffer_Summary *buffer, Buffer_Reopen_Flag flags)
#define GET_VIEW_FIRST_SIG(n) View_Summary n(Application_Links *app, Access_Flag access)
#define GET_VIEW_NEXT_SIG(n) void n(Application_Links *app, View_Summary *view, Access_Flag access)
#define GET_VIEW_SIG(n) View_Summary n(Application_Links *app, View_ID view_id, Access_Flag access)
#define GET_ACTIVE_VIEW_SIG(n) View_Summary n(Application_Links *app, Access_Flag access)
#define OPEN_VIEW_SIG(n) View_Summary n(Application_Links *app, View_Summary *view_location, View_Split_Position position)
#define CLOSE_VIEW_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define SET_ACTIVE_VIEW_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define VIEW_GET_SETTING_SIG(n) bool32 n(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out)
#define VIEW_SET_SETTING_SIG(n) bool32 n(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value)
#define VIEW_GET_MANAGED_SCOPE_SIG(n) Managed_Scope n(Application_Links *app, View_ID view_id)
#define VIEW_SET_SPLIT_SIG(n) bool32 n(Application_Links *app, View_Summary *view, View_Split_Kind kind, float t)
#define VIEW_GET_ENCLOSURE_RECT_SIG(n) i32_Rect n(Application_Links *app, View_Summary *view)
#define VIEW_COMPUTE_CURSOR_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out)
#define VIEW_SET_CURSOR_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x)
#define VIEW_SET_SCROLL_SIG(n) bool32 n(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll)
#define VIEW_SET_MARK_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek)
#define VIEW_SET_HIGHLIGHT_SIG(n) bool32 n(Application_Links *app, View_Summary *view, int32_t start, int32_t end, bool32 turn_on)
#define VIEW_SET_BUFFER_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags)
#define VIEW_POST_FADE_SIG(n) bool32 n(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color)
#define VIEW_BEGIN_UI_MODE_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define VIEW_END_UI_MODE_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define VIEW_SET_UI_SIG(n) bool32 n(Application_Links *app, View_Summary *view, UI_Control *control, UI_Quit_Function_Type *quit_function)
#define VIEW_GET_UI_COPY_SIG(n) UI_Control n(Application_Links *app, View_Summary *view, struct Partition *part)
#define CREATE_USER_MANAGED_SCOPE_SIG(n) Managed_Scope n(Application_Links *app)
#define DESTROY_USER_MANAGED_SCOPE_SIG(n) bool32 n(Application_Links *app, Managed_Scope scope)
#define GET_GLOBAL_MANAGED_SCOPE_SIG(n) Managed_Scope n(Application_Links *app)
#define GET_MANAGED_SCOPE_WITH_MULTIPLE_DEPENDENCIES_SIG(n) Managed_Scope n(Application_Links *app, Managed_Scope *scopes, int32_t count)
#define MANAGED_SCOPE_CLEAR_CONTENTS_SIG(n) bool32 n(Application_Links *app, Managed_Scope scope)
#define MANAGED_SCOPE_CLEAR_SELF_ALL_DEPENDENT_SCOPES_SIG(n) bool32 n(Application_Links *app, Managed_Scope scope)
#define MANAGED_VARIABLE_CREATE_SIG(n) Managed_Variable_ID n(Application_Links *app, char *null_terminated_name, uint64_t default_value)
#define MANAGED_VARIABLE_GET_ID_SIG(n) Managed_Variable_ID n(Application_Links *app, char *null_terminated_name)
#define MANAGED_VARIABLE_CREATE_OR_GET_ID_SIG(n) Managed_Variable_ID n(Application_Links *app, char *null_terminated_name, uint64_t default_value)
#define MANAGED_VARIABLE_SET_SIG(n) bool32 n(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t value)
#define MANAGED_VARIABLE_GET_SIG(n) bool32 n(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t *value_out)
#define ALLOC_MANAGED_MEMORY_IN_SCOPE_SIG(n) Managed_Object n(Application_Links *app, Managed_Scope scope, int32_t item_size, int32_t count)
#define ALLOC_BUFFER_MARKERS_ON_BUFFER_SIG(n) Managed_Object n(Application_Links *app, Buffer_ID buffer_id, int32_t count, Managed_Scope *optional_extra_scope)
#define CREATE_MARKER_VISUAL_SIG(n) Marker_Visual n(Application_Links *app, Managed_Object object)
#define MARKER_VISUAL_SET_EFFECT_SIG(n) bool32 n(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style)
#define MARKER_VISUAL_SET_TAKE_RULE_SIG(n) bool32 n(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule)
#define MARKER_VISUAL_SET_PRIORITY_SIG(n) bool32 n(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority)
#define MARKER_VISUAL_SET_VIEW_KEY_SIG(n) bool32 n(Application_Links *app, Marker_Visual visual, View_ID key_view_id)
#define DESTROY_MARKER_VISUAL_SIG(n) bool32 n(Application_Links *app, Marker_Visual visual)
#define BUFFER_MARKERS_GET_ATTACHED_VISUAL_COUNT_SIG(n) int32_t n(Application_Links *app, Managed_Object object)
#define BUFFER_MARKERS_GET_ATTACHED_VISUAL_SIG(n) Marker_Visual* n(Application_Links *app, Partition *part, Managed_Object object)
#define MANAGED_OBJECT_GET_ITEM_SIZE_SIG(n) uint32_t n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_GET_ITEM_COUNT_SIG(n) uint32_t n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_GET_TYPE_SIG(n) Managed_Object_Type n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_GET_CONTAINING_SCOPE_SIG(n) Managed_Scope n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_FREE_SIG(n) bool32 n(Application_Links *app, Managed_Object object)
#define MANAGED_OBJECT_STORE_DATA_SIG(n) bool32 n(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem)
#define MANAGED_OBJECT_LOAD_DATA_SIG(n) bool32 n(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem_out)
#define GET_USER_INPUT_SIG(n) User_Input n(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
#define GET_COMMAND_INPUT_SIG(n) User_Input n(Application_Links *app)
#define SET_COMMAND_INPUT_SIG(n) void n(Application_Links *app, Key_Event_Data key_data)
#define GET_MOUSE_STATE_SIG(n) Mouse_State n(Application_Links *app)
#define GET_ACTIVE_QUERY_BARS_SIG(n) int32_t n(Application_Links *app, View_ID view_id, int32_t max_result_count, Query_Bar **result_array)
#define START_QUERY_BAR_SIG(n) bool32 n(Application_Links *app, Query_Bar *bar, uint32_t flags)
#define END_QUERY_BAR_SIG(n) void n(Application_Links *app, Query_Bar *bar, uint32_t flags)
#define PRINT_MESSAGE_SIG(n) void n(Application_Links *app, char *str, int32_t len)
#define GET_THEME_COUNT_SIG(n) int32_t n(Application_Links *app)
#define GET_THEME_NAME_SIG(n) String n(Application_Links *app, struct Partition *arena, int32_t index)
#define CREATE_THEME_SIG(n) void n(Application_Links *app, Theme *theme, char *name, int32_t len)
#define CHANGE_THEME_SIG(n) void n(Application_Links *app, char *name, int32_t len)
#define CHANGE_THEME_BY_INDEX_SIG(n) bool32 n(Application_Links *app, int32_t index)
#define GET_LARGEST_FACE_ID_SIG(n) Face_ID n(Application_Links *app)
#define SET_GLOBAL_FACE_SIG(n) bool32 n(Application_Links *app, Face_ID id, bool32 apply_to_all_buffers)
#define BUFFER_SET_FACE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Face_ID id)
#define BUFFER_HISTORY_NEWEST_RECORD_INDEX_SIG(n) History_Record_Index n(Application_Links *app, Buffer_Summary *buffer)
#define BUFFER_HISTORY_GET_RECORD_INFO_SIG(n) Record_Info n(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index)
#define BUFFER_HISTORY_GET_GROUP_SUB_RECORD_SIG(n) Record_Info n(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index, int32_t sub_index)
#define BUFFER_HISTORY_GET_CURRENT_STATE_INDEX_SIG(n) History_Record_Index n(Application_Links *app, Buffer_Summary *buffer)
#define BUFFER_HISTORY_SET_CURRENT_STATE_INDEX_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index)
#define BUFFER_HISTORY_MERGE_RECORD_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, History_Record_Index first_index, History_Record_Index last_index)
#define BUFFER_HISTORY_CLEAR_AFTER_CURRENT_STATE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer)
#define GLOBAL_HISTORY_EDIT_GROUP_BEGIN_SIG(n) void n(Application_Links *app)
#define GLOBAL_HISTORY_EDIT_GROUP_END_SIG(n) void n(Application_Links *app)
#define GET_FACE_DESCRIPTION_SIG(n) Face_Description n(Application_Links *app, Face_ID id)
#define GET_FACE_ID_SIG(n) Face_ID n(Application_Links *app, Buffer_Summary *buffer)
#define TRY_CREATE_NEW_FACE_SIG(n) Face_ID n(Application_Links *app, Face_Description *description)
#define TRY_MODIFY_FACE_SIG(n) bool32 n(Application_Links *app, Face_ID id, Face_Description *description)
#define TRY_RELEASE_FACE_SIG(n) bool32 n(Application_Links *app, Face_ID id, Face_ID replacement_id)
#define GET_AVAILABLE_FONT_COUNT_SIG(n) int32_t n(Application_Links *app)
#define GET_AVAILABLE_FONT_SIG(n) Available_Font n(Application_Links *app, int32_t index)
#define SET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, int32_t count)
#define GET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, int32_t count)
#define DIRECTORY_GET_HOT_SIG(n) int32_t n(Application_Links *app, char *out, int32_t capacity)
#define DIRECTORY_SET_HOT_SIG(n) bool32 n(Application_Links *app, char *str, int32_t len)
#define GET_FILE_LIST_SIG(n) File_List n(Application_Links *app, char *dir, int32_t len)
#define FREE_FILE_LIST_SIG(n) void n(Application_Links *app, File_List list)
#define SET_GUI_UP_DOWN_KEYS_SIG(n) void n(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier)
#define MEMORY_ALLOCATE_SIG(n) void* n(Application_Links *app, int32_t size)
#define MEMORY_SET_PROTECTION_SIG(n) bool32 n(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags)
#define MEMORY_FREE_SIG(n) void n(Application_Links *app, void *ptr, int32_t size)
#define FILE_EXISTS_SIG(n) bool32 n(Application_Links *app, char *filename, int32_t len)
#define DIRECTORY_CD_SIG(n) bool32 n(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len)
#define GET_4ED_PATH_SIG(n) int32_t n(Application_Links *app, char *out, int32_t capacity)
#define SHOW_MOUSE_CURSOR_SIG(n) void n(Application_Links *app, Mouse_Cursor_Show_Type show)
#define SET_FULLSCREEN_SIG(n) bool32 n(Application_Links *app, bool32 full_screen)
#define IS_FULLSCREEN_SIG(n) bool32 n(Application_Links *app)
#define SEND_EXIT_SIGNAL_SIG(n) void n(Application_Links *app)
#define SET_WINDOW_TITLE_SIG(n) void n(Application_Links *app, char *title)
#define GET_MICROSECONDS_TIMESTAMP_SIG(n) Microsecond_Time_Stamp n(Application_Links *app)
#define DRAW_STRING_SIG(n) float n(Application_Links *app, Face_ID font_id, String str, int32_t x, int32_t y, int_color color, uint32_t flags, float dx, float dy)
#define GET_STRING_ADVANCE_SIG(n) float n(Application_Links *app, Face_ID font_id, String str)
#define DRAW_RECTANGLE_SIG(n) void n(Application_Links *app, f32_Rect rect, int_color color)
#define DRAW_RECTANGLE_OUTLINE_SIG(n) void n(Application_Links *app, f32_Rect rect, int_color color)
#define GET_DEFAULT_FONT_FOR_VIEW_SIG(n) Face_ID n(Application_Links *app, View_ID view_id)
typedef GLOBAL_SET_SETTING_SIG(Global_Set_Setting_Function);
typedef GLOBAL_SET_MAPPING_SIG(Global_Set_Mapping_Function);
typedef EXEC_SYSTEM_COMMAND_SIG(Exec_System_Command_Function);
typedef CLIPBOARD_POST_SIG(Clipboard_Post_Function);
typedef CLIPBOARD_COUNT_SIG(Clipboard_Count_Function);
typedef CLIPBOARD_INDEX_SIG(Clipboard_Index_Function);
typedef CREATE_PARSE_CONTEXT_SIG(Create_Parse_Context_Function);
typedef GET_BUFFER_COUNT_SIG(Get_Buffer_Count_Function);
typedef GET_BUFFER_FIRST_SIG(Get_Buffer_First_Function);
typedef GET_BUFFER_NEXT_SIG(Get_Buffer_Next_Function);
typedef GET_BUFFER_SIG(Get_Buffer_Function);
typedef GET_BUFFER_BY_NAME_SIG(Get_Buffer_By_Name_Function);
typedef GET_BUFFER_BY_FILE_NAME_SIG(Get_Buffer_By_File_Name_Function);
typedef BUFFER_READ_RANGE_SIG(Buffer_Read_Range_Function);
typedef BUFFER_REPLACE_RANGE_SIG(Buffer_Replace_Range_Function);
typedef BUFFER_COMPUTE_CURSOR_SIG(Buffer_Compute_Cursor_Function);
typedef BUFFER_BATCH_EDIT_SIG(Buffer_Batch_Edit_Function);
typedef BUFFER_GET_SETTING_SIG(Buffer_Get_Setting_Function);
typedef BUFFER_SET_SETTING_SIG(Buffer_Set_Setting_Function);
typedef BUFFER_GET_MANAGED_SCOPE_SIG(Buffer_Get_Managed_Scope_Function);
typedef BUFFER_TOKEN_COUNT_SIG(Buffer_Token_Count_Function);
typedef BUFFER_READ_TOKENS_SIG(Buffer_Read_Tokens_Function);
typedef BUFFER_GET_TOKEN_RANGE_SIG(Buffer_Get_Token_Range_Function);
typedef BUFFER_GET_TOKEN_INDEX_SIG(Buffer_Get_Token_Index_Function);
typedef BUFFER_SEND_END_SIGNAL_SIG(Buffer_Send_End_Signal_Function);
typedef CREATE_BUFFER_SIG(Create_Buffer_Function);
typedef SAVE_BUFFER_SIG(Save_Buffer_Function);
typedef KILL_BUFFER_SIG(Kill_Buffer_Function);
typedef REOPEN_BUFFER_SIG(Reopen_Buffer_Function);
typedef GET_VIEW_FIRST_SIG(Get_View_First_Function);
typedef GET_VIEW_NEXT_SIG(Get_View_Next_Function);
typedef GET_VIEW_SIG(Get_View_Function);
typedef GET_ACTIVE_VIEW_SIG(Get_Active_View_Function);
typedef OPEN_VIEW_SIG(Open_View_Function);
typedef CLOSE_VIEW_SIG(Close_View_Function);
typedef SET_ACTIVE_VIEW_SIG(Set_Active_View_Function);
typedef VIEW_GET_SETTING_SIG(View_Get_Setting_Function);
typedef VIEW_SET_SETTING_SIG(View_Set_Setting_Function);
typedef VIEW_GET_MANAGED_SCOPE_SIG(View_Get_Managed_Scope_Function);
typedef VIEW_SET_SPLIT_SIG(View_Set_Split_Function);
typedef VIEW_GET_ENCLOSURE_RECT_SIG(View_Get_Enclosure_Rect_Function);
typedef VIEW_COMPUTE_CURSOR_SIG(View_Compute_Cursor_Function);
typedef VIEW_SET_CURSOR_SIG(View_Set_Cursor_Function);
typedef VIEW_SET_SCROLL_SIG(View_Set_Scroll_Function);
typedef VIEW_SET_MARK_SIG(View_Set_Mark_Function);
typedef VIEW_SET_HIGHLIGHT_SIG(View_Set_Highlight_Function);
typedef VIEW_SET_BUFFER_SIG(View_Set_Buffer_Function);
typedef VIEW_POST_FADE_SIG(View_Post_Fade_Function);
typedef VIEW_BEGIN_UI_MODE_SIG(View_Begin_UI_Mode_Function);
typedef VIEW_END_UI_MODE_SIG(View_End_UI_Mode_Function);
typedef VIEW_SET_UI_SIG(View_Set_UI_Function);
typedef VIEW_GET_UI_COPY_SIG(View_Get_UI_Copy_Function);
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
typedef GET_THEME_COUNT_SIG(Get_Theme_Count_Function);
typedef GET_THEME_NAME_SIG(Get_Theme_Name_Function);
typedef CREATE_THEME_SIG(Create_Theme_Function);
typedef CHANGE_THEME_SIG(Change_Theme_Function);
typedef CHANGE_THEME_BY_INDEX_SIG(Change_Theme_By_Index_Function);
typedef GET_LARGEST_FACE_ID_SIG(Get_Largest_Face_ID_Function);
typedef SET_GLOBAL_FACE_SIG(Set_Global_Face_Function);
typedef BUFFER_SET_FACE_SIG(Buffer_Set_Face_Function);
typedef BUFFER_HISTORY_NEWEST_RECORD_INDEX_SIG(Buffer_History_Newest_Record_Index_Function);
typedef BUFFER_HISTORY_GET_RECORD_INFO_SIG(Buffer_History_Get_Record_Info_Function);
typedef BUFFER_HISTORY_GET_GROUP_SUB_RECORD_SIG(Buffer_History_Get_Group_Sub_Record_Function);
typedef BUFFER_HISTORY_GET_CURRENT_STATE_INDEX_SIG(Buffer_History_Get_Current_State_Index_Function);
typedef BUFFER_HISTORY_SET_CURRENT_STATE_INDEX_SIG(Buffer_History_Set_Current_State_Index_Function);
typedef BUFFER_HISTORY_MERGE_RECORD_RANGE_SIG(Buffer_History_Merge_Record_Range_Function);
typedef BUFFER_HISTORY_CLEAR_AFTER_CURRENT_STATE_SIG(Buffer_History_Clear_After_Current_State_Function);
typedef GLOBAL_HISTORY_EDIT_GROUP_BEGIN_SIG(Global_History_Edit_Group_Begin_Function);
typedef GLOBAL_HISTORY_EDIT_GROUP_END_SIG(Global_History_Edit_Group_End_Function);
typedef GET_FACE_DESCRIPTION_SIG(Get_Face_Description_Function);
typedef GET_FACE_ID_SIG(Get_Face_ID_Function);
typedef TRY_CREATE_NEW_FACE_SIG(Try_Create_New_Face_Function);
typedef TRY_MODIFY_FACE_SIG(Try_Modify_Face_Function);
typedef TRY_RELEASE_FACE_SIG(Try_Release_Face_Function);
typedef GET_AVAILABLE_FONT_COUNT_SIG(Get_Available_Font_Count_Function);
typedef GET_AVAILABLE_FONT_SIG(Get_Available_Font_Function);
typedef SET_THEME_COLORS_SIG(Set_Theme_Colors_Function);
typedef GET_THEME_COLORS_SIG(Get_Theme_Colors_Function);
typedef DIRECTORY_GET_HOT_SIG(Directory_Get_Hot_Function);
typedef DIRECTORY_SET_HOT_SIG(Directory_Set_Hot_Function);
typedef GET_FILE_LIST_SIG(Get_File_List_Function);
typedef FREE_FILE_LIST_SIG(Free_File_List_Function);
typedef SET_GUI_UP_DOWN_KEYS_SIG(Set_GUI_Up_Down_Keys_Function);
typedef MEMORY_ALLOCATE_SIG(Memory_Allocate_Function);
typedef MEMORY_SET_PROTECTION_SIG(Memory_Set_Protection_Function);
typedef MEMORY_FREE_SIG(Memory_Free_Function);
typedef FILE_EXISTS_SIG(File_Exists_Function);
typedef DIRECTORY_CD_SIG(Directory_CD_Function);
typedef GET_4ED_PATH_SIG(Get_4ed_Path_Function);
typedef SHOW_MOUSE_CURSOR_SIG(Show_Mouse_Cursor_Function);
typedef SET_FULLSCREEN_SIG(Set_Fullscreen_Function);
typedef IS_FULLSCREEN_SIG(Is_Fullscreen_Function);
typedef SEND_EXIT_SIGNAL_SIG(Send_Exit_Signal_Function);
typedef SET_WINDOW_TITLE_SIG(Set_Window_Title_Function);
typedef GET_MICROSECONDS_TIMESTAMP_SIG(Get_Microseconds_Timestamp_Function);
typedef DRAW_STRING_SIG(Draw_String_Function);
typedef GET_STRING_ADVANCE_SIG(Get_String_Advance_Function);
typedef DRAW_RECTANGLE_SIG(Draw_Rectangle_Function);
typedef DRAW_RECTANGLE_OUTLINE_SIG(Draw_Rectangle_Outline_Function);
typedef GET_DEFAULT_FONT_FOR_VIEW_SIG(Get_Default_Font_For_View_Function);
struct Application_Links{
#if defined(ALLOW_DEP_4CODER)
Global_Set_Setting_Function *global_set_setting;
Global_Set_Mapping_Function *global_set_mapping;
Exec_System_Command_Function *exec_system_command;
Clipboard_Post_Function *clipboard_post;
Clipboard_Count_Function *clipboard_count;
Clipboard_Index_Function *clipboard_index;
Create_Parse_Context_Function *create_parse_context;
Get_Buffer_Count_Function *get_buffer_count;
Get_Buffer_First_Function *get_buffer_first;
Get_Buffer_Next_Function *get_buffer_next;
Get_Buffer_Function *get_buffer;
Get_Buffer_By_Name_Function *get_buffer_by_name;
Get_Buffer_By_File_Name_Function *get_buffer_by_file_name;
Buffer_Read_Range_Function *buffer_read_range;
Buffer_Replace_Range_Function *buffer_replace_range;
Buffer_Compute_Cursor_Function *buffer_compute_cursor;
Buffer_Batch_Edit_Function *buffer_batch_edit;
Buffer_Get_Setting_Function *buffer_get_setting;
Buffer_Set_Setting_Function *buffer_set_setting;
Buffer_Get_Managed_Scope_Function *buffer_get_managed_scope;
Buffer_Token_Count_Function *buffer_token_count;
Buffer_Read_Tokens_Function *buffer_read_tokens;
Buffer_Get_Token_Range_Function *buffer_get_token_range;
Buffer_Get_Token_Index_Function *buffer_get_token_index;
Buffer_Send_End_Signal_Function *buffer_send_end_signal;
Create_Buffer_Function *create_buffer;
Save_Buffer_Function *save_buffer;
Kill_Buffer_Function *kill_buffer;
Reopen_Buffer_Function *reopen_buffer;
Get_View_First_Function *get_view_first;
Get_View_Next_Function *get_view_next;
Get_View_Function *get_view;
Get_Active_View_Function *get_active_view;
Open_View_Function *open_view;
Close_View_Function *close_view;
Set_Active_View_Function *set_active_view;
View_Get_Setting_Function *view_get_setting;
View_Set_Setting_Function *view_set_setting;
View_Get_Managed_Scope_Function *view_get_managed_scope;
View_Set_Split_Function *view_set_split;
View_Get_Enclosure_Rect_Function *view_get_enclosure_rect;
View_Compute_Cursor_Function *view_compute_cursor;
View_Set_Cursor_Function *view_set_cursor;
View_Set_Scroll_Function *view_set_scroll;
View_Set_Mark_Function *view_set_mark;
View_Set_Highlight_Function *view_set_highlight;
View_Set_Buffer_Function *view_set_buffer;
View_Post_Fade_Function *view_post_fade;
View_Begin_UI_Mode_Function *view_begin_ui_mode;
View_End_UI_Mode_Function *view_end_ui_mode;
View_Set_UI_Function *view_set_ui;
View_Get_UI_Copy_Function *view_get_ui_copy;
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
Get_Theme_Count_Function *get_theme_count;
Get_Theme_Name_Function *get_theme_name;
Create_Theme_Function *create_theme;
Change_Theme_Function *change_theme;
Change_Theme_By_Index_Function *change_theme_by_index;
Get_Largest_Face_ID_Function *get_largest_face_id;
Set_Global_Face_Function *set_global_face;
Buffer_Set_Face_Function *buffer_set_face;
Buffer_History_Newest_Record_Index_Function *buffer_history_newest_record_index;
Buffer_History_Get_Record_Info_Function *buffer_history_get_record_info;
Buffer_History_Get_Group_Sub_Record_Function *buffer_history_get_group_sub_record;
Buffer_History_Get_Current_State_Index_Function *buffer_history_get_current_state_index;
Buffer_History_Set_Current_State_Index_Function *buffer_history_set_current_state_index;
Buffer_History_Merge_Record_Range_Function *buffer_history_merge_record_range;
Buffer_History_Clear_After_Current_State_Function *buffer_history_clear_after_current_state;
Global_History_Edit_Group_Begin_Function *global_history_edit_group_begin;
Global_History_Edit_Group_End_Function *global_history_edit_group_end;
Get_Face_Description_Function *get_face_description;
Get_Face_ID_Function *get_face_id;
Try_Create_New_Face_Function *try_create_new_face;
Try_Modify_Face_Function *try_modify_face;
Try_Release_Face_Function *try_release_face;
Get_Available_Font_Count_Function *get_available_font_count;
Get_Available_Font_Function *get_available_font;
Set_Theme_Colors_Function *set_theme_colors;
Get_Theme_Colors_Function *get_theme_colors;
Directory_Get_Hot_Function *directory_get_hot;
Directory_Set_Hot_Function *directory_set_hot;
Get_File_List_Function *get_file_list;
Free_File_List_Function *free_file_list;
Set_GUI_Up_Down_Keys_Function *set_gui_up_down_keys;
Memory_Allocate_Function *memory_allocate;
Memory_Set_Protection_Function *memory_set_protection;
Memory_Free_Function *memory_free;
File_Exists_Function *file_exists;
Directory_CD_Function *directory_cd;
Get_4ed_Path_Function *get_4ed_path;
Show_Mouse_Cursor_Function *show_mouse_cursor;
Set_Fullscreen_Function *set_fullscreen;
Is_Fullscreen_Function *is_fullscreen;
Send_Exit_Signal_Function *send_exit_signal;
Set_Window_Title_Function *set_window_title;
Get_Microseconds_Timestamp_Function *get_microseconds_timestamp;
Draw_String_Function *draw_string;
Get_String_Advance_Function *get_string_advance;
Draw_Rectangle_Function *draw_rectangle;
Draw_Rectangle_Outline_Function *draw_rectangle_outline;
Get_Default_Font_For_View_Function *get_default_font_for_view;
#else
Global_Set_Setting_Function *global_set_setting_;
Global_Set_Mapping_Function *global_set_mapping_;
Exec_System_Command_Function *exec_system_command_;
Clipboard_Post_Function *clipboard_post_;
Clipboard_Count_Function *clipboard_count_;
Clipboard_Index_Function *clipboard_index_;
Create_Parse_Context_Function *create_parse_context_;
Get_Buffer_Count_Function *get_buffer_count_;
Get_Buffer_First_Function *get_buffer_first_;
Get_Buffer_Next_Function *get_buffer_next_;
Get_Buffer_Function *get_buffer_;
Get_Buffer_By_Name_Function *get_buffer_by_name_;
Get_Buffer_By_File_Name_Function *get_buffer_by_file_name_;
Buffer_Read_Range_Function *buffer_read_range_;
Buffer_Replace_Range_Function *buffer_replace_range_;
Buffer_Compute_Cursor_Function *buffer_compute_cursor_;
Buffer_Batch_Edit_Function *buffer_batch_edit_;
Buffer_Get_Setting_Function *buffer_get_setting_;
Buffer_Set_Setting_Function *buffer_set_setting_;
Buffer_Get_Managed_Scope_Function *buffer_get_managed_scope_;
Buffer_Token_Count_Function *buffer_token_count_;
Buffer_Read_Tokens_Function *buffer_read_tokens_;
Buffer_Get_Token_Range_Function *buffer_get_token_range_;
Buffer_Get_Token_Index_Function *buffer_get_token_index_;
Buffer_Send_End_Signal_Function *buffer_send_end_signal_;
Create_Buffer_Function *create_buffer_;
Save_Buffer_Function *save_buffer_;
Kill_Buffer_Function *kill_buffer_;
Reopen_Buffer_Function *reopen_buffer_;
Get_View_First_Function *get_view_first_;
Get_View_Next_Function *get_view_next_;
Get_View_Function *get_view_;
Get_Active_View_Function *get_active_view_;
Open_View_Function *open_view_;
Close_View_Function *close_view_;
Set_Active_View_Function *set_active_view_;
View_Get_Setting_Function *view_get_setting_;
View_Set_Setting_Function *view_set_setting_;
View_Get_Managed_Scope_Function *view_get_managed_scope_;
View_Set_Split_Function *view_set_split_;
View_Get_Enclosure_Rect_Function *view_get_enclosure_rect_;
View_Compute_Cursor_Function *view_compute_cursor_;
View_Set_Cursor_Function *view_set_cursor_;
View_Set_Scroll_Function *view_set_scroll_;
View_Set_Mark_Function *view_set_mark_;
View_Set_Highlight_Function *view_set_highlight_;
View_Set_Buffer_Function *view_set_buffer_;
View_Post_Fade_Function *view_post_fade_;
View_Begin_UI_Mode_Function *view_begin_ui_mode_;
View_End_UI_Mode_Function *view_end_ui_mode_;
View_Set_UI_Function *view_set_ui_;
View_Get_UI_Copy_Function *view_get_ui_copy_;
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
Get_Theme_Count_Function *get_theme_count_;
Get_Theme_Name_Function *get_theme_name_;
Create_Theme_Function *create_theme_;
Change_Theme_Function *change_theme_;
Change_Theme_By_Index_Function *change_theme_by_index_;
Get_Largest_Face_ID_Function *get_largest_face_id_;
Set_Global_Face_Function *set_global_face_;
Buffer_Set_Face_Function *buffer_set_face_;
Buffer_History_Newest_Record_Index_Function *buffer_history_newest_record_index_;
Buffer_History_Get_Record_Info_Function *buffer_history_get_record_info_;
Buffer_History_Get_Group_Sub_Record_Function *buffer_history_get_group_sub_record_;
Buffer_History_Get_Current_State_Index_Function *buffer_history_get_current_state_index_;
Buffer_History_Set_Current_State_Index_Function *buffer_history_set_current_state_index_;
Buffer_History_Merge_Record_Range_Function *buffer_history_merge_record_range_;
Buffer_History_Clear_After_Current_State_Function *buffer_history_clear_after_current_state_;
Global_History_Edit_Group_Begin_Function *global_history_edit_group_begin_;
Global_History_Edit_Group_End_Function *global_history_edit_group_end_;
Get_Face_Description_Function *get_face_description_;
Get_Face_ID_Function *get_face_id_;
Try_Create_New_Face_Function *try_create_new_face_;
Try_Modify_Face_Function *try_modify_face_;
Try_Release_Face_Function *try_release_face_;
Get_Available_Font_Count_Function *get_available_font_count_;
Get_Available_Font_Function *get_available_font_;
Set_Theme_Colors_Function *set_theme_colors_;
Get_Theme_Colors_Function *get_theme_colors_;
Directory_Get_Hot_Function *directory_get_hot_;
Directory_Set_Hot_Function *directory_set_hot_;
Get_File_List_Function *get_file_list_;
Free_File_List_Function *free_file_list_;
Set_GUI_Up_Down_Keys_Function *set_gui_up_down_keys_;
Memory_Allocate_Function *memory_allocate_;
Memory_Set_Protection_Function *memory_set_protection_;
Memory_Free_Function *memory_free_;
File_Exists_Function *file_exists_;
Directory_CD_Function *directory_cd_;
Get_4ed_Path_Function *get_4ed_path_;
Show_Mouse_Cursor_Function *show_mouse_cursor_;
Set_Fullscreen_Function *set_fullscreen_;
Is_Fullscreen_Function *is_fullscreen_;
Send_Exit_Signal_Function *send_exit_signal_;
Set_Window_Title_Function *set_window_title_;
Get_Microseconds_Timestamp_Function *get_microseconds_timestamp_;
Draw_String_Function *draw_string_;
Get_String_Advance_Function *get_string_advance_;
Draw_Rectangle_Function *draw_rectangle_;
Draw_Rectangle_Outline_Function *draw_rectangle_outline_;
Get_Default_Font_For_View_Function *get_default_font_for_view_;
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
app_links->exec_system_command_ = Exec_System_Command;\
app_links->clipboard_post_ = Clipboard_Post;\
app_links->clipboard_count_ = Clipboard_Count;\
app_links->clipboard_index_ = Clipboard_Index;\
app_links->create_parse_context_ = Create_Parse_Context;\
app_links->get_buffer_count_ = Get_Buffer_Count;\
app_links->get_buffer_first_ = Get_Buffer_First;\
app_links->get_buffer_next_ = Get_Buffer_Next;\
app_links->get_buffer_ = Get_Buffer;\
app_links->get_buffer_by_name_ = Get_Buffer_By_Name;\
app_links->get_buffer_by_file_name_ = Get_Buffer_By_File_Name;\
app_links->buffer_read_range_ = Buffer_Read_Range;\
app_links->buffer_replace_range_ = Buffer_Replace_Range;\
app_links->buffer_compute_cursor_ = Buffer_Compute_Cursor;\
app_links->buffer_batch_edit_ = Buffer_Batch_Edit;\
app_links->buffer_get_setting_ = Buffer_Get_Setting;\
app_links->buffer_set_setting_ = Buffer_Set_Setting;\
app_links->buffer_get_managed_scope_ = Buffer_Get_Managed_Scope;\
app_links->buffer_token_count_ = Buffer_Token_Count;\
app_links->buffer_read_tokens_ = Buffer_Read_Tokens;\
app_links->buffer_get_token_range_ = Buffer_Get_Token_Range;\
app_links->buffer_get_token_index_ = Buffer_Get_Token_Index;\
app_links->buffer_send_end_signal_ = Buffer_Send_End_Signal;\
app_links->create_buffer_ = Create_Buffer;\
app_links->save_buffer_ = Save_Buffer;\
app_links->kill_buffer_ = Kill_Buffer;\
app_links->reopen_buffer_ = Reopen_Buffer;\
app_links->get_view_first_ = Get_View_First;\
app_links->get_view_next_ = Get_View_Next;\
app_links->get_view_ = Get_View;\
app_links->get_active_view_ = Get_Active_View;\
app_links->open_view_ = Open_View;\
app_links->close_view_ = Close_View;\
app_links->set_active_view_ = Set_Active_View;\
app_links->view_get_setting_ = View_Get_Setting;\
app_links->view_set_setting_ = View_Set_Setting;\
app_links->view_get_managed_scope_ = View_Get_Managed_Scope;\
app_links->view_set_split_ = View_Set_Split;\
app_links->view_get_enclosure_rect_ = View_Get_Enclosure_Rect;\
app_links->view_compute_cursor_ = View_Compute_Cursor;\
app_links->view_set_cursor_ = View_Set_Cursor;\
app_links->view_set_scroll_ = View_Set_Scroll;\
app_links->view_set_mark_ = View_Set_Mark;\
app_links->view_set_highlight_ = View_Set_Highlight;\
app_links->view_set_buffer_ = View_Set_Buffer;\
app_links->view_post_fade_ = View_Post_Fade;\
app_links->view_begin_ui_mode_ = View_Begin_UI_Mode;\
app_links->view_end_ui_mode_ = View_End_UI_Mode;\
app_links->view_set_ui_ = View_Set_UI;\
app_links->view_get_ui_copy_ = View_Get_UI_Copy;\
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
app_links->get_theme_count_ = Get_Theme_Count;\
app_links->get_theme_name_ = Get_Theme_Name;\
app_links->create_theme_ = Create_Theme;\
app_links->change_theme_ = Change_Theme;\
app_links->change_theme_by_index_ = Change_Theme_By_Index;\
app_links->get_largest_face_id_ = Get_Largest_Face_ID;\
app_links->set_global_face_ = Set_Global_Face;\
app_links->buffer_set_face_ = Buffer_Set_Face;\
app_links->buffer_history_newest_record_index_ = Buffer_History_Newest_Record_Index;\
app_links->buffer_history_get_record_info_ = Buffer_History_Get_Record_Info;\
app_links->buffer_history_get_group_sub_record_ = Buffer_History_Get_Group_Sub_Record;\
app_links->buffer_history_get_current_state_index_ = Buffer_History_Get_Current_State_Index;\
app_links->buffer_history_set_current_state_index_ = Buffer_History_Set_Current_State_Index;\
app_links->buffer_history_merge_record_range_ = Buffer_History_Merge_Record_Range;\
app_links->buffer_history_clear_after_current_state_ = Buffer_History_Clear_After_Current_State;\
app_links->global_history_edit_group_begin_ = Global_History_Edit_Group_Begin;\
app_links->global_history_edit_group_end_ = Global_History_Edit_Group_End;\
app_links->get_face_description_ = Get_Face_Description;\
app_links->get_face_id_ = Get_Face_ID;\
app_links->try_create_new_face_ = Try_Create_New_Face;\
app_links->try_modify_face_ = Try_Modify_Face;\
app_links->try_release_face_ = Try_Release_Face;\
app_links->get_available_font_count_ = Get_Available_Font_Count;\
app_links->get_available_font_ = Get_Available_Font;\
app_links->set_theme_colors_ = Set_Theme_Colors;\
app_links->get_theme_colors_ = Get_Theme_Colors;\
app_links->directory_get_hot_ = Directory_Get_Hot;\
app_links->directory_set_hot_ = Directory_Set_Hot;\
app_links->get_file_list_ = Get_File_List;\
app_links->free_file_list_ = Free_File_List;\
app_links->set_gui_up_down_keys_ = Set_GUI_Up_Down_Keys;\
app_links->memory_allocate_ = Memory_Allocate;\
app_links->memory_set_protection_ = Memory_Set_Protection;\
app_links->memory_free_ = Memory_Free;\
app_links->file_exists_ = File_Exists;\
app_links->directory_cd_ = Directory_CD;\
app_links->get_4ed_path_ = Get_4ed_Path;\
app_links->show_mouse_cursor_ = Show_Mouse_Cursor;\
app_links->set_fullscreen_ = Set_Fullscreen;\
app_links->is_fullscreen_ = Is_Fullscreen;\
app_links->send_exit_signal_ = Send_Exit_Signal;\
app_links->set_window_title_ = Set_Window_Title;\
app_links->get_microseconds_timestamp_ = Get_Microseconds_Timestamp;\
app_links->draw_string_ = Draw_String;\
app_links->get_string_advance_ = Get_String_Advance;\
app_links->draw_rectangle_ = Draw_Rectangle;\
app_links->draw_rectangle_outline_ = Draw_Rectangle_Outline;\
app_links->get_default_font_for_view_ = Get_Default_Font_For_View;} while(false)
#if defined(ALLOW_DEP_4CODER)
static bool32 global_set_setting(Application_Links *app, Global_Setting_ID setting, int32_t value){return(app->global_set_setting(app, setting, value));}
static bool32 global_set_mapping(Application_Links *app, void *data, int32_t size){return(app->global_set_mapping(app, data, size));}
static bool32 exec_system_command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer_id, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags){return(app->exec_system_command(app, view, buffer_id, path, path_len, command, command_len, flags));}
static void clipboard_post(Application_Links *app, int32_t clipboard_id, char *str, int32_t len){(app->clipboard_post(app, clipboard_id, str, len));}
static int32_t clipboard_count(Application_Links *app, int32_t clipboard_id){return(app->clipboard_count(app, clipboard_id));}
static int32_t clipboard_index(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len){return(app->clipboard_index(app, clipboard_id, item_index, out, len));}
static Parse_Context_ID create_parse_context(Application_Links *app, Parser_String_And_Type *kw, uint32_t kw_count, Parser_String_And_Type *pp, uint32_t pp_count){return(app->create_parse_context(app, kw, kw_count, pp, pp_count));}
static int32_t get_buffer_count(Application_Links *app){return(app->get_buffer_count(app));}
static Buffer_Summary get_buffer_first(Application_Links *app, Access_Flag access){return(app->get_buffer_first(app, access));}
static void get_buffer_next(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access){(app->get_buffer_next(app, buffer, access));}
static Buffer_Summary get_buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access){return(app->get_buffer(app, buffer_id, access));}
static Buffer_Summary get_buffer_by_name(Application_Links *app, char *name, int32_t len, Access_Flag access){return(app->get_buffer_by_name(app, name, len, access));}
static Buffer_Summary get_buffer_by_file_name(Application_Links *app, char *name, int32_t len, Access_Flag access){return(app->get_buffer_by_file_name(app, name, len, access));}
static bool32 buffer_read_range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *out){return(app->buffer_read_range(app, buffer, start, end, out));}
static bool32 buffer_replace_range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *str, int32_t len){return(app->buffer_replace_range(app, buffer, start, end, str, len));}
static bool32 buffer_compute_cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out){return(app->buffer_compute_cursor(app, buffer, seek, cursor_out));}
static bool32 buffer_batch_edit(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type){return(app->buffer_batch_edit(app, buffer, str, str_len, edits, edit_count, type));}
static bool32 buffer_get_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out){return(app->buffer_get_setting(app, buffer, setting, value_out));}
static bool32 buffer_set_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value){return(app->buffer_set_setting(app, buffer, setting, value));}
static Managed_Scope buffer_get_managed_scope(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_get_managed_scope(app, buffer_id));}
static int32_t buffer_token_count(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_token_count(app, buffer));}
static bool32 buffer_read_tokens(Application_Links *app, Buffer_Summary *buffer, int32_t start_token, int32_t end_token, Cpp_Token *tokens_out){return(app->buffer_read_tokens(app, buffer, start_token, end_token, tokens_out));}
static bool32 buffer_get_token_range(Application_Links *app, Buffer_Summary *buffer, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out){return(app->buffer_get_token_range(app, buffer, first_token_out, one_past_last_token_out));}
static bool32 buffer_get_token_index(Application_Links *app, Buffer_Summary *buffer, int32_t pos, Cpp_Get_Token_Result *get_result){return(app->buffer_get_token_index(app, buffer, pos, get_result));}
static bool32 buffer_send_end_signal(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_send_end_signal(app, buffer));}
static Buffer_Summary create_buffer(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags){return(app->create_buffer(app, filename, filename_len, flags));}
static bool32 save_buffer(Application_Links *app, Buffer_Summary *buffer, char *file_name, int32_t file_name_len, uint32_t flags){return(app->save_buffer(app, buffer, file_name, file_name_len, flags));}
static Buffer_Kill_Result kill_buffer(Application_Links *app, Buffer_Identifier buffer, Buffer_Kill_Flag flags){return(app->kill_buffer(app, buffer, flags));}
static Buffer_Reopen_Result reopen_buffer(Application_Links *app, Buffer_Summary *buffer, Buffer_Reopen_Flag flags){return(app->reopen_buffer(app, buffer, flags));}
static View_Summary get_view_first(Application_Links *app, Access_Flag access){return(app->get_view_first(app, access));}
static void get_view_next(Application_Links *app, View_Summary *view, Access_Flag access){(app->get_view_next(app, view, access));}
static View_Summary get_view(Application_Links *app, View_ID view_id, Access_Flag access){return(app->get_view(app, view_id, access));}
static View_Summary get_active_view(Application_Links *app, Access_Flag access){return(app->get_active_view(app, access));}
static View_Summary open_view(Application_Links *app, View_Summary *view_location, View_Split_Position position){return(app->open_view(app, view_location, position));}
static bool32 close_view(Application_Links *app, View_Summary *view){return(app->close_view(app, view));}
static bool32 set_active_view(Application_Links *app, View_Summary *view){return(app->set_active_view(app, view));}
static bool32 view_get_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out){return(app->view_get_setting(app, view, setting, value_out));}
static bool32 view_set_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value){return(app->view_set_setting(app, view, setting, value));}
static Managed_Scope view_get_managed_scope(Application_Links *app, View_ID view_id){return(app->view_get_managed_scope(app, view_id));}
static bool32 view_set_split(Application_Links *app, View_Summary *view, View_Split_Kind kind, float t){return(app->view_set_split(app, view, kind, t));}
static i32_Rect view_get_enclosure_rect(Application_Links *app, View_Summary *view){return(app->view_get_enclosure_rect(app, view));}
static bool32 view_compute_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out){return(app->view_compute_cursor(app, view, seek, cursor_out));}
static bool32 view_set_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x){return(app->view_set_cursor(app, view, seek, set_preferred_x));}
static bool32 view_set_scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll){return(app->view_set_scroll(app, view, scroll));}
static bool32 view_set_mark(Application_Links *app, View_Summary *view, Buffer_Seek seek){return(app->view_set_mark(app, view, seek));}
static bool32 view_set_highlight(Application_Links *app, View_Summary *view, int32_t start, int32_t end, bool32 turn_on){return(app->view_set_highlight(app, view, start, end, turn_on));}
static bool32 view_set_buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags){return(app->view_set_buffer(app, view, buffer_id, flags));}
static bool32 view_post_fade(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color){return(app->view_post_fade(app, view, seconds, start, end, color));}
static bool32 view_begin_ui_mode(Application_Links *app, View_Summary *view){return(app->view_begin_ui_mode(app, view));}
static bool32 view_end_ui_mode(Application_Links *app, View_Summary *view){return(app->view_end_ui_mode(app, view));}
static bool32 view_set_ui(Application_Links *app, View_Summary *view, UI_Control *control, UI_Quit_Function_Type *quit_function){return(app->view_set_ui(app, view, control, quit_function));}
static UI_Control view_get_ui_copy(Application_Links *app, View_Summary *view, struct Partition *part){return(app->view_get_ui_copy(app, view, part));}
static Managed_Scope create_user_managed_scope(Application_Links *app){return(app->create_user_managed_scope(app));}
static bool32 destroy_user_managed_scope(Application_Links *app, Managed_Scope scope){return(app->destroy_user_managed_scope(app, scope));}
static Managed_Scope get_global_managed_scope(Application_Links *app){return(app->get_global_managed_scope(app));}
static Managed_Scope get_managed_scope_with_multiple_dependencies(Application_Links *app, Managed_Scope *scopes, int32_t count){return(app->get_managed_scope_with_multiple_dependencies(app, scopes, count));}
static bool32 managed_scope_clear_contents(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_contents(app, scope));}
static bool32 managed_scope_clear_self_all_dependent_scopes(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_self_all_dependent_scopes(app, scope));}
static Managed_Variable_ID managed_variable_create(Application_Links *app, char *null_terminated_name, uint64_t default_value){return(app->managed_variable_create(app, null_terminated_name, default_value));}
static Managed_Variable_ID managed_variable_get_id(Application_Links *app, char *null_terminated_name){return(app->managed_variable_get_id(app, null_terminated_name));}
static Managed_Variable_ID managed_variable_create_or_get_id(Application_Links *app, char *null_terminated_name, uint64_t default_value){return(app->managed_variable_create_or_get_id(app, null_terminated_name, default_value));}
static bool32 managed_variable_set(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t value){return(app->managed_variable_set(app, scope, id, value));}
static bool32 managed_variable_get(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t *value_out){return(app->managed_variable_get(app, scope, id, value_out));}
static Managed_Object alloc_managed_memory_in_scope(Application_Links *app, Managed_Scope scope, int32_t item_size, int32_t count){return(app->alloc_managed_memory_in_scope(app, scope, item_size, count));}
static Managed_Object alloc_buffer_markers_on_buffer(Application_Links *app, Buffer_ID buffer_id, int32_t count, Managed_Scope *optional_extra_scope){return(app->alloc_buffer_markers_on_buffer(app, buffer_id, count, optional_extra_scope));}
static Marker_Visual create_marker_visual(Application_Links *app, Managed_Object object){return(app->create_marker_visual(app, object));}
static bool32 marker_visual_set_effect(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style){return(app->marker_visual_set_effect(app, visual, type, color, text_color, text_style));}
static bool32 marker_visual_set_take_rule(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule){return(app->marker_visual_set_take_rule(app, visual, take_rule));}
static bool32 marker_visual_set_priority(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority){return(app->marker_visual_set_priority(app, visual, priority));}
static bool32 marker_visual_set_view_key(Application_Links *app, Marker_Visual visual, View_ID key_view_id){return(app->marker_visual_set_view_key(app, visual, key_view_id));}
static bool32 destroy_marker_visual(Application_Links *app, Marker_Visual visual){return(app->destroy_marker_visual(app, visual));}
static int32_t buffer_markers_get_attached_visual_count(Application_Links *app, Managed_Object object){return(app->buffer_markers_get_attached_visual_count(app, object));}
static Marker_Visual* buffer_markers_get_attached_visual(Application_Links *app, Partition *part, Managed_Object object){return(app->buffer_markers_get_attached_visual(app, part, object));}
static uint32_t managed_object_get_item_size(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_size(app, object));}
static uint32_t managed_object_get_item_count(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_count(app, object));}
static Managed_Object_Type managed_object_get_type(Application_Links *app, Managed_Object object){return(app->managed_object_get_type(app, object));}
static Managed_Scope managed_object_get_containing_scope(Application_Links *app, Managed_Object object){return(app->managed_object_get_containing_scope(app, object));}
static bool32 managed_object_free(Application_Links *app, Managed_Object object){return(app->managed_object_free(app, object));}
static bool32 managed_object_store_data(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem){return(app->managed_object_store_data(app, object, first_index, count, mem));}
static bool32 managed_object_load_data(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem_out){return(app->managed_object_load_data(app, object, first_index, count, mem_out));}
static User_Input get_user_input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type){return(app->get_user_input(app, get_type, abort_type));}
static User_Input get_command_input(Application_Links *app){return(app->get_command_input(app));}
static void set_command_input(Application_Links *app, Key_Event_Data key_data){(app->set_command_input(app, key_data));}
static Mouse_State get_mouse_state(Application_Links *app){return(app->get_mouse_state(app));}
static int32_t get_active_query_bars(Application_Links *app, View_ID view_id, int32_t max_result_count, Query_Bar **result_array){return(app->get_active_query_bars(app, view_id, max_result_count, result_array));}
static bool32 start_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){return(app->start_query_bar(app, bar, flags));}
static void end_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){(app->end_query_bar(app, bar, flags));}
static void print_message(Application_Links *app, char *str, int32_t len){(app->print_message(app, str, len));}
static int32_t get_theme_count(Application_Links *app){return(app->get_theme_count(app));}
static String get_theme_name(Application_Links *app, struct Partition *arena, int32_t index){return(app->get_theme_name(app, arena, index));}
static void create_theme(Application_Links *app, Theme *theme, char *name, int32_t len){(app->create_theme(app, theme, name, len));}
static void change_theme(Application_Links *app, char *name, int32_t len){(app->change_theme(app, name, len));}
static bool32 change_theme_by_index(Application_Links *app, int32_t index){return(app->change_theme_by_index(app, index));}
static Face_ID get_largest_face_id(Application_Links *app){return(app->get_largest_face_id(app));}
static bool32 set_global_face(Application_Links *app, Face_ID id, bool32 apply_to_all_buffers){return(app->set_global_face(app, id, apply_to_all_buffers));}
static bool32 buffer_set_face(Application_Links *app, Buffer_Summary *buffer, Face_ID id){return(app->buffer_set_face(app, buffer, id));}
static History_Record_Index buffer_history_newest_record_index(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_history_newest_record_index(app, buffer));}
static Record_Info buffer_history_get_record_info(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index){return(app->buffer_history_get_record_info(app, buffer, index));}
static Record_Info buffer_history_get_group_sub_record(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index, int32_t sub_index){return(app->buffer_history_get_group_sub_record(app, buffer, index, sub_index));}
static History_Record_Index buffer_history_get_current_state_index(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_history_get_current_state_index(app, buffer));}
static bool32 buffer_history_set_current_state_index(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index){return(app->buffer_history_set_current_state_index(app, buffer, index));}
static bool32 buffer_history_merge_record_range(Application_Links *app, Buffer_Summary *buffer, History_Record_Index first_index, History_Record_Index last_index){return(app->buffer_history_merge_record_range(app, buffer, first_index, last_index));}
static bool32 buffer_history_clear_after_current_state(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_history_clear_after_current_state(app, buffer));}
static void global_history_edit_group_begin(Application_Links *app){(app->global_history_edit_group_begin(app));}
static void global_history_edit_group_end(Application_Links *app){(app->global_history_edit_group_end(app));}
static Face_Description get_face_description(Application_Links *app, Face_ID id){return(app->get_face_description(app, id));}
static Face_ID get_face_id(Application_Links *app, Buffer_Summary *buffer){return(app->get_face_id(app, buffer));}
static Face_ID try_create_new_face(Application_Links *app, Face_Description *description){return(app->try_create_new_face(app, description));}
static bool32 try_modify_face(Application_Links *app, Face_ID id, Face_Description *description){return(app->try_modify_face(app, id, description));}
static bool32 try_release_face(Application_Links *app, Face_ID id, Face_ID replacement_id){return(app->try_release_face(app, id, replacement_id));}
static int32_t get_available_font_count(Application_Links *app){return(app->get_available_font_count(app));}
static Available_Font get_available_font(Application_Links *app, int32_t index){return(app->get_available_font(app, index));}
static void set_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->set_theme_colors(app, colors, count));}
static void get_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->get_theme_colors(app, colors, count));}
static int32_t directory_get_hot(Application_Links *app, char *out, int32_t capacity){return(app->directory_get_hot(app, out, capacity));}
static bool32 directory_set_hot(Application_Links *app, char *str, int32_t len){return(app->directory_set_hot(app, str, len));}
static File_List get_file_list(Application_Links *app, char *dir, int32_t len){return(app->get_file_list(app, dir, len));}
static void free_file_list(Application_Links *app, File_List list){(app->free_file_list(app, list));}
static void set_gui_up_down_keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier){(app->set_gui_up_down_keys(app, up_key, up_key_modifier, down_key, down_key_modifier));}
static void* memory_allocate(Application_Links *app, int32_t size){return(app->memory_allocate(app, size));}
static bool32 memory_set_protection(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags){return(app->memory_set_protection(app, ptr, size, flags));}
static void memory_free(Application_Links *app, void *ptr, int32_t size){(app->memory_free(app, ptr, size));}
static bool32 file_exists(Application_Links *app, char *filename, int32_t len){return(app->file_exists(app, filename, len));}
static bool32 directory_cd(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len){return(app->directory_cd(app, dir, len, capacity, rel_path, rel_len));}
static int32_t get_4ed_path(Application_Links *app, char *out, int32_t capacity){return(app->get_4ed_path(app, out, capacity));}
static void show_mouse_cursor(Application_Links *app, Mouse_Cursor_Show_Type show){(app->show_mouse_cursor(app, show));}
static bool32 set_fullscreen(Application_Links *app, bool32 full_screen){return(app->set_fullscreen(app, full_screen));}
static bool32 is_fullscreen(Application_Links *app){return(app->is_fullscreen(app));}
static void send_exit_signal(Application_Links *app){(app->send_exit_signal(app));}
static void set_window_title(Application_Links *app, char *title){(app->set_window_title(app, title));}
static Microsecond_Time_Stamp get_microseconds_timestamp(Application_Links *app){return(app->get_microseconds_timestamp(app));}
static float draw_string(Application_Links *app, Face_ID font_id, String str, int32_t x, int32_t y, int_color color, uint32_t flags, float dx, float dy){return(app->draw_string(app, font_id, str, x, y, color, flags, dx, dy));}
static float get_string_advance(Application_Links *app, Face_ID font_id, String str){return(app->get_string_advance(app, font_id, str));}
static void draw_rectangle(Application_Links *app, f32_Rect rect, int_color color){(app->draw_rectangle(app, rect, color));}
static void draw_rectangle_outline(Application_Links *app, f32_Rect rect, int_color color){(app->draw_rectangle_outline(app, rect, color));}
static Face_ID get_default_font_for_view(Application_Links *app, View_ID view_id){return(app->get_default_font_for_view(app, view_id));}
#else
static bool32 global_set_setting(Application_Links *app, Global_Setting_ID setting, int32_t value){return(app->global_set_setting_(app, setting, value));}
static bool32 global_set_mapping(Application_Links *app, void *data, int32_t size){return(app->global_set_mapping_(app, data, size));}
static bool32 exec_system_command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer_id, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags){return(app->exec_system_command_(app, view, buffer_id, path, path_len, command, command_len, flags));}
static void clipboard_post(Application_Links *app, int32_t clipboard_id, char *str, int32_t len){(app->clipboard_post_(app, clipboard_id, str, len));}
static int32_t clipboard_count(Application_Links *app, int32_t clipboard_id){return(app->clipboard_count_(app, clipboard_id));}
static int32_t clipboard_index(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len){return(app->clipboard_index_(app, clipboard_id, item_index, out, len));}
static Parse_Context_ID create_parse_context(Application_Links *app, Parser_String_And_Type *kw, uint32_t kw_count, Parser_String_And_Type *pp, uint32_t pp_count){return(app->create_parse_context_(app, kw, kw_count, pp, pp_count));}
static int32_t get_buffer_count(Application_Links *app){return(app->get_buffer_count_(app));}
static Buffer_Summary get_buffer_first(Application_Links *app, Access_Flag access){return(app->get_buffer_first_(app, access));}
static void get_buffer_next(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access){(app->get_buffer_next_(app, buffer, access));}
static Buffer_Summary get_buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access){return(app->get_buffer_(app, buffer_id, access));}
static Buffer_Summary get_buffer_by_name(Application_Links *app, char *name, int32_t len, Access_Flag access){return(app->get_buffer_by_name_(app, name, len, access));}
static Buffer_Summary get_buffer_by_file_name(Application_Links *app, char *name, int32_t len, Access_Flag access){return(app->get_buffer_by_file_name_(app, name, len, access));}
static bool32 buffer_read_range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *out){return(app->buffer_read_range_(app, buffer, start, end, out));}
static bool32 buffer_replace_range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *str, int32_t len){return(app->buffer_replace_range_(app, buffer, start, end, str, len));}
static bool32 buffer_compute_cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out){return(app->buffer_compute_cursor_(app, buffer, seek, cursor_out));}
static bool32 buffer_batch_edit(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type){return(app->buffer_batch_edit_(app, buffer, str, str_len, edits, edit_count, type));}
static bool32 buffer_get_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out){return(app->buffer_get_setting_(app, buffer, setting, value_out));}
static bool32 buffer_set_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value){return(app->buffer_set_setting_(app, buffer, setting, value));}
static Managed_Scope buffer_get_managed_scope(Application_Links *app, Buffer_ID buffer_id){return(app->buffer_get_managed_scope_(app, buffer_id));}
static int32_t buffer_token_count(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_token_count_(app, buffer));}
static bool32 buffer_read_tokens(Application_Links *app, Buffer_Summary *buffer, int32_t start_token, int32_t end_token, Cpp_Token *tokens_out){return(app->buffer_read_tokens_(app, buffer, start_token, end_token, tokens_out));}
static bool32 buffer_get_token_range(Application_Links *app, Buffer_Summary *buffer, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out){return(app->buffer_get_token_range_(app, buffer, first_token_out, one_past_last_token_out));}
static bool32 buffer_get_token_index(Application_Links *app, Buffer_Summary *buffer, int32_t pos, Cpp_Get_Token_Result *get_result){return(app->buffer_get_token_index_(app, buffer, pos, get_result));}
static bool32 buffer_send_end_signal(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_send_end_signal_(app, buffer));}
static Buffer_Summary create_buffer(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags){return(app->create_buffer_(app, filename, filename_len, flags));}
static bool32 save_buffer(Application_Links *app, Buffer_Summary *buffer, char *file_name, int32_t file_name_len, uint32_t flags){return(app->save_buffer_(app, buffer, file_name, file_name_len, flags));}
static Buffer_Kill_Result kill_buffer(Application_Links *app, Buffer_Identifier buffer, Buffer_Kill_Flag flags){return(app->kill_buffer_(app, buffer, flags));}
static Buffer_Reopen_Result reopen_buffer(Application_Links *app, Buffer_Summary *buffer, Buffer_Reopen_Flag flags){return(app->reopen_buffer_(app, buffer, flags));}
static View_Summary get_view_first(Application_Links *app, Access_Flag access){return(app->get_view_first_(app, access));}
static void get_view_next(Application_Links *app, View_Summary *view, Access_Flag access){(app->get_view_next_(app, view, access));}
static View_Summary get_view(Application_Links *app, View_ID view_id, Access_Flag access){return(app->get_view_(app, view_id, access));}
static View_Summary get_active_view(Application_Links *app, Access_Flag access){return(app->get_active_view_(app, access));}
static View_Summary open_view(Application_Links *app, View_Summary *view_location, View_Split_Position position){return(app->open_view_(app, view_location, position));}
static bool32 close_view(Application_Links *app, View_Summary *view){return(app->close_view_(app, view));}
static bool32 set_active_view(Application_Links *app, View_Summary *view){return(app->set_active_view_(app, view));}
static bool32 view_get_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out){return(app->view_get_setting_(app, view, setting, value_out));}
static bool32 view_set_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value){return(app->view_set_setting_(app, view, setting, value));}
static Managed_Scope view_get_managed_scope(Application_Links *app, View_ID view_id){return(app->view_get_managed_scope_(app, view_id));}
static bool32 view_set_split(Application_Links *app, View_Summary *view, View_Split_Kind kind, float t){return(app->view_set_split_(app, view, kind, t));}
static i32_Rect view_get_enclosure_rect(Application_Links *app, View_Summary *view){return(app->view_get_enclosure_rect_(app, view));}
static bool32 view_compute_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out){return(app->view_compute_cursor_(app, view, seek, cursor_out));}
static bool32 view_set_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x){return(app->view_set_cursor_(app, view, seek, set_preferred_x));}
static bool32 view_set_scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll){return(app->view_set_scroll_(app, view, scroll));}
static bool32 view_set_mark(Application_Links *app, View_Summary *view, Buffer_Seek seek){return(app->view_set_mark_(app, view, seek));}
static bool32 view_set_highlight(Application_Links *app, View_Summary *view, int32_t start, int32_t end, bool32 turn_on){return(app->view_set_highlight_(app, view, start, end, turn_on));}
static bool32 view_set_buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags){return(app->view_set_buffer_(app, view, buffer_id, flags));}
static bool32 view_post_fade(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color){return(app->view_post_fade_(app, view, seconds, start, end, color));}
static bool32 view_begin_ui_mode(Application_Links *app, View_Summary *view){return(app->view_begin_ui_mode_(app, view));}
static bool32 view_end_ui_mode(Application_Links *app, View_Summary *view){return(app->view_end_ui_mode_(app, view));}
static bool32 view_set_ui(Application_Links *app, View_Summary *view, UI_Control *control, UI_Quit_Function_Type *quit_function){return(app->view_set_ui_(app, view, control, quit_function));}
static UI_Control view_get_ui_copy(Application_Links *app, View_Summary *view, struct Partition *part){return(app->view_get_ui_copy_(app, view, part));}
static Managed_Scope create_user_managed_scope(Application_Links *app){return(app->create_user_managed_scope_(app));}
static bool32 destroy_user_managed_scope(Application_Links *app, Managed_Scope scope){return(app->destroy_user_managed_scope_(app, scope));}
static Managed_Scope get_global_managed_scope(Application_Links *app){return(app->get_global_managed_scope_(app));}
static Managed_Scope get_managed_scope_with_multiple_dependencies(Application_Links *app, Managed_Scope *scopes, int32_t count){return(app->get_managed_scope_with_multiple_dependencies_(app, scopes, count));}
static bool32 managed_scope_clear_contents(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_contents_(app, scope));}
static bool32 managed_scope_clear_self_all_dependent_scopes(Application_Links *app, Managed_Scope scope){return(app->managed_scope_clear_self_all_dependent_scopes_(app, scope));}
static Managed_Variable_ID managed_variable_create(Application_Links *app, char *null_terminated_name, uint64_t default_value){return(app->managed_variable_create_(app, null_terminated_name, default_value));}
static Managed_Variable_ID managed_variable_get_id(Application_Links *app, char *null_terminated_name){return(app->managed_variable_get_id_(app, null_terminated_name));}
static Managed_Variable_ID managed_variable_create_or_get_id(Application_Links *app, char *null_terminated_name, uint64_t default_value){return(app->managed_variable_create_or_get_id_(app, null_terminated_name, default_value));}
static bool32 managed_variable_set(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t value){return(app->managed_variable_set_(app, scope, id, value));}
static bool32 managed_variable_get(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, uint64_t *value_out){return(app->managed_variable_get_(app, scope, id, value_out));}
static Managed_Object alloc_managed_memory_in_scope(Application_Links *app, Managed_Scope scope, int32_t item_size, int32_t count){return(app->alloc_managed_memory_in_scope_(app, scope, item_size, count));}
static Managed_Object alloc_buffer_markers_on_buffer(Application_Links *app, Buffer_ID buffer_id, int32_t count, Managed_Scope *optional_extra_scope){return(app->alloc_buffer_markers_on_buffer_(app, buffer_id, count, optional_extra_scope));}
static Marker_Visual create_marker_visual(Application_Links *app, Managed_Object object){return(app->create_marker_visual_(app, object));}
static bool32 marker_visual_set_effect(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style){return(app->marker_visual_set_effect_(app, visual, type, color, text_color, text_style));}
static bool32 marker_visual_set_take_rule(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule){return(app->marker_visual_set_take_rule_(app, visual, take_rule));}
static bool32 marker_visual_set_priority(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority){return(app->marker_visual_set_priority_(app, visual, priority));}
static bool32 marker_visual_set_view_key(Application_Links *app, Marker_Visual visual, View_ID key_view_id){return(app->marker_visual_set_view_key_(app, visual, key_view_id));}
static bool32 destroy_marker_visual(Application_Links *app, Marker_Visual visual){return(app->destroy_marker_visual_(app, visual));}
static int32_t buffer_markers_get_attached_visual_count(Application_Links *app, Managed_Object object){return(app->buffer_markers_get_attached_visual_count_(app, object));}
static Marker_Visual* buffer_markers_get_attached_visual(Application_Links *app, Partition *part, Managed_Object object){return(app->buffer_markers_get_attached_visual_(app, part, object));}
static uint32_t managed_object_get_item_size(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_size_(app, object));}
static uint32_t managed_object_get_item_count(Application_Links *app, Managed_Object object){return(app->managed_object_get_item_count_(app, object));}
static Managed_Object_Type managed_object_get_type(Application_Links *app, Managed_Object object){return(app->managed_object_get_type_(app, object));}
static Managed_Scope managed_object_get_containing_scope(Application_Links *app, Managed_Object object){return(app->managed_object_get_containing_scope_(app, object));}
static bool32 managed_object_free(Application_Links *app, Managed_Object object){return(app->managed_object_free_(app, object));}
static bool32 managed_object_store_data(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem){return(app->managed_object_store_data_(app, object, first_index, count, mem));}
static bool32 managed_object_load_data(Application_Links *app, Managed_Object object, uint32_t first_index, uint32_t count, void *mem_out){return(app->managed_object_load_data_(app, object, first_index, count, mem_out));}
static User_Input get_user_input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type){return(app->get_user_input_(app, get_type, abort_type));}
static User_Input get_command_input(Application_Links *app){return(app->get_command_input_(app));}
static void set_command_input(Application_Links *app, Key_Event_Data key_data){(app->set_command_input_(app, key_data));}
static Mouse_State get_mouse_state(Application_Links *app){return(app->get_mouse_state_(app));}
static int32_t get_active_query_bars(Application_Links *app, View_ID view_id, int32_t max_result_count, Query_Bar **result_array){return(app->get_active_query_bars_(app, view_id, max_result_count, result_array));}
static bool32 start_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){return(app->start_query_bar_(app, bar, flags));}
static void end_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){(app->end_query_bar_(app, bar, flags));}
static void print_message(Application_Links *app, char *str, int32_t len){(app->print_message_(app, str, len));}
static int32_t get_theme_count(Application_Links *app){return(app->get_theme_count_(app));}
static String get_theme_name(Application_Links *app, struct Partition *arena, int32_t index){return(app->get_theme_name_(app, arena, index));}
static void create_theme(Application_Links *app, Theme *theme, char *name, int32_t len){(app->create_theme_(app, theme, name, len));}
static void change_theme(Application_Links *app, char *name, int32_t len){(app->change_theme_(app, name, len));}
static bool32 change_theme_by_index(Application_Links *app, int32_t index){return(app->change_theme_by_index_(app, index));}
static Face_ID get_largest_face_id(Application_Links *app){return(app->get_largest_face_id_(app));}
static bool32 set_global_face(Application_Links *app, Face_ID id, bool32 apply_to_all_buffers){return(app->set_global_face_(app, id, apply_to_all_buffers));}
static bool32 buffer_set_face(Application_Links *app, Buffer_Summary *buffer, Face_ID id){return(app->buffer_set_face_(app, buffer, id));}
static History_Record_Index buffer_history_newest_record_index(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_history_newest_record_index_(app, buffer));}
static Record_Info buffer_history_get_record_info(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index){return(app->buffer_history_get_record_info_(app, buffer, index));}
static Record_Info buffer_history_get_group_sub_record(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index, int32_t sub_index){return(app->buffer_history_get_group_sub_record_(app, buffer, index, sub_index));}
static History_Record_Index buffer_history_get_current_state_index(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_history_get_current_state_index_(app, buffer));}
static bool32 buffer_history_set_current_state_index(Application_Links *app, Buffer_Summary *buffer, History_Record_Index index){return(app->buffer_history_set_current_state_index_(app, buffer, index));}
static bool32 buffer_history_merge_record_range(Application_Links *app, Buffer_Summary *buffer, History_Record_Index first_index, History_Record_Index last_index){return(app->buffer_history_merge_record_range_(app, buffer, first_index, last_index));}
static bool32 buffer_history_clear_after_current_state(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_history_clear_after_current_state_(app, buffer));}
static void global_history_edit_group_begin(Application_Links *app){(app->global_history_edit_group_begin_(app));}
static void global_history_edit_group_end(Application_Links *app){(app->global_history_edit_group_end_(app));}
static Face_Description get_face_description(Application_Links *app, Face_ID id){return(app->get_face_description_(app, id));}
static Face_ID get_face_id(Application_Links *app, Buffer_Summary *buffer){return(app->get_face_id_(app, buffer));}
static Face_ID try_create_new_face(Application_Links *app, Face_Description *description){return(app->try_create_new_face_(app, description));}
static bool32 try_modify_face(Application_Links *app, Face_ID id, Face_Description *description){return(app->try_modify_face_(app, id, description));}
static bool32 try_release_face(Application_Links *app, Face_ID id, Face_ID replacement_id){return(app->try_release_face_(app, id, replacement_id));}
static int32_t get_available_font_count(Application_Links *app){return(app->get_available_font_count_(app));}
static Available_Font get_available_font(Application_Links *app, int32_t index){return(app->get_available_font_(app, index));}
static void set_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->set_theme_colors_(app, colors, count));}
static void get_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->get_theme_colors_(app, colors, count));}
static int32_t directory_get_hot(Application_Links *app, char *out, int32_t capacity){return(app->directory_get_hot_(app, out, capacity));}
static bool32 directory_set_hot(Application_Links *app, char *str, int32_t len){return(app->directory_set_hot_(app, str, len));}
static File_List get_file_list(Application_Links *app, char *dir, int32_t len){return(app->get_file_list_(app, dir, len));}
static void free_file_list(Application_Links *app, File_List list){(app->free_file_list_(app, list));}
static void set_gui_up_down_keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier){(app->set_gui_up_down_keys_(app, up_key, up_key_modifier, down_key, down_key_modifier));}
static void* memory_allocate(Application_Links *app, int32_t size){return(app->memory_allocate_(app, size));}
static bool32 memory_set_protection(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags){return(app->memory_set_protection_(app, ptr, size, flags));}
static void memory_free(Application_Links *app, void *ptr, int32_t size){(app->memory_free_(app, ptr, size));}
static bool32 file_exists(Application_Links *app, char *filename, int32_t len){return(app->file_exists_(app, filename, len));}
static bool32 directory_cd(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len){return(app->directory_cd_(app, dir, len, capacity, rel_path, rel_len));}
static int32_t get_4ed_path(Application_Links *app, char *out, int32_t capacity){return(app->get_4ed_path_(app, out, capacity));}
static void show_mouse_cursor(Application_Links *app, Mouse_Cursor_Show_Type show){(app->show_mouse_cursor_(app, show));}
static bool32 set_fullscreen(Application_Links *app, bool32 full_screen){return(app->set_fullscreen_(app, full_screen));}
static bool32 is_fullscreen(Application_Links *app){return(app->is_fullscreen_(app));}
static void send_exit_signal(Application_Links *app){(app->send_exit_signal_(app));}
static void set_window_title(Application_Links *app, char *title){(app->set_window_title_(app, title));}
static Microsecond_Time_Stamp get_microseconds_timestamp(Application_Links *app){return(app->get_microseconds_timestamp_(app));}
static float draw_string(Application_Links *app, Face_ID font_id, String str, int32_t x, int32_t y, int_color color, uint32_t flags, float dx, float dy){return(app->draw_string_(app, font_id, str, x, y, color, flags, dx, dy));}
static float get_string_advance(Application_Links *app, Face_ID font_id, String str){return(app->get_string_advance_(app, font_id, str));}
static void draw_rectangle(Application_Links *app, f32_Rect rect, int_color color){(app->draw_rectangle_(app, rect, color));}
static void draw_rectangle_outline(Application_Links *app, f32_Rect rect, int_color color){(app->draw_rectangle_outline_(app, rect, color));}
static Face_ID get_default_font_for_view(Application_Links *app, View_ID view_id){return(app->get_default_font_for_view_(app, view_id));}
#endif
