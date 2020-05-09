/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * Viewing
 *
 */

// TOP

function void
begin_handling_input(Models *models, User_Input *input){
    block_copy_struct(&models->current_input, input);
    models->current_input_sequence_number += 1;
}

////////////////////////////////

internal void
init_query_set(Query_Set *set){
    Query_Slot *slot = set->slots;
    set->free_slot = slot;
    set->used_slot = 0;
    for (i32 i = 0; i+1 < ArrayCount(set->slots); ++i, ++slot){
        slot->next = slot + 1;
    }
}

internal Query_Slot*
alloc_query_slot(Query_Set *set){
    Query_Slot *slot = set->free_slot;
    if (slot != 0){
        set->free_slot = slot->next;
        slot->next = set->used_slot;
        set->used_slot = slot;
    }
    return(slot);
}

internal void
free_query_slot(Query_Set *set, Query_Bar *match_bar){
    Query_Slot *slot = 0;
    Query_Slot *prev = 0;
    
    for (slot = set->used_slot; slot != 0; slot = slot->next){
        if (slot->query_bar == match_bar) break;
        prev = slot;
    }
    
    if (slot){
        if (prev){
            prev->next = slot->next;
        }
        else{
            set->used_slot = slot->next;
        }
        slot->next = set->free_slot;
        set->free_slot = slot;
    }
}

function void
free_all_queries(Query_Set *set){
    for (;set->used_slot != 0;){
        Query_Slot *slot = set->used_slot;
        set->used_slot = slot->next;
        slot->next = set->free_slot;
        set->free_slot = slot;
    }
}

////////////////////////////////

internal Access_Flag
view_get_access_flags(View *view){
    Access_Flag result = file_get_access_flags(view->file);
    View_Context_Node *node = view->ctx;
    b32 hides_buffer = (node != 0 && node->ctx.hides_buffer);
    if (hides_buffer){
        RemFlag(result, Access_Visible);
    }
    return(result);
}

internal i32
view_get_index(Live_Views *live_set, View *view){
    return((i32)(view - live_set->views));
}

internal View_ID
view_get_id(Live_Views *live_set, View *view){
    return((View_ID)(view - live_set->views) + 1);
}

internal View*
live_set_alloc_view(Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, Panel *panel){
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    View *result = live_set->free_sentinel.next;
    dll_remove(result);
    block_zero_struct(result);
    
    result->in_use = true;
    init_query_set(&result->query_set);
    result->lifetime_object = lifetime_alloc_object(lifetime_allocator, DynamicWorkspace_View, result);
    panel->view = result;
    result->panel = panel;
    
    return(result);
}

internal void
live_set_free_view(Lifetime_Allocator *lifetime_allocator, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    
    view->next = live_set->free_sentinel.next;
    view->prev = &live_set->free_sentinel;
    live_set->free_sentinel.next = view;
    view->next->prev = view;
    view->in_use = false;
    
    lifetime_free_object(lifetime_allocator, view->lifetime_object);
}

////////////////////////////////

internal File_Edit_Positions
view_get_edit_pos(View *view){
    return(view->edit_pos_);
}

internal void
view_set_edit_pos(View *view, File_Edit_Positions edit_pos){
    edit_pos.scroll.position.line_number = clamp_bot(1, edit_pos.scroll.position.line_number);
    edit_pos.scroll.target.line_number = clamp_bot(1, edit_pos.scroll.target.line_number);
    view->edit_pos_ = edit_pos;
    view->file->state.edit_pos_most_recent = edit_pos;
}

////////////////////////////////

internal Rect_f32
view_get_buffer_rect(Thread_Context *tctx, Models *models, View *view){
    Rect_f32 region = Rf32(view->panel->rect_full);
    if (models->buffer_region != 0){
        Rect_f32 rect = region;
        Rect_f32 sub_region = Rf32(V2f32(0, 0), rect_dim(rect));
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
        sub_region = models->buffer_region(&app, view_get_id(&models->view_set, view), sub_region);
        region.p0 = rect.p0 + sub_region.p0;
        region.p1 = rect.p0 + sub_region.p1;
        region.x1 = clamp_top(region.x1, rect.x1);
        region.y1 = clamp_top(region.y1, rect.y1);
        region.x0 = clamp_top(region.x0, region.x1);
        region.y0 = clamp_top(region.y0, region.y1);
    }
    return(region);
}

