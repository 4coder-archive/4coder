/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2014
 *
 * System functions for project codename "4ed"
 *
 */

// TOP

struct System_Functions;

struct Plat_Handle{
    u32 d[4];
};

struct Time_Stamp{
    u64 time;
    b32 success;
};

#define Sys_Load_File_Sig(name) Data name(char *filename)
typedef Sys_Load_File_Sig(System_Load_File);

#define Sys_Save_File_Sig(name) i32 name(char *filename, void *data, i32 size)
typedef Sys_Save_File_Sig(System_Save_File);

#define Sys_File_Size_Sig(name) i32 name(char *filename)
typedef Sys_File_Size_Sig(System_File_Size);

#define Sys_File_Time_Stamp_Sig(name) Time_Stamp name(char *filename)
typedef Sys_File_Time_Stamp_Sig(System_File_Time_Stamp);

#define Sys_Time_Stamp_Now_Sig(name) u64 name()
typedef Sys_Time_Stamp_Now_Sig(System_Time_Stamp_Now);

#define Sys_Free_File_Sig(name) void name(Data file)
typedef Sys_Free_File_Sig(System_Free_File);

#define Sys_Get_Current_Directory_Sig(name) i32 name(char *out, i32 max)
typedef Sys_Get_Current_Directory_Sig(System_Get_Current_Directory);

#define Sys_Get_Easy_Directory_Sig(name) i32 name(char *destination)
typedef Sys_Get_Easy_Directory_Sig(System_Get_Easy_Directory);

struct File_Info{
    String filename;
    b32 folder;
};

struct File_List{
    File_Info *infos;
    void *block;
    i32 count;
};

#define Sys_Get_File_List_Sig(name) File_List name(String directory)
typedef Sys_Get_File_List_Sig(System_Get_File_List);

#define Sys_Free_File_List_Sig(name) void name(File_List list)
typedef Sys_Free_File_List_Sig(System_Free_File_List);

#define Sys_Get_Memory_Sig(name) void* name(i32 size, i32 line_number, char *filename)
typedef Sys_Get_Memory_Sig(System_Get_Memory);

#define get_memory(size) get_memory_full(size, __LINE__, __FILE__)

#define Sys_Free_Memory_Sig(name) void name(void *block)
typedef Sys_Free_Memory_Sig(System_Free_Memory);

#define Sys_Post_Clipboard_Sig(name) void name(String str)
typedef Sys_Post_Clipboard_Sig(System_Post_Clipboard);

#define Sys_Time_Sig(name) i64 name()
typedef Sys_Time_Sig(System_Time);

struct CLI_Handles{
    Plat_Handle proc;
    Plat_Handle out_read;
    Plat_Handle out_write;
    u32 scratch_space[4];
};

#define Sys_CLI_Call_Sig(name) b32 name(char *path, char *script, CLI_Handles *cli)
typedef Sys_CLI_Call_Sig(System_CLI_Call);

#define Sys_CLI_Begin_Update_Sig(name) void name(CLI_Handles *cli)
typedef Sys_CLI_Begin_Update_Sig(System_CLI_Begin_Update);

#define Sys_CLI_Update_Step_Sig(name) b32 name(CLI_Handles *cli, char *dest, u32 max, u32 *amount)
typedef Sys_CLI_Update_Step_Sig(System_CLI_Update_Step);

#define Sys_CLI_End_Update_Sig(name) b32 name(CLI_Handles *cli)
typedef Sys_CLI_End_Update_Sig(System_CLI_End_Update);

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
    i32 size;
    i32 id;
};

#define Job_Callback_Sig(name) void name(\
        System_Functions *system, Thread_Context *thread, Thread_Memory *memory, void *data[2])
typedef Job_Callback_Sig(Job_Callback);

struct Job_Data{
    Job_Callback *callback;
    void *data[2];
    i32 memory_request;
};

#define Sys_Thread_Get_ID_Sig(name) u32 name(Thread_Context *thread)
typedef Sys_Thread_Get_ID_Sig(System_Thread_Get_ID);

#define Sys_Thread_Current_Job_ID_Sig(name) u32 name(Thread_Context *thread)
typedef Sys_Thread_Current_Job_ID_Sig(System_Thread_Current_Job_ID);

#define Sys_Post_Job_Sig(name) u32 name(Thread_Group_ID id, Job_Data job)
typedef Sys_Post_Job_Sig(System_Post_Job);

#define Sys_Cancel_Job_Sig(name) void name(Thread_Group_ID id, u32 job_id)
typedef Sys_Cancel_Job_Sig(System_Cancel_Job);

#define Sys_Job_Is_Pending_Sig(name) b32 name(Thread_Group_ID id, u32 job_id)
typedef Sys_Job_Is_Pending_Sig(System_Job_Is_Pending);

#define Sys_Grow_Thread_Memory_Sig(name) void name(Thread_Memory *memory)
typedef Sys_Grow_Thread_Memory_Sig(System_Grow_Thread_Memory);

#define Sys_Acquire_Lock_Sig(name) void name(i32 id)
typedef Sys_Acquire_Lock_Sig(System_Acquire_Lock);

#define Sys_Release_Lock_Sig(name) void name(i32 id)
typedef Sys_Release_Lock_Sig(System_Release_Lock);

#define Sys_Force_Redraw_Sig(name) void name()
typedef Sys_Force_Redraw_Sig(System_Force_Redraw);

#define INTERNAL_Sys_Sentinel_Sig(name) Bubble* name()
typedef INTERNAL_Sys_Sentinel_Sig(INTERNAL_System_Sentinel);

#define INTERNAL_Sys_Get_Thread_States_Sig(name) void name(Thread_Group_ID id, b8 *running, i32 *pending)
typedef INTERNAL_Sys_Get_Thread_States_Sig(INTERNAL_System_Get_Thread_States);

#define INTERNAL_Sys_Debug_Message_Sig(name) void name(char *message)
typedef INTERNAL_Sys_Debug_Message_Sig(INTERNAL_System_Debug_Message);

struct System_Functions{
    System_Load_File *load_file;
    System_Save_File *save_file;
    System_File_Time_Stamp *file_time_stamp;
    System_Time_Stamp_Now *time_stamp_now;
    System_Free_File *free_file;
    
    System_Get_Current_Directory *get_current_directory;
    System_Get_Easy_Directory *get_easy_directory;

    System_Get_File_List *get_file_list;
    System_Free_File_List *free_file_list;
    
    System_Get_Memory *get_memory_full;
    System_Free_Memory *free_memory;

    System_Post_Clipboard *post_clipboard;
    System_Time *time;

    System_CLI_Call *cli_call;
    System_CLI_Begin_Update *cli_begin_update;
    System_CLI_Update_Step *cli_update_step;
    System_CLI_End_Update *cli_end_update;

    System_Thread_Get_ID *thread_get_id;
    System_Thread_Current_Job_ID *thread_current_job_id;
    System_Post_Job *post_job;
    System_Cancel_Job *cancel_job;
    System_Job_Is_Pending *job_is_pending;
    System_Grow_Thread_Memory *grow_thread_memory;
    System_Acquire_Lock *acquire_lock;
    System_Release_Lock *release_lock;
    
    System_Force_Redraw *force_redraw;
    
    INTERNAL_System_Sentinel *internal_sentinel;
    INTERNAL_System_Get_Thread_States *internal_get_thread_states;
    INTERNAL_System_Debug_Message *internal_debug_message;
};

// BOTTOM

