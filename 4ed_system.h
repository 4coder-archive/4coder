/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2014
 *
 * System functions for project codename "4ed"
 *
 */

// TOP

#if !defined(FCODER_SYSTEM_INTERFACE_H)
#define FCODER_SYSTEM_INTERFACE_H

// types
struct Plat_Handle{
    u32 d[4];
};

internal b32
handle_equal(Plat_Handle a, Plat_Handle b){
    b32 result = (memcmp(&a, &b, sizeof(a)) == 0);
    return(result);
}

// files
#define Sys_Set_File_List_Sig(name) void name(File_List *file_list, char *directory, char *canon_directory_out, u32 *canon_directory_size_out, u32 canon_directory_max)
typedef Sys_Set_File_List_Sig(System_Set_File_List);

#define Sys_Get_Canonical_Sig(name) u32 name(char *filename, u32 len, char *buffer, u32 max)
typedef Sys_Get_Canonical_Sig(System_Get_Canonical);

// file load/save
#define Sys_Quick_File_Attributes_Sig(name) File_Attributes name(String file_name)
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

// file changes
#define Sys_Add_Listener_Sig(name) b32 name(char *filename)
typedef Sys_Add_Listener_Sig(System_Add_Listener);

#define Sys_Remove_Listener_Sig(name) b32 name(char *filename)
typedef Sys_Remove_Listener_Sig(System_Remove_Listener);

#define Sys_Get_File_Change_Sig(name) b32 name(char *buffer, i32 max, b32 *mem_too_small, i32 *required_size)
typedef Sys_Get_File_Change_Sig(System_Get_File_Change);

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

// clipboard
#define Sys_Post_Clipboard_Sig(name) void name(String str)
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

// coroutine

struct Coroutine_Head{
    void *in;
    void *out;
};

#define COROUTINE_SIG(n) void n(Coroutine_Head *coroutine)
typedef COROUTINE_SIG(Coroutine_Function);

#define Sys_Create_Coroutine_Sig(name) Coroutine_Head *name(Coroutine_Function *func)
typedef Sys_Create_Coroutine_Sig(System_Create_Coroutine);

#define Sys_Launch_Coroutine_Sig(name) Coroutine_Head *name(Coroutine_Head *head, void *in, void *out)
typedef Sys_Launch_Coroutine_Sig(System_Launch_Coroutine);

#define Sys_Resume_Coroutine_Sig(name) Coroutine_Head *name(Coroutine_Head *head, void *in, void *out)
typedef Sys_Resume_Coroutine_Sig(System_Resume_Coroutine);

#define Sys_Yield_Coroutine_Sig(name) void name(Coroutine_Head *head)
typedef Sys_Yield_Coroutine_Sig(System_Yield_Coroutine);

// thread
struct Thread_Context;

enum Lock_ID{
    FRAME_LOCK,
    CANCEL_LOCK0,
    CANCEL_LOCK1,
    CANCEL_LOCK2,
    CANCEL_LOCK3,
    CANCEL_LOCK4,
    CANCEL_LOCK5,
    CANCEL_LOCK6,
    CANCEL_LOCK7,
    LOCK_COUNT
};

enum Thread_Group_ID{
    BACKGROUND_THREADS,
    THREAD_GROUP_COUNT
};

struct Thread_Memory{
    void *data;
    u32 size;
    u32 id;
};
global Thread_Memory null_thread_memory = {};

struct Thread_Exchange;
struct System_Functions;

#define Job_Callback_Sig(name) void name(        \
System_Functions *system,                \
Thread_Context *thread,                  \
Thread_Memory *memory,                   \
void *data[4])

typedef void Job_Callback(System_Functions *system, Thread_Context *thread, Thread_Memory *memory, void *data[4]);

struct Job_Data{
    Job_Callback *callback;
    void *data[4];
};

#define QUEUE_WRAP 256

#define THREAD_NOT_ASSIGNED 0xFFFFFFFF

#define Sys_Post_Job_Sig(name) u32 name(Thread_Group_ID group_id, Job_Data job)
typedef Sys_Post_Job_Sig(System_Post_Job);

#define Sys_Cancel_Job_Sig(name) void name(Thread_Group_ID group_id, u32 job_id)
typedef Sys_Cancel_Job_Sig(System_Cancel_Job);

#define Sys_Check_Cancel_Sig(name) b32 name(Thread_Context *thread)
typedef Sys_Check_Cancel_Sig(System_Check_Cancel);

