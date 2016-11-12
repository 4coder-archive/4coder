/*
4coder development build rule.
*/

// TOP

#include <stdio.h>  // include system for windows
#include <stdlib.h> // include system for linux   (YAY!)
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "4coder_version.h"

#define FSTRING_INLINE static
#include "internal_4coder_string.cpp"

//
// reusable
//

#define IS_64BIT

// NOTE(allen): Compiler OS cracking.
#if defined(_MSC_VER)

# define IS_CL
# define snprintf _snprintf

# if defined(_WIN32)
#  define IS_WINDOWS
#  pragma comment(lib, "Kernel32.lib")
# else
#  error This compiler/platform combo is not supported yet
# endif

#elif defined(__GNUC__) || defined(__GNUG__)

# define IS_GCC

# if defined(__gnu_linux__)
#  define IS_LINUX
# else
#  error This compiler/platform combo is not supported yet
# endif

#else
#error This compiler is not supported yet
#endif


#if defined(IS_WINDOWS)
#  define ONLY_WINDOWS(x) x
#  define ONLY_LINUX(x) (void)0
#elif defined(IS_LINUX)
#  define ONLY_WINDOWS(x) (void)0
#  define ONLY_LINUX(x) x
#else
#  define ONLY_WIN(x) (void)0
#  define ONLY_LINUX(x) (void)0
#endif


static char cmd[4096];
static int32_t error_state = 0;
static int32_t prev_error = 0;

#define systemf(...) do{                                   \
    int32_t n = snprintf(cmd, sizeof(cmd), __VA_ARGS__);   \
    assert(n < sizeof(cmd));                               \
    prev_error = system(cmd);                              \
    if (prev_error != 0) error_state = 1;                  \
}while(0)

static void     init_time_system();
static uint64_t get_time();
static int32_t  get_current_directory(char *buffer, int32_t max);
static void     execute(char *dir, char *str);

static void make_folder_if_missing(char *dir, char *folder);
static void clear_folder(char *folder);
static void copy_file(char *path, char *file, char *folder1, char *folder2, char *newname);
static void copy_all(char *source, char *tag, char *folder);
static void zip(char *parent, char *folder, char *dest);

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

#if defined(IS_64BIT)
# define WINAPI
#endif

extern "C" DWORD WINAPI GetCurrentDirectoryA(_In_  DWORD  nBufferLength, _Out_ LPTSTR lpBuffer);
extern "C" BOOL WINAPI QueryPerformanceCounter(_Out_ LARGE_INTEGER *lpPerformanceCount);
extern "C" BOOL WINAPI QueryPerformanceFrequency(_Out_ LARGE_INTEGER *lpFrequency);
extern "C" BOOL WINAPI CreateDirectoryA(_In_ LPCTSTR lpPathName, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes);
extern "C" BOOL WINAPI CopyFileA(_In_ LPCTSTR lpExistingFileName, _In_ LPCTSTR lpNewFileName, _In_ BOOL bFailIfExists);

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
    slash_fix(dir);
    
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
    slash_fix(folder);
    systemf("del /S /Q /F %s\\* & rmdir /S /Q %s & mkdir %s",
            folder, folder, folder);
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
    
    slash_fix(src);
    slash_fix(dst);
    
    CopyFileA(src, dst, 0);
}

static void
copy_all(char *source, char *tag, char *folder){
    slash_fix(source);
    slash_fix(folder);
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
    
    slash_fix(parent);
    slash_fix(dest);
    
    systemf("pushd %s & %s\\zip %s\\4tech_gobble.zip", parent, cdir, cdir);
    systemf("copy %s\\4tech_gobble.zip %s & del %s\\4tech_gobble.zip", cdir, dest, cdir);
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
    systemf("cp -rf %s/%s %s", source, tag, folder);
    }
    else{
        systemf("cp -rf %s %s", tag, folder);
    }
}

static void
zip(char *parent, char *folder, char *file){
    Temp_Dir temp = linux_pushd(parent);
    systemf("zip -r %s %s", file, folder);
    linux_popd(temp);
}

#else
#error This OS is not supported yet
#endif

#define BEGIN_TIME_SECTION() uint64_t start = get_time()
#define END_TIME_SECTION(n) uint64_t total = get_time() - start; printf("%-20s: %.2lu.%.6lu\n", (n), total/1000000, total%1000000);

//
// 4coder specific
//

#if defined(IS_WINDOWS)
#define EXE ".exe"
#elif defined(IS_LINUX)
#define EXE ""
#else
#error No EXE format specified for this OS
#endif

