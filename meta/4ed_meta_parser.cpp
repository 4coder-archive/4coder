/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.11.2016
 *
 * meta-compilation core system
 *
 */

// TOP

#if !defined(FRED_META_PARSER_CPP)
#define FRED_META_PARSER_CPP

struct Parse_Context{
    Cpp_Token *token_s;
    Cpp_Token *token_e;
    Cpp_Token *token;
    char *data;
};

struct Argument{
    String_Const_char param_string;
    String_Const_char param_name;
    //String param_string;
    //String param_name;
};

struct Argument_Breakdown{
    i32 count;
    Argument *args;
};

struct Documentation{
    i32 param_count;
    String_Const_char *param_name;
    String_Const_char *param_docs;
    String_Const_char return_doc;
    String_Const_char main_doc;
    i32 see_also_count;
    String_Const_char *see_also;
    //String *param_name;
    //String *param_docs;
    //String return_doc;
    //String main_doc;
    //String *see_also;
};

enum Item_Type{
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
};

struct Item_Node{
    i32 t;
    
    String_Const_char cpp_name;
    String_Const_char name;
    String_Const_char ret;
    String_Const_char args;
    String_Const_char body;
    String_Const_char marker;
    
    String_Const_char value;
    String_Const_char type;
    String_Const_char type_postfix;
    String_Const_char doc_string;
    
    //String cpp_name;
    //String name;
    //String ret;
    //String args;
    //String body;
    //String marker;
    
    //String value;
    //String type;
    //String type_postfix;
    //String doc_string;
    
    Argument_Breakdown breakdown;
    Documentation doc;
    
    Item_Node *first_child;
    Item_Node *next_sibling;
};

struct Item_Set{
    Item_Node *items;
    i32 count;
};

struct Parse{
    String_Const_char code;
    //String code;
    Cpp_Token_Array tokens;
    i32 item_count;
};

struct Meta_Unit{
    Item_Set set;
    Parse *parse;
    i32 count;
};

struct Meta_Keywords{
    String_Const_char key;
    //String key;
    Item_Type type;
};

struct Used_Links{
    String_Const_char *strs;
    //String *strs;
    i32 count, max;
};

internal Item_Node null_item_node = {};

internal String_Const_char
SCchar_range(char *data, i32 start, i32 end){
    return(SCchar(data + start, data + end));
}

internal String_Const_char
get_lexeme(Cpp_Token token, char *code){
    String_Const_char str = SCchar(code + token.start, token.size);
    return(str);
}

internal Parse_Context
setup_parse_context(char *data, Cpp_Token_Array array){
    Parse_Context context;
    context.token_s = array.tokens;
    context.token_e = array.tokens + array.count;
    context.token = context.token_s;
    context.data = data;
    return(context);
}

internal Parse_Context
setup_parse_context(Parse parse){
    Parse_Context context;
    context.token_s = parse.tokens.tokens;
    context.token_e = parse.tokens.tokens + parse.tokens.count;
    context.token = context.token_s;
    context.data = parse.code.str;
    return(context);
}

internal Cpp_Token*
get_token(Parse_Context *context){
    Cpp_Token *result = context->token;
    if (result >= context->token_e){
        result = 0;
    }
    return(result);
}

internal Cpp_Token*
get_next_token(Parse_Context *context){
    Cpp_Token *result = context->token+1;
    context->token = result;
    if (result >= context->token_e){
        result = 0;
        context->token = context->token_e;
    }
    return(result);
}

internal Cpp_Token*
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

internal Cpp_Token*
can_back_step(Parse_Context *context){
    Cpp_Token *result = context->token-1;
    if (result < context->token_s){
        result = 0;
    }
    return(result);
}

internal Cpp_Token*
set_token(Parse_Context *context, Cpp_Token *token){
    Cpp_Token *result = 0;
    if (token >= context->token_s && token < context->token_e){
        context->token = token;
        result = token;
    }
    return(result);
}

internal Item_Set
allocate_item_set(Arena *arena, i32 count){
    Item_Set item_set = {};
    if (count > 0){
        item_set.items = push_array(arena, Item_Node, count);
        item_set.count = count;
        memset(item_set.items, 0, sizeof(Item_Node)*count);
    }
    return(item_set);
}

