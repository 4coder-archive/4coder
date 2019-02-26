/*
4coder_lists.cpp - List helpers and list commands
such as open file, switch buffer, or kill buffer.
*/

// TOP

CUSTOM_COMMAND_SIG(lister__quit)
CUSTOM_DOC("A lister mode command that quits the list without executing any actions.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
}

CUSTOM_COMMAND_SIG(lister__activate)
CUSTOM_DOC("A lister mode command that activates the list's action on the highlighted item.")
{
    Partition *scratch = &global_part;
    Heap *heap = &global_heap;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        void *user_data = 0;
        if (0 <= state->raw_item_index && state->raw_item_index < state->lister.data.options.count){
            user_data = lister_get_user_data(&state->lister.data, state->raw_item_index);
        }
        lister_call_activate_handler(app, scratch, heap, &view, state, user_data, false);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character)
CUSTOM_DOC("A lister mode command that dispatches to the lister's write character handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.data.handlers.write_character != 0){
        state->lister.data.handlers.write_character(app);
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field)
CUSTOM_DOC("A lister mode command that dispatches to the lister's backspace text field handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.data.handlers.backspace != 0){
        state->lister.data.handlers.backspace(app);
    }
}

CUSTOM_COMMAND_SIG(lister__move_up)
CUSTOM_DOC("A lister mode command that dispatches to the lister's navigate up handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.data.handlers.navigate_up != 0){
        state->lister.data.handlers.navigate_up(app);
    }
}

CUSTOM_COMMAND_SIG(lister__move_down)
CUSTOM_DOC("A lister mode command that dispatches to the lister's navigate down handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.data.handlers.navigate_down != 0){
        state->lister.data.handlers.navigate_down(app);
    }
}

CUSTOM_COMMAND_SIG(lister__wheel_scroll)
CUSTOM_DOC("A lister mode command that scrolls the list in response to the mouse wheel.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    GUI_Scroll_Vars scroll = view.scroll_vars;
    Mouse_State mouse = get_mouse_state(app);
    scroll.target_y += mouse.wheel;
    view_set_scroll(app, &view, scroll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__mouse_press)
CUSTOM_DOC("A lister mode command that beings a click interaction with a list item under the mouse.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        UI_Item clicked = lister_get_clicked_item(app, view.view_id, scratch);
        state->hot_user_data = clicked.user_data;
    }
}

CUSTOM_COMMAND_SIG(lister__mouse_release)
CUSTOM_DOC("A lister mode command that ends a click interaction with a list item under the mouse, possibly activating it.")
{
    Partition *scratch = &global_part;
    Heap *heap = &global_heap;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized && state->hot_user_data != 0){
        UI_Item clicked = lister_get_clicked_item(app, view.view_id, scratch);
        if (state->hot_user_data == clicked.user_data){
            lister_call_activate_handler(app, scratch, heap, &view, state, clicked.user_data, true);
        }
    }
    state->hot_user_data = 0;
}