#if defined(IS_WINDOWS)
#define PDB ".pdb"
#elif defined(IS_LINUX)
#define PDB ""
#else
#error No EXE format specified for this OS
#endif

#if defined(IS_WINDOWS)
#define DLL ".dll"
#elif defined(IS_LINUX)
#define DLL ".so"
#else
#error No EXE format specified for this OS
#endif

#if defined(IS_WINDOWS)
#define BAT ".bat"
#elif defined(IS_LINUX)
#define BAT ".sh"
#else
#error No EXE format specified for this OS
#endif

static void
swap_ptr(char **A, char **B){
    char *a = *A;
    char *b = *B;
    *A = b;
    *B = a;
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
"..\\res\\icon.res"

static void
build_cl(uint32_t flags,
         char *code_path, char *code_file,
         char *out_path, char *out_file,
         char *exports){
    slash_fix(out_path);
    slash_fix(code_path);
    
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


// NOTE(inso): added ../code to GCC_OPTS to allow metagen to build.
// this is currently needed because it is built with cwd ../meta but includes
// 4cpp_lexer.h which is in code/ whereas metagen.cpp is in code/site

#define GCC_OPTS                             \
"-Wno-write-strings -D_GNU_SOURCE -fPIC "    \
"-fno-threadsafe-statics -pthread -I../code"

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
        // TODO(allen): Abstract this out.
#if defined(IS_LINUX)
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
#endif
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
    
    // TODO(allen): Abstract this out.
#if defined(IS_LINUX)
    Temp_Dir temp = linux_pushd(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    linux_popd(temp);
#endif
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
    slash_fix(filename);
    slash_fix(out_path);
    slash_fix(code_path);
    
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

#define D_META_DIR "../meta"
#define D_META_FSM_DIR "../meta/fsmgen"
#define D_META_GEN_DIR "../meta/metagen"
#define D_BUILD_DIR "../build"

#define D_PACK_DIR "../distributions"
#define D_PACK_DATA_DIR "../data/dist_files"
#define D_DATA_DIR "../data/test"

#define D_PACK_ALPHA_PAR_DIR "../current_dist"
#define D_PACK_SUPER_PAR_DIR "../current_dist_super"
#define D_PACK_POWER_PAR_DIR "../current_dist_power"

#define D_PACK_ALPHA_DIR D_PACK_ALPHA_PAR_DIR"/4coder"
#define D_PACK_SUPER_DIR D_PACK_SUPER_PAR_DIR"/4coder"
#define D_PACK_POWER_DIR D_PACK_POWER_PAR_DIR"/power"

static char *META_DIR = 0;
static char *META_FSM_DIR = 0;
static char *META_GEN_DIR = 0;
static char *BUILD_DIR = 0;
static char *PACK_DIR = 0;
static char *PACK_DATA_DIR = 0;
static char *DATA_DIR = 0;
static char *PACK_ALPHA_PAR_DIR = 0;
static char *PACK_SUPER_PAR_DIR = 0;
static char *PACK_POWER_PAR_DIR = 0;
static char *PACK_ALPHA_DIR = 0;
static char *PACK_SUPER_DIR = 0;
static char *PACK_POWER_DIR = 0;

static char*
get_head(String builder){
    return(builder.str + builder.size);
}

static void
init_global_strings(){
    int32_t size = 1024;
    char *base = (char*)malloc(size);
    char term_space[1] = {0};
    String builder = make_string_cap(base, 0, size);
    String term = make_string_cap(term_space, 1, 1);
    
    META_DIR = get_head(builder);
    append_sc(&builder, D_META_DIR);
    append_ss(&builder, term);
    
    META_FSM_DIR = get_head(builder);
    append_sc(&builder, D_META_FSM_DIR);
    append_ss(&builder, term);
    
    META_GEN_DIR = get_head(builder);
    append_sc(&builder, D_META_GEN_DIR);
    append_ss(&builder, term);
    
    BUILD_DIR = get_head(builder);
    append_sc(&builder, D_BUILD_DIR);
    append_ss(&builder, term);
    
    PACK_DIR = get_head(builder);
    append_sc(&builder, D_PACK_DIR);
    append_ss(&builder, term);
    
    PACK_DATA_DIR = get_head(builder);
    append_sc(&builder, D_PACK_DATA_DIR);
    append_ss(&builder, term);
    
    DATA_DIR = get_head(builder);
    append_sc(&builder, D_DATA_DIR);
    append_ss(&builder, term);
    
    PACK_ALPHA_PAR_DIR = get_head(builder);
    append_sc(&builder, D_PACK_ALPHA_PAR_DIR);
    append_ss(&builder, term);
    
    PACK_SUPER_PAR_DIR = get_head(builder);
    append_sc(&builder, D_PACK_SUPER_PAR_DIR);
    append_ss(&builder, term);
    
    PACK_POWER_PAR_DIR = get_head(builder);
    append_sc(&builder, D_PACK_POWER_PAR_DIR);
    append_ss(&builder, term);
    
    PACK_ALPHA_DIR = get_head(builder);
    append_sc(&builder, D_PACK_ALPHA_DIR);
    append_ss(&builder, term);
    
    PACK_SUPER_DIR = get_head(builder);
    append_sc(&builder, D_PACK_SUPER_DIR);
    append_ss(&builder, term);
    
    PACK_POWER_DIR = get_head(builder);
    append_sc(&builder, D_PACK_POWER_DIR);
    append_ss(&builder, term);
}

#if defined(IS_WINDOWS)
#define PLAT_LAYER "win32_4ed.cpp"
#elif defined(IS_LINUX)
#define PLAT_LAYER "linux_4ed.cpp"
#else
#error No platform layer defined for this OS.
#endif

static void
fsm_generator(char *cdir){
    {
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, cdir, "fsm_table_generator.cpp",
              META_DIR, "fsmgen", 0);
        END_TIME_SECTION("build fsm generator");
    }
    
    if (prev_error == 0){
        BEGIN_TIME_SECTION();
        execute(cdir, META_FSM_DIR);
        END_TIME_SECTION("run fsm generator");
    }
}

static void
metagen(char *cdir){
    {
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, cdir, "4ed_metagen.cpp",
              META_DIR, "metagen", 0);
        END_TIME_SECTION("build metagen");
    }
    
    if (prev_error == 0){
        BEGIN_TIME_SECTION();
        execute(cdir, META_GEN_DIR);
        END_TIME_SECTION("run metagen");
    }
}

static void
do_buildsuper(char *cdir){
    char space[1024];
    String str = make_fixed_width_string(space);
    
    BEGIN_TIME_SECTION();
    //copy_sc(&str, "../code/4coder_default_bindings.cpp");
    //terminate_with_null(&str);
    //buildsuper(cdir, BUILD_DIR, str.str);
#if defined(IS_WINDOWS)
    copy_sc(&str, "../code/internal_4coder_tests.cpp");
    terminate_with_null(&str);
    buildsuper(cdir, BUILD_DIR, str.str);
#else
    copy_sc(&str, "../code/power/4coder_experiments.cpp");
    terminate_with_null(&str);
    buildsuper(cdir, BUILD_DIR, str.str);
#endif
    //copy_sc(&str, "../code/power/4coder_casey.cpp");
    //terminate_with_null(&str);
    //buildsuper(cdir, BUILD_DIR, str.str);
    //copy_sc(&str, "../4vim/4coder_chronal.cpp");
    //terminate_with_null(&str);
    //buildsuper(cdir, BUILD_DIR, str.str);

    END_TIME_SECTION("build custom");
}

static void
build_main(char *cdir, uint32_t flags){
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | SHARED_CODE | flags, cdir, "4ed_app_target.cpp",
              BUILD_DIR, "4ed_app"DLL, "/EXPORT:app_get_functions");
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | LIBS | ICON | flags, cdir, PLAT_LAYER,
              BUILD_DIR, "4ed", 0);
        END_TIME_SECTION("build 4ed");
    }
}

