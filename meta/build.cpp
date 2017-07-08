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
    Build_Line link_line;
    Build_Line line_prefix;

    fm_init_build_line(&line);
    fm_init_build_line(&link_line);
    fm_init_build_line(&line_prefix);

    if (flags & X86){
        fm_add_to_line(line_prefix, "%s\\windows_scripts\\setup_cl_x86.bat & ", code_path);
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, CL_OPTS);
    }
    
    if (flags & X86){
        fm_add_to_line(line, "/DFTECH_32_BIT");
    }
    
    if (flags & LOG){
        fm_add_to_line(line, "/DUSE_LOG /DUSE_LOGF");
    }
    
    if (flags & INCLUDES){
        fm_add_to_line(line, CL_INCLUDES);
    }
    
    if (flags & SITE_INCLUDES){
        fm_add_to_line(line, CL_SITE_INCLUDES);
    }
    
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            fm_add_to_line(line, "/I%s\\%s", code_path, inc_folders[i]);
        }
    }
    
    if (flags & LIBS){
        if (flags & X86){
            fm_add_to_line(line, CL_LIBS_X86);
        }
        else{
            fm_add_to_line(line, CL_LIBS_X64);
        }
    }
    
    if (flags & ICON){
        fm_add_to_line(line, CL_ICON);
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "/Zi");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "/O2");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "/LD");
    }
    
    if (flags & SUPER){
        fm_add_to_line(line, "/DFRED_SUPER");
    }
    
    if (flags & INTERNAL){
        fm_add_to_line(line, "/DFRED_INTERNAL");
    }
    
    if (flags & KEEP_ASSERT){
        fm_add_to_line(line, "/DFRED_KEEP_ASSERT");
    }
    
    if (flags & X86){
        fm_add_to_line(link_line, CL_X86);
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(link_line, "/DEBUG");
    }
    
    char link_type_string[1024];
    if (flags & SHARED_CODE){
        assert(exports);
        snprintf(link_type_string, sizeof(link_type_string), "/OPT:REF %s", exports);
    }
    else{
        snprintf(link_type_string, sizeof(link_type_string), "/NODEFAULTLIB:library");
    }
    fm_add_to_line(link_line, "%s", link_type_string);
    
    for (u32 i = 0; code_files[i]; ++i){
        fm_add_to_line(line, "\"%s\\%s\"", code_path, code_files[i]);
    }
    
    fm_finish_build_line(&line);
    fm_finish_build_line(&link_line);
    fm_finish_build_line(&line_prefix);
    
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
    fm_init_build_line(&line);
    
    if (flags & X86){
        fm_add_to_line(line, GCC_X86);
    }
    else{
        fm_add_to_line(line, GCC_X64);
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, GCC_OPTS);
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
        
        fm_add_to_line(line, GCC_INCLUDES" %s", freetype_include);
#else
        fm_add_to_line(line, GCC_INCLUDES);