internal f32
view_width(Thread_Context *tctx, Models *models, View *view){
    return(rect_width(view_get_buffer_rect(tctx, models, view)));
}

internal f32
view_height(Thread_Context *tctx, Models *models, View *view){
    return(rect_height(view_get_buffer_rect(tctx, models, view)));
}

////////////////////////////////

internal Layout_Item_List
view_get_line_layout(Thread_Context *tctx, Models *models, View *view, i64 line_number){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_get_line_layout(tctx, models, file, layout_func, width, face, line_number));
}

internal Line_Shift_Vertical
view_line_shift_y(Thread_Context *tctx, Models *models, View *view,
                  i64 line_number, f32 y_delta){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_line_shift_y(tctx, models, file, layout_func, width, face,
                             line_number, y_delta));
}

internal f32
view_line_y_difference(Thread_Context *tctx, Models *models, View *view,
                       i64 line_a, i64 line_b){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_line_y_difference(tctx, models, file,
                                  layout_func, width, face, line_a, line_b));
}

internal i64
view_pos_at_relative_xy(Thread_Context *tctx, Models *models, View *view,
                        i64 base_line, Vec2_f32 relative_xy){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_pos_at_relative_xy(tctx, models, file,
                                   layout_func, width, face, base_line, relative_xy));
}

internal Rect_f32
view_relative_box_of_pos(Thread_Context *tctx, Models *models, View *view,
                         i64 base_line, i64 pos){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_relative_box_of_pos(tctx, models, file,
                                    layout_func, width, face, base_line, pos));
}

internal Vec2_f32
view_relative_xy_of_pos(Thread_Context *tctx, Models *models, View *view,
                        i64 base_line, i64 pos){
    Rect_f32 rect = view_relative_box_of_pos(tctx, models, view, base_line, pos);
    return(rect_center(rect));
}

function Rect_f32
view_padded_box_of_pos(Thread_Context *tctx, Models *models, View *view,
                       i64 base_line, i64 pos){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_padded_box_of_pos(tctx, models, file,
                                  layout_func, width, face, base_line, pos));
}

internal Buffer_Point
view_normalize_buffer_point(Thread_Context *tctx, Models *models, View *view,
                            Buffer_Point point){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_normalize_buffer_point(tctx, models, file,
                                       layout_func, width, face, point));
}

internal Vec2_f32
view_buffer_point_difference(Thread_Context *tctx, Models *models, View *view,
                             Buffer_Point a, Buffer_Point b){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_buffer_point_difference(tctx, models, file,
                                        layout_func, width, face, a, b));
}

internal Buffer_Point
view_move_buffer_point(Thread_Context *tctx, Models *models, View *view,
                       Buffer_Point buffer_point, Vec2_f32 delta){
    delta += buffer_point.pixel_shift;
    Line_Shift_Vertical shift = view_line_shift_y(tctx, models, view, buffer_point.line_number, delta.y);
    buffer_point.line_number = shift.line;
    buffer_point.pixel_shift = V2f32(delta.x, delta.y - shift.y_delta);
    return(buffer_point);
}

internal Line_Shift_Character
view_line_shift_characters(Thread_Context *tctx, Models *models, View *view,
                           i64 line_number, i64 character_delta){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_line_shift_characters(tctx, models, file,
                                      layout_func, width, face, line_number, character_delta));
}

internal i64
view_line_character_difference(Thread_Context *tctx, Models *models, View *view,
                               i64 line_a, i64 line_b){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_line_character_difference(tctx, models, file, layout_func, width, face,
                                          line_a, line_b));
}

internal i64
view_pos_from_relative_character(Thread_Context *tctx, Models *models, View *view,
                                 i64 base_line, i64 relative_character){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_pos_from_relative_character(tctx, models, file, layout_func, width, face,
                                            base_line, relative_character));
}

internal i64
view_relative_character_from_pos(Thread_Context *tctx, Models *models, View *view,
                                 i64 base_line, i64 pos){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    f32 width = view_width(tctx, models, view);
    Layout_Function *layout_func = file_get_layout_func(file);
    return(file_relative_character_from_pos(tctx, models, file,
                                            layout_func, width, face, base_line, pos));
}

internal Buffer_Cursor
view_compute_cursor(View *view, Buffer_Seek seek){
    Editing_File *file = view->file;
    return(file_compute_cursor(file, seek));
}

