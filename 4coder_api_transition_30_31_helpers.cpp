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

static b32
exec_system_command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer_id,
                    char *path, int32_t path_len, char *command, i32 command_len, Command_Line_Interface_Flag flags){
    return(exec_system_command(app, (view == 0)?0:view->view_id, buffer_id,
                               SCu8(path, path_len), SCu8(command, command_len), flags));
    
#if 0
    b32 result = false;
    
    String_Const_u8 path_string = SCu8((u8*)path, path_len);
    String_Const_u8 command_string = SCu8((u8*)command, command_len);
    Child_Process_ID child_process_id = 0;
    if (create_child_process(app, path_string, command_string, &child_process_id)){
        result = true;
        
        Buffer_ID buffer_attach_id = 0;
        if (buffer_id.name != 0 && buffer_id.name_len > 0){
            String_Const_u8 buffer_name = SCu8((u8*)buffer_id.name, buffer_id.name_len);
            if (!get_buffer_by_name(app, buffer_name, AccessAll, &buffer_attach_id)){
                if (create_buffer(app, buffer_name, BufferCreate_AlwaysNew|BufferCreate_NeverAttachToFile, &buffer_attach_id)){
                    buffer_set_setting(app, buffer_attach_id, BufferSetting_ReadOnly, true);
                    buffer_set_setting(app, buffer_attach_id, BufferSetting_Unimportant, true);
                }
            }
        }
        else if (buffer_id.id != 0){
            buffer_attach_id = buffer_id.id;
        }
        
        if (buffer_attach_id != 0){
            Child_Process_Set_Target_Flags set_buffer_flags = 0;
            if (!HasFlag(flags, CLI_OverlapWithConflict)){
                set_buffer_flags |= ChildProcessSet_FailIfBufferAlreadyAttachedToAProcess;
            }
            if (HasFlag(flags, CLI_CursorAtEnd)){
                set_buffer_flags |= ChildProcessSet_CursorAtEnd;
            }
            
            if (child_process_set_target_buffer(app, child_process_id, buffer_attach_id, set_buffer_flags)){
                Buffer_Summary buffer = {};
                get_buffer_summary(app, buffer_attach_id, AccessAll, &buffer);
                buffer_replace_range(app, buffer_attach_id, make_range(0, buffer.size), string_u8_litexpr(""));
                if (HasFlag(flags, CLI_SendEndSignal)){
                    buffer_send_end_signal(app, buffer_attach_id);
                }
                if (view != 0){
                    view_set_buffer(app, view->view_id, buffer_attach_id, 0);
                    get_view_summary(app, view->view_id, AccessAll, view);
                }
            }
        }
    }
    
    return(result);
#endif
}

static b32
exec_system_command(Application_Links *app, View_ID view, Buffer_Identifier buffer_id,
                    char *path, int32_t path_len, char *command, i32 command_len, Command_Line_Interface_Flag flags){
    View_Summary view_summary = {};
    get_view_summary(app, view, AccessAll, &view_summary);
    return(exec_system_command(app, &view_summary, buffer_id, path, path_len, command, command_len, flags));
}

static char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:buffer_get_char(app, buffer->buffer_id, pos));
}

static i32
buffer_get_line_start(Application_Links *app, Buffer_Summary *buffer, i32 line){
    return(buffer==0?0:(i32)get_line_start_pos(app, buffer->buffer_id, line));
}

static i32
buffer_get_line_end(Application_Links *app, Buffer_Summary *buffer, i32 line){
    return(buffer==0?0:(i32)get_line_end_pos(app, buffer->buffer_id, line));
}

static Cpp_Token*
get_first_token_at_line(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 line){
    return(buffer==0?0:get_first_token_from_line(app, buffer->buffer_id, tokens, line));
}

