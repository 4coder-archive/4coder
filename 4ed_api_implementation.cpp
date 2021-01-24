/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.????
 *
 * Implementation of the API functions.
 *
 */

// TOP

function void
output_file_append(Thread_Context *tctx, Models *models, Editing_File *file, String_Const_u8 value){
    i64 end = buffer_size(&file->state.buffer);
    Edit_Behaviors behaviors = {};
    behaviors.pos_before_edit = end;
    edit_single(tctx, models, file, Ii64(end), value, behaviors);
}

function void
file_cursor_to_end(Thread_Context *tctx, Models *models, Editing_File *file){
    Assert(file != 0);
    i64 pos = buffer_size(&file->state.buffer);
    Layout *layout = &models->layout;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file != file){
            continue;
        }
        view_set_cursor(tctx, models, view, pos);
        view->mark = pos;
    }
}

////////////////////////////////

function b32
access_test(Access_Flag object_flags, Access_Flag access_flags){
    return((object_flags & access_flags) == access_flags);
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
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *info = (Thread_Context_Extra_Info*)tctx->user_data;
    return(info->coroutine != 0);
}

api(custom) function b32
global_set_setting(Application_Links *app, Global_Setting_ID setting, i64 value){
    Models *models = (Models*)app->cmd_context;
    b32 result = true;
    switch (setting){
        case GlobalSetting_LAltLCtrlIsAltGr:
        {
            models->settings.lctrl_lalt_is_altgr = (b32)(value != 0);
        }break;
        default:
        {
            result = false;
        }break;
    }
    return(result);
}

api(custom) function Rect_f32
global_get_screen_rectangle(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(Rf32(V2f32(0, 0), V2f32(layout_get_root_size(&models->layout))));
}

api(custom) function Thread_Context*
get_thread_context(Application_Links *app){
    return(app->tctx);
}

api(custom) function Child_Process_ID
create_child_process(Application_Links *app, String_Const_u8 path, String_Const_u8 command){
    Models *models = (Models*)app->cmd_context;
    Child_Process_ID result = 0;
    if (!child_process_call(app->tctx, models, path, command, &result)){
        result = 0;
    }
    return(result);
}

api(custom) function b32
child_process_set_target_buffer(Application_Links *app, Child_Process_ID child_process_id, Buffer_ID buffer_id, Child_Process_Set_Target_Flags flags){
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
buffer_get_attached_child_process(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Child_Process_ID result = 0;
    if (api_check_buffer(file)){
        result = file->state.attached_child_process;
    }
    return(result);
}

api(custom) function Buffer_ID
child_process_get_attached_buffer(Application_Links *app, Child_Process_ID child_process_id){
    Models *models = (Models*)app->cmd_context;
    Child_Process *child_process = child_process_from_id(&models->child_processes, child_process_id);
    Buffer_ID result = 0;
    if (child_process != 0 && child_process->out_file != 0){
        result = child_process->out_file->id;
    }
    return(result);
}

api(custom) function Process_State
child_process_get_state(Application_Links *app, Child_Process_ID child_process_id){
    Models *models = (Models*)app->cmd_context;
    return(child_process_get_state(&models->child_processes, child_process_id));
}

api(custom) function b32
enqueue_virtual_event(Application_Links *app, Input_Event *event){
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    if (InputEventKind_None < event->kind && event->kind < InputEventKind_COUNT){
        models_push_virtual_event(models, event);
    }
    return(result);
}

api(custom) function i32
get_buffer_count(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    return(working_set->active_file_count);
}

api(custom) function Buffer_ID
get_buffer_next(Application_Links *app, Buffer_ID buffer_id, Access_Flag access)
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
get_buffer_by_name(Application_Links *app, String_Const_u8 name, Access_Flag access)
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
get_buffer_by_file_name(Application_Links *app, String_Const_u8 file_name, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File_Name canon = {};
    Buffer_ID result = false;
    Scratch_Block scratch(app);
    if (get_canon_name(scratch, file_name, &canon)){
        Working_Set *working_set = &models->working_set;
        Editing_File *file = working_set_contains_canon(working_set, string_from_file_name(&canon));
        if (api_check_buffer(file, access)){
            result = file->id;
        }
    }
    return(result);
}

api(custom) function b32
buffer_read_range(Application_Links *app, Buffer_ID buffer_id, Range_i64 range, u8 *out)
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

function Edit_Behaviors
get_active_edit_behaviors(Models *models, Editing_File *file){
    Panel *panel = layout_get_active_panel(&models->layout);
    Assert(panel != 0);
    View *view = panel->view;
    Assert(view != 0);
    Edit_Behaviors behaviors = {};
    if (view->file == file){
        behaviors.pos_before_edit = view->edit_pos_.cursor_pos;
    }
    else{
        behaviors.pos_before_edit = -1;
    }
    return(behaviors);
}

api(custom) function b32
buffer_replace_range(Application_Links *app, Buffer_ID buffer_id, Range_i64 range, String_Const_u8 string)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        i64 size = buffer_size(&file->state.buffer);
        if (0 <= range.first && range.first <= range.one_past_last && range.one_past_last <= size){
            Edit_Behaviors behaviors = get_active_edit_behaviors(models, file);
            edit_single(app->tctx, models, file, range, string, behaviors);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
buffer_batch_edit(Application_Links *app, Buffer_ID buffer_id, Batch_Edit *batch)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        Edit_Behaviors behaviors = get_active_edit_behaviors(models, file);
        result = edit_batch(app->tctx, models, file, batch, behaviors);
    }
    return(result);
}

api(custom) function String_Match
buffer_seek_string(Application_Links *app, Buffer_ID buffer, String_Const_u8 needle, Scan_Direction direction, i64 start_pos){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match result = {};
    if (api_check_buffer(file)){
        if (needle.size == 0){
            result.flags = StringMatch_CaseSensitive;
            result.range = Ii64(start_pos);
        }
        else{
            Scratch_Block scratch(app);
            Gap_Buffer *gap_buffer = &file->state.buffer;
            i64 size = buffer_size(gap_buffer);
            List_String_Const_u8 chunks = buffer_get_chunks(scratch, gap_buffer);
            Range_i64 range = {};
            if (direction == Scan_Forward){
                i64 adjusted_pos = start_pos + 1;
                start_pos = clamp_top(adjusted_pos, size);
                range = Ii64(adjusted_pos, size);
            }
            else{
                i64 adjusted_pos = start_pos - 1 + needle.size;
                start_pos = clamp_bot(0, adjusted_pos);
                range = Ii64(0, adjusted_pos);
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
buffer_seek_character_class(Application_Links *app, Buffer_ID buffer, Character_Predicate *predicate, Scan_Direction direction, i64 start_pos){
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
                    result.range = Ii64(size);
                    break;
                }
                u8 v = chunks.vals[pos.chunk_index].str[pos.chunk_pos];
                if (character_predicate_check_character(*predicate, v)){
                    result.buffer = buffer;
                    result.range = Ii64(pos.real_pos, pos.real_pos + 1);
                    break;
                }
            }
        }
    }
    return(result);
}

api(custom) function f32
buffer_line_y_difference(Application_Links *app, Buffer_ID buffer_id,
                         f32 width, Face_ID face_id,
                         i64 line_a, i64 line_b){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    f32 result = 0.0f;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_line_y_difference(app->tctx, models, file,
                                            layout_func, width, face,
                                            line_a, line_b);
        }
    }
    return(result);
}

api(custom) function Line_Shift_Vertical
buffer_line_shift_y(Application_Links *app, Buffer_ID buffer_id,
                    f32 width, Face_ID face_id,
                    i64 line, f32 y_shift){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Line_Shift_Vertical result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_line_shift_y(app->tctx, models, file,
                                       layout_func, width, face,
                                       line, y_shift);
        }
    }
    return(result);
}