////////////////////////////////

internal b32
view_move_view_to_cursor(Thread_Context *tctx, Models *models, View *view, Buffer_Scroll *scroll){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    Rect_f32 rect = view_get_buffer_rect(tctx, models, view);
    Vec2_f32 view_dim = rect_dim(rect);
    
    Layout_Function *layout_func = file_get_layout_func(file);
    
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    Vec2_f32 p = file_relative_xy_of_pos(tctx, models, file,
                                         layout_func, view_dim.x, face,
                                         scroll->target.line_number, edit_pos.cursor_pos);
    p -= scroll->target.pixel_shift;
    
    f32 line_height = face->metrics.line_height;
    f32 normal_advance = face->metrics.normal_advance;
    
    Vec2_f32 margin = view->cursor_margin;
    Vec2_f32 push_in = view->cursor_push_in_multiplier;
    
    Vec2_f32 lim_dim = view_dim*0.45f;
    margin.x = clamp_top(margin.x, lim_dim.x);
    margin.y = clamp_top(margin.y, lim_dim.y);
    
    Vec2_f32 push_in_lim_dim = hadamard(lim_dim, V2f32(1.f/line_height, 1.f/normal_advance)) - margin;
	push_in_lim_dim.x = clamp_bot(0.f, push_in_lim_dim.x);
	push_in_lim_dim.y = clamp_bot(0.f, push_in_lim_dim.y);
    push_in.x = clamp_top(push_in.x, push_in_lim_dim.x);
    push_in.y = clamp_top(push_in.y, push_in_lim_dim.y);
    
    Vec2_f32 target_p_relative = {};
    if (p.y < margin.y){
        target_p_relative.y = p.y - margin.y - line_height*push_in.y;
    }
    else if (p.y > view_dim.y - margin.y){
        target_p_relative.y = (p.y + margin.y + line_height*push_in.y) - view_dim.y;
    }
    if (p.x < margin.x){
        target_p_relative.x = p.x - margin.x - normal_advance*push_in.x;
    }
    else if (p.x > view_dim.x - margin.x){
        target_p_relative.x = (p.x + margin.x + normal_advance*push_in.x) - view_dim.x;
    }
    scroll->target.pixel_shift += target_p_relative;
    scroll->target = view_normalize_buffer_point(tctx, models, view, scroll->target);
    scroll->target.pixel_shift.x = f32_round32(scroll->target.pixel_shift.x);
    scroll->target.pixel_shift.y = f32_round32(scroll->target.pixel_shift.y);
    
    return(target_p_relative != V2f32(0.f, 0.f));
}

internal b32
view_move_cursor_to_view(Thread_Context *tctx, Models *models, View *view, Buffer_Scroll scroll, i64 *pos_in_out, f32 preferred_x){
    Editing_File *file = view->file;
    Face *face = file_get_face(models, file);
    Rect_f32 rect = view_get_buffer_rect(tctx, models, view);
    Vec2_f32 view_dim = rect_dim(rect);
    
    Layout_Function *layout_func = file_get_layout_func(file);
    
    Vec2_f32 p = file_relative_xy_of_pos(tctx, models, file,
                                         layout_func, view_dim.x, face,
                                         scroll.target.line_number, *pos_in_out);
    p -= scroll.target.pixel_shift;
    
    f32 line_height = face->metrics.line_height;
    
    b32 adjusted_y = true;
    if (p.y < 0.f){
        p.y = line_height*1.5f;
    }
    else if (p.y > view_dim.y){
        p.y = view_dim.y - line_height*1.5f;
    }
    else{
        adjusted_y = false;
    }
    
    b32 result = false;
    if (adjusted_y){
        p += scroll.target.pixel_shift;
        *pos_in_out = file_pos_at_relative_xy(tctx, models, file,
                                              layout_func, view_dim.x, face,
                                              scroll.target.line_number, p);
        result = true;
    }
    
    return(result);
}

internal void
view_set_cursor(Thread_Context *tctx, Models *models, View *view, i64 pos){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_cursor(&edit_pos, pos);
    view_set_edit_pos(view, edit_pos);
    Buffer_Scroll scroll = edit_pos.scroll;
    if (view_move_view_to_cursor(tctx, models, view, &scroll)){
        edit_pos.scroll = scroll;
        view_set_edit_pos(view, edit_pos);
    }
}

