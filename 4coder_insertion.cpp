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

static Buffer_Insertion
begin_buffer_insertion_at_buffered(Application_Links *app, Buffer_ID buffer_id, i32 at, Cursor *cursor){
    Buffer_Insertion result = begin_buffer_insertion_at(app, buffer_id, at);
    result.buffering = true;
    result.cursor = cursor;
    result.temp = begin_temp(cursor);
    return(result);
}

static Buffer_Insertion
begin_buffer_insertion(Application_Links *app){
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessAll, &buffer);
    i32 cursor_pos = 0;
    view_get_cursor_pos(app, view, &cursor_pos);
    Buffer_Insertion result = begin_buffer_insertion_at(app, buffer, cursor_pos);
    return(result);
}

static void
insert_string__no_buffering(Buffer_Insertion *insertion, String_Const_u8 string){
    buffer_replace_range(insertion->app, insertion->buffer, make_range((i32)insertion->at), string);
    insertion->at += string.size;
}

static void
insert__flush(Buffer_Insertion *insertion){
    Cursor *cursor = insertion->cursor;
    umem pos = insertion->temp.temp_memory_cursor.pos;
    String_Const_u8 string = SCu8(cursor->base + pos, cursor->pos - pos);
    insert_string__no_buffering(insertion, string);
    end_temp(insertion->temp);
}

static char*
insert__reserve(Buffer_Insertion *insertion, umem size){
    char *space = push_array(insertion->cursor, char, size);
    if (space == 0){
        insert__flush(insertion);
        space = push_array(insertion->cursor, char, size);
    }
    return(space);
}

static void
end_buffer_insertion(Buffer_Insertion *insertion){
    if (insertion->buffering){
        insert__flush(insertion);
    }
}

static void
insert_string(Buffer_Insertion *insertion, String_Const_u8 string){
    if (!insertion->buffering){
        insert_string__no_buffering(insertion, string);
    }
    else{
        char *space = insert__reserve(insertion, string.size);
        if (space != 0){
            memcpy(space, string.str, string.size);
        }
        else{
            insert_string__no_buffering(insertion, string);
        }
    }
}

static umem
insertf(Buffer_Insertion *insertion, char *format, ...){
    Arena *scratch = context_get_arena(insertion->app);
    Temp_Memory temp = begin_temp(scratch);
    va_list args;
    va_start(args, format);
    String_Const_u8 string = string_u8_pushfv(scratch, format, args);
    va_end(args);
    insert_string(insertion, string);
    end_temp(temp);
    return(string.size);
}

static void
insertc(Buffer_Insertion *insertion, char C){
    insert_string(insertion, SCu8(&C, 1));
}

static b32
insert_line_from_buffer(Buffer_Insertion *insertion, Buffer_ID buffer_id, i32 line, i32 truncate_at){
    Scratch_Block scratch(insertion->app);
    
    Partial_Cursor begin = {};
    Partial_Cursor end = {};
    
    b32 success = false;
    if (buffer_compute_cursor(insertion->app, buffer_id, seek_line_char(line, 1), &begin)){
        if (buffer_compute_cursor(insertion->app, buffer_id, seek_line_char(line, -1), &end)){
            if (begin.line == line){
                i32 buffer_size = 0;
                buffer_get_size(insertion->app, buffer_id, &buffer_size);
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer_size){
                    i32 size = (end.pos - begin.pos);
                    if (truncate_at && (size > truncate_at)){
                        size = truncate_at;
                    }
                    
                    String_Const_u8 string = scratch_read(insertion->app, scratch, buffer_id, begin.pos, end.pos);
                    insert_string(insertion, string);
                    success = true;
                }
            }
        }
    }
    
    return(success);
}

static b32
insert_line_from_buffer(Buffer_Insertion *insertion, Buffer_ID buffer_id, i32 line){
    return(insert_line_from_buffer(insertion, buffer_id, line, 0));
}

// BOTTOM

