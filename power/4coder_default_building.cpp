
#ifndef FCODER_DEFAULT_BUILDING
#define FCODER_DEFAULT_BUILDING

#include "4coder_custom.h"

#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4coder_helper.h"


//
// Basic Build Behavior
//

struct Prev_Jump{
    int buffer_id;
    int line;
};

static Prev_Jump prev_location = {0};

CUSTOM_COMMAND_SIG(build_in_build_panel){
    Buffer_Summary buffer =
        app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    View_Summary build_view = {0};
    
    View_Summary original_view = app->get_active_view(app, AccessAll);
    Buffer_Summary original_buffer =
        app->get_buffer(app, original_view.buffer_id, AccessAll);
    
    if (buffer.exists){
        build_view = get_first_view_with_buffer(app, buffer.buffer_id);
    }
    
    if (!build_view.exists){
        exec_command(app, open_panel_hsplit);
        exec_command(app, hide_scrollbar);
        build_view = app->get_active_view(app, AccessAll);
        app->view_set_split_proportion(app, &build_view, .2f);
        app->set_active_view(app, &original_view);
    }
    
    execute_standard_build(app, &build_view, &original_buffer);
    
    
    prev_location = {0};
}

// TODO(allen): This is a bit nasty.  I want a system for picking
// the most advanced and correct version of a command to bind to a
// name based on which files are included.
#ifndef  BUILD_SEARCH
# define BUILD_SEARCH 2
#elif BUILD_SEARCH <= 2
# undef  BUILD_SEARCH
# define BUILD_SEARCH 2
#endif

#if BUILD_SEARCH <= 2
# ifdef build_search
#  undef build_search
# endif
# define build_search build_in_build_panel
#endif

#define GET_COMP_BUFFER() app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);

CUSTOM_COMMAND_SIG(close_build_panel){
    Buffer_Summary buffer = GET_COMP_BUFFER();
    View_Summary build_view = get_first_view_with_buffer(app, buffer.buffer_id);
    
    if (build_view.exists){
        View_Summary original_view = app->get_active_view(app, AccessAll);
        
        app->set_active_view(app, &build_view);
        exec_command(app, close_panel);
        app->set_active_view(app, &original_view);
    }
}

