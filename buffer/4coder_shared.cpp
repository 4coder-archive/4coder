/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 23.10.2015
 * 
 * Items shared by gap buffer types
 * 
 */

// TOP

#ifndef defines_4tech
#define inline_4tech inline
#define internal_4tech static
#define memset_4tech memset
#define memcpy_4tech memcpy
#define memmove_4tech memmove
#define defines_4tech 1
#define debug_4tech(x) x
#define assert_4tech assert

#define ceil_4tech CEIL32
#endif

inline_4tech float
measure_character(void *advance_data, int stride, char character){
    char *advances;
    float width;
    
    advances = (char*)advance_data;
    switch (character){
    case 0: width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * '0'); break;
    case '\n': width = 0; break;
    case '\r': width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * '\r'); break;
    default: width = *(float*)(advances + stride * character);
    }
    
    return(width);
}

typedef struct{
    int str_start, len;
    int start, end;
} Buffer_Edit;

typedef enum{
    buffer_seek_pos,
    buffer_seek_wrapped_xy,
    buffer_seek_unwrapped_xy,
    buffer_seek_line_char
} Buffer_Seek_Type;


typedef struct{
    Buffer_Seek_Type type;
    union{
        struct { int pos; };
        struct { int round_down; float x, y; };
        struct { int line, character; };
    };
} Buffer_Seek;

inline_4tech Buffer_Seek
seek_pos(int pos){
    Buffer_Seek result;
    result.type = buffer_seek_pos;
    result.pos = pos;
    return(result);
}

inline_4tech Buffer_Seek
seek_wrapped_xy(float x, float y, int round_down){
    Buffer_Seek result;
    result.type = buffer_seek_wrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

inline_4tech Buffer_Seek
seek_unwrapped_xy(float x, float y, int round_down){
    Buffer_Seek result;
    result.type = buffer_seek_unwrapped_xy;
    result.x = x;
    result.y = y;
    result.round_down = round_down;
    return(result);
}

inline_4tech Buffer_Seek
seek_line_char(int line, int character){
    Buffer_Seek result;
    result.type = buffer_seek_line_char;
    result.line = line;
    result.character = character;
    return(result);
}

typedef struct{
    int pos;
    int line, character;
    float unwrapped_x, unwrapped_y;
    float wrapped_x, wrapped_y;
} Full_Cursor;

typedef struct{
    int index;
    int glyphid;
    float x0, y0;
    float x1, y1;
} Buffer_Render_Item;

inline_4tech void
write_render_item(Buffer_Render_Item *item, int index, int glyphid,
                  float x, float y, float w, float h){
    item->index = index;
    item->glyphid = glyphid;
    item->x0 = x;
    item->y0 = y;
    item->x1 = x + w;
    item->y1 = y + h;
}

inline_4tech float
write_render_item_inline(Buffer_Render_Item *item, int index, int glyphid,
                         float x, float y, void *advance_data, int stride, float h){
    float ch_width;
    ch_width = measure_character(advance_data, stride, (char)glyphid);
    write_render_item(item, index, glyphid, x, y, ch_width, h);
    return(ch_width);
}

typedef struct{
    Full_Cursor cursor, prev_cursor;
} Seek_State;

internal_4tech int
cursor_seek_step(Seek_State *state, Buffer_Seek seek, int xy_seek, float max_width, float font_height,
                 char *advances, int stride, int size, char ch){
    Full_Cursor cursor, prev_cursor;
    float ch_width;
    int result;
    float x, px, y;
    
    cursor = state->cursor;
    prev_cursor = state->prev_cursor;
    
    result = 1;
    prev_cursor = cursor;
    switch (ch){
    case '\n':
        ++cursor.line;
        cursor.unwrapped_y += font_height;
        cursor.wrapped_y += font_height;
        cursor.character = 1;
        cursor.unwrapped_x = 0;
        cursor.wrapped_x = 0;
        break;
        
    default:
        ++cursor.character;
        if (ch == '\r') ch_width = *(float*)(advances + stride * '\\') + *(float*)(advances + stride * 'r');
        else ch_width = *(float*)(advances + stride * ch);
            
        if (cursor.wrapped_x + ch_width >= max_width){
            cursor.wrapped_y += font_height;
            cursor.wrapped_x = 0;
            prev_cursor = cursor;
        }
            
        cursor.unwrapped_x += ch_width;
        cursor.wrapped_x += ch_width;
            
        break;
    }
    
    ++cursor.pos;
    
    if (cursor.pos > size){
        cursor = prev_cursor;
        result = 0;
        goto cursor_seek_step_end;
    }
    
    x = y = px = 0;
    
    switch (seek.type){
    case buffer_seek_pos:
        if (cursor.pos > seek.pos){
            cursor = prev_cursor;
            result = 0;
            goto cursor_seek_step_end;
        }break;
        
    case buffer_seek_wrapped_xy:
        x = cursor.wrapped_x; px = prev_cursor.wrapped_x;
        y = cursor.wrapped_y; break;
        
    case buffer_seek_unwrapped_xy:
        x = cursor.unwrapped_x; px = prev_cursor.unwrapped_x;
        y = cursor.unwrapped_y; break;
        
    case buffer_seek_line_char:
        if (cursor.line == seek.line && cursor.character >= seek.character){
            result = 0;
            goto cursor_seek_step_end;
        }
        else if (cursor.line > seek.line){
            cursor = prev_cursor;
            result = 0;
            goto cursor_seek_step_end;
        }break;
    }
    
    if (xy_seek){
        if (y > seek.y){
            cursor = prev_cursor;
            result = 0;
            goto cursor_seek_step_end;
        }
        
        if (y > seek.y - font_height && x >= seek.x){
            if (!seek.round_down){
                if (ch != '\n' && (seek.x - px) < (x - seek.x)) cursor = prev_cursor;
                result = 0;
                goto cursor_seek_step_end;
            }
            
            if (x > seek.x){
                cursor = prev_cursor;
                result = 0;
                goto cursor_seek_step_end;
            }
        }
    }
    
cursor_seek_step_end:
    state->cursor = cursor;
    state->prev_cursor = prev_cursor;
    
    return(result);
}

inline_4tech int
is_whitespace(char c){
    int result;
    result = (c == ' ' || c == '\n' || c == '\r'  || c == '\t' || c == '\f' || c == '\v');
    return(result);
}

// BOTTOM

