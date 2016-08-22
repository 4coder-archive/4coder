/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2014
 *
 * System functions for project codename "4ed"
 *
 */

// TOP

struct Plat_Handle{
    u32 d[4];
};

inline int
handle_equal(Plat_Handle a, Plat_Handle b){
    int result = (memcmp(&a, &b, sizeof(a)) == 0);
    return(result);
}

struct Unique_Hash{
    u32 d[4];
};

inline Unique_Hash
uhash_zero(){
    Unique_Hash r = {0};
    return(r);
}

inline int
uhash_equal(Unique_Hash a, Unique_Hash b){
    int result = (memcmp(&a, &b, sizeof(a)) == 0);
    return(result);
}

// TODO(allen): make directory a char* to signal that it must be null terminated
#define Sys_Set_File_List_Sig(name) void name(File_List *file_list, String directory)
typedef Sys_Set_File_List_Sig(System_Set_File_List);

enum{
    TrackFileFlag_ExistingOrFail = 0x0,
    TrackFileFlag_NewOrFail = 0x1,
    TrackFileFlag_NewAlways = 0x2,
    TrackFileFlag_ExistingOrNew = 0x3,
};

#define Sys_Track_File_Sig(name) Unique_Hash name(char *filename, u32 flags)
typedef Sys_Track_File_Sig(System_Track_File);

#define Sys_Untrack_File_Sig(name) i32 name(Unique_Hash index)
typedef Sys_Untrack_File_Sig(System_Untrack_File);

#define Sys_Get_File_Index_Sig(name) i32 name(char *filename, Unique_Hash *index)
typedef Sys_Get_File_Index_Sig(System_Get_File_Index);

#define Sys_Get_File_Time_Sig(name) i32 name(Unique_Hash index, u64 *time)
typedef Sys_Get_File_Time_Sig(System_Get_File_Time);

#define Sys_Now_File_Time_Sig(name) void name(u64 *time)
typedef Sys_Now_File_Time_Sig(System_Now_File_Time);

#define Sys_Get_Changed_File_Sig(name) i32 name(Unique_Hash *index)
typedef Sys_Get_Changed_File_Sig(System_Get_Changed_File);

#define Sys_File_Size_Sig(name) u32 name(Unique_Hash index)
typedef Sys_File_Size_Sig(System_File_Size);

#define Sys_Load_File_Sig(name) i32 name(Unique_Hash index, char *buffer, i32 size)
typedef Sys_Load_File_Sig(System_Load_File);

#define Sys_Save_File_Sig(name) i32 name(Unique_Hash index, char *buffer, i32 size)
typedef Sys_Save_File_Sig(System_Save_File);


#define Sys_Post_Clipboard_Sig(name) void name(String str)
typedef Sys_Post_Clipboard_Sig(System_Post_Clipboard);

// cli
struct CLI_Handles{
    Plat_Handle proc;
    Plat_Handle out_read;
    Plat_Handle out_write;
    u32 scratch_space[4];
    i32 exit;
};

#define Sys_CLI_Call_Sig(name) b32 name(char *path, char *script_name, CLI_Handles *cli_out)
typedef Sys_CLI_Call_Sig(System_CLI_Call);

#define Sys_CLI_Begin_Update_Sig(name) void name(CLI_Handles *cli)
typedef Sys_CLI_Begin_Update_Sig(System_CLI_Begin_Update);

#define Sys_CLI_Update_Step_Sig(name) b32 name(CLI_Handles *cli, char *dest, u32 max, u32 *amount)
typedef Sys_CLI_Update_Step_Sig(System_CLI_Update_Step);

#define Sys_CLI_End_Update_Sig(name) b32 name(CLI_Handles *cli)
typedef Sys_CLI_End_Update_Sig(System_CLI_End_Update);

// coroutine

#define Coroutine_Function_Sig(name) void name(struct Coroutine *coroutine)
typedef Coroutine_Function_Sig(Coroutine_Function);

struct Coroutine{
    Plat_Handle plat_handle;
    Coroutine_Function *func;
    void *yield_handle;
    void *in;
    void *out;
};

#define Sys_Create_Coroutine_Sig(name) Coroutine *name(Coroutine_Function *func)
typedef Sys_Create_Coroutine_Sig(System_Create_Coroutine);

