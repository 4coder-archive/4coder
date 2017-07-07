/*
4coder development build rule.
*/

// TOP

//#define FM_PRINT_COMMANDS

#include "../4ed_defines.h"
#include "4ed_file_moving.h"

#include <assert.h>
#include <string.h>

#include "../4coder_API/version.h"

#define FSTRING_IMPLEMENTATION
#include "../4coder_lib/4coder_string.h"

//
// reusable
//

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

static void
swap_ptr(char **A, char **B){
    char *a = *A;
    char *b = *B;
    *A = b;
    *B = a;
}

#define BUILD_LINE_MAX 4096
typedef struct Build_Line{
    char build_optionsA[BUILD_LINE_MAX];
    char build_optionsB[BUILD_LINE_MAX];
    char *build_options;
    char *build_options_prev;
    i32 build_max;
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
    swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#elif defined(IS_GCC)

#define build_ap(line, str, ...) do{                         \
    snprintf(line.build_options, line.build_max, "%s "str,   \
    line.build_options_prev, ##__VA_ARGS__);        \
    swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#endif

//
// 4coder specific
//

enum{
    Platform_Windows,
    Platform_Linux,
    Platform_Mac,
    //
    Platform_COUNT,
};

enum{
    Compiler_CL,
    Compiler_GCC,
    //
    Compiler_COUNT,
};

#if defined(IS_WINDOWS)
# define THIS_OS Platform_Windows
#elif defined(IS_LINUX)
# define THIS_OS Platform_Linux
#elif defined(IS_MAC)
# define THIS_OS Platform_Mac
#else
# error This platform is not enumerated.
#endif

#if defined(IS_CL)
# define THIS_COMPILER Compiler_CL
#elif defined(IS_GCC)
# define THIS_COMPILER Compiler_GCC
#else
# error This compilers is not enumerated.
#endif

char *windows_platform_layer[] = { "platform_win32\\win32_4ed.cpp", 0 };
char *linux_platform_layer[] = { "platform_linux/linux_4ed.cpp", 0 };
char *mac_platform_layer[] = { "platform_mac/mac_4ed.m", "platform_mac/mac_4ed.cpp", 0 };

char **platform_layers[Platform_COUNT] = {
    windows_platform_layer,
    linux_platform_layer  ,
    mac_platform_layer    ,
};

char *windows_cl_platform_inc[] = { ".", "platform_all", 0 };
char *linux_gcc_platform_inc[] = { "platform_all", "platform_unix", 0 };
char *mac_gcc_platform_inc[] = { "platform_all", "platform_unix", 0 };

char **platform_includes[Platform_COUNT][Compiler_COUNT] = {
    {windows_cl_platform_inc, 0                      },
    {0                      , linux_gcc_platform_inc },
    {0                      , mac_gcc_platform_inc   },
};

#define BUILD_DIR "../build"

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
    KEEP_ASSERT = 0x200,
    SITE_INCLUDES = 0x400,
    X86 = 0x800,
    LOG = 0x1000,
};

#if defined(IS_CL)

//
// cl build
//

#define CL_OPTS                                  \
"-W4 -wd4310 -wd4100 -wd4201 -wd4505 -wd4996 "   \
"-wd4127 -wd4510 -wd4512 -wd4610 -wd4390 "       \
"-wd4611 -WX -GR- -EHa- -nologo -FC"

#define CL_INCLUDES "/I..\\foreign /I..\\foreign\\freetype2"

#define CL_SITE_INCLUDES "/I..\\..\\foreign /I..\\..\\code"

#define CL_LIBS_X64                              \
"user32.lib winmm.lib gdi32.lib opengl32.lib "   \
"..\\foreign_x64\\freetype.lib"

#define CL_LIBS_X86                              \
"user32.lib winmm.lib gdi32.lib opengl32.lib "   \
"..\\foreign_x86\\freetype.lib"

#define CL_ICON "..\\res\\icon.res"

#define CL_X86 "-MACHINE:X86"

static void
build(u32 flags, char *code_path, char **code_files, char *out_path, char *out_file, char *exports, char **inc_folders){
    Build_Line line;
    init_build_line(&line);
    
    Build_Line link_line;
    init_build_line(&link_line);
    
    Build_Line line_prefix;
    init_build_line(&line_prefix);
    
    if (flags & X86){
        build_ap(line_prefix, "%s\\windows_scripts\\setup_cl_x86.bat & ", code_path);
    }
    
    if (flags & OPTS){
        build_ap(line, CL_OPTS);
    }
    
    if (flags & X86){
        build_ap(line, "/DFTECH_32_BIT");
    }
    
    if (flags & LOG){
        build_ap(line, "/DUSE_LOG /DUSE_LOGF");
    }
    
    if (flags & INCLUDES){
        build_ap(line, CL_INCLUDES);
    }
    
    if (flags & SITE_INCLUDES){
        build_ap(line, CL_SITE_INCLUDES);
    }
    
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            build_ap(line, "/I%s\\%s", code_path, inc_folders[i]);
        }
    }
    
    if (flags & LIBS){
        if (flags & X86){
            build_ap(line, CL_LIBS_X86);
        }
        else{
            build_ap(line, CL_LIBS_X64);
        }
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
    
    if (flags & X86){
        build_ap(link_line, CL_X86);
    }
    
    if (flags & DEBUG_INFO){
        build_ap(link_line, "/DEBUG");
    }
    
    char link_type_string[1024];
    if (flags & SHARED_CODE){
        assert(exports);
        snprintf(link_type_string, sizeof(link_type_string), "/OPT:REF %s", exports);
    }
    else{
        snprintf(link_type_string, sizeof(link_type_string), "/NODEFAULTLIB:library");
    }
    build_ap(link_line, "%s", link_type_string);
    
    for (u32 i = 0; code_files[i]; ++i){
        build_ap(line, "\"%s\\%s\"", code_path, code_files[i]);
    }
    
    swap_ptr(&line.build_options, &line.build_options_prev);
    swap_ptr(&link_line.build_options, &link_line.build_options_prev);
    swap_ptr(&line_prefix.build_options, &line_prefix.build_options_prev);
    
    Temp_Dir temp = fm_pushdir(out_path);
    systemf("%scl %s /Fe%s /link /INCREMENTAL:NO %s", line_prefix.build_options, line.build_options, out_file, link_line.build_options);
    fm_popdir(temp);
}

