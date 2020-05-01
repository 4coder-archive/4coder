/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.05.2020
 *
 * Implementation of the root models features.
 *
 */

// TOP

function void
models_push_view_command_function(Models *models, View_ID view_id, Custom_Command_Function *custom_func){
    Model_View_Command_Function *node = models->free_view_cmd_funcs;
    if (node == 0){
        node = push_array(models->arena, Model_View_Command_Function, 1);
    }
    else{
        sll_stack_pop(models->free_view_cmd_funcs);
    }
    sll_queue_push(models->first_view_cmd_func, models->last_view_cmd_func, node);
    node->view_id = view_id;
    node->custom_func = custom_func;
}

function Model_View_Command_Function
models_pop_view_command_function(Models *models){
    Model_View_Command_Function result = {};
    if (models->first_view_cmd_func != 0){
        Model_View_Command_Function *node = models->first_view_cmd_func;
        result.custom_func = node->custom_func;
        result.view_id = node->view_id;
        sll_queue_pop(models->first_view_cmd_func, models->last_view_cmd_func);
        sll_stack_push(models->free_view_cmd_funcs, node);
    }
    return(result);
}

function void
models_push_virtual_event(Models *models, Input_Event *event){
    Model_Input_Event_Node *node = models->free_virtual_event;
    if (node == 0){
        node = push_array(&models->virtual_event_arena, Model_Input_Event_Node, 1);
    }
    else{
        sll_stack_pop(models->free_virtual_event);
    }
    sll_queue_push(models->first_virtual_event, models->last_virtual_event, node);
    node->event = copy_input_event(&models->virtual_event_arena, event);
}

function Input_Event
models_pop_virtual_event(Arena *arena, Models *models){
    Input_Event result = {};
    if (models->first_virtual_event != 0){
        Model_Input_Event_Node *node = models->first_virtual_event;
        result = copy_input_event(arena, &node->event);
        sll_queue_pop(models->first_virtual_event, models->last_virtual_event);
        sll_stack_push(models->free_virtual_event, node);
    }
    return(result);
}

function void
models_push_wind_down(Models *models, Coroutine *co){
    Model_Wind_Down_Co *node = models->free_wind_downs;
    if (node != 0){
        sll_stack_pop(models->free_wind_downs);
    }
    else{
        node = push_array(models->arena, Model_Wind_Down_Co, 1);
    }
    sll_stack_push(models->wind_down_stack, node);
    node->co = co;
}

// BOTTOM
