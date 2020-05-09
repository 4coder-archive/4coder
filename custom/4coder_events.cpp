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
copy_modifier_set(Input_Modifier_Set_Fixed *dst, Input_Modifier_Set *set){
    i32 count = clamp_top(set->count, ArrayCount(dst->mods));
    dst->count = count;
    block_copy(dst->mods, set->mods, count*sizeof(*set->mods));
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
has_modifier(Input_Event *event, Key_Code modifier){
    Input_Modifier_Set *set = get_modifiers(event);
    return(has_modifier(set, modifier));
}

function b32
is_unmodified_key(Input_Event *event){
    b32 result = false;
    if (event->kind == InputEventKind_KeyStroke){
        Input_Modifier_Set *set = get_modifiers(event);
        result = (!has_modifier(set, KeyCode_Control) &&
                  !has_modifier(set, KeyCode_Alt) &&
                  !has_modifier(set, KeyCode_Shift) &&
                  !has_modifier(set, KeyCode_Command));
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

function b32
event_is_dead_key(Input_Event *event){
    return(event->kind == InputEventKind_KeyStroke && HasFlag(event->key.flags, KeyFlag_IsDeadKey));
}

function Input_Event
event_next_text_event(Input_Event *event){
    Input_Event result = {};
    if (event != 0){
        if (event->kind == InputEventKind_KeyStroke &&
            event->key.first_dependent_text != 0){
            block_copy_struct(&result, event->key.first_dependent_text);
        }
        else if (event->kind == InputEventKind_TextInsert &&
                 event->text.next_text != 0){
            block_copy_struct(&result, event->text.next_text);
        }
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
        
        case InputEventKind_KeyRelease:
        {
            flags |= EventProperty_AnyKeyRelease;
        }break;
        
        case InputEventKind_MouseButton:
        {
            flags |= EventProperty_MouseButton;
        }break;
        
        case InputEventKind_MouseButtonRelease:
        {
            flags |= EventProperty_MouseRelease;
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
                
                case CoreCode_FileExternallyModified:
                {
                    flags |= EventProperty_AnyFile;
                }break;
                
                case CoreCode_Startup:
                {
                    flags |= EventProperty_Startup;
                }break;
                
                case CoreCode_TryExit:
                {
                    flags |= EventProperty_Exit;
                }break;
                
                case CoreCode_NewClipboardContents:
                {
                    flags |= EventProperty_Clipboard;
                }break;
            }
        }break;
        
        case InputEventKind_CustomFunction:
        {
            flags |= EventProperty_CustomFunction;
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

function Input_Event
copy_input_event(Arena *arena, Input_Event *event){
    Input_Event result = *event;
    switch (result.kind){
        case InputEventKind_TextInsert:
        {
            result.text.string = push_string_copy(arena, event->text.string);
        }break;
        
        case InputEventKind_KeyStroke:
        case InputEventKind_KeyRelease:
        {
            result.key.modifiers = copy_modifier_set(arena, &event->key.modifiers);
        }break;
        
        case InputEventKind_MouseButton:
        case InputEventKind_MouseButtonRelease:
        {
            result.mouse.modifiers = copy_modifier_set(arena, &event->mouse.modifiers);
        }break;
        
        case InputEventKind_MouseWheel:
        {
            result.mouse_wheel.modifiers = copy_modifier_set(arena, &event->mouse_wheel.modifiers);
        }break;
        
        case InputEventKind_MouseMove:
        {
            result.mouse_move.modifiers = copy_modifier_set(arena, &event->mouse_move.modifiers);
        }break;
        
        case InputEventKind_Core:
        {
            switch (result.core.code){
                case CoreCode_Startup:
                {
                    result.core.flag_strings = push_string_array_copy(arena, event->core.flag_strings);
                    result.core.file_names = push_string_array_copy(arena, event->core.file_names);
                }break;
                
                case CoreCode_FileExternallyModified:
                case CoreCode_NewClipboardContents:
                {
                    result.core.string = push_string_copy(arena, event->core.string);
                }break;
            }
        }break;
    }
    return(result);
}

////////////////////////////////

function String_Const_u8
stringize_keyboard_event(Arena *arena, Input_Event *event){
    List_String_Const_u8 list = {};
    
    switch (event->kind){
        case InputEventKind_TextInsert:
        {
            string_list_push(arena, &list, string_u8_litexpr("t"));
            u64 size = event->text.string.size;
            u8 *ptr = event->text.string.str;
            for (u64 i = 0; i < size; i += 1, ptr += 1){
                string_list_pushf(arena, &list, "%02X", (i32)(*ptr));
            }
            string_list_push(arena, &list, string_u8_litexpr("\n"));
        }break;
        
        case InputEventKind_KeyStroke:
        case InputEventKind_KeyRelease:
        {
            string_list_pushf(arena, &list, "k%X ", event->key.code);
            if (event->kind == InputEventKind_KeyRelease){
                string_list_push(arena, &list, string_u8_litexpr("^"));
            }
            i32 count = event->key.modifiers.count;
            if (count > 0){
                Key_Code *m = event->key.modifiers.mods;
                string_list_push(arena, &list, string_u8_litexpr("m{"));
                for (i32 i = 0; i < count; i += 1, m += 1){
                    string_list_pushf(arena, &list, "%X ", *m);
                }
                string_list_push(arena, &list, string_u8_litexpr("}"));
            }
            string_list_push(arena, &list, string_u8_litexpr("\n"));
        }break;
    }
    
    return(string_list_flatten(arena, list));
}

function Input_Event
parse_keyboard_event(Arena *arena, String_Const_u8 text){
    Input_Event result = {};
    u64 pos = 0;
    Range_i64 range = {};
    
    if (pos < text.size && text.str[pos] == 't'){
        pos += 1;
        result.kind = InputEventKind_TextInsert;
        
        u64 max_size = text.size/2;
        result.text.string.str = push_array(arena, u8, max_size);
        for (; pos + 1 < text.size; pos += 2){
            if (character_is_base16(text.str[pos]) &&
                character_is_base16(text.str[pos + 1])){
                String_Const_u8 byte_str = {text.str + pos, 2};
                result.text.string.str[result.text.string.size] = (u8)string_to_integer(byte_str, 16);
                result.text.string.size += 1;
            }
        }
    }
    else if (pos < text.size && text.str[pos] == 'k'){
        pos += 1;
        result.kind = InputEventKind_KeyStroke;
        
        range.first = pos;
        for (;pos < text.size && character_is_base16(text.str[pos]); pos += 1);
        range.one_past_last = pos;
        
        if (range.first == range.one_past_last){
            result.kind = InputEventKind_None;
        }
        else{
            String_Const_u8 code_str = string_substring(text, range);
            result.key.code = (u32)string_to_integer(code_str, 16);
            
            for (;pos < text.size && character_is_whitespace(text.str[pos]); pos += 1);
            
            if (pos < text.size && text.str[pos] == '^'){
                result.kind = InputEventKind_KeyRelease;
                pos += 1;
                for (;pos < text.size && character_is_whitespace(text.str[pos]); pos += 1);
            }
            
            if (pos < text.size && text.str[pos] == 'm'){
                pos += 1;
                if (pos < text.size && text.str[pos] == '{'){
                    pos += 1;
                    
                    Input_Modifier_Set_Fixed mods = {};
                    for (;mods.count < ArrayCount(mods.mods);){
                        for (;pos < text.size && character_is_whitespace(text.str[pos]); pos += 1);
                        range.first = pos;
                        for (;pos < text.size && character_is_base16(text.str[pos]); pos += 1);
                        range.one_past_last = pos;
                        
                        if (range.first == range.one_past_last){
                            break;
                        }
                        
                        code_str = string_substring(text, range);
                        mods.mods[mods.count] = (u32)string_to_integer(code_str, 16);
                        mods.count += 1;
                    }
                    
                    result.key.modifiers = copy_modifier_set(arena, &mods);
                }
            }
        }
    }
    
    return(result);
}

// BOTTOM
