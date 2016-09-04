/*
4coder development build rule.
*/

// TOP

#include <stdio.h>  // include system for windows
#include <stdlib.h> // include system for linux   (YAY!)
#include <stdint.h>
#include <assert.h>
#include <string.h>

//
// reusable
//

// NOTE(allen): Compiler OS cracking.
#if defined(_MSC_VER)

#define IS_CL
#define snprintf _snprintf

#if defined(_WIN32)
#  define IS_WINDOWS
#  pragma comment(lib, "Kernel32.lib")
#else
#  error This compiler/platform combo is not supported yet
#endif

#elif defined(__GNUC__) || defined(__GNUG__)

#define IS_GCC

#if defined(__gnu_linux__)
#  define IS_LINUX
#else
#  error This compiler/platform combo is not supported yet
#endif

#else
#error This compiler is not supported yet
#endif

static char cmd[1024];
static int32_t error_state = 0;

#define systemf(...) do{                                   \
    int32_t n = snprintf(cmd, sizeof(cmd), __VA_ARGS__);   \
    assert(n < sizeof(cmd));                               \
    if (system(cmd) != 0) error_state = 1;                 \
}while(0)


static void     init_time_system();
static uint64_t get_time();
static int32_t  get_current_directory(char *buffer, int32_t max);
static void     execute(char *dir, char *str);

#if defined(IS_WINDOWS)

typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef int64_t  LONGLONG;
typedef char*    LPTSTR;
typedef int32_t  BOOL;
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

#elif defined(IS_LINUX)

#include <time.h>
#include <unistd.h>

typedef struct Temp_Dir{
    char dir[512];
} Temp_Dir;

static Temp_Dir
linux_pushd(char *dir){
    Temp_Dir temp;
    char *result = getcwd(temp.dir, sizeof(temp.dir));
    int32_t chresult = chdir(dir);
    if (result == 0 || chresult != 0){
        printf("trying pushd %s\n", dir);
        assert(result != 0);
        assert(chresult == 0);
    }
    return(temp);
}

