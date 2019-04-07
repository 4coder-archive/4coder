/*
 * Helpers for the API transition from 4.0.30 to 4.0.31
 *
 * In order to keep your layer on the old API you don't have to do anything, this provides wrappers
 *  idential to the 4.0.30 API.
 * In order to transition your entire layer over to the 4.0.31 API define 'REMOVE_TRANSITION_HELPER_31' and fix errors.
 * Or you can do it step by step by removing a few wrappers at a time.
 * This transition helper will be removed in a future version so it is recommended to get off sooner or laster.
 *
 * Tips on transitioning:
*
* Wrather than just try to inline this code everywhere, you can simplify things quite a lot by storing references
* to buffers and views and Buffer_ID and View_ID instead of Buffer_Summary and View_Summary.
 * Just get the summaries when you need information in those structures.
 *
 * You will make your code simpler if you stick to String as much as possible, but whenever you want to you can switch
 * to any string type you have to String by calling make_string(char_ptr, length) or make_string_slowly(null_terminated_c_str).
 * To pull the char ptr and length out of a String named "string": string.str and str.size.
 * If you need a null terminated string from a String use get_null_terminated in 4coder_helper.cpp
 *
 */

// TOP

#if !defined(REMOVE_TRANSITION_HELPER_31)

static char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_get_char(app, buffer->buffer_id, pos));
}

static i32
buffer_get_line_start(Application_Links *app, Buffer_Summary *buffer, i32 line){
    return(buffer==0?0:buffer_get_line_start(app, buffer->buffer_id, line));
}

static i32
buffer_get_line_end(Application_Links *app, Buffer_Summary *buffer, i32 line){
    return(buffer==0?0:buffer_get_line_end(app, buffer->buffer_id, line));
}

static Cpp_Token*
get_first_token_at_line(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 line){
    return(buffer==0?0:get_first_token_at_line(app, buffer->buffer_id, tokens, line));
}

static b32
read_line(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 line, String *str,
          Partial_Cursor *start_out, Partial_Cursor *one_past_last_out){
    return(buffer==0?0:read_line(app, part, buffer->buffer_id, line, str, start_out, one_past_last_out));
}

static b32
read_line(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 line, String *str){
    return(buffer==0?0:read_line(app, part, buffer->buffer_id, line, str));
}


static b32
init_stream_chunk(Stream_Chunk *chunk, Application_Links *app, Buffer_Summary *buffer, i32 pos, char *data, u32 size){
    return(buffer==0?0:init_stream_chunk(chunk, app, buffer->buffer_id, pos, data, size));
}

static b32
init_stream_tokens(Stream_Tokens_DEP *stream, Application_Links *app, Buffer_Summary *buffer, i32 pos, Cpp_Token *data, i32 count){
    return(buffer==0?0:init_stream_tokens(stream, app, buffer->buffer_id, pos, data, count));
}

static String
token_get_lexeme(Application_Links *app, Buffer_Summary *buffer, Cpp_Token *token, char *out_buffer, i32 out_buffer_size){
    String result = {};
    if (buffer != 0){
        result = token_get_lexeme(app, buffer->buffer_id, token, out_buffer, out_buffer_size);
    }
    return(result);
}

static String
token_get_lexeme(Application_Links *app, Partition *part, Buffer_Summary *buffer, Cpp_Token *token){
    String result = {};
    if (buffer != 0){
        result = token_get_lexeme(app, part, buffer->buffer_id, token);
    }
    return(result);
}

static String
get_token_or_word_under_pos(Application_Links *app, Buffer_Summary *buffer, i32 pos, char *space, i32 capacity){
    String result = {};
    if (buffer != 0){
        result = get_token_or_word_under_pos(app, buffer->buffer_id, pos, space, capacity);
    }
    return(result);
}

static i32
seek_line_end(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:seek_line_end(app, buffer->buffer_id, pos));
}

