/*
4coder_jump_lister.cpp - Lister for jump buffers.
*/

// TOP

static void
activate_jump(Application_Links *app, Heap *heap,
              View_ID view, struct Lister_State *state,
              String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    Lister_Activation_Code result_code = ListerActivation_Finished;
    i32 list_index = (i32)PtrAsInt(user_data);
    Jump_Lister_Parameters *params = (Jump_Lister_Parameters*)state->lister.data.user_data;
    Marker_List *list = get_marker_list_for_buffer(params->list_buffer_id);
    if (list != 0){
        View_ID target_view = {};
        switch (params->activation_rule){
            case JumpListerActivation_OpenInUIView:
            {
                target_view = view;
                result_code = ListerActivation_Finished;
            }break;
            
            case JumpListerActivation_OpenInTargetViewKeepUI:
            {
                target_view = params->target_view_id;;
                result_code = ListerActivation_Continue;
            }break;
            
            case JumpListerActivation_OpenInTargetViewCloseUI:
            {
                target_view = params->target_view_id;
                result_code = ListerActivation_Finished;
            }break;
            
            case JumpListerActivation_OpenInNextViewKeepUI:
            {
                target_view = view;
                target_view = get_next_view_looped_primary_panels(app, target_view, AccessAll);
                result_code = ListerActivation_Continue;
            }break;
            
            case JumpListerActivation_OpenInNextViewCloseUI:
            {
                target_view = view;
                target_view = get_next_view_looped_primary_panels(app, target_view, AccessAll);
                result_code = ListerActivation_Finished;
            }break;
        }
        
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, list, list_index, &location)){
            Buffer_ID buffer = {};
            if (get_jump_buffer(app, &buffer, &location)){
                view_set_active(app, target_view);
                jump_to_location(app, target_view, buffer, location);
            }
        }
        
    }
    lister_default(app, heap, view, state, result_code);
}

static void
open_jump_lister(Application_Links *app, Heap *heap, View_ID ui_view, Buffer_ID list_buffer_id, Jump_Lister_Activation_Rule activation_rule, View_ID optional_target_view){
    
    Marker_List *list = get_or_make_list_for_buffer(app, heap, list_buffer_id);
    if (list != 0){
        i32 estimated_string_space_size = 0;
        view_end_ui_mode(app, ui_view);
        Scratch_Block scratch(app);
        i32 option_count = list->jump_count;
        Lister_Option *options = push_array(scratch, Lister_Option, option_count);
        Managed_Object stored_jumps = list->jump_array;
        for (i32 i = 0; i < option_count; i += 1){
            Sticky_Jump_Stored stored = {};
            managed_object_load_data(app, stored_jumps, i, 1, &stored);
            String_Const_u8 line = push_buffer_line(app, scratch, list_buffer_id, stored.list_line);
            options[i].string = line;
            block_zero_struct(&options[i].status);
            options[i].user_data = IntAsPtr(i);
            i32 aligned_size = ((i32)line.size) + 1 + 7;
            aligned_size = aligned_size - aligned_size%8;
            estimated_string_space_size += aligned_size;
        }
        
        Jump_Lister_Parameters jump_lister_params = {};
        jump_lister_params.list_buffer_id = list_buffer_id;
        jump_lister_params.activation_rule = activation_rule;
        if (optional_target_view != 0){
            jump_lister_params.target_view_id = optional_target_view;
        }
        
        begin_integrated_lister__basic_list(app, "Jump:", activate_jump,
                                            &jump_lister_params, sizeof(jump_lister_params),
                                            options, option_count,
                                            estimated_string_space_size,
                                            ui_view);
    }
}

CUSTOM_COMMAND_SIG(view_jump_list_with_lister)
CUSTOM_DOC("When executed on a buffer with jumps, creates a persistent lister for all the jumps")
{
    View_ID view = get_active_view(app, AccessAll);
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    open_jump_lister(app, &global_heap, view, buffer, JumpListerActivation_OpenInNextViewKeepUI, 0);
}

// BOTTOM

