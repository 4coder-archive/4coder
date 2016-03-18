
// TOP

#ifndef FCPP_NEW_LEXER_INC
#define FCPP_NEW_LEXER_INC

#include "../4cpp_lexer_types.h"

namespace new_lex{
//

#define lexer_link static

// TODO(allen): revisit this keyword data declaration system
struct String_And_Flag{
	char *str;
	fcpp_u32 flags;
};

struct String_List{
	String_And_Flag *data;
	int count;
};

struct Sub_Match_List_Result{
	int index;
	fcpp_i32 new_pos;
};


// TODO(allen): shift towards storing in a context
static String_And_Flag int_suf_strings[] = {
	{"ull"}, {"ULL"},
	{"llu"}, {"LLU"},
	{"ll"}, {"LL"},
	{"l"}, {"L"},
	{"u"}, {"U"}
};

#define lexer_string_list(x) {x, (sizeof(x)/sizeof(*x))}

static String_List int_sufs = lexer_string_list(int_suf_strings);

static String_And_Flag float_suf_strings[] = {
	{"f"}, {"F"},
	{"l"}, {"L"}
};
static String_List float_sufs = lexer_string_list(float_suf_strings);

static String_And_Flag bool_lit_strings[] = {
	{"true"}, {"false"}
};
static String_List bool_lits = lexer_string_list(bool_lit_strings);

static String_And_Flag keyword_strings[] = {
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

static String_And_Flag op_strings[] = {
    {"...", CPP_TOKEN_ELLIPSIS},
	{"<<=", CPP_TOKEN_LSHIFTEQ},
	{">>=", CPP_TOKEN_RSHIFTEQ},
	{"->*", CPP_TOKEN_PTRARROW},
	{"<<", CPP_TOKEN_LSHIFT},
	{">>", CPP_TOKEN_RSHIFT},
	{"&&", CPP_TOKEN_AND},
	{"||", CPP_TOKEN_OR},
	{"->", CPP_TOKEN_ARROW},
	{"++", CPP_TOKEN_INCREMENT},
	{"--", CPP_TOKEN_DECREMENT},
	{"::", CPP_TOKEN_SCOPE},
	{"+=", CPP_TOKEN_ADDEQ},
	{"-=", CPP_TOKEN_SUBEQ},
	{"*=", CPP_TOKEN_MULEQ},
	{"/=", CPP_TOKEN_DIVEQ},
	{"%=", CPP_TOKEN_MODEQ},
	{"&=", CPP_TOKEN_ANDEQ},
	{"|=", CPP_TOKEN_OREQ},
	{"^=", CPP_TOKEN_XOREQ},
	{"==", CPP_TOKEN_EQEQ},
	{">=", CPP_TOKEN_GRTREQ},
	{"<=", CPP_TOKEN_LESSEQ},
	{"!=", CPP_TOKEN_NOTEQ},
	{".*", CPP_TOKEN_PTRDOT},
	{"{", CPP_TOKEN_BRACE_OPEN},
	{"}", CPP_TOKEN_BRACE_CLOSE},
	{"[", CPP_TOKEN_BRACKET_OPEN},
	{"]", CPP_TOKEN_BRACKET_CLOSE},
	{"(", CPP_TOKEN_PARENTHESE_OPEN},
	{")", CPP_TOKEN_PARENTHESE_CLOSE},
	{"<", CPP_TOKEN_LESS},
	{">", CPP_TOKEN_GRTR},
	{"+", CPP_TOKEN_PLUS},
	{"-", CPP_TOKEN_MINUS},
	{"!", CPP_TOKEN_NOT},
	{"~", CPP_TOKEN_TILDE},
	{"*", CPP_TOKEN_STAR},
	{"&", CPP_TOKEN_AMPERSAND},
	{"|", CPP_TOKEN_BIT_OR},
	{"^", CPP_TOKEN_BIT_XOR},
	{"=", CPP_TOKEN_EQ},
	{",", CPP_TOKEN_COMMA},
	{":", CPP_TOKEN_COLON},
	{";", CPP_TOKEN_SEMICOLON},
	{"/", CPP_TOKEN_DIV},
	{"?", CPP_TOKEN_TERNARY_QMARK},
	{"%", CPP_TOKEN_MOD},
	{".", CPP_TOKEN_DOT},
};
static String_List ops = lexer_string_list(op_strings);

static String_And_Flag pp_op_strings[] = {
	{"##", CPP_PP_CONCAT},
	{"#", CPP_PP_STRINGIFY},
};
static String_List pp_ops = lexer_string_list(pp_op_strings);

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

lexer_link Sub_Match_List_Result
sub_match_list(char *chunk, int size, int pos, String_List list, int sub_size){
	Sub_Match_List_Result result;
    String str_main;
    char *str_check;
    int i,l;
    
    result.index = -1;
    result.new_pos = pos;
    str_main = make_string(chunk + pos, size - pos);
    if (sub_size > 0){
        str_main = substr(str_main, 0, sub_size);
        for (i = 0; i < list.count; ++i){
            str_check = list.data[i].str;
            if (match(str_main, str_check)){
                result.index = i;
                result.new_pos = pos + sub_size;
                break;
            }
        }
    }
    else{
        for (i = 0; i < list.count; ++i){
            str_check = list.data[i].str;
            if (match_part(str_main, str_check, &l)){
                result.index = i;
                result.new_pos = pos + l;
                break;
            }
        }
    }
	return result;
}


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
    LS_pound,
    LS_pp,
    LS_char,
    LS_char_slashed,
    LS_string,
    LS_string_slashed,
    LS_number,
    LS_number0,
    LS_float,
    LS_crazy_float0,
    LS_crazy_float1,
    LS_hex,
    LS_comment_pre,
    LS_comment,
    LS_comment_slashed,
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
    LS_error_message,
    //
    LS_count
};

enum Lex_Int_State{
	LSINT_default,
    LSINT_u,
    LSINT_l,
    LSINT_L,
    LSINT_ul,
    LSINT_uL,
    LSINT_ll,
    LSINT_extra
};

enum Lex_INC_State{
    LSINC_default,
    LSINC_quotes,
    LSINC_pointy,
    LSINC_junk,
};

enum Lex_PP_State{
    LSPP_default,
    LSPP_include,
    LSPP_macro_identifier,
    LSPP_identifier,
    LSPP_body_if,
    LSPP_body,
    LSPP_number,
    LSPP_error,
    LSPP_junk,
    //
    LSPP_count
};

enum Pos_Update_Rule{
    PUR_none,
    PUR_unget_whitespace
};

lexer_link Lex_PP_State
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

lexer_link Cpp_Token_Merge
cpp_attempt_token_merge(Cpp_Token prev_token, Cpp_Token next_token){
    Cpp_Token_Merge result = {(Cpp_Token_Type)0};
	if (next_token.type == CPP_TOKEN_COMMENT && prev_token.type == CPP_TOKEN_COMMENT &&
		next_token.flags == prev_token.flags && next_token.state_flags == prev_token.state_flags){
		result.did_merge = 1;
		prev_token.size = next_token.start + next_token.size - prev_token.start;
		result.new_token = prev_token;
	}
	else if (next_token.type == CPP_TOKEN_JUNK && prev_token.type == CPP_TOKEN_JUNK &&
			 next_token.flags == prev_token.flags && next_token.state_flags == prev_token.state_flags){
		result.did_merge = 1;
		prev_token.size = next_token.start + next_token.size - prev_token.start;
		result.new_token = prev_token;
	}
	return result;
}

lexer_link void
cpp_push_token_nonalloc(Cpp_Token *out_tokens, int *token_i, Cpp_Token token){
    Cpp_Token_Merge merge = {(Cpp_Token_Type)0};
    Cpp_Token prev_token = {(Cpp_Token_Type)0};
    
    if (*token_i > 0){
        prev_token = out_tokens[*token_i - 1];
        merge = new_lex::cpp_attempt_token_merge(prev_token, token);
        if (merge.did_merge){
            out_tokens[*token_i - 1] = merge.new_token;
        }
    }
    
    if (!merge.did_merge){
        out_tokens[(*token_i)++] = token;
    }
}

struct Whitespace_FSM{
    unsigned char pp_state;
    unsigned char white_done;
};

struct Lex_FSM{
    unsigned char state;
    unsigned char int_state;
    unsigned char emit_token;
    unsigned char multi_line;
    unsigned char completed;
};

struct Lex_Data{
    char *tb;
    int tb_pos;
    int token_start;
    
