/* "4cpp" Open C++ Parser v0.1: String
   no warranty implied; use at your own risk
   
NOTES ON USE:
   OPTIONS:
   Set options by defining macros before including this file
   
   FCPP_STRING_IMPLEMENTATION - causes this file to output function implementations
                              - this option is unset after use so that future includes of this file
                                in the same unit do not continue to output implementations
   
   FCPP_LINK - defines linkage of non-inline functions, defaults to static
   FCPP_EXTERN changes FCPP_LINK default to extern, this option is ignored if FCPP_LINK is defined

   include the file "4cpp_clear_config.h" if yo want to undefine all options for some reason

   HIDDEN DEPENDENCIES:
   none
 */

// TOP
// TODO(allen):
// - comments
// - memcpy / memmove replacements (different file for optimization options?)
// 

#include "4coder_config.h"

#ifndef FCPP_STRING_INC
#define FCPP_STRING_INC

#ifndef FRED_STRING_STRUCT
#define FRED_STRING_STRUCT
struct String{
    char *str;
    int size;
    int memory_size;
};
#endif

inline bool char_not_slash(char c) { return (c != '\\' && c != '/'); }
inline bool char_is_slash(char c) { return (c == '\\' || c == '/'); }

inline char char_to_upper(char c) { return (c >= 'a' && c <= 'z') ? c + (char)('A' - 'a') : c; }
inline char char_to_lower(char c) { return (c >= 'A' && c <= 'Z') ? c - (char)('A' - 'a') : c; }

