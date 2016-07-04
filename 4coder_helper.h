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
    Buffer_Rect rect;
    
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


struct Stream_Chunk{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    char *base_data;
    int start, end;
    int data_size;
    
    char *data;
};

int
round_down(int x, int b){
    int r = 0;
    if (x >= 0){
        r = x - (x % b);
    }
    return(r);
}

int
round_up(int x, int b){
    int r = 0;
    if (x >= 0){
        r = x - (x % b) + b;
    }
    return(r);
}

void
refresh_buffer(Application_Links *app, Buffer_Summary *buffer){
    *buffer = app->get_buffer(app, buffer->buffer_id, AccessAll);
}

void
refresh_view(Application_Links *app, View_Summary *view){
    *view = app->get_view(app, view->view_id, AccessAll);
}

int
init_stream_chunk(Stream_Chunk *chunk,
                  Application_Links *app, Buffer_Summary *buffer,
                  int pos, char *data, int size){
    int result = 0;
    
    refresh_buffer(app, buffer);
    if (pos >= 0 && pos < buffer->size && size > 0){
        result = 1;
        chunk->app = app;
        chunk->buffer = buffer;
        chunk->base_data = data;
        chunk->data_size = size;
        chunk->start = round_down(pos, size);
        chunk->end = round_up(pos, size);
        if (chunk->end > buffer->size){
            chunk->end = buffer->size;
        }
        app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
        chunk->data = chunk->base_data - chunk->start;
    }
    return(result);
}

int
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    int result = 0;
    
    refresh_buffer(app, buffer);
    if (chunk->end < buffer->size){
        result = 1;
        chunk->start = chunk->end;
        chunk->end += chunk->data_size;
        if (chunk->end > buffer->size){
            chunk->end = buffer->size;
        }
        app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
        chunk->data = chunk->base_data - chunk->start;
    }
    return(result);
}

int
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    int result = 0;
    
    refresh_buffer(app, buffer);
    if (chunk->start > 0){
        result = 1;
        chunk->end = chunk->start;
        chunk->start -= chunk->data_size;
        if (chunk->start < 0){
            chunk->start = 0;
        }
        app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
        chunk->data = chunk->base_data - chunk->start;
    }
    return(result);
}

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_Summary *buffer,
                              int pos, char delim, int *result){
    if (buffer->exists){
        char chunk[1024];
        int size = sizeof(chunk);
        Stream_Chunk stream = {0};
        
        if (init_stream_chunk(&stream, app, buffer, pos, chunk, size)){
            int still_looping = 1;
            do{
                for(; pos < stream.end; ++pos){
                    char at_pos = stream.data[pos];
                    if (at_pos == delim){
                        *result = pos;
                        goto finished;
                    }
                }
                still_looping = forward_stream_chunk(&stream);
            }while (still_looping);
        }
    }
    
    *result = buffer->size;
    
    finished:;
}

void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_Summary *buffer,
                              int pos, char delim, int *result){
    if (buffer->exists){
        char chunk[1024];
        int size = sizeof(chunk);
        Stream_Chunk stream = {0};
        
        if (init_stream_chunk(&stream, app, buffer, pos, chunk, size)){
            int still_looping = 1;
            do{
                for(; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (at_pos == delim){
                        *result = pos;
                        goto finished;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while (still_looping);
        }
    }
    
    *result = 0;
    
    finished:;
}

// TODO(allen): This duplication is driving me crazy... I've gotta
// upgrade the meta programming system another level.

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_forward(Application_Links *app, Buffer_Summary *buffer,
                           int pos, char *str, int size, int *result){
    char read_buffer[512];
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    if (size <= 0){
        *result = pos;
    }
    else if (size > sizeof(read_buffer)){
        *result = pos;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = str[0];
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int still_looping = 1;
                do{
                    for(; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = buffer->size;
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_backward(Application_Links *app, Buffer_Summary *buffer,
                            int pos, char *str, int size, int *result){
    char read_buffer[512];
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    if (size <= 0){
        *result = 0;
    }
    else if (size > sizeof(read_buffer)){
        *result = 0;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = str[0];
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int still_looping = 1;
                do{
                    for(; pos >= stream.start; --pos){
                        char at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = backward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = 0;
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_Summary *buffer,
                                       int pos, char *str, int size, int *result){
    char read_buffer[512];
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    if (size <= 0){
        *result = pos;
    }
    else if (size > sizeof(read_buffer)){
        *result = pos;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = char_to_upper(str[0]);
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int still_looping = 1;
                do{
                    for(; pos < stream.end; ++pos){
                        char at_pos = char_to_upper(stream.data[pos]);
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_insensitive(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = buffer->size;
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_Summary *buffer,
                                        int pos, char *str, int size, int *result){
    char read_buffer[512];
    char chunk[1024];
    int chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    if (size <= 0){
        *result = -1;
    }
    else if (size > sizeof(read_buffer)){
        *result = -1;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = char_to_upper(str[0]);
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int still_looping = 1;
                do{
                    for(; pos >= stream.start; --pos){
                        char at_pos = char_to_upper(stream.data[pos]);
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_insensitive(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = backward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = -1;
        
        finished:;
    }
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

#endif
