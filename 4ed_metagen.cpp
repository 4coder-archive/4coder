/*
* Mr. 4th Dimention - Allen Webster
*
* 25.02.2016
*
* File editing view for 4coder
*
*/

// TOP

#include "4ed_meta.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4cpp_types.h"
#include "4cpp_lexer_types.h"

#define FCPP_LEXER_IMPLEMENTATION
#include "4cpp_lexer.h"

#include "4coder_version.h"

#include "4coder_mem.h"

struct Global_Settings{
    int generate_docs;
};

static Global_Settings global_settings;

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

void to_lower(String *str){
    char *c;
    int i = 0;
    int size = str->size;
    for (c = str->str; i < size; ++c, ++i){
        *c = char_to_lower(*c);
    }
}

void to_upper(char *src, char *dst){
    char *c, ch;
    for (c = src; *c != 0; ++c){
        ch = char_to_upper(*c);
        *dst++ = ch;
    }
    *dst = 0;
}

void to_upper(String *str){
    char *c;
    int i = 0;
    int size = str->size;
    for (c = str->str; i < size; ++c, ++i){
        *c = char_to_upper(*c);
    }
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
#include <assert.h>

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
    
    "mouse_left",
    "mouse_right",
    
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
    fprintf(file, "enum Key_Code_Names{\n");
    count = ArrayCount(keys_that_need_codes);
    for (i = 0; i < count; i){
        if (strcmp(keys_that_need_codes[i], "f1") == 0 && code < 0x7F){
            code = 0x7F;
        }
        switch (code){
            case '\n': code++; break;
            case '\t': code++; break;
            case 0x20: code = 0x7F; break;
            default:
            fprintf(file, "key_%s = %d,\n", keys_that_need_codes[i++], code++);
            break;
        }
    }
    fprintf(file, "};\n");
    
    fprintf(file,
            "static char*\n"
            "global_key_name(int key_code, int *size){\n"
            "char *result = 0;\n"
            "switch(key_code){\n"
            );
    for (i = 0; i < count; ++i){
        fprintf(file,
                "case key_%s: result = \"%s\"; *size = sizeof(\"%s\")-1; break;\n",
                keys_that_need_codes[i],
                keys_that_need_codes[i],
                keys_that_need_codes[i]
                );
    }
    fprintf(file,
            "}\n"
            "return(result);\n"
            "}\n"
            );
    
    fclose(file);
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

//////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Argument_Breakdown{
    int count;
    String *param_string;
    String *param_name;
} Argument_Breakdown;

typedef struct Documentation{
    int param_count;
    String *param_name;
    String *param_docs;
    String return_doc;
    String main_doc;
    int see_also_count;
    String *see_also;
} Documentation;

struct Function_Set{
    String *name;
    String *ret;
    String *args;
    
    String *macros;
    String *public_name;
    String *doc_string;
    
    int    *valid;
    
    Argument_Breakdown *breakdown;
    Documentation *doc;
};

struct Typedef_Set{
    String *type;
    String *name;
    String *doc_string;
}; 

struct Struct_Member{
    String name;
    String type;
    String type_postfix;
    String doc_string;
    Struct_Member *first_child;
    Struct_Member *next_sibling;
};

struct Struct_Set{
    Struct_Member *structs;
};

struct Enum_Member{
    String name;
    String value;
    String doc_string;
    Enum_Member *next;
};

struct Enum_Set{
    String *name;
    String *type;
    Enum_Member **first_member;
    String *doc_string;
};

void
zero_index(Function_Set fnc_set, int sig_count){
    fnc_set.name [sig_count] = string_zero();
    fnc_set.ret  [sig_count] = string_zero();
    fnc_set.args [sig_count] = string_zero();
    fnc_set.valid[sig_count] = 0;
}

String
file_dump(char *filename){
    String result = {0};
    FILE *file = fopen(filename, "rb");
    
    if (file){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result.memory_size = result.size + 1;
        result.str = (char*)malloc(result.memory_size);
        
        fread(result.str, 1, result.size, file);
        result.str[result.size] = 0;
        
        fclose(file);
    }
    
    return(result);
}

String
get_first_line(String source){
    String line = {0};
    int pos = find(source, 0, '\n');
    
    line = substr(source, 0, pos);
    
    return(line);
}

String
get_next_line(String source, String line){
    String next = {0};
    int pos = (int)(line.str - source.str) + line.size;
    int start = 0;
    
    if (pos < source.size){
        assert(source.str[pos] == '\n');
        start = pos + 1;
        
        if (start < source.size){
            pos = find(source, start, '\n');
            next = substr(source, start, pos - start);
        }
    }
    
    return(next);
}

int
is_comment(String str){
    int result = 0;
    if (str.size >= 2){
        if (str.str[0] == '/' &&
            str.str[1] == '/'){
            result = 1;
        }
    }
    return(result);
}

struct Parse{
    Cpp_Token_Stack tokens;
};

int
check_and_fix_docs(String *lexeme){
    int result = false;
    
    if (lexeme->size > 4){
        if (lexeme->str[0] == '/'){
            if (lexeme->str[1] == '*'){
                if (lexeme->str[lexeme->size - 2] == '*'){
                    if (lexeme->str[lexeme->size - 1] == '/'){
                        result = true;
                        lexeme->str += 2;
                        lexeme->size -= 4;
                    }
                }
            }
        }
    }
    
    return(result);
}

enum Doc_Note_Type{
    DOC_PARAM,
    DOC_RETURN,
    DOC,
    DOC_SEE
};

static String
doc_note_string[] = {
    make_lit_string("DOC_PARAM"),
    make_lit_string("DOC_RETURN"),
    make_lit_string("DOC"),
    make_lit_string("DOC_SEE"),
};

String
doc_parse_note(String source, int *pos){
    String result = {0};
    
    int p = *pos;
    int start = p;
    for (; p < source.size; ++p){
        if (source.str[p] == '('){
            break;
        }
    }
    if (p != source.size){
        result = make_string(source.str + start, p - start);
        result = skip_chop_whitespace(result);
    }
    *pos = p;
    
    return(result);
}

