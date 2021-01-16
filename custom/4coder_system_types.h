/*
 * 4coder_system_types.h - Types relating to the system api.
 */

// TOP

#if !defined(FCODER_SYSTEM_TYPES_H)
#define FCODER_SYSTEM_TYPES_H

typedef i32 Key_Mode;
enum{
    KeyMode_LanguageArranged,
    KeyMode_Physical,
};

struct Plat_Handle{
    u32 d[4];
};
typedef Plat_Handle System_Library;
typedef Plat_Handle System_Thread;
typedef Plat_Handle System_Mutex;
typedef Plat_Handle System_Condition_Variable;
typedef void Thread_Function(void *ptr);
struct CLI_Handles{
    Plat_Handle proc;
    Plat_Handle out_read;
    Plat_Handle out_write;
    Plat_Handle in_read;
    Plat_Handle in_write;
    u32 scratch_space[4];
    i32 exit;
};

typedef i32 System_Path_Code;
enum{
    SystemPath_CurrentDirectory,
    SystemPath_Binary,
    SystemPath_UserDirectory,
};

struct Memory_Annotation_Node{
    Memory_Annotation_Node *next;
    String_Const_u8 location;
    void *address;
    u64 size;
};

struct Memory_Annotation{
    Memory_Annotation_Node *first;
    Memory_Annotation_Node *last;
    i32 count;
};

struct Mutex_Lock{
    Mutex_Lock(System_Mutex mutex);
    ~Mutex_Lock();
    operator System_Mutex();
    System_Mutex mutex;
};

#endif

// BOTTOM

