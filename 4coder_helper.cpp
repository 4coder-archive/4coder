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

static u32
get_key_code(char *buffer){
    u32 ignore;
    u32 result = utf8_to_u32_length_unchecked((u8*)buffer, &ignore);
    return(result);
}

////////////////////////////////

internal Character_Predicate
character_predicate_from_function(Character_Predicate_Function *func){
    Character_Predicate predicate = {};
    i32 byte_index = 0;
    for (u32 i = 0; i <= 255;){
        b8 v[8];
        for (i32 bit_index = 0; bit_index < 8; i += 1, bit_index += 1){
            v[bit_index] = func((u8)i);
        }
        predicate.b[byte_index] = (
            (v[0] << 0) |
            (v[1] << 1) |
            (v[2] << 2) |
            (v[3] << 3) |
            (v[4] << 4) |
            (v[5] << 5) |
            (v[6] << 6) |
            (v[7] << 7)
            );
        byte_index += 1;
    }
    return(predicate);
}

internal Character_Predicate
character_predicate_from_character(u8 character){
    Character_Predicate predicate = {};
    predicate.b[character/8] = (1 << (character%8));
    return(predicate);
}

#define character_predicate_check_character(p, c) (((p).b[(c)/8] & (1 << ((c)%8))) != 0)

internal Character_Predicate
character_predicate_or(Character_Predicate *a, Character_Predicate *b){
    Character_Predicate p = {};
    for (i32 i = 0; i < ArrayCount(p.b); i += 1){
        p.b[i] = a->b[i] | b->b[i];
    }
    return(p);
}

internal Character_Predicate
character_predicate_and(Character_Predicate *a, Character_Predicate *b){
    Character_Predicate p = {};
    for (i32 i = 0; i < ArrayCount(p.b); i += 1){
        p.b[i] = a->b[i] & b->b[i];
    }
    return(p);
}

internal Character_Predicate
character_predicate_not(Character_Predicate *a){
    Character_Predicate p = {};
    for (i32 i = 0; i < ArrayCount(p.b); i += 1){
        p.b[i] = ~(a->b[i]);
    }
    return(p);
}

internal i32
buffer_seek_character_class_change__inner(Application_Links *app, Buffer_ID buffer, Character_Predicate *positive, Character_Predicate *negative, Scan_Direction direction, i32 start_pos){
    i32 pos = start_pos;
    switch (direction){
        case Scan_Backward:
        {
            b32 s1 = buffer_seek_character_class(app, buffer, negative, direction, pos, &pos);
            b32 s2 = buffer_seek_character_class(app, buffer, positive, direction, pos, &pos);
            if (s1 && s2){
                pos += 1;
            }
        }break;
        case Scan_Forward:
        {
            pos -= 1;
            buffer_seek_character_class(app, buffer, positive, direction, pos, &pos);
            buffer_seek_character_class(app, buffer, negative, direction, pos, &pos);
        }break;
    }
    return(pos);
}

internal i32
buffer_seek_character_class_change_1_0(Application_Links *app, Buffer_ID buffer, Character_Predicate *predicate, Scan_Direction direction, i32 start_pos){
    Character_Predicate negative = character_predicate_not(predicate);
    return(buffer_seek_character_class_change__inner(app, buffer,
                                                     predicate, &negative,
                                                     direction, start_pos));
}

internal i32
buffer_seek_character_class_change_0_1(Application_Links *app, Buffer_ID buffer, Character_Predicate *predicate, Scan_Direction direction, i32 start_pos){
    Character_Predicate negative = character_predicate_not(predicate);
    return(buffer_seek_character_class_change__inner(app, buffer,
                                                     &negative, predicate,
                                                     direction, start_pos));
}

////////////////////////////////

static String_Const_u8
push_hot_directory(Application_Links *app, Arena *arena){
    String_Const_u8 result = {};
    get_hot_directory(app, arena, &result);
    return(result);
}

static String_Const_u8
push_4ed_path(Application_Links *app, Arena *arena){
    String_Const_u8 result = {};
    get_4ed_path(app, arena, &result);
    return(result);
}

static String_Const_u8
push_buffer_base_name(Application_Links *app, Arena *arena, Buffer_ID buffer){
    String_Const_u8 result = {};
    buffer_get_base_buffer_name(app, buffer, arena, &result);
    return(result);
}

static String_Const_u8
push_buffer_unique_name(Application_Links *app, Arena *arena, Buffer_ID buffer){
    String_Const_u8 result = {};
    buffer_get_unique_buffer_name(app, buffer, arena, &result);
    return(result);
}

static String_Const_u8
push_buffer_file_name(Application_Links *app, Arena *arena, Buffer_ID buffer){
    String_Const_u8 result = {};
    buffer_get_file_name(app, buffer, arena, &result);
    return(result);
}

////////////////////////////////

