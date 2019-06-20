/*
 * Helpers for the API transition from 4.0.30 to 4.0.31
 *
 * In order to keep your layer on the old API you don't have to do anything, this provides wrappers
 *  idential to the 4.0.30 API.
 * In order to transition your entire layer over to the 4.0.31 API define 'REMOVE_TRANSITION_HELPER_31' and fix errors.
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

#if !defined(REMOVE_TRANSITION_HELPER_31)

typedef b32 bool32;

static b32
get_buffer_summary(Application_Links *app, Buffer_ID buffer, Access_Flag access, Buffer_Summary *summary){
    b32 result = false;
    if (buffer_exists(app, buffer)){
        Scratch_Block scratch(app);
        Access_Flag buffer_access_flags = buffer_get_access_flags(app, buffer);
        if ((buffer_access_flags & ~access) == 0){
            result = true;
            summary->exists = true;
            summary->ready = buffer_ready(app, buffer);
            summary->buffer_id = buffer;
            summary->size = (i32)buffer_get_size(app, buffer);
            summary->line_count = (i32)buffer_get_line_count(app, buffer);
            
            String_Const_u8 file_name_get = push_buffer_file_name(app, scratch, buffer);
            block_copy(summary->file_name, file_name_get.str, file_name_get.size);
            summary->file_name_len = (i32)file_name_get.size;
            
            String_Const_u8 buffer_name_get = push_buffer_unique_name(app, scratch, buffer);
            block_copy(summary->buffer_name, buffer_name_get.str, buffer_name_get.size);
            summary->buffer_name_len = (i32)buffer_name_get.size;
            
            summary->dirty = buffer_get_dirty_state(app, buffer);
            buffer_get_setting(app, buffer, BufferSetting_Lex, &summary->is_lexed);
            summary->tokens_are_ready = buffer_tokens_are_ready(app, buffer);
            buffer_get_setting(app, buffer, BufferSetting_MapID, &summary->map_id);
            buffer_get_setting(app, buffer, BufferSetting_WrapLine, &summary->unwrapped_lines);
            summary->unwrapped_lines = !summary->unwrapped_lines;
            summary->lock_flags = buffer_access_flags;
        }
    }
    return(result);
}

static b32
get_view_summary(Application_Links *app, View_ID view_id, Access_Flag access, View_Summary *view){
    b32 result = false;
    if (view_exists(app, view_id)){
        Buffer_ID buffer = view_get_buffer(app, view_id, access);
        if (buffer != 0){
            result = true;
            
            Face_ID face_id = get_face_id(app, buffer);
            Face_Metrics metrics = get_face_metrics(app, face_id);
            
            view->exists = true;
            view->view_id = view_id;
            view->line_height = metrics.line_height;
            buffer_get_setting(app, buffer, BufferSetting_WrapLine, &view->unwrapped_lines);
            view->unwrapped_lines = !view->unwrapped_lines;
            view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &view->show_whitespace);
            view->buffer_id = buffer;
            i64 pos = view_get_mark_pos(app, view_id);
            view->mark = view_compute_cursor(app, view_id, seek_pos(pos));
            pos = view_get_cursor_pos(app, view_id);
            view->cursor = view_compute_cursor(app, view_id, seek_pos(pos));
            view->preferred_x = view_get_preferred_x(app, view_id);
            Rect_f32 screen_rect = view_get_screen_rect(app, view_id);
            view->view_region = screen_rect;
            view->render_region = Rf32(0.f, 0.f, rect_width(screen_rect), rect_height(screen_rect));
            view->scroll_vars = view_get_scroll_vars(app, view_id);
        }
    }
    return(result);
}

static b32
clipboard_post(Application_Links *app, i32 clipboard_id, char *str, i32 len){
    return(clipboard_post(app, clipboard_id, SCu8(str, len)));
}

static i32
clipboard_index(Application_Links *app, i32 clipboard_id, i32 item_index, char *out, i32 len){
    Scratch_Block scratch(app);
    String_Const_u8 string = push_clipboard_index(app, scratch, clipboard_id, item_index);
    block_copy(out, string.str, clamp_top((i32)string.size, len));
    return((i32)string.size);
}

static Buffer_Summary
get_buffer_first(Application_Links *app, Access_Flag access){
    Buffer_Summary buffer = {};
    Buffer_ID buffer_id = get_buffer_next(app, 0, access);
    if (buffer_id != 0){
        get_buffer_summary(app, buffer_id, access, &buffer);
    }
    return(buffer);
}

static void
get_buffer_next(Application_Links *app, Buffer_Summary *buffer, Access_Flag access){
    if (buffer != 0 && buffer->exists){
        Buffer_ID buffer_id = get_buffer_next(app, buffer->buffer_id, access);
        if (buffer_id != 0){
            get_buffer_summary(app, buffer_id, access, buffer);
        }
        else{
            block_zero_struct(buffer);
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
get_buffer_by_name(Application_Links *app, char *name, i32 len, Access_Flag access){
    Buffer_Summary buffer = {};
    Buffer_ID id = get_buffer_by_name(app, SCu8(name, len), access);
    if (id != 0){
        get_buffer_summary(app, id, access, &buffer);
    }
    return(buffer);
}

static Buffer_Summary
get_buffer_by_file_name(Application_Links *app, char *name, i32 len, Access_Flag access){
    Buffer_Summary buffer = {};
    Buffer_ID id = get_buffer_by_file_name(app, SCu8(name, len), access);
    if (id != 0){
        get_buffer_summary(app, id, access, &buffer);
    }
    return(buffer);
}

static b32
buffer_read_range(Application_Links *app, Buffer_Summary *buffer, i32 start, i32 one_past_last, char *out){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_read_range(app, buffer->buffer_id, Ii64(start, one_past_last), out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_replace_range(Application_Links *app, Buffer_Summary *buffer, i32 start, i32 one_past_last, char *str, i32 len){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_replace_range(app, buffer->buffer_id, Ii64(start, one_past_last), SCu8(str, len));
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_compute_cursor(Application_Links *app, Buffer_Summary *buffer, Buffer_Seek seek, Partial_Cursor *cursor_out){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        *cursor_out = buffer_compute_cursor(app, buffer->buffer_id, seek);
        result = (cursor_out->line > 0);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_batch_edit(Application_Links *app, Buffer_Summary *buffer, char *str, i32 str_len, Buffer_Edit *edits, i32 edit_count, i32 type){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_batch_edit(app, buffer->buffer_id, str, edits, edit_count);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_get_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, i32 *value_out){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_get_setting(app, buffer->buffer_id, setting, value_out);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_set_setting(Application_Links *app, Buffer_Summary *buffer, Buffer_Setting_ID setting, i32 value){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_set_setting(app, buffer->buffer_id, setting, value);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static i32
buffer_token_count(Application_Links *app, Buffer_Summary *buffer){
    i32 count = 0;
    if (buffer != 0 && buffer->exists){
        count = buffer_get_token_array(app, buffer->buffer_id).count;
    }
    return(count);
}

static b32
buffer_read_tokens(Application_Links *app, Buffer_ID buffer, i32 start_token, i32 end_token, Cpp_Token *tokens_out){
    b32 result = false;
    if (buffer != 0){
        Cpp_Token_Array array = buffer_get_token_array(app, buffer);
        if (0 <= start_token && start_token <= end_token && end_token < array.count){
            result = true;
            block_copy_dynamic_array(tokens_out, array.tokens + start_token, end_token - start_token);
        }
    }
    return(result);
}

static b32
buffer_read_tokens(Application_Links *app, Buffer_Summary *buffer, i32 start_token, i32 end_token, Cpp_Token *tokens_out){
    Buffer_ID buffer_id = (buffer==0?0:buffer->buffer_id);
    b32 result = buffer_read_tokens(app, buffer_id, start_token, end_token, tokens_out);
    get_buffer_summary(app, buffer_id, AccessAll, buffer);
    return(result);
}

static b32
buffer_get_token_range(Application_Links *app, Buffer_Summary *buffer, Cpp_Token **first_token_out, Cpp_Token **one_past_last_token_out){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        Cpp_Token_Array array = buffer_get_token_array(app, buffer->buffer_id);
        result = true;
        if (first_token_out != 0){
            *first_token_out = array.tokens;
        }
        if (one_past_last_token_out != 0){
            *one_past_last_token_out = array.tokens + array.count;
        }
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_get_token_index(Application_Links *app, Buffer_Summary *buffer, i32 pos, Cpp_Get_Token_Result *get_result){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        Cpp_Token_Array array = buffer_get_token_array(app, buffer->buffer_id);
        result = true;
        *get_result = cpp_get_token(array, pos);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static b32
buffer_send_end_signal(Application_Links *app, Buffer_Summary *buffer){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_send_end_signal(app, buffer->buffer_id);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static Buffer_Summary
create_buffer(Application_Links *app, char *filename, i32 filename_len, Buffer_Create_Flag flags){
    Buffer_Summary buffer = {};
    Buffer_ID buffer_id = create_buffer(app, SCu8(filename, filename_len), flags);
    if (buffer_id != 0){
        get_buffer_summary(app, buffer_id, AccessAll, &buffer);
    }
    return(buffer);
}

static b32
save_buffer(Application_Links *app, Buffer_Summary *buffer, char *file_name, i32 file_name_len, u32 flags){
    b32 result = false;
    if (buffer != 0 && buffer->exists){
        result = buffer_save(app, buffer->buffer_id, SCu8(file_name, file_name_len), flags);
        get_buffer_summary(app, buffer->buffer_id, AccessAll, buffer);
    }
    return(result);
}

static Buffer_Kill_Result
kill_buffer(Application_Links *app, Buffer_Identifier buffer, Buffer_Kill_Flag flags){
    Buffer_Kill_Result result = 0;
    if (buffer.id != 0){
        result = buffer_kill(app, buffer.id, flags);
    }
    else if (buffer.name != 0){
        Buffer_ID id = get_buffer_by_name(app, SCu8(buffer.name, buffer.name_len), AccessAll);
        if (id != 0){
            result = buffer_kill(app, id, flags);
        }
    }
    return(result);
}

static Buffer_Reopen_Result
reopen_buffer(Application_Links *app, Buffer_Summary *buffer, Buffer_Reopen_Flag flags){
    Buffer_Reopen_Result result = 0;
    if (buffer != 0 && buffer->exists){
        result = buffer_reopen(app, buffer->buffer_id, flags);
    }
    return(result);
}

static View_Summary
get_view_first(Application_Links *app, Access_Flag access){
    View_Summary view = {};
    View_ID view_id = get_view_next(app, 0, access);
    if (view_id != 0){
        get_view_summary(app, view_id, access, &view);
    }
    return(view);
}

static void
get_view_next(Application_Links *app, View_Summary *view, Access_Flag access){
    if (view != 0 && view->exists){
        View_ID view_id = get_view_next(app, view->view_id, access);
        if (view_id != 0){
            get_view_summary(app, view_id, access, view);
        }
        else{
            block_zero_struct(view);
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
get_active_view_DEP(Application_Links *app, Access_Flag access){
    View_Summary view = {};
    View_ID id = get_active_view(app, access);
    if (id != 0){
        get_view_summary(app, id, access, &view);
    }
    return(view);
}

static View_Summary
open_view(Application_Links *app, View_Summary *view_location, View_Split_Position position){
    View_Summary view = {};
    if (view_location != 0 && view_location->exists){
        Panel_ID panel_id = view_get_panel(app, view_location->view_id);
        if (panel_id != 0){
            b32 vertical = (position == ViewSplit_Left || position == ViewSplit_Right);
            if (panel_split(app, panel_id, vertical?PanelSplit_LeftAndRight:PanelSplit_TopAndBottom)){
                Panel_Child child = (position == ViewSplit_Left || position == ViewSplit_Top)?PanelChild_Min:PanelChild_Max;
                Panel_ID new_panel_id = panel_get_child(app, panel_id, child);
                if (new_panel_id != 0){
                    View_ID new_view_id = panel_get_view(app, new_panel_id);
                    if (new_view_id != 0){
                        get_view_summary(app, new_view_id, AccessAll, &view);
                        get_view_summary(app, view_location->view_id, AccessAll, view_location);
                    }
                }
            }
        }
    }
    return(view);
}

static b32
close_view(Application_Links *app, View_Summary *view){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_close(app, view->view_id);
    }
    return(result);
}

static b32
set_active_view(Application_Links *app, View_Summary *view){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_set_active(app, view->view_id);
    }
    return(result);
}

static b32
view_get_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, i32 *value_out){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_get_setting(app, view->view_id, setting, value_out);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_setting(Application_Links *app, View_Summary *view, View_Setting_ID setting, i32 value){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_set_setting(app, view->view_id, setting, value);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_compute_cursor_DEP(Application_Links *app, View_Summary *view, Buffer_Seek seek, Full_Cursor *cursor_out){
    b32 result = false;
    if (view != 0 && view->exists){
        *cursor_out = view_compute_cursor(app, view->view_id, seek);
        result = (cursor_out->line > 0);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_cursor(Application_Links *app, View_Summary *view, Buffer_Seek seek, b32 set_preferred_x){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_set_cursor(app, view->view_id, seek, set_preferred_x);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_scroll(Application_Links *app, View_Summary *view, GUI_Scroll_Vars scroll){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_set_scroll(app, view->view_id, scroll);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_mark(Application_Links *app, View_Summary *view, Buffer_Seek seek){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_set_mark(app, view->view_id, seek);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_buffer(Application_Links *app, View_Summary *view, Buffer_ID buffer_id, Set_Buffer_Flag flags){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_set_buffer(app, view->view_id, buffer_id, flags);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_post_fade(Application_Links *app, View_Summary *view, float seconds, i32 start, i32 end, int_color color){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_post_fade(app, view->view_id, seconds, Ii64(start, end), color);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_begin_ui_mode(Application_Links *app, View_Summary *view){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_begin_ui_mode(app, view->view_id);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_end_ui_mode(Application_Links *app, View_Summary *view){
    b32 result = false;
    if (view != 0 && view->exists){
        result = view_end_ui_mode(app, view->view_id);
        get_view_summary(app, view->view_id, AccessAll, view);
    }
    return(result);
}

static b32
view_set_highlight(Application_Links *app, View_ID view_id, i32 start, i32 end, b32 turn_on){
    // NOTE(allen): this feature is completely removed, transition to using highlighted markers instead
    return(false);
}

static b32
buffer_set_face(Application_Links *app, Buffer_Summary *buffer, Face_ID id){
    b32 result = false;
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
        result = get_face_id(app, buffer->buffer_id);
    }
    else{
        result = get_face_id(app, 0);
    }
    return(result);
}

static void
print_message(Application_Links *app, char *str, i32 len){
    print_message(app, SCu8(str, len));
}

#if 0
static void
create_theme(Application_Links *app, Theme *theme, char *name, i32 len){
    create_theme(app, theme, make_string(name, len));
}

static void
change_theme(Application_Links *app, char *name, i32 len){
    change_theme(app, make_string(name, len));
}
#endif

static i32
directory_get_hot(Application_Links *app, char *out, i32 capacity){
    Scratch_Block scratch(app);
    String_Const_u8 string = push_hot_directory(app, scratch);
    block_copy(out, string.str, clamp_top((i32)string.size, capacity));
    return((i32)string.size);
}

static b32
directory_set_hot(Application_Links *app, char *str, i32 len){
    return(set_hot_directory(app, SCu8(str, len)));
}

static File_List
get_file_list(Application_Links *app, char *dir, i32 len){
    File_List list = {};
    get_file_list(app, SCu8(dir, len), &list);
    return(list);
}

static b32
file_exists(Application_Links *app, char *file_name, i32 len){
    File_Attributes attributes = get_file_attributes(app, SCu8(file_name, len));
    return(attributes.last_write_time > 0);
}

static b32
directory_cd(Application_Links *app, char *dir, i32 *len, i32 capacity, char *rel_path, i32 rel_len){
    String_Const_u8 directory = SCu8(dir, *len);
    String_Const_u8 relative_path = SCu8(rel_path, rel_len);
    
    Scratch_Block scratch(app);
    String_Const_u8 new_directory = {};
    b32 result = false;
    if (relative_path.size > 0){
        if (string_match(relative_path, string_u8_litexpr("."))){
            new_directory = directory;
            result = true;
        }
        else if (string_match(relative_path, string_u8_litexpr(".."))){
            directory = string_remove_last_folder(directory);
            if (file_exists(app, (char*)directory.str, (i32)directory.size)){
                new_directory = directory;
                result = true;
            }
        }
        else{
            new_directory = push_u8_stringf(scratch, "%.*s/%.*s",
                                            string_expand(directory),
                                            string_expand(relative_path));
            if (file_exists(app, (char*)new_directory.str, (i32)new_directory.size)){
                result = true;
            }
        }
    }
    
    if (result){
        i32 new_len = clamp_top((i32)new_directory.size, capacity);
        block_copy(dir, new_directory.str, new_len);
        *len = new_len;
    }
    
    return(result);
}

static i32
get_4ed_path(Application_Links *app, char *out, i32 capacity){
    Scratch_Block scratch(app);
    String_Const_u8 string = push_4ed_path(app, scratch);
    block_copy(out, string.str, clamp_top((i32)string.size, capacity));
    return((i32)string.size);
}

static void
set_window_title(Application_Links *app, char *title){
    String_Const_u8 title_string = SCu8(title);
    set_window_title(app, title_string);
}

static Process_State
get_process_state(Application_Links *app, Buffer_ID buffer){
    Process_State state = {};
    Child_Process_ID child_process = buffer_get_attached_child_process(app, buffer);
    if (child_process != 0){
        state = child_process_get_state(app, child_process);
    }
    return(state);
}

#endif

// BOTTOM

