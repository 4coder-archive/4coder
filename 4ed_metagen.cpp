/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#include "4coder_version.h"

#include "internal_4coder_string.cpp"

#include "4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "4coder_mem.h"

#define InvalidPath Assert(!"Invalid path of execution")

typedef struct Out_Context{
    FILE *file;
    String *str;
} Out_Context;

static String
str_start_end(char *data, int32_t start, int32_t end){
    return(make_string(data + start, end - start));
}

static String
str_alloc(Partition *part, int32_t cap){
        return(make_string_cap(push_array(part, char, cap), 0, cap));
}

static int32_t
begin_file_out(Out_Context *out_context, char *filename, String *out){
    int32_t r = 0;
    out_context->file = fopen(filename, "wb");
    out_context->str = out;
    out->size = 0;
    if (out_context->file){
        r = 1;
    }
    return(r);
}

static void
dump_file_out(Out_Context out_context){
    fwrite(out_context.str->str, 1, out_context.str->size, out_context.file);
    out_context.str->size = 0;
}

static void
end_file_out(Out_Context out_context){
    dump_file_out(out_context);
    fclose(out_context.file);
}

static String
make_out_string(int32_t x){
    String str;
    str.size = 0;
    str.memory_size = x;
    str.str = (char*)malloc(x);
    return(str);
}

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