CUSTOM_COMMAND_SIG(lister__repaint)
CUSTOM_DOC("A lister mode command that updates the lists UI data.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__default)
CUSTOM_DOC("A lister mode command that inserts a new character to the text field.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        uint8_t character[4];
        u32 length = to_writable_character(in, character);
        if (length > 0){
            append(&state->lister.data.text_field, make_string(character, length));
            append(&state->lister.data.key_string, make_string(character, length));
            state->item_index = 0;
            view_zero_scroll(app, &view);
            lister_update_ui(app, scratch, &view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field__default)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        backspace_utf8(&state->lister.data.text_field);
        backspace_utf8(&state->lister.data.key_string);
        state->item_index = 0;
        view_zero_scroll(app, &view);
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__move_up__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one up in the list.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        state->item_index = state->item_index - 1;
        if (state->item_index < 0){
            state->item_index = state->item_count_after_filter - 1;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__move_down__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one down in the list.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        state->item_index = state->item_index + 1;
        if (state->item_index > state->item_count_after_filter - 1){
            state->item_index = 0;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__file_path)
CUSTOM_DOC("A lister mode command that inserts a character into the text field of a file system list.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        uint8_t character[4];
        u32 length = to_writable_character(in, character);
        if (length > 0){
            append(&state->lister.data.text_field, make_string(character, length));
            copy(&state->lister.data.key_string, front_of_directory(state->lister.data.text_field));
            if (character[0] == '/' || character[0] == '\\'){
                String new_hot = state->lister.data.text_field;
                directory_set_hot(app, new_hot.str, new_hot.size);
                lister_call_refresh_handler(app, &state->lister);
            }
            state->item_index = 0;
            view_zero_scroll(app, &view);
            lister_update_ui(app, scratch, &view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field__file_path)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field of a file system list.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        if (state->lister.data.text_field.size > 0){
            char last_char = state->lister.data.text_field.str[state->lister.data.text_field.size - 1];
            backspace_utf8(&state->lister.data.text_field);
            if (last_char == '/' || last_char == '\\'){
                User_Input input = get_command_input(app);
                b32 is_modified =
                    input.key.modifiers[MDFR_SHIFT_INDEX] ||
                    input.key.modifiers[MDFR_CONTROL_INDEX] ||
                    input.key.modifiers[MDFR_ALT_INDEX] ||
                    input.key.modifiers[MDFR_COMMAND_INDEX];
                String new_hot = path_of_directory(state->lister.data.text_field);
                if (!is_modified){
                    state->lister.data.text_field.size = new_hot.size;
                }
                directory_set_hot(app, new_hot.str, new_hot.size);
                lister_call_refresh_handler(app, &state->lister);
            }
            else{
                copy(&state->lister.data.key_string, front_of_directory(state->lister.data.text_field));
            }
            
            state->item_index = 0;
            view_zero_scroll(app, &view);
            lister_update_ui(app, scratch, &view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__fixed_list)
CUSTOM_DOC("A lister mode command that handles input for the fixed sure to kill list.")
{
    Partition *scratch = &global_part;
    Heap *heap = &global_heap;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        uint8_t character[4];
        u32 length = to_writable_character(in, character);
        if (length > 0){
            void *user_data = 0;
            b32 did_shortcut_key = false;
            for (Lister_Node *node = state->lister.data.options.first;
                 node != 0;
                 node = node->next){
                char *hotkeys = (char*)(node + 1);
                if (has_substr(hotkeys, make_string(character, length))){
                    user_data = node->user_data;
                    did_shortcut_key = true;
                    break;
                }
            }
            if (did_shortcut_key){
                lister_call_activate_handler(app, scratch, heap,
                                             &view, state,
                                             user_data, false);
            }
        }
    }
}

////////////////////////////////

static Lister_Handlers
lister_get_default_handlers(void){
    Lister_Handlers handlers = {};
    handlers.write_character = lister__write_character__default;
    handlers.backspace       = lister__backspace_text_field__default;
    handlers.navigate_up     = lister__move_up__default;
    handlers.navigate_down   = lister__move_down__default;
    return(handlers);
}

static Lister_Handlers
lister_get_fixed_list_handlers(void){
    Lister_Handlers handlers = {};
    handlers.write_character = lister__write_character__fixed_list;
    handlers.backspace       = 0;
    handlers.navigate_up     = lister__move_up__default;
    handlers.navigate_down   = lister__move_down__default;
    return(handlers);
}

static void
begin_integrated_lister__with_refresh_handler(Application_Links *app, char *query_string, 
                                              Lister_Handlers handlers,
                                              void *user_data, i32 user_data_size,
                                              View_Summary *view){
    if (handlers.refresh != 0){
        Partition *scratch = &global_part;
        Heap *heap = &global_heap;
        view_begin_ui_mode(app, view);
        view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
        Lister_State *state = view_get_lister_state(view);
        init_lister_state(app, state, heap);
        lister_first_init(app, &state->lister, user_data, user_data_size);
        lister_set_query_string(&state->lister.data, query_string);
        state->lister.data.handlers = handlers;
        handlers.refresh(app, &state->lister);
        lister_update_ui(app, scratch, view, state);
    }
    else{
        char space[256];
        String str = make_fixed_width_string(space);
        append(&str, "ERROR: No refresh handler specified for lister (query_string = \"");
        append(&str, query_string);
        append(&str, "\")\n");
        print_message(app, str.str, str.size);
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
                                    Lister_Activation_Function_Type *activate,
                                    void *user_data, i32 user_data_size,
                                    Lister_Option *options, i32 option_count,
                                    i32 estimated_string_space_size,
                                    View_Summary *view){
    Partition *scratch = &global_part;
    Heap *heap = &global_heap;
    view_begin_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        lister_add_item(&state->lister, options[i].string, options[i].status, options[i].user_data, 0);
    }
    lister_set_query_string(&state->lister.data, query_string);
    state->lister.data.handlers = lister_get_default_handlers();
    state->lister.data.handlers.activate = activate;
    lister_update_ui(app, scratch, view, state);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Handlers handlers,
                                            void *user_data, i32 user_data_size,
                                            Lister_Fixed_Option *options, i32 option_count,
                                            i32 estimated_string_space_size,
                                            View_Summary *view){
    Partition *scratch = &global_part;
    Heap *heap = &global_heap;
    view_begin_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        char *shortcut_chars = options[i].shortcut_chars;
        i32 shortcut_chars_length = str_size(shortcut_chars);
        void *extra = lister_add_item(&state->lister,
                                      make_string_slowly(options[i].string),
                                      make_string_slowly(options[i].status),
                                      options[i].user_data,
                                      shortcut_chars_length + 1);
        memcpy(extra, shortcut_chars, shortcut_chars_length + 1);
    }
    lister_set_query_string(&state->lister.data, query_string);
    state->lister.data.handlers = handlers;
    state->lister.data.handlers.refresh = 0;
    lister_update_ui(app, scratch, view, state);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Activation_Function_Type *activate,
                                            void *user_data, i32 user_data_size,
                                            Lister_Fixed_Option *options, i32 option_count,
                                            i32 estimated_string_space_size,
                                            View_Summary *view){
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
                                    View_Summary *view){
    Partition *scratch = &global_part;
    Heap *heap = &global_heap;
    view_begin_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    state->lister.data.theme_list = true;
    for (i32 i = 0; i < option_count; i += 1){
        lister_add_theme_item(&state->lister,
                              make_string_slowly(options[i].string),
                              options[i].index,
                              options[i].user_data, 0);
    }
    lister_set_query_string(&state->lister.data, query_string);
    state->lister.data.handlers = handlers;
    state->lister.data.handlers.refresh = 0;
    lister_update_ui(app, scratch, view, state);
}

static void
begin_integrated_lister__theme_list(Application_Links *app, char *query_string,
                                    Lister_Activation_Function_Type *activate,
                                    void *user_data, i32 user_data_size,
                                    Lister_UI_Option *options, i32 option_count,
                                    i32 estimated_string_space_size,
                                    View_Summary *view){
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
generate_all_buffers_list__output_buffer(Lister *lister, Buffer_Summary buffer){
    String status = {};
    switch (buffer.dirty){
        case DirtyState_UnsavedChanges:  status = make_lit_string("*"); break;
        case DirtyState_UnloadedChanges: status = make_lit_string("!"); break;
        case DirtyState_UnsavedChangesAndUnloadedChanges: status = make_lit_string("*!"); break;
    }
    String buffer_name = make_string(buffer.buffer_name, buffer.buffer_name_len);
    lister_add_item(lister, buffer_name, status, IntAsPtr(buffer.buffer_id), 0);
}

static void
generate_all_buffers_list(Application_Links *app, Lister *lister){
    i32 buffer_count = get_buffer_count(app);
    i32 memory_size = 0;
    memory_size += buffer_count*(sizeof(Lister_Node) + 3);
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        memory_size += buffer.buffer_name_len;
    }
    
    lister_begin_new_item_set(app, lister, memory_size);
    
    Buffer_ID buffers_currently_being_viewed[16];
    i32 currently_viewed_buffer_count = 0;
    
    // List currently viewed buffers
    for (View_Summary view = get_view_first(app, AccessAll);
         view.exists;
         get_view_next(app, &view, AccessAll)){
        Buffer_ID new_buffer_id = view.buffer_id;
        for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
            if (new_buffer_id == buffers_currently_being_viewed[i]){
                goto skip0;
            }
        }
        buffers_currently_being_viewed[currently_viewed_buffer_count++] = new_buffer_id;
        skip0:;
    }
    
    // Regular Buffers
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
            if (buffer.buffer_id == buffers_currently_being_viewed[i]){
                goto skip1;
            }
        }
        if (buffer.buffer_name_len == 0 || buffer.buffer_name[0] != '*'){
            generate_all_buffers_list__output_buffer(lister, buffer);
        }
        skip1:;
    }
    // Buffers Starting with *
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
            if (buffer.buffer_id == buffers_currently_being_viewed[i]){
                goto skip2;
            }
        }
        if (buffer.buffer_name_len != 0 && buffer.buffer_name[0] == '*'){
            generate_all_buffers_list__output_buffer(lister, buffer);
        }
        skip2:;
    }
    // Buffers That Are Open in Views
    for (i32 i = 0; i < currently_viewed_buffer_count; i += 1){
        Buffer_Summary buffer = get_buffer(app, buffers_currently_being_viewed[i], AccessAll);
        generate_all_buffers_list__output_buffer(lister, buffer);
    }
}

