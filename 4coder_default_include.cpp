
#ifndef FCODER_DEFAULT_INCLUDE
#define FCODER_DEFAULT_INCLUDE

#include "4coder_custom.h"

#define FSTRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4coder_helper.h"

#include <assert.h>

#ifndef DEFAULT_INDENT_FLAGS
# define DEFAULT_INDENT_FLAGS 0
#endif

#ifndef DEF_TAB_WIDTH
# define DEF_TAB_WIDTH 4
#endif

//
// Useful helper functions
//

static int32_t
open_file(Application_Links *app,
          Buffer_Summary *buffer_out,
          char *filename,
          int32_t filename_len,
          int32_t background,
          int32_t never_new){
    int32_t result = false;
    Buffer_Summary buffer =
        app->get_buffer_by_name(app, filename, filename_len,
                                AccessProtected|AccessHidden);
    
    if (buffer.exists){
        if (buffer_out) *buffer_out = buffer;
        result = true;
    }
    else{
        Buffer_Create_Flag flags = 0;
        if (background){
            flags |= BufferCreate_Background;
        }
        if (never_new){
            flags |= BufferCreate_NeverNew;
        }
        buffer = app->create_buffer(app, filename, filename_len, flags);
        if (buffer.exists){
            if (buffer_out) *buffer_out = buffer;
            result = true;
        }
    }
    
    return(result);
}

static int32_t
view_open_file(Application_Links *app,
               View_Summary *view,
               char *filename,
               int32_t filename_len,
               int32_t never_new){
    int32_t result = false;
    
    if (view){
        Buffer_Summary buffer = {0};
        if (open_file(app, &buffer, filename, filename_len, false, never_new)){
            app->view_set_buffer(app, view, buffer.buffer_id, 0);
            result = true;
        }
    }
    
    return(result);
}

static int32_t
read_line(Application_Links *app,
          Partition *part,
          Buffer_Summary *buffer,
          int32_t line,
          String *str){
    
    Partial_Cursor begin = {0};
    Partial_Cursor end = {0};
    
    int32_t success = false;
    
    if (app->buffer_compute_cursor(app, buffer,
                                   seek_line_char(line, 1), &begin)){
        if (app->buffer_compute_cursor(app, buffer,
                                       seek_line_char(line, 65536), &end)){
            if (begin.line == line){
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer->size){
                    int32_t size = (end.pos - begin.pos);
                    *str = make_string(push_array(part, char, size+1), size+1);
                    if (str->str){
                        success = true;
                        app->buffer_read_range(app, buffer, begin.pos, end.pos, str->str);
                        str->size = size;
                        terminate_with_null(str);
                    }
                }
            }
        }
    }
    
    return(success);
}


//
// Memory
//

static Partition global_part;
static General_Memory global_general;

void
init_memory(Application_Links *app){
    int32_t part_size = (1 << 20);
    int32_t general_size = (1 << 20);
    
    
    void *part_mem = app->memory_allocate(app, part_size);
    global_part = make_part(part_mem, part_size);
    
    void *general_mem = app->memory_allocate(app, general_size);
    general_memory_open(&global_general, general_mem, general_size);
}

//
// Buffer Streaming
//

struct Stream_Chunk{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    char *base_data;
    int32_t start, end;
    int32_t min_start, max_end;
    int32_t data_size;
    
    char *data;
};

int32_t
round_down(int32_t x, int32_t b){
    int32_t r = 0;
    if (x >= 0){
        r = x - (x % b);
    }
    return(r);
}

int32_t
round_up(int32_t x, int32_t b){
    int32_t r = 0;
    if (x >= 0){
        r = x - (x % b) + b;
    }
    return(r);
}

void
refresh_buffer(Application_Links *app, Buffer_Summary *buffer){
    *buffer = app->get_buffer(app, buffer->buffer_id, AccessAll);
}

void
refresh_view(Application_Links *app, View_Summary *view){
    *view = app->get_view(app, view->view_id, AccessAll);
}

int32_t
init_stream_chunk(Stream_Chunk *chunk,
                  Application_Links *app, Buffer_Summary *buffer,
                  int32_t pos, char *data, int32_t size){
    int32_t result = false;
    
    refresh_buffer(app, buffer);
    if (pos >= 0 && pos < buffer->size && size > 0){
        chunk->app = app;
        chunk->buffer = buffer;
        chunk->base_data = data;
        chunk->data_size = size;
        chunk->start = round_down(pos, size);
        chunk->end = round_up(pos, size);
        
        if (chunk->max_end > buffer->size 
            || chunk->max_end == 0){
            chunk->max_end = buffer->size;
        }
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = true;
        }
    }
    return(result);
}

int32_t
forward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    int32_t result = false;
    
    refresh_buffer(app, buffer);
    if (chunk->end < buffer->size){
        chunk->start = chunk->end;
        chunk->end += chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = true;
        }
    }
    return(result);
}

int32_t
backward_stream_chunk(Stream_Chunk *chunk){
    Application_Links *app = chunk->app;
    Buffer_Summary *buffer = chunk->buffer;
    int32_t result = false;
    
    refresh_buffer(app, buffer);
    if (chunk->start > 0){
        chunk->end = chunk->start;
        chunk->start -= chunk->data_size;
        
        if (chunk->max_end && chunk->max_end < chunk->end){
            chunk->end = chunk->max_end;
        }
        if (chunk->min_start && chunk->min_start > chunk->start){
            chunk->start = chunk->min_start;
        }
        
        if (chunk->start < chunk->end){
            app->buffer_read_range(app, buffer, chunk->start, chunk->end, chunk->base_data);
            chunk->data = chunk->base_data - chunk->start;
            result = true;
        }
    }
    return(result);
}

void
buffer_seek_delimiter_forward(Application_Links *app, Buffer_Summary *buffer,
                              int32_t pos, char delim, int32_t *result){
    if (buffer->exists){
        char chunk[1024];
        int32_t size = sizeof(chunk);
        Stream_Chunk stream = {0};
        
        if (init_stream_chunk(&stream, app, buffer, pos, chunk, size)){
            int32_t still_looping = 1;
            do{
                for(; pos < stream.end; ++pos){
                    char at_pos = stream.data[pos];
                    if (at_pos == delim){
                        *result = pos;
                        goto finished;
                    }
                }
                still_looping = forward_stream_chunk(&stream);
            }while (still_looping);
        }
    }
    
    *result = buffer->size;
    
    finished:;
}

void
buffer_seek_delimiter_backward(Application_Links *app, Buffer_Summary *buffer,
                              int32_t pos, char delim, int32_t *result){
    if (buffer->exists){
        char chunk[1024];
        int32_t size = sizeof(chunk);
        Stream_Chunk stream = {0};
        
        if (init_stream_chunk(&stream, app, buffer, pos, chunk, size)){
            int32_t still_looping = 1;
            do{
                for(; pos >= stream.start; --pos){
                    char at_pos = stream.data[pos];
                    if (at_pos == delim){
                        *result = pos;
                        goto finished;
                    }
                }
                still_looping = backward_stream_chunk(&stream);
            }while (still_looping);
        }
    }
    
    *result = 0;
    
    finished:;
}

