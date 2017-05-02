
// TOP

#ifndef FCPP_LEXER_TYPES_INC
#define FCPP_LEXER_TYPES_INC

#ifndef ENUM
#define ENUM(type,name) typedef type name; enum name##_
#endif

#ifndef ENUM_INTERNAL
#define ENUM_INTERNAL(type,name) typedef type name; enum name##_
#endif

#ifndef STRUCT
#define STRUCT struct
#endif

/* DOC(A Cpp_Token_Type classifies a token to make parsing easier. Some types are not actually output by the lexer, but exist because parsers will also make use of token types in their own output.) */
ENUM(uint32_t, Cpp_Token_Type){
    CPP_TOKEN_JUNK = 0,
    CPP_TOKEN_COMMENT = 1,
    
    CPP_PP_INCLUDE = 2,
    CPP_PP_VERSION = 3,
    CPP_PP_DEFINE = 4,
    CPP_PP_UNDEF = 5,
    CPP_PP_IF = 6,
    CPP_PP_IFDEF = 7,
    CPP_PP_IFNDEF = 8,
    CPP_PP_ELSE = 9,
    CPP_PP_ELIF = 10,
    CPP_PP_ENDIF = 11,
    CPP_PP_ERROR = 12,
    CPP_PP_IMPORT = 13,
    CPP_PP_USING = 14,
    CPP_PP_LINE = 15,
    CPP_PP_PRAGMA = 16,
    CPP_PP_STRINGIFY = 17,
    CPP_PP_CONCAT = 18,
    CPP_PP_UNKNOWN = 19,
    
    CPP_PP_DEFINED = 20,
    CPP_PP_INCLUDE_FILE = 21,
    CPP_PP_ERROR_MESSAGE = 22,
    
    CPP_TOKEN_KEY_TYPE = 23,
    CPP_TOKEN_KEY_MODIFIER = 24,
    CPP_TOKEN_KEY_QUALIFIER = 25,
    /* DOC(This type is not stored in token output from the lexer.) */
    CPP_TOKEN_KEY_OPERATOR = 26,
    CPP_TOKEN_KEY_CONTROL_FLOW = 27,
    CPP_TOKEN_KEY_CAST = 28,
    CPP_TOKEN_KEY_TYPE_DECLARATION = 29,
    CPP_TOKEN_KEY_ACCESS = 30,
    CPP_TOKEN_KEY_LINKAGE = 31,
    CPP_TOKEN_KEY_OTHER = 32,
    
    CPP_TOKEN_IDENTIFIER = 33,
    CPP_TOKEN_INTEGER_CONSTANT = 34,
    CPP_TOKEN_CHARACTER_CONSTANT = 35,
    CPP_TOKEN_FLOATING_CONSTANT = 36,
    CPP_TOKEN_STRING_CONSTANT = 37,
    CPP_TOKEN_BOOLEAN_CONSTANT = 38,
    
    CPP_TOKEN_STATIC_ASSERT = 39,
    
    CPP_TOKEN_BRACKET_OPEN = 40,
    CPP_TOKEN_BRACKET_CLOSE = 41,
    CPP_TOKEN_PARENTHESE_OPEN = 42,
    CPP_TOKEN_PARENTHESE_CLOSE = 43,
    CPP_TOKEN_BRACE_OPEN = 44,
    CPP_TOKEN_BRACE_CLOSE = 45,
    CPP_TOKEN_SEMICOLON = 46,
    CPP_TOKEN_ELLIPSIS = 47,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_STAR = 48,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_AMPERSAND = 49,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_TILDE = 50,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_PLUS = 51,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_MINUS = 52,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_INCREMENT = 53,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_DECREMENT = 54,
    
    // NOTE(allen): Precedence 1, LtoR
    CPP_TOKEN_SCOPE = 55,
    
    // NOTE(allen): Precedence 2, LtoR
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTINC = 56,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTDEC = 57,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_FUNC_STYLE_CAST = 58,
    CPP_TOKEN_CPP_STYLE_CAST = 59,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CALL = 60,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_INDEX = 61,
    CPP_TOKEN_DOT = 62,
    CPP_TOKEN_ARROW = 63,
    
    // NOTE(allen): Precedence 3, RtoL
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREINC = 64,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREDEC = 65,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSITIVE = 66,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEGAITVE = 67,
    CPP_TOKEN_NOT = 68,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_BIT_NOT = 69,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CAST = 70,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DEREF = 71,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_PTR = 72,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADDRESS = 73,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_REF = 74,
    CPP_TOKEN_SIZEOF = 75,
    CPP_TOKEN_ALIGNOF = 76,
    CPP_TOKEN_DECLTYPE = 77,
    CPP_TOKEN_TYPEID = 78,
    CPP_TOKEN_NEW = 79,
    CPP_TOKEN_DELETE = 80,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEW_ARRAY = 81,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DELETE_ARRAY = 82,
    
    // NOTE(allen): Precedence 4, LtoR
    CPP_TOKEN_PTRDOT = 83,
    CPP_TOKEN_PTRARROW = 84,
    
    // NOTE(allen): Precedence 5, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_MUL = 85,
    CPP_TOKEN_DIV = 86,
    CPP_TOKEN_MOD = 87,
    
    // NOTE(allen): Precedence 6, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADD = 88,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_SUB = 89,
    
    // NOTE(allen): Precedence 7, LtoR
    CPP_TOKEN_LSHIFT = 90,
    CPP_TOKEN_RSHIFT = 91,
    
    // NOTE(allen): Precedence 8, LtoR
    CPP_TOKEN_LESS = 92,
    CPP_TOKEN_GRTR = 93,
    CPP_TOKEN_GRTREQ = 94,
    CPP_TOKEN_LESSEQ = 95,
    
    // NOTE(allen): Precedence 9, LtoR
    CPP_TOKEN_EQEQ = 96,
    CPP_TOKEN_NOTEQ = 97,
    
    // NOTE(allen): Precedence 10, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_BIT_AND = 98,
    
    // NOTE(allen): Precedence 11, LtoR
    CPP_TOKEN_BIT_XOR = 99,
    
    // NOTE(allen): Precedence 12, LtoR
    CPP_TOKEN_BIT_OR = 100,
    
    // NOTE(allen): Precedence 13, LtoR
    CPP_TOKEN_AND = 101,
    
    // NOTE(allen): Precedence 14, LtoR
    CPP_TOKEN_OR = 102,
    
    // NOTE(allen): Precedence 15, RtoL
    CPP_TOKEN_TERNARY_QMARK = 103,
    CPP_TOKEN_COLON = 104,
    CPP_TOKEN_THROW = 105,
    CPP_TOKEN_EQ = 106,
    CPP_TOKEN_ADDEQ = 107,
    CPP_TOKEN_SUBEQ = 108,
    CPP_TOKEN_MULEQ = 109,
    CPP_TOKEN_DIVEQ = 110,
    CPP_TOKEN_MODEQ = 111,
    CPP_TOKEN_LSHIFTEQ = 112,
    CPP_TOKEN_RSHIFTEQ = 113,
    CPP_TOKEN_ANDEQ = 114,
    CPP_TOKEN_OREQ = 115,
    CPP_TOKEN_XOREQ = 116,
    
    // NOTE(allen): Precedence 16, LtoR
    CPP_TOKEN_COMMA = 117,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_EOF = 118,
    
    CPP_TOKEN_TYPE_COUNT = 119
};