static void
generate_hot_directory_file_list(Application_Links *app, Lister *lister){
    Temp_Memory_Arena temp = begin_temp_memory(&lister->arena);
    String hot = get_hot_directory(app, &lister->arena);
    if (hot.size > 0 && hot.str[hot.size - 1] != '/' && hot.str[hot.size - 1] != '\\'){
        if (push_array(&lister->arena, char, 1) != 0){
            hot.memory_size += 1;
            append_s_char(&hot, '/');
        }
    }
    lister_set_text_field_string(&lister->data, hot);
    lister_set_key_string(&lister->data, front_of_directory(hot));
    
    File_List file_list = get_file_list(app, hot.str, hot.size);
    end_temp_memory(temp);
    
    File_Info *one_past_last = file_list.infos + file_list.count;
    
    i32 memory_requirement = 0;
    memory_requirement += lister->data.user_data_size;
    memory_requirement += file_list.count*(sizeof(Lister_Node) + 10);
    memory_requirement += hot.size + 2;
    for (File_Info *info = file_list.infos;
         info < one_past_last;
         info += 1){
        memory_requirement += info->filename_len;
    }
    
    lister_begin_new_item_set(app, lister, memory_requirement);
    
    hot = get_hot_directory(app, &lister->arena);
    push_align(&lister->arena, 8);
    if (hot.str != 0){
        String empty_string = make_lit_string("");
        Lister_Prealloced_String empty_string_prealloced = {empty_string};
        for (File_Info *info = file_list.infos;
             info < one_past_last;
             info += 1){
            if (!info->folder) continue;
            String file_name = build_string(arena_use_as_part(&lister->arena, info->filename_len + 1),
                                            make_string(info->filename, info->filename_len), "/", "");
            lister_add_item(lister, lister_prealloced(file_name), empty_string_prealloced, file_name.str, 0);
        }
        
        for (File_Info *info = file_list.infos;
             info < one_past_last;
             info += 1){
            if (info->folder) continue;
            String file_name = string_push_copy(&lister->arena, make_string(info->filename, info->filename_len));
            char *is_loaded = "";
            char *status_flag = "";
            
            
            Buffer_Summary buffer = {};
            
            {
                Temp_Memory_Arena path_temp = begin_temp_memory(&lister->arena);
                String full_file_path = {};
                full_file_path.size = 0;
                full_file_path.memory_size = hot.size + 1 + info->filename_len + 1;
                full_file_path.str = push_array(&lister->arena, char, full_file_path.memory_size);
                append(&full_file_path, hot);
                if (full_file_path.size == 0 ||
                    char_is_slash(full_file_path.str[full_file_path.size - 1])){
                    append(&full_file_path, "/");
                }
                append(&full_file_path, make_string(info->filename, info->filename_len));
                buffer = get_buffer_by_file_name(app, full_file_path.str, full_file_path.size, AccessAll);
                end_temp_memory(path_temp);
            }
            
            if (buffer.exists){
                is_loaded = "LOADED";
                switch (buffer.dirty){
                    case DirtyState_UnsavedChanges:  status_flag = " *"; break;
                    case DirtyState_UnloadedChanges: status_flag = " !"; break;
                    case DirtyState_UnsavedChangesAndUnloadedChanges: status_flag = " *!"; break;
                }
            }
            i32 more_than_enough_memory = 32;
            String status = build_string(arena_use_as_part(&lister->arena, more_than_enough_memory), is_loaded, status_flag, "");
            lister_add_item(lister, lister_prealloced(file_name), lister_prealloced(status), file_name.str, 0);
        }
    }
    
    free_file_list(app, file_list);
}

