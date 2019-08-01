/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.????
 *
 * Implementation of the API functions.
 *
 */

// TOP

internal b32
access_test(u32 lock_flags, u32 access_flags){
    return((lock_flags & ~access_flags) == 0);
}

internal b32
api_check_panel(Panel *panel){
    b32 result = false;
    if (panel != 0 && panel->kind != PanelKind_Unused){
        result = true;
    }
    return(result);
}

internal b32
api_check_buffer(Editing_File *file){
    return(file != 0 && !file->is_dummy);
}

internal b32
api_check_buffer_and_tokens(Editing_File *file){
    return(api_check_buffer(file) && file->state.token_array.tokens != 0 && file->state.tokens_complete);
}

internal b32
api_check_buffer(Editing_File *file, Access_Flag access){
    return(api_check_buffer(file) && access_test(file_get_access_flags(file), access));
}

internal b32
api_check_view(View *view){
    return(view != 0 && view->in_use);
}

internal b32
api_check_view(View *view, Access_Flag access){
    return(api_check_view(view) && access_test(view_get_access_flags(view), access));
}

internal b32
is_running_coroutine(Application_Links *app){
    return(app->current_coroutine != 0);
}

API_EXPORT b32
Global_Set_Setting(Application_Links *app, Global_Setting_ID setting, i32 value)
/*
DOC_PARAM(setting, Which setting to change.)
DOC_PARAM(value, The new value to set ont he specified setting.)
DOC_SEE(Global_Setting_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    b32 result = true;
    switch (setting){
        case GlobalSetting_LAltLCtrlIsAltGr:
        {
            models->settings.lctrl_lalt_is_altgr = value;
        }break;
        default:
        {
            result = false;
        }break;
    }
    return(result);
}

API_EXPORT b32
Global_Set_Mapping(Application_Links *app, void *data, i32 size)
/*
DOC_PARAM(data, The beginning of a binding buffer.  Bind_Helper is designed to make it easy to produce such a buffer.)
DOC_PARAM(size, The size of the binding buffer in bytes.)
DOC_RETURN(Returns non-zero if no errors occurred while interpretting the binding buffer.  A return value of zero does not indicate that the old mappings are still in place.)
DOC(Dumps away the previous mappings and instantiates the mappings described in the binding buffer.  If any of the open buffers were bound to a command map that used to exist, but no command map with the same id exist after the new mappings are instantiated, the buffer's command map will be set to mapid_file and a warning will be posted to *messages*.)
*/{
    Models *models = (Models*)app->cmd_context;
    b32 result = interpret_binding_buffer(models, data, size);
    return(result);
}

API_EXPORT Rect_f32
Global_Get_Screen_Rectangle(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(Rf32(V2(0, 0), V2(layout_get_root_size(&models->layout))));
}

API_EXPORT Arena*
Context_Get_Arena(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(&models->custom_layer_arena);
}

API_EXPORT Base_Allocator*
Context_Get_Base_Allocator(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(models->base_allocator);
}

API_EXPORT b32
Create_Child_Process(Application_Links *app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *child_process_id_out){
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(child_process_call(models, system, path, command, child_process_id_out));
}

API_EXPORT b32
Child_Process_Set_Target_Buffer(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags){
    Models *models = (Models*)app->cmd_context;
    Child_Process *child_process = child_process_from_id(&models->child_processes, child_process_id);
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file) && child_process != 0){
        result = child_process_set_target_buffer(models, child_process, file, flags);
    }
    return(result);
}

API_EXPORT Child_Process_ID
Buffer_Get_Attached_Child_Process(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Child_Process_ID result = 0;
    if (api_check_buffer(file)){
        result = file->state.attached_child_process;
    }
    return(result);
}

API_EXPORT Buffer_ID
Child_Process_Get_Attached_Buffer(Application_Links *app, Child_Process_ID child_process_id){
    Models *models = (Models*)app->cmd_context;
    Child_Process *child_process = child_process_from_id(&models->child_processes, child_process_id);
    Buffer_ID result = 0;
    if (child_process != 0 && child_process->out_file != 0){
        result = child_process->out_file->id.id;
    }
    return(result);
}

API_EXPORT Process_State
Child_Process_Get_State(Application_Links *app, Child_Process_ID child_process_id){
    Models *models = (Models*)app->cmd_context;
    return(child_process_get_state(&models->child_processes, child_process_id));
}

// TODO(allen): redocument
API_EXPORT b32
Clipboard_Post(Application_Links *app, i32 clipboard_id, String_Const_u8 string)
/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC_PARAM(str, The str parameter specifies the string to be posted to the clipboard, it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the str string.)
DOC(Stores the string str in the clipboard initially with index 0. Also reports the copy to the operating system, so that it may be pasted into other applications.)
DOC_SEE(The_4coder_Clipboard)
*/{
    Models *models = (Models*)app->cmd_context;
    String_Const_u8 *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, (i32)string.size);
    block_copy(dest->str, string.str, string.size);
    models->system->post_clipboard(*dest);
    return(true);
}

// TODO(casey): Allen, why does this return a bool?  Like, shouldn't it just return the count?  If there's nothing in that clipboard, it'd just be 0?
API_EXPORT i32
Clipboard_Count(Application_Links *app, i32 clipboard_id)
/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC(This call returns the number of items in the clipboard.)
DOC_SEE(The_4coder_Clipboard)
*/{
    Models *models = (Models*)app->cmd_context;
    return(models->working_set.clipboard_size);
}

// TODO(allen): redocument
API_EXPORT String_Const_u8
Push_Clipboard_Index(Application_Links *app, Arena *arena, i32 clipboard_id, i32 item_index)
/*
DOC_PARAM(clipboard_id, This parameter is set up to prepare for future features, it should always be 0 for now.)
DOC_PARAM(item_index, This parameter specifies which item to read, 0 is the most recent copy, 1 is the second most recent copy, etc.)
DOC_PARAM(out, This parameter provides a buffer where the clipboard contents are written.  This parameter may be NULL.)
DOC_PARAM(len, This parameter specifies the length of the out buffer.)
DOC_RETURN(This call returns the size of the item associated with item_index.)

DOC(This function always returns the size of the item even if the output buffer is NULL. If the output buffer is too small to contain the whole string,
it is filled with the first len character of the clipboard contents.  The output string is not null terminated.)

DOC_SEE(The_4coder_Clipboard)
*/{
    Models *models = (Models*)app->cmd_context;
    String_Const_u8 *str = working_set_clipboard_index(&models->working_set, item_index);
    String_Const_u8 result = {};
    if (str != 0){
        result = push_string_copy(arena, *str);
    }
    return(result);
}

API_EXPORT Parse_Context_ID
Create_Parse_Context(Application_Links *app, Parser_String_And_Type *kw, u32 kw_count, Parser_String_And_Type *pp, u32 pp_count)
/*
DOC_PARAM(kw, The list of keywords and type of each.)
DOC_PARAM(kw_count, The number of keywords in the list.)
DOC_PARAM(pp, The list of preprocessor directives and the type of each.)
DOC_PARAM(pp_count, The number of preprocessor directives in the list.)
DOC_RETURN(On success returns an id for the new parse context.  If id == 0, then the maximum number of parse contexts has been reached.)
*/{
    Models *models = (Models*)app->cmd_context;
    Parse_Context_ID id = parse_context_add(&models->parse_context_memory, &models->mem.heap, kw, kw_count, pp, pp_count);
    return(id);
}

