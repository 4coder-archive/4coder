/*
4coder_lists.cpp - List helpers and list commands
such as open file, switch buffer, or kill buffer.
*/

// TOP

function void
generate_all_buffers_list__output_buffer(Application_Links *app, Lister *lister,
                                         Buffer_ID buffer){
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

function void
generate_all_buffers_list(Application_Links *app, Lister *lister){
    lister_begin_new_item_set(app, lister);
    
    Buffer_ID viewed_buffers[16];
    i32 viewed_buffer_count = 0;
    
    // List currently viewed buffers
    for (View_ID view = get_view_next(app, 0, Access_Always);
         view != 0;
         view = get_view_next(app, view, Access_Always)){
        Buffer_ID new_buffer_id = view_get_buffer(app, view, Access_Always);
        for (i32 i = 0; i < viewed_buffer_count; i += 1){
            if (new_buffer_id == viewed_buffers[i]){
                goto skip0;
            }
        }
        viewed_buffers[viewed_buffer_count++] = new_buffer_id;
        skip0:;
    }
    
    // Regular Buffers
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always)){
        for (i32 i = 0; i < viewed_buffer_count; i += 1){
            if (buffer == viewed_buffers[i]){
                goto skip1;
            }
        }
        if (!buffer_has_name_with_star(app, buffer)){
            generate_all_buffers_list__output_buffer(app, lister, buffer);
        }
        skip1:;
    }
    
    // Buffers Starting with *
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always)){
        for (i32 i = 0; i < viewed_buffer_count; i += 1){
            if (buffer == viewed_buffers[i]){
                goto skip2;
            }
        }
        if (buffer_has_name_with_star(app, buffer)){
            generate_all_buffers_list__output_buffer(app, lister, buffer);
        }
        skip2:;
    }
    
    // Buffers That Are Open in Views
    for (i32 i = 0; i < viewed_buffer_count; i += 1){
        generate_all_buffers_list__output_buffer(app, lister, viewed_buffers[i]);
    }
}

function Buffer_ID
get_buffer_from_user(Application_Links *app, String_Const_u8 query){
    View_ID view = get_active_view(app, Access_Always);
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.refresh = generate_all_buffers_list;
    Lister_Result l_result = run_lister_with_refresh_handler(app, query, handlers,
                                                             0, 0, view);
    Buffer_ID result = 0;
    if (!l_result.canceled){
        result = (Buffer_ID)(PtrAsInt(l_result.user_data));
    }
    return(result);
}

function Buffer_ID
get_buffer_from_user(Application_Links *app, char *query){
    return(get_buffer_from_user(app, SCu8(query)));
}

////////////////////////////////

function void
lister__write_character__file_path(Application_Links *app){
    View_ID view = get_active_view(app, Access_Always);
    Lister *lister = view_get_lister(view);
    if (lister != 0){
        User_Input in = get_current_input(app);
        String_Const_u8 string = to_writable(&in);
        if (string.str != 0 && string.size > 0){
            lister_append_text_field(lister, string);
            String_Const_u8 front_name = string_front_of_path(lister->text_field.string);
            lister_set_key(lister, front_name);
            if (character_is_slash(string.str[0])){
                String_Const_u8 new_hot = lister->text_field.string;
                set_hot_directory(app, new_hot);
                lister_call_refresh_handler(app, view, lister);
            }
            lister->item_index = 0;
            lister_zero_scroll(lister);
            lister_update_filtered_list(app, view, lister);
        }
    }
}

function void
lister__backspace_text_field__file_path(Application_Links *app){
    View_ID view = get_active_view(app, Access_Always);
    Lister *lister = view_get_lister(view);
    if (lister != 0){
        if (lister->text_field.size > 0){
            char last_char = lister->text_field.str[lister->text_field.size - 1];
            lister->text_field.string = backspace_utf8(lister->text_field.string);
            if (character_is_slash(last_char)){
                User_Input input = get_current_input(app);
                String_Const_u8 text_field = lister->text_field.string;
                String_Const_u8 new_hot = string_remove_last_folder(text_field);
                b32 is_modified = has_modifier(&input, KeyCode_Control);
                b32 whole_word_backspace = (is_modified == global_config.lister_whole_word_backspace_when_modified);
                if (whole_word_backspace){
                    lister->text_field.size = new_hot.size;
                }
                set_hot_directory(app, new_hot);
                // TODO(allen): We have to protect against lister_call_refresh_handler
                // changing the text_field here. Clean this up.
                String_u8 dingus = lister->text_field;
                lister_call_refresh_handler(app, view, lister);
                lister->text_field = dingus;
            }
            else{
                String_Const_u8 text_field = lister->text_field.string;
                String_Const_u8 new_key = string_front_of_path(text_field);
                lister_set_key(lister, new_key);
            }
            
            lister->item_index = 0;
            lister_zero_scroll(lister);
            lister_update_filtered_list(app, view, lister);
        }
    }
}

