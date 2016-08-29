/*
 * FSM table generator:
 *  Generate FSM tables as ".c" files from FSM functions.
 *
 * 23.03.2016 (dd.mm.yyyy)
 */

// TOP

/* TODO(allen):

1. Eliminate the complicated preprocessor directive parsing tables
2. Establish a clean systematic way of maintaining whatever is left

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define ArrayCount(a) (sizeof(a)/sizeof(*a))

#include "4cpp_lexer_types.h"
#include "4cpp_lexer_fsms.h"

struct String_And_Flag{
    char *str;
    uint32_t flags;
};

static String_And_Flag preprop_strings[] = {
	{"include", CPP_PP_INCLUDE},
	{"INCLUDE", CPP_PP_INCLUDE},
	{"ifndef", CPP_PP_IFNDEF},
	{"IFNDEF", CPP_PP_IFNDEF},
	{"define", CPP_PP_DEFINE},
	{"DEFINE", CPP_PP_DEFINE},
	{"import", CPP_PP_IMPORT},
	{"IMPORT", CPP_PP_IMPORT},
	{"pragma", CPP_PP_PRAGMA},
	{"PRAGMA", CPP_PP_PRAGMA},
	{"undef", CPP_PP_UNDEF},
	{"UNDEF", CPP_PP_UNDEF},
	{"endif", CPP_PP_ENDIF},
	{"ENDIF", CPP_PP_ENDIF},
	{"error", CPP_PP_ERROR},
	{"ERROR", CPP_PP_ERROR},
	{"ifdef", CPP_PP_IFDEF},
	{"IFDEF", CPP_PP_IFDEF},
	{"using", CPP_PP_USING},
	{"USING", CPP_PP_USING},
	{"else", CPP_PP_ELSE},
	{"ELSE", CPP_PP_ELSE},
	{"elif", CPP_PP_ELIF},
	{"ELIF", CPP_PP_ELIF},
	{"line", CPP_PP_LINE},
	{"LINE", CPP_PP_LINE},
	{"if", CPP_PP_IF},
    {"IF", CPP_PP_IF},
};
static String_And_Flag keyword_strings[] = {
    {"true", CPP_TOKEN_BOOLEAN_CONSTANT},
    {"false", CPP_TOKEN_BOOLEAN_CONSTANT},
    
    {"and", CPP_TOKEN_AND},
    {"and_eq", CPP_TOKEN_ANDEQ},
    {"bitand", CPP_TOKEN_BIT_AND},
    {"bitor", CPP_TOKEN_BIT_OR},
    {"or", CPP_TOKEN_OR},
    {"or_eq", CPP_TOKEN_OREQ},
    {"sizeof", CPP_TOKEN_SIZEOF},
    {"alignof", CPP_TOKEN_ALIGNOF},
    {"decltype", CPP_TOKEN_DECLTYPE},
    {"throw", CPP_TOKEN_THROW},
    {"new", CPP_TOKEN_NEW},
    {"delete", CPP_TOKEN_DELETE},
    {"xor", CPP_TOKEN_BIT_XOR},
    {"xor_eq", CPP_TOKEN_XOREQ},
    {"not", CPP_TOKEN_NOT},
    {"not_eq", CPP_TOKEN_NOTEQ},
    {"typeid", CPP_TOKEN_TYPEID},
    {"compl", CPP_TOKEN_BIT_NOT},
    
    {"void", CPP_TOKEN_KEY_TYPE},
    {"bool", CPP_TOKEN_KEY_TYPE},
    {"char", CPP_TOKEN_KEY_TYPE},
    {"int", CPP_TOKEN_KEY_TYPE},
    {"float", CPP_TOKEN_KEY_TYPE},
    {"double", CPP_TOKEN_KEY_TYPE},
    
    {"long", CPP_TOKEN_KEY_MODIFIER},
    {"short", CPP_TOKEN_KEY_MODIFIER},
    {"unsigned", CPP_TOKEN_KEY_MODIFIER},
    
    {"const", CPP_TOKEN_KEY_QUALIFIER},
    {"volatile", CPP_TOKEN_KEY_QUALIFIER},
    
    {"asm", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"break", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"case", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"catch", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"continue", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"default", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"do", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"else", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"for", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"goto", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"if", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"return", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"switch", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"try", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"while", CPP_TOKEN_KEY_CONTROL_FLOW},
    {"static_assert", CPP_TOKEN_KEY_CONTROL_FLOW},
    
    {"const_cast", CPP_TOKEN_KEY_CAST},
    {"dynamic_cast", CPP_TOKEN_KEY_CAST},
    {"reinterpret_cast", CPP_TOKEN_KEY_CAST},
    {"static_cast", CPP_TOKEN_KEY_CAST},
    
    {"class", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"enum", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"struct", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"typedef", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"union", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"template", CPP_TOKEN_KEY_TYPE_DECLARATION},
    {"typename", CPP_TOKEN_KEY_TYPE_DECLARATION},
    
    {"friend", CPP_TOKEN_KEY_ACCESS},
    {"namespace", CPP_TOKEN_KEY_ACCESS},
    {"private", CPP_TOKEN_KEY_ACCESS},
    {"protected", CPP_TOKEN_KEY_ACCESS},
    {"public", CPP_TOKEN_KEY_ACCESS},
    {"using", CPP_TOKEN_KEY_ACCESS},
    
    {"extern", CPP_TOKEN_KEY_LINKAGE},
    {"export", CPP_TOKEN_KEY_LINKAGE},
    {"inline", CPP_TOKEN_KEY_LINKAGE},
    {"static", CPP_TOKEN_KEY_LINKAGE},
    {"virtual", CPP_TOKEN_KEY_LINKAGE},
    
    {"alignas", CPP_TOKEN_KEY_OTHER},
    {"explicit", CPP_TOKEN_KEY_OTHER},
    {"noexcept", CPP_TOKEN_KEY_OTHER},
    {"nullptr", CPP_TOKEN_KEY_OTHER},
    {"operator", CPP_TOKEN_KEY_OTHER},
    {"register", CPP_TOKEN_KEY_OTHER},
    {"this", CPP_TOKEN_KEY_OTHER},
    {"thread_local", CPP_TOKEN_KEY_OTHER},
};

typedef struct FSM_State{
    uint32_t transition_rule[256];
    uint8_t override;
} FSM_State;

typedef struct FSM{
    FSM_State *states;
    unsigned short count, max;
    
    FSM_State *term_states;
    unsigned short term_count, term_max;
    
    unsigned char terminal_base;
    
    char *comment;
} FSM;

typedef struct FSM_Stack{
    FSM *fsms;
    int32_t count, max;
    
    uint8_t table_transition_state;
    uint8_t final_state;
} FSM_Stack;

typedef struct Match_Node{
    Match_Node *first_child;
    Match_Node *next_sibling;
    
    int32_t *words;
    int32_t count, max;
    int32_t index;
    
    FSM_State *state;
} Match_Node;

typedef struct Match_Tree{
    Match_Node *nodes;
    int32_t count, max;
} Match_Tree;

typedef struct Match_Tree_Stack{
    Match_Tree *trees;
    int32_t count, max;
} Match_Tree_Stack;

typedef struct Future_FSM{
    Match_Node *source;
} Future_FSM;

typedef struct Future_FSM_Stack{
    Future_FSM *futures;
    int32_t count, max;
} Future_FSM_Stack;

static FSM*
get_fsm(FSM_Stack *stack){
    FSM* result = 0;
    assert(stack->count < stack->max);
    result = &stack->fsms[stack->count];
    ++stack->count;
    return(result);
}

static Match_Tree*
get_tree(Match_Tree_Stack *stack){
    Match_Tree* result = 0;
    assert(stack->count < stack->max);
    result = &stack->trees[stack->count++];
    return(result);
}

static FSM
fsm_init(uint16_t max, uint8_t terminal_base){
    FSM fsm;
    int32_t memsize;
    fsm.max = max;
    fsm.count = 0;
    memsize = sizeof(FSM_State)*fsm.max;
    fsm.states = (FSM_State*)malloc(memsize);
    
    fsm.term_max = max;
    fsm.term_count = 0;
    memsize = sizeof(FSM_State)*fsm.term_max;
    fsm.term_states = (FSM_State*)malloc(memsize);
    
    fsm.comment = 0;
    fsm.terminal_base = terminal_base;
    return(fsm);
}

static void
fsm_add_comment(FSM *fsm, char *str){
    int32_t comment_len;
    int32_t str_len;
    char *new_comment;
    
    str_len = (int32_t)strlen(str);
    
    if (fsm->comment != 0){
        comment_len = (int32_t)strlen(fsm->comment);
        new_comment = (char*)malloc(str_len + comment_len + 1);
        
        memcpy(new_comment, fsm->comment, comment_len);
        memcpy(new_comment + comment_len, str, str_len);
        new_comment[comment_len + str_len] = 0;
        
        free(fsm->comment);
        fsm->comment = new_comment;
    }
    else{
        fsm->comment = (char*)malloc(str_len + 1);
        memcpy(fsm->comment, str, str_len);
        fsm->comment[str_len] = 0;
	}
}

static Match_Tree
tree_init(uint16_t max){
    Match_Tree tree;
    int32_t memsize;
    tree.max = max;
    tree.count = 0;
    memsize = sizeof(Match_Node)*tree.max;
    tree.nodes = (Match_Node*)malloc(memsize);
    return(tree);
}

static uint8_t
push_future_fsm(Future_FSM_Stack *stack, Match_Node *node){
    uint8_t index = 0;
    Future_FSM *future = 0;
    assert(stack->count < stack->max);
    assert(stack->max < 256);
    index = (uint8_t)(stack->count++);
    future = &stack->futures[index];
    future->source = node;
    return(index);
}

static Match_Node*
match_get_node(Match_Tree *tree){
    Match_Node *result;
    assert(tree->count < tree->max);
    result = &tree->nodes[tree->count++];
    return(result);
}

static void
match_init_node(Match_Node *node, int32_t match_count){
    *node = {};
    node->words = (int32_t*)malloc(sizeof(int32_t)*match_count);
    node->max = match_count;
}

static void
match_copy_init_node(Match_Node *node, Match_Node *source){
    *node = {};
    node->max = source->count;
    node->count = source->count;
    node->words = (int32_t*)malloc(sizeof(int32_t)*source->count);
    node->index = source->index;
    memcpy(node->words, source->words, sizeof(int32_t)*source->count);
}

static void
match_add_word(Match_Node *node, int32_t word){
    assert(node->count < node->max);
    node->words[node->count++] = word;
}

static FSM_State*
fsm_get_state(FSM *fsm, uint32_t terminal_base){
    FSM_State *result;
    uint16_t i;
    assert(fsm->count < fsm->max);
    result = &fsm->states[fsm->count++];
    for (i = 0; i < 256; ++i){
        result->transition_rule[i] = terminal_base;
	}
    result->override = 0;
    return(result);
}

static FSM_State*
fsm_get_state(FSM *fsm){
    FSM_State *result = fsm_get_state(fsm, fsm->terminal_base);
    return(result);
}

static FSM_State*
fsm_get_term_state(FSM *fsm, unsigned char override){
    FSM_State *result;
    assert(fsm->term_count < fsm->term_max);
    result = &fsm->term_states[fsm->term_count++];
    result->override = override;
    return(result);
}

static uint8_t
fsm_index(FSM *fsm, FSM_State *s){
    uint8_t result;
    result = (uint8_t)(uint64_t)(s - fsm->states);
    if (s->override){
        result = fsm->terminal_base + s->override;
	}
    return(result);
}

static void
fsm_add_transition(FSM_State *state, char c, unsigned char dest){
    state->transition_rule[c] = dest;
}

struct Terminal_Lookup_Table{
    uint32_t state_to_type[60];
    uint8_t type_to_state[CPP_TOKEN_TYPE_COUNT];
    uint8_t state_count;
};

static void
process_match_node(String_And_Flag *input, Match_Node *node, Match_Tree *tree, FSM *fsm){
    int32_t next_index = node->index + 1;
    int32_t match_count = node->count;
    FSM_State *this_state = node->state;
    uint8_t terminal_base = fsm->terminal_base;
    
    int32_t i, j, *words = node->words;
    
    String_And_Flag saf;
    int32_t l;
    
    char c;
    Match_Node *next_nodes[256];
    Match_Node *newest_child = 0;
    Match_Node *n;
    
    uint8_t unjunkify = 0;
    
    memset(next_nodes, 0, sizeof(next_nodes));
    
    for (i = 0; i < match_count; ++i){
        j = words[i];
        saf = input[j];
        l = (int32_t)strlen(saf.str);
        
        if (next_index < l){
            c = saf.str[next_index];
            
            if (next_nodes[c] == 0){
                next_nodes[c] = match_get_node(tree);
                match_init_node(next_nodes[c], match_count);
                
                next_nodes[c]->index = next_index;
                next_nodes[c]->state = fsm_get_state(fsm);
                
                if (newest_child == 0){
                    assert(node->first_child == 0);
                    node->first_child = next_nodes[c];
                }
                else{
                    assert(newest_child->next_sibling == 0);
                    newest_child->next_sibling = next_nodes[c];
                }
                newest_child = next_nodes[c];
            }
            
            match_add_word(next_nodes[c], j);
            fsm_add_transition(this_state, c, fsm_index(fsm, next_nodes[c]->state));
        }
        else if (next_index == l){
            assert(unjunkify == 0);
            unjunkify = (unsigned char)saf.flags;
        }
    }
    
    if (unjunkify){
        for (i = 0; i < 256; ++i){
            if (this_state->transition_rule[i] == terminal_base){
                this_state->transition_rule[i] = terminal_base + unjunkify;
            }
        }
    }
    
    for (n = node->first_child; n; n = n->next_sibling){
        process_match_node(input, n, tree, fsm);
    }
}

static FSM
generate_pp_directive_fsm(){
    Match_Tree tree;
    FSM fsm;
    Match_Node *root_node;
    FSM_State *root_state;
    int32_t i;
    
    fsm = fsm_init(200, 200);
    tree = tree_init(200);
    
    root_state = fsm_get_state(&fsm);
    
    root_node = match_get_node(&tree);
    match_init_node(root_node, ArrayCount(preprop_strings));
    for (i = 0; i < ArrayCount(preprop_strings); ++i){
        root_node->words[i] = i;
	}
    root_node->count = ArrayCount(preprop_strings);
    root_node->state = root_state;
    root_node->index = -1;
    process_match_node(preprop_strings, root_node, &tree, &fsm);
    
    root_state->transition_rule[' '] = 0;
    root_state->transition_rule['\t'] = 0;
    root_state->transition_rule['\r'] = 0;
    root_state->transition_rule['\v'] = 0;
    root_state->transition_rule['\f'] = 0;
    
    return(fsm);
}

/*

Each state needs a full set of transition rules.  Most transitions should go into a
"not-a-keyword-state".  The exceptions are:
1. When we see an alphanumeric character that is the next character of an actual keyword
 i. May need to transition to a new table at this point.
2. When we have just seen an entire valid keyword, and the next thing we see is not alphanumeric.

*/