api(custom) function i64
buffer_pos_at_relative_xy(Application_Links *app, Buffer_ID buffer_id,
                          f32 width, Face_ID face_id,
                          i64 base_line, Vec2_f32 relative_xy){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = -1;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_pos_at_relative_xy(app->tctx, models, file,
                                             layout_func, width, face,
                                             base_line, relative_xy);
        }
    }
    return(result);
}

api(custom) function Rect_f32
buffer_relative_box_of_pos(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Rect_f32 result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_relative_box_of_pos(app->tctx, models, file,
                                              layout_func, width, face,
                                              base_line, pos);
        }
    }
    return(result);
}

api(custom) function Rect_f32
buffer_padded_box_of_pos(Application_Links *app, Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Rect_f32 result = {};
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_padded_box_of_pos(app->tctx, models, file,
                                            layout_func, width, face,
                                            base_line, pos);
        }
    }
    return(result);
}

api(custom) function i64
buffer_relative_character_from_pos(Application_Links *app, Buffer_ID buffer_id,
                                   f32 width, Face_ID face_id, i64 base_line, i64 pos)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = 0;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_relative_character_from_pos(app->tctx, models, file,
                                                      layout_func, width, face,
                                                      base_line, pos);
        }
    }
    return(result);
}

api(custom) function i64
buffer_pos_from_relative_character(Application_Links *app,  Buffer_ID buffer_id, f32 width, Face_ID face_id, i64 base_line, i64 relative_character)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = -1;
    if (api_check_buffer(file)){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            Layout_Function *layout_func = file_get_layout_func(file);
            result = file_pos_from_relative_character(app->tctx, models, file,
                                                      layout_func, width, face,
                                                      base_line, relative_character);
        }
    }
    return(result);
}


api(custom) function f32
view_line_y_difference(Application_Links *app, View_ID view_id, i64 line_a, i64 line_b){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    f32 result = 0.0f;
    if (api_check_view(view)){
        result = view_line_y_difference(app->tctx, models, view, line_a, line_b);
    }
    return(result);
}

api(custom) function Line_Shift_Vertical
view_line_shift_y(Application_Links *app, View_ID view_id, i64 line, f32 y_shift){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Line_Shift_Vertical result = {};
    if (api_check_view(view)){
        result = view_line_shift_y(app->tctx, models, view, line, y_shift);
    }
    return(result);
}

api(custom) function i64
view_pos_at_relative_xy(Application_Links *app, View_ID view_id, i64 base_line, Vec2_f32 relative_xy){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = -1;
    if (api_check_view(view)){
        result = view_pos_at_relative_xy(app->tctx, models, view, base_line, relative_xy);
    }
    return(result);
}

api(custom) function Rect_f32
view_relative_box_of_pos(Application_Links *app, View_ID view_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Rect_f32 result = {};
    if (api_check_view(view)){
        result = view_relative_box_of_pos(app->tctx, models, view, base_line, pos);
    }
    return(result);
}

api(custom) function Rect_f32
view_padded_box_of_pos(Application_Links *app, View_ID view_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Rect_f32 result = {};
    if (api_check_view(view)){
        result = view_padded_box_of_pos(app->tctx, models, view, base_line, pos);
    }
    return(result);
}

api(custom) function i64
view_relative_character_from_pos(Application_Links *app,  View_ID view_id, i64 base_line, i64 pos){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = 0;
    if (api_check_view(view)){
        result = view_relative_character_from_pos(app->tctx, models, view, base_line, pos);
    }
    return(result);
}

api(custom) function i64
view_pos_from_relative_character(Application_Links *app,  View_ID view_id, i64 base_line, i64 character){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = 0;
    if (api_check_view(view)){
        result = view_pos_from_relative_character(app->tctx, models, view, base_line, character);
    }
    return(result);
}

api(custom) function b32
buffer_exists(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    return(api_check_buffer(file));
}

api(custom) function Access_Flag
buffer_get_access_flags(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Access_Flag result = 0;
    if (api_check_buffer(file)){
        result = file_get_access_flags(file);
    }
    return(result);
}

api(custom) function i64
buffer_get_size(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    i64 result = 0;
    if (api_check_buffer(file)){
        result = buffer_size(&file->state.buffer);
    }
    return(result);
}

api(custom) function i64
buffer_get_line_count(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    u64 result = 0;
    if (api_check_buffer(file)){
        result = buffer_line_count(&file->state.buffer);
    }
    return(result);
}

api(custom) function String_Const_u8
push_buffer_base_name(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(arena, string_from_file_name(&file->base_name));
    }
    return(result);
}

api(custom) function String_Const_u8
push_buffer_unique_name(Application_Links *app, Arena *out, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(out, string_from_file_name(&file->unique_name));
    }
    return(result);
}

api(custom) function String_Const_u8
push_buffer_file_name(Application_Links *app, Arena *arena, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    String_Const_u8 result = {};
    if (api_check_buffer(file)){
        result = push_string_copy(arena, string_from_file_name(&file->canon));
    }
    return(result);
}

api(custom) function Dirty_State
buffer_get_dirty_state(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Dirty_State result = 0;
    if (api_check_buffer(file)){
        result = file->state.dirty;
    }
    return(result);
}

api(custom) function b32
buffer_set_dirty_state(Application_Links *app, Buffer_ID buffer_id, Dirty_State dirty_state){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        file->state.dirty = dirty_state;
    }
    return(result);
}

api(custom) function b32
buffer_set_layout(Application_Links *app, Buffer_ID buffer_id, Layout_Function *layout_func){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        file->settings.layout_func = layout_func;
        file_clear_layout_cache(file);
    }
    return(result);
}

api(custom) function b32
buffer_clear_layout_cache(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        file_clear_layout_cache(file);
    }
    return(result);
}

api(custom) function Layout_Function*
buffer_get_layout(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Layout_Function *result = 0;
    if (api_check_buffer(file)){
        result = file->settings.layout_func;
    }
    return(result);
}

