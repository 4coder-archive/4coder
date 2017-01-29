/*
4coder_string.h - Version 1.0.59
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.

To include implementation: #define FSTRING_IMPLEMENTATION
To use in C mode: #define FSTRING_C
*/

// TOP

// 4tech_standard_preamble.h
#if !defined(FTECH_INTEGERS)
#define FTECH_INTEGERS
#include <stdint.h>
typedef int8_t i8_4tech;
typedef int16_t i16_4tech;
typedef int32_t i32_4tech;
typedef int64_t i64_4tech;

typedef uint8_t u8_4tech;
typedef uint16_t u16_4tech;
typedef uint32_t u32_4tech;
typedef uint64_t u64_4tech;

typedef float f32_4tech;
typedef double f64_4tech;

typedef int8_t b8_4tech;
typedef int32_t b32_4tech;
#endif

#if !defined(Assert)
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif
// standard preamble end 


#if !defined(FSTRING_LINK)
# define FSTRING_LINK static
#endif

#if !defined(FSTRING_INLINE)
# if defined(FSTRING_C)
#  define FSTRING_INLINE static
# else
#  define FSTRING_INLINE inline
# endif
#endif

#if !defined(FSTRING_GUARD)
#define literal(s) (s), (sizeof(s)-1)

typedef struct String{
    char *str;
    i32_4tech size;
    i32_4tech memory_size;
} String;

static String null_string = {0};
#endif

#if !defined(FCODER_STRING_H)
#define FCODER_STRING_H