/* DOC(Cpp_Token represents a single lexed token. It is the primary output of the lexing system.)
DOC_SEE(Cpp_Token_Flag) */
STRUCT Cpp_Token{
    /* DOC(The type field indicates the type of the token.  All tokens have a type no matter the circumstances.) */
    Cpp_Token_Type type;
    
    /* DOC(The start field indicates the index of the first character of this token's lexeme.) */
    int32_t start;
    
    /* DOC(The size field indicates the number of bytes in this token's lexeme.) */
    int32_t size;
    
    /* DOC(The state_flags should not be used outside of the lexer's implementation.) */
    uint16_t state_flags;
    
    /* DOC(The flags field contains extra useful information about the token.) */
    uint16_t flags;
};

/* DOC(The Cpp_Token_Flags are used to mark up tokens with additional information.) */
ENUM(uint16_t, Cpp_Token_Flag){
    /* DOC(Indicates that the token is a preprocessor directive.) */
    CPP_TFLAG_PP_DIRECTIVE = 0x1,
    
    /* DOC(Indicates that the token is on the line of a preprocessor directive.) */
    CPP_TFLAG_PP_BODY = 0x2,
    
    /* DOC(Indicates that the token spans across multiple lines.  This can show up on line comments and string literals with back slash line continuation. ) */
    CPP_TFLAG_MULTILINE = 0x4,
    
    /* DOC(Indicates that the token is some kind of operator or punctuation like braces.) */
    CPP_TFLAG_IS_OPERATOR = 0x8,
    
    /* DOC(Indicates that the token is a keyword.) */
    CPP_TFLAG_IS_KEYWORD = 0x10
};

