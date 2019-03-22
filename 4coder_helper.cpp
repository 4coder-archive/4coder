/*
 * Miscellaneous helpers for common operations.
 */

// TOP

static Binding_Unit*
write_unit(Bind_Helper *helper, Binding_Unit unit){
    Binding_Unit *p = 0;
    helper->write_total += sizeof(*p);
    if (helper->error == 0 && helper->cursor != helper->end){
        p = helper->cursor++;
        *p = unit;
    }
    return p;
}

static Bind_Helper
begin_bind_helper(void *data, i32 size){
    Bind_Helper result = {};
    result.cursor = (Binding_Unit*)data;
    result.start = result.cursor;
    result.end = result.start + size / sizeof(*result.cursor);
    Binding_Unit unit = {};
    unit.type = unit_header;
    unit.header.total_size = sizeof(*result.header);
    result.header = write_unit(&result, unit);
    result.header->header.user_map_count = 0;
    return(result);
}

static void
begin_map(Bind_Helper *helper, i32 mapid, b32 replace){
    if (helper->group != 0 && helper->error == 0){
        helper->error = BH_ERR_MISSING_END;
    }
    if (!helper->error && mapid < mapid_global){
        ++helper->header->header.user_map_count;
    }
    
    Binding_Unit unit;
    unit.type = unit_map_begin;
    unit.map_begin.mapid = mapid;
    unit.map_begin.replace = replace;
    helper->group = write_unit(helper, unit);
    helper->group->map_begin.bind_count = 0;
}

static void
begin_map(Bind_Helper *helper, i32 mapid){
    begin_map(helper, mapid, false);
}

static void
restart_map(Bind_Helper *helper, i32 mapid){
    begin_map(helper, mapid, true);
}

static void
end_map(Bind_Helper *helper){
    if (helper->group == 0 && helper->error == 0){
        helper->error = BH_ERR_MISSING_BEGIN;
    }
    helper->group = 0;
}

static void
bind(Bind_Helper *helper, Key_Code code, uint8_t modifiers, Custom_Command_Function *func){
    if (helper->group == 0 && helper->error == 0){
        helper->error = BH_ERR_MISSING_BEGIN;
    }
    if (!helper->error){
        ++helper->group->map_begin.bind_count;
    }
    
    Binding_Unit unit;
    unit.type = unit_callback;
    unit.callback.func = func;
    unit.callback.code = code;
    unit.callback.modifiers = modifiers;
    
    write_unit(helper, unit);
}

static void
bind(Bind_Helper *helper, Key_Code code, uint8_t modifiers, Generic_Command cmd){
    bind(helper, code, modifiers, cmd.command);
}

static void
bind_vanilla_keys(Bind_Helper *helper, Custom_Command_Function *func){
    bind(helper, 0, 0, func);
}

static void
bind_vanilla_keys(Bind_Helper *helper, unsigned char modifiers, Custom_Command_Function *func){
    bind(helper, 0, modifiers, func);
}

static void
inherit_map(Bind_Helper *helper, i32 mapid){
    if (helper->group == 0 && helper->error == 0) helper->error = BH_ERR_MISSING_BEGIN;
    if (!helper->error && mapid < mapid_global) ++helper->header->header.user_map_count;
    Binding_Unit unit = {};
    unit.type = unit_inherit;
    unit.map_inherit.mapid = mapid;
    write_unit(helper, unit);
}

