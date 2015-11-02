/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2014
 *
 * System functions for project codename "4ed"
 *
 */

struct Plat_Handle{ u64 d[2]; };

// TODO(allen): This should either be a String or it should be improved
// to handle 64-bit sized files.  Staying in this state, however, is unacceptable.
struct File_Data{
	void *data;
	u32 size;
};

struct Time_Stamp{
    u64 time;
    bool32 success;
};

internal File_Data
system_load_file(u8 *filename);

internal bool32
system_save_file(u8 *filename, void *data, i32 size);

internal Time_Stamp
system_file_time_stamp(u8 *filename);

internal u64
system_get_now();

internal void
system_free_file(File_Data data);

internal void
system_fatal_error(u8 *message);

internal i32
system_get_working_directory(u8 *destination, i32 max_size);

internal i32
system_get_easy_directory(u8 *destination);

struct File_Info{
    String filename;
    bool32 folder;
    //bool32 loaded;
};

struct File_List{
    File_Info *infos;
    i32 count;
    
    void *block;
};

internal File_List
system_get_files(String directory);

internal void
system_free_file_list(File_List list);

internal void*
system_get_memory_(i32 size, i32 line_number, char *file_name);

#define system_get_memory(size) system_get_memory_(size, __LINE__, __FILE__)

internal void
system_free_memory(void *block);

internal void
system_post_clipboard(String str);

internal i64
system_time();

struct CLI_Handles{
    Plat_Handle proc;
    Plat_Handle out_read;
    Plat_Handle out_write;
    u32 scratch_space[4];
};

internal b32
system_cli_call(char *path, char *script_name, CLI_Handles *cli_out);

internal void
system_cli_begin_update(CLI_Handles *cli);

internal b32
system_cli_update_step(CLI_Handles *cli, char *dest, u32 max, u32 *amount);

internal b32
system_cli_end_update(CLI_Handles *cli);

struct Thread_Context;

struct Thread_Memory{
    void *data;
    i32 size;
    i32 id;
};

internal u32
system_thread_get_id(Thread_Context *thread);

internal u32
system_thread_current_job_id(Thread_Context *thread);

enum Thread_Group_ID{
    BACKGROUND_THREADS,
    THREAD_GROUP_COUNT
};

#define JOB_CALLBACK(name) void name(Thread_Context *thread, Thread_Memory *memory, void *data[2])
typedef JOB_CALLBACK(Job_Callback);

struct Job_Data{
    Job_Callback *callback;
    void *data[2];
    i32 memory_request;
};

internal u32
system_post_job(Thread_Group_ID group_id, Job_Data job);

internal void
system_cancel_job(Thread_Group_ID group_id, u32 job_id);

internal bool32
system_job_is_pending(Thread_Group_ID group_id, u32 job_id);

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

internal void
system_aquire_lock(Lock_ID id);

internal void
system_release_lock(Lock_ID id);

internal void
system_aquire_lock(i32 id);

internal void
system_release_lock(i32 id);

internal void
system_grow_thread_memory(Thread_Memory *memory);

internal void
system_force_redraw();

#if FRED_INTERNAL
internal Bubble*
INTERNAL_system_sentinel();

internal void
INTERNAL_get_thread_states(Thread_Group_ID id, bool8 *running, i32 *pending);
#endif