internal String_Const_char
file_dump(char *filename){
    String_Const_char result = {};
    FILE *file = fopen(filename, "rb");
    if (file){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        result.str = (char*)malloc(result.size + 1);
        fread(result.str, 1, result.size, file);
        result.str[result.size] = 0;
        fclose(file);
    }
    return(result);
}

internal Parse
meta_lex(char *filename){
    Parse result = {};
    result.code = file_dump(filename);
    if (result.code.str != 0){
        result.tokens = cpp_make_token_array(1024);
        cpp_lex_file(result.code.str, (i32)result.code.size, &result.tokens);
    }
    return(result);
}

internal String_Const_char
get_first_line(String_Const_char source){
    umem pos = string_find_first(source, '\n');
    String_Const_char line = string_prefix(source, pos);
    return(line);
}

internal String_Const_char
get_next_line(String_Const_char source, String_Const_char line){
    String_Const_char next = {};
    umem pos = (umem)(line.str - source.str) + line.size;
    umem start = 0;
    
    if (pos < source.size){
        Assert(source.str[pos] == '\n');
        start = pos + 1;
        
        if (start < source.size){
            pos = string_find_first(string_skip(source, start), '\n');
            next = string_prefix(string_skip(source, start), pos - start);
        }
    }
    
    return(next);
}

internal b32
is_comment(String_Const_char str){
    b32 result = string_match(string_prefix(str, 2), string_litexpr("//"));
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

internal String_Const_char
defined_doc_notes[] = {
    string_litinit("DOC_PARAM"),
    string_litinit("DOC_RETURN"),
    string_litinit("DOC"),
    string_litinit("DOC_SEE"),
    string_litinit("DOC_HIDE"),
    string_litinit("HIDE_MEMBERS"),
};

internal b32
check_and_fix_docs(String_Const_char *doc_string){
    b32 result = false;
    
    if (doc_string->size > 4){
        if (doc_string->str[0] == '/'){
            if (doc_string->str[1] == '*'){
                if (doc_string->str[doc_string->size - 2] == '*'){
                    if (doc_string->str[doc_string->size - 1] == '/'){
                        result = true;
                        *doc_string = string_skip(string_chop(*doc_string, 2), 2);
                    }
                }
            }
        }
    }
    
    return(result);
}

internal i32
get_doc_string_from_prev(Parse_Context *context, String_Const_char *doc_string){
    i32 result = false;
    
    if (can_back_step(context)){
        Cpp_Token *prev_token = get_token(context) - 1;
        if (prev_token->type == CPP_TOKEN_COMMENT){
            *doc_string = get_lexeme(*prev_token, context->data);
            if (check_and_fix_docs(doc_string)){
                result = true;
            }
            else{
                block_zero_struct(doc_string);
            }
        }
    }
    
    return(result);
}

internal String_Const_char
doc_parse_note(String_Const_char source, i32 *pos){
    String_Const_char result = {};
    
    i32 p = *pos;
    i32 start = p;
    for (; p < source.size; ++p){
        if (source.str[p] == '('){
            break;
        }
    }
    if (p != source.size){
        result = SCchar(source.str + start, source.str + p);
        result = string_skip_chop_whitespace(result);
    }
    *pos = p;
    
    return(result);
}

internal String_Const_char
doc_parse_note_string(String_Const_char source, i32 *pos){
    String_Const_char result = {};
    
    Assert(source.str[*pos] == '(');
    
    i32 p = *pos + 1;
    i32 start = p;
    
    i32 nest_level = 0;
    
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
        result = SCchar(source.str + start, source.str + p);
        result = string_skip_chop_whitespace(result);
        ++p;
    }
    *pos = p;
    
    return(result);
}

internal String_Const_char
doc_parse_parameter(String_Const_char source, i32 *pos){
    String_Const_char result = {};
    
    i32 p = *pos;
    i32 start = p;
    
    for (; p < source.size; ++p){
        if (source.str[p] == ','){
            break;
        }
    }
    if (p != source.size){
        result = SCchar(source.str + start, source.str + start + p);
        result = string_skip_chop_whitespace(result);
        ++p;
    }
    *pos = p;
    
    return(result);
}