void
generate_keycode_enum(){
    char *filename_keycodes = "4coder_keycodes.h";
    Out_Context context = {0};
    int32_t i = 0, count = 0;
    unsigned char code = 1;
    
    String out = make_out_string(10 << 20);
    
    if (begin_file_out(&context, filename_keycodes, &out)){
        count = ArrayCount(keys_that_need_codes);
        
        append_sc(&out, "enum{\n");
        for (i = 0; i < count; i){
            if (strcmp(keys_that_need_codes[i], "f1") == 0 && code < 0x7F){
                code = 0x7F;
            }
            switch (code){
                case '\n': code++; break;
                case '\t': code++; break;
                case 0x20: code = 0x7F; break;
                default:
                append_sc(&out, "key_");
                append_sc(&out, keys_that_need_codes[i++]);
                append_sc(&out, " = ");
                append_int_to_str(&out, code++);
                append_sc(&out, ",\n");
                break;
            }
        }
        append_sc(&out, "};\n");
        
        append_sc(&out,
                  "static char*\n"
                  "global_key_name(int32_t key_code, int32_t *size){\n"
                  "char *result = 0;\n"
                  "switch(key_code){\n");
        
        for (i = 0; i < count; ++i){
            append_sc(&out, "case key_");
            append_sc(&out, keys_that_need_codes[i]);
            append_sc(&out, ": result = \"");
            append_sc(&out, keys_that_need_codes[i]);
            append_sc(&out, "\"; *size = sizeof(\"");
            append_sc(&out, keys_that_need_codes[i]);
            append_sc(&out, "\")-1; break;\n");
        }
        
        append_sc(&out,
                  "}\n"
                  "return(result);\n"
                  "}\n");
        
        end_file_out(context);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static void
struct_begin(String *str, char *name){
    append_sc(str, "struct ");
    append_sc(str, name);
    append_sc(str, "{\n");
}

static void
enum_begin(String *str, char *name){
    append_sc(str, "enum ");
    append_sc(str, name);
    append_sc(str, "{\n");
}

static void
struct_end(String *str){
    append_sc(str, "};\n\n");
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
    to_camel_cc(tag, str);
    str[len] = 0;
    
    return(str);
}

static void
generate_style(){
    char filename_4coder[] = "4coder_style.h";
    char filename_4ed[] = "4ed_style.h";
    char *tag = 0;
    int32_t count = 0, i = 0;
    
    String out = make_out_string(10 << 20);
    Out_Context context = {0};
    
    if (begin_file_out(&context, filename_4coder, &out)){
        
        enum_begin(&out, "Style_Tag");
        {
            count = ArrayCount(bar_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(bar_style_fields[i]);
                append_sc(&out, "Stag_");
                append_sc(&out, tag);
                append_sc(&out, ",\n");
                free(tag);
            }
            
            count = ArrayCount(main_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(main_style_fields[i]);
                append_sc(&out, "Stag_");
                append_sc(&out, tag);
                append_sc(&out, ",\n");
                free(tag);
            }
        }
        struct_end(&out);
        
        end_file_out(context);
    }
    
    if (begin_file_out(&context, filename_4ed, &out)){
        
        struct_begin(&out, "Interactive_Style");
        {
            count = ArrayCount(bar_style_fields);
            for (i = 0; i < count; ++i){
                append_sc(&out, "u32 ");
                append_sc(&out, bar_style_fields[i]);
                append_sc(&out, "_color;\n");
            }
        }
        struct_end(&out);
        
        struct_begin(&out, "Style_Main_Data");
        {
            count = ArrayCount(main_style_fields);
            for (i = 0; i < count; ++i){
                append_sc(&out, "u32 ");
                append_sc(&out, main_style_fields[i]);
                append_sc(&out, "_color;\n");
            }
            append_sc(&out, "Interactive_Style file_info_style;\n");
        }
        struct_end(&out);
        
        {
            append_sc(&out,
                      "inline u32*\n"
                      "style_index_by_tag(Style_Main_Data *s, u32 tag){\n"
                      "u32 *result = 0;\n"
                      "switch (tag){\n");
            
            count = ArrayCount(bar_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(bar_style_fields[i]);
                append_sc(&out, "case Stag_");
                append_sc(&out, tag);
                append_sc(&out, ": result = &s->file_info_style.");
                append_sc(&out, bar_style_fields[i]);
                append_sc(&out, "_color; break;\n");
                free(tag);
            }
            
            count = ArrayCount(main_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(main_style_fields[i]);
                append_sc(&out, "case Stag_");
                append_sc(&out, tag);
                append_sc(&out, ": result = &s->");
                append_sc(&out, main_style_fields[i]);
                append_sc(&out, "_color; break;\n");
                free(tag);
            }
            
            append_sc(&out,
                      "}\n"
                      "return(result);\n"
                      "}\n\n");
        }
        
        end_file_out(context);
    }
    
    free(out.str);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Parse_Context{
    Cpp_Token *token_s;
    Cpp_Token *token_e;
    Cpp_Token *token;
    char *data;
} Parse_Context;

typedef struct Argument{
    String param_string;
    String param_name;
} Argument;

typedef struct Argument_Breakdown{
    int32_t count;
    Argument *args;
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

typedef enum Item_Type{
    Item_Null,
    Item_Function,
    Item_CppName,
    Item_Macro,
    Item_Typedef,
    Item_Struct,
    Item_Union,
    Item_Enum,
    Item_Type_Count,
#define Item_Type_User0 Item_Type_Count
} Item_Type;

typedef struct Item_Node{
    int32_t t;
    
    String cpp_name;
    String name;
    String ret;
    String args;
    String body;
    String marker;
    
    String value;
    String type;
    String type_postfix;
    String doc_string;
    
    Argument_Breakdown breakdown;
    Documentation doc;
    
    Item_Node *first_child;
    Item_Node *next_sibling;
} Item_Node;

typedef struct Item_Set{
    Item_Node *items;
    int32_t count;
} Item_Set;

typedef struct Parse{
    String code;
    Cpp_Token_Array tokens;
    int32_t item_count;
} Parse;

typedef struct Meta_Unit{
    Item_Set set;
    
    Parse *parse;
    int32_t count;
} Meta_Unit;

typedef struct Meta_Keywords{
    String key;
    Item_Type type;
} Meta_Keywords;

typedef struct Used_Links{
    String *strs;
    int32_t count, max;
} Used_Links;

static Item_Node null_item_node = {0};

static String
get_lexeme(Cpp_Token token, char *code){
    String str = make_string(code + token.start, token.size);
    return(str);
}

static Parse_Context
setup_parse_context(char *data, Cpp_Token_Array array){
    Parse_Context context;
    context.token_s = array.tokens;
    context.token_e = array.tokens + array.count;
    context.token = context.token_s;
    context.data = data;
    return(context);
}

static Parse_Context
setup_parse_context(Parse parse){
    Parse_Context context;
    context.token_s = parse.tokens.tokens;
    context.token_e = parse.tokens.tokens + parse.tokens.count;
    context.token = context.token_s;
    context.data = parse.code.str;
    return(context);
}

static Cpp_Token*
get_token(Parse_Context *context){
    Cpp_Token *result = context->token;
    if (result >= context->token_e){
        result = 0;
    }
    return(result);
}

static Cpp_Token*
get_next_token(Parse_Context *context){
    Cpp_Token *result = context->token+1;
    context->token = result;
    if (result >= context->token_e){
        result = 0;
        context->token = context->token_e;
    }
    return(result);
}

static Cpp_Token*
get_prev_token(Parse_Context *context){
    Cpp_Token *result = context->token-1;
    if (result < context->token_s){
        result = 0;
    }
    else{
        context->token = result;
    }
    return(result);
}

static Cpp_Token*
can_back_step(Parse_Context *context){
    Cpp_Token *result = context->token-1;
    if (result < context->token_s){
        result = 0;
    }
    return(result);
}

static Cpp_Token*
set_token(Parse_Context *context, Cpp_Token *token){
    Cpp_Token *result = 0;
    if (token >= context->token_s && token < context->token_e){
        context->token = token;
        result = token;
    }
    return(result);
}

static String
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

static Parse
meta_lex(char *filename){
    Parse result = {0};
    result.code = file_dump(filename);
    result.tokens = cpp_make_token_array(1024);
    cpp_lex_file(result.code.str, result.code.size, &result.tokens);
    return(result);
}

static String
get_first_line(String source){
    String line = {0};
    int32_t pos = find_s_char(source, 0, '\n');
    line = substr(source, 0, pos);
    return(line);
}

static String
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

typedef enum Doc_Note_Type{
    DOC_PARAM,
    DOC_RETURN,
    DOC,
    DOC_SEE,
    DOC_HIDE,
    HIDE_MEMBERS,
} Doc_Note_Type;

static String
doc_note_string[] = {
    make_lit_string("DOC_PARAM"),
    make_lit_string("DOC_RETURN"),
    make_lit_string("DOC"),
    make_lit_string("DOC_SEE"),
    make_lit_string("DOC_HIDE"),
    make_lit_string("HIDE_MEMBERS"),
};

static int32_t
check_and_fix_docs(String *doc_string){
    int32_t result = false;
    
    if (doc_string->size > 4){
        if (doc_string->str[0] == '/'){
            if (doc_string->str[1] == '*'){
                if (doc_string->str[doc_string->size - 2] == '*'){
                    if (doc_string->str[doc_string->size - 1] == '/'){
                        result = true;
                        doc_string->str += 2;
                        doc_string->size -= 4;
                    }
                }
            }
        }
    }
    
    return(result);
}

static int32_t
get_doc_string_from_prev(Parse_Context *context, String *doc_string){
    int32_t result = false;
    
    if (can_back_step(context)){
        Cpp_Token *prev_token = get_token(context) - 1;
        if (prev_token->type == CPP_TOKEN_COMMENT){
            *doc_string = get_lexeme(*prev_token, context->data);
            if (check_and_fix_docs(doc_string)){
                result = true;
            }
            else{
                *doc_string = null_string;
            }
        }
    }
    
    return(result);
}

static String
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

static String
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

static String
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

static String
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

static void
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
                    case DOC_PARAM: ++param_count; break;
                    case DOC_SEE: ++see_count; break;
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

static Item_Set
allocate_item_set(Partition *part, int32_t count){
    Item_Set item_set = {0};
    if (count > 0){
        item_set.items = push_array(part, Item_Node, count);
        item_set.count = count;
        memset(item_set.items, 0, sizeof(Item_Node)*count);
    }
    return(item_set);
}


//
// Meta Parse Rules
//

static int32_t
struct_parse(Partition *part, int32_t is_struct,
             Parse_Context *context, Item_Node *top_member);

static int32_t
struct_parse_member(Partition *part, Parse_Context *context, Item_Node *member){
    
    int32_t result = false;
    
    Cpp_Token *token = get_token(context);
    
    String doc_string = {0};
    get_doc_string_from_prev(context, &doc_string);
    
    Cpp_Token *start_token = token;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_SEMICOLON){
            break;
        }
    }
    
    if (token){
        String name = {0};
        Cpp_Token *token_j = 0;
        int32_t nest_level = 0;
        
        for (; (token_j = get_token(context)) > start_token; get_prev_token(context)){
            if (token_j->type == CPP_TOKEN_BRACKET_CLOSE){
                ++nest_level;
            }
            else if (token_j->type == CPP_TOKEN_BRACKET_OPEN){
                --nest_level;
                if (nest_level < 0){
                    break;
                }
            }
            
            if (nest_level == 0){
                if (token_j->type == CPP_TOKEN_IDENTIFIER){
                    break;
                }
            }
        }
        
        name = skip_chop_whitespace(get_lexeme(*token_j, context->data));
        
        String type = skip_chop_whitespace(str_start_end(context->data, start_token->start, token_j->start));
        
        String type_postfix =
            skip_chop_whitespace(str_start_end(context->data, token_j->start + token_j->size, token->start));
        
        set_token(context, token+1);
        result = true;
        
        member->name = name;
        member->type = type;
        member->type_postfix = type_postfix;
        member->doc_string = doc_string;
        member->first_child = 0;
        member->next_sibling = 0;
    }
    
    return(result);
}

static Item_Node*
struct_parse_next_member(Partition *part, Parse_Context *context){
    Item_Node *result = 0;
    
    Cpp_Token *token = 0;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_IDENTIFIER ||
            (token->flags & CPP_TFLAG_IS_KEYWORD)){
            String lexeme = get_lexeme(*token, context->data);
            
            if (match_ss(lexeme, make_lit_string("struct"))){
                Item_Node *member = push_struct(part, Item_Node);
                if (struct_parse(part, true, context, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else if (match_ss(lexeme, make_lit_string("union"))){
                Item_Node *member = push_struct(part, Item_Node);
                if (struct_parse(part, false, context, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else{
                Item_Node *member = push_struct(part, Item_Node);
                if (struct_parse_member(part, context, member)){
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
    
    return(result);
}

static int32_t
struct_parse(Partition *part, int32_t is_struct, Parse_Context *context, Item_Node *top_member){
    
    int32_t result = false;
    
    Cpp_Token *start_token = get_token(context);
    Cpp_Token *token = 0;
    
    String doc_string = {0};
    get_doc_string_from_prev(context, &doc_string);
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_BRACE_OPEN){
            break;
        }
    }
    
    if (token){
        Cpp_Token *token_j = token;
        
        for (; (token_j = get_token(context)) > start_token; get_prev_token(context)){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        String name = {0};
        if (token_j != start_token){
            name = skip_chop_whitespace(get_lexeme(*token_j, context->data));
        }
        
        String type = {0};
        if (is_struct){
            type = make_lit_string("struct");
        }
        else{
            type = make_lit_string("union");
        }
        
        set_token(context, token+1);
        Item_Node *new_member = struct_parse_next_member(part, context);
        
        if (new_member){
            top_member->first_child = new_member;
            
            Item_Node *head_member = new_member;
            for(;;){
                new_member = struct_parse_next_member(part, context);
                if (new_member){
                    head_member->next_sibling = new_member;
                    head_member = new_member;
                }
                else{
                    break;
                }
            }
        }
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_SEMICOLON){
                break;
            }
        }
        ++token;
        
        if (is_struct){
            top_member->t = Item_Struct;
        }
        else{
            top_member->t = Item_Union;
        }
        top_member->name = name;
        top_member->type = type;
        top_member->doc_string = doc_string;
        top_member->next_sibling = 0;
        
        result = true;
    }
    
    return(result);
}

static int32_t
typedef_parse(Parse_Context *context, Item_Node *item){
    int32_t result = false;
    
    Cpp_Token *token = get_token(context);
    String doc_string = {0};
    get_doc_string_from_prev(context, &doc_string);
    
    Cpp_Token *start_token = token;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_SEMICOLON){
            break;
        }
    }
    
    if (token){
        Cpp_Token *token_j = token;
        
        for (; (token_j = get_token(context)) > start_token; get_prev_token(context)){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        String name = get_lexeme(*token_j, context->data);
        
        String type = skip_chop_whitespace(
            str_start_end(context->data, start_token->start + start_token->size, token_j->start)
            );
        
        item->t = Item_Typedef;
        item->type = type;
        item->name = name;
        item->doc_string = doc_string;
        result = true;
    }
    
    set_token(context, token);
    
    return(result);
}

static int32_t
enum_parse(Partition *part, Parse_Context *context, Item_Node *item){
    
    int32_t result = false;
    
    String doc_string = {0};
    get_doc_string_from_prev(context, &doc_string);
    
    Cpp_Token *start_token = get_token(context);
    Cpp_Token *token = 0;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_BRACE_OPEN){
            break;
        }
    }
    
    if (token){
        String name = {0};
        Cpp_Token *token_j = 0;
        
        for (; (token_j = get_token(context)) != 0; get_prev_token(context)){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        name = get_lexeme(*token_j, context->data);
        
        set_token(context, token);
        for (; (token = get_token(context)) > start_token; get_next_token(context)){
            if (token->type == CPP_TOKEN_BRACE_OPEN){
                break;
            }
        }
        
        if (token){
            Item_Node *first_member = 0;
            Item_Node *head_member = 0;
            
            for (; (token = get_token(context)) != 0; get_next_token(context)){
                if (token->type == CPP_TOKEN_BRACE_CLOSE){
                    break;
                }
                else if (token->type == CPP_TOKEN_IDENTIFIER){
                    String doc_string = {0};
                    String name = {0};
                    String value = {0};
                    get_doc_string_from_prev(context, &doc_string);
                    
                    name = get_lexeme(*token, context->data);
                    
                    token = get_next_token(context);
                    
                    if (token){
                        if (token->type == CPP_TOKEN_EQ){
                            Cpp_Token *start_token = token;
                            
                            for (; (token = get_token(context)) != 0; get_next_token(context)){
                                if (token->type == CPP_TOKEN_COMMA ||
                                    token->type == CPP_TOKEN_BRACE_CLOSE){
                                    break;
                                }
                            }
                            
                            value = skip_chop_whitespace(
                                str_start_end(context->data, start_token->start + start_token->size, token->start)
                                );
                            
                            get_prev_token(context);
                        }
                        else{
                            get_prev_token(context);
                        }
                    }
                    
                    Item_Node *new_member = push_struct(part, Item_Node);
                    if (first_member == 0){
                        first_member = new_member;
                    }
                    
                    if (head_member){
                        head_member->next_sibling = new_member;
                    }
                    head_member = new_member;
                    
                    new_member->name = name;
                    new_member->value = value;
                    new_member->doc_string = doc_string;
                    new_member->next_sibling = 0;
                }
            }
            
            if ((token = get_token(context)) != 0){
                for (; (token = get_token(context)) != 0; get_next_token(context)){
                    if (token->type == CPP_TOKEN_BRACE_CLOSE){
                        break;
                    }
                }
                get_next_token(context);
                
                item->t = Item_Enum;
                item->name = name;
                item->doc_string = doc_string;
                item->first_child = first_member;
                result = true;
            }
        }
    }
    
    return(result);
}

static Argument_Breakdown
allocate_argument_breakdown(Partition *part, int32_t count){
    Argument_Breakdown breakdown = {0};
    if (count > 0){
        breakdown.count = count;
        breakdown.args = push_array(part, Argument, count);
        memset(breakdown.args, 0, sizeof(Argument)*count);
    }
    return(breakdown);
}

/*
Parse arguments by giving pointers to the tokens:
foo(a, ... , z)
   ^          ^
*/
static Argument_Breakdown
parameter_parse(Partition *part, char *data, Cpp_Token *args_start_token, Cpp_Token *args_end_token){
    int32_t arg_index = 0;
    Cpp_Token *arg_token = args_start_token + 1;
    int32_t param_string_start = arg_token->start;
    
    int32_t arg_count = 1;
    arg_token = args_start_token;
    for (; arg_token < args_end_token; ++arg_token){
        if (arg_token->type == CPP_TOKEN_COMMA){
            ++arg_count;
        }
    }
    
    Argument_Breakdown breakdown = allocate_argument_breakdown(part, arg_count);
    
    arg_token = args_start_token + 1;
    for (; arg_token <= args_end_token; ++arg_token){
        if (arg_token->type == CPP_TOKEN_COMMA ||
            arg_token->type == CPP_TOKEN_PARENTHESE_CLOSE){
            
            int32_t size = arg_token->start - param_string_start;
            String param_string = make_string(data + param_string_start, size);
            param_string = chop_whitespace(param_string);
            breakdown.args[arg_index].param_string = param_string;
            
            for (Cpp_Token *param_name_token = arg_token - 1;
                 param_name_token->start > param_string_start;
                 --param_name_token){
                if (param_name_token->type == CPP_TOKEN_IDENTIFIER){
                    int32_t start = param_name_token->start;
                    int32_t size = param_name_token->size;
                    breakdown.args[arg_index].param_name = make_string(data + start, size);
                    break;
                }
            }
            
            ++arg_index;
            
            if (arg_token+1 <= args_end_token){
                param_string_start = arg_token[1].start;
            }
        }
    }
    
    return(breakdown);
}

/*
Moves the context in the following way:
~~~~~~~ name( ~~~~~~~
 ^  ->  ^
*/
static int32_t
function_parse_goto_name(Parse_Context *context){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    
    {
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                break;
            }
        }
        
        if (get_token(context)){
            do{
                token = get_prev_token(context);
            }while(token->type == CPP_TOKEN_COMMENT);
            
            if (token->type == CPP_TOKEN_IDENTIFIER){
                result = true;
            }
        }
    }
    
    return(result);
}

/*
Moves the context in the following way:
~~~~~~~ name( ~~~~~~~ /* XXX //
 ^  --------------->  ^
*/
static int32_t
function_get_doc(Parse_Context *context, char *data, String *doc_string){
    int32_t result = false;
    
    Cpp_Token *token = get_token(context);
    String lexeme = {0};
    
    if (function_parse_goto_name(context)){
        if (token->type == CPP_TOKEN_IDENTIFIER){
            for (; (token = get_token(context)) != 0; get_next_token(context)){
                if (token->type == CPP_TOKEN_COMMENT){
                    lexeme = get_lexeme(*token, data);
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
        }
    }
    
    return(result);
}

static int32_t
cpp_name_parse(Parse_Context *context, String *name){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *token_start = get_token(context);
    
    token = get_next_token(context);
    if (token && token->type == CPP_TOKEN_PARENTHESE_OPEN){
        token = get_next_token(context);
        if (token && token->type == CPP_TOKEN_IDENTIFIER){
            token = get_next_token(context);
            if (token && token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                *name = get_lexeme(*(token-1), context->data);
                result = true;
            }
        }
    }
    
    if (!result){
        set_token(context, token_start);
    }
    
    return(result);
}

/*
Moves the context in the following way:
 RETTY~ name( ~~~~~~~ )
 ^  --------------->  ^
*/
static int32_t
function_sig_parse(Partition *part, Parse_Context *context, Item_Node *item, String cpp_name){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *args_start_token = 0;
    Cpp_Token *ret_token = get_token(context);
    
    if (function_parse_goto_name(context)){
        token = get_token(context);
        args_start_token = token+1;
        item->name = get_lexeme(*token, context->data);
        
        item->ret = chop_whitespace(
            str_start_end(context->data, ret_token->start, token->start)
            );
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                break;
            }
        }
        
        if (token){
            item->args =
                str_start_end(context->data, args_start_token->start, token->start + token->size);
            item->t = Item_Function;
            item->cpp_name = cpp_name;
            item->breakdown = parameter_parse(part, context->data, args_start_token, token);
            
            Assert(get_token(context)->type == CPP_TOKEN_PARENTHESE_CLOSE);
            result = true;
        }
    }
    
    return(result);
}

/*
Moves the context in the following way:
 MARKER ~~~ name( ~~~~~~~ )
 ^  ------------------->  ^
*/
static int32_t
function_parse(Partition *part, Parse_Context *context, Item_Node *item, String cpp_name){
    int32_t result = false;
    
    String doc_string = {0};
    Cpp_Token *token = get_token(context);
    
    item->marker = get_lexeme(*token, context->data);
    
    if (function_get_doc(context, context->data, &doc_string)){
        item->doc_string = doc_string;
    }
    
    set_token(context, token);
    if (get_next_token(context)){
        if (function_sig_parse(part, context, item, cpp_name)){
            Assert(get_token(context)->type == CPP_TOKEN_PARENTHESE_CLOSE);
            result = true;
        }
    }
    
    return(result);
}

/*
Moves the context in the following way:
 /* ~~~ // #define
 ^  ---->  ^
*/
static int32_t
macro_parse_check(Parse_Context *context){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    
    if ((token = get_next_token(context)) != 0){
        if (token->type == CPP_TOKEN_COMMENT){
            if ((token = get_next_token(context)) != 0){
                if (token->type == CPP_PP_DEFINE){
                    result = true;
                }
            }
        }
    }
    
    return(result);
}

/*
Moves the context in the following way:
 /* ~~~ // #define ~~~~~~~~~~~~~~~~~   NOT_IN_MACRO_BODY
 ^  ---------------------------->  ^
*/
static int32_t
macro_parse(Partition *part, Parse_Context *context, Item_Node *item){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *doc_token = 0;
    Cpp_Token *args_start_token = 0;
    
    String doc_string = {0};
    
    if (macro_parse_check(context)){
        token = get_token(context);
        if (can_back_step(context)){
            doc_token = token-1;
            
            doc_string = get_lexeme(*doc_token, context->data);
            
            if (check_and_fix_docs(&doc_string)){
                item->doc_string = doc_string;
                
                for (; (token = get_token(context)) != 0; get_next_token(context)){
                    if (token->type == CPP_TOKEN_IDENTIFIER){
                        break;
                    }
                }
                
                if (get_token(context) && (token->flags & CPP_TFLAG_PP_BODY)){
                    item->name = get_lexeme(*token, context->data);
                    
                    if ((token = get_next_token(context)) != 0){
                        args_start_token = token;
                        for (; (token = get_token(context)) != 0; get_next_token(context)){
                            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                break;
                            }
                        }
                        
                        if (token){
                            item->args = str_start_end(context->data, args_start_token->start,
                                                    token->start + token->size);
                            
                            item->breakdown = parameter_parse(part, context->data, args_start_token, token);
                            
                            if ((token = get_next_token(context)) != 0){
                                Cpp_Token *body_start = token;
                                
                                if (body_start->flags & CPP_TFLAG_PP_BODY){
                                    for (; (token = get_token(context)) != 0; get_next_token(context)){
                                        if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                            break;
                                        }
                                    }
                                    
                                    token = get_prev_token(context);
                                    
                                    item->body =
                                        str_start_end(context->data, body_start->start,
                                                   token->start + token->size);
                                }
                            }
                            
                            item->t = Item_Macro;
                            result = true;
                        }
                    }
                }
            }
        }
    }
    
    return(result);
}

static Meta_Unit
compile_meta_unit(Partition *part, char **files, int32_t file_count,
                  Meta_Keywords *keywords, int32_t key_count){
    Meta_Unit unit = {0};
    int32_t i = 0;
    
    unit.count = file_count;
    unit.parse = push_array(part, Parse, file_count);
    
    for (i = 0; i < file_count; ++i){
        unit.parse[i] = meta_lex(files[i]);
    }
    
    // TODO(allen): This stage counts nested structs
    // and unions which is not correct.  Luckily it only
    // means we over allocate by a few items, but fixing it
    // to be exactly correct would be nice.
    for (int32_t J = 0; J < unit.count; ++J){
        Cpp_Token *token = 0;
        Parse_Context context_ = setup_parse_context(unit.parse[J]);
        Parse_Context *context = &context_;
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                
                String lexeme = get_lexeme(*token, context->data);
                int32_t match_index = 0;
                if (string_set_match_table(keywords, sizeof(*keywords), key_count, lexeme, &match_index)){
                    Item_Type type = keywords[match_index].type;
                    
                    if (type > Item_Null && type < Item_Type_Count){
                        ++unit.set.count;
                    }
                    else{
                        // TODO(allen): Warning
                    }
                }
            }
        }
    }
    
    if (unit.set.count >  0){
        unit.set = allocate_item_set(part, unit.set.count);
    }
    
    int32_t index = 0;
    
    for (int32_t J = 0; J < unit.count; ++J){
        Cpp_Token *token = 0;
        Parse_Context context_ = setup_parse_context(unit.parse[J]);
        Parse_Context *context = &context_;
        
        String cpp_name = {0};
        int32_t has_cpp_name = 0;
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                
                String lexeme = get_lexeme(*token, context->data);
                int32_t match_index = 0;
                if (string_set_match_table(keywords, sizeof(*keywords), key_count, lexeme, &match_index)){
                    Item_Type type = keywords[match_index].type;
                    
                    switch (type){
                        case Item_Function:
                        {
                            if (function_parse(part, context, unit.set.items + index, cpp_name)){
                                Assert(unit.set.items[index].t == Item_Function);
                                ++index;
                            }
                            else{
                                fprintf(stderr, "warning: invalid function signature\n");
                            }
                        }break;
                        
                        case Item_CppName:
                        {
                            if (cpp_name_parse(context, &cpp_name)){
                                has_cpp_name = 1;
                            }
                            else{
                                // TODO(allen): warning message
                            }
                        }break;
                        
                        case Item_Macro:
                        {
                            if (macro_parse(part, context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Macro);
                                ++index;
                            }
                            else{
                                // TODO(allen): warning message
                            }
                        }break;
                        
                        case Item_Typedef: //typedef
                        {
                            if (typedef_parse(context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Typedef);
                                ++index;
                            }
                            else{
                                // TODO(allen): warning message
                            }
                        }break;
                        
                        case Item_Struct: case Item_Union: //struct/union
                        {
                            if (struct_parse(part, (type == Item_Struct), context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Struct ||
                                       unit.set.items[index].t == Item_Union);
                                ++index;
                            }
                            else{
                                // TODO(allen): warning message
                            }
                        }break;
                        
                        case Item_Enum: //ENUM
                        {
                            if (enum_parse(part, context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Enum);
                                ++index;
                            }
                            else{
                                // TODO(allen): warning message
                            }
                        }break;
                        
                    }
                }
            }
            
            if (has_cpp_name){
                has_cpp_name = 0;
            }
            else{
                cpp_name = null_string;
            }
            
            unit.parse[J].item_count = index;
        }
        
        // NOTE(allen): This is necessary for now because
        // the original count is slightly overestimated thanks
        // to nested structs and unions.
        unit.set.count = index;
    }
    
    return(unit);
}