static b32
read_line(Application_Links *app, Arena *arena, Buffer_Summary *buffer, i32 line, String *str,
          Partial_Cursor *start_out, Partial_Cursor *one_past_last_out){
    String_Const_u8 string = {};
    Range_Partial_Cursor out = {};
    b32 result = (buffer != 0 && is_valid_line(app, buffer->buffer_id, line));
    if (result){
        out = get_line_range(app, buffer->buffer_id, line);
        string = push_buffer_range(app, arena, buffer->buffer_id, make_range_from_cursors(out));
    }
    *start_out = out.start;
    *one_past_last_out = out.one_past_last;
    *str = string_old_from_new(string);
    return(result);
}

static b32
read_line(Application_Links *app, Arena *arena, Buffer_Summary *buffer, i32 line, String *str){
    String_Const_u8 string = {};
    b32 result = (buffer != 0 && is_valid_line(app, buffer->buffer_id, line));
    if (result){
        Range_Partial_Cursor range = get_line_range(app, buffer->buffer_id, line);
        string = push_buffer_range(app, arena, buffer->buffer_id, make_range_from_cursors(range));
    }
    *str = string_old_from_new(string);
    return(result);
}


static b32
init_stream_chunk(Stream_Chunk *chunk, Application_Links *app, Buffer_Summary *buffer, i32 pos, char *data, u32 size){
    return(buffer==0?0:init_stream_chunk(chunk, app, buffer->buffer_id, pos, data, size));
}

static b32
init_stream_tokens(Stream_Tokens_DEP *stream, Application_Links *app, Buffer_Summary *buffer, i32 pos, Cpp_Token *data, i32 count){
    b32 result = false;
    
    if (buffer != 0){
        i32 token_count = buffer_token_count(app, buffer);
        if (buffer_tokens_are_ready(app, buffer->buffer_id) &&
            0 <= pos && pos < token_count && count > 0){
            stream->app = app;
            stream->buffer_id = buffer->buffer_id;
            stream->base_tokens = data;
            stream->count = count;
            stream->start = round_down_i32(pos, count);
            stream->end = round_up_i32(pos, count);
            stream->token_count = token_count;
            
            stream->start = clamp_bot(0, stream->start);
            stream->end = clamp_top(stream->end, stream->token_count);
            
            buffer_read_tokens(app, buffer, stream->start, stream->end, stream->base_tokens);
            stream->tokens = stream->base_tokens - stream->start;
            result = true;
        }
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

static String
token_get_lexeme(Application_Links *app, Arena *arena, Buffer_Summary *buffer, Cpp_Token *token){
    String result = {};
    if (buffer != 0 && token != 0){
        result = string_old_from_new(push_token_lexeme(app, arena, buffer->buffer_id, *token));
    }
    return(result);
}

static String
get_token_or_word_under_pos(Application_Links *app, Buffer_Summary *buffer, i32 pos, char *space, i32 capacity){
    String result = {};
    if (buffer != 0){
        Scratch_Block scratch(app);
        String_Const_u8 string = push_token_or_word_under_pos(app, scratch, buffer->buffer_id, pos);
        i32 size = (i32)string.size;
        size = clamp_top(size, capacity);
        block_copy(space, string.str, size);
        result = make_string_cap(space, size, capacity);
    }
    return(result);
}

static i32
seek_line_end(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)get_line_end_pos_from_pos(app, buffer->buffer_id, pos));
}

static i32
seek_line_beginning(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)get_line_start_pos_from_pos(app, buffer->buffer_id, pos));
}

static void
move_past_lead_whitespace(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    move_past_lead_whitespace(app, view!=0?0:view->view_id, buffer!=0?0:buffer->buffer_id);
}

static i32
buffer_seek_whitespace_up(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)get_pos_of_blank_line(app, buffer->buffer_id, Scan_Backward, pos));
}

static i32
buffer_seek_whitespace_down(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)get_pos_of_blank_line(app, buffer->buffer_id, Scan_Forward, pos));
}

static i32
buffer_seek_whitespace_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_non_whitespace, buffer->buffer_id, Scan_Forward, pos));
}

static i32
buffer_seek_whitespace_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_non_whitespace, buffer->buffer_id, Scan_Backward, pos));
}

static i32
buffer_seek_alphanumeric_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_alpha_numeric, buffer->buffer_id, Scan_Forward, pos));
}

