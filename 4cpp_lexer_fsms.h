/*
 * FSMs for 4cpp lexer
 * 
 * 23.03.2016 (dd.mm.yyyy)
 *
 */

// TOP

#if !defined(FCPP_LEXER_FSMS_H)
#define FCPP_LEXER_FSMS_H

enum Lex_State{
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

struct Lex_FSM{
    uint8_t state;
    uint8_t int_state;
    uint8_t emit_token;
    uint8_t multi_line;
};
static Lex_FSM null_lex_fsm = {0};

#endif

// BOTTOM


