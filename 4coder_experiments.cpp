/*
4coder_experiments.cpp - Supplies extension bindings to the defaults with experimental new features.
*/

// TOP

#include "4coder_default_include.cpp"
#include "4coder_miblo_numbers.cpp"

#define NO_BINDING
#include "4coder_default_bindings.cpp"

#include <string.h>

static float
get_line_y(Application_Links *app, View_ID view, i32 line){
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_line_char(line, 1), &cursor);
    return(cursor.wrapped_y);
}

static Rect_i32
get_line_x_rect(Application_Links *app, View_ID view){
    i32 cursor_pos = 0;
    view_get_cursor_pos(app, view, &cursor_pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(cursor_pos), &cursor);
    
    i32 mark_pos = 0;
    view_get_mark_pos(app, view, &mark_pos);
    Full_Cursor mark = {};
    view_compute_cursor(app, view, seek_pos(mark_pos), &mark);
    
    Rect_i32 rect = {};
    rect.x0 = (i32)mark.wrapped_x;
    rect.x1 = (i32)cursor.wrapped_x;
    rect.y0 = mark.line;
    rect.y1 = cursor.line;
    
    if (rect.y0 > rect.y1){
        Swap(i32, rect.y0, rect.y1);
    }
    if (rect.x0 > rect.x1){
        Swap(i32, rect.x0, rect.x1);
    }
    
    return(rect);
}

CUSTOM_COMMAND_SIG(kill_rect)
CUSTOM_DOC("Delete characters in a rectangular region. Range testing is done by unwrapped-xy coordinates.")
{
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    
    i32_Rect rect = get_line_x_rect(app, view);
    
    for (i32 line = rect.y1; line >= rect.y0; --line){
        Full_Cursor cursor = {};
        f32 y = get_line_y(app, view, line);
        if (view_compute_cursor(app, view, seek_wrapped_xy((float)rect.x0, y, 0), &cursor)){
            i32 start = cursor.pos;
            if (view_compute_cursor(app, view, seek_wrapped_xy((float)rect.x1, y, 0), &cursor)){
                i32 end = cursor.pos;
                buffer_replace_range(app, buffer, make_range(start, end), string_u8_litexpr(""));
            }
        }
    }
}

static void
pad_buffer_line(Application_Links *app, Buffer_ID buffer, i32 line, char padchar, i32 target){
    Partial_Cursor start = {};
    Partial_Cursor end = {};
    
    if (buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &start)){
        if (buffer_compute_cursor(app, buffer, seek_line_char(line, 65536), &end)){
            if (start.line == line){
                if (end.character-1 < target){
                    Arena *scratch = context_get_arena(app);
                    Temp_Memory temp = begin_temp(scratch);
                    i32 size = target - (end.character - 1);
                    char *str = push_array(scratch, char, size);
                    block_fill_u8(str, size, ' ');
                    buffer_replace_range(app, buffer, make_range(end.pos), SCu8(str, size));
                    end_temp(temp);
                }
            }
        }
    }
}

/*
NOTE(allen):  Things I learned from this experiment.

First of all the batch edits aren't too bad, but I think
there could be a single system that I run that through that
knows how to build the batch edit from slightly higher level
information. For instance the idea in point 2.

Secondly I definitely believe I need some sort of "mini-buffer"
concept where a view sends commands so that things like
pasting still work.  Then the contents of the "mini-buffer"
can be used to complete the edits at all cursor points.
This doesn't answer all questions, because somehow backspace
still wants to work for multi-lines even when the "mini-buffer"
is emtpy.  Such a system would also make it possible to send
paste commands and cursor navigation to interactive bars.

Thirdly any system like this will probably not want to
operate through the co-routine system, because that makes
sending these commands to the "mini-buffer" much more
difficult.

Fourthly I desperately need some way to do multi-highlighting
multi-cursor showing but it is unclear to me how to do that
conveniently.  Since this won't exist inside a coroutine
what does such an API even look like??? It's clear to me now
that I may need to start pushing for the view routine before
I am even able to support the GUI. Because that will set up the
system to allow me to think about the problem in more ways.

Finally I have decided not to pursue this direction any more,
it just seems like the wrong way to do it, so I'll stop without
doing multi-cursor for now.
*/

