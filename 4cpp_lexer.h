
// TOP

#ifndef FCPP_NEW_LEXER_INC
#define FCPP_NEW_LEXER_INC

#ifndef Assert
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif

#ifndef FCPP_LINK
# define FCPP_LINK static
#endif

#define FCPP_INTERNAL FCPP_LINK

#include <stdint.h>
#if !defined(FSTRING_GUARD)
# define FSTRING_IMPLEMENTATION
# include "4coder_string.h"
#endif
#include "4cpp_lexer_types.h"
#include "4cpp_lexer_tables.c"

// TODO(allen): revisit this keyword data declaration system
struct String_And_Flag{
    String str;
    uint32_t flags;
};

static String_And_Flag preprops[] = {
    {make_lit_string("include"), CPP_PP_INCLUDE } ,
    {make_lit_string("INCLUDE"), CPP_PP_INCLUDE } ,
    {make_lit_string("ifndef" ), CPP_PP_IFNDEF  } ,
    {make_lit_string("IFNDEF" ), CPP_PP_IFNDEF  } ,
    {make_lit_string("define" ), CPP_PP_DEFINE  } ,
    {make_lit_string("DEFINE" ), CPP_PP_DEFINE  } ,
    {make_lit_string("import" ), CPP_PP_IMPORT  } ,
    {make_lit_string("IMPORT" ), CPP_PP_IMPORT  } ,
    {make_lit_string("pragma" ), CPP_PP_PRAGMA  } ,
    {make_lit_string("PRAGMA" ), CPP_PP_PRAGMA  } ,
    {make_lit_string("undef"  ), CPP_PP_UNDEF   } ,
    {make_lit_string("UNDEF"  ), CPP_PP_UNDEF   } ,
    {make_lit_string("endif"  ), CPP_PP_ENDIF   } ,
    {make_lit_string("ENDIF"  ), CPP_PP_ENDIF   } ,
    {make_lit_string("error"  ), CPP_PP_ERROR   } ,
    {make_lit_string("ERROR"  ), CPP_PP_ERROR   } ,
    {make_lit_string("ifdef"  ), CPP_PP_IFDEF   } ,
    {make_lit_string("IFDEF"  ), CPP_PP_IFDEF   } ,
    {make_lit_string("using"  ), CPP_PP_USING   } ,
    {make_lit_string("USING"  ), CPP_PP_USING   } ,
    {make_lit_string("else"   ), CPP_PP_ELSE    } ,
    {make_lit_string("ELSE"   ), CPP_PP_ELSE    } ,
    {make_lit_string("elif"   ), CPP_PP_ELIF    } ,
    {make_lit_string("ELIF"   ), CPP_PP_ELIF    } ,
    {make_lit_string("line"   ), CPP_PP_LINE    } ,
    {make_lit_string("LINE"   ), CPP_PP_LINE    } ,
    {make_lit_string("if"     ), CPP_PP_IF      } ,
    {make_lit_string("IF"     ), CPP_PP_IF      } ,
};

