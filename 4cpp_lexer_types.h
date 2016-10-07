
// TOP

#ifndef FCPP_LEXER_TYPES_INC
#define FCPP_LEXER_TYPES_INC

#ifndef ENUM
#define ENUM(type,name) typedef type name; enum name##_
#endif

#ifndef ENUM_INTERNAL
#define ENUM_INTERNAL(type,name) typedef type name; enum name##_
#endif

#ifndef struct_internal
#define struct_internal struct
#endif

/* DOC(A Cpp_Token_Type classifies a token to make parsing easier. Some types are not
actually output by the lexer, but exist because parsers will also make use of token
types in their own output.) */
ENUM(uint32_t, Cpp_Token_Type){
    
	CPP_TOKEN_JUNK = 0,
    CPP_TOKEN_COMMENT = 1,
    
	CPP_PP_INCLUDE = 2,
	CPP_PP_DEFINE = 3,
	CPP_PP_UNDEF = 4,
	CPP_PP_IF = 5,
	CPP_PP_IFDEF = 6,
	CPP_PP_IFNDEF = 7,
	CPP_PP_ELSE = 8,
	CPP_PP_ELIF = 9,
	CPP_PP_ENDIF = 10,
	CPP_PP_ERROR = 11,
	CPP_PP_IMPORT = 12,
	CPP_PP_USING = 13,
	CPP_PP_LINE = 14,
	CPP_PP_PRAGMA = 15,
	CPP_PP_STRINGIFY = 16,
	CPP_PP_CONCAT = 17,
	CPP_PP_UNKNOWN = 18,
    
    CPP_PP_DEFINED = 19,
    CPP_PP_INCLUDE_FILE = 20,
    CPP_PP_ERROR_MESSAGE = 21,
    
	CPP_TOKEN_KEY_TYPE = 22,
	CPP_TOKEN_KEY_MODIFIER = 23,
	CPP_TOKEN_KEY_QUALIFIER = 24,
    /* DOC(This type is not stored in token output from the lexer.) */
	CPP_TOKEN_KEY_OPERATOR = 25,
	CPP_TOKEN_KEY_CONTROL_FLOW = 26,
    CPP_TOKEN_KEY_CAST = 27,
	CPP_TOKEN_KEY_TYPE_DECLARATION = 28,
	CPP_TOKEN_KEY_ACCESS = 29,
	CPP_TOKEN_KEY_LINKAGE = 30,
	CPP_TOKEN_KEY_OTHER = 31,
    
	CPP_TOKEN_IDENTIFIER = 32,
	CPP_TOKEN_INTEGER_CONSTANT = 33,
	CPP_TOKEN_CHARACTER_CONSTANT = 34,
	CPP_TOKEN_FLOATING_CONSTANT = 35,
	CPP_TOKEN_STRING_CONSTANT = 36,
	CPP_TOKEN_BOOLEAN_CONSTANT = 37,
    
    CPP_TOKEN_STATIC_ASSERT = 38,
    
	CPP_TOKEN_BRACKET_OPEN = 39,
	CPP_TOKEN_BRACKET_CLOSE = 40,
	CPP_TOKEN_PARENTHESE_OPEN = 41,
	CPP_TOKEN_PARENTHESE_CLOSE = 42,
	CPP_TOKEN_BRACE_OPEN = 43,
	CPP_TOKEN_BRACE_CLOSE = 44,
    CPP_TOKEN_SEMICOLON = 45,
    CPP_TOKEN_ELLIPSIS = 46,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_STAR = 47,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_AMPERSAND = 48,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_TILDE = 49,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_PLUS = 50,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_MINUS = 51,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_INCREMENT = 52,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_DECREMENT = 53,
    
    // NOTE(allen): Precedence 1, LtoR
	CPP_TOKEN_SCOPE = 54,
    
    // NOTE(allen): Precedence 2, LtoR
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTINC = 55,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTDEC = 56,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_FUNC_STYLE_CAST = 57,
    CPP_TOKEN_CPP_STYLE_CAST = 58,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CALL = 59,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_INDEX = 60,
	CPP_TOKEN_DOT = 61,
	CPP_TOKEN_ARROW = 62,
    
    // NOTE(allen): Precedence 3, RtoL
    
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREINC = 63,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREDEC = 64,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSITIVE = 65,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEGAITVE = 66,
	CPP_TOKEN_NOT = 67,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_BIT_NOT = 68,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CAST = 69,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DEREF = 70,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_PTR = 71,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADDRESS = 72,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_REF = 73,
    CPP_TOKEN_SIZEOF = 74,
    CPP_TOKEN_ALIGNOF = 75,
    CPP_TOKEN_DECLTYPE = 76,
    CPP_TOKEN_TYPEID = 77,
    CPP_TOKEN_NEW = 78,
    CPP_TOKEN_DELETE = 79,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEW_ARRAY = 80,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DELETE_ARRAY = 81,
    
    // NOTE(allen): Precedence 4, LtoR
	CPP_TOKEN_PTRDOT = 82,
	CPP_TOKEN_PTRARROW = 83,
    
    // NOTE(allen): Precedence 5, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
	CPP_TOKEN_MUL = 84,
	CPP_TOKEN_DIV = 85,
	CPP_TOKEN_MOD = 86,
    
    // NOTE(allen): Precedence 6, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADD = 87,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_SUB = 88,
    
    // NOTE(allen): Precedence 7, LtoR
	CPP_TOKEN_LSHIFT = 89,
	CPP_TOKEN_RSHIFT = 90,
    
    // NOTE(allen): Precedence 8, LtoR
	CPP_TOKEN_LESS = 91,
	CPP_TOKEN_GRTR = 92,
	CPP_TOKEN_GRTREQ = 93,
	CPP_TOKEN_LESSEQ = 94,
    
    // NOTE(allen): Precedence 9, LtoR
    CPP_TOKEN_EQEQ = 95,
    CPP_TOKEN_NOTEQ = 96,
    
    // NOTE(allen): Precedence 10, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
	CPP_TOKEN_BIT_AND = 97,
    
    // NOTE(allen): Precedence 11, LtoR
	CPP_TOKEN_BIT_XOR = 98,
    
    // NOTE(allen): Precedence 12, LtoR
	CPP_TOKEN_BIT_OR = 99,
    
    // NOTE(allen): Precedence 13, LtoR
	CPP_TOKEN_AND = 100,
    
    // NOTE(allen): Precedence 14, LtoR
	CPP_TOKEN_OR = 101,
    
    // NOTE(allen): Precedence 15, RtoL
    CPP_TOKEN_TERNARY_QMARK = 102,
	CPP_TOKEN_COLON = 103,
    CPP_TOKEN_THROW = 104,
	CPP_TOKEN_EQ = 105,
	CPP_TOKEN_ADDEQ = 106,
	CPP_TOKEN_SUBEQ = 107,
	CPP_TOKEN_MULEQ = 108,
	CPP_TOKEN_DIVEQ = 109,
	CPP_TOKEN_MODEQ = 110,
	CPP_TOKEN_LSHIFTEQ = 111,
	CPP_TOKEN_RSHIFTEQ = 112,
	CPP_TOKEN_ANDEQ = 113,
	CPP_TOKEN_OREQ = 114,
	CPP_TOKEN_XOREQ = 115,
    
    // NOTE(allen): Precedence 16, LtoR
	CPP_TOKEN_COMMA = 116,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_EOF = 117,
    
    CPP_TOKEN_TYPE_COUNT = 118
};