API_EXPORT i32
Get_Buffer_Count(Application_Links *app)
/*
DOC(Gives the total number of buffers in the application.)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    i32 result = working_set->file_count;
    return(result);
}

// TODO(allen): redocument
API_EXPORT Buffer_ID
Get_Buffer_Next(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
/*
DOC_PARAM(buffer, The  pointed to by buffer is iterated to the next buffer or to a null summary if this is the last buffer.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access. The buffer outputted will be the next buffer that is accessible.)
DOC(
This call steps a  to the next buffer in the global buffer order.
The global buffer order is kept roughly in the order of most recently used to least recently used.

If the buffer outputted does not exist, the loop is finished.
Buffers should not be killed or created durring a buffer loop.
)
DOC_SEE(Access_Flag)
DOC_SEE(get_buffer_first)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = working_set_get_active_file(working_set, buffer_id);
    file = file_get_next(working_set, file);
    for (;file != 0 && !access_test(file_get_access_flags(file), access);){
        file = file_get_next(working_set, file);
    }
    Buffer_ID result = 0;
    if (file != 0){
        result = file->id.id;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Buffer_ID
Get_Buffer_By_Name(Application_Links *app, String_Const_u8 name, Access_Flag access)
/*
DOC_PARAM(name, The name parameter specifies the buffer name to try to get. The string need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)

DOC(This call searches the buffers by their buffer name.  The buffer name is the short name in the file bar.  The name must match exactly including any alterations put on the buffer name to avoid duplicates.)

DOC_SEE(get_buffer_by_file_name)
DOC_SEE(Access_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = working_set_contains_name(working_set, name);
    Buffer_ID result = 0;
    if (api_check_buffer(file, access)){
        result = file->id.id;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Buffer_ID
Get_Buffer_By_File_Name(Application_Links *app, String_Const_u8 file_name, Access_Flag access)
/*
DOC_PARAM(name, The name parameter specifies the buffer name to try to get. The string need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the name string.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the indicated buffer if it exists and is accessible.)

DOC(This call searches the buffers by their canonicalized file names.  Not all buffers have file names, only buffers that are tied to files.  For instance *scratch* does not have a file name.  Every file has one canonicalized file name.  For instance on windows this involves converting w:/a/b into W:\a\b.  If the name passed is not canonicalized a canonicalized copy is made first.  This includes turning relative paths to files that exist into full paths.  So the passed in name can be relative to the working directory.)

DOC_SEE(get_buffer_by_name)
DOC_SEE(Access_Flag)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File_Name canon = {};
    Buffer_ID result = false;
    if (get_canon_name(system, file_name, &canon)){
        Working_Set *working_set = &models->working_set;
        Editing_File *file = working_set_contains_canon(working_set, string_from_file_name(&canon));
        if (api_check_buffer(file, access)){
            result = file->id.id;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Read_Range(Application_Links *app, Buffer_ID buffer_id, Range_i64 range, char *out)
/*
DOC_PARAM(buffer, This parameter specifies the buffer to read.)
DOC_PARAM(start, This parameter specifies absolute position of the first character in the read range.)
DOC_PARAM(one_past_last, This parameter specifies the absolute position of the the character one past the end of the read range.)
DOC_PARAM(out, This paramter provides the output character buffer to fill with the result of the read.)
DOC_RETURN(This call returns non-zero if the read succeeds.)
DOC(The output buffer must have a capacity of at least (one_past_last - start). The output is not null terminated.

This call fails if the buffer does not exist, or if the read range is not within the bounds of the buffer.)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        i64 size = buffer_size(&file->state.buffer);
        if (0 <= range.min && range.min <= range.max && range.max <= size){
            buffer_stringify(&file->state.buffer, (i32)range.min, (i32)range.max, out);
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Replace_Range(Application_Links *app, Buffer_ID buffer_id, Range_i64 range, String_Const_u8 string)
/*
DOC_PARAM(buffer, This parameter specifies the buffer to edit.)
DOC_PARAM(start, This parameter specifies absolute position of the first character in the replace range.)
DOC_PARAM(one_past_last, This parameter specifies the absolute position of the the character one past the end of the replace range.)
DOC_PARAM(str, This parameter specifies the the string to write into the range; it need not be null terminated.)
DOC_PARAM(len, This parameter specifies the length of the str string.)
DOC_RETURN(This call returns non-zero if the replacement succeeds.)
DOC(If this call succeeds it deletes the range from start to one_past_last and writes str in the same position.
If one_past_last == start then this call is equivalent to inserting the string at start.
If len == 0 this call is equivalent to deleteing the range from start to one_past_last.

This call fails if the buffer does not exist, or if the replace range is not within the bounds of the buffer.)
DOC_SEE(4coder_Buffer_Positioning_System)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        i64 size = buffer_size(&file->state.buffer);
        if (0 <= range.first && range.first <= range.one_past_last && range.one_past_last <= size){
            Edit_Behaviors behaviors = {};
            edit_single(models->system, models, file, Ii32((i32)range.min, (i32)range.max), string, behaviors);
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Batch_Edit(Application_Links *app, Buffer_ID buffer_id, char *str, Buffer_Edit *edits, i32 edit_count)
/*
DOC_PARAM(buffer, The buffer on which to apply the batch of edits.)
DOC_PARAM(str, This parameter provides all of the source string for the edits in the batch.)
DOC_PARAM(str_len, This parameter specifies the length of the str string.)
DOC_PARAM(edits, This parameter provides about the source string and destination range of each edit as an array.)
DOC_PARAM(edit_count, This parameter specifies the number of Buffer_Edit structs in edits.)
DOC_PARAM(type, This prameter specifies what type of batch edit to execute.)
DOC_RETURN(This call returns non-zero if the batch edit succeeds.  This call can fail if the provided buffer summary does not refer to an actual buffer in 4coder.)
DOC(Apply an array of edits all at once.  This combines all the edits into one undo operation.)
DOC_SEE(Buffer_Edit)
DOC_SEE(Buffer_Batch_Edit_Type)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        Edit_Behaviors behaviors = {};
        result = edit_batch(models->system, models, file, str, edits, edit_count, behaviors);
    }
    return(result);
}

API_EXPORT String_Match
Buffer_Seek_String(Application_Links *app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i64 start_pos){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match result = {};
    if (api_check_buffer(file)){
        if (needle.size == 0){
            result.flags = StringMatch_CaseSensitive;
            result.range = make_range_i64(start_pos);
        }
        else{
            Scratch_Block scratch(app);
            Gap_Buffer *gap_buffer = &file->state.buffer;
            i32 size = buffer_size(gap_buffer);
            String_Const_u8 space[3];
            Cursor cursor = make_cursor(space, sizeof(space));
            String_Const_u8_Array chunks = buffer_get_chunks(&cursor, gap_buffer);
            Range_i64 range = {};
            if (direction == Scan_Forward){
                i64 adjusted_pos = start_pos + 1;
                start_pos = clamp_top(adjusted_pos, size);
                range = make_range_i64(adjusted_pos, size);
            }
            else{
                i64 adjusted_pos = start_pos - 1;
                start_pos = clamp_bot(0, adjusted_pos);
                range = make_range_i64(0, adjusted_pos);
            }
            chunks = buffer_chunks_clamp(chunks, range);
            if (chunks.count > 0){
                u64_Array jump_table = string_compute_needle_jump_table(scratch, needle, direction);
                Character_Predicate dummy = {};
                String_Match_List list = find_all_matches(scratch, 1,
                                                          chunks, needle, jump_table, &dummy, direction,
                                                          range.min, buffer, 0);
                if (list.count == 1){
                    result = *list.first;
                }
            }
            else{
                result.range = Ii64(start_pos);
            }
        }
    }
    return(result);
}

API_EXPORT String_Match
Buffer_Seek_Character_Class(Application_Links *app, Buffer_ID buffer, Character_Predicate *predicate, Scan_Direction direction, i64 start_pos){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match result = {};
    if (api_check_buffer(file)){
        String_Const_u8 chunks_space[2];
        Cursor chunks_cursor = make_cursor(chunks_space, sizeof(chunks_space));
        Gap_Buffer *gap_buffer = &file->state.buffer;
        String_Const_u8_Array chunks = buffer_get_chunks(&chunks_cursor, gap_buffer, BufferGetChunk_Basic);
        
        if (chunks.count > 0){
            // TODO(allen): rewrite as loop-in-loop for better iteration performance
            i32 size = buffer_size(gap_buffer);
            start_pos = clamp(-1, start_pos, size);
            Buffer_Chunk_Position pos = buffer_get_chunk_position(chunks, size, (i32)start_pos);
            for (;;){
                i32 past_end = buffer_chunk_position_iterate(chunks, &pos, direction);
                if (past_end == -1){
                    break;
                }
                else if (past_end == 1){
                    result.range = make_range_i64(size);
                    break;
                }
                u8 v = chunks.vals[pos.chunk_index].str[pos.chunk_pos];
                if (character_predicate_check_character(*predicate, v)){
                    result.buffer = buffer;
                    result.range = make_range_i64(pos.real_pos, pos.real_pos + 1);
                    break;
                }
            }
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Partial_Cursor
Buffer_Compute_Cursor(Application_Links *app, Buffer_ID buffer_id, Buffer_Seek seek)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer on which to run the cursor computation.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(cursor_out, On success this struct is filled with the result of the seek.)
DOC_RETURN(This call returns non-zero on success.  This call can fail if the buffer summary provided
does not summarize an actual buffer in 4coder, or if the provided seek format is invalid.  The valid
seek types are seek_pos and seek_line_char.)
DOC(Computes a Partial_Cursor for the given seek position with no side effects.
The seek position must be one of the types supported by Partial_Cursor.  Those
types are absolute position and line,column position.)
DOC_SEE(Buffer_Seek)
DOC_SEE(Partial_Cursor)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Partial_Cursor result = {};
    if (api_check_buffer(file)){
        result = file_compute_partial_cursor(file, seek);
    }
    return(result);
}

API_EXPORT b32
Buffer_Exists(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    return(api_check_buffer(file));
}

API_EXPORT b32
Buffer_Ready(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = file_is_ready(file);
    }
    return(result);
}

API_EXPORT Access_Flag
Buffer_Get_Access_Flags(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Access_Flag result = 0;
    if (api_check_buffer(file)){
        result = file_get_access_flags(file);
    }
    return(result);
}

API_EXPORT i64
Buffer_Get_Size(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = 0;
    if (api_check_buffer(file)){
        result = buffer_size(&file->state.buffer);
    }
    return(result);
}

API_EXPORT i64
Buffer_Get_Line_Count(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    u64 result = 0;
    if (api_check_buffer(file)){
        result = file->state.buffer.line_count;
    }
    return(result);
}

API_EXPORT String_Const_u8
Push_Buffer_Base_Name(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(arena, string_from_file_name(&file->base_name));
    }
    return(result);
}

API_EXPORT String_Const_u8
Push_Buffer_Unique_Name(Application_Links *app, Arena *out, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(out, string_from_file_name(&file->unique_name));
    }
    return(result);
}

API_EXPORT String_Const_u8
Push_Buffer_File_Name(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(arena, string_from_file_name(&file->canon));
    }
    return(result);
}

API_EXPORT Dirty_State
Buffer_Get_Dirty_State(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Dirty_State result = {};
    if (api_check_buffer(file)){
        result = file->state.dirty;
    }
    return(result);
}

API_EXPORT b32
Buffer_Set_Dirty_State(Application_Links *app, Buffer_ID buffer_id, Dirty_State dirty_state){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        file->state.dirty = dirty_state;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Buffer_Tokens_Are_Ready(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = file_tokens_are_ready(file);
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Get_Setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 *value_out)
/*
DOC_PARAM(buffer, the buffer from which to read a setting)
DOC_PARAM(setting, the setting to read from the buffer)
DOC_PARAM(value_out, address to write the setting value on success)
DOC_RETURN(returns non-zero on success)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        switch (setting){
            case BufferSetting_Lex:
            {
                *value_out = file->settings.tokens_exist;
            }break;
            
            case BufferSetting_LexWithoutStrings:
            {
                *value_out = file->settings.tokens_without_strings;
            }break;
            
            case BufferSetting_ParserContext:
            {
                *value_out = file->settings.parse_context_id;
            }break;
            
            case BufferSetting_WrapLine:
            {
                *value_out = !file->settings.unwrapped_lines;
            }break;
            
            case BufferSetting_WrapPosition:
            {
                *value_out = file->settings.display_width;
            }break;
            
            case BufferSetting_MinimumBaseWrapPosition:
            {
                *value_out = file->settings.minimum_base_display_width;
            }break;
            
            case BufferSetting_MapID:
            {
                *value_out = file->settings.base_map_id;
            }break;
            
            case BufferSetting_Eol:
            {
                *value_out = file->settings.dos_write_mode;
            }break;
            
            case BufferSetting_Unimportant:
            {
                *value_out = file->settings.unimportant;
            }break;
            
            case BufferSetting_ReadOnly:
            {
                *value_out = file->settings.read_only;
            }break;
            
            case BufferSetting_VirtualWhitespace:
            {
                *value_out = file->settings.virtual_white;
            }break;
            
            case BufferSetting_RecordsHistory:
            {
                *value_out = history_is_activated(&file->state.history);
            }break;
            
            default:
            {
                result = false;
            }break;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Set_Setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 value)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer on which to set a setting.)
DOC_PARAM(setting, The setting parameter identifies the setting that shall be changed.)
DOC_PARAM(value, The value parameter specifies the value to which the setting shall be changed.)
DOC_SEE(Buffer_Setting_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        switch (setting){
            case BufferSetting_Lex:
            {
                if (file->settings.tokens_exist){
                    if (!value){
                        file_kill_tokens(system, &models->mem.heap, file);
                    }
                }
                else{
                    if (value){
                        file_first_lex(system, models, file);
                    }
                }
            }break;
            
            case BufferSetting_LexWithoutStrings:
            {
                if (file->settings.tokens_exist){
                    if ((b8)value != file->settings.tokens_without_strings){
                        file_kill_tokens(system, &models->mem.heap, file);
                        file->settings.tokens_without_strings = (b8)value;
                        file_first_lex(system, models, file);
                    }
                }
                else{
                    file->settings.tokens_without_strings = (b8)value;
                }
            }break;
            
            case BufferSetting_ParserContext:
            {
                u32 fixed_value = parse_context_valid_id(&models->parse_context_memory, (u32)value);
                
                if (file->settings.tokens_exist){
                    if (fixed_value != file->settings.parse_context_id){
                        file_kill_tokens(system, &models->mem.heap, file);
                        file->settings.parse_context_id = fixed_value;
                        file_first_lex(system, models, file);
                    }
                }
                else{
                    file->settings.parse_context_id = fixed_value;
                }
            }break;
            
            case BufferSetting_WrapLine:
            {
                file->settings.unwrapped_lines = !value;
            }break;
            
            case BufferSetting_WrapPosition:
            {
                i32 new_value = value;
                if (new_value < 48){
                    new_value = 48;
                }
                if (new_value != file->settings.display_width){
                    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
                    file->settings.display_width = new_value;
                    file_measure_wraps(system, &models->mem, file, face);
                    adjust_views_looking_at_file_to_new_cursor(system, models, file);
                }
            }break;
            
            case BufferSetting_MinimumBaseWrapPosition:
            {
                i32 new_value = value;
                if (new_value < 0){
                    new_value = 0;
                }
                if (new_value != file->settings.minimum_base_display_width){
                    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
                    file->settings.minimum_base_display_width = new_value;
                    file_measure_wraps(system, &models->mem, file, face);
                    adjust_views_looking_at_file_to_new_cursor(system, models, file);
                }
            }break;
            
            case BufferSetting_WrapIndicator:
            {
                file->settings.wrap_indicator = value;
            }break;
            
            case BufferSetting_MapID:
            {
                if (value < mapid_global){
                    i32 new_mapid = get_map_index(&models->mapping, value);
                    if (new_mapid < models->mapping.user_map_count){
                        file->settings.base_map_id = value;
                    }
                    else{
                        file->settings.base_map_id = mapid_file;
                    }
                }
                else if (value <= mapid_nomap){
                    file->settings.base_map_id = value;
                }
            }break;
            
            case BufferSetting_Eol:
            {
                file->settings.dos_write_mode = value;
            }break;
            
            case BufferSetting_Unimportant:
            {
                if (value != 0){
                    file_set_unimportant(file, true);
                }
                else{
                    file_set_unimportant(file, false);
                }
            }break;
            
            case BufferSetting_ReadOnly:
            {
                if (value){
                    file->settings.read_only = true;
                }
                else{
                    file->settings.read_only = false;
                }
            }break;
            
            case BufferSetting_VirtualWhitespace:
            {
                b32 full_remeasure = false;
                if (value){
                    if (!file->settings.virtual_white){
                        if (!file->settings.tokens_exist){
                            file_first_lex_serial(system, models, file);
                        }
                        if (!file->state.still_lexing){
                            file->settings.virtual_white = true;
                            full_remeasure = true;
                        }
                        else{
                            result = false;
                        }
                    }
                }
                else{
                    if (file->settings.virtual_white){
                        file->settings.virtual_white = false;
                        full_remeasure = true;
                    }
                }
                
                if (full_remeasure){
                    Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
                    file_allocate_character_starts_as_needed(&models->mem.heap, file);
                    buffer_measure_character_starts(system, &file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
                    file_measure_wraps(system, &models->mem, file, face);
                    adjust_views_looking_at_file_to_new_cursor(system, models, file);
                }
            }break;
            
            case BufferSetting_RecordsHistory:
            {
                if (value){
                    if (!history_is_activated(&file->state.history)){
                        history_init(app, &file->state.history);
                    }
                }
                else{
                    if (history_is_activated(&file->state.history)){
                        history_free(&models->mem.heap, &file->state.history);
                    }
                }
            }break;
            
            default:
            {
                result = 0;
            }break;
        }
    }
    
    return(result);
}

// TODO(allen): redocument
API_EXPORT Managed_Scope
Buffer_Get_Managed_Scope(Application_Links *app, Buffer_ID buffer_id)
/*
DOC_PARAM(buffer_id, The id of the buffer from which to get a managed scope.)
DOC_RETURN(If the buffer_id specifies a valid buffer, the scope returned is the scope tied to the
lifetime of the buffer.  This is a 'basic scope' and is not considered a 'user managed scope'.
If the buffer_id does not specify a valid buffer, the returned scope is null.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Managed_Scope result = 0;
    if (api_check_buffer(file)){
        result = file_get_managed_scope(file);
    }
    return(result);
}

// TODO(allen): redocument
// TODO(allen): Include warning note about pointers and editing buffers.
API_EXPORT Cpp_Token_Array
Buffer_Get_Token_Array(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Cpp_Token_Array result = {};
    if (api_check_buffer_and_tokens(file)){
        result = file->state.token_array;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Send_End_Signal(Application_Links *app, Buffer_ID buffer_id)
/*
DOC_PARAM(buffer, The buffer to which to send the end signal.)
DOC_RETURN(Returns non-zero on success.  This call can fail if the buffer doesn't exist.)
DOC(Whenever a buffer is killed an end signal is sent which triggers the end file hook.  This call sends the end signal to the buffer without killing the buffer.
This is useful in cases such as clearing a buffer and refilling it with new contents.)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        file_end_file(models, file);
        result = true;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Buffer_ID
Create_Buffer(Application_Links *app, String_Const_u8 file_name, Buffer_Create_Flag flags)
/*
DOC_PARAM(filename, The name of the file to associate to the new buffer.)
DOC_PARAM(filename_len, The length of the filename string.)
DOC_PARAM(flags, Flags controlling the buffer creation behavior.)
DOC_RETURN(Returns the newly created buffer or an already existing buffer with the given name.)
DOC(
Try to create a new buffer.  This call first checks to see if a buffer already exists that goes by the given name, if so, that buffer is returned.

If no buffer exists with the given name, then a new buffer is created.  If a file that matches the given filename exists, the file is loaded as the contents of the new buffer.
Otherwise a buffer is created without a matching file until the buffer is saved and the buffer is left blank.
)
DOC_SEE(Buffer_Create_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *new_file = create_file(models, file_name, flags);
    Buffer_ID result = 0;
    if (new_file != 0){
        result = new_file->id.id;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Buffer_Save(Application_Links *app, Buffer_ID buffer_id, String_Const_u8 file_name, Buffer_Save_Flag flags)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer to save to a file.)
DOC_PARAM(file_name, The file_name parameter specifies the name of the file to write with the contents of the buffer; it need not be null terminated.)
DOC_PARAM(file_name_len, The file_name_len parameter specifies the length of the file_name string.)
DOC_PARAM(flags, Specifies special behaviors for the save routine.)
DOC_RETURN(This call returns non-zero on success.)
DOC(Often it will make sense to set file_name and file_name_len to buffer.file_name and buffer.file_name_len)
DOC_SEE(Buffer_Save_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer_id);
    
    b32 result = false;
    if (api_check_buffer(file)){
        b32 skip_save = false;
        if (!HasFlag(flags, BufferSave_IgnoreDirtyFlag)){
            if (file->state.dirty == DirtyState_UpToDate){
                skip_save = true;
            }
        }
        
        if (!skip_save){
            Arena *scratch = &models->mem.arena;
            Temp_Memory temp = begin_temp(scratch);
            String_Const_u8 name = push_string_copy(scratch, file_name);
            save_file_to_name(system, models, file, name.str);
            end_temp(temp);
            result = true;
        }
    }
    
    return(result);
}

// TODO(allen): redocument
API_EXPORT Buffer_Kill_Result
Buffer_Kill(Application_Links *app, Buffer_ID buffer_id, Buffer_Kill_Flag flags)
/*
DOC_PARAM(buffer, The buffer parameter specifies the buffer to try to kill.)
DOC_PARAM(flags, The flags parameter specifies behaviors for the buffer kill.)
DOC_RETURN(This call returns BufferKillResult_Killed if the call successfully kills the buffer,
for extended information on other kill results see the Buffer_Kill_Result enumeration.)

DOC(Tries to kill the idenfied buffer, if the buffer is dirty or does not exist then nothing will
happen. The default rules about when to kill or not kill a buffer can be altered by the flags.)

DOC_SEE(Buffer_Kill_Result)
DOC_SEE(Buffer_Kill_Flag)
DOC_SEE(Buffer_Identifier)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = imp_get_file(models, buffer_id);
    Buffer_Kill_Result result = BufferKillResult_DoesNotExist;
    if (api_check_buffer(file)){
        if (!file->settings.never_kill){
            b32 needs_to_save = file_needs_save(file);
            if (!needs_to_save || (flags & BufferKill_AlwaysKill) != 0){
                if (models->hook_end_file != 0){
                    models->hook_end_file(&models->app_links, file->id.id);
                }
                
                buffer_unbind_name_low_level(working_set, file);
                if (file->canon.name_size != 0){
                    buffer_unbind_file(system, working_set, file);
                }
                file_free(system, &models->mem.heap, &models->lifetime_allocator, working_set, file);
                working_set_free_file(&models->mem.heap, working_set, file);
                
                Layout *layout = &models->layout;
                
                Node *used = &working_set->used_sentinel;
                Node *file_node = used->next;
                for (Panel *panel = layout_get_first_open_panel(layout);
                     panel != 0;
                     panel = layout_get_next_open_panel(layout, panel)){
                    View *view = panel->view;
                    if (view->file == file){
                        Assert(file_node != used);
                        view->file = 0;
                        Editing_File *new_file = CastFromMember(Editing_File, main_chain_node, file_node);
                        view_set_file(system, models, view, new_file);
                        if (file_node->next != used){
                            file_node = file_node->next;
                        }
                        else{
                            file_node = file_node->next->next;
                            Assert(file_node != used);
                        }
                    }
                }
                
                result = BufferKillResult_Killed;
            }
            else{
                result = BufferKillResult_Dirty;
            }
        }
        else{
            result = BufferKillResult_Unkillable;
        }
    }
    return(result);
}

API_EXPORT Buffer_Reopen_Result
Buffer_Reopen(Application_Links *app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer_id);
    Buffer_Reopen_Result result = BufferReopenResult_Failed;
    if (api_check_buffer(file)){
        if (file->canon.name_size > 0){
            Plat_Handle handle = {};
            if (system->load_handle((char*)file->canon.name_space, &handle)){
                File_Attributes attributes = system->load_attributes(handle);
                
                Arena *arena = &models->mem.arena;
                Temp_Memory temp = begin_temp(arena);
                char *file_memory = push_array(arena, char, (i32)attributes.size);
                
                if (file_memory != 0){
                    if (system->load_file(handle, file_memory, (i32)attributes.size)){
                        system->load_close(handle);
                        
                        // TODO(allen): try(perform a diff maybe apply edits in reopen)
                        
                        i32 line_numbers[16];
                        i32 column_numbers[16];
                        View *vptrs[16];
                        i32 vptr_count = 0;
                        
                        Layout *layout = &models->layout;
                        for (Panel *panel = layout_get_first_open_panel(layout);
                             panel != 0;
                             panel = layout_get_next_open_panel(layout, panel)){
                            View *view_it = panel->view;
                            if (view_it->file == file){
                                vptrs[vptr_count] = view_it;
                                File_Edit_Positions edit_pos = view_get_edit_pos(view_it);
                                Full_Cursor cursor = file_compute_cursor(models, view_it->file, seek_pos(edit_pos.cursor_pos));
                                line_numbers[vptr_count]   = (i32)cursor.line;
                                column_numbers[vptr_count] = (i32)cursor.character;
                                view_it->file = models->scratch_buffer;
                                ++vptr_count;
                            }
                        }
                        
                        Working_Set *working_set = &models->working_set;
                        file_free(system, &models->mem.heap, &models->lifetime_allocator, working_set, file);
                        working_set_file_default_settings(working_set, file);
                        file_create_from_string(system, models, file, SCu8(file_memory, attributes.size), attributes);
                        
                        for (i32 i = 0; i < vptr_count; ++i){
                            view_set_file(system, models, vptrs[i], file);
                            
                            vptrs[i]->file = file;
                            Full_Cursor cursor = file_compute_cursor(models, file, seek_line_char(line_numbers[i], column_numbers[i]));
                            
                            view_set_cursor(system, models, vptrs[i], cursor, true);
                        }
                        result = BufferReopenResult_Reopened;
                    }
                    else{
                        system->load_close(handle);
                    }
                }
                else{
                    system->load_close(handle);
                }
                
                end_temp(temp);
            }
        }
    }
    return(result);
}

API_EXPORT File_Attributes
Buffer_Get_File_Attributes(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    File_Attributes result = {};
    if (api_check_buffer(file)){
        block_copy_struct(&result, &file->attributes);
    }
    return(result);
}

API_EXPORT File_Attributes
Get_File_Attributes(Application_Links *app, String_Const_u8 file_name)
{
    Models *models = (Models*)app->cmd_context;
    File_Attributes attributes = models->system->quick_file_attributes(file_name);
    return(attributes);
}

internal View*
get_view_next__inner(Layout *layout, View *view){
    if (view != 0){
        Panel *panel = view->panel;
        panel = layout_get_next_open_panel(layout, panel);
        if (panel != 0){
            view = panel->view;
        }
        else{
            view = 0;
        }
    }
    else{
        Panel *panel = layout_get_first_open_panel(layout);
        view = panel->view;
    }
    return(view);
}

internal View*
get_view_prev__inner(Layout *layout, View *view){
    if (view != 0){
        Panel *panel = view->panel;
        panel = layout_get_prev_open_panel(layout, panel);
        if (panel != 0){
            view = panel->view;
        }
        else{
            view = 0;
        }
    }
    else{
        Panel *panel = layout_get_first_open_panel(layout);
        view = panel->view;
    }
    return(view);
}

// TODO(allen): redocument
API_EXPORT View_ID
Get_View_Next(Application_Links *app, View_ID view_id, Access_Flag access)
/*
DOC_PARAM(view, The  pointed to by view is iterated to the next view or to a null summary if this is the last view.)
DOC_PARAM(access, The access parameter determines what levels of protection this call can access. The view outputted will be the next view that is accessible.)
DOC
(
This call steps a  to the next view in the global view order.

If the view outputted does not exist, the loop is finished.
Views should not be closed or opened durring a view loop.
)
DOC_SEE(Access_Flag)
DOC_SEE(get_view_first)
*/{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *view = imp_get_view(models, view_id);
    view = get_view_next__inner(layout, view);
    for (;view != 0 && !access_test(view_get_access_flags(view), access);){
        view = get_view_next__inner(layout, view);
    }
    View_ID result = 0;
    if (view != 0){
        result = view_get_id(&models->live_set, view);
    }
    return(result);
}

