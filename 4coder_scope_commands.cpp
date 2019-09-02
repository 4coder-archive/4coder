/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

static b32 parse_statement_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out);

////////////////////////////////

static Find_Scope_Token_Type
find_scope_get_token_type(u32 flags, Cpp_Token_Type token_type){
    Find_Scope_Token_Type type = FindScopeTokenType_None;
    if (flags & FindScope_Brace){
        switch (token_type){
            case CPP_TOKEN_BRACE_OPEN:
            {
                type = FindScopeTokenType_Open;
            }break;
            case CPP_TOKEN_BRACE_CLOSE:
            {
                type = FindScopeTokenType_Close;
            }break;
        }
    }
    if (flags & FindScope_Paren){
        switch (token_type){
            case CPP_TOKEN_PARENTHESE_OPEN:
            {
                type = FindScopeTokenType_Open;
            }break;
            case CPP_TOKEN_PARENTHESE_CLOSE:
            {
                type = FindScopeTokenType_Close;
            }break;
        }
    }
    return(type);
}

static b32
find_scope_top(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = false;
    i32 position = 0;
    if (get_token_from_pos(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index;
        if (HasFlag(flags, FindScope_Parent)){
            --token_index;
            if (get_result.in_whitespace_after_token){
                ++token_index;
            }
        }
        if (token_index >= 0){
            Token_Range token_range = buffer_get_token_range(app, buffer);
            if (token_range.first != 0){
                Token_Iterator token_it = make_token_iterator(token_range, token_index);
                i32 nest_level = 0;
                for (Cpp_Token *token = token_iterator_current(&token_it);
                     token != 0;
                     token = token_iterator_goto_prev(&token_it)){
                    Find_Scope_Token_Type type = find_scope_get_token_type(flags, token->type);
                    switch (type){
                        case FindScopeTokenType_Open:
                        {
                            if (nest_level == 0){
                                success = true;
                                position = token->start;
                                if (flags & FindScope_EndOfToken){
                                    position += token->size;
                                }
                                goto finished;
                            }
                            else{
                                --nest_level;
                            }
                        }break;
                        case FindScopeTokenType_Close:
                        {
                            ++nest_level;
                        }break;
                    }
                }
            }
        }
    }
    finished:;
    *end_pos_out = position;
    return(success);
}

static b32
find_scope_bottom(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = false;
    i32 position = 0;
    if (get_token_from_pos(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index + 1;
        if (HasFlag(flags, FindScope_Parent)){
            --token_index;
            if (get_result.in_whitespace_after_token){
                ++token_index;
            }
        }
        if (token_index >= 0){
            Token_Range token_range = buffer_get_token_range(app, buffer);
            if (token_range.first != 0){
                Token_Iterator token_it = make_token_iterator(token_range, token_index);
                i32 nest_level = 0;
                for (Cpp_Token *token = token_iterator_current(&token_it);
                     token != 0;
                     token = token_iterator_goto_next(&token_it)){
                    Find_Scope_Token_Type type = find_scope_get_token_type(flags, token->type);
                    switch (type){
                        case FindScopeTokenType_Open:
                        {
                            ++nest_level;
                        }break;
                        case FindScopeTokenType_Close:
                        {
                            if (nest_level == 0){
                                success = true;
                                position = token->start;
                                if (flags & FindScope_EndOfToken){
                                    position += token->size;
                                }
                                goto finished;
                            }
                            else{
                                --nest_level;
                            }
                        }break;
                    }
                }
            }
        }
    }
    finished:;
    *end_pos_out = position;
    return(success);
}

static b32
find_next_scope(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = 0;
    i64 position = 0;
    if (get_token_from_pos(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index + 1;
        if (token_index >= 0){
            Token_Range token_range = buffer_get_token_range(app, buffer);
            if (token_range.first != 0){
                Token_Iterator token_it = make_token_iterator(token_range, token_index);
                if (HasFlag(flags, FindScope_NextSibling)){
                    i32 nest_level = 1;
                    for (Cpp_Token *token = token_iterator_current(&token_it);
                         token != 0;
                         token = token_iterator_goto_next(&token_it)){
                        Find_Scope_Token_Type type = find_scope_get_token_type(flags, token->type);
                        switch (type){
                            case FindScopeTokenType_Open:
                            {
                                if (nest_level == 0){
                                    success = 1;
                                    position = token->start;
                                    if (flags & FindScope_EndOfToken){
                                        position += token->size;
                                    }
                                    goto finished;
                                }
                                else{
                                    ++nest_level;
                                }
                            }break;
                            case FindScopeTokenType_Close:
                            {
                                --nest_level;
                                if (nest_level == -1){
                                    position = start_pos;
                                    goto finished;
                                }
                            }break;
                        }
                    }
                }
                else{
                    for (Cpp_Token *token = token_iterator_current(&token_it);
                         token != 0;
                         token = token_iterator_goto_next(&token_it)){
                        Find_Scope_Token_Type type = find_scope_get_token_type(flags, token->type);
                        if (type == FindScopeTokenType_Open){
                            success = 1;
                            position = token->start;
                            if (flags & FindScope_EndOfToken){
                                position += token->size;
                            }
                            goto finished;
                        }
                    }
                }
            }
        }
    }
    finished:;
    *end_pos_out = position;
    return(success);
}

static b32
find_prev_scope(Application_Links *app, Buffer_ID buffer, i64 start_pos, u32 flags, i64 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = false;
    i64 position = 0;
    if (get_token_from_pos(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index - 1;
        if (token_index >= 0){
            Token_Range token_range = buffer_get_token_range(app, buffer);
            if (token_range.first != 0){
                Token_Iterator token_it = make_token_iterator(token_range, token_index);
                if (flags & FindScope_NextSibling){
                    i32 nest_level = -1;
                    for (Cpp_Token *token = token_iterator_current(&token_it);
                         token != 0;
                         token = token_iterator_goto_prev(&token_it)){
                        Find_Scope_Token_Type type = find_scope_get_token_type(flags, token->type);
                        switch (type){
                            case FindScopeTokenType_Open:
                            {
                                if (nest_level == -1){
                                    position = start_pos;
                                    goto finished;
                                }
                                else if (nest_level == 0){
                                    success = true;
                                    position = token->start;
                                    if (flags & FindScope_EndOfToken){
                                        position += token->size;
                                    }
                                    goto finished;
                                }
                                else{
                                    --nest_level;
                                }
                            }break;
                            case FindScopeTokenType_Close:
                            {
                                ++nest_level;
                            }break;
                        }
                    }
                }
                else{
                    for (Cpp_Token *token = token_iterator_current(&token_it);
                         token != 0;
                         token = token_iterator_goto_prev(&token_it)){
                        Find_Scope_Token_Type type = find_scope_get_token_type(flags, token->type);
                        if (type == FindScopeTokenType_Open){
                            success = true;
                            position = token->start;
                            if (flags & FindScope_EndOfToken){
                                position += token->size;
                            }
                            goto finished;
                        }
                    }
                }
            }
        }
    }
    finished:;
    *end_pos_out = position;
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
    if (find_scope_range(app, buffer, pos, &range, FindScope_Brace)){
        view_set_cursor(app, view, seek_pos(range.first), true);
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
    if (find_next_scope(app, buffer, start_pos, FindScope_Brace, &top)){
        if (find_scope_bottom(app, buffer, top, FindScope_EndOfToken|FindScope_Brace, &bottom)){
            view_set_cursor(app, view, seek_pos(top), true);
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
    if (find_prev_scope(app, buffer, start_pos, FindScope_Brace, &top)){
        if (find_scope_bottom(app, buffer, top, FindScope_EndOfToken|FindScope_Brace, &bottom)){
            view_set_cursor(app, view, seek_pos(top), true);
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
        view_set_cursor(app, view, seek_pos(center_pos), true);
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

static Cpp_Token*
parser_next_token(Statement_Parser *parser){
    return(token_iterator_goto_next(&parser->token_iterator));
}

static b32
parse_for_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    b32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    i32 paren_level = 0;
    for (;token != 0;){
        if (!(token->flags & CPP_TFLAG_PP_BODY)){
            switch (token->type){
                case CPP_TOKEN_PARENTHESE_OPEN:
                {
                    ++paren_level;
                }break;
                
                case CPP_TOKEN_PARENTHESE_CLOSE:
                {
                    --paren_level;
                    if (paren_level == 0){
                        success = parse_statement_down(app, parser, token_out);
                        goto finished;
                    }
                    else if (paren_level < 0){
                        success = false;
                        goto finished;
                    }
                }break;
            }
        }
        token = parser_next_token(parser);
    }
    
    finished:;
    return(success);
}

static b32
parse_if_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    b32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    if (token != 0){
        success = parse_statement_down(app, parser, token_out);
        if (success){
            token = parser_next_token(parser);
            if (token != 0 && token->type == CPP_TOKEN_ELSE){
                success = parse_statement_down(app, parser, token_out);
            }
        }
    }
    return(success);
}

static b32
parse_block_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    b32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    i32 nest_level = 0;
    while (token != 0){
        switch (token->type){
            case CPP_TOKEN_BRACE_OPEN:
            {
                ++nest_level;
            }break;
            
            case CPP_TOKEN_BRACE_CLOSE:
            {
                if (nest_level == 0){
                    *token_out = *token;
                    success = true;
                    goto finished;
                }
                --nest_level;
            }break;
        }
        token = parser_next_token(parser);
    }
    
    finished:;
    return(success);
}

static b32
parse_statement_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    b32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    if (token != 0){
        b32 not_getting_block = false;
        
        do{
            switch (token->type){
                case CPP_TOKEN_BRACE_CLOSE:
                {
                    goto finished;
                }break;
                
                case CPP_TOKEN_FOR:
                {
                    success = parse_for_down(app, parser, token_out);
                    goto finished;
                }break;
                
                case CPP_TOKEN_IF:
                {
                    success = parse_if_down(app, parser, token_out);
                    goto finished;
                }break;
                
                case CPP_TOKEN_ELSE:
                {
                    success = false;
                    goto finished;
                }break;
                
                case CPP_TOKEN_BRACE_OPEN:
                {
                    if (!not_getting_block){
                        success = parse_block_down(app, parser, token_out);
                        goto finished;
                    }
                }break;
                
                case CPP_TOKEN_SEMICOLON:
                {
                    success = true;
                    *token_out = *token;
                    goto finished;
                }break;
                
                case CPP_TOKEN_EQ:
                {
                    not_getting_block = true;
                }break;
            }
            
            token = parser_next_token(parser);
        }while(token != 0);
    }
    
    finished:;
    return(success);
}

static Statement_Parser
make_statement_parser(Application_Links *app, Buffer_ID buffer, i32 token_index){
    Statement_Parser parser = {};
    Token_Range token_range = buffer_get_token_range(app, buffer);
    if (token_range.first != 0){
        parser.token_iterator = make_token_iterator(token_range, token_index);
        parser.buffer = buffer;
    }
    return(parser);
}

static b32
find_whole_statement_down(Application_Links *app, Buffer_ID buffer, i64 pos, i64 *start_out, i64 *end_out){
    b32 result = false;
    i64 start = pos;
    i64 end = start;
    
    Cpp_Get_Token_Result get_result = {};
    if (get_token_from_pos(app, buffer, pos, &get_result)){
        Statement_Parser parser = make_statement_parser(app, buffer, get_result.token_index);
        if (parser.buffer != 0){
            if (get_result.in_whitespace_after_token){
                parser_next_token(&parser);
            }
            
            Cpp_Token end_token = {};
            if (parse_statement_down(app, &parser, &end_token)){
                end = end_token.start + end_token.size;
                result = true;
            }
        }
    }
    
    *start_out = start;
    *end_out = end;
    return(result);
}

CUSTOM_COMMAND_SIG(scope_absorb_down)
CUSTOM_DOC("If a scope is currently selected, and a statement or block statement is present below the current scope, the statement is moved into the scope.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    
    Range_i64 range = get_view_range(app, view);
    if (buffer_get_char(app, buffer, range.min) == '{' &&
        buffer_get_char(app, buffer, range.max - 1) == '}'){
        Scratch_Block scratch(app);
        if (find_whole_statement_down(app, buffer, range.max, &range.start, &range.end)){
            String_Const_u8 base_string = push_buffer_range(app, scratch, buffer, range);
            String_Const_u8 string = string_skip_chop_whitespace(base_string);
            
            i32 newline_count = 0;
            for (u8 *ptr = base_string.str; ptr < string.str; ++ptr){
                if (*ptr == '\n'){
                    ++newline_count;
                }
            }
            
            b32 extra_newline = false;
            if (newline_count >= 2){
                extra_newline = true;
            }
            
            String_Const_u8 edit_str = {};
            if (extra_newline){
                edit_str = push_u8_stringf(scratch, "\n%.*s\n", string_expand(string));
            }
            else{
                edit_str = push_u8_stringf(scratch, "%.*s\n", string_expand(string));
            }
            
            Batch_Edit batch_first = {};
            Batch_Edit batch_last = {};
            
            batch_first.edit.text = edit_str;
            batch_first.edit.range = Ii64(range.min - 1, range.min - 1);
            batch_first.next = &batch_last;
            batch_last.edit.text = SCu8();
            batch_last.edit.range = range;
            
            buffer_batch_edit(app, buffer, &batch_first);
        }
    }
    
    no_mark_snap_to_cursor(app, view);
}

// BOTTOM