static String_And_Flag keywords[] = {
    {make_lit_string("true")  , CPP_TOKEN_BOOLEAN_CONSTANT},
    {make_lit_string("false") , CPP_TOKEN_BOOLEAN_CONSTANT},
    
    {make_lit_string("and")      , CPP_TOKEN_AND},
    {make_lit_string("and_eq")   , CPP_TOKEN_ANDEQ},
    {make_lit_string("bitand")   , CPP_TOKEN_BIT_AND},
    {make_lit_string("bitor")    , CPP_TOKEN_BIT_OR},
    {make_lit_string("or")       , CPP_TOKEN_OR},
    {make_lit_string("or_eq")    , CPP_TOKEN_OREQ},
    {make_lit_string("sizeof")   , CPP_TOKEN_SIZEOF},
    {make_lit_string("alignof")  , CPP_TOKEN_ALIGNOF},
    {make_lit_string("decltype") , CPP_TOKEN_DECLTYPE},
    {make_lit_string("throw")    , CPP_TOKEN_THROW},
    {make_lit_string("new")      , CPP_TOKEN_NEW},
    {make_lit_string("delete")   , CPP_TOKEN_DELETE},
    {make_lit_string("xor")      , CPP_TOKEN_BIT_XOR},
    {make_lit_string("xor_eq")   , CPP_TOKEN_XOREQ},
    {make_lit_string("not")      , CPP_TOKEN_NOT},
    {make_lit_string("not_eq")   , CPP_TOKEN_NOTEQ},
    {make_lit_string("typeid")   , CPP_TOKEN_TYPEID},
    {make_lit_string("compl")    , CPP_TOKEN_BIT_NOT},
    
    {make_lit_string("void")   , CPP_TOKEN_KEY_TYPE},
    {make_lit_string("bool")   , CPP_TOKEN_KEY_TYPE},
    {make_lit_string("char")   , CPP_TOKEN_KEY_TYPE},
    {make_lit_string("int")    , CPP_TOKEN_KEY_TYPE},
    {make_lit_string("float")  , CPP_TOKEN_KEY_TYPE},
    {make_lit_string("double") , CPP_TOKEN_KEY_TYPE},
    
    {make_lit_string("long")     , CPP_TOKEN_KEY_MODIFIER},
    {make_lit_string("short")    , CPP_TOKEN_KEY_MODIFIER},
    {make_lit_string("unsigned") , CPP_TOKEN_KEY_MODIFIER},
    
    {make_lit_string("const")    , CPP_TOKEN_KEY_QUALIFIER},
    {make_lit_string("volatile") , CPP_TOKEN_KEY_QUALIFIER},
    
    {make_lit_string("asm")           , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("break")         , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("case")          , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("catch")         , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("continue")      , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("default")       , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("do")            , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("else")          , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("for")           , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("goto")          , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("if")            , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("return")        , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("switch")        , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("try")           , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("while")         , CPP_TOKEN_KEY_CONTROL_FLOW},
    {make_lit_string("static_assert") , CPP_TOKEN_KEY_CONTROL_FLOW},
    
    {make_lit_string("const_cast")       , CPP_TOKEN_KEY_CAST},
    {make_lit_string("dynamic_cast")     , CPP_TOKEN_KEY_CAST},
    {make_lit_string("reinterpret_cast") , CPP_TOKEN_KEY_CAST},
    {make_lit_string("static_cast")      , CPP_TOKEN_KEY_CAST},
    
    {make_lit_string("class")    , CPP_TOKEN_KEY_TYPE_DECLARATION},
    {make_lit_string("enum")     , CPP_TOKEN_KEY_TYPE_DECLARATION},
    {make_lit_string("struct")   , CPP_TOKEN_KEY_TYPE_DECLARATION},
    {make_lit_string("typedef")  , CPP_TOKEN_KEY_TYPE_DECLARATION},
    {make_lit_string("union")    , CPP_TOKEN_KEY_TYPE_DECLARATION},
    {make_lit_string("template") , CPP_TOKEN_KEY_TYPE_DECLARATION},
    {make_lit_string("typename") , CPP_TOKEN_KEY_TYPE_DECLARATION},
    
    {make_lit_string("friend")    , CPP_TOKEN_KEY_ACCESS},
    {make_lit_string("namespace") , CPP_TOKEN_KEY_ACCESS},
    {make_lit_string("private")   , CPP_TOKEN_KEY_ACCESS},
    {make_lit_string("protected") , CPP_TOKEN_KEY_ACCESS},
    {make_lit_string("public")    , CPP_TOKEN_KEY_ACCESS},
    {make_lit_string("using")     , CPP_TOKEN_KEY_ACCESS},
    
    {make_lit_string("extern")  , CPP_TOKEN_KEY_LINKAGE},
    {make_lit_string("export")  , CPP_TOKEN_KEY_LINKAGE},
    {make_lit_string("inline")  , CPP_TOKEN_KEY_LINKAGE},
    {make_lit_string("static")  , CPP_TOKEN_KEY_LINKAGE},
    {make_lit_string("virtual") , CPP_TOKEN_KEY_LINKAGE},
    
    {make_lit_string("alignas")      , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("explicit")     , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("noexcept")     , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("nullptr")      , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("operator")     , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("register")     , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("this")         , CPP_TOKEN_KEY_OTHER},
    {make_lit_string("thread_local") , CPP_TOKEN_KEY_OTHER},
};


