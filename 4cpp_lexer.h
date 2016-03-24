/* "4cpp" Open C++ Parser v0.1: Lexer
   no warranty implied; use at your own risk

NOTES ON USE:
   OPTIONS:
   Set options by defining macros before including this file.

   FCPP_LEXER_IMPLEMENTATION - causes this file to output function implementations
                             - this option is unset after use so that future includes of this file
                               in the same unit do not continue to output implementations

   FCPP_NO_MALLOC - prevent including <stdlib.h>
   FCPP_NO_ASSERT - prevent including <assert.h>
   FCPP_NO_STRING - prevent including <string.h>
   FCPP_NO_CRT - FCPP_NO_MALLOC & FCPP_NO_ASSERT & FCPP_NO_STRING

   FCPP_FORBID_MALLOC - one step above *NO_MALLOC with this set 4cpp functions that do allocations
                         are not allowed to be declared or defined at all, forcing the user to handle
                         allocation themselves
                      - implies FCPP_NO_MALLOC

   FCPP_GET_MEMORY - defines how to make allocations, interface of malloc, defaults to malloc
   FCPP_FREE_MEMORY - defines how to free memory, interface of ree, defaults to free
   (The above must be defined if FCPP_NO_MALLOC is set, unless FCPP_FORBID_MALLOC is set)

   FCPP_ASSERT - defines how to make assertions, interface of assert, defaults to assert

   FCPP_MEM_COPY - defines how to copy blocks of memory, interface of memcpy, defaults to memcpy
   FCPP_MEM_MOVE - defines how to move blocks of memory, interface of memmove, defaults to memmove
   (The above must be defined if FCPP_NO_STRING is set)

   FCPP_LINK - defines linkage of non-inline functions, defaults to static
   FCPP_EXTERN - changes FCPP_LINK default to extern, this option is ignored if FCPP_LINK is defined

   include the file "4cpp_clear_config.h" if you want to undefine all options for some reason

   HIDDDEN DEPENDENCIES:
   4cpp is not a single file include library, there are dependencies between the files.
   Be sure to include these dependencies before 4cpp_lexer.h:

   4cpp_types.h
   4cpp_string.h
*/

// TOP
// TODO(allen):
// 
// EASE OF USE AND DEPLOYMENT
// - make it easier to locate the list of function declarations
// - more C compatibility
// 
// POTENTIAL
// - Experiment with optimizations. Sean's State machine?
// - Reserve 0th token for null? Put a EOF token at the end?
// - Pass Cpp_File and Cpp_Token_Stack by value instead of by pointer?
// 
// CURRENT
// - lex in chunks
// 

#include "4coder_config.h"

#ifndef FCPP_LEXER_INC
#define FCPP_LEXER_INC

#include "4cpp_lexer_types.h"

Cpp_File
data_as_cpp_file(Data data){
    Cpp_File result;
    result.data = (char*)data.data;
    result.size = data.size;
    return(result);
}

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

inline fcpp_u16
cpp_token_set_pp_state(fcpp_u16 bitfield, Cpp_Preprocessor_State state_value){
    return (fcpp_u16)state_value;
}

inline Cpp_Preprocessor_State
cpp_token_get_pp_state(fcpp_u16 bitfield){
    return (Cpp_Preprocessor_State)(bitfield);
}

inline String
cpp_get_lexeme(char *str, Cpp_Token *token){
    String result;
    result.str = str + token->start;
    result.size = token->size;
    return result;
}

inline bool
is_keyword(Cpp_Token_Type type){
    return (type >= CPP_TOKEN_KEY_TYPE && type <= CPP_TOKEN_KEY_OTHER);
}

FCPP_LINK Sub_Match_List_Result sub_match_list(Cpp_File file, int pos, String_List list, int sub_size);

FCPP_LINK Seek_Result seek_unescaped_eol(char *data, int size, int pos);
FCPP_LINK Seek_Result seek_unescaped_delim(char *data, int size, int pos, char delim);
FCPP_LINK Seek_Result seek_block_comment_end(char *data, int size, int pos);