function void
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
                buffer = get_buffer_by_file_name(app, full_file_path, Access_Always);
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

function void
run_lister_file_system_list(Application_Links *app, char *query_string,
                            Lister_Activation_Type *activate_procedure,
                            void *user_data, i32 user_data_size, View_ID target_view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.activate = activate_procedure;
    handlers.refresh = generate_hot_directory_file_list;
    handlers.write_character = lister__write_character__file_path;
    handlers.backspace = lister__backspace_text_field__file_path;
    run_lister_with_refresh_handler(app, SCu8(query_string), handlers,
                                    user_data, user_data_size, target_view);
}

////////////////////////////////

enum{
    SureToKill_NULL = 0,
    SureToKill_No = 1,
    SureToKill_Yes = 2,
    SureToKill_Save = 3,
};

function b32
do_gui_sure_to_kill(Application_Links *app, Buffer_ID buffer, View_ID view){
    Scratch_Block scratch(app);
    Lister_Choice_List list = {};
    lister_choice(scratch, &list, "(N)o"  , "", KeyCode_N, SureToKill_No);
    lister_choice(scratch, &list, "(Y)es" , "", KeyCode_Y, SureToKill_Yes);
    lister_choice(scratch, &list, "(S)ave", "", KeyCode_S, SureToKill_Save);
    
    Lister_Choice *choice =
        get_choice_from_user(app, "There are unsaved changes, close anyway?", list);
    
    b32 do_kill = false;
    if (choice != 0){
        switch (choice->user_data){
            case SureToKill_No:
            {}break;
            
            case SureToKill_Yes:
            {
                do_kill = true;
            }break;
            
            case SureToKill_Save:
            {
                String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer);
                if (buffer_save(app, buffer, file_name, BufferSave_IgnoreDirtyFlag)){
                    do_kill = true;
                }
                else{
#define M "Did not close '%.*s' because it did not successfully save."
                    String_Const_u8 str =
                        push_u8_stringf(scratch, M, string_expand(file_name));
#undef M
                    print_message(app, str);
                }
            }break;
        }
    }
    
    return(do_kill);
}

function Lister_Activation_Code
activate_confirm_close_4coder(Application_Links *app,
                              View_ID view, Lister *lister,
                              String_Const_u8 text_field, void *user_data, b32 clicked){
    i32 behavior = (i32)PtrAsInt(user_data);
    b32 *do_exit = (b32*)user_data;
    switch (behavior){
        case SureToKill_No:
        {}break;
        
        case SureToKill_Yes:
        {
            allow_immediate_close_without_checking_for_changes = true;
            *do_exit = true;
        }break;
        
        case SureToKill_Save:
        {
            save_all_dirty_buffers(app);
            allow_immediate_close_without_checking_for_changes = true;
            *do_exit = true;
        }break;
    }
    lister_default(app, view, lister, ListerActivation_Finished);
    return(ListerActivation_Finished);
}

function b32
do_gui_sure_to_close_4coder(Application_Links *app, View_ID view){
    Scratch_Block scratch(app);
    Lister_Choice_List list = {};
    lister_choice(scratch, &list, "(N)o"  , "", KeyCode_N, SureToKill_No);
    lister_choice(scratch, &list, "(Y)es" , "", KeyCode_Y, SureToKill_Yes);
    lister_choice(scratch, &list, "(S)ave all and close", "",
                  KeyCode_S, SureToKill_Save);
    
#define M "There are one or more buffers with unsave changes, close anyway?"
    Lister_Choice *choice = get_choice_from_user(app, M, list);
#undef M
    
    b32 do_exit = false;
    if (choice != 0){
        switch (choice->user_data){
            case SureToKill_No:
            {}break;
            
            case SureToKill_Yes:
            {
                allow_immediate_close_without_checking_for_changes = true;
                do_exit = true;
            }break;
            
            case SureToKill_Save:
            {
                save_all_dirty_buffers(app);
                allow_immediate_close_without_checking_for_changes = true;
                do_exit = true;
            }break;
        }
    }
    
    return(do_exit);
}

////////////////////////////////

#if 0
function Lister_Activation_Code
activate_switch_buffer(Application_Links *app,
                       View_ID view, Lister *lister,
                       String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    if (user_data != 0){
        Buffer_ID buffer_id = (Buffer_ID)(PtrAsInt(user_data));
        view_set_buffer(app, view, buffer_id, SetBuffer_KeepOriginalGUI);
    }
    lister_default(app, view, lister, ListerActivation_Finished);
    return(ListerActivation_Finished);
}
#endif

CUSTOM_UI_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    View_ID view = get_active_view(app, Access_Always);
    //run_lister_buffer_list(app, "Switch:", activate_switch_buffer, 0, 0, view);
    Buffer_ID buffer = get_buffer_from_user(app, "Switch: ");
    if (buffer != 0){
        view_set_buffer(app, view, buffer, 0);
    }
}

