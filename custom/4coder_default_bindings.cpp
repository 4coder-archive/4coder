/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"
#include "generated/remapping.h"

#if !defined(NO_BINDING)

void
custom_layer_init(Application_Links *app){
    set_all_default_hooks(app);
    Thread_Context *tctx = get_thread_context(app);
    mapping_init(tctx, &framework_mapping);
    setup_default_mapping(&framework_mapping);
    profile_history_init();
}

#endif

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

