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
# include "../4coder_lib/4coder_arena.h"
# include "../4coder_lib/4coder_arena.cpp"
# define FSTRING_IMPLEMENTATION
# include "../4coder_lib/4coder_string.h"

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4ed_file_moving.h"

#include "../4coder_API/4coder_version.h"

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
    "../code/4coder_experiments.cpp",
    "../code/4coder_casey.cpp",
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
get_defines_from_flags(Partition *part, u32 flags){
    char **result = 0;
    if (flags & KEEP_ASSERT){
        result = fm_list(part, fm_list_one_item(part, "FRED_KEEP_ASSERT"), result);
    }
    if (flags & INTERNAL){
        result = fm_list(part, fm_list_one_item(part, "FRED_INTERNAL"), result);
    }
    if (flags & SUPER){
        result = fm_list(part, fm_list_one_item(part, "FRED_SUPER"), result);
    }
    if (flags & LOG){
        char *log_defines[] = { "USE_LOG", "USE_LOGF", 0};
        result = fm_list(part, log_defines, result);
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

#define CL_ICON "..\\4coder-non-source\\res\\icon.res"

internal void
build(Partition *part, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
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
            char *str = fm_str(part, code_path, "/", inc_folders[i]);
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
        fm_add_to_line(line, CL_ICON);
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
            char *define_flag = fm_str(part, "-D", defines[i]);
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
            char *str = fm_str(part, "-EXPORT:", exports[i]);
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
"-fno-threadsafe-statics -pthread "   \
"-Wno-unused-result"

#define GCC_LIBS_COMMON                                   \
"-L/usr/local/lib -L/usr/bin -lX11 -lpthread -lm -lrt "   \
"-lGL -ldl -lXfixes -lfreetype -lfontconfig"

#define GCC_LIBS_X64 GCC_LIBS_COMMON
#define GCC_LIBS_X86 GCC_LIBS_COMMON

#elif defined(IS_MAC)

# define GCC_OPTS                                   \
"-Wno-write-strings -Wno-deprecated-declarations "  \
"-Wno-comment -Wno-switch -Wno-null-dereference "   \
"-Wno-tautological-compare "                        \
"-Wno-unused-result "


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
build(Partition *part, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
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
            char *str = fm_str(part, code_path, "/", inc_folders[i]);
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
            char *define_flag = fm_str(part, "-D", defines[i]);
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
build(Partition *part, u32 flags, u32 arch, char *code_path, char *code_file, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    char *code_files[2];
    code_files[0] = code_file;
    code_files[1] = 0;
    build(part, flags, arch, code_path, code_files, out_path, out_file, defines, exports, inc_folders);
}

// TODO(NAME): build metadata fully from C++ and eliminate build_metadata.bat and build_metadata.sh

internal void
build_metadata(void){
    systemf(".%s%s ..%scode%s4coder_default_bindings.cpp", SLASH, "build_metadata" BAT, SLASH, SLASH);
}

internal void
site_build(Partition *part, char *cdir, u32 flags){
    build_metadata();
    
    {
        char *file = fm_str(part, "site/4ed_sitegen.cpp");
        char *dir = fm_str(part, BUILD_DIR);
        BEGIN_TIME_SECTION();
        build(part, OPTS | flags, Arch_X64, cdir, file, dir, "sitegen", get_defines_from_flags(part, flags), 0, includes);
        END_TIME_SECTION("build sitegen");
    }
    
    if (prev_error == 0){
        BEGIN_TIME_SECTION();
        char *cmd = fm_str(part, BUILD_DIR "/sitegen");
        char *code_dir = fm_str(part, ".");
        char *asset_dir = fm_str(part, "../4coder-non-source/site_resources");
        char *site_source_dir = fm_str(part, "site/source_material");
        char *dest_dir = fm_str(part, "../site");
        fm_make_folder_if_missing(part, dest_dir);
        systemf("%s %s %s %s %s", cmd, code_dir, asset_dir, site_source_dir, dest_dir);
        END_TIME_SECTION("run sitegen");
    }
}

internal void
build_and_run(Partition *part, char *cdir, char *filename, char *name, u32 flags){
    char *dir = fm_str(part, BUILD_DIR);
    
    {
        char *file = fm_str(part, filename);
        BEGIN_TIME_SECTION();
        build(part, flags, Arch_X64, cdir, file, dir, name, get_defines_from_flags(part, flags), 0, includes);
        END_TIME_SECTION(fm_str(part, "build ", name));
    }
    
    if (prev_error == 0){
        char *cmd = fm_str(part, dir, "/", name);
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(cdir, cmd, 0);
        END_TIME_SECTION(fm_str(part, "run ", name));
    }
}

internal void
fsm_generator(Partition *part, char *cdir){
    build_and_run(part, cdir, "meta/4ed_fsm_table_generator.cpp", "fsmgen", OPTS | DEBUG_INFO);
}

internal void
metagen(Partition *part, char *cdir){
    build_and_run(part, cdir, "meta/4ed_metagen.cpp", "metagen", OPTS | DEBUG_INFO);
}

internal void
string_build(Partition *part, char *cdir){
    char *dir = fm_str(part, BUILD_DIR);
    
    {
        char *file = fm_str(part, "string/4ed_string_builder.cpp");
        BEGIN_TIME_SECTION();
        build(part, OPTS | DEBUG_INFO, Arch_X64, cdir, file, dir, "string_builder", 0, 0, includes);
        END_TIME_SECTION("build string_builder");
    }
    
    if (prev_error == 0){
        char *cmd = fm_str(part, cdir, "/", dir, "/string_builder");
        BEGIN_TIME_SECTION();
        fm_execute_in_dir(fm_str(part, cdir, "/string"), cmd, 0);
        END_TIME_SECTION("run string_builder");
    }
}

internal void
do_buildsuper(Partition *part, char *cdir, char *file, u32 arch){
    BEGIN_TIME_SECTION();
    Temp_Dir temp = fm_pushdir(fm_str(part, BUILD_DIR));
    
    char *build_script = fm_str(part, "buildsuper_", arch_names[arch], BAT);
    
    char *build_command = fm_str(part, "\"", cdir, "/", build_script, "\" \"", file, "\"");
    if (This_OS == Platform_Windows){
        build_command = fm_str(part, "call ", build_command);
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
build_main(Partition *part, char *cdir, b32 update_local_theme, u32 flags, u32 arch){
    char *dir = fm_str(part, BUILD_DIR);
    
    {
        char *file = fm_str(part, "4ed_app_target.cpp");
        char **exports = fm_list_one_item(part, "app_get_functions");
        
        char **build_includes = includes;
        
        char ft_include[512];
        i32 ft_size = get_freetype_include(ft_include, sizeof(ft_include) - 1);
        if (ft_size > 0){
            ft_include[ft_size] = 0;
            fprintf(stdout, "FREETYPE: %s\n", ft_include);
            build_includes = fm_list(part, build_includes, fm_list_one_item(part, ft_include));
        }
        
        BEGIN_TIME_SECTION();
        build(part, OPTS | SHARED_CODE | flags, arch, cdir, file, dir, "4ed_app" DLL, get_defines_from_flags(part, flags), exports, build_includes);
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        char **inc = (char**)fm_list(part, includes, platform_includes[This_OS][This_Compiler]);
        build(part, OPTS | LIBS | ICON | flags, arch, cdir, platform_layers[This_OS], dir, "4ed", get_defines_from_flags(part, flags), 0, inc);
        END_TIME_SECTION("build 4ed");
    }
    
    if (update_local_theme){
        BEGIN_TIME_SECTION();
        char *themes_folder = fm_str(part, "../build/themes");
        char *source_themes_folder = fm_str(part, "themes");
        fm_clear_folder(themes_folder);
        fm_make_folder_if_missing(part, themes_folder);
        fm_copy_all(source_themes_folder, "*", themes_folder);
        END_TIME_SECTION("move files");
    }
}

internal void
standard_build(Partition *part, char *cdir, u32 flags, u32 arch){
    fsm_generator(part, cdir);
    metagen(part, cdir);
    //do_buildsuper(part, cdir, fm_str(part, custom_files[Custom_Default]), arch);
    do_buildsuper(part, cdir, fm_str(part, custom_files[Custom_Experiments]), arch);
    //do_buildsuper(part, cdir, fm_str(part, custom_files[Custom_Casey]), arch);
    //do_buildsuper(part, cdir, fm_str(part, custom_files[Custom_ChronalVim]), arch);
    build_main(part, cdir, true, flags, arch);
}

internal char*
get_4coder_dist_name(Partition *part, u32 platform, char *tier, u32 arch){
    char *name = fm_str(part, "4coder-alpha-" MAJOR_STR "-" MINOR_STR "-" PATCH_STR);
    if (strcmp(tier, "alpha") != 0){
        name = fm_str(part, name, "-", tier);
    }
    if (platform != Platform_None){
        name = fm_str(part, name, "-", platform_names[platform]);
    }
    if (arch != Arch_None){
        name = fm_str(part, name, "-", arch_names[arch]);
    }
    return(name);
}

internal void
package(Partition *part, char *cdir){
    // NOTE(allen): meta
    fsm_generator(part, cdir);
    metagen(part, cdir);
    
    char *build_dir = fm_str(part, BUILD_DIR);
    char *pack_dir = fm_str(part, PACK_DIR);
    char *fonts_source_dir = fm_str(part, "../4coder-non-source/dist_files/fonts");
    
    char *base_package_root = "../current_dist";
    
    // NOTE(allen): alpha and super builds
    enum{
        Tier_Alpha = 0,
        Tier_Super = 1,
        Tier_COUNT = 2
    };
    
    char *tiers[] = { "alpha", "super" };
    u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG;
    u32 tier_flags[] = { 0, SUPER, };
    
    for (u32 tier_index = 0; tier_index < Tier_COUNT; ++tier_index){
        char *tier = tiers[tier_index];
        u32 flags = base_flags | tier_flags[tier_index];
        
        Temp_Memory temp = begin_temp_memory(part);
        char *tier_package_root = fm_str(part, base_package_root, "_", tier);
        for (u32 arch = 0; arch < Arch_COUNT; ++arch){
            char *par_dir      = fm_str(part, tier_package_root, "_", arch_names[arch]);
            char *dir          = fm_str(part, par_dir, "/4coder");
            char *fonts_dir    = fm_str(part, dir, "/fonts");
            char *zip_dir      = fm_str(part, pack_dir, "/", tier, "_", arch_names[arch]);
            
            build_metadata();
            build_main(part, cdir, false, flags, arch);
            
            fm_clear_folder(par_dir);
            
            fm_make_folder_if_missing(part, dir);
            fm_copy_file(fm_str(part, build_dir, "/4ed" EXE), fm_str(part, dir, "/4ed" EXE));
            fm_copy_file(fm_str(part, build_dir, "/4ed_app" DLL), fm_str(part, dir, "/4ed_app" DLL));
            
            fm_copy_folder(part, cdir, dir, "themes");
            fm_copy_file(fm_str(part, cdir, "/LICENSE.txt"), fm_str(part, dir, "/LICENSE.txt"));
            fm_copy_file(fm_str(part, cdir, "/README.txt"), fm_str(part, dir, "/README.txt"));
            fm_copy_file(fm_str(part, cdir, "/changes.txt"), fm_str(part, dir, "/changes.txt"));
            
            fm_make_folder_if_missing(part, fonts_dir);
            fm_copy_all(fonts_source_dir, "*", fonts_dir);
            fm_copy_file(fm_str(part, cdir, "/release-config.4coder"), fm_str(part, dir, "/config.4coder"));
            
            if (tier_index == Tier_Super){
                fm_copy_all(0, "4coder_*", dir);
                
                do_buildsuper(part, cdir, fm_str(part, custom_files[Custom_Default]), arch);
                fm_copy_file(fm_str(part, build_dir, "/custom_4coder" DLL), fm_str(part, dir, "/custom_4coder" DLL));
                
                char *build_script = fm_str(part, "buildsuper_", arch_names[arch], BAT);
                fm_copy_file(build_script, fm_str(part, dir, "/buildsuper" BAT));
                
                if (This_OS == Platform_Windows){
                    fm_copy_folder(part, cdir, dir, "windows_scripts");
                }
                
                fm_copy_folder(part, cdir, dir, "4coder_API");
                fm_copy_folder(part, cdir, dir, "4coder_lib");
                fm_copy_folder(part, cdir, dir, "4coder_generated");
                fm_copy_folder(part, cdir, dir, "languages");
            }
            
            char *dist_name = get_4coder_dist_name(part, This_OS, tier, arch);
            char *zip_name = fm_str(part, zip_dir, "/", dist_name, ".zip");
            fm_make_folder_if_missing(part, zip_dir);
            fm_zip(par_dir, "4coder", zip_name);
        }
        end_temp_memory(temp);
    }
}

int main(int argc, char **argv){
    Partition part = fm_init_system();
    
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
    standard_build(&part, cdir, flags, arch);
    
#elif defined(PACKAGE)
    package(&part, cdir);
    
#elif defined(SITE_BUILD)
    site_build(&part, cdir, DEBUG_INFO);
    
#elif defined(STRING_BUILD)
    string_build(&part, cdir);
    
#else
# error No build type specified.
#endif
    
    return(error_state);
}

// BOTTOM