FCPP_LINK Cpp_Get_Token_Result
cpp_get_token(Cpp_Token_Array *token_array_in, int32_t pos)/*
DOC_PARAM(token_array, The array of tokens from which to get a token.)
DOC_PARAM(pos, The position, measured in bytes, to get the token for.)
DOC_RETURN(A Cpp_Get_Token_Result struct is returned containing the index
of a token and a flag indicating whether the pos is contained in the token
or in whitespace after the token.)

DOC(This call performs a binary search over all of the tokens looking
for the token that contains the specified position. If the position
is in whitespace between the tokens, the returned token index is the
index of the token immediately before the provided position.  The returned
index can be -1 if the position is before the first token.)

DOC_SEE(Cpp_Get_Token_Result)
*/{
    Cpp_Get_Token_Result result = {};
    Cpp_Token *token_array = token_array_in->tokens;
    Cpp_Token *token = 0;
	int32_t first = 0;
    int32_t count = token_array_in->count;
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

FCPP_INTERNAL Cpp_Lex_PP_State
cpp_pp_directive_to_state(Cpp_Token_Type type){
    Cpp_Lex_PP_State result = LSPP_default;
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

// duff-routine defines
#define DrCase(PC) case PC: goto resumespot_##PC

#define DrYield(PC, n) {                                           \
    token_array_out->count = token_i;                              \
    *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }

#define DrReturn(n) {                           \
    token_array_out->count = token_i;           \
    *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

FCPP_INTERNAL Cpp_Lex_Result
cpp_lex_nonalloc_null_end_no_limit(Cpp_Lex_Data *S_ptr, char *chunk, int32_t size,
                                   Cpp_Token_Array *token_array_out){
    Cpp_Lex_Data S = *S_ptr;
    
    Cpp_Token *out_tokens = token_array_out->tokens;
    int32_t token_i = token_array_out->count;
    int32_t max_token_i = token_array_out->max_count;
    
    uint8_t c = 0;
    
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
                DrYield(4, LexResult_NeedChunk);
            }
            else{
                break;
            }
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
                uint16_t *eq_classes = get_eq_classes[S.pp_state];
                uint8_t *fsm_table = get_table[S.pp_state];
                
                for (; S.fsm.state < LS_count && S.pos < end_pos;){
                    c = chunk[S.pos++];
                    S.tb[(S.tb_pos++) & (sizeof(S.tb)-1)] = c;
                    
                    int32_t i = S.fsm.state + eq_classes[c];
                    S.fsm.state = fsm_table[i];
                    S.fsm.multi_line |= multiline_state_table[S.fsm.state];
                }
                S.fsm.emit_token = (S.fsm.state >= LS_count);
            }
            
            if (S.fsm.emit_token == 0){
                S.chunk_pos += size;
                DrYield(3, LexResult_NeedChunk);
            }
            else{
                break;
            }
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
                            DrYield(1, LexResult_NeedChunk);
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
                
                if (word_size < sizeof(S.tb)){
                    if (S.pp_state == LSPP_body_if){
                        if (match_ss(make_string(S.tb, word_size), make_lit_string("defined"))){
                            S.token.type = CPP_PP_DEFINED;
                            S.token.flags = CPP_TFLAG_IS_OPERATOR | CPP_TFLAG_IS_KEYWORD;
                            break;
                        }
                    }
                    
                    int32_t sub_match = -1;
                    string_set_match_table(keywords, sizeof(*keywords), ArrayCount(keywords),
                                           make_string(S.tb, S.tb_pos-1), &sub_match);
                    
                    if (sub_match != -1){
                        String_And_Flag data = keywords[sub_match];
                        S.token.type = (Cpp_Token_Type)data.flags;
                        S.token.flags = CPP_TFLAG_IS_KEYWORD;
                        break;
                    }
                }
                
                S.token.type = CPP_TOKEN_IDENTIFIER;
                S.token.flags = 0;
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
                
                if (S.tb_pos < sizeof(S.tb)){
                    int32_t pos = S.tb_pos-1;
                    int32_t i = 1;
                    for (;i < pos; ++i){
                        if (S.tb[i] != ' '){
                            break;
                        }
                    }
                    
                    int32_t sub_match = -1;
                    string_set_match_table(preprops, sizeof(*preprops), ArrayCount(preprops),
                                           make_string(S.tb+i, pos-i), &sub_match);
                    
                    if (sub_match != -1){
                        String_And_Flag data = preprops[sub_match];
                        S.token.type = (Cpp_Token_Type)data.flags;
                        S.token.flags = CPP_TFLAG_PP_DIRECTIVE;
                        S.pp_state = (uint8_t)cpp_pp_directive_to_state(S.token.type);
                        break;
                    }
                }
                
                S.token.type = CPP_TOKEN_JUNK;
                S.token.flags = 0;
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
                    DrYield(5, LexResult_NeedChunk);
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
                    S.token.type = CPP_PP_INCLUDE_FILE;
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
            S.token.type = CPP_PP_ERROR_MESSAGE;
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
        
        if (S.pos > S.chunk_pos && chunk[S.pos-1] == 0){
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
            
            out_tokens[token_i++] = S.token;
            if (token_i == max_token_i){
                if (S.pos == end_pos){
                    S.chunk_pos += size;
                    DrYield(7, LexResult_NeedChunk);
                }
                DrYield(2, LexResult_NeedTokenMemory);
            }
        }
        
        if (S.completed){
            break;
        }
    }
    
    DrReturn(LexResult_Finished);
}

#undef DrYield
#undef DrReturn
#undef DrCase

FCPP_INTERNAL Cpp_Lex_Result
cpp_lex_nonalloc_null_end_out_limit(Cpp_Lex_Data *S_ptr, char *chunk, int32_t size,
                                    Cpp_Token_Array *token_array_out, int32_t max_tokens_out){
    Cpp_Token_Array temp_array = *token_array_out;
    if (temp_array.max_count > temp_array.count + max_tokens_out){
        temp_array.max_count = temp_array.count + max_tokens_out;
    }
    
    Cpp_Lex_Result result = cpp_lex_nonalloc_null_end_no_limit(S_ptr, chunk, size, &temp_array);
    
    token_array_out->count = temp_array.count;
    if (result == LexResult_NeedTokenMemory){
        if (token_array_out->count < token_array_out->max_count){
            result = LexResult_HitTokenLimit;
        }
    }
    
    return(result);
}

FCPP_INTERNAL Cpp_Lex_Result
cpp_lex_nonalloc_no_null_no_limit(Cpp_Lex_Data *S_ptr, char *chunk, int32_t size, int32_t full_size,
                                  Cpp_Token_Array *token_array_out){
    Cpp_Lex_Result result = 0;
    if (S_ptr->pos >= full_size){
        char end_null = 0;
        result = cpp_lex_nonalloc_null_end_no_limit(S_ptr, &end_null, 1, token_array_out);
    }
    else{
        result = cpp_lex_nonalloc_null_end_no_limit(S_ptr, chunk, size, token_array_out);
        if (result == LexResult_NeedChunk){
            if (S_ptr->pos >= full_size){
                char end_null = 0;
                result = cpp_lex_nonalloc_null_end_no_limit(S_ptr, &end_null, 1, token_array_out);
            }
        }
    }
    return(result);
}

FCPP_INTERNAL Cpp_Lex_Result
cpp_lex_nonalloc_no_null_out_limit(Cpp_Lex_Data *S_ptr, char *chunk, int32_t size, int32_t full_size,
                                   Cpp_Token_Array *token_array_out, int32_t max_tokens_out){
    Cpp_Token_Array temp_stack = *token_array_out;
    if (temp_stack.max_count > temp_stack.count + max_tokens_out){
        temp_stack.max_count = temp_stack.count + max_tokens_out;
    }
    
    Cpp_Lex_Result result = cpp_lex_nonalloc_no_null_no_limit(S_ptr, chunk, size, full_size,
                                                       &temp_stack);
    
    token_array_out->count = temp_stack.count;
    
    if (result == LexResult_NeedTokenMemory){
        if (token_array_out->count < token_array_out->max_count){
            result = LexResult_HitTokenLimit;
        }
    }
    
    return(result);
}

