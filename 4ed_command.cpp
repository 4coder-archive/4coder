/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Command management functions for 4coder
 *
 */

// TOP

#define Command_Function_Sig(name) \
void (name)(System_Functions *system, struct Command_Data *command, struct Command_Binding binding)

typedef Command_Function_Sig(*Command_Function);

struct Command_Binding{
    Command_Function function;
    union{
        Custom_Command_Function *custom;
        u64 custom_id;
    };
    i64 hash;
};
static Command_Binding null_command_binding = {0};

struct Command_Map{
    Command_Map *parent;
    Command_Binding vanilla_keyboard_default;
    Command_Binding *commands;
    i32 count, max;
};

internal void command_null(Command_Data *command);

internal i64
map_hash(u16 event_code, u8 modifiers){
    i64 result = (event_code << 8) | modifiers;
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

inline b32
map_add(Command_Map *map, u16 event_code, u8 modifiers, Command_Function function, u64 custom_id){
    return (map_add(map, event_code, modifiers, function, (Custom_Command_Function*)custom_id));
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
map_clear(Command_Map *commands){
    i32 max = commands->max;
    memset(commands->commands, 0, max*sizeof(*commands->commands));
    commands->vanilla_keyboard_default = null_command_binding;
    commands->count = 0;
}

internal void
map_init(Command_Map *commands, Partition *part, i32 max, Command_Map *parent){
    if (commands->commands == 0){
        max = ((max < 6)?(6):(max));
        commands->parent = parent;
        commands->commands = push_array(part, Command_Binding, max);
        commands->max = max;
        map_clear(commands);
    }
}

internal void
map_get_vanilla_keyboard_default(Command_Map *map, u8 command, Command_Binding *bind_out){
    if (command == MDFR_NONE){
        *bind_out = map->vanilla_keyboard_default;
    }
}

internal Command_Binding
map_extract(Command_Map *map, Key_Event_Data key){
    Command_Binding bind = {0};
    
    b32 ctrl = key.modifiers[MDFR_CONTROL_INDEX];
    b32 alt = key.modifiers[MDFR_ALT_INDEX];
    b32 shift = key.modifiers[MDFR_SHIFT_INDEX];
    u8 command = MDFR_NONE;
    
    if (shift) command |= MDFR_SHIFT;
    if (ctrl) command |= MDFR_CTRL;
    if (alt) command |= MDFR_ALT;
    
    u16 code = key.character_no_caps_lock;
    if (code == 0){
        code = key.keycode;
        map_find(map, code, command, &bind);
    }
    else{
        if (code != '\n' && code != '\t' && code != ' '){
            command &= ~(MDFR_SHIFT);
        }
        map_find(map, code, command, &bind);
        if (bind.function == 0){
            map_get_vanilla_keyboard_default(map, command, &bind);
        }
    }
    
    return(bind);
}

internal Command_Binding
map_extract_recursive(Command_Map *map, Key_Event_Data key){
    Command_Binding cmd_bind = {};
    Command_Map *visited_maps[16] = {};
    i32 visited_top = 0;
    
    while (map){
        cmd_bind = map_extract(map, key);
        if (cmd_bind.function == 0){
            if (visited_top < ArrayCount(visited_maps)){
                visited_maps[visited_top++] = map;
                map = map->parent;
                for (i32 i = 0; i < visited_top; ++i){
                    if (map == visited_maps[i]){
                        map = 0;
                        break;
                    }
                }
            }
            else map = 0;
        }
        else map = 0;
    }
    
    return(cmd_bind);
}

// BOTTOM

