/*
4coder_lists.cpp - List helpers and list commands
such as open file, switch buffer, or kill buffer.
*/

// TOP

CUSTOM_COMMAND_SIG(lister__quit)
CUSTOM_DOC("A lister mode command that quits the list without executing any actions.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    state->initialized = false;
    view_end_ui_mode(app, &view);
}

CUSTOM_COMMAND_SIG(lister__activate)
CUSTOM_DOC("A lister mode command that activates the list's action on the highlighted item.")
{
    Partition *scratch = &global_part;
    General_Memory *general = &global_general;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        void *user_data = 0;
        if (0 <= state->raw_item_index && state->raw_item_index < state->lister.options.count){
            user_data = lister_get_user_data(&state->lister, state->raw_item_index);
        }
        lister_call_activate_handler(app, scratch, general, &view,
                                     state, user_data, false);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character)
CUSTOM_DOC("A lister mode command that dispatches to the lister's write character handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.handlers.write_character != 0){
        state->lister.handlers.write_character(app);
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field)
CUSTOM_DOC("A lister mode command that dispatches to the lister's backspace text field handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.handlers.backspace != 0){
        state->lister.handlers.backspace(app);
    }
}

CUSTOM_COMMAND_SIG(lister__move_up)
CUSTOM_DOC("A lister mode command that dispatches to the lister's navigate up handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.handlers.navigate_up != 0){
        state->lister.handlers.navigate_up(app);
    }
}

CUSTOM_COMMAND_SIG(lister__move_down)
CUSTOM_DOC("A lister mode command that dispatches to the lister's navigate down handler.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->lister.handlers.navigate_down != 0){
        state->lister.handlers.navigate_down(app);
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
        UI_Item clicked = lister_get_clicked_item(app, &view, scratch);
        state->hot_user_data = clicked.user_data;
    }
}