#define Sys_Launch_Coroutine_Sig(name) Coroutine *name(Coroutine *coroutine, void *in, void *out)
typedef Sys_Launch_Coroutine_Sig(System_Launch_Coroutine);

#define Sys_Resume_Coroutine_Sig(name) Coroutine *name(Coroutine *coroutine, void *in, void *out)
typedef Sys_Resume_Coroutine_Sig(System_Resume_Coroutine);

#define Sys_Yield_Coroutine_Sig(name) void name(Coroutine *coroutine)
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
    i32 size;
    i32 id;
};
inline Thread_Memory
thread_memory_zero(){
    Thread_Memory memory={0};
    return(memory);
}

struct Thread_Exchange;
struct System_Functions;

#define Job_Callback_Sig(name) void name(        \
        System_Functions *system,                \
        Thread_Context *thread,                  \
        Thread_Memory *memory,                   \
        void *data[2])
typedef Job_Callback_Sig(Job_Callback);

struct Job_Data{
    Job_Callback *callback;
    void *data[2];
};

struct Full_Job_Data{
    Job_Data job;
    
    u32 running_thread;
    u32 id;
};

struct Unbounded_Work_Queue{
    Full_Job_Data *jobs;
    i32 count, max, skip;
    
    u32 next_job_id;
};

#define QUEUE_WRAP 256

struct Work_Queue{
    Full_Job_Data jobs[QUEUE_WRAP];
    Plat_Handle semaphore;
    volatile u32 write_position;
    volatile u32 read_position;
};

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

// debug
#define INTERNAL_Sys_Sentinel_Sig(name) Bubble* name()
typedef INTERNAL_Sys_Sentinel_Sig(INTERNAL_System_Sentinel);

#define INTERNAL_Sys_Get_Thread_States_Sig(name) void name(Thread_Group_ID id, b8 *running, i32 *pending)
typedef INTERNAL_Sys_Get_Thread_States_Sig(INTERNAL_System_Get_Thread_States);

#define INTERNAL_Sys_Debug_Message_Sig(name) void name(char *message)
typedef INTERNAL_Sys_Debug_Message_Sig(INTERNAL_System_Debug_Message);

struct System_Functions{
    
    // files (tracked api): 10
    System_Set_File_List *set_file_list;
    System_Track_File *track_file;
    System_Untrack_File *untrack_file;
    System_Get_File_Index *get_file_index;
    System_Get_File_Time *get_file_time;
    System_Now_File_Time *now_file_time;
    System_Get_Changed_File *get_changed_file;
    System_File_Size *file_size;
    System_Load_File *load_file;
    System_Save_File *save_file;
    
    // 4coder_custom.h: 7
    Memory_Allocate_Function *memory_allocate;
    Memory_Set_Protection_Function *memory_set_protection;
    Memory_Free_Function *memory_free;
    File_Exists_Function *file_exists;
    Directory_CD_Function *directory_cd;
    Get_4ed_Path_Function *get_4ed_path;
    Show_Mouse_Cursor_Function *show_mouse_cursor;
    
    // clipboard: 1
    System_Post_Clipboard *post_clipboard;
    
    // coroutine: 4
    System_Create_Coroutine *create_coroutine;
    System_Launch_Coroutine *launch_coroutine;
    System_Resume_Coroutine *resume_coroutine;
    System_Yield_Coroutine *yield_coroutine;
    
    // cli: 4
    System_CLI_Call *cli_call;
    System_CLI_Begin_Update *cli_begin_update;
    System_CLI_Update_Step *cli_update_step;
    System_CLI_End_Update *cli_end_update;
    
    // threads: 7
    System_Post_Job *post_job;
    System_Cancel_Job *cancel_job;
    System_Check_Cancel *check_cancel;
    System_Grow_Thread_Memory *grow_thread_memory;
    System_Acquire_Lock *acquire_lock;
    System_Release_Lock *release_lock;
    
    // debug: 3
    INTERNAL_System_Sentinel *internal_sentinel;
    INTERNAL_System_Get_Thread_States *internal_get_thread_states;
    INTERNAL_System_Debug_Message *internal_debug_message;
    
    // non-function details
    char slash;
};

// BOTTOM

