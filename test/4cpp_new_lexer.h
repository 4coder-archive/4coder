
// TOP

#ifndef FCPP_NEW_LEXER_INC
#define FCPP_NEW_LEXER_INC

#include "../4cpp_lexer_types.h"

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
    LS_comment_pre,
    LS_comment,
    LS_comment_block,
    LS_comment_block_ending,
    LS_dot,
    LS_less,
    LS_more,
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
    
    char c;
    
    chunk -= file_absolute_pos;
    
    for (; pos < end_pos && token_i < max_token_i; ++pos){
        for (; pos < end_pos;){
            c = chunk[pos++];
            if (!(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v')) break;
        }
        
        --pos;
        lex_data.token_start = pos;
        
        state = LS_default;
        emit_token = 0;
        for (; emit_token == 0 && pos < end_pos;){
            c = chunk[pos++];

            switch (state){
                case LS_default:
                switch (c){
                    case '/': state = LS_comment_pre; break;

#define OperCase(op,type) case op: emit_token = 1; break;
                    OperCase('{', CPP_TOKEN_BRACE_OPEN);
                    OperCase('}', CPP_TOKEN_BRACE_CLOSE);

                    OperCase('[', CPP_TOKEN_BRACKET_OPEN);
                    OperCase(']', CPP_TOKEN_BRACKET_CLOSE);

                    OperCase('(', CPP_TOKEN_PARENTHESE_OPEN);
                    OperCase(')', CPP_TOKEN_PARENTHESE_CLOSE);

                    OperCase('~', CPP_TOKEN_TILDE);
                    OperCase(',', CPP_TOKEN_COMMA);
                    OperCase('?', CPP_TOKEN_TERNARY_QMARK);
#undef OperCase

#if 0
                    case '.': state = LS_dot; break;
                    case '<': state = LS_less; break;
                    case '>': state = LS_more; break;
#endif
                }
                break;

                case LS_dot:
                break;

                case LS_less:
                break;

                case LS_more:
                break;

                case LS_comment_pre:
                switch (c){
                    case '/': state = LS_comment; break;
                    case '*': state = LS_comment_block; break;
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
            }
        }
        
        if (emit_token){
            --pos;
            lex_data.token_end = pos;

            switch (state){
                case LS_default:
                switch (chunk[pos]){
#define OperCase(op,t) case op: token.type = t; break;
                    OperCase('{', CPP_TOKEN_BRACE_OPEN);
                    OperCase('}', CPP_TOKEN_BRACE_CLOSE);

                    OperCase('[', CPP_TOKEN_BRACKET_OPEN);
                    OperCase(']', CPP_TOKEN_BRACKET_CLOSE);

                    OperCase('(', CPP_TOKEN_PARENTHESE_OPEN);
                    OperCase(')', CPP_TOKEN_PARENTHESE_CLOSE);

                    OperCase('~', CPP_TOKEN_TILDE);
                    OperCase(',', CPP_TOKEN_COMMA);
                    OperCase('?', CPP_TOKEN_TERNARY_QMARK);
#undef OperCase
                }
                token.flags = CPP_TFLAG_IS_OPERATOR;
                break;

                case LS_comment: case LS_comment_block_ending:
                token.type = CPP_TOKEN_COMMENT;
                token.flags = 0;
                c = chunk[pos];
                while (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f'){
                    --pos;
                    c = chunk[pos];
                }
                ++pos;
                break;
            }

            token.start = lex_data.token_start;
            token.size = pos - lex_data.token_start;
            token.state_flags = pp_state;
            out_tokens[token_i++] = token;

            pos = lex_data.token_end;
        }
    }
    
    token_stack_out->count = token_i;
    
    if (pos == end_pos) lex_data.completed = 1;
    return(lex_data);
}

#endif

// BOTTOM
