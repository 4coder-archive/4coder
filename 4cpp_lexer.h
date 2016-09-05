
// TOP

#ifndef FCPP_NEW_LEXER_INC
#define FCPP_NEW_LEXER_INC

#ifndef Assert
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif

#ifndef FCPP_LINK
# define FCPP_LINK static
#endif

#include "4cpp_lexer_types.h"
#include "4cpp_lexer_fsms.h"
#include "4cpp_lexer_tables.c"

// TODO(allen): revisit this keyword data declaration system
struct String_And_Flag{
    char *str;
    uint32_t flags;
};

struct String_List{
	String_And_Flag *data;
	int32_t count;
};

#define lexer_string_list(x) {x, (sizeof(x)/sizeof(*(x)))}

static String_And_Flag preprop_strings[] = {
	{"include", CPP_PP_INCLUDE},
	{"INCLUDE", CPP_PP_INCLUDE},
	{"ifndef", CPP_PP_IFNDEF},
	{"IFNDEF", CPP_PP_IFNDEF},
	{"define", CPP_PP_DEFINE},
	{"DEFINE", CPP_PP_DEFINE},
	{"import", CPP_PP_IMPORT},
	{"IMPORT", CPP_PP_IMPORT},
	{"pragma", CPP_PP_PRAGMA},
	{"PRAGMA", CPP_PP_PRAGMA},
	{"undef", CPP_PP_UNDEF},
	{"UNDEF", CPP_PP_UNDEF},
	{"endif", CPP_PP_ENDIF},
	{"ENDIF", CPP_PP_ENDIF},
	{"error", CPP_PP_ERROR},
	{"ERROR", CPP_PP_ERROR},
	{"ifdef", CPP_PP_IFDEF},
	{"IFDEF", CPP_PP_IFDEF},
	{"using", CPP_PP_USING},
	{"USING", CPP_PP_USING},
	{"else", CPP_PP_ELSE},
	{"ELSE", CPP_PP_ELSE},
	{"elif", CPP_PP_ELIF},
	{"ELIF", CPP_PP_ELIF},
	{"line", CPP_PP_LINE},
	{"LINE", CPP_PP_LINE},
	{"if", CPP_PP_IF},
    {"IF", CPP_PP_IF},
};
static String_List preprops = lexer_string_list(preprop_strings);

static String_And_Flag keyword_strings[] = {
    {"true", CPP_TOKEN_BOOLEAN_CONSTANT},
    {"false", CPP_TOKEN_BOOLEAN_CONSTANT},
    
    {"and", CPP_TOKEN_AND},
    {"and_eq", CPP_TOKEN_ANDEQ},
    {"bitand", CPP_TOKEN_BIT_AND},
    {"bitor", CPP_TOKEN_BIT_OR},
    {"or", CPP_TOKEN_OR},
    {"or_eq", CPP_TOKEN_OREQ},
    {"sizeof", CPP_TOKEN_SIZEOF},
    {"alignof", CPP_TOKEN_ALIGNOF},
    {"decltype", CPP_TOKEN_DECLTYPE},
    {"throw", CPP_TOKEN_THROW},
    {"new", CPP_TOKEN_NEW},
    {"delete", CPP_TOKEN_DELETE},
    {"xor", CPP_TOKEN_BIT_XOR},
    {"xor_eq", CPP_TOKEN_XOREQ},
    {"not", CPP_TOKEN_NOT},
    {"not_eq", CPP_TOKEN_NOTEQ},
    {"typeid", CPP_TOKEN_TYPEID},
    {"compl", CPP_TOKEN_BIT_NOT},
    
    {"void", CPP_TOKEN_KEY_TYPE},
    {"bool", CPP_TOKEN_KEY_TYPE},
    {"char", CPP_TOKEN_KEY_TYPE},
    {"int", CPP_TOKEN_KEY_TYPE},
    {"float", CPP_TOKEN_KEY_TYPE},
    {"double", CPP_TOKEN_KEY_TYPE},
    
    {"long", CPP_TOKEN_KEY_MODIFIER},
    {"short", CPP_TOKEN_KEY_MODIFIER},
    {"unsigned", CPP_TOKEN_KEY_MODIFIER},
    
    {"const", CPP_TOKEN_KEY_QUALIFIER},
    {"volatile", CPP_TOKEN_KEY_QUALIFIER},
    
    {"asm", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"break", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"case", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"catch", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"continue", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"default", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"do", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"else", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"for", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"goto", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"if", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"return", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"switch", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"try", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"while", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"static_assert", CPP_TOKEN_KEY_CONTROL_FLOW},
    
    {"const_cast", CPP_TOKEN_KEY_CAST},
    {"dynamic_cast", CPP_TOKEN_KEY_CAST},
    {"reinterpret_cast", CPP_TOKEN_KEY_CAST},
    {"static_cast", CPP_TOKEN_KEY_CAST},
    
    {"class", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"enum", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"struct", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"typedef", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"union", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"template", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"typename", CPP_TOKEN_KEY_TYPE_DECLARATION},
    
    {"friend", CPP_TOKEN_KEY_ACCESS},
    {"namespace", CPP_TOKEN_KEY_ACCESS},
    {"private", CPP_TOKEN_KEY_ACCESS},
    {"protected", CPP_TOKEN_KEY_ACCESS},
    {"public", CPP_TOKEN_KEY_ACCESS},
    {"using", CPP_TOKEN_KEY_ACCESS},
    
    {"extern", CPP_TOKEN_KEY_LINKAGE},
    {"export", CPP_TOKEN_KEY_LINKAGE},
    {"inline", CPP_TOKEN_KEY_LINKAGE},
    {"static", CPP_TOKEN_KEY_LINKAGE},
    {"virtual", CPP_TOKEN_KEY_LINKAGE},
    
    {"alignas", CPP_TOKEN_KEY_OTHER},
    {"explicit", CPP_TOKEN_KEY_OTHER},
    {"noexcept", CPP_TOKEN_KEY_OTHER},
    {"nullptr", CPP_TOKEN_KEY_OTHER},
    {"operator", CPP_TOKEN_KEY_OTHER},
    {"register", CPP_TOKEN_KEY_OTHER},
    {"this", CPP_TOKEN_KEY_OTHER},
    {"thread_local", CPP_TOKEN_KEY_OTHER},
};
static String_List keywords = lexer_string_list(keyword_strings);

