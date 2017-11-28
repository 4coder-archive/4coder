/*
4coder_scope_commands.cpp - A set of commands and helpers relevant for scope level navigation and editing.

TYPE: 'drop-in-command-pack'
*/

// TOP

enum{
    FindScope_Parent = 0x1,
    FindScope_NextSibling = 0x1,
    FindScope_EndOfToken = 0x2,
};

static bool32
find_scope_top(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, uint32_t flags, int32_t *end_pos_out){
    Cpp_Get_Token_Result get_result = {0};
    
    bool32 success = 0;
    int32_t position = 0;
    
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        int32_t token_index = get_result.token_index;
        if (flags & FindScope_Parent){
            --token_index;
            if (get_result.in_whitespace){
                ++token_index;
            }
        }
        
        if (token_index >= 0){
            static const int32_t chunk_cap = 512;
            Cpp_Token chunk[chunk_cap];
            Stream_Tokens stream = {0};
            
            if (init_stream_tokens(&stream, app, buffer, token_index, chunk, chunk_cap)){int32_t nest_level = 0;
                bool32 still_looping = 0;
                do{
                    for (; token_index >= stream.start; --token_index){
                        Cpp_Token *token = &stream.tokens[token_index];
                        
                        switch (token->type){
                            case CPP_TOKEN_BRACE_OPEN:
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
                                    --nest_level;
                                }
                            }break;
                            
                            case CPP_TOKEN_BRACE_CLOSE:
                            {
                                ++nest_level;
                            }break;
                        }
                    }
                    still_looping = backward_stream_tokens(&stream);
                }while(still_looping);
            }
        }
    }
    
    finished:;
    *end_pos_out = position;
    return(success);
}

static bool32
find_scope_bottom(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, uint32_t flags, int32_t *end_pos_out){
    Cpp_Get_Token_Result get_result = {0};
    
    bool32 success = 0;
    int32_t position = 0;
    
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        int32_t token_index = get_result.token_index+1;
        if (flags & FindScope_Parent){
            --token_index;
            if (get_result.in_whitespace){
                ++token_index;
            }
        }
        
        if (token_index >= 0){
            static const int32_t chunk_cap = 512;
            Cpp_Token chunk[chunk_cap];
            Stream_Tokens stream = {0};
            
            if (init_stream_tokens(&stream, app, buffer, token_index, chunk, chunk_cap)){
                int32_t nest_level = 0;
                bool32 still_looping = 0;
                do{
                    for (; token_index < stream.end; ++token_index){
                        Cpp_Token *token = &stream.tokens[token_index];
                        
                        switch (token->type){
                            case CPP_TOKEN_BRACE_OPEN:
                            {
                                ++nest_level;
                            }break;
                            
                            case CPP_TOKEN_BRACE_CLOSE:
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
                                    --nest_level;
                                }
                            }break;
                        }
                    }
                    still_looping = forward_stream_tokens(&stream);
                }while(still_looping);
            }
        }
    }
    
    finished:;
    *end_pos_out = position;
    return(success);
}

static bool32
find_next_scope(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, uint32_t flags, int32_t *end_pos_out){
    Cpp_Get_Token_Result get_result = {0};
    
    bool32 success = 0;
    int32_t position = 0;
    
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        int32_t token_index = get_result.token_index+1;
        
        if (token_index >= 0){
            static const int32_t chunk_cap = 512;
            Cpp_Token chunk[chunk_cap];
            Stream_Tokens stream = {0};
            
            if (init_stream_tokens(&stream, app, buffer, token_index, chunk, chunk_cap)){
                if (flags & FindScope_NextSibling){
                    int32_t nest_level = 1;
                    
                    bool32 still_looping = 0;
                    do{
                        for (; token_index < stream.end; ++token_index){
                            Cpp_Token *token = &stream.tokens[token_index];
                            
                            switch (token->type){
                                case CPP_TOKEN_BRACE_OPEN:
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
                                
                                case CPP_TOKEN_BRACE_CLOSE:
                                {
                                    --nest_level;
                                    if (nest_level == -1){
                                        position = start_pos;
                                        goto finished;
                                    }
                                }break;
                            }
                        }
                        still_looping = forward_stream_tokens(&stream);
                    }while(still_looping);
                }
                else{
                    bool32 still_looping = 0;
                    do{
                        for (; token_index < stream.end; ++token_index){
                            Cpp_Token *token = &stream.tokens[token_index];
                            
                            if (token->type == CPP_TOKEN_BRACE_OPEN){
                                success = 1;
                                position = token->start;
                                if (flags & FindScope_EndOfToken){
                                    position += token->size;
                                }
                                goto finished;
                            }
                        }
                        still_looping = forward_stream_tokens(&stream);
                    }while(still_looping);
                }
            }
        }
    }
    
    finished:;
    *end_pos_out = position;
    return(success);
}

