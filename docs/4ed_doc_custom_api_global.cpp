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
    
    if (begin_doc_call(arena, cluster, api_def, "clipboard_post_internal_only", &func)){
        doc_function_brief(arena, &func, "Post a string to 4coder's internal list clipboard history; the string is not posted to the system's clipboard with this call");
        
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
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "create_user_managed_scope", &func)){
        doc_function_brief(arena, &func, "Create a managed scope that is not tied to anything in the core and that the user can destroy");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the new managed scope's id");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "destroy_user_managed_scope", &func)){
        doc_function_brief(arena, &func, "Destroy a managed scope that was previously created by the user");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scope");
        doc_text(arena, params, "the id of the managed scope to destroy");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the scope exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_global_managed_scope", &func)){
        doc_function_brief(arena, &func, "Get the id of the 'global' managed scope");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the global managed scope");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The global managed scope has a lifetime tied to the instance of 4coder itself. It is unique in that it does not combine with other scopes to create unique intersection scopes. To put it another way, all scopes are automatically implicitly dependent on the global scope, so adding it to the list of scopes in a 'multiple dependencies' scope has no effect on the result.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_managed_scope_with_multiple_dependencies", &func)){
        doc_function_brief(arena, &func, "Get a scope that has a lifetime dependent on all of the dependent scopes and is unique up to set isomorphisms of the 'hard' dependencies");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scopes");
        doc_text(arena, params, "a pointer at the base of an array of scope ids specifying the set of dependencies to use in querying or constructing the resulting scope");
        
        doc_function_param(arena, &func, "count");
        doc_text(arena, params, "the number of scope ids in the scopes array");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the scope id of the scope with multiple dependencies on success, when all the scopes in input array exist, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The behavior of this call can be confusing so here are some rules of thumb for how it works, assuming all input scopes are valid:");
        doc_paragraph(arena, det);
        doc_text(arena, det, "1. When there is only one scope in the parameters, that scope is the result. {A} -> A;");
        doc_paragraph(arena, det);
        doc_text(arena, det, "2. When there are two or more parameters that are the same scope, the result is that scope again. {A, A, ..., A} -> A;");
        doc_paragraph(arena, det);
        doc_text(arena, det, "3. When any scope in the parameters is the special global scope, it is as if it is not there. {A, G} -> A");
        doc_paragraph(arena, det);
        doc_text(arena, det, "4. When two scopes are constructed from the same set of parameters, they are the same, regardless of parameter order. {A, B} -> C; {B, A} -> C;");
        doc_paragraph(arena, det);
        doc_text(arena, det, "5. When any of the scopes in the parameters was itself returned by this call, it is as if the parameters from it's constructor are substituted for it. {A, B} -> C; {C, D} -> E; {A, B, D} -> E;");
        doc_paragraph(arena, det);
        doc_text(arena, det, "6. When the parameter set is empty the result is the global scope. {} -> G");
        doc_paragraph(arena, det);
        doc_text(arena, det, "For a set-theoretic definition one can think of scopes as being keyed by sets of 'atoms'. Getting the key for a scope with multiple dependencies is defined by the operation of union of sets. The global scope is keyed by the empty set. A scope continues to exist for as long all of the atoms in it's key set exist.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_scope_clear_contents", &func)){
        doc_function_brief(arena, &func, "Clear everything allocated inside a given scope without destroying the scope itself");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scope");
        doc_text(arena, params, "the id of the scope to clear");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the scope exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_scope_clear_self_all_dependent_scopes", &func)){
        doc_function_brief(arena, &func, "Clear everything allocated inside an atomic scope and all of it's dependent scopes without destroying any of them");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scope");
        doc_text(arena, params, "the id of the scope to modify");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the scope exists and is atomic, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "A scope is atomic when it uniquely tied to a specified entity or when it is directly created by the user. By the set-theoretic definition of scopes, a scope is atomic when it's key contains only one atom.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "A scope is atomic specifically whenever it satisfies any of these conditions:");
        doc_paragraph(arena, det);
        doc_text(arena, det, "1. It is a scope tied to a buffer and returned by buffer_get_managed_scope");
        doc_paragraph(arena, det);
        doc_text(arena, det, "2. It is a scope tied to a view and returned by view_get_managed_scope");
        doc_paragraph(arena, det);
        doc_text(arena, det, "3. It was created by a call to create_user_managed_scope");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_scope_allocator", &func)){
        doc_function_brief(arena, &func, "Get the base allocator for a managed scope");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scope");
        doc_text(arena, params, "the id of the scope to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a pointer to the base allocator for the managed scope if it exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Anything allocated by this base allocator exists only as long as the managed scope exists. All of the allocations in the managed scope are freed by a bulk free operation when the managed scopes life time ends.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_id_group_highest_id", &func)){
        doc_function_brief(arena, &func, "Get the highest id issued for a particular group of managed ids");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "group");
        doc_text(arena, params, "a name identifying a managed id group");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the highest id returned by managed_id_declare for the given group, zero if the group has never been used to declare an id");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_id_declare", &func)){
        doc_function_brief(arena, &func, "Get a unique id for a given name within a particular group");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "group");
        doc_text(arena, params, "a name identifying a managed id group");
        
        doc_function_param(arena, &func, "name");
        doc_text(arena, params, "a name identifying a managed id");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id assigned to the given (group, name) pair");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When this is called for the first time for a given group, the first id returned is 1, subsequent calls with new names for an existing group return the next highest id, calls for (group, name) pairs that have already been assigned an id return the same id again. The upshot of this is that managed ids can be used to essentially create run-time allocated co-operative enums, where the group names the enum, and the names are the elements of the enum.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_id_get", &func)){
        doc_function_brief(arena, &func, "Like managed_id_declare except never returns new ids, only ids that have already been declared.");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "group");
        doc_text(arena, params, "a name identifying a managed id group");
        
        doc_function_param(arena, &func, "name");
        doc_text(arena, params, "a name identifying a managed id");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id assigned to the given (group, name) pair");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "managed_id_declare");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_scope_get_attachment", &func)){
        doc_function_brief(arena, &func, "Get an attachment contained to a managed scope, allocating it if necessary");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scope");
        doc_text(arena, params, "the id of a scope to query");
        
        doc_function_param(arena, &func, "id");
        doc_text(arena, params, "the id of the attachment to query");
        
        doc_function_param(arena, &func, "size");
        doc_text(arena, params, "the expected size for the attachment, used to allocate memory when the attachment did not previously exist, used to check that the attachment is at least as large as expected if it already exists");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a pointer to the base of the attachment when the scope exists and no error ocurred in checking the size of the attachment, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Attachments are allocated on a scopes base allocator, and thus are only valid for as long as the scope itself is valid. Whe in doubt, re-query for an attachment and recheck that the pointer returned is non-zero, as often calls between one usage and another can have an effect on the location or existence of an attachment.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_scope_attachment_erase", &func)){
        doc_function_brief(arena, &func, "Free an attachment on a managed scope");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "scope");
        doc_text(arena, params, "the id of the scope to modify");
        
        doc_function_param(arena, &func, "id");
        doc_text(arena, params, "the id of the attachment to modify");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero when the scope exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "alloc_buffer_markers_on_buffer", &func)){
        doc_function_brief(arena, &func, "Allocate buffer markers inside a managed scope and attach them to a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the buffer on which them markers will be attached");
        
        doc_function_param(arena, &func, "count");
        doc_text(arena, params, "the number of markers to allocate");
        
        doc_function_param(arena, &func, "optional_extra_scope");
        doc_text(arena, params, "either a null pointer, or a pointer to a managed scope");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the buffer exists, and the optional_extra_scope is valid, a new managed object id, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "By default, markers are allocated in the scope of the buffer they are attached to, this is how their lifetime is tied to the lifetime of the buffer. When an additional scope is supplied, it is combined with the buffer's scope via the same joining operation used in get_managed_scope_with_multiple_dependencies and the markers are allocated in that scope instead.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Markers are updated by edits to the buffer to which they are attached, so that they the same position in the buffer even as text is shifted around by edit operations. This can be used, for instance, to track the locations of compilation errors even as some compilation errors have already been fixed and shifted the position of later errors.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "get_managed_scope_with_multiple_dependencies");
        doc_function_add_related(arena, rel, "Marker");
    }
    
    ////////////////////////////////
    
    // TODO(allen): Remove alloc_managed_memory_in_scope
    if (begin_doc_call(arena, cluster, api_def, "alloc_managed_memory_in_scope", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_object_get_item_size", &func)){
        doc_function_brief(arena, &func, "Get the size of the items in a managed object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "object");
        doc_text(arena, params, "the id of the managed object to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists, the number of bytes in each item of the managed object, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Managed objects are essentially arrays with special management inside the core, such as markers. This call returns the size of the items in the array.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_object_get_item_count", &func)){
        doc_function_brief(arena, &func, "Get the number of the items in a managed object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "object");
        doc_text(arena, params, "the id of the managed object to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists, the number of items in the managed object, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Managed objects are essentially arrays with special management inside the core, such as markers. This call returns the number of items in the array.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_object_get_pointer", &func)){
        doc_function_brief(arena, &func, "Get a pointer to he base of a managed object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "object");
        doc_text(arena, params, "the id of the managed object to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists, a pointer to the base of the memory allocated for the object, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Managed objects are essentially arrays with special management inside the core, such as markers. This call returns a pointer to base of the array. Careful! This pointer is a pointer to memory tied to a managed scope, so it can lose validity if the scope closes, and modifications to the memory at this pointer will be reflected throughout all systems relying on it.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_object_get_type", &func)){
        doc_function_brief(arena, &func, "Get a type code indicating what sort data is represented by a managed object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "object");
        doc_text(arena, params, "the id of the object to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists, the type code of the object, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_object_get_containing_scope", &func)){
        doc_function_brief(arena, &func, "Get the id of the scope that contains a particular managed object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "object");
        doc_text(arena, params, "the id of a managed object");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists, the id of the managed scope that contains the object, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "managed_object_free", &func)){
        doc_function_brief(arena, &func, "Destroy a managed object, free it's memory back to the scope containing it, and detach it from whatever associations it has in the core");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "object");
        doc_text(arena, params, "the id of the object to free");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero when the object exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove managed_object_store_data
    if (begin_doc_call(arena, cluster, api_def, "managed_object_store_data", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove managed_object_load_data
    if (begin_doc_call(arena, cluster, api_def, "managed_object_load_data", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_next_input", &func)){
        doc_function_brief(arena, &func, "In a view context, yield control to the core until new input is sent to this view, then return that input");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "get_properties");
        doc_text(arena, params, "flags for properties of an event that should be allowed to be returned from this call");
        
        doc_function_param(arena, &func, "abort_properties");
        doc_text(arena, params, "flags for properties of an event that should be converted into abort events before returning from this call");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the next input that is sent to this view context, or cleared to zero if this is not a view context thread");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "This works as a blocking call on it's thread, blocking until the core has more input. However it only works in view context threads. View context threads are created by the core whenever a view is created, and has the primary responsibility of dispatching user input and other events. The base of view context threads is determined by HookID_ViewEventHandler.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "User_Input");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_current_input_sequence_number", &func)){
        doc_function_brief(arena, &func, "In a view context, get the sequence number for the input that has been most recently sent to the view");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the sequence number of the most recent input sent to the view, or zero if this is not a view context thread");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "get_next_input");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_current_input", &func)){
        doc_function_brief(arena, &func, "In a view context, get the input that has been most recently sent to the view");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the most recent input sent to the view; cleared to zero if this is not called from a view context thread, or if no inputs have been sent to this view context thread yet");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "get_next_input");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "set_current_input", &func)){
        doc_function_brief(arena, &func, "Modify the memory of the event that was most recently sent to the calling view context memory");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "input");
        doc_text(arena, params, "a pointer to the input struct to copy over the existing input struct in the core");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "This call only has the effect of altering the result of future calls to get_current_input until the next time input is sent to the calling view context thread. There is no effect when called from threads that are not view contexts.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "leave_current_input_unhandled", &func)){
        doc_function_brief(arena, &func, "Notifies the core that the input currently being handled by a view context thread should be treated as if it were not handled");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When events are handled, future handlers for them are skipped. This is especially important because text input is passed as a seperate event after key stroke events, and if a key stroke event is marked as handled, then the text event that would have been generated by the key stroke is skipped.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "set_custom_hook", &func)){
        doc_function_brief(arena, &func, "Modify a global hook binding");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "hook_id");
        doc_text(arena, params, "the id of the hook to be modified");
        
        doc_function_param(arena, &func, "func_ptr");
        doc_text(arena, params, "a pointer to the hook function, the function pointer must have a specific signature to match the hook_id's expected signature, but this call does not do the type checking for this, so watch out for that");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Hook_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_custom_hook", &func)){
        doc_function_brief(arena, &func, "Get back a global hook binding");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "hook_id");
        doc_text(arena, params, "the id of the hook to be modified");
        
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "when the hook id is valid the function pointer to the current hook function, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Hook_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "set_custom_hook_memory_size", &func)){
        doc_function_brief(arena, &func, "Set the memory size for the extra memory used to store the state of certain hooks");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "hook_id");
        doc_text(arena, params, "the id of the hook to be modified");
        
        doc_function_param(arena, &func, "size");
        doc_text(arena, params, "the size in bytes of the memory set aside for the state of the specified hook");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero when the hook id is valid and accepts extra memory size, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "view_current_context_hook_memory");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_mouse_state", &func)){
        doc_function_brief(arena, &func, "Get the state of the mouse as of this frame");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the current mouse state struct");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove get_active_query_bars
    if (begin_doc_call(arena, cluster, api_def, "get_active_query_bars", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove start_query_bar
    if (begin_doc_call(arena, cluster, api_def, "start_query_bar", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove end_query_bar
    if (begin_doc_call(arena, cluster, api_def, "end_query_bar", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove clear_all_query_bars
    if (begin_doc_call(arena, cluster, api_def, "clear_all_query_bars", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "print_message", &func)){
        doc_function_brief(arena, &func, "Print a message to the *messages* buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "message");
        doc_text(arena, params, "the string to write to the *messages* buffer");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "log_string", &func)){
        doc_function_brief(arena, &func, "Write a string to the *log* buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "str");
        doc_text(arena, params, "the string to write to the log");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero when the string was written to the log, each thread can individually have logging disabled, in which case zero is returned");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_largest_face_id", &func)){
        doc_function_brief(arena, &func, "Get the largest face id that is currently assigned to a face");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the largest face id that is currently assigned to a face");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "set_global_face", &func)){
        doc_function_brief(arena, &func, "Change the global default face");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "id");
        doc_text(arena, params, "the id of the new global default face");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the face exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "global_history_edit_group_begin", &func)){
        doc_function_brief(arena, &func, "Begin an edit group to merge all edits across all buffers");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When inside a global history edit group, all edits are merged into group edits, and the global edit number counter is frozen. Groups are formed by a nest counter, so that each call to global_history_edit_group_begin must be matched by a call to global_history_edit_group_end.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "global_history_edit_group_end", &func)){
        doc_function_brief(arena, &func, "End an edit group to merge all edits across all buffers");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When inside a global history edit group, all edits are merged into group edits, and the global edit number counter is frozen. Groups are formed by a nest counter, so that each call to global_history_edit_group_begin must be matched by a call to global_history_edit_group_end.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_face_description", &func)){
        doc_function_brief(arena, &func, "Get the description of a face");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the id of the face to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the description of the given face on success, when the face exists, otherwise cleared to zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "A face description contains the parameters to initializing a face.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Face_Description");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_face_metrics", &func)){
        doc_function_brief(arena, &func, "Get the metrics of a face");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the id of the face to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the metrics of the given face on success, when the face exists, otherwise cleared to zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Face_Metrics");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_face_advance_map", &func)){
        doc_function_brief(arena, &func, "Get the advance map of a face");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the id of the face to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the advance map of the given face on success, when the face exists, otherwise cleared to zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Face_Advance_Map");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_face_id", &func)){
        doc_function_brief(arena, &func, "Get the id of a face attached to a buffer, or of the global face");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query, or zero to query for the global face");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the queried face, if the buffer exists or if the query is for the global face, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "try_create_new_face", &func)){
        doc_function_brief(arena, &func, "Attempt to create a new face given a face description");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "description");
        doc_text(arena, params, "the description of the new face");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the new face on success, when it is possible to instantiate the face, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Face_Description");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "try_modify_face", &func)){
        doc_function_brief(arena, &func, "Attempt to modify a face given a face description");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "id");
        doc_text(arena, params, "the id of the face to modify");
        
        doc_function_param(arena, &func, "description");
        doc_text(arena, params, "the description to use to reinstantiate the face");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the face already exists and the new instantiation is possible, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Face_Description");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "try_release_face", &func)){
        doc_function_brief(arena, &func, "Attempt to release the resources tied up in a face");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "id");
        doc_text(arena, params, "the id of the face to release");
        
        doc_function_param(arena, &func, "replacement_id");
        doc_text(arena, params, "the id of the new face to replace existing usages of the released face");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the face exists and the replacement also exists and is a different face, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "push_hot_directory", &func)){
        doc_function_brief(arena, &func, "Get a copy of the hot directory");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a copy of the hot directory");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "set_hot_directory", &func)){
        doc_function_brief(arena, &func, "Set the hot directory");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "string");
        doc_text(arena, params, "the new value of the hot directory");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "send_exit_signal", &func)){
        doc_function_brief(arena, &func, "Send the signal to the core to try to shutdown");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The exit signal does not automatically close 4coder, first the exit hook is called, which can cancel the exit. If it does not cancel the exit, then 4coder closes.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "hard_exit", &func)){
        doc_function_brief(arena, &func, "Exits 4coder at the end of the frame, no matter what; for instance, call from the exit signal handler to actual exit 4coder.");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "set_window_title", &func)){
        doc_function_brief(arena, &func, "Set the title of the 4coder window");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "title");
        doc_text(arena, params, "the new title of the window");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "open_color_picker", &func)){
        doc_function_brief(arena, &func, "Potentially going to be deprecated - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "animate_in_n_milliseconds", &func)){
        doc_function_brief(arena, &func, "Set a wakeup timer to run a 4coder update tick and render");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "n");
        doc_text(arena, params, "the number of milliseconds roughly approximating how long to wait until sending a wakeup signal, the wakeup is always delayed by a minimum of a 16 milliseconds");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Without an animate timer set, 4coder only wakes up when events are sent to it. When there is an animation timer, 4coder wakes up when the animation timer triggers a timer event.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_core_profile_list", &func)){
        doc_function_brief(arena, &func, "Potentially going to be deprecated - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_custom_layer_boundary_docs", &func)){
        doc_function_brief(arena, &func, "Construct the documentation content for the custom layer boundary API");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the documentation content will be allocated");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a pointer to the newly constructed documentation content");
    }
    
}

// BOTTOM