FCPP_LINK int32_t
sub_match_list(char *chunk, int32_t size, String_List list, int32_t sub_size){
    int32_t result = 0;
    String str_main = {0};
    char *str_check = 0;
    int32_t i = 0, l = 0;
    
    result = -1;
    str_main = make_string(chunk, size);
    if (sub_size > 0){
        str_main = substr(str_main, 0, sub_size);
        for (i = 0; i < list.count; ++i){
            str_check = list.data[i].str;
            if (match_sc(str_main, str_check)){
                result = i;
                break;
            }
        }
    }
    else{
        for (i = 0; i < list.count; ++i){
            str_check = list.data[i].str;
            if (match_part_scl(str_main, str_check, &l)){
                result = i;
                break;
            }
        }
    }
    
    return(result);
}


FCPP_LINK Cpp_Get_Token_Result
cpp_get_token(Cpp_Token_Stack *token_stack, int32_t pos){
    Cpp_Get_Token_Result result = {};
    Cpp_Token *token_array = token_stack->tokens;
    Cpp_Token *token = 0;
	int32_t first = 0;
    int32_t count = token_stack->count;
    int32_t last = count;
    int32_t this_start = 0, next_start = 0;
    
    if (count > 0){
        for (;;){
            result.token_index = (first + last)/2;
            token = token_array + result.token_index;
            
            this_start = token->start;
            
            if (result.token_index + 1 < count){
                next_start = (token + 1)->start;
            }
            else{
                next_start = this_start + token->size;
            }
            if (this_start <= pos && pos < next_start){
                break;
            }
            else if (pos < this_start){
                last = result.token_index;
            }
            else{
                first = result.token_index + 1;
            }
            if (first == last){
                result.token_index = first;
                break;
            }
        }
        
        if (result.token_index == count){
            --result.token_index;
            result.in_whitespace = 1;
        }
        else{
            if (token->start + token->size <= pos){
                result.in_whitespace = 1;
            }
        }
    }
    else{
        result.token_index = -1;
        result.in_whitespace = 1;
    }
	
    return(result);
}

FCPP_LINK void
cpp_shift_token_starts(Cpp_Token_Stack *stack, int32_t from_token_i, int32_t shift_amount){
    Cpp_Token *token = stack->tokens + from_token_i;
    int32_t count = stack->count, i;
    
    for (i = from_token_i; i < count; ++i, ++token){
        token->start += shift_amount;
    }
}