static void
standard_build(char *cdir, uint32_t flags){
    fsm_generator(cdir);
    metagen(cdir);
    do_buildsuper(cdir);
    build_main(cdir, flags);
}

static void
get_4coder_dist_name(String *zip_file, int32_t OS_specific, char *tier, char *ext){
    zip_file->size = 0;
    
    append_sc(zip_file, PACK_DIR);
    append_sc(zip_file, "/");
    append_sc(zip_file, tier);
    append_sc(zip_file, "/4coder-");
    
    if (OS_specific){
#if defined(IS_WINDOWS)
        append_sc(zip_file, "win-");
#elif defined(IS_LINUX) && defined(IS_64BIT)
        append_sc(zip_file, "linux-64-");
#else
#error No OS string for zips on this OS
#endif
    }
    
    append_sc         (zip_file, "alpha");
    append_sc         (zip_file, "-");
    append_int_to_str (zip_file, MAJOR);
    append_sc         (zip_file, "-");
    append_int_to_str (zip_file, MINOR);
    append_sc         (zip_file, "-");
    append_int_to_str (zip_file, PATCH);
    if (!match_cc(tier, "alpha")){
        append_sc     (zip_file, "-");
        append_sc     (zip_file, tier);
    }
    append_sc         (zip_file, ".");
    append_sc         (zip_file, ext);
    terminate_with_null(zip_file);
}

