/*
4tech_file_moving.h - Code for moving files around on the file system.
By Allen Webster
21.01.2017 (dd.mm.yyyy)
*/

// TOP

#if !defined(FRED_FILE_MOVING_H)
#define FRED_FILE_MOVING_H

#include "../4ed_os_comp_cracking.h"

#include <stdio.h>  // include system for windows
#include <stdlib.h> // include system for linux   (YAY!)
#include <stdarg.h>
#include <string.h>

//
// API
//

// System commands
static char SF_CMD[4096];
static i32 error_state = 0;
static i32 prev_error = 0;

#if defined(FM_PRINT_COMMANDS)
#define SYSTEMF_PRINTF(...) printf(__VA_ARGS__);
#else
#define SYSTEMF_PRINTF(...)
#endif

#define systemf(...) do{                                       \
    i32 n = snprintf(SF_CMD, sizeof(SF_CMD), __VA_ARGS__); \
    AllowLocal(n);                                             \
    Assert(n < sizeof(SF_CMD));                                \
    SYSTEMF_PRINTF("%s\n", SF_CMD);                            \
    prev_error = system(SF_CMD);                               \
    if (prev_error != 0) error_state = 1;                      \
}while(0)

internal void fm_execute_in_dir(char *dir, char *str, char *args);

// Init
internal void fm_init_system();

// Timing
internal u64  fm_get_time();

#define LLU_CAST(n) (long long unsigned int)(n)
#define BEGIN_TIME_SECTION() u64 start = fm_get_time()
#define END_TIME_SECTION(n) u64 total = fm_get_time() - start; printf("%-20s: %.2llu.%.6llu\n", (n), LLU_CAST(total/1000000), LLU_CAST(total%1000000));

// Files and Folders Manipulation
internal void fm_make_folder_if_missing(char *dir);
internal void fm_clear_folder(char *folder);
internal void fm_delete_file(char *file);
internal void fm_copy_file(char *file, char *newname);
internal void fm_copy_all(char *source, char *tag, char *folder);
internal void fm_copy_folder(char *src_dir, char *dst_dir, char *src_folder);

// File Reading and Writing
internal void fm_write_file(char *file_name, char *data, u32 size);

// Zip
internal void fm_zip(char *parent, char *folder, char *dest);

// Slash Correction
internal void fm_slash_fix(char *path);

// Memory concat helpers
internal char *fm_prepare_string_internal(char *s1, ...);
#define fm_str(...) fm_prepare_string_internal(__VA_ARGS__, 0)

internal char **fm_prepare_list_internal(char **l1, ...);
#define fm_list(...) fm_prepare_list_internal(__VA_ARGS__, 0)

internal char **fm_list_one_item(char *item);

internal void *fm__push(umem size);
internal void fm_align();

#define fm_push_array(T,c) (T*)fm__push(sizeof(T)*c)

// File System Navigation
typedef umem Temp;
internal Temp fm_begin_temp();
internal void fm_end_temp(Temp temp);

internal i32  fm_get_current_directory(char *buffer, i32 max);

struct Temp_Dir{
    char dir[512];
};

internal Temp_Dir fm_pushdir(char *dir);
internal void fm_popdir(Temp_Dir temp);

// Build Line
#define BUILD_LINE_MAX 4096
struct Build_Line{
    char build_optionsA[BUILD_LINE_MAX];
    char build_optionsB[BUILD_LINE_MAX];
    char *build_options;
    char *build_options_prev;
    i32 build_max;
};

internal void fm_init_build_line(Build_Line *line);
internal void fm_finish_build_line(Build_Line *line);

internal void fm__swap_ptr(char **A, char **B);

#if defined(IS_CL)

#define fm_add_to_line(line, str, ...) do{  \
    snprintf(line.build_options,            \
    line.build_max, "%s "str,               \
    line.build_options_prev, __VA_ARGS__);  \
    fm__swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#elif defined(IS_GCC)

