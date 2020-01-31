/*
4coder_clipboard.cpp - Copy paste commands and clipboard related setup.
*/

// TOP

function b32
clipboard_post_buffer_range(Application_Links *app, i32 clipboard_index, Buffer_ID buffer, Range_i64 range){
    b32 success = false;
    Scratch_Block scratch(app);
    String_Const_u8 string = push_buffer_range(app, scratch, buffer, range);
    if (string.size > 0){
        clipboard_post(app, clipboard_index, string);
        success = true;
    }
    return(success);
}

CUSTOM_COMMAND_SIG(copy)
CUSTOM_DOC("Copy the text in the range from the cursor to the mark onto the clipboard.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    Range_i64 range = get_view_range(app, view);
    clipboard_post_buffer_range(app, 0, buffer, range);
}

CUSTOM_COMMAND_SIG(cut)
CUSTOM_DOC("Cut the text in the range from the cursor to the mark onto the clipboard.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Range_i64 range = get_view_range(app, view);
    if (clipboard_post_buffer_range(app, 0, buffer, range)){
        buffer_replace_range(app, buffer, range, string_u8_empty);
    }
}

CUSTOM_COMMAND_SIG(paste)
CUSTOM_DOC("At the cursor, insert the text at the top of the clipboard.")
{
    i32 count = clipboard_count(app, 0);
    if (count > 0){
        View_ID view = get_active_view(app, Access_ReadWriteVisible);
        if_view_has_highlighted_range_delete_range(app, view);
        
        Managed_Scope scope = view_get_managed_scope(app, view);
        Rewrite_Type *next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
        if (next_rewrite != 0){
            *next_rewrite = Rewrite_Paste;
            i32 *paste_index = scope_attachment(app, scope, view_paste_index_loc, i32);
            *paste_index = 0;
            
            Scratch_Block scratch(app);
            
            String_Const_u8 string = push_clipboard_index(app, scratch, 0, *paste_index);
            if (string.size > 0){
                Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
                
                i64 pos = view_get_cursor_pos(app, view);
                buffer_replace_range(app, buffer, Ii64(pos), string);
                view_set_mark(app, view, seek_pos(pos));
                view_set_cursor_and_preferred_x(app, view, seek_pos(pos + (i32)string.size));
                
                ARGB_Color argb = fcolor_resolve(fcolor_id(defcolor_paste));
                buffer_post_fade(app, buffer, 0.667f, Ii64_size(pos, string.size), argb);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(paste_next)
CUSTOM_DOC("If the previous command was paste or paste_next, replaces the paste range with the next text down on the clipboard, otherwise operates as the paste command.")
{
    Scratch_Block scratch(app);
    
    i32 count = clipboard_count(app, 0);
    if (count > 0){
        View_ID view = get_active_view(app, Access_ReadWriteVisible);
        Managed_Scope scope = view_get_managed_scope(app, view);
        no_mark_snap_to_cursor(app, scope);
        
        Rewrite_Type *rewrite = scope_attachment(app, scope, view_rewrite_loc, Rewrite_Type);
        if (rewrite != 0){
            if (*rewrite == Rewrite_Paste){
                Rewrite_Type *next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
                *next_rewrite = Rewrite_Paste;
                
                i32 *paste_index_ptr = scope_attachment(app, scope, view_paste_index_loc, i32);
                i32 paste_index = (*paste_index_ptr) + 1;
                *paste_index_ptr = paste_index;
                
                String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
                
                Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
                
                Range_i64 range = get_view_range(app, view);
                i64 pos = range.min;
                
                buffer_replace_range(app, buffer, range, string);
                view_set_cursor_and_preferred_x(app, view, seek_pos(pos + string.size));
                
                ARGB_Color argb = fcolor_resolve(fcolor_id(defcolor_paste));
                buffer_post_fade(app, buffer, 0.667f, Ii64_size(pos, string.size), argb);
            }
            else{
                paste(app);
            }
        }
    }
}

CUSTOM_COMMAND_SIG(paste_and_indent)
CUSTOM_DOC("Paste from the top of clipboard and run auto-indent on the newly pasted text.")
{
    paste(app);
    auto_indent_range(app);
}

CUSTOM_COMMAND_SIG(paste_next_and_indent)
CUSTOM_DOC("Paste the next item on the clipboard and run auto-indent on the newly pasted text.")
{
    paste_next(app);
    auto_indent_range(app);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(multi_paste){
    Scratch_Block scratch(app);
    
    i32 count = clipboard_count(app, 0);
    if (count > 0){
        View_ID view = get_active_view(app, Access_ReadWriteVisible);
        Managed_Scope scope = view_get_managed_scope(app, view);
        
        Rewrite_Type *rewrite = scope_attachment(app, scope, view_rewrite_loc, Rewrite_Type);
        if (rewrite != 0){
            if (*rewrite == Rewrite_Paste){
                Rewrite_Type *next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
                *next_rewrite = Rewrite_Paste;
                i32 *paste_index_ptr = scope_attachment(app, scope, view_paste_index_loc, i32);
                i32 paste_index = (*paste_index_ptr) + 1;
                *paste_index_ptr = paste_index;
                
                String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
                
                String_Const_u8 insert_string = push_u8_stringf(scratch, "\n%.*s", string_expand(string));
                
                Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
                Range_i64 range = get_view_range(app, view);
                buffer_replace_range(app, buffer, Ii64(range.max), insert_string);
                view_set_mark(app, view, seek_pos(range.max + 1));
                view_set_cursor_and_preferred_x(app, view, seek_pos(range.max + insert_string.size));
                
                ARGB_Color argb = fcolor_resolve(fcolor_id(defcolor_paste));
                view_post_fade(app, buffer, 0.667f, Ii64(range.max + 1, range.max + insert_string.size), argb);
            }
            else{
                paste(app);
            }
        }
    }
}

function Range_i64
multi_paste_range(Application_Links *app, View_ID view, Range_i64 range, i32 paste_count, b32 old_to_new){
    Scratch_Block scratch(app);
    
    Range_i64 finish_range = range;
    if (paste_count >= 1){
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
        if (buffer != 0){
            i64 total_size = 0;
            for (i32 paste_index = 0; paste_index < paste_count; ++paste_index){
                Temp_Memory temp = begin_temp(scratch);
                String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
                total_size += string.size + 1;
                end_temp(temp);
            }
            total_size -= 1;
            
            i32 first = paste_count - 1;
            i32 one_past_last = -1;
            i32 step = -1;
            if (!old_to_new){
                first = 0;
                one_past_last = paste_count;
                step = 1;
            }
            
            List_String_Const_u8 list = {};
            
            for (i32 paste_index = first; paste_index != one_past_last; paste_index += step){
                if (paste_index != first){
                    string_list_push(scratch, &list, SCu8("\n", 1));
                }
                String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
                if (string.size > 0){
                    string_list_push(scratch, &list, string);
                }
            }
            
            String_Const_u8 flattened = string_list_flatten(scratch, list);
            
            buffer_replace_range(app, buffer, range, flattened);
            i64 pos = range.min;
            finish_range.min = pos;
            finish_range.max = pos + total_size;
            view_set_mark(app, view, seek_pos(finish_range.min));
            view_set_cursor_and_preferred_x(app, view, seek_pos(finish_range.max));
            
            ARGB_Color argb = fcolor_resolve(fcolor_id(defcolor_paste));
            buffer_post_fade(app, buffer, 0.667f, finish_range, argb);
        }
    }
    return(finish_range);
}

function void
multi_paste_interactive_up_down(Application_Links *app, i32 paste_count, i32 clip_count){
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    b32 old_to_new = true;
    Range_i64 range = multi_paste_range(app, view, Ii64(pos), paste_count, old_to_new);
    
    Query_Bar_Group group(app);
    Query_Bar bar = {};
    bar.prompt = string_u8_litexpr("Up and Down to condense and expand paste stages; R to reverse order; Return to finish; Escape to abort.");
    if (start_query_bar(app, &bar, 0) == 0) return;
    
    User_Input in = {};
    for (;;){
        in = get_next_input(app, EventProperty_AnyKey, EventProperty_Escape);
        if (in.abort) break;
        
        b32 did_modify = false;
        if (match_key_code(&in, KeyCode_Up)){
            if (paste_count > 1){
                --paste_count;
                did_modify = true;
            }
        }
        else if (match_key_code(&in, KeyCode_Down)){
            if (paste_count < clip_count){
                ++paste_count;
                did_modify = true;
            }
        }
        else if (match_key_code(&in, KeyCode_R)){
            old_to_new = !old_to_new;
            did_modify = true;
        }
        else if (match_key_code(&in, KeyCode_Return)){
            break;
        }
        
        if (did_modify){
            range = multi_paste_range(app, view, range, paste_count, old_to_new);
        }
    }
    
    if (in.abort){
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
        buffer_replace_range(app, buffer, range, SCu8(""));
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive){
    i32 clip_count = clipboard_count(app, 0);
    if (clip_count > 0){
        multi_paste_interactive_up_down(app, 1, clip_count);
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive_quick){
    i32 clip_count = clipboard_count(app, 0);
    if (clip_count > 0){
        u8 string_space[256];
        Query_Bar_Group group(app);
        Query_Bar bar = {};
        bar.prompt = string_u8_litexpr("How Many Slots To Paste: ");
        bar.string = SCu8(string_space, (u64)0);
        bar.string_capacity = sizeof(string_space);
        query_user_number(app, &bar);
        
        i32 initial_paste_count = (i32)string_to_integer(bar.string, 10);
        initial_paste_count = clamp(1, initial_paste_count, clip_count);
        end_query_bar(app, &bar, 0);
        
        multi_paste_interactive_up_down(app, initial_paste_count, clip_count);
    }
}

// BOTTOM