static Range
get_view_range(Application_Links *app, View_ID view){
    Range range = {};
    view_get_cursor_pos(app, view, &range.first);
    view_get_mark_pos(app, view, &range.one_past_last);
    return(rectify(range));
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

static b32
is_valid_line(Application_Links *app, Buffer_ID buffer_id, i32 line){
    i32 max_line = 0;
    buffer_get_line_count(app, buffer_id, &max_line);
    return(1 <= line && line <= max_line);
}

static b32
is_valid_line_range(Application_Links *app, Buffer_ID buffer_id, Range range){
    i32 max_line = 0;
    buffer_get_line_count(app, buffer_id, &max_line);
    return(1 <= range.first && range.first <= range.end && range.end <= max_line);
}

static i32
get_line_number_from_pos(Application_Links *app, Buffer_ID buffer, i32 pos){
    Partial_Cursor partial_cursor = {};
    buffer_compute_cursor(app, buffer, seek_pos(pos), &partial_cursor);
    return(partial_cursor.line);
}

static i32
character_pos_to_pos_view(Application_Links *app, View_ID view, i32 character_pos){
    i32 result = 0;
    Full_Cursor cursor = {};
    if (view_compute_cursor(app, view, seek_character_pos(character_pos), &cursor)){
        result = cursor.pos;
    }
    return(result);
}

static i32
character_pos_to_pos_buffer(Application_Links *app, Buffer_ID buffer, i32 character_pos){
    i32 result = 0;
    Partial_Cursor cursor = {};
    if (buffer_compute_cursor(app, buffer, seek_character_pos(character_pos), &cursor)){
        result = cursor.pos;
    }
    return(result);
}

static Partial_Cursor
get_line_side(Application_Links *app, Buffer_ID buffer, i32 line_number, Side side){
    Partial_Cursor result = {};
    i32 character_index = (side == Side_Min)?(1):(-1);
    if (!buffer_compute_cursor(app, buffer, seek_line_char(line_number, character_index), &result)){
        block_zero_struct(&result);
    }
    return(result);
}
static i32
get_line_side_pos(Application_Links *app, Buffer_ID buffer, i32 line_number, Side side){
    i32 pos = -1;
    Partial_Cursor partial_cursor = get_line_side(app, buffer, line_number, side);
    if (partial_cursor.line != 0){
        pos = partial_cursor.pos;
    }
    return(pos);
}

static Partial_Cursor
get_line_start(Application_Links *app, Buffer_ID buffer, i32 line_number){
    return(get_line_side(app, buffer, line_number, Side_Min));
}
static i32
get_line_start_pos(Application_Links *app, Buffer_ID buffer, i32 line_number){
    return(get_line_side_pos(app, buffer, line_number, Side_Min));
}

// NOTE(allen): The position returned has the index of the terminating newline character,
// not one past the newline character.
static Partial_Cursor
get_line_end(Application_Links *app, Buffer_ID buffer, i32 line_number){
    return(get_line_side(app, buffer, line_number, Side_Max));
}
static i32
get_line_end_pos(Application_Links *app, Buffer_ID buffer, i32 line_number){
    return(get_line_side_pos(app, buffer, line_number, Side_Max));
}

static i32
get_line_pos(Application_Links *app, Buffer_ID buffer, i32 line_number, Side side){
    i32 result = 0;
    if (side == Side_Min){
        result = get_line_start_pos(app, buffer, line_number);
    }
    else{
        result = get_line_start_pos(app, buffer, line_number);
    }
}

// NOTE(allen): The range returned does not include the terminating newline character
static Range_Partial_Cursor
get_line_range(Application_Links *app, Buffer_ID buffer, i32 line_number){
    b32 success = false;
    Range_Partial_Cursor result = {};
    result.begin = get_line_start(app, buffer, line_number);
    if (result.begin.line != 0){
        result.end = get_line_end(app, buffer, line_number);
        if (result.end.line != 0){
            success = true;
        }
    }
    if (!success){
        block_zero_struct(&result);
    }
    return(result);
}

// NOTE(allen): The range returned does not include the terminating newline character
static Range
get_line_pos_range(Application_Links *app, Buffer_ID buffer, i32 line_number){
    Range_Partial_Cursor range = get_line_range(app, buffer, line_number);
    Range result = {};
    if (range.begin.line != 0 && range.end.line != 0){
        result = make_range(range.begin.pos, range.end.pos);
    }
    return(result);
}

static Range
make_range_from_cursors(Range_Partial_Cursor range){
    return(make_range(range.begin.pos, range.end.pos));
}

static i32
get_line_side_pos_from_pos(Application_Links *app, Buffer_ID buffer, i32 pos, Side side){
    i32 line_number = get_line_number_from_pos(app, buffer, pos);
    return(get_line_side_pos(app, buffer, line_number, side));
}
static i32
get_line_start_pos_from_pos(Application_Links *app, Buffer_ID buffer, i32 pos){
    return(get_line_side_pos_from_pos(app, buffer, pos, Side_Min));
}
static i32
get_line_end_pos_from_pos(Application_Links *app, Buffer_ID buffer, i32 pos){
    return(get_line_side_pos_from_pos(app, buffer, pos, Side_Max));
}

static Cpp_Token*
get_first_token_from_pos(Cpp_Token_Array tokens, i32 pos){
    Cpp_Get_Token_Result get_token = cpp_get_token(tokens, pos);
    if (get_token.in_whitespace_after_token){
        get_token.token_index += 1;
    }
    Cpp_Token *result = 0;
    if (get_token.token_index < tokens.count){
        result = tokens.tokens + get_token.token_index;
    }
    return(result);
}

static Cpp_Token*
get_first_token_from_line(Application_Links *app, Buffer_ID buffer, Cpp_Token_Array tokens, i32 line){
    i32 line_start = get_line_start_pos(app, buffer, line);
    return(get_first_token_from_pos(tokens, line_start));
}

////////////////////////////////

static i32
scan_any_boundary(Application_Links *app, Boundary_Function *func, Buffer_ID buffer, Scan_Direction direction, i32 pos){
    i32 a = func(app, buffer, Side_Min, direction, pos);
    i32 b = func(app, buffer, Side_Max, direction, pos);
    i32 result = 0;
    if (direction == Scan_Forward){
        result = Min(a, b);
    }
    else{
        result = Max(a, b);
    }
    return(result);
}

static i32
scan(Application_Links *app, Boundary_Function *func, Buffer_ID buffer, Scan_Direction direction, i32 pos){
    Side side = (direction == Scan_Forward)?(Side_Max):(Side_Min);
    return(func(app, buffer, side, direction, pos));
}

static i32
scan(Application_Links *app, Boundary_Function_List funcs, Buffer_ID buffer, Scan_Direction direction, i32 start_pos){
    i32 result = 0;
    if (direction == Scan_Forward){
        i32 size = 0;
        buffer_get_size(app, buffer, &size);
        result = size + 1;
        for (Boundary_Function_Node *node = funcs.first;
             node != 0;
             node = node->next){
            i32 pos = scan(app, node->func, buffer, direction, start_pos);
            result = Min(result, pos);
        }
    }
    else{
        result = -1;
                                                                        for (Boundary_Function_Node *node = funcs.first;
                                                                             node != 0;
                                                                             node = node->next){
            i32 pos = scan(app, node->func, buffer, direction, start_pos);
            result = Max(result, pos);
        }
    }
    return(result);
}

static void
push_boundary(Arena *arena, Boundary_Function_List *list, Boundary_Function *func){
    Boundary_Function_Node *node = push_array(arena, Boundary_Function_Node, 1);
    sll_queue_push(list->first, list->last, node);
    list->count += 1;
    node->func = func;
}

static Boundary_Function_List
push_boundary_list__innerv(Arena *arena, va_list args){
    Boundary_Function_List list = {};
    for (;;){
        Boundary_Function *func = va_arg(args, Boundary_Function*);
        if (func == 0){
            break;
        }
        push_boundary(arena, &list, func);
    }
    return(list);
}
static Boundary_Function_List
push_boundary_list__inner(Arena *arena, ...){
    va_list args;
    va_start(args, arena);
    Boundary_Function_List result = push_boundary_list__innerv(arena, args);
    va_end(args);
    return(result);
}
#define push_boundary_list(a,...) push_boundary_list__inner((a), __VA_ARGS__, 0)

static i32
boundary_predicate(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos, Character_Predicate *predicate){
    i32 result = 0;
    switch (side){
        case Side_Min:
        {
            result = buffer_seek_character_class_change_0_1(app, buffer, predicate, direction, pos);
        }break;
        case Side_Max:
        {
            result = buffer_seek_character_class_change_1_0(app, buffer, predicate, direction, pos);
        }break;
    }
    return(result);
}

static i32
boundary_non_whitespace(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    return(boundary_predicate(app, buffer, side, direction, pos, &character_predicate_non_whitespace));
}

static i32
boundary_alpha_numeric(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    return(boundary_predicate(app, buffer, side, direction, pos, &character_predicate_alpha_numeric));
}

static i32
boundary_alpha_numeric_unicode(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    return(boundary_predicate(app, buffer, side, direction, pos, &character_predicate_alpha_numeric_underscore_utf8));
}

static i32
boundary_alpha_numeric_underscore(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    return(boundary_predicate(app, buffer, side, direction, pos,
                              &character_predicate_alpha_numeric_underscore));
}

static i32
boundary_alpha_numeric_camel(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    i32 an_pos = boundary_alpha_numeric(app, buffer, side, direction, pos);
    i32 cap_pos = 0;
    buffer_seek_character_class(app, buffer, &character_predicate_uppercase,
                                direction, pos, &cap_pos);
    if (side == Side_Max){
        i32 an_left_pos = boundary_alpha_numeric(app, buffer, flip_side(side),
                                                 flip_direction(direction), an_pos);
        if (cap_pos == an_left_pos){
            buffer_seek_character_class(app, buffer, &character_predicate_uppercase,
                                        direction, cap_pos, &cap_pos);
        }
    }
    i32 result = 0;
    if (direction == Scan_Backward){
        result = Max(an_pos, cap_pos);
    }
    else{
        result = Min(an_pos, cap_pos);
    }
    return(result);
}

static i32
boundary_token(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    i32 result = 0;
    if (!buffer_tokens_are_ready(app, buffer)){
        result = boundary_non_whitespace(app, buffer, side, direction, pos);
    }
    else{
        Cpp_Token *first_token = 0;
        Cpp_Token *last_token = 0;
        buffer_get_token_range(app, buffer, &first_token, &last_token);
        Cpp_Token_Array tokens = {first_token, (i32)(last_token - first_token)};
        switch (direction){
            case Scan_Forward:
            {
                i32 buffer_size = 0;
                buffer_get_size(app, buffer, &buffer_size);
                if (tokens.count > 0){
                    Cpp_Token *token = get_first_token_from_pos(tokens, pos);
                    if (token != 0){
                        if (side == Side_Max){
                            result = token->start + token->size;
                        }
                        else{
                            if (token->start > pos){
                                result = token->start;
                            }
                            else{
                                token += 1;
                                if (token < tokens.tokens + tokens.count){
                                    result = token->start;
                                }
                                else{
                                    result = buffer_size;
                                }
                            }
                        }
                    }
                    else{
                        result = buffer_size;
                    }
                }
                else{
                    result = buffer_size;
                }
            }break;
            
            case Scan_Backward:
            {
                if (tokens.count > 0){
                    Cpp_Token *token = get_first_token_from_pos(tokens, pos);
                    if (side == Side_Min){
                        if (token == 0){
                            token = tokens.tokens + tokens.count - 1;
                            result = token->start;
                        }
                        else{
                            if (token->start < pos){
                                result = token->start;
                            }
                            else{
                                token -= 1;
                                if (token >= tokens.tokens){
                                    result = token->start;
                                }
                                else{
                                    result = 0;
                                }
                            }
                        }
                    }
                    else{
                        if (token == 0){
                            token = tokens.tokens + tokens.count - 1;
                            result = token->start + token->size;
                        }
                        else{
                            token -= 1;
                            if (token >= tokens.tokens && token->start + token->size == pos){
                                token -= 1;
                            }
                            if (token >= tokens.tokens){
                                result = token->start + token->size;
                            }
                            else{
                                result = 0;
                            }
                        }
                    }
                }
                else{
                    result = 0;
                }
            }break;
        }
    }
    
    return(result);
}

static i32
boundary_line(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos){
    i32 line_number = get_line_number_from_pos(app, buffer, pos);
    i32 new_pos = get_line_side_pos(app, buffer, line_number, side);
    if (direction == Scan_Backward && new_pos >= pos){
        if (line_number > 1){
            new_pos = get_line_side_pos(app, buffer, line_number - 1, side);
        }
        else{
            new_pos = 0;
        }
    }
    else if (direction == Scan_Forward && new_pos <= pos){
        new_pos = get_line_side_pos(app, buffer, line_number + 1, side);
        if (new_pos <= pos){
            buffer_get_size(app, buffer, &new_pos);
        }
    }
    return(new_pos);
}

////////////////////////////////

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_ID buffer, i32 pos, char delim, i32 *result){
    Character_Predicate predicate = character_predicate_from_character((u8)delim);
    buffer_seek_character_class(app, buffer, &predicate, Scan_Forward, pos, result);
}

