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

/* DOC(A Cpp_Word_Table_Type names one of the keyword table slots in a parse context.) */
ENUM(uint32_t, Cpp_Word_Table_Type){
    /* DOC(The Cpp_Keyword_Table used to list the typical keywords.) */
    CPP_TABLE_KEYWORDS,
    /* DOC(The Cpp_Keyword_Table used to list preprocessor directives.) */
    CPP_TABLE_PREPROCESSOR_DIRECTIVES,
};

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
    
    CPP_TOKEN_VOID = 23,
    CPP_TOKEN_BOOL = 24,
    CPP_TOKEN_CHAR = 25,
    CPP_TOKEN_INT = 26,
    CPP_TOKEN_FLOAT = 27,
    CPP_TOKEN_DOUBLE = 28,
    CPP_TOKEN_LONG = 29,
    CPP_TOKEN_SHORT = 30,
    CPP_TOKEN_UNSIGNED = 31,
    CPP_TOKEN_SIGNED = 32,
    
    CPP_TOKEN_CONST = 33,
    CPP_TOKEN_VOLATILE = 34,
    
    CPP_TOKEN_ASM = 35,
    CPP_TOKEN_BREAK = 36,
    CPP_TOKEN_CASE = 37,
    CPP_TOKEN_CATCH = 38,
    CPP_TOKEN_CONTINUE = 39,
    CPP_TOKEN_DEFAULT = 40,
    CPP_TOKEN_DO = 41,
    CPP_TOKEN_ELSE = 42,
    CPP_TOKEN_FOR = 43,
    CPP_TOKEN_GOTO = 44,
    CPP_TOKEN_IF = 45,
    CPP_TOKEN_RETURN = 46,
    CPP_TOKEN_SWITCH = 47,
    CPP_TOKEN_TRY = 48,
    CPP_TOKEN_WHILE = 49,
    CPP_TOKEN_STATIC_ASSERT = 50,
    
    CPP_TOKEN_CONST_CAST = 51,
    CPP_TOKEN_DYNAMIC_CAST = 52,
    CPP_TOKEN_REINTERPRET_CAST = 53,
    CPP_TOKEN_STATIC_CAST = 54,
    
    CPP_TOKEN_CLASS = 55,
    CPP_TOKEN_ENUM = 56,
    CPP_TOKEN_STRUCT = 57,
    CPP_TOKEN_TYPEDEF = 58,
    CPP_TOKEN_UNION = 59,
    CPP_TOKEN_TEMPLATE = 60,
    CPP_TOKEN_TYPENAME = 61,
    
    CPP_TOKEN_FRIEND = 62,
    CPP_TOKEN_NAMESPACE = 63,
    CPP_TOKEN_PRIVATE = 64,
    CPP_TOKEN_PROTECTED = 65,
    CPP_TOKEN_PUBLIC = 66,
    CPP_TOKEN_USING = 67,
    
    CPP_TOKEN_EXTERN = 68,
    CPP_TOKEN_EXPORT = 69,
    CPP_TOKEN_INLINE = 70,
    CPP_TOKEN_STATIC = 71,
    CPP_TOKEN_VIRTUAL = 72,
    
    CPP_TOKEN_ALIGNAS = 73,
    CPP_TOKEN_EXPLICIT = 74,
    CPP_TOKEN_NOEXCEPT = 75,
    CPP_TOKEN_NULLPTR = 76,
    CPP_TOKEN_OPERATOR = 77,
    CPP_TOKEN_REGISTER = 78,
    CPP_TOKEN_THIS = 79,
    CPP_TOKEN_THREAD_LOCAL = 80,
    
    CPP_TOKEN_KEY_OTHER = 81,
    
    CPP_TOKEN_IDENTIFIER = 82,
    CPP_TOKEN_INTEGER_CONSTANT = 83,
    CPP_TOKEN_CHARACTER_CONSTANT = 84,
    CPP_TOKEN_FLOATING_CONSTANT = 85,
    CPP_TOKEN_STRING_CONSTANT = 86,
    CPP_TOKEN_TRUE = 87,
    CPP_TOKEN_FALSE = 88,
    
    CPP_TOKEN_BRACKET_OPEN = 89,
    CPP_TOKEN_BRACKET_CLOSE = 90,
    CPP_TOKEN_PARENTHESE_OPEN = 91,
    CPP_TOKEN_PARENTHESE_CLOSE = 92,
    CPP_TOKEN_BRACE_OPEN = 93,
    CPP_TOKEN_BRACE_CLOSE = 94,
    CPP_TOKEN_SEMICOLON = 95,
    CPP_TOKEN_ELLIPSIS = 96,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_STAR = 97,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_AMPERSAND = 98,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_TILDE = 99,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_PLUS = 100,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_MINUS = 101,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_INCREMENT = 102,
    
    /* DOC(This is an 'ambiguous' token type because it requires parsing to determine the full nature of the token.) */
    CPP_TOKEN_DECREMENT = 103,
    
    // NOTE(allen): Precedence 1, LtoR
    CPP_TOKEN_SCOPE = 104,
    
    // NOTE(allen): Precedence 2, LtoR
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTINC = 105,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTDEC = 106,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_FUNC_STYLE_CAST = 107,
    CPP_TOKEN_CPP_STYLE_CAST = 108,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CALL = 109,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_INDEX = 110,
    CPP_TOKEN_DOT = 111,
    CPP_TOKEN_ARROW = 112,
    
    // NOTE(allen): Precedence 3, RtoL
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREINC = 113,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREDEC = 114,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSITIVE = 115,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEGAITVE = 116,
    CPP_TOKEN_NOT = 117,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_BIT_NOT = 118,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CAST = 119,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DEREF = 120,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_PTR = 121,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADDRESS = 122,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_REF = 123,
    CPP_TOKEN_SIZEOF = 124,
    CPP_TOKEN_ALIGNOF = 125,
    CPP_TOKEN_DECLTYPE = 126,
    CPP_TOKEN_TYPEID = 127,
    CPP_TOKEN_NEW = 128,
    CPP_TOKEN_DELETE = 129,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEW_ARRAY = 130,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DELETE_ARRAY = 131,
    
    // NOTE(allen): Precedence 4, LtoR
    CPP_TOKEN_PTRDOT = 132,
    CPP_TOKEN_PTRARROW = 133,
    
    // NOTE(allen): Precedence 5, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_MUL = 134,
    CPP_TOKEN_DIV = 135,
    CPP_TOKEN_MOD = 136,
    
    // NOTE(allen): Precedence 6, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADD = 137,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_SUB = 138,
    
    // NOTE(allen): Precedence 7, LtoR
    CPP_TOKEN_LSHIFT = 139,
    CPP_TOKEN_RSHIFT = 140,
    
    // NOTE(allen): Precedence 8, LtoR
    CPP_TOKEN_LESS = 141,
    CPP_TOKEN_GRTR = 142,
    CPP_TOKEN_GRTREQ = 143,
    CPP_TOKEN_LESSEQ = 144,
    
    // NOTE(allen): Precedence 9, LtoR
    CPP_TOKEN_EQEQ = 145,
    CPP_TOKEN_NOTEQ = 146,
    
    // NOTE(allen): Precedence 10, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_BIT_AND = 147,
    
    // NOTE(allen): Precedence 11, LtoR
    CPP_TOKEN_BIT_XOR = 148,
    
    // NOTE(allen): Precedence 12, LtoR
    CPP_TOKEN_BIT_OR = 149,
    
    // NOTE(allen): Precedence 13, LtoR
    CPP_TOKEN_AND = 150,
    
    // NOTE(allen): Precedence 14, LtoR
    CPP_TOKEN_OR = 151,
    
    // NOTE(allen): Precedence 15, RtoL
    CPP_TOKEN_TERNARY_QMARK = 152,
    CPP_TOKEN_COLON = 153,
    CPP_TOKEN_THROW = 154,
    CPP_TOKEN_EQ = 155,
    CPP_TOKEN_ADDEQ = 156,
    CPP_TOKEN_SUBEQ = 157,
    CPP_TOKEN_MULEQ = 158,
    CPP_TOKEN_DIVEQ = 159,
    CPP_TOKEN_MODEQ = 160,
    CPP_TOKEN_LSHIFTEQ = 161,
    CPP_TOKEN_RSHIFTEQ = 162,
    CPP_TOKEN_ANDEQ = 163,
    CPP_TOKEN_OREQ = 164,
    CPP_TOKEN_XOREQ = 165,
    
    // NOTE(allen): Precedence 16, LtoR
    CPP_TOKEN_COMMA = 166,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_EOF = 167,
    
    CPP_TOKEN_TYPE_COUNT = 168,
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

