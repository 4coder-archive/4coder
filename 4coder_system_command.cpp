/*
4coder_system_command.cpp - Commands for executing arbitrary system command line instructions.

TYPE: 'drop-in-command-pack'
*/

// TOP

#if !defined(FCODER_SYSTEM_COMMAND_CPP)
#define FCODER_SYSTEM_COMMAND_CPP

#include "4coder_default_framework.h"

CUSTOM_COMMAND_SIG(execute_previous_cli){
    String out_buffer = make_string_slowly(out_buffer_space);
    String cmd = make_string_slowly(command_space);
    String hot_directory = make_string_slowly(hot_directory_space);
    
    if (out_buffer.size > 0 && cmd.size > 0 && hot_directory.size > 0){
        uint32_t access = AccessAll;
        View_Summary view = get_active_view(app, access);
        
        exec_system_command(app, &view, buffer_identifier(out_buffer.str, out_buffer.size), hot_directory.str, hot_directory.size, cmd.str, cmd.size, CLI_OverlapWithConflict | CLI_CursorAtEnd);
        lock_jump_buffer(out_buffer.str, out_buffer.size);
    }
}

CUSTOM_COMMAND_SIG(execute_any_cli){
    Query_Bar bar_out = {0};
    Query_Bar bar_cmd = {0};
    
    bar_out.prompt = make_lit_string("Output Buffer: ");
    bar_out.string = make_fixed_width_string(out_buffer_space);
    if (!query_user_string(app, &bar_out)) return;
    
    bar_cmd.prompt = make_lit_string("Command: ");
    bar_cmd.string = make_fixed_width_string(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    
    directory_get_hot(app, hot_directory_space, sizeof(hot_directory_space));
    
    execute_previous_cli(app);
}

#endif

// BOTTOM

