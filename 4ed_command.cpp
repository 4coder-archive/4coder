/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Command management functions for 4coder
 *
 */

// TOP

typedef void (*Command_Function)(System_Functions *system,
                                 struct Command_Data *command, struct Command_Binding binding);

struct Command_Binding{
    Command_Function function;
    Custom_Command_Function *custom;
    i64 hash;
};

struct Command_Map{
    Command_Map *parent;
    Command_Binding vanilla_keyboard_default;
    Command_Binding *commands;
    i32 count, max;
};

internal void command_null(Command_Data *command);

internal i64
map_hash(u16 event_code, u8 modifiers){
    i64 result = (event_code << 4) | modifiers;
    return result;
}

internal b32
map_add(Command_Map *map, u16 event_code, u8 modifiers, Command_Function function, Custom_Command_Function *custom = 0){
    Assert(map->count * 8 < map->max * 7);
    Command_Binding bind;
    bind.function = function;
    bind.custom = custom;
    bind.hash = map_hash(event_code, modifiers);
    
    i32 max = map->max;
    i32 index = bind.hash % max;
    Command_Binding entry;
    while ((entry = map->commands[index]).function && entry.hash != -1){
        if (entry.hash == bind.hash){
            return 1;
        }
        index = (index + 1) % max;
    }
    map->commands[index] = bind;
    ++map->count;
    return 0;
}

internal b32
map_find_entry(Command_Map *map, u16 event_code, u8 modifiers, i32 *index_out){
    i64 hash = map_hash(event_code, modifiers);
    i32 max = map->max;
    i32 index = hash % map->max;
    Command_Binding entry;
    while ((entry = map->commands[index]).function){
        if (entry.hash == hash){
            *index_out = index;
            return 1;
        }
        index = (index + 1) % max;
    }
    return 0;
}

internal b32
map_find(Command_Map *map, u16 event_code, u8 modifiers, Command_Binding *bind_out){
    b32 result;
    i32 index;
    result = map_find_entry(map, event_code, modifiers, &index);
    if (result){
        *bind_out = map->commands[index];
    }
    return result;
}

internal b32
map_drop(Command_Map *map, u16 event_code, u8 modifiers){
    b32 result;
    i32 index;
    result = map_find_entry(map, event_code, modifiers, &index);
    if (result){
        map->commands[index].function = 0;
        map->commands[index].hash = -1;
    }
    return result;
}

internal void
map_init(Command_Map *commands, Partition *part, i32 max, Command_Map *parent){
    commands->parent = parent;
    commands->commands = push_array(part, Command_Binding, max);
    memset(commands->commands, 0, max*sizeof(*commands->commands));
    commands->vanilla_keyboard_default = {};
    commands->max = max;
    commands->count = 0;
}

internal void
map_get_vanilla_keyboard_default(Command_Map *map, u8 command, Command_Binding *bind_out){
    if (command == MDFR_NONE){
        *bind_out = map->vanilla_keyboard_default;
    }
}

internal Command_Binding
map_extract(Command_Map *map, Key_Single key){
    Command_Binding bind = {};
    
    u8 command = MDFR_NONE;
    b32 ctrl = key.modifiers[CONTROL_KEY_CONTROL];
    b32 alt = key.modifiers[CONTROL_KEY_ALT];
    b32 shift = key.modifiers[CONTROL_KEY_SHIFT] && key.key.loose_keycode;
    
    if (shift) command |= MDFR_SHIFT;
    if (ctrl) command |= MDFR_CTRL;
    if (alt) command |= MDFR_ALT;
    
    u16 code = key.key.character_no_caps_lock;
    if (code == 0) code = key.key.keycode;
    map_find(map, code, command, &bind);
    
    if (bind.function == 0 && key.key.character_no_caps_lock != 0){
        map_get_vanilla_keyboard_default(map, command, &bind);
    }
    
    return bind;
}

// BOTTOM