static void
set_hook(Bind_Helper *helper, i32 hook_id, Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = hook_id;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_scroll_rule(Bind_Helper *helper, Scroll_Rule_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_scroll_rule;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_buffer_name_resolver(Bind_Helper *helper, Buffer_Name_Resolver_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_buffer_name_resolver;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_modify_color_table_hook(Bind_Helper *helper, Modify_Color_Table_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_modify_color_table;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_clipboard_change_hook(Bind_Helper *helper, Clipboard_Change_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_clipboard_change;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_get_view_buffer_region_hook(Bind_Helper *helper, Get_View_Buffer_Region_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_get_view_buffer_region;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_new_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_new_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_start_hook(Bind_Helper *helper, Start_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_start;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_open_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_open_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_save_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_save_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_end_file_hook(Bind_Helper *helper, Open_File_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_end_file;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_file_edit_range_hook(Bind_Helper *helper, File_Edit_Range_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_file_edit_range;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_file_edit_finished_hook(Bind_Helper *helper, File_Edit_Finished_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_file_edit_finished;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_command_caller(Bind_Helper *helper, Command_Caller_Hook_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_command_caller;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_render_caller(Bind_Helper *helper, Render_Caller_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_render_caller;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static void
set_input_filter(Bind_Helper *helper, Input_Filter_Function *func){
    Binding_Unit unit = {};
    unit.type = unit_hook;
    unit.hook.hook_id = special_hook_input_filter;
    unit.hook.func = (void*)func;
    write_unit(helper, unit);
}

static i32
end_bind_helper(Bind_Helper *helper){
    if (helper->header){
        helper->header->header.total_size = (i32)(helper->cursor - helper->start);
        helper->header->header.error = helper->error;
    }
    i32 result = helper->write_total;
    return(result);
}

static Bind_Buffer
end_bind_helper_get_buffer(Bind_Helper *helper){
    i32 size = end_bind_helper(helper);
    Bind_Buffer result = {};
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

static i32
round_down(i32 x, i32 b){
    i32 r = 0;
    if (x >= 0){
        r = x - (x % b);
    }
    return(r);
}

static i32
round_up(i32 x, i32 b){
    i32 r = 0;
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
    exec_command(app, cmd.command);
}

static i32
key_is_unmodified(Key_Event_Data *key){
    int8_t *mods = key->modifiers;
    i32 unmodified = (!mods[MDFR_CONTROL_INDEX] && !mods[MDFR_ALT_INDEX]);
    return(unmodified);
}

static u32
to_writable_character(User_Input in, uint8_t *character){
    u32 result = 0;
    if (in.key.character != 0){
        u32_to_utf8_unchecked(in.key.character, character, &result);
    }
    return(result);
}

static u32
to_writable_character(Key_Event_Data key, uint8_t *character){
    u32 result = 0;
    if (key.character != 0){
        u32_to_utf8_unchecked(key.character, character, &result);
    }
    return(result);
}

static b32
backspace_utf8(String *str){
    b32 result = false;
    uint8_t *s = (uint8_t*)str->str;
    if (str->size > 0){
        u32 i = str->size-1;
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

static b32
query_user_general(Application_Links *app, Query_Bar *bar, b32 force_number){
    // NOTE(allen|a3.4.4): It will not cause an *error* if we continue on after failing to.
    // start a query bar, but it will be unusual behavior from the point of view of the
    // user, if this command starts intercepting input even though no prompt is shown.
    // This will only happen if you have a lot of bars open already or if the current view
    // doesn't support query bars.
    if (start_query_bar(app, bar, 0) == 0){
        return(false);
    }
    
    b32 success = true;
    
    for (;;){
        // NOTE(allen|a3.4.4): This call will block until the user does one of the input
        // types specified in the flags.  The first set of flags are inputs you'd like to intercept
        // that you don't want to abort on.  The second set are inputs that you'd like to cause
        // the command to abort.  If an event satisfies both flags, it is treated as an abort.
        User_Input in = get_user_input(app, EventOnAnyKey, EventOnEsc|EventOnMouseLeftButton|EventOnMouseRightButton);
        
        // NOTE(allen|a3.4.4): The responsible thing to do on abort is to end the command
        // without waiting on get_user_input again.
        if (in.abort){
            success = false;
            break;
        }
        
        uint8_t character[4];
        u32 length = 0;
        b32 good_character = false;
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
        if (in.key.keycode == '\n' || in.key.keycode == '\t'){
            break;
        }
        else if (in.key.keycode == key_back){
            backspace_utf8(&bar->string);
        }
        else if (good_character){
            append(&bar->string, make_string(character, length));
        }
    }
    
    terminate_with_null(&bar->string);
    
    return(success);
}

static b32
query_user_string(Application_Links *app, Query_Bar *bar){
    return(query_user_general(app, bar, false));
}

static b32
query_user_number(Application_Links *app, Query_Bar *bar){
    return(query_user_general(app, bar, true));
}

static char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    char result = ' ';
    *buffer = get_buffer(app, buffer->buffer_id, AccessAll);
    if (pos < buffer->size){
        buffer_read_range(app, buffer, pos, pos + 1, &result);
    }
    return(result);
}

static Buffer_Identifier
buffer_identifier(char *str, i32 len){
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
make_range(i32 p1, i32 p2){
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
adjust_all_buffer_wrap_widths(Application_Links *app, i32 wrap_width, i32 min_base_width){
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap_width);
        buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, min_base_width);
    }
}

static Buffer_Rect
get_rect(View_Summary *view){
    Buffer_Rect rect = {};
    
    rect.char0 = view->mark.character;
    rect.line0 = view->mark.line;
    
    rect.char1 = view->cursor.character;
    rect.line1 = view->cursor.line;
    
    if (rect.line0 > rect.line1){
        Swap(i32, rect.line0, rect.line1);
    }
    if (rect.char0 > rect.char1){
        Swap(i32, rect.char0, rect.char1);
    }
    
    return(rect);
}

static i32_Rect
get_line_x_rect(View_Summary *view){
    i32_Rect rect = {};
    
    if (view->unwrapped_lines){
        rect.x0 = (i32)view->mark.unwrapped_x;
        rect.x1 = (i32)view->cursor.unwrapped_x;
    }
    else{
        rect.x0 = (i32)view->mark.wrapped_x;
        rect.x1 = (i32)view->cursor.wrapped_x;
    }
    rect.y0 = view->mark.line;
    rect.y1 = view->cursor.line;
    
    if (rect.y0 > rect.y1){
        Swap(i32, rect.y0, rect.y1);
    }
    if (rect.x0 > rect.x1){
        Swap(i32, rect.x0, rect.x1);
    }
    
    return(rect);
}

static View_Summary
get_first_view_with_buffer(Application_Links *app, i32 buffer_id){
    View_Summary result = {};
    View_Summary test = {};
    
    if (buffer_id != 0){
        u32 access = AccessAll;
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

static b32
open_file(Application_Links *app, Buffer_Summary *buffer_out,
          char *filename, i32 filename_len, b32 background, b32 never_new){
    b32 result = false;
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

static Buffer_Summary
buffer_identifier_to_buffer_summary(Application_Links *app, Buffer_Identifier identifier, Access_Flag access){
    Buffer_Summary buffer = {};
    if (identifier.id != 0){
        buffer = get_buffer(app, identifier.id, access);
    }
    else{
        buffer = get_buffer_by_name(app, identifier.name, identifier.name_len, access);
        if (!buffer.exists){
            buffer = get_buffer_by_file_name(app, identifier.name, identifier.name_len, access);
        }
    }
    return(buffer);
}

static b32
view_open_file(Application_Links *app, View_Summary *view,
               char *filename, i32 filename_len, b32 never_new){
    b32 result = false;
    
    if (view != 0){
        Buffer_Summary buffer = {};
        if (open_file(app, &buffer, filename, filename_len, false, never_new)){
            view_set_buffer(app, view, buffer.buffer_id, 0);
            result = true;
        }
    }
    
    return(result);
}

static void
get_view_prev(Application_Links *app, View_Summary *view, u32 access){
    if (view->exists){
        View_ID original_id = view->view_id;
        View_ID check_id = original_id;
        
        View_Summary new_view = {};
        
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

static Buffer_Kill_Result
kill_buffer(Application_Links *app, Buffer_Identifier identifier, View_ID gui_view_id, Buffer_Kill_Flag flags){
    Buffer_Kill_Result result = kill_buffer(app, identifier, flags);
    if (result == BufferKillResult_Dirty){
        Buffer_Summary buffer = buffer_identifier_to_buffer_summary(app, identifier, AccessAll);
        View_Summary view = get_view(app, gui_view_id, AccessAll);
        do_gui_sure_to_kill(app, &buffer, &view);
    }
    return(result);
}

static View_Summary
get_view_last(Application_Links *app, u32 access){
    View_Summary view = {};
    view.exists = true;
    get_view_prev(app, &view, access);
    if (view.view_id < 1 || view.view_id > 16){
        memset(&view, 0, sizeof(view));
    }
    return(view);
}

static void
get_next_view_looped_all_panels(Application_Links *app, View_Summary *view, u32 access){
    get_view_next(app, view, access);
    if (!view->exists){
        *view = get_view_first(app, access);
    }
}

static void
get_prev_view_looped_all_panels(Application_Links *app, View_Summary *view, u32 access){
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
static i32
character_pos_to_pos(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 character_pos){
    i32 result = 0;
    Full_Cursor cursor = {};
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

static b32
read_line(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 line, String *str,
          Partial_Cursor *start_out, Partial_Cursor *one_past_last_out){
    Partial_Cursor begin = {};
    Partial_Cursor end = {};
    
    b32 success = false;
    if (buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &begin)){
        if (buffer_compute_cursor(app, buffer, seek_line_char(line, -1), &end)){
            if (begin.line == line){
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer->size){
                    i32 size = (end.pos - begin.pos);
                    i32 alloc_size = size + 1;
                    char *memory = push_array(part, char, alloc_size);
                    if (memory != 0){
                        *str = make_string(memory, 0, alloc_size);
                        buffer_read_range(app, buffer, begin.pos, end.pos, str->str);
                        str->size = size;
                        terminate_with_null(str);
                        
                        *start_out = begin;
                        *one_past_last_out = end;
                        success = true;
                    }
                }
            }
        }
    }
    
    return(success);
}

static b32
read_line(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 line, String *str){
    Partial_Cursor ignore = {};
    return(read_line(app, part, buffer, line, str, &ignore, &ignore));
}

static String
scratch_read(Application_Links *app, Partition *scratch, Buffer_ID buffer, i32 start, i32 end){
    String result = {};
    if (start <= end){
        i32 len = end - start;
        result = string_push(scratch, len);
        if (buffer_read_range(app, buffer, start, end, result.str)){
            result.size = len;
        }
    }
    return(result);
}

static String
scratch_read(Application_Links *app, Partition *scratch, Buffer_ID buffer, Cpp_Token token){
    String result = scratch_read(app, scratch, buffer, token.start, token.start + token.size);
    return(result);
}

static i32
buffer_get_line_start(Application_Links *app, Buffer_Summary *buffer, i32 line){
    i32 result = buffer->size;
    if (line <= buffer->line_count){
        Partial_Cursor partial_cursor = {};
        buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &partial_cursor);
        result = partial_cursor.pos;
    }
    return(result);
}

static i32
buffer_get_line_end(Application_Links *app, Buffer_Summary *buffer, i32 line){
    i32 result = buffer->size;
    if (line <= buffer->line_count){
        Partial_Cursor partial_cursor = {};
        buffer_compute_cursor(app, buffer, seek_line_char(line, -1), &partial_cursor);
        result = partial_cursor.pos;
    }
    return(result);
}

static i32
buffer_get_line_number(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    Partial_Cursor partial_cursor = {};
    buffer_compute_cursor(app, buffer, seek_pos(pos), &partial_cursor);
    return(partial_cursor.line);
}

static Cpp_Token*
get_first_token_at_line(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 line, i32 *line_start_out = 0){
    i32 line_start = buffer_get_line_start(app, buffer, line);
    Cpp_Get_Token_Result get_token = cpp_get_token(tokens, line_start);
    
    if (get_token.in_whitespace_after_token){
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

static b32
init_stream_chunk(Stream_Chunk *chunk, Application_Links *app, Buffer_Summary *buffer,
                  i32 pos, char *data, u32 size){
    b32 result = false;
    
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

static b32
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    b32 result = 0;
    
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

static b32
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    b32 result = 0;
    
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

static b32
init_stream_tokens(Stream_Tokens_DEP *stream, Application_Links *app, Buffer_Summary *buffer,
                   i32 pos, Cpp_Token *data, i32 count){
    b32 result = false;
    
    refresh_buffer(app, buffer);
    
    i32 token_count = buffer_token_count(app, buffer);
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

static Stream_Tokens_DEP
begin_temp_stream_token(Stream_Tokens_DEP *stream){
    return(*stream);
}

static void
end_temp_stream_token(Stream_Tokens_DEP *stream, Stream_Tokens_DEP temp){
    if (stream->start != temp.start || stream->end != temp.end){
        Application_Links *app = stream->app;
        buffer_read_tokens(app, temp.buffer, temp.start, temp.end, temp.base_tokens);
        stream->tokens = stream->base_tokens - temp.start;
        stream->start = temp.start;
        stream->end = temp.end;
    }
}

static b32
forward_stream_tokens(Stream_Tokens_DEP *stream){
    Application_Links *app = stream->app;
    Buffer_Summary *buffer = stream->buffer;
    b32 result = false;
    
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

static b32
backward_stream_tokens(Stream_Tokens_DEP *stream){
    Application_Links *app = stream->app;
    Buffer_Summary *buffer = stream->buffer;
    b32 result = false;
    
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

////////////////////////////////

static Token_Range
buffer_get_token_range(Application_Links *app, Buffer_ID buffer_id){
    Token_Range range = {};
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    buffer_get_token_range(app, &buffer, &range.first, &range.one_past_last);
    return(range);
}

static Token_Iterator
make_token_iterator(Token_Range range, Cpp_Token *token){
    Token_Iterator iterator = {};
    if (range.first != 0 && range.one_past_last != 0){
        if (token == 0 || token < range.first){
            token = range.first;
        }
        if (token > range.one_past_last){
            token = range.one_past_last;
        }
        iterator.token = token;
        iterator.range = range;
    }
    return(iterator);
}

static Token_Iterator
make_token_iterator(Token_Range range, i32 index){
    return(make_token_iterator(range, range.first + index));
}

static void
token_iterator_set(Token_Iterator *iterator, Cpp_Token *token){
    *iterator = make_token_iterator(iterator->range, token);
}

static Cpp_Token*
token_range_check(Token_Range range, Cpp_Token *token){
    if (token < range.first || range.one_past_last <= token){
        token = 0;
    }
    return(token);
}

static Cpp_Token*
token_iterator_current(Token_Iterator *iterator){
    return(token_range_check(iterator->range, iterator->token));
}

static i32
token_iterator_current_index(Token_Iterator *iterator){
    i32 index = -1;
    Cpp_Token *token = token_iterator_current(iterator);
    if (token != 0 && iterator->range.first <= token && token <= iterator->range.one_past_last){
        index = (i32)(token - iterator->range.first);
    }
    return(index);
}

static Cpp_Token*
token_iterator_goto_next(Token_Iterator *iterator){
    Cpp_Token *token = iterator->token;
    Cpp_Token *one_past_last = iterator->range.one_past_last;
    for (token += 1; token < one_past_last; token += 1){
        if (token->type != CPP_TOKEN_COMMENT){
            break;
        }
    }
    Cpp_Token *result = token_range_check(iterator->range, token);
    *iterator = make_token_iterator(iterator->range, token);
    return(result);
}

static Cpp_Token*
token_iterator_goto_next_raw(Token_Iterator *iterator){
    Cpp_Token *result = token_range_check(iterator->range, iterator->token + 1);
    *iterator = make_token_iterator(iterator->range, iterator->token + 1);
    return(result);
}

static Cpp_Token*
token_iterator_goto_prev(Token_Iterator *iterator){
    Cpp_Token *token = iterator->token;
    Cpp_Token *first = iterator->range.first;
    for (token -= 1; token >= first; token -= 1){
        if (token->type != CPP_TOKEN_COMMENT){
            break;
        }
    }
    Cpp_Token *result = token_range_check(iterator->range, token);
    *iterator = make_token_iterator(iterator->range, token);
    return(result);
}

static Cpp_Token*
token_iterator_goto_prev_raw(Token_Iterator *iterator){
    Cpp_Token *result = token_range_check(iterator->range, iterator->token - 1);
    *iterator = make_token_iterator(iterator->range, iterator->token - 1);
    return(result);
}

static String
token_get_lexeme(Application_Links *app, Buffer_Summary *buffer, Cpp_Token *token, char *out_buffer, i32 out_buffer_size){
    String result = {};
    if (out_buffer_size > 1){
        i32 read_size = token->size;
        if (read_size >= out_buffer_size){
            read_size = out_buffer_size - 1;
        }
        if (buffer_read_range(app, buffer, token->start, token->start + read_size, out_buffer)){
            result = make_string(out_buffer, read_size, out_buffer_size);
            out_buffer[read_size] = 0;
        }
    }
    return(result);
}

static String
token_get_lexeme(Application_Links *app, Partition *part, Buffer_Summary *buffer, Cpp_Token *token){
    String result = {};
    Temp_Memory restore_point = begin_temp_memory(part);
    char *s = push_array(part, char, token->size);
    if (s != 0){
        result = token_get_lexeme(app, buffer, token, s, token->size);
    }
    else{
        end_temp_memory(restore_point);
    }
    return(result);
}

////////////////////////////////

static String
get_query_string(Application_Links *app, char *query_str, char *string_space, i32 space_size){
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
    String str = {};
    Buffer_Summary buffer = get_buffer(app, view->buffer_id, AccessProtected);
    if (!buffer.exists) return(str);
    Range range = get_view_range(view);
    i32 query_length = range.max - range.min;
    if (query_length != 0){
        char *query_space = push_array(arena, char, query_length);
        if (buffer_read_range(app, &buffer, range.min, range.max, query_space)){
            str = make_string(query_space, query_length);
        }
    }
    return(str);
}

static String
get_token_or_word_under_pos(Application_Links *app, Buffer_Summary *buffer, i32 pos, char *space, i32 capacity){
    String result = {};
    Cpp_Get_Token_Result get_result = {};
    b32 success = buffer_get_token_index(app, buffer, pos, &get_result);
    if (success && !get_result.in_whitespace_after_token){
        i32 size = get_result.token_one_past_last - get_result.token_start;
        if (size > 0 && size <= capacity){
            success = buffer_read_range(app, buffer, get_result.token_start, get_result.token_one_past_last, space);
            if (success){
                result = make_string(space, size);
            }
        }
    }
    return(result);
}

static String
build_string(Partition *part, char *s0, char *s1, char *s2){
    String sr = {};
    sr.memory_size = str_size(s0) + str_size(s1) + str_size(s2) + 1;
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
    String sr = {};
    sr.memory_size = str_size(s0) + str_size(s1) + s2.size + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
build_string(Partition *part, char *s0, String s1, char *s2){
    String sr = {};
    sr.memory_size = str_size(s0) + s1.size + str_size(s2) + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
build_string(Partition *part, char *s0, String s1, String s2){
    String sr = {};
    sr.memory_size = str_size(s0) + s1.size + s2.size + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
build_string(Partition *part, String s0, char *s1, char *s2){
    String sr = {};
    sr.memory_size = s0.size + str_size(s1) + str_size(s2) + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
build_string(Partition *part, String s0, char *s1, String s2){
    String sr = {};
    sr.memory_size = s0.size + str_size(s1) + s2.size + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
build_string(Partition *part, String s0, String s1, char *s2){
    String sr = {};
    sr.memory_size = s0.size + s1.size + str_size(s2) + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
build_string(Partition *part, String s0, String s1, String s2){
    String sr = {};
    sr.memory_size = s0.size + s1.size + s2.size + 1;
    sr.str = push_array(part, char, sr.memory_size);
    if (sr.str != 0){
        append(&sr, s0);
        append(&sr, s1);
        append(&sr, s2);
    }
    terminate_with_null(&sr);
    return(sr);
}

static String
string_push_copy(Arena *arena, String str){
    String result = {};
    if (str.str != 0){
        result.str = push_array(arena, char, str.size + 1);
        if (result.str != 0){
            result.memory_size = str.size + 1;
            copy(&result, str);
            result.str[result.size] = 0;
        }
    }
    return(result);
}

static void
append_int_to_str_left_pad(String *str, i32 x, i32 minimum_width, char pad_char){
    i32 length = int_to_str_size(x);
    i32 left_over = minimum_width - length;
    if (left_over > 0){
        append_padding(str, pad_char, str->size + left_over);
    }
    append_int_to_str(str, x);
}

static b32
lexer_keywords_default_init(Partition *arena, Cpp_Keyword_Table *kw_out, Cpp_Keyword_Table *pp_out){
    b32 success = false;
    umem_4tech kw_size = cpp_get_table_memory_size_default(CPP_TABLE_KEYWORDS);
    umem_4tech pp_size = cpp_get_table_memory_size_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES);
    void *kw_mem = push_array(arena, char, (i32_4tech)kw_size);
    void *pp_mem = push_array(arena, char, (i32_4tech)pp_size);
    if (kw_mem != 0 && pp_mem != 0){
        *kw_out = cpp_make_table_default(CPP_TABLE_KEYWORDS, kw_mem, kw_size);
        *pp_out = cpp_make_table_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES, pp_mem, pp_size);
        success = true;
    }
    return(success);
}

////////////////////////////////

static String
get_hot_directory(Application_Links *app, Partition *part){
    Temp_Memory temp = begin_temp_memory(part);
    i32 space_cap = part_remaining(part);
    char *space = push_array(part, char, space_cap);
    String hot_dir = make_string_cap(space, 0, space_cap);
    hot_dir.size = directory_get_hot(app, hot_dir.str, hot_dir.memory_size);
    end_temp_memory(temp);
    push_array(part, char, hot_dir.size);
    hot_dir.memory_size = hot_dir.size;
    return(hot_dir);
}

static String
get_hot_directory(Application_Links *app, Arena *arena){
    i32 space_required = directory_get_hot(app, 0, 0);
    char *space = push_array(arena, char, space_required);
    String hot_dir = make_string_cap(space, 0, space_required);
    hot_dir.size = directory_get_hot(app, hot_dir.str, hot_dir.memory_size);
    return(hot_dir);
}

////////////////////////////////

static String
dump_file_handle(Partition *arena, FILE *file){
    String str = {};
    if (file != 0){
        fseek(file, 0, SEEK_END);
        i32 size = ftell(file);
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

static File_Handle_Path
open_file_search_up_path(Partition *arena, String path, String file_name){
    File_Handle_Path result = {};
    
    i32 cap = path.size + file_name.size + 2;
    char *space = push_array(arena, char, cap);
    push_align(arena, 8);
    String name_str = make_string_cap(space, 0, cap);
    append(&name_str, path);
    if (name_str.size == 0 || !char_is_slash(name_str.str[name_str.size - 1])){
        append(&name_str, "/");
    }
    
    for (;;){
        i32 base_size = name_str.size;
        append(&name_str, file_name);
        terminate_with_null(&name_str);
        result.file = fopen(name_str.str, "rb");
        if (result.file != 0){
            result.path = name_str;
            break;
        }
        
        name_str.size = base_size;
        remove_last_folder(&name_str);
        if (name_str.size >= base_size){
            break;
        }
    }
    
    return(result);
}

static FILE*
open_file_try_current_path_then_binary_path(Application_Links *app, char *file_name){
    FILE *file = fopen(file_name, "rb");
    if (file == 0){
        char space[256];
        i32 size = get_4ed_path(app, space, sizeof(space));
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

static File_Name_Data
dump_file(Partition *arena, String file_name){
    File_Name_Data result = {};
    FILE *file = open_file(arena, file_name);
    if (file != 0){
        result.file_name = file_name;
        result.data = dump_file_handle(arena, file);
        fclose(file);
    }
    return(result);
}

static File_Name_Path_Data
dump_file_search_up_path(Partition *arena, String path, String file_name){
    File_Name_Path_Data result = {};
    File_Handle_Path file = open_file_search_up_path(arena, path, file_name);
    if (file.file != 0){
        result.file_name = file_name;
        result.path = file.path;
        result.data = dump_file_handle(arena, file.file);
        fclose(file.file);
    }
    return(result);
}

static void
sort_pairs_by_key__quick(Sort_Pair_i32 *pairs, i32 first, i32 one_past_last){
    i32 dif = one_past_last - first;
    if (dif >= 2){
        i32 pivot = one_past_last - 1;
        Sort_Pair_i32 pivot_pair = pairs[pivot];
        i32 j = first;
        b32 interleave = false;
        for (i32 i = first; i < pivot; i += 1){
            Sort_Pair_i32 pair = pairs[i];
            if (pair.key < pivot_pair.key){
                pairs[i] = pairs[j];
                pairs[j] = pair;
                j += 1;
            }
            else if (pair.key == pivot_pair.key){
                if (interleave){
                    pairs[i] = pairs[j];
                    pairs[j] = pair;
                    j += 1;
                }
                interleave = !interleave;
            }
        }
        pairs[pivot] = pairs[j];
        pairs[j] = pivot_pair;
        sort_pairs_by_key__quick(pairs, first, j);
        sort_pairs_by_key__quick(pairs, j + 1, one_past_last);
    }
}

static void
sort_pairs_by_key(Sort_Pair_i32 *pairs, i32 count){
    sort_pairs_by_key__quick(pairs, 0, count);
}

static Range_Array
get_ranges_of_duplicate_keys(Partition *arena, i32 *keys, i32 stride, i32 count){
    Range_Array result = {};
    result.ranges = push_array(arena, Range, 0);
    uint8_t *ptr = (uint8_t*)keys;
    i32 start_i = 0;
    for (i32 i = 1; i <= count; i += 1){
        b32 is_end = false;
        if (i == count){
            is_end = true;
        }
        else if (*(i32*)(ptr + i*stride) != *(i32*)(ptr + start_i*stride)){
            is_end = true;
        }
        if (is_end){
            Range *new_range = push_array(arena, Range, 1);
            new_range->first = start_i;
            new_range->one_past_last = i;
            start_i = i;
        }
    }
    result.count = (i32)(push_array(arena, Range, 0) - result.ranges);
    return(result);
}

static void
no_mark_snap_to_cursor(Application_Links *app, Managed_Scope view_scope){
    managed_variable_set(app, view_scope, view_snap_mark_to_cursor, false);
}

static void
no_mark_snap_to_cursor(Application_Links *app, View_ID view_id){
    Managed_Scope scope = view_get_managed_scope(app, view_id);
    no_mark_snap_to_cursor(app, scope);
}

static void
no_mark_snap_to_cursor_if_shift(Application_Links *app, View_ID view_id){
    User_Input in = get_command_input(app);
    if (in.key.modifiers[MDFR_SHIFT_INDEX]){
        no_mark_snap_to_cursor(app, view_id);
    }
}

static b32
view_has_highlighted_range(Application_Links *app, View_ID view_id){
    if (fcoder_mode == FCoderMode_NotepadLike){
        View_Summary view = get_view(app, view_id, AccessAll);
        return(view.cursor.pos != view.mark.pos);
    }
    return(false);
}

static b32
if_view_has_highlighted_range_delete_range(Application_Links *app, View_ID view_id){
    if (view_has_highlighted_range(app, view_id)){
        View_Summary view = get_view(app, view_id, AccessAll);
        Range range = get_view_range(&view);
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
        return(true);
    }
    return(false);
}

static void
begin_notepad_mode(Application_Links *app){
    fcoder_mode = FCoderMode_NotepadLike;
    for (View_Summary view = get_view_first(app, AccessAll);
         view.exists;
         get_view_next(app, &view, AccessAll)){
        view_set_mark(app, &view, seek_pos(view.cursor.pos));
    }
}

////////////////////////////////

static b32
view_set_split_proportion(Application_Links *app, View_Summary *view, float t){
    return(view_set_split(app, view, ViewSplitKind_Ratio, t));
}

static b32
view_set_split_pixel_size(Application_Links *app, View_Summary *view, i32 t){
    return(view_set_split(app, view, ViewSplitKind_FixedPixels, (float)t));
}

////////////////////////////////

static Record_Info
get_single_record(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Record_Info record = {};
    buffer_history_get_record_info(app, buffer_id, index, &record);
    if (record.error == RecordError_NoError && record.kind == RecordKind_Group){
        buffer_history_get_group_sub_record(app, buffer_id, index, record.group.count, &record);
    }
    return(record);
}

////////////////////////////////

static void
view_buffer_set(Application_Links *app, Buffer_ID *buffers, i32 count){
    // TODO(allen): do(implement view_buffer_set)
}

////////////////////////////////

static void
condense_whitespace(String *a)
{
    *a = skip_chop_whitespace(*a);
    
    int size = a->size;
    a->size = 0;
    int i = 0;
    while(i < size)
    {
        if(char_is_whitespace(a->str[i]))
        {
            a->str[a->size++] = ' ';
            while((i < size) && char_is_whitespace(a->str[i]))
            {
                ++i;
            }
        }
        else
        {
            a->str[a->size++] = a->str[i++];
        }
    }
}

////////////////////////////////

static Vec2
draw_string(Application_Links *app, Face_ID font_id, String string, Vec2 p, int_color color){
    return(draw_string(app, font_id, string, p, color, 0, V2(1.f, 0.f)));
}

static void
draw_margin(Application_Links *app, f32_Rect outer, f32_Rect inner, int_color color){
    draw_rectangle(app, f32R(outer.x0, outer.y0, outer.x1, inner.y0), color);
    draw_rectangle(app, f32R(outer.x0, inner.y1, outer.x1, outer.y1), color);
    draw_rectangle(app, f32R(outer.x0, inner.y0, inner.x0, inner.y1), color);
    draw_rectangle(app, f32R(inner.x1, inner.y0, outer.x1, inner.y1), color);
}

// BOTTOM