API_EXPORT View_ID
Get_View_Prev(Application_Links *app, View_ID view_id, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *view = imp_get_view(models, view_id);
    view = get_view_prev__inner(layout, view);
    for (;view != 0 && !access_test(view_get_access_flags(view), access);){
        view = get_view_prev__inner(layout, view);
    }
    View_ID result = 0;
    if (view != 0){
        result = view_get_id(&models->live_set, view);
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT View_ID
Get_Active_View(Application_Links *app, Access_Flag access)
/*
DOC_PARAM(access, The access parameter determines what levels of protection this call can access.)
DOC_RETURN(This call returns a summary that describes the active view.)
DOC_SEE(set_active_view)
DOC_SEE(Access_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    Panel *panel = layout_get_active_panel(&models->layout);
    Assert(panel != 0);
    View *view = panel->view;
    Assert(view != 0);
    View_ID result = 0;
    if (api_check_view(view, access)){
        result = view_get_id(&models->live_set, view);
    }
    return(result);
}

API_EXPORT Panel_ID
Get_Active_Panel(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    Panel *panel = layout_get_active_panel(&models->layout);
    Assert(panel != 0);
    Panel_ID result = 0;
    if (api_check_panel(panel)){
        result = panel_get_id(&models->layout, panel);
    }
    return(result);
}

API_EXPORT b32
View_Exists(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = true;
    }
    return(result);
}

API_EXPORT Buffer_ID
View_Get_Buffer(Application_Links *app, View_ID view_id, Access_Flag access){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Buffer_ID result = 0;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        if (api_check_buffer(file, access)){
            result = file->id.id;
        }
    }
    return(result);
}

API_EXPORT i64
View_Get_Cursor_Pos(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = 0;
    if (api_check_view(view)){
        File_Edit_Positions edit_pos = view_get_edit_pos(view);
        result = edit_pos.cursor_pos;
    }
    return(result);
}

API_EXPORT i64
View_Get_Mark_Pos(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = 0;;
    if (api_check_view(view)){
        result = view->mark;
    }
    return(result);
}

API_EXPORT f32
View_Get_Preferred_X(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    f32 result = 0.f;;
    if (api_check_view(view)){
        result = view->preferred_x;
    }
    return(result);
}

API_EXPORT Rect_f32
View_Get_Screen_Rect(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        result = f32R(view->panel->rect_full);
    }
    return(result);
}

API_EXPORT Panel_ID
View_Get_Panel(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *view = imp_get_view(models, view_id);
    Panel_ID result = 0;
    if (api_check_view(view)){
        Panel *panel = view->panel;
        result = panel_get_id(layout, panel);
    }
    return(result);
}

API_EXPORT View_ID
Panel_Get_View(Application_Links *app, Panel_ID panel_id){
    Models *models = (Models*)app->cmd_context;
    Panel *panel = imp_get_panel(models, panel_id);
    View_ID result = 0;
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Final){
            View *view = panel->view;
            Assert(view != 0);
            result = view_get_id(&models->live_set, view);
        }
    }
    return(result);
}

API_EXPORT b32
Panel_Is_Split(Application_Links *app, Panel_ID panel_id){
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Intermediate){
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Panel_Is_Leaf(Application_Links *app, Panel_ID panel_id){
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Final){
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Panel_Split(Application_Links *app, Panel_ID panel_id, Panel_Split_Orientation orientation){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        Panel *new_panel = 0;
        if (layout_split_panel(layout, panel, (orientation == PanelSplit_LeftAndRight), &new_panel)){
            Live_Views *live_set = &models->live_set;
            View *new_view = live_set_alloc_view(&models->mem.heap, &models->lifetime_allocator, live_set, new_panel);
            view_set_file(models->system, models, new_view, models->scratch_buffer);
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Panel_Set_Split(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Intermediate){
            panel->split.kind = kind;
            switch (kind){
                case PanelSplitKind_Ratio_Max:
                case PanelSplitKind_Ratio_Min:
                {
                    panel->split.v_f32 = clamp(0.f, t, 1.f);
                }break;
                
                case PanelSplitKind_FixedPixels_Max:
                case PanelSplitKind_FixedPixels_Min:
                {
                    panel->split.v_i32 = round32(t);
                }break;
                
                default:
                {
                    print_message(app, string_u8_litexpr("Invalid split kind passed to panel_set_split, no change made to view layout"));
                }break;
            }
            layout_propogate_sizes_down_from_node(layout, panel);
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Panel_Swap_Children(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind, float t){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Intermediate){
            Swap(Panel*, panel->tl_panel, panel->br_panel);
            layout_propogate_sizes_down_from_node(layout, panel);
        }
    }
    return(result);
}

API_EXPORT Panel_ID
Panel_Get_Parent(Application_Links *app, Panel_ID panel_id){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    Panel *panel = imp_get_panel(models, panel_id);
    Panel_ID result = false;
    if (api_check_panel(panel)){
        result = panel_get_id(layout, panel->parent);
    }
    return(result);
}

API_EXPORT Panel_ID
Panel_Get_Child(Application_Links *app, Panel_ID panel_id, Panel_Child which_child){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    Panel *panel = imp_get_panel(models, panel_id);
    Panel_ID result = 0;
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Intermediate){
            Panel *child = 0;
            switch (which_child){
                case PanelChild_Min:
                {
                    child = panel->tl_panel;
                }break;
                case PanelChild_Max:
                {
                    child = panel->br_panel;
                }break;
            }
            if (child != 0){
                result = panel_get_id(layout, child);
            }
        }
    }
    return(result);
}

API_EXPORT Panel_ID
Panel_Get_Max(Application_Links *app, Panel_ID panel_id){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    Panel *panel = imp_get_panel(models, panel_id);
    Panel_ID result = 0;
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Intermediate){
            Panel *child = panel->br_panel;
            result = panel_get_id(layout, child);
        }
    }
    return(result);
}

API_EXPORT Rect_i32
Panel_Get_Margin(Application_Links *app, Panel_ID panel_id){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    Panel *panel = imp_get_panel(models, panel_id);
    Rect_i32 result = {};
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Final){
            i32 margin = layout->margin;
            result.x0 = margin;
            result.x1 = margin;
            result.y0 = margin;
            result.y1 = margin;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Close(Application_Links *app, View_ID view_id)
/*
DOC_PARAM(view, The view parameter specifies which view to close.)
DOC_RETURN(This call returns non-zero on success.)

DOC(If the given view is open and is not the last view, it will be closed.
If the given view is the active view, the next active view in the global
order of view will be made active. If the given view is the last open view
in the system, the call will fail.)

*/{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        if (layout_close_panel(layout, view->panel)){
            live_set_free_view(&models->mem.heap, &models->lifetime_allocator, &models->live_set, view);
            result = true;
        }
    }
    return(result);
}

API_EXPORT Rect_f32
View_Get_Buffer_Region(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Rect_f32 result = {};
    if (api_check_view(view)){
        result = view_get_buffer_rect(models, view);
    }
    return(result);
}

API_EXPORT GUI_Scroll_Vars
View_Get_Scroll_Vars(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    GUI_Scroll_Vars result = {};
    if (api_check_view(view)){
        if (view->ui_mode){
            result = view->ui_scroll;
        }
        else{
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            result = edit_pos.scroll;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Active(Application_Links *app, View_ID view_id)
/*
DOC_PARAM(view, The view parameter specifies which view to make active.)
DOC_RETURN(This call returns non-zero on success.)
DOC(If the given view is open, it is set as the
active view, and takes subsequent commands and is returned
from get_active_view.)
DOC_SEE(get_active_view)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        models->layout.active_panel = view->panel;
        result = true;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Get_Setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 *value_out)
/*
DOC_PARAM(view, the view from which to read a setting)
DOC_PARAM(setting, the view setting to read)
DOC_PARAM(value_out, address to write the setting value on success)
DOC_RETURN(returns non-zero on success)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    
    b32 result = false;
    if (api_check_view(view)){
        result = true;
        switch (setting){
            case ViewSetting_ShowWhitespace:
            {
                *value_out = view->show_whitespace;
            }break;
            
            case ViewSetting_ShowScrollbar:
            {
                *value_out = !view->hide_scrollbar;
            }break;
            
            case ViewSetting_ShowFileBar:
            {
                *value_out = !view->hide_file_bar;
            }break;
            
            case ViewSetting_UICommandMap:
            {
                *value_out = view->ui_map_id;
            }break;
            
            default:
            {
                result = false;
            }break;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 value)
/*
DOC_PARAM(view, The view parameter specifies the view on which to set a setting.)
DOC_PARAM(setting, The setting parameter identifies the setting that shall be changed.)
DOC_PARAM(value, The value parameter specifies the value to which the setting shall be changed.)
DOC_RETURN(This call returns non-zero on success.)
DOC_SEE(View_Setting_ID)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    
    b32 result = false;
    if (api_check_view(view)){
        result = true;
        switch (setting){
            case ViewSetting_ShowWhitespace:
            {
                view->show_whitespace = value;
            }break;
            
            case ViewSetting_ShowScrollbar:
            {
                view->hide_scrollbar = !value;
            }break;
            
            case ViewSetting_ShowFileBar:
            {
                view->hide_file_bar = !value;
            }break;
            
            case ViewSetting_UICommandMap:
            {
                view->ui_map_id = value;
            }break;
            
            default:
            {
                result = false;
            }break;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Managed_Scope
View_Get_Managed_Scope(Application_Links *app, View_ID view_id)
/*
DOC_PARAM(view_id, The id of the view from which to get a managed scope.)
DOC_RETURN(If the view_id specifies a valid view, the scope returned is the scope tied to the
lifetime of the view.  This is a 'basic scope' and is not considered a 'user managed scope'.
If the view_id does not specify a valid view, the returned scope is null.)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Managed_Scope result = 0;
    if (api_check_view(view)){
        Assert(view->lifetime_object != 0);
        result = (Managed_Scope)(view->lifetime_object->workspace.scope_id);
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Full_Cursor
View_Compute_Cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek)
/*
DOC_PARAM(view, The view parameter specifies the view on which to run the cursor computation.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(cursor_out, On success this struct is filled with the result of the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(Computes a Full_Cursor for the given seek position with no side effects.)
DOC_SEE(Buffer_Seek)
DOC_SEE(Full_Cursor)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Full_Cursor result = {};
    if (api_check_view(view)){
        Editing_File *file = view->file;
        if (api_check_buffer(file)){
            if (file->settings.unwrapped_lines && seek.type == buffer_seek_wrapped_xy){
                seek.type = buffer_seek_unwrapped_xy;
            }
            result = file_compute_cursor(models, file, seek);
            if (file->settings.unwrapped_lines){
                result.wrapped_x = result.unwrapped_x;
                result.wrapped_y = result.unwrapped_y;
            }
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek, b32 set_preferred_x)
/*
DOC_PARAM(view, The view parameter specifies the view in which to set the cursor.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_PARAM(set_preferred_x, If this parameter is true the preferred x is updated to match the new cursor x.)
DOC_RETURN(This call returns non-zero on success.)
DOC(This call sets the the view's cursor position.  set_preferred_x should usually be true unless the change in cursor position is is a vertical motion that tries to keep the cursor in the same column or x position.)
DOC_SEE(Buffer_Seek)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Assert(file != 0);
        if (api_check_buffer(file)){
            if (file->settings.unwrapped_lines && seek.type == buffer_seek_wrapped_xy){
                seek.type = buffer_seek_unwrapped_xy;
            }
            Full_Cursor cursor = file_compute_cursor(models, file, seek);
            view_set_cursor(models->system, models, view, cursor, set_preferred_x);
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Scroll(Application_Links *app, View_ID view_id, GUI_Scroll_Vars scroll)
/*
DOC_PARAM(view, The view on which to change the scroll state.)
DOC_PARAM(scroll, The new scroll position for the view.)
DOC(Set the scrolling state of the view.)
DOC_SEE(GUI_Scroll_Vars)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Assert(file != 0);
        if (api_check_buffer(file)){
            if (!view->ui_mode){
                view_set_scroll(models->system, models, view, scroll);
            }
            else{
                view->ui_scroll = scroll;
            }
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Mark(Application_Links *app, View_ID view_id, Buffer_Seek seek)
/*
DOC_PARAM(view, The view parameter specifies the view in which to set the mark.)
DOC_PARAM(seek, The seek parameter specifies the target position for the seek.)
DOC_RETURN(This call returns non-zero on success.)
DOC(This call sets the the view's mark position.)
DOC_SEE(Buffer_Seek)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Assert(file != 0);
        if (api_check_buffer(file)){
            if (seek.type != buffer_seek_pos){
                Full_Cursor cursor = file_compute_cursor(models, file, seek);
                view->mark = cursor.pos;
            }
            else{
                view->mark = seek.pos;
            }
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags)
/*
DOC_PARAM(view, The view parameter specifies the view in which to display the buffer.)
DOC_PARAM(buffer_id, The buffer_id parameter specifies which buffer to show in the view.)
DOC_PARAM(flags, The flags parameter specifies behaviors for setting the buffer.)
DOC_RETURN(This call returns non-zero on success.)
DOC(On success view_set_buffer sets the specified view's current buffer and cancels and dialogue shown in the view and displays the file.)
DOC_SEE(Set_Buffer_Flag)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = working_set_get_active_file(&models->working_set, buffer_id);
        if (api_check_buffer(file)){
            if (file != view->file){
                view_set_file(models->system, models, view, file);
                if (!(flags & SetBuffer_KeepOriginalGUI)){
                    view_quit_ui(models->system, models, view);
                }
            }
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Post_Fade(Application_Links *app, View_ID view_id, f32 seconds, Range_i64 range, int_color color)
/*
DOC_PARAM(view, The view parameter specifies the view onto which the fade effect shall be posted.)
DOC_PARAM(seconds, This parameter specifies the number of seconds the fade effect should last.)
DOC_PARAM(start, This parameter specifies the absolute position of the first character of the fade range.)
DOC_PARAM(end, This parameter specifies the absolute position of the character one past the end of the fdae range.)
DOC_PARAM(color, The color parameter specifies the initial color of the text before it fades to it's natural color.)
DOC_RETURN(This call returns non-zero on success.)
DOC_SEE(int_color)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        i64 size = range_size(range);
        if (size > 0){
            view_post_paste_effect(view, seconds, (i32)range.start, (i32)size, color|0xFF000000);
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Begin_UI_Mode(Application_Links *app, View_ID view_id)
/*
DOC_PARAM(view, A summary for the view which is to be placed into UI mode.)
DOC_RETURN(This call returns non-zero on success.  It can fail if view is invalid, or if the view is already in UI mode.)
DOC(In UI mode, the view no longer renders it's buffer.  Instead it renders UI elements attached to the view.  The UI elements attached to a view can be modified by a view_set_ui call.)
DOC_SEE(view_end_ui_mode)
DOC_SEE(view_set_ui)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        if (!view->ui_mode){
            view->ui_mode = true;
            result = true;
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_End_UI_Mode(Application_Links *app, View_ID view_id)
/*
DOC_PARAM(view, A summary for the view which is to be taken out of UI mode.)
DOC_RETURN(This call returns non-zero on success.  It can fail if view is invalid, or if the view is not in UI mode.)
DOC(Taking a view out of UI mode not only causes it to resume showing a buffer, but also sends a quit UI signal.
The exit signal calls the quit_function that is set by a view_set_ui call.)
DOC_SEE(view_begin_ui_mode)
DOC_SEE(view_set_ui)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view) && view->ui_mode){
        view_quit_ui(models->system, models, view);
        view->ui_mode = false;
        result = true;
    }
    return(result);
}

API_EXPORT b32
View_Is_In_UI_Mode(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = view->ui_mode;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Set_Quit_UI_Handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type *quit_function)
/*
DOC_PARAM(view, A summary for the view which is to get the new UI widget data.)
DOC_PARAM(control, A pointer to the baked UI widget data.  To get data in the UI_Control format see the helper called ui_list_to_ui_control.  More information about specifying widget data can be found in a comment in '4coder_ui_helper.cpp'.  If this pointer is null the view's widget data is cleared to zero.  The core maintains it's own copy of the widget data, so after this call the memory used to specify widget data may be freed.)
DOC_PARAM(quit_function, A function pointer to be called when a quit UI signal is sent to the view.  This pointer may be set to null.)
DOC_RETURN(This call returns non-zero on success.  It can fail if view is invalid.)
DOC(Setting the UI widget data determines what the view will look like in UI mode.  The UI widget data can be set no matter what the current mode is.  The UI widget data can be replaced any number of times regardless of whether the view is in UI mode or not.  See documentation on the UI widget data type UI_Control and the comments at the top of '4coder_ui_helper.cpp' for more information about the types of widgets that can be used.)
DOC_SEE(view_begin_ui_mode)
DOC_SEE(view_end_ui_mode)
DOC_SEE(UI_Control)
DOC_SEE(UI_Quit_Function_Type)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        view->ui_quit = quit_function;
        result = true;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
View_Get_Quit_UI_Handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type **quit_function_out)
/*
DOC_PARAM(view, A summary for the view which is to get the new UI widget data.)
DOC_PARAM(part, A memory arena onto which the UI widget data will be allocated.)
DOC_RETURN(If the view is valid, and there is enough space in part, then the UI_Control stucture returned points to a copy of the widget data currently attached to the view.)
DOC_SEE(view_set_ui)
*/
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        *quit_function_out = view->ui_quit;
        result = true;
    }
    return(result);
}

internal Dynamic_Workspace*
get_dynamic_workspace(Models *models, Managed_Scope handle){
    u32_Ptr_Lookup_Result lookup_result = lookup_u32_Ptr_table(&models->lifetime_allocator.scope_id_to_scope_ptr_table, (u32)handle);
    if (!lookup_result.success){
        return(0);
    }
    return((Dynamic_Workspace*)*lookup_result.val);
}

API_EXPORT Managed_Scope
Create_User_Managed_Scope(Application_Links *app)
/*
DOC_RETURN(Always returns a newly created scope with a handle that is unique from all other scopes that have been created either by the core or the custom layer.  The new scope is a 'basic scope' and is a 'user managed scope'.)
DOC_SEE(destroy_user_managed_scope)
*/
{
    Models *models = (Models*)app->cmd_context;
    Heap *heap = &models->mem.heap;
    Lifetime_Object *object = lifetime_alloc_object(heap, &models->lifetime_allocator, DynamicWorkspace_Unassociated, 0);
    object->workspace.user_back_ptr = object;
    Managed_Scope scope = (Managed_Scope)object->workspace.scope_id;
    return(scope);
}

API_EXPORT b32
Destroy_User_Managed_Scope(Application_Links *app, Managed_Scope scope)
/*
DOC_PARAM(scope, The handle for the scope which is to be destroyed)
DOC_RETURN(If the scope is a valid 'user managed scope' this function returns non-zero, otherwise it returns zero.)
DOC(This call only operates on 'user managed scopes'.  'User managed scopes' are those scopes created by create_user_managed_scope.  Scopes tied to views, and buffers, and the global scope are not 'user managed scopes'.)
DOC_SEE(create_user_managed_scope)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0 && workspace->user_type == DynamicWorkspace_Unassociated){
        Lifetime_Object *lifetime_object = (Lifetime_Object*)workspace->user_back_ptr;
        lifetime_free_object(&models->mem.heap, &models->lifetime_allocator, lifetime_object);
        result = true;
    }
    return(result);
}

API_EXPORT Managed_Scope
Get_Global_Managed_Scope(Application_Links *app)
/*
DOC_RETURN(Always returns the handle to the 'global scope'.  The 'global scope' is unique in that it is not dependent on any object at all.  The handle for the 'global scope' never changes in a 4coder session, and when used as a dependency in get_managed_scope_with_multiple_dependencies it has no effect.)
*/
{
    Models *models = (Models*)app->cmd_context;
    return((Managed_Scope)models->dynamic_workspace.scope_id);
}

internal Lifetime_Object*
get_lifetime_object_from_workspace(Dynamic_Workspace *workspace){
    Lifetime_Object *result = 0;
    switch (workspace->user_type){
        case DynamicWorkspace_Unassociated:
        {
            result = (Lifetime_Object*)workspace->user_back_ptr;
        }break;
        case DynamicWorkspace_Buffer:
        {
            Editing_File *file = (Editing_File*)workspace->user_back_ptr;
            result = file->lifetime_object;
        }break;
        case DynamicWorkspace_View:
        {
            View *vptr = (View*)workspace->user_back_ptr;
            result = vptr->lifetime_object;
        }break;
        default:
        {
            InvalidPath;
        }break;
    }
    return(result);
}

API_EXPORT Managed_Scope
Get_Managed_Scope_With_Multiple_Dependencies(Application_Links *app, Managed_Scope *scopes, i32 count)
/*
DOC_PARAM(scopes, An array of scope handles which are to represent the various dependencies that the returned scope will have.)
DOC_PARAM(count, The number of scopes in the scopes array)
DOC_RETURN(Always returns the handle to a scope with the appropriate dependencies.)
DOC(Scopes with multiple dependencies are a subtle topic, for a full treatment please read
https://4coder.handmade.network/blogs/p/3412-new_features_p3__memory_management_scopes)
*/
{
    Models *models = (Models*)app->cmd_context;
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    Arena *scratch = &models->mem.arena;
    
    Temp_Memory temp = begin_temp(scratch);
    
    // TODO(allen): revisit this
    struct Node_Ptr{
        Node_Ptr *next;
        Lifetime_Object *object_ptr;
    };
    
    Node_Ptr *first = 0;
    Node_Ptr *last = 0;
    i32 member_count = 0;
    
    b32 filled_array = true;
    for (i32 i = 0; i < count; i += 1){
        Dynamic_Workspace *workspace = get_dynamic_workspace(models, scopes[i]);
        if (workspace == 0){
            filled_array = false;
            break;
        }
        
        switch (workspace->user_type){
            case DynamicWorkspace_Global:
            {
                // NOTE(allen): (global_scope INTERSECT X) == X for all X, therefore we emit nothing when a global group is in the key list.
            }break;
            
            case DynamicWorkspace_Unassociated:
            case DynamicWorkspace_Buffer:
            case DynamicWorkspace_View:
            {
                Lifetime_Object *object = get_lifetime_object_from_workspace(workspace);
                Assert(object != 0);
                Node_Ptr *new_node = push_array(scratch, Node_Ptr, 1);
                sll_queue_push(first, last, new_node);
                new_node->object_ptr = object;
                member_count += 1;
            }break;
            
            case DynamicWorkspace_Intersected:
            {
                Lifetime_Key *key = (Lifetime_Key*)workspace->user_back_ptr;
                if (lifetime_key_check(lifetime_allocator, key)){
                    i32 key_member_count = key->count;
                    Lifetime_Object **key_member_ptr = key->members;
                    for (i32 j = 0; j < key_member_count; j += 1, key_member_ptr += 1){
                        Node_Ptr *new_node = push_array(scratch, Node_Ptr, 1);
                        sll_queue_push(first, last, new_node);
                        new_node->object_ptr = *key_member_ptr;
                        member_count += 1;
                    }
                }
            }break;
            
            default:
            {
                InvalidPath;
            }break;
        }
    }
    
    Managed_Scope result = 0;
    if (filled_array){
        Lifetime_Object **object_ptr_array = push_array(scratch, Lifetime_Object*, member_count);
        i32 index = 0;
        for (Node_Ptr *node = first;
             node != 0;
             node = node->next){
            object_ptr_array[index] = node->object_ptr;
            index += 1;
        }
        member_count = lifetime_sort_and_dedup_object_set(object_ptr_array, member_count);
        Heap *heap = &models->mem.heap;
        Lifetime_Key *key = lifetime_get_or_create_intersection_key(heap, lifetime_allocator, object_ptr_array, member_count);
        result = (Managed_Scope)key->dynamic_workspace.scope_id;
    }
    
    end_temp(temp);
    
    return(result);
}

API_EXPORT b32
Managed_Scope_Clear_Contents(Application_Links *app, Managed_Scope scope)
/*
DOC_PARAM(scope, A handle to the scope which is to be cleared.)
DOC_RETURN(Returns non-zero on succes, and zero on failure.  This call fails when the scope handle does not refer to a valid scope.)
DOC(Clearing the contents of a scope resets all managed variables to have their default values, and frees all managed objects.  The scope handle remains valid and refers to the same scope which still has the same dependencies.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0){
        dynamic_workspace_clear_contents(&models->mem.heap, workspace);
        result = true;
    }
    return(result);
}

API_EXPORT b32
Managed_Scope_Clear_Self_All_Dependent_Scopes(Application_Links *app, Managed_Scope scope)
/*
DOC_PARAM(scope, A handle to the 'basic scope' which is to be cleared.)
DOC_RETURN(Returns non-zero on succes, and zero on failure.  This call fails when the scope handle does not refer to a valid 'basic scope'.)
DOC(The parameter scope must be a 'basic scope' which means a scope with a single dependency.  Scopes tied to buffers and view, and 'user managed scopes' are the 'basic scopes'.  This call treats the scope as an identifier for the single object up which the scope depends, and clears all scopes that depend on that object, this includes clearing 'scope' itself.  The scopes returned from get_managed_scope_with_multiple_dependencies that included 'scope' via the union rule are the only other scopes that can be cleared by this call.
For more on the union rule read the full treatment of managed scopes
https://4coder.handmade.network/blogs/p/3412-new_features_p3__memory_management_scopes)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0 && workspace->user_type != DynamicWorkspace_Global && workspace->user_type != DynamicWorkspace_Intersected){
        Lifetime_Object *object = get_lifetime_object_from_workspace(workspace);
        Assert(object != 0);
        lifetime_object_reset(&models->mem.heap, &models->lifetime_allocator, object);
        result = true;
    }
    return(result);
}

API_EXPORT Managed_Variable_ID
Managed_Variable_Create(Application_Links *app, char *null_terminated_name, u64 default_value)
/*
DOC_PARAM(null_terminated_name, The unique name for this managed variable.)
DOC_PARAM(default_value, The default value that this variable will have in a scope that has not set this variable.)
DOC_RETURN(Returns the Managed_Variable_ID for the new variable on success and zero on failure.  This call fails when a variable with the given name alraedy exists.)
DOC(Variables are stored in scopes but creating a variable does not mean that all scopes will allocate space for this variable.  Space for variables is allocated sparsly on demand in each scope.  Once a variable exists it will exist for the entire duration of the 4coder session and will never have a different id.  The id will be used to set and get the value of the variable in managed scopes.)
DOC_SEE(managed_variable_get_id)
DOC_SEE(managed_variable_create_or_get_id)
*/
{
    Models *models = (Models*)app->cmd_context;
    Heap *heap = &models->mem.heap;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    return(dynamic_variables_create(heap, layout, SCu8(null_terminated_name), default_value));
}

API_EXPORT Managed_Variable_ID
Managed_Variable_Get_ID(Application_Links *app, char *null_terminated_name)
/*
DOC_PARAM(null_terminated_name, The unique name for this managed variable.)
DOC_RETURN(Returns the Managed_Variable_ID for the variable on success and zero on failure.  This call fails when no variable that already exists has the given name.)
DOC_SEE(managed_variable_create)
DOC_SEE(managed_variable_create_or_get_id)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    return(dynamic_variables_lookup(layout, SCu8(null_terminated_name)));
}

API_EXPORT Managed_Variable_ID
Managed_Variable_Create_Or_Get_ID(Application_Links *app, char *null_terminated_name, u64 default_value)
/*
DOC_PARAM(null_terminated_name, The unique name for this managed variable.)
DOC_PARAM(default_value, The default value that this variable will have in a scope that has not set this variable.  This parameter is ignored if the variable already exists.)
DOC_RETURN(Returns the Managed_Variable_ID for the variable on success, this call never fails.)
DOC(This call first tries to get the variable id in the same way that managed_variable_get_id would, if this fails it creates the variable and returns the new id in the way that managed_variable_create would. )
DOC_SEE(managed_variable_create)
DOC_SEE(managed_variable_get_id)
*/
{
    Models *models = (Models*)app->cmd_context;
    Heap *heap = &models->mem.heap;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    return(dynamic_variables_lookup_or_create(heap, layout, SCu8(null_terminated_name), default_value));
}

internal b32
get_dynamic_variable__internal(Models *models, Managed_Scope handle, i32 location, u64 **ptr_out){
    Heap *heap = &models->mem.heap;
    Dynamic_Variable_Layout *layout = &models->variable_layout;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, handle);
    b32 result = false;
    if (workspace != 0){
        if (dynamic_variables_get_ptr(heap, &workspace->mem_bank, layout, &workspace->var_block, location, ptr_out)){
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Managed_Variable_Set(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 value)
/*
DOC_PARAM(scope, A handle to the scope in which the value of the given variable will be set.)
DOC_PARAM(id, The id of the variable to set.)
DOC_PARAM(value, The new value of the variable.)
DOC_RETURN(Returns non-zero on success.  This call fails if scope does not refer to a valid managed scope, or id does not refer to an existing managed variable.)
*/
{
    Models *models = (Models*)app->cmd_context;
    u64 *ptr = 0;
    b32 result = false;
    if (get_dynamic_variable__internal(models, scope, id, &ptr)){
        *ptr = value;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Managed_Variable_Get(Application_Links *app, Managed_Scope scope, Managed_Variable_ID id, u64 *value_out)
/*
DOC_PARAM(scope, A handle to the scope from which the value of the given variable will be queried.)
DOC_PARAM(id, The id of the variable to get.)
DOC_PARAM(value_out, An address where the value of the given variable in the given scope will be stored.)
DOC_RETURN(Returns non-zero on success.  This call fails if scope does not refer to a valid managed scope, or id does not refer to an existing managed variable.  If the managed scope and managed variable both exist, but the variable has never been set, then the default value for the variable that was determined when the variable was created is used to fill value_out, this is treated as a success and returns non-zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    u64 *ptr = 0;
    b32 result = false;
    if (get_dynamic_variable__internal(models, scope, id, &ptr)){
        *value_out = *ptr;
        result = true;
    }
    return(result);
}

API_EXPORT Managed_Object
Alloc_Managed_Memory_In_Scope(Application_Links *app, Managed_Scope scope, i32 item_size, i32 count)
/*
DOC_PARAM(scope, A handle to the scope in which the new object will be allocated.)
DOC_PARAM(item_size, The size, in bytes, of a single 'item' in this memory object.  This effects the size of the allocation, and the indexing of the memory in the store and load calls.)
DOC_PARAM(count, The number of 'items' allocated for this memory object.  The total memory size is item_size*count.)
DOC_RETURN(Returns the handle to the new object on success, and zero on failure.  This call fails if scope does not refer to a valid managed scope.)
DOC(Managed objects allocate memory that is tied to the scope.  When the scope is cleared or destroyed all of the memory allocated in it is freed in bulk and the handles to the objects never again become valid.  Thus the handle returned by this call will only ever refer to this memory allocation.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    Managed_Object result = 0;
    if (workspace != 0){
        result = managed_object_alloc_managed_memory(&models->mem.heap, workspace, item_size, count, 0);
    }
    return(result);
}

API_EXPORT Managed_Object
Alloc_Buffer_Markers_On_Buffer(Application_Links *app, Buffer_ID buffer_id, i32 count, Managed_Scope *optional_extra_scope)
/*
DOC_PARAM(buffer_id, The id for the buffer onto which these markers will be attached.  The markers will live in the scope of the buffer, or in another scope dependent on this buffer, thus guaranteeing that when the buffer is closed, all attached markers are freed in bulk with it.)
DOC_PARAM(count, The number of Marker items allocated in this object.  The total memory size is sizeof(Marker)*count.)
DOC_PARAM(optional_extra_scope, If this pointer is non-null, then it is treated as a scope with additional dependencies for the allocated markers.  In this case, the scope of buffer and extra scope are unioned via get_managed_scope_with_multiple_dependencies and marker object lives in the resulting scope.)
DOC_RETURN(Returns the handle to the new object on succes, and zero on failure.  This call fails if buffer_id does not refer to a valid buffer, or optional_extra_scope does not refer to a valid scope.)
DOC(The created managed object is essentially a memory object with item size equal to sizeof(Marker).  The primary difference is that if the buffer referred to by buffer_id is edited, the position of all markers attached to that buffer can be changed by the core.  Thus this not a memory storage so much as position marking and tracking in a buffer.)
DOC_SEE(alloc_managed_memory_in_scope)
DOC_SEE(Marker)
*/
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Managed_Scope markers_scope = file_get_managed_scope(file);
    if (optional_extra_scope != 0){
        Managed_Object scope_array[2];
        scope_array[0] = markers_scope;
        scope_array[1] = *optional_extra_scope;
        markers_scope = Get_Managed_Scope_With_Multiple_Dependencies(app, scope_array, 2);
    }
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, markers_scope);
    Managed_Object result = 0;
    if (workspace != 0){
        result = managed_object_alloc_buffer_markers(&models->mem.heap, workspace, buffer_id, count, 0);
    }
    return(result);
}

API_EXPORT Managed_Object
Alloc_Managed_Arena_In_Scope(Application_Links *app, Managed_Scope scope, i32 page_size){
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    Managed_Object result = 0;
    if (workspace != 0){
        result = managed_object_alloc_managed_arena_in_scope(&models->mem.heap, workspace, app, page_size, 0);
    }
    return(result);
}

internal Managed_Object_Ptr_And_Workspace
get_dynamic_object_ptrs(Models *models, Managed_Object object){
    Managed_Object_Ptr_And_Workspace result = {};
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    if (workspace != 0){
        u32 lo_id = object&max_u32;
        Managed_Object_Standard_Header *header = (Managed_Object_Standard_Header*)dynamic_workspace_get_pointer(workspace, lo_id);
        if (header != 0){
            result.workspace = workspace;
            result.header = header;
        }
    }
    return(result);
}

internal Marker_Visual_Data*
get_marker_visual_pointer(Models *models, Marker_Visual visual){
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, visual.scope);
    Marker_Visual_Data *result = 0;
    if (workspace != 0){
        result = dynamic_workspace_get_visual_pointer(workspace, visual.slot_id, visual.gen_id);
    }
    return(result);
}

API_EXPORT Marker_Visual
Create_Marker_Visual(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, A handle to the marker object on which the new visual will be attached.)
DOC_RETURN(Returns the handle to the newly created marker visual on success, and zero on failure.  This call fails when object does not refer to a valid marker object.)
DOC(A marker visual adds graphical effects to markers such as cursors, highlight ranges, text colors, etc.  A marker object can have any number of attached visuals.  The memory in the 4coder core for visuals is stored in the same scope as the object.)
DOC_SEE(destroy_marker_visuals)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    Marker_Visual visual = {};
    if (object_ptrs.header != 0 && object_ptrs.header->type == ManagedObjectType_Markers){
        Heap *heap = &models->mem.heap;
        Dynamic_Workspace *workspace = object_ptrs.workspace;
        Marker_Visual_Data *data = dynamic_workspace_alloc_visual(heap, &workspace->mem_bank, workspace);
        
        Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)object_ptrs.header;
        zdll_push_back(markers->visual_first, markers->visual_last, data);
        markers->visual_count += 1;
        data->owner_object = object;
        marker_visual_defaults(data);
        
        visual.scope = workspace->scope_id;
        visual.slot_id = data->slot_id;
        visual.gen_id = data->gen_id;
    }
    return(visual);
}

API_EXPORT b32
Marker_Visual_Set_Effect(Application_Links *app, Marker_Visual visual, Marker_Visual_Type type, int_color color, int_color text_color, Marker_Visual_Text_Style text_style)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(type, The new type of visual effect this marker visual will create.)
DOC_PARAM(color, The new color aspect of the effect, exact meaning depends on the type.)
DOC_PARAM(text_color, The new text color aspect of the effect, exact meaning depends on the type.)
DOC_PARAM(text_style, This feature is not yet implemented and the parameter should always be 0.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
DOC(Each effect type uses the color and text_color aspects differently.  These aspects can be specified as 32-bit colors, or as "symbolic coloes" which are special values small enough that their alpha channels would be zero as 32-bit color codes.  Valid symbolic color values have special rules for evaluation, and sometimes their meaning depends on the effect type too.)
DOC_SEE(Marker_Visuals_Type)
DOC_SEE(Marker_Visuals_Symbolic_Color)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    b32 result = false;
    if (data != 0){
        data->type = type;
        data->color = color;
        data->text_color = text_color;
        data->text_style = text_style;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Marker_Visual_Set_Take_Rule(Application_Links *app, Marker_Visual visual, Marker_Visual_Take_Rule take_rule)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(take_rule, The new take rule for the marker visual.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
DOC(Marker visuals have take rules so that they do not necessarily effect every marker in the marker object they were created to visualize.  The take rule can effect the start of the run of markers, the total number of markers, and the stride of the markers, "taken" by this visual when applying it's effect.  The word "take" should not be thought of as reserving a marker to the particular marker visual, multiple visuals may add effects to a single marker.  See the documentation for Marker_Visual_Take_Rule for specifics about how the take rule can be configured.)
DOC_SEE(Marker_Visual_Take_Rule)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    b32 result = false;
    if (data != 0){
        Assert(take_rule.take_count_per_step != 0);
        take_rule.first_index = clamp_bot(0, take_rule.first_index);
        take_rule.take_count_per_step = clamp_bot(1, take_rule.take_count_per_step);
        take_rule.step_stride_in_marker_count = clamp_bot(take_rule.take_count_per_step, take_rule.step_stride_in_marker_count);
        data->take_rule = take_rule;
        if (data->take_rule.maximum_number_of_markers != 0){
            i32 whole_steps = take_rule.maximum_number_of_markers/take_rule.take_count_per_step;
            i32 extra_in_last = take_rule.maximum_number_of_markers%take_rule.take_count_per_step;
            whole_steps = whole_steps + (extra_in_last > 0?1:0);
            data->one_past_last_take_index = take_rule.first_index + whole_steps*take_rule.step_stride_in_marker_count + extra_in_last;
        }
        else{
            data->one_past_last_take_index = max_i32;
        }
        result = true;
    }
    return(result);
}

API_EXPORT b32
Marker_Visual_Set_Priority(Application_Links *app, Marker_Visual visual, Marker_Visual_Priority_Level priority)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(priority, The new priority level for this marker visual.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
DOC(Multiple visuals effecting the same position, whether they are on the same marker object, or different marker objects, are sorted by their priority level, so that higher priority levels are displayed when they are in conflict with lower priority levels.  Some effects have implicit priorities over other effects which does not take priority level into account, other effects may occur in the same position without being considered "in conflict".  See the documentation for each effect for more information these relationships.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    b32 result = false;
    if (data != 0){
        data->priority = priority;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Marker_Visual_Set_View_Key(Application_Links *app, Marker_Visual visual, View_ID key_view_id)
/*
DOC_PARAM(visual, A handle to the marker visual to be modified by this call.)
DOC_PARAM(key_view_id, The new value of the marker visual's view keying.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does notrefer to a valid marker visual.)
DOC(View keying allows a marker visual to declare that it only appears in one view.  For instance, if a buffer is opened in two views side-by-side, and each view has it's own cursor position, this can be used to make sure that the cursor for one view does not appear in the other view.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Marker_Visual_Data *data = get_marker_visual_pointer(models, visual);
    b32 result = false;
    if (data != 0){
        data->key_view_id = key_view_id;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Destroy_Marker_Visual(Application_Links *app, Marker_Visual visual)
/*
DOC_PARAM(visual, A handle to the marker visual to be destroyed.)
DOC_RETURN(Returns non-zero on success, and zero on failure.  This call fails when the visual handle does not refer to a valid marker visual.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, visual.scope);
    b32 result = false;
    if (workspace != 0){
        Marker_Visual_Data *data = dynamic_workspace_get_visual_pointer(workspace, visual.slot_id, visual.gen_id);
        if (data != 0){
            void *ptr = dynamic_workspace_get_pointer(workspace, data->owner_object&max_u32);
            Managed_Object_Standard_Header *header = (Managed_Object_Standard_Header*)ptr;
            if (header != 0){
                Assert(header->type == ManagedObjectType_Markers);
                Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)header;
                zdll_remove(markers->visual_first, markers->visual_last, data);
                markers->visual_count -= 1;
                marker_visual_free(&workspace->visual_allocator, data);
                result = true;
            }
        }
    }
    return(result);
}

API_EXPORT i32
Buffer_Markers_Get_Attached_Visual_Count(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the marker object to be queried.)
DOC_RETURN(Returns the number of marker visuals that are currently attached to the given object.  If the object handle does not refer to a valid marker object, then this call returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    i32 result = 0;
    if (object_ptrs.header != 0 && object_ptrs.header->type == ManagedObjectType_Markers){
        Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)object_ptrs.header;
        result = markers->visual_count;
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Marker_Visual*
Buffer_Markers_Get_Attached_Visual(Application_Links *app, Arena *arena, Managed_Object object)
/*
DOC_PARAM(part, The arena to be used to allocate the returned array.)
DOC_PARAM(object, The handle to the marker object to be queried.)
DOC_RETURN(Pushes an array onto part containing the handle to every marker visual attached to this object, and returns the pointer to it's base.  If the object does not refer to a valid marker object or there is not enough space in part to allocate the array, then a null pointer is returned.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0 && object_ptrs.header->type == ManagedObjectType_Markers){
        Managed_Buffer_Markers_Header *markers = (Managed_Buffer_Markers_Header*)object_ptrs.header;
        i32 count = markers->visual_count;
        Marker_Visual *visual = push_array(arena, Marker_Visual, count);
        if (visual != 0){
            Marker_Visual *v = visual;
            Managed_Scope scope = object_ptrs.workspace->scope_id;
            for (Marker_Visual_Data *data = markers->visual_first;
                 data != 0;
                 data = data->next, v += 1){
                v->scope = scope;
                v->slot_id = data->slot_id;
                v->gen_id = data->gen_id;
            }
            Assert(v == visual + count);
        }
        return(visual);
    }
    return(0);
}

API_EXPORT u32
Managed_Object_Get_Item_Size(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns the size, in bytes, of a single item in the managed object.  Item size is multiplied by the indices in store and load calls, and it is multiplied by item count to discover the total memory size of the managed object.  If object does not refer to a valid managed object, this call returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u32 result = 0;
    if (object_ptrs.header != 0){
        result = object_ptrs.header->item_size;
    }
    return(result);
}

API_EXPORT u32
Managed_Object_Get_Item_Count(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns the count of items this object can store, this count is used to range check the indices in store and load calls.  If object does not refer to a valid managed object, this call returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u32 result = 0;
    if (object_ptrs.header != 0){
        result = object_ptrs.header->count;
    }
    return(result);
}

API_EXPORT Managed_Object_Type
Managed_Object_Get_Type(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns the type of the managed object, see Managed_Object_Type for the enumeration of possible values and their special meanings.  If object does not refer to a valid managed object, this call returns ManagedObjectType_Error.)
DOC_SEE(Managed_Object_Type)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    if (object_ptrs.header != 0){
        Managed_Object_Type type = object_ptrs.header->type;
        if (type < 0 || ManagedObjectType_COUNT <= type){
            type = ManagedObjectType_Error;
        }
        return(type);
    }
    return(ManagedObjectType_Error);
}

API_EXPORT Managed_Scope
Managed_Object_Get_Containing_Scope(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be queried.)
DOC_RETURN(Returns a handle to the managed scope in which this object is allocated, or zero if object does not refer to a valid managed object.)
*/
{
    Models *models = (Models*)app->cmd_context;
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    if (workspace != 0){
        return((Managed_Scope)hi_id);
    }
    return(0);
}

API_EXPORT b32
Managed_Object_Free(Application_Links *app, Managed_Object object)
/*
DOC_PARAM(object, The handle to the managed object to be freed.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when object does not refer to a valid managed object.)
DOC(Permanently frees the specified object.  Not only does this free up the memory this object had allocated, but it also triggers cleanup for some types of managed objects.  For instance after markers are freed, any visual effects from the markers are removed as well.  See Managed_Object_Type for more information about what cleanup each type performs when it is freed.)
 */
{
    Models *models = (Models*)app->cmd_context;
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    b32 result = false;
    if (workspace != 0){
        result = managed_object_free(workspace, object);
    }
    return(result);
}

API_EXPORT b32
Managed_Object_Store_Data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem)
/*
DOC_PARAM(object, The handle to the managed object in which data will be stored.)
DOC_PARAM(first_index, The first index of the range in the managed object to be stored.  Managed object indics are zero based.)
DOC_PARAM(count, The number of items in the managed object to be stored.)
DOC_PARAM(mem, A pointer to the data to be stored, it is expected that the size of this memory is item_size*count, item_size can be queried with managed_object_get_item_size.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when object does not refer to a valid managed object, and when the range of indices overflows the range of this managed object.  The range of the managed object can be queried with managed_object_get_item_count.)
DOC(All managed objects, in addition to whatever special behaviors they have, have the ability to store and load data.  This storage can have special properties in certain managed object types, for instance, the data stored in marker objects are edited by the core when the buffer to which they are attached is edited.  This call stores the data pointed to by mem into the item range specified by first_index and count.)
DOC_SEE(managed_object_get_item_size)
DOC_SEE(managed_object_get_item_count)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u8 *ptr = get_dynamic_object_memory_ptr(object_ptrs.header);
    b32 result = false;
    if (ptr != 0){
        u32 item_count = object_ptrs.header->count;
        if (0 <= first_index && first_index + count <= item_count){
            u32 item_size = object_ptrs.header->item_size;
            memcpy(ptr + first_index*item_size, mem, count*item_size);
            heap_assert_good(&object_ptrs.workspace->mem_bank.heap);
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Managed_Object_Load_Data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem_out)
/*
DOC_PARAM(object, The handle to the managed object  from which data will be loaded.)
DOC_PARAM(first_index, The first index of the range in the managed object to be loaded.  Managed object indics are zero based.)
DOC_PARAM(count, The number of items in the managed object to be loaded.)
DOC_PARAM(mem_out, A pointer to the memory where loaded data will be written, it is expected that the size of this memory is item_size*count, item_size can be queried with managed_object_get_item_size.)
DOC_RETURN(Returns non-zero on success and zero on failure.  This call fails when object does not refer to a valid managed object, and when the range of indices overflows the range of this managed object.  The range of the managed object can be queried with managed_object_get_item_count.)
DOC(All managed objects, in addition to whatever special behaviors they have, have the ability to store and load data.  This storage can have special properties in certain managed object types, for instance, the data stored in marker objects are edited by the core when the buffer to which they are attached is edited.  This call loads the data from the item range specified by first_index and count into mem_out.)
DOC_SEE(managed_object_get_item_size)
DOC_SEE(managed_object_get_item_count)
*/
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u8 *ptr = get_dynamic_object_memory_ptr(object_ptrs.header);
    b32 result = false;
    if (ptr != 0){
        u32 item_count = object_ptrs.header->count;
        if (0 <= first_index && first_index + count <= item_count){
            u32 item_size = object_ptrs.header->item_size;
            memcpy(mem_out, ptr + first_index*item_size, count*item_size);
            heap_assert_good(&object_ptrs.workspace->mem_bank.heap);
            result = true;
        }
    }
    return(result);
}

API_EXPORT User_Input
Get_User_Input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
/*
DOC_PARAM(get_type, The get_type parameter specifies the set of input types that should be returned.)
DOC_PARAM(abort_type, The get_type parameter specifies the set of input types that should trigger an abort signal.)
DOC_RETURN(This call returns a User_Input that describes a user input event.)
DOC(
This call preempts the command. The command is resumed if either a get or abort condition
is met, or if another command is executed.  If either the abort condition is met or another
command is executed an abort signal is returned.  If an abort signal is ever returned the
command should finish execution without any more calls that preempt the command.
If a get condition is met the user input is returned.
)
DOC_SEE(Input_Type_Flag)
DOC_SEE(User_Input)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    User_Input result = {};
    if (app->type_coroutine == Co_Command){
        Coroutine_Head *coroutine = (Coroutine_Head*)app->current_coroutine;
        Assert(coroutine != 0);
        ((u32*)coroutine->out)[0] = get_type;
        ((u32*)coroutine->out)[1] = abort_type;
        system->yield_coroutine(coroutine);
        result = *(User_Input*)coroutine->in;
    }
    return(result);
}

API_EXPORT User_Input
Get_Command_Input(Application_Links *app)
/*
DOC_RETURN(This call returns the input that triggered the currently executing command.)
DOC_SEE(User_Input)
*/{
    Models *models = (Models*)app->cmd_context;
    User_Input result = {};
    result.key = models->key;
    return(result);
}

API_EXPORT void
Set_Command_Input(Application_Links *app, Key_Event_Data key_data)
/*
DOC_PARAM(key_data, The new value of the "command input". Setting this effects the result returned by get_command_input until the end of this command.)
*/{
    Models *models = (Models*)app->cmd_context;
    models->key = key_data;
}

API_EXPORT Mouse_State
Get_Mouse_State(Application_Links *app)
/*
DOC_RETURN(This call returns the current mouse state as of the beginning of the frame.)
DOC_SEE(Mouse_State)
*/{
    Models *models = (Models*)app->cmd_context;
    return(models->input->mouse);
}

API_EXPORT b32
Get_Active_Query_Bars(Application_Links *app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array *array_out)
/*
DOC_PARAM(view_id, Specifies the view for which query bars should be retrieved.)
DOC_PARAM(max_result_count, Specifies the number of Query_Bar pointers available in result_array.)
DOC_PARAM(result_array, User-supplied empty array of max_result_count Query_Bar pointers.)
DOC_RETURN(This call returns the number of Query_Bar pointers successfully placed in result_array.)
DOC(This call allows the customization layer to inspect the set of active Query_Bar slots for a given
view_id.  By convention, the most recent query will be entry 0, the next most recent 1, etc., such
that if you only care about the most recent query bar, you can call Get_Active_Query_Bars with a
max_result_count of 1 and be assured you will get the most recent bar if any exist.)
*/{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (view != 0){
        i32 count = 0;
        Query_Bar **ptrs = array_out->ptrs;
        for (Query_Slot *slot = view->query_set.used_slot;
             slot != 0 && (count < max_result_count);
             slot = slot->next){
            if (slot->query_bar != 0){
                ptrs[count++] = slot->query_bar;
            }
        }
        array_out->count = count;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Start_Query_Bar(Application_Links *app, Query_Bar *bar, u32 flags)
/*
DOC_PARAM(bar, This parameter provides a Query_Bar that should remain in valid memory
until end_query_bar or the end of the command.  It is commonly a good idea to make
this a pointer to a Query_Bar stored on the stack.)
DOC_PARAM(flags, This parameter is not currently used and should be 0 for now.)
DOC_RETURN(This call returns non-zero on success.)
DOC(This call tells the active view to begin displaying a "Query_Bar" which is a small
GUI element that can overlap a buffer or other 4coder GUI.  The contents of the bar
can be changed after the call to start_query_bar and the query bar shown by 4coder
will reflect the change.  Since the bar stops showing when the command exits the
only use for this call is in an interactive command that makes calls to get_user_input.)
*/{
    Models *models = (Models*)app->cmd_context;
    Panel *active_panel = layout_get_active_panel(&models->layout);
    View *active_view = active_panel->view;
    Query_Slot *slot = alloc_query_slot(&active_view->query_set);
    b32 result = (slot != 0);
    if (result){
        slot->query_bar = bar;
    }
    return(result);
}

API_EXPORT void
End_Query_Bar(Application_Links *app, Query_Bar *bar, u32 flags)
/*
DOC_PARAM(bar, This parameter should be a bar pointer of a currently active query bar.)
DOC_PARAM(flags, This parameter is not currently used and should be 0 for now.)
DOC(Stops showing the particular query bar specified by the bar parameter.)
*/{
    Models *models = (Models*)app->cmd_context;
    Panel *active_panel = layout_get_active_panel(&models->layout);
    View *active_view = active_panel->view;
    free_query_slot(&active_view->query_set, bar);
}

API_EXPORT b32
Print_Message(Application_Links *app, String_Const_u8 message)
/*
DOC_PARAM(str, The str parameter specifies the string to post to *messages*; it need not be null terminated.)
DOC_PARAM(len, The len parameter specifies the length of the str string.)
DOC(This call posts a string to the *messages* buffer.)
*/{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = models->message_buffer;
    b32 result = false;
    if (file != 0){
        output_file_append(models, file, message);
        file_cursor_to_end(models->system, models, file);
        result = true;
    }
    return(result);
}

API_EXPORT Face_ID
Get_Largest_Face_ID(Application_Links *app)
/*
DOC_RETURN(Returns the largest face ID that could be valid.  There is no guarantee that the returned value is a valid face, or that every face less than the returned value is valid.  The guarantee is that all valid face ids are in the range between 1 and the return value.)
*/
{
    Models *models = (Models*)app->cmd_context;
    return(font_set_get_largest_id(&models->font_set));
}

API_EXPORT b32
Set_Global_Face(Application_Links *app, Face_ID id, b32 apply_to_all_buffers)
/*
DOC_PARAM(id, The id of the face to try to make the global face.)
DOC_PARAM(apply_to_all_buffers, If the face is valid, apply the face to change to all open buffers as well as setting the global default.)
DOC(Tries to set the global default face, which new buffers will use upon creation.)
DOC_RETURN(Returns true if the given id was a valid face and the change was made successfully.)
*/
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    
    b32 did_change = false;
    Face *face = font_set_face_from_id(&models->font_set, id);
    if (face != 0){
        if (apply_to_all_buffers){
            global_set_font_and_update_files(system, models, id);
        }
        else{
            models->global_font_id = id;
        }
        did_change = true;
    }
    
    return(did_change);
}

API_EXPORT History_Record_Index
Buffer_History_Get_Max_Record_Index(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    History_Record_Index result = 0;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        result = history_get_record_count(&file->state.history);
    }
    return(result);
}

internal void
buffer_history__fill_record_info(Record *record, Record_Info *out){
    out->kind = record->kind;
    out->edit_number = record->edit_number;
    switch (out->kind){
        case RecordKind_Single:
        {
            out->single.string_forward  = SCu8(record->single.str_forward , record->single.length_forward );
            out->single.string_backward = SCu8(record->single.str_backward, record->single.length_backward);
            out->single.first = record->single.first;
        }break;
        case RecordKind_Group:
        {
            out->group.count = record->group.count;
        }break;
        default:
        {
            InvalidPath;
        }break;
    }
}

API_EXPORT Record_Info
Buffer_History_Get_Record_Info(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Record_Info result = {};
    if (api_check_buffer(file)){
        History *history = &file->state.history;
        if (history_is_activated(history)){
            i32 max_index = history_get_record_count(history);
            if (0 <= index && index <= max_index){
                if (0 < index){
                    Record *record = history_get_record(history, index);
                    buffer_history__fill_record_info(record, &result);
                }
                else{
                    result.error = RecordError_InitialStateDummyRecord;
                }
            }
            else{
                result.error = RecordError_IndexOutOfBounds;
            }
        }
        else{
            result.error = RecordError_NoHistoryAttached;
        }
    }
    else{
        result.error = RecordError_InvalidBuffer;
    }
    return(result);
}

API_EXPORT Record_Info
Buffer_History_Get_Group_Sub_Record(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Record_Info result = {};
    if (api_check_buffer(file)){
        History *history = &file->state.history;
        if (history_is_activated(history)){
            i32 max_index = history_get_record_count(history);
            if (0 <= index && index <= max_index){
                if (0 < index){
                    Record *record = history_get_record(history, index);
                    if (record->kind == RecordKind_Group){
                        record = history_get_sub_record(record, sub_index + 1);
                        if (record != 0){
                            buffer_history__fill_record_info(record, &result);
                        }
                        else{
                            result.error = RecordError_SubIndexOutOfBounds;
                        }
                    }
                    else{
                        result.error = RecordError_WrongRecordTypeAtIndex;
                    }
                }
                else{
                    result.error = RecordError_InitialStateDummyRecord;
                }
            }
            else{
                result.error = RecordError_IndexOutOfBounds;
            }
        }
        else{
            result.error = RecordError_NoHistoryAttached;
        }
    }
    else{
        result.error = RecordError_InvalidBuffer;
    }
    return(result);
}

API_EXPORT History_Record_Index
Buffer_History_Get_Current_State_Index(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    History_Record_Index result = 0;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        result = file_get_current_record_index(file);
    }
    return(result);
}

API_EXPORT b32
Buffer_History_Set_Current_State_Index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        i32 max_index = history_get_record_count(&file->state.history);
        if (0 <= index && index <= max_index){
            System_Functions *system = models->system;
            edit_change_current_history_state(system, models, file, index);
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Buffer_History_Merge_Record_Range(Application_Links *app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = edit_merge_history_range(models, file, first_index, last_index, flags);
    }
    return(result);
}

API_EXPORT b32
Buffer_History_Clear_After_Current_State(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        history_dump_records_after_index(&file->state.history, file->state.current_record_index);
        result = true;
    }
    return(result);
}

API_EXPORT void
Global_History_Edit_Group_Begin(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, 1);
}

API_EXPORT void
Global_History_Edit_Group_End(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, -1);
}

API_EXPORT b32
Buffer_Set_Face(Application_Links *app, Buffer_ID buffer_id, Face_ID id)
/*
DOC_PARAM(buffer, The buffer on which to change the face.)
DOC_PARAM(id, The id of the face to try to set the buffer to use.)
DOC(Tries to set the buffer's face.)
DOC_RETURN(Returns true if the given id was a valid face and the change was made successfully.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    
    b32 did_change = false;
    if (api_check_buffer(file)){
        if (font_set_face_from_id(&models->font_set, id) != 0){
            did_change = true;
            file_set_font(models->system, models, file, id);
        }
    }
    return(did_change);
}

API_EXPORT Face_Description
Get_Face_Description(Application_Links *app, Face_ID face_id)
/*
DOC_PARAM(id, The face slot from which to read a description.  If zero gets default values.)
DOC(Fills out the values of a Face_Description struct, which includes all the information that determines the appearance of the face.  If the id does not specify a valid face the description will be invalid.  An invalid description has a zero length string in it's font.name field (i.e. description.font.name[0] == 0), and a valid description always contains a non-zero length string in the font.name field (i.e. description.font.name[0] != 0)

If the input id is zero, the description returned will be invalid, but the pt_size and hinting fields will reflect the default values for those fields as specified on the command line.  The default values, if unspecified, are pt_size=0 and hinting=false.  These default values are overriden by config.4coder when instantiating fonts at startup, but the original values of pt_size=0 and hinting=false from the command line are preserved and returned here for the lifetime of the program.

Note that the id of zero is reserved and is never a valid face.)
DOC_RETURN(Returns a Face_Description that is valid if the id references a valid face slot and is filled with the description of the face.  Otherwise returns an invalid Face_Description.)
DOC_SEE(Face_Description)
*/
{
    Models *models = (Models*)app->cmd_context;
    Face_Description description = {};
    if (face_id != 0){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            description = face->description;
        }
    }
    else{
        description.parameters.pt_size = models->settings.font_size;
        description.parameters.hinting = models->settings.use_hinting;
    }
    return(description);
}

