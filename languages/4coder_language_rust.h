/*
4coder_language_rust.h - Sets up the Rust language context.
*/

// TOP

#if !defined(FCODER_LANGUAGE_RUST_H)
#define FCODER_LANGUAGE_RUST_H

#if 0
static Parse_Context_ID parse_context_language_rust;

#define PSAT(s, t) {s, sizeof(s)-1, t}
static void
init_language_rust(Application_Links *app){
    if (parse_context_language_rust != 0) return;
    
    Parser_String_And_Type kw[] = {
        PSAT("abstract", CPP_TOKEN_KEY_OTHER),
        PSAT("alignof", CPP_TOKEN_KEY_OTHER),
        PSAT("as", CPP_TOKEN_KEY_OTHER),
        PSAT("become", CPP_TOKEN_KEY_OTHER),
        PSAT("box", CPP_TOKEN_KEY_OTHER),
        PSAT("const", CPP_TOKEN_KEY_OTHER),
        PSAT("crate", CPP_TOKEN_KEY_OTHER),
        PSAT("enum", CPP_TOKEN_KEY_OTHER),
        PSAT("extern", CPP_TOKEN_KEY_OTHER),
        PSAT("false", CPP_TOKEN_KEY_OTHER),
        PSAT("final", CPP_TOKEN_KEY_OTHER),
        PSAT("fn", CPP_TOKEN_KEY_OTHER),
        PSAT("impl", CPP_TOKEN_KEY_OTHER),
        PSAT("in", CPP_TOKEN_KEY_OTHER),
        PSAT("let", CPP_TOKEN_KEY_OTHER),
        PSAT("loop", CPP_TOKEN_KEY_OTHER),
        PSAT("macro", CPP_TOKEN_KEY_OTHER),
        PSAT("match", CPP_TOKEN_KEY_OTHER),
        PSAT("mod", CPP_TOKEN_KEY_OTHER),
        PSAT("move", CPP_TOKEN_KEY_OTHER),
        PSAT("mut", CPP_TOKEN_KEY_OTHER),
        PSAT("offsetof", CPP_TOKEN_KEY_OTHER),
        PSAT("override", CPP_TOKEN_KEY_OTHER),
        PSAT("priv", CPP_TOKEN_KEY_OTHER),
        PSAT("proc", CPP_TOKEN_KEY_OTHER),
        PSAT("pub", CPP_TOKEN_KEY_OTHER),
        PSAT("pure", CPP_TOKEN_KEY_OTHER),
        PSAT("ref", CPP_TOKEN_KEY_OTHER),
        PSAT("return", CPP_TOKEN_KEY_OTHER),
        PSAT("Self", CPP_TOKEN_KEY_OTHER),
        PSAT("self", CPP_TOKEN_KEY_OTHER),
        PSAT("sizeof", CPP_TOKEN_KEY_OTHER),
        PSAT("static", CPP_TOKEN_KEY_OTHER),
        PSAT("struct", CPP_TOKEN_KEY_OTHER),
        PSAT("super", CPP_TOKEN_KEY_OTHER),
        PSAT("trait", CPP_TOKEN_KEY_OTHER),
        PSAT("true", CPP_TOKEN_KEY_OTHER),
        PSAT("type", CPP_TOKEN_KEY_OTHER),
        PSAT("typeof", CPP_TOKEN_KEY_OTHER),
        PSAT("unsafe", CPP_TOKEN_KEY_OTHER),
        PSAT("unsized", CPP_TOKEN_KEY_OTHER),
        PSAT("use", CPP_TOKEN_KEY_OTHER),
        PSAT("virtual", CPP_TOKEN_KEY_OTHER),
        PSAT("where", CPP_TOKEN_KEY_OTHER),
        
        PSAT("break",    CPP_TOKEN_BREAK),
        PSAT("continue", CPP_TOKEN_CONTINUE),
        PSAT("do",       CPP_TOKEN_DO),
        PSAT("else",     CPP_TOKEN_ELSE),
        PSAT("for",      CPP_TOKEN_FOR),
        PSAT("if",       CPP_TOKEN_IF),
        PSAT("while",    CPP_TOKEN_WHILE),
        PSAT("yield",    CPP_TOKEN_KEY_OTHER),
    };
    
    parse_context_language_rust = create_parse_context(app, kw, ArrayCount(kw), 0, 0);
}
#undef PSAT
#endif

#endif

// BOTTOM
