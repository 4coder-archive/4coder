/*
4coder_clipboard.cpp - Copy paste commands and clipboard related setup.
*/

// TOP

global Clipboard clipboard0 = {};

function void
clipboard_init_empty(Clipboard *clipboard, u32 history_depth){
    history_depth = clamp_bot(1, history_depth);
    heap_init(&clipboard->heap, &clipboard->arena);
    clipboard->clip_index = 0;
    clipboard->clip_capacity = history_depth;
    clipboard->clips = push_array_zero(&clipboard->arena, String_Const_u8, history_depth);
}

function void
clipboard_init(Base_Allocator *allocator, u32 history_depth, Clipboard *clipboard_out){
    u64 memsize = sizeof(String_Const_u8)*history_depth;
    memsize = round_up_u64(memsize, KB(4));
    clipboard_out->arena = make_arena(allocator, memsize, 8);
    clipboard_init_empty(clipboard_out, history_depth);
}

function void
clipboard_clear(Clipboard *clipboard){
    linalloc_clear(&clipboard->arena);
    clipboard_init_empty(clipboard, clipboard->clip_capacity);
}

function String_Const_u8
clipboard_post_internal_only(Clipboard *clipboard, String_Const_u8 string){
    u32 rolled_index = clipboard->clip_index%clipboard->clip_capacity;
    clipboard->clip_index += 1;
    String_Const_u8 *slot = &clipboard->clips[rolled_index];
    if (slot->str != 0){
        if (slot->size < string.size ||
            (slot->size - string.size) > KB(1)){
            heap_free(&clipboard->heap, slot->str);
            goto alloc_new;
        }
    }
    else{
        alloc_new:;
        u8 *new_buf = (u8*)heap_allocate(&clipboard->heap, string.size);
        slot->str = new_buf;
    }
    block_copy(slot->str, string.str, string.size);
    slot->size = string.size;
    return(*slot);
}

function u32
clipboard_count(Clipboard *clipboard){
    u32 result = clipboard->clip_index;
    result = clamp_top(result, clipboard->clip_capacity);
    return(result);
}

function String_Const_u8
get_clipboard_index(Clipboard *clipboard, u32 item_index){
    String_Const_u8 result = {};
    u32 top = Min(clipboard->clip_index, clipboard->clip_capacity);
    if (top > 0){
        item_index = item_index%top;
        i32 array_index = ((clipboard->clip_index - 1) - item_index)%top;
        result = clipboard->clips[array_index];
    }
    return(result);
}

function String_Const_u8
push_clipboard_index(Arena *arena, Clipboard *clipboard, i32 item_index){
    String_Const_u8 result = get_clipboard_index(clipboard, item_index);
    result = push_string_copy(arena, result);
    return(result);
}

////////////////////////////////

function void
clipboard_clear(i32 clipboard_id){
    clipboard_clear(&clipboard0);
}

function String_Const_u8
clipboard_post_internal_only(i32 clipboard_id, String_Const_u8 string){
    return(clipboard_post_internal_only(&clipboard0, string));
}

function b32
clipboard_post(i32 clipboard_id, String_Const_u8 string){
    clipboard_post_internal_only(clipboard_id, string);
    system_post_clipboard(string, clipboard_id);
    return(true);
}

function i32
clipboard_count(i32 clipboard_id){
    return(clipboard_count(&clipboard0));
}

function String_Const_u8
push_clipboard_index(Arena *arena, i32 clipboard_id, i32 item_index){
    return(push_clipboard_index(arena, &clipboard0, item_index));
}

////////////////////////////////

CUSTOM_COMMAND_SIG(clipboard_record_clip)
CUSTOM_DOC("In response to a new clipboard contents events, saves the new clip onto the clipboard history")
{
    User_Input in = get_current_input(app);
    if (in.event.kind == InputEventKind_Core &&
        in.event.core.code == CoreCode_NewClipboardContents){
        clipboard_post_internal_only(0, in.event.core.string);
    }
}

////////////////////////////////

function b32
clipboard_post_buffer_range(Application_Links *app, i32 clipboard_index, Buffer_ID buffer, Range_i64 range){
    b32 success = false;
    Scratch_Block scratch(app);
    String_Const_u8 string = push_buffer_range(app, scratch, buffer, range);
    if (string.size > 0){
        clipboard_post(clipboard_index, string);
        success = true;
    }
    return(success);
}

function void
clipboard_update_history_from_system(Application_Links *app, i32 clipboard_id){
    Scratch_Block scratch(app);
    String_Const_u8 string = system_get_clipboard(scratch, clipboard_id);
    if (string.str != 0){
        clipboard_post_internal_only(clipboard_id, string);
    }
}

global List_String_Const_u8 clipboard_collection_list = {};

