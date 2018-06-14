/*
4coder_experiments.cpp - Supplies extension bindings to the defaults with experimental new features.
*/

// TOP

#if !defined(FCODER_EXPERIMENTS_CPP)
#define FCODER_EXPERIMENTS_CPP

#include "4coder_default_include.cpp"
#include "4coder_miblo_numbers.cpp"

#define NO_BINDING
#include "4coder_default_bindings.cpp"

#include <string.h>

static float
get_line_y(Application_Links *app, View_Summary *view, int32_t line){
    Full_Cursor cursor = {0};
    view_compute_cursor(app, view, seek_line_char(line, 1), &cursor);
    float y = cursor.wrapped_y;
    if (view->unwrapped_lines){
        y = cursor.unwrapped_y;
    }
    return(y);
}

CUSTOM_COMMAND_SIG(kill_rect)
CUSTOM_DOC("Delete characters in a rectangular region. Range testing is done by unwrapped-xy coordinates.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    i32_Rect rect = get_line_x_rect(&view);
    
    bool32 unwrapped = view.unwrapped_lines;
    
    for (int32_t line = rect.y1; line >= rect.y0; --line){
        int32_t start = 0;
        int32_t end = 0;
        
        bool32 success = 1;
        Full_Cursor cursor = {0};
        
        float y = get_line_y(app, &view, line);
        
        if (success){
            success = view_compute_cursor(app, &view, seek_xy((float)rect.x0, y, 0, unwrapped), &cursor);
        }
        start = cursor.pos;
        
        if (success){
            success = view_compute_cursor(app, &view, seek_xy((float)rect.x1, y, 0, unwrapped), &cursor);
        }
        end = cursor.pos;
        
        if (success){
            buffer_replace_range(app, &buffer, start, end, 0, 0);
        }
    }
}