void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_ID buffer, i32 pos, char delim, i32 *result){
    Character_Predicate predicate = character_predicate_from_character((u8)delim);
    buffer_seek_character_class(app, buffer, &predicate, Scan_Backward, pos, result);
}

static void
buffer_seek_string_forward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 end, String_Const_u8 needle, i32 *result){
    if (end == 0){
        buffer_get_size(app, buffer, &end);
    }
    b32 case_sensitive = false;
    b32 finding_matches = false;
    do{
        finding_matches = 
            buffer_seek_string(app, buffer, needle, Scan_Forward, pos, &pos, &case_sensitive);
    } while(!case_sensitive && pos < end && finding_matches);
    if (pos < end && finding_matches){
        *result = pos;
    }
    else{
        buffer_get_size(app, buffer, result);
    }
}

static void
buffer_seek_string_backward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 min, String_Const_u8 needle, i32 *result){
    b32 case_sensitive = false;
    b32 finding_matches = false;
    do{
        finding_matches = 
            buffer_seek_string(app, buffer, needle, Scan_Backward, pos, &pos, &case_sensitive);
    } while(!case_sensitive && pos >= min && finding_matches);
    if (pos >= min && finding_matches){
        *result = pos;
    }
    else{
        *result = -1;
    }
}

static void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 end, String_Const_u8 needle, i32 *result){
    if (end == 0){
        buffer_get_size(app, buffer, &end);
    }
    b32 finding_matches = false;
    b32 case_sensitive = false;
    finding_matches = 
        buffer_seek_string(app, buffer, needle, Scan_Forward, pos, &pos, &case_sensitive);
    if (pos < end && finding_matches){
        *result = pos;
    }
    else{
        buffer_get_size(app, buffer, result);
    }
}