struct Buffer_Rect{
    i32 char0;
    i32 line0;
    i32 char1;
    i32 line1;
};

CUSTOM_COMMAND_SIG(multi_line_edit)
CUSTOM_DOC("Begin multi-line mode.  In multi-line mode characters are inserted at every line between the mark and cursor.  All characters are inserted at the same character offset into the line.  This mode uses line_char coordinates.")
{
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    
    i32 cursor_pos = 0;
    view_get_cursor_pos(app, view, &cursor_pos);
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_pos(cursor_pos), &cursor);
    i32 mark_pos = 0;
    view_get_mark_pos(app, view, &mark_pos);
    Full_Cursor mark = {};
    view_compute_cursor(app, view, seek_pos(mark_pos), &mark);
    Buffer_Rect rect = {};
    rect.char0 = mark.character;
    rect.line0 = mark.line;
    rect.char1 = cursor.character;
    rect.line1 = cursor.line;
    if (rect.line0 > rect.line1){
        Swap(i32, rect.line0, rect.line1);
    }
    if (rect.char0 > rect.char1){
        Swap(i32, rect.char0, rect.char1);
    }
    
    i32 start_line = cursor.line;
    i32 pos = cursor.character - 1;
    
    for (i32 i = rect.line0; i <= rect.line1; ++i){
        pad_buffer_line(app, buffer, i, ' ', pos);
    }
    
    i32 line_count = rect.line1 - rect.line0 + 1;
    
    for (;;){
        User_Input in = get_user_input(app, EventOnAnyKey, EventOnEsc|EventOnMouseLeftButton|EventOnMouseRightButton);
        if (in.abort) break;
        
        if (in.key.character && key_is_unmodified(&in.key)){
            char str = (char)in.key.character;
            
            Arena *scratch = context_get_arena(app);
            Temp_Memory temp = begin_temp(scratch);
            Buffer_Edit *edit = push_array(scratch, Buffer_Edit, line_count);
            Buffer_Edit *edits = edit;
            
            for (i32 i = rect.line0; i <= rect.line1; ++i){
                Partial_Cursor partial_cursor = {};
                if (buffer_compute_cursor(app, buffer, seek_line_char(i, pos + 1), &partial_cursor)){
                    edit->str_start = 0;
                    edit->len = 1;
                    edit->start = cursor.pos;
                    edit->end = cursor.pos;
                    ++edit;
                }
            }
            
            i32 edit_count = (int)(edit - edits);
            buffer_batch_edit(app, buffer, &str, edits, edit_count);
            
            end_temp(temp);
            
            ++pos;
            
            view_set_cursor(app, view, seek_line_char(start_line, pos + 1), true);
        }
        else if (in.key.keycode == key_back){
            if (pos > 0){
                Arena *scratch = context_get_arena(app);
                Temp_Memory temp = begin_temp(scratch);
                Buffer_Edit *edit = push_array(scratch, Buffer_Edit, line_count);
                Buffer_Edit *edits = edit;
                
                for (i32 i = rect.line0; i <= rect.line1; ++i){
                    Partial_Cursor partial_cursor = {};
                    if (buffer_compute_cursor(app, buffer, seek_line_char(i, pos + 1), &partial_cursor)){
                        edit->str_start = 0;
                        edit->len = 0;
                        edit->start = cursor.pos-1;
                        edit->end = cursor.pos;
                        ++edit;
                    }
                }
                
                i32 edit_count = (int)(edit - edits);
                buffer_batch_edit(app, buffer, 0, edits, edit_count);
                
                end_temp(temp);
                
                --pos;
            }
        }
        else{
            break;
        }
    }
}

// NOTE(allen): An experimental mutli-pasting thing

