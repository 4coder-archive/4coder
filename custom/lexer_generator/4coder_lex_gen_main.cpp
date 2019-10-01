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
#include "pcg_basic.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_hash_functions.cpp"
#include "4coder_table.cpp"
#include "pcg_basic.c"

#define LANG_NAME_LOWER_STR stringify(LANG_NAME_LOWER)
#define LANG_NAME_CAMEL_STR stringify(LANG_NAME_CAMEL)

////////////////////////////////

// NOTE(allen): PRIMARY MODEL

struct Token_Kind_Node{
    Token_Kind_Node *next;
    b32 optimized_in;
    String_Const_u8 name;
    Token_Base_Kind base_kind;
};

struct Token_Kind_Set{
    Token_Kind_Node *first;
    Token_Kind_Node *last;
    i32 count;
    Table_Data_u64 name_to_ptr;
};

struct Keyword{
    Keyword *next;
    String_Const_u8 name;
    String_Const_u8 lexeme;
};

struct Keyword_Set{
    Keyword_Set *next;
    Keyword *first;
    Keyword *last;
    i32 count;
    b32 has_fallback_token_kind;
    String_Const_u8 fallback_name;
    Table_Data_u64 name_to_ptr;
    Table_Data_u64 lexeme_to_ptr;
    String_Const_u8 pretty_name;
};

struct Keyword_Set_List{
    Keyword_Set *first;
    Keyword_Set *last;
    i32 count;
};

struct Keyword_Layout{
    u64 seed;
    u64 error_score;
    u64 max_single_error_score;
    f32 iterations_per_lookup;
    u64 *hashes;
    u64 *contributed_error;
    Keyword **slots;
    i32 slot_count;
};

typedef i32 Flag_Reset_Rule;
enum{
    FlagResetRule_AutoZero,
    FlagResetRule_KeepState,
    FlagResetRule_COUNT,
};

struct Flag{
    Flag *next;
    Flag_Reset_Rule reset_rule;
    Token_Base_Flag emit_flags;
    u16 emit_sub_flags;
    
    b32 optimized_in;
    String_Const_u8 base_name;
    i32 number;
    i32 index;
    u32 value;
};

struct Flag_Set{
    Flag *first;
    Flag *last;
    i32 count;
};

typedef i32 Emit_Handler_Kind;
enum{
    EmitHandlerKind_Direct,
    EmitHandlerKind_Keywords,
    EmitHandlerKind_KeywordsDelim,
};

struct Emit_Handler{
    Emit_Handler *next;
    Emit_Handler_Kind kind;
    Flag *flag_check;
    union{
        String_Const_u8 token_name;
        Keyword_Set *keywords;
    };
};

struct Emit_Check{
    Emit_Check *next;
    String_Const_u8 emit_check;
    Flag *flag;
    b32 value;
};

struct Emit_Check_List{
    Emit_Check *first;
    Emit_Check *last;
    i32 count;
};

struct Emit_Rule{
    Emit_Check_List emit_checks;
    Emit_Handler *first;
    Emit_Handler *last;
    i32 count;
};

typedef i32 Action_Kind;
enum{
    ActionKind_SetFlag,
    ActionKind_ZeroFlags,
    ActionKind_DelimMarkFirst,
    ActionKind_DelimMarkOnePastLast,
    ActionKind_Consume,
    ActionKind_Emit,
};

struct Action{
    Action *next;
    Action *prev;
    Action_Kind kind;
    union{
        struct{
            Flag *flag;
            b32 value;
        } set_flag;
        Emit_Rule *emit_rule;
    };
};

struct Action_List{
    Action *first;
    Action *last;
    i32 count;
};

typedef i32 Action_Context;
enum{
    ActionContext_Normal,
    ActionContext_EndOfFile,
};

typedef i32 Transition_Consume_Rule;
enum{
    Transition_Consume,
    Transition_NoConsume,
};

global u16 smi_eof = 256;

struct Field_Pin{
    Field_Pin *next;
    
    // This represents the set of flags with the particular /flag/ set to /flag/
    // exactly half of all flag state possibilities.
    Flag *flag;
    b32 value;
};

struct Field_Pin_List{
    Field_Pin_List *next;
    
    // This set is the intersection of the set represented by each pin.
    // A list with nothing in it is _always_ the "full set".
    Field_Pin *first;
    Field_Pin *last;
    i32 count;
};

struct Field_Set{
    // This set is the union of the set represented by each list.
    Field_Pin_List *first;
    Field_Pin_List *last;
    i32 count;
};

struct Input_Set{
    u16 *inputs;
    i32 count;
};

struct Condition_Node{
    Condition_Node *next;
    Field_Set fields;
    Input_Set inputs;
};

struct Condition_Set{
    Condition_Node *first;
    Condition_Node *last;
    i32 count;
};

typedef i32 Transition_Case_Kind;
enum{
    TransitionCaseKind_NONE,
    
    // intermediates only
    TransitionCaseKind_CharaterArray,
    TransitionCaseKind_EOF,
    TransitionCaseKind_Fallback,
    
    // actually stored in Transition_Case "kind" field
    TransitionCaseKind_DelimMatch,
    TransitionCaseKind_DelimMatchFail,
    TransitionCaseKind_ConditionSet,
};

struct Transition_Case{
    Transition_Case_Kind kind;
    union{
        Condition_Set condition_set;
    };
};

struct Transition{
    Transition *next;
    Transition *prev;
    struct State *parent_state;
    Transition_Case condition;
    Action_List activation_actions;
    struct State *dst_state;
};

struct Transition_List{
    Transition *first;
    Transition *last;
    i32 count;
};

struct Transition_Ptr_Node{
    Transition_Ptr_Node *next;
    Transition *ptr;
};

struct Transition_Ptr_Set{
    Transition_Ptr_Node *first;
    Transition_Ptr_Node *last;
    i32 count;
};

struct State{
    State *next;
    Transition_List transitions;
    String_Const_u8 pretty_name;
    
    b32 optimized_in;
    i32 number;
    Transition_Ptr_Set back_references;
    
    Action_List on_entry_actions;
};

struct State_Set{
    State *first;
    State *last;
    i32 count;
};

struct Lexer_Model{
    State *root;
    Flag_Set flags;
    State_Set states;
};

struct Lexer_Primary_Context{
    Base_Allocator *allocator;
    Arena arena;
    Token_Kind_Set tokens;
    Keyword_Set_List keywords;
    Lexer_Model model;
};

////////////////////////////////

struct Flag_Ptr_Node{
    Flag_Ptr_Node *next;
    Flag *flag;
};

struct Flag_Bucket{
    String_Const_u8 pretty_name;
    Flag_Ptr_Node *first;
    Flag_Ptr_Node *last;
    i32 max_bits;
    i32 count;
    
    i32 number_of_variables;
};

typedef i32 Flag_Bind_Property;
enum{
    FlagBindProperty_Free,
    FlagBindProperty_Bound,
    FlagBindProperty_COUNT,
};

struct Flag_Bucket_Set{
    Flag_Bucket buckets[FlagBindProperty_COUNT][FlagResetRule_COUNT];
};

struct Partial_Transition{
    Partial_Transition *next;
    Field_Set fields;
    Action_List actions;
    State *dst_state;
};

struct Partial_Transition_List{
    Partial_Transition *first;
    Partial_Transition *last;
    i32 count;
};

struct Grouped_Input_Handler{
    Grouped_Input_Handler *next;
    
    u8 inputs[256];
    i32 input_count;
    b8 inputs_used[256];
    
    Partial_Transition_List partial_transitions;
};

struct Grouped_Input_Handler_List{
    Grouped_Input_Handler *first;
    Grouped_Input_Handler *last;
    i32 count;
    
    Grouped_Input_Handler *group_with_biggest_input_set;
};

////////////////////////////////

// NOTE(allen): MODELING SYNTAX HELPERS

struct Operator{
    Operator *next;
    String_Const_u8 name;
    String_Const_u8 op;
};

struct Operator_Set{
    Operator *first;
    Operator *last;
    i32 count;
    Table_Data_u64 lexeme_to_ptr;
};

struct Lexer_Helper_Context{
    Lexer_Primary_Context primary_ctx;
    Table_u64_Data char_to_name;
    Token_Base_Kind selected_base_kind;
    State *selected_state;
    Operator_Set *selected_op_set;
    Keyword_Set *selected_key_set;
    Emit_Rule *selected_emit_rule;
    Transition *selected_transition;
    
    // convenience pointer to primary's arena.
    Arena *arena;
};

struct Character_Set{
    Table_u64_u64 table;
};

#include "4coder_lex_gen_hand_written.h"
#include "4coder_lex_gen_hand_written.cpp"

////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////

// NOTE(allen): INTERNAL CONSTRUCTORS

internal void
smi_primary_init(Base_Allocator *allocator, Lexer_Primary_Context *ctx){
    ctx->allocator = allocator;
    ctx->arena = make_arena(allocator);
    ctx->model.root = 0;
    ctx->tokens.name_to_ptr = make_table_Data_u64(allocator, 400);
}

internal b32
smi_try_add_token(Lexer_Primary_Context *ctx, String_Const_u8 name, Token_Base_Kind base_kind){
    b32 result = false;
    Token_Kind_Set *set = &ctx->tokens;
    Table_Lookup lookup = table_lookup(&set->name_to_ptr, make_data(name.str, name.size));
    if (!lookup.found_match){
        Token_Kind_Node *node = push_array_zero(&ctx->arena, Token_Kind_Node, 1);
        node->name = push_string_copy(&ctx->arena, name);
        node->base_kind = base_kind;
        table_insert(&set->name_to_ptr, make_data(node->name.str, node->name.size), (u64)PtrAsInt(node));
        sll_queue_push(set->first, set->last, node);
        set->count += 1;
        result = true;
    }
    return(result);
}

internal b32
smi_key(Lexer_Primary_Context *ctx, Keyword_Set *set, String_Const_u8 name, String_Const_u8 lexeme, Token_Base_Kind base_kind){
    b32 result = false;
    Table_Lookup lookup = table_lookup(&set->name_to_ptr, make_data(name.str, name.size));
    if (!lookup.found_match){
        lookup = table_lookup(&set->lexeme_to_ptr, make_data(lexeme.str, lexeme.size));
        if (!lookup.found_match){
            if (smi_try_add_token(ctx, name, base_kind)){
                Keyword *key = push_array_zero(&ctx->arena, Keyword, 1);
                key->name = push_string_copy(&ctx->arena, name);
                key->lexeme = push_string_copy(&ctx->arena, lexeme);
                table_insert(&set->name_to_ptr, make_data(key->name.str, key->name.size), (u64)PtrAsInt(key));
                table_insert(&set->lexeme_to_ptr, make_data(key->lexeme.str, key->lexeme.size), (u64)PtrAsInt(key));
                sll_queue_push(set->first, set->last, key);
                set->count += 1;
                result = true;
            }
        }
    }
    return(result);
}

internal b32
smi_key_fallback(Lexer_Primary_Context *ctx, Keyword_Set *set, String_Const_u8 name, Token_Base_Kind base_kind){
    b32 result = false;
    if (!set->has_fallback_token_kind){
        if (smi_try_add_token(ctx, name, base_kind)){
            set->has_fallback_token_kind = true;
            set->fallback_name = push_string_copy(&ctx->arena, name);
            result = true;
        }
    }
    return(result);
}

internal State*
smi_add_state(Lexer_Primary_Context *ctx, String_Const_u8 pretty_name){
    State_Set *set = &ctx->model.states;
    State *state = push_array_zero(&ctx->arena, State, 1);
    sll_queue_push(set->first, set->last, state);
    set->count += 1;
    state->pretty_name = push_string_copy(&ctx->arena, pretty_name);
    return(state);
}

internal Flag*
smi_add_flag(Lexer_Primary_Context *ctx, Flag_Reset_Rule rule){
    Flag_Set *set = &ctx->model.flags;
    Flag *flag = push_array_zero(&ctx->arena, Flag, 1);
    flag->reset_rule = rule;
    sll_queue_push(set->first, set->last, flag);
    set->count += 1;
    return(flag);
}

internal Emit_Rule*
smi_emit_rule(Arena *arena){
    return(push_array_zero(arena, Emit_Rule, 1));
}

internal Emit_Handler*
smi_emit_handler__inner(Arena *arena, Emit_Rule *rule, Emit_Handler_Kind kind, Flag *flag_check){
    Emit_Handler *handler = push_array_zero(arena, Emit_Handler, 1);
    handler->kind = kind;
    handler->flag_check = flag_check;
    if (rule != 0){
        sll_queue_push(rule->first, rule->last, handler);
        rule->count += 1;
    }
    return(handler);
}

internal Emit_Handler*
smi_emit_handler(Arena *arena, Emit_Rule *rule, String_Const_u8 name, Flag *flag_check){
    Emit_Handler *handler = smi_emit_handler__inner(arena, rule, EmitHandlerKind_Direct, flag_check);
    handler->token_name = name;
    return(handler);
}

internal Emit_Handler*
smi_emit_handler(Arena *arena, Emit_Rule *rule, Keyword_Set *set, Flag *flag_check){
    Emit_Handler *handler = smi_emit_handler__inner(arena, rule, EmitHandlerKind_Keywords, flag_check);
    handler->keywords = set;
    return(handler);
}

internal Emit_Handler*
smi_emit_handler_delim(Arena *arena, Emit_Rule *rule, Keyword_Set *set, Flag *flag_check){
    Emit_Handler *handler = smi_emit_handler__inner(arena, rule, EmitHandlerKind_KeywordsDelim, flag_check);
    handler->keywords = set;
    return(handler);
}

internal void
smi_append_set_flag(Arena *arena, Action_List *list, Flag *flag, b32 value){
    Action *action = push_array_zero(arena, Action, 1);
    zdll_push_back(list->first, list->last, action);
    list->count += 1;
    action->kind = ActionKind_SetFlag;
    action->set_flag.flag = flag;
    action->set_flag.value = value;
}

internal void
smi_append_zero_flags(Arena *arena, Action_List *list){
    Action *action = push_array_zero(arena, Action, 1);
    zdll_push_back(list->first, list->last, action);
    list->count += 1;
    action->kind = ActionKind_ZeroFlags;
}

internal void
smi_append_delim_mark_first(Arena *arena, Action_List *list){
    Action *action = push_array_zero(arena, Action, 1);
    zdll_push_back(list->first, list->last, action);
    list->count += 1;
    action->kind = ActionKind_DelimMarkFirst;
}

internal void
smi_append_delim_mark_one_past_last(Arena *arena, Action_List *list){
    Action *action = push_array_zero(arena, Action, 1);
    zdll_push_back(list->first, list->last, action);
    list->count += 1;
    action->kind = ActionKind_DelimMarkOnePastLast;
}

internal void
smi_append_consume(Arena *arena, Action_List *list){
    Action *action = push_array_zero(arena, Action, 1);
    zdll_push_back(list->first, list->last, action);
    list->count += 1;
    action->kind = ActionKind_Consume;
}

internal void
smi_append_emit(Arena *arena, Action_List *list, Emit_Rule *emit){
    Action *action = push_array_zero(arena, Action, 1);
    zdll_push_back(list->first, list->last, action);
    list->count += 1;
    action->kind = ActionKind_Emit;
    action->emit_rule = emit;
}

////////////////////////////////

#if 0
internal void
CHECK_PIN_LIST(Field_Pin_List *list){
    i32 counter = 0;
    for (Field_Pin *pin = list->first;
         pin != 0;
         pin = pin->next){
        counter += 1;
    }
    Assert(counter == list->count);
}
#else
#define CHECK_PIN_LIST(x)
#endif

internal Field_Pin*
smi_field_pin_copy(Arena *arena, Field_Pin *pin){
    Field_Pin *result = push_array_zero(arena, Field_Pin, 1);
    result->flag = pin->flag;
    result->value = pin->value;
    return(result);
}

