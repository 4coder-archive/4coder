/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2017
 *
 * Moving files around on the file system.
 *
 */

// TOP

#if !defined(FRED_FILE_MOVING_H)
#define FRED_FILE_MOVING_H

#include <stdio.h>  // include system for windows
#include <stdlib.h> // include system for linux   (YAY!)
#include <stdarg.h>
#include <string.h>

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
i32 n = snprintf(SF_CMD, sizeof(SF_CMD), __VA_ARGS__);     \
Assert(n < sizeof(SF_CMD));                                \
SYSTEMF_PRINTF("%s\n", SF_CMD);                            \
prev_error = system(SF_CMD);                               \
if (prev_error != 0) error_state = 1;                      \
}while(0)

internal void fm_execute_in_dir(char *dir, char *str, char *args);

// Init
enum{
    DetailLevel_Basics = 0,
    DetailLevel_FileOperations = 1,
    DetailLevel_Everything = 2,
};
global i32 detail_level = 0;

internal Arena fm_init_system(i32 detail_level);

// Timing
internal u64 fm_get_time();

// Files and Folders Manipulation
internal void fm_make_folder_if_missing(Arena *arena, char *dir);
internal void fm_clear_folder(char *folder);
internal void fm_delete_file(char *file);
internal void fm_copy_file(char *file, char *newname);
internal void fm_copy_all(char *source, char *folder);
internal void fm_copy_folder(Arena *arena, char *src_dir, char *dst_dir, char *src_folder);

// File Reading and Writing
internal void fm_write_file(char *file_name, char *data, u32 size);

// Zip
internal void fm_zip(char *parent, char *folder, char *dest);

// Slash Correction
internal void fm_slash_fix(char *path);

// Memory concat helpers
internal char *fm_prepare_string_internal(Arena *arena, char *s1, ...);
#define fm_str(...) fm_prepare_string_internal(__VA_ARGS__, (void*)0)

internal char *fm_basic_string_internal(Arena *arena, char *s1, ...);
#define fm_basic_str(...) fm_basic_string_internal(__VA_ARGS__, (void*)0)

internal char **fm_prepare_list_internal(Arena *arena, char **l1, ...);
#define fm_list(...) fm_prepare_list_internal(__VA_ARGS__, (void*)0)

internal char **fm_list_one_item(Arena *arena, char *item);

// File System Navigation
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

#if COMPILER_CL

#define fm_add_to_line(line, str, ...) do{  \
snprintf(line.build_options,            \
line.build_max, "%s "str,               \
line.build_options_prev, __VA_ARGS__);  \
fm__swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#elif COMPILER_GCC | COMPILER_CLANG

