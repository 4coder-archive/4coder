
// TOP

#include "4coder_default_include.cpp"

#define NO_BINDING
#include "4coder_default_bindings.cpp"

#ifndef BIND_4CODER_TESTS
# define BIND_4CODER_TESTS(context) ((void)context)
#endif

#include <string.h>

CUSTOM_COMMAND_SIG(kill_rect){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Buffer_Rect rect = get_rect(&view);
    
    for (int32_t line = rect.line1; line >= rect.line0; --line){
        int32_t start = 0;
        int32_t end = 0;
        
        int32_t success = 1;
        Full_Cursor cursor = {0};
        
        if (success){
            success = view_compute_cursor(app, &view, seek_line_char(line, rect.char0), &cursor);
        }
        start = cursor.pos;
        
        if (success){
            success = view_compute_cursor(app, &view, seek_line_char(line, rect.char1), &cursor);
        }
        end = cursor.pos;
        
        if (success){
            buffer_replace_range(app, &buffer, start, end, 0, 0);
        }
    }
}

static void
pad_buffer_line(Application_Links *app, Partition *part,
                Buffer_Summary *buffer, int line,
                char padchar, int target){
    Partial_Cursor start = {0};
    Partial_Cursor end = {0};
    
    if (buffer_compute_cursor(app, buffer, seek_line_char(line, 1), &start)){
        if (buffer_compute_cursor(app, buffer, seek_line_char(line, 65536), &end)){
            if (start.line == line){
                if (end.character-1 < target){
                    Temp_Memory temp = begin_temp_memory(part);
                    int size = target - (end.character-1);
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
I am even able to support the GUI. Because that will the
system up to allow me to think about the problem in more ways.

Finally I have decided not to pursue this direction any more,
it just seems like the wrong way to do it, so I'll stop without
doing multi-cursor for now.
*/

CUSTOM_COMMAND_SIG(multi_line_edit){
    Partition *part = &global_part;
    
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Buffer_Rect rect = get_rect(&view);
    
    int start_line = view.cursor.line;
    int pos = view.cursor.character-1;
    
    for (int i = rect.line0; i <= rect.line1; ++i){
        pad_buffer_line(app, &global_part, &buffer, i, ' ', pos);
    }
    
    int line_count = rect.line1 - rect.line0 + 1;
    
    for (;;){
        User_Input in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        if (in.abort) break;
        
        if (in.key.character && key_is_unmodified(&in.key)){
            char str = (char)in.key.character;
            
            Temp_Memory temp = begin_temp_memory(part);
            Buffer_Edit *edit = push_array(part, Buffer_Edit, line_count);
            Buffer_Edit *edits = edit;
            
            for (int i = rect.line0; i <= rect.line1; ++i){
                Partial_Cursor cursor = {0};
                
                if (buffer_compute_cursor(app, &buffer, seek_line_char(i, pos+1), &cursor)){
                    edit->str_start = 0;
                    edit->len = 1;
                    edit->start = cursor.pos;
                    edit->end = cursor.pos;
                    ++edit;
                }
            }
            
            int edit_count = (int)(edit - edits);
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
                
                for (int i = rect.line0; i <= rect.line1; ++i){
                    Partial_Cursor cursor = {0};
                    
                    if (buffer_compute_cursor(app, &buffer, seek_line_char(i, pos+1), &cursor)){
                        edit->str_start = 0;
                        edit->len = 0;
                        edit->start = cursor.pos-1;
                        edit->end = cursor.pos;
                        ++edit;
                    }
                }
                
                int edit_count = (int)(edit - edits);
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

// TODO(allen): Both of these brace related commands would work better
// if the API exposed access to the tokens in a code file.
CUSTOM_COMMAND_SIG(mark_matching_brace){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    // NOTE(allen): The user provides the memory that the chunk uses,
    // this chunk will then be filled at each step of the text stream loop.
    // This way you can look for something that should be nearby without
    // having to copy the whole file in at once.
    Stream_Chunk chunk = {0};
    char chunk_space[(1 << 10)];
    
    int32_t result = 0;
    int32_t found_result = 0;
    
    int32_t i = view.cursor.pos;
    int32_t still_looping = 1;
    int32_t nesting_counter = 0;
    char at_cursor = 0;
    
    if (init_stream_chunk(&chunk, app, &buffer, i,
                          chunk_space, sizeof(chunk_space))){
        
        // NOTE(allen): This is important! The data array is a pointer that is adjusted
        // so that indexing it with "i" will put it with the chunk that is actually loaded.
        // If i goes below chunk.start or above chunk.end _that_ is an invalid access.
        at_cursor = chunk.data[i];
        if (at_cursor == '{'){
            ++i;
            do{
                for (; i < chunk.end; ++i){
                    at_cursor = chunk.data[i];
                    if (at_cursor == '{'){
                        ++nesting_counter;
                    }
                    else if (at_cursor == '}'){
                        if (nesting_counter == 0){
                            found_result = 1;
                            result = i;
                            goto finished;
                        }
                        else{
                            --nesting_counter;
                        }
                    }
                }
                still_looping = forward_stream_chunk(&chunk);
            }
            while (still_looping);
        }
        else if (at_cursor == '}'){
            --i;
            do{
                for (; i >= chunk.start; --i){
                    at_cursor = chunk.data[i];
                    if (at_cursor == '}'){
                        ++nesting_counter;
                    }
                    else if (at_cursor == '{'){
                        if (nesting_counter == 0){
                            found_result = 1;
                            result = i;
                            goto finished;
                        }
                        else{
                            --nesting_counter;
                        }
                    }
                }
                still_looping = backward_stream_chunk(&chunk);
            }
            while (still_looping);
        }
    }
    
    finished:;
    if (found_result){
        view_set_mark(app, &view, seek_pos(result+1));
    }
}

CUSTOM_COMMAND_SIG(cursor_to_surrounding_scope){
    unsigned int access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Stream_Chunk chunk = {0};
    char chunk_space[(1 << 10)];
    
    int32_t result = 0;
    int32_t found_result = 0;
    
    int32_t i = view.cursor.pos - 1;
    int32_t still_looping = 1;
    int32_t nesting_counter = 0;
    char at_cursor = 0;
    
    if (init_stream_chunk(&chunk, app, &buffer, i, chunk_space, sizeof(chunk_space))){
        do{
            for (; i >= chunk.start; --i){
                at_cursor = chunk.data[i];
                if (at_cursor == '}'){
                    ++nesting_counter;
                }
                else if (at_cursor == '{'){
                    if (nesting_counter == 0){
                        found_result = 1;
                        result = i;
                        goto finished;
                    }
                    else{
                        --nesting_counter;
                    }
                }
            }
            still_looping = backward_stream_chunk(&chunk);
        } while(still_looping);
    }
    
    finished:;
    if (found_result){
        view_set_cursor(app, &view, seek_pos(result), 0);
    }
}

// NOTE(allen): Some basic code manipulation ideas.

CUSTOM_COMMAND_SIG(rename_parameter){
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
                
                if (token.type == CPP_TOKEN_IDENTIFIER){
                    
                    char old_lexeme_base[128];
                    String old_lexeme = make_fixed_width_string(old_lexeme_base);
                    
                    if (token.size < sizeof(old_lexeme_base)){
                        Cpp_Token original_token = token;
                        old_lexeme.size = token.size;
                        buffer_read_range(app, &buffer, token.start,
                                               token.start+token.size,
                                               old_lexeme.str);
                        
                        int32_t proc_body_found = 0;
                        int32_t still_looping = 0;
                        
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

CUSTOM_COMMAND_SIG(write_explicit_enum_values){
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
                    
                    int32_t closed_correctly = 0;
                    int32_t seeker_index = token_index;
                    Stream_Tokens seek_stream = begin_temp_stream_token(&stream);
                    
                    int32_t still_looping = 0;
                    do{
                        for (; seeker_index < seek_stream.end; ++seeker_index){
                            Cpp_Token *token_seeker = seek_stream.tokens + seeker_index;
                            switch (token_seeker->type){
                                case CPP_TOKEN_BRACE_CLOSE:
                                closed_correctly = 1;
                                goto finished_seek;
                                
                                case CPP_TOKEN_BRACE_OPEN:
                                goto finished_seek;
                            }
                        }
                        still_looping = forward_stream_tokens(&seek_stream);
                    }while(still_looping);
                    finished_seek:;
                    end_temp_stream_token(&stream, seek_stream);
                    
                    if (closed_correctly){
                        int32_t count_estimate = 1 + (seeker_index - token_index)/2;
                        
                        Buffer_Edit *edits = push_array(part, Buffer_Edit, count_estimate);
                        int32_t edit_count = 0;
                        
                        char *string_base = (char*)partition_current(part);
                        String string = make_string(string_base, 0, partition_remaining(part));
                        
                        int32_t value = 0;
                        closed_correctly = 0;
                        still_looping = 0;
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
                                            ++value;
                                            
                                            int32_t str_size = string.size - str_pos;
                                            
                                            Buffer_Edit edit;
                                            edit.str_start = str_pos;
                                            edit.len = str_size;
                                            edit.start = edit_start;
                                            edit.end = edit_stop;
                                            
                                            assert(edit_count < count_estimate);
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

// TODO(allen): Query theme settings
#if 0
CUSTOM_COMMAND_SIG(save_theme_settings){
    FILE *file = fopen(".4coder_settings", "rb");
    char theme_name[128];
    char font_name[128];
    
    fscanf(file, "%*128s %*128s", theme_name, font_name);
    
    if (file){
        replace_char(theme_name, '#', ' ');
        replace_char(font_name, '#', ' ');
        
        fclose(file);
        
        change_theme(app, theme_name, strlen(theme_name));
        change_font(app, font_name, strlen(font_name));
    }
}
#endif

#include <stdio.h>

#define SETTINGS_FILE ".4coder_settings"
HOOK_SIG(experimental_start){
    init_memory(app);
    
    char theme_name[128];
    char font_name[128];
    
    FILE *file = fopen(SETTINGS_FILE, "rb");
    
    if (!file){
        char module_path[512];
        int len = get_4ed_path(app, module_path, 448);
        memcpy(module_path+len, SETTINGS_FILE, sizeof(SETTINGS_FILE));
        file = fopen(module_path, "rb");
    }
    
    if (file){
        fscanf(file, "%127s\n%127s", theme_name, font_name);
        
        String theme = make_string_slowly(theme_name);
        String font = make_string_slowly(font_name);
        
        replace_char(&theme, '#', ' ');
        replace_char(&font, '#', ' ');
        
        fclose(file);
        
        int theme_len = (int)strlen(theme_name);
        int font_len = (int)strlen(font_name);
        
        change_theme(app, theme_name, theme_len);
        change_font(app, font_name, font_len, true);
        
        exec_command(app, open_panel_vsplit);
        exec_command(app, hide_scrollbar);
        exec_command(app, change_active_panel);
        exec_command(app, hide_scrollbar);
    }
    
    return(0);
}

extern "C" int
get_bindings(void *data, int size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_start, experimental_start);
    set_open_file_hook(context, my_file_settings);
    set_input_filter(context, my_suppress_mouse_filter);
    set_command_caller(context, default_command_caller);
    
    set_scroll_rule(context, smooth_scroll_rule);
    
    default_keys(context);
    
    // NOTE(allen|a4.0.6): Command maps can be opened more than
    // once so that you can extend existing maps very easily.
    // You can also use the helper "restart_map" instead of
    // begin_map to clear everything that was in the map and
    // bind new things instead.
    begin_map(context, mapid_global);
    end_map(context);
    
    begin_map(context, mapid_file);
    bind(context, 'k', MDFR_ALT, kill_rect);
    bind(context, ' ', MDFR_ALT, multi_line_edit);
    end_map(context);
    
    begin_map(context, my_code_map);
    bind(context, '[', MDFR_ALT, cursor_to_surrounding_scope);
    bind(context, ']', MDFR_ALT, mark_matching_brace);
    bind(context, key_insert, MDFR_CTRL, write_explicit_enum_values);
    bind(context, 'p', MDFR_ALT, rename_parameter);
    end_map(context);
    
    BIND_4CODER_TESTS(context);
    
    int result = end_bind_helper(context);
    return(result);
}

// BOTTOM

