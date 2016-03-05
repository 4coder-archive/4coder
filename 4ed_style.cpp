/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.08.2015
 *
 * Styles for 4coder
 *
 */

// TOP

// TODO(allen):
//  Font changing UI should be in the library menu now, it's not tied to the fonts any more
//  Get the import export stuff up and running for styles again

struct Style_Font{
    i16 font_id;
    i16 font_changed;
};

struct Style{
    char name_[24];
    String name;
    Style_Main_Data main;
};

internal void
style_copy(Style *dst, Style *src){
    *dst = *src;
    dst->name.str = dst->name_;
}

internal void
style_set_name(Style *style, String name){
    i32 count = ArrayCount(style->name_);
    style->name = make_string(style->name_, 0, count - 1);
    copy(&style->name, name);
    terminate_with_null(&style->name);
}

struct Style_Library{
    Style styles[64];
    i32 count, max;
};

#if 0
struct Style_File_Format{
    i32 name_size;
    char name[24];
    
    i32 color_specifier_count;
};

internal b32
style_library_add(Style_Library *library, Style *style){
    b32 result = 0;
    i32 count = library->count;
    String my_name = style->name;
    Style *ostyle = library->styles;
    Style *out = 0;
    // TODO(allen): hashtable for name lookup?
    for (i32 i = 0; i < count; ++i, ++ostyle){
        if (match(my_name, ostyle->name)){
            out = ostyle;
            break;
        }
    }
    if (!out && count < library->max){
        out = library->styles + library->count++;
    }
    if (out){
        style_copy(out, style);
        result = 1;
    }
    return result;
}

internal Style_File_Format*
style_format_for_file(Font_Set *set, Style *style, Style_File_Format *out){
    out->name_size = style->name.size;
    memcpy(out->name, style->name.str, ArrayCount(out->name));
    
    Style_Color_Specifier *spec = (Style_Color_Specifier*)(out + 1);
    i32 count = 0;
    
    for (u32 i = 0; i < Stag_Count; ++i){
        u32 *color = style_index_by_tag(style, i);
        if (color){
            spec->tag = i;
            spec->color = *color;
            ++count;
            ++spec;
        }
    }
    out->color_specifier_count = count;
    
    return (Style_File_Format*)spec;
}
#endif

// BOTTOM

