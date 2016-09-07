
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
	CPP_TOKEN_JUNK,
	CPP_TOKEN_COMMENT,
    
	CPP_PP_INCLUDE,
	CPP_PP_DEFINE,
	CPP_PP_UNDEF,
	CPP_PP_IF,
	CPP_PP_IFDEF,
	CPP_PP_IFNDEF,
	CPP_PP_ELSE,
	CPP_PP_ELIF,
	CPP_PP_ENDIF,
	CPP_PP_ERROR,
	CPP_PP_IMPORT,
	CPP_PP_USING,
	CPP_PP_LINE,
	CPP_PP_PRAGMA,
	CPP_PP_STRINGIFY,
	CPP_PP_CONCAT,
	CPP_PP_UNKNOWN,
    
    CPP_PP_DEFINED,
    CPP_PP_INCLUDE_FILE,
    CPP_PP_ERROR_MESSAGE,
    
	CPP_TOKEN_KEY_TYPE,
	CPP_TOKEN_KEY_MODIFIER,
	CPP_TOKEN_KEY_QUALIFIER,
    /* DOC(This type is not stored in token output from the lexer.) */
	CPP_TOKEN_KEY_OPERATOR,
	CPP_TOKEN_KEY_CONTROL_FLOW,
    CPP_TOKEN_KEY_CAST,
	CPP_TOKEN_KEY_TYPE_DECLARATION,
	CPP_TOKEN_KEY_ACCESS,
	CPP_TOKEN_KEY_LINKAGE,
	CPP_TOKEN_KEY_OTHER,
    
	CPP_TOKEN_IDENTIFIER,
	CPP_TOKEN_INTEGER_CONSTANT,
	CPP_TOKEN_CHARACTER_CONSTANT,
	CPP_TOKEN_FLOATING_CONSTANT,
	CPP_TOKEN_STRING_CONSTANT,
	CPP_TOKEN_BOOLEAN_CONSTANT,
    
    CPP_TOKEN_STATIC_ASSERT,
    
	CPP_TOKEN_BRACKET_OPEN,
	CPP_TOKEN_BRACKET_CLOSE,
	CPP_TOKEN_PARENTHESE_OPEN,
	CPP_TOKEN_PARENTHESE_CLOSE,
	CPP_TOKEN_BRACE_OPEN,
	CPP_TOKEN_BRACE_CLOSE,
    CPP_TOKEN_SEMICOLON,
    CPP_TOKEN_ELLIPSIS,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_STAR,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_AMPERSAND,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_TILDE,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_PLUS,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_MINUS,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_INCREMENT,
    
    /* DOC(This is an 'ambiguous' token type because it requires
    parsing to determine the full nature of the token.) */
	CPP_TOKEN_DECREMENT,
    
    // NOTE(allen): Precedence 1, LtoR
	CPP_TOKEN_SCOPE,
    
    // NOTE(allen): Precedence 2, LtoR
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTINC,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSTDEC,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_FUNC_STYLE_CAST,
    CPP_TOKEN_CPP_STYLE_CAST,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CALL,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_INDEX,
	CPP_TOKEN_DOT,
	CPP_TOKEN_ARROW,
    
    // NOTE(allen): Precedence 3, RtoL
    
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREINC,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_PREDEC,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_POSITIVE,
    /* DOC(This token is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEGAITVE,
	CPP_TOKEN_NOT,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_BIT_NOT,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_CAST,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DEREF,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_PTR,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADDRESS,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_TYPE_REF,
    CPP_TOKEN_SIZEOF,
    CPP_TOKEN_ALIGNOF,
    CPP_TOKEN_DECLTYPE,
    CPP_TOKEN_TYPEID,
    CPP_TOKEN_NEW,
    CPP_TOKEN_DELETE,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_NEW_ARRAY,
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_DELETE_ARRAY,
    
    // NOTE(allen): Precedence 4, LtoR
	CPP_TOKEN_PTRDOT,
	CPP_TOKEN_PTRARROW,
    
    // NOTE(allen): Precedence 5, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
	CPP_TOKEN_MUL,
	CPP_TOKEN_DIV,
	CPP_TOKEN_MOD,
    
    // NOTE(allen): Precedence 6, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_ADD,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_SUB,
    
    // NOTE(allen): Precedence 7, LtoR
	CPP_TOKEN_LSHIFT,
	CPP_TOKEN_RSHIFT,
    
    // NOTE(allen): Precedence 8, LtoR
	CPP_TOKEN_LESS,
	CPP_TOKEN_GRTR,
	CPP_TOKEN_GRTREQ,
	CPP_TOKEN_LESSEQ,
    
    // NOTE(allen): Precedence 9, LtoR
    CPP_TOKEN_EQEQ,
    CPP_TOKEN_NOTEQ,
    
    // NOTE(allen): Precedence 10, LtoR
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
	CPP_TOKEN_BIT_AND,
    
    // NOTE(allen): Precedence 11, LtoR
	CPP_TOKEN_BIT_XOR,
    
    // NOTE(allen): Precedence 12, LtoR
	CPP_TOKEN_BIT_OR,
    
    // NOTE(allen): Precedence 13, LtoR
	CPP_TOKEN_AND,
    
    // NOTE(allen): Precedence 14, LtoR
	CPP_TOKEN_OR,
    
    // NOTE(allen): Precedence 15, RtoL
    CPP_TOKEN_TERNARY_QMARK,
	CPP_TOKEN_COLON,
    CPP_TOKEN_THROW,
	CPP_TOKEN_EQ,
	CPP_TOKEN_ADDEQ,
	CPP_TOKEN_SUBEQ,
	CPP_TOKEN_MULEQ,
	CPP_TOKEN_DIVEQ,
	CPP_TOKEN_MODEQ,
	CPP_TOKEN_LSHIFTEQ,
	CPP_TOKEN_RSHIFTEQ,
	CPP_TOKEN_ANDEQ,
	CPP_TOKEN_OREQ,
	CPP_TOKEN_XOREQ,
    
    // NOTE(allen): Precedence 16, LtoR
	CPP_TOKEN_COMMA,
    
    /* DOC(This type is for parser use, it is not output by the lexer.) */
    CPP_TOKEN_EOF,
    
    CPP_TOKEN_TYPE_COUNT
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
};

struct_internal Cpp_Relex_State{
    char *data;
    int32_t size;
    
    Cpp_Token_Array *array;
    int32_t start, end, amount;
    int32_t start_token_i;
    int32_t end_token_i;
    int32_t relex_start;
    int32_t tolerance;
    int32_t space_request;
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
a new lexer state that has not begun doing any lexing work call cpp_lex_data_init.

The internals of the lex state should not be treated as a part of the public API.)
DOC_SEE(cpp_lex_data_init)
HIDE_MEMBERS()*/
struct Cpp_Lex_Data{
    char *tb;
    
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
    LexResult_Finished,
    
    /* DOC(This indicates that the system got to the end of an input chunk and is ready to receive the
    next input chunk.) */
    LexResult_NeedChunk,
    
    /* DOC(This indicates that the output array ran out of space to store tokens and needs to be
    replaced or expanded before continuing.) */
    LexResult_NeedTokenMemory,
    
    /* DOC(This indicates that the maximum number of output tokens as specified by the user was hit.) */
    LexResult_HitTokenLimit,
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

