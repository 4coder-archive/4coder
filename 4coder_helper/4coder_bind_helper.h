/*
* Helpers for setting bindings.
*/

// TOP

#if !defined(FCODER_BIND_HELPER_H)
#define FCODER_BIND_HELPER_H

//
// Binding Helper
//

struct Bind_Helper{
    Binding_Unit *cursor, *start, *end;
    Binding_Unit *header, *group;
    int32_t write_total;
    int32_t error;
};

#define BH_ERR_NONE 0
#define BH_ERR_MISSING_END 1
#define BH_ERR_MISSING_BEGIN 2
#define BH_ERR_OUT_OF_MEMORY 3

inline Binding_Unit*
write_unit(Bind_Helper *helper, Binding_Unit unit){
    Binding_Unit *p = 0;
    helper->write_total += sizeof(*p);
    if (helper->error == 0 && helper->cursor != helper->end){
        p = helper->cursor++;
        *p = unit;
    }
    return p;
}

inline Bind_Helper
begin_bind_helper(void *data, int32_t size){
    Bind_Helper result;
    
    result.header = 0;
    result.group = 0;
    result.write_total = 0;
    result.error = 0;
    
    result.cursor = (Binding_Unit*)data;
    result.start = result.cursor;
    result.end = result.start + size / sizeof(*result.cursor);
    
    Binding_Unit unit = {0};
    unit.type = unit_header;
    unit.header.total_size = sizeof(*result.header);
    result.header = write_unit(&result, unit);
    result.header->header.user_map_count = 0;
    
    return result;
}

inline void
begin_map_(Bind_Helper *helper, int32_t mapid, int32_t replace){
    if (helper->group != 0 && helper->error == 0) helper->error = BH_ERR_MISSING_END;
    if (!helper->error && mapid < mapid_global) ++helper->header->header.user_map_count;
    
    Binding_Unit unit;
    unit.type = unit_map_begin;
    unit.map_begin.mapid = mapid;
    unit.map_begin.replace = replace;
    helper->group = write_unit(helper, unit);
    helper->group->map_begin.bind_count = 0;
}

inline void
begin_map(Bind_Helper *helper, int32_t mapid){
    begin_map_(helper, mapid, 0);
}

inline void
restart_map(Bind_Helper *helper, int32_t mapid){
    begin_map_(helper, mapid, 1);
}

inline void
end_map(Bind_Helper *helper){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    helper->group = 0;
}

inline void
bind(Bind_Helper *helper, Key_Code code, uint8_t modifiers, Command_ID cmdid){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    if (!helper->error) ++helper->group->map_begin.bind_count;
    
    Binding_Unit unit;
    unit.type = unit_binding;
    unit.binding.command_id = cmdid;
    unit.binding.code = code;
    unit.binding.modifiers = modifiers;
    
    write_unit(helper, unit);
}

inline void
bind(Bind_Helper *helper, Key_Code code, uint8_t modifiers, Custom_Command_Function *func){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    if (!helper->error) ++helper->group->map_begin.bind_count;
    
    Binding_Unit unit;
    unit.type = unit_callback;
    unit.callback.func = func;
    unit.callback.code = code;
    unit.callback.modifiers = modifiers;
    
    write_unit(helper, unit);
}

inline void
bind(Bind_Helper *helper, Key_Code code, uint8_t modifiers, Generic_Command cmd){
    if (cmd.cmdid < cmdid_count){
        bind(helper, code, modifiers, cmd.cmdid);
    }
    else{
        bind(helper, code, modifiers, cmd.command);
    }
}

inline void
bind_vanilla_keys(Bind_Helper *helper, int32_t cmdid){
    bind(helper, 0, 0, cmdid);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, Custom_Command_Function *func){
    bind(helper, 0, 0, func);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, unsigned char modifiers, int32_t cmdid){
    bind(helper, 0, modifiers, cmdid);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, unsigned char modifiers, Custom_Command_Function *func){
    bind(helper, 0, modifiers, func);
}

inline void
inherit_map(Bind_Helper *helper, int32_t mapid){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    if (!helper->error && mapid < mapid_global) ++helper->header->header.user_map_count;
    
    Binding_Unit unit = {0};
    unit.type = unit_inherit;
    unit.map_inherit.mapid = mapid;
    
    write_unit(helper, unit);
}

inline void
set_hook(Bind_Helper *helper, int32_t hook_id, Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = hook_id;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_scroll_rule(Bind_Helper *helper, Scroll_Rule_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_scroll_rule;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_buffer_name_resolver(Bind_Helper *helper, Buffer_Name_Resolver_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_buffer_name_resolver;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_new_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_new_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_start_hook(Bind_Helper *helper, Start_Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_start;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_open_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_open_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_save_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_save_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_end_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_end_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_command_caller(Bind_Helper *helper, Command_Caller_Hook_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_command_caller;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline void
set_input_filter(Bind_Helper *helper, Input_Filter_Function *func){
    Binding_Unit unit = {0};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_input_filter;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

inline int32_t
end_bind_helper(Bind_Helper *helper){
    if (helper->header){
        helper->header->header.total_size = (int32_t)(helper->cursor - helper->start);
        helper->header->header.error = helper->error;
    }
    int32_t result = helper->write_total;
    return(result);
}

struct Bind_Buffer{
    void *data;
    int32_t size;
};

inline Bind_Buffer
end_bind_helper_get_buffer(Bind_Helper *helper){
    int32_t size = end_bind_helper(helper);
    Bind_Buffer result = {0};
    result.data = helper->start;
    result.size = size;
    return(result);
}

static u32_4tech
get_key_code(char *buffer){
    u32_4tech ignore;
    u32_4tech result = utf8_to_u32_length_unchecked((u8_4tech*)buffer, &ignore);
    return(result);
}

#endif

// BOTTOM