FSTRING_INLINE b32_4tech           char_is_slash(char c);
FSTRING_INLINE b32_4tech           char_is_upper(char c);
FSTRING_INLINE b32_4tech           char_is_lower(char c);
FSTRING_INLINE char                char_to_upper(char c);
FSTRING_INLINE char                char_to_lower(char c);
FSTRING_INLINE b32_4tech           char_is_whitespace(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_numeric(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_numeric_true(char c);
FSTRING_INLINE b32_4tech           char_is_alpha(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_true(char c);
FSTRING_INLINE b32_4tech           char_is_hex(char c);
FSTRING_INLINE b32_4tech           char_is_numeric(char c);
FSTRING_INLINE String              make_string_cap(void *str, i32_4tech size, i32_4tech mem_size);
FSTRING_INLINE String              make_string(void *str, i32_4tech size);
#ifndef   make_lit_string
# define make_lit_string(s) (make_string_cap((char*)(s), sizeof(s)-1, sizeof(s)))
#endif
#ifndef   make_fixed_width_string
# define make_fixed_width_string(s) (make_string_cap((char*)(s), 0, sizeof(s)))
#endif
#ifndef   expand_str
# define expand_str(s) ((s).str), ((s).size)
#endif
FSTRING_LINK i32_4tech             str_size(char *str);
FSTRING_INLINE String              make_string_slowly(void *str);
FSTRING_INLINE String              substr_tail(String str, i32_4tech start);
FSTRING_INLINE String              substr(String str, i32_4tech start, i32_4tech size);
FSTRING_LINK String                skip_whitespace(String str);
FSTRING_LINK String                skip_whitespace_measure(String str, i32_4tech *skip_length);
FSTRING_LINK String                chop_whitespace(String str);
FSTRING_LINK String                skip_chop_whitespace(String str);
FSTRING_LINK String                skip_chop_whitespace_measure(String str, i32_4tech *skip_length);
FSTRING_INLINE String              tailstr(String str);
FSTRING_LINK b32_4tech             match_cc(char *a, char *b);
FSTRING_LINK b32_4tech             match_sc(String a, char *b);
FSTRING_INLINE b32_4tech           match_cs(char *a, String b);
FSTRING_LINK b32_4tech             match_ss(String a, String b);
FSTRING_LINK b32_4tech             match_part_ccl(char *a, char *b, i32_4tech *len);
FSTRING_LINK b32_4tech             match_part_scl(String a, char *b, i32_4tech *len);
FSTRING_INLINE b32_4tech           match_part_cc(char *a, char *b);
FSTRING_INLINE b32_4tech           match_part_sc(String a, char *b);
FSTRING_LINK b32_4tech             match_part_cs(char *a, String b);
FSTRING_LINK b32_4tech             match_part_ss(String a, String b);
FSTRING_LINK b32_4tech             match_insensitive_cc(char *a, char *b);
FSTRING_LINK b32_4tech             match_insensitive_sc(String a, char *b);
FSTRING_INLINE b32_4tech           match_insensitive_cs(char *a, String b);
FSTRING_LINK b32_4tech             match_insensitive_ss(String a, String b);
FSTRING_LINK b32_4tech             match_part_insensitive_ccl(char *a, char *b, i32_4tech *len);
FSTRING_LINK b32_4tech             match_part_insensitive_scl(String a, char *b, i32_4tech *len);
FSTRING_INLINE b32_4tech           match_part_insensitive_cc(char *a, char *b);
FSTRING_INLINE b32_4tech           match_part_insensitive_sc(String a, char *b);
FSTRING_LINK b32_4tech             match_part_insensitive_cs(char *a, String b);
FSTRING_LINK b32_4tech             match_part_insensitive_ss(String a, String b);
FSTRING_LINK i32_4tech             compare_cc(char *a, char *b);
FSTRING_LINK i32_4tech             compare_sc(String a, char *b);
FSTRING_INLINE i32_4tech           compare_cs(char *a, String b);
FSTRING_LINK i32_4tech             compare_ss(String a, String b);
FSTRING_LINK i32_4tech             find_c_char(char *str, i32_4tech start, char character);
FSTRING_LINK i32_4tech             find_s_char(String str, i32_4tech start, char character);
FSTRING_LINK i32_4tech             rfind_s_char(String str, i32_4tech start, char character);
FSTRING_LINK i32_4tech             find_c_chars(char *str, i32_4tech start, char *characters);
FSTRING_LINK i32_4tech             find_s_chars(String str, i32_4tech start, char *characters);
FSTRING_LINK i32_4tech             find_substr_c(char *str, i32_4tech start, String seek);
FSTRING_LINK i32_4tech             find_substr_s(String str, i32_4tech start, String seek);
FSTRING_LINK i32_4tech             rfind_substr_s(String str, i32_4tech start, String seek);
FSTRING_LINK i32_4tech             find_substr_insensitive_c(char *str, i32_4tech start, String seek);
FSTRING_LINK i32_4tech             find_substr_insensitive_s(String str, i32_4tech start, String seek);
FSTRING_INLINE b32_4tech           has_substr_c(char *s, String seek);
FSTRING_INLINE b32_4tech           has_substr_s(String s, String seek);
FSTRING_INLINE b32_4tech           has_substr_insensitive_c(char *s, String seek);
FSTRING_INLINE b32_4tech           has_substr_insensitive_s(String s, String seek);
FSTRING_LINK i32_4tech             copy_fast_unsafe_cc(char *dest, char *src);
FSTRING_LINK i32_4tech             copy_fast_unsafe_cs(char *dest, String src);
FSTRING_LINK b32_4tech             copy_checked_ss(String *dest, String src);
FSTRING_LINK b32_4tech             copy_partial_sc(String *dest, char *src);
FSTRING_LINK b32_4tech             copy_partial_ss(String *dest, String src);
FSTRING_INLINE i32_4tech           copy_cc(char *dest, char *src);
FSTRING_INLINE void                copy_ss(String *dest, String src);
FSTRING_INLINE void                copy_sc(String *dest, char *src);
FSTRING_LINK b32_4tech             append_checked_ss(String *dest, String src);
FSTRING_LINK b32_4tech             append_partial_sc(String *dest, char *src);
FSTRING_LINK b32_4tech             append_partial_ss(String *dest, String src);
FSTRING_LINK b32_4tech             append_s_char(String *dest, char c);
FSTRING_INLINE b32_4tech           append_ss(String *dest, String src);
FSTRING_INLINE b32_4tech           append_sc(String *dest, char *src);
FSTRING_LINK b32_4tech             terminate_with_null(String *str);
FSTRING_LINK b32_4tech             append_padding(String *dest, char c, i32_4tech target_size);
FSTRING_LINK void                  replace_char(String *str, char replace, char with);
FSTRING_LINK void                  to_lower_cc(char *src, char *dst);
FSTRING_LINK void                  to_lower_ss(String *dst, String src);
FSTRING_LINK void                  to_lower_s(String *str);
FSTRING_LINK void                  to_upper_cc(char *src, char *dst);
FSTRING_LINK void                  to_upper_ss(String *dst, String src);
FSTRING_LINK void                  to_upper_s(String *str);
FSTRING_LINK void                  to_camel_cc(char *src, char *dst);
FSTRING_LINK i32_4tech             int_to_str_size(i32_4tech x);
FSTRING_LINK b32_4tech             int_to_str(String *dest, i32_4tech x);
FSTRING_LINK b32_4tech             append_int_to_str(String *dest, i32_4tech x);
FSTRING_LINK i32_4tech             u64_to_str_size(uint64_t x);
FSTRING_LINK b32_4tech             u64_to_str(String *dest, uint64_t x);
FSTRING_LINK b32_4tech             append_u64_to_str(String *dest, uint64_t x);
FSTRING_LINK i32_4tech             float_to_str_size(float x);
FSTRING_LINK b32_4tech             append_float_to_str(String *dest, float x);
FSTRING_LINK b32_4tech             float_to_str(String *dest, float x);
FSTRING_LINK i32_4tech             str_is_int_c(char *str);
FSTRING_LINK b32_4tech             str_is_int_s(String str);
FSTRING_LINK i32_4tech             str_to_int_c(char *str);
FSTRING_LINK i32_4tech             str_to_int_s(String str);
FSTRING_LINK i32_4tech             hexchar_to_int(char c);
FSTRING_LINK char                  int_to_hexchar(i32_4tech x);
FSTRING_LINK u32_4tech             hexstr_to_int(String str);
FSTRING_LINK b32_4tech             color_to_hexstr(String *s, u32_4tech color);
FSTRING_LINK b32_4tech             hexstr_to_color(String s, u32_4tech *out);
FSTRING_LINK i32_4tech             reverse_seek_slash_pos(String str, i32_4tech pos);
FSTRING_INLINE i32_4tech           reverse_seek_slash(String str);
FSTRING_INLINE String              front_of_directory(String dir);
FSTRING_INLINE String              path_of_directory(String dir);
FSTRING_LINK b32_4tech             set_last_folder_sc(String *dir, char *folder_name, char slash);
FSTRING_LINK b32_4tech             set_last_folder_ss(String *dir, String folder_name, char slash);
FSTRING_LINK String                file_extension(String str);
FSTRING_LINK b32_4tech             remove_extension(String *str);
FSTRING_LINK b32_4tech             remove_last_folder(String *str);
FSTRING_LINK b32_4tech             string_set_match_table(void *str_set, i32_4tech item_size, i32_4tech count, String str, i32_4tech *match_index);
FSTRING_LINK b32_4tech             string_set_match(String *str_set, i32_4tech count, String str, i32_4tech *match_index);
FSTRING_LINK String                get_first_double_line(String source);
FSTRING_LINK String                get_next_double_line(String source, String line);
FSTRING_LINK String                get_next_word(String source, String prev_word);
FSTRING_LINK String                get_first_word(String source);

#endif

#if !defined(FSTRING_C) && !defined(FSTRING_GUARD)

FSTRING_INLINE String              make_string(void *str, i32_4tech size, i32_4tech mem_size){return(make_string_cap(str,size,mem_size));}
FSTRING_INLINE String              substr(String str, i32_4tech start){return(substr_tail(str,start));}
FSTRING_LINK String                skip_whitespace(String str, i32_4tech *skip_length){return(skip_whitespace_measure(str,skip_length));}
FSTRING_LINK String                skip_chop_whitespace(String str, i32_4tech *skip_length){return(skip_chop_whitespace_measure(str,skip_length));}
FSTRING_LINK b32_4tech             match(char *a, char *b){return(match_cc(a,b));}
FSTRING_LINK b32_4tech             match(String a, char *b){return(match_sc(a,b));}
FSTRING_INLINE b32_4tech           match(char *a, String b){return(match_cs(a,b));}
FSTRING_LINK b32_4tech             match(String a, String b){return(match_ss(a,b));}
FSTRING_LINK b32_4tech             match_part(char *a, char *b, i32_4tech *len){return(match_part_ccl(a,b,len));}
FSTRING_LINK b32_4tech             match_part(String a, char *b, i32_4tech *len){return(match_part_scl(a,b,len));}
FSTRING_INLINE b32_4tech           match_part(char *a, char *b){return(match_part_cc(a,b));}
FSTRING_INLINE b32_4tech           match_part(String a, char *b){return(match_part_sc(a,b));}
FSTRING_LINK b32_4tech             match_part(char *a, String b){return(match_part_cs(a,b));}
FSTRING_LINK b32_4tech             match_part(String a, String b){return(match_part_ss(a,b));}
FSTRING_LINK b32_4tech             match_insensitive(char *a, char *b){return(match_insensitive_cc(a,b));}
FSTRING_LINK b32_4tech             match_insensitive(String a, char *b){return(match_insensitive_sc(a,b));}
FSTRING_INLINE b32_4tech           match_insensitive(char *a, String b){return(match_insensitive_cs(a,b));}
FSTRING_LINK b32_4tech             match_insensitive(String a, String b){return(match_insensitive_ss(a,b));}
FSTRING_LINK b32_4tech             match_part_insensitive(char *a, char *b, i32_4tech *len){return(match_part_insensitive_ccl(a,b,len));}
FSTRING_LINK b32_4tech             match_part_insensitive(String a, char *b, i32_4tech *len){return(match_part_insensitive_scl(a,b,len));}
FSTRING_INLINE b32_4tech           match_part_insensitive(char *a, char *b){return(match_part_insensitive_cc(a,b));}
FSTRING_INLINE b32_4tech           match_part_insensitive(String a, char *b){return(match_part_insensitive_sc(a,b));}
FSTRING_LINK b32_4tech             match_part_insensitive(char *a, String b){return(match_part_insensitive_cs(a,b));}
FSTRING_LINK b32_4tech             match_part_insensitive(String a, String b){return(match_part_insensitive_ss(a,b));}
FSTRING_LINK i32_4tech             compare(char *a, char *b){return(compare_cc(a,b));}
FSTRING_LINK i32_4tech             compare(String a, char *b){return(compare_sc(a,b));}
FSTRING_INLINE i32_4tech           compare(char *a, String b){return(compare_cs(a,b));}
FSTRING_LINK i32_4tech             compare(String a, String b){return(compare_ss(a,b));}
FSTRING_LINK i32_4tech             find(char *str, i32_4tech start, char character){return(find_c_char(str,start,character));}
FSTRING_LINK i32_4tech             find(String str, i32_4tech start, char character){return(find_s_char(str,start,character));}
FSTRING_LINK i32_4tech             rfind(String str, i32_4tech start, char character){return(rfind_s_char(str,start,character));}
FSTRING_LINK i32_4tech             find(char *str, i32_4tech start, char *characters){return(find_c_chars(str,start,characters));}
FSTRING_LINK i32_4tech             find(String str, i32_4tech start, char *characters){return(find_s_chars(str,start,characters));}
FSTRING_LINK i32_4tech             find_substr(char *str, i32_4tech start, String seek){return(find_substr_c(str,start,seek));}
FSTRING_LINK i32_4tech             find_substr(String str, i32_4tech start, String seek){return(find_substr_s(str,start,seek));}
FSTRING_LINK i32_4tech             rfind_substr(String str, i32_4tech start, String seek){return(rfind_substr_s(str,start,seek));}
FSTRING_LINK i32_4tech             find_substr_insensitive(char *str, i32_4tech start, String seek){return(find_substr_insensitive_c(str,start,seek));}
FSTRING_LINK i32_4tech             find_substr_insensitive(String str, i32_4tech start, String seek){return(find_substr_insensitive_s(str,start,seek));}
FSTRING_INLINE b32_4tech           has_substr(char *s, String seek){return(has_substr_c(s,seek));}
FSTRING_INLINE b32_4tech           has_substr(String s, String seek){return(has_substr_s(s,seek));}
FSTRING_INLINE b32_4tech           has_substr_insensitive(char *s, String seek){return(has_substr_insensitive_c(s,seek));}
FSTRING_INLINE b32_4tech           has_substr_insensitive(String s, String seek){return(has_substr_insensitive_s(s,seek));}
FSTRING_LINK i32_4tech             copy_fast_unsafe(char *dest, char *src){return(copy_fast_unsafe_cc(dest,src));}
FSTRING_LINK i32_4tech             copy_fast_unsafe(char *dest, String src){return(copy_fast_unsafe_cs(dest,src));}
FSTRING_LINK b32_4tech             copy_checked(String *dest, String src){return(copy_checked_ss(dest,src));}
FSTRING_LINK b32_4tech             copy_partial(String *dest, char *src){return(copy_partial_sc(dest,src));}
FSTRING_LINK b32_4tech             copy_partial(String *dest, String src){return(copy_partial_ss(dest,src));}
FSTRING_INLINE i32_4tech           copy(char *dest, char *src){return(copy_cc(dest,src));}
FSTRING_INLINE void                copy(String *dest, String src){return(copy_ss(dest,src));}
FSTRING_INLINE void                copy(String *dest, char *src){return(copy_sc(dest,src));}
FSTRING_LINK b32_4tech             append_checked(String *dest, String src){return(append_checked_ss(dest,src));}
FSTRING_LINK b32_4tech             append_partial(String *dest, char *src){return(append_partial_sc(dest,src));}
FSTRING_LINK b32_4tech             append_partial(String *dest, String src){return(append_partial_ss(dest,src));}
FSTRING_LINK b32_4tech             append(String *dest, char c){return(append_s_char(dest,c));}
FSTRING_INLINE b32_4tech           append(String *dest, String src){return(append_ss(dest,src));}
FSTRING_INLINE b32_4tech           append(String *dest, char *src){return(append_sc(dest,src));}
FSTRING_LINK void                  to_lower(char *src, char *dst){return(to_lower_cc(src,dst));}
FSTRING_LINK void                  to_lower(String *dst, String src){return(to_lower_ss(dst,src));}
FSTRING_LINK void                  to_lower(String *str){return(to_lower_s(str));}
FSTRING_LINK void                  to_upper(char *src, char *dst){return(to_upper_cc(src,dst));}
FSTRING_LINK void                  to_upper(String *dst, String src){return(to_upper_ss(dst,src));}
FSTRING_LINK void                  to_upper(String *str){return(to_upper_s(str));}
FSTRING_LINK void                  to_camel(char *src, char *dst){return(to_camel_cc(src,dst));}
FSTRING_LINK i32_4tech             str_is_int(char *str){return(str_is_int_c(str));}
FSTRING_LINK b32_4tech             str_is_int(String str){return(str_is_int_s(str));}
FSTRING_LINK i32_4tech             str_to_int(char *str){return(str_to_int_c(str));}
FSTRING_LINK i32_4tech             str_to_int(String str){return(str_to_int_s(str));}
FSTRING_LINK i32_4tech             reverse_seek_slash(String str, i32_4tech pos){return(reverse_seek_slash_pos(str,pos));}
FSTRING_LINK b32_4tech             set_last_folder(String *dir, char *folder_name, char slash){return(set_last_folder_sc(dir,folder_name,slash));}
FSTRING_LINK b32_4tech             set_last_folder(String *dir, String folder_name, char slash){return(set_last_folder_ss(dir,folder_name,slash));}
FSTRING_LINK b32_4tech             string_set_match(void *str_set, i32_4tech item_size, i32_4tech count, String str, i32_4tech *match_index){return(string_set_match_table(str_set,item_size,count,str,match_index));}

#endif


//
// Character Helpers
//

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_slash(char c)
{
    return (c == '\\' || c == '/');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_upper(char c)
{
    return (c >= 'A' && c <= 'Z');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_lower(char c)
{
    return (c >= 'a' && c <= 'z');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE char
char_to_upper(char c)
{
    return (c >= 'a' && c <= 'z') ? c + (char)('A' - 'a') : c;
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE char
char_to_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c - (char)('A' - 'a') : c;
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_whitespace(char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_alpha_numeric(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_alpha_numeric_true(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_alpha_true(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_hex(char c)
{
    return (c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f');
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
char_is_numeric(char c)
{
    return (c >= '0' && c <= '9');
}
#endif


//
// String Making Functions
//


#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
make_string_cap(void *str, i32_4tech size, i32_4tech mem_size){
    String result;
    result.str = (char*)str;
    result.size = size;
    result.memory_size = mem_size;
    return(result);
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
make_string(void *str, i32_4tech size){
    String result;
    result.str = (char*)str;
    result.size = size;
    result.memory_size = size;
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
str_size(char *str)
{
    i32_4tech i = 0;
    while (str[i]) ++i;
    return(i);
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
make_string_slowly(void *str)
{
    String result;
    result.str = (char*)str;
    result.size = str_size((char*)str);
    result.memory_size = result.size+1;
    return(result);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
substr_tail(String str, i32_4tech start)
{
    String result;
    result.str = str.str + start;
    result.size = str.size - start;
    result.memory_size = 0;
    return(result);
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
substr(String str, i32_4tech start, i32_4tech size)
{
    String result;
    result.str = str.str + start;
    result.size = size;
    if (start + size > str.size){
        result.size = str.size - start;
    }
    result.memory_size = 0;
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
skip_whitespace(String str)
{
    String result = {0};
    i32_4tech i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr(str, i, str.size - i);
    return(result);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
skip_whitespace_measure(String str, i32_4tech *skip_length)
{
    String result = {0};
    i32_4tech i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr(str, i, str.size - i);
    *skip_length = i;
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
chop_whitespace(String str)
{
    String result = {0};
    i32_4tech i = str.size;
    for (; i > 0 && char_is_whitespace(str.str[i-1]); --i);
    result = substr(str, 0, i);
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
skip_chop_whitespace(String str)
{
    str = skip_whitespace(str);
    str = chop_whitespace(str);
    return(str);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
skip_chop_whitespace_measure(String str, i32_4tech *skip_length)
{
    str = skip_whitespace_measure(str, skip_length);
    str = chop_whitespace(str);
    return(str);
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
tailstr(String str)
{
    String result;
    result.str = str.str + str.size;
    result.memory_size = str.memory_size - str.size;
    result.size = 0;
    return(result);
}
#endif


//
// String Comparison
//


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_cc(char *a, char *b){
    for (i32_4tech i = 0;; ++i){
        if (a[i] != b[i]){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_sc(String a, char *b){
    i32_4tech i = 0;
    for (; i < a.size; ++i){
        if (a.str[i] != b[i]){
            return 0;
        }
    }
    if (b[i] != 0){
        return 0;
    }
    return 1;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
match_cs(char *a, String b){
    return(match_sc(b,a));
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_ss(String a, String b){
    if (a.size != b.size){
        return 0;
    }
    for (i32_4tech i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_ccl(char *a, char *b, i32_4tech *len){
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (a[i] != b[i]){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_scl(String a, char *b, i32_4tech *len){
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (a.str[i] != b[i] || i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
match_part_cc(char *a, char *b){
    i32_4tech x;
    return match_part_ccl(a,b,&x);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
match_part_sc(String a, char *b){
    i32_4tech x;
    return match_part_scl(a,b,&x);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_cs(char *a, String b){
    for (i32_4tech i = 0; i != b.size; ++i){
        if (a[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_ss(String a, String b){
    if (a.size < b.size){
        return 0;
    }
    for (i32_4tech i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_insensitive_cc(char *a, char *b){
    for (i32_4tech i = 0;; ++i){
        if (char_to_upper(a[i]) !=
            char_to_upper(b[i])){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_insensitive_sc(String a, char *b){
    i32_4tech i = 0;
    for (; i < a.size; ++i){
        if (char_to_upper(a.str[i]) !=
            char_to_upper(b[i])){
            return 0;
        }
    }
    if (b[i] != 0){
        return 0;
    }
    return 1;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
match_insensitive_cs(char *a, String b){
    return match_insensitive_sc(b,a);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_insensitive_ss(String a, String b){
    if (a.size != b.size){
        return 0;
    }
    for (i32_4tech i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) !=
            char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_insensitive_ccl(char *a, char *b, i32_4tech *len){
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b[i])){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_insensitive_scl(String a, char *b, i32_4tech *len){
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b[i]) ||
            i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
match_part_insensitive_cc(char *a, char *b){
    i32_4tech x;
    return match_part_insensitive_ccl(a,b,&x);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
match_part_insensitive_sc(String a, char *b){
    i32_4tech x;
    return match_part_insensitive_scl(a,b,&x);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_insensitive_cs(char *a, String b){
    for (i32_4tech i = 0; i != b.size; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b.str[i])){
            return(0);
        }
    }
    return(1);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
match_part_insensitive_ss(String a, String b){
    if (a.size < b.size){
        return(0);
    }
    for (i32_4tech i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b.str[i])){
            return(0);
        }
    }
    return(1);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
compare_cc(char *a, char *b){
    i32_4tech i = 0, r = 0;
    while (a[i] == b[i] && a[i] != 0){
        ++i;
    }
    r = (a[i] > b[i]) - (a[i] < b[i]);
    return(r);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
compare_sc(String a, char *b){
    i32_4tech i = 0, r = 0;
    while (i < a.size && a.str[i] == b[i]){
        ++i;
    }
    if (i < a.size){
        r = (a.str[i] > b[i]) - (a.str[i] < b[i]);
    }
    else{
        if (b[i] == 0){
            r = 0;
        }
        else{
            r = -1;
        }
    }
    return(r);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE i32_4tech
compare_cs(char *a, String b){
    i32_4tech r = -compare_sc(b,a);
    return(r);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
compare_ss(String a, String b){
    i32_4tech i = 0, r = 0;
    i32_4tech m = a.size;
    if (b.size < m){
        m = b.size;
    }
    while (i < m && a.str[i] == b.str[i]){
        ++i;
    }
    
    if (i < m){
        r = (a.str[i] > b.str[i]) - (b.str[i] > a.str[i]);
    }
    else{
        r = (a.size > b.size) - (b.size > a.size);
    }
    
    return(r);
}
#endif

//
// Finding Characters and Substrings
//


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_c_char(char *str, i32_4tech start, char character){
    i32_4tech i = start;
    while (str[i] != character && str[i] != 0) ++i;
    return(i);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_s_char(String str, i32_4tech start, char character){
    i32_4tech i = start;
    while (i < str.size && str.str[i] != character) ++i;
    return(i);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
rfind_s_char(String str, i32_4tech start, char character){
    i32_4tech i = start;
    while (i >= 0 && str.str[i] != character) --i;
    return(i);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_c_chars(char *str, i32_4tech start, char *characters){
    i32_4tech i = start, j;
    while (str[i] != 0){
        for (j = 0; characters[j]; ++j){
            if (str[i] == characters[j]){
                return(i);
            }
        }
        ++i;
    }
    return(i);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_s_chars(String str, i32_4tech start, char *characters){
    i32_4tech i = start, j;
    while (i < str.size){
        for (j = 0; characters[j]; ++j){
            if (str.str[i] == characters[j]){
                return(i);
            }
        }
        ++i;
    }
    return(i);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_substr_c(char *str, i32_4tech start, String seek){
    i32_4tech i, j, k;
    b32_4tech hit;
    
    if (seek.size == 0){
        i = str_size(str);
        return(i);
    }
    for (i = start; str[i]; ++i){
        if (str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                if (str[k] != seek.str[j]){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return(i);
            }
        }
    }
    return(i);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_substr_s(String str, i32_4tech start, String seek){
    i32_4tech stop_at, i, j, k;
    b32_4tech hit;
    
    if (seek.size == 0){
        return str.size;
    }
    stop_at = str.size - seek.size + 1;
    for (i = start; i < stop_at; ++i){
        if (str.str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                if (str.str[k] != seek.str[j]){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return(str.size);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
rfind_substr_s(String str, i32_4tech start, String seek){
    i32_4tech i, j, k;
    b32_4tech hit;
    
    if (seek.size == 0){
        return -1;
    }
    if (start + seek.size > str.size){
        start = str.size - seek.size;
    }
    for (i = start; i >= 0; --i){
        if (str.str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                if (str.str[k] != seek.str[j]){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return -1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_substr_insensitive_c(char *str, i32_4tech start, String seek){
    i32_4tech i, j, k;
    b32_4tech hit;
    char a_upper, b_upper;
    
    if (seek.size == 0){
        return str_size(str);
    }
    for (i = start; str[i]; ++i){
        if (str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                a_upper = char_to_upper(str[k]);
                b_upper = char_to_upper(seek.str[j]);
                if (a_upper != b_upper){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return i;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
find_substr_insensitive_s(String str, i32_4tech start, String seek){
    i32_4tech i, j, k;
    i32_4tech stop_at;
    b32_4tech hit;
    char a_upper, b_upper;
    
    if (seek.size == 0){
        return str.size;
    }
    stop_at = str.size - seek.size + 1;
    for (i = start; i < stop_at; ++i){
        if (str.str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                a_upper = char_to_upper(str.str[k]);
                b_upper = char_to_upper(seek.str[j]);
                if (a_upper != b_upper){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return str.size;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
has_substr_c(char *s, String seek){
    return (s[find_substr_c(s, 0, seek)] != 0);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
has_substr_s(String s, String seek){
    return (find_substr_s(s, 0, seek) < s.size);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
has_substr_insensitive_c(char *s, String seek){
    return (s[find_substr_insensitive_c(s, 0, seek)] != 0);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
has_substr_insensitive_s(String s, String seek){
    return (find_substr_insensitive_s(s, 0, seek) < s.size);
}
#endif

//
// String Copies and Appends
//


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
copy_fast_unsafe_cc(char *dest, char *src){
    char *start = dest;
    while (*src != 0){
        *dest = *src;
        ++dest;
        ++src;
    }
    return (i32_4tech)(dest - start);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
copy_fast_unsafe_cs(char *dest, String src){
    i32_4tech i = 0;
    while (i != src.size){
        dest[i] = src.str[i];
        ++i;
    }
    return(src.size);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
copy_checked_ss(String *dest, String src){
    char *dest_str;
    i32_4tech i;
    if (dest->memory_size < src.size){
        return 0;
    }
    dest_str = dest->str;
    for (i = 0; i < src.size; ++i){
        dest_str[i] = src.str[i];
    }
    dest->size = src.size;
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
copy_partial_sc(String *dest, char *src){
    i32_4tech i = 0;
    i32_4tech memory_size = dest->memory_size;
    char *dest_str = dest->str;
    while (src[i] != 0){
        if (i >= memory_size){
            return 0;
        }
        dest_str[i] = src[i];
        ++i;
    }
    dest->size = i;
    return 1;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
copy_partial_ss(String *dest, String src){
    char *dest_str = dest->str;
    i32_4tech memory_size = dest->memory_size;
    b32_4tech result = 0;
    if (memory_size >= src.size){
        result = 1;
        memory_size = src.size;
    }
    for (i32_4tech i = 0; i < memory_size; ++i){
        dest_str[i] = src.str[i];
    }
    dest->size = memory_size;
    return result;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE i32_4tech
copy_cc(char *dest, char *src){
    return copy_fast_unsafe_cc(dest, src);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE void
copy_ss(String *dest, String src){
    copy_checked_ss(dest, src);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE void
copy_sc(String *dest, char *src){
    copy_partial_sc(dest, src);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_checked_ss(String *dest, String src){
    String end;
    end = tailstr(*dest);
    b32_4tech result = copy_checked_ss(&end, src);
    // NOTE(allen): This depends on end.size still being 0 if
    // the check failed and no coppy occurred.
    dest->size += end.size;
    return result;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_partial_sc(String *dest, char *src){
    String end = tailstr(*dest);
    b32_4tech result = copy_partial_sc(&end, src);
    dest->size += end.size;
    return result;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_partial_ss(String *dest, String src){
    String end = tailstr(*dest);
    b32_4tech result = copy_partial_ss(&end, src);
    dest->size += end.size;
    return result;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_s_char(String *dest, char c){
    b32_4tech result = 0;
    if (dest->size < dest->memory_size){
        dest->str[dest->size++] = c;
        result = 1;
    }
    return result;
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
append_ss(String *dest, String src){
    return append_partial_ss(dest, src);
}
#endif


#if !defined(FSTRING_GUARD)
FSTRING_INLINE b32_4tech
append_sc(String *dest, char *src){
    return append_partial_sc(dest, src);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
terminate_with_null(String *str){
    b32_4tech result = 0;
    if (str->size < str->memory_size){
        str->str[str->size] = 0;
        result = 1;
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_padding(String *dest, char c, i32_4tech target_size){
    b32_4tech result = 1;
    i32_4tech offset = target_size - dest->size;
    i32_4tech r = 0;
    if (offset > 0){
        for (r = 0; r < offset; ++r){
            if (append_s_char(dest, c) == 0){
                result = 0;
                break;
            }
        }
    }
    return(result);
}
#endif


//
// Other Edits
//

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
replace_char(String *str, char replace, char with){
    char *s = str->str;
    i32_4tech i = 0;
    for (i = 0; i < str->size; ++i, ++s){
        if (*s == replace) *s = with;
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_lower_cc(char *src, char *dst){
    for (; *src != 0; ++src){
        *dst++ = char_to_lower(*src);
    }
    *dst++ = 0;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_lower_ss(String *dst, String src){
    i32_4tech i = 0;
    i32_4tech size = src.size;
    char *c = src.str;
    char *d = dst->str;
    
    if (dst->memory_size >= size){
        for (; i < size; ++i){
            *d++ = char_to_lower(*c++);
        }
        dst->size = size;
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_lower_s(String *str){
    i32_4tech i = 0;
    i32_4tech size = str->size;
    char *c = str->str;
    for (; i < size; ++c, ++i){
        *c = char_to_lower(*c);
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_upper_cc(char *src, char *dst){
    for (; *src != 0; ++src){
        *dst++ = char_to_upper(*src);
    }
    *dst++ = 0;
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_upper_ss(String *dst, String src){
    i32_4tech i = 0;
    i32_4tech size = src.size;
    char *c = src.str;
    char *d = dst->str;
    
    if (dst->memory_size >= size){
        for (; i < size; ++i){
            *d++ = char_to_upper(*c++);
        }
        dst->size = size;
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_upper_s(String *str){
    i32_4tech i = 0;
    i32_4tech size = str->size;
    char *c = str->str;
    for (; i < size; ++c, ++i){
        *c = char_to_upper(*c);
    }
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK void
to_camel_cc(char *src, char *dst){
    char *c, ch;
    i32_4tech is_first = 1;
    for (c = src; *c != 0; ++c){
        ch = *c;
        if (char_is_alpha_numeric_true(ch)){
            if (is_first){
                is_first = 0;
                ch = char_to_upper(ch);
            }
            else{
                ch = char_to_lower(ch);
            }
        }
        else{
            is_first = 1;
        }
        *dst++ = ch;
    }
    *dst = 0;
}
#endif


//
//  String <-> Number Conversions
//

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
int_to_str_size(i32_4tech x){
    i32_4tech size = 1;
    if (x < 0){
        size = 2;
    }
    x /= 10;
    while (x != 0){
        x /= 10;
        ++size;
    }
    return(size);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
int_to_str(String *dest, i32_4tech x){
    b32_4tech result = 1;
    char *str = dest->str;
    i32_4tech memory_size = dest->memory_size;
    i32_4tech size, i, j;
    b32_4tech negative;
    
    if (x == 0){
        str[0] = '0';
        dest->size = 1;
    }
    else{
        size = 0;
        negative = 0;
        if (x < 0){
            negative = 1;
            x = -x;
            str[size++] = '-';
        }
        while (x != 0){
            if (size == memory_size){
                result = 0;
                break;
            }
            i = x % 10;
            x /= 10;
            str[size++] = (char)('0' + i);
        }
        if (result){
            // NOTE(allen): Start i = 0 if not negative, start i = 1 if is negative because - should not be flipped if it is negative :)
            for (i = negative, j = size-1; i < j; ++i, --j){
                char temp = str[i];
                str[i] = str[j];
                str[j] = temp;
            }
            dest->size = size;
        }
        else{
            dest->size = 0;
        }
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_int_to_str(String *dest, i32_4tech x){
    String last_part = tailstr(*dest);
    b32_4tech result = int_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
u64_to_str_size(uint64_t x){
    i32_4tech size;
    if (x < 0){
        size = 0;
    }
    else{
        size = 1;
        x /= 10;
        while (x != 0){
            x /= 10;
            ++size;
        }
    }
    return(size);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
u64_to_str(String *dest, uint64_t x){
    b32_4tech result = 1;
    char *str = dest->str;
    i32_4tech memory_size = dest->memory_size;
    i32_4tech size, i, j;
    
    if (x == 0){
        str[0] = '0';
        dest->size = 1;
    }
    else{
        size = 0;
        while (x != 0){
            if (size == memory_size){
                result = 0;
                break;
            }
            i = x % 10;
            x /= 10;
            str[size++] = (char)('0' + i);
        }
        if (result){
            for (i = 0, j = size-1; i < j; ++i, --j){
                char temp = str[i];
                str[i] = str[j];
                str[j] = temp;
            }
            dest->size = size;
        }
        else{
            dest->size = 0;
        }
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_u64_to_str(String *dest, uint64_t x){
    String last_part = tailstr(*dest);
    b32_4tech result = u64_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}
#endif

#if !defined(FSTRING_GUARD)
typedef struct Float_To_Str_Variables{
    b32_4tech negative;
    i32_4tech int_part;
    i32_4tech dec_part;
} Float_To_Str_Variables;

static Float_To_Str_Variables
get_float_vars(float x){
    Float_To_Str_Variables vars = {0};
    
    if (x < 0){
        vars.negative = 1;
        x = -x;
    }
    
    vars.int_part = (i32_4tech)(x);
    vars.dec_part = (i32_4tech)((x - vars.int_part) * 1000);
    
    return(vars);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
float_to_str_size(float x){
    Float_To_Str_Variables vars = get_float_vars(x);
    i32_4tech size = vars.negative + int_to_str_size(vars.int_part) + 1 + int_to_str_size(vars.dec_part);
    return(size);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
append_float_to_str(String *dest, float x){
    b32_4tech result = 1;
    Float_To_Str_Variables vars = get_float_vars(x);
    
    if (vars.negative){
        append_s_char(dest, '-');
    }
    
    append_int_to_str(dest, vars.int_part);
    append_s_char(dest, '.');
    append_int_to_str(dest, vars.dec_part);
    
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
float_to_str(String *dest, float x){
    b32_4tech result = 1;
    dest->size = 0;
    append_float_to_str(dest, x);
    return(result);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
str_is_int_c(char *str){
    b32_4tech result = 1;
    for (; *str; ++str){
        if (!char_is_numeric(*str)){
            result = 0;
            break;
        }
    }
    return(result);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
str_is_int_s(String str){
    b32_4tech result = 1;
    for (i32_4tech i = 0; i < str.size; ++i){
        if (!char_is_numeric(str.str[i])){
            result = 0;
            break;
        }
    }
    return(result);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
str_to_int_c(char *str){
    i32_4tech x = 0;
    for (; *str; ++str){
        if (*str >= '0' && *str <= '9'){
            x *= 10;
            x += *str - '0';
        }
        else{
            x = 0;
            break;
        }
    }
    return(x);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
str_to_int_s(String str){
    i32_4tech x, i;
    if (str.size == 0){
        x = 0;
    }
    else{
        x = str.str[0] - '0';
        for (i = 1; i < str.size; ++i){
            x *= 10;
            x += str.str[i] - '0';
        }
    }
    return(x);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
hexchar_to_int(char c){
    i32_4tech x = 0;
    if (c >= '0' && c <= '9'){
        x = c-'0';
    }
    else if (c > 'F'){
        x = c+(10-'a');
    }
    else{
        x = c+(10-'A');
    }
    return(x);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK char
int_to_hexchar(i32_4tech x){
    return (x<10)?((char)x+'0'):((char)x+'a'-10);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK u32_4tech
hexstr_to_int(String str){
    u32_4tech x;
    i32_4tech i;
    if (str.size == 0){
        x = 0;
    }
    else{
        x = hexchar_to_int(str.str[0]);
        for (i = 1; i < str.size; ++i){
            x *= 0x10;
            x += hexchar_to_int(str.str[i]);
        }
    }
    return(x);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
color_to_hexstr(String *s, u32_4tech color){
    b32_4tech result = 0;
    i32_4tech i;
    
    if (s->memory_size == 7 || s->memory_size == 8){
        result = 1;
        s->size = 6;
        s->str[6] = 0;
        color = color & 0x00FFFFFF;
        for (i = 5; i >= 0; --i){
            s->str[i] = int_to_hexchar(color & 0xF);
            color >>= 4;
        }
    }
    else if (s->memory_size > 8){
        result = 1;
        s->size = 8;
        s->str[8] = 0;
        for (i = 7; i >= 0; --i){
            s->str[i] = int_to_hexchar(color & 0xF);
            color >>= 4;
        }
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
hexstr_to_color(String s, u32_4tech *out){
    b32_4tech result = 0;
    u32_4tech color = 0;
    if (s.size == 6){
        result = 1;
        color = (u32_4tech)hexstr_to_int(s);
        color |= (0xFF << 24);
        *out = color;
    }
    else if (s.size == 8){
        result = 1;
        color = (u32_4tech)hexstr_to_int(s);
        *out = color;
    }
    return(result);
}
#endif

//
// Directory String Management
//


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK i32_4tech
reverse_seek_slash_pos(String str, i32_4tech pos){
    i32_4tech i = str.size - 1 - pos;
    while (i >= 0 && !char_is_slash(str.str[i])){
        --i;
    }
    return i;
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE i32_4tech
reverse_seek_slash(String str){
    return(reverse_seek_slash_pos(str, 0));
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
front_of_directory(String dir){
    return substr_tail(dir, reverse_seek_slash(dir) + 1);
}
#endif

#if !defined(FSTRING_GUARD)
FSTRING_INLINE String
path_of_directory(String dir){
    return substr(dir, 0, reverse_seek_slash(dir) + 1);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
set_last_folder_sc(String *dir, char *folder_name, char slash){
    b32_4tech result = 0;
    i32_4tech size = reverse_seek_slash(*dir) + 1;
    dir->size = size;
    if (append_sc(dir, folder_name)){
        if (append_s_char(dir, slash)){
            result = 1;
        }
    }
    if (!result){
        dir->size = size;
    }
    return(result);
}
#endif


#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
set_last_folder_ss(String *dir, String folder_name, char slash){
    b32_4tech result = 0;
    i32_4tech size = reverse_seek_slash(*dir) + 1;
    dir->size = size;
    if (append_ss(dir, folder_name)){
        if (append_s_char(dir, slash)){
            result = 1;
        }
    }
    if (!result){
        dir->size = size;
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
file_extension(String str){
    i32_4tech i;
    for (i = str.size - 1; i >= 0; --i){
        if (str.str[i] == '.') break;
    }
    ++i;
    return(make_string(str.str+i, str.size-i));
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
remove_extension(String *str){
    b32_4tech result = 0;
    i32_4tech i;
    for (i = str->size - 1; i >= 0; --i){
        if (str->str[i] == '.') break;
    }
    if (i >= 0){
        result = 1;
        str->size = i + 1;
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
remove_last_folder(String *str){
    b32_4tech result = 0;
    i32_4tech end = reverse_seek_slash_pos(*str, 1);
    if (end >= 0){
        result = 1;
        str->size = end + 1;
    }
    return(result);
}
#endif

// TODO(allen): Add hash-table extension to string sets.

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
string_set_match_table(void *str_set, i32_4tech item_size, i32_4tech count, String str, i32_4tech *match_index){
    b32_4tech result = 0;
    i32_4tech i = 0;
    uint8_t *ptr = (uint8_t*)str_set;
    for (; i < count; ++i, ptr += item_size){
        if (match_ss(*(String*)ptr, str)){
            *match_index = i;
            result = 1;
            break;
        }
    }
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK b32_4tech
string_set_match(String *str_set, i32_4tech count, String str, i32_4tech *match_index){
    b32_4tech result = string_set_match_table(str_set, sizeof(String), count, str, match_index);
    return(result);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
get_first_double_line(String source){
    String line = {0};
    i32_4tech pos0 = find_substr_s(source, 0, make_lit_string("\n\n"));
    i32_4tech pos1 = find_substr_s(source, 0, make_lit_string("\r\n\r\n"));
    if (pos1 < pos0){
        pos0 = pos1;
    }
    line = substr(source, 0, pos0);
    return(line);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
get_next_double_line(String source, String line){
    String next = {0};
    i32_4tech pos = (i32_4tech)(line.str - source.str) + line.size;
    i32_4tech start = 0, pos0 = 0, pos1 = 0;
    
    if (pos < source.size){
        //Assert(source.str[pos] == '\n' || source.str[pos] == '\r');
        start = pos + 1;
        
        if (start < source.size){
            pos0 = find_substr_s(source, start, make_lit_string("\n\n"));
            pos1 = find_substr_s(source, start, make_lit_string("\r\n\r\n"));
            if (pos1 < pos0){
                pos0 = pos1;
            }
            next = substr(source, start, pos0 - start);
        }
    }
    
    return(next);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
get_next_word(String source, String prev_word){
    
    String word = {0};
    i32_4tech pos0 = (i32_4tech)(prev_word.str - source.str) + prev_word.size;
    i32_4tech pos1 = 0;
    char c = 0;
    
    for (; pos0 < source.size; ++pos0){
        c = source.str[pos0];
        if (!(char_is_whitespace(c) || c == '(' || c == ')')){
            break;
        }
    }
    
    if (pos0 < source.size){
        for (pos1 = pos0; pos1 < source.size; ++pos1){
            c = source.str[pos1];
            if (char_is_whitespace(c) || c == '(' || c == ')'){
                break;
            }
        }
        
        word = substr(source, pos0, pos1 - pos0);
    }
    
    return(word);
}
#endif

#if defined(FSTRING_IMPLEMENTATION)
FSTRING_LINK String
get_first_word(String source){
    String start_str = make_string(source.str, 0);
    String word = get_next_word(source, start_str);
    return(word);
}
#endif

// TODO(allen): eliminate this.
#ifndef FSTRING_EXPERIMENTAL
#define FSTRING_EXPERIMENTAL

// NOTE(allen): experimental section, things below here are
// not promoted to public API level yet.

typedef struct Absolutes{
    String a[8];
    i32_4tech count;
} Absolutes;

static void
get_absolutes(String name, Absolutes *absolutes, b32_4tech implicit_first, b32_4tech implicit_last){
    if (name.size != 0){
        i32_4tech count = 0;
        i32_4tech max = (sizeof(absolutes->a)/sizeof(*absolutes->a)) - 1;
        if (implicit_last) --max;
        
        String str;
        str.str = name.str;
        str.size = 0;
        str.memory_size = 0;
        b32_4tech prev_was_wild = 0;
        
        if (implicit_first){
            absolutes->a[count++] = str;
            prev_was_wild = 1;
        }
        
        i32_4tech i;
        for (i = 0; i < name.size; ++i){
            if (name.str[i] == '*' && count < max){
                if (!prev_was_wild){
                    str.memory_size = str.size;
                    absolutes->a[count++] = str;
                    str.size = 0;
                }
                str.str = name.str + i + 1;
                prev_was_wild = 1;
            }
            else{
                ++str.size;
                prev_was_wild = 0;
            }
        }
        
        str.memory_size = str.size;
        absolutes->a[count++] = str;
        
        if (implicit_last){
            str.size = 0;
            str.memory_size = 0;
            absolutes->a[count++] = str;
        }
        
        absolutes->count = count;
    }
    else{
        absolutes->count = 0;
    }
}

static b32_4tech
wildcard_match_c(Absolutes *absolutes, char *x, i32_4tech case_sensitive){
    b32_4tech r = 1;
    
    if (absolutes->count > 0){
        String *a = absolutes->a;
        
        b32_4tech (*match_func)(char*, String);
        b32_4tech (*match_part_func)(char*, String);
        
        if (case_sensitive){
            match_func = match_cs;
            match_part_func = match_part_cs;
        }
        else{
            match_func = match_insensitive_cs;
            match_part_func = match_part_insensitive_cs;
        }
        
        if (absolutes->count == 1){
            r = match_func(x, *a);
        }
        else{
            if (!match_part_func(x, *a)){
                r = 0;
            }
            else{
                String *max = a + absolutes->count - 1;
                x += a->size;
                ++a;
                while (a < max){
                    if (*x == 0){
                        r = 0;
                        break;
                    }
                    if (match_part_func(x, *a)){
                        x += a->size;
                        ++a;
                    }
                    else{
                        ++x;
                    }
                }
                if (r && a->size > 0){
                    r = 0;
                    while (*x != 0){
                        if (match_part_func(x, *a) && *(x + a->size) == 0){
                            r = 1;
                            break;
                        }
                        else{
                            ++x;
                        }
                    }
                }
            }
        }
    }
    return(r);
}

static b32_4tech
wildcard_match_s(Absolutes *absolutes, String x, i32_4tech case_sensitive){
    terminate_with_null(&x);
    return(wildcard_match_c(absolutes, x.str, case_sensitive));
}

#endif

#if defined(FSTRING_IMPLEMENTATION)
#undef FSTRING_IMPLEMENTATION
#define FSTRING_IMPL_GUARD
#endif

#if !defined(FSTRING_GUARD)
#define FSTRING_GUARD
#endif

// BOTTOM