static i32
seek_line_beginning(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:seek_line_beginning(app, buffer->buffer_id, pos));
}

static void
move_past_lead_whitespace(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    move_past_lead_whitespace(app, view!=0?0:view->view_id, buffer!=0?0:buffer->buffer_id);
}

static i32
buffer_seek_whitespace_up(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_whitespace_up(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_whitespace_down(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_whitespace_down(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_whitespace_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_whitespace_right(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_whitespace_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_whitespace_left(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_alphanumeric_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_alphanumeric_right(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_alphanumeric_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_alphanumeric_left(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_alphanumeric_or_underscore_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_alphanumeric_or_underscore_right(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_alphanumeric_or_underscore_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_alphanumeric_or_underscore_left(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_range_camel_right(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 an_pos){
    return(buffer==0?0:buffer_seek_range_camel_right(app, buffer->buffer_id, pos, an_pos));
}

static i32
buffer_seek_range_camel_left(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 an_pos){
    return(buffer==0?0:buffer_seek_range_camel_left(app, buffer->buffer_id, pos, an_pos));
}

static i32
buffer_seek_alphanumeric_or_camel_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_alphanumeric_or_camel_right(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_alphanumeric_or_camel_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_seek_alphanumeric_or_camel_left(app, buffer->buffer_id, pos));
}

static i32
buffer_seek_alpha_numeric_end(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    return(buffer!=0?0:buffer_seek_alpha_numeric_end(app, buffer->buffer_id, pos));
}

static Cpp_Token_Array
buffer_get_all_tokens(Application_Links *app, Partition *part, Buffer_Summary *buffer){
    Cpp_Token_Array result = {};
    if (buffer != 0){
        result = buffer_get_all_tokens(app, part, buffer->buffer_id);
    }
    return(result);
}

static i32
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer, Partition *part, i32 start_pos, b32 seek_forward, Seek_Boundary_Flag flags){
    return(buffer==0?0:buffer_boundary_seek(app, buffer->buffer_id, part, start_pos, seek_forward, flags));
}

static void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_Summary *buffer, i32 pos, char delim, i32 *result){
    if (buffer != 0){
        buffer_seek_delimiter_forward(app, buffer->buffer_id, pos, delim, result);
    }
}

static void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_Summary *buffer, i32 pos, char delim, i32 *result){
    if (buffer != 0){
        buffer_seek_delimiter_backward(app, buffer->buffer_id, pos, delim, result);
    }
}

static void
buffer_seek_string_forward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        buffer_seek_string_forward(app, buffer->buffer_id, pos, end, str, size, result);
    }
}

static void
buffer_seek_string_backward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        buffer_seek_string_backward(app, buffer->buffer_id, pos, end, str, size, result);
    }
}

static void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        buffer_seek_string_insensitive_forward(app, buffer->buffer_id, pos, end, str, size, result);
    }
}

static void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        buffer_seek_string_insensitive_backward(app, buffer->buffer_id, pos, end, str, size, result);
    }
}

static void
buffer_seek_string(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, i32 min, char *str, i32 size, i32 *result, Buffer_Seek_String_Flags flags){
    if (buffer != 0){
        buffer_seek_string(app, buffer->buffer_id, pos, end, min, str, size, result, flags);
    }
}

static b32
buffer_line_is_blank(Application_Links *app, Buffer_Summary *buffer, i32 line){
    return(buffer==0?0:buffer_line_is_blank(app, buffer->buffer_id, line));
}

static String
read_identifier_at_pos(Application_Links *app, Buffer_Summary *buffer, i32 pos, char *space, i32 max, Range *range_out){
    String result = {};
    if (buffer != 0){
        result = read_identifier_at_pos(app, buffer->buffer_id, pos, space, max, range_out);
    }
    return(result);
}

static i32
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, i32 dir, Seek_Boundary_Flag flags){
    return(buffer==0?0:buffer_boundary_seek(app, buffer->buffer_id, start_pos, dir, flags));
}

