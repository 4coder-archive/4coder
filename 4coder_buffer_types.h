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

typedef unsigned char Code;

typedef enum{
	MDFR_SHIFT_INDEX,
	MDFR_CONTROL_INDEX,
	MDFR_ALT_INDEX,
    MDFR_CAPS_INDEX,
	// always last
	MDFR_INDEX_COUNT
} Key_Control;

typedef struct Key_Event_Data{
	Code keycode;
	Code character;
	Code character_no_caps_lock;
    
	char modifiers[MDFR_INDEX_COUNT];
} Key_Event_Data;

typedef struct Mouse_State{
	char l, r;
    char press_l, press_r;
	char release_l, release_r;
	char wheel;
	char out_of_window;
	int x, y;
} Mouse_State;


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

typedef union Range{
    struct{
        int min, max;
    };
    struct{
        int start, end;
    };
} Range;

inline Range
make_range(int p1, int p2){
    Range range;
    if (p1 < p2){
        range.min = p1;
        range.max = p2;
    }
    else{
        range.min = p2;
        range.max = p1;
    }
    return(range);
}


enum Dynamic_Type{
    dynamic_type_int,
    dynamic_type_string,
    // never below this
    dynamic_type_count
};

struct Dynamic{
    int type;
    union{
        struct{
            int str_len;
            char *str_value;
        };
        int int_value;
    };
};

inline Dynamic
dynamic_int(int x){
    Dynamic result;
    result.type = dynamic_type_int;
    result.int_value = x;
    return result;
}

inline Dynamic
dynamic_string(const char *string, int len){
    Dynamic result;
    result.type = dynamic_type_string;
    result.str_len = len;
    result.str_value = (char*)(string);
    return result;
}

inline int
dynamic_to_int(Dynamic *dynamic){
    int result = 0;
    if (dynamic->type == dynamic_type_int){
        result = dynamic->int_value;
    }
    return result;
}

inline char*
dynamic_to_string(Dynamic *dynamic, int *len){
    char *result = 0;
    if (dynamic->type == dynamic_type_string){
        result = dynamic->str_value;
        *len = dynamic->str_len;
    }
    return result;
}

inline int
dynamic_to_bool(Dynamic *dynamic){
    int result = 0;
    if (dynamic->type == dynamic_type_int){
        result = (dynamic->int_value != 0);
    }
    else{
        result = 1;
    }
    return result;
}

#endif

// BOTTOM