#define fm_add_to_line(line, str, ...) do{                   \
    snprintf(line.build_options, line.build_max, "%s "str,   \
    line.build_options_prev, ##__VA_ARGS__);                 \
    fm__swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#endif

// Slashes
#if defined(IS_WINDOWS)
#define SLASH "\\"
static char platform_correct_slash = '\\';
#elif defined(IS_LINUX) || defined(IS_MAC)
#define SLASH "/"
static char platform_correct_slash = '/';
#else
#error Slash not set for this platform.
#endif

// File Extensions
#if defined(IS_WINDOWS)
# define EXE ".exe"
#elif defined(IS_LINUX) || defined(IS_MAC)
# define EXE ""
#else
# error No EXE format specified for this OS
#endif

#if defined(IS_WINDOWS)
# define PDB ".pdb"
#elif defined(IS_LINUX) || defined(IS_MAC)
# define PDB ""
#else
# error No PDB format specified for this OS
#endif

#if defined(IS_WINDOWS)
# define DLL ".dll"
#elif defined(IS_LINUX) || defined(IS_MAC)
# define DLL ".so"
#else
# error No DLL format specified for this OS
#endif

#if defined(IS_WINDOWS)
# define BAT ".bat"
#elif defined(IS_LINUX) || defined(IS_MAC)
# define BAT ".sh"
#else
# error No BAT format specified for this OS
#endif

#endif

//
// Implementation
//

#if defined(FTECH_FILE_MOVING_IMPLEMENTATION) && !defined(FTECH_FILE_MOVING_IMPL_GUARD)
#define FTECH_FILE_MOVING_IMPL_GUARD

char *fm_arena_memory = 0;
umem fm_arena_pos = 0;
umem fm_arena_max = 0;

internal void
fm__init_memory(){
    Assert(fm_arena_memory == 0);
    fm_arena_max = MB(512);
    fm_arena_memory = (char*)malloc(fm_arena_max);
}

internal Temp
fm_begin_temp(){
    return(fm_arena_pos);
}

internal void
fm_end_temp(Temp temp){
    fm_arena_pos = temp;
}

internal void*
fm__push(umem size){
    void *result = fm_arena_memory + fm_arena_pos;
    if (size + fm_arena_pos > fm_arena_max){
        result = 0;
    }
    else{
        fm_arena_pos += size;
    }
    return(result);
}

internal void
fm_align(){
    fm_arena_pos = (fm_arena_pos+7)&(~7);
}

//
// Windows implementation
//

#if defined(IS_WINDOWS)

typedef u32 DWORD;
typedef i32  LONG;
typedef i64  LONGLONG;
typedef char*    LPTSTR;
typedef char*    LPCTSTR;
typedef i32  BOOL;
typedef void*    LPSECURITY_ATTRIBUTES;
typedef union    _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG  HighPart;
    };
    struct {
        DWORD LowPart;
        LONG  HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
typedef void*    HANDLE;
typedef void*    PVOID;
typedef void*    LPVOID;
typedef void*    LPCVOID;
typedef DWORD*   LPDWORD;
#if defined(_WIN64)
typedef unsigned __int64 ULONG_PTR;
#else
typedef unsigned long ULONG_PTR;
#endif
typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID  Pointer;
    };
    HANDLE    hEvent;
} OVERLAPPED, *LPOVERLAPPED;

#define WINAPI