api(custom) function b32
buffer_get_setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64 *value_out)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        switch (setting){
            case BufferSetting_Unimportant:
            {
                *value_out = file->settings.unimportant;
            }break;
            
            case BufferSetting_Unkillable:
            {
                *value_out = (file->settings.never_kill || file->settings.unkillable);
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
buffer_set_setting(Application_Links *app, Buffer_ID buffer_id, Buffer_Setting_ID setting, i64 value)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = true;
        switch (setting){
            case BufferSetting_Unimportant:
            {
                if (value != 0){
                    file_set_unimportant(file, true);
                }
                else{
                    file_set_unimportant(file, false);
                }
            }break;
            
            case BufferSetting_Unkillable:
            {
                file->settings.unkillable = (value != 0);
            }break;
            
            case BufferSetting_ReadOnly:
            {
                file->settings.read_only = (value != 0);
            }break;
            
            case BufferSetting_RecordsHistory:
            {
                if (value){
                    if (!history_is_activated(&file->state.history)){
                        history_init(app->tctx, models, &file->state.history);
                    }
                }
                else{
                    if (history_is_activated(&file->state.history)){
                        history_free(app->tctx, &file->state.history);
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
buffer_get_managed_scope(Application_Links *app, Buffer_ID buffer_id)
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
buffer_send_end_signal(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        file_end_file(app->tctx, models, file);
        result = true;
    }
    return(result);
}

api(custom) function Buffer_ID
create_buffer(Application_Links *app, String_Const_u8 file_name, Buffer_Create_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *new_file = create_file(app->tctx, models, file_name, flags);
    Buffer_ID result = 0;
    if (new_file != 0){
        result = new_file->id;
    }
    return(result);
}

api(custom) function b32
buffer_save(Application_Links *app, Buffer_ID buffer_id, String_Const_u8 file_name, Buffer_Save_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
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
            Thread_Context *tctx = app->tctx;
            Scratch_Block scratch(tctx);
            String_Const_u8 name = push_string_copy(scratch, file_name);
            save_file_to_name(tctx, models, file, name.str);
            result = true;
        }
    }
    
    return(result);
}

api(custom) function Buffer_Kill_Result
buffer_kill(Application_Links *app, Buffer_ID buffer_id, Buffer_Kill_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    Working_Set *working_set = &models->working_set;
    Editing_File *file = imp_get_file(models, buffer_id);
    Buffer_Kill_Result result = BufferKillResult_DoesNotExist;
    if (api_check_buffer(file)){
        if (!file->settings.never_kill && !file->settings.unkillable){
            b32 needs_to_save = file_needs_save(file);
            if (!needs_to_save || (flags & BufferKill_AlwaysKill) != 0){
                Thread_Context *tctx = app->tctx;
                if (models->end_buffer != 0){
                    models->end_buffer(app, file->id);
                }
                
                buffer_unbind_name_low_level(working_set, file);
                if (file->canon.name_size != 0){
                    buffer_unbind_file(working_set, file);
                }
                file_free(tctx, models, file);
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
                        view_set_file(tctx, models, view, new_file);
                        file_node = file_node->next;
                        if (file_node == order){
                            file_node = file_node->next;
                        }
                        Assert(file_node != order);
                    }
                }
                
                Child_Process_Container *child_processes = &models->child_processes;
                for (Node *node = child_processes->child_process_active_list.next;
                     node != &child_processes->child_process_active_list;
                     node = node->next){
                    Child_Process *child_process = CastFromMember(Child_Process, node, node);
                    if (child_process->out_file == file){
                        child_process->out_file = 0;
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
buffer_reopen(Application_Links *app, Buffer_ID buffer_id, Buffer_Reopen_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    Thread_Context *tctx = app->tctx;
    Scratch_Block scratch(tctx);
    Editing_File *file = imp_get_file(models, buffer_id);
    Buffer_Reopen_Result result = BufferReopenResult_Failed;
    if (api_check_buffer(file)){
        if (file->canon.name_size > 0){
            Plat_Handle handle = {};
            if (system_load_handle(scratch, (char*)file->canon.name_space, &handle)){
                File_Attributes attributes = system_load_attributes(handle);
                
                char *file_memory = push_array(scratch, char, (i32)attributes.size);
                
                if (file_memory != 0){
                    if (system_load_file(handle, file_memory, (i32)attributes.size)){
                        system_load_close(handle);
                        
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
                        file_free(tctx, models, file);
                        working_set_file_default_settings(working_set, file);
                        file_create_from_string(tctx, models, file, SCu8(file_memory, attributes.size), attributes);
                        
                        for (i32 i = 0; i < vptr_count; ++i){
                            view_set_file(tctx, models, vptrs[i], file);
                            
                            vptrs[i]->file = file;
                            i64 line = line_numbers[i];
                            i64 col = column_numbers[i];
                            Buffer_Cursor cursor = file_compute_cursor(file, seek_line_col(line, col));
                            view_set_cursor(tctx, models, vptrs[i], cursor.pos);
                        }
                        result = BufferReopenResult_Reopened;
                    }
                    else{
                        system_load_close(handle);
                    }
                }
                else{
                    system_load_close(handle);
                }
            }
        }
    }
    return(result);
}

api(custom) function File_Attributes
buffer_get_file_attributes(Application_Links *app, Buffer_ID buffer_id)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    File_Attributes result = {};
    if (api_check_buffer(file)){
        block_copy_struct(&result, &file->attributes);
    }
    return(result);
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
        Panel *panel = layout_get_last_open_panel(layout);
        view = panel->view;
    }
    return(view);
}

api(custom) function View_ID
get_view_next(Application_Links *app, View_ID view_id, Access_Flag access)
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
        result = view_get_id(&models->view_set, view);
    }
    return(result);
}

api(custom) function View_ID
get_view_prev(Application_Links *app, View_ID view_id, Access_Flag access)
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
        result = view_get_id(&models->view_set, view);
    }
    return(result);
}

api(custom) function View_ID
get_this_ctx_view(Application_Links *app, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *tctx_info = (Thread_Context_Extra_Info*)tctx->user_data;
    View_ID result = 0;
    if (tctx_info->coroutine != 0){
        Coroutine *coroutine = (Coroutine*)tctx_info->coroutine;
        View *view = (View*)coroutine->user_data;
        if (view != 0){
            result = view_get_id(&models->view_set, view);
        }
    }
    return(result);
}

api(custom) function View_ID
get_active_view(Application_Links *app, Access_Flag access)
{
    Models *models = (Models*)app->cmd_context;
    Panel *panel = layout_get_active_panel(&models->layout);
    Assert(panel != 0);
    View *view = panel->view;
    Assert(view != 0);
    View_ID result = 0;
    if (api_check_view(view, access)){
        result = view_get_id(&models->view_set, view);
    }
    return(result);
}

api(custom) function b32
view_exists(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = true;
    }
    return(result);
}

