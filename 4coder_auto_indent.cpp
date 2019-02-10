/*
4coder_auto_indent.cpp - Commands for auto-indentation of C++ code.
*/

// TOP

static Hard_Start_Result
buffer_find_hard_start(Application_Links *app, Buffer_Summary *buffer, int32_t line_start, int32_t tab_width){
    tab_width -= 1;
    
    Hard_Start_Result result = {};
    result.all_space = true;
    result.indent_pos = 0;
    result.char_pos = line_start;
    
    char data_chunk[1024];
    Stream_Chunk stream = {};
    stream.add_null = true;
    if (init_stream_chunk(&stream, app, buffer, line_start, data_chunk, sizeof(data_chunk))){
        bool32 still_looping = true;
        do{
            for (; result.char_pos < stream.end; ++result.char_pos){
                char c = stream.data[result.char_pos];
                
                if (c == '\n' || c == 0){
                    result.all_whitespace = 1;
                    goto double_break;
                }
                
                if (c >= '!' && c <= '~'){
                    goto double_break;
                }
                
                if (c == '\t'){
                    result.indent_pos += tab_width;
                }
                
                if (c != ' '){
                    result.all_space = false;
                }
                
                result.indent_pos += 1;
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
    }
    
    double_break:;
    return(result);
}

static Buffer_Batch_Edit
make_batch_from_indent_marks(Application_Links *app, Partition *arena, Buffer_Summary *buffer,
                             int32_t first_line, int32_t one_past_last_line, int32_t *indent_marks, Indent_Options opts){
    int32_t *shifted_indent_marks = indent_marks - first_line;
    
    int32_t edit_count = 0;
    int32_t edit_max = one_past_last_line - first_line;
    Buffer_Edit *edits = push_array(arena, Buffer_Edit, edit_max);
    
    char *str_base = push_array(arena, char, 0);
    
    for (int32_t line_number = first_line;
         line_number < one_past_last_line;
         ++line_number){
        int32_t line_start_pos = buffer_get_line_start(app, buffer, line_number);
        Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, line_start_pos, opts.tab_width);
        
        int32_t correct_indentation = shifted_indent_marks[line_number];
        if (hard_start.all_whitespace && opts.empty_blank_lines){
            correct_indentation = 0;
        }
        if (correct_indentation == -1){
            correct_indentation = hard_start.indent_pos;
        }
        
        if (correct_indentation != hard_start.indent_pos){
            int32_t str_size = correct_indentation;
            if (opts.use_tabs){
                str_size = correct_indentation/opts.tab_width + correct_indentation%opts.tab_width;
            }
            char *str = push_array(arena, char, str_size);
            if (opts.use_tabs){
                int32_t indent = 0;
                int32_t j = 0;
                for (;indent + opts.tab_width <= correct_indentation;
                     indent += opts.tab_width){
                    str[j++] = '\t';
                }
                for (;indent < correct_indentation;
                     indent += 1){
                    str[j++] = ' ';
                }
            }
            else{
                for (int32_t j = 0; j < correct_indentation;){
                    str[j++] = ' ';
                }
            }
            
            Buffer_Edit new_edit;
            new_edit.str_start = (int32_t)(str - str_base);
            new_edit.len = str_size;
            new_edit.start = line_start_pos;
            new_edit.end = hard_start.char_pos;
            edits[edit_count++] = new_edit;
        }
        
        Assert(edit_count <= edit_max);
    }
    
    Buffer_Batch_Edit result = {};
    result.str = str_base;
    result.str_len = (int32_t)(push_array(arena, char, 0) - str_base);
    result.edits = edits;
    result.edit_count = edit_count;
    return(result);
}

static void
set_line_indents(Application_Links *app, Partition *part, Buffer_Summary *buffer,
                 int32_t first_line, int32_t one_past_last_line,
                 int32_t *indent_marks, Indent_Options opts){
    Buffer_Batch_Edit batch = make_batch_from_indent_marks(app, part, buffer,
                                                           first_line, one_past_last_line,
                                                           indent_marks, opts);
    if (batch.edit_count > 0){
        buffer_batch_edit(app, buffer,
                          batch.str, batch.str_len, batch.edits, batch.edit_count,
                          BatchEdit_PreserveTokens);
    }
}

static Cpp_Token*
seek_matching_token_backwards(Cpp_Token_Array tokens, Cpp_Token *token,
                              Cpp_Token_Type open_type, Cpp_Token_Type close_type){
    if (token <= tokens.tokens){
        token = tokens.tokens;
    }
    else{
        int32_t nesting_level = 0;
        for (; token > tokens.tokens; --token){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                if (token->type == close_type){
                    ++nesting_level;
                }
                else if (token->type == open_type){
                    if (nesting_level == 0){
                        break;
                    }
                    else{
                        --nesting_level;
                    }
                }
            }
        }
    }
    return(token);
}

