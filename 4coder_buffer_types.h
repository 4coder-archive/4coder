/* 
 * Mr. 4th Dimention - Allen Webster
 * 
 * 23.02.2016
 * 
 * Types shared by custom and application
 * 
 */

// TOP

#ifndef FRED_BUFFER_TYPES_H
#define FRED_BUFFER_TYPES_H

typedef struct Full_Cursor{
    int pos;
    int line, character;
    float unwrapped_x, unwrapped_y;
    float wrapped_x, wrapped_y;
} Full_Cursor;

typedef enum{
    buffer_seek_pos,
    buffer_seek_wrapped_xy,
    buffer_seek_unwrapped_xy,
    buffer_seek_line_char
} Buffer_Seek_Type;

typedef struct Buffer_Seek{
    Buffer_Seek_Type type;
    union{
        struct { int pos; };
        struct { int round_down; float x, y; };
        struct { int line, character; };
    };
} Buffer_Seek;

static Buffer_Seek
seek_pos(int pos){
    Buffer_Seek result;
    result.type = buffer_seek_pos;
    result.pos = pos;
    return(result);
}

static Buffer_Seek
seek_wrapped_xy(float x, float y, int round_down){
    Buffer_Seek result;
    result.type = buffer_seek_wrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

static Buffer_Seek
seek_unwrapped_xy(float x, float y, int round_down){
    Buffer_Seek result;
    result.type = buffer_seek_unwrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

static Buffer_Seek
seek_xy(float x, float y, int round_down, int unwrapped){
    Buffer_Seek result;
    result.type = unwrapped?buffer_seek_unwrapped_xy:buffer_seek_wrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

static Buffer_Seek
seek_line_char(int line, int character){
    Buffer_Seek result;
    result.type = buffer_seek_line_char;
    result.line = line;
    result.character = character;
    return(result);
}


#endif

// BOTTOM