static i32
buffer_seek_alphanumeric_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_alpha_numeric, buffer->buffer_id, Scan_Backward, pos));
}

static i32
buffer_seek_alphanumeric_or_underscore_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_alpha_numeric_underscore, buffer->buffer_id,
                                 Scan_Forward, pos));
}

static i32
buffer_seek_alphanumeric_or_underscore_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_alpha_numeric_underscore, buffer->buffer_id,
                                 Scan_Backward, pos));
}

static i32
buffer_seek_alphanumeric_or_camel_right(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_alpha_numeric_camel, buffer->buffer_id,
                                 Scan_Forward, pos));
}

static i32
buffer_seek_alphanumeric_or_camel_left(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)scan(app, boundary_alpha_numeric_camel, buffer->buffer_id,
                                 Scan_Backward, pos));
}

static i32
buffer_seek_alpha_numeric_end(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    return(buffer!=0?0:(i32)scan(app, boundary_alpha_numeric_unicode,
                                 buffer->buffer_id,
                                 Scan_Forward, pos));
}

static Cpp_Token_Array
buffer_get_all_tokens(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Cpp_Token_Array array = {};
    if (buffer_exists(app, buffer_id)){
        b32 is_lexed = false;
        if (buffer_get_setting(app, buffer_id, BufferSetting_Lex, &is_lexed)){
            if (is_lexed){
                array.count = buffer_get_token_array(app, buffer_id).count;
                array.max_count = array.count;
                array.tokens = push_array(arena, Cpp_Token, array.count);
                buffer_read_tokens(app, buffer_id, 0, array.count, array.tokens);
            }
        }
    }
    return(array);
}

static Cpp_Token_Array
buffer_get_all_tokens(Application_Links *app, Arena *arena, Buffer_Summary *buffer){
    Cpp_Token_Array result = {};
    if (buffer != 0){
        result = buffer_get_all_tokens(app, arena, buffer->buffer_id);
    }
    return(result);
}

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_ID buffer, i32 pos, char delim, i32 *result){
    Character_Predicate predicate = character_predicate_from_character((u8)delim);
    String_Match m = buffer_seek_character_class(app, buffer, &predicate, Scan_Forward, pos);
    *result = (i32)m.range.min;
}

void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_ID buffer, i32 pos, char delim, i32 *result){
    Character_Predicate predicate = character_predicate_from_character((u8)delim);
    String_Match m = buffer_seek_character_class(app, buffer, &predicate, Scan_Backward, pos);
    *result = (i32)m.range.min;
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
        i64 result_i64 = 0;
        buffer_seek_string_forward(app, buffer->buffer_id, pos, end, SCu8(str, size), &result_i64);
        *result = (i32)result_i64;
    }
}

static void
buffer_seek_string_backward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        i64 result_i64 = 0;
        buffer_seek_string_backward(app, buffer->buffer_id, pos, end, SCu8(str, size), &result_i64);
        *result = (i32)result_i64;
    }
}

static void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        i64 result_i64 = 0;
        buffer_seek_string_insensitive_forward(app, buffer->buffer_id, pos, end, SCu8(str, size), &result_i64);
        *result = (i32)result_i64;
    }
}

static void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, char *str, i32 size, i32 *result){
    if (buffer != 0){
        i64 result_i64 = 0;
        buffer_seek_string_insensitive_backward(app, buffer->buffer_id, pos, end, SCu8(str, size), &result_i64);
        *result = (i32)result_i64;
    }
}

static void
buffer_seek_string(Application_Links *app, Buffer_Summary *buffer, i32 pos, i32 end, i32 min, char *str, i32 size, i32 *result, Buffer_Seek_String_Flags flags){
    if (buffer != 0){
        i64 result_i64 = 0;
        buffer_seek_string(app, buffer->buffer_id, pos, end, min, SCu8(str, size), &result_i64, flags);
        *result = (i32)result_i64;
    }
}

static b32
buffer_line_is_blank(Application_Links *app, Buffer_Summary *buffer, i32 line){
    return(buffer==0?0:line_is_valid_and_blank(app, buffer->buffer_id, line));
}

