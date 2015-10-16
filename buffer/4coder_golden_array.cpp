/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * 16.10.2015
 * 
 * Buffer data object
 *  type - Golden Array
 * 
 */

#define inline_4tech inline
#define internal_4tech static

typedef struct{
    char *data;
    int size, max;
} Buffer;

typedef struct{
    Buffer *buffer;
    char *data;
    int size;
} Buffer_Save_Loop;

inline_4tech Buffer_Save_Loop
buffer_save_loop(Buffer *buffer){
    Buffer_Save_Loop result;
    result.buffer = buffer;
    result.data = buffer->data;
    result.size = buffer->size;
    return(result);
}

inline_4tech int
buffer_save_good(Buffer_Save_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_save_next(Buffer_Save_Loop *loop){
    loop->buffer = 0;
}

internal_4tech int
buffer_count_newlines(Buffer *buffer, int start, int end, int CR, int LF){
    int new_line, count;
    char *data;
    int i;
    
    data = buffer->data;
    new_line = 0;
    count = 0;
    
    for (i = start; i < end; ++i){
        switch(data[i]){
        case '\n': new_line = LF; break;
        case '\r': new_line = CR; break;
        default: new_line = 0; break;
        }
        count += new_line;
    }
    
    return (count);
}

typedef struct{
    int i;
    int count;
    int start;
} Buffer_Measure_Starts;

internal_4tech int
buffer_measure_starts_(Buffer_Measure_Starts *state, Buffer *buffer, int *starts, int max, int CR, int LF){
    char *data;
    int size;
    int start, count, i, new_line;
    int result;
    
    data = buffer->data;
    size = buffer->size;
    
    result = 0;
    start = state->start;
    count = state->count;
    
    for (i = state->i; i < size; ++i){
        switch (data[i]){
        case '\n': new_line = LF; break;
        case '\r': new_line = CR; break;
        default: new_line = 0; break;
        }
        
        if (new_line){
            if (count == max){
                result = 1;
                break;
            }
            
            starts[count++] = start;
            start = i + 1;
        }
    }
    
    if (i == size){
        if (count == max) result = 1;
        else starts[count++] = start;
    }
    
    state->i = i;
    state->count = count;
    state->start = start;
    
    return (result);
}

