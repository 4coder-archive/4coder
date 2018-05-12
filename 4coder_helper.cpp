/*
 * Miscellaneous helpers for common operations.
 */

// TOP

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
    Bind_Helper result = {0};
    result.cursor = (Binding_Unit*)data;
    result.start = result.cursor;
    result.end = result.start + size / sizeof(*result.cursor);
    Binding_Unit unit = {0};
    unit.type = unit_header;
    unit.header.total_size = sizeof(*result.header);
    result.header = write_unit(&result, unit);
    result.header->header.user_map_count = 0;
    return(result);
}

inline void
begin_map(Bind_Helper *helper, int32_t mapid, bool32 replace){
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
    begin_map(helper, mapid, false);
}

inline void
restart_map(Bind_Helper *helper, int32_t mapid){
    begin_map(helper, mapid, true);
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

////////////////////////////////

static int32_t
round_down(int32_t x, int32_t b){
    int32_t r = 0;
    if (x >= 0){
        r = x - (x % b);
    }
    return(r);
}

static int32_t
round_up(int32_t x, int32_t b){
    int32_t r = 0;
    if (x >= 0){
        r = x + b - (x % b);
    }
    return(r);
}

static void
exec_command(Application_Links *app, Custom_Command_Function *func){
    func(app);
}

static void
exec_command(Application_Links *app, Generic_Command cmd){
    if (cmd.cmdid < cmdid_count){
        exec_command(app, cmd.cmdid);
    }
    else{
        exec_command(app, cmd.command);
    }
}

static int32_t
key_is_unmodified(Key_Event_Data *key){
    int8_t *mods = key->modifiers;
    int32_t unmodified = (!mods[MDFR_CONTROL_INDEX] && !mods[MDFR_ALT_INDEX]);
    return(unmodified);
}

static uint32_t
to_writable_character(User_Input in, uint8_t *character){
    uint32_t result = 0;
    if (in.type == UserInputKey){
        if (in.key.character != 0){
            u32_to_utf8_unchecked(in.key.character, character, &result);
        }
    }
    return(result);
}

static uint32_t
to_writable_character(Key_Event_Data key, uint8_t *character){
    uint32_t result = 0;
    if (key.character != 0){
        u32_to_utf8_unchecked(key.character, character, &result);
    }
    return(result);
}

static bool32
backspace_utf8(String *str){
    bool32 result = false;
    uint8_t *s = (uint8_t*)str->str;
    if (str->size > 0){
        uint32_t i = str->size-1;
        for (; i > 0; --i){
            if (s[i] <= 0x7F || s[i] >= 0xC0){
                break;
            }
        }
        str->size = i;
        result = true;
    }
    return(result);
}

static bool32
query_user_general(Application_Links *app, Query_Bar *bar, bool32 force_number){
    bool32 success = true;
    
    // NOTE(allen|a3.4.4): It will not cause an *error* if we continue on after failing to.
    // start a query bar, but it will be unusual behavior from the point of view of the
    // user, if this command starts intercepting input even though no prompt is shown.
    // This will only happen if you have a lot of bars open already or if the current view
    // doesn't support query bars.
    if (start_query_bar(app, bar, 0) == 0) return 0;
    
    for (;;){
        // NOTE(allen|a3.4.4): This call will block until the user does one of the input
        // types specified in the flags.  The first set of flags are inputs you'd like to intercept
        // that you don't want to abort on.  The second set are inputs that you'd like to cause
        // the command to abort.  If an event satisfies both flags, it is treated as an abort.
        User_Input in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        
        // NOTE(allen|a3.4.4): The responsible thing to do on abort is to end the command
        // without waiting on get_user_input again.
        if (in.abort){
            success = false;
            break;
        }
        
        uint8_t character[4];
        uint32_t length = 0;
        bool32 good_character = false;
        if (key_is_unmodified(&in.key)){
            if (force_number){
                if (in.key.character >= '0' && in.key.character <= '9'){
                    good_character = true;
                    length = to_writable_character(in, character);
                }
            }
            else{
                length = to_writable_character(in, character);
                if (length != 0){
                    good_character = true;
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
                backspace_utf8(&bar->string);
            }
            else if (good_character){
                append_ss(&bar->string, make_string(character, length));
            }
        }
    }
    
    terminate_with_null(&bar->string);
    
    return(success);
}

static bool32
query_user_string(Application_Links *app, Query_Bar *bar){
    return(query_user_general(app, bar, false));
}

static bool32
query_user_number(Application_Links *app, Query_Bar *bar){
    return(query_user_general(app, bar, true));
}

static void
init_theme_zero(Theme *theme){
    for (int32_t i = 0; i < Stag_COUNT; ++i){
        theme->colors[i] = 0;
    }
}

static char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char result = ' ';
    *buffer = get_buffer(app, buffer->buffer_id, AccessAll);
    if (pos < buffer->size){
        buffer_read_range(app, buffer, pos, pos+1, &result);
    }
    return(result);
}

static Buffer_Identifier
buffer_identifier(char *str, int32_t len){
    Buffer_Identifier identifier;
    identifier.name = str;
    identifier.name_len = len;
    identifier.id = 0;
    return(identifier);
}

static Buffer_Identifier
buffer_identifier(Buffer_ID id){
    Buffer_Identifier identifier;
    identifier.name = 0;
    identifier.name_len = 0;
    identifier.id = id;
    return(identifier);
}

static Range
make_range(int32_t p1, int32_t p2){
    Range range;
    if (p1 < p2){
        range.min = p1;
        range.max = p2;
    }
    else{
        range.min = p2;
        range.max = p1;
    }
    return(range);
}

static void
adjust_all_buffer_wrap_widths(Application_Links *app, int32_t wrap_width, int32_t min_base_width){
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap_width);
        buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, min_base_width);
    }
}

