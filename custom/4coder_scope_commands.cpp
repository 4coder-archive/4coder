/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

function Nest_Delimiter_Kind
get_nest_delimiter_kind(Token_Base_Kind kind, Find_Scope_Flag flags){
    Nest_Delimiter_Kind result = NestDelimiterKind_None;
    switch (kind){
        case TokenBaseKind_ScopeOpen:
        {
            if (HasFlag(flags, FindScope_Scope)){
                result = NestDelimiterKind_Open;
            }
        }break;
        case TokenBaseKind_ScopeClose:
        {
            if (HasFlag(flags, FindScope_Scope)){
                result = NestDelimiterKind_Close;
            }
        }break;
        case TokenBaseKind_ParentheticalOpen:
        {
            if (HasFlag(flags, FindScope_Paren)){
                result = NestDelimiterKind_Open;
            }
        }break;
        case TokenBaseKind_ParentheticalClose:
        {
            if (HasFlag(flags, FindScope_Paren)){
                result = NestDelimiterKind_Close;
            }
        }break;
    }
    return(result);
}

static b32
find_scope_top(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    b32 success = false;
    Token_Array array = get_token_array_from_buffer(app, buffer);
    if (array.tokens != 0){
        i64 position = start_pos;
        i64 token_index = token_index_from_pos(&array, start_pos);
        Token_Iterator_Array it = token_iterator_index(buffer, &array, token_index);
        b32 good_status = true;
        if (HasFlag(flags, FindScope_Parent)){
            good_status = token_it_dec(&it);
        }
        i32 nest_level = 0;
        for (;good_status;){
            Token *token = token_it_read(&it);
            Nest_Delimiter_Kind delim = get_nest_delimiter_kind(token->kind, flags);
            switch (delim){
                case NestDelimiterKind_Open:
                {
                    if (nest_level == 0){
                        success = true;
                        position = token->pos;
                        if (HasFlag(flags, FindScope_EndOfToken)){
                            position += token->size;
                        }
                        goto finished;
                    }
                    else{
                        --nest_level;
                    }
                }break;
                case NestDelimiterKind_Close:
                {
                    ++nest_level;
                }break;
            }
            good_status = token_it_dec(&it);
        }
        finished:;
        *end_pos_out = position;
    }
    return(success);
}

static b32
find_scope_bottom(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    b32 success = false;
    Token_Array array = get_token_array_from_buffer(app, buffer);
    if (array.tokens != 0){
        i64 position = start_pos;
        i64 token_index = token_index_from_pos(&array, start_pos);
        Token_Iterator_Array it = token_iterator_index(buffer, &array, token_index);
        token_it_inc(&it);
        if (HasFlag(flags, FindScope_Parent)){
            token_it_dec(&it);
        }
        b32 good_status = true;
        i32 nest_level = 0;
        for (;good_status;){
            Token *token = token_it_read(&it);
            Nest_Delimiter_Kind delim = get_nest_delimiter_kind(token->kind, flags);
            switch (delim){
                case NestDelimiterKind_Open:
                {
                    ++nest_level;
                }break;
                case NestDelimiterKind_Close:
                {
                    if (nest_level == 0){
                        success = true;
                        position = token->pos;
                        if (HasFlag(flags, FindScope_EndOfToken)){
                            position += token->size;
                        }
                        goto finished;
                    }
                    else{
                        --nest_level;
                    }
                }break;
            }
            good_status = token_it_inc(&it);
        }
        finished:;
        *end_pos_out = position;
    }
    return(success);
}

static b32
find_next_scope(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    b32 success = false;
    Token_Array array = get_token_array_from_buffer(app, buffer);
    if (array.tokens != 0){
        i64 position = start_pos;
        i64 token_index = token_index_from_pos(&array, start_pos);
        Token_Iterator_Array it = token_iterator_index(buffer, &array, token_index);
        token_it_inc(&it);
        if (HasFlag(flags, FindScope_NextSibling)){
            b32 good_status = true;
            i32 nest_level = 1;
            for (;good_status;){
                Token *token = token_it_read(&it);
                Nest_Delimiter_Kind delim = get_nest_delimiter_kind(token->kind, flags);
                switch (delim){
                    case NestDelimiterKind_Open:
                    {
                        if (nest_level == 0){
                            success = true;
                            position = token->pos;
                            if (HasFlag(flags, FindScope_EndOfToken)){
                                position += token->size;
                            }
                            goto finished;
                        }
                        else{
                            ++nest_level;
                        }
                    }break;
                    case NestDelimiterKind_Close:
                    {
                        --nest_level;
                        if (nest_level == -1){
                            position = start_pos;
                            goto finished;
                        }
                    }break;
                }
                good_status = token_it_inc(&it);
            }
        }
        else{
            b32 good_status = true;
            for (;good_status;){
                Token *token = token_it_read(&it);
                Nest_Delimiter_Kind delim = get_nest_delimiter_kind(token->kind, flags);
                if (delim == NestDelimiterKind_Open){
                    success = true;
                    position = token->pos;
                    if (flags & FindScope_EndOfToken){
                        position += token->size;
                    }
                    goto finished;
                }
                good_status = token_it_inc(&it);
            }
        }
        finished:;
        *end_pos_out = position;
    }
    return(success);
}

