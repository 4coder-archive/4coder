/*
 * FSM table generator:
 *  Generate FSM tables as ".c" files from FSM functions.
 *
 * 23.03.2016 (dd.mm.yyyy)
 */

// TOP

// 4tech_standard_preamble.h
#if !defined(FTECH_INTEGERS)
#define FTECH_INTEGERS
#include <stdint.h>
typedef int8_t i8_4tech;
typedef int16_t i16_4tech;
typedef int32_t i32_4tech;
typedef int64_t i64_4tech;

typedef uint8_t u8_4tech;
typedef uint16_t u16_4tech;
typedef uint32_t u32_4tech;
typedef uint64_t u64_4tech;

typedef float f32_4tech;
typedef double f64_4tech;

typedef int8_t b8_4tech;
typedef int32_t b32_4tech;
#endif

#if !defined(Assert)
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif

#if !defined(API_EXPORT)
# define API_EXPORT
#endif
// standard preamble end 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef int32_t bool32;

#define ArrayCount(a) (sizeof(a)/sizeof(*a))

#define LEXER_TABLE_FILE "4cpp/4cpp_lexer_tables.c"

#include "../4cpp/4cpp_lexer_types.h"
#include "../4ed_mem_ansi.c"

typedef struct Whitespace_FSM{
    unsigned char pp_state;
    unsigned char white_done;
} Whitespace_FSM;