static void
begin_integrated_lister__buffer_list(Application_Links *app, char *query_string, Lister_Activation_Function_Type *activate_procedure,
                                     void *user_data, i32 user_data_size, View_Summary *target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_all_buffers_list;
    begin_integrated_lister__with_refresh_handler(app, query_string, handlers, user_data, user_data_size, target_view);
}

static void
begin_integrated_lister__file_system_list(Application_Links *app, char *query_string, Lister_Activation_Function_Type *activate_procedure,
                                          void *user_data, i32 user_data_size, View_Summary *target_view){
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

static void
activate_confirm_kill(Application_Links *app, Partition *scratch, Heap *heap, View_Summary *view, Lister_State *state,
                      String text_field, void *user_data, b32 clicked){
    i32 behavior = (i32)PtrAsInt(user_data);
    Buffer_ID buffer_id = *(Buffer_ID*)(state->lister.data.user_data);
    switch (behavior){
        case SureToKill_No:
        {}break;
        
        case SureToKill_Yes:
        {
            kill_buffer(app, buffer_identifier(buffer_id), BufferKill_AlwaysKill);
        }break;
        
        case SureToKill_Save:
        {
            Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
            if (save_buffer(app, &buffer, buffer.file_name, buffer.file_name_len, BufferSave_IgnoreDirtyFlag)){
                kill_buffer(app, buffer_identifier(buffer_id), BufferKill_AlwaysKill);
            }
            else{
                char space[256];
                String str = make_fixed_width_string(space);
                append(&str, "Did not close '");
                append(&str, make_string(buffer.file_name, buffer.file_name_len));
                append(&str, "' because it did not successfully save.\n");
                print_message(app, str.str, str.size);
            }
        }break;
    }
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
}

static void
do_gui_sure_to_kill(Application_Links *app, Buffer_Summary *buffer, View_Summary *view){
    Lister_Fixed_Option options[] = {
        {"(N)o"           , "", "Nn", IntAsPtr(SureToKill_No)  },
        {"(Y)es"          , "", "Yy", IntAsPtr(SureToKill_Yes) },
        {"(S)ave and Kill", "", "Ss", IntAsPtr(SureToKill_Save)},
    };
    i32 option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app, "There are unsaved changes, close anyway?",
                                                activate_confirm_kill,
                                                &buffer->buffer_id, sizeof(buffer->buffer_id),
                                                options, option_count, default_string_size_estimation,
                                                view);
}

