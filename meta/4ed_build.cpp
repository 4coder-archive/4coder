/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.????
 *
 * 4coder development build rule.
 *
 */

// TOP

//#define FM_PRINT_COMMANDS

#include "../4ed_defines.h"

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4ed_file_moving.h"

#include "../4coder_API/version.h"

//
// OS and compiler index
//

enum{
    Platform_Windows,
    Platform_Linux,
    Platform_Mac,
    //
    Platform_COUNT,
    Platform_None = Platform_COUNT,
};

char *platform_names[] = {
    "win",
    "linux",
    "mac",
};

enum{
    Compiler_CL,
    Compiler_GCC,
    //
    Compiler_COUNT,
    Compiler_None = Compiler_COUNT,
};

char *compiler_names[] = {
    "cl",
    "gcc",
};

#if defined(IS_WINDOWS)
# define This_OS Platform_Windows
#elif defined(IS_LINUX)
# define This_OS Platform_Linux
#elif defined(IS_MAC)
# define This_OS Platform_Mac
#else
# error This platform is not enumerated.
#endif

#if defined(IS_CL)
# define This_Compiler Compiler_CL
#elif defined(IS_GCC)
# define This_Compiler Compiler_GCC
#else
# error This compilers is not enumerated.
#endif

//
// Universal directories
//

#define BUILD_DIR "../build"
#define PACK_DIR "../distributions"
#define SITE_DIR "../site"

#define FOREIGN "../4coder-non-source/foreign"
#define FOREIGN_WIN "..\\4coder-non-source\\foreign"

char *includes[] = { FOREIGN, FOREIGN"/freetype2", 0, };

//
// Platform layer file tables
//

char *windows_platform_layer[] = { "platform_win32/win32_4ed.cpp", 0 };
char *linux_platform_layer[] = { "platform_linux/linux_4ed.cpp", 0 };
char *mac_platform_layer[] = { "platform_mac/mac_4ed.m", "platform_mac/mac_4ed.cpp", 0 };

char **platform_layers[Platform_COUNT] = {
    windows_platform_layer,
    linux_platform_layer  ,
    mac_platform_layer    ,
};

char *windows_cl_platform_inc[] = { "platform_all", 0 };
char *linux_gcc_platform_inc[] = { "platform_all", "platform_unix", 0 };
char *mac_gcc_platform_inc[] = { "platform_all", "platform_unix", 0 };

char **platform_includes[Platform_COUNT][Compiler_COUNT] = {
    {windows_cl_platform_inc, 0                     },
    {0                      , linux_gcc_platform_inc},
    {0                      , mac_gcc_platform_inc  },
};

//
// Custom targets
//

enum{
    Custom_Default,
    Custom_Experiments,
    Custom_Casey,
    Custom_ChronalVim,
    //
    Custom_COUNT
};

char *custom_files[] = {
    "../code/4coder_default_bindings.cpp",
    "../code/power/4coder_experiments.cpp",
    "../code/power/4coder_casey.cpp",
    "../4vim/4coder_chronal.cpp",
};

//
// Architectures
//

enum{
    Arch_X64,
    Arch_X86,
    //
    Arch_COUNT,
    Arch_None = Arch_COUNT,
};

char *arch_names[] = {
    "x64",
    "x86",
};

//
// Build flags
//

enum{
    OPTS = 0x1,
    LIBS = 0x2,
    ICON = 0x4,
    SHARED_CODE = 0x8,
    DEBUG_INFO = 0x10,
    OPTIMIZATION = 0x20,
    SUPER = 0x40,
    INTERNAL = 0x80,
    KEEP_ASSERT = 0x100,
    LOG = 0x200,
};

internal char**
get_defines_from_flags(u32 flags){
    char **result = 0;
    if (flags & KEEP_ASSERT){
        result = fm_list(fm_list_one_item("FRED_KEEP_ASSERT"), result);
    }
    if (flags & INTERNAL){
        result = fm_list(fm_list_one_item("FRED_INTERNAL"), result);
    }
    if (flags & SUPER){
        result = fm_list(fm_list_one_item("FRED_SUPER"), result);
    }
    if (flags & LOG){
        char *log_defines[] = { "USE_LOG", "USE_LOGF", 0};
        result = fm_list(log_defines, result);
    }
    return(result);
}

//
// build implementation: cl
//

#if defined(IS_CL)

