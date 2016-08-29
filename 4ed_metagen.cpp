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
#include "internal_4coder_string.cpp"

#include "4cpp_lexer_types.h"

#define FCPP_LEXER_IMPLEMENTATION
#include "4cpp_lexer.h"

#include "4coder_version.h"

#include "4coder_mem.h"

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
    int32_t i = 0;
    int32_t size = str->size;
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
    int32_t i = 0;
    int32_t size = str->size;
    for (c = str->str; i < size; ++c, ++i){
        *c = char_to_upper(*c);
    }
}

void to_camel(char *src, char *dst){
    char *c, ch;
    int32_t is_first = 1;
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

void struct_fields(FILE *file, Struct_Field *fields, int32_t count){
    int32_t i;
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
    int32_t i, count;
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
            "global_key_name(int32_t key_code, int32_t *size){\n"
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
    int32_t len;
    
    len = (int32_t)strlen(tag);
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
    int32_t count, i;
    
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
    int32_t count;
    String *param_string;
    String *param_name;
} Argument_Breakdown;

typedef struct Documentation{
    int32_t param_count;
    String *param_name;
    String *param_docs;
    String return_doc;
    String main_doc;
    int32_t see_also_count;
    String *see_also;
} Documentation;

struct App_API{
    String *macros;
    String *public_name;
};

struct Function_Set{
    String *name;
    String *ret;
    String *args;
    String *body;
    String *marker;
    String *cpp_name;
    
    String *doc_string;
    
    int32_t    *is_macro;
    int32_t    *valid;
    
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
zero_index(Function_Set fnc_set, int32_t sig_count){
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
    int32_t pos = find_s_char(source, 0, '\n');
    
    line = substr(source, 0, pos);
    
    return(line);
}

String
get_next_line(String source, String line){
    String next = {0};
    int32_t pos = (int32_t)(line.str - source.str) + line.size;
    int32_t start = 0;
    
    if (pos < source.size){
        assert(source.str[pos] == '\n');
        start = pos + 1;
        
        if (start < source.size){
            pos = find_s_char(source, start, '\n');
            next = substr(source, start, pos - start);
        }
    }
    
    return(next);
}

static int32_t
is_comment(String str){
    int32_t result = 0;
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

static int32_t
check_and_fix_docs(String *lexeme){
    int32_t result = false;
    
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
doc_parse_note(String source, int32_t *pos){
    String result = {0};
    
    int32_t p = *pos;
    int32_t start = p;
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
doc_parse_note_string(String source, int32_t *pos){
    String result = {0};
    
    assert(source.str[*pos] == '(');
    
    int32_t p = *pos + 1;
    int32_t start = p;
    
    int32_t nest_level = 0;
    
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
doc_parse_parameter(String source, int32_t *pos){
    String result = {0};
    
    int32_t p = *pos;
    int32_t start = p;
    
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
doc_parse_last_parameter(String source, int32_t *pos){
    String result = {0};
    
    int32_t p = *pos;
    int32_t start = p;
    
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
    int32_t keep_parsing = true;
    int32_t pos = 0;
    
    int32_t param_count = 0;
    int32_t see_count = 0;
    
    do{
        String doc_note = doc_parse_note(doc_string, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            int32_t doc_note_type;
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
        int32_t memory_size = sizeof(String)*(2*param_count + see_count);
        doc->param_name = push_array(part, String, memory_size);
        doc->param_docs = doc->param_name + param_count;
        doc->see_also   = doc->param_docs + param_count;
        
        doc->param_count = param_count;
        doc->see_also_count = see_count;
    }
    
    int32_t param_index = 0;
    int32_t see_index = 0;
    
    keep_parsing = true;
    pos = 0;
    do{
        String doc_note = doc_parse_note(doc_string, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            int32_t doc_note_type;
            if (string_set_match(doc_note_string, ArrayCount(doc_note_string), doc_note, &doc_note_type)){
                
                String doc_note_string = doc_parse_note_string(doc_string, &pos);
                
                switch (doc_note_type){
                    case DOC_PARAM:
                    {
                        assert(param_index < param_count);
                        int32_t param_pos = 0;
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

static int32_t
get_type_doc_string(char *data, Cpp_Token *tokens, int32_t i,
                    String *doc_string){
    int32_t result = false;
    
    if (i > 0){
        Cpp_Token *prev_token = tokens + i - 1;
        if (prev_token->type == CPP_TOKEN_COMMENT){
            *doc_string = make_string(data + prev_token->start, prev_token->size);
            if (check_and_fix_docs(doc_string)){
                result = true;
            }
        }
    }
    
    return(result);
}

static int32_t
parse_struct(Partition *part, int32_t is_struct,
             char *data, Cpp_Token *tokens, int32_t count,
             Cpp_Token **token_ptr,
             Struct_Member *top_member);

static int32_t
parse_struct_member(Partition *part,
                    char *data, Cpp_Token *tokens, int32_t count,
                    Cpp_Token **token_ptr,
                    Struct_Member *member){
    
    int32_t result = false;
    
    Cpp_Token *token = *token_ptr;
    int32_t i = (int32_t)(token - tokens);
    
    String doc_string = {0};
    get_type_doc_string(data, tokens, i, &doc_string);
    
    int32_t start_i = i;
    Cpp_Token *start_token = token;
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_SEMICOLON){
            break;
        }
    }
    
    if (i < count){
        Cpp_Token *token_j = token;
        
        int32_t nest_level = 0;
        for (int32_t j = i; j > start_i; --j, --token_j){
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
        
        String name = make_string(data + token_j->start, token_j->size);
        name = skip_chop_whitespace(name);
        
        int32_t type_start = start_token->start;
        int32_t type_end = token_j->start;
        String type = make_string(data + type_start, type_end - type_start);
        type = skip_chop_whitespace(type);
        
        type_start = token_j->start + token_j->size;
        type_end = token->start;
        
        String type_postfix = make_string(data + type_start, type_end - type_start);
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
                         char *data, Cpp_Token *tokens, int32_t count,
                         Cpp_Token **token_ptr){
    Struct_Member *result = 0;
    
    Cpp_Token *token = *token_ptr;
    int32_t i = (int32_t)(token - tokens);
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_IDENTIFIER ||
            (token->flags & CPP_TFLAG_IS_KEYWORD)){
            String lexeme = make_string(data + token->start, token->size);
            
            if (match_ss(lexeme, make_lit_string("struct"))){
                Struct_Member *member = push_struct(part, Struct_Member);
                if (parse_struct(part, true, data, tokens, count, &token, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else if (match_ss(lexeme, make_lit_string("union"))){
                Struct_Member *member = push_struct(part, Struct_Member);
                if (parse_struct(part, false, data, tokens, count, &token, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else{
                Struct_Member *member = push_struct(part, Struct_Member);
                if (parse_struct_member(part, data, tokens, count, &token, member)){
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

static int32_t
parse_struct(Partition *part, int32_t is_struct,
             char *data, Cpp_Token *tokens, int32_t count,
             Cpp_Token **token_ptr,
             Struct_Member *top_member){
    
    int32_t result = false;
    
    Cpp_Token *token = *token_ptr;
    int32_t i = (int32_t)(token - tokens);
    
    String doc_string = {0};
    get_type_doc_string(data, tokens, i, &doc_string);
    
    int32_t start_i = i;
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_BRACE_OPEN){
            break;
        }
    }
    
    if (i < count){
        Cpp_Token *token_j = token;
        int32_t j = i;
        
        for (; j > start_i; --j, --token_j){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        String name = {0};
        
        if (j != start_i){
            name = make_string(data + token_j->start, token_j->size);
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
            parse_struct_next_member(part, data, tokens, count, &token);
        
        if (new_member){
            top_member->first_child = new_member;
            
            Struct_Member *head_member = new_member;
            for(;;){
                new_member = 
                    parse_struct_next_member(part, data, tokens, count, &token);
                if (new_member){
                    head_member->next_sibling = new_member;
                    head_member = new_member;
                }
                else{
                    break;
                }
            }
        }
        
        i = (int32_t)(token - tokens);
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
    
    if (match_ss(type, make_lit_string("struct")) ||
        match_ss(type, make_lit_string("union"))){
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

static void
print_function_html(FILE *file, Function_Set function_set, int32_t i, String name,
                    char *function_call_head){
    String ret = function_set.ret[i];
    fprintf(file,
            "%.*s %s%.*s(\n"
            "<div style='margin-left: 4mm;'>",
            ret.size, ret.str,
            function_call_head,
            name.size, name.str);
    
    Argument_Breakdown *breakdown = &function_set.breakdown[i];
    int32_t arg_count = breakdown->count;
    for (int32_t j = 0; j < arg_count; ++j){
        String param_string = breakdown->param_string[j];
        if (j < arg_count - 1){
            fprintf(file, "%.*s,<br>", param_string.size, param_string.str);
        }
        else{
            fprintf(file, "%.*s<br>", param_string.size, param_string.str);
        }
    }
    
    fprintf(file, "</div>)\n");
}

static void
print_macro_html(FILE *file, Function_Set function_set, int32_t i, String name){
    Argument_Breakdown *breakdown = &function_set.breakdown[i];
    int32_t arg_count = breakdown->count;
    if (arg_count == 0){
        fprintf(file,
                "#define %.*s()",
                name.size, name.str);
    }
    else if (arg_count == 1){
        String param_string = breakdown->param_string[0];
        fprintf(file,
                "#define %.*s(%.*s)",
                name.size, name.str,
                param_string.size, param_string.str);
    }
    else{
        fprintf(file,
                "#define %.*s(\n"
                "<div style='margin-left: 4mm;'>",
                name.size, name.str);
        
        for (int32_t j = 0; j < arg_count; ++j){
            String param_string = breakdown->param_string[j];
            if (j < arg_count - 1){
                fprintf(file, "%.*s,<br>", param_string.size, param_string.str);
            }
            else{
                fprintf(file, "%.*s<br>", param_string.size, param_string.str);
            }
        }
        
        fprintf(file, "</div>)\n");
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
        if (match_ss(type, make_lit_string("struct")) ||
            match_ss(type, make_lit_string("union"))){
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
    int32_t doc_see_count = doc->see_also_count;
    if (doc_see_count > 0){
        fprintf(file, DOC_HEAD_OPEN"See Also"DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_see_count; ++j){
            String see_also = doc->see_also[j];
            fprintf(file,
                    DOC_ITEM_OPEN"<a href='#%.*s_doc'>%.*s</a>"DOC_ITEM_CLOSE,
                    see_also.size, see_also.str,
                    see_also.size, see_also.str
                    );
        }
    }
}

static int32_t
parse_enum(Partition *part, char *data,
           Cpp_Token *tokens, int32_t count,
           Cpp_Token **token_ptr, int32_t start_i,
           Enum_Set flag_set, int32_t flag_index){
    
    int32_t result = false;
    
    Cpp_Token *token = *token_ptr;
    int32_t i = (int32_t)(token - tokens);
    
    if (i < count){
        Cpp_Token *token_j = token;
        
        for (int32_t j = i; j > start_i; --j, --token_j){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        String name = make_string(data + token_j->start, token_j->size);
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
                    get_type_doc_string(data, tokens, i, &doc_string);
                    
                    String name = make_string(data + token->start, token->size);
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
                        
                        int32_t val_start = start_token->start + start_token->size;
                        int32_t val_end = token->start;
                        
                        value = make_string(data + val_start, val_end - val_start);
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

static App_API
allocate_app_api(int32_t count){
    App_API app_api = {0};
    int32_t memory_size = (sizeof(String)*2)*count;
    app_api.macros      = (String*)malloc(memory_size);
    app_api.public_name = app_api.macros + count;
    memset(app_api.macros, 0, memory_size);
    return(app_api);
}

static Function_Set
allocate_function_set(int32_t count){
    Function_Set function_set = {0};
    int32_t memory_size = (sizeof(String)*7 +
                           sizeof(int32_t)*2 +
                           sizeof(Argument_Breakdown) +
                           sizeof(Documentation))*count;
    
    String *str_ptr = (String*)malloc(memory_size);
    function_set.name        = str_ptr; str_ptr += count;
    function_set.ret         = str_ptr; str_ptr += count;
    function_set.args        = str_ptr; str_ptr += count;
    function_set.body        = str_ptr; str_ptr += count;
    function_set.marker      = str_ptr; str_ptr += count;
    function_set.cpp_name    = str_ptr; str_ptr += count;
    function_set.doc_string  = str_ptr; str_ptr += count;
    
    function_set.is_macro    = (int32_t*)(function_set.doc_string + count);
    function_set.valid       = function_set.is_macro + count;
    
    function_set.breakdown   = (Argument_Breakdown*)(function_set.valid + count);
    
    function_set.doc         = (Documentation*)(function_set.breakdown + count);
    
    memset(function_set.name, 0, memory_size);
    return(function_set);
}

static Argument_Breakdown
allocate_argument_breakdown(int32_t count){
    Argument_Breakdown breakdown = {0};
    int32_t memory_size = (sizeof(String)*2)*count;
    breakdown.count = count;
    breakdown.param_string = (String*)malloc(memory_size);
    breakdown.param_name = breakdown.param_string + count;
    memset(breakdown.param_string, 0, memory_size);
    return(breakdown);
}

static Argument_Breakdown
do_parameter_parse(char *data, Cpp_Token *args_start_token, Cpp_Token *token){
    int32_t arg_index = 0;
    Cpp_Token *arg_token = args_start_token + 1;
    int32_t param_string_start = arg_token->start;
    
    int32_t arg_count = 1;
    arg_token = args_start_token;
    for (; arg_token < token; ++arg_token){
        if (arg_token->type == CPP_TOKEN_COMMA){
            ++arg_count;
        }
    }
    
    Argument_Breakdown breakdown = allocate_argument_breakdown(arg_count);
    
    arg_token = args_start_token + 1;
    for (; arg_token <= token; ++arg_token){
        if (arg_token->type == CPP_TOKEN_COMMA ||
            arg_token->type == CPP_TOKEN_PARENTHESE_CLOSE){
            
            int32_t size = arg_token->start - param_string_start;
            String param_string = make_string(data + param_string_start, size);
            param_string = chop_whitespace(param_string);
            breakdown.param_string[arg_index] = param_string;
            
            for (Cpp_Token *param_name_token = arg_token - 1;
                 param_name_token->start > param_string_start;
                 --param_name_token){
                if (param_name_token->type == CPP_TOKEN_IDENTIFIER){
                    int32_t start = param_name_token->start;
                    int32_t size = param_name_token->size;
                    breakdown.param_name[arg_index] = make_string(data + start, size);
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
    
    return(breakdown);
}

static int32_t
do_function_parse_check(int32_t *index, Cpp_Token **token_ptr, int32_t count){
    int32_t result = false;
    
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    {
        for (; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                break;
            }
        }
        
        if (i < count){
            --i;
            --token;
            
            if (token->type == CPP_TOKEN_IDENTIFIER){
                result = true;
            }
        }
    }
    
    *index = i;
    *token_ptr = token;
    
    return(result);
}

static int32_t
do_function_get_doc(int32_t *index, Cpp_Token **token_ptr, int32_t count,
                    char *data, String *doc_string){
    int32_t result = false;
    
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_COMMENT){
            String lexeme = make_string(data + token->start, token->size);
            if (check_and_fix_docs(&lexeme)){
                *doc_string = lexeme;
                result = true;
                break;
            }
        }
        else if (token->type == CPP_TOKEN_BRACE_OPEN){
            break;
        }
    }
    
    *index = i;
    *token_ptr = token;
    
    return(result);
}

static String
get_lexeme(Cpp_Token token, char *code){
    String str = make_string(code + token.start, token.size);
    return(str);
}

static int32_t
do_parse_cpp_name(int32_t *i_ptr, Cpp_Token **token_ptr, int32_t count, char *data, String *name){
    int32_t result = false;
    
    int32_t i = *i_ptr;
    Cpp_Token *token = *token_ptr;
    
    int32_t i_start = i;
    Cpp_Token *token_start = token;
    
    ++i, ++token;
    if (i < count && token->type == CPP_TOKEN_PARENTHESE_OPEN){
        ++i, ++token;
        if (i < count && token->type == CPP_TOKEN_IDENTIFIER){
            ++i, ++token;
            if (i < count && token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                *name = get_lexeme(*(token-1), data);
                result = true;
            }
        }
    }
    
    if (!result){
        i = i_start;
        token = token_start;
    }
    
    *i_ptr = i;
    *token_ptr = token;
    
    return(result);
}

static int32_t
do_function_parse(int32_t *index, Cpp_Token **token_ptr, int32_t count, Cpp_Token *ret_start_token,
                  char *data, Function_Set function_set, int32_t sig_count, String cpp_name){
    int32_t result = false;
    
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    Cpp_Token *args_start_token = token+1;
    
    function_set.name[sig_count] = make_string(data + token->start, token->size);
    
    int32_t size = token->start - ret_start_token->start;
    String ret = make_string(data + ret_start_token->start, size);
    ret = chop_whitespace(ret);
    function_set.ret[sig_count] = ret;
    
    for (; i < count; ++i, ++token){
        if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
            break;
        }
    }
    
    if (i < count){
        int32_t size = token->start + token->size - args_start_token->start;;
        function_set.args[sig_count] =
            make_string(data + args_start_token->start, size);
        function_set.valid[sig_count] = true;
        result = true;
        
        Argument_Breakdown *breakdown = &function_set.breakdown[sig_count];
        *breakdown = do_parameter_parse(data, args_start_token, token);
        
        function_set.cpp_name[sig_count] = cpp_name;
    }
    
    *index = i;
    *token_ptr = token;
    
    return(result);
}

static int32_t
do_full_function_parse(int32_t *index, Cpp_Token **token_ptr, int32_t count, char *data,
                       Function_Set function_set, int32_t sig_count, String cpp_name){
    int32_t result = false;
    
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    {
        function_set.marker[sig_count] = make_string(data + token->start, token->size);
        
        int32_t j = i;
        Cpp_Token *jtoken = token;
        
        if (do_function_parse_check(&j, &jtoken, count)){
            if (token->type == CPP_TOKEN_IDENTIFIER){
                String doc_string = {0};
                if (do_function_get_doc(&j, &jtoken, count, data, &doc_string)){
                    function_set.doc_string[sig_count] = doc_string;
                }
            }
        }
        
        ++i, ++token;
        if (i < count){
            Cpp_Token *ret_start_token = token;
            if (do_function_parse_check(&i, &token, count)){
                if (do_function_parse(&i, &token, count, ret_start_token,
                                      data, function_set, sig_count, cpp_name)){
                    result = true;
                }
            }
        }
    }
    
    *index = i;
    *token_ptr = token;
    
    return(result);
}

static int32_t
do_macro_parse_check(int32_t *index, Cpp_Token **token_ptr, int32_t count){
    int32_t result = false;
    
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    {
        ++i, ++token;
        if (i < count){
            if (token->type == CPP_TOKEN_COMMENT){
                ++i, ++token;
                if (i < count){
                    if (token->type == CPP_PP_DEFINE){
                        result = true;
                    }
                }
            }
        }
    }
    
    *index = i;
    *token_ptr = token;
    
    return(result);
}

static int32_t
do_macro_parse(int32_t *index, Cpp_Token **token_ptr, int32_t count,
               char *data, Function_Set macro_set, int32_t sig_count){
    int32_t result = false;
    
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    if (i > 0){
        Cpp_Token *doc_token = token-1;
        
        String doc_string = make_string(data + doc_token->start, doc_token->size);
        
        if (check_and_fix_docs(&doc_string)){
            macro_set.doc_string[sig_count] = doc_string;
            
            for (; i < count; ++i, ++token){
                if (token->type == CPP_TOKEN_IDENTIFIER){
                    break;
                }
            }
            
            if (i < count && (token->flags & CPP_TFLAG_PP_BODY)){
                macro_set.name[sig_count] = make_string(data + token->start, token->size);
                
                ++i, ++token;
                if (i < count){
                    Cpp_Token *args_start_token = token;
                    for (; i < count; ++i, ++token){
                        if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                            break;
                        }
                    }
                    
                    if (i < count){
                        int32_t start = args_start_token->start;
                        int32_t end = token->start + token->size;
                        macro_set.args[sig_count] = make_string(data + start, end - start);
                        
                        Argument_Breakdown *breakdown = &macro_set.breakdown[sig_count];
                        *breakdown = do_parameter_parse(data, args_start_token, token);
                        
                        ++i, ++token;
                        if (i < count){
                            Cpp_Token *body_start = token;
                            
                            if (body_start->flags & CPP_TFLAG_PP_BODY){
                                for (; i < count; ++i, ++token){
                                    if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                        break;
                                    }
                                }
                                
                                --i, --token;
                                
                                Cpp_Token *body_end = token;
                                
                                start = body_start->start;
                                end = body_end->start + body_end->size;
                                macro_set.body[sig_count] = make_string(data + start, end - start);
                            }
                        }
                        
                        macro_set.is_macro[sig_count] = true;
                        macro_set.valid[sig_count] = true;
                        result = true;
                    }
                }
            }
        }
    }
    
    *index = i;
    *token_ptr = token;
    
    return(result);
}

struct String_Function_Marker{
    int32_t parse_function;
    int32_t is_inline;
    int32_t parse_doc;
    int32_t cpp_name;
};

static String_Function_Marker
do_string_function_marker_check(String lexeme){
    String_Function_Marker result = {0};
    
    if (match_ss(lexeme, make_lit_string("FSTRING_INLINE"))){
        result.is_inline = true;
        result.parse_function = true;
    }
    else if (match_ss(lexeme, make_lit_string("FSTRING_LINK"))){
        result.parse_function = true;
    }
    else if (match_ss(lexeme, make_lit_string("DOC_EXPORT"))){
        result.parse_doc = true;
    }
    else if (match_ss(lexeme, make_lit_string("CPP_NAME"))){
        result.cpp_name = true;
    }
    
    return(result);
}

static String
get_string(char *data, int32_t start, int32_t end){
    return(make_string(data + start, end - start));
}

static void
print_str(FILE *file, String str){
    if (str.size > 0){
        fprintf(file, "%.*s", str.size, str.str);
    }
}

static void
print_function_body_code(FILE *file, int32_t *index, Cpp_Token **token_ptr, int32_t count, String *code,
                         int32_t start){
    int32_t i = *index;
    Cpp_Token *token = *token_ptr;
    
    String pstr = {0};
    
    int32_t nest_level = 0;
    int32_t finish = false;
    int32_t do_whitespace_print = false;
    for (; i < count; ++i, ++token){
        if (do_whitespace_print){
            pstr = get_string(code->str, start, token->start);
            print_str(file, pstr);
        }
        else{
            do_whitespace_print = true;
        }
        
        int32_t do_print = true;
        if (token->type == CPP_TOKEN_COMMENT){
            String lexeme = make_string(code->str + token->start, token->size);
            if (check_and_fix_docs(&lexeme)){
                do_print = false;
            }
        }
        else if (token->type == CPP_TOKEN_BRACE_OPEN){
            ++nest_level;
        }
        else if (token->type == CPP_TOKEN_BRACE_CLOSE){
            --nest_level;
            if (nest_level == 0){
                finish = true;
            }
        }
        
        if (i < count){
            if (do_print){
                pstr = make_string(code->str + token->start, token->size);
                print_str(file, pstr);
            }
            
            start = token->start + token->size;
        }
        
        if (finish){
            break;
        }
    }
    
    *index = i;
    *token_ptr = token;
}

static void
print_function_docs(FILE *file, Partition *part, String name, String doc_string){
    if (doc_string.size == 0){
        fprintf(file, "No documentation generated for this function, assume it is non-public.\n");
        fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
    }
    
    Documentation doc_ = {0};
    Documentation *doc = &doc_;
    
    perform_doc_parse(part, doc_string, doc);
    
    int32_t doc_param_count = doc->param_count;
    if (doc_param_count > 0){
        fprintf(file, DOC_HEAD_OPEN"Parameters"DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_param_count; ++j){
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
}

char*
generate_custom_headers(){
#define API_H "4coder_custom_api.h"
#define API_DOC "4coder_API.html"
#define STRING_H "4coder_string.h"
    
    int32_t size = Mbytes(512);
    void *mem = malloc(size);
    memset(mem, 0, size);
    
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    char *filename = API_H " & " API_DOC " & " STRING_H;
    
    String string_code = file_dump("internal_4coder_string.cpp");
    Cpp_Token_Stack string_tokens = {0};
    
    int32_t string_function_count = 0;
    
    {
        String *code = &string_code;
        Cpp_Token_Stack *token_stack = &string_tokens;
        
        char *data = code->str;
        int32_t size = code->size;
        
        *token_stack = cpp_make_token_stack(1024);
        cpp_lex_file(data, size, token_stack);
        
        
        int32_t count = token_stack->count;
        Cpp_Token *tokens = token_stack->tokens;
        Cpp_Token *token = tokens;
        
        for (int32_t i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                
                String_Function_Marker marker =
                    do_string_function_marker_check(lexeme);
                
                if (marker.parse_function){
                    if (do_function_parse_check(&i, &token, count)){
                        ++string_function_count;
                    }
                }
                else if (marker.parse_doc){
                    if (do_macro_parse_check(&i, &token, count)){
                        ++string_function_count;
                    }
                }
            }
        }
    }
    
    Function_Set string_function_set = allocate_function_set(string_function_count);
    int32_t string_sig_count = 0;
    
    {
        String *code = &string_code;
        Cpp_Token_Stack *token_stack = &string_tokens;
        
        char *data = code->str;
        
        int32_t count = token_stack->count;
        Cpp_Token *tokens = token_stack->tokens;
        Cpp_Token *token = tokens;
        
        String cpp_name = {0};
        int32_t has_cpp_name = 0;
        
        for (int32_t i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                
                String_Function_Marker marker =
                    do_string_function_marker_check(lexeme);
                
                if (marker.cpp_name){
                    if (do_parse_cpp_name(&i, &token, count, data, &cpp_name)){
                        has_cpp_name = 1;
                    }
                }
                else if (marker.parse_function){
                    if (do_full_function_parse(&i, &token, count, data,
                                               string_function_set, string_sig_count,
                                               cpp_name)){
                        ++string_sig_count;
                    }
                }
                else if (marker.parse_doc){
                    if (do_macro_parse_check(&i, &token, count)){
                        do_macro_parse(&i, &token, count, data,
                                       string_function_set, string_sig_count);
                        ++string_sig_count;
                    }
                }
                
                if (has_cpp_name){
                    has_cpp_name = 0;
                }
                else{
                    cpp_name = string_zero();
                }
            }
        }
    }
    
    //
    // App API parsing
    //
    
    String code_data[2];
    code_data[0] = file_dump("4ed_api_implementation.cpp");
    code_data[1] = file_dump("win32_api_impl.cpp");
    Parse parses[2];
    
    int32_t line_count = 0;
    
    for (int32_t J = 0; J < 2; ++J){
        String *code = &code_data[J];
        Parse *parse = &parses[J];
        
        char *data = code->str;
        int32_t size = code->size;
        
        parse->tokens = cpp_make_token_stack(512);
        cpp_lex_file(data, size, &parse->tokens);
        
        int32_t count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        Cpp_Token *token = tokens;
        
        for (int32_t i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                if (match_ss(lexeme, make_lit_string("API_EXPORT"))){
                    if (do_function_parse_check(&i, &token, count)){
                        ++line_count;
                    }
                }
            }
        }
    }
    
    Function_Set function_set = allocate_function_set(line_count);
    App_API app_function_set = allocate_app_api(line_count);
    int32_t sig_count = 0;
    
    for (int32_t J = 0; J < 2; ++J){
        String *code = &code_data[J];
        Parse *parse = &parses[J];
        
        char *data = code->str;
        
        int32_t count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        
        // NOTE(allen): Header Parse
        Cpp_Token *token = tokens;
        for (int32_t i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                if (match_ss(lexeme, make_lit_string("API_EXPORT"))){
                    do_full_function_parse(&i, &token, count, data, function_set,
                                           sig_count, string_zero());
                    if (!function_set.valid[sig_count]){
                        zero_index(function_set, sig_count);
                        // TODO(allen): get warning file name and line numbers
                        fprintf(stderr, "generator warning : invalid function signature\n");
                    }
                    ++sig_count;
                }
            }
        }
    }
    
    for (int32_t i = 0; i < sig_count; ++i){
        String name_string = function_set.name[i];
        String *macro = app_function_set.macros + i;
        String *public_name = app_function_set.public_name + i;
        
        macro->size = 0;
        macro->memory_size = name_string.size+4;
        
        macro->str = (char*)malloc(macro->memory_size);
        copy_ss(macro, name_string);
        to_upper(macro);
        append_ss(macro, make_lit_string("_SIG"));
        
        
        public_name->size = 0;
        public_name->memory_size = name_string.size;
        
        public_name->str = (char*)malloc(public_name->memory_size);
        copy_ss(public_name, name_string);
        to_lower(public_name);
    }
    
    // NOTE(allen): Header
    FILE *file = fopen(API_H, "wb");
    
    for (int32_t i = 0; i < sig_count; ++i){
        String ret_string   = function_set.ret[i];
        String args_string  = function_set.args[i];
        String macro_string = app_function_set.macros[i];
        
        fprintf(file, "#define %.*s(n) %.*s n%.*s\n",
                macro_string.size, macro_string.str,
                ret_string.size, ret_string.str,
                args_string.size, args_string.str
                );
    }
    
    fprintf(file, "extern \"C\"{\n");
    for (int32_t i = 0; i < sig_count; ++i){
        String name_string  = function_set.name[i];
        String macro_string = app_function_set.macros[i];
        
        fprintf(file, "    typedef %.*s(%.*s_Function);\n",
                macro_string.size, macro_string.str,
                name_string.size, name_string.str);
    }
    fprintf(file, "}\n");
    
    fprintf(file, "struct Application_Links{\n");
    fprintf(file,
            "    void *memory;\n"
            "    int32_t memory_size;\n"
            );
    for (int32_t i = 0; i < sig_count; ++i){
        String name_string  = function_set.name[i];
        String public_string = app_function_set.public_name[i];
        
        fprintf(file, "    %.*s_Function *%.*s;\n",
                name_string.size, name_string.str,
                public_string.size, public_string.str);
    }
    fprintf(file,
            "    void *cmd_context;\n"
            "    void *system_links;\n"
            "    void *current_coroutine;\n"
            "    int32_t type_coroutine;\n"
            );
    fprintf(file, "};\n");
    
    fprintf(file, "#define FillAppLinksAPI(app_links) do{");
    for (int32_t i = 0; i < sig_count; ++i){
        String name = function_set.name[i];
        String public_string = app_function_set.public_name[i];
        
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
    {
        Typedef_Set typedef_set = {0};
        Struct_Set struct_set = {0};
        Enum_Set flag_set = {0};
        Enum_Set enum_set = {0};
        
        String type_code[1];
        type_code[0] = file_dump("4coder_os_types.h");
        
        Cpp_Token_Stack types_token_array[1];
        
        int32_t typedef_count = 0;
        int32_t struct_count = 0;
        int32_t flag_count = 0;
        int32_t enum_count = 0;
        
        static String type_spec_keys[] = {
            make_lit_string("typedef"),
            make_lit_string("struct"),
            make_lit_string("union"),
            make_lit_string("ENUM"),
            make_lit_string("FLAGENUM"),
        };
        
        for (int32_t J = 0; J < 1; ++J){
            char *data = type_code[J].str;
            int32_t size = type_code[J].size;
            
            Cpp_Token_Stack types_tokens = cpp_make_token_stack(512);
            cpp_lex_file(data, size, &types_tokens);
            types_token_array[J] = types_tokens;
            
            int32_t count = types_tokens.count;
            Cpp_Token *tokens = types_tokens.tokens;
            Cpp_Token *token = tokens;
            
            for (int32_t i = 0; i < count; ++i, ++token){
                if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                    (token->type == CPP_TOKEN_KEY_TYPE_DECLARATION ||
                     token->type == CPP_TOKEN_IDENTIFIER)){
                    
                    String lexeme = make_string(data + token->start, token->size);
                    int32_t match_index = 0;
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
        
        int32_t typedef_index = 0;
        int32_t struct_index = 0;
        int32_t flag_index = 0;
        int32_t enum_index = 0;
        
        for (int32_t J = 0; J < 1; ++J){
            char *data = type_code[J].str;
            
            Cpp_Token_Stack types_tokens = types_token_array[J];
            
            int32_t count = types_tokens.count;
            Cpp_Token *tokens = types_tokens.tokens;
            Cpp_Token *token = tokens;
            
            for (int32_t i = 0; i < count; ++i, ++token){
                Assert(i == (i32)(token - tokens));
                if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                    (token->type == CPP_TOKEN_KEY_TYPE_DECLARATION ||
                     token->type == CPP_TOKEN_IDENTIFIER)){
                    
                    String lexeme = make_string(data + token->start, token->size);
                    int32_t match_index = 0;
                    if (string_set_match(type_spec_keys, ArrayCount(type_spec_keys),
                                         lexeme, &match_index)){
                        switch (match_index){
                            case 0: //typedef
                            {
                                String doc_string = {0};
                                get_type_doc_string(data, tokens, i, &doc_string);
                                
                                int32_t start_i = i;
                                Cpp_Token *start_token = token;
                                
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_SEMICOLON){
                                        break;
                                    }
                                }
                                
                                if (i < count){
                                    Cpp_Token *token_j = token;
                                    
                                    for (int32_t j = i; j > start_i; --j, --token_j){
                                        if (token_j->type == CPP_TOKEN_IDENTIFIER){
                                            break;
                                        }
                                    }
                                    
                                    String name = make_string(data + token_j->start, token_j->size);
                                    name = skip_chop_whitespace(name);
                                    
                                    int32_t type_start = start_token->start + start_token->size;
                                    int32_t type_end = token_j->start;
                                    String type = make_string(data + type_start, type_end - type_start);
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
                                                 data, tokens, count, &token,
                                                 struct_set.structs + struct_index)){
                                    ++struct_index;
                                }
                                i = (int32_t)(token - tokens);
                            }break;
                            
                            case 3: //ENUM
                            {
                                String doc_string = {0};
                                get_type_doc_string(data, tokens, i, &doc_string);
                                
                                int32_t start_i = i;
                                
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                        break;
                                    }
                                }
                                
                                if (parse_enum(part, data,
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
                                get_type_doc_string(data, tokens, i, &doc_string);
                                
                                int32_t start_i = i;
                                
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                        break;
                                    }
                                }
                                
                                if (parse_enum(part, data,
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
            
        //
        // Output 4coder_string.h
        //
        
        file = fopen(STRING_H, "wb");
        
        {
            String *code = &string_code;
            Cpp_Token_Stack *token_stack = &string_tokens;
            
            int32_t start = 0;
            
            int32_t count = token_stack->count;
            Cpp_Token *tokens = token_stack->tokens;
            Cpp_Token *token = tokens;
            int32_t i = 0;
            
            for (i = 0; i < count; ++i, ++token){
                if (token->type == CPP_TOKEN_IDENTIFIER &&
                    !(token->flags & CPP_TFLAG_PP_BODY)){
                    String lexeme = make_string(code->str + token->start, token->size);
                    if (match_ss(lexeme, make_lit_string("FSTRING_BEGIN"))){
                        start = token->start + token->size;
                        break;
                    }
                }
            }
            
            String pstr = {0};
            int32_t do_whitespace_print = true;
            
            for(++i, ++token; i < count; ++i, ++token){
                if (do_whitespace_print){
                    pstr = get_string(code->str, start, token->start);
                    print_str(file, pstr);
                }
                else{
                    do_whitespace_print = true;
                }
                
                String lexeme = get_lexeme(*token, code->str);
                
                int32_t do_print = true;
                if (match_ss(lexeme, make_lit_string("FSTRING_DECLS"))){
                    fprintf(file, "#if !defined(FCODER_STRING_H)\n#define FCODER_STRING_H\n\n");
                    
                    do_print = false;
                    
#define RETURN_PADDING 16
#define SIG_PADDING 30
                    
                    for (int32_t j = 0; j < string_sig_count; ++j){
                        char line_space[2048];
                        String line = make_fixed_width_string(line_space);
                        
                        if (!string_function_set.is_macro[j]){
                            String marker = string_function_set.marker[j];
                            String ret = string_function_set.ret[j];
                            String name = string_function_set.name[j];
                            String args = string_function_set.args[j];
                            
                            append_ss(&line, marker);
                            append_padding(&line, ' ', RETURN_PADDING);
                            append_ss(&line, ret);
                            append_padding(&line, ' ', SIG_PADDING);
                            append_ss(&line, name);
                            append_ss(&line, args);
                            terminate_with_null(&line);
                            
                            fprintf(file, "%s;\n", line.str);
                        }
                        else{
                            String name = string_function_set.name[j];
                            String args = string_function_set.args[j];
                            String body = string_function_set.body[j];
                            
                            append_ss(&line, make_lit_string("#ifndef "));
                            append_padding(&line, ' ', 10);
                            append_ss(&line, name);
                            terminate_with_null(&line);
                            fprintf(file, "%s\n", line.str);
                            line.size = 0;
                            
                            append_ss(&line, make_lit_string("# define "));
                            append_padding(&line, ' ', 10);
                            append_ss(&line, name);
                            append_ss(&line, args);
                            append_s_char(&line, ' ');
                            append_ss(&line, body);
                            terminate_with_null(&line);
                            fprintf(file, "%s\n", line.str);
                            line.size = 0;
                            
                            append_ss(&line, make_lit_string("#endif"));
                            terminate_with_null(&line);
                            fprintf(file, "%s\n", line.str);
                        }
                    }
                    
                    {
                        fprintf(file, "\n#if !defined(FSTRING_C)\n\n"
                                "// NOTE(allen): This section is here to enable nicer names\n"
                                "// for C++ users who can have overloaded functions.  None of\n"
                                "// these functions add new features.\n");
                        
                        for (int32_t j = 0; j < string_sig_count; ++j){
                            char line_space[2048];
                            String line = make_fixed_width_string(line_space);
                            
                            if (!string_function_set.is_macro[j]){
                                String cpp_name = string_function_set.cpp_name[j];
                                if (cpp_name.str != 0){
                                    String ret = string_function_set.ret[j];
                                    String args = string_function_set.args[j];
                                    
                                    append_ss(&line, make_lit_string("FSTRING_INLINE"));
                                    append_padding(&line, ' ', RETURN_PADDING);
                                    append_ss(&line, ret);
                                    append_padding(&line, ' ', SIG_PADDING);
                                    append_ss(&line, cpp_name);
                                    append_ss(&line, args);
                                    terminate_with_null(&line);
                                    
                                    fprintf(file, "%s;\n", line.str);
                                }
                            }
                        }
                        
                        fprintf(file, "\n#endif\n");
                    }
                    
                    fprintf(file, "\n#endif\n");
                    
                    {
                        fprintf(file, "\n#if !defined(FSTRING_C) && !defined(FSTRING_GUARD)\n\n");
                        
                        for (int32_t j = 0; j < string_sig_count; ++j){
                            char line_space[2048];
                            String line = make_fixed_width_string(line_space);
                            
                            if (!string_function_set.is_macro[j]){
                                String cpp_name = string_function_set.cpp_name[j];
                                if (cpp_name.str != 0){
                                    String name = string_function_set.name[j];
                                    String ret = string_function_set.ret[j];
                                    String args = string_function_set.args[j];
                                    Argument_Breakdown breakdown = string_function_set.breakdown[j];
                                    
                                    append_ss(&line, make_lit_string("FSTRING_INLINE"));
                                    append_s_char(&line, ' ');
                                    append_ss(&line, ret);
                                    append_s_char(&line, '\n');
                                    append_ss(&line, cpp_name);
                                    append_ss(&line, args);
                                    if (match_ss(ret, make_lit_string("void"))){
                                        append_ss(&line, make_lit_string("{("));
                                    }
                                    else{
                                        append_ss(&line, make_lit_string("{return("));
                                    }
                                    append_ss(&line, name);
                                    append_s_char(&line, '(');
                                    
                                    if (breakdown.count > 0){
                                        for (int32_t i = 0; i < breakdown.count; ++i){
                                            if (i != 0){
                                                append_s_char(&line, ',');
                                            }
                                            append_ss(&line, breakdown.param_name[i]);
                                        }
                                    }
                                    else{
                                        append_ss(&line, make_lit_string("void"));
                                    }
                                    
                                    append_ss(&line, make_lit_string("));}"));
                                    terminate_with_null(&line);
                                    
                                    fprintf(file, "%s\n", line.str);
                                }
                            }
                        }
                        
                        fprintf(file, "\n#endif\n");
                    }
                }
                else if (match_ss(lexeme, make_lit_string("DOC_EXPORT"))){
                    ++i, ++token;
                    if (i < count && token->type == CPP_TOKEN_COMMENT){
                        ++i, ++token;
                        if (i < count && token->type == CPP_PP_DEFINE){
                            ++i, ++token;
                            for (; i < count; ++i, ++token){
                                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                    break;
                                }
                            }
                            --i, --token;
                            do_print = false;
                            do_whitespace_print = false;
                        }
                    }
                }
                else if (match_ss(lexeme, make_lit_string("FSTRING_INLINE"))){
                    if (!(token->flags & CPP_TFLAG_PP_BODY)){
                        fprintf(file, "#if !defined(FSTRING_GUARD)\n");
                        
                        print_function_body_code(file, &i, &token, count, code, start);
                        
                        fprintf(file, "\n#endif");
                        do_print = false;
                    }
                }
                else if (match_ss(lexeme, make_lit_string("FSTRING_LINK"))){
                    if (!(token->flags & CPP_TFLAG_PP_BODY)){
                        fprintf(file, "#if defined(FSTRING_IMPLEMENTATION)\n");
                        
                        print_function_body_code(file, &i, &token, count, code, start);
                        
                        fprintf(file, "\n#endif");
                        do_print = false;
                    }
                }
                else if (match_ss(lexeme, make_lit_string("CPP_NAME"))){
                    
                    Cpp_Token *token_start = token;
                    int32_t i_start = i;
                    int32_t has_cpp_name = false;
                    
                    ++i, ++token;
                    if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                        ++i, ++token;
                        if (token->type == CPP_TOKEN_IDENTIFIER){
                            ++i, ++token;
                            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                has_cpp_name = true;
                            }
                        }
                    }
                    
                    if (!has_cpp_name){
                        i = i_start;
                        token = token_start;
                    }
                    
                    do_print = false;
                }
                else if (token->type == CPP_TOKEN_COMMENT){
                    lexeme = make_string(code->str + token->start, token->size);
                    if (check_and_fix_docs(&lexeme)){
                        do_print = false;
                    }
                }
                
                if (i < count){
                    if (do_print){
                        pstr = make_string(code->str + token->start, token->size);
                        print_str(file, pstr);
                    }
                    
                    start = token->start + token->size;
                }
            }
            pstr = get_string(code->str, start, code->size);
            print_str(file, pstr);
        }
        
        fclose(file);
        
        //
        // Output Docs
        //
        
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
                
                "h2 { "
                "margin-top: 6mm; "
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
                
                "</style>\n"
                "</head>\n"
                "<body>\n"
                "<div style='font-family:Arial; margin: 0 auto; "
                "width: 800px; text-align: justify; line-height: 1.25;'>\n"
                "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4coder API</h1>\n"
                );
        
        struct Section{
            char *id_string;
            char *display_string;
        };
        
        static Section sections[] = {
            {"introduction", "Introduction"},
            {"4coder_systems", "4coder Systems"},
            {"types_and_functions", "Types and Functions"},
            {"string_library", "String Library"}
        };
        
        fprintf(file,
                "<h3 style='margin:0;'>Table of Contents</h3>\n"
                "<ul>\n");
        
        int32_t section_count = ArrayCount(sections);
        for (int32_t i = 0; i < section_count; ++i){
            fprintf(file,
                    "<li><a href='#section_%s'>&sect;%d %s</a></li>",
                    sections[i].id_string,
                    i+1,
                    sections[i].display_string);
        }
        
        fprintf(file,
                "</ul>\n"
                );
        
#define MAJOR_SECTION "1"
        
        fprintf(file,
                "<h2 id='section_%s'>&sect;"MAJOR_SECTION" %s</h2>\n"
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
                
                "</div>\n",
                sections[0].id_string,
                sections[0].display_string
                );
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "2"
        // TODO(allen): Write the 4coder system descriptions.
        fprintf(file,
                "<h2 id='section_%s'>&sect;"MAJOR_SECTION" %s</h2>\n",
                sections[1].id_string,
                sections[1].display_string);
        
        {
            fprintf(file,
                "<div><i>\n"
                "Coming Soon"
                "</i><div>\n");
        }
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "3"
        
        fprintf(file, 
                "<h2 id='section_%s'>&sect;"MAJOR_SECTION" %s</h2>\n",
                sections[2].id_string,
                sections[2].display_string);
        {
#undef SECTION
#define SECTION MAJOR_SECTION".1"
            
            fprintf(file,
                    "<h3>&sect;"SECTION" Function List</h3>\n"
                    "<ul>\n");
            
            for (int32_t i = 0; i < sig_count; ++i){
                String name = app_function_set.public_name[i];
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
            
            for (int32_t i = 0; i < typedef_count; ++i){
                String name = typedef_set.name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            for (int32_t i = 0; i < enum_count; ++i){
                String name = enum_set.name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            for (int32_t i = 0; i < flag_count; ++i){
                String name = flag_set.name[i];
                fprintf(file,
                        "<li>"
                        "<a href='#%.*s_doc'>%.*s</a>"
                        "</li>\n",
                        name.size, name.str,
                        name.size, name.str
                        );
            }
            
            for (int32_t i = 0; i < struct_count; ++i){
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
            for (int32_t i = 0; i < sig_count; ++i){
                String name = app_function_set.public_name[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, i+1,
                        name.size, name.str
                        );
                print_function_html(file, function_set, i, name, "app->");
                fprintf(file, "</div>\n");
                
                String doc_string = function_set.doc_string[i];
                print_function_docs(file, part, name, doc_string);
                
                fprintf(file, "</div><hr>\n");
            }
            
#undef SECTION
#define SECTION MAJOR_SECTION".4"
            
            fprintf(file, "<h3>&sect;"SECTION" Type Descriptions</h3>\n");
            int32_t I = 1;
            for (int32_t i = 0; i < typedef_count; ++i, ++I){
                String name = typedef_set.name[i];
                String type = typedef_set.type[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
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
            
            for (int32_t i = 0; i < enum_count; ++i, ++I){
                String name = enum_set.name[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
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
            
            for (int32_t i = 0; i < flag_count; ++i, ++I){
                String name = flag_set.name[i];
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
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
            
            for (int32_t i = 0; i < struct_count; ++i, ++I){
                Struct_Member *member = &struct_set.structs[i];
                String name = member->name;
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
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
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "4"
        
        fprintf(file, 
                "<h2 id='section_%s'>&sect;"MAJOR_SECTION" %s</h2>\n",
                sections[3].id_string,
                sections[3].display_string);
        
        {
            
#undef SECTION
#define SECTION MAJOR_SECTION".1"
            
            fprintf(file,
                    "<h3>&sect;"SECTION" String Intro</h3>\n"
                    "<ul>\n");
            
            {
                fprintf(file,
                        "<div><i>\n"
                        "Coming Soon"
                        "</i><div>\n");
            }
            
#undef SECTION
#define SECTION MAJOR_SECTION".2"
            
            fprintf(file,
                    "<h3>&sect;"SECTION" String Function List</h3>\n"
                    "<ul>\n");
            
            String *used_strings = 0;
            int32_t used_string_count = 0;
            
            {
                int32_t memory_size = sizeof(String)*(string_sig_count);
                used_strings = (String*)malloc(memory_size);
                memset(used_strings, 0, memory_size);
            }
            
            for (int32_t i = 0; i < string_sig_count; ++i){
                String name = string_function_set.name[i];
                int32_t index = 0;
                if (!string_set_match(used_strings, used_string_count, name, &index)){
                    fprintf(file,
                            "<li>"
                            "<a href='#%.*s_str_doc'>%.*s</a>"
                            "</li>\n",
                            name.size, name.str,
                            name.size, name.str
                            );
                    used_strings[used_string_count++] = name;
                }
            }
            fprintf(file, "</ul>\n");
            
            used_string_count = 0;
            
#undef SECTION
#define SECTION MAJOR_SECTION".3"
            
            fprintf(file,
                    "<h3>&sect;"SECTION" String Function Descriptions</h3>\n"
                    "<ul>\n");
            
            for (int32_t i = 0; i < string_sig_count; ++i){
                String name = string_function_set.name[i];
                int32_t index = 0;
                int32_t do_id = false;
                if (!string_set_match(used_strings, used_string_count, name, &index)){
                    do_id = true;
                    used_strings[used_string_count++] = name;
                }
                
                if (do_id){
                    fprintf(file,
                            "<div id='%.*s_str_doc'>",
                            name.size, name.str
                            );
                }
                else{
                    fprintf(file, "<div>");
                }
                
                fprintf(file,
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>\n",
                        i+1, name.size, name.str);
                
                if (string_function_set.is_macro[i]){
                    print_macro_html(file, string_function_set, i, name);
                }
                else{
                    print_function_html(file, string_function_set, i, name, "");
                }
                
                fprintf(file, "</div>\n");
                
                
                String doc_string = string_function_set.doc_string[i];
                print_function_docs(file, part, name, doc_string);
                
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
    
    filename = generate_keycode_enum();
    filename = generate_style();
    filename = generate_custom_headers();
}

// BOTTOM

