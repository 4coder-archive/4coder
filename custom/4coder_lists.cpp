/*
4coder_lists.cpp - List helpers and list commands
such as open file, switch buffer, or kill buffer.
*/

// TOP

CUSTOM_COMMAND_SIG(lister__write_string__default)
CUSTOM_DOC("A lister mode command that inserts a new character to the text field.")
{
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        String_Const_u8 string = to_writable(&in);
        if (string.str != 0 && string.size > 0){
            lister_append_text_field(&state->lister, string);
            lister_append_key(&state->lister, string);
            state->item_index = 0;
            view_zero_scroll(app, view);
            lister_update_filtered_list(app, view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field__default)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field.")
{
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        state->lister.data.text_field.string = backspace_utf8(state->lister.data.text_field.string);
        state->lister.data.key_string.string = backspace_utf8(state->lister.data.key_string.string);
        state->item_index = 0;
        view_zero_scroll(app, view);
        lister_update_filtered_list(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__move_up__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one up in the list.")
{
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        state->item_index = state->item_index - 1;
        if (state->item_index < 0){
            state->item_index = state->filtered.count - 1;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_filtered_list(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__move_down__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one down in the list.")
{
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        state->item_index = state->item_index + 1;
        if (state->item_index > state->filtered.count - 1){
            state->item_index = 0;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_filtered_list(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__file_path)
CUSTOM_DOC("A lister mode command that inserts a character into the text field of a file system list.")
{
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        String_Const_u8 string = to_writable(&in);
        if (string.str != 0 && string.size > 0){
            lister_append_text_field(&state->lister, string);
            String_Const_u8 front_name = string_front_of_path(state->lister.data.text_field.string);
            lister_set_key(&state->lister, front_name);
            if (character_is_slash(string.str[0])){
                String_Const_u8 new_hot = state->lister.data.text_field.string;
                set_hot_directory(app, new_hot);
                lister_call_refresh_handler(app, &state->lister);
            }
            state->item_index = 0;
            view_zero_scroll(app, view);
            lister_update_filtered_list(app, view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field__file_path)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field of a file system list.")
{
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        if (state->lister.data.text_field.size > 0){
            char last_char = state->lister.data.text_field.str[state->lister.data.text_field.size - 1];
            state->lister.data.text_field.string = backspace_utf8(state->lister.data.text_field.string);
            if (character_is_slash(last_char)){
                User_Input input = get_command_input(app);
                String_Const_u8 text_field = state->lister.data.text_field.string;
                String_Const_u8 new_hot = string_remove_last_folder(text_field);
                b32 is_modified = has_modifier(&input, KeyCode_Control);
                b32 whole_word_backspace = (is_modified == global_config.lister_whole_word_backspace_when_modified);
                if (whole_word_backspace){
                    state->lister.data.text_field.size = new_hot.size;
                }
                set_hot_directory(app, new_hot);
                // TODO(allen): We have to protect against lister_call_refresh_handler changing 
                // the text_field here. Clean this up.
                String_u8 dingus = state->lister.data.text_field;
                lister_call_refresh_handler(app, &state->lister);
                state->lister.data.text_field = dingus;
            }
            else{
                String_Const_u8 text_field = state->lister.data.text_field.string;
                String_Const_u8 new_key = string_front_of_path(text_field);
                lister_set_key(&state->lister, new_key);
            }
            
            state->item_index = 0;
            view_zero_scroll(app, view);
            lister_update_filtered_list(app, view, state);
        }
    }
}

function Lister_Activation_Code
lister__key_stroke__fixed_list(Application_Links *app)
{
    Lister_Activation_Code result = ListerActivation_Continue;
    Heap *heap = &global_heap;
    View_ID view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        if (in.event.kind == InputEventKind_KeyStroke){
            void *user_data = 0;
            b32 did_shortcut_key = false;
            for (Lister_Node *node = state->lister.data.options.first;
                 node != 0;
                 node = node->next){
                Key_Code *key_code = (Key_Code*)(node + 1);
                if (*key_code == in.event.key.code){
                    user_data = node->user_data;
                    did_shortcut_key = true;
                    break;
                }
            }
            if (did_shortcut_key){
                result = lister_call_activate_handler(app, heap, view, state, user_data, false);
            }
        }
    }
    return(result);
}

////////////////////////////////

function void
lister_input_handling_loop(Application_Links *app, View_ID view){
    Heap *heap = &global_heap;
    Lister_State *state = view_get_lister_state(view);
    
    Managed_Scope scope = view_get_managed_scope(app, view);
    View_Render_Hook **hook = scope_attachment(app, scope, view_render_hook, View_Render_Hook*);
    *hook = lister_render;
    
    for (;;){
        User_Input in = get_user_input(app,
                                       EventPropertyGroup_Any,
                                       EventProperty_Escape);
        if (in.abort){
            break;
        }
        
        Lister_Activation_Code result = ListerActivation_Continue;
        b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_TextInsert:
            {
                if (state->lister.data.handlers.write_character != 0){
                    state->lister.data.handlers.write_character(app);
                }
            }break;
            
            case InputEventKind_KeyStroke:
            {
                switch (in.event.key.code){
                    case KeyCode_Return:
                    case KeyCode_Tab:
                    {
                        void *user_data = 0;
                        if (0 <= state->raw_item_index &&
                            state->raw_item_index < state->lister.data.options.count){
                            user_data = lister_get_user_data(&state->lister.data,
                                                             state->raw_item_index);
                        }
                        result = lister_call_activate_handler(app, heap, view, state,
                                                              user_data, false);
                    }break;
                    
                    case KeyCode_Backspace:
                    {
                        if (state->lister.data.handlers.backspace != 0){
                            state->lister.data.handlers.backspace(app);
                        }
                        else if (state->lister.data.handlers.key_stroke != 0){
                            result = state->lister.data.handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_Up:
                    {
                        if (state->lister.data.handlers.navigate_up != 0){
                            state->lister.data.handlers.navigate_up(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    case KeyCode_Down:
                    {
                        if (state->lister.data.handlers.navigate_down != 0){
                            state->lister.data.handlers.navigate_down(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                    
                    default:
                    {
                        if (state->lister.data.handlers.key_stroke != 0){
                            result = state->lister.data.handlers.key_stroke(app);
                        }
                        else{
                            handled = false;
                        }
                    }break;
                }
            }break;
            
            case InputEventKind_MouseButton:
            {
                switch (in.event.mouse.code){
                    case MouseCode_Left:
                    {
                        Vec2_f32 p = V2f32(in.event.mouse.p);
                        void *clicked = lister_user_data_at_p(app, view, state, p);
                        state->hot_user_data = clicked;
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            case InputEventKind_MouseButtonRelease:
            {
                switch (in.event.mouse.code){
                    case MouseCode_Left:
                    {
                        if (state->hot_user_data != 0){
                            Vec2_f32 p = V2f32(in.event.mouse.p);
                            void *clicked = lister_user_data_at_p(app, view, state, p);
                            if (state->hot_user_data == clicked){
                                result = lister_call_activate_handler(app, heap, view, state,
                                                                      clicked, true);
                            }
                        }
                        state->hot_user_data = 0;
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            case InputEventKind_MouseWheel:
            {
                Mouse_State mouse = get_mouse_state(app);
                state->scroll.target.y += mouse.wheel;
                if (state->initialized){
                    lister_update_filtered_list(app, view, state);
                }
            }break;
            
            case InputEventKind_MouseMove:
            case InputEventKind_Core:
            {
                lister_update_filtered_list(app, view, state);
            }break;
            
            default:
            {
                handled = false;
            }break;
        }
        
        if (result == ListerActivation_Finished){
            break;
        }
        if (!handled){
            leave_command_input_unhandled(app);
        }
    }
    
    hook = scope_attachment(app, scope, view_render_hook, View_Render_Hook*);
    *hook = 0;
}

////////////////////////////////

static Lister_Handlers
lister_get_default_handlers(void){
    Lister_Handlers handlers = {};
    handlers.write_character = lister__write_string__default;
    handlers.backspace       = lister__backspace_text_field__default;
    handlers.navigate_up     = lister__move_up__default;
    handlers.navigate_down   = lister__move_down__default;
    return(handlers);
}

static Lister_Handlers
lister_get_fixed_list_handlers(void){
    Lister_Handlers handlers = {};
    handlers.navigate_up     = lister__move_up__default;
    handlers.navigate_down   = lister__move_down__default;
    handlers.key_stroke      = lister__key_stroke__fixed_list;
    return(handlers);
}

static void
begin_integrated_lister__with_refresh_handler(Application_Links *app, char *query_string, 
                                              Lister_Handlers handlers,
                                              void *user_data, i32 user_data_size,
                                              View_ID view){
    if (handlers.refresh != 0){
        Heap *heap = &global_heap;
        Lister_State *state = view_get_lister_state(view);
        init_lister_state(app, state, heap);
        lister_first_init(app, &state->lister, user_data, user_data_size);
        lister_set_query(&state->lister, query_string);
        state->lister.data.handlers = handlers;
        handlers.refresh(app, &state->lister);
        lister_update_filtered_list(app, view, state);
        
        lister_input_handling_loop(app, view);
        state->initialized = false;
        linalloc_clear(state->lister.arena);
        
    }
    else{
        Scratch_Block scratch(app);
        List_String_Const_u8 list = {};
        string_list_push(scratch, &list, string_u8_litexpr("ERROR: No refresh handler specified for lister (query_string = \""));
        string_list_push(scratch, &list, SCu8(query_string));
        string_list_push(scratch, &list, string_u8_litexpr("\")\n"));
        String_Const_u8 str = string_list_flatten(scratch, list);
        print_message(app, str);
    }
}

static const i32 default_string_size_estimation = 0;

static i32
lister__get_arena_size_(i32 option_count, i32 user_data_size,
                        i32 estimated_string_space_size){
    i32 arena_size = (user_data_size + 7 + option_count*sizeof(Lister_Node) + estimated_string_space_size);
    return(arena_size);
}

static void
begin_integrated_lister__basic_list(Application_Links *app, char *query_string,
                                    Lister_Activation_Type *activate,
                                    void *user_data, i32 user_data_size,
                                    Lister_Option *options, i32 option_count,
                                    i32 estimated_string_space_size,
                                    View_ID view){
    Heap *heap = &global_heap;
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        lister_add_item(&state->lister, options[i].string, options[i].status, options[i].user_data, 0);
    }
    lister_set_query(&state->lister, query_string);
    state->lister.data.handlers = lister_get_default_handlers();
    state->lister.data.handlers.activate = activate;
    lister_update_filtered_list(app, view, state);
    
    lister_input_handling_loop(app, view);
    state->initialized = false;
    linalloc_clear(state->lister.arena);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Handlers handlers,
                                            void *user_data, i32 user_data_size,
                                            Lister_Fixed_Option *options, i32 option_count,
                                            i32 estimated_string_space_size,
                                            View_ID view){
    Heap *heap = &global_heap;
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        Key_Code code = options[i].key_code;
        void *extra = lister_add_item(&state->lister,
                                      SCu8(options[i].string),
                                      SCu8(options[i].status),
                                      options[i].user_data,
                                      sizeof(code));
        block_copy(extra, &code, sizeof(code));
    }
    lister_set_query(&state->lister, query_string);
    state->lister.data.handlers = handlers;
    state->lister.data.handlers.refresh = 0;
    lister_update_filtered_list(app, view, state);
    
    lister_input_handling_loop(app, view);
    state->initialized = false;
    linalloc_clear(state->lister.arena);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Activation_Type *activate,
                                            void *user_data, i32 user_data_size,
                                            Lister_Fixed_Option *options, i32 option_count,
                                            i32 estimated_string_space_size,
                                            View_ID view){
    Lister_Handlers handlers = lister_get_fixed_list_handlers();
    handlers.activate = activate;
    begin_integrated_lister__with_fixed_options(app, query_string,
                                                handlers, user_data, user_data_size,
                                                options, option_count,
                                                estimated_string_space_size,
                                                view);
}

static void
begin_integrated_lister__theme_list(Application_Links *app, char *query_string,
                                    Lister_Handlers handlers,
                                    void *user_data, i32 user_data_size,
                                    Lister_UI_Option *options, i32 option_count,
                                    i32 estimated_string_space_size,
                                    View_ID view){
    Heap *heap = &global_heap;
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        lister_add_theme_item(&state->lister,
                              SCu8(options[i].string),
                              options[i].index,
                              options[i].user_data, 0);
    }
    lister_set_query(&state->lister, query_string);
    state->lister.data.handlers = handlers;
    state->lister.data.handlers.refresh = 0;
    lister_update_filtered_list(app, view, state);
    
    lister_input_handling_loop(app, view);
    state->initialized = false;
    linalloc_clear(state->lister.arena);
}

static void
begin_integrated_lister__theme_list(Application_Links *app, char *query_string,
                                    Lister_Activation_Type *activate,
                                    void *user_data, i32 user_data_size,
                                    Lister_UI_Option *options, i32 option_count,
                                    i32 estimated_string_space_size,
                                    View_ID view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate;
    begin_integrated_lister__theme_list(app, query_string,
                                        handlers, user_data, user_data_size,
                                        options, option_count,
                                        estimated_string_space_size,
                                        view);
}

////////////////////////////////

static void
generate_all_buffers_list__output_buffer(Application_Links *app, Lister *lister, Buffer_ID buffer){
    Dirty_State dirty = buffer_get_dirty_state(app, buffer);
    String_Const_u8 status = {};
    switch (dirty){
        case DirtyState_UnsavedChanges:  status = string_u8_litexpr("*"); break;
        case DirtyState_UnloadedChanges: status = string_u8_litexpr("!"); break;
        case DirtyState_UnsavedChangesAndUnloadedChanges: status = string_u8_litexpr("*!"); break;
    }
    Scratch_Block scratch(app);
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer);
    lister_add_item(lister, buffer_name, status, IntAsPtr(buffer), 0);
}

static void
generate_all_buffers_list(Application_Links *app, Lister *lister){
    lister_begin_new_item_set(app, lister);
    
    Buffer_ID buffers_currently_being_viewed[16];
    i32 currently_viewed_buffer_count = 0;
    
    // List currently viewed buffers
    {
        for (View_ID view = get_view_next(app, 0, AccessAll);
             view != 0;
             view = get_view_next(app, view, AccessAll)){
            Buffer_ID new_buffer_id = view_get_buffer(app, view, AccessAll);
            for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
                if (new_buffer_id == buffers_currently_being_viewed[i]){
                    goto skip0;
                }
            }
            buffers_currently_being_viewed[currently_viewed_buffer_count++] = new_buffer_id;
            skip0:;
        }
    }
    
    // Regular Buffers
    {
        for (Buffer_ID buffer = get_buffer_next(app, 0, AccessAll);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, AccessAll)){
            for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
                if (buffer == buffers_currently_being_viewed[i]){
                    goto skip1;
                }
            }
            if (!buffer_has_name_with_star(app, buffer)){
                generate_all_buffers_list__output_buffer(app, lister, buffer);
            }
            skip1:;
        }
    }
    // Buffers Starting with *
    {
        for (Buffer_ID buffer = get_buffer_next(app, 0, AccessAll);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, AccessAll)){
            for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
                if (buffer == buffers_currently_being_viewed[i]){
                    goto skip2;
                }
            }
            if (buffer_has_name_with_star(app, buffer)){
                generate_all_buffers_list__output_buffer(app, lister, buffer);
            }
            skip2:;
        }
    }
    // Buffers That Are Open in Views
    for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
        generate_all_buffers_list__output_buffer(app, lister, buffers_currently_being_viewed[i]);
    }
}

static void
generate_hot_directory_file_list(Application_Links *app, Lister *lister){
    Scratch_Block scratch(app);
    
    Temp_Memory temp = begin_temp(lister->arena);
    String_Const_u8 hot = push_hot_directory(app, lister->arena);
    if (!character_is_slash(string_get_character(hot, hot.size - 1))){
        hot = push_u8_stringf(lister->arena, "%.*s/", string_expand(hot));
    }
    lister_set_text_field(lister, hot);
    lister_set_key(lister, string_front_of_path(hot));
    
    File_List file_list = system_get_file_list(scratch, hot);
    end_temp(temp);
    
    File_Info **one_past_last = file_list.infos + file_list.count;
    
    lister_begin_new_item_set(app, lister);
    
    hot = push_hot_directory(app, lister->arena);
    push_align(lister->arena, 8);
    if (hot.str != 0){
        String_Const_u8 empty_string = string_u8_litexpr("");
        Lister_Prealloced_String empty_string_prealloced = lister_prealloced(empty_string);
        for (File_Info **info = file_list.infos;
             info < one_past_last;
             info += 1){
            if (!HasFlag((**info).attributes.flags, FileAttribute_IsDirectory)) continue;
            String_Const_u8 file_name = push_u8_stringf(lister->arena, "%.*s/",
                                                        string_expand((**info).file_name));
            lister_add_item(lister, lister_prealloced(file_name), empty_string_prealloced, file_name.str, 0);
        }
        
        for (File_Info **info = file_list.infos;
             info < one_past_last;
             info += 1){
            if (HasFlag((**info).attributes.flags, FileAttribute_IsDirectory)) continue;
            String_Const_u8 file_name = push_string_copy(lister->arena, (**info).file_name);
            char *is_loaded = "";
            char *status_flag = "";
            
            Buffer_ID buffer = {};
            
            {
                Temp_Memory path_temp = begin_temp(lister->arena);
                List_String_Const_u8 list = {};
                string_list_push(lister->arena, &list, hot);
                string_list_push_overlap(lister->arena, &list, '/', (**info).file_name);
                String_Const_u8 full_file_path = string_list_flatten(lister->arena, list);
                buffer = get_buffer_by_file_name(app, full_file_path, AccessAll);
                end_temp(path_temp);
            }
            
            if (buffer != 0){
                is_loaded = "LOADED";
                Dirty_State dirty = buffer_get_dirty_state(app, buffer);
                switch (dirty){
                    case DirtyState_UnsavedChanges:  status_flag = " *"; break;
                    case DirtyState_UnloadedChanges: status_flag = " !"; break;
                    case DirtyState_UnsavedChangesAndUnloadedChanges: status_flag = " *!"; break;
                }
            }
            String_Const_u8 status = push_u8_stringf(lister->arena, "%s%s", is_loaded, status_flag);
            lister_add_item(lister, lister_prealloced(file_name), lister_prealloced(status), file_name.str, 0);
        }
    }
}

static void
begin_integrated_lister__buffer_list(Application_Links *app, char *query_string, Lister_Activation_Type *activate_procedure,
                                     void *user_data, i32 user_data_size, View_ID target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_all_buffers_list;
    begin_integrated_lister__with_refresh_handler(app, query_string, handlers, user_data, user_data_size, target_view);
}

static void
begin_integrated_lister__file_system_list(Application_Links *app, char *query_string, Lister_Activation_Type *activate_procedure,
                                          void *user_data, i32 user_data_size, View_ID target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_hot_directory_file_list;
    handlers.write_character = lister__write_character__file_path;
    handlers.backspace = lister__backspace_text_field__file_path;
    begin_integrated_lister__with_refresh_handler(app, query_string, handlers, user_data, user_data_size, target_view);
}

////////////////////////////////

enum{
    SureToKill_NULL = 0,
    SureToKill_No = 1,
    SureToKill_Yes = 2,
    SureToKill_Save = 3,
};

static Lister_Activation_Code
activate_confirm_kill(Application_Links *app, Heap *heap, View_ID view, Lister_State *state, String_Const_u8 text_field, void *user_data, b32 clicked){
    i32 behavior = (i32)PtrAsInt(user_data);
    Buffer_ID buffer_id = *(Buffer_ID*)(state->lister.data.user_data);
    switch (behavior){
        case SureToKill_No:
        {}break;
        
        case SureToKill_Yes:
        {
            buffer_kill(app, buffer_id, BufferKill_AlwaysKill);
        }break;
        
        case SureToKill_Save:
        {
            Scratch_Block scratch(app);
            String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
            if (buffer_save(app, buffer_id, file_name, BufferSave_IgnoreDirtyFlag)){
                buffer_kill(app, buffer_id, BufferKill_AlwaysKill);
            }
            else{
                String_Const_u8 str = push_u8_stringf(scratch, "Did not close '%.*s' because it did not successfully save.",
                                                      string_expand(file_name));
                print_message(app, str);
            }
        }break;
    }
    lister_default(app, heap, view, state, ListerActivation_Finished);
    return(ListerActivation_Finished);
}

static void
do_gui_sure_to_kill(Application_Links *app, Buffer_ID buffer, View_ID view){
    Lister_Fixed_Option options[] = {
        {"(N)o"           , "", KeyCode_N, IntAsPtr(SureToKill_No)  },
        {"(Y)es"          , "", KeyCode_Y, IntAsPtr(SureToKill_Yes) },
        {"(S)ave and Kill", "", KeyCode_S, IntAsPtr(SureToKill_Save)},
    };
    i32 option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app, "There are unsaved changes, close anyway?",
                                                activate_confirm_kill, &buffer, sizeof(buffer),
                                                options, option_count, default_string_size_estimation,
                                                view);
}

static Lister_Activation_Code
activate_confirm_close_4coder(Application_Links *app, Heap *heap,
                              View_ID view, Lister_State *state,
                              String_Const_u8 text_field, void *user_data, b32 clicked){
    i32 behavior = (i32)PtrAsInt(user_data);
    switch (behavior){
        case SureToKill_No:
        {}break;
        
        case SureToKill_Yes:
        {
            allow_immediate_close_without_checking_for_changes = true;
            send_exit_signal(app);
        }break;
        
        case SureToKill_Save:
        {
            save_all_dirty_buffers(app);
            allow_immediate_close_without_checking_for_changes = true;
            send_exit_signal(app);
        }break;
    }
    lister_default(app, heap, view, state, ListerActivation_Finished);
    return(ListerActivation_Finished);
}

static void
do_gui_sure_to_close_4coder(Application_Links *app, View_ID view){
    Lister_Fixed_Option options[] = {
        {"(N)o"                , "", KeyCode_N, (void*)SureToKill_No  },
        {"(Y)es"               , "", KeyCode_Y, (void*)SureToKill_Yes },
        {"(S)ave All and Close", "", KeyCode_S, (void*)SureToKill_Save},
    };
    i32 option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app, "There are one or more buffers with unsave changes, close anyway?",
                                                activate_confirm_close_4coder, 0, 0,
                                                options, option_count, default_string_size_estimation,
                                                view);
}

////////////////////////////////

static Lister_Activation_Code
activate_switch_buffer(Application_Links *app, Heap *heap,
                       View_ID view, Lister_State *state,
                       String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(PtrAsInt(user_data));
        view_set_buffer(app, view, buffer_id, SetBuffer_KeepOriginalGUI);
    }
    lister_default(app, heap, view, state, ListerActivation_Finished);
    return(ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    View_ID view = get_active_view(app, AccessAll);
    begin_integrated_lister__buffer_list(app, "Switch:", activate_switch_buffer, 0, 0, view);
}

static Lister_Activation_Code
activate_kill_buffer(Application_Links *app, Heap *heap,
                     View_ID view, struct Lister_State *state,
                     String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, heap, view, state, ListerActivation_Finished);
    if (user_data != 0){
        Buffer_ID buffer = (Buffer_ID)(PtrAsInt(user_data));
        try_buffer_kill(app, buffer, view, 0);
    }
    return(ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    View_ID view = get_active_view(app, AccessAll);
    begin_integrated_lister__buffer_list(app, "Kill:", activate_kill_buffer, 0, 0, view);
}

static Lister_Activation_Code
activate_open_or_new__generic(Application_Links *app, View_ID view,
                              String_Const_u8 path, String_Const_u8 file_name, b32 is_folder,
                              Buffer_Create_Flag flags){
    Lister_Activation_Code result = 0;
    
    if (file_name.size == 0){
#define M "Zero length file_name passed to activate_open_or_new__generic\n"
        print_message(app, string_u8_litexpr(M));
#undef M
        result = ListerActivation_Finished;
    }
    else{
        Scratch_Block scratch(app, Scratch_Share);
        String_Const_u8 full_file_name = {};
        if (character_is_slash(string_get_character(path, path.size - 1))){
            path = string_chop(path, 1);
        }
        full_file_name = push_u8_stringf(scratch, "%.*s/%.*s", string_expand(path), string_expand(file_name));
        if (is_folder){
            set_hot_directory(app, full_file_name);
            result = ListerActivation_ContinueAndRefresh;
        }
        else{
            Buffer_ID buffer = create_buffer(app, full_file_name, flags);
            if (buffer != 0){
                view_set_buffer(app, view, buffer, SetBuffer_KeepOriginalGUI);
            }
            result = ListerActivation_Finished;
        }
    }
    
    return(result);
}

static Lister_Activation_Code
activate_open_or_new(Application_Links *app, Heap *heap,
                     View_ID view, struct Lister_State *state,
                     String_Const_u8 text_field, void *user_data, b32 clicked){
    Lister_Activation_Code result = 0;
    String_Const_u8 file_name = {};
    if (user_data == 0){
        file_name = string_front_of_path(text_field);
    }
    else{
        file_name = SCu8((u8*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String_Const_u8 path = state->lister.data.text_field.string;
        if (!character_is_slash(string_get_character(path, path.size - 1))){
            path = string_remove_last_folder(path);
        }
        b32 is_folder = (character_is_slash(string_get_character(file_name, file_name.size - 1)) &&
                         user_data != 0);
        Buffer_Create_Flag flags = 0;
        result = activate_open_or_new__generic(app, view, path, file_name, is_folder, flags);
    }
    lister_default(app, heap, view, state, result);
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    View_ID view = get_active_view(app, AccessAll);
    begin_integrated_lister__file_system_list(app, "Open:", activate_open_or_new, 0, 0, view);
}

static Lister_Activation_Code
activate_new(Application_Links *app, Heap *heap,
             View_ID view, struct Lister_State *state,
             String_Const_u8 text_field, void *user_data, b32 clicked){
    Lister_Activation_Code result = 0;
    String_Const_u8 file_name = string_front_of_path(text_field);
    if (user_data != 0){
        String_Const_u8 item_name = SCu8((u8*)user_data);
        if (item_name.str[item_name.size - 1] == '/'){
            file_name = item_name;
        }
        else if (clicked){
            file_name = item_name;
        }
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String_Const_u8 path = state->lister.data.text_field.string;
        if (character_is_slash(string_get_character(path, path.size - 1))){
            path = string_remove_last_folder(path);
        }
        b32 is_folder = (character_is_slash(string_get_character(file_name, file_name.size - 1)) &&
                         user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_AlwaysNew;
        result = activate_open_or_new__generic(app, view, path, file_name, is_folder, flags);
    }
    lister_default(app, heap, view, state, result);
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    View_ID view = get_active_view(app, AccessAll);
    begin_integrated_lister__file_system_list(app, "New:", activate_new, 0, 0, view);
}

static Lister_Activation_Code
activate_open(Application_Links *app, Heap *heap,
              View_ID view, struct Lister_State *state,
              String_Const_u8 text_field, void *user_data, b32 clicked){
    Lister_Activation_Code result = 0;
    String_Const_u8 file_name = {};
    if (user_data != 0){
        file_name = SCu8((u8*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String_Const_u8 path = state->lister.data.text_field.string;
        if (!character_is_slash(string_get_character(path, path.size - 1))){
            path = string_remove_last_folder(path);
        }
        b32 is_folder = (character_is_slash(string_get_character(file_name, file_name.size - 1)) &&
                         user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_NeverNew;
        result = activate_open_or_new__generic(app, view, path, file_name, is_folder, flags);
    }
    lister_default(app, heap, view, state, result);
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    View_ID view = get_active_view(app, AccessAll);
    begin_integrated_lister__file_system_list(app, "Open:", activate_open, 0, 0, view);
}

#if 0
static void
activate_select_theme(Application_Links *app, Partition *scratch, Heap *heap,
                      View_ID view, struct Lister_State *state,
                      String_Const_u8 text_field, void *user_data, b32 clicked){
    change_theme_by_index(app, (i32)PtrAsInt(user_data));
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(open_color_tweaker)
CUSTOM_DOC("Opens the 4coder theme selector list.")
{
    Partition *scratch = &global_part;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    View_ID view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, view);
    i32 theme_count = get_theme_count(app);
    Lister_UI_Option *options = push_array(scratch, Lister_UI_Option, theme_count);
    for (i32 i = 0; i < theme_count; i += 1){
        String name = get_theme_name(app, scratch, i);
        options[i].string = name.str;
        options[i].index = i;
        options[i].user_data = IntAsPtr(i);
    }
    begin_integrated_lister__theme_list(app,
                                        "Select a theme:",
                                        activate_select_theme, 0, 0,
                                        options, theme_count,
                                        default_string_size_estimation,
                                        view);
    
    end_temp_memory(temp);
}
#endif

////////////////////////////////

static Lister_Activation_Code
activate_command(Application_Links *app, Heap *heap,
                 View_ID view, Lister_State *state,
                 String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, heap, view, state, ListerActivation_Finished);
    if (user_data != 0){
        Custom_Command_Function *command = (Custom_Command_Function*)user_data;
        command(app);
    }
    return(ListerActivation_Finished);
}

static void
launch_custom_command_lister(Application_Links *app, i32 *command_ids, i32 command_id_count){
    if (command_ids == 0){
        command_id_count = command_one_past_last_id;
    }
    
    Scratch_Block scratch(app, Scratch_Share);
    View_ID view = get_active_view(app, AccessAll);
    Lister_Option *options = push_array(scratch, Lister_Option, command_id_count);
    for (i32 i = 0; i < command_id_count; i += 1){
        i32 j = i;
        if (command_ids != 0){
            j = command_ids[i];
        }
        j = clamp(0, j, command_one_past_last_id);
        options[i].string = SCu8(fcoder_metacmd_table[j].name);
        options[i].status = SCu8(fcoder_metacmd_table[j].description);
        options[i].user_data = (void*)fcoder_metacmd_table[j].proc;
    }
    begin_integrated_lister__basic_list(app, "Command:", activate_command, 0, 0, options, command_id_count, 0, view);
}

CUSTOM_COMMAND_SIG(command_lister)
CUSTOM_DOC("Opens an interactive list of all registered commands.")
{
    launch_custom_command_lister(app, 0, 0);
}

// BOTTOM

