/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

struct Struct_Field{
    char *type;
    char *name;
};

void to_lower(char *src, char *dst){
    char *c, ch;
    for (c = src; *c != 0; ++c){
        ch = char_to_lower(*c);
        *dst++ = ch;
    }
    *dst = 0;
}

void to_upper(char *src, char *dst){
    char *c, ch;
    for (c = src; *c != 0; ++c){
        ch = char_to_upper(*c);
        *dst++ = ch;
    }
    *dst = 0;
}

void to_camel(char *src, char *dst){
    char *c, ch;
    int is_first = 1;
    for (c = src; *c != 0; ++c){
        ch = *c;
        if (char_is_alpha_numeric_true(ch)){
            if (is_first){
                is_first = 0;
                ch = char_to_upper(ch);
            }
            else{
                ch = char_to_lower(ch);
            }
        }
        else{
            is_first = 1;
        }
        *dst++ = ch;
    }
    *dst = 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void struct_begin(FILE *file, char *name){
    fprintf(file, "struct %s{\n", name);
}

void struct_fields(FILE *file, Struct_Field *fields, int count){
    int i;
    for (i = 0; i < count; ++i){
        fprintf(file, "    %s %s;\n", fields[i].type, fields[i].name);
    }
}

void struct_end(FILE *file){
    fprintf(file, "};\n\n");
}


void enum_begin(FILE *file, char *name){
    fprintf(file, "enum %s{\n", name);
}


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

char* generate_keycode_enum(){
    FILE *file;
    char *filename = "4coder_keycodes.h";
    int i, count;
    unsigned char code = 1;
    
    file = fopen(filename, "wb");
    fprintf(file, "enum Key_Code{\n");
    count = ArrayCount(keys_that_need_codes);
    for (i = 0; i < count;){
        if (strcmp(keys_that_need_codes[i], "f1") == 0 && code < 0x7F){
            code = 0x7F;
        }
        switch (code){
        case '\n': code++; break;
        case '\t': code++; break;
        case 0x20: code = 0x7F; break;
        default:
        fprintf(file, "    key_%s = %d,\n", keys_that_need_codes[i++], code++);
        break;
        }
    }
    fprintf(file, "};\n");
    fclose(file);
    return(filename);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
char daction_enum_name[] = "Action_Type";
char *daction_enum[] = {
    "OPEN",
    "OPEN_BACKGROUND",
    "SET_LINE",
    "SAVE_AS",
    "SAVE",
    "NEW",
    "SWITCH",
    "TRY_KILL",
    "KILL",
    "TOUCH_FILE",
};

char str_alloc_copy[] =
"internal String\n"
"str_alloc_copy(General_Memory *general, String str){\n"
"    String result;\n"
"    result.memory_size = str.memory_size + 1;\n"
"    result.size = str.size;\n"
"    result.str = (char*)general_memory_allocate(general, result.memory_size, 0);\n"
"    memcpy(result.str, str.str, str.size);\n"
"    result.str[result.size] = 0;\n"
"    return(result);"
"}\n\n";

char daction_name[] = "Delayed_Action";
Struct_Field daction_fields[] = {
    {"Action_Type", "type"},
};
Struct_Field daction_fields_primary[] = {
    {"String", "string"},
    {"Panel*", "panel"},
    {"Editing_File*", "file"},
    {"i32", "integer"},
};
enum Daction_Field_Handle{
    dfph_null,
    dfph_string,
    dfph_panel,
    dfph_file,
    dfph_integer,
};
Daction_Field_Handle dact_param_sets[] = {
    dfph_string, dfph_null,
    dfph_panel, dfph_null,
    dfph_file, dfph_null,
    dfph_file, dfph_panel, dfph_null,
    dfph_string, dfph_panel, dfph_null,
    dfph_string, dfph_file, dfph_null,
    dfph_panel, dfph_integer, dfph_null,
};

char delay_name[] = "Delay";
Struct_Field delay_fields[] = {
    {"General_Memory*", "general"},
    {"Delayed_Action*", "acts"},
    {"i32", "count"},
    {"i32", "max"},
};

char delayed_action_function_top[] = 
"inline Delayed_Action*\n"
"delayed_action_(Delay *delay, Action_Type type";

char delayed_action_function_bottom[] = 
"){\n"
"    Delayed_Action *result;\n"
"    if (delay->count == delay->max){\n"
"        delay->max *= 2;\n"
"        delay->acts = (Delayed_Action*)general_memory_reallocate("
"delay->general, delay->acts, delay->count*sizeof(Delayed_Action), delay->max*sizeof(Delayed_Action), 0);\n"
"    }\n"
"    result = delay->acts + delay->count++;\n"
"    *result = {};\n"
"    result->type = type;\n"
"    return(result);\n"
"}\n\n";

char delayed_action_special_param[] = ", %s %s";

char delayed_action_specialized_middle[] =
"){\n"
"    Delayed_Action *result;\n"
"    result = delayed_action_(delay, type);\n";

char delayed_action_special_line[] =
"    result->%s = %s;\n";

char delayed_action_special_string_line[] =
"    result->%s = str_alloc_copy(delay->general, %s);\n";

char delayed_action_specialized_bottom[] =
"    return(result);\n"
"}\n\n";

char delayed_action_macro[] =
"#define delayed_%s(delay, ...) delayed_action_(delay, DACT_%s, __VA_ARGS__)\n";

char delayed_action_repush_function[] =
"inline Delayed_Action*\n"
"delayed_action_repush(Delay *delay, Delayed_Action *act){\n"
"    Delayed_Action *new_act = delayed_action_(delay, (Action_Type)0);\n"
"    *new_act = *act;\n"
"    if (act->string.str){\n"
"        new_act->string = str_alloc_copy(delay->general, act->string);\n"
"    }\n"
"    return(new_act);\n"
"}\n\n";

char* generate_delayed_action(){
    FILE *file;
    char *filename = "4ed_delay.cpp";
    char scratch[256];
    int i,j;
    
    file = fopen(filename, "wb");
    
    fprintf(file, "enum %s{\n", daction_enum_name);
    for (i = 0; i < ArrayCount(daction_enum); ++i){
        fprintf(file, "    DACT_%s,\n", daction_enum[i]);
    }
    fprintf(file, "};\n\n");
    
    struct_begin(file, daction_name);
    struct_fields(file, daction_fields, ArrayCount(daction_fields));
    struct_fields(file, daction_fields_primary, ArrayCount(daction_fields_primary));
    struct_end(file);
    
    struct_begin(file, delay_name);
    struct_fields(file, delay_fields, ArrayCount(delay_fields));
    struct_end(file);
    
    fprintf(file, "%s", str_alloc_copy);
    fprintf(file, "%s%s", delayed_action_function_top, delayed_action_function_bottom);
    
    for (i = 0; i < ArrayCount(dact_param_sets); ++i){
        j =  i;
        fprintf(file, "%s", delayed_action_function_top);
        for (; dact_param_sets[i] != dfph_null; ++i){
            Struct_Field field = daction_fields_primary[dact_param_sets[i] - 1];
            fprintf(file, delayed_action_special_param, field.type, field.name);
        }
        fprintf(file, "%s", delayed_action_specialized_middle);
        for (; dact_param_sets[j] != dfph_null; ++j){
            int handle = (int)(dact_param_sets[j]);
            Struct_Field field = daction_fields_primary[handle - 1];
            if (handle == dfph_string){
                fprintf(file, delayed_action_special_string_line, field.name, field.name);
            }
            else{
                fprintf(file, delayed_action_special_line, field.name, field.name);
            }
        }
        fprintf(file, "%s", delayed_action_specialized_bottom);
    }
    
    fprintf(file, "%s", delayed_action_repush_function);
    
    for (i = 0; i < ArrayCount(daction_enum); ++i){
        to_lower(daction_enum[i], scratch);
        fprintf(file, delayed_action_macro, scratch, daction_enum[i]);
    }
    
    return(filename);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
char* bar_style_fields[] = {
    "bar",
    "bar_active",
    "base",
    "pop1",
    "pop2",
};

char* main_style_fields[] = {
    "back",
	"margin",
	"margin_hover",
	"margin_active",
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
	"highlight_junk",
	"highlight_white",
    "paste",
    "undo",
    "next_undo",
};

static char*
make_style_tag(char *tag){
    char *str;
    int len;
    
    len = (int)strlen(tag);
    str = (char*)malloc(len + 1);
    to_camel(tag, str);
    str[len] = 0;

    return(str);
}

char style_index_function_start[] =
"inline u32*\n"
"style_index_by_tag(Style_Main_Data *s, u32 tag){\n"
" u32 *result = 0;\n"
" switch (tag){\n";

char style_index_function_end[] =
" }\n"
" return(result);\n"
"}\n\n";

char style_case[] = " case Stag_%s: result = &s->%s_color; break;\n";
char style_info_case[] = " case Stag_%s: result = &s->file_info_style.%s_color; break;\n";

char* generate_style(){
    char *filename = "4coder_style.h & 4ed_style.h";
    char filename_4coder[] = "4coder_style.h";
    char filename_4ed[] = "4ed_style.h";
    FILE *file;
    char *tag;
    int count, i;

    file = fopen(filename_4coder, "wb");
    enum_begin(file, "Style_Tag");
    {
        count = ArrayCount(bar_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(bar_style_fields[i]);
            fprintf(file, "Stag_%s,\n", tag);
            free(tag);
        }
        
        count = ArrayCount(main_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(main_style_fields[i]);
            fprintf(file, "Stag_%s,\n", tag);
            free(tag);
        }
    }
    struct_end(file);
    fclose(file);
    
    file = fopen(filename_4ed, "wb");
    struct_begin(file, "Interactive_Style");
    {
        count = ArrayCount(bar_style_fields);
        for (i = 0; i < count; ++i){
            fprintf(file, "u32 %s_color;\n", bar_style_fields[i]);
        }
    }
    struct_end(file);
    
    struct_begin(file, "Style_Main_Data");
    {
        count = ArrayCount(main_style_fields);
        for (i = 0; i < count; ++i){
            fprintf(file, "u32 %s_color;\n", main_style_fields[i]);
        }
        fprintf(file, "Interactive_Style file_info_style;\n");
    }
    struct_end(file);
    
    {
        fprintf(file, "%s", style_index_function_start);
        count = ArrayCount(bar_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(bar_style_fields[i]);
            fprintf(file, style_info_case, tag, bar_style_fields[i]);
            free(tag);
        }
        
        count = ArrayCount(main_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(main_style_fields[i]);
            fprintf(file, style_case, tag, main_style_fields[i]);
            free(tag);
        }
        fprintf(file, "%s", style_index_function_end);
    }
    
    fclose(file);
    
    return(filename);
}

int main(){
    char *filename;
    
    filename = generate_keycode_enum();
    printf("gen success: %s\n", filename);
    
    filename = generate_delayed_action();
    printf("gen success: %s\n", filename);
    
    filename = generate_style();
    printf("gen success: %s\n", filename);
}

// BOTTOM