static Indent_Anchor_Position
find_anchor_token(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens,
                  int32_t line_start, int32_t tab_width){
#if 1
    // NOTE(allen): New implementation of find_anchor_token (4.0.26) revert if it is a problem.
    Indent_Anchor_Position anchor = {};
    
    if (tokens.count > 0){
        Cpp_Token *first_invalid_token = get_first_token_at_line(app, buffer, tokens, line_start);
        if (first_invalid_token <= tokens.tokens){
            anchor.token = tokens.tokens;
        }
        else{
            int32_t stack[256];
            int32_t top = -1;
            
            Cpp_Token *token_it = tokens.tokens;
            int32_t highest_checked_line_number = -1;
            for (; token_it < first_invalid_token; ++token_it){
                int32_t line_number = buffer_get_line_number(app, buffer, token_it->start);
                if (highest_checked_line_number < line_number){
                    highest_checked_line_number = line_number;
                    if (top == -1){
                        anchor.token = token_it;
                    }
                }
                
                switch (token_it->type){
                    case CPP_TOKEN_BRACE_OPEN:
                    case CPP_TOKEN_BRACKET_OPEN:
                    case CPP_TOKEN_PARENTHESE_OPEN:
                    {
                        top += 1;
                        stack[top] = token_it->type;
                    }break;
                    
                    case CPP_TOKEN_PARENTHESE_CLOSE:
                    {
                        for (;top >= 0;){
                            int32_t index = top;
                            top -= 1;
                            if (stack[index] == CPP_TOKEN_PARENTHESE_OPEN){
                                break;
                            }
                        }
                    }break;
                    
                    case CPP_TOKEN_BRACE_CLOSE:
                    {
                        for (;top >= 0;){
                            int32_t index = top;
                            if (stack[index] == CPP_TOKEN_PARENTHESE_OPEN){
                                break;
                            }
                            top -= 1;
                            if (stack[index] == CPP_TOKEN_BRACE_OPEN){
                                break;
                            }
                        }
                    }break;
                    
                    case CPP_TOKEN_BRACKET_CLOSE:
                    {
                        for (;top >= 0;){
                            int32_t index = top;
                            if (stack[index] == CPP_TOKEN_PARENTHESE_OPEN ||
                                stack[index] == CPP_TOKEN_BRACE_OPEN){
                                break;
                            }
                            top -= 1;
                            if (stack[index] == CPP_TOKEN_BRACKET_OPEN){
                                break;
                            }
                        }
                    }break;
                }
            }
        }
    }
    return(anchor);
    
#else
    // NOTE(allen): Old (4.0.25) implementation of find_anchor_token.
    Indent_Anchor_Position anchor = {};
    
    if (tokens.count != 0){
        anchor.token = get_first_token_at_line(app, buffer, tokens, line_start);
        if (anchor.token == 0){
            anchor.token = tokens.tokens + (tokens.count - 1);
        }
        
        if (anchor.token > tokens.tokens){
            --anchor.token;
            for (; anchor.token > tokens.tokens; --anchor.token){
                if (!(anchor.token->flags & CPP_TFLAG_PP_BODY)){
                    switch(anchor.token->type){
                        case CPP_TOKEN_BRACE_OPEN:
                        case CPP_TOKEN_BRACE_CLOSE:
                        goto out_of_loop;
                    }
                }
            }
            out_of_loop:;
        }
        
        int32_t current_indent = 0;
        int32_t found_safe_start_position = 0;
        do{
            int32_t line = buffer_get_line_number(app, buffer, anchor.token->start);
            int32_t start = buffer_get_line_start(app, buffer, line);
            
            Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, start, tab_width);
            current_indent = hard_start.indent_pos;
            
            Cpp_Token *start_token = get_first_token_at_line(app, buffer, tokens, line);
            Cpp_Token *brace_token = anchor.token;
            
            if (start_token->type == CPP_TOKEN_PARENTHESE_OPEN){
                if (start_token == tokens.tokens){
                    found_safe_start_position = true;
                }
                else{
                    anchor.token = start_token - 1;
                }
            }
            else{
                int32_t close = 0;
                
                for (anchor.token = brace_token;
                     anchor.token > start_token;
                     --anchor.token){
                    switch (anchor.token->type){
                        case CPP_TOKEN_PARENTHESE_CLOSE:
                        case CPP_TOKEN_BRACKET_CLOSE:
                        case CPP_TOKEN_BRACE_CLOSE:
                        {
                            close = anchor.token->type;
                        }goto out_of_loop2;
                    }
                }
                out_of_loop2:;
                
                Cpp_Token_Type open_type = CPP_TOKEN_JUNK;
                Cpp_Token_Type close_type = CPP_TOKEN_JUNK;
                switch (close){
                    case 0:
                    {
                        anchor.token = start_token;
                        found_safe_start_position = true;
                    }break;
                    
                    case CPP_TOKEN_PARENTHESE_CLOSE:
                    {
                        open_type = CPP_TOKEN_PARENTHESE_OPEN;
                        close_type = CPP_TOKEN_PARENTHESE_CLOSE;
                    }break;
                    
                    case CPP_TOKEN_BRACKET_CLOSE:
                    {
                        open_type = CPP_TOKEN_BRACKET_OPEN;
                        close_type = CPP_TOKEN_BRACKET_CLOSE;
                    }break;
                    
                    case CPP_TOKEN_BRACE_CLOSE:
                    {
                        open_type = CPP_TOKEN_BRACE_OPEN;
                        close_type = CPP_TOKEN_BRACE_CLOSE;
                    }break;
                }
                if (open_type != CPP_TOKEN_JUNK){
                    anchor.token = seek_matching_token_backwards(tokens, anchor.token - 1, open_type, close_type);
                }
            }
        } while(found_safe_start_position == 0);
        anchor.indentation = current_indent;
    }
    
    return(anchor);