api(custom) function Buffer_ID
view_get_buffer(Application_Links *app, View_ID view_id, Access_Flag access){
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
view_get_cursor_pos(Application_Links *app, View_ID view_id){
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
view_get_mark_pos(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    i64 result = 0;;
    if (api_check_view(view)){
        result = view->mark;
    }
    return(result);
}

api(custom) function f32
view_get_preferred_x(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    f32 result = 0.f;;
    if (api_check_view(view)){
        result = view->preferred_x;
    }
    return(result);
}

api(custom) function b32
view_set_preferred_x(Application_Links *app, View_ID view_id, f32 x){
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
view_get_screen_rect(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        result = Rf32(view->panel->rect_full);
    }
    return(result);
}

api(custom) function Panel_ID
view_get_panel(Application_Links *app, View_ID view_id){
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
panel_get_view(Application_Links *app, Panel_ID panel_id, Access_Flag access){
    Models *models = (Models*)app->cmd_context;
    Panel *panel = imp_get_panel(models, panel_id);
    View_ID result = 0;
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Final){
            View *view = panel->view;
            if (api_check_view(view, access)){
                result = view_get_id(&models->view_set, view);
            }
        }
    }
    return(result);
}

api(custom) function b32
panel_is_split(Application_Links *app, Panel_ID panel_id){
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
panel_is_leaf(Application_Links *app, Panel_ID panel_id){
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
panel_split(Application_Links *app, Panel_ID panel_id, Dimension split_dim){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    b32 result = false;
    Panel *panel = imp_get_panel(models, panel_id);
    if (api_check_panel(panel)){
        Panel *new_panel = 0;
        if (layout_split_panel(layout, panel, (split_dim == Dimension_X), &new_panel)){
            Live_Views *view_set = &models->view_set;
            View *new_view = live_set_alloc_view(&models->lifetime_allocator, view_set, new_panel);
            view_init(app->tctx, models, new_view, models->scratch_buffer,
                      models->view_event_handler);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
panel_set_split(Application_Links *app, Panel_ID panel_id, Panel_Split_Kind kind,
                f32 t){
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
                    panel->split.v_i32 = i32_round32(t);
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
panel_swap_children(Application_Links *app, Panel_ID panel_id){
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
panel_get_root(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    Panel *panel = layout->root;
    return(panel_get_id(layout, panel));
}

api(custom) function Panel_ID
panel_get_parent(Application_Links *app, Panel_ID panel_id){
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
panel_get_child(Application_Links *app, Panel_ID panel_id, Side which_child){
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    Panel *panel = imp_get_panel(models, panel_id);
    Panel_ID result = 0;
    if (api_check_panel(panel)){
        if (panel->kind == PanelKind_Intermediate){
            Panel *child = 0;
            switch (which_child){
                case Side_Min:
                {
                    child = panel->tl_panel;
                }break;
                case Side_Max:
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

api(custom) function b32
view_close(Application_Links *app, View_ID view_id)
{
    Models *models = (Models*)app->cmd_context;
    Layout *layout = &models->layout;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = view_close(models, view);
    }
    return(result);
}

api(custom) function Rect_f32
view_get_buffer_region(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Rect_f32 result = {};
    if (api_check_view(view)){
        result = view_get_buffer_rect(app->tctx, models, view);
    }
    return(result);
}

api(custom) function Buffer_Scroll
view_get_buffer_scroll(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    Buffer_Scroll  result = {};
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        File_Edit_Positions edit_pos = view_get_edit_pos(view);
        result = edit_pos.scroll;
    }
    return(result);
}

api(custom) function b32
view_set_active(Application_Links *app, View_ID view_id)
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
view_enqueue_command_function(Application_Links *app, View_ID view_id, Custom_Command_Function *custom_func)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        models_push_view_command_function(models, view_id, custom_func);
        result = true;
    }
    return(result);
}

api(custom) function b32
view_get_setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i64 *value_out)
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
            
            default:
            {
                result = false;
            }break;
        }
    }
    return(result);
}

api(custom) function b32
view_set_setting(Application_Links *app, View_ID view_id, View_Setting_ID setting, i64 value)
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
            
            default:
            {
                result = false;
            }break;
        }
    }
    return(result);
}

api(custom) function Managed_Scope
view_get_managed_scope(Application_Links *app, View_ID view_id)
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
buffer_compute_cursor(Application_Links *app, Buffer_ID buffer, Buffer_Seek seek)
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
view_compute_cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    Buffer_Cursor result = {};
    if (api_check_view(view)){
        result = view_compute_cursor(view, seek);
    }
    return(result);
}

api(custom) function b32
view_set_camera_bounds(Application_Links *app, View_ID view_id, Vec2_f32 margin, Vec2_f32 push_in_multiplier)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = true;
        margin.x = clamp_bot(0.f, margin.x);
        margin.y = clamp_bot(0.f, margin.y);
        push_in_multiplier.x = clamp_bot(1.5f, push_in_multiplier.x);
        push_in_multiplier.y = clamp_bot(1.5f, push_in_multiplier.y);
        view->cursor_margin = margin;
        view->cursor_push_in_multiplier = push_in_multiplier;
    }
    return(result);
}

api(custom) function b32
view_get_camera_bounds(Application_Links *app, View_ID view_id, Vec2_f32 *margin, Vec2_f32 *push_in_multiplier)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        result = true;
        *margin = view->cursor_margin;
        *push_in_multiplier = view->cursor_push_in_multiplier;
    }
    return(result);
}

api(custom) function b32
view_set_cursor(Application_Links *app, View_ID view_id, Buffer_Seek seek)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = view->file;
        Assert(file != 0);
        if (api_check_buffer(file)){
            Buffer_Cursor cursor = file_compute_cursor(file, seek);
            view_set_cursor(app->tctx, models, view, cursor.pos);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
view_set_buffer_scroll(Application_Links *app, View_ID view_id, Buffer_Scroll scroll,
                       Set_Buffer_Scroll_Rule rule)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        Thread_Context *tctx = app->tctx;
        scroll.position = view_normalize_buffer_point(tctx, models, view, scroll.position);
        scroll.target = view_normalize_buffer_point(tctx, models, view, scroll.target);
        scroll.target.pixel_shift.x = f32_round32(scroll.target.pixel_shift.x);
        scroll.target.pixel_shift.y = f32_round32(scroll.target.pixel_shift.y);
        scroll.target.pixel_shift.x = clamp_bot(0.f, scroll.target.pixel_shift.x);
        Layout_Item_List line = view_get_line_layout(tctx, models, view,
                                                     scroll.target.line_number);
        scroll.target.pixel_shift.y =
            clamp(0.f, scroll.target.pixel_shift.y, line.height);
        if (rule == SetBufferScroll_SnapCursorIntoView){
            view_set_scroll(tctx, models, view, scroll);
        }
        else{
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            edit_pos.scroll = scroll;
            view_set_edit_pos(view, edit_pos);
        }
        view->new_scroll_target = true;
        result = true;
    }
    return(result);
}

api(custom) function b32
view_set_mark(Application_Links *app, View_ID view_id, Buffer_Seek seek)
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
view_quit_ui(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (view != 0){
        view_quit_ui(app->tctx, models, view);
        result = true;
    }
    return(result);
}

api(custom) function b32
view_set_buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer_id, Set_Buffer_Flag flags)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        Editing_File *file = working_set_get_file(&models->working_set, buffer_id);
        if (api_check_buffer(file)){
            if (file != view->file){
                view_set_file(app->tctx, models, view, file);
                if (!(flags & SetBuffer_KeepOriginalGUI)){
                    view_quit_ui(app->tctx, models, view);
                }
            }
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
view_push_context(Application_Links *app, View_ID view_id, View_Context *ctx){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        view_push_context(view, ctx);
        result = true;
    }
    return(result);
}

api(custom) function b32
view_pop_context(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        view_pop_context(view);
        result = true;
    }
    return(result);
}

api(custom) function b32
view_alter_context(Application_Links *app, View_ID view_id, View_Context *ctx){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
        view_alter_context(view, ctx);
        result = true;
    }
    return(result);
}

api(custom) function View_Context
view_current_context(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    View_Context result = {};
    if (api_check_view(view)){
        result = view_current_context(view);
    }
    return(result);
}