static void
view_buffer_boundary_seek_set_pos(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 dir, u32 flags){
    view_buffer_boundary_seek_set_pos(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, dir, flags);
}

static void
view_boundary_seek_set_pos(Application_Links *app, View_Summary *view, i32 dir, u32 flags){
    view_boundary_seek_set_pos(app, view==0?0:view->view_id, dir, flags);
}

static Range
view_buffer_boundary_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 dir, u32 flags){
    return(view_buffer_boundary_range(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, dir, flags));
}

static Range
view_buffer_snipe_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 dir, u32 flags){
    return(view_buffer_snipe_range(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, dir, flags));
}

static void
query_replace_base(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 pos, String r, String w){
    query_replace_base(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, pos, r, w);
}

static Statement_Parser
make_statement_parser(Application_Links *app, Buffer_Summary *buffer, i32 token_index){
    Statement_Parser parser = {};
    if (buffer != 0){
        parser = make_statement_parser(app, buffer->buffer_id, token_index);
    }
    return(parser);
}

static b32
find_whole_statement_down(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 *start_out, i32 *end_out){
    return(buffer==0?false:find_whole_statement_down(app, buffer->buffer_id, pos, start_out, end_out));
}

static b32
find_scope_top(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    return(buffer==0?false:find_scope_top(app, buffer->buffer_id, start_pos, flags, end_pos_out));
}

static b32
find_scope_bottom(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    return(buffer==0?false:find_scope_bottom(app, buffer->buffer_id, start_pos, flags, end_pos_out));
}

static b32
find_scope_range(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, Range *range_out, u32 flags){
    return(buffer==0?false:find_scope_range(app, buffer->buffer_id, start_pos, range_out, flags));
}

static b32
find_next_scope(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    return(buffer==0?false:find_next_scope(app, buffer->buffer_id, start_pos, flags, end_pos_out));
}

static b32
find_prev_scope(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    return(buffer==0?false:find_prev_scope(app, buffer->buffer_id, start_pos, flags, end_pos_out));
}

static Range_Array
get_enclosure_ranges(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 pos, u32 flags){
    Range_Array result = {};
    if (buffer != 0){
        result = get_enclosure_ranges(app, part, buffer->buffer_id, pos, flags);
    }
    return(result);
}

static void
mark_enclosures(Application_Links *app, Partition *scratch, Managed_Scope render_scope, Buffer_Summary *buffer, i32 pos, u32 flags, Marker_Visual_Type type, int_color *back_colors, int_color *fore_colors, i32 color_count){
    if (buffer != 0){
        mark_enclosures(app, scratch, render_scope, buffer->buffer_id, pos, flags, type, back_colors, fore_colors, color_count);
    }
}

static Hard_Start_Result
buffer_find_hard_start(Application_Links *app, Buffer_Summary *buffer, i32 line_start, i32 tab_width){
    Hard_Start_Result result = {};
    if (buffer != 0){
        buffer_find_hard_start(app, buffer->buffer_id, line_start, tab_width);
    }
    return(result);
}

static Buffer_Batch_Edit
make_batch_from_indent_marks(Application_Links *app, Partition *arena, Buffer_Summary *buffer, i32 first_line, i32 one_past_last_line, i32 *indent_marks, Indent_Options opts){
    Buffer_Batch_Edit result = {};
    if (buffer != 0){
        make_batch_from_indent_marks(app, arena, buffer->buffer_id, first_line, one_past_last_line, indent_marks, opts);
    }
    return(result);
}

static void
set_line_indents(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 first_line, i32 one_past_last_line, i32 *indent_marks, Indent_Options opts){
    if (buffer != 0){
        set_line_indents(app, part, buffer->buffer_id, first_line, one_past_last_line, indent_marks, opts);
    }
}