#define Sys_Grow_Thread_Memory_Sig(name) void name(Thread_Memory *memory)
typedef Sys_Grow_Thread_Memory_Sig(System_Grow_Thread_Memory);

#define Sys_Acquire_Lock_Sig(name) void name(i32 id)
typedef Sys_Acquire_Lock_Sig(System_Acquire_Lock);

#define Sys_Release_Lock_Sig(name) void name(i32 id)
typedef Sys_Release_Lock_Sig(System_Release_Lock);

// memory
#define Sys_Memory_Allocate_Sig(name) void* name(umem size)
typedef Sys_Memory_Allocate_Sig(System_Memory_Allocate);

#define Sys_Memory_Set_Protection_Sig(name) bool32 name(void *ptr, umem size, u32 flags)
typedef Sys_Memory_Set_Protection_Sig(System_Memory_Set_Protection);

#define Sys_Memory_Free_Sig(name) void name(void *ptr, umem size)
typedef Sys_Memory_Free_Sig(System_Memory_Free);

// file system
#define Sys_Directory_CD_Sig(name) bool32 name(char *dir, i32 *len, i32 cap, char *rel_path, i32 rel_len)
typedef Sys_Directory_CD_Sig(System_Directory_CD);

#define Sys_Get_Current_Path_Sig(name) int32_t name(char *out, i32 capacity)
typedef Sys_Get_Current_Path_Sig(System_Get_Current_Path);

#define Sys_Get_4ed_Path_Sig(name) int32_t name(char *out, i32 capacity)
typedef Sys_Get_4ed_Path_Sig(System_Get_4ed_Path);

// behavior and appearance options
#define Sys_Show_Mouse_Cursor_Sig(name) void name(i32 show)
typedef Sys_Show_Mouse_Cursor_Sig(System_Show_Mouse_Cursor);

#define Sys_Set_Fullscreen_Sig(name) b32 name(b32 full_screen)
typedef Sys_Set_Fullscreen_Sig(System_Set_Fullscreen);

#define Sys_Is_Fullscreen_Sig(name) bool32 name()
typedef Sys_Is_Fullscreen_Sig(System_Is_Fullscreen);

// debug
#define Sys_Log_Sig(name) void name(char *message, u32 length)
typedef Sys_Log_Sig(System_Log);

struct System_Functions{
    Font_Functions font;
    
    // files (tracked api): 11
    System_Set_File_List         *set_file_list;
    System_Get_Canonical         *get_canonical;
    System_Add_Listener          *add_listener;  
    System_Remove_Listener       *remove_listener;
    System_Get_File_Change       *get_file_change;
    System_Quick_File_Attributes *quick_file_attributes;
    System_Load_Handle           *load_handle;
    System_Load_Attributes       *load_attributes;
    System_Load_File             *load_file;
    System_Load_Close            *load_close;
    System_Save_File             *save_file;
    
    // time: 4
    System_Now_Time *now_time;
    System_Wake_Up_Timer_Create *wake_up_timer_create;
    System_Wake_Up_Timer_Release *wake_up_timer_release;
    System_Wake_Up_Timer_Set *wake_up_timer_set;
    
    // clipboard: 1
    System_Post_Clipboard *post_clipboard;
    
    // coroutine: 4
    System_Create_Coroutine *create_coroutine;
    System_Launch_Coroutine *launch_coroutine;
    System_Resume_Coroutine *resume_coroutine;
    System_Yield_Coroutine  *yield_coroutine;
    
    // cli: 4
    System_CLI_Call         *cli_call;
    System_CLI_Begin_Update *cli_begin_update;
    System_CLI_Update_Step  *cli_update_step;
    System_CLI_End_Update   *cli_end_update;
    
    // threads: 6
    System_Post_Job           *post_job;
    System_Cancel_Job         *cancel_job;
    System_Check_Cancel       *check_cancel;
    System_Grow_Thread_Memory *grow_thread_memory;
    System_Acquire_Lock       *acquire_lock;
    System_Release_Lock       *release_lock;
    
    // custom: 9
    System_Memory_Allocate        *memory_allocate;
    System_Memory_Set_Protection  *memory_set_protection;
    System_Memory_Free            *memory_free;
    
    System_Directory_CD           *directory_cd;
    System_Get_Current_Path       *get_current_path;
    System_Get_4ed_Path           *get_4ed_path;
    
    System_Show_Mouse_Cursor      *show_mouse_cursor;
    System_Set_Fullscreen         *set_fullscreen;
    System_Is_Fullscreen          *is_fullscreen;
    
    // debug: 1
    System_Log *log;
};

#endif

// BOTTOM

