// old string api

// TOP

#if !defined(OLD_STRING_API_H)
#define OLD_STRING_API_H

struct String_OLD{
    char *str;
    i32 size;
    i32 memory_size;
};

static String null_string = {};

FSTRING_INLINE b32_4tech           char_is_slash(char c);
FSTRING_INLINE b32_4tech           char_is_upper(char c);
FSTRING_INLINE b32_4tech           char_is_upper_utf8(char c);
FSTRING_INLINE b32_4tech           char_is_lower(char c);
FSTRING_INLINE b32_4tech           char_is_lower_utf8(u8_4tech c);
FSTRING_INLINE char                char_to_upper(char c);
FSTRING_INLINE char                char_to_lower(char c);
FSTRING_INLINE b32_4tech           char_is_whitespace(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_numeric(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_numeric_utf8(u8_4tech c);
FSTRING_INLINE b32_4tech           char_is_alpha_numeric_true(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_numeric_true_utf8(u8_4tech c);
FSTRING_INLINE b32_4tech           char_is_alpha(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_utf8(u8_4tech c);
FSTRING_INLINE b32_4tech           char_is_alpha_true(char c);
FSTRING_INLINE b32_4tech           char_is_alpha_true_utf8(u8_4tech c);
FSTRING_INLINE b32_4tech           char_is_hex(char c);
FSTRING_INLINE b32_4tech           char_is_hex_utf8(u8_4tech c);
FSTRING_INLINE b32_4tech           char_is_numeric(char c);
FSTRING_INLINE b32_4tech           char_is_numeric_utf8(u8_4tech c);
FSTRING_INLINE String              make_string_cap(void *str, i32_4tech size, i32_4tech mem_size);
FSTRING_INLINE String              make_string(void *str, i32_4tech size);
#ifndef   make_lit_string
# define make_lit_string(s) (make_string_cap((char*)(s), sizeof(s)-1, sizeof(s)))
#endif
#ifndef   lit
# define lit(s) make_lit_string(s)
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
FSTRING_LINK b32_4tech             copy_checked_cs(char *dest, i32_4tech dest_cap, String src);
FSTRING_LINK b32_4tech             copy_partial_sc(String *dest, char *src);
FSTRING_LINK b32_4tech             copy_partial_ss(String *dest, String src);
FSTRING_LINK b32_4tech             copy_partial_cs(char *dest, i32_4tech dest_cap, String src);
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
FSTRING_LINK void                  string_interpret_escapes(String src, char *dst);
FSTRING_LINK void                  replace_char(String *str, char replace, char with);
FSTRING_LINK void                  replace_str_ss(String *str, String replace, String with);
FSTRING_LINK void                  replace_str_sc(String *str, String replace, char *with);
FSTRING_LINK void                  replace_str_cs(String *str, char *replace, String with);
FSTRING_LINK void                  replace_str_cc(String *str, char *replace, char *with);
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
FSTRING_LINK String                string_push(Partition *part, i32_4tech size);
FSTRING_LINK String                string_push_copy(Partition *part, String str);

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
FSTRING_LINK b32_4tech             copy_checked(char *dest, i32_4tech dest_cap, String src){return(copy_checked_cs(dest,dest_cap,src));}
FSTRING_LINK b32_4tech             copy_partial(String *dest, char *src){return(copy_partial_sc(dest,src));}
FSTRING_LINK b32_4tech             copy_partial(String *dest, String src){return(copy_partial_ss(dest,src));}
FSTRING_LINK b32_4tech             copy_partial(char *dest, i32_4tech dest_cap, String src){return(copy_partial_cs(dest,dest_cap,src));}
FSTRING_INLINE i32_4tech           copy(char *dest, char *src){return(copy_cc(dest,src));}
FSTRING_INLINE void                copy(String *dest, String src){return(copy_ss(dest,src));}
FSTRING_INLINE void                copy(String *dest, char *src){return(copy_sc(dest,src));}
FSTRING_LINK b32_4tech             append_checked(String *dest, String src){return(append_checked_ss(dest,src));}
FSTRING_LINK b32_4tech             append_partial(String *dest, char *src){return(append_partial_sc(dest,src));}
FSTRING_LINK b32_4tech             append_partial(String *dest, String src){return(append_partial_ss(dest,src));}
FSTRING_LINK b32_4tech             append(String *dest, char c){return(append_s_char(dest,c));}
FSTRING_INLINE b32_4tech           append(String *dest, String src){return(append_ss(dest,src));}
FSTRING_INLINE b32_4tech           append(String *dest, char *src){return(append_sc(dest,src));}
FSTRING_LINK void                  replace_str(String *str, String replace, String with){return(replace_str_ss(str,replace,with));}
FSTRING_LINK void                  replace_str(String *str, String replace, char *with){return(replace_str_sc(str,replace,with));}
FSTRING_LINK void                  replace_str(String *str, char *replace, String with){return(replace_str_cs(str,replace,with));}
FSTRING_LINK void                  replace_str(String *str, char *replace, char *with){return(replace_str_cc(str,replace,with));}
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

// BOTTOM
