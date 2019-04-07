/*
4coder_system_command.cpp - Commands for executing arbitrary system command line instructions.
*/

// TOP

CUSTOM_COMMAND_SIG(execute_previous_cli)
CUSTOM_DOC("If the command execute_any_cli has already been used, this will execute a CLI reusing the most recent buffer name and command.")
{
    String out_buffer = make_string_slowly(out_buffer_space);
    String cmd = make_string_slowly(command_space);
    String hot_directory = make_string_slowly(hot_directory_space);
    
    if (out_buffer.size > 0 && cmd.size > 0 && hot_directory.size > 0){
        View_ID view = 0;
        get_active_view(app, AccessAll, &view);
        exec_system_command(app, view, buffer_identifier(out_buffer.str, out_buffer.size), hot_directory.str, hot_directory.size, cmd.str, cmd.size, CLI_OverlapWithConflict|CLI_CursorAtEnd|CLI_SendEndSignal);
        lock_jump_buffer(out_buffer);
    }
}

CUSTOM_COMMAND_SIG(execute_any_cli)
CUSTOM_DOC("Queries for an output buffer name and system command, runs the system command as a CLI and prints the output to the specified buffer."){
    Query_Bar bar_out = {};
    Query_Bar bar_cmd = {};
    
    bar_out.prompt = make_lit_string("Output Buffer: ");
    bar_out.string = make_fixed_width_string(out_buffer_space);
    if (!query_user_string(app, &bar_out)) return;
    
    bar_cmd.prompt = make_lit_string("Command: ");
    bar_cmd.string = make_fixed_width_string(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    
    directory_get_hot(app, hot_directory_space, sizeof(hot_directory_space));
    
    execute_previous_cli(app);
}

// BOTTOM