internal void
view_set_scroll(Thread_Context *tctx, Models *models, View *view, Buffer_Scroll scroll){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_scroll(&edit_pos, scroll);
    view_set_edit_pos(view, edit_pos);
    if (view_move_cursor_to_view(tctx, models, view, edit_pos.scroll, &edit_pos.cursor_pos, view->preferred_x)){
        view_set_edit_pos(view, edit_pos);
    }
}

internal void
view_set_cursor_and_scroll(Thread_Context *tctx, Models *models, View *view, i64 pos, Buffer_Scroll scroll){
    File_Edit_Positions edit_pos = view_get_edit_pos(view);
    file_edit_positions_set_cursor(&edit_pos, pos);
    Buffer_Cursor cursor = view_compute_cursor(view, seek_pos(pos));
    Vec2_f32 p = view_relative_xy_of_pos(tctx, models, view, cursor.line, pos);
    view->preferred_x = p.x;
    file_edit_positions_set_scroll(&edit_pos, scroll);
    edit_pos.last_set_type = EditPos_None;
    view_set_edit_pos(view, edit_pos);
}

////////////////////////////////

internal void
view_set_file(Thread_Context *tctx, Models *models, View *view, Editing_File *file){
    Assert(file != 0);
    
    Editing_File *old_file = view->file;
    
    if (models->view_change_buffer != 0){
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
        models->view_change_buffer(&app, view_get_id(&models->view_set, view),
                                   (old_file != 0)?old_file->id:0, file->id);
    }
    
    if (old_file != 0){
        file_touch(&models->working_set, old_file);
        file_edit_positions_push(old_file, view_get_edit_pos(view));
    }
    
    view->file = file;
    
    File_Edit_Positions edit_pos = file_edit_positions_pop(file);
    view_set_edit_pos(view, edit_pos);
    view->mark = edit_pos.cursor_pos;
    Buffer_Cursor cursor = view_compute_cursor(view, seek_pos(edit_pos.cursor_pos));
    Vec2_f32 p = view_relative_xy_of_pos(tctx, models, view, cursor.line, edit_pos.cursor_pos);
    view->preferred_x = p.x;
    
    models->layout.panel_state_dirty = true;
}

////////////////////////////////

function void
view_push_context(View *view, View_Context *ctx){
    Temp_Memory pop_me = begin_temp(&view->node_arena);
    View_Context_Node *node = push_array_zero(&view->node_arena, View_Context_Node, 1);
    sll_stack_push(view->ctx, node);
    node->pop_me = pop_me;
    block_copy_struct(&node->ctx, ctx);
    node->delta_rule_memory = push_array_zero(&view->node_arena, u8, ctx->delta_rule_memory_size);
}

function void
view_alter_context(View *view, View_Context *ctx){
    View_Context_Node *node = view->ctx;
    Assert(node != 0);
    block_copy_struct(&node->ctx, ctx);
}

function void
view_pop_context(View *view){
    View_Context_Node *node = view->ctx;
    if (node != 0 && node->next != 0){
        sll_stack_pop(view->ctx);
        end_temp(node->pop_me);
    }
}

function View_Context_Node*
view_current_context_node(View *view){
    return(view->ctx);
}

function View_Context
view_current_context(View *view){
    View_Context ctx = {};
    View_Context_Node *node = view->ctx;
    if (node != 0){
        block_copy_struct(&ctx, &node->ctx);
    }
    return(ctx);
}

////////////////////////////////

internal Coroutine*
co_handle_request(Thread_Context *tctx, Models *models, Coroutine *co, Co_Out *out){
    Coroutine *result = 0;
    switch (out->request){
        case CoRequest_NewFontFace:
        {
            Face_Description *description = out->face_description;
            Face *face = font_set_new_face(&models->font_set, description);
            Co_In in = {};
            in.face_id = (face != 0)?face->id:0;
            result = coroutine_run(&models->coroutines, co, &in, out);
        }break;
        
        case CoRequest_ModifyFace:
        {
            Face_Description *description = out->face_description;
            Face_ID face_id = out->face_id;
            Co_In in = {};
            in.success = font_set_modify_face(&models->font_set, face_id, description);
            result = coroutine_run(&models->coroutines, co, &in, out);
        }break;
        
        case CoRequest_AcquireGlobalFrameMutex:
        {
            system_acquire_global_frame_mutex(tctx);
            result = coroutine_run(&models->coroutines, co, 0, out);
        }break;
        
        case CoRequest_ReleaseGlobalFrameMutex:
        {
            system_release_global_frame_mutex(tctx);
            result = coroutine_run(&models->coroutines, co, 0, out);
        }break;
    }
    return(result);
}

