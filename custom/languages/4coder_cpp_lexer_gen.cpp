/*
4coder_lex_gen_cpp.cpp - Model definition for a C++ lexer.
*/

// TOP

#define LANG_NAME_LOWER cpp
#define LANG_NAME_CAMEL Cpp

#include "lexer_generator/4coder_lex_gen_main.cpp"

internal void
build_language_model(void){
    u8 utf8[129];
    smh_utf8_fill(utf8);
    
    smh_set_base_character_names();
    smh_typical_tokens();
    
    // CPP Names
    sm_char_name('!', "Not");
    sm_char_name('&', "And");
    sm_char_name('|', "Or");
    sm_char_name('%', "Mod");
    sm_char_name('^', "Xor");
    sm_char_name('?', "Ternary");
    sm_char_name('/', "Div");
    
    // CPP Direct Toke Kinds
    sm_select_base_kind(TokenBaseKind_Comment);
    sm_direct_token_kind("BlockComment");
    sm_direct_token_kind("LineComment");
    
    sm_select_base_kind(TokenBaseKind_Whitespace);
    sm_direct_token_kind("Backslash");
    
    sm_select_base_kind(TokenBaseKind_LiteralInteger);
    sm_direct_token_kind("LiteralInteger");
    sm_direct_token_kind("LiteralIntegerU");
    sm_direct_token_kind("LiteralIntegerL");
    sm_direct_token_kind("LiteralIntegerUL");
    sm_direct_token_kind("LiteralIntegerLL");
    sm_direct_token_kind("LiteralIntegerULL");
    sm_direct_token_kind("LiteralIntegerHex");
    sm_direct_token_kind("LiteralIntegerHexU");
    sm_direct_token_kind("LiteralIntegerHexL");
    sm_direct_token_kind("LiteralIntegerHexUL");
    sm_direct_token_kind("LiteralIntegerHexLL");
    sm_direct_token_kind("LiteralIntegerHexULL");
    sm_direct_token_kind("LiteralIntegerOct");
    sm_direct_token_kind("LiteralIntegerOctU");
    sm_direct_token_kind("LiteralIntegerOctL");
    sm_direct_token_kind("LiteralIntegerOctUL");
    sm_direct_token_kind("LiteralIntegerOctLL");
    sm_direct_token_kind("LiteralIntegerOctULL");
    
    sm_select_base_kind(TokenBaseKind_LiteralFloat);
    sm_direct_token_kind("LiteralFloat32");
    sm_direct_token_kind("LiteralFloat64");
    
    sm_select_base_kind(TokenBaseKind_LiteralString);
    sm_direct_token_kind("LiteralString");
    sm_direct_token_kind("LiteralStringWide");
    sm_direct_token_kind("LiteralStringUTF8");
    sm_direct_token_kind("LiteralStringUTF16");
    sm_direct_token_kind("LiteralStringUTF32");
    sm_direct_token_kind("LiteralStringRaw");
    sm_direct_token_kind("LiteralStringWideRaw");
    sm_direct_token_kind("LiteralStringUTF8Raw");
    sm_direct_token_kind("LiteralStringUTF16Raw");
    sm_direct_token_kind("LiteralStringUTF32Raw");
    sm_direct_token_kind("LiteralCharacter");
    sm_direct_token_kind("LiteralCharacterWide");
    sm_direct_token_kind("LiteralCharacterUTF8");
    sm_direct_token_kind("LiteralCharacterUTF16");
    sm_direct_token_kind("LiteralCharacterUTF32");
    sm_direct_token_kind("PPIncludeFile");
    sm_direct_token_kind("PPErrorMessage");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_direct_token_kind("KeywordGeneric");
    
    // CPP Operators
    Operator_Set *main_ops = sm_begin_op_set();
    
    sm_select_base_kind(TokenBaseKind_ScopeOpen);
    sm_op("{");
    sm_select_base_kind(TokenBaseKind_ScopeClose);
    sm_op("}");
    sm_select_base_kind(TokenBaseKind_ParentheticalOpen);
    sm_op("(");
    sm_op("[");
    sm_select_base_kind(TokenBaseKind_ParentheticalClose);
    sm_op(")");
    sm_op("]");
    sm_select_base_kind(TokenBaseKind_StatementClose);
    sm_op(";");
    sm_op(":");
    sm_select_base_kind(TokenBaseKind_Operator);
    sm_op("...");
    
    sm_op("::");
    sm_op("++");
    sm_op("--");
    sm_op(".");
    sm_op("->", "Arrow");
    sm_op("+");
    sm_op("-");
    sm_op("!");
    sm_op("~");
    sm_op("*");
    sm_op("&");
    sm_op(".*");
    sm_op("->*", "ArrowStar");
    sm_op("/");
    sm_op("%");
    
    sm_char_name('<', "Left");
    sm_char_name('>', "Right");
    sm_op("<<");
    sm_op(">>");
    
    sm_op("<=>", "Compare");
    
    sm_char_name('<', "Less");
    sm_char_name('>', "Grtr");
    sm_op("<");
    sm_op("<=");
    sm_op(">");
    sm_op(">=");
    sm_op("==");
    sm_op("!=");
    
    sm_op("^");
    sm_op("|");
    sm_op("&&");
    sm_op("||");
    sm_op("?");
    sm_op("=");
    sm_op("+=");
    sm_op("-=");
    sm_op("*=");
    sm_op("/=");
    sm_op("%=");
    
    sm_char_name('<', "Left");
    sm_char_name('>', "Right");
    sm_op("<<=");
    sm_op(">>=");
    
    sm_select_base_kind(TokenBaseKind_StatementClose);
    sm_op(",");
    
    // CPP Preprocess Operators
    Operator_Set *pp_ops = sm_begin_op_set();
    
    sm_op("#", "PPStringify");
    sm_op("##", "PPConcat");
    
    // CPP Keywords
    Keyword_Set *main_keys = sm_begin_key_set("main_keys");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_key("Void");
    sm_key("Bool");
    sm_key("Char");
    sm_key("Int");
    sm_key("Float");
    sm_key("Double");
    sm_key("Long");
    sm_key("Short");
    sm_key("Unsigned");
    sm_key("Signed");
    sm_key("Const");
    sm_key("Volatile");
    sm_key("Asm");
    sm_key("Break");
    sm_key("Case");
    sm_key("Catch");
    sm_key("Continue");
    sm_key("Default");
    sm_key("Do");
    sm_key("Else");
    sm_key("For");
    sm_key("Goto");
    sm_key("If");
    sm_key("Return");
    sm_key("Switch");
    sm_key("Try");
    sm_key("While");
    sm_key("StaticAssert", "static_assert");
    sm_key("ConstCast", "const_cast");
    sm_key("DynamicCast", "dynamic_cast");
    sm_key("ReinterpretCast", "reinterpret_cast");
    sm_key("StaticCast", "static_cast");
    sm_key("Class");
    sm_key("Enum");
    sm_key("Struct");
    sm_key("Typedef");
    sm_key("Union");
    sm_key("Template");
    sm_key("Typename");
    sm_key("Friend");
    sm_key("Namespace");
    sm_key("Private");
    sm_key("Protected");
    sm_key("Public");
    sm_key("Using");
    sm_key("Extern");
    sm_key("Export");
    sm_key("Inline");
    sm_key("Static");
    sm_key("Virtual");
    sm_key("AlignAs");
    sm_key("Explicit");
    sm_key("NoExcept");
    sm_key("NullPtr");
    sm_key("Operator");
    sm_key("Register");
    sm_key("This");
    sm_key("ThreadLocal", "thread_local");
    sm_key("SizeOf");
    sm_key("AlignOf");
    sm_key("DeclType");
    sm_key("TypeID");
    sm_key("New");
    sm_key("Delete");
    
    sm_select_base_kind(TokenBaseKind_LiteralInteger);
    sm_key("LiteralTrue", "true");
    sm_key("LiteralFalse", "false");
    
    sm_select_base_kind(TokenBaseKind_Identifier);
    sm_key_fallback("Identifier");
    
    // CPP Preprocess Directives
    Keyword_Set *pp_directive_set = sm_begin_key_set("pp_directives");
    
    sm_select_base_kind(TokenBaseKind_Preprocessor);
    sm_key("PPInclude", "include");
    sm_key("PPVersion", "version");
    sm_key("PPDefine", "define");
    sm_key("PPUndef", "undef");
    sm_key("PPIf", "if");
    sm_key("PPIfDef", "ifdef");
    sm_key("PPIfNDef", "ifndef");
    sm_key("PPElse", "else");
    sm_key("PPElIf", "elif");
    sm_key("PPEndIf", "endif");
    sm_key("PPError", "error");
    sm_key("PPImport", "import");
    sm_key("PPUsing", "using");
    sm_key("PPLine", "line");
    sm_key("PPPragma", "pragma");
    
    sm_select_base_kind(TokenBaseKind_LexError);
    sm_key_fallback("PPUnknown");
    
    // CPP Preprocess Keywords
    Keyword_Set *pp_keys = sm_begin_key_set("pp_keys");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_key("PPDefined", "defined");
    
    // State Machine
    State *root = sm_begin_state_machine();
    
    Flag *is_hex = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_oct = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_wide  = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_utf8  = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_utf16 = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_utf32 = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_char  = sm_add_flag(FlagResetRule_AutoZero);
    
    Flag *is_pp_body      = sm_add_flag(FlagResetRule_KeepState);
    Flag *is_include_body = sm_add_flag(FlagResetRule_KeepState);
    Flag *is_error_body   = sm_add_flag(FlagResetRule_KeepState);
    
    sm_flag_bind(is_pp_body, TokenBaseFlag_PreprocessorBody);
    
#define AddState(N) State *N = sm_add_state(#N)
    
    AddState(identifier);
    AddState(whitespace);
    AddState(whitespace_end_pp);
    AddState(error_body);
    AddState(backslash);
    
    AddState(operator_or_fnumber_dot);
    AddState(operator_or_comment_slash);
    
    AddState(number);
    AddState(znumber);
    
    AddState(fnumber_decimal);
    AddState(fnumber_exponent);
    AddState(fnumber_exponent_sign);
    AddState(fnumber_exponent_digits);
    
    AddState(number_hex_first);
    AddState(number_hex);
    AddState(number_oct);
    
    AddState(U_number);
    AddState(L_number);
    AddState(UL_number);
    AddState(LU_number);
    AddState(l_number);
    AddState(Ul_number);
    AddState(lU_number);
    AddState(LL_number);
    AddState(ULL_number);
    
    AddState(pp_directive_whitespace);
    AddState(pp_directive_first);
    AddState(pp_directive);
    AddState(pp_directive_emit);
    
    AddState(include_pointy);
    AddState(include_quotes);
    
    AddState(pre_L);
    AddState(pre_u);
    AddState(pre_U);
    AddState(pre_u8);
    AddState(pre_R);
    
    AddState(character);
    AddState(string);
    AddState(string_esc);
    AddState(string_esc_oct2);
    AddState(string_esc_oct1);
    AddState(string_esc_hex);
    AddState(string_esc_universal_8);
    AddState(string_esc_universal_7);
    AddState(string_esc_universal_6);
    AddState(string_esc_universal_5);
    AddState(string_esc_universal_4);
    AddState(string_esc_universal_3);
    AddState(string_esc_universal_2);
    AddState(string_esc_universal_1);
    
    AddState(raw_string);
    AddState(raw_string_get_delim);
    AddState(raw_string_finish_delim);
    AddState(raw_string_find_close);
    AddState(raw_string_try_delim);
    AddState(raw_string_try_quote);
    
    AddState(comment_block);
    AddState(comment_block_try_close);
    AddState(comment_block_newline);
    AddState(comment_line);
    
    Operator_Set *main_ops_without_dot_or_slash = smo_copy_op_set(main_ops);
    smo_remove_ops_with_prefix(main_ops_without_dot_or_slash, ".");
    smo_remove_ops_with_prefix(main_ops_without_dot_or_slash, "/");
    
    Operator_Set *main_ops_with_dot = smo_copy_op_set(main_ops);
    smo_remove_ops_without_prefix(main_ops_with_dot, ".");
    smo_ops_string_skip(main_ops_with_dot, 1);
    
    ////
    
    sm_select_state(root);
    
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("EOF");
        sm_case_eof(emit);
    }
    
    sm_case("abcdefghijklmnopqrstvwxyz"
            "ABCDEFGHIJKMNOPQSTVWXYZ"
            "_$",
            identifier);
    sm_case(utf8, identifier);
    sm_case("L", pre_L);
    sm_case("u", pre_u);
    sm_case("U", pre_U);
    sm_case("R", pre_R);
    
    sm_case_flagged(is_error_body, true, " \r\t\f\v", error_body);
    sm_case_flagged(is_error_body, false, " \r\t\f\v", whitespace);
    sm_case("\n", whitespace_end_pp);
    sm_case("\\", backslash);
    
    sm_case(".", operator_or_fnumber_dot);
    sm_case("/", operator_or_comment_slash);
    {
        Character_Set *char_set = smo_new_char_set();
        smo_char_set_union_ops_firsts(char_set, main_ops_without_dot_or_slash);
        smo_char_set_remove(char_set, ".</");
        char *char_set_array = smo_char_set_get_array(char_set);
        State *operator_state = smo_op_set_lexer_root(main_ops_without_dot_or_slash, root, "LexError");
        sm_case_peek(char_set_array, operator_state);
        sm_case_peek_flagged(is_include_body, false, "<", operator_state);
    }
    
    sm_case_flagged(is_include_body, true, "<", include_pointy);
    sm_case_flagged(is_include_body, true, "\"", include_quotes);
    
    sm_case("123456789", number);
    sm_case("0", znumber);
    
    sm_case_flagged(is_include_body, false, "\"", string);
    sm_case("\'", character);
    sm_case_flagged(is_pp_body, false, "#", pp_directive_whitespace);
    {
        State *operator_state = smo_op_set_lexer_root(pp_ops, root, "LexError");
        sm_case_peek_flagged(is_pp_body, true, "#", operator_state);
    }
    
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback(emit);
    }
    
    ////
    
    sm_select_state(identifier);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_$"
            "0123456789",
            identifier);
    sm_case(utf8, identifier);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_keys(is_pp_body, pp_keys);
        sm_emit_handler_keys(main_keys);
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(whitespace);
    sm_case(" \t\r\f\v", whitespace);
    sm_case("\n", whitespace_end_pp);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Whitespace");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(whitespace_end_pp);
    sm_set_flag(is_pp_body, false);
    sm_set_flag(is_include_body, false);
    sm_set_flag(is_error_body, false);
    sm_fallback_peek(whitespace);
    
    ////
    
    sm_select_state(error_body);
    sm_case("\r", error_body);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("PPErrorMessage");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("PPErrorMessage");
        sm_case_eof_peek(emit);
    }
    sm_fallback(error_body);
    
    ////
    
    sm_select_state(backslash);
    sm_case("\r", backslash);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Backslash");
        sm_case("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Backslash");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(operator_or_comment_slash);
    sm_case("*", comment_block);
    sm_case("/", comment_line);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("DivEq");
        sm_case("=", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Div");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(operator_or_fnumber_dot);
    sm_case("0123456789", fnumber_decimal);
    {
        Character_Set *char_set = smo_new_char_set();
        smo_char_set_union_ops_firsts(char_set, main_ops_with_dot);
        char *char_set_array = smo_char_set_get_array(char_set);
        State *operator_state = smo_op_set_lexer_root(main_ops_with_dot, root, "LexError");
        sm_case_peek(char_set_array, operator_state);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Dot");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number);
    sm_case("0123456789", number);
    sm_case(".", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralInteger");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(znumber);
    sm_case(".", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    sm_case("Xx", number_hex_first);
    sm_case("01234567", number_oct);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralInteger");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_decimal);
    sm_case("0123456789", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_exponent);
    sm_case("+-", fnumber_exponent_sign);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_exponent_sign);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_exponent_digits);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_hex_first);
    sm_set_flag(is_hex, true);
    sm_case("0123456789abcdefABCDEF", number_hex);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_hex);
    sm_case("0123456789abcdefABCDEF", number_hex);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralIntegerHex");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_oct);
    sm_set_flag(is_oct, true);
    sm_case("01234567", number_oct);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralIntegerOct");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(U_number);
    sm_case("L", UL_number);
    sm_case("l", Ul_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexU");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctU");
        sm_emit_handler_direct("LiteralIntegerU");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(L_number);
    sm_case("L", LL_number);
    sm_case("Uu", LU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctL");
        sm_emit_handler_direct("LiteralIntegerL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(l_number);
    sm_case("l", LL_number);
    sm_case("Uu", lU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctL");
        sm_emit_handler_direct("LiteralIntegerL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(LL_number);
    sm_case("Uu", ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexLL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctLL");
        sm_emit_handler_direct("LiteralIntegerLL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(UL_number);
    sm_case("L", ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(Ul_number);
    sm_case("l", ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(LU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(lU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexULL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctULL");
        sm_emit_handler_direct("LiteralIntegerULL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(pp_directive_whitespace);
    sm_case(" \t\f\v", pp_directive_whitespace);
    sm_case_peek("abcdefghijklmnopqrstuvwxyz"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "_"
                 "0123456789",
                 pp_directive_first);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(pp_directive_first);
    sm_delim_mark_first();
    sm_set_flag(is_pp_body, true);
    sm_fallback_peek(pp_directive);
    
    ////
    
    sm_select_state(pp_directive);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_"
            "0123456789",
            pp_directive);
    sm_fallback_peek(pp_directive_emit);
    
    ////
    
    sm_select_state(pp_directive_emit);
    sm_delim_mark_one_past_last();
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_check_set_flag("PPInclude", is_include_body, true);
        sm_emit_check_set_flag("PPError", is_error_body, true);
        sm_emit_handler_keys_delim(pp_directive_set);
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(include_pointy);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_. /\\"
            "0123456789",
            include_pointy);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("PPIncludeFile");
        sm_case(">", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(include_quotes);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_. /\\"
            "0123456789",
            include_quotes);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("PPIncludeFile");
        sm_case("\"", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(pre_L);
    sm_set_flag(is_wide, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_u);
    sm_set_flag(is_utf16, true);
    sm_case("\"", string);
    sm_case("8", pre_u8);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_U);
    sm_set_flag(is_utf32, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_u8);
    sm_set_flag(is_utf8, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_R);
    sm_case("\"", raw_string);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(character);
    sm_set_flag(is_char, true);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_wide, "LiteralStringWide");
        sm_emit_handler_direct(is_utf8 , "LiteralStringUTF8");
        sm_emit_handler_direct(is_utf16, "LiteralStringUTF16");
        sm_emit_handler_direct(is_utf32, "LiteralStringUTF32");
        sm_emit_handler_direct("LiteralString");
        sm_case_flagged(is_char, false, "\"", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_wide, "LiteralCharacterWide");
        sm_emit_handler_direct(is_utf8 , "LiteralCharacterUTF8");
        sm_emit_handler_direct(is_utf16, "LiteralCharacterUTF16");
        sm_emit_handler_direct(is_utf32, "LiteralCharacterUTF32");
        sm_emit_handler_direct("LiteralCharacter");
        sm_case_flagged(is_char, true, "\'", emit);
    }
    sm_case("\\", string_esc);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_case_flagged(is_char, true, "\"", string);
    sm_case_flagged(is_char, false, "\'", string);
    sm_fallback(string);
    
    ////
    
    sm_select_state(string_esc);
    sm_case("\n'\"?\\abfnrtv", string);
    sm_case("01234567", string_esc_oct2);
    sm_case("x", string_esc_hex);
    sm_case("u", string_esc_universal_4);
    sm_case("U", string_esc_universal_8);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(string);
    
    ////
    
    sm_select_state(string_esc_oct2);
    sm_case("01234567", string_esc_oct1);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_oct1);
    sm_case("01234567", string);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_hex);
    sm_case("0123456789abcdefABCDEF", string_esc_hex);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_8);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_7);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_7);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_6);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_6);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_5);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_5);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_4);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_4);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_3);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_3);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_2);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_2);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_1);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_1);
    sm_case("0123456789abcdefABCDEF", string);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(raw_string);
    sm_delim_mark_first();
    sm_fallback_peek(raw_string_get_delim);
    
    ////
    
    sm_select_state(raw_string_get_delim);
    sm_case_peek("(", raw_string_finish_delim);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case(" \\)", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(raw_string_get_delim);
    
    ////
    
    sm_select_state(raw_string_finish_delim);
    sm_delim_mark_one_past_last();
    sm_fallback_peek(raw_string_find_close);
    
    ////
    
    sm_select_state(raw_string_find_close);
    sm_case(")", raw_string_try_delim);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(raw_string_find_close);
    
    ////
    
    sm_select_state(raw_string_try_delim);
    sm_match_delim(raw_string_try_quote, raw_string_find_close);
    
    ////
    
    sm_select_state(raw_string_try_quote);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_wide, "LiteralStringWideRaw");
        sm_emit_handler_direct(is_utf8 , "LiteralStringUTF8Raw");
        sm_emit_handler_direct(is_utf16, "LiteralStringUTF16Raw");
        sm_emit_handler_direct(is_utf32, "LiteralStringUTF32Raw");
        sm_emit_handler_direct("LiteralStringRaw");
        sm_case("\"", emit);
    }
    sm_fallback_peek(raw_string_find_close);
    
    ////
    
    sm_select_state(comment_block);
    sm_case("*", comment_block_try_close);
    sm_case("\n", comment_block_newline);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("BlockComment");
        sm_case_eof_peek(emit);
    }
    sm_fallback(comment_block);
    
    ////
    
    sm_select_state(comment_block_try_close);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("BlockComment");
        sm_case("/", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("BlockComment");
        sm_case_eof_peek(emit);
    }
    sm_case("*", comment_block_try_close);
    sm_fallback(comment_block);
    
    ////
    
    sm_select_state(comment_block_newline);
    sm_set_flag(is_pp_body, false);
    sm_set_flag(is_include_body, false);
    sm_fallback_peek(comment_block);
    
    ////
    
    sm_select_state(comment_line);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LineComment");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LineComment");
        sm_case_eof_peek(emit);
    }
    sm_fallback(comment_line);
}

// BOTTOM

