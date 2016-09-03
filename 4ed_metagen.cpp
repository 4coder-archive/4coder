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

#define FCPP_ALLOW_MALLOC
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
get_string(char *data, int32_t start, int32_t end){
    return(make_string(data + start, end - start));
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
    Item_Macro,
    Item_Typedef,
    Item_Struct,
    Item_Union,
    Item_Enum,
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
    Cpp_Token_Stack tokens;
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

static Item_Node null_item_node = {0};

static String
get_lexeme(Cpp_Token token, char *code){
    String str = make_string(code + token.start, token.size);
    return(str);
}

static Parse_Context
setup_parse_context(char *data, Cpp_Token_Stack stack){
    Parse_Context context;
    context.token_s = stack.tokens;
    context.token_e = stack.tokens + stack.count;
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
    result.tokens = cpp_make_token_stack(1024);
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
    DOC_SEE
} Doc_Note_Type;

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
        }
    }
    
    return(result);
}

static String
doc_note_string[] = {
    make_lit_string("DOC_PARAM"),
    make_lit_string("DOC_RETURN"),
    make_lit_string("DOC"),
    make_lit_string("DOC_SEE"),
};

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
        
        String type = skip_chop_whitespace(get_string(context->data, start_token->start, token_j->start));
        
        String type_postfix =
            skip_chop_whitespace(get_string(context->data, token_j->start + token_j->size, token->start));
        
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
struct_parse(Partition *part, int32_t is_struct,
             Parse_Context *context, Item_Node *top_member){
    
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
            get_string(context->data, start_token->start + start_token->size, token_j->start)
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
                                get_string(context->data, start_token->start + start_token->size, token->start)
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
            if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                ((token->flags & CPP_TFLAG_IS_KEYWORD) ||
                 token->type == CPP_TOKEN_IDENTIFIER)){
                
                String lexeme = get_lexeme(*token, context->data);
                int32_t match_index = 0;
                if (string_set_match_table(keywords, sizeof(*keywords), key_count, lexeme, &match_index)){
                    switch (match_index){
                        case 0: //typedef
                        case 1: case 2: //struct/union
                        case 3: //ENUM
                        ++unit.set.count; break;
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
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                ((token->flags & CPP_TFLAG_IS_KEYWORD) ||
                 token->type == CPP_TOKEN_IDENTIFIER)){
                
                String lexeme = get_lexeme(*token, context->data);
                int32_t match_index = 0;
                if (string_set_match_table(keywords, sizeof(*keywords), key_count, lexeme, &match_index)){
                    switch (match_index){
                        case 0: //typedef
                        {
                            if (typedef_parse(context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Typedef);
                                ++index;
                            }
                            else{
                                InvalidPath;
                            }
                        }break;
                        
                        case 1: case 2: //struct/union
                        {
                            if (struct_parse(part, (match_index == 1),
                                             context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Struct ||
                                       unit.set.items[index].t == Item_Union);
                                ++index;
                            }
                            else{
                                InvalidPath;
                            }
                        }break;
                        
                        case 3: //ENUM
                        {
                            if (enum_parse(part, context, unit.set.items + index)){
                                Assert(unit.set.items[index].t == Item_Enum);
                                ++index;
                            }
                            else{
                                InvalidPath;
                            }
                        }break;
                        
                    }
                }
            }
        }
        
        // NOTE(allen): This is necessary for now because
        // the original count is slightly overestimated thanks
        // to nested structs and unions.
        unit.set.count = index;
    }
    
    return(unit);
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
parse_cpp_name(Parse_Context *context, char *data, String *name){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *token_start = get_token(context);
    
    token = get_next_token(context);
    if (token && token->type == CPP_TOKEN_PARENTHESE_OPEN){
        token = get_next_token(context);
        if (token && token->type == CPP_TOKEN_IDENTIFIER){
            token = get_next_token(context);
            if (token && token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                *name = get_lexeme(*(token-1), data);
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
function_sig_parse(Partition *part, Parse_Context *context,
                   char *data, Item_Set item_set, int32_t sig_count, String cpp_name){
    int32_t result = false;
    
    int32_t size = 0;
    Cpp_Token *token = 0;
    Cpp_Token *args_start_token = 0;
    Cpp_Token *ret_token = get_token(context);
    
    if (function_parse_goto_name(context)){
        token = get_token(context);
        args_start_token = token+1;
        item_set.items[sig_count].name = get_lexeme(*token, data);
        
        size = token->start - ret_token->start;
        item_set.items[sig_count].ret =
            chop_whitespace(make_string(data + ret_token->start, size));
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                break;
            }
        }
        
        if (token){
            size = token->start + token->size - args_start_token->start;;
            item_set.items[sig_count].args =
                make_string(data + args_start_token->start, size);
            item_set.items[sig_count].t = Item_Function;
            item_set.items[sig_count].cpp_name = cpp_name;
            item_set.items[sig_count].breakdown =
                parameter_parse(part, data, args_start_token, token);
            
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
function_parse(Partition *part, Parse_Context *context,
               char *data, Item_Set item_set, int32_t sig_count, String cpp_name){
    int32_t result = false;
    
    String doc_string = {0};
    Cpp_Token *token = get_token(context);
    
    item_set.items[sig_count].marker = get_lexeme(*token, data);
    
    if (function_get_doc(context, data, &doc_string)){
        item_set.items[sig_count].doc_string = doc_string;
    }
    
    set_token(context, token);
    if (get_next_token(context)){
        if (function_sig_parse(part, context, data, item_set, sig_count, cpp_name)){
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
macro_parse(Partition *part, Parse_Context *context,
            char *data, Item_Set item_set, int32_t sig_count){
    int32_t result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *doc_token = 0;
    Cpp_Token *args_start_token = 0;
    
    String doc_string = {0};
    
    if (macro_parse_check(context)){
        token = get_token(context);
        if (can_back_step(context)){
            doc_token = token-1;
            
            doc_string = get_lexeme(*doc_token, data);
            
            if (check_and_fix_docs(&doc_string)){
                item_set.items[sig_count].doc_string = doc_string;
                
                for (; (token = get_token(context)) != 0; get_next_token(context)){
                    if (token->type == CPP_TOKEN_IDENTIFIER){
                        break;
                    }
                }
                
                if (get_token(context) && (token->flags & CPP_TFLAG_PP_BODY)){
                    item_set.items[sig_count].name = get_lexeme(*token, data);
                    
                    if ((token = get_next_token(context)) != 0){
                        args_start_token = token;
                        for (; (token = get_token(context)) != 0; get_next_token(context)){
                            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                break;
                            }
                        }
                        
                        if (token){
                            int32_t start = args_start_token->start;
                            int32_t end = token->start + token->size;
                            item_set.items[sig_count].args = make_string(data + start, end - start);
                            
                            item_set.items[sig_count].breakdown =
                                parameter_parse(part, data, args_start_token, token);
                            
                            if ((token = get_next_token(context)) != 0){
                                Cpp_Token *body_start = token;
                                
                                if (body_start->flags & CPP_TFLAG_PP_BODY){
                                    for (; (token = get_token(context)) != 0; get_next_token(context)){
                                        if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                            break;
                                        }
                                    }
                                    
                                    token = get_prev_token(context);
                                    
                                    start = body_start->start;
                                    end = token->start + token->size;
                                    item_set.items[sig_count].body = make_string(data + start, end - start);
                                }
                            }
                            
                            item_set.items[sig_count].t = Item_Macro;
                            result = true;
                        }
                    }
                }
            }
        }
    }
    
    return(result);
}

static void
print_struct_html(FILE *file, Item_Node *member){
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
        
        for (Item_Node *member_iter = member->first_child;
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
print_function_html(FILE *file, Item_Node item, String name,
                    char *function_call_head){
    String ret = item.ret;
    fprintf(file,
            "%.*s %s%.*s(\n"
            "<div style='margin-left: 4mm;'>",
            ret.size, ret.str,
            function_call_head,
            name.size, name.str);
    
    Argument_Breakdown breakdown = item.breakdown;
    int32_t arg_count = breakdown.count;
    for (int32_t j = 0; j < arg_count; ++j){
        String param_string = breakdown.args[j].param_string;
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
print_macro_html(FILE *file, Item_Node item, String name){
    Argument_Breakdown breakdown = item.breakdown;
    int32_t arg_count = breakdown.count;
    if (arg_count == 0){
        fprintf(file,
                "#define %.*s()",
                name.size, name.str);
    }
    else if (arg_count == 1){
        String param_string = breakdown.args[0].param_string;
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
            String param_string = breakdown.args[j].param_string;
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
print_struct_docs(FILE *file, Partition *part, Item_Node *member){
    for (Item_Node *member_iter = member->first_child;
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

typedef struct String_Function_Marker{
    int32_t parse_function;
    int32_t is_inline;
    int32_t parse_doc;
    int32_t cpp_name;
} String_Function_Marker;

static String_Function_Marker
string_function_marker_check(String lexeme){
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

static void
print_item(Partition *part, FILE *file, Item_Node *item, char *section, int32_t I){
    String name = item->name;
    /* NOTE(allen):
    Open a div for the whole item.
    Put a heading in it with the name and section.
    Open a "descriptive" box for the display of the code interface.
    */
    fprintf(file,
            "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
            "<h4>&sect;%s.%d: %.*s</h4>\n"
            "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
            name.size, name.str, section, I, name.size, name.str);
    
    Temp_Memory temp = begin_temp_memory(part);
    
    switch (item->t){
        case Item_Typedef:
        {
            String type = item->type;
            
            // NOTE(allen): Code box
            {
                fprintf(file,
                        "typedef %.*s %.*s;",
                        type.size, type.str,
                        name.size, name.str
                        );
            }
            
            // NOTE(allen): Close the descriptive box
            fprintf(file, "</div>\n");
            
            // NOTE(allen): Descriptive section
            {
                String doc_string = item->doc_string;
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
        }break;
        
        case Item_Enum:
        {
            // NOTE(allen): Code box
            {
                fprintf(file,
                        "enum %.*s;",
                        name.size, name.str);
            }
            
            // NOTE(allen): Close the descriptive box
            fprintf(file, "</div>\n");
            
            // NOTE(allen): Descriptive section
            {
                String doc_string = item->doc_string;
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
                
                if (item->first_child){
                    fprintf(file, DOC_HEAD_OPEN"Values"DOC_HEAD_CLOSE);
                    for (Item_Node *member = item->first_child;
                         member;
                         member = member->next_sibling){
                        Documentation doc = {0};
                        perform_doc_parse(part, member->doc_string, &doc);
                        
                        if (member->value.str){
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
                        else{
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
                }
                
                print_see_also(file, &doc);
            }
        }break;
        
        case Item_Struct: case Item_Union:
        {
            Item_Node *member = item;
            
            // NOTE(allen): Code box
            {
                print_struct_html(file, member);
            }
            
            // NOTE(allen): Close the descriptive box
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
        }break;
    }
    
    // NOTE(allen): Close the item box
    fprintf(file, "</div><hr>\n");
    
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
    
    Parse string_parse = meta_lex("internal_4coder_string.cpp");
    
    int32_t string_function_count = 0;
    
    {
        char *data = string_parse.code.str;
        
        Cpp_Token *token = 0;
        
        Parse_Context context_ = setup_parse_context(data, string_parse.tokens);
        Parse_Context *context = &context_;
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_IDENTIFIER && !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = get_lexeme(*token, data);
                
                String_Function_Marker marker =
                    string_function_marker_check(lexeme);
                
                if (marker.parse_function){
                    if (function_parse_goto_name(context)){
                        ++string_function_count;
                    }
                }
                else if (marker.parse_doc){
                    if (macro_parse_check(context)){
                        ++string_function_count;
                    }
                }
            }
        }
    }
    
    Item_Set string_function_set = allocate_item_set(part, string_function_count);
    int32_t string_sig_count = 0;
    
    {
        String *code = &string_parse.code;
        Cpp_Token_Stack *token_stack = &string_parse.tokens;
        
        char *data = code->str;
        
        Parse_Context context_ = setup_parse_context(data, *token_stack);
        Parse_Context *context = &context_;
        
        String cpp_name = {0};
        int32_t has_cpp_name = 0;
        
        for (; get_token(context); get_next_token(context)){
            Cpp_Token *token = get_token(context);
            if (token->type == CPP_TOKEN_IDENTIFIER && !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                
                String_Function_Marker marker =
                    string_function_marker_check(lexeme);
                
                if (marker.cpp_name){
                    if (parse_cpp_name(context, data, &cpp_name)){
                        has_cpp_name = 1;
                    }
                }
                else if (marker.parse_function){
                    if (function_parse(part, context, data,
                                       string_function_set, string_sig_count,
                                       cpp_name)){
                        ++string_sig_count;
                    }
                }
                else if (marker.parse_doc){
                    if (macro_parse(part, context, data,
                                    string_function_set, string_sig_count)){
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
    
    Parse parses[2];
    parses[0] = meta_lex("4ed_api_implementation.cpp");
    parses[1] = meta_lex("win32_api_impl.cpp");
    
    int32_t line_count = 0;
    
    for (int32_t J = 0; J < 2; ++J){
        Parse *parse = &parses[J];
        
        char *data = parse->code.str;
        
        int32_t count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        Cpp_Token *token = tokens;
        
        Parse_Context context_ = setup_parse_context(data, parse->tokens);
        Parse_Context *context = &context_;
        
        for (int32_t i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                if (match_ss(lexeme, make_lit_string("API_EXPORT"))){
                    set_token(context, token);
                    if (function_parse_goto_name(context)){
                        ++line_count;
                    }
                }
            }
        }
    }
    
    Item_Set function_set = allocate_item_set(part, line_count);
    App_API func_4ed_names = allocate_app_api(part, line_count);
    int32_t sig_count = 0;
    int32_t sig_count_per_file[2];
    
    for (int32_t J = 0; J < 2; ++J){
        Parse *parse = &parses[J];
        
        char *data = parse->code.str;
        
        Parse_Context context_ = setup_parse_context(data, parse->tokens);
        Parse_Context *context = &context_;
        
        // NOTE(allen): Header Parse
        for (; get_token(context); get_next_token(context)){
            Cpp_Token *token = get_token(context);
            if (token->type == CPP_TOKEN_IDENTIFIER && !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(data + token->start, token->size);
                if (match_ss(lexeme, make_lit_string("API_EXPORT"))){
                    set_token(context, token);
                    function_parse(part, context, data, function_set,
                                   sig_count, string_zero());
                    if (function_set.items[sig_count].t == Item_Null){
                        function_set.items[sig_count] = null_item_node;
                        // TODO(allen): get warning file name and line numbers
                        fprintf(stderr, "generator warning : invalid function signature\n");
                    }
                    ++sig_count;
                }
            }
        }
        
        ++sig_count_per_file[J] = sig_count;
    }
    
    for (int32_t i = 0; i < sig_count; ++i){
        String name_string = function_set.items[i].name;
        String *macro = &func_4ed_names.names[i].macro;
        String *public_name = &func_4ed_names.names[i].public_name;
        
        macro->size = 0;
        macro->memory_size = name_string.size+4;
        
        macro->str = (char*)malloc(macro->memory_size);
        copy_ss(macro, name_string);
        to_upper_s(macro);
        append_ss(macro, make_lit_string("_SIG"));
        
        
        public_name->size = 0;
        public_name->memory_size = name_string.size;
        
        public_name->str = (char*)malloc(public_name->memory_size);
        copy_ss(public_name, name_string);
        to_lower_s(public_name);
    }
    
    // NOTE(allen): Header
    FILE *file = fopen(OS_API_H, "wb");
    
    int32_t main_api_count = sig_count_per_file[0];
    for (int32_t i = main_api_count; i < sig_count; ++i){
        String ret_string   = function_set.items[i].ret;
        String args_string  = function_set.items[i].args;
        String macro_string = func_4ed_names.names[i].macro;
        
        fprintf(file, "#define %.*s(n) %.*s n%.*s\n",
                macro_string.size, macro_string.str,
                ret_string.size, ret_string.str,
                args_string.size, args_string.str);
    }
    
    for (int32_t i = main_api_count; i < sig_count; ++i){
        String name_string  = function_set.items[i].name;
        String macro_string = func_4ed_names.names[i].macro;
        
        fprintf(file, "typedef %.*s(%.*s_Function);\n",
                macro_string.size, macro_string.str,
                name_string.size, name_string.str);
    }
    
    fclose(file);
    
    file = fopen(API_H, "wb");
    
    for (int32_t i = 0; i < sig_count; ++i){
        String ret_string   = function_set.items[i].ret;
        String args_string  = function_set.items[i].args;
        String macro_string = func_4ed_names.names[i].macro;
        
        fprintf(file, "#define %.*s(n) %.*s n%.*s\n",
                macro_string.size, macro_string.str,
                ret_string.size, ret_string.str,
                args_string.size, args_string.str);
    }
    
    for (int32_t i = 0; i < sig_count; ++i){
        String name_string  = function_set.items[i].name;
        String macro_string = func_4ed_names.names[i].macro;
        
        fprintf(file, "typedef %.*s(%.*s_Function);\n",
                macro_string.size, macro_string.str,
                name_string.size, name_string.str);
    }
    
    fprintf(file, "struct Application_Links{\n");
    fprintf(file,
            "    void *memory;\n"
            "    int32_t memory_size;\n"
            );
    for (int32_t i = 0; i < sig_count; ++i){
        String name_string  = function_set.items[i].name;
        String public_string = func_4ed_names.names[i].public_name;
        
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
        String name = function_set.items[i].name;
        String public_string = func_4ed_names.names[i].public_name;
        
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
        static char *type_files[] = {
            "4coder_types.h"
        };
        
        static Meta_Keywords type_spec_keys[] = {
            {make_lit_string("typedef") , Item_Typedef } ,
            {make_lit_string("struct")  , Item_Struct  } ,
            {make_lit_string("union")   , Item_Union   } ,
            {make_lit_string("ENUM")    , Item_Enum    } ,
        };
        
        Meta_Unit unit = compile_meta_unit(part, type_files, ArrayCount(type_files),
                                           type_spec_keys, ArrayCount(type_spec_keys));
        
        //
        // Output 4coder_string.h
        //
        
        file = fopen(STRING_H, "wb");
        
        {
            String *code = &string_parse.code;
            Cpp_Token_Stack *token_stack = &string_parse.tokens;
            
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
                        
                        if (string_function_set.items[j].t != Item_Macro){
                            String marker = string_function_set.items[j].marker;
                            String ret    = string_function_set.items[j].ret;
                            String name   = string_function_set.items[j].name;
                            String args   = string_function_set.items[j].args;
                                                              
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
                            String name = string_function_set.items[j].name;
                            String args = string_function_set.items[j].args;
                            String body = string_function_set.items[j].body;
                            
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
                            
                            if (string_function_set.items[j].t != Item_Macro){
                                String cpp_name = string_function_set.items[j].cpp_name;
                                if (cpp_name.str != 0){
                                    String ret = string_function_set.items[j].ret;
                                    String args = string_function_set.items[j].args;
                                    
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
                            
                            if (string_function_set.items[j].t != Item_Macro){
                                String cpp_name = string_function_set.items[j].cpp_name;
                                if (cpp_name.str != 0){
                                    String name = string_function_set.items[j].name;
                                    String ret  = string_function_set.items[j].ret;
                                    String args = string_function_set.items[j].args;
                                Argument_Breakdown breakdown = string_function_set.items[j].breakdown;
                                    
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
                                            append_ss(&line, breakdown.args[i].param_name);
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
                String name = func_4ed_names.names[i].public_name;
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
            
            for (int32_t i = 0; i < unit.set.count; ++i){
                String name = unit.set.items[i].name;
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
                String name = func_4ed_names.names[i].public_name;
                
                fprintf(file,
                        "<div id='%.*s_doc' style='margin-bottom: 1cm;'>\n"
                        "<h4>&sect;"SECTION".%d: %.*s</h4>\n"
                        "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>",
                        name.size, name.str, i+1,
                        name.size, name.str
                        );
                print_function_html(file, function_set.items[i], name, "app->");
                fprintf(file, "</div>\n");
                
                String doc_string = function_set.items[i].doc_string;
                print_function_docs(file, part, name, doc_string);
                
                fprintf(file, "</div><hr>\n");
            }
            
#undef SECTION
#define SECTION MAJOR_SECTION".4"
            
            fprintf(file, "<h3>&sect;"SECTION" Type Descriptions</h3>\n");
            int32_t I = 1;
            for (int32_t i = 0; i < unit.set.count; ++i, ++I){
                print_item(part, file, unit.set.items + i, SECTION, I);
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
                String name = string_function_set.items[i].name;
                int32_t index = 0;
                if (!string_set_match(used_strings, used_string_count, name, &index)){
                    fprintf(file,
                            "<li><a href='#%.*s_str_doc'>%.*s</a></li>\n",
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
                String name = string_function_set.items[i].name;
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
                
                if (string_function_set.items[i].t == Item_Macro){
                    print_macro_html(file, string_function_set.items[i], name);
                }
                else{
                    print_function_html(file, string_function_set.items[i], name, "");
                }
                
                fprintf(file, "</div>\n");
                
                
                String doc_string = string_function_set.items[i].doc_string;
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
}

int main(int argc, char **argv){
    generate_keycode_enum();
    generate_style();
    generate_custom_headers();
}

// BOTTOM