static Buffer_Rect
get_rect(View_Summary *view){
    Buffer_Rect rect = {0};
    
    rect.char0 = view->mark.character;
    rect.line0 = view->mark.line;
    
    rect.char1 = view->cursor.character;
    rect.line1 = view->cursor.line;
    
    if (rect.line0 > rect.line1){
        Swap(int32_t, rect.line0, rect.line1);
    }
    if (rect.char0 > rect.char1){
        Swap(int32_t, rect.char0, rect.char1);
    }
    
    return(rect);
}

static i32_Rect
get_line_x_rect(View_Summary *view){
    i32_Rect rect = {0};
    
    if (view->unwrapped_lines){
        rect.x0 = (int32_t)view->mark.unwrapped_x;
        rect.x1 = (int32_t)view->cursor.unwrapped_x;
    }
    else{
        rect.x0 = (int32_t)view->mark.wrapped_x;
        rect.x1 = (int32_t)view->cursor.wrapped_x;
    }
    rect.y0 = view->mark.line;
    rect.y1 = view->cursor.line;
    
    if (rect.y0 > rect.y1){
        Swap(int32_t, rect.y0, rect.y1);
    }
    if (rect.x0 > rect.x1){
        Swap(int32_t, rect.x0, rect.x1);
    }
    
    return(rect);
}

static View_Summary
get_first_view_with_buffer(Application_Links *app, int32_t buffer_id){
    View_Summary result = {0};
    View_Summary test = {0};
    
    if (buffer_id != 0){
        uint32_t access = AccessAll;
        for(test = get_view_first(app, access);
            test.exists;
            get_view_next(app, &test, access)){
            
            Buffer_Summary buffer = get_buffer(app, test.buffer_id, access);
            
            if(buffer.buffer_id == buffer_id){
                result = test;
                break;
            }
        }
    }
    
    return(result);
}

static bool32
open_file(Application_Links *app, Buffer_Summary *buffer_out,
          char *filename, int32_t filename_len, bool32 background, bool32 never_new){
    bool32 result = false;
    Buffer_Summary buffer = get_buffer_by_name(app, filename, filename_len, AccessProtected|AccessHidden);
    
    if (buffer.exists){
        if (buffer_out){
            *buffer_out = buffer;
        }
        result = true;
    }
    else{
        Buffer_Create_Flag flags = 0;
        if (background){
            flags |= BufferCreate_Background;
        }
        if (never_new){
            flags |= BufferCreate_NeverNew;
        }
        buffer = create_buffer(app, filename, filename_len, flags);
        if (buffer.exists){
            if (buffer_out != 0){
                *buffer_out = buffer;
            }
            result = true;
        }
    }
    
    return(result);
}

