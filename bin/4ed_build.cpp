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

#include "4coder_base_types.h"
#include "4coder_version.h"

#include "4coder_base_types.cpp"
#include "4coder_malloc_allocator.cpp"

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4coder_file_moving.h"


//
// OS and compiler index
//

typedef u32 Tier_Code;
enum{
    Tier_Demo,
    Tier_Super,
    Tier_COUNT,
};

char *tier_names[] = {
    "demo",
    "super",
};

typedef u32 Platform_Code;
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

typedef u32 Compiler_Code;
enum{
    Compiler_CL,
    Compiler_GCC,
    Compiler_Clang,
    //
    Compiler_COUNT,
    Compiler_None = Compiler_COUNT,
};

char *compiler_names[] = {
    "cl",
    "gcc",
    "clang",
};

typedef u32 Arch_Code;
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

#if OS_WINDOWS
# define This_OS Platform_Windows
#elif OS_LINUX
# define This_OS Platform_Linux
#elif OS_MAC
# define This_OS Platform_Mac
#else
# error This platform is not enumerated.
#endif

#if COMPILER_CL
# define This_Compiler Compiler_CL
#elif COMPILER_GCC
# define This_Compiler Compiler_GCC
#elif COMPILER_CLANG
# define This_Compiler Compiler_Clang
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

char *includes[] = { "custom", FOREIGN "/freetype2", 0, };

//
// Platform layer file tables
//

char *windows_platform_layer[] = { "platform_win32/win32_4ed.cpp", 0 };
char *linux_platform_layer[] = { "platform_linux/linux_4ed.cpp", 0 };
char *mac_platform_layer[] = { "platform_mac/mac_4ed.mm", 0 };

char **platform_layers[Platform_COUNT] = {
    windows_platform_layer,
    linux_platform_layer  ,
    mac_platform_layer    ,
};

char *windows_cl_platform_inc[] = { "platform_all", 0 };
char *linux_gcc_platform_inc[] = { "platform_all", "platform_unix", 0 };

char *mac_clang_platform_inc[] = { "platform_all", "platform_unix", 0 };

char **platform_includes[Platform_COUNT][Compiler_COUNT] = {
    {windows_cl_platform_inc, 0                     , 0},
    {0                      , linux_gcc_platform_inc, 0},
    {0                      , 0                     , mac_clang_platform_inc},
};

char *default_custom_target = "../code/custom/4coder_default_bindings.cpp";

// NOTE(allen): Build flags

enum{
    OPTS = 0x1,
    LIBS = 0x2,
    ICON = 0x4,
    SHARED_CODE = 0x8,
    DEBUG_INFO = 0x10,
    OPTIMIZATION = 0x20,
    SUPER = 0x40,
    INTERNAL = 0x80,
    SHIP = 0x100,
};

internal char**
get_defines_from_flags(Arena *arena, u32 flags){
    char **result = 0;
    if (HasFlag(flags, SHIP)){
        result = fm_list(arena, fm_list_one_item(arena, "SHIP_MODE"), result);
    }
    if (HasFlag(flags, INTERNAL)){
        result = fm_list(arena, fm_list_one_item(arena, "FRED_INTERNAL"), result);
    }
    if (HasFlag(flags, SUPER)){
        result = fm_list(arena, fm_list_one_item(arena, "FRED_SUPER"), result);
    }
    return(result);
}

//
// build implementation: cl
//

#if COMPILER_CL

#define CL_OPTS                                  \
"-W4 -wd4310 -wd4100 -wd4201 -wd4505 -wd4996 "   \
"-wd4127 -wd4510 -wd4512 -wd4610 -wd4390 "       \
"-wd4611 -wd4189 -WX -GR- -EHa- -nologo -FC"

#define CL_LIBS_X64                              \
"user32.lib winmm.lib gdi32.lib opengl32.lib comdlg32.lib "   \
FOREIGN_WIN "\\x64\\freetype.lib"

#define CL_LIBS_X86                              \
"user32.lib winmm.lib gdi32.lib opengl32.lib comdlg32.lib "   \
FOREIGN_WIN "\\x86\\freetype.lib"