/* DOC(Cpp_Token represents a single lexed token.
It is the primary output of the lexing system.)
DOC_SEE(Cpp_Token_Flag) */
struct Cpp_Token{
    /* DOC(The type field indicates the type of the token. 
    All tokens have a type no matter the circumstances.) */
	Cpp_Token_Type type;
    
    /* DOC(The start field indicates the index of the first character
    of this token's lexeme.) */
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
    
    /* DOC(Indicates that the token spans across multiple lines.  This can show up
    on line comments and string literals with back slash line continuation. ) */
	CPP_TFLAG_MULTILINE = 0x4,
    
    /* DOC(Indicates that the token is some kind of operator or punctuation like braces.) */
    CPP_TFLAG_IS_OPERATOR = 0x8,
    
    /* DOC(Indicates that the token is a keyword.) */
    CPP_TFLAG_IS_KEYWORD = 0x10
};

/* DOC(Cpp_Token_Array is used to bundle together the common elements
of a growing array of Cpp_Tokens.  To initialize it the tokens field should
point to a block of memory with a size equal to max_count*sizeof(Cpp_Token)
and the count should be initialized to zero.) */
struct Cpp_Token_Array{
    /* DOC(The tokens field points to the memory used to store the array of tokens.) */
	Cpp_Token *tokens;
    