api(custom) function String_Const_u8
view_current_context_hook_memory(Application_Links *app, View_ID view_id,
                                 Hook_ID hook_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    String_Const_u8 result = {};
    if (api_check_view(view)){
        View_Context_Node *ctx = view_current_context_node(view);
        if (ctx != 0){
            switch (hook_id){
                case HookID_DeltaRule:
                {
                    result = make_data(ctx->delta_rule_memory,
                                       ctx->ctx.delta_rule_memory_size);
                }break;
            }
        }
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
create_user_managed_scope(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    Lifetime_Object *object = lifetime_alloc_object(&models->lifetime_allocator, DynamicWorkspace_Unassociated, 0);
    object->workspace.user_back_ptr = object;
    Managed_Scope scope = (Managed_Scope)object->workspace.scope_id;
    return(scope);
}

api(custom) function b32
destroy_user_managed_scope(Application_Links *app, Managed_Scope scope)
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
get_global_managed_scope(Application_Links *app)
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
get_managed_scope_with_multiple_dependencies(Application_Links *app, Managed_Scope *scopes, i32 count)
{
    Models *models = (Models*)app->cmd_context;
    Lifetime_Allocator *lifetime_allocator = &models->lifetime_allocator;
    
    Scratch_Block scratch(app);
    
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
managed_scope_clear_contents(Application_Links *app, Managed_Scope scope)
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
managed_scope_clear_self_all_dependent_scopes(Application_Links *app, Managed_Scope scope)
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
managed_scope_allocator(Application_Links *app, Managed_Scope scope)
{
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    Base_Allocator *result = 0;
    if (workspace != 0){
        result = &workspace->heap_wrapper;
    }
    return(result);
}

api(custom) function u64
managed_id_group_highest_id(Application_Links *app, String_Const_u8 group){
    Models *models = (Models*)app->cmd_context;
    Managed_ID_Set *set = &models->managed_id_set;
    return(managed_ids_group_highest_id(set, group));
}

api(custom) function Managed_ID
managed_id_declare(Application_Links *app, String_Const_u8 group, String_Const_u8 name)
{
    Models *models = (Models*)app->cmd_context;
    Managed_ID_Set *set = &models->managed_id_set;
    return(managed_ids_declare(set, group, name));
}

api(custom) function Managed_ID
managed_id_get(Application_Links *app, String_Const_u8 group, String_Const_u8 name){
    Models *models = (Models*)app->cmd_context;
    Managed_ID_Set *set = &models->managed_id_set;
    return(managed_ids_get(set, group, name));
}

api(custom) function void*
managed_scope_get_attachment(Application_Links *app, Managed_Scope scope, Managed_ID id, u64 size){
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    void *result = 0;
    if (workspace != 0){
        Dynamic_Variable_Block *var_block = &workspace->var_block;
        String_Const_u8 data = dynamic_variable_get(var_block, id, size);
        if (data.size >= size){
            result = data.str;
        }
        else{
#define M \
"ERROR: scope attachment already exists with a size smaller than the requested size; no attachment pointer can be returned."
            print_message(app, string_u8_litexpr(M));
#undef M
        }
    }
    return(result);
}

api(custom) function b32
managed_scope_attachment_erase(Application_Links *app, Managed_Scope scope, Managed_ID id){
    Models *models = (Models*)app->cmd_context;
    Dynamic_Workspace *workspace = get_dynamic_workspace(models, scope);
    b32 result = false;
    if (workspace != 0){
        Dynamic_Variable_Block *var_block = &workspace->var_block;
        dynamic_variable_erase(var_block, id);
        result = true;
    }
    return(result);
}

api(custom) function Managed_Object
alloc_managed_memory_in_scope(Application_Links *app, Managed_Scope scope, i32 item_size, i32 count)
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
alloc_buffer_markers_on_buffer(Application_Links *app, Buffer_ID buffer_id, i32 count, Managed_Scope *optional_extra_scope)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Managed_Scope markers_scope = file_get_managed_scope(file);
    if (optional_extra_scope != 0){
        Managed_Object scope_array[2];
        scope_array[0] = markers_scope;
        scope_array[1] = *optional_extra_scope;
        markers_scope = get_managed_scope_with_multiple_dependencies(app, scope_array, 2);
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
managed_object_get_item_size(Application_Links *app, Managed_Object object)
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
managed_object_get_item_count(Application_Links *app, Managed_Object object)
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
managed_object_get_pointer(Application_Links *app, Managed_Object object)
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    return(get_dynamic_object_memory_ptr(object_ptrs.header));
}

api(custom) function Managed_Object_Type
managed_object_get_type(Application_Links *app, Managed_Object object)
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
managed_object_get_containing_scope(Application_Links *app, Managed_Object object)
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
managed_object_free(Application_Links *app, Managed_Object object)
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
managed_object_store_data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem)
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u8 *ptr = get_dynamic_object_memory_ptr(object_ptrs.header);
    b32 result = false;
    if (ptr != 0){
        u32 item_count = object_ptrs.header->count;
        if (0 <= first_index && first_index + count <= item_count){
            u32 item_size = object_ptrs.header->item_size;
            block_copy(ptr + first_index*item_size, mem, count*item_size);
            heap_assert_good(&object_ptrs.workspace->heap);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
managed_object_load_data(Application_Links *app, Managed_Object object, u32 first_index, u32 count, void *mem_out)
{
    Models *models = (Models*)app->cmd_context;
    Managed_Object_Ptr_And_Workspace object_ptrs = get_dynamic_object_ptrs(models, object);
    u8 *ptr = get_dynamic_object_memory_ptr(object_ptrs.header);
    b32 result = false;
    if (ptr != 0){
        u32 item_count = object_ptrs.header->count;
        if (0 <= first_index && first_index + count <= item_count){
            u32 item_size = object_ptrs.header->item_size;
            block_copy(mem_out, ptr + first_index*item_size, count*item_size);
            heap_assert_good(&object_ptrs.workspace->heap);
            result = true;
        }
    }
    return(result);
}

api(custom) function User_Input
get_next_input_raw(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *tctx_info = (Thread_Context_Extra_Info*)tctx->user_data;
    User_Input result = {};
    if (tctx_info->coroutine != 0){
        Coroutine *coroutine = (Coroutine*)tctx_info->coroutine;
        Co_Out *out = (Co_Out*)coroutine->out;
        out->request = CoRequest_None;
        coroutine_yield(coroutine);
        Co_In *in = (Co_In*)coroutine->in;
        result = in->user_input;
    }
    else{
#define M "ERROR: get_next_input called in a hook that may not make calls to blocking APIs"
        print_message(app, string_u8_litexpr(M));
#undef M
    }
    return(result);
}

api(custom) function i64
get_current_input_sequence_number(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return(models->current_input_sequence_number);
}

api(custom) function User_Input
get_current_input(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return(models->current_input);
}

api(custom) function void
set_current_input(Application_Links *app, User_Input *input)
{
    Models *models = (Models*)app->cmd_context;
    block_copy_struct(&models->current_input, input);
}

api(custom) function void
leave_current_input_unhandled(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    models->current_input_unhandled = true;
}

api(custom) function void
set_custom_hook(Application_Links *app, Hook_ID hook_id, Void_Func *func_ptr){
    Models *models = (Models*)app->cmd_context;
    switch (hook_id){
        case HookID_BufferViewerUpdate:
        {
            models->buffer_viewer_update = (Hook_Function*)func_ptr;
        }break;
        case HookID_DeltaRule:
        {
            models->delta_rule = (Delta_Rule_Function*)func_ptr;
        }break;
        case HookID_ViewEventHandler:
        {
            models->view_event_handler = (Custom_Command_Function*)func_ptr;
        }break;
        case HookID_Tick:
        {
            models->tick = (Tick_Function*)func_ptr;
        }break;
        case HookID_RenderCaller:
        {
            models->render_caller = (Render_Caller_Function*)func_ptr;
        }break;
        case HookID_WholeScreenRenderCaller:
        {
            models->whole_screen_render_caller = (Whole_Screen_Render_Caller_Function*)func_ptr;
        }break;
        case HookID_BufferNameResolver:
        {
            models->buffer_name_resolver = (Buffer_Name_Resolver_Function*)func_ptr;
        }break;
        case HookID_BeginBuffer:
        {
            models->begin_buffer = (Buffer_Hook_Function*)func_ptr;
        }break;
        case HookID_EndBuffer:
        {
            models->end_buffer = (Buffer_Hook_Function*)func_ptr;
        }break;
        case HookID_NewFile:
        {
            models->new_file = (Buffer_Hook_Function*)func_ptr;
        }break;
        case HookID_SaveFile:
        {
            models->save_file = (Buffer_Hook_Function*)func_ptr;
        }break;
        case HookID_BufferEditRange:
        {
            models->buffer_edit_range = (Buffer_Edit_Range_Function*)func_ptr;
        }break;
        case HookID_BufferRegion:
        {
            models->buffer_region = (Buffer_Region_Function*)func_ptr;
        }break;
        case HookID_Layout:
        {
            models->layout_func = (Layout_Function*)func_ptr;
        }break;
        case HookID_ViewChangeBuffer:
        {
            models->view_change_buffer = (View_Change_Buffer_Function*)func_ptr;
        }break;
    }
}

api(custom) function Void_Func*
get_custom_hook(Application_Links *app, Hook_ID hook_id){
    Void_Func *result = 0;
    Models *models = (Models*)app->cmd_context;
    switch (hook_id){
        case HookID_BufferViewerUpdate:
        {
            result = (Void_Func*)models->buffer_viewer_update;
        }break;
        case HookID_DeltaRule:
        {
            result = (Void_Func*)models->delta_rule;
        }break;
        case HookID_ViewEventHandler:
        {
            result = (Void_Func*)models->view_event_handler;
        }break;
        case HookID_Tick:
        {
            result = (Void_Func*)models->tick;
        }break;
        case HookID_RenderCaller:
        {
            result = (Void_Func*)models->render_caller;
        }break;
        case HookID_WholeScreenRenderCaller:
        {
            result = (Void_Func*)models->whole_screen_render_caller;
        }break;
        case HookID_BufferNameResolver:
        {
            result = (Void_Func*)models->buffer_name_resolver;
        }break;
        case HookID_BeginBuffer:
        {
            result = (Void_Func*)models->begin_buffer;
        }break;
        case HookID_EndBuffer:
        {
            result = (Void_Func*)models->end_buffer;
        }break;
        case HookID_NewFile:
        {
            result = (Void_Func*)models->new_file;
        }break;
        case HookID_SaveFile:
        {
            result = (Void_Func*)models->save_file;
        }break;
        case HookID_BufferEditRange:
        {
            result = (Void_Func*)models->buffer_edit_range;
        }break;
        case HookID_BufferRegion:
        {
            result = (Void_Func*)models->buffer_region;
        }break;
        case HookID_Layout:
        {
            result = (Void_Func*)models->layout_func;
        }break;
        case HookID_ViewChangeBuffer:
        {
            result = (Void_Func*)models->view_change_buffer;
        }break;
    }
    return(result);
}

api(custom) function b32
set_custom_hook_memory_size(Application_Links *app, Hook_ID hook_id, u64 size){
    Models *models = (Models*)app->cmd_context;
    b32 result = true;
    switch (hook_id){
        case HookID_DeltaRule:
        {
            models->delta_rule_memory_size = size;
        }break;
        default:
        {
            result = false;
        }break;
    }
    return(result);
}

api(custom) function Mouse_State
get_mouse_state(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return(models->input->mouse);
}

api(custom) function b32
get_active_query_bars(Application_Links *app, View_ID view_id, i32 max_result_count, Query_Bar_Ptr_Array *array_out)
{
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    b32 result = false;
    if (api_check_view(view)){
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
start_query_bar(Application_Links *app, Query_Bar *bar, u32 flags)
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
end_query_bar(Application_Links *app, Query_Bar *bar, u32 flags)
{
    Models *models = (Models*)app->cmd_context;
    Panel *active_panel = layout_get_active_panel(&models->layout);
    View *active_view = active_panel->view;
    free_query_slot(&active_view->query_set, bar);
}

api(custom) function void
clear_all_query_bars(Application_Links *app, View_ID view_id){
    Models *models = (Models*)app->cmd_context;
    View *view = imp_get_view(models, view_id);
    if (api_check_view(view)){
        free_all_queries(&view->query_set);
    }
}

api(custom) function void
print_message(Application_Links *app, String_Const_u8 message)
{
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = models->message_buffer;
    if (file != 0){
        output_file_append(app->tctx, models, file, message);
        file_cursor_to_end(app->tctx, models, file);
    }
}

api(custom) function b32
log_string(Application_Links *app, String_Const_u8 str){
    return(log_string(str));
}

api(custom) function Face_ID
get_largest_face_id(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    return(font_set_get_largest_id(&models->font_set));
}

api(custom) function b32
set_global_face(Application_Links *app, Face_ID id)
{
    Models *models = (Models*)app->cmd_context;
    b32 result = false;
    Face *face = font_set_face_from_id(&models->font_set, id);
    if (face != 0){
        models->global_face_id = face->id;
        result = true;
    }
    return(result);
}

api(custom) function History_Record_Index
buffer_history_get_max_record_index(Application_Links *app, Buffer_ID buffer_id){
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
    out->pos_before_edit = record->pos_before_edit;
    out->edit_number = record->edit_number;
    switch (out->kind){
        case RecordKind_Single:
        {
            out->single_string_forward  = record->single.forward_text ;
            out->single_string_backward = record->single.backward_text;
            out->single_first = record->single.first;
        }break;
        case RecordKind_Group:
        {
            out->group_count = record->group.count;
        }break;
        default:
        {
            InvalidPath;
        }break;
    }
}

api(custom) function Record_Info
buffer_history_get_record_info(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
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
buffer_history_get_group_sub_record(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index, i32 sub_index){
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
buffer_history_get_current_state_index(Application_Links *app, Buffer_ID buffer_id){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    History_Record_Index result = 0;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        result = file_get_current_record_index(file);
    }
    return(result);
}

api(custom) function b32
buffer_history_set_current_state_index(Application_Links *app, Buffer_ID buffer_id, History_Record_Index index){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file) && history_is_activated(&file->state.history)){
        i32 max_index = history_get_record_count(&file->state.history);
        if (0 <= index && index <= max_index){
            edit_change_current_history_state(app->tctx, models, file, index);
            result = true;
        }
    }
    return(result);
}

api(custom) function b32
buffer_history_merge_record_range(Application_Links *app, Buffer_ID buffer_id, History_Record_Index first_index, History_Record_Index last_index, Record_Merge_Flag flags){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    b32 result = false;
    if (api_check_buffer(file)){
        result = edit_merge_history_range(app->tctx, models, file, first_index, last_index, flags);
    }
    return(result);
}

api(custom) function b32
buffer_history_clear_after_current_state(Application_Links *app, Buffer_ID buffer_id){
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
global_history_edit_group_begin(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, 1);
}

api(custom) function void
global_history_edit_group_end(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    global_history_adjust_edit_grouping_counter(&models->global_history, -1);
}

api(custom) function b32
buffer_set_face(Application_Links *app, Buffer_ID buffer_id, Face_ID id)
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
get_face_description(Application_Links *app, Face_ID face_id)
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
        description.parameters.aa_mode = FaceAntialiasingMode_8BitMono;
        description.parameters.pt_size = models->settings.font_size;
        description.parameters.hinting = models->settings.use_hinting;
    }
    return(description);
}

api(custom) function Face_Metrics
get_face_metrics(Application_Links *app, Face_ID face_id){
    Models *models = (Models*)app->cmd_context;
    Face_Metrics result = {};
    if (face_id != 0){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = face->metrics;
        }
    }
    return(result);
}

api(custom) function Face_Advance_Map
get_face_advance_map(Application_Links *app, Face_ID face_id){
    Models *models = (Models*)app->cmd_context;
    Face_Advance_Map result = {};
    if (face_id != 0){
        Face *face = font_set_face_from_id(&models->font_set, face_id);
        if (face != 0){
            result = face->advance_map;
        }
    }
    return(result);
}

api(custom) function Face_ID
get_face_id(Application_Links *app, Buffer_ID buffer_id)
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
try_create_new_face(Application_Links *app, Face_Description *description)
{
    Models *models = (Models*)app->cmd_context;
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *tctx_info = (Thread_Context_Extra_Info*)tctx->user_data;
    Face_ID result = 0;
    if (tctx_info != 0 && tctx_info->coroutine != 0){
        Coroutine *coroutine = (Coroutine*)tctx_info->coroutine;
        Assert(coroutine != 0);
        Co_Out *out = (Co_Out*)coroutine->out;
        out->request = CoRequest_NewFontFace;
        out->face_description = description;
        coroutine_yield(coroutine);
        Co_In *in = (Co_In*)coroutine->in;
        result = in->face_id;
    }
    else if (tctx_info != 0){
        // This API does nothing when called from an async thread.
    }
    else{
        Face *new_face = font_set_new_face(&models->font_set, description);
        if (new_face != 0){
            result = new_face->id;
        }
    }
    return(result);
}

api(custom) function b32
try_modify_face(Application_Links *app, Face_ID id, Face_Description *description)
{
    Models *models = (Models*)app->cmd_context;
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *tctx_info = (Thread_Context_Extra_Info*)tctx->user_data;
    b32 result = false;
    if (tctx_info != 0 && tctx_info->coroutine != 0){
        Coroutine *coroutine = (Coroutine*)tctx_info->coroutine;
        Assert(coroutine != 0);
        Co_Out *out = (Co_Out*)coroutine->out;
        out->request = CoRequest_ModifyFace;
        out->face_description = description;
        out->face_id = id;
        coroutine_yield(coroutine);
        Co_In *in = (Co_In*)coroutine->in;
        result = in->success;
    }
    else if (tctx_info != 0){
        // This API does nothing when called from an async thread.
    }
    else{
        result = font_set_modify_face(&models->font_set, id, description);
    }
    return(result);
}

api(custom) function b32
try_release_face(Application_Links *app, Face_ID id, Face_ID replacement_id)
{
    Models *models = (Models*)app->cmd_context;
    Font_Set *font_set = &models->font_set;
    Face *face = font_set_face_from_id(font_set, id);
    Face *replacement = font_set_face_from_id(font_set, replacement_id);
    return(release_font_and_update(models, face, replacement));
}

api(custom) function String_Const_u8
push_hot_directory(Application_Links *app, Arena *arena)
{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    hot_directory_clean_end(hot);
    return(push_string_copy(arena, hot->string));
}

api(custom) function void
set_hot_directory(Application_Links *app, String_Const_u8 string)
{
    Models *models = (Models*)app->cmd_context;
    Hot_Directory *hot = &models->hot_directory;
    hot_directory_set(hot, string);
}

api(custom) function void
send_exit_signal(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    models->keep_playing = false;
}

api(custom) function void
hard_exit(Application_Links *app)
{
    Models *models = (Models*)app->cmd_context;
    models->hard_exit = true;
}

api(custom) function void
set_window_title(Application_Links *app, String_Const_u8 title)
{
    Models *models = (Models*)app->cmd_context;
    models->has_new_title = true;
    u64 cap_before_null = (u64)(models->title_capacity - 1);
    u64 copy_size = clamp_top(title.size, cap_before_null);
    block_copy(models->title_space, title.str, copy_size);
    models->title_space[copy_size] = 0;
}

api(custom) function void
acquire_global_frame_mutex(Application_Links *app){
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *tctx_info = (Thread_Context_Extra_Info*)tctx->user_data;
    if (tctx_info != 0 && tctx_info->coroutine != 0){
        Coroutine *coroutine = (Coroutine*)tctx_info->coroutine;
        Assert(coroutine != 0);
        Co_Out *out = (Co_Out*)coroutine->out;
        out->request = CoRequest_AcquireGlobalFrameMutex;
        coroutine_yield(coroutine);
    }
    else{
        system_acquire_global_frame_mutex(tctx);
    }
}

api(custom) function void
release_global_frame_mutex(Application_Links *app){
    Thread_Context *tctx = app->tctx;
    Thread_Context_Extra_Info *tctx_info = (Thread_Context_Extra_Info*)tctx->user_data;
    if (tctx_info != 0 && tctx_info->coroutine != 0){
        Coroutine *coroutine = (Coroutine*)tctx_info->coroutine;
        Assert(coroutine != 0);
        Co_Out *out = (Co_Out*)coroutine->out;
        out->request = CoRequest_ReleaseGlobalFrameMutex;
        coroutine_yield(coroutine);
    }
    else{
        system_release_global_frame_mutex(tctx);
    }
}

////////////////////////////////

api(custom) function Vec2_f32
draw_string_oriented(Application_Links *app, Face_ID font_id, ARGB_Color color,
                     String_Const_u8 str, Vec2_f32 point, u32 flags, Vec2_f32 delta)
{
    Vec2_f32 result = point;
    Models *models = (Models*)app->cmd_context;
    Face *face = font_set_face_from_id(&models->font_set, font_id);
    if (models->target == 0){
        f32 width = font_string_width(models->target, face, str);
        result += delta*width;
    }
    else{
        f32 width = draw_string(models->target, face, str, point, color, flags, delta);
        result += delta*width;
    }
    return(result);
}

api(custom) function f32
get_string_advance(Application_Links *app, Face_ID font_id, String_Const_u8 str)
{
    Models *models = (Models*)app->cmd_context;
    Face *face = font_set_face_from_id(&models->font_set, font_id);
    return(font_string_width(models->target, face, str));
}

api(custom) function void
draw_rectangle(Application_Links *app, Rect_f32 rect, f32 roundness, ARGB_Color color){
    Models *models = (Models*)app->cmd_context;
    if (models->in_render_mode){
        draw_rectangle(models->target, rect, roundness, color);
    }
}

api(custom) function void
draw_rectangle_outline(Application_Links *app, Rect_f32 rect, f32 roundness, f32 thickness, ARGB_Color color){
    Models *models = (Models*)app->cmd_context;
    if (models->in_render_mode){
        draw_rectangle_outline(models->target, rect, roundness, thickness, color);
    }
}

api(custom) function Rect_f32
draw_set_clip(Application_Links *app, Rect_f32 new_clip){
    Models *models = (Models*)app->cmd_context;
    return(draw_set_clip(models->target, new_clip));
}

api(custom) function Text_Layout_ID
text_layout_create(Application_Links *app, Buffer_ID buffer_id, Rect_f32 rect, Buffer_Point buffer_point){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer_id);
    Text_Layout_ID result = 0;
    if (api_check_buffer(file)){
        Thread_Context *tctx = app->tctx;
        Face *face = file_get_face(models, file);
        
        Gap_Buffer *buffer = &file->state.buffer;
        
        Layout_Function *layout_func = file_get_layout_func(file);
        
        Vec2_f32 dim = rect_dim(rect);
        
        i64 line_count = buffer_line_count(buffer);
        i64 line_number = buffer_point.line_number;
        f32 y = -buffer_point.pixel_shift.y;
        for (;line_number <= line_count;
             line_number += 1){
            Layout_Item_List line = file_get_line_layout(tctx, models, file,
                                                         layout_func, dim.x, face,
                                                         line_number);
            f32 next_y = y + line.height;
            if (next_y >= dim.y){
                break;
            }
            y = next_y;
        }
        
        Range_i64 visible_line_number_range = Ii64(buffer_point.line_number, line_number);
        Range_i64 visible_range = Ii64(buffer_get_first_pos_from_line_number(buffer, visible_line_number_range.min),
                                       buffer_get_last_pos_from_line_number(buffer, visible_line_number_range.max));
        
        i64 item_count = range_size_inclusive(visible_range);
        
        Arena arena = make_arena_system();
        Arena *arena_ptr = push_array_zero(&arena, Arena, 1);
        *arena_ptr = arena;
        ARGB_Color *colors_array = push_array_zero(arena_ptr, ARGB_Color, item_count);
        result = text_layout_new(&models->text_layouts, arena_ptr, buffer_id, buffer_point,
                                 visible_range, visible_line_number_range, rect, colors_array,
                                 layout_func);
    }
    return(result);
}

api(custom) function Rect_f32
text_layout_region(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, text_layout_id);
    if (layout != 0){
        result = layout->rect;
    }
    return(result);
}

