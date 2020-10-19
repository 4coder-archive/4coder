/*
4coder_command_map.h - Command management types
*/

// TOP

#if !defined(FCODER_CODEPOINT_MAP_H)
#define FCODER_CODEPOINT_MAP_H

typedef i64 Command_Map_ID;

struct Command_Trigger{
    Command_Trigger *next;
    Input_Event_Kind kind;
    u32 sub_code;
    Input_Modifier_Set mods;
};

struct Command_Trigger_List{
    Command_Trigger *first;
    Command_Trigger *last;
};

struct Command_Binding{
    union{
        Custom_Command_Function *custom;
        char *name;
    };
    
    Command_Binding();
    Command_Binding(Custom_Command_Function *c);
    Command_Binding(char *n);
    
    operator Custom_Command_Function*();
    operator char*();
};

struct Command_Modified_Binding{
    Command_Modified_Binding *next;
    SNode order_node;
    Input_Modifier_Set mods;
    Command_Binding binding;
};

struct Command_Binding_List{
    Command_Binding_List *next;
    SNode *first;
    SNode *last;
    i32 count;
};

struct Command_Map{
    Command_Map *next;
    Command_Map *prev;
    Command_Map_ID id;
    Command_Map_ID parent;
    Command_Binding text_input_command;
    Arena node_arena;
    Table_u64_u64 event_code_to_binding_list;
    Table_u64_u64 cmd_to_binding_trigger;
    Command_Modified_Binding *binding_first;
    Command_Modified_Binding *binding_last;
    Command_Binding_List *list_first;
    Command_Binding_List *list_last;
    
    struct Binding_Unit *real_beginning;
};

struct Mapping{
    Arena node_arena;
    Heap heap;
    Base_Allocator heap_wrapper;
    Table_u64_u64 id_to_map;
    Command_Map_ID id_counter;
    Command_Map *first_map;
    Command_Map *last_map;
    Command_Map *free_maps;
    Command_Modified_Binding *free_bindings;
    Command_Binding_List *free_lists;
};

typedef i32 Binding_Match_Rule;
enum{
    BindingMatchRule_Strict,
    BindingMatchRule_Loose,
};

struct Map_Event_Breakdown{
    Input_Modifier_Set *mod_set;
    u64 key;
    Key_Code skip_self_mod;
};

#endif

// BOTTOM