/* DOC(Cpp_Token_Array is used to bundle together the common elements of a growing array of Cpp_Tokens.  To initialize it the tokens field should point to a block of memory with a size equal to max_count*sizeof(Cpp_Token) and the count should be initialized to zero.) */
STRUCT Cpp_Token_Array{
    /* DOC(The tokens field points to the memory used to store the array of tokens.) */
    Cpp_Token *tokens;
    
    /* DOC(The count field counts how many tokens in the array are currently used.) */
    int32_t count;
    
    /* DOC(The max_count field specifies the maximum size the count field may grow to before the tokens array is out of space.) */
    int32_t max_count;
};

static Cpp_Token_Array null_cpp_token_array = {0};

/* DOC(Cpp_Get_Token_Result is the return result of the cpp_get_token call.)
DOC_SEE(cpp_get_token) */
STRUCT Cpp_Get_Token_Result{
    /* DOC(The token_index field indicates which token answers the query.  To get the token from the source array CODE_EXAMPLE(array.tokens[result.token_index])) */
    int32_t token_index;
    
    /* DOC(The in_whitespace field is true when the query position was actually in whitespace after the result token.) */
    int32_t in_whitespace;
    
    /* DOC(If the token_index refers to an actual token, this is the start value of the token. Otherwise this is zero.) */
    int32_t token_start;
    
    /* DOC(If the token_index refers to an actual token, this is the start+size value of the token. Otherwise this is zero.) */
    int32_t token_end;
};

/* DOC(Cpp_Relex_Range is the return result of the cpp_get_relex_range call.)
DOC_SEE(cpp_get_relex_range) */
STRUCT Cpp_Relex_Range{
    /* DOC(The index of the first token in the unedited array that needs to be relexed.) */
    int32_t start_token_index;
    /* DOC(The index of the first token in the unedited array after the edited range that may not need to be relexed.  Sometimes a relex operation has to lex past this position to find a token that is not effected by the edit.) */
    int32_t end_token_index;
};

struct Cpp_Lex_FSM{
    uint8_t state;
    uint8_t emit_token;
    uint8_t flags;
};
static Cpp_Lex_FSM null_lex_fsm = {0};

/* DOC(Cpp_Lex_Data represents the state of the lexer so that the system may be resumable and the user can manage the lexer state and decide when to resume lexing with it.  To create a new lexer state call cpp_lex_data_init.

The internals of the lex state should not be treated as a part of the public API.)
DOC_SEE(cpp_lex_data_init)
HIDE_MEMBERS() */
STRUCT Cpp_Lex_Data{
    char tb[32];
    i32_4tech tb_pos;
    i32_4tech token_start;
    
    i32_4tech pos;
    i32_4tech pos_overide;
    i32_4tech chunk_pos;
    
    Cpp_Lex_FSM fsm;
    u8_4tech white_done;
    u8_4tech pp_state;
    u8_4tech completed;
    
    Cpp_Token token;
    
    char raw_delim[16];
    i32_4tech delim_length;
    
    b8_4tech str_raw;
    b8_4tech str_include;
    
    i32_4tech ignore_string_delims;
    
    i32_4tech __pc__;
};