API_EXPORT Face_Metrics
Get_Face_Metrics(Application_Links *app, Face_ID face_id){
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Face_Metrics result = {};
    if (face_id != 0){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result.line_height = (f32)face->height;
            result.typical_character_width = face->byte_sub_advances[1];
        }
    }
    return(result);
}

// TODO(allen): redocument
API_EXPORT Face_ID
Get_Face_ID(Application_Links *app, Buffer_ID buffer_id)
/*
DOC_PARAM(buffer, The buffer from which to get a face id.  If NULL gets global face id.)
DOC(Retrieves a face id if buffer is a valid   If buffer is set to NULL, the parameter is ignored and the global default face is returned.)
DOC_RETURN(On success a valid Face_ID, otherwise returns zero.)
*/
{
    Models *models = (Models*)app->cmd_context;
    Face_ID result = 0;
    if (buffer_id != 0){
        Editing_File *file = imp_get_file(models, buffer_id);
        if (api_check_buffer(file)){
            result = file->settings.font_id;
        }
    }
    else{
        result = models->global_font_id;
    }
    return(result);
}

API_EXPORT Face_ID
Try_Create_New_Face(Application_Links *app, Face_Description *description)
/*
DOC_PARAM(description, A description of the new face to try to create.)
DOC(Attempts to create a new face and configure it with the provided description.  This call can fail for a number of reasons:

- If the description's font field is not one of the available fonts no face is created.

- If the specified font cannot actually be loaded by the 4coder font system no face is created.

- If the specified font with the specified configuration is too large or too small no face is created.

Note, not all fonts will support all styles.  The fields for italic, bold, underline, and hinting, are only requests that the system try to apply these configurations, but if any cannot be done the face will still be created but without the unsupported configurations.  4coder does not try to simulate the effects of missing styles.)
DOC_RETURN(Returns a new valid face id if the font system successfully instanatiates the new face, otherwise returns zero.  Note that zero is a reserved id and is never a valid id.)
DOC_SEE(Face_Description)
*/
{
    Models *models = (Models*)app->cmd_context;
    Face_ID result = 0;
    if (is_running_coroutine(app)){
        System_Functions *system = models->system;
        Coroutine_Head *coroutine = (Coroutine_Head*)app->current_coroutine;
        Assert(coroutine != 0);
        ((Face_Description**)coroutine->out)[0] = description;
        ((u32*)coroutine->out)[2] = AppCoroutineRequest_NewFontFace;
        system->yield_coroutine(coroutine);
        result = *(Face_ID*)(coroutine->in);
    }
    else{
        result = font_set_new_face(&models->font_set, description);
    }
    return(result);
}