#define HAS_NULL_TERM ((int32_t)(-1))
#define NO_OUT_LIMIT ((int32_t)(-1))

FCPP_LINK Cpp_Lex_Result
cpp_lex_step(Cpp_Lex_Data *S_ptr, char *chunk, int32_t size, int32_t full_size,
             Cpp_Token_Array *token_array_out, int32_t max_tokens_out)/*
DOC_PARAM(S_ptr, The lexer state.  Go to the Cpp_Lex_Data section to see how to initialize the state.)
DOC_PARAM(chunk, The first or next chunk of the file being lexed.)
DOC_PARAM(size, The number of bytes in the chunk including the null terminator if the chunk ends in a null terminator.
If the chunk ends in a null terminator the system will interpret it as the end of the file.)
DOC_PARAM(full_size, If the final chunk is not null terminated this parameter should specify the length of the
file in bytes.  To rely on an eventual null terminator use HAS_NULL_TERM for this parameter.)
DOC_PARAM(token_array_out, The token array structure that will receive the tokens output by the lexer.)
DOC_PARAM(max_tokens_out, The maximum number of tokens to be output to the token array.  To rely on the
max built into the token array pass NO_OUT_LIMIT here.)

DOC(This call is the primary interface of the lexing system.  It is quite general so it can be used in
a lot of different ways.  I will explain the general rules first, and then give some examples of common
ways it might be used.

First a lexing state, Cpp_Lex_Data, must be initialized. The file to lex must be read into N contiguous chunks
of memory.  An output Cpp_Token_Array must be allocated and initialized with the appropriate count and max_count
values. Then each chunk of the file must be passed to cpp_lex_step in order using the same lexing state for each call.
Every time a call to cpp_lex_step returns LexResult_NeedChunk, the next call to cpp_lex_step should use the
next chunk.  If the return is some other value, the lexer hasn't finished with the current chunk and it sopped for some
other reason, so the same chunk should be used again in the next call.

If the file chunks contain a null terminator the lexer will return LexResult_Finished when it finds this character. 
At this point calling the lexer again with the same state will result in an error.  If you do not have a null
terminated chunk to end the file, you may instead pass the exact size in bytes of the entire file to the full_size
parameter and it will automatically handle the termination of the lexing state when it has read that many bytes.
If a full_size is specified and the system terminates for having seen that many bytes, it will return
LexResult_Finished. If a full_size is specified and a null character is read before the total number of bytes have
been read the system will still terminate as usual and return LexResult_Finished.

If the system has filled the entire output array it will return LexResult_NeedTokenMemory.  When this happens if you
want to continue lexing the file you can grow the token array, or switch to a new output array and then call
cpp_lex_step again with the chunk that was being lexed and the new output.  You can also specify a max_tokens_out
which is limits how many new tokens will be added to the token array.  Even if token_array_out still had more space
to hold tokens, if the max_tokens_out limit is hit, the lexer will stop and return LexResult_HitTokenLimit.  If this
happens there is still space left in the token array, so you can resume simply by calling cpp_lex_step again with
the same chunk and the same output array.  Also note that, unlike the chunks which must only be replaced when the
system says it needs a chunk.  You may switch to or modify the output array in between calls as much as you like.

The most basic use of this system is to get it all done in one big chunk and try to allocate a nearly "infinite" output
array so that it will not run out of memory.  This way you can get the entire job done in one call and then just assert
to make sure it returns LexResult_Finished to you:

CODE_EXAMPLE(
Cpp_Token_Array lex_file(char *file_name){
    File_Data file = read_whole_file(file_name);
    
    char *temp = (char*)malloc(4096); // hopefully big enough
    Cpp_Lex_Data lex_state = cpp_lex_data_init(temp); 
    
    Cpp_Token_Array array = {0};
    array.tokens = (Cpp_Token*)malloc(1 << 20); // hopefully big enough
    array.max_count = (1 << 20)/sizeof(Cpp_Token);
    
    Cpp_Lex_Result result = 
        cpp_lex_step(&lex_state, file.data, file.size, file.size,
                     &array, NO_OUT_LIMIT);
    Assert(result == LexResult_Finished);
    
    free(temp);
    
    return(array);
})

)

DOC_SEE(Cpp_Lex_Data)
DOC_SEE(Cpp_Lex_Result)
*/{
    Cpp_Lex_Result result = 0;
    if (full_size == HAS_NULL_TERM){
        if (max_tokens_out == NO_OUT_LIMIT){
            result = cpp_lex_nonalloc_null_end_no_limit(S_ptr, chunk, size, token_array_out);
        }
        else{
            result = cpp_lex_nonalloc_null_end_out_limit(S_ptr, chunk, size, token_array_out, max_tokens_out);
        }
    }
    else{
        if (max_tokens_out == NO_OUT_LIMIT){
            result = cpp_lex_nonalloc_no_null_no_limit(S_ptr, chunk, size, full_size, token_array_out);
        }
        else{
            result = cpp_lex_nonalloc_no_null_out_limit(S_ptr, chunk, size, full_size, token_array_out, max_tokens_out);
        }
    }
    return(result);
}

