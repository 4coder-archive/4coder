#define EXEC_COMMAND_SIG(n) bool32 n(Application_Links *app, Command_ID command_id)
#define EXEC_SYSTEM_COMMAND_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Identifier buffer, char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags)
#define CLIPBOARD_POST_SIG(n) void n(Application_Links *app, int32_t clipboard_id, char *str, int32_t len)
#define CLIPBOARD_COUNT_SIG(n) int32_t n(Application_Links *app, int32_t clipboard_id)
#define CLIPBOARD_INDEX_SIG(n) int32_t n(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len)
#define GET_BUFFER_COUNT_SIG(n) int32_t n(Application_Links *app)
#define GET_BUFFER_FIRST_SIG(n) Buffer_Summary n(Application_Links *app, Access_Flag access)
#define GET_BUFFER_NEXT_SIG(n) void n(Application_Links *app, Buffer_Summary *buffer, Access_Flag  access)
#define GET_BUFFER_SIG(n) Buffer_Summary n(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
#define GET_BUFFER_BY_NAME_SIG(n) Buffer_Summary n(Application_Links *app, char *name, int32_t len, Access_Flag access)
#define BUFFER_READ_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *out)
#define BUFFER_REPLACE_RANGE_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, char *str, int32_t len)
#define BUFFER_COMPUTE_CURSOR_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out)
#define BUFFER_BATCH_EDIT_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type)
#define BUFFER_SET_SETTING_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value)
#define BUFFER_TOKEN_COUNT_SIG(n) int32_t n(Application_Links *app, Buffer_Summary *buffer)
#define BUFFER_READ_TOKENS_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, int32_t start_token, int32_t end_token, Cpp_Token *tokens_out)
#define CREATE_BUFFER_SIG(n) Buffer_Summary n(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags)
#define SAVE_BUFFER_SIG(n) bool32 n(Application_Links *app, Buffer_Summary *buffer, char *filename, int32_t filename_len, uint32_t flags)
#define KILL_BUFFER_SIG(n) bool32 n(Application_Links *app, Buffer_Identifier buffer, View_ID view_id, Buffer_Kill_Flag flags)
#define GET_VIEW_FIRST_SIG(n) View_Summary n(Application_Links *app, Access_Flag access)
#define GET_VIEW_NEXT_SIG(n) void n(Application_Links *app, View_Summary *view, Access_Flag access)
#define GET_VIEW_SIG(n) View_Summary n(Application_Links *app, View_ID view_id, Access_Flag access)
#define GET_ACTIVE_VIEW_SIG(n) View_Summary n(Application_Links *app, Access_Flag access)
#define OPEN_VIEW_SIG(n) View_Summary n(Application_Links *app, View_Summary *view_location, View_Split_Position position)
#define CLOSE_VIEW_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define SET_ACTIVE_VIEW_SIG(n) bool32 n(Application_Links *app, View_Summary *view)
#define VIEW_SET_SETTING_SIG(n) bool32 n(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value)
#define VIEW_SET_SPLIT_PROPORTION_SIG(n) bool32 n(Application_Links *app, View_Summary *view, float t)
#define VIEW_COMPUTE_CURSOR_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out)
#define VIEW_SET_CURSOR_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x)
#define VIEW_SET_SCROLL_SIG(n) bool32 n(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll)
#define VIEW_SET_MARK_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_Seek seek)
#define VIEW_SET_HIGHLIGHT_SIG(n) bool32 n(Application_Links *app, View_Summary *view, int32_t start, int32_t end, bool32 turn_on)
#define VIEW_SET_BUFFER_SIG(n) bool32 n(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags)
#define VIEW_POST_FADE_SIG(n) bool32 n(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color)
#define GET_USER_INPUT_SIG(n) User_Input n(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
#define GET_COMMAND_INPUT_SIG(n) User_Input n(Application_Links *app)
#define GET_MOUSE_STATE_SIG(n) Mouse_State n(Application_Links *app)
#define START_QUERY_BAR_SIG(n) bool32 n(Application_Links *app, Query_Bar *bar, uint32_t flags)
#define END_QUERY_BAR_SIG(n) void n(Application_Links *app, Query_Bar *bar, uint32_t flags)
#define PRINT_MESSAGE_SIG(n) void n(Application_Links *app, char *str, int32_t len)
#define CHANGE_THEME_SIG(n) void n(Application_Links *app, char *name, int32_t len)
#define CHANGE_FONT_SIG(n) void n(Application_Links *app, char *name, int32_t len, bool32 apply_to_all_files)
#define BUFFER_SET_FONT_SIG(n) void n(Application_Links *app, Buffer_Summary *buffer, char *name, int32_t len)
#define SET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, int32_t count)
#define GET_THEME_COLORS_SIG(n) void n(Application_Links *app, Theme_Color *colors, int32_t count)
#define DIRECTORY_GET_HOT_SIG(n) int32_t n(Application_Links *app, char *out, int32_t capacity)
#define GET_FILE_LIST_SIG(n) File_List n(Application_Links *app, char *dir, int32_t len)
#define FREE_FILE_LIST_SIG(n) void n(Application_Links *app, File_List list)
#define MEMORY_ALLOCATE_SIG(n) void* n(Application_Links *app, int32_t size)
#define MEMORY_SET_PROTECTION_SIG(n) bool32 n(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags)
#define MEMORY_FREE_SIG(n) void n(Application_Links *app, void *ptr, int32_t size)
#define FILE_EXISTS_SIG(n) bool32 n(Application_Links *app, char *filename, int32_t len)
#define DIRECTORY_CD_SIG(n) bool32 n(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len)
#define GET_4ED_PATH_SIG(n) bool32 n(Application_Links *app, char *out, int32_t capacity)
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
typedef BUFFER_SET_SETTING_SIG(Buffer_Set_Setting_Function);
typedef BUFFER_TOKEN_COUNT_SIG(Buffer_Token_Count_Function);
typedef BUFFER_READ_TOKENS_SIG(Buffer_Read_Tokens_Function);
typedef CREATE_BUFFER_SIG(Create_Buffer_Function);
typedef SAVE_BUFFER_SIG(Save_Buffer_Function);
typedef KILL_BUFFER_SIG(Kill_Buffer_Function);
typedef GET_VIEW_FIRST_SIG(Get_View_First_Function);
typedef GET_VIEW_NEXT_SIG(Get_View_Next_Function);
typedef GET_VIEW_SIG(Get_View_Function);
typedef GET_ACTIVE_VIEW_SIG(Get_Active_View_Function);
typedef OPEN_VIEW_SIG(Open_View_Function);
typedef CLOSE_VIEW_SIG(Close_View_Function);
typedef SET_ACTIVE_VIEW_SIG(Set_Active_View_Function);
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
typedef SET_THEME_COLORS_SIG(Set_Theme_Colors_Function);
typedef GET_THEME_COLORS_SIG(Get_Theme_Colors_Function);
typedef DIRECTORY_GET_HOT_SIG(Directory_Get_Hot_Function);
typedef GET_FILE_LIST_SIG(Get_File_List_Function);
typedef FREE_FILE_LIST_SIG(Free_File_List_Function);
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
Buffer_Set_Setting_Function *buffer_set_setting;
Buffer_Token_Count_Function *buffer_token_count;
Buffer_Read_Tokens_Function *buffer_read_tokens;
Create_Buffer_Function *create_buffer;
Save_Buffer_Function *save_buffer;
Kill_Buffer_Function *kill_buffer;
Get_View_First_Function *get_view_first;
Get_View_Next_Function *get_view_next;
Get_View_Function *get_view;
Get_Active_View_Function *get_active_view;
Open_View_Function *open_view;
Close_View_Function *close_view;
Set_Active_View_Function *set_active_view;
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
Set_Theme_Colors_Function *set_theme_colors;
Get_Theme_Colors_Function *get_theme_colors;
Directory_Get_Hot_Function *directory_get_hot;
Get_File_List_Function *get_file_list;
Free_File_List_Function *free_file_list;
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
void *memory;
int32_t memory_size;
void *cmd_context;
void *system_links;
void *current_coroutine;
int32_t type_coroutine;
};
#define FillAppLinksAPI(app_links) do{\
app_links->exec_command = Exec_Command;\
app_links->exec_system_command = Exec_System_Command;\
app_links->clipboard_post = Clipboard_Post;\
app_links->clipboard_count = Clipboard_Count;\
app_links->clipboard_index = Clipboard_Index;\
app_links->get_buffer_count = Get_Buffer_Count;\
app_links->get_buffer_first = Get_Buffer_First;\
app_links->get_buffer_next = Get_Buffer_Next;\
app_links->get_buffer = Get_Buffer;\
app_links->get_buffer_by_name = Get_Buffer_By_Name;\
app_links->buffer_read_range = Buffer_Read_Range;\
app_links->buffer_replace_range = Buffer_Replace_Range;\
app_links->buffer_compute_cursor = Buffer_Compute_Cursor;\
app_links->buffer_batch_edit = Buffer_Batch_Edit;\
app_links->buffer_set_setting = Buffer_Set_Setting;\
app_links->buffer_token_count = Buffer_Token_Count;\
app_links->buffer_read_tokens = Buffer_Read_Tokens;\
app_links->create_buffer = Create_Buffer;\
app_links->save_buffer = Save_Buffer;\
app_links->kill_buffer = Kill_Buffer;\
app_links->get_view_first = Get_View_First;\
app_links->get_view_next = Get_View_Next;\
app_links->get_view = Get_View;\
app_links->get_active_view = Get_Active_View;\
app_links->open_view = Open_View;\
app_links->close_view = Close_View;\
app_links->set_active_view = Set_Active_View;\
app_links->view_set_setting = View_Set_Setting;\
app_links->view_set_split_proportion = View_Set_Split_Proportion;\
app_links->view_compute_cursor = View_Compute_Cursor;\
app_links->view_set_cursor = View_Set_Cursor;\
app_links->view_set_scroll = View_Set_Scroll;\
app_links->view_set_mark = View_Set_Mark;\
app_links->view_set_highlight = View_Set_Highlight;\
app_links->view_set_buffer = View_Set_Buffer;\
app_links->view_post_fade = View_Post_Fade;\
app_links->get_user_input = Get_User_Input;\
app_links->get_command_input = Get_Command_Input;\
app_links->get_mouse_state = Get_Mouse_State;\
app_links->start_query_bar = Start_Query_Bar;\
app_links->end_query_bar = End_Query_Bar;\
app_links->print_message = Print_Message;\
app_links->change_theme = Change_Theme;\
app_links->change_font = Change_Font;\
app_links->buffer_set_font = Buffer_Set_Font;\
app_links->set_theme_colors = Set_Theme_Colors;\
app_links->get_theme_colors = Get_Theme_Colors;\
app_links->directory_get_hot = Directory_Get_Hot;\
app_links->get_file_list = Get_File_List;\
app_links->free_file_list = Free_File_List;\
app_links->memory_allocate = Memory_Allocate;\
app_links->memory_set_protection = Memory_Set_Protection;\
app_links->memory_free = Memory_Free;\
app_links->file_exists = File_Exists;\
app_links->directory_cd = Directory_CD;\
app_links->get_4ed_path = Get_4ed_Path;\
app_links->show_mouse_cursor = Show_Mouse_Cursor;\
app_links->toggle_fullscreen = Toggle_Fullscreen;\
app_links->is_fullscreen = Is_Fullscreen;\
app_links->send_exit_signal = Send_Exit_Signal;} while(false)
