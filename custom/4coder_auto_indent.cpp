/*
4coder_auto_indent.cpp - Commands for automatic indentation.
*/

// TOP

internal Batch_Edit*
make_batch_from_indentations(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 lines, i64 *indentations, Indent_Flag flags, i32 tab_width){
    i64 *shifted_indentations = indentations - lines.first;
    
    Batch_Edit *batch_first = 0;
    Batch_Edit *batch_last = 0;
    
    for (i64 line_number = lines.first;
         line_number <= lines.max;
         ++line_number){
        i64 line_start_pos = get_line_start_pos(app, buffer, line_number);
        Indent_Info indent_info = get_indent_info_line_start(app, buffer, line_start_pos, tab_width);
        
        i64 correct_indentation = shifted_indentations[line_number];
        if (indent_info.is_blank && HasFlag(flags, Indent_ClearLine)){
            correct_indentation = 0;
        }
        if (correct_indentation == -1){
            correct_indentation = indent_info.indent_pos;
        }
        
        if (correct_indentation != indent_info.indent_pos){
            umem str_size = 0;
            u8 *str = 0;
            if (HasFlag(flags, Indent_UseTab)){
                i64 tab_count = correct_indentation/tab_width;
                i64 indent = tab_count*tab_width;
                i64 space_count = correct_indentation - indent;
                str_size = tab_count + space_count;
                str = push_array(arena, u8, str_size);
                block_fill_u8(str, tab_count, '\t');
                block_fill_u8(str + tab_count, space_count, ' ');
            }
            else{
                str_size = correct_indentation;
                str = push_array(arena, u8, str_size);
                block_fill_u8(str, str_size, ' ');
            }
            
            Batch_Edit *batch = push_array(arena, Batch_Edit, 1);
            sll_queue_push(batch_first, batch_last, batch);
            batch->edit.text = SCu8(str, str_size);
            batch->edit.range = Ii64(line_start_pos, indent_info.first_char_pos);
        }
    }
    
    return(batch_first);
}

internal void
set_line_indents(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 lines, i64 *indentations, Indent_Flag flags, i32 tab_width){
    Batch_Edit *batch = make_batch_from_indentations(app, arena, buffer, lines, indentations, flags, tab_width);
    if (batch != 0){
        buffer_batch_edit(app, buffer, batch);
    }
}

internal Token*
find_anchor_token(Application_Links *app, Buffer_ID buffer, Token_Array *tokens, i64 invalid_line){
    Token *result = 0;
    
    if (tokens != 0 && tokens->tokens != 0){
        result = tokens->tokens;
        i64 invalid_pos = get_line_start_pos(app, buffer, invalid_line);
        i32 scope_counter = 0;
        i32 paren_counter = 0;
        Token *token = tokens->tokens;
        for (;;token += 1){
            if (token->pos + token->size > invalid_pos){
                break;
            }
            if (!HasFlag(token->flags,  TokenBaseFlag_PreprocessorBody)){
                if (scope_counter == 0 && paren_counter == 0){
                    result = token;
                }
                switch (token->kind){
                    case TokenBaseKind_ScopeOpen:
                    {
                        scope_counter += 1;
                    }break;
                    case TokenBaseKind_ScopeClose:
                    {
                        paren_counter = 0;
                        if (scope_counter > 0){
                            scope_counter -= 1;
                        }
                    }break;
                    case TokenBaseKind_ParentheticalOpen:
                    {
                        paren_counter += 1;
                    }break;
                    case TokenBaseKind_ParentheticalClose:
                    {
                        if (paren_counter > 0){
                            paren_counter -= 1;
                        }
                    }break;
                }
            }
        }
    }
    
    return(result);
}

internal Nest*
indent__new_nest(Arena *arena, Nest_Alloc *alloc){
    Nest *new_nest = alloc->free_nest;
    if (new_nest == 0){
        new_nest = push_array(arena, Nest, 1);
    }
    else{
        sll_stack_pop(alloc->free_nest);
    }
    return(new_nest);
}

internal void
indent__free_nest(Nest_Alloc *alloc, Nest *nest){
    sll_stack_push(alloc->free_nest, nest);
}

internal b32
indent__unfinished_statement(Token *token, Nest *current_nest){
    b32 result = false;
    if (current_nest != 0 && current_nest->kind == TokenBaseKind_ScopeOpen){
        result = true;
        switch (token->sub_kind){
            case TokenCppKind_BraceOp:
            case TokenCppKind_BraceCl:
            case TokenCppKind_Semicolon:
            case TokenCppKind_Colon:
            case TokenCppKind_Comma:
            {
                result = false;
            }break;
        }
        if (HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
            result = false;
        }
    }
    return(result);
}

