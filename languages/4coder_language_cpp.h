/*
4coder_language_cpp.h - Sets up the C++ language context.
*/

// TOP

#if !defined(FCODER_LANGUAGE_CPP_H)
#define FCODER_LANGUAGE_CPP_H

// TODO(allen): Organize this better!

internal Token_Array
lex_cpp_initial(Base_Allocator *allocator, String_Const_u8 contents){
    Token_Array result = {};
    result.tokens = base_array(allocator, Token, 2);
    result.count = 2;
    result.max = 2;
    result.tokens[0].pos = 0;
    result.tokens[0].size = contents.size;
    result.tokens[0].kind = TokenBaseKind_COUNT;
    result.tokens[0].sub_kind = 0;
    result.tokens[1].pos = contents.size;
    result.tokens[1].size = 0;
    result.tokens[1].kind = TokenBaseKind_EOF;
    return(result);
}

#if 0
static Parse_Context_ID parse_context_language_cpp;

#define PSAT(s, t) {s, sizeof(s)-1, t}
static void
init_language_cpp(Application_Links *app){
    if (parse_context_language_cpp != 0) return;
    
    Parser_String_And_Type kw[] = {
        PSAT("true"  , CPP_TOKEN_TRUE),
        PSAT("false" , CPP_TOKEN_FALSE),
        
        PSAT("and"      , CPP_TOKEN_AND),
        PSAT("and_eq"   , CPP_TOKEN_ANDEQ),
        PSAT("bitand"   , CPP_TOKEN_BIT_AND),
        PSAT("bitor"    , CPP_TOKEN_BIT_OR),
        PSAT("or"       , CPP_TOKEN_OR),
        PSAT("or_eq"    , CPP_TOKEN_OREQ),
        PSAT("sizeof"   , CPP_TOKEN_SIZEOF),
        PSAT("alignof"  , CPP_TOKEN_ALIGNOF),
        PSAT("decltype" , CPP_TOKEN_DECLTYPE),
        PSAT("throw"    , CPP_TOKEN_THROW),
        PSAT("new"      , CPP_TOKEN_NEW),
        PSAT("delete"   , CPP_TOKEN_DELETE),
        PSAT("xor"      , CPP_TOKEN_BIT_XOR),
        PSAT("xor_eq"   , CPP_TOKEN_XOREQ),
        PSAT("not"      , CPP_TOKEN_NOT),
        PSAT("not_eq"   , CPP_TOKEN_NOTEQ),
        PSAT("typeid"   , CPP_TOKEN_TYPEID),
        PSAT("compl"    , CPP_TOKEN_BIT_NOT),
        
        PSAT("void"   , CPP_TOKEN_VOID),
        PSAT("bool"   , CPP_TOKEN_BOOL),
        PSAT("char"   , CPP_TOKEN_CHAR),
        PSAT("int"    , CPP_TOKEN_INT),
        PSAT("float"  , CPP_TOKEN_FLOAT),
        PSAT("double" , CPP_TOKEN_DOUBLE),
        
        PSAT("long"     , CPP_TOKEN_LONG),
        PSAT("short"    , CPP_TOKEN_SHORT),
        PSAT("unsigned" , CPP_TOKEN_UNSIGNED),
        PSAT("signed"   , CPP_TOKEN_SIGNED),
        
        PSAT("const"    , CPP_TOKEN_CONST),
        PSAT("volatile" , CPP_TOKEN_VOLATILE),
        
        PSAT("asm"           , CPP_TOKEN_ASM),
        PSAT("break"         , CPP_TOKEN_BREAK),
        PSAT("case"          , CPP_TOKEN_CASE),
        PSAT("catch"         , CPP_TOKEN_CATCH),
        PSAT("continue"      , CPP_TOKEN_CONTINUE),
        PSAT("default"       , CPP_TOKEN_DEFAULT),
        PSAT("do"            , CPP_TOKEN_DO),
        PSAT("else"          , CPP_TOKEN_ELSE),
        PSAT("for"           , CPP_TOKEN_FOR),
        PSAT("goto"          , CPP_TOKEN_GOTO),
        PSAT("if"            , CPP_TOKEN_IF),
        PSAT("return"        , CPP_TOKEN_RETURN),
        PSAT("switch"        , CPP_TOKEN_SWITCH),
        PSAT("try"           , CPP_TOKEN_TRY),
        PSAT("while"         , CPP_TOKEN_WHILE),
        PSAT("static_assert" , CPP_TOKEN_STATIC_ASSERT),
        
        PSAT("const_cast"       , CPP_TOKEN_CONST_CAST),
        PSAT("dynamic_cast"     , CPP_TOKEN_DYNAMIC_CAST),
        PSAT("reinterpret_cast" , CPP_TOKEN_REINTERPRET_CAST),
        PSAT("static_cast"      , CPP_TOKEN_STATIC_CAST),
        
        PSAT("class"    , CPP_TOKEN_CLASS),
        PSAT("enum"     , CPP_TOKEN_ENUM),
        PSAT("struct"   , CPP_TOKEN_STRUCT),
        PSAT("typedef"  , CPP_TOKEN_TYPEDEF),
        PSAT("union"    , CPP_TOKEN_UNION),
        PSAT("template" , CPP_TOKEN_TEMPLATE),
        PSAT("typename" , CPP_TOKEN_TYPENAME),
        
        PSAT("friend"    , CPP_TOKEN_FRIEND),
        PSAT("namespace" , CPP_TOKEN_NAMESPACE),
        PSAT("private"   , CPP_TOKEN_PRIVATE),
        PSAT("protected" , CPP_TOKEN_PROTECTED),
        PSAT("public"    , CPP_TOKEN_PUBLIC),
        PSAT("using"     , CPP_TOKEN_USING),
        
        PSAT("extern"  , CPP_TOKEN_EXTERN),
        PSAT("export"  , CPP_TOKEN_EXPORT),
        PSAT("inline"  , CPP_TOKEN_INLINE),
        PSAT("static"  , CPP_TOKEN_STATIC),
        PSAT("internal", CPP_TOKEN_STATIC),
        PSAT("virtual" , CPP_TOKEN_VIRTUAL),
        
        PSAT("alignas"      , CPP_TOKEN_ALIGNAS),
        PSAT("explicit"     , CPP_TOKEN_EXPLICIT),
        PSAT("noexcept"     , CPP_TOKEN_NOEXCEPT),
        PSAT("nullptr"      , CPP_TOKEN_NULLPTR),
        PSAT("operator"     , CPP_TOKEN_OPERATOR),
        PSAT("register"     , CPP_TOKEN_REGISTER),
        PSAT("this"         , CPP_TOKEN_THIS),
        PSAT("thread_local" , CPP_TOKEN_THREAD_LOCAL),
        
#if defined(EXTRA_KEYWORDS)
#include EXTRA_KEYWORDS
#undef EXTRA_KEYWORDS
#endif
    };
    
    Parser_String_And_Type pp[] = {
        PSAT("include" , CPP_PP_INCLUDE ),
        PSAT("INCLUDE" , CPP_PP_INCLUDE ),
        PSAT("version" , CPP_PP_VERSION ),
        PSAT("VERSION" , CPP_PP_VERSION ),
        PSAT("ifndef"  , CPP_PP_IFNDEF  ),
        PSAT("IFNDEF"  , CPP_PP_IFNDEF  ),
        PSAT("define"  , CPP_PP_DEFINE  ),
        PSAT("DEFINE"  , CPP_PP_DEFINE  ),
        PSAT("import"  , CPP_PP_IMPORT  ),
        PSAT("IMPORT"  , CPP_PP_IMPORT  ),
        PSAT("pragma"  , CPP_PP_PRAGMA  ),
        PSAT("PRAGMA"  , CPP_PP_PRAGMA  ),
        PSAT("undef"   , CPP_PP_UNDEF   ),
        PSAT("UNDEF"   , CPP_PP_UNDEF   ),
        PSAT("endif"   , CPP_PP_ENDIF   ),
        PSAT("ENDIF"   , CPP_PP_ENDIF   ),
        PSAT("error"   , CPP_PP_ERROR   ),
        PSAT("ERROR"   , CPP_PP_ERROR   ),
        PSAT("ifdef"   , CPP_PP_IFDEF   ),
        PSAT("IFDEF"   , CPP_PP_IFDEF   ),
        PSAT("using"   , CPP_PP_USING   ),
        PSAT("USING"   , CPP_PP_USING   ),
        PSAT("else"    , CPP_PP_ELSE    ),
        PSAT("ELSE"    , CPP_PP_ELSE    ),
        PSAT("elif"    , CPP_PP_ELIF    ),
        PSAT("ELIF"    , CPP_PP_ELIF    ),
        PSAT("line"    , CPP_PP_LINE    ),
        PSAT("LINE"    , CPP_PP_LINE    ),
        PSAT("if"      , CPP_PP_IF      ),
        PSAT("IF"      , CPP_PP_IF      ),
        
#if defined(EXTRA_PREPROPS)
#include EXTRA_PREPROPS
#undef EXTRA_PREPROPS
#endif
    };
    
    parse_context_language_cpp = create_parse_context(app, kw, ArrayCount(kw), pp, ArrayCount(pp));
}
#undef PSAT
#endif

#endif

// BOTTOM