static void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_ID buffer, i32 pos, i32 min, String_Const_u8 needle, i32 *result){
    b32 finding_matches = false;
    b32 case_sensitive = false;
    finding_matches = 
        buffer_seek_string(app, buffer, needle, Scan_Backward, pos, &pos, &case_sensitive);
    if (pos >= min && finding_matches){
        *result = pos;
    }
    else{
        *result = -1;
    }
}

static void
buffer_seek_string(Application_Links *app, Buffer_ID buffer_id, i32 pos, i32 end, i32 min, String_Const_u8 str, i32 *result, Buffer_Seek_String_Flags flags){
    switch (flags & 3){
        case 0:
        {
            buffer_seek_string_forward(app, buffer_id, pos, end, str, result);
        }break;
        
        case BufferSeekString_Backward:
        {
            buffer_seek_string_backward(app, buffer_id, pos, min, str, result);
        }break;
        
        case BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_forward(app, buffer_id, pos, end, str, result);
        }break;
        
        case BufferSeekString_Backward|BufferSeekString_CaseInsensitive:
        {
            buffer_seek_string_insensitive_backward(app, buffer_id, pos, min, str, result);
        }break;
    }
}

////////////////////////////////

static Range
get_line_range_from_pos_range(Application_Links *app, Buffer_ID buffer, Range pos_range){
    Range line_range = {};
    line_range.first = get_line_number_from_pos(app, buffer, pos_range.first);
    if (line_range.first != 0){
        line_range.end = get_line_number_from_pos(app, buffer, pos_range.one_past_last);
        if (line_range.end == 0){
            line_range.first = 0;
        }
    }
    return(line_range);
}

// NOTE(allen): The end of the returned range does not include the terminating newline character of
// the last line.
static Range
get_pos_range_from_line_range(Application_Links *app, Buffer_ID buffer, Range line_range){
    Range pos_range = {};
    if (is_valid_line_range(app, buffer, line_range)){
        pos_range.first = get_line_start_pos(app, buffer, line_range.first);
        pos_range.one_past_last = get_line_end_pos(app, buffer, line_range.end);
    }
    return(pos_range);
}

static Range
enclose_boundary(Application_Links *app, Buffer_ID buffer, Range range, Boundary_Function *func){
    i32 new_min       = func(app, buffer, Side_Min, Scan_Backward, range.min + 1);
    i32 new_min_check = func(app, buffer, Side_Max, Scan_Backward, range.min + 1);
    if (new_min_check <= new_min && new_min < range.min){
        range.min = new_min;
    }
    i32 new_max       = func(app, buffer, Side_Max, Scan_Forward, range.max - 1);
    i32 new_max_check = func(app, buffer, Side_Min, Scan_Forward, range.max - 1);
    if (new_max_check >= new_max && new_max > range.max){
        range.max = new_max;
    }
    return(range);
}

static Range
enclose_non_whitespace(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_non_whitespace));
}

static Range
enclose_tokens(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_token));
}

static Range
enclose_alpha_numeric(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_alpha_numeric));
}

static Range
enclose_alpha_numeric_unicode(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_alpha_numeric_unicode));
}

static Range
enclose_alpha_numeric_underscore(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_alpha_numeric_underscore));
}

static Range
enclose_alpha_numeric_camel(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_alpha_numeric_camel));
}

static Range
pos_range_enclose_whole_lines(Application_Links *app, Buffer_ID buffer, Range range){
    return(enclose_boundary(app, buffer, range, boundary_line));
}

////////////////////////////////

static Range
get_snipe_range(Application_Links *app, Boundary_Function_List funcs, Buffer_ID buffer, i32 pos, Scan_Direction direction){
    Range result = {};
    i32 buffer_size = 0;
    buffer_get_size(app, buffer, &buffer_size);
    i32 pos0 = pos;
    i32 pos1 = scan(app, funcs, buffer, direction, pos0);
    if (0 <= pos1 && pos1 <= buffer_size){
        i32 pos2 = scan(app, funcs, buffer, flip_direction(direction), pos1);
        if (0 <= pos2 && pos2 <= buffer_size){
            if (direction == Scan_Backward){
                pos2 = clamp_bot(pos2, pos0);
            }
            else{
                pos2 = clamp_top(pos2, pos0);
            }
            result = make_range(pos1, pos2);
        }
    }
    return(result);
}

static Range
get_snipe_range(Application_Links *app, Boundary_Function *func, Buffer_ID buffer, i32 pos, Scan_Direction direction){
    Scratch_Block scratch(app);
    return(get_snipe_range(app, push_boundary_list(scratch, func), buffer, pos, direction));
}

////////////////////////////////

static String_Const_u8
push_buffer_range(Application_Links *app, Arena *arena, Buffer_ID buffer, umem first, umem one_past_last){
    String_Const_u8 result = {};
    if (first <= one_past_last){
        umem length = one_past_last - first;
        Temp_Memory restore_point = begin_temp(arena);
        u8 *memory = push_array(arena, u8, length);
        if (buffer_read_range(app, buffer, (i32)first, (i32)one_past_last, (char*)memory)){
            result = SCu8(memory, length);
        }
        else{
            end_temp(restore_point);
        }
    }
    return(result);
}

static String_Const_u8
push_buffer_range(Application_Links *app, Arena *arena, Buffer_ID buffer, Range range){
    return(push_buffer_range(app, arena, buffer, range.first, range.one_past_last));
}

static String_Const_u8
push_buffer_range(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_umem range){
    return(push_buffer_range(app, arena, buffer, range.first, range.one_past_last));
}

static String_Const_u8
push_token_lexeme(Application_Links *app, Arena *arena, Buffer_ID buffer, Cpp_Token token){
    return(push_buffer_range(app, arena, buffer, token.start, token.start + token.size));
}

static String_Const_u8
push_buffer_line(Application_Links *app, Arena *arena, Buffer_ID buffer, i32 line_number){
    return(push_buffer_range(app, arena, buffer, get_line_pos_range(app, buffer, line_number)));
}

static String_Const_u8
push_whole_buffer(Application_Links *app, Arena *arena, Buffer_ID buffer){
    i32 size = 0;
    buffer_get_size(app, buffer, &size);
    return(push_buffer_range(app, arena, buffer, 0, size));
}

static String_Const_u8
push_string_in_view_range(Application_Links *app, Arena *arena, View_ID view){
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    return(push_buffer_range(app, arena, buffer, get_view_range(app, view)));
}

