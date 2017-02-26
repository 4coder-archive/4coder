/*
4coder_experiments.cpp - Supplies extension bindings to the defaults with experimental new features.

TYPE: 'build-target'
*/

// TOP

#if !defined(FCODER_EXPERIMENTS_CPP)
#define FCODER_EXPERIMENTS_CPP

#include "4coder_default_include.cpp"
#include "4coder_miblo_numbers.cpp"

#define NO_BINDING
#include "4coder_default_bindings.cpp"

#ifndef BIND_4CODER_TESTS
# define BIND_4CODER_TESTS(context) ((void)context)
#endif

#include <assert.h>
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

CUSTOM_COMMAND_SIG(kill_rect){
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

CUSTOM_COMMAND_SIG(multi_line_edit){
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

//
// Scope-Smart Editing Basics
//

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

CUSTOM_COMMAND_SIG(highlight_surrounding_scope){
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

// TODO(allen): These aren't super top notch yet (the ones about children and siblings).  I am not sure I want them around anyway, because it's not that fluid feeling even when it works.
CUSTOM_COMMAND_SIG(highlight_first_child_scope){
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

CUSTOM_COMMAND_SIG(highlight_next_sibling_scope){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t start_pos = view.cursor.pos;
    int32_t top = 0, bottom = 0;
    if (find_next_scope(app, &buffer, start_pos, FindScope_NextSibling, &top)){
        if (find_scope_bottom(app, &buffer, top, FindScope_EndOfToken, &bottom)){
            view_set_cursor(app, &view, seek_pos(top), true);
            view_set_mark(app, &view, seek_pos(bottom));
            view_set_to_region(app, &view, top, bottom, scope_center_threshold);
        }
    }
}

CUSTOM_COMMAND_SIG(highlight_prev_sibling_scope){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t start_pos = view.cursor.pos;
    int32_t top = 0, bottom = 0;
    if (find_prev_scope(app, &buffer, start_pos, FindScope_NextSibling, &top)){
        if (find_scope_bottom(app, &buffer, top, FindScope_EndOfToken, &bottom)){
            view_set_cursor(app, &view, seek_pos(top), true);
            view_set_mark(app, &view, seek_pos(bottom));
            view_set_to_region(app, &view, top, bottom, scope_center_threshold);
        }
    }
}

CUSTOM_COMMAND_SIG(highlight_next_scope_absolute){
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

CUSTOM_COMMAND_SIG(highlight_prev_scope_absolute){
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

CUSTOM_COMMAND_SIG(place_in_scope){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Range lines;
    Range range = get_range(&view);
    lines.min = buffer_get_line_index(app, &buffer, range.min);
    range.min = buffer_get_line_start(app, &buffer, lines.min);
    
    lines.max = buffer_get_line_index(app, &buffer, range.max);
    range.max = buffer_get_line_end(app, &buffer, lines.max);
    
    bool32 do_full = false;
    
    if (lines.min < lines.max){
        do_full = true;
    }
    else if (!buffer_line_is_blank(app, &buffer, lines.min)){
        do_full = true;
    }
    
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

CUSTOM_COMMAND_SIG(delete_current_scope){
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

CUSTOM_COMMAND_SIG(scope_absorb_down){
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
                        int32_t value = 0;
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

CUSTOM_COMMAND_SIG(punishment){
    Theme_Color colors[4];
    colors[0].tag = Stag_Back;
    colors[1].tag = Stag_Margin;
    colors[2].tag = Stag_Margin_Hover;
    colors[3].tag = Stag_Margin_Active;
    get_theme_colors(app, colors, 4);
    
    for (uint32_t i = 0; i < 4; ++i){
        int_color color = colors[i].color;
        uint8_t *c = (uint8_t*)(&color);
        c[0] = 0xFF - c[0];
        c[1] = 0xFF - c[1];
        c[2] = 0xFF - c[2];
        c[3] = 0xFF - c[3];
        colors[i].color = color;
    }
    
    set_theme_colors(app, colors, 4);
}

extern "C" int32_t
get_bindings(void *data, int32_t size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_start, default_start);
    set_hook(context, hook_view_size_change, default_view_adjust);
    
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    set_input_filter(context, default_suppress_mouse_filter);
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
    bind(context, 's', MDFR_CTRL, punishment);
    bind(context, 's', MDFR_ALT, save);
    
    bind(context, 'k', MDFR_ALT, kill_rect);
    bind(context, ' ', MDFR_ALT | MDFR_CTRL, multi_line_edit);
    
    bind(context, key_page_up, MDFR_ALT, miblo_increment_time_stamp);
    bind(context, key_page_down, MDFR_ALT, miblo_decrement_time_stamp);
    
    bind(context, key_home, MDFR_ALT, miblo_increment_time_stamp_minute);
    bind(context, key_end, MDFR_ALT, miblo_decrement_time_stamp_minute);
    
    end_map(context);
    
    begin_map(context, default_code_map);
    bind(context, '[', MDFR_ALT, highlight_surrounding_scope);
    bind(context, ']', MDFR_ALT, highlight_prev_scope_absolute);
    bind(context, '\'', MDFR_ALT, highlight_next_scope_absolute);
    
    bind(context, '/', MDFR_ALT, place_in_scope);
    bind(context, '-', MDFR_ALT, delete_current_scope);
    bind(context, 'j', MDFR_ALT, scope_absorb_down);
    
    bind(context, key_insert, MDFR_CTRL, write_explicit_enum_values);
    bind(context, 'p', MDFR_ALT, rename_parameter);
    end_map(context);
    
    BIND_4CODER_TESTS(context);
    
    int32_t result = end_bind_helper(context);
    return(result);
}

#endif

// BOTTOM