#elif defined(IS_GCC)

//
// gcc build
//

#if defined(IS_LINUX)

# define GCC_OPTS                     \
"-Wno-write-strings "                 \
"-D_GNU_SOURCE -fPIC "                \
"-fno-threadsafe-statics -pthread"

#define GCC_LIBS                               \
"-L/usr/local/lib -lX11 -lpthread -lm -lrt "   \
"-lGL -ldl -lXfixes -lfreetype -lfontconfig"

#elif defined(IS_MAC)

# define GCC_OPTS                                   \
"-Wno-write-strings -Wno-deprecated-declarations "  \
"-Wno-comment -Wno-switch -Wno-null-dereference "

#define GCC_LIBS                          \
"-framework Cocoa -framework QuartzCore " \
"-framework OpenGL -framework IOKit"

#else
# error gcc options not set for this platform
#endif

#define GCC_X86 "-m32"

#define GCC_X64 "-m64"

#define GCC_INCLUDES "-I../foreign -I../code"

#define GCC_SITE_INCLUDES "-I../../foreign -I../../code"

static void
build(u32 flags, char *code_path, char **code_files, char *out_path, char *out_file, char *exports, char **inc_folders){
    Build_Line line;
    init_build_line(&line);
    
    if (flags & X86){
        build_ap(line, GCC_X86);
    }
    else{
        build_ap(line, GCC_X64);
    }
    
    if (flags & OPTS){
        build_ap(line, GCC_OPTS);
    }
    
    if (flags & INCLUDES){
#if defined(IS_LINUX)
        i32 size = 0;
        char freetype_include[512];
        FILE *file = popen("pkg-config --cflags freetype2", "r");
        if (file != 0){
            fgets(freetype_include, sizeof(freetype_include), file);
            size = strlen(freetype_include);
            freetype_include[size-1] = 0;
            pclose(file);
        }
        
        build_ap(line, GCC_INCLUDES" %s", freetype_include);
#else
        build_ap(line, GCC_INCLUDES);
#endif
    }
    
    if (flags & SITE_INCLUDES){
        build_ap(line, GCC_SITE_INCLUDES);
    }
    
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            build_ap(line, "-I%s/%s", code_path, inc_folders[i]);
        }
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
    
    if (flags & LOG){
        build_ap(line, "-DUSE_LOG -DUSE_LOGF");
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
    
    build_ap(line, "-I\"%s\"", code_path);
    for (u32 i = 0; code_files[i] != 0; ++i){
        build_ap(line, "\"%s/%s\"", code_path, code_files[i]);
    }
    
    if (flags & LIBS){
        build_ap(line, GCC_LIBS);
    }
    
    swap_ptr(&line.build_options, &line.build_options_prev);
    
    Temp_Dir temp = pushdir(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    popdir(temp);
}

#else
# error build function not defined for this compiler
#endif

static void
build(u32 flags, char *code_path, char *code_file, char *out_path, char *out_file, char *exports, char **inc_folders){
    char *code_files[2];
    code_files[0] = code_file;
    code_files[1] = 0;
    
    build(flags, code_path, code_files, out_path, out_file, exports, inc_folders);
}

static void
buildsuper(char *code_path, char *out_path, char *filename, b32 x86_build){
    Temp_Dir temp = fm_pushdir(out_path);
#if defined(IS_CL)
    {
        char *prefix_1 = "";
        char *prefix_2 = "";
        char *build_script = "buildsuper.bat";
        if (x86_build){
            prefix_1 = code_path;
            prefix_2 = "\\windows_scripts\\setup_cl_x86.bat & ";
            build_script = "buildsuper_x86.bat";
        }
        systemf("%s%scall \"%s\\%s\" %s", prefix_1, prefix_2, code_path, build_script, filename);
    }
#elif defined(IS_GCC)
    {
        systemf("\"%s/buildsuper.sh\" \"%s\"", code_path, filename);
    }
#else
# error The build rule for this compiler is not ready
#endif
    fm_popdir(temp);
}

static void
fsm_generator(char *cdir){
    {
        char *file = fm_prepare_string("meta/fsm_table_generator.cpp", 0);
        char *dir = fm_prepare_string(BUILD_DIR, 0);
        
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, cdir, file, dir, "fsmgen", 0, 0);
        END_TIME_SECTION("build fsm generator");
    }
    
    if (prev_error == 0){
        char *cmd = fm_prepare_string(BUILD_DIR"/fsmgen", 0);
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(cdir, cmd, 0);
        END_TIME_SECTION("run fsm generator");
    }
}