static void
init_used_links(Partition *part, Used_Links *used, int32_t count){
    used->strs = push_array(part, String, count);
    used->count = 0;
    used->max = count;
}

static int32_t
try_to_use(Used_Links *used, String str){
    int32_t result = 1;
    int32_t index = 0;
    
    if (string_set_match(used->strs, used->count, str, &index)){
        result = 0;
    }
    else{
        used->strs[used->count++] = str;
    }
    
    return(result);
}

static void
print_struct_html(String *out, Item_Node *member, int32_t hide_children){
    String name = member->name;
    String type = member->type;
    String type_postfix = member->type_postfix;
    
    append_ss     (out, type);
    append_s_char (out, ' ');
    append_ss     (out, name);
    append_ss     (out, type_postfix);
    
    if (match_ss(type, make_lit_string("struct")) ||
        match_ss(type, make_lit_string("union"))){
        
        if (hide_children){
            append_sc(out, " { /* non-public internals */ } ;");
        }
        else{
            append_sc(out, " {<br><div style='margin-left: 8mm;'>");
            
            for (Item_Node *member_iter = member->first_child;
                 member_iter != 0;
                 member_iter = member_iter->next_sibling){
                print_struct_html(out, member_iter, hide_children);
            }
            
            append_sc(out, "</div>};<br>");
        }
    }
    else{
        append_sc(out, ";<br>");
    }
}

