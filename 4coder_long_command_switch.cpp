/*
4coder_long_command_switch.cpp - Implementation of a command for executing uncommon commands 
by inputting key words.
*/

// TOP

CUSTOM_COMMAND_SIG(execute_arbitrary_command)
CUSTOM_DOC("Execute a 'long form' command.")
{
    Query_Bar bar = {0};
    char space[1024];
    bar.prompt = make_lit_string("Command: ");
    bar.string = make_fixed_width_string(space);
    if (!query_user_string(app, &bar)) return;
    end_query_bar(app, &bar, 0);
    
    if (match(bar.string, make_lit_string("toggle fullscreen"))){
        toggle_fullscreen(app);
    }
    else if (match(bar.string, make_lit_string("load project"))){
        load_project(app);
    }
    else if (match(bar.string, make_lit_string("open all code"))){
        open_all_code(app);
    }
    else if (match(bar.string, make_lit_string("open all code recursive"))){
        open_all_code_recursive(app);
    }
    else if(match(bar.string, make_lit_string("close all code"))){
        close_all_code(app);
    }
    else if (match(bar.string, make_lit_string("dos lines")) ||
             match(bar.string, make_lit_string("dosify"))){
        eol_dosify(app);
    }
    else if (match(bar.string, make_lit_string("nix lines")) ||
             match(bar.string, make_lit_string("nixify"))){
        eol_nixify(app);
    }
    else if (match(bar.string, make_lit_string("remap"))){
        remap_interactive(app);
    }
    else if (match(bar.string, make_lit_string("new project"))){
        setup_new_project(app);
    }
    else if (match(bar.string, make_lit_string("delete file"))){
        delete_file_query(app);
    }
    else if (match(bar.string, make_lit_string("rename file"))){
        rename_file_query(app);
    }
    else if (match(bar.string, make_lit_string("mkdir"))){
        make_directory_query(app);
    }
    else{
        print_message(app, literal("unrecognized command\n"));
    }
}

// BOTTOM