static Indent_Anchor_Position
find_anchor_token(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 line_start, i32 tab_width){
    Indent_Anchor_Position result = {};
    if (buffer != 0){
        result = find_anchor_token(app, buffer->buffer_id, tokens, line_start, tab_width);
    }
    return(result);
}

static i32*
get_indentation_marks(Application_Links *app, Partition *arena, Buffer_Summary *buffer,
                      Cpp_Token_Array tokens, i32 first_line, i32 one_past_last_line,
                      b32 exact_align, i32 tab_width){
    return(buffer==0?0:get_indentation_marks(app, arena, buffer->buffer_id, tokens, first_line, one_past_last_line, exact_align, tab_width));
}

static i32
buffer_get_line_number(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_get_line_number(app, buffer->buffer_id, pos));
}

static void
get_indent_lines_minimum(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, i32 end_pos, i32 *line_start_out, i32 *line_end_out){
    if (buffer != 0){
        get_indent_lines_minimum(app, buffer->buffer_id, start_pos, end_pos, line_start_out, line_end_out);
    }
}

static void
get_indent_lines_whole_tokens(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 start_pos, i32 end_pos, i32 *line_start_out, i32 *line_end_out){
    if (buffer != 0){
        get_indent_lines_whole_tokens(app, buffer->buffer_id, tokens, start_pos, end_pos, line_start_out, line_end_out);
    }
}

static b32
buffer_auto_indent(Application_Links *app, Partition *part, Buffer_Summary *buffer, i32 start, i32 end, i32 tab_width, Auto_Indent_Flag flags){
    return(buffer==0?0:buffer_auto_indent(app, part, buffer->buffer_id, start, end, tab_width, flags));
}

static b32
buffer_auto_indent(Application_Links *app, Buffer_Summary *buffer, i32 start, i32 end, i32 tab_width, Auto_Indent_Flag flags){
    return(buffer==0?0:buffer_auto_indent(app, buffer->buffer_id, start, end, tab_width, flags));
}

static void
print_positions_buffered(Application_Links *app, Buffer_Summary *buffer, Function_Positions *positions_array, i32 positions_count, Buffered_Write_Stream *stream){
    if (buffer != 0){
        print_positions_buffered(app, buffer->buffer_id, positions_array, positions_count, stream);
    }
}

static Get_Positions_Results
get_function_positions(Application_Links *app, Buffer_Summary *buffer, i32 first_token_index, Function_Positions *positions_array, i32 positions_max){
    Get_Positions_Results result = {};
    if (buffer != 0){
        result = get_function_positions(app, buffer->buffer_id, first_token_index, positions_array, positions_max);
    }
    return(result);
}

static void
list_all_functions(Application_Links *app, Partition *part, Buffer_Summary *optional_target_buffer){
    if (optional_target_buffer != 0){
        list_all_functions(app, part, optional_target_buffer->buffer_id);
    }
}

static i32
get_start_of_line_at_cursor(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    return(get_start_of_line_at_cursor(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id));
}

static b32
c_line_comment_starts_at_position(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:c_line_comment_starts_at_position(app, buffer->buffer_id, pos));
}

static void
write_string(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, String string){
    write_string(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, string);
}

static b32
open_file(Application_Links *app, Buffer_Summary *buffer_out, char *filename, i32 filename_len, b32 background, b32 never_new){
    b32 result = false;
    Buffer_ID id_out = 0;
    result = open_file(app, &id_out, filename, filename_len, background, never_new);
    if (result && buffer_out != 0){
        get_buffer_summary(app, id_out, AccessAll, buffer_out);
    }
    return(result);
}

static b32
get_cpp_matching_file(Application_Links *app, Buffer_Summary buffer, Buffer_Summary *buffer_out){
    b32 result = false;
    if (buffer.exists){
        Buffer_ID id_out = 0;
        result = get_cpp_matching_file(app, buffer.buffer_id, &id_out);
        if (result && buffer_out != 0){
            get_buffer_summary(app, id_out, AccessAll, buffer_out);
        }
    }
    return(result);
}

