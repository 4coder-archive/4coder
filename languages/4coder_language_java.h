/*
4coder_language_java.h - Sets up the Java language context.
*/

// TOP

#if !defined(FCODER_LANGUAGE_JAVA_H)
#define FCODER_LANGUAGE_JAVA_H

static Parse_Context_ID parse_context_language_java;

#define PSAT(s, t) {s, sizeof(s)-1, t}
static void
init_language_java(Application_Links *app){
    if (parse_context_language_java != 0) return;
    
    Parser_String_And_Type kw[] = {
        PSAT("abstract", CPP_TOKEN_KEY_OTHER),
        PSAT("assert", CPP_TOKEN_KEY_OTHER),
        PSAT("boolean", CPP_TOKEN_KEY_OTHER),
        PSAT("char", CPP_TOKEN_KEY_OTHER),
        PSAT("class", CPP_TOKEN_KEY_OTHER),
        PSAT("const", CPP_TOKEN_KEY_OTHER),
        PSAT("default", CPP_TOKEN_KEY_OTHER),
        PSAT("double", CPP_TOKEN_KEY_OTHER),
        PSAT("enum", CPP_TOKEN_KEY_OTHER),
        PSAT("extends", CPP_TOKEN_KEY_OTHER),
        PSAT("final", CPP_TOKEN_KEY_OTHER),
        PSAT("float", CPP_TOKEN_KEY_OTHER),
        PSAT("implements", CPP_TOKEN_KEY_OTHER),
        PSAT("import", CPP_TOKEN_KEY_OTHER),
        PSAT("instanceof", CPP_TOKEN_KEY_OTHER),
        PSAT("int", CPP_TOKEN_KEY_OTHER),
        PSAT("interface", CPP_TOKEN_KEY_OTHER),
        PSAT("long", CPP_TOKEN_KEY_OTHER),
        PSAT("native", CPP_TOKEN_KEY_OTHER),
        PSAT("new", CPP_TOKEN_KEY_OTHER),
        PSAT("package", CPP_TOKEN_KEY_OTHER),
        PSAT("private", CPP_TOKEN_KEY_OTHER),
        PSAT("protected", CPP_TOKEN_KEY_OTHER),
        PSAT("public", CPP_TOKEN_KEY_OTHER),
        PSAT("return", CPP_TOKEN_KEY_OTHER),
        PSAT("short", CPP_TOKEN_KEY_OTHER),
        PSAT("static", CPP_TOKEN_KEY_OTHER),
        PSAT("strictfp", CPP_TOKEN_KEY_OTHER),
        PSAT("super", CPP_TOKEN_KEY_OTHER),
        PSAT("synchronized", CPP_TOKEN_KEY_OTHER),
        PSAT("this", CPP_TOKEN_KEY_OTHER),
        PSAT("transient", CPP_TOKEN_KEY_OTHER),
        PSAT("void", CPP_TOKEN_KEY_OTHER),
        PSAT("volatile", CPP_TOKEN_KEY_OTHER),
        PSAT("true", CPP_TOKEN_KEY_OTHER),
        PSAT("false", CPP_TOKEN_KEY_OTHER),
        PSAT("null", CPP_TOKEN_KEY_OTHER),
        
        PSAT("if",       CPP_TOKEN_IF),
        PSAT("else",     CPP_TOKEN_ELSE),
        PSAT("switch",   CPP_TOKEN_SWITCH),
        PSAT("case",     CPP_TOKEN_CASE),
        PSAT("while",    CPP_TOKEN_WHILE),
        PSAT("do",       CPP_TOKEN_DO),
        PSAT("for",      CPP_TOKEN_FOR),
        PSAT("goto",     CPP_TOKEN_GOTO),
        PSAT("break",    CPP_TOKEN_BREAK),
        PSAT("continue", CPP_TOKEN_CONTINUE),
        PSAT("throw",    CPP_TOKEN_THROW),
        PSAT("throws",   CPP_TOKEN_THROW),
        PSAT("try",      CPP_TOKEN_TRY),
        PSAT("catch",    CPP_TOKEN_CATCH),
        PSAT("finally",  CPP_TOKEN_CATCH),
    };
    
    parse_context_language_java = create_parse_context(app, kw, ArrayCount(kw), 0, 0);
}
#undef PSAT

#endif

// BOTTOM