FCPP_LINK Lex_PP_State
cpp_pp_directive_to_state(Cpp_Token_Type type){
    Lex_PP_State result = LSPP_default;
    switch (type){
        case CPP_PP_INCLUDE:
        case CPP_PP_IMPORT:
        case CPP_PP_USING:
        result = LSPP_include;
        break;
        
        case CPP_PP_DEFINE:
        result = LSPP_macro_identifier;
        break;
        
        case CPP_PP_UNDEF:
        case CPP_PP_IFDEF:
        case CPP_PP_IFNDEF:
        result = LSPP_identifier;
        break;
        
        case CPP_PP_IF:
        case CPP_PP_ELIF:
        result = LSPP_body_if;
        break;
        
        case CPP_PP_PRAGMA:
        result = LSPP_body;
        break;
        
        case CPP_PP_LINE:
        result = LSPP_number;
        break;
        
        case CPP_PP_ERROR:
        result = LSPP_error;
        break;
        
        case CPP_PP_UNKNOWN:
        case CPP_PP_ELSE:
        case CPP_PP_ENDIF:
        result = LSPP_junk;
        break;
    }
    return(result);
}

FCPP_LINK int32_t
cpp_place_token_nonalloc(Cpp_Token *out_tokens, int32_t token_i, Cpp_Token token){
    Cpp_Token prev_token = {(Cpp_Token_Type)0};
    
    if (token_i > 0){
        prev_token = out_tokens[token_i - 1];
    }
    
    out_tokens[token_i++] = token;
    
    return(token_i);
}

FCPP_LINK bool
cpp_push_token_nonalloc(Cpp_Token_Stack *out_tokens, Cpp_Token token){
    bool result = 0;
    if (out_tokens->count == out_tokens->max_count){
        out_tokens->count = 
            cpp_place_token_nonalloc(out_tokens->tokens, out_tokens->count, token);
        result = 1;
    }
    return(result);
}

struct Lex_Data{
    char *tb;
    int32_t tb_pos;
    int32_t token_start;
    
    int32_t pos;
    int32_t pos_overide;
    int32_t chunk_pos;
    
    Lex_FSM fsm;
    unsigned char white_done;
    unsigned char pp_state;
    unsigned char completed;
    
    Cpp_Token token;
    
    int32_t __pc__;
};

FCPP_LINK Lex_Data
lex_data_init(char *tb){
    Lex_Data data = {0};
    data.tb = tb;
    return(data);
}


#define DrCase(PC) case PC: goto resumespot_##PC

#define DrYield(PC, n) {                                           \
    token_stack_out->count = token_i;                              \
    *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }

#define DrReturn(n) {                           \
    token_stack_out->count = token_i;           \
    *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

enum Lex_Result{
    LexFinished,
    LexNeedChunk,
    LexNeedTokenMemory,
    LexHitTokenLimit
};