internal Coroutine*
co_run(Thread_Context *tctx, Models *models, Coroutine *co, Co_In *in, Co_Out *out){
    Coroutine *result = coroutine_run(&models->coroutines, co, in, out);
    for (;result != 0 && out->request != CoRequest_None;){
        result = co_handle_request(tctx, models, result, out);
    }
    return(result);
}

internal void
view_event_context_base__inner(Coroutine *coroutine){
    Co_In *in = (Co_In*)coroutine->in;
    Models *models = in->models;
    Custom_Command_Function *event_context_base = in->event_context_base;
    Assert(event_context_base != 0);
    Application_Links app = {};
    app.tctx = coroutine->tctx;
    app.cmd_context = models;
    event_context_base(&app);
}

function void
view_init(Thread_Context *tctx, Models *models, View *view, Editing_File *initial_buffer,
          Custom_Command_Function *event_context_base){
    view_set_file(tctx, models, view, initial_buffer);
    
    view->node_arena = make_arena_system();
    
    View_Context first_ctx = {};
    first_ctx.render_caller = models->render_caller;
    first_ctx.delta_rule = models->delta_rule;
    first_ctx.delta_rule_memory_size = models->delta_rule_memory_size;
    view_push_context(view, &first_ctx);
    
    view->cursor_margin = V2f32(0.f, 0.f);
    view->cursor_push_in_multiplier = V2f32(1.5f, 1.5f);
    
    view->co = coroutine_create(&models->coroutines, view_event_context_base__inner);
    view->co->user_data = view;
    Co_In in = {};
    in.models = models;
    in.event_context_base = event_context_base;
    view->co = co_run(tctx, models, view->co, &in, &view->co_out);
    // TODO(allen): deal with this kind of problem!
    Assert(view->co != 0);
}

// TODO(allen): This doesn't make any sense!!!!!! COROUTINE SHUTDOWN? VIEW CLOSING? WADAFUQ?

function b32
view_close(Models *models, View *view){
    Layout *layout = &models->layout;
    b32 result = false;
    if (layout_close_panel(layout, view->panel)){
        if (view->co != 0){
            models_push_wind_down(models, view->co);
        }
        live_set_free_view(&models->lifetime_allocator, &models->view_set, view);
        result = true;
    }
    return(result);
}

internal void
view_check_co_exited(Models *models, View *view){
    if (view->co == 0){
        b32 result = view_close(models, view);
        // TODO(allen): Here it looks like the final view has
        // exited from it's event handler.  We should probably
        // have a failsafe restarter for the event handler when
        // this happens.
        Assert(result);
    }
}

// TODO(allen): This is dumb. Let's rethink view cleanup strategy.

internal void
co_single_abort(Thread_Context *tctx, Models *models, View *view){
    Coroutine *co = view->co;
    Co_In in = {};
    in.user_input.abort = true;
    view->co = co_run(tctx, models, co, &in, &view->co_out);
    view_check_co_exited(models, view);
}

internal void
co_full_abort(Thread_Context *tctx, Models *models, View *view){
    Coroutine *co = view->co;
    Co_In in = {};
    in.user_input.abort = true;
    for (u32 j = 0; j < 100 && co != 0; ++j){
        co  = co_run(tctx, models, co, &in, &view->co_out);
    }
    if (co != 0){
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
#define M "SERIOUS ERROR: full stack abort did not complete"
        print_message(&app, string_u8_litexpr(M));
#undef M
    }
    view->co = 0;
    init_query_set(&view->query_set);
}

function b32
co_send_event(Thread_Context *tctx, Models *models, View *view, Input_Event *event){
    b32 event_was_handled = false;
    
    Coroutine *co = view->co;
    Co_Out *co_out = &view->co_out;
    
    {
        models->current_input_unhandled = false;
        Co_In in = {};
        in.user_input.event = *event;
        in.user_input.abort = false;
        begin_handling_input(models, &in.user_input);
        view->co = co_run(tctx, models, view->co, &in, &view->co_out);
        view_check_co_exited(models, view);
        if (!(event->kind == InputEventKind_Core && event->core.code == CoreCode_Animate)){
            models->animate_next_frame = true;
        }
        event_was_handled = !models->current_input_unhandled;
    }
    
    return(event_was_handled);
}

