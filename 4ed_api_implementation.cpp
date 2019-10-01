/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.????
 *
 * Implementation of the API functions.
 *
 */

// TOP

function b32
access_test(u32 lock_flags, u32 access_flags){
    return((lock_flags & ~access_flags) == 0);
}

function b32
api_check_panel(Panel *panel){
    b32 result = false;
    if (panel != 0 && panel->kind != PanelKind_Unused){
        result = true;
    }
    return(result);
}

function b32
api_check_buffer(Editing_File *file){
    return(file != 0);
}

function b32
api_check_buffer(Editing_File *file, Access_Flag access){
    return(api_check_buffer(file) && access_test(file_get_access_flags(file), access));
}

function b32
api_check_view(View *view){
    return(view != 0 && view->in_use);
}

function b32
api_check_view(View *view, Access_Flag access){
    return(api_check_view(view) && access_test(view_get_access_flags(view), access));
}

function b32
is_running_coroutine(Application_Links *app){
    return(app->current_coroutine != 0);
}

api(custom) function b32
Global_Set_Setting(Application_Links *app, Global_Setting_ID setting, i32 value)
{
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

api(custom) function b32
Global_Set_Mapping(Application_Links *app, void *data, i32 size)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = interpret_binding_buffer(models, data, size);
    return(result);
}

api(custom) function Rect_f32
Global_Get_Screen_Rectangle(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(Rf32(V2(0, 0), V2(layout_get_root_size(&models->layout))));
}

api(custom) function Thread_Context*
Get_Thread_Context(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(models->tctx);
}

api(custom) function b32
Create_Child_Process(Application_Links *app, String_Const_u8 path, String_Const_u8 command, Child_Process_ID *child_process_id_out){
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(child_process_call(models, system, path, command, child_process_id_out));
}

api(custom) function b32
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

api(custom) function Child_Process_ID
Buffer_Get_Attached_Child_Process(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Child_Process_ID result = 0;
    if (api_check_buffer(file)){
        result = file->state.attached_child_process;
    }
    return(result);
}

api(custom) function Buffer_ID
Child_Process_Get_Attached_Buffer(Application_Links *app, Child_Process_ID child_process_id){
    Models *models = (Models*)app->cmd_context;
    Child_Process *child_process = child_process_from_id(&models->child_processes, child_process_id);
    Buffer_ID result = 0;
    if (child_process != 0 && child_process->out_file != 0){
        result = child_process->out_file->id;
    }
    return(result);
}

api(custom) function Process_State
Child_Process_Get_State(Application_Links *app, Child_Process_ID child_process_id){
    Models *models = (Models*)app->cmd_context;
    return(child_process_get_state(&models->child_processes, child_process_id));
}

api(custom) function b32
Clipboard_Post(Application_Links *app, i32 clipboard_id, String_Const_u8 string)
{
    Models *models = (Models*)app->cmd_context;
    String_Const_u8 *dest = working_set_next_clipboard_string(&models->heap, &models->working_set, (i32)string.size);
    block_copy(dest->str, string.str, string.size);
    models->system->post_clipboard(*dest);
    return(true);
}

api(custom) function i32
Clipboard_Count(Application_Links *app, i32 clipboard_id)
{
    Models *models = (Models*)app->cmd_context;
    return(models->working_set.clipboard_size);
}

api(custom) function String_Const_u8
Push_Clipboard_Index(Application_Links *app, Arena *arena, i32 clipboard_id, i32 item_index)
{
    Models *models = (Models*)app->cmd_context;
    String_Const_u8 *str = working_set_clipboard_index(&models->working_set, item_index);
    String_Const_u8 result = {};
    if (str != 0){
        result = push_string_copy(arena, *str);
    }
    return(result);
}

api(custom) function i32
Get_Buffer_Count(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    return(working_set->active_file_count);
}

api(custom) function Buffer_ID
Get_Buffer_Next(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = working_set_get_file(working_set, buffer_id);
    file = file_get_next(working_set, file);
    for (;file != 0 && !access_test(file_get_access_flags(file), access);){
        file = file_get_next(working_set, file);
    }
    Buffer_ID result = 0;
    if (file != 0){
        result = file->id;
    }
    return(result);
}

api(custom) function Buffer_ID
Get_Buffer_By_Name(Application_Links *app, String_Const_u8 name, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = working_set_contains_name(working_set, name);
    Buffer_ID result = 0;
    if (api_check_buffer(file, access)){
        result = file->id;
    }
    return(result);
}

api(custom) function Buffer_ID
Get_Buffer_By_File_Name(Application_Links *app, String_Const_u8 file_name, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File_Name canon = {};
    Buffer_ID result = false;
    Scratch_Block scratch(app);
    if (get_canon_name(system, scratch, file_name, &canon)){
        Working_Set *working_set = &models->working_set;
        Editing_File *file = working_set_contains_canon(working_set, string_from_file_name(&canon));
        if (api_check_buffer(file, access)){
            result = file->id;
        }
    }
    return(result);
}

api(custom) function b32
Buffer_Read_Range(Application_Links *app, Buffer_ID buffer_id, Range_i64 range, char *out)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        i64 size = buffer_size(&file->state.buffer);
        if (0 <= range.min && range.min <= range.max && range.max <= size){
            Scratch_Block scratch(app);
            String_Const_u8 string = buffer_stringify(scratch, &file->state.buffer, range);
            block_copy(out, string.str, string.size);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
Buffer_Replace_Range(Application_Links *app, Buffer_ID buffer_id, Range_i64 range, String_Const_u8 string)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        i64 size = buffer_size(&file->state.buffer);
        if (0 <= range.first && range.first <= range.one_past_last && range.one_past_last <= size){
            Edit_Behaviors behaviors = {};
            edit_single(models, file, range, string, behaviors);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
Buffer_Batch_Edit(Application_Links *app, Buffer_ID buffer_id, Batch_Edit *batch)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        Edit_Behaviors behaviors = {};
        result = edit_batch(models, file, batch, behaviors);
    }
    return(result);
}