#define CL_ICON "..\\4coder-non-source\\res\\icon.res"

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Temp_Dir temp = fm_pushdir(out_path);
    
    Build_Line line;
    fm_init_build_line(&line);
    
    if (arch == Arch_X86){
        fm_add_to_line(line, "%s\\custom\\bin\\setup_cl_x86.bat &", code_path);
    }
    
    fm_add_to_line(line, "cl");
    
    if (flags & OPTS){
        fm_add_to_line(line, CL_OPTS);
    }
    
    switch (arch){
        case Arch_X64: fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        case Arch_X86: fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        default: InvalidPath;
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(arena, code_path, "/", inc_folders[i]);
            fm_add_to_line(line, "-I%s", str);
        }
    }
    
    if (flags & LIBS){
        switch (arch){
            case Arch_X64: fm_add_to_line(line, CL_LIBS_X64); break;
            case Arch_X86: fm_add_to_line(line, CL_LIBS_X86); break;
            default: InvalidPath;
        }
    }
    
    if (flags & ICON){
        fm_add_to_line(line, CL_ICON);
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-Zi");
        fm_add_to_line(line, "-DDO_CRAZY_EXPENSIVE_ASSERTS");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "-O2");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "-LD");
    }
    
    if (defines != 0){
        for (u32 i = 0; defines[i] != 0; ++i){
            char *define_flag = fm_str(arena, "-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    for (u32 i = 0; code_files[i]; ++i){
        fm_add_to_line(line, "\"%s\\%s\"", code_path, code_files[i]);
    }
    
    fm_add_to_line(line, "-Fe%s", out_file);
    
    fm_add_to_line(line, "-link -INCREMENTAL:NO -RELEASE -PDBALTPATH:%%_PDB%%");
    switch (arch){
        case Arch_X64: fm_add_to_line(line, "-MACHINE:X64"); break;
        case Arch_X86: fm_add_to_line(line, "-MACHINE:X86"); break;
        default: InvalidPath;
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-DEBUG");
    }
    
    if (flags & SHARED_CODE){
        Assert(exports != 0);
        fm_add_to_line(line, "-OPT:REF");
        for (u32 i = 0; exports[i] != 0; ++i){
            char *str = fm_str(arena, "-EXPORT:", exports[i]);
            fm_add_to_line(line, "%s", str);
        }
    }
    else{
        fm_add_to_line(line, "-NODEFAULTLIB:library");
    }
    
    fm_finish_build_line(&line);
    
    //printf("%s\n", line.build_options);
    systemf("%s", line.build_options);
    fm_popdir(temp);
    
    fflush(stdout);
}

//
// build implementation: gcc
//

#elif COMPILER_GCC

#if OS_LINUX

# define GCC_OPTS                     \
"-Wno-write-strings "                 \
"-D_GNU_SOURCE -fPIC "                \
"-fno-threadsafe-statics -pthread "   \
"-Wno-unused-result "                 \
"-std=c++11"

#define GCC_LIBS_COMMON                        \
"-lX11 -lpthread -lm -lrt "   \
"-lGL -ldl -lXfixes -lfreetype -lfontconfig"

#define GCC_LIBS_X64 GCC_LIBS_COMMON
#define GCC_LIBS_X86 GCC_LIBS_COMMON

#else
# error gcc options not set for this platform
#endif

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Build_Line line;
    fm_init_build_line(&line);
    
    switch (arch){
        case Arch_X64:
        fm_add_to_line(line, "-m64");
        fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        
        case Arch_X86:
        fm_add_to_line(line, "-m32");
        fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        
        default: InvalidPath;
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, GCC_OPTS);
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(arena, code_path, "/", inc_folders[i]);
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
            char *define_flag = fm_str(arena, "-D", defines[i]);
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

#elif COMPILER_CLANG

#if OS_MAC

# define CLANG_OPTS \
"-Wno-write-strings -Wno-deprecated-declarations " \
"-Wno-comment -Wno-switch -Wno-null-dereference " \
"-Wno-tautological-compare -Wno-unused-result " \
"-Wno-missing-declarations -Wno-nullability-completeness " \
"-std=c++11 "

#define CLANG_LIBS_COMMON \
"-framework Cocoa -framework QuartzCore " \
"-framework CoreServices " \
"-framework OpenGL -framework IOKit -framework Metal -framework MetalKit "

#define CLANG_LIBS_X64 CLANG_LIBS_COMMON \
FOREIGN "/x64/libfreetype-mac.a"

#define CLANG_LIBS_X86 CLANG_LIBS_COMMON \
FOREIGN "/x86/libfreetype-mac.a"

#else
# error clang options not set for this platform
#endif

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Build_Line line;
    fm_init_build_line(&line);
    
    switch (arch){
        case Arch_X64:
        fm_add_to_line(line, "-m64");
        fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        
        case Arch_X86:
        fm_add_to_line(line, "-m32");
        fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        
        default: InvalidPath;
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, CLANG_OPTS);
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(arena, code_path, "/", inc_folders[i]);
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
            char *define_flag = fm_str(arena, "-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    fm_add_to_line(line, "-I\"%s\"", code_path);
    for (u32 i = 0; code_files[i] != 0; ++i){
        fm_add_to_line(line, "\"%s/%s\"", code_path, code_files[i]);
    }
    
    if (flags & LIBS){
        if (arch == Arch_X64){
            fm_add_to_line(line, CLANG_LIBS_X64);
        }
        else if (arch == Arch_X86)
        {
            fm_add_to_line(line, CLANG_LIBS_X86);
        }
    }
    
    fm_finish_build_line(&line);
    
    Temp_Dir temp = fm_pushdir(out_path);
    
    // systemf("clang++ %s -E -o %s", line.build_options, "4ed.i");
    systemf("clang++ %s -o %s", line.build_options, out_file);
    fm_popdir(temp);
}

