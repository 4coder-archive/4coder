
#define FSTRING_DECLS
#define FSTRING_BEGIN
#define DOC_EXPORT

#define CPP_NAME(n)

FSTRING_BEGIN
// TOP

#if defined(FSTRING_C)
# if defined(FSTRING_INLINE)
#  undef FSTRING_INLINE
# endif
# define FSTRING_INLINE static
#endif

#if defined(FSTRING_IMPLEMENTATION) && defined(FSTRING_GUARD)
# undef FSTRING_IMPLEMENTATION
#endif

#include <stdint.h>

#ifndef FSTRING_LINK
# define FSTRING_LINK static
#endif

#ifndef FSTRING_INLINE
# define FSTRING_INLINE inline
#endif

#ifndef FSTRING_STRUCT
#define FSTRING_STRUCT
typedef struct String{
    char *str;
    int32_t size;
    int32_t memory_size;
} String;

typedef struct Offset_String{
    int32_t offset;
    int32_t size;
} Offset_String;
#endif

#ifndef fstr_bool
# define fstr_bool int32_t
#endif

#ifndef literal
# define literal(s) (s), (sizeof(s)-1)
#endif

FSTRING_DECLS

//
// Character Helpers
//

FSTRING_INLINE fstr_bool
char_is_slash(char c)
/* DOC(This call returns non-zero if c is \ or /.) */{
    return (c == '\\' || c == '/');
}

FSTRING_INLINE char
char_to_upper(char c)
/* DOC(If c is a lowercase letter this call returns the uppercase equivalent, otherwise it returns c.) */{
    return (c >= 'a' && c <= 'z') ? c + (char)('A' - 'a') : c;
}

FSTRING_INLINE char
char_to_lower(char c)
/* DOC(If c is an uppercase letter this call returns the lowercase equivalent, otherwise it returns c.) */{
    return (c >= 'A' && c <= 'Z') ? c - (char)('A' - 'a') : c;
}