static String_Const_u8
push_enclose_range_at_pos(Application_Links *app, Arena *arena, Buffer_ID buffer, i32 pos, Enclose_Function *enclose){
    Range range = enclose(app, buffer, make_range(pos));
    return(push_buffer_range(app, arena, buffer, range));
}

////////////////////////////////

static b32
buffer_has_name_with_star(Application_Links *app, Buffer_ID buffer){
    Scratch_Block scratch(app);
    String_Const_u8 str = push_buffer_unique_name(app, scratch, buffer);
    return(str.size > 0 && str.str[0] == '*');
}

static char
buffer_get_char(Application_Links *app, Buffer_ID buffer_id, i32 pos){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    char result = ' ';
    if (0 <= pos && pos < buffer_size){
        buffer_read_range(app, buffer_id, pos, pos + 1, &result);
    }
    return(result);
}

static b32
token_lexeme_string_match(Application_Links *app, Buffer_ID buffer, Cpp_Token token, String_Const_u8 b){
    Scratch_Block scratch(app);
    return(string_match(push_token_lexeme(app, scratch, buffer, token), b));
}

static b32
line_is_valid_and_blank(Application_Links *app, Buffer_ID buffer, i32 line_number){
    b32 result = false;
    if (is_valid_line(app, buffer, line_number)){
        Scratch_Block scratch(app);
        String_Const_u8 line = push_buffer_line(app, scratch, buffer, line_number);
        result = true;
        for (umem i = 0; i < line.size; i += 1){
            if (!character_is_whitespace(line.str[i])){
                result = false;
                break;
            }
        }
    }
    return(result);
}

////////////////////////////////

static i32
get_pos_past_lead_whitespace(Application_Links *app, Buffer_ID buffer, i32 pos){
    Scratch_Block scratch(app);
    i32 line_number = get_line_number_from_pos(app, buffer, pos);
    Range line_range = get_line_pos_range(app, buffer, line_number);
    String_Const_u8 line = push_buffer_range(app, scratch, buffer, line_range);
    i32 result = line_range.end;
    for (umem i = 0; i < line.size; i += 1){
        if (!character_is_whitespace(line.str[i])){
            result = line_range.start + (i32)i;
            break;
        }
    }
    result = clamp_bot(result, pos);
    return(result);
}

static void
move_past_lead_whitespace(Application_Links *app, View_ID view, Buffer_ID buffer){
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 new_pos = get_pos_past_lead_whitespace(app, buffer, pos);
    view_set_cursor(app, view, seek_pos(new_pos), true);
}

static void
move_past_lead_whitespace(Application_Links *app, View_ID view){
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    move_past_lead_whitespace(app, view, buffer);
}

static b32
line_is_blank(Application_Links *app, Buffer_ID buffer, i32 line_number){
    Scratch_Block scratch(app);
    String_Const_u8 line = push_buffer_line(app, scratch, buffer, line_number);
    b32 is_blank = true;
    for (umem i = 0; i < line.size; i += 1){
        if (!character_is_whitespace(line.str[i])){
            is_blank = false;
            break;
        }
    }
    return(is_blank);
}

static i32
get_line_number_of__whitespace_status_line(Application_Links *app, Buffer_ID buffer, Scan_Direction direction, i32 line_number_start, b32 get_blank_line){
    i32 line_count = 0;
    buffer_get_line_count(app, buffer, &line_count);
    i32 line_number = line_number_start + direction;
    for (;1 <= line_number && line_number <= line_count; line_number += direction){
        b32 is_blank = line_is_blank(app, buffer, line_number);
        if (is_blank == get_blank_line){
            break;
        }
    }
    line_number = clamp(1, line_number, line_count);
    return(line_number);
}

static i32
get_line_number_of_non_blank_line(Application_Links *app, Buffer_ID buffer, Scan_Direction direction, i32 line_number_start){
    return(get_line_number_of__whitespace_status_line(app, buffer, direction, line_number_start, false));
}

static i32
get_line_number_of_blank_line(Application_Links *app, Buffer_ID buffer, Scan_Direction direction, i32 line_number_start){
    return(get_line_number_of__whitespace_status_line(app, buffer, direction, line_number_start, true));
}

static i32
get_pos_of_blank_line(Application_Links *app, Buffer_ID buffer, Scan_Direction direction, i32 pos_start){
    i32 line_number_start = get_line_number_from_pos(app, buffer, pos_start);
    i32 blank_line = get_line_number_of_blank_line(app, buffer, direction, line_number_start);
    i32 pos = get_line_start_pos(app, buffer, blank_line);
    return(pos);
}

static i32
get_line_number_of_blank_line_grouped(Application_Links *app, Buffer_ID buffer, Scan_Direction direction, i32 line_number_start){
    i32 line_number = line_number_start;
    if (line_is_blank(app, buffer, line_number)){
        line_number = get_line_number_of_non_blank_line(app, buffer, direction, line_number);
    }
    line_number = get_line_number_of_blank_line(app, buffer, direction, line_number);
    return(line_number);
}

static i32
get_pos_of_blank_line_grouped(Application_Links *app, Buffer_ID buffer, Scan_Direction direction, i32 pos_start){
    i32 line_number_start = get_line_number_from_pos(app, buffer, pos_start);
    i32 blank_line = get_line_number_of_blank_line_grouped(app, buffer, direction, line_number_start);
    i32 pos = get_line_start_pos(app, buffer, blank_line);
    return(pos);
}

////////////////////////////////

static History_Group
history_group_begin(Application_Links *app, Buffer_ID buffer){
    History_Group group = {};
    group.app = app;
    group.buffer = buffer;
    buffer_history_get_current_state_index(app, buffer, &group.first);
    group.first += 1;
    return(group);
}

static void
history_group_end(History_Group group){
    History_Record_Index last = 0;
    buffer_history_get_current_state_index(group.app, group.buffer, &last);
    if (group.first < last){
        buffer_history_merge_record_range(group.app, group.buffer, group.first, last, RecordMergeFlag_StateInRange_MoveStateForward);
    }
}

////////////////////////////////

static void
replace_in_range(Application_Links *app, Buffer_ID buffer, Range range, String_Const_u8 needle, String_Const_u8 string){
    History_Group group = history_group_begin(app, buffer);
    i32 pos = range.min - 1;
    i32 new_pos = 0;
    buffer_seek_string_forward(app, buffer, pos, range.end, needle, &new_pos);
    i32 shift = replace_range_compute_shift((i32)needle.size, (i32)string.size);
    for (; new_pos + (imem)needle.size <= range.end;){
        Range needle_range = make_range(new_pos, new_pos + (i32)needle.size);
        buffer_replace_range(app, buffer, needle_range, string);
        range.end += shift;
        pos = new_pos + (i32)string.size - 1;
        buffer_seek_string_forward(app, buffer, pos, range.end, needle, &new_pos);
    }
    history_group_end(group);
}