CUSTOM_COMMAND_SIG(change_to_build_panel){
    Buffer_Summary buffer = GET_COMP_BUFFER();
    
    if (buffer.exists){
        View_Summary build_view = get_first_view_with_buffer(app, buffer.buffer_id);
        
        app->set_active_view(app, &build_view);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel_build){
    Buffer_Summary buffer = GET_COMP_BUFFER();
    
    if (buffer.exists){
        View_Summary build_view = get_first_view_with_buffer(app, buffer.buffer_id);
        
        View_Summary view = app->get_active_view(app, AccessAll);
        int prev_view_id = view.view_id;
        
        exec_command(app, change_active_panel_regular);
        view = app->get_active_view(app, AccessAll);
        
        for (;(view.view_id != prev_view_id &&
               build_view.view_id == view.view_id);){
            prev_view_id = view.view_id;
            exec_command(app, change_active_panel_regular);
            view = app->get_active_view(app, AccessAll);
        }
    }
    else{
        exec_command(app, change_active_panel_regular);
    }
}

// TODO(allen): This is a bit nasty.  I want a system for picking
// the most advanced and correct version of a command to bind to a
// name based on which files are included.
#ifndef  CHANGE_ACTIVE_PANEL
# define CHANGE_ACTIVE_PANEL 2
#elif CHANGE_ACTIVE_PANEL <= 2
# undef  CHANGE_ACTIVE_PANEL
# define CHANGE_ACTIVE_PANEL 2
#endif

#if CHANGE_ACTIVE_PANEL <= 2
# ifdef change_active_panel
#  undef change_active_panel
# endif
# define change_active_panel change_active_panel_build
#endif

CUSTOM_COMMAND_SIG(open_file_in_quotes_build){
    char file_name_[256];
    String file_name = make_fixed_width_string(file_name_);
    
    if (file_name_in_quotes(app, &file_name)){
        exec_command(app, change_active_panel_build);
        View_Summary view = app->get_active_view(app, AccessAll);
        view_open_file(app, &view, expand_str(file_name), false);
    }
}

// TODO(allen): This is a bit nasty.  I want a system for picking
// the most advanced and correct version of a command to bind to a
// name based on which files are included.
#ifndef  OPEN_FILE_IN_QUOTES
# define OPEN_FILE_IN_QUOTES 2
#elif OPEN_FILE_IN_QUOTES <= 2
# undef  OPEN_FILE_IN_QUOTES
# define OPEN_FILE_IN_QUOTES 2
#endif

#if OPEN_FILE_IN_QUOTES <= 2
# ifdef open_file_in_quotes
#  undef open_file_in_quotes
# endif
# define open_file_in_quotes open_file_in_quotes_build
#endif

CUSTOM_COMMAND_SIG(open_in_other_build){
    exec_command(app, change_active_panel_build);
    exec_command(app, cmdid_interactive_open);
}

// TODO(allen): This is a bit nasty.  I want a system for picking
// the most advanced and correct version of a command to bind to a
// name based on which files are included.
#ifndef  OPEN_IN_OTHER
# define OPEN_IN_OTHER 1
#elif OPEN_IN_OTHER <= 1
# undef  OPEN_IN_OTHER
# define OPEN_IN_OTHER 1
#endif

#if OPEN_IN_OTHER <= 1
# ifdef open_in_other
#  undef open_in_other
# endif
# define open_in_other open_in_other_build
#endif

//
// Jump to Error
//

static int
read_line(Application_Links *app,
          View_Summary *view,
          int line,
          String *str){
    
    Full_Cursor begin = {0};
    Full_Cursor end = {0};
    
    int success = false;
    
    if (app->view_compute_cursor(app, view,
                                 seek_line_char(line, 1), &begin)){
        if (app->view_compute_cursor(app, view,
                                     seek_line_char(line, 65536), &end)){
            
            Buffer_Summary buffer = app->get_buffer(app, view->buffer_id, AccessAll);
            if (begin.line == line){
                if (0 <= begin.pos && begin.pos <= end.pos && end.pos <= buffer.size){
                    int size = (end.pos - begin.pos);
                    if (size <= str->memory_size){
                        success = true;
                        app->buffer_read_range(app, &buffer, begin.pos, end.pos, str->str);
                        str->size = size;
                    }
                }
            }
            
        }
    }
    
    return(success);
}

struct Jump_Location{
    String file;
    int line;
};

static void
jump_to_location(Application_Links *app, View_Summary *view, Jump_Location *l){
    view_open_file(app, view, l->file.str, l->file.size, false);
    app->view_set_cursor(app, view, seek_line_char(l->line, 1), true);
}

static int
msvc_parse_error(String line, Jump_Location *location,
                 int skip_sub_errors, int *colon_char){
    int result = false;
    
    int colon_pos = find(line, 0, ')');
    colon_pos = find(line, colon_pos, ':');
    if (colon_pos < line.size){
        String location_str = substr(line, 0, colon_pos);
        
        if (!(skip_sub_errors && location_str.str[0] == ' ')){
            location_str = skip_chop_whitespace(location_str);
            
            int paren_pos = find(location_str, 0, '(');
            if (paren_pos < location_str.size){
                String file = substr(location_str, 0, paren_pos);
                file = skip_chop_whitespace(file);
                
                int close_pos = find(location_str, 0, ')') + 1;
                if (close_pos == location_str.size && file.size > 0){
                    String line_number = substr(location_str,
                                                paren_pos+1,
                                                close_pos-paren_pos-2);
                    line_number = skip_chop_whitespace(line_number);
                    
                    if (line_number.size > 0){
                        copy(&location->file, file);
                        location->line = str_to_int(line_number);
                        *colon_char = colon_pos;
                        result = true;
                    }
                }
            }
        }
    }
    
    return(result);
}

static int
msvc_next_error(Application_Links *app,
                View_Summary *comp_out, int *start_line,
                void *memory, int memory_size,
                Jump_Location *location,
                int direction,
                int skip_sub_errors,
                int *colon_char){
    
    int result = false;
    int line = *start_line + direction;
    String line_str = make_string(memory, 0, memory_size);
    for (;;){
        if (read_line(app, comp_out, line, &line_str)){
            if (msvc_parse_error(line_str, location, skip_sub_errors, colon_char)){
                result = true;
                break;
            }
            line += direction;
        }
        else{
            break;
        }
    }
    
    if (line < 0){
        line = 0;
    }
    
    *start_line = line;
    
    return(result);
}

static int
msvc_goto_error(Application_Links *app, int direction, int skip_sub_errors, Jump_Location *loc){
    int result = false;
    View_Summary active_view = app->get_active_view(app, AccessAll);
    
    Jump_Location location = {0};
    Buffer_Summary buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    if (buffer.exists){
        View_Summary view = get_first_view_with_buffer(app, buffer.buffer_id);
        int line = view.cursor.line;
        
        int ms = app->memory_size/2;
        location.file = make_string(app->memory, 0, ms);
        void *m = (char*)app->memory + ms;
        
        int colon_char = 0;
        if (msvc_next_error(app, &view, &line, m, ms, &location,
                            skip_sub_errors, direction, &colon_char)){
            jump_to_location(app, &active_view, &location);
            app->view_set_cursor(app, &view, seek_line_char(line, colon_char+1), true);
            result = true;
            if (loc){
                *loc = location;
            }
        }
    }
    return(result);
}

static Prev_Jump
jump_location_store(Application_Links *app, Jump_Location loc){
    Prev_Jump result = {0};
    Buffer_Summary buffer =
        app->get_buffer_by_name(app, loc.file.str, loc.file.size, AccessAll);
    
    if (buffer.exists){
        result.buffer_id = buffer.buffer_id;
        result.line = loc.line;
    }
    
    return(result);
}

static int
skip_this_jump(Prev_Jump prev, Prev_Jump jump){
    int result = false;
    if (prev.buffer_id != 0 && prev.buffer_id == jump.buffer_id &&
        prev.line == jump.line){
        result = true;
    }
    return(result);
}

CUSTOM_COMMAND_SIG(msvc_goto_next_error){
    Jump_Location location = {0};
    Prev_Jump jump = {0};
    
    do{
        if (msvc_goto_error(app, true, 1, &location)){
            jump = jump_location_store(app, location);
        }
        else{
            jump.buffer_id = 0;
        }
    }while(skip_this_jump(prev_location, jump));
    prev_location = jump;
}

CUSTOM_COMMAND_SIG(msvc_goto_prev_error){
    Jump_Location location = {0};
    Prev_Jump jump = {0};
    
    do{
        if (msvc_goto_error(app, true, -1, &location)){
            jump = jump_location_store(app, location);
        }
        else{
            jump.buffer_id = 0;
        }
    }while(skip_this_jump(prev_location, jump));
    prev_location = jump;
}

CUSTOM_COMMAND_SIG(msvc_goto_first_error){
    View_Summary active_view = app->get_active_view(app, AccessAll);
    app->view_set_cursor(app, &active_view, seek_pos(0), true);
    
    Jump_Location location;
    msvc_goto_error(app, true, 1, &location);
    prev_location = jump_location_store(app, location);
}


#endif