static void
print_function_html(String *out, Used_Links *used, String cpp_name,
                    String ret, char *function_call_head, String name, Argument_Breakdown breakdown){
    
    append_ss     (out, ret);
    append_s_char (out, ' ');
    append_sc     (out, function_call_head);
    append_ss     (out, name);
    append_sc     (out, "(<div style='margin-left: 4mm;'>");
    
    for (int32_t j = 0; j < breakdown.count; ++j){
        append_ss(out, breakdown.args[j].param_string);
        if (j < breakdown.count - 1){
            append_s_char(out, ',');
        }
        append_sc(out, "<br>");
    }
    
    append_sc(out, "</div>)");
}

static void
print_macro_html(String *out, String name, Argument_Breakdown breakdown){
    
    if (breakdown.count == 0){
        append_sc(out, "#define ");
        append_ss(out, name);
        append_sc(out, "()");
    }
    else if (breakdown.count == 1){
        append_sc      (out, "#define ");
        append_ss      (out, name);
        append_s_char  (out, '(');
        append_ss      (out, breakdown.args[0].param_string);
        append_s_char  (out, ')');
    }
    else{
        append_sc (out, "#define ");
        append_ss (out, name);
        append_sc (out, "(<div style='margin-left: 4mm;'>");
        
        for (int32_t j = 0; j < breakdown.count; ++j){
            append_ss(out, breakdown.args[j].param_string);
            if (j < breakdown.count - 1){
                append_s_char(out, ',');
            }
            append_sc(out, "<br>");
        }
        
        append_sc(out, ")</div>)");
    }
}

#define BACK_COLOR   "#FAFAFA"
#define TEXT_COLOR   "#0D0D0D"
#define CODE_BACK    "#DFDFDF"
#define EXAMPLE_BACK "#EFEFDF"

#define POP_COLOR_1  "#309030"
#define POP_BACK_1   "#E0FFD0"
#define VISITED_LINK "#A0C050"

#define POP_COLOR_2  "#005000"

#define CODE_STYLE "font-family: \"Courier New\", Courier, monospace; text-align: left;"

#define CODE_BLOCK_STYLE(back)                             \
"margin-top: 3mm; margin-bottom: 3mm; font-size: .95em; "  \
"background: "back"; padding: 0.25em;"

#define DESCRIPT_SECTION_STYLE CODE_BLOCK_STYLE(CODE_BACK)
#define EXAMPLE_CODE_STYLE CODE_BLOCK_STYLE(EXAMPLE_BACK)

#define DOC_HEAD_OPEN  "<div style='margin-top: 3mm; margin-bottom: 3mm; color: "POP_COLOR_1";'><b><i>"
#define DOC_HEAD_CLOSE "</i></b></div>"

#define DOC_ITEM_HEAD_STYLE "font-weight: 600;"

#define DOC_ITEM_HEAD_INL_OPEN  "<span style='"DOC_ITEM_HEAD_STYLE"'>"
#define DOC_ITEM_HEAD_INL_CLOSE "</span>"

#define DOC_ITEM_HEAD_OPEN  "<div style='"DOC_ITEM_HEAD_STYLE"'>"
#define DOC_ITEM_HEAD_CLOSE "</div>"

#define DOC_ITEM_OPEN  "<div style='margin-left: 5mm; margin-right: 5mm;'>"
#define DOC_ITEM_CLOSE "</div>"

#define EXAMPLE_CODE_OPEN  "<div style='"CODE_STYLE EXAMPLE_CODE_STYLE"'>"
#define EXAMPLE_CODE_CLOSE "</div>"

static String
get_first_double_line(String source){
    String line = {0};
    int32_t pos0 = find_substr_s(source, 0, make_lit_string("\n\n"));
    int32_t pos1 = find_substr_s(source, 0, make_lit_string("\r\n\r\n"));
    if (pos1 < pos0){
        pos0 = pos1;
    }
    line = substr(source, 0, pos0);
    return(line);
}