// TODO(allen): This duplication is driving me crazy... I've gotta
// upgrade the meta programming system another level.

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_forward(Application_Links *app, Buffer_Summary *buffer,
                           int32_t pos, int32_t end, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    
    if (size <= 0){
        *result = pos;
    }
    else if (size > sizeof(read_buffer)){
        *result = pos;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = str[0];
            
            read_str.size = size;
            
            char chunk[1024];
            int32_t chunk_size = sizeof(chunk);
            Stream_Chunk stream = {0};
            stream.max_end = end;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int32_t still_looping = 1;
                do{
                    for(; pos < stream.end; ++pos){
                        char at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        if (end == 0){
            *result = buffer->size;
        }
        else{
            *result = end;
        }
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_backward(Application_Links *app, Buffer_Summary *buffer,
                            int32_t pos, int32_t min, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    if (size <= 0){
        *result = min-1;
    }
    else if (size > sizeof(read_buffer)){
        *result = min-1;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = str[0];
            
            read_str.size = size;
            
            char chunk[1024];
            int32_t chunk_size = sizeof(chunk);
            Stream_Chunk stream = {0};
            stream.min_start = min;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int32_t still_looping = 1;
                do{
                    for(; pos >= stream.start; --pos){
                        char at_pos = stream.data[pos];
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = backward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = min-1;
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_insensitive_forward(Application_Links *app, Buffer_Summary *buffer,
                                       int32_t pos, int32_t end, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    stream.max_end = end;
    
    if (size <= 0){
        *result = buffer->size;
    }
    else if (size > sizeof(read_buffer)){
        *result = buffer->size;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = char_to_upper(str[0]);
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int32_t still_looping = 1;
                do{
                    for(; pos < stream.end; ++pos){
                        char at_pos = char_to_upper(stream.data[pos]);
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_insensitive_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = forward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = buffer->size;
        
        finished:;
    }
}

// NOTE(allen): This is limitted to a string size of 512.
// You can push it up or do something more clever by just
// replacing char read_buffer[512]; with more memory.
void
buffer_seek_string_insensitive_backward(Application_Links *app, Buffer_Summary *buffer,
                                        int32_t pos, int32_t min, char *str, int32_t size, int32_t *result){
    char read_buffer[512];
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    stream.min_start = min;
    
    if (size <= 0){
        *result = -1;
    }
    else if (size > sizeof(read_buffer)){
        *result = -1;
    }
    else{
        if (buffer->exists){
            String read_str = make_fixed_width_string(read_buffer);
            String needle_str = make_string(str, size);
            char first_char = char_to_upper(str[0]);
            
            read_str.size = size;
            
            if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
                int32_t still_looping = 1;
                do{
                    for(; pos >= stream.start; --pos){
                        char at_pos = char_to_upper(stream.data[pos]);
                        if (at_pos == first_char){
                            app->buffer_read_range(app, buffer, pos, pos+size, read_buffer);
                            if (match_insensitive_ss(needle_str, read_str)){
                                *result = pos;
                                goto finished;
                            }
                        }
                    }
                    still_looping = backward_stream_chunk(&stream);
                }while (still_looping);
            }
        }
        
        *result = -1;
        
        finished:;
    }
}

//
// Fundamental Editing
//

inline float
get_view_y(View_Summary view){
    float y = view.cursor.wrapped_y;
    if (view.unwrapped_lines){
        y = view.cursor.unwrapped_y;
    }
    return(y);
}

inline float
get_view_x(View_Summary view){
    float x = view.cursor.wrapped_x;
    if (view.unwrapped_lines){
        x = view.cursor.unwrapped_x;
    }
    return(x);
}

CUSTOM_COMMAND_SIG(write_character){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    
    User_Input in = app->get_command_input(app);
    char character = 0;
    
    if (in.type == UserInputKey){
        character = in.key.character;
    }
    
    if (character != 0){
        Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
        
        int32_t pos = view.cursor.pos;
        int32_t next_pos = pos + 1;
        app->buffer_replace_range(app, &buffer,
                                  pos, pos, &character, 1);
        app->view_set_cursor(app, &view, seek_pos(next_pos), true);
    }
}

CUSTOM_COMMAND_SIG(delete_char){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    int32_t pos = view.cursor.pos;
    if (0 < buffer.size && pos < buffer.size){
        app->buffer_replace_range(app, &buffer,
                                  pos, pos+1, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(backspace_char){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    int32_t pos = view.cursor.pos;
    if (0 < pos && pos <= buffer.size){
        app->buffer_replace_range(app, &buffer,
                                  pos-1, pos, 0, 0);
        
        app->view_set_cursor(app, &view, seek_pos(pos-1), true);
    }
}

CUSTOM_COMMAND_SIG(set_mark){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    
    app->view_set_mark(app, &view, seek_pos(view.cursor.pos));
    // TODO(allen): Just expose the preferred_x seperately
    app->view_set_cursor(app, &view, seek_pos(view.cursor.pos), true);
}

CUSTOM_COMMAND_SIG(cursor_mark_swap){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    
    int32_t cursor = view.cursor.pos;
    int32_t mark = view.mark.pos;
    
    app->view_set_cursor(app, &view, seek_pos(mark), true);
    app->view_set_mark(app, &view, seek_pos(cursor));
}

CUSTOM_COMMAND_SIG(delete_range){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    Range range = get_range(&view);
    
    app->buffer_replace_range(app, &buffer,
                              range.min, range.max,
                              0, 0);
}

//
// Basic Navigation
//

CUSTOM_COMMAND_SIG(center_view){
    View_Summary view = app->get_active_view(app, AccessProtected);
    
    i32_Rect region = view.file_region;
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    float h = (float)(region.y1 - region.y0);
    float y = get_view_y(view);
    y = y - h*.5f;
    scroll.target_y = (int32_t)(y + .5f);
    app->view_set_scroll(app, &view, scroll);
}

CUSTOM_COMMAND_SIG(left_adjust_view){
    View_Summary view = app->get_active_view(app, AccessProtected);
    
    GUI_Scroll_Vars scroll = view.scroll_vars;
    
    float x = get_view_x(view);
    x = x - 30.f;
    scroll.target_x = (int32_t)(x + .5f);
    app->view_set_scroll(app, &view, scroll);
}

int32_t
get_relative_xy(View_Summary *view, int32_t x, int32_t y, float *x_out, float *y_out){
    int32_t result = false;
    
    i32_Rect region = view->file_region;
    
    int32_t max_x = (region.x1 - region.x0);
    int32_t max_y = (region.y1 - region.y0);
    GUI_Scroll_Vars scroll_vars = view->scroll_vars;
    
    int32_t rx = x - region.x0;
    int32_t ry = y - region.y0;
    
    if (ry >= 0){
        if (rx >= 0 && rx < max_x && ry >= 0 && ry < max_y){
            result = 1;
        }
    }
    
    *x_out = (float)rx + scroll_vars.scroll_x;
    *y_out = (float)ry + scroll_vars.scroll_y;
    
    return(result);
}

CUSTOM_COMMAND_SIG(click_set_cursor){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    
    Mouse_State mouse = app->get_mouse_state(app);
    float rx = 0, ry = 0;
    if (get_relative_xy(&view, mouse.x, mouse.y, &rx, &ry)){
        app->view_set_cursor(app, &view,
                             seek_xy(rx, ry, true,
                                     view.unwrapped_lines),
                             true);
    }
}

CUSTOM_COMMAND_SIG(click_set_mark){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    
    Mouse_State mouse = app->get_mouse_state(app);
    float rx = 0, ry = 0;
    if (get_relative_xy(&view, mouse.x, mouse.y, &rx, &ry)){
        app->view_set_mark(app, &view,
                           seek_xy(rx, ry, true,
                                   view.unwrapped_lines)
                           );
    }
}

inline void
move_vertical(Application_Links *app, float line_multiplier){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    
    float new_y = get_view_y(view) + line_multiplier*view.line_height;
    float x = view.preferred_x;
    
    app->view_set_cursor(app, &view,
                         seek_xy(x, new_y, false, view.unwrapped_lines),
                         false);
}

CUSTOM_COMMAND_SIG(move_up){
    move_vertical(app, -1.f);
}

CUSTOM_COMMAND_SIG(move_down){
    move_vertical(app, 1.f);
}

CUSTOM_COMMAND_SIG(move_up_10){
    move_vertical(app, -10.f);
}

CUSTOM_COMMAND_SIG(move_down_10){
    move_vertical(app, 10.f);
}

static float
get_page_jump(View_Summary *view){
    i32_Rect region = view->file_region;
    float page_jump = 1;
    
    if (view->line_height > 0){
        page_jump = (float)(region.y1 - region.y0) / view->line_height;
        page_jump -= 3.f;
        if (page_jump <= 0){
            page_jump = 1.f;
        }
    }
    
    return(page_jump);
}

CUSTOM_COMMAND_SIG(page_up){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    float page_jump = get_page_jump(&view);
    move_vertical(app, -page_jump);
}

CUSTOM_COMMAND_SIG(page_down){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    float page_jump = get_page_jump(&view);
    move_vertical(app, page_jump);
}


CUSTOM_COMMAND_SIG(move_left){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    int32_t new_pos = view.cursor.pos - 1;
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

CUSTOM_COMMAND_SIG(move_right){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    int32_t new_pos = view.cursor.pos + 1;
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

//
// Auto Indenting and Whitespace
//

static int32_t
seek_line_end(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int32_t still_looping;
    char at_pos;
    
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        still_looping = 1;
        do{
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        
        if (pos > buffer->size){
            pos = buffer->size;
        }
    }
    
    return(pos);
}

static int32_t
seek_line_beginning(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int32_t still_looping;
    char at_pos;
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        still_looping = 1;
        do{
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    goto double_break;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }while(still_looping);
        double_break:;
        
        if (pos != 0){
            ++pos;
        }
        if (pos < 0){
            pos = 0;
        }
    }
    
    return(pos);
}

static void
move_past_lead_whitespace(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    refresh_view(app, view);
    
    int32_t new_pos = seek_line_beginning(app, buffer, view->cursor.pos);
    char space[1024];
    Stream_Chunk chunk = {0};
    int32_t still_looping = false;
    
    int32_t i = new_pos;
    if (init_stream_chunk(&chunk, app, buffer, i, space, sizeof(space))){
        do{
            for (; i < chunk.end; ++i){
                char at_pos = chunk.data[i];
                if (at_pos == '\n' || !char_is_whitespace(at_pos)){
                    goto break2;
                }
            }
            still_looping = forward_stream_chunk(&chunk);
        }while(still_looping);
        break2:;
        
        if (i > view->cursor.pos){
            app->view_set_cursor(app, view, seek_pos(i), true);
        }
    }
}

CUSTOM_COMMAND_SIG(auto_tab_line_at_cursor){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    app->buffer_auto_indent(app, &buffer,
                            view.cursor.pos, view.cursor.pos,
                            DEF_TAB_WIDTH,
                            DEFAULT_INDENT_FLAGS);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(auto_tab_whole_file){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    app->buffer_auto_indent(app, &buffer,
                            0, buffer.size,
                            DEF_TAB_WIDTH,
                            DEFAULT_INDENT_FLAGS);
}

CUSTOM_COMMAND_SIG(auto_tab_range){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    Range range = get_range(&view);
    
    app->buffer_auto_indent(app, &buffer,
                            range.min, range.max,
                            DEF_TAB_WIDTH,
                            DEFAULT_INDENT_FLAGS);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(write_and_auto_tab){
    exec_command(app, write_character);
    exec_command(app, auto_tab_line_at_cursor);
}

CUSTOM_COMMAND_SIG(clean_all_lines){
    // TODO(allen): This command always iterates accross the entire
    // buffer, so streaming it is actually the wrong call.  Rewrite this
    // to minimize calls to app->buffer_read_range.
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    
    int32_t line_count = buffer.line_count;
    int32_t edit_max = line_count;
    
    if (edit_max*sizeof(Buffer_Edit) < app->memory_size){
        Buffer_Edit *edits = (Buffer_Edit*)app->memory;
        
        char data[1024];
        Stream_Chunk chunk = {0};
        
        int32_t i = 0;
        if (init_stream_chunk(&chunk, app, &buffer,
                              i, data, sizeof(data))){
            Buffer_Edit *edit = edits;
            
            int32_t buffer_size = buffer.size;
            int32_t still_looping = true;
            int32_t last_hard = buffer_size;
            do{
                for (; i < chunk.end; ++i){
                    char at_pos = chunk.data[i];
                    if (at_pos == '\n'){
                        if (last_hard+1 < i){
                            edit->str_start = 0;
                            edit->len = 0;
                            edit->start = last_hard+1;
                            edit->end = i;
                            ++edit;
                        }
                        last_hard = buffer_size;
                    }
                    else if (char_is_whitespace(at_pos)){
                        // NOTE(allen): do nothing
                    }
                    else{
                        last_hard = i;
                    }
                }
                
                still_looping = forward_stream_chunk(&chunk);
            }while(still_looping);
            
            if (last_hard+1 < buffer_size){
                edit->str_start = 0;
                edit->len = 0;
                edit->start = last_hard+1;
                edit->end = buffer_size;
                ++edit;
            }
            
            int32_t edit_count = (int32_t)(edit - edits);
            app->buffer_batch_edit(app, &buffer, 0, 0, edits, edit_count, BatchEdit_PreserveTokens);
        }
    }
}

//
// Clipboard
//

static int32_t
clipboard_copy(Application_Links *app, int32_t start, int32_t end, Buffer_Summary *buffer_out,
               uint32_t access){
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    int32_t result = false;
    
    if (buffer.exists){
        if (0 <= start && start <= end && end <= buffer.size){
            int32_t size = (end - start);
            char *str = (char*)app->memory;
            
            if (size <= app->memory_size){
                app->buffer_read_range(app, &buffer, start, end, str);
                app->clipboard_post(app, 0, str, size);
                if (buffer_out){*buffer_out = buffer;}
                result = true;
            }
        }
    }
    
    return(result);
}

static int32_t
clipboard_cut(Application_Links *app, int32_t start, int32_t end, Buffer_Summary *buffer_out,
              uint32_t access){
    Buffer_Summary buffer = {0};
    int32_t result = false;
    
    if (clipboard_copy(app, start, end, &buffer, access)){
        app->buffer_replace_range(app, &buffer, start, end, 0, 0);
        if (buffer_out){*buffer_out = buffer;}
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(copy){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Range range = get_range(&view);
    clipboard_copy(app, range.min, range.max, 0, access);
}

CUSTOM_COMMAND_SIG(cut){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Range range = get_range(&view);
    clipboard_cut(app, range.min, range.max, 0, access);
}

enum Rewrite_Type{
    RewriteNone,
    RewritePaste,
    RewriteWordComplete
};

struct View_Paste_Index{
    int32_t rewrite;
    int32_t next_rewrite;
    int32_t index;
};

View_Paste_Index view_paste_index_[16];
View_Paste_Index *view_paste_index = view_paste_index_ - 1;

CUSTOM_COMMAND_SIG(paste){
    uint32_t access = AccessOpen;
    int32_t count = app->clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = app->get_active_view(app, access);
        
        view_paste_index[view.view_id].next_rewrite = RewritePaste;
        
        int32_t paste_index = 0;
        view_paste_index[view.view_id].index = paste_index;
        
        int32_t len = app->clipboard_index(app, 0, paste_index, 0, 0);
        char *str = 0;
        
        if (len <= app->memory_size){
            str = (char*)app->memory;
        }
        
        if (str){
            app->clipboard_index(app, 0, paste_index, str, len);
            
            Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
            int32_t pos = view.cursor.pos;
            app->buffer_replace_range(app, &buffer, pos, pos, str, len);
            app->view_set_mark(app, &view, seek_pos(pos));
            app->view_set_cursor(app, &view, seek_pos(pos + len), true);
            
            // TODO(allen): Send this to all views.
            Theme_Color paste;
            paste.tag = Stag_Paste;
            app->get_theme_colors(app, &paste, 1);
            app->view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_next){
    uint32_t access = AccessOpen;
    int32_t count = app->clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = app->get_active_view(app, access);
        
        if (view_paste_index[view.view_id].rewrite == RewritePaste){
            view_paste_index[view.view_id].next_rewrite = RewritePaste;
            
            int32_t paste_index = view_paste_index[view.view_id].index + 1;
            view_paste_index[view.view_id].index = paste_index;
            
            int32_t len = app->clipboard_index(app, 0, paste_index, 0, 0);
            char *str = 0;
            
            if (len <= app->memory_size){
                str = (char*)app->memory;
            }
            
            if (str){
                app->clipboard_index(app, 0, paste_index, str, len);
                
                Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
                Range range = get_range(&view);
                int32_t pos = range.min;
                
                app->buffer_replace_range(app, &buffer, range.min, range.max, str, len);
                app->view_set_cursor(app, &view, seek_pos(pos + len), true);
                
                // TODO(allen): Send this to all views.
                Theme_Color paste;
                paste.tag = Stag_Paste;
                app->get_theme_colors(app, &paste, 1);
                app->view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
            }
        }
        else{
            exec_command(app, paste);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_and_indent){
    exec_command(app, paste);
    exec_command(app, auto_tab_range);
}

CUSTOM_COMMAND_SIG(paste_next_and_indent){
    exec_command(app, paste_next);
    exec_command(app, auto_tab_range);
}

//
// Fancy Editing
//

CUSTOM_COMMAND_SIG(to_uppercase){
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_range(&view);
    int32_t size = range.max - range.min;
    if (size <= app->memory_size){
        char *mem = (char*)app->memory;
        
        app->buffer_read_range(app, &buffer, range.min, range.max, mem);
        for (int32_t i = 0; i < size; ++i){
            mem[i] = char_to_upper(mem[i]);
        }
        app->buffer_replace_range(app, &buffer, range.min, range.max, mem, size);
        app->view_set_cursor(app, &view, seek_pos(range.max), true);
    }
}

CUSTOM_COMMAND_SIG(to_lowercase){
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_range(&view);
    int32_t size = range.max - range.min;
    if (size <= app->memory_size){
        char *mem = (char*)app->memory;
        
        app->buffer_read_range(app, &buffer, range.min, range.max, mem);
        for (int32_t i = 0; i < size; ++i){
            mem[i] = char_to_lower(mem[i]);
        }
        app->buffer_replace_range(app, &buffer, range.min, range.max, mem, size);
        app->view_set_cursor(app, &view, seek_pos(range.max), true);
    }
}

//
// Various Forms of Seek
//

static int32_t
buffer_seek_whitespace_up(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int32_t no_hard;
    int32_t still_looping;
    char at_pos;
    
    --pos;
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        // Step 1: Find the first non-whitespace character
        // behind the current position.
        still_looping = true;
        do{
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (!char_is_whitespace(at_pos)){
                    goto double_break_1;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        } while(still_looping);
        double_break_1:;
        
        // Step 2: Continue scanning backward, at each '\n'
        // mark the beginning of another line by setting
        // no_hard to true, set it back to false if a
        // non-whitespace character is discovered before
        // the next '\n'
        no_hard = false;
        while (still_looping){
            for (; pos >= stream.start; --pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                    }
                }
                else if (!char_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = backward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (pos != 0){
            ++pos;
        }
    }

    return(pos);
}

static int32_t
buffer_seek_whitespace_down(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char chunk[1024];
    int32_t chunk_size = sizeof(chunk);
    Stream_Chunk stream = {0};
    
    int32_t no_hard;
    int32_t prev_endline;
    int32_t still_looping;
    char at_pos;
    
    if (init_stream_chunk(&stream, app, buffer, pos, chunk, chunk_size)){
        // step 1: find the first non-whitespace character
        // ahead of the current position.
        still_looping = true;
        do{
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (!char_is_whitespace(at_pos)){
                    goto double_break_1;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        } while(still_looping);
        double_break_1:;
        
        // step 2: continue scanning forward, at each '\n'
        // mark it as the beginning of a new line by updating
        // the prev_endline value.  if another '\n' is found
        // with non-whitespace then the previous line was
        // all whitespace.
        no_hard = false;
        prev_endline = -1;
        while(still_looping){
            for (; pos < stream.end; ++pos){
                at_pos = stream.data[pos];
                if (at_pos == '\n'){
                    if (no_hard){
                        goto double_break_2;
                    }
                    else{
                        no_hard = true;
                        prev_endline = pos;
                    }
                }
                else if (!char_is_whitespace(at_pos)){
                    no_hard = false;
                }
            }
            still_looping = forward_stream_chunk(&stream);
        }
        double_break_2:;
        
        if (prev_endline == -1 || prev_endline+1 >= buffer->size){
            pos = buffer->size;
        }
        else{
            pos = prev_endline+1;
        }
    }
    
    return(pos);
}

CUSTOM_COMMAND_SIG(seek_whitespace_up){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = buffer_seek_whitespace_up(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

CUSTOM_COMMAND_SIG(seek_whitespace_down){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = buffer_seek_whitespace_down(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view,
                         seek_pos(new_pos),
                         true);
}

CUSTOM_COMMAND_SIG(seek_end_of_line){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = seek_line_end(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view, seek_pos(new_pos), true);
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    int32_t new_pos = seek_line_beginning(app, &buffer, view.cursor.pos);
    app->view_set_cursor(app, &view, seek_pos(new_pos), true);
}

static void
basic_seek(Application_Links *app, int32_t seek_type, uint32_t flags){
    uint32_t access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    int32_t pos = app->buffer_boundary_seek(app, &buffer, view.cursor.pos, seek_type, flags);
    app->view_set_cursor(app, &view, seek_pos(pos), true);
}

#define seek_command(n, dir, flags)\
CUSTOM_COMMAND_SIG(seek_##n##_##dir){ basic_seek(app, dir, flags); }

#define right true
#define left false

seek_command(whitespace,            right, BoundaryWhitespace)
seek_command(whitespace,            left,  BoundaryWhitespace)
seek_command(token,                 right, BoundaryToken)
seek_command(token,                 left,  BoundaryToken)
seek_command(white_or_token,        right, BoundaryToken | BoundaryWhitespace)
seek_command(white_or_token,        left,  BoundaryToken | BoundaryWhitespace)
seek_command(alphanumeric,          right, BoundaryAlphanumeric)
seek_command(alphanumeric,          left,  BoundaryAlphanumeric)
seek_command(alphanumeric_or_camel, right, BoundaryAlphanumeric | BoundaryCamelCase)
seek_command(alphanumeric_or_camel, left,  BoundaryAlphanumeric | BoundaryCamelCase)

#undef right
#undef left


//
// special string writing commands
//

static void
write_string(Application_Links *app, String string){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    app->buffer_replace_range(app, &buffer,
                              view.cursor.pos, view.cursor.pos,
                              string.str, string.size);
    app->view_set_cursor(app, &view, seek_pos(view.cursor.pos + string.size), true);
}

CUSTOM_COMMAND_SIG(write_increment){
    write_string(app, make_lit_string("++"));
}

static void
long_braces(Application_Links *app, char *text, int32_t size){
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    int32_t pos = view.cursor.pos;
    
    app->buffer_replace_range(app, &buffer, pos, pos, text, size);
    app->view_set_cursor(app, &view, seek_pos(pos + 2), true);
    
    app->buffer_auto_indent(app, &buffer,
                            pos, pos + size,
                            DEF_TAB_WIDTH,
                            DEFAULT_INDENT_FLAGS);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(open_long_braces){
    char text[] = "{\n\n}";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_semicolon){
    char text[] = "{\n\n};";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_break){
    char text[] = "{\n\n}break;";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

// TODO(allen): have this thing check if it is on
// a blank line and insert newlines as needed.
CUSTOM_COMMAND_SIG(if0_off){
    char text1[] = "\n#if 0";
    int32_t size1 = sizeof(text1) - 1;
    
    char text2[] = "#endif\n";
    int32_t size2 = sizeof(text2) - 1;
    
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_range(&view);
    
    if (range.min < range.max){
        Buffer_Edit edits[2];
        char *str = 0;
        char *base = (char*)partition_current(&global_part);
        
        str = push_array(&global_part, char, size1);
        memcpy(str, text1, size1);
        edits[0].str_start = (int32_t)(str - base);
        edits[0].len = size1;
        edits[0].start = range.min;
        edits[0].end = range.min;
        
        str = push_array(&global_part, char, size2);
        memcpy(str, text2, size2);
        edits[1].str_start = (int32_t)(str - base);
        edits[1].len = size2;
        edits[1].start = range.max;
        edits[1].end = range.max;
        
        app->buffer_batch_edit(app,&buffer,
                               base, global_part.pos,
                               edits, ArrayCount(edits), BatchEdit_Normal);
        
        view = app->get_view(app, view.view_id, AccessAll);
        if (view.cursor.pos > view.mark.pos){
            app->view_set_cursor(app, &view,
                                 seek_line_char(view.cursor.line+1, view.cursor.character), 
                                 true);
        }
        else{
            app->view_set_mark(app, &view,
                               seek_line_char(view.mark.line+1, view.mark.character));
        }
        
        range = get_range(&view);
        app->buffer_auto_indent(app, &buffer,
                                range.min, range.max,
                                DEF_TAB_WIDTH,
                                DEFAULT_INDENT_FLAGS);
        move_past_lead_whitespace(app, &view, &buffer);
    }
}


//
// Fast Deletes 
//

CUSTOM_COMMAND_SIG(backspace_word){
    uint32_t access = AccessOpen;
    
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    if (buffer.exists){
        int32_t pos2 = 0, pos1 = 0;
        
        pos2 = view.cursor.pos;
        exec_command(app, seek_alphanumeric_left);
        refresh_view(app, &view);
        pos1 = view.cursor.pos;
        
        app->buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(delete_word){
    uint32_t access = AccessOpen;
    
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    if (buffer.exists){
        int32_t pos2 = 0, pos1 = 0;
        
        pos1 = view.cursor.pos;
        exec_command(app, seek_alphanumeric_right);
        refresh_view(app, &view);
        pos2 = view.cursor.pos;
        
        app->buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(snipe_token_or_word){
    uint32_t access = AccessOpen;
    
    View_Summary view;
    Buffer_Summary buffer;
    int32_t pos1, pos2;
    
    view = app->get_active_view(app, access);
    buffer = app->get_buffer(app, view.buffer_id, access);
    
    pos1 = app->buffer_boundary_seek(app, &buffer, view.cursor.pos, false, BoundaryToken | BoundaryWhitespace);
    pos2 = app->buffer_boundary_seek(app, &buffer, pos1,            true,  BoundaryToken | BoundaryWhitespace);
    
    Range range = make_range(pos1, pos2);
    app->buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
}

//
// Scroll Bar Controlling
//

CUSTOM_COMMAND_SIG(show_scrollbar){
    View_Summary view = app->get_active_view(app, AccessProtected);
    app->view_set_setting(app, &view, ViewSetting_ShowScrollbar, true);
}

CUSTOM_COMMAND_SIG(hide_scrollbar){
    View_Summary view = app->get_active_view(app, AccessProtected);
    app->view_set_setting(app, &view, ViewSetting_ShowScrollbar, false);
}

//
// Panel Management
//

static void
get_view_next_looped(Application_Links *app, View_Summary *view, uint32_t access){
    app->get_view_next(app, view, access);
    if (!view->exists){
        *view = app->get_view_first(app, access);
    }
}

static View_ID special_note_view_id = 0;

static void
close_special_note_view(Application_Links *app){
    View_Summary special_view = app->get_view(app, special_note_view_id, AccessAll);
    if (special_view.exists){
        app->close_view(app, &special_view);
    }
    special_note_view_id = 0;
}

static View_Summary
open_special_note_view(Application_Links *app, bool32 create_if_not_exist = true){
    View_Summary special_view = app->get_view(app, special_note_view_id, AccessAll);
    
    if (create_if_not_exist && !special_view.exists){
        View_Summary view = app->get_active_view(app, AccessAll);
        special_view = app->open_view(app, &view, ViewSplit_Bottom);
        app->view_set_setting(app, &special_view, ViewSetting_ShowScrollbar, false);
        app->view_set_split_proportion(app, &special_view, .2f);
        app->set_active_view(app, &view);
        special_note_view_id = special_view.view_id;
    }
    
    return(special_view);
}

CUSTOM_COMMAND_SIG(change_active_panel){
    View_Summary view = app->get_active_view(app, AccessAll);
    View_ID original_view_id = view.view_id;
    
    do{
        get_view_next_looped(app, &view, AccessAll);
        if (view.view_id != special_note_view_id){
            break;
        }
    }while(view.view_id != original_view_id);
    
    if (view.exists){
        app->set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(close_panel){
    View_Summary view = app->get_active_view(app, AccessAll);
    app->close_view(app, &view);
}

CUSTOM_COMMAND_SIG(open_panel_vsplit){
    View_Summary view = app->get_active_view(app, AccessAll);
    View_Summary new_view = app->open_view(app, &view, ViewSplit_Right);
    app->view_set_setting(app, &new_view, ViewSetting_ShowScrollbar, false);
}

CUSTOM_COMMAND_SIG(open_panel_hsplit){
    View_Summary view = app->get_active_view(app, AccessAll);
    View_Summary new_view = app->open_view(app, &view, ViewSplit_Bottom);
    app->view_set_setting(app, &new_view, ViewSetting_ShowScrollbar, false);
}

//
// Open File In Quotes
//

static int32_t
file_name_in_quotes(Application_Links *app, String *file_name){
    int32_t result = false;
    uint32_t access = AccessProtected;
    
    View_Summary view;
    Buffer_Summary buffer;
    char short_file_name[128];
    int32_t pos, start, end, size;
    
    view = app->get_active_view(app, access);
    buffer = app->get_buffer(app, view.buffer_id, access);
    pos = view.cursor.pos;
    buffer_seek_delimiter_forward(app, &buffer, pos, '"', &end);
    buffer_seek_delimiter_backward(app, &buffer, pos, '"', &start);
    
    ++start;
    size = end - start;
    
    // NOTE(allen): This check is necessary because app->buffer_read_range
    // requiers that the output buffer you provide is at least (end - start) bytes long.
    if (size < sizeof(short_file_name)){
        if (app->buffer_read_range(app, &buffer, start, end, short_file_name)){
            result = true;
            copy_ss(file_name, make_string(buffer.file_name, buffer.file_name_len));
            remove_last_folder(file_name);
            append_ss(file_name, make_string(short_file_name, size));
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(open_file_in_quotes){
    char file_name_[256];
    String file_name = make_fixed_width_string(file_name_);
    
    if (file_name_in_quotes(app, &file_name)){
        exec_command(app, change_active_panel);
        View_Summary view = app->get_active_view(app, AccessAll);
        view_open_file(app, &view, expand_str(file_name), true);
    }
}

CUSTOM_COMMAND_SIG(open_in_other){
    exec_command(app, change_active_panel);
    exec_command(app, cmdid_interactive_open);
}


CUSTOM_COMMAND_SIG(save_as){
    exec_command(app, cmdid_save_as);
}

CUSTOM_COMMAND_SIG(goto_line){
    uint32_t access = AccessProtected;
    
    Query_Bar bar = {0};
    char string_space[256];
    
    bar.prompt = make_lit_string("Goto Line: ");
    bar.string = make_fixed_width_string(string_space);
    
    if (query_user_number(app, &bar)){
        int32_t line_number = str_to_int_s(bar.string);
        active_view_to_line(app, access, line_number);
    }
}

CUSTOM_COMMAND_SIG(search);
CUSTOM_COMMAND_SIG(reverse_search);

static void
isearch(Application_Links *app, int32_t start_reversed){
    uint32_t access = AccessProtected;
    
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    if (!buffer.exists) return;
    
    Query_Bar bar = {0};
    if (app->start_query_bar(app, &bar, 0) == 0) return;
    
    int32_t reverse = start_reversed;
    int32_t pos = view.cursor.pos;
    int32_t start_pos = pos;
    int32_t first_pos = pos;
    Range match = make_range(pos, pos);
    
    char bar_string_space[256];
    bar.string = make_fixed_width_string(bar_string_space);
    
    String isearch_str = make_lit_string("I-Search: ");
    String rsearch_str = make_lit_string("Reverse-I-Search: ");
    
    User_Input in = {0};
    for (;;){
        app->view_set_highlight(app, &view, match.start, match.end, true);
        
        // NOTE(allen): Change the bar's prompt to match the current direction.
        if (reverse) bar.prompt = rsearch_str;
        else bar.prompt = isearch_str;
        
        in = app->get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        if (in.abort) break;
        
        // NOTE(allen): If we're getting mouse events here it's a 4coder bug, because we
        // only asked to intercept key events.
        
        assert(in.type == UserInputKey);
        
        int32_t made_change = 0;
        if (in.key.keycode == '\n' || in.key.keycode == '\t'){
            break;
        }
        else if (in.key.character && key_is_unmodified(&in.key)){
            append_s_char(&bar.string, in.key.character);
            made_change = 1;
        }
        else if (in.key.keycode == key_back){
            if (bar.string.size > 0){
                --bar.string.size;
                made_change = 1;
            }
        }
        
        int32_t step_forward = 0;
        int32_t step_backward = 0;
        
        if ((in.command.command == search) ||
            in.key.keycode == key_page_down || in.key.keycode == key_down) step_forward = 1;
        if ((in.command.command == reverse_search) ||
            in.key.keycode == key_page_up || in.key.keycode == key_up) step_backward = 1;
        
        start_pos = pos;
        if (step_forward && reverse){
            start_pos = match.start + 1;
            pos = start_pos;
            reverse = 0;
            step_forward = 0;
        }
        if (step_backward && !reverse){
            start_pos = match.start - 1;
            pos = start_pos;
            reverse = 1;
            step_backward = 0;
        }
        
        if (in.key.keycode != key_back){
            int32_t new_pos;
            if (reverse){
                buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1, 0,
                                                        bar.string.str, bar.string.size, &new_pos);
                if (new_pos >= 0){
                    if (step_backward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_backward(app, &buffer, start_pos - 1, 0,
                                                                bar.string.str, bar.string.size, &new_pos);
                        if (new_pos < 0) new_pos = start_pos;
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
            else{
                buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1, 0,
                                                       bar.string.str, bar.string.size, &new_pos);
                if (new_pos < buffer.size){
                    if (step_forward){
                        pos = new_pos;
                        start_pos = new_pos;
                        buffer_seek_string_insensitive_forward(app, &buffer, start_pos + 1, 0,
                                                               bar.string.str, bar.string.size, &new_pos);
                        if (new_pos >= buffer.size) new_pos = start_pos;
                    }
                    match.start = new_pos;
                    match.end = match.start + bar.string.size;
                }
            }
        }
        else{
            if (match.end > match.start + bar.string.size){
                match.end = match.start + bar.string.size;
            }
        }
    }
    app->view_set_highlight(app, &view, 0, 0, false);
    if (in.abort){
        app->view_set_cursor(app, &view, seek_pos(first_pos), true);
        return;
    }
    
    app->view_set_cursor(app, &view, seek_pos(match.min), true);
}

CUSTOM_COMMAND_SIG(search){
    isearch(app, false);
}

CUSTOM_COMMAND_SIG(reverse_search){
    isearch(app, true);
}

CUSTOM_COMMAND_SIG(replace_in_range){
    Query_Bar replace;
    char replace_space[1024];
    replace.prompt = make_lit_string("Replace: ");
    replace.string = make_fixed_width_string(replace_space);
    
    Query_Bar with;
    char with_space[1024];
    with.prompt = make_lit_string("With: ");
    with.string = make_fixed_width_string(with_space);
    
    if (!query_user_string(app, &replace)) return;
    if (replace.string.size == 0) return;
    
    if (!query_user_string(app, &with)) return;
    
    String r, w;
    r = replace.string;
    w = with.string;
    
    uint32_t access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    
    Range range = get_range(&view);
    
    int32_t pos, new_pos;
    pos = range.min;
    
    buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    
    while (new_pos + r.size <= range.end){
        app->buffer_replace_range(app, &buffer, new_pos, new_pos + r.size, w.str, w.size);
        refresh_view(app, &view);
        range = get_range(&view);
        pos = new_pos + w.size;
        buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    }
}

CUSTOM_COMMAND_SIG(query_replace){
    Query_Bar replace;
    char replace_space[1024];
    replace.prompt = make_lit_string("Replace: ");
    replace.string = make_fixed_width_string(replace_space);
    
    Query_Bar with;
    char with_space[1024];
    with.prompt = make_lit_string("With: ");
    with.string = make_fixed_width_string(with_space);
    
    if (!query_user_string(app, &replace)) return;
    if (replace.string.size == 0) return;
    
    if (!query_user_string(app, &with)) return;
    
    String r, w;
    r = replace.string;
    w = with.string;
    
    Query_Bar bar;
    Buffer_Summary buffer;
    View_Summary view;
    int32_t pos, new_pos;
    
    bar.prompt = make_lit_string("Replace? (y)es, (n)ext, (esc)\n");
    bar.string = null_string;
    
    app->start_query_bar(app, &bar, 0);
    
    uint32_t access = AccessOpen;
    view = app->get_active_view(app, access);
    buffer = app->get_buffer(app, view.buffer_id, access);
    
    pos = view.cursor.pos;
    buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    
    User_Input in = {0};
    while (new_pos < buffer.size){
        Range match = make_range(new_pos, new_pos + r.size);
        app->view_set_highlight(app, &view, match.min, match.max, 1);
        
        in = app->get_user_input(app, EventOnAnyKey, EventOnButton);
        if (in.abort || in.key.keycode == key_esc || !key_is_unmodified(&in.key))  break;
        
        if (in.key.character == 'y' ||
            in.key.character == 'Y' ||
            in.key.character == '\n' ||
            in.key.character == '\t'){
            app->buffer_replace_range(app, &buffer, match.min, match.max, w.str, w.size);
            pos = match.start + w.size;
        }
        else{
            pos = match.max;
        }
        
        buffer_seek_string_forward(app, &buffer, pos, 0, r.str, r.size, &new_pos);
    }
    
    app->view_set_highlight(app, &view, 0, 0, 0);
    if (in.abort) return;
    
    app->view_set_cursor(app, &view, seek_pos(pos), 1);
}

//
// Fast Buffer Management
//

CUSTOM_COMMAND_SIG(close_all_code){
    String extension;
    Buffer_Summary buffer;
    
    // TODO(allen): Get better memory constructs to the custom layer
    // so that it doesn't have to rely on arbitrary limits like this one.
    int32_t buffers_to_close[2048];
    int32_t buffers_to_close_count = 0;
    
    uint32_t access = AccessAll;
    for (buffer = app->get_buffer_first(app, access);
         buffer.exists;
         app->get_buffer_next(app, &buffer, access)){
        
        extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match_ss(extension, make_lit_string("cpp")) ||
            match_ss(extension, make_lit_string("hpp")) ||
            match_ss(extension, make_lit_string("c")) ||
            match_ss(extension, make_lit_string("h")) ||
            match_ss(extension, make_lit_string("cc"))){
            
            buffers_to_close[buffers_to_close_count++] = buffer.buffer_id;
        }
    }
    
    for (int32_t i = 0; i < buffers_to_close_count; ++i){
        app->kill_buffer(app, buffer_identifier(buffers_to_close[i]), true, 0);
    }
}

CUSTOM_COMMAND_SIG(open_all_code){
    // NOTE(allen|a3.4.4): This method of getting the hot directory works
    // because this custom.cpp gives no special meaning to app->memory
    // and doesn't set up a persistent allocation system within app->memory.
    // push_directory isn't a very good option since it's tied to the parameter
    // stack, so I am phasing that idea out now.
    String dir = make_string_cap(app->memory, 0, app->memory_size);
    dir.size = app->directory_get_hot(app, dir.str, dir.memory_size);
    int32_t dir_size = dir.size;
    
    // NOTE(allen|a3.4.4): Here we get the list of files in this directory.
    // Notice that we free_file_list at the end.
    File_List list = app->get_file_list(app, dir.str, dir.size);
    
    for (int32_t i = 0; i < list.count; ++i){
        File_Info *info = list.infos + i;
        if (!info->folder){
            String extension = make_string_cap(info->filename, info->filename_len, info->filename_len+1);
            extension = file_extension(extension);
            if (match_ss(extension, make_lit_string("cpp")) ||
                match_ss(extension, make_lit_string("hpp")) ||
                match_ss(extension, make_lit_string("c")) ||
                match_ss(extension, make_lit_string("h")) ||
                match_ss(extension, make_lit_string("cc"))){
                // NOTE(allen): There's no way in the 4coder API to use relative
                // paths at the moment, so everything should be full paths.  Which is
                // managable.  Here simply set the dir string size back to where it
                // was originally, so that new appends overwrite old ones.
                dir.size = dir_size;
                append_sc(&dir, info->filename);
                app->create_buffer(app, dir.str, dir.size, 0);
            }
        }
    }
    
    app->free_file_list(app, list);
}

char out_buffer_space[1024];
char command_space[1024];
char hot_directory_space[1024];

CUSTOM_COMMAND_SIG(execute_any_cli){
    Query_Bar bar_out = {0};
    Query_Bar bar_cmd = {0};
    
    bar_out.prompt = make_lit_string("Output Buffer: ");
    bar_out.string = make_fixed_width_string(out_buffer_space);
    if (!query_user_string(app, &bar_out)) return;
    
    bar_cmd.prompt = make_lit_string("Command: ");
    bar_cmd.string = make_fixed_width_string(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    
    String hot_directory = make_fixed_width_string(hot_directory_space);
    hot_directory.size = app->directory_get_hot(app, hot_directory.str, hot_directory.memory_size);
    
    uint32_t access = AccessAll;
    View_Summary view = app->get_active_view(app, access);
    
    app->exec_system_command(app, &view,
                             buffer_identifier(bar_out.string.str, bar_out.string.size),
                             hot_directory.str, hot_directory.size,
                             bar_cmd.string.str, bar_cmd.string.size,
                             CLI_OverlapWithConflict | CLI_CursorAtEnd);
}

CUSTOM_COMMAND_SIG(execute_previous_cli){
    String out_buffer = make_string_slowly(out_buffer_space);
    String cmd = make_string_slowly(command_space);
    String hot_directory = make_string_slowly(hot_directory_space);
    
    if (out_buffer.size > 0 && cmd.size > 0 && hot_directory.size > 0){
        uint32_t access = AccessAll;
        View_Summary view = app->get_active_view(app, access);
        
        app->exec_system_command(app, &view,
                                 buffer_identifier(out_buffer.str, out_buffer.size),
                                 hot_directory.str, hot_directory.size,
                                 cmd.str, cmd.size,
                                 CLI_OverlapWithConflict | CLI_CursorAtEnd);
    }
}

//
// Common Settings Commands
//

CUSTOM_COMMAND_SIG(toggle_fullscreen){
    app->toggle_fullscreen(app);
}

CUSTOM_COMMAND_SIG(toggle_line_wrap){
    View_Summary view = app->get_active_view(app, AccessProtected);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessProtected);
    
    bool32 unwrapped = view.unwrapped_lines;
    
    app->view_set_setting(app, &view, ViewSetting_WrapLine, unwrapped);
    app->buffer_set_setting(app, &buffer, BufferSetting_WrapLine, unwrapped);
}

CUSTOM_COMMAND_SIG(toggle_show_whitespace){
    View_Summary view = app->get_active_view(app, AccessProtected);
    app->view_set_setting(app, &view, ViewSetting_ShowWhitespace, !view.show_whitespace);
}

CUSTOM_COMMAND_SIG(eol_dosify){
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    app->buffer_set_setting(app, &buffer, BufferSetting_Eol, true);
}

CUSTOM_COMMAND_SIG(eol_nixify){
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    app->buffer_set_setting(app, &buffer, BufferSetting_Eol, false);
}

CUSTOM_COMMAND_SIG(exit_4coder){
    app->send_exit_signal(app);
}


//
// "Full Search" Based Commands
//

#include "4coder_table.cpp"
#include "4coder_search.cpp"
#include "4coder_jump_parsing.cpp"

static void
generic_search_all_buffers(Application_Links *app, General_Memory *general, Partition *part,
                           uint32_t match_flags){
    
    Query_Bar string;
    char string_space[1024];
    string.prompt = make_lit_string("List Locations For: ");
    string.string = make_fixed_width_string(string_space);
    
    if (!query_user_string(app, &string)) return;
    if (string.string.size == 0) return;
    
    Search_Set set = {0};
    Search_Iter iter = {0};
    
    search_iter_init(general, &iter, string.string.size);
    copy_ss(&iter.word, string.string);
    
    int32_t buffer_count = app->get_buffer_count(app);
    search_set_init(general, &set, buffer_count);
    
    Search_Range *ranges = set.ranges;
    
    String search_name = make_lit_string("*search*");
    
    Buffer_Summary search_buffer = app->get_buffer_by_name(app, search_name.str, search_name.size,
                                                           AccessAll);
    if (!search_buffer.exists){
        search_buffer = app->create_buffer(app, search_name.str, search_name.size, BufferCreate_AlwaysNew);
        app->buffer_set_setting(app, &search_buffer, BufferSetting_Unimportant, true);
        app->buffer_set_setting(app, &search_buffer, BufferSetting_ReadOnly, true);
        app->buffer_set_setting(app, &search_buffer, BufferSetting_WrapLine, false);
    }
    else{
        app->buffer_replace_range(app, &search_buffer, 0, search_buffer.size, 0, 0);
    }
    
    {
        View_Summary view = app->get_active_view(app, AccessProtected);
        Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessProtected);
        
        int32_t j = 0;
        if (buffer.exists){
            if (buffer.buffer_id != search_buffer.buffer_id){
                ranges[0].type = SearchRange_FrontToBack;
                ranges[0].flags = match_flags;
                ranges[0].buffer = buffer.buffer_id;
                ranges[0].start = 0;
                ranges[0].size = buffer.size;
                j = 1;
            }
        }
        
        for (Buffer_Summary buffer_it = app->get_buffer_first(app, AccessAll);
             buffer_it.exists;
             app->get_buffer_next(app, &buffer_it, AccessAll)){
            if (buffer.buffer_id != buffer_it.buffer_id){
                if (search_buffer.buffer_id != buffer_it.buffer_id){
                    ranges[j].type = SearchRange_FrontToBack;
                    ranges[j].flags = match_flags;
                    ranges[j].buffer = buffer_it.buffer_id;
                    ranges[j].start = 0;
                    ranges[j].size = buffer_it.size;
                    ++j;
                }
            }
        }
        set.count = j;
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    Partition line_part = partition_sub_part(part, (4 << 10));
    char *str = (char*)partition_current(part);
    int32_t part_size = 0;
    int32_t size = 0;
    for (;;){
        Search_Match match = search_next_match(app, &set, &iter);
        if (match.found_match){
            Partial_Cursor word_pos = {0};
            if (app->buffer_compute_cursor(app, &match.buffer, seek_pos(match.start), &word_pos)){
                int32_t file_len = match.buffer.file_name_len;
                int32_t line_num_len = int_to_str_size(word_pos.line);
                int32_t column_num_len = int_to_str_size(word_pos.character);
                
                Temp_Memory line_temp = begin_temp_memory(&line_part);
                String line_str = {0};
                read_line(app, &line_part, &match.buffer, word_pos.line, &line_str);
                line_str = skip_chop_whitespace(line_str);
                
                int32_t str_len = file_len + 1 + line_num_len + 1 + column_num_len + 1 + 1 + line_str.size + 1;
                
                char *spare = push_array(part, char, str_len);
                
                if (spare == 0){
                    app->buffer_replace_range(app, &search_buffer,
                                              size, size, str, part_size);
                    size += part_size;
                    
                    end_temp_memory(temp);
                    temp = begin_temp_memory(part);
                    
                    part_size = 0;
                    spare = push_array(part, char, str_len);
                }
                
                part_size += str_len;
                
                String out_line = make_string_cap(spare, 0, str_len);
                append_ss(&out_line, make_string(match.buffer.file_name, file_len));
                append_s_char(&out_line, ':');
                append_int_to_str(&out_line, word_pos.line);
                append_s_char(&out_line, ':');
                append_int_to_str(&out_line, word_pos.character);
                append_s_char(&out_line, ':');
                append_s_char(&out_line, ' ');
                append_ss(&out_line, line_str);
                append_s_char(&out_line, '\n');
                
                end_temp_memory(line_temp);
            }
        }
        else{
            break;
        }
    }
    
    app->buffer_replace_range(app, &search_buffer, size, size, str, part_size);
    
    View_Summary view = app->get_active_view(app, AccessAll);
    app->view_set_buffer(app, &view, search_buffer.buffer_id, 0);
    
    lock_jump_buffer(search_name.str, search_name.size);
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(list_all_locations){
    generic_search_all_buffers(app, &global_general, &global_part, SearchFlag_MatchWholeWord);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations){
    generic_search_all_buffers(app, &global_general, &global_part, SearchFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(list_all_locations_case_insensitive){
    generic_search_all_buffers(app, &global_general, &global_part, SearchFlag_CaseInsensitive | SearchFlag_MatchWholeWord);
}

CUSTOM_COMMAND_SIG(list_all_substring_locations_case_insensitive){
    generic_search_all_buffers(app, &global_general, &global_part, SearchFlag_CaseInsensitive | SearchFlag_MatchSubstring);
}

struct Word_Complete_State{
    Search_Set set;
    Search_Iter iter;
    Table hits;
    String_Space str;
    int32_t word_start;
    int32_t word_end;
    int32_t initialized;
};

static Word_Complete_State complete_state = {0};

CUSTOM_COMMAND_SIG(word_complete){
    View_Summary view = app->get_active_view(app, AccessOpen);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, AccessOpen);
    
    // NOTE(allen): I just do this because this command is a lot of work
    // and there is no point in doing any of it if nothing will happen anyway.
    if (buffer.exists){
        int32_t do_init = false;
        
        if (view_paste_index[view.view_id].rewrite != RewriteWordComplete){
            do_init = true;
        }
        view_paste_index[view.view_id].next_rewrite = RewriteWordComplete;
        if (!complete_state.initialized){
            do_init = true;
        }
        
        int32_t word_end = 0;
        int32_t word_start = 0;
        int32_t cursor_pos = 0;
        int32_t size = 0;
        
        if (do_init){
            // NOTE(allen): Get the range where the
            // partial word is written.
            word_end = view.cursor.pos;
            word_start = word_end;
            cursor_pos = word_end - 1;
            
            char space[1024];
            Stream_Chunk chunk = {0};
            if (init_stream_chunk(&chunk, app, &buffer,
                                  cursor_pos, space, sizeof(space))){
                int32_t still_looping = true;
                do{
                    for (; cursor_pos >= chunk.start; --cursor_pos){
                        char c = chunk.data[cursor_pos];
                        if (char_is_alpha(c)){
                            word_start = cursor_pos;
                        }
                        else if (!char_is_numeric(c)){
                            goto double_break;
                        }
                    }
                    still_looping = backward_stream_chunk(&chunk);
                }while(still_looping);
            }
            double_break:;
            
            size = word_end - word_start;
            
            if (size == 0){
                complete_state.initialized = false;
                return;
            }
            
            // NOTE(allen): Initialize the search iterator
            // with the partial word.
            complete_state.initialized = true;
            search_iter_init(&global_general, &complete_state.iter, size);
            app->buffer_read_range(app, &buffer, word_start, word_end,
                                   complete_state.iter.word.str);
            complete_state.iter.word.size = size;
            
            // NOTE(allen): Initialize the set of ranges to be searched.
            int32_t buffer_count = app->get_buffer_count(app);
            search_set_init(&global_general, &complete_state.set, buffer_count);
            
            Search_Range *ranges = complete_state.set.ranges;
            ranges[0].type = SearchRange_Wave;
            ranges[0].flags = SearchFlag_MatchWordPrefix;
            ranges[0].buffer = buffer.buffer_id;
            ranges[0].start = 0;
            ranges[0].size = buffer.size;
            ranges[0].mid_start = word_start;
            ranges[0].mid_size = size;
            
            int32_t j = 1;
            for (Buffer_Summary buffer_it = app->get_buffer_first(app, AccessAll);
                 buffer_it.exists;
                 app->get_buffer_next(app, &buffer_it, AccessAll)){
                if (buffer.buffer_id != buffer_it.buffer_id){
                    ranges[j].type = SearchRange_FrontToBack;
                    ranges[j].flags = SearchFlag_MatchWordPrefix;
                    ranges[j].buffer = buffer_it.buffer_id;
                    ranges[j].start = 0;
                    ranges[j].size = buffer_it.size;
                    ++j;
                }
            }
            complete_state.set.count = j;
            
            // NOTE(allen): Initialize the search hit table.
            search_hits_init(&global_general, &complete_state.hits, &complete_state.str,
                             100, (4 << 10));
            search_hit_add(&global_general, &complete_state.hits, &complete_state.str,
                           complete_state.iter.word.str,
                           complete_state.iter.word.size);
            
            complete_state.word_start = word_start;
            complete_state.word_end = word_end;
        }
        else{
            word_start = complete_state.word_start;
            word_end = complete_state.word_end;
            size = complete_state.iter.word.size;
        }
        
        // NOTE(allen): Iterate through matches.
        if (size > 0){
            for (;;){
                int32_t match_size = 0;
                Search_Match match =
                    search_next_match(app, &complete_state.set,
                                      &complete_state.iter);
                
                if (match.found_match){
                    match_size = match.end - match.start;
                    Temp_Memory temp = begin_temp_memory(&global_part);
                    char *spare = push_array(&global_part, char, match_size);
                    
                    app->buffer_read_range(app, &match.buffer,
                                           match.start, match.end, spare);
                    
                    if (search_hit_add(&global_general, &complete_state.hits, &complete_state.str,
                                       spare, match_size)){
                        app->buffer_replace_range(app, &buffer, word_start, word_end,
                                                  spare, match_size);
                        app->view_set_cursor(app, &view,
                                             seek_pos(word_start + match_size),
                                             true);
                        
                        complete_state.word_end = word_start + match_size;
                        complete_state.set.ranges[0].mid_size = match_size;
                        end_temp_memory(temp);
                        break;
                    }
                    end_temp_memory(temp);
                }
                else{
                    complete_state.iter.pos = 0;
                    complete_state.iter.i = 0;
                    
                    search_hits_init(&global_general, &complete_state.hits, &complete_state.str,
                                     100, (4 << 10));
                    search_hit_add(&global_general, &complete_state.hits, &complete_state.str,
                                   complete_state.iter.word.str,
                                   complete_state.iter.word.size);
                    
                    match_size = complete_state.iter.word.size;
                    char *str = complete_state.iter.word.str;
                    app->buffer_replace_range(app, &buffer, word_start, word_end,
                                              str, match_size);
                    app->view_set_cursor(app, &view,
                                         seek_pos(word_start + match_size),
                                         true);
                    
                    complete_state.word_end = word_start + match_size;
                    complete_state.set.ranges[0].mid_size = match_size;
                    break;
                }
            }
        }
    }
}


//
// Default Building Stuff
//

// NOTE(allen|a4.0.9): This is provided to establish a default method of getting
// a "build directory".  This function tries to setup the build directory in the
// directory of the given buffer, if it cannot get that information it get's the
// 4coder hot directory.
//
//  There is no requirement that a custom build system in 4coder actually use the
// directory given by this function.
enum Get_Build_Directory_Result{
    BuildDir_None,
    BuildDir_AtFile,
    BuildDir_AtHot
};

static int32_t
get_build_directory(Application_Links *app, Buffer_Summary *buffer, String *dir_out){
    int32_t result = BuildDir_None;
    
    if (buffer && buffer->file_name){
        if (!match_cc(buffer->file_name, buffer->buffer_name)){
            String dir = make_string_cap(buffer->file_name,
                                         buffer->file_name_len,
                                         buffer->file_name_len+1);
            remove_last_folder(&dir);
            append_ss(dir_out, dir);
            result = BuildDir_AtFile;
        }
    }
    
    if (!result){
        int32_t len = app->directory_get_hot(app, dir_out->str,
                                         dir_out->memory_size - dir_out->size);
        if (len + dir_out->size < dir_out->memory_size){
            dir_out->size += len;
            result = BuildDir_AtHot;
        }
    }
    
    return(result);
}

// TODO(allen): Better names for the "standard build search" family.
static int32_t
standard_build_search(Application_Links *app,
                      View_Summary *view,
                      Buffer_Summary *active_buffer,
                      String *dir, String *command,
                      int32_t perform_backup,
                      int32_t use_path_in_command,
                      String filename,
                      String commandname){
    int32_t result = false;
    
    for(;;){
        int32_t old_size = dir->size;
        append_ss(dir, filename);
        
        if (app->file_exists(app, dir->str, dir->size)){
            dir->size = old_size;
            
            if (use_path_in_command){
                append_s_char(command, '"');
                append_ss(command, *dir);
                append_ss(command, commandname);
                append_s_char(command, '"');
            }
            else{
                append_ss(command, commandname);
            }
            
            char space[512];
            String message = make_fixed_width_string(space);
            append_ss(&message, make_lit_string("Building with: "));
            append_ss(&message, *command);
            append_s_char(&message, '\n');
            app->print_message(app, message.str, message.size);
            
            
            app->exec_system_command(app, view,
                                     buffer_identifier(literal("*compilation*")),
                                     dir->str, dir->size,
                                     command->str, command->size,
                                     CLI_OverlapWithConflict);
            result = true;
            break;
        }
        dir->size = old_size;
        
        if (app->directory_cd(app, dir->str, &dir->size, dir->memory_size, literal("..")) == 0){
            if (perform_backup){
                dir->size = app->directory_get_hot(app, dir->str, dir->memory_size);
                char backup_space[256];
                String backup_command = make_fixed_width_string(backup_space);
                append_ss(&backup_command, make_lit_string("echo could not find "));
                append_ss(&backup_command, filename);
                app->exec_system_command(app, view,
                                         buffer_identifier(literal("*compilation*")),
                                         dir->str, dir->size,
                                         backup_command.str, backup_command.size,
                                         CLI_OverlapWithConflict);
            }
            break;
        }
    }
    
    return(result);
}

#if defined(_WIN32)

// NOTE(allen): Build search rule for windows.
static int32_t
execute_standard_build_search(Application_Links *app, View_Summary *view,
                              Buffer_Summary *active_buffer,
                              String *dir, String *command, int32_t perform_backup){
    int32_t result = standard_build_search(app, view,
                                           active_buffer,
                                           dir, command, perform_backup, true,
                                           make_lit_string("build.bat"),
                                           make_lit_string("build"));
    return(result);
}

#elif defined(__linux__)

// NOTE(allen): Build search rule for linux.
static int32_t
execute_standard_build_search(Application_Links *app, View_Summary *view,
                              Buffer_Summary *active_buffer,
                              String *dir, String *command, int32_t perform_backup){
    
    char dir_space[512];
    String dir_copy = make_fixed_width_string(dir_space);
    copy(&dir_copy, *dir);
    
    int32_t result = standard_build_search(app, view,
                                       active_buffer,
                                       dir, command, false, true,
                                       make_lit_string("build.sh"),
                                       make_lit_string("build.sh"));
    
    if (!result){
        result = standard_build_search(app, view,
                                       active_buffer,
                                       &dir_copy, command, perform_backup, false,
                                       make_lit_string("Makefile"),
                                       make_lit_string("make"));
    }
    
    return(result);
}

#else
# error No build search rule for this platform.
#endif


// NOTE(allen): This searches first using the active file's directory,
// then if no build script is found, it searches from 4coders hot directory.
static void
execute_standard_build(Application_Links *app, View_Summary *view,
                       Buffer_Summary *active_buffer){
    char dir_space[512];
    String dir = make_fixed_width_string(dir_space);
    
    char command_str_space[512];
    String command = make_fixed_width_string(command_str_space);
    
    int32_t build_dir_type = get_build_directory(app, active_buffer, &dir);
    
    if (build_dir_type == BuildDir_AtFile){
        if (!execute_standard_build_search(app, view, active_buffer,
                                           &dir, &command, false)){
            dir.size = 0;
            command.size = 0;
            build_dir_type = get_build_directory(app, 0, &dir);
        }
    }
    
    if (build_dir_type == BuildDir_AtHot){
        execute_standard_build_search(app, view, active_buffer,
                                      &dir, &command, true);
    }
}

CUSTOM_COMMAND_SIG(build_search){
    uint32_t access = AccessAll;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    execute_standard_build(app, &view, &buffer);
    prev_location = null_location;
    lock_jump_buffer(literal("*compilation*"));
}

#define GET_COMP_BUFFER(app) app->get_buffer_by_name(app, literal("*compilation*"), AccessAll)

static View_Summary
get_or_open_build_panel(Application_Links *app){
    View_Summary view = {0};
    
    Buffer_Summary buffer = GET_COMP_BUFFER(app);
    if (buffer.exists){
        view = get_first_view_with_buffer(app, buffer.buffer_id);
    }
    if (!view.exists){
        view = open_special_note_view(app);
    }
    
    return(view);
}

static void
set_fancy_compilation_buffer_font(Application_Links *app){
    Buffer_Summary comp_buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    app->buffer_set_font(app, &comp_buffer, literal("Inconsolata"));
}                                                       
                                                        
CUSTOM_COMMAND_SIG(build_in_build_panel){               
    uint32_t access = AccessAll;                        
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
                                                        
    View_Summary build_view = get_or_open_build_panel(app);
                                                        
    execute_standard_build(app, &build_view, &buffer);
    set_fancy_compilation_buffer_font(app);
    
    prev_location = null_location;
    lock_jump_buffer(literal("*compilation*"));
}

CUSTOM_COMMAND_SIG(close_build_panel){
    close_special_note_view(app);
}

CUSTOM_COMMAND_SIG(change_to_build_panel){
    View_Summary view = open_special_note_view(app, false);
    
    if (!view.exists){
        Buffer_Summary buffer = GET_COMP_BUFFER(app);
        if (buffer.exists){
            view = open_special_note_view(app);
            app->view_set_buffer(app, &view, buffer.buffer_id, 0);
        }
    }
    
    if (view.exists){
        app->set_active_view(app, &view);
    }
}

//
// Other
//

CUSTOM_COMMAND_SIG(execute_arbitrary_command){
    // NOTE(allen): This isn't a super powerful version of this command, I will expand
    // upon it so that it has all the cmdid_* commands by default.  However, with this
    // as an example you have everything you need to make it work already. You could
    // even use app->memory to create a hash table in the start hook.
    Query_Bar bar;
    char space[1024];
    bar.prompt = make_lit_string("Command: ");
    bar.string = make_fixed_width_string(space);
    
    if (!query_user_string(app, &bar)) return;
    
    // NOTE(allen): Here I chose to end this query bar because when I call another
    // command it might ALSO have query bars and I don't want this one hanging
    // around at that point.  Since the bar exists on my stack the result of the query
    // is still available in bar.string though.
    app->end_query_bar(app, &bar, 0);
    
    if (match_ss(bar.string, make_lit_string("open all code"))){
        exec_command(app, open_all_code);
    }
    else if(match_ss(bar.string, make_lit_string("close all code"))){
        exec_command(app, close_all_code);
    }
    else if (match_ss(bar.string, make_lit_string("open menu"))){
        exec_command(app, cmdid_open_menu);
    }
    else if (match_ss(bar.string, make_lit_string("dos lines"))){
        exec_command(app, eol_dosify);
    }
    else if (match_ss(bar.string, make_lit_string("nix lines"))){
        exec_command(app, eol_nixify);
    }
    else{
        app->print_message(app, literal("unrecognized command\n"));
    }
}

// NOTE(allen|a4): scroll rule information
//
// The parameters:
// target_x, target_y
//  This is where the view would like to be for the purpose of
// following the cursor, doing mouse wheel work, etc.
//
// scroll_x, scroll_y
//  These are pointers to where the scrolling actually is. If you bind
// the scroll rule it is you have to update these in some way to move
// the actual location of the scrolling.
//
// view_id
//  This corresponds to which view is computing it's new scrolling position.
// This id DOES correspond to the views that View_Summary contains.
// This will always be between 1 and 16 (0 is a null id).
// See below for an example of having state that carries across scroll udpates.
//
// is_new_target
//  If the target of the view is different from the last target in either x or y
// this is true, otherwise it is false.
//
// The return:
//  Should be true if and only if scroll_x or scroll_y are changed.
//
// Don't try to use the app pointer in a scroll rule, you're asking for trouble.
//
// If you don't bind scroll_rule, nothing bad will happen, yo will get default
// 4coder scrolling behavior.
//

struct Scroll_Velocity{
    float x, y;
};

Scroll_Velocity scroll_velocity_[16] = {0};
Scroll_Velocity *scroll_velocity = scroll_velocity_ - 1;

static int32_t
smooth_camera_step(float target, float *current, float *vel, float S, float T){
    int32_t result = 0;
    float curr = *current;
    float v = *vel;
    if (curr != target){
        if (curr > target - .1f && curr < target + .1f){
            curr = target;
            v = 1.f;
        }
        else{
            float L = curr + T*(target - curr);
            
            int32_t sign = (target > curr) - (target < curr);
            float V = curr + sign*v;
            
            if (sign > 0) curr = (L<V)?(L):(V);
            else curr = (L>V)?(L):(V);
            
            if (curr == V){
                v *= S;
            }
        }
        
        *current = curr;
        *vel = v;
        result = 1;
    }
    return(result);
}

SCROLL_RULE_SIG(smooth_scroll_rule){
    Scroll_Velocity *velocity = scroll_velocity + view_id;
    int32_t result = 0;
    if (velocity->x == 0.f){
        velocity->x = 1.f;
        velocity->y = 1.f;
    }

    if (smooth_camera_step(target_y, scroll_y, &velocity->y, 80.f, 1.f/2.f)){
        result = 1;
    }
    if (smooth_camera_step(target_x, scroll_x, &velocity->x, 80.f, 1.f/2.f)){
        result = 1;
    }

    return(result);
}

// NOTE(allen|a4.0.9): All command calls can now go through this hook
// If this hook is not implemented a default behavior of calling the
// command is used.  It is important to note that paste_next does not
// work without this hook.
// NOTE(allen|a4.0.10): As of this version the word_complete command
// also relies on this particular command caller hook.
COMMAND_CALLER_HOOK(default_command_caller){
    View_Summary view = app->get_active_view(app, AccessAll);
    
    view_paste_index[view.view_id].next_rewrite = false;
    
    exec_command(app, cmd);
    
    view_paste_index[view.view_id].rewrite = 
        view_paste_index[view.view_id].next_rewrite;
    
    return(0);
}

#endif