internal String_Const_char
doc_parse_last_parameter(String_Const_char source, i32 *pos){
    String_Const_char result = {};
    
    i32 p = *pos;
    i32 start = p;
    
    for (; p < source.size; ++p){
        if (source.str[p] == ')'){
            break;
        }
    }
    if (p == source.size){
        result = SCchar(source.str + start, source.str + p);
        result = string_skip_chop_whitespace(result);
    }
    *pos = p;
    
    return(result);
}

internal b32
string_set_match_table(void *string_array, umem item_size, i32 count, String_Const_char needle, i32 *index_out){
    b32 result = false;
    u8 *ptr = (u8*)string_array;
    for (i32 i = 0; i < count; i += 1, ptr += item_size){
        String_Const_char *string_ptr = (String_Const_char*)ptr;
        if (string_match(*string_ptr, needle)){
            *index_out = i;
            result = true;
            break;
        }
    }
    return(result);
}

internal b32
string_set_match(String_Const_char *string_array, i32 count, String_Const_char needle, i32 *index_out){
    return(string_set_match_table(string_array, sizeof(*string_array), count, needle, index_out));
}
internal void
perform_doc_parse(Arena *arena, String_Const_char doc_string, Documentation *doc){
    i32 keep_parsing = true;
    i32 pos = 0;
    
    i32 param_count = 0;
    i32 see_count = 0;
    
    do{
        String_Const_char doc_note = doc_parse_note(doc_string, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            i32 doc_note_type;
            if (string_set_match(defined_doc_notes, ArrayCount(defined_doc_notes), doc_note, &doc_note_type)){
                
                doc_parse_note_string(doc_string, &pos);
                
                switch (doc_note_type){
                    case DOC_PARAM: ++param_count; break;
                    case DOC_SEE: ++see_count; break;
                }
            }
        }
    }while(keep_parsing);
    
    if (param_count + see_count > 0){
        doc->param_name = push_array(arena, String_Const_char, param_count);
        doc->param_docs = push_array(arena, String_Const_char, param_count);
        doc->see_also   = push_array(arena, String_Const_char, see_count);
        doc->param_count = param_count;
        doc->see_also_count = see_count;
    }
    
    i32 param_index = 0;
    i32 see_index = 0;
    
    keep_parsing = true;
    pos = 0;
    do{
        String_Const_char doc_note = doc_parse_note(doc_string, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            i32 doc_note_type;
            if (string_set_match(defined_doc_notes, ArrayCount(defined_doc_notes), doc_note, &doc_note_type)){
                
                String_Const_char doc_note_string = doc_parse_note_string(doc_string, &pos);
                
                switch (doc_note_type){
                    case DOC_PARAM:
                    {
                        Assert(param_index < param_count);
                        i32 param_pos = 0;
                        String_Const_char param_name = doc_parse_parameter(doc_note_string, &param_pos);
                        String_Const_char param_docs = doc_parse_last_parameter(doc_note_string, &param_pos);
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
                        Assert(see_index < see_count);
                        doc->see_also[see_index++] = doc_note_string;
                    }break;
                }
            }
            else{
                fprintf(stderr, "warning: invalid doc note %.*s\n", (i32)doc_note.size, doc_note.str);
            }
        }
    }while(keep_parsing);
}

internal i32
struct_parse(Arena *arena, i32 is_struct, Parse_Context *context, Item_Node *top_member);

