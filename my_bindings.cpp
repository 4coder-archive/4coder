/*
New File
*/

#include "4coder_default_include.cpp"

extern "C" int32_t
get_bindings(void *data, int32_t size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_all_default_hooks(context);
    
    default_keys(context);
    begin_map(context, mapid_global);
    bind(context, 'M', MDFR_ALT, goto_prev_jump_no_skips_sticky);
    bind(context, 'N', MDFR_ALT, goto_first_jump_sticky);
    end_map(context);
    begin_map(context, mapid_file);
    end_map(context);
    begin_map(context, default_code_map);
    end_map(context);
    
    
    int32_t result = end_bind_helper(context);
    return(result);
}

