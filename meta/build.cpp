/*
4coder development build rule.
*/

// TOP

#include "../4tech_defines.h"
#include "4tech_file_moving.h"

#include <assert.h>
#include <string.h>

#include "../4coder_API/version.h"

#define FSTRING_IMPLEMENTATION
#include "../4coder_lib/4coder_string.h"

//
// reusable
//

#define IS_64BIT

#define LLU_CAST(n) (long long unsigned int)(n)

#define BEGIN_TIME_SECTION() uint64_t start = get_time()
#define END_TIME_SECTION(n) uint64_t total = get_time() - start; printf("%-20s: %.2llu.%.6llu\n", (n), LLU_CAST(total/1000000), LLU_CAST(total%1000000));

//
// 4coder specific
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
    swap_ptr(&line.build_options,           \
    &line.build_options_prev);              \
}while(0)

#elif defined(IS_GCC)

#define build_ap(line, str, ...) do{                         \
    snprintf(line.build_options, line.build_max, "%s "str,   \
    line.build_options_prev, ##__VA_ARGS__);        \
    swap_ptr(&line.build_options, &line.build_options_prev); \
}while(0)

#endif

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
build_cl(u32 flags, char *code_path, char *code_file, char *out_path, char *out_file, char *exports){
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
    
    swap_ptr(&line.build_options, &line.build_options_prev);
    swap_ptr(&link_line.build_options, &link_line.build_options_prev);
    swap_ptr(&line_prefix.build_options, &line_prefix.build_options_prev);
    
    Temp_Dir temp = pushdir(out_path);
    systemf("%scl %s \"%s\\%s\" /Fe%s /link /INCREMENTAL:NO %s", line_prefix.build_options, line.build_options, code_path, code_file, out_file, link_line.build_options);
    popdir(temp);
}

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
build_gcc(u32 flags, char *code_path, char *code_file, char *out_path, char *out_file, char *exports){
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
        // TODO(allen): Abstract this out.
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
#endif
    }
    
    if (flags & SITE_INCLUDES){
        build_ap(line, GCC_SITE_INCLUDES);
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
    build_ap(line, "\"%s/%s\"", code_path, code_file);
    
    if (flags & LIBS){
        build_ap(line, GCC_LIBS);
    }
    
    swap_ptr(&line.build_options, &line.build_options_prev);
    
    Temp_Dir temp = pushdir(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    popdir(temp);
}

static void
build(u32 flags, char *code_path, char *code_file, char *out_path, char *out_file, char *exports){
#if defined(IS_CL)
    build_cl(flags, code_path, code_file, out_path, out_file, exports);
#elif defined(IS_GCC)
    build_gcc(flags, code_path, code_file, out_path, out_file, exports);
#else
#error The build rule for this compiler is not ready
#endif
}

static void
buildsuper(char *code_path, char *out_path, char *filename, b32 x86_build){
    Temp_Dir temp = pushdir(out_path);
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
    popdir(temp);
}

#if defined(IS_WINDOWS)
#define PLAT_LAYER "win32_4ed.cpp"
#elif defined(IS_LINUX)
#define PLAT_LAYER "platform_linux/linux_4ed.cpp"
#elif defined(IS_MAC)
#define PLAT_LAYER "platform_mac/mac_4ed.m"
#else
#error No platform layer defined for this OS.
#endif

#define BUILD_DIR "../build"

static void
fsm_generator(char *cdir){
    {
        DECL_STR(file, "meta/fsm_table_generator.cpp");
        DECL_STR(dir, BUILD_DIR);
        
        BEGIN_TIME_SECTION();
        build(OPTS | DEBUG_INFO, cdir, file, dir, "fsmgen", 0);
        END_TIME_SECTION("build fsm generator");
    }
    
    if (prev_error == 0){
        DECL_STR(cmd, BUILD_DIR"/fsmgen");
        BEGIN_TIME_SECTION();
        execute_in_dir(cdir, cmd, 0);
        END_TIME_SECTION("run fsm generator");
    }
}

static void
metagen(char *cdir){
    {
        DECL_STR(file, "meta/4ed_metagen.cpp");
        DECL_STR(dir, BUILD_DIR);
        
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | DEBUG_INFO, cdir, file, dir, "metagen", 0);
        END_TIME_SECTION("build metagen");
    }
    
    if (prev_error == 0){
        DECL_STR(cmd, BUILD_DIR"/metagen");
        BEGIN_TIME_SECTION();
        execute_in_dir(cdir, cmd, 0);
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
    
    DECL_STR(dir, BUILD_DIR);
    buildsuper(cdir, dir, str.str, x86_build);
    
    END_TIME_SECTION("build custom");
}

static void
build_main(char *cdir, u32 flags){
    DECL_STR(dir, BUILD_DIR);
    
    {
        DECL_STR(file, "4ed_app_target.cpp");
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | SHARED_CODE | flags, cdir, file, dir, "4ed_app" DLL, "/EXPORT:app_get_functions");
        END_TIME_SECTION("build 4ed_app");
    }
    
    {
        BEGIN_TIME_SECTION();
        build(OPTS | INCLUDES | LIBS | ICON | flags, cdir, PLAT_LAYER, dir, "4ed", 0);
        END_TIME_SECTION("build 4ed");
    }
    
    {
        BEGIN_TIME_SECTION();
        DECL_STR(themes_folder, "../build/themes");
        
        DECL_STR(source_themes_folder, "themes");
        clear_folder(themes_folder);
        make_folder_if_missing(themes_folder, 0);
        copy_all(source_themes_folder, "*", themes_folder);
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
        DECL_STR(file, "site/sitegen.cpp");
        DECL_STR(dir, BUILD_DIR"/site");
        BEGIN_TIME_SECTION();
        build(OPTS | SITE_INCLUDES | flags, cdir, file, dir, "sitegen", 0);
        END_TIME_SECTION("build sitegen");
    }
    
    {
        BEGIN_TIME_SECTION();
        DECL_STR(cmd, "../build/site/sitegen . ../site_resources site/source_material ../site");
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
#elif defined(IS_LINUX) && defined(IS_64BIT)
        append_sc(zip_file, "-linux");
#elif defined(IS_MAC) && defined(IS_64BIT)
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
    
    slash_fix(zip_file->str);
}

static void
copy_folder(char *dst_dir, char *src_folder){
    make_folder_if_missing(dst_dir, src_folder);
    
    char space[256];
    String copy_name = make_fixed_width_string(space);
    append_sc(&copy_name, dst_dir);
    append_s_char(&copy_name, platform_correct_slash);
    append_sc(&copy_name, src_folder);
    terminate_with_null(&copy_name);
    
    copy_all(src_folder, "*", copy_name.str);
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
    
    DECL_STR(build_dir, BUILD_DIR);
    DECL_STR(site_dir, SITE_DIR);
    DECL_STR(pack_dir, PACK_DIR);
    DECL_STR(pack_fonts_dir, PACK_FONTS_DIR);
    
#define PACK_ALPHA_PAR_DIR   "../current_dist"
#define PACK_ALPHA_DIR       PACK_ALPHA_PAR_DIR"/4coder"
#define PACK_ALPHA_FONTS_DIR PACK_ALPHA_DIR"/fonts"
    DECL_STR(pack_alpha_par_dir, PACK_ALPHA_PAR_DIR);
    DECL_STR(pack_alpha_dir, PACK_ALPHA_DIR);
    DECL_STR(pack_alpha_fonts_dir, PACK_ALPHA_FONTS_DIR);
    
#define PACK_ALPHA_X86_PAR_DIR   "../current_dist_x86"
#define PACK_ALPHA_X86_DIR       PACK_ALPHA_X86_PAR_DIR"/4coder"
#define PACK_ALPHA_X86_FONTS_DIR PACK_ALPHA_X86_DIR"/fonts"
    DECL_STR(pack_alpha_x86_par_dir, PACK_ALPHA_X86_PAR_DIR);
    DECL_STR(pack_alpha_x86_dir, PACK_ALPHA_X86_DIR);
    DECL_STR(pack_alpha_x86_fonts_dir, PACK_ALPHA_X86_FONTS_DIR);
    
    // NOTE(allen): alpha
    {
        char *dest_dirs[] = {
            pack_alpha_dir,
            pack_alpha_x86_dir,
        };
        
        char *dest_par_dirs[] = {
            pack_alpha_par_dir,
            pack_alpha_x86_par_dir,
        };
        
        char *dest_fonts_dirs[] = {
            pack_alpha_fonts_dir,
            pack_alpha_x86_fonts_dir,
        };
        
        char *zip_dirs[] = {
            "alpha",
            "alpha_x86",
        };
        
        char *tier = "alpha";
        
        char *archs[] = {
            "x64",
            "x86",
        };
        
        Assert(ArrayCount(dest_dirs) == ArrayCount(dest_par_dirs));
        u32 count = ArrayCount(dest_dirs);
        
        u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | LOG;
        u32 flags[] = {
            0,
            X86,
        };
        
        for (u32 i = 0; i < count; ++i){
            char *dir = dest_dirs[i];
            char *par_dir = dest_par_dirs[i];
            char *fonts_dir = dest_fonts_dirs[i];
            char *zip_dir = zip_dirs[i];
            char *arch = archs[i];
            
            build_main(cdir, base_flags | flags[i]);
            
            clear_folder(par_dir);
            make_folder_if_missing(dir, 0);
            make_folder_if_missing(dir, "fonts");
            make_folder_if_missing(pack_dir, zip_dir);
            copy_file(build_dir, "4ed" EXE, dir, 0, 0);
            copy_file(build_dir, "4ed_app" DLL, dir, 0, 0);
            copy_all(pack_fonts_dir, "*", fonts_dir);
            copy_file(cdir, "release-config.4coder", dir, 0, "config.4coder");
            
            copy_folder(dir, "themes");
            
            copy_file(cdir, "LICENSE.txt", dir, 0, 0);
            copy_file(cdir, "README.txt", dir, 0, 0);
            
            get_4coder_dist_name(&str, true, zip_dir, tier, arch, "zip");
            zip(par_dir, "4coder", str.str);
        }
    }
    
    // NOTE(allen): super
#define PACK_SUPER_PAR_DIR   "../current_dist_super"
#define PACK_SUPER_DIR       PACK_SUPER_PAR_DIR"/4coder"
#define PACK_SUPER_FONTS_DIR PACK_SUPER_DIR"/fonts"
    DECL_STR(pack_super_par_dir, PACK_SUPER_PAR_DIR);
    DECL_STR(pack_super_dir, PACK_SUPER_DIR);
    DECL_STR(pack_super_fonts_dir, PACK_SUPER_FONTS_DIR);
    
#define PACK_SUPER_X86_PAR_DIR   "../current_dist_super_x86"
#define PACK_SUPER_X86_DIR       PACK_SUPER_X86_PAR_DIR"/4coder"
#define PACK_SUPER_X86_FONTS_DIR PACK_SUPER_X86_DIR"/fonts"
    DECL_STR(pack_super_x86_par_dir, PACK_SUPER_X86_PAR_DIR);
    DECL_STR(pack_super_x86_dir, PACK_SUPER_X86_DIR);
    DECL_STR(pack_super_x86_fonts_dir, PACK_SUPER_X86_FONTS_DIR);
    
    {
        char *dest_dirs[] = {
            pack_super_dir,
            pack_super_x86_dir,
        };
        
        char *dest_par_dirs[] = {
            pack_super_par_dir,
            pack_super_x86_par_dir,
        };
        
        char *dest_fonts_dirs[] = {
            pack_super_fonts_dir,
            pack_super_x86_fonts_dir,
        };
        
        char *zip_dirs[] = {
            "super",
            "super_x86",
        };
        
        char *tier = "super";
        
        char *archs[] = {
            "x64",
            "x86",
        };
        
        Assert(ArrayCount(dest_dirs) == ArrayCount(dest_par_dirs));
        u32 count = ArrayCount(dest_dirs);
        
        u32 base_flags = OPTIMIZATION | KEEP_ASSERT | DEBUG_INFO | SUPER | LOG;
        u32 flags[] = {
            0,
            X86,
        };
        
        for (u32 i = 0; i < count; ++i){
            char *dir = dest_dirs[i];
            char *par_dir = dest_par_dirs[i];
            char *fonts_dir = dest_fonts_dirs[i];
            char *zip_dir = zip_dirs[i];
            char *arch = archs[i];
            
            build_main(cdir, base_flags | flags[i]);
            do_buildsuper(cdir, Custom_Default, flags[i]);
            
            clear_folder(par_dir);
            make_folder_if_missing(dir, 0);
            make_folder_if_missing(dir, "fonts");
            make_folder_if_missing(pack_dir, zip_dir);
            
            copy_file(build_dir, "4ed" EXE, dir, 0, 0);
            copy_file(build_dir, "4ed_app" DLL, dir, 0, 0);
            copy_file(build_dir, "custom_4coder" DLL, dir, 0, 0);
            copy_all(pack_fonts_dir, "*", fonts_dir);
            copy_file(cdir, "release-config.4coder", dir, 0, "config.4coder");
            
            copy_all(0, "4coder_*", dir);
            
            if (!(flags[i] & X86)){
                copy_file(0, "buildsuper" BAT, dir, 0, 0);
            }
            else{
                copy_file(0, "buildsuper_x86" BAT, dir, 0, "buildsuper" BAT);
            }
            
#if defined(IS_WINDOWS)
            copy_folder(dir, "windows_scripts");
#endif
            
            copy_folder(dir, "4coder_API");
            copy_folder(dir, "4coder_helper");
            copy_folder(dir, "4coder_lib");
            copy_folder(dir, "4cpp");
            copy_folder(dir, "languages");
            copy_folder(dir, "themes");
            
            copy_file(cdir, "LICENSE.txt", dir, 0, 0);
            copy_file(cdir, "README.txt", dir, 0, 0);
            
            get_4coder_dist_name(&str, true, zip_dir, tier, arch, "zip");
            zip(par_dir, "4coder", str.str);
        }
        
        make_folder_if_missing(pack_dir, "super-docs");
        get_4coder_dist_name(&str, false, "super-docs", "API", 0, "html");
        String str2 = front_of_directory(str);
        copy_file(site_dir, "custom_docs.html", pack_dir, "super-docs", str2.str);
    }
    
    // NOTE(allen): power
#define PACK_POWER_PAR_DIR "../current_dist_power"
#define PACK_POWER_DIR PACK_POWER_PAR_DIR"/power"
    DECL_STR(pack_power_par_dir, PACK_POWER_PAR_DIR);
    DECL_STR(pack_power_dir, PACK_POWER_DIR);
    
    clear_folder(pack_power_par_dir);
    make_folder_if_missing(pack_power_dir, 0);
    make_folder_if_missing(pack_dir, "power");
    copy_all("power", "*", pack_power_dir);
    
    get_4coder_dist_name(&str, 0, "power", 0, 0, "zip");
    zip(pack_power_par_dir, "power", str.str);
}

#if defined(DEV_BUILD) || defined(OPT_BUILD)

int main(int argc, char **argv){
    init_time_system();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    i32 n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
    u32 flags = DEBUG_INFO | SUPER | INTERNAL | LOG;
#if defined(OPT_BUILD)
    flags |= OPTIMIZATION;
#endif
    
    standard_build(cdir, flags);
    
    return(error_state);
}

#elif defined(DEV_BUILD_X86)

int main(int argc, char **argv){
    init_time_system();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    i32 n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
    u32 flags = DEBUG_INFO | SUPER | INTERNAL | X86 | LOG;
    
    standard_build(cdir, flags);
    
    return(error_state);
}

#elif defined(PACKAGE)

int main(int argc, char **argv){
    init_time_system();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    i32 n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
    package(cdir);
    
    return(error_state);
}

#elif defined(SITE_BUILD)

int main(int argc, char **argv){
    init_time_system();
    
    char cdir[256];
    
    BEGIN_TIME_SECTION();
    i32 n = get_current_directory(cdir, sizeof(cdir));
    assert(n < sizeof(cdir));
    END_TIME_SECTION("current directory");
    
    site_build(cdir, DEBUG_INFO);
    
    return(error_state);
}

#else
#error No build type specified
#endif

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4tech_file_moving.h"

// BOTTOM


