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
    if (view != 0 && buffer != 0){
        move_past_lead_whitespace(app, view, buffer->buffer_id);
    }
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
    if (buffer != 0){
        view_buffer_boundary_seek_set_pos(app, view, buffer->buffer_id, dir, flags);
    }
}

static Range
view_buffer_boundary_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 dir, u32 flags){
    Range result = {};
    if (buffer != 0){
        result = view_buffer_boundary_range(app, view, buffer->buffer_id, dir, flags);
    }
    return(result);
}

static Range
view_buffer_snipe_range(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 dir, u32 flags){
    Range result = {};
    if (buffer != 0){
        result = view_buffer_snipe_range(app, view, buffer->buffer_id, dir, flags);
    }
    return(result);
}

static void
query_replace_base(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, i32 pos, String r, String w){
    if (buffer != 0){
        query_replace_base(app, view, buffer->buffer_id, pos, r, w);
    }
}

#endif

// BOTTOM

