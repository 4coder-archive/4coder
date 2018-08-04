/*
4coder_lists.cpp - List helpers and list commands
such as open file, switch buffer, or kill buffer.
*/

// TOP

CUSTOM_COMMAND_SIG(list_mode__quit)
CUSTOM_DOC("A list mode command that quits the list without executing any actions.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    state->initialized = false;
    view_end_ui_mode(app, &view);
}

CUSTOM_COMMAND_SIG(list_mode__activate)
CUSTOM_DOC("A list mode command that activates the list's action on the highlighted item.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        void *user_data = 0;
        if (0 <= state->raw_item_index && state->raw_item_index < state->lister.options.count){
            user_data = lister_get_user_data(&state->lister, state->raw_item_index);
        }
        lister_call_activate_handler(app, &global_part, &global_general, &view,
                                     state, user_data, false);
    }
}

CUSTOM_COMMAND_SIG(list_mode__write_character__default)
CUSTOM_DOC("A list mode command that inserts a new character to the text field.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        uint8_t character[4];
        uint32_t length = to_writable_character(in, character);
        if (length > 0){
            append(&state->lister.text_field, make_string(character, length));
            append(&state->lister.key_string, make_string(character, length));
            state->item_index = 0;
            view_zero_scroll(app, &view);
            lister_update_ui(app, scratch, &view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(list_mode__backspace_text_field__default)
CUSTOM_DOC("A list mode command that backspaces one character from the text field.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        backspace_utf8(&state->lister.text_field);
        backspace_utf8(&state->lister.key_string);
        state->item_index = 0;
        view_zero_scroll(app, &view);
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(list_mode__move_up__default)
CUSTOM_DOC("A list mode command that moves the highlighted item one up in the list.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        state->item_index = state->item_index - 1;
        if (state->item_index < 0){
            state->item_index = state->option_item_count - 1;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(list_mode__move_down__default)
CUSTOM_DOC("A list mode command that moves the highlighted item one down in the list.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        state->item_index = state->item_index + 1;
        if (state->item_index > state->option_item_count - 1){
            state->item_index = 0;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_ui(app, scratch, &view, state);
    }
}

CUSTOM_COMMAND_SIG(list_mode__write_character__file_path)
CUSTOM_DOC("A list mode command that inserts a new character to the text field.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        uint8_t character[4];
        uint32_t length = to_writable_character(in, character);
        if (length > 0){
            append(&state->lister.text_field, make_string(character, length));
            copy(&state->lister.key_string, front_of_directory(state->lister.text_field));
            if (character[0] == '/' || character[0] == '\\'){
                String new_hot = state->lister.text_field;
                directory_set_hot(app, new_hot.str, new_hot.size);
                lister_call_refresh_handler(app, &state->arena, &state->lister);
            }
            state->item_index = 0;
            view_zero_scroll(app, &view);
            lister_update_ui(app, scratch, &view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(list_mode__backspace_text_field__file_path)
CUSTOM_DOC("A list mode command that backspaces one character from the text field.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        if (state->lister.text_field.size > 0){
            char last_char = state->lister.text_field.str[state->lister.text_field.size - 1];
            backspace_utf8(&state->lister.text_field);
            if (last_char == '/' || last_char == '\\'){
                User_Input input = get_command_input(app);
                bool32 is_modified =
                    input.key.modifiers[MDFR_SHIFT_INDEX] ||
                    input.key.modifiers[MDFR_CONTROL_INDEX] ||
                    input.key.modifiers[MDFR_ALT_INDEX] ||
                    input.key.modifiers[MDFR_COMMAND_INDEX];
                String new_hot = path_of_directory(state->lister.text_field);
                if (!is_modified){
                    state->lister.text_field.size = new_hot.size;
                }
                directory_set_hot(app, new_hot.str, new_hot.size);
                lister_call_refresh_handler(app, &state->arena, &state->lister);
            }
            else{
                copy(&state->lister.key_string, front_of_directory(state->lister.text_field));
            }
            
            state->item_index = 0;
            view_zero_scroll(app, &view);
            lister_update_ui(app, scratch, &view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(list_mode__write_character)
CUSTOM_DOC("A list mode command that inserts a new character to the text field.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.write_character != 0){
        state->lister.write_character(app);
    }
}

CUSTOM_COMMAND_SIG(list_mode__backspace_text_field)
CUSTOM_DOC("A list mode command that backspaces one character from the text field.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.backspace != 0){
        state->lister.backspace(app);
    }
}

CUSTOM_COMMAND_SIG(list_mode__move_up)
CUSTOM_DOC("A list mode command that moves the highlighted item one up in the list.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.navigate_up != 0){
        state->lister.navigate_up(app);
    }
}

CUSTOM_COMMAND_SIG(list_mode__move_down)
CUSTOM_DOC("A list mode command that moves the highlighted item one down in the list.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.navigate_down != 0){
        state->lister.navigate_down(app);
    }
}

CUSTOM_COMMAND_SIG(list_mode__wheel_scroll)
CUSTOM_DOC("A list mode command that scrolls the list in response to the mouse wheel.")
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

CUSTOM_COMMAND_SIG(list_mode__mouse_press)
CUSTOM_DOC("A list mode command that beings a click interaction with a list item under the mouse.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        UI_Item clicked = lister_get_clicked_item(app, &view, scratch);
        state->hot_user_data = clicked.user_data;
    }
}

CUSTOM_COMMAND_SIG(list_mode__mouse_release)
CUSTOM_DOC("A list mode command that ends a click interaction with a list item under the mouse, possibly activating it.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized && state->hot_user_data != 0){
        UI_Item clicked = lister_get_clicked_item(app, &view, scratch);
        if (state->hot_user_data == clicked.user_data){
            lister_call_activate_handler(app, &global_part, &global_general, &view,
                                         state, clicked.user_data, true);
        }
    }
    state->hot_user_data = 0;
}

CUSTOM_COMMAND_SIG(list_mode__repaint)
CUSTOM_DOC("A list mode command that updates the lists UI data.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        lister_update_ui(app, scratch, &view, state);
    }
}

////////////////////////////////

static void
list_mode_use_default_handlers(Lister *lister){
    lister->write_character = list_mode__write_character__default;
    lister->backspace       = list_mode__backspace_text_field__default;
    lister->navigate_up     = list_mode__move_up__default;
    lister->navigate_down   = list_mode__move_down__default;
}

static void
generate_all_buffers_list(Application_Links *app, Partition *arena, Lister *lister){
    lister_begin_new_item_set(lister);
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        String buffer_name = make_string(buffer.buffer_name, buffer.buffer_name_len);
        Dirty_State dirty = buffer.dirty;
        int32_t buffer_id = buffer.buffer_id;
        String status = {0};
        switch (dirty){
            case DirtyState_UnsavedChanges:  status = make_lit_string(" *"); break;
            case DirtyState_UnloadedChanges: status = make_lit_string(" !"); break;
        }
        lister_add_item(arena, lister, buffer_name, status, (void*)buffer_id);
    }
}

static void
generate_hot_directory_file_list(Application_Links *app, Partition *arena, Lister *lister){
    {
        Temp_Memory temp = begin_temp_memory(arena);
        String hot = get_hot_directory(app, arena);
        if (hot.str[hot.size - 1] != '/' &&
            hot.str[hot.size - 1] != '\\'){
            append_s_char(&hot, '/');
        }
        lister_set_text_field_string(lister, hot);
        lister_set_key_string(lister, front_of_directory(hot));
        end_temp_memory(temp);
    }
    
    lister_begin_new_item_set(lister);
    Temp_Memory temp = begin_temp_memory(arena);
    String hot = get_hot_directory(app, arena);
    File_List file_list = {0};
    if (hot.str != 0){
        file_list = get_file_list(app, hot.str, hot.size);
    }
    end_temp_memory(temp);
    if (hot.str != 0){
        for (File_Info *info = file_list.infos, *one_past_last = file_list.infos + file_list.count;
             info < one_past_last;
             info += 1){
            if (!info->folder) continue;
            String file_name = build_string(arena,
                                            make_string(info->filename, info->filename_len),
                                            "/", "");
            String status = make_lit_string("");
            lister_add_item(arena, lister, lister_prealloced(file_name), status, file_name.str);
        }
        
        for (File_Info *info = file_list.infos, *one_past_last = file_list.infos + file_list.count;
             info < one_past_last;
             info += 1){
            if (info->folder) continue;
            String file_name = push_string_copy(arena, make_string(info->filename, info->filename_len));
            char *is_loaded = "";
            char *status_flag = "";
            Buffer_Summary buffer = get_buffer_by_file_name(app,
                                                            info->filename, info->filename_len,
                                                            AccessAll);
            if (buffer.exists){
                is_loaded = " LOADED";
                switch (buffer.dirty){
                    case DirtyState_UnsavedChanges:  status_flag = " *"; break;
                    case DirtyState_UnloadedChanges: status_flag = " !"; break;
                }
            }
            String status = build_string(arena, is_loaded, status_flag, "");
            lister_add_item(arena, lister, lister_prealloced(file_name), lister_prealloced(status), file_name.str);
        }
        free_file_list(app, file_list);
    }
}

static Lister_Activation_Code
activate_switch_buffer(Application_Links *app, View_Summary *view, String text_field,
                       void *user_data, bool32 activated_by_mouse){
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(user_data);
        view_set_buffer(app, view, buffer_id, SetBuffer_KeepOriginalGUI);
    }
    return(ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    view_start_ui_mode(app, &view);
    view_set_setting(app, &view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(&view);
    init_lister_state(state, &global_general);
    lister_first_init(&state->lister);
    lister_set_query_string(&state->lister, "Switch: ");
    list_mode_use_default_handlers(&state->lister);
    state->lister.activate = activate_switch_buffer;
    state->lister.refresh = generate_all_buffers_list;
    generate_all_buffers_list(app, &state->arena, &state->lister);
    lister_update_ui(app, scratch, &view, state);
}

static Lister_Activation_Code
activate_kill_buffer(Application_Links *app, View_Summary *view, String text_field,
                     void *user_data, bool32 activated_by_mouse){
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(user_data);
        kill_buffer(app, buffer_identifier(buffer_id), view->view_id, 0);
    }
    return(ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    view_start_ui_mode(app, &view);
    view_set_setting(app, &view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(&view);
    init_lister_state(state, &global_general);
    lister_first_init(&state->lister);
    lister_set_query_string(&state->lister, "Kill: ");
    list_mode_use_default_handlers(&state->lister);
    state->lister.activate = activate_kill_buffer;
    state->lister.refresh = generate_all_buffers_list;
    generate_all_buffers_list(app, &state->arena, &state->lister);
    lister_update_ui(app, scratch, &view, state);
}

static Lister_Activation_Code
activate_open_or_new(Application_Links *app, View_Summary *view, String text_field,
                     void *user_data, bool32 activated_by_mouse){
    Partition *scratch = &global_part;
    Lister_Activation_Code result = 0;
    Temp_Memory temp = begin_temp_memory(scratch);
    String file_name = {0};
    if (user_data == 0){
        file_name = text_field;
    }
    else{
        file_name = make_string_slowly((char*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String full_file_name = get_hot_directory(app, scratch);
        if (full_file_name.str[full_file_name.size - 1] != '/' &&
            full_file_name.str[full_file_name.size - 1] != '\\'){
            full_file_name = build_string(scratch, full_file_name, "/", file_name);
        }
        else{
            full_file_name = build_string(scratch, full_file_name, "", file_name);
        }
        if (file_name.str[file_name.size - 1] == '/' && user_data != 0){
            directory_set_hot(app, full_file_name.str, full_file_name.size);
            result = ListerActivation_ContinueAndRefresh;
        }
        else{
            Buffer_Summary buffer = create_buffer(app, full_file_name.str, full_file_name.size, 0);
            if (buffer.exists){
                view_set_buffer(app, view, buffer.buffer_id, SetBuffer_KeepOriginalGUI);
            }
            result = ListerActivation_Finished;
        }
    }
    end_temp_memory(temp);
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    view_start_ui_mode(app, &view);
    view_set_setting(app, &view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(&view);
    init_lister_state(state, &global_general);
    lister_first_init(&state->lister);
    lister_set_query_string(&state->lister, "Open: ");
    list_mode_use_default_handlers(&state->lister);
    state->lister.write_character = list_mode__write_character__file_path;
    state->lister.backspace = list_mode__backspace_text_field__file_path;
    state->lister.activate = activate_open_or_new;
    state->lister.refresh = generate_hot_directory_file_list;
    generate_hot_directory_file_list(app, &state->arena, &state->lister);
    lister_update_ui(app, scratch, &view, state);
}

static Lister_Activation_Code
activate_new(Application_Links *app, View_Summary *view, String text_field,
             void *user_data, bool32 activated_by_mouse){
    Partition *scratch = &global_part;
    Lister_Activation_Code result = 0;
    Temp_Memory temp = begin_temp_memory(scratch);
    String file_name = front_of_directory(text_field);
    bool32 is_folder = false;
    if (user_data != 0){
        String item_name = make_string_slowly((char*)user_data);
        if (item_name.str[item_name.size - 1] == '/'){
            file_name = item_name;
            is_folder = true;
        }
        else if (activated_by_mouse){
            file_name = item_name;
        }
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String full_file_name = get_hot_directory(app, scratch);
        if (full_file_name.str[full_file_name.size - 1] != '/' &&
            full_file_name.str[full_file_name.size - 1] != '\\'){
            full_file_name = build_string(scratch, full_file_name, "/", file_name);
        }
        else{
            full_file_name = build_string(scratch, full_file_name, "", file_name);
        }
        if (is_folder){
            directory_set_hot(app, full_file_name.str, full_file_name.size);
            result = ListerActivation_ContinueAndRefresh;
        }
        else{
            Buffer_Summary buffer = create_buffer(app, full_file_name.str, full_file_name.size, BufferCreate_AlwaysNew);
            if (buffer.exists){
                view_set_buffer(app, view, buffer.buffer_id, SetBuffer_KeepOriginalGUI);
            }
            result = ListerActivation_Finished;
        }
    }
    end_temp_memory(temp);
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    view_start_ui_mode(app, &view);
    view_set_setting(app, &view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(&view);
    init_lister_state(state, &global_general);
    lister_first_init(&state->lister);
    lister_set_query_string(&state->lister, "New: ");
    list_mode_use_default_handlers(&state->lister);
    state->lister.write_character = list_mode__write_character__file_path;
    state->lister.backspace = list_mode__backspace_text_field__file_path;
    state->lister.activate = activate_new;
    state->lister.refresh = generate_hot_directory_file_list;
    generate_hot_directory_file_list(app, &state->arena, &state->lister);
    lister_update_ui(app, scratch, &view, state);
}

static Lister_Activation_Code
activate_open(Application_Links *app, View_Summary *view, String text_field,
              void *user_data, bool32 activated_by_mouse){
    Partition *scratch = &global_part;
    Lister_Activation_Code result = 0;
    Temp_Memory temp = begin_temp_memory(scratch);
    String file_name = {0};
    if (user_data != 0){
        file_name = make_string_slowly((char*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        String full_file_name = get_hot_directory(app, scratch);
        if (full_file_name.str[full_file_name.size - 1] != '/' &&
            full_file_name.str[full_file_name.size - 1] != '\\'){
            full_file_name = build_string(scratch, full_file_name, "/", file_name);
        }
        else{
            full_file_name = build_string(scratch, full_file_name, "", file_name);
        }
        if (file_name.str[file_name.size - 1] == '/' && user_data != 0){
            directory_set_hot(app, full_file_name.str, full_file_name.size);
            result = ListerActivation_ContinueAndRefresh;
        }
        else{
            Buffer_Summary buffer = create_buffer(app, full_file_name.str, full_file_name.size, 0);
            if (buffer.exists){
                view_set_buffer(app, view, buffer.buffer_id, SetBuffer_KeepOriginalGUI);
            }
            result = ListerActivation_Finished;
        }
    }
    end_temp_memory(temp);
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    Partition *scratch = &global_part;
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    view_start_ui_mode(app, &view);
    view_set_setting(app, &view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(&view);
    init_lister_state(state, &global_general);
    lister_first_init(&state->lister);
    lister_set_query_string(&state->lister, "Open: ");
    list_mode_use_default_handlers(&state->lister);
    state->lister.write_character = list_mode__write_character__file_path;
    state->lister.backspace = list_mode__backspace_text_field__file_path;
    state->lister.activate = activate_open;
    state->lister.refresh = generate_hot_directory_file_list;
    generate_hot_directory_file_list(app, &state->arena, &state->lister);
    lister_update_ui(app, scratch, &view, state);
}

// BOTTOM

