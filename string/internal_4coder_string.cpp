/*
internal_4coder_string.cpp - Base file for generating 4coder_string.h
*/

#define FSTRING_DECLS
#define FSTRING_BEGIN
#define API_EXPORT_MACRO

#ifndef API_EXPORT
# define API_EXPORT
#endif

#ifndef API_EXPORT_INLINE
# define API_EXPORT_INLINE
#endif

#define CPP_NAME(n)

FSTRING_BEGIN
// TOP

#include "4tech_standard_preamble.h"

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

FSTRING_DECLS

//
// Character Helpers
//

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_slash(char c)
/* DOC(This call returns non-zero if c is \ or /.) */{
    return (c == '\\' || c == '/');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_upper(char c)
/* DOC(If c is an uppercase letter this call returns true.) */{
    return (c >= 'A' && c <= 'Z');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_upper_utf8(char c)
/* DOC(If c is an uppercase letter this call returns true.) */{
    return ((c >= 'A' && c <= 'Z') || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_lower(char c)
/* DOC(If c is a lower letter this call returns true.) */{
    return (c >= 'a' && c <= 'z');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_lower_utf8(u8_4tech c)
/* DOC(If c is a lower letter this call returns true.) */{
    return ((c >= 'a' && c <= 'z') || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE char
char_to_upper(char c)
/* DOC(If c is a lowercase letter this call returns the uppercase equivalent, otherwise it returns c.) */{
    return (c >= 'a' && c <= 'z') ? c + (char)('A' - 'a') : c;
}

API_EXPORT_INLINE FSTRING_INLINE char
char_to_lower(char c)
/* DOC(If c is an uppercase letter this call returns the lowercase equivalent, otherwise it returns c.) */{
    return (c >= 'A' && c <= 'Z') ? c - (char)('A' - 'a') : c;
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_whitespace(char c)
/* DOC(This call returns non-zero if c is whitespace.) */{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_numeric(char c)
/* DOC(This call returns non-zero if c is any alphanumeric character including underscore.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_numeric_utf8(u8_4tech c)
/* DOC(This call returns non-zero if c is any alphanumeric character including underscore, or is a part of a UTF8 sequence outside of ASCII.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_numeric_true(char c)
/* DOC(This call returns non-zero if c is any alphanumeric character no including underscore.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_numeric_true_utf8(u8_4tech c)
/* DOC(This call returns non-zero if c is any alphanumeric character no including underscore.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha(char c)
/* DOC(This call returns non-zero if c is any alphabetic character including underscore.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_utf8(u8_4tech c)
/* DOC(This call returns non-zero if c is any alphabetic character including underscore, or is a part of a UTF8 sequence outside of ASCII.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_true(char c)
/* DOC(This call returns non-zero if c is any alphabetic character.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_alpha_true_utf8(u8_4tech c)
/* DOC(This call returns non-zero if c is any alphabetic character, or is a part of a UTF8 sequence outside of ASCII.) */{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_hex(char c)
/* DOC(This call returns non-zero if c is any valid hexadecimal digit.) */{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_hex_utf8(u8_4tech c)
/* DOC(This call returns non-zero if c is any valid hexadecimal digit, or is a part of a UTF8 sequence outside of ASCII.) */{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (unsigned char)c >= 128);
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_numeric(char c)
/* DOC(This call returns non-zero if c is any valid decimal digit.) */{
    return (c >= '0' && c <= '9');
}

API_EXPORT_INLINE FSTRING_INLINE b32_4tech
char_is_numeric_utf8(u8_4tech c)
/* DOC(This call returns non-zero if c is any valid decimal digit, or is a part of a UTF8 sequence outside of ASCII.) */{
    return ((c >= '0' && c <= '9') || (unsigned char)c >= 128);
}


//
// String Making Functions
//

CPP_NAME(make_string)
API_EXPORT_INLINE FSTRING_INLINE String
make_string_cap(void *str, i32_4tech size, i32_4tech mem_size)/*
DOC_PARAM(str, The str parameter provides the of memory with which the string shall operate.)
DOC_PARAM(size, The size parameter expresses the initial size of the string.
If the memory does not already contain a useful string this should be zero.)
DOC_PARAM(mem_size, The mem_size parameter expresses the full size of the memory provided by str.)
DOC(This call returns the String created from the parameters.)
*/{
    String result;
    result.str = (char*)str;
    result.size = size;
    result.memory_size = mem_size;
    return(result);
}

API_EXPORT_INLINE FSTRING_INLINE String
make_string(void *str, i32_4tech size)/*
DOC_PARAM(str, The str parameter provides the of memory with which the string shall operate.)
DOC_PARAM(size, The size parameter expresses the initial size of the string.
If the memory does not already contain a useful string this should be zero. Since this version
does not specify the size of the memory it is also assumed that this size is the maximum size
of the memory.)
DOC(This call returns the String created from the parameters.)
*/{
    String result;
    result.str = (char*)str;
    result.size = size;
    result.memory_size = size;
    return(result);
}

API_EXPORT_MACRO 
/* DOC(This macro takes a literal string in quotes and uses it to create a String with the correct size and memory size.  Strings created this way should usually not be mutated.) */
#define make_lit_string(s) (make_string_cap((char*)(s), sizeof(s)-1, sizeof(s)))

API_EXPORT_MACRO 
/* DOC(Rename for make_lit_string.) DOC_SEE(make_lit_string) */
#define lit(s) make_lit_string(s)

API_EXPORT_MACRO
/* DOC(This macro takes a local char array with a fixed width and uses it to create an empty String with the correct size and memory size to operate on the array.) */
#define make_fixed_width_string(s) (make_string_cap((char*)(s), 0, sizeof(s)))

API_EXPORT_MACRO
/* DOC(This macro is a helper for any calls that take a char*,integer pair to specify a string. This macro expands to both of those parameters from one String struct.) */
#define expand_str(s) ((s).str), ((s).size)

API_EXPORT FSTRING_LINK i32_4tech
str_size(char *str)
/* DOC(This call returns the number of bytes before a null terminator starting at str.) */{
    i32_4tech i = 0;
    if (str != 0){
        for (;str[i];++i);
    }
    return(i);
}

API_EXPORT_INLINE FSTRING_INLINE String
make_string_slowly(void *str)
/* DOC(This call makes a string by counting the number of bytes before a null terminator and treating that as the size and memory size of the string.) */{
    String result;
    result.str = (char*)str;
    result.size = str_size((char*)str);
    result.memory_size = result.size + 1;
    return(result);
}

CPP_NAME(substr)
API_EXPORT_INLINE FSTRING_INLINE String
substr_tail(String str, i32_4tech start)
/* DOC(This call creates a substring of str that starts with an offset from str's base.
The new string uses the same underlying memory so both strings will see changes.
Usually strings created this way should only go through immutable calls.) */{
    String result;
    result.str = str.str + start;
    result.size = str.size - start;
    result.memory_size = 0;
    return(result);
}

API_EXPORT_INLINE FSTRING_INLINE String
substr(String str, i32_4tech start, i32_4tech size)
/* DOC(This call creates a substring of str that starts with an offset from str's base,
and has a fixed size. The new string uses the same underlying memory so both strings
will see changes. Usually strings created this way should only go through immutable calls.) */{
    String result;
    result.str = str.str + start;
    result.size = size;
    if (start + size > str.size){
        result.size = str.size - start;
    }
    result.memory_size = 0;
    return(result);
}

API_EXPORT FSTRING_LINK String
skip_whitespace(String str)
/* DOC(This call creates a substring that starts with the first non-whitespace character of str.
Like other substr calls, the new string uses the underlying memory and so should usually be
considered immutable.) DOC_SEE(substr) */{
    String result = {0};
    i32_4tech i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr(str, i, str.size - i);
    return(result);
}

CPP_NAME(skip_whitespace)
API_EXPORT FSTRING_LINK String
skip_whitespace_measure(String str, i32_4tech *skip_length)
/* DOC(This call creates a substring that starts with the first non-whitespace character of str.
Like other substr calls, the new string uses the underlying memory and so should usually be
considered immutable.) DOC_SEE(substr) */{
    String result = {0};
    i32_4tech i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr(str, i, str.size - i);
    *skip_length = i;
    return(result);
}

API_EXPORT FSTRING_LINK String
chop_whitespace(String str)
/* DOC(This call creates a substring that ends with the last non-whitespace character of str.
Like other substr calls, the new string uses the underlying memory and so should usually be
considered immutable.) DOC_SEE(substr) */{
    String result = {0};
    i32_4tech i = str.size;
    for (; i > 0 && char_is_whitespace(str.str[i-1]); --i);
    result = substr(str, 0, i);
    return(result);
}

API_EXPORT FSTRING_LINK String
skip_chop_whitespace(String str)
/* DOC(This call is equivalent to calling skip_whitespace and chop_whitespace together.)
DOC_SEE(skip_whitespace) DOC_SEE(chop_whitespace)*/{
    str = skip_whitespace(str);
    str = chop_whitespace(str);
    return(str);
}

CPP_NAME(skip_chop_whitespace)
API_EXPORT FSTRING_LINK String
skip_chop_whitespace_measure(String str, i32_4tech *skip_length)
/* DOC(This call is equivalent to calling skip_whitespace and chop_whitespace together.)
DOC_SEE(skip_whitespace) DOC_SEE(chop_whitespace)*/{
    str = skip_whitespace_measure(str, skip_length);
    str = chop_whitespace(str);
    return(str);
}

API_EXPORT_INLINE FSTRING_INLINE String
tailstr(String str)
/* DOC(This call returns an empty String with underlying memory taken from
the portion of str's memory that is not used.) */{
    String result;
    result.str = str.str + str.size;
    result.memory_size = str.memory_size - str.size;
    result.size = 0;
    return(result);
}


//
// String Comparison
//

CPP_NAME(match)
API_EXPORT FSTRING_LINK b32_4tech
match_cc(char *a, char *b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
    for (i32_4tech i = 0;; ++i){
        if (a[i] != b[i]){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}

CPP_NAME(match)
API_EXPORT FSTRING_LINK b32_4tech
match_sc(String a, char *b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
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

CPP_NAME(match)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
match_cs(char *a, String b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
    return(match_sc(b,a));
}

CPP_NAME(match)
API_EXPORT FSTRING_LINK b32_4tech
match_ss(String a, String b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
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

CPP_NAME(match_part)
API_EXPORT FSTRING_LINK b32_4tech
match_part_ccl(char *a, char *b, i32_4tech *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    if (a == 0){
        a = "";
    }
    if (b == 0){
        b = "";
    }
    
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (a[i] != b[i]){
            return(0);
        }
    }
    *len = i;
    return(1);
}

CPP_NAME(match_part)
API_EXPORT FSTRING_LINK b32_4tech
match_part_scl(String a, char *b, i32_4tech *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    if (b == 0){
        b = "";
    }
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (i == a.size || a.str[i] != b[i]){
            return(0);
        }
    }
    *len = i;
    return(1);
}

CPP_NAME(match_part)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
match_part_cc(char *a, char *b)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    i32_4tech x;
    return match_part_ccl(a,b,&x);
}

CPP_NAME(match_part)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
match_part_sc(String a, char *b)/*
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    i32_4tech x;
    return match_part_scl(a,b,&x);
}

CPP_NAME(match_part)
API_EXPORT FSTRING_LINK b32_4tech
match_part_cs(char *a, String b)/*
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    for (i32_4tech i = 0; i != b.size; ++i){
        if (a[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

CPP_NAME(match_part)
API_EXPORT FSTRING_LINK b32_4tech
match_part_ss(String a, String b)/*
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    if (a.size < b.size){
        return(0);
    }
    for (i32_4tech i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return(0);
        }
    }
    return(1);
}

CPP_NAME(match_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_insensitive_cc(char *a, char *b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
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

CPP_NAME(match_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_insensitive_sc(String a, char *b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
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

CPP_NAME(match_insensitive)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
match_insensitive_cs(char *a, String b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
    return match_insensitive_sc(b,a);
}

CPP_NAME(match_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_insensitive_ss(String a, String b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
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

CPP_NAME(match_part_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_part_insensitive_ccl(char *a, char *b, i32_4tech *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    i32_4tech i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b[i])){
            return 0;
        }
    }
    *len = i;
    return 1;
}

CPP_NAME(match_part_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_part_insensitive_scl(String a, char *b, i32_4tech *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
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

CPP_NAME(match_part_insensitive)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
match_part_insensitive_cc(char *a, char *b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    i32_4tech x;
    return match_part_insensitive_ccl(a,b,&x);
}

CPP_NAME(match_part_insensitive)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
match_part_insensitive_sc(String a, char *b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    i32_4tech x;
    return match_part_insensitive_scl(a,b,&x);
}

CPP_NAME(match_part_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_part_insensitive_cs(char *a, String b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    for (i32_4tech i = 0; i != b.size; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b.str[i])){
            return(0);
        }
    }
    return(1);
}

CPP_NAME(match_part_insensitive)
API_EXPORT FSTRING_LINK b32_4tech
match_part_insensitive_ss(String a, String b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
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

CPP_NAME(compare)
API_EXPORT FSTRING_LINK i32_4tech
compare_cc(char *a, char *b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
    i32_4tech i = 0, r = 0;
    while (a[i] == b[i] && a[i] != 0){
        ++i;
    }
    r = (a[i] > b[i]) - (a[i] < b[i]);
    return(r);
}

CPP_NAME(compare)
API_EXPORT FSTRING_LINK i32_4tech
compare_sc(String a, char *b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
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

CPP_NAME(compare)
API_EXPORT_INLINE FSTRING_INLINE i32_4tech
compare_cs(char *a, String b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
    i32_4tech r = -compare_sc(b,a);
    return(r);
}

CPP_NAME(compare)
API_EXPORT FSTRING_LINK i32_4tech
compare_ss(String a, String b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
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

//
// Finding Characters and Substrings
//

CPP_NAME(find)
API_EXPORT FSTRING_LINK i32_4tech
find_c_char(char *str, i32_4tech start, char character)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The character parameter provides the character for which to search.)
DOC(This call returns the index of the first occurance of character, or the size of the string
if the character is not found.) */{
    i32_4tech i = start;
    while (str[i] != character && str[i] != 0) ++i;
    return(i);
}

CPP_NAME(find)
API_EXPORT FSTRING_LINK i32_4tech
find_s_char(String str, i32_4tech start, char character)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The character parameter provides the character for which to search.)
DOC(This call returns the index of the first occurance of character, or the size of the string
if the character is not found.) */{
    i32_4tech i = start;
    while (i < str.size && str.str[i] != character) ++i;
    return(i);
}

CPP_NAME(rfind)
API_EXPORT FSTRING_LINK i32_4tech
rfind_s_char(String str, i32_4tech start, char character)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The character parameter provides the character for which to search.)
DOC(This call looks for the largest index less than or equal to the start position where
the given character occurs.  If the index is found it is returned otherwise -1 is returned.) */{
    i32_4tech i = start;
    while (i >= 0 && str.str[i] != character) --i;
    return(i);
}

CPP_NAME(find)
API_EXPORT FSTRING_LINK i32_4tech
find_c_chars(char *str, i32_4tech start, char *characters)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The characters parameter provides a null terminated array of characters for which to search.)
DOC(This call returns the index of the first occurance of a character in the characters array,
or the size of the string if no such character is not found.) */{
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

CPP_NAME(find)
API_EXPORT FSTRING_LINK i32_4tech
find_s_chars(String str, i32_4tech start, char *characters)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The characters parameter provides a null terminated array of characters for which to search.)
DOC(This call returns the index of the first occurance of a character in the characters array,
or the size of the string if no such character is not found.) */{
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

CPP_NAME(find_substr)
API_EXPORT FSTRING_LINK i32_4tech
find_substr_c(char *str, i32_4tech start, String seek)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call returns the index of the first occurance of the seek substring in str or the
size of str if no such substring in str is found.) */{
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

CPP_NAME(find_substr)
API_EXPORT FSTRING_LINK i32_4tech
find_substr_s(String str, i32_4tech start, String seek)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call returns the index of the first occurance of the seek substring in str or the
size of str if no such substring in str is found.) */{
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

CPP_NAME(rfind_substr)
API_EXPORT FSTRING_LINK i32_4tech
rfind_substr_s(String str, i32_4tech start, String seek)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call returns the index of the last occurance of the seek substring in str
or -1 if no such substring in str is found.) */{
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

CPP_NAME(find_substr_insensitive)
API_EXPORT FSTRING_LINK i32_4tech
find_substr_insensitive_c(char *str, i32_4tech start, String seek)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call acts as find_substr under case insensitive comparison.)
DOC_SEE(find_substr)*/{
    i32_4tech i, j, k;
    b32_4tech hit;
    char a_upper, b_upper;
    char first_test_char;
    
    if (seek.size == 0){
        return str_size(str);
    }
    first_test_char = char_to_upper(seek.str[0]);
    for (i = start; str[i]; ++i){
        a_upper = char_to_upper(str[i]);
        if (a_upper == first_test_char){
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

CPP_NAME(find_substr_insensitive)
API_EXPORT FSTRING_LINK i32_4tech
find_substr_insensitive_s(String str, i32_4tech start, String seek)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call acts as find_substr under case insensitive comparison.)
DOC_SEE(find_substr)*/{
    i32_4tech i, j, k;
    i32_4tech stop_at;
    b32_4tech hit;
    char a_upper, b_upper;
    char first_test_char;
    
    if (seek.size == 0){
        return str.size;
    }
    stop_at = str.size - seek.size + 1;
    first_test_char = char_to_upper(seek.str[0]);
    for (i = start; i < stop_at; ++i){
        a_upper = char_to_upper(str.str[i]);
        if (a_upper == first_test_char){
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

CPP_NAME(has_substr)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
has_substr_c(char *s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek.) */{
    return (s[find_substr_c(s, 0, seek)] != 0);
}

CPP_NAME(has_substr)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
has_substr_s(String s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek.) */{
    return (find_substr_s(s, 0, seek) < s.size);
}

CPP_NAME(has_substr_insensitive)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
has_substr_insensitive_c(char *s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek
under case insensitive comparison.) */{
    return (s[find_substr_insensitive_c(s, 0, seek)] != 0);
}

CPP_NAME(has_substr_insensitive)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
has_substr_insensitive_s(String s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek
under case insensitive comparison.) */{
    return (find_substr_insensitive_s(s, 0, seek) < s.size);
}

//
// String Copies and Appends
//

CPP_NAME(copy_fast_unsafe)
API_EXPORT FSTRING_LINK i32_4tech
copy_fast_unsafe_cc(char *dest, char *src)/*
DOC(This call performs a copy from the src buffer to the dest buffer.
The copy does not stop until a null terminator is found in src.  There
is no safety against overrun so dest must be large enough to contain src.
The null terminator is not written to dest. This call returns the number
of bytes coppied to dest.) */{
    char *start = dest;
    while (*src != 0){
        *dest = *src;
        ++dest;
        ++src;
    }
    return (i32_4tech)(dest - start);
}

CPP_NAME(copy_fast_unsafe)
API_EXPORT FSTRING_LINK i32_4tech
copy_fast_unsafe_cs(char *dest, String src)/*
DOC(This call performs a copy from the src string to the dest buffer.
The copy does not stop until src.size characters are coppied.  There
is no safety against overrun so dest must be large enough to contain src.
The null terminator is not written to dest. This call returns the number
of bytes coppied to dest.) */{
    i32_4tech i = 0;
    while (i != src.size){
        dest[i] = src.str[i];
        ++i;
    }
    return(src.size);
}

CPP_NAME(copy_checked)
API_EXPORT FSTRING_LINK b32_4tech
copy_checked_ss(String *dest, String src)/*
DOC(This call performs a copy from the src string to the dest string.
The memory_size of dest is checked before any coppying is done.
This call returns non-zero on a successful copy.) */{
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

CPP_NAME(copy_checked)
API_EXPORT FSTRING_LINK b32_4tech
copy_checked_cs(char *dest, i32_4tech dest_cap, String src)/*
DOC(This call performs a copy from the src string to the dest string.
The value dest_cap is checked before any coppying is done.
This call returns non-zero on a successful copy.)
*/{
    i32_4tech i;
    if (dest_cap < src.size){
        return 0;
    }
    for (i = 0; i < src.size; ++i){
        dest[i] = src.str[i];
    }
    return 1;
}

CPP_NAME(copy_partial)
API_EXPORT FSTRING_LINK b32_4tech
copy_partial_sc(String *dest, char *src)/*
DOC(This call performs a copy from the src buffer to the dest string.
The memory_size of dest is checked if the entire copy cannot be performed,
as many bytes as possible are coppied to dest. This call returns non-zero
if the entire string is coppied to dest.) */{
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

CPP_NAME(copy_partial)
API_EXPORT FSTRING_LINK b32_4tech
copy_partial_ss(String *dest, String src)/*
DOC(This call performs a copy from the src string to the dest string.
The memory_size of dest is checked. If the entire copy cannot be performed,
as many bytes as possible are coppied to dest.
This call returns non-zero if the entire string is coppied to dest.) */{
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
    return(result);
}

CPP_NAME(copy_partial)
API_EXPORT FSTRING_LINK b32_4tech
copy_partial_cs(char *dest, i32_4tech dest_cap, String src)/*
DOC(This call performs a copy from the src string to the dest string.
The value dest_cap is checked.  If the entire copy cannot be performed,
as many bytes as possible are coppied to dest.
This call returns non-zero if the entire string is coppied to dest.)
*/{
    b32_4tech result = 0;
    i32_4tech copy_size = dest_cap;
    i32_4tech i;
    if (dest_cap >= src.size){
        result = 1;
        copy_size = src.size;
    }
    for (i = 0; i < copy_size; ++i){
        dest[i] = src.str[i];
    }
    return(result);
}

CPP_NAME(copy)
API_EXPORT_INLINE FSTRING_INLINE i32_4tech
copy_cc(char *dest, char *src)/*
DOC(This call performs a copy from src to dest equivalent to copy_fast_unsafe.)
DOC_SEE(copy_fast_unsafe) */{
    return copy_fast_unsafe_cc(dest, src);
}

CPP_NAME(copy)
API_EXPORT_INLINE FSTRING_INLINE void
copy_ss(String *dest, String src)/*
DOC(This call performs a copy from src to dest equivalent to copy_checked.)
DOC_SEE(copy_checked) */{
    copy_checked_ss(dest, src);
}

CPP_NAME(copy)
API_EXPORT_INLINE FSTRING_INLINE void
copy_sc(String *dest, char *src)/*
DOC(This call performs a copy from src to dest equivalent to copy_partial.)
DOC_SEE(copy_partial) */{
    copy_partial_sc(dest, src);
}

CPP_NAME(append_checked)
API_EXPORT FSTRING_LINK b32_4tech
append_checked_ss(String *dest, String src)/*
DOC(This call checks if there is enough space in dest's underlying memory
to append src onto dest. If there is src is appended and the call returns non-zero.) */{
    String end;
    end = tailstr(*dest);
    b32_4tech result = copy_checked_ss(&end, src);
    // NOTE(allen): This depends on end.size still being 0 if
    // the check failed and no coppy occurred.
    dest->size += end.size;
    return result;
}

CPP_NAME(append_partial)
API_EXPORT FSTRING_LINK b32_4tech
append_partial_sc(String *dest, char *src)/*
DOC(This call attemps to append as much of src into the space in dest's underlying memory
as possible.  If the entire string is appended the call returns non-zero.) */{
    String end = tailstr(*dest);
    b32_4tech result = copy_partial_sc(&end, src);
    dest->size += end.size;
    return result;
}

CPP_NAME(append_partial)
API_EXPORT FSTRING_LINK b32_4tech
append_partial_ss(String *dest, String src)/*
DOC(This call attemps to append as much of src into the space in dest's underlying memory
as possible.  If the entire string is appended the call returns non-zero.) */{
    String end = tailstr(*dest);
    b32_4tech result = copy_partial_ss(&end, src);
    dest->size += end.size;
    return result;
}

CPP_NAME(append)
API_EXPORT FSTRING_LINK b32_4tech
append_s_char(String *dest, char c)/*
DOC(This call attemps to append c onto dest.  If there is space left in dest's underlying
memory the character is appended and the call returns non-zero.) */{
    b32_4tech result = 0;
    if (dest->size < dest->memory_size){
        dest->str[dest->size++] = c;
        result = 1;
    }
    return result;
}

CPP_NAME(append)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
append_ss(String *dest, String src)/*
DOC(This call is equivalent to append_partial.) DOC_SEE(append_partial) */{
    return append_partial_ss(dest, src);
}

CPP_NAME(append)
API_EXPORT_INLINE FSTRING_INLINE b32_4tech
append_sc(String *dest, char *src)/*
DOC(This call is equivalent to append_partial.) DOC_SEE(append_partial) */{
    return append_partial_sc(dest, src);
}

API_EXPORT FSTRING_LINK b32_4tech
terminate_with_null(String *str)/*
DOC(This call attemps to append a null terminator onto str without effecting the
size of str. This is usually called when the time comes to pass the the string to an
API that requires a null terminator.  This call returns non-zero if there was a spare
byte in the strings underlying memory.) */{
    b32_4tech result = 0;
    if (str->size < str->memory_size){
        str->str[str->size] = 0;
        result = 1;
    }
    return(result);
}

API_EXPORT FSTRING_LINK b32_4tech
append_padding(String *dest, char c, i32_4tech target_size)/*
DOC(This call pads out dest so that it has a size of target_size by appending
the padding character c until the target size is achieved.  This call returns
non-zero if dest does not run out of space in the underlying memory.) */{
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


//
// Other Edits
//

API_EXPORT FSTRING_LINK void
string_interpret_escapes(String src, char *dst){
    i32_4tech mode = 0;
    i32_4tech j = 0;
    for (i32_4tech i = 0; i < src.size; ++i){
        switch (mode){
            case 0:
            {
                if (src.str[i] == '\\'){
                    mode = 1;
                }
                else{
                    dst[j++] = src.str[i];
                }
            }break;
            
            case 1:
            {
                char c = src.str[i];
                switch (c){
                    case '\\':{dst[j++] = '\\';} break;
                    case 'n': {dst[j++] = '\n';} break;
                    case 't': {dst[j++] = '\t';} break;
                    case '"': {dst[j++] = '"'; } break;
                    case '0': {dst[j++] = '\0';} break;
                    default: {dst[j++] = '\\'; dst[j++] = c;}break;
                }
                mode = 0;
            }break;
        }
    }
    dst[j] = 0;
}

API_EXPORT FSTRING_LINK void
replace_char(String *str, char replace, char with)/*
DOC_PARAM(str, The str parameter provides the string in which replacement shall be performed.)
DOC_PARAM(replace, The replace character specifies which character should be replaced.)
DOC_PARAM(with, The with character specifies what to write into the positions where replacement occurs.)
DOC(This call replaces all occurances of character in str with another character.) */{
    char *s = str->str;
    i32_4tech i = 0;
    for (i = 0; i < str->size; ++i, ++s){
        if (*s == replace) *s = with;
    }
}

#if !defined(FSTRING_GUARD)
void
block_move(void *a_ptr, void *b_ptr, i32_4tech s){
    u8_4tech *a = (u8_4tech*)a_ptr;
    u8_4tech *b = (u8_4tech*)b_ptr;
    if (a < b){
        for (i32_4tech i = 0; i < s; ++i, ++a, ++b){
            *a = *b;
        }
    }
    else if (a > b){
        a = a + s - 1;
        b = b + s - 1;
        for (i32_4tech i = 0; i < s; ++i, --a, --b){
            *a = *b;
        }
    }
}

void
replace_range_str(String *str, i32_4tech first, i32_4tech one_past_last, String with){
    i32_4tech shift = with.size - (one_past_last - first);
    i32_4tech new_size = str->size + shift;
    if (new_size <= str->memory_size){
        if (shift != 0){
            char *tail = str->str + one_past_last;
            char *new_tail_pos = tail + shift;
            block_move(new_tail_pos, tail, str->size - one_past_last);
        }
        block_move(str->str + first, with.str, with.size);
        str->size += shift;
    }
}
#endif

CPP_NAME(replace_str)
API_EXPORT FSTRING_LINK void
replace_str_ss(String *str, String replace, String with)/*
DOC_PARAM(str, The string to modify.)
DOC_PARAM(replace, A string matching the zero or more substring to be replaced within str.)
DOC_PARAM(with, The string to be placed into str in place of occurrences of replace.)
DOC(Modifies str so that every occurence of replace that was within str is gone and the string in with has taken their places.)
*/{
    i32_4tech i = 0;
    for (;;){
        i = find_substr_s(*str, i, replace);
        if (i >= str->size){
            break;
        }
        replace_range_str(str, i, i + replace.size, with);
        i += with.size;
    }
}

CPP_NAME(replace_str)
API_EXPORT FSTRING_LINK void
replace_str_sc(String *str, String replace, char *with)/*
DOC_PARAM(str, The string to modify.)
DOC_PARAM(replace, A string matching the zero or more substring to be replaced within str.  Must be null terminated, and will be counted, it is always faster to use a String parameter here when possible.)
DOC_PARAM(with, The string to be placed into str in place of occurrences of replace.)
DOC(Modifies str so that every occurence of replace that was within str is gone and the string in with has taken their places.)
*/{
    String w = make_string_slowly(with);
    replace_str_ss(str, replace, w);
}

CPP_NAME(replace_str)
API_EXPORT FSTRING_LINK void
replace_str_cs(String *str, char *replace, String with)/*
DOC_PARAM(str, The string to modify.)
DOC_PARAM(replace, A string matching the zero or more substring to be replaced within str.)
DOC_PARAM(with, The string to be placed into str in place of occurrences of replace. Must be null terminated, and will be counted, it is always faster to use a String parameter here when possible.)
DOC(Modifies str so that every occurence of replace that was within str is gone and the string in with has taken their places.)
*/{
    String r = make_string_slowly(replace);
    replace_str_ss(str, r, with);
}

CPP_NAME(replace_str)
API_EXPORT FSTRING_LINK void
replace_str_cc(String *str, char *replace, char *with)/*
DOC_PARAM(str, The string to modify.)
DOC_PARAM(replace, A string matching the zero or more substring to be replaced within str.  Must be null terminated, and will be counted, it is always faster to use a String parameter here when possible.)
DOC_PARAM(with, The string to be placed into str in place of occurrences of replace. Must be null terminated, and will be counted, it is always faster to use a String parameter here when possible.)
DOC(Modifies str so that every occurence of replace that was within str is gone and the string in with has taken their places.)
*/{
    String r = make_string_slowly(replace);
    String w = make_string_slowly(with);
    replace_str_ss(str, r, w);
}

CPP_NAME(to_lower)
API_EXPORT FSTRING_LINK void
to_lower_cc(char *src, char *dst)/*
DOC_PARAM(src, The source string to conver to lowercase.  This string must be null terminated.)
DOC_PARAM(dst, The destination buffer to receive the converted string.  This must be large
enough to contain all of src and a null terminator.)
DOC(Rewrites the string in src into dst with all letters lowercased. src and dst should not
overlap with the exception that src and dst may be exactly equal in order to convert the
string in place.)
*/{
    for (; *src != 0; ++src){
        *dst++ = char_to_lower(*src);
    }
    *dst++ = 0;
}

CPP_NAME(to_lower)
API_EXPORT FSTRING_LINK void
to_lower_ss(String *dst, String src)/*
DOC_PARAM(dst, The destination buffer to receive the converted string.
This must have a capacity of at least the size of src.)
DOC_PARAM(src, The source string to conver to lowercase.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap with the exception
that src and dst may be exactly equal in order to convert the string in place.)
*/{
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

CPP_NAME(to_lower)
API_EXPORT FSTRING_LINK void
to_lower_s(String *str)/*
DOC_PARAM(str, The string to be converted to all lowercase.)
DOC(This version of to_lower converts str to lowercase in place.)
*/{
    i32_4tech i = 0;
    i32_4tech size = str->size;
    char *c = str->str;
    for (; i < size; ++c, ++i){
        *c = char_to_lower(*c);
    }
}

CPP_NAME(to_upper)
API_EXPORT FSTRING_LINK void
to_upper_cc(char *src, char *dst)/*
DOC_PARAM(src, The source string to convert to uppercase.  This string must be null terminated.)
DOC_PARAM(dst, The destination buffer to receive the converted string. 
This must be large enough to contain all of src and a null terminator.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap with the exception
that src and dst may be exactly equal in order to convert the string in place.)
*/{
    for (; *src != 0; ++src){
        *dst++ = char_to_upper(*src);
    }
    *dst++ = 0;
}

CPP_NAME(to_upper)
API_EXPORT FSTRING_LINK void
to_upper_ss(String *dst, String src)/*
DOC_PARAM(dst, The destination buffer to receive the converted string.
This must have a capacity of at least the size of src.)
DOC_PARAM(src, The source string to convert to uppercase.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap with the exception
that src and dst may be exactly equal in order to convert the string in place.)
*/{
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

CPP_NAME(to_upper)
API_EXPORT FSTRING_LINK void
to_upper_s(String *str)/*
DOC_PARAM(str, The string to be converted to all uppercase.)
DOC(This version of to_upper converts str to uppercase in place.)
*/{
    i32_4tech i = 0;
    i32_4tech size = str->size;
    char *c = str->str;
    for (; i < size; ++c, ++i){
        *c = char_to_upper(*c);
    }
}

CPP_NAME(to_camel)
API_EXPORT FSTRING_LINK void
to_camel_cc(char *src, char *dst)/*
DOC_PARAM(src, The source string to convert to camel case.)
DOC_PARAM(dst, The destination buffer to receive the converted string.
This must be large enough to contain all of src and a null terminator.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap
with the exception that src and dst may be exactly equal in order to
convert the string in place.)
*/{
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


//
//  String <-> Number Conversions
//

API_EXPORT FSTRING_LINK i32_4tech
int_to_str_size(i32_4tech x)/*
DOC(This call returns the number of bytes required to represent x as a string.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
int_to_str(String *dest, i32_4tech x)/*
DOC(This call writes a string representation of x into dest. If there is enough
space in dest this call returns non-zero.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
append_int_to_str(String *dest, i32_4tech x)/*
DOC(This call appends a string representation of x onto dest. If there is enough
space in dest this call returns non-zero.) */{
    String last_part = tailstr(*dest);
    b32_4tech result = int_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}

API_EXPORT FSTRING_LINK i32_4tech
u64_to_str_size(uint64_t x)/*
DOC(This call returns the number of bytes required to represent x as a string.) */{
    i32_4tech size = 1;
    x /= 10;
    while (x != 0){
        x /= 10;
        ++size;
    }
    return(size);
}

API_EXPORT FSTRING_LINK b32_4tech
u64_to_str(String *dest, uint64_t x)/*
DOC(This call writes a string representation of x into dest. If there is enough
space in dest this call returns non-zero.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
append_u64_to_str(String *dest, uint64_t x)/*
DOC(This call appends a string representation of x onto dest. If there is enough
space in dest this call returns non-zero.) */{
    String last_part = tailstr(*dest);
    b32_4tech result = u64_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}

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

API_EXPORT FSTRING_LINK i32_4tech
float_to_str_size(float x)/*
DOC(This call returns the number of bytes required to represent x as a string.) */{
    Float_To_Str_Variables vars = get_float_vars(x);
    i32_4tech size = vars.negative + int_to_str_size(vars.int_part) + 1 + int_to_str_size(vars.dec_part);
    return(size);
}

API_EXPORT FSTRING_LINK b32_4tech
append_float_to_str(String *dest, float x)/*
DOC(This call writes a string representation of x into dest. If there is enough space in dest this call returns non-zero.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
float_to_str(String *dest, float x)/*
DOC(This call appends a string representation of x onto dest. If there is enough space in dest this call returns non-zero.) */{
    b32_4tech result = 1;
    dest->size = 0;
    append_float_to_str(dest, x);
    return(result);
}

CPP_NAME(str_is_int)
API_EXPORT FSTRING_LINK i32_4tech
str_is_int_c(char *str)/*
DOC(If str is a valid string representation of an integer, this call returns non-zero) */{
    b32_4tech result = 1;
    for (; *str; ++str){
        if (!char_is_numeric(*str)){
            result = 0;
            break;
        }
    }
    return(result);
}

CPP_NAME(str_is_int)
API_EXPORT FSTRING_LINK b32_4tech
str_is_int_s(String str)/*
DOC(If str is a valid string representation of an integer, this call returns non-zero.) */{
    b32_4tech result = 1;
    for (i32_4tech i = 0; i < str.size; ++i){
        if (!char_is_numeric(str.str[i])){
            result = 0;
            break;
        }
    }
    return(result);
}

CPP_NAME(str_to_int)
API_EXPORT FSTRING_LINK i32_4tech
str_to_int_c(char *str)/*
DOC(If str is a valid string representation of an integer, this call will return
the integer represented by the string.  Otherwise this call returns zero.) */{
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

CPP_NAME(str_to_int)
API_EXPORT FSTRING_LINK i32_4tech
str_to_int_s(String str)/*
DOC(If str represents a valid string representation of an integer, this call will return
the integer represented by the string.  Otherwise this call returns zero.) */{
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

API_EXPORT FSTRING_LINK i32_4tech
hexchar_to_int(char c)/*
DOC(If c is a valid hexadecimal digit [0-9a-fA-F] this call returns the value of
the integer value of the digit. Otherwise the return is some nonsense value.) */{
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

API_EXPORT FSTRING_LINK char
int_to_hexchar(i32_4tech x)/*
DOC(If x is in the range [0,15] this call returns the equivalent lowercase hexadecimal digit.
Otherwise the return is some nonsense value.) */{
    return (x<10)?((char)x+'0'):((char)x+'a'-10);
}

API_EXPORT FSTRING_LINK u32_4tech
hexstr_to_int(String str)/*
DOC(This call interprets str has a hexadecimal representation of an integer and returns
the represented integer value.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
color_to_hexstr(String *s, u32_4tech color)/*
DOC(This call fills s with the hexadecimal representation of the color.
If there is enough memory in s to represent the color this call returns non-zero.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
hexstr_to_color(String s, u32_4tech *out)/*
DOC(This call interprets s as a color and writes the 32-bit integer representation into out.) */{
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

//
// Directory String Management
//

CPP_NAME(reverse_seek_slash)
API_EXPORT FSTRING_LINK i32_4tech
reverse_seek_slash_pos(String str, i32_4tech pos)/*
DOC(This call searches for a slash in str by starting pos bytes from the end and going backwards.) */{
    i32_4tech i = str.size - 1 - pos;
    while (i >= 0 && !char_is_slash(str.str[i])){
        --i;
    }
    return i;
}

API_EXPORT_INLINE FSTRING_INLINE i32_4tech
reverse_seek_slash(String str)/*
DOC(This call searches for a slash in str by starting at the end and going backwards.) */{
    return(reverse_seek_slash_pos(str, 0));
}

API_EXPORT_INLINE FSTRING_INLINE String
front_of_directory(String dir)/*
DOC(This call returns a substring of dir containing only the file name or
folder name furthest to the right in the directory.) DOC_SEE(substr) */{
    return substr_tail(dir, reverse_seek_slash(dir) + 1);
}

API_EXPORT_INLINE FSTRING_INLINE String
path_of_directory(String dir)/*
DOC(This call returns a substring of dir containing the whole path except
for the final file or folder name.) DOC_SEE(substr) */{
    return substr(dir, 0, reverse_seek_slash(dir) + 1);
}

CPP_NAME(set_last_folder)
API_EXPORT FSTRING_LINK b32_4tech
set_last_folder_sc(String *dir, char *folder_name, char slash)/*
DOC_PARAM(dir, The dir parameter is the directory string in which to set the last folder in the directory.)
DOC_PARAM(folder_name, The folder_name parameter is a null terminated string specifying the name to set
at the end of the directory.)
DOC_PARAM(slash, The slash parameter specifies what slash to use between names in the directory.)
DOC(This call deletes the last file name or folder name in the dir string and appends the new provided one.
If there is enough memory in dir this call returns non-zero.) */{
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

CPP_NAME(set_last_folder)
API_EXPORT FSTRING_LINK b32_4tech
set_last_folder_ss(String *dir, String folder_name, char slash)/*
DOC_PARAM(dir, The dir parameter is the directory string in which to set the last folder in the directory.)
DOC_PARAM(folder_name, The folder_name parameter is a string specifying the name to set at the end of the directory.)
DOC_PARAM(slash, The slash parameter specifies what slash to use between names in the directory.)
DOC(This call deletes the last file name or folder name in the dir string and appends the new provided one.
If there is enough memory in dir this call returns non-zero.) */{
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

API_EXPORT FSTRING_LINK String
file_extension(String str)/*
DOC(This call returns a substring containing only the file extension of the provided filename.)
DOC_SEE(substr) */{
    i32_4tech i;
    for (i = str.size - 1; i >= 0; --i){
        if (str.str[i] == '.') break;
    }
    ++i;
    return(make_string(str.str+i, str.size-i));
}

API_EXPORT FSTRING_LINK b32_4tech
remove_extension(String *str)/*
DOC(This call attemps to delete a file extension off the end of a filename.
This call returns non-zero on success.) */{
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

API_EXPORT FSTRING_LINK b32_4tech
remove_last_folder(String *str)/*
DOC(This call attemps to delete a folder or filename off the end of a path string.
This call returns non-zero on success.) */{
    b32_4tech result = 0;
    i32_4tech end = reverse_seek_slash_pos(*str, 1);
    if (end >= 0){
        result = 1;
        str->size = end + 1;
    }
    return(result);
}

CPP_NAME(string_set_match)
API_EXPORT FSTRING_LINK b32_4tech
string_set_match_table(void *str_set, i32_4tech item_size, i32_4tech count, String str, i32_4tech *match_index)/*
DOC_PARAM(str_set, The str_set parameter may be an array of any type. It should point at the String in the first element of the array.)
DOC_PARAM(count, The item_size parameter should describe the "stride" from one String to the next, in other words it should be the size of one element of the array.)
DOC_PARAM(count, The count parameter specifies the number of elements in the str_set array.)
DOC_PARAM(str, The str parameter specifies the string to match against the str_set.)
DOC_PARAM(match_index, If this call succeeds match_index is filled with the index into str_set where the match occurred.)
DOC(This call tries to see if str matches any of the strings in str_set.  If there is a match the call succeeds and returns non-zero.  The matching rule is equivalent to the matching rule for match.)
DOC_SEE(match) */{
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

API_EXPORT FSTRING_LINK b32_4tech
string_set_match(String *str_set, i32_4tech count, String str, i32_4tech *match_index)/*
DOC_PARAM(str_set, The str_set parameter is an array of String structs specifying matchable strings.)
DOC_PARAM(count, The count parameter specifies the number of String structs in the str_set array.)
DOC_PARAM(str, The str parameter specifies the string to match against the str_set.)
DOC_PARAM(match_index, If this call succeeds match_index is filled with the index into str_set where the match occurred.)
DOC(This call tries to see if str matches any of the strings in str_set.  If there is a match the call succeeds and returns non-zero.  The matching rule is equivalent to the matching rule for match.)
DOC_SEE(match) */{
    b32_4tech result = string_set_match_table(str_set, sizeof(String), count, str, match_index);
    return(result);
}

API_EXPORT FSTRING_LINK String
get_first_double_line(String source)/*
DOC_PARAM(source, the source string accross which a 'double line' iteration will occur)
DOC_RETURN(The returned value is the first 'double line' in the source string.)
DOC(A 'double line' is a string of characters delimited by two new line characters.  This call begins an iteration over all the double lines in the given source string.)
DOC_SEE(get_next_double_line)
*/{
    String line = {0};
    i32_4tech pos0 = find_substr_s(source, 0, make_lit_string("\n\n"));
    i32_4tech pos1 = find_substr_s(source, 0, make_lit_string("\r\n\r\n"));
    if (pos1 < pos0){
        pos0 = pos1;
    }
    line = substr(source, 0, pos0);
    return(line);
}

API_EXPORT FSTRING_LINK String
get_next_double_line(String source, String line)/*
DOC_PARAM(source, the source string accross which the 'double line' iteration is occurring)
DOC_PARAM(line, the value returned from the previous call of get_first_double_line or get_next_double_line)
DOC_RETURN(The returned value is the first 'double line' in the source string.)
DOC_SEE(get_first_double_line)
*/{
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

API_EXPORT FSTRING_LINK String
get_next_word(String source, String prev_word)/*
DOC_PARAM(source, the source string accross which the 'word' iteration is occurring)
DOC_PARAM(line, the value returned from the previous call of get_first_word or get_next_word)
DOC_RETURN(The returned value is the first 'word' in the source string.)
DOC_SEE(get_first_word)
*/{
    
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

API_EXPORT FSTRING_LINK String
get_first_word(String source)/*
DOC_PARAM(source, the source string accross which a 'word' iteration will occur)
DOC_RETURN(The returned value is the first 'word' in the source string.)
DOC(A 'word' is a string of characters delimited by whitespace or parentheses.  This call begins an iteration over all the double lines in the given source string.)
DOC_SEE(get_next_word)
*/{
    String start_str = make_string(source.str, 0);
    String word = get_next_word(source, start_str);
    return(word);
}

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