static Buffer_ID
buffer_identifier_to_id(Application_Links *app, Buffer_Identifier identifier){
    Buffer_ID id = 0;
    if (identifier.id != 0){
        id = identifier.id;
    }
    else{
        Buffer_Summary buffer = get_buffer_by_name(app, identifier.name, identifier.name_len, AccessAll);
        id = buffer.buffer_id;
        if (id == 0){
            buffer = get_buffer_by_file_name(app, identifier.name, identifier.name_len, AccessAll);
            id = buffer.buffer_id;
        }
    }
    return(id);
}

static bool32
view_open_file(Application_Links *app, View_Summary *view,
               char *filename, int32_t filename_len, bool32 never_new){
    bool32 result = false;
    
    if (view != 0){
        Buffer_Summary buffer = {0};
        if (open_file(app, &buffer, filename, filename_len, false, never_new)){
            view_set_buffer(app, view, buffer.buffer_id, 0);
            result = true;
        }
    }
    
    return(result);
}

static void
get_view_prev(Application_Links *app, View_Summary *view, uint32_t access){
    if (view->exists){
        View_ID original_id = view->view_id;
        View_ID check_id = original_id;
        
        View_Summary new_view = {0};
        
        for (;;){
            --check_id;
            if (check_id <= 0){
                check_id = 16;
            }
            if (check_id == original_id){
                new_view = *view;
                break;
            }
            new_view = get_view(app, check_id, access);
            if (new_view.exists){
                break;
            }
        }
        
        *view = new_view;
    }
}

static View_Summary
get_view_last(Application_Links *app, uint32_t access){
    View_Summary view = {0};
    view.exists = true;
    get_view_prev(app, &view, access);
    if (view.view_id < 1 || view.view_id > 16){
        view = null_view_summary;
    }
    return(view);
}

static void
get_view_next_looped(Application_Links *app, View_Summary *view, uint32_t access){
    get_view_next(app, view, access);
    if (!view->exists){
        *view = get_view_first(app, access);
    }
}

static void
get_view_prev_looped(Application_Links *app, View_Summary *view, uint32_t access){
    get_view_prev(app, view, access);
    if (!view->exists){
        *view = get_view_last(app, access);
    }
}

static void
refresh_buffer(Application_Links *app, Buffer_Summary *buffer){
    *buffer = get_buffer(app, buffer->buffer_id, AccessAll);
}

static void
refresh_view(Application_Links *app, View_Summary *view){
    *view = get_view(app, view->view_id, AccessAll);
}

// TODO(allen): Setup buffer seeking to do character_pos and get View_Summary out of this parameter list.
static int32_t
character_pos_to_pos(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, int32_t character_pos){
    int32_t result = 0;
    Full_Cursor cursor = {0};
    if (view_compute_cursor(app, view, seek_character_pos(character_pos), &cursor)){
        result = cursor.pos;
    }
    return(result);
}

static float
get_view_y(View_Summary *view){
    float y = view->cursor.wrapped_y;
    if (view->unwrapped_lines){
        y = view->cursor.unwrapped_y;
    }
    return(y);
}

static float
get_view_x(View_Summary *view){
    float x = view->cursor.wrapped_x;
    if (view->unwrapped_lines){
        x = view->cursor.unwrapped_x;
    }
    return(x);
}

static Range
get_view_range(View_Summary *view){
    return(make_range(view->cursor.pos, view->mark.pos));
}

static bool32
read_line(Application_Links *app, Partition *part, Buffer_Summary *buffer, int32_t line, String *str){
    Partial_Cursor begin = {0};
    Partial_Cursor end = {0};
    
    bool32 success = false;
    if (buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &begin)){
        if (buffer_compute_cursor(app, buffer, seek_line_char(line, -1), &end)){
            if (begin.line == line){
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer->size){
                    int32_t size = (end.pos - begin.pos);
                    int32_t alloc_size = size + 1;
                    char *memory = push_array(part, char, alloc_size);
                    if (memory != 0){
                        *str = make_string(memory, 0, alloc_size);
                        success = true;
                        buffer_read_range(app, buffer, begin.pos, end.pos, str->str);
                        str->size = size;
                        terminate_with_null(str);
                    }
                }
            }
        }
    }
    
    return(success);
}