api(custom) function String_Match
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
            i64 size = buffer_size(gap_buffer);
            List_String_Const_u8 chunks = buffer_get_chunks(scratch, gap_buffer);
            Interval_i64 range = {};
            if (direction == Scan_Forward){
                i64 adjusted_pos = start_pos + 1;
                start_pos = clamp_top(adjusted_pos, size);
                range = make_range_i64(adjusted_pos, size);
            }
            else{
                i64 adjusted_pos = start_pos - 1 + needle.size;
                start_pos = clamp_bot(0, adjusted_pos);
                range = make_range_i64(0, adjusted_pos);
            }
            buffer_chunks_clamp(&chunks, range);
            if (chunks.first != 0){
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

api(custom) function String_Match
Buffer_Seek_Character_Class(Application_Links *app, Buffer_ID buffer, Character_Predicate *predicate, Scan_Direction direction, i64 start_pos){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match result = {};
    if (api_check_buffer(file)){
        Scratch_Block scratch(app);
        Gap_Buffer *gap_buffer = &file->state.buffer;
        List_String_Const_u8 chunks_list = buffer_get_chunks(scratch, gap_buffer);
        
        if (chunks_list.node_count > 0){
            // TODO(allen): If you are reading this comment, then I haven't revisited this to tighten it up yet.
            // buffer_seek_character_class was originally implemented using the chunk indexer helper
            // Buffer_Chunk_Position, and it was written when buffer chunks were in an array instead
            // of the new method of listing strings in a linked list.
            //   This should probably be implemented as a direct iteration-in-an-iteration that avoids the
            // extra function calls and branches to achieve the iteration.  However, this is a very easy API to
            // get wrong.  There are _a lot_ of opportunities for off by one errors and necessary code duplication,
            // really tedious stuff.  Anyway, this is all just to say, cleaning this up would be really nice, but
            // there are almost certainly lower hanging fruit with higher payoffs elsewhere... unless need to change
            // this anyway or whatever.
            String_Const_u8 chunk_mem[3] = {};
            String_Const_u8_Array chunks = {chunk_mem};
            for (Node_String_Const_u8 *node = chunks_list.first;
                 node != 0;
                 node = node->next){
                chunks.vals[chunks.count] = node->string;
                chunks.count += 1;
            }
            
            i64 size = buffer_size(gap_buffer);
            start_pos = clamp(-1, start_pos, size);
            Buffer_Chunk_Position pos = buffer_get_chunk_position(chunks, size, start_pos);
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

api(custom) function f32
Buffer_Line_Y_Difference(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line_a, i64 line_b){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    f32 result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = file_line_y_difference(models, file, width, face, line_a, line_b);
        }
    }
    return(result);
}

api(custom) function Line_Shift_Vertical
Buffer_Line_Shift_Y(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 line, f32 y_shift){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Line_Shift_Vertical result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = file_line_shift_y(models, file, width, face, line, y_shift);
        }
    }
    return(result);
}

api(custom) function i64
Buffer_Pos_At_Relative_XY(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, Vec2_f32 relative_xy){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = -1;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = file_pos_at_relative_xy(models, file, width, face, base_line, relative_xy);
        }
    }
    return(result);
}

api(custom) function Vec2_f32
Buffer_Relative_XY_Of_Pos(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Vec2_f32 result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = file_relative_xy_of_pos(models, file, width, face, base_line, pos);
        }
    }
    return(result);
}

api(custom) function i64
Buffer_Relative_Character_From_Pos(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = file_relative_character_from_pos(models, file, width, face, base_line, pos);
        }
    }
    return(result);
}

api(custom) function i64
Buffer_Pos_From_Relative_Character(Application_Links *app,  Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 relative_character)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = -1;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = file_pos_from_relative_character(models, file, width, face, base_line, relative_character);
        }
    }
    return(result);
}

api(custom) function f32
View_Line_Y_Difference(Application_Links *app, View_ID view_id, i64 line_a, i64 line_b){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    f32 result = {};
    if (api_check_view(view)){
        result = view_line_y_difference(models, view, line_a, line_b);
    }
    return(result);
}

api(custom) function Line_Shift_Vertical
View_Line_Shift_Y(Application_Links *app, View_ID view_id, i64 line, f32 y_shift){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Line_Shift_Vertical result = {};
    if (api_check_view(view)){
        result = view_line_shift_y(models, view, line, y_shift);
    }
    return(result);
}

api(custom) function i64
View_Pos_At_Relative_XY(Application_Links *app, View_ID view_id, i64 base_line, Vec2_f32 relative_xy){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = -1;
    if (api_check_view(view)){
        result = view_pos_at_relative_xy(models, view, base_line, relative_xy);
    }
    return(result);
}

api(custom) function Vec2_f32
View_Relative_XY_Of_Pos(Application_Links *app, View_ID view_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Vec2_f32 result = {};
    if (api_check_view(view)){
        result = view_relative_xy_of_pos(models, view, base_line, pos);
    }
    return(result);
}

api(custom) function i64
View_Relative_Character_From_Pos(Application_Links *app,  View_ID view_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = {};
    if (api_check_view(view)){
        result = view_relative_character_from_pos(models, view, base_line, pos);
    }
    return(result);
}

api(custom) function i64
View_Pos_From_Relative_Character(Application_Links *app,  View_ID view_id, i64 base_line, i64 character){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = {};
    if (api_check_view(view)){
        result = view_pos_from_relative_character(models, view, base_line, character);
    }
    return(result);
}

api(custom) function b32
Buffer_Exists(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    return(api_check_buffer(file));
}

api(custom) function b32
Buffer_Ready(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = file_is_ready(file);
    }
    return(result);
}

api(custom) function Access_Flag
Buffer_Get_Access_Flags(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Access_Flag result = 0;
    if (api_check_buffer(file)){
        result = file_get_access_flags(file);
    }
    return(result);
}

api(custom) function i64
Buffer_Get_Size(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = 0;
    if (api_check_buffer(file)){
        result = buffer_size(&file->state.buffer);
    }
    return(result);
}

api(custom) function i64
Buffer_Get_Line_Count(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    u64 result = 0;
    if (api_check_buffer(file)){
        result = buffer_line_count(&file->state.buffer);
    }
    return(result);
}

api(custom) function String_Const_u8
Push_Buffer_Base_Name(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(arena, string_from_file_name(&file->base_name));
    }
    return(result);
}

api(custom) function String_Const_u8
Push_Buffer_Unique_Name(Application_Links *app, Arena *out, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(out, string_from_file_name(&file->unique_name));
    }
    return(result);
}

api(custom) function String_Const_u8
Push_Buffer_File_Name(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(arena, string_from_file_name(&file->canon));
    }
    return(result);
}

api(custom) function Dirty_State
Buffer_Get_Dirty_State(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Dirty_State result = {};
    if (api_check_buffer(file)){
        result = file->state.dirty;
    }
    return(result);
}

api(custom) function b32
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