#endif
}

static int32_t*
get_indentation_marks(Application_Links *app, Partition *arena, Buffer_Summary *buffer,
                      Cpp_Token_Array tokens, int32_t first_line, int32_t one_past_last_line,
                      bool32 exact_align, int32_t tab_width){
    int32_t indent_mark_count = one_past_last_line - first_line;
    int32_t *indent_marks = push_array(arena, int32_t, indent_mark_count);
    // Shift the array so line_index works correctly.
    indent_marks -= first_line;
    
    // Decide where to start indentation parsing.
    Indent_Anchor_Position anchor = find_anchor_token(app, buffer, tokens, first_line, tab_width);
    Cpp_Token *token_ptr = anchor.token;
    Indent_Parse_State indent = {};
    indent.current_indent = anchor.indentation;
    
    if (token_ptr == 0){
        for (int32_t line_index = first_line; line_index < one_past_last_line; ++line_index){
            indent_marks[line_index] = 0;
        }
    }
    else{
        int32_t line_number = buffer_get_line_number(app, buffer, token_ptr->start);
        if (line_number > first_line){
            line_number = first_line;
        }
        
        if (token_ptr == tokens.tokens){
            indent.current_indent = 0;
        }
        
        int32_t next_line_start_pos = buffer_get_line_start(app, buffer, line_number);
        indent.previous_line_indent = indent.current_indent;
        Cpp_Token prev_token = {};
        Cpp_Token token = {};
        if (token_ptr < tokens.tokens + tokens.count){
            token = *token_ptr;
        }
        
        // Back up and consume this token too IF it is a scope opener.
        if (token.type == CPP_TOKEN_BRACE_OPEN || token.type == CPP_TOKEN_BRACKET_OPEN){
            --token_ptr;
        }
        
        // LOOP OVER TOKENS
        for (;;){
            if (line_number >= one_past_last_line){
                break;
            }
            
            prev_token = token;
            ++token_ptr;
            if (token_ptr < tokens.tokens + tokens.count){
                token = *token_ptr;
            }
            else{
                token.type = CPP_TOKEN_EOF;
                token.start = buffer->size;
                token.flags = 0;
            }
            
            for (;token.start >= next_line_start_pos && line_number < one_past_last_line;){
                next_line_start_pos = buffer_get_line_start(app, buffer, line_number + 1);
                
                int32_t this_indent = 0;
                int32_t previous_indent = indent.previous_line_indent;
                
                int32_t this_line_start = buffer_get_line_start(app, buffer, line_number);
                int32_t next_line_start = next_line_start_pos;
                
                bool32 did_multi_line_behavior = false;
                
                // NOTE(allen): Check for multi-line tokens
                if (prev_token.start <= this_line_start && prev_token.start + prev_token.size > this_line_start){
                    if (prev_token.type == CPP_TOKEN_COMMENT || prev_token.type == CPP_TOKEN_STRING_CONSTANT){
                        Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, this_line_start, tab_width);
                        
                        if (exact_align){
                            this_indent = indent.previous_comment_indent;
                        }
                        else{
                            if (hard_start.all_whitespace){
                                this_indent = previous_indent;
                            }
                            else{
                                int32_t line_pos = hard_start.char_pos - this_line_start;
                                this_indent = line_pos + indent.comment_shift;
                                if (this_indent < 0){
                                    this_indent = 0;
                                }
                            }
                        }
                        
                        if (!hard_start.all_whitespace){
                            if (line_number >= first_line){
                                indent.previous_comment_indent = this_indent;
                            }
                            else{
                                indent.previous_comment_indent = hard_start.indent_pos;
                            }
                        }
                        
                        did_multi_line_behavior = true;
                    }
                }
                
                if (!did_multi_line_behavior){
                    this_indent = indent.current_indent;
                    if (token.start < next_line_start){
                        if (token.flags & CPP_TFLAG_PP_DIRECTIVE){
                            this_indent = 0;
                        }
                        else{
                            switch (token.type){
                                case CPP_TOKEN_BRACKET_CLOSE: this_indent -= tab_width; break;
                                case CPP_TOKEN_BRACE_CLOSE: this_indent -= tab_width; break;
                                case CPP_TOKEN_BRACE_OPEN: break;
                                
                                default:
                                if (indent.current_indent > 0){
                                    bool32 statement_complete = false;
                                    
                                    Cpp_Token *prev_usable_token_ptr = token_ptr - 1;
                                    Cpp_Token prev_usable_token = {};
                                    if (prev_usable_token_ptr >= tokens.tokens){
                                        prev_usable_token = *prev_usable_token_ptr;
                                    }
                                    
                                    // Scan backwards for the previous token that actually tells us about the statement.
                                    bool32 has_prev_usable_token = true;
#define NotUsable(T) \
                                    (((T).flags&CPP_TFLAG_PP_BODY) || ((T).flags&CPP_TFLAG_PP_DIRECTIVE) || ((T).type == CPP_TOKEN_COMMENT))
                                    if (NotUsable(prev_usable_token)){
                                        has_prev_usable_token = false;
                                        
                                        for (--prev_usable_token_ptr;
                                             prev_usable_token_ptr >= tokens.tokens;
                                             --prev_usable_token_ptr){
                                            
                                            prev_usable_token = *prev_usable_token_ptr;
                                            if (!NotUsable(prev_usable_token)){
                                                has_prev_usable_token = true;
                                                break;
                                            }
                                        }
                                    }
#undef NotUsable
                                    
                                    if (!has_prev_usable_token){
                                        statement_complete = true;
                                    }
                                    else{
                                        if (prev_usable_token.type == CPP_TOKEN_BRACKET_OPEN ||
                                            prev_usable_token.type == CPP_TOKEN_BRACE_OPEN ||
                                            prev_usable_token.type == CPP_TOKEN_BRACE_CLOSE ||
                                            prev_usable_token.type == CPP_TOKEN_SEMICOLON ||
                                            prev_usable_token.type == CPP_TOKEN_COLON ||
                                            prev_usable_token.type == CPP_TOKEN_COMMA){
                                            statement_complete = true;
                                        }
                                    }
                                    
                                    if (!statement_complete){
                                        this_indent += tab_width;
                                    }
                                }
                            }
                        }
                    }
                    if (this_indent < 0) this_indent = 0;
                }
                
                if (indent.paren_nesting > 0){
                    if (prev_token.type != CPP_TOKEN_PARENTHESE_OPEN){
                        int32_t level = indent.paren_nesting-1;
                        if (level >= ArrayCount(indent.paren_anchor_indent)){
                            level = ArrayCount(indent.paren_anchor_indent)-1;
                        }
                        this_indent = indent.paren_anchor_indent[level];
                    }
                }
                
                // Rebase the paren anchor if the first token
                // after the open paren is on the next line.
                if (indent.paren_nesting > 0){
                    if (prev_token.type == CPP_TOKEN_PARENTHESE_OPEN){
                        int32_t level = indent.paren_nesting-1;
                        if (level >= ArrayCount(indent.paren_anchor_indent)){
                            level = ArrayCount(indent.paren_anchor_indent)-1;
                        }
                        indent.paren_anchor_indent[level] = this_indent;
                    }
                }
                
                if (line_number >= first_line){
                    indent_marks[line_number] = this_indent;
                }
                ++line_number;
                
                indent.previous_line_indent = this_indent;
            }
            
            // Update indent state.
            switch (token.type){
                case CPP_TOKEN_BRACKET_OPEN: indent.current_indent += tab_width; break;
                case CPP_TOKEN_BRACKET_CLOSE: indent.current_indent -= tab_width; break;
                case CPP_TOKEN_BRACE_OPEN: indent.current_indent += tab_width; break;
                case CPP_TOKEN_BRACE_CLOSE: indent.current_indent -= tab_width; break;
                
                case CPP_TOKEN_COMMENT:
                case CPP_TOKEN_STRING_CONSTANT:
                {
                    int32_t line = buffer_get_line_number(app, buffer, token.start);
                    int32_t start = buffer_get_line_start(app, buffer, line);
                    Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, start, tab_width);
                    
                    int32_t old_dist_to_token = (token.start - start);
                    int32_t old_indent = hard_start.indent_pos;
                    int32_t token_start_inset = old_dist_to_token - old_indent;
                    int32_t new_dist_to_token = indent.current_indent + token_start_inset;
                    
                    indent.comment_shift = (new_dist_to_token - old_dist_to_token);
                    indent.previous_comment_indent = old_indent;
                }break;
                
                case CPP_TOKEN_PARENTHESE_OPEN:
                {
                    if (!(token.flags & CPP_TFLAG_PP_BODY)){
                        if (indent.paren_nesting < ArrayCount(indent.paren_anchor_indent)){
                            int32_t line = buffer_get_line_number(app, buffer, token.start);
                            int32_t start = buffer_get_line_start(app, buffer, line);
                            int32_t char_pos = token.start - start;
                            
                            Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, start, tab_width);
                            
                            int32_t line_pos = hard_start.char_pos - start;
                            
                            indent.paren_anchor_indent[indent.paren_nesting] = char_pos - line_pos + indent.previous_line_indent + 1;
                        }
                        ++indent.paren_nesting;
                    }
                }break;
                
                case CPP_TOKEN_PARENTHESE_CLOSE:
                {
                    if (!(token.flags & CPP_TFLAG_PP_BODY)){
                        if (indent.paren_nesting > 0){
                            --indent.paren_nesting;
                        }
                    }
                }break;
            }
        }
    }
    
    // Unshift the indent_marks array.
    indent_marks += first_line;
    return(indent_marks);
}

