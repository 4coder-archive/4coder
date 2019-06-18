/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.
*/

// TOP

static b32 parse_statement_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out);
static f32 scope_center_threshold = 0.75f;

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
find_scope_top(Application_Links *app, Buffer_ID buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = false;
    i32 position = 0;
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index;
        if (flags & FindScope_Parent){
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
find_scope_bottom(Application_Links *app, Buffer_ID buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = false;
    i32 position = 0;
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index + 1;
        if (flags & FindScope_Parent){
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
find_next_scope(Application_Links *app, Buffer_ID buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = 0;
    i32 position = 0;
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index + 1;
        if (token_index >= 0){
            Token_Range token_range = buffer_get_token_range(app, buffer);
            if (token_range.first != 0){
                Token_Iterator token_it = make_token_iterator(token_range, token_index);
                if ((flags & FindScope_NextSibling) != 0){
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
find_prev_scope(Application_Links *app, Buffer_ID buffer, i32 start_pos, u32 flags, i32 *end_pos_out){
    Cpp_Get_Token_Result get_result = {};
    b32 success = false;
    i32 position = 0;
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        i32 token_index = get_result.token_index-1;
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
find_scope_range(Application_Links *app, Buffer_ID buffer, i32 start_pos, Range *range_out, u32 flags){
    b32 result = false;
    Range range = {};
    if (find_scope_top(app, buffer, start_pos, FindScope_Parent|flags, &range.start)){
        if (find_scope_bottom(app, buffer, start_pos, FindScope_Parent|FindScope_EndOfToken|flags, &range.end)){
            *range_out = range;
            result = true;
        }
    }
    return(result);
}

static void
view_set_to_region(Application_Links *app, View_ID view, i32 major_pos, i32 minor_pos, f32 normalized_threshold){
    Range range = make_range(major_pos, minor_pos);
    b32 bottom_major = false;
    if (major_pos == range.max){
        bottom_major = true;
    }
    
    Full_Cursor top = {};
    Full_Cursor bottom = {};
    if (view_compute_cursor(app, view, seek_pos(range.min), &top)){
        if (view_compute_cursor(app, view, seek_pos(range.max), &bottom)){
            f32 top_y = top.wrapped_y;
            f32 bottom_y = bottom.wrapped_y;
            
            Rect_i32 region = {};
            view_get_buffer_region(app, view, &region);
            
            GUI_Scroll_Vars scroll = {};
            view_get_scroll_vars(app, view, &scroll);
            f32 half_view_height = .5f*(f32)(rect_height(region));
            f32 threshold = normalized_threshold * half_view_height;
            f32 current_center_y = ((f32)scroll.target_y) + half_view_height;
            
            if (top_y < current_center_y - threshold || bottom_y > current_center_y + threshold){
                f32 center_target_y = .5f*(top_y + bottom_y);
                
                if (bottom_major){
                    if (center_target_y < bottom_y - half_view_height * .9f){
                        center_target_y = bottom_y - half_view_height * .9f;
                    }
                }
                else{
                    if (center_target_y > top_y + half_view_height * .9f){
                        center_target_y = top_y + half_view_height * .9f;
                    }
                }
                
                f32 target_y = center_target_y - half_view_height;
                if (target_y < 0){
                    target_y = 0;
                }
                
                scroll.target_y = (i32)(target_y);
                view_set_scroll(app, view, scroll);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(select_surrounding_scope)
CUSTOM_DOC("Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    Range range = {};
    if (find_scope_range(app, buffer, pos, &range, FindScope_Brace)){
        view_set_cursor(app, view, seek_pos(range.first), true);
        view_set_mark(app, view, seek_pos(range.end));
        view_set_to_region(app, view, range.first, range.end, scope_center_threshold);
        no_mark_snap_to_cursor(app, view);
    }
}

CUSTOM_COMMAND_SIG(select_next_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 start_pos = pos;
    i32 top = 0;
    i32 bottom = 0;
    if (find_next_scope(app, buffer, start_pos, FindScope_Brace, &top)){
        if (find_scope_bottom(app, buffer, top, FindScope_EndOfToken|FindScope_Brace, &bottom)){
            view_set_cursor(app, view, seek_pos(top), true);
            view_set_mark(app, view, seek_pos(bottom));
            view_set_to_region(app, view, top, bottom, scope_center_threshold);
            no_mark_snap_to_cursor(app, view);
        }
    }
}

CUSTOM_COMMAND_SIG(select_prev_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    i32 start_pos = pos;
    i32 top = 0;
    i32 bottom = 0;
    if (find_prev_scope(app, buffer, start_pos, FindScope_Brace, &top)){
        if (find_scope_bottom(app, buffer, top, FindScope_EndOfToken|FindScope_Brace, &bottom)){
            view_set_cursor(app, view, seek_pos(top), true);
            view_set_mark(app, view, seek_pos(bottom));
            view_set_to_region(app, view, top, bottom, scope_center_threshold);
            no_mark_snap_to_cursor(app, view);
        }
    }
}

static void
place_begin_and_end_on_own_lines(Application_Links *app, char *begin, char *end){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    
    Range lines = {};
    Range range = get_view_range(app, view);
    lines.min = get_line_number_from_pos(app, buffer, range.min);
    lines.max = get_line_number_from_pos(app, buffer, range.max);
    range.min = get_line_start_pos(app, buffer, lines.min);
    range.max = get_line_end_pos(app, buffer, lines.max);
    
    Scratch_Block scratch(app);
    
    b32 min_line_blank = line_is_valid_and_blank(app, buffer, lines.min);
    b32 max_line_blank = line_is_valid_and_blank(app, buffer, lines.max);
    
    if ((lines.min < lines.max) || (!min_line_blank)){
        String_Const_u8 begin_str = {};
        String_Const_u8 end_str = {};
        
        umem min_adjustment = 0;
        umem max_adjustment = 0;
        
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
        Range new_pos = make_range(range.min + (i32)min_adjustment, range.max + (i32)max_adjustment);
        
        i32 cursor_pos = 0;
        i32 mark_pos = 0;
        view_get_cursor_pos(app, view, &cursor_pos);
        
        if (cursor_pos == range.min){
            cursor_pos = new_pos.min;
            mark_pos = new_pos.max;
        }
        else{
            cursor_pos = new_pos.max;
            mark_pos = new_pos.min;
        }
        
        History_Group group = history_group_begin(app, buffer);
        buffer_replace_range(app, buffer, make_range(range.min), begin_str);
        buffer_replace_range(app, buffer, make_range(range.max + (i32)begin_str.size), end_str);
        history_group_end(group);
        
        view_set_cursor(app, view, seek_pos(cursor_pos), true);
        view_set_mark(app, view, seek_pos(mark_pos));
    }
    else{
        String_Const_u8 str = push_u8_stringf(scratch, "%s\n\n%s", begin, end);
        buffer_replace_range(app, buffer, range, str);
        i32 center_pos = range.min + (i32)cstring_length(begin) + 1;
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
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    
    i32 view_cursor_pos = 0;
    view_get_cursor_pos(app, view, &view_cursor_pos);
    i32 view_mark_pos = 0;
    view_get_mark_pos(app, view, &view_mark_pos);
    
    i32 top = view_cursor_pos;
    i32 bottom = view_mark_pos;
    
    if (top > bottom){
        i32 x = top;
        top = bottom;
        bottom = x;
    }
    
    if (buffer_get_char(app, buffer, top) == '{' && buffer_get_char(app, buffer, bottom - 1) == '}'){
        i32 top_len = 1;
        i32 bottom_len = 1;
        if (buffer_get_char(app, buffer, top - 1) == '\n'){
            top_len = 2;
        }
        if (buffer_get_char(app, buffer, bottom + 1) == '\n'){
            bottom_len = 2;
        }
        
        Buffer_Edit edits[2];
        edits[0].str_start = 0;
        edits[0].len = 0;
        edits[0].start = top + 1 - top_len;
        edits[0].end = top + 1;
        
        edits[1].str_start = 0;
        edits[1].len = 0;
        edits[1].start = bottom - 1;
        edits[1].end = bottom - 1 + bottom_len;
        
        buffer_batch_edit(app, buffer, 0, edits, 2);
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
find_whole_statement_down(Application_Links *app, Buffer_ID buffer, i32 pos, i32 *start_out, i32 *end_out){
    b32 result = false;
    i32 start = pos;
    i32 end = start;
    
    Cpp_Get_Token_Result get_result = {};
    
    if (buffer_get_token_index(app, buffer, pos, &get_result)){
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
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    
    i32 view_cursor_pos = 0;
    view_get_cursor_pos(app, view, &view_cursor_pos);
    i32 view_mark_pos = 0;
    view_get_mark_pos(app, view, &view_mark_pos);
    
    i32 top = view_cursor_pos;
    i32 bottom = view_mark_pos;
    
    if (top > bottom){
        i32 x = top;
        top = bottom;
        bottom = x;
    }
    
    if (buffer_get_char(app, buffer, top) == '{' && buffer_get_char(app, buffer, bottom - 1) == '}'){
        Scratch_Block scratch(app);
        Range range = {};
        if (find_whole_statement_down(app, buffer, bottom, &range.start, &range.end)){
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
            
            Buffer_Edit edits[2];
            edits[0].str_start = 0;
            edits[0].len = (i32)edit_str.size;
            edits[0].start = bottom - 1;
            edits[0].end = bottom - 1;
            edits[1].str_start = 0;
            edits[1].len = 0;
            edits[1].start = range.start;
            edits[1].end = range.end;
            buffer_batch_edit(app, buffer, (char*)edit_str.str, edits, 2);
        }
    }
    
    no_mark_snap_to_cursor(app, view);
}

// BOTTOM