API_EXPORT b32
Try_Modify_Face(Application_Links *app, Face_ID id, Face_Description *description)
/*
DOC_PARAM(id, The id of the face slot to try to modify.)
DOC_PARAM(description, The new description for the face slot to use.)
DOC(Attempts to modify the face in a particular face slot.  If successful all buffers using the face will continue using the same face slot, and will therefore change appearance to the new configuration of the face slot.

This call can fail for all the same reasons that try_create_new_face can fail, and the same rules about failure to apply specific styles also apply.  If the call does fail, the original configuration of the face slot stays in place.  A valid face slot should never become invalid except by releasing it.

Performance Warning: Modifying a face slot should only be done a couple of times per frame in most cases.  Not only does this call reconfigure a slot, it also recomputes the layout for all buffers that use this face slot.)
DOC_RETURN(Returns true on success and false on failure.)
DOC_SEE(try_create_new_face)
*/
{
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    if (is_running_coroutine(app)){
        System_Functions *system = models->system;
        Coroutine_Head *coroutine = (Coroutine_Head*)app->current_coroutine;
        Assert(coroutine != 0);
        ((Face_Description**)coroutine->out)[0] = description;
        ((u32*)coroutine->out)[2] = AppCoroutineRequest_ModifyFace;
        ((u32*)coroutine->out)[3] = id;
        system->yield_coroutine(coroutine);
        result = *(b32*)(coroutine->in);
    }
    else{
        result = alter_font_and_update_files(models->system, models, id, description);
    }
    return(result);
}