static void
get_indent_lines_minimum(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, int32_t end_pos, int32_t *line_start_out, int32_t *line_end_out){
    int32_t line_start = buffer_get_line_number(app, buffer, start_pos);
    int32_t line_end = buffer_get_line_number(app, buffer, end_pos) + 1;
    
    *line_start_out = line_start;
    *line_end_out = line_end;
}

static void
get_indent_lines_whole_tokens(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, int32_t start_pos, int32_t end_pos, int32_t *line_start_out, int32_t *line_end_out){
    int32_t line_start = buffer_get_line_number(app, buffer, start_pos);
    int32_t line_end = buffer_get_line_number(app, buffer, end_pos);
    
    for (;line_start > 1;){
        int32_t line_start_pos = 0;
        Cpp_Token *token = get_first_token_at_line(app, buffer, tokens, line_start, &line_start_pos);
        if (token && token->start < line_start_pos){
            line_start = buffer_get_line_number(app, buffer, token->start);
        }
        else{
            break;
        }
    }
    
    for (;line_end < buffer->line_count;){
        int32_t next_line_start_pos = 0;
        Cpp_Token *token = get_first_token_at_line(app, buffer, tokens, line_end+1, &next_line_start_pos);
        if (token && token->start < next_line_start_pos){
            line_end = buffer_get_line_number(app, buffer, token->start+token->size);
        }
        else{
            break;
        }
    }
    
    if (line_end > buffer->line_count){
        line_end = buffer->line_count + 1;
    }
    else{
        line_end += 1;
    }
    
    *line_start_out = line_start;
    *line_end_out = line_end;
}