Whitespace_FSM
whitespace_skip_fsm(Whitespace_FSM wfsm, char c){
    if (wfsm.pp_state != LSPP_default){
        if (c == '\n') wfsm.pp_state = LSPP_default;
    }
    if (!(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v')){
        wfsm.white_done = 1;
    }
    return(wfsm);
}

#define FSM_SIG(n) Cpp_Lex_FSM n(Cpp_Lex_FSM fsm, char c, b32_4tech get_flags)
typedef FSM_SIG(FSM_Function);

FSM_SIG(int_fsm){
    switch (fsm.state){
        case LSINT_default:
        {
            switch (c){
                case 'u': case 'U': fsm.state = LSINT_u; break;
                case 'l': fsm.state = LSINT_l; break;
                case 'L': fsm.state = LSINT_L; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_u:
        {
            switch (c){
                case 'l': fsm.state = LSINT_ul; break;
                case 'L': fsm.state = LSINT_uL; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_l:
        {
            switch (c){
                case 'l': fsm.state = LSINT_ll; break;
                case 'U': case 'u': fsm.state = LSINT_extra; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_L:
        {
            switch (c){
                case 'L': fsm.state = LSINT_ll; break;
                case 'U': case 'u': fsm.state = LSINT_extra; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_ul:
        {
            switch (c){
                case 'l': fsm.state = LSINT_extra; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_uL:
        {
            switch (c){
                case 'L': fsm.state = LSINT_extra; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_ll:
        {
            switch (c){
                case 'u': case 'U': fsm.state = LSINT_extra; break;
                default: fsm.emit_token = true; break;
            }
        }break;
        
        case LSINT_extra:
        fsm.emit_token = true;
        break;
    }
    return(fsm);
}

FSM_SIG(normal_str_fsm){
    
    if (!get_flags){
        switch (fsm.state){
            case LSSTR_default:
            case LSSTR_multiline:
            {
                switch (c){
                    case '\n': case 0: fsm.state = LSSTR_error; fsm.emit_token = true; break;
                    case '\\': fsm.state = LSSTR_escape; break;
                    case '"': fsm.emit_token = true; break;
                    default: break;
                }
            }break;
            
            case LSSTR_escape:
            {
                switch (c){
                    case '\n': fsm.state = LSSTR_multiline; break;
                    default: fsm.state = LSSTR_default; break;
                }
            }break;
        }
    }
    else{
        switch (fsm.state){
            case LSSTR_multiline:
            {
                fsm.flags = 1;
            }break;
        }
    }
    
    return(fsm);
}

FSM_SIG(normal_char_fsm){
    
    if (!get_flags){
        switch (fsm.state){
            case LSSTR_default:
            case LSSTR_multiline:
            {
                switch (c){
                    case '\n': case 0: fsm.state = LSSTR_error; fsm.emit_token = true; break;
                    case '\\': fsm.state = LSSTR_escape; break;
                    case '\'': fsm.emit_token = true; break;
                    default: break;
                }
            }break;
            
            case LSSTR_escape:
            {
                switch (c){
                    case '\n': fsm.state = LSSTR_multiline; break;
                    default: fsm.state = LSSTR_default; break;
                }
            }break;
        }
    }
    else{
        switch (fsm.state){
            case LSSTR_multiline:
            {
                fsm.flags = 1;
            }break;
        }
    }
    
    return(fsm);
}

FSM_SIG(raw_str_fsm){
    
    if (!get_flags){
        switch (fsm.state){
            case LSSTR_default:
            {
                switch (c){
                    case ')': case '\\': case ' ': case '\n': fsm.emit_token = true; break;
                    case '(': fsm.state = LSSTR_get_delim; fsm.emit_token = true; break;
                    default: break;
                }
            }break;
            
            case LSSTR_get_delim:
            case LSSTR_multiline:
            {
                switch (c){
                    case '\n': fsm.state = LSSTR_multiline; break;
                    case 0: case '"': fsm.state = LSSTR_check_delim; fsm.emit_token = true; break;
                    default: break;
                }
            }break;
        }
    }
    else{
        switch (fsm.state){
            case LSSTR_multiline:
            {
                fsm.flags = 1;
            }break;
        }
    }
    
    return(fsm);
}

FSM_SIG(include_str_fsm){
    switch (fsm.state){
        case LSSTR_default:
        {
            switch (c){
                case '\n': case 0: fsm.state = LSSTR_error; fsm.emit_token = true; break;
                case '>': fsm.emit_token = true; break;
                default: break;
            }
        }break;
    }
    return(fsm);
}

b32_4tech
is_identifier_char(u8_4tech c, b32_4tech ignore_string_delims){
    b32_4tech result = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c >= 128 || (ignore_string_delims && (c == '\'' || c == '"'));
    return(result);
}

b32_4tech
is_identifier_char_restricted(u8_4tech c, b32_4tech ignore_string_delims){
    b32_4tech result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c >= 128 || (ignore_string_delims && (c == '\'' || c == '"'));
    return(result);
}

b32_4tech
is_identifier_char_non_numeric(u8_4tech c, b32_4tech ignore_string_delims){
    b32_4tech result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c >= 128 || (ignore_string_delims && (c == '\'' || c == '"'));
    return(result);
}

Cpp_Lex_FSM
main_fsm(Cpp_Lex_FSM fsm, uint8_t pp_state, uint8_t c, bool32 ignore_string_delims){
    if (c == 0){
        fsm.emit_token = true;
    }
    else{
        switch (pp_state){
            case LSPP_error:
            {
                fsm.state = LS_error_message;
                if (c == '\n'){
                    fsm.emit_token = true;
                }
            }break;
            
            default:
            switch (fsm.state){
                case LS_default:
                if (c == 'R'){
                    fsm.state = LS_string_R;
                }
                else if (c == 'U' || c == 'L'){
                    fsm.state = LS_string_LUu8;
                }
                else if (c == 'u'){
                    fsm.state = LS_string_u;
                }
                else if (is_identifier_char_non_numeric(c, ignore_string_delims)){
                    fsm.state = LS_identifier;
                }
                else if (c >= '1' && c <= '9'){
                    fsm.state = LS_number;
                }
                else if (c == '0'){
                    fsm.state = LS_number0;
                }
                else{
                    switch (c){
                        case '\'':
                        {
                            if (ignore_string_delims){
                                fsm.state = LS_identifier;
                            }
                            else{
                                fsm.state = LS_char;
                                fsm.emit_token = true;
                            }
                        }break;
                        
                        case '"':
                        {
                            if (ignore_string_delims){
                                fsm.state = LS_identifier;
                            }
                            else{
                                fsm.state = LS_string_normal;
                                fsm.emit_token = true;
                            }
                        }break;
                        
                        case '/': fsm.state = LS_comment_pre; break;
                        
                        case '.': fsm.state = LS_dot; break;
                        
                        case '<':
                        if (pp_state == LSPP_include && !ignore_string_delims){
                            fsm.state = LS_string_include;
                            fsm.emit_token = true;
                        }
                        else{
                            fsm.state = LS_less;
                        }
                        break;
                        
                        case '>': fsm.state = LS_more; break;
                        
                        case '-': fsm.state = LS_minus; break;
                        
                        case '&': fsm.state = LS_and; break;
                        case '|': fsm.state = LS_or; break;
                        
                        case '+': fsm.state = LS_plus; break;
                        
                        case ':': fsm.state = LS_colon; break;
                        
                        case '*': case '%': case '^': case '=':
                        case '!': fsm.state = LS_single_op; break;
                        
                        case '#':
                        if (pp_state == LSPP_default){
                            fsm.state = LS_pp;
                        }
                        else{
                            fsm.state = LS_pound;
                        }
                        break;
                        
#define OperCase(op,type) case op: fsm.emit_token = true; break;
                        OperCase('{', CPP_TOKEN_BRACE_OPEN);
                        OperCase('}', CPP_TOKEN_BRACE_CLOSE);
                        
                        OperCase('[', CPP_TOKEN_BRACKET_OPEN);
                        OperCase(']', CPP_TOKEN_BRACKET_CLOSE);
                        
                        OperCase('(', CPP_TOKEN_PARENTHESE_OPEN);
                        OperCase(')', CPP_TOKEN_PARENTHESE_CLOSE);
                        
                        OperCase('~', CPP_TOKEN_TILDE);
                        OperCase(',', CPP_TOKEN_COMMA);
                        OperCase(';', CPP_TOKEN_SEMICOLON);
                        OperCase('?', CPP_TOKEN_TERNARY_QMARK);
                        
                        OperCase('@', CPP_TOKEN_JUNK);
                        OperCase('\\', CPP_TOKEN_JUNK);
#undef OperCase
                    }
                }
                break;
                
                case LS_identifier:
                {
                    b32_4tech is_ident = is_identifier_char(c, ignore_string_delims);
                    if (!is_ident){
                        fsm.emit_token = true;
                    }
                }
                break;
                
                case LS_pound:
                {
                    fsm.emit_token = true;
                }break;
                
                case LS_pp:
                {
                    if (c == ' ' || c == '\r' || c == '\v' || c == '\f'){
                        // NOTE(allen): do nothing
                    }
                    else if (is_identifier_char_restricted(c, ignore_string_delims)){
                        fsm.state = LS_ppdef;
                    }
                    else{
                        fsm.emit_token = true;
                    }
                }break;
                
                case LS_ppdef:
                {
                    int is_ident = is_identifier_char_restricted(c, ignore_string_delims);
                    if (!is_ident){
                        fsm.emit_token = true;
                    }
                }break;
                
                case LS_string_R:
                {
                    if (ignore_string_delims){
                        fsm.state = LS_count;
                        fsm.emit_token = true;
                    }
                    else{
                        switch (c){
                            case '"': fsm.state = LS_string_raw; fsm.emit_token = true; break;
                            default:
                            {
                                fsm.state = LS_identifier;
                                b32_4tech is_ident = is_identifier_char(c, ignore_string_delims);
                                if (!is_ident){
                                    fsm.emit_token = true;
                                }
                            }break;
                        }
                    }
                }break;
                
                case LS_string_LUu8:
                {
                    if (ignore_string_delims){
                        fsm.state = LS_count;
                        fsm.emit_token = true;
                    }
                    else{
                        switch (c){
                            case '"':  fsm.state = LS_string_normal; fsm.emit_token = true; break;
                            case '\'': fsm.state = LS_char; fsm.emit_token = true; break;
                            case 'R':  fsm.state = LS_string_R; break;
                            default:
                            {
                                fsm.state = LS_identifier;
                                b32_4tech is_ident = is_identifier_char(c, ignore_string_delims);
                                if (!is_ident){
                                    fsm.emit_token = true;
                                }
                            }break;
                        }
                    }
                }break;
                
                case LS_string_u:
                {
                    if (ignore_string_delims){
                        fsm.state = LS_count;
                        fsm.emit_token = true;
                    }
                    else{
                        switch (c){
                            case '"':  fsm.state = LS_string_normal; fsm.emit_token = true; break;
                            case '\'': fsm.state = LS_char; fsm.emit_token = true; break;
                            case '8':  fsm.state = LS_string_LUu8; break;
                            case 'R':  fsm.state = LS_string_R; break;
                            default:
                            {
                                fsm.state = LS_identifier;
                                b32_4tech is_ident = is_identifier_char(c, ignore_string_delims);
                                if (!is_ident){
                                    fsm.emit_token = true;
                                }
                            }break;
                        }
                    }
                }break;
                
                case LS_number:
                if (c >= '0' && c <= '9'){
                    fsm.state = LS_number;
                }
                else{
                    switch (c){
                        case '.': fsm.state = LS_float; break;
                        default: fsm.emit_token = true; break;
                    }
                }
                break;
                
                case LS_number0:
                if (c >= '0' && c <= '9'){
                    fsm.state = LS_number;
                }
                else if (c == 'x'){
                    fsm.state = LS_hex;
                }
                else if (c == '.'){
                    fsm.state = LS_float;
                }
                else{
                    fsm.emit_token = true;
                }
                break;
                
                case LS_float:
                if (!(c >= '0' && c <= '9')){
                    switch (c){
                        case 'e': fsm.state = LS_crazy_float0; break;
                        default: fsm.emit_token = true; break;
                    }
                }
                break;
                
                case LS_crazy_float0:
                {
                    if ((c >= '0' && c <= '9') || c == '-'){
                        fsm.state = LS_crazy_float1;
                    }
                    else{
                        fsm.emit_token = true;
                    }
                }
                break;
                
                case LS_crazy_float1:
                {
                    if (!(c >= '0' && c <= '9')){
                        fsm.emit_token = true;
                    }
                }
                break;
                
                case LS_hex:
                {
                    int is_hex = c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F' || c >= 128;
                    if (!is_hex){
                        fsm.emit_token = true;
                    }
                }break;
                
                case LS_dot:
                {
                    if (c >= '0' && c <= '9'){
                        fsm.state = LS_float;
                    }
                    else{
                        switch (c){
                            case '.': fsm.state = LS_ellipsis; break;
                            case '*': fsm.emit_token = true; break;
                            default: fsm.emit_token = true; break;
                        }
                    }
                }break;
                
                case LS_ellipsis: fsm.emit_token = true; break;
                
                case LS_less:
                {
                    switch (c){
                        case '<': fsm.state = LS_less_less; break;
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_less_less:
                {
                    switch (c){
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_more:
                {
                    switch (c){
                        case '>': fsm.state = LS_more_more; break;
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_more_more:
                {
                    switch (c){
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_comment_pre:
                {
                    switch (c){
                        case '/': fsm.state = LS_comment; break;
                        case '*': fsm.state = LS_comment_block; break;
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_comment:
                {
                    switch (c){
                        case '\\': fsm.state = LS_comment_slashed; break;
                        case '\n': fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_comment_slashed:
                {
                    switch (c){
                        case '\r': case '\f': case '\v': break;
                        default: fsm.state = LS_comment; break;
                    }
                }break;
                
                case LS_comment_block:
                {
                    switch (c){
                        case '*': fsm.state = LS_comment_block_ending; break;
                    }
                }break;
                
                case LS_comment_block_ending:
                {
                    switch (c){
                        case '*': fsm.state = LS_comment_block_ending; break;
                        case '/': fsm.emit_token = true; break;
                        default: fsm.state = LS_comment_block; break;
                    }
                }break;
                
                case LS_minus:
                {
                    switch (c){
                        case '>': fsm.state = LS_arrow; break;
                        case '-': fsm.emit_token = true; break;
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_arrow:
                {
                    switch (c){
                        case '*': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_and:
                switch (c){
                    case '&': fsm.emit_token = true; break;
                    case '=': fsm.emit_token = true; break;
                    default: fsm.emit_token = true; break;
                }
                break;
                
                case LS_or:
                {
                    switch (c){
                        case '|': fsm.emit_token = true; break;
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_plus:
                {
                    switch (c){
                        case '+': fsm.emit_token = true; break;
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_colon:
                {
                    switch (c){
                        case ':': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
                
                case LS_single_op:
                {
                    switch (c){
                        case '=': fsm.emit_token = true; break;
                        default: fsm.emit_token = true; break;
                    }
                }break;
            }
            break;
        }
    }
    return(fsm);
}

static void
begin_table(FILE *file, char *type, char *group_name, char *table_name){
    fprintf(file, "%s %s_%s[] = {\n", type, group_name, table_name);
}

static void
begin_table(FILE *file, char *type, char *table_name){
    fprintf(file, "%s %s[] = {\n", type, table_name);
}

static void
begin_ptr_table(FILE *file, char *type, char *table_name){
    fprintf(file, "%s * %s[] = {\n", type, table_name);
}

static void
do_table_item(FILE *file, uint16_t item){
    fprintf(file, "%3d,", (int32_t)item);
}

static void
do_table_item_binary(FILE *file, uint16_t item){
    if (item == 0){
        fprintf(file, "0x00,");
    }
    else{
        fprintf(file, "%#04x,", item);
    }
}

static void
do_table_item_direct(FILE *file, char *item, char *tail){
    fprintf(file, "%s%s,", item, tail);
}

static void
end_row(FILE *file){
    fprintf(file, "\n");
}

static void
end_table(FILE *file){
    fprintf(file, "};\n\n");
}

typedef struct FSM_Tables{
    u8_4tech *full_transition_table;
    u8_4tech *marks;
    u8_4tech *eq_class;
    u8_4tech *eq_class_rep;
    u8_4tech *reduced_transition_table;
    u8_4tech *flags;
    
    u8_4tech eq_class_counter;
    u16_4tech state_count;
} FSM_Tables;

static void
allocate_full_tables(FSM_Tables *table, uint8_t state_count){
    table->full_transition_table = (uint8_t*)malloc(state_count * 256);
    table->marks = (uint8_t*)malloc(state_count * 256);
    table->eq_class = (uint8_t*)malloc(state_count * 256);
    table->eq_class_rep = (uint8_t*)malloc(state_count * 256);
    table->state_count = state_count;
    block_zero(table->marks, 256);
}

static void
do_table_reduction(FSM_Tables *table, uint16_t state_count){
    {
        table->eq_class_counter = 0;
        uint8_t *c_line = table->full_transition_table;
        for (uint16_t c = 0; c < 256; ++c){
            if (table->marks[c] == 0){
                table->eq_class[c] = table->eq_class_counter;
                table->eq_class_rep[table->eq_class_counter] = (uint8_t)c;
                uint8_t *c2_line = c_line + state_count;
                for (uint16_t c2 = c + 1; c2 < 256; ++c2){
                    if (block_compare(c_line, c2_line, state_count) == 0){
                        table->marks[c2] = 1;
                        table->eq_class[c2] = table->eq_class_counter;
                    }
                    c2_line += state_count;
                }
                ++table->eq_class_counter;
            }
            c_line += state_count;
        }
    }
    
    table->reduced_transition_table = (uint8_t*)malloc(state_count * table->eq_class_counter);
    {
        uint8_t *r_line = table->reduced_transition_table;
        for (uint16_t eq = 0; eq < table->eq_class_counter; ++eq){
            uint8_t *u_line = table->full_transition_table + state_count * table->eq_class_rep[eq];
            block_copy(r_line, u_line, state_count);
            r_line += state_count;
        }
    }
}

static FSM_Tables
generate_whitespace_skip_table(){
    uint8_t state_count = LSPP_count;
    FSM_Tables table = {0};
    allocate_full_tables(&table, state_count);
    
    int32_t i = 0;
    Whitespace_FSM wfsm = {0};
    Whitespace_FSM new_wfsm;
    for (uint16_t c = 0; c < 256; ++c){
        for (uint8_t state = 0; state < state_count; ++state){
            wfsm.pp_state = state;
            wfsm.white_done = 0;
            new_wfsm = whitespace_skip_fsm(wfsm, (uint8_t)c);
            table.full_transition_table[i++] = new_wfsm.pp_state + state_count*new_wfsm.white_done;
        }
    }
    
    do_table_reduction(&table, state_count);
    
    return(table);
}

static FSM_Tables
generate_table(u8_4tech state_count, FSM_Function *fsm_call){
    FSM_Tables table = {0};
    allocate_full_tables(&table, state_count);
    
    i32_4tech i = 0;
    Cpp_Lex_FSM fsm = {0};
    Cpp_Lex_FSM new_fsm = {0};
    for (uint16_t c = 0; c < 256; ++c){
        for (u8_4tech state = 0; state < state_count; ++state){
            fsm.state = state;
            fsm.emit_token = false;
            new_fsm = fsm_call(fsm, (u8_4tech)c, false);
            table.full_transition_table[i++] = new_fsm.state + state_count*new_fsm.emit_token;
        }
    }
    
    for (u8_4tech state = 0; state < state_count; ++state){
        fsm.state = state;
        fsm.emit_token = false;
        fsm.flags = 0;
        new_fsm = fsm_call(fsm, 0, true);
        if (new_fsm.flags != 0){
            if (table.flags == 0){
                table.flags = (u8_4tech*)malloc(state_count);
                memset(table.flags, 0, state_count);
            }
            table.flags[state] = new_fsm.flags;
        }
    }
    
    do_table_reduction(&table, state_count);
    
    return(table);
}

static FSM_Tables
generate_fsm_table(uint8_t pp_state, bool32 ignore_string_delims){
    uint8_t state_count = LS_count;
    FSM_Tables table = {0};
    allocate_full_tables(&table, state_count);
    
    int32_t i = 0;
    Cpp_Lex_FSM fsm = {0};
    Cpp_Lex_FSM new_fsm = {0};
    for (uint16_t c = 0; c < 256; ++c){
        for (uint8_t state = 0; state < state_count; ++state){
            fsm.state = state;
            fsm.emit_token = false;
            new_fsm = main_fsm(fsm, pp_state, (uint8_t)c, ignore_string_delims);
            table.full_transition_table[i++] = new_fsm.state + state_count*new_fsm.emit_token;
        }
    }
    
    do_table_reduction(&table, state_count);
    
    return(table);
}

static void
render_fsm_table(FILE *file, FSM_Tables tables, char *group_name){
    begin_table(file, "u16_4tech", group_name, "eq_classes");
    for (u16_4tech c = 0; c < 256; ++c){
        if ((c % 16) == 0 && c > 0){
            end_row(file);
        }
        do_table_item(file, tables.eq_class[c]*tables.state_count);
    }
    end_row(file);
    end_table(file);
    
    fprintf(file, "const i32_4tech num_%s_eq_classes = %d;\n\n", group_name, tables.eq_class_counter);
    
    i32_4tech i = 0;
    begin_table(file, "u8_4tech", group_name, "table");
    for (u16_4tech c = 0; c < tables.eq_class_counter; ++c){
        for (u8_4tech state = 0; state < tables.state_count; ++state){
            do_table_item(file, tables.reduced_transition_table[i++]);
        }
        end_row(file);
    }
    end_table(file);
    
    if (tables.flags != 0){
        begin_table(file, "u8_4tech", group_name, "flags");
        for (u8_4tech state = 0; state < tables.state_count; ++state){
            if ((state % 4) == 0 && state > 0){
                end_row(file);
            }
            do_table_item_binary(file, tables.flags[state]);
        }
        end_row(file);
        end_table(file);
    }
}

static void
render_variable(FILE *file, char *type, char *variable, uint32_t x){
    fprintf(file, "%s %s = %d;\n\n", type, variable, x);
}

static void
render_comment(FILE *file, char *comment){
    fprintf(file, "/*\n%s*/\n", comment);
}

typedef struct PP_Names{
    uint8_t pp_state;
    char *name;
    bool32 ignore_string_delims;
}  PP_Names;

static PP_Names pp_names[] = {
    {LSPP_default,          "main_fsm",          false},
    {LSPP_include,          "pp_include_fsm",    false},
    {LSPP_macro_identifier, "pp_macro_fsm",      false},
    {LSPP_identifier,       "pp_identifier_fsm", false},
    {LSPP_body_if,          "pp_body_if_fsm",    false},
    {LSPP_body,             "pp_body_fsm",       false},
    {LSPP_number,           "pp_number_fsm",     false},
    {LSPP_error,            "pp_error_fsm",      false},
    {LSPP_junk,             "pp_junk_fsm",       false},
    {LSPP_default,          "no_string_fsm",     true },
};

int
main(){
    FILE *file = fopen(LEXER_TABLE_FILE, "wb");
    
    FSM_Tables wtables = generate_whitespace_skip_table();
    render_fsm_table(file, wtables, "whitespace_fsm");
    
    FSM_Tables itables = generate_table(LSINT_count, int_fsm);
    render_fsm_table(file, itables, "int_fsm");
    
    {
        struct{
            char *name;
            uint8_t count;
            FSM_Function *fsm_call;
        } static tables[] = {
            {"raw_str",     LSSTR_count,         raw_str_fsm     },
            {"normal_str",  LSSTR_count,         normal_str_fsm  },
            {"include_str", LSSTR_include_count, include_str_fsm },
            {"normal_char", LSSTR_count,         normal_char_fsm }
        };
        
        for (u32_4tech i = 0; i < ArrayCount(tables); ++i){
            FSM_Tables str_tables = generate_table(tables[i].count, tables[i].fsm_call);
            render_fsm_table(file, str_tables, tables[i].name);
        }
    }
    
    for (int32_t i = 0; i < ArrayCount(pp_names); ++i){
        FSM_Tables tables = generate_fsm_table(pp_names[i].pp_state, pp_names[i].ignore_string_delims);
        render_fsm_table(file, tables, pp_names[i].name);
    }
    
    begin_ptr_table(file, "uint16_t", "get_eq_classes");
    for (int32_t i = 0; i < ArrayCount(pp_names); ++i){
        do_table_item_direct(file, pp_names[i].name, "_eq_classes");
        end_row(file);
    }
    end_table(file);
    
    begin_ptr_table(file, "uint8_t", "get_table");
    for (int32_t i = 0; i < ArrayCount(pp_names); ++i){
        do_table_item_direct(file, pp_names[i].name, "_table");
        end_row(file);
    }
    end_table(file);
    
    fclose(file);
    return(0);
}

// BOTTOM