    /* DOC(The count field counts how many tokens in the array are currently used.) */
    int32_t count;
    
    /* DOC(The max_count field specifies the maximum size the count field may grow to before
    the tokens array is out of space.) */
    int32_t max_count;
};

static Cpp_Token_Array null_cpp_token_array = {0};

/* DOC(Cpp_Get_Token_Result is the return result of the cpp_get_token call.)
DOC_SEE(cpp_get_token) */
struct Cpp_Get_Token_Result{
    /* DOC(The token_index field indicates which token answers the query.  To get the token from
    the source array CODE_EXAMPLE(array.tokens[result.token_index])) */
	int32_t token_index;
    
    /* DOC(The in_whitespace field is true when the query position was actually in whitespace
    after the result token.) */
	int32_t in_whitespace;
    
    /* DOC(If the token_index refers to an actual token, this is the start value of the token.
    Otherwise this is zero.) */
    int32_t token_start;
    
    /* DOC(If the token_index refers to an actual token, this is the start+size value of the token.
    Otherwise this is zero.) */
    int32_t token_end;
};

/* DOC(Cpp_Relex_Range is the return result of the cpp_get_relex_range call.)
DOC_SEE(cpp_get_relex_range) */
struct Cpp_Relex_Range{
    /* DOC(The index of the first token in the unedited array that needs to be relexed.) */
    int32_t start_token_index;
    /* DOC(The index of the first token in the unedited array after the edited range
    that may not need to be relexed.  Sometimes a relex operation has to lex past this
    position to find a token that is not effected by the edit.) */
    int32_t end_token_index;
};

struct_internal Cpp_Lex_FSM{
    uint8_t state;
    uint8_t int_state;
    uint8_t emit_token;
    uint8_t multi_line;
};
static Cpp_Lex_FSM null_lex_fsm = {0};

/* DOC(Cpp_Lex_Data represents the state of the lexer so that the system may be resumable
and the user can manage the lexer state and decide when to resume lexing with it.  To create
a new lexer state call cpp_lex_data_init.

The internals of the lex state should not be treated as a part of the public API.)
DOC_SEE(cpp_lex_data_init)
HIDE_MEMBERS()*/
struct Cpp_Lex_Data{
    char tb[32];
    int32_t tb_pos;
    int32_t token_start;
    
    int32_t pos;
    int32_t pos_overide;
    int32_t chunk_pos;
    
    Cpp_Lex_FSM fsm;
    uint8_t white_done;
    uint8_t pp_state;
    uint8_t completed;
    
    Cpp_Token token;
    
    int32_t __pc__;
};

/* DOC(Cpp_Lex_Result is returned from the lexing engine to indicate why it stopped lexing.) */
ENUM(int32_t, Cpp_Lex_Result){
    /* DOC(This indicates that the system got to the end of the file and will not accept more input.) */
    LexResult_Finished = 0,
    
    /* DOC(This indicates that the system got to the end of an input chunk and is ready to receive the
    next input chunk.) */
    LexResult_NeedChunk = 1,
    
    /* DOC(This indicates that the output array ran out of space to store tokens and needs to be
    replaced or expanded before continuing.) */
    LexResult_NeedTokenMemory = 2,
    
    /* DOC(This indicates that the maximum number of output tokens as specified by the user was hit.) */
    LexResult_HitTokenLimit = 3,
};

/* DOC(Cpp_Relex_Data represents the state of the relexer so that the system may be resumable.
To create a new relex state call cpp_relex_init.)
DOC_SEE(cpp_relex_init)
HIDE_MEMBERS()*/
struct Cpp_Relex_Data{
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
    CPP_LEX_PP_DEFAULT,
    CPP_LEX_PP_IDENTIFIER,
    CPP_LEX_PP_MACRO_IDENTIFIER,
    CPP_LEX_PP_INCLUDE,
    CPP_LEX_PP_BODY,
    CPP_LEX_PP_BODY_IF,
    CPP_LEX_PP_NUMBER,
    CPP_LEX_PP_ERROR,
    CPP_LEX_PP_JUNK,
    CPP_LEX_PP_COUNT
};

ENUM_INTERNAL(uint8_t, Cpp_Lex_State){
    LS_default,
    LS_identifier,
    LS_pound,
    LS_pp,
    LS_ppdef,
    LS_char,
    LS_char_multiline,
    LS_char_slashed,
    LS_string,
    LS_string_multiline,
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
    //
    LSPP_count
};

#endif

// BOTTOM

