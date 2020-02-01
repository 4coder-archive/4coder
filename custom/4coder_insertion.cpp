/*
 * Serial inserts helpers
 */

// TOP

function Buffer_Insertion
begin_buffer_insertion_at(Application_Links *app, Buffer_ID buffer_id, i64 at){
    Buffer_Insertion result = {};
    result.app = app;
    result.buffer = buffer_id;
    result.at = at;
    return(result);
}

function Buffer_Insertion
begin_buffer_insertion_at_buffered(Application_Links *app, Buffer_ID buffer_id, i64 at, Cursor *cursor){
    Buffer_Insertion result = begin_buffer_insertion_at(app, buffer_id, at);
    result.buffering = true;
    result.cursor = cursor;
    result.temp = begin_temp(cursor);
    return(result);
}

function Buffer_Insertion
begin_buffer_insertion_at_buffered(Application_Links *app, Buffer_ID buffer_id, i64 at, Arena *buffer_memory, u64 buffer_memory_size){
    Cursor *cursor = push_array(buffer_memory, Cursor, 1);
    *cursor = make_cursor(push_array(buffer_memory, u8, buffer_memory_size), buffer_memory_size);
    return(begin_buffer_insertion_at_buffered(app, buffer_id, at, cursor));
}

function Buffer_Insertion
begin_buffer_insertion(Application_Links *app){
    View_ID view = get_active_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    i64 cursor_pos = view_get_cursor_pos(app, view);
    Buffer_Insertion result = begin_buffer_insertion_at(app, buffer, cursor_pos);
    return(result);
}

function void
insert_string__no_buffering(Buffer_Insertion *insertion, String_Const_u8 string){
    buffer_replace_range(insertion->app, insertion->buffer, Ii64(insertion->at), string);
    insertion->at += string.size;
}

function void
insert__flush(Buffer_Insertion *insertion){
    Cursor *cursor = insertion->cursor;
    u64 pos = insertion->temp.temp_memory_cursor.pos;
    String_Const_u8 string = SCu8(cursor->base + pos, cursor->pos - pos);
    insert_string__no_buffering(insertion, string);
    end_temp(insertion->temp);
}

function char*
insert__reserve(Buffer_Insertion *insertion, u64 size){
    char *space = push_array(insertion->cursor, char, size);
    if (space == 0){
        insert__flush(insertion);
        space = push_array(insertion->cursor, char, size);
    }
    return(space);
}

function void
end_buffer_insertion(Buffer_Insertion *insertion){
    if (insertion->buffering){
        insert__flush(insertion);
    }
}

function void
insert_string(Buffer_Insertion *insertion, String_Const_u8 string){
    if (!insertion->buffering){
        insert_string__no_buffering(insertion, string);
    }
    else{
        char *space = insert__reserve(insertion, string.size);
        if (space != 0){
            block_copy(space, string.str, string.size);
        }
        else{
            insert_string__no_buffering(insertion, string);
        }
    }
}

function u64
insertf(Buffer_Insertion *insertion, char *format, ...){
    Scratch_Block scratch(insertion->app);
    va_list args;
    va_start(args, format);
    String_Const_u8 string = push_u8_stringfv(scratch, format, args);
    va_end(args);
    insert_string(insertion, string);
    return(string.size);
}

function void
insertc(Buffer_Insertion *insertion, char C){
    insert_string(insertion, SCu8(&C, 1));
}

function b32
insert_line_from_buffer(Buffer_Insertion *insertion, Buffer_ID buffer_id, i32 line, i32 truncate_at){
    b32 success = is_valid_line(insertion->app, buffer_id, line);
    if (success){
        Scratch_Block scratch(insertion->app);
        insert_string(insertion, push_buffer_line(insertion->app, scratch, buffer_id, line));
    }
    return(success);
}

function b32
insert_line_from_buffer(Buffer_Insertion *insertion, Buffer_ID buffer_id, i32 line){
    return(insert_line_from_buffer(insertion, buffer_id, line, 0));
}

// BOTTOM

