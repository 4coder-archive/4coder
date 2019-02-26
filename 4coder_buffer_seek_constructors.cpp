/* 
 * Buffer seek descriptor constructors.
 */

// TOP

static Buffer_Seek
seek_pos(i32 pos){
    Buffer_Seek result;
    result.type = buffer_seek_pos;
    result.pos = pos;
    return(result);
}

static Buffer_Seek
seek_character_pos(i32 pos){
    Buffer_Seek result;
    result.type = buffer_seek_character_pos;
    result.pos = pos;
    return(result);
}

static Buffer_Seek
seek_wrapped_xy(f32 x, f32 y, b32 round_down){
    Buffer_Seek result;
    result.type = buffer_seek_wrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

static Buffer_Seek
seek_unwrapped_xy(f32 x, f32 y, b32 round_down){
    Buffer_Seek result;
    result.type = buffer_seek_unwrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

static Buffer_Seek
seek_xy(f32 x, f32 y, b32 round_down, b32 unwrapped){
    Buffer_Seek result;
    result.type = unwrapped?buffer_seek_unwrapped_xy:buffer_seek_wrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

static Buffer_Seek
seek_line_char(i32 line, i32 character){
    Buffer_Seek result;
    result.type = buffer_seek_line_char;
    result.line = line;
    result.character = character;
    return(result);
}

// BOTTOM

