/*
4coder_miblo_numbers.cpp - Commands for so called "Miblo Number Operations" which involve incrementing
and decrementing various forms of number as numerical objects despite being encoded as text objects.
*/

// TOP

static Range
get_numeric_string_at_cursor(Application_Links *app, Buffer_ID buffer, i32 start_pos){
    Range result = {};
    
    char current = buffer_get_char(app, buffer, start_pos);
    if (character_is_base10(current)){
        char chunk[1024];
        i32 chunk_size = sizeof(chunk);
        Stream_Chunk stream = {};
        
        i32 pos = start_pos;
        if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
            b32 still_looping = true;
            for (;still_looping;){
                for (; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (!character_is_base10(at_pos)){
                        ++pos;
                        goto double_break_1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }
            double_break_1:;
            i32 pos1 = pos;
            
            if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
                still_looping = true;
                while (still_looping){
                    for (; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (!character_is_base10(at_pos)){
                            goto double_break_2;
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }
                double_break_2:;
                i32 pos2 = pos;
                
                result = make_range(pos1, pos2);
            }
        }
    }
    
    return(result);
}

#if 0
static b32
get_numeric_string_at_cursor(Application_Links *app, Buffer_ID buffer, i32 start_pos, i32 *numeric_start, i32 *numeric_end){
    Range range = get_numeric_string_at_cursor(app, buffer, start_pos);
    b32 result = (range_size(range) > 0);
    if (result){
        *numeric_start = range.start;
        *numeric_end = range.end;
    }
    return(result);
}
#endif

struct Miblo_Number_Info{
    union{
        Range range;
        i32 start;
        i32 end;
    };
    i32 x;
};

static b32
get_numeric_at_cursor(Application_Links *app, Buffer_ID buffer, i32 pos, Miblo_Number_Info *info){
    b32 result = false;
    Range range = get_numeric_string_at_cursor(app, buffer, pos);
    if (range_size(range) > 0){
        Scratch_Block scratch(app);
        String_Const_u8 str = scratch_read(app, scratch, buffer, range);
        if (str.size > 0){
            i32 x = (i32)string_to_integer(str, 10);
            info->range = range;
            info->x = x;
            result = true;
        }
    }
    return(result);
}

CUSTOM_COMMAND_SIG(miblo_increment_basic)
CUSTOM_DOC("Increment an integer under the cursor by one.")
{
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Miblo_Number_Info number = {};
    if (get_numeric_at_cursor(app, buffer, pos, &number)){
        Scratch_Block scratch(app);
        String_Const_u8 str = string_u8_pushf(scratch, "%d", number.x + 1);
        buffer_replace_range(app, buffer, number.range, str);
        view_set_cursor(app, view, seek_pos(number.start + (i32)str.size - 1), true);
    }
}

CUSTOM_COMMAND_SIG(miblo_decrement_basic)
CUSTOM_DOC("Decrement an integer under the cursor by one.")
{
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Miblo_Number_Info number = {};
    if (get_numeric_at_cursor(app, buffer, pos, &number)){
        Scratch_Block scratch(app);
        String_Const_u8 str = string_u8_pushf(scratch, "%d", number.x - 1);
        buffer_replace_range(app, buffer, number.range, str);
        view_set_cursor(app, view, seek_pos(number.start + (i32)str.size - 1), true);
    }
}

// NOTE(allen): miblo time stamp format
// (h+:)?m?m:ss

static Range
get_timestamp_string_at_cursor(Application_Links *app, Buffer_ID buffer, i32 start_pos){
    Range result = {};
    
    char current = buffer_get_char(app, buffer, start_pos);
    
    if (character_is_base10(current) || current == ':'){
        char chunk[1024];
        i32 chunk_size = sizeof(chunk);
        Stream_Chunk stream = {};
        
        i32 pos = start_pos;
        
        i32 pos1 = 0;
        i32 pos2 = 0;
        
        if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
            b32 still_looping = true;
            while (still_looping){
                for (; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (!(character_is_base10(at_pos) || at_pos == ':')){
                        ++pos;
                        goto double_break_1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }
            double_break_1:;
            pos1 = pos;
            
            if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
                still_looping = true;
                while (still_looping){
                    for (; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (!(character_is_base10(at_pos) || at_pos == ':')){
                            goto double_break_2;
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }
                double_break_2:;
                pos2 = pos;
                
                result = make_range(pos1, pos2);
            }
        }
    }
    
    return(result);
}

#if 0
static b32
get_timestamp_string_at_cursor(Application_Links *app, Buffer_ID buffer, i32 start_pos, i32 *timestamp_start, i32 *timestamp_end){
    Range range = get_timestamp_string_at_cursor(app, buffer, start_pos);
    *timestamp_start = range.start;
    *timestamp_end = range.end;
    return(range_size(range) > 0);
}
#endif

struct Miblo_Timestamp{
    i32 second;
    i32 minute;
    i32 hour;
};
static Miblo_Timestamp null_miblo_timestamp = {};

enum{
    MIBLO_SECOND,
    MIBLO_MINUTE,
    MIBLO_HOUR
};

static Miblo_Timestamp
increment_timestamp(Miblo_Timestamp t, i32 type, i32 amt){
    Miblo_Timestamp r = t;
    switch (type){
        case MIBLO_SECOND: /* CASE second */
        r.second += amt;
        
        // 1. Modulo r.second into [0,59]
        // 2. What is thrown away by (1) store in amt, divide by 60, round down even when negative.
        amt = 0;
        if (r.second < 0){
            i32 pos_second = -r.second;
            amt = -((pos_second + 59)/60);
            r.second = 60 - (pos_second % 60);
        }
        else if (r.second >= 60){
            amt = r.second/60;
            r.second = (r.second % 60);
        }
        
        case MIBLO_MINUTE:  /* CASE minute */
        r.minute += amt;
        
        // 1. Modulo r.minute into [0,59]
        // 2. What is thrown away by (1) store in amt, divide by 60, round down even when negative.
        amt = 0;
        if (r.minute < 0){
            i32 pos_minute = -r.minute;
            amt = -((pos_minute + 59)/60);
            r.minute = 60 - (pos_minute % 60);
        }
        else if (r.minute >= 60){
            amt = r.minute/60;
            r.minute = (r.minute % 60);
        }
        
        case MIBLO_HOUR:  /* CASE hour */
        r.hour += amt;
        if (r.hour < 0){
            r.second = 0;
            r.minute = 0;
            r.hour = 0;
        }
    }
    
    return(r);
}

static String_Const_u8
timestamp_to_string(Arena *arena, Miblo_Timestamp t){
    List_String_Const_u8 list = {};
    if (t.hour > 0){
        string_list_pushf(arena, &list, "%d:", t.hour);
    }
    i32 minute = clamp_bot(0, t.minute);
    string_list_pushf(arena, &list, "%02d:", minute);
    i32 second = clamp_bot(0, t.second);
    string_list_pushf(arena, &list, "%02d", second);
    String_Const_u8 str = string_list_flatten(arena, list);
    return(str);
}

struct Miblo_Timestamp_Info{
    union{
        Range range;
        i32 start;
        i32 end;
    };
    Miblo_Timestamp time;
};

static b32
get_timestamp_at_cursor(Application_Links *app, Buffer_ID buffer, i32 pos, Miblo_Timestamp_Info *info){
    b32 result = false;
    
    Scratch_Block scratch(app);
    
    Range time_stamp_range = get_timestamp_string_at_cursor(app, buffer, pos);
    if (range_size(time_stamp_range) > 0){
        String_Const_u8 string = scratch_read(app, scratch, buffer, time_stamp_range);
        if (string.size > 0){
            i32 count_colons = 0;
            for (umem i = 0; i < string.size; ++i){
                if (string.str[i] == ':'){
                    count_colons += 1;
                }
            }
            
            if (count_colons == 1 || count_colons == 2){
                Miblo_Timestamp t = {};
                
                b32 success = false;
                
                umem i = 0;
                umem number_start[3];
                umem number_end[3];
                for (i32 k = 0; k < 3; ++k){
                    number_start[k] = i;
                    for (; i <= string.size; ++i){
                        if (i == string.size || string.str[i] == ':'){
                            number_end[k] = i;
                            break;
                        }
                    }
                    ++i;
                    if (i >= time_stamp_range.one_past_last){
                        break;
                    }
                }
                
                if (count_colons == 2){
                    String_Const_u8 hour_str = SCu8(string.str + number_start[0],
                                                    string.str + number_end[0]);
                    t.hour = (i32)string_to_integer(hour_str, 10);
                    
                    if (number_end[1] - number_start[1] == 2){
                        String_Const_u8 minute_str = SCu8(string.str + number_start[1],
                                                          string.str + number_end[1]);
                        t.minute = (i32)string_to_integer(minute_str, 10);
                        if (number_end[2] - number_start[2] == 2){
                            String_Const_u8 second_str = SCu8(string.str + number_start[2],
                                                              string.str + number_end[2]);
                            t.second = (i32)string_to_integer(second_str, 10);
                            success = true;
                        }
                    }
                }
                else{
                    if (number_end[0] - number_start[0] == 2 || number_end[0] - number_start[0] == 1){
                        String_Const_u8 minute_str = SCu8(string.str + number_start[0],
                                                          string.str + number_end[0]);
                        t.minute = (i32)string_to_integer(minute_str, 10);
                        
                        if (number_end[1] - number_start[1] == 2){
                            String_Const_u8 second_str = SCu8(string.str + number_start[2],
                                                              string.str + number_end[2]);
                            t.second = (i32)string_to_integer(second_str, 10);
                            success = true;
                        }
                    }
                }
                
                if (success){
                    info->range = time_stamp_range;
                    info->time = t;
                    result = true;
                }
            }
        }
    }
    
    return(result);
}

static void
miblo_time_stamp_alter(Application_Links *app, i32 unit_type, i32 amt){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    
    Miblo_Timestamp_Info timestamp = {};
    if (get_timestamp_at_cursor(app, buffer, pos, &timestamp)){
        Scratch_Block scratch(app);
        Miblo_Timestamp inc_timestamp = increment_timestamp(timestamp.time, unit_type, amt);
        String_Const_u8 str = timestamp_to_string(scratch, inc_timestamp);
        buffer_replace_range(app, buffer, timestamp.range, str);
        view_set_cursor(app, view, seek_pos(timestamp.start + (i32)str.size - 1), true);
    }
}

CUSTOM_COMMAND_SIG(miblo_increment_time_stamp)
CUSTOM_DOC("Increment a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss")
{
    miblo_time_stamp_alter(app, MIBLO_SECOND, 1);
}

CUSTOM_COMMAND_SIG(miblo_decrement_time_stamp)
CUSTOM_DOC("Decrement a time stamp under the cursor by one second. (format [m]m:ss or h:mm:ss")
{
    miblo_time_stamp_alter(app, MIBLO_SECOND, -1);
}

CUSTOM_COMMAND_SIG(miblo_increment_time_stamp_minute)
CUSTOM_DOC("Increment a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss")
{
    miblo_time_stamp_alter(app, MIBLO_MINUTE, 1);
}

CUSTOM_COMMAND_SIG(miblo_decrement_time_stamp_minute)
CUSTOM_DOC("Decrement a time stamp under the cursor by one minute. (format [m]m:ss or h:mm:ss")
{
    miblo_time_stamp_alter(app, MIBLO_MINUTE, -1);
}

// BOTTOM