static String
get_next_double_line(String source, String line){
    String next = {0};
    int32_t pos = (int32_t)(line.str - source.str) + line.size;
    int32_t start = 0, pos0 = 0, pos1 = 0;
    
    if (pos < source.size){
        assert(source.str[pos] == '\n' || source.str[pos] == '\r');
        start = pos + 1;
        
        if (start < source.size){
            pos0 = find_substr_s(source, start, make_lit_string("\n\n"));
            pos1 = find_substr_s(source, start, make_lit_string("\r\n\r\n"));
            if (pos1 < pos0){
                pos0 = pos1;
            }
            next = substr(source, start, pos0 - start);
        }
    }
    
    return(next);
}

static String
get_next_word(String source, String prev_word){
    String word = {0};
    int32_t pos0 = (int32_t)(prev_word.str - source.str) + prev_word.size;
    int32_t pos1 = 0;
    char c = 0;
    
    for (; pos0 < source.size; ++pos0){
        c = source.str[pos0];
        if (!(char_is_whitespace(c) || c == '(' || c == ')')){
            break;
        }
    }
    
    if (pos0 < source.size){
        for (pos1 = pos0; pos1 < source.size; ++pos1){
            c = source.str[pos1];
            if (char_is_whitespace(c) || c == '(' || c == ')'){
                break;
            }
        }
        
        word = substr(source, pos0, pos1 - pos0);
    }
    
    return(word);
}

static String
get_first_word(String source){
    String start_str = make_string(source.str, 0);
    String word = get_next_word(source, start_str);
    return(word);
}

enum Doc_Chunk_Type{
    DocChunk_PlainText,
    DocChunk_CodeExample,
    
    DocChunk_Count
};

static String doc_chunk_headers[] = {
    make_lit_string(""),
    make_lit_string("CODE_EXAMPLE"),
};

static String
get_next_doc_chunk(String source, String prev_chunk, Doc_Chunk_Type *type){
    String chunk = {0};
    String word = {0};
    int32_t pos = source.size;
    int32_t word_index = 0;
    Doc_Chunk_Type t = DocChunk_PlainText;
    
    int32_t start_pos = (int32_t)(prev_chunk.str - source.str) + prev_chunk.size;
    String source_tail = substr_tail(source, start_pos);
    
    Assert(DocChunk_Count == ArrayCount(doc_chunk_headers));
    
    for (word = get_first_word(source_tail);
         word.str;
         word = get_next_word(source_tail, word), ++word_index){
        
        for (int32_t i = 1; i < DocChunk_Count; ++i){
            if (match_ss(word, doc_chunk_headers[i])){
                pos = (int32_t)(word.str - source.str);
                t = (Doc_Chunk_Type)i;
                goto doublebreak;
            }
        }
    }
    doublebreak:;
    
    *type = DocChunk_PlainText;
    if (word_index == 0){
        *type = t;
        
        int32_t nest_level = 1;
        int32_t i = find_s_char(source, pos, '(');
        for (++i; i < source.size; ++i){
            if (source.str[i] == '('){
                ++nest_level;
            }
            else if (source.str[i] == ')'){
                --nest_level;
                if (nest_level == 0){
                    break;
                }
            }
        }
        
        pos = i+1;
    }
    
    chunk = substr(source, start_pos, pos - start_pos);
    
    int32_t is_all_white = 1;
    for (int32_t i = 0; i < chunk.size; ++i){
        if (!char_is_whitespace(chunk.str[i])){
            is_all_white = 0;
            break;
        }
    }
    
    if (is_all_white){
        chunk = null_string;
    }
    
    return(chunk);
}

static String
get_first_doc_chunk(String source, Doc_Chunk_Type *type){
    String start_str = make_string(source.str, 0);
    String chunk = get_next_doc_chunk(source, start_str, type);
    return(chunk);
}

static void
print_doc_description(String *out, Partition *part, String src){
    Doc_Chunk_Type type;
    
    for (String chunk = get_first_doc_chunk(src, &type);
         chunk.str;
         chunk = get_next_doc_chunk(src, chunk, &type)){
        
        switch (type){
            case DocChunk_PlainText:
            {
                for (String line = get_first_double_line(chunk);
                     line.str;
                     line = get_next_double_line(chunk, line)){
                    append_ss(out, line);
                    append_sc(out, "<br><br>");
                }
            }break;
            
            case DocChunk_CodeExample:
            {
                int32_t start = 0;
                int32_t end = chunk.size-1;
                while (start < end && chunk.str[start] != '(') ++start;
                start += 1;
                while (end > start && chunk.str[end] != ')') --end;
                
                
                append_sc(out, EXAMPLE_CODE_OPEN);
                
                if (start < end){
                    String code_example = substr(chunk, start, end - start);
                    int32_t first_line = 1;
                    
                    for (String line = get_first_line(code_example);
                         line.str;
                         line = get_next_line(code_example, line)){
                        
                        if (!(first_line && line.size == 0)){
                            int32_t space_i = 0;
                            for (; space_i < line.size; ++space_i){
                                if (line.str[space_i] == ' '){
                                    append_sc(out, "&nbsp;");
                                }
                                else{
                                    break;
                                }
                            }
                            
                            String line_tail = substr_tail(line, space_i);
                            append_ss(out, line_tail);
                            append_sc(out, "<br>");
                        }
                        first_line = 0;
                    }
                }
                
                append_sc(out, EXAMPLE_CODE_CLOSE);
            }break;
        }
    }
}

