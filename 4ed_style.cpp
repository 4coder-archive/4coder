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

inline u32*
style_index_by_tag(Style *s, u32 tag){
    u32 *result = 0;
    switch (tag){
    case Stag_Bar: result = &s->main.file_info_style.bar_color; break;
    case Stag_Bar_Active: result = &s->main.file_info_style.bar_active_color; break;
    case Stag_Base: result = &s->main.file_info_style.base_color; break;
    case Stag_Pop1: result = &s->main.file_info_style.pop1_color; break;
    case Stag_Pop2: result = &s->main.file_info_style.pop2_color; break;
            
    case Stag_Back: result = &s->main.back_color; break;
    case Stag_Margin: result = &s->main.margin_color; break;
    case Stag_Margin_Hover: result = &s->main.margin_hover_color; break;
    case Stag_Margin_Active: result = &s->main.margin_active_color; break;
            
    case Stag_Cursor: result = &s->main.cursor_color; break;
    case Stag_At_Cursor: result = &s->main.at_cursor_color; break;
    case Stag_Highlight: result = &s->main.highlight_color; break;
    case Stag_At_Highlight: result = &s->main.at_highlight_color; break;
    case Stag_Mark: result = &s->main.mark_color; break;
        
    case Stag_Default: result = &s->main.default_color; break;
    case Stag_Comment: result = &s->main.comment_color; break;
    case Stag_Keyword: result = &s->main.keyword_color; break;
    case Stag_Str_Constant: result = &s->main.str_constant_color; break;
    case Stag_Char_Constant: result = &s->main.char_constant_color; break;
    case Stag_Int_Constant: result = &s->main.int_constant_color; break;
    case Stag_Float_Constant: result = &s->main.float_constant_color; break;
    case Stag_Bool_Constant: result = &s->main.bool_constant_color; break;
        
    case Stag_Preproc: result = &s->main.preproc_color; break;
    case Stag_Include: result = &s->main.include_color; break;
        
    case Stag_Special_Character: result = &s->main.special_character_color; break;
        
    case Stag_Highlight_Junk: result = &s->main.highlight_junk_color; break;
    case Stag_Highlight_White: result = &s->main.highlight_white_color; break;
        
    case Stag_Paste: result = &s->main.paste_color; break;
    case Stag_Undo: result = &s->main.undo_color; break;
    }
    return result;
}

struct Style_Library{
    Style styles[64];
    i32 count, max;
};

#if 0
struct Style_Color_Specifier{
    u32 tag;
    u32 color;
};

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