String
doc_parse_note_string(String source, int *pos){
    String result = {0};
    
    assert(source.str[*pos] == '(');
    
    int p = *pos + 1;
    int start = p;
    
    int nest_level = 0;
    
    for (; p < source.size; ++p){
        if (source.str[p] == ')'){
            if (nest_level == 0){
                break;
            }
            else{
                --nest_level;
            }
        }
        else if (source.str[p] == '('){
            ++nest_level;
        }
    }
    if (p != source.size){
        result = make_string(source.str + start, p - start);
        result = skip_chop_whitespace(result);
        ++p;
    }
    *pos = p;
    
    return(result);
}

String
doc_parse_parameter(String source, int *pos){
    String result = {0};
    
    int p = *pos;
    int start = p;
    
    for (; p < source.size; ++p){
        if (source.str[p] == ','){
            break;
        }
    }
    if (p != source.size){
        result = make_string(source.str + start, p - start);
        result = skip_chop_whitespace(result);
        ++p;
    }
    *pos = p;
    
    return(result);
}

String
doc_parse_last_parameter(String source, int *pos){
    String result = {0};
    
    int p = *pos;
    int start = p;
    
    for (; p < source.size; ++p){
        if (source.str[p] == ')'){
            break;
        }
    }
    if (p == source.size){
        result = make_string(source.str + start, p - start);
        result = skip_chop_whitespace(result);
    }
    *pos = p;
    
    return(result);    
}

void
perform_doc_parse(Partition *part, String doc_string, Documentation *doc){
    int keep_parsing = true;
    int pos = 0;
    
    int param_count = 0;
    int see_count = 0;
    
    do{
        String doc_note = doc_parse_note(doc_string, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            int doc_note_type;
            if (string_set_match(doc_note_string, ArrayCount(doc_note_string), doc_note, &doc_note_type)){
                
                doc_parse_note_string(doc_string, &pos);
                
                switch (doc_note_type){
                    case DOC_PARAM:
                    ++param_count;
                    break;
                    
                    case DOC_SEE:
                    ++see_count;
                    break;
                }
            }
        }
    }while(keep_parsing);
    
    if (param_count + see_count > 0){
        int memory_size = sizeof(String)*(2*param_count + see_count);
        doc->param_name = push_array(part, String, memory_size);
        doc->param_docs = doc->param_name + param_count;
        doc->see_also   = doc->param_docs + param_count;
        
        doc->param_count = param_count;
        doc->see_also_count = see_count;
    }
    
    int param_index = 0;
    int see_index = 0;
    
    keep_parsing = true;
    pos = 0;
    do{
        String doc_note = doc_parse_note(doc_string, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            int doc_note_type;
            if (string_set_match(doc_note_string, ArrayCount(doc_note_string), doc_note, &doc_note_type)){
                
                String doc_note_string = doc_parse_note_string(doc_string, &pos);
                
                switch (doc_note_type){
                    case DOC_PARAM:
                    {
                        assert(param_index < param_count);
                        int param_pos = 0;
                        String param_name = doc_parse_parameter(doc_note_string, &param_pos);
                        String param_docs = doc_parse_last_parameter(doc_note_string, &param_pos);
                        doc->param_name[param_index] = param_name;
                        doc->param_docs[param_index] = param_docs;
                        ++param_index;
                    }break;
                    
                    case DOC_RETURN:
                    {
                        doc->return_doc = doc_note_string;
                    }break;
                    
                    case DOC:
                    {
                        doc->main_doc = doc_note_string;
                    }break;
                    
                    case DOC_SEE:
                    {
                        assert(see_index < see_count);
                        doc->see_also[see_index++] = doc_note_string;
                    }break;
                }
            }
            else{
                fprintf(stderr, "warning: invalid doc note %.*s\n", doc_note.size, doc_note.str);
            }
        }
    }while(keep_parsing);
}

static int
get_type_doc_string(Cpp_File file, Cpp_Token *tokens, int i,
                    String *doc_string){
    int result = false;
    
    if (i > 0){
        Cpp_Token *prev_token = tokens + i - 1;
        if (prev_token->type == CPP_TOKEN_COMMENT){
            *doc_string = make_string(file.data + prev_token->start, prev_token->size);
            if (check_and_fix_docs(doc_string)){
                result = true;
            }
        }
    }
    
    return(result);
}

static int
parse_struct(Partition *part, int is_struct,
             Cpp_File file, Cpp_Token *tokens, int count,
             Cpp_Token **token_ptr,
             Struct_Member *top_member);

static int
parse_struct_member(Partition *part,
                    Cpp_File file, Cpp_Token *tokens, int count,
                    Cpp_Token **token_ptr,
                    Struct_Member *member){
    
    int result = false;
    
    Cpp_Token *token = *token_ptr;
    int i = (int)(token - tokens);
    
    String doc_string = {0};
    get_type_doc_string(file, tokens, i, &doc_string);
    
    int start_i = i;
    Cpp_Token *start_token = token;
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_SEMICOLON){
            break;
        }
    }
    
    if (i < count){
        Cpp_Token *token_j = token;
        
        int nest_level = 0;
        for (int j = i; j > start_i; --j, --token_j){
            if (token_j->type == CPP_TOKEN_BRACKET_CLOSE){
                ++nest_level;
            }
            else if (token_j->type == CPP_TOKEN_BRACKET_OPEN){
                --nest_level;
                if (nest_level < 0){
                    j = start_i;
                    break;
                }
            }
            
            if (nest_level == 0){
                if (token_j->type == CPP_TOKEN_IDENTIFIER){
                    break;
                }
            }
        }
        
        String name = make_string(file.data + token_j->start, token_j->size);
        name = skip_chop_whitespace(name);
        
        int type_start = start_token->start;
        int type_end = token_j->start;
        String type = make_string(file.data + type_start, type_end - type_start);
        type = skip_chop_whitespace(type);
        
        type_start = token_j->start + token_j->size;
        type_end = token->start;
        
        String type_postfix = make_string(file.data + type_start, type_end - type_start);
        type_postfix = skip_chop_whitespace(type_postfix);
        
        ++token;
        result = true;
        
        member->name = name;
        member->type = type;
        member->type_postfix = type_postfix;
        member->doc_string = doc_string;
        member->first_child = 0;
        member->next_sibling = 0;
    }
    
    *token_ptr = token;
    
    return(result);
}