FCPP_LINK int32_t
cpp_lex_nonalloc(Lex_Data *S_ptr,
                 char *chunk, int32_t size,
                 Cpp_Token_Stack *token_stack_out){
    Lex_Data S = *S_ptr;
    
    Cpp_Token *out_tokens = token_stack_out->tokens;
    int32_t token_i = token_stack_out->count;
    int32_t max_token_i = token_stack_out->max_count;
    
    //Pos_Update_Rule pos_update_rule = PUR_none;
    
    char c = 0;
    
    int32_t end_pos = size + S.chunk_pos;
    chunk -= S.chunk_pos;
    
    switch (S.__pc__){
        DrCase(1);
        DrCase(2);
        DrCase(3);
        DrCase(4);
        DrCase(5);
        DrCase(7);
    }
    
    for (;;){
        S.white_done = 0;
        for(;;){
            for (; S.pp_state < LSPP_count && S.pos < end_pos;){
                c = chunk[S.pos++];
                int32_t i = S.pp_state + whitespace_fsm_eq_classes[c];
                S.pp_state = whitespace_fsm_table[i];
            }
            S.white_done = (S.pp_state >= LSPP_count);
            
            if (S.white_done == 0){
                S.chunk_pos += size;
                DrYield(4, LexNeedChunk);
            }
            else break;
        }
        --S.pos;
        if (S.pp_state >= LSPP_count){
            S.pp_state -= LSPP_count;
        }
        
        S.token.state_flags = S.pp_state;
        
        S.token_start = S.pos;
        S.tb_pos = 0;
        S.fsm = null_lex_fsm;
        for(;;){
            {
                unsigned short *eq_classes = get_eq_classes[S.pp_state];
                unsigned char *fsm_table = get_table[S.pp_state];
                
                for (; S.fsm.state < LS_count && S.pos < end_pos;){
                    c = chunk[S.pos++];
                    S.tb[S.tb_pos++] = c;
                    
                    int32_t i = S.fsm.state + eq_classes[c];
                    S.fsm.state = fsm_table[i];
                    S.fsm.multi_line |= multiline_state_table[S.fsm.state];
                }
                S.fsm.emit_token = (S.fsm.state >= LS_count);
            }
            
            if (S.fsm.emit_token == 0){
                S.chunk_pos += size;
                DrYield(3, LexNeedChunk);
            }
            else break;
        }
        
        Assert(S.fsm.emit_token == 1);
        
        if (c == 0){
            S.completed = 1;
        }
        
        if (S.fsm.state >= LS_count){
            S.fsm.state -= LS_count;
        }
        
        switch (S.fsm.state){
            case LS_default:
            switch (c){
                case 0: S.fsm.emit_token = 0; break;
                
#define OperCase(op,t) case op: S.token.type = t; break;
                OperCase('{', CPP_TOKEN_BRACE_OPEN);
                OperCase('}', CPP_TOKEN_BRACE_CLOSE);
                
                OperCase('[', CPP_TOKEN_BRACKET_OPEN);
                OperCase(']', CPP_TOKEN_BRACKET_CLOSE);
                
                OperCase('(', CPP_TOKEN_PARENTHESE_OPEN);
                OperCase(')', CPP_TOKEN_PARENTHESE_CLOSE);
                
                OperCase('~', CPP_TOKEN_TILDE);
                OperCase(',', CPP_TOKEN_COMMA);
                OperCase(';', CPP_TOKEN_SEMICOLON);
                OperCase('?', CPP_TOKEN_TERNARY_QMARK);
                
                OperCase('@', CPP_TOKEN_JUNK);
#undef OperCase
                
                case '\\':
                if (S.pp_state == LSPP_default){
                    S.token.type = CPP_TOKEN_JUNK;
                }
                else{
                    S.pos_overide = S.pos;
                    S.white_done = 0;
                    for (;;){
                        for (; S.white_done == 0 && S.pos < end_pos;){
                            c = chunk[S.pos++];
                            if (!(c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f')){
                                S.white_done = 1;
                            }
                        }
                        
                        if (S.white_done == 0){
                            S.chunk_pos += size;
                            DrYield(1, LexNeedChunk);
                        }
                        else break;
                    }
                    
                    if (c == '\n'){
                        S.fsm.emit_token = 0;
                        S.pos_overide = 0;
                    }
                    else{
                        S.token.type = CPP_TOKEN_JUNK;
                    }
                }
                break;
            }
            
            if (c != '@' && c != '\\'){
                S.token.flags = CPP_TFLAG_IS_OPERATOR;
            }
            break;
            
            case LS_identifier:
            {
                --S.pos;
                
                int32_t word_size = S.pos - S.token_start;
                
                if (S.pp_state == LSPP_body_if){
                    if (match_ss(make_string(S.tb, word_size), make_lit_string("defined"))){
                        S.token.type = CPP_TOKEN_DEFINED;
                        S.token.flags = CPP_TFLAG_IS_OPERATOR | CPP_TFLAG_IS_KEYWORD;
                        break;
                    }
                }
                
                int32_t sub_match = sub_match_list(S.tb, S.tb_pos, keywords, word_size);
                
                if (sub_match != -1){
                    String_And_Flag data = keywords.data[sub_match];
                    S.token.type = (Cpp_Token_Type)data.flags;
                    S.token.flags = CPP_TFLAG_IS_KEYWORD;
                }
                else{
                    S.token.type = CPP_TOKEN_IDENTIFIER;
                    S.token.flags = 0;
                }
            }break;
            
            case LS_pound:
            S.token.flags = 0;
            switch (c){
                case '#': S.token.type = CPP_PP_CONCAT; break;
                default:
                S.token.type = CPP_PP_STRINGIFY;
                --S.pos;
                break;
            }
            break;
            
            case LS_pp:
            {
                S.token.type = CPP_TOKEN_JUNK;
                S.token.flags = 0;
                --S.pos;
            }break;
            
            case LS_ppdef:
            {
                --S.pos;
                
                int32_t word_size = S.pos - S.token_start;
                int32_t pos = S.tb_pos-1;
                int32_t i = 1;
                for (;i < pos; ++i){
                    if (S.tb[i] != ' '){
                        break;
                    }
                }
                
                int32_t sub_match = sub_match_list(S.tb+i, pos-i, preprops, word_size);
                
                if (sub_match != -1){
                    String_And_Flag data = preprops.data[sub_match];
                    S.token.type = (Cpp_Token_Type)data.flags;
                    S.token.flags = CPP_TFLAG_PP_DIRECTIVE;
                    S.pp_state = (unsigned char)cpp_pp_directive_to_state(S.token.type);
                }
                else{
                    S.token.type = CPP_TOKEN_JUNK;
                    S.token.flags = 0;
                }
            }break;
            
            case LS_number:
            case LS_number0:
            case LS_hex:
            S.fsm.int_state = LSINT_default;
            S.fsm.emit_token = 0;
            --S.pos;
            for (;;){
                for (; S.fsm.int_state < LSINT_count && S.pos < end_pos;){
                    c = chunk[S.pos++];
                    S.fsm.int_state = int_fsm_table[S.fsm.int_state + int_fsm_eq_classes[c]];
                }
                S.fsm.emit_token = (S.fsm.int_state >= LSINT_count);
                
                if (S.fsm.emit_token == 0){
                    S.chunk_pos += size;
                    DrYield(5, LexNeedChunk);
                }
                else break;
            }
            --S.pos;
            
            S.token.type = CPP_TOKEN_INTEGER_CONSTANT;
            S.token.flags = 0;
            break;
            
            case LS_float:
            case LS_crazy_float0:
            case LS_crazy_float1:
            S.token.type = CPP_TOKEN_FLOATING_CONSTANT;
            S.token.flags = 0;
            switch (c){
                case 'f': case 'F':
                case 'l': case 'L':break;
                default:
                --S.pos;
                break;
            }
            break;
            
            case LS_char:
            case LS_char_slashed:
            S.token.type = CPP_TOKEN_JUNK;
            if (c == '\''){
                S.token.type = CPP_TOKEN_CHARACTER_CONSTANT;
            }
            S.token.flags = 0;
            break;
            
            case LS_char_multiline:
            S.token.type = CPP_TOKEN_JUNK;
            if (c == '\''){
                S.token.type = CPP_TOKEN_CHARACTER_CONSTANT;
            }
            S.token.flags = CPP_TFLAG_MULTILINE;
            break;
            
            case LS_string:
            case LS_string_slashed:
            S.token.type = CPP_TOKEN_JUNK;
            if (S.pp_state == LSPP_include){
                if (c == '>' || c == '"'){
                    S.token.type = CPP_TOKEN_INCLUDE_FILE;
                }
            }
            else{
                if (c == '"'){
                    S.token.type = CPP_TOKEN_STRING_CONSTANT;
                }
            }
            S.token.flags = 0;
            break;
            
            case LS_string_multiline:
            S.token.type = CPP_TOKEN_JUNK;
            if (c == '"'){
                S.token.type = CPP_TOKEN_STRING_CONSTANT;
            }
            S.token.flags = CPP_TFLAG_MULTILINE;
            break;
            
            case LS_comment_pre:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_DIVEQ; break;
                default:
                S.token.type = CPP_TOKEN_DIV;
                --S.pos;
                break;
            }
            break;
            
            case LS_comment:
            case LS_comment_slashed:
            S.token.type = CPP_TOKEN_COMMENT;
            S.token.flags = 0;
            --S.pos;
            break;
            
            case LS_comment_block:
            case LS_comment_block_ending:
            S.token.type = CPP_TOKEN_COMMENT;
            S.token.flags = 0;
            break;
            
            case LS_error_message:
            S.token.type = CPP_TOKEN_ERROR_MESSAGE;
            S.token.flags = 0;
            --S.pos;
            break;
            
            case LS_dot:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '*': S.token.type = CPP_TOKEN_PTRDOT; break;
                default:
                S.token.type = CPP_TOKEN_DOT;
                --S.pos;
                break;
            }
            break;
            
            case LS_ellipsis:
            switch (c){
                case '.':
                S.token.flags = CPP_TFLAG_IS_OPERATOR;
                S.token.type = CPP_TOKEN_ELLIPSIS;
                break;
                
                default:
                S.token.type = CPP_TOKEN_JUNK;
                --S.pos;
                break;
            }
            break;
            
            case LS_less:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_LESSEQ; break;
                default:
                S.token.type = CPP_TOKEN_LESS;
                --S.pos;
                break;
            }
            break;
            
            case LS_less_less:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_LSHIFTEQ; break;
                default:
                S.token.type = CPP_TOKEN_LSHIFT;
                --S.pos;
                break;
            }
            break;
            
            case LS_more:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_GRTREQ; break;
                default:
                S.token.type = CPP_TOKEN_GRTR;
                --S.pos;
                break;
            }
            break;
            
            case LS_more_more:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_RSHIFTEQ; break;
                default:
                S.token.type = CPP_TOKEN_RSHIFT;
                --S.pos;
                break;
            }
            break;
            
            case LS_minus:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '-': S.token.type = CPP_TOKEN_DECREMENT; break;
                case '=': S.token.type = CPP_TOKEN_SUBEQ; break;
                default:
                S.token.type = CPP_TOKEN_MINUS;
                --S.pos;
                break;
            }
            break;
            
            case LS_arrow:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '*': S.token.type = CPP_TOKEN_PTRARROW; break;
                default:
                S.token.type = CPP_TOKEN_ARROW;
                --S.pos;
                break;
            }
            break;
            
            case LS_and:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '&': S.token.type = CPP_TOKEN_AND; break;
                case '=': S.token.type = CPP_TOKEN_ANDEQ; break;
                default:
                S.token.type = CPP_TOKEN_AMPERSAND;
                --S.pos;
                break;
            }
            break;
            
            case LS_or:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '|': S.token.type = CPP_TOKEN_OR; break;
                case '=': S.token.type = CPP_TOKEN_OREQ; break;
                default:
                S.token.type = CPP_TOKEN_BIT_OR;
                --S.pos;
                break;
            }
            break;
            
            case LS_plus:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '+': S.token.type = CPP_TOKEN_INCREMENT; break;
                case '=': S.token.type = CPP_TOKEN_ADDEQ; break;
                default:
                S.token.type = CPP_TOKEN_PLUS;
                --S.pos;
                break;
            }
            break;
            
            case LS_colon:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case ':': S.token.type = CPP_TOKEN_SCOPE; break;
                default:
                S.token.type = CPP_TOKEN_COLON;
                --S.pos;
                break;
            }
            break;
            
            case LS_star:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_MULEQ; break;
                default:
                S.token.type = CPP_TOKEN_STAR;
                --S.pos;
                break;
            }
            break;
            
            case LS_modulo:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_MODEQ; break;
                default:
                S.token.type = CPP_TOKEN_MOD;
                --S.pos;
                break;
            }
            break;
            
            case LS_caret:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_XOREQ; break;
                default:
                S.token.type = CPP_TOKEN_BIT_XOR;
                --S.pos;
                break;
            }
            break;
            
            case LS_eq:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_EQEQ; break;
                default:
                S.token.type = CPP_TOKEN_EQ;
                --S.pos;
                break;
            }
            break;
            
            case LS_bang:
            S.token.flags = CPP_TFLAG_IS_OPERATOR;
            switch (c){
                case '=': S.token.type = CPP_TOKEN_NOTEQ; break;
                default:
                S.token.type = CPP_TOKEN_NOT;
                --S.pos;
                break;
            }
            break;
        }
        
        if (chunk[S.pos-1] == 0){
            --S.pos;
        }
        
        if ((S.token.flags & CPP_TFLAG_PP_DIRECTIVE) == 0){
            switch (S.pp_state){
                case LSPP_macro_identifier:
                if (S.fsm.state != LS_identifier){
                    S.token.type = CPP_TOKEN_JUNK;
                    S.pp_state = LSPP_junk;
                }
                else{
                    S.pp_state = LSPP_body;
                }
                break;
                
                case LSPP_identifier:
                if (S.fsm.state != LS_identifier){
                    S.token.type = CPP_TOKEN_JUNK;
                }
                S.pp_state = LSPP_junk;
                break;
                
                case LSPP_number:
                if (S.token.type != CPP_TOKEN_INTEGER_CONSTANT){
                    S.token.type = CPP_TOKEN_JUNK;
                    S.pp_state = LSPP_junk;
                }
                else{
                    S.pp_state = LSPP_include;
                }
                break;
                
                case LSPP_junk:
                if (S.token.type != CPP_TOKEN_COMMENT){
                    S.token.type = CPP_TOKEN_JUNK;
                }
                break;
            }
        }
        
        
        if (S.fsm.emit_token){
            S.token.start = S.token_start;
            if (S.pos_overide){
                S.token.size = S.pos_overide - S.token_start;
                S.pos_overide = 0;
            }
            else{
                S.token.size = S.pos - S.token_start;
            }
            if ((S.token.flags & CPP_TFLAG_PP_DIRECTIVE) == 0){
                S.token.flags |= (S.pp_state != LSPP_default)?(CPP_TFLAG_PP_BODY):(0);
            }
            
            token_i = cpp_place_token_nonalloc(out_tokens, token_i, S.token);
            if (token_i == max_token_i){
                if (S.pos == end_pos){
                    S.chunk_pos += size;
                    DrYield(7, LexNeedChunk);
                }
                DrYield(2, LexNeedTokenMemory);
            }
        }
        
        if (S.completed){
            break;
        }
    }
    
    DrReturn(LexFinished);
}