FCPP_LINK Cpp_Lex_Data
cpp_lex_data_init()/*
DOC_RETURN(A brand new lex state ready to begin lexing a file from the beginning.)

DOC(Creates a new lex state in the form of a Cpp_Lex_Data struct and returns the struct.
The system needs a temporary buffer that is as long as the longest token.  4096 is usually
enough but the buffer is not checked, so to be 100% bullet proof it has to be the same length
as the file being lexed.)
*/{
    Cpp_Lex_Data data = {0};
    return(data);
}

FCPP_LINK int32_t
cpp_lex_data_temp_size(Cpp_Lex_Data *lex_data)/*
DOC_PARAM(lex_data, The lex state from which to get the temporary buffer size.)
DOC(This call gets the current size of the temporary buffer in the lexer state so
that you can move to a new temporary buffer by copying the data over.)
DOC_SEE(cpp_lex_data_temp_read)
DOC_SEE(cpp_lex_data_new_temp)
*/{
    int32_t result = lex_data->tb_pos;
    Assert(lex_data->tb != 0);
    return(result);
}

FCPP_LINK void
cpp_lex_data_temp_read(Cpp_Lex_Data *lex_data, char *out_buffer)/*
DOC_PARAM(lex_data, The lex state from which to read the temporary buffer.)
DOC_PARAM(out_buffer, The buffer into which the contents of the temporary buffer will be written.
The size of the buffer must be at least the size as returned by cpp_lex_data_temp_size.)
DOC(This call reads the current contents of the temporary buffer.)
DOC_SEE(cpp_lex_data_temp_size)
DOC_SEE(cpp_lex_data_new_temp)
*/{
    int32_t size = lex_data->tb_pos;
    char *src = lex_data->tb;
    char *end = src + size;
    for (; src < end; ++src, ++out_buffer){
        *out_buffer = *src;
    }
}

FCPP_LINK void
cpp_lex_data_new_temp_DEP(Cpp_Lex_Data *lex_data, char *new_buffer)
/*DOC(Deprecated in 4cpp Lexer 1.0.1*/{}

FCPP_INTERNAL char
cpp_token_get_pp_state(uint16_t bitfield){
    return (char)(bitfield);
}

FCPP_INTERNAL void
cpp_shift_token_starts(Cpp_Token_Array *array, int32_t from_token_i, int32_t shift_amount){
    Cpp_Token *token = array->tokens + from_token_i;
    int32_t count = array->count, i = 0;
    for (i = from_token_i; i < count; ++i, ++token){
        token->start += shift_amount;
    }
}

FCPP_INTERNAL Cpp_Token
cpp_index_array(Cpp_Token_Array *array, int32_t file_size, int32_t index){
    Cpp_Token result;
    if (index < array->count){
        result = array->tokens[index];
    }
    else{
        result.start = file_size;
        result.size = 0;
        result.type = CPP_TOKEN_EOF;
        result.flags = 0;
        result.state_flags = 0;
    }
    return(result);
}

FCPP_LINK Cpp_Relex_Range
cpp_get_relex_range(Cpp_Token_Array *array, int32_t start_pos, int32_t end_pos)
/*
DOC_PARAM(array, A pointer to the token array that will be modified by the relex,
this array should already contain the tokens for the previous state of the file.)
DOC_PARAM(start_pos, The start position of the edited region of the file.
The start and end points are based on the edited region of the file before the edit.)
DOC_PARAM(end_pos, The end position of the edited region of the file.
In particular, end_pos is the first character after the edited region not effected by the edit.
Thus if the edited region contained one character end_pos - start_pos should equal 1.
The start and end points are based on the edited region of the file before the edit.)
*/{
    Cpp_Relex_Range range = {0};
    Cpp_Get_Token_Result get_result = {0};
    
    get_result = cpp_get_token(array, start_pos);
    range.start_token_index = get_result.token_index-1;
    if (range.start_token_index < 0){
        range.start_token_index = 0;
    }
    
    get_result = cpp_get_token(array, end_pos);
    range.end_token_index = get_result.token_index;
    if (end_pos > array->tokens[range.end_token_index].start){
        ++range.end_token_index;
    }
    if (range.end_token_index < 0){
        range.end_token_index = 0;
    }
    
    return(range);
}

FCPP_LINK Cpp_Relex_Data
cpp_relex_init(Cpp_Token_Array *array, int32_t start_pos, int32_t end_pos, int32_t character_shift_amount)
/*
DOC_PARAM(array, A pointer to the token array that will be modified by the relex,
this array should already contain the tokens for the previous state of the file.)
DOC_PARAM(start_pos, The start position of the edited region of the file.
The start and end points are based on the edited region of the file before the edit.)
DOC_PARAM(end_pos, The end position of the edited region of the file.
In particular, end_pos is the first character after the edited region not effected by the edit.
Thus if the edited region contained one character end_pos - start_pos should equal 1.
The start and end points are based on the edited region of the file before the edit.)
DOC_PARAM(character_shift_amount, The shift in the characters after the edited region.)
DOC_RETURN(Returns a partially initialized relex state.)

DOC(This call does the first setup step of initializing a relex state.  To finish initializing the relex state
you must tell the state about the positioning of the first chunk it will be fed.  There are two methods of doing
this, the direct method is with cpp_relex_declare_first_chunk_position, the method that is often more convenient
is with cpp_relex_is_start_chunk.  If the file is not chunked the second step of initialization can be skipped.)

DOC_SEE(cpp_relex_declare_first_chunk_position)
DOC_SEE(cpp_relex_is_start_chunk)

*/{
    Cpp_Relex_Data state = {0};
    
    Cpp_Relex_Range range = cpp_get_relex_range(array, start_pos, end_pos);
    state.start_token_index = range.start_token_index;
    state.end_token_index = range.end_token_index;
    state.original_end_token_index = range.end_token_index;
    
    state.relex_start_position = array->tokens[state.start_token_index].start;
    if (start_pos < state.relex_start_position){
        state.relex_start_position = start_pos;
    }
    
    state.character_shift_amount = character_shift_amount;
    
    state.lex = cpp_lex_data_init();
    state.lex.pp_state = cpp_token_get_pp_state(array->tokens[state.start_token_index].state_flags);
    state.lex.pos = state.relex_start_position;
    
    return(state);
}

