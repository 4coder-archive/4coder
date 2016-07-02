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