function void
clipboard_collection_render(Application_Links *app, Frame_Info frame_info, View_ID view){
    Scratch_Block scratch(app);
    Rect_f32 region = draw_background_and_margin(app, view);
    Vec2_f32 mid_p = (region.p1 + region.p0)*0.5f;
    
    Fancy_Block message = {};
    Fancy_Line *line = push_fancy_line(scratch, &message);
    push_fancy_string(scratch, line, fcolor_id(defcolor_pop2),
                      string_u8_litexpr("Collecting all clipboard events "));
    push_fancy_string(scratch, line, fcolor_id(defcolor_pop1),
                      string_u8_litexpr("press [escape] to stop"));
    
    for (Node_String_Const_u8 *node = clipboard_collection_list.first;
         node != 0;
         node = node->next){
        line = push_fancy_line(scratch, &message);
        push_fancy_string(scratch, line, fcolor_id(defcolor_text_default), node->string);
    }
    
    Face_ID face_id = get_face_id(app, 0);
    Vec2_f32 dim = get_fancy_block_dim(app, face_id, &message);
    Vec2_f32 half_dim = dim*0.5f;
    draw_fancy_block(app, face_id, fcolor_zero(), &message, mid_p - half_dim);
}

CUSTOM_UI_COMMAND_SIG(begin_clipboard_collection_mode)
CUSTOM_DOC("Allows the user to copy multiple strings from other applications before switching to 4coder and pasting them all.")
{
    local_persist b32 in_clipboard_collection_mode = false;
    if (!in_clipboard_collection_mode){
        in_clipboard_collection_mode = true;
        system_set_clipboard_catch_all(true);
        
        Scratch_Block scratch(app);
        block_zero_struct(&clipboard_collection_list);
        
        View_ID view = get_this_ctx_view(app, Access_Always);
        View_Context ctx = view_current_context(app, view);
        ctx.render_caller = clipboard_collection_render;
        ctx.hides_buffer = true;
        View_Context_Block ctx_block(app, view, &ctx);
        
        for (;;){
            User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
            if (in.abort){
                break;
            }
            if (in.event.kind == InputEventKind_KeyStroke && in.event.key.code == KeyCode_Escape){
                break;
            }
            if (in.event.kind == InputEventKind_Core &&
                in.event.core.code == CoreCode_NewClipboardContents){
                String_Const_u8 stable_clip = clipboard_post_internal_only(0, in.event.core.string);
                string_list_push(scratch, &clipboard_collection_list, stable_clip);
            }
        }
        
        block_zero_struct(&clipboard_collection_list);
        
        system_set_clipboard_catch_all(false);
        in_clipboard_collection_mode = false;
    }
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
    clipboard_update_history_from_system(app, 0);
    i32 count = clipboard_count(0);
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
            
            String_Const_u8 string = push_clipboard_index(scratch, 0, *paste_index);
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
    
    i32 count = clipboard_count(0);
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
                
                String_Const_u8 string = push_clipboard_index(scratch, 0, paste_index);
                
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

CUSTOM_COMMAND_SIG(clear_clipboard)
CUSTOM_DOC("Clears the history of the clipboard")
{
    clipboard_clear(0);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(multi_paste){
    Scratch_Block scratch(app);
    
    i32 count = clipboard_count(0);
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
                
                String_Const_u8 string = push_clipboard_index(scratch, 0, paste_index);
                
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
                String_Const_u8 string = push_clipboard_index(scratch, 0, paste_index);
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
                String_Const_u8 string = push_clipboard_index(scratch, 0, paste_index);
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

CUSTOM_COMMAND_SIG(multi_paste_interactive)
CUSTOM_DOC("Paste multiple lines from the clipboard history, controlled with arrow keys")
{
    i32 clip_count = clipboard_count(0);
    if (clip_count > 0){
        multi_paste_interactive_up_down(app, 1, clip_count);
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive_quick)
CUSTOM_DOC("Paste multiple lines from the clipboard history, controlled by inputing the number of lines to paste")
{
    i32 clip_count = clipboard_count(0);
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

////////////////////////////////

#if FCODER_TRANSITION_TO < 4001004
function void
clipboard_clear(Application_Links *app, i32 clipboard_id){
    clipboard_clear(clipboard_id);
}
function b32
clipboard_post(Application_Links *app, i32 clipboard_id, String_Const_u8 string){
    return(clipboard_post(clipboard_id, string));
}
function i32
clipboard_count(Application_Links *app, i32 clipboard_id){
    return(clipboard_count(clipboard_id));
}
function String_Const_u8
push_clipboard_index(Application_Links *app, Arena *arena, i32 clipboard_id, i32 item_index){
    return(push_clipboard_index(arena, clipboard_id, item_index));
}
#endif

// BOTTOM

