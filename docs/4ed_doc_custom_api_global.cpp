/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 07.12.2019
 *
 * Documentation of the custom layer's primary api : global-related calls
 *
 */

// TOP

function void
doc_custom_api__global(Arena *arena, API_Definition *api_def, Doc_Cluster *cluster){
    Doc_Function func = {};
    
    if (begin_doc_call(arena, cluster, api_def, "global_set_setting", &func)){
        doc_function_brief(arena, &func, "Modify a core setting");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "setting");
        doc_text(arena, params, "the id of the setting to set");
        
        doc_function_param(arena, &func, "value");
        doc_text(arena, params, "the new value for the setting");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero when the setting is valid and updated without error, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Global_Setting_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "global_get_screen_rectangle", &func)){
        doc_function_brief(arena, &func, "Get the rectangle the represents the renderable region in pixels");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "an f32 rectangle based as (0, 0) that represents the entire renderable region in pixels");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_thread_context", &func)){
        doc_function_brief(arena, &func, "Get the current thread's context");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the pointer to the current thread's context");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "create_child_process", &func)){
        doc_function_brief(arena, &func, "Create a child process of the 4coder process");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "path");
        doc_text(arena, params, "the active path set for the new child process");
        
        doc_function_param(arena, &func, "command");
        doc_text(arena, params, "the command that defines the action of the child process - this string is passed to the OS's default command line handler");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success a non-zero id for the new child process which is unique across the entire 4coder session, zero on failure");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "child_process_set_target_buffer", &func)){
        doc_function_brief(arena, &func, "Create a link between a child process and a buffer, so that output from the child process is written to the buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "child_process_id");
        doc_text(arena, params, "the id of the child process who's output will be linked");
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer that will receive the output");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags setting the behavior when the child process or buffer already have a link");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success when the two entities exist and the link is created non-zero, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Each child process and each buffer may only be a part of one link at a time. So when either is already linked a decision needs to be made to either destroy that link or not create the new link. This decision is controlled by the flags.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Child_Process_Set_Target_Flags");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_attached_child_process", &func)){
        doc_function_brief(arena, &func, "Retrieve the child process linked to a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "if the given buffer exists and has an attached child process it's id is returned, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "child_process_get_attached_buffer", &func)){
        doc_function_brief(arena, &func, "Retrieve the buffer linked to a child process");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "child_process_id");
        doc_text(arena, params, "the id of the child process");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "if the given child process exists and has an attached buffer it's id is returned, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "child_process_get_state", &func)){
        doc_function_brief(arena, &func, "Get the state of a child process");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "child_process_id");
        doc_text(arena, params, "the id of the child process");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the state fields of the child process");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Process_State");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "clipboard_post", &func)){
        doc_function_brief(arena, &func, "Post a string to 4coder internal clipboard and the system clipboard");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "clipboard_id");
        doc_text(arena, params, "the id of the clipboard to receive the post - currently an ignored parameter and should be set to zero");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the state fields of the child process");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "clipboard_count", &func)){
        doc_function_brief(arena, &func, "Retrieve the number of strings on the clipboard's history");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "clipboard_id");
        doc_text(arena, params, "the id of the clipboard to query - currently an ignored parameter and should be set to zero");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the number of strings on the clipboard's history");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "push_clipboard_index", &func)){
        doc_function_brief(arena, &func, "Get a copy of a string from the clipboard history by index");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        doc_function_param(arena, &func, "clipboard_id");
        doc_text(arena, params, "the id of the clipboard to query - currently an ignored parameter and should be set to zero");
        
        doc_function_param(arena, &func, "item_index");
        doc_text(arena, params, "the index in the clipboard history to query, 0 always the most recent string in the history");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the clipboard contains a string at the given index, a copy of the string is returned, otherwise an empty string is returned");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "push_clipboard_index", &func)){
        doc_function_brief(arena, &func, "Get a copy of a string from the clipboard history by index");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        doc_function_param(arena, &func, "clipboard_id");
        doc_text(arena, params, "the id of the clipboard to query - currently an ignored parameter and should be set to zero");
        
        doc_function_param(arena, &func, "item_index");
        doc_text(arena, params, "the index in the clipboard history to query, 0 always the most recent string in the history");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the clipboard contains a string at the given index, a copy of the string is returned, otherwise an empty string is returned");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "enqueue_virtual_event", &func)){
        doc_function_brief(arena, &func, "Push an event to the core to be processed as if it were a real event");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "event");
        doc_text(arena, params, "a pointer to an event struct that contains the information that will be in the virtual event");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the event specified is validly formed, non-zero is returned, otherwise zero is returned");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "All virtual events are triggered after all directly enqueued commands, before the next real event, and before the current frame renders.");
    }
}

// BOTTOM