internal i64*
get_indentation_array(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 lines, Indent_Flag flags, i32 tab_width, i32 indent_width){
    i64 count = lines.max - lines.min + 1;
    i64 *indentations = push_array(arena, i64, count);
    i64 *shifted_indentations = indentations - lines.first;
    block_fill_u64(indentations, sizeof(*indentations)*count, (u64)(-1));
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens = scope_attachment(app, scope, attachment_tokens, Token_Array);
    i64 anchor_line = clamp_bot(1, lines.first - 1);
    Token *anchor_token = find_anchor_token(app, buffer, tokens, anchor_line);
    if (anchor_token != 0 &&
        anchor_token >= tokens->tokens &&
        anchor_token < tokens->tokens + tokens->count){
        i64 line = get_line_number_from_pos(app, buffer, anchor_token->pos);
        line = clamp_top(line, lines.first);
        
        Token_Iterator_Array token_it = token_iterator(0, tokens, anchor_token);
        
        Scratch_Block scratch(app);
        Nest *nest = 0;
        Nest_Alloc nest_alloc = {};
        
        i64 line_last_indented = line - 1;
        i64 last_indent = 0;
        i64 actual_indent = 0;
        b32 in_unfinished_statement = false;
        
        for (;;){
            Token *token = token_it_read(&token_it);
            i64 line_where_token_starts = get_line_number_from_pos(app, buffer, token->pos);
            i64 line_start_pos = get_line_start_pos(app, buffer, line_where_token_starts);
            Indent_Info line_indent_info = get_indent_info_line_start(app, buffer, line_start_pos, tab_width);
            
            i64 current_indent = 0;
            if (nest != 0){
                current_indent = nest->indent;
            }
            i64 this_indent = current_indent;
            i64 following_indent = current_indent;
            
            b32 shift_by_actual_indent = false;
            b32 ignore_unfinished_statement = false;
            if (HasFlag(token->flags, TokenBaseFlag_PreprocessorBody)){
                this_indent = 0;
            }
            else{
                switch (token->kind){
                    case TokenBaseKind_ScopeOpen:
                    {
                        Nest *new_nest = indent__new_nest(arena, &nest_alloc);
                        sll_stack_push(nest, new_nest);
                        nest->kind = TokenBaseKind_ScopeOpen;
                        nest->indent = current_indent + indent_width;
                        following_indent = nest->indent;
                        ignore_unfinished_statement = true;
                    }break;
                    
                    case TokenBaseKind_ScopeClose:
                    {
                        for (;nest != 0 && nest->kind != TokenBaseKind_ScopeOpen;){
                            Nest *n = nest;
                            sll_stack_pop(nest);
                            indent__free_nest(&nest_alloc, n);
                        }
                        if (nest != 0 && nest->kind == TokenBaseKind_ScopeOpen){
                            Nest *n = nest;
                            sll_stack_pop(nest);
                            indent__free_nest(&nest_alloc, n);
                        }
                        this_indent = 0;
                        if (nest != 0){
                            this_indent = nest->indent;
                        }
                        following_indent = this_indent;
                        ignore_unfinished_statement = true;
                    }break;
                    
                    case TokenBaseKind_ParentheticalOpen:
                    {
                        Nest *new_nest = indent__new_nest(arena, &nest_alloc);
                        sll_stack_push(nest, new_nest);
                        nest->kind = TokenBaseKind_ParentheticalOpen;
                        nest->indent = (token->pos - line_indent_info.first_char_pos) + 1;
                        following_indent = nest->indent;
                        shift_by_actual_indent = true;
                    }break;
                    
                    case TokenBaseKind_ParentheticalClose:
                    {
                        if (nest != 0 && nest->kind == TokenBaseKind_ParentheticalOpen){
                            Nest *n = nest;
                            sll_stack_pop(nest);
                            indent__free_nest(&nest_alloc, n);
                        }
                        following_indent = 0;
                        if (nest != 0){
                            following_indent = nest->indent;
                        }
                        ignore_unfinished_statement = true;
                    }break;
                }
            }
            
            if (in_unfinished_statement && !ignore_unfinished_statement){
                this_indent += indent_width;
            }
            
#define EMIT(N) \
            Stmnt(if (lines.first <= line_it){shifted_indentations[line_it]=N;} \
            if (line_it == lines.end){goto finished;} \
            actual_indent = N; )
            
            i64 line_it = line_last_indented;
            for (;line_it < line_where_token_starts;){
                line_it += 1;
                if (line_it == line_where_token_starts){
                    EMIT(this_indent);
                }
                else{
                    EMIT(last_indent);
                }
            }
            
            i64 line_where_token_starts_shift = this_indent - line_indent_info.indent_pos;
            i64 line_where_token_ends = get_line_number_from_pos(app, buffer, token->pos + token->size);
            for (;line_it < line_where_token_ends;){
                line_it += 1;
                i64 line_it_start_pos = get_line_start_pos(app, buffer, line_it);
                Indent_Info line_it_indent_info = get_indent_info_line_start(app, buffer, line_it_start_pos, tab_width);
                i64 new_indent = line_it_indent_info.indent_pos + line_where_token_starts_shift;
                new_indent = clamp_bot(0, new_indent);
                EMIT(new_indent);
            }
#undef EMIT
            
            if (shift_by_actual_indent){
                nest->indent += actual_indent;
                following_indent += actual_indent;
            }
            
            if (token->kind != TokenBaseKind_Comment){
                in_unfinished_statement = indent__unfinished_statement(token, nest);
                if (in_unfinished_statement){
                    following_indent += indent_width;
                }
            }
            
            last_indent = following_indent;
            line_last_indented = line_it;
            
            if (!token_it_inc_non_whitespace(&token_it)){
                break;
            }
        }
    }
    
    finished:;
    return(indentations);
}

