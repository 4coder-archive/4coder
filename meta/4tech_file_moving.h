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

#if defined(IS_WINDOWS)
#  define ONLY_WINDOWS(x) x
#  define ONLY_LINUX(x) (void)0

#define SLASH "\\"
static char platform_correct_slash = '\\';

#elif defined(IS_LINUX)
#  define ONLY_WINDOWS(x) (void)0
#  define ONLY_LINUX(x) x

#define SLASH "/"
static char platform_correct_slash = '/';

#elif defined(IS_MAC)
#  define ONLY_WINDOWS(x) (void)0
#  define ONLY_LINUX(x) (void)0

#define SLASH "/"
static char platform_correct_slash = '/';

#else
#  define ONLY_WINDOWS(x) (void)0
#  define ONLY_LINUX(x) (void)0

#define SLASH "/"
static char platform_correct_slash = '/';

#endif


static char SF_CMD[4096];
static i32 error_state = 0;
static i32 prev_error = 0;

#define systemf(...) do{                                       \
    int32_t n = snprintf(SF_CMD, sizeof(SF_CMD), __VA_ARGS__); \
    AllowLocal(n);                                             \
    Assert(n < sizeof(SF_CMD));                                \
    /** printf("%s\n", SF_CMD); /**/                          \
    prev_error = system(SF_CMD);                               \
    if (prev_error != 0) error_state = 1;                      \
}while(0)

static void init_time_system();
static u64  get_time();
static i32  get_current_directory(char *buffer, i32 max);
static void execute_in_dir(char *dir, char *str, char *args);

static void make_folder_if_missing(char *dir, char *folder);
static void clear_folder(char *folder);
static void delete_file(char *file);
static void copy_file(char *path, char *file, char *folder1, char *folder2, char *newname);
static void copy_all(char *source, char *tag, char *folder);
static void zip(char *parent, char *folder, char *dest);

static void slash_fix(char *path);
#define DECL_STR(n,s) char n[] = s; slash_fix(n)

typedef struct Temp_Dir{
    char dir[512];
} Temp_Dir;

static Temp_Dir pushdir(char *dir);
static void popdir(Temp_Dir temp);

#endif

#if defined(FTECH_FILE_MOVING_IMPLEMENTATION) && !defined(FTECH_FILE_MOVING_IMPL_GUARD)
#define FTECH_FILE_MOVING_IMPL_GUARD

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

static Temp_Dir
pushdir(char *dir){
    Temp_Dir temp = {0};
    GetCurrentDirectoryA(sizeof(temp.dir), temp.dir);
    SetCurrentDirectoryA(dir);
    return(temp);
}

static void
popdir(Temp_Dir temp){
    SetCurrentDirectoryA(temp.dir);
}

static void
init_time_system(){
    LARGE_INTEGER lint;
    if (QueryPerformanceFrequency(&lint)){
        perf_frequency = lint.QuadPart;
    }
}

static uint64_t
get_time(){
    uint64_t time = 0;
    LARGE_INTEGER lint;
    if (QueryPerformanceCounter(&lint)){
        time = lint.QuadPart;
        time = (time * 1000000) / perf_frequency;
    }
    return(time);
}

static int32_t
get_current_directory(char *buffer, int32_t max){
    int32_t result = GetCurrentDirectoryA(max, buffer);
    return(result);
}

static void
execute_in_dir(char *dir, char *str, char *args){
    if (dir){
        Temp_Dir temp = pushdir(dir);
        if (args){
            systemf("call \"%s\" %s", str, args);
        }
        else{
            systemf("call \"%s\"", str);
        }
        popdir(temp);
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
slash_fix(char *path){
    if (path){
        for (int32_t i = 0; path[i]; ++i){
            if (path[i] == '/') path[i] = '\\';
        }
    }
}

static void
make_folder_if_missing(char *dir, char *folder){
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
clear_folder(char *folder){
    systemf("del /S /Q /F %s\\* & rmdir /S /Q %s & mkdir %s", folder, folder, folder);
}

static void
delete_file(char *file){
    systemf("del %s", file);
}

static void
copy_file(char *path, char *file, char *folder1, char *folder2, char *newname){
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
copy_all(char *source, char *tag, char *folder){
    if (source){
        systemf("copy %s\\%s %s\\*", source, tag, folder);
    }
    else{
        systemf("copy %s %s\\*", tag, folder);
    }
}

static void
zip(char *parent, char *folder, char *dest){
    char cdir[512];
    get_current_directory(cdir, sizeof(cdir));
    
    Temp_Dir temp = pushdir(parent);
    systemf("%s\\zip %s\\4tech_gobble.zip", cdir, cdir);
    popdir(temp);
    
    systemf("copy %s\\4tech_gobble.zip %s & del %s\\4tech_gobble.zip", cdir, dest, cdir);
}

#elif defined(IS_LINUX) || defined(IS_MAC)

#include <time.h>
#include <unistd.h>

static Temp_Dir
pushdir(char *dir){
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
popdir(Temp_Dir temp){
    chdir(temp.dir);
}

static void
init_time_system(){
    // NOTE(allen): do nothing
}

static uint64_t
get_time(){
    struct timespec spec;
    uint64_t result;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * (uint64_t)(1000000)) + (spec.tv_nsec / (uint64_t)(1000));
    return(result);
}

static int32_t
get_current_directory(char *buffer, int32_t max){
    int32_t result = 0;
    char *d = getcwd(buffer, max);
    if (d == buffer){
        result = strlen(buffer);
    }
    return(result);
}

static void
execute_in_dir(char *dir, char *str, char *args){
    if (dir){
        if (args){
            Temp_Dir temp = pushdir(dir);
            systemf("%s %s", str, args);
            popdir(temp);
        }
        else{
            Temp_Dir temp = pushdir(dir);
            systemf("%s", str);
            popdir(temp);
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
slash_fix(char *path){}

static void
make_folder_if_missing(char *dir, char *folder){
    if (folder){
        systemf("mkdir -p %s/%s", dir, folder);
    }
    else{
        systemf("mkdir -p %s", dir);
    }
}

static void
clear_folder(char *folder){
    systemf("rm -rf %s*", folder);
}

static void
delete_file(char *file){
    systemf("rm %s", file);
}

static void
copy_file(char *path, char *file, char *folder1, char *folder2, char *newname){
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
copy_all(char *source, char *tag, char *folder){
    if (source){
        systemf("cp -f %s/%s %s", source, tag, folder);
    }
    else{
        systemf("cp -f %s %s", tag, folder);
    }
}

static void
zip(char *parent, char *folder, char *file){
    Temp_Dir temp = pushdir(parent);
    printf("PARENT DIR: %s\n", parent);
    systemf("zip -r %s %s", file, folder);

    popdir(temp);
}

#else
#error This OS is not supported yet
#endif

#endif

// BOTTOM