#undef DrYield
#undef DrReturn
#undef DrCase

FCPP_LINK int32_t
cpp_lex_nonalloc(Lex_Data *S_ptr,
                 char *chunk, int32_t size,
                 Cpp_Token_Stack *token_stack_out, int32_t max_tokens){
    Cpp_Token_Stack temp_stack = *token_stack_out;
    if (temp_stack.max_count > temp_stack.count + max_tokens){
        temp_stack.max_count = temp_stack.count + max_tokens;
    }
    
    int32_t result = cpp_lex_nonalloc(S_ptr, chunk, size, &temp_stack);
    
    token_stack_out->count = temp_stack.count;
    
    if (result == LexNeedTokenMemory){
        if (token_stack_out->count < token_stack_out->max_count){
            result = LexHitTokenLimit;
        }
    }
    
    return(result);
}

FCPP_LINK int32_t
cpp_lex_size_nonalloc(Lex_Data *S_ptr,
                      char *chunk, int32_t size, int32_t full_size,
                      Cpp_Token_Stack *token_stack_out){
    int32_t result = 0;
    if (S_ptr->pos >= full_size){
        char end_null = 0;
        result = cpp_lex_nonalloc(S_ptr, &end_null, 1, token_stack_out);
    }
    else{
        result = cpp_lex_nonalloc(S_ptr, chunk, size, token_stack_out);
        if (result == LexNeedChunk){
            if (S_ptr->pos >= full_size){
                char end_null = 0;
                result = cpp_lex_nonalloc(S_ptr, &end_null, 1, token_stack_out);
            }
        }
    }
    return(result);
}

