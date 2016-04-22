/*
 * FSMs for 4c++ lexer
 * 
 * 23.03.2016 (dd.mm.yyyy)
 */

// TOP

struct String_And_Flag{
	char *str;
	unsigned int flags;
};

enum Lex_State{
    LS_default,
    LS_identifier,
    LS_pound,
    LS_pp,
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

enum Lex_Int_State{
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

enum Lex_INC_State{
    LSINC_default,
    LSINC_quotes,
    LSINC_pointy,
    LSINC_junk,
};

enum Lex_PP_State{
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

struct Whitespace_FSM{
    unsigned char pp_state;
    unsigned char white_done;
};

struct Lex_FSM{
    unsigned char state;
    union{
        unsigned char int_state;
        unsigned char directive_state;
        unsigned char sub_machine;
    };
    unsigned char emit_token;
    unsigned char multi_line;
};

// BOTTOM