static b32
find_prev_scope(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    b32 success = false;
    Token_Array array = get_token_array_from_buffer(app, buffer);
    if (array.tokens != 0){
        i64 position = start_pos;
        i64 token_index = token_index_from_pos(&array, start_pos);
        Token_Iterator_Array it = token_iterator_index(buffer, &array, token_index);
        if (HasFlag(flags, FindScope_NextSibling)){
            b32 status_good = token_it_dec(&it);
            i32 nest_level = -1;
            for (;status_good;){
                Token *token = token_it_read(&it);
                Nest_Delimiter_Kind delim = get_nest_delimiter_kind(token->kind, flags);
                switch (delim){
                    case NestDelimiterKind_Open:
                    {
                        if (nest_level == -1){
                            position = start_pos;
                            goto finished;
                        }
                        else if (nest_level == 0){
                            success = true;
                            position = token->pos;
                            if (HasFlag(flags, FindScope_EndOfToken)){
                                position += token->size;
                            }
                            goto finished;
                        }
                        else{
                            --nest_level;
                        }
                    }break;
                    case NestDelimiterKind_Close:
                    {
                        ++nest_level;
                    }break;
                }
                status_good = token_it_dec(&it);
            }
        }
        else{
            b32 status_good = token_it_dec(&it);
            for (;status_good;){
                Token *token = token_it_read(&it);
                Nest_Delimiter_Kind delim = get_nest_delimiter_kind(token->kind, flags);
                if (delim == NestDelimiterKind_Open){
                    success = true;
                    position = token->pos;
                    if (HasFlag(flags, FindScope_EndOfToken)){
                        position += token->size;
                    }
                    goto finished;
                }
                status_good = token_it_dec(&it);
            }
        }
        finished:;
        *end_pos_out = position;
    }
    return(success);
}

static b32
find_scope_range(Application_Links *app, Buffer_ID buffer, i64 start_pos, Range_i64 *range_out, u32 flags){
    b32 result = false;
    Range_i64 range = {};
    if (find_scope_top(app, buffer, start_pos, FindScope_Parent|flags, &range.start)){
        if (find_scope_bottom(app, buffer, start_pos, FindScope_Parent|FindScope_EndOfToken|flags, &range.end)){
            *range_out = range;
            result = true;
        }
    }
    return(result);
}

static void
view_set_to_region(Application_Links *app, View_ID view, i64 major_pos, i64 minor_pos){
    Range_i64 range = Ii64(major_pos, minor_pos);
    b32 bottom_major = false;
    if (major_pos == range.max){
        bottom_major = true;
    }
    
    Buffer_Cursor top = view_compute_cursor(app, view, seek_pos(range.min));
    if (top.line > 0){
        Buffer_Cursor bottom = view_compute_cursor(app, view, seek_pos(range.max));
        if (bottom.line > 0){
            Rect_f32 region = view_get_buffer_region(app, view);
            f32 view_height = rect_height(region);
            f32 skirt_height = view_height*.1f;
            Interval_f32 acceptable_y = If32(skirt_height, view_height*.9f);
            
            f32 target_height = view_line_y_difference(app, view, bottom.line, top.line);
            
            if (target_height > view_height){
                i64 major_line = bottom.line;
                if (range.min == major_pos){
                    major_line = top.line;
                }
                
                Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
                scroll.target.line_number = major_line;
                scroll.target.pixel_shift.y = -skirt_height;
                view_set_buffer_scroll(app, view, scroll);
            }
            else{
                Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
                Vec2_f32 top_p = view_relative_xy_of_pos(app, view, scroll.position.line_number, range.min);
                top_p -= scroll.position.pixel_shift;
                if (top_p.y < acceptable_y.min){
                    scroll.target.line_number = top.line;
                    scroll.target.pixel_shift.y = -skirt_height;
                    view_set_buffer_scroll(app, view, scroll);
                }
                else{
                    Vec2_f32 bot_p = view_relative_xy_of_pos(app, view, scroll.position.line_number, range.max);
                    bot_p -= scroll.position.pixel_shift;
                    if (bot_p.y > acceptable_y.max){
                        scroll.target.line_number = bottom.line;
                        scroll.target.pixel_shift.y = skirt_height - view_height;
                        view_set_buffer_scroll(app, view, scroll);
                    }
                }
            }
        }
    }
}

