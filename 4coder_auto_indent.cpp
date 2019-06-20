/*
4coder_auto_indent.cpp - Commands for auto-indentation of C++ code.
*/

// TOP

internal Buffer_Batch_Edit
make_batch_from_indent_marks(Application_Links *app, Arena *arena, Buffer_ID buffer,
                             i64 first_line, i64 one_past_last_line, i64 *indent_marks,
                             Indent_Options opts){
    i64 *shifted_indent_marks = indent_marks - first_line;
    
    i64 edit_count = 0;
    i64 edit_max = one_past_last_line - first_line;
    Buffer_Edit *edits = push_array(arena, Buffer_Edit, edit_max);
    
    List_String_Const_u8 list = {};
    
    for (i64 line_number = first_line;
         line_number < one_past_last_line;
         ++line_number){
        i64 line_start_pos = get_line_start_pos(app, buffer, line_number);
        Indent_Info hard_start = get_indent_info_line_start(app, buffer, line_start_pos, opts.tab_width);
        
        i64 correct_indentation = shifted_indent_marks[line_number];
        if (hard_start.is_blank && opts.empty_blank_lines){
            correct_indentation = 0;
        }
        if (correct_indentation == -1){
            correct_indentation = hard_start.indent_pos;
        }
        
        if (correct_indentation != hard_start.indent_pos){
            umem str_size = 0;
            char *str = 0;
            if (opts.use_tabs){
                i64 tab_count = correct_indentation/opts.tab_width;
                i64 indent = tab_count*opts.tab_width;
                i64 space_count = correct_indentation - indent;
                str_size = tab_count + space_count;
                str = push_array(arena, char, str_size);
                block_fill_u8(str, tab_count, '\t');
                block_fill_u8(str + tab_count, space_count, ' ');
            }
            else{
                str_size = correct_indentation;
                str = push_array(arena, char, str_size);
                block_fill_u8(str, str_size, ' ');
            }
            
            i64 str_position = list.total_size;
            string_list_push(arena, &list, SCu8(str, str_size));
            
            edits[edit_count].str_start = (i32)str_position;
            edits[edit_count].len = (i32)str_size;
            edits[edit_count].start = (i32)line_start_pos;
            edits[edit_count].end = (i32)hard_start.first_char_pos;
            edit_count += 1;
        }
        
        Assert(edit_count <= edit_max);
    }
    
    String_Const_u8 contiguous_text = string_list_flatten(arena, list);
    
    Buffer_Batch_Edit result = {};
    result.str = (char*)contiguous_text.str;
    result.str_len = (i32)contiguous_text.size;
    result.edits = edits;
    result.edit_count = (i32)edit_count;
    return(result);
}

internal void
set_line_indents(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 first_line, i64 one_past_last_line, i64 *indent_marks, Indent_Options opts){
    Buffer_Batch_Edit batch = make_batch_from_indent_marks(app, arena, buffer, first_line, one_past_last_line, indent_marks, opts);
    if (batch.edit_count > 0){
        buffer_batch_edit(app, buffer, batch.str, batch.edits, batch.edit_count);
    }
}