CUSTOM_COMMAND_SIG(lister__mouse_release)
CUSTOM_DOC("A lister mode command that ends a click interaction with a list item under the mouse, possibly activating it.")
{
    Partition *scratch = &global_part;
    General_Memory *general = &global_general;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized && state->hot_user_data != 0){
        UI_Item clicked = lister_get_clicked_item(app, &view, scratch);
        if (state->hot_user_data == clicked.user_data){
            lister_call_activate_handler(app, scratch, general, &view,
                                         state, clicked.user_data, true);
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

CUSTOM_COMMAND_SIG(lister__backspace_text_field__default)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field.")
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

CUSTOM_COMMAND_SIG(lister__move_up__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one up in the list.")
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

CUSTOM_COMMAND_SIG(lister__move_down__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one down in the list.")
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

CUSTOM_COMMAND_SIG(lister__write_character__file_path)
CUSTOM_DOC("A lister mode command that inserts a character into the text field of a file system list.")
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

CUSTOM_COMMAND_SIG(lister__backspace_text_field__file_path)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field of a file system list.")
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

CUSTOM_COMMAND_SIG(lister__write_character__fixed_list)
CUSTOM_DOC("A lister mode command that handles input for the fixed sure to kill list.")
{
    Partition *scratch = &global_part;
    General_Memory *general = &global_general;
    View_Summary view = get_active_view(app, AccessAll);
    Lister_State *state = view_get_lister_state(&view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        uint8_t character[4];
        uint32_t length = to_writable_character(in, character);
        if (length > 0){
            void *user_data = 0;
            bool32 did_shortcut_key = false;
            for (Lister_Option_Node *node = state->lister.options.first;
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
                lister_call_activate_handler(app, scratch, general,
                                             &view, state,
                                             user_data, false);
            }
        }
    }
}

////////////////////////////////

static Lister_Handlers
lister_get_default_handlers(void){
    Lister_Handlers handlers = {0};
    handlers.write_character = lister__write_character__default;
    handlers.backspace       = lister__backspace_text_field__default;
    handlers.navigate_up     = lister__move_up__default;
    handlers.navigate_down   = lister__move_down__default;
    return(handlers);
}

static Lister_Handlers
lister_get_fixed_list_handlers(void){
    Lister_Handlers handlers = {0};
    handlers.write_character = lister__write_character__fixed_list;
    handlers.backspace       = 0;
    handlers.navigate_up     = lister__move_up__default;
    handlers.navigate_down   = lister__move_down__default;
    return(handlers);
}

static void
begin_integrated_lister__with_refresh_handler(Application_Links *app, char *query_string,
                                              Lister_Handlers handlers, void *user_data,
                                              View_Summary *view){
    if (handlers.refresh != 0){
        Partition *scratch = &global_part;
        General_Memory *general = &global_general;
        view_start_ui_mode(app, view);
        view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
        Lister_State *state = view_get_lister_state(view);
        init_lister_state(state, general);
        lister_first_init(&state->lister);
        lister_set_query_string(&state->lister, query_string);
        state->lister.handlers = handlers;
        state->lister.user_data = user_data;
        handlers.refresh(app, &state->arena, &state->lister);
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

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Handlers handlers, void *user_data,
                                            Lister_Fixed_Option *options, int32_t option_count,
                                            View_Summary *view){
    Partition *scratch = &global_part;
    General_Memory *general = &global_general;
    view_start_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(state, general);
    lister_first_init(&state->lister);
    for (int32_t i = 0; i < option_count; i += 1){
        char *shortcut_chars = options[i].shortcut_chars;
        int32_t shortcut_chars_length = str_size(shortcut_chars);
        void *extra = lister_add_item(&state->arena, &state->lister,
                                      make_string_slowly(options[i].string),
                                      make_string_slowly(options[i].status),
                                      options[i].user_data,
                                      shortcut_chars_length + 1);
        memcpy(extra, shortcut_chars, shortcut_chars_length + 1);
    }
    lister_set_query_string(&state->lister, query_string);
    state->lister.handlers = handlers;
    state->lister.handlers.refresh = 0;
    state->lister.user_data = user_data;
    lister_update_ui(app, scratch, view, state);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Activation_Function_Type *activate, void *user_data,
                                            Lister_Fixed_Option *options, int32_t option_count,
                                            View_Summary *view){
    Lister_Handlers handlers = lister_get_fixed_list_handlers();
    handlers.activate = activate;
    begin_integrated_lister__with_fixed_options(app, query_string,
                                                handlers, user_data,
                                                options, option_count,
                                                view);
}

////////////////////////////////

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
        lister_add_item(arena, lister, buffer_name, status, (void*)buffer_id, 0);
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
            lister_add_item(arena, lister, lister_prealloced(file_name), status, file_name.str, 0);
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
            lister_add_item(arena, lister, lister_prealloced(file_name), lister_prealloced(status), file_name.str, 0);
        }
        free_file_list(app, file_list);
    }
}

static void
begin_integrated_lister__buffer_list(Application_Links *app, char *query_string,
                                     Lister_Activation_Function_Type *activate_procedure, void *user_data,
                                     View_Summary *target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_all_buffers_list;
    begin_integrated_lister__with_refresh_handler(app, query_string, handlers, user_data, target_view);
}

static void
begin_integrated_lister__file_system_list(Application_Links *app, char *query_string,
                                          Lister_Activation_Function_Type *activate_procedure,
                                          void *user_data,
                                          View_Summary *target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_hot_directory_file_list;
    handlers.write_character = lister__write_character__file_path;
    handlers.backspace = lister__backspace_text_field__file_path;
    begin_integrated_lister__with_refresh_handler(app, query_string, handlers, user_data, target_view);
}

////////////////////////////////

enum{
    SureToKill_NULL = 0,
    SureToKill_No = 1,
    SureToKill_Yes = 2,
    SureToKill_Save = 3,
};

static Lister_Activation_Code
activate_confirm_kill(Application_Links *app, View_Summary *view, String text_field,
                      void *user_data, bool32 clicked){
    int32_t behavior = (int32_t)user_data;
    Lister_State *state = view_get_lister_state(view);
    Buffer_ID buffer_id = (Buffer_ID)(state->lister.user_data);
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
    return(ListerActivation_Finished);
}

static void
do_gui_sure_to_kill(Application_Links *app, Buffer_Summary *buffer, View_Summary *view){
    Lister_Fixed_Option options[] = {
        {"(N)o"           , "", "Nn", (void*)SureToKill_No  },
        {"(Y)es"          , "", "Yy", (void*)SureToKill_Yes },
        {"(S)ave and Kill", "", "Ss", (void*)SureToKill_Save},
    };
    int32_t option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app, "There are unsaved changes, close anyway?",
                                                activate_confirm_kill, (void*)buffer->buffer_id,
                                                options, option_count,
                                                view);
}

static Lister_Activation_Code
activate_confirm_close_4coder(Application_Links *app, View_Summary *view, String text_field,
                              void *user_data, bool32 clicked){
    int32_t behavior = (int32_t)user_data;
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
    return(ListerActivation_Finished);
}

static void
do_gui_sure_to_close_4coder(Application_Links *app, View_Summary *view){
    Lister_Fixed_Option options[] = {
        {"(N)o"                , "", "Nn", (void*)SureToKill_No  },
        {"(Y)es"               , "", "Yy", (void*)SureToKill_Yes },
        {"(S)ave All and Close", "", "Ss", (void*)SureToKill_Save},
    };
    int32_t option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app,
                                                "There are one or more buffers with unsave changes, close anyway?",
                                                activate_confirm_close_4coder, 0,
                                                options, option_count,
                                                view);
}