api(custom) function b32
Buffer_Get_Setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 *value_out)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        switch (setting){
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

api(custom) function b32
Buffer_Set_Setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i32 value)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        switch (setting){
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
#if 0
                b32 full_remeasure = false;
                if (value){
                    if (!file->settings.virtual_white){
                        if (!file->settings.tokens_exist){
                            file_first_lex_serial(system, models, file);
                        }
                        file->settings.virtual_white = true;
                        full_remeasure = true;
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
#endif
            }break;
            
            case BufferSetting_RecordsHistory:
            {
                if (value){
                    if (!history_is_activated(&file->state.history)){
                        history_init(models, &file->state.history);
                    }
                }
                else{
                    if (history_is_activated(&file->state.history)){
                        history_free(models, &file->state.history);
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

api(custom) function Managed_Scope
Buffer_Get_Managed_Scope(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Managed_Scope result = 0;
    if (api_check_buffer(file)){
        result = file_get_managed_scope(file);
    }
    return(result);
}

api(custom) function b32
Buffer_Send_End_Signal(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        file_end_file(models, file);
        result = true;
    }
    return(result);
}

api(custom) function Buffer_ID
Create_Buffer(Application_Links *app, String_Const_u8 file_name, Buffer_Create_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *new_file = create_file(models, file_name, flags);
    Buffer_ID result = 0;
    if (new_file != 0){
        result = new_file->id;
    }
    return(result);
}

api(custom) function b32
Buffer_Save(Application_Links *app, Buffer_ID buffer_id, String_Const_u8 file_name, Buffer_Save_Flag flags)
{
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
            Scratch_Block scratch(models->tctx, Scratch_Share);
            String_Const_u8 name = push_string_copy(scratch, file_name);
            save_file_to_name(system, models, file, name.str);
            result = true;
        }
    }
    
    return(result);
}

api(custom) function Buffer_Kill_Result
Buffer_Kill(Application_Links *app, Buffer_ID buffer_id, Buffer_Kill_Flag flags)
{
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
                    models->hook_end_file(&models->app_links, file->id);
                }
                
                buffer_unbind_name_low_level(working_set, file);
                if (file->canon.name_size != 0){
                    buffer_unbind_file(system, working_set, file);
                }
                file_free(models, file);
                working_set_free_file(&models->heap, working_set, file);
                
                Layout *layout = &models->layout;
                
                Node *order = &working_set->touch_order_sentinel;
                Node *file_node = order->next;
                for (Panel *panel = layout_get_first_open_panel(layout);
                     panel != 0;
                     panel = layout_get_next_open_panel(layout, panel)){
                    View *view = panel->view;
                    if (view->file == file){
                        Assert(file_node != order);
                        view->file = 0;
                        Editing_File *new_file = CastFromMember(Editing_File, touch_node, file_node);
                        view_set_file(system, models, view, new_file);
                        file_node = file_node->next;
                        if (file_node == order){
                            file_node = file_node->next;
                        }
                        Assert(file_node != order);
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

api(custom) function Buffer_Reopen_Result
Buffer_Reopen(Application_Links *app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    Scratch_Block scratch(models->tctx, Scratch_Share);
    Editing_File *file = imp_get_file(models, buffer_id);
    Buffer_Reopen_Result result = BufferReopenResult_Failed;
    if (api_check_buffer(file)){
        if (file->canon.name_size > 0){
            Plat_Handle handle = {};
            if (system->load_handle(scratch, (char*)file->canon.name_space, &handle)){
                File_Attributes attributes = system->load_attributes(handle);
                
                char *file_memory = push_array(scratch, char, (i32)attributes.size);
                
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
                                Buffer_Cursor cursor = file_compute_cursor(view_it->file, seek_pos(edit_pos.cursor_pos));
                                line_numbers[vptr_count]   = (i32)cursor.line;
                                column_numbers[vptr_count] = (i32)cursor.col;
                                view_it->file = models->scratch_buffer;
                                ++vptr_count;
                            }
                        }
                        
                        Working_Set *working_set = &models->working_set;
                        file_free(models, file);
                        working_set_file_default_settings(working_set, file);
                        file_create_from_string(models, file, SCu8(file_memory, attributes.size), attributes);
                        
                        for (i32 i = 0; i < vptr_count; ++i){
                            view_set_file(system, models, vptrs[i], file);
                            
                            vptrs[i]->file = file;
                            i64 line = line_numbers[i];
                            i64 col = column_numbers[i];
                            Buffer_Cursor cursor = file_compute_cursor(file, seek_line_col(line, col));
                            view_set_cursor(models, vptrs[i], cursor.pos);
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
            }
        }
    }
    return(result);
}

api(custom) function File_Attributes
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

api(custom) function File_Attributes
Get_File_Attributes(Application_Links *app, String_Const_u8 file_name)
{
    Models *models = (Models*)app->cmd_context;
    Scratch_Block scratch(models->tctx, Scratch_Share);
    return(models->system->quick_file_attributes(scratch, file_name));
}

function View*
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

function View*
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

api(custom) function View_ID
Get_View_Next(Application_Links *app, View_ID view_id, Access_Flag access)
{
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

api(custom) function View_ID
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

api(custom) function View_ID
Get_Active_View(Application_Links *app, Access_Flag access)
{
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

api(custom) function Panel_ID
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

api(custom) function b32
View_Exists(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = true;
    }
    return(result);
}

api(custom) function Buffer_ID
View_Get_Buffer(Application_Links *app, View_ID view_id, Access_Flag access){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Buffer_ID result = 0;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        if (api_check_buffer(file, access)){
            result = file->id;
        }
    }
    return(result);
}

api(custom) function i64
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

api(custom) function i64
View_Get_Mark_Pos(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = 0;;
    if (api_check_view(view)){
        result = view->mark;
    }
    return(result);
}

api(custom) function f32
View_Get_Preferred_X(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    f32 result = 0.f;;
    if (api_check_view(view)){
        result = view->preferred_x;
    }
    return(result);
}

api(custom) function b32
View_Set_Preferred_X(Application_Links *app, View_ID view_id, f32 x){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        view->preferred_x = x;
        result = true;
    }
    return(result);
}

api(custom) function Rect_f32
View_Get_Screen_Rect(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        result = f32R(view->panel->rect_full);
    }
    return(result);
}

api(custom) function Panel_ID
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

api(custom) function View_ID
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

api(custom) function b32
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

api(custom) function b32
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

api(custom) function b32
Panel_Split(Application_Links *app, Panel_ID panel_id, Panel_Split_Orientation orientation){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        Panel *new_panel = 0;
        if (layout_split_panel(layout, panel, (orientation == PanelSplit_LeftAndRight), &new_panel)){
            Live_Views *live_set = &models->live_set;
            View *new_view = live_set_alloc_view(&models->lifetime_allocator, live_set, new_panel);
            view_set_file(models->system, models, new_view, models->scratch_buffer);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
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

api(custom) function b32
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

api(custom) function Panel_ID
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

api(custom) function Panel_ID
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

api(custom) function Panel_ID
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

api(custom) function Rect_i32
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

api(custom) function b32
View_Close(Application_Links *app, View_ID view_id)
{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        if (layout_close_panel(layout, view->panel)){
            live_set_free_view(&models->lifetime_allocator, &models->live_set, view);
            result = true;
        }
    }
    return(result);
}

api(custom) function Rect_f32
View_Get_Buffer_Region(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Rect_f32 result = {};
    if (api_check_view(view)){
        result = view_get_buffer_rect(models, view);
    }
    return(result);
}

api(custom) function Buffer_Scroll
View_Get_Buffer_Scroll(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Buffer_Scroll  result = {};
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        if (!view->ui_mode){
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            result = edit_pos.scroll;
        }
    }
    return(result);
}

api(custom) function Basic_Scroll
View_Get_Basic_Scroll(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Basic_Scroll  result = {};
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        if (view->ui_mode){
            result = view->ui_scroll;
        }
    }
    return(result);
}