#endif
    }
    
    if (flags & SITE_INCLUDES){
        fm_add_to_line(line, GCC_SITE_INCLUDES);
    }
    
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            fm_add_to_line(line, "-I%s/%s", code_path, inc_folders[i]);
        }
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-g -O0");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "-O3");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "-shared");
    }
    
    if (flags & LOG){
        fm_add_to_line(line, "-DUSE_LOG -DUSE_LOGF");
    }
    
    if (flags & SUPER){
        fm_add_to_line(line, "-DFRED_SUPER");
    }
    
    if (flags & INTERNAL){
        fm_add_to_line(line, "-DFRED_INTERNAL");
    }
    
    if (flags & KEEP_ASSERT){
        fm_add_to_line(line, "-DFRED_KEEP_ASSERT");
    }
    
    fm_add_to_line(line, "-I\"%s\"", code_path);
    for (u32 i = 0; code_files[i] != 0; ++i){
        fm_add_to_line(line, "\"%s/%s\"", code_path, code_files[i]);
    }
    
    if (flags & LIBS){
        fm_add_to_line(line, GCC_LIBS);
    }
    
    fm_finish_build_line(&line);
    
    Temp_Dir temp = fm_pushdir(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    fm_popdir(temp);
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
build_and_run(char *cdir, char *filename, char *name, u32 flags){
    char *dir = fm_str(BUILD_DIR);

    {
        char *file = fm_str(filename);
        BEGIN_TIME_SECTION();
        build(flags, cdir, file, dir, name, 0, 0);
        END_TIME_SECTION(fm_str("build ", name));
    }

    if (prev_error == 0){
        char *cmd = fm_str(dir, "/", name);
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(cdir, cmd, 0);
        END_TIME_SECTION(fm_str("run ", name));
    }
}

static void
fsm_generator(char *cdir){
    build_and_run(cdir, "meta/fsm_table_generator.cpp", "fsmgen", OPTS | DEBUG_INFO);
}

static void
metagen(char *cdir){
    build_and_run(cdir, "meta/4ed_metagen.cpp", "metagen", OPTS | DEBUG_INFO | INCLUDES);
}

static void
buildsuper(char *code_path, char *out_path, char *filename, b32 x86_build){
    Temp_Dir temp = fm_pushdir(out_path);
#if defined(IS_WINDOWS)
    {
        char *build_script = "buildsuper.bat";
        if (x86_build){
            build_script = "buildsuper_x86.bat";
        }
        systemf("call \"%s\\%s\" \"%s\"", code_path, build_script, filename);
    }
#elif defined(IS_LINUX) || defined(IS_MAC)
    {
        char *build_script = "buildsuper.sh";
        if (x86_build){
            build_script = "buildsuper_x86.sh";
        }
        systemf("\"%s/%s\" \"%s\"", code_path, build_script, filename);
    }
#else
# error The buildsuper rule for this OS is not ready
#endif
    fm_popdir(temp);
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
    BEGIN_TIME_SECTION();
    
    char *str = 0;
    switch (custom_option){
        case Custom_Default:
        {
            str = fm_str("../code/4coder_default_bindings.cpp");
        }break;
        
        case Custom_Experiments:
        {
            str = fm_str("../code/power/4coder_experiments.cpp");
        }break;
        
        case Custom_Casey:
        {
            str = fm_str("../code/power/4coder_casey.cpp");
        }break;
        
        case Custom_ChronalVim:
        {
            str = fm_str("../4vim/4coder_chronal.cpp");
        }break;
    }
    
    if (str != 0){
        b32 x86_build = ((flags & X86) != 0);
        char *dir = fm_str(BUILD_DIR);
        buildsuper(cdir, dir, str
            , x86_build);
        END_TIME_SECTION("build custom");
    }
}

static void
build_main(char *cdir, u32 flags){
    char *dir = fm_str(BUILD_DIR);
    
    {
        char *file = fm_str("4ed_app_target.cpp");
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
        char *themes_folder = fm_str("../build/themes");
        char *source_themes_folder = fm_str("themes");
        fm_clear_folder(themes_folder);
        fm_make_folder_if_missing(themes_folder);
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
        char *file = fm_str("site/sitegen.cpp");
        char *dir = fm_str(BUILD_DIR"/site");
        BEGIN_TIME_SECTION();
        build(OPTS | SITE_INCLUDES | flags, cdir, file, dir, "sitegen", 0, 0);
        END_TIME_SECTION("build sitegen");
    }
    
    {
        BEGIN_TIME_SECTION();
        char *cmd = fm_str("../build/site/sitegen . ../site_resources site/source_material ../site");
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
    char *build_dir = fm_str(BUILD_DIR);
    char *site_dir = fm_str(SITE_DIR);
    char *pack_dir = fm_str(PACK_DIR);
    char *pack_fonts_dir = fm_str(PACK_FONTS_DIR);
    
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
        
        char *tier_package_root = fm_str(base_package_root, "_", tier);
        u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG;
        for (u32 i = 0; i < arch_count; ++i){
            char *package_root = fm_str(tier_package_root, "_", arch_names[i]);
            char *par_dir   = fm_str(package_root);
            char *dir       = fm_str(par_dir, "/4coder");
            char *fonts_dir = fm_str(dir, "/fonts");
            char *zip_dir   = fm_str(tier, "_", arch_names[i]);
            
            build_main(cdir, base_flags | arch_flags[i]);
            
            fm_clear_folder(par_dir);
            fm_make_folder_if_missing(dir);
            fm_make_folder_if_missing(fonts_dir);
            fm_make_folder_if_missing(fm_str(pack_dir, "/", zip_dir));
            fm_copy_file(fm_str(build_dir, "/4ed"EXE), fm_str(dir, "/4ed"EXE));
            fm_copy_file(fm_str(build_dir, "/4ed_app"DLL), fm_str(dir, "/4ed_app"DLL));
            fm_copy_all(pack_fonts_dir, "*", fonts_dir);
            fm_copy_file(fm_str(cdir, "/release-config.4coder"), fm_str(dir, "/config.4coder"));
            
            fm_copy_folder(dir, "themes");
            
            fm_copy_file(fm_str(cdir, "/LICENSE.txt"), fm_str(dir, "/LICENSE.txt"));
            fm_copy_file(fm_str(cdir, "/README.txt"), fm_str(dir, "/README.txt"));
            
            get_4coder_dist_name(&str, true, zip_dir, tier, arch_names[i], "zip");
            fm_zip(par_dir, "4coder", str.str);
        }
        
        fm_end_temp(temp);
    }
    
    // NOTE(allen): super
    
    {
        Temp_Memory temp = fm_begin_temp();
        
        char *tier = "super";
        
        char *tier_package_root = fm_str(base_package_root, "_", tier);
        u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG | SUPER;
        for (u32 i = 0; i < arch_count; ++i){
            char *package_root = fm_str(tier_package_root, "_", arch_names[i]);
            char *par_dir   = fm_str(package_root);
            char *dir       = fm_str(par_dir, "/4coder");
            char *fonts_dir = fm_str(dir, "/fonts");
            char *zip_dir   = fm_str(tier, "_", arch_names[i]);
            
            build_main(cdir, base_flags | arch_flags[i]);
            do_buildsuper(cdir, Custom_Default, arch_flags[i]);
            
            fm_clear_folder(par_dir);
            fm_make_folder_if_missing(dir);
            fm_make_folder_if_missing(fonts_dir);
            fm_make_folder_if_missing(fm_str(pack_dir, "/", zip_dir));
            fm_copy_file(fm_str(build_dir, "/4ed" EXE), fm_str(dir, "/4ed" EXE));
            fm_copy_file(fm_str(build_dir, "/4ed_app" DLL), fm_str(dir, "/4ed_app" DLL));
            fm_copy_file(fm_str(build_dir, "/custom_4coder" DLL), fm_str(dir, "/custom_4coder" DLL));
            fm_copy_all(pack_fonts_dir, "*", fonts_dir);
            fm_copy_file(fm_str(cdir, "/release-config.4coder"), fm_str(dir, "/config.4coder"));
            
            fm_copy_all(0, "4coder_*", dir);
            
            if (!(arch_flags[i] & X86)){
                fm_copy_file("buildsuper" BAT, fm_str(dir, "/buildsuper" BAT));
            }
            else{
                fm_copy_file("buildsuper_x86" BAT, fm_str(dir, "/buildsuper" BAT));
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
            
            fm_copy_file(fm_str(cdir, "/LICENSE.txt"), fm_str(dir, "/LICENSE.txt"));
            fm_copy_file(fm_str(cdir, "/README.txt"), fm_str(dir, "/README.txt"));
            
            get_4coder_dist_name(&str, true, zip_dir, tier, arch_names[i], "zip");
            fm_zip(par_dir, "4coder", str.str);
        }
        
        fm_make_folder_if_missing(fm_str(pack_dir, "/super-docs"));
        get_4coder_dist_name(&str, false, "super-docs", "API", 0, "html");
        String str2 = front_of_directory(str);
        fm_copy_file(fm_str(site_dir, "/custom_docs.html"), fm_str(pack_dir, "/super-docs/", str2.str));
        
        fm_end_temp(temp);
    }
    
    // NOTE(allen): power
    {
        Temp_Memory temp = fm_begin_temp();
        
        char *pack_power_par_dir = fm_str("../current_dist_power");
        char *pack_power_dir = fm_str(pack_power_par_dir, "/power");
        
        fm_clear_folder(pack_power_par_dir);
        fm_make_folder_if_missing(pack_power_dir);
        fm_make_folder_if_missing(fm_str(pack_dir, "/power"));
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