#else
# error build function not defined for this compiler
#endif

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char *code_file, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    char **code_files = fm_list_one_item(arena, code_file);
    build(arena, flags, arch, code_path, code_files, out_path, out_file, defines, exports, inc_folders);
}

internal void
build_and_run(Arena *arena, char *cdir, char *filename, char *name, u32 flags){
    char *dir = fm_str(arena, BUILD_DIR);
    
    {
        char *file = fm_str(arena, filename);
        build(arena, flags, Arch_X64, cdir, file, dir, name, get_defines_from_flags(arena, flags), 0, includes);
    }
    
    if (prev_error == 0){
        char *cmd = fm_str(arena, dir, "/", name);
        fm_execute_in_dir(cdir, cmd, 0);
    }
}

internal void
buildsuper(Arena *arena, char *cdir, char *file, u32 arch){
    printf("BUILDSUPER:\n cdir = %s;\n file = %s;\n arch = %s;\n", cdir, file, arch_names[arch]);
    fflush(stdout);
    
    Temp_Dir temp = fm_pushdir(fm_str(arena, BUILD_DIR));
    
    char *build_script_postfix = "";
    switch (This_OS){
        case Platform_Windows:
        {
            build_script_postfix = "-win";
        }break;
        case Platform_Linux:
        {
            build_script_postfix = "-linux";
        }break;
        case Platform_Mac:
        {
            build_script_postfix = "-mac";
        }break;
    }
    char *build_script = fm_str(arena, "custom/bin/buildsuper_", arch_names[arch], build_script_postfix, BAT);
    
    char *build_command = fm_str(arena, "\"", cdir, "/", build_script, "\" \"", file, "\"");
    if (This_OS == Platform_Windows){
        build_command = fm_str(arena, "call ", build_command);
    }
    systemf("%s", build_command);
    
    fm_popdir(temp);
    fflush(stdout);
}

internal void
build_main(Arena *arena, char *cdir, b32 update_local_theme, u32 flags, u32 arch){
    char *dir = fm_str(arena, BUILD_DIR);
    
    {
        char *file = fm_str(arena, "4ed_app_target.cpp");
        char **exports = fm_list_one_item(arena, "app_get_functions");
        
        char **build_includes = includes;
        
        build(arena, OPTS | SHARED_CODE | flags, arch, cdir, file, dir, "4ed_app" DLL, get_defines_from_flags(arena, flags), exports, build_includes);
    }
    
    {
        char **inc = (char**)fm_list(arena, includes, platform_includes[This_OS][This_Compiler]);
        build(arena, OPTS | LIBS | ICON | flags, arch, cdir, platform_layers[This_OS], dir, "4ed", get_defines_from_flags(arena, flags), 0, inc);
    }
    
    if (update_local_theme){
        char *themes_folder = fm_str(arena, "../build/themes");
        char *source_themes_folder = fm_str(arena, "ship_files/themes");
        fm_clear_folder(themes_folder);
        fm_make_folder_if_missing(arena, themes_folder);
        fm_copy_all(source_themes_folder, themes_folder);
    }
    
    fflush(stdout);
}

internal void
standard_build(Arena *arena, char *cdir, u32 flags, u32 arch){
    buildsuper(arena, cdir, fm_str(arena, default_custom_target), arch);
    build_main(arena, cdir, true, flags, arch);
}

internal char*
get_4coder_dist_name(Arena *arena, u32 platform, char *tier, u32 arch){
    char *name = fm_str(arena, "4coder-" MAJOR_STR "-" MINOR_STR "-" PATCH_STR "-", tier);
    if (platform != Platform_None){
        name = fm_str(arena, name, "-", platform_names[platform]);
    }
    if (arch != Arch_None){
        name = fm_str(arena, name, "-", arch_names[arch]);
    }
    return(name);
}