extern "C"{
    DWORD WINAPI GetCurrentDirectoryA(_In_  DWORD  nBufferLength, _Out_ LPTSTR lpBuffer);
    BOOL WINAPI SetCurrentDirectoryA(_In_ LPCTSTR lpPathName);
    BOOL WINAPI QueryPerformanceCounter(_Out_ LARGE_INTEGER *lpPerformanceCount);
    BOOL WINAPI QueryPerformanceFrequency(_Out_ LARGE_INTEGER *lpFrequency);
    BOOL WINAPI CreateDirectoryA(_In_ LPCTSTR lpPathName, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL WINAPI CopyFileA(_In_ LPCTSTR lpExistingFileName, _In_ LPCTSTR lpNewFileName, _In_ BOOL bFailIfExists);
    
    HANDLE WINAPI CreateFileA(_In_ LPCTSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode,_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);
    BOOL WINAPI WriteFile(_In_ HANDLE hFile, _In_ LPCVOID lpBuffer, _In_ DWORD nNumberOfBytesToWrite, _Out_opt_ LPDWORD lpNumberOfBytesWritten, _Inout_opt_ LPOVERLAPPED lpOverlapped);
    BOOL WINAPI ReadFile(_In_ HANDLE hFile, _Out_ LPVOID lpBuffer, _In_ DWORD nNumberOfBytesToRead, _Out_opt_ LPDWORD lpNumberOfBytesRead, _Inout_opt_ LPOVERLAPPED lpOverlapped);
    BOOL WINAPI CloseHandle(_In_ HANDLE hObject);
}

#define INVALID_HANDLE_VALUE ((HANDLE) -1)

#define GENERIC_READ                     0x80000000
#define GENERIC_WRITE                    0x40000000
#define GENERIC_EXECUTE                  0x20000000
#define GENERIC_ALL                      0x10000000

#define CREATE_NEW                       1
#define CREATE_ALWAYS                    2
#define OPEN_EXISTING                    3
#define OPEN_ALWAYS                      4
#define TRUNCATE_EXISTING                5

#define FILE_ATTRIBUTE_READONLY          0x00000001  
#define FILE_ATTRIBUTE_NORMAL            0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY         0x00000100 

static u64 perf_frequency;

static void
fm_init_system(){
    LARGE_INTEGER lint;
    if (QueryPerformanceFrequency(&lint)){
        perf_frequency = lint.QuadPart;
    }
    fm__init_memory();
}

static Temp_Dir
fm_pushdir(char *dir){
    Temp_Dir temp = {0};
    GetCurrentDirectoryA(sizeof(temp.dir), temp.dir);
    SetCurrentDirectoryA(dir);
    return(temp);
}

static void
fm_popdir(Temp_Dir temp){
    SetCurrentDirectoryA(temp.dir);
}

static u64
fm_get_time(){
    u64 time = 0;
    LARGE_INTEGER lint;
    if (QueryPerformanceCounter(&lint)){
        time = lint.QuadPart;
        time = (time * 1000000) / perf_frequency;
    }
    return(time);
}

static i32
fm_get_current_directory(char *buffer, i32 max){
    i32 result = GetCurrentDirectoryA(max, buffer);
    return(result);
}

static void
fm_execute_in_dir(char *dir, char *str, char *args){
    if (dir){
        Temp_Dir temp = fm_pushdir(dir);
        if (args){
            systemf("call \"%s\" %s", str, args);
        }
        else{
            systemf("call \"%s\"", str);
        }
        fm_popdir(temp);
    }
    else{
        if (args){
            systemf("call \"%s\" %s", str, args);
        }
        else{
            systemf("call \"%s\"", str);
        }
    }
}

static void
fm_slash_fix(char *path){
    if (path != 0){
        for (i32 i = 0; path[i]; ++i){
            if (path[i] == '/') path[i] = '\\';
        }
    }
}

static void
fm_make_folder_if_missing(char *dir){
    char *path = fm_str(dir);
    char *p = path;
    for (; *p; ++p){
        if (*p == '\\'){
            *p = 0;
            CreateDirectoryA(path, 0);
            *p = '\\';
        }
    }
    CreateDirectoryA(path, 0);
}

static void
fm_clear_folder(char *folder){
    fprintf(stdout, "clearing folder %s\n", folder);
    systemf("del /S /Q /F %s\\* > nul & rmdir /S /Q %s > nul & mkdir %s > nul", folder, folder, folder);
}

static void
fm_delete_file(char *file){
    systemf("del %s", file);
}

static void
fm_copy_file(char *file, char *newname){
    CopyFileA(file, newname, 0);
}

static void
fm_copy_all(char *source, char *tag, char *folder){
    if (source){
        fprintf(stdout, "copy %s\\%s to %s\n", source, tag, folder);
        systemf("copy %s\\%s %s\\* > nul", source, tag, folder);
    }
    else{
        fprintf(stdout, "copy %s to %s\n", tag, folder);
        systemf("copy %s %s\\* > nul", tag, folder);
    }
}

internal void
fm_write_file(char *file_name, char *data, u32 size){
    HANDLE file = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE){
        DWORD written = 0;
        for (;written < size;){
            DWORD newly_written = 0;
            if (!WriteFile(file, data + written, size - written, &newly_written, 0)){
                break;
            }
            written += newly_written;
        }
        Assert(written == size);
        CloseHandle(file);
    }
}

static void
fm_zip(char *parent, char *folder, char *dest){
    char cdir[512];
    fm_get_current_directory(cdir, sizeof(cdir));
    
    Temp_Dir temp = fm_pushdir(parent);
    systemf("%s\\zip %s\\4ed_gobble.zip", cdir, cdir);
    fm_popdir(temp);
    
    systemf("copy %s\\4ed_gobble.zip %s & del %s\\4ed_gobble.zip", cdir, dest, cdir);
}

//
// Unix implementation
//

#elif defined(IS_LINUX) || defined(IS_MAC)

#include <time.h>
#include <unistd.h>

static Temp_Dir
fm_pushdir(char *dir){
    Temp_Dir temp;
    char *result = getcwd(temp.dir, sizeof(temp.dir));
    i32 chresult = chdir(dir);
    if (result == 0 || chresult != 0){
        printf("trying pushdir %s\n", dir);
        Assert(result != 0);
        Assert(chresult == 0);
    }
    return(temp);
}

static void
fm_popdir(Temp_Dir temp){
    chdir(temp.dir);
}

static void
fm_init_system(){
    fm__init_memory();
}

static u64
fm_get_time(){
    struct timespec spec;
    u64 result;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * (u64)(1000000)) + (spec.tv_nsec / (u64)(1000));
    return(result);
}