FSTRING_INLINE fstr_bool
char_is_whitespace(char c)
/* DOC(This call returns non-zero if c is whitespace.) */{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

FSTRING_INLINE fstr_bool
char_is_alpha_numeric(char c)
/* DOC(This call returns non-zero if c is any alphanumeric character including underscore.) */{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_');
}

FSTRING_INLINE fstr_bool
char_is_alpha_numeric_true(char c)
/* DOC(This call returns non-zero if c is any alphanumeric character no including underscore.) */{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9');
}

FSTRING_INLINE fstr_bool
char_is_alpha(char c)
/* DOC(This call returns non-zero if c is any alphabetic character including underscore.) */{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_');
}

FSTRING_INLINE fstr_bool
char_is_alpha_true(char c)
/* DOC(This call returns non-zero if c is any alphabetic character.) */{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z');
}

FSTRING_INLINE fstr_bool
char_is_hex(char c)
/* DOC(This call returns non-zero if c is any valid hexadecimal digit.) */{
    return (c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f');
}

FSTRING_INLINE fstr_bool
char_is_numeric(char c)
/* DOC(This call returns non-zero if c is any valid decimal digit.) */{
    return (c >= '0' && c <= '9');
}


//
// String Making Functions
//

FSTRING_INLINE String
string_zero(void)
/* DOC(This call returns a String struct of zeroed members.) */{
    String str={0};
    return(str);
}

CPP_NAME(make_string)
FSTRING_INLINE String
make_string_cap(void *str, int32_t size, int32_t mem_size)/*
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

FSTRING_INLINE String
make_string(void *str, int32_t size)/*
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

DOC_EXPORT /* DOC(This macro takes a literal string in quotes and uses it to create a String
with the correct size and memory size.  Strings created this way should usually not be mutated.) */
#define make_lit_string(s) (make_string_cap((char*)(s), sizeof(s)-1, sizeof(s)))

DOC_EXPORT /* DOC(This macro takes a local char array with a fixed width and uses it to create
an empty String with the correct size and memory size to operate on the array.) */
#define make_fixed_width_string(s) (make_string_cap((char*)(s), 0, sizeof(s)))

DOC_EXPORT /* DOC(This macro is a helper for any calls that take a char*,integer pair to specify a
string. This macro expands to both of those parameters from one String struct.) */
#define expand_str(s) ((s).str), ((s).size)

FSTRING_LINK int32_t
str_size(char *str)
/* DOC(This call returns the number of bytes before a null terminator starting at str.) */{
    int32_t i = 0;
    while (str[i]) ++i;
    return(i);
}

FSTRING_INLINE String
make_string_slowly(void *str)
/* DOC(This call makes a string by counting the number of bytes before a null terminator and
treating that as the size and memory size of the string.) */{
    String result;
    result.str = (char*)str;
    result.size = str_size((char*)str);
    result.memory_size = result.size;
    return(result);
}

CPP_NAME(substr)
FSTRING_INLINE String
substr_tail(String str, int32_t start)
/* DOC(This call creates a substring of str that starts with an offset from str's base.
The new string uses the same underlying memory so both strings will see changes.
Usually strings created this way should only go through immutable calls.) */{
    String result;
    result.str = str.str + start;
    result.size = str.size - start;
    result.memory_size = 0;
    return(result);
}

FSTRING_INLINE String
substr(String str, int32_t start, int32_t size)
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

FSTRING_LINK String
skip_whitespace(String str)
/* DOC(This call creates a substring that starts with the first non-whitespace character of str.
Like other substr calls, the new string uses the underlying memory and so should usually be
considered immutable.) DOC_SEE(substr) */{
    String result = {0};
    int32_t i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr(str, i, str.size - i);
    return(result);
}

FSTRING_LINK String
chop_whitespace(String str)
/* DOC(This call creates a substring that ends with the last non-whitespace character of str.
Like other substr calls, the new string uses the underlying memory and so should usually be
considered immutable.) DOC_SEE(substr) */{
    String result = {0};
    int32_t i = str.size;
    for (; i > 0 && char_is_whitespace(str.str[i-1]); --i);
    result = substr(str, 0, i);
    return(result);
}

FSTRING_LINK String
skip_chop_whitespace(String str)
/* DOC(This call is equivalent to calling skip_whitespace and chop_whitespace together.)
DOC_SEE(skip_whitespace) DOC_SEE(chop_whitespace)*/{
    str = skip_whitespace(str);
    str = chop_whitespace(str);
    return(str);
}

FSTRING_INLINE String
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
FSTRING_LINK fstr_bool
match_cc(char *a, char *b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
    for (int32_t i = 0;; ++i){
        if (a[i] != b[i]){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}

CPP_NAME(match)
FSTRING_LINK fstr_bool
match_sc(String a, char *b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
    int32_t i = 0;
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
FSTRING_INLINE fstr_bool
match_cs(char *a, String b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
    return(match_sc(b,a));
}

CPP_NAME(match)
FSTRING_LINK fstr_bool
match_ss(String a, String b)/* DOC(This call returns non-zero if a and b are equivalent.) */{
    if (a.size != b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

CPP_NAME(match_part)
FSTRING_LINK fstr_bool
match_part_ccl(char *a, char *b, int32_t *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (a[i] != b[i]){
            return 0;
        }
    }
    *len = i;
    return 1;
}

CPP_NAME(match_part)
FSTRING_LINK fstr_bool
match_part_scl(String a, char *b, int32_t *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (a.str[i] != b[i] || i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}

CPP_NAME(match_part)
FSTRING_INLINE fstr_bool
match_part_cc(char *a, char *b)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    int32_t x;
    return match_part_ccl(a,b,&x);
}

CPP_NAME(match_part)
FSTRING_INLINE fstr_bool
match_part_sc(String a, char *b)/*
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    int32_t x;
    return match_part_scl(a,b,&x);
}

CPP_NAME(match_part)
FSTRING_LINK fstr_bool
match_part_cs(char *a, String b)/*
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    for (int32_t i = 0; i != b.size; ++i){
        if (a[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

CPP_NAME(match_part)
FSTRING_LINK fstr_bool
match_part_ss(String a, String b)/*
DOC(This call is similar to a match call, except that it is permitted for a to be longer than b.
In other words this call returns non-zero if b is a prefix of a.) */{
    if (a.size < b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

CPP_NAME(match_insensitive)
FSTRING_LINK fstr_bool
match_insensitive_cc(char *a, char *b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
    for (int32_t i = 0;; ++i){
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
FSTRING_LINK fstr_bool
match_insensitive_sc(String a, char *b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
    int32_t i = 0;
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
FSTRING_INLINE fstr_bool
match_insensitive_cs(char *a, String b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
    return match_insensitive_sc(b,a);
}

CPP_NAME(match_insensitive)
FSTRING_LINK fstr_bool
match_insensitive_ss(String a, String b)/*
DOC(This call returns non-zero if a and b are equivalent under case insensitive comparison.) */{
    if (a.size != b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) !=
            char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}

CPP_NAME(match_part_insensitive)
FSTRING_LINK fstr_bool
match_part_insensitive_ccl(char *a, char *b, int32_t *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b[i])){
            return 0;
        }
    }
    *len = i;
    return 1;
}

CPP_NAME(match_part_insensitive)
FSTRING_LINK fstr_bool
match_part_insensitive_scl(String a, char *b, int32_t *len)/*
DOC_PARAM(len, If this call returns non-zero this parameter is used to output the length of b.)
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    int32_t i;
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
FSTRING_INLINE fstr_bool
match_part_insensitive_cc(char *a, char *b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    int32_t x;
    return match_part_insensitive_ccl(a,b,&x);
}

CPP_NAME(match_part_insensitive)
FSTRING_INLINE fstr_bool
match_part_insensitive_sc(String a, char *b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    int32_t x;
    return match_part_insensitive_scl(a,b,&x);
}

CPP_NAME(match_part_insensitive)
FSTRING_LINK fstr_bool
match_part_insensitive_cs(char *a, String b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    for (int32_t i = 0; i != b.size; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b.str[i])){
            return(0);
        }
    }
    return(1);
}

CPP_NAME(match_part_insensitive)
FSTRING_LINK fstr_bool
match_part_insensitive_ss(String a, String b)/*
DOC(This call performs the same partial matching rule as match_part under case insensitive comparison.)
DOC_SEE(match_part) */{
    if (a.size < b.size){
        return(0);
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b.str[i])){
            return(0);
        }
    }
    return(1);
}

CPP_NAME(compare)
FSTRING_LINK int32_t
compare_cc(char *a, char *b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
    int32_t i = 0, r = 0;
    while (a[i] == b[i] && a[i] != 0){
        ++i;
    }
    r = (a[i] > b[i]) - (a[i] < b[i]);
    return(r);
}

CPP_NAME(compare)
FSTRING_LINK int32_t
compare_sc(String a, char *b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
    int32_t i = 0, r = 0;
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
FSTRING_INLINE int32_t
compare_cs(char *a, String b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
    int32_t r = -compare_sc(b,a);
    return(r);
}

CPP_NAME(compare)
FSTRING_LINK int32_t
compare_ss(String a, String b)/*
DOC(This call returns zero if a and b are equivalent,
it returns negative if a sorts before b alphabetically,
and positive if a sorts after b alphabetically.) */{
    int32_t i = 0, r = 0;
    int32_t m = a.size;
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
FSTRING_LINK int32_t
find_c_char(char *str, int32_t start, char character)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The character parameter provides the character for which to search.)
DOC(This call returns the index of the first occurance of character, or the size of the string
if the character is not found.) */{
    int32_t i = start;
    while (str[i] != character && str[i] != 0) ++i;
    return(i);
}

CPP_NAME(find)
FSTRING_LINK int32_t
find_s_char(String str, int32_t start, char character)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The character parameter provides the character for which to search.)
DOC(This call returns the index of the first occurance of character, or the size of the string
if the character is not found.) */{
    int32_t i = start;
    while (i < str.size && str.str[i] != character) ++i;
    return(i);
}

CPP_NAME(rfind)
FSTRING_LINK int32_t
rfind_s_char(String str, int32_t start, char character)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The character parameter provides the character for which to search.)
DOC(This call looks for the largest index less than or equal to the start position where
the given character occurs.  If the index is found it is returned otherwise -1 is returned.) */{
    int32_t i = start;
    while (i >= 0 && str.str[i] != character) --i;
    return(i);
}

CPP_NAME(find)
FSTRING_LINK int32_t
find_c_chars(char *str, int32_t start, char *characters)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The characters parameter provides a null terminated array of characters for which to search.)
DOC(This call returns the index of the first occurance of a character in the characters array,
or the size of the string if no such character is not found.) */{
    int32_t i = start, j;
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
FSTRING_LINK int32_t
find_s_chars(String str, int32_t start, char *characters)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(character, The characters parameter provides a null terminated array of characters for which to search.)
DOC(This call returns the index of the first occurance of a character in the characters array,
or the size of the string if no such character is not found.) */{
    int32_t i = start, j;
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
FSTRING_LINK int32_t
find_substr_c(char *str, int32_t start, String seek)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call returns the index of the first occurance of the seek substring in str or the
size of str if no such substring in str is found.) */{
    int32_t i, j, k;
    fstr_bool hit;
    
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
FSTRING_LINK int32_t
find_substr_s(String str, int32_t start, String seek)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call returns the index of the first occurance of the seek substring in str or the
size of str if no such substring in str is found.) */{
    int32_t stop_at, i, j, k;
    fstr_bool hit;
    
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
FSTRING_LINK int32_t
rfind_substr_s(String str, int32_t start, String seek)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call returns the index of the last occurance of the seek substring in str
or -1 if no such substring in str is found.) */{
    int32_t i, j, k;
    fstr_bool hit;
    
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
FSTRING_LINK int32_t
find_substr_insensitive_c(char *str, int32_t start, String seek)/*
DOC_PARAM(str, The str parameter provides a null terminated string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call acts as find_substr under case insensitive comparison.)
DOC_SEE(find_substr)*/{
    int32_t i, j, k;
    fstr_bool hit;
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

CPP_NAME(find_substr_insensitive)
FSTRING_LINK int32_t
find_substr_insensitive_s(String str, int32_t start, String seek)/*
DOC_PARAM(str, The str parameter provides a string to search.)
DOC_PARAM(start, The start parameter provides the index of the first character in str to search.)
DOC_PARAM(seek, The seek parameter provides a string to find in str.)
DOC(This call acts as find_substr under case insensitive comparison.)
DOC_SEE(find_substr)*/{
    int32_t i, j, k;
    int32_t stop_at;
    fstr_bool hit;
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

CPP_NAME(has_substr)
FSTRING_INLINE fstr_bool
has_substr_c(char *s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek.) */{
    return (s[find_substr_c(s, 0, seek)] != 0);
}

CPP_NAME(has_substr)
FSTRING_INLINE fstr_bool
has_substr_s(String s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek.) */{
    return (find_substr_s(s, 0, seek) < s.size);
}

CPP_NAME(has_substr_insensitive)
FSTRING_INLINE fstr_bool
has_substr_insensitive_c(char *s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek
under case insensitive comparison.) */{
    return (s[find_substr_insensitive_c(s, 0, seek)] != 0);
}

CPP_NAME(has_substr_insensitive)
FSTRING_INLINE fstr_bool
has_substr_insensitive_s(String s, String seek)/*
DOC(This call returns non-zero if the string s contains a substring equivalent to seek
under case insensitive comparison.) */{
    return (find_substr_insensitive_s(s, 0, seek) < s.size);
}

//
// String Copies and Appends
//

CPP_NAME(copy_fast_unsafe)
FSTRING_LINK int32_t
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
    return (int32_t)(dest - start);
}

CPP_NAME(copy_fast_unsafe)
FSTRING_LINK int32_t
copy_fast_unsafe_cs(char *dest, String src)/*
DOC(This call performs a copy from the src string to the dest buffer.
The copy does not stop until src.size characters are coppied.  There
is no safety against overrun so dest must be large enough to contain src.
The null terminator is not written to dest. This call returns the number
of bytes coppied to dest.) */{
    int32_t i = 0;
    while (i != src.size){
        dest[i] = src.str[i];
        ++i;
    }
    return(src.size);
}

CPP_NAME(copy_checked)
FSTRING_LINK fstr_bool
copy_checked_ss(String *dest, String src)/*
DOC(This call performs a copy from the src string to the dest string.
The memory_size of dest is checked before any coppying is done.
This call returns non-zero on a successful copy.) */{
    char *dest_str;
    int32_t i;
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

CPP_NAME(copy_partial)
FSTRING_LINK fstr_bool
copy_partial_sc(String *dest, char *src)/*
DOC(This call performs a copy from the src buffer to the dest string.
The memory_size of dest is checked if the entire copy cannot be performed,
as many bytes as possible are coppied to dest. This call returns non-zero
if the entire string is coppied to dest.) */{
    int32_t i = 0;
    int32_t memory_size = dest->memory_size;
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
FSTRING_LINK fstr_bool
copy_partial_ss(String *dest, String src)/*
DOC(This call performs a copy from the src string to the dest string.
The memory_size of dest is checked if the entire copy cannot be performed,
as many bytes as possible are coppied to dest. This call returns non-zero
if the entire string is coppied to dest.) */{
    char *dest_str = dest->str;
    int32_t memory_size = dest->memory_size;
    fstr_bool result = 0;
    if (memory_size >= src.size){
        result = 1;
        memory_size = src.size;
    }
    for (int32_t i = 0; i < memory_size; ++i){
        dest_str[i] = src.str[i];
    }
    dest->size = memory_size;
    return result;
}

CPP_NAME(copy)
FSTRING_INLINE int32_t
copy_cc(char *dest, char *src)/*
DOC(This call performs a copy from src to dest equivalent to copy_fast_unsafe.)
DOC_SEE(copy_fast_unsafe) */{
    return copy_fast_unsafe_cc(dest, src);
}

CPP_NAME(copy)
FSTRING_INLINE void
copy_ss(String *dest, String src)/*
DOC(This call performs a copy from src to dest equivalent to copy_checked.)
DOC_SEE(copy_checked) */{
    copy_checked_ss(dest, src);
}

CPP_NAME(copy)
FSTRING_INLINE void
copy_sc(String *dest, char *src)/*
DOC(This call performs a copy from src to dest equivalent to copy_partial.)
DOC_SEE(copy_partial) */{
    copy_partial_sc(dest, src);
}

CPP_NAME(append_checked)
FSTRING_LINK fstr_bool
append_checked_ss(String *dest, String src)/*
DOC(This call checks if there is enough space in dest's underlying memory
to append src onto dest. If there is src is appended and the call returns non-zero.) */{
    String end;
    end = tailstr(*dest);
    fstr_bool result = copy_checked_ss(&end, src);
    // NOTE(allen): This depends on end.size still being 0 if
    // the check failed and no coppy occurred.
    dest->size += end.size;
    return result;
}

CPP_NAME(append_partial)
FSTRING_LINK fstr_bool
append_partial_sc(String *dest, char *src)/*
DOC(This call attemps to append as much of src into the space in dest's underlying memory
as possible.  If the entire string is appended the call returns non-zero.) */{
    String end = tailstr(*dest);
    fstr_bool result = copy_partial_sc(&end, src);
    dest->size += end.size;
    return result;
}

CPP_NAME(append_partial)
FSTRING_LINK fstr_bool
append_partial_ss(String *dest, String src)/*
DOC(This call attemps to append as much of src into the space in dest's underlying memory
as possible.  If the entire string is appended the call returns non-zero.) */{
    String end = tailstr(*dest);
    fstr_bool result = copy_partial_ss(&end, src);
    dest->size += end.size;
    return result;
}

CPP_NAME(append)
FSTRING_LINK fstr_bool
append_s_char(String *dest, char c)/*
DOC(This call attemps to append c onto dest.  If there is space left in dest's underlying
memory the character is appended and the call returns non-zero.) */{
    fstr_bool result = 0;
    if (dest->size < dest->memory_size){
        dest->str[dest->size++] = c;
        result = 1;
    }
    return result;
}

CPP_NAME(append)
FSTRING_INLINE fstr_bool
append_ss(String *dest, String src)/*
DOC(This call is equivalent to append_partial.) DOC_SEE(append_partial) */{
    return append_partial_ss(dest, src);
}

CPP_NAME(append)
FSTRING_INLINE fstr_bool
append_sc(String *dest, char *src)/*
DOC(This call is equivalent to append_partial.) DOC_SEE(append_partial) */{
    return append_partial_sc(dest, src);
}

FSTRING_LINK fstr_bool
terminate_with_null(String *str)/*
DOC(This call attemps to append a null terminator onto str without effecting the
size of str. This is usually called when the time comes to pass the the string to an
API that requires a null terminator.  This call returns non-zero if there was a spare
byte in the strings underlying memory.) */{
    fstr_bool result = 0;
    if (str->size < str->memory_size){
        str->str[str->size] = 0;
        result = 1;
    }
    return(result);
}

FSTRING_LINK fstr_bool
append_padding(String *dest, char c, int32_t target_size)/*
DOC(This call pads out dest so that it has a size of target_size by appending
the padding character c until the target size is achieved.  This call returns
non-zero if dest does not run out of space in the underlying memory.) */{
    fstr_bool result = 1;
    int32_t offset = target_size - dest->size;
    int32_t r = 0;
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

FSTRING_LINK void
replace_char(String *str, char replace, char with)/*
DOC_PARAM(str, The str parameter provides the string in which replacement shall be performed.)
DOC_PARAM(replace, The replace character specifies which character should be replaced.)
DOC_PARAM(with, The with character specifies what to write into the positions where replacement occurs.)
DOC(This call replaces all occurances of character in str with another character.) */{
    char *s = str->str;
    int32_t i = 0;
    for (i = 0; i < str->size; ++i, ++s){
        if (*s == replace) *s = with;
    }
}

CPP_NAME(to_lower)
FSTRING_LINK void
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
FSTRING_LINK void
to_lower_ss(String *dst, String src)/*
DOC_PARAM(dst, The destination buffer to receive the converted string.
This must have a capacity of at least the size of src.)
DOC_PARAM(src, The source string to conver to lowercase.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap with the exception
that src and dst may be exactly equal in order to convert the string in place.)
*/{
    int32_t i = 0;
    int32_t size = src.size;
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
FSTRING_LINK void
to_lower_s(String *str)/*
DOC_PARAM(str, The string to be converted to all lowercase.)
DOC(This version of to_lower converts str to lowercase in place.)
*/{
    int32_t i = 0;
    int32_t size = str->size;
    char *c = str->str;
    for (; i < size; ++c, ++i){
        *c = char_to_lower(*c);
    }
}

CPP_NAME(to_upper)
FSTRING_LINK void
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
FSTRING_LINK void
to_upper_ss(String *dst, String src)/*
DOC_PARAM(dst, The destination buffer to receive the converted string.
This must have a capacity of at least the size of src.)
DOC_PARAM(src, The source string to convert to uppercase.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap with the exception
that src and dst may be exactly equal in order to convert the string in place.)
*/{
    int32_t i = 0;
    int32_t size = src.size;
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
FSTRING_LINK void
to_upper_s(String *str)/*
DOC_PARAM(str, The string to be converted to all uppercase.)
DOC(This version of to_upper converts str to uppercase in place.)
*/{
    int32_t i = 0;
    int32_t size = str->size;
    char *c = str->str;
    for (; i < size; ++c, ++i){
        *c = char_to_upper(*c);
    }
}

CPP_NAME(to_camel)
FSTRING_LINK void
to_camel_cc(char *src, char *dst)/*
DOC_PARAM(src, The source string to convert to camel case.)
DOC_PARAM(dst, The destination buffer to receive the converted string.
This must be large enough to contain all of src and a null terminator.)
DOC(Rewrites the string in src into dst.  src and dst should not overlap
with the exception that src and dst may be exactly equal in order to
convert the string in place.)
*/{
    char *c, ch;
    int32_t is_first = 1;
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

FSTRING_LINK int32_t
int_to_str_size(int32_t x)/*
DOC(This call returns the number of bytes required to represent x as a string.) */{
    int32_t size = 1;
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

FSTRING_LINK fstr_bool
int_to_str(String *dest, int32_t x)/*
DOC(This call writes a string representation of x into dest. If there is enough
space in dest this call returns non-zero.) */{
    fstr_bool result = 1;
    char *str = dest->str;
    int32_t memory_size = dest->memory_size;
    int32_t size, i, j;
    fstr_bool negative;
    
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
            // NOTE(allen): Start i = 0 if not negative, start i = 1 if is negative
            // because - should not be flipped if it is negative :)
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

FSTRING_LINK fstr_bool
append_int_to_str(String *dest, int32_t x)/*
DOC(This call appends a string representation of x onto dest. If there is enough
space in dest this call returns non-zero.) */{
    String last_part = tailstr(*dest);
    fstr_bool result = int_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}

FSTRING_LINK int32_t
u64_to_str_size(uint64_t x)/*
DOC(This call returns the number of bytes required to represent x as a string.) */{
    int32_t size;
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

FSTRING_LINK fstr_bool
u64_to_str(String *dest, uint64_t x)/*
DOC(This call writes a string representation of x into dest. If there is enough
space in dest this call returns non-zero.) */{
    fstr_bool result = 1;
    char *str = dest->str;
    int32_t memory_size = dest->memory_size;
    int32_t size, i, j;
    
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

FSTRING_LINK fstr_bool
append_u64_to_str(String *dest, uint64_t x)/*
DOC(This call appends a string representation of x onto dest. If there is enough
space in dest this call returns non-zero.) */{
    String last_part = tailstr(*dest);
    fstr_bool result = u64_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}

#ifndef FSTRING_GUARD
typedef struct Float_To_Str_Variables{
    fstr_bool negative;
    int32_t int_part;
    int32_t dec_part;
} Float_To_Str_Variables;

static Float_To_Str_Variables
get_float_vars(float x){
    Float_To_Str_Variables vars = {0};
    
    if (x < 0){
        vars.negative = 1;
        x = -x;
    }
    
    vars.int_part = (int32_t)(x);
    vars.dec_part = (int32_t)((x - vars.int_part) * 1000);
    
    return(vars);
}
#endif

FSTRING_LINK int32_t
float_to_str_size(float x)/*
DOC(This call returns the number of bytes required to represent x as a string.) */{
    Float_To_Str_Variables vars = get_float_vars(x);
    int32_t size =
        vars.negative + int_to_str_size(vars.int_part) + 1 + int_to_str_size(vars.dec_part);
    return(size);
}

FSTRING_LINK fstr_bool
append_float_to_str(String *dest, float x)/*
DOC(This call writes a string representation of x into dest. If there is enough
space in dest this call returns non-zero.) */{
    fstr_bool result = 1;
    Float_To_Str_Variables vars = get_float_vars(x);
    
    if (vars.negative){
        append_s_char(dest, '-');
    }
    
    append_int_to_str(dest, vars.int_part);
    append_s_char(dest, '.');
    append_int_to_str(dest, vars.dec_part);
    
    return(result);
}

FSTRING_LINK fstr_bool
float_to_str(String *dest, float x)/*
DOC(This call appends a string representation of x onto dest. If there is enough
space in dest this call returns non-zero.) */{
    fstr_bool result = 1;
    dest->size = 0;
    append_float_to_str(dest, x);
    return(result);
}

CPP_NAME(str_is_int)
FSTRING_LINK int32_t
str_is_int_c(char *str)/*
DOC(If str is a valid string representation of an integer, this call returns non-zero) */{
    fstr_bool result = 1;
    for (; *str; ++str){
        if (!char_is_numeric(*str)){
            result = 0;
            break;
        }
    }
    return(result);
}

CPP_NAME(str_is_int)
FSTRING_LINK fstr_bool
str_is_int_s(String str)/*
DOC(If str is a valid string representation of an integer, this call returns non-zero.) */{
    fstr_bool result = 1;
    for (int32_t i = 0; i < str.size; ++i){
        if (!char_is_numeric(str.str[i])){
            result = 0;
            break;
        }
    }
    return(result);
}

CPP_NAME(str_to_int)
FSTRING_LINK int32_t
str_to_int_c(char *str)/*
DOC(If str is a valid string representation of an integer, this call will return
the integer represented by the string.  Otherwise this call returns zero.) */{
    int32_t x = 0;
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
FSTRING_LINK int32_t
str_to_int_s(String str)/*
DOC(If str represents a valid string representation of an integer, this call will return
the integer represented by the string.  Otherwise this call returns zero.) */{
    int32_t x, i;
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

FSTRING_LINK int32_t
hexchar_to_int(char c)/*
DOC(If c is a valid hexadecimal digit [0-9a-fA-F] this call returns the value of
the integer value of the digit. Otherwise the return is some nonsense value.) */{
    int32_t x = 0;
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

FSTRING_LINK char
int_to_hexchar(int32_t x)/*
DOC(If x is in the range [0,15] this call returns the equivalent lowercase hexadecimal digit.
Otherwise the return is some nonsense value.) */{
    return (x<10)?((char)x+'0'):((char)x+'a'-10);
}

FSTRING_LINK uint32_t
hexstr_to_int(String str)/*
DOC(This call interprets str has a hexadecimal representation of an integer and returns
the represented integer value.) */{
    uint32_t x;
    int32_t i;
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

FSTRING_LINK fstr_bool
color_to_hexstr(String *s, uint32_t color)/*
DOC(This call fills s with the hexadecimal representation of the color.
If there is enough memory in s to represent the color this call returns non-zero.) */{
    fstr_bool result = 0;
    int32_t i;
    
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

FSTRING_LINK fstr_bool
hexstr_to_color(String s, uint32_t *out)/*
DOC(This call interprets s as a color and writes the 32-bit integer representation into out.) */{
    fstr_bool result = 0;
    uint32_t color = 0;
    if (s.size == 6){
        result = 1;
        color = (uint32_t)hexstr_to_int(s);
        color |= (0xFF << 24);
        *out = color;
    }
    else if (s.size == 8){
        result = 1;
        color = (uint32_t)hexstr_to_int(s);
        *out = color;
    }
    return(result);
}

//
// Directory String Management
//

CPP_NAME(reverse_seek_slash)
FSTRING_LINK int32_t
reverse_seek_slash_pos(String str, int32_t pos)/*
DOC(This call searches for a slash in str by starting pos bytes from the end and going backwards.) */{
    int32_t i = str.size - 1 - pos;
    while (i >= 0 && !char_is_slash(str.str[i])){
        --i;
    }
    return i;
}

FSTRING_INLINE int32_t
reverse_seek_slash(String str)/*
DOC(This call searches for a slash in str by starting at the end and going backwards.) */{
    return(reverse_seek_slash_pos(str, 0));
}

FSTRING_INLINE String
front_of_directory(String dir)/*
DOC(This call returns a substring of dir containing only the file name or
folder name furthest to the right in the directory.) DOC_SEE(substr) */{
    return substr_tail(dir, reverse_seek_slash(dir) + 1);
}

FSTRING_INLINE String
path_of_directory(String dir)/*
DOC(This call returns a substring of dir containing the whole path except
for the final file or folder name.) DOC_SEE(substr) */{
    return substr(dir, 0, reverse_seek_slash(dir) + 1);
}

CPP_NAME(set_last_folder)
FSTRING_LINK fstr_bool
set_last_folder_sc(String *dir, char *folder_name, char slash)/*
DOC_PARAM(dir, The dir parameter is the directory string in which to set the last folder in the directory.)
DOC_PARAM(folder_name, The folder_name parameter is a null terminated string specifying the name to set
at the end of the directory.)
DOC_PARAM(slash, The slash parameter specifies what slash to use between names in the directory.)
DOC(This call deletes the last file name or folder name in the dir string and appends the new provided one.
If there is enough memory in dir this call returns non-zero.) */{
    fstr_bool result = 0;
    int32_t size = reverse_seek_slash(*dir) + 1;
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
FSTRING_LINK fstr_bool
set_last_folder_ss(String *dir, String folder_name, char slash)/*
DOC_PARAM(dir, The dir parameter is the directory string in which to set the last folder in the directory.)
DOC_PARAM(folder_name, The folder_name parameter is a string specifying the name to set at the end of the directory.)
DOC_PARAM(slash, The slash parameter specifies what slash to use between names in the directory.)
DOC(This call deletes the last file name or folder name in the dir string and appends the new provided one.
If there is enough memory in dir this call returns non-zero.) */{
    fstr_bool result = 0;
    int32_t size = reverse_seek_slash(*dir) + 1;
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

FSTRING_LINK String
file_extension(String str)/*
DOC(This call returns a substring containing only the file extension of the provided filename.)
DOC_SEE(substr) */{
    int32_t i;
    for (i = str.size - 1; i >= 0; --i){
        if (str.str[i] == '.') break;
    }
    ++i;
    return(make_string(str.str+i, str.size-i));
}

FSTRING_LINK fstr_bool
remove_extension(String *str)/*
DOC(This call attemps to delete a file extension off the end of a filename.
This call returns non-zero on success.) */{
    fstr_bool result = 0;
    int32_t i;
    for (i = str->size - 1; i >= 0; --i){
        if (str->str[i] == '.') break;
    }
    if (i >= 0){
        result = 1;
        str->size = i + 1;
    }
    return(result);
}

FSTRING_LINK fstr_bool
remove_last_folder(String *str)/*
DOC(This call attemps to delete a folder or filename off the end of a path string.
This call returns non-zero on success.) */{
    fstr_bool result = 0;
    int32_t end = reverse_seek_slash_pos(*str, 1);
    if (end >= 0){
        result = 1;
        str->size = end + 1;
    }
    return(result);
}

// TODO(allen): Add hash-table extension to string sets.
CPP_NAME(string_set_match)
FSTRING_LINK fstr_bool
string_set_match_table(void *str_set, int32_t item_size, int32_t count, String str, int32_t *match_index)/*
DOC_PARAM(str_set, The str_set parameter is an array of String structs specifying matchable strings.)
DOC_PARAM(count, The count parameter specifies the number of String structs in the str_set array.)
DOC_PARAM(str, The str parameter specifies the string to match against the str_set.)
DOC_PARAM(match_index, If this call succeeds match_index is filled with the index into str_set where the match occurred.)
DOC(This call tries to see if str matches any of the strings in str_set.  If there is a match the call
succeeds and returns non-zero.  The matching rule is equivalent to the matching rule for match.)
DOC_SEE(match) */{
    fstr_bool result = 0;
    int32_t i = 0;
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

FSTRING_LINK fstr_bool
string_set_match(String *str_set, int32_t count, String str, int32_t *match_index)/*
DOC_PARAM(str_set, The str_set parameter is an array of String structs specifying matchable strings.)
DOC_PARAM(count, The count parameter specifies the number of String structs in the str_set array.)
DOC_PARAM(str, The str parameter specifies the string to match against the str_set.)
DOC_PARAM(match_index, If this call succeeds match_index is filled with the index into str_set where the match occurred.)
DOC(This call tries to see if str matches any of the strings in str_set.  If there is a match the call
succeeds and returns non-zero.  The matching rule is equivalent to the matching rule for match.)
DOC_SEE(match) */{
    fstr_bool result = string_set_match_table(str_set, sizeof(String), count, str, match_index);
    return(result);
}


#ifndef FSTRING_EXPERIMENTAL
#define FSTRING_EXPERIMENTAL

// NOTE(allen): experimental section, things below here are
// not promoted to public API level yet.

#ifndef ArrayCount
# define ArrayCount(a) ((sizeof(a))/sizeof(*a))
#endif

typedef struct Absolutes{
    String a[8];
    int32_t count;
} Absolutes;

static void
get_absolutes(String name, Absolutes *absolutes, fstr_bool implicit_first, fstr_bool implicit_last){
    int32_t count = 0;
    int32_t max = ArrayCount(absolutes->a) - 1;
    if (implicit_last) --max;
    
    String str;
    str.str = name.str;
    str.size = 0;
    str.memory_size = 0;
    fstr_bool prev_was_wild = 0;
    
    if (implicit_first){
        absolutes->a[count++] = str;
        prev_was_wild = 1;
    }
    
    int32_t i;
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

static fstr_bool
wildcard_match_c(Absolutes *absolutes, char *x, int32_t case_sensitive){
    fstr_bool r = 1;
    String *a = absolutes->a;
    
    fstr_bool (*match_func)(char*, String);
    fstr_bool (*match_part_func)(char*, String);
    
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
    return(r);
}

static fstr_bool
wildcard_match_s(Absolutes *absolutes, String x, int32_t case_sensitive){
    terminate_with_null(&x);
    return(wildcard_match_c(absolutes, x.str, case_sensitive));
}

#endif

#if defined(FSTRING_IMPLEMENTATION)
#undef FSTRING_IMPLEMENTATION
#endif

#if !defined(FSTRING_GUARD)
#define FSTRING_GUARD
#endif

// BOTTOM