internal i32
struct_parse_member(Parse_Context *context, Item_Node *member){
    i32 result = false;
    
    Cpp_Token *token = get_token(context);
    
    String_Const_char doc_string = {};
    get_doc_string_from_prev(context, &doc_string);
    
    Cpp_Token *start_token = token;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_SEMICOLON){
            break;
        }
    }
    
    if (token){
        String_Const_char name = {};
        Cpp_Token *token_j = 0;
        i32 nest_level = 0;
        
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
        
        name = string_skip_chop_whitespace(get_lexeme(*token_j, context->data));
        
        String_Const_char type = string_skip_chop_whitespace(SCchar_range(context->data, start_token->start, token_j->start));
        
        String_Const_char type_postfix = string_skip_chop_whitespace(SCchar_range(context->data, token_j->start + token_j->size, token->start));
        
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

internal Item_Node*
struct_parse_next_member(Arena *arena, Parse_Context *context){
    Item_Node *result = 0;
    
    Cpp_Token *token = 0;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_IDENTIFIER ||
            (token->flags & CPP_TFLAG_IS_KEYWORD)){
            String_Const_char lexeme = get_lexeme(*token, context->data);
            
            if (string_match(lexeme, string_litexpr("STRUCT"))){
                Item_Node *member = push_array(arena, Item_Node, 1);
                if (struct_parse(arena, true, context, member)){
                    result = member;
                    break;
                }
                else{
                    Assert(!"unhandled error");
                }
            }
            else if (string_match(lexeme, string_litexpr("UNION"))){
                Item_Node *member = push_array(arena, Item_Node, 1);
                if (struct_parse(arena, false, context, member)){
                    result = member;
                    break;
                }
                else{
                    Assert(!"unhandled error");
                }
            }
            else{
                Item_Node *member = push_array(arena, Item_Node, 1);
                if (struct_parse_member(context, member)){
                    result = member;
                    break;
                }
                else{
                    Assert(!"unhandled error");
                }
            }
        }
        else if (token->type == CPP_TOKEN_BRACE_CLOSE){
            break;
        }
    }
    
    return(result);
}

internal i32
struct_parse(Arena *arena, i32 is_struct, Parse_Context *context, Item_Node *top_member){
    i32 result = false;
    
    Cpp_Token *start_token = get_token(context);
    Cpp_Token *token = 0;
    
    String_Const_char doc_string = {};
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
        
        String_Const_char name = {};
        if (token_j != start_token){
            name = string_skip_chop_whitespace(get_lexeme(*token_j, context->data));
        }
        
        String_Const_char type = {};
        if (is_struct){
            type = string_litexpr("struct");
        }
        else{
            type = string_litexpr("union");
        }
        
        set_token(context, token+1);
        Item_Node *new_member = struct_parse_next_member(arena, context);
        
        if (new_member){
            top_member->first_child = new_member;
            
            Item_Node *head_member = new_member;
            for(;;){
                new_member = struct_parse_next_member(arena, context);
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

internal i32
typedef_parse(Parse_Context *context, Item_Node *item){
    i32 result = false;
    
    Cpp_Token *token = get_token(context);
    String_Const_char doc_string = {};
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
        
        String_Const_char name = get_lexeme(*token_j, context->data);
        
        String_Const_char type = string_skip_chop_whitespace(SCchar_range(context->data, start_token->start + start_token->size, token_j->start));
        
        item->t = Item_Typedef;
        item->type = type;
        item->name = name;
        item->doc_string = doc_string;
        result = true;
    }
    
    set_token(context, token);
    
    return(result);
}

internal i32
enum_parse(Arena *arena, Parse_Context *context, Item_Node *item){
    i32 result = false;
    
    String_Const_char parent_doc_string = {};
    get_doc_string_from_prev(context, &parent_doc_string);
    
    Cpp_Token *parent_start_token = get_token(context);
    Cpp_Token *token = 0;
    
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (token->type == CPP_TOKEN_BRACE_OPEN){
            break;
        }
    }
    
    if (token){
        String_Const_char parent_name = {};
        Cpp_Token *token_j = 0;
        
        for (; (token_j = get_token(context)) != 0; get_prev_token(context)){
            if (token_j->type == CPP_TOKEN_IDENTIFIER){
                break;
            }
        }
        
        parent_name = get_lexeme(*token_j, context->data);
        
        set_token(context, token);
        for (; (token = get_token(context)) > parent_start_token; get_next_token(context)){
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
                    String_Const_char doc_string = {};
                    String_Const_char name = {};
                    String_Const_char value = {};
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
                            
                            value = string_skip_chop_whitespace(SCchar_range(context->data, start_token->start + start_token->size, token->start));
                            
                            get_prev_token(context);
                        }
                        else{
                            get_prev_token(context);
                        }
                    }
                    
                    Item_Node *new_member = push_array(arena, Item_Node, 1);
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
                item->name = parent_name;
                item->doc_string = parent_doc_string;
                item->first_child = first_member;
                result = true;
            }
        }
    }
    
    return(result);
}

internal Argument_Breakdown
allocate_argument_breakdown(Arena *arena, i32 count){
    Argument_Breakdown breakdown = {};
    if (count > 0){
        breakdown.count = count;
        breakdown.args = push_array(arena, Argument, count);
        memset(breakdown.args, 0, sizeof(Argument)*count);
    }
    return(breakdown);
}

