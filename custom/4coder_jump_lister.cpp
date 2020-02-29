/*
4coder_jump_lister.cpp - Lister for jump buffers.
*/

// TOP

function Jump_Lister_Result
get_jump_index_from_user(Application_Links *app, Marker_List *list,
                         String_Const_u8 query){
    Jump_Lister_Result result = {};
    if (list != 0){
        Scratch_Block scratch(app);
        Lister_Block lister(app, scratch);
        lister_set_query(lister, query);
        lister_set_default_handlers(lister);
        
        Buffer_ID list_buffer = list->buffer_id;
        
        i32 option_count = list->jump_count;
        Managed_Object stored_jumps = list->jump_array;
        for (i32 i = 0; i < option_count; i += 1){
            Sticky_Jump_Stored stored = {};
            managed_object_load_data(app, stored_jumps, i, 1, &stored);
            String_Const_u8 line = push_buffer_line(app, scratch, list_buffer,
                                                    stored.list_line);
            lister_add_item(lister, line, SCu8(), IntAsPtr(i), 0);
        }
        
        Lister_Result l_result = run_lister(app, lister);
        if (!l_result.canceled){
            result.success = true;
            result.index = (i32)PtrAsInt(l_result.user_data);
        }
    }
    
    return(result);
}

function Jump_Lister_Result
get_jump_index_from_user(Application_Links *app, Marker_List *list, char *query){
    return(get_jump_index_from_user(app, list, SCu8(query)));
}

function void
jump_to_jump_lister_result(Application_Links *app, View_ID view,
                           Marker_List *list, Jump_Lister_Result *jump){
    if (jump->success){
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, list, jump->index, &location)){
            Buffer_ID jump_dst_buffer = {};
            if (get_jump_buffer(app, &jump_dst_buffer, &location)){
                view_set_active(app, view);
                jump_to_location(app, view, jump_dst_buffer, location);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(view_jump_list_with_lister)
CUSTOM_DOC("When executed on a buffer with jumps, creates a persistent lister for all the jumps")
{
    Heap *heap = &global_heap;
    View_ID view = get_active_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Marker_List *list = get_or_make_list_for_buffer(app, heap, buffer);
    if (list != 0){
        Jump_Lister_Result jump = get_jump_index_from_user(app, list, "Jump:");
        jump_to_jump_lister_result(app, view, list, &jump);
    }
}

// BOTTOM

