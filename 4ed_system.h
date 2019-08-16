/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2014
 *
 * System functions for project codename "4ed"
 *
 */

// TOP

// TODO(allen): rewrite using new arenas and strings!!!!! rewrite rewrite rewrite rewrite rewrite

#if !defined(FCODER_SYSTEM_INTERFACE_H)
#define FCODER_SYSTEM_INTERFACE_H

// types
struct Plat_Handle{
    u32 d[4];
};

// files
#define Sys_Get_Canonical_Sig(n) String_Const_u8 n(Arena *arena, String_Const_u8 name)
typedef Sys_Get_Canonical_Sig(System_Get_Canonical);

#define Sys_Get_File_List_Sig(name) File_List name(Arena *arena, String_Const_u8 directory)
typedef Sys_Get_File_List_Sig(System_Get_File_List);

// file load/save
#define Sys_Quick_File_Attributes_Sig(name) File_Attributes name(String_Const_u8 file_name)
typedef Sys_Quick_File_Attributes_Sig(System_Quick_File_Attributes);

#define Sys_Load_Handle_Sig(name) b32 name(char *filename, Plat_Handle *handle_out)
typedef Sys_Load_Handle_Sig(System_Load_Handle);

#define Sys_Load_Attributes_Sig(name) File_Attributes name(Plat_Handle handle)
typedef Sys_Load_Attributes_Sig(System_Load_Attributes);

#define Sys_Load_File_Sig(name) b32 name(Plat_Handle handle, char *buffer, u32 size)
typedef Sys_Load_File_Sig(System_Load_File);

#define Sys_Load_Close_Sig(name) b32 name(Plat_Handle handle)
typedef Sys_Load_Close_Sig(System_Load_Close);

#define Sys_Save_File_Sig(name) File_Attributes name(char *filename, char *buffer, u32 size)
typedef Sys_Save_File_Sig(System_Save_File);

// time
#define Sys_Now_Time_Sig(name) u64 name()
typedef Sys_Now_Time_Sig(System_Now_Time);

#define Sys_Wake_Up_Timer_Create_Sig(name) Plat_Handle name()
typedef Sys_Wake_Up_Timer_Create_Sig(System_Wake_Up_Timer_Create);

#define Sys_Wake_Up_Timer_Release_Sig(name) void name(Plat_Handle handle)
typedef Sys_Wake_Up_Timer_Release_Sig(System_Wake_Up_Timer_Release);

#define Sys_Wake_Up_Timer_Set_Sig(name) void name(Plat_Handle handle, u32 time_milliseconds)
typedef Sys_Wake_Up_Timer_Set_Sig(System_Wake_Up_Timer_Set);

#define Sys_Wake_Up_Timer_Check_Sig(name) u64 name(Plat_Handle handle)
typedef Sys_Wake_Up_Timer_Check_Sig(System_Wake_Up_Timer_Check);

#define Sys_Signal_Step_Sig(name) void name(u32 code)
typedef Sys_Signal_Step_Sig(System_Signal_Step);

#define Sys_Sleep_Sig(name) void name(u64 microseconds)
typedef Sys_Sleep_Sig(System_Sleep);

// clipboard
#define Sys_Post_Clipboard_Sig(name) void name(String_Const_u8 str)
typedef Sys_Post_Clipboard_Sig(System_Post_Clipboard);

// cli
struct CLI_Handles{
    Plat_Handle proc;
    Plat_Handle out_read;
    Plat_Handle out_write;
    Plat_Handle in_read;
    Plat_Handle in_write;
    u32 scratch_space[4];
    i32 exit;
};

#define Sys_CLI_Call_Sig(n, path, script, cli_out) b32 n(char *path, char *script, CLI_Handles *cli_out)
typedef Sys_CLI_Call_Sig(System_CLI_Call, path, script, cli_out);

#define Sys_CLI_Begin_Update_Sig(name) void name(CLI_Handles *cli)
typedef Sys_CLI_Begin_Update_Sig(System_CLI_Begin_Update);

#define Sys_CLI_Update_Step_Sig(name) b32 name(CLI_Handles *cli, char *dest, u32 max, u32 *amount)
typedef Sys_CLI_Update_Step_Sig(System_CLI_Update_Step);

#define Sys_CLI_End_Update_Sig(name) b32 name(CLI_Handles *cli)
typedef Sys_CLI_End_Update_Sig(System_CLI_End_Update);

//

#define Sys_Open_Color_Picker_Sig(name) void name(Color_Picker *picker)
typedef Sys_Open_Color_Picker_Sig(System_Open_Color_Picker);

// thread
typedef Plat_Handle System_Thread;
typedef Plat_Handle System_Mutex;
typedef Plat_Handle System_Condition_Variable;
typedef void Thread_Function(void *ptr);

#define Sys_Thread_Launch_Sig(name) System_Thread name(Thread_Function *proc, void *ptr)
typedef Sys_Thread_Launch_Sig(System_Thread_Launch);

#define Sys_Thread_Join_Sig(name) void name(System_Thread thread)
typedef Sys_Thread_Join_Sig(System_Thread_Join);

#define Sys_Thread_Free_Sig(name) void name(System_Thread thread)
typedef Sys_Thread_Free_Sig(System_Thread_Free);

#define Sys_Thread_Get_ID_Sig(name) i32 name(void)
typedef Sys_Thread_Get_ID_Sig(System_Thread_Get_ID);

