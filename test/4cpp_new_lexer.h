
// TOP

#ifndef FCPP_NEW_LEXER_INC
#define FCPP_NEW_LEXER_INC

#include "../4cpp_lexer_types.h"

namespace new_lex{
//

#define lexer_link static


lexer_link Cpp_Get_Token_Result
cpp_get_token(Cpp_Token_Stack *token_stack, int pos){
    Cpp_Get_Token_Result result = {};
    Cpp_Token *token_array = token_stack->tokens;
    Cpp_Token *token = 0;
	int first = 0;
    int count = token_stack->count;
    int last = count;
    int this_start = 0, next_start = 0;
    
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

lexer_link void
cpp_shift_token_starts(Cpp_Token_Stack *stack, int from_token_i, int shift_amount){
    Cpp_Token *token = stack->tokens + from_token_i;
    int count = stack->count, i;
    
    for (i = from_token_i; i < count; ++i, ++token){
        token->start += shift_amount;
    }
}

enum Lex_State{
    LS_default,
    LS_identifier,
    LS_char,
    LS_string,
    LS_number,
    LS_float,
    LS_comment_pre,
    LS_comment,
    LS_comment_block,
    LS_comment_block_ending,
    LS_dot,
    LS_ellipsis,
    LS_less,
    LS_less_less,
    LS_more,
    LS_more_more,
    LS_minus,
    LS_arrow,
    LS_and,
    LS_or,
    LS_plus,
    LS_colon,
    LS_star,
    LS_modulo,
    LS_caret,
    LS_eq,
    LS_bang,
};

struct Lex_Data{
    int token_start;
    int token_end;
    int completed;
};

lexer_link Lex_Data
cpp_lex_nonalloc(char *chunk, int file_absolute_pos, int size, Cpp_Token_Stack *token_stack_out){
    Cpp_Token *out_tokens = token_stack_out->tokens;
    int token_i = token_stack_out->count;
    int max_token_i = token_stack_out->max_count;
    
    Cpp_Token token = {};
    
    int pos = file_absolute_pos;
    int end_pos = size + file_absolute_pos;
    unsigned short state = LS_default;
    unsigned short pp_state = 0;
    
    Lex_Data lex_data = {};
    
    int emit_token = 0;
    
    char c = 0;
    
    chunk -= file_absolute_pos;
    
    for (; pos < end_pos && token_i < max_token_i;){
        
        c = chunk[pos];
        
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v'){
            for (; pos < end_pos;){
                c = chunk[pos++];
                if (!(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v')) break;
            }
            --pos;
        }
        
        lex_data.token_start = pos;
        
        state = LS_default;
        emit_token = 0;
        for (; emit_token == 0 && pos <= end_pos;){
            if (pos < end_pos){
                c = chunk[pos++];
            }
            else{
                c = 0;
                ++pos;
            }
            
            switch (state){
                case LS_default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'){
                    state = LS_identifier;
                }
                else if (c >= '1' && c <= '9'){
                    state = LS_number;
                }
                else
                switch (c){
                    case '\'': state = LS_char; break;
                    case '"': state = LS_string; break;
                    
                    case '/': state = LS_comment_pre; break;
                    
                    case '.': state = LS_dot; break;
                    
                    case '<': state = LS_less; break;
                    case '>': state = LS_more; break;
                    
                    case '-': state = LS_minus; break;
                    
                    case '&': state = LS_and; break;
                    case '|': state = LS_or; break;
                    
                    case '+': state = LS_plus; break;
                    
                    case ':': state = LS_colon; break;
                    
                    case '*': state = LS_star; break;
                    
                    case '%': state = LS_modulo; break;
                    case '^': state = LS_caret; break;
                    
                    case '=': state = LS_eq; break;
                    case '!': state = LS_bang; break;
                    
#define OperCase(op,type) case op: emit_token = 1; break;
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
#undef OperCase
                }
                break;
                
                case LS_identifier:
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')){
                    emit_token = 1;
                }
                break;
                
                case LS_char:
                // TODO
                break;
                
                case LS_string:
                // TODO
                break;
                
                case LS_number:
                if (c >= '0' && c <= '9'){
                    state = LS_number;
                }
                else{
                    emit_token = 1;
                }
                break;
                
                case LS_dot:
                switch (c){
                    case '.': state = LS_ellipsis; break;
                    case '*': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_ellipsis:
                emit_token = 1;
                break;
                
                case LS_less:
                switch (c){
                    case '<': state = LS_less_less; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_less_less:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_more:
                switch (c){
                    case '>': state = LS_more_more; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_more_more:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_comment_pre:
                switch (c){
                    case '/': state = LS_comment; break;
                    case '*': state = LS_comment_block; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;

                case LS_comment:
                switch (c){
                    case '\n': emit_token = 1; break;
                }
                break;

                case LS_comment_block:
                switch (c){
                    case '*': state = LS_comment_block_ending; break;
                }
                break;

                case LS_comment_block_ending:
                switch (c){
                    case '*': state = LS_comment_block_ending; break;
                    case '/': emit_token = 1; break;
                    default: state = LS_comment_block; break;
                }
                break;
                
                case LS_minus:
                switch (c){
                    case '>': state = LS_arrow; break;
                    case '-': emit_token = 1; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_arrow:
                switch (c){
                    case '*': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_and:
                switch (c){
                    case '&': emit_token = 1; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_or:
                switch (c){
                    case '|': emit_token = 1; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_plus:
                switch (c){
                    case '+': emit_token = 1; break;
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_colon:
                switch (c){
                    case ':': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_star:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_modulo:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_caret:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_eq:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
                
                case LS_bang:
                switch (c){
                    case '=': emit_token = 1; break;
                    default: emit_token = 1; break;
                }
                break;
            }
        }
        
        if (emit_token){
            lex_data.token_end = pos;

            switch (state){
                case LS_default:
                switch (c){
#define OperCase(op,t) case op: token.type = t; break;
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
#undef OperCase
                }
                token.flags = CPP_TFLAG_IS_OPERATOR;
                break;
                
                case LS_identifier:
                token.type = CPP_TOKEN_IDENTIFIER;
                token.flags = 0;
                --lex_data.token_end;
                --pos;
                break;
                
                case LS_number:
                token.type = CPP_TOKEN_INTEGER_CONSTANT;
                token.flags = 0;
                --lex_data.token_end;
                --pos;
                break;
                
                case LS_comment_pre:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_DIVEQ; break;
                    default:
                    token.type = CPP_TOKEN_DIV;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_comment: case LS_comment_block_ending:
                token.type = CPP_TOKEN_COMMENT;
                token.flags = 0;
                c = chunk[--lex_data.token_end];
                while (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f'){
                    --lex_data.token_end;
                    c = chunk[lex_data.token_end];
                }
                ++lex_data.token_end;
                break;
                
                case LS_dot:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '*': token.type = CPP_TOKEN_PTRDOT; break;
                    default:
                    token.type = CPP_TOKEN_DOT;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_ellipsis:
                switch (c){
                    case '.':
                    token.flags = CPP_TFLAG_IS_OPERATOR;
                    token.type = CPP_TOKEN_ELLIPSIS;
                    break;
                    
                    default:
                    token.type = CPP_TOKEN_JUNK;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_less:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_LESSEQ; break;
                    default:
                    token.type = CPP_TOKEN_LESS;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_less_less:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_LSHIFTEQ; break;
                    default:
                    token.type = CPP_TOKEN_LSHIFT;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_more:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_GRTREQ; break;
                    default:
                    token.type = CPP_TOKEN_GRTR;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_more_more:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_RSHIFTEQ; break;
                    default:
                    token.type = CPP_TOKEN_RSHIFT;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_minus:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '-': token.type = CPP_TOKEN_DECREMENT; break;
                    case '=': token.type = CPP_TOKEN_SUBEQ; break;
                    default:
                    token.type = CPP_TOKEN_MINUS;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_arrow:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '*': token.type = CPP_TOKEN_PTRARROW; break;
                    default:
                    token.type = CPP_TOKEN_ARROW;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_and:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '&': token.type = CPP_TOKEN_AND; break;
                    case '=': token.type = CPP_TOKEN_ANDEQ; break;
                    default:
                    token.type = CPP_TOKEN_AMPERSAND;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_or:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '|': token.type = CPP_TOKEN_OR; break;
                    case '=': token.type = CPP_TOKEN_OREQ; break;
                    default:
                    token.type = CPP_TOKEN_BIT_OR;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_plus:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '+': token.type = CPP_TOKEN_INCREMENT; break;
                    case '=': token.type = CPP_TOKEN_ADDEQ; break;
                    default:
                    token.type = CPP_TOKEN_PLUS;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_colon:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case ':': token.type = CPP_TOKEN_SCOPE; break;
                    default:
                    token.type = CPP_TOKEN_COLON;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_star:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_MULEQ; break;
                    default:
                    token.type = CPP_TOKEN_STAR;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_modulo:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_MODEQ; break;
                    default:
                    token.type = CPP_TOKEN_MOD;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_caret:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_XOREQ; break;
                    default:
                    token.type = CPP_TOKEN_BIT_XOR;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_eq:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_EQEQ; break;
                    default:
                    token.type = CPP_TOKEN_EQ;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
                
                case LS_bang:
                token.flags = CPP_TFLAG_IS_OPERATOR;
                switch (c){
                    case '=': token.type = CPP_TOKEN_NOTEQ; break;
                    default:
                    token.type = CPP_TOKEN_BIT_NOT;
                    --lex_data.token_end;
                    --pos;
                    break;
                }
                break;
            }
            
            token.start = lex_data.token_start;
            token.size = lex_data.token_end - lex_data.token_start;
            token.state_flags = pp_state;
            out_tokens[token_i++] = token;
        }
    }
    
    token_stack_out->count = token_i;
    
    if (pos == end_pos) lex_data.completed = 1;
    return(lex_data);
}

}

#endif

// BOTTOM