FCPP_LINK int32_t
cpp_lex_size_nonalloc(Lex_Data *S_ptr,
                      char *chunk, int32_t size, int32_t full_size,
                      Cpp_Token_Stack *token_stack_out, int32_t max_tokens){
    Cpp_Token_Stack temp_stack = *token_stack_out;
    if (temp_stack.max_count > temp_stack.count + max_tokens){
        temp_stack.max_count = temp_stack.count + max_tokens;
    }
    
    int32_t result = cpp_lex_size_nonalloc(S_ptr, chunk, size, full_size,
                                           &temp_stack);
    
    token_stack_out->count = temp_stack.count;
    
    if (result == LexNeedTokenMemory){
        if (token_stack_out->count < token_stack_out->max_count){
            result = LexHitTokenLimit;
        }
    }
    
    return(result);
}

FCPP_LINK Cpp_Relex_State
cpp_relex_nonalloc_start(char *data, int32_t size, Cpp_Token_Stack *stack,
                         int32_t start, int32_t end, int32_t amount, int32_t tolerance){
    Cpp_Relex_State state;
    state.data = data;
    state.size = size;
    state.stack = stack;
    state.start = start;
    state.end = end;
    state.amount = amount;
    state.tolerance = tolerance;
    
    Cpp_Get_Token_Result result = cpp_get_token(stack, start);
    
    state.start_token_i = result.token_index-1;
    if (state.start_token_i < 0){
        state.start_token_i = 0;
    }
    
    result = cpp_get_token(stack, end);
    
    state.end_token_i = result.token_index;
    if (end > stack->tokens[state.end_token_i].start){
        ++state.end_token_i;
    }
    if (state.end_token_i < 0){
        state.end_token_i = 0;
    }
    
    state.relex_start = stack->tokens[state.start_token_i].start;
    if (start < state.relex_start){
        state.relex_start = start;
    }
    
    state.space_request = state.end_token_i - state.start_token_i + tolerance + 1;
    
    return(state);
}