#define fm_add_to_line(line, str, ...) do{                   \
snprintf(line.build_options, line.build_max, "%s " str,  \
line.build_options_prev, ##__VA_ARGS__);                 \
fm__swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#endif

// Slashes
#if OS_WINDOWS
# define SLASH "\\"
static char platform_correct_slash = '\\';
#elif OS_LINUX || OS_MAC
# define SLASH "/"
static char platform_correct_slash = '/';
#else
# error Slash not set for this platform.
#endif

// File Extensions
#if OS_WINDOWS
# define EXE ".exe"
#elif OS_LINUX || OS_MAC
# define EXE ""
#else
# error No EXE format specified for this OS
#endif

#if OS_WINDOWS
# define PDB ".pdb"
#elif OS_LINUX || OS_MAC
# define PDB ""
#else
# error No PDB format specified for this OS
#endif

#if OS_WINDOWS
# define DLL ".dll"
#elif OS_LINUX || OS_MAC
# define DLL ".so"
#else
# error No DLL format specified for this OS
#endif

#if OS_WINDOWS
# define BAT ".bat"
#elif OS_LINUX || OS_MAC
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

internal Arena
fm__init_memory(void){
    return(make_arena_malloc(MB(512), 8));
}

function b32
fm__show_details_for_file_operations(void){
    return(detail_level >= DetailLevel_FileOperations);
}

function b32
fm__show_details_for_zip_output(void){
    return(detail_level >= DetailLevel_Everything);
}

//
// Windows implementation
//

#if OS_WINDOWS

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

global u64 perf_frequency;

internal Arena
fm_init_system(i32 det){
    detail_level = det;
    LARGE_INTEGER lint;
    if (QueryPerformanceFrequency(&lint)){
        perf_frequency = lint.QuadPart;
    }
    return(fm__init_memory());
}

internal Temp_Dir
fm_pushdir(char *dir){
    Temp_Dir temp = {};
    GetCurrentDirectoryA(sizeof(temp.dir), temp.dir);
    SetCurrentDirectoryA(dir);
    return(temp);
}

internal void
fm_popdir(Temp_Dir temp){
    SetCurrentDirectoryA(temp.dir);
}

internal u64
fm_get_time(){
    u64 time = 0;
    LARGE_INTEGER lint;
    if (QueryPerformanceCounter(&lint)){
        time = lint.QuadPart;
        time = (time * 1000000) / perf_frequency;
    }
    return(time);
}

internal i32
fm_get_current_directory(char *buffer, i32 max){
    i32 result = GetCurrentDirectoryA(max, buffer);
    return(result);
}

internal void
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

internal void
fm_slash_fix(char *path){
    if (path != 0){
        for (i32 i = 0; path[i]; ++i){
            if (path[i] == '/') path[i] = '\\';
        }
    }
}

internal void
fm_make_folder_if_missing(Arena *arena, char *dir){
    char *path = fm_str(arena, dir);
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

internal void
fm_clear_folder(char *folder){
    if (fm__show_details_for_file_operations()){
        fprintf(stdout, "clearing folder %s\n", folder);
    }
    fflush(stdout);
    systemf("del /S /Q /F %s\\* > nul & rmdir /S /Q %s > nul & mkdir %s > nul", folder, folder, folder);
}

internal void
fm_delete_file(char *file){
    systemf("del %s", file);
}

internal void
fm_copy_file(char *file, char *newname){
    if (fm__show_details_for_file_operations()){
        printf("copy %s to %s\n", file, newname);
    }
    fflush(stdout);
    CopyFileA(file, newname, 0);
}

internal void
fm_copy_all(char *source, char *folder){
    if (fm__show_details_for_file_operations()){
        fprintf(stdout, "copy %s to %s\n", source, folder);
    }
    fflush(stdout);
    systemf("xcopy /s /e /y /q %s %s > nul", source, folder);
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

internal void
fm_zip(char *parent, char *folder, char *dest){
    if (fm__show_details_for_file_operations()){
        printf("zipping %s\\%s to %s\n", parent, folder, dest);
    }
    fflush(stdout);
    
    char cdir[512];
    fm_get_current_directory(cdir, sizeof(cdir));
    
    char *hide_output = " > nul >> nul";
    char *show_output = "";
    char *output_rule = hide_output;
    if (fm__show_details_for_zip_output()){
        output_rule = show_output;
    }
    
    Temp_Dir temp = fm_pushdir(parent);
    systemf("%s\\bin\\zip %s\\4ed_gobble.zip%s", cdir, cdir, output_rule);
    fm_popdir(temp);
    
    systemf("copy %s\\4ed_gobble.zip %s%s & del %s\\4ed_gobble.zip%s",
            cdir, dest, output_rule, cdir, output_rule);
}

//
// Unix implementation
//

#elif OS_LINUX || OS_MAC

#include <time.h>
#include <unistd.h>

internal Temp_Dir
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

internal void
fm_popdir(Temp_Dir temp){
    chdir(temp.dir);
}

internal Arena
fm_init_system(i32 det){
    detail_level = det;
    return(fm__init_memory());
}

internal u64
fm_get_time(){
    struct timespec spec;
    u64 result;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * (u64)(1000000)) + (spec.tv_nsec / (u64)(1000));
    return(result);
}

internal i32
fm_get_current_directory(char *buffer, i32 max){
    i32 result = 0;
    char *d = getcwd(buffer, max);
    if (d == buffer){
        result = strlen(buffer);
    }
    return(result);
}

internal void
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

internal void
fm_slash_fix(char *path){}

internal void
fm_make_folder_if_missing(Arena *arena, char *dir){
    systemf("mkdir -p %s", dir);
}

internal void
fm_clear_folder(char *folder){
    if (fm__show_details_for_file_operations()){
        fprintf(stdout, "clearing folder %s\n", folder);
    }
    fflush(stdout);
    systemf("rm -rf %s* > /dev/null", folder);
}

internal void
fm_delete_file(char *file){
    systemf("rm %s", file);
}

internal void
fm_copy_file(char *file, char *newname){
    if (fm__show_details_for_file_operations()){
        printf("copy %s to %s\n", file, newname);
    }
    fflush(stdout);
    systemf("cp %s %s", file, newname);
}

internal void
fm_copy_all(char *source, char *folder){
    if (fm__show_details_for_file_operations()){
        fprintf(stdout, "copy %s to %s\n", source, folder);
    }
    fflush(stdout);
    systemf("cp -rf %s/* %s > /dev/null", source, folder);
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

internal void
fm_zip(char *parent, char *folder, char *file){
    if (fm__show_details_for_file_operations()){
        printf("zipping %s/%s to %s\n", parent, folder, file);
    }
    fflush(stdout);
    
    char *hide_output = " > nul 2> nul";
    char *show_output = "";
    char *output_rule = hide_output;
    if (fm__show_details_for_zip_output()){
        output_rule = show_output;
    }
    
    Temp_Dir temp = fm_pushdir(parent);
    systemf("zip -r %s %s%s", file, folder, output_rule);
    fm_popdir(temp);
}

#else
#error This OS is not supported yet
#endif

internal void
fm_copy_folder(Arena *arena, char *src_dir, char *dst_dir, char *src_folder){
    Temp_Dir temp = fm_pushdir(src_dir);
    fm_make_folder_if_missing(arena, fm_str(arena, dst_dir, "/", src_folder));
    char *copy_name = fm_str(arena, dst_dir, "/", src_folder);
    fm_copy_all(src_folder, copy_name);
    fm_popdir(temp);
}

// List Helpers
internal i32
listsize(void *p, u64 item_size){
    u64 zero = 0;
    u8 *ptr = (u8*)p;
    for (;memcmp(ptr, &zero, (size_t)item_size) != 0; ptr += item_size);
    i32 size = (i32)(ptr - (u8*)p);
    return(size);
}

internal void*
fm__prepare(Arena *arena, i32 item_size, void *i1, va_list list){
    List_String_Const_char out_list = {};
    i32 size = listsize(i1, item_size);
    string_list_push(arena, &out_list, SCchar((char*)i1, size));
    void *ln = va_arg(list, void*);
    for (;ln != 0;){
        size = listsize(ln, item_size);
        string_list_push(arena, &out_list, SCchar((char*)ln, size));
        ln = va_arg(list, void*);
    }
    void *terminator = push_array_zero(arena, char, item_size);
    string_list_push(arena, &out_list, SCchar((char*)terminator, item_size));
    String_Const_char result = string_list_flatten(arena, out_list);
    return(result.str);
}

internal char*
fm_basic_string_internal(Arena *arena, char *s1, ...){
    i32 item_size = sizeof(*s1);
    va_list list;
    va_start(list, s1);
    char *result = (char*)fm__prepare(arena, item_size, s1, list);
    va_end(list);
    return(result);
}

internal char*
fm_prepare_string_internal(Arena *arena, char *s1, ...){
    i32 item_size = sizeof(*s1);
    va_list list;
    va_start(list, s1);
    char *result = (char*)fm__prepare(arena, item_size, s1, list);
    va_end(list);
    fm_slash_fix(result);
    return(result);
}

internal char**
fm_prepare_list_internal(Arena *arena, char **p1, ...){
    i32 item_size = sizeof(*p1);
    va_list list;
    va_start(list, p1);
    char **result = (char**)fm__prepare(arena, item_size, p1, list);
    va_end(list);
    return(result);
}

internal char**
fm_list_one_item(Arena *arena, char *item){
    char **result = push_array(arena, char*, 2);
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