#if 0
function Lister_Activation_Code
activate_kill_buffer(Application_Links *app,
                     View_ID view, Lister *lister,
                     String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    lister_default(app, view, lister, ListerActivation_Finished);
    if (user_data != 0){
        Buffer_ID buffer = (Buffer_ID)(PtrAsInt(user_data));
        try_buffer_kill(app, buffer, view, 0);
    }
    return(ListerActivation_Finished);
}
#endif

CUSTOM_UI_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    View_ID view = get_active_view(app, Access_Always);
    //run_lister_buffer_list(app, "Kill:", activate_kill_buffer, 0, 0, view);
    Buffer_ID buffer = get_buffer_from_user(app, "Kill: ");
    if (buffer != 0){
        try_buffer_kill(app, buffer, view, 0);
    }
}

function Lister_Activation_Code
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

function Lister_Activation_Code
activate_open_or_new(Application_Links *app,
                     View_ID view, Lister *lister,
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
        String_Const_u8 path = lister->text_field.string;
        if (!character_is_slash(string_get_character(path, path.size - 1))){
            path = string_remove_last_folder(path);
        }
        b32 is_folder = (character_is_slash(string_get_character(file_name, file_name.size - 1)) &&
                         user_data != 0);
        Buffer_Create_Flag flags = 0;
        result = activate_open_or_new__generic(app, view, path, file_name, is_folder, flags);
    }
    lister_default(app, view, lister, result);
    return(result);
}

CUSTOM_UI_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    View_ID view = get_active_view(app, Access_Always);
    run_lister_file_system_list(app, "Open:", activate_open_or_new, 0, 0, view);
}

function Lister_Activation_Code
activate_new(Application_Links *app,
             View_ID view, Lister *lister,
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
        String_Const_u8 path = lister->text_field.string;
        if (character_is_slash(string_get_character(path, path.size - 1))){
            path = string_remove_last_folder(path);
        }
        b32 is_folder = (character_is_slash(string_get_character(file_name, file_name.size - 1)) &&
                         user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_AlwaysNew;
        result = activate_open_or_new__generic(app, view, path, file_name, is_folder, flags);
    }
    lister_default(app, view, lister, result);
    return(result);
}

CUSTOM_UI_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    View_ID view = get_active_view(app, Access_Always);
    run_lister_file_system_list(app, "New:", activate_new, 0, 0, view);
}

function Lister_Activation_Code
activate_open(Application_Links *app,
              View_ID view, Lister *lister,
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
        String_Const_u8 path = lister->text_field.string;
        if (!character_is_slash(string_get_character(path, path.size - 1))){
            path = string_remove_last_folder(path);
        }
        b32 is_folder = (character_is_slash(string_get_character(file_name, file_name.size - 1)) &&
                         user_data != 0);
        Buffer_Create_Flag flags = BufferCreate_NeverNew;
        result = activate_open_or_new__generic(app, view, path, file_name, is_folder, flags);
    }
    lister_default(app, view, lister, result);
    return(result);
}

CUSTOM_UI_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    View_ID view = get_active_view(app, Access_Always);
    run_lister_file_system_list(app, "Open:", activate_open, 0, 0, view);
}

#if 0
function Lister_Activation_Code
activate_select_theme(Application_Links *app,
                      View_ID view, struct Lister *lister,
                      String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    change_theme_by_index(app, (i32)PtrAsInt(user_data));
    lister_default(app, scratch, view, state, ListerActivation_Finished);
    return(ListerActivation_Finished);
}

CUSTOM_COMMAND_SIG(open_color_tweaker)
CUSTOM_DOC("Opens the 4coder theme selector list.")
{
    
}
#endif

////////////////////////////////

function Lister_Activation_Code
activate_command(Application_Links *app,
                 View_ID view, Lister *lister,
                 String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    (*(Custom_Command_Function**)lister->user_data) = (Custom_Command_Function*)user_data;
    lister_default(app, view, lister, ListerActivation_Finished);
    return(ListerActivation_Finished);
}

function void
launch_custom_command_lister(Application_Links *app, i32 *command_ids, i32 command_id_count){
    if (command_ids == 0){
        command_id_count = command_one_past_last_id;
    }
    
    Scratch_Block scratch(app, Scratch_Share);
    View_ID view = get_active_view(app, Access_Always);
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
    Custom_Command_Function *custom_cmd = 0;
    run_lister_with_options_array(app, "Command:", activate_command, &custom_cmd, sizeof(custom_cmd),
                                  options, command_id_count, 0, view);
    if (custom_cmd != 0){
		animate_in_n_milliseconds(app, 0);
        custom_cmd(app);
    }
}

CUSTOM_UI_COMMAND_SIG(command_lister)
CUSTOM_DOC("Opens an interactive list of all registered commands.")
{
    launch_custom_command_lister(app, 0, 0);
}

// BOTTOM

