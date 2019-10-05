/*
 * Printf style operations for strings and what-not
 */

// TOP

#if !defined(FCODER_STRINGF_CPP)
#define FCODER_STRINGF_CPP

#include <stdarg.h>
#include <stdio.h>

static String_Const_char
push_stringfv(Arena *arena, char *format, va_list args){
    i32 size = vsnprintf(0, 0, format, args);
    String_Const_char result = string_const_char_push(arena, size + 1);
    vsnprintf(result.str, result.size, format, args);
    result.size -= 1;
    result.str[result.size] = 0;
    return(result);
}
static String_Const_char
push_stringf(Arena *arena, char *format, ...){
    va_list args;
    va_start(args, format);
    String_Const_char result = push_stringfv(arena, format, args);
    va_end(args);
    return(result);
}
static String_Const_u8 
push_u8_stringfv(Arena *arena, char *format, va_list args){
    return(SCu8(push_stringfv(arena, format, args)));
}
static String_Const_u8
push_u8_stringf(Arena *arena, char *format, ...){
    va_list args;
    va_start(args, format);
    String_Const_u8 result = SCu8(push_stringfv(arena, format, args));
    va_end(args);
    return(result);
}

static void
string_list_pushfv(Arena *arena, List_String_Const_char *list, char *format, va_list args){
    String_Const_char string = push_stringfv(arena, format, args);
    if (arena->alignment < sizeof(umem)){
        push_align(arena, sizeof(umem));
    }
    string_list_push(arena, list, string);
}
static void
string_list_pushf(Arena *arena, List_String_Const_char *list, char *format, ...){
    va_list args;
    va_start(args, format);
    string_list_pushfv(arena, list, format, args);
    va_end(args);
}
static void
string_list_pushfv(Arena *arena, List_String_Const_u8 *list, char *format, va_list args){
    String_Const_u8 string = push_u8_stringfv(arena, format, args);
    if (arena->alignment < sizeof(umem)){
        push_align(arena, sizeof(umem));
    }
    string_list_push(arena, list, string);
}
static void
string_list_pushf(Arena *arena, List_String_Const_u8 *list, char *format, ...){
    va_list args;
    va_start(args, format);
    string_list_pushfv(arena, list, format, args);
    va_end(args);
}

#endif

// BOTTOM

