
// TOP

#ifndef FCPP_LEXER_TYPES_INC
#define FCPP_LEXER_TYPES_INC

enum Cpp_Token_Type{
	CPP_TOKEN_JUNK,
	CPP_TOKEN_COMMENT,
    
	CPP_TOKEN_KEY_TYPE,
	CPP_TOKEN_KEY_MODIFIER,
	CPP_TOKEN_KEY_QUALIFIER,
	CPP_TOKEN_KEY_OPERATOR, // NOTE(allen): This type is not actually stored in tokens
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
    
    // NOTE(allen): Ambiguous tokens, lexer only,
    // parser figures out the real meaning
	CPP_TOKEN_STAR,
	CPP_TOKEN_AMPERSAND,
	CPP_TOKEN_TILDE,
	CPP_TOKEN_PLUS,
	CPP_TOKEN_MINUS,
	CPP_TOKEN_INCREMENT,
	CPP_TOKEN_DECREMENT,
    
    // NOTE(allen): Precedence 1, LtoR
	CPP_TOKEN_SCOPE,
    
    // NOTE(allen): Precedence 2, LtoR
    CPP_TOKEN_POSTINC, // from increment, parser only
    CPP_TOKEN_POSTDEC, // from decrement, parser only
    CPP_TOKEN_FUNC_STYLE_CAST, // parser only
    CPP_TOKEN_CPP_STYLE_CAST,
    CPP_TOKEN_CALL, // from open paren, parser only
    CPP_TOKEN_INDEX, // from bracket open, parser only
	CPP_TOKEN_DOT,
	CPP_TOKEN_ARROW,
    
    // NOTE(allen): Precedence 3, RtoL
    CPP_TOKEN_PREINC, // from increment, parser only
    CPP_TOKEN_PREDEC, // from decrement, parser only
    CPP_TOKEN_POSITIVE, // from plus, parser only
    CPP_TOKEN_NEGAITVE, // from minus, parser only
	CPP_TOKEN_NOT,
    CPP_TOKEN_BIT_NOT, // from tilde, direct from 'compl'
    CPP_TOKEN_CAST, // from open paren, parser only
    CPP_TOKEN_DEREF, // from star, parser only
    CPP_TOKEN_TYPE_PTR, // from star, parser only
    CPP_TOKEN_ADDRESS, // from ampersand, parser only
    CPP_TOKEN_TYPE_REF, // from ampersand, parser only
    CPP_TOKEN_SIZEOF,
    CPP_TOKEN_ALIGNOF,
    CPP_TOKEN_DECLTYPE,
    CPP_TOKEN_TYPEID,
    CPP_TOKEN_NEW,
    CPP_TOKEN_DELETE,
    CPP_TOKEN_NEW_ARRAY, // from new and bracket open, parser only
    CPP_TOKEN_DELETE_ARRAY, // from delete and bracket open, parser only
    
    // NOTE(allen): Precedence 4, LtoR
	CPP_TOKEN_PTRDOT,
	CPP_TOKEN_PTRARROW,
    
    // NOTE(allen): Precedence 5, LtoR
	CPP_TOKEN_MUL, // from start, parser only
	CPP_TOKEN_DIV,
	CPP_TOKEN_MOD,
    
    // NOTE(allen): Precedence 6, LtoR
    CPP_TOKEN_ADD, // from plus, parser only
    CPP_TOKEN_SUB, // from minus, parser only
    
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
	CPP_TOKEN_BIT_AND, // from ampersand, direct from 'bitand'
    
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
    CPP_TOKEN_DEFINED,
	CPP_TOKEN_INCLUDE_FILE,
    CPP_TOKEN_ERROR_MESSAGE,
    
    // NOTE(allen): used in the parser
    CPP_TOKEN_EOF
};

// TODO(allen): This is a dumb redundant type... probably just
// move towards using String for this everywhere eventually.
struct Cpp_File{
	char *data;
	int size;
};

struct Cpp_Token{
	Cpp_Token_Type type;
	fcpp_i32 start, size;
	fcpp_u16 state_flags;
	fcpp_u16 flags;
};

enum Cpp_Token_Flag{
	CPP_TFLAG_IGNORE = 1 << 0,
	CPP_TFLAG_PP_DIRECTIVE = 1 << 1,
	CPP_TFLAG_PP_BODY = 1 << 2,
	CPP_TFLAG_BAD_ENDING = 1 << 3,
	CPP_TFLAG_MULTILINE = 1 << 4,
	CPP_TFLAG_PARAMETERIZED = 1 << 5,
	CPP_TFLAG_IS_OPERATOR = 1 << 6,
    CPP_TFLAG_IS_KEYWORD = 1 << 7
};

enum Cpp_Preprocessor_State{
	CPP_LEX_PP_DEFAULT,
	CPP_LEX_PP_IDENTIFIER,
	CPP_LEX_PP_MACRO_IDENTIFIER,
	CPP_LEX_PP_INCLUDE,
	CPP_LEX_PP_BODY,
	CPP_LEX_PP_BODY_IF,
	CPP_LEX_PP_NUMBER,
    CPP_LEX_PP_ERROR,
	CPP_LEX_PP_JUNK,
	// NEVER ADD BELOW THIS
	CPP_LEX_PP_COUNT
};

struct Cpp_Lex_Data{
	Cpp_Preprocessor_State pp_state;
	fcpp_i32 pos;
    fcpp_bool32 complete;
};

struct Cpp_Read_Result{
	Cpp_Token token;
	fcpp_i32 pos;
	fcpp_bool8 newline;
	fcpp_bool8 has_result;
};

struct Cpp_Token_Stack{
	Cpp_Token *tokens;
	int count, max_count;
};

struct Cpp_Token_Merge{
	Cpp_Token new_token;
	fcpp_bool32 did_merge;
};

struct Seek_Result{
    fcpp_i32 pos;
    fcpp_bool32 new_line;
};

struct Cpp_Get_Token_Result{
	fcpp_i32 token_index;
	fcpp_bool32 in_whitespace;
};

struct Cpp_Relex_State{
    Cpp_File file;
    Cpp_Token_Stack *stack;
    int start, end, amount;
    int start_token_i;
    int end_token_i;
    int relex_start;
    int tolerance;
    int space_request;
};

#endif

// BOTTOM