////////////////////////////////

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
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    begin_integrated_lister__buffer_list(app, "Switch: ", activate_switch_buffer, 0, &view);
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
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    begin_integrated_lister__buffer_list(app, "Kill: ", activate_kill_buffer, 0, &view);
}

static Lister_Activation_Code
activate_open_or_new__generic(Application_Links *app, View_Summary *view,
                              String file_name, bool32 is_folder,
                              Buffer_Create_Flag flags){
    Lister_Activation_Code result = 0;
    
    if (file_name.size == 0){
        char msg[] = "Zero length file_name passed to activate_open_or_new__generic\n";
        print_message(app, msg, sizeof(msg) - 1);
        result = ListerActivation_Finished;
    }
    else{
        Partition *scratch = &global_part;
        Temp_Memory temp = begin_temp_memory(scratch);
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

static Lister_Activation_Code
activate_open_or_new(Application_Links *app, View_Summary *view, String text_field,
                     void *user_data, bool32 clicked){
    Lister_Activation_Code result = 0;
    String file_name = {0};
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
        bool32 is_folder = (file_name.str[file_name.size - 1] == '/' && user_data != 0);
        Buffer_Create_Flag flags = 0;
        result = activate_open_or_new__generic(app, view, file_name, is_folder, flags);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    begin_integrated_lister__file_system_list(app, "Open: ", activate_open_or_new, 0, &view);
}

static Lister_Activation_Code
activate_new(Application_Links *app, View_Summary *view, String text_field,
             void *user_data, bool32 clicked){
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
        bool32 is_folder = (file_name.str[file_name.size - 1] == '/' && user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_AlwaysNew;
        result = activate_open_or_new__generic(app, view, file_name, is_folder, flags);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    begin_integrated_lister__file_system_list(app, "New: ", activate_new, 0, &view);
}

static Lister_Activation_Code
activate_open(Application_Links *app, View_Summary *view, String text_field,
              void *user_data, bool32 clicked){
    Lister_Activation_Code result = 0;
    String file_name = {0};
    if (user_data != 0){
        file_name = make_string_slowly((char*)user_data);
    }
    if (file_name.size == 0){
        result = ListerActivation_Finished;
    }
    else{
        bool32 is_folder = (file_name.str[file_name.size - 1] == '/' && user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_NeverNew;
        result = activate_open_or_new__generic(app, view, file_name, is_folder, flags);
    }
    return(result);
}

CUSTOM_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    View_Summary view = get_active_view(app, AccessAll);
    for (;view_end_ui_mode(app, &view););
    begin_integrated_lister__file_system_list(app, "Open: ", activate_open, 0, &view);
}

// BOTTOM