internal Range
swap_lines(Application_Links *app, Buffer_ID buffer, i32 line_1, i32 line_2){
    Range result = {};
    i32 line_count = 0;
    buffer_get_line_count(app, buffer, &line_count);
    if (1 <= line_1 && line_2 <= line_count){
        Range range_1 = get_line_pos_range(app, buffer, line_1);
        Range range_2 = get_line_pos_range(app, buffer, line_2);
        
        Scratch_Block scratch(app);
        
        String_Const_u8 text_1 = push_buffer_range(app, scratch, buffer, range_1);
        String_Const_u8 text_2 = push_buffer_range(app, scratch, buffer, range_2);
        
        History_Group group = history_group_begin(app, buffer);
        buffer_replace_range(app, buffer, range_2, text_1);
        buffer_replace_range(app, buffer, range_1, text_2);
        history_group_end(group);
        
        i32 shift = replace_range_compute_shift(range_1, (i32)text_2.size);
        result.min = range_1.min;
        result.max = range_2.min + shift;
    }
    return(result);
}

internal i32
move_line(Application_Links *app, Buffer_ID buffer, i32 line_number, Scan_Direction direction){
    i32 line_1 = 0;
    i32 line_2 = 0;
    if (direction == Scan_Forward){
        line_1 = line_number;
        line_2 = line_number + 1;
    }
    else{
        line_1 = line_number - 1;
        line_2 = line_number;
    }
    Range line_starts = swap_lines(app, buffer, line_1, line_2);
    i32 result = 0;
    if (line_starts.min < line_starts.max){
        if (direction == Scan_Forward){
            result = line_starts.max;
        }
        else{
            result = line_starts.min;
        }
    }
    else{
        result = get_line_side_pos(app, buffer, line_number, Side_Min);
    }
    return(result);
}

internal void
move_line_current_view(Application_Links *app, Scan_Direction direction){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 line_number = get_line_number_from_pos(app, buffer, pos);
    pos = move_line(app, buffer, line_number, direction);
    view_set_cursor(app, view, seek_pos(pos), true);
}

////////////////////////////////

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

static String_Const_u8
backspace_utf8(String_Const_u8 string){
    if (string.size > 0){
        umem i = string.size - 1;
        for (; i > 0; --i){
            if (string.str[i] <= 0x7F || string.str[i] >= 0xC0){
                break;
            }
        }
        string.size = i;
    }
    return(string);
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
            bar->string = backspace_utf8(bar->string);
        }
        else if (good_character){
            String_u8 string = Su8(bar->string.str, bar->string.size, bar->string_capacity);
            string_append(&string, SCu8(character, length));
            bar->string = string.string;
        }
    }
    
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

////////////////////////////////

static Buffer_Identifier
buffer_identifier(char *str, i32 len){
    Buffer_Identifier identifier;
    identifier.name = str;
    identifier.name_len = len;
    identifier.id = 0;
    return(identifier);
}

static Buffer_Identifier
buffer_identifier(String_Const_u8 str){
    return(buffer_identifier((char*)str.str, (i32)str.size));
}

static Buffer_Identifier
buffer_identifier(Buffer_ID id){
    Buffer_Identifier identifier;
    identifier.name = 0;
    identifier.name_len = 0;
    identifier.id = id;
    return(identifier);
}

static Buffer_ID
buffer_identifier_to_id(Application_Links *app, Buffer_Identifier identifier){
    Buffer_ID id = 0;
    if (identifier.id != 0){
        id = identifier.id;
    }
    else{
        String_Const_u8 name = SCu8(identifier.name, identifier.name_len);
        get_buffer_by_name(app, name, AccessAll, &id);
        if (id == 0){
            get_buffer_by_file_name(app, name, AccessAll, &id);
        }
    }
    return(id);
}

static Buffer_ID
buffer_identifier_to_id_create_out_buffer(Application_Links *app, Buffer_Identifier buffer_id){
    Buffer_ID result = 0;
    if (buffer_id.name != 0 && buffer_id.name_len > 0){
        String_Const_u8 buffer_name = SCu8(buffer_id.name, buffer_id.name_len);
        Buffer_ID buffer_attach_id = 0;
        if (get_buffer_by_name(app, buffer_name, AccessAll, &buffer_attach_id)){
            result = buffer_attach_id;
        }
        else{
            if (create_buffer(app, buffer_name, BufferCreate_AlwaysNew|BufferCreate_NeverAttachToFile, &buffer_attach_id)){
                buffer_set_setting(app, buffer_attach_id, BufferSetting_ReadOnly, true);
                buffer_set_setting(app, buffer_attach_id, BufferSetting_Unimportant, true);
                result = buffer_attach_id;
            }
        }
    }
    else{
        result = buffer_id.id;
    }
    return(result);
}