API_EXPORT b32
Try_Release_Face(Application_Links *app, Face_ID id, Face_ID replacement_id)
/*
DOC_PARAM(id, The id of the face slot to release.)
DOC_PARAM(replacement_id, Optional.  Specifies what buffers that were using id should switch to after the release.)
DOC(Attempts to release the slot referred to by id.  If successful all buffers using the face will switch to using a new valid face, and will therefore change appearance to the new face slot.  If replacement_id refers to a valid face slot, it will be used for the new slot, otherwise the slot is chosen arbitrarily out of the remaining valid faces.

This call can fail if the id does not name a valid face slot, or if there is only one face slot left in the system.

Performance Warning: Releasing a face slot should only be done a couple of times per frame in most cases.  Not only does it release all the resources used by the slot, it also recomputes the layout for all buffers that used the released slot.  If no buffers use the slots that are released, it is generally okay to use it more frequently.)
DOC_RETURN(Returns true on success and zero on failure.)
*/
{
    Models *models = (Models*)app->cmd_context;
    b32 success = release_font_and_update_files(models->system, models, id, replacement_id);
    return(success);
}

API_EXPORT void
Set_Theme_Colors(Application_Links *app, Theme_Color *colors, i32 count)
/*
DOC_PARAM(colors, The colors pointer provides an array of color structs pairing differet style tags to color codes.)
DOC_PARAM(count, The count parameter specifies the number of Theme_Color structs in the colors array.)
DOC(For each struct in the array, the slot in the main color pallet specified by the struct's tag is set to the color code in the struct. If the tag value is invalid no change is made to the color pallet.)
DOC_SEE(Theme_Color)
*/{
    Models *models = (Models*)app->cmd_context;
    Color_Table color_table = models->color_table;
    Theme_Color *theme_color = colors;
    for (i32 i = 0; i < count; ++i, ++theme_color){
        if (theme_color->tag < color_table.count){
            color_table.vals[theme_color->tag] = theme_color->color;
        }
    }
}

