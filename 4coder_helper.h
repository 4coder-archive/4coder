/*
 * Bind helper struct and functions
 */

struct Bind_Helper{
    Binding_Unit *cursor, *start, *end;
    Binding_Unit *header, *group;
    int write_total;
    int error;
};

#define BH_ERR_NONE 0
#define BH_ERR_MISSING_END 1
#define BH_ERR_MISSING_BEGIN 2
#define BH_ERR_OUT_OF_MEMORY 3

inline void
copy(char *dest, const char *src, int len){
    for (int i = 0; i < len; ++i){
        *dest++ = *src++;
    }
}

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

inline char*
write_inline_string(Bind_Helper *helper, char *value, int len){
    char *dest = 0;
    helper->write_total += len;
    if (helper->error == 0){
        dest = (char*)helper->cursor;
        int cursor_advance = len + sizeof(*helper->cursor) - 1;
        cursor_advance /= sizeof(*helper->cursor);
        cursor_advance *= sizeof(*helper->cursor);
        helper->cursor += cursor_advance;
        if (helper->cursor < helper->end){
            copy(dest, value, len);
        }
        else{
            helper->error = BH_ERR_OUT_OF_MEMORY;
        }
    }
    return dest;
}

inline Bind_Helper
begin_bind_helper(void *data, int size){
    Bind_Helper result;
    
    result.header = 0;
    result.group = 0;
    result.write_total = 0;
    result.error = 0;
    
    result.cursor = (Binding_Unit*)data;
    result.start = result.cursor;
    result.end = result.start + size / sizeof(*result.cursor);
    
    Binding_Unit unit;
    unit.type = unit_header;
    unit.header.total_size = sizeof(*result.header);
    result.header = write_unit(&result, unit);
    result.header->header.user_map_count = 0;
    
    return result;
}

inline void
begin_map(Bind_Helper *helper, int mapid){
    if (helper->group != 0 && helper->error == 0) helper->error = BH_ERR_MISSING_END;
    if (!helper->error && mapid < mapid_global) ++helper->header->header.user_map_count;
    
    Binding_Unit unit;
    unit.type = unit_map_begin;
    unit.map_begin.mapid = mapid;
    helper->group = write_unit(helper, unit);
    helper->group->map_begin.bind_count = 0;
}

inline void
end_map(Bind_Helper *helper){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    helper->group = 0;
}

inline void
bind(Bind_Helper *helper, short code, unsigned char modifiers, int cmdid){
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
bind(Bind_Helper *helper, short code, unsigned char modifiers, Custom_Command_Function *func){
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
bind_vanilla_keys(Bind_Helper *helper, int cmdid){
    bind(helper, 0, 0, cmdid);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, Custom_Command_Function *func){
    bind(helper, 0, 0, func);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, unsigned char modifiers, int cmdid){
    bind(helper, 0, modifiers, cmdid);
}

inline void
bind_vanilla_keys(Bind_Helper *helper, unsigned char modifiers, Custom_Command_Function *func){
    bind(helper, 0, modifiers, func);
}

inline void
inherit_map(Bind_Helper *helper, int mapid){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    if (!helper->error && mapid < mapid_global) ++helper->header->header.user_map_count;
    
    Binding_Unit unit;
    unit.type = unit_inherit;
    unit.map_inherit.mapid = mapid;
    
    write_unit(helper, unit);
}

inline void
set_hook(Bind_Helper *helper, int hook_id, Custom_Command_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = hook_id;
    unit.hook.func = func;
    
    write_unit(helper, unit);
}

inline void
end_bind_helper(Bind_Helper *helper){
    if (helper->header){
        helper->header->header.total_size = (int)(helper->cursor - helper->start);
        helper->header->header.error = helper->error;
    }
}

// NOTE(allen): Useful functions and overloads on app links
inline void
push_parameter(Application_Links *app, void *cmd_context, int param, int value){
    app->push_parameter(cmd_context, dynamic_int(param), dynamic_int(value));
}

inline void
push_parameter(Application_Links *app, void *cmd_context, int param, const char *value, int value_len){
    char *value_copy = app->push_memory(cmd_context, value_len+1);
    copy(value_copy, value, value_len);
    value_copy[value_len] = 0;
    app->push_parameter(cmd_context, dynamic_int(param), dynamic_string(value_copy, value_len));
}

inline void
push_parameter(Application_Links *app, void *cmd_context, const char *param, int param_len, int value){
    char *param_copy = app->push_memory(cmd_context, param_len+1);
    copy(param_copy, param, param_len);
    param_copy[param_len] = 0;
    app->push_parameter(cmd_context, dynamic_string(param_copy, param_len), dynamic_int(value));
}

inline void
push_parameter(Application_Links *app, void *cmd_context, const char *param, int param_len, const char *value, int value_len){
    char *param_copy = app->push_memory(cmd_context, param_len+1);
    char *value_copy = app->push_memory(cmd_context, value_len+1);
    copy(param_copy, param, param_len);
    copy(value_copy, value, value_len);
    value_copy[value_len] = 0;
    param_copy[param_len] = 0;
    
    app->push_parameter(cmd_context, dynamic_string(param_copy, param_len), dynamic_string(value_copy, value_len));
}

inline String
push_directory(Application_Links *app, void *cmd_context){
    String result;
    result.memory_size = 512;
    result.str = app->push_memory(cmd_context, result.memory_size);
    result.size = app->directory_get_hot(cmd_context, result.str, result.memory_size);
    return(result);
}

#define expand_string(d) ((d).str), ((d).size)

inline Range
get_range(File_View_Summary *view){
    Range range;
    if (view->cursor.pos < view->mark.pos){
        range.min = view->cursor.pos;
        range.max = view->mark.pos;
    }
    else{
        range.max = view->cursor.pos;
        range.min = view->mark.pos;
    }
    
    return(range);
}

#if DisableMacroTranslations == 0

inline void
exec_command_(Application_Links *app, void *cmd_context, Command_ID id){
    app->exec_command_keep_stack(cmd_context, id);
    app->clear_parameters(cmd_context);
}

inline void
exec_command_(Application_Links *app, void *cmd_context, Custom_Command_Function *func){
    func(cmd_context, app);
    app->clear_parameters(cmd_context);
}

#define exec_command_keep_stack app->exec_command_keep_stack
#define clear_parameters app->clear_parameters

#define exec_command(cmd_context, cmd) exec_command_(app, cmd_context, cmd)

#endif

