
// Command exectuion
void Exec_Command(Application_Links *app, int command_id);
int Exec_System_Command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer, char *path, int path_len, char *command, int command_len, unsigned int flags);

// File system navigation
int Directory_Get_Hot(Application_Links *app, char *out, int capacity);
int Get_4ed_Path(Application_Links *app, char *out, int capacity);
int File_Exists(Application_Links *app, char *filename, int len);
int Directory_CD(Application_Links *app, char *dir, int *len, int capacity, char *rel_path, int rel_len);
File_List Get_File_List(Application_Links *app, char *dir, int len);
void Free_File_List(Application_Links *app, File_List list);

// Clipboard

// TODO(allen): extend this API out a little bit to allow for future expansion.
void Clipboard_Post(Application_Links *app, char *str, int len);
int  Clipboard_Count(Application_Links *app);
int  Clipboard_Index(Application_Links *app, int index, char *out, int len);

// Direct buffer manipulation
Buffer_Summary Get_Buffer_First(Application_Links *app, unsigned int access);
void Get_Buffer_Next(Application_Links *app, Buffer_Summary *buffer, unsigned int access);

Buffer_Summary Get_Buffer(Application_Links *app, int buffer_id, unsigned int access);
Buffer_Summary Get_Buffer_By_Name(Application_Links *app, char *name, int len, unsigned int access);

int Buffer_Seek(Application_Links *app, Buffer_Summary *buffer, int start_pos, int seek_forward, unsigned int flags);
int Buffer_Read_Range(Application_Links *app, Buffer_Summary *buffer, int start, int end, char *out);

int Buffer_Replace_Range(Application_Links *app, Buffer_Summary *buffer, int start, int end, char *str, int len);
int Buffer_Set_Setting(Application_Links *app, Buffer_Summary *buffer, int setting, int value);
int Buffer_Auto_Indent(Application_Links *app, Buffer_Summary *buffer, int start, int end, int tab_width, unsigned int flags);

Buffer_Summary Create_Buffer(Application_Links *app, char *filename, int filename_len, unsigned int flags);
int Save_Buffer(Application_Links *app, Buffer_Summary *buffer, char *filename, int filename_len, unsigned int flags);
int Kill_Buffer(Application_Links *app, Buffer_Identifier buffer, int view_id, unsigned int flags);

// View manipulation
View_Summary Get_View_First(Application_Links *app, unsigned int access);
void Get_View_Next(Application_Links *app, View_Summary *view, unsigned int access);

View_Summary Get_View(Application_Links *app, int view_id, unsigned int access);
View_Summary Get_Active_View(Application_Links *app, unsigned int access);

Full_Cursor View_Compute_Cursor (Application_Links *app, View_Summary *view, Buffer_Seek seek);
int         View_Set_Cursor     (Application_Links *app, View_Summary *view, Buffer_Seek seek, int set_preferred_x);
int         View_Set_Mark       (Application_Links *app, View_Summary *view, Buffer_Seek seek);
int         View_Set_Highlight  (Application_Links *app, View_Summary *view, int start, int end, int turn_on);
int         View_Set_Buffer     (Application_Links *app, View_Summary *view, int buffer_id);
// TODO(allen): Switch from ticks to seconds.
int         View_Post_Fade      (Application_Links *app, View_Summary *view, int ticks, int start, int end, unsigned int color);

// TODO(allen):
// Get rid of this temporary hack ass soon ass possible.
void View_Set_Paste_Rewrite_(Application_Links *app, View_Summary *view);
int View_Get_Paste_Rewrite_(Application_Links *app, View_Summary *view);

// Directly get user input
User_Input Get_User_Input(Application_Links *app, unsigned int get_type, unsigned int abort_type);
User_Input Get_Command_Input(Application_Links *app);
Event_Message Get_Event_Message(Application_Links *app);
Mouse_State Get_Mouse_State(Application_Links *app);

// Queries and information display
int Start_Query_Bar(Application_Links *app, Query_Bar *bar, unsigned int flags);
void End_Query_Bar(Application_Links *app, Query_Bar *bar, unsigned int flags);
void Print_Message(Application_Links *app, char *string, int len);
//GUI_Functions* Get_GUI_Functions(Application_Links *app);
//GUI* Get_GUI(Application_Links *app, int view_id);

// Color settings
void Change_Theme(Application_Links *app, char *name, int len);
void Change_Font(Application_Links *app, char *name, int len);
void Set_Theme_Colors(Application_Links *app, Theme_Color *colors, int count);
void Get_Theme_Colors(Application_Links *app, Theme_Color *colors, int count);

