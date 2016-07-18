/*
 * Bind helper struct and functions
 */

#ifndef FCODER_HELPER_H
#define FCODER_HELPER_H

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
begin_map_(Bind_Helper *helper, int mapid, int replace){
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
begin_map(Bind_Helper *helper, int mapid){
    begin_map_(helper, mapid, 0);
}

inline void
restart_map(Bind_Helper *helper, int mapid){
    begin_map_(helper, mapid, 1);
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
set_open_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = _hook_open_file;
    unit.hook.func = (void*) func;
    
    write_unit(helper, unit);
}

inline void
set_new_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = _hook_new_file;
    unit.hook.func = (void*) func;
    
    write_unit(helper, unit);
}

inline void
set_command_caller(Bind_Helper *helper, Command_Caller_Hook_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = _hook_command_caller;
    unit.hook.func = (void*) func;
    
    write_unit(helper, unit);
}

inline void
set_input_filter(Bind_Helper *helper, Input_Filter_Function *func){
    Binding_Unit unit;
    unit.type = unit_hook;
    unit.hook.hook_id = _hook_input_filter;
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
#define Swap(T,a,b) do{ T t = a; a = b; b = t; } while(0)
#endif

inline Buffer_Rect
get_rect(View_Summary *view){
    Buffer_Rect rect = {0};
    
    rect.char0 = view->mark.character;
    rect.line0 = view->mark.line;
        
    rect.char1 = view->cursor.character;
    rect.line1 = view->cursor.line;
    
    if (rect.line0 > rect.line1){
        Swap(int, rect.line0, rect.line1);
    }
    if (rect.char0 > rect.char1){
        Swap(int, rect.char0, rect.char1);
    }
    
    return(rect);
}

inline void
exec_command(Application_Links *app, Command_ID id){
    app->exec_command(app, id);
}

inline void
exec_command(Application_Links *app, Custom_Command_Function *func){
    func(app);
}

inline void
exec_command(Application_Links *app, Generic_Command cmd){
    if (cmd.cmdid < cmdid_count){
        exec_command(app, cmd.cmdid);
    }
    else{
        exec_command(app, cmd.command);
    }
}

inline void
active_view_to_line(Application_Links *app, unsigned int access, int line_number){
    View_Summary view;
    view = app->get_active_view(app, access);
    
    // NOTE(allen|a3.4.4): We don't have to worry about whether this is a valid line number.
    // When it's not possible to place a cursor at the position for whatever reason it will set the
    // cursor to a nearby valid position.
    app->view_set_cursor(app, &view, seek_line_char(line_number, 0), 1);
}

inline View_Summary
get_first_view_with_buffer(Application_Links *app, int buffer_id){
    View_Summary result = {};
    View_Summary test = {};
    
    if (buffer_id != 0){
        unsigned int access = AccessAll;
        for(test = app->get_view_first(app, access);
            test.exists;
            app->get_view_next(app, &test, access)){
            
            Buffer_Summary buffer = app->get_buffer(app, test.buffer_id, access);
            
            if(buffer.buffer_id == buffer_id){
                result = test;
                break;
            }
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

inline Buffer_Summary
get_active_buffer(Application_Links *app, unsigned int access){
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    return(buffer);
}

inline char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, int pos){
    char result = ' ';
    *buffer = app->get_buffer(app, buffer->buffer_id, AccessAll);
    if (pos >= 0 && pos < buffer->size){
        app->buffer_read_range(app, buffer, pos, pos+1, &result);
    }
    return(result);
}

inline Buffer_Identifier
buffer_identifier(char *str, int len){
    Buffer_Identifier identifier;
    identifier.name = str;
    identifier.name_len = len;
    identifier.id = 0;
    return(identifier);
}

inline Buffer_Identifier
buffer_identifier(int id){
    Buffer_Identifier identifier;
    identifier.name = 0;
    identifier.name_len = 0;
    identifier.id = id;
    return(identifier);
}

static int
view_open_file(Application_Links *app, View_Summary *view,
               char *filename, int filename_len, int do_in_background){
    int result = false;
    Buffer_Summary buffer = app->get_buffer_by_name(app, filename, filename_len, AccessProtected|AccessHidden);
    if (buffer.exists){
        if (!do_in_background){
            if (view){
                app->view_set_buffer(app, view, buffer.buffer_id, 0);
            }
        }
        result = true;
    }
    else{
        buffer = app->create_buffer(app, filename, filename_len, do_in_background);
        if (!do_in_background){
            if (buffer.exists){
                if (view){
                    app->view_set_buffer(app, view, buffer.buffer_id, 0);
                    result = true;
                }
            }
        }
    }
    return(result);
}

static int
read_line(Application_Links *app,
          Partition *part,
          Buffer_Summary *buffer,
          int line,
          String *str){
    
    Partial_Cursor begin = {0};
    Partial_Cursor end = {0};
    
    int success = false;
    
    if (app->buffer_compute_cursor(app, buffer,
                                   seek_line_char(line, 1), &begin)){
        if (app->buffer_compute_cursor(app, buffer,
                                       seek_line_char(line, 65536), &end)){
            if (begin.line == line){
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer->size){
                    int size = (end.pos - begin.pos);
                    *str = make_string(push_array(part, char, size+1), size+1);
                    if (str->str){
                        success = true;
                        app->buffer_read_range(app, buffer, begin.pos, end.pos, str->str);
                        str->size = size;
                        terminate_with_null(str);
                    }
                }
            }
        }
    }
    
    return(success);
}

#endif
