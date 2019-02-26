/*
4coder_clipboard.cpp - Copy paste commands and clipboard related setup.
*/

// TOP

static b32
post_buffer_range_to_clipboard(Application_Links *app, Partition *scratch, i32 clipboard_index,
                               Buffer_Summary *buffer, i32 first, i32 one_past_last){
    b32 success = false;
    if (buffer->exists &&
        0 <= first && first < one_past_last && one_past_last <= buffer->size){
        Temp_Memory temp = begin_temp_memory(scratch);
        i32 size = one_past_last - first;
        char *str = push_array(scratch, char, size);
        if (str != 0){
            buffer_read_range(app, buffer, first, one_past_last, str);
            clipboard_post(app, clipboard_index, str, size);
            success = true;
        }
        end_temp_memory(temp);
    }
    return(success);
}

CUSTOM_COMMAND_SIG(copy)
CUSTOM_DOC("Copy the text in the range from the cursor to the mark onto the clipboard.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    Range range = get_view_range(&view);
    post_buffer_range_to_clipboard(app, &global_part, 0, &buffer, range.min, range.max);
}

CUSTOM_COMMAND_SIG(cut)
CUSTOM_DOC("Cut the text in the range from the cursor to the mark onto the clipboard.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    Range range = get_view_range(&view);
    if (post_buffer_range_to_clipboard(app, &global_part, 0, &buffer, range.min, range.max)){
        buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(paste)
CUSTOM_DOC("At the cursor, insert the text at the top of the clipboard.")
{
    u32 access = AccessOpen;
    i32 count = clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = get_active_view(app, access);
        if_view_has_highlighted_range_delete_range(app, view.view_id);
        view = get_view(app, view.view_id, access);
        
        Managed_Scope scope = view_get_managed_scope(app, view.view_id);
        managed_variable_set(app, scope, view_next_rewrite_loc, RewritePaste);
        i32 paste_index = 0;
        managed_variable_set(app, scope, view_paste_index_loc, paste_index);
        
        i32 len = clipboard_index(app, 0, paste_index, 0, 0);
        char *str = 0;
        if (len <= app->memory_size){
            str = (char*)app->memory;
        }
        
        if (str != 0){
            clipboard_index(app, 0, paste_index, str, len);
            
            Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
            i32 pos = view.cursor.pos;
            buffer_replace_range(app, &buffer, pos, pos, str, len);
            view_set_mark(app, &view, seek_pos(pos));
            view_set_cursor(app, &view, seek_pos(pos + len), true);
            
            // TODO(allen): Send this to all views.
            Theme_Color paste = {};
            paste.tag = Stag_Paste;
            get_theme_colors(app, &paste, 1);
            view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_next)
CUSTOM_DOC("If the previous command was paste or paste_next, replaces the paste range with the next text down on the clipboard, otherwise operates as the paste command.")
{
    u32 access = AccessOpen;
    i32 count = clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = get_active_view(app, access);
        Managed_Scope scope = view_get_managed_scope(app, view.view_id);
        no_mark_snap_to_cursor(app, scope);
        
        uint64_t rewrite = 0;
        managed_variable_get(app, scope, view_rewrite_loc, &rewrite);
        if (rewrite == RewritePaste){
            managed_variable_set(app, scope, view_next_rewrite_loc, RewritePaste);
            uint64_t prev_paste_index = 0;
            managed_variable_get(app, scope, view_paste_index_loc, &prev_paste_index);
            i32 paste_index = (i32)prev_paste_index + 1;
            managed_variable_set(app, scope, view_paste_index_loc, paste_index);
            
            i32 len = clipboard_index(app, 0, paste_index, 0, 0);
            char *str = 0;
            
            if (len <= app->memory_size){
                str = (char*)app->memory;
            }
            
            if (str != 0){
                clipboard_index(app, 0, paste_index, str, len);
                
                Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
                Range range = get_view_range(&view);
                i32 pos = range.min;
                
                buffer_replace_range(app, &buffer, range.min, range.max, str, len);
                view_set_cursor(app, &view, seek_pos(pos + len), true);
                
                // TODO(allen): Send this to all views.
                Theme_Color paste = {};
                paste.tag = Stag_Paste;
                get_theme_colors(app, &paste, 1);
                view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
            }
        }
        else{
            exec_command(app, paste);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_and_indent)
CUSTOM_DOC("Paste from the top of clipboard and run auto-indent on the newly pasted text.")
{
    paste(app);
    auto_tab_range(app);
}

CUSTOM_COMMAND_SIG(paste_next_and_indent)
CUSTOM_DOC("Paste the next item on the clipboard and run auto-indent on the newly pasted text.")
{
    paste_next(app);
    auto_tab_range(app);
}

// BOTTOM