internal Cpp_Token*
seek_matching_token_backwards(Cpp_Token_Array tokens, Cpp_Token *token,
                              Cpp_Token_Type open_type, Cpp_Token_Type close_type){
    if (token <= tokens.tokens){
        token = tokens.tokens;
    }
    else{
        i32 nesting_level = 0;
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

internal Indent_Anchor_Position
find_anchor_token(Application_Links *app, Buffer_ID buffer, Cpp_Token_Array tokens, i64 line_start, i32 tab_width){
    Indent_Anchor_Position anchor = {};
    if (tokens.count > 0){
        Cpp_Token *first_invalid_token = get_first_token_from_line(app, buffer, tokens, line_start);
        if (first_invalid_token <= tokens.tokens){
            anchor.token = tokens.tokens;
        }
        else{
            i32 stack[256];
            i32 top = -1;
            Cpp_Token *token_it = tokens.tokens;
            i64 highest_checked_line_number = -1;
            for (; token_it < first_invalid_token; ++token_it){
                i64 line_number = get_line_number_from_pos(app, buffer, token_it->start);
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
                            i32 index = top;
                            top -= 1;
                            if (stack[index] == CPP_TOKEN_PARENTHESE_OPEN){
                                break;
                            }
                        }
                    }break;
                    
                    case CPP_TOKEN_BRACE_CLOSE:
                    {
                        for (;top >= 0;){
                            i32 index = top;
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
                            i32 index = top;
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
}

internal i64*
get_indentation_marks(Application_Links *app, Arena *arena, Buffer_ID buffer,
                      Cpp_Token_Array tokens, i64 first_line, i64 one_past_last_line,
                      b32 exact_align, i32 tab_width){
    i64 indent_mark_count = one_past_last_line - first_line;
    i64 *indent_marks = push_array(arena, i64, indent_mark_count);
    // Shift the array so line_index works correctly.
    indent_marks -= first_line;
    
    // Decide where to start indentation parsing.
    Indent_Anchor_Position anchor = find_anchor_token(app, buffer, tokens, first_line, tab_width);
    Cpp_Token *token_ptr = anchor.token;
    Indent_Parse_State indent = {};
    indent.current_indent = anchor.indentation;
    
    if (token_ptr == 0){
        for (i64 line_index = first_line; line_index < one_past_last_line; ++line_index){
            indent_marks[line_index] = 0;
        }
    }
    else{
        i64 line_number = get_line_number_from_pos(app, buffer, token_ptr->start);
        if (line_number > first_line){
            line_number = first_line;
        }
        
        if (token_ptr == tokens.tokens){
            indent.current_indent = 0;
        }
        
        i64 next_line_start_pos = get_line_start_pos(app, buffer, line_number);
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
                token.start = (i32)buffer_get_size(app, buffer);
                token.flags = 0;
            }
            
            for (;token.start >= next_line_start_pos && line_number < one_past_last_line;){
                next_line_start_pos = get_line_start_pos(app, buffer, line_number + 1);
                
                i64 this_indent = 0;
                i64 previous_indent = indent.previous_line_indent;
                
                i64 this_line_start = get_line_start_pos(app, buffer, line_number);
                i64 next_line_start = next_line_start_pos;
                
                b32 did_multi_line_behavior = false;
                
                // NOTE(allen): Check for multi-line tokens
                if (prev_token.start <= this_line_start && prev_token.start + prev_token.size > this_line_start){
                    if (prev_token.type == CPP_TOKEN_COMMENT || prev_token.type == CPP_TOKEN_STRING_CONSTANT){
                        Indent_Info hard_start = get_indent_info_line_start(app, buffer, this_line_start, tab_width);
                        
                        if (exact_align){
                            this_indent = indent.previous_comment_indent;
                        }
                        else{
                            if (hard_start.is_blank){
                                this_indent = previous_indent;
                            }
                            else{
                                i64 line_pos = hard_start.first_char_pos - this_line_start;
                                this_indent = line_pos + indent.comment_shift;
                                if (this_indent < 0){
                                    this_indent = 0;
                                }
                            }
                        }
                        
                        if (!hard_start.is_blank){
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
                                case CPP_TOKEN_BRACKET_CLOSE:
                                {
                                    this_indent -= tab_width;
                                }break;
                                case CPP_TOKEN_BRACE_CLOSE:
                                {
                                    this_indent -= tab_width;
                                }break;
                                case CPP_TOKEN_BRACE_OPEN: 
                                {}break;
                                
                                default:
                                if (indent.current_indent > 0){
                                    b32 statement_complete = false;
                                    
                                    Cpp_Token *prev_usable_token_ptr = token_ptr - 1;
                                    Cpp_Token prev_usable_token = {};
                                    if (prev_usable_token_ptr >= tokens.tokens){
                                        prev_usable_token = *prev_usable_token_ptr;
                                    }
                                    
                                    // Scan backwards for the previous token that actually tells us about the statement.
                                    b32 has_prev_usable_token = true;
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
                    if (this_indent < 0){
                        this_indent = 0;
                    }
                }
                
                if (indent.paren_nesting > 0){
                    if (prev_token.type != CPP_TOKEN_PARENTHESE_OPEN){
                        i64 level = indent.paren_nesting - 1;
                        level = clamp_top(level, ArrayCount(indent.paren_anchor_indent) - 1);
                        this_indent = indent.paren_anchor_indent[level];
                    }
                }
                
                // Rebase the paren anchor if the first token
                // after the open paren is on the next line.
                if (indent.paren_nesting > 0){
                    if (prev_token.type == CPP_TOKEN_PARENTHESE_OPEN){
                        i64 level = indent.paren_nesting - 1;
                        level = clamp_top(level, ArrayCount(indent.paren_anchor_indent) - 1);
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
                    i64 line = get_line_number_from_pos(app, buffer, token.start);
                    i64 start = get_line_start_pos(app, buffer, line);
                    Indent_Info hard_start = get_indent_info_line_start(app, buffer, start, tab_width);
                    
                    i64 old_dist_to_token = (token.start - start);
                    i64 old_indent = hard_start.indent_pos;
                    i64 token_start_inset = old_dist_to_token - old_indent;
                    i64 new_dist_to_token = indent.current_indent + token_start_inset;
                    
                    indent.comment_shift = (new_dist_to_token - old_dist_to_token);
                    indent.previous_comment_indent = old_indent;
                }break;
                
                case CPP_TOKEN_PARENTHESE_OPEN:
                {
                    if (!(token.flags & CPP_TFLAG_PP_BODY)){
                        if (indent.paren_nesting < ArrayCount(indent.paren_anchor_indent)){
                            i64 line = get_line_number_from_pos(app, buffer, token.start);
                            i64 start = get_line_start_pos(app, buffer, line);
                            i64 char_pos = token.start - start;
                            
                            Indent_Info hard_start = get_indent_info_line_start(app, buffer, start, tab_width);
                            
                            i64 line_pos = hard_start.first_char_pos - start;
                            
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

// TODO(allen): replace these with new range operators.
internal void
get_indent_lines_minimum(Application_Links *app, Buffer_ID buffer, i64 start_pos, i64 end_pos, i64 *line_start_out, i64 *line_end_out){
    i64 line_start = get_line_number_from_pos(app, buffer, start_pos);
    i64 line_end = get_line_number_from_pos(app, buffer, end_pos) + 1;
    *line_start_out = line_start;
    *line_end_out = line_end;
}

internal void
get_indent_lines_whole_tokens(Application_Links *app, Buffer_ID buffer, Cpp_Token_Array tokens, i64 start_pos, i64 end_pos, i64 *line_start_out, i64 *line_end_out){
    i64 line_start = get_line_number_from_pos(app, buffer, start_pos);
    i64 line_end = get_line_number_from_pos(app, buffer, end_pos);
    
    for (;line_start > 1;){
        i64 line_start_pos = get_line_start_pos(app, buffer, line_start);
        Cpp_Token *token = get_first_token_from_pos(tokens, line_start_pos);
        if (token != 0 && token->start < line_start_pos){
            line_start = get_line_number_from_pos(app, buffer, token->start);
        }
        else{
            break;
        }
    }
    
    i64 line_count = buffer_get_line_count(app, buffer);
    
    for (;line_end < line_count;){
        i64 next_line_start_pos = get_line_start_pos(app, buffer, line_end + 1);
        Cpp_Token *token = get_first_token_from_pos(tokens, next_line_start_pos);
        if (token != 0 && token->start < next_line_start_pos){
            line_end = get_line_number_from_pos(app, buffer, token->start + token->size);
        }
        else{
            break;
        }
    }
    
    line_end = clamp_top(line_end, line_count);
    line_end += 1;
    
    *line_start_out = line_start;
    *line_end_out = line_end;
}

internal b32
buffer_auto_indent(Application_Links *app, Buffer_ID buffer, i64 start, i64 end, i32 tab_width, Auto_Indent_Flag flags){
    b32 result = false;
    if (buffer_exists(app, buffer) && buffer_tokens_are_ready(app, buffer)){
        result = true;
        
        Scratch_Block scratch(app);
        
        // Stage 1: Read the tokens to be used for indentation.
        Cpp_Token_Array tokens = buffer_get_token_array(app, buffer);
        
        // Stage 2: Decide where the first and last lines are.
        //  The lines in the range [line_start,line_end) will be indented.
        i64 line_start = 0;
        i64 line_end = 0;
        if (HasFlag(flags, AutoIndent_FullTokens)){
            get_indent_lines_whole_tokens(app, buffer, tokens, start, end, &line_start, &line_end);
        }
        else{
            get_indent_lines_minimum(app, buffer, start, end, &line_start, &line_end);
        }
        
        // Stage 3: Decide Indent Amounts
        //  Get an array representing how much each line in
        //   the range [line_start,line_end) should be indented.
        i64 *indent_marks = get_indentation_marks(app, scratch, buffer, tokens, line_start, line_end, (flags & AutoIndent_ExactAlignBlock), tab_width);
        
        // Stage 4: Set the Line Indents
        Indent_Options opts = {};
        opts.empty_blank_lines = (flags & AutoIndent_ClearLine);
        opts.use_tabs = (flags & AutoIndent_UseTab);
        opts.tab_width = tab_width;
        
        set_line_indents(app, scratch, buffer, line_start, line_end, indent_marks, opts);
    }
    
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
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    i32 buffer_size = (i32)buffer_get_size(app, buffer);
    buffer_auto_indent(app, buffer, 0, buffer_size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
}

CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor)
CUSTOM_DOC("Auto-indents the line on which the cursor sits.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    i64 pos = view_get_cursor_pos(app, view);
    buffer_auto_indent(app, buffer, pos, pos, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, view, buffer);
}

CUSTOM_COMMAND_SIG(auto_tab_range)
CUSTOM_DOC("Auto-indents the range between the cursor and the mark.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    Range_i64 range = get_view_range(app, view);
    buffer_auto_indent(app, buffer, range.min, range.max, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, view, buffer);
}

CUSTOM_COMMAND_SIG(write_and_auto_tab)
CUSTOM_DOC("Inserts a character and auto-indents the line on which the cursor sits.")
{
    write_character(app);
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    u32 flags = DEFAULT_INDENT_FLAGS;
    User_Input in = get_command_input(app);
    if (in.key.character == '\n'){
        flags |= AutoIndent_ExactAlignBlock;
    }
    i64 pos = view_get_cursor_pos(app, view);
    buffer_auto_indent(app, buffer, pos, pos, DEF_TAB_WIDTH, flags);
    move_past_lead_whitespace(app, view, buffer);
}

// BOTTOM

