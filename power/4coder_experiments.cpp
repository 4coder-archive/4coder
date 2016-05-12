
// TOP

#define NO_BINDING
#include "../4coder_default_bindings.cpp"

CUSTOM_COMMAND_SIG(kill_rect){
    View_Summary view = app->get_active_view(app);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id);
    
    Buffer_Rect rect = get_rect(&view);
    
    for (int line = rect.line1; line >= rect.line0; --line){
        int start = 0;
        int end = 0;
        
        app->view_set_cursor(app, &view, seek_line_char(line, rect.char0), 0);
        app->refresh_view(app, &view);
        start = view.cursor.pos;
        
        app->view_set_cursor(app, &view, seek_line_char(line, rect.char1), 0);
        app->refresh_view(app, &view);
        end = view.cursor.pos;
        
        app->buffer_replace_range(app, &buffer, start, end, 0, 0);
    }
}

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
    int size = (start - end);
    char complete_space[256];
    
    if (size < sizeof(complete_space) - 1){
        complete_string = make_fixed_width_string(complete_space);
        app->buffer_read_range(app, &buffer, start, end, complete_space);
        complete_string.size = size;
        complete_string.str[size] = 0;
        
        // TODO(allen): Complete this when the heavy duty coroutine stuff
        // and the hash table are available
        
        app->print_message(app, complete_string.str, complete_string.size);
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

void experiment_extension(Bind_Helper *context){
    bind(context, 'k', MDFR_ALT, kill_rect);
    bind(context, '+', MDFR_CTRL, complete_word);
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

