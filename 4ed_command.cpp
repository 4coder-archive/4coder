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
    u64 hash;
};
static Command_Binding null_command_binding = {0};

struct Command_Map{
    Command_Map *parent;
    Command_Binding vanilla_keyboard_default;
    Command_Binding *commands;
    u32 count, max;
};

#define COMMAND_HASH_EMPTY 0
#define COMMAND_HASH_ERASED max_u64

internal void command_null(Command_Data *command);

internal u64
map_hash(Key_Code event_code, u8 modifiers){
    u64 result = (event_code << 8) | modifiers;
    return(result);
}

internal b32
map_add(Command_Map *map, Key_Code event_code, u8 modifiers, Command_Function function, Custom_Command_Function *custom = 0, b32 override_original = true){
    b32 result = false;
    Assert(map->count * 8 < map->max * 7);
    u64 hash = map_hash(event_code, modifiers);
    
    u32 max = map->max;
    u32 index = hash % max;
    Command_Binding entry = map->commands[index];
    while (entry.function != 0 && entry.hash != COMMAND_HASH_ERASED){
        if (entry.hash == hash){
            result = true;
            break;
        }
        index = (index + 1) % max;
        entry = map->commands[index];
    }
    
    if (override_original || !result){
        Command_Binding bind;
        bind.function = function;
        bind.custom = custom;
        bind.hash = hash;
        map->commands[index] = bind;
        ++map->count;
    }
    
    return(result);
}

inline b32
map_add(Command_Map *map, Key_Code event_code, u8 modifiers, Command_Function function, u64 custom_id, b32 override_original = true){
    return (map_add(map, event_code, modifiers, function, (Custom_Command_Function*)custom_id, override_original));
}

internal b32
map_find_entry(Command_Map *map, Key_Code event_code, u8 modifiers, u32 *index_out){
    u64 hash = map_hash(event_code, modifiers);
    u32 max = map->max;
    u32 index = hash % max;
    b32 result = false;
    Command_Binding entry = map->commands[index];
    while (entry.function != 0){
        if (entry.hash == hash){
            *index_out = index;
            result = true;
            break;
        }
        index = (index + 1) % max;
        entry = map->commands[index];
    }
    return(result);
}

internal b32
map_find(Command_Map *map, Key_Code event_code, u8 modifiers, Command_Binding *bind_out){
    u32 index = 0;
    b32 result = map_find_entry(map, event_code, modifiers, &index);
    if (result){
        *bind_out = map->commands[index];
    }
    return(result);
}

internal b32
map_drop(Command_Map *map, Key_Code event_code, u8 modifiers){
    u32 index = 0;
    b32 result = map_find_entry(map, event_code, modifiers, &index);
    if (result){
        map->commands[index].function = 0;
        map->commands[index].hash = COMMAND_HASH_ERASED;
    }
    return(result);
}

internal void
map_clear(Command_Map *commands){
    u32 max = commands->max;
    memset(commands->commands, 0, max*sizeof(*commands->commands));
    commands->vanilla_keyboard_default = null_command_binding;
    commands->count = 0;
}

internal void
map_init(Command_Map *commands, Partition *part, u32 max, Command_Map *parent){
    if (commands->commands == 0){
        max = clamp_bottom((u32)6, max);
        commands->parent = parent;
        commands->commands = push_array(part, Command_Binding, max);
        commands->max = max;
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
    
    Key_Code code = key.character_no_caps_lock;
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
    Command_Binding cmd_bind = {0};
    Command_Map *visited_maps[16] = {0};
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
            else{
                map = 0;
            }
        }
        else{
            map = 0;
        }
    }
    
    return(cmd_bind);
}

// BOTTOM