FCPP_LINK Cpp_Read_Result cpp_read_whitespace(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_junk_line(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_operator(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_pp_operator(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_alpha_numeric(Cpp_File file, int pos, bool in_if_body);
inline    Cpp_Read_Result cpp_read_alpha_numeric(Cpp_File file, int pos) { return cpp_read_alpha_numeric(file, pos, 0); }
FCPP_LINK Cpp_Read_Result cpp_read_number(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_string_litteral(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_character_litteral(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_line_comment(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_block_comment(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_preprocessor(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_pp_include_file(Cpp_File file, int pos);
FCPP_LINK Cpp_Read_Result cpp_read_pp_default_mode(Cpp_File file, int pos, bool in_if_body);
inline    Cpp_Read_Result cpp_read_pp_default_mode(Cpp_File file, int pos) { return cpp_read_pp_default_mode(file, pos, 0); }

FCPP_LINK Cpp_Token_Merge cpp_attempt_token_merge(Cpp_Token prev, Cpp_Token next);

FCPP_LINK bool cpp_push_token_no_merge(Cpp_Token_Stack *stack, Cpp_Token token);
FCPP_LINK bool cpp_push_token_nonalloc(Cpp_Token_Stack *stack, Cpp_Token token);

FCPP_LINK Cpp_Read_Result cpp_lex_step(Cpp_File file, Cpp_Lex_Data *lex);

FCPP_LINK int            cpp_lex_file_token_count(Cpp_File file);
FCPP_LINK Cpp_Lex_Data   cpp_lex_file_nonalloc(Cpp_File file, Cpp_Token_Stack *stack, Cpp_Lex_Data data);
inline    Cpp_Lex_Data   cpp_lex_file_nonalloc(Cpp_File file, Cpp_Token_Stack *stack) { return cpp_lex_file_nonalloc(file, stack, {}); }

FCPP_LINK Cpp_Get_Token_Result cpp_get_token(Cpp_Token_Stack *stack, int pos);

FCPP_LINK int  cpp_get_end_token(Cpp_Token_Stack *stack, int end);
FCPP_LINK void cpp_shift_token_starts(Cpp_Token_Stack *stack, int from_token, int amount);

FCPP_LINK Cpp_Relex_State cpp_relex_nonalloc_start(Cpp_File file, Cpp_Token_Stack *stack, int start, int end, int amount, int tolerance);
FCPP_LINK bool            cpp_relex_nonalloc_main(Cpp_Relex_State state, Cpp_Token_Stack *stack);

#ifndef FCPP_FORBID_MALLOC
FCPP_LINK Cpp_Token_Stack cpp_make_token_stack(int max);
FCPP_LINK void cpp_free_token_stack(Cpp_Token_Stack stack);
FCPP_LINK void cpp_resize_token_stack(Cpp_Token_Stack *stack, int new_max);

FCPP_LINK void cpp_push_token(Cpp_Token_Stack *stack, Cpp_Token token);
FCPP_LINK void cpp_lex_file(Cpp_File file, Cpp_Token_Stack *stack);
FCPP_LINK bool cpp_relex_file_limited(Cpp_File file, Cpp_Token_Stack *stack, int start_i, int end_i, int amount, int extra_tolerance);
inline    void cpp_relex_file(Cpp_File file, Cpp_Token_Stack *stack, int start_i, int end_i, int amount)
{ cpp_relex_file_limited(file, stack, start_i, end_i, amount, -1); }
#endif

#define FCPP_STRING_LIST(x) {x, FCPP_COUNT(x)}

// TODO(allen): shift towards storing in a context
FCPP_GLOBAL String_And_Flag int_suf_strings[] = {
    {"ull"}, {"ULL"},
    {"llu"}, {"LLU"},
    {"ll"}, {"LL"},
    {"l"}, {"L"},
    {"u"}, {"U"}
};

FCPP_GLOBAL String_List int_sufs = FCPP_STRING_LIST(int_suf_strings);

FCPP_GLOBAL String_And_Flag float_suf_strings[] = {
    {"f"}, {"F"},
    {"l"}, {"L"}
};
FCPP_GLOBAL String_List float_sufs = FCPP_STRING_LIST(float_suf_strings);

FCPP_GLOBAL String_And_Flag bool_lit_strings[] = {
    {"true"}, {"false"}
};
FCPP_GLOBAL String_List bool_lits = FCPP_STRING_LIST(bool_lit_strings);

FCPP_GLOBAL String_And_Flag keyword_strings[] = {
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
FCPP_GLOBAL String_List keywords = FCPP_STRING_LIST(keyword_strings);

FCPP_GLOBAL String_And_Flag op_strings[] = {
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
FCPP_GLOBAL String_List ops = FCPP_STRING_LIST(op_strings);

FCPP_GLOBAL String_And_Flag pp_op_strings[] = {
    {"##", CPP_PP_CONCAT},
    {"#", CPP_PP_STRINGIFY},
};
FCPP_GLOBAL String_List pp_ops = FCPP_STRING_LIST(pp_op_strings);

FCPP_GLOBAL String_And_Flag preprop_strings[] = {
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
FCPP_GLOBAL String_List preprops = FCPP_STRING_LIST(preprop_strings);

#undef FCPP_STRING_LIST

#endif // #ifndef FCPP_CPP_LEXER

#ifdef FCPP_LEXER_IMPLEMENTATION

#define _Assert FCPP_ASSERT
#define _TentativeAssert FCPP_ASSERT

FCPP_LINK Sub_Match_List_Result
sub_match_list(Cpp_File file, int pos, String_List list, int sub_size){
    Sub_Match_List_Result result;
    String str_main;
    char *str_check;
    int i,l;

    result.index = -1;
    result.new_pos = pos;
    str_main = make_string(file.data + pos, file.size - pos);
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

FCPP_LINK Seek_Result
seek_unescaped_eol(char *data, int size, int pos){
    Seek_Result result = {};
    ++pos;
    while (pos < size){
        if (data[pos] == '\\'){
            if (pos + 1 < size &&
                data[pos+1] == '\n'){
                result.new_line = 1;
                ++pos;
            }
            else if (pos + 1 < size &&
                     data[pos+1] == '\r' &&
                     pos + 2 < size &&
                     data[pos+2] == '\n'){
                result.new_line = 1;
                pos += 2;
            }
        }
        else if (data[pos] == '\n'){
            break;
        }
        ++pos;
    }
    ++pos;

    result.pos = pos;
    return result;
}

FCPP_LINK Seek_Result
seek_unescaped_delim(char *data, int size, int pos, char delim){
    Seek_Result result = {};
    bool escape = 0;
    ++pos;
    while (pos < size){
        if (data[pos] == '\n'){
            result.new_line = 1;
        }
        if (escape){
            escape = 0;
        }
        else{
            if (data[pos] == '\\'){
                escape = 1;
            }
            else if (data[pos] == delim){
                break;
            }
        }
        ++pos;
    }
    ++pos;

    result.pos = pos;
    return result;
}

FCPP_LINK Seek_Result
seek_block_comment_end(char *data, int size, int pos){
    Seek_Result result = {};
    pos += 2;
    while (pos < size){
        if (data[pos] == '*' &&
            pos + 1 < size &&
            data[pos+1] == '/'){
            break;
        }
        if (data[pos] == '\n'){
            result.new_line = 1;
        }
        ++pos;
    }
    pos += 2;
    result.pos = pos;
    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_whitespace(Cpp_File file, int pos){
    Cpp_Read_Result result = {};

    while (pos < file.size && char_is_whitespace(file.data[pos])){
        if (file.data[pos] == '\n'){
            result.newline = 1;
        }
        ++pos;
    }

    result.pos = pos;

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_junk_line(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.token.start = pos;
    result.token.type = CPP_TOKEN_JUNK;

    bool comment_end = 0;
    while (pos < file.size && file.data[pos] != '\n'){
        if (file.data[pos] == '/' && pos + 1 < file.size){
            if (file.data[pos + 1] == '/' ||
                file.data[pos + 1] == '*'){
                comment_end = 1;
                break;
            }
        }
        ++pos;
    }

    if (comment_end){
        result.pos = pos;
        result.token.size = pos - result.token.start;
    }
    else{
        while (pos > 0 && file.data[pos - 1] == '\r'){
            --pos;
        }
        if (pos > 0 && file.data[pos - 1] == '\\'){
            --pos;
        }
        result.pos = pos;
        result.token.size = pos - result.token.start;
    }

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_operator(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.pos = pos;
    result.token.start = pos;

    Sub_Match_List_Result match;
    match = sub_match_list(file, result.token.start, ops, -1);

    if (match.index != -1){
        result.pos = match.new_pos;
        result.token.size = result.pos - result.token.start;
        result.token.type = (Cpp_Token_Type)ops.data[match.index].flags;
        result.token.flags |= CPP_TFLAG_IS_OPERATOR;
    }
    else{
        result.token.size = 1;
        result.token.type = CPP_TOKEN_JUNK;
        result.pos = pos + 1;
    }

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_pp_operator(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.pos = pos;
    result.token.start = pos;

    Sub_Match_List_Result match;
    match = sub_match_list(file, result.token.start, pp_ops, -1);

    _Assert(match.index != -1);
    result.pos = match.new_pos;
    result.token.size = result.pos - result.token.start;
    result.token.type = (Cpp_Token_Type)pp_ops.data[match.index].flags;

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_alpha_numeric(Cpp_File file, int pos, bool in_if_body){
    Cpp_Read_Result result = {};
    result.pos = pos;
    result.token.start = pos;

    while (result.pos < file.size &&
           char_is_alpha_numeric(file.data[result.pos])){
        ++result.pos;
    }

    result.token.size = result.pos - result.token.start;

    // TODO(allen): do better
    if (in_if_body){
        String word;
        word.size = result.token.size;
        word.str = file.data + result.token.start;
        if (match(word, "defined")){
            result.token.type = CPP_TOKEN_DEFINED;
            result.token.flags |= CPP_TFLAG_IS_OPERATOR;
            result.token.flags |= CPP_TFLAG_IS_KEYWORD;
        }
    }

    if (result.token.type == CPP_TOKEN_JUNK){
        Sub_Match_List_Result match;
        match = sub_match_list(file, result.token.start, bool_lits, result.token.size);

        if (match.index != -1){
            result.token.type = CPP_TOKEN_BOOLEAN_CONSTANT;
            result.token.flags |= CPP_TFLAG_IS_KEYWORD;
        }
        else{
            match = sub_match_list(file, result.token.start, keywords, result.token.size);

            if (match.index != -1){
                String_And_Flag data = keywords.data[match.index];
                result.token.type = (Cpp_Token_Type)data.flags;
                result.token.flags |= CPP_TFLAG_IS_KEYWORD;
            }
            else{
                result.token.type = CPP_TOKEN_IDENTIFIER;
            }
        }
    }

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_number(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.pos = pos;
    result.token.start = pos;

    bool is_float = 0;
    bool is_integer = 0;
    bool is_oct = 0;
    bool is_hex = 0;
    bool is_zero = 0;

    if (file.data[pos] == '0'){
        if (pos+1 < file.size){
            char next = file.data[pos+1];
            if (next == 'x'){
                is_hex = 1;
                is_integer = 1;
            }
            else if (next == '.'){
                is_float = 1;
                ++result.pos;
            }
            else if (next >= '0' && next <= '9'){
                is_oct = 1;
                is_integer = 1;
            }
            else{
                is_zero = 1;
                is_integer = 1;
            }
        }
        else{
            is_zero = 1;
            is_integer = 1;
        }
    }
    else if (file.data[pos] == '.'){
        is_float = 1;
    }

    if (is_zero){
        ++result.pos;
    }
    else if (is_hex){
        ++result.pos;
        char character;
        do{
            ++result.pos;
            if (result.pos >= file.size){
                break;
            }
            character = file.data[result.pos];
        } while(char_is_hex(character));
    }
    else if (is_oct){
        char character;
        do{
            ++result.pos;
            if (result.pos >= file.size){
                break;
            }
            character = file.data[result.pos];
        }while(char_is_numeric(character));
    }
    else{
        if (!is_float){
            is_integer = 1;
             while (1){
                ++result.pos;

                if (result.pos >= file.size){
                    break;
                }
                bool is_good = 0;
                char character = file.data[result.pos];
                if (character >= '0' && character <= '9'){
                    is_good = 1;
                }
                else if (character == '.'){
                    is_integer = 0;
                    is_float = 1;
                }
                if (!is_good){
                    break;
                }
            }
        }

        if (is_float){
            bool e_mode = 0;
            bool e_minus = 0;
            bool is_good = 0;
            char character;

            while (1){
                ++result.pos;
                if (result.pos >= file.size){
                    break;
                }
                is_good = 0;
                character = file.data[result.pos];
                if (character >= '0' && character <= '9'){
                    is_good = 1;
                }
                else{
                    if (character == 'e' && !e_mode){
                        e_mode = 1;
                        is_good = 1;
                    }
                    else if (character == '-' && e_mode && !e_minus){
                        e_minus = 1;
                        is_good = 1;
                    }
                }
                if (!is_good){
                    break;
                }
            }
        }
    }

    if (is_integer){
        Sub_Match_List_Result match =
            sub_match_list(file, result.pos, int_sufs, -1);
        if (match.index != -1){
            result.pos = match.new_pos;
        }
        result.token.type = CPP_TOKEN_INTEGER_CONSTANT;
        result.token.size = result.pos - result.token.start;
    }
    else if (is_float){
        Sub_Match_List_Result match =
            sub_match_list(file, result.pos, float_sufs, -1);
        if (match.index != -1){
            result.pos = match.new_pos;
        }
        result.token.type = CPP_TOKEN_FLOATING_CONSTANT;
        result.token.size = result.pos - result.token.start;
    }
    else{
        _Assert(!"This shouldn't happen!");
    }

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_string_litteral(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.token.start = pos;

    _Assert(file.data[pos] == '"');
    Seek_Result seek = seek_unescaped_delim(file.data, file.size, pos, '"');
    pos = seek.pos;
    if (seek.new_line){
        result.token.flags |= CPP_TFLAG_MULTILINE;
    }

    result.token.size = pos - result.token.start;
    result.token.type = CPP_TOKEN_STRING_CONSTANT;
    result.pos = pos;

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_character_litteral(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.token.start = pos;

    _Assert(file.data[pos] == '\'');
    Seek_Result seek = seek_unescaped_delim(file.data, file.size, pos, '\'');
    pos = seek.pos;
    if (seek.new_line){
        result.token.flags |= CPP_TFLAG_MULTILINE;
    }

    result.token.size = pos - result.token.start;
    result.token.type = CPP_TOKEN_CHARACTER_CONSTANT;
    result.pos = pos;

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_line_comment(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.token.start = pos;

    _Assert(file.data[pos] == '/' && file.data[pos + 1] == '/');

    pos += 2;
    while (pos < file.size){
        if (file.data[pos] == '\n'){
            break;
        }
        if (file.data[pos] == '\\'){
            if (pos + 1 < file.size &&
                file.data[pos + 1] == '\n'){
                ++pos;
            }
            else if (pos + 2 < file.size &&
                     file.data[pos + 1] == '\r' &&
                     file.data[pos + 2] == '\n'){
                pos += 2;
            }
        }
        ++pos;
    }
    if (pos > 0 && file.data[pos-1] == '\r'){
        --pos;
    }
    result.token.size = pos - result.token.start;
    result.token.type = CPP_TOKEN_COMMENT;
    result.pos = pos;
    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_block_comment(Cpp_File file, int pos){
    Cpp_Read_Result result = {};
    result.token.start = pos;

    _Assert(file.data[pos] == '/' && file.data[pos + 1] == '*');
    pos += 2;
    while (pos < file.size){
        if (file.data[pos] == '*' &&
            pos + 1 < file.size &&
            file.data[pos+1] == '/'){
            break;
        }
        ++pos;
    }
    pos += 2;
    result.token.size = pos - result.token.start;
    result.token.type = CPP_TOKEN_COMMENT;
    result.pos = pos;
    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_preprocessor(Cpp_File file, int pos){
    _Assert(file.data[pos] == '#');
    Cpp_Read_Result result = {};
    result.token.start = pos;
    result.token.type = CPP_PP_UNKNOWN;
    result.token.flags |= CPP_TFLAG_PP_DIRECTIVE;

    ++pos;
    while (pos < file.size &&
           (file.data[pos] == ' ' ||
            file.data[pos] == '\t')){
        ++pos;
    }

    Sub_Match_List_Result match
        = sub_match_list(file, pos, preprops, -1);

    if (match.index != -1){
        result.token.size = match.new_pos - result.token.start;
        result.token.type = (Cpp_Token_Type)preprops.data[match.index].flags;
        result.pos = match.new_pos;
    }
    else{
        while (pos < file.size &&
               !char_is_whitespace(file.data[pos])){
            ++pos;
        }
        result.token.size = pos - result.token.start;
        result.pos = pos;
    }

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_pp_include_file(Cpp_File file, int pos){
    char start = file.data[pos];
    _Assert(start == '<' || start == '"');

    Cpp_Read_Result result = {};
    result.token.start = pos;
    result.token.type = CPP_TOKEN_INCLUDE_FILE;
    result.token.flags |= CPP_TFLAG_PP_BODY;

    char end;
    if (start == '<'){
        end = '>';
    }
    else{
        end = '"';
    }

    ++pos;
    while (pos < file.size && file.data[pos] != end){
        if (file.data[pos] == '\n'){
            result.token.type = CPP_TOKEN_JUNK;
            result.token.flags |= CPP_TFLAG_BAD_ENDING;
            break;
        }
        if (file.data[pos] == '\\'){
            // TODO(allen): Not sure that this is 100% correct.
            if (pos + 1 < file.size &&
                file.data[pos + 1] == '\n'){
                ++pos;
                result.token.flags |= CPP_TFLAG_MULTILINE;
            }
            else if (pos + 2 < file.size &&
                     file.data[pos + 1] == '\r' &&
                     file.data[pos + 2] == '\n'){
                pos += 2;
                result.token.flags |= CPP_TFLAG_MULTILINE;
            }
        }
        ++pos;
    }

    if (result.token.type != CPP_TOKEN_JUNK){
        if (pos < file.size){
            ++pos;
        }
    }

    result.token.size = pos - result.token.start;
    result.pos = pos;

    return result;
}

FCPP_LINK Cpp_Read_Result
cpp_read_pp_default_mode(Cpp_File file, int pos, bool in_if_body){
    char current = file.data[pos];
    Cpp_Read_Result result;
    if (char_is_numeric(current)){
        result = cpp_read_number(file, pos);
    }
    else if (char_is_alpha(current)){
        result = cpp_read_alpha_numeric(file, pos, in_if_body);
    }
    else if (current == '.'){
        if (pos + 1 < file.size){
            char next = file.data[pos + 1];
            if (char_is_numeric(next)){
                result = cpp_read_number(file, pos);
            }
            else{
                result = cpp_read_operator(file, pos);
            }
        }
        else{
            result = cpp_read_operator(file, pos);
        }
    }

    else if (current == '/'){
        if (pos + 1 < file.size){
            char next = file.data[pos + 1];
            if (next == '/'){
                result = cpp_read_line_comment(file, pos);
            }
            else if (next == '*'){
                result = cpp_read_block_comment(file, pos);
            }
            else{
                result = cpp_read_operator(file, pos);
            }
        }
        else{
            result = cpp_read_operator(file, pos);
        }
    }
    else if (current == '"'){
        result = cpp_read_string_litteral(file, pos);
    }
    else if (current == '\''){
        result = cpp_read_character_litteral(file, pos);
    }
    else{
        result = cpp_read_operator(file, pos);
    }

    return result;
}

FCPP_LINK Cpp_Token_Merge
cpp_attempt_token_merge(Cpp_Token prev_token, Cpp_Token next_token){
    Cpp_Token_Merge result = {};
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

FCPP_LINK bool
cpp_push_token_no_merge(Cpp_Token_Stack *token_stack, Cpp_Token token){
    if (token_stack->count >= token_stack->max_count){
        return 0;
    }

    token_stack->tokens[token_stack->count++] = token;
    return 1;
}

FCPP_LINK bool
cpp_push_token_nonalloc(Cpp_Token_Stack *token_stack, Cpp_Token token){
    Cpp_Token_Merge merge = {};

    if (token_stack->count > 0){
        Cpp_Token prev_token = token_stack->tokens[token_stack->count - 1];
        merge = cpp_attempt_token_merge(prev_token, token);
        if (merge.did_merge){
            token_stack->tokens[token_stack->count - 1] = merge.new_token;
        }
    }

    if (!merge.did_merge){
        if (token_stack->count >= token_stack->max_count){
            return 0;
        }

        token_stack->tokens[token_stack->count++] = token;
    }

    return 1;
}

FCPP_LINK Cpp_Read_Result
cpp_lex_step(Cpp_File file, Cpp_Lex_Data *lex_data){
    Cpp_Lex_Data lex = *lex_data;
    Cpp_Read_Result result = {};
    bool has_result = 1;

    fcpp_u16 state_flags = cpp_token_set_pp_state(0, lex.pp_state);

    char current = file.data[lex.pos];
    if (char_is_whitespace(current)){
        result = cpp_read_whitespace(file, lex.pos);
        lex.pos = result.pos;
        if (result.newline && lex.pp_state != CPP_LEX_PP_DEFAULT){
            lex.pp_state = CPP_LEX_PP_DEFAULT;
        }
        has_result = 0;
    }

    else{
        if (lex.pp_state == CPP_LEX_PP_DEFAULT){
            // TODO(allen): Not first hard of the line?  Then it's junk.
            if (current == '#'){
                result = cpp_read_preprocessor(file, lex.pos);
                lex.pos = result.pos;
                switch (result.token.type){
                case CPP_PP_INCLUDE:
                case CPP_PP_IMPORT:
                case CPP_PP_USING:
                    lex.pp_state = CPP_LEX_PP_INCLUDE;
                    break;
                case CPP_PP_DEFINE:
                    lex.pp_state = CPP_LEX_PP_MACRO_IDENTIFIER;
                    break;
                case CPP_PP_UNDEF:
                case CPP_PP_IFDEF:
                case CPP_PP_IFNDEF:
                    lex.pp_state = CPP_LEX_PP_IDENTIFIER;
                    break;
                case CPP_PP_IF:
                case CPP_PP_ELIF:
                    lex.pp_state = CPP_LEX_PP_BODY_IF;
                    break;
                case CPP_PP_PRAGMA:
                    lex.pp_state = CPP_LEX_PP_BODY;
                    break;
                case CPP_PP_LINE:
                    lex.pp_state = CPP_LEX_PP_NUMBER;
                    break;
                case CPP_PP_ERROR:
                    lex.pp_state = CPP_LEX_PP_ERROR;
                    break;

                case CPP_PP_UNKNOWN:
                case CPP_PP_ELSE:
                case CPP_PP_ENDIF:
                    lex.pp_state = CPP_LEX_PP_JUNK;
                    break;
                }
            }
            else{
                result = cpp_read_pp_default_mode(file, lex.pos);
                lex.pos = result.pos;
            }
        }

        else{
            if (current == '\\'){
                fcpp_i32 seek = lex.pos;
                ++seek;
                while (seek < file.size && file.data[seek] == '\r'){
                    ++seek;
                }
                if ((seek < file.size && file.data[seek] == '\n') || seek >= file.size){
                    lex.pos = seek + 1;
                    has_result = 0;
                }
                else{
                    lex.pp_state = CPP_LEX_PP_JUNK;
                    result.token.type = CPP_TOKEN_JUNK;
                    result.token.start = lex.pos;
                    result.token.size = 1;
                    result.token.flags |= CPP_TFLAG_PP_BODY;
                    lex.pos = seek;
                }
            }

            else{
                switch (lex.pp_state){
                case CPP_LEX_PP_IDENTIFIER:
                    if (!char_is_alpha_numeric(current)){
                        has_result = 0;
                        lex.pp_state = CPP_LEX_PP_JUNK;
                    }
                    else{
                        result = cpp_read_alpha_numeric(file, lex.pos);
                        result.token.flags |= CPP_TFLAG_PP_BODY;
                        lex.pos = result.pos;
                        lex.pp_state = CPP_LEX_PP_JUNK;
                    }
                    break;

                case CPP_LEX_PP_MACRO_IDENTIFIER:
                    if (!char_is_alpha_numeric(current)){
                        has_result = 0;
                        lex.pp_state = CPP_LEX_PP_JUNK;
                    }
                    else{
                        result = cpp_read_alpha_numeric(file, lex.pos);
                        result.token.flags |= CPP_TFLAG_PP_BODY;
                        lex.pos = result.pos;
                        lex.pp_state = CPP_LEX_PP_BODY;
                    }
                    break;

                case CPP_LEX_PP_INCLUDE:
                    if (current != '"' && current != '<'){
                        has_result = 0;
                        lex.pp_state = CPP_LEX_PP_JUNK;
                    }
                    else{
                        result = cpp_read_pp_include_file(file, lex.pos);
                        lex.pos = result.pos;
                        lex.pp_state = CPP_LEX_PP_JUNK;
                    }
                    break;

                case CPP_LEX_PP_BODY:
                    if (current == '#'){
                        result = cpp_read_pp_operator(file, lex.pos);
                    }
                    else{
                        result = cpp_read_pp_default_mode(file, lex.pos);
                    }
                    lex.pos = result.pos;
                    result.token.flags |= CPP_TFLAG_PP_BODY;
                    break;

                case CPP_LEX_PP_BODY_IF:
                    if (current == '#'){
                        result = cpp_read_pp_operator(file, lex.pos);
                    }
                    else{
                        result = cpp_read_pp_default_mode(file, lex.pos, 1);
                    }
                    lex.pos = result.pos;
                    result.token.flags |= CPP_TFLAG_PP_BODY;
                    break;

                case CPP_LEX_PP_NUMBER:
                    if (!char_is_numeric(current)){
                        has_result = 0;
                        lex.pp_state = CPP_LEX_PP_JUNK;
                    }
                    else{	
                        result = cpp_read_number(file, lex.pos);
                        lex.pos = result.pos;
                        result.token.flags |= CPP_TFLAG_PP_BODY;
                        lex.pp_state = CPP_LEX_PP_INCLUDE;
                    }
                    break;

                case CPP_LEX_PP_ERROR:
                    result = cpp_read_junk_line(file, lex.pos);
                    lex.pos = result.pos;
                    result.token.type = CPP_TOKEN_ERROR_MESSAGE;
                    result.token.flags |= CPP_TFLAG_PP_BODY;
                    break;

                default:
                {
                    bool took_comment = 0;
                    if (current == '/' && lex.pos + 1 < file.size){
                        if (file.data[lex.pos + 1] == '/'){
                            result = cpp_read_line_comment(file, lex.pos);
                            lex.pp_state = CPP_LEX_PP_DEFAULT;
                            lex.pos = result.pos;
                            took_comment = 1;
                        }else if (file.data[lex.pos + 1] == '*'){
                            result = cpp_read_block_comment(file, lex.pos);
                            lex.pos = result.pos;
                            took_comment = 1;
                        }
                    }

                    if (!took_comment){
                        result = cpp_read_junk_line(file, lex.pos);
                        lex.pos = result.pos;
                        result.token.flags |= CPP_TFLAG_PP_BODY;
                    }
                }break;

                }
            }
        }
    }

    result.token.state_flags = state_flags;
    result.has_result = has_result;

    *lex_data = lex;
    return result;
}

FCPP_LINK int
cpp_lex_file_token_count(Cpp_File file){
    int count = 0;
    Cpp_Lex_Data lex = {};
    Cpp_Token token = {};
    while (lex.pos < file.size){
        Cpp_Read_Result step_result = cpp_lex_step(file, &lex);

        if (step_result.has_result){
            if (count > 0){
                Cpp_Token_Merge merge = cpp_attempt_token_merge(token, step_result.token);
                if (merge.did_merge){
                    token = merge.new_token;
                }
                else{
                    token = step_result.token;
                    ++count;
                }
            }
            else{
                token = step_result.token;
                ++count;
            }
        }
    }
    return count;
}

FCPP_LINK Cpp_Lex_Data
cpp_lex_file_nonalloc(Cpp_File file, Cpp_Token_Stack *token_stack_out, Cpp_Lex_Data data){
    while (data.pos < file.size){
        Cpp_Lex_Data prev_lex = data;
        Cpp_Read_Result step_result = cpp_lex_step(file, &data);

        if (step_result.has_result){
            if (!cpp_push_token_nonalloc(token_stack_out, step_result.token)){
                data = prev_lex;
                return data;
            }
        }
    }

    data.complete = 1;
    return data;
}

FCPP_LINK Cpp_Get_Token_Result
cpp_get_token(Cpp_Token_Stack *token_stack, int pos){
    int first, last;
    first = 0;
    last = token_stack->count;

    Cpp_Get_Token_Result result = {};
    if (token_stack->count > 0){
        for (;;){
            result.token_index = (first + last)/2;

            int this_start = token_stack->tokens[result.token_index].start;
            int next_start;
            if (result.token_index + 1 < token_stack->count){
                next_start = token_stack->tokens[result.token_index+1].start;
            }
            else{
                next_start = this_start + token_stack->tokens[result.token_index].size;
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

        if (result.token_index == token_stack->count){
            --result.token_index;
            result.in_whitespace = 1;
        }
        else{
            Cpp_Token *token = token_stack->tokens + result.token_index;
            if (token->start + token->size <= pos){
                result.in_whitespace = 1;
            }
        }
    }
    else{
        result.token_index = -1;
        result.in_whitespace = 1;
    }

    return result;
}

FCPP_LINK void
cpp_shift_token_starts(Cpp_Token_Stack *stack, int from_token_i, int amount){
    int count = stack->count;
    Cpp_Token *token = stack->tokens + from_token_i;
    for (int i = from_token_i; i < count; ++i, ++token){
        token->start += amount;
    }
}

FCPP_LINK Cpp_Relex_State
cpp_relex_nonalloc_start(Cpp_File file, Cpp_Token_Stack *stack,
                         int start, int end, int amount, int tolerance){
    Cpp_Relex_State state;
    state.file = file;
    state.stack = stack;
    state.start = start;
    state.end = end;
    state.amount = amount;
    state.tolerance = tolerance;

    Cpp_Get_Token_Result result = cpp_get_token(stack, start);
    if (result.token_index <= 0){
        state.start_token_i = 0;
    }
    else{
        state.start_token_i = result.token_index-1;
    }

    result = cpp_get_token(stack, end);
    if (result.token_index < 0) result.token_index = 0;
    else if (end > stack->tokens[result.token_index].start) ++result.token_index;
    state.end_token_i = result.token_index;

    state.relex_start = stack->tokens[state.start_token_i].start;
    if (start < state.relex_start) state.relex_start = start;

    state.space_request = state.end_token_i - state.start_token_i + tolerance + 1;

    return state;
}

inline Cpp_Token
cpp__get_token(Cpp_Token_Stack *stack, Cpp_Token *tokens, int size, int index){
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

FCPP_LINK bool
cpp_relex_nonalloc_main(Cpp_Relex_State *state, Cpp_Token_Stack *relex_stack, int *relex_end){
    Cpp_Token_Stack *stack = state->stack;
    Cpp_Token *tokens = stack->tokens;

    cpp_shift_token_starts(stack, state->end_token_i, state->amount);

    Cpp_Lex_Data lex = {};
    lex.pp_state = cpp_token_get_pp_state(tokens[state->start_token_i].state_flags);
    lex.pos = state->relex_start;

    int relex_end_i = state->end_token_i;
    Cpp_Token match_token = cpp__get_token(stack, tokens, state->file.size, relex_end_i);
    Cpp_Token end_token = match_token;
    bool went_too_far = 0;

    for (;;){
        Cpp_Read_Result read = cpp_lex_step(state->file, &lex);
        if (read.has_result){
            if (read.token.start == end_token.start &&
                read.token.size == end_token.size &&
                read.token.flags == end_token.flags &&
                read.token.state_flags == end_token.state_flags){
                break;
            }
            cpp_push_token_nonalloc(relex_stack, read.token);

            while (lex.pos > end_token.start && relex_end_i < stack->count){
                ++relex_end_i;
                end_token = cpp__get_token(stack, tokens, state->file.size, relex_end_i);
            }
            if (relex_stack->count == relex_stack->max_count){
                went_too_far = 1;
                break;
            }
        }
        if (lex.pos >= state->file.size) break;
    }

    if (!went_too_far){
        if (relex_stack->count > 0){
            if (state->start_token_i > 0){
                Cpp_Token_Merge merge =
                    cpp_attempt_token_merge(tokens[state->start_token_i - 1],
                                            relex_stack->tokens[0]);
                if (merge.did_merge){
                    --state->start_token_i;
                    relex_stack->tokens[0] = merge.new_token;
                }
            }

            if (relex_end_i < state->stack->count){
                Cpp_Token_Merge merge =
                    cpp_attempt_token_merge(relex_stack->tokens[relex_stack->count-1],
                                            tokens[relex_end_i]);
                if (merge.did_merge){
                    ++relex_end_i;
                    relex_stack->tokens[relex_stack->count-1] = merge.new_token;
                }
            }
        }

        *relex_end = relex_end_i;
    }
    else{
        cpp_shift_token_starts(stack, state->end_token_i, -state->amount);
    }

    return went_too_far;
}

#ifndef FCPP_FORBID_MALLOC
FCPP_LINK Cpp_Token_Stack
cpp_make_token_stack(int starting_max){
    Cpp_Token_Stack token_stack;
    token_stack.count = 0;
    token_stack.max_count = starting_max;
    token_stack.tokens = (Cpp_Token*)FCPP_GET_MEMORY(sizeof(Cpp_Token)*starting_max);
    return token_stack;
}

FCPP_LINK void
cpp_free_token_stack(Cpp_Token_Stack token_stack){
    FCPP_FREE_MEMORY(token_stack.tokens);
}

FCPP_LINK void
cpp_resize_token_stack(Cpp_Token_Stack *token_stack, int new_max){
    Cpp_Token *new_tokens = (Cpp_Token*)FCPP_GET_MEMORY(sizeof(Cpp_Token)*new_max);

    if (new_tokens){
        FCPP_MEM_COPY(new_tokens, token_stack->tokens, sizeof(Cpp_Token)*token_stack->count);
        FCPP_FREE_MEMORY(token_stack->tokens);
        token_stack->tokens = new_tokens;
        token_stack->max_count = new_max;
    }	
}

FCPP_LINK void
cpp_push_token(Cpp_Token_Stack *token_stack, Cpp_Token token){
    if (!cpp_push_token_nonalloc(token_stack, token)){
        int new_max = 2*token_stack->max_count + 1;
        cpp_resize_token_stack(token_stack, new_max);
        bool result = cpp_push_token_nonalloc(token_stack, token);
        _Assert(result);
    }
}

FCPP_LINK void
cpp_lex_file(Cpp_File file, Cpp_Token_Stack *token_stack_out){
    Cpp_Lex_Data lex = {};
    while (lex.pos < file.size){
        Cpp_Read_Result step_result = cpp_lex_step(file, &lex);
        if (step_result.has_result){
            cpp_push_token(token_stack_out, step_result.token);
        }
    }
}

FCPP_LINK bool
cpp_relex_file_limited(Cpp_File file, Cpp_Token_Stack *stack,
                       int start, int end, int amount, int tolerance){
#if 0
    int start_token_i, end_token_i;
    Cpp_Get_Token_Result get_result = cpp_get_token(token_stack, start_i);
    start_token_i = get_result.token_index;
    get_result = cpp_get_token(token_stack, end_i);
    end_token_i = get_result.token_index;
    if (end_token_i == -1){
        end_token_i = 0;
    }
    else if (end > token_stack->tokens[end_token_i].start){
        ++end_token_i;
    }
    cpp_shift_token_starts(token_stack, end_token_i, amount);

    int relex_start_i = start_token_i - 1;
    if (relex_start_i < 0){
        relex_start_i = 0;
    }

    int end_guess_i = end_token_i + 1;
    if (end_guess_i > token_stack->count){
        --end_guess_i;
    }
#endif

    int relex_start_i;
    int end_token_i, end_guess_i;
    {
        Cpp_Get_Token_Result result = cpp_get_token(stack, start);
        if (result.token_index <= 0){
            relex_start_i = 0;
        }
        else{
            relex_start_i = result.token_index-1;
        }

        result = cpp_get_token(stack, end);
        if (result.token_index < 0) result.token_index = 0;
        else if (end > stack->tokens[result.token_index].start) ++result.token_index;
        end_token_i = result.token_index;
        end_guess_i = result.token_index+1;
    }

    int relex_start = stack->tokens[relex_start_i].start;
    if (start < relex_start) relex_start = start;

    cpp_shift_token_starts(stack, end_token_i, amount);
    Cpp_Token_Stack relex_stack = cpp_make_token_stack((end_guess_i - relex_start_i + 1) * 3 / 2);
    Cpp_Lex_Data lex = {};
    lex.pp_state = cpp_token_get_pp_state(stack->tokens[relex_start_i].state_flags);
    lex.pos = relex_start;
    bool went_too_far = 0;

    while (1){
        Cpp_Read_Result result = cpp_lex_step(file, &lex);
        if (result.has_result){
            if (end_guess_i < stack->count &&
                result.token.start == stack->tokens[end_guess_i].start &&
                result.token.size == stack->tokens[end_guess_i].size &&
                result.token.flags == stack->tokens[end_guess_i].flags &&
                result.token.state_flags == stack->tokens[end_guess_i].state_flags){
                break;
            }
            else{
                cpp_push_token(&relex_stack, result.token);
                while (lex.pos > stack->tokens[end_guess_i].start &&
                       end_guess_i < stack->count){
                    ++end_guess_i;
                }
            }
        }

        if (lex.pos >= file.size){
            break;
        }

        if (tolerance >= 0 && relex_stack.count + relex_start_i >= end_guess_i + tolerance){
            went_too_far = 1;
            break;
        }
    }

    if (!went_too_far){
        int relex_end_i = end_guess_i;

        if (relex_stack.count > 0){
            if (relex_start_i > 0){
                Cpp_Token_Merge merge = cpp_attempt_token_merge(stack->tokens[relex_start_i - 1],
                                                                relex_stack.tokens[0]);
                if (merge.did_merge){
                    --relex_start_i;
                    relex_stack.tokens[0] = merge.new_token;
                }
            }

            if (relex_end_i < stack->count){
                Cpp_Token_Merge merge = cpp_attempt_token_merge(relex_stack.tokens[relex_stack.count - 1],
                                                                stack->tokens[relex_end_i]);
                if (merge.did_merge){
                    ++relex_end_i;
                    relex_stack.tokens[relex_stack.count - 1] = merge.new_token;
                }
            }
        }

        int token_delete_amount = relex_end_i - relex_start_i;
        int token_shift_amount = relex_stack.count - token_delete_amount;

        if (token_shift_amount != 0){
            int new_token_count = stack->count + token_shift_amount;
            if (new_token_count > stack->max_count){
                int new_max = 2*stack->max_count + 1;
                while (new_token_count > new_max){
                    new_max = 2*new_max + 1;
                }
                cpp_resize_token_stack(stack, new_max);
            }

            if (relex_end_i < stack->count){
                FCPP_MEM_MOVE(stack->tokens + relex_end_i + token_shift_amount,
                              stack->tokens + relex_end_i, sizeof(Cpp_Token)*(stack->count - relex_end_i));
            }

            stack->count += token_shift_amount;
        }

        FCPP_MEM_COPY(stack->tokens + relex_start_i, relex_stack.tokens, sizeof(Cpp_Token)*relex_stack.count);
        cpp_free_token_stack(relex_stack);
    }

    else{
        cpp_shift_token_starts(stack, end_token_i, -amount);
        cpp_free_token_stack(relex_stack);
    }

    return went_too_far;
}
#endif

#undef _Assert
#undef _TentativeAssert

#undef FCPP_LEXER_IMPLEMENTATION
#endif // #ifdef FCPP_LEXER_IMPLEMENTATION

// BOTTOM
