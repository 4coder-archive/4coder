/*
 * 4coder event helpers
 */

// TOP

internal b32
is_unmodified_key(Input_Event *event){
    b32 result = false;
    if (event->kind == InputEventKind_KeyStroke){
        b8 *mods = event->key.modifiers.modifiers;
        result = (!mods[MDFR_CONTROL_INDEX] && !mods[MDFR_ALT_INDEX]);
    }
    return(result);
}

internal b32
is_modified(Input_Event *event){
    b8 *mods = 0;
    switch (event->kind){
        case InputEventKind_KeyStroke:
        {
            mods = event->key.modifiers.modifiers;
        }break;
        case InputEventKind_MouseButton:
        {
            mods = event->mouse.modifiers.modifiers;
        }break;
    }
    b32 result = false;
    if (mods != 0){
        result = (mods[MDFR_CONTROL_INDEX] ||
                  mods[MDFR_ALT_INDEX] ||
                  mods[MDFR_SHIFT_INDEX] ||
                  mods[MDFR_COMMAND_INDEX]);
    }
    return(result);
}

internal String_Const_u8
to_writable(Input_Event *event){
    String_Const_u8 result = {};
    if (event->kind == InputEventKind_TextInsert){
        result = event->text.string;
    }
    return(result);
}

internal b32
match_key_code(Input_Event *event, Key_Code code){
    return(event->kind == InputEventKind_KeyStroke && event->key.code == code);
}

internal b32
match_mouse_code(Input_Event *event, Mouse_Code code){
    return(event->kind == InputEventKind_MouseButton &&
           event->mouse.code == code && !event->mouse.release);
}

internal b32
match_mouse_code_release(Input_Event *event, Mouse_Code code){
    return(event->kind == InputEventKind_MouseButton &&
           event->mouse.code == code && event->mouse.release);
}

internal b32
match_core_code(Input_Event *event, Core_Code code){
    return(event->kind == InputEventKind_Core && event->core.code == code);
}

internal Event_Property
get_event_properties(Input_Event *event){
    Event_Property flags = 0;
    
    switch (event->kind){
        case InputEventKind_TextInsert:
        {
            flags |= EventProperty_TextInsert;
        }break;
        
        case InputEventKind_KeyStroke:
        {
            if (event->key.code == KeyCode_Escape){
                flags |= EventProperty_Escape;
            }
            flags |= EventProperty_AnyKey;
        }break;
        
        case InputEventKind_MouseButton:
        {
            switch (event->mouse.code){
                case MouseCode_Left:
                {
                    flags |= EventProperty_MouseLeft;
                }break;
                
                case MouseCode_Middle:
                {
                    flags |= EventProperty_MouseMiddle;
                }break;
                
                case MouseCode_Right:
                {
                    flags |= EventProperty_MouseRight;
                }break;
            }
        }break;
        
        case InputEventKind_MouseWheel:
        {
            flags |= EventProperty_MouseWheel;
        }break;
        
        case InputEventKind_MouseMove:
        {
            flags |= EventProperty_MouseMove;
        }break;
        
        case InputEventKind_Core:
        {
            switch (event->core.code){
                case CoreCode_Animate:
                {
                    flags |= EventProperty_Animate;
                }break;
                
                case CoreCode_ClickActivateView:
                case CoreCode_ClickDeactivateView:
                {
                    flags |= EventProperty_ViewActivation;
                }break;
            }
        }break;
    }
    
    return(flags);
}

internal Input_Event*
push_input_event(Arena *arena, Input_List *list){
    Input_Event_Node *node = push_array_zero(arena, Input_Event_Node, 1);
    sll_queue_push(list->first, list->last, node);
    list->count += 1;
    return(&node->event);
}

internal Input_Event*
push_input_event(Arena *arena, Input_List *list, Input_Event *event){
    Input_Event_Node *node = push_array(arena, Input_Event_Node, 1);
    block_copy_struct(&node->event, event);
    sll_queue_push(list->first, list->last, node);
    list->count += 1;
    return(&node->event);
}

// BOTTOM