static b32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, Name_Line_Column_Location *location){
    Buffer_ID id = 0;
    b32 result = get_jump_buffer(app, &id, location);
    if (result){
        get_buffer_summary(app, id, AccessAll, buffer);
    }
    return(result);
}

static b32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, ID_Pos_Jump_Location *location, Access_Flag access){
    Buffer_ID id = 0;
    b32 result = get_jump_buffer(app, &id, location, access);
    if (result){
        get_buffer_summary(app, id, AccessAll, buffer);
    }
    return(result);
}

static b32
get_jump_buffer(Application_Links *app, Buffer_Summary *buffer, ID_Pos_Jump_Location *location){
    Buffer_ID id = 0;
    b32 result = get_jump_buffer(app, &id, location);
    if (result){
        get_buffer_summary(app, id, AccessAll, buffer);
    }
    return(result);
}

static void
switch_to_existing_view(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    View_ID result = switch_to_existing_view(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id);
    if (view != 0 && result != 0){
        get_view_summary(app, result, AccessAll, view);
    }
}

static void
set_view_to_location(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, Buffer_Seek seek){
    set_view_to_location(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, seek);
}

static void
jump_to_location(Application_Links *app, View_Summary *view, Buffer_ID buffer, Name_Line_Column_Location location){
    jump_to_location(app, view==0?0:view->view_id, buffer, location);
}