static void
activate_confirm_close_4coder(Application_Links *app, Partition *scratch, Heap *heap,
                              View_Summary *view, Lister_State *state,
                              String text_field, void *user_data, b32 clicked){
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
    
}

static void
do_gui_sure_to_close_4coder(Application_Links *app, View_Summary *view){
    Lister_Fixed_Option options[] = {
        {"(N)o"                , "", "Nn", (void*)SureToKill_No  },
        {"(Y)es"               , "", "Yy", (void*)SureToKill_Yes },
        {"(S)ave All and Close", "", "Ss", (void*)SureToKill_Save},
    };
    i32 option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app,
                                                "There are one or more buffers with unsave changes, close anyway?",
                                                activate_confirm_close_4coder,
                                                0, 0,
                                                options, option_count,
                                                default_string_size_estimation,
                                                view);
}

////////////////////////////////

static void
activate_switch_buffer(Application_Links *app, Partition *scratch, Heap *heap,
                       View_Summary *view, Lister_State *state,
                       String text_field, void *user_data, b32 activated_by_mouse){
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(PtrAsInt(user_data));
        view_set_buffer(app, view, buffer_id, SetBuffer_KeepOriginalGUI);
    }
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    begin_integrated_lister__buffer_list(app, "Switch:", activate_switch_buffer, 0, 0, &view);
}