static String
read_identifier_at_pos(Application_Links *app, Buffer_Summary *buffer, i32 pos, char *space, i32 max, Range *range_out){
    String result = {};
    if (buffer != 0){
        Scratch_Block scratch(app);
        Range_i64 range = enclose_pos_alpha_numeric_underscore(app, buffer->buffer_id, pos);
        String_Const_u8 string = push_buffer_range(app, scratch, buffer->buffer_id, range);
        if (range_out != 0){
            *range_out = Ii32((i32)range.min, (i32)range.max);;
        }
        i32 size = (i32)string.size;
        size = clamp_top(size, max);
        block_copy(space, string.str, size);
        result = make_string_cap(space, size, max);
    }
    return(result);
}

internal Boundary_Function_List
boundary_list_from_old_flags(Arena *arena, Seek_Boundary_Flag flags){
    Boundary_Function_List list = {};
    if (HasFlag(flags, BoundaryWhitespace)){
        push_boundary(arena, &list, boundary_non_whitespace);
    }
    if (HasFlag(flags, BoundaryToken)){
        push_boundary(arena, &list, boundary_token);
    }
    if (HasFlag(flags, BoundaryAlphanumeric)){
        push_boundary(arena, &list, boundary_alpha_numeric);
    }
    if (HasFlag(flags, BoundaryCamelCase)){
        push_boundary(arena, &list, boundary_alpha_numeric_camel);
    }
    return(list);
}

static i32
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, i32 dir, Seek_Boundary_Flag flags){
    i32 result = 0;
    if (buffer != 0){
        Scratch_Block scratch(app);
        result = (i32)scan(app, boundary_list_from_old_flags(scratch, flags), buffer->buffer_id, dir, start_pos);
    }
    return(result);
}

static void
query_replace_base(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 pos, String r, String w){
    query_replace_base(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, pos,
                       string_new_u8_from_old(r),
                       string_new_u8_from_old(w));
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
    Range_i64 r = {};
    b32 result = (buffer==0?false:find_whole_statement_down(app, buffer->buffer_id, pos, &r.min, &r.max));
    *start_out = (i32)r.min;
    *end_out = (i32)r.max;
    return(result);
}

static b32
find_scope_top(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    i64 r = 0;
    b32 result = (buffer==0?false:find_scope_top(app, buffer->buffer_id, start_pos, flags, &r));
    *end_pos_out = (i32)r;
    return(result);
}

static b32
find_scope_bottom(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    i64 r = 0;
    b32 result = (buffer==0?false:find_scope_bottom(app, buffer->buffer_id, start_pos, flags, &r));
    *end_pos_out = (i32)r;
    return(result);
}

static b32
find_scope_range(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, Range *range_out, u32 flags){
    Range_i64 r = {};
    b32 result = (buffer==0?false:find_scope_range(app, buffer->buffer_id, start_pos, &r, flags));
    *range_out = Ii32((i32)r.min, (i32)r.max);
    return(result);
}

static b32
find_next_scope(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    i64 r = 0;
    b32 result = (buffer==0?false:find_next_scope(app, buffer->buffer_id, start_pos, flags, &r));
    *end_pos_out = (i32)r;
    return(result);
}

static b32
find_prev_scope(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    i64 r = 0;
    b32 result = (buffer==0?false:find_prev_scope(app, buffer->buffer_id, start_pos, flags, &r));
    *end_pos_out = (i32)r;
    return(result);
}

static void
mark_enclosures(Application_Links *app, Managed_Scope render_scope, Buffer_Summary *buffer, i32 pos, u32 flags, Marker_Visual_Type type, int_color *back_colors, int_color *fore_colors, i32 color_count){
    if (buffer != 0){
        mark_enclosures(app, render_scope, buffer->buffer_id, pos, flags, type, back_colors, fore_colors, color_count);
    }
}

struct Hard_Start_Result{
    i32 char_pos;
    i32 indent_pos;
    i32 all_whitespace;
    i32 all_space;
};