/* DOC(Cpp_Lex_Result is returned from the lexing engine to indicate why it stopped lexing.) */
ENUM(int32_t, Cpp_Lex_Result){
    /* DOC(This indicates that the system got to the end of the file and will not accept more input.) */
    LexResult_Finished = 0,
    
    /* DOC(This indicates that the system got to the end of an input chunk and is ready to receive the next input chunk.) */
    LexResult_NeedChunk = 1,
    
    /* DOC(This indicates that the output array ran out of space to store tokens and needs to be replaced or expanded before continuing.) */
    LexResult_NeedTokenMemory = 2,
    
    /* DOC(This indicates that the maximum number of output tokens as specified by the user was hit.) */
    LexResult_HitTokenLimit = 3,
};

/* DOC(Cpp_Relex_Data represents the state of the relexer so that the system may be resumable. To create a new relex state call cpp_relex_init.)
DOC_SEE(cpp_relex_init)
HIDE_MEMBERS()*/
STRUCT Cpp_Relex_Data{
    Cpp_Lex_Data lex;
    
    Cpp_Token end_token;
    
    int32_t relex_start_position;
    int32_t start_token_index;
    int32_t end_token_index;
    int32_t original_end_token_index;
    
    int32_t character_shift_amount;
    
    Cpp_Lex_Result result_state;
    
    int32_t __pc__;
};

ENUM_INTERNAL(uint16_t, Cpp_Preprocessor_State){
    CPP_LEX_PP_DEFAULT = 0,
    CPP_LEX_PP_IDENTIFIER = 1,
    CPP_LEX_PP_MACRO_IDENTIFIER = 2,
    CPP_LEX_PP_INCLUDE = 3,
    CPP_LEX_PP_BODY = 4,
    CPP_LEX_PP_BODY_IF = 5,
    CPP_LEX_PP_NUMBER = 6,
    CPP_LEX_PP_ERROR = 7,
    CPP_LEX_PP_JUNK = 8,
    CPP_LEX_PP_COUNT = 9
};

ENUM_INTERNAL(uint8_t, Cpp_Lex_State){
    LS_default = 0,
    LS_identifier = 1,
    LS_pound = 2,
    LS_pp = 3,
    LS_ppdef = 4,
    LS_string_R = 5,
    LS_string_LUu8 = 6,
    LS_string_u = 7,
    LS_number = 8,
    LS_number0 = 9,
    LS_float = 10,
    LS_crazy_float0 = 11,
    LS_crazy_float1 = 12,
    LS_hex = 13,
    LS_comment_pre = 14,
    LS_comment = 15,
    LS_comment_slashed = 16,
    LS_comment_block = 17,
    LS_comment_block_ending = 18,
    LS_dot = 19,
    LS_ellipsis = 20,
    LS_less = 21,
    LS_more = 22,
    LS_minus = 23,
    LS_arrow = 24,
    LS_and = 25,
    LS_or = 26,
    LS_plus = 27,
    LS_colon = 28,
    LS_single_op = 29,
    LS_error_message = 30,
    //
    LS_count = 31,
    LS_char = 32,
};

// NOTE(allen): These provide names that match the overloaded meanings of string states.
#define LS_string_raw LS_string_R
#define LS_string_normal LS_string_LUu8
#define LS_string_include LS_string_u

ENUM_INTERNAL(uint8_t, Cpp_Lex_Int_State){
    LSINT_default,
    LSINT_u,
    LSINT_l,
    LSINT_L,
    LSINT_ul,
    LSINT_uL,
    LSINT_ll,
    LSINT_extra,
    //
    LSINT_count
};

ENUM_INTERNAL(uint8_t, Cpp_Lex_Str_State){
    LSSTR_default,
    LSSTR_escape,
    LSSTR_multiline,
    //
    LSSTR_count
};

#define LSSTR_include_count 1
#define LSSTR_error LSSTR_escape
#define LSSTR_get_delim LSSTR_escape
#define LSSTR_check_delim LSSTR_count

ENUM_INTERNAL(uint8_t, Cpp_Lex_PP_State){
    LSPP_default,
    LSPP_include,
    LSPP_macro_identifier,
    LSPP_identifier,
    LSPP_body_if,
    LSPP_body,
    LSPP_number,
    LSPP_error,
    LSPP_junk,
    LSPP_no_strings,
    //
    LSPP_count
};

#endif

// BOTTOM