#define Sys_Mutex_Make_Sig(name) System_Mutex name(void)
typedef Sys_Mutex_Make_Sig(System_Mutex_Make);

#define Sys_Mutex_Acquire_Sig(name) void name(System_Mutex mutex)
typedef Sys_Mutex_Acquire_Sig(System_Mutex_Acquire);

#define Sys_Mutex_Release_Sig(name) void name(System_Mutex mutex)
typedef Sys_Mutex_Release_Sig(System_Mutex_Release);

#define Sys_Mutex_Free_Sig(name) void name(System_Mutex mutex)
typedef Sys_Mutex_Free_Sig(System_Mutex_Free);

#define Sys_Condition_Variable_Make_Sig(name) System_Condition_Variable name(void)
typedef Sys_Condition_Variable_Make_Sig(System_Condition_Variable_Make);

#define Sys_Condition_Variable_Wait_Sig(name) void name(System_Condition_Variable cv, System_Mutex mutex)
typedef Sys_Condition_Variable_Wait_Sig(System_Condition_Variable_Wait);

#define Sys_Condition_Variable_Signal_Sig(name) void name(System_Condition_Variable cv)
typedef Sys_Condition_Variable_Signal_Sig(System_Condition_Variable_Signal);

#define Sys_Condition_Variable_Free_Sig(name) void name(System_Condition_Variable cv)
typedef Sys_Condition_Variable_Free_Sig(System_Condition_Variable_Free);

// memory
#define Sys_Memory_Allocate_Sig(name) void* name(umem size)
typedef Sys_Memory_Allocate_Sig(System_Memory_Allocate);

#define Sys_Memory_Set_Protection_Sig(name) b32 name(void *ptr, umem size, u32 flags)
typedef Sys_Memory_Set_Protection_Sig(System_Memory_Set_Protection);

#define Sys_Memory_Free_Sig(name) void name(void *ptr, umem size)
typedef Sys_Memory_Free_Sig(System_Memory_Free);

// file system
#define Sys_Get_Current_Path_Sig(name) String_Const_u8 name(Arena *arena)
typedef Sys_Get_Current_Path_Sig(System_Get_Current_Path);

#define Sys_Get_4ed_Path_Sig(name) String_Const_u8 name(Arena *arena)
typedef Sys_Get_4ed_Path_Sig(System_Get_4ed_Path);

// behavior and appearance options
#define Sys_Show_Mouse_Cursor_Sig(name) void name(i32 show)
typedef Sys_Show_Mouse_Cursor_Sig(System_Show_Mouse_Cursor);

#define Sys_Set_Fullscreen_Sig(name) b32 name(b32 full_screen)
typedef Sys_Set_Fullscreen_Sig(System_Set_Fullscreen);

#define Sys_Is_Fullscreen_Sig(name) b32 name()
typedef Sys_Is_Fullscreen_Sig(System_Is_Fullscreen);

struct System_Functions{
    Font_Make_Face_Function *font_make_face;
    Graphics_Get_Texture_Function *get_texture;
    Graphics_Fill_Texture_Function *fill_texture;
    
    // files
    System_Get_Canonical         *get_canonical;
    System_Get_File_List         *get_file_list;
    System_Quick_File_Attributes *quick_file_attributes;
    System_Load_Handle           *load_handle;
    System_Load_Attributes       *load_attributes;
    System_Load_File             *load_file;
    System_Load_Close            *load_close;
    System_Save_File             *save_file;
    
    // time
    System_Now_Time *now_time;
    System_Wake_Up_Timer_Create *wake_up_timer_create;
    System_Wake_Up_Timer_Release *wake_up_timer_release;
    System_Wake_Up_Timer_Set *wake_up_timer_set;
    System_Signal_Step *signal_step;
    System_Sleep *sleep;
    
    // clipboard
    System_Post_Clipboard *post_clipboard;
    
    // cli
    System_CLI_Call         *cli_call;
    System_CLI_Begin_Update *cli_begin_update;
    System_CLI_Update_Step  *cli_update_step;
    System_CLI_End_Update   *cli_end_update;
    
    // TODO(allen): 
    System_Open_Color_Picker *open_color_picker;
    
    // threads
    System_Thread_Launch *thread_launch;
    System_Thread_Join *thread_join;
    System_Thread_Free *thread_free;
    System_Thread_Get_ID *thread_get_id;
    System_Mutex_Make *mutex_make;
    System_Mutex_Acquire *mutex_acquire;
    System_Mutex_Release *mutex_release;
    System_Mutex_Free *mutex_free;
    System_Condition_Variable_Make *condition_variable_make;
    System_Condition_Variable_Wait *condition_variable_wait;
    System_Condition_Variable_Signal *condition_variable_signal;
    System_Condition_Variable_Free *condition_variable_free;
    
    // custom
    System_Memory_Allocate        *memory_allocate;
    System_Memory_Set_Protection  *memory_set_protection;
    System_Memory_Free            *memory_free;
    
    System_Get_Current_Path       *get_current_path;
    System_Get_4ed_Path           *get_4ed_path;
    
    System_Show_Mouse_Cursor      *show_mouse_cursor;
    System_Set_Fullscreen         *set_fullscreen;
    System_Is_Fullscreen          *is_fullscreen;
};

#endif

// BOTTOM