////////////////////////////////

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
open_view(Application_Links *app, View_ID view_location, View_Split_Position position){
    View_ID result = 0;
    if (view_location != 0 && view_exists(app, view_location)){
        Panel_ID panel_id = 0;
        if (view_get_panel(app, view_location, &panel_id)){
            b32 vertical = (position == ViewSplit_Left || position == ViewSplit_Right);
            if (panel_split(app, panel_id, vertical?PanelSplit_LeftAndRight:PanelSplit_TopAndBottom)){
                Panel_ID new_panel_id = 0;
                Panel_Child child = (position == ViewSplit_Left || position == ViewSplit_Top)?PanelChild_Min:PanelChild_Max;
                if (panel_get_child(app, panel_id, child, &new_panel_id)){
                    View_ID new_view_id = 0;
                    if (panel_get_view(app, new_panel_id, &new_view_id)){
                        result = new_view_id;
                    }
                }
            }
        }
    }
    return(result);
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
open_file(Application_Links *app, Buffer_ID *buffer_out, String_Const_u8 file_name, b32 background, b32 never_new){
    b32 result = false;
    Buffer_ID buffer = 0;
    get_buffer_by_name(app, file_name, AccessProtected, &buffer);
    b32 exists = buffer_exists(app, buffer);
    if (!exists){
        Buffer_Create_Flag flags = 0;
        if (background){
            flags |= BufferCreate_Background;
        }
        if (never_new){
            flags |= BufferCreate_NeverNew;
        }
        create_buffer(app, file_name, flags, &buffer);
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

static b32
view_open_file(Application_Links *app, View_ID view, String_Const_u8 file_name, b32 never_new){
    b32 result = false;
    if (view != 0){
        Buffer_ID buffer = 0;
        if (open_file(app, &buffer, file_name, false, never_new)){
            view_set_buffer(app, view, buffer, 0);
            result = true;
        }
    }
    return(result);
}

////////////////////////////////

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

////////////////////////////////

static Buffer_Kill_Result
kill_buffer(Application_Links *app, Buffer_Identifier identifier, View_ID gui_view_id, Buffer_Kill_Flag flags){
    Buffer_ID buffer = buffer_identifier_to_id(app, identifier);
    Buffer_Kill_Result result = 0;
    buffer_kill(app, buffer, flags, &result);
    if (result == BufferKillResult_Dirty){
        do_gui_sure_to_kill(app, buffer, gui_view_id);
    }
    return(result);
}

////////////////////////////////

static void
clear_buffer(Application_Links *app, Buffer_ID buffer_id){
    i32 buffer_size = 0;
    buffer_get_size(app, buffer_id, &buffer_size);
    buffer_replace_range(app, buffer_id, make_range(0, buffer_size), string_u8_litexpr(""));
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
        chunk->start = round_down_i32(pos, size);
        chunk->end = round_up_i32(pos, size);
        
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
        stream->start = round_down_i32(pos, count);
        stream->end = round_up_i32(pos, count);
        stream->token_count = token_count;
        
        stream->start = clamp_bot(0, stream->start);
        stream->end = clamp_top(stream->end, stream->token_count);
        
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

////////////////////////////////

static String_Const_u8
get_query_string(Application_Links *app, char *query_str, char *string_space, i32 space_size){
    Query_Bar bar;
    bar.prompt = SCu8((u8*)query_str);
    bar.string = SCu8((u8*)string_space, (umem)0);
    bar.string_capacity = space_size;
    if (!query_user_string(app, &bar)){
        bar.string.size = 0;
    }
    return(bar.string);
}

static String_Const_u8
get_token_or_word_under_pos(Application_Links *app, Arena *arena, Buffer_ID buffer, umem pos){
    String_Const_u8 result = {};
    Cpp_Get_Token_Result get_result = {};
    b32 success = buffer_get_token_index(app, buffer, (i32)pos, &get_result);
    if (success && !get_result.in_whitespace_after_token){
        umem size = get_result.token_one_past_last - get_result.token_start;
        if (0 < size){
            result = push_buffer_range(app, arena, buffer, get_result.token_start, get_result.token_one_past_last);
        }
    }
    return(result);
}

static b32
lexer_keywords_default_init(Arena *arena, Cpp_Keyword_Table *kw_out, Cpp_Keyword_Table *pp_out){
    b32 success = false;
    umem kw_size = cpp_get_table_memory_size_default(CPP_TABLE_KEYWORDS);
    umem pp_size = cpp_get_table_memory_size_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES);
    void *kw_mem = push_array(arena, char, kw_size);
    void *pp_mem = push_array(arena, char, pp_size);
    if (kw_mem != 0 && pp_mem != 0){
        *kw_out = cpp_make_table_default(CPP_TABLE_KEYWORDS, kw_mem, kw_size);
        *pp_out = cpp_make_table_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES, pp_mem, pp_size);
        success = true;
    }
    return(success);
}

////////////////////////////////

static b32
file_exists(Application_Links *app, String_Const_u8 file_name){
    File_Attributes attributes = {};
    file_get_attributes(app, file_name, &attributes);
    return(attributes.last_write_time > 0);
}

static b32
file_exists_and_is_file(Application_Links *app, String_Const_u8 file_name){
    File_Attributes attributes = {};
    file_get_attributes(app, file_name, &attributes);
    return(attributes.last_write_time > 0 &&
           !HasFlag(attributes.flags, FileAttribute_IsDirectory));
}

static b32
file_exists_and_is_folder(Application_Links *app, String_Const_u8 file_name){
    File_Attributes attributes = {};
    file_get_attributes(app, file_name, &attributes);
    return(attributes.last_write_time > 0 &&
           HasFlag(attributes.flags, FileAttribute_IsDirectory));
}

static Data
dump_file_handle(Arena *arena, FILE *file){
    Data result = {};
    if (file != 0){
        fseek(file, 0, SEEK_END);
        umem size = ftell(file);
        char *mem = push_array(arena, char, size);
        if (mem != 0){
            fseek(file, 0, SEEK_SET);
            fread(mem, 1, size, file);
            result = make_data(mem, size);
        }
    }
    return(result);
}

static String_Const_u8
push_file_search_up_path(Application_Links *app, Arena *arena, String_Const_u8 start_path, String_Const_u8 file_name){
    String_Const_u8 result = {};
    String_Const_u8 path = start_path;
    for (;path.size > 0;){
        Temp_Memory temp = begin_temp(arena);
        if (character_is_slash(string_get_character(path, path.size - 1))){
            path = string_chop(path, 1);
        }
        String_Const_u8 full_path = string_u8_pushf(arena, "%.*s/%.*s",
                                                    string_expand(path),
                                                    string_expand(file_name));
        if (file_exists(app, full_path)){
            result = full_path;
            break;
        }
        path = string_remove_last_folder(path);
        end_temp(temp);
    }
    return(result);
}

static FILE*
open_file_try_current_path_then_binary_path(Application_Links *app, char *file_name){
    FILE *file = fopen(file_name, "rb");
    if (file == 0){
        Scratch_Block scratch(app);
        List_String_Const_u8 list = {};
        string_list_push(scratch, &list, push_4ed_path(app, scratch));
        string_list_push_overlap(scratch, &list, '/', SCu8(file_name));
        String_Const_u8 str = string_list_flatten(scratch, list, StringFill_NullTerminate);
        file = fopen((char*)str.str, "rb");
    }
    return(file);
}

static FILE*
open_file(Arena *scratch, String_Const_u8 name){
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 name_copy = string_copy(scratch, name);
    FILE *file = fopen((char*)name_copy.str, "rb");
    end_temp(temp);
    return(file);
}

static File_Name_Data
dump_file(Arena *arena, String_Const_u8 file_name){
    File_Name_Data result = {};
    FILE *file = open_file(arena, file_name);
    if (file != 0){
        result.file_name = file_name;
        result.data = dump_file_handle(arena, file);
        fclose(file);
    }
    return(result);
}

static File_Name_Data
dump_file_search_up_path(Application_Links *app, Arena *arena, String_Const_u8 path, String_Const_u8 file_name){
    File_Name_Data result = {};
    String_Const_u8 full_path = push_file_search_up_path(app, arena, path, file_name);
    if (full_path.size > 0){
        result = dump_file(arena, full_path);
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
get_ranges_of_duplicate_keys(Arena *arena, i32 *keys, i32 stride, i32 count){
    Range_Array result = {};
    result.ranges = push_array(arena, Range, count);
    u8 *ptr = (u8*)keys;
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
            Range *new_range = &result.ranges[result.count++];
            new_range->first = start_i;
            new_range->one_past_last = i;
            start_i = i;
        }
    }
    pop_array(arena, Range, count - result.count);
    return(result);
}

static void
no_mark_snap_to_cursor(Application_Links *app, Managed_Scope view_scope){
    managed_variable_set(app, view_scope, view_snap_mark_to_cursor, false);
}

static void
no_mark_snap_to_cursor(Application_Links *app, View_ID view_id){
    Managed_Scope scope = 0;
    view_get_managed_scope(app, view_id, &scope);
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
    b32 result = false;
    if (fcoder_mode == FCoderMode_NotepadLike){
        i32 pos = 0;
        i32 mark = 0;
        view_get_cursor_pos(app, view_id, &pos);
        view_get_mark_pos(app, view_id, &mark);
        result = (pos != mark);
    }
    return(result);
}

static b32
if_view_has_highlighted_range_delete_range(Application_Links *app, View_ID view_id){
    b32 result = false;
    if (view_has_highlighted_range(app, view_id)){
        Range range = get_view_range(app, view_id);
        Buffer_ID buffer = 0;
        view_get_buffer(app, view_id, AccessOpen, &buffer);
        buffer_replace_range(app, buffer, range, string_u8_litexpr(""));
        result = true;
    }
    return(result);
}

static void
begin_notepad_mode(Application_Links *app){
    fcoder_mode = FCoderMode_NotepadLike;
    View_ID view = 0;
    for (get_view_next(app, 0, AccessAll, &view);
         view != 0;
         get_view_next(app, view, AccessAll, &view)){
        i32 pos = 0;
        view_get_cursor_pos(app, view, &pos);
        view_set_mark(app, view, seek_pos(pos));
    }
}

////////////////////////////////

typedef i32 View_Split_Kind;
enum{
    ViewSplitKind_Ratio,
    ViewSplitKind_FixedPixels,
};

static b32
view_set_split(Application_Links *app, View_ID view, View_Split_Kind kind, f32 t){
    b32 result = false;
    if (view != 0){
        Panel_ID panel_id = 0;
        if (view_get_panel(app, view, &panel_id)){
            Panel_ID parent_panel_id = 0;
            if (panel_get_parent(app, panel_id, &parent_panel_id)){
                Panel_ID min_child_id = 0;
                if (panel_get_child(app, parent_panel_id, PanelChild_Min, &min_child_id)){
                    b32 panel_is_min = (min_child_id == panel_id);
                    Panel_Split_Kind panel_kind = ((kind == ViewSplitKind_Ratio)?
                                                   (panel_is_min?PanelSplitKind_Ratio_Min:PanelSplitKind_Ratio_Max):
                                                   (panel_is_min?PanelSplitKind_FixedPixels_Min:PanelSplitKind_FixedPixels_Max));
                    result = panel_set_split(app, parent_panel_id, panel_kind, t);
                }
            }
        }
    }
    return(result);
}

static b32
view_set_split_proportion(Application_Links *app, View_ID view, f32 t){
    return(view_set_split(app, view, ViewSplitKind_Ratio, t));
}

static b32
view_set_split_pixel_size(Application_Links *app, View_ID view, i32 t){
    return(view_set_split(app, view, ViewSplitKind_FixedPixels, (f32)t));
}

////////////////////////////////

static Record_Info
get_single_record(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Record_Info record = {};
    buffer_history_get_record_info(app, buffer_id, index, &record);
    if (record.error == RecordError_NoError && record.kind == RecordKind_Group){
        buffer_history_get_group_sub_record(app, buffer_id, index, record.group.count - 1, &record);
    }
    return(record);
}

////////////////////////////////

static Vec2
draw_string(Application_Links *app, Face_ID font_id, String_Const_u8 string, Vec2 p, int_color color){
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

static f32
get_dpi_scaling_value(Application_Links *app){
    // TODO(casey): Allen, this should return the multiplier for the display relative to whatever 4coder
    // gets tuned to.
    f32 result = 2.0f;
    return(result);
}

////////////////////////////////

// TODO(allen): REWRITE THIS EXACTLY HOW YOU WANT IT --- start ---

static Child_Process_Set_Target_Flags
flags_system_command(Command_Line_Interface_Flag flags){
    Child_Process_Set_Target_Flags set_buffer_flags = 0;
    if (!HasFlag(flags, CLI_OverlapWithConflict)){
        set_buffer_flags |= ChildProcessSet_FailIfBufferAlreadyAttachedToAProcess;
    }
    if (HasFlag(flags, CLI_CursorAtEnd)){
        set_buffer_flags |= ChildProcessSet_CursorAtEnd;
    }
    return(set_buffer_flags);
}

static b32
set_buffer_system_command(Application_Links *app, Child_Process_ID process, Buffer_ID buffer, Command_Line_Interface_Flag flags){
    b32 result = false;
    Child_Process_Set_Target_Flags set_buffer_flags = flags_system_command(flags);
    if (child_process_set_target_buffer(app, process, buffer, set_buffer_flags)){
        i32 buffer_size = 0;
        buffer_get_size(app, buffer, &buffer_size);
        buffer_replace_range(app, buffer, make_range(0, buffer_size), string_u8_litexpr(""));
        if (HasFlag(flags, CLI_SendEndSignal)){
            buffer_send_end_signal(app, buffer);
        }
        result = true;
    }
    return(result);
}

static b32
exec_system_command(Application_Links *app, View_ID view, Buffer_Identifier buffer_id,
                    String_Const_u8 path, String_Const_u8 command, Command_Line_Interface_Flag flags){
    b32 result = false;
    Child_Process_ID child_process_id = 0;
    if (create_child_process(app, path, command, &child_process_id)){
        result = true;
        Buffer_ID buffer_attach_id = buffer_identifier_to_id_create_out_buffer(app, buffer_id);
        if (buffer_attach_id != 0){
            if (set_buffer_system_command(app, child_process_id, buffer_attach_id, flags)){
                if (view != 0){
                    view_set_buffer(app, view, buffer_attach_id, 0);
                }
            }
        }
    }
    return(result);
}

// TODO(allen): --- end ---

// BOTTOM