#define RealTerminateBase 65536

static int32_t
char_is_alphanumeric(char x){
    int32_t result = 0;
    if ((x >= '0' && x <= '9') ||
        (x >= 'A' && x <= 'Z') ||
        (x >= 'a' && x <= 'z') ||
        x == '_'){
        result = 1;
    }
    return(result);
}

static void
process_match_node(String_And_Flag *input, Match_Node *node, Match_Tree *tree, FSM *fsm,
                   Terminal_Lookup_Table *terminal_table, int32_t levels_to_go,
                   Future_FSM_Stack *unfinished_fsms){
    
    int32_t next_index = node->index + 1;
    int32_t match_count = node->count;
    int32_t *words = node->words;
    FSM_State *this_state = node->state;
    
    int32_t word_index = 0;
    int32_t good_transition = 0;
    int32_t len = 0;
    int32_t i = 0;
    
    String_And_Flag saf = {0};
    
    Match_Node *next_nodes[256];
    Match_Node *newest_child = 0;
    Match_Node *n = 0;
    char c = 0;
    
    uint8_t override = 0;
    
    memset(next_nodes, 0, sizeof(next_nodes));
    
    for (i = 0; i < match_count; ++i){
        word_index = words[i];
        saf = input[word_index];

        len = (int32_t)strlen(saf.str);
        if (next_index < len){
            c = saf.str[next_index];
            
            if (next_nodes[c] == 0){
                next_nodes[c] = match_get_node(tree);
                match_init_node(next_nodes[c], match_count);
                
                next_nodes[c]->index = next_index;
                
                if (levels_to_go == 1){
                    override = push_future_fsm(unfinished_fsms, next_nodes[c]);
                    next_nodes[c]->state = fsm_get_term_state(fsm, override);
                }
                else{
                    next_nodes[c]->state = fsm_get_state(fsm, RealTerminateBase);
                }
                
                if (newest_child == 0){
                    assert(node->first_child == 0);
                    node->first_child = next_nodes[c];
                }
                else{
                    assert(newest_child->next_sibling == 0);
                    newest_child->next_sibling = next_nodes[c];
                }
                newest_child = next_nodes[c];
            }
            
            match_add_word(next_nodes[c], word_index);
            fsm_add_transition(this_state, c, fsm_index(fsm, next_nodes[c]->state));
        }
        else{
            assert(next_index == len);
            assert(good_transition == 0);
            good_transition = terminal_table->type_to_state[saf.flags] + RealTerminateBase;
        }
    }
    
    if (good_transition){
        for (i = 0; i < 256; ++i){
            if (!char_is_alphanumeric((char)i)){
                this_state->transition_rule[i] = good_transition;
            }
        }
    }
    
    if (levels_to_go != 1){
        for (n = node->first_child; n; n = n->next_sibling){
            process_match_node(input, n, tree, fsm, terminal_table, levels_to_go-1, unfinished_fsms);
        }
    }
}

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