api(custom) function b32
View_Set_Active(Application_Links *app, View_ID view_id)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        models->layout.active_panel = view->panel;
        result = true;
    }
    return(result);
}

api(custom) function b32
View_Get_Setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 *value_out)
{
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

api(custom) function b32
View_Set_Setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i32 value)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    
    b32 result = false;
    if (api_check_view(view)){
        result = true;
        switch (setting){
            case ViewSetting_ShowWhitespace:
            {
                view->show_whitespace = (b8)value;
            }break;
            
            case ViewSetting_ShowScrollbar:
            {
                view->hide_scrollbar = (b8)!value;
            }break;
            
            case ViewSetting_ShowFileBar:
            {
                view->hide_file_bar = (b8)!value;
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

api(custom) function Managed_Scope
View_Get_Managed_Scope(Application_Links *app, View_ID view_id)
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

api(custom) function Buffer_Cursor
Buffer_Compute_Cursor(Application_Links *app, Buffer_ID buffer, Buffer_Seek seek)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    Buffer_Cursor result = {};
    if (api_check_buffer(file)){
        result = file_compute_cursor(file, seek);
    }
    return(result);
}

api(custom) function Buffer_Cursor
View_Compute_Cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Buffer_Cursor result = {};
    if (api_check_view(view)){
        result = view_compute_cursor(view, seek);
    }
    return(result);
}

api(custom) function b32
View_Set_Cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Assert(file != 0);
        if (api_check_buffer(file)){
            Buffer_Cursor cursor = file_compute_cursor(file, seek);
            view_set_cursor(models, view, cursor.pos);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
View_Set_Buffer_Scroll(Application_Links *app, View_ID view_id, Buffer_Scroll scroll)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        if (!view->ui_mode){
            scroll.position = view_normalize_buffer_point(models, view, scroll.position);
            scroll.target = view_normalize_buffer_point(models, view, scroll.target);
            scroll.target.pixel_shift.x = f32_round32(scroll.target.pixel_shift.x);
            scroll.target.pixel_shift.y = f32_round32(scroll.target.pixel_shift.y);
            scroll.target.pixel_shift.x = clamp_bot(0.f, scroll.target.pixel_shift.x);
            Buffer_Layout_Item_List line = view_get_line_layout(models, view, scroll.target.line_number);
            scroll.target.pixel_shift.y = clamp(0.f, scroll.target.pixel_shift.y, line.height);
            view_set_scroll(models, view, scroll);
            view->new_scroll_target = true;
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
View_Set_Basic_Scroll(Application_Links *app, View_ID view_id, Basic_Scroll scroll)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        if (view->ui_mode){
            scroll.target.x = f32_round32(scroll.target.x);
            scroll.target.y = f32_round32(scroll.target.y);
            view->ui_scroll = scroll;
            view->new_scroll_target = true;
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
View_Set_Mark(Application_Links *app, View_ID view_id, Buffer_Seek seek)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Assert(file != 0);
        if (api_check_buffer(file)){
            if (seek.type != buffer_seek_pos){
                Buffer_Cursor cursor = file_compute_cursor(file, seek);
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

api(custom) function b32
View_Set_Buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = working_set_get_file(&models->working_set, buffer_id);
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

api(custom) function b32
View_Post_Fade(Application_Links *app, View_ID view_id, f32 seconds, Range_i64 range, int_color color)
{
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

api(custom) function b32
View_Begin_UI_Mode(Application_Links *app, View_ID view_id)
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

api(custom) function b32
View_End_UI_Mode(Application_Links *app, View_ID view_id)
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

api(custom) function b32
View_Is_In_UI_Mode(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = view->ui_mode;
    }
    return(result);
}

api(custom) function b32
View_Set_Quit_UI_Handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type *quit_function)
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

api(custom) function b32
View_Get_Quit_UI_Handler(Application_Links *app, View_ID view_id, UI_Quit_Function_Type **quit_function_out)
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

function Dynamic_Workspace*
get_dynamic_workspace(Models *models, Managed_Scope handle){
    Dynamic_Workspace *result = 0;
    Table_Lookup lookup = table_lookup(&models->lifetime_allocator.scope_id_to_scope_ptr_table, handle);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&models->lifetime_allocator.scope_id_to_scope_ptr_table, lookup, &val);
        result = (Dynamic_Workspace*)IntAsPtr(val);
    }
    return(result);
}

api(custom) function Managed_Scope
Create_User_Managed_Scope(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    Lifetime_Object *object = lifetime_alloc_object(&models->lifetime_allocator, DynamicWorkspace_Unassociated, 0);
    object->workspace.user_back_ptr = object;
    Managed_Scope scope = (Managed_Scope)object->workspace.scope_id;
    return(scope);
}

api(custom) function b32
Destroy_User_Managed_Scope(Application_Links *app, Managed_Scope scope)
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0 && workspace->user_type == DynamicWorkspace_Unassociated){
        Lifetime_Object *lifetime_object = (Lifetime_Object*)workspace->user_back_ptr;
        lifetime_free_object(&models->lifetime_allocator, lifetime_object);
        result = true;
    }
    return(result);
}

api(custom) function Managed_Scope
Get_Global_Managed_Scope(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return((Managed_Scope)models->dynamic_workspace.scope_id);
}

function Lifetime_Object*
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

api(custom) function Managed_Scope
Get_Managed_Scope_With_Multiple_Dependencies(Application_Links *app, Managed_Scope *scopes, i32 count)
{
    Models *models = (Models*)app->cmd_context;
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    
    Scratch_Block scratch(models->tctx, Scratch_Share);
    
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
        Lifetime_Key *key = lifetime_get_or_create_intersection_key(lifetime_allocator, object_ptr_array, member_count);
        result = (Managed_Scope)key->dynamic_workspace.scope_id;
    }
    
    return(result);
}

