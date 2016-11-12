/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.11.2016
 *
 * meta-compilation core system
 *
 */

// TOP

#if !defined(META_PARSER_CPP_4CODER)
#define META_PARSER_CPP_4CODER

#if !defined(FSTRING_GUARD)
#include "internal_4coder_string.cpp"
#endif

#include "4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "4coder_mem.h"

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
str_start_end(char *data, int32_t start, int32_t end){
    return(make_string(data + start, end - start));
}

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
str_alloc(Partition *part, int32_t cap){
    return(make_string_cap(push_array(part, char, cap), 0, cap));
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
        
        String type_postfix = skip_chop_whitespace(str_start_end(context->data, token_j->start + token_j->size, token->start));
        
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
            
            if (match_ss(lexeme, make_lit_string("STRUCT"))){
                Item_Node *member = push_struct(part, Item_Node);
                if (struct_parse(part, true, context, member)){
                    result = member;
                    break;
                }
                else{
                    assert(!"unhandled error");
                }
            }
            else if (match_ss(lexeme, make_lit_string("UNION"))){
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
        
        String type = skip_chop_whitespace(str_start_end(context->data, start_token->start + start_token->size, token_j->start));
        
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
                            
                            value = skip_chop_whitespace(str_start_end(context->data, start_token->start + start_token->size, token->start));
                            
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
        
        item->ret = chop_whitespace(str_start_end(context->data, ret_token->start, token->start));
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                break;
            }
        }
        
        if (token){
            item->args = str_start_end(context->data, args_start_token->start, token->start + token->size);
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
                            item->args = str_start_end(context->data, args_start_token->start, token->start + token->size);
                            
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
                                    
                                    item->body = str_start_end(context->data, body_start->start,token->start + token->size);
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
compile_meta_unit(Partition *part, char *code_directory, char **files, Meta_Keywords *keywords, int32_t key_count){
    Meta_Unit unit = {0};
    
    int32_t file_count = 0;
    for (char **file_ptr = files; *file_ptr; ++file_ptr, ++file_count);
    
    unit.count = file_count;
    unit.parse = push_array(part, Parse, file_count);
    
    int32_t i = 0;
    for (char **file_ptr = files; *file_ptr; ++file_ptr, ++i){
        char str_space[512];
        String name = make_fixed_width_string(str_space);
        append_sc(&name, code_directory);
#ifdef _WIN32
        append_sc(&name, "\\");
#else
        append_sc(&name, "/");
#endif
        append_sc(&name, *file_ptr);
        terminate_with_null(&name);
        
        unit.parse[i] = meta_lex(name.str);
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

static Meta_Unit
compile_meta_unit(Partition *part, char *code_directory, char *file, Meta_Keywords *keywords, int32_t key_count){
    char *file_array[2] = {file, 0};
    Meta_Unit unit = compile_meta_unit(part, code_directory, file_array, keywords, key_count);
    return(unit);
}

#endif

// BOTTOM

