/*
 * Serial inserts helpers
 */

// TOP

static Buffer_Insertion
begin_buffer_insertion_at(Application_Links *app, Buffer_ID buffer_id, i32 at){
    Buffer_Insertion result = {};
    result.app = app;
    result.buffer = buffer_id;
    result.at = at;
    return(result);
}

static Buffer_Summary
get_active_buffer(Application_Links *app, Access_Flag access){
    View_Summary view = get_active_view(app, access);
    Buffer_Summary result = get_buffer(app, view.buffer_id, access);
    return(result);
}

static Buffer_Insertion
begin_buffer_insertion(Application_Links *app){
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Insertion result = begin_buffer_insertion_at(app, view.buffer_id, view.cursor.pos);
    return(result);
}

static void
insert_string(Buffer_Insertion *insertion, String string){
    buffer_replace_range(insertion->app, insertion->buffer, insertion->at, insertion->at, string);
    insertion->at += string.size;
}

static i32
insertf(Buffer_Insertion *insertion, char *format, ...){
    Arena *arena = context_get_arena(insertion->app);
    Temp_Memory_Arena temp = begin_temp_memory(arena);
    va_list args;
    va_start(args, format);
    String string = string_push_fv(arena, format, args);
    va_end(args);
    insert_string(insertion, string);
    end_temp_memory(temp);
    return(string.size);
}

static void
insertc(Buffer_Insertion *insertion, char C){
    buffer_replace_range(insertion->app, insertion->buffer, insertion->at, insertion->at, make_string(&C, 1));
    insertion->at += 1;
}

static b32
insert_line_from_buffer(Buffer_Insertion *insertion, Buffer_ID buffer_id, i32 line, i32 truncate_at){
    Partition *part = &global_part;
    Temp_Memory temp = begin_temp_memory(part);
    
    Partial_Cursor begin = {};
    Partial_Cursor end = {};
    
    Buffer_Summary buffer = get_buffer(insertion->app, buffer_id, AccessAll);
    
    b32 success = false;
    if (buffer_compute_cursor(insertion->app, &buffer, seek_line_char(line, 1), &begin)){
        if (buffer_compute_cursor(insertion->app, &buffer, seek_line_char(line, -1), &end)){
            if (begin.line == line){
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer.size){
                    i32 size = (end.pos - begin.pos);
                    if(truncate_at && (size > truncate_at))
                    {
                        size = truncate_at;
                    }
                    
                    char *memory = push_array(part, char, size);
                    if (memory != 0){
                        String str = make_string(memory, 0, size);
                        success = true;
                        buffer_read_range(insertion->app, &buffer, begin.pos, end.pos, str.str);
                        str.size = size;
                        insert_string(insertion, str);
                    }
                }
            }
        }
    }
    
    end_temp_memory(temp);
    return(success);
}

static b32
insert_line_from_buffer(Buffer_Insertion *insertion, Buffer_ID buffer_id, i32 line){
    return(insert_line_from_buffer(insertion, buffer_id, line, 0));
}

// BOTTOM