API_EXPORT void
Get_Theme_Colors(Application_Links *app, Theme_Color *colors, i32 count)
/*
DOC_PARAM(colors, an array of color structs listing style tags to get color values for)
DOC_PARAM(count, the number of color structs in the colors array)
DOC(For each struct in the array, the color field of the struct is filled with the color from the slot in the main color pallet specified by the tag.  If the tag value is invalid the color is filled with black.)
DOC_SEE(Theme_Color)
*/{
    Models *models = (Models*)app->cmd_context;
    Color_Table color_table = models->color_table;
    Theme_Color *theme_color = colors;
    for (i32 i = 0; i < count; ++i, ++theme_color){
        theme_color->color = finalize_color(color_table, theme_color->tag);
    }
}

API_EXPORT argb_color
Finalize_Color(Application_Links *app, int_color color){
    Models *models = (Models*)app->cmd_context;
    Color_Table color_table = models->color_table;
    u32 color_rgb = color;
    if ((color & 0xFF000000) == 0){
        color_rgb = color_table.vals[color % color_table.count];
    }
    return(color_rgb);
}

// TODO(allen): redocument
API_EXPORT String_Const_u8
Push_Hot_Directory(Application_Links *app, Arena *arena)
/*
DOC_PARAM(out, On success this character buffer is filled with the 4coder 'hot directory'.)
DOC_PARAM(capacity, Specifies the capacity in bytes of the of the out buffer.)
DOC(4coder has a concept of a 'hot directory' which is the directory most recently accessed in the GUI.  Whenever the GUI is opened it shows the hot directory. In the future this will be deprecated and eliminated in favor of more flexible hot directories created and controlled in the custom layer.)
DOC_RETURN(This call returns the length of the hot directory string whether or not it was successfully copied into the output buffer.  The call is successful if and only if capacity is greater than or equal to the return value.)
DOC_SEE(directory_set_hot)
*/{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    hot_directory_clean_end(hot);
    String_Const_u8 result = push_string_copy(arena, SCu8(hot->string_space, hot->string_size));
    return(result);
}

// TODO(allen): redocument
API_EXPORT b32
Set_Hot_Directory(Application_Links *app, String_Const_u8 string)
/*
DOC_PARAM(str, The new value of the hot directory.  This does not need to be a null terminated string.)
DOC_PARAM(len, The length of str in bytes.)
DOC_RETURN(Returns non-zero on success.)
DOC(4coder has a concept of a 'hot directory' which is the directory most recently accessed in the GUI.  Whenever the GUI is opened it shows the hot directory. In the future this will be deprecated and eliminated in favor of more flexible directories controlled on the custom side.)
DOC_SEE(directory_get_hot)
*/{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    b32 success = false;
    if (string.size < sizeof(hot->string_space)){
        hot_directory_set(models->system, hot, string);
        success = true;
    }
    return(success);
}

// TODO(allen): redocument
API_EXPORT b32
Get_File_List(Application_Links *app, String_Const_u8 directory, File_List *list_out)
/*
DOC_PARAM(dir, This parameter specifies the directory whose files will be enumerated in the returned list; it need not be null terminated.)
DOC_PARAM(len, This parameter the length of the dir string.)
DOC_RETURN(This call returns a File_List struct containing pointers to the names of the files in the specified directory.  The File_List returned should be passed to free_file_list when it is no longer in use.)
DOC_SEE(File_List)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    block_zero_struct(list_out);
    Editing_File_Name canon = {};
    b32 result = false;
    if (get_canon_name(system, directory, &canon)){
        Arena *scratch = &models->mem.arena;
        Temp_Memory temp = begin_temp(scratch);
        char *str = 0;
        if (canon.name_size < sizeof(canon.name_space)){
            canon.name_space[canon.name_size] = 0;
            str = (char*)canon.name_space;
        }
        else{
            String_Const_u8 s = push_string_copy(scratch, string_from_file_name(&canon));
            str = (char*)s.str;
        }
        system->set_file_list(list_out, str, 0, 0, 0);
        end_temp(temp);
        result = true;
    }
    return(result);
}

API_EXPORT void
Free_File_List(Application_Links *app, File_List list)
/*
DOC_PARAM(list, This parameter provides the file list to be freed.)
DOC(After this call the file list passed in should not be read or written to.)
DOC_SEE(File_List)
*/{
    Models *models = (Models*)app->cmd_context;
    models->system->set_file_list(&list, 0, 0, 0, 0);
}

API_EXPORT void
Set_GUI_Up_Down_Keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier)
/*
DOC_PARAM(up_key, the code of the key that should be interpreted as an up key)
DOC_PARAM(up_key_modifier, the modifier for the key that should be interpreted as an up key)
DOC_PARAM(down_key, the code of the key that should be interpreted as a down key)
DOC_PARAM(down_key_modifier, the modifier for the key that should be interpreted as a down key)

DOC(This is a temporary ad-hoc solution to allow some customization of the behavior of the built in GUI. There is a high chance that it will be removed and not replaced at some point, so it is not recommended that it be heavily used.) */
{
    Models *models = (Models*)app->cmd_context;
    models->user_up_key = up_key;
    models->user_up_key_modifier = up_key_modifier;
    models->user_down_key = down_key;
    models->user_down_key_modifier = down_key_modifier;
}

API_EXPORT void*
Memory_Allocate(Application_Links *app, i32 size)
/*
DOC_PARAM(size, The size in bytes of the block that should be returned.)
DOC(This calls to a low level OS allocator which means it is best used for infrequent, large allocations.  The size of the block must be remembered if it will be freed or if it's mem protection status will be changed.)
DOC_SEE(memory_free)
*/{
    Models *models = (Models*)app->cmd_context;
    void *result = models->system->memory_allocate(size);
    return(result);
}

API_EXPORT b32
Memory_Set_Protection(Application_Links *app, void *ptr, i32 size, Memory_Protect_Flags flags)
/*
DOC_PARAM(ptr, The base of the block on which to set memory protection flags.)
DOC_PARAM(size, The size that was originally used to allocate this block.)
DOC_PARAM(flags, The new memory protection flags.)
DOC(This call sets the memory protection flags of a block of memory that was previously allocate by memory_allocate.)
DOC_SEE(memory_allocate)
DOC_SEE(Memory_Protect_Flags)
*/{
    Models *models = (Models*)app->cmd_context;
    return(models->system->memory_set_protection(ptr, size, flags));
}

API_EXPORT void
Memory_Free(Application_Links *app, void *ptr, i32 size)
/*
DOC_PARAM(mem, The base of a block to free.)
DOC_PARAM(size, The size that was originally used to allocate this block.)
DOC(This call frees a block of memory that was previously allocated by memory_allocate.)
DOC_SEE(memory_allocate)
*/{
    Models *models = (Models*)app->cmd_context;
    models->system->memory_free(ptr, size);
}

// TODO(allen): redocument
API_EXPORT String_Const_u8
Push_4ed_Path(Application_Links *app, Arena *arena)
/*
DOC_PARAM(out, This parameter provides a character buffer that receives the path to the 4ed executable file.)
DOC_PARAM(capacity, This parameter specifies the maximum capacity of the out buffer.)
DOC_RETURN(This call returns non-zero on success.)
*/{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(system->get_4ed_path(arena));
}

// TODO(allen): do(add a "shown but auto-hides on timer" setting for cursor show type)
API_EXPORT void
Show_Mouse_Cursor(Application_Links *app, Mouse_Cursor_Show_Type show)
/*
DOC_PARAM(show, This parameter specifies the new state of the mouse cursor.)
DOC_SEE(Mouse_Cursor_Show_Type)
*/{
    Models *models = (Models*)app->cmd_context;
    models->system->show_mouse_cursor(show);
}

API_EXPORT b32
Set_Edit_Finished_Hook_Repeat_Speed(Application_Links *app, u32 milliseconds){
    Models *models = (Models*)app->cmd_context;
    models->edit_finished_hook_repeat_speed = milliseconds;
    return(true);
}

API_EXPORT b32
Set_Fullscreen(Application_Links *app, b32 full_screen)
/*
DOC_PARAM(full_screen, The new value of the global full_screen setting.)
DOC(This call tells 4coder to set the full_screen mode.  The change to full screen mode does not take effect until the end of the current frame.  But is_fullscreen does report the new state.)
*/{
    Models *models = (Models*)app->cmd_context;
    b32 success = models->system->set_fullscreen(full_screen);
    if (!success){
        print_message(app, string_u8_litexpr("ERROR: Failed to go fullscreen.\n"));
    }
    return(success);
}

API_EXPORT b32
Is_Fullscreen(Application_Links *app)
/*
DOC(This call returns true if the 4coder is in full screen mode.  This call takes toggles that have already occured this frame into account.  So it may return true even though the frame has not ended and actually put 4coder into full screen. If it returns true though, 4coder will definitely be full screen by the beginning of the next frame if the state is not changed.)
*/{
    Models *models = (Models*)app->cmd_context;
    b32 result = models->system->is_fullscreen();
    return(result);
}

API_EXPORT void
Send_Exit_Signal(Application_Links *app)
/*
DOC(This call sends a signal to 4coder to attempt to exit, which will trigger the exit hook before the end of the frame.  That hook will have the chance to cancel the exit.

In the default behavior of the exit hook, the exit is cancelled if there are unsaved changes, and instead a UI querying the user for permission to close without saving is presented, if the user confirms the UI sends another exit signal that will not be canceled.

To make send_exit_signal exit no matter what, setup your hook in such a way that it knows when you are trying to exit no matter what, such as with a global variable that you set before calling send_exit_signal.)
*/{
    Models *models = (Models*)app->cmd_context;
    models->keep_playing = false;
}

// TODO(allen): redocument
API_EXPORT b32
Set_Window_Title(Application_Links *app, String_Const_u8 title)
/*
DOC_PARAM(title, A null terminated string indicating the new title for the 4coder window.)
DOC(Sets 4coder's window title to the specified title string.)
*/{
    Models *models = (Models*)app->cmd_context;
    models->has_new_title = true;
    umem cap_before_null = (umem)(models->title_capacity - 1);
    umem copy_size = clamp_top(title.size, cap_before_null);
    block_copy(models->title_space, title.str, copy_size);
    models->title_space[copy_size] = 0;
    return(true);
}

API_EXPORT Microsecond_Time_Stamp
Get_Microseconds_Timestamp(Application_Links *app)
/*
DOC(Returns a microsecond resolution timestamp.)
*/
{
    // TODO(allen): do(decrease indirection in API calls)
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(system->now_time());
}

internal Vec2
models_get_coordinate_center(Models *models){
    Vec2 result = {};
    if (models->coordinate_center_stack_top > 0){
        result = models->coordinate_center_stack[models->coordinate_center_stack_top - 1];
    }
    return(result);
}

internal Vec2
draw_helper__models_space_to_screen_space(Models *models, Vec2 point){
    Vec2 c = models_get_coordinate_center(models);
    return(point + c);
}

internal f32_Rect
draw_helper__models_space_to_screen_space(Models *models, f32_Rect rect){
    Vec2 c = models_get_coordinate_center(models);
    rect.p0 += c;
    rect.p1 += c;
    return(rect);
}

internal Vec2
draw_helper__view_space_to_screen_space(View *view, Vec2 point){
    i32_Rect region = view->render.view_rect;
    point.x += (f32)region.x0;
    point.y += (f32)region.y0;
    return(point);
}

internal f32_Rect
draw_helper__view_space_to_screen_space(View *view, f32_Rect rect){
    i32_Rect region = view->render.view_rect;
    f32 x_corner = (f32)region.x0;
    f32 y_corner = (f32)region.y0;
    rect.x0 += x_corner;
    rect.y0 += y_corner;
    rect.x1 += x_corner;
    rect.y1 += y_corner;
    return(rect);
}

internal Vec2
draw_helper__screen_space_to_view_space(View *view, Vec2 point){
    i32_Rect region = view->render.view_rect;
    point.x -= (f32)region.x0;
    point.y -= (f32)region.y0;
    return(point);
}

internal f32_Rect
draw_helper__screen_space_to_view_space(View *view, f32_Rect rect){
    i32_Rect region = view->render.view_rect;
    f32 x_corner = (f32)region.x0;
    f32 y_corner = (f32)region.y0;
    rect.x0 -= x_corner;
    rect.y0 -= y_corner;
    rect.x1 -= x_corner;
    rect.y1 -= y_corner;
    return(rect);
}

// NOTE(allen): Coordinate space of draw calls:
// The render space is such that 0,0 is _always_ the top left corner of the renderable region of the view.
// To make text scroll with the buffer users should read the view's scroll position and subtract it first.

API_EXPORT Vec2
Draw_String(Application_Links *app, Face_ID font_id, String_Const_u8 str, Vec2 point, int_color color, u32 flags, Vec2 delta)
{
    Vec2 result = point;
    Models *models = (Models*)app->cmd_context;
    Face *face = font_set_face_from_id(&models->font_set, font_id);
    if (models->target == 0){
        f32 width = font_string_width(models->target, face, str);
        result += delta*width;
    }
    else{
        Color_Table color_table = models->color_table;
        point = draw_helper__models_space_to_screen_space(models, point);
        u32 actual_color = finalize_color(color_table, color);
        f32 width = draw_string(models->target, face, str, point, actual_color, flags, delta);
        result += delta*width;
    }
    return(result);
}

