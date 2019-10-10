/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2015
 *
 * Command representation structures
 *
 */

// TOP

#if !defined(FRED_COMMAND_H)
#define FRED_COMMAND_H

union Command_Binding{
    Custom_Command_Function *custom;
};

struct Command_Modified_Binding{
    Command_Modified_Binding *next;
    SNode order_node;
    Key_Modifiers modifiers;
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
    Command_Map_ID id;
    Command_Map_ID parent;
    Command_Binding text_input_command;
    Table_u64_u64 key_code_to_binding_list;
    Command_Modified_Binding *binding_first;
    Command_Modified_Binding *binding_last;
    Command_Binding_List *list_first;
    Command_Binding_List *list_last;
    
    struct Binding_Unit *real_beginning;
};

struct Mapping{
    Arena *node_arena;
    Heap heap;
    Base_Allocator heap_wrapper;
    Table_u64_u64 id_to_map;
    Command_Map_ID id_counter;
    Command_Map *free_maps;
    Command_Modified_Binding *free_bindings;
    Command_Binding_List *free_lists;
};

#endif

// BOTTOM

