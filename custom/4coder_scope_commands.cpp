/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

function void
select_next_scope_after_pos(Application_Links *app, View_ID view, Buffer_ID buffer,
                            i64 pos){
    Find_Nest_Flag flags = FindNest_Scope;
    Range_i64 range = {};
    if (find_nest_side(app, buffer, pos + 1, flags, Scan_Forward, NestDelim_Open,
                       &range) &&
        find_nest_side(app, buffer, range.end,
                       flags|FindNest_Balanced|FindNest_EndOfToken, Scan_Forward,
                       NestDelim_Close, &range.end)){
        select_scope(app, view, range);
    }
}

function b32
range_is_scope_selection(Application_Links *app, Buffer_ID buffer, Range_i64 range){
    return (buffer_get_char(app, buffer, range.min) == '{' &&
            buffer_get_char(app, buffer, range.max - 1) == '}');
}

CUSTOM_COMMAND_SIG(select_surrounding_scope)
CUSTOM_DOC("Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Range_i64 range = {};
    if (find_surrounding_nest(app, buffer, pos, FindNest_Scope, &range)){
        select_scope(app, view, range);
    }
}

CUSTOM_COMMAND_SIG(select_surrounding_scope_maximal)
CUSTOM_DOC("Selects the top-most scope that surrounds the cursor.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Range_i64 range = {};
    if (find_surrounding_nest(app, buffer, pos, FindNest_Scope, &range)){
        for (;;){
            pos = range.min;
            if (!find_surrounding_nest(app, buffer, pos, FindNest_Scope, &range)){
                break;
            }
        }
        select_scope(app, view, range);
    }
}

CUSTOM_COMMAND_SIG(select_next_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    select_next_scope_after_pos(app, view, buffer, pos);
}

CUSTOM_COMMAND_SIG(select_next_scope_after_current)
CUSTOM_DOC("If a scope is selected, find first scope that starts after the selected scope. Otherwise find the first scope that starts after the cursor.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 cursor_pos = view_get_cursor_pos(app, view);
    i64 mark_pos = view_get_mark_pos(app, view);
    Range_i64 range = Ii64(cursor_pos, mark_pos);
    if (range_is_scope_selection(app, buffer, range)){
        select_next_scope_after_pos(app, view, buffer, range.max);
    }
    else{
        select_next_scope_after_pos(app, view, buffer, cursor_pos);
    }
}

CUSTOM_COMMAND_SIG(select_prev_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Find_Nest_Flag flags = FindNest_Scope;
    Range_i64 range = {};
    if (find_nest_side(app, buffer, pos - 1,
                       flags, Scan_Backward, NestDelim_Open, &range) &&
        find_nest_side(app, buffer, range.end,
                       flags|FindNest_Balanced|FindNest_EndOfToken, Scan_Forward,
                       NestDelim_Close, &range.end)){
        select_scope(app, view, range);
    }
}

CUSTOM_COMMAND_SIG(select_prev_top_most_scope)
CUSTOM_DOC("Finds the first scope that starts before the cursor, then finds the top most scope that contains that scope.")
{
    select_prev_scope_absolute(app);
    select_surrounding_scope_maximal(app);
}

CUSTOM_COMMAND_SIG(place_in_scope)
CUSTOM_DOC("Wraps the code contained in the range between cursor and mark with a new curly brace scope.")
{
    place_begin_and_end_on_own_lines(app, "{", "}");
}

CUSTOM_COMMAND_SIG(delete_current_scope)
CUSTOM_DOC("Deletes the braces surrounding the currently selected scope.  Leaves the contents within the scope.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    
    Range_i64 range = get_view_range(app, view);
    if (range_is_scope_selection(app, buffer, range)){
        i32 top_len = 1;
        i32 bot_len = 1;
        if (buffer_get_char(app, buffer, range.min - 1) == '\n'){
            top_len = 2;
        }
        if (buffer_get_char(app, buffer, range.max + 1) == '\n'){
            bot_len = 2;
        }
        
        Batch_Edit batch_first = {};
        Batch_Edit batch_last = {};
        
        batch_first.edit.text = SCu8();
        batch_first.edit.range = Ii64(range.min + 1 - top_len, range.min + 1);
        batch_first.next = &batch_last;
        batch_last.edit.text = SCu8();
        batch_last.edit.range = Ii64((i32)(range.max - 1), (i32)(range.max - 1 + bot_len));
        
        buffer_batch_edit(app, buffer, &batch_first);
    }
}

// BOTTOM

