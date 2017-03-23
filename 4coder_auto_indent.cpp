/*
4coder_auto_indent.cpp - Commands for auto-indentation of C++ code.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_AUTO_INDENT_CPP)
#define FCODER_AUTO_INDENT_CPP

#include "4coder_base_commands.cpp"
#include "4coder_helper/4coder_streaming.h"
#include "4coder_helper/4coder_long_seek.h"
#include "4coder_lib/4coder_mem.h"
#include "4coder_default_framework.h"

#if !defined(DEFAULT_INDENT_FLAGS)
# define DEFAULT_INDENT_FLAGS 0
#endif

#if !defined(DEF_TAB_WIDTH)
# define DEF_TAB_WIDTH 4
#endif

//
// Generic Line Indenter
//

struct Hard_Start_Result{
    int32_t char_pos;
    int32_t indent_pos;
    int32_t all_whitespace;
    int32_t all_space;
};

static Hard_Start_Result
buffer_find_hard_start(Application_Links *app, Buffer_Summary *buffer, int32_t line_start, int32_t tab_width){
    Hard_Start_Result result = {0};
    char data_chunk[1024];
    Stream_Chunk stream = {0};
    char c;
    
    tab_width -= 1;
    
    result.all_space = 1;
    result.indent_pos = 0;
    result.char_pos = line_start;
    
    stream.add_null = 1;
    if (init_stream_chunk(&stream, app, buffer, line_start, data_chunk, sizeof(data_chunk))){
        int32_t still_looping = 1;
        do{
            for (; result.char_pos < stream.end; ++result.char_pos){
                c = stream.data[result.char_pos];
                
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
                    result.all_space = 0;
                }
                
                result.indent_pos += 1;
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
    }
    
    double_break:;
    return(result);
}

struct Indent_Options{
    bool32 empty_blank_lines;
    bool32 use_tabs;
    int32_t tab_width;
};

static Buffer_Batch_Edit
make_batch_from_indent_marks(Application_Links *app, Partition *part, Buffer_Summary *buffer, int32_t line_start, int32_t line_end, int32_t *indent_marks, Indent_Options opts){
    
    Buffer_Batch_Edit result = {0};
    
    int32_t edit_max = line_end - line_start;
    int32_t edit_count = 0;
    
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    int32_t str_size = 0;
    
    // NOTE(allen): Shift the array so that line_i can just operate in
    // it's natural value range.
    indent_marks -= line_start;
    
    for (int32_t line_i = line_start; line_i < line_end; ++line_i){
        int32_t line_start_pos = buffer_get_line_start(app, buffer, line_i);
        Hard_Start_Result hard_start = 
            buffer_find_hard_start(app, buffer, line_start_pos, opts.tab_width);
        
        int32_t correct_indentation = indent_marks[line_i];
        if (hard_start.all_whitespace && opts.empty_blank_lines){
            correct_indentation = 0;
        }
        if (correct_indentation == -1){
            correct_indentation = hard_start.indent_pos;
        }
        
        // TODO(allen): Only replace spaces if we are using space based indentation.
        if (!hard_start.all_space || correct_indentation != hard_start.indent_pos){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += correct_indentation;
            char *str = push_array(part, char, correct_indentation);
            int32_t j = 0;
            
            if (opts.use_tabs){
                int32_t i = 0;
                for (; i + opts.tab_width <= correct_indentation; i += opts.tab_width){
                    str[j++] = '\t';
                }
                for (; i < correct_indentation; ++i){
                    str[j++] = ' ';
                }
            }
            else{
                for (; j < correct_indentation; ++j){
                    str[j] = ' ';
                }
            }
            
            new_edit.len = j;
            new_edit.start = line_start_pos;
            new_edit.end = hard_start.char_pos;
            edits[edit_count++] = new_edit;
        }
        
        Assert(edit_count <= edit_max);
    }
    
    result.str = str_base;
    result.str_len = str_size;
    
    result.edits = edits;
    result.edit_count = edit_count;
    
    return(result);
}

static void
set_line_indents(Application_Links *app, Partition *part, Buffer_Summary *buffer, int32_t line_start, int32_t line_end, int32_t *indent_marks, Indent_Options opts){
    Buffer_Batch_Edit batch =
        make_batch_from_indent_marks(app, part, buffer, line_start, line_end, indent_marks, opts);
    
    if (batch.edit_count > 0){
        buffer_batch_edit(app, buffer, batch.str, batch.str_len,
                          batch.edits, batch.edit_count, BatchEdit_PreserveTokens);
    }
}

static Cpp_Token*
seek_matching_token_backwards(Cpp_Token_Array tokens, Cpp_Token *token, Cpp_Token_Type open_type, Cpp_Token_Type close_type){
    int32_t nesting_level = 0;
    if (token <= tokens.tokens){
        token = tokens.tokens;
    }
    else{
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

static Cpp_Token*
find_anchor_token(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens, int32_t line_start, int32_t tab_width, int32_t *current_indent_out){
    Cpp_Token *token = get_first_token_at_line(app, buffer, tokens, line_start);
    
    if (token == 0 && tokens.count == 0){
        // In this case just let the null token pointer be returned.
    }
    else{
        if (token == 0){
            token = tokens.tokens + (tokens.count - 1);
        }
        
        if (token != tokens.tokens){
            --token;
            for (; token > tokens.tokens; --token){
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    switch(token->type){
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
            int32_t line = buffer_get_line_index(app, buffer, token->start);
            int32_t start = buffer_get_line_start(app, buffer, line);
            
            Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, start, tab_width);
            current_indent = hard_start.indent_pos;
            
            Cpp_Token *start_token = get_first_token_at_line(app, buffer, tokens, line);
            Cpp_Token *brace_token = token;
            
            if (start_token->type == CPP_TOKEN_PARENTHESE_OPEN){
                if (start_token == tokens.tokens){
                    found_safe_start_position = 1;
                }
                else{
                    token = start_token-1;
                }
            }
            else{
                int32_t close = 0;
                
                for (token = brace_token; token > start_token; --token){
                    switch(token->type){
                        case CPP_TOKEN_PARENTHESE_CLOSE:
                        case CPP_TOKEN_BRACKET_CLOSE:
                        case CPP_TOKEN_BRACE_CLOSE:
                        close = token->type;
                        goto out_of_loop2;
                    }
                }
                out_of_loop2:;
                
                switch (close){
                    case 0: token = start_token; found_safe_start_position = 1; break;
                    
                    case CPP_TOKEN_PARENTHESE_CLOSE:
                    token = seek_matching_token_backwards(tokens, token-1,
                                                          CPP_TOKEN_PARENTHESE_OPEN,
                                                          CPP_TOKEN_PARENTHESE_CLOSE);
                    break;
                    
                    case CPP_TOKEN_BRACKET_CLOSE:
                    token = seek_matching_token_backwards(tokens, token-1,
                                                          CPP_TOKEN_BRACKET_OPEN,
                                                          CPP_TOKEN_BRACKET_CLOSE);
                    break;
                    
                    case CPP_TOKEN_BRACE_CLOSE:
                    token = seek_matching_token_backwards(tokens, token-1,
                                                          CPP_TOKEN_BRACE_OPEN,
                                                          CPP_TOKEN_BRACE_CLOSE);
                    break;
                }
            }
        } while(found_safe_start_position == 0);
        *current_indent_out = current_indent;
    }
    
    return(token);
}

//
// Indent Rules
//

struct Indent_Parse_State{
    int32_t current_indent;
    int32_t previous_line_indent;
    int32_t paren_nesting;
    int32_t paren_anchor_indent[16];
    int32_t comment_shift;
    int32_t previous_comment_indent;
};

static int32_t*
get_indentation_marks(Application_Links *app, Partition *part, Buffer_Summary *buffer, Cpp_Token_Array tokens, int32_t line_start, int32_t line_end, bool32 exact_align, int32_t tab_width){
    
    int32_t indent_mark_count = line_end - line_start;
    int32_t *indent_marks = push_array(part, int32_t, indent_mark_count);
    // Shift the array so line_index works correctly.
    indent_marks -= line_start;
    
    
    // Decide where to start indentation parsing.
    Indent_Parse_State indent = {0};
    Cpp_Token *token_ptr = find_anchor_token(app, buffer, tokens, line_start,
                                             tab_width, &indent.current_indent);
    
    if (token_ptr == 0){
        for (int32_t line_index = line_start; line_index < line_end; ++line_index){
            indent_marks[line_index] = 0;
        }
    }
    else{
        int32_t line_index = buffer_get_line_index(app, buffer, token_ptr->start);
        
        if (line_index > line_start){
            line_index = line_start;
        }
        
        uint32_t next_line_start_pos = buffer_get_line_start(app, buffer, line_index+1);
        
        switch (token_ptr->type){
            case CPP_TOKEN_BRACKET_OPEN: indent.current_indent += tab_width; break;
            case CPP_TOKEN_BRACE_OPEN: indent.current_indent += tab_width; break;
            case CPP_TOKEN_PARENTHESE_OPEN: indent.current_indent += tab_width; break;
        }
        
        indent.previous_line_indent = indent.current_indent;
        
        for (;line_index < line_end;){
            Cpp_Token prev_token = *token_ptr, token = {0};
            
            ++token_ptr;
            if (token_ptr < tokens.tokens + tokens.count){
                token = *token_ptr;
            }
            else{
                token.type = CPP_TOKEN_EOF;
                token.start = buffer->size;
                token.flags = 0;
            }
            
            for (;token.start >= next_line_start_pos && line_index < line_end;){
                next_line_start_pos = buffer_get_line_start(app, buffer, line_index+1);
                
                int32_t this_indent = 0;
                {
                    int32_t previous_indent = indent.previous_line_indent;
                    
                    uint32_t this_line_start = buffer_get_line_start(app, buffer, line_index);
                    uint32_t next_line_start = buffer_get_line_start(app, buffer, line_index+1);
                    
                    bool32 did_special_behavior = false;
                    
                    if (prev_token.start <= this_line_start && prev_token.start + prev_token.size > this_line_start){
                        if (prev_token.type == CPP_TOKEN_COMMENT){
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
                                if (line_index >= line_start){
                                    indent.previous_comment_indent = this_indent;
                                }
                                else{
                                    indent.previous_comment_indent = hard_start.indent_pos;
                                }
                            }
                            
                            did_special_behavior = true;
                        }
                        else if (prev_token.type == CPP_TOKEN_STRING_CONSTANT){
                            this_indent = previous_indent;
                            did_special_behavior = true;
                        }
                    }
                    
                    if (!did_special_behavior){
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
                                        if (!(prev_token.flags & CPP_TFLAG_PP_BODY) && !(prev_token.flags & CPP_TFLAG_PP_DIRECTIVE)){
                                            switch (prev_token.type){
                                                case CPP_TOKEN_BRACKET_OPEN: case CPP_TOKEN_BRACE_OPEN: case CPP_TOKEN_BRACE_CLOSE: case CPP_TOKEN_SEMICOLON: case CPP_TOKEN_COLON: case CPP_TOKEN_COMMA: case CPP_TOKEN_COMMENT: break;
                                                default: this_indent += tab_width;
                                            }
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
                
                if (line_index >= line_start){
                    indent_marks[line_index] = this_indent;
                }
                ++line_index;
                
                indent.previous_line_indent = this_indent;
            }
            
            // Update indent state.
            switch (token.type){
                case CPP_TOKEN_BRACKET_OPEN: indent.current_indent += 4; break;
                case CPP_TOKEN_BRACKET_CLOSE: indent.current_indent -= 4; break;
                case CPP_TOKEN_BRACE_OPEN: indent.current_indent += 4; break;
                case CPP_TOKEN_BRACE_CLOSE: indent.current_indent -= 4; break;
                
                case CPP_TOKEN_COMMENT:
                {
                    int32_t line = buffer_get_line_index(app, buffer, token.start);
                    int32_t start = buffer_get_line_start(app, buffer, line);
                    
                    indent.comment_shift = (indent.current_indent - (token.start - start));
                    indent.previous_comment_indent = (token.start - start);
                }break;
                
                case CPP_TOKEN_PARENTHESE_OPEN:
                if (!(token.flags & CPP_TFLAG_PP_BODY)){
                    if (indent.paren_nesting < ArrayCount(indent.paren_anchor_indent)){
                        int32_t line = buffer_get_line_index(app, buffer, token.start);
                        int32_t start = buffer_get_line_start(app, buffer, line);
                        int32_t char_pos = token.start - start;
                        
                        Hard_Start_Result hard_start =
                            buffer_find_hard_start(app, buffer, start, tab_width);
                        
                        int32_t line_pos = hard_start.char_pos - start;
                        
                        indent.paren_anchor_indent[indent.paren_nesting] =
                            char_pos - line_pos + indent.previous_line_indent + 1;
                    }
                    ++indent.paren_nesting;
                }
                break;
                
                case CPP_TOKEN_PARENTHESE_CLOSE:
                if (!(token.flags & CPP_TFLAG_PP_BODY)){
                    if (indent.paren_nesting > 0){
                        --indent.paren_nesting;
                    }
                }
                break;
            }
        }
    }
    
    // Unshift the indent_marks array.
    indent_marks += line_start;
    return(indent_marks);
}

static void
get_indent_lines_minimum(Application_Links *app, Buffer_Summary *buffer,
                         int32_t start_pos, int32_t end_pos,
                         int32_t *line_start_out, int32_t *line_end_out){
    int32_t line_start = buffer_get_line_index(app, buffer, start_pos);
    int32_t line_end = buffer_get_line_index(app, buffer, end_pos) + 1;
    
    *line_start_out = line_start;
    *line_end_out = line_end;
}

static void
get_indent_lines_whole_tokens(Application_Links *app, Buffer_Summary *buffer, Cpp_Token_Array tokens,
                              int32_t start_pos, int32_t end_pos,
                              int32_t *line_start_out, int32_t *line_end_out){
    int32_t line_start = buffer_get_line_index(app, buffer, start_pos);
    int32_t line_end = buffer_get_line_index(app, buffer, end_pos);
    
    for (;line_start > 0;){
        uint32_t line_start_pos = 0;
        Cpp_Token *token = get_first_token_at_line(app, buffer, tokens, line_start, &line_start_pos);
        if (token != 0 && token->start < line_start_pos){
            line_start = buffer_get_line_index(app, buffer, token->start);
        }
        else{
            break;
        }
    }
    
    for (;line_end+1 < buffer->line_count;){
        uint32_t next_line_start_pos = 0;
        Cpp_Token *token = get_first_token_at_line(app, buffer, tokens, line_end+1, &next_line_start_pos);
        if (token && token->start < next_line_start_pos){
            line_end = buffer_get_line_index(app, buffer, token->start+token->size);
        }
        else{
            break;
        }
    }
    if (line_end >= buffer->line_count){
        line_end = buffer->line_count;
    }
    else{
        line_end += 1;
    }
    
    *line_start_out = line_start;
    *line_end_out = line_end;
}

static bool32
buffer_auto_indent(Application_Links *app, Partition *part, Buffer_Summary *buffer,
                   int32_t start, int32_t end, int32_t tab_width, Auto_Indent_Flag flags){
    
    bool32 result = 0;
    if (buffer->exists && buffer->tokens_are_ready){
        result = 1;
        
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
        int32_t *indent_marks =
            get_indentation_marks(app, part, buffer, tokens, line_start, line_end,
                                  (flags & AutoIndent_ExactAlignBlock), tab_width);
        
        // Stage 4: Set the Line Indents
        Indent_Options opts = {0};
        opts.empty_blank_lines = (flags & AutoIndent_ClearLine);
        opts.use_tabs = (flags & AutoIndent_UseTab);
        opts.tab_width = tab_width;
        
        set_line_indents(app, part, buffer, line_start, line_end, indent_marks, opts);
        
        end_temp_memory(temp);
    }
    
    return(result);
}

static bool32
buffer_auto_indent(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t end,
                   int32_t tab_width, Auto_Indent_Flag flags){
    bool32 result = buffer_auto_indent(app, &global_part, buffer, start, end, tab_width, flags);
    return(result);
}

//
// Commands
//

static void
auto_tab_whole_file_by_summary(Application_Links *app, Buffer_Summary *buffer){
    buffer_auto_indent(app, buffer, 0, buffer->size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
}

CUSTOM_COMMAND_SIG(auto_tab_whole_file){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    auto_tab_whole_file_by_summary(app, &buffer);
}

CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    buffer_auto_indent(app, &buffer, view.cursor.pos, view.cursor.pos, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(auto_tab_range){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    Range range = get_range(&view);
    
    buffer_auto_indent(app, &buffer, range.min, range.max, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(write_and_auto_tab){
    exec_command(app, write_character);
    
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    buffer_auto_indent(app, &buffer, view.cursor.pos, view.cursor.pos, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_ExactAlignBlock);
    move_past_lead_whitespace(app, &view, &buffer);
}



#endif

// BOTTOM