static Hard_Start_Result
buffer_find_hard_start(Application_Links *app, Buffer_Summary *buffer, i32 line_start, i32 tab_width){
    Hard_Start_Result result = {};
    if (buffer != 0){
        Indent_Info info = get_indent_info_line_start(app, buffer->buffer_id, line_start, tab_width);
        result.char_pos = (i32)info.first_char_pos;
        result.indent_pos = info.indent_pos;
        result.all_whitespace = info.is_blank;
        result.all_space = info.all_space;
    }
    return(result);
}

static Indent_Anchor_Position
find_anchor_token(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 line_start, i32 tab_width){
    Indent_Anchor_Position result = {};
    if (buffer != 0){
        result = find_anchor_token(app, buffer->buffer_id, tokens, line_start, tab_width);
    }
    return(result);
}

static i32
buffer_get_line_number(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:(i32)get_line_number_from_pos(app, buffer->buffer_id, pos));
}

static void
get_indent_lines_minimum(Application_Links *app, Buffer_Summary *buffer, i32 start_pos, i32 end_pos, i32 *line_start_out, i32 *line_end_out){
    if (buffer != 0){
        Range_i64 r = {};
        get_indent_lines_minimum(app, buffer->buffer_id, start_pos, end_pos, &r.min, &r.max);
        *line_start_out = (i32)r.min;
        *line_end_out = (i32)r.max;
    }
}

static void
get_indent_lines_whole_tokens(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, i32 start_pos, i32 end_pos, i32 *line_start_out, i32 *line_end_out){
    if (buffer != 0){
        Range_i64 r = {};
        get_indent_lines_whole_tokens(app, buffer->buffer_id, tokens, start_pos, end_pos, &r.min, &r.max);
        *line_start_out = (i32)r.min;
        *line_end_out = (i32)r.max;
    }
}

static b32
buffer_auto_indent(Application_Links *app, Arena *arena, Buffer_Summary *buffer, i32 start, i32 end, i32 tab_width, Auto_Indent_Flag flags){
    return(buffer==0?0:buffer_auto_indent(app, buffer->buffer_id, start, end, tab_width, flags));
}