static bool32
buffer_auto_indent(Application_Links *app, Partition *part, Buffer_Summary *buffer, int32_t start, int32_t end, int32_t tab_width, Auto_Indent_Flag flags){
    
    bool32 result = false;
    if (buffer->exists && buffer->tokens_are_ready){
        result = true;
        
        Temp_Memory temp = begin_temp_memory(part);
        
        // Stage 1: Read the tokens to be used for indentation.
        Cpp_Token_Array tokens;
        tokens.count = buffer_token_count(app, buffer);
        tokens.max_count = tokens.count;
        tokens.tokens = push_array(part, Cpp_Token, tokens.count);
        buffer_read_tokens(app, buffer, 0, tokens.count, tokens.tokens);
        
        // Stage 2: Decide where the first and last lines are.
        //  The lines in the range [line_start,line_end) will be indented.
        int32_t line_start = 0, line_end = 0;
        if (flags & AutoIndent_FullTokens){
            get_indent_lines_whole_tokens(app, buffer, tokens, start, end, &line_start, &line_end);
        }
        else{
            get_indent_lines_minimum(app, buffer, start, end, &line_start, &line_end);
        }
        
        // Stage 3: Decide Indent Amounts
        //  Get an array representing how much each line in
        //   the range [line_start,line_end) should be indented.
        int32_t *indent_marks = get_indentation_marks(app, part, buffer, tokens, line_start, line_end, (flags & AutoIndent_ExactAlignBlock), tab_width);
        
        // Stage 4: Set the Line Indents
        Indent_Options opts = {};
        opts.empty_blank_lines = (flags & AutoIndent_ClearLine);
        opts.use_tabs = (flags & AutoIndent_UseTab);
        opts.tab_width = tab_width;
        
        set_line_indents(app, part, buffer, line_start, line_end, indent_marks, opts);
        
        end_temp_memory(temp);
    }
    
    return(result);
}

