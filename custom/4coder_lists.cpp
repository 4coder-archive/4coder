/*
4coder_lists.cpp - List helpers and list commands such as:
open file, switch buffer, or kill buffer.
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
    Scratch_Block scratch(app, lister->arena);
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
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.refresh = generate_all_buffers_list;
    Lister_Result l_result = run_lister_with_refresh_handler(app, query, handlers);
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

typedef i32 Command_Lister_Status_Mode;
enum{
    CommandLister_None,
    CommandLister_Descriptions,
    CommandLister_Bindings
};

struct Command_Lister_Status_Rule{
    Command_Lister_Status_Mode mode;
    Mapping *mapping;
    Command_Map_ID map_id;
};

function Command_Lister_Status_Rule
command_lister_status_descriptions(void){
    Command_Lister_Status_Rule result = {};
    result.mode = CommandLister_Descriptions;
    return(result);
}

function Command_Lister_Status_Rule
command_lister_status_bindings(Mapping *mapping, Command_Map_ID map_id){
    Command_Lister_Status_Rule result = {};
    result.mode = CommandLister_Bindings;
    result.mapping = mapping;
    result.map_id = map_id;
    return(result);
}

function Custom_Command_Function*
get_command_from_user(Application_Links *app, String_Const_u8 query, i32 *command_ids, i32 command_id_count, Command_Lister_Status_Rule *status_rule){
    if (command_ids == 0){
        command_id_count = command_one_past_last_id;
    }
    
    Scratch_Block scratch(app);
    Lister_Block lister(app, scratch);
    lister_set_query(lister, query);
    lister_set_default_handlers(lister);
    
    for (i32 i = 0; i < command_id_count; i += 1){
        i32 j = i;
        if (command_ids != 0){
            j = command_ids[i];
        }
        j = clamp(0, j, command_one_past_last_id);
        
        Custom_Command_Function *proc = fcoder_metacmd_table[j].proc;
        String_Const_u8 status = {};
        switch (status_rule->mode){
            case CommandLister_Descriptions:
            {
                status = SCu8(fcoder_metacmd_table[j].description);
            }break;
            case CommandLister_Bindings:
            {
                Command_Trigger_List triggers = map_get_triggers_recursive(scratch, status_rule->mapping, status_rule->map_id, proc);
                
                List_String_Const_u8 list = {};
                for (Command_Trigger *node = triggers.first;
                     node != 0;
                     node = node->next){
                    command_trigger_stringize(scratch, &list, node);
                    if (node->next != 0){
                        string_list_push(scratch, &list, string_u8_litexpr(" "));
                    }
                }
                
                status = string_list_flatten(scratch, list);
            }break;
        }
        
        lister_add_item(lister, SCu8(fcoder_metacmd_table[j].name), status,
                        (void*)proc, 0);
    }
    
    Lister_Result l_result = run_lister(app, lister);
    
    Custom_Command_Function *result = 0;
    if (!l_result.canceled){
        result = (Custom_Command_Function*)l_result.user_data;
    }
    return(result);
}

function Custom_Command_Function*
get_command_from_user(Application_Links *app, String_Const_u8 query, Command_Lister_Status_Rule *status_rule){
    return(get_command_from_user(app, query, 0, 0, status_rule));
}

function Custom_Command_Function*
get_command_from_user(Application_Links *app, char *query,
                      i32 *command_ids, i32 command_id_count, Command_Lister_Status_Rule *status_rule){
    return(get_command_from_user(app, SCu8(query), command_ids, command_id_count, status_rule));
}

function Custom_Command_Function*
get_command_from_user(Application_Links *app, char *query, Command_Lister_Status_Rule *status_rule){
    return(get_command_from_user(app, SCu8(query), 0, 0, status_rule));
}

////////////////////////////////

function Color_Table*
get_color_table_from_user(Application_Links *app, String_Const_u8 query, Color_Table_List *color_table_list){
    if (color_table_list == 0){
        color_table_list = &global_theme_list;
    }
    
    Scratch_Block scratch(app);
    Lister_Block lister(app, scratch);
    lister_set_query(lister, query);
    lister_set_default_handlers(lister);
    
    lister_add_item(lister, string_u8_litexpr("4coder"), string_u8_litexpr(""),
                    (void*)&default_color_table, 0);
    
    for (Color_Table_Node *node = color_table_list->first;
         node != 0;
         node = node->next){
        lister_add_item(lister, node->name, string_u8_litexpr(""),
                        (void*)&node->table, 0);
    }
    
    Lister_Result l_result = run_lister(app, lister);
    
    Color_Table *result = 0;
    if (!l_result.canceled){
        result = (Color_Table*)l_result.user_data;
    }
    return(result);
}

function Color_Table*
get_color_table_from_user(Application_Links *app){
    return(get_color_table_from_user(app, string_u8_litexpr("Theme:"), 0));
}

////////////////////////////////

function Lister_Activation_Code
lister__write_character__file_path(Application_Links *app){
    Lister_Activation_Code result = ListerActivation_Continue;
    View_ID view = get_this_ctx_view(app, Access_Always);
    Lister *lister = view_get_lister(app, view);
    if (lister != 0){
        User_Input in = get_current_input(app);
        String_Const_u8 string = to_writable(&in);
        if (string.str != 0 && string.size > 0){
            lister_append_text_field(lister, string);
            if (character_is_slash(string.str[0])){
                lister->out.text_field = lister->text_field.string;
                result = ListerActivation_Finished;
            }
            else{
                String_Const_u8 front_name = string_front_of_path(lister->text_field.string);
                lister_set_key(lister, front_name);
            }
            lister->item_index = 0;
            lister_zero_scroll(lister);
            lister_update_filtered_list(app, lister);
        }
    }
    return(result);
}

function void
lister__backspace_text_field__file_path(Application_Links *app){
    View_ID view = get_this_ctx_view(app, Access_Always);
    Lister *lister = view_get_lister(app, view);
    if (lister != 0){
        if (lister->text_field.size > 0){
            char last_char = lister->text_field.str[lister->text_field.size - 1];
            lister->text_field.string = backspace_utf8(lister->text_field.string);
            if (character_is_slash(last_char)){
                User_Input input = get_current_input(app);
                String_Const_u8 text_field = lister->text_field.string;
                String_Const_u8 new_hot = string_remove_last_folder(text_field);
                b32 is_modified = has_modifier(&input, KeyCode_Control);
                b32 whole_word_when_mod = def_get_config_b32(vars_save_string_lit("lister_whole_word_backspace_when_modified"));
                b32 whole_word_backspace = (is_modified == whole_word_when_mod);
                if (whole_word_backspace){
                    lister->text_field.size = new_hot.size;
                }
                set_hot_directory(app, new_hot);
                // TODO(allen): We have to protect against lister_call_refresh_handler
                // changing the text_field here. Clean this up.
                String_u8 dingus = lister->text_field;
                lister_call_refresh_handler(app, lister);
                lister->text_field = dingus;
            }
            else{
                String_Const_u8 text_field = lister->text_field.string;
                String_Const_u8 new_key = string_front_of_path(text_field);
                lister_set_key(lister, new_key);
            }
            
            lister->item_index = 0;
            lister_zero_scroll(lister);
            lister_update_filtered_list(app, lister);
        }
    }
}

function void
generate_hot_directory_file_list(Application_Links *app, Lister *lister){
    Scratch_Block scratch(app, lister->arena);
    
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

struct File_Name_Result{
    b32 canceled;
    b32 clicked;
    b32 is_folder;
    String_Const_u8 file_name_activated;
    String_Const_u8 file_name_in_text_field;
    String_Const_u8 path_in_text_field;
};

function File_Name_Result
get_file_name_from_user(Application_Links *app, Arena *arena, String_Const_u8 query, View_ID view){
    Lister_Handlers handlers = lister_get_default_handlers();
    handlers.refresh = generate_hot_directory_file_list;
    handlers.write_character = lister__write_character__file_path;
    handlers.backspace = lister__backspace_text_field__file_path;
    
    Lister_Result l_result = run_lister_with_refresh_handler(app, arena, query, handlers);
    
    File_Name_Result result = {};
    result.canceled = l_result.canceled;
    if (!l_result.canceled){
        result.clicked = l_result.activated_by_click;
        if (l_result.user_data != 0){
            String_Const_u8 name = SCu8((u8*)l_result.user_data);
            result.file_name_activated = name;
            result.is_folder = character_is_slash(string_get_character(name, name.size - 1));
        }
        result.file_name_in_text_field = string_front_of_path(l_result.text_field);
        
        String_Const_u8 path = {};
        if (l_result.user_data == 0 && result.file_name_in_text_field.size == 0 && l_result.text_field.size > 0){
            result.file_name_in_text_field = string_front_folder_of_path(l_result.text_field);
            path = string_remove_front_folder_of_path(l_result.text_field);
        }
        else{
            path = string_remove_front_of_path(l_result.text_field);
        }
        if (character_is_slash(string_get_character(path, path.size - 1))){
            path = string_chop(path, 1);
        }
        result.path_in_text_field = path;
    }
    
    return(result);
}

function File_Name_Result
get_file_name_from_user(Application_Links *app, Arena *arena, char *query, View_ID view){
    return(get_file_name_from_user(app, arena, SCu8(query), view));
}

////////////////////////////////

enum{
    SureToKill_NULL = 0,
    SureToKill_No = 1,
    SureToKill_Yes = 2,
    SureToKill_Save = 3,
};

function b32
do_buffer_kill_user_check(Application_Links *app, Buffer_ID buffer, View_ID view){
    Scratch_Block scratch(app);
    Lister_Choice_List list = {};
    lister_choice(scratch, &list, "(N)o"  , "", KeyCode_N, SureToKill_No);
    lister_choice(scratch, &list, "(Y)es" , "", KeyCode_Y, SureToKill_Yes);
    lister_choice(scratch, &list, "(S)ave", "", KeyCode_S, SureToKill_Save);
    
    Lister_Choice *choice = get_choice_from_user(app, "There are unsaved changes, close anyway?", list);
    
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

function b32
do_4coder_close_user_check(Application_Links *app, View_ID view){
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

CUSTOM_UI_COMMAND_SIG(interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
    Buffer_ID buffer = get_buffer_from_user(app, "Switch:");
    if (buffer != 0){
        View_ID view = get_this_ctx_view(app, Access_Always);
        view_set_buffer(app, view, buffer, 0);
    }
}

CUSTOM_UI_COMMAND_SIG(interactive_kill_buffer)
CUSTOM_DOC("Interactively kill an open buffer.")
{
    Buffer_ID buffer = get_buffer_from_user(app, "Kill:");
    if (buffer != 0){
        View_ID view = get_this_ctx_view(app, Access_Always);
        try_buffer_kill(app, buffer, view, 0);
    }
}

////////////////////////////////

enum{
    SureToCreateFolder_NULL = 0,
    SureToCreateFolder_No = 1,
    SureToCreateFolder_Yes = 2,
};

function b32
query_create_folder(Application_Links *app, String_Const_u8 folder_name){
    Scratch_Block scratch(app);
    Lister_Choice_List list = {};
    lister_choice(scratch, &list, "(N)o"  , "", KeyCode_N, SureToKill_No);
    lister_choice(scratch, &list, "(Y)es" , "", KeyCode_Y, SureToKill_Yes);
    
    String_Const_u8 message = push_u8_stringf(scratch, "Create the folder %.*s?", string_expand(folder_name));
    Lister_Choice *choice = get_choice_from_user(app, message, list);
    
    b32 did_create_folder = false;
    if (choice != 0){
        switch (choice->user_data){
            case SureToCreateFolder_No:
            {}break;
            
            case SureToCreateFolder_Yes:
            {
                String_Const_u8 hot = push_hot_directory(app, scratch);
                String_Const_u8 fixed_folder_name = folder_name;
                for (;fixed_folder_name.size > 0 &&
                     character_is_slash(fixed_folder_name.str[fixed_folder_name.size - 1]);){
                    fixed_folder_name = string_chop(fixed_folder_name, 1);
                }
                if (fixed_folder_name.size > 0){
                    String_Const_u8 cmd = push_u8_stringf(scratch, "mkdir %.*s", string_expand(fixed_folder_name));
                    exec_system_command(app, 0, buffer_identifier(0), hot, cmd, 0);
                    did_create_folder = true;
                }
            }break;
        }
    }
    
    return(did_create_folder);
}

////////////////////////////////

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
        Scratch_Block scratch(app);
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

CUSTOM_UI_COMMAND_SIG(interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system.")
{
    for (;;){
        Scratch_Block scratch(app);
        View_ID view = get_this_ctx_view(app, Access_Always);
        File_Name_Result result = get_file_name_from_user(app, scratch, "Open:", view);
        if (result.canceled) break;
        
        String_Const_u8 file_name = result.file_name_activated;
        if (file_name.size == 0){
            file_name = result.file_name_in_text_field;
        }
        if (file_name.size == 0) break;
        
        String_Const_u8 path = result.path_in_text_field;
        String_Const_u8 full_file_name = push_u8_stringf(scratch, "%.*s/%.*s",
                                                         string_expand(path), string_expand(file_name));
        
        if (result.is_folder){
            set_hot_directory(app, full_file_name);
            continue;
        }
        
        if (character_is_slash(file_name.str[file_name.size - 1])){
            File_Attributes attribs = system_quick_file_attributes(scratch, full_file_name);
            if (HasFlag(attribs.flags, FileAttribute_IsDirectory)){
                set_hot_directory(app, full_file_name);
                continue;
			}
			if (string_looks_like_drive_letter(file_name)){
				set_hot_directory(app, file_name);
				continue;
			}
            if (query_create_folder(app, file_name)){
                set_hot_directory(app, full_file_name);
                continue;
            }
            break;
        }
        
        Buffer_ID buffer = create_buffer(app, full_file_name, 0);
        if (buffer != 0){
            view_set_buffer(app, view, buffer, 0);
        }
        break;
    }
}

CUSTOM_UI_COMMAND_SIG(interactive_new)
CUSTOM_DOC("Interactively creates a new file.")
{
    for (;;){
        Scratch_Block scratch(app);
        View_ID view = get_this_ctx_view(app, Access_Always);
        File_Name_Result result = get_file_name_from_user(app, scratch, "New:",
                                                          view);
        if (result.canceled) break;
        
        // NOTE(allen): file_name from the text field always
        // unless this is a folder or a mouse click.
        String_Const_u8 file_name = result.file_name_in_text_field;
        if (result.is_folder || result.clicked){
            file_name = result.file_name_activated;
        }
        if (file_name.size == 0) break;
        
        String_Const_u8 path = result.path_in_text_field;
        String_Const_u8 full_file_name =
            push_u8_stringf(scratch, "%.*s/%.*s",
                            string_expand(path), string_expand(file_name));
        
        if (result.is_folder){
            set_hot_directory(app, full_file_name);
            continue;
        }
        
        if (character_is_slash(file_name.str[file_name.size - 1])){
            File_Attributes attribs = system_quick_file_attributes(scratch, full_file_name);
            if (HasFlag(attribs.flags, FileAttribute_IsDirectory)){
                set_hot_directory(app, full_file_name);
                continue;
			}
			if (string_looks_like_drive_letter(file_name)){
				set_hot_directory(app, file_name);
				continue;
			}
            if (query_create_folder(app, file_name)){
                set_hot_directory(app, full_file_name);
                continue;
            }
            break;
        }
        
        Buffer_Create_Flag flags = BufferCreate_AlwaysNew;
        Buffer_ID buffer = create_buffer(app, full_file_name, flags);
        if (buffer != 0){
            view_set_buffer(app, view, buffer, 0);
        }
        break;
    }
}

CUSTOM_UI_COMMAND_SIG(interactive_open)
CUSTOM_DOC("Interactively opens a file.")
{
    for (;;){
        Scratch_Block scratch(app);
        View_ID view = get_this_ctx_view(app, Access_Always);
        File_Name_Result result = get_file_name_from_user(app, scratch, "Open:", view);
        if (result.canceled) break;
        
        String_Const_u8 file_name = result.file_name_activated;
        if (file_name.size == 0) break;
        
        String_Const_u8 path = result.path_in_text_field;
        String_Const_u8 full_file_name =
            push_u8_stringf(scratch, "%.*s/%.*s",
                            string_expand(path), string_expand(file_name));
        
        if (result.is_folder){
            set_hot_directory(app, full_file_name);
            continue;
        }
        
        if (character_is_slash(file_name.str[file_name.size - 1])){
            File_Attributes attribs = system_quick_file_attributes(scratch, full_file_name);
            if (HasFlag(attribs.flags, FileAttribute_IsDirectory)){
                set_hot_directory(app, full_file_name);
                continue;
            }
            if (query_create_folder(app, file_name)){
                set_hot_directory(app, full_file_name);
                continue;
            }
            break;
        }
        
        Buffer_Create_Flag flags = BufferCreate_NeverNew;
        Buffer_ID buffer = create_buffer(app, full_file_name, flags);
        if (buffer != 0){
            view_set_buffer(app, view, buffer, 0);
        }
        break;
    }
}

////////////////////////////////

CUSTOM_UI_COMMAND_SIG(command_lister)
CUSTOM_DOC("Opens an interactive list of all registered commands.")
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    if (view != 0){
        Command_Lister_Status_Rule rule = {};
        Buffer_ID buffer = view_get_buffer(app, view, Access_Visible);
        Managed_Scope buffer_scope = buffer_get_managed_scope(app, buffer);
        Command_Map_ID *map_id_ptr = scope_attachment(app, buffer_scope, buffer_map_id, Command_Map_ID);
        if (map_id_ptr != 0){
            rule = command_lister_status_bindings(&framework_mapping, *map_id_ptr);
        }
        else{
            rule = command_lister_status_descriptions();
        }
        Custom_Command_Function *func = get_command_from_user(app, "Command:", &rule);
        if (func != 0){
            view_enqueue_command_function(app, view, func);
        }
    }
}

////////////////////////////////

CUSTOM_UI_COMMAND_SIG(theme_lister)
CUSTOM_DOC("Opens an interactive list of all registered themes.")
{
    Color_Table *color_table = get_color_table_from_user(app);
    if (color_table != 0){
        active_color_table = *color_table;
    }
}

// BOTTOM