api(custom) function b32
Managed_Scope_Clear_Contents(Application_Links *app, Managed_Scope scope)
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0){
        dynamic_workspace_clear_contents(workspace);
        result = true;
    }
    return(result);
}

api(custom) function b32
Managed_Scope_Clear_Self_All_Dependent_Scopes(Application_Links *app, Managed_Scope scope)
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0 && workspace->user_type != DynamicWorkspace_Global && workspace->user_type != DynamicWorkspace_Intersected){
        Lifetime_Object *object = get_lifetime_object_from_workspace(workspace);
        Assert(object != 0);
        lifetime_object_reset(&models->lifetime_allocator, object);
        result = true;
    }
    return(result);
}

api(custom) function Base_Allocator*
Managed_Scope_Allocator(Application_Links *app, Managed_Scope scope)
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    Base_Allocator *result = 0;
    if (workspace != 0){
        result = &workspace->heap_wrapper;
    }
    return(result);
}

api(custom) function Managed_ID
Managed_Id_Declare(Application_Links *app, String_Const_u8 name)
{
    Models *models = (Models*)app->cmd_context;
    Managed_ID_Set *set = &models->managed_id_set;
    return(managed_ids_declare(set, name));
}

api(custom) function void*
Managed_Scope_Get_Attachment(Application_Links *app, Managed_Scope scope, Managed_ID id, umem size){
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    void *result = 0;
    if (workspace != 0){
        Dynamic_Variable_Block *var_block = &workspace->var_block;
        Data data = dynamic_variable_get(var_block, id, size);
        if (data.size >= size){
            result = data.data;
        }
        else{
#define M \
            "ERROR: scope attachment already exists with a size smaller than the requested size; no attachment pointer can be returned."
            print_message(app, string_u8_litexpr(M));
        }
    }
    return(result);
}

api(custom) function void*
Managed_Scope_Attachment_Erase(Application_Links *app, Managed_Scope scope, Managed_ID id){
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    void *result = 0;
    if (workspace != 0){
        Dynamic_Variable_Block *var_block = &workspace->var_block;
        dynamic_variable_erase(var_block, id);
    }
    return(result);
}

api(custom) function Managed_Object
Alloc_Managed_Memory_In_Scope(Application_Links *app, Managed_Scope scope, i32 item_size, i32 count)
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    Managed_Object result = 0;
    if (workspace != 0){
        result = managed_object_alloc_managed_memory(workspace, item_size, count, 0);
    }
    return(result);
}

api(custom) function Managed_Object
Alloc_Buffer_Markers_On_Buffer(Application_Links *app, Buffer_ID buffer_id, i32 count, Managed_Scope *optional_extra_scope)
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
        result = managed_object_alloc_buffer_markers(workspace, buffer_id, count, 0);
    }
    return(result);
}

function Managed_Object_Ptr_And_Workspace
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

api(custom) function u32
Managed_Object_Get_Item_Size(Application_Links *app, Managed_Object object)
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u32 result = 0;
    if (object_ptrs.header != 0){
        result = object_ptrs.header->item_size;
    }
    return(result);
}

api(custom) function u32
Managed_Object_Get_Item_Count(Application_Links *app, Managed_Object object)
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u32 result = 0;
    if (object_ptrs.header != 0){
        result = object_ptrs.header->count;
    }
    return(result);
}

api(custom) function void*
Managed_Object_Get_Pointer(Application_Links *app, Managed_Object object)
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    return(get_dynamic_object_memory_ptr(object_ptrs.header));
}

api(custom) function Managed_Object_Type
Managed_Object_Get_Type(Application_Links *app, Managed_Object object)
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

api(custom) function Managed_Scope
Managed_Object_Get_Containing_Scope(Application_Links *app, Managed_Object object)
{
    Models *models = (Models*)app->cmd_context;
    u32 hi_id = (object >> 32)&max_u32;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, hi_id);
    if (workspace != 0){
        return((Managed_Scope)hi_id);
    }
    return(0);
}

api(custom) function b32
Managed_Object_Free(Application_Links *app, Managed_Object object)
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

// TODO(allen): ELIMINATE STORE & LOAD
api(custom) function b32
Managed_Object_Store_Data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem)
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
            heap_assert_good(&object_ptrs.workspace->heap);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
Managed_Object_Load_Data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem_out)
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
            heap_assert_good(&object_ptrs.workspace->heap);
            result = true;
        }
    }
    return(result);
}

api(custom) function User_Input
Get_User_Input(Application_Links *app, Input_Type_Flag get_type, Input_Type_Flag abort_type)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    User_Input result = {};
    if (app->type_coroutine == Co_Command){
        Coroutine *coroutine = (Coroutine*)app->current_coroutine;
        Assert(coroutine != 0);
        ((u32*)coroutine->out)[0] = get_type;
        ((u32*)coroutine->out)[1] = abort_type;
        coroutine_yield(coroutine);
        result = *(User_Input*)coroutine->in;
    }
    return(result);
}

api(custom) function User_Input
Get_Command_Input(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    User_Input result = {};
    result.key = models->key;
    return(result);
}

api(custom) function void
Set_Command_Input(Application_Links *app, Key_Event_Data key_data)
{
    Models *models = (Models*)app->cmd_context;
    models->key = key_data;
}

api(custom) function Mouse_State
Get_Mouse_State(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return(models->input->mouse);
}