FCPP_LINK char
cpp_token_get_pp_state(uint16_t bitfield){
    return (char)(bitfield);
}

// TODO(allen): Eliminate this once we actually store the EOF token
// in the token stack.
FCPP_LINK Cpp_Token
cpp__get_token(Cpp_Token_Stack *stack, Cpp_Token *tokens, int32_t size, int32_t index){
    Cpp_Token result;
    if (index < stack->count){
        result = tokens[index];
    }
    else{
        result.start = size;
        result.size = 0;
        result.type = CPP_TOKEN_EOF;
        result.flags = 0;
        result.state_flags = 0;
    }
    return result;
}

FCPP_LINK int32_t
cpp_relex_nonalloc_main(Cpp_Relex_State *state,
                        Cpp_Token_Stack *relex_stack,
                        int32_t *relex_end,
                        char *spare){
    Cpp_Token_Stack *stack = state->stack;
    Cpp_Token *tokens = stack->tokens;
    
    cpp_shift_token_starts(stack, state->end_token_i, state->amount);
    
    Lex_Data lex = lex_data_init(spare);
    lex.pp_state = cpp_token_get_pp_state(tokens[state->start_token_i].state_flags);
    lex.pos = state->relex_start;
    
    int32_t relex_end_i = state->end_token_i;
    Cpp_Token match_token = cpp__get_token(stack, tokens, state->size, relex_end_i);
    Cpp_Token end_token = match_token;
    int32_t went_too_far = false;
    
    // TODO(allen): This can be better I suspect.
    for (;;){
        int32_t result = 
            cpp_lex_size_nonalloc(&lex,
                                  state->data,
                                  state->size,
                                  state->size,
                                  relex_stack, 1);
        
        switch (result){
            case LexHitTokenLimit:
            {
                Cpp_Token token = relex_stack->tokens[relex_stack->count-1];
                if (token.start == end_token.start &&
                    token.size == end_token.size &&
                    token.flags == end_token.flags &&
                    token.state_flags == end_token.state_flags){
                    --relex_stack->count;
                    goto double_break;
                }
                
                while (lex.pos > end_token.start && relex_end_i < stack->count){
                    ++relex_end_i;
                    end_token = cpp__get_token(stack, tokens, state->size, relex_end_i);
                }
            }
            break;
            
            case LexNeedChunk: Assert(!"Invalid path"); break;
            
            case LexNeedTokenMemory:
            went_too_far = true;
            goto double_break;
            
            case LexFinished:
            goto double_break;
        }
    }
    double_break:;
    
    if (!went_too_far){
        *relex_end = relex_end_i;
    }
    else{
        cpp_shift_token_starts(stack, state->end_token_i, -state->amount);
    }
    
    return(went_too_far);
}

