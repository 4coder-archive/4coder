/*
 * Printf style operations for strings and what-not
 */

// TOP

#if !defined(FCODER_STRINGF_CPP)
#define FCODER_STRINGF_CPP

#include <stdarg.h>
#include <stdio.h>

function String_Const_u8
push_stringfv(Arena *arena, char *format, va_list args){
    va_list args2;
    va_copy(args2, args);
    i32 size = vsnprintf(0, 0, format, args);
    String_Const_u8 result = string_const_u8_push(arena, size + 1);
    vsnprintf((char*)result.str, (size_t)result.size, format, args2);
    result.size -= 1;
    result.str[result.size] = 0;
    return(result);
}
function String_Const_u8
push_stringf(Arena *arena, char *format, ...){
    va_list args;
    va_start(args, format);
    String_Const_u8 result = push_stringfv(arena, format, args);
    va_end(args);
    return(result);
}
function String_Const_u8
push_u8_stringfv(Arena *arena, char *format, va_list args){
    return(push_stringfv(arena, format, args));
}
function String_Const_u8
push_u8_stringf(Arena *arena, char *format, ...){
    va_list args;
    va_start(args, format);
    String_Const_u8 result = push_stringfv(arena, format, args);
    va_end(args);
    return(result);
}

function void
string_list_pushfv(Arena *arena, List_String_Const_char *list, char *format, va_list args){
    String_Const_u8 string = push_stringfv(arena, format, args);
    if (arena->alignment < sizeof(u64)){
        push_align(arena, sizeof(u64));
    }
    string_list_push(arena, list, SCchar(string));
}
function void
string_list_pushf(Arena *arena, List_String_Const_char *list, char *format, ...){
    va_list args;
    va_start(args, format);
    string_list_pushfv(arena, list, format, args);
    va_end(args);
}
function void
string_list_pushfv(Arena *arena, List_String_Const_u8 *list, char *format, va_list args){
    String_Const_u8 string = push_u8_stringfv(arena, format, args);
    if (arena->alignment < sizeof(u64)){
        push_align(arena, sizeof(u64));
    }
    string_list_push(arena, list, string);
}
function void
string_list_pushf(Arena *arena, List_String_Const_u8 *list, char *format, ...){
    va_list args;
    va_start(args, format);
    string_list_pushfv(arena, list, format, args);
    va_end(args);
}

////////////////////////////////

// yyyy
function void
push_year_full(Arena *arena, List_String_Const_u8 *list, u32 year){
    string_list_pushf(arena, list, "%u", year);
}
// yy
function void
push_year_abrev(Arena *arena, List_String_Const_u8 *list, u32 year){
    string_list_pushf(arena, list, "%u", year % 100);
}

// m
function void
push_month_num(Arena *arena, List_String_Const_u8 *list, u8 mon){
    string_list_pushf(arena, list, "%u", mon + 1);
}
// mm
function void
push_month_num_zeros(Arena *arena, List_String_Const_u8 *list, u8 mon){
    string_list_pushf(arena, list, "%02u", mon + 1);
}
// month
function void
push_month_name(Arena *arena, List_String_Const_u8 *list, u8 mon){
    string_list_push(arena, list, month_full_name[mon%12]);
}
// mon
function void
push_month_abrev(Arena *arena, List_String_Const_u8 *list, u8 mon){
    string_list_push(arena, list, month_abrev_name[mon%12]);
}

// d
function void
push_day_num(Arena *arena, List_String_Const_u8 *list, u8 day){
    string_list_pushf(arena, list, "%u", day + 1);
}
// dd
function void
push_day_num_zeroes(Arena *arena, List_String_Const_u8 *list, u8 day){
    string_list_pushf(arena, list, "%02u", day + 1);
}
// day
function void
push_day_ord(Arena *arena, List_String_Const_u8 *list, u8 day){
    string_list_push(arena, list, ordinal_numeric_name[day%100]);
}

// h24
function void
push_hour_24(Arena *arena, List_String_Const_u8 *list, u8 hour){
    string_list_pushf(arena, list, "%u", hour);
}
// hh24
function void
push_hour_24_zeroes(Arena *arena, List_String_Const_u8 *list, u8 hour){
    string_list_pushf(arena, list, "%02u", hour);
}
// h
function void
push_hour_12(Arena *arena, List_String_Const_u8 *list, u8 hour){
    string_list_pushf(arena, list, "%u", hour%12);
}
// hh
function void
push_hour_12_zeroes(Arena *arena, List_String_Const_u8 *list, u8 hour){
    string_list_pushf(arena, list, "%02u", hour%12);
}
// ampm
function void
push_hour_am_pm(Arena *arena, List_String_Const_u8 *list, u8 hour){
    if (hour >= 12){
        string_list_push(arena, list, string_u8_litexpr("pm"));
    }
    else{
        string_list_push(arena, list, string_u8_litexpr("am"));
    }
}

// mi
function void
push_minute(Arena *arena, List_String_Const_u8 *list, u8 min){
    string_list_pushf(arena, list, "%u", min);
}
// mimi
function void
push_minute_zeroes(Arena *arena, List_String_Const_u8 *list, u8 min){
    string_list_pushf(arena, list, "%02u", min);
}