static b32
buffer_auto_indent(Application_Links *app, Buffer_Summary *buffer, i32 start, i32 end, i32 tab_width, Auto_Indent_Flag flags){
    return(buffer==0?0:buffer_auto_indent(app, buffer->buffer_id, start, end, tab_width, flags));
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
list_all_functions(Application_Links *app, Buffer_Summary *optional_target_buffer){
    if (optional_target_buffer != 0){
        list_all_functions(app, optional_target_buffer->buffer_id);
    }
}

static i32
get_start_of_line_at_cursor(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    return((i32)get_start_of_line_at_cursor(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id));
}

static b32
c_line_comment_starts_at_position(Application_Links *app, Buffer_Summary *buffer, i32 pos){
    return(buffer==0?0:c_line_comment_starts_at_position(app, buffer->buffer_id, pos));
}

static void
write_string(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, String string){
    write_string(app, view==0?0:view->view_id, buffer==0?0:buffer->buffer_id, string_new_u8_from_old(string));
}

static b32
open_file(Application_Links *app, Buffer_Summary *buffer_out, char *filename, i32 filename_len, b32 background, b32 never_new){
    b32 result = false;
    Buffer_ID id_out = 0;
    result = open_file(app, &id_out, SCu8(filename, filename_len), background, never_new);
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
parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_Summary buffer){
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
execute_standard_build(Application_Links *app, View_Summary *view, Buffer_ID active_buffer){
    standard_search_and_build(app, view==0?0:view->view_id, active_buffer);
}

static b32
post_buffer_range_to_clipboard(Application_Links *app, i32 clipboard_index, Buffer_Summary *buffer, i32 first, i32 one_past_last){
    return(clipboard_post_buffer_range(app, clipboard_index, buffer==0?0:buffer->buffer_id, Ii64(first, one_past_last)));
}

static void
view_set_vertical_focus(Application_Links *app, View_Summary *view, i32 y_top, i32 y_bot){
    view_set_vertical_focus(app, view==0?0:view->view_id, (f32)y_top, (f32)y_bot);
}

static b32
advance_cursor_in_jump_view(Application_Links *app, View_Summary *view, i32 skip_repeats, i32 skip_sub_error, i32 direction, Name_Line_Column_Location *location_out){
    return(advance_cursor_in_jump_view(app, view==0?0:view->view_id, skip_repeats, skip_sub_error, direction, location_out));
}

static Parsed_Jump
seek_next_jump_in_view(Application_Links *app, Arena *arena, View_Summary *view, i32 skip_sub_errors, i32 direction, i32 *line_out){
    i64 r = 0;
    Parsed_Jump result = (seek_next_jump_in_view(app, arena, view==0?0:view->view_id, skip_sub_errors, direction, &r));
    *line_out = (i32)r;
    return(result);
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
open_jump_lister(Application_Links *app, Heap *heap, View_Summary *ui_view, Buffer_ID list_buffer_id, Jump_Lister_Activation_Rule activation_rule, View_Summary *optional_target_view){
    open_jump_lister(app, heap, ui_view==0?0:ui_view->view_id, list_buffer_id, activation_rule, optional_target_view==0?0:optional_target_view->view_id);
}

static void
view_set_to_region(Application_Links *app, View_Summary *view, i32 major_pos, i32 minor_pos, f32 normalized_threshold){
    view_set_to_region(app, view==0?0:view->view_id, major_pos, minor_pos, normalized_threshold);
}

static i32
character_pos_to_pos(Application_Links *app, View_Summary *view, i32 character_pos){
    return((i32)character_pos_to_pos_view(app, view==0?0:view->view_id, character_pos));
}

static b32
view_open_file(Application_Links *app, View_Summary *view, char *filename, i32 filename_len, b32 never_new){
    return(view_open_file(app, view==0?0:view->view_id, SCu8(filename, filename_len), never_new));
}

static f32
get_page_jump(Application_Links *app, View_Summary *view){
    return(get_page_jump(app, view==0?0:view->view_id));
}

static void
isearch__update_highlight(Application_Links *app, View_Summary *view, Managed_Object highlight, i32 start, i32 end){
    isearch__update_highlight(app, view==0?0:view->view_id, highlight, Ii64(start, end));
}

static void
get_view_prev(Application_Links *app, View_Summary *view, Access_Flag access){
    View_ID new_id = get_view_prev(app, view->view_id, access);
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
get_string_in_view_range(Application_Links *app, Arena *arena, View_Summary *view){
    return(string_old_from_new(push_view_range_string(app, arena, view==0?0:view->view_id)));
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

static Buffer_ID
create_or_switch_to_buffer_by_name(Application_Links *app, char *name, i32 name_length, View_Summary default_target_view){
    return(create_or_switch_to_buffer_and_clear_by_name(app, SCu8(name, name_length), default_target_view.view_id));
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

static void
change_mapping(Application_Links *app, String mapping){
    change_mapping(app, string_new_u8_from_old(mapping));
}

static void
query_replace_parameter(Application_Links *app, String replace_str, i32 start_pos, b32 add_replace_query_bar){
    query_replace_parameter(app, SCu8(string_new_from_old(replace_str)), start_pos, add_replace_query_bar);
}

static String
hot_directory_push(Application_Links *app, Arena *arena){
    return(string_old_from_new(push_hot_directory(app, arena)));
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

static void
condense_whitespace(String *a){
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

static void
insert_string__no_buffering(Buffer_Insertion *insertion, String string){
    insert_string__no_buffering(insertion, string_new_u8_from_old(string));
}

static void
insert_string(Buffer_Insertion *insertion, String string){
    insert_string(insertion, string_new_u8_from_old(string));
}

static void
save_all_dirty_buffers_with_postfix(Application_Links *app, String postfix){
    save_all_dirty_buffers_with_postfix(app, string_new_u8_from_old(postfix));
}

static void
delete_file_base(Application_Links *app, String file_name, Buffer_ID buffer_id){
    delete_file_base(app, string_new_u8_from_old(file_name), buffer_id);
}

static b32
ms_style_verify(String line, i32 left_paren_pos, i32 right_paren_pos){
    return(ms_style_verify(string_new_u8_from_old(line), left_paren_pos, right_paren_pos));
}

static i32
try_skip_rust_arrow(String line){
    return((i32)(try_skip_rust_arrow(string_new_u8_from_old(line))));
}

static b32
check_is_note(String line, i32 colon_pos){
    return(check_is_note(string_new_u8_from_old(line), colon_pos));
}

static void
close_all_files_with_extension(Application_Links *app, CString_Array extension_array){
    Scratch_Block scratch(app);
    String_Const_u8_Array array = {};
    array.count = extension_array.count;
    array.strings = push_array(scratch, String_Const_u8, array.count);
    for (i32 i = 0; i < array.count; i += 1){
        array.strings[i] = SCu8(extension_array.strings[i]);
    }
    close_all_files_with_extension(app, array);
}

static void
open_all_files_in_directory_pattern_match(Application_Links *app,
                                          String dir,
                                          Project_File_Pattern_Array whitelist,
                                          Project_File_Pattern_Array blacklist,
                                          u32 flags){
    open_all_files_in_directory_pattern_match(app, string_new_u8_from_old(dir), whitelist, blacklist, flags);
}

static String_Const_u8
scratch_read(Application_Links *app, Arena *arena, Buffer_ID buffer, umem start, umem end){
    return(push_buffer_range(app, arena, buffer, Ii64(start, end)));
}

static String_Const_u8
scratch_read(Application_Links *app, Arena *arena, Buffer_ID buffer, Range range){
    return(push_buffer_range(app, arena, buffer, Ii64(range.min, range.max)));
}

static String_Const_u8
scratch_read(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_u64 range){
    return(push_buffer_range(app, arena, buffer, Ii64(range.min, range.max)));
}

static String_Const_u8
scratch_read(Application_Links *app, Arena *arena, Buffer_ID buffer, Cpp_Token token){
    return(push_token_lexeme(app, arena, buffer, token));
}

static String_Const_u8
scratch_read_line(Application_Links *app, Arena *arena, Buffer_ID buffer, i32 line){
    return(push_buffer_line(app, arena, buffer, line));
}

static String_Const_u8
token_get_lexeme(Application_Links *app, Arena *arena, Buffer_ID buffer, Cpp_Token token){
    return(push_token_lexeme(app, arena, buffer, token));
}

static String_Const_u8
get_token_lexeme(Application_Links *app, Arena *arena, Buffer_ID buffer, Cpp_Token token){
    return(push_token_lexeme(app, arena, buffer, token));
}

static String_Const_u8
read_entire_buffer(Application_Links *app, Buffer_ID buffer_id, Arena *scratch){
    return(push_whole_buffer(app, scratch, buffer_id));
}

static String_Const_u8
buffer_read_string(Application_Links *app, Buffer_ID buffer, Range range, void *dest){
    Scratch_Block scratch(app);
    String_Const_u8 result = push_buffer_range(app, scratch, buffer, Ii64(range.min, range.max));
    block_copy(dest, result.str, result.size);
    result.str = (u8*)dest;
    return(result);
}

static b32
token_match(Application_Links *app, Buffer_ID buffer, Cpp_Token token, String b){
    return(token_lexeme_string_match(app, buffer, token, string_new_u8_from_old(b)));
}

static i32
view_get_line_number(Application_Links *app, View_ID view, i32 pos){
    Full_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    return((i32)cursor.line);
}

static void
current_view_boundary_seek_set_pos(Application_Links *app, Scan_Direction direction, u32 flags){
    Scratch_Block scratch(app);
    current_view_scan_move(app, direction, boundary_list_from_old_flags(scratch, flags));
}

static Buffer_Kill_Result
kill_buffer(Application_Links *app, Buffer_Identifier identifier, View_ID gui_view_id, Buffer_Kill_Flag flags){
    return(try_buffer_kill(app, buffer_identifier_to_id(app, identifier), gui_view_id, flags));
}

#endif

// BOTTOM

