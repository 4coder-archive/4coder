struct Application_Links;
#define EXEC_COMMAND_SIG(n) bool32 n(Application_Links *app, Command_ID command_id)
#define EXEC_SYSTEM_COMMAND_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Identifier buffer, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags)
#define CLIPBOARD_POST_SIG(n) void n(Application_Links *app, int32_t clipboard_id, char *str, uint32_t len)
#define CLIPBOARD_COUNT_SIG(n) uint32_t n(Application_Links *app, uint32_t clipboard_id)
#define CLIPBOARD_INDEX_SIG(n) uint32_t n(Application_Links *app, uint32_t clipboard_id, uint32_t item_index, char *out, uint32_t len)
#define GET_BUFFER_COUNT_SIG(n) int32_t n(Application_Links *app)
#define GET_BUFFER_FIRST_SIG(n) Buffer_Summary n(Application_Links *app, Access_Flag access)
#define GET_BUFFER_NEXT_SIG(n) void n(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access)
#define GET_BUFFER_SIG(n) Buffer_Summary n(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
#define GET_BUFFER_BY_NAME_SIG(n) Buffer_Summary n(Application_Links *app, char *name, int32_t len, Access_Flag access)
#define BUFFER_READ_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, size_t start, size_t end, char *out)
#define BUFFER_REPLACE_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, size_t start, size_t end, char *str, size_t len)
#define BUFFER_COMPUTE_CURSOR_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out)
#define BUFFER_BATCH_EDIT_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *str, size_t str_len, Buffer_Edit *edits, uint32_t edit_count, Buffer_Batch_Edit_Type type)
#define BUFFER_ADD_MARKERS_SIG(n) Marker_Handle n(Application_Links *app, Buffer_Summary *buffer, uint32_t marker_count)
#define BUFFER_SET_MARKERS_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker, uint32_t first_marker_index, uint32_t marker_count, Marker *source_markers)
#define BUFFER_GET_MARKERS_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker, uint32_t first_marker_index, uint32_t marker_count, Marker *markers_out)
#define BUFFER_REMOVE_MARKERS_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker)
#define BUFFER_GET_SETTING_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out)
#define BUFFER_SET_SETTING_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value)
#define BUFFER_TOKEN_COUNT_SIG(n) int32_t n(Application_Links *app, Buffer_Summary *buffer)
#define BUFFER_READ_TOKENS_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, size_t start_token, size_t end_token, Cpp_Token *tokens_out)
#define BUFFER_GET_TOKEN_INDEX_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, size_t pos, Cpp_Get_Token_Result *get_result)
#define BEGIN_BUFFER_CREATION_SIG(n) bool32 n(Application_Links *app, Buffer_Creation_Data *data, Buffer_Create_Flag flags)
#define BUFFER_CREATION_NAME_SIG(n) bool32 n(Application_Links *app, Buffer_Creation_Data *data, char *filename, int32_t filename_len, uint32_t flags)
#define END_BUFFER_CREATION_SIG(n) Buffer_Summary n(Application_Links *app, Buffer_Creation_Data *data)
#define SAVE_BUFFER_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *filename, int32_t filename_len, uint32_t flags)
#define KILL_BUFFER_SIG(n) bool32 n(Application_Links *app, Buffer_Identifier buffer, View_ID view_id, Buffer_Kill_Flag flags)
#define GET_VIEW_FIRST_SIG(n) View_Summary n(Application_Links *app, Access_Flag access)
#define GET_VIEW_NEXT_SIG(n) void n(Application_Links *app, View_Summary *view, Access_Flag access)
#define GET_VIEW_SIG(n) View_Summary n(Application_Links *app, View_ID view_id, Access_Flag access)
#define GET_ACTIVE_VIEW_SIG(n) View_Summary n(Application_Links *app, Access_Flag access)
#define OPEN_VIEW_SIG(n) View_Summary n(Application_Links *app, View_Summary *view_location, View_Split_Position position)
#define CLOSE_VIEW_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define SET_ACTIVE_VIEW_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define VIEW_GET_SETTING_SIG(n) bool32 n(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out)
#define VIEW_SET_SETTING_SIG(n) bool32 n(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value)
#define VIEW_SET_SPLIT_PROPORTION_SIG(n) bool32 n(Application_Links *app, View_Summary *view, float t)
#define VIEW_COMPUTE_CURSOR_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out)
#define VIEW_SET_CURSOR_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x)
#define VIEW_SET_SCROLL_SIG(n) bool32 n(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll)
#define VIEW_SET_MARK_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek)
#define VIEW_SET_HIGHLIGHT_SIG(n) bool32 n(Application_Links *app, View_Summary *view, size_t start, size_t end, bool32 turn_on)
#define VIEW_SET_BUFFER_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags)
#define VIEW_POST_FADE_SIG(n) bool32 n(Application_Links *app, View_Summary *view, float seconds, size_t start, size_t end, int_color color)
#define GET_USER_INPUT_SIG(n) User_Input n(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
#define GET_COMMAND_INPUT_SIG(n) User_Input n(Application_Links *app)
#define GET_MOUSE_STATE_SIG(n) Mouse_State n(Application_Links *app)
#define START_QUERY_BAR_SIG(n) bool32 n(Application_Links *app, Query_Bar *bar, uint32_t flags)
#define END_QUERY_BAR_SIG(n) void n(Application_Links *app, Query_Bar *bar, uint32_t flags)
#define PRINT_MESSAGE_SIG(n) void n(Application_Links *app, char *str, int32_t len)
#define CHANGE_THEME_SIG(n) void n(Application_Links *app, char *name, int32_t len)
#define CHANGE_FONT_SIG(n) void n(Application_Links *app, char *name, int32_t len, bool32 apply_to_all_files)
#define BUFFER_SET_FONT_SIG(n) void n(Application_Links *app, Buffer_Summary *buffer, char *name, int32_t len)
#define BUFFER_GET_FONT_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *name_out, int32_t name_max)
#define SET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, int32_t count)
#define GET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, int32_t count)
#define DIRECTORY_GET_HOT_SIG(n) int32_t n(Application_Links *app, char *out, int32_t capacity)
#define GET_FILE_LIST_SIG(n) File_List n(Application_Links *app, char *dir, int32_t len)
#define FREE_FILE_LIST_SIG(n) void n(Application_Links *app, File_List list)
#define SET_GUI_UP_DOWN_KEYS_SIG(n) void n(Application_Links *app, int16_t up_key, int16_t down_key)
#define MEMORY_ALLOCATE_SIG(n) void* n(Application_Links *app, size_t size)
#define MEMORY_SET_PROTECTION_SIG(n) bool32 n(Application_Links *app, void *ptr, size_t size, Memory_Protect_Flags flags)
#define MEMORY_FREE_SIG(n) void n(Application_Links *app, void *ptr, size_t size)
#define FILE_EXISTS_SIG(n) bool32 n(Application_Links *app, char *filename, int32_t len)
#define DIRECTORY_CD_SIG(n) bool32 n(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len)
#define GET_4ED_PATH_SIG(n) int32_t n(Application_Links *app, char *out, int32_t capacity)
#define SHOW_MOUSE_CURSOR_SIG(n) void n(Application_Links *app, Mouse_Cursor_Show_Type show)
#define TOGGLE_FULLSCREEN_SIG(n) void n(Application_Links *app)
#define IS_FULLSCREEN_SIG(n) bool32 n(Application_Links *app)
#define SEND_EXIT_SIGNAL_SIG(n) void n(Application_Links *app)
typedef EXEC_COMMAND_SIG(Exec_Command_Function);
typedef EXEC_SYSTEM_COMMAND_SIG(Exec_System_Command_Function);
typedef CLIPBOARD_POST_SIG(Clipboard_Post_Function);
typedef CLIPBOARD_COUNT_SIG(Clipboard_Count_Function);
typedef CLIPBOARD_INDEX_SIG(Clipboard_Index_Function);
typedef GET_BUFFER_COUNT_SIG(Get_Buffer_Count_Function);
typedef GET_BUFFER_FIRST_SIG(Get_Buffer_First_Function);
typedef GET_BUFFER_NEXT_SIG(Get_Buffer_Next_Function);
typedef GET_BUFFER_SIG(Get_Buffer_Function);
typedef GET_BUFFER_BY_NAME_SIG(Get_Buffer_By_Name_Function);
typedef BUFFER_READ_RANGE_SIG(Buffer_Read_Range_Function);
typedef BUFFER_REPLACE_RANGE_SIG(Buffer_Replace_Range_Function);
typedef BUFFER_COMPUTE_CURSOR_SIG(Buffer_Compute_Cursor_Function);
typedef BUFFER_BATCH_EDIT_SIG(Buffer_Batch_Edit_Function);
typedef BUFFER_ADD_MARKERS_SIG(Buffer_Add_Markers_Function);
typedef BUFFER_SET_MARKERS_SIG(Buffer_Set_Markers_Function);
typedef BUFFER_GET_MARKERS_SIG(Buffer_Get_Markers_Function);
typedef BUFFER_REMOVE_MARKERS_SIG(Buffer_Remove_Markers_Function);
typedef BUFFER_GET_SETTING_SIG(Buffer_Get_Setting_Function);
typedef BUFFER_SET_SETTING_SIG(Buffer_Set_Setting_Function);
typedef BUFFER_TOKEN_COUNT_SIG(Buffer_Token_Count_Function);
typedef BUFFER_READ_TOKENS_SIG(Buffer_Read_Tokens_Function);
typedef BUFFER_GET_TOKEN_INDEX_SIG(Buffer_Get_Token_Index_Function);
typedef BEGIN_BUFFER_CREATION_SIG(Begin_Buffer_Creation_Function);
typedef BUFFER_CREATION_NAME_SIG(Buffer_Creation_Name_Function);
typedef END_BUFFER_CREATION_SIG(End_Buffer_Creation_Function);
typedef SAVE_BUFFER_SIG(Save_Buffer_Function);
typedef KILL_BUFFER_SIG(Kill_Buffer_Function);
typedef GET_VIEW_FIRST_SIG(Get_View_First_Function);
typedef GET_VIEW_NEXT_SIG(Get_View_Next_Function);
typedef GET_VIEW_SIG(Get_View_Function);
typedef GET_ACTIVE_VIEW_SIG(Get_Active_View_Function);
typedef OPEN_VIEW_SIG(Open_View_Function);
typedef CLOSE_VIEW_SIG(Close_View_Function);
typedef SET_ACTIVE_VIEW_SIG(Set_Active_View_Function);
typedef VIEW_GET_SETTING_SIG(View_Get_Setting_Function);
typedef VIEW_SET_SETTING_SIG(View_Set_Setting_Function);
typedef VIEW_SET_SPLIT_PROPORTION_SIG(View_Set_Split_Proportion_Function);
typedef VIEW_COMPUTE_CURSOR_SIG(View_Compute_Cursor_Function);
typedef VIEW_SET_CURSOR_SIG(View_Set_Cursor_Function);
typedef VIEW_SET_SCROLL_SIG(View_Set_Scroll_Function);
typedef VIEW_SET_MARK_SIG(View_Set_Mark_Function);
typedef VIEW_SET_HIGHLIGHT_SIG(View_Set_Highlight_Function);
typedef VIEW_SET_BUFFER_SIG(View_Set_Buffer_Function);
typedef VIEW_POST_FADE_SIG(View_Post_Fade_Function);
typedef GET_USER_INPUT_SIG(Get_User_Input_Function);
typedef GET_COMMAND_INPUT_SIG(Get_Command_Input_Function);
typedef GET_MOUSE_STATE_SIG(Get_Mouse_State_Function);
typedef START_QUERY_BAR_SIG(Start_Query_Bar_Function);
typedef END_QUERY_BAR_SIG(End_Query_Bar_Function);
typedef PRINT_MESSAGE_SIG(Print_Message_Function);
typedef CHANGE_THEME_SIG(Change_Theme_Function);
typedef CHANGE_FONT_SIG(Change_Font_Function);
typedef BUFFER_SET_FONT_SIG(Buffer_Set_Font_Function);
typedef BUFFER_GET_FONT_SIG(Buffer_Get_Font_Function);
typedef SET_THEME_COLORS_SIG(Set_Theme_Colors_Function);
typedef GET_THEME_COLORS_SIG(Get_Theme_Colors_Function);
typedef DIRECTORY_GET_HOT_SIG(Directory_Get_Hot_Function);
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
typedef TOGGLE_FULLSCREEN_SIG(Toggle_Fullscreen_Function);
typedef IS_FULLSCREEN_SIG(Is_Fullscreen_Function);
typedef SEND_EXIT_SIGNAL_SIG(Send_Exit_Signal_Function);
struct Application_Links{
#if defined(ALLOW_DEP_4CODER)
Exec_Command_Function *exec_command;
Exec_System_Command_Function *exec_system_command;
Clipboard_Post_Function *clipboard_post;
Clipboard_Count_Function *clipboard_count;
Clipboard_Index_Function *clipboard_index;
Get_Buffer_Count_Function *get_buffer_count;
Get_Buffer_First_Function *get_buffer_first;
Get_Buffer_Next_Function *get_buffer_next;
Get_Buffer_Function *get_buffer;
Get_Buffer_By_Name_Function *get_buffer_by_name;
Buffer_Read_Range_Function *buffer_read_range;
Buffer_Replace_Range_Function *buffer_replace_range;
Buffer_Compute_Cursor_Function *buffer_compute_cursor;
Buffer_Batch_Edit_Function *buffer_batch_edit;
Buffer_Add_Markers_Function *buffer_add_markers;
Buffer_Set_Markers_Function *buffer_set_markers;
Buffer_Get_Markers_Function *buffer_get_markers;
Buffer_Remove_Markers_Function *buffer_remove_markers;
Buffer_Get_Setting_Function *buffer_get_setting;
Buffer_Set_Setting_Function *buffer_set_setting;
Buffer_Token_Count_Function *buffer_token_count;
Buffer_Read_Tokens_Function *buffer_read_tokens;
Buffer_Get_Token_Index_Function *buffer_get_token_index;
Begin_Buffer_Creation_Function *begin_buffer_creation;
Buffer_Creation_Name_Function *buffer_creation_name;
End_Buffer_Creation_Function *end_buffer_creation;
Save_Buffer_Function *save_buffer;
Kill_Buffer_Function *kill_buffer;
Get_View_First_Function *get_view_first;
Get_View_Next_Function *get_view_next;
Get_View_Function *get_view;
Get_Active_View_Function *get_active_view;
Open_View_Function *open_view;
Close_View_Function *close_view;
Set_Active_View_Function *set_active_view;
View_Get_Setting_Function *view_get_setting;
View_Set_Setting_Function *view_set_setting;
View_Set_Split_Proportion_Function *view_set_split_proportion;
View_Compute_Cursor_Function *view_compute_cursor;
View_Set_Cursor_Function *view_set_cursor;
View_Set_Scroll_Function *view_set_scroll;
View_Set_Mark_Function *view_set_mark;
View_Set_Highlight_Function *view_set_highlight;
View_Set_Buffer_Function *view_set_buffer;
View_Post_Fade_Function *view_post_fade;
Get_User_Input_Function *get_user_input;
Get_Command_Input_Function *get_command_input;
Get_Mouse_State_Function *get_mouse_state;
Start_Query_Bar_Function *start_query_bar;
End_Query_Bar_Function *end_query_bar;
Print_Message_Function *print_message;
Change_Theme_Function *change_theme;
Change_Font_Function *change_font;
Buffer_Set_Font_Function *buffer_set_font;
Buffer_Get_Font_Function *buffer_get_font;
Set_Theme_Colors_Function *set_theme_colors;
Get_Theme_Colors_Function *get_theme_colors;
Directory_Get_Hot_Function *directory_get_hot;
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
Toggle_Fullscreen_Function *toggle_fullscreen;
Is_Fullscreen_Function *is_fullscreen;
Send_Exit_Signal_Function *send_exit_signal;
#else
Exec_Command_Function *exec_command_;
Exec_System_Command_Function *exec_system_command_;
Clipboard_Post_Function *clipboard_post_;
Clipboard_Count_Function *clipboard_count_;
Clipboard_Index_Function *clipboard_index_;
Get_Buffer_Count_Function *get_buffer_count_;
Get_Buffer_First_Function *get_buffer_first_;
Get_Buffer_Next_Function *get_buffer_next_;
Get_Buffer_Function *get_buffer_;
Get_Buffer_By_Name_Function *get_buffer_by_name_;
Buffer_Read_Range_Function *buffer_read_range_;
Buffer_Replace_Range_Function *buffer_replace_range_;
Buffer_Compute_Cursor_Function *buffer_compute_cursor_;
Buffer_Batch_Edit_Function *buffer_batch_edit_;
Buffer_Add_Markers_Function *buffer_add_markers_;
Buffer_Set_Markers_Function *buffer_set_markers_;
Buffer_Get_Markers_Function *buffer_get_markers_;
Buffer_Remove_Markers_Function *buffer_remove_markers_;
Buffer_Get_Setting_Function *buffer_get_setting_;
Buffer_Set_Setting_Function *buffer_set_setting_;
Buffer_Token_Count_Function *buffer_token_count_;
Buffer_Read_Tokens_Function *buffer_read_tokens_;
Buffer_Get_Token_Index_Function *buffer_get_token_index_;
Begin_Buffer_Creation_Function *begin_buffer_creation_;
Buffer_Creation_Name_Function *buffer_creation_name_;
End_Buffer_Creation_Function *end_buffer_creation_;
Save_Buffer_Function *save_buffer_;
Kill_Buffer_Function *kill_buffer_;
Get_View_First_Function *get_view_first_;
Get_View_Next_Function *get_view_next_;
Get_View_Function *get_view_;
Get_Active_View_Function *get_active_view_;
Open_View_Function *open_view_;
Close_View_Function *close_view_;
Set_Active_View_Function *set_active_view_;
View_Get_Setting_Function *view_get_setting_;
View_Set_Setting_Function *view_set_setting_;
View_Set_Split_Proportion_Function *view_set_split_proportion_;
View_Compute_Cursor_Function *view_compute_cursor_;
View_Set_Cursor_Function *view_set_cursor_;
View_Set_Scroll_Function *view_set_scroll_;
View_Set_Mark_Function *view_set_mark_;
View_Set_Highlight_Function *view_set_highlight_;
View_Set_Buffer_Function *view_set_buffer_;
View_Post_Fade_Function *view_post_fade_;
Get_User_Input_Function *get_user_input_;
Get_Command_Input_Function *get_command_input_;
Get_Mouse_State_Function *get_mouse_state_;
Start_Query_Bar_Function *start_query_bar_;
End_Query_Bar_Function *end_query_bar_;
Print_Message_Function *print_message_;
Change_Theme_Function *change_theme_;
Change_Font_Function *change_font_;
Buffer_Set_Font_Function *buffer_set_font_;
Buffer_Get_Font_Function *buffer_get_font_;
Set_Theme_Colors_Function *set_theme_colors_;
Get_Theme_Colors_Function *get_theme_colors_;
Directory_Get_Hot_Function *directory_get_hot_;
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
Toggle_Fullscreen_Function *toggle_fullscreen_;
Is_Fullscreen_Function *is_fullscreen_;
Send_Exit_Signal_Function *send_exit_signal_;
#endif
void *memory;
int32_t memory_size;
void *cmd_context;
void *system_links;
void *current_coroutine;
int32_t type_coroutine;
};
#define FillAppLinksAPI(app_links) do{\
app_links->exec_command_ = Exec_Command;\
app_links->exec_system_command_ = Exec_System_Command;\
app_links->clipboard_post_ = Clipboard_Post;\
app_links->clipboard_count_ = Clipboard_Count;\
app_links->clipboard_index_ = Clipboard_Index;\
app_links->get_buffer_count_ = Get_Buffer_Count;\
app_links->get_buffer_first_ = Get_Buffer_First;\
app_links->get_buffer_next_ = Get_Buffer_Next;\
app_links->get_buffer_ = Get_Buffer;\
app_links->get_buffer_by_name_ = Get_Buffer_By_Name;\
app_links->buffer_read_range_ = Buffer_Read_Range;\
app_links->buffer_replace_range_ = Buffer_Replace_Range;\
app_links->buffer_compute_cursor_ = Buffer_Compute_Cursor;\
app_links->buffer_batch_edit_ = Buffer_Batch_Edit;\
app_links->buffer_add_markers_ = Buffer_Add_Markers;\
app_links->buffer_set_markers_ = Buffer_Set_Markers;\
app_links->buffer_get_markers_ = Buffer_Get_Markers;\
app_links->buffer_remove_markers_ = Buffer_Remove_Markers;\
app_links->buffer_get_setting_ = Buffer_Get_Setting;\
app_links->buffer_set_setting_ = Buffer_Set_Setting;\
app_links->buffer_token_count_ = Buffer_Token_Count;\
app_links->buffer_read_tokens_ = Buffer_Read_Tokens;\
app_links->buffer_get_token_index_ = Buffer_Get_Token_Index;\
app_links->begin_buffer_creation_ = Begin_Buffer_Creation;\
app_links->buffer_creation_name_ = Buffer_Creation_Name;\
app_links->end_buffer_creation_ = End_Buffer_Creation;\
app_links->save_buffer_ = Save_Buffer;\
app_links->kill_buffer_ = Kill_Buffer;\
app_links->get_view_first_ = Get_View_First;\
app_links->get_view_next_ = Get_View_Next;\
app_links->get_view_ = Get_View;\
app_links->get_active_view_ = Get_Active_View;\
app_links->open_view_ = Open_View;\
app_links->close_view_ = Close_View;\
app_links->set_active_view_ = Set_Active_View;\
app_links->view_get_setting_ = View_Get_Setting;\
app_links->view_set_setting_ = View_Set_Setting;\
app_links->view_set_split_proportion_ = View_Set_Split_Proportion;\
app_links->view_compute_cursor_ = View_Compute_Cursor;\
app_links->view_set_cursor_ = View_Set_Cursor;\
app_links->view_set_scroll_ = View_Set_Scroll;\
app_links->view_set_mark_ = View_Set_Mark;\
app_links->view_set_highlight_ = View_Set_Highlight;\
app_links->view_set_buffer_ = View_Set_Buffer;\
app_links->view_post_fade_ = View_Post_Fade;\
app_links->get_user_input_ = Get_User_Input;\
app_links->get_command_input_ = Get_Command_Input;\
app_links->get_mouse_state_ = Get_Mouse_State;\
app_links->start_query_bar_ = Start_Query_Bar;\
app_links->end_query_bar_ = End_Query_Bar;\
app_links->print_message_ = Print_Message;\
app_links->change_theme_ = Change_Theme;\
app_links->change_font_ = Change_Font;\
app_links->buffer_set_font_ = Buffer_Set_Font;\
app_links->buffer_get_font_ = Buffer_Get_Font;\
app_links->set_theme_colors_ = Set_Theme_Colors;\
app_links->get_theme_colors_ = Get_Theme_Colors;\
app_links->directory_get_hot_ = Directory_Get_Hot;\
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
app_links->toggle_fullscreen_ = Toggle_Fullscreen;\
app_links->is_fullscreen_ = Is_Fullscreen;\
app_links->send_exit_signal_ = Send_Exit_Signal;} while(false)
#if defined(ALLOW_DEP_4CODER)
static inline bool32 exec_command(Application_Links *app, Command_ID command_id){return(app->exec_command(app, command_id));}
static inline bool32 exec_system_command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags){return(app->exec_system_command(app, view, buffer, path, path_len, command, command_len, flags));}
static inline void clipboard_post(Application_Links *app, int32_t clipboard_id, char *str, uint32_t len){(app->clipboard_post(app, clipboard_id, str, len));}
static inline uint32_t clipboard_count(Application_Links *app, uint32_t clipboard_id){return(app->clipboard_count(app, clipboard_id));}
static inline uint32_t clipboard_index(Application_Links *app, uint32_t clipboard_id, uint32_t item_index, char *out, uint32_t len){return(app->clipboard_index(app, clipboard_id, item_index, out, len));}
static inline int32_t get_buffer_count(Application_Links *app){return(app->get_buffer_count(app));}
static inline Buffer_Summary get_buffer_first(Application_Links *app, Access_Flag access){return(app->get_buffer_first(app, access));}
static inline void get_buffer_next(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access){(app->get_buffer_next(app, buffer, access));}
static inline Buffer_Summary get_buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access){return(app->get_buffer(app, buffer_id, access));}
static inline Buffer_Summary get_buffer_by_name(Application_Links *app, char *name, int32_t len, Access_Flag access){return(app->get_buffer_by_name(app, name, len, access));}
static inline bool32 buffer_read_range(Application_Links *app, Buffer_Summary *buffer, size_t start, size_t end, char *out){return(app->buffer_read_range(app, buffer, start, end, out));}
static inline bool32 buffer_replace_range(Application_Links *app, Buffer_Summary *buffer, size_t start, size_t end, char *str, size_t len){return(app->buffer_replace_range(app, buffer, start, end, str, len));}
static inline bool32 buffer_compute_cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out){return(app->buffer_compute_cursor(app, buffer, seek, cursor_out));}
static inline bool32 buffer_batch_edit(Application_Links *app, Buffer_Summary *buffer, char *str, size_t str_len, Buffer_Edit *edits, uint32_t edit_count, Buffer_Batch_Edit_Type type){return(app->buffer_batch_edit(app, buffer, str, str_len, edits, edit_count, type));}
static inline Marker_Handle buffer_add_markers(Application_Links *app, Buffer_Summary *buffer, uint32_t marker_count){return(app->buffer_add_markers(app, buffer, marker_count));}
static inline bool32 buffer_set_markers(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker, uint32_t first_marker_index, uint32_t marker_count, Marker *source_markers){return(app->buffer_set_markers(app, buffer, marker, first_marker_index, marker_count, source_markers));}
static inline bool32 buffer_get_markers(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker, uint32_t first_marker_index, uint32_t marker_count, Marker *markers_out){return(app->buffer_get_markers(app, buffer, marker, first_marker_index, marker_count, markers_out));}
static inline bool32 buffer_remove_markers(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker){return(app->buffer_remove_markers(app, buffer, marker));}
static inline bool32 buffer_get_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out){return(app->buffer_get_setting(app, buffer, setting, value_out));}
static inline bool32 buffer_set_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value){return(app->buffer_set_setting(app, buffer, setting, value));}
static inline int32_t buffer_token_count(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_token_count(app, buffer));}
static inline bool32 buffer_read_tokens(Application_Links *app, Buffer_Summary *buffer, size_t start_token, size_t end_token, Cpp_Token *tokens_out){return(app->buffer_read_tokens(app, buffer, start_token, end_token, tokens_out));}
static inline bool32 buffer_get_token_index(Application_Links *app, Buffer_Summary *buffer, size_t pos, Cpp_Get_Token_Result *get_result){return(app->buffer_get_token_index(app, buffer, pos, get_result));}
static inline bool32 begin_buffer_creation(Application_Links *app, Buffer_Creation_Data *data, Buffer_Create_Flag flags){return(app->begin_buffer_creation(app, data, flags));}
static inline bool32 buffer_creation_name(Application_Links *app, Buffer_Creation_Data *data, char *filename, int32_t filename_len, uint32_t flags){return(app->buffer_creation_name(app, data, filename, filename_len, flags));}
static inline Buffer_Summary end_buffer_creation(Application_Links *app, Buffer_Creation_Data *data){return(app->end_buffer_creation(app, data));}
static inline bool32 save_buffer(Application_Links *app, Buffer_Summary *buffer, char *filename, int32_t filename_len, uint32_t flags){return(app->save_buffer(app, buffer, filename, filename_len, flags));}
static inline bool32 kill_buffer(Application_Links *app, Buffer_Identifier buffer, View_ID view_id, Buffer_Kill_Flag flags){return(app->kill_buffer(app, buffer, view_id, flags));}
static inline View_Summary get_view_first(Application_Links *app, Access_Flag access){return(app->get_view_first(app, access));}
static inline void get_view_next(Application_Links *app, View_Summary *view, Access_Flag access){(app->get_view_next(app, view, access));}
static inline View_Summary get_view(Application_Links *app, View_ID view_id, Access_Flag access){return(app->get_view(app, view_id, access));}
static inline View_Summary get_active_view(Application_Links *app, Access_Flag access){return(app->get_active_view(app, access));}
static inline View_Summary open_view(Application_Links *app, View_Summary *view_location, View_Split_Position position){return(app->open_view(app, view_location, position));}
static inline bool32 close_view(Application_Links *app, View_Summary *view){return(app->close_view(app, view));}
static inline bool32 set_active_view(Application_Links *app, View_Summary *view){return(app->set_active_view(app, view));}
static inline bool32 view_get_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out){return(app->view_get_setting(app, view, setting, value_out));}
static inline bool32 view_set_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value){return(app->view_set_setting(app, view, setting, value));}
static inline bool32 view_set_split_proportion(Application_Links *app, View_Summary *view, float t){return(app->view_set_split_proportion(app, view, t));}
static inline bool32 view_compute_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out){return(app->view_compute_cursor(app, view, seek, cursor_out));}
static inline bool32 view_set_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x){return(app->view_set_cursor(app, view, seek, set_preferred_x));}
static inline bool32 view_set_scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll){return(app->view_set_scroll(app, view, scroll));}
static inline bool32 view_set_mark(Application_Links *app, View_Summary *view, Buffer_Seek seek){return(app->view_set_mark(app, view, seek));}
static inline bool32 view_set_highlight(Application_Links *app, View_Summary *view, size_t start, size_t end, bool32 turn_on){return(app->view_set_highlight(app, view, start, end, turn_on));}
static inline bool32 view_set_buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags){return(app->view_set_buffer(app, view, buffer_id, flags));}
static inline bool32 view_post_fade(Application_Links *app, View_Summary *view, float seconds, size_t start, size_t end, int_color color){return(app->view_post_fade(app, view, seconds, start, end, color));}
static inline User_Input get_user_input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type){return(app->get_user_input(app, get_type, abort_type));}
static inline User_Input get_command_input(Application_Links *app){return(app->get_command_input(app));}
static inline Mouse_State get_mouse_state(Application_Links *app){return(app->get_mouse_state(app));}
static inline bool32 start_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){return(app->start_query_bar(app, bar, flags));}
static inline void end_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){(app->end_query_bar(app, bar, flags));}
static inline void print_message(Application_Links *app, char *str, int32_t len){(app->print_message(app, str, len));}
static inline void change_theme(Application_Links *app, char *name, int32_t len){(app->change_theme(app, name, len));}
static inline void change_font(Application_Links *app, char *name, int32_t len, bool32 apply_to_all_files){(app->change_font(app, name, len, apply_to_all_files));}
static inline void buffer_set_font(Application_Links *app, Buffer_Summary *buffer, char *name, int32_t len){(app->buffer_set_font(app, buffer, name, len));}
static inline bool32 buffer_get_font(Application_Links *app, Buffer_Summary *buffer, char *name_out, int32_t name_max){return(app->buffer_get_font(app, buffer, name_out, name_max));}
static inline void set_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->set_theme_colors(app, colors, count));}
static inline void get_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->get_theme_colors(app, colors, count));}
static inline int32_t directory_get_hot(Application_Links *app, char *out, int32_t capacity){return(app->directory_get_hot(app, out, capacity));}
static inline File_List get_file_list(Application_Links *app, char *dir, int32_t len){return(app->get_file_list(app, dir, len));}
static inline void free_file_list(Application_Links *app, File_List list){(app->free_file_list(app, list));}
static inline void set_gui_up_down_keys(Application_Links *app, int16_t up_key, int16_t down_key){(app->set_gui_up_down_keys(app, up_key, down_key));}
static inline void* memory_allocate(Application_Links *app, size_t size){return(app->memory_allocate(app, size));}
static inline bool32 memory_set_protection(Application_Links *app, void *ptr, size_t size, Memory_Protect_Flags flags){return(app->memory_set_protection(app, ptr, size, flags));}
static inline void memory_free(Application_Links *app, void *ptr, size_t size){(app->memory_free(app, ptr, size));}
static inline bool32 file_exists(Application_Links *app, char *filename, int32_t len){return(app->file_exists(app, filename, len));}
static inline bool32 directory_cd(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len){return(app->directory_cd(app, dir, len, capacity, rel_path, rel_len));}
static inline int32_t get_4ed_path(Application_Links *app, char *out, int32_t capacity){return(app->get_4ed_path(app, out, capacity));}
static inline void show_mouse_cursor(Application_Links *app, Mouse_Cursor_Show_Type show){(app->show_mouse_cursor(app, show));}
static inline void toggle_fullscreen(Application_Links *app){(app->toggle_fullscreen(app));}
static inline bool32 is_fullscreen(Application_Links *app){return(app->is_fullscreen(app));}
static inline void send_exit_signal(Application_Links *app){(app->send_exit_signal(app));}
#else
static inline bool32 exec_command(Application_Links *app, Command_ID command_id){return(app->exec_command_(app, command_id));}
static inline bool32 exec_system_command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags){return(app->exec_system_command_(app, view, buffer, path, path_len, command, command_len, flags));}
static inline void clipboard_post(Application_Links *app, int32_t clipboard_id, char *str, uint32_t len){(app->clipboard_post_(app, clipboard_id, str, len));}
static inline uint32_t clipboard_count(Application_Links *app, uint32_t clipboard_id){return(app->clipboard_count_(app, clipboard_id));}
static inline uint32_t clipboard_index(Application_Links *app, uint32_t clipboard_id, uint32_t item_index, char *out, uint32_t len){return(app->clipboard_index_(app, clipboard_id, item_index, out, len));}
static inline int32_t get_buffer_count(Application_Links *app){return(app->get_buffer_count_(app));}
static inline Buffer_Summary get_buffer_first(Application_Links *app, Access_Flag access){return(app->get_buffer_first_(app, access));}
static inline void get_buffer_next(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access){(app->get_buffer_next_(app, buffer, access));}
static inline Buffer_Summary get_buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access){return(app->get_buffer_(app, buffer_id, access));}
static inline Buffer_Summary get_buffer_by_name(Application_Links *app, char *name, int32_t len, Access_Flag access){return(app->get_buffer_by_name_(app, name, len, access));}
static inline bool32 buffer_read_range(Application_Links *app, Buffer_Summary *buffer, size_t start, size_t end, char *out){return(app->buffer_read_range_(app, buffer, start, end, out));}
static inline bool32 buffer_replace_range(Application_Links *app, Buffer_Summary *buffer, size_t start, size_t end, char *str, size_t len){return(app->buffer_replace_range_(app, buffer, start, end, str, len));}
static inline bool32 buffer_compute_cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out){return(app->buffer_compute_cursor_(app, buffer, seek, cursor_out));}
static inline bool32 buffer_batch_edit(Application_Links *app, Buffer_Summary *buffer, char *str, size_t str_len, Buffer_Edit *edits, uint32_t edit_count, Buffer_Batch_Edit_Type type){return(app->buffer_batch_edit_(app, buffer, str, str_len, edits, edit_count, type));}
static inline Marker_Handle buffer_add_markers(Application_Links *app, Buffer_Summary *buffer, uint32_t marker_count){return(app->buffer_add_markers_(app, buffer, marker_count));}
static inline bool32 buffer_set_markers(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker, uint32_t first_marker_index, uint32_t marker_count, Marker *source_markers){return(app->buffer_set_markers_(app, buffer, marker, first_marker_index, marker_count, source_markers));}
static inline bool32 buffer_get_markers(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker, uint32_t first_marker_index, uint32_t marker_count, Marker *markers_out){return(app->buffer_get_markers_(app, buffer, marker, first_marker_index, marker_count, markers_out));}
static inline bool32 buffer_remove_markers(Application_Links *app, Buffer_Summary *buffer, Marker_Handle marker){return(app->buffer_remove_markers_(app, buffer, marker));}
static inline bool32 buffer_get_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out){return(app->buffer_get_setting_(app, buffer, setting, value_out));}
static inline bool32 buffer_set_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value){return(app->buffer_set_setting_(app, buffer, setting, value));}
static inline int32_t buffer_token_count(Application_Links *app, Buffer_Summary *buffer){return(app->buffer_token_count_(app, buffer));}
static inline bool32 buffer_read_tokens(Application_Links *app, Buffer_Summary *buffer, size_t start_token, size_t end_token, Cpp_Token *tokens_out){return(app->buffer_read_tokens_(app, buffer, start_token, end_token, tokens_out));}
static inline bool32 buffer_get_token_index(Application_Links *app, Buffer_Summary *buffer, size_t pos, Cpp_Get_Token_Result *get_result){return(app->buffer_get_token_index_(app, buffer, pos, get_result));}
static inline bool32 begin_buffer_creation(Application_Links *app, Buffer_Creation_Data *data, Buffer_Create_Flag flags){return(app->begin_buffer_creation_(app, data, flags));}
static inline bool32 buffer_creation_name(Application_Links *app, Buffer_Creation_Data *data, char *filename, int32_t filename_len, uint32_t flags){return(app->buffer_creation_name_(app, data, filename, filename_len, flags));}
static inline Buffer_Summary end_buffer_creation(Application_Links *app, Buffer_Creation_Data *data){return(app->end_buffer_creation_(app, data));}
static inline bool32 save_buffer(Application_Links *app, Buffer_Summary *buffer, char *filename, int32_t filename_len, uint32_t flags){return(app->save_buffer_(app, buffer, filename, filename_len, flags));}
static inline bool32 kill_buffer(Application_Links *app, Buffer_Identifier buffer, View_ID view_id, Buffer_Kill_Flag flags){return(app->kill_buffer_(app, buffer, view_id, flags));}
static inline View_Summary get_view_first(Application_Links *app, Access_Flag access){return(app->get_view_first_(app, access));}
static inline void get_view_next(Application_Links *app, View_Summary *view, Access_Flag access){(app->get_view_next_(app, view, access));}
static inline View_Summary get_view(Application_Links *app, View_ID view_id, Access_Flag access){return(app->get_view_(app, view_id, access));}
static inline View_Summary get_active_view(Application_Links *app, Access_Flag access){return(app->get_active_view_(app, access));}
static inline View_Summary open_view(Application_Links *app, View_Summary *view_location, View_Split_Position position){return(app->open_view_(app, view_location, position));}
static inline bool32 close_view(Application_Links *app, View_Summary *view){return(app->close_view_(app, view));}
static inline bool32 set_active_view(Application_Links *app, View_Summary *view){return(app->set_active_view_(app, view));}
static inline bool32 view_get_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out){return(app->view_get_setting_(app, view, setting, value_out));}
static inline bool32 view_set_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value){return(app->view_set_setting_(app, view, setting, value));}
static inline bool32 view_set_split_proportion(Application_Links *app, View_Summary *view, float t){return(app->view_set_split_proportion_(app, view, t));}
static inline bool32 view_compute_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out){return(app->view_compute_cursor_(app, view, seek, cursor_out));}
static inline bool32 view_set_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x){return(app->view_set_cursor_(app, view, seek, set_preferred_x));}
static inline bool32 view_set_scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll){return(app->view_set_scroll_(app, view, scroll));}
static inline bool32 view_set_mark(Application_Links *app, View_Summary *view, Buffer_Seek seek){return(app->view_set_mark_(app, view, seek));}
static inline bool32 view_set_highlight(Application_Links *app, View_Summary *view, size_t start, size_t end, bool32 turn_on){return(app->view_set_highlight_(app, view, start, end, turn_on));}
static inline bool32 view_set_buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags){return(app->view_set_buffer_(app, view, buffer_id, flags));}
static inline bool32 view_post_fade(Application_Links *app, View_Summary *view, float seconds, size_t start, size_t end, int_color color){return(app->view_post_fade_(app, view, seconds, start, end, color));}
static inline User_Input get_user_input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type){return(app->get_user_input_(app, get_type, abort_type));}
static inline User_Input get_command_input(Application_Links *app){return(app->get_command_input_(app));}
static inline Mouse_State get_mouse_state(Application_Links *app){return(app->get_mouse_state_(app));}
static inline bool32 start_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){return(app->start_query_bar_(app, bar, flags));}
static inline void end_query_bar(Application_Links *app, Query_Bar *bar, uint32_t flags){(app->end_query_bar_(app, bar, flags));}
static inline void print_message(Application_Links *app, char *str, int32_t len){(app->print_message_(app, str, len));}
static inline void change_theme(Application_Links *app, char *name, int32_t len){(app->change_theme_(app, name, len));}
static inline void change_font(Application_Links *app, char *name, int32_t len, bool32 apply_to_all_files){(app->change_font_(app, name, len, apply_to_all_files));}
static inline void buffer_set_font(Application_Links *app, Buffer_Summary *buffer, char *name, int32_t len){(app->buffer_set_font_(app, buffer, name, len));}
static inline bool32 buffer_get_font(Application_Links *app, Buffer_Summary *buffer, char *name_out, int32_t name_max){return(app->buffer_get_font_(app, buffer, name_out, name_max));}
static inline void set_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->set_theme_colors_(app, colors, count));}
static inline void get_theme_colors(Application_Links *app, Theme_Color *colors, int32_t count){(app->get_theme_colors_(app, colors, count));}
static inline int32_t directory_get_hot(Application_Links *app, char *out, int32_t capacity){return(app->directory_get_hot_(app, out, capacity));}
static inline File_List get_file_list(Application_Links *app, char *dir, int32_t len){return(app->get_file_list_(app, dir, len));}
static inline void free_file_list(Application_Links *app, File_List list){(app->free_file_list_(app, list));}
static inline void set_gui_up_down_keys(Application_Links *app, int16_t up_key, int16_t down_key){(app->set_gui_up_down_keys_(app, up_key, down_key));}
static inline void* memory_allocate(Application_Links *app, size_t size){return(app->memory_allocate_(app, size));}
static inline bool32 memory_set_protection(Application_Links *app, void *ptr, size_t size, Memory_Protect_Flags flags){return(app->memory_set_protection_(app, ptr, size, flags));}
static inline void memory_free(Application_Links *app, void *ptr, size_t size){(app->memory_free_(app, ptr, size));}
static inline bool32 file_exists(Application_Links *app, char *filename, int32_t len){return(app->file_exists_(app, filename, len));}
static inline bool32 directory_cd(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len){return(app->directory_cd_(app, dir, len, capacity, rel_path, rel_len));}
static inline int32_t get_4ed_path(Application_Links *app, char *out, int32_t capacity){return(app->get_4ed_path_(app, out, capacity));}
static inline void show_mouse_cursor(Application_Links *app, Mouse_Cursor_Show_Type show){(app->show_mouse_cursor_(app, show));}
static inline void toggle_fullscreen(Application_Links *app){(app->toggle_fullscreen_(app));}
static inline bool32 is_fullscreen(Application_Links *app){return(app->is_fullscreen_(app));}
static inline void send_exit_signal(Application_Links *app){(app->send_exit_signal_(app));}
#endif
