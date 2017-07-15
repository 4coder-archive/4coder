/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#define KEYCODES_FILE "4coder_API/keycodes.h"
#define STYLE_FILE "4coder_API/style.h"

#define API_H "4coder_API/app_functions.h"
#define OS_API_H "4ed_os_custom_api.h"

#include "../4ed_defines.h"
#include "4ed_meta_defines.h"
#include "../4coder_API/version.h"

#define FSTRING_IMPLEMENTATION
#include "../4coder_lib/4coder_string.h"

#include "../4cpp/4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4ed_file_moving.h"
#include "4ed_meta_parser.cpp"
#include "4ed_meta_keywords.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
char *keys_that_need_codes[] = {
    "back",
    "up",
    "down",
    "left",
    "right",
    "del",
    "insert",
    "home",
    "end",
    "page_up",
    "page_down",
    "esc",
    
    "mouse_left",
    "mouse_right",
    "mouse_left_release",
    "mouse_right_release",
    
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    
    "f9",
    "f10",
    "f11",
    "f12",
    "f13",
    "f14",
    "f15",
    "f16",
};

internal void
generate_keycode_enum(){
    Temp temp = fm_begin_temp();
    
    char *filename_keycodes = KEYCODES_FILE;
    
    u16 code = 0xD800;
    String out = str_alloc(10 << 20);
    
    i32 count = ArrayCount(keys_that_need_codes);
    
    append(&out, "enum{\n");
    for (i32 i = 0; i < count;){
        append(&out, "key_");
        append(&out, keys_that_need_codes[i++]);
        append(&out, " = ");
        append_int_to_str(&out, code++);
        append(&out, ",\n");
    }
    append(&out, "};\n");
    
    append(&out,
           "static char*\n"
           "global_key_name(uint32_t key_code, int32_t *size){\n"
           "char *result = 0;\n"
           "switch(key_code){\n");
    
    for (i32 i = 0; i < count; ++i){
        append(&out, "case key_");
        append(&out, keys_that_need_codes[i]);
        append(&out, ": result = \"");
        append(&out, keys_that_need_codes[i]);
        append(&out, "\"; *size = sizeof(\"");
        append(&out, keys_that_need_codes[i]);
        append(&out, "\")-1; break;\n");
    }
    
    append(&out,
           "}\n"
           "return(result);\n"
           "}\n");
    
    fm_write_file(filename_keycodes, out.str, out.size);
    out.size = 0;
    
    fm_end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
internal void
struct_begin(String *str, char *name){
    append(str, "struct ");
    append(str, name);
    append(str, "{\n"); //}
}

internal void
enum_begin(String *str, char *name){
    append(str, "enum ");
    append(str, name);
    append(str, "{\n"); //}
}

internal void
struct_end(String *str){ //{
    append(str, "};\n\n");
}

static char* bar_style_fields[] = {
    "bar",
    "bar_active",
    "base",
    "pop1",
    "pop2",
};

static char* main_style_fields[] = {
    "back",
    "margin",
    "margin_hover",
    "margin_active",
    "list_item",
    "list_item_hover",
    "list_item_active",
    "cursor",
    "at_cursor",
    "highlight",
    "at_highlight",
    "mark",
    "default",
    "comment",
    "keyword",
    "str_constant",
    "char_constant",
    "int_constant",
    "float_constant",
    "bool_constant",
    "preproc",
    "include",
    "special_character",
    "ghost_character",
    "highlight_junk",
    "highlight_white",
    "paste",
    "undo",
    "next_undo",
};

internal char*
make_style_tag(char *tag){
    i32 len = (i32)strlen(tag);
    char *str = fm_push_array(char, len + 1);
    to_camel(tag, str);
    str[len] = 0;
    return(str);
}

internal void
generate_style(){
    Temp temp = fm_begin_temp();
    
    char filename_4coder[] = STYLE_FILE;
    char filename_4ed[] = "4ed_style.h";
    
    String out = str_alloc(10 << 20);;
    
    enum_begin(&out, "Style_Tag");
    {
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(bar_style_fields[i]);
            append(&out, "Stag_");
            append(&out, tag);
            append(&out, ",\n");
        }
        
        count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(main_style_fields[i]);
            append(&out, "Stag_");
            append(&out, tag);
            append(&out, ",\n");
        }
        
        append(&out, "Stag_COUNT\n");
    }
    struct_end(&out);
    
    append(&out, "static char *style_tag_names[] = {\n");
    {
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(bar_style_fields[i]);
            append(&out, "\"");
            append(&out, tag);
            append(&out, "\",\n");
        }
        
        count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(main_style_fields[i]);
            append(&out, "\"");
            append(&out, tag);
            append(&out, "\",\n");
        }
    }
    append(&out, "};\n");
    
    fm_write_file(filename_4coder, out.str, out.size);
    out.size = 0;
    
    struct_begin(&out, "Interactive_Style");
    {
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            append(&out, "u32 ");
            append(&out, bar_style_fields[i]);
            append(&out, "_color;\n");
        }
    }
    struct_end(&out);
    
    struct_begin(&out, "Style_Main_Data");
    {
        i32 count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            append(&out, "u32 ");
            append(&out, main_style_fields[i]);
            append(&out, "_color;\n");
        }
        append(&out, "Interactive_Style file_info_style;\n");
    }
    struct_end(&out);
    
    {
        append(&out,
               "inline u32*\n"
               "style_index_by_tag(Style_Main_Data *s, u32 tag){\n"
               "u32 *result = 0;\n"
               "switch (tag){\n");
        
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(bar_style_fields[i]);
            append(&out, "case Stag_");
            append(&out, tag);
            append(&out, ": result = &s->file_info_style.");
            append(&out, bar_style_fields[i]);
            append(&out, "_color; break;\n");
        }
        
        count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(main_style_fields[i]);
            append(&out, "case Stag_");
            append(&out, tag);
            append(&out, ": result = &s->");
            append(&out, main_style_fields[i]);
            append(&out, "_color; break;\n");
        }
        
        append(&out,
               "}\n"
               "return(result);\n"
               "}\n\n");
    }
    
    fm_write_file(filename_4ed, out.str, out.size);
    out.size = 0;
    
    fm_end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//
