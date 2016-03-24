
#include "4coder_default_bindings.cpp"

enum Experiment_Maps{
	my_experiment_map = my_maps_count
};

HOOK_SIG(my_file_settings){
    Buffer_Summary buffer = app->get_parameter_buffer(app, 0);
    assert(buffer.exists);

    int treat_as_code = 0;
    int wrap_lines = 1;

    if (buffer.file_name && buffer.size < (16 << 20)){
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        if (match(ext, make_lit_string("cpp"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("h"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("c"))) treat_as_code = 1;
        else if (match(ext, make_lit_string("hpp"))) treat_as_code = 1;
    }

    if (treat_as_code){
        wrap_lines = 0;
    }
    if (buffer.file_name[0] == '*'){
        wrap_lines = 0;
    }

    push_parameter(app, par_lex_as_cpp_file, treat_as_code);
    push_parameter(app, par_wrap_lines, !treat_as_code);
    push_parameter(app, par_key_mapid, (treat_as_code)?((int)my_experiment_map):((int)mapid_file));
    exec_command(app, cmdid_set_settings);

    // no meaning for return
    return(0);
}

CUSTOM_COMMAND_SIG(kill_rect){
	// TODO
}

void experiments_get_bindings(Bind_Helper *context){
    default_get_bindings(context, 0);
    
    set_hook(context, hook_start, my_start);
    set_hook(context, hook_open_file, experiment_file_settings);
    
    begin_map(context, my_experiment_map);
    inherit_map(my_code_map);
    
    
    end_map(context);
}