#if defined(FCPP_ALLOW_MALLOC)

#include <stdlib.h>
#include <string.h>

FCPP_LINK Cpp_Token_Stack
cpp_make_token_stack(int32_t starting_max){
    Cpp_Token_Stack token_stack;
    token_stack.count = 0;
    token_stack.max_count = starting_max;
    token_stack.tokens = (Cpp_Token*)malloc(sizeof(Cpp_Token)*starting_max);
    return(token_stack);
}

FCPP_LINK void
cpp_free_token_stack(Cpp_Token_Stack token_stack){
    free(token_stack.tokens);
}

FCPP_LINK void
cpp_resize_token_stack(Cpp_Token_Stack *token_stack, int32_t new_max){
    Cpp_Token *new_tokens = (Cpp_Token*)malloc(sizeof(Cpp_Token)*new_max);
    
    if (new_tokens){
        memcpy(new_tokens, token_stack->tokens, sizeof(Cpp_Token)*token_stack->count);
        free(token_stack->tokens);
        token_stack->tokens = new_tokens;
        token_stack->max_count = new_max;
    }	
}

FCPP_LINK void
cpp_push_token(Cpp_Token_Stack *token_stack, Cpp_Token token){
    if (!cpp_push_token_nonalloc(token_stack, token)){
        int32_t new_max = 2*token_stack->max_count + 1;
        cpp_resize_token_stack(token_stack, new_max);
        cpp_push_token_nonalloc(token_stack, token);
    }
}

FCPP_LINK void
cpp_lex_file(char *data, int32_t size, Cpp_Token_Stack *token_stack_out){
    Lex_Data S = {0};
    S.tb = (char*)malloc(size);
    int32_t quit = 0;
    
    token_stack_out->count = 0;
    for (;!quit;){
        int32_t result = cpp_lex_nonalloc(&S, data, size, token_stack_out);
        switch (result){
            case LexFinished:
            {
                quit = 1;
            }break;
            
            case LexNeedChunk:
            {
                char empty = 0;
                cpp_lex_nonalloc(&S, &empty, 1, token_stack_out);
                quit = 1;
            }break;
            
            case LexNeedTokenMemory:
            {
                int32_t new_max = 2*token_stack_out->max_count + 1;
                cpp_resize_token_stack(token_stack_out, new_max);
            }break;
        }
    }
    
    free(S.tb);
}

#endif

#endif

// BOTTOM
