/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.10.2019
 *
 * Basic helpers for C std file handling.
 *
 */

// TOP

#include <stdio.h>

function String_Const_u8
data_from_file(Arena *arena, FILE *file){
    String_Const_u8 result = {};
    if (file != 0){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        result.str = push_array(arena, u8, result.size + 1);
        fread(result.str, 1, (size_t)result.size, file);
        result.str[result.size] = 0;
    }
    return(result);
}

// BOTTOM

