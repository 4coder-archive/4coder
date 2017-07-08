/*
4tech_file_moving.h - Code for moving files around on the file system.
By Allen Webster
21.01.2017 (dd.mm.yyyy)
*/

// TOP

#if !defined(FTECH_FILE_MOVING_H)
#define FTECH_FILE_MOVING_H

#include "../4ed_os_comp_cracking.h"

#include <stdio.h>  // include system for windows
#include <stdlib.h> // include system for linux   (YAY!)
#include <stdarg.h>

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
    int32_t n = snprintf(SF_CMD, sizeof(SF_CMD), __VA_ARGS__); \
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
#define BEGIN_TIME_SECTION() uint64_t start = fm_get_time()
#define END_TIME_SECTION(n) uint64_t total = fm_get_time() - start; printf("%-20s: %.2llu.%.6llu\n", (n), LLU_CAST(total/1000000), LLU_CAST(total%1000000));

// Files and Folders Manipulation
internal void fm_make_folder_if_missing(char *dir, char *folder);
internal void fm_clear_folder(char *folder);
internal void fm_delete_file(char *file);
internal void fm_copy_file(char *path, char *file, char *folder1, char *folder2, char *newname);
internal void fm_copy_all(char *source, char *tag, char *folder);
internal void fm_copy_folder(char *dst_dir, char *src_folder);

// Zip
internal void fm_zip(char *parent, char *folder, char *dest);

// File Name Manipulation
internal void fm_slash_fix(char *path);

internal char *fm_prepare_string_internal(char *s1, ...);
#define fm_prepare_string(...) fm_prepare_string_internal(__VA_ARGS__, 0)

typedef umem Temp_Memory;
internal Temp_Memory fm_begin_temp();
internal void fm_end_temp(Temp_Memory temp);

// File System Navigation
internal i32  fm_get_current_directory(char *buffer, i32 max);

typedef struct Temp_Dir{
    char dir[512];
} Temp_Dir;

internal Temp_Dir fm_pushdir(char *dir);
internal void fm_popdir(Temp_Dir temp);

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
#define EXE ".exe"
#elif defined(IS_LINUX) || defined(IS_MAC)
#define EXE ""
#else
#error No EXE format specified for this OS
#endif

#if defined(IS_WINDOWS)
#define PDB ".pdb"
#elif defined(IS_LINUX) || defined(IS_MAC)
#define PDB ""
#else
#error No PDB format specified for this OS
#endif

#if defined(IS_WINDOWS)
#define DLL ".dll"
#elif defined(IS_LINUX) || defined(IS_MAC)
#define DLL ".so"
#else
#error No DLL format specified for this OS
#endif

#if defined(IS_WINDOWS)
#define BAT ".bat"
#elif defined(IS_LINUX) || defined(IS_MAC)
#define BAT ".sh"
#else
#error No BAT format specified for this OS
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
    fm_arena_max = MB(16);
    fm_arena_memory = (char*)malloc(fm_arena_max);
}

internal Temp_Memory
fm_begin_temp(){
    return(fm_arena_pos);
}