api(custom) function Buffer_ID
text_layout_get_buffer(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    Buffer_ID result = 0;
    Text_Layout *layout = text_layout_get(&models->text_layouts, text_layout_id);
    if (layout != 0){
        result = layout->buffer_id;
    }
    return(result);
}

api(custom) function Range_i64
text_layout_get_visible_range(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    Range_i64 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, text_layout_id);
    if (layout != 0){
        result = layout->visible_range;
    }
    return(result);
}

api(custom) function Range_f32
text_layout_line_on_screen(Application_Links *app, Text_Layout_ID layout_id, i64 line_number){
    Models *models = (Models*)app->cmd_context;
    Range_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout == 0){
        return(result);
    }
    
    Layout_Function *layout_func = layout->layout_func;
    
    Rect_f32 rect = layout->rect;
    if (range_contains_inclusive(layout->visible_line_number_range, line_number)){
        Editing_File *file = imp_get_file(models, layout->buffer_id);
        if (api_check_buffer(file)){
            f32 width = rect_width(rect);
            Face *face = file_get_face(models, file);
            
            for (i64 line_number_it = layout->visible_line_number_range.first;;
                 line_number_it += 1){
                Layout_Item_List line = file_get_line_layout(app->tctx, models, file,
                                                             layout_func, width, face,
                                                             line_number_it);
                result.max += line.height;
                if (line_number_it == line_number){
                    break;
                }
                result.min = result.max;
            }
            
            result += rect.y0 - layout->point.pixel_shift.y;
        }
    }
    else if (line_number < layout->visible_line_number_range.min){
        result = If32(rect.y0, rect.y0);
    }
    else if (line_number > layout->visible_line_number_range.max){
        result = If32(rect.y1, rect.y1);
    }
    
    return(result);
}