static void
activate_kill_buffer(Application_Links *app, Partition *scratch, Heap *heap,
                     View_Summary *view, struct Lister_State *state,
                     String text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(PtrAsInt(user_data));
        kill_buffer(app, buffer_identifier(buffer_id), view->view_id, 0);
    }
}

CUSTOM_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    begin_integrated_lister__buffer_list(app, "Kill:", activate_kill_buffer, 0, 0, &view);
}

static Lister_Activation_Code
activate_open_or_new__generic(Application_Links *app, Partition *scratch, View_Summary *view,
                              String path, String file_name, b32 is_folder,
                              Buffer_Create_Flag flags){
    Lister_Activation_Code result = 0;
    
    if (file_name.size == 0){
        char msg[] = "Zero length file_name passed to activate_open_or_new__generic\n";
        print_message(app, msg, sizeof(msg) - 1);
        result = ListerActivation_Finished;
    }
    else{
        Temp_Memory temp = begin_temp_memory(scratch);
        String full_file_name = {};
        if (path.size == 0 || !char_is_slash(path.str[path.size - 1])){
            full_file_name = build_string(scratch, path, "/", file_name);
        }
        else{
            full_file_name = build_string(scratch, path, "", file_name);
        }
        if (is_folder){
            directory_set_hot(app, full_file_name.str, full_file_name.size);
            result = ListerActivation_ContinueAndRefresh;
        }
        else{
            Buffer_Summary buffer = create_buffer(app, full_file_name.str, full_file_name.size, flags);
            if (buffer.exists){
                view_set_buffer(app, view, buffer.buffer_id, SetBuffer_KeepOriginalGUI);
            }
            result = ListerActivation_Finished;
        }
        end_temp_memory(temp);
    }
    
    return(result);
}