api(custom) function b32
Get_Active_Query_Bars(Application_Links *app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array *array_out)
{
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

api(custom) function b32
Start_Query_Bar(Application_Links *app, Query_Bar *bar, u32 flags)
{
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

api(custom) function void
End_Query_Bar(Application_Links *app, Query_Bar *bar, u32 flags)
{
    Models *models = (Models*)app->cmd_context;
    Panel *active_panel = layout_get_active_panel(&models->layout);
    View *active_view = active_panel->view;
    free_query_slot(&active_view->query_set, bar);
}

api(custom) function b32
Print_Message(Application_Links *app, String_Const_u8 message)
{
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

api(custom) function b32
Log_String(Application_Links *app, String_Const_u8 str){
    return(log_string(str));
}

api(custom) function i32
Thread_Get_ID(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(models->system->thread_get_id());
}

api(custom) function Face_ID
Get_Largest_Face_ID(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return(font_set_get_largest_id(&models->font_set));
}

api(custom) function b32
Set_Global_Face(Application_Links *app, Face_ID id, b32 apply_to_all_buffers)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    
    b32 did_change = false;
    Face *face = font_set_face_from_id(&models->font_set, id);
    if (face != 0){
        if (apply_to_all_buffers){
            global_set_font_and_update_files(system, models, face);
        }
        else{
            models->global_face_id = face->id;
        }
        did_change = true;
    }
    
    return(did_change);
}

api(custom) function History_Record_Index
Buffer_History_Get_Max_Record_Index(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    History_Record_Index result = 0;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        result = history_get_record_count(&file->state.history);
    }
    return(result);
}

function void
buffer_history__fill_record_info(Record *record, Record_Info *out){
    out->kind = record->kind;
    out->edit_number = record->edit_number;
    switch (out->kind){
        case RecordKind_Single:
        {
            out->single.string_forward  = record->single.forward_text ;
            out->single.string_backward = record->single.backward_text;
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

api(custom) function Record_Info
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

api(custom) function Record_Info
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

api(custom) function History_Record_Index
Buffer_History_Get_Current_State_Index(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    History_Record_Index result = 0;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        result = file_get_current_record_index(file);
    }
    return(result);
}

api(custom) function b32
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

api(custom) function b32
Buffer_History_Merge_Record_Range(Application_Links *app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = edit_merge_history_range(models, file, first_index, last_index, flags);
    }
    return(result);
}

api(custom) function b32
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

api(custom) function void
Global_History_Edit_Group_Begin(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, 1);
}

api(custom) function void
Global_History_Edit_Group_End(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, -1);
}

api(custom) function b32
Buffer_Set_Face(Application_Links *app, Buffer_ID buffer_id, Face_ID id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    
    b32 did_change = false;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, id);
        if (face != 0){
            file->settings.face_id = face->id;
            did_change = true;
        }
    }
    return(did_change);
}

api(custom) function Face_Description
Get_Face_Description(Application_Links *app, Face_ID face_id)
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

api(custom) function Face_Metrics
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

api(custom) function Face_ID
Get_Face_ID(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Face_ID result = 0;
    if (buffer_id != 0){
        Editing_File *file = imp_get_file(models, buffer_id);
        if (api_check_buffer(file)){
            result = file->settings.face_id;
        }
    }
    else{
        result = models->global_face_id;
    }
    return(result);
}

api(custom) function Face_ID
Try_Create_New_Face(Application_Links *app, Face_Description *description)
{
    Models *models = (Models*)app->cmd_context;
    Face_ID result = 0;
    if (is_running_coroutine(app)){
        System_Functions *system = models->system;
        Coroutine *coroutine = (Coroutine*)app->current_coroutine;
        Assert(coroutine != 0);
        ((Face_Description**)coroutine->out)[0] = description;
        ((u32*)coroutine->out)[2] = AppCoroutineRequest_NewFontFace;
        coroutine_yield(coroutine);
        result = *(Face_ID*)(coroutine->in);
    }
    else{
        Face *new_face = font_set_new_face(&models->font_set, description);
        result = new_face->id;
    }
    return(result);
}

api(custom) function b32
Try_Modify_Face(Application_Links *app, Face_ID id, Face_Description *description)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    if (is_running_coroutine(app)){
        System_Functions *system = models->system;
        Coroutine *coroutine = (Coroutine*)app->current_coroutine;
        Assert(coroutine != 0);
        ((Face_Description**)coroutine->out)[0] = description;
        ((u32*)coroutine->out)[2] = AppCoroutineRequest_ModifyFace;
        ((u32*)coroutine->out)[3] = id;
        coroutine_yield(coroutine);
        result = *(b32*)(coroutine->in);
    }
    else{
        result = font_set_modify_face(&models->font_set, id, description);
    }
    return(result);
}

api(custom) function b32
Try_Release_Face(Application_Links *app, Face_ID id, Face_ID replacement_id)
{
    Models *models = (Models*)app->cmd_context;
    Font_Set *font_set = &models->font_set;
    Face *face = font_set_face_from_id(font_set, id);
    Face *replacement = font_set_face_from_id(font_set, replacement_id);
    return(release_font_and_update(models->system, models, face, replacement));
}

api(custom) function void
Set_Theme_Colors(Application_Links *app, Theme_Color *colors, i32 count)
{
    Models *models = (Models*)app->cmd_context;
    Color_Table color_table = models->color_table;
    Theme_Color *theme_color = colors;
    for (i32 i = 0; i < count; ++i, ++theme_color){
        if (theme_color->tag < color_table.count){
            color_table.vals[theme_color->tag] = theme_color->color;
        }
    }
}

api(custom) function void
Get_Theme_Colors(Application_Links *app, Theme_Color *colors, i32 count)
{
    Models *models = (Models*)app->cmd_context;
    Color_Table color_table = models->color_table;
    Theme_Color *theme_color = colors;
    for (i32 i = 0; i < count; ++i, ++theme_color){
        theme_color->color = finalize_color(color_table, theme_color->tag);
    }
}

api(custom) function argb_color
Finalize_Color(Application_Links *app, int_color color){
    Models *models = (Models*)app->cmd_context;
    Color_Table color_table = models->color_table;
    u32 color_rgb = color;
    if ((color & 0xFF000000) == 0){
        color_rgb = color_table.vals[color % color_table.count];
    }
    return(color_rgb);
}

api(custom) function String_Const_u8
Push_Hot_Directory(Application_Links *app, Arena *arena)
{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    hot_directory_clean_end(hot);
    return(push_string_copy(arena, hot->string));
}

api(custom) function b32
Set_Hot_Directory(Application_Links *app, String_Const_u8 string)
{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    hot_directory_set(models->system, hot, string);
    return(true);
}

api(custom) function File_List
Get_File_List(Application_Links *app, Arena *arena, String_Const_u8 directory){
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    String_Const_u8 canonical_directory = system->get_canonical(arena, directory);
    File_List list = {};
    if (canonical_directory.str != 0){
        list = system->get_file_list(arena, canonical_directory);
    }
    return(list);
}