static void
jump_to_location(Application_Links *app, View_Summary *view, Buffer_ID buffer, ID_Pos_Jump_Location location){
    jump_to_location(app, view==0?0:view->view_id, buffer, location);
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

static void
refresh_buffer(Application_Links *app, Buffer_Summary *buffer){
    get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
}

static Sticky_Jump_Array
parse_buffer_to_jump_array(Application_Links *app, Partition *arena, Buffer_Summary buffer){
    return(parse_buffer_to_jump_array(app, arena, buffer.buffer_id));
}

static void
lock_jump_buffer(Buffer_Summary buffer){
    lock_jump_buffer(buffer.buffer_name, buffer.buffer_name_len);
}

static Face_Description
get_buffer_face_description(Application_Links *app, Buffer_Summary *buffer){
    Face_Description result = {};
    if (buffer != 0){
        result = get_buffer_face_description(app, buffer->buffer_id);
    }
    return(result);
}

static void
set_buffer_face_by_name(Application_Links *app, Buffer_Summary *buffer, char *name, i32 len){
    if (buffer != 0){
        set_buffer_face_by_name(app, buffer->buffer_id, name, len);
    }
}

static i32
get_build_directory(Application_Links *app, Buffer_Summary *buffer, String *dir_out){
    return(get_build_directory(app, buffer==0?0:buffer->buffer_id, dir_out));
}

static i32
standard_build_search(Application_Links *app, View_Summary *view, String *dir, String *command, b32 perform_backup, b32 use_path_in_command, String filename, String command_name){
    return(standard_build_search(app, view==0?0:view->view_id, dir, command, perform_backup, use_path_in_command, filename, command_name));
}

static i32
execute_standard_build_search(Application_Links *app, View_Summary *view, String *dir, String *command, i32 perform_backup){
    return(execute_standard_build_search(app, view==0?0:view->view_id, dir, command, perform_backup));
}

static void
execute_standard_build(Application_Links *app, View_Summary *view, Buffer_ID active_buffer){
    execute_standard_build(app, view==0?0:view->view_id, active_buffer);
}

static b32
post_buffer_range_to_clipboard(Application_Links *app, Partition *scratch, i32 clipboard_index, Buffer_Summary *buffer, i32 first, i32 one_past_last){
    return(post_buffer_range_to_clipboard(app, scratch, clipboard_index, buffer==0?0:buffer->buffer_id, first, one_past_last));
}

static void
view_set_vertical_focus(Application_Links *app, View_Summary *view, i32 y_top, i32 y_bot){
    view_set_vertical_focus(app, view==0?0:view->view_id, y_top, y_bot);
}

static b32
advance_cursor_in_jump_view(Application_Links *app, Partition *part, View_Summary *view, i32 skip_repeats, i32 skip_sub_error, i32 direction, Name_Line_Column_Location *location_out){
    return(advance_cursor_in_jump_view(app, part, view==0?0:view->view_id, skip_repeats, skip_sub_error, direction, location_out));
}

static Parsed_Jump
seek_next_jump_in_view(Application_Links *app, Partition *part, View_Summary *view, i32 skip_sub_errors, i32 direction, i32 *line_out){
    return(seek_next_jump_in_view(app, part, view==0?0:view->view_id, skip_sub_errors, direction, line_out));
}

static void
goto_next_filtered_jump(Application_Links *app, Marker_List *list, View_Summary *jump_view, i32 list_index, i32 direction, b32 skip_repeats, b32 skip_sub_errors){
    goto_next_filtered_jump(app, list, jump_view==0?0:jump_view->view_id, list_index, direction, skip_repeats, skip_sub_errors);
}

static void
goto_jump_in_order(Application_Links *app, Marker_List *list, View_Summary *jump_view, ID_Pos_Jump_Location location){
    goto_jump_in_order(app, list, jump_view==0?0:jump_view->view_id, location);
}

static void
open_jump_lister(Application_Links *app, Partition *scratch, Heap *heap, View_Summary *ui_view, Buffer_ID list_buffer_id, Jump_Lister_Activation_Rule activation_rule, View_Summary *optional_target_view){
    open_jump_lister(app, scratch, heap, ui_view==0?0:ui_view->view_id, list_buffer_id, activation_rule, optional_target_view==0?0:optional_target_view->view_id);
}

static void
activate_project_command(Application_Links *app, Partition *scratch, Heap *heap, View_Summary *view, Lister_State *state, String text_field, void *user_data, b32 activated_by_mouse){
    activate_project_command(app, scratch, heap, view==0?0:view->view_id, state, text_field, user_data, activated_by_mouse);
}

static void
activate_snippet(Application_Links *app, Partition *scratch, Heap *heap, View_Summary *view, struct Lister_State *state, String text_field, void *user_data, b32 activated_by_mouse){
    activate_snippet(app, scratch, heap, view==0?0:view->view_id, state, text_field, user_data, activated_by_mouse);
}

static void
view_set_to_region(Application_Links *app, View_Summary *view, i32 major_pos, i32 minor_pos, f32 normalized_threshold){
    view_set_to_region(app, view==0?0:view->view_id, major_pos, minor_pos, normalized_threshold);
}

static i32
character_pos_to_pos(Application_Links *app, View_Summary *view, i32 character_pos){
    return(character_pos_to_pos(app, view==0?0:view->view_id, character_pos));
}

static b32
view_open_file(Application_Links *app, View_Summary *view, char *filename, i32 filename_len, b32 never_new){
    return(view_open_file(app, view==0?0:view->view_id, filename, filename_len, never_new));
}

static f32
get_page_jump(Application_Links *app, View_Summary *view){
    return(get_page_jump(app, view==0?0:view->view_id));
}

static void
isearch__update_highlight(Application_Links *app, View_Summary *view, Managed_Object highlight, i32 start, i32 end){
    isearch__update_highlight(app, view==0?0:view->view_id, highlight, start, end);
}

static void
get_view_prev(Application_Links *app, View_Summary *view, Access_Flag access){
    View_ID new_id = 0;
    get_view_prev(app, view->view_id, access, &new_id);
    get_view_summary(app, new_id, access, view);
}

static void
get_next_view_looped_all_panels(Application_Links *app, View_Summary *view, Access_Flag access){
    View_ID new_id = get_next_view_looped_all_panels(app, view==0?0:view->view_id, access);
    get_view_summary(app, new_id, access, view);
}

static void
get_prev_view_looped_all_panels(Application_Links *app, View_Summary *view, Access_Flag access){
    View_ID new_id = get_prev_view_looped_all_panels(app, view->view_id, access);
    get_view_summary(app, new_id, access, view);
}

static void
refresh_view(Application_Links *app, View_Summary *view){
    *view = get_view(app, view->view_id, AccessAll);
}

static String
get_string_in_view_range(Application_Links *app, Partition *arena, View_Summary *view){
    return(get_string_in_view_range(app, arena, view==0?0:view->view_id));
}

static b32
view_set_split(Application_Links *app, View_Summary *view, View_Split_Kind kind, f32 t){
    b32 result = view_set_split(app, view==0?0:view->view_id, kind, t);
    if (result && view != 0){
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_split_proportion(Application_Links *app, View_Summary *view, f32 t){
    b32 result = view_set_split_proportion(app, view==0?0:view->view_id, t);
    if (result && view != 0){
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_split_pixel_size(Application_Links *app, View_Summary *view, i32 t){
    b32 result = view_set_split_pixel_size(app, view==0?0:view->view_id, t);
    if (result && view != 0){
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static View_Summary
open_footer_panel(Application_Links *app, View_Summary *view){
    View_ID new_id = open_footer_panel(app, view==0?0:view->view_id);
    View_Summary result = {};
    get_view_summary(app, new_id, AccessAll, &result);
    return(result);
}

static void
new_view_settings(Application_Links *app, View_Summary *view){
    new_view_settings(app, view==0?0:view->view_id);
}

static void
view_set_passive(Application_Links *app, View_Summary *view, b32 value){
    view_set_passive(app, view==0?0:view->view_id, value);
}

static b32
view_get_is_passive(Application_Links *app, View_Summary *view){
    return(view != 0 && view_get_is_passive(app, view->view_id));
}

static void
get_next_view_looped_primary_panels(Application_Links *app, View_Summary *view_start, Access_Flag access){
    View_ID new_id = get_next_view_looped_primary_panels(app, view_start->view_id, access);
    get_view_summary(app, new_id, AccessAll, view_start);
}

static void
get_prev_view_looped_primary_panels(Application_Links *app, View_Summary *view_start, Access_Flag access){
    View_ID new_id = get_prev_view_looped_primary_panels(app, view_start->view_id, access);
    get_view_summary(app, new_id, AccessAll, view_start);
}

static void
list__parameters(Application_Links *app, Heap *heap, Partition *scratch, String *strings, i32 count, Search_Range_Flag match_flags, View_Summary default_target_view){
    list__parameters(app, heap, scratch, strings, count, match_flags, default_target_view.view_id);
}

static void
list_query__parameters(Application_Links *app, Heap *heap, Partition *scratch, b32 substrings, b32 case_insensitive, View_Summary default_target_view){
    list_query__parameters(app, heap, scratch, substrings, case_insensitive, default_target_view.view_id);
}

static Buffer_ID
create_or_switch_to_buffer_by_name(Application_Links *app, char *name, i32 name_length, View_Summary default_target_view){
    return(create_or_switch_to_buffer_by_name(app, make_string(name, name_length), default_target_view.view_id));
}

static void
list_identifier__parameters(Application_Links *app, Heap *heap, Partition *scratch, b32 substrings, b32 case_insensitive, View_Summary default_target_view){
    list_identifier__parameters(app, heap, scratch, substrings, case_insensitive, default_target_view.view_id);
}

static void
list_type_definition__parameters(Application_Links *app, Heap *heap, Partition *scratch, String str, View_Summary default_target_view){
    list_type_definition__parameters(app, heap, scratch, str, default_target_view.view_id);
}

#endif

// BOTTOM

