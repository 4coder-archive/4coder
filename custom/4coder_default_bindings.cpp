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
    async_task_handler_init(app, &global_async_system);
    code_index_init();
    buffer_modified_set_init();
    
    Profile_Global_List *list = get_core_profile_list(app);
    ProfileThreadName(tctx, list, string_u8_litexpr("main"));
    
    initialize_managed_id_metadata(app);
    set_default_color_scheme(app);
}

#endif

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

