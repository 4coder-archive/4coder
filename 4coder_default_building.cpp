
#include "4coder_custom.h"

#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4coder_helper.h"

CUSTOM_COMMAND_SIG(build_search){
    // NOTE(allen|a3.3): An example of traversing the filesystem through parent
    // directories looking for a file, in this case a batch file to execute.
    //
    //
    // Step 1: Grab all of the user memory (or, you know, less if you've got better
    //     thing to do with some of it).  Make a string and store the hot directory in it.
    //
    // Step 2: app->file_exists queries the file system to see if "<somedir>/build.bat" exists.
    // If it does exist several parameters are pushed and cmdid_command_line is executed:
    //   - par_flags: flags for specifiying behaviors
    //        CLI_OverlapWithConflict - (on by default) if another CLI is still using the output buffer
    //        that process is detached from the buffer and this process executes outputing to the buffer
    //        CLI_AlwaysBindToView - if set, the current view always switches to the output buffer
    //        even if the output buffer is open in another view
    //
    //   - par_name: the name of the buffer to fill with the output from the process
    //   - par_buffer_id: the buffer_id of the buffer to to fill with output
    //     If both are set buffer_id is used and the name is ignored.
    //     If neither is set the command runs without storing output anywhere.
    //
    //   - par_cli_path: sets the path from which the command is executed
    //     If this parameter is unset the command runs from the hot directory.
    //
    //   - par_cli_command: sets the actual command to be executed, this can be almost any
    //     command that you could execute through a command line interface.
    //     If this parameter is unset the command get's it's command from the range between
    //     the mark and cursor.
    // 
    // Step 3: If the batch file did not exist change the dir string to the parent directory using
    // app->directory_cd. The cd function can also be used to navigate to subdirectories.
    // It returns true if it can actually move in the specified direction, and false otherwise.
    // 
    // This doesn't actually change the hot directory of 4coder, it's only effect is to
    // modify the string you passed in to reflect the change in directory if that change was possible.
    
    int old_size;
    int size = app->memory_size/2;
    
    unsigned int access = AccessAll;
    View_Summary view = app->get_active_view(app, access);
    
    String dir = make_string(app->memory, 0, size);
    dir.size = app->directory_get_hot(app, dir.str, dir.memory_size);
    
    String command = make_string((char*)app->memory + size, 0, size);
    
    for(;;){
        old_size = dir.size;
        append(&dir, "build.bat");
        
        if (app->file_exists(app, dir.str, dir.size)){
            dir.size = old_size;
            append(&command, '"');
            append(&command, dir);
            append(&command, "build\"");
            
            app->exec_system_command(app, &view,
                                     buffer_identifier(literal("*compilation*")),
                                     dir.str, dir.size,
                                     command.str, command.size,
                                     CLI_OverlapWithConflict);
            
            break;
        }
        dir.size = old_size;
        
        if (app->directory_cd(app, dir.str, &dir.size, dir.memory_size, literal("..")) == 0){
            dir.size = app->directory_get_hot(app, dir.str, dir.memory_size);
            command = make_lit_string("echo couldn't find build.bat");
            app->exec_system_command(app, &view,
                                     buffer_identifier(literal("*compilation*")),
                                     dir.str, dir.size,
                                     command.str, command.size,
                                     CLI_OverlapWithConflict);
            break;
        }
    }
}

CUSTOM_COMMAND_SIG(build_in_build_panel){
    Buffer_Summary buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    View_Summary build_view = {0};
    View_Summary original_view = app->get_active_view(app, AccessAll);
    
    if (buffer.exists){
        build_view = get_first_view_with_buffer(app, buffer.buffer_id);
    }
    
    if (!build_view.exists){
        exec_command(app, cmdid_open_panel_hsplit);
        build_view = app->get_active_view(app, AccessAll);
    }
    
    app->set_active_view(app, &build_view);
    exec_command(app, build_search);
    app->set_active_view(app, &original_view);
}

CUSTOM_COMMAND_SIG(close_build_panel){
    Buffer_Summary buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    
    if (buffer.exists){
        View_Summary build_view = get_first_view_with_buffer(app, buffer.buffer_id);
        View_Summary original_view = app->get_active_view(app, AccessAll);
        
        app->set_active_view(app, &build_view);
        exec_command(app, cmdid_close_panel);
        app->set_active_view(app, &original_view);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel_skip_build){
    Buffer_Summary buffer = app->get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    
    if (buffer.exists){
        View_Summary build_view = get_first_view_with_buffer(app, buffer.buffer_id);
        
        View_Summary view = app->get_active_view(app, AccessAll);
        int prev_view_id = view.view_id;
        
        exec_command(app, cmdid_change_active_panel);
        view = app->get_active_view(app, AccessAll);
        
        for (;(view.view_id != prev_view_id &&
               build_view.view_id == view.view_id);){
            prev_view_id = view.view_id;
            exec_command(app, cmdid_change_active_panel);
            view = app->get_active_view(app, AccessAll);
        }
    }
}

CUSTOM_COMMAND_SIG(open_file_in_quotes_build){
    char file_name_[256];
    String file_name = make_fixed_width_string(file_name_);
    
    if (file_name_in_quotes(app, &file_name)){
        exec_command(app, change_active_panel_skip_build);
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
    exec_command(app, change_active_panel_skip_build);
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


