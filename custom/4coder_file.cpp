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
file_load_all(Arena *arena, FILE *file){
    fseek(file, 0, SEEK_END);
    u64 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    u8 *buffer = push_array(arena, u8, size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = 0;
    return(SCu8(buffer, size));
}

// BOTTOM