FCPP_LINK int32_t
cpp_relex_start_position(Cpp_Relex_Data *S_ptr)
/*
DOC_PARAM(S_ptr, A pointer to a state that is done with the first stage of initialization (cpp_relex_init))
DOC_RETURN(Returns the first position in the file the relexer wants to read.  This is usually a position slightly
earlier than the start_pos provided as the edit range.)

DOC(After doing the first stage of initialization this call is useful for figuring out what chunk
of the file to feed to the lexer first.  It should be a chunk that contains the position returned
by this call.)

DOC_SEE(cpp_relex_init)
DOC_SEE(cpp_relex_declare_first_chunk_position)

*/{
    int32_t result = S_ptr->relex_start_position;
    return(result);
}

FCPP_LINK void
cpp_relex_declare_first_chunk_position(Cpp_Relex_Data *S_ptr, int32_t position)
/*
DOC_PARAM(S_ptr, A pointer to a state that is done with the first stage of initialization (cpp_relex_init))
DOC_PARAM(position, The start position of the first chunk that will be fed to the relex process.)

DOC(To initialize the relex system completely, the system needs to know how the characters in the
first file line up with the file's absolute layout.  This call declares where the first chunk's start
position is in the absolute file layout, and the system infers the alignment from that.  For this method
to work the starting position of the relexing needs to be inside the first chunk.  To get the relexers
starting position call cpp_relex_start_position.)

DOC_SEE(cpp_relex_init)
DOC_SEE(cpp_relex_start_position)

*/{
    S_ptr->lex.chunk_pos = position;
}

FCPP_LINK int32_t
cpp_relex_is_start_chunk(Cpp_Relex_Data *S_ptr, char *chunk, int32_t chunk_size)
/*
DOC_PARAM(S_ptr, A pointer to a state that is done with the first stage of initialization (cpp_relex_init))
DOC_PARAM(chunk, The chunk to check.)
DOC_PARAM(chunk_size, The size of the chunk to check.)

DOC_RETURN(Returns non-zero if the passed in chunk should be used as the first chunk for lexing.)

DOC(With this method, once a state is initialized, each chunk can be fed in one after the other in
the order they appear in the absolute file layout.  When this call returns non-zero it means that
the chunk that was passed in on that call should be used in the first call to cpp_relex_step.  If,
after trying all of the chunks, they all return zero, pass in NULL for chunk and 0 for chunk_size
to tell the system that all possible chunks have already been tried, and then use those values again
in the one and only call to cpp_relex_step.)

DOC_SEE(cpp_relex_init)
*/{
    int32_t pos = S_ptr->relex_start_position;
    int32_t start = S_ptr->lex.chunk_pos;
    int32_t end = start + chunk_size;
    
    int32_t good_chunk = 0;
    if (start <= pos && pos < end){
        good_chunk = 1;
    }
    else{
        if (chunk == 0){
            good_chunk = 1;
            S_ptr->lex.chunk_pos = pos;
        }
        else{
            S_ptr->lex.chunk_pos += chunk_size;
        }
    }
    
    return(good_chunk);
}

// duff-routine defines
#define DrCase(PC) case PC: goto resumespot_##PC

#define DrYield(PC, n) {                                           \
    S_ptr->result_state = n;                                       \
    *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }

#define DrReturn(n) {                            \
    S_ptr->result_state = n;                     \
    *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

FCPP_LINK Cpp_Lex_Result
cpp_relex_step(Cpp_Relex_Data *S_ptr, char *chunk, int32_t chunk_size, int32_t full_size,
               Cpp_Token_Array *array, Cpp_Token_Array *relex_array)