// s
function void
push_second(Arena *arena, List_String_Const_u8 *list, u8 sec){
    string_list_pushf(arena, list, "%u", sec);
}
// ss
function void
push_second_zeroes(Arena *arena, List_String_Const_u8 *list, u8 sec){
    string_list_pushf(arena, list, "%02u", sec);
}

// ms
function void
push_millisecond_zeroes(Arena *arena, List_String_Const_u8 *list, u16 msec){
    string_list_pushf(arena, list, "%03u", msec);
}

function void
date_time_format(Arena *arena, List_String_Const_u8 *list, String_Const_u8 format, Date_Time *date_time){
    u8 *ptr = format.str;
    u8 *end = format.str + format.size;
    for (;ptr < end;){
        if (character_is_alpha_numeric(*ptr)){
            u8 *start = ptr;
            for (;ptr < end; ptr += 1){
                if (!character_is_alpha_numeric(*ptr)){
                    break;
                }
            }
            
            String_Const_u8 field = SCu8(start, ptr);
            for (; field.size > 0;){
                if (string_match(string_prefix(field, 5), string_u8_litexpr("month"))){
                    field = string_skip(field, 5);
                    push_month_name(arena, list, date_time->mon);
                }
                
                else if (string_match(string_prefix(field, 4), string_u8_litexpr("yyyy"))){
                    field = string_skip(field, 4);
                    push_year_full(arena, list, date_time->year);
                }
                else if (string_match(string_prefix(field, 4), string_u8_litexpr("hh24"))){
                    field = string_skip(field, 4);
                    push_hour_24_zeroes(arena, list, date_time->hour);
                }
                else if (string_match(string_prefix(field, 4), string_u8_litexpr("ampm"))){
                    field = string_skip(field, 4);
                    push_hour_am_pm(arena, list, date_time->hour);
                }
                else if (string_match(string_prefix(field, 4), string_u8_litexpr("mimi"))){
                    field = string_skip(field, 4);
                    push_minute_zeroes(arena, list, date_time->min);
                }
                
                else if (string_match(string_prefix(field, 3), string_u8_litexpr("mon"))){
                    field = string_skip(field, 3);
                    push_month_abrev(arena, list, date_time->mon);
                }
                else if (string_match(string_prefix(field, 3), string_u8_litexpr("day"))){
                    field = string_skip(field, 3);
                    push_day_ord(arena, list, date_time->day);
                }
                else if (string_match(string_prefix(field, 3), string_u8_litexpr("h24"))){
                    field = string_skip(field, 3);
                    push_hour_24(arena, list, date_time->hour);
                }
                
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("yy"))){
                    field = string_skip(field, 2);
                    push_year_abrev(arena, list, date_time->year);
                }
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("mm"))){
                    field = string_skip(field, 2);
                    push_month_num_zeros(arena, list, date_time->mon);
                }
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("dd"))){
                    field = string_skip(field, 2);
                    push_day_num_zeroes(arena, list, date_time->day);
                }
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("hh"))){
                    field = string_skip(field, 2);
                    push_hour_12_zeroes(arena, list, date_time->hour);
                }
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("mi"))){
                    field = string_skip(field, 2);
                    push_minute(arena, list, date_time->min);
                }
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("ss"))){
                    field = string_skip(field, 2);
                    push_second_zeroes(arena, list, date_time->sec);
                }
                else if (string_match(string_prefix(field, 2), string_u8_litexpr("ms"))){
                    field = string_skip(field, 2);
                    push_millisecond_zeroes(arena, list, date_time->msec);
                }
                
                else if (string_match(string_prefix(field, 1), string_u8_litexpr("m"))){
                    field = string_skip(field, 1);
                    push_month_num(arena, list, date_time->mon);
                }
                else if (string_match(string_prefix(field, 1), string_u8_litexpr("d"))){
                    field = string_skip(field, 1);
                    push_day_num(arena, list, date_time->day);
                }
                else if (string_match(string_prefix(field, 1), string_u8_litexpr("h"))){
                    field = string_skip(field, 1);
                    push_hour_12(arena, list, date_time->hour);
                }
                else if (string_match(string_prefix(field, 1), string_u8_litexpr("s"))){
                    field = string_skip(field, 1);
                    push_second(arena, list, date_time->sec);
                }
                
                else{
                    string_list_push(arena, list, SCu8(start, ptr));
                    break;
                }
            }
        }
        else{
            u8 *start = ptr;
            for (;ptr < end; ptr += 1){
                if (character_is_alpha_numeric(*ptr)){
                    break;
                }
            }
            string_list_push(arena, list, SCu8(start, ptr));
        }
    }
}
function void
date_time_format(Arena *arena, List_String_Const_u8 *list, char *format, Date_Time *date_time){
    date_time_format(arena, list, SCu8(format), date_time);
}

function String_Const_u8
date_time_format(Arena *arena, String_Const_u8 format, Date_Time *date_time){
    List_String_Const_u8 list = {};
    date_time_format(arena, &list, format, date_time);
    return(string_list_flatten(arena, list));
}
function String_Const_u8
date_time_format(Arena *arena, char *format, Date_Time *date_time){
    return(date_time_format(arena, SCu8(format), date_time));
}

#endif

// BOTTOM