static Struct_Member*
parse_struct_next_member(Partition *part,
                         Cpp_File file, Cpp_Token *tokens, int count,
                         Cpp_Token **token_ptr){
    Struct_Member *result = 0;
    
    Cpp_Token *token = *token_ptr;
    int i = (int)(token - tokens);
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_IDENTIFIER ||
            (token->flags & CPP_TFLAG_IS_KEYWORD)){
            String lexeme = make_string(file.data + token->start, token->size);
            
            if (match(lexeme, make_lit_string("struct"))){
                Struct_Member *member = push_struct(part, Struct_Member);
                if (parse_struct(part, true, file, tokens, count, &token, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else if (match(lexeme, make_lit_string("union"))){
                Struct_Member *member = push_struct(part, Struct_Member);
                if (parse_struct(part, false, file, tokens, count, &token, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else{
                Struct_Member *member = push_struct(part, Struct_Member);
                if (parse_struct_member(part, file, tokens, count, &token, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
        }
        else if (token->type == CPP_TOKEN_BRACE_CLOSE){
            break;
        }
    }
    
    *token_ptr = token;
    
    return(result);
}

static int
parse_struct(Partition *part, int is_struct,
             Cpp_File file, Cpp_Token *tokens, int count,
             Cpp_Token **token_ptr,
             Struct_Member *top_member){
    
    int result = false;
    
    Cpp_Token *token = *token_ptr;
    int i = (int)(token - tokens);
    
    String doc_string = {0};
    get_type_doc_string(file, tokens, i, &doc_string);
    
    int start_i = i;
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_BRACE_OPEN){
            break;
        }
    }
    
    if (i < count){
        Cpp_Token *token_j = token;
        int j = i;
        
        for (; j > start_i; --j, --token_j){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        String name = {0};
        
        if (j != start_i){
            name = make_string(file.data + token_j->start, token_j->size);
            name = skip_chop_whitespace(name);
        }
        
        String type = {0};
        if (is_struct){
            type = make_lit_string("struct");
        }
        else{
            type = make_lit_string("union");
        }
        
        ++token;
        Struct_Member *new_member = 
            parse_struct_next_member(part, file, tokens, count, &token);
        
        if (new_member){
            top_member->first_child = new_member;
            
            Struct_Member *head_member = new_member;
            for(;;){
                new_member = 
                    parse_struct_next_member(part, file, tokens, count, &token);
                if (new_member){
                    head_member->next_sibling = new_member;
                    head_member = new_member;
                }
                else{
                    break;
                }
            }
        }
        
        i = (int)(token - tokens);
        for (; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_SEMICOLON){
                break;
            }
        }
        ++token;
        
        top_member->name = name;
        top_member->type = type;
        top_member->doc_string = doc_string;
        top_member->next_sibling = 0;
        
        result = true;
    }
    
    *token_ptr = token;
    
    return(result);
}

static void
print_struct_html(FILE *file, Struct_Member *member){
    String name = member->name;
    String type = member->type;
    String type_postfix = member->type_postfix;
    
    if (match(type, make_lit_string("struct")) ||
        match(type, make_lit_string("union"))){
        fprintf(file,
                "%.*s %.*s {<br>\n"
                "<div style='margin-left: 8mm;'>\n",
                type.size, type.str,
                name.size, name.str);
        
        for (Struct_Member *member_iter = member->first_child;
             member_iter != 0;
             member_iter = member_iter->next_sibling){
            print_struct_html(file, member_iter);
        }
        
        fprintf(file,
                "</div>\n"
                "};<br>\n");
    }
    else{
        fprintf(file,
                "%.*s %.*s%.*s;<br>\n",
                type.size, type.str,
                name.size, name.str,
                type_postfix.size, type_postfix.str
                );
    }
}

#define BACK_COLOR   "#FAFAFA"
#define TEXT_COLOR   "#0D0D0D"
#define CODE_BACK    "#DFDFDF"

#define POP_COLOR_1  "#309030"
#define POP_BACK_1   "#E0FFD0"
#define VISITED_LINK "#A0C050"

#define POP_COLOR_2  "#005000"

#define CODE_STYLE "font-family: \"Courier New\", Courier, monospace; text-align: left;"

#define DESCRIPT_SECTION_STYLE \
"margin-top: 3mm; margin-bottom: 3mm; font-size: .95em; " \
"background: "CODE_BACK"; padding: 0.25em;"

#define DOC_HEAD_OPEN  "<div style='margin-top: 3mm; margin-bottom: 3mm; color: "POP_COLOR_1";'><b><i>"
#define DOC_HEAD_CLOSE "</i></b></div>"

#define DOC_ITEM_HEAD_STYLE "font-weight: 600;"

#define DOC_ITEM_HEAD_INL_OPEN  "<span style='"DOC_ITEM_HEAD_STYLE"'>"
#define DOC_ITEM_HEAD_INL_CLOSE "</span>"

#define DOC_ITEM_HEAD_OPEN  "<div style='"DOC_ITEM_HEAD_STYLE"'>"
#define DOC_ITEM_HEAD_CLOSE "</div>"

#define DOC_ITEM_OPEN  "<div style='margin-left: 5mm; margin-right: 5mm;'>"
#define DOC_ITEM_CLOSE "</div>"

static void
print_struct_docs(FILE *file, Partition *part, Struct_Member *member){
    for (Struct_Member *member_iter = member->first_child;
         member_iter != 0;
         member_iter = member_iter->next_sibling){
        String type = member_iter->type;
        if (match(type, make_lit_string("struct")) ||
            match(type, make_lit_string("union"))){
            print_struct_docs(file, part, member_iter);
        }
        else{
            Documentation doc = {0};
            perform_doc_parse(part, member_iter->doc_string, &doc);
            
            fprintf(file,
                    "<div>\n"
                    "<div style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN
                    "%.*s"DOC_ITEM_HEAD_INL_CLOSE"</div>\n"
                    "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE"</div>\n"
                    "</div>\n",
                    member_iter->name.size, member_iter->name.str,
                    doc.main_doc.size, doc.main_doc.str
                    );
        }
    }
}

static void
print_see_also(FILE *file, Documentation *doc){
    int doc_see_count = doc->see_also_count;
    if (doc_see_count > 0){
        fprintf(file, DOC_HEAD_OPEN"See Also"DOC_HEAD_CLOSE);
        
        for (int j = 0; j < doc_see_count; ++j){
            String see_also = doc->see_also[j];
            fprintf(file,
                    DOC_ITEM_OPEN"<a href='#%.*s_doc'>%.*s</a>"DOC_ITEM_CLOSE,
                    see_also.size, see_also.str,
                    see_also.size, see_also.str
                    );
        }
    }
}

static int
parse_enum(Partition *part, Cpp_File file,
           Cpp_Token *tokens, int count,
           Cpp_Token **token_ptr, int start_i,
           Enum_Set flag_set, int flag_index){
    
    int result = false;
    
    Cpp_Token *token = *token_ptr;
    int i = (int)(token - tokens);
    
    if (i < count){
        Cpp_Token *token_j = token;
        
        for (int j = i; j > start_i; --j, --token_j){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        String name = make_string(file.data + token_j->start, token_j->size);
        name = skip_chop_whitespace(name);
        
        for (; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_BRACE_OPEN){
                break;
            }
        }
        
        if (i < count){
            Enum_Member *first_member = 0;
            Enum_Member *head_member = 0;
            
            for (; i < count; ++i, ++token){
                if (token->type == CPP_TOKEN_BRACE_CLOSE){
                    break;
                }
                else if (token->type == CPP_TOKEN_IDENTIFIER){
                    String doc_string = {0};
                    get_type_doc_string(file, tokens, i, &doc_string);
                    
                    String name = make_string(file.data + token->start, token->size);
                    name = skip_chop_whitespace(name);
                    
                    String value = {0};
                    
                    ++i;
                    ++token;
                    
                    if (token->type == CPP_TOKEN_EQ){
                        Cpp_Token *start_token = token;
                        
                        for (; i < count; ++i, ++token){
                            if (token->type == CPP_TOKEN_COMMA ||
                                token->type == CPP_TOKEN_BRACE_CLOSE){
                                break;
                            }
                        }
                        
                        int val_start = start_token->start + start_token->size;
                        int val_end = token->start;
                        
                        value = make_string(file.data + val_start, val_end - val_start);
                        value = skip_chop_whitespace(value);
                        
                        --i;
                        --token;
                    }
                    else{
                        --i;
                        --token;
                    }
                    
                    Enum_Member *new_member = push_struct(part, Enum_Member);
                    if (first_member == 0){
                        first_member = new_member;
                    }
                    
                    if (head_member){
                        head_member->next = new_member;
                    }
                    head_member = new_member;
                    
                    new_member->name = name;
                    new_member->value = value;
                    new_member->doc_string = doc_string;
                    new_member->next = 0;
                }
            }
            
            if (i < count){
                for (; i < count; ++i, ++token){
                    if (token->type == CPP_TOKEN_BRACE_CLOSE){
                        break;
                    }
                }
                ++i;
                ++token;
                
                result = true;
                flag_set.name[flag_index] = name;
                flag_set.first_member[flag_index] = first_member;
            }
        }
    }
    
    *token_ptr = token;
    
    return(result);
}

char*
generate_custom_headers(){
#define API_H "4coder_custom_api.h"
#define API_DOC "4coder_API.html"
    
    int size = Mbytes(512);
    void *mem = malloc(size);
    memset(mem, 0, size);
    
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    char *filename = API_H " & " API_DOC;
    
    
    String code_data[2];
    code_data[0] = file_dump("4ed_api_implementation.cpp");
    code_data[1] = file_dump("win32_api_impl.cpp");
    Parse parses[2];
    
    int max_name_size = 0;
    int line_count = 0;
    
    for (int J = 0; J < 2; ++J){
        String *code = &code_data[J];
        Parse *parse = &parses[J];
        
        // TODO(allen): KILL THIS FUCKIN' Cpp_File FUCKIN' NONSENSE HORSE SHIT!!!!!
        Cpp_File file;
        file.data = code->str;
        file.size = code->size;
        
        parse->tokens = cpp_make_token_stack(512);
        cpp_lex_file(file, &parse->tokens);
        
        int count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        
        Cpp_Token *token = tokens;
        
        for (int i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(file.data + token->start, token->size);
                if (match(lexeme, "API_EXPORT")){
                    for (; i < count; ++i, ++token){
                        if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                            break;
                        }
                    }
                    
                    if (i < count){
                        --i;
                        --token;
                        
                        if (token->type == CPP_TOKEN_IDENTIFIER){
                            ++line_count;
                            
                            if (max_name_size < token->size){
                                max_name_size = token->size;
                            }
                        }
                    }
                }
            }
        }
    }
    
    int memory_size = (sizeof(String)*6 + sizeof(int) + sizeof(Argument_Breakdown) + sizeof(Documentation))*line_count;
    
    Function_Set function_set = {0};
    function_set.name        = (String*)malloc(memory_size);
    function_set.ret         = function_set.name + line_count;
    function_set.args        = function_set.ret + line_count;
    function_set.macros      = function_set.args + line_count;
    function_set.public_name = function_set.macros + line_count;
    function_set.doc_string  = function_set.public_name + line_count;
    function_set.valid       = (int*)(function_set.doc_string + line_count);
    function_set.breakdown   = (Argument_Breakdown*)(function_set.valid + line_count);
    function_set.doc         = (Documentation*)(function_set.breakdown + line_count);
    
    memset(function_set.name, 0, memory_size);
    
    int sig_count = 0;
    for (int J = 0; J < 2; ++J){
        String *code = &code_data[J];
        Parse *parse = &parses[J];
        
        // TODO(allen): KILL THIS FUCKIN' Cpp_File FUCKIN' NONSENSE HORSE SHIT!!!!!
        Cpp_File file;
        file.data = code->str;
        file.size = code->size;
        
        int count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        
        Cpp_Token *token = 0;
        
        // NOTE(allen): Header Parse
        token = tokens;
        for (int i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(file.data + token->start, token->size);
                if (match(lexeme, "API_EXPORT")){
                    ++i;
                    ++token;
                    if (i < count){
                        Cpp_Token *ret_start_token = token;
                        
                        for (; i < count; ++i, ++token){
                            if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                                break;
                            }
                        }
                        
                        Cpp_Token *args_start_token = token;
                        
                        if (i < count){
                            --i;
                            --token;
                            
                            function_set.name[sig_count] = make_string(file.data + token->start, token->size);
                            
                            int size = token->start - ret_start_token->start;
                            String ret = make_string(file.data + ret_start_token->start, size);
                            ret = chop_whitespace(ret);
                            function_set.ret[sig_count] = ret;
                            
                            for (; i < count; ++i, ++token){
                                if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                    break;
                                }
                            }
                            
                            if (i < count){
                                int size = token->start + token->size - args_start_token->start;;
                                function_set.args[sig_count] =
                                    make_string(file.data + args_start_token->start, size);
                                function_set.valid[sig_count] = true;
                                
                                int arg_count = 1;
                                Cpp_Token *arg_token = args_start_token;
                                for (; arg_token < token; ++arg_token){
                                    if (arg_token->type == CPP_TOKEN_COMMA){
                                        ++arg_count;
                                    }
                                }
                                
                                Argument_Breakdown *breakdown = &function_set.breakdown[sig_count];
                                breakdown->count = arg_count;
                                
                                int memory_size = (sizeof(String)*2)*arg_count;
                                
                                breakdown->param_string = (String*)malloc(memory_size);
                                breakdown->param_name = breakdown->param_string + arg_count;
                                
                                memset(breakdown->param_string, 0, memory_size);
                                
                                int arg_index = 0;
                                arg_token = args_start_token + 1;
                                int param_string_start = arg_token->start;
                                
                                for (; arg_token <= token; ++arg_token){
                                    if (arg_token->type == CPP_TOKEN_COMMA ||
                                        arg_token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                        
                                        int size = arg_token->start - param_string_start;
                                        String param_string = make_string(file.data + param_string_start, size);
                                        param_string = chop_whitespace(param_string);
                                        breakdown->param_string[arg_index] = param_string;
                                        
                                        for (Cpp_Token *param_name_token = arg_token - 1;
                                             param_name_token->start > param_string_start;
                                             --param_name_token){
                                            if (param_name_token->type == CPP_TOKEN_IDENTIFIER){
                                                int start = param_name_token->start;
                                                int size = param_name_token->size;
                                                breakdown->param_name[arg_index] = make_string(file.data + start, size);
                                                break;
                                            }
                                        }
                                        
                                        ++arg_index;
                                        
                                        ++arg_token;
                                        if (arg_token <= token){
                                            param_string_start = arg_token->start;
                                        }
                                        --arg_token;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (!function_set.valid[sig_count]){
                        function_set.ret[sig_count] = string_zero();
                        function_set.name[sig_count] = string_zero();
                        function_set.args[sig_count] = string_zero();
                        // TODO(allen): get warning line numbers
                        fprintf(stderr, "custom_api_spec.cpp(???) : generator warning : invalid function signature\n");
                    }
                    ++sig_count;
                }
            }
        }
        
        // NOTE(allen): Documentation Parse
        token = tokens;
        for (int i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(file.data + token->start, token->size);
                if (match(lexeme, "API_EXPORT")){
                    for (; i < count; ++i, ++token){
                        if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                            break;
                        }
                    }
                    
                    if (i < count){
                        --i;
                        --token;
                        
                        if (token->type == CPP_TOKEN_IDENTIFIER){
                            lexeme = make_string(file.data + token->start, token->size);
                            int match = 0;
                            if (string_set_match(function_set.name, sig_count, lexeme, &match)){
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_COMMENT){
                                        lexeme = make_string(file.data + token->start, token->size);
                                        if (check_and_fix_docs(&lexeme)){
                                            function_set.doc_string[match] = lexeme;
                                            perform_doc_parse(part, lexeme, &function_set.doc[match]);
                                            break;
                                        }
                                    }
                                    else if (token->type == CPP_TOKEN_BRACE_OPEN){
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < sig_count; ++i){
        String name_string = function_set.name[i];
        String *macro = function_set.macros + i;
        String *public_name = function_set.public_name + i;
        
        macro->size = 0;
        macro->memory_size = name_string.size+4;
        
        macro->str = (char*)malloc(macro->memory_size);
        copy(macro, name_string);
        to_upper(macro);
        append(macro, make_lit_string("_SIG"));
        
        
        public_name->size = 0;
        public_name->memory_size = name_string.size;
        
        public_name->str = (char*)malloc(public_name->memory_size);
        copy(public_name, name_string);
        to_lower(public_name);
    }
    
    // NOTE(allen): Header
    FILE *file = fopen(API_H, "wb");
    
    for (int i = 0; i < sig_count; ++i){
        String ret_string   = function_set.ret[i];
        String args_string  = function_set.args[i];
        String macro_string = function_set.macros[i];
        
        fprintf(file, "#define %.*s(n) %.*s n%.*s\n",
                macro_string.size, macro_string.str,
                ret_string.size, ret_string.str,
                args_string.size, args_string.str
                );
    }
    
    fprintf(file, "extern \"C\"{\n");
    for (int i = 0; i < sig_count; ++i){
        String name_string  = function_set.name[i];
        String macro_string = function_set.macros[i];
        
        fprintf(file, "    typedef %.*s(%.*s_Function);\n",
                macro_string.size, macro_string.str,
                name_string.size, name_string.str);
    }
    fprintf(file, "}\n");
    
    fprintf(file, "struct Application_Links{\n");
    fprintf(file,
            "    void *memory;\n"
            "    int memory_size;\n"
            );
    for (int i = 0; i < sig_count; ++i){
        String name_string  = function_set.name[i];
        String public_string = function_set.public_name[i];
        
        fprintf(file, "    %.*s_Function *%.*s;\n",
                name_string.size, name_string.str,
                public_string.size, public_string.str);
    }
    fprintf(file,
            "    void *cmd_context;\n"
            "    void *system_links;\n"
            "    void *current_coroutine;\n"
            "    int type_coroutine;\n"
            );
    fprintf(file, "};\n");
    
    fprintf(file, "#define FillAppLinksAPI(app_links) do{");
    for (int i = 0; i < sig_count; ++i){
        String name = function_set.name[i];
        String public_string = function_set.public_name[i];
        
        fprintf(file,
                "\\\n"
                "app_links->%.*s = %.*s;",
                public_string.size, public_string.str,
                name.size, name.str
                );
    }
    fprintf(file, " } while(false)\n");
    
    fclose(file);
    
    // NOTE(allen): Documentation
    if (global_settings.generate_docs){
        Typedef_Set typedef_set = {0};
        Struct_Set struct_set = {0};
        Enum_Set flag_set = {0};
        Enum_Set enum_set = {0};
        
        
        String type_code = file_dump("4coder_types.h");
        
        
        // TODO(allen): KILL THIS FUCKIN' Cpp_File FUCKIN' NONSENSE HORSE SHIT!!!!!
        Cpp_File type_file;
        type_file.data = type_code.str;
        type_file.size = type_code.size;
        
        Cpp_Token_Stack types_tokens = cpp_make_token_stack(512);
        cpp_lex_file(type_file, &types_tokens);
        
        int typedef_count = 0;
        int struct_count = 0;
        int flag_count = 0;
        int enum_count = 0;
        
        {
            int count = types_tokens.count;
            Cpp_Token *tokens = types_tokens.tokens;
            Cpp_Token *token = tokens;
            
            static String type_spec_keys[] = {
                make_lit_string("typedef"),
                make_lit_string("struct"),
                make_lit_string("union"),
                make_lit_string("ENUM"),
                make_lit_string("FLAGENUM"),
            };
            
            for (int i = 0; i < count; ++i, ++token){
                if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                    (token->type == CPP_TOKEN_KEY_TYPE_DECLARATION ||
                     token->type == CPP_TOKEN_IDENTIFIER)){
                    
                    String lexeme = make_string(type_file.data + token->start, token->size);
                    int match_index = 0;
                    if (string_set_match(type_spec_keys, ArrayCount(type_spec_keys),
                                         lexeme, &match_index)){
                        switch (match_index){
                            case 0: //typedef
                            ++typedef_count; break;
                            
                            case 1: case 2: //struct/union
                            ++struct_count; break;
                            
                            case 3: //ENUM
                            ++enum_count; break;
                            
                            case 4: //FLAGENUM
                            ++flag_count; break;
                        }
                    }
                }
            }
            
            if (typedef_count >  0){
                typedef_set.type = push_array(part, String, typedef_count);
                typedef_set.name = push_array(part, String, typedef_count);
                typedef_set.doc_string = push_array(part, String, typedef_count);
            }
            
            if (struct_count > 0){
                struct_set.structs = push_array(part, Struct_Member, struct_count);
            }
            
            if (enum_count > 0){
                enum_set.name = push_array(part, String, enum_count);
                enum_set.type = push_array(part, String, enum_count);
                enum_set.first_member = push_array(part, Enum_Member*, enum_count);
                enum_set.doc_string = push_array(part, String, enum_count);
            }
            
            if (flag_count > 0){
                flag_set.name = push_array(part, String, flag_count);
                flag_set.first_member = push_array(part, Enum_Member*, flag_count);
                flag_set.doc_string = push_array(part, String, flag_count);
            }
            
            int typedef_index = 0;
            int struct_index = 0;
            int flag_index = 0;
            int enum_index = 0;
            
            token = tokens;
            for (int i = 0; i < count; ++i, ++token){
                Assert(i == (i32)(token - tokens));
                if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                    (token->type == CPP_TOKEN_KEY_TYPE_DECLARATION ||
                     token->type == CPP_TOKEN_IDENTIFIER)){
                    
                    String lexeme = make_string(type_file.data + token->start, token->size);
                    int match_index = 0;
                    if (string_set_match(type_spec_keys, ArrayCount(type_spec_keys),
                                         lexeme, &match_index)){
                        switch (match_index){
                            case 0: //typedef
                            {
                                String doc_string = {0};
                                get_type_doc_string(type_file, tokens, i, &doc_string);
                                
                                int start_i = i;
                                Cpp_Token *start_token = token;
                                
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_SEMICOLON){
                                        break;
                                    }
                                }
                                
                                if (i < count){
                                    Cpp_Token *token_j = token;
                                    
                                    for (int j = i; j > start_i; --j, --token_j){
                                        if (token_j->type == CPP_TOKEN_IDENTIFIER){
                                            break;
                                        }
                                    }
                                    
                                    String name = make_string(type_file.data + token_j->start, token_j->size);
                                    name = skip_chop_whitespace(name);
                                    
                                    int type_start = start_token->start + start_token->size;
                                    int type_end = token_j->start;
                                    String type = make_string(type_file.data + type_start, type_end - type_start);
                                    type = skip_chop_whitespace(type);
                                    
                                    typedef_set.type[typedef_index] = type;
                                    typedef_set.name[typedef_index] = name;
                                    typedef_set.doc_string[typedef_index] = doc_string;
                                    ++typedef_index;
                                }
                            }break;
                            
                            case 1: case 2: //struct/union
                            {
                                if (parse_struct(part, (match_index == 1),
                                                 type_file, tokens, count, &token,
                                                 struct_set.structs + struct_index)){
                                    ++struct_index;
                                }
                                i = (int)(token - tokens);
                            }break;
                            
                            case 3: //ENUM
                            {
                                String doc_string = {0};
                                get_type_doc_string(type_file, tokens, i, &doc_string);
                                
                                int start_i = i;
                                
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                        break;
                                    }
                                }
                                
                                if (parse_enum(part, type_file,
                                               tokens, count,
                                               &token, start_i,
                                               enum_set, enum_index)){
                                    enum_set.doc_string[enum_index] = doc_string;
                                    ++enum_index;
                                }
                                i = (i32)(token - tokens);
                            }break;
                            
                            case 4: //FLAGENUM
                            {
                                String doc_string = {0};
                                get_type_doc_string(type_file, tokens, i, &doc_string);
                                
                                int start_i = i;
                                
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                        break;
                                    }
                                }
                                
                                if (parse_enum(part, type_file,
                                               tokens, count,
                                               &token, start_i,
                                               flag_set, flag_index)){
                                    flag_set.doc_string[flag_index] = doc_string;
                                    ++flag_index;
                                }
                                i = (i32)(token - tokens);
                            }break;
                        }
                    }
                }
            }
            
            typedef_count = typedef_index;
            struct_count = struct_index;
            enum_count = enum_index;
            flag_count = flag_index;
        }
        
        
        file = fopen(API_DOC, "wb");
        
        fprintf(file,
                "<html lang=\"en-US\">\n"
                "<head>\n"
                "<title>4coder API Docs</title>\n"
                "<style>\n"
                
                "body { "
                "background: " BACK_COLOR "; "
                "color: " TEXT_COLOR "; "
                "}\n"
                
                // H things
                "h1,h2,h3,h4 { "
                "color: " POP_COLOR_1 "; "
                "margin: 0; "
                "}\n"
                
                "h3 { "
                "margin-top: 5mm; margin-bottom: 5mm; "
                "}\n"
                
                "h4 { "
                "font-size: 1.1em; "
                "}\n"
                
                // ANCHORS
                "a { "
                "color: " POP_COLOR_1 "; "
                "text-decoration: none; "
                "}\n"
                "a:visited { "
                "color: " VISITED_LINK "; "
                "}\n"
                "a:hover { "
                "background: " POP_BACK_1 "; "
                "}\n"
                
                // LIST
                "ul { "
                "list-style: none; "
                "padding: 0; "
                "margin: 0; "
                "}\n"
                "li { "
                "padding-left: 1em;"
                "text-indent: -.7em;"
                "}\n"
                "li:before { "
                "content: \"4\"; "
                "color: " POP_COLOR_2 "; "
                "font-family:\"Webdings\"; "
                "}\n"
                
                "</style>\n"
                "</head>\n"
                "<body>\n"
                "<div style='font-family:Arial; margin: 0 auto; "
                "width: 800px; text-align: justify; line-height: 1.25;'>\n"
                "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4coder API</h1>\n"
                );
        
#define MAJOR_SECTION "1"
        
        fprintf(file,
                "<h2>&sect;"MAJOR_SECTION" Introduction</h2>\n"
                "<div>\n"
                
                "<p>\n"
                "This is the documentation for " VERSION " The documentation is still under "
                "construction so some of the links are linking to sections that have not "
                "been written yet.  What is here should be correct and I suspect useful "
                "even without some of the other sections. "
                "</p>\n"
                
                "<p>\n"
                "If you have questions or discover errors please contact "
                "<span style='"CODE_STYLE"'>editor@4coder.net</span> or "
                "to get help from community members you can post on the "
                "4coder forums hosted on handmade.network at "
                "<span style='"CODE_STYLE"'>4coder.handmade.network</span>"
                "</p>\n"
                
                "</div>\n");
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "2"
        // TODO(allen): Write the 4coder system descriptions.
        fprintf(file, "<h2>&sect;"MAJOR_SECTION" 4coder Systems</h2>\n");
        {
            fprintf(file,
                "<div><i>\n"
                "Coming Soon"
                "</i><div>\n");
        }
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "3"
        
        fprintf(file, "<h2>&sect;"MAJOR_SECTION" Types and Functions</h2>\n");
        {
#undef SECTION
#define SECTION MAJOR_SECTION".1"
            
            fprintf(file,
                    "<h3>&sect;"SECTION" Function List</h3>\n"
                    "<ul>\n");
            
            for (int i = 0; i < sig_count; ++i){
                String name = function_set.public_name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            fprintf(file, "</ul>\n");
            
#undef SECTION
#define SECTION MAJOR_SECTION".2"
            
            fprintf(file,
                    "<h3>&sect;"SECTION" Type List</h3>\n"
                    "<ul>\n"
                    );
            
            for (int i = 0; i < typedef_count; ++i){
                String name = typedef_set.name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            for (int i = 0; i < enum_count; ++i){
                String name = enum_set.name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            for (int i = 0; i < flag_count; ++i){
                String name = flag_set.name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            for (int i = 0; i < struct_count; ++i){
                String name = struct_set.structs[i].name;
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            fprintf(file, "</ul>\n");
            
#undef SECTION
#define SECTION MAJOR_SECTION".3"
            
            fprintf(file, "<h3>&sect;"SECTION" Function Descriptions</h3>\n");
            for (int i = 0; i < sig_count; ++i){
                String name = function_set.public_name[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        " <h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        " <div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, i+1,
                        name.size, name.str
                        );
                
                String ret = function_set.ret[i];
                fprintf(file,
                        "%.*s app->%.*s(\n"
                        "<div style='margin-left: 4mm;'>",
                        ret.size, ret.str, name.size, name.str);
                
                Argument_Breakdown *breakdown = &function_set.breakdown[i];
                int arg_count = breakdown->count;
                for (int j = 0; j < arg_count; ++j){
                    String param_string = breakdown->param_string[j];
                    if (j < arg_count - 1){
                        fprintf(file, "%.*s,<br>", param_string.size, param_string.str);
                    }
                    else{
                        fprintf(file, "%.*s<br>", param_string.size, param_string.str);
                    }
                }
                
                fprintf(file,
                        "</div>)\n"
                        "</div>\n");
                
                if (function_set.doc_string[i].size == 0){
                    fprintf(file, "No documentation generated for this function, assume it is non-public.\n");
                    fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
                }
                
                Documentation *doc = &function_set.doc[i];
                
                int doc_param_count = doc->param_count;
                if (doc_param_count > 0){
                    fprintf(file, DOC_HEAD_OPEN"Parameters"DOC_HEAD_CLOSE);
                    
                    for (int j = 0; j < doc_param_count; ++j){
                        String param_name = doc->param_name[j];
                        String param_docs = doc->param_docs[j];
                        
                        // TODO(allen): check that param_name is actually
                        // a parameter to this function!
                        
                        fprintf(file,
                                "<div>\n"
                                DOC_ITEM_HEAD_OPEN"%.*s"DOC_ITEM_HEAD_CLOSE"\n"
                                "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE"</div>\n"
                                "</div>\n",
                                param_name.size, param_name.str,
                                param_docs.size, param_docs.str
                                );
                    }
                }
                
                String ret_doc = doc->return_doc;
                if (ret_doc.size != 0){
                    fprintf(file, DOC_HEAD_OPEN"Return"DOC_HEAD_CLOSE);
                    fprintf(file,
                            DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE,
                            ret_doc.size, ret_doc.str
                            );
                }
                
                String main_doc = doc->main_doc;
                if (main_doc.size != 0){
                    fprintf(file, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                    fprintf(file,
                            DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE,
                            main_doc.size, main_doc.str
                            );
                }
                
                print_see_also(file, doc);
                
                fprintf(file, "</div><hr>\n");
            }
            
#undef SECTION
#define SECTION MAJOR_SECTION".4"
            
            fprintf(file, "<h3>&sect;"SECTION" Type Descriptions</h3>\n");
            int I = 1;
            for (int i = 0; i < typedef_count; ++i, ++I){
                String name = typedef_set.name[i];
                String type = typedef_set.type[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        " <h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        " <div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, I,
                        name.size, name.str
                        );
                
                // NOTE(allen): Code box
                {
                    fprintf(file,
                            "typedef %.*s %.*s;",
                            type.size, type.str,
                            name.size, name.str
                            );
                }
                
                fprintf(file, "</div>\n");
                
                // NOTE(allen): Descriptive section
                {
                    String doc_string = typedef_set.doc_string[i];
                    Documentation doc = {0};
                    perform_doc_parse(part, doc_string, &doc);
                    
                    String main_doc = doc.main_doc;
                    if (main_doc.size != 0){
                        fprintf(file, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                        fprintf(file,
                                DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE,
                                main_doc.size, main_doc.str
                                );
                    }
                    
                    print_see_also(file, &doc);
                }
                
                fprintf(file, "</div><hr>\n");
            }
            
            for (int i = 0; i < enum_count; ++i, ++I){
                String name = enum_set.name[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        " <h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        " <div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, I,
                        name.size, name.str
                        );
                
                // NOTE(allen): Code box
                {
                    fprintf(file,
                            "enum %.*s;",
                            name.size, name.str);
                }
                
                fprintf(file, "</div>\n");
                
                // NOTE(allen): Descriptive section
                {
                    String doc_string = enum_set.doc_string[i];
                    Documentation doc = {0};
                    perform_doc_parse(part, doc_string, &doc);
                    
                    String main_doc = doc.main_doc;
                    if (main_doc.size != 0){
                        fprintf(file, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                        fprintf(file,
                                DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE,
                                main_doc.size, main_doc.str
                                );
                    }
                    
                    if (enum_set.first_member[i]){
                        fprintf(file, DOC_HEAD_OPEN"Values"DOC_HEAD_CLOSE);
                        for (Enum_Member *member = enum_set.first_member[i];
                             member;
                             member = member->next){
                            Documentation doc = {0};
                            perform_doc_parse(part, member->doc_string, &doc);
                            
                            fprintf(file,
                                    "<div>\n"
                                    "<div><span style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN
                                    "%.*s"DOC_ITEM_HEAD_INL_CLOSE"</span></div>\n"
                                    "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE"</div>\n"
                                    "</div>\n",
                                    member->name.size, member->name.str,
                                    doc.main_doc.size, doc.main_doc.str
                                    );
                        }
                    }
                    
                    print_see_also(file, &doc);
                }
                
                fprintf(file, "</div><hr>\n");
            }
            
            for (int i = 0; i < flag_count; ++i, ++I){
                String name = flag_set.name[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        " <h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        " <div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, I,
                        name.size, name.str
                        );
                
                // NOTE(allen): Code box
                {
                    fprintf(file,
                            "enum %.*s;",
                            name.size, name.str);
                }
                
                fprintf(file, "</div>\n");
                
                // NOTE(allen): Descriptive section
                {
                    String doc_string = flag_set.doc_string[i];
                    Documentation doc = {0};
                    perform_doc_parse(part, doc_string, &doc);
                    
                    String main_doc = doc.main_doc;
                    if (main_doc.size != 0){
                        fprintf(file, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                        fprintf(file,
                                DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE,
                                main_doc.size, main_doc.str
                                );
                    }
                    
                    if (flag_set.first_member[i]){
                        fprintf(file, DOC_HEAD_OPEN"Flags"DOC_HEAD_CLOSE);
                        for (Enum_Member *member = flag_set.first_member[i];
                             member;
                             member = member->next){
                            Documentation doc = {0};
                            perform_doc_parse(part, member->doc_string, &doc);
                            
                            fprintf(file,
                                    "<div>\n"
                                    "<div><span style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN
                                    "%.*s"DOC_ITEM_HEAD_INL_CLOSE" = %.*s</span></div>\n"
                                    "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE"</div>\n"
                                    "</div>\n",
                                    member->name.size, member->name.str,
                                    member->value.size, member->value.str,
                                    doc.main_doc.size, doc.main_doc.str
                                    );
                        }
                    }
                    
                    print_see_also(file, &doc);
                }
                
                fprintf(file, "</div><hr>\n");
            }
            
            for (int i = 0; i < struct_count; ++i, ++I){
                Struct_Member *member = &struct_set.structs[i];
                String name = member->name;
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        " <h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        " <div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, I,
                        name.size, name.str
                        );
                
                // NOTE(allen): Code box
                {
                    print_struct_html(file, member);
                }
                
                fprintf(file, "</div>\n");
                
                // NOTE(allen): Descriptive section
                {
                    String doc_string = member->doc_string;
                    Documentation doc = {0};
                    perform_doc_parse(part, doc_string, &doc);
                    
                    String main_doc = doc.main_doc;
                    if (main_doc.size != 0){
                        fprintf(file, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                        fprintf(file,
                                DOC_ITEM_OPEN"%.*s"DOC_ITEM_CLOSE,
                                main_doc.size, main_doc.str
                                );
                    }
                    
                    if (member->first_child){
                        fprintf(file, DOC_HEAD_OPEN"Fields"DOC_HEAD_CLOSE);
                        print_struct_docs(file, part, member);
                    }
                    
                    print_see_also(file, &doc);
                }
                
                fprintf(file, "</div><hr>\n");
            }
        }
        
        fprintf(file,
                "</div>\n"
                "</body>\n"
                "</html>\n"
                );
        
        fclose(file);
    }

    return(filename);
}

int main(int argc, char **argv){
    char *filename = 0;
    
    memset(&global_settings, 0, sizeof(global_settings));
    
    global_settings.generate_docs = true;
    
    filename = generate_keycode_enum();
    filename = generate_style();
    filename = generate_custom_headers();
}

// BOTTOM