static int32_t
buffer_get_line_start(Application_Links *app, Buffer_Summary *buffer, int32_t line){
    int32_t result = buffer->size;
    if (line <= buffer->line_count){
        Partial_Cursor partial_cursor = {0};
        buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &partial_cursor);
        result = partial_cursor.pos;
    }
    return(result);
}

static int32_t
buffer_get_line_end(Application_Links *app, Buffer_Summary *buffer, int32_t line){
    int32_t result = buffer->size;
    if (line <= buffer->line_count){
        Partial_Cursor partial_cursor = {0};
        buffer_compute_cursor(app, buffer, seek_line_char(line, -1), &partial_cursor);
        result = partial_cursor.pos;
    }
    return(result);
}

static int32_t
buffer_get_line_number(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    Partial_Cursor partial_cursor = {0};
    buffer_compute_cursor(app, buffer, seek_pos(pos), &partial_cursor);
    return(partial_cursor.line);
}

static Cpp_Token*
get_first_token_at_line(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, int32_t line, int32_t *line_start_out = 0){
    int32_t line_start = buffer_get_line_start(app, buffer, line);
    Cpp_Get_Token_Result get_token = cpp_get_token(tokens, line_start);
    
    if (get_token.in_whitespace){
        get_token.token_index += 1;
    }
    
    if (line_start_out){
        *line_start_out = line_start;
    }
    
    Cpp_Token *result = 0;
    if (get_token.token_index < tokens.count){
        result = &tokens.tokens[get_token.token_index];
    }
    
    return(result);
}

////////////////////////////////

static bool32
init_stream_chunk(Stream_Chunk *chunk, Application_Links *app, Buffer_Summary *buffer,
                  int32_t pos, char *data, uint32_t size){
    bool32 result = false;
    
    refresh_buffer(app, buffer);
    if (0 <= pos && pos < buffer->size && size > 0){
        chunk->app = app;
        chunk->buffer = buffer;
        chunk->base_data = data;
        chunk->data_size = size;
        chunk->start = round_down(pos, size);
        chunk->end = round_up(pos, size);
        
        if (chunk->max_end > buffer->size || chunk->max_end == 0){
            chunk->max_end = buffer->size;
        }
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = true;
        }
    }
    
    return(result);
}

static bool32
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (chunk->end < buffer->size){
        chunk->start = chunk->end;
        chunk->end += chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = 1;
        }
    }
    
    else if (chunk->add_null && chunk->end + 1 < buffer->size){
        chunk->start = buffer->size;
        chunk->end = buffer->size + 1;
        chunk->base_data[0] = 0;
        chunk->data = chunk->base_data - chunk->start;
        result = 1;
    }
    
    return(result);
}

static bool32
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    bool32 result = 0;
    
    refresh_buffer(app, buffer);
    if (chunk->start > 0){
        chunk->end = chunk->start;
        chunk->start -= chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = 1;
        }
    }
    
    else if (chunk->add_null && chunk->start > -1){
        chunk->start = -1;
        chunk->end = 0;
        chunk->base_data[0] = 0;
        chunk->data = chunk->base_data - chunk->start;
        result = 1;
    }
    
    return(result);
}

////////////////////////////////

static bool32
init_stream_tokens(Stream_Tokens *stream, Application_Links *app, Buffer_Summary *buffer,
                   int32_t pos, Cpp_Token *data, int32_t count){
    bool32 result = false;
    
    refresh_buffer(app, buffer);
    
    int32_t token_count = buffer_token_count(app, buffer);
    if (buffer->tokens_are_ready && pos >= 0 && pos < token_count && count > 0){
        stream->app = app;
        stream->buffer = buffer;
        stream->base_tokens = data;
        stream->count = count;
        stream->start = round_down(pos, count);
        stream->end = round_up(pos, count);
        stream->token_count = token_count;
        
        if (stream->start < 0){
            stream->start = 0;
        }
        if (stream->end > stream->token_count){
            stream->end = stream->token_count;
        }
        
        buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
        stream->tokens = stream->base_tokens - stream->start;
        result = true;
    }
    
    return(result);
}

