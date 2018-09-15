/*
4coder_jump_lister.cpp - Lister for jump buffers.
*/

// TOP

static Lister_Activation_Code
activate_jump(Application_Links *app, View_Summary *view, String text_field,
              void *user_data, bool32 activated_by_mouse){
    int32_t list_index = (int32_t)PtrAsInt(user_data);
    Lister_State *state = view_get_lister_state(view);
    Jump_Lister_Parameters *params = (Jump_Lister_Parameters*)state->lister.user_data;
    Marker_List *list = get_marker_list_for_buffer(params->list_buffer_id);
    if (list != 0){
        Lister_Activation_Code result_code = ListerActivation_Finished;
        View_Summary target_view = {0};
        switch (params->activation_rule){
            case JumpListerActivation_OpenInUIView:
            {
                target_view = *view;
                result_code = ListerActivation_Finished;
            }break;
            
            case JumpListerActivation_OpenInTargetViewKeepUI:
            {
                target_view = get_view(app, params->target_view_id, AccessAll);
                result_code = ListerActivation_Continue;
            }break;
            
            case JumpListerActivation_OpenInTargetViewCloseUI:
            {
                target_view = get_view(app, params->target_view_id, AccessAll);
                result_code = ListerActivation_Finished;
            }break;
            
            case JumpListerActivation_OpenInNextViewKeepUI:
            {
                target_view = *view;
                get_next_view_looped_primary_panels(app, &target_view, AccessAll);
                result_code = ListerActivation_Continue;
            }break;
            
            case JumpListerActivation_OpenInNextViewCloseUI:
            {
                target_view = *view;
                get_next_view_looped_primary_panels(app, &target_view, AccessAll);
                result_code = ListerActivation_Finished;
            }break;
        }
        
        ID_Pos_Jump_Location location = {0};
        if (get_jump_from_list(app, list, list_index, &location)){
            Buffer_Summary buffer = {0};
            if (get_jump_buffer(app, &buffer, &location)){
                set_active_view(app, &target_view);
                jump_to_location(app, &target_view, &buffer, location);
            }
        }
        
        return(result_code);
    }
    return(ListerActivation_Finished);
}

static void
open_jump_lister(Application_Links *app, Partition *scratch, Heap *heap,
                 View_Summary *ui_view, Buffer_ID list_buffer_id,
                 Jump_Lister_Activation_Rule activation_rule, View_Summary *optional_target_view){
    
    Marker_List *list = get_or_make_list_for_buffer(app, scratch, heap, list_buffer_id);
    if (list != 0){
        Buffer_Summary list_buffer = get_buffer(app, list_buffer_id, AccessAll);
        
        int32_t estimated_string_space_size = 0;
        view_end_ui_mode(app, ui_view);
        Temp_Memory temp = begin_temp_memory(scratch);
        int32_t option_count = list->jump_count;
        Lister_Option *options = push_array(scratch, Lister_Option, option_count);
        Managed_Object stored_jumps = list->jump_array;
        for (int32_t i = 0; i < option_count; i += 1){
            Sticky_Jump_Stored stored = {0};
            managed_object_load_data(app, stored_jumps, i, 1, &stored);
            String line = {0};
            read_line(app, scratch, &list_buffer, stored.list_line, &line);
            options[i].string = line.str;
            options[i].status = 0;
            options[i].user_data = IntAsPtr(i);
            int32_t aligned_size = line.size + 1 + 7;
            aligned_size = aligned_size - aligned_size%8;
            estimated_string_space_size += aligned_size;
        }
        
        Jump_Lister_Parameters jump_lister_params = {0};
        jump_lister_params.list_buffer_id = list_buffer_id;
        jump_lister_params.activation_rule = activation_rule;
        if (optional_target_view != 0){
            jump_lister_params.target_view_id = optional_target_view->view_id;
        }
        
        begin_integrated_lister__basic_list(app, "Jump:", activate_jump,
                                            &jump_lister_params, sizeof(jump_lister_params),
                                            options, option_count,
                                            estimated_string_space_size,
                                            ui_view);
        end_temp_memory(temp);
    }
}

CUSTOM_COMMAND_SIG(view_jump_list_with_lister)
CUSTOM_DOC("When executed on a buffer with jumps, creates a persistent lister for all the jumps")
{
    View_Summary view = get_active_view(app, AccessAll);
    open_jump_lister(app, &global_part, &global_heap,
                     &view, view.buffer_id, JumpListerActivation_OpenInNextViewKeepUI, 0);
}

// BOTTOM