static void
activate_open_or_new(Application_Links *app, Partition *scratch, Heap *heap,
                     View_Summary *view, struct Lister_State *state,
                     String text_field, void *user_data, b32 clicked){
    Lister_Activation_Code result = 0;
    String file_name = {};
    if (user_data == 0){
        file_name = front_of_directory(text_field);
    }
    else{
        file_name = make_string_slowly((char*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String path = state->lister.data.text_field;
        if (path.size > 0 && !char_is_slash(path.str[path.size - 1])){
            path = path_of_directory(path);
        }
        b32 is_folder = (file_name.str[file_name.size - 1] == '/' && user_data != 0);
        Buffer_Create_Flag flags = 0;
        result = activate_open_or_new__generic(app, scratch, view, path, file_name, is_folder, flags);
    }
    lister_default(app, scratch, heap, view, state, result);
}

CUSTOM_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    begin_integrated_lister__file_system_list(app, "Open:", activate_open_or_new, 0, 0, &view);
}

static void
activate_new(Application_Links *app, Partition *scratch, Heap *heap,
             View_Summary *view, struct Lister_State *state,
             String text_field, void *user_data, b32 clicked){
    Lister_Activation_Code result = 0;
    String file_name = front_of_directory(text_field);
    if (user_data != 0){
        String item_name = make_string_slowly((char*)user_data);
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
        String path = state->lister.data.text_field;
        if (path.size > 0 && !char_is_slash(path.str[path.size - 1])){
            path = path_of_directory(path);
        }
        b32 is_folder = (file_name.str[file_name.size - 1] == '/' && user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_AlwaysNew;
        result = activate_open_or_new__generic(app, scratch, view, path, file_name, is_folder, flags);
    }
    lister_default(app, scratch, heap, view, state, result);
}

CUSTOM_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    begin_integrated_lister__file_system_list(app, "New:", activate_new, 0, 0, &view);
}

static void
activate_open(Application_Links *app, Partition *scratch, Heap *heap,
              View_Summary *view, struct Lister_State *state,
              String text_field, void *user_data, b32 clicked){
    Lister_Activation_Code result = 0;
    String file_name = {};
    if (user_data != 0){
        file_name = make_string_slowly((char*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String path = state->lister.data.text_field;
        if (path.size > 0 && !char_is_slash(path.str[path.size - 1])){
            path = path_of_directory(path);
        }
        b32 is_folder = (file_name.str[file_name.size - 1] == '/' && user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_NeverNew;
        result = activate_open_or_new__generic(app, scratch, view, path, file_name, is_folder, flags);
    }
    lister_default(app, scratch, heap, view, state, result);
}

CUSTOM_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    begin_integrated_lister__file_system_list(app, "Open:", activate_open, 0, 0, &view);
}

#if 0
static void
activate_select_theme(Application_Links *app, Partition *scratch, Heap *heap,
                      View_Summary *view, struct Lister_State *state,
                      String text_field, void *user_data, b32 clicked){
    change_theme_by_index(app, (i32)PtrAsInt(user_data));
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(open_color_tweaker)
CUSTOM_DOC("Opens the 4coder theme selector list.")
{
    Partition *scratch = &global_part;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
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
                                        &view);
    
    end_temp_memory(temp);
}
#endif

////////////////////////////////

static void
activate_command(Application_Links *app, Partition *scratch, Heap *heap,
                 View_Summary *view, Lister_State *state,
                 String text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
    if (user_data != 0){
        Custom_Command_Function *command = (Custom_Command_Function*)user_data;
        command(app);
    }
}

CUSTOM_COMMAND_SIG(command_lister)
CUSTOM_DOC("Opens an interactive list of all registered commands.")
{
    Partition *arena = &global_part;
    
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    Temp_Memory temp = begin_temp_memory(arena);
    i32 option_count = command_one_past_last_id;
    Lister_Option *options = push_array(arena, Lister_Option, option_count);
    for (i32 i = 0; i < command_one_past_last_id; i += 1){
        options[i].string = make_string_slowly(fcoder_metacmd_table[i].name);
        options[i].status = make_string_slowly(fcoder_metacmd_table[i].description);
        options[i].user_data = (void*)fcoder_metacmd_table[i].proc;
    }
    begin_integrated_lister__basic_list(app, "Command:", activate_command, 0, 0, options, option_count, 0, &view);
    end_temp_memory(temp);
}

// BOTTOM

