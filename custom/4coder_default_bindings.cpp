/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"
#include "generated/remapping.h"

// NOTE(allen|a4.0.22): This no longer serves as very good example code.
// Good example code will be coming soon, but in the mean time you can go
// to 4coder_remapping_commands.cpp for examples of what binding code looks like.

#if !defined(NO_BINDING)
void
custom_layer_init(Application_Links *app){
    set_all_default_hooks(app);
    Thread_Context *tctx = get_thread_context(app);
    mapping_init(tctx, &framework_mapping);
    setup_default_mapping(&framework_mapping);
    fill_log_graph_command_map(&framework_mapping);
}
#endif //NO_BINDING

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

