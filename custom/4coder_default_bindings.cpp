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
    
#define BindAttachmentID(N) N = managed_id_declare(app, SCu8("attachment"), SCu8(#N))
    
    BindAttachmentID(view_rewrite_loc);
    BindAttachmentID(view_next_rewrite_loc);
    BindAttachmentID(view_paste_index_loc);
    BindAttachmentID(view_is_passive_loc);
    BindAttachmentID(view_snap_mark_to_cursor);
    BindAttachmentID(view_ui_data);
    BindAttachmentID(view_highlight_range);
    BindAttachmentID(view_highlight_buffer);
    BindAttachmentID(view_render_hook);
    BindAttachmentID(view_word_complete_menu);
    
    BindAttachmentID(buffer_map_id);
    BindAttachmentID(buffer_eol_setting);
    BindAttachmentID(buffer_lex_task);
    
    BindAttachmentID(sticky_jump_marker_handle);
    BindAttachmentID(attachment_tokens);
}

#endif

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