/*
Parse arguments by giving pointers to the tokens:
foo(a, ... , z)
   ^          ^
*/
internal Argument_Breakdown
parameter_parse(Arena *arena, char *data, Cpp_Token *args_start_token, Cpp_Token *args_end_token){
    i32 arg_index = 0;
    Cpp_Token *arg_token = args_start_token + 1;
    i32 param_string_start = arg_token->start;
    
    i32 arg_count = 1;
    arg_token = args_start_token;
    for (; arg_token < args_end_token; ++arg_token){
        if (arg_token->type == CPP_TOKEN_COMMA){
            ++arg_count;
        }
    }
    
    Argument_Breakdown breakdown = allocate_argument_breakdown(arena, arg_count);
    
    arg_token = args_start_token + 1;
    for (; arg_token <= args_end_token; ++arg_token){
        if (arg_token->type == CPP_TOKEN_COMMA ||
            arg_token->type == CPP_TOKEN_PARENTHESE_CLOSE){
            
            i32 size = arg_token->start - param_string_start;
            String_Const_char param_string = SCchar(data + param_string_start, size);
            param_string = string_chop_whitespace(param_string);
            breakdown.args[arg_index].param_string = param_string;
            
            for (Cpp_Token *param_name_token = arg_token - 1;
                 param_name_token->start > param_string_start;
                 --param_name_token){
                if (param_name_token->type == CPP_TOKEN_IDENTIFIER){
                    i32 name_start = param_name_token->start;
                    i32 name_size = param_name_token->size;
                    breakdown.args[arg_index].param_name = SCchar(data + name_start, name_size);
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
internal i32
function_parse_goto_name(Parse_Context *context){
    i32 result = false;
    
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
internal i32
function_get_doc(Parse_Context *context, char *data, String_Const_char *doc_string){
    i32 result = false;
    
    Cpp_Token *token = get_token(context);
    String_Const_char lexeme = {};
    
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

internal i32
cpp_name_parse(Parse_Context *context, String_Const_char *name){
    i32 result = false;
    
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
internal i32
function_sig_parse(Arena *arena, Parse_Context *context, Item_Node *item, String_Const_char cpp_name){
    i32 result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *args_start_token = 0;
    Cpp_Token *ret_token = get_token(context);
    
    if (function_parse_goto_name(context)){
        token = get_token(context);
        args_start_token = token+1;
        item->name = get_lexeme(*token, context->data);
        
        item->ret = string_chop_whitespace(SCchar_range(context->data, ret_token->start, token->start));
        
        for (; (token = get_token(context)) != 0; get_next_token(context)){
            if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                break;
            }
        }
        
        if (token){
            item->args = SCchar_range(context->data, args_start_token->start, token->start + token->size);
            item->t = Item_Function;
            item->cpp_name = cpp_name;
            item->breakdown = parameter_parse(arena, context->data, args_start_token, token);
            
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
internal i32
function_parse(Arena *arena, Parse_Context *context, Item_Node *item, String_Const_char cpp_name){
    i32 result = false;
    
    String_Const_char doc_string = {};
    Cpp_Token *token = get_token(context);
    
    item->marker = get_lexeme(*token, context->data);
    
    if (function_get_doc(context, context->data, &doc_string)){
        item->doc_string = doc_string;
    }
    
    set_token(context, token);
    if (get_next_token(context)){
        if (function_sig_parse(arena, context, item, cpp_name)){
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
internal i32
macro_parse_check(Parse_Context *context){
    i32 result = false;
    
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
internal i32
macro_parse(Arena *arena, Parse_Context *context, Item_Node *item){
    i32 result = false;
    
    Cpp_Token *token = 0;
    Cpp_Token *doc_token = 0;
    Cpp_Token *args_start_token = 0;
    
    String_Const_char doc_string = {};
    
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
                            item->args = SCchar_range(context->data, args_start_token->start, token->start + token->size);
                            
                            item->breakdown = parameter_parse(arena, context->data, args_start_token, token);
                            
                            if ((token = get_next_token(context)) != 0){
                                Cpp_Token *body_start = token;
                                
                                if (body_start->flags & CPP_TFLAG_PP_BODY){
                                    for (; (token = get_token(context)) != 0; get_next_token(context)){
                                        if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                            break;
                                        }
                                    }
                                    
                                    token = get_prev_token(context);
                                    
                                    item->body = SCchar_range(context->data, body_start->start,token->start + token->size);
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

internal Meta_Unit
compile_meta_unit(Arena *arena, char *code_directory, char **files, Meta_Keywords *meta_keywords, i32 key_count){
    Meta_Unit unit = {};
    
    i32 file_count = 0;
    for (char **file_ptr = files; *file_ptr; ++file_ptr, ++file_count);
    
    unit.count = file_count;
    unit.parse = push_array(arena, Parse, file_count);
    
    b32 all_files_lexed = true;
    i32 i = 0;
    for (char **file_ptr = files; *file_ptr; ++file_ptr, ++i){
        List_String_Const_char name_list = {};
        string_list_push(arena, &name_list, SCchar(code_directory));
#if OS_WINDOWS
        string_list_push_lit(arena, &name_list, "\\");
#else
        string_list_push_lit(arena, &name_list, "/");
#endif
        string_list_push(arena, &name_list, SCchar(*file_ptr));
        String_Const_char name = string_list_flatten(arena, name_list, StringFill_NullTerminate);
        
        unit.parse[i] = meta_lex(name.str);
        if (unit.parse[i].code.str == 0){
            all_files_lexed = false;
            break;
        }
    }
    
    if (all_files_lexed){
        // TODO(allen): This stage counts nested structs and unions which is not correct.  Luckily it only means we over allocate by a few items, but fixing it to be exactly correct would be nice.
        for (i32 J = 0; J < unit.count; ++J){
            Cpp_Token *token = 0;
            Parse_Context context_ = setup_parse_context(unit.parse[J]);
            Parse_Context *context = &context_;
            
            for (; (token = get_token(context)) != 0; get_next_token(context)){
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    
                    String_Const_char lexeme = get_lexeme(*token, context->data);
                    i32 match_index = 0;
                    if (string_set_match_table(meta_keywords, sizeof(*meta_keywords), key_count, lexeme, &match_index)){
                        Item_Type type = meta_keywords[match_index].type;
                        
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
            unit.set = allocate_item_set(arena, unit.set.count);
        }
        
        i32 index = 0;
        
        for (i32 J = 0; J < unit.count; ++J){
            Cpp_Token *token = 0;
            Parse_Context context_ = setup_parse_context(unit.parse[J]);
            Parse_Context *context = &context_;
            
            String_Const_char cpp_name = {};
            i32 has_cpp_name = 0;
            
            for (; (token = get_token(context)) != 0; get_next_token(context)){
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    
                    String_Const_char lexeme = get_lexeme(*token, context->data);
                    i32 match_index = 0;
                    if (string_set_match_table(meta_keywords, sizeof(*meta_keywords), key_count, lexeme, &match_index)){
                        Item_Type type = meta_keywords[match_index].type;
                        
                        switch (type){
                            case Item_Function:
                            {
                                if (function_parse(arena, context, unit.set.items + index, cpp_name)){
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
                                if (macro_parse(arena, context, unit.set.items + index)){
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
                                if (struct_parse(arena, (type == Item_Struct), context, unit.set.items + index)){
                                    Assert(unit.set.items[index].t == Item_Struct ||unit.set.items[index].t == Item_Union);
                                    ++index;
                                }
                                else{
                                    // TODO(allen): warning message
                                }
                            }break;
                            
                            case Item_Enum: //ENUM
                            {
                                if (enum_parse(arena, context, unit.set.items + index)){
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
                    block_zero_struct(&cpp_name);
                }
                
                unit.parse[J].item_count = index;
            }
            
            // NOTE(allen): This is necessary for now because
            // the original count is slightly overestimated thanks
            // to nested structs and unions.
            unit.set.count = index;
        }
    }
    else{
        unit.parse = 0;
        unit.count = 0;
    }
    
    return(unit);
}

internal Meta_Unit
compile_meta_unit(Arena *arena, char *code_directory, char *file, Meta_Keywords *meta_keywords, i32 key_count){
    char *file_array[2] = {file, 0};
    return(compile_meta_unit(arena, code_directory, file_array, meta_keywords, key_count));
}

#endif

// BOTTOM