static void
metagen(char *cdir){
    {
        char *file = fm_prepare_string("meta/4ed_metagen.cpp", 0);
        char *dir = fm_prepare_string(BUILD_DIR, 0);
        
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | DEBUG_INFO, cdir, file, dir, "metagen", 0, 0);
        END_TIME_SECTION("build metagen");
    }
    
    if (prev_error == 0){
        char *cmd = fm_prepare_string(BUILD_DIR"/metagen", 0);
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(cdir, cmd, 0);
        END_TIME_SECTION("run metagen");
    }
}

enum{
    Custom_Default,
    Custom_Experiments,
    Custom_Casey,
    Custom_ChronalVim,
    Custom_COUNT
};

static void
do_buildsuper(char *cdir, i32 custom_option, u32 flags){
    char space[1024];
    String str = make_fixed_width_string(space);
    
    BEGIN_TIME_SECTION();
    
    switch (custom_option){
        case Custom_Default:
        {
            copy_sc(&str, "../code/4coder_default_bindings.cpp");
        }break;
        
        case Custom_Experiments:
        {
            copy_sc(&str, "../code/power/4coder_experiments.cpp");
        }break;
        
        case Custom_Casey:
        {
            copy_sc(&str, "../code/power/4coder_casey.cpp");
        }break;
        
        case Custom_ChronalVim:
        {
            copy_sc(&str, "../4vim/4coder_chronal.cpp");
        }break;
    }
    terminate_with_null(&str);
    
    b32 x86_build = false;
    if (flags & X86){
        x86_build = true;
    }
    
    char *dir = fm_prepare_string(BUILD_DIR, 0);
    buildsuper(cdir, dir, str.str, x86_build);
    
    END_TIME_SECTION("build custom");
}

