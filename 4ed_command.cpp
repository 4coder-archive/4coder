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

typedef Command_Function_Sig(Command_Function);

struct Command_Binding{
    Command_Function *function;
    union{
        Custom_Command_Function *custom;
        u64 custom_id;
    };
    u64 hash;
};
static Command_Binding null_command_binding = {0};

struct Command_Map{
    i32 parent;
    Command_Binding vanilla_keyboard_default[1 << MDFR_INDEX_BINDABLE_COUNT];
    Command_Binding *commands;
    i32 count, max;
    void *real_beginning;
};

struct Mapping{
    void *memory;
    
    Command_Map map_top;
    Command_Map map_file;
    Command_Map map_ui;
    
    i32 *map_id_table;
    Command_Map *user_maps;
    i32 user_map_count;
};

internal i32
get_or_add_map_index(Mapping *mapping, i32 mapid){
    i32 user_map_count = mapping->user_map_count;
    i32 *map_id_table = mapping->map_id_table;
    i32 result = 0;
    for (; result < user_map_count; ++result){
        if (map_id_table[result] == mapid){
            break;
        }
        if (map_id_table[result] == -1){
            map_id_table[result] = mapid;
            break;
        }
    }
    return(result);
}

// HACK(allen): This seems busted, investigate.
internal i32
get_map_index(Mapping *mapping, i32 mapid){
    i32 user_map_count = mapping->user_map_count;
    i32 *map_id_table = mapping->map_id_table;
    i32 result = 0;
    for (; result < user_map_count; ++result){
        if (map_id_table[result] == mapid){
            break;
        }
        if (map_id_table[result] == 0){
            result = user_map_count;
            break;
        }
    }
    return(result);
}

internal Command_Map*
get_map_base(Mapping *mapping, i32 mapid, b32 add){
    Command_Map *map = 0;
    if (mapid < mapid_global){
        i32 map_index = 0;
        if (add){
            map_index = get_or_add_map_index(mapping, mapid);
        }
        else{
            map_index = get_map_index(mapping, mapid);
        }
        if (map_index < mapping->user_map_count){
            map = &mapping->user_maps[map_index];
        }
    }
    else if (mapid == mapid_global){
        map = &mapping->map_top;
    }
    else if (mapid == mapid_file){
        map = &mapping->map_file;
    }
    else if (mapid == mapid_ui){
        map = &mapping->map_ui;
    }
    return(map);
}

internal Command_Map*
get_or_add_map(Mapping *mapping, i32 mapid){
    Command_Map *map = get_map_base(mapping, mapid, true);
    return(map);
}

internal Command_Map*
get_map(Mapping *mapping, i32 mapid){
    Command_Map *map = get_map_base(mapping, mapid, false);
    return(map);
}

#define COMMAND_HASH_EMPTY 0
#define COMMAND_HASH_ERASED max_u64

internal void command_null(Command_Data *command);

internal u64
map_hash(Key_Code event_code, u8 modifiers){
    u64 result = (event_code << 8) | modifiers;
    return(result);
}

internal b32
map_add(Command_Map *map, Key_Code event_code, u8 modifiers, Command_Function *function, Custom_Command_Function *custom = 0, b32 override_original = true){
    b32 result = false;
    Assert(map->count * 8 < map->max * 7);
    u64 hash = map_hash(event_code, modifiers);
    
    u32 max = map->max;
    u32 index = hash % max;
    Command_Binding entry = map->commands[index];
    for (; entry.function != 0 && entry.hash != COMMAND_HASH_ERASED;){
        if (entry.hash == hash){
            result = true;
            break;
        }
        index = (index + 1) % max;
        entry = map->commands[index];
    }
    
    if (override_original || !result){
        Command_Binding bind = {0};
        bind.function = function;
        bind.custom = custom;
        bind.hash = hash;
        map->commands[index] = bind;
        if (!result){
            ++map->count;
        }
    }
    
    return(result);
}

inline b32
map_add(Command_Map *map, Key_Code event_code, u8 modifiers, Command_Function *function, u64 custom_id, b32 override_original = true){
    return (map_add(map, event_code, modifiers, function, (Custom_Command_Function*)custom_id, override_original));
}

internal b32
map_find_entry(Command_Map *map, Key_Code event_code, u8 modifiers, u32 *index_out){
    u64 hash = map_hash(event_code, modifiers);
    u32 max = map->max;
    u32 index = hash % max;
    b32 result = false;
    Command_Binding entry = map->commands[index];
    for (; entry.function != 0;){
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
map_init(Command_Map *map, Partition *part, i32 max, i32 parent){
    Assert(max >= 6);
    Assert(map->commands == 0);
    map->parent = parent;
    map->commands = push_array(part, Command_Binding, max);
    map->count = 0;
    map->max = max;
    
    memset(map->commands, 0, max*sizeof(*map->commands));
    memset(map->vanilla_keyboard_default, 0, sizeof(map->vanilla_keyboard_default));
}

internal b32
map_get_modifiers_hash(u8 modifiers, u32 *hash_out){
    b32 result = true;
    u32 hash = 0;
    if (modifiers & MDFR_SHIFT){
        hash += MDFR_SHIFT;
    }
    if (modifiers & MDFR_CTRL){
        hash += MDFR_CTRL;
    }
    if (modifiers & MDFR_CMND){
        hash += MDFR_CMND;
    }
    if (modifiers & MDFR_ALT){
        hash += MDFR_ALT;
    }
    *hash_out = hash;
    return(result);
}

internal void
map_get_vanilla_keyboard_default(Command_Map *map, u8 command, Command_Binding *bind_out){
    u32 modifiers_hashed = 0;
    if (map_get_modifiers_hash(command, &modifiers_hashed)){
        *bind_out = map->vanilla_keyboard_default[modifiers_hashed];
    }
}

internal Command_Binding
map_extract(Command_Map *map, Key_Event_Data key){
    Command_Binding bind = {0};
    
    b32 ctrl    = key.modifiers[MDFR_CONTROL_INDEX];
    b32 alt     = key.modifiers[MDFR_ALT_INDEX];
    b32 command = key.modifiers[MDFR_COMMAND_INDEX];
    b32 shift   = key.modifiers[MDFR_SHIFT_INDEX];
    
    u8 mod_flags = MDFR_NONE;
    if (ctrl)    mod_flags |= MDFR_CTRL;
    if (alt)     mod_flags |= MDFR_ALT;
    if (command) mod_flags |= MDFR_CMND;
    if (shift)   mod_flags |= MDFR_SHIFT;
    
    Key_Code code = key.character_no_caps_lock;
    if (code == 0){
        code = key.keycode;
        map_find(map, code, mod_flags, &bind);
    }
    else{
        if (code != '\n' && code != '\t' && code != ' '){
            mod_flags &= ~(MDFR_SHIFT);
        }
        map_find(map, code, mod_flags, &bind);
        if (bind.function == 0){
            map_get_vanilla_keyboard_default(map, mod_flags, &bind);
        }
    }
    
    return(bind);
}

internal Command_Binding
map_extract_recursive(Mapping *mapping, i32 map_id, Key_Event_Data key){
    Command_Map *map = get_map(mapping, map_id);
    if (map == 0){
        map = &mapping->map_top;
    }
    
    Command_Map *visited_maps[16] = {0};
    i32 visited_top = 0;
    
    Command_Binding cmd_bind = {0};
    for (; map != 0; ){
        cmd_bind = map_extract(map, key);
        if (cmd_bind.function == 0){
            if (visited_top < ArrayCount(visited_maps)){
                visited_maps[visited_top++] = map;
                map = get_map(mapping, map->parent);
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

