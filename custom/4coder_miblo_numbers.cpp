/*
4coder_miblo_numbers.cpp - Commands for so called "Miblo Number Operations" which involve incrementing
and decrementing various forms of number as numerical objects despite being encoded as text objects.
*/

// TOP

struct Miblo_Number_Info{
    Range_i64 range;
    i64 x;
};

static b32
get_numeric_at_cursor(Application_Links *app, Buffer_ID buffer, i64 pos, Miblo_Number_Info *info){
    b32 result = false;
    Range_i64 range = enclose_pos_base10(app, buffer, pos);
    if (range_size(range) > 0){
        Scratch_Block scratch(app);
        String_Const_u8 str = push_buffer_range(app, scratch, buffer, range);
        if (str.size > 0){
            info->range = range;
            info->x = string_to_integer(str, 10);
            result = true;
        }
    }
    return(result);
}

CUSTOM_COMMAND_SIG(miblo_increment_basic)
CUSTOM_DOC("Increment an integer under the cursor by one.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Miblo_Number_Info number = {};
    if (get_numeric_at_cursor(app, buffer, pos, &number)){
        Scratch_Block scratch(app);
        String_Const_u8 str = push_u8_stringf(scratch, "%d", number.x + 1);
        buffer_replace_range(app, buffer, number.range, str);
        view_set_cursor_and_preferred_x(app, view, seek_pos(number.range.start + str.size - 1));
    }
}

CUSTOM_COMMAND_SIG(miblo_decrement_basic)
CUSTOM_DOC("Decrement an integer under the cursor by one.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Miblo_Number_Info number = {};
    if (get_numeric_at_cursor(app, buffer, pos, &number)){
        Scratch_Block scratch(app);
        String_Const_u8 str = push_u8_stringf(scratch, "%d", number.x - 1);
        buffer_replace_range(app, buffer, number.range, str);
        view_set_cursor_and_preferred_x(app, view, seek_pos(number.range.start + str.size - 1));
    }
}

// NOTE(allen): miblo time stamp format
// (h+:)?m?m:ss

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
    Range_i64 range;
    Miblo_Timestamp time;
};

static b32
get_timestamp_at_cursor(Application_Links *app, Buffer_ID buffer, i64 pos, Miblo_Timestamp_Info *info){
    b32 result = false;
    
    Scratch_Block scratch(app);
    
    Range_i64 time_stamp_range = enclose_pos_base10_colon(app, buffer, pos);
    if (range_size(time_stamp_range) > 0){
        String_Const_u8 string = push_buffer_range(app, scratch, buffer, time_stamp_range);
        if (string.size > 0){
            i32 count_colons = 0;
            for (u64 i = 0; i < string.size; ++i){
                if (string.str[i] == ':'){
                    count_colons += 1;
                }
            }
            
            if (count_colons == 1 || count_colons == 2){
                Miblo_Timestamp t = {};
                
                b32 success = false;
                
                Range_i64 number[3];
                i32 k = 0;
                number[0].min = 0;
                for (i64 i = 0; i < (i64)string.size; i += 1){
                    if (string.str[i] == ':'){
                        number[k].max = i;
                        k += 1;
                        number[k].min = i + 1;
                    }
                }
                number[k].max = (i64)string.size;
                
                if (count_colons == 2){
                    String_Const_u8 hour_str = string_substring(string, number[0]);
                    t.hour = (i32)string_to_integer(hour_str, 10);
                    
                    if (range_size(number[1]) == 2){
                        String_Const_u8 minute_str = string_substring(string, number[1]);
                        t.minute = (i32)string_to_integer(minute_str, 10);
                        if (range_size(number[2]) == 2){
                            String_Const_u8 second_str = string_substring(string, number[2]);
                            t.second = (i32)string_to_integer(second_str, 10);
                            success = true;
                        }
                    }
                }
                else{
                    if (range_size(number[0]) == 2 || range_size(number[0]) == 1){
                        String_Const_u8 minute_str = string_substring(string, number[0]);
                        t.minute = (i32)string_to_integer(minute_str, 10);
                        
                        if (range_size(number[1]) == 2){
                            String_Const_u8 second_str = string_substring(string, number[1]);
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
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    
    Miblo_Timestamp_Info timestamp = {};
    if (get_timestamp_at_cursor(app, buffer, pos, &timestamp)){
        Scratch_Block scratch(app);
        Miblo_Timestamp inc_timestamp = increment_timestamp(timestamp.time, unit_type, amt);
        String_Const_u8 str = timestamp_to_string(scratch, inc_timestamp);
        buffer_replace_range(app, buffer, timestamp.range, str);
        view_set_cursor_and_preferred_x(app, view, seek_pos(timestamp.range.start + str.size - 1));
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
