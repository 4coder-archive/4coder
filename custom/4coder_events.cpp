/*
 * 4coder event helpers
 */

// TOP

function b32
has_modifier(Key_Code *mods, i32 count, Key_Code modifier){
    b32 result = false;
    for (i32 i = 0; i < count; i += 1){
        if (mods[i] == modifier){
            result = true;
            break;
        }
    }
    return(result);
}

function b32
has_modifier(Input_Modifier_Set_Fixed *set, Key_Code modifier){
    return(has_modifier(set->mods, set->count, modifier));
}

function b32
has_modifier(Input_Modifier_Set *set, Key_Code modifier){
    return(has_modifier(set->mods, set->count, modifier));
}

function Input_Modifier_Set
copy_modifier_set(Arena *arena, Input_Modifier_Set_Fixed *set){
    Input_Modifier_Set result = {};
    result.count = set->count;
    if (result.count > 0){
        result.mods = push_array_write(arena, Key_Code, result.count, set->mods);
    }
    return(result);
}

function void
add_modifier(Input_Modifier_Set_Fixed *set, Key_Code mod){
    if (!has_modifier(set, mod)){
        if (set->count < ArrayCount(set->mods)){
            set->mods[set->count] = mod;
            set->count += 1;
        }
    }
}

function void
remove_modifier(Input_Modifier_Set_Fixed *set, Key_Code mod){
    i32 count = set->count;
    Key_Code *mods = set->mods;
    for (i32 i = 0; i < count; i += 1){
        if (mods[i] == mod){
            i32 new_count = count - 1;
            mods[i] = mods[new_count];
            set->count = new_count;
            break;
        }
    }
}

function void
set_modifier(Input_Modifier_Set_Fixed *set, Key_Code mod, b32 val){
    if (val){
        add_modifier(set, mod);
    }
    else{
        remove_modifier(set, mod);
    }
}

function Input_Modifier_Set
copy_modifier_set(Arena *arena, Input_Modifier_Set *set){
    Input_Modifier_Set result = {};
    result.count = set->count;
    if (result.count > 0){
        result.mods = push_array_write(arena, Key_Code, result.count, set->mods);
    }
    return(result);
}

function b32
is_unmodified_key(Input_Event *event){
    b32 result = false;
    if (event->kind == InputEventKind_KeyStroke){
        result = (event->key.modifiers.count == 0);
    }
    return(result);
}

function Input_Modifier_Set*
get_modifiers(Input_Event *event){
    Input_Modifier_Set *result = 0;
    switch (event->kind){
        case InputEventKind_KeyStroke:
        {
            result = &event->key.modifiers;
        }break;
        case InputEventKind_MouseButton:
        {
            result = &event->mouse.modifiers;
        }break;
        case InputEventKind_MouseWheel:
        {
            result = &event->mouse_wheel.modifiers;
        }break;
        case InputEventKind_MouseMove:
        {
            result = &event->mouse_move.modifiers;
        }break;
    }
    return(result);
}

function b32
is_modified(Input_Event *event){
    Input_Modifier_Set *mods = get_modifiers(event);
    b32 result = false;
    if (mods != 0){
        result = (mods->count > 0);
    }
    return(result);
}

function String_Const_u8
to_writable(Input_Event *event){
    String_Const_u8 result = {};
    if (event->kind == InputEventKind_TextInsert){
        result = event->text.string;
    }
    return(result);
}

function b32
match_key_code(Input_Event *event, Key_Code code){
    return(event->kind == InputEventKind_KeyStroke && event->key.code == code);
}

function b32
match_mouse_code(Input_Event *event, Mouse_Code code){
    return(event->kind == InputEventKind_MouseButton && event->mouse.code == code);
}

function b32
match_mouse_code_release(Input_Event *event, Mouse_Code code){
    return(event->kind == InputEventKind_MouseButtonRelease && event->mouse.code == code);
}

function b32
match_core_code(Input_Event *event, Core_Code code){
    return(event->kind == InputEventKind_Core && event->core.code == code);
}

function Event_Property
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

function Input_Event*
push_input_event(Arena *arena, Input_List *list){
    Input_Event_Node *node = push_array_zero(arena, Input_Event_Node, 1);
    sll_queue_push(list->first, list->last, node);
    list->count += 1;
    return(&node->event);
}

function Input_Event*
push_input_event(Arena *arena, Input_List *list, Input_Event *event){
    Input_Event_Node *node = push_array(arena, Input_Event_Node, 1);
    block_copy_struct(&node->event, event);
    sll_queue_push(list->first, list->last, node);
    list->count += 1;
    return(&node->event);
}

// BOTTOM
