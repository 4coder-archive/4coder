/*
4coder_miblo_numbers.cpp - Commands for so called "Miblo Number Operations" which involve incrementing
and decrementing various forms of number as numerical objects despite being encoded as text objects.
*/

// TOP

static i32
get_numeric_string_at_cursor(Application_Links *app, Buffer_ID buffer, i32 start_pos, i32 *numeric_start, i32 *numeric_end){
    i32 result = 0;
    
    char current = buffer_get_char(app, buffer, start_pos);
    
    if (char_is_numeric(current)){
        char chunk[1024];
        i32 chunk_size = sizeof(chunk);
        Stream_Chunk stream = {};
        
        i32 pos = start_pos;
        
        i32 pos1 = 0;
        i32 pos2 = 0;
        
        if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
            
            b32 still_looping = true;
            for (;still_looping;){
                for (; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (!char_is_numeric(at_pos)){
                        ++pos;
                        goto double_break_1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }
            double_break_1:;
            pos1 = pos;
            
            if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
                still_looping = 1;
                while (still_looping){
                    for (; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (!char_is_numeric(at_pos)){
                            goto double_break_2;
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }
                double_break_2:;
                pos2 = pos;
                
                result = 1;
                *numeric_start = pos1;
                *numeric_end = pos2;
            }
        }
    }
    
    return(result);
}

struct Miblo_Number_Info{
    union{
        Range range;
        i32 start;
        i32 end;
    };
    i32 x;
};

static i32
get_numeric_at_cursor(Application_Links *app, Buffer_ID buffer, i32 pos, Miblo_Number_Info *info){
    i32 result = 0;
    
    i32 numeric_start = 0, numeric_end = 0;
    if (get_numeric_string_at_cursor(app, buffer, pos, &numeric_start, &numeric_end)){
        char numeric_string[1024];
        String str = make_string(numeric_string, numeric_end - numeric_start, sizeof(numeric_string));
        if (str.size < str.memory_size){
            buffer_read_range(app, buffer, numeric_start, numeric_end, numeric_string);
            
            i32 x = str_to_int(str);
            int_to_str(&str, x+1);
            
            info->start = numeric_start;
            info->end = numeric_end;
            info->x = x;
            result = 1;
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(miblo_increment_basic)
CUSTOM_DOC("Increment an integer under the cursor by one.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessOpen, &buffer);
    Miblo_Number_Info number = {};
    if (get_numeric_at_cursor(app, buffer, view.cursor.pos, &number)){
        char str_space[1024];
        String str = make_fixed_width_string(str_space);
        int_to_str(&str, number.x + 1);
        buffer_replace_range(app, buffer, number.range, str);
        view_set_cursor(app, &view, seek_pos(number.start + str.size - 1), 1);
    }
}

CUSTOM_COMMAND_SIG(miblo_decrement_basic)
CUSTOM_DOC("Decrement an integer under the cursor by one.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessOpen, &buffer);
    Miblo_Number_Info number = {};
    if (get_numeric_at_cursor(app, buffer, view.cursor.pos, &number)){
        char str_space[1024];
        String str = make_fixed_width_string(str_space);
        int_to_str(&str, number.x - 1);
        buffer_replace_range(app, buffer, number.range, str);
        view_set_cursor(app, &view, seek_pos(number.start + str.size - 1), 1);
    }
}

// NOTE(allen): miblo time stamp format
// (h+:)?m?m:ss

static i32
get_timestamp_string_at_cursor(Application_Links *app, Buffer_ID buffer, i32 start_pos, i32 *timestamp_start, i32 *timestamp_end){
    i32 result = 0;
    
    char current = buffer_get_char(app, buffer, start_pos);
    
    if (char_is_numeric(current) || current == ':'){
        char chunk[1024];
        i32 chunk_size = sizeof(chunk);
        Stream_Chunk stream = {};
        
        i32 pos = start_pos;
        
        i32 pos1 = 0;
        i32 pos2 = 0;
        
        if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
            
            i32 still_looping = 1;
            while (still_looping){
                for (; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (!(char_is_numeric(at_pos) || at_pos == ':')){
                        ++pos;
                        goto double_break_1;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }
            double_break_1:;
            pos1 = pos;
            
            if (init_stream_chunk(&stream, app, buffer, start_pos, chunk, chunk_size)){
                
                still_looping = 1;
                while (still_looping){
                    for (; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (!(char_is_numeric(at_pos) || at_pos == ':')){
                            goto double_break_2;
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }
                double_break_2:;
                pos2 = pos;
                
                result = 1;
                *timestamp_start = pos1;
                *timestamp_end = pos2;
            }
        }
    }
    
    return(result);
}

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

static void
timestamp_to_str(String *dest, Miblo_Timestamp t){
    dest->size = 0;
    
    if (t.hour > 0){
        append_int_to_str(dest, t.hour);
        append(dest, ":");
    }
    
    if (t.minute >= 10){
        append_int_to_str(dest, t.minute);
    }
    else if (t.hour > 0){
        append(dest, "0");
        append_int_to_str(dest, t.minute);
    }
    else{
        append_int_to_str(dest, t.minute);
    }
    append(dest, ":");
    
    if (t.second >= 10){
        append_int_to_str(dest, t.second);
    }
    else{
        append(dest, "0");
        append_int_to_str(dest, t.second);
    }
}

struct Miblo_Timestamp_Info{
    union{
        Range range;
        i32 start;
        i32 end;
    };
    Miblo_Timestamp time;
};

static i32
get_timestamp_at_cursor(Application_Links *app, Buffer_ID buffer, i32 pos, Miblo_Timestamp_Info *info){
    i32 result = 0;
    
    i32 timestamp_start = 0, timestamp_end = 0;
    if (get_timestamp_string_at_cursor(app, buffer, pos, &timestamp_start, &timestamp_end)){
        char timestamp_string[1024];
        String str = make_string(timestamp_string, timestamp_end - timestamp_start, sizeof(timestamp_string));
        if (str.size < str.memory_size){
            buffer_read_range(app, buffer, timestamp_start, timestamp_end, timestamp_string);
            
            i32 count_colons = 0;
            for (i32 i = 0; i < str.size; ++i){
                if (str.str[i] == ':'){
                    ++count_colons;
                }
            }
            
            if (count_colons == 1 || count_colons == 2){
                Miblo_Timestamp t = {};
                
                i32 success = 0;
                
                i32 i = 0;
                i32 number_start[3], number_end[3];
                for (i32 k = 0; k < 3; ++k){
                    number_start[k] = i;
                    for (; i <= str.size; ++i){
                        if (i == str.size || str.str[i] == ':'){
                            number_end[k] = i;
                            break;
                        }
                    }
                    ++i;
                    if (i >= timestamp_end){
                        break;
                    }
                }
                
                if (count_colons == 2){
                    t.hour = str_to_int(make_string(str.str + number_start[0], number_end[0] - number_start[0]));
                    
                    if (number_end[1] - number_start[1] == 2){
                        
                        t.minute = str_to_int(make_string(str.str + number_start[1], number_end[1] - number_start[1]));
                        
                        if (number_end[2] - number_start[2] == 2){
                            t.second = str_to_int(make_string(str.str + number_start[2], number_end[2] - number_start[2]));
                            
                            success = 1;
                        }
                    }
                }
                else{
                    if (number_end[0] - number_start[0] == 2 || number_end[0] - number_start[0] == 1){
                        t.minute = str_to_int(make_string(str.str + number_start[0], number_end[0] - number_start[0]));
                        
                        if (number_end[1] - number_start[1] == 2){
                            t.second = str_to_int(make_string(str.str + number_start[1], number_end[1] - number_start[1]));
                            
                            success = 1;
                        }
                    }
                }
                
                if (success){
                    info->start = timestamp_start;
                    info->end = timestamp_end;
                    info->time = t;
                    result = 1;
                }
            }
        }
    }
    
    return(result);
}

static void
miblo_time_stamp_alter(Application_Links *app, i32 unit_type, i32 amt){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessOpen, &buffer);
    
    Miblo_Timestamp_Info timestamp = {};
    if (get_timestamp_at_cursor(app, buffer, view.cursor.pos, &timestamp)){
        char str_space[1024];
        String str = make_fixed_width_string(str_space);
        
        Miblo_Timestamp inc_timestamp = increment_timestamp(timestamp.time, unit_type, amt);
        timestamp_to_str(&str, inc_timestamp);
        buffer_replace_range(app, buffer, timestamp.range, str);
        view_set_cursor(app, &view, seek_pos(timestamp.start + str.size - 1), 1);
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
