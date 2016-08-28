/*

4coder development build rule.

*/

// TOP

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

//
// reusable
//

#define CL_OPTS \
"/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX "\
"/GR- /EHa- /nologo /FC"

#if defined(_MSC_VER)

#define IS_CL
#define snprintf _snprintf

#if defined(_WIN32)
#  define IS_WINDOWS
#  pragma comment(lib, "Kernel32.lib")
#endif

#else
#error This compiler is not supported yet
#endif

static char cmd[1024];
static int error_state = 0;

#define systemf(...) do{\
    int32_t n = snprintf(cmd, sizeof(cmd), __VA_ARGS__);\
    assert(n < sizeof(cmd));\
    if (system(cmd) != 0) error_state = 1;\
}while(0)


#if defined(IS_WINDOWS)

typedef uint32_t DWORD;
typedef int32_t LONG;
typedef int64_t LONGLONG;
typedef char* LPTSTR;
typedef int32_t BOOL;
typedef union _LARGE_INTEGER {
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

DWORD GetCurrentDirectoryA(_In_  DWORD  nBufferLength, _Out_ LPTSTR lpBuffer);
BOOL QueryPerformanceCounter(_Out_ LARGE_INTEGER *lpPerformanceCount);
BOOL QueryPerformanceFrequency(_Out_ LARGE_INTEGER *lpFrequency);

static uint64_t perf_frequency;

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
execute(char *dir, char *str){
    if (dir){
        systemf("pushd %s & call \"%s\"", dir, str);
    }
    else{
        systemf("call \"%s\"", str);
    }
}

#else
#error This OS is not supported yet
#endif

#define BEGIN_TIME_SECTION() uint64_t start = get_time()
#define END_TIME_SECTION(n) uint64_t total = get_time() - start; printf("%-20s: %.2lu.%.6lu\n", (n), total/1000000, total%1000000);

//
// 4coder specific
//

#define CL_INCLUDES \
"/I..\\foreign /I..\\foreign\\freetype2"

#define CL_LIBS \
"user32.lib winmm.lib gdi32.lib opengl32.lib ..\\foreign\\freetype.lib"

#define CL_ICON \
"..\\foreign\\freetype.lib"

static void
swap_ptr(void **A, void **B){
    void *a = *A;
    void *b = *B;
    *A = b;
    *B = a;
}

static void
win32_slash_fix(char *path){
    for (int32_t i = 0; path[i]; ++i){
        if (path[i] == '/') path[i] = '\\';
    }
}

enum{
    OPTS = 0x1,
    INCLUDES = 0x2,
    LIBS = 0x4,
    ICON = 0x8,
    SHARED_CODE = 0x10,
    DEBUG_INFO = 0x20,
    SUPER = 0x40,
    INTERNAL = 0x80,
    OPTIMIZATION = 0x100
};


static void
build_cl(uint32_t flags,
         char *code_path, char *code_file,
         char *out_path, char *out_file,
         char *exports){
    win32_slash_fix(out_path);
    win32_slash_fix(code_path);
    
    char link_options[1024];
    if (flags & SHARED_CODE){
        assert(exports);
        snprintf(link_options, sizeof(link_options), "/OPT:REF %s", exports);
    }
    else{
        snprintf(link_options, sizeof(link_options), "/NODEFAULTLIB:library");
    }
    
    char build_optionsA[4096];
    char build_optionsB[4096];
    char *build_options =      build_optionsA;
    char *build_options_prev = build_optionsB;
    int32_t build_max = sizeof(build_optionsA);
    
    build_optionsA[0] = 0;
    build_optionsB[0] = 0;
    
    if (flags & OPTS){
        snprintf(build_options, build_max, "%s "CL_OPTS, build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & INCLUDES){
        snprintf(build_options, build_max, "%s "CL_INCLUDES, build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & LIBS){
        snprintf(build_options, build_max, "%s "CL_LIBS, build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & ICON){
        snprintf(build_options, build_max, "%s "CL_ICON, build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & DEBUG_INFO){
        snprintf(build_options, build_max, "%s /Zi", build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & OPTIMIZATION){
        snprintf(build_options, build_max, "%s /O2", build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & SHARED_CODE){
        snprintf(build_options, build_max, "%s /LD", build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & SUPER){
        snprintf(build_options, build_max, "%s /DFRED_SUPER", build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    if (flags & INTERNAL){
        snprintf(build_options, build_max, "%s /DFRED_INTERNAL", build_options_prev);
        swap_ptr(&build_options, &build_options_prev);
    }
    
    swap_ptr(&build_options, &build_options_prev);
    
    systemf("pushd %s & cl %s %s\\%s /Fe%s /link /DEBUG /INCREMENTAL:NO %s",
            out_path, build_options, code_path, code_file, out_file, link_options);
}

static void
build(uint32_t flags,
      char *code_path, char *code_file,
      char *out_path, char *out_file,
      char *exports){
#if defined(IS_CL)
    build_cl(flags, code_path, code_file, out_path, out_file, exports);
#else
#error The build rule for this compiler is not ready
#endif
}

static void
buildsuper(char *code_path, char *out_path, char *filename){
#if defined(IS_CL)
    win32_slash_fix(filename);
    win32_slash_fix(out_path);
    win32_slash_fix(code_path);
    
    systemf("pushd %s & call \"%s\\buildsuper.bat\" %s",
            out_path, code_path, filename);
    
#else
#error The build rule for this compiler is not ready
#endif
}

#if defined(DEV_BUILD)

int main(int argc, char **argv){
    init_time_system();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    int32_t n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
#define META_DIR "../meta"
#define BUILD_DIR "../build"
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, cdir, "4ed_metagen.cpp",
              META_DIR, "metagen", 0);
        END_TIME_SECTION("build metagen");
    }
    
    {
        BEGIN_TIME_SECTION();
        execute(cdir, META_DIR"/metagen");
        END_TIME_SECTION("run metagen");
    }
    
    {
        BEGIN_TIME_SECTION();
        //buildsuper(cdir, BUILD_DIR, "../code/4coder_default_bindings.cpp");
        buildsuper(cdir, BUILD_DIR, "../code/internal_4coder_tests.cpp");
        //buildsuper(cdir, BUILD_DIR, "../code/power/4coder_casey.cpp");
        //buildsuper(cdir, BUILD_DIR, "../4vim/4coder_chronal.cpp");
        END_TIME_SECTION("build custom");
    }
    
    uint32_t flags = DEBUG_INFO | SUPER | INTERNAL;
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | SHARED_CODE | flags, cdir, "4ed_app_target.cpp",
              BUILD_DIR, "4ed_app", "/EXPORT:app_get_functions");
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | LIBS | ICON | flags, cdir, "win32_4ed.cpp",
              BUILD_DIR, "4ed", 0);
        END_TIME_SECTION("build 4ed");
    }
    
    return(error_state);
}

#elif defined(PACKAGE)



#endif

// BOTTOM