static void
package(char *cdir){
    char str_space[1024];
    String str = make_fixed_width_string(str_space), str2 = {0};
    
    // NOTE(allen): meta
    fsm_generator(cdir);
    metagen(cdir);
    
    // NOTE(allen): alpha
    build_main(cdir, OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO);
    
    clear_folder(PACK_ALPHA_PAR_DIR);
    make_folder_if_missing(PACK_ALPHA_DIR, "3rdparty");
    make_folder_if_missing(PACK_DIR, "alpha");
    copy_file(BUILD_DIR, "4ed"EXE, PACK_ALPHA_DIR, 0, 0);
    ONLY_WINDOWS(copy_file(BUILD_DIR, "4ed"PDB, PACK_ALPHA_DIR, 0, 0));
    copy_file(BUILD_DIR, "4ed_app"DLL, PACK_ALPHA_DIR, 0, 0);
    ONLY_WINDOWS(copy_file(BUILD_DIR, "4ed_app"PDB, PACK_ALPHA_DIR, 0, 0));
    copy_all (PACK_DATA_DIR, "*", PACK_ALPHA_DIR);
    copy_file(0, "README.txt", PACK_ALPHA_DIR, 0, 0);
    copy_file(0, "TODO.txt", PACK_ALPHA_DIR, 0, 0);
    copy_file(DATA_DIR, "config.4coder", PACK_ALPHA_DIR, 0, 0);
    
    get_4coder_dist_name(&str, 1, "alpha", "zip");
    zip(PACK_ALPHA_PAR_DIR, "4coder", str.str);
    
    // NOTE(allen): super
    build_main(cdir, OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | SUPER);
    
    clear_folder(PACK_SUPER_PAR_DIR);
    make_folder_if_missing(PACK_SUPER_DIR, "3rdparty");
    make_folder_if_missing(PACK_DIR, "super");
    make_folder_if_missing(PACK_DIR, "super-docs");
    copy_file(BUILD_DIR, "4ed"EXE, PACK_SUPER_DIR, 0, 0);
    ONLY_WINDOWS(copy_file(BUILD_DIR, "4ed"PDB, PACK_SUPER_DIR, 0, 0));
    copy_file(BUILD_DIR, "4ed_app"DLL, PACK_SUPER_DIR, 0, 0);
    ONLY_WINDOWS(copy_file(BUILD_DIR, "4ed_app"PDB, PACK_SUPER_DIR, 0, 0));
    copy_all (PACK_DATA_DIR, "*", PACK_SUPER_DIR);
    copy_file(0, "README.txt", PACK_SUPER_DIR, 0, 0);
    copy_file(0, "TODO.txt", PACK_SUPER_DIR, 0, 0);
    copy_file(DATA_DIR, "config.4coder", PACK_SUPER_DIR, 0, 0);
    
    copy_all (0, "4coder_*.h", PACK_SUPER_DIR);
    copy_all (0, "4coder_*.cpp", PACK_SUPER_DIR);
    copy_all (0, "4cpp_*.h", PACK_SUPER_DIR);
    copy_all (0, "4cpp_*.c", PACK_SUPER_DIR);
    copy_file(0, "buildsuper"BAT, PACK_SUPER_DIR, 0, 0);
    
    get_4coder_dist_name(&str, 0, "API", "html");
    str2 = front_of_directory(str);
    copy_file(0, "4coder_API.html", PACK_DIR, "super-docs", str2.str);
    
    get_4coder_dist_name(&str, 1, "super", "zip");
    zip(PACK_SUPER_PAR_DIR, "4coder", str.str);
    
    // NOTE(allen): power
    clear_folder(PACK_POWER_PAR_DIR);
    make_folder_if_missing(PACK_POWER_DIR, 0);
    make_folder_if_missing(PACK_DIR, "power");
    copy_all("power", "*", PACK_POWER_DIR);
    
    get_4coder_dist_name(&str, 0, "power", "zip");
    zip(PACK_POWER_PAR_DIR, "power", str.str);
}

#if defined(DEV_BUILD)

int main(int argc, char **argv){
    init_time_system();
    init_global_strings();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    int32_t n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
    standard_build(cdir, DEBUG_INFO | SUPER | INTERNAL);
    
    return(error_state);
}

#elif defined(PACKAGE)

int main(int argc, char **argv){
    init_time_system();
    init_global_strings();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    int32_t n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
    package(cdir);
    
    return(error_state);
}

#else
#error No build type specified
#endif

// BOTTOM
