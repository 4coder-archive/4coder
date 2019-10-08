/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

function Nest_Delimiter_Kind
get_nest_delimiter_kind(Token_Base_Kind kind, Find_Nest_Flag flags){
    Nest_Delimiter_Kind result = NestDelim_None;
    switch (kind){
        case TokenBaseKind_ScopeOpen:
        {
            if (HasFlag(flags, FindNest_Scope)){
                result = NestDelim_Open;
            }
        }break;
        case TokenBaseKind_ScopeClose:
        {
            if (HasFlag(flags, FindNest_Scope)){
                result = NestDelim_Close;
            }
        }break;
        case TokenBaseKind_ParentheticalOpen:
        {
            if (HasFlag(flags, FindNest_Paren)){
                result = NestDelim_Open;
            }
        }break;
        case TokenBaseKind_ParentheticalClose:
        {
            if (HasFlag(flags, FindNest_Paren)){
                result = NestDelim_Close;
            }
        }break;
    }
    return(result);
}

function b32
find_nest_side(Application_Links *app, Buffer_ID buffer, i64 pos,
               Find_Nest_Flag flags, Scan_Direction scan, Nest_Delimiter_Kind delim,
               Range_i64 *out){
    b32 result = false;
    
    b32 balanced = HasFlag(flags, FindNest_Balanced);
    if (balanced){
        if ((delim == NestDelim_Open && scan == Scan_Forward) ||
            (delim == NestDelim_Close && scan == Scan_Backward)){
            balanced = false;
        }
    }
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens = scope_attachment(app, scope, attachment_tokens, Token_Array);
    if (tokens != 0 && tokens->count > 0){
        Token_Iterator_Array it = token_iterator_pos(0, tokens, pos);
        i32 level = 0;
        for (;;){
            Token *token = token_it_read(&it);
            Nest_Delimiter_Kind token_delim = get_nest_delimiter_kind(token->kind, flags);
            
            if (level == 0 && token_delim == delim){
                *out = Ii64_size(token->pos, token->size);
                result = true;
                break;
            }
            
            if (balanced && token_delim != NestDelim_None){
                level += (token_delim == delim)?-1:1;
            }
            
            b32 good = false;
            if (scan == Scan_Forward){
                good = token_it_inc(&it);
            }
            else{
                good = token_it_dec(&it);
            }
            if (!good){
                break;
            }
        }
    }
    
    return(result);
}

function b32
find_nest_side(Application_Links *app, Buffer_ID buffer, i64 pos,
               Find_Nest_Flag flags, Scan_Direction scan, Nest_Delimiter_Kind delim,
               i64 *out){
    Range_i64 range = {};
    b32 result = find_nest_side(app, buffer, pos, flags, scan, delim, &range);
    if (result){
        if (HasFlag(flags, FindNest_EndOfToken)){
            *out = range.end;
        }
        else{
            *out = range.start;
        }
    }
    return(result);
}

function b32
find_surrounding_nest(Application_Links *app, Buffer_ID buffer, i64 pos,
                      Find_Nest_Flag flags, Range_i64 *out){
    b32 result = false;
    Range_i64 range = {};
    if (find_nest_side(app, buffer, pos - 1, flags|FindNest_Balanced,
                       Scan_Backward, NestDelim_Open, &range.start) &&
        find_nest_side(app, buffer, pos, flags|FindNest_Balanced|FindNest_EndOfToken,
                       Scan_Forward, NestDelim_Close, &range.end)){
        *out = range;
        result = true;
    }
    return(result);
}

function void
select_scope(Application_Links *app, View_ID view, Range_i64 range){
    view_set_cursor_and_preferred_x(app, view, seek_pos(range.first));
    view_set_mark(app, view, seek_pos(range.end));
    view_look_at_region(app, view, range.first, range.end);
    no_mark_snap_to_cursor(app, view);
}

CUSTOM_COMMAND_SIG(select_surrounding_scope)
CUSTOM_DOC("Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    Range_i64 range = {};
    if (find_surrounding_nest(app, buffer, pos, FindNest_Scope, &range)){
        select_scope(app, view, range);
    }
}

function void
select_next_scope_after_pos(Application_Links *app, View_ID view, Buffer_ID buffer, i64 pos){
    Find_Nest_Flag flags = FindNest_Scope;
    Range_i64 range = {};
    if (find_nest_side(app, buffer, pos + 1,
                       flags, Scan_Forward, NestDelim_Open, &range) &&
        find_nest_side(app, buffer, range.end,
                       flags|FindNest_Balanced|FindNest_EndOfToken, Scan_Forward,
                       NestDelim_Close, &range.end)){
        select_scope(app, view, range);
    }
}

CUSTOM_COMMAND_SIG(select_next_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    select_next_scope_after_pos(app, view, buffer, pos);
}

CUSTOM_COMMAND_SIG(select_next_scope_after_current)
CUSTOM_DOC("Finds the first scope started by '{' after the mark and puts the cursor and mark on the '{' and '}'.  This command is meant to be used after a scope is already selected so that it will have the effect of selecting the next scope after the current scope.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_mark_pos(app, view);
    select_next_scope_after_pos(app, view, buffer, pos);
}

CUSTOM_COMMAND_SIG(select_prev_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
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

CUSTOM_COMMAND_SIG(place_in_scope)
CUSTOM_DOC("Wraps the code contained in the range between cursor and mark with a new curly brace scope.")
{
    place_begin_and_end_on_own_lines(app, "{", "}");
}

CUSTOM_COMMAND_SIG(delete_current_scope)
CUSTOM_DOC("Deletes the braces surrounding the currently selected scope.  Leaves the contents within the scope.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    
    Range_i64 range = get_view_range(app, view);
    if (buffer_get_char(app, buffer, range.min) == '{' &&
        buffer_get_char(app, buffer, range.max - 1) == '}'){
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