inline bool char_is_whitespace(char c) { return (c == ' ' || c == '\n' || c == '\r' || c == '\t'); }
inline bool char_is_white_not_r(char c) { return (c == ' ' || c == '\n' || c == '\t'); }
inline bool char_is_lower(char c) { return (c >= 'a' && c <= 'z'); }
inline bool char_is_upper(char c) { return (c >= 'A' && c <= 'Z'); }
inline bool char_is_alpha(char c) { return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_'); }
inline bool char_is_alpha_true(char c) { return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'); }
inline bool char_is_numeric(char c) { return (c >= '0' && c <= '9'); }
inline bool char_is_alpha_numeric_true(char c) { return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9'); }
inline bool char_is_alpha_numeric(char c) { return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_'); }
inline bool char_is_hex(char c) { return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f'; }
inline bool char_is_basic(char c) { return c >= ' ' && c <= '~'; }

inline String make_string(char *s, int size, int mem_size);
inline String make_string(char *s, int size);

#define make_lit_string(str) (make_string((char*)(str), sizeof(str)-1, sizeof(str)))
#define make_fixed_width_string(str) (make_string((char*)(str), 0, sizeof(str)))

#define expand_str(s) ((s).str), ((s).size)

inline String make_string_slowly(char *s);
inline char*  make_c_str(String s);

inline String substr(String str, int start);
inline String substr(String str, int start, int size);
inline String substr_slowly(char *s, int start);
inline String substr(char *s, int start, int size);
inline String tailstr(String s);


FCPP_LINK int   str_size(char *s);

FCPP_LINK bool  match(char *a, char *b);
FCPP_LINK bool  match(String a, char *b);
inline    bool  match(char *a, String b) { return match(b,a); }
FCPP_LINK bool  match(String a, String b);

FCPP_LINK bool  match_part(char *a, char *b, int *len);
FCPP_LINK bool  match_part(String a, char *b, int *len);
inline    bool  match_part(char *a, char *b) { int x; return match_part(a,b,&x); }
inline    bool  match_part(String a, char *b) { int x; return match_part(a,b,&x); }
FCPP_LINK bool  match_part(char *a, String b);
FCPP_LINK bool  match_part(String a, String b);

FCPP_LINK bool  match_unsensitive(char *a, char *b);
FCPP_LINK bool  match_unsensitive(String a, char *b);
inline    bool  match_unsensitive(char *a, String b) { return match_unsensitive(b,a); }
FCPP_LINK bool  match_unsensitive(String a, String b);

FCPP_LINK bool  match_part_unsensitive(char *a, char *b, int *len);
FCPP_LINK bool  match_part_unsensitive(String a, char *b, int *len);
inline    bool  match_part_unsensitive(char *a, char *b) { int x; return match_part(a,b,&x); }
inline    bool  match_part_unsensitive(String a, char *b) { int x; return match_part(a,b,&x); }
FCPP_LINK bool  match_part_unsensitive(char *a, String b);
FCPP_LINK bool  match_part_unsensitive(String a, String b);

FCPP_LINK int   find(char *s, int start, char c);
FCPP_LINK int   find(String s, int start, char c);
FCPP_LINK int   find(char *s, int start, char *c);
FCPP_LINK int   find(String s, int start, char *c);

FCPP_LINK int   find_substr(char *s, int start, String seek);
FCPP_LINK int   find_substr(String s, int start, String seek);
FCPP_LINK int   rfind_substr(String s, int start, String seek);

FCPP_LINK int   find_substr_unsensitive(char *s, int start, String seek);
FCPP_LINK int   find_substr_unsensitive(String s, int start, String seek);

inline    bool  has_substr(char *s, String seek) { return (s[find_substr(s, 0, seek)] != 0); }
inline    bool  has_substr(String s, String seek) { return (find_substr(s, 0, seek) < s.size); }

inline    bool  has_substr_unsensitive(char *s, String seek) { return (s[find_substr_unsensitive(s, 0, seek)] != 0); }
inline    bool  has_substr_unsensitive(String s, String seek) { return (find_substr_unsensitive(s, 0, seek) < s.size); }

FCPP_LINK int   int_to_str_size(int x);
FCPP_LINK int   int_to_str(int x, char *s_out);
FCPP_LINK bool  int_to_str(int x, String *s_out);
FCPP_LINK bool  append_int_to_str(int x, String *s_out);

FCPP_LINK int   str_to_int(char *s);
FCPP_LINK int   str_to_int(String s);
FCPP_LINK int   hexchar_to_int(char c);
FCPP_LINK int   int_to_hexchar(char c);
FCPP_LINK int   hexstr_to_int(String s);

FCPP_LINK int   copy_fast_unsafe(char *dest, char *src);
FCPP_LINK void  copy_fast_unsafe(char *dest, String src);
FCPP_LINK bool  copy_checked(String *dest, String src);
FCPP_LINK bool  copy_partial(String *dest, char *src);
FCPP_LINK bool  copy_partial(String *dest, String src);

inline    int   copy(char *dest, char *src) { return copy_fast_unsafe(dest, src); }
inline    void  copy(String *dest, String src) { copy_checked(dest, src); }
inline    void  copy(String *dest, char *src) { copy_partial(dest, src); }

FCPP_LINK bool  append_checked(String *dest, String src);
FCPP_LINK bool  append_partial(String *dest, char *src);
FCPP_LINK bool  append_partial(String *dest, String src);

FCPP_LINK bool  append(String *dest, char c);
inline    bool  append(String *dest, String src) { return append_partial(dest, src); }
inline    bool  append(String *dest, char *src) { return append_partial(dest, src); }
inline    bool  terminate_with_null(String *str){
    bool result;
    if (str->size < str->memory_size){
        str->str[str->size] = 0;
        result = 1;
    }
    else{
        str->str[str->size-1] = 0;
        result = 0;
    }
    return result;
}

FCPP_LINK int   compare(char *a, char *b);
FCPP_LINK int   compare(String a, char *b);
inline    int   compare(char *a, String b) { return -compare(b,a); }
FCPP_LINK int   compare(String a, String b);

FCPP_LINK int    reverse_seek_slash(String str);
FCPP_LINK int    reverse_seek_slash(String str, int start_pos);
inline    bool   get_front_of_directory(String *dest, String dir) { return append_checked(dest, substr(dir, reverse_seek_slash(dir) + 1)); }
inline    bool   get_path_of_directory(String *dest, String dir) { return append_checked(dest, substr(dir, 0, reverse_seek_slash(dir) + 1)); }
FCPP_LINK bool   set_last_folder(String *dir, char *folder_name);
FCPP_LINK bool   set_last_folder(String *dir, String folder_name);
FCPP_LINK String file_extension(String str);
FCPP_LINK String file_extension_slowly(char *str);
FCPP_LINK bool   remove_last_folder(String *str);

inline String make_string(char *str, int size, int mem_size){
    String result;
    result.str = str;
    result.size = size;
    result.memory_size = mem_size;
    return result;
}

inline String
make_string(char *str, int size){
    String result;
    result.str = str;
    result.size = size;
    result.memory_size = size;
    return result;
}

inline String
make_string_slowly(char *str){
    String result;
    result.str = str;
    result.size = str_size(str);
    result.memory_size = result.size;
    return result;
}

inline char*
make_c_str(String str){
    if (str.size < str.memory_size){
        str.str[str.size] = 0;
    }
    else{
        str.str[str.memory_size-1] = 0;
    }
    return (char*)str.str;
}

inline String
substr(String str, int start){
    String result;
    result.str = str.str + start;
    result.size = str.size - start;
    return result;
}

inline String
substr(String str, int start, int size){
    String result;
    result.str = str.str + start;
    if (start + size > str.size){
        result.size = str.size - start;
    }
    else{
        result.size = size;
    }
    return result;
}

inline String
substr_slowly(char *str, int start){
    String result;
    result.str = str + start;
    result.size = str_size(result.str);
    return result;
}

inline String
substr(char *str, int start, int size){
    String result;
    result.str = str + start;
    result.size = size;
    for (int i = 0; i < size; ++i){
        if (result.str[i] == 0){
            result.size = i;
            break;
        }
    }
    return result;
}

inline String
tailstr(String str){
    String result;
    result.str = str.str + str.size;
    result.memory_size = str.memory_size - str.size;
    result.size = 0;
    return result;
}

#endif // #ifndef FCPP_STRING_INC

#ifdef FCPP_STRING_IMPLEMENTATION

FCPP_LINK int
str_size(char *str){
    int i = 0;
    while (str[i]) ++i;
    return i;
}

FCPP_LINK bool
match(char *a, char *b){
    for (int i = 0;; ++i){
        if (a[i] != b[i]){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}

FCPP_LINK bool
match(String a, char *b){
    int i = 0;
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

FCPP_LINK bool
match(String a, String b){
    if (a.size != b.size){
        return 0;
    }
    for (int i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

FCPP_LINK bool
match_part(char *a, char *b, int *len){
    int i;
    for (i = 0; b[i] != 0; ++i){
        if (a[i] != b[i]){
            return 0;
        }
    }
    *len = i;
    return 1;
}

FCPP_LINK bool
match_part(String a, char *b, int *len){
    int i;
    for (i = 0; b[i] != 0; ++i){
        if (a.str[i] != b[i] || i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}

FCPP_LINK bool
match_part(char *a, String b){
    for (int i = 0; i != b.size; ++i){
        if (a[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

FCPP_LINK bool
match_part(String a, String b){
    if (a.size < b.size){
        return 0;
    }
    for (int i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}

FCPP_LINK bool
match_unsensitive(char *a, char *b){
    for (int i = 0;; ++i){
        if (char_to_upper(a[i]) !=
            char_to_upper(b[i])){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}

FCPP_LINK bool
match_unsensitive(String a, char *b){
    int i = 0;
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

FCPP_LINK bool
match_unsensitive(String a, String b){
    if (a.size != b.size){
        return 0;
    }
    for (int i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) !=
            char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}

FCPP_LINK bool
match_part_unsensitive(char *a, char *b, int *len){
    int i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b[i])){
            return 0;
        }
    }
    *len = i;
    return 1;
}

FCPP_LINK bool
match_part_unsensitive(String a, char *b, int *len){
    int i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b[i]) ||
            i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}

FCPP_LINK bool
match_part_unsensitive(char *a, String b){
    for (int i = 0; i != b.size; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}

FCPP_LINK bool
match_part_unsensitive(String a, String b){
    if (a.size < b.size){
        return 0;
    }
    for (int i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}

FCPP_LINK int
find(char *str, int start, char character){
    int i = start;
    while (str[i] != character && str[i] != 0) ++i;
    return i;
}

FCPP_LINK int
find(String str, int start, char character){
    int i = start;
    while (i < str.size && str.str[i] != character) ++i;
    return i;
}

FCPP_LINK int
find(char *str, int start, char *characters){
    int i = start, j;
    while (str[i] != 0){
        for (j = 0; characters[j]; ++j){
            if (str[i] == characters[j]){
                return i;
            }
        }
        ++i;
    }
    return i;
}

FCPP_LINK int
find(String str, int start, char *characters){
    int i = start, j;
    while (i < str.size){
        for (j = 0; characters[j]; ++j){
            if (str.str[i] == characters[j]){
                return i;
            }
        }
        ++i;
    }
    return i;
}

FCPP_LINK int
find_substr(char *str, int start, String seek){
    int i, j, k;
    bool hit;
    
    if (seek.size == 0){
        return str_size(str);
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
                return i;
            }
        }
    }
    return i;
}

FCPP_LINK int
find_substr(String str, int start, String seek){
    int stop_at, i, j, k;
    bool hit;
    
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
    return str.size;
}

FCPP_LINK int
rfind_substr(String str, int start, String seek){
    int i, j, k;
    bool hit;
    
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

FCPP_LINK int
find_substr_unsensitive(char *str, int start, String seek){
    int i, j, k;
    bool hit;
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

FCPP_LINK int
find_substr_unsensitive(String str, int start, String seek){
    int i, j, k;
    int stop_at;
    bool hit;
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

FCPP_LINK int
int_to_str_size(int x){
    int size;
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
    return size;
}

FCPP_LINK int
int_to_str(int x, char *str){
    int size, i, j;
    bool negative;
    char temp;
    
    size = 0;
    if (x == 0){
        str[0] = '0';
        str[1] = 0;
        size = 1;
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
            i = x % 10;
            x /= 10;
            str[size++] = (char)('0' + i);
        }
        // NOTE(allen): Start i = 0 if not negative, start i = 1 if is negative
        // because - should not be flipped if it is negative :)
        for (i = negative, j = size-1; i < j; ++i, --j){
            temp = str[i];
            str[i] = str[j];
            str[j] = temp;
        }
        str[size] = 0;
    }
    return size;
}

FCPP_LINK bool
int_to_str(int x, String *dest){
    bool result = 1;
    char *str = dest->str;
    int memory_size = dest->memory_size;
    int size, i, j;
    bool negative;
    
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
    return result;
}

FCPP_LINK bool
append_int_to_str(int x, String *dest){
    String last_part = tailstr(*dest);
    bool result = int_to_str(x, &last_part);
    if (result){
        dest->size += last_part.size;
    }
    return result;
}

FCPP_LINK int
str_to_int(char *str){
    int x = 0;
    for (; *str; ++str){
        if (*str >= '0' || *str <= '9'){
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

FCPP_LINK int
str_to_int(String str){
    int x, i;
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
    return x;
}

FCPP_LINK int
hexchar_to_int(char c){
    int x;
    if (c >= '0' && c <= '9'){
        x = c-'0';
    }
    else if (c > 'F'){
        x = c+(10-'a');
    }
    else{
        x = c+(10-'A');
    }
    return x;
}

FCPP_LINK char
int_to_hexchar(int x){
    return (x<10)?((char)x+'0'):((char)x+'a'-10);
}

FCPP_LINK int
hexstr_to_int(String str){
    int x, i;
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
    return x;
}

FCPP_LINK int
copy_fast_unsafe(char *dest, char *src){
    char *start = dest;
    while (*src != 0){
        *dest = *src;
        ++dest;
        ++src;
    }
    return (int)(dest - start);
}

FCPP_LINK void
copy_fast_unsafe(char *dest, String src){
    int i = 0;
    while (i != src.size){
        dest[i] = src.str[i];
        ++i;
    }
}

FCPP_LINK bool
copy_checked(String *dest, String src){
    char *dest_str;
    int i;
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

FCPP_LINK bool
copy_partial(String *dest, char *src){
    int i = 0;
    int memory_size = dest->memory_size;
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

FCPP_LINK bool
copy_partial(String *dest, String src){
    bool result;
    int memory_size = dest->memory_size;
    char *dest_str = dest->str;
    if (memory_size < src.size){
        result = 0;
        for (int i = 0; i < memory_size; ++i){
            dest_str[i] = src.str[i];
        }
        dest->size = memory_size;
    }
    else{
        result = 1;
        for (int i = 0; i < src.size; ++i){
            dest_str[i] = src.str[i];
        }
        dest->size = src.size;
    }
    return result;
}

FCPP_LINK bool
append_checked(String *dest, String src){
    String end;
    end = tailstr(*dest);
    bool result = copy_checked(&end, src);
    // NOTE(allen): This depends on end.size still being 0 if
    // the check failed and no coppy occurred.
    dest->size += end.size;
    return result;
}

FCPP_LINK bool
append_partial(String *dest, char *src){
    String end = tailstr(*dest);
    bool result = copy_partial(&end, src);
    dest->size += end.size;
    return result;
}

FCPP_LINK bool
append_partial(String *dest, String src){
    String end = tailstr(*dest);
    bool result = copy_partial(&end, src);
    dest->size += end.size;
    return result;
}

FCPP_LINK bool
append(String *dest, char c){
    bool result = 0;
    if (dest->size < dest->memory_size){
        dest->str[dest->size++] = c;
        result = 1;
    }
    return result;
}

FCPP_LINK int
compare(char *a, char *b){
    int i = 0;
    while (a[i] == b[i] && a[i] != 0){
        ++i;
    }
    return (a[i] > b[i]) - (a[i] < b[i]);
}

FCPP_LINK int
compare(String a, char *b){
    int i = 0;
    while (i < a.size && a.str[i] == b[i]){
        ++i;
    }
    if (i < a.size){
        return (a.str[i] > b[i]) - (a.str[i] < b[i]);
    }
    else{
        if (b[i] == 0){
            return 0;
        }
        else{
            return -1;
        }
    }
}

FCPP_LINK int
compare(String a, String b){
    int i = 0;
    while (i < a.size && i < b.size && a.str[i] == b.str[i]){
        ++i;
    }
    if (i < a.size && i < b.size){
        return (a.str[i] > b.str[i]) - (a.str[i] < b.str[i]);
    }
    else{
        return (a.size > b.size) - (a.size < b.size);
    }
}

FCPP_LINK int
reverse_seek_slash(String str, int pos){
    int i = str.size - 1 - pos;
    while (i >= 0 && char_not_slash(str.str[i])){
        --i;
    }
    return i;
}

FCPP_LINK int
reverse_seek_slash(String str){
    return(reverse_seek_slash(str, 0));
}

FCPP_LINK bool
set_last_folder(String *dir, char *folder_name){
    bool result = 0;
    int size = reverse_seek_slash(*dir) + 1;
    dir->size = size;
    if (append(dir, folder_name)){
        if (append(dir, (char*)"\\")){
            result = 1;
        }
    }
    if (!result){
        dir->size = size;
    }
    return result;
}

FCPP_LINK bool
set_last_folder(String *dir, String folder_name){
    bool result = 0;
    int size = reverse_seek_slash(*dir) + 1;
    dir->size = size;
    if (append(dir, folder_name)){
        if (append(dir, (char*)"\\")){
            result = 1;
        }
    }
    if (!result){
        dir->size = size;
    }
    return result;
}

FCPP_LINK String
file_extension(String str){
    int i;
    for (i = str.size - 1; i >= 0; --i){
        if (str.str[i] == '.') break;
    }
    ++i;
    return make_string(str.str+i, str.size-i);
}

FCPP_LINK String
file_extension_slowly(char *str){
    int s, i;
    for (s = 0; str[s]; ++s);
    for (i = s - 1; i >= 0; --i){
        if (str[i] == '.') break;
    }
    ++i;
    return make_string(str+i, s-i);
}

FCPP_LINK bool
remove_last_folder(String *str){
    bool result = 0;
    int end = reverse_seek_slash(*str, 1);
    if (end >= 0){
        result = 1;
        str->size = end + 1;
    }
    return(result);
}

// NOTE(allen): experimental section, things below here are
// not promoted to public API level yet.

#ifndef ArrayCount
#define ArrayCount(a) ((sizeof(a))/sizeof(a))
#endif

struct Absolutes{
    String a[8];
    int count;
};

FCPP_LINK void
get_absolutes(String name, Absolutes *absolutes, bool implicit_first, bool implicit_last){
    int count = 0;
    int max = ArrayCount(absolutes->a) - 1;
    if (implicit_last) --max;
    
    String str;
    str.str = name.str;
    str.size = 0;
    str.memory_size = 0;
    bool prev_was_wild = 0;
    
    if (implicit_first){
        absolutes->a[count++] = str;
        prev_was_wild = 1;
    }
    
    int i;
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

FCPP_LINK bool
wildcard_match(Absolutes *absolutes, char *x, int case_sensitive){
    bool r = 1;
    String *a = absolutes->a;
    
    bool (*match_func)(char*, String);
    bool (*match_part_func)(char*, String);
    
    if (case_sensitive){
        match_func = match;
        match_part_func = match_part;
    }
    else{
        match_func = match_unsensitive;
        match_part_func = match_part_unsensitive;
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
    return r;
}

FCPP_LINK bool
wildcard_match(Absolutes *absolutes, String x, int case_sensitive){
    terminate_with_null(&x);
    return wildcard_match(absolutes, x.str, case_sensitive);
}

#undef FCPP_STRING_IMPLEMENTATION
#endif // #ifdef FCPP_STRING_IMPLEMENTATION

// BOTTOM