/*
DOC_PARAM(S_ptr, A pointer to a fully initiazed relex state.)
DOC_PARAM(chunk, A chunk of the edited file being relexed.)
DOC_PARAM(chunk_size, The size of the current chunk.)
DOC_PARAM(full_size, The full size of the edited file.)
DOC_PARAM(array, A pointer to a token array that contained the original tokens before the edit.)
DOC_PARAM(relex_array, A pointer to a token array for spare space.  The capacity of the
relex_array determines how far the relex process can go.  If it runs out, the process
can be continued if the same relex_array is extended without losing the tokens it contains.

To get an appropriate capacity for relex_array, you can get the range of tokens that the relex
operation is likely to traverse by looking at the result from cpp_get_relex_range.)

DOC(When a file has already been lexed, and then it is edited in a small local way,
rather than lexing the new file all over again, cpp_relex_step can try to find just
the range of tokens that need to be updated and fix them in.

First the lex state must be initialized (cpp_relex_init).  Then one or more calls to
cpp_relex_step will start editing the array and filling out the relex_array.  The return
value of cpp_relex_step indicates whether the relex was successful or was interrupted
and if it was interrupted, what the system needs to resume.

LexResult_Finished indicates that the relex engine finished successfully.

LexResult_NeedChunk indicates that the system needs the next chunk of the file.

LexResult_NeedTokenMemory indicates that the relex_array has reached capacity, and that
it needs to be extended if it is going to continue.  Sometimes in this case it is better
to stop and just lex the entire file normally, because there are a few cases where a small
local change effects a long range of the lexers output.

The relex operation can be closed in one of two ways.  If the LexResult_Finished
value has been returned by this call, then to complete the edits to the array make
sure the original array has enough capacity to store the final result by calling
cpp_relex_get_new_count.  Then the operation can be finished successfully by calling
cpp_relex_complete.

Whether or not the relex process finished with LexResult_Finished the process can be
finished by calling cpp_relex_abort, which puts the array back into it's original state.
No close is necessary if getting the original array state back is not necessary.)

DOC_SEE(cpp_relex_init)
DOC_SEE(cpp_get_relex_range)
DOC_SEE(Cpp_Lex_Result)
DOC_SEE(cpp_relex_get_new_count)
DOC_SEE(cpp_relex_complete)
DOC_SEE(cpp_relex_abort)
*/{
    
    Cpp_Relex_Data S = *S_ptr;
    Cpp_Lex_Result step_result = LexResult_Finished;
    
    switch (S.__pc__){
        DrCase(1);
        DrCase(2);
    }
    
    cpp_shift_token_starts(array, S.end_token_index, S.character_shift_amount);
    S.end_token = cpp_index_array(array, full_size, S.end_token_index);
    
    // TODO(allen): This can be better I suspect.
    for (;;){
        step_result = 
            cpp_lex_nonalloc_no_null_out_limit(&S.lex, chunk, chunk_size, full_size,
                                               relex_array, 1);
        
        switch (step_result){
            case LexResult_HitTokenLimit:
            {
                Cpp_Token token = relex_array->tokens[relex_array->count-1];
                if (token.type == S.end_token.type &&
                    token.start == S.end_token.start &&
                    token.size == S.end_token.size &&
                    token.flags == S.end_token.flags &&
                    token.state_flags == S.end_token.state_flags){
                    --relex_array->count;
                    goto double_break;
                }
                
                while (S.lex.pos > S.end_token.start && S.end_token_index < array->count){
                    ++S.end_token_index;
                    S.end_token = cpp_index_array(array, full_size, S.end_token_index);
                }
            }
            break;
            
            case LexResult_NeedChunk: DrYield(1, LexResult_NeedChunk); break;
            
            case LexResult_NeedTokenMemory: DrYield(2, LexResult_NeedTokenMemory); break;
            
            case LexResult_Finished: goto double_break;
        }
    }
    
    double_break:;
    DrReturn(LexResult_Finished);
}

#undef DrYield
#undef DrReturn
#undef DrCase

FCPP_LINK int32_t
cpp_relex_get_new_count(Cpp_Relex_Data *S_ptr, int32_t current_count, Cpp_Token_Array *relex_array)
/*
DOC_PARAM(S_ptr, A pointer to a state that has gone through cpp_relex_step with a LexResult_Finished return.)
DOC_PARAM(current_count, The count of tokens in the original array before the edit.)
DOC_PARAM(relex_array, The relex_array that was used in the cpp_relex_step call/calls.)

DOC(After getting a LexResult_Finished from cpp_relex_step, this call can be used to get
the size the new array will have.  If the original array doesn't have enough capacity to store
the new array, it's capacity should be increased before passing to cpp_relex_complete.)
*/{
    int32_t result = -1;
    
    if (S_ptr->result_state == LexResult_Finished){
        int32_t delete_amount = S_ptr->end_token_index - S_ptr->start_token_index;
        int32_t shift_amount = relex_array->count - delete_amount;
        result = current_count + shift_amount;
    }
    
    return(result);
}

#if !defined(FCPP_FORBID_MEMCPY)
#include <string.h>
#endif

FCPP_INTERNAL void
cpp__block_move(void *dst, void *src, int32_t size){
#if !defined(FCPP_FORBID_MEMCPY)
    memmove(dst, src, size);
#else
    // TODO(allen): find a way to write a fast one of these.
    uint8_t *d = (uint8_t*)dst, *s = (uint8_t*)src;
    if (d < s || d >= s + size){
        for (; size > 0; --size){
            *(d++) = *(s++);
        }
    }
    else{
        d += size - 1;
        s += size - 1;
        for (; size > 0; --size){
            *(d--) = *(s--);
        }
    }
#endif
}

