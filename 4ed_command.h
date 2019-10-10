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

struct Command_Binding{
    Custom_Command_Function *custom;
    u64 hash;
};

struct Command_Map{
    i32 parent;
    Command_Binding vanilla_keyboard_default[1 << MDFR_INDEX_BINDABLE_COUNT];
    Command_Binding *commands;
    i32 count;
    i32 max;
    void *real_beginning;
};

struct Mapping{
    void *memory;
    
    Command_Map map_top;
    Command_Map map_file;
    
    i32 *map_id_table;
    Command_Map *user_maps;
    i32 user_map_count;
};

#endif

// BOTTOM