Lex_FSM
int_fsm(Lex_FSM fsm, char c){
    switch (fsm.int_state){
        case LSINT_default:
        switch (c){
            case 'u': case 'U': fsm.int_state = LSINT_u; break;
            case 'l': fsm.int_state = LSINT_l; break;
            case 'L': fsm.int_state = LSINT_L; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_u:
        switch (c){
            case 'l': fsm.int_state = LSINT_ul; break;
            case 'L': fsm.int_state = LSINT_uL; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_l:
        switch (c){
            case 'l': fsm.int_state = LSINT_ll; break;
            case 'U': case 'u': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_L:
        switch (c){
            case 'L': fsm.int_state = LSINT_ll; break;
            case 'U': case 'u': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_ul:
        switch (c){
            case 'l': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_uL:
        switch (c){
            case 'L': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_ll:
        switch (c){
            case 'u': case 'U': fsm.int_state = LSINT_extra; break;
            default: fsm.emit_token = 1; break;
        }
        break;
        
        case LSINT_extra:
        fsm.emit_token = 1;
        break;
    }
    return(fsm);
}

Lex_FSM
main_fsm(Lex_FSM fsm, unsigned char pp_state, unsigned char c){
    if (c == 0) fsm.emit_token = 1;
    else
        switch (pp_state){
        case LSPP_error:
        fsm.state = LS_error_message;
        if (c == '\n') fsm.emit_token = 1;
        break;
        
        case LSPP_include:
        switch (fsm.state){
            case LSINC_default:
            switch (c){
                case '"': fsm.state = LSINC_quotes; break;
                case '<': fsm.state = LSINC_pointy; break;
                default: fsm.state = LSINC_junk; break;
            }
            break;
            
            case LSINC_quotes:
            if (c == '"') fsm.emit_token = 1;
            else if (c == '\n'){
                fsm.emit_token = 1;
                fsm.state = LSINC_junk;
            }
            break;
            
            case LSINC_pointy:
            if (c == '>') fsm.emit_token = 1;
            else if (c == '\n'){
                fsm.emit_token = 1;
                fsm.state = LSINC_junk;
            }
            break;
            
            case LSINC_junk:
            if (c == '\n') fsm.emit_token = 1;
            break;
        }
        break;
        
        default:
        switch (fsm.state){
            case LS_default:
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'){
                fsm.state = LS_identifier;
            }
            else if (c >= '1' && c <= '9'){
                fsm.state = LS_number;
            }
            else if (c == '0'){
                fsm.state = LS_number0;
            }
            else switch (c){
                case '\'': fsm.state = LS_char; break;
                case '"': fsm.state = LS_string; break;
                
                case '/': fsm.state = LS_comment_pre; break;
                
                case '.': fsm.state = LS_dot; break;
                
                case '<': fsm.state = LS_less; break;
                case '>': fsm.state = LS_more; break;
                
                case '-': fsm.state = LS_minus; break;
                
                case '&': fsm.state = LS_and; break;
                case '|': fsm.state = LS_or; break;
                
                case '+': fsm.state = LS_plus; break;
                
                case ':': fsm.state = LS_colon; break;
                
                case '*': fsm.state = LS_star; break;
                
                case '%': fsm.state = LS_modulo; break;
                case '^': fsm.state = LS_caret; break;
                
                case '=': fsm.state = LS_eq; break;
                case '!': fsm.state = LS_bang; break;
                
                case '#':
                if (pp_state == LSPP_default){
                    fsm.state = LS_pp;
                    fsm.emit_token = 1;
                }
                else{
                    fsm.state = LS_pound;
                }
                break;
                
#define OperCase(op,type) case op: fsm.emit_token = 1; break;
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
                OperCase('$', CPP_TOKEN_JUNK);
                OperCase('\\', CPP_TOKEN_JUNK);
#undef OperCase
            }
            break;
            
            case LS_identifier:
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')){
                fsm.emit_token = 1;
            }
            break;
            
            case LS_pound:
            switch (c){
                case '#': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_pp:break;
            
            case LS_char:
            case LS_char_multiline:
            switch(c){
                case '\n': case '\'': fsm.emit_token = 1; break;
                case '\\': fsm.state = LS_char_slashed; break;
            }
            break;
            
            case LS_char_slashed:
            switch (c){
                case '\r': case '\f': case '\v': break;
                case '\n': fsm.state = LS_char_multiline; break;
                default: fsm.state = LS_char; break;
            }
            break;
            
            case LS_string:
            case LS_string_multiline:
            switch(c){
                case '\n': case '\"': fsm.emit_token = 1; break;
                case '\\': fsm.state = LS_string_slashed; break;
            } 
            break;
            
            case LS_string_slashed:
            switch (c){
                case '\r': case '\f': case '\v': break;
                case '\n': fsm.state = LS_string_multiline; break;
                default: fsm.state = LS_string; break;
            }
            break;
            
            case LS_number:
            if (c >= '0' && c <= '9'){
                fsm.state = LS_number;
            }
            else{
                switch (c){
                    case '.': fsm.state = LS_float; break;
                    default: fsm.emit_token = 1; break;
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
                fsm.emit_token = 1;
            }
            break;
            
            case LS_float:
            if (!(c >= '0' && c <= '9')){
                switch (c){
                    case 'e': fsm.state = LS_crazy_float0; break;
                    default: fsm.emit_token = 1; break;
                }
            }
            break;
            
            case LS_crazy_float0:
            {
                if ((c >= '0' && c <= '9') || c == '-'){
                    fsm.state = LS_crazy_float1;
                }
                else{
                    fsm.emit_token = 1;
                }
            }
            break;
            
            case LS_crazy_float1:
            {
                if (!(c >= '0' && c <= '9')){
                    fsm.emit_token = 1;
                }
            }
            break;
            
            case LS_hex:
            if (!(c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F')){
                fsm.emit_token = 1;
            }
            break;
            
            case LS_dot:
            if (c >= '0' && c <= '9'){
                fsm.state = LS_float;
            }
            else
                switch (c){
                case '.': fsm.state = LS_ellipsis; break;
                case '*': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_ellipsis: fsm.emit_token = 1; break;
            
            case LS_less:
            switch (c){
                case '<': fsm.state = LS_less_less; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_less_less:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_more:
            switch (c){
                case '>': fsm.state = LS_more_more; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_more_more:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_comment_pre:
            switch (c){
                case '/': fsm.state = LS_comment; break;
                case '*': fsm.state = LS_comment_block; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_comment:
            switch (c){
                case '\\': fsm.state = LS_comment_slashed; break;
                case '\n': fsm.emit_token = 1; break;
            }
            break;
            
            case LS_comment_slashed:
            switch (c){
                case '\r': case '\f': case '\v': break;
                default: fsm.state = LS_comment; break;
            }
            break;
            
            case LS_comment_block:
            switch (c){
                case '*': fsm.state = LS_comment_block_ending; break;
            }
            break;
            
            case LS_comment_block_ending:
            switch (c){
                case '*': fsm.state = LS_comment_block_ending; break;
                case '/': fsm.emit_token = 1; break;
                default: fsm.state = LS_comment_block; break;
            }
            break;
            
            case LS_minus:
            switch (c){
                case '>': fsm.state = LS_arrow; break;
                case '-': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_arrow:
            switch (c){
                case '*': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_and:
            switch (c){
                case '&': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_or:
            switch (c){
                case '|': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_plus:
            switch (c){
                case '+': fsm.emit_token = 1; break;
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_colon:
            switch (c){
                case ':': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_star:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_modulo:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_caret:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_eq:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
            
            case LS_bang:
            switch (c){
                case '=': fsm.emit_token = 1; break;
                default: fsm.emit_token = 1; break;
            }
            break;
        }
        break;
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
    fprintf(file, "%2d,", (int32_t)item);
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
    uint8_t *full_transition_table;
    uint8_t *marks;
    uint8_t *eq_class;
    uint8_t *eq_class_rep;
    uint8_t *reduced_transition_table;
    
    uint8_t eq_class_counter;
    uint16_t state_count;
} FSM_Tables;

static void
allocate_full_tables(FSM_Tables *table, uint8_t state_count){
    table->full_transition_table = (uint8_t*)malloc(state_count * 256);
    table->marks = (uint8_t*)malloc(state_count * 256);
    table->eq_class = (uint8_t*)malloc(state_count * 256);
    table->eq_class_rep = (uint8_t*)malloc(state_count * 256);
    table->state_count = state_count;
    memset(table->marks, 0, 256);
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
                    if (memcmp(c_line, c2_line, state_count) == 0){
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
            memcpy(r_line, u_line, state_count);
            r_line += state_count;
        }
    }
}

static FSM_Tables
generate_whitespace_skip_table(){
    uint8_t state_count = LSPP_count;
    FSM_Tables table;
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
generate_int_table(){
    uint8_t state_count = LSINT_count;
    FSM_Tables table;
    allocate_full_tables(&table, state_count);
    
    int32_t i = 0;
    Lex_FSM fsm = {0};
    Lex_FSM new_fsm;
    for (uint16_t c = 0; c < 256; ++c){
        for (uint8_t state = 0; state < state_count; ++state){
            fsm.int_state = state;
            fsm.emit_token = 0;
            new_fsm = int_fsm(fsm, (uint8_t)c);
            table.full_transition_table[i++] = new_fsm.int_state + state_count*new_fsm.emit_token;
        }
    }
    
    do_table_reduction(&table, state_count);
    
    return(table);
}

static FSM_Tables
generate_fsm_table(uint8_t pp_state){
    uint8_t state_count = LS_count;
    FSM_Tables table;
    allocate_full_tables(&table, state_count);
    
    int32_t i = 0;
    Lex_FSM fsm = {0};
    Lex_FSM new_fsm;
    for (uint16_t c = 0; c < 256; ++c){
        for (uint8_t state = 0; state < state_count; ++state){
            fsm.state = state;
            fsm.emit_token = 0;
            new_fsm = main_fsm(fsm, pp_state, (uint8_t)c);
            table.full_transition_table[i++] = new_fsm.state + state_count*new_fsm.emit_token;
        }
    }
    
    do_table_reduction(&table, state_count);
    
    return(table);
}

static void
render_fsm_table(FILE *file, FSM_Tables tables, char *group_name){
    begin_table(file, "uint16_t", group_name, "eq_classes");
    for (uint16_t c = 0; c < 256; ++c){
        do_table_item(file, tables.eq_class[c]*tables.state_count);
    }
    end_row(file);
    end_table(file);
    
    fprintf(file, "const int32_t num_%s_eq_classes = %d;\n\n", group_name, tables.eq_class_counter);
    
    int32_t i = 0;
    begin_table(file, "uint8_t", group_name, "table");
    for (uint16_t c = 0; c < tables.eq_class_counter; ++c){
        for (uint8_t state = 0; state < tables.state_count; ++state){
            do_table_item(file, tables.reduced_transition_table[i++]);
        }
        end_row(file);
    }
    end_table(file);
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
}  PP_Names;

static PP_Names pp_names[] = {
    {LSPP_default, "main_fsm"},
    {LSPP_include, "pp_include_fsm"},
    {LSPP_macro_identifier, "pp_macro_fsm"},
    {LSPP_identifier, "pp_identifier_fsm"},
    {LSPP_body_if, "pp_body_if_fsm"},
    {LSPP_body, "pp_body_fsm"},
    {LSPP_number, "pp_number_fsm"},
    {LSPP_error, "pp_error_fsm"},
    {LSPP_junk, "pp_junk_fsm"},
};

static FSM_Tables
generate_table_from_abstract_fsm(FSM fsm, uint8_t real_term_base){
    uint8_t  state_count = (uint8_t )fsm.count;
    FSM_Tables table;

    allocate_full_tables(&table, state_count);
    
    int32_t i = 0;
    uint32_t new_state;
    for (uint16_t c = 0; c < 256; ++c){
        for (uint8_t state = 0; state < state_count; ++state){
            new_state = fsm.states[state].transition_rule[c];
            if (new_state >= RealTerminateBase){
                new_state = new_state - RealTerminateBase + real_term_base;
            }
            table.full_transition_table[i++] = (uint8_t)new_state;
        }
    }
    
    do_table_reduction(&table, state_count);
    
    return(table);
}

int
main(){
    FILE *file;
    file = fopen("4cpp_lexer_tables.c", "wb");
    
    FSM_Tables wtables = generate_whitespace_skip_table();
    render_fsm_table(file, wtables, "whitespace_fsm");
    
    FSM_Tables itables = generate_int_table();
    render_fsm_table(file, itables, "int_fsm");
    
    begin_table(file, "uint8_t", "multiline_state_table");
    for (uint8_t state = 0; state < LS_count; ++state){
        do_table_item(file, (state == LS_string_multiline || state == LS_char_multiline));
    }
    end_row(file);
    end_table(file);
    
    for (int32_t i = 0; i < ArrayCount(pp_names); ++i){
        assert(i == pp_names[i].pp_state);
        FSM_Tables tables = generate_fsm_table(pp_names[i].pp_state);
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
    
    FSM pp_directive_fsm = generate_pp_directive_fsm();
    FSM_Tables pp_directive_tables = generate_table_from_abstract_fsm(pp_directive_fsm, 0);
    
    render_fsm_table(file, pp_directive_tables, "pp_directive");
    render_variable(file, "uint8_t", "LSDIR_default", 0);
    render_variable(file, "uint8_t", "LSDIR_count", pp_directive_fsm.count);
    render_variable(file, "uint8_t", "pp_directive_terminal_base", pp_directive_fsm.terminal_base);
    
    fclose(file);
    return(0);
}

// BOTTOM