static void
print_struct_docs(String *out, Partition *part, Item_Node *member){
    for (Item_Node *member_iter = member->first_child;
         member_iter != 0;
         member_iter = member_iter->next_sibling){
        String type = member_iter->type;
        if (match_ss(type, make_lit_string("struct")) ||
            match_ss(type, make_lit_string("union"))){
            print_struct_docs(out, part, member_iter);
        }
        else{
            Documentation doc = {0};
            perform_doc_parse(part, member_iter->doc_string, &doc);
            
            append_sc(out, "<div>");
            
            append_sc(out, "<div style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN);
            append_ss(out, member_iter->name);
            append_sc(out, DOC_ITEM_HEAD_INL_CLOSE"</div>");
            
            append_sc(out, "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN);
            print_doc_description(out, part, doc.main_doc);
            append_sc(out, DOC_ITEM_CLOSE"</div>");
            
            append_sc(out, "</div>");
        }
    }
}

static void
print_see_also(String *out, Documentation *doc){
    int32_t doc_see_count = doc->see_also_count;
    if (doc_see_count > 0){
        append_sc(out, DOC_HEAD_OPEN"See Also"DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_see_count; ++j){
            String see_also = doc->see_also[j];
            append_sc(out, DOC_ITEM_OPEN"<a href='#");
            append_ss(out, see_also);
            append_sc(out, "_doc'>");
            append_ss(out, see_also);
            append_sc(out, "</a>"DOC_ITEM_CLOSE);
        }
    }
}

static void
print_function_body_code(String *out, Parse_Context *context, int32_t start){
    String pstr = {0}, lexeme = {0};
    Cpp_Token *token = 0;
    
    int32_t do_print = 0;
    int32_t nest_level = 0;
    int32_t finish = false;
    int32_t do_whitespace_print = false;
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (do_whitespace_print){
            pstr = str_start_end(context->data, start, token->start);
            append_ss(out, pstr);
        }
        else{
            do_whitespace_print = true;
        }
        
        do_print = true;
        if (token->type == CPP_TOKEN_COMMENT){
            lexeme = get_lexeme(*token, context->data);
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
        
        if (do_print){
            pstr = get_lexeme(*token, context->data);
            append_ss(out, pstr);
        }
        
        start = token->start + token->size;
        
        if (finish){
            break;
        }
    }
}

static void
print_function_docs(String *out, Partition *part, String name, String doc_string){
    if (doc_string.size == 0){
        append_sc(out, "No documentation generated for this function.");
        fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    
    Documentation doc = {0};
    
    perform_doc_parse(part, doc_string, &doc);
    
    int32_t doc_param_count = doc.param_count;
    if (doc_param_count > 0){
        append_sc(out, DOC_HEAD_OPEN"Parameters"DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_param_count; ++j){
            String param_name = doc.param_name[j];
            String param_docs = doc.param_docs[j];
            
            // TODO(allen): check that param_name is actually
            // a parameter to this function!
            
            append_sc(out, "<div>"DOC_ITEM_HEAD_OPEN);
            append_ss(out, param_name);
            append_sc(out, DOC_ITEM_HEAD_CLOSE"<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN);
            append_ss(out, param_docs);
            append_sc(out, DOC_ITEM_CLOSE"</div></div>");
        }
    }
    
    String ret_doc = doc.return_doc;
    if (ret_doc.size != 0){
        append_sc(out, DOC_HEAD_OPEN"Return"DOC_HEAD_CLOSE DOC_ITEM_OPEN);
        append_ss(out, ret_doc);
        append_sc(out, DOC_ITEM_CLOSE);
    }
    
    String main_doc = doc.main_doc;
    if (main_doc.size != 0){
        append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE DOC_ITEM_OPEN);
        print_doc_description(out, part, main_doc);
        append_sc(out, DOC_ITEM_CLOSE);
    }
    
    print_see_also(out, &doc);
    
    end_temp_memory(temp);
}

static void
print_item_in_list(String *out, String name, char *id_postfix){
    append_sc(out, "<li><a href='#");
    append_ss(out, name);
    append_sc(out, id_postfix);
    append_sc(out, "'>");
    append_ss(out, name);
    append_sc(out, "</a></li>");
}

static void
print_item(String *out, Partition *part, Used_Links *used,
           Item_Node *item, char *id_postfix, char *function_prefix,
           char *section, int32_t I){
    Temp_Memory temp = begin_temp_memory(part);
    
    String name = item->name;
    /* NOTE(allen):
    Open a div for the whole item.
    Put a heading in it with the name and section.
    Open a "descriptive" box for the display of the code interface.
    */
    append_sc(out, "<div id='");
    append_ss(out, name);
    append_sc(out, id_postfix);
    append_sc(out, "' style='margin-bottom: 1cm;'>");
    
    int32_t has_cpp_name = 0;
    if (item->cpp_name.str != 0){
        if (try_to_use(used, item->cpp_name)){
            append_sc(out, "<div id='");
            append_ss(out, item->cpp_name);
            append_sc(out, id_postfix);
            append_sc(out, "'>");
            has_cpp_name = 1;
        }
    }
    
    append_sc         (out, "<h4>&sect;");
    append_sc         (out, section);
    append_s_char     (out, '.');
    append_int_to_str (out, I);
    append_sc         (out, ": ");
    append_ss         (out, name);
    append_sc         (out, "</h4>");
    
    append_sc(out, "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>");
    
    switch (item->t){
        case Item_Function:
        {
            // NOTE(allen): Code box
            Assert(function_prefix != 0);
            print_function_html(out, used, item->cpp_name,
                                item->ret, function_prefix, item->name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, part, item->name, item->doc_string);
        }break;
        
        case Item_Macro:
        {
            // NOTE(allen): Code box
            print_macro_html(out, item->name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, part, item->name, item->doc_string);
        }break;
        
        case Item_Typedef:
        {
            String type = item->type;
            
            // NOTE(allen): Code box
            append_sc     (out, "typedef ");
            append_ss     (out, type);
            append_s_char (out, ' ');
            append_ss     (out, name);
            append_s_char (out, ';');
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            String doc_string = item->doc_string;
            Documentation doc = {0};
            perform_doc_parse(part, doc_string, &doc);
            
            String main_doc = doc.main_doc;
            if (main_doc.size != 0){
                append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                
                append_sc(out, DOC_ITEM_OPEN);
                print_doc_description(out, part, main_doc);
                append_sc(out, DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            print_see_also(out, &doc);
            
        }break;
        
        case Item_Enum:
        {
            // NOTE(allen): Code box
            append_sc     (out, "enum ");
            append_ss     (out, name);
            append_s_char (out, ';');
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            String doc_string = item->doc_string;
            Documentation doc = {0};
            perform_doc_parse(part, doc_string, &doc);
            
            String main_doc = doc.main_doc;
            if (main_doc.size != 0){
                append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                
                append_sc(out, DOC_ITEM_OPEN);
                print_doc_description(out, part, main_doc);
                append_sc(out, DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            if (item->first_child){
                append_sc(out, DOC_HEAD_OPEN"Values"DOC_HEAD_CLOSE);
                
                for (Item_Node *member = item->first_child;
                     member;
                     member = member->next_sibling){
                    Documentation doc = {0};
                    perform_doc_parse(part, member->doc_string, &doc);
                    
                    append_sc(out, "<div>");
                    
                    // NOTE(allen): Dafuq is this all?
                    append_sc(out, "<div><span style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN);
                    append_ss(out, member->name);
                    append_sc(out, DOC_ITEM_HEAD_INL_CLOSE);
                    
                    if (member->value.str){
                        append_sc(out, " = ");
                        append_ss(out, member->value);
                    }
                    
                    append_sc(out, "</span></div>");
                    
                    append_sc(out, "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN);
                    print_doc_description(out, part, doc.main_doc);
                    append_sc(out, DOC_ITEM_CLOSE"</div>");
                    
                    append_sc(out, "</div>");
                }
            }
            
            print_see_also(out, &doc);
            
        }break;
        
        case Item_Struct: case Item_Union:
        {
            String doc_string = item->doc_string;
            
            int32_t hide_members = 0;
            
            if (doc_string.size == 0){
                hide_members = 1;
            }
            else{
                for (String word = get_first_word(doc_string);
                     word.str;
                     word = get_next_word(doc_string, word)){
                    if (match_ss(word, make_lit_string("HIDE_MEMBERS"))){
                        hide_members = 1;
                        break;
                    }
                }
            }
            
            // NOTE(allen): Code box
            print_struct_html(out, item, hide_members);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            {
                Documentation doc = {0};
                perform_doc_parse(part, doc_string, &doc);
                
                String main_doc = doc.main_doc;
                if (main_doc.size != 0){
                    append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                    
                    append_sc(out, DOC_ITEM_OPEN);
                    print_doc_description(out, part, main_doc);
                    append_sc(out, DOC_ITEM_CLOSE);
                }
                else{
                    fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
                }
                
                if (!hide_members){
                    if (item->first_child){
                        append_sc(out, DOC_HEAD_OPEN"Fields"DOC_HEAD_CLOSE);
                        print_struct_docs(out, part, item);
                    }
                }
                
                print_see_also(out, &doc);
            }
        }break;
    }
    
    if (has_cpp_name){
        append_sc(out, "</div>");
    }
    
    // NOTE(allen): Close the item box
    append_sc(out, "</div><hr>");
    
    end_temp_memory(temp);
}

typedef struct App_API_Name{
    String macro;
    String public_name;
} App_API_Name;

typedef struct App_API{
    App_API_Name *names;
} App_API;

static App_API
allocate_app_api(Partition *part, int32_t count){
    App_API app_api = {0};
    app_api.names = push_array(part, App_API_Name, count);
    memset(app_api.names, 0, sizeof(App_API_Name)*count);
    return(app_api);
}

static void
generate_custom_headers(){
#define API_H "4coder_custom_api.h"
#define OS_API_H "4ed_os_custom_api.h"
#define API_DOC "4coder_API.html"
#define STRING_H "4coder_string.h"
    
    int32_t size = (512 << 20);
    void *mem = malloc(size);
    memset(mem, 0, size);
    
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    
    // NOTE(allen): Parse the internal string file.
    static char *string_files[] = {
        "internal_4coder_string.cpp"
    };
    
    static Meta_Keywords string_keys[] = {
        {make_lit_string("FSTRING_INLINE") , Item_Function } ,
        {make_lit_string("FSTRING_LINK")   , Item_Function } ,
        {make_lit_string("DOC_EXPORT")     , Item_Macro    } ,
        {make_lit_string("CPP_NAME")       , Item_CppName  } ,
    };
    
    Meta_Unit string_unit = compile_meta_unit(part, string_files, ArrayCount(string_files),
                                              string_keys, ArrayCount(string_keys));
    
    
    // NOTE(allen): Parse the lexer library
    static char *lexer_types_files[] = {
        "4cpp_lexer_types.h",
    };
    
    static Meta_Keywords lexer_types_keys[] = {
        {make_lit_string("typedef") , Item_Typedef } ,
        {make_lit_string("struct")  , Item_Struct  } ,
        {make_lit_string("union")   , Item_Union   } ,
        {make_lit_string("ENUM")    , Item_Enum    } ,
    };
    
    Meta_Unit lexer_types_unit =
        compile_meta_unit(part, lexer_types_files, ArrayCount(lexer_types_files),
                          lexer_types_keys, ArrayCount(lexer_types_keys));
    
    static char *lexer_funcs_files[] = {
        "4cpp_lexer.h",
    };
    
    static Meta_Keywords lexer_funcs_keys[] = {
        {make_lit_string("FCPP_LINK") , Item_Function } ,
    };
    
    Meta_Unit lexer_funcs_unit =
        compile_meta_unit(part, lexer_funcs_files, ArrayCount(lexer_funcs_files),
                          lexer_funcs_keys, ArrayCount(lexer_funcs_keys));
    
    
    
    // NOTE(allen): Parse the customization API files
    static char *functions_files[] = {
        "4ed_api_implementation.cpp",
        "win32_api_impl.cpp",
    };
    
    static Meta_Keywords functions_keys[] = {
        {make_lit_string("API_EXPORT"), Item_Function},
    };
    
    Meta_Unit unit_custom = compile_meta_unit(part, functions_files, ArrayCount(functions_files),
                                              functions_keys, ArrayCount(functions_keys));
    
    
    // NOTE(allen): Compute and store variations of the function names
    App_API func_4ed_names = allocate_app_api(part, unit_custom.set.count);
    
    for (int32_t i = 0; i < unit_custom.set.count; ++i){
        String name_string = unit_custom.set.items[i].name;
        String *macro = &func_4ed_names.names[i].macro;
        String *public_name = &func_4ed_names.names[i].public_name;
        
        *macro = str_alloc(part, name_string.size+4);
        to_upper_ss(macro, name_string);
        append_ss(macro, make_lit_string("_SIG"));
        
        *public_name = str_alloc(part, name_string.size);
        to_lower_ss(public_name, name_string);
        
        partition_align(part, 4);
    }
    
    // NOTE(allen): Parse the customization API types
    static char *type_files[] = {
        "4coder_types.h",
    };
    
    static Meta_Keywords type_keys[] = {
        {make_lit_string("typedef") , Item_Typedef } ,
        {make_lit_string("struct")  , Item_Struct  } ,
        {make_lit_string("union")   , Item_Union   } ,
        {make_lit_string("ENUM")    , Item_Enum    } ,
    };
    
    Meta_Unit unit = compile_meta_unit(part, type_files, ArrayCount(type_files),
                                       type_keys, ArrayCount(type_keys));
    
    // NOTE(allen): Output
    String out = str_alloc(part, 10 << 20);
    Out_Context context = {0};
    
    // NOTE(allen): Custom API headers
    if (begin_file_out(&context, OS_API_H, &out)){
        int32_t main_api_count = unit_custom.parse[0].item_count;
        int32_t os_api_count = unit_custom.parse[1].item_count;
        for (int32_t i = main_api_count; i < os_api_count; ++i){
            append_sc(&out, "#define ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_sc(&out, "(n) ");
            append_ss(&out, unit_custom.set.items[i].ret);
            append_sc(&out, " n");
            append_ss(&out, unit_custom.set.items[i].args);
            append_s_char(&out, '\n');
        }
        
        for (int32_t i = main_api_count; i < os_api_count; ++i){
            append_sc(&out, "typedef ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_s_char(&out, '(');
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function);\n");
        }
        
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    if (begin_file_out(&context, API_H, &out)){
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_sc(&out, "#define ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_sc(&out, "(n) ");
            append_ss(&out, unit_custom.set.items[i].ret);
            append_sc(&out, " n");
            append_ss(&out, unit_custom.set.items[i].args);
            append_s_char(&out, '\n');
        }
        
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_sc(&out, "typedef ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_s_char(&out, '(');
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function);\n");
        }
        
        append_sc(&out,
                  "struct Application_Links{\n");
        
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function *");
            append_ss(&out, func_4ed_names.names[i].public_name);
            append_sc(&out, ";\n");
        }
        
        append_sc(&out,
                  "void *memory;\n"
                  "int32_t memory_size;\n"
                  "void *cmd_context;\n"
                  "void *system_links;\n"
                  "void *current_coroutine;\n"
                  "int32_t type_coroutine;\n"
                  "};\n");
        
        append_sc(&out, "#define FillAppLinksAPI(app_links) do{");
        
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_sc(&out, "\\\napp_links->");
            append_ss(&out, func_4ed_names.names[i].public_name);
            append_sc(&out, " = ");
            append_ss(&out, unit_custom.set.items[i].name);
            append_s_char(&out, ';');
        }
        append_sc(&out, "} while(false)\n");
        
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    // NOTE(allen): String Library
    if (begin_file_out(&context, STRING_H, &out)){
        Cpp_Token *token = 0;
        int32_t start = 0;
        
        Parse parse = string_unit.parse[0];
        Parse_Context pcontext = setup_parse_context(parse);
        
        for (; (token = get_token(&pcontext)) != 0; get_next_token(&pcontext)){
            if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                token->type == CPP_TOKEN_IDENTIFIER){
                String lexeme = get_lexeme(*token, pcontext.data);
                if (match_ss(lexeme, make_lit_string("FSTRING_BEGIN"))){
                    start = token->start + token->size;
                    break;
                }
            }
        }
        
        String pstr = {0};
        int32_t do_whitespace_print = true;
        
        for(;(token = get_next_token(&pcontext)) != 0;){
            if (do_whitespace_print){
                pstr = str_start_end(pcontext.data, start, token->start);
                append_ss(&out, pstr);
            }
            else{
                do_whitespace_print = true;
            }
            
            String lexeme = get_lexeme(*token, pcontext.data);
            
            int32_t do_print = true;
            if (match_ss(lexeme, make_lit_string("FSTRING_DECLS"))){
                append_sc(&out, "#if !defined(FCODER_STRING_H)\n#define FCODER_STRING_H\n\n");
                do_print = false;
                
                static int32_t RETURN_PADDING = 16;
                static int32_t SIG_PADDING = 27;
                
                for (int32_t j = 0; j < string_unit.set.count; ++j){
                    char line_[2048];
                    String line = make_fixed_width_string(line_);
                    Item_Node *item = string_unit.set.items + j;
                    
                    if (item->t == Item_Function){
                        append_ss       (&line, item->marker);
                        append_padding  (&line, ' ', RETURN_PADDING);
                        append_ss       (&line, item->ret);
                        append_padding  (&line, ' ', SIG_PADDING);
                        append_ss       (&line, item->name);
                        append_ss       (&line, item->args);
                        append_sc       (&line, ";\n");
                    }
                    else if (item->t == Item_Macro){
                        append_ss       (&line, make_lit_string("#ifndef "));
                        append_padding  (&line, ' ', 10);
                        append_ss       (&line, item->name);
                        append_s_char   (&line, '\n');
                        
                        append_ss       (&line, make_lit_string("# define "));
                        append_padding  (&line, ' ', 10);
                        append_ss       (&line, item->name);
                        append_ss       (&line, item->args);
                        append_s_char   (&line, ' ');
                        append_ss       (&line, item->body);
                        append_s_char   (&line, '\n');
                        
                        append_ss       (&line, make_lit_string("#endif"));
                        append_s_char   (&line, '\n');
                    }
                    else{
                        InvalidPath;
                    }
                    
                    append_ss(&out, line);
                }
                
                append_sc(&out, "\n#endif\n");
                
                // NOTE(allen): C++ overload definitions
                append_sc(&out, "\n#if !defined(FSTRING_C) && !defined(FSTRING_GUARD)\n\n");
                
                for (int32_t j = 0; j < string_unit.set.count; ++j){
                    char line_space[2048];
                    String line = make_fixed_width_string(line_space);
                    
                    Item_Node *item = &string_unit.set.items[j];
                    
                    if (item->t == Item_Function){
                        String cpp_name = item->cpp_name;
                        if (cpp_name.str != 0){
                            Argument_Breakdown breakdown = item->breakdown;
                            
                            append_ss     (&line, make_lit_string("FSTRING_INLINE"));
                            append_padding(&line, ' ', RETURN_PADDING);
                            append_ss     (&line, item->ret);
                            append_padding(&line, ' ', SIG_PADDING);
                            append_ss     (&line, cpp_name);
                            append_ss     (&line, item->args);
                            if (match_ss(item->ret, make_lit_string("void"))){
                                append_ss(&line, make_lit_string("{("));//}
                            }
                            else{
                                append_ss(&line, make_lit_string("{return("));//}
                            }
                            append_ss    (&line, item->name);
                            append_s_char(&line, '(');
                            
                            if (breakdown.count > 0){
                                for (int32_t i = 0; i < breakdown.count; ++i){
                                    if (i != 0){
                                        append_s_char(&line, ',');
                                    }
                                    append_ss(&line, breakdown.args[i].param_name);
                                }
                            }
                            else{
                                append_ss(&line, make_lit_string("void"));
                            }
                            
                            //{
                            append_ss(&line, make_lit_string("));}\n"));
                            
                            append_ss(&out, line);
                        }
                    }
                }
                
                append_sc(&out, "\n#endif\n");
            }
            
            else if (match_ss(lexeme, make_lit_string("DOC_EXPORT"))){
                token = get_next_token(&pcontext);
                if (token && token->type == CPP_TOKEN_COMMENT){
                    token = get_next_token(&pcontext);
                    if (token && token->type == CPP_PP_DEFINE){
                        for (;(token = get_next_token(&pcontext)) != 0;){
                            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                break;
                            }
                        }
                        if (token != 0){
                            get_prev_token(&pcontext);
                        }
                        do_print = false;
                        do_whitespace_print = false;
                    }
                }
            }
            
            else if (match_ss(lexeme, make_lit_string("FSTRING_INLINE")) ||
                     match_ss(lexeme, make_lit_string("FSTRING_LINK"))){
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    if (match_ss(lexeme, make_lit_string("FSTRING_INLINE"))){
                        append_sc(&out, "#if !defined(FSTRING_GUARD)\n");
                    }
                    else{
                        append_sc(&out, "#if defined(FSTRING_IMPLEMENTATION)\n");
                    }
                    print_function_body_code(&out, &pcontext, start);
                    append_sc(&out, "\n#endif");
                    do_print = false;
                }
            }
            
            else if (match_ss(lexeme, make_lit_string("CPP_NAME"))){
                Cpp_Token *token_start = token;
                int32_t has_cpp_name = false;
                
                token = get_next_token(&pcontext);
                if (token && token->type == CPP_TOKEN_PARENTHESE_OPEN){
                    token = get_next_token(&pcontext);
                    if (token && token->type == CPP_TOKEN_IDENTIFIER){
                        token = get_next_token(&pcontext);
                        if (token && token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                            has_cpp_name = true;
                            do_print = false;
                        }
                    }
                }
                
                if (!has_cpp_name){
                    token = set_token(&pcontext, token_start);
                }
            }
            
            else if (token->type == CPP_TOKEN_COMMENT){
                if (check_and_fix_docs(&lexeme)){
                    do_print = false;
                }
            }
            
            if ((token = get_token(&pcontext)) != 0){
                if (do_print){
                    pstr = get_lexeme(*token, pcontext.data);
                    append_ss(&out, pstr);
                }
                start = token->start + token->size;
            }
        }
        pstr = str_start_end(pcontext.data, start, parse.code.size);
        append_ss(&out, pstr);
        
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    
    // Output Docs
    
    if (begin_file_out(&context, API_DOC, &out)){
        
        Used_Links used_links = {0};
        init_used_links(part, &used_links, 4000);
        
        append_sc(&out,
                  "<html lang=\"en-US\">"
                  "<head>"
                  "<title>4coder API Docs</title>"
                  "<style>"
                  
                  "body { "
                  "background: " BACK_COLOR "; "
                  "color: " TEXT_COLOR "; "
                  "}"
                  
                  // H things
                  "h1,h2,h3,h4 { "
                  "color: " POP_COLOR_1 "; "
                  "margin: 0; "
                  "}"
                  
                  "h2 { "
                  "margin-top: 6mm; "
                  "}"
                  
                  "h3 { "
                  "margin-top: 5mm; margin-bottom: 5mm; "
                  "}"
                  
                  "h4 { "
                  "font-size: 1.1em; "
                  "}"
                  
                  // ANCHORS
                  "a { "
                  "color: " POP_COLOR_1 "; "
                  "text-decoration: none; "
                  "}"
                  "a:visited { "
                  "color: " VISITED_LINK "; "
                  "}"
                  "a:hover { "
                  "background: " POP_BACK_1 "; "
                  "}"
                  
                  // LIST
                  "ul { "
                  "list-style: none; "
                  "padding: 0; "
                  "margin: 0; "
                  "}"
                  
                  "</style>"
                  "</head>\n"
                  "<body>"
                  "<div style='font-family:Arial; margin: 0 auto; "
                  "width: 800px; text-align: justify; line-height: 1.25;'>"
                  "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4cpp Lexing Library</h1>");
        
//                "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4coder API</h1>");
        
        struct Section{
            char *id_string;
            char *display_string;
        };
        
        static int32_t msection = -1;
        
        static Section sections[] = {
            {"introduction", "Introduction"},
            {"4coder_systems", "4coder Systems"},
            {"types_and_functions", "Types and Functions"},
            {"string_library", "String Library"},
            {"lexer_library", "Lexer Library"}
        };
        
        append_sc(&out, "<h3 style='margin:0;'>Table of Contents</h3>""<ul>");
        
        int32_t section_count = ArrayCount(sections);
        for (int32_t i = 0; i < section_count; ++i){
            append_sc         (&out, "<li><a href='#section_");
            append_sc         (&out, sections[i].id_string);
            append_sc         (&out, "'>&sect;");
            append_int_to_str (&out, i+1);
            append_s_char     (&out, ' ');
            append_sc         (&out, sections[i].display_string);
            append_sc         (&out, "</a></li>");
        }
        
        append_sc(&out, "</ul>");
        
#define MAJOR_SECTION "1"
        msection = 0;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#if 0
        // NOTE(allen): doc intro for lexer standalone
        append_sc(&out,
                  "<div>"
                  "<p>This is the documentation for the 4cpp lexer version 1.0. "
                  "The documentation is the newest piece of this lexer project "
                  "so it may still have problems.  What is here should be correct "
                  "and mostly complete.</p>"
                  "<p>If you have questions or discover errors please contact "
                  "<span style='"CODE_STYLE"'>editor@4coder.net</span> or "
                  "to get help from community members you can post on the "
                  "4coder forums hosted on handmade.network at "
                  "<span style='"CODE_STYLE"'>4coder.handmade.network</span></p>"
                  "</div>");
#endif
        
        append_sc(&out,
                  "<div>"
                  "<p>This is the documentation for " VERSION " The documentation is still "
                  "under construction so some of the links are linking to sections that "
                  "have not been written yet.  What is here should be correct and I suspect "
                  "useful even without some of the other sections.</p>"
                  "<p>If you have questions or discover errors please contact "
                  "<span style='"CODE_STYLE"'>editor@4coder.net</span> or "
                  "to get help from community members you can post on the "
                  "4coder forums hosted on handmade.network at "
                  "<span style='"CODE_STYLE"'>4coder.handmade.network</span></p>"
                  "</div>");
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "2"
        msection = 1;
        
        // TODO(allen): Write the 4coder system descriptions.
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
        append_sc(&out, "<div><i>Coming Soon</i><div>");
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "3"
        msection = 2;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".1"
        
        append_sc(&out, "<h3>&sect;"SECTION" Function List</h3><ul>");
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            print_item_in_list(&out, func_4ed_names.names[i].public_name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".2"
        
        append_sc(&out, "<h3>&sect;"SECTION" Type List</h3><ul>");
        for (int32_t i = 0; i < unit.set.count; ++i){
            print_item_in_list(&out, unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".3"
        
        append_sc(&out, "<h3>&sect;"SECTION" Function Descriptions</h3>");
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            Item_Node *item = &unit_custom.set.items[i];
            String name = func_4ed_names.names[i].public_name;
            
            append_sc        (&out, "<div id='");
            append_ss        (&out, name);
            append_sc        (&out, "_doc' style='margin-bottom: 1cm;'><h4>&sect;"SECTION".");
            append_int_to_str(&out, i+1);
            append_sc        (&out, ": ");
            append_ss        (&out, name);
            append_sc        (&out, "</h4><div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>");
            
            print_function_html(&out, &used_links, item->cpp_name, item->ret, "app->", name, item->breakdown);
            append_sc(&out, "</div>");
            
            print_function_docs(&out, part, name, item->doc_string);
            
            append_sc(&out, "</div><hr>");
        }
        
#undef SECTION
#define SECTION MAJOR_SECTION".4"
        
        append_sc(&out, "<h3>&sect;"SECTION" Type Descriptions</h3>");
        
        int32_t I = 1;
        for (int32_t i = 0; i < unit.set.count; ++i, ++I){
            print_item(&out, part, &used_links, unit.set.items + i, "_doc", 0, SECTION, I);
        }
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "4"
        msection = 3;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".1"
        
        append_sc(&out, "<h3>&sect;"SECTION" String Intro</h3>");
        
        append_sc(&out, "<div><i>Coming Soon</i><div>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".2"
        
        append_sc(&out, "<h3>&sect;"SECTION" String Function List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < string_unit.set.count; ++i){
            print_item_in_list(&out, string_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".3"
        
        append_sc(&out, "<h3>&sect;"SECTION" String Function Descriptions</h3>");
        
        for (int32_t i = 0; i < string_unit.set.count; ++i){
            print_item(&out, part, &used_links, string_unit.set.items+i, "_doc", "", SECTION, i+1);
        }
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "5"
        msection = 4;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".1"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Intro</h3>");
        
        append_sc(&out,
                  "<div>"
                  "The 4cpp lexer system provides a polished, fast, flexible system that "
                  "takes in C/C++ and outputs a tokenization of the text data.  There are "
                  "two API levels. One level is setup to let you easily get a tokenization "
                  "of the file.  This level manages memory for you with malloc to make it "
                  "as fast as possible to start getting your tokens. The second level "
                  "enables deep integration by allowing control over allocation, data "
                  "chunking, and output rate control.<br><br>"
                  "To use the quick setup API you simply include 4cpp_lexer.h and read the "
                  "documentation at <a href='#cpp_lex_file_doc'>cpp_lex_file</a>.<br><br>"
                  "To use the the fancier API include 4cpp_lexer.h and read the "
                  "documentation at <a href='#cpp_lex_step_doc'>cpp_lex_step</a>. "
                  "If you want to be absolutely sure you are not including malloc into "
                  "your program you can define FCPP_FORBID_MALLOC before the include and "
                  "the \"step\" API will continue to work.<br><br>"
                  "There are a few more features in 4cpp that are not documented yet. "
                  "You are free to try to use these, but I am not totally sure they are "
                  "ready yet, and when they are they will be documented."
                  "</div>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".2"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Function List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < lexer_funcs_unit.set.count; ++i){
            print_item_in_list(&out, lexer_funcs_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".3"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Types List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < lexer_types_unit.set.count; ++i){
            print_item_in_list(&out, lexer_types_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".4"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Function Descriptions</h3>");
        for (int32_t i = 0; i < lexer_funcs_unit.set.count; ++i){
            print_item(&out, part, &used_links, lexer_funcs_unit.set.items+i, "_doc", "", SECTION, i+1);
        }
        
#undef SECTION
#define SECTION MAJOR_SECTION".5"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Type Descriptions</h3>");
        for (int32_t i = 0; i < lexer_types_unit.set.count; ++i){
            print_item(&out, part, &used_links, lexer_types_unit.set.items+i, "_doc", "", SECTION, i+1);
        }
        
        
        append_sc(&out, "</div></body></html>");
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
}

int main(int argc, char **argv){
    generate_keycode_enum();
    generate_style();
    generate_custom_headers();
}

// BOTTOM