static void
build_main(char *cdir, u32 flags){
    char *dir = fm_prepare_string(BUILD_DIR);
    
    {
        char *file = fm_prepare_string("4ed_app_target.cpp");
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | SHARED_CODE | flags, cdir, file, dir, "4ed_app" DLL, "/EXPORT:app_get_functions", 0);
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | LIBS | ICON | flags, cdir, platform_layers[THIS_OS], dir, "4ed", 0, platform_includes[THIS_OS][THIS_COMPILER]);
        END_TIME_SECTION("build 4ed");
    }
    
    {
        BEGIN_TIME_SECTION();
        char *themes_folder = fm_prepare_string("../build/themes");
        char *source_themes_folder = fm_prepare_string("themes");
        fm_clear_folder(themes_folder);
        fm_make_folder_if_missing(themes_folder, 0);
        fm_copy_all(source_themes_folder, "*", themes_folder);
        END_TIME_SECTION("move files");
    }
}

static void
standard_build(char *cdir, u32 flags){
    fsm_generator(cdir);
    metagen(cdir);
    do_buildsuper(cdir, Custom_Default, flags);
    //do_buildsuper(cdir, Custom_Experiments, flags);
    //do_buildsuper(cdir, Custom_Casey, flags);
    //do_buildsuper(cdir, Custom_ChronalVim, flags);
    build_main(cdir, flags);
}

static void
site_build(char *cdir, u32 flags){
    {
        char *file = fm_prepare_string("site/sitegen.cpp");
        char *dir = fm_prepare_string(BUILD_DIR"/site");
        BEGIN_TIME_SECTION();
        build(OPTS | SITE_INCLUDES | flags, cdir, file, dir, "sitegen", 0, 0);
        END_TIME_SECTION("build sitegen");
    }
    
    {
        BEGIN_TIME_SECTION();
        char *cmd = fm_prepare_string("../build/site/sitegen . ../site_resources site/source_material ../site");
        systemf("%s", cmd);
        END_TIME_SECTION("run sitegen");
    }
}

#define PACK_DIR "../distributions"

static void
get_4coder_dist_name(String *zip_file, b32 OS_specific, char *folder, char *tier, char *arch, char *ext){
    zip_file->size = 0;
    
    append_sc(zip_file, PACK_DIR"/");
    if (folder != 0){
        append_sc(zip_file, folder);
        append_sc(zip_file, "/");
    }
    append_sc(zip_file, "4coder-");
    
    append_sc         (zip_file, "alpha");
    append_sc         (zip_file, "-");
    append_int_to_str (zip_file, MAJOR);
    append_sc         (zip_file, "-");
    append_int_to_str (zip_file, MINOR);
    append_sc         (zip_file, "-");
    append_int_to_str (zip_file, PATCH);
    if (tier != 0 && !match_cc(tier, "alpha")){
        append_sc     (zip_file, "-");
        append_sc     (zip_file, tier);
    }
    if (OS_specific){
#if defined(IS_WINDOWS)
        append_sc(zip_file, "-win");
#elif defined(IS_LINUX)
        append_sc(zip_file, "-linux");
#elif defined(IS_MAC)
        append_sc(zip_file, "-mac");
#else
#error No OS string for zips on this OS
#endif
    }
    if (arch != 0){
        append_sc     (zip_file, "-");
        append_sc     (zip_file, arch);
    }
    append_sc     (zip_file, ".");
    append_sc     (zip_file, ext);
    terminate_with_null(zip_file);
    
    fm_slash_fix(zip_file->str);
}