internal b32
auto_indent_buffer(Application_Links *app, Buffer_ID buffer, Range_i64 pos, Indent_Flag flags, i32 tab_width, i32 indent_width){
    ProfileScope(app, "auto indent buffer");
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens = scope_attachment(app, scope, attachment_tokens, Token_Array);
    
    b32 result = false;
    if (tokens != 0 && tokens->tokens != 0){
        result = true;
        
        Scratch_Block scratch(app);
        Range_i64 line_numbers = {};
        if (HasFlag(flags, Indent_FullTokens)){
            i32 safety_counter = 0;
            for (;;){
                Range_i64 expanded = enclose_tokens(app, buffer, pos);
                expanded = enclose_whole_lines(app, buffer, expanded);
                if (expanded == pos){
                    break;
                }
                pos = expanded;
                safety_counter += 1;
                if (safety_counter == 20){
                    pos = buffer_range(app, buffer);
                    break;
                }
            }
        }
        line_numbers = get_line_range_from_pos_range(app, buffer, pos);
        
        i64 *indentations = get_indentation_array(app, scratch, buffer, line_numbers, flags, tab_width, indent_width);
        set_line_indents(app, scratch, buffer, line_numbers, indentations, flags, tab_width);
    }
    
    return(result);
}

global_const i32 auto_indent_tab_width = 4;

function void
auto_indent_buffer(Application_Links *app, Buffer_ID buffer, Range_i64 pos, Indent_Flag flags){
    i32 indent_width = global_config.indent_width;
    AddFlag(flags, Indent_FullTokens);
    if (global_config.indent_with_tabs){
        AddFlag(flags, Indent_UseTab);
    }
    auto_indent_buffer(app, buffer, pos, flags, indent_width, auto_indent_tab_width);
}

function void
auto_indent_buffer(Application_Links *app, Buffer_ID buffer, Range_i64 pos){
    auto_indent_buffer(app, buffer, pos, 0);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(auto_indent_whole_file)
CUSTOM_DOC("Audo-indents the entire current buffer.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 buffer_size = buffer_get_size(app, buffer);
    auto_indent_buffer(app, buffer, Ii64(0, buffer_size));
}

CUSTOM_COMMAND_SIG(auto_indent_line_at_cursor)
CUSTOM_DOC("Auto-indents the line on which the cursor sits.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    auto_indent_buffer(app, buffer, Ii64(pos));
    move_past_lead_whitespace(app, view, buffer);
}

CUSTOM_COMMAND_SIG(auto_indent_range)
CUSTOM_DOC("Auto-indents the range between the cursor and the mark.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Range_i64 range = get_view_range(app, view);
    auto_indent_buffer(app, buffer, range);
    move_past_lead_whitespace(app, view, buffer);
}

CUSTOM_COMMAND_SIG(write_text_and_auto_indent)
CUSTOM_DOC("Inserts text and auto-indents the line on which the cursor sits if any of the text contains 'layout punctuation' such as ;:{}()[]# and new lines.")
{
    User_Input in = get_current_input(app);
    String_Const_u8 insert = to_writable(&in);
    if (insert.str != 0 && insert.size > 0){
        b32 do_auto_indent = false;
        for (umem i = 0; !do_auto_indent && i < insert.size; i += 1){
            switch (insert.str[i]){
                case ';': case ':':
                case '{': case '}':
                case '(': case ')':
                case '[': case ']':
                case '#':
                case '\n': case '\t':
                {
                    do_auto_indent = true;
                }break;
            }
        }
        if (do_auto_indent){
            View_ID view = get_active_view(app, Access_ReadWriteVisible);
            Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
            Range_i64 pos = {};
            pos.min = view_get_cursor_pos(app, view);
            write_text_input(app);
            pos.max= view_get_cursor_pos(app, view);
            auto_indent_buffer(app, buffer, pos, 0);
            move_past_lead_whitespace(app, view, buffer);
        }
        else{
            write_text_input(app);
        }
    }
}

// BOTTOM

