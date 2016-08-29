
#ifndef FCODER_DEFAULT_BUILDING
#define FCODER_DEFAULT_BUILDING

#include "4coder_custom.h"

#define FSTRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4coder_helper.h"
#include "4coder_jump_parsing.cpp"

//
// Basic Build Behavior
//

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
    
    buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    app->buffer_set_font(app, &buffer, literal("Inconsolata"));
    
    prev_location = null_location;
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
        int32_t prev_view_id = view.view_id;
        
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

#endif