#define CL_OPTS                                  \
"-W4 -wd4310 -wd4100 -wd4201 -wd4505 -wd4996 "   \
"-wd4127 -wd4510 -wd4512 -wd4610 -wd4390 "       \
"-wd4611 -WX -GR- -EHa- -nologo -FC"

#define CL_LIBS_X64                              \
"user32.lib winmm.lib gdi32.lib opengl32.lib "   \
FOREIGN_WIN"\\x64\\freetype.lib"

#define CL_LIBS_X86                              \
"user32.lib winmm.lib gdi32.lib opengl32.lib "   \
FOREIGN_WIN"\\x86\\freetype.lib"

#define CL_ICON "..\\res\\icon.res"

internal void
build(u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Temp_Dir temp = fm_pushdir(out_path);
    
    Build_Line line;
    fm_init_build_line(&line);
    
    if (arch == Arch_X86){
        fm_add_to_line(line, "%s\\windows_scripts\\setup_cl_x86.bat &", code_path);
    }
    
    fm_add_to_line(line, "cl");
    
    if (flags & OPTS){
        fm_add_to_line(line, CL_OPTS);
    }
    
    switch (arch){
        case Arch_X64: fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        case Arch_X86: fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        default: InvalidCodePath;
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(code_path, "/", inc_folders[i]);
            fm_add_to_line(line, "-I%s", str);
        }
    }
    
    if (flags & LIBS){
        switch (arch){
            case Arch_X64: fm_add_to_line(line, CL_LIBS_X64); break;
            case Arch_X86: fm_add_to_line(line, CL_LIBS_X86); break;
            default: InvalidCodePath;
        }
    }
    
    if (flags & ICON){
        // TODO(allen): Get this icon in the non-source repository to avoid having to work around it in the future.
        //fm_add_to_line(line, CL_ICON);
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-Zi");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "-O2");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "-LD");
    }
    
    if (defines != 0){
        for (u32 i = 0; defines[i] != 0; ++i){
            char *define_flag = fm_str("-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    for (u32 i = 0; code_files[i]; ++i){
        fm_add_to_line(line, "\"%s\\%s\"", code_path, code_files[i]);
    }
    
    fm_add_to_line(line, "-Fe%s", out_file);
    
    fm_add_to_line(line, "-link -INCREMENTAL:NO");
    switch (arch){
        case Arch_X64: fm_add_to_line(line, "-MACHINE:X64"); break;
        case Arch_X86: fm_add_to_line(line, "-MACHINE:X86"); break;
        default: InvalidCodePath;
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-DEBUG");
    }
    
    if (flags & SHARED_CODE){
        Assert(exports != 0);
        fm_add_to_line(line, "-OPT:REF");
        for (u32 i = 0; exports[i] != 0; ++i){
            char *str = fm_str("-EXPORT:", exports[i]);
            fm_add_to_line(line, "%s", str);
        }
    }
    else{
        fm_add_to_line(line, "-NODEFAULTLIB:library");
    }
    
    fm_finish_build_line(&line);
    
    systemf("%s", line.build_options);
    fm_popdir(temp);
}

//
// build implementation: gcc
//

#elif defined(IS_GCC)

#if defined(IS_LINUX)

# define GCC_OPTS                     \
"-Wno-write-strings "                 \
"-D_GNU_SOURCE -fPIC "                \
"-fno-threadsafe-statics -pthread"

#define GCC_LIBS_COMMON                        \
"-L/usr/local/lib -lX11 -lpthread -lm -lrt "   \
"-lGL -ldl -lXfixes -lfreetype -lfontconfig"

#define GCC_LIBS_X64 GCC_LIBS_COMMON
#define GCC_LIBS_X86 GCC_LIBS_COMMON

#elif defined(IS_MAC)

# define GCC_OPTS                                   \
"-Wno-write-strings -Wno-deprecated-declarations "  \
"-Wno-comment -Wno-switch -Wno-null-dereference "

#define GCC_LIBS_COMMON \
"-framework Cocoa -framework QuartzCore " \
"-framework CoreServices " \
"-framework OpenGL -framework IOKit "

#define GCC_LIBS_X64 GCC_LIBS_COMMON \
FOREIGN"/x64/libfreetype-mac.a"

#define GCC_LIBS_X86 GCC_LIBS_COMMON \
FOREIGN"/x86/libfreetype-mac.a"

#else
# error gcc options not set for this platform
#endif

internal void
build(u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Build_Line line;
    fm_init_build_line(&line);
    
    switch (arch){
        case Arch_X64:
        fm_add_to_line(line, "-m64");
        fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        
        case Arch_X86:
        fm_add_to_line(line, "-m32");
        fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        
        default: InvalidCodePath;
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, GCC_OPTS);
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(code_path, "/", inc_folders[i]);
            fm_add_to_line(line, "-I%s", str);
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
    
    if (defines != 0){
        for (u32 i = 0; defines[i]; ++i){
            char *define_flag = fm_str("-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    fm_add_to_line(line, "-I\"%s\"", code_path);
    for (u32 i = 0; code_files[i] != 0; ++i){
        fm_add_to_line(line, "\"%s/%s\"", code_path, code_files[i]);
    }
    
    if (flags & LIBS){
        if (arch == Arch_X64){
            fm_add_to_line(line, GCC_LIBS_X64);
        }
        else if (arch == Arch_X86)
        {
            fm_add_to_line(line, GCC_LIBS_X86);
        }
    }
    
    fm_finish_build_line(&line);
    
    Temp_Dir temp = fm_pushdir(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    fm_popdir(temp);
}

#else
# error build function not defined for this compiler
#endif

internal void
build(u32 flags, u32 arch, char *code_path, char *code_file, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    char *code_files[2];
    code_files[0] = code_file;
    code_files[1] = 0;
    build(flags, arch, code_path, code_files, out_path, out_file, defines, exports, inc_folders);
}

// TODO(NAME): build metadata fully from C++ and eliminate build_metadata.bat and build_metadata.sh

internal void
site_build(char *cdir, u32 flags){
    {
        systemf("%s -R %s", "build_metadata" BAT, cdir);
    }
    
    {
        char *file = fm_str("site/4ed_sitegen.cpp");
        char *dir = fm_str(BUILD_DIR);
        BEGIN_TIME_SECTION();
        build(OPTS | flags, Arch_X64, cdir, file, dir, "sitegen", get_defines_from_flags(flags), 0, includes);
        END_TIME_SECTION("build sitegen");
    }
    
    if (prev_error == 0){
        BEGIN_TIME_SECTION();
        char *cmd = fm_str(BUILD_DIR "/sitegen");
        char *code_dir = fm_str(".");
        char *asset_dir = fm_str("../4coder-non-source/site_resources");
        char *site_source_dir = fm_str("site/source_material");
        char *dest_dir = fm_str("../site");
        fm_make_folder_if_missing(dest_dir);
        systemf("%s %s %s %s %s", cmd, code_dir, asset_dir, site_source_dir, dest_dir);
        END_TIME_SECTION("run sitegen");
    }
}

internal void
build_and_run(char *cdir, char *filename, char *name, u32 flags){
    char *dir = fm_str(BUILD_DIR);
    
    {
        char *file = fm_str(filename);
        BEGIN_TIME_SECTION();
        build(flags, Arch_X64, cdir, file, dir, name, get_defines_from_flags(flags), 0, includes);
        END_TIME_SECTION(fm_str("build ", name));
    }
    
    if (prev_error == 0){
        char *cmd = fm_str(dir, "/", name);
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(cdir, cmd, 0);
        END_TIME_SECTION(fm_str("run ", name));
    }
}

internal void
fsm_generator(char *cdir){
    build_and_run(cdir, "meta/4ed_fsm_table_generator.cpp", "fsmgen", OPTS | DEBUG_INFO);
}

internal void
metagen(char *cdir){
    build_and_run(cdir, "meta/4ed_metagen.cpp", "metagen", OPTS | DEBUG_INFO);
}

internal void
string_build(char *cdir){
    char *dir = fm_str(BUILD_DIR);
    
    {
        char *file = fm_str("string/4ed_string_builder.cpp");
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, Arch_X64, cdir, file, dir, "string_builder", 0, 0, includes);
        END_TIME_SECTION("build string_builder");
    }
    
    if (prev_error == 0){
        char *cmd = fm_str(cdir, "/", dir, "/string_builder");
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(fm_str(cdir, "/string"), cmd, 0);
        END_TIME_SECTION("run string_builder");
    }
}

internal void
do_buildsuper(char *cdir, char *file, u32 arch){
    BEGIN_TIME_SECTION();
    Temp_Dir temp = fm_pushdir(fm_str(BUILD_DIR));
    
    char *build_script = fm_str("buildsuper_", arch_names[arch], BAT);
    
    char *build_command = fm_str("\"", cdir, "/", build_script, "\" \"", file, "\"");
    if (This_OS == Platform_Windows){
        build_command = fm_str("call ", build_command);
    }
    systemf("%s", build_command);
    
    fm_popdir(temp);
    END_TIME_SECTION("build custom");
}

// TODO(allen): Remove this
internal i32
get_freetype_include(char *out, u32 max){
    i32 size = 0;
#if 0
#if defined(IS_LINUX)
    char freetype_include[512];
    FILE *file = popen("pkg-config --cflags freetype2", "r");
    if (file != 0){
        fgets(freetype_include, sizeof(freetype_include), file);
        size = strlen(freetype_include);
        memcpy(out, freetype_include, size);
        pclose(file);
    }
#elif defined(IS_MAC)
    char *freetype_include = "/usr/local/include/freetype2";
    size = strlen(freetype_include);
    memcpy(out, freetype_include, size
           );
#endif
#endif
    return(size);
}

internal void
build_main(char *cdir, b32 update_local_theme, u32 flags, u32 arch){
    char *dir = fm_str(BUILD_DIR);
    
    {
        char *file = fm_str("4ed_app_target.cpp");
        char **exports = fm_list_one_item("app_get_functions");
        
        char **build_includes = includes;
        
        char ft_include[512];
        i32 ft_size = get_freetype_include(ft_include, sizeof(ft_include) - 1);
        if (ft_size > 0){
            ft_include[ft_size] = 0;
            fprintf(stdout, "FREETYPE: %s\n", ft_include);
            build_includes = fm_list(build_includes, fm_list_one_item(ft_include));
        }
        
        BEGIN_TIME_SECTION();
        build(OPTS | SHARED_CODE | flags, arch, cdir, file, dir, "4ed_app" DLL, get_defines_from_flags(flags), exports, build_includes);
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        char **inc = (char**)fm_list(includes, platform_includes[This_OS][This_Compiler]);
        build(OPTS | LIBS | ICON | flags, arch, cdir, platform_layers[This_OS], dir, "4ed", get_defines_from_flags(flags), 0, inc);
        END_TIME_SECTION("build 4ed");
    }
    
    if (update_local_theme){
        BEGIN_TIME_SECTION();
        char *themes_folder = fm_str("../build/themes");
        char *source_themes_folder = fm_str("themes");
        fm_clear_folder(themes_folder);
        fm_make_folder_if_missing(themes_folder);
        fm_copy_all(source_themes_folder, "*", themes_folder);
        END_TIME_SECTION("move files");
    }
}

internal void
standard_build(char *cdir, u32 flags, u32 arch){
    fsm_generator(cdir);
    metagen(cdir);
    //do_buildsuper(cdir, fm_str(custom_files[Custom_Default]), arch);
    do_buildsuper(cdir, fm_str(custom_files[Custom_Experiments]), arch);
    //do_buildsuper(cdir, fm_str(custom_files[Custom_Casey]), arch);
    //do_buildsuper(cdir, fm_str(custom_files[Custom_ChronalVim]), arch);
    build_main(cdir, true, flags, arch);
}

internal char*
get_4coder_dist_name(u32 platform, char *tier, u32 arch){
    char *name = fm_str("4coder-alpha-"MAJOR_STR"-"MINOR_STR"-"PATCH_STR);
    if (strcmp(tier, "alpha") != 0){
        name = fm_str(name, "-", tier);
    }
    if (platform != Platform_None){
        name = fm_str(name, "-", platform_names[platform]);
    }
    if (arch != Arch_None){
        name = fm_str(name, "-", arch_names[arch]);
    }
    return(name);
}

internal void
package(char *cdir){
    // NOTE(allen): meta
    fsm_generator(cdir);
    metagen(cdir);
    
    char *build_dir = fm_str(BUILD_DIR);
    char *pack_dir = fm_str(PACK_DIR);
    char *fonts_source_dir = fm_str("../4coder-non-source/dist_files/fonts");
    
    char *base_package_root = "../current_dist";
    
    // NOTE(allen): alpha and super builds
    enum{
        Tier_Alpha,
        Tier_Super,
        Tier_COUNT
    };
    
    char *tiers[] = { "alpha", "super" };
    u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG;
    u32 tier_flags[] = { 0, SUPER, };
    
    for (u32 tier_index = 0; tier_index < Tier_COUNT; ++tier_index){
        char *tier = tiers[tier_index];
        u32 flags = base_flags | tier_flags[tier_index];
        
        Temp temp = fm_begin_temp();
        char *tier_package_root = fm_str(base_package_root, "_", tier);
        for (u32 arch = 0; arch < Arch_COUNT; ++arch){
            char *par_dir      = fm_str(tier_package_root, "_", arch_names[arch]);
            char *dir          = fm_str(par_dir, "/4coder");
            char *fonts_dir    = fm_str(dir, "/fonts");
            char *zip_dir      = fm_str(pack_dir, "/", tier, "_", arch_names[arch]);
            
            build_main(cdir, false, flags, arch);
            
            fm_clear_folder(par_dir);
            
            fm_make_folder_if_missing(dir);
            fm_copy_file(fm_str(build_dir, "/4ed" EXE), fm_str(dir, "/4ed" EXE));
            fm_copy_file(fm_str(build_dir, "/4ed_app" DLL), fm_str(dir, "/4ed_app" DLL));
            
            fm_copy_folder(cdir, dir, "themes");
            fm_copy_file(fm_str(cdir, "/LICENSE.txt"), fm_str(dir, "/LICENSE.txt"));
            fm_copy_file(fm_str(cdir, "/README.txt"), fm_str(dir, "/README.txt"));
            
            fm_make_folder_if_missing(fonts_dir);
            fm_copy_all(fonts_source_dir, "*", fonts_dir);
            fm_copy_file(fm_str(cdir, "/release-config.4coder"), fm_str(dir, "/config.4coder"));
            
            if (tier_index == Tier_Super){
                fm_copy_all(0, "4coder_*", dir);
                
                do_buildsuper(cdir, fm_str(custom_files[Custom_Default]), arch);
                fm_copy_file(fm_str(build_dir, "/custom_4coder" DLL), fm_str(dir, "/custom_4coder" DLL));
                
                char *build_script = fm_str("buildsuper_", arch_names[arch], BAT);
                fm_copy_file(build_script, fm_str(dir, "/buildsuper" BAT));
                
                if (This_OS == Platform_Windows){
                    fm_copy_folder(cdir, dir, "windows_scripts");
                }
                
                fm_copy_folder(cdir, dir, "4coder_API");
                fm_copy_folder(cdir, dir, "4coder_helper");
                fm_copy_folder(cdir, dir, "4coder_lib");
                fm_copy_folder(cdir, dir, "4coder_generated");
                fm_copy_folder(cdir, dir, "languages");
            }
            
            char *dist_name = get_4coder_dist_name(This_OS, tier, arch);
            char *zip_name = fm_str(zip_dir, "/", dist_name, ".zip");
            fm_make_folder_if_missing(zip_dir);
            fm_zip(par_dir, "4coder", zip_name);
        }
        fm_end_temp(temp);
    }
    
    // NOTE(allen): power
    {
        Temp temp = fm_begin_temp();
        char *pack_power_par_dir = fm_str("../current_dist_power");
        char *pack_power_dir = fm_str(pack_power_par_dir, "/power");
        
        fm_clear_folder(pack_power_par_dir);
        fm_make_folder_if_missing(pack_power_dir);
        fm_make_folder_if_missing(fm_str(pack_dir, "/power"));
        fm_copy_all("power", "*", pack_power_dir);
        
        char *dist_name = get_4coder_dist_name(Platform_None, "power", Arch_None);
        char *zip_name = fm_str(pack_dir, "/power/", dist_name, ".zip");
        fm_zip(pack_power_par_dir, "power", zip_name);
        fm_end_temp(temp);
    }
}

int main(int argc, char **argv){
    fm_init_system();
    
    char cdir[256];
    BEGIN_TIME_SECTION();
    i32 n = fm_get_current_directory(cdir, sizeof(cdir));
    Assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
#if defined(DEV_BUILD) || defined(OPT_BUILD) || defined(DEV_BUILD_X86)
    u32 flags = DEBUG_INFO | SUPER | INTERNAL | LOG;
    u32 arch = Arch_X64;
#if defined(OPT_BUILD)
    flags |= OPTIMIZATION;
#endif
#if defined(DEV_BUILD_X86)
    arch = Arch_X86;
#endif
    standard_build(cdir, flags, arch);
    
#elif defined(PACKAGE)
    package(cdir);
    
#elif defined(SITE_BUILD)
    site_build(cdir, DEBUG_INFO);
    
#elif defined(STRING_BUILD)
    string_build(cdir);
    
#else
# error No build type specified.
#endif
    
    return(error_state);
}

// BOTTOM

