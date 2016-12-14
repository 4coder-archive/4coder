#define MEMORY_ALLOCATE_SIG(n) void* n(Application_Links *app, int32_t size)
#define MEMORY_SET_PROTECTION_SIG(n) bool32 n(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags)
#define MEMORY_FREE_SIG(n) void n(Application_Links *app, void *ptr, int32_t size)
#define FILE_EXISTS_SIG(n) bool32 n(Application_Links *app, char *filename, int32_t len)
#define DIRECTORY_CD_SIG(n) bool32 n(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len)
#define GET_4ED_PATH_SIG(n) int32_t n(Application_Links *app, char *out, int32_t capacity)
#define SHOW_MOUSE_CURSOR_SIG(n) void n(Application_Links *app, Mouse_Cursor_Show_Type show)
#define TOGGLE_FULLSCREEN_SIG(n) void n(Application_Links *app)
#define IS_FULLSCREEN_SIG(n) bool32 n(Application_Links *app)
#define SEND_EXIT_SIGNAL_SIG(n) void n(Application_Links *app)
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