API_EXPORT f32
Get_String_Advance(Application_Links *app, Face_ID font_id, String_Const_u8 str)
{
    Models *models = (Models*)app->cmd_context;
    Face *face = font_set_face_from_id(&models->font_set, font_id);
    return(font_string_width(models->target, face, str));
}

API_EXPORT void
Draw_Rectangle(Application_Links *app, Rect_f32 rect, int_color color){
    Models *models = (Models*)app->cmd_context;
    if (models->in_render_mode){
        Color_Table color_table = models->color_table;
        rect = draw_helper__models_space_to_screen_space(models, rect);
        u32 actual_color = finalize_color(color_table, color);
        draw_rectangle(models->target, rect, actual_color);
    }
}

API_EXPORT void
Draw_Rectangle_Outline(Application_Links *app, Rect_f32 rect, int_color color)
{
    Models *models = (Models*)app->cmd_context;
    if (models->in_render_mode){
        Color_Table color_table = models->color_table;
        rect = draw_helper__models_space_to_screen_space(models, rect);
        u32 actual_color = finalize_color(color_table, color);
        draw_rectangle_outline(models->target, rect, actual_color);
    }
}

API_EXPORT void
Draw_Clip_Push(Application_Links *app, Rect_f32 clip_box){
    Models *models = (Models*)app->cmd_context;
    //clip_box = draw_helper__models_space_to_screen_space(models, clip_box);
    draw_push_clip(models->target, Ri32(clip_box));
}

API_EXPORT f32_Rect
Draw_Clip_Pop(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(Rf32(draw_pop_clip(models->target)));
}

API_EXPORT void
Draw_Coordinate_Center_Push(Application_Links *app, Vec2 point){
    Models *models = (Models*)app->cmd_context;
    if (models->coordinate_center_stack_top < ArrayCount(models->coordinate_center_stack)){
        point = draw_helper__models_space_to_screen_space(models, point);
        models->coordinate_center_stack[models->coordinate_center_stack_top] = point;
        models->coordinate_center_stack_top += 1;
    }
}

API_EXPORT Vec2
Draw_Coordinate_Center_Pop(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    Vec2 result = {};
    if (models->coordinate_center_stack_top > 0){
        models->coordinate_center_stack_top -= 1;
        result = models->coordinate_center_stack[models->coordinate_center_stack_top];
    }
    return(result);
}

API_EXPORT b32
Text_Layout_Get_Buffer(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID *buffer_id_out){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    b32 result = false;
    if (text_layout_get(&models->text_layouts, text_layout_id, &layout)){
        *buffer_id_out = layout.buffer_id;
        result = true;
    }
    return(result);
}

API_EXPORT b32
Text_Layout_Buffer_Point_To_Layout_Point(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 buffer_relative_p, Vec2 *p_out){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    b32 result = false;
    if (text_layout_get(&models->text_layouts, text_layout_id, &layout)){
        Editing_File *file = imp_get_file(models, layout.buffer_id);
        if (api_check_buffer(file)){
            System_Functions *system = models->system;
            // TODO(allen): this could be computed and stored _once_ right?
            Full_Cursor top = file_compute_cursor(models, file, seek_line_char(layout.point.line_number, 1));
            f32 top_y = top.wrapped_y;
            if (file->settings.unwrapped_lines){
                top_y = top.unwrapped_y;
            }
            buffer_relative_p -= layout.point.pixel_shift;
            buffer_relative_p.y -= top_y;
            *p_out = buffer_relative_p;
            result = true;
        }
    }
    return(result);
}

API_EXPORT b32
Text_Layout_Layout_Point_To_Buffer_Point(Application_Links *app, Text_Layout_ID text_layout_id, Vec2 layout_relative_p, Vec2 *p_out){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    b32 result = false;
    if (text_layout_get(&models->text_layouts, text_layout_id, &layout)){
        Editing_File *file = imp_get_file(models, layout.buffer_id);
        if (api_check_buffer(file)){
            System_Functions *system = models->system;
            // TODO(allen): this could be computed and stored _once_ right?
            Full_Cursor top = file_compute_cursor(models, file, seek_line_char(layout.point.line_number, 1));
            f32 top_y = top.wrapped_y;
            if (file->settings.unwrapped_lines){
                top_y = top.unwrapped_y;
            }
            layout_relative_p += layout.point.pixel_shift;
            layout_relative_p.y += top_y;
            *p_out = layout_relative_p;
            result = true;
        }
    }
    return(result);
}

API_EXPORT Range_i64
Text_Layout_Get_On_Screen_Range(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    Range_i64 result = {};
    if (text_layout_get(&models->text_layouts, text_layout_id, &layout)){
        result = Ii64(layout.on_screen_range.min, layout.on_screen_range.max);
    }
    return(result);
}

API_EXPORT Rect_f32
Text_Layout_On_Screen(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    Rect_f32 result = {};
    if (text_layout_get(&models->text_layouts, text_layout_id, &layout)){
        result = Rf32_xy_wh(layout.coordinates.on_screen_p0, layout.coordinates.dim);
        Vec2_f32 coordinate_center = models_get_coordinate_center(models);
        result.p0 -= coordinate_center;
        result.p1 -= coordinate_center;
    }
    return(result);
}

API_EXPORT Rect_f32
Text_Layout_Line_On_Screen(Application_Links *app, Text_Layout_ID layout_id, i64 line_number){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    Rect_f32 result = {};
    if (text_layout_get(&models->text_layouts, layout_id, &layout)){
        Editing_File *file = imp_get_file(models, layout.buffer_id);
        if (api_check_buffer(file)){
            Partial_Cursor first_partial = file_compute_partial_cursor(file, seek_line_char(line_number, 1));
            Partial_Cursor last_partial = file_compute_partial_cursor(file, seek_line_char(line_number, -1));
            Full_Cursor first = file_compute_cursor(models, file, seek_pos(first_partial.pos));
            Full_Cursor last = file_compute_cursor(models, file, seek_pos(last_partial.pos));
            
            f32 top = first.wrapped_y;
            f32 bot = last.wrapped_y;
            if (file->settings.unwrapped_lines){
                top = first.unwrapped_y;
                bot = last.unwrapped_y;
            }
            
            Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
            f32 line_height = face->height;
            bot += line_height;

            result = Rf32(layout.coordinates.on_screen_p0.x, top,
                          layout.coordinates.on_screen_p0.x + layout.coordinates.dim.x, bot);
            
            f32 shift_y = layout.coordinates.on_screen_p0.y - layout.coordinates.in_buffer_p0.y;
            result.y0 += shift_y;
            result.y1 += shift_y;
            
            Rect_f32 whole_layout = Rf32_xy_wh(layout.coordinates.on_screen_p0, layout.coordinates.dim);
            result = rect_intersect(result, whole_layout);
            
            Vec2_f32 coordinate_center = models_get_coordinate_center(models);
            result.p0 -= coordinate_center;
            result.p1 -= coordinate_center;
        }
    }
    return(result);
}

API_EXPORT Rect_f32
Text_Layout_Character_On_Screen(Application_Links *app, Text_Layout_ID layout_id, i64 pos){
    Models *models = (Models*)app->cmd_context;
    Text_Layout layout = {};
    Rect_f32 result = {};
    if (text_layout_get(&models->text_layouts, layout_id, &layout)){
        Editing_File *file = imp_get_file(models, layout.buffer_id);
        if (api_check_buffer(file)){
            Full_Cursor cursor = file_compute_cursor(models, file, seek_pos(pos));
            
            f32 top = cursor.wrapped_y;
            f32 left = cursor.wrapped_x;
            if (file->settings.unwrapped_lines){
                top = cursor.unwrapped_y;
                left = cursor.unwrapped_x;
            }
            
            Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
            f32 line_height = face->height;
            f32 bot = top + line_height;
            
            i64 size = buffer_size(&file->state.buffer);
            Range_i64 range = Ii64(pos, pos + 4);
            range.max = clamp_top(range.max, size);
            u8 space[4];
            buffer_read_range(app, layout.buffer_id, range, (char*)space);
            u32 codepoint = utf8_to_u32_unchecked(space);
            f32 advance = font_get_glyph_advance(face, codepoint);
            
            result = Rf32(left, top, left + advance, bot);
            
            Vec2_f32 shift = layout.coordinates.on_screen_p0 - layout.coordinates.in_buffer_p0;
            result.p0 += shift;
            result.p1 += shift;
            
            Rect_f32 whole_layout = Rf32_xy_wh(layout.coordinates.on_screen_p0, layout.coordinates.dim);
            result = rect_intersect(result, whole_layout);
            
            Vec2_f32 coordinate_center = models_get_coordinate_center(models);
            result.p0 -= coordinate_center;
            result.p1 -= coordinate_center;
        }
    }
    return(result);
}

API_EXPORT void
Paint_Text_Color(Application_Links *app, Text_Layout_ID layout_id, Range_i64 range, int_color color){
    //NotImplemented;
}

API_EXPORT b32
Text_Layout_Free(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    return(text_layout_erase(&models->text_layouts, text_layout_id));
}

API_EXPORT Text_Layout_ID
Compute_Render_Layout(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Vec2 screen_p, Vec2 layout_dim, Buffer_Point buffer_point, i32 one_past_last){
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    View *view = imp_get_view(models, view_id);
    Editing_File *file = imp_get_file(models, buffer_id);
    Text_Layout_ID result = 0;
    if (api_check_view(view) && api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
        Assert(face != 0);
        
        f32 line_height = (f32)face->height;
        f32 max_x = (f32)file->settings.display_width;
        f32 max_y = layout_dim.y + line_height;
        
        // TODO(allen): Don't force me to over-allocate!  Write a looser buffer render thingy.
        f32 smallest_char_width  = 6.f;
        f32 smallest_char_height = 8.f;
        i32 max = (i32)(layout_dim.x*layout_dim.y/(smallest_char_width*smallest_char_height))*2;
        max = clamp_bot(0, max);
        if (view->layout_arena.base_allocator == 0){
            view->layout_arena = make_arena_app_links(app);
        }
        else{
            linalloc_clear(&view->layout_arena);
        }
        Buffer_Render_Item *items = push_array(&view->layout_arena, Buffer_Render_Item, max);
        
        b32 wrapped = (!file->settings.unwrapped_lines);
        
        Full_Cursor intermediate_cursor = file_compute_cursor(models, file, seek_line_char(buffer_point.line_number, 1));
        f32 scroll_x = buffer_point.pixel_shift.x;
        f32 scroll_y = intermediate_cursor.wrapped_y;
        if (file->settings.unwrapped_lines){
            scroll_y = intermediate_cursor.unwrapped_y;
        }
        scroll_y += buffer_point.pixel_shift.y;
        Vec2_f32 in_buffer_p = V2f32(scroll_x, scroll_y);
        Full_Cursor render_cursor = file_get_render_cursor(models, file, scroll_y);
        
        i32 item_count = 0;
        i32 end_pos = 0;
        {
            Buffer_Render_Params params;
            params.buffer        = &file->state.buffer;
            params.items         = items;
            params.max           = max;
            params.count         = &item_count;
            params.port_x        = screen_p.x;
            params.port_y        = screen_p.y;
            params.clip_w        = layout_dim.x;
            params.scroll_x      = scroll_x;
            params.scroll_y      = scroll_y;
            params.width         = max_x;
            params.height        = (f32)max_y;
            params.start_cursor  = render_cursor;
            params.wrapped       = wrapped;
            params.system        = system;
            params.face          = face;
            params.virtual_white = file->settings.virtual_white;
            params.wrap_slashes  = file->settings.wrap_indicator;
            params.one_past_last_abs_pos = one_past_last;
            
            Buffer_Render_State state = {};
            Buffer_Layout_Stop stop = {};
            
            f32 line_shift = 0.f;
            b32 do_wrap = false;
            i32 wrap_unit_end = 0;
            
            b32 first_wrap_determination = true;
            i32 wrap_array_index = 0;
            
            do{
                stop = buffer_render_data(&state, params, line_shift, do_wrap, wrap_unit_end);
                switch (stop.status){
                    case BLStatus_NeedWrapDetermination:
                    {
                        if (first_wrap_determination){
                            wrap_array_index = binary_search(file->state.wrap_positions, stop.pos, 0, file->state.wrap_position_count);
                            ++wrap_array_index;
                            if (file->state.wrap_positions[wrap_array_index] == stop.pos){
                                do_wrap = true;
                                wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                            }
                            else{
                                do_wrap = false;
                                wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                            }
                            first_wrap_determination = false;
                        }
                        else{
                            Assert(stop.pos == wrap_unit_end);
                            do_wrap = true;
                            ++wrap_array_index;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                    }break;
                    
                    case BLStatus_NeedWrapLineShift:
                    case BLStatus_NeedLineShift:
                    {
                        line_shift = file->state.line_indents[stop.wrap_line_index];
                    }break;
                }
            }while(stop.status != BLStatus_Finished);
            
            end_pos = state.i;
        }
        
        Range range = Ii32((i32)render_cursor.pos, (i32)end_pos);
        
        // TODO(allen): 
        view->render.view_rect = view->panel->rect_inner;
        view->render.buffer_rect = i32R(f32R(screen_p.x, screen_p.y,
                                             screen_p.x + layout_dim.x, screen_p.y + layout_dim.y));
        view->render.cursor = render_cursor;
        view->render.range = range;
        view->render.items = items;
        view->render.item_count = item_count;
        
        f32 height = 0.f;
        if (item_count > 0){
            Buffer_Render_Item *render_item_first = items;
            Buffer_Render_Item *render_item_last = items + item_count - 1;
            height = render_item_last->y1 - render_item_first->y0;
        }
        Text_Layout_Coordinates coordinates = {};
        coordinates.on_screen_p0 = screen_p;
        coordinates.in_buffer_p0 = in_buffer_p;
        coordinates.dim = layout_dim;
        result = text_layout_new(&models->mem.heap, &models->text_layouts, buffer_id, buffer_point, range, height, coordinates);
    }
    return(result);
}

API_EXPORT void
Draw_Render_Layout(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view) && models->target != 0){
        render_loaded_file_in_view__inner(models, models->target, view, view->render.buffer_rect,
                                          view->render.cursor, view->render.range,
                                          view->render.items, view->render.item_count);
        linalloc_clear(&view->layout_arena);
    }
}

API_EXPORT void
Open_Color_Picker(Application_Links *app, Color_Picker *picker)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    if (picker->finished){
        *picker->finished = false;
    }
    system->open_color_picker(picker);
}

API_EXPORT void
Animate_In_N_Milliseconds(Application_Links *app, u32 n)
{
    Models *models = (Models*)app->cmd_context;
    if (n == 0){
        models->animate_next_frame = true;
    }
    else{
        models->next_animate_delay = Min(models->next_animate_delay, n);
    }
}

API_EXPORT String_Match_List
Buffer_Find_All_Matches(Application_Links *app, Arena *arena, Buffer_ID buffer, i32 string_id, Range_i64 range, String_Const_u8 needle, Character_Predicate *predicate, Scan_Direction direction)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match_List list = {};
    if (api_check_buffer(file)){
        if (needle.size > 0){
            String_Const_u8 space[3];
            Cursor cursor = make_cursor(space, sizeof(space));
            String_Const_u8_Array chunks = buffer_get_chunks(&cursor, &file->state.buffer);
            chunks = buffer_chunks_clamp(chunks, range);
            if (chunks.count > 0){
                u64_Array jump_table = string_compute_needle_jump_table(arena, needle, direction);
                Character_Predicate dummy = {};
                if (predicate == 0){
                    predicate = &dummy;
                }
                list = find_all_matches(arena, max_i32,
                                        chunks, needle, jump_table, predicate, direction,
                                        range.min, buffer, string_id);
            }
        }
    }
    return(list);
}

API_EXPORT Range
Get_View_Visible_Range(Application_Links *app, View_ID view_id){
    Range result = {};
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Face *face = font_set_face_from_id(&models->font_set, file->settings.font_id);
        i32 view_height = rect_height(view->panel->rect_inner);
        i32 line_height = (i32)face->height;
        Full_Cursor min_cursor = view_get_render_cursor(models, view);
        Buffer_Seek seek = seek_unwrapped_xy(0.f, min_cursor.wrapped_y + view_height + line_height, false);
        Full_Cursor max_cursor = view_compute_cursor(app, view_id, seek);
        result = Ii32((i32)min_cursor.pos, (i32)max_cursor.pos);
    }
    return(result);
}

// BOTTOM
