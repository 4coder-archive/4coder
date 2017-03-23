/*
 * Helpers for parsing jump location strings.
 */

// TOP

#if !defined(FCODER_JUMP_PARSING_H)
#define FCODER_JUMP_PARSING_H

#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_long_seek.h"

#include "4coder_lib/4coder_mem.h"

struct Name_Based_Jump_Location{
    String file;
    int32_t line;
    int32_t column;
};

static void
jump_to_location(Application_Links *app, View_Summary *view, Name_Based_Jump_Location *l){
    if (view_open_file(app, view, l->file.str, l->file.size, true)){
        view_set_cursor(app, view, seek_line_char(l->line, l->column), true);
    }
}

static int32_t
ms_style_verify(String line, int32_t paren_pos){
    int32_t result = false;
    
    String line_part = substr_tail(line, paren_pos);
    if (match_part_sc(line_part, ") : ")){
        result = true;
    }
    else if (match_part_sc(line_part, "): ")){
        result = true;
    }
    
    return(result);
}

static int32_t
parse_jump_location(String line, Name_Based_Jump_Location *location, int32_t skip_sub_errors, size_t *colon_char){
    bool32 result = false;
    
    int32_t whitespace_length = 0;
    String original_line = line;
    line = skip_chop_whitespace(line, &whitespace_length);
    
    int32_t colon_pos = 0;
    bool32 is_ms_style = false;
    
    int32_t paren_pos = find_s_char(line, 0, ')');
    while (!is_ms_style && paren_pos < line.size){
        if (ms_style_verify(line, paren_pos)){
            is_ms_style = true;
            colon_pos = find_s_char(line, paren_pos, ':');
            if (colon_pos < line.size){
                String location_str = substr(line, 0, colon_pos);
                
                if (!(skip_sub_errors && original_line.str[0] == ' ')){
                    location_str = skip_chop_whitespace(location_str);
                    
                    int32_t close_pos = paren_pos;
                    int32_t open_pos = rfind_s_char(location_str, close_pos, '(');
                    
                    if (0 < open_pos && open_pos < location_str.size){
                        String file = substr(location_str, 0, open_pos);
                        file = skip_chop_whitespace(file);
                        
                        if (file.size > 0){
                            String line_number = substr(location_str,
                                                        open_pos+1,
                                                        close_pos-open_pos-1);
                            line_number = skip_chop_whitespace(line_number);
                            
                            if (line_number.size > 0){
                                location->file = file;
                                
                                int32_t comma_pos = find_s_char(line_number, 0, ',');
                                if (comma_pos < line_number.size){
                                    int32_t start = comma_pos+1;
                                    String column_number = substr(line_number, start, line_number.size-start);
                                    line_number = substr(line_number, 0, comma_pos);
                                    
                                    location->line = str_to_int_s(line_number);
                                    location->column = str_to_int_s(column_number);
                                }
                                else{
                                    location->line = str_to_int_s(line_number);
                                    location->column = 1;
                                }
                                
                                *colon_char = colon_pos + whitespace_length;
                                result = true;
                            }
                        }
                    }
                }
            }
        }
        else{
            paren_pos = find_s_char(line, paren_pos+1, ')');
        }
    }
    
    if (!is_ms_style){
        int32_t colon_pos1 = find_s_char(line, 0, ':');
        if (line.size > colon_pos1+1){
            if (char_is_slash(line.str[colon_pos1+1])){
                colon_pos1 = find_s_char(line, colon_pos1+1, ':');
            }
        }
        
        int32_t colon_pos2 = find_s_char(line, colon_pos1+1, ':');
        int32_t colon_pos3 = find_s_char(line, colon_pos2+1, ':');
        
        if (colon_pos3+1 <= line.size){
            String filename = substr(line, 0, colon_pos1);
            String line_number = substr(line, colon_pos1+1, colon_pos2 - colon_pos1 - 1);
            String column_number = substr(line, colon_pos2+1, colon_pos3 - colon_pos2 - 1);
            
            if (filename.size > 0 &&
                line_number.size > 0 &&
                column_number.size > 0){
                location->file = filename;
                location->line = str_to_int_s(line_number);
                location->column = str_to_int_s(column_number);
                *colon_char = colon_pos3 + whitespace_length;
                result = true;
            }
        }
        else{
            if (colon_pos2+1 <= line.size){
                String filename = substr(line, 0, colon_pos1);
                String line_number = substr(line, colon_pos1+1, colon_pos2 - colon_pos1 - 1);
                
                if (str_is_int_s(line_number)){
                    if (filename.size > 0 && line_number.size > 0){
                        location->file = filename;
                        location->line = str_to_int_s(line_number);
                        location->column = 0;
                        *colon_char = colon_pos2 + whitespace_length;
                        result = true;
                    }
                }
            }
        }
    }
    
    return(result);
}

static bool32
parse_jump_from_buffer_line(Application_Links *app, Partition *part, int32_t buffer_id, size_t line, int32_t skip_sub_errors, Name_Based_Jump_Location *location){
    
    bool32 result = false;
    String line_str = {0};
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    if (read_line(app, part, &buffer, line, &line_str)){
        size_t colon_char = 0;
        if (parse_jump_location(line_str, location, skip_sub_errors, &colon_char)){
            result = true;
        }
    }
    
    return(result);
}

#endif

// BOTTOM

