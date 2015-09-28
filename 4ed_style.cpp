/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.08.2015
 *
 * Styles for 4coder
 *
 */

// TOP

struct P4C_Page_Header{
    i32 size;
    u32 id;
};

#define P4C_STYLE_ID COMPOSE_ID('s', 't', 'y', 'l')

struct Style_Page_Header{
    i32 version;
    i32 count;
};

struct Style_Main_Data_v1{
    u32 back_color;
	u32 margin_color;
	u32 margin_active_color;
	u32 cursor_color;
	u32 at_cursor_color;
	u32 highlight_color;
	u32 at_highlight_color;
	u32 mark_color;
	u32 default_color;
	u32 comment_color;
	u32 keyword_color;
	u32 constant_color;
	u32 special_character_color;
	u32 highlight_junk_color;
	u32 highlight_white_color;
    u32 paste_color;
    Interactive_Style file_info_style;
};

struct Style_File_Format_v1{
    i32 name_size;
    char name[24];
    i32 font_name_size;
    char font_name[24];
    Style_Main_Data_v1 main;
};

struct Style_Main_Data_v2{
    u32 back_color;
	u32 margin_color;
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
    Interactive_Style file_info_style;
};

struct Style_File_Format_v2{
    i32 name_size;
    char name[24];
    i32 font_name_size;
    char font_name[24];
    Style_Main_Data_v2 main;
};

struct Style_Main_Data_v3{
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
    Interactive_Style file_info_style;
};

struct Style_File_Format_v3{
    i32 name_size;
    char name[24];
    i32 font_name_size;
    char font_name[24];
    Style_Main_Data_v3 main;
};

struct Style{
    char name_[24];
    String name;
	Font *font;
    Style_Main_Data_v3 main;
    bool32 font_changed;
};

struct Style_Library{
    Style styles[64];
    i32 count, max;
};