/* DOC(The Cpp_Token_Category enum groups certain Cpp_Token_Type values into greater categories)
DOC_SEE(cpp_token_category_from_type) */
ENUM(uint32_t, Cpp_Token_Category){
    CPP_TOKEN_CAT_NONE = 0,
    CPP_TOKEN_CAT_BOOLEAN_CONSTANT = 1,
    CPP_TOKEN_CAT_TYPE = 2,
    CPP_TOKEN_CAT_MODIFIER = 3,
    CPP_TOKEN_CAT_QUALIFIER = 4,
    CPP_TOKEN_CAT_OPERATOR = 5,
    CPP_TOKEN_CAT_CONTROL_FLOW = 6,
    CPP_TOKEN_CAT_CAST = 7,
    CPP_TOKEN_CAT_TYPE_DECLARATION = 8,
    CPP_TOKEN_CAT_ACCESS = 9,
    CPP_TOKEN_CAT_LINKAGE = 10,
    CPP_TOKEN_CAT_OTHER = 11,
    CPP_TOKEN_CAT_EOF = 12,
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
    CPP_TFLAG_IS_KEYWORD = 0x10,
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

static Cpp_Token_Array null_cpp_token_array = {};

/* DOC(Cpp_Get_Token_Result is the return result of the cpp_get_token call.)
DOC_SEE(cpp_get_token) */
STRUCT Cpp_Get_Token_Result{
    /* DOC(The token_index field indicates which token answers the query.  To get the token from the source array CODE_EXAMPLE(array.tokens[result.token_index])) */
    int32_t token_index;
    
    /* DOC(The in_whitespace field is true when the query position was actually in whitespace after the result token.) */
    int32_t in_whitespace_after_token;
    
    /* DOC(If the token_index refers to an actual token, this is the start value of the token. Otherwise this is zero.) */
    int32_t token_start;
    
    /* DOC(If the token_index refers to an actual token, this is the start + size value of the token. Otherwise this is zero.) */
    int32_t token_one_past_last;
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
static Cpp_Lex_FSM null_lex_fsm = {};

/* DOC(A Cpp_Keyword_Table contains a list of keywords and a hashed lookup table for the keywords.  They are used to setup a parse context.)
DOC_SEE(cpp_make_token_array)
HIDE_MEMBERS() */
STRUCT Cpp_Keyword_Table{
    void *mem;
    umem memsize;
    u64 *keywords;
    u32 max;
};

/* DOC(Cpp_Lex_Data represents the state of the lexer so that the system may be resumable and the user can manage the lexer state and decide when to resume lexing with it.  To create a new lexer state call cpp_lex_data_init.

The internals of the lex state should not be treated as a part of the public API.)
DOC_SEE(cpp_lex_data_init)
HIDE_MEMBERS() */
STRUCT Cpp_Lex_Data{
    char tb[32];
    i32 tb_pos;
    i32 token_start;
    
    i32 pos;
    i32 pos_overide;
    i32 chunk_pos;
    
    Cpp_Lex_FSM fsm;
    u8 white_done;
    u8 pp_state;
    u8 completed;
    
    Cpp_Token token;
    
    char raw_delim[16];
    i32 delim_length;
    
    b8 str_raw;
    b8 str_include;
    b8 ignore_string_delims;
    
    Cpp_Keyword_Table keyword_table;
    Cpp_Keyword_Table preprops_table;
    
    i32 __pc__;
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
    LSPP_default = 0,
    LSPP_include = 1,
    LSPP_macro_identifier = 2,
    LSPP_identifier = 3,
    LSPP_body_if = 4,
    LSPP_body = 5,
    LSPP_number = 6,
    LSPP_error = 7,
    LSPP_junk = 8,
    LSPP_no_strings = 9,
    //
    LSPP_count = 10
};

#endif

// BOTTOM

