/*
 * Helpers for the API transition from 4.0.30 to 4.0.31
 *
 * In order to keep your layer on the old API you don't have to do anything, this provides wrappers
 *  idential to the 4.0.30 API.
 * In order to transition your entire layer over to the 4.0.31 API define 'REMOVE_TRANSITION_HELPER' and fix errors.
 * Or you can do it step by step by removing a few wrappers at a time.
 * This transition helper will be removed in a future version so it is recommended to get off sooner or laster.
 *
 * Tips on transitioning:
*
* Wrather than just try to inline this code everywhere, you can simplify things quite a lot by storing references
* to buffers and views and Buffer_ID and View_ID instead of Buffer_Summary and View_Summary.
 * Just get the summaries when you need information in those structures.
 *
 * You will make your code simpler if you stick to String as much as possible, but whenever you want to you can switch
 * to any string type you have to String by calling make_string(char_ptr, length) or make_string_slowly(null_terminated_c_str).
 * To pull the char ptr and length out of a String named "string": string.str and str.size.
 * If you need a null terminated string from a String use get_null_terminated in 4coder_helper.cpp
 *
 */

// TOP

#if !defined(REMOVE_TRANSITION_HELPER)

static bool32
exec_system_command(Application_Links *app, View_Summary *view, Buffer_Identifier buffer_id,
                    char *path, int32_t path_len, char *command, int32_t command_len, Command_Line_Interface_Flag flags){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = exec_system_command(app, view->view_id, buffer_id, make_string(path, path_len), make_string(command, command_len), flags);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
clipboard_post(Application_Links *app, int32_t clipboard_id, char *str, int32_t len){
    return(clipboard_post(app, clipboard_id, make_string(str, len)));
}

static int32_t
clipboard_count(Application_Links *app, int32_t clipboard_id){
    int32_t count = 0;
    clipboard_count(app, clipboard_id, &count);
    return(count);
}

static int32_t
clipboard_index(Application_Links *app, int32_t clipboard_id, int32_t item_index, char *out, int32_t len){
    int32_t required_size = 0;
    String string = make_string_cap(out, 0, len);
    clipboard_index(app, clipboard_id, item_index, &string, &required_size);
    return(required_size);
}

static Buffer_Summary
get_buffer_first(Application_Links *app, Access_Flag access){
    Buffer_ID buffer_id = 0;
    Buffer_Summary buffer = {};
    if (get_buffer_first(app, access, &buffer_id)){
        get_buffer_summary(app, buffer_id, access, &buffer);
    }
    return(buffer);
}

static void
get_buffer_next(Application_Links *app, Buffer_Summary *buffer, Access_Flag access){
    if (buffer != 0 && buffer->exists){
        Buffer_ID buffer_id = 0;
        if (get_buffer_next(app, buffer->buffer_id, access, &buffer_id)){
            get_buffer_summary(app, buffer_id, access, buffer);
        }
        else{
            memset(buffer, 0, sizeof(*buffer));
        }
    }
}

static Buffer_Summary
get_buffer(Application_Links *app, Buffer_ID buffer_id, Access_Flag access){
    Buffer_Summary buffer = {};
    get_buffer_summary(app, buffer_id, access, &buffer);
    return(buffer);
}

static Buffer_Summary
get_buffer_by_name(Application_Links *app, char *name, int32_t len, Access_Flag access){
    Buffer_ID id = 0;
    Buffer_Summary buffer = {};
    if (get_buffer_by_name(app, make_string(name, len), access, &id)){
        get_buffer_summary(app, id, access, &buffer);
    }
    return(buffer);
}

static Buffer_Summary
get_buffer_by_file_name(Application_Links *app, char *name, int32_t len, Access_Flag access){
    Buffer_ID id = 0;
    Buffer_Summary buffer = {};
    if (get_buffer_by_file_name(app, make_string(name, len), access, &id)){
        get_buffer_summary(app, id, access, &buffer);
    }
    return(buffer);
}

static bool32
buffer_read_range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t one_past_last, char *out){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_read_range(app, buffer->buffer_id, start, one_past_last, out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_replace_range(Application_Links *app, Buffer_Summary *buffer, int32_t start, int32_t one_past_last, char *str, int32_t len){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_replace_range(app, buffer->buffer_id, start, one_past_last, make_string(str, len));
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_compute_cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_compute_cursor(app, buffer->buffer_id, seek, cursor_out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_batch_edit(Application_Links *app, Buffer_Summary *buffer, char *str, int32_t str_len, Buffer_Edit *edits, int32_t edit_count, Buffer_Batch_Edit_Type type){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_batch_edit(app, buffer->buffer_id, str, str_len, edits, edit_count, type);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_get_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t *value_out){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_get_setting(app, buffer->buffer_id, setting, value_out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_set_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, int32_t value){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_set_setting(app, buffer->buffer_id, setting, value);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static Managed_Scope
buffer_get_managed_scope(Application_Links *app, Buffer_ID buffer_id){
    Managed_Scope scope = 0;
    buffer_get_managed_scope(app, buffer_id, &scope);
    return(scope);
}

static int32_t
buffer_token_count(Application_Links *app, Buffer_Summary *buffer){
    int32_t count = 0;
    if (buffer != 0 && buffer->exists){
        buffer_token_count(app, buffer->buffer_id, &count);
    }
    return(count);
}

static bool32
buffer_read_tokens(Application_Links *app, Buffer_Summary *buffer, int32_t start_token, int32_t end_token, Cpp_Token *tokens_out){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_read_tokens(app, buffer->buffer_id, start_token, end_token, tokens_out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_get_token_range(Application_Links *app, Buffer_Summary *buffer, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_get_token_range(app, buffer->buffer_id, first_token_out, one_past_last_token_out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_get_token_index(Application_Links *app, Buffer_Summary *buffer, int32_t pos, Cpp_Get_Token_Result *get_result){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_get_token_index(app, buffer->buffer_id, pos, get_result);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static bool32
buffer_send_end_signal(Application_Links *app, Buffer_Summary *buffer){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_send_end_signal(app, buffer->buffer_id);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static Buffer_Summary
create_buffer(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags){
    Buffer_Summary buffer = {};
    Buffer_ID buffer_id = 0;
    if (create_buffer(app, make_string(filename, filename_len), flags, &buffer_id)){
        get_buffer_summary(app, buffer_id, AccessAll, &buffer);
    }
    return(buffer);
}

static bool32
save_buffer(Application_Links *app, Buffer_Summary *buffer, char *file_name, int32_t file_name_len, uint32_t flags){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_save(app, buffer->buffer_id, make_string(file_name, file_name_len), flags);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static Buffer_Kill_Result
kill_buffer(Application_Links *app, Buffer_Identifier buffer, Buffer_Kill_Flag flags){
    Buffer_Kill_Result result = 0;
    if (buffer.id != 0){
        buffer_kill(app, buffer.id, flags, &result);
    }
    else if (buffer.name != 0){
        Buffer_ID id = 0;
        if (get_buffer_by_name(app, make_string(buffer.name, buffer.name_len), AccessAll, &id)){
            buffer_kill(app, id, flags, &result);
        }
    }
    return(result);
}

static Buffer_Reopen_Result
reopen_buffer(Application_Links *app, Buffer_Summary *buffer, Buffer_Reopen_Flag flags){
    Buffer_Kill_Result result = 0;
    if (buffer != 0 && buffer->exists){
        buffer_reopen(app, buffer->buffer_id, flags, &result);
    }
    return(result);
}

static View_Summary
get_view_first(Application_Links *app, Access_Flag access){
    View_Summary view = {};
    View_ID view_id = 0;
    if (get_view_first(app, access, &view_id)){
        get_view_summary(app, view_id, access, &view);
    }
    return(view);
}

static void
get_view_next(Application_Links *app, View_Summary *view, Access_Flag access){
    if (view != 0 && view->exists){
        View_ID view_id = 0;
        if (get_view_next(app, view->view_id, access, &view_id)){
            get_view_summary(app, view_id, access, view);
        }
        else{
            memset(view, 0, sizeof(*view));
        }
    }
}

static View_Summary
get_view(Application_Links *app, View_ID view_id, Access_Flag access){
    View_Summary view = {};
    get_view_summary(app, view_id, access, &view);
    return(view);
}

static View_Summary
get_active_view(Application_Links *app, Access_Flag access){
    View_Summary view = {};
    View_ID id = 0;
    if (get_active_view(app, access, &id)){
        get_view_summary(app, id, access, &view);
    }
    return(view);
}

static View_Summary
open_view(Application_Links *app, View_Summary *view_location, View_Split_Position position){
    View_Summary view = {};
    if (view_location != 0 && view_location->exists){
        Panel_ID panel_id = 0;
        if (view_get_panel(app, view_location->view_id, &panel_id)){
            bool32 vertical = (position == ViewSplit_Left || position == ViewSplit_Right);
            if (panel_split(app, panel_id, vertical?PanelSplit_LeftAndRight:PanelSplit_TopAndBottom)){
                Panel_ID left_panel_id = 0;
                if (panel_get_child(app, panel_id, PanelChild_Min, &left_panel_id)){
                    View_ID new_view_id = 0;
                    if (panel_get_view(app, left_panel_id, &new_view_id)){
                        get_view_summary(app, new_view_id, AccessAll, view_location);
                    }
                }
            }
        }
    }
    return(view);
}

static bool32
close_view(Application_Links *app, View_Summary *view){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_close(app, view->view_id);
    }
    return(result);
}

static bool32
set_active_view(Application_Links *app, View_Summary *view){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_set_active(app, view->view_id);
    }
    return(result);
}

static bool32
view_get_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t *value_out){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_get_setting(app, view->view_id, setting, value_out);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_set_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, int32_t value){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_set_setting(app, view->view_id, setting, value);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static Managed_Scope
view_get_managed_scope(Application_Links *app, View_ID view_id){
    Managed_Scope scope = 0;
    view_get_managed_scope(app, view_id, &scope);
    return(scope);
}

typedef int32_t View_Split_Kind;
enum{
    ViewSplitKind_Ratio,
    ViewSplitKind_FixedPixels,
};

static bool32
view_set_split(Application_Links *app, View_Summary *view, View_Split_Kind kind, float t){
    bool32 result = false;
    if (view != 0 && view->exists){
        Panel_ID panel_id = 0;
        if (view_get_panel(app, view->view_id, &panel_id)){
            result = panel_set_split(app, panel_id, kind, t);
            get_view_summary(app, view->view_id, AccessAll, view);
        }
    }
    return(result);
}

static i32_Rect
view_get_enclosure_rect(Application_Links *app, View_Summary *view){
    i32_Rect result = {};
    if (view != 0 && view->exists){
        view_get_enclosure_rect(app, view->view_id, &result);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_compute_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_compute_cursor(app, view->view_id, seek, cursor_out);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_set_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, bool32 set_preferred_x){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_set_cursor(app, view->view_id, seek, set_preferred_x);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_set_scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_set_scroll(app, view->view_id, scroll);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_set_mark(Application_Links *app, View_Summary *view, Buffer_Seek seek){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_set_mark(app, view->view_id, seek);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_set_buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_set_buffer(app, view->view_id, buffer_id, flags);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_post_fade(Application_Links *app, View_Summary *view, float seconds, int32_t start, int32_t end, int_color color){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_post_fade(app, view->view_id, seconds, start, end, color);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_begin_ui_mode(Application_Links *app, View_Summary *view){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_begin_ui_mode(app, view->view_id);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_end_ui_mode(Application_Links *app, View_Summary *view){
    bool32 result = false;
    if (view != 0 && view->exists){
        result = view_end_ui_mode(app, view->view_id);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static bool32
view_set_highlight(Application_Links *app, View_ID view_id, int32_t start, int32_t end, bool32 turn_on){
    // NOTE(allen): this feature is completely removed, transition to using highlighted markers instead
    return(false);
}

static bool32
buffer_set_face(Application_Links *app, Buffer_Summary *buffer, Face_ID id){
    bool32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_set_face(app, buffer->buffer_id, id);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static Face_ID
get_face_id(Application_Links *app, Buffer_Summary *buffer){
    Face_ID result = 0;
    if (buffer != 0 && buffer->exists){
        get_face_id(app, buffer->buffer_id, &result);
    }
    else{
        get_face_id(app, 0, &result);
    }
    return(result);
}

static void
print_message(Application_Links *app, char *str, int32_t len){
    print_message(app, make_string(str, len));
}

#if 0
static void
create_theme(Application_Links *app, Theme *theme, char *name, int32_t len){
    create_theme(app, theme, make_string(name, len));
}

static void
change_theme(Application_Links *app, char *name, int32_t len){
    change_theme(app, make_string(name, len));
}
#endif

static int32_t
directory_get_hot(Application_Links *app, char *out, int32_t capacity){
    int32_t required_size = 0;
    String string = make_string_cap(out, 0, capacity);
    get_hot_directory(app, &string, &required_size);
    return(required_size);
}

static bool32
directory_set_hot(Application_Links *app, char *str, int32_t len){
    return(set_hot_directory(app, make_string(str, len)));
}

static File_List
get_file_list(Application_Links *app, char *dir, int32_t len){
    File_List list = {};
    get_file_list(app, make_string(dir, len), &list);
    return(list);
}

static bool32
file_exists(Application_Links *app, char *file_name, int32_t len){
    File_Attributes attributes = {};
    file_get_attributes(app, make_string(file_name, len), &attributes);
    return(attributes.last_write_time > 0);
}

static bool32
directory_cd(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len){
    String directory = make_string_cap(dir, *len, capacity);
    String relative_path = make_string(rel_path, rel_len);
    bool32 result = directory_cd(app, &directory, relative_path);
    *len = directory.size;
    return(result);
}

static int32_t
get_4ed_path(Application_Links *app, char *out, int32_t capacity){
    int32_t required_size = 0;
    String string = make_string_cap(out, 0, capacity);
    get_4ed_path(app, &string, &required_size);
    return(required_size);
}

static void
set_window_title(Application_Links *app, char *title){
    String title_string = make_string_slowly(title);
    set_window_title(app, title_string);
}

#endif

// BOTTOM