static Stream_Tokens
begin_temp_stream_token(Stream_Tokens *stream){
    return(*stream);
}

static void
end_temp_stream_token(Stream_Tokens *stream, Stream_Tokens temp){
    if (stream->start != temp.start || stream->end != temp.end){
        Application_Links *app = stream->app;
        buffer_read_tokens(app, temp.buffer, temp.start, temp.end, temp.base_tokens);
        stream->tokens = stream->base_tokens - temp.start;
        stream->start = temp.start;
        stream->end = temp.end;
    }
}

static bool32
forward_stream_tokens(Stream_Tokens *stream){
    Application_Links *app = stream->app;
    Buffer_Summary *buffer = stream->buffer;
    bool32 result = false;
    
    refresh_buffer(app, buffer);
    if (stream->end < stream->token_count){
        stream->start = stream->end;
        stream->end += stream->count;
        
        if (stream->end > stream->token_count){
            stream->end = stream->token_count;
        }
        
        if (stream->start < stream->end){
            buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = true;
        }
    }
    
    return(result);
}

static bool32
backward_stream_tokens(Stream_Tokens *stream){
    Application_Links *app = stream->app;
    Buffer_Summary *buffer = stream->buffer;
    bool32 result = false;
    
    refresh_buffer(app, buffer);
    if (stream->start > 0){
        stream->end = stream->start;
        stream->start -= stream->count;
        
        if (0 > stream->start){
            stream->start = 0;
        }
        
        if (stream->start < stream->end){
            buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = true;
        }
    }
    
    return(result);
}

static String
get_query_string(Application_Links *app, char *query_str, char *string_space, int32_t space_size){
    Query_Bar bar;
    bar.prompt = make_string_slowly(query_str);
    bar.string = make_string_cap(string_space, 0, space_size);
    if (!query_user_string(app, &bar)){
        bar.string.size = 0;
    }
    return(bar.string);
}

static String
get_string_in_view_range(Application_Links *app, Partition *arena, View_Summary *view){
    String str = {0};
    Buffer_Summary buffer = get_buffer(app, view->buffer_id, AccessProtected);
    if (!buffer.exists) return(str);
    Range range = get_view_range(view);
    int32_t query_length = range.max - range.min;
    if (query_length != 0){
        char *query_space = push_array(arena, char, query_length);
        if (buffer_read_range(app, &buffer, range.min, range.max, query_space)){
            str = make_string(query_space, query_length);
        }
    }
    return(str);
}

static String
get_token_or_word_under_pos(Application_Links *app, Buffer_Summary *buffer, int32_t pos, char *space, int32_t capacity){
    String result = {0};
    Cpp_Get_Token_Result get_result = {0};
    bool32 success = buffer_get_token_index(app, buffer, pos, &get_result);
    if (success && !get_result.in_whitespace){
        int32_t size = get_result.token_end - get_result.token_start;
        if (size > 0 && size <= capacity){
            success = buffer_read_range(app, buffer, get_result.token_start, get_result.token_end, space);
            if (success){
                result = make_string(space, size);
            }
        }
    }
    return(result);
}

static String
build_string(Partition *part, char *s0, char *s1, char *s2){
    String sr = {0};
    sr.memory_size = str_size(s0) + str_size(s1) + str_size(s2);
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, char *s0, char *s1, String s2){
    String sr = {0};
    sr.memory_size = str_size(s0) + str_size(s1) + s2.size;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, char *s0, String s1, char *s2){
    String sr = {0};
    sr.memory_size = str_size(s0) + s1.size + str_size(s2);
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, char *s0, String s1, String s2){
    String sr = {0};
    sr.memory_size = str_size(s0) + s1.size + s2.size;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, String s0, char *s1, char *s2){
    String sr = {0};
    sr.memory_size = s0.size + str_size(s1) + str_size(s2);
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, String s0, char *s1, String s2){
    String sr = {0};
    sr.memory_size = s0.size + str_size(s1) + s2.size;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, String s0, String s1, char *s2){
    String sr = {0};
    sr.memory_size = s0.size + s1.size + str_size(s2);
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static String
build_string(Partition *part, String s0, String s1, String s2){
    String sr = {0};
    sr.memory_size = s0.size + s1.size + s2.size;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    return(sr);
}

static void
lexer_keywords_default_init(Partition *part, Cpp_Keyword_Table *kw_out, Cpp_Keyword_Table *pp_out){
    umem_4tech kw_size = cpp_get_table_memory_size_default(CPP_TABLE_KEYWORDS);
    umem_4tech pp_size = cpp_get_table_memory_size_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES);
    void *kw_mem = push_array(part, char, (i32_4tech)kw_size);
    void *pp_mem = push_array(part, char, (i32_4tech)pp_size);
    *kw_out = cpp_make_table_default(CPP_TABLE_KEYWORDS, kw_mem, kw_size);
    *pp_out = cpp_make_table_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES, pp_mem, pp_size);
}

