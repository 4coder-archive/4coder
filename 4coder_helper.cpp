/*
 * Miscellaneous helpers for common operations.
 */

// TOP

static String
string_push(Arena *arena, i32 size){
    String result = {};
    if (size != 0){
        result.str = push_array(arena, char, size);
        if (result.str != 0){
            result.memory_size = size;
        }
    }
    return(result);
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

static String
string_push_fv(Arena *arena, char *format, va_list args){
    char temp[KB(4)];
    i32 n = vsnprintf(temp, sizeof(temp), format, args);
    String result = {};
    if (0 <= n && n < sizeof(temp)){
        result = string_push_copy(arena, make_string(temp, n));
    }
    return(result);
}

static String
string_push_fv(Partition *part, char *format, va_list args){
    char temp[KB(4)];
    i32 n = vsnprintf(temp, sizeof(temp), format, args);
    String result = {};
    if (0 <= n && n < sizeof(temp)){
        result = string_push_copy(part, make_string(temp, n));
    }
    return(result);
}

static String
string_push_f(Arena *arena, char *format, ...){
    va_list args;
    va_start(args, format);
    String result = string_push_fv(arena, format, args);
    va_end(args);
    return(result);
}

static String
string_push_f(Partition *part, char *format, ...){
    va_list args;
    va_start(args, format);
    String result = string_push_fv(part, format, args);
    va_end(args);
    return(result);
}

////////////////////////////////

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
buffer_get_char(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    char result = ' ';
    if (pos < buffer_size){
        buffer_read_range(app, buffer_id, pos, pos + 1, &result);
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

static void
adjust_all_buffer_wrap_widths(Application_Links *app, i32 wrap_width, i32 min_base_width){
    Buffer_ID buffer = 0;
    for (get_buffer_next(app, 0, AccessAll, &buffer);
         buffer != 0;
         get_buffer_next(app, buffer, AccessAll, &buffer)){
        buffer_set_setting(app, buffer, BufferSetting_WrapPosition, wrap_width);
        buffer_set_setting(app, buffer, BufferSetting_MinimumBaseWrapPosition, min_base_width);
    }
}

static View_ID
get_first_view_with_buffer(Application_Links *app, Buffer_ID buffer_id){
    View_ID result = {};
    View_ID test = {};
    if (buffer_id != 0){
        for (get_view_next(app, 0, AccessAll, &test);
             test != 0;
             get_view_next(app, test, AccessAll, &test)){
            Buffer_ID test_buffer = 0;
            view_get_buffer(app, test, AccessAll, &test_buffer);
            if (test_buffer == buffer_id){
                result = test;
                break;
            }
        }
    }
    return(result);
}

static b32
open_file(Application_Links *app, Buffer_ID *buffer_out, char *filename, i32 filename_len, b32 background, b32 never_new){
    b32 result = false;
    Buffer_ID buffer = 0;
    get_buffer_by_name(app, filename, filename_len, AccessProtected);
    b32 exists = buffer_exists(app, buffer);
    if (!exists){
        Buffer_Create_Flag flags = 0;
        if (background){
            flags |= BufferCreate_Background;
        }
        if (never_new){
            flags |= BufferCreate_NeverNew;
        }
        create_buffer(app, make_string(filename, filename_len), flags, &buffer);
        exists = buffer_exists(app, buffer);
    }
    if (exists){
        if (buffer_out != 0){
            *buffer_out = buffer;
        }
        result = true;
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
        String name = make_string(identifier.name, identifier.name_len);
        get_buffer_by_name(app, name, AccessAll, &id);
        if (id == 0){
            get_buffer_by_file_name(app, name, AccessAll, &id);
        }
    }
    return(id);
}

static b32
view_open_file(Application_Links *app, View_ID view, char *filename, i32 filename_len, b32 never_new){
    b32 result = false;
    if (view != 0){
        Buffer_ID buffer = 0;
        if (open_file(app, &buffer, filename, filename_len, false, never_new)){
            view_set_buffer(app, view, buffer, 0);
            result = true;
        }
    }
    return(result);
}

////////////////////////////////

// TODO(allen): replace this with get_view_prev(app, 0, access, view_id_out);
static b32
get_view_last(Application_Links *app, Access_Flag access, View_ID *view_id_out){
    return(get_view_prev(app, 0, access, view_id_out));
}

static View_ID
get_next_view_looped_all_panels(Application_Links *app, View_ID view_id, Access_Flag access){
    if (!get_view_next(app, view_id, access, &view_id)){
        if (!get_view_next(app, 0, access, &view_id)){
            view_id = 0;
        }
    }
    return(view_id);
}

static View_ID
get_prev_view_looped_all_panels(Application_Links *app, View_ID view_id, Access_Flag access){
    if (!get_view_prev(app, view_id, access, &view_id)){
        if (!get_view_prev(app, 0, access, &view_id)){
            view_id = 0;
        }
    }
    return(view_id);
}

////

static void
get_view_prev(Application_Links *app, View_Summary *view, Access_Flag access){
    View_ID new_id = 0;
    get_view_prev(app, view->view_id, access, &new_id);
    get_view_summary(app, new_id, access, view);
}

static View_Summary
get_view_last(Application_Links *app, Access_Flag access){
    View_Summary view = {};
    View_ID new_id = 0;
    if (get_view_last(app, access, &new_id)){
        get_view_summary(app, new_id, access, &view);
    }
    return(view);
}

static void
get_next_view_looped_all_panels(Application_Links *app, View_Summary *view, Access_Flag access){
    View_ID new_id = get_next_view_looped_all_panels(app, view->view_id, access);
    get_view_summary(app, new_id, access, view);
}

static void
get_prev_view_looped_all_panels(Application_Links *app, View_Summary *view, u32 access){
    View_ID new_id = get_prev_view_looped_all_panels(app, view->view_id, access);
    get_view_summary(app, new_id, access, view);
}

////////////////////////////////

static Buffer_Kill_Result
kill_buffer(Application_Links *app, Buffer_Identifier identifier, View_ID gui_view_id, Buffer_Kill_Flag flags){
    Buffer_Kill_Result result = kill_buffer(app, identifier, flags);
    if (result == BufferKillResult_Dirty){
        Buffer_ID buffer = buffer_identifier_to_id(app, identifier);
        do_gui_sure_to_kill(app, buffer, gui_view_id);
    }
    return(result);
}

static void
refresh_view(Application_Links *app, View_Summary *view){
    *view = get_view(app, view->view_id, AccessAll);
}

static i32
character_pos_to_pos(Application_Links *app, View_ID view, i32 character_pos){
    i32 result = 0;
    Full_Cursor cursor = {};
    if (view_compute_cursor(app, view, seek_character_pos(character_pos), &cursor)){
        result = cursor.pos;
    }
    return(result);
}

static f32
get_view_y(Application_Links *app, View_ID view){
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(pos), &cursor);
    return(cursor.wrapped_y);
}

static f32
get_view_x(Application_Links *app, View_ID view){
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(pos), &cursor);
    return(cursor.wrapped_x);
}

static Range
get_view_range(Application_Links *app, View_ID view){
    Range range = {};
    view_get_cursor_pos(app, view, &range.first);
    view_get_mark_pos(app, view, &range.one_past_last);
    return(rectify(range));
}

static String
buffer_read_string(Application_Links *app, Buffer_ID buffer, Range range, void *dest) {
    String result = {};
    if (dest != 0 && buffer_read_range(app, buffer, range.start, range.end, (char *)dest)){
        result = make_string((char *)dest, get_width(range));
    }
    return(result);
}

static b32
read_line(Application_Links *app, Partition *part, Buffer_ID buffer_id, i32 line, String *str,
          Partial_Cursor *start_out, Partial_Cursor *one_past_last_out){
    Partial_Cursor begin = {};
    Partial_Cursor end = {};
    
    b32 success = false;
    if (buffer_compute_cursor(app, buffer_id, seek_line_char(line, 1), &begin) &&
        buffer_compute_cursor(app, buffer_id, seek_line_char(line, -1), &end) &&
        begin.line == line){
        i32 buffer_size = 0;
        buffer_get_size(app, buffer_id, &buffer_size);
        if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer_size){
            i32 size = (end.pos - begin.pos);
            i32 alloc_size = size + 1;
            char *memory = push_array(part, char, alloc_size);
            if (memory != 0){
                *str = make_string(memory, 0, alloc_size);
                buffer_read_range(app, buffer_id, begin.pos, end.pos, str->str);
                str->size = size;
                terminate_with_null(str);
                
                *start_out = begin;
                *one_past_last_out = end;
                success = true;
            }
        }
    }
    
    return(success);
}

