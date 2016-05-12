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
set_hook(Bind_Helper *helper, int hook_id, Hook_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = hook_id;
    unit.hook.func = (void*) func;
    
    write_unit(helper, unit);
}

inline void
set_scroll_rule(Bind_Helper *helper, Scroll_Rule_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = _hook_scroll_rule;
    unit.hook.func = (void*) func;
    
    write_unit(helper, unit);
}

inline int
end_bind_helper(Bind_Helper *helper){
    int result;
    if (helper->header){
        helper->header->header.total_size = (int)(helper->cursor - helper->start);
        helper->header->header.error = helper->error;
    }
    result = helper->write_total;
    return(result);
}

// NOTE(allen): Useful functions and overloads on app links
inline void
push_parameter(Application_Links *app, int param, int value){
    app->push_parameter(app, dynamic_int(param), dynamic_int(value));
}

inline void
push_parameter(Application_Links *app, int param, const char *value, int value_len){
    char *value_copy = app->push_memory(app, value_len+1);
    copy(value_copy, value, value_len);
    value_copy[value_len] = 0;
    app->push_parameter(app, dynamic_int(param), dynamic_string(value_copy, value_len));
}

inline void
push_parameter(Application_Links *app, const char *param, int param_len, int value){
    char *param_copy = app->push_memory(app, param_len+1);
    copy(param_copy, param, param_len);
    param_copy[param_len] = 0;
    app->push_parameter(app, dynamic_string(param_copy, param_len), dynamic_int(value));
}

inline void
push_parameter(Application_Links *app, const char *param, int param_len, const char *value, int value_len){
    char *param_copy = app->push_memory(app, param_len+1);
    char *value_copy = app->push_memory(app, value_len+1);
    copy(param_copy, param, param_len);
    copy(value_copy, value, value_len);
    value_copy[value_len] = 0;
    param_copy[param_len] = 0;
    
    app->push_parameter(app, dynamic_string(param_copy, param_len), dynamic_string(value_copy, value_len));
}

inline Range
get_range(View_Summary *view){
    Range range;
    range = make_range(view->cursor.pos, view->mark.pos);
    return(range);
}

struct Buffer_Rect{
    int char0,line0;
    int char1,line1;
};

#ifndef Swap
#define Swap(a,b) do{ auto t = a; a = b; b = t; } while(0)
#endif

inline Buffer_Rect
get_rect(View_Summary *view){
    Buffer_Rect rect;
    
    rect.char0 = view->mark.character;
    rect.line0 = view->mark.line;
        
    rect.char1 = view->cursor.character;
    rect.line1 = view->cursor.line;
    
    if (rect.line0 > rect.line1){
        Swap(rect.line0, rect.line1);
    }
    if (rect.char0 > rect.char1){
        Swap(rect.char0, rect.char1);
    }
    
    return(rect);
}

inline void
exec_command(Application_Links *app, Command_ID id){
    app->exec_command_keep_stack(app, id);
    app->clear_parameters(app);
}

inline void
exec_command(Application_Links *app, Custom_Command_Function *func){
    func(app);
    app->clear_parameters(app);
}

inline void
active_view_to_line(Application_Links *app, int line_number){
    View_Summary view;
    view = app->get_active_view(app);
    
    // NOTE(allen|a3.4.4): We don't have to worry about whether this is a valid line number.
    // When it's not possible to place a cursor at the position for whatever reason it will set the
    // cursor to a nearby valid position.
    app->view_set_cursor(app, &view, seek_line_char(line_number, 0), 1);
}

inline View_Summary
get_first_view_with_buffer(Application_Links *app, int buffer_id){
    View_Summary result = {};
    View_Summary test = {};

    for(test = app->get_view_first(app);
        test.exists;
        app->get_view_next(app, &test)){
        if(test.locked_buffer_id == buffer_id){
            result = test;
            break;
        }
    }

    return(result);
}

inline int
key_is_unmodified(Key_Event_Data *key){
    char *mods = key->modifiers;
    int unmodified = !mods[MDFR_CONTROL_INDEX] && !mods[MDFR_ALT_INDEX];
    return(unmodified);
}

static int
query_user_general(Application_Links *app, Query_Bar *bar, int force_number){
    User_Input in;
    int success = 1;
    int good_character = 0;
    
    // NOTE(allen|a3.4.4): It will not cause an *error* if we continue on after failing to.
    // start a query bar, but it will be unusual behavior from the point of view of the
    // user, if this command starts intercepting input even though no prompt is shown.
    // This will only happen if you have a lot of bars open already or if the current view
    // doesn't support query bars.
    if (app->start_query_bar(app, bar, 0) == 0) return 0;
    
    while (1){
        // NOTE(allen|a3.4.4): This call will block until the user does one of the input
        // types specified in the flags.  The first set of flags are inputs you'd like to intercept
        // that you don't want to abort on.  The second set are inputs that you'd like to cause
        // the command to abort.  If an event satisfies both flags, it is treated as an abort.
        in = app->get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        
        // NOTE(allen|a3.4.4): The responsible thing to do on abort is to end the command
        // without waiting on get_user_input again.
        if (in.abort){
            success = 0;
            break;
        }
        
        good_character = 0;
        if (key_is_unmodified(&in.key)){
            if (force_number){
                if (in.key.character >= '0' && in.key.character <= '9'){
                    good_character = 1;
                }
            }
            else{
                if (in.key.character != 0){
                    good_character = 1;
                }
            }
        }

        // NOTE(allen|a3.4.4): All we have to do to update the query bar is edit our
        // local Query_Bar struct!  This is handy because it means our Query_Bar
        // is always correct for typical use without extra work updating the bar.
        if (in.type == UserInputKey){
            if (in.key.keycode == '\n' || in.key.keycode == '\t'){
                break;
            }
            else if (in.key.keycode == key_back){
                if (bar->string.size > 0){
                    --bar->string.size;
                }
            }
            else if (good_character){
                append(&bar->string, in.key.character);
            }
        }
    }
    
    return(success);
}

inline int
query_user_string(Application_Links *app, Query_Bar *bar){
    int success = query_user_general(app, bar, 0);
    return(success);
}

inline int
query_user_number(Application_Links *app, Query_Bar *bar){
    int success = query_user_general(app, bar, 1);
    return(success);
}

inline String empty_string() {String Result = {}; return(Result);}
