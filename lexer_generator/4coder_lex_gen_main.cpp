/*
4coder_lex_gen_main.cpp - A generator for language lexers.
*/

// TOP

#if !defined(LANG_NAME_LOWER) || !defined(LANG_NAME_CAMEL)
#error 4coder_lex_get_main.cpp not correctly included.
#endif

#include "4coder_base_types.h"
#include "4coder_table.h"
#include "4coder_token.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_table.cpp"

////////////////////////////////

struct Keyword{
    Keyword *next;
    String_Const_u8 name;
    String_Const_u8 lexeme;
    Token_Base_Kind base_kind;
};

struct Keyword_Set{
    Keyword *first;
    Keyword *last;
    i32 count;
};

struct Character_Name_Set{
    Table_u64_Data char_to_name;
};

struct Operator{
    Operator *next;
    String_Const_u8 op;
    String_Const_u8 name;
    Token_Base_Kind base_kind;
};

struct Operator_Set{
    Operator *first;
    Operator *last;
    i32 count;
};

struct Direct_Kind{
    Direct_Kind *next;
    String_Const_u8 name;
    Token_Base_Kind base_kind;
};

////////////////////////////////

int main(void){
    // utf-8 bytes
    u8 utf8[129];
    for (u16 i = 0; i < 128; i += 1){
        utf8[i] = i + 128;
    }
    utf8[128] = 0;
    
    // Base Names
    char_name('{', "BraceOp");
    char_name('}', "BraceCl");
    char_name('(', "ParenOp");
    char_name(')', "ParenCl");
    char_name('[', "BrackOp");
    char_name(']', "BrackCl");
    char_name('-', "Minus");
    char_name('+', "Plus");
    char_name('.', "Dot");
    char_name('!', "Bang");
    char_name('*', "Star");
    char_name(',', "Comma");
    char_name(':', "Colon");
    char_name(';', "Semicolon");
    char_name('@', "At");
    char_name('#', "Pound");
    char_name('$', "Dollar");
    char_name('%', "Percent");
    char_name('^', "Carrot");
    char_name('&', "And");
    char_name('=', "Eq");
    char_name('<', "Less");
    char_name('>', "Grtr");
    char_name('~', "Tilde");
    char_name('/', "Slash");
    char_name('?', "Question");
    char_name('|', "Pipe");
    
    // CPP Names
    char_name('!', "Not");
    char_name('|', "Or");
    char_name('%', "Mod");
    char_name('^', "Xor");
    char_name('?', "Ternary");
    
    // Typical Base Token Kinds
    select_base_kind(TokenBaseKind_EOF);
    direct_token_kind("EOF");
    
    select_base_kind(TokenBaseKind_Whitespace);
    direct_token_kind("Whitespace");
    
    select_base_kind(TokenBaseKind_LexError);
    direct_token_kind("LexError");
    
    // CPP Direct Toke Kinds
    select_base_kind(TokenBaseKind_Comment);
    direct_token_kind("BlockComment");
    direct_token_kind("LineComment");
    
    select_base_kind(TokenBaseKind_Whitespace);
    direct_token_kind("Backslash");
    
    select_base_kind(TokenBaseKind_LiteralInteger);
    direct_token_kind("LiteralInteger"):
    direct_token_kind("LiteralIntegerU"):
    direct_token_kind("LiteralIntegerL"):
    direct_token_kind("LiteralIntegerUL"):
    direct_token_kind("LiteralIntegerLL"):
    direct_token_kind("LiteralIntegerULL"):
    direct_token_kind("LiteralIntegerHex"):
    direct_token_kind("LiteralIntegerHexU"):
    direct_token_kind("LiteralIntegerHexL"):
    direct_token_kind("LiteralIntegerHexUL"):
    direct_token_kind("LiteralIntegerHexLL"):
    direct_token_kind("LiteralIntegerHexULL"):
    direct_token_kind("LiteralIntegerOct"):
    direct_token_kind("LiteralIntegerOctU"):
    direct_token_kind("LiteralIntegerOctL"):
    direct_token_kind("LiteralIntegerOctUL"):
    direct_token_kind("LiteralIntegerOctLL"):
    direct_token_kind("LiteralIntegerOctULL"):
    
    select_base_kind(TokenBaseKind_LiteralFloat);
    direct_token_kind("LiteralFloat32"):
    direct_token_kind("LiteralFloat64"):
    
    select_base_kind(TokenBaseKind_LiteralString);
    direct_token_kind("LiteralString"):
    direct_token_kind("LiteralStringWide"):
    direct_token_kind("LiteralStringUTF8"):
    direct_token_kind("LiteralStringUTF16"):
    direct_token_kind("LiteralStringUTF32"):
    direct_token_kind("LiteralStringRaw"):
    direct_token_kind("LiteralStringWideRaw"):
    direct_token_kind("LiteralStringUTF8Raw"):
    direct_token_kind("LiteralStringUTF16Raw"):
    direct_token_kind("LiteralStringUTF32Raw"):
    direct_token_kind("LiteralCharacter"):
    direct_token_kind("LiteralCharacterWide"):
    direct_token_kind("LiteralCharacterUTF8"):
    direct_token_kind("LiteralCharacterUTF16"):
    direct_token_kind("LiteralCharacterUTF32"):
    direct_token_kind("PPIncludeFile");
    
    // CPP Operators
    Operator_Set *main_ops = begin_op_set();
    
    select_base_kind(TokenBaseKind_ScopeOpen);
    op("{");
    select_base_kind(TokenBaseKind_ScopeClose);
    op("}");
    select_base_kind(TokenBaseKind_ParentheticalOpen);
    op("(");
    op("[");
    select_base_kind(TokenBaseKind_ParentheticalClose);
    op(")");
    op("]");
    select_base_kind(TokenBaseKind_Operator);
    op(":");
    op("...");
    
    op("::");
    op("++");
    op("--");
    op(".");
    op("->", "Arrow");
    op("+");
    op("-");
    op("!");
    op("~");
    op("*");
    op("&");
    op(".*");
    op("->*", "ArrowStar");
    op("/");
    op("%");
    
    char_name('<', "Left");
    char_name('>', "Right");
    op("<<");
    op(">>");
    
    op("<=>", "Compare");
    
    char_name('<', "Less");
    char_name('>', "Grtr");
    op("<");
    op("<=");
    op(">");
    op(">=");
    op("==");
    op("!=");
    
    op("^");
    op("|");
    op("&&");
    op("||");
    op("?");
    op("=");
    op("+=");
    op("-=");
    op("*=");
    op("/=");
    op("%=");
    
    char_name('<', "Left");
    char_name('>', "Right");
    op("<<=");
    op(">>=");
    
    op(",");
    
    // CPP Preprocess Operators
    Operator_Set *pp_ops = begin_op_set();
    
    op("#", "PPStringify");
    op("##", "PPConcat");
    
    // CPP Keywords
    Keyword_Set *main_keys = begin_key_set();
    
    select_base_kind(TokenBaseKind_Keyword);
    key("Void");
    key("Bool");
    key("Char");
    key("Int");
    key("Float");
    key("Double");
    key("Long");
    key("Short");
    key("Unsigned");
    key("Signed");
    key("Const");
    key("Volatile");
    key("Asm");
    key("Break");
    key("Case");
    key("Catch");
    key("Continue");
    key("Default");
    key("Do");
    key("Else");
    key("For");
    key("Goto");
    key("If");
    key("Return");
    key("Switch");
    key("Try");
    key("While");
    key("StaticAssert", "static_assert");
    key("ConstCast", "const_cast");
    key("DynamicCast", "dynamic_cast");
    key("ReinterpretCast", "reinterpret_cast");
    key("StaticCast", "static_cast");
    key("Class");
    key("Enum");
    key("Struct");
    key("Typedef");
    key("Union");
    key("Template");
    key("Typename");
    key("Friend");
    key("Namespace");
    key("Private");
    key("Protected");
    key("Public");
    key("Using");
    key("Extern");
    key("Export");
    key("Inline");
    key("Static");
    key("Virtual");
    key("AlignAs");
    key("Explicit");
    key("NoExcept");
    key("NullPtr");
    key("Operator");
    key("Register");
    key("This");
    key("ThreadLocal", "thread_local");
    key("SizeOf");
    key("AlignOf");
    key("DeclType");
    key("TypeID");
    key("New");
    key("Delete");
    key_unmatchable("KeywordGeneric");
    
    select_base_kind(TokenBaseKind_LiteralInteger);
    key("LiteralTrue", "true");
    key("LiteralFalse", "false");
    
    select_base_kind(TokenBaseKind_Identifier);
    key_fallback("Identifier");
    
    // CPP Preprocess Directives
    Keyword_Set *pp_directive_set = begin_key_set();
    
    select_base_kind(TokenBaseKind_Preprocessor);
    key("PPInclude", "#include");
    key("PPVersion", "#version");
    key("PPDefine", "#define");
    key("PPUndef", "#undef");
    key("PPIf", "#if");
    key("PPIfDef", "#ifdef");
    key("PPIfNDef", "#ifndef");
    key("PPElse", "#else");
    key("PPElIf", "#elif");
    key("PPEndIf", "#endif");
    key("PPError", "#error");
    key("PPImport", "#import");
    key("PPUsing", "#using");
    key("PPLine", "#line");
    key("PPPragma", "#pragma");
    key_fallback("PPUnknown");
    
    // CPP Preprocess Keywords
    Keyword_Set *pp_keys = begin_key_set();
    
    select_base_kind(TokenBaseKind_Keyword);
    key("PPDefined", "defined");
    
    // State Machine
    State *root = begin_state_machine();
    
    Flag *is_hex = add_flag(AutoZero);
    Flag *is_oct = add_flag(AutoZero);
    Flag *is_pp_body = add_flag(KeepState);
    Flag *is_include_body = add_flag(KeepState);
    Flag *is_wide = add_flag(AutoZero);
    Flag *is_utf8 = add_flag(AutoZero);
    Flag *is_utf16 = add_flag(AutoZero);
    Flag *is_utf32 = add_flag(AutoZero);
    Flag *is_char = add_flag(AutoZero);
    
    flag_bind(is_pp_body, TokenBaseFlag_PreprocessorBody);
    
    State *identifier = add_state();
    State *whitespace = add_state();
    State *whitespace_end_pp = add_state();
    State *backslash = add_state();
    
    State *operator_or_fnumber_dot = add_state();
    State *operator_or_comment_slash = add_state();
    
    State *number = add_state();
    State *znumber = add_state();
    
    State *fnumber_decimal = add_state();
    State *fnumber_exponent = add_state();
    State *fnumber_exponent_sign = add_state();
    State *fnumber_exponent_digits = add_state();
    
    State *number_hex = add_state();
    State *number_oct = add_state();
    
    State *U_number = add_state();
    State *L_number = add_state();
    State *UL_number = add_state();
    State *LU_number = add_state();
    State *l_number = add_state();
    State *Ul_number = add_state();
    State *lU_number = add_state();
    State *LL_number = add_state();
    State *ULL_number = add_state();
    
    State *pp_directive = add_state();
    
    State *include_pointy = add_state();
    State *include_quotes = add_state();
    
    State *pre_L = add_state();
    State *pre_u = add_state();
    State *pre_U = add_state();
    State *pre_u8 = add_state();
    State *pre_R = add_state();
    
    State *character = add_state();
    State *string = add_state();
    State *string_esc = add_state();
    State *string_esc_oc2 = add_state();
    State *string_esc_oc1 = add_state();
    State *string_esc_hex = add_state();
    State *string_esc_universal_8 = add_state();
    State *string_esc_universal_7 = add_state();
    State *string_esc_universal_6 = add_state();
    State *string_esc_universal_5 = add_state();
    State *string_esc_universal_4 = add_state();
    State *string_esc_universal_3 = add_state();
    State *string_esc_universal_2 = add_state();
    State *string_esc_universal_1 = add_state();
    
    State *raw_string = add_state();
    State *raw_string_get_delim = add_state();
    State *raw_string_finish_delim = add_state();
    State *raw_string_find_close = add_state();
    State *raw_string_try_delim = add_state();
    State *raw_string_try_quote = add_state();
    
    State *comment_block = add_state();
    State *comment_block_try_close = add_state();
    State *comment_block_newline = add_state();
    State *comment_line = add_state();
    
    Operator_Set *main_ops_without_dot_or_slash = copy_op_set(main_ops);
    remove_ops_with_prefix(main_ops_without_dot, ".");
    remove_ops_with_prefix(main_ops_without_dot, "/");
    
    Operator_Set *main_ops_with_dot = copy_op_set(main_ops);
    remove_ops_without_prefix(main_ops_with_dot, ".");
    ops_string_skip(main_ops_with_dot, 1);
    
    ////
    
    select_state(root);
    sm_case("abcdefghijklmnopqrstvwxyz"
            "ABCDEFGHIJKMNOPQSTVWXYZ"
            "_$",
            identifier);
    sm_case(utf8, identifier);
    sm_case("L", pre_L);
    sm_case("u", pre_u);
    sm_case("U", pre_U);
    sm_case("R", pre_R);
    
    sm_case(" \r\t\f\v", whitespace);
    sm_case("\n", whitespace_end_pp);
    sm_case("\\", backslash);
    
    sm_case(".", operator_or_fnumber_dot);
    sm_case("/", operator_or_comment_slash);
    {
        Character_Set *char_set = new_char_set();
        char_set_union_ops(char_set, main_ops_without_dot_or_slash);
        char_set_remove(char_set, ".</");
        char *char_set_array = char_set_get_array(char_set);
        State *operator_state = op_set_lexer_root(main_ops_without_dot_or_slash);
        sm_case(char_set_array, operator_state);
        sm_case_flagged(is_include_body, false, "<", operator_state);
    }
    
    sm_case_flagged(is_include_body, true, "<", include_pointy);
    sm_case_flagged(is_include_body, true, "\"", include_quotes);
    
    sm_case("123456789", number);
    sm_case("0", znumber);
    
    sm_case_flagged(is_include_body, false, "\"", string);
    sm_case("\'", character);
    sm_case("#", pp_directive);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LexError");
        sm_fallback(emit);
    }
    
    ////
    
    select_state(identifier);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_$"
            "0123456789",
            identifier);
    sm_case(utf8);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_keys(emit, main_keys);
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(whitespace);
    sm_case(" \t\r\f\v", whitespace);
    sm_case("\n", whitespace_end_pp);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("Whitespace");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(whitespace_end_pp);
    sm_set_flag(is_pp_body, false);
    sm_set_flag(is_include_body, false);
    sm_fallback_peek(whitespace);
    
    ////
    
    select_state(backslash);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("Backslash");
        sm_case("\n", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("Backslash");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(operator_or_comment_slash);
    sm_case("*", comment_block);
    sm_case("/", comment_line);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("SlashEq");
        sm_case("=", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("Slash");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(operator_or_fnumber_dot);
    sm_case("012345679", fnumber_decimal);
    {
        Character_Set *char_set = new_char_set();
        char_set_union_ops(main_ops_with_dot);
        char *char_set_array = char_set_get_array(char_set);
        State *operator_state = op_set_lexer_root(main_ops_with_dot);
        sm_case(char_set_array, operator_state);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("Dot");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(number);
    sm_case("012345679", number);
    sm_case(".", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralInteger");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(znumber);
    sm_case(".", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    sm_case("Xx", number_hex);
    sm_case("01234567", number_oct);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralInteger");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(fnumber_decimal);
    sm_case("012345679", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(fnumber_exponent);
    sm_case("+-", fnumber_exponent_sign);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(fnumber_exponent_sign);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(fnumber_exponent_digits);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(number_hex);
    sm_set_flag(is_hex, true);
    sm_case("012345679abcdefABCDEF", number_hex);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralIntegerHex");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(number_oct);
    sm_set_flag(is_oct, true);
    sm_case("01234567", number_oct);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LiteralIntegerOct");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(U_number);
    sm_case("L", UL_number);
    sm_case("l", Ul_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexU");
        handler_token_kind(is_oct, "LiteralIntegerOctU");
        handler_token_kind("LiteralIntegerU");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(L_number);
    sm_case("L", LL_number);
    sm_case("Uu", LU_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexL");
        handler_token_kind(is_oct, "LiteralIntegerOctL");
        handler_token_kind("LiteralIntegerL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(l_number);
    sm_case("l", LL_number);
    sm_case("Uu", lU_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexL");
        handler_token_kind(is_oct, "LiteralIntegerOctL");
        handler_token_kind("LiteralIntegerL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(LL_number);
    sm_case("Uu", ULL_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexLL");
        handler_token_kind(is_oct, "LiteralIntegerOctLL");
        handler_token_kind("LiteralIntegerLL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(UL_number);
    sm_case("L", ULL_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexUL");
        handler_token_kind(is_oct, "LiteralIntegerOctUL");
        handler_token_kind("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(Ul_number);
    sm_case("l", ULL_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexUL");
        handler_token_kind(is_oct, "LiteralIntegerOctUL");
        handler_token_kind("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(LU_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexUL");
        handler_token_kind(is_oct, "LiteralIntegerOctUL");
        handler_token_kind("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(lU_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexUL");
        handler_token_kind(is_oct, "LiteralIntegerOctUL");
        handler_token_kind("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(ULL_number);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_hex, "LiteralIntegerHexULL");
        handler_token_kind(is_oct, "LiteralIntegerOctULL");
        handler_token_kind("LiteralIntegerULL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(pp_directive);
    sm_set_flag(is_pp_body, true);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_"
            "0123456789",
            pp_directive);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_keys(emit, pp_directive_set);
        handler_key_flag_on_result(emit, "PPInclude", is_include_body, true);
        sm_fallback_peek(emit);
    }
    
    ////
    
    select_state(include_pointy);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_. /\\"
            "0123456789",
            include_pointy);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("PPIncludeFile");
        sm_case(">", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LexError");
        sm_fallback(emit);
    }
    
    ////
    
    select_state(include_quotes);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_. /\\"
            "0123456789",
            include_pointy);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("PPIncludeFile");
        sm_case("\"", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LexError");
        sm_fallback(emit);
    }
    
    ////
    
    select_state(pre_L);
    sm_set_flag(is_wide, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    select_state(pre_u);
    sm_set_flag(is_utf16, true);
    sm_case("\"", string);
    sm_case("8", pre_u8);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    select_state(pre_U);
    sm_set_flag(is_utf32, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    select_state(pre_u8);
    sm_set_flag(is_utf8, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    select_state(pre_R);
    sm_case("\"", raw_string);
    sm_fallback_peek(identifier);
    
    ////
    
    select_state(character);
    sm_set_flag(is_char, true);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_wide, "LiteralStringWide");
        handler_token_kind(is_utf8 , "LiteralStringUTF8");
        handler_token_kind(is_utf16, "LiteralStringUTF16");
        handler_token_kind(is_utf32, "LiteralStringUTF32");
        handler_token_kind("LiteralString");
        sm_case_flagged(is_char, false, "\"", emit);
    }
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_wide, "LiteralCharacterWide");
        handler_token_kind(is_utf8 , "LiteralCharacterUTF8");
        handler_token_kind(is_utf16, "LiteralCharacterUTF16");
        handler_token_kind(is_utf32, "LiteralCharacterUTF32");
        handler_token_kind("LiteralCharacter");
        sm_case_flagged(is_char, true, "\'", emit);
    }
    sm_case("\\", string_esc);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LexError");
        sm_case_peek("\n", emit);
    }
    sm_case_flagged(is_char, true, "\"", string);
    sm_case_flagged(is_char, false, "\'", string);
    sm_fallback(string);
    
    ////
    
    select_state(string_esc);
    sm_case("'\"?\\abfnrtv", string);
    sm_case("01234567", string_esc_oct2);
    sm_case("x", string_esc_hex);
    sm_case("u", string_esc_universal_4);
    sm_case("U", string_esc_universal_8);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LexError");
        sm_case_peek("\n", emit);
    }
    sm_fallback(string);
    
    ////
    
    select_state(string_esc_oct2);
    sm_case("01234567", string_esc_oct1);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_oct1);
    sm_case("01234567", string);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_hex);
    sm_case("0123456789abcdefABCDEF", string_esc_hex);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_8);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_7);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_7);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_6);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_6);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_5);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_5);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_4);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_4);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_3);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_3);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_2);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_2);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_1);
    sm_fallback_peek(string);
    
    ////
    
    select_state(string_esc_universal_1);
    sm_case("0123456789abcdefABCDEF", string);
    sm_fallback_peek(string);
    
    ////
    
    select_state(raw_string);
    sm_delim_mark_first();
    sm_fallback_peek(raw_string_get_delim);
    
    ////
    
    select_state(raw_string_get_delim);
    sm_case_peek("(", raw_string_finish_delim);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LexError");
        sm_case(" \\)", emit);
    }
    sm_fallback(raw_string_get_delim);
    
    ////
    
    select_state(raw_string_finish_delim);
    sm_delim_mark_one_past_last();
    sm_fallback(raw_string_find_close);
    
    ////
    
    select_state(raw_string_find_close);
    sm_case(")", raw_string_try_delim);
    sm_fallback(raw_string_find_close);
    
    ////
    
    select_state(raw_string_try_delim);
    sm_match_delim(raw_string_try_quote);
    sm_fallback_peek(raw_string_find_close);
    
    ////
    
    select_state(raw_string_try_quote);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind(is_wide, "LiteralStringWideRaw");
        handler_token_kind(is_utf8 , "LiteralStringUTF8Raw");
        handler_token_kind(is_utf16, "LiteralStringUTF16Raw");
        handler_token_kind(is_utf32, "LiteralStringUTF32Raw");
        handler_token_kind("LiteralStringRaw");
        sm_case("\"", emit);
    }
    sm_fallback_peek(raw_string_find_close);
    
    ////
    
    select_state(comment_block);
    sm_case("*", comment_block_try_close);
    sm_case("\n", comment_block_newline);
    sm_fallback(comment_block);
    
    ////
    
    select_state(comment_block_try_close);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("BlockComment");
        sm_case("/", emit);
    }
    sm_case("*", comment_block_try_close);
    sm_fallback(comment_block);
    
    ////
    
    select_state(comment_block_newline);
    sm_set_flag(is_pp_body, false);
    sm_set_flag(is_include_body, false);
    sm_fallback_peek(comment_block);
    
    ////
    
    select_state(comment_line);
    {
        Emit_Rule *emit = add_emit_rule();
        handler_token_kind("LineComment");
        sm_case_peek("\n", emit);
    }
    sm_fallback(comment_line);
    
}

// BOTTOM

