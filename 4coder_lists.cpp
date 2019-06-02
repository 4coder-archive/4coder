/*
4coder_lists.cpp - List helpers and list commands
such as open file, switch buffer, or kill buffer.
*/

// TOP

CUSTOM_COMMAND_SIG(lister__quit)
CUSTOM_DOC("A lister mode command that quits the list without executing any actions.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
}

CUSTOM_COMMAND_SIG(lister__activate)
CUSTOM_DOC("A lister mode command that activates the list's action on the highlighted item.")
{
    Heap *heap = &global_heap;
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        void *user_data = 0;
        if (0 <= state->raw_item_index && state->raw_item_index < state->lister.data.options.count){
            user_data = lister_get_user_data(&state->lister.data, state->raw_item_index);
        }
        lister_call_activate_handler(app, heap, view, state, user_data, false);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character)
CUSTOM_DOC("A lister mode command that dispatches to the lister's write character handler.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->lister.data.handlers.write_character != 0){
        state->lister.data.handlers.write_character(app);
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field)
CUSTOM_DOC("A lister mode command that dispatches to the lister's backspace text field handler.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->lister.data.handlers.backspace != 0){
        state->lister.data.handlers.backspace(app);
    }
}

CUSTOM_COMMAND_SIG(lister__move_up)
CUSTOM_DOC("A lister mode command that dispatches to the lister's navigate up handler.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->lister.data.handlers.navigate_up != 0){
        state->lister.data.handlers.navigate_up(app);
    }
}

CUSTOM_COMMAND_SIG(lister__move_down)
CUSTOM_DOC("A lister mode command that dispatches to the lister's navigate down handler.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->lister.data.handlers.navigate_down != 0){
        state->lister.data.handlers.navigate_down(app);
    }
}

CUSTOM_COMMAND_SIG(lister__wheel_scroll)
CUSTOM_DOC("A lister mode command that scrolls the list in response to the mouse wheel.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    GUI_Scroll_Vars scroll = {};
    view_get_scroll_vars(app, view, &scroll);
    Mouse_State mouse = get_mouse_state(app);
    scroll.target_y += mouse.wheel;
    view_set_scroll(app, view, scroll);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        lister_update_ui(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__mouse_press)
CUSTOM_DOC("A lister mode command that beings a click interaction with a list item under the mouse.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        UI_Item clicked = lister_get_clicked_item(app, view);
        state->hot_user_data = clicked.user_data;
    }
}

CUSTOM_COMMAND_SIG(lister__mouse_release)
CUSTOM_DOC("A lister mode command that ends a click interaction with a list item under the mouse, possibly activating it.")
{
    Heap *heap = &global_heap;
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized && state->hot_user_data != 0){
        UI_Item clicked = lister_get_clicked_item(app, view);
        if (state->hot_user_data == clicked.user_data){
            lister_call_activate_handler(app, heap, view, state, clicked.user_data, true);
        }
    }
    state->hot_user_data = 0;
}