    int pos;
    int pos_overide;
    
    Lex_FSM fsm;
    Whitespace_FSM wfsm;
    unsigned char pp_state;
    unsigned char completed;
    
    Cpp_Token token;
    
    int __pc__;
};

Whitespace_FSM
whitespace_skip_fsm(Whitespace_FSM wfsm, char c){
    if (wfsm.pp_state != LSPP_default){
        if (c == '\n') wfsm.pp_state = LSPP_default;
    }
    if (!(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v')){
        wfsm.white_done = 1;
    }
    return(wfsm);
}

Lex_FSM
int_fsm(Lex_FSM fsm, char c){
    switch (fsm.int_state){
        case LSINT_default:
        switch (c){
            case 'u': case 'U': fsm.int_state = LSINT_u; break;
            case 'l': fsm.int_state = LSINT_l; break;
            case 'L': fsm.int_state = LSINT_L; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_u:
        switch (c){
            case 'l': fsm.int_state = LSINT_ul; break;
            case 'L': fsm.int_state = LSINT_uL; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_l:
        switch (c){
            case 'l': fsm.int_state = LSINT_ll; break;
            case 'U': case 'u': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_L:
        switch (c){
            case 'L': fsm.int_state = LSINT_ll; break;
            case 'U': case 'u': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_ul:
        switch (c){
            case 'l': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_uL:
        switch (c){
            case 'L': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_ll:
        switch (c){
            case 'u': case 'U': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;

        case LSINT_extra:
        fsm.emit_token = 1;
        break;
    }
    return(fsm);
}

Lex_FSM
main_fsm(Lex_FSM fsm, unsigned char pp_state, char c){
    if (c == 0) fsm.emit_token = 1;
    else
        switch (pp_state){
        case LSPP_error:
        fsm.state = LS_error_message;
        if (c == '\n') fsm.emit_token = 1;
        break;

        case LSPP_include:
        switch (fsm.state){
            case LSINC_default:
            switch (c){
                case '"': fsm.state = LSINC_quotes; break;
                case '<': fsm.state = LSINC_pointy; break;
                default: fsm.state = LSINC_junk; break;
            }
            break;

            case LSINC_quotes:
            if (c == '"') fsm.emit_token = 1;
            break;

            case LSINC_pointy:
            if (c == '>') fsm.emit_token = 1;
            break;

            case LSINC_junk:
            if (c == '\n') fsm.emit_token = 1;
            break;
        }
        break;

        default:
        switch (fsm.state){
            case LS_default:
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'){
                fsm.state = LS_identifier;
            }
            else if (c >= '1' && c <= '9'){
                fsm.state = LS_number;
            }
            else if (c == '0'){
                fsm.state = LS_number0;
            }
            else switch (c){
                case '\'': fsm.state = LS_char; break;
                case '"': fsm.state = LS_string; break;

                case '/': fsm.state = LS_comment_pre; break;

                case '.': fsm.state = LS_dot; break;

                case '<': fsm.state = LS_less; break;
                case '>': fsm.state = LS_more; break;

                case '-': fsm.state = LS_minus; break;

                case '&': fsm.state = LS_and; break;
                case '|': fsm.state = LS_or; break;

                case '+': fsm.state = LS_plus; break;

                case ':': fsm.state = LS_colon; break;

                case '*': fsm.state = LS_star; break;

                case '%': fsm.state = LS_modulo; break;
                case '^': fsm.state = LS_caret; break;

                case '=': fsm.state = LS_eq; break;
                case '!': fsm.state = LS_bang; break;

                case '#': fsm.state = LS_pound; break;

#define OperCase(op,type) case op: fsm.emit_token = 1; break;
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
                OperCase('$', CPP_TOKEN_JUNK);
                OperCase('\\', CPP_TOKEN_JUNK);
#undef OperCase
            }
            break;

            case LS_identifier:
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')){
                fsm.emit_token = 1;
            }
            break;

            case LS_pound:
            if (pp_state == LSPP_default){
                if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v'){
                    fsm.state = LS_pound;
                }
                else if (c == '\n'){
                    fsm.emit_token = 1;
                }
                else{
                    fsm.state = LS_pp;
                }
            }
            else{
                switch (c){
                    case '#': fsm.emit_token = 1; break;
                    default: fsm.emit_token = 1; break;
                }
            }
            break;

            case LS_pp:
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')){
                fsm.emit_token = 1;
            }
            break;

            case LS_char:
            switch(c){
                case '\'': fsm.emit_token = 1; break;
                case '\\': fsm.state = LS_char_slashed; break;
            }
            break;

            case LS_char_slashed:
            switch (c){
                case '\r': case '\f': case '\v': break;
                case '\n': fsm.state = LS_string; fsm.multi_line |= 1; break;
                default: fsm.state = LS_char; break;
            }
            break;

            case LS_string:
            switch(c){
                case '\"': fsm.emit_token = 1; break;
                case '\\': fsm.state = LS_string_slashed; break;
            }
            break;

            case LS_string_slashed:
            switch (c){
                case '\r': case '\f': case '\v': break;
                case '\n': fsm.state = LS_string; fsm.multi_line |= 1; break;
                default: fsm.state = LS_string; break;
            }
            break;

            case LS_number:
            if (c >= '0' && c <= '9'){
                fsm.state = LS_number;
            }
            else{
                switch (c){
                    case '.': fsm.state = LS_float; break;
                    default: fsm.emit_token = 1; break;
                }
            }
            break;

            case LS_number0:
            if (c >= '0' && c <= '9'){
                fsm.state = LS_number;
            }
            else if (c == 'x'){
                fsm.state = LS_hex;
            }
            else if (c == '.'){
                fsm.state = LS_float;
            }
            else{
                fsm.emit_token = 1;
            }
            break;

            case LS_float:
            if (!(c >= '0' && c <= '9')){
                switch (c){
                    case 'e': fsm.state = LS_crazy_float0; break;
                    default: fsm.emit_token = 1; break;
                }
            }
            break;

            case LS_crazy_float0:
            {
                if ((c >= '0' && c <= '9') || c == '-'){
                    fsm.state = LS_crazy_float1;
                }
                else{
                    fsm.emit_token = 1;
                }
            }
            break;

            case LS_crazy_float1:
            {
                if (!(c >= '0' && c <= '9')){
                    fsm.emit_token = 1;
                }
            }
            break;

            case LS_hex:
            if (!(c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F')){
                fsm.emit_token = 1;
            }
            break;

            case LS_dot:
            if (c >= '0' && c <= '9'){
                fsm.state = LS_float;
            }
            else
                switch (c){
                case '.': fsm.state = LS_ellipsis; break;
                case '*': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_ellipsis: fsm.emit_token = 1; break;

            case LS_less:
            switch (c){
                case '<': fsm.state = LS_less_less; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_less_less:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_more:
            switch (c){
                case '>': fsm.state = LS_more_more; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_more_more:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_comment_pre:
            switch (c){
                case '/': fsm.state = LS_comment; break;
                case '*': fsm.state = LS_comment_block; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_comment:
            switch (c){
                case '\\': fsm.state = LS_comment_slashed; break;
                case '\n': fsm.emit_token = 1; break;
            }
            break;

            case LS_comment_slashed:
            switch (c){
                case '\r': case '\f': case '\v': break;
                default: fsm.state = LS_comment; break;
            }
            break;

            case LS_comment_block:
            switch (c){
                case '*': fsm.state = LS_comment_block_ending; break;
            }
            break;

            case LS_comment_block_ending:
            switch (c){
                case '*': fsm.state = LS_comment_block_ending; break;
                case '/': fsm.emit_token = 1; break;
                default: fsm.state = LS_comment_block; break;
            }
            break;

            case LS_minus:
            switch (c){
                case '>': fsm.state = LS_arrow; break;
                case '-': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_arrow:
            switch (c){
                case '*': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_and:
            switch (c){
                case '&': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_or:
            switch (c){
                case '|': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_plus:
            switch (c){
                case '+': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_colon:
            switch (c){
                case ':': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_star:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_modulo:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_caret:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_eq:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;

            case LS_bang:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
        }
        break;
    }
    return(fsm);
}

#define DrCase(PC) case PC: goto resumespot_##PC

#define DrYield(PC, n) {\
    token_stack_out->count = token_i;\
    *S_ptr = S; S_ptr->__pc__ = PC; return(n); resumespot_##PC:; }

#define DrReturn(n) {\
    token_stack_out->count = token_i;\
    *S_ptr = S; S_ptr->__pc__ = -1; return(n); }

lexer_link int
cpp_lex_nonalloc(Lex_Data *S_ptr, char *chunk, int size, Cpp_Token_Stack *token_stack_out){
    Lex_Data S = *S_ptr;

    Cpp_Token *out_tokens = token_stack_out->tokens;
    int token_i = token_stack_out->count;
    int max_token_i = token_stack_out->max_count;

    Lex_FSM fsm = {0};
    Pos_Update_Rule pos_update_rule = PUR_none;

    char c = 0;

    int end_pos = size + S.pos;
    chunk -= S.pos;

    switch (S.__pc__){
        DrCase(1);
        DrCase(2);
        DrCase(3);
        DrCase(4);
        DrCase(5);
    }
    
    for (;;){
        S.wfsm.white_done = 0;
        S.wfsm.pp_state = S.pp_state;
        for(;;){
            for (; S.wfsm.white_done == 0 && S.pos < end_pos;){
                c = chunk[S.pos++];
                S.wfsm = whitespace_skip_fsm(S.wfsm, c);
            }
            if (S.wfsm.white_done == 0){
                DrYield(4, 1);
            }
            else break;
        }
        --S.pos;
        S.pp_state = S.wfsm.pp_state;
        
        S.token_start = S.pos;
        S.tb_pos = 0;
        S.fsm = {0};
        for(;;){
            for (; S.fsm.emit_token == 0 && S.pos < end_pos;){
                c = chunk[S.pos++];
                S.tb[S.tb_pos++] = c;

                fsm = S.fsm;
                fsm = main_fsm(fsm, S.pp_state, c);
                S.fsm = fsm;
            }
            if (S.fsm.emit_token == 0){
                DrYield(3, 1);
            }
            else break;
        }
        
        Assert(S.fsm.emit_token == 1);
        
        if (c != 0){
            pos_update_rule = PUR_none;
            if (S.pp_state == LSPP_include){
                switch (S.fsm.state){
                    case LSINC_default:break;

                    case LSINC_quotes:
                    case LSINC_pointy:
                    S.token.type = CPP_TOKEN_INCLUDE_FILE;
                    S.token.flags = 0;
                    break;

                    case LSINC_junk:
                    S.token.type = CPP_TOKEN_JUNK;
                    S.token.flags = 0;
                    break;
                }
            }
            else switch (S.fsm.state){
                case LS_default:
                switch (c){
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
                    OperCase('$', CPP_TOKEN_JUNK);
#undef OperCase

                    case '\\':
                    if (S.pp_state == LSPP_default){
                        S.token.type = CPP_TOKEN_JUNK;
                    }
                    else{
                        S.pos_overide = S.pos;
                        S.wfsm.white_done = 0;
                        for (;;){
                            for (; S.wfsm.white_done == 0 && S.pos < end_pos;){
                                c = chunk[S.pos++];
                                if (!(c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f')) S.wfsm.white_done = 1;
                            }
                            
                            if (S.wfsm.white_done == 0){
                                DrYield(1, 1);
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
                if (c != '@' && c != '$' && c != '\\'){
                    S.token.flags = CPP_TFLAG_IS_OPERATOR;
                }
                break;

                case LS_identifier:
                {
                    --S.pos;

                    int word_size = S.pos - S.token_start;
                    
                    if (S.pp_state == LSPP_body_if){
                        if (match(make_string(S.tb, word_size), make_lit_string("defined"))){
                            S.token.type = CPP_TOKEN_DEFINED;
                            S.token.flags = CPP_TFLAG_IS_OPERATOR | CPP_TFLAG_IS_KEYWORD;
                            break;
                        }
                    }

                    Sub_Match_List_Result sub_match;
                    sub_match = sub_match_list(S.tb, S.tb_pos, 0, bool_lits, word_size);

                    if (sub_match.index != -1){
                        S.token.type = CPP_TOKEN_BOOLEAN_CONSTANT;
                        S.token.flags = CPP_TFLAG_IS_KEYWORD;
                    }
                    else{
                        sub_match = sub_match_list(S.tb, S.tb_pos, 0, keywords, word_size);

                        if (sub_match.index != -1){
                            String_And_Flag data = keywords.data[sub_match.index];
                            S.token.type = (Cpp_Token_Type)data.flags;
                            S.token.flags = CPP_TFLAG_IS_KEYWORD;
                        }
                        else{
                            S.token.type = CPP_TOKEN_IDENTIFIER;
                            S.token.flags = 0;
                        }
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
                    --S.pos;
                    int start = 1;

                    c = S.tb[start];
                    while (start < S.tb_pos && (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f')){
                        ++start;
                        c = S.tb[start];
                    }

                    int word_size = S.tb_pos - start - 1;
                    Sub_Match_List_Result match;
                    match = sub_match_list(S.tb, S.tb_pos, start, preprops, word_size);

                    if (match.index != -1){
                        String_And_Flag data = preprops.data[match.index];
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
                
                {
                    S.fsm.emit_token = 0;
                    --S.pos;
                    for (;;){
                        for (; S.fsm.emit_token == 0 && S.pos < end_pos;){
                            c = chunk[S.pos++];
                            S.fsm = int_fsm(S.fsm, c);
                        }
                        if (S.fsm.emit_token == 0){
                            DrYield(5, 1);
                        }
                        else break;
					}
                    --S.pos;
				}
                
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
                    default: --S.pos; break;
				}
                break;
                
                case LS_char:
                S.token.type = CPP_TOKEN_CHARACTER_CONSTANT;
                S.token.flags = 0;
                break;

                case LS_string:
                S.token.type = CPP_TOKEN_STRING_CONSTANT;
                S.token.flags = 0;
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

                case LS_comment: case LS_comment_block_ending:
                S.token.type = CPP_TOKEN_COMMENT;
                S.token.flags = 0;
                pos_update_rule = PUR_unget_whitespace;
                break;

                case LS_error_message:
                S.token.type = CPP_TOKEN_ERROR_MESSAGE;
                S.token.flags = 0;
                pos_update_rule = PUR_unget_whitespace;
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
            
            switch (pos_update_rule){
                case PUR_unget_whitespace:
                c = chunk[--S.pos];
                while (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f'){
                    c = chunk[--S.pos];
                }
                ++S.pos;
                break;
            }
            
            if ((S.token.flags & CPP_TFLAG_PP_DIRECTIVE) == 0){
                switch (S.pp_state){
                    case LSPP_include:
                    if (S.token.type != CPP_TOKEN_INCLUDE_FILE){
                        S.token.type = CPP_TOKEN_JUNK;
                    }
                    S.pp_state = LSPP_junk;
                    break;
                    
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
                    S.token.type = CPP_TOKEN_JUNK;
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
                S.token.flags |= (S.fsm.multi_line)?(CPP_TFLAG_MULTILINE):(0);
                if ((S.token.flags & CPP_TFLAG_PP_DIRECTIVE) == 0){
                    S.token.flags |= (S.pp_state != LSPP_default)?(CPP_TFLAG_PP_BODY):(0);
                }
                S.token.state_flags = S.pp_state;
                
                cpp_push_token_nonalloc(out_tokens, &token_i, S.token);
                if (token_i == max_token_i){
                    DrYield(2, 2);
                }
            }
        }
        // NOTE(allen): else case for "if (c != 0) {...}
        else{
            S.completed = 1;
            break;
        }
    }
    
    DrReturn(0);
}

#undef DrYield
#undef DrReturn
#undef DrCase

}

#endif

// BOTTOM
