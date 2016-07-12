
// TOP

#include "4coder_default_include.cpp"
#include "4coder_default_building.cpp"

#define NO_BINDING
#include "4coder_default_bindings.cpp"

#include <string.h>

CUSTOM_COMMAND_SIG(kill_rect){
    unsigned int access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    Buffer_Rect rect = get_rect(&view);
    
    for (int line = rect.line1; line >= rect.line0; --line){
        int start = 0;
        int end = 0;
        
        int success = true;
        Full_Cursor cursor = {0};
        
        success = success &&
            app->view_compute_cursor(app, &view, seek_line_char(line, rect.char0), &cursor);
        start = cursor.pos;
        
        success = success &&
            app->view_compute_cursor(app, &view, seek_line_char(line, rect.char1), &cursor);
        end = cursor.pos;
        
        if (success){
            app->buffer_replace_range(app, &buffer, start, end, 0, 0);
        }
    }
}

// TODO(allen): Both of these brace related commands would work better
// if the API exposed access to the tokens in a code file.
CUSTOM_COMMAND_SIG(mark_matching_brace){
    unsigned int access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
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
    
    if (init_stream_chunk(&chunk, app, &buffer, i,
                          chunk_space, sizeof(chunk_space))){
        
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
    unsigned int access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
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

#include <stdio.h>

#define SETTINGS_FILE ".4coder_settings"
HOOK_SIG(experimental_start_hook){
    my_start(app);
    
    char theme_name[128];
    char font_name[128];
    
    FILE *file = fopen(SETTINGS_FILE, "rb");
    
    if (!file){
        char module_path[512];
        int len;
        len = app->get_4ed_path(app, module_path, 448);
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
        
        app->change_theme(app, theme_name, (int)strlen(theme_name));
        app->change_font(app, font_name, (int)strlen(font_name));
    }
    
    return(0);
}

extern "C" int
get_bindings(void *data, int size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_start, experimental_start_hook);
    set_open_file_hook(context, my_file_settings);
    set_input_filter(context, my_suppress_mouse_filter);
    set_command_caller(context, my_command_caller);
    
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
    end_map(context);
    
    begin_map(context, my_code_map);
    bind(context, '/', MDFR_ALT, mark_matching_brace);
    bind(context, '\'', MDFR_ALT, cursor_to_surrounding_scope);
    end_map(context);
    
    int result = end_bind_helper(context);
    return(result);
}

// BOTTOM