// Meta Parse Rules
//

struct App_API_Name{
    String macro;
    String public_name;
};

struct App_API{
    App_API_Name *names;
};

internal App_API
allocate_app_api(i32 count){
    App_API app_api = {0};
    app_api.names = fm_push_array(App_API_Name, count);
    memset(app_api.names, 0, sizeof(App_API_Name)*count);
    return(app_api);
}

internal void
generate_custom_headers(){
    Temp temp = fm_begin_temp();
    
    // NOTE(allen): Parse the customization API files
    static char *functions_files[] = { "4ed_api_implementation.cpp", 0 };
    Meta_Unit unit_custom = compile_meta_unit(".", functions_files, ExpandArray(meta_keywords));
    if (unit_custom.parse == 0){
        Assert(!"Missing one or more input files!");
    }
    
    // NOTE(allen): Compute and store variations of the function names
    App_API func_4ed_names = allocate_app_api(unit_custom.set.count);
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        String name_string = unit_custom.set.items[i].name;
        String *macro = &func_4ed_names.names[i].macro;
        String *public_name = &func_4ed_names.names[i].public_name;
        
        *macro = str_alloc(name_string.size+4);
        to_upper(macro, name_string);
        append(macro, make_lit_string("_SIG"));
        
        *public_name = str_alloc(name_string.size);
        to_lower(public_name, name_string);
        
        fm_align();
    }
    
    // NOTE(allen): Output
    String out = str_alloc(10 << 20);
    
    // NOTE(allen): Custom API headers
    i32 main_api_count = unit_custom.parse[0].item_count;
    i32 os_api_count = unit_custom.parse[1].item_count;
    append(&out, "struct Application_Links;\n");
    
    for (i32 i = main_api_count; i < os_api_count; ++i){
        append(&out, "#define ");
        append(&out, func_4ed_names.names[i].macro);
        append(&out, "(n) ");
        append(&out, unit_custom.set.items[i].ret);
        append(&out, " n");
        append(&out, unit_custom.set.items[i].args);
        append_s_char(&out, '\n');
    }
    
    for (i32 i = main_api_count; i < os_api_count; ++i){
        append(&out, "typedef ");
        append(&out, func_4ed_names.names[i].macro);
        append_s_char(&out, '(');
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function);\n");
    }
    
    fm_write_file(OS_API_H, out.str, out.size);
    out.size = 0;
    
    append(&out, "struct Application_Links;\n");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, "#define ");
        append(&out, func_4ed_names.names[i].macro);
        append(&out, "(n) ");
        append(&out, unit_custom.set.items[i].ret);
        append(&out, " n");
        append(&out, unit_custom.set.items[i].args);
        append_s_char(&out, '\n');
    }
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, "typedef ");
        append(&out, func_4ed_names.names[i].macro);
        append_s_char(&out, '(');
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function);\n");
    }
    
    append(&out, "struct Application_Links{\n");
    
    
    append(&out, "#if defined(ALLOW_DEP_4CODER)\n");
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function *");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, ";\n");
    }
    
    append(&out, "#else\n");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function *");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, "_;\n");
    }
    append(&out, "#endif\n");
    
    append(&out,
           "void *memory;\n"
           "int32_t memory_size;\n"
           "void *cmd_context;\n"
           "void *system_links;\n"
           "void *current_coroutine;\n"
           "int32_t type_coroutine;\n"
           "};\n");
    
    append(&out, "#define FillAppLinksAPI(app_links) do{");
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, "\\\napp_links->");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, "_ = ");
        append(&out, unit_custom.set.items[i].name);
        append_s_char(&out, ';');
    }
    append(&out, "} while(false)\n");
    
    append(&out, "#if defined(ALLOW_DEP_4CODER)\n");
    for (i32 use_dep = 1; use_dep >= 0; --use_dep){
        for (i32 i = 0; i < unit_custom.set.count; ++i){
            Argument_Breakdown breakdown = unit_custom.set.items[i].breakdown;
            String ret = unit_custom.set.items[i].ret;
            String public_name = func_4ed_names.names[i].public_name;
            
            append(&out, "static inline ");
            append(&out, ret);
            append(&out, " ");
            append(&out, public_name);
            
            append(&out, "(");
            for (i32 j = 0; j < breakdown.count; ++j){
                append(&out, breakdown.args[j].param_string);
                if (j+1 != breakdown.count){
                    append(&out, ", ");
                }
            }
            append(&out, "){");
            
            if (match(ret, make_lit_string("void"))){
                append(&out, "(");
            }
            else{
                append(&out, "return(");
            }
            
            append(&out, "app->");
            append(&out, public_name);
            if (!use_dep){
                append(&out, "_");
            }
            
            append(&out, "(");
            for (i32 j = 0; j < breakdown.count; ++j){
                append(&out, breakdown.args[j].param_name);
                if (j+1 != breakdown.count){
                    append(&out, ", ");
                }
            }
            append(&out, ")");
            
            append(&out, ");}\n");
        }
        if (use_dep == 1){
            append(&out, "#else\n");
        }
    }
    append(&out, "#endif\n");
    
    fm_write_file(API_H, out.str, out.size);
    out.size = 0;
    
    fm_end_temp(temp);
}

int main(int argc, char **argv){
    META_BEGIN();
    
    fm_init_system();
    generate_keycode_enum();
    generate_style();
    generate_custom_headers();
    
    META_FINISH();
}

// BOTTOM