CUSTOM_COMMAND_SIG(multi_paste){
    Scratch_Block scratch(app);
    
    i32 count = 0;
    clipboard_count(app, 0, &count);
    if (count > 0){
        View_ID view = 0;
        get_active_view(app, AccessOpen, &view);
        Managed_Scope scope = 0;
        view_get_managed_scope(app, view, &scope);
        
        u64 rewrite = 0;
        managed_variable_get(app, scope, view_rewrite_loc, &rewrite);
        if (rewrite == RewritePaste){
            managed_variable_set(app, scope, view_next_rewrite_loc, RewritePaste);
            u64 prev_paste_index = 0;
            managed_variable_get(app, scope, view_paste_index_loc, &prev_paste_index);
            i32 paste_index = (i32)prev_paste_index + 1;
            managed_variable_set(app, scope, view_paste_index_loc, paste_index);
            
            String_Const_u8 string = {};
            clipboard_index(app, 0, paste_index, scratch, &string);
            
            String_Const_u8 insert_string = push_u8_stringf(scratch, "\n%.*s", string_expand(string));
            
            Buffer_ID buffer = 0;
            view_get_buffer(app, view, AccessOpen, &buffer);
            Range range = get_view_range(app, view);
            buffer_replace_range(app, buffer, make_range(range.max), insert_string);
            view_set_mark(app, view, seek_pos(range.max + 1));
            view_set_cursor(app, view, seek_pos((i32)(range.max + insert_string.size)), true);
            
            // TODO(allen): Send this to all views.
            Theme_Color paste = {};
            paste.tag = Stag_Paste;
            get_theme_colors(app, &paste, 1);
            view_post_fade(app, view, 0.667f, range.max + 1, (i32)(range.max + insert_string.size), paste.color);
        }
        else{
            paste(app);
        }
    }
}

static Range
multi_paste_range(Application_Links *app, View_ID view, Range range, i32 paste_count, b32 old_to_new){
    Scratch_Block scratch(app);
    
    Range finish_range = range;
    if (paste_count >= 1){
        Buffer_ID buffer = 0;
        if (view_get_buffer(app, view, AccessOpen, &buffer)){
            i32 total_size = 0;
            for (i32 paste_index = 0; paste_index < paste_count; ++paste_index){
                total_size += 1 + clipboard_index(app, 0, paste_index, 0, 0);
            }
            total_size -= 1;
            
            if (total_size <= app->memory_size){
                i32 first = paste_count - 1;
                i32 one_past_last = -1;
                i32 step = -1;
                if (!old_to_new){
                    first = 0;
                    one_past_last = paste_count;
                    step = 1;
                }
                
                List_String_Const_u8 list = {};
                
                for (i32 paste_index = first; paste_index != one_past_last; paste_index += step){
                    if (paste_index != first){
                        string_list_push(scratch, &list, SCu8("\n", 1));
                    }
                    String_Const_u8 string = {};
                    if (clipboard_index(app, 0, paste_index, scratch, &string)){
                        string_list_push(scratch, &list, string);
                    }
                }
                
                String_Const_u8 flattened = string_list_flatten(scratch, list);
                
                i32 pos = range.min;
                buffer_replace_range(app, buffer, range, flattened);
                finish_range.min = pos;
                finish_range.max = pos + total_size;
                view_set_mark(app, view, seek_pos(finish_range.min));
                view_set_cursor(app, view, seek_pos(finish_range.max), true);
                
                // TODO(allen): Send this to all views.
                Theme_Color paste;
                paste.tag = Stag_Paste;
                get_theme_colors(app, &paste, 1);
                view_post_fade(app, view, 0.667f, finish_range.min, finish_range.max, paste.color);
            }
        }
    }
    return(finish_range);
}

