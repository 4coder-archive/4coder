/*
 * 4coder token types
 */

// TOP

#if !defined(FCODER_TOKEN_H)
#define FCODER_TOKEN_H

typedef i16 Token_Base_Kind;
enum{
    TokenBaseKind_EOF,
    TokenBaseKind_Whitespace,
    TokenBaseKind_LexError,
    TokenBaseKind_Comment,
    TokenBaseKind_Keyword,
    TokenBaseKind_Preprocessor,
    TokenBaseKind_Identifier,
    TokenBaseKind_Operator,
    TokenBaseKind_LiteralInteger,
    TokenBaseKind_LiteralFloat,
    TokenBaseKind_LiteralString,
    TokenBaseKind_ScopeOpen,
    TokenBaseKind_ScopeClose,
    TokenBaseKind_ParentheticalOpen,
    TokenBaseKind_ParentheticalClose,
    
    TokenBaseKind_COUNT,
};

typedef u16 Token_Base_Flag;
enum{
    TokenBaseFlag_PreprocessorBody = 1,
};

struct Token{
    i64 pos;
    i64 size;
    Token_Base_Kind kind;
    Token_Base_Flag flags;
    i16 sub_kind;
    u16 sub_flags;
};

struct Token_Array{
    Token *tokens;
    i64 count;
    i64 max;
};

#endif

// BOTTOM