static bool32
buffer_auto_indent(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end, int32_t tab_width, Auto_Indent_Flag flags){
    bool32 result = buffer_auto_indent(app, &global_part, buffer, start, end, tab_width, flags);
    return(result);
}

//
// Commands
//

#if !defined(DEFAULT_INDENT_FLAGS)
# define DEFAULT_INDENT_FLAGS ((global_config.indent_with_tabs)?(AutoIndent_UseTab):(0))
#endif

#if !defined(DEF_TAB_WIDTH)
# define DEF_TAB_WIDTH global_config.indent_width
#endif

CUSTOM_COMMAND_SIG(auto_tab_whole_file)
CUSTOM_DOC("Audo-indents the entire current buffer.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    buffer_auto_indent(app, &global_part, &buffer, 0, buffer.size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
}

CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor)
CUSTOM_DOC("Auto-indents the line on which the cursor sits.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    buffer_auto_indent(app, &global_part, &buffer, view.cursor.pos, view.cursor.pos, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(auto_tab_range)
CUSTOM_DOC("Auto-indents the range between the cursor and the mark.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    Range range = get_view_range(&view);
    
    buffer_auto_indent(app, &global_part, &buffer, range.min, range.max, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(write_and_auto_tab)
CUSTOM_DOC("Inserts a character and auto-indents the line on which the cursor sits.")
{
    exec_command(app, write_character);
    
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    uint32_t flags = DEFAULT_INDENT_FLAGS;
    User_Input in = get_command_input(app);
    if (in.key.character == '\n'){
        flags |= AutoIndent_ExactAlignBlock;
    }
    buffer_auto_indent(app, &global_part, &buffer, view.cursor.pos, view.cursor.pos, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS);
    move_past_lead_whitespace(app, &view, &buffer);
}

// BOTTOM