static b32
read_line(Application_Links *app, Partition *part, Buffer_ID buffer_id, i32 line, String *str){
    Partial_Cursor ignore = {};
    return(read_line(app, part, buffer_id, line, str, &ignore, &ignore));
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
scratch_read(Application_Links *app, Partition *scratch, Buffer_ID buffer, Range range){
    return(scratch_read(app, scratch, buffer, range.first, range.one_past_last));
}

static String
scratch_read(Application_Links *app, Arena *scratch, Buffer_ID buffer, i32 start, i32 end){
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
scratch_read(Application_Links *app, Arena *scratch, Buffer_ID buffer, Range range){
    return(scratch_read(app, scratch, buffer, range.first, range.one_past_last));
}

static String
scratch_read(Application_Links *app, Partition *scratch, Buffer_ID buffer, Cpp_Token token){
    String result = scratch_read(app, scratch, buffer, token.start, token.start + token.size);
    return(result);
}

static b32
token_match(Application_Links *app, Buffer_ID buffer, Cpp_Token token, String b){
    Arena *scratch = context_get_arena(app);
    Temp_Memory_Arena temp = begin_temp_memory(scratch);
    String a = scratch_read(app, scratch, buffer, make_range(token.start, token.start + token.size));
    b32 result = match(a, b);
    end_temp_memory(temp);
    return(result);
}

static String
read_entire_buffer(Application_Links *app, Buffer_ID buffer_id, Arena *scratch){
    i32 size = 0;
    buffer_get_size(app, buffer_id, &size);
    return(scratch_read(app, scratch, buffer_id, 0, size));
}

static i32
buffer_get_line_number(Application_Links *app, Buffer_ID buffer, i32 pos){
    Partial_Cursor partial_cursor = {};
    buffer_compute_cursor(app, buffer, seek_pos(pos), &partial_cursor);
    return(partial_cursor.line);
}

static i32
view_get_line_number(Application_Links *app, View_ID view, i32 pos){
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(pos), &cursor);
    return(cursor.line);
}

static i32
buffer_get_line_start(Application_Links *app, Buffer_ID buffer_id, i32 line){
    i32 result = 0;
    buffer_get_size(app, buffer_id, &result);
    i32 line_count = 0;
    buffer_get_line_count(app, buffer_id, &line_count);
    if (line <= line_count){
        Partial_Cursor partial_cursor = {};
        buffer_compute_cursor(app, buffer_id, seek_line_char(line, 1), &partial_cursor);
        result = partial_cursor.pos;
    }
    return(result);
}

static i32
buffer_get_line_end(Application_Links *app, Buffer_ID buffer_id, i32 line){
    i32 result = 0;
    buffer_get_size(app, buffer_id, &result);
    i32 line_count = 0;
    buffer_get_line_count(app, buffer_id, &line_count);
    if (line <= line_count){
        Partial_Cursor partial_cursor = {};
        buffer_compute_cursor(app, buffer_id, seek_line_char(line, -1), &partial_cursor);
        result = partial_cursor.pos;
    }
    return(result);
}

static Cpp_Token*
get_first_token_at_line(Application_Links *app, Buffer_ID buffer_id, Cpp_Token_Array tokens, i32 line, i32 *line_start_out = 0){
    i32 line_start = buffer_get_line_start(app, buffer_id, line);
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

static void
clear_buffer(Application_Links *app, Buffer_ID buffer_id){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    buffer_replace_range(app, buffer_id, make_range(0, buffer_size), make_lit_string(""));
}

////////////////////////////////

static b32
init_stream_chunk(Stream_Chunk *chunk, Application_Links *app, Buffer_ID buffer_id,
                  i32 pos, char *data, u32 size){
    b32 result = false;
    
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    if (0 <= pos && pos < buffer_size && size > 0){
        chunk->app = app;
        chunk->buffer_id = buffer_id;
        chunk->base_data = data;
        chunk->data_size = size;
        chunk->start = round_down(pos, size);
        chunk->end = round_up(pos, size);
        
        if (chunk->max_end > buffer_size || chunk->max_end == 0){
            chunk->max_end = buffer_size;
        }
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer_id, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = true;
        }
    }
    
    return(result);
}

static b32
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_ID buffer_id = chunk->buffer_id;
    b32 result = 0;
    
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    if (chunk->end < buffer_size){
        chunk->start = chunk->end;
        chunk->end += chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            buffer_read_range(app, buffer_id, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = 1;
        }
    }
    
    else if (chunk->add_null && chunk->end + 1 < buffer_size){
        chunk->start = buffer_size;
        chunk->end = buffer_size + 1;
        chunk->base_data[0] = 0;
        chunk->data = chunk->base_data - chunk->start;
        result = 1;
    }
    
    return(result);
}

