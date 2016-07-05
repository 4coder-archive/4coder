#undef internal
#include <fontconfig/fontconfig.h>
#define internal static

//TODO(inso): put in linuxvars
static FcConfig* fc;

internal char*
linux_get_sys_font(char* name, i32 pt_size){
    char* result = 0;

    if(!fc){
        fc = FcInitLoadConfigAndFonts();
    }

    FcPattern* pat = FcPatternBuild(
        NULL,
        FC_POSTSCRIPT_NAME, FcTypeString, name,
        FC_SIZE,            FcTypeDouble, (double)pt_size,
        FC_FONTFORMAT,      FcTypeString, "TrueType",
        NULL
    );

    FcConfigSubstitute(fc, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult res;
    FcPattern* font = FcFontMatch(fc, pat, &res);
    FcChar8* fname = 0;

    if(font){
        FcPatternGetString(font, FC_FILE, 0, &fname);
        if(fname){
            result = strdup((char*)fname);
            fprintf(stderr, "Got system font from FontConfig: %s\n", result);
        }
        FcPatternDestroy(font);
    }

    FcPatternDestroy(pat);

    if(!result){
        char space[1024];
        String str = make_fixed_width_string(space);
        if(sysshared_to_binary_path(&str, name)){
            result =  strdup(space);
        } else {
            result = strdup(name);
        }
    }

    return result;
}

internal b32
linux_font_load(Partition *part, Render_Font *rf, char *name, i32 pt_size, i32 tab_width, b32 use_hinting){
    b32 result = 0;

    Temp_Memory temp = begin_temp_memory(part);

#if 0
    char* filename = linux_get_sys_font(name, pt_size);
#else
    char* filename = push_array(part, char, 256);
    if (filename != 0){
        String str = make_string(filename, 0, 256);
        sysshared_to_binary_path(&str, name);
    }
#endif

    if (filename != 0){
        result = font_load_freetype(part, rf, filename, pt_size, tab_width, use_hinting);
    }

    end_temp_memory(temp);

    return(result);
}