static i32
fm_get_current_directory(char *buffer, i32 max){
    i32 result = 0;
    char *d = getcwd(buffer, max);
    if (d == buffer){
        result = strlen(buffer);
    }
    return(result);
}

static void
fm_execute_in_dir(char *dir, char *str, char *args){
    if (dir){
        if (args){
            Temp_Dir temp = fm_pushdir(dir);
            systemf("%s %s", str, args);
            fm_popdir(temp);
        }
        else{
            Temp_Dir temp = fm_pushdir(dir);
            systemf("%s", str);
            fm_popdir(temp);
        }
    }
    else{
        if (args){
            systemf("%s %s", str, args);
        }
        else{
            systemf("%s", str);
        }
    }
}

static void
fm_slash_fix(char *path){}

static void
fm_make_folder_if_missing(char *dir){
    systemf("mkdir -p %s", dir);
}

static void
fm_clear_folder(char *folder){
    fprintf(stdout, "clearing folder %s\n", folder);
    systemf("rm -rf %s* > /dev/null", folder);
}

static void
fm_delete_file(char *file){
    systemf("rm %s", file);
}

static void
fm_copy_file(char *file, char *newname){
    systemf("cp %s %s", file, newname);
}

static void
fm_copy_all(char *source, char *tag, char *folder){
    if (source){
        fprintf(stdout, "copy %s/%s to %s\n", source, tag, folder);
        systemf("cp -f %s/%s %s > /dev/null", source, tag, folder);
    }
    else{
        fprintf(stdout, "copy %s to %s\n", tag, folder);
        systemf("cp -f %s %s > /dev/null", tag, folder);
    }
}

internal void
fm_write_file(char *file_name, char *data, u32 size){
    // TODO(allen): Real unix version?
    FILE *file = fopen(file_name, "wb");
    if (file != 0){
        fwrite(data, 1, size, file);
        fclose(file);
    }
}

static void
fm_zip(char *parent, char *folder, char *file){
    Temp_Dir temp = fm_pushdir(parent);
    printf("PARENT DIR: %s\n", parent);
    systemf("zip -r %s %s", file, folder);
    fm_popdir(temp);
}

#else
#error This OS is not supported yet
#endif

internal void
fm_copy_folder(char *src_dir, char *dst_dir, char *src_folder){
    Temp_Dir temp = fm_pushdir(src_dir);
    fm_make_folder_if_missing(fm_str(dst_dir, "/", src_folder));
    char *copy_name = fm_str(dst_dir, "/", src_folder);
    fm_copy_all(src_folder, "*", copy_name);
    fm_popdir(temp);
}

// List Helpers
internal umem
listsize(void *p, umem item_size){
    u64 zero = 0;
    u8 *ptr = (u8*)p;
    for (;memcmp(ptr, &zero, item_size) != 0; ptr += item_size);
    umem size = (ptr - (u8*)p);
    return(size);
}

internal void*
fm__prepare(umem item_size, void *i1, va_list list){
    umem size = listsize(i1, item_size);
    void *result = (void*)fm__push(size);
    memcpy(result, i1, size);
    
    void *ln = va_arg(list, void*);
    for (;ln != 0;){
        size = listsize(ln, item_size);
        void *new_str = (void*)fm__push(size);
        memcpy(new_str, ln, size);
        ln = va_arg(list, void*);
    }
    
    void *terminator = (void*)fm__push(item_size);
    memset(terminator, 0, item_size);
    return(result);
}

internal char*
fm_prepare_string_internal(char *s1, ...){
    umem item_size = sizeof(*s1);
    va_list list;
    va_start(list, s1);
    char *result = (char*)fm__prepare(item_size, s1, list);
    va_end(list);
    fm_slash_fix(result);
    return(result);
}

internal char**
fm_prepare_list_internal(char **p1, ...){
    umem item_size = sizeof(*p1);
    va_list list;
    va_start(list, p1);
    char **result = (char**)fm__prepare(item_size, p1, list);
    va_end(list);
    return(result);
}

internal char**
fm_list_one_item(char *item){
    char **result = (char**)fm__push(sizeof(char*)*2);
    result[0] = item;
    result[1] = 0;
    return(result);
}

// Build Line
internal void
fm_init_build_line(Build_Line *line){
    line->build_options = line->build_optionsA;
    line->build_options_prev = line->build_optionsB;
    line->build_optionsA[0] = 0;
    line->build_optionsB[0] = 0;
    line->build_max = BUILD_LINE_MAX;
}

internal void
fm_finish_build_line(Build_Line *line){
    fm__swap_ptr(&line->build_options, &line->build_options_prev);
}

internal void
fm__swap_ptr(char **A, char **B){
    char *a = *A;
    char *b = *B;
    *A = b;
    *B = a;
}

#endif

// BOTTOM