CUSTOM_COMMAND_SIG(lister__repaint)
CUSTOM_DOC("A lister mode command that updates the lists UI data.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        lister_update_ui(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__default)
CUSTOM_DOC("A lister mode command that inserts a new character to the text field.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        u8 character[4];
        u32 length = to_writable_character(in, character);
        if (length > 0){
            lister_append_text_field(&state->lister, SCu8(character, length));
            lister_append_key(&state->lister, SCu8(character, length));
            state->item_index = 0;
            view_zero_scroll(app, view);
            lister_update_ui(app, view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field__default)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        state->lister.data.text_field.string = backspace_utf8(state->lister.data.text_field.string);
        state->lister.data.key_string.string = backspace_utf8(state->lister.data.key_string.string);
        state->item_index = 0;
        view_zero_scroll(app, view);
        lister_update_ui(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__move_up__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one up in the list.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        state->item_index = state->item_index - 1;
        if (state->item_index < 0){
            state->item_index = state->item_count_after_filter - 1;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_ui(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__move_down__default)
CUSTOM_DOC("A lister mode command that moves the highlighted item one down in the list.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        state->item_index = state->item_index + 1;
        if (state->item_index > state->item_count_after_filter - 1){
            state->item_index = 0;
        }
        state->set_view_vertical_focus_to_item = true;
        lister_update_ui(app, view, state);
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__file_path)
CUSTOM_DOC("A lister mode command that inserts a character into the text field of a file system list.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        u8 character[4];
        u32 length = to_writable_character(in, character);
        if (length > 0){
            lister_append_text_field(&state->lister, SCu8(character, length));
            String_Const_u8 front_name = string_front_of_path(state->lister.data.text_field.string);
            lister_set_key(&state->lister, front_name);
            if (character[0] == '/' || character[0] == '\\'){
                String_Const_u8 new_hot = state->lister.data.text_field.string;
                set_hot_directory(app, new_hot);
                lister_call_refresh_handler(app, &state->lister);
            }
            state->item_index = 0;
            view_zero_scroll(app, view);
            lister_update_ui(app, view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__backspace_text_field__file_path)
CUSTOM_DOC("A lister mode command that backspaces one character from the text field of a file system list.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        if (state->lister.data.text_field.size > 0){
            char last_char = state->lister.data.text_field.str[state->lister.data.text_field.size - 1];
            state->lister.data.text_field.string = backspace_utf8(state->lister.data.text_field.string);
            if (last_char == '/' || last_char == '\\'){
                User_Input input = get_command_input(app);
                String_Const_u8 text_field = state->lister.data.text_field.string;
                String_Const_u8 new_hot = string_remove_last_folder(text_field);
                b32 is_modified = (input.key.modifiers[MDFR_SHIFT_INDEX] ||
                                   input.key.modifiers[MDFR_CONTROL_INDEX] ||
                                   input.key.modifiers[MDFR_ALT_INDEX] ||
                                   input.key.modifiers[MDFR_COMMAND_INDEX]);
                b32 whole_word_backspace = (is_modified == global_config.file_lister_per_character_backspace);
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
            lister_update_ui(app, view, state);
        }
    }
}

CUSTOM_COMMAND_SIG(lister__write_character__fixed_list)
CUSTOM_DOC("A lister mode command that handles input for the fixed sure to kill list.")
{
    Heap *heap = &global_heap;
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    Lister_State *state = view_get_lister_state(view);
    if (state->initialized){
        User_Input in = get_command_input(app);
        u8 character[4];
        u32 length = to_writable_character(in, character);
        if (length > 0){
            void *user_data = 0;
            b32 did_shortcut_key = false;
            for (Lister_Node *node = state->lister.data.options.first;
                 node != 0;
                 node = node->next){
                char *hotkeys = (char*)(node + 1);
                String_Const_u8 hot_key_string = SCu8(hotkeys);
                if (string_find_first(hot_key_string, SCu8(character, length)) < hot_key_string.size){
                    user_data = node->user_data;
                    did_shortcut_key = true;
                    break;
                }
            }
            if (did_shortcut_key){
                lister_call_activate_handler(app, heap, view, state, user_data, false);
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
                                              View_ID view){
    if (handlers.refresh != 0){
        Heap *heap = &global_heap;
        view_begin_ui_mode(app, view);
        view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
        Lister_State *state = view_get_lister_state(view);
        init_lister_state(app, state, heap);
        lister_first_init(app, &state->lister, user_data, user_data_size);
        lister_set_query(&state->lister, query_string);
        state->lister.data.handlers = handlers;
        handlers.refresh(app, &state->lister);
        lister_update_ui(app, view, state);
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
                                    Lister_Activation_Function_Type *activate,
                                    void *user_data, i32 user_data_size,
                                    Lister_Option *options, i32 option_count,
                                    i32 estimated_string_space_size,
                                    View_ID view){
    Heap *heap = &global_heap;
    view_begin_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        lister_add_item(&state->lister, options[i].string, options[i].status, options[i].user_data, 0);
    }
    lister_set_query(&state->lister, query_string);
    state->lister.data.handlers = lister_get_default_handlers();
    state->lister.data.handlers.activate = activate;
    lister_update_ui(app, view, state);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Handlers handlers,
                                            void *user_data, i32 user_data_size,
                                            Lister_Fixed_Option *options, i32 option_count,
                                            i32 estimated_string_space_size,
                                            View_ID view){
    Heap *heap = &global_heap;
    view_begin_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    for (i32 i = 0; i < option_count; i += 1){
        char *shortcut_chars = options[i].shortcut_chars;
        umem shortcut_chars_length = cstring_length(shortcut_chars);
        void *extra = lister_add_item(&state->lister,
                                      SCu8(options[i].string),
                                      SCu8(options[i].status),
                                      options[i].user_data,
                                      shortcut_chars_length + 1);
        memcpy(extra, shortcut_chars, shortcut_chars_length + 1);
    }
    lister_set_query(&state->lister, query_string);
    state->lister.data.handlers = handlers;
    state->lister.data.handlers.refresh = 0;
    lister_update_ui(app, view, state);
}

static void
begin_integrated_lister__with_fixed_options(Application_Links *app, char *query_string,
                                            Lister_Activation_Function_Type *activate,
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
    view_begin_ui_mode(app, view);
    view_set_setting(app, view, ViewSetting_UICommandMap, default_lister_ui_map);
    Lister_State *state = view_get_lister_state(view);
    init_lister_state(app, state, heap);
    lister_first_init(app, &state->lister, user_data, user_data_size);
    state->lister.data.theme_list = true;
    for (i32 i = 0; i < option_count; i += 1){
        lister_add_theme_item(&state->lister,
                              SCu8(options[i].string),
                              options[i].index,
                              options[i].user_data, 0);
    }
    lister_set_query(&state->lister, query_string);
    state->lister.data.handlers = handlers;
    state->lister.data.handlers.refresh = 0;
    lister_update_ui(app, view, state);
}

static void
begin_integrated_lister__theme_list(Application_Links *app, char *query_string,
                                    Lister_Activation_Function_Type *activate,
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
    Dirty_State dirty = 0;
    buffer_get_dirty_state(app, buffer, &dirty);
    String_Const_u8 status = {};
    switch (dirty){
        case DirtyState_UnsavedChanges:  status = string_u8_litexpr("*"); break;
        case DirtyState_UnloadedChanges: status = string_u8_litexpr("!"); break;
        case DirtyState_UnsavedChangesAndUnloadedChanges: status = string_u8_litexpr("*!"); break;
    }
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer);
    lister_add_item(lister, buffer_name, status, IntAsPtr(buffer), 0);
    end_temp(temp);
}

static void
generate_all_buffers_list(Application_Links *app, Lister *lister){
    lister_begin_new_item_set(app, lister);
    
    Buffer_ID buffers_currently_being_viewed[16];
    i32 currently_viewed_buffer_count = 0;
    
    // List currently viewed buffers
    {
        View_ID view = 0;
        for (get_view_next(app, 0, AccessAll, &view);
             view != 0;
             get_view_next(app, view, AccessAll, &view)){
            Buffer_ID new_buffer_id = 0;
            view_get_buffer(app, view, AccessAll, &new_buffer_id);
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
        Buffer_ID buffer = 0;
        for (get_buffer_next(app, 0, AccessAll, &buffer);
             buffer != 0;
             get_buffer_next(app, buffer, AccessAll, &buffer)){
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
        Buffer_ID buffer = 0;
        for (get_buffer_next(app, 0, AccessAll, &buffer);
             buffer != 0;
             get_buffer_next(app, buffer, AccessAll, &buffer)){
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
    Temp_Memory temp = begin_temp(&lister->arena);
    String_Const_u8 hot = push_hot_directory(app, &lister->arena);
    if (!character_is_slash(string_get_character(hot, hot.size - 1))){
        hot = string_u8_pushf(&lister->arena, "%.*s/", string_expand(hot));
    }
    lister_set_text_field(lister, hot);
    lister_set_key(lister, string_front_of_path(hot));
    
    File_List file_list = {};
    get_file_list(app, hot, &file_list);
    end_temp(temp);
    
    File_Info *one_past_last = file_list.infos + file_list.count;
    
    lister_begin_new_item_set(app, lister);
    
    hot = push_hot_directory(app, &lister->arena);
    push_align(&lister->arena, 8);
    if (hot.str != 0){
        String_Const_u8 empty_string = string_u8_litexpr("");
        Lister_Prealloced_String empty_string_prealloced = lister_prealloced(empty_string);
        for (File_Info *info = file_list.infos;
             info < one_past_last;
             info += 1){
            if (!info->folder) continue;
            String_Const_u8 file_name = string_u8_pushf(&lister->arena, "%.*s/", info->filename_len, info->filename);
            lister_add_item(lister, lister_prealloced(file_name), empty_string_prealloced, file_name.str, 0);
        }
        
        for (File_Info *info = file_list.infos;
             info < one_past_last;
             info += 1){
            if (info->folder) continue;
            String_Const_u8 file_name = string_copy(&lister->arena, SCu8(info->filename, info->filename_len));
            char *is_loaded = "";
            char *status_flag = "";
            
            
            Buffer_ID buffer = {};
            
            {
                Temp_Memory path_temp = begin_temp(&lister->arena);
                List_String_Const_u8 list = {};
                string_list_push(&lister->arena, &list, hot);
                string_list_push_overlap(&lister->arena, &list, '/',
                                         SCu8(info->filename, info->filename_len));
                String_Const_u8 full_file_path = string_list_flatten(&lister->arena, list);
                get_buffer_by_file_name(app, full_file_path, AccessAll, &buffer);
                end_temp(path_temp);
            }
            
            if (buffer != 0){
                is_loaded = "LOADED";
                Dirty_State dirty = 0;
                buffer_get_dirty_state(app, buffer, &dirty);
                switch (dirty){
                    case DirtyState_UnsavedChanges:  status_flag = " *"; break;
                    case DirtyState_UnloadedChanges: status_flag = " !"; break;
                    case DirtyState_UnsavedChangesAndUnloadedChanges: status_flag = " *!"; break;
                }
            }
            String_Const_u8 status = string_u8_pushf(&lister->arena, "%s%s", is_loaded, status_flag);
            lister_add_item(lister, lister_prealloced(file_name), lister_prealloced(status), file_name.str, 0);
        }
    }
    
    free_file_list(app, file_list);
}

static void
begin_integrated_lister__buffer_list(Application_Links *app, char *query_string, Lister_Activation_Function_Type *activate_procedure,
                                     void *user_data, i32 user_data_size, View_ID target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_all_buffers_list;
    begin_integrated_lister__with_refresh_handler(app, query_string, handlers, user_data, user_data_size, target_view);
}

static void
begin_integrated_lister__file_system_list(Application_Links *app, char *query_string, Lister_Activation_Function_Type *activate_procedure,
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

static void
activate_confirm_kill(Application_Links *app, Heap *heap, View_ID view, Lister_State *state, String_Const_u8 text_field, void *user_data, b32 clicked){
    i32 behavior = (i32)PtrAsInt(user_data);
    Buffer_ID buffer_id = *(Buffer_ID*)(state->lister.data.user_data);
    switch (behavior){
        case SureToKill_No:
        {}break;
        
        case SureToKill_Yes:
        {
            Buffer_Kill_Result ignore = 0;
            buffer_kill(app, buffer_id, BufferKill_AlwaysKill, &ignore);
        }break;
        
        case SureToKill_Save:
        {
            Arena *scratch = context_get_arena(app);
            Temp_Memory temp = begin_temp(scratch);
            String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
            if (buffer_save(app, buffer_id, file_name, BufferSave_IgnoreDirtyFlag)){
                buffer_kill(app, buffer_id, BufferKill_AlwaysKill, 0);
            }
            else{
                String_Const_u8 str = string_u8_pushf(scratch, "Did not close '%.*s' because it did not successfully save.",
                                                      string_expand(file_name));
                print_message(app, str);
            }
            
            end_temp(temp);
        }break;
    }
    lister_default(app, heap, view, state, ListerActivation_Finished);
}

static void
do_gui_sure_to_kill(Application_Links *app, Buffer_ID buffer, View_ID view){
    Lister_Fixed_Option options[] = {
        {"(N)o"           , "", "Nn", IntAsPtr(SureToKill_No)  },
        {"(Y)es"          , "", "Yy", IntAsPtr(SureToKill_Yes) },
        {"(S)ave and Kill", "", "Ss", IntAsPtr(SureToKill_Save)},
    };
    i32 option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app, "There are unsaved changes, close anyway?",
                                                activate_confirm_kill, &buffer, sizeof(buffer),
                                                options, option_count, default_string_size_estimation,
                                                view);
}

static void
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
}

static void
do_gui_sure_to_close_4coder(Application_Links *app, View_ID view){
    Lister_Fixed_Option options[] = {
        {"(N)o"                , "", "Nn", (void*)SureToKill_No  },
        {"(Y)es"               , "", "Yy", (void*)SureToKill_Yes },
        {"(S)ave All and Close", "", "Ss", (void*)SureToKill_Save},
    };
    i32 option_count = sizeof(options)/sizeof(options[0]);
    begin_integrated_lister__with_fixed_options(app, "There are one or more buffers with unsave changes, close anyway?",
                                                activate_confirm_close_4coder, 0, 0,
                                                options, option_count, default_string_size_estimation,
                                                view);
}

////////////////////////////////

static void
activate_switch_buffer(Application_Links *app, Heap *heap,
                       View_ID view, Lister_State *state,
                       String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(PtrAsInt(user_data));
        view_set_buffer(app, view, buffer_id, SetBuffer_KeepOriginalGUI);
    }
    lister_default(app, heap, view, state, ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
    begin_integrated_lister__buffer_list(app, "Switch:", activate_switch_buffer, 0, 0, view);
}

static void
activate_kill_buffer(Application_Links *app, Heap *heap,
                     View_ID view, struct Lister_State *state,
                     String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, heap, view, state, ListerActivation_Finished);
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(PtrAsInt(user_data));
        kill_buffer(app, buffer_identifier(buffer_id), view, 0);
    }
}

CUSTOM_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
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
        Arena *scratch = context_get_arena(app);
        Temp_Memory temp = begin_temp(scratch);
        String_Const_u8 full_file_name = {};
        if (character_is_slash(string_get_character(path, path.size - 1))){
            path = string_chop(path, 1);
        }
        full_file_name = string_u8_pushf(scratch, "%.*s/%.*s", string_expand(path), string_expand(file_name));
        if (is_folder){
            set_hot_directory(app, full_file_name);
            result = ListerActivation_ContinueAndRefresh;
        }
        else{
            Buffer_ID buffer = 0;
            create_buffer(app, full_file_name, flags, &buffer);
            if (buffer != 0){
                view_set_buffer(app, view, buffer, SetBuffer_KeepOriginalGUI);
            }
            result = ListerActivation_Finished;
        }
        end_temp(temp);
    }
    
    return(result);
}

static void
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
}

CUSTOM_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
    begin_integrated_lister__file_system_list(app, "Open:", activate_open_or_new, 0, 0, view);
}

static void
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
}

CUSTOM_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
    begin_integrated_lister__file_system_list(app, "New:", activate_new, 0, 0, view);
}

static void
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
}

CUSTOM_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
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
    
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
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

static void
activate_command(Application_Links *app, Heap *heap,
                 View_ID view, Lister_State *state,
                 String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, heap, view, state, ListerActivation_Finished);
    if (user_data != 0){
        Custom_Command_Function *command = (Custom_Command_Function*)user_data;
        command(app);
    }
}

static void
launch_custom_command_lister(Application_Links *app, i32 *command_ids, i32 command_id_count){
    if (command_ids == 0){
        command_id_count = command_one_past_last_id;
    }
    
    Arena *scratch = context_get_arena(app);
    View_ID view = 0;
    get_active_view(app, AccessAll, &view);
    view_end_ui_mode(app, view);
    Temp_Memory temp = begin_temp(scratch);
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
    end_temp(temp);
}

CUSTOM_COMMAND_SIG(command_lister)
CUSTOM_DOC("Opens an interactive list of all registered commands.")
{
    launch_custom_command_lister(app, 0, 0);
}

// BOTTOM

