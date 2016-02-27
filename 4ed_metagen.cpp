/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#define ArrayCount(a) (sizeof(a)/sizeof(*a))

struct Struct_Field{
    char *type;
    char *name;
};

void to_lower(char *src, char *dst){
    char *c, ch;
    for (c = src; *c != 0; ++c){
        ch = *c;
        if (ch >= 'A' && ch <= 'Z'){
            ch += ('a' - 'A');
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


char daction_enum_name[] = "Action_Type";
char *daction_enum[] = {
    "OPEN",
    "SAVE_AS",
    "SAVE",
    "NEW",
    "SWITCH",
    "TRY_KILL",
    "KILL",
    "CLOSE_MINOR",
    "CLOSE_MAJOR",
    "THEME_OPTIONS",
    "KEYBOARD_OPTIONS"
};

char daction_name[] = "Delayed_Action";
Struct_Field daction_fields[] = {
    {"Action_Type", "type"},
};
Struct_Field daction_fields_primary[] = {
    {"String", "string"},
    {"Panel*", "panel"},
    {"Editing_File*", "file"},
};
enum Daction_Field_Handle{
    dfph_null,
    dfph_string,
    dfph_panel,
    dfph_file,
};
Daction_Field_Handle dact_param_sets[] = {
    dfph_panel, dfph_null,
    dfph_string, dfph_panel, dfph_null,
    dfph_string, dfph_file, dfph_null
};

char delay_name[] = "Delay";
Struct_Field delay_fields[] = {
    {"Delayed_Action*", "acts"},
    {"i32", "count"},
    {"i32", "max"},
};

// TODO(allen): Make delay buffer expandable (general memory probably)
char delayed_action_function_top[] = 
"inline Delayed_Action*\n"
"delayed_action_(Delay *delay, Action_Type type";

char delayed_action_function_bottom[] = 
"){\n"
"    Delayed_Action *result;\n"
"    Assert(delay->count < delay->max);\n"
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

char delayed_action_specialized_bottom[] =
"    return(result);\n"
"}\n\n";

char delayed_action_macro[] =
"#define delayed_%s(delay, ...) delayed_action_(delay, DACT_%s, __VA_ARGS__)\n";

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
            Struct_Field field = daction_fields_primary[dact_param_sets[j] - 1];
            fprintf(file, delayed_action_special_line, field.name, field.name);
        }
        fprintf(file, "%s", delayed_action_specialized_bottom);
    }
    
    for (i = 0; i < ArrayCount(daction_enum); ++i){
        to_lower(daction_enum[i], scratch);
        fprintf(file, delayed_action_macro, scratch, daction_enum[i]);
    }
    
    return(filename);
}

int main(){
    char *filename;
    
    
    filename = generate_keycode_enum();
    printf("gen success: %s\n", filename);
    
    filename = generate_delayed_action();
    printf("gen success: %s\n", filename);
}

// BOTTOM

