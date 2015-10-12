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

inline int
seek_null(char *str){
    char *start = str;
    while (*str) ++str;
    return (int)(str - start);
}

inline void
copy(char *dest, char *src, int len){
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
    unit.type = UNIT_HEADER;
    unit.header.total_size = sizeof(*result.header);
    result.header = write_unit(&result, unit);
    result.header->header.map_count = 0;
    result.header->header.group_count = 0;
    
    return result;
}

inline void
begin_map(Bind_Helper *helper, int mapid){
    if (helper->group != 0 && helper->error == 0) helper->error = BH_ERR_MISSING_END;
    if (!helper->error) ++helper->header->header.map_count;
    
    Binding_Unit unit;
    unit.type = UNIT_MAP_BEGIN;
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
    unit.type = UNIT_BINDING;
    unit.binding.command_id = cmdid;
    unit.binding.code = code;
    unit.binding.modifiers = modifiers;
    
    write_unit(helper, unit);
}

inline void
bind_me(Bind_Helper *helper, short code, unsigned char modifiers, Custom_Command_Function *func){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    if (!helper->error) ++helper->group->map_begin.bind_count;
    
    Binding_Unit unit;
    unit.type = UNIT_CALLBACK;
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
bind_me_vanilla_keys(Bind_Helper *helper, Custom_Command_Function *func){
    bind_me(helper, 0, 0, func);
}

inline void
inherit_map(Bind_Helper *helper, int mapid){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    
    Binding_Unit unit;
    unit.type = UNIT_INHERIT;
    unit.map_inherit.mapid = mapid;
    
    write_unit(helper, unit);
}

inline void
end_bind_helper(Bind_Helper *helper){
    if (helper->header){
        helper->header->header.total_size = (int)(helper->cursor - helper->start);
        helper->header->header.error = helper->error;
    }
}

inline void
begin_settings_group(Bind_Helper *helper){
    if (helper->group != 0 && helper->error == 0) helper->error = BH_ERR_MISSING_END;
    if (!helper->error) ++helper->header->header.group_count;
    
    Binding_Unit unit;
    unit.type = UNIT_SETTINGS_BEGIN;
    helper->group = write_unit(helper, unit);
}

inline void
end_group(Bind_Helper *helper){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    helper->group = 0;
}

inline void
use_when(Bind_Helper *helper, int clause_type, int value){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;

    Binding_Unit unit;
    unit.type = UNIT_USE_CLAUSE;
    unit.use_clause.clause_type = clause_type;
    unit.use_clause.value = value;
    write_unit(helper, unit);
}

inline void
use_when(Bind_Helper *helper, int clause_type, char *value){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    
    Binding_Unit unit;
    unit.type = UNIT_USE_CLAUSE;
    unit.use_clause_string.clause_type = clause_type;
    unit.use_clause_string.len = seek_null(value);
    Binding_Unit *u = write_unit(helper, unit);
    u->use_clause_string.value = write_inline_string(helper, value, unit.use_clause_string.len);
}

inline void
use_when(Bind_Helper *helper, int clause_type, char *value, int len){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    
    Binding_Unit unit;
    unit.type = UNIT_USE_CLAUSE;
    unit.use_clause_string.clause_type = clause_type;
    unit.use_clause_string.len = len;
    unit.use_clause_string.value = value;
    write_unit(helper, unit);
}

inline void
set(Bind_Helper *helper, int setting_id, int value){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    
    Binding_Unit unit;
    unit.type = UNIT_SETTING;
    unit.setting.setting_id = setting_id;
    unit.setting.value = value;
    write_unit(helper, unit);
}

inline void
end_settings_helper(Bind_Helper *helper){
    if (helper->header){
        helper->header->header.error = helper->error;
    }
}