static b32
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_ID buffer_id = chunk->buffer_id;
    b32 result = false;
    
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
            buffer_read_range(app, buffer_id, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = true;
        }
    }
    
    else if (chunk->add_null && chunk->start > -1){
        chunk->start = -1;
        chunk->end = 0;
        chunk->base_data[0] = 0;
        chunk->data = chunk->base_data - chunk->start;
        result = true;
    }
    
    return(result);
}

////////////////////////////////

static b32
init_stream_tokens(Stream_Tokens_DEP *stream, Application_Links *app, Buffer_ID buffer_id, i32 pos, Cpp_Token *data, i32 count){
    b32 result = false;
    
    i32 token_count = 0;
    buffer_token_count(app, buffer_id, &token_count);
    if (buffer_tokens_are_ready(app, buffer_id) && pos >= 0 && pos < token_count && count > 0){
        stream->app = app;
        stream->buffer_id = buffer_id;
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
        
        buffer_read_tokens(app, buffer_id, stream->start, stream->end, stream->base_tokens);
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
        buffer_read_tokens(app, temp.buffer_id, temp.start, temp.end, temp.base_tokens);
        stream->tokens = stream->base_tokens - temp.start;
        stream->start = temp.start;
        stream->end = temp.end;
    }
}

static b32
forward_stream_tokens(Stream_Tokens_DEP *stream){
    Application_Links *app = stream->app;
    Buffer_ID buffer_id = stream->buffer_id;
    b32 result = false;
    if (stream->end < stream->token_count){
        stream->start = stream->end;
        stream->end += stream->count;
        if (stream->end > stream->token_count){
            stream->end = stream->token_count;
        }
        if (stream->start < stream->end){
            buffer_read_tokens(app, buffer_id, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = true;
        }
    }
    return(result);
}

static b32
backward_stream_tokens(Stream_Tokens_DEP *stream){
    Application_Links *app = stream->app;
    Buffer_ID buffer_id = stream->buffer_id;
    b32 result = false;
    if (stream->start > 0){
        stream->end = stream->start;
        stream->start -= stream->count;
        if (0 > stream->start){
            stream->start = 0;
        }
        if (stream->start < stream->end){
            buffer_read_tokens(app, buffer_id, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = true;
        }
    }
    return(result);
}

////////////////////////////////

static Token_Range
buffer_get_token_range(Application_Links *app, Buffer_ID buffer){
    Token_Range range = {};
    buffer_get_token_range(app, buffer, &range.first, &range.one_past_last);
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
token_get_lexeme(Application_Links *app, Buffer_ID buffer, Cpp_Token *token, char *out_buffer, i32 out_buffer_size){
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
token_get_lexeme(Application_Links *app, Partition *part, Buffer_ID buffer, Cpp_Token *token){
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
    Buffer_ID buffer = 0;
    view_get_buffer(app, view->view_id, AccessProtected, &buffer);
    if (buffer_exists(app, buffer)){
        Range range = get_view_range(app, view->view_id);
        i32 query_length = range.max - range.min;
        if (query_length != 0){
            char *query_space = push_array(arena, char, query_length);
            if (buffer_read_range(app, buffer, range.min, range.max, query_space)){
                str = make_string(query_space, query_length);
            }
        }
    }
    return(str);
}

static String
get_token_or_word_under_pos(Application_Links *app, Buffer_ID buffer, i32 pos, char *space, i32 capacity){
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
    b32 result = false;
    if (view_has_highlighted_range(app, view_id)){
        View_Summary view = get_view(app, view_id, AccessAll);
        Range range = get_view_range(app, view_id);
        Buffer_ID buffer = 0;
        view_get_buffer(app, view.view_id, AccessOpen, &buffer);
        buffer_replace_range(app, buffer, range, make_lit_string(""));
        result = true;
    }
    return(result);
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
view_set_split_proportion(Application_Links *app, View_Summary *view, f32 t){
    return(view_set_split(app, view, ViewSplitKind_Ratio, t));
}

static b32
view_set_split_pixel_size(Application_Links *app, View_Summary *view, i32 t){
    return(view_set_split(app, view, ViewSplitKind_FixedPixels, (f32)t));
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
condense_whitespace(String *a)
{
    *a = skip_chop_whitespace(*a);
    int size = a->size;
    a->size = 0;
    int i = 0;
    for (;i < size;){
        if (char_is_whitespace(a->str[i])){
            a->str[a->size++] = ' ';
            for (;(i < size) && char_is_whitespace(a->str[i]);){
                ++i;
            }
        }
        else{
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

////////////////////////////////

static f32_Rect_Pair
split_rect(f32_Rect rect, View_Split_Kind kind, Coordinate coord, Side from_side, f32 t){
    f32_Rect_Pair result = {};
    if (kind == ViewSplitKind_FixedPixels){
        result.E[0] = rect;
        result.E[1] = rect;
        if (coord == Coordinate_X){
            result.E[0].x1 = (from_side == Side_Max) ? (rect.x1 - t) : (rect.x0 + t);
            result.E[1].x0 = result.E[0].x1;
        }
        else{
            Assert(coord == Coordinate_Y);
            result.E[0].y1 = (from_side == Side_Max) ? (rect.y1 - t) : (rect.y0 + t);
            result.E[1].y0 = result.E[0].y1;
        }
    }
    else{
        Assert(kind == ViewSplitKind_Ratio);
        f32 pixel_count;
        if (coord == Coordinate_X){
            pixel_count = t*(rect.x1 - rect.x0);
        }
        else{
            Assert(coord == Coordinate_Y);
            pixel_count = t*(rect.y1 - rect.y0);
        }
        result = split_rect(rect, ViewSplitKind_FixedPixels, coord, from_side, pixel_count);
    }
    return(result);
}


////////////////////////////////

static String
buffer_push_base_buffer_name(Application_Links *app, Buffer_ID buffer, Arena *arena){
    String result = {};
    buffer_get_base_buffer_name(app, buffer, 0, &result.memory_size);
    result.str = push_array(arena, char, result.memory_size);
    buffer_get_base_buffer_name(app, buffer, &result, 0);
    return(result);
}

static String
buffer_push_unique_buffer_name(Application_Links *app, Buffer_ID buffer, Arena *arena){
    String result = {};
    buffer_get_unique_buffer_name(app, buffer, 0, &result.memory_size);
    result.str = push_array(arena, char, result.memory_size);
    buffer_get_unique_buffer_name(app, buffer, &result, 0);
    return(result);
}

static String
buffer_push_file_name(Application_Links *app, Buffer_ID buffer, Arena *arena){
    String result = {};
    buffer_get_file_name(app, buffer, 0, &result.memory_size);
    result.str = push_array(arena, char, result.memory_size);
    buffer_get_file_name(app, buffer, &result, 0);
    return(result);
}

static String
buffer_limited_base_buffer_name(Application_Links *app, Buffer_ID buffer, char *memory, i32 max){
    String result = {};
    result.str = memory;
    result.memory_size = max;
    buffer_get_base_buffer_name(app, buffer, &result, 0);
    return(result);
}

static String
buffer_limited_unique_buffer_name(Application_Links *app, Buffer_ID buffer, char *memory, i32 max){
    String result = {};
    result.str = memory;
    result.memory_size = max;
    buffer_get_unique_buffer_name(app, buffer, &result, 0);
    return(result);
}

static String
buffer_limited_file_name(Application_Links *app, Buffer_ID buffer, char *memory, i32 max){
    String result = {};
    result.str = memory;
    result.memory_size = max;
    buffer_get_file_name(app, buffer, &result, 0);
    return(result);
}

////////////////////////////////

static b32
buffer_has_name_with_star(Application_Links *app, Buffer_ID buffer){
    char first = 0;
    String str = buffer_limited_unique_buffer_name(app, buffer, &first, 1);
    return(str.size == 0 || first == '*');
}

////////////////////////////////

static f32
get_dpi_scaling_value(Application_Links *app)
{
    // TODO(casey): Allen, this should return the multiplier for the display relative to whatever 4coder
    // gets tuned to.
    f32 result = 2.0f;
    return(result);
}

// BOTTOM