function void
package_for_arch(Arena *arena, u32 arch, char *cdir, char *build_dir, char *pack_dir, i32 tier, char *tier_name,  char *current_dist_tier, u32 flags, char** dist_files, i32 dist_file_count){
    char *arch_name = arch_names[arch];
    char *parent_dir = fm_str(arena, current_dist_tier, "_", arch_name);
    char *dir        = fm_str(arena, parent_dir, SLASH "4coder");
    char *zip_dir    = fm_str(arena, pack_dir, SLASH, tier_name, "_", arch_name);
    
    printf("\nBUILD: %s_%s\n", tier_name, arch_name);
    printf(" parent_dir = %s;\n", parent_dir);
    printf(" dir = %s;\n", dir);
    printf(" zip_dir = %s;\n", zip_dir);
    fflush(stdout);
    
    buildsuper(arena, cdir, fm_str(arena, default_custom_target), arch);
    build_main(arena, cdir, false, flags, arch);
    
    fm_clear_folder(parent_dir);
    fm_make_folder_if_missing(arena, parent_dir);
    
    fm_make_folder_if_missing(arena, dir);
    fm_copy_file(fm_str(arena, build_dir, "/4ed" EXE), fm_str(arena, dir, "/4ed" EXE));
    fm_copy_file(fm_str(arena, build_dir, "/4ed_app" DLL), fm_str(arena, dir, "/4ed_app" DLL));
    fm_copy_file(fm_str(arena, build_dir, "/custom_4coder" DLL), fm_str(arena, dir, "/custom_4coder" DLL));
    
    if (tier == Tier_Demo){
        dist_file_count -= 1;
    }
    
    for (i32 j = 0; j < dist_file_count; j += 1){
        fm_copy_all(dist_files[j], dir);
    }
    
    if (tier == Tier_Super){
        char *custom_src_dir = fm_str(arena, cdir, SLASH, "custom");
        char *custom_dst_dir = fm_str(arena, dir, SLASH, "custom");
        fm_make_folder_if_missing(arena, custom_dst_dir);
        fm_copy_all(custom_src_dir, custom_dst_dir);
    }
    
    char *dist_name = get_4coder_dist_name(arena, This_OS, tier_name, arch);
    char *zip_name = fm_str(arena, zip_dir, SLASH, dist_name, ".zip");
    fm_make_folder_if_missing(arena, zip_dir);
    fm_zip(parent_dir, "4coder", zip_name);
}

internal u32
tier_flags(Tier_Code code){
    u32 result = 0;
    switch (code){
        case Tier_Super:
        {
            result = SUPER;
        }break;
    }
    return(result);
}

internal void
package(Arena *arena, char *cdir, Tier_Code tier, Arch_Code arch){
    // NOTE(allen): meta
    char *build_dir = fm_str(arena, BUILD_DIR);
    char *pack_dir = fm_str(arena, PACK_DIR);
    char *dist_files[3];
    dist_files[0] = fm_str(arena, "../4coder-non-source/dist_files");
    dist_files[1] = fm_str(arena, "ship_files");
    dist_files[2] = fm_str(arena, "ship_files_super");
    
    printf("build dir: %s\n", build_dir);
    printf("pack dir: %s\n", pack_dir);
    printf("dist files: %s, %s, %s\n", dist_files[0], dist_files[1], dist_files[2]);
    fflush(stdout);
    
    u32 base_flags = SHIP | DEBUG_INFO | OPTIMIZATION;
    
    fm_make_folder_if_missing(arena, pack_dir);
    
    char *tier_name = tier_names[tier];
    u32 flags = base_flags | tier_flags(tier);
    Temp_Memory temp = begin_temp(arena);
    char *current_dist_tier = fm_str(arena, ".." SLASH "current_dist_", tier_name);
    package_for_arch(arena, arch, cdir, build_dir, pack_dir, tier, tier_name, current_dist_tier, flags, dist_files, ArrayCount(dist_files));
    end_temp(temp);
}

int main(int argc, char **argv){
    Arena arena = fm_init_system(DetailLevel_FileOperations);
    
    char cdir[256];
    i32 n = fm_get_current_directory(cdir, sizeof(cdir));
    Assert(n < sizeof(cdir));
    
    u32 flags = SUPER;
    u32 arch = Arch_X64;
#if defined(DEV_BUILD) || defined(DEV_BUILD_X86)
    flags |= DEBUG_INFO | INTERNAL;
#endif
#if defined(OPT_BUILD) || defined(OPT_BUILD_X86)
    flags |= OPTIMIZATION;
#endif
#if defined(DEV_BUILD_X86) || defined(OPT_BUILD_X86)
    arch = Arch_X86;
#endif
    
#if defined(DEV_BUILD) || defined(OPT_BUILD) || defined(DEV_BUILD_X86) || defined(OPT_BUILD_X86)
    standard_build(&arena, cdir, flags, arch);
    
#elif defined(PACKAGE_DEMO_X64)
    package(&arena, cdir, Tier_Demo, Arch_X64);
    
#elif defined(PACKAGE_DEMO_X86)
    package(&arena, cdir, Tier_Demo, Arch_X86);
    
#elif defined(PACKAGE_SUPER_X64)
    package(&arena, cdir, Tier_Super, Arch_X64);
    
#elif defined(PACKAGE_SUPER_X86)
    package(&arena, cdir, Tier_Super, Arch_X86);
    
#else
# error No build type specified.
#endif
    
    return(error_state);
}

// BOTTOM