api(custom) function void
Set_GUI_Up_Down_Keys(Application_Links *app, Key_Code up_key, Key_Modifier up_key_modifier, Key_Code down_key, Key_Modifier down_key_modifier)
{
    Models *models = (Models*)app->cmd_context;
    models->user_up_key = up_key;
    models->user_up_key_modifier = up_key_modifier;
    models->user_down_key = down_key;
    models->user_down_key_modifier = down_key_modifier;
}

api(custom) function void*
Memory_Allocate(Application_Links *app, i32 size)
{
    Models *models = (Models*)app->cmd_context;
    void *result = models->system->memory_allocate(size);
    return(result);
}

api(custom) function b32
Memory_Set_Protection(Application_Links *app, void *ptr, i32 size, Memory_Protect_Flags flags)
{
    Models *models = (Models*)app->cmd_context;
    return(models->system->memory_set_protection(ptr, size, flags));
}

api(custom) function void
Memory_Free(Application_Links *app, void *ptr, i32 size)
{
    Models *models = (Models*)app->cmd_context;
    models->system->memory_free(ptr, size);
}

api(custom) function String_Const_u8
Push_4ed_Path(Application_Links *app, Arena *arena)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(system->get_4ed_path(arena));
}

// TODO(allen): do(add a "shown but auto-hides on timer" setting for cursor show type)
api(custom) function void
Show_Mouse_Cursor(Application_Links *app, Mouse_Cursor_Show_Type show)
{
    Models *models = (Models*)app->cmd_context;
    models->system->show_mouse_cursor(show);
}

api(custom) function b32
Set_Edit_Finished_Hook_Repeat_Speed(Application_Links *app, u32 milliseconds){
    Models *models = (Models*)app->cmd_context;
    models->edit_finished_hook_repeat_speed = milliseconds;
    return(true);
}

api(custom) function b32
Set_Fullscreen(Application_Links *app, b32 full_screen)
{
    Models *models = (Models*)app->cmd_context;
    b32 success = models->system->set_fullscreen(full_screen);
    if (!success){
        print_message(app, string_u8_litexpr("ERROR: Failed to go fullscreen.\n"));
    }
    return(success);
}

api(custom) function b32
Is_Fullscreen(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = models->system->is_fullscreen();
    return(result);
}

api(custom) function void
Send_Exit_Signal(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    models->keep_playing = false;
}

api(custom) function b32
Set_Window_Title(Application_Links *app, String_Const_u8 title)
{
    Models *models = (Models*)app->cmd_context;
    models->has_new_title = true;
    umem cap_before_null = (umem)(models->title_capacity - 1);
    umem copy_size = clamp_top(title.size, cap_before_null);
    block_copy(models->title_space, title.str, copy_size);
    models->title_space[copy_size] = 0;
    return(true);
}

api(custom) function Microsecond_Time_Stamp
Get_Microseconds_Timestamp(Application_Links *app)
{
    // TODO(allen): do(decrease indirection in API calls)
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    return(system->now_time());
}

function Vec2
models_get_coordinate_center(Models *models){
    Vec2 result = {};
    if (models->coordinate_center_stack_top > 0){
        result = models->coordinate_center_stack[models->coordinate_center_stack_top - 1];
    }
    return(result);
}

function Vec2
draw_helper__models_space_to_screen_space(Models *models, Vec2 point){
    Vec2 c = models_get_coordinate_center(models);
    return(point + c);
}

function f32_Rect
draw_helper__models_space_to_screen_space(Models *models, f32_Rect rect){
    Vec2 c = models_get_coordinate_center(models);
    rect.p0 += c;
    rect.p1 += c;
    return(rect);
}

api(custom) function Vec2
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

api(custom) function f32
Get_String_Advance(Application_Links *app, Face_ID font_id, String_Const_u8 str)
{
    Models *models = (Models*)app->cmd_context;
    Face *face = font_set_face_from_id(&models->font_set, font_id);
    return(font_string_width(models->target, face, str));
}

api(custom) function void
Draw_Rectangle(Application_Links *app, Rect_f32 rect, int_color color){
    Models *models = (Models*)app->cmd_context;
    if (models->in_render_mode){
        Color_Table color_table = models->color_table;
        rect = draw_helper__models_space_to_screen_space(models, rect);
        u32 actual_color = finalize_color(color_table, color);
        draw_rectangle(models->target, rect, actual_color);
    }
}

api(custom) function void
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

api(custom) function void
Draw_Clip_Push(Application_Links *app, Rect_f32 clip_box){
    Models *models = (Models*)app->cmd_context;
    //clip_box = draw_helper__models_space_to_screen_space(models, clip_box);
    draw_push_clip(models->target, Ri32(clip_box));
}

api(custom) function f32_Rect
Draw_Clip_Pop(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(Rf32(draw_pop_clip(models->target)));
}

api(custom) function void
Draw_Coordinate_Center_Push(Application_Links *app, Vec2 point){
    Models *models = (Models*)app->cmd_context;
    if (models->coordinate_center_stack_top < ArrayCount(models->coordinate_center_stack)){
        point = draw_helper__models_space_to_screen_space(models, point);
        models->coordinate_center_stack[models->coordinate_center_stack_top] = point;
        models->coordinate_center_stack_top += 1;
    }
}

api(custom) function Vec2
Draw_Coordinate_Center_Pop(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    Vec2 result = {};
    if (models->coordinate_center_stack_top > 0){
        models->coordinate_center_stack_top -= 1;
        result = models->coordinate_center_stack[models->coordinate_center_stack_top];
    }
    return(result);
}