api(custom) function Rect_f32
text_layout_character_on_screen(Application_Links *app, Text_Layout_ID layout_id, i64 pos){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0 && range_contains_inclusive(layout->visible_range, pos)){
        Editing_File *file = imp_get_file(models, layout->buffer_id);
        if (api_check_buffer(file)){
            Gap_Buffer *buffer = &file->state.buffer;
            i64 line_number = buffer_get_line_index(buffer, pos) + 1;
            
            if (range_contains_inclusive(layout->visible_line_number_range, line_number)){
                Rect_f32 rect = layout->rect;
                f32 width = rect_width(rect);
                Face *face = file_get_face(models, file);
                
                Layout_Function *layout_func = layout->layout_func;
                
                f32 y = 0.f;
                Layout_Item_List line = {};
                for (i64 line_number_it = layout->visible_line_number_range.first;;
                     line_number_it += 1){
                    line = file_get_line_layout(app->tctx, models, file,
                                                layout_func, width, face,
                                                line_number_it);
                    if (line_number_it == line_number){
                        break;
                    }
                    y += line.height;
                }
                
                // TODO(allen): optimization: This is some fairly heavy computation.  We really
                // need to accelerate the (pos -> item) lookup within a single
                // Buffer_Layout_Item_List.
                b32 is_first_item = true;
                result = Rf32_negative_infinity;
                for (Layout_Item_Block *block = line.first;
                     block != 0;
                     block = block->next){
                    i64 count = block->item_count;
                    Layout_Item *item_ptr = block->items;
                    for (i32 i = 0; i < count; i += 1, item_ptr += 1){
                        if (HasFlag(item_ptr->flags, LayoutItemFlag_Ghost_Character)){
                            continue;
                        }
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
            }
        }
    }
    return(result);
}

api(custom) function void
paint_text_color(Application_Links *app, Text_Layout_ID layout_id, Range_i64 range, ARGB_Color color){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0){
        range.min = clamp_bot(layout->visible_range.min, range.min);
        range.max = clamp_top(range.max, layout->visible_range.max);
        range.min -= layout->visible_range.min;
        range.max -= layout->visible_range.min;
        ARGB_Color *color_ptr = layout->item_colors + range.min;
        for (i64 i = range.min; i < range.max; i += 1, color_ptr += 1){
            *color_ptr = color;
        }
    }
}