static bool32
find_prev_scope(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, uint32_t flags, int32_t *end_pos_out){
    Cpp_Get_Token_Result get_result = {0};
    
    bool32 success = 0;
    int32_t position = 0;
    
    if (buffer_get_token_index(app, buffer, start_pos, &get_result)){
        int32_t token_index = get_result.token_index-1;
        
        if (token_index >= 0){
            static const int32_t chunk_cap = 512;
            Cpp_Token chunk[chunk_cap];
            Stream_Tokens stream = {0};
            
            if (init_stream_tokens(&stream, app, buffer, token_index, chunk, chunk_cap)){
                if (flags & FindScope_NextSibling){
                    int32_t nest_level = -1;
                    bool32 still_looping = 0;
                    do{
                        for (; token_index >= stream.start; --token_index){
                            Cpp_Token *token = &stream.tokens[token_index];
                            
                            switch (token->type){
                                case CPP_TOKEN_BRACE_OPEN:
                                {
                                    if (nest_level == -1){
                                        position = start_pos;
                                        goto finished;
                                    }
                                    else if (nest_level == 0){
                                        success = 1;
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
                                
                                case CPP_TOKEN_BRACE_CLOSE:
                                {
                                    ++nest_level;
                                }break;
                            }
                        }
                        still_looping = backward_stream_tokens(&stream);
                    }while(still_looping);
                }
                else{
                    bool32 still_looping = 0;
                    do{
                        for (; token_index >= stream.start; --token_index){
                            Cpp_Token *token = &stream.tokens[token_index];
                            
                            if (token->type == CPP_TOKEN_BRACE_OPEN){
                                success = 1;
                                position = token->start;
                                if (flags & FindScope_EndOfToken){
                                    position += token->size;
                                }
                                goto finished;
                            }
                        }
                        still_looping = backward_stream_tokens(&stream);
                    }while(still_looping);
                }
            }
        }
    }
    
    finished:;
    *end_pos_out = position;
    return(success);
}

static void
view_set_to_region(Application_Links *app, View_Summary *view, int32_t major_pos, int32_t minor_pos, float normalized_threshold){
    Range range = make_range(major_pos, minor_pos);
    bool32 bottom_major = false;
    if (major_pos == range.max){
        bottom_major = true;
    }
    
    Full_Cursor top, bottom;
    if (view_compute_cursor(app, view, seek_pos(range.min), &top)){
        if (view_compute_cursor(app, view, seek_pos(range.max), &bottom)){
            float top_y = top.wrapped_y;
            float bottom_y = bottom.wrapped_y;
            if (view->unwrapped_lines){
                top_y = top.unwrapped_y;
                bottom_y = bottom.unwrapped_y;
            }
            
            GUI_Scroll_Vars scroll = view->scroll_vars;
            float half_view_height = .5f*(float)(view->file_region.y1 - view->file_region.y0);
            float threshold = normalized_threshold * half_view_height;
            float current_center_y = ((float)scroll.target_y) + half_view_height;
            
            if (top_y < current_center_y - threshold || bottom_y > current_center_y + threshold){
                float center_target_y = .5f*(top_y + bottom_y);
                
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
                
                float target_y = center_target_y - half_view_height;
                if (target_y < 0){
                    target_y = 0;
                }
                
                scroll.target_y = (int32_t)(target_y);
                view_set_scroll(app, view, scroll);
            }
        }
    }
}

static float scope_center_threshold = 0.75f;

CUSTOM_COMMAND_SIG(highlight_surrounding_scope)
CUSTOM_DOC("Finds the scope enclosed by '{' '}' surrounding the cursor and puts the cursor and mark on the '{' and '}'.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t start_pos = view.cursor.pos;
    int32_t top = 0, bottom = 0;
    if (find_scope_top(app, &buffer, start_pos, FindScope_Parent, &top)){
        view_set_cursor(app, &view, seek_pos(top), true);
        if (find_scope_bottom(app, &buffer, start_pos, FindScope_Parent | FindScope_EndOfToken, &bottom)){
            view_set_mark(app, &view, seek_pos(bottom));
            view_set_to_region(app, &view, top, bottom, scope_center_threshold);
        }
        else{
            view_set_to_region(app, &view, top, top, scope_center_threshold);
        }
    }
}

CUSTOM_COMMAND_SIG(highlight_next_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' after the cursor and puts the cursor and mark on the '{' and '}'.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t start_pos = view.cursor.pos;
    int32_t top = 0, bottom = 0;
    if (find_next_scope(app, &buffer, start_pos, 0, &top)){
        if (find_scope_bottom(app, &buffer, top, FindScope_EndOfToken, &bottom)){
            view_set_cursor(app, &view, seek_pos(top), true);
            view_set_mark(app, &view, seek_pos(bottom));
            view_set_to_region(app, &view, top, bottom, scope_center_threshold);
        }
    }
}

CUSTOM_COMMAND_SIG(highlight_prev_scope_absolute)
CUSTOM_DOC("Finds the first scope started by '{' before the cursor and puts the cursor and mark on the '{' and '}'.")
{
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t start_pos = view.cursor.pos;
    int32_t top = 0, bottom = 0;
    if (find_prev_scope(app, &buffer, start_pos, 0, &top)){
        if (find_scope_bottom(app, &buffer, top, FindScope_EndOfToken, &bottom)){
            view_set_cursor(app, &view, seek_pos(top), true);
            view_set_mark(app, &view, seek_pos(bottom));
            view_set_to_region(app, &view, top, bottom, scope_center_threshold);
        }
    }
}

CUSTOM_COMMAND_SIG(place_in_scope)
CUSTOM_DOC("Wraps the code contained in the range between cursor and mark with a new curly brace scope.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Range lines = {0};
    Range range = get_range(&view);
    lines.min = buffer_get_line_number(app, &buffer, range.min);
    range.min = buffer_get_line_start(app, &buffer, lines.min);
    
    lines.max = buffer_get_line_number(app, &buffer, range.max);
    range.max = buffer_get_line_end(app, &buffer, lines.max);
    
    bool32 do_full = (lines.min < lines.max) || (!buffer_line_is_blank(app, &buffer, lines.min));
    
    if (do_full){
        Buffer_Edit edits[2];
        char str[5] = "{\n\n}";
        
        int32_t min_adjustment = 0;
        int32_t max_adjustment = 4;
        
        if (buffer_line_is_blank(app, &buffer, lines.min)){
            str[0] = '\n';
            str[1] = '{';
            ++min_adjustment;
        }
        
        if (buffer_line_is_blank(app, &buffer, lines.max)){
            str[2] = '}';
            str[3] = '\n';
            --max_adjustment;
        }
        
        int32_t min_pos = range.min + min_adjustment;
        int32_t max_pos = range.max + max_adjustment;
        
        int32_t cursor_pos = min_pos;
        int32_t mark_pos = max_pos;
        
        if (view.cursor.pos > view.mark.pos){
            cursor_pos = max_pos;
            mark_pos = min_pos;
        }
        
        edits[0].str_start = 0;
        edits[0].len = 2;
        edits[0].start = range.min;
        edits[0].end = range.min;
        
        edits[1].str_start = 2;
        edits[1].len = 2;
        edits[1].start = range.max;
        edits[1].end = range.max;
        
        buffer_batch_edit(app, &buffer, str, 4, edits, 2, BatchEdit_Normal);
        
        view_set_cursor(app, &view, seek_pos(cursor_pos), true);
        view_set_mark(app, &view, seek_pos(mark_pos));
    }
    else{
        buffer_replace_range(app, &buffer, range.min, range.max, "{\n\n}", 4);
        view_set_cursor(app, &view, seek_pos(range.min + 2), true);
        view_set_mark(app, &view, seek_pos(range.min + 2));
    }
}

CUSTOM_COMMAND_SIG(delete_current_scope)
CUSTOM_DOC("Deletes the braces surrounding the currently selected scope.  Leaves the contents within the scope.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t top = view.cursor.pos;
    int32_t bottom = view.mark.pos;
    
    if (top > bottom){
        int32_t x = top;
        top = bottom;
        bottom = x;
    }
    
    if (buffer_get_char(app, &buffer, top) == '{' && buffer_get_char(app, &buffer, bottom-1) == '}'){
        int32_t top_len = 1;
        int32_t bottom_len = 1;
        if (buffer_get_char(app, &buffer, top-1) == '\n'){
            top_len = 2;
        }
        if (buffer_get_char(app, &buffer, bottom+1) == '\n'){
            bottom_len = 2;
        }
        
        Buffer_Edit edits[2];
        edits[0].str_start = 0;
        edits[0].len = 0;
        edits[0].start = top+1 - top_len;
        edits[0].end = top+1;
        
        edits[1].str_start = 0;
        edits[1].len = 0;
        edits[1].start = bottom-1;
        edits[1].end = bottom-1 + bottom_len;
        
        buffer_batch_edit(app, &buffer, 0, 0, edits, 2, BatchEdit_Normal);
    }
}

struct Statement_Parser{
    Stream_Tokens stream;
    int32_t token_index;
    Buffer_Summary *buffer;
};

static Cpp_Token*
parser_next_token(Statement_Parser *parser){
    Cpp_Token *result = 0;
    bool32 still_looping = true;
    while (parser->token_index >= parser->stream.end && still_looping){
        still_looping = forward_stream_tokens(&parser->stream);
    }
    if (parser->token_index < parser->stream.end){
        result = &parser->stream.tokens[parser->token_index];
        ++parser->token_index;
    }
    return(result);
}

static bool32 parse_statement_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out);

static bool32
parse_for_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    bool32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    int32_t paren_level = 0;
    while (token != 0){
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

static bool32
parse_if_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    bool32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    if (token != 0){
        success = parse_statement_down(app, parser, token_out);
        if (success){
            token = parser_next_token(parser);
            if (token != 0 && token->type == CPP_TOKEN_KEY_CONTROL_FLOW){
                char lexeme[32];
                if (sizeof(lexeme)-1 >= token->size){
                    if (buffer_read_range(app, parser->buffer, token->start, token->start + token->size, lexeme)){
                        lexeme[token->size] = 0;
                        if (match(lexeme, "else")){
                            success = parse_statement_down(app, parser, token_out);
                        }
                    }
                }
            }
        }
    }
    
    return(success);
}

static bool32
parse_block_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    bool32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    int32_t nest_level = 0;
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

static bool32
parse_statement_down(Application_Links *app, Statement_Parser *parser, Cpp_Token *token_out){
    bool32 success = false;
    Cpp_Token *token = parser_next_token(parser);
    
    if (token != 0){
        bool32 not_getting_block = false;
        
        do{
            switch (token->type){
                case CPP_TOKEN_BRACE_CLOSE:
                {
                    goto finished;
                }break;
                
                case CPP_TOKEN_KEY_CONTROL_FLOW:
                {
                    char lexeme[32];
                    if (sizeof(lexeme)-1 >= token->size){
                        if (buffer_read_range(app, parser->buffer, token->start, token->start + token->size, lexeme)){
                            lexeme[token->size] = 0;
                            if (match(lexeme, "for")){
                                success = parse_for_down(app, parser, token_out);
                                goto finished;
                            }
                            else if (match(lexeme, "if")){
                                success = parse_if_down(app, parser, token_out);
                                goto finished;
                            }
                            else if (match(lexeme, "else")){
                                success = false;
                                goto finished;
                            }
                        }
                    }
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

static bool32
find_whole_statement_down(Application_Links *app, Buffer_Summary *buffer, int32_t pos, int32_t *start_out, int32_t *end_out){
    bool32 result = false;
    int32_t start = pos;
    int32_t end = start;
    
    Cpp_Get_Token_Result get_result = {0};
    
    if (buffer_get_token_index(app, buffer, pos, &get_result)){
        Statement_Parser parser = {0};
        parser.token_index = get_result.token_index;
        
        if (parser.token_index < 0){
            parser.token_index = 0;
        }
        if (get_result.in_whitespace){
            parser.token_index += 1;
        }
        
        static const int32_t chunk_cap = 512;
        Cpp_Token chunk[chunk_cap];
        
        if (init_stream_tokens(&parser.stream, app, buffer, parser.token_index, chunk, chunk_cap)){
            parser.buffer = buffer;
            
            Cpp_Token end_token = {0};
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
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t top = view.cursor.pos;
    int32_t bottom = view.mark.pos;
    
    if (top > bottom){
        int32_t x = top;
        top = bottom;
        bottom = x;
    }
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    if (buffer_get_char(app, &buffer, top) == '{' && buffer_get_char(app, &buffer, bottom-1) == '}'){
        Range range;
        if (find_whole_statement_down(app, &buffer, bottom, &range.start, &range.end)){
            char *string_space = push_array(part, char, range.end - range.start);
            buffer_read_range(app, &buffer, range.start, range.end, string_space);
            
            String string = make_string(string_space, range.end - range.start);
            string = skip_chop_whitespace(string);
            
            int32_t newline_count = 0;
            for (char *ptr = string_space; ptr < string.str; ++ptr){
                if (*ptr == '\n'){
                    ++newline_count;
                }
            }
            
            bool32 extra_newline = false;
            if (newline_count >= 2){
                extra_newline = true;
            }
            
            int32_t edit_len = string.size + 1;
            if (extra_newline){
                edit_len += 1;
            }
            
            char *edit_str = push_array(part, char, edit_len);
            if (extra_newline){
                edit_str[0] = '\n';
                copy_fast_unsafe(edit_str+1, string);
                edit_str[edit_len-1] = '\n';
            }
            else{
                copy_fast_unsafe(edit_str, string);
                edit_str[edit_len-1] = '\n';
            }
            
            Buffer_Edit edits[2];
            edits[0].str_start = 0;
            edits[0].len = edit_len;
            edits[0].start = bottom-1;
            edits[0].end = bottom-1;
            
            edits[1].str_start = 0;
            edits[1].len = 0;
            edits[1].start = range.start;
            edits[1].end = range.end;
            
            buffer_batch_edit(app, &buffer, edit_str, edit_len, edits, 2, BatchEdit_Normal);
        }
    }
    end_temp_memory(temp);
}

// BOTTOM