static void
linux_popd(Temp_Dir temp){
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
execute(char *dir, char *str){
    if (dir){
        Temp_Dir temp = linux_pushd(dir);
        systemf("%s", str);
        linux_popd(temp);
    }
    else{
        systemf("%s", str);
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


static void
swap_ptr(char **A, char **B){
    char *a = *A;
    char *b = *B;
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
    OPTIMIZATION = 0x100,
    KEEP_ASSERT = 0x200
};


#define BUILD_LINE_MAX 4096
typedef struct Build_Line{
    char build_optionsA[BUILD_LINE_MAX];
    char build_optionsB[BUILD_LINE_MAX];
    char *build_options;
    char *build_options_prev;
    int32_t build_max;
} Build_Line;

static void
init_build_line(Build_Line *line){
    line->build_options = line->build_optionsA;
    line->build_options_prev = line->build_optionsB;
    line->build_optionsA[0] = 0;
    line->build_optionsB[0] = 0;
    line->build_max = BUILD_LINE_MAX;
}

#if defined(IS_CL)

#define build_ap(line, str, ...) do{        \
    snprintf(line.build_options,            \
    line.build_max, "%s "str,               \
    line.build_options_prev, __VA_ARGS__);  \
    swap_ptr(&line.build_options,           \
    &line.build_options_prev);              \
}while(0)

#elif defined(IS_GCC)

#define build_ap(line, str, ...) do{        \
    snprintf(line.build_options,            \
    line.build_max, "%s "str,               \
    line.build_options_prev, ##__VA_ARGS__);\
    swap_ptr(&line.build_options,           \
    &line.build_options_prev);              \
}while(0)

#endif


#define CL_OPTS                                  \
"/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 "   \
"/wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX "   \
"/GR- /EHa- /nologo /FC"

#define CL_INCLUDES \
"/I..\\foreign /I..\\foreign\\freetype2"

#define CL_LIBS                                  \
"user32.lib winmm.lib gdi32.lib opengl32.lib "   \
"..\\foreign\\freetype.lib"

#define CL_ICON \
"..\\foreign\\freetype.lib"

static void
build_cl(uint32_t flags,
         char *code_path, char *code_file,
         char *out_path, char *out_file,
         char *exports){
    win32_slash_fix(out_path);
    win32_slash_fix(code_path);
    
    Build_Line line;
    init_build_line(&line);
    
    if (flags & OPTS){
        build_ap(line, CL_OPTS);
    }
    
    if (flags & INCLUDES){
        build_ap(line, CL_INCLUDES);
    }
    
    if (flags & LIBS){
        build_ap(line, CL_LIBS);
    }
    
    if (flags & ICON){
        build_ap(line, CL_ICON);
    }
    
    if (flags & DEBUG_INFO){
        build_ap(line, "/Zi");
    }
    
    if (flags & OPTIMIZATION){
        build_ap(line, "/O2");
    }
    
    if (flags & SHARED_CODE){
        build_ap(line, "/LD");
    }
    
    if (flags & SUPER){
        build_ap(line, "/DFRED_SUPER");
    }
    
    if (flags & INTERNAL){
        build_ap(line, "/DFRED_INTERNAL");
    }
    
    if (flags & KEEP_ASSERT){
        build_ap(line, "/DFRED_KEEP_ASSERT");
    }
    
    swap_ptr(&line.build_options, &line.build_options_prev);
    
    char link_options[1024];
    if (flags & SHARED_CODE){
        assert(exports);
        snprintf(link_options, sizeof(link_options), "/OPT:REF %s", exports);
    }
    else{
        snprintf(link_options, sizeof(link_options), "/NODEFAULTLIB:library");
    }
    
    systemf("pushd %s & cl %s %s\\%s /Fe%s /link /DEBUG /INCREMENTAL:NO %s",
            out_path, line.build_options, code_path, code_file, out_file, link_options);
}



#define GCC_OPTS                             \
"-Wno-write-strings -D_GNU_SOURCE -fPIC "    \
"-fno-threadsafe-statics -pthread"

#define GCC_INCLUDES \
"-I../foreign"

#define GCC_LIBS                               \
"-L/usr/local/lib -lX11 -lpthread -lm -lrt "   \
"-lGL -ldl -lXfixes -lfreetype -lfontconfig"

static void
build_gcc(uint32_t flags,
          char *code_path, char *code_file,
          char *out_path, char *out_file,
          char *exports){
    Build_Line line;
    init_build_line(&line);
    
    if (flags & OPTS){
        build_ap(line, GCC_OPTS);
    }
    
    if (flags & INCLUDES){
        int32_t size = 0;
        char freetype_include[512];
        FILE *file = popen("pkg-config --cflags freetype2", "r");
        if (file != 0){
            fgets(freetype_include, sizeof(freetype_include), file);
            size = strlen(freetype_include);
            freetype_include[size-1] = 0;
            pclose(file);
        }
        
        build_ap(line, GCC_INCLUDES" %s", freetype_include);
    }
    
    if (flags & DEBUG_INFO){
        build_ap(line, "-g -O0");
    }
    
    if (flags & OPTIMIZATION){
        build_ap(line, "-O3");
    }
    
    if (flags & SHARED_CODE){
        build_ap(line, "-shared");
    }
    
    if (flags & SUPER){
        build_ap(line, "-DFRED_SUPER");
    }
    
    if (flags & INTERNAL){
        build_ap(line, "-DFRED_INTERNAL");
    }
    
    if (flags & KEEP_ASSERT){
        build_ap(line, "-DFRED_KEEP_ASSERT");
    }
    
    build_ap(line, "%s/%s", code_path, code_file);
    
    if (flags & LIBS){
        build_ap(line, GCC_LIBS);
    }
    
    swap_ptr(&line.build_options, &line.build_options_prev);
    
    Temp_Dir temp = linux_pushd(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    linux_popd(temp);
}

static void
build(uint32_t flags,
      char *code_path, char *code_file,
      char *out_path, char *out_file,
      char *exports){
#if defined(IS_CL)
    build_cl(flags, code_path, code_file, out_path, out_file, exports);
#elif defined(IS_GCC)
    build_gcc(flags, code_path, code_file, out_path, out_file, exports);
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
    
#elif defined(IS_GCC)
    
    Temp_Dir temp = linux_pushd(out_path);
    
    systemf("\"%s/buildsuper.sh\" %s",
            code_path, filename);
    
    linux_popd(temp);
    
#else
#error The build rule for this compiler is not ready
#endif
}

#define META_DIR "../meta"
#define BUILD_DIR "../build"

#if defined(IS_WINDOWS)
#define PLAT_LAYER "win32_4ed.cpp"
#elif defined(IS_LINUX)
#define PLAT_LAYER "linux_4ed.cpp"
#else
#error No platform layer defined for this OS.
#endif

static void
standard_build(char *cdir, uint32_t flags){
#if 1
    {
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, cdir, "fsm_table_generator.cpp",
              META_DIR, "fsmgen", 0);
        END_TIME_SECTION("build fsm generator");
    }
    
    {
        BEGIN_TIME_SECTION();
        execute(cdir, META_DIR"/fsmgen");
        END_TIME_SECTION("run fsm generator");
    }
#endif
    
#if 1
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
#endif
    
#if 1
    {
        BEGIN_TIME_SECTION();
        //buildsuper(cdir, BUILD_DIR, "../code/4coder_default_bindings.cpp");
#if IS_WINDOWS
        buildsuper(cdir, BUILD_DIR, "../code/internal_4coder_tests.cpp");
#else
        buildsuper(cdir, BUILD_DIR, "../code/power/4coder_experiments.cpp");
#endif
        //buildsuper(cdir, BUILD_DIR, "../code/power/4coder_casey.cpp");
        //buildsuper(cdir, BUILD_DIR, "../4vim/4coder_chronal.cpp");
        END_TIME_SECTION("build custom");
    }
#endif
    
#if 1
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | SHARED_CODE | flags, cdir, "4ed_app_target.cpp",
              BUILD_DIR, "4ed_app", "/EXPORT:app_get_functions");
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | LIBS | ICON | flags, cdir, PLAT_LAYER,
              BUILD_DIR, "4ed", 0);
        END_TIME_SECTION("build 4ed");
    }
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
    
    standard_build(cdir, DEBUG_INFO | SUPER | INTERNAL);
    
    return(error_state);
}


#elif defined(PACKAGE)



#else
#error No build type specified
#endif

// BOTTOM