api(custom) function Text_Layout_ID
Text_Layout_Create(Application_Links *app, Buffer_ID buffer_id, Rect_f32 rect, Buffer_Point buffer_point){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Text_Layout_ID result = {};
    if (api_check_buffer(file)){
        Scratch_Block scratch(app);
        Arena *arena = reserve_arena(models->tctx);
        Face *face = file_get_face(models, file);
        
        Gap_Buffer *buffer = &file->state.buffer;
        
        Vec2_f32 dim = rect_dim(rect);
        
        i64 line_count = buffer_line_count(buffer);
        i64 line_number = buffer_point.line_number;
        f32 y = -buffer_point.pixel_shift.y;
        for (;line_number <= line_count;
             line_number += 1){
            Buffer_Layout_Item_List line = file_get_line_layout(models, file, dim.x, face, line_number);
            f32 next_y = y + line.height;
            if (next_y >= dim.y){
                break;
            }
            y = next_y;
        }
        
        Interval_i64 visible_line_number_range = Ii64(buffer_point.line_number, line_number);
        Interval_i64 visible_range = Ii64(buffer_get_first_pos_from_line_number(buffer, visible_line_number_range.min),
                                          buffer_get_last_pos_from_line_number(buffer, visible_line_number_range.max));
        
        i64 item_count = range_size_inclusive(visible_range);
        int_color *colors_array = push_array(arena, int_color, item_count);
        for (i64 i = 0; i < item_count; i += 1){
            colors_array[i] = Stag_Default;
        }
        result = text_layout_new(&models->text_layouts, arena, buffer_id, buffer_point,
                                 visible_range, visible_line_number_range, rect, colors_array);
    }
    return(result);
}

api(custom) function b32
Text_Layout_Get_Buffer(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID *buffer_id_out){
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    Text_Layout *layout = text_layout_get(&models->text_layouts, text_layout_id);
    if (layout != 0){
        *buffer_id_out = layout->buffer_id;
        result = true;
    }
    return(result);
}

api(custom) function Interval_i64
Text_Layout_Get_Visible_Range(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    Range_i64 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, text_layout_id);
    if (layout != 0){
        result = layout->visible_range;
    }
    return(result);
}

api(custom) function Rect_f32
Text_Layout_Line_On_Screen(Application_Links *app, Text_Layout_ID layout_id, i64 line_number){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0 && range_contains_inclusive(layout->visible_line_number_range, line_number)){
        Editing_File *file = imp_get_file(models, layout->buffer_id);
        if (api_check_buffer(file)){
            Rect_f32 rect = layout->rect;
            f32 width = rect_width(rect);
            Face *face = file_get_face(models, file);
            
            f32 top = 0.f;
            f32 bot = 0.f;
            for (i64 line_number_it = layout->visible_line_number_range.first;;
                 line_number_it += 1){
                Buffer_Layout_Item_List line = file_get_line_layout(models, file, width, face, line_number_it);
                bot += line.height;
                if (line_number_it == line_number){
                    break;
                }
                top = bot;
            }
            
            top -= layout->point.pixel_shift.y;
            bot -= layout->point.pixel_shift.y;
            
            result = Rf32(rect.x0, rect.y0 + top, rect.x1, rect.y0 + bot);
            result = rect_intersect(rect, result);
            
            Vec2_f32 coordinate_center = models_get_coordinate_center(models);
            result.p0 -= coordinate_center;
            result.p1 -= coordinate_center;
        }
    }
    return(result);
}

api(custom) function Rect_f32
Text_Layout_Character_On_Screen(Application_Links *app, Text_Layout_ID layout_id, i64 pos){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0 && range_contains_inclusive(layout->visible_range, pos)){
        Editing_File *file = imp_get_file(models, layout->buffer_id);
        if (api_check_buffer(file)){
            Gap_Buffer *buffer = &file->state.buffer;
            i64 line_number = buffer_get_line_index(buffer, pos) + 1;
            
            Rect_f32 rect = layout->rect;
            f32 width = rect_width(rect);
            Face *face = file_get_face(models, file);
            
            f32 y = 0.f;
            Buffer_Layout_Item_List line = {};
            for (i64 line_number_it = layout->visible_line_number_range.first;;
                 line_number_it += 1){
                line = file_get_line_layout(models, file, width, face, line_number_it);
                if (line_number_it == line_number){
                    break;
                }
                y += line.height;
            }
            
            // TODO(allen): optimization: This is some fairly heavy computation.  We really need
            // to accelerate the (pos -> item) lookup within a single Buffer_Layout_Item_List.
            b32 is_first_item = true;
            result = Rf32_negative_infinity;
            for (Buffer_Layout_Item_Block *block = line.first;
                 block != 0;
                 block = block->next){
                Buffer_Layout_Item *item_ptr = block->items;
                i64 count = block->count;
                for (i32 i = 0; i < count; i += 1, item_ptr += 1){
                    i64 index = item_ptr->index;
                    if (index == pos){
                        result = rect_union(result, item_ptr->rect);
                    }
                    else if (index > pos){
                        break;
                    }
                }
            }
            
            Vec2_f32 shift = V2f32(rect.x0, rect.y0 + y) - layout->point.pixel_shift;
            result.p0 += shift;
            result.p1 += shift;
            result = rect_intersect(rect, result);
            
            Vec2_f32 coordinate_center = models_get_coordinate_center(models);
            result.p0 -= coordinate_center;
            result.p1 -= coordinate_center;
        }
    }
    return(result);
}

api(custom) function void
Paint_Text_Color(Application_Links *app, Text_Layout_ID layout_id, Interval_i64 range, int_color color){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0){
        range.min = clamp_bot(layout->visible_range.min, range.min);
        range.max = clamp_top(range.max, layout->visible_range.max);
        range.min -= layout->visible_range.min;
        range.max -= layout->visible_range.min;
        int_color *color_ptr = layout->item_colors + range.min;
        for (i64 i = range.min; i < range.max; i += 1, color_ptr += 1){
            *color_ptr = color;
        }
    }
}

api(custom) function b32
Text_Layout_Free(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    return(text_layout_erase(models, &models->text_layouts, text_layout_id));
}

api(custom) function void
Draw_Text_Layout(Application_Links *app, Text_Layout_ID layout_id){
    Models *models = (Models*)app->cmd_context;
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0 && models->target != 0){
        text_layout_render(models, layout);
    }
}

api(custom) function void
Open_Color_Picker(Application_Links *app, Color_Picker *picker)
{
    Models *models = (Models*)app->cmd_context;
    System_Functions *system = models->system;
    if (picker->finished){
        *picker->finished = false;
    }
    system->open_color_picker(picker);
}

api(custom) function void
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

api(custom) function String_Match_List
Buffer_Find_All_Matches(Application_Links *app, Arena *arena, Buffer_ID buffer, i32 string_id, Range_i64 range, String_Const_u8 needle, Character_Predicate *predicate, Scan_Direction direction)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match_List list = {};
    if (api_check_buffer(file)){
        if (needle.size > 0){
            Scratch_Block scratch(app);
            List_String_Const_u8 chunks = buffer_get_chunks(scratch, &file->state.buffer);
            buffer_chunks_clamp(&chunks, range);
            if (chunks.node_count > 0){
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

// BOTTOM