////////////////////////////////

// TODO(allen): Stop handling files this way!  My own API should be able to do this!!?!?!?!!?!?!!!!?
// NOTE(allen): Actually need binary buffers for some stuff to work, but not this parsing thing here.
#include <stdio.h>

static String
dump_file_handle(Partition *arena, FILE *file){
    String str = {0};
    if (file != 0){
        fseek(file, 0, SEEK_END);
        int32_t size = ftell(file);
        char *mem = push_array(arena, char, size + 1);
        push_align(arena, 8);
        if (mem != 0){
            fseek(file, 0, SEEK_SET);
            fread(mem, 1, size, file);
            mem[size] = 0;
            str = make_string_cap(mem, size, size + 1);
        }
    }
    return(str);
}

static FILE*
open_file_search_up_path(Partition *scratch, String path, String file_name){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    int32_t cap = path.size + file_name.size + 2;
    char *space = push_array(scratch, char, cap);
    String name_str = make_string_cap(space, 0, cap);
    append(&name_str, path);
    if (name_str.size == 0 || !char_is_slash(name_str.str[name_str.size - 1])){
        append(&name_str, "/");
    }
    
    FILE *file = 0;
    for (;;){
        int32_t base_size = name_str.size;
        append(&name_str, file_name);
        terminate_with_null(&name_str);
        file = fopen(name_str.str, "rb");
        if (file != 0){
            break;
        }
        
        name_str.size = base_size;
        remove_last_folder(&name_str);
        if (name_str.size >= base_size){
            break;
        }
    }
    
    end_temp_memory(temp);
    return(file);
}

static FILE*
open_file_try_current_path_then_binary_path(Application_Links *app, char *file_name){
    FILE *file = fopen(file_name, "rb");
    if (file == 0){
        char space[256];
        int32_t size = get_4ed_path(app, space, sizeof(space));
        String str = make_string_cap(space, size, sizeof(space));
        append(&str, "/");
        append(&str, file_name);
        if (terminate_with_null(&str)){
        file = fopen(str.str, "rb");
}
    }
    return(file);
}

static char*
get_null_terminated(Partition *scratch, String name){
    char *name_terminated = 0;
    if (name.size < name.memory_size){
        terminate_with_null(&name);
        name_terminated = name.str;
    }
    else{
        name_terminated = push_array(scratch, char, name.size + 1);
        if (name_terminated != 0){
            memcpy(name_terminated, name.str, name.size);
            name_terminated[name.size] = 0;
        }
    }
    return(name_terminated);
}

static FILE*
open_file(Partition *scratch, String name){
    FILE *file = 0;
    Temp_Memory temp = begin_temp_memory(scratch);
    char *name_terminated = get_null_terminated(scratch, name);
    if (name_terminated != 0){
        file = fopen(name_terminated, "rb");
    }
    end_temp_memory(temp);
    return(file);
}

static String
dump_file(Partition *arena, String name){
    String result = {0};
    FILE *file = open_file(arena, name);
    if (file != 0){
        result = dump_file_handle(arena, file);
        fclose(file);
    }
    return(result);
}

static String
dump_file_search_up_path(Partition *arena, String path, String file_name){
    String result = {0};
    FILE *file = open_file_search_up_path(arena, path, file_name);
    if (file != 0){
        result = dump_file_handle(arena, file);
        fclose(file);
    }
    return(result);
}

// BOTTOM