static void
multi_paste_interactive_up_down(Application_Links *app, i32 paste_count, i32 clip_count){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    
    Range range = make_range(pos);;
    
    b32 old_to_new = true;
    
    range = multi_paste_range(app, view, range, paste_count, old_to_new);
    
    Query_Bar bar = {};
    bar.prompt = string_u8_litexpr("Up and Down to condense and expand paste stages; R to reverse order; Return to finish; Escape to abort.");
    if (start_query_bar(app, &bar, 0) == 0) return;
    
    User_Input in = {};
    for (;;){
        in = get_user_input(app, EventOnAnyKey, EventOnEsc);
        if (in.abort) break;
        
        b32 did_modify = false;
        if (in.key.keycode == key_up){
            if (paste_count > 1){
                --paste_count;
                did_modify = true;
            }
        }
        else if (in.key.keycode == key_down){
            if (paste_count < clip_count){
                ++paste_count;
                did_modify = true;
            }
        }
        else if (in.key.keycode == 'r' || in.key.keycode == 'R'){
            old_to_new = !old_to_new;
            did_modify = true;
        }
        else if (in.key.keycode == '\n'){
            break;
        }
        
        if (did_modify){
            range = multi_paste_range(app, view, range, paste_count, old_to_new);
        }
    }
    
    if (in.abort){
        Buffer_ID buffer = 0;
        view_get_buffer(app, view, AccessOpen, &buffer);
        buffer_replace_range(app, buffer, range, SCu8(""));
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive){
    i32 clip_count = 0;
    clipboard_count(app, 0, &clip_count);
    if (clip_count > 0){
        multi_paste_interactive_up_down(app, 1, clip_count);
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive_quick){
    i32 clip_count = 0;
    clipboard_count(app, 0, &clip_count);
    if (clip_count > 0){
        u8 string_space[256];
        Query_Bar bar = {};
        bar.prompt = string_u8_litexpr("How Many Slots To Paste: ");
        bar.string = SCu8(string_space, (umem)0);
        bar.string_capacity = sizeof(string_space);
        query_user_number(app, &bar);
        
        i32 initial_paste_count = (i32)string_to_integer(bar.string, 10);
        initial_paste_count = clamp(1, initial_paste_count, clip_count);
        end_query_bar(app, &bar, 0);
        
        multi_paste_interactive_up_down(app, initial_paste_count, clip_count);
    }
}

// NOTE(allen): Some basic code manipulation ideas.

CUSTOM_COMMAND_SIG(rename_parameter)
CUSTOM_DOC("If the cursor is found to be on the name of a function parameter in the signature of a function definition, all occurences within the scope of the function will be replaced with a new provided string.")
{
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    i32 cursor_pos = 0;
    view_get_cursor_pos(app, view, &cursor_pos);
    
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    
    Cpp_Get_Token_Result result;
    if (buffer_get_token_index(app, buffer, cursor_pos, &result)){
        if (!result.in_whitespace_after_token){
            static const i32 stream_space_size = 512;
            Cpp_Token stream_space[stream_space_size];
            Stream_Tokens_DEP stream = {};
            
            if (init_stream_tokens(&stream, app, buffer, result.token_index, stream_space, stream_space_size)){
                i32 token_index = result.token_index;
                Cpp_Token token = stream.tokens[token_index];
                
                if (token.type == CPP_TOKEN_IDENTIFIER){
                    Cpp_Token original_token = token;
                    String_Const_u8 old_lexeme = push_token_lexeme(app, scratch, buffer, token);
                    
                    i32 proc_body_found = 0;
                    b32 still_looping = 0;
                    
                    ++token_index;
                    do{
                        for (; token_index < stream.end; ++token_index){
                            Cpp_Token *token_ptr = stream.tokens + token_index;
                            switch (token_ptr->type){
                                case CPP_TOKEN_BRACE_OPEN:
                                {
                                    proc_body_found = 1;
                                    goto doublebreak;
                                }break;
                                
                                case CPP_TOKEN_BRACE_CLOSE:
                                case CPP_TOKEN_PARENTHESE_OPEN:
                                {
                                    goto doublebreak; 
                                }break;
                            }
                        }
                        still_looping = forward_stream_tokens(&stream);
                    }while(still_looping);
                    doublebreak:;
                    
                    if (proc_body_found){
                        
                        u8 with_space[1024];
                        Query_Bar with = {};
                        with.prompt = string_u8_litexpr("New Name: ");
                        with.string = SCu8(with_space, (umem)0);
                        with.string_capacity = sizeof(with_space);
                        if (!query_user_string(app, &with)) return;
                        
                        String_Const_u8 replace_string = with.string;
                        
                        // TODO(allen): fix this up to work with arena better
                        i32 edit_max = Thousand(100);
                        Buffer_Edit *edits = push_array(scratch, Buffer_Edit, edit_max);
                        i32 edit_count = 0;
                        
                        if (edit_max >= 1){
                            Buffer_Edit edit = {};
                            edit.str_start = 0;
                            edit.len = (i32)replace_string.size;
                            edit.start = original_token.start;
                            edit.end = original_token.start + original_token.size;
                            
                            edits[edit_count] = edit;
                            edit_count += 1;
                        }
                        
                        i32 nesting_level = 0;
                        i32 closed_correctly = 0;
                        ++token_index;
                        still_looping = 0;
                        do{
                            for (; token_index < stream.end; ++token_index){
                                Cpp_Token *token_ptr = stream.tokens + token_index;
                                switch (token_ptr->type){
                                    case CPP_TOKEN_IDENTIFIER:
                                    {
                                        if (token_ptr->size == old_lexeme.size){
                                            String_Const_u8 other_lexeme = push_token_lexeme(app, scratch, buffer, *token_ptr);
                                            if (string_match(old_lexeme, other_lexeme)){
                                                Buffer_Edit edit = {};
                                                edit.str_start = 0;
                                                edit.len = (i32)replace_string.size;
                                                edit.start = token_ptr->start;
                                                edit.end = token_ptr->start + token_ptr->size;
                                                if (edit_count < edit_max){
                                                    edits[edit_count] = edit;
                                                    edit_count += 1;
                                                }
                                                else{
                                                    goto doublebreak2;
                                                }
                                            }
                                        }
                                    }break;
                                    
                                    case CPP_TOKEN_BRACE_OPEN:
                                    {
                                        ++nesting_level;
                                    }break;
                                    
                                    case CPP_TOKEN_BRACE_CLOSE:
                                    {
                                        if (nesting_level == 0){
                                            closed_correctly = 1;
                                            goto doublebreak2;
                                        }
                                        else{
                                            --nesting_level;
                                        }
                                    }break;
                                }
                            }
                            still_looping = forward_stream_tokens(&stream);
                        }while(still_looping);
                        doublebreak2:;
                        
                        if (closed_correctly){
                            buffer_batch_edit(app, buffer, (char*)replace_string.str, edits, edit_count);
                        }
                    }
                }
            }
        }
    }
    
    end_temp(temp);
}

typedef u32 Write_Explicit_Enum_Values_Mode;
enum{
    WriteExplicitEnumValues_Integers,
    WriteExplicitEnumValues_Flags,
};

static void
write_explicit_enum_values_parameters(Application_Links *app, Write_Explicit_Enum_Values_Mode mode){
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    
    i32 pos = 0;
    view_get_cursor_pos(app, view, &pos);
    
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    
    Cpp_Get_Token_Result result;
    if (buffer_get_token_index(app, buffer, pos, &result)){
        if (!result.in_whitespace_after_token){
            Cpp_Token stream_space[32];
            Stream_Tokens_DEP stream = {};
            
            if (init_stream_tokens(&stream, app, buffer, result.token_index, stream_space, 32)){
                i32 token_index = result.token_index;
                Cpp_Token token = stream.tokens[token_index];
                
                if (token.type == CPP_TOKEN_BRACE_OPEN){
                    ++token_index;
                    
                    i32 seeker_index = token_index;
                    Stream_Tokens_DEP seek_stream = begin_temp_stream_token(&stream);
                    
                    b32 closed_correctly = false;
                    b32 still_looping = false;
                    do{
                        for (; seeker_index < stream.end; ++seeker_index){
                            Cpp_Token *token_seeker = stream.tokens + seeker_index;
                            switch (token_seeker->type){
                                case CPP_TOKEN_BRACE_CLOSE:
                                closed_correctly = true;
                                goto finished_seek;
                                
                                case CPP_TOKEN_BRACE_OPEN:
                                goto finished_seek;
                            }
                        }
                        still_looping = forward_stream_tokens(&stream);
                    }while(still_looping);
                    finished_seek:;
                    end_temp_stream_token(&stream, seek_stream);
                    
                    if (closed_correctly){
                        i32 count_estimate = 1 + (seeker_index - token_index)/2;
                        
                        i32 edit_count = 0;
                        Buffer_Edit *edits = push_array(scratch, Buffer_Edit, count_estimate);
                        
                        List_String_Const_char list = {};
                        
                        closed_correctly = false;
                        still_looping = false;
                        u32 value = 0;
                        if (mode == WriteExplicitEnumValues_Flags){
                            value = 1;
                        }
                        
                        do{
                            for (;token_index < stream.end; ++token_index){
                                Cpp_Token *token_ptr = stream.tokens + token_index;
                                switch (token_ptr->type){
                                    case CPP_TOKEN_IDENTIFIER:
                                    {
                                        i32 edit_start = token_ptr->start + token_ptr->size;
                                        i32 edit_stop = edit_start;
                                        
                                        i32 edit_is_good = 0;
                                        ++token_index;
                                        do{
                                            for (; token_index < stream.end; ++token_index){
                                                token_ptr = stream.tokens + token_index;
                                                switch (token_ptr->type){
                                                    case CPP_TOKEN_COMMA:
                                                    {
                                                        edit_stop = token_ptr->start;
                                                        edit_is_good = 1;
                                                        goto good_edit;
                                                    }break;
                                                    
                                                    case CPP_TOKEN_BRACE_CLOSE:
                                                    {
                                                        edit_stop = token_ptr->start;
                                                        closed_correctly = 1;
                                                        edit_is_good = 1;
                                                        goto good_edit;
                                                    }break;
                                                }
                                            }
                                            still_looping = forward_stream_tokens(&stream);
                                        }while(still_looping);
                                        
                                        good_edit:;
                                        if (edit_is_good){
                                            i32 str_pos = (i32)(list.total_size);
                                            
                                            string_list_pushf(scratch, &list, " = %d", value);
                                            if (closed_correctly){
                                                string_list_push_lit(scratch, &list, "\n");
                                            }
                                            
                                            switch (mode){
                                                case WriteExplicitEnumValues_Integers:
                                                {
                                                    ++value;
                                                }break;
                                                case WriteExplicitEnumValues_Flags:
                                                {
                                                    if (value < (1 << 31)){
                                                        value <<= 1;
                                                    }
                                                }break;
                                            }
                                            
                                            i32 str_size = (i32)(list.total_size) - str_pos;
                                            
                                            Buffer_Edit edit;
                                            edit.str_start = str_pos;
                                            edit.len = str_size;
                                            edit.start = edit_start;
                                            edit.end = edit_stop;
                                            
                                            Assert(edit_count < count_estimate);
                                            edits[edit_count] = edit;
                                            ++edit_count;
                                        }
                                        if (!edit_is_good || closed_correctly){
                                            goto finished;
                                        }
                                    }break;
                                    
                                    case CPP_TOKEN_BRACE_CLOSE:
                                    {
                                        closed_correctly = 1;
                                        goto finished;
                                    }break;
                                }
                            }
                            
                            still_looping = forward_stream_tokens(&stream);
                        }while(still_looping);
                        
                        finished:;
                        if (closed_correctly){
                            String_Const_char text = string_list_flatten(scratch, list);
                            buffer_batch_edit(app, buffer, text.str, edits, edit_count);
                        }
                    }
                }
            }
        }
    }
    
    end_temp(temp);
}

CUSTOM_COMMAND_SIG(write_explicit_enum_values)
CUSTOM_DOC("If the cursor is found to be on the '{' of an enum definition, the values of the enum will be filled in sequentially starting from zero.  Existing values are overwritten.")
{
    write_explicit_enum_values_parameters(app, WriteExplicitEnumValues_Integers);
}

CUSTOM_COMMAND_SIG(write_explicit_enum_flags)
CUSTOM_DOC("If the cursor is found to be on the '{' of an enum definition, the values of the enum will be filled in to give each a unique power of 2 value, starting from 1.  Existing values are overwritten.")
{
    write_explicit_enum_values_parameters(app, WriteExplicitEnumValues_Flags);
}

//
// Rename All
//

struct Replace_Target{
    Replace_Target *next;
    Buffer_ID buffer_id;
    i32 start_pos;
};

static void
replace_all_occurrences_parameters(Application_Links *app, Heap *heap, String_Const_u8 target_string, String_Const_u8 new_string){
    if (target_string.size > 0){
        global_history_edit_group_begin(app);
        
        // Initialize a generic search all buffers
        Search_Set set = {};
        Search_Iter iter = {};
        initialize_generic_search_all_buffers(app, heap, &target_string, 1, SearchFlag_MatchSubstring, 0, 0, &set, &iter);
        
        // Visit all locations and create replacement list
        Arena *scratch = context_get_arena(app);
        Temp_Memory temp = begin_temp(scratch);
        
        Replace_Target *target_first = 0;
        Replace_Target *target_last = 0;
        i32 target_count = 0;
        
        for (Search_Match match = search_next_match(app, &set, &iter);
             match.found_match;
             match = search_next_match(app, &set, &iter)){
            
            Replace_Target *new_target = push_array(scratch, Replace_Target, 1);
            sll_queue_push(target_first, target_last, new_target);
            ++target_count;
            new_target->buffer_id = match.buffer;
            new_target->start_pos = match.start;
        }
        
        // Use replacement list to do replacements
        imem shift_per_replacement = new_string.size - target_string.size;
        imem current_offset = 0;
        Buffer_ID current_buffer_id = 0;
        for (Replace_Target *target = target_first;
             target != 0;
             target = target->next){
            if (target->buffer_id != current_buffer_id){
                current_buffer_id = target->buffer_id;
                current_offset = 0;
            }
            i32 pos = target->start_pos + (i32)current_offset;
            buffer_replace_range(app, target->buffer_id, make_range(pos, pos + (i32)target_string.size), new_string);
            current_offset += shift_per_replacement;
        }
        
        end_temp(temp);
        
        global_history_edit_group_end(app);
    }
}

CUSTOM_COMMAND_SIG(replace_all_occurrences)
CUSTOM_DOC("Queries the user for two strings, and replaces all occurrences of the first string with the second string in all open buffers.")
{
    u8 replace_space[1024];
    Query_Bar replace = {};
    replace.prompt = string_u8_litexpr("Replace (In All Buffers): ");
    replace.string = SCu8(replace_space, (umem)0);
    replace.string_capacity = sizeof(replace_space);
    
    u8 with_space[1024];
    Query_Bar with = {};
    with.prompt = string_u8_litexpr("With: ");
    with.string = SCu8(with_space, (umem)0);
    with.string_capacity = sizeof(with_space);
    
    if (!query_user_string(app, &replace)) return;
    if (replace.string.size == 0) return;
    
    if (!query_user_string(app, &with)) return;
    
    String_Const_u8 r = replace.string;
    String_Const_u8 w = with.string;
    replace_all_occurrences_parameters(app, &global_heap, r, w);
}

extern "C" i32
get_bindings(void *data, i32 size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_buffer_viewer_update, default_view_adjust);
    
    set_start_hook(context, default_start);
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    set_end_file_hook(context, end_file_close_jump_list);
    
    set_input_filter(context, default_suppress_mouse_filter);
    set_command_caller(context, default_command_caller);
    set_render_caller(context, default_render_caller);
    
    set_scroll_rule(context, smooth_scroll_rule);
    set_buffer_name_resolver(context, default_buffer_name_resolution);
    set_modify_color_table_hook(context, default_modify_color_table);
    set_get_view_buffer_region_hook(context, default_view_buffer_region);
    
    default_keys(context);
    
    // NOTE(allen|a4.0.6): Command maps can be opened more than
    // once so that you can extend existing maps very easily.
    // You can also use the helper "restart_map" instead of
    // begin_map to clear everything that was in the map and
    // bind new things instead.
    begin_map(context, mapid_file);
    bind(context, key_back, MDFR_ALT|MDFR_CTRL, kill_rect);
    bind(context, ' ', MDFR_ALT|MDFR_CTRL, multi_line_edit);
    
    bind(context, key_page_up, MDFR_ALT, miblo_increment_time_stamp);
    bind(context, key_page_down, MDFR_ALT, miblo_decrement_time_stamp);
    
    bind(context, key_home, MDFR_ALT, miblo_increment_time_stamp_minute);
    bind(context, key_end, MDFR_ALT, miblo_decrement_time_stamp_minute);
    
    bind(context, 'b', MDFR_CTRL, multi_paste_interactive_quick);
    bind(context, 'A', MDFR_CTRL, replace_all_occurrences);
    end_map(context);
    
    begin_map(context, default_code_map);
    bind(context, key_insert, MDFR_CTRL, write_explicit_enum_values);
    bind(context, key_insert, MDFR_CTRL|MDFR_SHIFT, write_explicit_enum_flags);
    bind(context, 'p', MDFR_ALT, rename_parameter);
    end_map(context);
    
    return(end_bind_helper(context));
}

// BOTTOM