internal Field_Pin_List*
smi_field_pin_list_copy(Arena *arena, Field_Pin_List list){
    CHECK_PIN_LIST(&list);
    Field_Pin_List *new_list = push_array_zero(arena, Field_Pin_List, 1);
    new_list->count = list.count;
    for (Field_Pin *node = list.first;
         node != 0;
         node = node->next){
        Field_Pin *new_pin = smi_field_pin_copy(arena, node);
        sll_queue_push(new_list->first, new_list->last, new_pin);
    }
    CHECK_PIN_LIST(new_list);
    return(new_list);
}

internal Field_Set
smi_field_set_copy(Arena *arena, Field_Set set){
    Field_Set result = {};
    result.count = set.count;
    for (Field_Pin_List *pin_list = set.first;
         pin_list != 0;
         pin_list = pin_list->next){
        Field_Pin_List *new_list = smi_field_pin_list_copy(arena, *pin_list);
        sll_queue_push(result.first, result.last, new_list);
    }
    return(result);
}

internal void
smi_field_pin_sub__recursive(Arena *arena, Field_Pin_List a, Field_Pin_List *list, Field_Pin_List growing_list, Field_Set *result){
    if (list != 0){
        growing_list.count += 1;
        Field_Pin_List *next_list = list->next;
        for (Field_Pin *pin = list->first;
             pin != 0;
             pin = pin->next){
            Field_Pin local_pin = *pin;
            local_pin.next = 0;
            sll_queue_push(growing_list.first, growing_list.last, &local_pin);
            smi_field_pin_sub__recursive(arena, a, next_list, growing_list, result);
        }
    }
    else{
        b32 has_conflicts = false;
        Temp_Memory restore_point = begin_temp(arena);
        Field_Pin_List *new_list = smi_field_pin_list_copy(arena, a);
        for (Field_Pin *pin = growing_list.first;
             pin != 0;
             pin = pin->next){
            b32 is_duplicate = false;
            for (Field_Pin *a_pin = new_list->first;
                 a_pin != 0;
                 a_pin = a_pin->next){
                if (pin->flag == a_pin->flag){
                    if (pin->value == a_pin->value){
                        end_temp(restore_point);
                        has_conflicts = true;
                        goto double_break;
                    }
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate){
                Field_Pin *new_pin = smi_field_pin_copy(arena, pin);
                new_pin->value = !new_pin->value;
                sll_queue_push(new_list->first, new_list->last, new_pin);
                new_list->count += 1;
            }
        }
        double_break:;
        
        if (!has_conflicts){
            CHECK_PIN_LIST(new_list);
            sll_queue_push(result->first, result->last, new_list);
            result->count += 1;
        }
    }
}

internal Field_Set
smi_field_pin_sub(Arena *arena, Field_Pin_List a, Field_Set b){
    Field_Set result = {};
    Field_Pin_List *list = b.first;
    Field_Pin_List growing_list = {};
    smi_field_pin_sub__recursive(arena, a, list, growing_list, &result);
    return(result);
}

internal Field_Set
smi_field_set_subtract(Arena *arena, Field_Set a, Field_Set b){
    Field_Set result = {};
    for (Field_Pin_List *list = a.first;
         list != 0;
         list = list->next){
        Field_Set partial = smi_field_pin_sub(arena, *list, b);
        if (result.first == 0){
            result = partial;
        }
        else{
            if (partial.first != 0){
                result.last->next = partial.first;
                result.last = partial.last;
                result.count += partial.count;
            }
        }
    }
    return(result);
}

internal Field_Set
smi_field_set_intersect(Arena *arena, Field_Set a, Field_Set b){
    Field_Set result = {};
    for (Field_Pin_List *a_list = a.first;
         a_list != 0;
         a_list = a_list->next){
        for (Field_Pin_List *b_list = b.first;
             b_list != 0;
             b_list = b_list->next){
            b32 has_conflicts = false;
            Temp_Memory restore_point = begin_temp(arena);
            Field_Pin_List *new_list = smi_field_pin_list_copy(arena, *a_list);
            for (Field_Pin *b_pin = b_list->first;
                 b_pin != 0;
                 b_pin = b_pin->next){
                b32 is_duplicate = false;
                for (Field_Pin *pin = new_list->first;
                     pin != 0;
                     pin = pin->next){
                    if (pin->flag == pin->flag){
                        if (pin->value != pin->value){
                            end_temp(restore_point);
                            has_conflicts = true;
                            goto double_break;
                        }
                        is_duplicate = true;
                        break;
                    }
                }
                if (!is_duplicate){
                    Field_Pin *new_pin = smi_field_pin_copy(arena, b_pin);
                    sll_queue_push(new_list->first, new_list->last, new_pin);
                    new_list->count += 1;
                }
            }
            double_break:;
            
            if (!has_conflicts){
                sll_queue_push(result.first, result.last, new_list);
                result.count += 1;
            }
        }
    }
    return(result);
}

internal b32
smi_field_set_match(Arena *scratch, Field_Set a, Field_Set b){
    Temp_Memory temp = begin_temp(scratch);
    b32 result = false;
    Field_Set sub = smi_field_set_subtract(scratch, a, b);
    if (sub.count == 0){
        sub = smi_field_set_subtract(scratch, b, a);
        if (sub.count == 0){
            result = true;
        }
    }
    end_temp(temp);
    return(result);
}

internal Field_Set
smi_field_set_union(Arena *arena, Field_Set a, Field_Set b){
    Field_Set result = {};
    if (a.first != 0){
        if (b.first != 0){
            a = smi_field_set_copy(arena, a);
            // TODO(allen): simplify these lists by seeing if they union
            // cleanly with the lists in a!
            b = smi_field_set_copy(arena, b);
            result.first = a.first;
            a.last->next = b.first;
            result.last = b.last;
            result.count = a.count + b.count;
        }
        else{
            result = smi_field_set_copy(arena, a);
        }
    }
    else{
        if (b.first != 0){
            result = smi_field_set_copy(arena, b);
        }
    }
    return(result);
}

internal Field_Set
smi_field_set_construct(Arena *arena){
    Field_Set result = {};
    Field_Pin_List *list = push_array_zero(arena, Field_Pin_List, 1);
    sll_queue_push(result.first, result.last, list);
    result.count += 1;
    return(result);
}

internal Field_Set
smi_field_set_construct(Arena *arena, Flag *flag, b32 value){
    Field_Set result = {};
    if (flag != 0){
        Field_Pin_List *list = push_array_zero(arena, Field_Pin_List, 1);
        sll_queue_push(result.first, result.last, list);
        result.count += 1;
        Field_Pin *pin = push_array_zero(arena, Field_Pin, 1);
        sll_queue_push(list->first, list->last, pin);
        list->count += 1;
        pin->flag = flag;
        pin->value = value;
    }
    else{
        result = smi_field_set_construct(arena);
    }
    return(result);
}

internal Input_Set
smi_input_set_copy(Arena *arena, Input_Set set){
    Input_Set result = {};
    result.inputs = push_array_write(arena, u16, set.count, set.inputs);
    result.count = set.count;
    return(result);
}

internal Input_Set
smi_input_set_subtract(Arena *arena, Input_Set a, Input_Set b){
    Input_Set result = {};
    if (a.count > 0){
        Temp_Memory restore_point = begin_temp(arena);
        result = smi_input_set_copy(arena, a);
        for (i32 i = 0; i < result.count; i += 1){
            b32 is_subtracted = false;
            for (i32 j = 0; j < b.count; j += 1){
                if (result.inputs[i] == b.inputs[j]){
                    is_subtracted = true;
                    break;
                }
            }
            if (is_subtracted){
                result.count -= 1;
                result.inputs[i] = result.inputs[result.count];
                i -= 1;
            }
        }
        if (a.count == 0){
            end_temp(restore_point);
            block_zero_struct(&result);
        }
    }
    return(result);
}

internal Input_Set
smi_input_set_intersect(Arena *arena, Input_Set a, Input_Set b){
    Input_Set result = {};
    if (a.count > 0 && b.count > 0){
        Temp_Memory restore_point = begin_temp(arena);
        result = smi_input_set_copy(arena, a);
        for (i32 i = 0; i < result.count; i += 1){
            b32 is_shared = false;
            for (i32 j = 0; j < b.count; j += 1){
                if (result.inputs[i] == b.inputs[j]){
                    is_shared = true;
                    break;
                }
            }
            if (!is_shared){
                result.count -= 1;
                result.inputs[i] = result.inputs[result.count];
                i -= 1;
            }
        }
        if (result.count == 0){
            end_temp(restore_point);
            block_zero_struct(&result);
        }
    }
    return(result);
}

internal Input_Set
smi_input_set_union(Arena *arena, Input_Set a, Input_Set b){
    Input_Set result = {};
    if (a.count > 0 || b.count > 0){
        result.inputs = push_array_zero(arena, u16, a.count + b.count);
        block_copy_dynamic_array(result.inputs, a.inputs, a.count);
        result.count = a.count;
        for (i32 i = 0; i < b.count; i += 1){
            b32 is_duplicate = false;
            for (i32 j = 0; j < result.count; j += 1){
                if (result.inputs[j] == b.inputs[i]){
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate){
                result.inputs[result.count] = b.inputs[i];
                result.count += 1;
            }
        }
    }
    return(result);
}

internal Input_Set
smi_input_set_construct(Arena *arena, String_Const_u8 characters){
    Input_Set result = {};
    result.count = (i32)characters.size;
    result.inputs = push_array_zero(arena, u16, result.count);
    for (umem i = 0; i < characters.size; i += 1){
        result.inputs[i] = (u16)characters.str[i];
    }
    return(result);
}

internal Input_Set
smi_input_set_construct_eof(Arena *arena){
    Input_Set result = {};
    result.count = 1;
    result.inputs = push_array_zero(arena, u16, result.count);
    result.inputs[0] = smi_eof;
    return(result);
}

internal Input_Set
smi_input_set_construct_fallback(Arena *arena){
    Input_Set result = {};
    result.count = 257;
    result.inputs = push_array_zero(arena, u16, result.count);
    for (u16 i = 0; i < 257; i += 1){
        result.inputs[i] = i;
    }
    return(result);
}

internal Condition_Node*
smi_condition_node_copy(Arena *arena, Condition_Node *node){
    Condition_Node *result = push_array_zero(arena, Condition_Node, 1);
    result->fields = smi_field_set_copy(arena, node->fields);
    result->inputs = smi_input_set_copy(arena, node->inputs);
    return(result);
}

internal Condition_Set
smi_condition_set_copy(Arena *arena, Condition_Set set){
    Condition_Set result = {};
    for (Condition_Node *node = set.first;
         node != 0;
         node = node->next){
        Condition_Node *new_node = smi_condition_node_copy(arena, node);
        sll_queue_push(result.first, result.last, new_node);
        result.count += 1;
    }
    return(result);
}

internal Condition_Set
smi_condition_node_sub(Arena *arena, Condition_Node a, Condition_Node b){
    Condition_Set result = {};
    Input_Set a_minus_b_input = smi_input_set_subtract(arena, a.inputs, b.inputs);
    if (a_minus_b_input.count == 0){
        Field_Set a_minus_b_fields = smi_field_set_subtract(arena, a.fields, b.fields);
        if (a_minus_b_fields.count > 0){
            Condition_Node *new_node = push_array_zero(arena, Condition_Node, 1);
            new_node->inputs = smi_input_set_copy(arena, a.inputs);
            new_node->fields = a_minus_b_fields;
            sll_queue_push(result.first, result.last, new_node);
            result.count += 1;
        }
    }
    else{
        if (a_minus_b_input.count == a.inputs.count){
            Condition_Node *new_node = push_array_zero(arena, Condition_Node, 1);
            new_node->inputs = a_minus_b_input;
            new_node->fields = smi_field_set_copy(arena, a.fields);
            sll_queue_push(result.first, result.last, new_node);
            result.count += 1;
        }
        else{
            Field_Set a_minus_b_fields = smi_field_set_subtract(arena, a.fields, b.fields);
            if (a_minus_b_fields.count == 0){
                Condition_Node *new_node = push_array_zero(arena, Condition_Node, 1);
                new_node->inputs = a_minus_b_input;
                new_node->fields = smi_field_set_copy(arena, a.fields);
                sll_queue_push(result.first, result.last, new_node);
                result.count += 1;
            }
            else{
                Input_Set a_int_b_input = smi_input_set_intersect(arena, a.inputs, b.inputs);
                Condition_Node *node_1 = push_array_zero(arena, Condition_Node, 1);
                node_1->inputs = a_int_b_input;
                node_1->fields = a_minus_b_fields;
                sll_queue_push(result.first, result.last, node_1);
                Condition_Node *node_2 = push_array_zero(arena, Condition_Node, 1);
                node_2->inputs = a_minus_b_input;
                node_2->fields = smi_field_set_copy(arena, a.fields);
                sll_queue_push(result.first, result.last, node_2);
                result.count += 2;
            }
        }
    }
    return(result);
}

internal Condition_Node*
smi_condition_node_int(Arena *arena, Condition_Node a, Condition_Node b){
    Condition_Node *result = push_array_zero(arena, Condition_Node, 1);
    result->inputs = smi_input_set_intersect(arena, a.inputs, b.inputs);
    result->fields = smi_field_set_intersect(arena, a.fields, b.fields);
    return(result);
}

internal Condition_Set
smi_condition_set_subtract_node(Arena *arena, Condition_Set a, Condition_Node *b){
    Condition_Set result = {};
    for (Condition_Node *node = a.first;
         node != 0;
         node = node->next){
        Condition_Set partial = smi_condition_node_sub(arena, *node, *b);
        if (result.first == 0){
            result = partial;
        }
        else{
            if (partial.first != 0){
                result.last->next = partial.first;
                result.last = partial.last;
                result.count += partial.count;
            }
        }
    }
    return(result);
}

internal Condition_Set
smi_condition_set_subtract(Arena *arena, Condition_Set a, Condition_Set b){
    Condition_Set result = a;
    for (Condition_Node *node = b.first;
         node != 0;
         node = node->next){
        result = smi_condition_set_subtract_node(arena, result, node);
    }
    return(result);
}

internal Condition_Set
smi_condition_set_intersect(Arena *arena, Condition_Set a, Condition_Set b){
    Condition_Set result = {};
    for (Condition_Node *a_node = a.first;
         a_node != 0;
         a_node = a_node->next){
        for (Condition_Node *b_node = b.first;
             b_node != 0;
             b_node = b_node->next){
            Condition_Node *node = smi_condition_node_int(arena, *a_node, *b_node);
            if (node->inputs.count > 0 && node->fields.count > 0){
                sll_queue_push(result.first, result.last, node);
                result.count += 1;
            }
        }
    }
    return(result);
}

internal Condition_Set
smi_condition_set_union(Arena *arena, Condition_Set a, Condition_Set b){
    Condition_Set result = {};
    if (a.count != 0){
        if (b.count != 0){
            a = smi_condition_set_copy(arena, a);
            // TODO(allen): simplify these down!
            b = smi_condition_set_copy(arena, b);
            result.first = a.first;
            a.last->next = b.first;
            result.last = b.last;
            result.count = a.count + b.count;
        }
        else{
            result = smi_condition_set_copy(arena, a);
        }
    }
    else{
        if (b.count != 0){
            result = smi_condition_set_copy(arena, b);
        }
    }
    return(result);
}

internal Condition_Node*
smi_condition_node(Arena *arena, Input_Set inputs, Field_Set fields){
    Condition_Node *node = push_array_zero(arena, Condition_Node, 1);
    node->fields = fields;
    node->inputs = inputs;
    return(node);
}

internal Condition_Set
smi_condition(Arena *arena, Input_Set inputs, Field_Set fields){
    Condition_Set result = {};
    Condition_Node *node = smi_condition_node(arena, inputs, fields);
    sll_queue_push(result.first, result.last, node);
    result.count += 1;
    return(result);
}

////////////////////////////////

internal Transition*
smi_case(Lexer_Primary_Context *ctx, State *state,
         Transition_Case_Kind kind, String_Const_u8 characters, Flag *flag_check,b32 flag_check_value,
         State *dst, Transition_Consume_Rule consume_rule, Emit_Rule *emit){
    Transition *transition = push_array_zero(&ctx->arena, Transition, 1);
    transition->parent_state = state;
    
    switch (kind){
        default:
        {
            transition->condition.kind = kind;
        }break;
        
        case TransitionCaseKind_CharaterArray:
        {
            transition->condition.kind = TransitionCaseKind_ConditionSet;
            Input_Set inputs = smi_input_set_construct(&ctx->arena, characters);
            Field_Set fields = smi_field_set_construct(&ctx->arena,
                                                       flag_check, flag_check_value);
            transition->condition.condition_set = smi_condition(&ctx->arena, inputs, fields);
        }break;
        
        case TransitionCaseKind_EOF:
        {
            transition->condition.kind = TransitionCaseKind_ConditionSet;
            Input_Set inputs = smi_input_set_construct_eof(&ctx->arena);
            Field_Set fields = smi_field_set_construct(&ctx->arena,
                                                       flag_check, flag_check_value);
            transition->condition.condition_set = smi_condition(&ctx->arena, inputs, fields);
        }break;
        
        case TransitionCaseKind_Fallback:
        {
            transition->condition.kind = TransitionCaseKind_ConditionSet;
            Input_Set inputs = smi_input_set_construct_fallback(&ctx->arena);
            Field_Set fields = smi_field_set_construct(&ctx->arena,
                                                       flag_check, flag_check_value);
            transition->condition.condition_set = smi_condition(&ctx->arena, inputs, fields);
        }break;
    }
    
    transition->dst_state = dst;
    
    if (consume_rule == Transition_Consume){
        smi_append_consume(&ctx->arena, &transition->activation_actions);
    }
    
    if (emit != 0){
        smi_append_emit(&ctx->arena, &transition->activation_actions, emit);
    }
    
    zdll_push_back(state->transitions.first, state->transitions.last, transition);
    state->transitions.count += 1;
    return(transition);
}

////////////////////////////////

// NOTE(allen): CONSTRUCTORS

global Lexer_Helper_Context helper_ctx = {};

internal void
sm_helper_init(Base_Allocator *allocator){
    smi_primary_init(allocator, &helper_ctx.primary_ctx);
    helper_ctx.char_to_name = make_table_u64_Data(allocator, 100);
    
    helper_ctx.arena = &helper_ctx.primary_ctx.arena;
}

internal void
sm_char_name(u8 c, char *str){
    Table_Lookup lookup = table_lookup(&helper_ctx.char_to_name, c);
    if (lookup.found_match){
        table_erase(&helper_ctx.char_to_name, lookup);
    }
    String_Const_u8 string = push_string_copy(helper_ctx.arena, SCu8(str));
    table_insert(&helper_ctx.char_to_name, c, make_data(string.str, string.size));
}

internal void
sm_select_base_kind(Token_Base_Kind kind){
    helper_ctx.selected_base_kind = kind;
}

internal void
sm_select_state(State *state){
    helper_ctx.selected_state = state;
}

internal void
sm_select_op_set(Operator_Set *set){
    helper_ctx.selected_op_set = set;
}

internal void
sm_select_key_set(Keyword_Set *set){
    helper_ctx.selected_key_set = set;
}

internal void
sm_select_emit(Emit_Rule *emit){
    helper_ctx.selected_emit_rule = emit;
}

internal void
sm_select_transition(Transition *transition){
    helper_ctx.selected_transition = transition;
}

internal b32
sm_direct_token_kind(char *str){
    return(smi_try_add_token(&helper_ctx.primary_ctx, SCu8(str), helper_ctx.selected_base_kind));
}

internal Operator_Set*
sm_begin_op_set(void){
    Operator_Set *set = push_array_zero(helper_ctx.arena, Operator_Set, 1);
    set->lexeme_to_ptr = make_table_Data_u64(helper_ctx.primary_ctx.allocator, 100);
    helper_ctx.selected_op_set = set;
    return(set);
}

internal b32
sm_op(String_Const_u8 lexeme, String_Const_u8 name){
    b32 result = false;
    Operator_Set *set = helper_ctx.selected_op_set;
    Table_Lookup lookup = table_lookup(&set->lexeme_to_ptr, make_data(lexeme.str, lexeme.size));
    if (!lookup.found_match){
        if (smi_try_add_token(&helper_ctx.primary_ctx, name, helper_ctx.selected_base_kind)){
            Operator *op = push_array_zero(helper_ctx.arena, Operator, 1);
            op->name = push_string_copy(helper_ctx.arena, name);
            op->op = push_string_copy(helper_ctx.arena, lexeme);
            table_insert(&set->lexeme_to_ptr, make_data(op->op.str, op->op.size), (u64)PtrAsInt(op));
            sll_queue_push(set->first, set->last, op);
            set->count += 1;
            result = true;
        }
    }
    return(result);
}

internal b32
sm_op(char *lexeme, char *name){
    return(sm_op(SCu8(lexeme), SCu8(name)));
}

internal b32
sm_op(char *lexeme){
    String_Const_u8 l = SCu8(lexeme);
    List_String_Const_u8 name_list = {};
    for (umem i = 0; i < l.size; i += 1){
        Table_Lookup lookup = table_lookup(&helper_ctx.char_to_name, l.str[i]);
        // If this fails first check that all the characters in the lexeme are named!
        Assert(lookup.found_match);
        Data name_data = {};
        table_read(&helper_ctx.char_to_name, lookup, &name_data);
        string_list_push(helper_ctx.arena, &name_list, SCu8(name_data.data, name_data.size));
    }
    String_Const_u8 name = string_list_flatten(helper_ctx.arena, name_list);
    return(sm_op(l, name));
}

internal Keyword_Set*
sm_begin_key_set(String_Const_u8 pretty_name){
    Keyword_Set *set = push_array_zero(helper_ctx.arena, Keyword_Set, 1);
    set->name_to_ptr = make_table_Data_u64(helper_ctx.primary_ctx.allocator, 100);
    set->lexeme_to_ptr = make_table_Data_u64(helper_ctx.primary_ctx.allocator, 100);
    set->pretty_name = push_string_copy(helper_ctx.arena, pretty_name);
    sll_queue_push(helper_ctx.primary_ctx.keywords.first,
                   helper_ctx.primary_ctx.keywords.last, set);
    helper_ctx.primary_ctx.keywords.count += 1;
    helper_ctx.selected_key_set = set;
    return(set);
}

internal Keyword_Set*
sm_begin_key_set(char *pretty_name){
    return(sm_begin_key_set(SCu8(pretty_name)));
}

internal b32
sm_key(String_Const_u8 name, String_Const_u8 lexeme){
    return(smi_key(&helper_ctx.primary_ctx, helper_ctx.selected_key_set, name, lexeme, helper_ctx.selected_base_kind));
}

internal b32
sm_key(char *str, char *lexeme){
    return(sm_key(SCu8(str), SCu8(lexeme)));
}

internal b32
sm_key(char *str){
    String_Const_u8 name = SCu8(str);
    String_Const_u8 lexeme = push_string_copy(helper_ctx.arena,  name);
    lexeme = string_mod_lower(lexeme);
    return(sm_key(name, lexeme));
}

internal b32
sm_key_fallback(String_Const_u8 name){
    return(smi_key_fallback(&helper_ctx.primary_ctx, helper_ctx.selected_key_set, name, helper_ctx.selected_base_kind));
}

internal b32
sm_key_fallback(char *str){
    return(sm_key_fallback(SCu8(str)));
}

internal State*
sm_add_state(String_Const_u8 pretty_name){
    return(smi_add_state(&helper_ctx.primary_ctx, pretty_name));
}

internal State*
sm_add_state(char *pretty_name){
    return(smi_add_state(&helper_ctx.primary_ctx, SCu8(pretty_name)));
}

internal State*
sm_begin_state_machine(void){
    State *state = sm_add_state("root");
    // If this fails first check sm_begin_state_machine is only called once
    Assert(helper_ctx.primary_ctx.model.root == 0);
    helper_ctx.primary_ctx.model.root = state;
    return(state);
}

internal Flag*
sm_add_flag(Flag_Reset_Rule rule){
    return(smi_add_flag(&helper_ctx.primary_ctx, rule));
}

internal void
sm_flag_bind(Flag *flag, Token_Base_Kind emit_flags){
    flag->emit_flags = emit_flags;
}

internal void
sm_sub_flag_bind(Flag *flag, u16 emit_sub_flags){
    flag->emit_sub_flags = emit_sub_flags;
}

internal Emit_Rule*
sm_emit_rule(void){
    Emit_Rule *rule = smi_emit_rule(helper_ctx.arena);
    helper_ctx.selected_emit_rule = rule;
    return(rule);
}

internal void
sm_emit_handler_direct(Flag *flag_check, String_Const_u8 name){
    Emit_Rule *rule = helper_ctx.selected_emit_rule;
    smi_emit_handler(helper_ctx.arena, rule, name, flag_check);
}

internal void
sm_emit_handler_direct(char *name){
    sm_emit_handler_direct(0, SCu8(name));
}

internal void
sm_emit_handler_direct(Flag *flag_check, char *name){
    sm_emit_handler_direct(flag_check, SCu8(name));
}

internal void
sm_emit_handler_keys(Flag *flag_check, Keyword_Set *set){
    Emit_Rule *rule = helper_ctx.selected_emit_rule;
    smi_emit_handler(helper_ctx.arena, rule, set, flag_check);
}

internal void
sm_emit_handler_keys(Keyword_Set *set){
    sm_emit_handler_keys(0, set);
}

internal void
sm_emit_handler_keys_delim(Flag *flag_check, Keyword_Set *set){
    Emit_Rule *rule = helper_ctx.selected_emit_rule;
    smi_emit_handler_delim(helper_ctx.arena, rule, set, flag_check);
}

internal void
sm_emit_handler_keys_delim(Keyword_Set *set){
    sm_emit_handler_keys_delim(0, set);
}

internal Transition*
sm_case(String_Const_u8 str, Flag *flag_check, b32 flag_check_value, State *dst, Transition_Consume_Rule consume_rule, Emit_Rule *emit){
    Transition *transition = smi_case(&helper_ctx.primary_ctx, helper_ctx.selected_state, TransitionCaseKind_CharaterArray, str,
                                      flag_check, flag_check_value, dst, consume_rule, emit);
    helper_ctx.selected_transition = transition;
    return(transition);
}
internal Transition*
sm_case(Transition_Case_Kind kind, Flag *flag_check, b32 flag_check_value, State *dst, Transition_Consume_Rule consume_rule, Emit_Rule *emit){
    Assert(kind != TransitionCaseKind_CharaterArray);
    String_Const_u8 str = {};
    Transition *transition = smi_case(&helper_ctx.primary_ctx, helper_ctx.selected_state, kind, str,
                                      flag_check, flag_check_value, dst, consume_rule, emit);
    helper_ctx.selected_transition = transition;
    return(transition);
}

internal Transition*
sm_case(char *str, State *dst){
    return(sm_case(SCu8(str), 0, 0, dst, Transition_Consume, 0));
}
internal Transition*
sm_case(u8 *str, State *dst){
    return(sm_case(SCu8(str), 0, 0, dst, Transition_Consume, 0));
}
internal Transition*
sm_case_peek(char *str, State *dst){
    return(sm_case(SCu8(str), 0, 0, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_case_peek(u8 *str, State *dst){
    return(sm_case(SCu8(str), 0, 0, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_case_flagged(Flag *flag_check, b32 flag_check_value, char *str, State *dst){
    return(sm_case(SCu8(str), flag_check, flag_check_value, dst, Transition_Consume, 0));
}
internal Transition*
sm_case_flagged(Flag *flag_check, b32 flag_check_value, u8 *str, State *dst){
    return(sm_case(SCu8(str), flag_check, flag_check_value, dst, Transition_Consume, 0));
}
internal Transition*
sm_case_peek_flagged(Flag *flag_check, b32 flag_check_value, char *str, State *dst){
    return(sm_case(SCu8(str), flag_check, flag_check_value, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_case_peek_flagged(Flag *flag_check, b32 flag_check_value, u8 *str, State *dst){
    return(sm_case(SCu8(str), flag_check, flag_check_value, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_case(char *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), 0, 0, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_case(u8 *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), 0, 0, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_case_peek(char *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), 0, 0, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}
internal Transition*
sm_case_peek(u8 *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), 0, 0, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}
internal Transition*
sm_case_flagged(Flag *flag_check, b32 flag_check_value, char *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_case_flagged(Flag *flag_check, b32 flag_check_value, u8 *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_case_peek_flagged(Flag *flag_check, b32 flag_check_value, char *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}
internal Transition*
sm_case_peek_flagged(Flag *flag_check, b32 flag_check_value, u8 *str, Emit_Rule *emit){
    return(sm_case(SCu8(str), flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}

internal Transition*
sm_case_eof(State *dst){
    return(sm_case(TransitionCaseKind_EOF, 0, 0, dst, Transition_Consume, 0));
}
internal Transition*
sm_case_eof_peek(State *dst){
    return(sm_case(TransitionCaseKind_EOF, 0, 0, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_case_eof_flagged(Flag *flag_check, b32 flag_check_value, State *dst){
    return(sm_case(TransitionCaseKind_EOF, flag_check, flag_check_value, dst, Transition_Consume, 0));
}
internal Transition*
sm_case_eof_peek_flagged(Flag *flag_check, b32 flag_check_value, State *dst){
    return(sm_case(TransitionCaseKind_EOF, flag_check, flag_check_value, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_case_eof(Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_EOF, 0, 0, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_case_eof_peek(Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_EOF, 0, 0, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}
internal Transition*
sm_case_eof_flagged(Flag *flag_check, b32 flag_check_value, Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_EOF, flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_case_eof_peek_flagged(Flag *flag_check, b32 flag_check_value, Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_EOF, flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}

internal Transition*
sm_fallback(State *dst){
    return(sm_case(TransitionCaseKind_Fallback, 0, 0, dst, Transition_Consume, 0));
}
internal Transition*
sm_fallback_peek(State *dst){
    return(sm_case(TransitionCaseKind_Fallback, 0, 0, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_fallback_flagged(Flag *flag_check, b32 flag_check_value, State *dst){
    return(sm_case(TransitionCaseKind_Fallback, flag_check, flag_check_value, dst, Transition_Consume, 0));
}
internal Transition*
sm_fallback_peek_flagged(Flag *flag_check, b32 flag_check_value, State *dst){
    return(sm_case(TransitionCaseKind_Fallback, flag_check, flag_check_value, dst, Transition_NoConsume, 0));
}
internal Transition*
sm_fallback(Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_Fallback, 0, 0, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_fallback_peek(Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_Fallback, 0, 0, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}
internal Transition*
sm_fallback_flagged(Flag *flag_check, b32 flag_check_value, Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_Fallback, flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_Consume, emit));
}
internal Transition*
sm_fallback_peek_flagged(Flag *flag_check, b32 flag_check_value, Emit_Rule *emit){
    return(sm_case(TransitionCaseKind_Fallback, flag_check, flag_check_value, helper_ctx.primary_ctx.model.root, Transition_NoConsume, emit));
}

internal void
sm_match_delim(State *dst, State *fail_dst){
    sm_case(TransitionCaseKind_DelimMatch, 0, 0, dst, Transition_NoConsume, 0);
    sm_case(TransitionCaseKind_DelimMatchFail, 0, 0, fail_dst, Transition_NoConsume, 0);
}

internal void
sm_on_transition_set_flag(Flag *flag, b32 value){
    Transition *transition = helper_ctx.selected_transition;
    smi_append_set_flag(helper_ctx.arena, &transition->activation_actions, flag, value);
}

internal void
sm_emit_check_set_flag(String_Const_u8 emit_check, Flag *flag, b32 value){
    Emit_Rule *rule = helper_ctx.selected_emit_rule;
    Emit_Check *new_check = push_array_zero(helper_ctx.arena, Emit_Check, 1);
    sll_queue_push(rule->emit_checks.first, rule->emit_checks.last, new_check);
    rule->emit_checks.count += 1;
    new_check->emit_check = push_string_copy(helper_ctx.arena, emit_check);
    new_check->flag = flag;
    new_check->value = value;
}

internal void
sm_emit_check_set_flag(char *emit_check, Flag *flag, b32 value){
    sm_emit_check_set_flag(SCu8(emit_check), flag, value);
}

internal void
sm_set_flag(Flag *flag, b32 value){
    State *state = helper_ctx.selected_state;
    smi_append_set_flag(helper_ctx.arena, &state->on_entry_actions, flag, value);
}

internal void
sm_delim_mark_first(void){
    State *state = helper_ctx.selected_state;
    smi_append_delim_mark_first(helper_ctx.arena, &state->on_entry_actions);
}

internal void
sm_delim_mark_one_past_last(void){
    State *state = helper_ctx.selected_state;
    smi_append_delim_mark_one_past_last(helper_ctx.arena, &state->on_entry_actions);
}

////////////////////////////////

// NOTE(allen): OPERATORS FOR COMPOSING MODEL COMPONENTS AS EXPRESSIONS

internal Operator_Set*
smo_copy_op_set(Operator_Set *set){
    Operator_Set *new_set = push_array_zero(helper_ctx.arena, Operator_Set, 1);
    new_set->lexeme_to_ptr = make_table_Data_u64(helper_ctx.primary_ctx.allocator, set->count*2);
    for (Operator *node = set->first;
         node != 0;
         node = node->next){
        Operator *new_node = push_array_zero(helper_ctx.arena, Operator, 1);
        sll_queue_push(new_set->first, new_set->last, new_node);
        new_set->count += 1;
        new_node->name = node->name;
        new_node->op = node->op;
        table_insert(&new_set->lexeme_to_ptr, make_data(new_node->op.str, new_node->op.size), (u64)PtrAsInt(new_node));
    }
    return(new_set);
}

internal void
smo_remove_ops_with_prefix(Operator_Set *set, String_Const_u8 prefix){
    Operator *first = 0;
    Operator *last = 0;
    i32 count = 0;
    
    for (Operator *node = set->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        if (string_match(prefix, string_prefix(node->op, prefix.size))){
            table_erase(&set->lexeme_to_ptr, make_data(node->op.str, node->op.size));
        }
        else{
            sll_queue_push(first, last, node);
            count += 1;
        }
    }
    
    set->first = first;
    set->last = last;
    set->count = count;
}

internal void
smo_remove_ops_with_prefix(Operator_Set *set, char *prefix){
    smo_remove_ops_with_prefix(set, SCu8(prefix));
}

internal void
smo_remove_ops_without_prefix(Operator_Set *set, String_Const_u8 prefix){
    Operator *first = 0;
    Operator *last = 0;
    i32 count = 0;
    
    for (Operator *node = set->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        if (!string_match(prefix, string_prefix(node->op, prefix.size))){
            table_erase(&set->lexeme_to_ptr, make_data(node->op.str, node->op.size));
        }
        else{
            sll_queue_push(first, last, node);
            count += 1;
        }
    }
    
    set->first = first;
    set->last = last;
    set->count = count;
}

internal void
smo_remove_ops_without_prefix(Operator_Set *set, char *prefix){
    smo_remove_ops_without_prefix(set, SCu8(prefix));
}

internal void
smo_ops_string_skip(Operator_Set *set, umem size){
    Operator_Set new_set = {};
    new_set.lexeme_to_ptr = make_table_Data_u64(helper_ctx.primary_ctx.allocator, set->count*2);
    
    for (Operator *node = set->first, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        if (node->op.size > size){
            String_Const_u8 new_op = string_skip(node->op, size);
            if (table_insert(&new_set.lexeme_to_ptr, make_data(new_op.str, new_op.size), (u64)PtrAsInt(node))){
                node->op = new_op;
                sll_queue_push(new_set.first, new_set.last, node);
                new_set.count += 1;
            }
        }
    }
    
    table_free(&set->lexeme_to_ptr);
    *set = new_set;
}

internal Character_Set*
smo_new_char_set(void){
    Character_Set *set = push_array_zero(helper_ctx.arena, Character_Set, 1);
    set->table = make_table_u64_u64(helper_ctx.primary_ctx.allocator, 100);
    return(set);
}

internal void
smo_char_set_union_ops_firsts(Character_Set *chars, Operator_Set *ops){
    for (Operator *node = ops->first;
         node != 0;
         node = node->next){
        String_Const_u8 lexeme = node->op;
        u64 c = lexeme.str[0];
        table_insert(&chars->table, c, c);
    }
}

internal void
smo_char_set_remove(Character_Set *set, char *str){
    for (char *ptr = str; *ptr != 0; ptr += 1){
        table_erase(&set->table, (u64)(*ptr));
    }
}

internal char*
smo_char_set_get_array(Character_Set *set){
    u32 count = set->table.used_count;
    char *result = push_array_zero(helper_ctx.arena, char, count + 1);
    u32 index = 0;
    u32 slot_count = set->table.slot_count;
    for (u32 i = 0; i < slot_count; i += 1){
        u64 c = set->table.keys[i];
        if (c != table_empty_key && c != table_erased_key){
            result[index] = (u8)(c);
            index += 1;
        }
    }
    result[count] = 0;
    return(result);
}

internal State*
smo_op_set_lexer_root(Operator_Set *set, State *machine_root, String_Const_u8 fallback_token_name){
    Base_Allocator *allocator = helper_ctx.primary_ctx.allocator;
    Table_Data_u64 string_to_state = make_table_Data_u64(allocator, set->count*8);
    
    State *root = sm_add_state("op root");
    
    for (Operator *node = set->first;
         node != 0;
         node = node->next){
        String_Const_u8 lexeme = node->op;
        for (umem i = 1; i < lexeme.size; i += 1){
            String_Const_u8 prefix = string_prefix(lexeme, i);
            Table_Lookup lookup = table_lookup(&string_to_state, make_data(prefix.str, prefix.size));
            if (!lookup.found_match){
                State *state = sm_add_state("op stage");
                State *parent = 0;
                if (prefix.size == 1){
                    parent = root;
                }
                else{
                    lookup = table_lookup(&string_to_state, make_data(prefix.str, prefix.size - 1));
                    Assert(lookup.found_match);
                    u64 val = 0;
                    table_read(&string_to_state, lookup, &val);
                    parent = (State*)IntAsPtr(val);
                }
                u8 space[1];
                space[0] = prefix.str[prefix.size - 1];
                String_Const_u8 string = {space, 1};
                smi_case(&helper_ctx.primary_ctx, parent, TransitionCaseKind_CharaterArray, string, 0, 0, state, Transition_Consume, 0);
                table_insert(&string_to_state, make_data(prefix.str, prefix.size), (u64)PtrAsInt(state));
            }
        }
    }
    
    for (Operator *node = set->first;
         node != 0;
         node = node->next){
        String_Const_u8 lexeme = node->op;
        Table_Lookup lookup = table_lookup(&string_to_state, make_data(lexeme.str, lexeme.size));
        if (!lookup.found_match){
            State *parent = 0;
            if (lexeme.size == 1){
                parent = root;
            }
            else{
                lookup = table_lookup(&string_to_state, make_data(lexeme.str, lexeme.size - 1));
                Assert(lookup.found_match);
                u64 val = 0;
                table_read(&string_to_state, lookup, &val);
                parent = (State*)IntAsPtr(val);
            }
            u8 space[1];
            space[0] = lexeme.str[lexeme.size - 1];
            String_Const_u8 string = {space, 1};
            Emit_Rule *emit = smi_emit_rule(helper_ctx.arena);
            smi_emit_handler(helper_ctx.arena, emit, node->name, 0);
            smi_case(&helper_ctx.primary_ctx, parent, TransitionCaseKind_CharaterArray, string, 0, 0, machine_root, Transition_Consume, emit);
        }
    }
    
    for (Operator *node = set->first;
         node != 0;
         node = node->next){
        String_Const_u8 lexeme = node->op;
        Table_Lookup lookup = table_lookup(&string_to_state, make_data(lexeme.str, lexeme.size));
        if (lookup.found_match){
            u64 val = 0;
            table_read(&string_to_state, lookup, &val);
            State *state = (State*)IntAsPtr(val);
            String_Const_u8 string = {};
            Emit_Rule *emit = smi_emit_rule(helper_ctx.arena);
            smi_emit_handler(helper_ctx.arena, emit, node->name, 0);
            smi_case(&helper_ctx.primary_ctx, state, TransitionCaseKind_Fallback, string, 0, 0, machine_root, Transition_NoConsume, emit);
        }
    }
    
    {
        String_Const_u8 zero_string = {};
        Emit_Rule *emit = smi_emit_rule(helper_ctx.arena);
        smi_emit_handler(helper_ctx.arena, emit, fallback_token_name, 0);
        smi_case(&helper_ctx.primary_ctx, root, TransitionCaseKind_Fallback, zero_string, 0, 0, machine_root, Transition_NoConsume, emit);
    }
    for (Operator *node = set->first;
         node != 0;
         node = node->next){
        String_Const_u8 lexeme = node->op;
        for (umem i = 1; i < lexeme.size; i += 1){
            String_Const_u8 prefix = string_prefix(lexeme, i);
            Table_Lookup lookup = table_lookup(&string_to_state, make_data(prefix.str, prefix.size));
            Assert(lookup.found_match);
            u64 val = 0;
            table_read(&string_to_state, lookup, &val);
            State *state = (State*)IntAsPtr(val);
            String_Const_u8 string = {};
            Emit_Rule *emit = smi_emit_rule(helper_ctx.arena);
            smi_emit_handler(helper_ctx.arena, emit, fallback_token_name, 0);
            smi_case(&helper_ctx.primary_ctx, state, TransitionCaseKind_Fallback, string, 0, 0, machine_root, Transition_NoConsume, emit);
        }
    }
    
    table_free(&string_to_state);
    
    return(root);
}

internal State*
smo_op_set_lexer_root(Operator_Set *set, State *machine_root, char *fallback_token_name){
    return(smo_op_set_lexer_root(set, machine_root, SCu8(fallback_token_name)));
}

////////////////////////////////

// NOTE(allen): HELPERS

// NOTE(allen): utf8 should be an u8 array with 129 slots.
// This will fill it out to represent all characters above the ASCII range.
internal void
smh_utf8_fill(u8 *utf8){
    for (u16 i = 0; i < 128; i += 1){
        utf8[i] = (u8)(i + 128);
    }
    utf8[128] = 0;
}

internal void
smh_set_base_character_names(void){
    sm_char_name('{', "BraceOp");
    sm_char_name('}', "BraceCl");
    sm_char_name('(', "ParenOp");
    sm_char_name(')', "ParenCl");
    sm_char_name('[', "BrackOp");
    sm_char_name(']', "BrackCl");
    sm_char_name('-', "Minus");
    sm_char_name('+', "Plus");
    sm_char_name('.', "Dot");
    sm_char_name('!', "Bang");
    sm_char_name('*', "Star");
    sm_char_name(',', "Comma");
    sm_char_name(':', "Colon");
    sm_char_name(';', "Semicolon");
    sm_char_name('@', "At");
    sm_char_name('#', "Pound");
    sm_char_name('$', "Dollar");
    sm_char_name('%', "Percent");
    sm_char_name('^', "Carrot");
    sm_char_name('&', "Amp");
    sm_char_name('=', "Eq");
    sm_char_name('<', "Less");
    sm_char_name('>', "Grtr");
    sm_char_name('~', "Tilde");
    sm_char_name('/', "Slash");
    sm_char_name('?', "Question");
    sm_char_name('|', "Pipe");
}

internal void
smh_typical_tokens(void){
    sm_select_base_kind(TokenBaseKind_EOF);
    sm_direct_token_kind("EOF");
    
    sm_select_base_kind(TokenBaseKind_Whitespace);
    sm_direct_token_kind("Whitespace");
    
    sm_select_base_kind(TokenBaseKind_LexError);
    sm_direct_token_kind("LexError");
}

////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////

// NOTE(allen): OPTIMIZER

internal String_Const_u8
string_char_subtract(String_Const_u8 a, String_Const_u8 b){
    for (umem i = 0; i < b.size; i += 1){
        u8 c = b.str[i];
        for (umem j = 0; j < a.size;){
            if (a.str[j] == c){
                a.str[j] = a.str[a.size - 1];
                a.size -= 1;
            }
            else{
                j += 1;
            }
        }
    }
    return(a);
}

internal Action_List
opt_copy_action_list(Arena *arena, Action_List actions){
    Action_List result = {};
    for (Action *node = actions.first;
         node != 0;
         node = node->next){
        Action *new_node = push_array_write(arena, Action, 1, node);
        zdll_push_back(result.first, result.last, new_node);
        result.count += 1;
    }
    return(result);
}

internal Flag*
opt_flag_fixup(Flag *old_flag, Table_u64_u64 old_to_new){
    Flag *result = 0;
    if (old_flag != 0){
        Table_Lookup lookup = table_lookup(&old_to_new, (u64)PtrAsInt(old_flag));
        Assert(lookup.found_match);
        u64 val = 0;
        table_read(&old_to_new, lookup, &val);
        result = (Flag*)IntAsPtr(val);
    }
    return(result);
}

internal Transition_Case
opt_copy_condition(Arena *arena, Transition_Case condition, Table_u64_u64 old_to_new){
    Transition_Case result = condition;
    if (result.kind == TransitionCaseKind_ConditionSet){
        result.condition_set = smi_condition_set_copy(arena, condition.condition_set);
        for (Condition_Node *node = result.condition_set.first;
             node != 0;
             node = node->next){
            Field_Set fields = node->fields;
            for (Field_Pin_List *pin_list = fields.first;
                 pin_list != 0;
                 pin_list = pin_list->next){
                for (Field_Pin *pin = pin_list->first;
                     pin != 0;
                     pin = pin->next){
                    pin->flag = opt_flag_fixup(pin->flag, old_to_new);
                }
            }
        }
    }
    return(result);
}

internal Emit_Rule*
opt_copy_emit_rule(Arena *arena, Emit_Rule *emit, Table_u64_u64 old_to_new){
    Emit_Rule *new_emit = push_array_write(arena, Emit_Rule, 1, emit);
    block_zero_struct(&new_emit->emit_checks);
    for (Emit_Check *emit_check = emit->emit_checks.first;
         emit_check != 0;
         emit_check = emit_check->next){
        Emit_Check *new_emit_check = push_array_write(arena, Emit_Check, 1, emit_check);
        sll_queue_push(new_emit->emit_checks.first, new_emit->emit_checks.last, new_emit_check);
        new_emit->emit_checks.count += 1;
        new_emit_check->flag = opt_flag_fixup(new_emit_check->flag, old_to_new);
    }
    new_emit->first = 0;
    new_emit->last = 0;
    for (Emit_Handler *handler = emit->first;
         handler != 0;
         handler = handler->next){
        Emit_Handler *new_handler = push_array_write(arena, Emit_Handler, 1, handler);
        sll_queue_push(new_emit->first, new_emit->last, new_handler);
        new_handler->flag_check = opt_flag_fixup(handler->flag_check, old_to_new);
    }
    return(new_emit);
}

internal Lexer_Model
opt_copy_model(Arena *arena, Lexer_Model model){
    Lexer_Model result = {};
    
    i32 pointer_count = model.states.count + model.flags.count;
    Table_u64_u64 old_to_new = make_table_u64_u64(arena->base_allocator, pointer_count*2);
    Table_u64_u64 new_to_old = make_table_u64_u64(arena->base_allocator, pointer_count*2);
    
    for (Flag *flag = model.flags.first;
         flag != 0;
         flag = flag->next){
        Flag *new_flag = push_array_zero(arena, Flag, 1);
        sll_queue_push(result.flags.first, result.flags.last, new_flag);
        result.flags.count += 1;
        new_flag->reset_rule = flag->reset_rule;
        new_flag->emit_flags = flag->emit_flags;
        new_flag->emit_sub_flags = flag->emit_sub_flags;
        table_insert(&old_to_new, (u64)PtrAsInt(flag), (u64)PtrAsInt(new_flag));
        table_insert(&new_to_old, (u64)PtrAsInt(new_flag), (u64)PtrAsInt(flag));
    }
    
    for (State *state = model.states.first;
         state != 0;
         state = state->next){
        State *new_state = push_array_zero(arena, State, 1);
        sll_queue_push(result.states.first, result.states.last, new_state);
        result.states.count += 1;
        table_insert(&old_to_new, (u64)PtrAsInt(state), (u64)PtrAsInt(new_state));
        table_insert(&new_to_old, (u64)PtrAsInt(new_state), (u64)PtrAsInt(state));
        new_state->pretty_name = push_string_copy(arena, state->pretty_name);
    }
    
    for (State *new_state = result.states.first;
         new_state != 0;
         new_state = new_state->next){
        Table_Lookup lookup = table_lookup(&new_to_old, (u64)PtrAsInt(new_state));
        Assert(lookup.found_match);
        State *state = 0;
        u64 val = 0;
        table_read(&new_to_old, lookup, &val);
        state = (State*)(IntAsPtr(val));
        
        for (Transition *trans = state->transitions.first;
             trans != 0;
             trans = trans->next){
            Transition *new_trans = push_array_zero(arena, Transition, 1);
            zdll_push_back(new_state->transitions.first, new_state->transitions.last, new_trans);
            new_state->transitions.count += 1;
            new_trans->parent_state = new_state;
            new_trans->condition = opt_copy_condition(arena, trans->condition, old_to_new);
            new_trans->activation_actions = opt_copy_action_list(arena, trans->activation_actions);
            for (Action *action = new_trans->activation_actions.first;
                 action != 0;
                 action = action->next){
                switch (action->kind){
                    case ActionKind_SetFlag:
                    {
                        action->set_flag.flag = opt_flag_fixup(action->set_flag.flag, old_to_new);
                    }break;
                    
                    case ActionKind_Emit:
                    {
                        action->emit_rule = opt_copy_emit_rule(arena, action->emit_rule, old_to_new);
                    }break;
                }
            }
            
            lookup = table_lookup(&old_to_new, (u64)PtrAsInt(trans->dst_state));
            Assert(lookup.found_match);
            
            State *new_dst_state = 0;
            table_read(&old_to_new, lookup, &val);
            new_dst_state = (State*)(IntAsPtr(val));
            
            new_trans->dst_state = new_dst_state;
        }
    }
    
    table_free(&old_to_new);
    table_free(&new_to_old);
    
    for (State *state = model.states.first, *new_state = result.states.first;
         state != 0 && new_state != 0;
         state = state->next, new_state = new_state->next){
        if (model.root == state){
            result.root = new_state;
            break;
        }
    }
    Assert(result.root);
    return(result);
}

internal void
opt_simplify_transitions(Lexer_Primary_Context *ctx){
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        Transition_List *transitions = &state->transitions;
        
        b32 is_delim_match = false;
        if (transitions->first->condition.kind == TransitionCaseKind_DelimMatch){
            is_delim_match = true;
        }
        
        if (!is_delim_match){
            Transition *first = 0;
            Transition *last = 0;
            i32 count = 0;
            
            for (Transition *trans = transitions->first, *next = 0;
                 trans != 0;
                 trans = next){
                next = trans->next;
                Transition_Case condition = trans->condition;
                Assert(condition.kind == TransitionCaseKind_ConditionSet);
                Condition_Set condition_set = condition.condition_set;
                for (Transition *prev_trans = first;
                     prev_trans != 0;
                     prev_trans = prev_trans->next){
                    Transition_Case prev_condition = prev_trans->condition;
                    condition_set = smi_condition_set_subtract(&ctx->arena,
                                                               condition_set,
                                                               prev_condition.condition_set);
                    if (condition_set.count == 0){
                        break;
                    }
                }
                if (condition_set.count != 0){
                    trans->condition.condition_set = condition_set;
                    zdll_push_back(first, last, trans);
                    count += 1;
                }
            }
            
            transitions->first = first;
            transitions->last = last;
            transitions->count = count;
        }
    }
}

internal void
opt_mark_all_states_excluded(Lexer_Primary_Context *ctx){
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        state->optimized_in = false;
    }
}

internal void
opt_mark_all_states_included(Lexer_Primary_Context *ctx){
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        state->optimized_in = true;
    }
}

internal void
opt_discard_all_excluded_states(Lexer_Primary_Context *ctx){
    State *first = 0;
    State *last = 0;
    i32 count = 0;
    for (State *state = ctx->model.states.first, *next = 0;
         state != 0;
         state = next){
        next = state->next;
        if (state->optimized_in){
            state->optimized_in = false;
            sll_queue_push(first, last, state);
            count += 1;
        }
    }
    ctx->model.states.first = first;
    ctx->model.states.last = last;
    ctx->model.states.count = count;
}

internal void
opt_include_reachable_states(State *state){
    if (!state->optimized_in){
        state->optimized_in = true;
        for (Transition *trans = state->transitions.first;
             trans != 0;
             trans = trans->next){
            opt_include_reachable_states(trans->dst_state);
        }
    }
}

internal void
opt_update_state_back_references(Lexer_Primary_Context *ctx){
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        block_zero_struct(&state->back_references);
    }
    
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        for (Transition *trans = state->transitions.first;
             trans != 0;
             trans = trans->next){
            State *dst = trans->dst_state;
            Transition_Ptr_Node *new_ptr_node = push_array_zero(&ctx->arena, Transition_Ptr_Node, 1);
            new_ptr_node->ptr = trans;
            sll_queue_push(dst->back_references.first,
                           dst->back_references.last,
                           new_ptr_node);
            dst->back_references.count += 1;
        }
    }
}

internal void
opt_set_auto_zero_flags_on_root(Lexer_Primary_Context *ctx){
    State *root = ctx->model.root;
    smi_append_zero_flags(&ctx->arena, &root->on_entry_actions);
}

internal void
opt_transfer_state_actions_to_transitions(Lexer_Primary_Context *ctx){
    opt_update_state_back_references(ctx);
    
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        Action_List actions = state->on_entry_actions;
        if (actions.count > 0){
            for (Transition_Ptr_Node *node = state->back_references.first;
                 node != 0;
                 node = node->next){
                Transition *trans = node->ptr;
                Action_List actions_copy = opt_copy_action_list(&ctx->arena, actions);
                if (trans->activation_actions.first == 0){
                    trans->activation_actions = actions_copy;
                }
                else{
                    trans->activation_actions.last->next = actions_copy.first;
                    actions_copy.first->prev = trans->activation_actions.last;
                    trans->activation_actions.last = actions_copy.last;
                    trans->activation_actions.count += actions_copy.count;
                }
            }
            block_zero_struct(&state->on_entry_actions);
        }
    }
}

internal void
opt_flags_set_numbers(Lexer_Model model){
    i32 number = 0;
    for (Flag *flag = model.flags.first;
         flag != 0;
         flag = flag->next){
        flag->number = number;
        number += 1;
    }
}

internal void
opt_states_set_numbers(Lexer_Model model){
    i32 number = 1;
    for (State *state = model.states.first;
         state != 0;
         state = state->next){
        state->number = number;
        number += 1;
    }
}

internal void
opt_transition_pull_actions_backward(Lexer_Primary_Context *ctx, Transition *a, Transition *b){
    if (b->activation_actions.count > 0){
        Action_List b_actions = opt_copy_action_list(&ctx->arena, b->activation_actions);
        if (a->activation_actions.first == 0){
            a->activation_actions = b_actions;
        }
        else{
            if (b_actions.first != 0){
                a->activation_actions.last->next = b_actions.first;
                a->activation_actions.last = b_actions.last;
                a->activation_actions.count += b_actions.count;
            }
        }
    }
    a->dst_state = b->dst_state;
}

internal void
opt_transition_push_actions_forward(Lexer_Primary_Context *ctx, Transition *a, Transition *b){
    if (b->activation_actions.count > 0){
        Action_List a_actions = opt_copy_action_list(&ctx->arena, a->activation_actions);
        if (b->activation_actions.first == 0){
            b->activation_actions = a_actions;
        }
        else{
            if (a_actions.first != 0){
                a_actions.last->next = b->activation_actions.first;
                b->activation_actions.first = a_actions.first;
                b->activation_actions.count += a_actions.count;
            }
        }
    }
}

internal b32
opt_action_list_contains_consume(Action_List list){
    b32 result = false;
    for (Action *act = list.first;
         act != 0;
         act = act->next){
        if (act->kind == ActionKind_Consume){
            result = true;
            break;
        }
    }
    return(result);
}

internal void
opt_skip_past_thunk_states(Lexer_Primary_Context *ctx){
    opt_mark_all_states_included(ctx);
    
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        // TODO(allen): A more complete thunk state test would check if all transitions
        // have the same effect.  If they do, then it is a thunk state.  Only having
        // one transition is just a special case of this more general rule.
        if (state->transitions.count == 1){
            Transition *trans = state->transitions.first;
            // TODO(allen): Consumes could be pulled forward into the transition actions
            // for these types of "thunk states" as well, but only if we add a new concept
            // for representing "action blocks" separately from actions contained in a
            // transition handler, so that a handler can have multiple blocks.  Then we would
            // need to be able to identify thunk cycles, and add an entire extra concept to
            // the state machine generated code, that it can sometimes get into a "stateless"
            // thunk loop that can never be exited, but continues to consume one input at
            // a time doing each action block.
            b32 contains_consume = opt_action_list_contains_consume(trans->activation_actions);
            if (!contains_consume){
                state->optimized_in = false;
            }
        }
    }
    
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        if (state->optimized_in){
            Transition_List *transitions = &state->transitions;
            for (Transition *trans = transitions->first;
                 trans != 0;
                 trans = trans->next){
                for (;!trans->dst_state->optimized_in;){
                    Transition *dst_trans = trans->dst_state->transitions.first;
                    opt_transition_pull_actions_backward(ctx, trans, dst_trans);
                }
            }
        }
    }
}

internal b32
opt_emit_rule_match(Emit_Rule *rule_a, Emit_Rule *rule_b){
    b32 result = true;
    if (rule_a->emit_checks.count != rule_b->emit_checks.count){
        result = false;
        goto end;
    }
    for (Emit_Check *check_a = rule_a->emit_checks.first, *check_b = rule_b->emit_checks.first;
         check_a != 0 && check_b != 0;
         check_a = check_a->next, check_b = check_b->next){
        if (check_a->flag != check_b->flag ||
            !string_match(check_a->emit_check, check_b->emit_check) ||
            check_a->value != check_b->value){
            result = false;
            goto end;
        }
    }
    
    if (rule_a->count != rule_b->count){
        result = false;
        goto end;
    }
    
    for (Emit_Handler *handler_a = rule_a->first, *handler_b = rule_b->first;
         handler_a != 0 && handler_b != 0;
         handler_a = handler_a->next, handler_b = handler_b->next){
        if (handler_a->kind != handler_b->kind ||
            handler_a->flag_check != handler_b->flag_check){
            result = false;
            goto end;
        }
        switch (handler_a->kind){
            case EmitHandlerKind_Direct:
            {
                if (!string_match(handler_a->token_name, handler_b->token_name)){
                    result = false;
                    goto end;
                }
            }break;
            case EmitHandlerKind_Keywords:
            case EmitHandlerKind_KeywordsDelim:
            {
                if (handler_a->keywords != handler_b->keywords){
                    result = false;
                    goto end;
                }
            }break;
        }
    }
    
    end:;
    return(result);
}

internal b32
opt_action_lists_match(Action_List a, Action_List b){
    b32 result = false;
    if (a.count == b.count){
        result = true;
        for (Action *node_a = a.first, *node_b = b.first;
             node_a != 0 && node_b != 0;
             node_a = node_a->next, node_b = node_b->next){
            if (node_a->kind != node_b->kind){
                result = false;
                goto double_break;
            }
            
            switch (node_a->kind){
                case ActionKind_SetFlag:
                {
                    if (node_a->set_flag.flag != node_b->set_flag.flag ||
                        node_a->set_flag.value != node_b->set_flag.value){
                        result = false;
                        goto double_break;
                    }
                }break;
                
                case ActionKind_Emit:
                {
                    if (!opt_emit_rule_match(node_a->emit_rule, node_b->emit_rule)){
                        result = false;
                        goto double_break;
                    }
                }break;
            }
        }
    }
    double_break:;
    return(result);
}

internal void
opt_merge_redundant_transitions_in_each_state(Lexer_Primary_Context *ctx){
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        Transition_List *transitions = &state->transitions;
        
        Transition *first = 0;
        Transition *last = 0;
        i32 count = 0;
        
        for (Transition *trans = transitions->first, *next = 0;
             trans != 0;
             trans = next){
            next = trans->next;
            
            Transition *merge_trans = 0;
            for (Transition *comp_trans = trans->next;
                 comp_trans != 0;
                 comp_trans = comp_trans->next){
                if (opt_action_lists_match(trans->activation_actions, comp_trans->activation_actions) &&
                    trans->dst_state == comp_trans->dst_state){
                    merge_trans = comp_trans;
                    break;
                }
            }
            
            if (merge_trans != 0){
                Assert(trans->condition.kind == TransitionCaseKind_ConditionSet);
                Assert(merge_trans->condition.kind == TransitionCaseKind_ConditionSet);
                merge_trans->condition.condition_set =
                    smi_condition_set_union(&ctx->arena,
                                            trans->condition.condition_set,
                                            merge_trans->condition.condition_set);
            }
            else{
                zdll_push_back(first, last, trans);
                count += 1;
            }
        }
        
        transitions->first = first;
        transitions->last = last;
        transitions->count = count;
    }
}

internal b32
opt_condition_set_is_subset(Arena *scratch, Condition_Set sub, Condition_Set super){
    Temp_Memory temp = begin_temp(scratch);
    Condition_Set left_over = smi_condition_set_subtract(scratch, sub, super);
    b32 result = (left_over.count == 0);
    end_temp(temp);
    return(result);
}

internal void
opt_remove_peeks_without_creating_transition_splits(Lexer_Primary_Context *ctx){
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        Transition_List *transitions = &state->transitions;
        if (transitions->first->condition.kind != TransitionCaseKind_ConditionSet){
            continue;
        }
        
        for (Transition *trans = transitions->first;
             trans != 0;
             trans = trans->next){
            i32 step_counter = 0;
            for (;!opt_action_list_contains_consume(trans->activation_actions);
                 step_counter += 1){
                // NOTE(allen): Hitting this (most likely) indicates a peek cycle
                // that wasn't caught by type checking.
                Assert(step_counter < ctx->model.states.count);
                
                b32 found_action_extension = false;
                State *dst_state = trans->dst_state;
                Transition_List *dst_transitions = &dst_state->transitions;
                if (dst_transitions->first->condition.kind != TransitionCaseKind_ConditionSet){
                    break;
                }
                
                for (Transition *dst_trans = dst_transitions->first;
                     dst_trans != 0;
                     dst_trans = dst_trans->next){
                    if (opt_condition_set_is_subset(&ctx->arena,
                                                    trans->condition.condition_set,
                                                    dst_trans->condition.condition_set)){
                        opt_transition_pull_actions_backward(ctx, trans, dst_trans);
                        found_action_extension = true;
                        break;
                    }
                }
                if (!found_action_extension){
                    break;
                }
            }
        }
    }
}

internal void
opt_remove_peeks_into_single_entry_point_states(Lexer_Primary_Context *ctx){
    opt_update_state_back_references(ctx);
    opt_mark_all_states_included(ctx);
    
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        if (state->transitions.first->condition.kind != TransitionCaseKind_ConditionSet){
            continue;
        }
        
        if (state->back_references.count == 1){
            Transition *src_trans = state->back_references.first->ptr;
            if (src_trans->condition.kind != TransitionCaseKind_ConditionSet){
                continue;
            }
            
            if (!opt_action_list_contains_consume(src_trans->activation_actions)){
                State *src_state = src_trans->parent_state;
                
                state->optimized_in = false;
                
                Transition *first = 0;
                Transition *last = 0;
                i32 count = 0;
                
                for (Transition *trans = state->transitions.first, *next = 0;
                     trans != 0;
                     trans = next){
                    next = trans->next;
                    trans->condition.condition_set =
                        smi_condition_set_intersect(&ctx->arena,
                                                    trans->condition.condition_set,
                                                    src_trans->condition.condition_set);
                    if (trans->condition.condition_set.count > 0){
                        trans->parent_state = src_state;
                        opt_transition_push_actions_forward(ctx, src_trans, trans);
                        zdll_push_back(first, last, trans);
                        count += 1;
                    }
                }
                
                Assert(count != 0);
                if (src_trans->prev != 0){
                    src_trans->prev->next = first;
                }
                if (src_trans->next != 0){
                    src_trans->next->prev = last;
                }
                first->prev = src_trans->prev;
                last->next = src_trans->next;
                src_state->transitions.count += count;
            }
        }
    }
}

internal b32
opt_condition_is_eof_only(Transition_Case condition){
    b32 result = false;
    if (condition.kind == TransitionCaseKind_ConditionSet){
        result = true;
        for (Condition_Node *node = condition.condition_set.first;
             node != 0;
             node = node->next){
            Input_Set inputs = node->inputs;
            if (inputs.count > 1 || inputs.inputs[0] != smi_eof){
                result = false;
                break;
            }
        }
    }
    return(result);
}

internal Keyword_Layout
opt_key_layout(Arena *arena, Keyword_Set keywords, i32 slot_count, u64 seed){
    Keyword_Layout layout = {};
    slot_count = clamp_bot(keywords.count + 1, slot_count);
    layout.seed = seed;
    layout.hashes = push_array_zero(arena, u64, slot_count);
    layout.contributed_error = push_array_zero(arena, u64, slot_count);
    layout.slots = push_array_zero(arena, Keyword*, slot_count);
    layout.slot_count = slot_count;
    for (Keyword *keyword = keywords.first;
         keyword != 0;
         keyword = keyword->next){
        u64 hash = lexeme_hash(seed, keyword->lexeme.str, keyword->lexeme.size);
        i32 first_index = (hash%slot_count);
        i32 index = first_index;
        
        Keyword *keyword_insert = keyword;
        u64 contributed_error = 0;
        
        for (;;){
            if (layout.slots[index] == 0){
                layout.hashes[index] = hash;
                layout.contributed_error[index] = contributed_error;
                layout.slots[index] = keyword_insert;
                break;
            }
            else{
                if (contributed_error > layout.contributed_error[index]){
                    Swap(u64, hash, layout.hashes[index]);
                    Swap(Keyword*, keyword_insert, layout.slots[index]);
                    Swap(u64, contributed_error, layout.contributed_error[index]);
                }
            }
            index += 1;
            contributed_error += 1;
            if (index >= slot_count){
                index = 0;
            }
            if (index == first_index){
                InvalidPath;
            }
        }
    }
    i32 max_run_length = 0;
    i32 run_length = 0;
    for (i32 i = 0; i < slot_count; i += 1){
        if (layout.slots[i] == 0){
            run_length = 0;
        }
        else{
            run_length += 1;
            layout.error_score += run_length;
            max_run_length = max(max_run_length, run_length);
        }
    }
    i32 total_run_length = run_length;
    for (i32 i = 0; i < slot_count; i += 1){
        if (layout.slots[i] == 0){
            break;
        }
        else{
            layout.error_score += run_length;
            total_run_length += 1;
            max_run_length = max(max_run_length, total_run_length);
        }
    }
    layout.max_single_error_score = max_run_length;
    layout.iterations_per_lookup = (f32)layout.error_score/(f32)layout.slot_count;
    return(layout);
}

internal u64
random_u64_dirty(void){
    u64 a = pcg32_random();
    u64 b = pcg32_random();
    return((b << 32) | a);
}

#if 0
internal Keyword_Layout
opt_key_layout(Arena *arena, Keyword_Set keywords){
    i32 slot_count = keywords.count*2;
    u64 seed = random_u64_dirty();
    return(opt_key_layout(arena, keywords, slot_count, seed));
}
#endif

internal Keyword_Layout
opt_key_layout(Arena *arena, Keyword_Set keywords){
    i32 init_slot_count = keywords.count + 1;
    if (keywords.count == 1){
        init_slot_count = 1;
    }
    
#if 0
    // heavy optimization effort
    f32 acceptable_error_threshold = 2.f;
    f32 accumulated_error_threshold = 8000.f;
    i32 acceptable_max_single_error = 4;
    i32 accumulated_max_single_error_threshold = Thousand(800);
#else
    // light optimization effort
    f32 acceptable_error_threshold = 1.1f;
    f32 accumulated_error_threshold = 200.f;
    i32 acceptable_max_single_error = 5;
    i32 accumulated_max_single_error_threshold = Thousand(40);
#endif
    
    Keyword_Layout best_layout = {};
    best_layout.iterations_per_lookup = max_f32;
    i32 slot_count = init_slot_count;
    for (;; slot_count += 1){
        f32 accumulated_error = 0;
        for (;;){
            u64 seed = random_u64_dirty();
            Temp_Memory restore_point = begin_temp(arena);
            Keyword_Layout layout = opt_key_layout(arena, keywords, slot_count, seed);
            accumulated_error += layout.iterations_per_lookup;
            
            if (layout.iterations_per_lookup < best_layout.iterations_per_lookup){
                best_layout = layout;
                if (layout.iterations_per_lookup <= acceptable_error_threshold){
                    goto optimize_max_single_error;
                }
            }
            else{
                end_temp(restore_point);
            }
            if (accumulated_error >= accumulated_error_threshold){
                break;
            }
        }
    }
    
    optimize_max_single_error:
    if (best_layout.max_single_error_score <= acceptable_max_single_error){
        goto finished;
    }
    for (;; slot_count += 1){
        u64 accumulated_error = 0;
        for (;;){
            u64 seed = random_u64_dirty();
            Temp_Memory restore_point = begin_temp(arena);
            Keyword_Layout layout = opt_key_layout(arena, keywords, slot_count, seed);
            
            u64 adjusted_error_score = (layout.max_single_error_score + acceptable_max_single_error - 1)/acceptable_max_single_error;
            adjusted_error_score *= adjusted_error_score;
            adjusted_error_score *= acceptable_max_single_error;
            
            accumulated_error += adjusted_error_score;
            
            if (layout.max_single_error_score < best_layout.max_single_error_score &&
                layout.iterations_per_lookup <= best_layout.iterations_per_lookup){
                best_layout = layout;
                if (layout.max_single_error_score <= acceptable_max_single_error){
                    goto finished;
                }
            }
            else{
                end_temp(restore_point);
            }
            if (accumulated_error >= accumulated_max_single_error_threshold){
                break;
            }
        }
    }
    
    
    finished:;
    return(best_layout);
}

////////////////////////////////

internal b32
opt__input_set_contains(Input_Set set, u16 x){
    b32 result = false;
    for (i32 i = 0; i < set.count; i += 1){
        if (set.inputs[i] == x){
            result = true;
            break;
        }
    }
    return(result);
}

internal b32
opt__partial_transition_match(Arena *scratch, Partial_Transition *a, Partial_Transition *b){
    b32 result = false;
    if (smi_field_set_match(scratch, a->fields, b->fields)){
        if (opt_action_lists_match(a->actions, b->actions)){
            if (a->dst_state == b->dst_state){
                result = true;
            }
        }
    }
    return(result);
}

internal void
opt__push_partial_transition(Arena *arena, Partial_Transition_List *list, Field_Set fields, Transition *trans){
    Partial_Transition partial = {};
    partial.fields = fields;
    partial.actions = trans->activation_actions;
    partial.dst_state = trans->dst_state;
    
    b32 is_duplicate = false;
    for (Partial_Transition *node = list->first;
         node != 0;
         node = node->next){
        if (opt__partial_transition_match(arena, node, &partial)){
            is_duplicate = true;
            break;
        }
    }
    
    if (!is_duplicate){
        Partial_Transition *result = push_array_write(arena, Partial_Transition, 1, &partial);
        sll_queue_push(list->first, list->last, result);
        list->count += 1;
    }
}

internal b32
opt__partial_transition_list_match(Arena *scratch, Partial_Transition_List *a, Partial_Transition_List *b){
    b32 result = false;
    if (a->count == b->count){
        result = true;
        for (Partial_Transition *node_a = a->first;
             node_a != 0;
             node_a = node_a->next){
            b32 has_match = false;
            for (Partial_Transition *node_b = b->first;
                 node_b != 0;
                 node_b = node_b->next){
                if (opt__partial_transition_match(scratch, node_a, node_b)){
                    has_match = true;
                    break;
                }
            }
            if (!has_match){
                result = false;
            }
        }
    }
    return(result);
}

internal void
opt__insert_input_into_group(Grouped_Input_Handler *group, u8 x){
    if (!group->inputs_used[x]){
        group->inputs_used[x] = true;
        group->inputs[group->input_count] = x;
        group->input_count += 1;
    }
}

internal Grouped_Input_Handler_List
opt_grouped_input_handlers(Arena *arena, Transition *first_trans){
    Grouped_Input_Handler_List result = {};
    
    Assert(first_trans->condition.kind == TransitionCaseKind_ConditionSet);
    
    Grouped_Input_Handler *biggest_group = 0;
    i32 size_of_biggest = 0;
    
    for (u16 i = 0; i <= 255; i += 1){
        Temp_Memory restore_point = begin_temp(arena);
        Partial_Transition_List list = {};
        for (Transition *trans = first_trans;
             trans != 0;
             trans = trans->next){
            Assert(trans->condition.kind == TransitionCaseKind_ConditionSet);
            Condition_Set condition_set = trans->condition.condition_set;
            for (Condition_Node *node = condition_set.first;
                 node != 0;
                 node = node->next){
                if (opt__input_set_contains(node->inputs, i)){
                    opt__push_partial_transition(arena, &list, node->fields, trans);
                }
            }
        }
        
        Grouped_Input_Handler *matching_group = 0;
        for (Grouped_Input_Handler *group = result.first;
             group != 0;
             group = group->next){
            if (opt__partial_transition_list_match(arena, &group->partial_transitions, &list)){
                matching_group = group;
                break;
            }
        }
        
        if (matching_group != 0){
            end_temp(restore_point);
        }
        else{
            matching_group = push_array_zero(arena, Grouped_Input_Handler, 1);
            sll_queue_push(result.first, result.last, matching_group);
            result.count += 1;
            matching_group->partial_transitions = list;
        }
        opt__insert_input_into_group(matching_group, (u8)i);
        
        if (matching_group->input_count > size_of_biggest){
            size_of_biggest = matching_group->input_count;
            biggest_group = matching_group;
        }
    }
    
    result.group_with_biggest_input_set = biggest_group;
    return(result);
}

////////////////////////////////

internal void
debug_print_states(Lexer_Primary_Context *ctx){
    printf("Number of States: %d\n", ctx->model.states.count);
    i32 transition_count = 0;
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        Transition_List *transitions = &state->transitions;
        transition_count += transitions->count;
    }
    printf("Number of Transitions: %d\n", transition_count);
    for (State *state = ctx->model.states.first;
         state != 0;
         state = state->next){
        printf("State: %.*s\n", string_expand(state->pretty_name));
    }
}

internal void
debug_print_transitions(Arena *scratch, Lexer_Model model){
    Temp_Memory temp = begin_temp(scratch);
    
    i32 field_bit_width = model.flags.count;
    char *field_memory = push_array(scratch, char, field_bit_width);
    
    printf("Number of States: %d\n", model.states.count);
    i32 transition_count = 0;
    for (State *state = model.states.first;
         state != 0;
         state = state->next){
        Transition_List *transitions = &state->transitions;
        transition_count += transitions->count;
    }
    printf("Number of Transitions: %d\n", transition_count);
    
    for (State *state = model.states.first;
         state != 0;
         state = state->next){
        printf("State: %.*s\n", string_expand(state->pretty_name));
        
        Transition_List *transitions = &state->transitions;
        for (Transition *trans = transitions->first;
             trans != 0;
             trans = trans->next){
#define transition_on "Transition on "
            if (trans->condition.kind == TransitionCaseKind_DelimMatch){
                printf("\t" transition_on "<DelimMatch>\n");
            }
            else{
                printf("\t" transition_on "");
                for (Condition_Node *node = trans->condition.condition_set.first;
                     node != 0;
                     node = node->next){
                    printf("([%3d]", node->inputs.count);
                    if (node->inputs.count < 10){
                        b32 all_printable = true;
                        char ascii[30];
                        i32 j = 0;
                        for (i32 i = 0; i < node->inputs.count; i += 1){
                            b32 is_ascii = character_is_basic_ascii(node->inputs.inputs[i]);
                            b32 is_eof = (node->inputs.inputs[i] == smi_eof);
                            if (!(is_ascii || is_eof)){
                                all_printable = false;
                                break;
                            }
                            if (is_ascii){
                                ascii[j] = (char)(node->inputs.inputs[i]);
                                j += 1;
                            }
                            else if (is_eof){
                                ascii[j] = 'E';
                                j += 1;
                                ascii[j] = 'O';
                                j += 1;
                                ascii[j] = 'F';
                                j += 1;
                            }
                        }
                        if (all_printable){
                            printf(" = {%.*s}", j, ascii);
                        }
                    }
                    
                    printf(" x ");
                    
                    printf("(");
                    for (Field_Pin_List *pins = node->fields.first;
                         pins != 0;
                         pins = pins->next){
                        block_fill_u8(field_memory, field_bit_width, '*');
                        for (Field_Pin *pin = pins->first;
                             pin != 0;
                             pin = pin->next){
                            i32 flag_number = pin->flag->number;
                            field_memory[flag_number] = pin->value?'1':'0';
                        }
                        printf("%.*s", field_bit_width, field_memory);
                        if (pins->next != 0){
                            printf(", ");
                        }
                    }
                    printf("))");
                    if (node->next != 0){
                        printf(" union\n\t%.*s", (i32)(sizeof(transition_on) - 1),
                               "                                            ");
                    }
                }
                printf(":\n");
            }
            
            for (Action *act = trans->activation_actions.first;
                 act != 0;
                 act = act->next){
                switch (act->kind){
                    case ActionKind_SetFlag:
                    {
                        printf("\t\tSet Flag\n");
                    }break;
                    
                    case ActionKind_ZeroFlags:
                    {
                        printf("\t\tZero Flags\n");
                    }break;
                    
                    case ActionKind_DelimMarkFirst:
                    {
                        printf("\t\tDelim Mark First\n");
                    }break;
                    
                    case ActionKind_DelimMarkOnePastLast:
                    {
                        printf("\t\tDelim Mark One Past Last\n");
                    }break;
                    
                    case ActionKind_Consume:
                    {
                        printf("\t\tConsume\n");
                    }break;
                    
                    case ActionKind_Emit:
                    {
                        printf("\t\tEmit\n");
                    }break;
                }
            }
            printf("\t\tGo to %.*s;\n", string_expand(trans->dst_state->pretty_name));
        }
    }
    
    end_temp(temp);
}

internal void
debug_print_transitions(Lexer_Primary_Context *ctx){
    debug_print_transitions(&ctx->arena, ctx->model);
}

internal void
debug_print_keyword_table_metrics(Keyword_Layout key_layout, i32 keyword_count){
    printf("used count: %d\n", keyword_count);
    printf("slot count: %d\n", key_layout.slot_count);
    printf("table load factor: %f\n", (f32)keyword_count/(f32)key_layout.slot_count);
    printf("error score: %llu\n", key_layout.error_score);
    printf("error per lookup: %f\n", key_layout.iterations_per_lookup);
    printf("max single error score: %llu\n", key_layout.max_single_error_score);
    for (i32 i = 0; i < key_layout.slot_count; i += 1){
        Keyword *keyword = key_layout.slots[i];
        if (keyword == 0){
            printf("[%d] -> <null>\n", i);
        }
        else{
            printf("[%d] -> \"%.*s\"\n", i, string_expand(keyword->lexeme));
        }
    }
}

////////////////////////////////

internal char*
gen_token_full_name(Arena *arena, String_Const_u8 base_name){
    String_Const_u8 string = push_u8_stringf(arena,
                                             "Token" LANG_NAME_CAMEL_STR "Kind_%.*s",
                                             string_expand(base_name));
    return((char*)(string.str));
}

internal void
gen_tokens(Arena *scratch, Token_Kind_Set tokens, FILE *out){
    Temp_Memory temp = begin_temp(scratch);
    i32 counter = 0;
    fprintf(out, "typedef u16 Token_" LANG_NAME_CAMEL_STR "_Kind;\n");
    fprintf(out, "enum{\n");
    for (Token_Kind_Node *node = tokens.first;
         node != 0;
         node = node->next){
        char *full_name = gen_token_full_name(scratch, node->name);
        fprintf(out, "%s = %d,\n", full_name, counter);
        counter += 1;
    }
    char *full_name = gen_token_full_name(scratch, SCu8("COUNT"));
    fprintf(out, "%s = %d,\n", full_name, counter);
    fprintf(out, "};\n");
    fprintf(out, "char *token_" LANG_NAME_LOWER_STR "_kind_names[] = {\n");
    for (Token_Kind_Node *node = tokens.first;
         node != 0;
         node = node->next){
        fprintf(out, "\"%.*s\",\n", string_expand(node->name));
    }
    fprintf(out, "};\n");
    end_temp(temp);
}

internal void
gen_keyword_table(Arena *scratch, Token_Kind_Set tokens, Keyword_Set keywords, FILE *out){
    Temp_Memory temp = begin_temp(scratch);
    Keyword_Layout key_layout = opt_key_layout(scratch, keywords);
    
    fprintf(out, "u64 %.*s_hash_array[%d] = {\n",
            string_expand(keywords.pretty_name), key_layout.slot_count);
    for (i32 i = 0; i < key_layout.slot_count; i += 1){
        if (key_layout.slots[i] == 0){
            fprintf(out, "0x%016x,", 0);
        }
        else{
            fprintf(out, "0x%016llx,", (u64)((u64)(key_layout.hashes[i]) | 1));
        }
        if (i % 4 == 3 || i + 1 == key_layout.slot_count){
            fprintf(out, "\n");
        }
    }
    fprintf(out, "};\n");
    
    for (i32 i = 0; i < key_layout.slot_count; i += 1){
        if (key_layout.slots[i] != 0){
            fprintf(out, "u8 %.*s_key_array_%d[] = {",
                    string_expand(keywords.pretty_name), i);
            String_Const_u8 lexeme = key_layout.slots[i]->lexeme;
            for (umem j = 0; j < lexeme.size; j += 1){
                fprintf(out, "0x%02x,", lexeme.str[j]);
            }
            fprintf(out, "};\n");
        }
    }
    
    fprintf(out, "String_Const_u8 %.*s_key_array[%d] = {\n",
            string_expand(keywords.pretty_name), key_layout.slot_count);
    for (i32 i = 0; i < key_layout.slot_count; i += 1){
        if (key_layout.slots[i] == 0){
            fprintf(out, "{0, 0},\n");
        }
        else{
            fprintf(out, "{%.*s_key_array_%d, %llu},\n",
                    string_expand(keywords.pretty_name), i, key_layout.slots[i]->lexeme.size);
        }
    }
    fprintf(out, "};\n");
    
    fprintf(out, "Lexeme_Table_Value %.*s_value_array[%d] = {\n",
            string_expand(keywords.pretty_name), key_layout.slot_count);
    for (i32 i = 0; i < key_layout.slot_count; i += 1){
        if (key_layout.slots[i] == 0){
            fprintf(out, "{0, 0},\n");
        }
        else{
            Temp_Memory temp2 = begin_temp(scratch);
            Keyword *keyword = key_layout.slots[i];
            String_Const_u8 name = keyword->name;
            
            char *full_token_name = gen_token_full_name(scratch, name);
            Table_Lookup lookup = table_lookup(&tokens.name_to_ptr, make_data(name.str, name.size));
            Assert(lookup.found_match);
            u64 val = 0;
            table_read(&tokens.name_to_ptr, lookup, &val);
            Token_Kind_Node *token_node = (Token_Kind_Node*)IntAsPtr(val);
            
            fprintf(out, "{%u, %s},\n", token_node->base_kind, full_token_name);
            end_temp(temp2);
        }
    }
    fprintf(out, "};\n");
    
    fprintf(out, "i32 %.*s_slot_count = %d;\n",
            string_expand(keywords.pretty_name), key_layout.slot_count);
    fprintf(out, "u64 %.*s_seed = 0x%016llx;\n",
            string_expand(keywords.pretty_name), key_layout.seed);
    
    end_temp(temp);
}

internal void
gen_flag_check__cont_flow(Flag *flag, b32 value, FILE *out){
    if (value == 0){
        fprintf(out, "!");
    }
    fprintf(out, "HasFlag(%.*s%d, 0x%x)", string_expand(flag->base_name), flag->index, flag->value);
}

internal void
gen_SLOW_field_set_check__cont_flow(Field_Set fields, FILE *out){
    for (Field_Pin_List *pin_list = fields.first;
         pin_list != 0;
         pin_list = pin_list->next){
        fprintf(out, "(");
        if (pin_list->count > 0){
            for (Field_Pin *pin = pin_list->first;
                 pin != 0;
                 pin = pin->next){
                gen_flag_check__cont_flow(pin->flag, pin->value, out);
                if (pin->next != 0){
                    fprintf(out, " && ");
                }
            }
        }
        else{
            fprintf(out, "true");
        }
        fprintf(out, ")");
        
        if (pin_list->next != 0){
            fprintf(out, " || ");
        }
    }
}

internal void
gen_goto_state__cont_flow(State *state, Action_Context context, FILE *out){
    switch (context){
        case ActionContext_Normal:
        {
            fprintf(out, "goto state_label_%d; // %.*s\n",
                    state->number, string_expand(state->pretty_name));
        }break;
        case ActionContext_EndOfFile:
        {
            fprintf(out, "goto end;\n");
        }break;
    }
}

internal void
gen_goto_dst_state__cont_flow(Transition *trans, Action_Context context, FILE *out){
    gen_goto_state__cont_flow(trans->dst_state, context, out);
}

internal void
gen_action__set_flag(Flag *flag, b32 value, FILE *out){
    if (flag != 0){
        if (value == 0){
            fprintf(out, "%.*s%d &= ~(0x%x);\n",
                    string_expand(flag->base_name), flag->index, flag->value);
        }
        else{
            fprintf(out, "%.*s%d |= 0x%x;\n",
                    string_expand(flag->base_name), flag->index, flag->value);
        }
    }
}

internal void
gen_emit__fill_token_flags(Flag_Set flags, Flag_Bucket_Set bucket_set, FILE *out){
    if (bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_AutoZero].count > 0){
        if (bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_KeepState].count > 0){
            fprintf(out, "token.flags = flag_ZB0 | flags_KB0;\n");
        }
        else{
            fprintf(out, "token.flags = flags_ZB0;\n");
        }
    }
    else{
        if (bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_KeepState].count > 0){
            fprintf(out, "token.flags = flags_KB0;\n");
        }
    }
    for (Flag *flag = flags.first;
         flag != 0;
         flag = flag->next){
        if (flag->emit_sub_flags != 0){
            fprintf(out, "if (");
            gen_flag_check__cont_flow(flag, true, out);
            fprintf(out, "){\n");
            fprintf(out, "token.sub_flags |= 0x%x;\n", flag->emit_sub_flags);
            fprintf(out, "}\n");
        }
    }
}

internal void
gen_emit__fill_token_base_kind(Token_Kind_Set tokens, String_Const_u8 name, FILE *out){
    Table_Lookup lookup = table_lookup(&tokens.name_to_ptr, make_data(name.str, name.size));
    Assert(lookup.found_match);
    u64 val = 0;
    table_read(&tokens.name_to_ptr, lookup, &val);
    Token_Kind_Node *node = (Token_Kind_Node*)IntAsPtr(val);
    Token_Base_Kind base_kind = node->base_kind;
    // TODO(allen): pretty names for token base kinds?
    fprintf(out, "token.kind = %u;\n", base_kind);
}

internal void
gen_emit__direct(Arena *scratch, Token_Kind_Set tokens, String_Const_u8 base_name, FILE *out){
    Temp_Memory temp = begin_temp(scratch);
    char *token_full_name = gen_token_full_name(scratch, base_name);
    fprintf(out, "token.sub_kind = %s;\n", token_full_name);
    gen_emit__fill_token_base_kind(tokens, base_name, out);
    end_temp(temp);
}

internal Action_Context
gen_SLOW_action_list__cont_flow(Arena *scratch, Token_Kind_Set tokens, Flag_Set flags,
                                Flag_Bucket_Set bucket_set, Action_List action_list,
                                Action_Context context, FILE *out){
    Action_Context result_context = ActionContext_Normal;
    for (Action *action = action_list.first;
         action != 0;
         action = action->next){
        switch (action->kind){
            case ActionKind_SetFlag:
            {
                gen_action__set_flag(action->set_flag.flag, action->set_flag.value, out);
            }break;
            
            case ActionKind_ZeroFlags:
            {
                for (i32 i = 0; i < FlagBindProperty_COUNT; i += 1){
                    Flag_Bucket *bucket = &bucket_set.buckets[i][FlagResetRule_AutoZero];
                    for (i32 j = 0; j < bucket->number_of_variables; j += 1){
                        fprintf(out, "%.*s%d = 0;\n", string_expand(bucket->pretty_name), j);
                    }
                }
            }break;
            
            case ActionKind_DelimMarkFirst:
            {
                fprintf(out, "delim_first = ptr;\n");
            }break;
            
            case ActionKind_DelimMarkOnePastLast:
            {
                fprintf(out, "delim_one_past_last = ptr;\n");
            }break;
            
            case ActionKind_Consume:
            {
                if (context != ActionContext_EndOfFile){
                    fprintf(out, "ptr += 1;\n");
                }
                else{
                    result_context = ActionContext_EndOfFile;
                }
            }break;
            
            case ActionKind_Emit:
            {
                Emit_Rule *emit = action->emit_rule;
                
                fprintf(out, "{\n");
                fprintf(out, "Token token = {};\n");
                
                fprintf(out, "token.pos = (i64)(emit_ptr - input.str);\n");
                fprintf(out, "token.size = (i64)(ptr - emit_ptr);\n");
                
                gen_emit__fill_token_flags(flags, bucket_set, out);
                
                fprintf(out, "do{\n");
                b32 keep_looping = true;
                for (Emit_Handler *handler = emit->first;
                     handler != 0 && keep_looping;
                     handler = handler->next){
                    if (handler->flag_check != 0){
                        fprintf(out, "if (");
                        gen_flag_check__cont_flow(handler->flag_check, true, out);
                        fprintf(out, "){\n");
                    }
                    
                    switch (handler->kind){
                        case EmitHandlerKind_Direct:
                        {
                            gen_emit__direct(scratch, tokens, handler->token_name, out);
                            if (handler->flag_check != 0){
                                fprintf(out, "break;\n");
                            }
                            keep_looping = false;
                        }break;
                        
                        case EmitHandlerKind_Keywords:
                        {
                            Keyword_Set *keywords = handler->keywords;
                            fprintf(out, "Lexeme_Table_Lookup lookup = "
                                    "lexeme_table_lookup(%.*s_hash_array, %.*s_key_array, "
                                    "%.*s_value_array, %.*s_slot_count, %.*s_seed, "
                                    "emit_ptr, token.size);\n",
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name));
                            fprintf(out, "if (lookup.found_match){\n");
                            fprintf(out, "token.kind = lookup.base_kind;\n");
                            fprintf(out, "token.sub_kind = lookup.sub_kind;\n");
                            fprintf(out, "break;\n");
                            fprintf(out, "}\n");
                            if (handler->keywords->has_fallback_token_kind){
                                gen_emit__direct(scratch, tokens,
                                                 keywords->fallback_name, out);
                                keep_looping = false;
                            }
                        }break;
                        
                        case EmitHandlerKind_KeywordsDelim:
                        {
                            Keyword_Set *keywords = handler->keywords;
                            fprintf(out, "Lexeme_Table_Lookup lookup = "
                                    "lexeme_table_lookup(%.*s_hash_array, %.*s_key_array, "
                                    "%.*s_value_array, %.*s_slot_count, %.*s_seed, "
                                    "delim_first, (delim_one_past_last - delim_first));\n",
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name),
                                    string_expand(keywords->pretty_name));
                            fprintf(out, "if (lookup.found_match){\n");
                            fprintf(out, "token.kind = lookup.base_kind;\n");
                            fprintf(out, "token.sub_kind = lookup.sub_kind;\n");
                            fprintf(out, "break;\n");
                            fprintf(out, "}\n");
                            if (handler->keywords->has_fallback_token_kind){
                                gen_emit__direct(scratch, tokens,
                                                 keywords->fallback_name, out);
                                keep_looping = false;
                            }
                        }break;
                    }
                    
                    if (handler->flag_check != 0){
                        fprintf(out, "}\n");
                        keep_looping = true;
                    }
                }
                fprintf(out, "}while(0);\n");
                
                if (emit->emit_checks.count > 0){
                    fprintf(out, "switch (token.sub_kind){\n");
                    for (Emit_Check *emit_check = emit->emit_checks.first;
                         emit_check != 0;
                         emit_check = emit_check->next){
                        Temp_Memory temp = begin_temp(scratch);
                        char *emit_check_full_name = gen_token_full_name(scratch, emit_check->emit_check);
                        fprintf(out, "case %s:\n", emit_check_full_name);
                        fprintf(out, "{\n");
                        gen_action__set_flag(emit_check->flag, emit_check->value, out);
                        fprintf(out, "}break;\n");
                        end_temp(temp);
                    }
                    fprintf(out, "}\n");
                }
                
                fprintf(out, "token_list_push(arena, &list, &token);\n");
                fprintf(out, "emit_ptr = ptr;\n");
                fprintf(out, "}\n");
            }break;
        }
    }
    return(result_context);
}

internal void
gen_flag_declarations__cont_flow(Flag_Bucket *bucket, FILE *out){
    i32 max_bits = bucket->max_bits;
    i32 number_of_flag_variables = (bucket->count + max_bits - 1)/max_bits;
    String_Const_u8 pretty_name = bucket->pretty_name;
    for (i32 i = 0; i < number_of_flag_variables; i += 1){
        fprintf(out, "u%d %.*s%d = 0;\n", max_bits, string_expand(pretty_name), i);
    }
    bucket->number_of_variables = number_of_flag_variables;
}

internal void
gen_bound_flag_fill_lookup__cont_flow(Flag_Bucket *bucket){
    i32 counter = 0;
    for (Flag_Ptr_Node *node = bucket->first;
         node != 0;
         node = node->next, counter += 1){
        Flag *flag = node->flag;
        flag->base_name = bucket->pretty_name;
        flag->number = counter;
        flag->index = 0;
        flag->value = flag->emit_flags;
    }
}

internal void
gen_flag_fill_lookup__cont_flow(Flag_Bucket *bucket){
    i32 max_bits = bucket->max_bits;
    i32 counter = 0;
    for (Flag_Ptr_Node *node = bucket->first;
         node != 0;
         node = node->next, counter += 1){
        Flag *flag = node->flag;
        flag->base_name = bucket->pretty_name;
        flag->number = counter;
        flag->index = counter/max_bits;
        flag->value = (1 << (counter % max_bits));
    }
}

internal void
gen_contiguous_control_flow_lexer(Arena *scratch, Token_Kind_Set tokens, Lexer_Model model, FILE *out){
    Temp_Memory temp = begin_temp(scratch);
    
    model = opt_copy_model(scratch, model);
    
    opt_flags_set_numbers(model);
    opt_states_set_numbers(model);
    
    Input_Set cut_inputs = smi_input_set_construct_eof(scratch);
    Field_Set cut_fields = smi_field_set_construct(scratch);
    Condition_Set cut_set = smi_condition(scratch, cut_inputs, cut_fields);
    
    // Split EOFs and insert at beginning
    for (State *state = model.states.first;
         state != 0;
         state = state->next){
        Transition_List *transitions = &state->transitions;
        if (transitions->first->condition.kind == TransitionCaseKind_ConditionSet){
            Transition *first = 0;
            Transition *last = 0;
            i32 count = 0;
            
            for (Transition *trans = transitions->first, *next = 0;
                 trans != 0;
                 trans = next){
                next = trans->next;
                
                Assert(trans->condition.kind == TransitionCaseKind_ConditionSet);
                Condition_Set original = trans->condition.condition_set;
                Condition_Set condition_int = smi_condition_set_intersect(scratch, original, cut_set);
                if (condition_int.count == 0){
                    zdll_push_back(first, last, trans);
                    count += 1;
                }
                else{
                    trans->condition.condition_set = condition_int;
                    zdll_push_front(first, last, trans);
                    count += 1;
                    
                    Condition_Set condition_sub = smi_condition_set_subtract(scratch, original, cut_set);
                    if (condition_sub.count > 0){
                        Transition *new_trans = push_array(scratch, Transition, 1);
                        zdll_push_back(first, last, new_trans);
                        count += 1;
                        new_trans->parent_state = state;
                        new_trans->condition.kind = TransitionCaseKind_ConditionSet;
                        new_trans->condition.condition_set = condition_sub;
                        new_trans->activation_actions = opt_copy_action_list(scratch, trans->activation_actions);
                        new_trans->dst_state = trans->dst_state;
                    }
                }
            }
            
            state->transitions.first = first;
            state->transitions.last = last;
            state->transitions.count = count;
        }
    }
    
    Flag_Bucket_Set bucket_set = {};
    bucket_set.buckets[FlagBindProperty_Free][FlagResetRule_AutoZero].pretty_name = string_u8_litexpr("flags_ZF");
    bucket_set.buckets[FlagBindProperty_Free][FlagResetRule_AutoZero].max_bits = 32;
    bucket_set.buckets[FlagBindProperty_Free][FlagResetRule_KeepState].pretty_name = string_u8_litexpr("flags_KF");
    bucket_set.buckets[FlagBindProperty_Free][FlagResetRule_KeepState].max_bits = 32;
    bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_AutoZero].pretty_name = string_u8_litexpr("flags_ZB");
    bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_AutoZero].max_bits = 16;
    bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_KeepState].pretty_name = string_u8_litexpr("flags_KB");
    bucket_set.buckets[FlagBindProperty_Bound][FlagResetRule_KeepState].max_bits = 16;
    
    for (Flag *flag = model.flags.first;
         flag != 0;
         flag = flag->next){
        Flag_Reset_Rule reset_rule = flag->reset_rule;
        Flag_Bind_Property bind_property =
            (flag->emit_flags != 0)?FlagBindProperty_Bound:FlagBindProperty_Free;
        
        Flag_Bucket *bucket = &bucket_set.buckets[bind_property][reset_rule];
        Flag_Ptr_Node *node = push_array(scratch, Flag_Ptr_Node, 1);
        sll_queue_push(bucket->first, bucket->last, node);
        bucket->count += 1;
        node->flag = flag;
    }
    
    for (i32 i = 0; i < FlagBindProperty_COUNT; i += 1){
        for (i32 j = 0; j < FlagResetRule_COUNT; j += 1){
            if (i == FlagBindProperty_Bound){
                gen_bound_flag_fill_lookup__cont_flow(&bucket_set.buckets[i][j]);
            }
            else{
                gen_flag_fill_lookup__cont_flow(&bucket_set.buckets[i][j]);
            }
        }
    }
    
    fprintf(out, "internal Token_List\n");
    fprintf(out, "lex_full_input_" LANG_NAME_LOWER_STR "(Arena *arena, String_Const_u8 input){\n");
    fprintf(out, "Token_List list = {};\n");
    
    for (i32 i = 0; i < FlagBindProperty_COUNT; i += 1){
        for (i32 j = 0; j < FlagResetRule_COUNT; j += 1){
            gen_flag_declarations__cont_flow(&bucket_set.buckets[i][j], out);
        }
    }
    
    fprintf(out, "u8 *delim_first = input.str;\n");
    fprintf(out, "u8 *delim_one_past_last = input.str;\n");
    
    fprintf(out, "u8 *emit_ptr = input.str;\n");
    
    fprintf(out, "u8 *ptr = input.str;\n");
    fprintf(out, "u8 *opl_ptr = ptr + input.size;\n");
    
    for (State *state = model.states.first;
         state != 0;
         state = state->next){
        fprintf(out, "{\n");
        fprintf(out, "state_label_%d: // %.*s\n",
                state->number, string_expand(state->pretty_name));
        
        Transition_List *transitions = &state->transitions;
        Transition *trans = transitions->first;
        
        Transition_Case_Kind state_trans_kind = trans->condition.kind;
        
        switch (state_trans_kind){
            default:
            {
                InvalidPath;
            }break;
            
            case TransitionCaseKind_DelimMatch:
            {
                Transition *success_trans = trans;
                Transition *failure_trans = trans->next;
                Assert(failure_trans->condition.kind == TransitionCaseKind_DelimMatchFail);
                
                fprintf(out, "umem delim_length = delim_one_past_last - delim_first;\n");
                fprintf(out, "umem parse_length = 0;\n");
                fprintf(out, "for (;;){\n");
                {
                    fprintf(out, "if (parse_length == delim_length){\n");
                    {
                        gen_SLOW_action_list__cont_flow(scratch, tokens, model.flags, bucket_set,
                                                        success_trans->activation_actions, 
                                                        ActionContext_Normal, out);
                        gen_goto_dst_state__cont_flow(success_trans, ActionContext_Normal, out);
                    }
                    fprintf(out, "}\n");
                    fprintf(out, "if (ptr == opl_ptr){\n");
                    {
                        gen_SLOW_action_list__cont_flow(scratch, tokens, model.flags, bucket_set,
                                                        failure_trans->activation_actions,
                                                        ActionContext_Normal, out);
                        gen_goto_dst_state__cont_flow(success_trans, ActionContext_Normal, out);
                    }
                    fprintf(out, "}\n");
                    
                    fprintf(out, "if (*ptr == delim_first[parse_length]){\n");
                    fprintf(out, "ptr += 1;\n");
                    fprintf(out, "parse_length += 1;\n");
                    fprintf(out, "}\n");
                    fprintf(out, "else{\n");
                    {
                        gen_SLOW_action_list__cont_flow(scratch, tokens, model.flags, bucket_set,
                                                        failure_trans->activation_actions,
                                                        ActionContext_Normal, out);
                        gen_goto_dst_state__cont_flow(failure_trans, ActionContext_Normal, out);
                    }
                    fprintf(out, "}\n");
                }
                fprintf(out, "}\n");
            }break;
            
            case TransitionCaseKind_ConditionSet:
            {
                {
                    fprintf(out, "if (ptr == opl_ptr){\n");
                    for (;
                         trans != 0;
                         trans = trans->next){
                        if (opt_condition_is_eof_only(trans->condition)){
                            Assert(trans->condition.condition_set.count == 1);
                            Condition_Node *node = trans->condition.condition_set.first;
                            fprintf(out, "if (");
                            gen_SLOW_field_set_check__cont_flow(node->fields, out);
                            fprintf(out, "){\n");
                            Action_Context action_ctx = ActionContext_EndOfFile;
                            action_ctx = gen_SLOW_action_list__cont_flow(scratch, tokens, model.flags,
                                                                         bucket_set,
                                                                         trans->activation_actions,
                                                                         action_ctx, out);
                            gen_goto_dst_state__cont_flow(trans, action_ctx, out);
                            fprintf(out, "}\n");
                        }
                        else{
                            break;
                        }
                    }
                    fprintf(out, "}\n");
                }
                
                Grouped_Input_Handler_List group_list = opt_grouped_input_handlers(scratch, trans);
                
                fprintf(out, "switch (*ptr){\n");
                for (Grouped_Input_Handler *group = group_list.first;
                     group != 0;
                     group = group->next){
                    
                    if (group == group_list.group_with_biggest_input_set){
                        fprintf(out, "default:\n");
                    }
                    else{
                        i32 input_count = group->input_count;
                        u8 *inputs = group->inputs;
                        for (i32 i = 0; i < input_count; i += 1){
                            fprintf(out, "case 0x%02x:", inputs[i]);
                            if ((i % 7) == 6 || i + 1 == input_count){
                                fprintf(out, "\n");
                            }
                        }
                    }
                    
                    fprintf(out, "{\n");
                    for (Partial_Transition *partial = group->partial_transitions.first;
                         partial != 0;
                         partial = partial->next){
                        if (partial->next != 0){
                            fprintf(out, "if (");
                            gen_SLOW_field_set_check__cont_flow(partial->fields, out);
                            fprintf(out, "){\n");
                        }
                        
                        {
                            gen_SLOW_action_list__cont_flow(scratch, tokens, model.flags, bucket_set,
                                                            partial->actions, ActionContext_Normal,
                                                            out);
                            gen_goto_state__cont_flow(partial->dst_state, ActionContext_Normal, out);
                        }
                        
                        if (partial->next != 0){
                            fprintf(out, "}\n");
                        }
                    }
                    fprintf(out, "}break;\n");
                }
                fprintf(out, "}\n");
            }break;
        }
        
        fprintf(out, "}\n");
    }
    
    fprintf(out, "end:;\n");
    fprintf(out, "return(list);\n");
    fprintf(out, "}\n");
    
    end_temp(temp);
}

////////////////////////////////

#include <stdio.h>
#include <time.h>

internal void
build_language_model(void);

internal String_Const_u8
file_read_all(Arena *arena, FILE *file){
    String_Const_u8 result = {};
    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    fseek(file, 0, SEEK_SET);
    result.str = push_array(arena, u8, result.size + 1);
    fread(result.str, result.size, 1, file);
    result.str[result.size] = 0;
    return(result);
}

int main(void){
    pcg32_srandom(time(0), time(0));
    
    Base_Allocator *allocator = get_allocator_malloc();
    sm_helper_init(allocator);
    
    build_language_model();
    
    Lexer_Primary_Context *ctx = &helper_ctx.primary_ctx;
    
    // NOTE(allen): Type checking
    // DelimMatch only with a single catch-all fallback, no peeks.
    // Remove the declaration of states and flags?
    // Flag bindings are one to one
    
    ////////////////////////////////
    
    // NOTE(allen): High level reorganization of state machine
    
    opt_set_auto_zero_flags_on_root(ctx);
    opt_transfer_state_actions_to_transitions(ctx);
    
    ////////////////////////////////
    
    // NOTE(allen): High level optimization
    
    opt_simplify_transitions(ctx);
    
    opt_mark_all_states_excluded(ctx);
    opt_include_reachable_states(ctx->model.root);
    opt_discard_all_excluded_states(ctx);
    
    opt_merge_redundant_transitions_in_each_state(ctx);
    
    opt_skip_past_thunk_states(ctx);
    
    opt_mark_all_states_excluded(ctx);
    opt_include_reachable_states(ctx->model.root);
    opt_discard_all_excluded_states(ctx);
    
    opt_remove_peeks_without_creating_transition_splits(ctx);
    
    opt_mark_all_states_excluded(ctx);
    opt_include_reachable_states(ctx->model.root);
    opt_discard_all_excluded_states(ctx);
    
    opt_remove_peeks_into_single_entry_point_states(ctx);
    
    opt_discard_all_excluded_states(ctx);
    
    opt_states_set_numbers(ctx->model);
    
    ////////////////////////////////
    
    // NOTE(allen): Debug inspection of model
    
#if 0    
    opt_flags_set_numbers(ctx->model);
    debug_print_transitions(ctx);
#endif
    
    ////////////////////////////////
    
    // NOTE(allen): Arrange input files and output files
    
    String_Const_u8 path_to_self = string_u8_litexpr(__FILE__);
    path_to_self = string_remove_last_folder(path_to_self);
    
    String_Const_u8 hand_written_h_name = push_u8_stringf(&ctx->arena,
                                                          "%.*s4coder_lex_gen_hand_written.h",
                                                          string_expand(path_to_self));
    String_Const_u8 hand_written_name = push_u8_stringf(&ctx->arena,
                                                        "%.*s4coder_lex_gen_hand_written.cpp",
                                                        string_expand(path_to_self));
    
    
    FILE *hand_written_h_file = fopen((char*)hand_written_h_name.str, "rb");
    if (hand_written_h_file == 0){
        printf("error: could not open 4coder_lex_gen_hand_written.h\n");
        exit(1);
    }
    
    String_Const_u8 hand_written_h = file_read_all(&ctx->arena, hand_written_h_file);
    fclose(hand_written_h_file);
    
    FILE *hand_written_file = fopen((char*)hand_written_name.str  , "rb");
    if (hand_written_file == 0){
        printf("error: could not open 4coder_lex_gen_hand_written.cpp\n");
        exit(1);
    }
    
    String_Const_u8 hand_written = file_read_all(&ctx->arena, hand_written_file);
    fclose(hand_written_file);
    
    String_Const_u8 path_to_src = string_remove_last_folder(path_to_self);
    
    String_Const_u8 out_h_name = push_u8_stringf(&ctx->arena, "%.*sgenerated/lexer_" LANG_NAME_LOWER_STR ".h",
                                                 string_expand(path_to_src));
    String_Const_u8 out_cpp_name = push_u8_stringf(&ctx->arena, "%.*sgenerated/lexer_" LANG_NAME_LOWER_STR ".cpp",
                                                   string_expand(path_to_src));
    
    FILE *out_h_file = fopen((char*)out_h_name.str, "wb");
    if (out_h_file == 0){
        printf("error: could not open output file %.*s\n", string_expand(out_h_name));
        exit(1);
    }
    
    FILE *out_cpp_file = fopen((char*)out_cpp_name.str, "wb");
    if (out_cpp_file == 0){
        printf("error: could not open output file %.*s\n", string_expand(out_cpp_name));
        exit(1);
    }
    
    ////////////////////////////////
    
    // NOTE(allen): Code generation
    
    fprintf(out_h_file, "%s\n", hand_written_h.str);
    gen_tokens(&ctx->arena, ctx->tokens, out_h_file);
    
    fprintf(out_cpp_file, "%s\n", hand_written.str);
    for (Keyword_Set *set = ctx->keywords.first;
         set != 0;
         set = set->next){
        gen_keyword_table(&ctx->arena, ctx->tokens, *set, out_cpp_file);
    }
    gen_contiguous_control_flow_lexer(&ctx->arena, ctx->tokens, ctx->model, out_cpp_file);
    
    fclose(out_h_file);
    fclose(out_cpp_file);
    
    printf("%.*s:1:\n", string_expand(out_h_name));
    printf("%.*s:1:\n", string_expand(out_cpp_name));
    
    // NOTE(allen): Simplifying the state machine
    // Isolate the state machine's parts into small L.U.T. then generate tables?
    // If using L.U.T: Optimize all action lists that don't contain a "consume" action
    
    // NOTE(allen): State machine generation
    // Implementation: Control Flow
    // Feature: Fully Contiguous input
    // 
    // Implementation: L.U.T. Accelerated
    // 
    // Feature: Spatially chunked input
    // Feature: Temporally chunked input
    return(0);
}

// BOTTOM