static void
package(char *cdir){
    char str_space[1024];
    String str = make_fixed_width_string(str_space);
    
    // NOTE(allen): meta
    fsm_generator(cdir);
    metagen(cdir);
    
#define SITE_DIR "../site"
#define PACK_FONTS_DIR "../code/dist_files/fonts"
    char *build_dir = fm_prepare_string(BUILD_DIR);
    char *site_dir = fm_prepare_string(SITE_DIR);
    char *pack_dir = fm_prepare_string(PACK_DIR);
    char *pack_fonts_dir = fm_prepare_string(PACK_FONTS_DIR);
    
    u32 arch_count = 2;
    char *arch_names[] = {
        "x64",
        "x86",
    };
    Assert(ArrayCount(arch_names) == arch_count);
    
    u32 arch_flags[] = {
        0,
        X86,
    };
    Assert(ArrayCount(arch_flags) == arch_count);
    
    char *base_package_root = "../current_dist";
    
    // NOTE(allen): alpha
    {
        Temp_Memory temp = fm_begin_temp();
        
        char *tier = "alpha";
        
        char *tier_package_root = fm_prepare_string(base_package_root, "_", tier);
        u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG;
        for (u32 i = 0; i < arch_count; ++i){
            char *package_root = fm_prepare_string(tier_package_root, "_", arch_names[i]);
            char *par_dir   = fm_prepare_string(package_root);
            char *dir       = fm_prepare_string(par_dir, "/4coder");
            char *fonts_dir = fm_prepare_string(dir, "/fonts");
            char *zip_dir   = fm_prepare_string(tier, "_", arch_names[i]);
            
            build_main(cdir, base_flags | arch_flags[i]);
            
            fm_clear_folder(par_dir);
            fm_make_folder_if_missing(dir, 0);
            fm_make_folder_if_missing(dir, "fonts");
            fm_make_folder_if_missing(pack_dir, zip_dir);
            fm_copy_file(build_dir, "4ed" EXE, dir, 0, 0);
            fm_copy_file(build_dir, "4ed_app" DLL, dir, 0, 0);
            fm_copy_all(pack_fonts_dir, "*", fonts_dir);
            fm_copy_file(cdir, "release-config.4coder", dir, 0, "config.4coder");
            
            fm_copy_folder(dir, "themes");
            
            fm_copy_file(cdir, "LICENSE.txt", dir, 0, 0);
            fm_copy_file(cdir, "README.txt", dir, 0, 0);
            
            get_4coder_dist_name(&str, true, zip_dir, tier, arch_names[i], "zip");
            fm_zip(par_dir, "4coder", str.str);
        }
        
        fm_end_temp(temp);
    }
    
    // NOTE(allen): super
    
    {
        Temp_Memory temp = fm_begin_temp();
        
        char *tier = "super";
        
        char *tier_package_root = fm_prepare_string(base_package_root, "_", tier);
        u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG | SUPER;
        for (u32 i = 0; i < arch_count; ++i){
            char *package_root = fm_prepare_string(tier_package_root, "_", arch_names[i]);
            char *par_dir   = fm_prepare_string(package_root);
            char *dir       = fm_prepare_string(par_dir, "/4coder");
            char *fonts_dir = fm_prepare_string(dir, "/fonts");
            char *zip_dir   = fm_prepare_string(tier, "_", arch_names[i]);
            
            build_main(cdir, base_flags | arch_flags[i]);
            do_buildsuper(cdir, Custom_Default, arch_flags[i]);
            
            fm_clear_folder(par_dir);
            fm_make_folder_if_missing(dir, 0);
            fm_make_folder_if_missing(dir, "fonts");
            fm_make_folder_if_missing(pack_dir, zip_dir);
            
            fm_copy_file(build_dir, "4ed" EXE, dir, 0, 0);
            fm_copy_file(build_dir, "4ed_app" DLL, dir, 0, 0);
            fm_copy_file(build_dir, "custom_4coder" DLL, dir, 0, 0);
            fm_copy_all(pack_fonts_dir, "*", fonts_dir);
            fm_copy_file(cdir, "release-config.4coder", dir, 0, "config.4coder");
            
            fm_copy_all(0, "4coder_*", dir);
            
            if (!(arch_flags[i] & X86)){
                fm_copy_file(0, "buildsuper" BAT, dir, 0, "buildsuper" BAT);
            }
            else{
                fm_copy_file(0, "buildsuper_x86" BAT, dir, 0, "buildsuper" BAT);
            }
            
#if defined(IS_WINDOWS)
            fm_copy_folder(dir, "windows_scripts");
#endif
            
            fm_copy_folder(dir, "4coder_API");
            fm_copy_folder(dir, "4coder_helper");
            fm_copy_folder(dir, "4coder_lib");
            fm_copy_folder(dir, "4cpp");
            fm_copy_folder(dir, "languages");
            fm_copy_folder(dir, "themes");
            
            fm_copy_file(cdir, "LICENSE.txt", dir, 0, 0);
            fm_copy_file(cdir, "README.txt", dir, 0, 0);
            
            get_4coder_dist_name(&str, true, zip_dir, tier, arch_names[i], "zip");
            fm_zip(par_dir, "4coder", str.str);
        }
        
        fm_make_folder_if_missing(pack_dir, "super-docs");
        get_4coder_dist_name(&str, false, "super-docs", "API", 0, "html");
        String str2 = front_of_directory(str);
        fm_copy_file(site_dir, "custom_docs.html", pack_dir, "super-docs", str2.str);
        
        fm_end_temp(temp);
    }
    
    // NOTE(allen): power
    {
        Temp_Memory temp = fm_begin_temp();
        
        char *pack_power_par_dir = fm_prepare_string("../current_dist_power");
        char *pack_power_dir = fm_prepare_string(pack_power_par_dir, "/power");
        
        fm_clear_folder(pack_power_par_dir);
        fm_make_folder_if_missing(pack_power_dir, 0);
        fm_make_folder_if_missing(pack_dir, "power");
        fm_copy_all("power", "*", pack_power_dir);
        
        get_4coder_dist_name(&str, 0, "power", "power", 0, "zip");
        fm_zip(pack_power_par_dir, "power", str.str);
        
        fm_end_temp(temp);
    }
}

int main(int argc, char **argv){
    fm_init_system();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    i32 n = fm_get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
#if defined(DEV_BUILD) || defined(OPT_BUILD) || defined(DEV_BUILD_X86)
    u32 flags = DEBUG_INFO | SUPER | INTERNAL | LOG;
#if defined(OPT_BUILD)
    flags |= OPTIMIZATION;
#endif
#if defined(DEV_BUILD_X86)
    flags |= X86;
#endif
    standard_build(cdir, flags);
    
#elif defined(PACKAGE)
    package(cdir);
#elif defined(SITE_BUILD)
    site_build(cdir, DEBUG_INFO);
    
#else
#error No build type specified.
#endif
    
    return(error_state);
}

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4ed_file_moving.h"

// BOTTOM