static void
pad_buffer_line(Application_Links *app, Partition *part, Buffer_Summary *buffer, int32_t line, char padchar, int32_t target){
    Partial_Cursor start = {0};
    Partial_Cursor end = {0};
    
    if (buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &start)){
        if (buffer_compute_cursor(app, buffer, seek_line_char(line, 65536), &end)){
            if (start.line == line){
                if (end.character-1 < target){
                    Temp_Memory temp = begin_temp_memory(part);
                    int32_t size = target - (end.character-1);
                    char *str = push_array(part, char, size);
                    memset(str, ' ', size);
                    buffer_replace_range(app, buffer, end.pos, end.pos, str, size);
                    end_temp_memory(temp);
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

CUSTOM_COMMAND_SIG(multi_line_edit)
CUSTOM_DOC("Begin multi-line mode.  In multi-line mode characters are inserted at every line between the mark and cursor.  All characters are inserted at the same character offset into the line.  This mode uses line_char coordinates.")
{
    Partition *part = &global_part;
    
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Buffer_Rect rect = get_rect(&view);
    
    int32_t start_line = view.cursor.line;
    int32_t pos = view.cursor.character-1;
    
    for (int32_t i = rect.line0; i <= rect.line1; ++i){
        pad_buffer_line(app, &global_part, &buffer, i, ' ', pos);
    }
    
    int32_t line_count = rect.line1 - rect.line0 + 1;
    
    for (;;){
        User_Input in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        if (in.abort) break;
        
        if (in.key.character && key_is_unmodified(&in.key)){
            char str = (char)in.key.character;
            
            Temp_Memory temp = begin_temp_memory(part);
            Buffer_Edit *edit = push_array(part, Buffer_Edit, line_count);
            Buffer_Edit *edits = edit;
            
            for (int32_t i = rect.line0; i <= rect.line1; ++i){
                Partial_Cursor cursor = {0};
                
                if (buffer_compute_cursor(app, &buffer, seek_line_char(i, pos+1), &cursor)){
                    edit->str_start = 0;
                    edit->len = 1;
                    edit->start = cursor.pos;
                    edit->end = cursor.pos;
                    ++edit;
                }
            }
            
            int32_t edit_count = (int)(edit - edits);
            buffer_batch_edit(app, &buffer, &str, 1, edits, edit_count, BatchEdit_Normal);
            
            end_temp_memory(temp);
            
            ++pos;
            
            view_set_cursor(app, &view, seek_line_char(start_line, pos+1), true);
        }
        else if (in.key.keycode == key_back){
            if (pos > 0){
                
                Temp_Memory temp = begin_temp_memory(part);
                Buffer_Edit *edit = push_array(part, Buffer_Edit, line_count);
                Buffer_Edit *edits = edit;
                
                for (int32_t i = rect.line0; i <= rect.line1; ++i){
                    Partial_Cursor cursor = {0};
                    
                    if (buffer_compute_cursor(app, &buffer, seek_line_char(i, pos+1), &cursor)){
                        edit->str_start = 0;
                        edit->len = 0;
                        edit->start = cursor.pos-1;
                        edit->end = cursor.pos;
                        ++edit;
                    }
                }
                
                int32_t edit_count = (int)(edit - edits);
                buffer_batch_edit(app, &buffer, 0, 0, edits, edit_count, BatchEdit_Normal);
                
                end_temp_memory(temp);
                
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
    uint32_t access = AccessOpen;
    int32_t count = clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = get_active_view(app, access);
        
        if (view_paste_index[view.view_id].rewrite == RewritePaste){
            view_paste_index[view.view_id].next_rewrite = RewritePaste;
            
            int32_t paste_index = view_paste_index[view.view_id].index + 1;
            view_paste_index[view.view_id].index = paste_index;
            
            int32_t len = clipboard_index(app, 0, paste_index, 0, 0);
            
            
            if (len + 1 <= app->memory_size){
                char *str = (char*)app->memory;
                str[0] = '\n';
                clipboard_index(app, 0, paste_index, str + 1, len);
                
                Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
                Range range = get_view_range(&view);
                buffer_replace_range(app, &buffer, range.max, range.max, str, len + 1);
                view_set_mark(app, &view, seek_pos(range.max + 1));
                view_set_cursor(app, &view, seek_pos(range.max + len + 1), true);
                
                // TODO(allen): Send this to all views.
                Theme_Color paste;
                paste.tag = Stag_Paste;
                get_theme_colors(app, &paste, 1);
                view_post_fade(app, &view, 0.667f, range.max + 1, range.max + len + 1, paste.color);
            }
        }
        else{
            exec_command(app, paste);
        }
    }
}

static Range
multi_paste_range(Application_Links *app, View_Summary *view, Range range, int32_t paste_count, bool32 old_to_new){
    Range finish_range = range;
    if (paste_count >= 1){
        Buffer_Summary buffer = get_buffer(app, view->buffer_id, AccessOpen);
        if (buffer.exists){
            int32_t total_size = 0;
            for (int32_t paste_index = 0; paste_index < paste_count; ++paste_index){
                total_size += 1 + clipboard_index(app, 0, paste_index, 0, 0);
            }
            total_size -= 1;
            
            if (total_size <= app->memory_size){
                char *str = (char*)app->memory;
                int32_t position = 0;
                
                int32_t first = paste_count - 1;
                int32_t one_past_last = -1;
                int32_t step = -1;
                
                if (!old_to_new){
                    first = 0;
                    one_past_last = paste_count;
                    step = 1;
                }
                
                for (int32_t paste_index = first; paste_index != one_past_last; paste_index += step){
                    if (paste_index != first){
                        str[position] = '\n';
                        ++position;
                    }
                    
                    int32_t len = clipboard_index(app, 0, paste_index, str + position, total_size - position);
                    position += len;
                }
                
                int32_t pos = range.min;
                buffer_replace_range(app, &buffer, range.min, range.max, str, total_size);
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
multi_paste_interactive_up_down(Application_Links *app, int32_t paste_count, int32_t clip_count){
    View_Summary view = get_active_view(app, AccessOpen);
    
    Range range = {0};
    range.min = range.max = view.cursor.pos;
    
    bool32 old_to_new = true;
    
    range = multi_paste_range(app, &view, range, paste_count, old_to_new);
    
    Query_Bar bar = {0};
    bar.prompt = make_lit_string("Up and Down to condense and expand paste stages; R to reverse order; Return to finish; Escape to abort.");
    if (start_query_bar(app, &bar, 0) == 0) return;
    
    User_Input in = {0};
    for (;;){
        in = get_user_input(app, EventOnAnyKey, EventOnEsc);
        if (in.abort) break;
        Assert(in.type == UserInputKey);
        
        bool32 did_modify = false;
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
            range = multi_paste_range(app, &view, range, paste_count, old_to_new);
        }
    }
    
    if (in.abort){
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive){
    int32_t clip_count = clipboard_count(app, 0);
    if (clip_count > 0){
        multi_paste_interactive_up_down(app, 1, clip_count);
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive_quick){
    int32_t clip_count = clipboard_count(app, 0);
    if (clip_count > 0){
        char string_space[256];
        Query_Bar bar = {0};
        bar.prompt = make_lit_string("How Many Slots To Paste: ");
        bar.string = make_fixed_width_string(string_space);
        query_user_number(app, &bar);
        
        int32_t initial_paste_count = str_to_int_s(bar.string);
        if (initial_paste_count > clip_count){
            initial_paste_count = clip_count;
        }
        if (initial_paste_count < 1){
            initial_paste_count = 1;
        }
        
        end_query_bar(app, &bar, 0);
        
        multi_paste_interactive_up_down(app, initial_paste_count, clip_count);
    }
}

// NOTE(allen): Some basic code manipulation ideas.

CUSTOM_COMMAND_SIG(rename_parameter)
CUSTOM_DOC("If the cursor is found to be on the name of a function parameter in the signature of a function definition, all occurences within the scope of the function will be replaced with a new provided string.")
{
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    Cpp_Get_Token_Result result;
    if (buffer_get_token_index(app, &buffer, view.cursor.pos, &result)){
        if (!result.in_whitespace){
            static const int32_t stream_space_size = 512;
            Cpp_Token stream_space[stream_space_size];
            Stream_Tokens stream = {0};
            
            if (init_stream_tokens(&stream, app, &buffer, result.token_index, stream_space, stream_space_size)){
                int32_t token_index = result.token_index;
                Cpp_Token token = stream.tokens[token_index];
                
                if (token.type == CPP_TOKEN_IDENTIFIER){
                    
                    char old_lexeme_base[128];
                    String old_lexeme = make_fixed_width_string(old_lexeme_base);
                    
                    if (token.size < sizeof(old_lexeme_base)){
                        Cpp_Token original_token = token;
                        old_lexeme.size = token.size;
                        buffer_read_range(app, &buffer, token.start, token.start+token.size, old_lexeme.str);
                        
                        int32_t proc_body_found = 0;
                        bool32 still_looping = 0;
                        
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
                            
                            Query_Bar with;
                            char with_space[1024];
                            with.prompt = make_lit_string("New Name: ");
                            with.string = make_fixed_width_string(with_space);
                            if (!query_user_string(app, &with)) return;
                            
                            String replace_string = with.string;
                            
                            Buffer_Edit *edits = (Buffer_Edit*)partition_current(part);
                            int32_t edit_max = (partition_remaining(part))/sizeof(Buffer_Edit);
                            int32_t edit_count = 0;
                            
                            if (edit_max >= 1){
                                Buffer_Edit edit;
                                edit.str_start = 0;
                                edit.len = replace_string.size;
                                edit.start = original_token.start;
                                edit.end = original_token.start + original_token.size;
                                
                                edits[edit_count] = edit;
                                ++edit_count;
                            }
                            
                            int32_t nesting_level = 0;
                            int32_t closed_correctly = 0;
                            ++token_index;
                            still_looping = 0;
                            do{
                                for (; token_index < stream.end; ++token_index){
                                    Cpp_Token *token_ptr = stream.tokens + token_index;
                                    switch (token_ptr->type){
                                        case CPP_TOKEN_IDENTIFIER:
                                        {
                                            if (token_ptr->size == old_lexeme.size){
                                                char other_lexeme_base[128];
                                                String other_lexeme = make_fixed_width_string(other_lexeme_base);
                                                other_lexeme.size = old_lexeme.size;
                                                buffer_read_range(app, &buffer, token_ptr->start,
                                                                  token_ptr->start+token_ptr->size,
                                                                  other_lexeme.str);
                                                
                                                if (match(old_lexeme, other_lexeme)){
                                                    Buffer_Edit edit;
                                                    edit.str_start = 0;
                                                    edit.len = replace_string.size;
                                                    edit.start = token_ptr->start;
                                                    edit.end = token_ptr->start + token_ptr->size;
                                                    
                                                    if (edit_count < edit_max){
                                                        edits[edit_count] = edit;
                                                        ++edit_count;
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
                                buffer_batch_edit(app, &buffer, replace_string.str, replace_string.size,
                                                  edits, edit_count, BatchEdit_Normal);
                            }
                        }
                    }
                }
            }
        }
    }
    
    end_temp_memory(temp);
}

typedef uint32_t Write_Explicit_Enum_Values_Mode;
enum{
    WriteExplicitEnumValues_Integers,
    WriteExplicitEnumValues_Flags,
};

static void
write_explicit_enum_values_parameters(Application_Links *app, Write_Explicit_Enum_Values_Mode mode){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    Cpp_Get_Token_Result result;
    if (buffer_get_token_index(app, &buffer, view.cursor.pos, &result)){
        if (!result.in_whitespace){
            Cpp_Token stream_space[32];
            Stream_Tokens stream = {0};
            
            if (init_stream_tokens(&stream, app, &buffer, result.token_index, stream_space, 32)){
                int32_t token_index = result.token_index;
                Cpp_Token token = stream.tokens[token_index];
                
                if (token.type == CPP_TOKEN_BRACE_OPEN){
                    
                    ++token_index;
                    
                    int32_t seeker_index = token_index;
                    Stream_Tokens seek_stream = begin_temp_stream_token(&stream);
                    
                    bool32 closed_correctly = false;
                    bool32 still_looping = false;
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
                        int32_t count_estimate = 1 + (seeker_index - token_index)/2;
                        
                        int32_t edit_count = 0;
                        Buffer_Edit *edits = push_array(part, Buffer_Edit, count_estimate);
                        
                        char *string_base = (char*)partition_current(part);
                        String string = make_string(string_base, 0, partition_remaining(part));
                        
                        closed_correctly = false;
                        still_looping = false;
                        uint32_t value = 0;
                        if (mode == WriteExplicitEnumValues_Flags){
                            value = 1;
                        }
                        
                        do{
                            for (;token_index < stream.end; ++token_index){
                                Cpp_Token *token_ptr = stream.tokens + token_index;
                                switch (token_ptr->type){
                                    case CPP_TOKEN_IDENTIFIER:
                                    {
                                        int32_t edit_start = token_ptr->start + token_ptr->size;
                                        int32_t edit_stop = edit_start;
                                        
                                        int32_t edit_is_good = 0;
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
                                            int32_t str_pos = string.size;
                                            
                                            append(&string, " = ");
                                            append_int_to_str(&string, value);
                                            if (closed_correctly){
                                                append(&string, "\n");
                                            }
                                            
                                            if (mode == WriteExplicitEnumValues_Integers){
                                                ++value;
                                            }
                                            else if (mode == WriteExplicitEnumValues_Flags){
                                                if (value < (1 << 31)){
                                                    value <<= 1;
                                                }
                                            }
                                            
                                            int32_t str_size = string.size - str_pos;
                                            
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
                            buffer_batch_edit(app, &buffer, string_base, string.size,
                                              edits, edit_count, BatchEdit_Normal);
                        }
                    }
                }
            }
        }
    }
    
    end_temp_memory(temp);
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
    Buffer_ID buffer_id;
    int32_t start_pos;
};

static void
replace_all_occurrences_parameters(Application_Links *app, General_Memory *general, Partition *part, String target_string, String new_string){
    if (target_string.size <= 0) return;
    
    for (bool32 got_all_occurrences = false;
         !got_all_occurrences;){
        // Initialize a generic search all buffers
        Search_Set set = {0};
        Search_Iter iter = {0};
        initialize_generic_search_all_buffers(app, general, &target_string, 1, SearchFlag_MatchSubstring, 0, 0, &set, &iter);
        
        // Visit all locations and create replacement list
        Temp_Memory temp = begin_temp_memory(part);
        Replace_Target *targets = push_array(part, Replace_Target, 0);
        int32_t target_count = 0;
        
        got_all_occurrences = true;
        for (Search_Match match = search_next_match(app, &set, &iter);
             match.found_match;
             match = search_next_match(app, &set, &iter)){
            
            Replace_Target *new_target= push_array(part, Replace_Target, 1);
            if (new_target != 0){
                new_target->buffer_id = match.buffer.buffer_id;
                new_target->start_pos = match.start;
                ++target_count;
            }
            else{
                if (!has_substr(new_string, target_string)){
                    got_all_occurrences = false;
                }
                else{
                    print_message(app, literal("Could not replace all occurrences, ran out of memory\n"));
                }
                break;
            }
        }
        
        // Use replacement list to do replacements
        for (int32_t i = 0; i < target_count; ++i){
            Replace_Target *target = &targets[i];
            Buffer_Summary buffer= get_buffer(app, target->buffer_id, AccessOpen);
            buffer_replace_range(app, &buffer, target->start_pos, target->start_pos + target_string.size, new_string.str, new_string.size);
        }
        
        end_temp_memory(temp);
    }
}

CUSTOM_COMMAND_SIG(replace_all_occurrences)
CUSTOM_DOC("Queries the user for two strings, and replaces all occurrences of the first string with the second string in all open buffers.")
{
    Query_Bar replace;
    char replace_space[1024];
    replace.prompt = make_lit_string("Replace (In All Buffers): ");
    replace.string = make_fixed_width_string(replace_space);
    
    Query_Bar with;
    char with_space[1024];
    with.prompt = make_lit_string("With: ");
    with.string = make_fixed_width_string(with_space);
    
    if (!query_user_string(app, &replace)) return;
    if (replace.string.size == 0) return;
    
    if (!query_user_string(app, &with)) return;
    
    String r = replace.string;
    String w = with.string;
    
    replace_all_occurrences_parameters(app, &global_general, &global_part, r, w);
}

extern "C" int32_t
get_bindings(void *data, int32_t size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_view_size_change, default_view_adjust);
    
    set_start_hook(context, default_start);
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    set_end_file_hook(context, end_file_close_jump_list);
    
    set_input_filter(context, default_suppress_mouse_filter);
    set_command_caller(context, default_command_caller);
    
    set_scroll_rule(context, smooth_scroll_rule);
    set_buffer_name_resolver(context, default_buffer_name_resolution);
    
    default_keys(context);
    
    // NOTE(allen|a4.0.6): Command maps can be opened more than
    // once so that you can extend existing maps very easily.
    // You can also use the helper "restart_map" instead of
    // begin_map to clear everything that was in the map and
    // bind new things instead.
    begin_map(context, mapid_file);
    bind(context, 'k', MDFR_ALT, kill_rect);
    bind(context, ' ', MDFR_ALT | MDFR_CTRL, multi_line_edit);
    
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
    
    int32_t result = end_bind_helper(context);
    return(result);
}

#endif

// BOTTOM