CUSTOM_COMMAND_SIG(select_surrounding_scope)
CUSTOM_DOC("Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    Range_i64 range = {};
    if (find_scope_range(app, buffer, pos, &range, FindScope_Scope)){
        view_set_cursor_and_preferred_x(app, view, seek_pos(range.first));
        view_set_mark(app, view, seek_pos(range.end));
        view_set_to_region(app, view, range.first, range.end);
        no_mark_snap_to_cursor(app, view);
    }
}

CUSTOM_COMMAND_SIG(select_next_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    i64 start_pos = pos;
    i64 top = 0;
    i64 bottom = 0;
    if (find_next_scope(app, buffer, start_pos, FindScope_Scope, &top)){
        if (find_scope_bottom(app, buffer, top, FindScope_EndOfToken|FindScope_Scope, &bottom)){
            view_set_cursor_and_preferred_x(app, view, seek_pos(top));
            view_set_mark(app, view, seek_pos(bottom));
            view_set_to_region(app, view, top, bottom);
            no_mark_snap_to_cursor(app, view);
        }
    }
}

CUSTOM_COMMAND_SIG(select_prev_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = get_active_view(app, AccessProtected);
    Buffer_ID buffer = view_get_buffer(app, view, AccessProtected);
    i64 pos = view_get_cursor_pos(app, view);
    i64 start_pos = pos;
    i64 top = 0;
    i64 bottom = 0;
    if (find_prev_scope(app, buffer, start_pos, FindScope_Scope, &top)){
        if (find_scope_bottom(app, buffer, top, FindScope_EndOfToken|FindScope_Scope, &bottom)){
            view_set_cursor_and_preferred_x(app, view, seek_pos(top));
            view_set_mark(app, view, seek_pos(bottom));
            view_set_to_region(app, view, top, bottom);
            no_mark_snap_to_cursor(app, view);
        }
    }
}

static void
place_begin_and_end_on_own_lines(Application_Links *app, char *begin, char *end){
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    
    Range_i64 range = get_view_range(app, view);
    Range_i64 lines = get_line_range_from_pos_range(app, buffer, range);
    range = get_pos_range_from_line_range(app, buffer, lines);
    
    Scratch_Block scratch(app);
    
    b32 min_line_blank = line_is_valid_and_blank(app, buffer, lines.min);
    b32 max_line_blank = line_is_valid_and_blank(app, buffer, lines.max);
    
    if ((lines.min < lines.max) || (!min_line_blank)){
        String_Const_u8 begin_str = {};
        String_Const_u8 end_str = {};
        
        i64 min_adjustment = 0;
        i64 max_adjustment = 0;
        
        if (min_line_blank){
            begin_str = push_u8_stringf(scratch, "\n%s", begin);
            min_adjustment += 1;
        }
        else{
            begin_str = push_u8_stringf(scratch, "%s\n", begin);
        }
        if (max_line_blank){
            end_str = push_u8_stringf(scratch, "%s\n", end);
        }
        else{
            end_str = push_u8_stringf(scratch, "\n%s", end);
            max_adjustment += 1;
        }
        
        max_adjustment += begin_str.size;
        Range_i64 new_pos = Ii64(range.min + min_adjustment, range.max + max_adjustment);
        
        History_Group group = history_group_begin(app, buffer);
        buffer_replace_range(app, buffer, Ii64(range.min), begin_str);
        buffer_replace_range(app, buffer, Ii64(range.max + begin_str.size), end_str);
        history_group_end(group);
        
        set_view_range(app, view, new_pos);
    }
    else{
        String_Const_u8 str = push_u8_stringf(scratch, "%s\n\n%s", begin, end);
        buffer_replace_range(app, buffer, range, str);
        i64 center_pos = range.min + cstring_length(begin) + 1;
        view_set_cursor_and_preferred_x(app, view, seek_pos(center_pos));
        view_set_mark(app, view, seek_pos(center_pos));
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

