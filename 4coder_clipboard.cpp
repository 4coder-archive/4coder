/*
4coder_clipboard.cpp - Copy paste commands and clipboard related setup.
*/

// TOP

static b32
post_buffer_range_to_clipboard(Application_Links *app, i32 clipboard_index, Buffer_ID buffer, i32 first, i32 one_past_last){
    b32 success = false;
    i32 buffer_size = 0;
    buffer_get_size(app, buffer, &buffer_size);
    if (buffer != 0 && 0 <= first && first < one_past_last && one_past_last <= buffer_size){
        Scratch_Block scratch(app);
        Range range = make_range(first, one_past_last);
        String_Const_u8 string = scratch_read(app, scratch, buffer, range);
        if (string.size > 0){
            clipboard_post(app, clipboard_index, string);
            success = true;
        }
    }
    return(success);
}

CUSTOM_COMMAND_SIG(copy)
CUSTOM_DOC("Copy the text in the range from the cursor to the mark onto the clipboard.")
{
    View_ID view = 0;
    get_active_view(app, AccessProtected, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessProtected, &buffer);
    Range range = get_view_range(app, view);
    post_buffer_range_to_clipboard(app, 0, buffer, range.min, range.max);
}

CUSTOM_COMMAND_SIG(cut)
CUSTOM_DOC("Cut the text in the range from the cursor to the mark onto the clipboard.")
{
    View_ID view = 0;
    get_active_view(app, AccessOpen, &view);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view, AccessOpen, &buffer);
    Range range = get_view_range(app, view);
    if (post_buffer_range_to_clipboard(app, 0, buffer, range.min, range.max)){
        buffer_replace_range(app, buffer, range, string_u8_litexpr(""));
    }
}

CUSTOM_COMMAND_SIG(paste)
CUSTOM_DOC("At the cursor, insert the text at the top of the clipboard.")
{
    i32 count = 0;
    clipboard_count(app, 0, &count);
    if (count > 0){
        View_ID view = 0;
        get_active_view(app, AccessOpen, &view);
        if_view_has_highlighted_range_delete_range(app, view);
        
        Managed_Scope scope = 0;
        view_get_managed_scope(app, view, &scope);
        managed_variable_set(app, scope, view_next_rewrite_loc, RewritePaste);
        i32 paste_index = 0;
        managed_variable_set(app, scope, view_paste_index_loc, paste_index);
        
        Scratch_Block scratch(app);
        
        String_Const_u8 string = {};
        clipboard_index(app, 0, paste_index, scratch, &string);
        if (string.size > 0){
            Buffer_ID buffer = 0;
            view_get_buffer(app, view, AccessOpen, &buffer);
            
            i32 pos = 0;
            view_get_cursor_pos(app, view, &pos);
            buffer_replace_range(app, buffer, make_range(pos), string);
            view_set_mark(app, view, seek_pos(pos));
            view_set_cursor(app, view, seek_pos(pos + (i32)string.size), true);
            
            // TODO(allen): Send this to all views.
            Theme_Color paste = {};
            paste.tag = Stag_Paste;
            get_theme_colors(app, &paste, 1);
            view_post_fade(app, view, 0.667f, pos, pos + (i32)string.size, paste.color);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_next)
CUSTOM_DOC("If the previous command was paste or paste_next, replaces the paste range with the next text down on the clipboard, otherwise operates as the paste command.")
{
    Scratch_Block scratch(app);
    
    i32 count = 0;
    clipboard_count(app, 0, &count);
    if (count > 0){
        View_ID view = 0;
        get_active_view(app, AccessOpen, &view);
        Managed_Scope scope = 0;
        view_get_managed_scope(app, view, &scope);
        no_mark_snap_to_cursor(app, scope);
        
        u64 rewrite = 0;
        managed_variable_get(app, scope, view_rewrite_loc, &rewrite);
        if (rewrite == RewritePaste){
            managed_variable_set(app, scope, view_next_rewrite_loc, RewritePaste);
            u64 prev_paste_index = 0;
            managed_variable_get(app, scope, view_paste_index_loc, &prev_paste_index);
            i32 paste_index = (i32)prev_paste_index + 1;
            managed_variable_set(app, scope, view_paste_index_loc, paste_index);
            
            String_Const_u8 string = {};
            clipboard_index(app, 0, paste_index, scratch, &string);
            
            Buffer_ID buffer = 0;
            view_get_buffer(app, view, AccessOpen, &buffer);
            
            Range range = get_view_range(app, view);
            i32 pos = range.min;
            
            buffer_replace_range(app, buffer, range, string);
            view_set_cursor(app, view, seek_pos(pos + (i32)string.size), true);
            
            // TODO(allen): Send this to all views.
            Theme_Color paste = {};
            paste.tag = Stag_Paste;
            get_theme_colors(app, &paste, 1);
            view_post_fade(app, view, 0.667f, pos, pos + (i32)string.size, paste.color);
            
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