internal void
fm_end_temp(Temp_Memory temp){
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

#if defined(IS_WINDOWS)

typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef int64_t  LONGLONG;
typedef char*    LPTSTR;
typedef char*    LPCTSTR;
typedef int32_t  BOOL;
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

#define WINAPI

extern "C"{
    DWORD WINAPI GetCurrentDirectoryA(_In_  DWORD  nBufferLength, _Out_ LPTSTR lpBuffer);
    BOOL WINAPI SetCurrentDirectoryA(_In_ LPCTSTR lpPathName);
    
    BOOL WINAPI QueryPerformanceCounter(_Out_ LARGE_INTEGER *lpPerformanceCount);
    
    BOOL WINAPI QueryPerformanceFrequency(_Out_ LARGE_INTEGER *lpFrequency);
    
    BOOL WINAPI CreateDirectoryA(_In_ LPCTSTR lpPathName, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    
    BOOL WINAPI CopyFileA(_In_ LPCTSTR lpExistingFileName, _In_ LPCTSTR lpNewFileName, _In_ BOOL bFailIfExists);
}

static uint64_t perf_frequency;

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

static uint64_t
fm_get_time(){
    uint64_t time = 0;
    LARGE_INTEGER lint;
    if (QueryPerformanceCounter(&lint)){
        time = lint.QuadPart;
        time = (time * 1000000) / perf_frequency;
    }
    return(time);
}

static int32_t
fm_get_current_directory(char *buffer, int32_t max){
    int32_t result = GetCurrentDirectoryA(max, buffer);
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
    if (path){
        for (int32_t i = 0; path[i]; ++i){
            if (path[i] == '/') path[i] = '\\';
        }
    }
}

static void
fm_make_folder_if_missing(char *dir, char *folder){
    char space[1024];
    String path = make_fixed_width_string(space);
    append_sc(&path, dir);
    if (folder){
        append_sc(&path, "\\");
        append_sc(&path, folder);
    }
    terminate_with_null(&path);
    
    char *p = path.str;
    for (; *p; ++p){
        if (*p == '\\'){
            *p = 0;
            CreateDirectoryA(path.str, 0);
            *p = '\\';
        }
    }
    CreateDirectoryA(path.str, 0);
}

static void
fm_clear_folder(char *folder){
    systemf("del /S /Q /F %s\\* & rmdir /S /Q %s & mkdir %s", folder, folder, folder);
}

static void
fm_delete_file(char *file){
    systemf("del %s", file);
}

static void
fm_copy_file(char *path, char *file, char *folder1, char *folder2, char *newname){
    char src[256], dst[256];
    String b = make_fixed_width_string(src);
    if (path){
        append_sc(&b, path);
        append_sc(&b, "\\");
    }
    append_sc(&b, file);
    terminate_with_null(&b);
    
    b = make_fixed_width_string(dst);
    append_sc(&b, folder1);
    append_sc(&b, "\\");
    if (folder2){
        append_sc(&b, folder2);
        append_sc(&b, "\\");
    }
    if (newname){
        append_sc(&b, newname);
    }
    else{
        append_sc(&b, file);
    }
    terminate_with_null(&b);
    
    CopyFileA(src, dst, 0);
}

static void
fm_copy_all(char *source, char *tag, char *folder){
    if (source){
        systemf("copy %s\\%s %s\\*", source, tag, folder);
    }
    else{
        systemf("copy %s %s\\*", tag, folder);
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

#elif defined(IS_LINUX) || defined(IS_MAC)

#include <time.h>
#include <unistd.h>

static Temp_Dir
fm_pushdir(char *dir){
    Temp_Dir temp;
    char *result = getcwd(temp.dir, sizeof(temp.dir));
    int32_t chresult = chdir(dir);
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
    // NOTE(allen): do nothing
}

static uint64_t
fm_get_time(){
    struct timespec spec;
    uint64_t result;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * (uint64_t)(1000000)) + (spec.tv_nsec / (uint64_t)(1000));
    return(result);
}

static int32_t
fm_get_current_directory(char *buffer, int32_t max){
    int32_t result = 0;
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
fm_make_folder_if_missing(char *dir, char *folder){
    if (folder){
        systemf("mkdir -p %s/%s", dir, folder);
    }
    else{
        systemf("mkdir -p %s", dir);
    }
}

static void
fm_clear_folder(char *folder){
    systemf("rm -rf %s*", folder);
}

static void
fm_delete_file(char *file){
    systemf("rm %s", file);
}

static void
fm_copy_file(char *path, char *file, char *folder1, char *folder2, char *newname){
    if (!newname){
        newname = file;
    }
    
    if (path){
        if (folder2){
            systemf("cp %s/%s %s/%s/%s", path, file, folder1, folder2, newname);
        }
        else{
            systemf("cp %s/%s %s/%s", path, file, folder1, newname);
        }
    }
    else{
        if (folder2){
            systemf("cp %s %s/%s/%s", file, folder1, folder2, newname);
        }
        else{
            systemf("cp %s %s/%s", file, folder1, newname);
        }
    }
}

static void
fm_copy_all(char *source, char *tag, char *folder){
    if (source){
        systemf("cp -f %s/%s %s", source, tag, folder);
    }
    else{
        systemf("cp -f %s %s", tag, folder);
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
fm_copy_folder(char *dst_dir, char *src_folder){
    fm_make_folder_if_missing(dst_dir, src_folder);
    
    char space[256];
    String copy_name = make_fixed_width_string(space);
    append_sc(&copy_name, dst_dir);
    append_s_char(&copy_name, platform_correct_slash);
    append_sc(&copy_name, src_folder);
    terminate_with_null(&copy_name);
    
    fm_copy_all(src_folder, "*", copy_name.str);
}

internal char*
fm_prepare_string_internal(char *s1, ...){
    i32 len = str_size(s1);
    char *result = (char*)fm__push(len);
    memcpy(result, s1, len);
    
    va_list list;
    va_start(list, s1);
    for (;;){
        char *sn = va_arg(list, char*);
        if (sn == 0){
            break;
        }
        else{
            len = str_size(sn);
            char *new_str = (char*)fm__push(len);
            memcpy(new_str, sn, len);
        }
    }
    va_end(list);
    
    char *terminator = (char*)fm__push(1);
    *terminator = 0;
    
    fm_slash_fix(result);
    return(result);
}

#endif

// BOTTOM

