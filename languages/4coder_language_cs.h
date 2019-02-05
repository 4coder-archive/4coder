/*
4coder_language_cs.h - Sets up the C# language context.
*/

// TOP

#if !defined(FCODER_LANGUAGE_CS_H)
#define FCODER_LANGUAGE_CS_H

static Parse_Context_ID parse_context_language_cs;

#define PSAT(s, t) {s, sizeof(s)-1, t}
static void
init_language_cs(Application_Links *app){
    if (parse_context_language_cs != 0) return;
    
    Parser_String_And_Type kw[] = {
        PSAT("abstract", CPP_TOKEN_KEY_OTHER),
        PSAT("as", CPP_TOKEN_KEY_OTHER),
        PSAT("base", CPP_TOKEN_KEY_OTHER),
        PSAT("bool", CPP_TOKEN_KEY_OTHER),
        PSAT("byte", CPP_TOKEN_KEY_OTHER),
        PSAT("char", CPP_TOKEN_KEY_OTHER),
        PSAT("checked", CPP_TOKEN_KEY_OTHER),
        PSAT("class", CPP_TOKEN_KEY_OTHER),
        PSAT("const", CPP_TOKEN_KEY_OTHER),
        PSAT("decimal", CPP_TOKEN_KEY_OTHER),
        PSAT("delegate", CPP_TOKEN_KEY_OTHER),
        PSAT("double", CPP_TOKEN_KEY_OTHER),
        PSAT("enum", CPP_TOKEN_KEY_OTHER),
        PSAT("event", CPP_TOKEN_KEY_OTHER),
        PSAT("explicit", CPP_TOKEN_KEY_OTHER),
        PSAT("extern", CPP_TOKEN_KEY_OTHER),
        PSAT("false", CPP_TOKEN_KEY_OTHER),
        PSAT("fixed", CPP_TOKEN_KEY_OTHER),
        PSAT("float", CPP_TOKEN_KEY_OTHER),
        PSAT("implicit", CPP_TOKEN_KEY_OTHER),
        PSAT("int", CPP_TOKEN_KEY_OTHER),
        PSAT("interface", CPP_TOKEN_KEY_OTHER),
        PSAT("internal", CPP_TOKEN_KEY_OTHER),
        PSAT("is", CPP_TOKEN_KEY_OTHER),
        PSAT("long", CPP_TOKEN_KEY_OTHER),
        PSAT("namespace", CPP_TOKEN_KEY_OTHER),
        PSAT("new", CPP_TOKEN_KEY_OTHER),
        PSAT("null", CPP_TOKEN_KEY_OTHER),
        PSAT("object", CPP_TOKEN_KEY_OTHER),
        PSAT("operator", CPP_TOKEN_KEY_OTHER),
        PSAT("out", CPP_TOKEN_KEY_OTHER),
        PSAT("override", CPP_TOKEN_KEY_OTHER),
        PSAT("params", CPP_TOKEN_KEY_OTHER),
        PSAT("private", CPP_TOKEN_KEY_OTHER),
        PSAT("protected", CPP_TOKEN_KEY_OTHER),
        PSAT("public", CPP_TOKEN_KEY_OTHER),
        PSAT("readonly", CPP_TOKEN_KEY_OTHER),
        PSAT("ref", CPP_TOKEN_KEY_OTHER),
        PSAT("sbyte", CPP_TOKEN_KEY_OTHER),
        PSAT("sealed", CPP_TOKEN_KEY_OTHER),
        PSAT("short", CPP_TOKEN_KEY_OTHER),
        PSAT("sizeof", CPP_TOKEN_KEY_OTHER),
        PSAT("stackalloc", CPP_TOKEN_KEY_OTHER),
        PSAT("static", CPP_TOKEN_KEY_OTHER),
        PSAT("string", CPP_TOKEN_KEY_OTHER),
        PSAT("struct", CPP_TOKEN_KEY_OTHER),
        PSAT("this", CPP_TOKEN_KEY_OTHER),
        PSAT("true", CPP_TOKEN_KEY_OTHER),
        PSAT("typeof", CPP_TOKEN_KEY_OTHER),
        PSAT("uint", CPP_TOKEN_KEY_OTHER),
        PSAT("ulong", CPP_TOKEN_KEY_OTHER),
        PSAT("unchecked", CPP_TOKEN_KEY_OTHER),
        PSAT("unsafe", CPP_TOKEN_KEY_OTHER),
        PSAT("ushort", CPP_TOKEN_KEY_OTHER),
        PSAT("using", CPP_TOKEN_KEY_OTHER),
        PSAT("void", CPP_TOKEN_KEY_OTHER),
        PSAT("volatile", CPP_TOKEN_KEY_OTHER),
        
        PSAT("if",       CPP_TOKEN_IF),
        PSAT("else",     CPP_TOKEN_ELSE),
        PSAT("switch",   CPP_TOKEN_SWITCH),
        PSAT("case",     CPP_TOKEN_CASE),
        PSAT("do",       CPP_TOKEN_DO),
        PSAT("for",      CPP_TOKEN_FOR),
        PSAT("foreach",  CPP_TOKEN_FOR),
        PSAT("in",       CPP_TOKEN_KEY_OTHER),
        PSAT("while",    CPP_TOKEN_WHILE),
        PSAT("break",    CPP_TOKEN_BREAK),
        PSAT("continue", CPP_TOKEN_CONTINUE),
        PSAT("default",  CPP_TOKEN_DEFAULT),
        PSAT("goto",     CPP_TOKEN_GOTO),
        PSAT("return",   CPP_TOKEN_RETURN),
        PSAT("yield",    CPP_TOKEN_KEY_OTHER),
        PSAT("throw",    CPP_TOKEN_THROW),
        PSAT("try",      CPP_TOKEN_TRY),
        PSAT("catch",    CPP_TOKEN_CATCH),
        PSAT("finally",  CPP_TOKEN_CATCH),
        PSAT("lock",     CPP_TOKEN_KEY_OTHER),
    };
    
    Parser_String_And_Type pp[] = {
        PSAT("if", CPP_PP_IF),
        PSAT("else", CPP_PP_ELSE),
        PSAT("elif", CPP_PP_ELIF),
        PSAT("endif", CPP_PP_ENDIF),
        PSAT("define", CPP_PP_DEFINED),
        PSAT("undef", CPP_PP_UNDEF),
        PSAT("warning", CPP_PP_ERROR),
        PSAT("error", CPP_PP_ERROR),
        PSAT("line", CPP_PP_LINE),
        PSAT("pragma", CPP_PP_PRAGMA),
        PSAT("region", CPP_PP_UNKNOWN),
        PSAT("endregion", CPP_PP_UNKNOWN),
    };
    
    parse_context_language_cs = create_parse_context(app, kw, ArrayCount(kw), pp, ArrayCount(pp));
}
#undef PSAT

#endif

// BOTTOM