function b32
co_send_core_event(Thread_Context *tctx, Models *models, View *view, Core_Code code, String_Const_u8 string){
    Input_Event event = {};
    event.kind = InputEventKind_Core;
    event.core.code = code;
    event.core.string = string;
    return(co_send_event(tctx, models, view, &event));
}

function b32
co_send_core_event(Thread_Context *tctx, Models *models, View *view, Core_Code code, Buffer_ID id){
    Input_Event event = {};
    event.kind = InputEventKind_Core;
    event.core.code = code;
    event.core.id = id;
    return(co_send_event(tctx, models, view, &event));
}

function b32
co_send_core_event(Thread_Context *tctx, Models *models, View *view, Core_Code code){
    return(co_send_core_event(tctx, models, view, code, SCu8()));
}

function b32
co_send_event(Thread_Context *tctx, Models *models, Input_Event *event){
    Panel *active_panel = models->layout.active_panel;
    View *view = active_panel->view;
    return(co_send_event(tctx, models, view, event));
}

function b32
co_send_core_event(Thread_Context *tctx, Models *models, Core_Code code, String_Const_u8 string){
    Panel *active_panel = models->layout.active_panel;
    View *view = active_panel->view;
    return(co_send_core_event(tctx, models, view, code, string));
}

function b32
co_send_core_event(Thread_Context *tctx, Models *models, Core_Code code, Buffer_ID buffer_id){
    Panel *active_panel = models->layout.active_panel;
    View *view = active_panel->view;
    return(co_send_core_event(tctx, models, view, code, buffer_id));
}

function b32
co_send_core_event(Thread_Context *tctx, Models *models, Core_Code code){
    return(co_send_core_event(tctx, models, code, SCu8()));
}

////////////////////////////////

function void
view_quit_ui(Thread_Context *tctx, Models *models, View *view){
    for (u32 j = 0;; j += 1){
        if (j == 100){
            Application_Links app = {};
            app.tctx = tctx;
            app.cmd_context = models;
#define M "SERIOUS ERROR: view quit ui did not complete"
            print_message(&app, string_u8_litexpr(M));
#undef M
            break;
        }
        View_Context_Node *ctx = view->ctx;
        if (ctx->next == 0){
            break;
        }
        co_single_abort(tctx, models, view);
    }
}

////////////////////////////////

internal b32
file_is_viewed(Layout *layout, Editing_File *file){
    b32 is_viewed = false;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file == file){
            is_viewed = true;
            break;
        }
    }
    return(is_viewed);
}

internal void
adjust_views_looking_at_file_to_new_cursor(Thread_Context *tctx, Models *models, Editing_File *file){
    Layout *layout = &models->layout;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file == file){
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            view_set_cursor(tctx, models, view, edit_pos.cursor_pos);
        }
    }
}

internal void
global_set_font_and_update_files(Models *models, Face *new_global_face){
    for (Node *node = models->working_set.active_file_sentinel.next;
         node != &models->working_set.active_file_sentinel;
         node = node->next){
        Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
        file->settings.face_id = new_global_face->id;
    }
    models->global_face_id = new_global_face->id;
}

internal b32
release_font_and_update(Models *models, Face *face, Face *replacement_face){
    b32 success = false;
    Assert(replacement_face != 0 && replacement_face != face);
    if (font_set_release_face(&models->font_set, face->id)){
        for (Node *node = models->working_set.active_file_sentinel.next;
             node != &models->working_set.active_file_sentinel;
             node = node->next){
            Editing_File *file = CastFromMember(Editing_File, main_chain_node, node);
            if (file->settings.face_id == face->id){
                file->settings.face_id = replacement_face->id;
            }
        }
        if (models->global_face_id == face->id){
            models->global_face_id = replacement_face->id;
        }
        success = true;
    }
    return(success);
}

////////////////////////////////

internal View*
imp_get_view(Models *models, View_ID view_id){
    Live_Views *view_set = &models->view_set;
    View *view = 0;
    view_id -= 1;
    if (0 <= view_id && view_id < view_set->max){
        view = view_set->views + view_id;
        if (!view->in_use){
            view = 0;
        }
    }
    return(view);
}

// BOTTOM

