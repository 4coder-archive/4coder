/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.08.2015
 *
 * Styles for 4coder
 *
 */

// TOP

struct Style_Main_Data{
    u32 back_color;
	u32 margin_color;
	u32 margin_hover_color;
	u32 margin_active_color;
	u32 cursor_color;
	u32 at_cursor_color;
	u32 highlight_color;
	u32 at_highlight_color;
	u32 mark_color;
	u32 default_color;
	u32 comment_color;
	u32 keyword_color;
	u32 str_constant_color;
	u32 char_constant_color;
	u32 int_constant_color;
	u32 float_constant_color;
	u32 bool_constant_color;
    u32 preproc_color;
	u32 include_color;
	u32 special_character_color;
	u32 highlight_junk_color;
	u32 highlight_white_color;
    u32 paste_color;
    u32 undo_color;
    Interactive_Style file_info_style;
};

struct Style_File_Format_v4{
    i32 name_size;
    char name[24];
    i32 font_name_size;
    char font_name[24];
    Style_Main_Data main;
};

enum Style_Color_Tag{
    STAG_BAR_COLOR,
    STAG_BAR_ACTIVE_COLOR,
    STAG_BAR_BASE_COLOR,
    STAG_BAR_POP1_COLOR,
    STAG_BAR_POP2_COLOR,
    STAG_BACK_COLOR,
	STAG_MARGIN_COLOR,
	STAG_MARGIN_HOVER_COLOR,
	STAG_MARGIN_ACTIVE_COLOR,
	STAG_CURSOR_COLOR,
	STAG_AT_CURSOR_COLOR,
	STAG_HIGHLIGHT_COLOR,
	STAG_AT_HIGHLIGHT_COLOR,
	STAG_MARK_COLOR,
	STAG_DEFAULT_COLOR,
	STAG_COMMENT_COLOR,
	STAG_KEYWORD_COLOR,
	STAG_STR_CONSTANT_COLOR,
	STAG_CHAR_CONSTANT_COLOR,
	STAG_INT_CONSTANT_COLOR,
	STAG_FLOAT_CONSTANT_COLOR,
	STAG_BOOL_CONSTANT_COLOR,
    STAG_PREPROC_COLOR,
	STAG_INCLUDE_COLOR,
	STAG_SPECIAL_CHARACTER_COLOR,
	STAG_HIGHLIGHT_JUNK_COLOR,
	STAG_HIGHLIGHT_WHITE_COLOR,
    STAG_PASTE_COLOR,
    STAG_UNDO_COLOR,
    STAG_NEXT_UNDO_COLOR,
    STAG_RESULT_LINK_COLOR,
    STAG_RELATED_LINK_COLOR,
    STAG_ERROR_LINK_COLOR,
    STAG_WARNING_LINK_COLOR,
    // never below this
    STAG_COUNT
};

struct Style_Color_Specifier{
    u32 tag;
    u32 color;
};

struct Style_File_Format{
    i32 name_size;
    char name[24];
    i32 font_name_size;
    char font_name[24];
    
    i32 color_specifier_count;
};

struct Style{
    char name_[24];
    String name;
    Style_Main_Data main;
    b32 font_changed;
    i16 font_id;
};

struct Style_Library{
    Style styles[64];
    i32 count, max;
};

internal void
style_copy(Style *dst, Style *src){
    *dst = *src;
    dst->name.str = dst->name_;
}

internal void
style_set_name(Style *style, String name){
    i32 count = ArrayCount(style->name_);
    style->name_[count - 1] = 0;
    style->name = make_string(style->name_, 0, count - 1);
    copy(&style->name, name);
}

inline u32*
style_index_by_tag(Style *s, u32 tag){
    u32 *result = 0;
    switch (tag){
    case STAG_BAR_COLOR: result = &s->main.file_info_style.bar_color; break;
    case STAG_BAR_ACTIVE_COLOR: result = &s->main.file_info_style.bar_active_color; break;
    case STAG_BAR_BASE_COLOR: result = &s->main.file_info_style.base_color; break;
    case STAG_BAR_POP1_COLOR: result = &s->main.file_info_style.pop1_color; break;
    case STAG_BAR_POP2_COLOR: result = &s->main.file_info_style.pop2_color; break;
            
    case STAG_BACK_COLOR: result = &s->main.back_color; break;
    case STAG_MARGIN_COLOR: result = &s->main.margin_color; break;
    case STAG_MARGIN_HOVER_COLOR: result = &s->main.margin_hover_color; break;
    case STAG_MARGIN_ACTIVE_COLOR: result = &s->main.margin_active_color; break;
            
    case STAG_CURSOR_COLOR: result = &s->main.cursor_color; break;
    case STAG_AT_CURSOR_COLOR: result = &s->main.at_cursor_color; break;
    case STAG_HIGHLIGHT_COLOR: result = &s->main.highlight_color; break;
    case STAG_AT_HIGHLIGHT_COLOR: result = &s->main.at_highlight_color; break;
    case STAG_MARK_COLOR: result = &s->main.mark_color; break;
        
    case STAG_DEFAULT_COLOR: result = &s->main.default_color; break;
    case STAG_COMMENT_COLOR: result = &s->main.comment_color; break;
    case STAG_KEYWORD_COLOR: result = &s->main.keyword_color; break;
    case STAG_STR_CONSTANT_COLOR: result = &s->main.str_constant_color; break;
    case STAG_CHAR_CONSTANT_COLOR: result = &s->main.char_constant_color; break;
    case STAG_INT_CONSTANT_COLOR: result = &s->main.int_constant_color; break;
    case STAG_FLOAT_CONSTANT_COLOR: result = &s->main.float_constant_color; break;
    case STAG_BOOL_CONSTANT_COLOR: result = &s->main.bool_constant_color; break;
        
    case STAG_PREPROC_COLOR: result = &s->main.preproc_color; break;
    case STAG_INCLUDE_COLOR: result = &s->main.include_color; break;
        
    case STAG_SPECIAL_CHARACTER_COLOR: result = &s->main.special_character_color; break;
        
    case STAG_HIGHLIGHT_JUNK_COLOR: result = &s->main.highlight_junk_color; break;
    case STAG_HIGHLIGHT_WHITE_COLOR: result = &s->main.highlight_white_color; break;
        
    case STAG_PASTE_COLOR: result = &s->main.paste_color; break;
    case STAG_UNDO_COLOR: result = &s->main.undo_color; break;
    }
    return result;
}

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

    String font_name = get_font_info(set, style->font_id)->name;
    out->font_name_size = font_name.size;
    memcpy(out->font_name, font_name.str, font_name.size);
    
    Style_Color_Specifier *spec = (Style_Color_Specifier*)(out + 1);
    i32 count = 0;
    
    for (u32 i = 0; i < STAG_COUNT; ++i){
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

// BOTTOM