api(custom) function void
paint_text_color_blend(Application_Links *app, Text_Layout_ID layout_id, Range_i64 range, ARGB_Color color, f32 blend){
    Models *models = (Models*)app->cmd_context;
    Rect_f32 result = {};
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0){
        range.min = clamp_bot(layout->visible_range.min, range.min);
        range.max = clamp_top(range.max, layout->visible_range.max);
        range.min -= layout->visible_range.min;
        range.max -= layout->visible_range.min;
        Vec4_f32 color_v4f32 = unpack_color(color);
        Vec4_f32 color_pm_v4f32 = color_v4f32*blend;
        f32 neg_blend = 1.f - blend;
        ARGB_Color *color_ptr = layout->item_colors + range.min;
        for (i64 i = range.min; i < range.max; i += 1, color_ptr += 1){
            Vec4_f32 color_ptr_v4f32 = unpack_color(*color_ptr);
            Vec4_f32 blended_v4f32 = color_ptr_v4f32*neg_blend + color_pm_v4f32;
            *color_ptr = pack_color(blended_v4f32);
        }
    }
}

api(custom) function b32
text_layout_free(Application_Links *app, Text_Layout_ID text_layout_id){
    Models *models = (Models*)app->cmd_context;
    return(text_layout_erase(app->tctx, models, &models->text_layouts, text_layout_id));
}

api(custom) function void
draw_text_layout(Application_Links *app, Text_Layout_ID layout_id, ARGB_Color special_color, ARGB_Color ghost_color){
    Models *models = (Models*)app->cmd_context;
    Text_Layout *layout = text_layout_get(&models->text_layouts, layout_id);
    if (layout != 0 && models->target != 0){
        text_layout_render(app->tctx, models, layout, special_color, ghost_color);
    }
}

api(custom) function void
open_color_picker(Application_Links *app, Color_Picker *picker)
{
    Models *models = (Models*)app->cmd_context;
    if (picker->finished != 0){
        *picker->finished = false;
    }
    system_open_color_picker(picker);
}

api(custom) function void
animate_in_n_milliseconds(Application_Links *app, u32 n)
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
buffer_find_all_matches(Application_Links *app, Arena *arena, Buffer_ID buffer,
                        i32 string_id, Range_i64 range, String_Const_u8 needle,
                        Character_Predicate *predicate, Scan_Direction direction){
    Models *models = (Models*)app->cmd_context;
    Editing_File *file = imp_get_file(models, buffer);
    String_Match_List list = {};
    if (api_check_buffer(file)){
        if (needle.size > 0){
            Scratch_Block scratch(app, arena);
            List_String_Const_u8 chunks = buffer_get_chunks(scratch, &file->state.buffer);
            buffer_chunks_clamp(&chunks, range);
            if (chunks.node_count > 0){
                u64_Array jump_table = string_compute_needle_jump_table(arena, needle, direction);
                Character_Predicate dummy = {};
                if (predicate == 0){
                    predicate = &dummy;
                }
                list = find_all_matches(arena, max_i32,
                                        chunks, needle, jump_table, predicate,
                                        direction, range.min, buffer, string_id);
            }
        }
    }
    return(list);
}

////////////////////////////////

api(custom) function Profile_Global_List*
get_core_profile_list(Application_Links *app){
    Models *models = (Models*)app->cmd_context;
    return(&models->profile_list);
}

api(custom) function Doc_Cluster*
get_custom_layer_boundary_docs(Application_Links *app, Arena *arena){
    API_Definition *api_def = custom_api_construct(arena);
    return(doc_custom_api(arena, api_def));
}

// BOTTOM