FCPP_LINK void
cpp_relex_complete(Cpp_Relex_Data *S_ptr, Cpp_Token_Array *array, Cpp_Token_Array *relex_array)
/*
DOC_PARAM(S_ptr, A pointer to a state that has gone through cpp_relex_step with a LexResult_Finished return.)
DOC_PARAM(array, The original array being edited by cpp_relex_step calls.)
DOC_PARAM(relex_array, The relex_array that was filled by cpp_relex_step.)

DOC(After getting a LexResult_Finished from cpp_relex_step, and ensuring that
array has a large enough capacity by calling cpp_relex_get_new_count, this call
does the necessary replacement of tokens in the array to make it match the new file.)
*/{
    int32_t delete_amount = S_ptr->end_token_index - S_ptr->start_token_index;
    int32_t shift_amount = relex_array->count - delete_amount;
    
    if (shift_amount != 0){
        int32_t shift_size = array->count - S_ptr->end_token_index;
        if (shift_size > 0){
            Cpp_Token *old_base = array->tokens + S_ptr->end_token_index;
            cpp__block_move(old_base + shift_amount, old_base, sizeof(Cpp_Token)*shift_size);
        }
        array->count += shift_amount;
    }
    
    cpp__block_move(array->tokens + S_ptr->start_token_index, relex_array->tokens,
                    sizeof(Cpp_Token)*relex_array->count);
}

FCPP_LINK void
cpp_relex_abort(Cpp_Relex_Data *S_ptr, Cpp_Token_Array *array)
/*
DOC_PARAM(S_ptr, A pointer to a state that has gone through at least one cpp_relex_step.)
DOC_PARAM(array, The original array that went through cpp_relex_step to be edited.)

DOC(After the first call to cpp_relex_step, the array's contents may have been changed,
this call assures the array is in it's original state.  After this call the relex state
is dead.)
*/{
    cpp_shift_token_starts(array, S_ptr->original_end_token_index, -S_ptr->character_shift_amount);
    S_ptr->__pc__ = -1;
}


#if !defined(FCPP_FORBID_MALLOC)

#include <stdlib.h>
#include <string.h>

FCPP_LINK Cpp_Token_Array
cpp_make_token_array(int32_t starting_max)/*
DOC_PARAM(starting_max, The number of tokens to initialize the array with.)
DOC_RETURN(An empty Cpp_Token_Array with memory malloc'd for storing tokens.)
DOC(This call allocates a Cpp_Token_Array with malloc for use in other
convenience functions.  Stacks that are not allocated this way should not be
used in the convenience functions.)
*/{
    Cpp_Token_Array token_array;
    token_array.tokens = (Cpp_Token*)malloc(sizeof(Cpp_Token)*starting_max);
    token_array.count = 0;
    token_array.max_count = starting_max;
    return(token_array);
}

FCPP_LINK void
cpp_free_token_array(Cpp_Token_Array token_array)/*
DOC_PARAM(token_array, An array previously allocated by cpp_make_token_array)
DOC(This call frees a Cpp_Token_Array.)
DOC_SEE(cpp_make_token_array)
*/{
    free(token_array.tokens);
}

FCPP_LINK void
cpp_resize_token_array(Cpp_Token_Array *token_array, int32_t new_max)/*
DOC_PARAM(token_array, An array previously allocated by cpp_make_token_array.)
DOC_PARAM(new_max, The new maximum size the array should support.  If this is not greater
than the current size of the array the operation is ignored.)
DOC(This call allocates a new memory chunk and moves the existing tokens in the array
over to the new chunk.)
DOC_SEE(cpp_make_token_array)
*/{
    if (new_max > token_array->count){
        Cpp_Token *new_tokens = (Cpp_Token*)malloc(sizeof(Cpp_Token)*new_max);
        
        if (new_tokens){
            memcpy(new_tokens, token_array->tokens, sizeof(Cpp_Token)*token_array->count);
            free(token_array->tokens);
            token_array->tokens = new_tokens;
            token_array->max_count = new_max;
        }	
    }
}

FCPP_LINK void
cpp_lex_file(char *data, int32_t size, Cpp_Token_Array *token_array_out)/*
DOC_PARAM(data, The file data to be lexed in a single contiguous block.)
DOC_PARAM(size, The number of bytes in data.)
DOC_PARAM(token_array_out, The token array where the output tokens will be pushed.
This token array must be previously allocated with cpp_make_token_array)
DOC(Lexes an entire file and manages the interaction with the lexer system so that
it is quick and convenient to lex files.

CODE_EXAMPLE(
Cpp_Token_Array lex_file(char *file_name){
    File_Data file = read_whole_file(file_name);
    
    // This array will be automatically grown if it runs
    // out of memory.
    Cpp_Token_Array array = cpp_make_token_array(100);
    
    cpp_lex_file(file.data, file.size, &array);
    
    return(array);
})

)
DOC_SEE(cpp_make_token_array)
*/{
    Cpp_Lex_Data S = cpp_lex_data_init();
    int32_t quit = 0;
    
    char empty = 0;
    
    token_array_out->count = 0;
    for (;!quit;){
        int32_t result = cpp_lex_step(&S, data, size, HAS_NULL_TERM, token_array_out, NO_OUT_LIMIT);
        switch (result){
            case LexResult_Finished:
            {
                quit = 1;
            }break;
            
            case LexResult_NeedChunk:
            {
                Assert(token_array_out->count < token_array_out->max_count);
                
                // NOTE(allen): We told the system we would provide the null
                // terminator, but as it turned out we didn't actually. So in
                // the next iteration pass a 1 byte chunk with the null terminator.
                data = &empty;
                size = 1;
            }break;
            
            case LexResult_NeedTokenMemory:
            {
                // NOTE(allen): We told the system to use all of the output memory
                // but we ran out anyway, so allocate more memory.  We hereby assume
                // the stack was allocated using cpp_make_token_array.
                int32_t new_max = 2*token_array_out->max_count + 1;
                cpp_resize_token_array(token_array_out, new_max);
            }break;
        }
    }
}

#endif

#endif

// BOTTOM
