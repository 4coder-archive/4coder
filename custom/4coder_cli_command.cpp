/*
4coder_system_command.cpp - Commands for executing arbitrary system command line instructions.
*/

// TOP

CUSTOM_COMMAND_SIG(execute_previous_cli)
CUSTOM_DOC("If the command execute_any_cli has already been used, this will execute a CLI reusing the most recent buffer name and command.")
{
    String_Const_u8 out_buffer = SCu8(out_buffer_space);
    String_Const_u8 cmd = SCu8(command_space);
    String_Const_u8 hot_directory = SCu8(hot_directory_space);
    
    if (out_buffer.size > 0 && cmd.size > 0 && hot_directory.size > 0){
        View_ID view = get_active_view(app, Access_Always);
        Buffer_Identifier id = buffer_identifier(out_buffer);
        exec_system_command(app, view, id, hot_directory, cmd, CLI_OverlapWithConflict|CLI_CursorAtEnd|CLI_SendEndSignal);
        lock_jump_buffer(app, out_buffer);
    }
}

CUSTOM_COMMAND_SIG(execute_any_cli)
CUSTOM_DOC("Queries for an output buffer name and system command, runs the system command as a CLI and prints the output to the specified buffer."){
    Scratch_Block scratch(app);
    Query_Bar_Group group(app);
    
    Query_Bar bar_out = {};
    bar_out.prompt = string_u8_litexpr("Output Buffer: ");
    bar_out.string = SCu8(out_buffer_space, (u64)0);
    bar_out.string_capacity = sizeof(out_buffer_space);
    if (!query_user_string(app, &bar_out)) return;
    bar_out.string.size = clamp_top(bar_out.string.size, sizeof(out_buffer_space) - 1);
    out_buffer_space[bar_out.string.size] = 0;
    
    Query_Bar bar_cmd = {};
    bar_cmd.prompt = string_u8_litexpr("Command: ");
    bar_cmd.string = SCu8(command_space, (u64)0);
    bar_cmd.string_capacity = sizeof(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    bar_cmd.string.size = clamp_top(bar_cmd.string.size, sizeof(command_space) - 1);
    command_space[bar_cmd.string.size] = 0;
    
    String_Const_u8 hot = push_hot_directory(app, scratch);
    {
        u64 size = clamp_top(hot.size, sizeof(hot_directory_space));
        block_copy(hot_directory_space, hot.str, size);
        hot_directory_space[hot.size] = 0;
    }
    
    execute_previous_cli(app);
}

// BOTTOM

