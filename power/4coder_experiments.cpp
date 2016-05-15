
// TOP

#define NO_BINDING
#include "../4coder_default_bindings.cpp"

CUSTOM_COMMAND_SIG(kill_rect){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    Full_Cursor cursor;
    
    Buffer_Rect rect = get_rect(&view);
    
    for (int line = rect.line1; line >= rect.line0; --line){
        int start = 0;
        int end = 0;
        
        cursor = app->view_compute_cursor(app, &view, seek_line_char(line, rect.char0));
        start = cursor.pos;
        
        cursor = app->view_compute_cursor(app, &view, seek_line_char(line, rect.char1));
        end = cursor.pos;
        
        app->buffer_replace_range(app, &buffer, start, end, 0, 0);
    }
}

// NOTE(allen): This stream stuff will be moved to helper if it looks
// like it will be helpful.  So if you want to use it to build your own
// commands I suggest you move it there first.
struct Stream_Chunk{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    char *base_data;
    int start, end;
    int data_size;
    
    char *data;
};

int
round_down(int x, int b){
    int r = 0;
    if (x >= 0){
        r = x - (x % b);
    }
    return(r);
}

int
round_up(int x, int b){
    int r = 0;
    if (x >= 0){
        r = x - (x % b) + b;
    }
    return(r);
}

int
init_stream_chunk(Stream_Chunk *chunk,
    Application_Links *app, Buffer_Summary *buffer,
    int pos, char *data, int size){
    int result = 0;
    
    app->refresh_buffer(app, buffer);
    if (pos >= 0 && pos < buffer->size && size > 0){
        result = 1;
        chunk->app = app;
        chunk->buffer = buffer;
        chunk->base_data = data;
        chunk->data_size = size;
        chunk->start = round_down(pos, size);
        chunk->end = round_up(pos, size);
        if (chunk->end > buffer->size){
            chunk->end = buffer->size;
        }
        app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
        chunk->data = chunk->base_data - chunk->start;
    }
    return(result);
}

int
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    int result = 0;
    
    app->refresh_buffer(app, buffer);
    if (chunk->end < buffer->size){
        result = 1;
        chunk->start = chunk->end;
        chunk->end += chunk->data_size;
        if (chunk->end > buffer->size){
            chunk->end = buffer->size;
        }
        app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
        chunk->data = chunk->base_data - chunk->start;
    }
    return(result);
}

int
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    int result = 0;
    
    app->refresh_buffer(app, buffer);
    if (chunk->start > 0){
        result = 1;
        chunk->end = chunk->start;
        chunk->start -= chunk->data_size;
        if (chunk->start < 0){
            chunk->start = 0;
        }
        app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
        chunk->data = chunk->base_data - chunk->start;
    }
    return(result);
}

// TODO(allen): Both of these brace related commands would work better
// if the API exposed access to the tokens in a code file.
CUSTOM_COMMAND_SIG(mark_matching_brace){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    int start_pos = view.cursor.pos;
    
    // NOTE(allen): The user provides the memory that the chunk uses,
    // this chunk will then be filled at each step of the text stream loop.
    // This way you can look for something that should be nearby without
    // having to copy the whole file in at once.
    Stream_Chunk chunk;
    char chunk_space[(1 << 10)];
    
    int result = 0;
    int found_result = 0;
    
    int i = start_pos;
    int still_looping = 1;
    int nesting_counter = 0;
    char at_cursor = 0;
    
    if (init_stream_chunk(&chunk, app, &buffer, i, chunk_space, sizeof(chunk_space))){
        
        // NOTE(allen): This is important! The data array is a pointer that is adjusted
        // so that indexing it with "i" will put it with the chunk that is actually loaded.
        // If i goes below chunk.start or above chunk.end _that_ is an invalid access.
        at_cursor = chunk.data[i];
        if (at_cursor == '{'){
            do{
                for (++i; i < chunk.end; ++i){
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
            do{
                for (--i; i >= chunk.start; --i){
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
    
    finished:
    if (found_result){
        app->view_set_mark(app, &view, seek_pos(result));
    }
}

CUSTOM_COMMAND_SIG(cursor_to_surrounding_scope){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    int start_pos = view.cursor.pos - 1;
    
    Stream_Chunk chunk;
    char chunk_space[(1 << 10)];
    
    int result = 0;
    int found_result = 0;
    
    int i = start_pos;
    int still_looping = 1;
    int nesting_counter = 0;
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
    
    finished:
    if (found_result){
        app->view_set_cursor(app, &view, seek_pos(result), 0);
    }
}

// NOTE(allen): Incomplete
#if 0
CUSTOM_COMMAND_SIG(complete_word){
    app->print_message(app, literal("complete_word\n"));
    
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    int start = 0;
    int end = 0;
    
    end = view.cursor.pos;
    
    push_parameter(app, par_flags, BoundryAlphanumeric);
    exec_command(app, cmdid_seek_left);
    
    app->refresh_view(app, &view);
    start = view.cursor.pos;
    
    String complete_string;
    int size = (end - start);
    char complete_space[256];
    
    if (size < sizeof(complete_space) - 1){
        complete_string = make_fixed_width_string(complete_space);
        app->buffer_read_range(app, &buffer, start, end, complete_space);
        complete_string.size = size;
        complete_string.str[size] = 0;
        
        // TODO(allen): Complete this when the heavy duty coroutine stuff
        // and the hash table are available.
        
        app->print_message(app, complete_string.str, complete_string.size);
    }
}
#endif

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

        app->change_theme(app, theme_name, strlen(theme_name));
        app->change_font(app, font_name, strlen(font_name));
    }
}
#endif

void experiment_extension(Bind_Helper *context){
    bind(context, 'k', MDFR_ALT, kill_rect);
    bind(context, '/', MDFR_ALT, mark_matching_brace);
    bind(context, '\'', MDFR_ALT, cursor_to_surrounding_scope);
}

#include <stdio.h>

HOOK_SIG(experimental_start_hook){
    my_start(app);
    
    FILE *file = fopen(".4coder_settings", "rb");
    char theme_name[128];
    char font_name[128];
    
    if (file){
        fscanf(file, "%127s\n%127s", theme_name, font_name);

        replace_char(theme_name, '#', ' ');
        replace_char(font_name, '#', ' ');

        fclose(file);

        app->change_theme(app, theme_name, (int)strlen(theme_name));
        app->change_font(app, font_name, (int)strlen(font_name));
    }
    
    return(0);
}

int get_bindings(void *data, int size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_start, experimental_start_hook);
    set_hook(context, hook_open_file, my_file_settings);

    set_scroll_rule(context, smooth_scroll_rule);
    
    default_keys(context, experiment_extension);
    
    int result = end_bind_helper(context);
    return(result);
}

// BOTTOM