struct Font_Set{
    Font *fonts;
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

internal Font*
font_set_extract(Font_Set *fonts, char *name, i32 size){
    String n = make_string(name, size);
    i32 count = fonts->count;
    Font *result = 0;
    Font *font = fonts->fonts;
    for (i32 i = 0; i < count; ++i, ++font){
        if (match(n, font->name)){
            result = font;
            break;
        }
    }
    return result;
}

internal void
style_form_convert(Style_File_Format_v2 *o, Style_File_Format_v1 *i){
    o->name_size = i->name_size;
    memcpy(o->name, i->name, i->name_size);
    o->font_name_size = i->font_name_size;
    memcpy(o->font_name, i->font_name, i->font_name_size);
    
    o->main.back_color = i->main.back_color;
    o->main.margin_color = i->main.margin_color;
    o->main.margin_active_color = i->main.margin_active_color;
    o->main.cursor_color = i->main.cursor_color;
    o->main.at_cursor_color = i->main.at_cursor_color;
    o->main.highlight_color = i->main.highlight_color;
    o->main.at_highlight_color = i->main.at_highlight_color;
    o->main.mark_color = i->main.mark_color;
    o->main.default_color = i->main.default_color;
    o->main.comment_color = i->main.comment_color;
    o->main.keyword_color = i->main.keyword_color;
    o->main.str_constant_color = i->main.constant_color;
    o->main.char_constant_color = i->main.constant_color;
    o->main.int_constant_color = i->main.constant_color;
    o->main.float_constant_color = i->main.constant_color;
    o->main.bool_constant_color = i->main.constant_color;
    o->main.include_color = i->main.constant_color;
    o->main.preproc_color = i->main.default_color;
    o->main.special_character_color = i->main.special_character_color;
    o->main.highlight_junk_color = i->main.highlight_junk_color;
    o->main.highlight_white_color = i->main.highlight_white_color;
    o->main.paste_color = i->main.paste_color;
    o->main.file_info_style = i->main.file_info_style;
}

internal void
style_form_convert(Style_File_Format_v3 *o, Style_File_Format_v2 *i){
    o->name_size = i->name_size;
    memcpy(o->name, i->name, i->name_size);
    o->font_name_size = i->font_name_size;
    memcpy(o->font_name, i->font_name, i->font_name_size);
    
    o->main.back_color = i->main.back_color;
    o->main.margin_color = i->main.margin_color;
    o->main.margin_active_color = i->main.margin_active_color;
    
    o->main.margin_hover_color = color_blend(i->main.margin_color, .5f, i->main.margin_active_color);
    
    o->main.cursor_color = i->main.cursor_color;
    o->main.at_cursor_color = i->main.at_cursor_color;
    o->main.highlight_color = i->main.highlight_color;
    o->main.at_highlight_color = i->main.at_highlight_color;
    o->main.mark_color = i->main.mark_color;
    o->main.default_color = i->main.default_color;
    o->main.comment_color = i->main.comment_color;
    o->main.keyword_color = i->main.keyword_color;
    o->main.str_constant_color = i->main.str_constant_color;
    o->main.char_constant_color = i->main.char_constant_color;
    o->main.int_constant_color = i->main.int_constant_color;
    o->main.float_constant_color = i->main.float_constant_color;
    o->main.bool_constant_color = i->main.bool_constant_color;
    o->main.include_color = i->main.include_color;
    o->main.preproc_color = i->main.preproc_color;
    o->main.special_character_color = i->main.special_character_color;
    o->main.highlight_junk_color = i->main.highlight_junk_color;
    o->main.highlight_white_color = i->main.highlight_white_color;
    o->main.paste_color = i->main.paste_color;
    o->main.file_info_style = i->main.file_info_style;
}

typedef Style_Main_Data_v3 Style_Main_Data;
typedef Style_File_Format_v3 Style_File_Format;

internal void
style_format_for_use(Font_Set *fonts, Style *out, Style_File_Format *style){
    out->name = make_string(out->name_, 0, ArrayCount(out->name_) - 1);
    out->name_[ArrayCount(out->name_) - 1] = 0;
    copy(&out->name, style->name);
    out->font = font_set_extract(fonts, style->font_name, style->font_name_size);
    out->main = style->main;
}

inline void
style_format_for_use(Font_Set *fonts, Style *out, Style_File_Format_v1 *style){
    Style_File_Format_v2 form2;
    Style_File_Format form;
    style_form_convert(&form2, style);
    style_form_convert(&form, &form2);
    style_format_for_use(fonts, out, &form);
}

inline void
style_format_for_use(Font_Set *fonts, Style *out, Style_File_Format_v2 *style){
    Style_File_Format form;
    style_form_convert(&form, style);
    style_format_for_use(fonts, out, &form);
}

internal bool32
style_library_import(u8 *filename, Font_Set *fonts, Style *out, i32 max,
                     i32 *count_opt, i32 *total_opt = 0){
    bool32 result = 1;
    File_Data file = system_load_file(filename);
    if (!file.data){
        result = 0;
    }
    else{
        void *cursor = file.data;
        i32 to_read = 0;
        
        {
            P4C_Page_Header *h = (P4C_Page_Header*)cursor;
            if (h->id != P4C_STYLE_ID){
                result = 0;
                goto early_exit;
            }
            cursor = h+1;
        }
        
        Style_Page_Header *h = (Style_Page_Header*)cursor;
        to_read = h->count;
        cursor = h+1;
        
        if (total_opt) *total_opt = to_read;
        if (to_read > max) to_read = max;
        if (count_opt) *count_opt = to_read;
        
        switch (h->version){
        case 1:
        {
            Style_File_Format_v1 *in = (Style_File_Format_v1*)cursor;
            for (i32 i = 0; i < to_read; ++i){
                style_format_for_use(fonts, out++, in++);
            }
        }break;
        case 2:
        {
            Style_File_Format_v2 *in = (Style_File_Format_v2*)cursor;
            for (i32 i = 0; i < to_read; ++i){
                style_format_for_use(fonts, out++, in++);
            }
        }break;
        case 3:
        {
            Style_File_Format_v3 *in = (Style_File_Format_v3*)cursor;
            for (i32 i = 0; i < to_read; ++i){
                style_format_for_use(fonts, out++, in++);
            }
        }break;
        default: result = 0; break;
        }
        
early_exit:
        system_free_file(file);
    }
    
    return result;
}

internal bool32
style_library_add(Style_Library *library, Style *style){
    bool32 result = 0;
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

internal Style_File_Format
style_format_for_file(Style *style){
    Style_File_Format result;
    Font *font = style->font;
    result.name_size = style->name.size;
    memcpy(result.name, style->name.str, ArrayCount(result.name));
    result.font_name_size = font->name.size;
    memcpy(result.font_name, font->name.str, ArrayCount(result.font_name));
    result.main = style->main;
    return result;
}

internal void
style_library_export(u8 *filename, Style **styles, i32 count){
    i32 size = count*sizeof(Style_File_Format) +
        sizeof(P4C_Page_Header) + sizeof(Style_Page_Header);
    void *data = system_get_memory(size);
    void *cursor = data;
    
    {
        P4C_Page_Header *h = (P4C_Page_Header*)cursor;
        h->size = size - sizeof(P4C_Page_Header);
        h->id = P4C_STYLE_ID;
        cursor = h+1;
    }
    
    {
        Style_Page_Header *h = (Style_Page_Header*)cursor;
        h->version = 1;
        h->count = count;
        cursor = h+1;
    }
    
    Style_File_Format *out = (Style_File_Format*)cursor;
    Style **in = styles;
    for (i32 i = 0; i < count; ++i){
        *out++ = style_format_for_file(*in++);
    }
    system_save_file(filename, data, size);
    system_free_memory(data);
}

// BOTTOM

